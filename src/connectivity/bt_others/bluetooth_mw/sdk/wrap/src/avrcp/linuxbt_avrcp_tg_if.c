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

/* FILE NAME:  linuxbt_avrcp_tg_if.c
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
#include <unistd.h>
#include "bluetooth.h"

#include "bt_mw_common.h"
#include "linuxbt_avrcp_tg_if.h"
#include "bt_base_types.h"
#include "mtk_bluetooth.h"

#include "linuxbt_common.h"
#include "linuxbt_gap_if.h"
#include "bt_mw_message_queue.h"

#if defined(MTK_LINUX_AVRCP_PLUS) && (MTK_LINUX_AVRCP_PLUS == TRUE)
#include "mtk_bt_rc.h"
#else
#include "bt_rc.h"
#endif


/* NAMING CONSTANT DECLARATIONS
 */
/* MACRO FUNCTION DECLARATIONS
 */

#define LINUXBT_AVRCP_TG_SET_MSG_LEN(msg) do{       \
    msg.hdr.len = sizeof(tBT_MW_AVRCP_MSG);         \
    }while(0)

/* DATA TYPE DECLARATIONS
 */
/* GLOBAL VARIABLE DECLARATIONS
 */
CHAR g_bt_avrcp_addr[MAX_BDADDR_LEN];
/* LOCAL SUBPROGRAM DECLARATIONS
 */

static void linuxbt_rc_tg_remote_features_cb (bt_bdaddr_t *bd_addr,
    btrc_remote_features_t features);

static void linuxbt_rc_tg_get_play_status_cb(void);

static void linuxbt_rc_tg_get_element_attr_cb (uint8_t num_attr,
    btrc_media_attr_t *p_attrs);

static void linuxbt_rc_tg_register_notification_cb (btrc_event_id_t event_id,
    uint32_t param);

static void linuxbt_rc_tg_volume_change_cb (uint8_t volume, uint8_t ctype);

static void linuxbt_rc_tg_passthrough_cmd_cb (int id, int key_state);

static void linuxbt_rc_rg_connection_state_cb(bool state, bt_bdaddr_t *bd_addr);

static void linuxbt_rc_tg_passthrough_cmd_rsp_cb(int id, int key_state);


/* STATIC VARIABLE DECLARATIONS
 */
static btrc_interface_t *g_bt_rc_tg_interface = NULL;
#if defined(MTK_LINUX_AVRCP_PLUS) && (MTK_LINUX_AVRCP_PLUS == TRUE)
static const btrc_ex_interface_t *g_bt_avrcp_tg_ex_interface = NULL;
#endif

static btrc_callbacks_t g_bt_rc_tg_callbacks =
{
    sizeof(btrc_callbacks_t),
    linuxbt_rc_tg_remote_features_cb,
    linuxbt_rc_tg_get_play_status_cb,
    NULL,//linuxbt_rc_tg_list_player_app_attr_cb,
    NULL,//linuxbt_rc_tg_list_player_app_values_cb,
    NULL,//linuxbt_rc_tg_get_player_app_value_cb,
    NULL,//linuxbt_rc_tg_get_player_app_attrs_text_cb,
    NULL,//linuxbt_rc_tg_get_player_app_values_text_cb,
    NULL,//linuxbt_rc_tg_set_player_app_value_cb,
    linuxbt_rc_tg_get_element_attr_cb,
    linuxbt_rc_tg_register_notification_cb,
    linuxbt_rc_tg_volume_change_cb,
    linuxbt_rc_tg_passthrough_cmd_cb,
    NULL,
    NULL,
    NULL,
};

#if defined(MTK_LINUX_AVRCP_PLUS) && (MTK_LINUX_AVRCP_PLUS == TRUE)
static btrc_ext_callbacks_t g_bt_rc_tg_ext_callbacks =
{
    sizeof(btrc_ext_callbacks_t),
    linuxbt_rc_rg_connection_state_cb,
    linuxbt_rc_tg_passthrough_cmd_rsp_cb,
};
#endif

/* EXPORTED SUBPROGRAM BODIES
 */

int linuxbt_rc_tg_init(void)
{
    int ret = 0;

    BT_MW_FUNC_ENTER(BT_DEBUG_AVRCP, "");


#if defined(MTK_LINUX_AVRCP_PLUS) && (MTK_LINUX_AVRCP_PLUS == TRUE)
    g_bt_avrcp_tg_ex_interface = (btrc_ex_interface_t *) linuxbt_gap_get_profile_interface(BT_PROFILE_AVRCP_TG_EX_ID);
    if (NULL == g_bt_avrcp_tg_ex_interface)
    {
        BT_DBG_WARNING(BT_DEBUG_AVRCP, "Failed to get AVRCP TG ext interface");
        return -1;
    }
    // Init AVRCP ext interface
    ret = g_bt_avrcp_tg_ex_interface->init(&g_bt_rc_tg_ext_callbacks);
    if (ret == BT_STATUS_SUCCESS)
    {
        BT_DBG_WARNING(BT_DEBUG_AVRCP, " success to init AVRCP TG ext interface");
    }
    else if(ret == BT_STATUS_DONE)
    {
        BT_DBG_WARNING(BT_DEBUG_AVRCP, " already init AVRCP TG ext interface");
    }
#endif

    g_bt_rc_tg_interface = (btrc_interface_t *) linuxbt_gap_get_profile_interface(BT_PROFILE_AV_RC_ID);

    if (g_bt_rc_tg_interface == NULL)
    {
        BT_DBG_ERROR(BT_DEBUG_AVRCP,"[AVRCP] Failed to get AVRCP TG interface");
        return -1;
    }

    // Init AVRCP interface
    ret = g_bt_rc_tg_interface->init(&g_bt_rc_tg_callbacks);
    if (ret == BT_STATUS_SUCCESS)
    {
        BT_DBG_NOTICE(BT_DEBUG_AVRCP,"[AVRCP] success to init AVRCP TG interface");
        return 0;
    }
    else if(ret == BT_STATUS_DONE)
    {
        BT_DBG_NOTICE(BT_DEBUG_AVRCP,"[AVRCP] already init AVRCP TG interface");
        return 0;
    }
    BT_DBG_ERROR(BT_DEBUG_AVRCP,"[AVRCP] Failed to init AVRCP TG interface");

    return ret;
}

int linuxbt_rc_tg_deinit(void)
{
    BT_MW_FUNC_ENTER(BT_DEBUG_AVRCP, "");

    if (g_bt_rc_tg_interface == NULL)
    {
        BT_DBG_ERROR(BT_DEBUG_AVRCP,"[AVRCP] Failed to get AVRCP TG interface");
        return -1;
    }

    g_bt_rc_tg_interface->cleanup();
    g_bt_rc_tg_interface = NULL;

    return 0;
}


int linuxbt_rc_tg_set_volume(char *addr, UINT8 volume)
{
    if (g_bt_rc_tg_interface == NULL)
    {
        BT_DBG_ERROR(BT_DEBUG_AVRCP," Failed to get AVRCP TG interface");
        return -1;
    }
    BT_MW_FUNC_ENTER(BT_DEBUG_AVRCP, "addr = %s volume = %d ", addr, volume);
    g_bt_rc_tg_interface->set_volume(volume);

    return 0;
}


#if defined(MTK_LINUX_AVRCP_PLUS) && (MTK_LINUX_AVRCP_PLUS == TRUE)
INT32 linuxbt_rc_tg_send_passthrough_cmd_handler(CHAR *addr,
    BT_AVRCP_CMD_TYPE cmd_type, BT_AVRCP_KEY_STATE key_state)
{
    bt_bdaddr_t bdaddr;
    INT32 key_map[BT_AVRCP_CMD_TYPE_MAX] = {
        AVRC_ID_PLAY,
        AVRC_ID_PAUSE,
        AVRC_ID_FORWARD,
        AVRC_ID_BACKWARD,
        AVRC_ID_FAST_FOR,
        AVRC_ID_REWIND,
        AVRC_ID_STOP,
        AVRC_ID_VOL_UP,
        AVRC_ID_VOL_DOWN };

    BT_MW_FUNC_ENTER(BT_DEBUG_AVRCP, "%s, cmd=%d, action=%d",
        addr, cmd_type, key_state);

    if (BT_AVRCP_CMD_TYPE_MAX <= cmd_type)
    {
        BT_DBG_ERROR(BT_DEBUG_AVRCP,  "cmd_type=%d", cmd_type);
        return -1;
    }
    if (BT_AVRCP_CMD_TYPE_VOL_UP != cmd_type &&
        BT_AVRCP_CMD_TYPE_VOL_DOWN != cmd_type)
    {
        BT_DBG_ERROR(BT_DEBUG_AVRCP,  "cmd_type=%d", cmd_type);
        return -1;
    }
    BT_CHECK_POINTER(BT_DEBUG_AVRCP, addr);

    linuxbt_btaddr_stoh(addr, &bdaddr);
    if (g_bt_avrcp_tg_ex_interface == NULL)
    {
        BT_DBG_ERROR(BT_DEBUG_AVRCP,  " Failed to get AVRCP TG Ex interface");
        return -1;
    }

    if (BT_AVRCP_KEY_STATE_PRESS == key_state)
    {
        g_bt_avrcp_tg_ex_interface->send_pass_through_cmd(&bdaddr,
            key_map[cmd_type], AVRC_STATE_PRESS);
    }
    else if (BT_AVRCP_KEY_STATE_RELEASE == key_state)
    {
        g_bt_avrcp_tg_ex_interface->send_pass_through_cmd(&bdaddr,
            key_map[cmd_type], AVRC_STATE_RELEASE);
    }
    else
    {
        g_bt_avrcp_tg_ex_interface->send_pass_through_cmd(&bdaddr,
            key_map[cmd_type], AVRC_STATE_PRESS);
        usleep(200*1000);
        g_bt_avrcp_tg_ex_interface->send_pass_through_cmd(&bdaddr,
            key_map[cmd_type], AVRC_STATE_RELEASE);
    }

    return BT_SUCCESS;
}

#endif




int linuxbt_rc_tg_get_play_status_rsp(char *addr,
    BT_AVRCP_PLAY_STATUS play_status, UINT32 song_len, UINT32 song_pos)
{
    if (g_bt_rc_tg_interface == NULL)
    {
        BT_DBG_ERROR(BT_DEBUG_AVRCP," Failed to get AVRCP TG interface");
        return -1;
    }
    BT_MW_FUNC_ENTER(BT_DEBUG_AVRCP, "addr = %s play_status = %d, song_len=%d, song_pos=%d ",
        addr, play_status, song_len, song_pos);
    g_bt_rc_tg_interface->get_play_status_rsp(play_status, song_len, song_pos);

    return 0;
}

int linuxbt_rc_tg_get_element_attr_rsp(char *addr,
    BT_MW_AVRCP_ELEMENT_ATTR_REQ *req, BT_AVRCP_MEDIA_INFO *info)
{
    UINT8 i = 0;
    btrc_element_attr_val_t element_attr[BT_AVRCP_MEDIA_ATTR_PLAYING_TIME];

    BT_CHECK_POINTER(BT_DEBUG_AVRCP, addr);
    BT_CHECK_POINTER(BT_DEBUG_AVRCP, req);
    BT_CHECK_POINTER(BT_DEBUG_AVRCP, info);

    if (g_bt_rc_tg_interface == NULL)
    {
        BT_DBG_ERROR(BT_DEBUG_AVRCP," Failed to get AVRCP TG interface");
        return -1;
    }
    for (i=0;i<req->attr_cnt;i++)
    {
        element_attr[i].attr_id = req->attrs[i];
        switch (req->attrs[i])
        {
            case BT_AVRCP_MEDIA_ATTR_TITLE:
                strncpy((char*)&element_attr[i].text, info->title, BTRC_MAX_ATTR_STR_LEN);
                break;
            case BT_AVRCP_MEDIA_ATTR_ARTIST:
                strncpy((char*)&element_attr[i].text, info->artist, BTRC_MAX_ATTR_STR_LEN);
                break;
            case BT_AVRCP_MEDIA_ATTR_ALBUM:
                strncpy((char*)&element_attr[i].text, info->album, BTRC_MAX_ATTR_STR_LEN);
                break;
            case BT_AVRCP_MEDIA_ATTR_TRACK_NUM:
                snprintf((char*)&element_attr[i].text, BTRC_MAX_ATTR_STR_LEN, "%d",
                    info->current_track_number);
                break;
            case BT_AVRCP_MEDIA_ATTR_NUM_TRACKS:
                snprintf((char*)&element_attr[i].text, BTRC_MAX_ATTR_STR_LEN, "%d",
                    info->number_of_tracks);
                break;
            case BT_AVRCP_MEDIA_ATTR_GENRE:
                strncpy((char*)&element_attr[i].text, info->genre, BTRC_MAX_ATTR_STR_LEN);
                break;
            case BT_AVRCP_MEDIA_ATTR_PLAYING_TIME:
                snprintf((char*)&element_attr[i].text, BTRC_MAX_ATTR_STR_LEN, "%d",
                    info->position);
                break;
            default:
                break;
        }
    }


    g_bt_rc_tg_interface->get_element_attr_rsp(req->attr_cnt, &element_attr[0]);

    return 0;
}


int linuxbt_rc_tg_event_change(CHAR *addr,
    BT_AVRCP_REG_EVT_ID event, BOOL interim, BT_AVRCP_EVENT_CHANGE_RSP *data)
{
    int event_map[BT_AVRCP_REG_EVT_MAX-1] = {
        BTRC_EVT_TRACK_CHANGE,
        BTRC_EVT_PLAY_POS_CHANGED,
        BTRC_EVT_PLAY_STATUS_CHANGED,
        BTRC_EVT_TRACK_REACHED_END,
        BTRC_EVT_TRACK_REACHED_START,
        BTRC_EVT_APP_SETTINGS_CHANGED,
        BTRC_EVT_AVAL_PLAYERS_CHANGE,
        BTRC_EVT_ADDR_PLAYER_CHANGE };
    btrc_register_notification_t param;
    if (BT_AVRCP_REG_EVT_MAX-1 <= event)
    {
        BT_DBG_ERROR(BT_DEBUG_AVRCP," event(%d) invalid", event);
        return -1;
    }
    BT_MW_FUNC_ENTER(BT_DEBUG_AVRCP, "addr = %s event=%d, interim=%d ",
        addr, event, interim);

    switch (event_map[event])
    {
        case BTRC_EVT_TRACK_CHANGE:
            memcpy(&param.track, &data->track_change.track_uid, sizeof(param.track));
            break;
        case BTRC_EVT_PLAY_POS_CHANGED:
            param.song_pos = data->pos_change.song_pos;
            break;
        case BTRC_EVT_PLAY_STATUS_CHANGED:
            param.play_status = data->play_status_change.play_status;
            break;
        case BTRC_EVT_TRACK_REACHED_END:
            break;
        case BTRC_EVT_TRACK_REACHED_START:
            break;
        case BTRC_EVT_APP_SETTINGS_CHANGED:
            memcpy(&param.player_setting, &data->play_setting_change.setting,
                sizeof(param.player_setting));
            break;
        case BTRC_EVT_AVAL_PLAYERS_CHANGE:
            break;
        case BTRC_EVT_ADDR_PLAYER_CHANGE:
            memcpy(&param.addr_player, &data->addr_player_change.player,
                sizeof(param.addr_player));
            break;
        default:
            break;
    }

    if (interim)
    {
        g_bt_rc_tg_interface->register_notification_rsp(event_map[event],
            BTRC_NOTIFICATION_TYPE_INTERIM, &param);
    }
    else
    {
        g_bt_rc_tg_interface->register_notification_rsp(event_map[event],
            BTRC_NOTIFICATION_TYPE_CHANGED, &param);
    }
    return 0;
}


/* LOCAL SUBPROGRAM BODIES
 */

static void linuxbt_rc_tg_remote_features_cb (bt_bdaddr_t *bd_addr,
    btrc_remote_features_t features)
{
    tBTMW_MSG btmw_msg = {0};

    BT_CHECK_POINTER_RETURN(BT_DEBUG_AVRCP, bd_addr);

    linuxbt_btaddr_htos(bd_addr, btmw_msg.data.avrcp_msg.addr);

    BT_MW_FUNC_ENTER(BT_DEBUG_AVRCP, "[AVRCP]%s TG features = 0x%x ",
        btmw_msg.data.avrcp_msg.addr, features);
    btmw_msg.data.avrcp_msg.role = BT_MW_AVRCP_ROLE_TG;
    btmw_msg.data.avrcp_msg.data.feature = features;

    btmw_msg.hdr.event = BTMW_AVRCP_FEATURE;
    LINUXBT_AVRCP_TG_SET_MSG_LEN(btmw_msg);

    linuxbt_send_msg(&btmw_msg);
}

static void linuxbt_rc_tg_get_play_status_cb(void)
{
    tBTMW_MSG btmw_msg = {0};

    BT_MW_FUNC_ENTER(BT_DEBUG_AVRCP, "");

    strcpy(btmw_msg.data.avrcp_msg.addr, g_bt_avrcp_addr);

    btmw_msg.data.avrcp_msg.role = BT_MW_AVRCP_ROLE_TG;

    btmw_msg.hdr.event = BTMW_AVRCP_GET_PLAYSTATUS_REQ;
    LINUXBT_AVRCP_TG_SET_MSG_LEN(btmw_msg);

    linuxbt_send_msg(&btmw_msg);
}

static void linuxbt_rc_tg_get_element_attr_cb (uint8_t num_attr,
    btrc_media_attr_t *p_attrs)
{
    tBTMW_MSG btmw_msg = {0};
    int i = 0;

    BT_MW_FUNC_ENTER(BT_DEBUG_AVRCP, "num_attr = %x, p_attrs = %x",
        num_attr, *p_attrs);

    btmw_msg.data.avrcp_msg.role = BT_MW_AVRCP_ROLE_TG;
    strcpy(btmw_msg.data.avrcp_msg.addr, g_bt_avrcp_addr);

    btmw_msg.hdr.event = BTMW_AVRCP_GET_ELEMENT_ATTR_REQ;
    btmw_msg.data.avrcp_msg.data.media_attr_req.attr_cnt =
        num_attr > BT_AVRCP_MAX_ATTR_CNT ? BT_AVRCP_MAX_ATTR_CNT : num_attr;

    for (i = 0;i < btmw_msg.data.avrcp_msg.data.media_attr_req.attr_cnt; i++)
    {
        btmw_msg.data.avrcp_msg.data.media_attr_req.attrs[i] = p_attrs[i];
    }
    LINUXBT_AVRCP_TG_SET_MSG_LEN(btmw_msg);

    linuxbt_send_msg(&btmw_msg);


    return;
}

static void linuxbt_rc_tg_register_notification_cb (btrc_event_id_t event_id,
    uint32_t param)
{
    tBTMW_MSG btmw_msg = {0};
    INT32 i = 0;
    BT_AVRCP_REG_EVT_ID mw_event_id = BT_AVRCP_REG_EVT_MAX;
    btrc_event_id_t event_map[BT_AVRCP_REG_EVT_MAX-1] = {
        BTRC_EVT_TRACK_CHANGE,
        BTRC_EVT_PLAY_POS_CHANGED,
        BTRC_EVT_PLAY_STATUS_CHANGED,
        BTRC_EVT_TRACK_REACHED_END,
        BTRC_EVT_TRACK_REACHED_START,
        BTRC_EVT_APP_SETTINGS_CHANGED,
        BTRC_EVT_AVAL_PLAYERS_CHANGE,
        BTRC_EVT_ADDR_PLAYER_CHANGE };

    BT_MW_FUNC_ENTER(BT_DEBUG_AVRCP,"event_id = %d param = %d",
        event_id, param);

    btmw_msg.hdr.event = BTMW_AVRCP_REG_EVENT_REQ;

    btmw_msg.data.avrcp_msg.role = BT_MW_AVRCP_ROLE_TG;
    for (i=0;i<BT_AVRCP_REG_EVT_MAX-1;i++)
    {
        if (event_map[i] == event_id)
        {
            mw_event_id = i;
        }
    }
    if (BT_AVRCP_REG_EVT_MAX == mw_event_id)
    {
        BT_DBG_ERROR(BT_DEBUG_AVRCP,"invalid event(%d)", event_id);
        return;
    }
    strcpy(btmw_msg.data.avrcp_msg.addr, g_bt_avrcp_addr);
    btmw_msg.data.avrcp_msg.data.reg_event_req.event_id = mw_event_id;
    btmw_msg.data.avrcp_msg.data.reg_event_req.param = param;
    LINUXBT_AVRCP_TG_SET_MSG_LEN(btmw_msg);

    linuxbt_send_msg(&btmw_msg);
}

static void linuxbt_rc_tg_volume_change_cb (uint8_t volume, uint8_t ctype)
{
    tBTMW_MSG btmw_msg = {0};
    BT_MW_FUNC_ENTER(BT_DEBUG_AVRCP, "volume = %x ctype = %x",
            volume, ctype);

    btmw_msg.data.avrcp_msg.role = BT_MW_AVRCP_ROLE_TG;
    strcpy(btmw_msg.data.avrcp_msg.addr, g_bt_avrcp_addr);

    btmw_msg.hdr.event = BTMW_AVRCP_VOLUME_CHANGE;
    btmw_msg.data.avrcp_msg.data.volume_change.abs_volume = volume * 100 / 127;
    LINUXBT_AVRCP_TG_SET_MSG_LEN(btmw_msg);

    linuxbt_send_msg(&btmw_msg);
}


static void linuxbt_rc_tg_passthrough_cmd_cb (int id, int is_press)
{
    tBTMW_MSG btmw_msg = {0};

    BT_AVRCP_CMD_TYPE bt_cmd_type = BT_AVRCP_CMD_TYPE_MAX;
    int i = 0;

    INT32 key_map[BT_AVRCP_CMD_TYPE_MAX] = {
        AVRC_ID_PLAY,
        AVRC_ID_PAUSE,
        AVRC_ID_FORWARD,
        AVRC_ID_BACKWARD,
        AVRC_ID_FAST_FOR,
        AVRC_ID_REWIND,
        AVRC_ID_STOP,
        AVRC_ID_VOL_UP,
        AVRC_ID_VOL_DOWN };
    BT_MW_FUNC_ENTER(BT_DEBUG_AVRCP, "id=%d, is_press=%d", id, is_press);

    for (i = 0;i < BT_AVRCP_CMD_TYPE_MAX;i ++)
    {
        if (key_map[i] == id)
        {
            bt_cmd_type = i;
            break;
        }
    }

    if (BT_AVRCP_CMD_TYPE_MAX == bt_cmd_type)
    {
        return;
    }

    strcpy(btmw_msg.data.avrcp_msg.addr, g_bt_avrcp_addr);
    btmw_msg.data.avrcp_msg.role = BT_MW_AVRCP_ROLE_TG;
    btmw_msg.hdr.event = BTMW_AVRCP_PASSTHROUGH_CMD_REQ;
    btmw_msg.data.avrcp_msg.data.passthrough_cmd_req.cmd_type = bt_cmd_type;
    btmw_msg.data.avrcp_msg.data.passthrough_cmd_req.action =
        is_press==0?BT_AVRCP_KEY_STATE_RELEASE:BT_AVRCP_KEY_STATE_PRESS;
    LINUXBT_AVRCP_TG_SET_MSG_LEN(btmw_msg);

    linuxbt_send_msg(&btmw_msg);
}


static void linuxbt_rc_rg_connection_state_cb(bool state, bt_bdaddr_t *bd_addr)
{
    tBTMW_MSG btmw_msg = {0};

    BT_CHECK_POINTER_RETURN(BT_DEBUG_AVRCP, bd_addr);

    linuxbt_btaddr_htos(bd_addr, btmw_msg.data.avrcp_msg.addr);
    BT_MW_FUNC_ENTER(BT_DEBUG_AVRCP, "state = %x bd_addr = %s",
        state, btmw_msg.data.avrcp_msg.addr);
    btmw_msg.data.avrcp_msg.role = BT_MW_AVRCP_ROLE_TG;
    if(FALSE == state)
    {
        g_bt_avrcp_addr[0] = 0;
        btmw_msg.hdr.event = BTMW_AVRCP_DISCONNECTED;
    }
    else
    {
        strcpy(g_bt_avrcp_addr, btmw_msg.data.avrcp_msg.addr);
        btmw_msg.hdr.event = BTMW_AVRCP_CONNECTED;
    }
    LINUXBT_AVRCP_TG_SET_MSG_LEN(btmw_msg);
    linuxbt_send_msg(&btmw_msg);
}

static void linuxbt_rc_tg_passthrough_cmd_rsp_cb(int id, int key_state)
{
    BT_DBG_NORMAL(BT_DEBUG_AVRCP, "id = %x, key state = %x ", id, key_state);
}


