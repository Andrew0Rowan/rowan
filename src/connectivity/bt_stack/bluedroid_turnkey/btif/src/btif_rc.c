/*
 * Copyright (C) 2015 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/*****************************************************************************
 *
 *  Filename:      btif_rc.c
 *
 *  Description:   Bluetooth AVRC implementation
 *
 *****************************************************************************/

#define LOG_TAG "bt_btif_avrc"

#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <hardware/bluetooth.h>
#if defined(MTK_LINUX_AVRCP_PLUS) && (MTK_LINUX_AVRCP_PLUS == TRUE)
#include "mtk_bt_rc.h"
#else
#include <hardware/bt_rc.h>
#endif
#include "avrc_defs.h"
#include "bta_api.h"
#include "bta_av_api.h"
#include "btif_av.h"
#include "btif_common.h"
#include "btif_util.h"
#include "bt_common.h"
#include "device/include/interop.h"
#include "uinput.h"
#include "bdaddr.h"
#include "osi/include/list.h"
#include "osi/include/properties.h"
#include "btu.h"
#if defined(MTK_STACK_CONFIG_BL) && (MTK_STACK_CONFIG_BL == TRUE)
#include "interop_mtk.h"
#endif

/*****************************************************************************
**  Constants & Macros
******************************************************************************/
#define RC_INVALID_TRACK_ID (0xFFFFFFFFFFFFFFFFULL)

/* cod value for Headsets */
#define COD_AV_HEADSETS        0x0404
/* for AVRC 1.4 need to change this */
#define MAX_RC_NOTIFICATIONS AVRC_EVT_VOLUME_CHANGE

#define IDX_GET_PLAY_STATUS_RSP   0
#define IDX_LIST_APP_ATTR_RSP     1
#define IDX_LIST_APP_VALUE_RSP    2
#define IDX_GET_CURR_APP_VAL_RSP  3
#define IDX_SET_APP_VAL_RSP       4
#define IDX_GET_APP_ATTR_TXT_RSP  5
#define IDX_GET_APP_VAL_TXT_RSP   6
#define IDX_GET_ELEMENT_ATTR_RSP  7
#if defined(MTK_AVRCP_TG_15_BROWSE)&&(MTK_AVRCP_TG_15_BROWSE == TRUE)
#define IDX_SET_ADDR_PLAYER_RSP    8
#define IDX_SET_BROWSED_PLAYER_RSP 9
#define IDX_GET_FOLDER_ITEMS_RSP   10
#define IDX_CHG_PATH_RSP           11
#define IDX_GET_ITEM_ATTR_RSP      12
#define IDX_PLAY_ITEM_RSP          13
#define IDX_GET_TOTAL_NUM_OF_ITEMS_RSP 14
#define IDX_SEARCH_RSP             15
#define IDX_ADD_TO_NOW_PLAYING_RSP 16
#define MAX_CMD_QUEUE_LEN          17
#else
#define MAX_CMD_QUEUE_LEN 8
#endif

#define MAX_VOLUME 128
#define MAX_LABEL 16
#define MAX_TRANSACTIONS_PER_SESSION 16
#define MAX_CMD_QUEUE_LEN 8
#define PLAY_STATUS_PLAYING 1

#define CHECK_RC_CONNECTED                                                                  \
    BTIF_TRACE_DEBUG("## %s ##", __FUNCTION__);                                            \
    if (btif_rc_cb.rc_connected == FALSE)                                                    \
    {                                                                                       \
        BTIF_TRACE_WARNING("Function %s() called when RC is not connected", __FUNCTION__); \
        return BT_STATUS_NOT_READY;                                                         \
    }

#if defined(MTK_AVRCP_TG_15_BROWSE)&&(MTK_AVRCP_TG_15_BROWSE == TRUE)
#define CHECK_BR_CONNECTED                                          \
    if (btif_rc_cb.br_connected == false) {               \
      BTIF_TRACE_WARNING("%s: called when BR is not connected", __func__); \
      return BT_STATUS_NOT_READY;                                          \
    }
#endif

#define FILL_PDU_QUEUE(index, ctype, label, pending)        \
{                                                           \
    btif_rc_cb.rc_pdu_info[index].ctype = ctype;            \
    btif_rc_cb.rc_pdu_info[index].label = label;            \
    btif_rc_cb.rc_pdu_info[index].is_rsp_pending = pending; \
}

#define SEND_METAMSG_RSP(index, avrc_rsp)                                                      \
{                                                                                              \
    if (btif_rc_cb.rc_pdu_info[index].is_rsp_pending == FALSE)                                  \
    {                                                                                          \
        BTIF_TRACE_WARNING("%s Not sending response as no PDU was registered", __FUNCTION__); \
        return BT_STATUS_UNHANDLED;                                                            \
    }                                                                                          \
    send_metamsg_rsp(btif_rc_cb.rc_handle, btif_rc_cb.rc_pdu_info[index].label,                \
        btif_rc_cb.rc_pdu_info[index].ctype, avrc_rsp);                                        \
    btif_rc_cb.rc_pdu_info[index].ctype = 0;                                                   \
    btif_rc_cb.rc_pdu_info[index].label = 0;                                                   \
    btif_rc_cb.rc_pdu_info[index].is_rsp_pending = FALSE;                                      \
}

/*****************************************************************************
**  Local type definitions
******************************************************************************/
typedef struct {
    UINT8 bNotify;
    UINT8 label;
} btif_rc_reg_notifications_t;

typedef struct
{
    UINT8   label;
    UINT8   ctype;
    BOOLEAN is_rsp_pending;
} btif_rc_cmd_ctxt_t;

/* 2 second timeout to get interim response */
#define BTIF_TIMEOUT_RC_INTERIM_RSP_MS     (2 * 1000)
#define BTIF_TIMEOUT_RC_STATUS_CMD_MS      (2 * 1000)
#define BTIF_TIMEOUT_RC_CONTROL_CMD_MS     (2 * 1000)


typedef enum
{
    eNOT_REGISTERED,
    eREGISTERED,
    eINTERIM
} btif_rc_nfn_reg_status_t;

typedef struct {
    UINT8                       event_id;
    UINT8                       label;
    btif_rc_nfn_reg_status_t    status;
} btif_rc_supported_event_t;

#define BTIF_RC_STS_TIMEOUT     0xFE
typedef struct {
    UINT8   label;
    UINT8   pdu_id;
} btif_rc_status_cmd_timer_t;

typedef struct {
    UINT8   label;
    UINT8   pdu_id;
} btif_rc_control_cmd_timer_t;

typedef struct {
    union {
        btif_rc_status_cmd_timer_t rc_status_cmd;
        btif_rc_control_cmd_timer_t rc_control_cmd;
    };
} btif_rc_timer_context_t;

typedef struct {
    BOOLEAN  query_started;
    UINT8 num_attrs;
    UINT8 num_ext_attrs;

    UINT8 attr_index;
    UINT8 ext_attr_index;
    UINT8 ext_val_index;
    btrc_player_app_attr_t attrs[AVRC_MAX_APP_ATTR_SIZE];
    btrc_player_app_ext_attr_t ext_attrs[AVRC_MAX_APP_ATTR_SIZE];
} btif_rc_player_app_settings_t;

/* TODO : Merge btif_rc_reg_notifications_t and btif_rc_cmd_ctxt_t to a single struct */
typedef struct {
    BOOLEAN                     rc_connected;
#if defined(MTK_AVRCP_TG_15_BROWSE)&&(MTK_AVRCP_TG_15_BROWSE == TRUE)
    BOOLEAN                     br_connected;  // Browsing channel.
#endif
    UINT8                       rc_handle;
    tBTA_AV_FEAT                rc_features;
#if defined(MTK_A2DP_SRC_SINK_BOTH) && (MTK_A2DP_SRC_SINK_BOTH == TRUE)
    tBTA_AV_FEAT                peer_ct_features;
    tBTA_AV_FEAT                peer_tg_features;
    BOOLEAN                     launch_cmd_pending; /* TRUE: getcap/regvolume */
#endif
    BD_ADDR                     rc_addr;
    UINT16                      rc_pending_play;
    btif_rc_cmd_ctxt_t          rc_pdu_info[MAX_CMD_QUEUE_LEN];
    btif_rc_reg_notifications_t rc_notif[MAX_RC_NOTIFICATIONS];
    unsigned int                rc_volume;
    uint8_t                     rc_vol_label;
    list_t                      *rc_supported_event_list;
    btif_rc_player_app_settings_t   rc_app_settings;
    alarm_t                     *rc_play_status_timer;
#if defined(MTK_LINUX) && defined(MTK_COMMON) && (MTK_COMMON == TRUE)
    tAVRC_PLAYSTATE             rc_play_status;
#endif
    BOOLEAN                     rc_features_processed;
    UINT64                      rc_playing_uid;
    BOOLEAN                     rc_procedure_complete;
#if defined(MTK_STACK_CONFIG_BL) && (MTK_STACK_CONFIG_BL == TRUE)
    BOOLEAN                     rc_disalbe_song_pos;
    uint8_t                     rc_notif_virtual[MAX_RC_NOTIFICATIONS];
#endif
} btif_rc_cb_t;

typedef struct {
    BOOLEAN in_use;
    UINT8 lbl;
    UINT8 handle;
    btif_rc_timer_context_t txn_timer_context;
    alarm_t *txn_timer;
} rc_transaction_t;

typedef struct
{
    pthread_mutex_t lbllock;
    rc_transaction_t transaction[MAX_TRANSACTIONS_PER_SESSION];
} rc_device_t;


#if defined(MTK_STACK_CONFIG_BL) && (MTK_STACK_CONFIG_BL == TRUE)
typedef struct {
    int pressed;
    UINT8 key;
} btif_rc_key_status_t;
static btif_rc_key_status_t prestatus = {-1, 0};
#endif


rc_device_t device;

#define MAX_UINPUT_PATHS 3
static const char* uinput_dev_path[] =
                       {"/dev/uinput", "/dev/input/uinput", "/dev/misc/uinput" };
static int uinput_fd = -1;

#if defined(MTK_LINUX_AVRCP) && (MTK_LINUX_AVRCP == TRUE)
static void btif_send_get_playstatus_cmd_ex(void);
const btrc_ctrl_ex_interface_t *btif_rc_ex_get_interface(void);
const btrc_ex_interface_t *btif_rc_tg_ex_get_interface(void);
#endif

static int  send_event (int fd, uint16_t type, uint16_t code, int32_t value);
static void send_key (int fd, uint16_t key, int pressed);
static int  uinput_driver_check();
static int  uinput_create(char *name);
static int  init_uinput (void);
static void close_uinput (void);
static void sleep_ms(period_ms_t timeout_ms);

#if defined(MTK_AVRCP_TG_15_BROWSE)&&(MTK_AVRCP_TG_15_BROWSE == TRUE)
/* Response status code - Unknown Error - this is changed to "reserved" */
#define BTIF_STS_GEN_ERROR 0x06

static bt_status_t list_player_app_attr_rsp( int num_attr, btrc_player_attr_t *p_attrs);
static bt_status_t list_player_app_value_rsp( int num_val, uint8_t *value);
static bt_status_t get_player_app_value_rsp(btrc_player_settings_t *p_vals);
static bt_status_t get_player_app_attr_text_rsp(int num_attr, btrc_player_setting_text_t *p_attrs);
static bt_status_t get_player_app_value_text_rsp(int num_attr, btrc_player_setting_text_t *p_attrs);
static bt_status_t set_player_app_value_rsp (btrc_status_t rsp_status);

#if defined(MTK_AV_VIRTUAL_BR_HAL_CB)&&(MTK_AV_VIRTUAL_BR_HAL_CB == TRUE)
static bt_status_t set_addressed_player_rsp(btrc_status_t rsp_status);
static bt_status_t register_notification_rsp(btrc_event_id_t event_id,
                                                btrc_notification_type_t type,
                                                btrc_register_notification_t *p_param);
static bt_status_t get_folder_items_list_rsp(btrc_status_t rsp_status,
                                             uint16_t uid_counter,
                                             uint8_t num_items,
                                             btrc_folder_items_t* p_items);

static bt_status_t get_total_num_of_items_rsp(btrc_status_t rsp_status,
                                              uint32_t uid_counter,
                                              uint32_t num_items);

#endif //(end -- MTK_AV_VIRTUAL_BR_HAL_CB)

/* Utility table to map hal status codes to bta status codes for the response
 * status */
static const uint8_t status_code_map[] = {
    /* BTA_Status codes        HAL_Status codes */
    AVRC_STS_BAD_CMD,         /* BTRC_STS_BAD_CMD */
    AVRC_STS_BAD_PARAM,       /* BTRC_STS_BAD_PARAM */
    AVRC_STS_NOT_FOUND,       /* BTRC_STS_NOT_FOUND */
    AVRC_STS_INTERNAL_ERR,    /* BTRC_STS_INTERNAL_ERR */
    AVRC_STS_NO_ERROR,        /* BTRC_STS_NO_ERROR */
    AVRC_STS_UID_CHANGED,     /* BTRC_STS_UID_CHANGED */
    BTIF_STS_GEN_ERROR,       /* BTRC_STS_RESERVED */
    AVRC_STS_BAD_DIR,         /* BTRC_STS_INV_DIRN */
    AVRC_STS_NOT_DIR,         /* BTRC_STS_INV_DIRECTORY */
    AVRC_STS_NOT_EXIST,       /* BTRC_STS_INV_ITEM */
    AVRC_STS_BAD_SCOPE,       /* BTRC_STS_INV_SCOPE */
    AVRC_STS_BAD_RANGE,       /* BTRC_STS_INV_RANGE */
    AVRC_STS_UID_IS_DIR,      /* BTRC_STS_DIRECTORY */
    AVRC_STS_IN_USE,          /* BTRC_STS_MEDIA_IN_USE */
    AVRC_STS_NOW_LIST_FULL,   /* BTRC_STS_PLAY_LIST_FULL */
    AVRC_STS_SEARCH_NOT_SUP,  /* BTRC_STS_SRCH_NOT_SPRTD */
    AVRC_STS_SEARCH_BUSY,     /* BTRC_STS_SRCH_IN_PROG */
    AVRC_STS_BAD_PLAYER_ID,   /* BTRC_STS_INV_PLAYER */
    AVRC_STS_PLAYER_N_BR,     /* BTRC_STS_PLAY_NOT_BROW */
    AVRC_STS_PLAYER_N_ADDR,   /* BTRC_STS_PLAY_NOT_ADDR */
    AVRC_STS_BAD_SEARCH_RES,  /* BTRC_STS_INV_RESULTS */
    AVRC_STS_NO_AVAL_PLAYER,  /* BTRC_STS_NO_AVBL_PLAY */
    AVRC_STS_ADDR_PLAYER_CHG, /* BTRC_STS_ADDR_PLAY_CHGD */
};
#endif //(end -- #if defined(MTK_AVRCP_TG_15_BROWSE))

static const struct {
    const char *name;
    uint8_t avrcp;
    uint16_t mapped_id;
    uint8_t release_quirk;
} key_map[] = {
    { "PLAY",         AVRC_ID_PLAY,     KEY_PLAYCD,       1 },
#if defined(MTK_COMMON) && (MTK_COMMON == TRUE)
    { "STOP",         AVRC_ID_STOP,     KEY_STOPCD,       1 },
#else
    { "STOP",         AVRC_ID_STOP,     KEY_STOPCD,       0 },
#endif
    { "PAUSE",        AVRC_ID_PAUSE,    KEY_PAUSECD,      1 },
    { "FORWARD",      AVRC_ID_FORWARD,  KEY_NEXTSONG,     0 },
    { "BACKWARD",     AVRC_ID_BACKWARD, KEY_PREVIOUSSONG, 0 },
    { "REWIND",       AVRC_ID_REWIND,   KEY_REWIND,       0 },
    { "FAST FORWARD", AVRC_ID_FAST_FOR, KEY_FAST_FORWARD, 0 },
    { NULL,           0,                0,                0 }
};

static void send_reject_response (UINT8 rc_handle, UINT8 label,
    UINT8 pdu, UINT8 status);
static UINT8 opcode_from_pdu(UINT8 pdu);
static void send_metamsg_rsp (UINT8 rc_handle, UINT8 label,
    tBTA_AV_CODE code, tAVRC_RESPONSE *pmetamsg_resp);
#if (AVRC_ADV_CTRL_INCLUDED == TRUE)
static void register_volumechange(UINT8 label);
#endif
static void lbl_init();
static void lbl_destroy();
static void init_all_transactions();
static bt_status_t  get_transaction(rc_transaction_t **ptransaction);
static void release_transaction(UINT8 label);
static rc_transaction_t* get_transaction_by_lbl(UINT8 label);
#if (AVRC_ADV_CTRL_INCLUDED == TRUE)
static void handle_rc_metamsg_rsp(tBTA_AV_META_MSG *pmeta_msg);
#endif
#if (AVRC_CTLR_INCLUDED == TRUE)
static void handle_avk_rc_metamsg_cmd(tBTA_AV_META_MSG *pmeta_msg);
static void handle_avk_rc_metamsg_rsp(tBTA_AV_META_MSG *pmeta_msg);
static void btif_rc_ctrl_upstreams_rsp_cmd(
    UINT8 event, tAVRC_COMMAND *pavrc_cmd, UINT8 label);
static void rc_ctrl_procedure_complete();
static void rc_stop_play_status_timer();
static void register_for_event_notification (btif_rc_supported_event_t *p_event);
static void handle_get_capability_response (tBTA_AV_META_MSG *pmeta_msg, tAVRC_GET_CAPS_RSP *p_rsp);
static void handle_app_attr_response (tBTA_AV_META_MSG *pmeta_msg, tAVRC_LIST_APP_ATTR_RSP *p_rsp);
static void handle_app_val_response (tBTA_AV_META_MSG *pmeta_msg, tAVRC_LIST_APP_VALUES_RSP *p_rsp);
static void handle_app_cur_val_response (tBTA_AV_META_MSG *pmeta_msg, tAVRC_GET_CUR_APP_VALUE_RSP *p_rsp);
static void handle_app_attr_txt_response (tBTA_AV_META_MSG *pmeta_msg, tAVRC_GET_APP_ATTR_TXT_RSP *p_rsp);
static void handle_app_attr_val_txt_response (tBTA_AV_META_MSG *pmeta_msg, tAVRC_GET_APP_ATTR_TXT_RSP *p_rsp);
static void handle_get_playstatus_response (tBTA_AV_META_MSG *pmeta_msg, tAVRC_GET_PLAY_STATUS_RSP *p_rsp);
static void handle_get_elem_attr_response (tBTA_AV_META_MSG *pmeta_msg, tAVRC_GET_ELEM_ATTRS_RSP *p_rsp);
static void handle_set_app_attr_val_response (tBTA_AV_META_MSG *pmeta_msg, tAVRC_RSP *p_rsp);
static bt_status_t get_play_status_cmd(void);
static bt_status_t get_player_app_setting_attr_text_cmd (UINT8 *attrs, UINT8 num_attrs);
static bt_status_t get_player_app_setting_value_text_cmd (UINT8 *vals, UINT8 num_vals);
static bt_status_t register_notification_cmd (UINT8 label, UINT8 event_id, UINT32 event_value);
static bt_status_t get_element_attribute_cmd (uint8_t num_attribute, uint32_t *p_attr_ids);
static bt_status_t getcapabilities_cmd (uint8_t cap_id);
static bt_status_t list_player_app_setting_attrib_cmd(void);
static bt_status_t list_player_app_setting_value_cmd(uint8_t attrib_id);
static bt_status_t get_player_app_setting_cmd(uint8_t num_attrib, uint8_t* attrib_ids);
#if defined(MTK_AVRCP_TG_15_BROWSE)&&(MTK_AVRCP_TG_15_BROWSE == TRUE)
static uint8_t fill_attribute_id_array(
    uint8_t cmd_attribute_number, btrc_media_attr_t* cmd_attribute_id_array,
    size_t out_array_size, btrc_media_attr_t* out_attribute_id_array);
static tBTA_AV_CODE get_rsp_type_code(tAVRC_STS status, tBTA_AV_CODE code) ;
static void handle_set_addressed_player_response(tBTA_AV_META_MSG* pmeta_msg,
                                                 tAVRC_RSP* p_rsp);
static bt_status_t get_folder_items_cmd(bt_bdaddr_t* bd_addr, uint8_t scope,
                                        uint8_t start_item, uint8_t num_items);

void fill_avrc_attr_entry(tAVRC_ATTR_ENTRY* attr_vals, int num_attrs,
                          btrc_element_attr_val_t* p_attrs);
bt_status_t build_and_send_vendor_cmd(tAVRC_COMMAND* avrc_cmd,
                                      tBTA_AV_CODE cmd_code);
void get_folder_item_type_media(const tAVRC_ITEM* avrc_item,
                                btrc_folder_items_t* btrc_item);
void get_folder_item_type_folder(const tAVRC_ITEM* avrc_item,
                                 btrc_folder_items_t* btrc_item);
void get_folder_item_type_player(const tAVRC_ITEM* avrc_item,
                                 btrc_folder_items_t* btrc_item);
#endif //(end - #if defined(MTK_AVRCP_TG_15_BROWSE))
#endif
static void btif_rc_upstreams_evt(UINT16 event, tAVRC_COMMAND* p_param, UINT8 ctype, UINT8 label);
#if (AVRC_ADV_CTRL_INCLUDED == TRUE)
static void btif_rc_upstreams_rsp_evt(UINT16 event, tAVRC_RESPONSE *pavrc_resp, UINT8 ctype, UINT8 label);
#endif
static void rc_start_play_status_timer(void);
static bool absolute_volume_disabled(void);
static char const* key_id_to_str(uint16_t id);

/*****************************************************************************
**  Static variables
******************************************************************************/
static btif_rc_cb_t btif_rc_cb;
static btrc_callbacks_t *bt_rc_callbacks = NULL;
static btrc_ctrl_callbacks_t *bt_rc_ctrl_callbacks = NULL;
#if defined(MTK_LINUX_AVRCP_PLUS) && (MTK_LINUX_AVRCP_PLUS == TRUE)
static btrc_ctrl_ext_callbacks_t *bt_rc_ctrl_ext_callbacks = NULL;
static btrc_ext_callbacks_t *bt_rc_ext_callbacks = NULL;
#endif

/*****************************************************************************
**  Static functions
******************************************************************************/

/*****************************************************************************
**  Externs
******************************************************************************/
extern BOOLEAN btif_hf_call_terminated_recently();
extern BOOLEAN check_cod(const bt_bdaddr_t *remote_bdaddr, uint32_t cod);

extern fixed_queue_t *btu_general_alarm_queue;
/*****************************************************************************
**  Functions
******************************************************************************/

/*****************************************************************************
**   Local uinput helper functions
******************************************************************************/
int send_event (int fd, uint16_t type, uint16_t code, int32_t value)
{
    struct uinput_event event;
    BTIF_TRACE_DEBUG("%s type:%u code:%u value:%d", __FUNCTION__,
        type, code, value);
    memset(&event, 0, sizeof(event));
    event.type  = type;
    event.code  = code;
    event.value = value;

    ssize_t ret;
    OSI_NO_INTR(ret = write(fd, &event, sizeof(event)));
    return (int)ret;
}

void send_key (int fd, uint16_t key, int pressed)
{
    BTIF_TRACE_DEBUG("%s fd:%d key:%u pressed:%d", __FUNCTION__,
        fd, key, pressed);

    if (fd < 0)
    {
        return;
    }

    LOG_INFO(LOG_TAG, "AVRCP: Send key %s (%d) fd=%d", key_id_to_str(key), pressed, fd);
    send_event(fd, EV_KEY, key, pressed);
    send_event(fd, EV_SYN, SYN_REPORT, 0);
}

/************** uinput related functions **************/
int uinput_driver_check()
{
    uint32_t i;
    for (i=0; i < MAX_UINPUT_PATHS; i++)
    {
        if (access(uinput_dev_path[i], O_RDWR) == 0) {
           return 0;
        }
    }
    BTIF_TRACE_ERROR("%s ERROR: uinput device is not in the system", __FUNCTION__);
    return -1;
}

int uinput_create(char *name)
{
    struct uinput_dev dev;
    int fd, x = 0;

    for(x=0; x < MAX_UINPUT_PATHS; x++)
    {
        fd = open(uinput_dev_path[x], O_RDWR);
        if (fd < 0)
            continue;
        break;
    }
    if (x == MAX_UINPUT_PATHS) {
        BTIF_TRACE_ERROR("%s ERROR: uinput device open failed", __FUNCTION__);
        return -1;
    }
    memset(&dev, 0, sizeof(dev));
    if (name)
        strncpy(dev.name, name, UINPUT_MAX_NAME_SIZE-1);

    dev.id.bustype = BUS_BLUETOOTH;
    dev.id.vendor  = 0x0000;
    dev.id.product = 0x0000;
    dev.id.version = 0x0000;

    ssize_t ret;
    OSI_NO_INTR(ret = write(fd, &dev, sizeof(dev)));
    if (ret < 0) {
        BTIF_TRACE_ERROR("%s Unable to write device information", __FUNCTION__);
        close(fd);
        return -1;
    }

    ioctl(fd, UI_SET_EVBIT, EV_KEY);
    ioctl(fd, UI_SET_EVBIT, EV_REL);
    ioctl(fd, UI_SET_EVBIT, EV_SYN);

    for (x = 0; key_map[x].name != NULL; x++)
        ioctl(fd, UI_SET_KEYBIT, key_map[x].mapped_id);

    if (ioctl(fd, UI_DEV_CREATE, NULL) < 0) {
        BTIF_TRACE_ERROR("%s Unable to create uinput device", __FUNCTION__);
        close(fd);
        return -1;
    }
    return fd;
}

int init_uinput (void)
{
    char *name = "AVRCP";

    BTIF_TRACE_DEBUG("%s", __FUNCTION__);
    uinput_fd = uinput_create(name);
    if (uinput_fd < 0) {
        BTIF_TRACE_ERROR("%s AVRCP: Failed to initialize uinput for %s (%d)",
                          __FUNCTION__, name, uinput_fd);
    } else {
        BTIF_TRACE_DEBUG("%s AVRCP: Initialized uinput for %s (fd=%d)",
                          __FUNCTION__, name, uinput_fd);
    }
    return uinput_fd;
}

void close_uinput (void)
{
    BTIF_TRACE_DEBUG("%s", __FUNCTION__);
    if (uinput_fd > 0) {
        ioctl(uinput_fd, UI_DEV_DESTROY);

        close(uinput_fd);
        uinput_fd = -1;
    }
}

#if (AVRC_CTLR_INCLUDED == TRUE)
void rc_cleanup_sent_cmd (void *p_data)
{
    BTIF_TRACE_DEBUG("%s", __FUNCTION__);

}
#if defined(MTK_A2DP_SRC_SINK_BOTH) && (MTK_A2DP_SRC_SINK_BOTH == TRUE)
BOOLEAN btif_rc_both_enable(void)
{
    return (bt_rc_ctrl_callbacks != NULL && bt_rc_callbacks != NULL);
}

static void handle_rc_ctrl_features_all(BD_ADDR bd_addr)
{
    btrc_remote_features_t peer_tg_feature = BTRC_FEAT_NONE;

    peer_tg_feature = btif_rc_cb.peer_tg_features;

    BTIF_TRACE_DEBUG("%s peer_tg_feature 0x%x", __FUNCTION__, peer_tg_feature);

    if ((peer_tg_feature & BTA_AV_FEAT_RCTG)||
       ((peer_tg_feature & BTA_AV_FEAT_RCCT)&&
        (peer_tg_feature & BTA_AV_FEAT_ADV_CTRL)))
    {
        bt_bdaddr_t rc_addr;
        int rc_features = 0;
        bdcpy(rc_addr.address,bd_addr);

        if ((peer_tg_feature & BTA_AV_FEAT_ADV_CTRL)&&
             (peer_tg_feature & BTA_AV_FEAT_RCCT))
        {
            rc_features |= BTRC_FEAT_ABSOLUTE_VOLUME;
        }
        if ((peer_tg_feature & BTA_AV_FEAT_METADATA)&&
            (peer_tg_feature & BTA_AV_FEAT_VENDOR)&&
            (btif_rc_cb.rc_features_processed != TRUE))
        {
            rc_features |= BTRC_FEAT_METADATA;
            /* Mark rc features processed to avoid repeating
             * the AVRCP procedure every time on receiving this
             * update.
             */
            btif_rc_cb.rc_features_processed = TRUE;
            if (btif_av_is_connected())
            {
                if (btif_av_peer_is_src())
                {
                    btif_rc_cb.rc_features = peer_tg_feature;
                    getcapabilities_cmd (AVRC_CAP_COMPANY_ID);
                }
            }
            else
            {
                BTIF_TRACE_DEBUG("%s launch_cmd_pending", __FUNCTION__);
                btif_rc_cb.launch_cmd_pending = TRUE;
            }
        }
        BTIF_TRACE_DEBUG("%s Update rc features to CTRL %d", __FUNCTION__, rc_features);
        HAL_CBACK(bt_rc_ctrl_callbacks, getrcfeatures_cb, &rc_addr, rc_features);
    }
}
#endif

void handle_rc_ctrl_features(BD_ADDR bd_addr)
{
#if defined(MTK_A2DP_SRC_SINK_BOTH) && (MTK_A2DP_SRC_SINK_BOTH == TRUE)
    if (btif_rc_both_enable())
    {
        return handle_rc_ctrl_features_all(bd_addr);
    }
#endif

    if ((btif_rc_cb.rc_features & BTA_AV_FEAT_RCTG)||
       ((btif_rc_cb.rc_features & BTA_AV_FEAT_RCCT)&&
        (btif_rc_cb.rc_features & BTA_AV_FEAT_ADV_CTRL)))
    {
        bt_bdaddr_t rc_addr;
        int rc_features = 0;
        bdcpy(rc_addr.address,bd_addr);

        if ((btif_rc_cb.rc_features & BTA_AV_FEAT_ADV_CTRL)&&
             (btif_rc_cb.rc_features & BTA_AV_FEAT_RCCT))
        {
            rc_features |= BTRC_FEAT_ABSOLUTE_VOLUME;
        }
        if ((btif_rc_cb.rc_features & BTA_AV_FEAT_METADATA)&&
            (btif_rc_cb.rc_features & BTA_AV_FEAT_VENDOR)&&
            (btif_rc_cb.rc_features_processed != TRUE))
        {
            rc_features |= BTRC_FEAT_METADATA;
            /* Mark rc features processed to avoid repeating
             * the AVRCP procedure every time on receiving this
             * update.
             */
            btif_rc_cb.rc_features_processed = TRUE;

            if (btif_av_is_sink_enabled())
                getcapabilities_cmd (AVRC_CAP_COMPANY_ID);
        }
#if defined(MTK_AVRCP_TG_15_BROWSE)&&(MTK_AVRCP_TG_15_BROWSE == TRUE)
        /* Add browsing feature capability */
        if (btif_rc_cb.rc_features & BTA_AV_FEAT_BROWSE) {
           rc_features |= BTRC_FEAT_BROWSE;
        }
#endif

        BTIF_TRACE_DEBUG("%s Update rc features to CTRL %d", __FUNCTION__, rc_features);
        HAL_CBACK(bt_rc_ctrl_callbacks, getrcfeatures_cb, &rc_addr, rc_features);
    }
}
#endif

#if defined(MTK_A2DP_SRC_SINK_BOTH) && (MTK_A2DP_SRC_SINK_BOTH == TRUE)
static void btif_reg_abs_volume_event(void)
{
    rc_transaction_t *p_transaction=NULL;
    bt_status_t status = BT_STATUS_NOT_READY;
    if (MAX_LABEL==btif_rc_cb.rc_vol_label)
    {
       status=get_transaction(&p_transaction);
    }
    else
    {
       p_transaction=get_transaction_by_lbl(btif_rc_cb.rc_vol_label);
       if (NULL!=p_transaction)
       {
          BTIF_TRACE_DEBUG("%s register_volumechange already in progress for label %d",
                             __FUNCTION__, btif_rc_cb.rc_vol_label);
          return;
       }
       else
         status=get_transaction(&p_transaction);
    }

    if (BT_STATUS_SUCCESS == status && NULL!=p_transaction)
    {
       btif_rc_cb.rc_vol_label=p_transaction->lbl;
       register_volumechange(btif_rc_cb.rc_vol_label);
    }
}

static void handle_rc_features_all(BD_ADDR bd_addr)
{
    btrc_remote_features_t rc_features = BTRC_FEAT_NONE;
    btrc_remote_features_t *peer_ct_feature = NULL;
    bt_bdaddr_t rc_addr;

    bdcpy(rc_addr.address, btif_rc_cb.rc_addr);
    bt_bdaddr_t avdtp_addr  = btif_av_get_addr();

    bdstr_t addr1, addr2;
    BTIF_TRACE_DEBUG("%s: AVDTP Address: %s AVCTP address: %s", __func__,
                     bdaddr_to_string(&avdtp_addr, addr1, sizeof(addr1)),
                     bdaddr_to_string(&rc_addr, addr2, sizeof(addr2)));

    peer_ct_feature = &btif_rc_cb.peer_ct_features;

    if (0 == *peer_ct_feature)
    {
        return;
    }

    if (interop_match_addr(INTEROP_DISABLE_ABSOLUTE_VOLUME, &rc_addr)
        || absolute_volume_disabled()
        || bdcmp(avdtp_addr.address, rc_addr.address))
        *peer_ct_feature &= ~BTA_AV_FEAT_ADV_CTRL;

    if (*peer_ct_feature & BTA_AV_FEAT_BROWSE)
    {
        rc_features |= BTRC_FEAT_BROWSE;
    }

#if (AVRC_ADV_CTRL_INCLUDED == TRUE)
    if ( (*peer_ct_feature & BTA_AV_FEAT_ADV_CTRL) &&
         (*peer_ct_feature & BTA_AV_FEAT_RCTG))
    {
        rc_features |= BTRC_FEAT_ABSOLUTE_VOLUME;
    }
#endif

    if (*peer_ct_feature & BTA_AV_FEAT_METADATA)
    {
        rc_features |= BTRC_FEAT_METADATA;
    }

    BTIF_TRACE_DEBUG("%s: rc_features=0x%x, peer_ct_feature=0x%x",
        __FUNCTION__, rc_features, *peer_ct_feature);
    HAL_CBACK(bt_rc_callbacks, remote_features_cb, &rc_addr, rc_features)

#if (AVRC_ADV_CTRL_INCLUDED == TRUE)
     BTIF_TRACE_DEBUG("%s Checking for feature flags in btif_rc_handler with label %d",
                        __FUNCTION__, btif_rc_cb.rc_vol_label);
    // Register for volume change on connect
    if ((*peer_ct_feature & BTA_AV_FEAT_ADV_CTRL) &&
        (*peer_ct_feature & BTA_AV_FEAT_RCTG))
    {
        /* if local ct(sink) is enable,
         * should wait a2dp connected, then send this cmd
         */
        if (btif_av_is_connected())
        {
            if (!btif_av_peer_is_src())
            {
                btif_rc_cb.rc_features = *peer_ct_feature;
                btif_reg_abs_volume_event();
            }
        }
        else
        {
            BTIF_TRACE_DEBUG("%s launch_cmd_pending", __FUNCTION__);
            btif_rc_cb.launch_cmd_pending = TRUE;
        }
    }
#endif
}
#endif

void handle_rc_features(BD_ADDR bd_addr)
{
#if defined(MTK_A2DP_SRC_SINK_BOTH) && (MTK_A2DP_SRC_SINK_BOTH == TRUE)
    if (btif_rc_both_enable())
    {
        return handle_rc_features_all(bd_addr);
    }
#endif

    if (bt_rc_callbacks != NULL)
    {
    btrc_remote_features_t rc_features = BTRC_FEAT_NONE;
    bt_bdaddr_t rc_addr;

    bdcpy(rc_addr.address, btif_rc_cb.rc_addr);
    bt_bdaddr_t avdtp_addr  = btif_av_get_addr();

    bdstr_t addr1, addr2;
    BTIF_TRACE_DEBUG("%s: AVDTP Address: %s AVCTP address: %s", __func__,
                     bdaddr_to_string(&avdtp_addr, addr1, sizeof(addr1)),
                     bdaddr_to_string(&rc_addr, addr2, sizeof(addr2)));

    if (interop_match_addr(INTEROP_DISABLE_ABSOLUTE_VOLUME, &rc_addr)
        || absolute_volume_disabled()
        || bdcmp(avdtp_addr.address, rc_addr.address))
        btif_rc_cb.rc_features &= ~BTA_AV_FEAT_ADV_CTRL;

    if (btif_rc_cb.rc_features & BTA_AV_FEAT_BROWSE)
    {
        rc_features |= BTRC_FEAT_BROWSE;
    }

#if (AVRC_ADV_CTRL_INCLUDED == TRUE)
    if ( (btif_rc_cb.rc_features & BTA_AV_FEAT_ADV_CTRL) &&
         (btif_rc_cb.rc_features & BTA_AV_FEAT_RCTG))
    {
        rc_features |= BTRC_FEAT_ABSOLUTE_VOLUME;
    }
#endif

    if (btif_rc_cb.rc_features & BTA_AV_FEAT_METADATA)
    {
        rc_features |= BTRC_FEAT_METADATA;
    }

    BTIF_TRACE_DEBUG("%s: rc_features=0x%x", __FUNCTION__, rc_features);
    HAL_CBACK(bt_rc_callbacks, remote_features_cb, &rc_addr, rc_features)

#if (AVRC_ADV_CTRL_INCLUDED == TRUE)
     BTIF_TRACE_DEBUG("%s Checking for feature flags in btif_rc_handler with label %d",
                        __FUNCTION__, btif_rc_cb.rc_vol_label);
     // Register for volume change on connect
      if (btif_rc_cb.rc_features & BTA_AV_FEAT_ADV_CTRL &&
         btif_rc_cb.rc_features & BTA_AV_FEAT_RCTG)
      {
         rc_transaction_t *p_transaction=NULL;
         bt_status_t status = BT_STATUS_NOT_READY;
         if (MAX_LABEL==btif_rc_cb.rc_vol_label)
         {
            status=get_transaction(&p_transaction);
         }
         else
         {
            p_transaction=get_transaction_by_lbl(btif_rc_cb.rc_vol_label);
            if (NULL!=p_transaction)
            {
               BTIF_TRACE_DEBUG("%s register_volumechange already in progress for label %d",
                                  __FUNCTION__, btif_rc_cb.rc_vol_label);
               return;
            }
            else
              status=get_transaction(&p_transaction);
         }

         if (BT_STATUS_SUCCESS == status && NULL!=p_transaction)
         {
            btif_rc_cb.rc_vol_label=p_transaction->lbl;
            register_volumechange(btif_rc_cb.rc_vol_label);
         }
       }
#endif
    }
}

#if defined(MTK_AVRCP_TG_15_BROWSE)&&(MTK_AVRCP_TG_15_BROWSE == TRUE)
/***************************************************************************
 *  Function       handle_rc_connect
 *
 *  - Argument:    tBTA_AV_RC_OPEN  browse RC open data structure
 *
 *  - Description: browse RC connection event handler
 *
 ***************************************************************************/
void handle_rc_browse_connect(tBTA_AV_RC_BROWSE_OPEN* p_rc_br_open) {
  BTIF_TRACE_DEBUG("%s: rc_handle %d status %d", __func__,
                   p_rc_br_open->rc_handle, p_rc_br_open->status);

  /* check that we are already connected to this address since being connected
   * to a browse when not connected to the control channel over AVRCP is
   * probably not preferred anyways. */
  if (p_rc_br_open->status == BTA_AV_SUCCESS) {
    bt_bdaddr_t rc_addr;
    bdcpy(rc_addr.address, btif_rc_cb.rc_addr);
    btif_rc_cb.br_connected = true;
#if defined(MTK_AV_SUPPORT_BR_HAL_CB)&&(MTK_AV_SUPPORT_BR_HAL_CB == TRUE)\
    && !(defined MTK_BT_AVRCP_TG_15_BW_NO_CONN_CB)
    HAL_CBACK(bt_rc_ctrl_callbacks, connection_state_cb, TRUE, TRUE, &rc_addr);
#endif
  }
}
#endif

/***************************************************************************
 *  Function       handle_rc_connect
 *
 *  - Argument:    tBTA_AV_RC_OPEN  RC open data structure
 *
 *  - Description: RC connection event handler
 *
 ***************************************************************************/
void handle_rc_connect (tBTA_AV_RC_OPEN *p_rc_open)
{
    BTIF_TRACE_DEBUG("%s: rc_handle: %d", __FUNCTION__, p_rc_open->rc_handle);
    bt_status_t result = BT_STATUS_SUCCESS;
#if (AVRC_CTLR_INCLUDED == TRUE)
    bt_bdaddr_t rc_addr;
#endif

    if (p_rc_open->status == BTA_AV_SUCCESS)
    {
        //check if already some RC is connected
        if (btif_rc_cb.rc_connected)
        {
            BTIF_TRACE_ERROR("%s Got RC OPEN in connected state, Connected RC: %d \
                and Current RC: %d", __FUNCTION__, btif_rc_cb.rc_handle,p_rc_open->rc_handle );
            if ((btif_rc_cb.rc_handle != p_rc_open->rc_handle)
                && (bdcmp(btif_rc_cb.rc_addr, p_rc_open->peer_addr)))
            {
                BTIF_TRACE_DEBUG("%s Got RC connected for some other handle", __FUNCTION__);
                BTA_AvCloseRc(p_rc_open->rc_handle);
                return;
            }
        }

        memcpy(btif_rc_cb.rc_addr, p_rc_open->peer_addr, sizeof(BD_ADDR));
        btif_rc_cb.rc_features = p_rc_open->peer_features;
#if defined(MTK_A2DP_SRC_SINK_BOTH) && (MTK_A2DP_SRC_SINK_BOTH == TRUE)
        btif_rc_cb.peer_ct_features = p_rc_open->peer_ct_features;
        btif_rc_cb.peer_tg_features = p_rc_open->peer_tg_features;
#endif
        btif_rc_cb.rc_vol_label=MAX_LABEL;
        btif_rc_cb.rc_volume=MAX_VOLUME;

        btif_rc_cb.rc_connected = TRUE;
        btif_rc_cb.rc_handle = p_rc_open->rc_handle;

#if defined(MTK_STACK_CONFIG_BL) && (MTK_STACK_CONFIG_BL == TRUE)
        LOG_DEBUG(LOG_TAG,"%s :FF/REW prestatus initialize ", __FUNCTION__);
        prestatus.key = 0;
        prestatus.pressed = -1;
#endif

#if defined(MTK_LINUX_AVRCP_PLUS) && (MTK_LINUX_AVRCP_PLUS == TRUE)
        /* new list when avrcp connected avoid crash if register_notification_rsp before handle_get_capability_response */
        btif_rc_cb.rc_supported_event_list = list_new(osi_free);
#endif

#if !defined(MTK_A2DP_SRC_SINK_BOTH) || (MTK_A2DP_SRC_SINK_BOTH == FALSE)
        /* on locally initiated connection we will get remote features as part of connect */
#if defined(MTK_AVRCP_TG_15_BROWSE)&&(MTK_AVRCP_TG_15_BROWSE == TRUE)
        if (btif_rc_cb.rc_features != 0 && bt_rc_callbacks != NULL)
#else
        if (btif_rc_cb.rc_features != 0)
#endif
            handle_rc_features(btif_rc_cb.rc_addr);
#endif
        if (bt_rc_callbacks)
        {
            result = uinput_driver_check();
            if (result == BT_STATUS_SUCCESS)
            {
                init_uinput();
            }
        }
        else
        {
            BTIF_TRACE_WARNING("%s Avrcp TG role not enabled, not initializing UInput",
                               __FUNCTION__);
        }
#if defined(MTK_A2DP_SRC_SINK_BOTH) && (MTK_A2DP_SRC_SINK_BOTH == TRUE)
        BTIF_TRACE_DEBUG("%s ct_feature=0x%x, tg_feature=0x%x ",
            __FUNCTION__,
            btif_rc_cb.peer_ct_features,
            btif_rc_cb.peer_tg_features);
#endif
        BTIF_TRACE_DEBUG("%s handle_rc_connect features %d ",__FUNCTION__, btif_rc_cb.rc_features);

#if defined(MTK_STACK_CONFIG_BL) && (MTK_STACK_CONFIG_BL == TRUE)
        memset(&btif_rc_cb.rc_notif_virtual, 0, sizeof(uint8_t) * MAX_RC_NOTIFICATIONS);
        for (int i = 0; key_map[i].name != NULL; i++)
        {
            btif_rc_cb.rc_notif_virtual[i] = key_map[i].release_quirk;
        }
        if (interop_mtk_match_addr_name(INTEROP_MTK_AVRCP_DO_RELEASE_KEY, (const bt_bdaddr_t *)&btif_rc_cb.rc_addr))
        {
            for (int i = 0; key_map[i].name != NULL; i++)
            {
                switch(key_map[i].avrcp)
                {
                    case AVRC_ID_PLAY:
                    case AVRC_ID_STOP:
                    case AVRC_ID_PAUSE:
                    case AVRC_ID_FORWARD:
                    case AVRC_ID_REWIND:
                        btif_rc_cb.rc_notif_virtual[i] = 1;
                }
            }
        }
#endif

#if (AVRC_CTLR_INCLUDED == TRUE)
        btif_rc_cb.rc_playing_uid = RC_INVALID_TRACK_ID;
        bdcpy(rc_addr.address, btif_rc_cb.rc_addr);
        if (bt_rc_ctrl_callbacks != NULL)
        {
#if defined MTK_BT_AVRCP_TG_15_BW && !(defined MTK_BT_AVRCP_TG_15_BW_NO_CONN_CB)
            HAL_CBACK(bt_rc_ctrl_callbacks, connection_state_cb, TRUE, FALSE, &rc_addr);
#else
            HAL_CBACK(bt_rc_ctrl_callbacks, connection_state_cb, TRUE, &rc_addr);
#endif
        }
#if defined(MTK_LINUX_AVRCP_PLUS) && (MTK_LINUX_AVRCP_PLUS == TRUE)
        else if (bt_rc_ext_callbacks != NULL)
       {
            HAL_CBACK(bt_rc_ext_callbacks, connection_state_cb, TRUE, &rc_addr);
       }
#endif
#if defined(MTK_A2DP_SRC_SINK_BOTH) && (MTK_A2DP_SRC_SINK_BOTH == TRUE)
        handle_rc_ctrl_features(btif_rc_cb.rc_addr);
#else
        /* report connection state if remote device is AVRCP target */
        if ((btif_rc_cb.rc_features & BTA_AV_FEAT_RCTG)||
           ((btif_rc_cb.rc_features & BTA_AV_FEAT_RCCT)&&
            (btif_rc_cb.rc_features & BTA_AV_FEAT_ADV_CTRL)))
        {
            handle_rc_ctrl_features(btif_rc_cb.rc_addr);
        }
#endif
#endif

#if defined(MTK_A2DP_SRC_SINK_BOTH) && (MTK_A2DP_SRC_SINK_BOTH == TRUE)
        handle_rc_features(btif_rc_cb.rc_addr);
#endif


#if defined(MTK_STACK_CONFIG_BL) && (MTK_STACK_CONFIG_BL == TRUE)
        btif_rc_cb.rc_disalbe_song_pos = FALSE;
        if (interop_mtk_match_addr_name(INTEROP_MTK_AVRCP_DISABLE_SONG_POS, (const bt_bdaddr_t *)&btif_rc_cb.rc_addr)
           && interop_mtk_match_name(INTEROP_MTK_AVRCP_DISABLE_SONG_POS, (const bt_bdaddr_t *)&btif_rc_cb.rc_addr))
        {
            btif_rc_cb.rc_disalbe_song_pos = TRUE;
            BTIF_TRACE_DEBUG("rc_disalbe_song_pos is TRUE!");
        }
#endif

    }
    else
    {
        BTIF_TRACE_ERROR("%s Connect failed with error code: %d",
            __FUNCTION__, p_rc_open->status);
        btif_rc_cb.rc_connected = FALSE;
    }

#if defined(MTK_PTS_AVRCP_CT_PTH_BV_01_C) && (MTK_PTS_AVRCP_CT_PTH_BV_01_C == TRUE)
    rc_transaction_t *p_transaction = NULL;
    bt_status_t tran_status = get_transaction(&p_transaction);
    if (BT_STATUS_SUCCESS == tran_status && NULL != p_transaction) {
        usleep(200000);
        BTA_AvRemoteCmd(btif_rc_cb.rc_handle, p_transaction->lbl,
                (tBTA_AV_RC)AVRC_ID_VOL_UP, (tBTA_AV_STATE)BTA_AV_STATE_PRESS);
        usleep(200000);
        BTA_AvRemoteCmd(btif_rc_cb.rc_handle, p_transaction->lbl,
                (tBTA_AV_RC)AVRC_ID_VOL_UP, (tBTA_AV_STATE)BTA_AV_STATE_RELEASE);
        usleep(200000);
        BTA_AvRemoteCmd(btif_rc_cb.rc_handle, p_transaction->lbl,
                (tBTA_AV_RC)AVRC_ID_VOL_DOWN, (tBTA_AV_STATE)BTA_AV_STATE_PRESS);
        usleep(200000);
        BTA_AvRemoteCmd(btif_rc_cb.rc_handle, p_transaction->lbl,
                (tBTA_AV_RC)AVRC_ID_VOL_DOWN, (tBTA_AV_STATE)BTA_AV_STATE_RELEASE);
    }
#endif
}

/***************************************************************************
 *  Function       handle_rc_disconnect
 *
 *  - Argument:    tBTA_AV_RC_CLOSE     RC close data structure
 *
 *  - Description: RC disconnection event handler
 *
 ***************************************************************************/
void handle_rc_disconnect (tBTA_AV_RC_CLOSE *p_rc_close)
{
#if (AVRC_CTLR_INCLUDED == TRUE)
    bt_bdaddr_t rc_addr;
#if defined(MTK_COMMON) && (MTK_COMMON == FALSE)
    tBTA_AV_FEAT features;
#endif
#endif
    BTIF_TRACE_DEBUG("%s: rc_handle: %d", __FUNCTION__, p_rc_close->rc_handle);
    if ((p_rc_close->rc_handle != btif_rc_cb.rc_handle)
        && (bdcmp(btif_rc_cb.rc_addr, p_rc_close->peer_addr)))
    {
        BTIF_TRACE_ERROR("Got disconnect of unknown device");
        return;
    }
#if (AVRC_CTLR_INCLUDED == TRUE)
    bdcpy(rc_addr.address, btif_rc_cb.rc_addr);
#if defined(MTK_COMMON) && (MTK_COMMON == FALSE)
    features = btif_rc_cb.rc_features;
#endif
        /* Clean up AVRCP procedure flags */
    memset(&btif_rc_cb.rc_app_settings, 0,
        sizeof(btif_rc_player_app_settings_t));
    btif_rc_cb.rc_features_processed = FALSE;
    btif_rc_cb.rc_procedure_complete = FALSE;
    rc_stop_play_status_timer();
#if defined(MTK_LINUX) && defined(MTK_COMMON) && (MTK_COMMON == TRUE)
    btif_rc_cb.rc_play_status = AVRC_PLAYSTATE_STOPPED;
#endif
    /* Check and clear the notification event list */
    if (btif_rc_cb.rc_supported_event_list != NULL)
    {
        list_clear(btif_rc_cb.rc_supported_event_list);
#if defined(MTK_LINUX_AVRCP_PLUS) && (MTK_LINUX_AVRCP_PLUS == TRUE)
        list_free(btif_rc_cb.rc_supported_event_list);
#endif
        btif_rc_cb.rc_supported_event_list = NULL;
    }
#endif
    btif_rc_cb.rc_handle = 0;
    btif_rc_cb.rc_connected = FALSE;
    memset(btif_rc_cb.rc_addr, 0, sizeof(BD_ADDR));
    memset(btif_rc_cb.rc_notif, 0, sizeof(btif_rc_cb.rc_notif));

#if defined(MTK_STACK_CONFIG_BL) && (MTK_STACK_CONFIG_BL == TRUE)
    LOG_DEBUG(LOG_TAG,"%s :FF/REW prestatus initialize ", __FUNCTION__);
    prestatus.key = 0;
    prestatus.pressed = -1;
#endif
    btif_rc_cb.rc_features = 0;
#if defined(MTK_A2DP_SRC_SINK_BOTH) && (MTK_A2DP_SRC_SINK_BOTH == TRUE)
    btif_rc_cb.peer_ct_features = 0;
    btif_rc_cb.peer_tg_features = 0;
    btif_rc_cb.launch_cmd_pending = 0;
#endif

    btif_rc_cb.rc_vol_label=MAX_LABEL;
    btif_rc_cb.rc_volume=MAX_VOLUME;
    init_all_transactions();
    if (bt_rc_callbacks != NULL)
    {
        close_uinput();
    }
    else
    {
        BTIF_TRACE_WARNING("%s Avrcp TG role not enabled, not closing UInput", __FUNCTION__);
    }

    memset(btif_rc_cb.rc_addr, 0, sizeof(BD_ADDR));
#if defined(MTK_STACK_CONFIG_BL) && (MTK_STACK_CONFIG_BL == TRUE)
    memset(&btif_rc_cb.rc_notif_virtual, 0, sizeof(UINT8) * MAX_RC_NOTIFICATIONS);
#endif
#if (AVRC_CTLR_INCLUDED == TRUE)
    /* report connection state if device is AVRCP target */
    if (bt_rc_ctrl_callbacks != NULL)
   {
#if defined MTK_BT_AVRCP_TG_15_BW && !(defined MTK_BT_AVRCP_TG_15_BW_NO_CONN_CB)
        HAL_CBACK(bt_rc_ctrl_callbacks, connection_state_cb, FALSE, FALSE, &rc_addr);
#else
        HAL_CBACK(bt_rc_ctrl_callbacks, connection_state_cb, FALSE, &rc_addr);
        return;
   }
#endif
#if defined(MTK_LINUX_AVRCP_PLUS) && (MTK_LINUX_AVRCP_PLUS == TRUE)
    if (bt_rc_ext_callbacks != NULL)
   {
    /* if support browsing channel need modify here */
        HAL_CBACK(bt_rc_ext_callbacks, connection_state_cb, FALSE, &rc_addr);
        return;
   }
#endif
}

/***************************************************************************
 *  Function       handle_rc_passthrough_cmd
 *
 *  - Argument:    tBTA_AV_RC rc_id   remote control command ID
 *                 tBTA_AV_STATE key_state status of key press
 *
 *  - Description: Remote control command handler
 *
 ***************************************************************************/
void handle_rc_passthrough_cmd ( tBTA_AV_REMOTE_CMD *p_remote_cmd)
{
    const char *status;
    int pressed, i;

    BTIF_TRACE_DEBUG("%s: p_remote_cmd->rc_id=%d", __FUNCTION__, p_remote_cmd->rc_id);

    /* If AVRC is open and peer sends PLAY but there is no AVDT, then we queue-up this PLAY */
    if (p_remote_cmd)
    {
        /* queue AVRC PLAY if GAVDTP Open notification to app is pending (2 second timer) */
        if ((p_remote_cmd->rc_id == BTA_AV_RC_PLAY) && (!btif_av_is_connected()))
        {
            if (p_remote_cmd->key_state == AVRC_STATE_PRESS)
            {
                APPL_TRACE_WARNING("%s: AVDT not open, queuing the PLAY command", __FUNCTION__);
                btif_rc_cb.rc_pending_play = TRUE;
            }
            return;
        }
#if defined(MTK_COMMON) && (MTK_COMMON == TRUE)
        if (btif_av_is_idle())
        {
            APPL_TRACE_WARNING("%s: AVDT not connection, drop command", __FUNCTION__);
            return;
        }
#endif

        if ((p_remote_cmd->rc_id == BTA_AV_RC_PAUSE) && (btif_rc_cb.rc_pending_play))
        {
            APPL_TRACE_WARNING("%s: Clear the pending PLAY on PAUSE received", __FUNCTION__);
            btif_rc_cb.rc_pending_play = FALSE;
            return;
        }
#if defined(MTK_AVRCP_SEND_TO_HAL) && (MTK_AVRCP_SEND_TO_HAL == FALSE)
        if ((p_remote_cmd->rc_id == BTA_AV_RC_VOL_UP)||(p_remote_cmd->rc_id == BTA_AV_RC_VOL_DOWN))
            return; // this command is not to be sent to UINPUT, only needed for PTS
#endif
    }

    if ((p_remote_cmd->rc_id == BTA_AV_RC_STOP) && (!btif_av_stream_started_ready()))
    {
        APPL_TRACE_WARNING("%s: Stream suspended, ignore STOP cmd",__FUNCTION__);
        return;
    }

    if (p_remote_cmd->key_state == AVRC_STATE_RELEASE) {
        status = "released";
        pressed = 0;
    } else {
        status = "pressed";
        pressed = 1;
    }
#if defined(MTK_AVRCP_SEND_TO_HAL) && (MTK_AVRCP_SEND_TO_HAL == TRUE)
    HAL_CBACK(bt_rc_callbacks, passthrough_cmd_cb, p_remote_cmd->rc_id, pressed);
    return;
#else
    if (p_remote_cmd->rc_id == BTA_AV_RC_FAST_FOR || p_remote_cmd->rc_id == BTA_AV_RC_REWIND) {
        HAL_CBACK(bt_rc_callbacks, passthrough_cmd_cb, p_remote_cmd->rc_id, pressed);
#if defined(MTK_STACK_CONFIG_BL) && (MTK_STACK_CONFIG_BL == TRUE)
        prestatus.key = p_remote_cmd->rc_id;
        prestatus.pressed= pressed;
#endif
        return;
    }
#endif
    for (i = 0; key_map[i].name != NULL; i++) {
        if (p_remote_cmd->rc_id == key_map[i].avrcp) {
            BTIF_TRACE_DEBUG("%s: %s %s", __FUNCTION__, key_map[i].name, status);

           /* MusicPlayer uses a long_press_timeout of 1 second for PLAYPAUSE button
            * and maps that to autoshuffle. So if for some reason release for PLAY/PAUSE
            * comes 1 second after the press, the MediaPlayer UI goes into a bad state.
            * The reason for the delay could be sniff mode exit or some AVDTP procedure etc.
            * The fix is to generate a release right after the press and drown the 'actual'
            * release.
            */
            if ((key_map[i].release_quirk == 1) && (pressed == 0))
            {
                BTIF_TRACE_DEBUG("%s: AVRC %s Release Faked earlier, drowned now",
                                  __FUNCTION__, key_map[i].name);
                return;
            }
            send_key(uinput_fd, key_map[i].mapped_id, pressed);
            if ((key_map[i].release_quirk == 1) && (pressed == 1))
            {
                sleep_ms(30);
                BTIF_TRACE_DEBUG("%s: AVRC %s Release quirk enabled, send release now",
                                  __FUNCTION__, key_map[i].name);
                send_key(uinput_fd, key_map[i].mapped_id, 0);
            }
            break;
        }
    }

    if (key_map[i].name == NULL)
        BTIF_TRACE_ERROR("%s AVRCP: unknown button 0x%02X %s", __FUNCTION__,
                        p_remote_cmd->rc_id, status);
}

/***************************************************************************
 *  Function       handle_rc_passthrough_rsp
 *
 *  - Argument:    tBTA_AV_REMOTE_RSP passthrough command response
 *
 *  - Description: Remote control passthrough response handler
 *
 ***************************************************************************/
void handle_rc_passthrough_rsp ( tBTA_AV_REMOTE_RSP *p_remote_rsp)
{
#if (AVRC_CTLR_INCLUDED == TRUE)
    const char *status;
    if (btif_rc_cb.rc_features & BTA_AV_FEAT_RCTG)
    {
        int key_state;
        if (p_remote_rsp->key_state == AVRC_STATE_RELEASE)
        {
            status = "released";
            key_state = 1;
        }
        else
        {
            status = "pressed";
            key_state = 0;
        }

        BTIF_TRACE_DEBUG("%s: rc_id=%d status=%s", __FUNCTION__, p_remote_rsp->rc_id, status);

        release_transaction(p_remote_rsp->label);
        if (bt_rc_ctrl_callbacks != NULL) {
            HAL_CBACK(bt_rc_ctrl_callbacks, passthrough_rsp_cb, p_remote_rsp->rc_id, key_state);
        }
#if defined(MTK_LINUX_AVRCP_PLUS) && (MTK_LINUX_AVRCP_PLUS == TRUE)
        else if (bt_rc_ext_callbacks != NULL) {
            HAL_CBACK(bt_rc_ext_callbacks, passthrough_rsp_cb, p_remote_rsp->rc_id, key_state);
        }
#endif
    }
    else
    {
        BTIF_TRACE_ERROR("%s DUT does not support AVRCP controller role", __FUNCTION__);
    }
#else
    BTIF_TRACE_ERROR("%s AVRCP controller role is not enabled", __FUNCTION__);
#endif
}

/***************************************************************************
 *  Function       handle_rc_vendorunique_rsp
 *
 *  - Argument:    tBTA_AV_REMOTE_RSP  command response
 *
 *  - Description: Remote control vendor unique response handler
 *
 ***************************************************************************/
void handle_rc_vendorunique_rsp ( tBTA_AV_REMOTE_RSP *p_remote_rsp)
{
#if (AVRC_CTLR_INCLUDED == TRUE)
    const char *status;
    UINT8 vendor_id = 0;
    if (btif_rc_cb.rc_features & BTA_AV_FEAT_RCTG)
    {
        int key_state;
        if (p_remote_rsp->key_state == AVRC_STATE_RELEASE)
        {
            status = "released";
            key_state = 1;
        }
        else
        {
            status = "pressed";
            key_state = 0;
        }

        if (p_remote_rsp->len > 0)
        {
            if (p_remote_rsp->len >= AVRC_PASS_THRU_GROUP_LEN)
                vendor_id = p_remote_rsp->p_data[AVRC_PASS_THRU_GROUP_LEN -1];
            osi_free_and_reset((void **)&p_remote_rsp->p_data);
        }
        BTIF_TRACE_DEBUG("%s: vendor_id=%d status=%s", __FUNCTION__, vendor_id, status);

        release_transaction(p_remote_rsp->label);
        HAL_CBACK(bt_rc_ctrl_callbacks, groupnavigation_rsp_cb, vendor_id, key_state);
    }
    else
    {
        BTIF_TRACE_ERROR("%s Remote does not support AVRCP TG role", __FUNCTION__);
    }
#else
    BTIF_TRACE_ERROR("%s AVRCP controller role is not enabled", __FUNCTION__);
#endif
}

void handle_uid_changed_notification(tBTA_AV_META_MSG *pmeta_msg, tAVRC_COMMAND *pavrc_command)
{
    tAVRC_RESPONSE avrc_rsp = {0};
    avrc_rsp.rsp.pdu = pavrc_command->pdu;
    avrc_rsp.rsp.status = AVRC_STS_NO_ERROR;
    avrc_rsp.rsp.opcode = pavrc_command->cmd.opcode;

    avrc_rsp.reg_notif.event_id = pavrc_command->reg_notif.event_id;
    avrc_rsp.reg_notif.param.uid_counter = 0;

    send_metamsg_rsp(pmeta_msg->rc_handle, pmeta_msg->label, AVRC_RSP_INTERIM, &avrc_rsp);
    send_metamsg_rsp(pmeta_msg->rc_handle, pmeta_msg->label, AVRC_RSP_CHANGED, &avrc_rsp);

}

/***************************************************************************
 *  Function       handle_rc_metamsg_cmd
 *
 *  - Argument:    tBTA_AV_VENDOR Structure containing the received
 *                          metamsg command
 *
 *  - Description: Remote control metamsg command handler (AVRCP 1.3)
 *
 ***************************************************************************/
void handle_rc_metamsg_cmd (tBTA_AV_META_MSG *pmeta_msg)
{
    /* Parse the metamsg command and pass it on to BTL-IFS */
    UINT8             scratch_buf[512] = {0};
    tAVRC_COMMAND    avrc_command = {0};
    tAVRC_STS status;

    BTIF_TRACE_EVENT("+ %s", __FUNCTION__);

#if defined(MTK_AVRCP_TG_15_BROWSE)&&(MTK_AVRCP_TG_15_BROWSE == TRUE)
    if (pmeta_msg->p_msg->hdr.opcode != AVRC_OP_VENDOR
        && pmeta_msg->p_msg->hdr.opcode != AVRC_OP_BROWSE)
#else
    if (pmeta_msg->p_msg->hdr.opcode != AVRC_OP_VENDOR)
#endif
    {
        BTIF_TRACE_WARNING("Invalid opcode: %x", pmeta_msg->p_msg->hdr.opcode);
        return;
    }
    if (pmeta_msg->len < 3)
    {
        BTIF_TRACE_WARNING("Invalid length.Opcode: 0x%x, len: 0x%x", pmeta_msg->p_msg->hdr.opcode,
            pmeta_msg->len);
        return;
    }

    if (pmeta_msg->code >= AVRC_RSP_NOT_IMPL)
    {
#if (AVRC_ADV_CTRL_INCLUDED == TRUE)
{
     rc_transaction_t *transaction=NULL;
     transaction=get_transaction_by_lbl(pmeta_msg->label);
     if (NULL!=transaction)
     {
        handle_rc_metamsg_rsp(pmeta_msg);
     }
     else
     {
         BTIF_TRACE_DEBUG("%s:Discard vendor dependent rsp. code: %d label:%d.",
             __FUNCTION__, pmeta_msg->code, pmeta_msg->label);
     }
     return;
}
#else
{
        BTIF_TRACE_DEBUG("%s:Received vendor dependent rsp. code: %d len: %d. Not processing it.",
            __FUNCTION__, pmeta_msg->code, pmeta_msg->len);
        return;
}
#endif
      }

    status=AVRC_ParsCommand(pmeta_msg->p_msg, &avrc_command, scratch_buf, sizeof(scratch_buf));
    BTIF_TRACE_DEBUG("%s Received vendor command.code,PDU and label: %d, %d,%d",
                     __FUNCTION__, pmeta_msg->code, avrc_command.cmd.pdu, pmeta_msg->label);

    if (status != AVRC_STS_NO_ERROR)
    {
        /* return error */
        BTIF_TRACE_WARNING("%s: Error in parsing received metamsg command. status: 0x%02x",
            __FUNCTION__, status);
        send_reject_response(pmeta_msg->rc_handle, pmeta_msg->label, avrc_command.pdu, status);
    }
    else
    {
        /* if RegisterNotification, add it to our registered queue */

        if (avrc_command.cmd.pdu == AVRC_PDU_REGISTER_NOTIFICATION)
        {
            UINT8 event_id = avrc_command.reg_notif.event_id;
            BTIF_TRACE_EVENT("%s:New register notification received.event_id:%s,label:0x%x,code:%x",
            __FUNCTION__,dump_rc_notification_event_id(event_id), pmeta_msg->label,pmeta_msg->code);
            btif_rc_cb.rc_notif[event_id-1].bNotify = TRUE;
            btif_rc_cb.rc_notif[event_id-1].label = pmeta_msg->label;

            if (event_id == AVRC_EVT_UIDS_CHANGE)
            {
                handle_uid_changed_notification(pmeta_msg, &avrc_command);
                return;
            }
#if defined(MTK_A2DP_SRC_SINK_BOTH) && (MTK_A2DP_SRC_SINK_BOTH == TRUE)
            /* this is sink(tg) feature, so it should not handle here */
            if (btif_rc_both_enable() && event_id == AVRC_EVT_VOLUME_CHANGE)
            {
                return;
            }
#endif
        }

#if defined(MTK_A2DP_SRC_SINK_BOTH) && (MTK_A2DP_SRC_SINK_BOTH == TRUE)
        /* this is sink(tg) feature, so it should not handle here */
        if (btif_rc_both_enable() && avrc_command.cmd.pdu == AVRC_PDU_SET_ABSOLUTE_VOLUME)
        {
            return;
        }
#endif

        BTIF_TRACE_EVENT("%s: Passing received metamsg command to app. pdu: %s",
            __FUNCTION__, dump_rc_pdu(avrc_command.cmd.pdu));

        /* Since handle_rc_metamsg_cmd() itself is called from
            *btif context, no context switching is required. Invoke
            * btif_rc_upstreams_evt directly from here. */
        btif_rc_upstreams_evt((uint16_t)avrc_command.cmd.pdu, &avrc_command, pmeta_msg->code,
                               pmeta_msg->label);
    }
}

#if defined(MTK_A2DP_SRC_SINK_BOTH) && (MTK_A2DP_SRC_SINK_BOTH == TRUE)
static tAVRC_STS btif_rc_upstreams_rsp_evt_all(UINT16 event, tAVRC_RESPONSE *pavrc_resp, UINT8 ctype, UINT8 label)
{
    tAVRC_STS status = AVRC_STS_INTERNAL_ERR;
    BTIF_TRACE_EVENT("%s pdu: %s handle: 0x%x ctype:%x label:%x", __FUNCTION__,
        dump_rc_pdu(pavrc_resp->pdu), btif_rc_cb.rc_handle, ctype, label);

    switch (event)
    {
        case AVRC_PDU_REGISTER_NOTIFICATION:
        {
            if (AVRC_EVT_VOLUME_CHANGE==pavrc_resp->reg_notif.event_id)
            {
                 if (AVRC_RSP_CHANGED==ctype)
                     btif_rc_cb.rc_volume=pavrc_resp->reg_notif.param.volume;
                 HAL_CBACK(bt_rc_callbacks, volume_change_cb, pavrc_resp->reg_notif.param.volume,ctype)
                 status = AVRC_STS_NO_ERROR;
            }
        }
        break;

        case AVRC_PDU_SET_ABSOLUTE_VOLUME:
        {
            BTIF_TRACE_DEBUG("%s Set absolute volume change event received: volume %d,ctype %d",
                             __FUNCTION__, pavrc_resp->volume.volume,ctype);
            if (AVRC_RSP_ACCEPT==ctype)
                btif_rc_cb.rc_volume=pavrc_resp->volume.volume;
            HAL_CBACK(bt_rc_callbacks,volume_change_cb,pavrc_resp->volume.volume,ctype)
            status = AVRC_STS_NO_ERROR;
        }
        break;

        default:
            break;
    }
    return status;
}


static tAVRC_STS handle_rc_metamsg_rsp_all(tBTA_AV_META_MSG *pmeta_msg)
{
    tAVRC_RESPONSE    avrc_response = {0};
    UINT8             scratch_buf[512] = {0};
    tAVRC_STS status = BT_STATUS_UNSUPPORTED;

    if (AVRC_OP_VENDOR==pmeta_msg->p_msg->hdr.opcode &&(AVRC_RSP_CHANGED==pmeta_msg->code
      || AVRC_RSP_INTERIM==pmeta_msg->code || AVRC_RSP_ACCEPT==pmeta_msg->code
      || AVRC_RSP_REJ==pmeta_msg->code || AVRC_RSP_NOT_IMPL==pmeta_msg->code))
    {
        status=AVRC_ParsResponse(pmeta_msg->p_msg, &avrc_response, scratch_buf, sizeof(scratch_buf));
        BTIF_TRACE_DEBUG("%s: code %d,event ID %d,PDU %x,parsing status %d, label:%d",
          __FUNCTION__,pmeta_msg->code,avrc_response.reg_notif.event_id,avrc_response.reg_notif.pdu,
          status, pmeta_msg->label);

        if (status != AVRC_STS_NO_ERROR)
        {
            if (AVRC_PDU_REGISTER_NOTIFICATION==avrc_response.rsp.pdu
                && AVRC_EVT_VOLUME_CHANGE==avrc_response.reg_notif.event_id
                && btif_rc_cb.rc_vol_label==pmeta_msg->label)
            {
                btif_rc_cb.rc_vol_label=MAX_LABEL;
                release_transaction(btif_rc_cb.rc_vol_label);
            }
            else if (AVRC_PDU_SET_ABSOLUTE_VOLUME==avrc_response.rsp.pdu)
            {
                release_transaction(pmeta_msg->label);
            }
            return status;
        }
        else if (AVRC_PDU_REGISTER_NOTIFICATION==avrc_response.rsp.pdu
            && AVRC_EVT_VOLUME_CHANGE==avrc_response.reg_notif.event_id
            && btif_rc_cb.rc_vol_label!=pmeta_msg->label)
            {
                // Just discard the message, if the device sends back with an incorrect label
                BTIF_TRACE_DEBUG("%s:Discarding register notfn in rsp.code: %d and label %d",
                __FUNCTION__, pmeta_msg->code, pmeta_msg->label);
                return status;
            }
    }
    else
    {
        BTIF_TRACE_DEBUG("%s:Received vendor dependent in adv ctrl rsp. code: %d len: %d. Not processing it.",
        __FUNCTION__, pmeta_msg->code, pmeta_msg->len);
        return status;
    }

    if (AVRC_PDU_REGISTER_NOTIFICATION==avrc_response.rsp.pdu
        && AVRC_EVT_VOLUME_CHANGE==avrc_response.reg_notif.event_id
        && AVRC_RSP_CHANGED==pmeta_msg->code)
     {
         /* re-register for volume change notification */
         // Do not re-register for rejected case, as it might get into endless loop
         register_volumechange(btif_rc_cb.rc_vol_label);
     }
     else if (AVRC_PDU_SET_ABSOLUTE_VOLUME==avrc_response.rsp.pdu)
     {
          /* free up the label here */
          release_transaction(pmeta_msg->label);
     }

     BTIF_TRACE_EVENT("%s: Passing received metamsg response to app. pdu: %s",
             __FUNCTION__, dump_rc_pdu(avrc_response.pdu));
     return btif_rc_upstreams_rsp_evt_all((uint16_t)avrc_response.rsp.pdu, &avrc_response, pmeta_msg->code,
                                pmeta_msg->label);
}

tAVRC_STS handle_rc_metamsg_cmd_all (tBTA_AV_META_MSG *pmeta_msg)
{
    /* Parse the metamsg command and pass it on to BTL-IFS */
    UINT8             scratch_buf[512] = {0};
    tAVRC_COMMAND    avrc_command = {0};
    tAVRC_STS status = AVRC_STS_INTERNAL_ERR;

    BTIF_TRACE_EVENT("+ %s", __FUNCTION__);

    if (pmeta_msg->p_msg->hdr.opcode != AVRC_OP_VENDOR)
    {
        BTIF_TRACE_WARNING("Invalid opcode: %x", pmeta_msg->p_msg->hdr.opcode);
        return AVRC_STS_BAD_CMD;
    }
    if (pmeta_msg->len < 3)
    {
        BTIF_TRACE_WARNING("Invalid length.Opcode: 0x%x, len: 0x%x", pmeta_msg->p_msg->hdr.opcode,
            pmeta_msg->len);
        return AVRC_STS_BAD_CMD;
    }

    if (pmeta_msg->code >= AVRC_RSP_NOT_IMPL)
    {
#if (AVRC_ADV_CTRL_INCLUDED == TRUE)
        {
             rc_transaction_t *transaction=NULL;
             transaction=get_transaction_by_lbl(pmeta_msg->label);
             if (NULL!=transaction)
             {
                status = handle_rc_metamsg_rsp_all(pmeta_msg);
             }
             else
             {
                 BTIF_TRACE_DEBUG("%s:Discard vendor dependent rsp. code: %d label:%d.",
                     __FUNCTION__, pmeta_msg->code, pmeta_msg->label);
             }
             return status;
        }
#else
        {
                BTIF_TRACE_DEBUG("%s:Received vendor dependent rsp. code: %d len: %d. Not processing it.",
                    __FUNCTION__, pmeta_msg->code, pmeta_msg->len);
                return status;
        }
#endif
      }

    status=AVRC_ParsCommand(pmeta_msg->p_msg, &avrc_command, scratch_buf, sizeof(scratch_buf));
    BTIF_TRACE_DEBUG("%s Received vendor command.code,PDU and label: %d, %d,%d",
                     __FUNCTION__, pmeta_msg->code, avrc_command.cmd.pdu, pmeta_msg->label);

    if (status != AVRC_STS_NO_ERROR)
    {
        /* return error */
        BTIF_TRACE_WARNING("%s: Error in parsing received metamsg command. status: 0x%02x",
            __FUNCTION__, status);
        send_reject_response(pmeta_msg->rc_handle, pmeta_msg->label, avrc_command.pdu, status);
        status = AVRC_STS_NO_ERROR;
    }
    else
    {
        /* if RegisterNotification, add it to our registered queue */

        if (avrc_command.cmd.pdu == AVRC_PDU_REGISTER_NOTIFICATION)
        {
            UINT8 event_id = avrc_command.reg_notif.event_id;
            BTIF_TRACE_EVENT("%s:New register notification received.event_id:%s,label:0x%x,code:%x",
            __FUNCTION__,dump_rc_notification_event_id(event_id), pmeta_msg->label,pmeta_msg->code);
            btif_rc_cb.rc_notif[event_id-1].bNotify = TRUE;
            btif_rc_cb.rc_notif[event_id-1].label = pmeta_msg->label;

            if (event_id == AVRC_EVT_UIDS_CHANGE)
            {
                handle_uid_changed_notification(pmeta_msg, &avrc_command);
                return status;
            }

            /* this is sink(tg) feature, so it should not handle here */
            if (event_id == AVRC_EVT_VOLUME_CHANGE)
            {
                return AVRC_STS_INTERNAL_ERR;
            }
        }

        /* this is sink(tg) feature, so it should not handle here */
        if (avrc_command.cmd.pdu == AVRC_PDU_SET_ABSOLUTE_VOLUME)
        {
            return AVRC_STS_INTERNAL_ERR;
        }

        BTIF_TRACE_EVENT("%s: Passing received metamsg command to app. pdu: %s",
            __FUNCTION__, dump_rc_pdu(avrc_command.cmd.pdu));

        /* Since handle_rc_metamsg_cmd() itself is called from
            *btif context, no context switching is required. Invoke
            * btif_rc_upstreams_evt directly from here. */
        btif_rc_upstreams_evt((uint16_t)avrc_command.cmd.pdu, &avrc_command, pmeta_msg->code,
                               pmeta_msg->label);
        return AVRC_STS_NO_ERROR;
    }
    return status;
}

void btif_rc_handle_meta_all(tBTA_AV_EVT event, tBTA_AV *p_data)
{
    tAVRC_STS status = AVRC_STS_INTERNAL_ERR;
    BTIF_TRACE_DEBUG("%s TG BTA_AV_META_MSG_EVT  code:%d label:%d",
                     __FUNCTION__,
                     p_data->meta_msg.code,
                     p_data->meta_msg.label);
    BTIF_TRACE_DEBUG("%s company_id:0x%x len:%d handle:%d",
                     __FUNCTION__,
                     p_data->meta_msg.company_id,
                     p_data->meta_msg.len,
                     p_data->meta_msg.rc_handle);
    /* handle the metamsg command */
    status = handle_rc_metamsg_cmd_all(&(p_data->meta_msg));
    /* Free the Memory allocated for tAVRC_MSG */
    if (status == AVRC_STS_NO_ERROR)
    {
        return;
    }

    /* This is case of Sink + CT + TG(for abs vol)) */
    BTIF_TRACE_DEBUG("%s CT BTA_AV_META_MSG_EVT  code:%d label:%d",
                     __FUNCTION__,
                     p_data->meta_msg.code,
                     p_data->meta_msg.label);
    BTIF_TRACE_DEBUG("%s company_id:0x%x len:%d handle:%d",
                     __FUNCTION__,
                     p_data->meta_msg.company_id,
                     p_data->meta_msg.len,
                     p_data->meta_msg.rc_handle);
    if ((p_data->meta_msg.code >= AVRC_RSP_NOT_IMPL)&&
        (p_data->meta_msg.code <= AVRC_RSP_INTERIM))
    {
        /* Its a response */
        handle_avk_rc_metamsg_rsp(&(p_data->meta_msg));
    }
    else if (p_data->meta_msg.code <= AVRC_CMD_GEN_INQ)
    {
        /* Its a command  */
        handle_avk_rc_metamsg_cmd(&(p_data->meta_msg));
    }
}

void btif_rc_handle_remote_cmd_all(tBTA_AV_EVT event, tBTA_AV *p_data)
{
    if ((p_data->remote_cmd.rc_id == BTA_AV_RC_VOL_UP) || (p_data->remote_cmd.rc_id == BTA_AV_RC_VOL_DOWN))
    {
#if defined(MTK_COMMON) && (MTK_COMMON == TRUE)
        if (btif_av_is_idle())
        {
            APPL_TRACE_WARNING("%s: AVDT not connection, drop command", __FUNCTION__);
            return;
        }
#endif
        const char *status;
        int pressed;
        if (AVRC_STATE_RELEASE == p_data->remote_cmd.key_state)
        {
            status = "released";
            pressed = 0;
        }
        else
        {
            status = "pressed";
            pressed = 1;
        }
        HAL_CBACK(bt_rc_ctrl_ext_callbacks, passthrough_volume_cmd_cb, p_data->remote_cmd.rc_id, pressed);
    }
    else
    {
      BTIF_TRACE_DEBUG("%s rc_id:0x%x key_state:%d",
                       __FUNCTION__, p_data->remote_cmd.rc_id,
                       p_data->remote_cmd.key_state);
        /** In race conditions just after 2nd AVRCP is connected
         *  remote might send pass through commands, so check for
         *  Rc handle before processing pass through commands
         **/
        if (btif_rc_cb.rc_handle == p_data->remote_cmd.rc_handle)
        {
            handle_rc_passthrough_cmd( (&p_data->remote_cmd) );
        }
        else
        {
            BTIF_TRACE_DEBUG("%s Pass-through command for Invalid rc handle", __FUNCTION__);
        }
    }
}
#endif

/***************************************************************************
 **
 ** Function       btif_rc_handler
 **
 ** Description    RC event handler
 **
 ***************************************************************************/
void btif_rc_handler(tBTA_AV_EVT event, tBTA_AV *p_data)
{
    BTIF_TRACE_DEBUG ("%s event:%s", __FUNCTION__, dump_rc_event(event));
    switch (event)
    {
        case BTA_AV_RC_OPEN_EVT:
        {
            BTIF_TRACE_DEBUG("%s Peer_features:%x", __FUNCTION__, p_data->rc_open.peer_features);
            handle_rc_connect( &(p_data->rc_open) );
        }break;

        case BTA_AV_RC_CLOSE_EVT:
        {
            handle_rc_disconnect( &(p_data->rc_close) );
        }break;

#if defined(MTK_AVRCP_TG_15_BROWSE)&&(MTK_AVRCP_TG_15_BROWSE == TRUE)
        case BTA_AV_RC_BROWSE_OPEN_EVT: {
            /* tell the UL that we have connection to browse channel and that
            * browse commands can be directed accordingly. */
            handle_rc_browse_connect(&p_data->rc_browse_open);
        } break;

        case BTA_AV_RC_BROWSE_CLOSE_EVT: {
            BTIF_TRACE_DEBUG("%s: BTA_AV_RC_BROWSE_CLOSE_EVT", __func__);
        } break;
#endif

        case BTA_AV_REMOTE_CMD_EVT:
        {
#if defined(MTK_A2DP_SRC_SINK_BOTH) && (MTK_A2DP_SRC_SINK_BOTH == TRUE)
            if (btif_rc_both_enable())
            {
                btif_rc_handle_remote_cmd_all(event, p_data);
                break;
            }
#endif
            if (bt_rc_callbacks != NULL)
            {
              BTIF_TRACE_DEBUG("%s rc_id:0x%x key_state:%d",
                               __FUNCTION__, p_data->remote_cmd.rc_id,
                               p_data->remote_cmd.key_state);
                /** In race conditions just after 2nd AVRCP is connected
                 *  remote might send pass through commands, so check for
                 *  Rc handle before processing pass through commands
                 **/
#if defined(MTK_AVRCP_TG_15_BROWSE)&&(MTK_AVRCP_TG_15_BROWSE == TRUE)
                handle_rc_passthrough_cmd( (&p_data->remote_cmd) );
#else
                if (btif_rc_cb.rc_handle == p_data->remote_cmd.rc_handle)
                {
                    handle_rc_passthrough_cmd( (&p_data->remote_cmd) );
                }
                else
                {
                    BTIF_TRACE_DEBUG("%s Pass-through command for Invalid rc handle", __FUNCTION__);
                }
 #endif //(end -- #if defined(MTK_AVRCP_TG_15_BROWSE))
           }
#if defined(MTK_LINUX_AVRCP_PLUS) && (MTK_LINUX_AVRCP_PLUS == TRUE)
            else if ((bt_rc_ctrl_ext_callbacks != NULL) && (btif_rc_cb.rc_handle == p_data->remote_cmd.rc_handle))
            {
                if ((p_data->remote_cmd.rc_id == BTA_AV_RC_VOL_UP) || (p_data->remote_cmd.rc_id == BTA_AV_RC_VOL_DOWN))
                {
#if defined(MTK_COMMON) && (MTK_COMMON == TRUE)
                    if (btif_av_is_idle())
                    {
                        APPL_TRACE_WARNING("%s: AVDT not connection, drop command", __FUNCTION__);
                        return;
                    }
#endif
                    const char *status;
                    int pressed;
                    if (AVRC_STATE_RELEASE == p_data->remote_cmd.key_state)
                    {
                        status = "released";
                        pressed = 0;
                    }
                    else
                    {
                        status = "pressed";
                        pressed = 1;
                    }
                    BTIF_TRACE_DEBUG("%s: VOLUME %s", __FUNCTION__, status);
                    HAL_CBACK(bt_rc_ctrl_ext_callbacks, passthrough_volume_cmd_cb, p_data->remote_cmd.rc_id, pressed);
                }
                else
                {
                    BTIF_TRACE_WARNING("%s, isn't volume command, just drop it", __FUNCTION__);
                }
            }
#endif
            else
            {
                BTIF_TRACE_ERROR("AVRCP TG role not up, drop passthrough commands");
            }
        }
        break;

#if (AVRC_CTLR_INCLUDED == TRUE)
        case BTA_AV_REMOTE_RSP_EVT:
        {
            BTIF_TRACE_DEBUG("%s RSP: rc_id:0x%x key_state:%d",
                             __FUNCTION__, p_data->remote_rsp.rc_id, p_data->remote_rsp.key_state);
            if (p_data->remote_rsp.rc_id == AVRC_ID_VENDOR)
            {
                handle_rc_vendorunique_rsp(&p_data->remote_rsp);
            }
            else
            {
                handle_rc_passthrough_rsp(&p_data->remote_rsp);
            }
        }
        break;

#endif
        case BTA_AV_RC_FEAT_EVT:
        {
            BTIF_TRACE_DEBUG("%s Peer_features:%x", __FUNCTION__, p_data->rc_feat.peer_features);
            btif_rc_cb.rc_features = p_data->rc_feat.peer_features;

#if defined(MTK_A2DP_SRC_SINK_BOTH) && (MTK_A2DP_SRC_SINK_BOTH == TRUE)
            BTIF_TRACE_DEBUG("%s peer_ct_features:0x%x, peer_tg_features=0x%x",
                __FUNCTION__,
                p_data->rc_feat.peer_ct_features,
                p_data->rc_feat.peer_tg_features);
            btif_rc_cb.peer_ct_features = p_data->rc_feat.peer_ct_features;
            btif_rc_cb.peer_tg_features = p_data->rc_feat.peer_tg_features;
#endif

            handle_rc_features(p_data->rc_feat.peer_addr);
#if (AVRC_CTLR_INCLUDED == TRUE)
            if ((btif_rc_cb.rc_connected) && (bt_rc_ctrl_callbacks != NULL))
            {
                handle_rc_ctrl_features(btif_rc_cb.rc_addr);
            }
#endif
        }
        break;

        case BTA_AV_META_MSG_EVT:
        {
#if defined(MTK_A2DP_SRC_SINK_BOTH) && (MTK_A2DP_SRC_SINK_BOTH == TRUE)
            if (btif_rc_both_enable())
            {
                btif_rc_handle_meta_all(event, p_data);
                break;
            }
#endif
            if (bt_rc_callbacks != NULL)
            {
                BTIF_TRACE_DEBUG("%s BTA_AV_META_MSG_EVT  code:%d label:%d",
                                 __FUNCTION__,
                                 p_data->meta_msg.code,
                                 p_data->meta_msg.label);
                BTIF_TRACE_DEBUG("%s company_id:0x%x len:%d handle:%d",
                                 __FUNCTION__,
                                 p_data->meta_msg.company_id,
                                 p_data->meta_msg.len,
                                 p_data->meta_msg.rc_handle);
                /* handle the metamsg command */
                handle_rc_metamsg_cmd(&(p_data->meta_msg));
                /* Free the Memory allocated for tAVRC_MSG */
            }
#if (AVRC_CTLR_INCLUDED == TRUE)
#if defined(MTK_AVRCP_TG_15_BROWSE)&&(MTK_AVRCP_TG_15_BROWSE == TRUE)
            else if (bt_rc_ctrl_callbacks != NULL)
#else
            else if ((bt_rc_callbacks == NULL)&&(bt_rc_ctrl_callbacks != NULL))
#endif
            {
                /* This is case of Sink + CT + TG(for abs vol)) */
                BTIF_TRACE_DEBUG("%s BTA_AV_META_MSG_EVT  code:%d label:%d",
                                 __FUNCTION__,
                                 p_data->meta_msg.code,
                                 p_data->meta_msg.label);
                BTIF_TRACE_DEBUG("%s company_id:0x%x len:%d handle:%d",
                                 __FUNCTION__,
                                 p_data->meta_msg.company_id,
                                 p_data->meta_msg.len,
                                 p_data->meta_msg.rc_handle);
#if defined(MTK_AVRCP_TG_15_BROWSE)&&(MTK_AVRCP_TG_15_BROWSE == TRUE)
                switch (p_data->meta_msg.p_msg->hdr.opcode) {
                    case AVRC_OP_VENDOR:
                        if ((p_data->meta_msg.code >= AVRC_RSP_NOT_IMPL) &&
                                (p_data->meta_msg.code <= AVRC_RSP_INTERIM)) {
                            /* Its a response */
                            handle_avk_rc_metamsg_rsp(&(p_data->meta_msg));
                        } else if (p_data->meta_msg.code <= AVRC_CMD_GEN_INQ) {
                            /* Its a command  */
                            handle_avk_rc_metamsg_cmd(&(p_data->meta_msg));
                        }
                        break;

                    case AVRC_OP_BROWSE:
                        if (p_data->meta_msg.p_msg->hdr.ctype == AVRC_CMD) {
                            handle_avk_rc_metamsg_cmd(&(p_data->meta_msg));
                        } else if (p_data->meta_msg.p_msg->hdr.ctype == AVRC_RSP) {
                            handle_avk_rc_metamsg_rsp(&(p_data->meta_msg));
                        }
                        break;
                }
#else
                if ((p_data->meta_msg.code >= AVRC_RSP_NOT_IMPL)&&
                    (p_data->meta_msg.code <= AVRC_RSP_INTERIM))
                {
                    /* Its a response */
                    handle_avk_rc_metamsg_rsp(&(p_data->meta_msg));
                }
                else if (p_data->meta_msg.code <= AVRC_CMD_GEN_INQ)
                {
                    /* Its a command  */
                    handle_avk_rc_metamsg_cmd(&(p_data->meta_msg));
                }
#endif //(end -- #if defined(MTK_AVRCP_TG_15_BROWSE))
            }
#endif
            else
            {
                BTIF_TRACE_ERROR("Neither CTRL, nor TG is up, drop meta commands");
            }
        }
        break;

        default:
            BTIF_TRACE_DEBUG("%s Unhandled RC event : 0x%x", __FUNCTION__, event);
    }
}

/***************************************************************************
 **
 ** Function       btif_rc_get_connected_peer
 **
 ** Description    Fetches the connected headset's BD_ADDR if any
 **
 ***************************************************************************/
BOOLEAN btif_rc_get_connected_peer(BD_ADDR peer_addr)
{
    if (btif_rc_cb.rc_connected == TRUE) {
        bdcpy(peer_addr, btif_rc_cb.rc_addr);
        return TRUE;
    }
    return FALSE;
}

/***************************************************************************
 **
 ** Function       btif_rc_get_connected_peer_handle
 **
 ** Description    Fetches the connected headset's handle if any
 **
 ***************************************************************************/
UINT8 btif_rc_get_connected_peer_handle(void)
{
    return btif_rc_cb.rc_handle;
}

/***************************************************************************
 **
 ** Function       btif_rc_check_handle_pending_play
 **
 ** Description    Clears the queued PLAY command. if bSend is TRUE, forwards to app
 **
 ***************************************************************************/

/* clear the queued PLAY command. if bSend is TRUE, forward to app */
void btif_rc_check_handle_pending_play (BD_ADDR peer_addr, BOOLEAN bSendToApp)
{
    UNUSED(peer_addr);

    BTIF_TRACE_DEBUG("%s: bSendToApp=%d", __FUNCTION__, bSendToApp);
    if (btif_rc_cb.rc_pending_play)
    {
#if defined(MTK_COMMON) && (MTK_COMMON == TRUE)
        if (bSendToApp && !btif_av_is_idle())
#else
        if (bSendToApp)
#endif
        {
            tBTA_AV_REMOTE_CMD remote_cmd;
            APPL_TRACE_DEBUG("%s: Sending queued PLAYED event to app", __FUNCTION__);

            memset (&remote_cmd, 0, sizeof(tBTA_AV_REMOTE_CMD));
            remote_cmd.rc_handle  = btif_rc_cb.rc_handle;
            remote_cmd.rc_id      = AVRC_ID_PLAY;
            remote_cmd.hdr.ctype  = AVRC_CMD_CTRL;
            remote_cmd.hdr.opcode = AVRC_OP_PASS_THRU;

            /* delay sending to app, else there is a timing issue in the framework,
             ** which causes the audio to be on th device's speaker. Delay between
             ** OPEN & RC_PLAYs
            */
            sleep_ms(200);
            /* send to app - both PRESSED & RELEASED */
            remote_cmd.key_state  = AVRC_STATE_PRESS;
            handle_rc_passthrough_cmd( &remote_cmd );

            sleep_ms(100);

            remote_cmd.key_state  = AVRC_STATE_RELEASE;
            handle_rc_passthrough_cmd( &remote_cmd );
        }
        btif_rc_cb.rc_pending_play = FALSE;
    }
}

#if defined(MTK_A2DP_SRC_SINK_BOTH) && (MTK_A2DP_SRC_SINK_BOTH == TRUE)
void btif_rc_check_pending_cmd (BD_ADDR peer_addr)
{
    UNUSED(peer_addr);

    BTIF_TRACE_DEBUG("%s: launch_cmd_pending=%d, rc_connected=%d, peer_ct_features=0x%x, peer_tg_features=0x%x",
        __FUNCTION__, btif_rc_cb.launch_cmd_pending,
        btif_rc_cb.rc_connected,
        btif_rc_cb.peer_ct_features,
        btif_rc_cb.peer_tg_features);
    if (btif_rc_cb.launch_cmd_pending && btif_rc_cb.rc_connected)
    {
        /* if remote is sink */
        if(!btif_av_peer_is_src())
        {
            btif_rc_cb.rc_features = btif_rc_cb.peer_ct_features;
            btif_reg_abs_volume_event();
        }
        else
        {
            btif_rc_cb.rc_features = btif_rc_cb.peer_tg_features;
            getcapabilities_cmd (AVRC_CAP_COMPANY_ID);
        }
    }
    btif_rc_cb.launch_cmd_pending = FALSE;
}
#endif

/* Generic reject response */
static void send_reject_response (UINT8 rc_handle, UINT8 label, UINT8 pdu, UINT8 status)
{
    UINT8 ctype = AVRC_RSP_REJ;
    tAVRC_RESPONSE avrc_rsp;
    BT_HDR *p_msg = NULL;
    memset (&avrc_rsp, 0, sizeof(tAVRC_RESPONSE));

    avrc_rsp.rsp.opcode = opcode_from_pdu(pdu);
    avrc_rsp.rsp.pdu    = pdu;
    avrc_rsp.rsp.status = status;

    if (AVRC_STS_NO_ERROR == (status = AVRC_BldResponse(rc_handle, &avrc_rsp, &p_msg)) )
    {
        BTIF_TRACE_DEBUG("%s:Sending error notification to handle:%d. pdu:%s,status:0x%02x",
            __FUNCTION__, rc_handle, dump_rc_pdu(pdu), status);
        BTA_AvMetaRsp(rc_handle, label, ctype, p_msg);
    }
}

/***************************************************************************
 *  Function       send_metamsg_rsp
 *
 *  - Argument:
 *                  rc_handle     RC handle corresponding to the connected RC
 *                  label            Label of the RC response
 *                  code            Response type
 *                  pmetamsg_resp    Vendor response
 *
 *  - Description: Remote control metamsg response handler (AVRCP 1.3)
 *
 ***************************************************************************/
static void send_metamsg_rsp (UINT8 rc_handle, UINT8 label, tBTA_AV_CODE code,
    tAVRC_RESPONSE *pmetamsg_resp)
{
    UINT8 ctype;

    if (!pmetamsg_resp)
    {
        BTIF_TRACE_WARNING("%s: Invalid response received from application", __FUNCTION__);
        return;
    }

    BTIF_TRACE_EVENT("+%s: rc_handle: %d, label: %d, code: 0x%02x, pdu: %s", __FUNCTION__,
        rc_handle, label, code, dump_rc_pdu(pmetamsg_resp->rsp.pdu));

    if (pmetamsg_resp->rsp.status != AVRC_STS_NO_ERROR)
    {
        ctype = AVRC_RSP_REJ;
    }
    else
    {
        if ( code < AVRC_RSP_NOT_IMPL)
        {
            if (code == AVRC_CMD_NOTIF)
            {
               ctype = AVRC_RSP_INTERIM;
            }
            else if (code == AVRC_CMD_STATUS)
            {
               ctype = AVRC_RSP_IMPL_STBL;
            }
            else
            {
               ctype = AVRC_RSP_ACCEPT;
            }
        }
        else
        {
            ctype = code;
        }
    }
    /* if response is for register_notification, make sure the rc has
    actually registered for this */
#if defined(MTK_AVRCP_TG_15_BROWSE)&&(MTK_AVRCP_TG_15_BROWSE == TRUE)
    if ((pmetamsg_resp->rsp.pdu == AVRC_PDU_REGISTER_NOTIFICATION) &&
        ((code == AVRC_RSP_CHANGED) || (code == AVRC_RSP_INTERIM)))
#else
    if ((pmetamsg_resp->rsp.pdu == AVRC_PDU_REGISTER_NOTIFICATION) && (code == AVRC_RSP_CHANGED))
#endif
    {
        BOOLEAN bSent = FALSE;
        UINT8   event_id = pmetamsg_resp->reg_notif.event_id;
        BOOLEAN bNotify = (btif_rc_cb.rc_connected) && (btif_rc_cb.rc_notif[event_id-1].bNotify);

        /* de-register this notification for a CHANGED response */
        btif_rc_cb.rc_notif[event_id-1].bNotify = FALSE;
        BTIF_TRACE_DEBUG("%s rc_handle: %d. event_id: 0x%02d bNotify:%u", __FUNCTION__,
             btif_rc_cb.rc_handle, event_id, bNotify);
        if (bNotify)
        {
            BT_HDR *p_msg = NULL;
            tAVRC_STS status;

            if (AVRC_STS_NO_ERROR == (status = AVRC_BldResponse(btif_rc_cb.rc_handle,
                pmetamsg_resp, &p_msg)) )
            {
                BTIF_TRACE_DEBUG("%s Sending notification to rc_handle: %d. event_id: 0x%02d",
                     __FUNCTION__, btif_rc_cb.rc_handle, event_id);
                bSent = TRUE;
                BTA_AvMetaRsp(btif_rc_cb.rc_handle, btif_rc_cb.rc_notif[event_id-1].label,
                    ctype, p_msg);
            }
            else
            {
                BTIF_TRACE_WARNING("%s failed to build metamsg response. status: 0x%02x",
                    __FUNCTION__, status);
            }

        }

        if (!bSent)
        {
            BTIF_TRACE_DEBUG("%s: Notification not sent, as there are no RC connections or the \
                CT has not subscribed for event_id: %s", __FUNCTION__, dump_rc_notification_event_id(event_id));
        }
    }
    else
    {
        /* All other commands go here */

        BT_HDR *p_msg = NULL;
        tAVRC_STS status;

        status = AVRC_BldResponse(rc_handle, pmetamsg_resp, &p_msg);

        if (status == AVRC_STS_NO_ERROR)
        {
            BTA_AvMetaRsp(rc_handle, label, ctype, p_msg);
        }
        else
        {
            BTIF_TRACE_ERROR("%s: failed to build metamsg response. status: 0x%02x",
                __FUNCTION__, status);
        }
    }
}

static UINT8 opcode_from_pdu(UINT8 pdu)
{
    UINT8 opcode = 0;

    switch (pdu)
    {
#if defined(MTK_AVRCP_TG_15_BROWSE)&&(MTK_AVRCP_TG_15_BROWSE == TRUE)
    case AVRC_PDU_SET_BROWSED_PLAYER:
    case AVRC_PDU_GET_FOLDER_ITEMS:
    case AVRC_PDU_CHANGE_PATH:
    case AVRC_PDU_GET_ITEM_ATTRIBUTES:
    case AVRC_PDU_ADD_TO_NOW_PLAYING:
    case AVRC_PDU_SEARCH:
    case AVRC_PDU_GET_TOTAL_NUM_OF_ITEMS:
    case AVRC_PDU_GENERAL_REJECT:
        opcode = AVRC_OP_BROWSE;
        break;
#endif

    case AVRC_PDU_NEXT_GROUP:
    case AVRC_PDU_PREV_GROUP: /* pass thru */
        opcode  = AVRC_OP_PASS_THRU;
        break;

    default: /* vendor */
        opcode  = AVRC_OP_VENDOR;
        break;
    }

    return opcode;
}

/*******************************************************************************
**
** Function         btif_rc_upstreams_evt
**
** Description      Executes AVRC UPSTREAMS events in btif context.
**
** Returns          void
**
*******************************************************************************/
static void btif_rc_upstreams_evt(UINT16 event, tAVRC_COMMAND *pavrc_cmd, UINT8 ctype, UINT8 label)
{
    BTIF_TRACE_EVENT("%s pdu: %s handle: 0x%x ctype:%x label:%x", __FUNCTION__,
        dump_rc_pdu(pavrc_cmd->pdu), btif_rc_cb.rc_handle, ctype, label);

    switch (event)
    {
        case AVRC_PDU_GET_PLAY_STATUS:
        {
            FILL_PDU_QUEUE(IDX_GET_PLAY_STATUS_RSP, ctype, label, TRUE)
            HAL_CBACK(bt_rc_callbacks, get_play_status_cb);
        }
        break;
#if defined(MTK_AVRCP_TG_15_BROWSE)&&(MTK_AVRCP_TG_15_BROWSE == TRUE)
        case AVRC_PDU_LIST_PLAYER_APP_ATTR:
        {
            FILL_PDU_QUEUE(IDX_LIST_APP_ATTR_RSP, ctype, label, TRUE)
            HAL_CBACK(bt_rc_callbacks, list_player_app_attr_cb);
        }
        break;
        case AVRC_PDU_LIST_PLAYER_APP_VALUES:
        {
            BTIF_TRACE_DEBUG("AVRC_PDU_LIST_PLAYER_APP_VALUES =%d" ,pavrc_cmd->list_app_values.attr_id);
            if (pavrc_cmd->list_app_values.attr_id == 0)
            {
                send_reject_response (btif_rc_cb.rc_handle, label, pavrc_cmd->pdu,
                                    AVRC_STS_BAD_PARAM);
                break;
            }
            FILL_PDU_QUEUE(IDX_LIST_APP_VALUE_RSP, ctype, label, TRUE)
            HAL_CBACK(bt_rc_callbacks, list_player_app_values_cb,
                    pavrc_cmd->list_app_values.attr_id);
        }
        break;
        case AVRC_PDU_GET_CUR_PLAYER_APP_VALUE:
        {
            btrc_player_attr_t player_attr[BTRC_MAX_ELEM_ATTR_SIZE];
            UINT8 player_attr_num;
            BTIF_TRACE_DEBUG("PLAYER_APP_VALUE PDU 0x13 = %d",pavrc_cmd->get_cur_app_val.num_attr);
            if ((pavrc_cmd->get_cur_app_val.num_attr == 0) ||
                  (pavrc_cmd->get_cur_app_val.num_attr > BTRC_MAX_ELEM_ATTR_SIZE))
            {
                send_reject_response (btif_rc_cb.rc_handle, label, pavrc_cmd->pdu,
                                    AVRC_STS_BAD_PARAM);
                break;
            }
            memset( player_attr, 0, sizeof(player_attr));
            for (player_attr_num = 0 ; player_attr_num < pavrc_cmd->get_cur_app_val.num_attr;
                                                                            ++player_attr_num)
            {
                player_attr[player_attr_num] = pavrc_cmd->get_cur_app_val.attrs[player_attr_num];
            }
            FILL_PDU_QUEUE(IDX_GET_CURR_APP_VAL_RSP, ctype, label, TRUE)
            HAL_CBACK(bt_rc_callbacks, get_player_app_value_cb ,
                    pavrc_cmd->get_cur_app_val.num_attr, player_attr);
        }
        break;
        case AVRC_PDU_SET_PLAYER_APP_VALUE:
        {
            btrc_player_settings_t attr;
            UINT8 count;
            if ((pavrc_cmd->set_app_val.num_val== 0) ||
                              (pavrc_cmd->set_app_val.num_val > BTRC_MAX_ELEM_ATTR_SIZE))
            {
                send_reject_response (btif_rc_cb.rc_handle, label,
                                       pavrc_cmd->pdu, AVRC_STS_BAD_PARAM);
                break;
            }
            else
            {
                for(count = 0; count < pavrc_cmd->set_app_val.num_val ; ++count)
                {
                    attr.attr_ids[count] = pavrc_cmd->set_app_val.p_vals[count].attr_id ;
                    attr.attr_values[count]= pavrc_cmd->set_app_val.p_vals[count].attr_val;
                }
                attr.num_attr  =  pavrc_cmd->set_app_val.num_val ;
                FILL_PDU_QUEUE(IDX_SET_APP_VAL_RSP, ctype, label, TRUE)
                HAL_CBACK(bt_rc_callbacks, set_player_app_value_cb, &attr);
            }
        }
        break;
        case AVRC_PDU_GET_PLAYER_APP_ATTR_TEXT:
        {
            btrc_player_attr_t player_attr_txt [BTRC_MAX_ELEM_ATTR_SIZE];
            UINT8 count_txt = 0 ;
            if ((pavrc_cmd->get_app_attr_txt.num_attr == 0) ||
                   (pavrc_cmd->get_app_attr_txt.num_attr > BTRC_MAX_ELEM_ATTR_SIZE))
            {
                send_reject_response (btif_rc_cb.rc_handle, label, pavrc_cmd->pdu, AVRC_STS_BAD_PARAM);
            }
            else
            {
                for (count_txt = 0;count_txt < pavrc_cmd->get_app_attr_txt.num_attr ; ++count_txt)
                {
                    player_attr_txt[count_txt] = pavrc_cmd->get_app_attr_txt.attrs[count_txt];
                }
                FILL_PDU_QUEUE(IDX_GET_APP_ATTR_TXT_RSP, ctype, label, TRUE)
                HAL_CBACK(bt_rc_callbacks, get_player_app_attrs_text_cb,
                            pavrc_cmd->get_app_attr_txt.num_attr, player_attr_txt);
            }
        }
        break;
        case AVRC_PDU_GET_PLAYER_APP_VALUE_TEXT:
        {
            if (pavrc_cmd->get_app_val_txt.attr_id == 0 ||
                     pavrc_cmd->get_app_val_txt.attr_id > AVRC_PLAYER_VAL_GROUP_REPEAT)
            {
                send_reject_response (btif_rc_cb.rc_handle, label, pavrc_cmd->pdu, AVRC_STS_BAD_PARAM);
                break;
            }
            if (pavrc_cmd->get_app_val_txt.num_val == 0)
            {
                send_reject_response (btif_rc_cb.rc_handle, label, pavrc_cmd->pdu, AVRC_STS_BAD_PARAM);
            }
            else
            {
                FILL_PDU_QUEUE(IDX_GET_APP_VAL_TXT_RSP, ctype, label, TRUE)
                HAL_CBACK(bt_rc_callbacks, get_player_app_values_text_cb,
                          pavrc_cmd->get_app_val_txt.attr_id, pavrc_cmd->get_app_val_txt.num_val,
                          pavrc_cmd->get_app_val_txt.vals);
            }
        }
        break;
#else
        case AVRC_PDU_LIST_PLAYER_APP_ATTR:
        case AVRC_PDU_LIST_PLAYER_APP_VALUES:
        case AVRC_PDU_GET_CUR_PLAYER_APP_VALUE:
        case AVRC_PDU_SET_PLAYER_APP_VALUE:
        case AVRC_PDU_GET_PLAYER_APP_ATTR_TEXT:
        case AVRC_PDU_GET_PLAYER_APP_VALUE_TEXT:
        {
            /* TODO: Add support for Application Settings */
            send_reject_response (btif_rc_cb.rc_handle, label, pavrc_cmd->pdu, AVRC_STS_BAD_CMD);
        }
        break;
#endif //(end -- #if defined(MTK_AVRCP_TG_15_BROWSE))
        case AVRC_PDU_GET_ELEMENT_ATTR:
        {
            btrc_media_attr_t element_attrs[BTRC_MAX_ELEM_ATTR_SIZE];
            UINT8 num_attr;
             memset(&element_attrs, 0, sizeof(element_attrs));
            if (pavrc_cmd->get_elem_attrs.num_attr == 0)
            {
                /* CT requests for all attributes */
                int attr_cnt;
#if defined(MTK_AVRCP_TG_15_BROWSE)&&(MTK_AVRCP_TG_15_BROWSE == TRUE)\
    &&defined(MTK_PTS_AV_TEST)&&(MTK_PTS_AV_TEST == TRUE)
                num_attr = BTRC_MAX_ELEM_ATTR_SIZE-1; //not support cover art now for AVRCP/TG/MDI/BV-04-C
#else
                num_attr = BTRC_MAX_ELEM_ATTR_SIZE;
#endif
                for (attr_cnt = 0; attr_cnt < BTRC_MAX_ELEM_ATTR_SIZE; attr_cnt++)
                {
                    element_attrs[attr_cnt] = attr_cnt + 1;
                }
            }
            else if (pavrc_cmd->get_elem_attrs.num_attr == 0xFF)
            {
                /* 0xff indicates, no attributes requested - reject */
                send_reject_response (btif_rc_cb.rc_handle, label, pavrc_cmd->pdu,
                    AVRC_STS_BAD_PARAM);
                return;
            }
            else
            {
                int attr_cnt, filled_attr_count;

                num_attr = 0;
                /* Attribute IDs from 1 to AVRC_MAX_NUM_MEDIA_ATTR_ID are only valid,
                 * hence HAL definition limits the attributes to AVRC_MAX_NUM_MEDIA_ATTR_ID.
                 * Fill only valid entries.
                 */
                for (attr_cnt = 0; (attr_cnt < pavrc_cmd->get_elem_attrs.num_attr) &&
                    (num_attr < AVRC_MAX_NUM_MEDIA_ATTR_ID); attr_cnt++)
                {
                    if ((pavrc_cmd->get_elem_attrs.attrs[attr_cnt] > 0) &&
                        (pavrc_cmd->get_elem_attrs.attrs[attr_cnt] <= AVRC_MAX_NUM_MEDIA_ATTR_ID))
                    {
                        /* Skip the duplicate entries : PTS sends duplicate entries for Fragment cases
                         */
                        for (filled_attr_count = 0; filled_attr_count < num_attr; filled_attr_count++)
                        {
                            if (element_attrs[filled_attr_count] == pavrc_cmd->get_elem_attrs.attrs[attr_cnt])
                                break;
                        }
                        if (filled_attr_count == num_attr)
                        {
                            element_attrs[num_attr] = pavrc_cmd->get_elem_attrs.attrs[attr_cnt];
                            num_attr++;
                        }
                    }
                }
            }
            FILL_PDU_QUEUE(IDX_GET_ELEMENT_ATTR_RSP, ctype, label, TRUE);
            HAL_CBACK(bt_rc_callbacks, get_element_attr_cb, num_attr, element_attrs);
        }
        break;
        case AVRC_PDU_REGISTER_NOTIFICATION:
        {
            if (pavrc_cmd->reg_notif.event_id == BTRC_EVT_PLAY_POS_CHANGED &&
                pavrc_cmd->reg_notif.param == 0)
            {
                BTIF_TRACE_WARNING("%s Device registering position changed with illegal param 0.",
                    __FUNCTION__);
                send_reject_response (btif_rc_cb.rc_handle, label, pavrc_cmd->pdu, AVRC_STS_BAD_PARAM);
                /* de-register this notification for a rejected response */
                btif_rc_cb.rc_notif[BTRC_EVT_PLAY_POS_CHANGED - 1].bNotify = FALSE;
                return;
            }
#if defined(MTK_AV_VIRTUAL_BR_HAL_CB)&&(MTK_AV_VIRTUAL_BR_HAL_CB == FALSE)
            HAL_CBACK(bt_rc_callbacks, register_notification_cb, pavrc_cmd->reg_notif.event_id,
                      pavrc_cmd->reg_notif.param);
#else
            switch(pavrc_cmd->reg_notif.event_id)
            {
                case BTRC_EVT_ADDR_PLAYER_CHANGE:
                {
                    BTIF_TRACE_DEBUG("[pts] response reg. notification for AddrPlayerChange");
                    btrc_notification_type_t type = BTRC_NOTIFICATION_TYPE_INTERIM;
                    btrc_register_notification_t param;
                    param.addr_player_changed.player_id = 0;
                    param.addr_player_changed.uid_counter = 0;
                    register_notification_rsp(BTRC_EVT_ADDR_PLAYER_CHANGE,type,&param);

                }
                break;
                case BTRC_EVT_AVAL_PLAYER_CHANGE:
                {
                    BTIF_TRACE_DEBUG("[pts] response reg. notification for AvalPlayerChange");
                    btrc_notification_type_t type = BTRC_NOTIFICATION_TYPE_INTERIM;
                    register_notification_rsp(BTRC_EVT_AVAL_PLAYER_CHANGE,type,NULL);
                }
                break;
                case BTRC_EVT_UIDS_CHANGED:
                {
                    BTIF_TRACE_DEBUG("[pts] response reg. notification for UidsChanged");
                    btrc_notification_type_t type = BTRC_NOTIFICATION_TYPE_INTERIM;
                    btrc_register_notification_t param;
                    param.uids_changed.uid_counter = 0;
                    register_notification_rsp(BTRC_EVT_UIDS_CHANGED,type,NULL);

                }
                break;
                case BTRC_EVT_NOW_PLAYING_CONTENT_CHANGED:
                {
                    BTIF_TRACE_DEBUG("[pts] response reg. notification for NowPlayingChanged");
                    btrc_notification_type_t type = BTRC_NOTIFICATION_TYPE_INTERIM;
                    register_notification_rsp(BTRC_EVT_NOW_PLAYING_CONTENT_CHANGED,type,NULL);

                }
                break;
                case BTRC_EVT_APP_SETTINGS_CHANGED:
                {
                    BTIF_TRACE_DEBUG("[pts] response reg. notification for AppSettingsChanged");
                    btrc_notification_type_t type = BTRC_NOTIFICATION_TYPE_INTERIM;
                    btrc_register_notification_t param;
                    memset(&param,0x0,sizeof(btrc_register_notification_t));
                    param.player_setting.num_attr =1;
                    param.player_setting.attr_ids[0] = 0x01; //Equalizer on/off state
                    param.player_setting.attr_values[0] =0x01; //off

                    register_notification_rsp(BTRC_EVT_APP_SETTINGS_CHANGED,type,&param);

                }
                break;

                default:

                    HAL_CBACK(bt_rc_callbacks, register_notification_cb, pavrc_cmd->reg_notif.event_id,
                               pavrc_cmd->reg_notif.param);
                break;
            }

#endif
        }
        break;
        case AVRC_PDU_INFORM_DISPLAY_CHARSET:
        {
            tAVRC_RESPONSE avrc_rsp;
            BTIF_TRACE_EVENT("%s() AVRC_PDU_INFORM_DISPLAY_CHARSET", __FUNCTION__);
            if (btif_rc_cb.rc_connected == TRUE)
            {
                memset(&(avrc_rsp.inform_charset), 0, sizeof(tAVRC_RSP));
                avrc_rsp.inform_charset.opcode=opcode_from_pdu(AVRC_PDU_INFORM_DISPLAY_CHARSET);
                avrc_rsp.inform_charset.pdu=AVRC_PDU_INFORM_DISPLAY_CHARSET;
                avrc_rsp.inform_charset.status=AVRC_STS_NO_ERROR;
                send_metamsg_rsp(btif_rc_cb.rc_handle, label, ctype, &avrc_rsp);
            }
        }
        break;

#if defined(MTK_AVRCP_TG_15_BROWSE)&&(MTK_AVRCP_TG_15_BROWSE == TRUE)
        case AVRC_PDU_GET_FOLDER_ITEMS:
        {
            uint32_t attr_ids[BTRC_MAX_ELEM_ATTR_SIZE];
            uint8_t num_attr;
            num_attr = pavrc_cmd->get_items.attr_count;

            BTIF_TRACE_EVENT("%s: AVRC_PDU_GET_FOLDER_ITEMS num_attr: %d, start_item [%d] end_item [%d]",\
              __func__, num_attr, pavrc_cmd->get_items.start_item, pavrc_cmd->get_items.end_item);

            /* num_attr requested:
             *     0x00: All attributes requested
             *     0xFF: No Attributes requested
             *     0x01 to 0x07: Specified number of attributes
             */
            if ((num_attr != 0xFF && num_attr > BTRC_MAX_ELEM_ATTR_SIZE)) {
                send_reject_response(btif_rc_cb.rc_handle, label, pavrc_cmd->pdu,AVRC_STS_BAD_PARAM);
                return;
            }

            /* Except num_attr is None(0xff) / All(0x00), request follows with an
            * Attribute List */
            if ((num_attr != 0xFF) && (num_attr != 0x00)) {
                memcpy(attr_ids, pavrc_cmd->get_items.p_attr_list,sizeof(uint32_t) * num_attr);
            }

            FILL_PDU_QUEUE(IDX_GET_FOLDER_ITEMS_RSP, ctype, label, true);
#if defined(MTK_AV_SUPPORT_BR_HAL_CB)&&(MTK_AV_SUPPORT_BR_HAL_CB == TRUE)\
    &&(MTK_AV_VIRTUAL_BR_HAL_CB == FALSE)
            HAL_CBACK(bt_rc_callbacks, get_folder_items_cb,
                      pavrc_cmd->get_items.scope, pavrc_cmd->get_items.start_item,
                      pavrc_cmd->get_items.end_item, num_attr, attr_ids);
#else
            //to simulate the behavior from application
            btrc_status_t rsp_status = BTRC_STS_NO_ERROR;
            uint16_t uid_counter = 0;
            uint8_t num_items = 1;
            btrc_folder_items_t item;
            memset(&item,0x0,sizeof(item));
            BTIF_TRACE_DEBUG("[pts] auto response get_folder_items_list");

            item.item_type = AVRC_ITEM_PLAYER;
            item.player.charset_id = 0;
            item.player.major_type = 0;
            item.player.sub_type = 0;
            item.player.play_status = AVRC_PLAYSTATE_STOPPED;
            item.player.player_id = 0;
            strcpy((char*)item.player.name,"TT_MTK");
            get_folder_items_list_rsp(rsp_status, uid_counter,num_items, &item);
#endif
        } break;

        case AVRC_PDU_SET_ADDRESSED_PLAYER:
        {
            FILL_PDU_QUEUE(IDX_SET_ADDR_PLAYER_RSP, ctype, label, true);
#if defined(MTK_AV_SUPPORT_BR_HAL_CB)&&(MTK_AV_SUPPORT_BR_HAL_CB == TRUE)\
    &&(MTK_AV_VIRTUAL_BR_HAL_CB == FALSE)
            HAL_CBACK(bt_rc_callbacks, set_addressed_player_cb,
                      pavrc_cmd->addr_player.player_id);
#else
            //to simulate the behavior from application
            if(pavrc_cmd->addr_player.player_id == 0xffff){
                BTIF_TRACE_DEBUG("[pts] auto response bad player id");
                set_addressed_player_rsp(BTRC_STS_INV_PLAYER);
            }else{
                BTIF_TRACE_DEBUG("[pts] auto response no error");
                set_addressed_player_rsp(BTRC_STS_NO_ERROR);
            }
#endif
        } break;

        case AVRC_PDU_SET_BROWSED_PLAYER:
        {
            FILL_PDU_QUEUE(IDX_SET_BROWSED_PLAYER_RSP, ctype, label, true);
#if defined(MTK_AV_SUPPORT_BR_HAL_CB)&&(MTK_AV_SUPPORT_BR_HAL_CB == TRUE)
            HAL_CBACK(bt_rc_callbacks, set_browsed_player_cb,
                      pavrc_cmd->br_player.player_id);
#endif
        } break;

        case AVRC_PDU_REQUEST_CONTINUATION_RSP:
        {
            BTIF_TRACE_EVENT("%s() REQUEST CONTINUATION: target_pdu: 0x%02d",
                           __func__, pavrc_cmd->continu.target_pdu);
            tAVRC_RESPONSE avrc_rsp;
            if (btif_rc_cb.rc_connected == TRUE) {
                memset(&(avrc_rsp.continu), 0, sizeof(tAVRC_NEXT_RSP));
                avrc_rsp.continu.opcode =
                    opcode_from_pdu(AVRC_PDU_REQUEST_CONTINUATION_RSP);
                avrc_rsp.continu.pdu = AVRC_PDU_REQUEST_CONTINUATION_RSP;
                avrc_rsp.continu.status = AVRC_STS_NO_ERROR;
                avrc_rsp.continu.target_pdu = pavrc_cmd->continu.target_pdu;
                send_metamsg_rsp(btif_rc_cb.rc_handle, label, ctype, &avrc_rsp);
            }
        } break;

        case AVRC_PDU_ABORT_CONTINUATION_RSP:
        {
            BTIF_TRACE_EVENT("%s() ABORT CONTINUATION: target_pdu: 0x%02d", __func__,
                           pavrc_cmd->abort.target_pdu);
            tAVRC_RESPONSE avrc_rsp;
            if (btif_rc_cb.rc_connected == TRUE) {
                memset(&(avrc_rsp.abort), 0, sizeof(tAVRC_NEXT_RSP));
                avrc_rsp.abort.opcode =
                    opcode_from_pdu(AVRC_PDU_ABORT_CONTINUATION_RSP);
                avrc_rsp.abort.pdu = AVRC_PDU_ABORT_CONTINUATION_RSP;
                avrc_rsp.abort.status = AVRC_STS_NO_ERROR;
                avrc_rsp.abort.target_pdu = pavrc_cmd->continu.target_pdu;
                send_metamsg_rsp(btif_rc_cb.rc_handle, label, ctype, &avrc_rsp);
            }
        } break;

        case AVRC_PDU_CHANGE_PATH:
        {
            FILL_PDU_QUEUE(IDX_CHG_PATH_RSP, ctype, label, true);
#if defined(MTK_AV_SUPPORT_BR_HAL_CB)&&(MTK_AV_SUPPORT_BR_HAL_CB == TRUE)
            HAL_CBACK(bt_rc_callbacks, change_path_cb, pavrc_cmd->chg_path.direction,
                      pavrc_cmd->chg_path.folder_uid);
#endif
        } break;

        case AVRC_PDU_SEARCH:
        {
            FILL_PDU_QUEUE(IDX_SEARCH_RSP, ctype, label, true);
#if defined(MTK_AV_SUPPORT_BR_HAL_CB)&&(MTK_AV_SUPPORT_BR_HAL_CB == TRUE)
            HAL_CBACK(bt_rc_callbacks, search_cb, pavrc_cmd->search.string.charset_id,
                      pavrc_cmd->search.string.str_len,
                      pavrc_cmd->search.string.p_str);
#endif
        } break;

        case AVRC_PDU_GET_ITEM_ATTRIBUTES:
        {
            btrc_media_attr_t item_attrs[BTRC_MAX_ELEM_ATTR_SIZE];
            uint8_t num_attr = fill_attribute_id_array(pavrc_cmd->get_attrs.attr_count,
                                                      (btrc_media_attr_t*)pavrc_cmd->get_attrs.p_attr_list,
                                                       BTRC_MAX_ELEM_ATTR_SIZE, item_attrs);
            if (num_attr == 0) {
                BTIF_TRACE_ERROR("%s: No valid attributes requested in GET_ITEM_ATTRIBUTES",__func__);
                send_reject_response(btif_rc_cb.rc_handle, label, pavrc_cmd->pdu,AVRC_STS_BAD_PARAM);
                return;
            }
            FILL_PDU_QUEUE(IDX_GET_ITEM_ATTR_RSP, ctype, label, true);
            BTIF_TRACE_DEBUG("%s: GET_ITEM_ATTRIBUTES: num_attr: %d", __func__,num_attr);
#if defined(MTK_AV_SUPPORT_BR_HAL_CB)&&(MTK_AV_SUPPORT_BR_HAL_CB == TRUE)
            HAL_CBACK(bt_rc_callbacks, get_item_attr_cb, pavrc_cmd->get_attrs.scope,
                      pavrc_cmd->get_attrs.uid, pavrc_cmd->get_attrs.uid_counter,
                      num_attr, item_attrs);
#endif
        } break;

        case AVRC_PDU_GET_TOTAL_NUM_OF_ITEMS:
        {
            FILL_PDU_QUEUE(IDX_GET_TOTAL_NUM_OF_ITEMS_RSP, ctype, label, true);

#if defined(MTK_AV_SUPPORT_BR_HAL_CB)&&(MTK_AV_SUPPORT_BR_HAL_CB == TRUE)\
    &&(MTK_AV_VIRTUAL_BR_HAL_CB == FALSE)
            HAL_CBACK(bt_rc_callbacks, get_total_num_of_items_cb,
                      pavrc_cmd->get_num_of_items.scope);
#else
            UINT32 uid_counter = 0;
            UINT32 num_items = 0;
            get_total_num_of_items_rsp(BTRC_STS_NO_ERROR, uid_counter, num_items);
#endif
        } break;

        case AVRC_PDU_ADD_TO_NOW_PLAYING:
        {
            FILL_PDU_QUEUE(IDX_ADD_TO_NOW_PLAYING_RSP, ctype, label, true);

#if defined(MTK_AV_SUPPORT_BR_HAL_CB)&&(MTK_AV_SUPPORT_BR_HAL_CB == TRUE)
            HAL_CBACK(bt_rc_callbacks, add_to_now_playing_cb,
                      pavrc_cmd->add_to_play.scope, pavrc_cmd->add_to_play.uid,
                      pavrc_cmd->add_to_play.uid_counter);
#endif
        } break;

        case AVRC_PDU_PLAY_ITEM:
        {
            FILL_PDU_QUEUE(IDX_PLAY_ITEM_RSP, ctype, label, true);

#if defined(MTK_AV_SUPPORT_BR_HAL_CB)&&(MTK_AV_SUPPORT_BR_HAL_CB == TRUE)
            HAL_CBACK(bt_rc_callbacks, play_item_cb, pavrc_cmd->play_item.scope,
                      pavrc_cmd->play_item.uid_counter, pavrc_cmd->play_item.uid);
#endif
        } break;
#endif //(end -- #if defined(MTK_AVRCP_TG_15_BROWSE))
        default:
        {
        send_reject_response (btif_rc_cb.rc_handle, label, pavrc_cmd->pdu,
            (pavrc_cmd->pdu == AVRC_PDU_SEARCH)?AVRC_STS_SEARCH_NOT_SUP:AVRC_STS_BAD_CMD);
        return;
        }
        break;
    }
}

#if (AVRC_CTLR_INCLUDED == TRUE)
/*******************************************************************************
**
** Function         btif_rc_ctrl_upstreams_rsp_cmd
**
** Description      Executes AVRC UPSTREAMS response events in btif context.
**
** Returns          void
**
*******************************************************************************/
static void btif_rc_ctrl_upstreams_rsp_cmd(UINT8 event, tAVRC_COMMAND *pavrc_cmd,
        UINT8 label)
{
    BTIF_TRACE_DEBUG("%s pdu: %s handle: 0x%x", __FUNCTION__,
        dump_rc_pdu(pavrc_cmd->pdu), btif_rc_cb.rc_handle);
    bt_bdaddr_t rc_addr;
    bdcpy(rc_addr.address, btif_rc_cb.rc_addr);
#if (AVRC_CTLR_INCLUDED == TRUE)
    switch (event)
    {
    case AVRC_PDU_SET_ABSOLUTE_VOLUME:
         HAL_CBACK(bt_rc_ctrl_callbacks,setabsvol_cmd_cb, &rc_addr,
                 pavrc_cmd->volume.volume, label);
         break;
    case AVRC_PDU_REGISTER_NOTIFICATION:
         if (pavrc_cmd->reg_notif.event_id == AVRC_EVT_VOLUME_CHANGE)
         {
             HAL_CBACK(bt_rc_ctrl_callbacks, registernotification_absvol_cb,
                    &rc_addr, label);
         }
         break;
    }
#endif
}
#endif

#if (AVRC_ADV_CTRL_INCLUDED == TRUE)
/*******************************************************************************
**
** Function         btif_rc_upstreams_rsp_evt
**
** Description      Executes AVRC UPSTREAMS response events in btif context.
**
** Returns          void
**
*******************************************************************************/
static void btif_rc_upstreams_rsp_evt(UINT16 event, tAVRC_RESPONSE *pavrc_resp, UINT8 ctype, UINT8 label)
{
    BTIF_TRACE_EVENT("%s pdu: %s handle: 0x%x ctype:%x label:%x", __FUNCTION__,
        dump_rc_pdu(pavrc_resp->pdu), btif_rc_cb.rc_handle, ctype, label);

    switch (event)
    {
        case AVRC_PDU_REGISTER_NOTIFICATION:
        {
             if (AVRC_RSP_CHANGED==ctype)
                 btif_rc_cb.rc_volume=pavrc_resp->reg_notif.param.volume;
             HAL_CBACK(bt_rc_callbacks, volume_change_cb, pavrc_resp->reg_notif.param.volume,ctype)
        }
        break;

        case AVRC_PDU_SET_ABSOLUTE_VOLUME:
        {
            BTIF_TRACE_DEBUG("%s Set absolute volume change event received: volume %d,ctype %d",
                             __FUNCTION__, pavrc_resp->volume.volume,ctype);
            if (AVRC_RSP_ACCEPT==ctype)
                btif_rc_cb.rc_volume=pavrc_resp->volume.volume;
            HAL_CBACK(bt_rc_callbacks,volume_change_cb,pavrc_resp->volume.volume,ctype)
        }
        break;

        default:
            return;
    }
}
#endif

#if defined(MTK_PTS_AV_TEST) && (MTK_PTS_AV_TEST == TRUE)

#include "osi/include/thread.h"
static bt_status_t send_passthrough_cmd(bt_bdaddr_t *bd_addr, uint8_t key_code, uint8_t key_state);
static bt_status_t get_element_attr_rsp(uint8_t num_attr, btrc_element_attr_val_t *p_attrs);
static bt_status_t register_notification_rsp(btrc_event_id_t event_id, btrc_notification_type_t type,
                                                btrc_register_notification_t *p_param);

static thread_t *pts_test_thread;

#if defined(MTK_AVRCP_TG_15_BROWSE)&&(MTK_AVRCP_TG_15_BROWSE == TRUE)

static bt_status_t btif_rc_pts_rejectAllRegNotification()
{
    UINT8 event_index;
    btrc_event_id_t event_id;
    tAVRC_RESPONSE avrc_rsp;
    CHECK_RC_CONNECTED

    memset(&(avrc_rsp.reg_notif), 0, sizeof(tAVRC_REG_NOTIF_RSP));
    avrc_rsp.pdu = AVRC_PDU_REGISTER_NOTIFICATION;

    for(event_index=0;event_index<MAX_RC_NOTIFICATIONS;event_index++)
    {
        if(btif_rc_cb.rc_notif[event_index].bNotify == true)
        {
            event_id = event_index+1;
            switch(event_index){
                case AVRC_EVT_PLAY_STATUS_CHANGE:
                case AVRC_EVT_TRACK_CHANGE:
                case AVRC_EVT_TRACK_REACHED_END:
                case AVRC_EVT_TRACK_REACHED_START:
                case AVRC_EVT_PLAY_POS_CHANGED:
                case AVRC_EVT_APP_SETTING_CHANGE:
                case AVRC_EVT_ADDR_PLAYER_CHANGE:
                {
                    avrc_rsp.reg_notif.event_id = event_id;
                    avrc_rsp.reg_notif.pdu = AVRC_PDU_REGISTER_NOTIFICATION;
                    avrc_rsp.reg_notif.opcode = opcode_from_pdu(AVRC_PDU_REGISTER_NOTIFICATION);
                    avrc_rsp.reg_notif.status = AVRC_STS_ADDR_PLAYER_CHG;

                }
                    break;
                default: //others don't need reject as addressed player changed
                    break;
                }

            /* Send the response. */
            send_metamsg_rsp(btif_rc_cb.rc_handle, btif_rc_cb.rc_notif[event_index].label,
                    AVRC_RSP_CHANGED, &avrc_rsp);
        }
    }

    return BT_STATUS_SUCCESS;
}
#endif //(end -- #if defined(MTK_AVRCP_TG_15_BROWSE))

static void btif_rc_pts_test_thread(UNUSED_ATTR void *context) {
    char value[PROPERTY_VALUE_MAX] = {0};
    bt_bdaddr_t rc_addr;

    /* properties
       [1] Use "setprop bluetooth.pts.avrcp.auto on" to trigger commands in auto mode.
           -- AVRCP/CT/PTH/BV-01-C
           -- AVRCP/CT/PTT/BV-01-I

       [2] Use "setprop bluetooth.pts.avrcp.cmd 00" to send any commands.(here 00 is hex value)

       [3] Use "setprop bluetooth.pts.avrcp.attr_rsp true" to trigger an attribute response with extra long size.
          --AVCTP/TG/FRA/BV-02-C
       [4] Use "setprop bluetooth.pts.avrcp.uni_cmd true" to send a UNIT INFO command.
    */

    while(1){
        if ((osi_property_get("bluetooth.pts.avrcp.auto", value, "off"))
            && (!strcmp(value, "on"))){

            BTIF_TRACE_DEBUG("[pts] run avrcp auto mode");

            //below command should be run
            UINT8 cmdTable[] = {BTA_AV_RC_PLAY,
                                BTA_AV_RC_PAUSE,
                                BTA_AV_RC_STOP,
                                BTA_AV_RC_REWIND,
                                BTA_AV_RC_FAST_FOR,
                                BTA_AV_RC_FORWARD,
                                BTA_AV_RC_BACKWARD
                                };
            char *pCmdTableStr[] = {"PLAY",
                                    "PAUSE",
                                    "STOP",
                                    "REWIND",
                                    "FAST_FOR",
                                    "FORWARD",
                                    "BACKWARD"
                    };
            UINT8 i =0,cmdSize = sizeof(cmdTable);
            for(i=0;i<cmdSize;i++){
                BTIF_TRACE_DEBUG("[pts] run avrcp cmd 0x%x(%s)",cmdTable[i],pCmdTableStr[i]);
                send_passthrough_cmd(&rc_addr, cmdTable[i], BTA_AV_STATE_PRESS);
                send_passthrough_cmd(&rc_addr, cmdTable[i], BTA_AV_STATE_RELEASE);
                sleep(5); //play/stop may need more time in PTS tool.
            }
            osi_property_set("bluetooth.pts.avrcp.auto","off");

        }
        else if ((osi_property_get("bluetooth.pts.avrcp.cmd", value, "00"))
            && (strcmp(value, "00"))){
            UINT8 key_code;
            ascii_2_hex(value,2,&key_code);
            BTIF_TRACE_DEBUG("[pts] get avrcp cmd 0x%x",key_code);

            send_passthrough_cmd(&rc_addr, key_code, BTA_AV_STATE_PRESS);
            send_passthrough_cmd(&rc_addr, key_code, BTA_AV_STATE_RELEASE);

            osi_property_set("bluetooth.pts.avrcp.cmd","00");

        }
        else if ((osi_property_get("bluetooth.pts.avrcp.uni_cmd", value, "false"))
            && (!strcmp(value, "true"))){
            // AVRCP/CT/ICC/BV-01-I
            BTIF_TRACE_DEBUG("[pts] send UNIT INFO cmd");

            rc_transaction_t *p_transaction=NULL;
            bt_status_t tran_status = get_transaction(&p_transaction);

            if (BT_STATUS_SUCCESS == tran_status && NULL != p_transaction){
                AVRC_UnitCmd(btif_rc_cb.rc_handle, p_transaction->lbl);
            }

            osi_property_set("bluetooth.pts.avrcp.uni_cmd","false");

        }
#if defined(MTK_AVRCP_TG_15_BROWSE)&&(MTK_AVRCP_TG_15_BROWSE == TRUE)
        else if ((osi_property_get("bluetooth.pts.avrcp.addr_player", value, "false"))
            && (!strcmp(value, "true"))){
            BTIF_TRACE_DEBUG("[pts] get addr player changed req");

            btrc_notification_type_t type = BTRC_NOTIFICATION_TYPE_CHANGED;
            btrc_register_notification_t param;
            param.addr_player_changed.player_id = 1;
            param.addr_player_changed.uid_counter = 0;
            register_notification_rsp(BTRC_EVT_ADDR_PLAYER_CHANGE,type,&param);

            btif_rc_pts_rejectAllRegNotification();

            osi_property_set("bluetooth.pts.avrcp.addr_player","false");

        }
        else if ((osi_property_get("bluetooth.pts.avrcp.aval_player", value, "false"))
            && (!strcmp(value, "true"))){
            BTIF_TRACE_DEBUG("[pts] get avaiable player changed req");

            btrc_notification_type_t type = BTRC_NOTIFICATION_TYPE_CHANGED;
            btrc_register_notification_t param;
            param.addr_player_changed.player_id = 1;
            param.addr_player_changed.uid_counter = 0;
            register_notification_rsp(BTRC_EVT_AVAL_PLAYER_CHANGE,type,&param);

            btif_rc_pts_rejectAllRegNotification();

            osi_property_set("bluetooth.pts.avrcp.aval_player","false");

        }
#endif //(end -- #if defined(MTK_AVRCP_TG_15_BROWSE))
        else if ((osi_property_get("bluetooth.pts.avrcp.attr_rsp", value, "false"))
            && (!strcmp(value, "true"))){
            //AVCTP/TG/FRA/BV-02-C
            BTIF_TRACE_DEBUG("[pts] get get_element_attr_rsp req");

            UINT8 num_attr = 8;
            btrc_element_attr_val_t attrs[num_attr];

            memset(attrs,0x0,sizeof(attrs));
            attrs[1].attr_id = 1; //Title
            memset(attrs[1].text,0x65,sizeof(attrs[1].text)-1);
            attrs[2].attr_id = 2; //Artist Name
            memset(attrs[2].text,0x66,sizeof(attrs[1].text)-1);
            attrs[3].attr_id = 3; //Album Name
            memset(attrs[3].text,0x67,sizeof(attrs[1].text)-1);
            attrs[4].attr_id = 4; //Track Number
            attrs[4].text[0] = 0x49;attrs[4].text[1] = 0x50;
            attrs[5].attr_id = 5; //Total Number of Tracs
            attrs[5].text[0] = 0x49;attrs[5].text[1] = 0x50;
            attrs[6].attr_id = 6; //Genre
            memset(attrs[6].text,0x68,sizeof(attrs[1].text)-1);
            attrs[7].attr_id = 7; //Playing time

            get_element_attr_rsp(num_attr, attrs);

            osi_property_set("bluetooth.pts.avrcp.attr_rsp","false");

        }
        else{
            //unknow
        }
        sleep(1);
    }
}

static void btif_rc_pts_test_init()
{
    //1. create a thread to check property
    // 2. check the property and do sth.
    pts_test_thread = thread_new("pts_avrcp_test");
    BTIF_TRACE_EVENT("[pts] start %s()",__func__);

    thread_post(pts_test_thread, btif_rc_pts_test_thread, NULL);
}
#endif //(end #ifdef MTK_PTS_AV_TEST

/************************************************************************************
**  AVRCP API Functions
************************************************************************************/

/*******************************************************************************
**
** Function         init
**
** Description      Initializes the AVRC interface
**
** Returns          bt_status_t
**
*******************************************************************************/
static bt_status_t init(btrc_callbacks_t* callbacks )
{
    BTIF_TRACE_EVENT("## %s ##", __FUNCTION__);
    bt_status_t result = BT_STATUS_SUCCESS;

    if (bt_rc_callbacks)
        return BT_STATUS_DONE;

    bt_rc_callbacks = callbacks;
    memset (&btif_rc_cb, 0, sizeof(btif_rc_cb));
    btif_rc_cb.rc_vol_label=MAX_LABEL;
    btif_rc_cb.rc_volume=MAX_VOLUME;
    lbl_init();

#if defined(MTK_PTS_AV_TEST) && (MTK_PTS_AV_TEST == TRUE)
        btif_rc_pts_test_init();
#endif

    return result;
}

/*******************************************************************************
**
** Function         init_ctrl
**
** Description      Initializes the AVRC interface
**
** Returns          bt_status_t
**
*******************************************************************************/
static bt_status_t init_ctrl(btrc_ctrl_callbacks_t* callbacks )
{
    BTIF_TRACE_EVENT("## %s ##", __FUNCTION__);
    bt_status_t result = BT_STATUS_SUCCESS;

    if (bt_rc_ctrl_callbacks)
        return BT_STATUS_DONE;

    bt_rc_ctrl_callbacks = callbacks;
    memset (&btif_rc_cb, 0, sizeof(btif_rc_cb));
    btif_rc_cb.rc_vol_label=MAX_LABEL;
    btif_rc_cb.rc_volume=MAX_VOLUME;
    lbl_init();

#if defined(MTK_PTS_AV_TEST) && (MTK_PTS_AV_TEST == TRUE)
    btif_rc_pts_test_init();
#endif

    return result;
}

static void rc_ctrl_procedure_complete ()
{
    if (btif_rc_cb.rc_procedure_complete == TRUE)
    {
        return;
    }
    btif_rc_cb.rc_procedure_complete = TRUE;
    UINT32 attr_list[] = {
            AVRC_MEDIA_ATTR_ID_TITLE,
            AVRC_MEDIA_ATTR_ID_ARTIST,
            AVRC_MEDIA_ATTR_ID_ALBUM,
            AVRC_MEDIA_ATTR_ID_TRACK_NUM,
            AVRC_MEDIA_ATTR_ID_NUM_TRACKS,
            AVRC_MEDIA_ATTR_ID_GENRE,
            AVRC_MEDIA_ATTR_ID_PLAYING_TIME
            };
    get_element_attribute_cmd (AVRC_MAX_NUM_MEDIA_ATTR_ID, attr_list);
}

/***************************************************************************
**
** Function         get_play_status_rsp
**
** Description      Returns the current play status.
**                      This method is called in response to
**                      GetPlayStatus request.
**
** Returns          bt_status_t
**
***************************************************************************/
static bt_status_t get_play_status_rsp(btrc_play_status_t play_status, uint32_t song_len,
    uint32_t song_pos)
{
    tAVRC_RESPONSE avrc_rsp;
    CHECK_RC_CONNECTED
    memset(&(avrc_rsp.get_play_status), 0, sizeof(tAVRC_GET_PLAY_STATUS_RSP));
    avrc_rsp.get_play_status.song_len = song_len;
#if defined(MTK_STACK_CONFIG_BL) && (MTK_STACK_CONFIG_BL == TRUE)
    if (btif_rc_cb.rc_disalbe_song_pos == TRUE)
    {
        avrc_rsp.get_play_status.song_pos = (UINT32)-1;
    }
    else
    {
        avrc_rsp.get_play_status.song_pos = song_pos;
    }
#else
    avrc_rsp.get_play_status.song_pos = song_pos;
#endif
    avrc_rsp.get_play_status.play_status = play_status;

    avrc_rsp.get_play_status.pdu = AVRC_PDU_GET_PLAY_STATUS;
    avrc_rsp.get_play_status.opcode = opcode_from_pdu(AVRC_PDU_GET_PLAY_STATUS);
    avrc_rsp.get_play_status.status = AVRC_STS_NO_ERROR;
    /* Send the response */
    SEND_METAMSG_RSP(IDX_GET_PLAY_STATUS_RSP, &avrc_rsp);
    return BT_STATUS_SUCCESS;
}

/***************************************************************************
**
** Function         get_element_attr_rsp
**
** Description      Returns the current songs' element attributes
**                      in text.
**
** Returns          bt_status_t
**
***************************************************************************/
static bt_status_t get_element_attr_rsp(uint8_t num_attr, btrc_element_attr_val_t *p_attrs)
{
    tAVRC_RESPONSE avrc_rsp;
    UINT32 i;
    tAVRC_ATTR_ENTRY element_attrs[BTRC_MAX_ELEM_ATTR_SIZE];
    CHECK_RC_CONNECTED
    memset(element_attrs, 0, sizeof(tAVRC_ATTR_ENTRY) * num_attr);

    if (num_attr == 0)
    {
        avrc_rsp.get_play_status.status = AVRC_STS_BAD_PARAM;
    }
    else
    {
#if defined(MTK_STACK_CONFIG_BL) && (MTK_STACK_CONFIG_BL == TRUE)
        {
            UINT32 j = 0;

            for (i=0; i<num_attr; i++)
            {
                if ((btif_rc_cb.rc_disalbe_song_pos == TRUE) && (AVRC_MEDIA_ATTR_ID_PLAYING_TIME == p_attrs[i].attr_id))
                {
                    BTIF_TRACE_DEBUG("%s skip get_element_attr_rsp invalid attr 0x%08x",
                            __FUNCTION__, p_attrs[i].attr_id);
                    continue;
                }
                element_attrs[i].attr_id = p_attrs[i].attr_id;
                element_attrs[i].name.charset_id = AVRC_CHARSET_ID_UTF8;
                element_attrs[i].name.str_len = (UINT16)strlen((char *)p_attrs[i].text);
                element_attrs[i].name.p_str = p_attrs[i].text;
                BTIF_TRACE_DEBUG("%s attr_id:0x%x, charset_id:0x%x, str_len:%d, str:%s",
                        __FUNCTION__, (unsigned int)element_attrs[i].attr_id,
                        element_attrs[i].name.charset_id, element_attrs[i].name.str_len,
                        element_attrs[i].name.p_str);
                j++;
            }
            num_attr = j;
        }
#else
        for (i=0; i<num_attr; i++) {
            element_attrs[i].attr_id = p_attrs[i].attr_id;
            element_attrs[i].name.charset_id = AVRC_CHARSET_ID_UTF8;
            element_attrs[i].name.str_len = (UINT16)strlen((char *)p_attrs[i].text);
            element_attrs[i].name.p_str = p_attrs[i].text;
            BTIF_TRACE_DEBUG("%s attr_id:0x%x, charset_id:0x%x, str_len:%d, str:%s",
                             __FUNCTION__, (unsigned int)element_attrs[i].attr_id,
                             element_attrs[i].name.charset_id, element_attrs[i].name.str_len,
                             element_attrs[i].name.p_str);
        }
#endif
        avrc_rsp.get_play_status.status = AVRC_STS_NO_ERROR;
    }
    avrc_rsp.get_elem_attrs.num_attr = num_attr;
    avrc_rsp.get_elem_attrs.p_attrs = element_attrs;
    avrc_rsp.get_elem_attrs.pdu = AVRC_PDU_GET_ELEMENT_ATTR;
    avrc_rsp.get_elem_attrs.opcode = opcode_from_pdu(AVRC_PDU_GET_ELEMENT_ATTR);
    /* Send the response */
    SEND_METAMSG_RSP(IDX_GET_ELEMENT_ATTR_RSP, &avrc_rsp);
    return BT_STATUS_SUCCESS;
}

/***************************************************************************
**
** Function         register_notification_rsp
**
** Description      Response to the register notification request.
**                      in text.
**
** Returns          bt_status_t
**
***************************************************************************/
static bt_status_t register_notification_rsp(btrc_event_id_t event_id,
    btrc_notification_type_t type, btrc_register_notification_t *p_param)
{
    tAVRC_RESPONSE avrc_rsp;
    CHECK_RC_CONNECTED
    BTIF_TRACE_EVENT("## %s ## event_id:%s", __FUNCTION__, dump_rc_notification_event_id(event_id));
    if (btif_rc_cb.rc_notif[event_id-1].bNotify == FALSE)
    {
        BTIF_TRACE_ERROR("Avrcp Event id not registered: event_id = %x", event_id);
        return BT_STATUS_NOT_READY;
    }
    memset(&(avrc_rsp.reg_notif), 0, sizeof(tAVRC_REG_NOTIF_RSP));
    avrc_rsp.reg_notif.event_id = event_id;

    switch(event_id)
    {
        case BTRC_EVT_PLAY_STATUS_CHANGED:
            avrc_rsp.reg_notif.param.play_status = p_param->play_status;
            if (avrc_rsp.reg_notif.param.play_status == PLAY_STATUS_PLAYING)
                btif_av_clear_remote_suspend_flag();
            break;
        case BTRC_EVT_TRACK_CHANGE:
            memcpy(&(avrc_rsp.reg_notif.param.track), &(p_param->track), sizeof(btrc_uid_t));
            break;
        case BTRC_EVT_PLAY_POS_CHANGED:
            avrc_rsp.reg_notif.param.play_pos = p_param->song_pos;
            break;
#if defined(MTK_AVRCP_TG_15_BROWSE)&&(MTK_AVRCP_TG_15_BROWSE == TRUE)
        case BTRC_EVT_AVAL_PLAYER_CHANGE:
            break;
        case BTRC_EVT_ADDR_PLAYER_CHANGE:
            avrc_rsp.reg_notif.param.addr_player.player_id =
                p_param->addr_player_changed.player_id;
            avrc_rsp.reg_notif.param.addr_player.uid_counter =
                p_param->addr_player_changed.uid_counter;
        break;
        case BTRC_EVT_UIDS_CHANGED:
            avrc_rsp.reg_notif.param.uid_counter =
                p_param->uids_changed.uid_counter;
            break;
        case BTRC_EVT_NOW_PLAYING_CONTENT_CHANGED:
            break;
        case BTRC_EVT_APP_SETTINGS_CHANGED:
            avrc_rsp.reg_notif.param.player_setting.num_attr = p_param->player_setting.num_attr;

            memcpy(avrc_rsp.reg_notif.param.player_setting.attr_id,
                                       p_param->player_setting.attr_ids, AVRC_MAX_APP_SETTINGS);
            memcpy(avrc_rsp.reg_notif.param.player_setting.attr_value,
                                       p_param->player_setting.attr_values, AVRC_MAX_APP_SETTINGS);
            break;
#endif //(end -- #if defined(MTK_AVRCP_TG_15_BROWSE))
        default:
            BTIF_TRACE_WARNING("%s : Unhandled event ID : 0x%x", __FUNCTION__, event_id);
            return BT_STATUS_UNHANDLED;
    }

    avrc_rsp.reg_notif.pdu = AVRC_PDU_REGISTER_NOTIFICATION;
    avrc_rsp.reg_notif.opcode = opcode_from_pdu(AVRC_PDU_REGISTER_NOTIFICATION);
    avrc_rsp.get_play_status.status = AVRC_STS_NO_ERROR;

    /* Send the response. */
    send_metamsg_rsp(btif_rc_cb.rc_handle, btif_rc_cb.rc_notif[event_id-1].label,
        ((type == BTRC_NOTIFICATION_TYPE_INTERIM)?AVRC_CMD_NOTIF:AVRC_RSP_CHANGED), &avrc_rsp);
    return BT_STATUS_SUCCESS;
}

#if defined(MTK_AVRCP_TG_15_BROWSE)&&(MTK_AVRCP_TG_15_BROWSE == TRUE)
/***************************************************************************
 *
 * Function         get_folder_items_list_rsp
 *
 * Description      Returns the list of media items in current folder along with
 *                  requested attributes. This is called in response to
 *                  GetFolderItems request.
 *
 * Returns          bt_status_t
 *                      BT_STATUS_NOT_READY - when RC is not connected.
 *                      BT_STATUS_SUCCESS   - always if RC is connected
 *                      BT_STATUS_UNHANDLED - when rsp is not pending for
 *                                            get_folder_items_list PDU
 *
 **************************************************************************/
static bt_status_t get_folder_items_list_rsp(btrc_status_t rsp_status,
                                             uint16_t uid_counter,
                                             uint8_t num_items,
                                             btrc_folder_items_t* p_items) {
  tAVRC_RESPONSE avrc_rsp;
  tAVRC_ITEM item;
  tBTA_AV_CODE code = 0, ctype = 0;
  BT_HDR* p_msg = NULL;
  int item_cnt;
  tAVRC_STS status = AVRC_STS_NO_ERROR;
  btrc_folder_items_t* cur_item = NULL;

  BTIF_TRACE_DEBUG("%s: uid_counter %d num_items %d", __func__, uid_counter,
                   num_items);
  CHECK_RC_CONNECTED

  /* check if rsp to previous cmd was completed */
  if ( btif_rc_cb.rc_pdu_info[IDX_GET_FOLDER_ITEMS_RSP].is_rsp_pending == false) {
    BTIF_TRACE_WARNING("%s: Not sending response as no PDU was registered",
                       __func__);
    return BT_STATUS_UNHANDLED;
  }

  memset(&avrc_rsp, 0, sizeof(tAVRC_RESPONSE));
  memset(&item, 0, sizeof(tAVRC_ITEM));

  avrc_rsp.get_items.pdu = AVRC_PDU_GET_FOLDER_ITEMS;
  avrc_rsp.get_items.opcode = opcode_from_pdu(AVRC_PDU_GET_FOLDER_ITEMS);
  avrc_rsp.get_items.status = status_code_map[rsp_status];

  if (avrc_rsp.get_items.status != AVRC_STS_NO_ERROR) {
    BTIF_TRACE_WARNING(
        "%s: Error in parsing the received getfolderitems cmd. status: 0x%02x",
        __func__, avrc_rsp.get_items.status);
    status = avrc_rsp.get_items.status;
  } else {
    avrc_rsp.get_items.uid_counter = uid_counter;
    avrc_rsp.get_items.item_count = 1;

    /* create single item and build response iteratively for all num_items */
    for (item_cnt = 0; item_cnt < num_items; item_cnt++) {
      cur_item = &p_items[item_cnt];
      item.item_type = p_items->item_type;
      /* build respective item based on item_type. All items should be of same
       * type within
       * a response */
      switch (p_items->item_type) {
        case AVRC_ITEM_PLAYER: {
          item.u.player.name.charset_id = cur_item->player.charset_id;
          memcpy(&(item.u.player.features), &(cur_item->player.features),
                 sizeof(cur_item->player.features));
          item.u.player.major_type = cur_item->player.major_type;
          item.u.player.sub_type = cur_item->player.sub_type;
          item.u.player.play_status = cur_item->player.play_status;
          item.u.player.player_id = cur_item->player.player_id;
          item.u.player.name.p_str = cur_item->player.name;
          item.u.player.name.str_len =
              (uint16_t)strlen((char*)(cur_item->player.name));
        } break;

        case AVRC_ITEM_FOLDER: {
          memcpy(item.u.folder.uid, cur_item->folder.uid, sizeof(tAVRC_UID));
          item.u.folder.type = cur_item->folder.type;
          item.u.folder.playable = cur_item->folder.playable;
          item.u.folder.name.charset_id = AVRC_CHARSET_ID_UTF8;
          item.u.folder.name.str_len = strlen((char*)cur_item->folder.name);
          item.u.folder.name.p_str = cur_item->folder.name;
        } break;

        case AVRC_ITEM_MEDIA: {
          tAVRC_ATTR_ENTRY attr_vals[BTRC_MAX_ELEM_ATTR_SIZE];

          memcpy(item.u.media.uid, cur_item->media.uid, sizeof(tAVRC_UID));
          item.u.media.type = cur_item->media.type;
          item.u.media.name.charset_id = cur_item->media.charset_id;
          item.u.media.name.str_len = strlen((char*)cur_item->media.name);
          item.u.media.name.p_str = cur_item->media.name;
          item.u.media.attr_count = cur_item->media.num_attrs;

          /* Handle attributes of given item */
          if (item.u.media.attr_count == 0) {
            item.u.media.p_attr_list = NULL;
          } else {
            memset(&attr_vals, 0,
                   sizeof(tAVRC_ATTR_ENTRY) * BTRC_MAX_ELEM_ATTR_SIZE);
            fill_avrc_attr_entry(attr_vals, item.u.media.attr_count,
                                 cur_item->media.p_attrs);
            item.u.media.p_attr_list = attr_vals;
          }
        } break;

        default: {
          BTIF_TRACE_ERROR("%s: Unknown item_type: %d. Internal Error",
                           __func__, p_items->item_type);
          status = AVRC_STS_INTERNAL_ERR;
        } break;
      }

      avrc_rsp.get_items.p_item_list = &item;

      /* Add current item to buffer and build response if no error in item type
       */
      if (status != AVRC_STS_NO_ERROR) {
        /* Reject response due to error occured for unknown item_type, break the
         * loop */
        break;
      }

      int len_before = p_msg ? p_msg->len : 0;
      BTIF_TRACE_DEBUG("%s: item_cnt: %d len: %d", __func__, item_cnt,
                       len_before);
      status = AVRC_BldResponse( btif_rc_cb.rc_handle, &avrc_rsp, &p_msg);
      BTIF_TRACE_DEBUG("%s: Build rsp status: %d len: %d", __func__, status,
                       (p_msg ? p_msg->len : 0));
      int len_after = p_msg ? p_msg->len : 0;
      if (status != AVRC_STS_NO_ERROR || len_before == len_after) {
        /* Error occured in build response or we ran out of buffer so break the
         * loop */
        break;
      }
    }

    /* setting the error status */
    avrc_rsp.get_items.status = status;
  }

  /* if packet built successfully, send the built items to BTA layer */
  if (status == AVRC_STS_NO_ERROR) {
    code =  btif_rc_cb.rc_pdu_info[IDX_GET_FOLDER_ITEMS_RSP].ctype;
    ctype = get_rsp_type_code(avrc_rsp.get_items.status, code);
    BTA_AvMetaRsp( btif_rc_cb.rc_handle,
                   btif_rc_cb.rc_pdu_info[IDX_GET_FOLDER_ITEMS_RSP].label, ctype,
                  p_msg);
  } else /* Error occured, send reject response */
  {
    BTIF_TRACE_ERROR("%s: Error status: 0x%02X. Sending reject rsp", __func__,
                     avrc_rsp.rsp.status);
    send_reject_response(
         btif_rc_cb.rc_handle,  btif_rc_cb.rc_pdu_info[IDX_GET_FOLDER_ITEMS_RSP].label,
        avrc_rsp.pdu, avrc_rsp.get_items.status);
  }

  /* Reset values for current pdu. */
  btif_rc_cb.rc_pdu_info[IDX_GET_FOLDER_ITEMS_RSP].ctype = 0;
  btif_rc_cb.rc_pdu_info[IDX_GET_FOLDER_ITEMS_RSP].label = 0;
  btif_rc_cb.rc_pdu_info[IDX_GET_FOLDER_ITEMS_RSP].is_rsp_pending = false;

  return status == AVRC_STS_NO_ERROR ? BT_STATUS_SUCCESS : BT_STATUS_FAIL;
}

/***************************************************************************
 *
 * Function         set_addressed_player_rsp
 *
 * Description      Response to set the addressed player for specified media
 *                  player based on id in the media player list.
 *
 * Returns          bt_status_t
 *                      BT_STATUS_NOT_READY - when RC is not connected.
 *                      BT_STATUS_SUCCESS   - always if RC is connected
 *
 **************************************************************************/
static bt_status_t set_addressed_player_rsp(btrc_status_t rsp_status) {
  tAVRC_RESPONSE avrc_rsp;

  BTIF_TRACE_DEBUG("%s", __func__);
  CHECK_RC_CONNECTED

  avrc_rsp.addr_player.pdu = AVRC_PDU_SET_ADDRESSED_PLAYER;
  avrc_rsp.addr_player.opcode = opcode_from_pdu(AVRC_PDU_SET_ADDRESSED_PLAYER);
  avrc_rsp.addr_player.status = status_code_map[rsp_status];

  /* Send the response. */
  SEND_METAMSG_RSP(IDX_SET_ADDR_PLAYER_RSP,
                   &avrc_rsp);

  return BT_STATUS_SUCCESS;
}

/***************************************************************************
 *
 * Function         set_browsed_player_rsp
 *
 * Description      Response to set the browsed player command which contains
 *                  current browsed path of the media player. By default,
 *                  current_path = root and folder_depth = 0 for
 *                  every set_browsed_player request.
 *
 * Returns          bt_status_t
 *                      BT_STATUS_NOT_READY - when RC is not connected.
 *                      BT_STATUS_SUCCESS   - if RC is connected and reponse
 *                                            sent successfully
 *                      BT_STATUS_UNHANDLED - when rsp is not pending for
 *                                            set_browsed_player PDU
 *
 **************************************************************************/
static bt_status_t set_browsed_player_rsp(btrc_status_t rsp_status,
                                          uint32_t num_items,
                                          uint16_t charset_id,
                                          uint8_t folder_depth,
                                          btrc_br_folder_name_t* p_folders) {
  tAVRC_RESPONSE avrc_rsp;
  tAVRC_NAME item;
  BT_HDR* p_msg = NULL;
  tBTA_AV_CODE code = 0;
  tBTA_AV_CODE ctype = 0;
  unsigned int item_cnt;
  tAVRC_STS status = AVRC_STS_NO_ERROR;

  CHECK_RC_CONNECTED

  memset(&avrc_rsp, 0, sizeof(tAVRC_RESPONSE));
  memset(&item, 0, sizeof(tAVRC_NAME));

  avrc_rsp.br_player.status = status_code_map[rsp_status];
  avrc_rsp.br_player.pdu = AVRC_PDU_SET_BROWSED_PLAYER;
  avrc_rsp.br_player.opcode = opcode_from_pdu(AVRC_PDU_SET_BROWSED_PLAYER);

  BTIF_TRACE_DEBUG("%s: rsp_status: 0x%02X avrc_rsp.br_player.status: 0x%02X",
                   __func__, rsp_status, avrc_rsp.br_player.status);

  /* check if rsp to previous cmd was completed */
  if (btif_rc_cb.rc_pdu_info[IDX_SET_BROWSED_PLAYER_RSP].is_rsp_pending == false) {
    BTIF_TRACE_WARNING("%s: Not sending response as no PDU was registered",
                       __func__);
    return BT_STATUS_UNHANDLED;
  }

  if (AVRC_STS_NO_ERROR == avrc_rsp.get_items.status) {
    avrc_rsp.br_player.num_items = num_items;
    avrc_rsp.br_player.charset_id = charset_id;
    avrc_rsp.br_player.folder_depth = folder_depth;
    avrc_rsp.br_player.p_folders = (tAVRC_NAME*)p_folders;

    BTIF_TRACE_DEBUG("%s: folder_depth: 0x%02X num_items: %d", __func__,
                     folder_depth, num_items);

    if (folder_depth > 0) {
      /* Iteratively build response for all folders across folder depth upto
       * current path */
      avrc_rsp.br_player.folder_depth = 1;
      for (item_cnt = 0; item_cnt < folder_depth; item_cnt++) {
        BTIF_TRACE_DEBUG("%s: iteration: %d", __func__, item_cnt);
        item.str_len = p_folders[item_cnt].str_len;
        item.p_str = p_folders[item_cnt].p_str;
        avrc_rsp.br_player.p_folders = &item;

        /* Add current item to buffer and build response */
        status = AVRC_BldResponse(btif_rc_cb.rc_handle, &avrc_rsp, &p_msg);
        if (AVRC_STS_NO_ERROR != status) {
          BTIF_TRACE_WARNING("%s: Build rsp status: %d", __func__, status);
          /* if the build fails, it is likely that we ran out of buffer. so if
        * we have
        * some items to send, reset this error to no error for sending what we
        * have */
          if (item_cnt > 0) status = AVRC_STS_NO_ERROR;

          /* Error occured in build response so break the loop */
          break;
        }
      }
    } else /* current path is root folder, no folders navigated yet */
    {
      status = AVRC_BldResponse(btif_rc_cb.rc_handle, &avrc_rsp, &p_msg);
    }

    /* setting the error status */
    avrc_rsp.br_player.status = status;
  } else /* error received from above layer */
  {
    BTIF_TRACE_WARNING(
        "%s: Error in parsing the received setbrowsed command. status: 0x%02x",
        __func__, avrc_rsp.br_player.status);
    status = avrc_rsp.br_player.status;
  }

  /* if packet built successfully, send the built items to BTA layer */
  if (status == AVRC_STS_NO_ERROR) {
    code = btif_rc_cb.rc_pdu_info[IDX_SET_BROWSED_PLAYER_RSP].ctype;
    ctype = get_rsp_type_code(avrc_rsp.br_player.status, code);
    BTA_AvMetaRsp(btif_rc_cb.rc_handle,
                  btif_rc_cb.rc_pdu_info[IDX_SET_BROWSED_PLAYER_RSP].label, ctype,
                  p_msg);
  } else /* Error occured, send reject response */
  {
    BTIF_TRACE_ERROR("%s: Error status: 0x%02X. Sending reject rsp", __func__,
                     avrc_rsp.br_player.status);
    send_reject_response(
        btif_rc_cb.rc_handle, btif_rc_cb.rc_pdu_info[IDX_SET_BROWSED_PLAYER_RSP].label,
        avrc_rsp.pdu, avrc_rsp.br_player.status);
  }

  /* Reset values for set_browsed_player pdu.*/
  btif_rc_cb.rc_pdu_info[IDX_SET_BROWSED_PLAYER_RSP].ctype = 0;
  btif_rc_cb.rc_pdu_info[IDX_SET_BROWSED_PLAYER_RSP].label = 0;
  btif_rc_cb.rc_pdu_info[IDX_SET_BROWSED_PLAYER_RSP].is_rsp_pending = false;

  return status == AVRC_STS_NO_ERROR ? BT_STATUS_SUCCESS : BT_STATUS_FAIL;
}

/*******************************************************************************
 *
 * Function         change_path_rsp
 *
 * Description      Response to the change path command which
 *                  contains number of items in the changed path.
 *
 * Returns          bt_status_t
 *                      BT_STATUS_NOT_READY - when RC is not connected.
 *                      BT_STATUS_SUCCESS   - always if RC is connected
 *
 **************************************************************************/
static bt_status_t change_path_rsp(btrc_status_t rsp_status,
                                   uint32_t num_items) {
  tAVRC_RESPONSE avrc_rsp;

  BTIF_TRACE_DEBUG("%s", __func__);
  CHECK_RC_CONNECTED

  avrc_rsp.chg_path.pdu = AVRC_PDU_CHANGE_PATH;
  avrc_rsp.chg_path.opcode = opcode_from_pdu(AVRC_PDU_CHANGE_PATH);
  avrc_rsp.chg_path.num_items = num_items;
  avrc_rsp.chg_path.status = status_code_map[rsp_status];

  /* Send the response. */
  SEND_METAMSG_RSP(IDX_CHG_PATH_RSP, &avrc_rsp);

  return BT_STATUS_SUCCESS;
}

/***************************************************************************
 *
 * Function         search_rsp
 *
 * Description      Response to search a string from media content command.
 *
 * Returns          bt_status_t
 *                      BT_STATUS_NOT_READY - when RC is not connected.
 *                      BT_STATUS_SUCCESS   - always if RC is connected
 *
 **************************************************************************/
static bt_status_t search_rsp(btrc_status_t rsp_status,
                              uint32_t uid_counter, uint32_t num_items) {
  tAVRC_RESPONSE avrc_rsp;

  BTIF_TRACE_DEBUG("%s", __func__);
  CHECK_RC_CONNECTED

  avrc_rsp.search.pdu = AVRC_PDU_SEARCH;
  avrc_rsp.search.opcode = opcode_from_pdu(AVRC_PDU_SEARCH);
  avrc_rsp.search.num_items = num_items;
  avrc_rsp.search.uid_counter = uid_counter;
  avrc_rsp.search.status = status_code_map[rsp_status];

  /* Send the response. */
  SEND_METAMSG_RSP(IDX_SEARCH_RSP, &avrc_rsp);

  return BT_STATUS_SUCCESS;
}
/***************************************************************************
 *
 * Function         get_item_attr_rsp
 *
 * Description      Response to the get item's attributes command which
 *                  contains number of attributes and values list in text.
 *
 * Returns          bt_status_t
 *                      BT_STATUS_NOT_READY - when RC is not connected.
 *                      BT_STATUS_SUCCESS   - always if RC is connected
 *
 **************************************************************************/
static bt_status_t get_item_attr_rsp(btrc_status_t rsp_status, uint8_t num_attr,
                                     btrc_element_attr_val_t* p_attrs) {
  tAVRC_RESPONSE avrc_rsp;
  tAVRC_ATTR_ENTRY item_attrs[BTRC_MAX_ELEM_ATTR_SIZE];

  BTIF_TRACE_DEBUG("%s", __func__);
  CHECK_RC_CONNECTED

  memset(item_attrs, 0, sizeof(tAVRC_ATTR_ENTRY) * num_attr);

  avrc_rsp.get_attrs.status = status_code_map[rsp_status];
  if (rsp_status == BTRC_STS_NO_ERROR) {
    fill_avrc_attr_entry(item_attrs, num_attr, p_attrs);
  }

  avrc_rsp.get_attrs.attr_count = num_attr;
  avrc_rsp.get_attrs.p_attr_list = item_attrs;
  avrc_rsp.get_attrs.pdu = AVRC_PDU_GET_ITEM_ATTRIBUTES;
  avrc_rsp.get_attrs.opcode = opcode_from_pdu(AVRC_PDU_GET_ITEM_ATTRIBUTES);

  /* Send the response. */
  SEND_METAMSG_RSP(IDX_GET_ITEM_ATTR_RSP, &avrc_rsp);

  return BT_STATUS_SUCCESS;
}

/***************************************************************************
 *
 * Function         add_to_now_playing_rsp
 *
 * Description      Response to command for adding speciafied media item
 *                  to Now Playing queue.
 *
 * Returns          bt_status_t
 *                      BT_STATUS_NOT_READY - when RC is not connected.
 *                      BT_STATUS_SUCCESS   - always if RC is connected
 *
 **************************************************************************/
static bt_status_t add_to_now_playing_rsp(btrc_status_t rsp_status) {
  tAVRC_RESPONSE avrc_rsp;

  BTIF_TRACE_DEBUG("%s", __func__);
  CHECK_RC_CONNECTED

  avrc_rsp.add_to_play.pdu = AVRC_PDU_ADD_TO_NOW_PLAYING;
  avrc_rsp.add_to_play.opcode = opcode_from_pdu(AVRC_PDU_ADD_TO_NOW_PLAYING);
  avrc_rsp.add_to_play.status = status_code_map[rsp_status];

  /* Send the response. */
  SEND_METAMSG_RSP(IDX_ADD_TO_NOW_PLAYING_RSP,
                   &avrc_rsp);

  return BT_STATUS_SUCCESS;
}

/***************************************************************************
 *
 * Function         play_item_rsp
 *
 * Description      Response to command for playing the specified media item.
 *
 * Returns          bt_status_t
 *                      BT_STATUS_NOT_READY - when RC is not connected.
 *                      BT_STATUS_SUCCESS   - always if RC is connected
 *
 **************************************************************************/
static bt_status_t play_item_rsp(btrc_status_t rsp_status) {
  tAVRC_RESPONSE avrc_rsp;

  BTIF_TRACE_DEBUG("%s", __func__);
  CHECK_RC_CONNECTED

  avrc_rsp.play_item.pdu = AVRC_PDU_PLAY_ITEM;
  avrc_rsp.play_item.opcode = opcode_from_pdu(AVRC_PDU_PLAY_ITEM);
  avrc_rsp.play_item.status = status_code_map[rsp_status];

  /* Send the response. */
  SEND_METAMSG_RSP(IDX_PLAY_ITEM_RSP, &avrc_rsp);

  return BT_STATUS_SUCCESS;
}

/***************************************************************************
 *
 * Function         get_total_num_of_items_rsp
 *
 * Description      response to command to get the Number of Items
 *                  in the selected folder at the selected scope
 *
 * Returns          bt_status_t
 *                      BT_STATUS_NOT_READY - when RC is not connected.
 *                      BT_STATUS_SUCCESS   - always if RC is connected
 *
 **************************************************************************/
static bt_status_t get_total_num_of_items_rsp(btrc_status_t rsp_status,
                                              uint32_t uid_counter,
                                              uint32_t num_items) {
  tAVRC_RESPONSE avrc_rsp;

  BTIF_TRACE_DEBUG("%s", __func__);
  CHECK_RC_CONNECTED

  avrc_rsp.get_num_of_items.pdu = AVRC_PDU_GET_TOTAL_NUM_OF_ITEMS;
  avrc_rsp.get_num_of_items.opcode =
      opcode_from_pdu(AVRC_PDU_GET_TOTAL_NUM_OF_ITEMS);
  avrc_rsp.get_num_of_items.num_items = num_items;
  avrc_rsp.get_num_of_items.uid_counter = uid_counter;
  avrc_rsp.get_num_of_items.status = status_code_map[rsp_status];

  /* Send the response. */
  SEND_METAMSG_RSP(IDX_GET_TOTAL_NUM_OF_ITEMS_RSP,
                   &avrc_rsp);

  return BT_STATUS_SUCCESS;
}
#endif //(end -- #if defined(MTK_AVRCP_TG_15_BROWSE))

/***************************************************************************
**
** Function         set_volume
**
** Description      Send current volume setting to remote side.
**                  Support limited to SetAbsoluteVolume
**                  This can be enhanced to support Relative Volume (AVRCP 1.0).
**                  With RelateVolume, we will send VOLUME_UP/VOLUME_DOWN
**                  as opposed to absolute volume level
** volume: Should be in the range 0-127. bit7 is reseved and cannot be set
**
** Returns          bt_status_t
**
***************************************************************************/
static bt_status_t set_volume(uint8_t volume)
{
    BTIF_TRACE_DEBUG("%s", __FUNCTION__);
    CHECK_RC_CONNECTED
    tAVRC_STS status = BT_STATUS_UNSUPPORTED;
    rc_transaction_t *p_transaction=NULL;

    if (btif_rc_cb.rc_volume==volume)
    {
        status=BT_STATUS_DONE;
        BTIF_TRACE_ERROR("%s: volume value already set earlier: 0x%02x",__FUNCTION__, volume);
        return status;
    }

    if ((btif_rc_cb.rc_features & BTA_AV_FEAT_RCTG) &&
        (btif_rc_cb.rc_features & BTA_AV_FEAT_ADV_CTRL))
    {
        tAVRC_COMMAND avrc_cmd = {0};
        BT_HDR *p_msg = NULL;

        BTIF_TRACE_DEBUG("%s: Peer supports absolute volume. newVolume=%d", __FUNCTION__, volume);
        avrc_cmd.volume.opcode = AVRC_OP_VENDOR;
        avrc_cmd.volume.pdu = AVRC_PDU_SET_ABSOLUTE_VOLUME;
        avrc_cmd.volume.status = AVRC_STS_NO_ERROR;
        avrc_cmd.volume.volume = volume;

        if (AVRC_BldCommand(&avrc_cmd, &p_msg) == AVRC_STS_NO_ERROR)
        {
            bt_status_t tran_status=get_transaction(&p_transaction);
            if (BT_STATUS_SUCCESS == tran_status && NULL!=p_transaction)
            {
                BTIF_TRACE_DEBUG("%s msgreq being sent out with label %d",
                                   __FUNCTION__,p_transaction->lbl);
                BTA_AvMetaCmd(btif_rc_cb.rc_handle,p_transaction->lbl, AVRC_CMD_CTRL, p_msg);
                status =  BT_STATUS_SUCCESS;
            }
            else
            {
                osi_free(p_msg);
                BTIF_TRACE_ERROR("%s: failed to obtain transaction details. status: 0x%02x",
                                    __FUNCTION__, tran_status);
                status = BT_STATUS_FAIL;
            }
        }
        else
        {
            BTIF_TRACE_ERROR("%s: failed to build absolute volume command. status: 0x%02x",
                                __FUNCTION__, status);
            status = BT_STATUS_FAIL;
        }
    }
    else
        status=BT_STATUS_NOT_READY;
    return status;
}

#if (AVRC_ADV_CTRL_INCLUDED == TRUE)
/***************************************************************************
**
** Function         register_volumechange
**
** Description     Register for volume change notification from remote side.
**
** Returns          void
**
***************************************************************************/

static void register_volumechange (UINT8 lbl)
{
    tAVRC_COMMAND avrc_cmd = {0};
    BT_HDR *p_msg = NULL;
    tAVRC_STS BldResp=AVRC_STS_BAD_CMD;
    rc_transaction_t *p_transaction=NULL;

    BTIF_TRACE_DEBUG("%s called with label:%d",__FUNCTION__,lbl);

    avrc_cmd.cmd.opcode=0x00;
    avrc_cmd.pdu = AVRC_PDU_REGISTER_NOTIFICATION;
    avrc_cmd.reg_notif.event_id = AVRC_EVT_VOLUME_CHANGE;
    avrc_cmd.reg_notif.status = AVRC_STS_NO_ERROR;
    avrc_cmd.reg_notif.param = 0;

    BldResp=AVRC_BldCommand(&avrc_cmd, &p_msg);
    if (AVRC_STS_NO_ERROR == BldResp && p_msg) {
        p_transaction = get_transaction_by_lbl(lbl);
        if (p_transaction != NULL) {
            BTA_AvMetaCmd(btif_rc_cb.rc_handle, p_transaction->lbl,
                          AVRC_CMD_NOTIF, p_msg);
            BTIF_TRACE_DEBUG("%s:BTA_AvMetaCmd called", __func__);
         } else {
            osi_free(p_msg);
            BTIF_TRACE_ERROR("%s transaction not obtained with label: %d",
                             __func__, lbl);
         }
    } else {
        BTIF_TRACE_ERROR("%s failed to build command:%d", __func__, BldResp);
    }
}

/***************************************************************************
**
** Function         handle_rc_metamsg_rsp
**
** Description      Handle RC metamessage response
**
** Returns          void
**
***************************************************************************/
static void handle_rc_metamsg_rsp(tBTA_AV_META_MSG *pmeta_msg)
{
    tAVRC_RESPONSE    avrc_response = {0};
    UINT8             scratch_buf[512] = {0};
    tAVRC_STS status = BT_STATUS_UNSUPPORTED;

    if (AVRC_OP_VENDOR==pmeta_msg->p_msg->hdr.opcode &&(AVRC_RSP_CHANGED==pmeta_msg->code
      || AVRC_RSP_INTERIM==pmeta_msg->code || AVRC_RSP_ACCEPT==pmeta_msg->code
      || AVRC_RSP_REJ==pmeta_msg->code || AVRC_RSP_NOT_IMPL==pmeta_msg->code))
    {
        status=AVRC_ParsResponse(pmeta_msg->p_msg, &avrc_response, scratch_buf, sizeof(scratch_buf));
        BTIF_TRACE_DEBUG("%s: code %d,event ID %d,PDU %x,parsing status %d, label:%d",
          __FUNCTION__,pmeta_msg->code,avrc_response.reg_notif.event_id,avrc_response.reg_notif.pdu,
          status, pmeta_msg->label);

        if (status != AVRC_STS_NO_ERROR)
        {
            if (AVRC_PDU_REGISTER_NOTIFICATION==avrc_response.rsp.pdu
                && AVRC_EVT_VOLUME_CHANGE==avrc_response.reg_notif.event_id
                && btif_rc_cb.rc_vol_label==pmeta_msg->label)
            {
                btif_rc_cb.rc_vol_label=MAX_LABEL;
                release_transaction(btif_rc_cb.rc_vol_label);
            }
            else if (AVRC_PDU_SET_ABSOLUTE_VOLUME==avrc_response.rsp.pdu)
            {
                release_transaction(pmeta_msg->label);
            }
            return;
        }
        else if (AVRC_PDU_REGISTER_NOTIFICATION==avrc_response.rsp.pdu
            && AVRC_EVT_VOLUME_CHANGE==avrc_response.reg_notif.event_id
            && btif_rc_cb.rc_vol_label!=pmeta_msg->label)
            {
                // Just discard the message, if the device sends back with an incorrect label
                BTIF_TRACE_DEBUG("%s:Discarding register notfn in rsp.code: %d and label %d",
                __FUNCTION__, pmeta_msg->code, pmeta_msg->label);
                return;
            }
    }
    else
    {
        BTIF_TRACE_DEBUG("%s:Received vendor dependent in adv ctrl rsp. code: %d len: %d. Not processing it.",
        __FUNCTION__, pmeta_msg->code, pmeta_msg->len);
        return;
    }

    if (AVRC_PDU_REGISTER_NOTIFICATION==avrc_response.rsp.pdu
        && AVRC_EVT_VOLUME_CHANGE==avrc_response.reg_notif.event_id
        && AVRC_RSP_CHANGED==pmeta_msg->code)
     {
         /* re-register for volume change notification */
         // Do not re-register for rejected case, as it might get into endless loop
         register_volumechange(btif_rc_cb.rc_vol_label);
     }
     else if (AVRC_PDU_SET_ABSOLUTE_VOLUME==avrc_response.rsp.pdu)
     {
          /* free up the label here */
          release_transaction(pmeta_msg->label);
     }

     BTIF_TRACE_EVENT("%s: Passing received metamsg response to app. pdu: %s",
             __FUNCTION__, dump_rc_pdu(avrc_response.pdu));
     btif_rc_upstreams_rsp_evt((uint16_t)avrc_response.rsp.pdu, &avrc_response, pmeta_msg->code,
                                pmeta_msg->label);
}
#endif

#if (AVRC_CTLR_INCLUDED == TRUE)
/***************************************************************************
**
** Function         iterate_supported_event_list_for_interim_rsp
**
** Description      iterator callback function to match the event and handle
**                  timer cleanup
** Returns          true to continue iterating, false to stop
**
***************************************************************************/
bool iterate_supported_event_list_for_interim_rsp(void *data, void *cb_data)
{
    UINT8 *p_event_id;
    btif_rc_supported_event_t *p_event = (btif_rc_supported_event_t *)data;

    p_event_id = (UINT8*)cb_data;

    if (p_event->event_id == *p_event_id)
    {
        p_event->status = eINTERIM;
        return false;
    }
    return true;
}

/***************************************************************************
**
** Function         iterate_supported_event_list_for_timeout
**
** Description      Iterator callback function for timeout handling.
**                  As part of the failure handling, it releases the
**                  transaction label and removes the event from list,
**                  this event will not be requested again during
**                  the lifetime of the connection.
** Returns          false to stop iterating, true to continue
**
***************************************************************************/
bool iterate_supported_event_list_for_timeout(void *data, void *cb_data)
{
    UINT8 label;
    btif_rc_supported_event_t *p_event = (btif_rc_supported_event_t *)data;

    label = (*(UINT8*)cb_data) & 0xFF;

    if (p_event->label == label)
    {
        list_remove(btif_rc_cb.rc_supported_event_list, p_event);
        return false;
    }
    return true;
}

/***************************************************************************
**
** Function         rc_notification_interim_timout
**
** Description      Interim response timeout handler.
**                  Runs the iterator to check and clear the timed out event.
**                  Proceeds to register for the unregistered events.
** Returns          None
**
***************************************************************************/
static void rc_notification_interim_timout (UINT8 label)
{
    list_node_t *node;

    list_foreach(btif_rc_cb.rc_supported_event_list,
                     iterate_supported_event_list_for_timeout, &label);
    /* Timeout happened for interim response for the registered event,
     * check if there are any pending for registration
     */
    node = list_begin(btif_rc_cb.rc_supported_event_list);
    while (node != NULL)
    {
        btif_rc_supported_event_t *p_event;

        p_event = (btif_rc_supported_event_t *)list_node(node);
        if ((p_event != NULL) && (p_event->status == eNOT_REGISTERED))
        {
            register_for_event_notification(p_event);
            break;
        }
        node = list_next (node);
    }
    /* Todo. Need to initiate application settings query if this
     * is the last event registration.
     */
}

/***************************************************************************
**
** Function         btif_rc_status_cmd_timeout_handler
**
** Description      RC status command timeout handler (Runs in BTIF context).
** Returns          None
**
***************************************************************************/
static void btif_rc_status_cmd_timeout_handler(UNUSED_ATTR uint16_t event,
                                               char *data)
{
    btif_rc_timer_context_t *p_context;
    tAVRC_RESPONSE      avrc_response = {0};
    tBTA_AV_META_MSG    meta_msg;

    p_context = (btif_rc_timer_context_t *)data;
    memset(&meta_msg, 0, sizeof(tBTA_AV_META_MSG));
    meta_msg.rc_handle = btif_rc_cb.rc_handle;

    switch (p_context->rc_status_cmd.pdu_id) {
    case AVRC_PDU_REGISTER_NOTIFICATION:
        rc_notification_interim_timout(p_context->rc_status_cmd.label);
        break;

    case AVRC_PDU_GET_CAPABILITIES:
        avrc_response.get_caps.status = BTIF_RC_STS_TIMEOUT;
        handle_get_capability_response(&meta_msg, &avrc_response.get_caps);
        break;

    case AVRC_PDU_LIST_PLAYER_APP_ATTR:
        avrc_response.list_app_attr.status = BTIF_RC_STS_TIMEOUT;
        handle_app_attr_response(&meta_msg, &avrc_response.list_app_attr);
        break;

    case AVRC_PDU_LIST_PLAYER_APP_VALUES:
        avrc_response.list_app_values.status = BTIF_RC_STS_TIMEOUT;
        handle_app_val_response(&meta_msg, &avrc_response.list_app_values);
        break;

    case AVRC_PDU_GET_CUR_PLAYER_APP_VALUE:
        avrc_response.get_cur_app_val.status = BTIF_RC_STS_TIMEOUT;
        handle_app_cur_val_response(&meta_msg, &avrc_response.get_cur_app_val);
        break;

    case AVRC_PDU_GET_PLAYER_APP_ATTR_TEXT:
        avrc_response.get_app_attr_txt.status = BTIF_RC_STS_TIMEOUT;
        handle_app_attr_txt_response(&meta_msg, &avrc_response.get_app_attr_txt);
        break;

    case AVRC_PDU_GET_PLAYER_APP_VALUE_TEXT:
        avrc_response.get_app_val_txt.status = BTIF_RC_STS_TIMEOUT;
        handle_app_attr_txt_response(&meta_msg, &avrc_response.get_app_val_txt);
        break;

    case AVRC_PDU_GET_ELEMENT_ATTR:
        avrc_response.get_elem_attrs.status = BTIF_RC_STS_TIMEOUT;
        handle_get_elem_attr_response(&meta_msg, &avrc_response.get_elem_attrs);
        break;

    case AVRC_PDU_GET_PLAY_STATUS:
        avrc_response.get_play_status.status = BTIF_RC_STS_TIMEOUT;
        handle_get_playstatus_response(&meta_msg, &avrc_response.get_play_status);
        break;
    }
    release_transaction(p_context->rc_status_cmd.label);
}

/***************************************************************************
**
** Function         btif_rc_status_cmd_timer_timeout
**
** Description      RC status command timeout callback.
**                  This is called from BTU context and switches to BTIF
**                  context to handle the timeout events
** Returns          None
**
***************************************************************************/
static void btif_rc_status_cmd_timer_timeout(void *data)
{
    btif_rc_timer_context_t *p_data = (btif_rc_timer_context_t *)data;

    btif_transfer_context(btif_rc_status_cmd_timeout_handler, 0,
                          (char *)p_data, sizeof(btif_rc_timer_context_t),
                          NULL);
}

/***************************************************************************
**
** Function         btif_rc_control_cmd_timeout_handler
**
** Description      RC control command timeout handler (Runs in BTIF context).
** Returns          None
**
***************************************************************************/
static void btif_rc_control_cmd_timeout_handler(UNUSED_ATTR uint16_t event,
                                                char *data)
{
    btif_rc_timer_context_t *p_context = (btif_rc_timer_context_t *)data;
    tAVRC_RESPONSE      avrc_response = {0};
    tBTA_AV_META_MSG    meta_msg;

    memset(&meta_msg, 0, sizeof(tBTA_AV_META_MSG));
    meta_msg.rc_handle = btif_rc_cb.rc_handle;

    switch (p_context->rc_control_cmd.pdu_id) {
    case AVRC_PDU_SET_PLAYER_APP_VALUE:
        avrc_response.set_app_val.status = BTIF_RC_STS_TIMEOUT;
        handle_set_app_attr_val_response(&meta_msg,
                                         &avrc_response.set_app_val);
        break;
    }
    release_transaction(p_context->rc_control_cmd.label);
}

/***************************************************************************
**
** Function         btif_rc_control_cmd_timer_timeout
**
** Description      RC control command timeout callback.
**                  This is called from BTU context and switches to BTIF
**                  context to handle the timeout events
** Returns          None
**
***************************************************************************/
static void btif_rc_control_cmd_timer_timeout(void *data)
{
    btif_rc_timer_context_t *p_data = (btif_rc_timer_context_t *)data;

    btif_transfer_context(btif_rc_control_cmd_timeout_handler, 0,
                          (char *)p_data, sizeof(btif_rc_timer_context_t),
                          NULL);
}

/***************************************************************************
**
** Function         btif_rc_play_status_timeout_handler
**
** Description      RC play status timeout handler (Runs in BTIF context).
** Returns          None
**
***************************************************************************/
static void btif_rc_play_status_timeout_handler(UNUSED_ATTR uint16_t event,
                                                UNUSED_ATTR char *p_data)
{
    get_play_status_cmd();
    rc_start_play_status_timer();
}

/***************************************************************************
**
** Function         btif_rc_play_status_timer_timeout
**
** Description      RC play status timeout callback.
**                  This is called from BTU context and switches to BTIF
**                  context to handle the timeout events
** Returns          None
**
***************************************************************************/
static void btif_rc_play_status_timer_timeout(UNUSED_ATTR void *data)
{
    btif_transfer_context(btif_rc_play_status_timeout_handler, 0, 0, 0, NULL);
}

/***************************************************************************
**
** Function         rc_start_play_status_timer
**
** Description      Helper function to start the timer to fetch play status.
** Returns          None
**
***************************************************************************/
static void rc_start_play_status_timer(void)
{
    /* Start the Play status timer only if it is not started */
    if (!alarm_is_scheduled(btif_rc_cb.rc_play_status_timer)) {
        if (btif_rc_cb.rc_play_status_timer == NULL) {
            btif_rc_cb.rc_play_status_timer =
                alarm_new("btif_rc.rc_play_status_timer");
        }
        alarm_set_on_queue(btif_rc_cb.rc_play_status_timer,
                           BTIF_TIMEOUT_RC_INTERIM_RSP_MS,
                           btif_rc_play_status_timer_timeout, NULL,
                           btu_general_alarm_queue);
    }
}

/***************************************************************************
**
** Function         rc_stop_play_status_timer
**
** Description      Helper function to stop the play status timer.
** Returns          None
**
***************************************************************************/
void rc_stop_play_status_timer()
{
    if (btif_rc_cb.rc_play_status_timer != NULL)
        alarm_cancel(btif_rc_cb.rc_play_status_timer);
}

/***************************************************************************
**
** Function         register_for_event_notification
**
** Description      Helper function registering notification events
**                  sets an interim response timeout to handle if the remote
**                  does not respond.
** Returns          None
**
***************************************************************************/
static void register_for_event_notification(btif_rc_supported_event_t *p_event)
{
    bt_status_t status;
    rc_transaction_t *p_transaction;

    status = get_transaction(&p_transaction);
    if (status == BT_STATUS_SUCCESS)
    {
        btif_rc_timer_context_t *p_context = &p_transaction->txn_timer_context;

        status = register_notification_cmd (p_transaction->lbl, p_event->event_id, 0);
        if (status != BT_STATUS_SUCCESS)
        {
            BTIF_TRACE_ERROR("%s Error in Notification registration %d",
                __FUNCTION__, status);
            release_transaction (p_transaction->lbl);
            return;
        }
        p_event->label = p_transaction->lbl;
        p_event->status = eREGISTERED;
        p_context->rc_status_cmd.label = p_transaction->lbl;
        p_context->rc_status_cmd.pdu_id = AVRC_PDU_REGISTER_NOTIFICATION;

        alarm_free(p_transaction->txn_timer);
        p_transaction->txn_timer =
            alarm_new("btif_rc.status_command_txn_timer");
        alarm_set_on_queue(p_transaction->txn_timer,
                           BTIF_TIMEOUT_RC_INTERIM_RSP_MS,
                           btif_rc_status_cmd_timer_timeout, p_context,
                           btu_general_alarm_queue);
    }
    else
    {
        BTIF_TRACE_ERROR("%s Error No more Transaction label %d",
            __FUNCTION__, status);
    }
}

static void start_status_command_timer(UINT8 pdu_id, rc_transaction_t *p_txn)
{
    btif_rc_timer_context_t *p_context = &p_txn->txn_timer_context;
    p_context->rc_status_cmd.label = p_txn->lbl;
    p_context->rc_status_cmd.pdu_id = pdu_id;

    alarm_free(p_txn->txn_timer);
    p_txn->txn_timer = alarm_new("btif_rc.status_command_txn_timer");
    alarm_set_on_queue(p_txn->txn_timer, BTIF_TIMEOUT_RC_STATUS_CMD_MS,
                       btif_rc_status_cmd_timer_timeout, p_context,
                       btu_general_alarm_queue);
}

static void start_control_command_timer(UINT8 pdu_id, rc_transaction_t *p_txn)
{
    btif_rc_timer_context_t *p_context = &p_txn->txn_timer_context;
    p_context->rc_control_cmd.label = p_txn->lbl;
    p_context->rc_control_cmd.pdu_id = pdu_id;

    alarm_free(p_txn->txn_timer);
    p_txn->txn_timer = alarm_new("btif_rc.control_command_txn_timer");
    alarm_set_on_queue(p_txn->txn_timer,
                       BTIF_TIMEOUT_RC_CONTROL_CMD_MS,
                       btif_rc_control_cmd_timer_timeout, p_context,
                       btu_general_alarm_queue);
}

/***************************************************************************
**
** Function         handle_get_capability_response
**
** Description      Handles the get_cap_response to populate company id info
**                  and query the supported events.
**                  Initiates Notification registration for events supported
** Returns          None
**
***************************************************************************/
static void handle_get_capability_response (tBTA_AV_META_MSG *pmeta_msg, tAVRC_GET_CAPS_RSP *p_rsp)
{
    int xx = 0;

    /* Todo: Do we need to retry on command timeout */
    if (p_rsp->status != AVRC_STS_NO_ERROR)
    {
        BTIF_TRACE_ERROR("%s Error capability response 0x%02X",
                __FUNCTION__, p_rsp->status);
        return;
    }

    if (p_rsp->capability_id == AVRC_CAP_EVENTS_SUPPORTED)
    {
        btif_rc_supported_event_t *p_event;

#if defined(MTK_LINUX_AVRCP_PLUS) && (MTK_LINUX_AVRCP_PLUS == TRUE)
        /* new list when avrcp connected avoid crash if register_notification_rsp before handle_get_capability_response */
#else
        /* Todo: Check if list can be active when we hit here */
        btif_rc_cb.rc_supported_event_list = list_new(osi_free);
#endif
        for (xx = 0; xx < p_rsp->count; xx++)
        {
            /* Skip registering for Play position change notification */
            if ((p_rsp->param.event_id[xx] == AVRC_EVT_PLAY_STATUS_CHANGE)||
                (p_rsp->param.event_id[xx] == AVRC_EVT_TRACK_CHANGE)||
                (p_rsp->param.event_id[xx] == AVRC_EVT_APP_SETTING_CHANGE))
            {
                p_event = (btif_rc_supported_event_t *)osi_malloc(sizeof(btif_rc_supported_event_t));
                p_event->event_id = p_rsp->param.event_id[xx];
                p_event->status = eNOT_REGISTERED;
                list_append(btif_rc_cb.rc_supported_event_list, p_event);
            }
        }
#if defined(MTK_COMMON) && (MTK_COMMON == TRUE)
        /* To avoid the condtion: Host and remote device are all sink. In this case, list length is zero */
        if (list_length(btif_rc_cb.rc_supported_event_list) != 0)
        {
            BTIF_TRACE_EVENT("supported event list size:%d", list_length(btif_rc_cb.rc_supported_event_list));
            p_event = list_front(btif_rc_cb.rc_supported_event_list);
        }
        else
        {
            p_event = NULL;
        }
#else
        p_event = list_front(btif_rc_cb.rc_supported_event_list);
#endif

        if (p_event != NULL)
        {
            register_for_event_notification(p_event);
        }
    }
    else if (p_rsp->capability_id == AVRC_CAP_COMPANY_ID)
    {
        getcapabilities_cmd (AVRC_CAP_EVENTS_SUPPORTED);
        BTIF_TRACE_EVENT("%s AVRC_CAP_COMPANY_ID: ", __FUNCTION__);
        for (xx = 0; xx < p_rsp->count; xx++)
        {
            BTIF_TRACE_EVENT("%s    : %d", __FUNCTION__, p_rsp->param.company_id[xx]);
        }
    }
}

bool rc_is_track_id_valid (tAVRC_UID uid)
{
    tAVRC_UID invalid_uid = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

    if (memcmp(uid, invalid_uid, sizeof(tAVRC_UID)) == 0)
    {
        return false;
    }
    else
    {
        return true;
    }
}

/***************************************************************************
**
** Function         handle_notification_response
**
** Description      Main handler for notification responses to registered events
**                  1. Register for unregistered event(in interim response path)
**                  2. After registering for all supported events, start
**                     retrieving application settings and values
**                  3. Reregister for events on getting changed response
**                  4. Run play status timer for getting position when the
**                     status changes to playing
**                  5. Get the Media details when the track change happens
**                     or track change interim response is received with
**                     valid track id
**                  6. HAL callback for play status change and application
**                     setting change
** Returns          None
**
***************************************************************************/
static void handle_notification_response (tBTA_AV_META_MSG *pmeta_msg, tAVRC_REG_NOTIF_RSP *p_rsp)
{
    bt_bdaddr_t rc_addr;
    UINT32 attr_list[] = {
        AVRC_MEDIA_ATTR_ID_TITLE,
        AVRC_MEDIA_ATTR_ID_ARTIST,
        AVRC_MEDIA_ATTR_ID_ALBUM,
        AVRC_MEDIA_ATTR_ID_TRACK_NUM,
        AVRC_MEDIA_ATTR_ID_NUM_TRACKS,
        AVRC_MEDIA_ATTR_ID_GENRE,
        AVRC_MEDIA_ATTR_ID_PLAYING_TIME
        };


    bdcpy(rc_addr.address, btif_rc_cb.rc_addr);

    if (pmeta_msg->code == AVRC_RSP_INTERIM)
    {
        btif_rc_supported_event_t *p_event;
        list_node_t *node;

        BTIF_TRACE_DEBUG("%s Interim response : 0x%2X ", __FUNCTION__, p_rsp->event_id);
        switch (p_rsp->event_id)
        {
            case AVRC_EVT_PLAY_STATUS_CHANGE:
                /* Start timer to get play status periodically
                 * if the play state is playing.
                 */
#if defined(MTK_LINUX) && defined(MTK_LINUX_AVRCP_GET_PLAY_STATUS) && (MTK_LINUX_AVRCP_GET_PLAY_STATUS == FALSE)
                //don't start play status timer
#else
                if (p_rsp->param.play_status == AVRC_PLAYSTATE_PLAYING)
                {
                    rc_start_play_status_timer();
                }
#endif
#if defined(MTK_LINUX) && defined(MTK_COMMON) && (MTK_COMMON == TRUE)
                if (btif_rc_cb.rc_play_status == p_rsp->param.play_status)
                {
                    break;
                }
                btif_rc_cb.rc_play_status = p_rsp->param.play_status;
#endif
                HAL_CBACK(bt_rc_ctrl_callbacks, play_status_changed_cb,
                    &rc_addr, p_rsp->param.play_status);
                break;

            case AVRC_EVT_TRACK_CHANGE:
                if (rc_is_track_id_valid (p_rsp->param.track) != true)
                {
                    break;
                }
                else
                {
                    UINT8 *p_data = p_rsp->param.track;
                    /* Update the UID for current track
                     * Attributes will be fetched after the AVRCP procedure
                     */
                    BE_STREAM_TO_UINT64(btif_rc_cb.rc_playing_uid, p_data);
                }
                break;

            case AVRC_EVT_APP_SETTING_CHANGE:
                break;

            case AVRC_EVT_NOW_PLAYING_CHANGE:
                break;

            case AVRC_EVT_AVAL_PLAYERS_CHANGE:
                break;

            case AVRC_EVT_ADDR_PLAYER_CHANGE:
                break;

            case AVRC_EVT_UIDS_CHANGE:
                break;

            case AVRC_EVT_TRACK_REACHED_END:
            case AVRC_EVT_TRACK_REACHED_START:
            case AVRC_EVT_PLAY_POS_CHANGED:
            case AVRC_EVT_BATTERY_STATUS_CHANGE:
            case AVRC_EVT_SYSTEM_STATUS_CHANGE:
            default:
                BTIF_TRACE_ERROR("%s  Unhandled interim response 0x%2X", __FUNCTION__,
                    p_rsp->event_id);
                return;
        }
        list_foreach(btif_rc_cb.rc_supported_event_list,
                iterate_supported_event_list_for_interim_rsp,
                &p_rsp->event_id);

        node = list_begin(btif_rc_cb.rc_supported_event_list);
        while (node != NULL)
        {
            p_event = (btif_rc_supported_event_t *)list_node(node);
            if ((p_event != NULL) && (p_event->status == eNOT_REGISTERED))
            {
                register_for_event_notification(p_event);
                break;
            }
            node = list_next (node);
            p_event = NULL;
        }
        /* Registered for all events, we can request application settings */
        if ((p_event == NULL) && (btif_rc_cb.rc_app_settings.query_started == false))
        {
            /* we need to do this only if remote TG supports
             * player application settings
             */
            btif_rc_cb.rc_app_settings.query_started = TRUE;
            if (btif_rc_cb.rc_features & BTA_AV_FEAT_APP_SETTING)
            {
                list_player_app_setting_attrib_cmd();
            }
            else
            {
                BTIF_TRACE_DEBUG("%s App setting not supported, complete procedure", __FUNCTION__);
                rc_ctrl_procedure_complete();
            }
        }
    }
    else if (pmeta_msg->code == AVRC_RSP_CHANGED)
    {
        btif_rc_supported_event_t *p_event;
        list_node_t *node;

        BTIF_TRACE_DEBUG("%s Notification completed : 0x%2X ", __FUNCTION__,
            p_rsp->event_id);

        node = list_begin(btif_rc_cb.rc_supported_event_list);
        while (node != NULL)
        {
            p_event = (btif_rc_supported_event_t *)list_node(node);
            if ((p_event != NULL) && (p_event->event_id == p_rsp->event_id))
            {
                p_event->status = eNOT_REGISTERED;
                register_for_event_notification(p_event);
                break;
            }
            node = list_next (node);
        }

        switch (p_rsp->event_id)
        {
            case AVRC_EVT_PLAY_STATUS_CHANGE:
                /* Start timer to get play status periodically
                 * if the play state is playing.
                 */
#if defined(MTK_LINUX) && defined(MTK_LINUX_AVRCP_GET_PLAY_STATUS) && (MTK_LINUX_AVRCP_GET_PLAY_STATUS == FALSE)
                //don't start play status timer
#else
                if (p_rsp->param.play_status == AVRC_PLAYSTATE_PLAYING)
                {
                    rc_start_play_status_timer();
                }
                else
                {
                    rc_stop_play_status_timer();
                }
#endif
#if defined(MTK_LINUX) && defined(MTK_COMMON) && (MTK_COMMON == TRUE)
                if (btif_rc_cb.rc_play_status == p_rsp->param.play_status)
                {
                    break;
                }
                btif_rc_cb.rc_play_status = p_rsp->param.play_status;
#endif
                HAL_CBACK(bt_rc_ctrl_callbacks, play_status_changed_cb,
                    &rc_addr, p_rsp->param.play_status);
                break;

            case AVRC_EVT_TRACK_CHANGE:
                if (rc_is_track_id_valid (p_rsp->param.track) != true)
                {
                    break;
                }
                get_element_attribute_cmd (AVRC_MAX_NUM_MEDIA_ATTR_ID, attr_list);
                break;

            case AVRC_EVT_APP_SETTING_CHANGE:
            {
                btrc_player_settings_t app_settings;
                UINT16 xx;

                app_settings.num_attr = p_rsp->param.player_setting.num_attr;
                for (xx = 0; xx < app_settings.num_attr; xx++)
                {
                    app_settings.attr_ids[xx] = p_rsp->param.player_setting.attr_id[xx];
                    app_settings.attr_values[xx] = p_rsp->param.player_setting.attr_value[xx];
                }
                HAL_CBACK(bt_rc_ctrl_callbacks, playerapplicationsetting_changed_cb,
                    &rc_addr, &app_settings);
            }
                break;

            case AVRC_EVT_NOW_PLAYING_CHANGE:
                break;

            case AVRC_EVT_AVAL_PLAYERS_CHANGE:
                break;

            case AVRC_EVT_ADDR_PLAYER_CHANGE:
                break;

            case AVRC_EVT_UIDS_CHANGE:
                break;

            case AVRC_EVT_TRACK_REACHED_END:
            case AVRC_EVT_TRACK_REACHED_START:
            case AVRC_EVT_PLAY_POS_CHANGED:
            case AVRC_EVT_BATTERY_STATUS_CHANGE:
            case AVRC_EVT_SYSTEM_STATUS_CHANGE:
            default:
                BTIF_TRACE_ERROR("%s  Unhandled completion response 0x%2X",
                    __FUNCTION__, p_rsp->event_id);
                return;
        }
    }
}

/***************************************************************************
**
** Function         handle_app_attr_response
**
** Description      handles the the application attributes response and
**                  initiates procedure to fetch the attribute values
** Returns          None
**
***************************************************************************/
static void handle_app_attr_response (tBTA_AV_META_MSG *pmeta_msg, tAVRC_LIST_APP_ATTR_RSP *p_rsp)
{
    UINT8 xx;

    if (p_rsp->status != AVRC_STS_NO_ERROR)
    {
        BTIF_TRACE_ERROR("%s Error getting Player application settings: 0x%2X",
                __FUNCTION__, p_rsp->status);
        rc_ctrl_procedure_complete();
        return;
    }

    for (xx = 0; xx < p_rsp->num_attr; xx++)
    {
        UINT8 st_index;

        if (p_rsp->attrs[xx] > AVRC_PLAYER_SETTING_LOW_MENU_EXT)
        {
            st_index = btif_rc_cb.rc_app_settings.num_ext_attrs;
            btif_rc_cb.rc_app_settings.ext_attrs[st_index].attr_id = p_rsp->attrs[xx];
            btif_rc_cb.rc_app_settings.num_ext_attrs++;
        }
        else
        {
            st_index = btif_rc_cb.rc_app_settings.num_attrs;
            btif_rc_cb.rc_app_settings.attrs[st_index].attr_id = p_rsp->attrs[xx];
            btif_rc_cb.rc_app_settings.num_attrs++;
        }
    }
    btif_rc_cb.rc_app_settings.attr_index = 0;
    btif_rc_cb.rc_app_settings.ext_attr_index = 0;
    btif_rc_cb.rc_app_settings.ext_val_index = 0;
    if (p_rsp->num_attr)
    {
        list_player_app_setting_value_cmd (btif_rc_cb.rc_app_settings.attrs[0].attr_id);
    }
    else
    {
        BTIF_TRACE_ERROR("%s No Player application settings found",
                __FUNCTION__);
    }
}

/***************************************************************************
**
** Function         handle_app_val_response
**
** Description      handles the the attributes value response and if extended
**                  menu is available, it initiates query for the attribute
**                  text. If not, it initiates procedure to get the current
**                  attribute values and calls the HAL callback for provding
**                  application settings information.
** Returns          None
**
***************************************************************************/
static void handle_app_val_response (tBTA_AV_META_MSG *pmeta_msg, tAVRC_LIST_APP_VALUES_RSP *p_rsp)
{
    UINT8 xx, attr_index;
    UINT8 attrs[AVRC_MAX_APP_ATTR_SIZE];
    btif_rc_player_app_settings_t *p_app_settings;
    bt_bdaddr_t rc_addr;

    /* Todo: Do we need to retry on command timeout */
    if (p_rsp->status != AVRC_STS_NO_ERROR)
    {
        BTIF_TRACE_ERROR("%s Error fetching attribute values 0x%02X",
                __FUNCTION__, p_rsp->status);
        return;
    }

    p_app_settings = &btif_rc_cb.rc_app_settings;
    bdcpy(rc_addr.address, btif_rc_cb.rc_addr);

    if (p_app_settings->attr_index < p_app_settings->num_attrs)
    {
        attr_index = p_app_settings->attr_index;
        p_app_settings->attrs[attr_index].num_val = p_rsp->num_val;
        for (xx = 0; xx < p_rsp->num_val; xx++)
        {
            p_app_settings->attrs[attr_index].attr_val[xx] = p_rsp->vals[xx];
        }
        attr_index++;
        p_app_settings->attr_index++;
        if (attr_index < p_app_settings->num_attrs)
        {
            list_player_app_setting_value_cmd (p_app_settings->attrs[p_app_settings->attr_index].attr_id);
        }
        else if (p_app_settings->ext_attr_index < p_app_settings->num_ext_attrs)
        {
            attr_index = 0;
            p_app_settings->ext_attr_index = 0;
            list_player_app_setting_value_cmd (p_app_settings->ext_attrs[attr_index].attr_id);
        }
        else
        {
            for (xx = 0; xx < p_app_settings->num_attrs; xx++)
            {
                attrs[xx] = p_app_settings->attrs[xx].attr_id;
            }
            get_player_app_setting_cmd (p_app_settings->num_attrs, attrs);
            HAL_CBACK (bt_rc_ctrl_callbacks, playerapplicationsetting_cb, &rc_addr,
                        p_app_settings->num_attrs, p_app_settings->attrs, 0, NULL);
        }
    }
    else if (p_app_settings->ext_attr_index < p_app_settings->num_ext_attrs)
    {
        attr_index = p_app_settings->ext_attr_index;
        p_app_settings->ext_attrs[attr_index].num_val = p_rsp->num_val;
        for (xx = 0; xx < p_rsp->num_val; xx++)
        {
            p_app_settings->ext_attrs[attr_index].ext_attr_val[xx].val = p_rsp->vals[xx];
        }
        attr_index++;
        p_app_settings->ext_attr_index++;
        if (attr_index < p_app_settings->num_ext_attrs)
        {
            list_player_app_setting_value_cmd (p_app_settings->ext_attrs[p_app_settings->ext_attr_index].attr_id);
        }
        else
        {
            UINT8 attr[AVRC_MAX_APP_ATTR_SIZE];
            UINT8 xx;

            for (xx = 0; xx < p_app_settings->num_ext_attrs; xx++)
            {
                attr[xx] = p_app_settings->ext_attrs[xx].attr_id;
            }
            get_player_app_setting_attr_text_cmd(attr, xx);
        }
    }
}

/***************************************************************************
**
** Function         handle_app_cur_val_response
**
** Description      handles the the get attributes value response.
**
** Returns          None
**
***************************************************************************/
static void handle_app_cur_val_response (tBTA_AV_META_MSG *pmeta_msg, tAVRC_GET_CUR_APP_VALUE_RSP *p_rsp)
{
    btrc_player_settings_t app_settings;
    bt_bdaddr_t rc_addr;
    UINT16 xx;

    /* Todo: Do we need to retry on command timeout */
    if (p_rsp->status != AVRC_STS_NO_ERROR)
    {
        BTIF_TRACE_ERROR("%s Error fetching current settings: 0x%02X",
                __FUNCTION__, p_rsp->status);
        return;
    }

    bdcpy(rc_addr.address, btif_rc_cb.rc_addr);

    app_settings.num_attr = p_rsp->num_val;
    for (xx = 0; xx < app_settings.num_attr; xx++)
    {
        app_settings.attr_ids[xx] = p_rsp->p_vals[xx].attr_id;
        app_settings.attr_values[xx] = p_rsp->p_vals[xx].attr_val;
    }
    HAL_CBACK(bt_rc_ctrl_callbacks, playerapplicationsetting_changed_cb,
        &rc_addr, &app_settings);
    /* Application settings are fetched only once for initial values
     * initiate anything that follows after RC procedure.
     * Defer it if browsing is supported till players query
     */
    rc_ctrl_procedure_complete ();
    osi_free_and_reset((void **)&p_rsp->p_vals);
}

/***************************************************************************
**
** Function         handle_app_attr_txt_response
**
** Description      handles the the get attributes text response, if fails
**                  calls HAL callback with just normal settings and initiates
**                  query for current settings else initiates query for value text
** Returns          None
**
***************************************************************************/
static void handle_app_attr_txt_response (tBTA_AV_META_MSG *pmeta_msg, tAVRC_GET_APP_ATTR_TXT_RSP *p_rsp)
{
    UINT8 xx;
    UINT8 vals[AVRC_MAX_APP_ATTR_SIZE];
    btif_rc_player_app_settings_t *p_app_settings;
    bt_bdaddr_t rc_addr;

    p_app_settings = &btif_rc_cb.rc_app_settings;
    bdcpy(rc_addr.address, btif_rc_cb.rc_addr);

    /* Todo: Do we need to retry on command timeout */
    if (p_rsp->status != AVRC_STS_NO_ERROR)
    {
        UINT8 attrs[AVRC_MAX_APP_ATTR_SIZE];

        BTIF_TRACE_ERROR("%s Error fetching attribute text: 0x%02X",
                __FUNCTION__, p_rsp->status);
        /* Not able to fetch Text for extended Menu, skip the process
         * and cleanup used memory. Proceed to get the current settings
         * for standard attributes.
         */
        p_app_settings->num_ext_attrs = 0;
        for (xx = 0; xx < p_app_settings->ext_attr_index; xx++)
            osi_free_and_reset((void **)&p_app_settings->ext_attrs[xx].p_str);
        p_app_settings->ext_attr_index = 0;

        for (xx = 0; xx < p_app_settings->num_attrs; xx++)
        {
            attrs[xx] = p_app_settings->attrs[xx].attr_id;
        }
        HAL_CBACK (bt_rc_ctrl_callbacks, playerapplicationsetting_cb, &rc_addr,
                    p_app_settings->num_attrs, p_app_settings->attrs, 0, NULL);

        get_player_app_setting_cmd (xx, attrs);
        return;
    }

    for (xx = 0; xx < p_rsp->num_attr; xx++)
    {
        UINT8 x;
        for (x = 0; x < p_app_settings->num_ext_attrs; x++)
        {
            if (p_app_settings->ext_attrs[x].attr_id == p_rsp->p_attrs[xx].attr_id)
            {
                p_app_settings->ext_attrs[x].charset_id = p_rsp->p_attrs[xx].charset_id;
                p_app_settings->ext_attrs[x].str_len = p_rsp->p_attrs[xx].str_len;
                p_app_settings->ext_attrs[x].p_str = p_rsp->p_attrs[xx].p_str;
                break;
            }
        }
    }

    for (xx = 0; xx < p_app_settings->ext_attrs[0].num_val; xx++)
    {
        vals[xx] = p_app_settings->ext_attrs[0].ext_attr_val[xx].val;
    }
    get_player_app_setting_value_text_cmd(vals, xx);
}


/***************************************************************************
**
** Function         handle_app_attr_val_txt_response
**
** Description      handles the the get attributes value text response, if fails
**                  calls HAL callback with just normal settings and initiates
**                  query for current settings
** Returns          None
**
***************************************************************************/
static void handle_app_attr_val_txt_response (tBTA_AV_META_MSG *pmeta_msg, tAVRC_GET_APP_ATTR_TXT_RSP *p_rsp)
{
    UINT8 xx, attr_index;
    UINT8 vals[AVRC_MAX_APP_ATTR_SIZE];
    UINT8 attrs[AVRC_MAX_APP_ATTR_SIZE];
    btif_rc_player_app_settings_t *p_app_settings;
    bt_bdaddr_t rc_addr;

    bdcpy(rc_addr.address, btif_rc_cb.rc_addr);
    p_app_settings = &btif_rc_cb.rc_app_settings;

    /* Todo: Do we need to retry on command timeout */
    if (p_rsp->status != AVRC_STS_NO_ERROR)
    {
        UINT8 attrs[AVRC_MAX_APP_ATTR_SIZE];

        BTIF_TRACE_ERROR("%s Error fetching attribute value text: 0x%02X",
                __FUNCTION__, p_rsp->status);

        /* Not able to fetch Text for extended Menu, skip the process
         * and cleanup used memory. Proceed to get the current settings
         * for standard attributes.
         */
        p_app_settings->num_ext_attrs = 0;
        for (xx = 0; xx < p_app_settings->ext_attr_index; xx++)
        {
            int x;
            btrc_player_app_ext_attr_t *p_ext_attr = &p_app_settings->ext_attrs[xx];

            for (x = 0; x < p_ext_attr->num_val; x++)
                osi_free_and_reset((void **)&p_ext_attr->ext_attr_val[x].p_str);
            p_ext_attr->num_val = 0;
            osi_free_and_reset((void **)&p_app_settings->ext_attrs[xx].p_str);
        }
        p_app_settings->ext_attr_index = 0;

        for (xx = 0; xx < p_app_settings->num_attrs; xx++)
        {
            attrs[xx] = p_app_settings->attrs[xx].attr_id;
        }
        HAL_CBACK (bt_rc_ctrl_callbacks, playerapplicationsetting_cb, &rc_addr,
                    p_app_settings->num_attrs, p_app_settings->attrs, 0, NULL);

        get_player_app_setting_cmd (xx, attrs);
        return;
    }

    for (xx = 0; xx < p_rsp->num_attr; xx++)
    {
        UINT8 x;
        btrc_player_app_ext_attr_t *p_ext_attr;
        p_ext_attr = &p_app_settings->ext_attrs[p_app_settings->ext_val_index];
        for (x = 0; x < p_rsp->num_attr; x++)
        {
            if (p_ext_attr->ext_attr_val[x].val == p_rsp->p_attrs[xx].attr_id)
            {
                p_ext_attr->ext_attr_val[x].charset_id = p_rsp->p_attrs[xx].charset_id;
                p_ext_attr->ext_attr_val[x].str_len = p_rsp->p_attrs[xx].str_len;
                p_ext_attr->ext_attr_val[x].p_str = p_rsp->p_attrs[xx].p_str;
                break;
            }
        }
    }
    p_app_settings->ext_val_index++;

    if (p_app_settings->ext_val_index < p_app_settings->num_ext_attrs)
    {
        attr_index = p_app_settings->ext_val_index;
        for (xx = 0; xx < p_app_settings->ext_attrs[attr_index].num_val; xx++)
        {
            vals[xx] = p_app_settings->ext_attrs[attr_index].ext_attr_val[xx].val;
        }
        get_player_app_setting_value_text_cmd(vals, xx);
    }
    else
    {
        UINT8 x;

        for (xx = 0; xx < p_app_settings->num_attrs; xx++)
        {
            attrs[xx] = p_app_settings->attrs[xx].attr_id;
        }
        for (x = 0; x < p_app_settings->num_ext_attrs; x++)
        {
            attrs[xx+x] = p_app_settings->ext_attrs[x].attr_id;
        }
        HAL_CBACK (bt_rc_ctrl_callbacks, playerapplicationsetting_cb, &rc_addr,
                    p_app_settings->num_attrs, p_app_settings->attrs,
                    p_app_settings->num_ext_attrs, p_app_settings->ext_attrs);
        get_player_app_setting_cmd (xx + x, attrs);

        /* Free the application settings information after sending to
         * application.
         */
        for (xx = 0; xx < p_app_settings->ext_attr_index; xx++)
        {
            int x;
            btrc_player_app_ext_attr_t *p_ext_attr = &p_app_settings->ext_attrs[xx];

            for (x = 0; x < p_ext_attr->num_val; x++)
                osi_free_and_reset((void **)&p_ext_attr->ext_attr_val[x].p_str);
            p_ext_attr->num_val = 0;
            osi_free_and_reset((void **)&p_app_settings->ext_attrs[xx].p_str);
        }
        p_app_settings->num_attrs = 0;
    }
}

/***************************************************************************
**
** Function         handle_set_app_attr_val_response
**
** Description      handles the the set attributes value response, if fails
**                  calls HAL callback to indicate the failure
** Returns          None
**
***************************************************************************/
static void handle_set_app_attr_val_response (tBTA_AV_META_MSG *pmeta_msg, tAVRC_RSP *p_rsp)
{
    uint8_t accepted = 0;
    bt_bdaddr_t rc_addr;

    bdcpy(rc_addr.address, btif_rc_cb.rc_addr);

    /* For timeout pmeta_msg will be NULL, else we need to
     * check if this is accepted by TG
     */
    if (pmeta_msg && (pmeta_msg->code == AVRC_RSP_ACCEPT))
    {
        accepted = 1;
    }
    HAL_CBACK(bt_rc_ctrl_callbacks, setplayerappsetting_rsp_cb, &rc_addr, accepted);
}

/***************************************************************************
**
** Function         handle_get_elem_attr_response
**
** Description      handles the the element attributes response, calls
**                  HAL callback to update track change information.
** Returns          None
**
***************************************************************************/
static void handle_get_elem_attr_response (tBTA_AV_META_MSG *pmeta_msg,
                                           tAVRC_GET_ELEM_ATTRS_RSP *p_rsp)
{
    if (p_rsp->status == AVRC_STS_NO_ERROR) {
        bt_bdaddr_t rc_addr;
        size_t buf_size = p_rsp->num_attr * sizeof(btrc_element_attr_val_t);
        btrc_element_attr_val_t *p_attr =
            (btrc_element_attr_val_t *)osi_calloc(buf_size);

        bdcpy(rc_addr.address, btif_rc_cb.rc_addr);

        for (int i = 0; i < p_rsp->num_attr; i++) {
            p_attr[i].attr_id = p_rsp->p_attrs[i].attr_id;
            /* Todo. Legth limit check to include null */
            if (p_rsp->p_attrs[i].name.str_len &&
                p_rsp->p_attrs[i].name.p_str) {
                memcpy(p_attr[i].text, p_rsp->p_attrs[i].name.p_str,
                       p_rsp->p_attrs[i].name.str_len);
#if defined(MTK_LINUX_AVRCP_PLUS) && (MTK_LINUX_AVRCP_PLUS == TRUE)
                p_attr[i].text[p_rsp->p_attrs[i].name.str_len] = '\0';
#endif
                osi_free_and_reset((void **)&p_rsp->p_attrs[i].name.p_str);
            }
        }
        HAL_CBACK(bt_rc_ctrl_callbacks, track_changed_cb,
                  &rc_addr, p_rsp->num_attr, p_attr);
#if defined(MTK_LINUX_AVRCP_PLUS) && (MTK_LINUX_AVRCP_PLUS == TRUE)
        osi_free(p_rsp->p_attrs);
#endif
        osi_free(p_attr);
    } else if (p_rsp->status == BTIF_RC_STS_TIMEOUT) {
        /* Retry for timeout case, this covers error handling
         * for continuation failure also.
         */
        UINT32 attr_list[] = {
            AVRC_MEDIA_ATTR_ID_TITLE,
            AVRC_MEDIA_ATTR_ID_ARTIST,
            AVRC_MEDIA_ATTR_ID_ALBUM,
            AVRC_MEDIA_ATTR_ID_TRACK_NUM,
            AVRC_MEDIA_ATTR_ID_NUM_TRACKS,
            AVRC_MEDIA_ATTR_ID_GENRE,
            AVRC_MEDIA_ATTR_ID_PLAYING_TIME
            };
        get_element_attribute_cmd (AVRC_MAX_NUM_MEDIA_ATTR_ID, attr_list);
    } else {
        BTIF_TRACE_ERROR("%s: Error in get element attr procedure %d",
                         __func__, p_rsp->status);
    }
}

/***************************************************************************
**
** Function         handle_get_playstatus_response
**
** Description      handles the the play status response, calls
**                  HAL callback to update play position.
** Returns          None
**
***************************************************************************/
static void handle_get_playstatus_response (tBTA_AV_META_MSG *pmeta_msg, tAVRC_GET_PLAY_STATUS_RSP *p_rsp)
{
    bt_bdaddr_t rc_addr;

    bdcpy(rc_addr.address, btif_rc_cb.rc_addr);

    if (p_rsp->status == AVRC_STS_NO_ERROR)
    {
        HAL_CBACK(bt_rc_ctrl_callbacks, play_position_changed_cb,
            &rc_addr, p_rsp->song_len, p_rsp->song_pos);
    }
    else
    {
        BTIF_TRACE_ERROR("%s: Error in get play status procedure %d",
            __FUNCTION__, p_rsp->status);
    }
}

#if defined(MTK_AVRCP_TG_15_BROWSE)&&(MTK_AVRCP_TG_15_BROWSE == TRUE)
/***************************************************************************
 *
 * Function         handle_set_addressed_player_response
 *
 * Description      handles the the set addressed player response, calls
 *                  HAL callback
 * Returns          None
 *
 **************************************************************************/
static void handle_set_addressed_player_response(tBTA_AV_META_MSG* pmeta_msg,
                                                 tAVRC_RSP* p_rsp) {
  bt_bdaddr_t rc_addr;

  bdcpy(rc_addr.address, btif_rc_cb.rc_addr);

  if (p_rsp->status == AVRC_STS_NO_ERROR) {

#if defined(MTK_AV_SUPPORT_BR_HAL_CB)&&(MTK_AV_SUPPORT_BR_HAL_CB == TRUE)
    HAL_CBACK(bt_rc_ctrl_callbacks, set_addressed_player_cb, &rc_addr,
              p_rsp->status);
#endif
  } else {
    BTIF_TRACE_ERROR("%s: Error in get play status procedure %d", __func__,
                     p_rsp->status);
  }
}

/***************************************************************************
 *
 * Function         handle_get_folder_items_response
 *
 * Description      handles the the get folder items response, calls
 *                  HAL callback to send the folder items.
 * Returns          None
 *
 **************************************************************************/
static void handle_get_folder_items_response(tBTA_AV_META_MSG* pmeta_msg,
                                             tAVRC_GET_ITEMS_RSP* p_rsp) {
  bt_bdaddr_t rc_addr;

  bdcpy(rc_addr.address, btif_rc_cb.rc_addr);

  if (p_rsp->status == AVRC_STS_NO_ERROR) {
    /* Convert the internal folder listing into a response that can
     * be passed onto JNI via HAL_CBACK
     */
    uint8_t item_count = p_rsp->item_count;
    btrc_folder_items_t* btrc_items = (btrc_folder_items_t*)osi_malloc(
        sizeof(btrc_folder_items_t) * item_count);
    for (uint8_t i = 0; i < item_count; i++) {
      const tAVRC_ITEM* avrc_item = &(p_rsp->p_item_list[i]);
      btrc_folder_items_t* btrc_item = &(btrc_items[i]);
      BTIF_TRACE_DEBUG("%s folder item type %d", __func__,
                       avrc_item->item_type);
      switch (avrc_item->item_type) {
        case AVRC_ITEM_MEDIA:
          BTIF_TRACE_DEBUG("%s setting type to %d", __func__, BTRC_ITEM_MEDIA);
          get_folder_item_type_media(avrc_item, btrc_item);
          break;

        case AVRC_ITEM_FOLDER:
          BTIF_TRACE_DEBUG("%s setting type to BTRC_ITEM_FOLDER", __func__);
          get_folder_item_type_folder(avrc_item, btrc_item);
          break;

        case AVRC_ITEM_PLAYER:
          BTIF_TRACE_DEBUG("%s setting type to BTRC_ITEM_PLAYER", __func__);
          get_folder_item_type_player(avrc_item, btrc_item);
          break;

        default:
          BTIF_TRACE_ERROR("%s cannot understand folder item type %d", __func__,
                           avrc_item->item_type);
      }
    }
#if defined(MTK_AV_SUPPORT_BR_HAL_CB)&&(MTK_AV_SUPPORT_BR_HAL_CB == TRUE)
    HAL_CBACK(bt_rc_ctrl_callbacks, get_folder_items_cb, &rc_addr,
              BTRC_STS_NO_ERROR,
              /* We want to make the ownership explicit in native */
              (const btrc_folder_items_t*)btrc_items, item_count);
#endif
    BTIF_TRACE_DEBUG("%s HAL CBACK get_folder_items_cb finished", __func__);

    /* Release the memory block for items since we OWN the object */
    osi_free(btrc_items);
  } else {
    BTIF_TRACE_ERROR("%s: Error %d", __func__, p_rsp->status);
#if defined(MTK_AV_SUPPORT_BR_HAL_CB)&&(MTK_AV_SUPPORT_BR_HAL_CB == TRUE)
    HAL_CBACK(bt_rc_ctrl_callbacks, get_folder_items_cb, &rc_addr,
              (btrc_status_t)p_rsp->status, NULL, 0);
#endif
  }
}

/***************************************************************************
 *
 * Function         get_folder_item_type_media
 *
 * Description      Converts the AVRC representation of a folder item with
 *                  TYPE media to BTIF representation.
 * Returns          None
 *
 **************************************************************************/
void get_folder_item_type_media(const tAVRC_ITEM* avrc_item,
                                btrc_folder_items_t* btrc_item) {
  btrc_item->item_type = BTRC_ITEM_MEDIA;
  const tAVRC_ITEM_MEDIA* avrc_item_media = &(avrc_item->u.media);
  btrc_item_media_t* btrc_item_media = &(btrc_item->media);
  /* UID */
  memset(btrc_item_media->uid, 0, BTRC_UID_SIZE * sizeof(uint8_t));
  memcpy(btrc_item_media->uid, avrc_item_media->uid,
         sizeof(uint8_t) * BTRC_UID_SIZE);

  /* Audio/Video type */
  switch (avrc_item_media->type) {
    case AVRC_MEDIA_TYPE_AUDIO:
      btrc_item_media->type = BTRC_MEDIA_TYPE_AUDIO;
      break;
    case AVRC_MEDIA_TYPE_VIDEO:
      btrc_item_media->type = BTRC_MEDIA_TYPE_VIDEO;
      break;
  }

  /* Charset ID */
  btrc_item_media->charset_id = avrc_item_media->name.charset_id;

  /* Copy the name */
  BTIF_TRACE_DEBUG("%s max len %d str len %d", __func__, BTRC_MAX_ATTR_STR_LEN,
                   avrc_item_media->name.str_len);
  memset(btrc_item_media->name, 0, BTRC_MAX_ATTR_STR_LEN * sizeof(uint8_t));
  memcpy(btrc_item_media->name, avrc_item_media->name.p_str,
         sizeof(uint8_t) * (avrc_item_media->name.str_len));

  /* Copy the parameters */
  btrc_item_media->num_attrs = avrc_item_media->attr_count;
  btrc_item_media->p_attrs = (btrc_element_attr_val_t*)osi_malloc(
      btrc_item_media->num_attrs * sizeof(btrc_element_attr_val_t));

  /* Extract each attribute */
  for (int i = 0; i < avrc_item_media->attr_count; i++) {
    btrc_element_attr_val_t* btrc_attr_pair = &(btrc_item_media->p_attrs[i]);
    tAVRC_ATTR_ENTRY* avrc_attr_pair = &(avrc_item_media->p_attr_list[i]);

    BTIF_TRACE_DEBUG("%s media attr id 0x%x", __func__,
                     avrc_attr_pair->attr_id);

    switch (avrc_attr_pair->attr_id) {
      case AVRC_MEDIA_ATTR_ID_TITLE:
        btrc_attr_pair->attr_id = BTRC_MEDIA_ATTR_ID_TITLE;
        break;
      case AVRC_MEDIA_ATTR_ID_ARTIST:
        btrc_attr_pair->attr_id = BTRC_MEDIA_ATTR_ID_ARTIST;
        break;
      case AVRC_MEDIA_ATTR_ID_ALBUM:
        btrc_attr_pair->attr_id = BTRC_MEDIA_ATTR_ID_ALBUM;
        break;
      case AVRC_MEDIA_ATTR_ID_TRACK_NUM:
        btrc_attr_pair->attr_id = BTRC_MEDIA_ATTR_ID_TRACK_NUM;
        break;
      case AVRC_MEDIA_ATTR_ID_NUM_TRACKS:
        btrc_attr_pair->attr_id = BTRC_MEDIA_ATTR_ID_NUM_TRACKS;
        break;
      case AVRC_MEDIA_ATTR_ID_GENRE:
        btrc_attr_pair->attr_id = BTRC_MEDIA_ATTR_ID_GENRE;
        break;
      case AVRC_MEDIA_ATTR_ID_PLAYING_TIME:
        btrc_attr_pair->attr_id = BTRC_MEDIA_ATTR_ID_PLAYING_TIME;
        break;
      default:
        BTIF_TRACE_ERROR("%s invalid media attr id: 0x%x", __func__,
                         avrc_attr_pair->attr_id);
        btrc_attr_pair->attr_id = BTRC_MEDIA_ATTR_ID_INVALID;
    }

    memset(btrc_attr_pair->text, 0, BTRC_MAX_ATTR_STR_LEN * sizeof(uint8_t));
    memcpy(btrc_attr_pair->text, avrc_attr_pair->name.p_str,
           avrc_attr_pair->name.str_len);
  }
}

/***************************************************************************
 *
 * Function         get_folder_item_type_folder
 *
 * Description      Converts the AVRC representation of a folder item with
 *                  TYPE folder to BTIF representation.
 * Returns          None
 *
 **************************************************************************/
void get_folder_item_type_folder(const tAVRC_ITEM* avrc_item,
                                 btrc_folder_items_t* btrc_item) {
  btrc_item->item_type = BTRC_ITEM_FOLDER;
  const tAVRC_ITEM_FOLDER* avrc_item_folder = &(avrc_item->u.folder);
  btrc_item_folder_t* btrc_item_folder = &(btrc_item->folder);
  /* Copy the UID */
  memset(btrc_item_folder->uid, 0, BTRC_UID_SIZE * sizeof(uint8_t));
  memcpy(btrc_item_folder->uid, avrc_item_folder->uid,
         sizeof(uint8_t) * BTRC_UID_SIZE);

  /* Copy the type */
  switch (avrc_item_folder->type) {
    case AVRC_FOLDER_TYPE_MIXED:
      btrc_item_folder->type = BTRC_FOLDER_TYPE_MIXED;
      break;
    case AVRC_FOLDER_TYPE_TITLES:
      btrc_item_folder->type = BTRC_FOLDER_TYPE_TITLES;
      break;
    case AVRC_FOLDER_TYPE_ALNUMS:
      btrc_item_folder->type = BTRC_FOLDER_TYPE_ALBUMS;
      break;
    case AVRC_FOLDER_TYPE_ARTISTS:
      btrc_item_folder->type = BTRC_FOLDER_TYPE_ARTISTS;
      break;
    case AVRC_FOLDER_TYPE_GENRES:
      btrc_item_folder->type = BTRC_FOLDER_TYPE_GENRES;
      break;
    case AVRC_FOLDER_TYPE_PLAYLISTS:
      btrc_item_folder->type = BTRC_FOLDER_TYPE_PLAYLISTS;
      break;
    case AVRC_FOLDER_TYPE_YEARS:
      btrc_item_folder->type = BTRC_FOLDER_TYPE_YEARS;
      break;
  }

  /* Copy if playable */
  btrc_item_folder->playable = avrc_item_folder->playable;

  /* Copy name */
  BTIF_TRACE_DEBUG("%s max len %d str len %d", __func__, BTRC_MAX_ATTR_STR_LEN,
                   avrc_item_folder->name.str_len);
  memset(btrc_item_folder->name, 0, BTRC_MAX_ATTR_STR_LEN * sizeof(uint8_t));
  memcpy(btrc_item_folder->name, avrc_item_folder->name.p_str,
         avrc_item_folder->name.str_len * sizeof(uint8_t));

  /* Copy charset */
  btrc_item_folder->charset_id = avrc_item_folder->name.charset_id;
}

/***************************************************************************
 *
 * Function         get_folder_item_type_player
 *
 * Description      Converts the AVRC representation of a folder item with
 *                  TYPE player to BTIF representation.
 * Returns          None
 *
 **************************************************************************/
void get_folder_item_type_player(const tAVRC_ITEM* avrc_item,
                                 btrc_folder_items_t* btrc_item) {
  btrc_item->item_type = BTRC_ITEM_PLAYER;
  const tAVRC_ITEM_PLAYER* avrc_item_player = &(avrc_item->u.player);
  btrc_item_player_t* btrc_item_player = &(btrc_item->player);
  /* Player ID */
  btrc_item_player->player_id = avrc_item_player->player_id;
  /* Major type */
  btrc_item_player->major_type = avrc_item_player->major_type;
  /* Sub type */
  btrc_item_player->sub_type = avrc_item_player->sub_type;
  /* Features */
  memcpy(btrc_item_player->features, avrc_item_player->features,
         BTRC_FEATURE_BIT_MASK_SIZE);

  memset(btrc_item_player->name, 0, BTRC_MAX_ATTR_STR_LEN * sizeof(uint8_t));
  memcpy(btrc_item_player->name, avrc_item_player->name.p_str,
         avrc_item_player->name.str_len);
}

/***************************************************************************
 *
 * Function         handle_change_path_response
 *
 * Description      handles the the change path response, calls
 *                  HAL callback to send the updated folder
 * Returns          None
 *
 **************************************************************************/
static void handle_change_path_response(tBTA_AV_META_MSG* pmeta_msg,
                                        tAVRC_CHG_PATH_RSP* p_rsp) {
  bt_bdaddr_t rc_addr;

  bdcpy(rc_addr.address, btif_rc_cb.rc_addr);

  if (p_rsp->status == AVRC_STS_NO_ERROR) {
#if defined(MTK_AV_SUPPORT_BR_HAL_CB)&&(MTK_AV_SUPPORT_BR_HAL_CB == TRUE)
    HAL_CBACK(bt_rc_ctrl_callbacks, change_folder_path_cb, &rc_addr,
              p_rsp->num_items);
#endif
  } else {
    BTIF_TRACE_ERROR("%s error in handle_change_path_response %d", __func__,
                     p_rsp->status);
  }
}

/***************************************************************************
 *
 * Function         handle_set_browsed_player_response
 *
 * Description      handles the the change path response, calls
 *                  HAL callback to send the updated folder
 * Returns          None
 *
 **************************************************************************/
static void handle_set_browsed_player_response(tBTA_AV_META_MSG* pmeta_msg,
                                               tAVRC_SET_BR_PLAYER_RSP* p_rsp) {
  bt_bdaddr_t rc_addr;

  bdcpy(rc_addr.address, btif_rc_cb.rc_addr);

  if (p_rsp->status == AVRC_STS_NO_ERROR) {
#if defined(MTK_AV_SUPPORT_BR_HAL_CB)&&(MTK_AV_SUPPORT_BR_HAL_CB == TRUE)
    HAL_CBACK(bt_rc_ctrl_callbacks, set_browsed_player_cb, &rc_addr,
              p_rsp->num_items, p_rsp->folder_depth);
#endif
  } else {
    BTIF_TRACE_ERROR("%s error %d", __func__, p_rsp->status);
  }
}
#endif //(end -- #if defined(MTK_AVRCP_TG_15_BROWSE))

/***************************************************************************
**
** Function         clear_cmd_timeout
**
** Description      helper function to stop the command timeout timer
** Returns          None
**
***************************************************************************/
static void clear_cmd_timeout (UINT8 label)
{
    rc_transaction_t *p_txn;

    p_txn = get_transaction_by_lbl (label);
    if (p_txn == NULL)
    {
        BTIF_TRACE_ERROR("%s: Error in transaction label lookup", __FUNCTION__);
        return;
    }

    if (p_txn->txn_timer != NULL)
        alarm_cancel(p_txn->txn_timer);
}

/***************************************************************************
**
** Function         handle_avk_rc_metamsg_rsp
**
** Description      Handle RC metamessage response
**
** Returns          void
**
***************************************************************************/
static void handle_avk_rc_metamsg_rsp(tBTA_AV_META_MSG *pmeta_msg)
{
    tAVRC_RESPONSE    avrc_response = {0};
    UINT8             scratch_buf[512] = {0};// this variable is unused
    UINT16            buf_len;
    tAVRC_STS         status;

    BTIF_TRACE_DEBUG("%s opcode = %d rsp_code = %d  ", __FUNCTION__,
                        pmeta_msg->p_msg->hdr.opcode, pmeta_msg->code);

#if defined(MTK_AVRCP_TG_15_BROWSE)&&(MTK_AVRCP_TG_15_BROWSE == TRUE)
    status = AVRC_Ctrl_ParsResponse(pmeta_msg->p_msg, &avrc_response, scratch_buf, &buf_len);
#endif
    if ((AVRC_OP_VENDOR == pmeta_msg->p_msg->hdr.opcode)&&
                (pmeta_msg->code >= AVRC_RSP_NOT_IMPL)&&
                (pmeta_msg->code <= AVRC_RSP_INTERIM))
    {
#if (defined(MTK_AVRCP_TG_15_BROWSE)&&(MTK_AVRCP_TG_15_BROWSE == FALSE))||(!defined MTK_AVRCP_TG_15_BROWSE)
        status = AVRC_Ctrl_ParsResponse(pmeta_msg->p_msg, &avrc_response, scratch_buf, &buf_len);
#endif
        BTIF_TRACE_DEBUG("%s parse status %d pdu = %d rsp_status = %d",
                         __FUNCTION__, status, avrc_response.pdu,
                         pmeta_msg->p_msg->vendor.hdr.ctype);

        switch (avrc_response.pdu)
        {
            case AVRC_PDU_REGISTER_NOTIFICATION:
                handle_notification_response(pmeta_msg, &avrc_response.reg_notif);
                if (pmeta_msg->code == AVRC_RSP_INTERIM)
                {
                    /* Don't free the transaction Id */
                    clear_cmd_timeout (pmeta_msg->label);
                    return;
                }
                break;

            case AVRC_PDU_GET_CAPABILITIES:
                handle_get_capability_response(pmeta_msg, &avrc_response.get_caps);
                break;

            case AVRC_PDU_LIST_PLAYER_APP_ATTR:
                handle_app_attr_response(pmeta_msg, &avrc_response.list_app_attr);
                break;

            case AVRC_PDU_LIST_PLAYER_APP_VALUES:
                handle_app_val_response(pmeta_msg, &avrc_response.list_app_values);
                break;

            case AVRC_PDU_GET_CUR_PLAYER_APP_VALUE:
                handle_app_cur_val_response(pmeta_msg, &avrc_response.get_cur_app_val);
                break;

            case AVRC_PDU_GET_PLAYER_APP_ATTR_TEXT:
                handle_app_attr_txt_response(pmeta_msg, &avrc_response.get_app_attr_txt);
                break;

            case AVRC_PDU_GET_PLAYER_APP_VALUE_TEXT:
                handle_app_attr_val_txt_response(pmeta_msg, &avrc_response.get_app_val_txt);
                break;

            case AVRC_PDU_SET_PLAYER_APP_VALUE:
                handle_set_app_attr_val_response(pmeta_msg, &avrc_response.set_app_val);
                break;

            case AVRC_PDU_GET_ELEMENT_ATTR:
                handle_get_elem_attr_response(pmeta_msg, &avrc_response.get_elem_attrs);
                break;

            case AVRC_PDU_GET_PLAY_STATUS:
                handle_get_playstatus_response(pmeta_msg, &avrc_response.get_play_status);
                break;

#if defined(MTK_AVRCP_TG_15_BROWSE)&&(MTK_AVRCP_TG_15_BROWSE == TRUE)
            case AVRC_PDU_SET_ADDRESSED_PLAYER:
                handle_set_addressed_player_response(pmeta_msg, &avrc_response.rsp);
                break;
#endif
        }
#if (defined(MTK_AVRCP_TG_15_BROWSE)&&(MTK_AVRCP_TG_15_BROWSE == FALSE))||(!defined MTK_AVRCP_TG_15_BROWSE)
        release_transaction(pmeta_msg->label);
#endif
    }
#if defined(MTK_AVRCP_TG_15_BROWSE)&&(MTK_AVRCP_TG_15_BROWSE == TRUE)
    else if (AVRC_OP_BROWSE == pmeta_msg->p_msg->hdr.opcode)
    {
        BTIF_TRACE_DEBUG("%s AVRC_OP_BROWSE pdu %d", __func__, avrc_response.pdu);
        /* check what kind of command it is for browsing */
        switch (avrc_response.pdu) {
          case AVRC_PDU_GET_FOLDER_ITEMS:
            handle_get_folder_items_response(pmeta_msg, &avrc_response.get_items);
            break;
          case AVRC_PDU_CHANGE_PATH:
            handle_change_path_response(pmeta_msg, &avrc_response.chg_path);
            break;
          case AVRC_PDU_SET_BROWSED_PLAYER:
            handle_set_browsed_player_response(pmeta_msg, &avrc_response.br_player);
            break;
          default:
            BTIF_TRACE_ERROR("%s cannot handle browse pdu %d", __func__,
                         pmeta_msg->p_msg->hdr.opcode);
    }
  }
#endif
    else
    {
        BTIF_TRACE_DEBUG("%s:Invalid Vendor Command  code: %d len: %d. Not processing it.",
            __FUNCTION__, pmeta_msg->code, pmeta_msg->len);
        return;
    }

#if defined(MTK_AVRCP_TG_15_BROWSE)&&(MTK_AVRCP_TG_15_BROWSE == TRUE)
    release_transaction(pmeta_msg->label);
#endif
}

/***************************************************************************
**
** Function         handle_avk_rc_metamsg_cmd
**
** Description      Handle RC metamessage response
**
** Returns          void
**
***************************************************************************/
static void handle_avk_rc_metamsg_cmd(tBTA_AV_META_MSG *pmeta_msg)
{
    tAVRC_COMMAND    avrc_cmd = {0};
    tAVRC_STS status = BT_STATUS_UNSUPPORTED;
    BTIF_TRACE_DEBUG("%s opcode = %d rsp_code = %d  ",__FUNCTION__,
                     pmeta_msg->p_msg->hdr.opcode,pmeta_msg->code);
#if defined(MTK_AVRCP_TG_15_BROWSE)&&(MTK_AVRCP_TG_15_BROWSE == TRUE)
    status = AVRC_Ctrl_ParsCommand(pmeta_msg->p_msg, &avrc_cmd);
#endif

    if ((AVRC_OP_VENDOR==pmeta_msg->p_msg->hdr.opcode)&&
                (pmeta_msg->code <= AVRC_CMD_GEN_INQ))
    {
#if (defined(MTK_AVRCP_TG_15_BROWSE)&&(MTK_AVRCP_TG_15_BROWSE == FALSE))||(!defined MTK_AVRCP_TG_15_BROWSE)
        status = AVRC_Ctrl_ParsCommand(pmeta_msg->p_msg, &avrc_cmd);
#endif
        BTIF_TRACE_DEBUG("%s Received vendor command.code %d, PDU %d label %d",
                         __FUNCTION__, pmeta_msg->code, avrc_cmd.pdu, pmeta_msg->label);

        if (status != AVRC_STS_NO_ERROR)
        {
            /* return error */
            BTIF_TRACE_WARNING("%s: Error in parsing received metamsg command. status: 0x%02x",
                __FUNCTION__, status);
            send_reject_response(pmeta_msg->rc_handle, pmeta_msg->label, avrc_cmd.pdu, status);
        }
        else
        {
            if (avrc_cmd.pdu == AVRC_PDU_REGISTER_NOTIFICATION)
            {
                UINT8 event_id = avrc_cmd.reg_notif.event_id;
                BTIF_TRACE_EVENT("%s:Register notification event_id: %s",
                        __FUNCTION__, dump_rc_notification_event_id(event_id));
            }
            else if (avrc_cmd.pdu == AVRC_PDU_SET_ABSOLUTE_VOLUME)
            {
                BTIF_TRACE_EVENT("%s: Abs Volume Cmd Recvd", __FUNCTION__);
            }
            btif_rc_ctrl_upstreams_rsp_cmd(avrc_cmd.pdu, &avrc_cmd, pmeta_msg->label);
        }
    }
    else
    {
      BTIF_TRACE_DEBUG("%s:Invalid Vendor Command  code: %d len: %d. Not processing it.",
                       __FUNCTION__, pmeta_msg->code, pmeta_msg->len);
        return;
    }
}
#endif

/***************************************************************************
**
** Function         cleanup
**
** Description      Closes the AVRC interface
**
** Returns          void
**
***************************************************************************/
static void cleanup()
{
    BTIF_TRACE_EVENT("## %s ##", __FUNCTION__);
    close_uinput();
    if (bt_rc_callbacks)
    {
        bt_rc_callbacks = NULL;
    }
    alarm_free(btif_rc_cb.rc_play_status_timer);
    memset(&btif_rc_cb, 0, sizeof(btif_rc_cb_t));
    lbl_destroy();
    BTIF_TRACE_EVENT("## %s ## completed", __FUNCTION__);
}

/***************************************************************************
**
** Function         cleanup_ctrl
**
** Description      Closes the AVRC Controller interface
**
** Returns          void
**
***************************************************************************/
static void cleanup_ctrl()
{
    BTIF_TRACE_EVENT("## %s ##", __FUNCTION__);

    if (bt_rc_ctrl_callbacks)
    {
        bt_rc_ctrl_callbacks = NULL;
    }
    alarm_free(btif_rc_cb.rc_play_status_timer);
    memset(&btif_rc_cb, 0, sizeof(btif_rc_cb_t));
    lbl_destroy();
    BTIF_TRACE_EVENT("## %s ## completed", __FUNCTION__);
}

/***************************************************************************
**
** Function         getcapabilities_cmd
**
** Description      GetCapabilties from Remote(Company_ID, Events_Supported)
**
** Returns          void
**
***************************************************************************/
static bt_status_t getcapabilities_cmd (uint8_t cap_id)
{
    tAVRC_STS status = BT_STATUS_UNSUPPORTED;
    rc_transaction_t *p_transaction = NULL;
#if (AVRC_CTLR_INCLUDED == TRUE)
    BTIF_TRACE_DEBUG("%s: cap_id %d", __FUNCTION__, cap_id);
    CHECK_RC_CONNECTED
    bt_status_t tran_status=get_transaction(&p_transaction);
    if (BT_STATUS_SUCCESS != tran_status)
        return BT_STATUS_FAIL;

     tAVRC_COMMAND avrc_cmd = {0};
     BT_HDR *p_msg = NULL;
     avrc_cmd.get_caps.opcode = AVRC_OP_VENDOR;
     avrc_cmd.get_caps.capability_id = cap_id;
     avrc_cmd.get_caps.pdu = AVRC_PDU_GET_CAPABILITIES;
     avrc_cmd.get_caps.status = AVRC_STS_NO_ERROR;
     status = AVRC_BldCommand(&avrc_cmd, &p_msg);
     if ((status == AVRC_STS_NO_ERROR)&&(p_msg != NULL))
     {
         UINT8* data_start = (UINT8*)(p_msg + 1) + p_msg->offset;
         BTIF_TRACE_DEBUG("%s msgreq being sent out with label %d",
                            __FUNCTION__,p_transaction->lbl);
         BTA_AvVendorCmd(btif_rc_cb.rc_handle,p_transaction->lbl,AVRC_CMD_STATUS,
                                                          data_start, p_msg->len);
         status =  BT_STATUS_SUCCESS;
         start_status_command_timer (AVRC_PDU_GET_CAPABILITIES, p_transaction);
     }
     else
     {
         BTIF_TRACE_ERROR("%s: failed to build command. status: 0x%02x",
                             __FUNCTION__, status);
     }
     osi_free(p_msg);
#else
    BTIF_TRACE_DEBUG("%s: feature not enabled", __FUNCTION__);
#endif
    return status;
}

/***************************************************************************
**
** Function         list_player_app_setting_attrib_cmd
**
** Description      Get supported List Player Attributes
**
** Returns          void
**
***************************************************************************/
static bt_status_t list_player_app_setting_attrib_cmd(void)
{
    tAVRC_STS status = BT_STATUS_UNSUPPORTED;
    rc_transaction_t *p_transaction = NULL;
#if (AVRC_CTLR_INCLUDED == TRUE)
    BTIF_TRACE_DEBUG("%s: ", __FUNCTION__);
    CHECK_RC_CONNECTED
    bt_status_t tran_status=get_transaction(&p_transaction);
    if (BT_STATUS_SUCCESS != tran_status)
        return BT_STATUS_FAIL;

     tAVRC_COMMAND avrc_cmd = {0};
     BT_HDR *p_msg = NULL;
     avrc_cmd.list_app_attr.opcode = AVRC_OP_VENDOR;
     avrc_cmd.list_app_attr.pdu = AVRC_PDU_LIST_PLAYER_APP_ATTR;
     avrc_cmd.list_app_attr.status = AVRC_STS_NO_ERROR;
     status = AVRC_BldCommand(&avrc_cmd, &p_msg);
     if ((status == AVRC_STS_NO_ERROR)&&(p_msg != NULL))
     {
         UINT8* data_start = (UINT8*)(p_msg + 1) + p_msg->offset;
         BTIF_TRACE_DEBUG("%s msgreq being sent out with label %d",
                            __FUNCTION__,p_transaction->lbl);
         BTA_AvVendorCmd(btif_rc_cb.rc_handle,p_transaction->lbl,AVRC_CMD_STATUS,
                                                          data_start, p_msg->len);
         status =  BT_STATUS_SUCCESS;
         start_status_command_timer (AVRC_PDU_LIST_PLAYER_APP_ATTR, p_transaction);
     }
     else
     {

         BTIF_TRACE_ERROR("%s: failed to build command. status: 0x%02x",
                            __FUNCTION__, status);
     }
     osi_free(p_msg);
#else
    BTIF_TRACE_DEBUG("%s: feature not enabled", __FUNCTION__);
#endif
    return status;
}

/***************************************************************************
**
** Function         list_player_app_setting_value_cmd
**
** Description      Get values of supported Player Attributes
**
** Returns          void
**
***************************************************************************/
static bt_status_t list_player_app_setting_value_cmd(uint8_t attrib_id)
{
    tAVRC_STS status = BT_STATUS_UNSUPPORTED;
    rc_transaction_t *p_transaction=NULL;
#if (AVRC_CTLR_INCLUDED == TRUE)
    BTIF_TRACE_DEBUG("%s: attrib_id %d", __FUNCTION__, attrib_id);
    CHECK_RC_CONNECTED
    bt_status_t tran_status=get_transaction(&p_transaction);
    if (BT_STATUS_SUCCESS != tran_status)
        return BT_STATUS_FAIL;

     tAVRC_COMMAND avrc_cmd = {0};
     BT_HDR *p_msg = NULL;
     avrc_cmd.list_app_values.attr_id = attrib_id;
     avrc_cmd.list_app_values.opcode = AVRC_OP_VENDOR;
     avrc_cmd.list_app_values.pdu = AVRC_PDU_LIST_PLAYER_APP_VALUES;
     avrc_cmd.list_app_values.status = AVRC_STS_NO_ERROR;
     status = AVRC_BldCommand(&avrc_cmd, &p_msg);
     if ((status == AVRC_STS_NO_ERROR) && (p_msg != NULL))
     {
         UINT8* data_start = (UINT8*)(p_msg + 1) + p_msg->offset;
         BTIF_TRACE_DEBUG("%s msgreq being sent out with label %d",
                            __FUNCTION__,p_transaction->lbl);
         BTA_AvVendorCmd(btif_rc_cb.rc_handle,p_transaction->lbl,AVRC_CMD_STATUS,
                               data_start, p_msg->len);
         status =  BT_STATUS_SUCCESS;
         start_status_command_timer (AVRC_PDU_LIST_PLAYER_APP_VALUES, p_transaction);
     }
     else
     {
         BTIF_TRACE_ERROR("%s: failed to build command. status: 0x%02x", __FUNCTION__, status);
     }
     osi_free(p_msg);
#else
    BTIF_TRACE_DEBUG("%s: feature not enabled", __FUNCTION__);
#endif
    return status;
}

#if defined(MTK_AVRCP_TG_15_BROWSE)&&(MTK_AVRCP_TG_15_BROWSE == TRUE)
/***************************************************************************
 *
 * Function         get_playback_state_cmd
 *
 * Description      Fetch the current playback state for the device
 *
 * Returns          BT_STATUS_SUCCESS if command issued successfully otherwise
 *                  BT_STATUS_FAIL.
 *
 **************************************************************************/
static bt_status_t get_playback_state_cmd(bt_bdaddr_t* bd_addr) {
  BTIF_TRACE_DEBUG("%s", __func__);
  return get_play_status_cmd();
}

/***************************************************************************
 *
 * Function         get_now_playing_list_cmd
 *
 * Description      Fetch the now playing list
 *
 * Paramters        start_item: First item to fetch (0 to fetch from beganning)
 *                  end_item: Last item to fetch (0xff to fetch until end)
 *
 * Returns          BT_STATUS_SUCCESS if command issued successfully otherwise
 *                  BT_STATUS_FAIL.
 *
 **************************************************************************/
static bt_status_t get_now_playing_list_cmd(bt_bdaddr_t* bd_addr,
                                            uint8_t start_item,
                                            uint8_t num_items) {
  BTIF_TRACE_DEBUG("%s start, end: (%d, %d)", __func__, start_item, num_items);
  return get_folder_items_cmd(bd_addr, AVRC_SCOPE_NOW_PLAYING, start_item,
                              num_items);
}

/***************************************************************************
 *
 * Function         get_folder_list_cmd
 *
 * Description      Fetch the currently selected folder list
 *
 * Paramters        start_item: First item to fetch (0 to fetch from beganning)
 *                  end_item: Last item to fetch (0xff to fetch until end)
 *
 * Returns          BT_STATUS_SUCCESS if command issued successfully otherwise
 *                  BT_STATUS_FAIL.
 *
 **************************************************************************/
static bt_status_t get_folder_list_cmd(bt_bdaddr_t* bd_addr, uint8_t start_item,
                                       uint8_t num_items) {
  BTIF_TRACE_DEBUG("%s start, end: (%d, %d)", __func__, start_item, num_items);
  return get_folder_items_cmd(bd_addr, AVRC_SCOPE_FILE_SYSTEM, start_item,
                              num_items);
}

/***************************************************************************
 *
 * Function         get_player_list_cmd
 *
 * Description      Fetch the player list
 *
 * Paramters        start_item: First item to fetch (0 to fetch from beganning)
 *                  end_item: Last item to fetch (0xff to fetch until end)
 *
 * Returns          BT_STATUS_SUCCESS if command issued successfully otherwise
 *                  BT_STATUS_FAIL.
 *
 **************************************************************************/
static bt_status_t get_player_list_cmd(bt_bdaddr_t* bd_addr, uint8_t start_item,
                                       uint8_t num_items) {
  BTIF_TRACE_DEBUG("%s start, end: (%d, %d)", __func__, start_item, num_items);
  return get_folder_items_cmd(bd_addr, AVRC_SCOPE_PLAYER_LIST, start_item,
                              num_items);
}

/***************************************************************************
 *
 * Function         change_folder_path_cmd
 *
 * Description      Change the folder.
 *
 * Paramters        direction: Direction (Up/Down) to change folder
 *                  uid: The UID of folder to move to
 *                  start_item: First item to fetch (0 to fetch from beganning)
 *                  end_item: Last item to fetch (0xff to fetch until end)
 *
 * Returns          BT_STATUS_SUCCESS if command issued successfully otherwise
 *                  BT_STATUS_FAIL.
 *
 **************************************************************************/
static bt_status_t change_folder_path_cmd(bt_bdaddr_t* bd_addr,
                                          uint8_t direction, uint8_t* uid) {
  BTIF_TRACE_DEBUG("%s: direction %d", __func__, direction);

  CHECK_RC_CONNECTED
  CHECK_BR_CONNECTED

  tAVRC_COMMAND avrc_cmd = {0};

  avrc_cmd.chg_path.pdu = AVRC_PDU_CHANGE_PATH;
  avrc_cmd.chg_path.status = AVRC_STS_NO_ERROR;
  // TODO(sanketa): Improve for database aware clients.
  avrc_cmd.chg_path.uid_counter = 0;
  avrc_cmd.chg_path.direction = direction;

  memset(avrc_cmd.chg_path.folder_uid, 0, AVRC_UID_SIZE * sizeof(uint8_t));
  memcpy(avrc_cmd.chg_path.folder_uid, uid, AVRC_UID_SIZE * sizeof(uint8_t));

  BT_HDR* p_msg = NULL;
  tAVRC_STS status = AVRC_BldCommand(&avrc_cmd, &p_msg);
  if (status != AVRC_STS_NO_ERROR) {
    BTIF_TRACE_ERROR("%s failed to build command status %d", __func__, status);
    return BT_STATUS_FAIL;
  }

  rc_transaction_t* p_transaction = NULL;
  bt_status_t tran_status = get_transaction(&p_transaction);
  if (tran_status != BT_STATUS_SUCCESS || p_transaction == NULL) {
    osi_free(p_msg);
    BTIF_TRACE_ERROR("%s: failed to obtain transaction details. status: 0x%02x",
                     __func__, tran_status);
    return BT_STATUS_FAIL;
  }

  BTIF_TRACE_DEBUG("%s msgreq being sent out with label %d", __func__,
                   p_transaction->lbl);
  BTA_AvMetaCmd(btif_rc_cb.rc_handle, p_transaction->lbl, AVRC_CMD_CTRL, p_msg);
  return BT_STATUS_SUCCESS;
}

/***************************************************************************
 *
 * Function         set_browsed_player_cmd
 *
 * Description      Change the browsed player.
 *
 * Paramters        id: The UID of player to move to
 *
 * Returns          BT_STATUS_SUCCESS if command issued successfully otherwise
 *                  BT_STATUS_FAIL.
 *
 **************************************************************************/
static bt_status_t set_browsed_player_cmd(bt_bdaddr_t* bd_addr, uint16_t id) {
  BTIF_TRACE_DEBUG("%s: id %d", __func__, id);
  CHECK_RC_CONNECTED
  CHECK_BR_CONNECTED

  rc_transaction_t* p_transaction = NULL;

  tAVRC_COMMAND avrc_cmd = {0};
  avrc_cmd.br_player.pdu = AVRC_PDU_SET_BROWSED_PLAYER;
  avrc_cmd.br_player.status = AVRC_STS_NO_ERROR;
  // TODO(sanketa): Improve for database aware clients.
  avrc_cmd.br_player.player_id = id;

  BT_HDR* p_msg = NULL;
  tAVRC_STS status = AVRC_BldCommand(&avrc_cmd, &p_msg);
  if (status != AVRC_STS_NO_ERROR) {
    BTIF_TRACE_ERROR("%s failed to build command status %d", __func__, status);
    return BT_STATUS_FAIL;
  }

  bt_status_t tran_status = get_transaction(&p_transaction);
  if (tran_status != BT_STATUS_SUCCESS || p_transaction == NULL) {
    osi_free(p_msg);
    BTIF_TRACE_ERROR("%s: failed to obtain transaction details. status: 0x%02x",
                     __func__, tran_status);
    return BT_STATUS_FAIL;
  }

  BTIF_TRACE_DEBUG("%s msgreq being sent out with label %d", __func__,
                   p_transaction->lbl);
  BTA_AvMetaCmd(btif_rc_cb.rc_handle, p_transaction->lbl, AVRC_CMD_CTRL, p_msg);
  return BT_STATUS_SUCCESS;
}

/***************************************************************************
 **
 ** Function         set_addressed_player_cmd
 **
 ** Description      Change the addressed player.
 **
 ** Paramters        id: The UID of player to move to
 **
 ** Returns          BT_STATUS_SUCCESS if command issued successfully otherwise
 **                  BT_STATUS_FAIL.
 **
 ***************************************************************************/
static bt_status_t set_addressed_player_cmd(bt_bdaddr_t* bd_addr, uint16_t id) {
  BTIF_TRACE_DEBUG("%s: id %d", __func__, id);

  CHECK_RC_CONNECTED
  CHECK_BR_CONNECTED

  tAVRC_COMMAND avrc_cmd = {0};
  BT_HDR* p_msg = NULL;

  avrc_cmd.addr_player.pdu = AVRC_PDU_SET_ADDRESSED_PLAYER;
  avrc_cmd.addr_player.status = AVRC_STS_NO_ERROR;
  // TODO(sanketa): Improve for database aware clients.
  avrc_cmd.addr_player.player_id = id;

  tAVRC_STS status = AVRC_BldCommand(&avrc_cmd, &p_msg);
  if (status != AVRC_STS_NO_ERROR) {
    BTIF_TRACE_ERROR("%s: failed to build command status %d", __func__, status);
    return BT_STATUS_FAIL;
  }

  rc_transaction_t* p_transaction = NULL;
  bt_status_t tran_status = get_transaction(&p_transaction);

  if (tran_status != BT_STATUS_SUCCESS || p_transaction == NULL) {
    osi_free(p_msg);
    BTIF_TRACE_ERROR("%s: failed to obtain txn details. status: 0x%02x",
                     __func__, tran_status);
    return BT_STATUS_FAIL;
  }

  BTIF_TRACE_DEBUG("%s msgreq being sent out with label %d", __func__,
                   p_transaction->lbl);
  BTA_AvMetaCmd(btif_rc_cb.rc_handle, p_transaction->lbl, AVRC_CMD_CTRL, p_msg);
  return BT_STATUS_SUCCESS;
}

/***************************************************************************
 *
 * Function         get_folder_items_cmd
 *
 * Description      Helper function to browse the content hierarchy of the
 *                  TG device.
 *
 * Paramters        scope: AVRC_SCOPE_NOW_PLAYING (etc) for various browseable
 *                  content
 *                  start_item: First item to fetch (0 to fetch from beganning)
 *                  end_item: Last item to fetch (0xff to fetch until end)
 *
 * Returns          BT_STATUS_SUCCESS if command issued successfully otherwise
 *                  BT_STATUS_FAIL.
 *
 **************************************************************************/
static bt_status_t get_folder_items_cmd(bt_bdaddr_t* bd_addr, uint8_t scope,
                                        uint8_t start_item, uint8_t end_item) {
  /* Check that both avrcp and browse channel are connected. */
  BTIF_TRACE_DEBUG("%s", __func__);
  CHECK_RC_CONNECTED
  CHECK_BR_CONNECTED

  tAVRC_COMMAND avrc_cmd = {0};

  /* Set the layer specific to point to browse although this should really
   * be done by lower layers and looking at the PDU
   */
  avrc_cmd.get_items.pdu = AVRC_PDU_GET_FOLDER_ITEMS;
  avrc_cmd.get_items.status = AVRC_STS_NO_ERROR;
  avrc_cmd.get_items.scope = scope;
  avrc_cmd.get_items.start_item = start_item;
  avrc_cmd.get_items.end_item = end_item;
  avrc_cmd.get_items.attr_count = 0; /* p_attr_list does not matter hence */

  BT_HDR* p_msg = NULL;
  tAVRC_STS status = AVRC_BldCommand(&avrc_cmd, &p_msg);
  if (status != AVRC_STS_NO_ERROR) {
    BTIF_TRACE_ERROR("%s failed to build command status %d", __func__, status);
    return BT_STATUS_FAIL;
  }

  rc_transaction_t* p_transaction = NULL;
  bt_status_t tran_status = get_transaction(&p_transaction);
  if (tran_status != BT_STATUS_SUCCESS || p_transaction == NULL) {
    osi_free(p_msg);
    BTIF_TRACE_ERROR("%s: failed to obtain transaction details. status: 0x%02x",
                     __func__, tran_status);
    return BT_STATUS_FAIL;
  }

  BTIF_TRACE_DEBUG("%s msgreq being sent out with label %d", __func__,
                   p_transaction->lbl);
  BTA_AvMetaCmd(btif_rc_cb.rc_handle, p_transaction->lbl, AVRC_CMD_CTRL, p_msg);
  return BT_STATUS_SUCCESS;
}

/***************************************************************************
 *
 * Function         play_item_cmd
 *
 * Description      Play the item specified by UID & scope
 *
 * Returns          void
 *
 **************************************************************************/
static bt_status_t play_item_cmd(bt_bdaddr_t* bd_addr, uint8_t scope,
                                 uint8_t* uid, uint16_t uid_counter) {
  BTIF_TRACE_DEBUG("%s: scope %d uid_counter %d", __func__, scope, uid_counter);
  CHECK_RC_CONNECTED
  CHECK_BR_CONNECTED

  tAVRC_COMMAND avrc_cmd = {0};
  avrc_cmd.pdu = AVRC_PDU_PLAY_ITEM;
  avrc_cmd.play_item.opcode = AVRC_OP_VENDOR;
  avrc_cmd.play_item.status = AVRC_STS_NO_ERROR;
  avrc_cmd.play_item.scope = scope;
  memcpy(avrc_cmd.play_item.uid, uid, AVRC_UID_SIZE);
  avrc_cmd.play_item.uid_counter = uid_counter;

  return build_and_send_vendor_cmd(&avrc_cmd, AVRC_CMD_CTRL);
}
#endif //(end -- #if defined(MTK_AVRCP_TG_15_BROWSE))

/***************************************************************************
**
** Function         get_player_app_setting_cmd
**
** Description      Get current values of Player Attributes
**
** Returns          void
**
***************************************************************************/
static bt_status_t get_player_app_setting_cmd(uint8_t num_attrib, uint8_t* attrib_ids)
{
    tAVRC_STS status = BT_STATUS_UNSUPPORTED;
    rc_transaction_t *p_transaction = NULL;
    int count  = 0;
#if (AVRC_CTLR_INCLUDED == TRUE)
    BTIF_TRACE_DEBUG("%s: num attrib_id %d", __FUNCTION__, num_attrib);
    CHECK_RC_CONNECTED
    bt_status_t tran_status=get_transaction(&p_transaction);
    if (BT_STATUS_SUCCESS != tran_status)
        return BT_STATUS_FAIL;

     tAVRC_COMMAND avrc_cmd = {0};
     BT_HDR *p_msg = NULL;
     avrc_cmd.get_cur_app_val.opcode = AVRC_OP_VENDOR;
     avrc_cmd.get_cur_app_val.status = AVRC_STS_NO_ERROR;
     avrc_cmd.get_cur_app_val.num_attr = num_attrib;
     avrc_cmd.get_cur_app_val.pdu = AVRC_PDU_GET_CUR_PLAYER_APP_VALUE;

     for (count = 0; count < num_attrib; count++)
     {
         avrc_cmd.get_cur_app_val.attrs[count] = attrib_ids[count];
     }
     status = AVRC_BldCommand(&avrc_cmd, &p_msg);
     if ((status == AVRC_STS_NO_ERROR) && (p_msg != NULL))
     {
         UINT8* data_start = (UINT8*)(p_msg + 1) + p_msg->offset;
         BTIF_TRACE_DEBUG("%s msgreq being sent out with label %d",
                            __FUNCTION__,p_transaction->lbl);
         BTA_AvVendorCmd(btif_rc_cb.rc_handle,p_transaction->lbl,AVRC_CMD_STATUS,
                          data_start, p_msg->len);
         status =  BT_STATUS_SUCCESS;
         start_status_command_timer (AVRC_PDU_GET_CUR_PLAYER_APP_VALUE, p_transaction);
     }
     else
     {
         BTIF_TRACE_ERROR("%s: failed to build command. status: 0x%02x",
                            __FUNCTION__, status);
     }
     osi_free(p_msg);
#else
    BTIF_TRACE_DEBUG("%s: feature not enabled", __FUNCTION__);
#endif
    return status;
}

/***************************************************************************
**
** Function         change_player_app_setting
**
** Description      Set current values of Player Attributes
**
** Returns          void
**
***************************************************************************/
static bt_status_t change_player_app_setting(bt_bdaddr_t *bd_addr, uint8_t num_attrib, uint8_t* attrib_ids, uint8_t* attrib_vals)
{
    tAVRC_STS status = BT_STATUS_UNSUPPORTED;
    rc_transaction_t *p_transaction = NULL;
    int count  = 0;
#if (AVRC_CTLR_INCLUDED == TRUE)
    BTIF_TRACE_DEBUG("%s: num attrib_id %d", __FUNCTION__, num_attrib);
    CHECK_RC_CONNECTED
    bt_status_t tran_status=get_transaction(&p_transaction);
    if (BT_STATUS_SUCCESS != tran_status)
        return BT_STATUS_FAIL;

     tAVRC_COMMAND avrc_cmd = {0};
     BT_HDR *p_msg = NULL;
     avrc_cmd.set_app_val.opcode = AVRC_OP_VENDOR;
     avrc_cmd.set_app_val.status = AVRC_STS_NO_ERROR;
     avrc_cmd.set_app_val.num_val = num_attrib;
     avrc_cmd.set_app_val.pdu = AVRC_PDU_SET_PLAYER_APP_VALUE;
     avrc_cmd.set_app_val.p_vals =
           (tAVRC_APP_SETTING *)osi_malloc(sizeof(tAVRC_APP_SETTING) * num_attrib);
     for (count = 0; count < num_attrib; count++)
     {
         avrc_cmd.set_app_val.p_vals[count].attr_id = attrib_ids[count];
         avrc_cmd.set_app_val.p_vals[count].attr_val = attrib_vals[count];
     }
     status = AVRC_BldCommand(&avrc_cmd, &p_msg);
     if ((status == AVRC_STS_NO_ERROR) && (p_msg != NULL))
     {
         UINT8* data_start = (UINT8*)(p_msg + 1) + p_msg->offset;
         BTIF_TRACE_DEBUG("%s msgreq being sent out with label %d",
                            __FUNCTION__,p_transaction->lbl);
         BTA_AvVendorCmd(btif_rc_cb.rc_handle,p_transaction->lbl,AVRC_CMD_CTRL,
                              data_start, p_msg->len);
         status =  BT_STATUS_SUCCESS;
         start_control_command_timer (AVRC_PDU_SET_PLAYER_APP_VALUE, p_transaction);
     }
     else
     {
         BTIF_TRACE_ERROR("%s: failed to build command. status: 0x%02x",
                            __FUNCTION__, status);
     }
     osi_free(p_msg);
     osi_free_and_reset((void **)&avrc_cmd.set_app_val.p_vals);
#else
    BTIF_TRACE_DEBUG("%s: feature not enabled", __FUNCTION__);
#endif
    return status;
}

/***************************************************************************
**
** Function         get_player_app_setting_attr_text_cmd
**
** Description      Get text description for app attribute
**
** Returns          void
**
***************************************************************************/
static bt_status_t get_player_app_setting_attr_text_cmd (UINT8 *attrs, UINT8 num_attrs)
{
    tAVRC_STS status = BT_STATUS_UNSUPPORTED;
    rc_transaction_t *p_transaction = NULL;
    int count  = 0;
#if (AVRC_CTLR_INCLUDED == TRUE)
    tAVRC_COMMAND avrc_cmd = {0};
    BT_HDR *p_msg = NULL;
    bt_status_t tran_status;
    CHECK_RC_CONNECTED

    BTIF_TRACE_DEBUG("%s: num attrs %d", __FUNCTION__, num_attrs);

    tran_status = get_transaction(&p_transaction);
    if (BT_STATUS_SUCCESS != tran_status)
        return BT_STATUS_FAIL;

    avrc_cmd.pdu = AVRC_PDU_GET_PLAYER_APP_ATTR_TEXT;
    avrc_cmd.get_app_attr_txt.opcode = AVRC_OP_VENDOR;
    avrc_cmd.get_app_attr_txt.num_attr = num_attrs;

    for (count = 0; count < num_attrs; count++)
    {
        avrc_cmd.get_app_attr_txt.attrs[count] = attrs[count];
    }
    status = AVRC_BldCommand(&avrc_cmd, &p_msg);
    if (status == AVRC_STS_NO_ERROR)
    {
        UINT8* data_start = (UINT8*)(p_msg + 1) + p_msg->offset;
                BTIF_TRACE_DEBUG("%s msgreq being sent out with label %d",
                __FUNCTION__, p_transaction->lbl);
        BTA_AvVendorCmd(btif_rc_cb.rc_handle, p_transaction->lbl,
                AVRC_CMD_STATUS, data_start, p_msg->len);
        osi_free(p_msg);
        status =  BT_STATUS_SUCCESS;
        start_status_command_timer (AVRC_PDU_GET_PLAYER_APP_ATTR_TEXT, p_transaction);
    }
    else
    {
        BTIF_TRACE_ERROR("%s: failed to build command. status: 0x%02x", __FUNCTION__, status);
    }
    osi_free(p_msg);
#else
    BTIF_TRACE_DEBUG("%s: feature not enabled", __FUNCTION__);
#endif
    return status;
}

/***************************************************************************
**
** Function         get_player_app_setting_val_text_cmd
**
** Description      Get text description for app attribute values
**
** Returns          void
**
***************************************************************************/
static bt_status_t get_player_app_setting_value_text_cmd (UINT8 *vals, UINT8 num_vals)
{
    tAVRC_STS status = BT_STATUS_UNSUPPORTED;
    rc_transaction_t *p_transaction = NULL;
    int count  = 0;
#if (AVRC_CTLR_INCLUDED == TRUE)
    tAVRC_COMMAND avrc_cmd = {0};
    BT_HDR *p_msg = NULL;
    bt_status_t tran_status;
    CHECK_RC_CONNECTED

    BTIF_TRACE_DEBUG("%s: num_vals %d", __FUNCTION__, num_vals);

    tran_status = get_transaction(&p_transaction);
    if (BT_STATUS_SUCCESS != tran_status)
        return BT_STATUS_FAIL;

    avrc_cmd.pdu = AVRC_PDU_GET_PLAYER_APP_VALUE_TEXT;
    avrc_cmd.get_app_val_txt.opcode = AVRC_OP_VENDOR;
    avrc_cmd.get_app_val_txt.num_val = num_vals;

    for (count = 0; count < num_vals; count++)
    {
        avrc_cmd.get_app_val_txt.vals[count] = vals[count];
    }
    status = AVRC_BldCommand(&avrc_cmd, &p_msg);
    if (status == AVRC_STS_NO_ERROR)
    {
        UINT8* data_start = (UINT8*)(p_msg + 1) + p_msg->offset;
        BTIF_TRACE_DEBUG("%s msgreq being sent out with label %d",
                         __FUNCTION__, p_transaction->lbl);
        if (p_msg != NULL)
        {
            BTA_AvVendorCmd(btif_rc_cb.rc_handle, p_transaction->lbl,
                    AVRC_CMD_STATUS, data_start, p_msg->len);
            status =  BT_STATUS_SUCCESS;
            start_status_command_timer (AVRC_PDU_GET_PLAYER_APP_VALUE_TEXT, p_transaction);
        }
    }
    else
    {
        BTIF_TRACE_ERROR("%s: failed to build command. status: 0x%02x",
                __FUNCTION__, status);
    }
    osi_free(p_msg);
#else
    BTIF_TRACE_DEBUG("%s: feature not enabled", __FUNCTION__);
#endif
    return status;
}

/***************************************************************************
**
** Function         register_notification_cmd
**
** Description      Send Command to register for a Notification ID
**
** Returns          void
**
***************************************************************************/
static bt_status_t register_notification_cmd (UINT8 label, UINT8 event_id, UINT32 event_value)
{

    tAVRC_STS status = BT_STATUS_UNSUPPORTED;
#if (AVRC_CTLR_INCLUDED == TRUE)
    tAVRC_COMMAND avrc_cmd = {0};
    BT_HDR *p_msg = NULL;
    CHECK_RC_CONNECTED


    BTIF_TRACE_DEBUG("%s: event_id %d  event_value", __FUNCTION__, event_id, event_value);

    avrc_cmd.reg_notif.opcode = AVRC_OP_VENDOR;
    avrc_cmd.reg_notif.status = AVRC_STS_NO_ERROR;
    avrc_cmd.reg_notif.event_id = event_id;
    avrc_cmd.reg_notif.pdu = AVRC_PDU_REGISTER_NOTIFICATION;
    avrc_cmd.reg_notif.param = event_value;
    status = AVRC_BldCommand(&avrc_cmd, &p_msg);
    if (status == AVRC_STS_NO_ERROR)
    {
        UINT8* data_start = (UINT8*)(p_msg + 1) + p_msg->offset;
        BTIF_TRACE_DEBUG("%s msgreq being sent out with label %d",
                __FUNCTION__, label);
        if (p_msg != NULL)
        {
            BTA_AvVendorCmd(btif_rc_cb.rc_handle, label, AVRC_CMD_NOTIF,
                    data_start, p_msg->len);
            status =  BT_STATUS_SUCCESS;
        }
    }
    else
    {
         BTIF_TRACE_ERROR("%s: failed to build command. status: 0x%02x",
                            __FUNCTION__, status);
    }
    osi_free(p_msg);
#else
    BTIF_TRACE_DEBUG("%s: feature not enabled", __FUNCTION__);
#endif
    return status;
}

/***************************************************************************
**
** Function         get_element_attribute_cmd
**
** Description      Get Element Attribute for  attributeIds
**
** Returns          void
**
***************************************************************************/
static bt_status_t get_element_attribute_cmd (uint8_t num_attribute, uint32_t *p_attr_ids)
{
    tAVRC_STS status = BT_STATUS_UNSUPPORTED;
    rc_transaction_t *p_transaction=NULL;
    int count  = 0;
#if (AVRC_CTLR_INCLUDED == TRUE)
    tAVRC_COMMAND avrc_cmd = {0};
    BT_HDR *p_msg = NULL;
    bt_status_t tran_status;
    CHECK_RC_CONNECTED

    BTIF_TRACE_DEBUG("%s: num_attribute  %d attribute_id %d",
                   __FUNCTION__, num_attribute, p_attr_ids[0]);

    tran_status = get_transaction(&p_transaction);
    if (BT_STATUS_SUCCESS != tran_status)
        return BT_STATUS_FAIL;

    avrc_cmd.get_elem_attrs.opcode = AVRC_OP_VENDOR;
    avrc_cmd.get_elem_attrs.status = AVRC_STS_NO_ERROR;
    avrc_cmd.get_elem_attrs.num_attr = num_attribute;
    avrc_cmd.get_elem_attrs.pdu = AVRC_PDU_GET_ELEMENT_ATTR;
    for (count = 0; count < num_attribute; count++)
    {
        avrc_cmd.get_elem_attrs.attrs[count] = p_attr_ids[count];
    }

    status = AVRC_BldCommand(&avrc_cmd, &p_msg);
    if (status == AVRC_STS_NO_ERROR)
    {
        UINT8* data_start = (UINT8*)(p_msg + 1) + p_msg->offset;
        BTIF_TRACE_DEBUG("%s msgreq being sent out with label %d",
                __FUNCTION__, p_transaction->lbl);
        if (p_msg != NULL)
        {
            BTA_AvVendorCmd(btif_rc_cb.rc_handle, p_transaction->lbl,
                    AVRC_CMD_STATUS, data_start, p_msg->len);
            status =  BT_STATUS_SUCCESS;
            start_status_command_timer (AVRC_PDU_GET_ELEMENT_ATTR,
                    p_transaction);
        }
    }
    else
    {
         BTIF_TRACE_ERROR("%s: failed to build command. status: 0x%02x",
                            __FUNCTION__, status);
    }
    osi_free(p_msg);
#else
    BTIF_TRACE_DEBUG("%s: feature not enabled", __FUNCTION__);
#endif
    return status;
}

/***************************************************************************
**
** Function         get_play_status_cmd
**
** Description      Get Element Attribute for  attributeIds
**
** Returns          void
**
***************************************************************************/
static bt_status_t get_play_status_cmd(void)
{
    tAVRC_STS status = BT_STATUS_UNSUPPORTED;
    rc_transaction_t *p_transaction = NULL;
#if (AVRC_CTLR_INCLUDED == TRUE)
    tAVRC_COMMAND avrc_cmd = {0};
    BT_HDR *p_msg = NULL;
    bt_status_t tran_status;
    CHECK_RC_CONNECTED

    BTIF_TRACE_DEBUG("%s: ", __FUNCTION__);
    tran_status = get_transaction(&p_transaction);
    if (BT_STATUS_SUCCESS != tran_status)
        return BT_STATUS_FAIL;

    avrc_cmd.get_play_status.opcode = AVRC_OP_VENDOR;
    avrc_cmd.get_play_status.pdu = AVRC_PDU_GET_PLAY_STATUS;
    avrc_cmd.get_play_status.status = AVRC_STS_NO_ERROR;
    status = AVRC_BldCommand(&avrc_cmd, &p_msg);
    if (status == AVRC_STS_NO_ERROR)
    {
        UINT8* data_start = (UINT8*)(p_msg + 1) + p_msg->offset;
        BTIF_TRACE_DEBUG("%s msgreq being sent out with label %d",
                __FUNCTION__, p_transaction->lbl);
        if (p_msg != NULL)
        {
            BTA_AvVendorCmd(btif_rc_cb.rc_handle,p_transaction->lbl,
                    AVRC_CMD_STATUS, data_start, p_msg->len);
            status =  BT_STATUS_SUCCESS;
            start_status_command_timer (AVRC_PDU_GET_PLAY_STATUS, p_transaction);
        }
    }
    else
    {
         BTIF_TRACE_ERROR("%s: failed to build command. status: 0x%02x",
                            __FUNCTION__, status);
    }
    osi_free(p_msg);
#else
    BTIF_TRACE_DEBUG("%s: feature not enabled", __FUNCTION__);
#endif
    return status;

}

/***************************************************************************
**
** Function         set_volume_rsp
**
** Description      Rsp for SetAbsoluteVolume Command
**
** Returns          void
**
***************************************************************************/
static bt_status_t set_volume_rsp(bt_bdaddr_t *bd_addr, uint8_t abs_vol, uint8_t label)
{
    tAVRC_STS status = BT_STATUS_UNSUPPORTED;
#if (AVRC_CTLR_INCLUDED == TRUE)
    tAVRC_RESPONSE avrc_rsp;
    BT_HDR *p_msg = NULL;
    CHECK_RC_CONNECTED

    BTIF_TRACE_DEBUG("%s: abs_vol %d", __FUNCTION__, abs_vol);

    avrc_rsp.volume.opcode = AVRC_OP_VENDOR;
    avrc_rsp.volume.pdu = AVRC_PDU_SET_ABSOLUTE_VOLUME;
    avrc_rsp.volume.status = AVRC_STS_NO_ERROR;
    avrc_rsp.volume.volume = abs_vol;
    status = AVRC_BldResponse(btif_rc_cb.rc_handle, &avrc_rsp, &p_msg);
    if (status == AVRC_STS_NO_ERROR)
    {
        UINT8* data_start = (UINT8*)(p_msg + 1) + p_msg->offset;
        BTIF_TRACE_DEBUG("%s msgreq being sent out with label %d",
                __FUNCTION__, btif_rc_cb.rc_vol_label);
        if (p_msg != NULL)
        {
            BTA_AvVendorRsp(btif_rc_cb.rc_handle, label,
                    BTA_AV_RSP_ACCEPT, data_start, p_msg->len, 0);
            status =  BT_STATUS_SUCCESS;
        }
    }
    else
    {
         BTIF_TRACE_ERROR("%s: failed to build command. status: 0x%02x",
                            __FUNCTION__, status);
    }
    osi_free(p_msg);
#else
    BTIF_TRACE_DEBUG("%s: feature not enabled", __FUNCTION__);
#endif
    return status;
}

/***************************************************************************
**
** Function         send_register_abs_vol_rsp
**
** Description      Rsp for Notification of Absolute Volume
**
** Returns          void
**
***************************************************************************/
static bt_status_t volume_change_notification_rsp(bt_bdaddr_t *bd_addr, btrc_notification_type_t rsp_type,
            uint8_t abs_vol, uint8_t label)
{
    tAVRC_STS status = BT_STATUS_UNSUPPORTED;
    tAVRC_RESPONSE avrc_rsp;
    BT_HDR *p_msg = NULL;
#if (AVRC_CTLR_INCLUDED == TRUE)
    BTIF_TRACE_DEBUG("%s: rsp_type  %d abs_vol %d", __func__, rsp_type, abs_vol);
    CHECK_RC_CONNECTED

    avrc_rsp.reg_notif.opcode = AVRC_OP_VENDOR;
    avrc_rsp.reg_notif.pdu = AVRC_PDU_REGISTER_NOTIFICATION;
    avrc_rsp.reg_notif.status = AVRC_STS_NO_ERROR;
    avrc_rsp.reg_notif.param.volume = abs_vol;
    avrc_rsp.reg_notif.event_id = AVRC_EVT_VOLUME_CHANGE;

    status = AVRC_BldResponse(btif_rc_cb.rc_handle, &avrc_rsp, &p_msg);
    if (status == AVRC_STS_NO_ERROR) {
        BTIF_TRACE_DEBUG("%s msgreq being sent out with label %d",
                         __func__, label);
        UINT8* data_start = (UINT8*)(p_msg + 1) + p_msg->offset;
        BTA_AvVendorRsp(btif_rc_cb.rc_handle, label,
                        (rsp_type == BTRC_NOTIFICATION_TYPE_INTERIM) ?
                            AVRC_RSP_INTERIM : AVRC_RSP_CHANGED,
                        data_start, p_msg->len, 0);
        status = BT_STATUS_SUCCESS;
    } else {
        BTIF_TRACE_ERROR("%s: failed to build command. status: 0x%02x",
                         __func__, status);
    }
    osi_free(p_msg);

#else
    BTIF_TRACE_DEBUG("%s: feature not enabled", __func__);
#endif
    return status;
}

/***************************************************************************
**
** Function         send_groupnavigation_cmd
**
** Description      Send Pass-Through command
**
** Returns          void
**
***************************************************************************/
static bt_status_t send_groupnavigation_cmd(bt_bdaddr_t *bd_addr, uint8_t key_code,
                                            uint8_t key_state)
{
    tAVRC_STS status = BT_STATUS_UNSUPPORTED;
#if (AVRC_CTLR_INCLUDED == TRUE)
    rc_transaction_t *p_transaction=NULL;
    BTIF_TRACE_DEBUG("%s: key-code: %d, key-state: %d", __FUNCTION__,
                                                    key_code, key_state);
    CHECK_RC_CONNECTED
    if (btif_rc_cb.rc_features & BTA_AV_FEAT_RCTG)
    {
        bt_status_t tran_status = get_transaction(&p_transaction);
        if ((BT_STATUS_SUCCESS == tran_status) && (NULL != p_transaction)) {
             UINT8 buffer[AVRC_PASS_THRU_GROUP_LEN] = {0};
             UINT8* start = buffer;
             UINT24_TO_BE_STREAM(start, AVRC_CO_METADATA);
             *(start)++ = 0;
             UINT8_TO_BE_STREAM(start, key_code);
             BTA_AvRemoteVendorUniqueCmd(btif_rc_cb.rc_handle,
                                         p_transaction->lbl,
                                         (tBTA_AV_STATE)key_state, buffer,
                                         AVRC_PASS_THRU_GROUP_LEN);
             status =  BT_STATUS_SUCCESS;
             BTIF_TRACE_DEBUG("%s: succesfully sent group_navigation command to BTA",
                              __FUNCTION__);
        }
        else
        {
            status =  BT_STATUS_FAIL;
            BTIF_TRACE_DEBUG("%s: error in fetching transaction", __FUNCTION__);
        }
    }
    else
    {
        status =  BT_STATUS_FAIL;
        BTIF_TRACE_DEBUG("%s: feature not supported", __FUNCTION__);
    }
#else
    BTIF_TRACE_DEBUG("%s: feature not enabled", __FUNCTION__);
#endif
    return status;
}

/***************************************************************************
**
** Function         send_passthrough_cmd
**
** Description      Send Pass-Through command
**
** Returns          void
**
***************************************************************************/
static bt_status_t send_passthrough_cmd(bt_bdaddr_t *bd_addr, uint8_t key_code, uint8_t key_state)
{
    tAVRC_STS status = BT_STATUS_UNSUPPORTED;
#if (AVRC_CTLR_INCLUDED == TRUE)
    CHECK_RC_CONNECTED
    rc_transaction_t *p_transaction=NULL;
    BTIF_TRACE_DEBUG("%s: key-code: %d, key-state: %d", __FUNCTION__,
                                                    key_code, key_state);
    if (btif_rc_cb.rc_features & BTA_AV_FEAT_RCTG)
    {
        bt_status_t tran_status = get_transaction(&p_transaction);
        if (BT_STATUS_SUCCESS == tran_status && NULL != p_transaction)
        {
            BTA_AvRemoteCmd(btif_rc_cb.rc_handle, p_transaction->lbl,
                (tBTA_AV_RC)key_code, (tBTA_AV_STATE)key_state);
            status =  BT_STATUS_SUCCESS;
            BTIF_TRACE_DEBUG("%s: succesfully sent passthrough command to BTA", __FUNCTION__);

#if defined(MTK_LINUX) && defined(MTK_COMMON) && (MTK_COMMON == TRUE)
            start_control_command_timer (0, p_transaction);
#endif
        }
        else
        {
            status =  BT_STATUS_FAIL;
            BTIF_TRACE_DEBUG("%s: error in fetching transaction", __FUNCTION__);
        }
    }
    else
    {
        status =  BT_STATUS_FAIL;
        BTIF_TRACE_DEBUG("%s: feature not supported", __FUNCTION__);
    }
#else
    BTIF_TRACE_DEBUG("%s: feature not enabled", __FUNCTION__);
#endif
    return status;
}

static const btrc_interface_t bt_rc_interface = {
    sizeof(bt_rc_interface),
    init,
    get_play_status_rsp,
#if defined(MTK_AVRCP_TG_15_BROWSE)&&(MTK_AVRCP_TG_15_BROWSE == TRUE)
    list_player_app_attr_rsp,
    list_player_app_value_rsp,
    get_player_app_value_rsp,
    get_player_app_attr_text_rsp,
    get_player_app_value_text_rsp,
    get_element_attr_rsp,
    set_player_app_value_rsp,
#else
    NULL, /* list_player_app_attr_rsp */
    NULL, /* list_player_app_value_rsp */
    NULL, /* get_player_app_value_rsp */
    NULL, /* get_player_app_attr_text_rsp */
    NULL, /* get_player_app_value_text_rsp */
    get_element_attr_rsp,
    NULL, /* set_player_app_value_rsp */
#endif
    register_notification_rsp,
    set_volume,
#if defined(MTK_AVRCP_TG_15_BROWSE)&&(MTK_AVRCP_TG_15_BROWSE == TRUE)
    set_addressed_player_rsp,
    set_browsed_player_rsp,
    get_folder_items_list_rsp,
    change_path_rsp,
    get_item_attr_rsp,
    play_item_rsp,
    get_total_num_of_items_rsp,
    search_rsp,
    add_to_now_playing_rsp,
#endif
    cleanup,
};

static const btrc_ctrl_interface_t bt_rc_ctrl_interface = {
    sizeof(bt_rc_ctrl_interface),
    init_ctrl,
    send_passthrough_cmd,
    send_groupnavigation_cmd,
    change_player_app_setting,
#if defined(MTK_AVRCP_TG_15_BROWSE)&&(MTK_AVRCP_TG_15_BROWSE == TRUE)
    play_item_cmd,
    get_playback_state_cmd,
    get_now_playing_list_cmd,
    get_folder_list_cmd,
    get_player_list_cmd,
    change_folder_path_cmd,
    set_browsed_player_cmd,
    set_addressed_player_cmd,
#endif
    set_volume_rsp,
    volume_change_notification_rsp,
    cleanup_ctrl,
};

/*******************************************************************************
**
** Function         btif_rc_get_interface
**
** Description      Get the AVRCP Target callback interface
**
** Returns          btav_interface_t
**
*******************************************************************************/
const btrc_interface_t *btif_rc_get_interface(void)
{
    BTIF_TRACE_EVENT("%s", __FUNCTION__);
    return &bt_rc_interface;
}

/*******************************************************************************
**
** Function         btif_rc_ctrl_get_interface
**
** Description      Get the AVRCP Controller callback interface
**
** Returns          btav_interface_t
**
*******************************************************************************/
const btrc_ctrl_interface_t *btif_rc_ctrl_get_interface(void)
{
    BTIF_TRACE_EVENT("%s", __FUNCTION__);
    return &bt_rc_ctrl_interface;
}

/*******************************************************************************
**      Function         initialize_transaction
**
**      Description    Initializes fields of the transaction structure
**
**      Returns          void
*******************************************************************************/
static void initialize_transaction(int lbl)
{
    pthread_mutex_lock(&device.lbllock);
    if (lbl < MAX_TRANSACTIONS_PER_SESSION) {
        if (alarm_is_scheduled(device.transaction[lbl].txn_timer)) {
            clear_cmd_timeout(lbl);
        }
        device.transaction[lbl].lbl = lbl;
        device.transaction[lbl].in_use=FALSE;
        device.transaction[lbl].handle=0;
    }
    pthread_mutex_unlock(&device.lbllock);
}

/*******************************************************************************
**      Function         lbl_init
**
**      Description    Initializes label structures and mutexes.
**
**      Returns         void
*******************************************************************************/
void lbl_init()
{
    memset(&device,0,sizeof(rc_device_t));
    pthread_mutexattr_t attr;
    pthread_mutexattr_init(&attr);
    pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
    pthread_mutex_init(&(device.lbllock), &attr);
    pthread_mutexattr_destroy(&attr);
    init_all_transactions();
}

/*******************************************************************************
**
** Function         init_all_transactions
**
** Description    Initializes all transactions
**
** Returns          void
*******************************************************************************/
void init_all_transactions()
{
    UINT8 txn_indx=0;
    for(txn_indx=0; txn_indx < MAX_TRANSACTIONS_PER_SESSION; txn_indx++)
    {
        initialize_transaction(txn_indx);
    }
}

/*******************************************************************************
**
** Function         get_transaction_by_lbl
**
** Description    Will return a transaction based on the label. If not inuse
**                     will return an error.
**
** Returns          bt_status_t
*******************************************************************************/
rc_transaction_t *get_transaction_by_lbl(UINT8 lbl)
{
    rc_transaction_t *transaction = NULL;
    pthread_mutex_lock(&device.lbllock);

    /* Determine if this is a valid label */
    if (lbl < MAX_TRANSACTIONS_PER_SESSION)
    {
        if (FALSE==device.transaction[lbl].in_use)
        {
            transaction = NULL;
        }
        else
        {
            transaction = &(device.transaction[lbl]);
            BTIF_TRACE_DEBUG("%s: Got transaction.label: %d",__FUNCTION__,lbl);
        }
    }

    pthread_mutex_unlock(&device.lbllock);
    return transaction;
}

/*******************************************************************************
**
** Function         get_transaction
**
** Description    Obtains the transaction details.
**
** Returns          bt_status_t
*******************************************************************************/

bt_status_t  get_transaction(rc_transaction_t **ptransaction)
{
    bt_status_t result = BT_STATUS_NOMEM;
    UINT8 i=0;
    pthread_mutex_lock(&device.lbllock);

    // Check for unused transactions
    for (i=0; i<MAX_TRANSACTIONS_PER_SESSION; i++)
    {
        if (FALSE==device.transaction[i].in_use)
        {
            BTIF_TRACE_DEBUG("%s:Got transaction.label: %d",__FUNCTION__,device.transaction[i].lbl);
            device.transaction[i].in_use = TRUE;
            *ptransaction = &(device.transaction[i]);
            result = BT_STATUS_SUCCESS;
            break;
        }
    }

    pthread_mutex_unlock(&device.lbllock);
    return result;
}

/*******************************************************************************
**
** Function         release_transaction
**
** Description    Will release a transaction for reuse
**
** Returns          bt_status_t
*******************************************************************************/
void release_transaction(UINT8 lbl)
{
    rc_transaction_t *transaction = get_transaction_by_lbl(lbl);

    /* If the transaction is in use... */
    if (transaction != NULL)
    {
        BTIF_TRACE_DEBUG("%s: lbl: %d", __FUNCTION__, lbl);
        initialize_transaction(lbl);
    }
}

/*******************************************************************************
**
** Function         lbl_destroy
**
** Description    Cleanup of the mutex
**
** Returns          void
*******************************************************************************/
void lbl_destroy()
{
    pthread_mutex_destroy(&(device.lbllock));
}

/*******************************************************************************
**      Function       sleep_ms
**
**      Description    Sleep the calling thread unconditionally for
**                     |timeout_ms| milliseconds.
**
**      Returns        void
*******************************************************************************/
static void sleep_ms(period_ms_t timeout_ms) {
    struct timespec delay;
    delay.tv_sec = timeout_ms / 1000;
    delay.tv_nsec = 1000 * 1000 * (timeout_ms % 1000);

    OSI_NO_INTR(nanosleep(&delay, &delay));
}

static bool absolute_volume_disabled() {
    char volume_disabled[PROPERTY_VALUE_MAX] = {0};
    osi_property_get("persist.bluetooth.disableabsvol", volume_disabled, "false");
    if (strncmp(volume_disabled, "true", 4) == 0) {
        BTIF_TRACE_WARNING("%s: Absolute volume disabled by property", __func__);
        return true;
    }
    return false;
}

static char const* key_id_to_str(uint16_t id) {
    for (int i = 0; key_map[i].name != NULL; i++) {
        if (id == key_map[i].mapped_id)
            return key_map[i].name;
    }
    return "UNKNOWN KEY";
}

#if defined(MTK_STACK_CONFIG_BL) && (MTK_STACK_CONFIG_BL == TRUE)
/*******************************************************************************
**
** Function         btif_avrcp_key_state_reset
**
** Description      key value reset if active call
**
** Returns          void
**
*******************************************************************************/
void btif_avrcp_key_state_reset() {
    LOG_INFO(LOG_TAG,"%s key value reset if active call", __FUNCTION__);
    prestatus.key = 0;
    prestatus.pressed = -1;
}
#endif

#if defined(MTK_LINUX_AVRCP_PLUS) && (MTK_LINUX_AVRCP_PLUS == TRUE)

static bt_status_t init_ext( btrc_ctrl_ext_callbacks_t* ext_callbacks )
{
    BTIF_TRACE_EVENT("## %s ##", __FUNCTION__);
    bt_status_t result = BT_STATUS_SUCCESS;

    if (bt_rc_ctrl_ext_callbacks)
        return BT_STATUS_DONE;

    bt_rc_ctrl_ext_callbacks = ext_callbacks;

    return result;
}

static bt_status_t init_tg_ext( btrc_ext_callbacks_t* ext_callbacks )
{
    BTIF_TRACE_EVENT("## %s ##", __FUNCTION__);
    bt_status_t result = BT_STATUS_SUCCESS;

    if (bt_rc_ext_callbacks)
        return BT_STATUS_DONE;

    bt_rc_ext_callbacks = ext_callbacks;

    return result;
}


static void btif_send_get_playstatus_cmd_ex(void)
{
    BTIF_TRACE_EVENT("## %s ##", __FUNCTION__);

    get_play_status_cmd();
}

static const btrc_ctrl_ex_interface_t btrcExInterface = {
    sizeof(btrcExInterface),
    init_ext,
    btif_send_get_playstatus_cmd_ex,
};

/* extend AVRCP TG interface */
static const btrc_ex_interface_t btrc_ex_interface = {
    sizeof(btrc_ex_interface),
    init_tg_ext,
    send_passthrough_cmd,
};

/*******************************************************************************
**
** Function         btif_rc_ex_get_interface
**
** Description      Get the AVRCP (Ctrl) expanded callback interface
**
** Returns          btrc_ctrl_ex_interface_t
**
*******************************************************************************/
const btrc_ctrl_ex_interface_t *btif_rc_ex_get_interface(void)
{
    BTIF_TRACE_EVENT("%s", __FUNCTION__);
    return &btrcExInterface;
}

#endif //(end -- #if defined(MTK_LINUX_AVRCP_PLUS))

#if defined(MTK_AVRCP_TG_15_BROWSE)&&(MTK_AVRCP_TG_15_BROWSE == TRUE)
void fill_avrc_attr_entry(tAVRC_ATTR_ENTRY* attr_vals, int num_attrs,
                          btrc_element_attr_val_t* p_attrs) {
  for (int attr_cnt = 0; attr_cnt < num_attrs; attr_cnt++) {
    attr_vals[attr_cnt].attr_id = p_attrs[attr_cnt].attr_id;
    attr_vals[attr_cnt].name.charset_id = AVRC_CHARSET_ID_UTF8;
    attr_vals[attr_cnt].name.str_len =
        (uint16_t)strlen((char*)p_attrs[attr_cnt].text);
    attr_vals[attr_cnt].name.p_str = p_attrs[attr_cnt].text;
    BTIF_TRACE_DEBUG(
        "%s: attr_id: 0x%x, charset_id: 0x%x, str_len: %d, str: %s", __func__,
        (unsigned int)attr_vals[attr_cnt].attr_id,
        attr_vals[attr_cnt].name.charset_id, attr_vals[attr_cnt].name.str_len,
        attr_vals[attr_cnt].name.p_str);
  }
}

bt_status_t build_and_send_vendor_cmd(tAVRC_COMMAND* avrc_cmd,
                                      tBTA_AV_CODE cmd_code) {
  rc_transaction_t* p_transaction = NULL;
  bt_status_t tran_status = get_transaction(&p_transaction);
  if (BT_STATUS_SUCCESS != tran_status) return BT_STATUS_FAIL;

  BT_HDR* p_msg = NULL;
  tAVRC_STS status = AVRC_BldCommand(avrc_cmd, &p_msg);
  if (status == AVRC_STS_NO_ERROR && p_msg != NULL) {
    uint8_t* data_start = (uint8_t*)(p_msg + 1) + p_msg->offset;
    BTIF_TRACE_DEBUG("%s: %s msgreq being sent out with label: %d", __func__,
                     dump_rc_pdu(avrc_cmd->pdu), p_transaction->lbl);
    BTA_AvVendorCmd(btif_rc_cb.rc_handle, p_transaction->lbl, cmd_code, data_start,
                    p_msg->len);
    status = BT_STATUS_SUCCESS;
    if (cmd_code == AVRC_CMD_STATUS) {
      start_status_command_timer(avrc_cmd->pdu, p_transaction);
    } else if (cmd_code == AVRC_CMD_CTRL) {
      start_control_command_timer(avrc_cmd->pdu, p_transaction);
    }
  } else {
    BTIF_TRACE_ERROR("%s: failed to build command. status: 0x%02x", __func__,
                     status);
  }
  osi_free(p_msg);
  return (bt_status_t)status;
}

/***************************************************************************
 * Function:  fill_attribute_id_array
 *
 * - Argument:
 *     cmd_attribute_number         input attribute number from AVRCP command
 *     cmd_attribute_id_array       input attribute list from AVRCP command
 *     out_array_size               allocated size of out attribute id array
 *     out_attribute_id_array       output attribute list resolved here
 *
 * - Description:
 *     Resolve attribute id array as defined by the AVRCP specification.
 *
 * - Returns:
 *     The number of attributes filled in
 *
 ***************************************************************************/
static uint8_t fill_attribute_id_array(
    uint8_t cmd_attribute_number, btrc_media_attr_t* cmd_attribute_id_array,
    size_t out_array_size, btrc_media_attr_t* out_attribute_id_array) {
  /* Reset attribute array */
  memset(out_attribute_id_array, 0, out_array_size);
  /* Default case for cmd_attribute_number == 0xFF, No attribute */
  uint8_t out_attribute_number = 0;
  if (cmd_attribute_number == 0) {
    /* All attributes */
    out_attribute_number = out_array_size < AVRC_MAX_NUM_MEDIA_ATTR_ID
                               ? out_array_size
                               : AVRC_MAX_NUM_MEDIA_ATTR_ID;
    for (int i = 0; i < out_attribute_number; i++) {
      out_attribute_id_array[i] = (btrc_media_attr_t)(i + 1);
    }
  } else if (cmd_attribute_number != 0xFF) {
    /* Attribute List */
    out_attribute_number = 0;
    int filled_id_count = 0;
    for (int i = 0; (i < cmd_attribute_number) &&
                    (out_attribute_number < out_array_size) &&
                    (out_attribute_number < AVRC_MAX_NUM_MEDIA_ATTR_ID);
         i++) {
      /* Fill only valid entries */
      if (AVRC_IS_VALID_MEDIA_ATTRIBUTE(cmd_attribute_id_array[i])) {
        /* Skip the duplicate entries */
        for (filled_id_count = 0; filled_id_count < out_attribute_number;
             filled_id_count++) {
          if (out_attribute_id_array[filled_id_count] ==
              cmd_attribute_id_array[i])
            break;
        }
        /* New ID */
        if (filled_id_count == out_attribute_number) {
          out_attribute_id_array[out_attribute_number] =
              (btrc_media_attr_t)cmd_attribute_id_array[i];
          out_attribute_number++;
        }
      }
    }
  }
  return out_attribute_number;
}

/***************************************************************************
 *  Function         get_rsp_type_code
 *
 *  - Argument:   status
 *  - Description: Returns response type codes for particular command code and
 *                 status.
 *
 ***************************************************************************/
static tBTA_AV_CODE get_rsp_type_code(tAVRC_STS status, tBTA_AV_CODE code) {
  if (status != AVRC_STS_NO_ERROR) {
    return AVRC_RSP_REJ;
  }

  if (code < AVRC_RSP_NOT_IMPL) {
    if (code == AVRC_CMD_NOTIF) return AVRC_RSP_INTERIM;

    if (code == AVRC_CMD_STATUS) return AVRC_RSP_IMPL_STBL;

    return AVRC_RSP_ACCEPT;
  }

  return code;
}

/**************************************************************************
**
** Function         list_player_app_attr_rsp
**
** Description      ListPlayerApplicationSettingAttributes (PDU ID: 0x11)
**                  This method is callled in response to PDU 0x11
**
** Returns          bt_status_t
**
****************************************************************************/
static bt_status_t  list_player_app_attr_rsp( int num_attr, btrc_player_attr_t *p_attrs)
{
    tAVRC_RESPONSE avrc_rsp;
    int i;
    CHECK_RC_CONNECTED

    BTIF_TRACE_DEBUG("-%s on ", __FUNCTION__);

    memset(&(avrc_rsp.list_app_attr), 0, sizeof(tAVRC_LIST_APP_ATTR_RSP));
    if (num_attr == 0)
    {
        avrc_rsp.list_app_attr.status = AVRC_STS_BAD_PARAM;
    }
    else
    {
        avrc_rsp.list_app_attr.num_attr = num_attr;
        for (i = 0 ; i < num_attr ; ++i)
        {
            avrc_rsp.list_app_attr.attrs[i] = p_attrs[i];
        }
        avrc_rsp.list_app_attr.status = AVRC_STS_NO_ERROR;
    }
    avrc_rsp.list_app_attr.pdu  = AVRC_PDU_LIST_PLAYER_APP_ATTR ;
    avrc_rsp.list_app_attr.opcode = opcode_from_pdu(AVRC_PDU_LIST_PLAYER_APP_ATTR);
    /* Send the response */
    SEND_METAMSG_RSP(IDX_LIST_APP_ATTR_RSP, &avrc_rsp);
    return BT_STATUS_SUCCESS;
}

/**********************************************************************
**
** Function list_player_app_value_rsp
**
** Description      ListPlayerApplicationSettingValues (PDU ID: 0x12)
                    This method is called in response to PDU 0x12
************************************************************************/
static bt_status_t  list_player_app_value_rsp( int num_val, uint8_t *value)
{
    tAVRC_RESPONSE avrc_rsp;
    int i;
    CHECK_RC_CONNECTED

    BTIF_TRACE_DEBUG("-%s on ", __FUNCTION__);

    memset(&(avrc_rsp.list_app_values), 0, sizeof(tAVRC_LIST_APP_VALUES_RSP));
    if ((num_val == 0) || (num_val > AVRC_MAX_APP_ATTR_SIZE))
    {
        avrc_rsp.list_app_values.status = AVRC_STS_BAD_PARAM;
    }
    else
    {
        avrc_rsp.list_app_values.num_val = num_val;
        for (i = 0; i < num_val; ++i)
        {
            avrc_rsp.list_app_values.vals[i] = value[i];
        }
        avrc_rsp.list_app_values.status = AVRC_STS_NO_ERROR;
    }
    avrc_rsp.list_app_values.pdu   = AVRC_PDU_LIST_PLAYER_APP_VALUES;
    avrc_rsp.list_app_attr.opcode  = opcode_from_pdu(AVRC_PDU_LIST_PLAYER_APP_VALUES);
    /* Send the response */
    SEND_METAMSG_RSP(IDX_LIST_APP_VALUE_RSP, &avrc_rsp);
    return BT_STATUS_SUCCESS;
}


/**********************************************************************
**
** Function  get_player_app_value_rsp
**
** Description  This methos is called in response to PDU ID 0x13
**
***********************************************************************/
static bt_status_t get_player_app_value_rsp(btrc_player_settings_t *p_vals)
{
    tAVRC_RESPONSE avrc_rsp;
    int i;
    tAVRC_APP_SETTING app_sett[AVRC_MAX_APP_ATTR_SIZE];
    CHECK_RC_CONNECTED

    BTIF_TRACE_DEBUG("-%s on ", __FUNCTION__);
    memset(&(avrc_rsp.get_cur_app_val) ,0 , sizeof(tAVRC_GET_CUR_APP_VALUE_RSP));
    avrc_rsp.get_cur_app_val.p_vals   = app_sett ;
    //Check for Error Condition
    if ((p_vals == NULL) || (p_vals->num_attr== 0) || (p_vals->num_attr > AVRC_MAX_APP_ATTR_SIZE))
    {
        avrc_rsp.get_cur_app_val.status = AVRC_STS_BAD_PARAM;
    }
    else if (p_vals->num_attr <= BTRC_MAX_APP_SETTINGS)
    {
        memset(app_sett, 0, sizeof(tAVRC_APP_SETTING)*p_vals->num_attr );
        //update num_val
        avrc_rsp.get_cur_app_val.num_val  = p_vals->num_attr ;
        avrc_rsp.get_cur_app_val.p_vals   = app_sett ;
        for (i = 0; i < p_vals->num_attr; ++i)
        {
            app_sett[i].attr_id  = p_vals->attr_ids[i] ;
            app_sett[i].attr_val = p_vals->attr_values[i];
            BTIF_TRACE_DEBUG("%s attr_id:0x%x, charset_id:0x%x, num_element:%d",
                           __FUNCTION__, (unsigned int)app_sett[i].attr_id,
                              app_sett[i].attr_val ,p_vals->num_attr );
        }
        //Update PDU , status aind
        avrc_rsp.get_cur_app_val.status = AVRC_STS_NO_ERROR;
    }
    avrc_rsp.get_cur_app_val.pdu = AVRC_PDU_GET_CUR_PLAYER_APP_VALUE;
    avrc_rsp.get_cur_app_val.opcode = opcode_from_pdu(AVRC_PDU_GET_CUR_PLAYER_APP_VALUE);
    SEND_METAMSG_RSP(IDX_GET_CURR_APP_VAL_RSP, &avrc_rsp);
    return BT_STATUS_SUCCESS;
}

/********************************************************************
**
** Function	        get_player_app_attr_text_rsp
**
** Description   This method is called in response to get player
**                         applicaton attribute text response
**
**
*******************************************************************/
static bt_status_t get_player_app_attr_text_rsp(int num_attr, btrc_player_setting_text_t *p_attrs)
{
    tAVRC_RESPONSE avrc_rsp;
    tAVRC_APP_SETTING_TEXT attr_txt[AVRC_MAX_APP_ATTR_SIZE];
    int i;
    CHECK_RC_CONNECTED

    BTIF_TRACE_DEBUG("-%s ", __FUNCTION__);

    if (num_attr == 0)
    {
        avrc_rsp.get_app_attr_txt.status = AVRC_STS_BAD_PARAM;
    }
    else
    {
        for (i =0; i< num_attr; ++i)
        {
            attr_txt[i].charset_id = AVRC_CHARSET_ID_UTF8;
            attr_txt[i].attr_id = p_attrs[i].id ;
            attr_txt[i].str_len = (UINT8)strnlen((char *)p_attrs[i].text, BTRC_MAX_ATTR_STR_LEN);
            attr_txt[i].p_str = p_attrs[i].text ;
            BTIF_TRACE_DEBUG("%s attr_id:0x%x, charset_id:0x%x, str_len:%d, str:%s",
                    __FUNCTION__, (unsigned int)attr_txt[i].attr_id,
                    attr_txt[i].charset_id , attr_txt[i].str_len, attr_txt[i].p_str);
        }
        avrc_rsp.get_app_attr_txt.status = AVRC_STS_NO_ERROR;
    }
    avrc_rsp.get_app_attr_txt.p_attrs = attr_txt ;
    avrc_rsp.get_app_attr_txt.num_attr = (UINT8)num_attr;
    avrc_rsp.get_app_attr_txt.pdu = AVRC_PDU_GET_PLAYER_APP_ATTR_TEXT;
    avrc_rsp.get_app_attr_txt.opcode = opcode_from_pdu(AVRC_PDU_GET_PLAYER_APP_ATTR_TEXT);
    /* Send the response */
    SEND_METAMSG_RSP(IDX_GET_APP_ATTR_TXT_RSP, &avrc_rsp);
    return BT_STATUS_SUCCESS;
}

/********************************************************************
**
** Function	        get_player_app_value_text_rsp
**
** Description   This method is called in response to Player application
**                         value text
**
** Return            bt_status_t
**
*******************************************************************/
static bt_status_t get_player_app_value_text_rsp(int num_attr, btrc_player_setting_text_t *p_attrs)
{
    tAVRC_RESPONSE avrc_rsp;
    tAVRC_APP_SETTING_TEXT attr_txt[AVRC_MAX_APP_ATTR_SIZE];
    int i;
    CHECK_RC_CONNECTED
    BTIF_TRACE_DEBUG("- %s ", __FUNCTION__);

    if (num_attr == 0)
    {
        avrc_rsp.get_app_val_txt.status = AVRC_STS_BAD_PARAM;
    }
    else
    {
        for (i =0; i< num_attr; ++i)
        {
            attr_txt[i].charset_id = AVRC_CHARSET_ID_UTF8;
            attr_txt[i].attr_id  = p_attrs[i].id ;
            attr_txt[i].str_len  = (UINT8)strnlen((char *)p_attrs[i].text ,BTRC_MAX_ATTR_STR_LEN );
            attr_txt[i].p_str = p_attrs[i].text ;
            BTIF_TRACE_DEBUG("%s attr_id:0x%x, charset_id:0x%x, str_len:%d, str:%s",
                    __FUNCTION__, (unsigned int)attr_txt[i].attr_id,
                    attr_txt[i].charset_id , attr_txt[i].str_len,attr_txt[i].p_str);
        }
        avrc_rsp.get_app_val_txt.status = AVRC_STS_NO_ERROR;
    }
    avrc_rsp.get_app_val_txt.p_attrs = attr_txt;
    avrc_rsp.get_app_val_txt.num_attr = (UINT8)num_attr;
    avrc_rsp.get_app_val_txt.pdu = AVRC_PDU_GET_PLAYER_APP_VALUE_TEXT;
    avrc_rsp.get_app_val_txt.opcode = opcode_from_pdu(AVRC_PDU_GET_PLAYER_APP_VALUE_TEXT);
    /* Send the response */
    SEND_METAMSG_RSP(IDX_GET_APP_VAL_TXT_RSP, &avrc_rsp);
    return BT_STATUS_SUCCESS;
}

/********************************************************************
**
** Function        set_player_app_value_rsp
**
** Description  This method is called in response to
**                       application value
**
** Return           bt_staus_t
**
*******************************************************************/
static bt_status_t set_player_app_value_rsp (btrc_status_t rsp_status)
{
    tAVRC_RESPONSE avrc_rsp;

    BTIF_TRACE_DEBUG("-%s", __FUNCTION__);

    CHECK_RC_CONNECTED
    avrc_rsp.set_app_val.opcode = opcode_from_pdu(AVRC_PDU_SET_PLAYER_APP_VALUE);
    avrc_rsp.set_app_val.pdu =  AVRC_PDU_SET_PLAYER_APP_VALUE ;
    avrc_rsp.set_app_val.status =  rsp_status ;
    SEND_METAMSG_RSP(IDX_SET_APP_VAL_RSP, &avrc_rsp);
    return BT_STATUS_SUCCESS;
}
#endif //(end -- #if defined(MTK_AVRCP_TG_15_BROWSE))
/*******************************************************************************
**
** Function         btrc_ex_interface_t
**
** Description      Get the AVRCP (TG) expanded callback interface
**
** Returns          btrc_ex_interface_t
**
*******************************************************************************/
const btrc_ex_interface_t *btif_rc_tg_ex_get_interface(void)
{
    BTIF_TRACE_EVENT("%s", __FUNCTION__);
    return &btrc_ex_interface;
}

#endif
