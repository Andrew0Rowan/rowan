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

/* FILE NAME:  bt_mw_avrcp.c
 * PURPOSE:
 *  {1. What is covered in this file - function and scope.}
 *  {2. Related documents or hardware information}
 * NOTES:
 *  {Something must be known or noticed}
 *  {1. How to use these functions - Give an example.}
 *  {2. Sequence of messages if applicable.}
 *  {3. Any design limitation}
 *  {4. Any performance limitation}
 *  {5. Is it a reusable component}
 *
 *
 *
 */
/* INCLUDE FILE DECLARATIONS
 */
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>

#include "bt_mw_common.h"
#include "bt_mw_avrcp.h"
#include "c_mw_config.h"

#include "linuxbt_avrcp_ct_if.h"
#include "linuxbt_avrcp_tg_if.h"

#include "bt_mw_message_queue.h"

#include "mtk_bt_rc.h"
/* NAMING CONSTANT DECLARATIONS
 */
#define BT_MW_AVRCP_MAX_DEVICES  (3)

typedef enum
{
    BT_MW_AVRCP_CONNECT_STATUS_DISCONNECTED,
    BT_MW_AVRCP_CONNECT_STATUS_CONNECTED,
    BT_MW_AVRCP_CONNECT_STATUS_MAX
} BT_MW_AVRCP_CONNECT_STATUS;

/* MACRO FUNCTION DECLARATIONS
 */

#define BT_MW_AVRCP_LOCK()   pthread_mutex_lock(&g_bt_mw_avrcp_cb.lock)

#define BT_MW_AVRCP_UNLOCK() pthread_mutex_unlock(&g_bt_mw_avrcp_cb.lock)

#define BT_MW_AVRCP_IS_VALID_ADDR(addr) do{                                         \
        if ((NULL == addr) || (MAX_BDADDR_LEN - 1 != strlen(addr)))                 \
        {                                                                           \
            BT_DBG_ERROR(BT_DEBUG_AVRCP, "invalid addr(%s)", addr==NULL?"NULL":addr);\
            return -1;                                                               \
        }                                                                           \
    }while(0)

#define BT_MW_AVRCP_IS_VALID_ADDR_RETURN(addr) do{                                      \
        if ((NULL == addr) || (MAX_BDADDR_LEN - 1 != strlen(addr)))                     \
        {                                                                               \
            BT_DBG_ERROR(BT_DEBUG_AVRCP, "invalid addr(%s)", addr==NULL?"NULL":addr);   \
            return;                                                                  \
        }                                                                               \
    }while(0)

#define BT_MW_AVRCP_REPORT_EVENT(param) bt_mw_avrcp_notify_app(&param)

#define BT_MW_AVRCP_CHECK_INITED_RETURN    do {              \
        if (FALSE == g_bt_mw_avrcp_cb.inited)                \
        {                                                    \
            BT_DBG_ERROR(BT_DEBUG_AVRCP, "avrcp not init");  \
            return;                                          \
        }                                                    \
    }while(0)

#define BT_MW_AVRCP_CHECK_INITED(ret)    do {                \
        if (FALSE == g_bt_mw_avrcp_cb.inited)                \
        {                                                    \
            BT_DBG_ERROR(BT_DEBUG_AVRCP, "avrcp not init");  \
            return ret;                                      \
        }                                                    \
    }while(0)

/* DATA TYPE DECLARATIONS
 */

typedef struct
{
    CHAR addr[MAX_BDADDR_LEN];
    BT_MW_AVRCP_ROLE role;
    BT_MW_AVRCP_CONNECT_STATUS conn_status;
    UINT32 ct_feature; /* local is CT and remote support feature */
    UINT32 tg_feature; /* local is TG and remote support feature */
    UINT32 reg_event;
    UINT8 abs_vol_lable;
    BOOL in_use;
} BT_MW_AVRCP_DEV;


typedef struct
{
    BT_MW_AVRCP_DEV devices[BT_MW_AVRCP_MAX_DEVICES];
    BT_AVRCP_PLAYER_STATUS player_status;
    BT_AVRCP_PLAYER_MEDIA_INFO player_media_info;
    BT_AVRCP_PLAYER_SETTING player_setting;
    BT_AVRCP_ADDR_PLAYER addr_player;
    UINT8 abs_volume; /* unit: 1% */
    BOOL inited;
    pthread_mutex_t lock;
} BT_MW_AVRCP_CB;

/* GLOBAL VARIABLE DECLARATIONS
 */
/* LOCAL SUBPROGRAM DECLARATIONS
 */

static INT32 bt_mw_avrcp_init(VOID);

static INT32 bt_mw_avrcp_deinit(VOID);

static BOOL bt_mw_avrcp_is_connected(CHAR *addr);

static INT32 bt_mw_avrcp_get_dev_index(CHAR *addr);

static INT32 bt_mw_avrcp_alloc_dev_index(CHAR *addr);

static INT32 bt_mw_avrcp_free_dev(INT32 index);

static VOID bt_mw_avrcp_handle_connect_cb(CHAR *addr, BT_MW_AVRCP_ROLE role);

static VOID bt_mw_avrcp_handle_disconnect_cb(CHAR *addr, BT_MW_AVRCP_ROLE role);

static VOID bt_mw_avrcp_handle_feature_cb(CHAR *addr,
    BT_MW_AVRCP_ROLE role, UINT32 feature);

static VOID bt_mw_avrcp_handle_set_volume_req(CHAR *addr,
    UINT8 label, UINT8 volume);


static VOID bt_mw_avrcp_handle_reg_event_req(CHAR *addr, UINT8 label,
    BT_MW_AVRCP_REG_EVENT_REQ *reg_event);

static VOID bt_mw_avrcp_handle_track_change(CHAR *addr,
    BT_AVRCP_MEDIA_INFO *media_info);

static VOID bt_mw_avrcp_handle_play_position_change(CHAR *addr,
    BT_AVRCP_POS_CHANGE *pos_change);

static VOID bt_mw_avrcp_handle_play_status_change(CHAR *addr,
    BT_AVRCP_PLAY_STATUS status);

static VOID bt_mw_avrcp_handle_passthrough_cmd_req(CHAR *addr,
    BT_AVRCP_PASSTHROUGH_CMD_REQ *key);

static VOID bt_mw_avrcp_handle_get_play_status_req(CHAR *addr);


static VOID bt_mw_avrcp_handle_get_element_attr_req(CHAR *addr,
    BT_MW_AVRCP_ELEMENT_ATTR_REQ *req);

static VOID bt_mw_avrcp_handle_volume_change_cb(CHAR *addr, UINT8 volume);

static INT32 bt_mw_avrcp_avrcp_event_change(CHAR *addr,
    BT_AVRCP_REG_EVT_ID event, BOOL interim, BT_AVRCP_EVENT_CHANGE_RSP *data);

static INT32 bt_mw_avrcp_volume_change(CHAR *addr, BOOL interim, UINT8 volume);

static VOID bt_mw_avrcp_msg_handle(tBTMW_MSG *p_msg);

static VOID bt_mw_avrcp_notify_handle(tBTMW_MSG *p_msg);

static VOID bt_mw_avrcp_notify_app(BT_AVRCP_EVENT_PARAM *param);

static VOID bt_mw_avrcp_print_key(BT_AVRCP_CMD_TYPE cmd_type,
    BT_AVRCP_KEY_STATE key_state);

static CHAR* bt_mw_avrcp_get_reg_event_str(BT_AVRCP_REG_EVT_ID event);

static CHAR* bt_mw_avrcp_get_event_str(UINT32 event);

static CHAR* bt_mw_avrcp_get_app_event_str(UINT32 event);

/* STATIC VARIABLE DECLARATIONS
 */
static BT_MW_AVRCP_CB g_bt_mw_avrcp_cb;
static BT_AVRCP_EVENT_HANDLE_CB g_bt_mw_avrcp_event_handle_cb = NULL;

/* EXPORTED SUBPROGRAM BODIES
 */

INT32 bt_mw_avrcp_register_callback(BT_AVRCP_EVENT_HANDLE_CB avrcp_handle)
{
    if (NULL != avrcp_handle)
    {
        bt_mw_avrcp_init();
    }
    else
    {
        bt_mw_avrcp_deinit();
    }
    g_bt_mw_avrcp_event_handle_cb = avrcp_handle;
    return BT_SUCCESS;
}


INT32 bt_mw_avrcp_send_passthrough_cmd(CHAR *addr,
    BT_AVRCP_CMD_TYPE cmd_type, BT_AVRCP_KEY_STATE key_state)
{
    if (FALSE == bt_mw_avrcp_is_connected(addr))
    {
        return BT_ERR_STATUS_NOT_READY;
    }
    bt_mw_avrcp_print_key(cmd_type, key_state);

    if ((BT_AVRCP_CMD_TYPE_VOL_UP == cmd_type)
        || (BT_AVRCP_CMD_TYPE_VOL_DOWN == cmd_type))
    {
        return linuxbt_rc_tg_send_passthrough_cmd_handler(addr, cmd_type, key_state);
    }
    else
    {
        return linuxbt_rc_send_passthrough_cmd_handler(addr, cmd_type, key_state);
    }
    return BT_SUCCESS;

} /*bt_mw_avrcp_send_passthrough_cmd*/

INT32 bt_mw_avrcp_send_vendor_unique_cmd(CHAR *addr, UINT8 key,
    BT_AVRCP_KEY_STATE key_state)
{
    if (FALSE == bt_mw_avrcp_is_connected(addr))
    {
        return BT_ERR_STATUS_NOT_READY;
    }

    return linuxbt_rc_send_vendor_unique_cmd_handler(addr, key, key_state);
} /*bt_mw_avrcp_send_passthrough_cmd*/


INT32 bt_mw_avrcp_change_volume(CHAR *addr, UINT8 volume)
{
    INT32 idx = 0;

    if (FALSE == bt_mw_avrcp_is_connected(addr))
    {
        return BT_ERR_STATUS_NOT_READY;
    }

    BT_MW_AVRCP_LOCK();
    idx = bt_mw_avrcp_get_dev_index(addr);
    if (-1 == idx)
    {
        BT_MW_AVRCP_UNLOCK();
        return BT_ERR_STATUS_NOT_READY;
    }
    if (!(g_bt_mw_avrcp_cb.devices[idx].tg_feature & BTRC_FEAT_ABSOLUTE_VOLUME))
    {
        BT_DBG_WARNING(BT_DEBUG_AVRCP,"not support, feature=0x%x",
            g_bt_mw_avrcp_cb.devices[idx].tg_feature);
        BT_MW_AVRCP_UNLOCK();
        return BT_ERR_STATUS_UNSUPPORTED;
    }
    BT_MW_AVRCP_UNLOCK();

    return linuxbt_rc_tg_set_volume(addr, volume * 127 / 100);
}

INT32 bt_mw_avrcp_update_player_status(BT_AVRCP_PLAYER_STATUS *play_status)
{
    INT32 i = 0;
    BT_AVRCP_EVENT_CHANGE_RSP event_data = {0};
    BT_CHECK_POINTER(BT_DEBUG_AVRCP, play_status);

    BT_DBG_NORMAL(BT_DEBUG_AVRCP,"old pos=%d, len=%d, status=%d",
        g_bt_mw_avrcp_cb.player_status.song_pos,
        g_bt_mw_avrcp_cb.player_status.song_len,
        g_bt_mw_avrcp_cb.player_status.play_status);
    BT_DBG_NORMAL(BT_DEBUG_AVRCP,"new pos=%d, len=%d, status=%d",
        play_status->song_pos,
        play_status->song_len,
        play_status->play_status);

    for(i=0;i<BT_MW_AVRCP_MAX_DEVICES;i++)
    {
        if (TRUE == g_bt_mw_avrcp_cb.devices[i].in_use)
        {
            if ((g_bt_mw_avrcp_cb.devices[i].reg_event
                 & (1 << BT_AVRCP_REG_EVT_PLAY_STATUS_CHANGED))
                 && (play_status->play_status !=
                        g_bt_mw_avrcp_cb.player_status.play_status))
            {
                event_data.play_status_change.play_status =
                    play_status->play_status;
                bt_mw_avrcp_avrcp_event_change(g_bt_mw_avrcp_cb.devices[i].addr,
                    BT_AVRCP_REG_EVT_PLAY_STATUS_CHANGED,
                    FALSE, &event_data);
            }

            if ((g_bt_mw_avrcp_cb.devices[i].reg_event
             & (1 << BT_AVRCP_REG_EVT_PLAY_POS_CHANGED))
                 && (play_status->song_pos !=
                    g_bt_mw_avrcp_cb.player_status.song_pos))
            {
                event_data.pos_change.song_pos = play_status->song_pos;
                bt_mw_avrcp_avrcp_event_change(g_bt_mw_avrcp_cb.devices[i].addr,
                    BT_AVRCP_REG_EVT_PLAY_POS_CHANGED,
                    FALSE, &event_data);
            }
        }
    }
    memcpy(&g_bt_mw_avrcp_cb.player_status,
        play_status, sizeof(g_bt_mw_avrcp_cb.player_status));

    return BT_SUCCESS;
}


INT32 bt_mw_avrcp_update_player_media_info(BT_AVRCP_PLAYER_MEDIA_INFO *player_media_info)
{
    INT32 i = 0;
    BT_AVRCP_EVENT_CHANGE_RSP event_data = {0};
    BT_CHECK_POINTER(BT_DEBUG_AVRCP, player_media_info);

    BT_MW_FUNC_ENTER(BT_DEBUG_AVRCP, "title = %s track=0x%x 0x%x",
            player_media_info->media_info.title,
            player_media_info->track[0],
            player_media_info->track[BT_AVRCP_UID_SIZE-1]);

    if (0 == memcmp(&g_bt_mw_avrcp_cb.player_media_info,
        player_media_info, sizeof(BT_AVRCP_PLAYER_MEDIA_INFO)))
    {
        BT_DBG_WARNING(BT_DEBUG_AVRCP,"player media info no change");
        return BT_ERR_STATUS_UNHANDLED;
    }


    for(i=0;i<BT_MW_AVRCP_MAX_DEVICES;i++)
    {
        if (TRUE == g_bt_mw_avrcp_cb.devices[i].in_use)
        {
            if (g_bt_mw_avrcp_cb.devices[i].reg_event
                 & (1 << BT_AVRCP_REG_EVT_TRACK_CHANGED))
            {
                memcpy(&event_data.track_change.track_uid,
                    player_media_info->track,
                    sizeof(event_data.track_change.track_uid));

                bt_mw_avrcp_avrcp_event_change(g_bt_mw_avrcp_cb.devices[i].addr,
                    BT_AVRCP_REG_EVT_TRACK_CHANGED,
                    FALSE, &event_data);
            }
        }
    }
    memcpy(&g_bt_mw_avrcp_cb.player_media_info,
        player_media_info, sizeof(g_bt_mw_avrcp_cb.player_media_info));

    return BT_SUCCESS;
}

INT32 bt_mw_avrcp_update_absolute_volume(const UINT8 abs_volume)
{
    INT32 i = 0;

    if (g_bt_mw_avrcp_cb.abs_volume == abs_volume)
    {
        BT_DBG_WARNING(BT_DEBUG_AVRCP,"abs_volume no change");
        return BT_ERR_STATUS_UNHANDLED;
    }

    for(i=0;i<BT_MW_AVRCP_MAX_DEVICES;i++)
    {
        if (TRUE == g_bt_mw_avrcp_cb.devices[i].in_use)
        {
            if (g_bt_mw_avrcp_cb.devices[i].reg_event
                 & (1 << BT_AVRCP_REG_EVT_ABS_VOLUME_CHANGED))
            {
                bt_mw_avrcp_volume_change(g_bt_mw_avrcp_cb.devices[i].addr,
                    FALSE, abs_volume);
            }
        }
    }
    g_bt_mw_avrcp_cb.abs_volume = abs_volume;

    return BT_SUCCESS;
}


/* LOCAL SUBPROGRAM BODIES
 */


INT32 bt_mw_avrcp_init(VOID)
{
    BT_MW_FUNC_ENTER(BT_DEBUG_AVRCP, "");
    if (TRUE == g_bt_mw_avrcp_cb.inited)
    {
        BT_DBG_NORMAL(BT_DEBUG_AVRCP,"has inited");
        return BT_SUCCESS;
    }

    memset((void*)&g_bt_mw_avrcp_cb, 0, sizeof(g_bt_mw_avrcp_cb));


    pthread_mutex_init(&g_bt_mw_avrcp_cb.lock, NULL);

    linuxbt_hdl_register(BTWM_ID_AVRCP, bt_mw_avrcp_msg_handle);
    bt_mw_nty_hdl_register(BTWM_ID_AVRCP, bt_mw_avrcp_notify_handle);

    g_bt_mw_avrcp_cb.inited = TRUE;

    return BT_SUCCESS;
}

INT32 bt_mw_avrcp_deinit(VOID)
{
    BT_MW_FUNC_ENTER(BT_DEBUG_AVRCP, "");
    linuxbt_hdl_register(BTWM_ID_AVRCP, NULL);
    bt_mw_nty_hdl_register(BTWM_ID_AVRCP, NULL);
    if (FALSE == g_bt_mw_avrcp_cb.inited)
    {
        BT_DBG_NORMAL(BT_DEBUG_AVRCP,"has deinited");
        return BT_SUCCESS;
    }
    pthread_mutex_destroy(&g_bt_mw_avrcp_cb.lock);

    memset((void*)&g_bt_mw_avrcp_cb, 0, sizeof(g_bt_mw_avrcp_cb));

    return BT_SUCCESS;
}

static BOOL bt_mw_avrcp_is_connected(CHAR *addr)
{
    BOOL ret = FALSE;
    INT32 idx = 0;

    BT_MW_AVRCP_IS_VALID_ADDR(addr);
    BT_MW_AVRCP_CHECK_INITED(FALSE);

    BT_MW_AVRCP_LOCK();
    idx = bt_mw_avrcp_get_dev_index(addr);
    if (-1 == idx)
    {
        BT_DBG_ERROR(BT_DEBUG_AVRCP,"%s not connected", addr);
        BT_MW_AVRCP_UNLOCK();
        return ret;
    }
    if (BT_MW_AVRCP_CONNECT_STATUS_CONNECTED ==
        g_bt_mw_avrcp_cb.devices[idx].conn_status)
    {
        ret = TRUE;
    }
    BT_MW_AVRCP_UNLOCK();
    return ret;
}

static INT32 bt_mw_avrcp_get_dev_index(CHAR *addr)
{
    int i = 0;
    BT_MW_AVRCP_IS_VALID_ADDR(addr);

    for(i = 0;i < BT_MW_AVRCP_MAX_DEVICES;i++)
    {
        BT_DBG_INFO(BT_DEBUG_AVRCP, "device[%d] addr=%s, in_use=%d",
            i, g_bt_mw_avrcp_cb.devices[i].addr, g_bt_mw_avrcp_cb.devices[i].in_use);
        if (TRUE == g_bt_mw_avrcp_cb.devices[i].in_use
            && 0 == strcasecmp(g_bt_mw_avrcp_cb.devices[i].addr, addr))
        {
            BT_DBG_INFO(BT_DEBUG_AVRCP, "found at %d", i);
            return i;
        }
    }

    BT_DBG_WARNING(BT_DEBUG_AVRCP, "%s not found", addr);
    return -1;
}

static INT32 bt_mw_avrcp_alloc_dev_index(CHAR *addr)
{
    int i = 0;

    for(i = 0;i < BT_MW_AVRCP_MAX_DEVICES;i++)
    {
        BT_DBG_INFO(BT_DEBUG_AVRCP, "device[%d] addr=%s, in_use=%d",
            i, g_bt_mw_avrcp_cb.devices[i].addr, g_bt_mw_avrcp_cb.devices[i].in_use);
        if (FALSE == g_bt_mw_avrcp_cb.devices[i].in_use)
        {
            BT_DBG_INFO(BT_DEBUG_AVRCP, "found free at %d", i);
            g_bt_mw_avrcp_cb.devices[i].in_use = TRUE;
            strncpy(g_bt_mw_avrcp_cb.devices[i].addr, addr, MAX_BDADDR_LEN - 1);
            g_bt_mw_avrcp_cb.devices[i].addr[MAX_BDADDR_LEN - 1] = '\0';

            return i;
        }
    }

    BT_DBG_INFO(BT_DEBUG_AVRCP, "not free found");
    return -1;
}

static INT32 bt_mw_avrcp_free_dev(INT32 index)
{
    if (index >= BT_MW_AVRCP_MAX_DEVICES)
    {
        BT_DBG_ERROR(BT_DEBUG_AVRCP, "invalid index %d", index);
    }

    memset((void*)&g_bt_mw_avrcp_cb.devices[index], 0, sizeof(BT_MW_AVRCP_DEV));

    BT_DBG_INFO(BT_DEBUG_AVRCP, "not free found");
    return -1;
}


static VOID bt_mw_avrcp_handle_connect_cb(CHAR *addr, BT_MW_AVRCP_ROLE role)
{
    INT32 idx = 0;
    BT_AVRCP_EVENT_PARAM param = {0};

    BT_MW_AVRCP_LOCK();
    idx = bt_mw_avrcp_get_dev_index(addr);
    if (-1 != idx)
    {
        if (BT_MW_AVRCP_CONNECT_STATUS_CONNECTED ==
            g_bt_mw_avrcp_cb.devices[idx].conn_status)
        {
            BT_DBG_WARNING(BT_DEBUG_AVRCP, "%s has connected", addr);
            BT_MW_AVRCP_UNLOCK();
            return;
        }
    }
    else
    {
        idx = bt_mw_avrcp_alloc_dev_index(addr);
        if (-1 == idx)
        {
            BT_DBG_INFO(BT_DEBUG_AVRCP, "no device resource dev %s", addr);
            BT_MW_AVRCP_UNLOCK();
            return;
        }
        g_bt_mw_avrcp_cb.devices[idx].role = role;
    }
    g_bt_mw_avrcp_cb.devices[idx].conn_status =
            BT_MW_AVRCP_CONNECT_STATUS_CONNECTED;

    BT_MW_AVRCP_UNLOCK();

    strcpy(param.addr, addr);
    param.event = BT_AVRCP_EVENT_CONNECTED;
    BT_MW_AVRCP_REPORT_EVENT(param);
}

static VOID bt_mw_avrcp_handle_disconnect_cb(CHAR *addr, BT_MW_AVRCP_ROLE role)
{
    INT32 idx = 0;
    BT_AVRCP_EVENT_PARAM param = {0};

    BT_MW_AVRCP_LOCK();
    idx = bt_mw_avrcp_get_dev_index(addr);
    if (-1 == idx)
    {
        BT_MW_AVRCP_UNLOCK();
        BT_DBG_ERROR(BT_DEBUG_AVRCP, "no device  %s", addr);
        return;
    }

    bt_mw_avrcp_free_dev(idx);
    BT_MW_AVRCP_UNLOCK();

    strcpy(param.addr, addr);
    param.event = BT_AVRCP_EVENT_DISCONNECTED;

    BT_MW_AVRCP_REPORT_EVENT(param);
}

static VOID bt_mw_avrcp_handle_feature_cb(CHAR *addr, BT_MW_AVRCP_ROLE role, UINT32 feature)
{
    INT32 idx = 0;
    BT_MW_AVRCP_LOCK();
    idx = bt_mw_avrcp_get_dev_index(addr);
    if (-1 == idx)
    {
        BT_MW_AVRCP_UNLOCK();
        BT_DBG_ERROR(BT_DEBUG_AVRCP, "no device  %s", addr);
        return;
    }

    if (BT_MW_AVRCP_ROLE_CT == role)
    {
        g_bt_mw_avrcp_cb.devices[idx].ct_feature = feature;
    }
    else
    {
        g_bt_mw_avrcp_cb.devices[idx].tg_feature = feature;
    }

    BT_MW_AVRCP_UNLOCK();
}

static VOID bt_mw_avrcp_handle_set_volume_req(CHAR *addr, UINT8 label, UINT8 volume)
{
    BT_AVRCP_EVENT_PARAM param = {0};

    if (FALSE == bt_mw_avrcp_is_connected(addr))
    {
        return;
    }

    strcpy(param.addr, addr);
    param.event = BT_AVRCP_EVENT_SET_VOLUME_REQ;

    param.data.set_vol_req.abs_volume = volume * 100 / 127;
    BT_MW_AVRCP_REPORT_EVENT(param);

    linuxbt_rc_set_volume_rsp(addr, label, volume);
    return;
}

static VOID bt_mw_avrcp_reg_event_interim_rsp(CHAR *addr,
    UINT8 label, BT_MW_AVRCP_REG_EVENT_REQ *reg_event_req)
{
    BT_AVRCP_EVENT_CHANGE_RSP data;

    memset((void*)&data, 0, sizeof(data));

    switch (reg_event_req->event_id)
    {
        case BT_AVRCP_REG_EVT_TRACK_CHANGED:
            BT_DBG_NORMAL(BT_DEBUG_AVRCP, "title = %s track[0]=0x%x,track[0]=0x%x",
                g_bt_mw_avrcp_cb.player_media_info.media_info.title,
                g_bt_mw_avrcp_cb.player_media_info.track[0],
                g_bt_mw_avrcp_cb.player_media_info.track[BT_AVRCP_UID_SIZE-1]);
            memcpy(&data.track_change.track_uid,
                g_bt_mw_avrcp_cb.player_media_info.track,
                sizeof(data.track_change.track_uid));
            break;
        case BT_AVRCP_REG_EVT_PLAY_POS_CHANGED:
            data.pos_change.song_pos = g_bt_mw_avrcp_cb.player_status.song_pos;
            break;
        case BT_AVRCP_REG_EVT_PLAY_STATUS_CHANGED:
            data.play_status_change.play_status =
                g_bt_mw_avrcp_cb.player_status.play_status;
            break;
        case BT_AVRCP_REG_EVT_TRACK_REACHED_END:
            break;
        case BT_AVRCP_REG_EVT_TRACK_REACHED_START:
            break;
        case BT_AVRCP_REG_EVT_APP_SETTINGS_CHANGED:
            data.play_setting_change.setting = g_bt_mw_avrcp_cb.player_setting;
            break;
        case BT_AVRCP_REG_EVT_AVAL_PLAYERS_CHANGED:
            break;
        case BT_AVRCP_REG_EVT_ADDR_PLAYER_CHANGED:
            data.addr_player_change.player = g_bt_mw_avrcp_cb.addr_player;
            break;
        case BT_AVRCP_REG_EVT_ABS_VOLUME_CHANGED:
            bt_mw_avrcp_volume_change(addr, TRUE, g_bt_mw_avrcp_cb.abs_volume);
            return;
        default:
            return;
    }
    bt_mw_avrcp_avrcp_event_change(addr, reg_event_req->event_id, TRUE,
        &data);
    return;
}

static VOID bt_mw_avrcp_handle_reg_event_req(CHAR *addr,
    UINT8 label, BT_MW_AVRCP_REG_EVENT_REQ *reg_event)
{
    INT32 idx = 0;
    BT_MW_AVRCP_IS_VALID_ADDR_RETURN(addr);
    BT_CHECK_POINTER_RETURN(BT_DEBUG_AVRCP, reg_event);
    BT_MW_FUNC_ENTER(BT_DEBUG_AVRCP, "event=%s(%d)",
        bt_mw_avrcp_get_reg_event_str(reg_event->event_id),
        reg_event->event_id);

    BT_MW_AVRCP_LOCK();
    idx = bt_mw_avrcp_get_dev_index(addr);
    if (-1 == idx)
    {
        BT_MW_AVRCP_UNLOCK();
        BT_DBG_ERROR(BT_DEBUG_AVRCP, "no device  %s", addr);
        return;
    }
    if (BT_AVRCP_REG_EVT_ABS_VOLUME_CHANGED == reg_event->event_id)
    {
        g_bt_mw_avrcp_cb.devices[idx].abs_vol_lable = label;
    }

    g_bt_mw_avrcp_cb.devices[idx].reg_event |= (1 << reg_event->event_id);
    BT_MW_AVRCP_UNLOCK();
    bt_mw_avrcp_reg_event_interim_rsp(addr, label, reg_event);

    return;
}

static VOID bt_mw_avrcp_handle_track_change(CHAR *addr, BT_AVRCP_MEDIA_INFO *media_info)
{
    BT_AVRCP_EVENT_PARAM param = {0};

    if (FALSE == bt_mw_avrcp_is_connected(addr))
    {
        return;
    }
    strcpy(param.addr, addr);
    param.event = BT_AVRCP_EVENT_TRACK_CHANGE;

    memcpy(&param.data.track_change.element_attr,
        media_info, sizeof(BT_AVRCP_MEDIA_INFO));

    BT_MW_AVRCP_REPORT_EVENT(param);

    return;
}

static VOID bt_mw_avrcp_handle_play_position_change(CHAR *addr, BT_AVRCP_POS_CHANGE *pos_change)
{
    BT_AVRCP_EVENT_PARAM param = {0};
    if (FALSE == bt_mw_avrcp_is_connected(addr))
    {
        return;
    }
    strcpy(param.addr, addr);
    param.event = BT_AVRCP_EVENT_POS_CHANGE;

    memcpy(&param.data.pos_change, pos_change, sizeof(BT_AVRCP_POS_CHANGE));

    BT_MW_AVRCP_REPORT_EVENT(param);

    return;
}

static VOID bt_mw_avrcp_handle_play_status_change(CHAR *addr, BT_AVRCP_PLAY_STATUS status)
{
    BT_AVRCP_EVENT_PARAM param = {0};
    if (FALSE == bt_mw_avrcp_is_connected(addr))
    {
        return;
    }
    strcpy(param.addr, addr);
    param.event = BT_AVRCP_EVENT_PLAY_STATUS_CHANGE;
    param.data.play_status_change.play_status = status;

    BT_MW_AVRCP_REPORT_EVENT(param);

    return;
}

static VOID bt_mw_avrcp_handle_passthrough_cmd_req(CHAR *addr, BT_AVRCP_PASSTHROUGH_CMD_REQ *key)
{
    BT_AVRCP_EVENT_PARAM param = {0};
    param.event = BT_AVRCP_EVENT_PASSTHROUGH_CMD_REQ;
    strcpy(param.addr, addr);
    param.data.passthrough_cmd_req.cmd_type = key->cmd_type;
    param.data.passthrough_cmd_req.action = key->action;

    BT_MW_AVRCP_REPORT_EVENT(param);

    return;
}

static VOID bt_mw_avrcp_handle_get_play_status_req(CHAR *addr)
{
    BT_CHECK_POINTER_RETURN(BT_DEBUG_AVRCP, addr);
    BT_MW_AVRCP_LOCK();
    BT_DBG_NORMAL(BT_DEBUG_AVRCP,"player_status pos=%d, len=%d, status=%d",
            g_bt_mw_avrcp_cb.player_status.song_pos,
            g_bt_mw_avrcp_cb.player_status.song_len,
            g_bt_mw_avrcp_cb.player_status.play_status);

    linuxbt_rc_tg_get_play_status_rsp(addr,
        g_bt_mw_avrcp_cb.player_status.play_status,
        g_bt_mw_avrcp_cb.player_status.song_len,
        g_bt_mw_avrcp_cb.player_status.song_pos);

    BT_MW_AVRCP_UNLOCK();
    return;
}


static VOID bt_mw_avrcp_handle_get_element_attr_req(CHAR *addr,
    BT_MW_AVRCP_ELEMENT_ATTR_REQ *req)
{
    BT_CHECK_POINTER_RETURN(BT_DEBUG_AVRCP, addr);
    BT_CHECK_POINTER_RETURN(BT_DEBUG_AVRCP, req);

    BT_MW_AVRCP_LOCK();

    linuxbt_rc_tg_get_element_attr_rsp(addr, req,
        &g_bt_mw_avrcp_cb.player_media_info.media_info);

    BT_MW_AVRCP_UNLOCK();

    return;
}

static VOID bt_mw_avrcp_handle_volume_change_cb(CHAR *addr, UINT8 volume)
{
    BT_AVRCP_EVENT_PARAM param = {0};
    param.event = BT_AVRCP_EVENT_VOLUME_CHANGE;
    strcpy(param.addr, addr);

    param.data.volume_change.abs_volume = volume * 100 / 127;

    BT_MW_AVRCP_REPORT_EVENT(param);

    return;
}

static INT32 bt_mw_avrcp_avrcp_event_change(CHAR *addr,
    BT_AVRCP_REG_EVT_ID event, BOOL interim, BT_AVRCP_EVENT_CHANGE_RSP *data)
{
    if (FALSE == bt_mw_avrcp_is_connected(addr))
    {
        return BT_ERR_STATUS_NOT_READY;
    }
    return linuxbt_rc_tg_event_change(addr, event, interim, data);
}


static INT32 bt_mw_avrcp_volume_change(CHAR *addr, BOOL interim, UINT8 volume)
{
    INT32 idx = 0;
    INT8 lable = 0;
    UINT8 abs_volume = 0;

    if (FALSE == bt_mw_avrcp_is_connected(addr))
    {
        return BT_ERR_STATUS_NOT_READY;
    }
    BT_MW_FUNC_ENTER(BT_DEBUG_AVRCP, "addr=%s, interim=%d, volume=%d",
            addr, interim, volume);
    BT_MW_AVRCP_LOCK();
    idx = bt_mw_avrcp_get_dev_index(addr);
    if (-1 == idx)
    {
        BT_MW_AVRCP_UNLOCK();
        BT_DBG_ERROR(BT_DEBUG_AVRCP, "no device  %s", addr);
        return BT_ERR_STATUS_NOT_READY;
    }
    BT_MW_AVRCP_UNLOCK();
    lable = g_bt_mw_avrcp_cb.devices[idx].abs_vol_lable;
    abs_volume = volume * 127 / 100;
    return linuxbt_rc_send_volume_change_rsp_handler(addr, interim, lable, abs_volume);
}

static VOID bt_mw_avrcp_msg_handle(tBTMW_MSG *p_msg)
{
    tBT_MW_AVRCP_MSG *avrcp_msg = &p_msg->data.avrcp_msg;
    BT_DBG_NORMAL(BT_DEBUG_AVRCP, "event:%s(%d), addr:%s",
        bt_mw_avrcp_get_event_str(p_msg->hdr.event),
        p_msg->hdr.event, avrcp_msg->addr);
    BT_MW_AVRCP_CHECK_INITED();

    switch(p_msg->hdr.event)
    {
        case BTMW_AVRCP_CONNECTED:
            bt_mw_avrcp_handle_connect_cb(avrcp_msg->addr, avrcp_msg->role);
            break;
        case BTMW_AVRCP_DISCONNECTED:
            bt_mw_avrcp_handle_disconnect_cb(avrcp_msg->addr, avrcp_msg->role);
            break;
        case BTMW_AVRCP_FEATURE:
            bt_mw_avrcp_handle_feature_cb(avrcp_msg->addr,
                avrcp_msg->role, avrcp_msg->data.feature);
            break;
        case BTMW_AVRCP_SET_VOLUME_REQ:
            bt_mw_avrcp_handle_set_volume_req(avrcp_msg->addr,
                avrcp_msg->label, avrcp_msg->data.set_vol_req.abs_volume);
            break;
        case BTMW_AVRCP_REG_EVENT_REQ:
            bt_mw_avrcp_handle_reg_event_req(avrcp_msg->addr,
                avrcp_msg->label, &avrcp_msg->data.reg_event_req);
            break;
        case BTMW_AVRCP_TRACK_CHANGE:
            bt_mw_avrcp_handle_track_change(avrcp_msg->addr,
                &avrcp_msg->data.track_change.element_attr);
            break;
        case BTMW_AVRCP_PLAY_POS_CHANGE:
            bt_mw_avrcp_handle_play_position_change(avrcp_msg->addr,
                &avrcp_msg->data.pos_change);
            break;
        case BTMW_AVRCP_PLAY_STATUS_CHANGE:
            bt_mw_avrcp_handle_play_status_change(avrcp_msg->addr,
                avrcp_msg->data.play_status_change.play_status);
            break;
        case BTMW_AVRCP_PASSTHROUGH_CMD_REQ:
            bt_mw_avrcp_handle_passthrough_cmd_req(avrcp_msg->addr,
                &avrcp_msg->data.passthrough_cmd_req);
            break;
        case BTMW_AVRCP_GET_PLAYSTATUS_REQ:
            bt_mw_avrcp_handle_get_play_status_req(avrcp_msg->addr);
            break;
        case BTMW_AVRCP_GET_ELEMENT_ATTR_REQ:
            bt_mw_avrcp_handle_get_element_attr_req(avrcp_msg->addr,
                &avrcp_msg->data.media_attr_req);
            break;
        case BTMW_AVRCP_VOLUME_CHANGE:
            bt_mw_avrcp_handle_volume_change_cb(avrcp_msg->addr,
                avrcp_msg->data.volume_change.abs_volume);
            break;
        default:
            break;
    }
}

static VOID bt_mw_avrcp_notify_handle(tBTMW_MSG *p_msg)
{
    BT_AVRCP_EVENT_PARAM *avrcp_event = &p_msg->data.avrcp_event;
    BT_DBG_INFO(BT_DEBUG_AVRCP, "event:%d, addr:%s", p_msg->hdr.event);
    BT_MW_AVRCP_CHECK_INITED_RETURN;

    switch(p_msg->hdr.event)
    {
        case BTMW_AVRCP_NOTIFY_APP:
            if(g_bt_mw_avrcp_event_handle_cb)
            {
                BT_DBG_NORMAL(BT_DEBUG_AVRCP, "report event: %s(%d)",
                    bt_mw_avrcp_get_app_event_str(avrcp_event->event),
                    avrcp_event->event);
                g_bt_mw_avrcp_event_handle_cb(avrcp_event);
            }
            break;
        default:
            break;
    }
}

static VOID bt_mw_avrcp_notify_app(BT_AVRCP_EVENT_PARAM *param)
{
    tBTMW_MSG msg;
    msg.hdr.event = BTMW_AVRCP_NOTIFY_APP;
    msg.hdr.len = sizeof(*param);
    memcpy((void*)&msg.data.avrcp_event, param, sizeof(*param));
    bt_mw_nty_send_msg(&msg);

}

static VOID bt_mw_avrcp_print_key(BT_AVRCP_CMD_TYPE cmd_type, BT_AVRCP_KEY_STATE key_state)
{
    char *key_name[BT_AVRCP_CMD_TYPE_MAX] = {"play", "pause", "next", "previous",
        "fast forward", "reward", "stop", "volume up", "volume down"};
    BT_DBG_NORMAL(BT_DEBUG_AVRCP, "cmd_type: %s, action: %s", key_name[cmd_type],
        key_state==BT_AVRCP_KEY_STATE_PRESS?"press":
        (key_state==BT_AVRCP_KEY_STATE_RELEASE?"release":
        (key_state==BT_AVRCP_KEY_STATE_AUTO?"auto":"invalid")));
}

static CHAR* bt_mw_avrcp_get_reg_event_str(BT_AVRCP_REG_EVT_ID event)
{
    switch((int)event)
    {
        BT_MW_AVRCP_CASE_RETURN_STR(BT_AVRCP_REG_EVT_TRACK_CHANGED)
        BT_MW_AVRCP_CASE_RETURN_STR(BT_AVRCP_REG_EVT_PLAY_POS_CHANGED)
        BT_MW_AVRCP_CASE_RETURN_STR(BT_AVRCP_REG_EVT_PLAY_STATUS_CHANGED)
        BT_MW_AVRCP_CASE_RETURN_STR(BT_AVRCP_REG_EVT_TRACK_REACHED_END)
        BT_MW_AVRCP_CASE_RETURN_STR(BT_AVRCP_REG_EVT_TRACK_REACHED_START)
        BT_MW_AVRCP_CASE_RETURN_STR(BT_AVRCP_REG_EVT_APP_SETTINGS_CHANGED)
        BT_MW_AVRCP_CASE_RETURN_STR(BT_AVRCP_REG_EVT_AVAL_PLAYERS_CHANGED)
        BT_MW_AVRCP_CASE_RETURN_STR(BT_AVRCP_REG_EVT_ADDR_PLAYER_CHANGED)
        BT_MW_AVRCP_CASE_RETURN_STR(BT_AVRCP_REG_EVT_ABS_VOLUME_CHANGED)
        default: return "UNKNOWN_AVRCP_REG_EVENT";
   }
}

static CHAR* bt_mw_avrcp_get_event_str(UINT32 event)
{
    switch((int)event)
    {
        BT_MW_AVRCP_CASE_RETURN_STR(BTMW_AVRCP_CONNECTED)
        BT_MW_AVRCP_CASE_RETURN_STR(BTMW_AVRCP_DISCONNECTED)
        BT_MW_AVRCP_CASE_RETURN_STR(BTMW_AVRCP_FEATURE)
        BT_MW_AVRCP_CASE_RETURN_STR(BTMW_AVRCP_REG_EVENT_RSP)
        BT_MW_AVRCP_CASE_RETURN_STR(BTMW_AVRCP_TRACK_CHANGE)
        BT_MW_AVRCP_CASE_RETURN_STR(BTMW_AVRCP_PLAY_POS_CHANGE)
        BT_MW_AVRCP_CASE_RETURN_STR(BTMW_AVRCP_PLAY_STATUS_CHANGE)
        BT_MW_AVRCP_CASE_RETURN_STR(BTMW_AVRCP_VOLUME_CHANGE)
        BT_MW_AVRCP_CASE_RETURN_STR(BTMW_AVRCP_SET_VOLUME_REQ)

        BT_MW_AVRCP_CASE_RETURN_STR(BTMW_AVRCP_REG_EVENT_REQ)
        BT_MW_AVRCP_CASE_RETURN_STR(BTMW_AVRCP_GET_PLAYSTATUS_REQ)
        BT_MW_AVRCP_CASE_RETURN_STR(BTMW_AVRCP_GET_ELEMENT_ATTR_REQ)
        BT_MW_AVRCP_CASE_RETURN_STR(BTMW_AVRCP_PASSTHROUGH_CMD_REQ)
        default: return "UNKNOWN_EVENT";
   }
}

static CHAR* bt_mw_avrcp_get_app_event_str(UINT32 event)
{
    switch((int)event)
    {
        BT_MW_AVRCP_CASE_RETURN_STR(BT_AVRCP_EVENT_CONNECTED)
        BT_MW_AVRCP_CASE_RETURN_STR(BT_AVRCP_EVENT_DISCONNECTED)
        BT_MW_AVRCP_CASE_RETURN_STR(BT_AVRCP_EVENT_TRACK_CHANGE)
        BT_MW_AVRCP_CASE_RETURN_STR(BT_AVRCP_EVENT_POS_CHANGE)
        BT_MW_AVRCP_CASE_RETURN_STR(BT_AVRCP_EVENT_PLAY_STATUS_CHANGE)
        BT_MW_AVRCP_CASE_RETURN_STR(BT_AVRCP_EVENT_VOLUME_CHANGE)
        BT_MW_AVRCP_CASE_RETURN_STR(BT_AVRCP_EVENT_SET_VOLUME_REQ)
        BT_MW_AVRCP_CASE_RETURN_STR(BT_AVRCP_EVENT_PASSTHROUGH_CMD_REQ)
        default: return "UNKNOWN_EVENT";
   }
}

