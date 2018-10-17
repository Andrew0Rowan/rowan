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

/* FILE NAME:  linuxbt_avrcp_ct_if.c
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
#include "mtk_bluetooth.h"
#include "bt_mw_avrcp.h"
#include "bt_mw_message_queue.h"
#include "linuxbt_avrcp_ct_if.h"
#include "linuxbt_common.h"
#include "linuxbt_gap_if.h"

#if defined(MTK_LINUX_AVRCP_PLUS) && (MTK_LINUX_AVRCP_PLUS == TRUE)
#include "mtk_bt_rc.h"
#else
#include "bt_rc.h"
#endif

/* NAMING CONSTANT DECLARATIONS
 */
/* MACRO FUNCTION DECLARATIONS
 */
#define LINUXBT_AVRCP_CT_SET_MSG_LEN(msg) do{       \
    msg.hdr.len = sizeof(tBT_MW_AVRCP_MSG);         \
    }while(0)
/* DATA TYPE DECLARATIONS
 */
/* GLOBAL VARIABLE DECLARATIONS
 */
extern CHAR g_bt_avrcp_addr[MAX_BDADDR_LEN];
/* LOCAL SUBPROGRAM DECLARATIONS
 */
static void linuxbt_rc_passthrough_cmd_cb(int id, int key_state);

static void linuxbt_rc_connection_state_cb (bool state, bt_bdaddr_t *bd_addr);
static void linuxbt_rc_ctrl_groupnavigation_rsp_cb(int id, int key_state);
static void linuxbt_rc_ctrl_setabsvol_cmd_cb(bt_bdaddr_t *bd_addr,
    uint8_t abs_vol, uint8_t label);
static void linuxbt_rc_ctrl_registernotification_abs_vol_cb (bt_bdaddr_t *bd_addr,
    uint8_t label);
static void linuxbt_rc_ctrl_play_position_changed_cb(bt_bdaddr_t *bd_addr,
    uint32_t song_len, uint32_t song_pos);
static void linuxbt_rc_ctrl_play_status_changed_cb(bt_bdaddr_t *bd_addr,
    btrc_play_status_t play_status);
#if defined(MTK_LINUX_AVRCP_PLUS) && (MTK_LINUX_AVRCP_PLUS == TRUE)
static void linuxbt_rc_tg_passthrough_volume_cmd_cb (int id, int key_state);
#endif
static void linuxbt_rc_ctrl_groupnavigation_rsp_cb(int id, int key_state);
static void linuxbt_rc_ctrl_remote_features_cb(bt_bdaddr_t *bd_addr, int features);
static void linuxbt_rc_ctrl_setplayerapplicationsetting_rsp_cb(bt_bdaddr_t *bd_addr,
    uint8_t accepted);
static void linuxbt_rc_ctrl_playerapplicationsetting_cb(bt_bdaddr_t *bd_addr,
     uint8_t num_attr,
     btrc_player_app_attr_t *app_attrs,
     uint8_t num_ext_attr,
     btrc_player_app_ext_attr_t *ext_attrs);

static void linuxbt_rc_ctrl_playerapplicationsetting_changed_cb(bt_bdaddr_t *bd_addr,
    btrc_player_settings_t *p_vals);
static void linuxbt_rc_ctrl_track_changed_cb(bt_bdaddr_t *bd_addr,
    uint8_t num_attr, btrc_element_attr_val_t *p_attrs);


/* STATIC VARIABLE DECLARATIONS
 */
static btrc_ctrl_interface_t *g_bt_rc_ct_interface = NULL;

#if defined(MTK_LINUX_AVRCP_PLUS) && (MTK_LINUX_AVRCP_PLUS == TRUE)
static const btrc_ctrl_ex_interface_t *g_bt_avrcp_ex_interface = NULL;
#endif


static btrc_ctrl_callbacks_t g_bt_rc_callbacks =
{
    sizeof(btrc_ctrl_callbacks_t),
    linuxbt_rc_passthrough_cmd_cb,
    linuxbt_rc_ctrl_groupnavigation_rsp_cb, //btrc_groupnavigation_rsp_callback
    linuxbt_rc_connection_state_cb,
    linuxbt_rc_ctrl_remote_features_cb, //btrc_ctrl_getrcfeatures_callback
    linuxbt_rc_ctrl_setplayerapplicationsetting_rsp_cb, //btrc_ctrl_setplayerapplicationsetting_rsp_callback
    linuxbt_rc_ctrl_playerapplicationsetting_cb, //btrc_ctrl_playerapplicationsetting_callback
    linuxbt_rc_ctrl_playerapplicationsetting_changed_cb, //btrc_ctrl_playerapplicationsetting_changed_callback
    linuxbt_rc_ctrl_setabsvol_cmd_cb, //btrc_ctrl_setabsvol_cmd_callback
    linuxbt_rc_ctrl_registernotification_abs_vol_cb, //btrc_ctrl_registernotification_abs_vol_callback
    linuxbt_rc_ctrl_track_changed_cb, //btrc_ctrl_track_changed_callback
    linuxbt_rc_ctrl_play_position_changed_cb, //btrc_ctrl_play_position_changed_callback
    linuxbt_rc_ctrl_play_status_changed_cb, //btrc_ctrl_play_status_changed_callback
};

#if defined(MTK_LINUX_AVRCP_PLUS) && (MTK_LINUX_AVRCP_PLUS == TRUE)
static btrc_ctrl_ext_callbacks_t g_bt_rc_ext_callbacks =
{
    sizeof(btrc_ctrl_ext_callbacks_t),
    linuxbt_rc_tg_passthrough_volume_cmd_cb,// local device (A2DP SNK) works as AVRCP TG, remote device (A2DP SRC ,AVRCP CT) send VOLUME UP/DOWN passthrough cmd
};
#endif


/* EXPORTED SUBPROGRAM BODIES
 */

INT32 linuxbt_rc_init(void)
{
    int ret = 0;
    BT_MW_FUNC_ENTER(BT_DEBUG_AVRCP, "");

    g_bt_rc_ct_interface =
        (btrc_ctrl_interface_t *)linuxbt_gap_get_profile_interface(BT_PROFILE_AV_RC_CTRL_ID);

    if (g_bt_rc_ct_interface == NULL)
    {
        BT_DBG_ERROR(BT_DEBUG_AVRCP,  " Failed to get AVRCP interface");
        return -1;
    }
#if defined(MTK_LINUX_AVRCP_PLUS) && (MTK_LINUX_AVRCP_PLUS == TRUE)
    g_bt_avrcp_ex_interface =
    (btrc_ctrl_ex_interface_t *)linuxbt_gap_get_profile_interface(BT_PROFILE_AVRCP_EX_ID);
    if (NULL == g_bt_avrcp_ex_interface)
    {
        BT_DBG_WARNING(BT_DEBUG_AVRCP, "Failed to get AVRCP extended interface");
    }
    // Init AVRCP ext interface
    ret = g_bt_avrcp_ex_interface->init(&g_bt_rc_ext_callbacks);
    if (ret == BT_STATUS_SUCCESS)
    {
        BT_DBG_WARNING(BT_DEBUG_AVRCP, " success to init AVRCP CT ext interface");
    }
    else if(ret == BT_STATUS_DONE)
    {
        BT_DBG_WARNING(BT_DEBUG_AVRCP, " already init AVRCP CT ext interface");
    }
#endif

    // Init AVRCP interface
    ret = g_bt_rc_ct_interface->init(&g_bt_rc_callbacks);
    if (ret == BT_STATUS_SUCCESS)
    {
        BT_DBG_NORMAL(BT_DEBUG_AVRCP, " success to init AVRCP CT interface");
        return BT_SUCCESS;
    }
    else if(ret == BT_STATUS_DONE)
    {
        BT_DBG_NORMAL(BT_DEBUG_AVRCP, " already init AVRCP CT interface");
        return BT_SUCCESS;
    }
    BT_DBG_ERROR(BT_DEBUG_AVRCP,  " Failed to init AVRCP CT interface");

    return ret;
}

INT32 linuxbt_rc_deinit(void)
{
    BT_MW_FUNC_ENTER(BT_DEBUG_AVRCP, "");

    if (g_bt_rc_ct_interface == NULL)
    {
        BT_DBG_ERROR(BT_DEBUG_AVRCP,  " Failed to get AVRCP interface");
        return -1;
    }

    g_bt_rc_ct_interface->cleanup();

    g_bt_rc_ct_interface = NULL;

    return BT_SUCCESS;

}


INT32 linuxbt_rc_send_passthrough_cmd_handler(CHAR *addr,
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
    BT_CHECK_POINTER(BT_DEBUG_AVRCP, addr);

    linuxbt_btaddr_stoh(addr, &bdaddr);
    if (g_bt_rc_ct_interface == NULL)
    {
        BT_DBG_ERROR(BT_DEBUG_AVRCP,  " Failed to get AVRCP interface");
        return -1;
    }

    if (BT_AVRCP_KEY_STATE_PRESS == key_state)
    {
        g_bt_rc_ct_interface->send_pass_through_cmd(&bdaddr,
            key_map[cmd_type], AVRC_STATE_PRESS);
    }
    else if (BT_AVRCP_KEY_STATE_RELEASE == key_state)
    {
        g_bt_rc_ct_interface->send_pass_through_cmd(&bdaddr,
            key_map[cmd_type], AVRC_STATE_RELEASE);
    }
    else
    {
        g_bt_rc_ct_interface->send_pass_through_cmd(&bdaddr,
            key_map[cmd_type], AVRC_STATE_PRESS);
        usleep(200*1000);
        g_bt_rc_ct_interface->send_pass_through_cmd(&bdaddr,
            key_map[cmd_type], AVRC_STATE_RELEASE);
    }

    return BT_SUCCESS;
}

INT32 linuxbt_rc_send_vendor_unique_cmd_handler(CHAR *addr, UINT8 key,
    BT_AVRCP_KEY_STATE key_state)
{
    bt_bdaddr_t bdaddr;
    BT_CHECK_POINTER(BT_DEBUG_AVRCP, addr);
    BT_MW_FUNC_ENTER(BT_DEBUG_AVRCP, "key=%d, action=%d", key, key_state);

    linuxbt_btaddr_stoh(addr, &bdaddr);
    if (g_bt_rc_ct_interface == NULL)
    {
        BT_DBG_ERROR(BT_DEBUG_AVRCP,  " Failed to get AVRCP interface");
        return -1;
    }

    if (BT_AVRCP_KEY_STATE_PRESS == key_state)
    {
        g_bt_rc_ct_interface->send_group_navigation_cmd(&bdaddr,
            key, AVRC_STATE_PRESS);
    }
    else if (BT_AVRCP_KEY_STATE_RELEASE == key_state)
    {
        g_bt_rc_ct_interface->send_group_navigation_cmd(&bdaddr,
            key, AVRC_STATE_RELEASE);
    }
    else
    {
        g_bt_rc_ct_interface->send_group_navigation_cmd(&bdaddr,
            key, AVRC_STATE_PRESS);
        usleep(200*1000);
        g_bt_rc_ct_interface->send_group_navigation_cmd(&bdaddr,
            key, AVRC_STATE_RELEASE);
    }

    return BT_SUCCESS;
}



INT32 linuxbt_rc_send_get_playstatus_cmd_handler(VOID)
{
    BT_MW_FUNC_ENTER(BT_DEBUG_AVRCP, "");
#if defined(MTK_LINUX_AVRCP_PLUS) && (MTK_LINUX_AVRCP_PLUS == TRUE)
    if (g_bt_avrcp_ex_interface == NULL)
    {
        BT_DBG_ERROR(BT_DEBUG_AVRCP,  " Failed to get AVRCP interface");
        return -1;
    }

    g_bt_avrcp_ex_interface->send_get_playstatus_cmd();
#endif

    return BT_SUCCESS;
}


INT32 linuxbt_rc_set_volume_rsp(CHAR *addr, UINT8 label, UINT8 volume)
{
    bt_bdaddr_t bdaddr;
    BT_CHECK_POINTER(BT_DEBUG_AVRCP, addr);

    linuxbt_btaddr_stoh(addr, &bdaddr);
    if (g_bt_rc_ct_interface == NULL)
    {
        BT_DBG_NOTICE(BT_DEBUG_AVRCP,  "[AVRCP] Failed to get AVRCP interface");
        return -1;
    }
    g_bt_rc_ct_interface->set_volume_rsp(&bdaddr, volume, label);
    return BT_SUCCESS;
}


INT32 linuxbt_rc_send_volume_change_rsp_handler(CHAR *addr, INT32 interim,
    UINT8 label, UINT32 volume)
{
    bt_bdaddr_t bdaddr;
    BT_CHECK_POINTER(BT_DEBUG_AVRCP, addr);

    linuxbt_btaddr_stoh(addr, &bdaddr);
    BT_DBG_NORMAL(BT_DEBUG_AVRCP,  "[AVRCP] send volume:%x", volume);

    if (g_bt_rc_ct_interface == NULL)
    {
        BT_DBG_NOTICE(BT_DEBUG_AVRCP,  "[AVRCP] Failed to get AVRCP interface");
        return -1;
    }

    if (interim)
    {
        g_bt_rc_ct_interface->register_abs_vol_rsp(&bdaddr,
            BTRC_NOTIFICATION_TYPE_INTERIM, volume, label);
    }
    else
    {
        g_bt_rc_ct_interface->register_abs_vol_rsp(&bdaddr,
            BTRC_NOTIFICATION_TYPE_CHANGED, volume, label);
    }

    return BT_SUCCESS;
}

#if defined(BT_RPC_DBG_SERVER)
EXPORT_SYMBOL int dbg_avrcp_get_g_media_info(int array_index, int offset,
char *name, char *data, int length)
{
    switch(offset)
    {
        case 0:
            sprintf(name, "title");
            break;
        case 1:
            sprintf(name, "artist");
            break;
        case 2:
            sprintf(name, "album");
            break;
        case 3:
            sprintf(name, "current_track_number");
            break;
        case 4:
            sprintf(name, "number_of_tracks");
            break;
        case 5:
            sprintf(name, "genre");
            break;
        case 6:
            sprintf(name, "position");
            break;
        default:
            return BT_SUCCESS;
    }

    return offset + 1;
}

EXPORT_SYMBOL int dbg_avrcp_get_g_player_status(int array_index, int offset,
    char *name, char *data, int length)
{
    switch(offset)
    {
        case 0:
            sprintf(name, "song_length");
            break;
        case 1:
            sprintf(name, "song_position");
            break;
        case 2:
            sprintf(name, "play_status");
            break;

        default:
            return BT_SUCCESS;
    }

        return offset + 1;
    }


#endif

/* LOCAL SUBPROGRAM BODIES
 */


static void linuxbt_rc_passthrough_cmd_cb(int id, int key_state)
{
    BT_MW_FUNC_ENTER(BT_DEBUG_AVRCP, "id = %x, key state = %x ", id, key_state);
}

static void linuxbt_rc_connection_state_cb(bool state, bt_bdaddr_t *bd_addr)
{
    tBTMW_MSG btmw_msg = {0};

    BT_CHECK_POINTER_RETURN(BT_DEBUG_AVRCP, bd_addr);

    linuxbt_btaddr_htos(bd_addr, btmw_msg.data.avrcp_msg.addr);
    BT_MW_FUNC_ENTER(BT_DEBUG_AVRCP, "state = %x bd_addr = %s",
        state, btmw_msg.data.avrcp_msg.addr);
    btmw_msg.data.avrcp_msg.role = BT_MW_AVRCP_ROLE_CT;
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
    LINUXBT_AVRCP_CT_SET_MSG_LEN(btmw_msg);
    linuxbt_send_msg(&btmw_msg);
}

static void linuxbt_rc_ctrl_groupnavigation_rsp_cb(int id, int key_state)
{
    BT_MW_FUNC_ENTER(BT_DEBUG_AVRCP, "id = %d key_state = %d ", id, key_state);
}

static void linuxbt_rc_ctrl_remote_features_cb(bt_bdaddr_t *bd_addr, int features)
{
    tBTMW_MSG btmw_msg = {0};

    BT_CHECK_POINTER_RETURN(BT_DEBUG_AVRCP, bd_addr);

    linuxbt_btaddr_htos(bd_addr, btmw_msg.data.avrcp_msg.addr);
    BT_MW_FUNC_ENTER(BT_DEBUG_AVRCP, "[AVRCP]%s CT features = 0x%x ",
        btmw_msg.data.avrcp_msg.addr, features);
    btmw_msg.data.avrcp_msg.role = BT_MW_AVRCP_ROLE_CT;
    btmw_msg.data.avrcp_msg.data.feature = features;

    btmw_msg.hdr.event = BTMW_AVRCP_FEATURE;
    LINUXBT_AVRCP_CT_SET_MSG_LEN(btmw_msg);

    linuxbt_send_msg(&btmw_msg);
}

static void linuxbt_rc_ctrl_setplayerapplicationsetting_rsp_cb(bt_bdaddr_t *bd_addr,
    uint8_t accepted)
{
    CHAR addr[MAX_BDADDR_LEN];
    BT_CHECK_POINTER_RETURN(BT_DEBUG_AVRCP, bd_addr);
    linuxbt_btaddr_htos(bd_addr, addr);
    BT_MW_FUNC_ENTER(BT_DEBUG_AVRCP, "%s accepted = %d ", addr, accepted);
}

static void linuxbt_rc_ctrl_playerapplicationsetting_cb(bt_bdaddr_t *bd_addr,
     uint8_t num_attr,
     btrc_player_app_attr_t *app_attrs,
     uint8_t num_ext_attr,
     btrc_player_app_ext_attr_t *ext_attrs)
{
    CHAR addr[MAX_BDADDR_LEN];
    BT_CHECK_POINTER_RETURN(BT_DEBUG_AVRCP, bd_addr);
    linuxbt_btaddr_htos(bd_addr, addr);
    BT_MW_FUNC_ENTER(BT_DEBUG_AVRCP, "%s num_attr = %d, num_ext_attr = %d ",
        addr, num_attr, num_ext_attr);
}

static void linuxbt_rc_ctrl_playerapplicationsetting_changed_cb(bt_bdaddr_t *bd_addr,
    btrc_player_settings_t *p_vals)
{
    CHAR addr[MAX_BDADDR_LEN];
    BT_CHECK_POINTER_RETURN(BT_DEBUG_AVRCP, bd_addr);
    linuxbt_btaddr_htos(bd_addr, addr);
    BT_MW_FUNC_ENTER(BT_DEBUG_AVRCP, "%s num_attr = %d ", addr, p_vals->num_attr);
}


static void linuxbt_rc_ctrl_setabsvol_cmd_cb(bt_bdaddr_t *bd_addr,
    uint8_t abs_vol, uint8_t label)
{
    tBTMW_MSG btmw_msg = {0};
    BT_CHECK_POINTER_RETURN(BT_DEBUG_AVRCP, bd_addr);

    linuxbt_btaddr_htos(bd_addr, btmw_msg.data.avrcp_msg.addr);
    BT_MW_FUNC_ENTER(BT_DEBUG_AVRCP, "%s label = %x volume = %d",
        btmw_msg.data.avrcp_msg.addr, label,abs_vol);

    btmw_msg.hdr.event = BTMW_AVRCP_SET_VOLUME_REQ;

    btmw_msg.data.avrcp_msg.role = BT_MW_AVRCP_ROLE_CT;
    btmw_msg.data.avrcp_msg.label = label;
    btmw_msg.data.avrcp_msg.data.set_vol_req.abs_volume = abs_vol;
    LINUXBT_AVRCP_CT_SET_MSG_LEN(btmw_msg);

    linuxbt_send_msg(&btmw_msg);
}

static void linuxbt_rc_ctrl_registernotification_abs_vol_cb(bt_bdaddr_t *bd_addr,
    uint8_t label)
{
    tBTMW_MSG btmw_msg = {0};
    BT_CHECK_POINTER_RETURN(BT_DEBUG_AVRCP, bd_addr);

    linuxbt_btaddr_htos(bd_addr, btmw_msg.data.avrcp_msg.addr);
    BT_MW_FUNC_ENTER(BT_DEBUG_AVRCP, "%s label = %x ",
        btmw_msg.data.avrcp_msg.addr, label);

    btmw_msg.hdr.event = BTMW_AVRCP_REG_EVENT_REQ;

    btmw_msg.data.avrcp_msg.role = BT_MW_AVRCP_ROLE_CT;
    btmw_msg.data.avrcp_msg.label = label;
    btmw_msg.data.avrcp_msg.data.reg_event_req.event_id = BT_AVRCP_REG_EVT_ABS_VOLUME_CHANGED;
    LINUXBT_AVRCP_CT_SET_MSG_LEN(btmw_msg);

    linuxbt_send_msg(&btmw_msg);
}

static void linuxbt_rc_ctrl_track_changed_cb(bt_bdaddr_t *bd_addr,
    uint8_t num_attr, btrc_element_attr_val_t *p_attrs)
{
    tBTMW_MSG btmw_msg = {0};
    int i = 0;
    UINT8 *ptr_data = NULL;
    BT_CHECK_POINTER_RETURN(BT_DEBUG_AVRCP, bd_addr);

    linuxbt_btaddr_htos(bd_addr, btmw_msg.data.avrcp_msg.addr);
    BT_MW_FUNC_ENTER(BT_DEBUG_AVRCP, "%s num_attr = %d ",
        btmw_msg.data.avrcp_msg.addr, num_attr);

    btmw_msg.hdr.event = BTMW_AVRCP_TRACK_CHANGE;

    btmw_msg.data.avrcp_msg.role = BT_MW_AVRCP_ROLE_CT;
    if(p_attrs[0].attr_id == 0xFF)
    {
        BT_DBG_ERROR(BT_DEBUG_AVRCP,  " ERROR %d", p_attrs[0].attr_id);
        return;
    }

    for ( i = 0 ; i < num_attr ; i++ )
    {
        ptr_data = (UINT8 *)&p_attrs[i].text;

        switch ((int)p_attrs[i].attr_id)
        {
            case BTRC_MEDIA_ATTR_TITLE://BTRC_MEDIA_ATTR_TITLE = 0x01
            {
                strncpy((char *)btmw_msg.data.avrcp_msg.data.track_change.element_attr.title,
                    (char *)ptr_data, BT_AVRCP_MAX_NAME_LEN-1);
                BT_DBG_NOTICE(BT_DEBUG_AVRCP, "[%d](title):%s",
                    p_attrs[i].attr_id,
                    btmw_msg.data.avrcp_msg.data.track_change.element_attr.title);
                break;
            }
            case BTRC_MEDIA_ATTR_ARTIST://BTRC_MEDIA_ATTR_ARTIST = 0x02
            {
                strncpy((char *)btmw_msg.data.avrcp_msg.data.track_change.element_attr.artist,
                    (char *)ptr_data, BT_AVRCP_MAX_NAME_LEN-1);
                BT_DBG_NOTICE(BT_DEBUG_AVRCP, "[%d](artist):%s",
                    (int)p_attrs[i].attr_id,
                    btmw_msg.data.avrcp_msg.data.track_change.element_attr.artist);
                break;
            }
            case BTRC_MEDIA_ATTR_ALBUM://BTRC_MEDIA_ATTR_ALBUM = 0x03
            {
                strncpy((char *)btmw_msg.data.avrcp_msg.data.track_change.element_attr.album,
                    (char *)ptr_data, BT_AVRCP_MAX_NAME_LEN-1);
                BT_DBG_NOTICE(BT_DEBUG_AVRCP, "[%d](album):%s",
                    (int)p_attrs[i].attr_id,
                    btmw_msg.data.avrcp_msg.data.track_change.element_attr.album);
                break;
            }
            case BTRC_MEDIA_ATTR_TRACK_NUM://BTRC_MEDIA_ATTR_TRACK_NUM = 0x04
            {
                BT_DBG_NOTICE(BT_DEBUG_AVRCP, "[%d](track_number):%s",
                    (int)p_attrs[i].attr_id, (char *)ptr_data);
                btmw_msg.data.avrcp_msg.data.track_change.element_attr.current_track_number = atol((char *)ptr_data);
                BT_DBG_NOTICE(BT_DEBUG_AVRCP, "current_track_number:%ld",
                    (long)btmw_msg.data.avrcp_msg.data.track_change.element_attr.current_track_number);
                break;
            }
            case BTRC_MEDIA_ATTR_NUM_TRACKS://BTRC_MEDIA_ATTR_NUM_TRACKS = 0x05
            {
                BT_DBG_NOTICE(BT_DEBUG_AVRCP, "[%d](track_number):%s",
                    (int)p_attrs[i].attr_id, (char *)ptr_data);
                btmw_msg.data.avrcp_msg.data.track_change.element_attr.number_of_tracks = atol((char *)ptr_data);
                BT_DBG_NOTICE(BT_DEBUG_AVRCP, "number_of_tracks:%ld",
                    (long)btmw_msg.data.avrcp_msg.data.track_change.element_attr.number_of_tracks);
                break;
            }
            case BTRC_MEDIA_ATTR_GENRE://BTRC_MEDIA_ATTR_GENRE = 0x06
            {
                strncpy((char *)btmw_msg.data.avrcp_msg.data.track_change.element_attr.genre,
                    (char *)ptr_data, BT_AVRCP_MAX_NAME_LEN-1);
                BT_DBG_NOTICE(BT_DEBUG_AVRCP, "[%d](genre):%s",
                    (int)p_attrs[i].attr_id, btmw_msg.data.avrcp_msg.data.track_change.element_attr.genre);
                break;
            }
            case BTRC_MEDIA_ATTR_PLAYING_TIME://BTRC_MEDIA_ATTR_PLAYING_TIME = 0x07
            {
                BT_DBG_NOTICE(BT_DEBUG_AVRCP, "[%d](genre):%s", (int)p_attrs[i].attr_id, (char *)ptr_data);
                btmw_msg.data.avrcp_msg.data.track_change.element_attr.position = atol((char *)ptr_data);
                BT_DBG_NOTICE(BT_DEBUG_AVRCP, "position:%ld",
                    (long)btmw_msg.data.avrcp_msg.data.track_change.element_attr.position);
                break;
            }
            default:
            {
                BT_DBG_WARNING(BT_DEBUG_AVRCP, "Unknown Attribute_ID : %d",
                    (int)p_attrs[i].attr_id);
                break;
            }
        }
    }
    LINUXBT_AVRCP_CT_SET_MSG_LEN(btmw_msg);

    linuxbt_send_msg(&btmw_msg);
}


static void linuxbt_rc_ctrl_play_position_changed_cb(bt_bdaddr_t *bd_addr,
    uint32_t song_len, uint32_t song_pos)
{
    tBTMW_MSG btmw_msg = {0};
    BT_CHECK_POINTER_RETURN(BT_DEBUG_AVRCP, bd_addr);

    linuxbt_btaddr_htos(bd_addr, btmw_msg.data.avrcp_msg.addr);

    BT_MW_FUNC_ENTER(BT_DEBUG_AVRCP, "%s play changed, song_len=%d, song_pos=%d",
        btmw_msg.data.avrcp_msg.addr, song_len, song_pos);

    btmw_msg.hdr.event = BTMW_AVRCP_PLAY_POS_CHANGE;

    btmw_msg.data.avrcp_msg.role = BT_MW_AVRCP_ROLE_CT;
    btmw_msg.data.avrcp_msg.data.pos_change.song_len = song_len;
    btmw_msg.data.avrcp_msg.data.pos_change.song_pos = song_pos;
    LINUXBT_AVRCP_CT_SET_MSG_LEN(btmw_msg);

    linuxbt_send_msg(&btmw_msg);
}

static void linuxbt_rc_ctrl_play_status_changed_cb(bt_bdaddr_t *bd_addr,
    btrc_play_status_t play_status)
{
    tBTMW_MSG btmw_msg = {0};
    BT_CHECK_POINTER_RETURN(BT_DEBUG_AVRCP, bd_addr);

    linuxbt_btaddr_htos(bd_addr, btmw_msg.data.avrcp_msg.addr);

    BT_MW_FUNC_ENTER(BT_DEBUG_AVRCP, "%s play status changed, play_status=%d",
        btmw_msg.data.avrcp_msg.addr, play_status);

    btmw_msg.hdr.event = BTMW_AVRCP_PLAY_STATUS_CHANGE;

    btmw_msg.data.avrcp_msg.role = BT_MW_AVRCP_ROLE_CT;
    btmw_msg.data.avrcp_msg.data.play_status_change.play_status = play_status;
    LINUXBT_AVRCP_CT_SET_MSG_LEN(btmw_msg);

    linuxbt_send_msg(&btmw_msg);
}

#if defined(MTK_LINUX_AVRCP_PLUS) && (MTK_LINUX_AVRCP_PLUS == TRUE)
/* just for PTS:AVRCP_TG_PTT_BV_02_I
local role:     A2DP SNK, AVRCP TG
remote role:  A2DP SRC, AVRCP CT
remote send AVRCP VOLUME UP/DOWN passthrough command to local
*/
static void linuxbt_rc_tg_passthrough_volume_cmd_cb (int id, int key_state)
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

    btmw_msg.data.avrcp_msg.role = BT_MW_AVRCP_ROLE_TG;
    btmw_msg.hdr.event = BTMW_AVRCP_PASSTHROUGH_CMD_REQ;
    btmw_msg.data.avrcp_msg.data.passthrough_cmd_req.cmd_type = bt_cmd_type;
    btmw_msg.data.avrcp_msg.data.passthrough_cmd_req.action = key_state;
    LINUXBT_AVRCP_CT_SET_MSG_LEN(btmw_msg);

    linuxbt_send_msg(&btmw_msg);
}
#endif


