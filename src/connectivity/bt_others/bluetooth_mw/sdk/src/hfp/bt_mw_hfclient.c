/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 * the prior written permission of MediaTek inc. and/or its licensors, any
 * reproduction, modification, use or disclosure of MediaTek Software, and
 * information contained herein, in whole or in part, shall be strictly
 * prohibited.
 *
 * MediaTek Inc. (C) 2016-2017. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 * ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 * NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
 * RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 * INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
 * TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 * RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 * OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
 * SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
 * RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
 * ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
 * RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
 * MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 * CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek
 * Software") have been modified by MediaTek Inc. All revisions are subject to
 * any receiver's applicable license agreements with MediaTek Inc.
 */

 
#include <stdio.h>
#include <sys/prctl.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include "bt_mw_hfclient.h"
#include "bt_mw_common.h"
#include "linuxbt_hfclient_if.h"
#include "u_bt_mw_hfclient.h"
#include "bt_mw_message_queue.h"
#if defined(MTK_LINUX_HFP_PHONEBOOK) && (MTK_LINUX_HFP_PHONEBOOK == TRUE)
#include <pthread.h>
#include "osi/include/allocator.h"
#include "osi/include/hash_map.h"
#endif


#if defined(MTK_LINUX_HFP_PHONEBOOK) && (MTK_LINUX_HFP_PHONEBOOK == TRUE)
#define BT_HFCLIENT_ENTRY_MSG_TYPE_CTRL    0x1
#define BT_HFCLIENT_READ_PB_ENTRIES_NUM    10  /*AT+CPBR=<min>,<max>,max number between min and max*/
static const size_t phonebook_entry_buckets = 100;

/* AT+CSCS="##": TE Charset Values*/
#define BTA_HF_CLIENT_CHARSET_GSM  "GSM"
#define BTA_HF_CLIENT_CHARSET_HEX  "HEX"
#define BTA_HF_CLIENT_CHARSET_IRA  "IRA"
#define BTA_HF_CLIENT_CHARSET_PCCP "PCCPxxx"
#define BTA_HF_CLIENT_CHARSET_PCDN "PCDN"
#define BTA_HF_CLIENT_CHARSET_USC2 "USC2"
#define BTA_HF_CLIENT_CHARSET_UTF8 "UTF-8"
#define BTA_HF_CLIENT_CHARSET_8859N  "8859-n"
#define BTA_HF_CLIENT_CHARSET_8859C  "8859-C"
#define BTA_HF_CLIENT_CHARSET_8859A  "8859-A"
#define BTA_HF_CLIENT_CHARSET_8859G  "8859-G"
#define BTA_HF_CLIENT_CHARSET_8859H  "8859-H"


typedef enum
{
    BT_HFCLIENT_EVENT_NONE,
    BT_HFCLIENT_SELECT_PB_STORAGE,  /*AT+CPBS=?:        List supported memory storages of AG*/
    BT_HFCLIENT_SET_PB_STORAGE,     /*AT+CPBS="##":     Set phonebook memory storage for use*/
    BT_HFCLIENT_SET_CHARSET,        /*AT+CSCS="##":     Inform TA which charset is used by TE */
    BT_HFCLIENT_READ_PB_ENTRY,      /*AT+CPBR=1,max:    Return phonebook entries List in range(1,max)*/
    BT_HFCLIENT_READ_ALL_PB_ENTRY,  /*Callback all phonebook entries stored in HFP MW*/
    BT_HFCLIENT_EVENT_EXIT,         /*exit thread pb_entry_thd*/
}BT_HFCLIENT_ENTRY_EVENT_T;


/* AT+CPBS="##": Phonebook Storage Memory Values*/
typedef enum
{
    BT_HFCLIENT_PB_STORAGE_DC = 0,
    BT_HFCLIENT_PB_STORAGE_EN,
    BT_HFCLIENT_PB_STORAGE_FD,
    BT_HFCLIENT_PB_STORAGE_LD,
    BT_HFCLIENT_PB_STORAGE_MC,
    BT_HFCLIENT_PB_STORAGE_ME,
    BT_HFCLIENT_PB_STORAGE_MT,
    BT_HFCLIENT_PB_STORAGE_ON,
    BT_HFCLIENT_PB_STORAGE_RC,
    BT_HFCLIENT_PB_STORAGE_SM,
    BT_HFCLIENT_PB_STORAGE_TA,
    BT_HFCLIENT_PB_STORAGE_AP,
    BT_HFCLIENT_PB_STORAGE_MAX,
}BT_HFCLIENT_PB_STORAGE_TYPE_T;

typedef struct
{
    long int                    msg_type;
    BT_HFCLIENT_ENTRY_EVENT_T   msg_id;
    void*                       param1;
    void*                       param2;
}BT_HFCLIENT_ENTRY_MSG_T;


typedef struct
{
    UINT16                        entry_num;        /*phonebook entry number supported by the current storage*/
    UINT16                        read_idx;         /*phonebook entry max index, by AT+CPBR=<min>,<max>*/
    bool                          is_exist;         /*The current storage is supported by AG or not*/
    bool                          is_read_done;     /*All phonebook entries have beed parsed of current storage or not*/
}BT_HFCLIENT_PB_STORAGE_T;


typedef struct
{
    hash_map_t *                   list;            /*Hash map to store phonebook entries in HFP MW*/
    BT_HFCLIENT_PB_STORAGE_TYPE_T  cur_storage;     /*Storage type being used by AT+CPBR=?,AT+CPBR=<min>,<max>*/
    BT_HFCLIENT_PB_STORAGE_T       storage[BT_HFCLIENT_PB_STORAGE_MAX];  /*storage memory arrays*/
    bool                           all_read_done;         /*Phonebook entries of all memory storages parsed or not*/
}BT_HFCLIENT_PB_ENTRY_LIST_T;

typedef struct
{
    pthread_t       thd_id;
    bool            is_exit;                        /*Thread exists or not,TRUE: inactive,FALSE:active*/
}BT_HFCLIENT_PB_ENTRY_THD_T;

typedef struct
{
    int                              msg_q_id;
    BT_HFCLIENT_PB_ENTRY_LIST_T      entry_list;
    BT_HFCLIENT_PB_ENTRY_THD_T       entry_thd;
}BT_HFCLIENT_PHONEBOOK_T;
#endif

static BT_HFCLIENT_EVENT_HANDLE_CB g_bt_mw_hfclient_event_handle_cb = NULL;
static INT32 bluetooth_hfclient_init(VOID);
static INT32 bluetooth_hfclient_deinit(VOID);
static VOID bluetooth_hfclient_msg_handle(tBTMW_MSG *p_msg);
static VOID bluetooth_hfclient_notify_handle(tBTMW_MSG *p_msg);
static VOID bluetooth_hfclient_connection_state_cb(BT_HFCLIENT_CONNECTION_CB_DATA_T *connect_cb);
static VOID bluetooth_hfclient_audio_connection_state_cb(BT_HFCLIENT_AUDIO_CONNECTION_CB_DATA_T *audio_connect_cb);
static VOID bluetooth_hfclient_vr_cmd_cb(BT_HFCLIENT_BVRA_CB_DATA_T *bvra_cb);
static VOID bluetooth_hfclient_indication_service_cb(BT_HFCLIENT_IND_SERVICE_CB_DATA_T *service_cb);
static VOID bluetooth_hfclient_indication_roaming_cb(BT_HFCLIENT_IND_ROAM_CB_DATA_T *roam_cb);
static VOID bluetooth_hfclient_indication_signal_cb(BT_HFCLIENT_IND_SIGNAL_CB_DATA_T *signal_cb);
static VOID bluetooth_hfclient_indication_battery_cb(BT_HFCLIENT_IND_BATTCH_CB_DATA_T *battery_cb);
static VOID bluetooth_hfclient_current_operator_cb(BT_HFCLIENT_COPS_CB_DATA_T *cops_cb);
static VOID bluetooth_hfclient_call_cb(BT_HFCLIENT_IND_CALL_CB_DATA_T  *call_cb);
static VOID bluetooth_hfclient_callsetup_cb(BT_HFCLIENT_IND_CALLSETUP_CB_DATA_T *callsetup_cb);
static VOID bluetooth_hfclient_callheld_cb(BT_HFCLIENT_IND_CALLHELD_CB_DATA_T *callheld_cb);
static VOID bluetooth_hfclient_clip_cb(BT_HFCLIENT_CLIP_CB_DATA_T *clip_cb);
static VOID bluetooth_hfclient_call_waiting_cb(BT_HFCLIENT_CCWA_CB_DATA_T *ccwa_cb);
static VOID bluetooth_hfclient_current_calls_cb(BT_HFCLIENT_CLCC_CB_DATA_T *clcc_cb);
static VOID bluetooth_hfclient_volume_change_cb(BT_HFCLIENT_VGM_VGS_CB_DATA_T *volume_cb);
static VOID bluetooth_hfclient_cmd_complete_cb(BT_HFCLIENT_CMD_COMPLETE_CB_DATA_T *cmd_complete_cb);
static VOID bluetooth_hfclient_subscriber_info_cb(BT_HFCLIENT_CNUM_CB_DATA_T *cnum_cb);
static VOID bluetooth_hfclient_in_band_ring_tone_cb(BT_HFCLIENT_BSIR_CB_DATA_T *bsir_cb);
static VOID bluetooth_hfclient_last_voice_tag_number_cb(BT_HFCLIENT_BINP_CB_DATA_T *binp_cb);
static VOID bluetooth_hfclient_ring_indication_cb(VOID);
static VOID bluetooth_hfclient_ability_state_cb(BT_HFCLIENT_ABILITY_CB_DATA_T *ability_cb);

#if defined(MTK_LINUX_HFP_PHONEBOOK) && (MTK_LINUX_HFP_PHONEBOOK == TRUE)
/*Initialization here to cover case:Receive HFP disconnection event at first*/
static BT_HFCLIENT_PHONEBOOK_T pb_list = {
   .entry_thd.is_exit = true,
   .entry_list.list = NULL,
   };

static INT32 bluetooth_hfclient_select_pb_storage();
static INT32 bluetooth_hfclient_set_pb_storage(BT_HFCLIENT_PB_STORAGE_TYPE_T storage_idx);
static INT32 bluetooth_hfclient_set_charset(const char* charset);
static INT32 bluetooth_hfclient_test_pb_entry(VOID);
static INT32 bluetooth_hfclient_read_pb_entry(UINT16 idx_min, UINT16 idx_max);
static INT32 bluetooth_hfclient_msg_queue_init();
static INT32 bluetooth_hfclient_msg_queue_uninit();
static INT32 bluetooth_hfclient_send_msg(BT_HFCLIENT_ENTRY_MSG_T *p_send_msg);
static INT32 bluetooth_hfclient_receive_msg(BT_HFCLIENT_ENTRY_MSG_T *p_receive_msg);

static INT32 bluetooth_hfclient_cpbs_cb(BT_HFCLIENT_CPBS_CB_DATA_T *cpbs_cb);
static INT32 bluetooth_hfclient_cpbr_count_cb(BT_HFCLIENT_CPBR_COUNT_CB_DATA_T *cpbr_count_cb);
static VOID  bluetooth_hfclient_cpbr_entry_cb(BT_HFCLIENT_CPBR_ENTRY_CB_DATA_T *cpbr_entry_cb);
static INT32 bluetooth_hfclient_cpbr_complete_cb(VOID);


static bool name_equals(const char *first, const char *second) {
  if ((NULL == first) || (NULL == second))
    return false;

  return memcmp(first, second, HFCLIENT_CPBR_NAME_LEN) == 0;
}

static hash_index_t hash_function_name(const void *key) {
  hash_index_t hash = 5381;
  const char *bytes = (const char *)key;
  for (size_t i = 0; i < HFCLIENT_CPBR_NAME_LEN; ++i)
    hash = ((hash << 5) + hash) + bytes[i];
  return hash;
}

static bool name_equality_fn(const void *x, const void *y) {
  return name_equals((char *)x, (char *)y);
}

static void phonebook_entries_lazy_init()
{
  if (!pb_list.entry_list.list)
  {
    pb_list.entry_list.list = hash_map_new(phonebook_entry_buckets,
                                            hash_function_name, NULL, osi_free, name_equality_fn);

    if (NULL == pb_list.entry_list.list)
    {
        BT_DBG_ERROR(BT_DEBUG_HFP, "pb_list.entry_list.list NULL\n");
    }
  }
}

static void phonebook_entry_add(int index, const char *number, int type, const char* name)
{
  BT_HFCLIENT_PHONEBOOK_ENTRY_T *pb_entry_new = NULL;

  /*skip entry no name*/
  if ((NULL == name) || (0 == strlen(name)))
  {
    BT_DBG_ERROR(BT_DEBUG_HFP, "number=%s, name empty!", number ? number : "");
    return;
  }

  phonebook_entries_lazy_init();

  BT_HFCLIENT_PHONEBOOK_ENTRY_T *pb_entry = hash_map_get(pb_list.entry_list.list, name);
  if (pb_entry && (strncmp(pb_entry->number, number, HFCLIENT_CPBR_NUMBER_LEN) == 0))
  {
    BT_DBG_ERROR(BT_DEBUG_HFP, "name=%s, number=%s exists!", name, number);
    return;
  }

  pb_entry_new = osi_calloc(sizeof(BT_HFCLIENT_PHONEBOOK_ENTRY_T));
  strncpy(pb_entry_new->name, name, HFCLIENT_CPBR_NAME_LEN + 1);
  pb_entry_new->name[HFCLIENT_CPBR_NAME_LEN] = '\0';

  strncpy(pb_entry_new->number, number, HFCLIENT_CPBR_NUMBER_LEN + 1);
  pb_entry_new->number[HFCLIENT_CPBR_NUMBER_LEN] = '\0';

  pb_entry_new->index = index;
  pb_entry_new->type = type;
  bool ret = hash_map_set_duplicate(pb_list.entry_list.list, pb_entry_new->name, pb_entry_new);
  if (!ret)
  {
    BT_DBG_ERROR(BT_DEBUG_HFP, "hash_map_set_duplicate failed, name=%s, number=%s, ret=%d\n",
                            pb_entry_new->name, pb_entry_new->number, ret);
  }
}

static void phonebook_entry_remove(const char *name) {
  if (name && pb_list.entry_list.list)
    hash_map_erase(pb_list.entry_list.list, name);
}

static void phonebook_entries_free() {
  BT_DBG_NORMAL(BT_DEBUG_HFP, "begin\n");
  if (pb_list.entry_list.list)
  {
    hash_map_free(pb_list.entry_list.list);
  }
  pb_list.entry_list.list = NULL;
  BT_DBG_NORMAL(BT_DEBUG_HFP, "end\n");
}

static bool phonebook_entries_list_cb(hash_map_entry_t *hash_entry, void *context) {
  BT_HFCLIENT_PHONEBOOK_ENTRY_T *pb_entry = hash_entry->data;

  tBTMW_MSG msg = {0};
  msg.hdr.event = BTMW_HFCLIENT_CPBR_ENTRY_EVT;
  msg.data.hfclient_event.event = BTAPP_HFCLIENT_CPBR_ENTRY_EVT;

  memset(msg.data.hfclient_event.data.pb_entry_app.number, 0, HFCLIENT_CPBR_NUMBER_LEN + 1);
  memset(msg.data.hfclient_event.data.pb_entry_app.name, 0, HFCLIENT_CPBR_NAME_LEN + 1);

  strncpy(msg.data.hfclient_event.data.pb_entry_app.number, pb_entry->number, HFCLIENT_CPBR_NUMBER_LEN + 1);
  msg.data.hfclient_event.data.pb_entry_app.number[HFCLIENT_CPBR_NUMBER_LEN] = '\0';
  strncpy(msg.data.hfclient_event.data.pb_entry_app.name, pb_entry->name, HFCLIENT_CPBR_NAME_LEN + 1);
  msg.data.hfclient_event.data.pb_entry_app.name[HFCLIENT_CPBR_NAME_LEN] = '\0';

  bt_mw_nty_send_msg(&msg);
  return true;
}

static void phonebook_entries_list() {
  if (pb_list.entry_list.list)
  {
    hash_map_foreach(pb_list.entry_list.list, phonebook_entries_list_cb, NULL);

    tBTMW_MSG msg = {0};
    msg.hdr.event = BTMW_HFCLIENT_CPBR_DONE_EVT;
    msg.data.hfclient_event.event = BTAPP_HFCLIENT_CPBR_DONE_EVT;
    bt_mw_nty_send_msg(&msg);
  }
}


static void* bluetooth_hfclient_entry_event_thread()
{
    BT_DBG_NORMAL(BT_DEBUG_HFP, "begin\n");
    BT_HFCLIENT_ENTRY_MSG_T entry_msg;

    if (0 != prctl(PR_SET_NAME,(unsigned long)"pb_entry_thd"))
    {
        BT_DBG_ERROR(BT_DEBUG_HFP, "prctl fail!\n");
    }

    while (1)
    {
        if (pb_list.entry_thd.is_exit)
        {
            BT_DBG_WARNING(BT_DEBUG_HFP, "is_exit true!\n");
            break;
        }

        memset(&entry_msg, 0, sizeof(BT_HFCLIENT_ENTRY_MSG_T));
        if (0 != bluetooth_hfclient_receive_msg(&entry_msg))
        {
            BT_DBG_ERROR(BT_DEBUG_HFP, "receive msg fail!\n");
        }

        BT_DBG_INFO(BT_DEBUG_HFP, "receive msg = %d\n", entry_msg.msg_id);
        switch(entry_msg.msg_id)
        {
            case BT_HFCLIENT_SELECT_PB_STORAGE:
                BT_DBG_INFO(BT_DEBUG_HFP, "BT_HFCLIENT_SELECT_PB_STORAGE\n");
                bluetooth_hfclient_select_pb_storage();
                break;
            case BT_HFCLIENT_SET_PB_STORAGE:
                BT_DBG_INFO(BT_DEBUG_HFP, "BT_HFCLIENT_SET_PB_STORAGE\n");
                int i;
                for (i = 0; i < BT_HFCLIENT_PB_STORAGE_MAX; i++)
                {
                    if (pb_list.entry_list.storage[i].is_exist &&
                        !pb_list.entry_list.storage[i].is_read_done)
                    {
                        bluetooth_hfclient_set_pb_storage((BT_HFCLIENT_PB_STORAGE_TYPE_T)i);
                        pb_list.entry_list.cur_storage = (BT_HFCLIENT_PB_STORAGE_TYPE_T)i;

                        bluetooth_hfclient_test_pb_entry();
                        break;
                    }
                }
                if (BT_HFCLIENT_PB_STORAGE_MAX == i)
                {
                    BT_DBG_NORMAL(BT_DEBUG_HFP, "all pb entries read out!\n");
                    pb_list.entry_list.all_read_done = true;

                    tBTMW_MSG msg = {0};
                    msg.hdr.event = BTMW_HFCLIENT_CPBR_READY_EVT;
                    msg.data.hfclient_event.event = BTAPP_HFCLIENT_CPBR_READY_EVT;
                    bt_mw_nty_send_msg(&msg);
                }

                break;
            case BT_HFCLIENT_SET_CHARSET:
                BT_DBG_INFO(BT_DEBUG_HFP, "BT_HFCLIENT_SET_CHARSET\n");
                bluetooth_hfclient_set_charset(BTA_HF_CLIENT_CHARSET_UTF8);
                break;
            case BT_HFCLIENT_READ_PB_ENTRY:
                BT_DBG_INFO(BT_DEBUG_HFP, "BT_HFCLIENT_READ_PB_ENTRY\n");
                UINT8  cur_storage_idx = pb_list.entry_list.cur_storage;
                UINT16 idx_min = pb_list.entry_list.storage[cur_storage_idx].read_idx + 1;
                UINT16 idx_max = pb_list.entry_list.storage[cur_storage_idx].entry_num;

                if ((idx_max - idx_min) > (BT_HFCLIENT_READ_PB_ENTRIES_NUM - 1))
                {
                    idx_max = idx_min + (BT_HFCLIENT_READ_PB_ENTRIES_NUM - 1);
                }

                bluetooth_hfclient_read_pb_entry(idx_min, idx_max);
                pb_list.entry_list.storage[cur_storage_idx].read_idx = idx_max;
                break;
            case BT_HFCLIENT_READ_ALL_PB_ENTRY:
                BT_DBG_INFO(BT_DEBUG_HFP, "BT_HFCLIENT_READ_ALL_PB_ENTRY\n");
                if (pb_list.entry_list.all_read_done)
                {
                    phonebook_entries_list();
                }
                break;
            case BT_HFCLIENT_EVENT_EXIT:
                BT_DBG_NORMAL(BT_DEBUG_HFP, "BT_HFCLIENT_EVENT_EXIT\n");
                phonebook_entries_free();
                pb_list.entry_thd.is_exit = true;
                break;
            default:
                break;
        }
    }

    if (0 != bluetooth_hfclient_msg_queue_uninit())
    {
        BT_DBG_ERROR(BT_DEBUG_HFP, "uninit message queue fail!\n");
    }
    BT_DBG_NORMAL(BT_DEBUG_HFP, "exit ok\n");
    return NULL;
}

static INT32 bluetooth_hfclient_entry_thd_init()
{
    BT_DBG_NORMAL(BT_DEBUG_HFP, "begin\n");
    pb_list.entry_list.list = NULL;
    INT32 i4_ret = 0;
    pthread_attr_t attr;

    i4_ret = pthread_attr_init(&attr);
    if (0 != i4_ret)
    {
        BT_DBG_ERROR(BT_DEBUG_HFP,"pthread_attr_init failed:%ld\n", (long)i4_ret);
        return i4_ret;
    }

    i4_ret = pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    if (0 != i4_ret)
    {
        BT_DBG_ERROR(BT_DEBUG_HFP,"pthread_attr_setdetachstate failed:%ld\n", (long)i4_ret);
        pthread_attr_destroy(&attr);
        return i4_ret;
    }

    if (0 != pthread_create(&pb_list.entry_thd.thd_id,
                         &attr,
                         bluetooth_hfclient_entry_event_thread,
                         NULL))
    {
       BT_DBG_ERROR(BT_DEBUG_HFP, "init entry_thd failed\n");
       pthread_attr_destroy(&attr);
       return -1;
    }

    pthread_attr_destroy(&attr);
    return 0;
}

static INT32 bluetooth_hfclient_pb_list_init()
{
    BT_DBG_NORMAL(BT_DEBUG_HFP, "begin\n");
    BT_HFCLIENT_ENTRY_MSG_T entry_msg;
    memset(&pb_list, 0, sizeof(BT_HFCLIENT_PHONEBOOK_T));

    if (0 != bluetooth_hfclient_msg_queue_init())
    {
        BT_DBG_ERROR(BT_DEBUG_HFP, "msg queue init faild\n");
        return -1;
    }

    if (0 != bluetooth_hfclient_entry_thd_init())
    {
        BT_DBG_ERROR(BT_DEBUG_HFP, "entry_thd init faild\n");
        if (0 != bluetooth_hfclient_msg_queue_uninit())
        {
            BT_DBG_ERROR(BT_DEBUG_HFP, "uninit message queue fail!\n");
        }
        return -1;
    }

    pb_list.entry_thd.is_exit = false;
    pb_list.entry_list.all_read_done = false;
    for (int i = 0; i < BT_HFCLIENT_PB_STORAGE_MAX; i++)
    {
        pb_list.entry_list.storage[i].is_exist = false;
        pb_list.entry_list.storage[i].is_read_done = false;
        pb_list.entry_list.storage[i].read_idx = 0;
        pb_list.entry_list.storage[i].entry_num = 0;
    }

    /*Set TE charset*/
    memset(&entry_msg, 0, sizeof(BT_HFCLIENT_ENTRY_MSG_T));
    entry_msg.msg_id = BT_HFCLIENT_SET_CHARSET;
    entry_msg.msg_type = BT_HFCLIENT_ENTRY_MSG_TYPE_CTRL;
    if (0 != bluetooth_hfclient_send_msg(&entry_msg))
    {
        BT_DBG_ERROR(BT_DEBUG_HFP, "send message fail!\n");
        return -1;
    }

    /*Select all pb memory storages supported by AG*/
    memset(&entry_msg, 0, sizeof(BT_HFCLIENT_ENTRY_MSG_T));
    entry_msg.msg_id = BT_HFCLIENT_SELECT_PB_STORAGE;
    entry_msg.msg_type = BT_HFCLIENT_ENTRY_MSG_TYPE_CTRL;
    if (0 != bluetooth_hfclient_send_msg(&entry_msg))
    {
        BT_DBG_ERROR(BT_DEBUG_HFP, "send message fail!\n");
        return -1;
    }
    BT_DBG_NORMAL(BT_DEBUG_HFP, "end");
    return 0;
}


static INT32 bluetooth_hfclient_pb_list_uninit()
{
    BT_DBG_NORMAL(BT_DEBUG_HFP, "begin");
    BT_HFCLIENT_ENTRY_MSG_T entry_msg;

    /*thread exit*/
    memset(&entry_msg, 0, sizeof(BT_HFCLIENT_ENTRY_MSG_T));
    entry_msg.msg_id = BT_HFCLIENT_EVENT_EXIT;
    entry_msg.msg_type = BT_HFCLIENT_ENTRY_MSG_TYPE_CTRL;
    if (0 != bluetooth_hfclient_send_msg(&entry_msg))
    {
        BT_DBG_ERROR(BT_DEBUG_HFP, "send message fail!\n");
        return -1;
    }
    BT_DBG_NORMAL(BT_DEBUG_HFP, "end");
    return 0;
}
#endif

INT32 bluetooth_hfclient_enable(VOID)
{
    return linuxbt_hfclient_enable();
}

INT32 bluetooth_hfclient_disable(VOID)
{
    return linuxbt_hfclient_disable();
}

INT32 bluetooth_hfclient_connect(CHAR *bt_addr)
{
    return linuxbt_hfclient_connect(bt_addr);
}

INT32 bluetooth_hfclient_disconnect(CHAR *bt_addr)
{
    return linuxbt_hfclient_disconnect(bt_addr);
}

INT32 bluetooth_hfclient_connect_audio(CHAR *bt_addr)
{
    return linuxbt_hfclient_connect_audio(bt_addr);
}

INT32 bluetooth_hfclient_disconnect_audio(CHAR *bt_addr)
{
    return linuxbt_hfclient_disconnect_audio(bt_addr);
}

INT32 bluetooth_hfclient_start_voice_recognition(VOID)
{
    return linuxbt_hfclient_start_voice_recognition();
}

INT32 bluetooth_hfclient_stop_voice_recognition(VOID)
{
    return linuxbt_hfclient_stop_voice_recognition();
}

INT32 bluetooth_hfclient_volume_control(BT_HFCLIENT_VOLUME_TYPE_T type, INT32 volume)
{
    return linuxbt_hfclient_volume_control(type, volume);
}

INT32 bluetooth_hfclient_dial(const CHAR *number)
{
    return linuxbt_hfclient_dial(number);
}

INT32 bluetooth_hfclient_dial_memory(INT32 location)
{
    return linuxbt_hfclient_dial_memory(location);
}

INT32 bluetooth_hfclient_handle_call_action(BT_HFCLIENT_CALL_ACTION_T action, INT32 idx)
{
    return linuxbt_hfclient_handle_call_action(action, idx);
}

INT32 bluetooth_hfclient_query_current_calls(VOID)
{
    return linuxbt_hfclient_query_current_calls();
}

INT32 bluetooth_hfclient_query_current_operator_name(VOID)
{
    return linuxbt_hfclient_query_current_operator_name();
}

INT32 bluetooth_hfclient_retrieve_subscriber_info(VOID)
{
    return linuxbt_hfclient_retrieve_subscriber_info();
}

INT32 bluetooth_hfclient_send_dtmf(CHAR code)
{
    return linuxbt_hfclient_send_dtmf(code);
}

INT32 bluetooth_hfclient_request_last_voice_tag_number(VOID)
{
    return linuxbt_hfclient_request_last_voice_tag_number();
}

INT32 bluetooth_hfclient_send_at_cmd(INT32 cmd, INT32 val1, INT32 val2, const CHAR *arg)
{
    return linuxbt_hfclient_send_at_cmd(cmd, val1, val2, arg);
}

#if defined(MTK_LINUX_HFP_PHONEBOOK) && (MTK_LINUX_HFP_PHONEBOOK == TRUE)
INT32 bluetooth_hfclient_select_pb_storage()
{
    return linuxbt_hfclient_select_pb_storage();
}

INT32 bluetooth_hfclient_set_pb_storage(BT_HFCLIENT_PB_STORAGE_TYPE_T storage_idx)
{
    return linuxbt_hfclient_set_pb_storage(storage_idx);
}

INT32 bluetooth_hfclient_set_charset(const char* charset)
{
    return linuxbt_hfclient_set_charset(charset);
}

INT32 bluetooth_hfclient_test_pb_entry(VOID)
{
    return linuxbt_hfclient_test_pb_entry();
}

INT32 bluetooth_hfclient_read_pb_entry(UINT16 idx_min, UINT16 idx_max)
{
    return linuxbt_hfclient_read_pb_entry(idx_min, idx_max);
}

/*******************************************************************************
**
** Function         bluetooth_hfclient_read_pb_entries
**
** Description      The function is used to read all phonebook entries stored
**                      in btmw by BT APP.
** Returns          INT32
**
*******************************************************************************/
INT32 bluetooth_hfclient_read_pb_entries(VOID)
{
     BT_DBG_NORMAL(BT_DEBUG_HFP, "begin\n");
     BT_HFCLIENT_ENTRY_MSG_T entry_msg;

     memset(&entry_msg, 0, sizeof(BT_HFCLIENT_ENTRY_MSG_T));
     entry_msg.msg_id = BT_HFCLIENT_READ_ALL_PB_ENTRY;
     entry_msg.msg_type = BT_HFCLIENT_ENTRY_MSG_TYPE_CTRL;
     if (0 != bluetooth_hfclient_send_msg(&entry_msg))
     {
        BT_DBG_ERROR(BT_DEBUG_HFP, "send message fail\n");
        return -1;
     }
     return 0;
}
#endif

VOID bluetooth_hfclient_connection_state_cb(BT_HFCLIENT_CONNECTION_CB_DATA_T *connect_cb)
{
    BT_DBG_NORMAL(BT_DEBUG_HFP, "state=%d, peer_feat=%d, chld_feat=%d, bt_addr=%s\n",
        connect_cb->state, connect_cb->peer_feat, connect_cb->chld_feat, connect_cb->addr);

    tBTMW_MSG msg = {0};
    msg.hdr.event = BTMW_HFCLIENT_CONNECTION_CB_EVT;
    msg.data.hfclient_event.event = BTAPP_HFCLIENT_CONNECTION_CB_EVT;
    memcpy((BT_HFCLIENT_CONNECTION_CB_DATA_T*)&msg.data.hfclient_event.data.connect_cb,
        connect_cb, sizeof(BT_HFCLIENT_CONNECTION_CB_DATA_T));
    bt_mw_nty_send_msg(&msg);

    switch (connect_cb->state)
    {
    case BT_HFCLIENT_CONNECTION_STATE_DISCONNECTED:
        BT_DBG_NORMAL(BT_DEBUG_HFP, "HFP DISCONNECTED\n");
#if defined(MTK_LINUX_HFP_PHONEBOOK) && (MTK_LINUX_HFP_PHONEBOOK == TRUE)
        bluetooth_hfclient_pb_list_uninit();
#endif
        break;
    case BT_HFCLIENT_CONNECTION_STATE_SLC_CONNECTED:
        BT_DBG_NORMAL(BT_DEBUG_HFP, "HFP CONNECTED\n");
#if defined(MTK_LINUX_HFP_PHONEBOOK) && (MTK_LINUX_HFP_PHONEBOOK == TRUE)
        bluetooth_hfclient_pb_list_init();
#endif
        break;
    default:
        break;
    }
}

VOID bluetooth_hfclient_audio_connection_state_cb(BT_HFCLIENT_AUDIO_CONNECTION_CB_DATA_T *audio_connect_cb)
{
    BT_DBG_NORMAL(BT_DEBUG_HFP, "state=%d, bt_addr=%s\n",
        audio_connect_cb->state,
        audio_connect_cb->addr);

    tBTMW_MSG msg = {0};
    msg.hdr.event = BTMW_HFCLIENT_AUDIO_CONNECTION_CB_EVT;
    msg.data.hfclient_event.event = BTAPP_HFCLIENT_AUDIO_CONNECTION_CB_EVT;
    memcpy((BT_HFCLIENT_AUDIO_CONNECTION_CB_DATA_T*)&msg.data.hfclient_event.data.auido_connect_cb,
        audio_connect_cb, sizeof(BT_HFCLIENT_AUDIO_CONNECTION_CB_DATA_T));
    bt_mw_nty_send_msg(&msg);
}

VOID bluetooth_hfclient_vr_cmd_cb(BT_HFCLIENT_BVRA_CB_DATA_T *bvra_cb)
{
    BT_DBG_NORMAL(BT_DEBUG_HFP, "vr_state=%d\n", bvra_cb->vr_state);

    tBTMW_MSG msg = {0};
    msg.hdr.event = BTMW_HFCLIENT_BVRA_CB_EVT;
    msg.data.hfclient_event.event = BTAPP_HFCLIENT_BVRA_CB_EVT;
    memcpy((BT_HFCLIENT_BVRA_CB_DATA_T*)&msg.data.hfclient_event.data.bvra_cb,
        bvra_cb, sizeof(BT_HFCLIENT_BVRA_CB_DATA_T));
    bt_mw_nty_send_msg(&msg);
}

VOID bluetooth_hfclient_indication_service_cb(BT_HFCLIENT_IND_SERVICE_CB_DATA_T *service_cb)
{
    BT_DBG_NORMAL(BT_DEBUG_HFP, "network_state=%d\n", service_cb->network_state);

    tBTMW_MSG msg = {0};
    msg.hdr.event = BTMW_HFCLIENT_IND_SERVICE_CB_EVT;
    msg.data.hfclient_event.event = BTAPP_HFCLIENT_IND_SERVICE_CB_EVT;
    memcpy((BT_HFCLIENT_IND_SERVICE_CB_DATA_T*)&msg.data.hfclient_event.data.service_cb,
        service_cb, sizeof(BT_HFCLIENT_IND_SERVICE_CB_DATA_T));
    bt_mw_nty_send_msg(&msg);
}

VOID bluetooth_hfclient_indication_roaming_cb(BT_HFCLIENT_IND_ROAM_CB_DATA_T *roam_cb)
{
    BT_DBG_NORMAL(BT_DEBUG_HFP, "service_type=%d\n", roam_cb->service_type);

    tBTMW_MSG msg = {0};
    msg.hdr.event = BTMW_HFCLIENT_IND_ROAM_CB_EVT;
    msg.data.hfclient_event.event = BTAPP_HFCLIENT_IND_ROAM_CB_EVT;
    memcpy((BT_HFCLIENT_IND_ROAM_CB_DATA_T*)&msg.data.hfclient_event.data.roam_cb,
        roam_cb, sizeof(BT_HFCLIENT_IND_ROAM_CB_DATA_T));
    bt_mw_nty_send_msg(&msg);
}

VOID bluetooth_hfclient_indication_signal_cb(BT_HFCLIENT_IND_SIGNAL_CB_DATA_T *signal_cb)
{
    BT_DBG_NORMAL(BT_DEBUG_HFP, "signal_strength=%d\n", signal_cb->signal_strength);

    tBTMW_MSG msg = {0};
    msg.hdr.event = BTMW_HFCLIENT_IND_SIGNAL_CB_EVT;
    msg.data.hfclient_event.event = BTAPP_HFCLIENT_IND_SIGNAL_CB_EVT;
    memcpy((BT_HFCLIENT_IND_SIGNAL_CB_DATA_T*)&msg.data.hfclient_event.data.signal_cb,
        signal_cb, sizeof(BT_HFCLIENT_IND_SIGNAL_CB_DATA_T));
    bt_mw_nty_send_msg(&msg);
}

VOID bluetooth_hfclient_indication_battery_cb(BT_HFCLIENT_IND_BATTCH_CB_DATA_T *battery_cb)
{
    BT_DBG_NORMAL(BT_DEBUG_HFP, "battery_level=%d\n", battery_cb->battery_level);

    tBTMW_MSG msg = {0};
    msg.hdr.event = BTMW_HFCLIENT_IND_BATTCH_CB_EVT;
    msg.data.hfclient_event.event = BTAPP_HFCLIENT_IND_BATTCH_CB_EVT;
    memcpy((BT_HFCLIENT_IND_BATTCH_CB_DATA_T*)&msg.data.hfclient_event.data.battery_cb,
        battery_cb, sizeof(BT_HFCLIENT_IND_BATTCH_CB_DATA_T));
    bt_mw_nty_send_msg(&msg);
}

VOID bluetooth_hfclient_current_operator_cb(BT_HFCLIENT_COPS_CB_DATA_T *cops_cb)
{
    BT_DBG_NORMAL(BT_DEBUG_HFP, "operator_name=%s\n", cops_cb->operator_name);

    tBTMW_MSG msg = {0};
    msg.hdr.event = BTMW_HFCLIENT_COPS_CB_EVT;
    msg.data.hfclient_event.event = BTAPP_HFCLIENT_COPS_CB_EVT;
    memcpy((BT_HFCLIENT_COPS_CB_DATA_T*)&msg.data.hfclient_event.data.cops_cb,
        cops_cb, sizeof(BT_HFCLIENT_COPS_CB_DATA_T));
    bt_mw_nty_send_msg(&msg);
}

VOID bluetooth_hfclient_call_cb(BT_HFCLIENT_IND_CALL_CB_DATA_T  *call_cb)
{
    BT_DBG_NORMAL(BT_DEBUG_HFP, "call=%d\n", call_cb->call);
    tBTMW_MSG msg = {0};
    msg.hdr.event = BTMW_HFCLIENT_IND_CALL_CB_EVT;
    msg.data.hfclient_event.event = BTAPP_HFCLIENT_IND_CALL_CB_EVT;
    memcpy((BT_HFCLIENT_IND_CALL_CB_DATA_T*)&msg.data.hfclient_event.data.call_cb,
        call_cb, sizeof(BT_HFCLIENT_IND_CALL_CB_DATA_T));
    bt_mw_nty_send_msg(&msg);
}

VOID bluetooth_hfclient_callsetup_cb(BT_HFCLIENT_IND_CALLSETUP_CB_DATA_T *callsetup_cb)
{
    BT_DBG_NORMAL(BT_DEBUG_HFP, "callsetup=%d\n", callsetup_cb->callsetup);
    tBTMW_MSG msg = {0};
    msg.hdr.event = BTMW_HFCLIENT_IND_CALLSETUP_CB_EVT;
    msg.data.hfclient_event.event = BTAPP_HFCLIENT_IND_CALLSETUP_CB_EVT;
    memcpy((BT_HFCLIENT_IND_CALLSETUP_CB_DATA_T*)&msg.data.hfclient_event.data.callsetup_cb,
        callsetup_cb, sizeof(BT_HFCLIENT_IND_CALLSETUP_CB_DATA_T));
    bt_mw_nty_send_msg(&msg);
}

VOID bluetooth_hfclient_callheld_cb(BT_HFCLIENT_IND_CALLHELD_CB_DATA_T *callheld_cb)
{
    BT_DBG_NORMAL(BT_DEBUG_HFP, "callheld=%d\n", callheld_cb->callheld);
    tBTMW_MSG msg = {0};
    msg.hdr.event = BTMW_HFCLIENT_IND_CALLHELD_CB_EVT;
    msg.data.hfclient_event.event = BTAPP_HFCLIENT_IND_CALLHELD_CB_EVT;
    memcpy((BT_HFCLIENT_IND_CALLHELD_CB_DATA_T*)&msg.data.hfclient_event.data.callheld_cb,
        callheld_cb, sizeof(BT_HFCLIENT_IND_CALLHELD_CB_DATA_T));
    bt_mw_nty_send_msg(&msg);
}

VOID bluetooth_hfclient_resp_and_hold_cb(BT_HFCLIENT_BTRH_CB_DATA_T *btrh_cb)
{
    BT_DBG_NORMAL(BT_DEBUG_HFP, "resp_and_hold=%d\n", btrh_cb->resp_and_hold);
    tBTMW_MSG msg = {0};
    msg.hdr.event = BTMW_HFCLIENT_BTRH_CB_EVT;
    msg.data.hfclient_event.event = BTAPP_HFCLIENT_BTRH_CB_EVT;
    memcpy((BT_HFCLIENT_BTRH_CB_DATA_T*)&msg.data.hfclient_event.data.btrh_cb,
        btrh_cb, sizeof(BT_HFCLIENT_BTRH_CB_DATA_T));
    bt_mw_nty_send_msg(&msg);
}

VOID bluetooth_hfclient_clip_cb(BT_HFCLIENT_CLIP_CB_DATA_T *clip_cb)
{
    BT_DBG_NORMAL(BT_DEBUG_HFP, "number=%s\n", clip_cb->number);
    tBTMW_MSG msg = {0};
    msg.hdr.event = BTMW_HFCLIENT_CLIP_CB_EVT;
    msg.data.hfclient_event.event = BTAPP_HFCLIENT_CLIP_CB_EVT;
    memcpy((BT_HFCLIENT_CLIP_CB_DATA_T*)&msg.data.hfclient_event.data.clip_cb,
        clip_cb, sizeof(BT_HFCLIENT_CLIP_CB_DATA_T));
    bt_mw_nty_send_msg(&msg);
}

VOID bluetooth_hfclient_call_waiting_cb(BT_HFCLIENT_CCWA_CB_DATA_T *ccwa_cb)
{
    BT_DBG_NORMAL(BT_DEBUG_HFP, "number=%s\n", ccwa_cb->number);
    tBTMW_MSG msg = {0};
    msg.hdr.event = BTMW_HFCLIENT_CCWA_CB_EVT;
    msg.data.hfclient_event.event = BTAPP_HFCLIENT_CCWA_CB_EVT;
    memcpy((BT_HFCLIENT_CCWA_CB_DATA_T*)&msg.data.hfclient_event.data.ccwa_cb,
        ccwa_cb, sizeof(BT_HFCLIENT_CCWA_CB_DATA_T));
    bt_mw_nty_send_msg(&msg);
}

VOID bluetooth_hfclient_current_calls_cb(BT_HFCLIENT_CLCC_CB_DATA_T *clcc_cb)
{
    BT_DBG_NORMAL(BT_DEBUG_HFP, "index=%d, dir=%d, state=%d, mpty=%d, number=%s\n",
                   clcc_cb->index, clcc_cb->dir, clcc_cb->mpty, clcc_cb->number);

    tBTMW_MSG msg = {0};
    msg.hdr.event = BTMW_HFCLIENT_CLCC_CB_EVT;
    msg.data.hfclient_event.event = BTAPP_HFCLIENT_CLCC_CB_EVT;
    memcpy((BT_HFCLIENT_CLCC_CB_DATA_T*)&msg.data.hfclient_event.data.clcc_cb,
        clcc_cb, sizeof(BT_HFCLIENT_CLCC_CB_DATA_T));
    bt_mw_nty_send_msg(&msg);
}

VOID bluetooth_hfclient_volume_change_cb(BT_HFCLIENT_VGM_VGS_CB_DATA_T *volume_cb)
{
    BT_DBG_NORMAL(BT_DEBUG_HFP, "type=%d, volume=%d\n", volume_cb->type, volume_cb->volume);

    tBTMW_MSG msg = {0};
    msg.hdr.event = BTMW_HFCLIENT_VGM_VGS_CB_EVT;
    msg.data.hfclient_event.event = BTAPP_HFCLIENT_VGM_VGS_CB_EVT;
    memcpy((BT_HFCLIENT_VGM_VGS_CB_DATA_T*)&msg.data.hfclient_event.data.volume_cb,
        volume_cb, sizeof(BT_HFCLIENT_VGM_VGS_CB_DATA_T));
    bt_mw_nty_send_msg(&msg);
}

VOID bluetooth_hfclient_cmd_complete_cb(BT_HFCLIENT_CMD_COMPLETE_CB_DATA_T *cmd_complete_cb)
{
    BT_DBG_NORMAL(BT_DEBUG_HFP, "type=%d, cme=%d\n", cmd_complete_cb->type, cmd_complete_cb->cme);

    tBTMW_MSG msg = {0};
    msg.hdr.event = BTMW_HFCLIENT_CMD_COMPLETE_CB_EVT;
    msg.data.hfclient_event.event = BTAPP_HFCLIENT_CMD_COMPLETE_CB_EVT;
    memcpy((BT_HFCLIENT_CMD_COMPLETE_CB_DATA_T*)&msg.data.hfclient_event.data.cmd_complete_cb,
        cmd_complete_cb, sizeof(BT_HFCLIENT_CMD_COMPLETE_CB_DATA_T));
    bt_mw_nty_send_msg(&msg);
}

VOID bluetooth_hfclient_subscriber_info_cb(BT_HFCLIENT_CNUM_CB_DATA_T *cnum_cb)
{
    BT_DBG_NORMAL(BT_DEBUG_HFP, "number=%s, type=%d\n", cnum_cb->number, cnum_cb->type);

    tBTMW_MSG msg = {0};
    msg.hdr.event = BTMW_HFCLIENT_CNUM_CB_EVT;
    msg.data.hfclient_event.event = BTAPP_HFCLIENT_CNUM_CB_EVT;
    memcpy((BT_HFCLIENT_CNUM_CB_DATA_T*)&msg.data.hfclient_event.data.cnum_cb,
        cnum_cb, sizeof(BT_HFCLIENT_CNUM_CB_DATA_T));
    bt_mw_nty_send_msg(&msg);
}

VOID bluetooth_hfclient_in_band_ring_tone_cb(BT_HFCLIENT_BSIR_CB_DATA_T *bsir_cb)
{
    BT_DBG_NORMAL(BT_DEBUG_HFP, "state=%d\n", bsir_cb->state);

    tBTMW_MSG msg = {0};
    msg.hdr.event = BTMW_HFCLIENT_BSIR_CB_EVT;
    msg.data.hfclient_event.event = BTAPP_HFCLIENT_BSIR_CB_EVT;
    memcpy((BT_HFCLIENT_BSIR_CB_DATA_T*)&msg.data.hfclient_event.data.bsir_cb,
        bsir_cb, sizeof(BT_HFCLIENT_BSIR_CB_DATA_T));
    bt_mw_nty_send_msg(&msg);
}

VOID bluetooth_hfclient_last_voice_tag_number_cb(BT_HFCLIENT_BINP_CB_DATA_T *binp_cb)
{
    BT_DBG_NORMAL(BT_DEBUG_HFP, "number=%s\n", binp_cb->number);

    tBTMW_MSG msg = {0};
    msg.hdr.event = BTMW_HFCLIENT_BINP_CB_EVT;
    msg.data.hfclient_event.event = BTAPP_HFCLIENT_BINP_CB_EVT;
    memcpy((BT_HFCLIENT_BINP_CB_DATA_T*)&msg.data.hfclient_event.data.binp_cb,
        binp_cb, sizeof(BT_HFCLIENT_BINP_CB_DATA_T));
    bt_mw_nty_send_msg(&msg);
}

VOID bluetooth_hfclient_ring_indication_cb(VOID)
{
    BT_DBG_NORMAL(BT_DEBUG_HFP, "ring indication!\n");

    tBTMW_MSG msg = {0};
    msg.hdr.event = BTMW_HFCLIENT_RING_IND_CB_EVT;
    msg.data.hfclient_event.event = BTAPP_HFCLIENT_RING_IND_CB_EVT;
    bt_mw_nty_send_msg(&msg);
}

VOID bluetooth_hfclient_ability_state_cb(BT_HFCLIENT_ABILITY_CB_DATA_T *ability_cb)
{
    BT_DBG_NORMAL(BT_DEBUG_HFP, "ability state=%s\n", ability_cb->state ? "DISABLE" : "ENABLE");

    tBTMW_MSG msg = {0};
    msg.hdr.event = BTMW_HFCLIENT_ABILITY_CB_EVT;
    msg.data.hfclient_event.event = BTAPP_HFCLIENT_ABILITY_CB_EVT;
    memcpy((BT_HFCLIENT_ABILITY_CB_DATA_T*)&msg.data.hfclient_event.data.ability_cb,
        ability_cb, sizeof(BT_HFCLIENT_ABILITY_CB_DATA_T));
    bt_mw_nty_send_msg(&msg);
}


#if defined(MTK_LINUX_HFP_PHONEBOOK) && (MTK_LINUX_HFP_PHONEBOOK == TRUE)
INT32 bluetooth_hfclient_cpbs_cb(BT_HFCLIENT_CPBS_CB_DATA_T *cpbs_cb)
{
    BT_HFCLIENT_ENTRY_MSG_T entry_msg;

    BT_DBG_NORMAL(BT_DEBUG_HFP, "begin!\n");
    if ((NULL == cpbs_cb) || (NULL == cpbs_cb->storage_lookup))
    {
        return -1;
    }

    for (int i = 0; i < BT_HFCLIENT_PB_STORAGE_MAX; i++)
    {
        if (1 == cpbs_cb->storage_lookup[i])
        {
            pb_list.entry_list.storage[i].is_exist = true;
        }
    }

    memset(&entry_msg, 0, sizeof(BT_HFCLIENT_ENTRY_MSG_T));
    entry_msg.msg_id = BT_HFCLIENT_SET_PB_STORAGE;
    entry_msg.msg_type = BT_HFCLIENT_ENTRY_MSG_TYPE_CTRL;
    if (0 != bluetooth_hfclient_send_msg(&entry_msg))
    {
        BT_DBG_ERROR(BT_DEBUG_HFP, "send message fail!\n");
        return -1;
    }
    return 0;
}

INT32 bluetooth_hfclient_cpbr_count_cb(BT_HFCLIENT_CPBR_COUNT_CB_DATA_T *cpbr_count_cb)
{
    BT_HFCLIENT_ENTRY_MSG_T entry_msg;

    BT_DBG_NORMAL(BT_DEBUG_HFP, "storage index=%d, entry count=%d\n",
                                pb_list.entry_list.cur_storage, cpbr_count_cb->idx_max);
    pb_list.entry_list.storage[pb_list.entry_list.cur_storage].entry_num= cpbr_count_cb->idx_max;

    memset(&entry_msg, 0, sizeof(BT_HFCLIENT_ENTRY_MSG_T));
    entry_msg.msg_id = BT_HFCLIENT_READ_PB_ENTRY;
    entry_msg.msg_type = BT_HFCLIENT_ENTRY_MSG_TYPE_CTRL;
    if (0 != bluetooth_hfclient_send_msg(&entry_msg))
    {
        BT_DBG_ERROR(BT_DEBUG_HFP, "send message fail!\n");
        return -1;
    }
    return 0;
}

VOID bluetooth_hfclient_cpbr_entry_cb(BT_HFCLIENT_CPBR_ENTRY_CB_DATA_T *cpbr_entry_cb)
{
    phonebook_entry_add(cpbr_entry_cb->pb_entry.index,
                        cpbr_entry_cb->pb_entry.number,
                        cpbr_entry_cb->pb_entry.type,
                        cpbr_entry_cb->pb_entry.name);
}


INT32 bluetooth_hfclient_cpbr_complete_cb(VOID)
{
    BT_HFCLIENT_ENTRY_MSG_T entry_msg;
    BT_DBG_INFO(BT_DEBUG_HFP, "cpbr complete cb!\n");
    UINT8  cur_storage_idx = pb_list.entry_list.cur_storage;
    UINT16 read_idx = pb_list.entry_list.storage[cur_storage_idx].read_idx;
    UINT16 entry_num = pb_list.entry_list.storage[cur_storage_idx].entry_num;
    if (entry_num == read_idx)
    {
        BT_DBG_INFO(BT_DEBUG_HFP, "storage=%d read complete!\n", cur_storage_idx);
        pb_list.entry_list.storage[cur_storage_idx].is_read_done = true;

        memset(&entry_msg, 0, sizeof(BT_HFCLIENT_ENTRY_MSG_T));
        entry_msg.msg_id = BT_HFCLIENT_SET_PB_STORAGE;
        entry_msg.msg_type = BT_HFCLIENT_ENTRY_MSG_TYPE_CTRL;
        if (0 != bluetooth_hfclient_send_msg(&entry_msg))
        {
            BT_DBG_ERROR(BT_DEBUG_HFP, "send message fail!\n");
            return -1;
        }
    }
    else
    {
        BT_DBG_INFO(BT_DEBUG_HFP, "storage index=%d ,read next index!\n", pb_list.entry_list.cur_storage);

        memset(&entry_msg, 0, sizeof(BT_HFCLIENT_ENTRY_MSG_T));
        entry_msg.msg_id = BT_HFCLIENT_READ_PB_ENTRY;
        entry_msg.msg_type = BT_HFCLIENT_ENTRY_MSG_TYPE_CTRL;
        if (0 != bluetooth_hfclient_send_msg(&entry_msg))
        {
            BT_DBG_ERROR(BT_DEBUG_HFP, "send message fail!\n");
            return -1;
        }
    }
    return 0;
}


static INT32 bluetooth_hfclient_msg_queue_init()
{
    key_t key;
    BT_DBG_NORMAL(BT_DEBUG_HFP, "begin\n");

    //remember to set the path
    key = ftok(".", 1);
    if (-1 == key)
    {
        BT_DBG_ERROR(BT_DEBUG_HFP, "ftok failed!\n");
        return -1;
    }

    pb_list.msg_q_id = msgget(key, IPC_CREAT);
    if (-1 == pb_list.msg_q_id)
    {
        BT_DBG_ERROR(BT_DEBUG_HFP, "msgget failed!\n");
        return -1;
    }
    BT_DBG_NORMAL(BT_DEBUG_HFP, "end!\n");
    return 0;
}


static INT32 bluetooth_hfclient_msg_queue_uninit()
{
    BT_DBG_NORMAL(BT_DEBUG_HFP, "begin!\n");
    if (-1 == msgctl(pb_list.msg_q_id, IPC_RMID, NULL))
    {
        BT_DBG_ERROR(BT_DEBUG_HFP, "msgctl failed!\n");
        return -1;
    }
    BT_DBG_NOTICE(BT_DEBUG_HFP, "end!\n");
    return 0;
}


static INT32 bluetooth_hfclient_send_msg(BT_HFCLIENT_ENTRY_MSG_T* p_send_msg)
{
    uint32_t msg_len = 0;
    if (NULL == p_send_msg)
    {
        BT_DBG_ERROR(BT_DEBUG_HFP, "send message is null\n");
        return -1;
    }
    msg_len = sizeof(BT_HFCLIENT_ENTRY_MSG_T) - sizeof(long int);
    if (-1 == msgsnd(pb_list.msg_q_id,
                    (void *)p_send_msg, msg_len, 0))
    {
        BT_DBG_ERROR(BT_DEBUG_HFP, "msgsnd failed!\n");
        return -1;
    }
    return 0;
}

static INT32 bluetooth_hfclient_receive_msg(BT_HFCLIENT_ENTRY_MSG_T* p_receive_msg)
{
    uint32_t msg_len = 0;
    if (NULL == p_receive_msg)
    {
        BT_DBG_ERROR(BT_DEBUG_HFP, "receive message is null\n");
        return -1;
    }
    msg_len = sizeof(BT_HFCLIENT_ENTRY_MSG_T) - sizeof(long int);

    if (-1 == msgrcv(pb_list.msg_q_id,
                        (void *)p_receive_msg, msg_len, 0, 0))
    {
        BT_DBG_ERROR(BT_DEBUG_HFP, "msgsnd failed!\n");
        return -1;
    }
    return 0;
}
#endif

/*******************************************************************************
**
** Function         bluetooth_hfclient_msg_handle
**
** Description      The function is used to handle HFP Client message sent from
**                      linuxbt
** Returns          void
**
*******************************************************************************/
static VOID bluetooth_hfclient_msg_handle(tBTMW_MSG *p_msg)
{
    tBT_MW_HFCLIENT_MSG *hfp_msg = &p_msg->data.hfclient_msg;

    switch(p_msg->hdr.event)
    {
        case BTMW_HFCLIENT_CONNECTION_CB_EVT:
            bluetooth_hfclient_connection_state_cb(&hfp_msg->data.connect_cb);
            break;
        case BTMW_HFCLIENT_AUDIO_CONNECTION_CB_EVT:
            bluetooth_hfclient_audio_connection_state_cb(&hfp_msg->data.auido_connect_cb);
            break;
        case BTMW_HFCLIENT_BVRA_CB_EVT:
            bluetooth_hfclient_vr_cmd_cb(&hfp_msg->data.bvra_cb);
            break;
        case BTMW_HFCLIENT_IND_SERVICE_CB_EVT:
            bluetooth_hfclient_indication_service_cb(&hfp_msg->data.service_cb);
            break;
        case BTMW_HFCLIENT_IND_ROAM_CB_EVT:
            bluetooth_hfclient_indication_roaming_cb(&hfp_msg->data.roam_cb);
            break;
        case BTMW_HFCLIENT_IND_SIGNAL_CB_EVT:
            bluetooth_hfclient_indication_signal_cb(&hfp_msg->data.signal_cb);
            break;
        case BTMW_HFCLIENT_IND_BATTCH_CB_EVT:
            bluetooth_hfclient_indication_battery_cb(&hfp_msg->data.battery_cb);
            break;
        case BTMW_HFCLIENT_COPS_CB_EVT:
            bluetooth_hfclient_current_operator_cb(&hfp_msg->data.cops_cb);
            break;
        case BTMW_HFCLIENT_IND_CALL_CB_EVT:
            bluetooth_hfclient_call_cb(&hfp_msg->data.call_cb);
            break;
        case BTMW_HFCLIENT_IND_CALLSETUP_CB_EVT:
            bluetooth_hfclient_callsetup_cb(&hfp_msg->data.callsetup_cb);
            break;
        case BTMW_HFCLIENT_IND_CALLHELD_CB_EVT:
            bluetooth_hfclient_callheld_cb(&hfp_msg->data.callheld_cb);
            break;
        case BTMW_HFCLIENT_BTRH_CB_EVT:
            bluetooth_hfclient_resp_and_hold_cb(&hfp_msg->data.btrh_cb);
            break;
        case BTMW_HFCLIENT_CLIP_CB_EVT:
            bluetooth_hfclient_clip_cb(&hfp_msg->data.clip_cb);
            break;
        case BTMW_HFCLIENT_CCWA_CB_EVT:
            bluetooth_hfclient_call_waiting_cb(&hfp_msg->data.ccwa_cb);
            break;
        case BTMW_HFCLIENT_CLCC_CB_EVT:
            bluetooth_hfclient_current_calls_cb(&hfp_msg->data.clcc_cb);
            break;
        case BTMW_HFCLIENT_VGM_VGS_CB_EVT:
            bluetooth_hfclient_volume_change_cb(&hfp_msg->data.volume_cb);
            break;
        case BTMW_HFCLIENT_CMD_COMPLETE_CB_EVT:
            bluetooth_hfclient_cmd_complete_cb(&hfp_msg->data.cmd_complete_cb);
            break;
        case BTMW_HFCLIENT_CNUM_CB_EVT:
            bluetooth_hfclient_subscriber_info_cb(&hfp_msg->data.cnum_cb);
            break;
        case BTMW_HFCLIENT_BSIR_CB_EVT:
            bluetooth_hfclient_in_band_ring_tone_cb(&hfp_msg->data.bsir_cb);
            break;
        case BTMW_HFCLIENT_BINP_CB_EVT:
            bluetooth_hfclient_last_voice_tag_number_cb(&hfp_msg->data.binp_cb);
            break;
        case BTMW_HFCLIENT_RING_IND_CB_EVT:
            bluetooth_hfclient_ring_indication_cb();
            break;
        case BTMW_HFCLIENT_ABILITY_CB_EVT:
            bluetooth_hfclient_ability_state_cb(&hfp_msg->data.ability_cb);
            break;
#if defined(MTK_LINUX_HFP_PHONEBOOK) && (MTK_LINUX_HFP_PHONEBOOK == TRUE)
        case BTMW_HFCLIENT_CPBS_CB_EVT:
            bluetooth_hfclient_cpbs_cb(&hfp_msg->data.cpbs_cb);
            break;
        case BTMW_HFCLIENT_CPBR_COUNT_CB_EVT:
            bluetooth_hfclient_cpbr_count_cb(&hfp_msg->data.cpbr_count_cb);
            break;
        case BTMW_HFCLIENT_CPBR_ENTRY_CB_EVT:
            bluetooth_hfclient_cpbr_entry_cb(&hfp_msg->data.cpbr_entry_cb);
            break;
        case BTMW_HFCLIENT_CPBR_COMPLETE_CB_EVT:
            bluetooth_hfclient_cpbr_complete_cb();
            break;
#endif
        default:
            break;
    }
}


/*******************************************************************************
**
** Function         bluetooth_hfclient_msg_handle
**
** Description      The function is used to handle HFP Client event sent from
**                      MW itself to App
** Returns          void
**
*******************************************************************************/
static VOID  bluetooth_hfclient_notify_handle(tBTMW_MSG *p_msg)
{
    BT_HFCLIENT_EVENT_PARAM *hfclient_event = &p_msg->data.hfclient_event;

    switch(p_msg->hdr.event)
    {
        case BTMW_HFCLIENT_CONNECTION_CB_EVT:
        case BTMW_HFCLIENT_AUDIO_CONNECTION_CB_EVT:
        case BTMW_HFCLIENT_BVRA_CB_EVT:
        case BTMW_HFCLIENT_IND_SERVICE_CB_EVT:
        case BTMW_HFCLIENT_IND_ROAM_CB_EVT:
        case BTMW_HFCLIENT_IND_SIGNAL_CB_EVT:
        case BTMW_HFCLIENT_IND_BATTCH_CB_EVT:
        case BTMW_HFCLIENT_COPS_CB_EVT:
        case BTMW_HFCLIENT_IND_CALL_CB_EVT:
        case BTMW_HFCLIENT_IND_CALLSETUP_CB_EVT:
        case BTMW_HFCLIENT_IND_CALLHELD_CB_EVT:
        case BTMW_HFCLIENT_BTRH_CB_EVT:
        case BTMW_HFCLIENT_CLIP_CB_EVT:
        case BTMW_HFCLIENT_CCWA_CB_EVT:
        case BTMW_HFCLIENT_CLCC_CB_EVT:
        case BTMW_HFCLIENT_VGM_VGS_CB_EVT:
        case BTMW_HFCLIENT_CMD_COMPLETE_CB_EVT:
        case BTMW_HFCLIENT_CNUM_CB_EVT:
        case BTMW_HFCLIENT_BSIR_CB_EVT:
        case BTMW_HFCLIENT_BINP_CB_EVT:
        case BTMW_HFCLIENT_RING_IND_CB_EVT:
        case BTMW_HFCLIENT_ABILITY_CB_EVT:
#if defined(MTK_LINUX_HFP_PHONEBOOK) && (MTK_LINUX_HFP_PHONEBOOK == TRUE)
        case BTMW_HFCLIENT_CPBR_ENTRY_EVT:
        case BTMW_HFCLIENT_CPBR_READY_EVT:
        case BTMW_HFCLIENT_CPBR_DONE_EVT:
#endif
            if(g_bt_mw_hfclient_event_handle_cb)
            {
                BT_DBG_INFO(BT_DEBUG_HFP, "notify to app event:%d",
                    hfclient_event->event);
                g_bt_mw_hfclient_event_handle_cb(hfclient_event);
            }
            break;
        default:
            break;
    }
}

INT32 bluetooth_hfclient_register_callback(BT_HFCLIENT_EVENT_HANDLE_CB hfclient_handle)
{
    int ret = BT_SUCCESS;
    if (NULL != hfclient_handle)
    {
        ret = bluetooth_hfclient_init();
    }
    else
    {
        ret = bluetooth_hfclient_deinit();
    }
    g_bt_mw_hfclient_event_handle_cb = hfclient_handle;
    return ret;
}

INT32 bluetooth_hfclient_init(VOID)
{
    BT_DBG_NORMAL(BT_DEBUG_HFP, "\n");
    linuxbt_hdl_register(BTWM_ID_HFP, bluetooth_hfclient_msg_handle);
    bt_mw_nty_hdl_register(BTWM_ID_HFP, bluetooth_hfclient_notify_handle);

    return linuxbt_hfclient_init();
}

INT32 bluetooth_hfclient_deinit(VOID)
{
    BT_DBG_NORMAL(BT_DEBUG_HFP, "\n");
    return linuxbt_hfclient_deinit();
}


