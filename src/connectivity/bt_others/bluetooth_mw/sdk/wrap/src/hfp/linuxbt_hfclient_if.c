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

#include "bt_mw_common.h"
#include "linuxbt_common.h"
#include "linuxbt_hfclient_if.h"
#include "bt_mw_hfclient.h"
#include "bt_mw_message_queue.h"
#if defined(MTK_LINUX_HFP) && (MTK_LINUX_HFP == TRUE)
#include "mtk_bt_hf_client.h"
#include "mtk_bluetooth.h"
#endif

extern void *linuxbt_gap_get_profile_interface(const char *profile_id);
static void linuxbt_hfclient_connection_state_cb(bthf_client_connection_state_t state,
    unsigned int peer_feat, unsigned int chld_feat, bt_bdaddr_t *bd_addr);
static void linuxbt_hfclient_audio_state_cb(bthf_client_audio_state_t state, bt_bdaddr_t *bd_addr);
static void linuxbt_hfclient_vr_cmd_cb(bthf_client_vr_state_t state);
static void linuxbt_hfclient_network_state_cb(bthf_client_network_state_t state);
static void linuxbt_hfclient_network_roaming_cb(bthf_client_service_type_t type);
static void linuxbt_hfclient_network_signal_cb(int signal_strength);
static void linuxbt_hfclient_battery_level_cb(int battery_level);
static void linuxbt_hfclient_current_operator_cb(const char *name);
static void linuxbt_hfclient_call_cb(bthf_client_call_t call);
static void linuxbt_hfclient_callsetup_cb(bthf_client_callsetup_t callsetup);
static void linuxbt_hfclient_callheld_cb(bthf_client_callheld_t callheld);
static void linuxbt_hfclient_resp_and_hold_cb(bthf_client_resp_and_hold_t resp_and_hold);
static void linuxbt_hfclient_clip_cb(const char *number);
static void linuxbt_hfclient_call_waiting_cb(const char *number);
static void linuxbt_hfclient_current_calls_cb(int index, bthf_client_call_direction_t dir,
    bthf_client_call_state_t state, bthf_client_call_mpty_type_t mpty, const char *number);
static void linuxbt_hfclient_volume_change_cb(bthf_client_volume_type_t type, int volume);
static void linuxbt_hfclient_cmd_complete_cb(bthf_client_cmd_complete_t type, int cme);
static void linuxbt_hfclient_subscriber_info_cb(const char *name, bthf_client_subscriber_service_type_t type);
static void linuxbt_hfclient_in_band_ring_tone_cb(bthf_client_in_band_ring_state_t state);
static void linuxbt_hfclient_last_voice_tag_number_cb(const char *number);
static void linuxbt_hfclient_ring_indication_cb(void);

#if defined(MTK_LINUX_HFP) && (MTK_LINUX_HFP == TRUE)
static void linuxbt_client_ability_cb(bthf_client_ability_status_t state);
#if defined(MTK_LINUX_HFP_PHONEBOOK) && (MTK_LINUX_HFP_PHONEBOOK == TRUE)
static void linuxbt_hfclient_cpbs_cb(int *storage);
static void linuxbt_hfclient_cpbr_count_cb(int idx_max);
static void linuxbt_hfclient_cpbr_entry_cb(int index,  const char *number, int type,  const char *name);
static void linuxbt_hfclient_cpbr_complete_cb();
#endif
#endif

static bthf_client_interface_t *g_bt_hfclient_interface = NULL;
static bthf_client_callbacks_t g_bt_hfclient_callbacks =
{
    sizeof(bthf_client_callbacks_t),
    linuxbt_hfclient_connection_state_cb,
    linuxbt_hfclient_audio_state_cb,
    linuxbt_hfclient_vr_cmd_cb,
    linuxbt_hfclient_network_state_cb,
    linuxbt_hfclient_network_roaming_cb,
    linuxbt_hfclient_network_signal_cb,
    linuxbt_hfclient_battery_level_cb,
    linuxbt_hfclient_current_operator_cb,
    linuxbt_hfclient_call_cb,
    linuxbt_hfclient_callsetup_cb,
    linuxbt_hfclient_callheld_cb,
    linuxbt_hfclient_resp_and_hold_cb,
    linuxbt_hfclient_clip_cb,
    linuxbt_hfclient_call_waiting_cb,
    linuxbt_hfclient_current_calls_cb,
    linuxbt_hfclient_volume_change_cb,
    linuxbt_hfclient_cmd_complete_cb,
    linuxbt_hfclient_subscriber_info_cb,
    linuxbt_hfclient_in_band_ring_tone_cb,
    linuxbt_hfclient_last_voice_tag_number_cb,
    linuxbt_hfclient_ring_indication_cb,
};

#if defined(MTK_LINUX_HFP) && (MTK_LINUX_HFP == TRUE)
static void linuxbt_client_ability_cb(bthf_client_ability_status_t state)
{
    BT_DBG_INFO(BT_DEBUG_HFP, "state=%s\n", state ? "DISABLE" : "ENABLE");

    tBTMW_MSG btmw_msg = {0};
    btmw_msg.hdr.event = BTMW_HFCLIENT_ABILITY_CB_EVT;
    btmw_msg.data.hfclient_msg.data.ability_cb.state = (BT_HFCLIENT_ABILITY_STATE_T)state;

    linuxbt_send_msg(&btmw_msg);
}

#if defined(MTK_LINUX_HFP_PHONEBOOK) && (MTK_LINUX_HFP_PHONEBOOK == TRUE)
static void linuxbt_hfclient_cpbs_cb(int *storage)
{
    BT_DBG_INFO(BT_DEBUG_HFP, "\n");

    tBTMW_MSG btmw_msg = {0};
    btmw_msg.hdr.event = BTMW_HFCLIENT_CPBS_CB_EVT;

    memcpy(btmw_msg.data.hfclient_msg.data.cpbs_cb.storage_lookup, storage, HFCLIENT_PB_STORAGE_COUNT*sizeof(UINT8));

    linuxbt_send_msg(&btmw_msg);
}

static void linuxbt_hfclient_cpbr_count_cb(int idx_max)
{
    BT_DBG_INFO(BT_DEBUG_HFP, "index_max=%d\n", idx_max);

    tBTMW_MSG btmw_msg = {0};
    btmw_msg.hdr.event = BTMW_HFCLIENT_CPBR_COUNT_CB_EVT;

    btmw_msg.data.hfclient_msg.data.cpbr_count_cb.idx_max = idx_max;

    linuxbt_send_msg(&btmw_msg);
}

static void linuxbt_hfclient_cpbr_entry_cb(int index,  const char *number, int type,  const char *name)
{
    tBTMW_MSG btmw_msg = {0};
    btmw_msg.hdr.event = BTMW_HFCLIENT_CPBR_ENTRY_CB_EVT;

    memset(btmw_msg.data.hfclient_msg.data.cpbr_entry_cb.pb_entry.number, 0, HFCLIENT_CPBR_NUMBER_LEN + 1);
    memset(btmw_msg.data.hfclient_msg.data.cpbr_entry_cb.pb_entry.name, 0, HFCLIENT_CPBR_NAME_LEN + 1);

    btmw_msg.data.hfclient_msg.data.cpbr_entry_cb.pb_entry.index = index;
    strncpy(btmw_msg.data.hfclient_msg.data.cpbr_entry_cb.pb_entry.number, number, HFCLIENT_CPBR_NUMBER_LEN + 1);
    btmw_msg.data.hfclient_msg.data.cpbr_entry_cb.pb_entry.number[HFCLIENT_CPBR_NUMBER_LEN] = '\0';
    btmw_msg.data.hfclient_msg.data.cpbr_entry_cb.pb_entry.type = type;
    strncpy(btmw_msg.data.hfclient_msg.data.cpbr_entry_cb.pb_entry.name, name, HFCLIENT_CPBR_NAME_LEN + 1);
    btmw_msg.data.hfclient_msg.data.cpbr_entry_cb.pb_entry.name[HFCLIENT_CPBR_NAME_LEN] = '\0';

    linuxbt_send_msg(&btmw_msg);
}

static void linuxbt_hfclient_cpbr_complete_cb()
{
    BT_DBG_INFO(BT_DEBUG_HFP, "\n");

    tBTMW_MSG btmw_msg = {0};
    btmw_msg.hdr.event = BTMW_HFCLIENT_CPBR_COMPLETE_CB_EVT;

    linuxbt_send_msg(&btmw_msg);
}
#endif
#endif

#if defined(MTK_LINUX_HFP) && (MTK_LINUX_HFP == TRUE)
static bthf_client_ex_interface_t *g_bt_hfclient_ex_interface = NULL;

static bthf_client_ex_callbacks_t g_bt_hfclient_ex_callbacks =
{
    sizeof(bthf_client_ex_callbacks_t),
    linuxbt_client_ability_cb,
#if defined(MTK_LINUX_HFP_PHONEBOOK) && (MTK_LINUX_HFP_PHONEBOOK == TRUE)
    linuxbt_hfclient_cpbs_cb,
    linuxbt_hfclient_cpbr_count_cb,
    linuxbt_hfclient_cpbr_entry_cb,
    linuxbt_hfclient_cpbr_complete_cb,
#endif
};
#endif

#if defined(MTK_LINUX_HFP) && (MTK_LINUX_HFP == TRUE)
int linuxbt_hfclient_enable(void)
{
    BT_DBG_INFO(BT_DEBUG_HFP, "begin \n");
    INT32 ret = BT_STATUS_SUCCESS;
    g_bt_hfclient_ex_interface = (bthf_client_ex_interface_t *)linuxbt_gap_get_profile_interface(BT_PROFILE_HANDSFREE_CLIENT_EX_ID);
    if (NULL == g_bt_hfclient_ex_interface)
    {
        BT_DBG_ERROR(BT_DEBUG_HFP, "Failed to get HFP Client EX interface\n");
        return BT_ERR_STATUS_FAIL;
    }

    ret = g_bt_hfclient_ex_interface->enable(&g_bt_hfclient_callbacks);
    if (BT_STATUS_SUCCESS != ret)
    {
        BT_DBG_ERROR(BT_DEBUG_HFP, "enable failed! ret=%d\n", ret);
        return ret;
    }

    ret = g_bt_hfclient_ex_interface->enable_ex(&g_bt_hfclient_ex_callbacks);
    if (BT_STATUS_SUCCESS != ret)
    {
        BT_DBG_ERROR(BT_DEBUG_HFP, "enable_ex failed! ret=%d\n", ret);
        return ret;
    }
    return ret;
}

int linuxbt_hfclient_disable(void)
{
    BT_DBG_INFO(BT_DEBUG_HFP, "begin \n");
    bt_status_t ret = BT_STATUS_SUCCESS;

    if (NULL == g_bt_hfclient_ex_interface)
    {
        BT_DBG_ERROR(BT_DEBUG_HFP, "g_bt_hfclient_ex_interface not init\n");
        return BT_ERR_STATUS_PARM_INVALID;
    }

    ret = g_bt_hfclient_ex_interface->disable();
    if (BT_STATUS_SUCCESS != ret)
    {
        BT_DBG_ERROR(BT_DEBUG_HFP, "disable failed! ret=%d\n", ret);
        return ret;
    }
    return ret;
}
#endif

int linuxbt_hfclient_init(void)
{
    INT32 ret = 0;

    g_bt_hfclient_interface = (bthf_client_interface_t *)linuxbt_gap_get_profile_interface(BT_PROFILE_HANDSFREE_CLIENT_ID);
    if (NULL == g_bt_hfclient_interface)
    {
        BT_DBG_ERROR(BT_DEBUG_HFP, "Failed to get HFP Client interface\n");
        return BT_ERR_STATUS_FAIL;
    }

    ret = g_bt_hfclient_interface->init(&g_bt_hfclient_callbacks);
    BT_DBG_MINOR(BT_DEBUG_HFP, "g_bt_hfclient_interface init ret=%d\n", ret);

#if defined(MTK_LINUX_HFP) && (MTK_LINUX_HFP == TRUE)
    g_bt_hfclient_ex_interface = (bthf_client_ex_interface_t *)linuxbt_gap_get_profile_interface(BT_PROFILE_HANDSFREE_CLIENT_EX_ID);
    if (NULL == g_bt_hfclient_ex_interface)
    {
        BT_DBG_ERROR(BT_DEBUG_HFP, "Failed to get HFP Client Ex interface\n");
        return BT_ERR_STATUS_FAIL;
    }

    ret = g_bt_hfclient_ex_interface->init_ex(&g_bt_hfclient_ex_callbacks);
    BT_DBG_MINOR(BT_DEBUG_HFP, "g_bt_hfclient_ex_interface init_ex ret=%d\n", ret);
#endif

    return BT_STATUS_SUCCESS;
}

int linuxbt_hfclient_deinit(void)
{
    if (NULL == g_bt_hfclient_interface)
    {
        BT_DBG_ERROR(BT_DEBUG_HFP, "g_bt_hfclient_interface not init\n");
        return BT_ERR_STATUS_PARM_INVALID;
    }

    g_bt_hfclient_interface->cleanup();

#if defined(MTK_LINUX_HFP) && (MTK_LINUX_HFP == TRUE)
    if (NULL == g_bt_hfclient_ex_interface)
    {
        BT_DBG_ERROR(BT_DEBUG_HFP, "g_bt_hfclient_ex_interface not init\n");
        return BT_ERR_STATUS_PARM_INVALID;
    }

    g_bt_hfclient_ex_interface->cleanup_ex();
#endif
    return BT_SUCCESS;
}

int linuxbt_hfclient_connect(char *bt_addr)
{
    bt_bdaddr_t bdaddr;
    bt_status_t ret = BT_STATUS_SUCCESS;

    if (NULL == g_bt_hfclient_interface)
    {
        BT_DBG_ERROR(BT_DEBUG_HFP, "g_bt_hfclient_interface not init\n");
        return BT_ERR_STATUS_PARM_INVALID;
    }

    if (NULL == bt_addr)
    {
        BT_DBG_ERROR(BT_DEBUG_HFP, "null pointer\n");
        return BT_ERR_STATUS_PARM_INVALID;
    }

    linuxbt_btaddr_stoh(bt_addr, &bdaddr);
    BT_DBG_INFO(BT_DEBUG_HFP, "HFClient connect to %02X:%02X:%02X:%02X:%02X:%02X\n",
                bdaddr.address[0], bdaddr.address[1], bdaddr.address[2],
                bdaddr.address[3], bdaddr.address[4], bdaddr.address[5]);

    ret = g_bt_hfclient_interface->connect(&bdaddr);
    BT_DBG_MINOR(BT_DEBUG_HFP, "connect ret=%d\n", ret);
    return BT_STATUS_SUCCESS;
}

int linuxbt_hfclient_disconnect(char *bt_addr)
{
    bt_bdaddr_t bdaddr;
    bt_status_t ret = BT_STATUS_SUCCESS;

    if (NULL == g_bt_hfclient_interface)
    {
        BT_DBG_ERROR(BT_DEBUG_HFP, "g_bt_hfclient_interface not init\n");
        return BT_ERR_STATUS_PARM_INVALID;
    }

    if (NULL == bt_addr)
    {
        BT_DBG_ERROR(BT_DEBUG_HFP, "null pointer\n");
        return BT_ERR_STATUS_PARM_INVALID;
    }

    linuxbt_btaddr_stoh(bt_addr, &bdaddr);
    BT_DBG_INFO(BT_DEBUG_HFP, "HFClient disconnect to %02X:%02X:%02X:%02X:%02X:%02X\n",
                bdaddr.address[0], bdaddr.address[1], bdaddr.address[2],
                bdaddr.address[3], bdaddr.address[4], bdaddr.address[5]);

    ret = g_bt_hfclient_interface->disconnect(&bdaddr);
    BT_DBG_MINOR(BT_DEBUG_HFP, "disconnect ret=%d\n", ret);
    return BT_STATUS_SUCCESS;
}

int linuxbt_hfclient_connect_audio(char *bt_addr)
{
    bt_bdaddr_t bdaddr;
    bt_status_t ret = BT_STATUS_SUCCESS;

    if (NULL == g_bt_hfclient_interface)
    {
        BT_DBG_ERROR(BT_DEBUG_HFP, "g_bt_hfclient_interface not init\n");
        return BT_ERR_STATUS_PARM_INVALID;
    }

    if (NULL == bt_addr)
    {
        BT_DBG_ERROR(BT_DEBUG_HFP, "null pointer\n");
        return BT_ERR_STATUS_PARM_INVALID;
    }

    linuxbt_btaddr_stoh(bt_addr, &bdaddr);
    BT_DBG_INFO(BT_DEBUG_HFP, "HFClient connect audio to %02X:%02X:%02X:%02X:%02X:%02X\n",
                bdaddr.address[0], bdaddr.address[1], bdaddr.address[2],
                bdaddr.address[3], bdaddr.address[4], bdaddr.address[5]);

    ret = g_bt_hfclient_interface->connect_audio(&bdaddr);
    BT_DBG_MINOR(BT_DEBUG_HFP, "connect_audio ret=%d\n", ret);
    return BT_STATUS_SUCCESS;
}

int linuxbt_hfclient_disconnect_audio(char *bt_addr)
{
    bt_bdaddr_t bdaddr;
    bt_status_t ret = BT_STATUS_SUCCESS;

    if (NULL == g_bt_hfclient_interface)
    {
        BT_DBG_ERROR(BT_DEBUG_HFP, "g_bt_hfclient_interface not init\n");
        return BT_ERR_STATUS_PARM_INVALID;
    }

    if (NULL == bt_addr)
    {
        BT_DBG_ERROR(BT_DEBUG_HFP, "null pointer\n");
        return BT_ERR_STATUS_PARM_INVALID;
    }

    linuxbt_btaddr_stoh(bt_addr, &bdaddr);
    BT_DBG_INFO(BT_DEBUG_HFP, "HFClient disconnect audio to %02X:%02X:%02X:%02X:%02X:%02X\n",
                bdaddr.address[0], bdaddr.address[1], bdaddr.address[2],
                bdaddr.address[3], bdaddr.address[4], bdaddr.address[5]);

    ret = g_bt_hfclient_interface->disconnect_audio(&bdaddr);
    BT_DBG_MINOR(BT_DEBUG_HFP, "disconnect_audio ret=%d\n", ret);
    return BT_STATUS_SUCCESS;
}

int linuxbt_hfclient_start_voice_recognition(void)
{
    bt_status_t ret = BT_STATUS_SUCCESS;

    if (NULL == g_bt_hfclient_interface)
    {
        BT_DBG_ERROR(BT_DEBUG_HFP, "g_bt_hfclient_interface not init\n");
        return BT_ERR_STATUS_PARM_INVALID;
    }

    ret = g_bt_hfclient_interface->start_voice_recognition();
    BT_DBG_MINOR(BT_DEBUG_HFP, "start_voice_recognition ret=%d\n", ret);
    return BT_STATUS_SUCCESS;
}

int linuxbt_hfclient_stop_voice_recognition(void)
{
    bt_status_t ret = BT_STATUS_SUCCESS;

    if (NULL == g_bt_hfclient_interface)
    {
        BT_DBG_ERROR(BT_DEBUG_HFP, "g_bt_hfclient_interface not init\n");
        return BT_ERR_STATUS_PARM_INVALID;
    }

    ret = g_bt_hfclient_interface->stop_voice_recognition();
    BT_DBG_MINOR(BT_DEBUG_HFP, "stop_voice_recognition ret=%d\n", ret);
    return BT_STATUS_SUCCESS;
}

int linuxbt_hfclient_volume_control(bthf_client_volume_type_t type, int volume)
{
    bt_status_t ret = BT_STATUS_SUCCESS;

    if (NULL == g_bt_hfclient_interface)
    {
        BT_DBG_ERROR(BT_DEBUG_HFP, "g_bt_hfclient_interface not init\n");
        return BT_ERR_STATUS_PARM_INVALID;
    }

    ret = g_bt_hfclient_interface->volume_control(type, volume);
    BT_DBG_MINOR(BT_DEBUG_HFP, "volume_control(type=%d, volume=%d) ret=%d\n", type, volume, ret);
    return BT_STATUS_SUCCESS;
}

int linuxbt_hfclient_dial(const char *number)
{
    bt_status_t ret = BT_STATUS_SUCCESS;

    if (NULL == g_bt_hfclient_interface)
    {
        BT_DBG_ERROR(BT_DEBUG_HFP, "g_bt_hfclient_interface not init\n");
        return BT_ERR_STATUS_PARM_INVALID;
    }

    ret = g_bt_hfclient_interface->dial(number);
    BT_DBG_MINOR(BT_DEBUG_HFP, "dial(number=%s) ret=%d\n", (number != NULL) ? number : "", ret);
    return BT_STATUS_SUCCESS;
}

int linuxbt_hfclient_dial_memory(int location)
{
    bt_status_t ret = BT_STATUS_SUCCESS;

    if (NULL == g_bt_hfclient_interface)
    {
        BT_DBG_ERROR(BT_DEBUG_HFP, "g_bt_hfclient_interface not init\n");
        return BT_ERR_STATUS_PARM_INVALID;
    }

    ret = g_bt_hfclient_interface->dial_memory(location);
    BT_DBG_MINOR(BT_DEBUG_HFP, "dial_memory(location=%d) ret=%d\n", location, ret);
    return BT_STATUS_SUCCESS;
}

int linuxbt_hfclient_handle_call_action(bthf_client_call_action_t action, int idx)
{
    bt_status_t ret = BT_STATUS_SUCCESS;

    if (NULL == g_bt_hfclient_interface)
    {
        BT_DBG_ERROR(BT_DEBUG_HFP, "g_bt_hfclient_interface not init\n");
        return BT_ERR_STATUS_PARM_INVALID;
    }

    ret = g_bt_hfclient_interface->handle_call_action(action, idx);
    BT_DBG_MINOR(BT_DEBUG_HFP, "handle_call_action(action=%d, idx=%d) ret=%d\n", action, idx, ret);
    return BT_STATUS_SUCCESS;
}

int linuxbt_hfclient_query_current_calls(void)
{
    bt_status_t ret = BT_STATUS_SUCCESS;

    if (NULL == g_bt_hfclient_interface)
    {
        BT_DBG_ERROR(BT_DEBUG_HFP, "g_bt_hfclient_interface not init\n");
        return BT_ERR_STATUS_PARM_INVALID;
    }

    ret = g_bt_hfclient_interface->query_current_calls();
    BT_DBG_MINOR(BT_DEBUG_HFP, "query_current_calls ret=%d\n", ret);
    return BT_STATUS_SUCCESS;
}

int linuxbt_hfclient_query_current_operator_name(void)
{
    bt_status_t ret = BT_STATUS_SUCCESS;

    if (NULL == g_bt_hfclient_interface)
    {
        BT_DBG_ERROR(BT_DEBUG_HFP, "g_bt_hfclient_interface not init\n");
        return BT_ERR_STATUS_PARM_INVALID;
    }

    ret = g_bt_hfclient_interface->query_current_operator_name();
    BT_DBG_MINOR(BT_DEBUG_HFP, "query_current_operator_name ret=%d\n", ret);
    return BT_STATUS_SUCCESS;
}

int linuxbt_hfclient_retrieve_subscriber_info(void)
{
    bt_status_t ret = BT_STATUS_SUCCESS;

    if (NULL == g_bt_hfclient_interface)
    {
        BT_DBG_ERROR(BT_DEBUG_HFP, "g_bt_hfclient_interface not init\n");
        return BT_ERR_STATUS_PARM_INVALID;
    }

    ret = g_bt_hfclient_interface->retrieve_subscriber_info();
    BT_DBG_MINOR(BT_DEBUG_HFP, "retrieve_subscriber_info ret=%d\n", ret);
    return BT_STATUS_SUCCESS;
}

int linuxbt_hfclient_send_dtmf(char code)
{
    bt_status_t ret = BT_STATUS_SUCCESS;

    if (NULL == g_bt_hfclient_interface)
    {
        BT_DBG_ERROR(BT_DEBUG_HFP, "g_bt_hfclient_interface not init\n");
        return BT_ERR_STATUS_PARM_INVALID;
    }

    ret = g_bt_hfclient_interface->send_dtmf(code);
    BT_DBG_MINOR(BT_DEBUG_HFP, "send_dtmf(code=%d) ret=%d\n", code, ret);
    return BT_STATUS_SUCCESS;
}

int linuxbt_hfclient_request_last_voice_tag_number(void)
{
    bt_status_t ret = BT_STATUS_SUCCESS;

    if (NULL == g_bt_hfclient_interface)
    {
        BT_DBG_ERROR(BT_DEBUG_HFP, "g_bt_hfclient_interface not init\n");
        return BT_ERR_STATUS_PARM_INVALID;
    }

    ret = g_bt_hfclient_interface->request_last_voice_tag_number();
    BT_DBG_MINOR(BT_DEBUG_HFP, "request_last_voice_tag_number ret=%d\n", ret);
    return BT_STATUS_SUCCESS;
}

int linuxbt_hfclient_send_at_cmd(int cmd, int val1, int val2, const char *arg)
{
    bt_status_t ret = BT_STATUS_SUCCESS;

    if (NULL == g_bt_hfclient_interface)
    {
        BT_DBG_ERROR(BT_DEBUG_HFP, "g_bt_hfclient_interface not init\n");
        return BT_ERR_STATUS_PARM_INVALID;
    }

    ret = g_bt_hfclient_interface->send_at_cmd(cmd, val1, val2, arg);
    BT_DBG_MINOR(BT_DEBUG_HFP, "send_at_cmd(cmd=%d, val1=%d, val2=%d, arg=%s) ret=%d\n", cmd, val1, val2, arg, ret);
    return BT_STATUS_SUCCESS;
}

#if defined(MTK_LINUX_HFP_PHONEBOOK) && (MTK_LINUX_HFP_PHONEBOOK == TRUE)
int linuxbt_hfclient_select_pb_storage()
{
     bt_status_t ret = BT_STATUS_SUCCESS;

    if (NULL == g_bt_hfclient_ex_interface)
    {
        BT_DBG_ERROR(BT_DEBUG_HFP, "g_bt_hfclient_ex_interface not init\n");
        return BT_ERR_STATUS_PARM_INVALID;
    }

    ret = g_bt_hfclient_ex_interface->select_pb_storage();
    return ret;
}

int linuxbt_hfclient_set_pb_storage(UINT8 storage_idx)
{
    bt_status_t ret = BT_STATUS_SUCCESS;

    if (NULL == g_bt_hfclient_ex_interface)
    {
        BT_DBG_ERROR(BT_DEBUG_HFP, "g_bt_hfclient_ex_interface not init\n");
        return BT_ERR_STATUS_PARM_INVALID;
    }

    ret = g_bt_hfclient_ex_interface->set_pb_storage(storage_idx);
    return ret;
}

int linuxbt_hfclient_set_charset(const char* charset)
{
    bt_status_t ret = BT_STATUS_SUCCESS;

    if (NULL == g_bt_hfclient_ex_interface)
    {
        BT_DBG_ERROR(BT_DEBUG_HFP, "g_bt_hfclient_ex_interface not init\n");
        return BT_ERR_STATUS_PARM_INVALID;
    }

    ret = g_bt_hfclient_ex_interface->set_charset(charset);
    return ret;
}

int linuxbt_hfclient_test_pb_entry(void)
{
    bt_status_t ret = BT_STATUS_SUCCESS;

    if (NULL == g_bt_hfclient_ex_interface)
    {
        BT_DBG_ERROR(BT_DEBUG_HFP, "g_bt_hfclient_ex_interface not init\n");
        return BT_ERR_STATUS_PARM_INVALID;
    }

    ret = g_bt_hfclient_ex_interface->read_pb_entry(0, 0);
    return ret;
}

int linuxbt_hfclient_read_pb_entry(UINT16 idx_min, UINT16 idx_max)
{
    bt_status_t ret = BT_STATUS_SUCCESS;

    if (NULL == g_bt_hfclient_ex_interface)
    {
        BT_DBG_ERROR(BT_DEBUG_HFP, "g_bt_hfclient_ex_interface not init\n");
        return BT_ERR_STATUS_PARM_INVALID;
    }

    ret = g_bt_hfclient_ex_interface->read_pb_entry(idx_min, idx_max);
    return ret;
}
#endif

static void linuxbt_hfclient_connection_state_cb(bthf_client_connection_state_t state,
    unsigned int peer_feat, unsigned int chld_feat, bt_bdaddr_t *bd_addr)
{
    BT_DBG_NORMAL(BT_DEBUG_HFP, "state=%d, peer_feat=%d, chld_feat=%d, addr=%02X:%02X:%02X:%02X:%02X:%02X\n",
                state, peer_feat, chld_feat,
                bd_addr->address[0], bd_addr->address[1], bd_addr->address[2],
                bd_addr->address[3], bd_addr->address[4], bd_addr->address[5]);

    tBTMW_MSG btmw_msg = {0};
    btmw_msg.hdr.event = BTMW_HFCLIENT_CONNECTION_CB_EVT;

    btmw_msg.data.hfclient_msg.data.connect_cb.state = (BT_HFCLIENT_CONNECTION_STATE_T)state;
    btmw_msg.data.hfclient_msg.data.connect_cb.peer_feat = peer_feat;
    btmw_msg.data.hfclient_msg.data.connect_cb.chld_feat = chld_feat;
    linuxbt_btaddr_htos(bd_addr, btmw_msg.data.hfclient_msg.data.connect_cb.addr);

    linuxbt_send_msg(&btmw_msg);
}

static void linuxbt_hfclient_audio_state_cb(bthf_client_audio_state_t state, bt_bdaddr_t *bd_addr)
{
    BT_DBG_NORMAL(BT_DEBUG_HFP, "state=%d, addr=%02X:%02X:%02X:%02X:%02X:%02X\n", state,
                bd_addr->address[0], bd_addr->address[1], bd_addr->address[2],
                bd_addr->address[3], bd_addr->address[4], bd_addr->address[5]);

    tBTMW_MSG btmw_msg = {0};
    btmw_msg.hdr.event = BTMW_HFCLIENT_AUDIO_CONNECTION_CB_EVT;

    btmw_msg.data.hfclient_msg.data.auido_connect_cb.state = (BT_HFCLIENT_AUDIO_STATE_T)state;
    linuxbt_btaddr_htos(bd_addr, btmw_msg.data.hfclient_msg.data.auido_connect_cb.addr);

    linuxbt_send_msg(&btmw_msg);
}

static void linuxbt_hfclient_vr_cmd_cb(bthf_client_vr_state_t state)
{
    BT_DBG_NORMAL(BT_DEBUG_HFP, "vr_state=%d\n", state);

    tBTMW_MSG btmw_msg = {0};
    btmw_msg.hdr.event = BTMW_HFCLIENT_BVRA_CB_EVT;

    btmw_msg.data.hfclient_msg.data.bvra_cb.vr_state = (BT_HFCLIENT_VR_STATE_T)state;

    linuxbt_send_msg(&btmw_msg);
}

static void linuxbt_hfclient_network_state_cb(bthf_client_network_state_t state)
{
    BT_DBG_INFO(BT_DEBUG_HFP, "network_state=%d\n", state);

    tBTMW_MSG btmw_msg = {0};
    btmw_msg.hdr.event = BTMW_HFCLIENT_IND_SERVICE_CB_EVT;

    btmw_msg.data.hfclient_msg.data.service_cb.network_state = (BT_HFCLIENT_NETWORK_STATE_T)state;

    linuxbt_send_msg(&btmw_msg);
}

static void linuxbt_hfclient_network_roaming_cb(bthf_client_service_type_t type)
{
    BT_DBG_INFO(BT_DEBUG_HFP, "service_type=%d\n", type);

    tBTMW_MSG btmw_msg = {0};
    btmw_msg.hdr.event = BTMW_HFCLIENT_IND_ROAM_CB_EVT;

    btmw_msg.data.hfclient_msg.data.roam_cb.service_type = (BT_HFCLIENT_SERVICE_TYPE_T)type;

    linuxbt_send_msg(&btmw_msg);
}

static void linuxbt_hfclient_network_signal_cb(int signal_strength)
{
    BT_DBG_INFO(BT_DEBUG_HFP, "signal_strength=%d\n", signal_strength);

    tBTMW_MSG btmw_msg = {0};
    btmw_msg.hdr.event = BTMW_HFCLIENT_IND_SIGNAL_CB_EVT;

    btmw_msg.data.hfclient_msg.data.signal_cb.signal_strength = signal_strength;

    linuxbt_send_msg(&btmw_msg);
}

static void linuxbt_hfclient_battery_level_cb(int battery_level)
{
    BT_DBG_INFO(BT_DEBUG_HFP, "battery_level=%d\n", battery_level);

    tBTMW_MSG btmw_msg = {0};
    btmw_msg.hdr.event = BTMW_HFCLIENT_IND_BATTCH_CB_EVT;

    btmw_msg.data.hfclient_msg.data.battery_cb.battery_level = battery_level;

    linuxbt_send_msg(&btmw_msg);
}

static void linuxbt_hfclient_current_operator_cb(const char *name)
{
    BT_DBG_INFO(BT_DEBUG_HFP, "operator name=%s\n", name);

    tBTMW_MSG btmw_msg = {0};
    btmw_msg.hdr.event = BTMW_HFCLIENT_COPS_CB_EVT;

    memset(btmw_msg.data.hfclient_msg.data.cops_cb.operator_name, 0, HFCLIENT_OPERATOR_NAME_LEN + 1);
    strncpy(btmw_msg.data.hfclient_msg.data.cops_cb.operator_name, name, HFCLIENT_OPERATOR_NAME_LEN + 1);
    btmw_msg.data.hfclient_msg.data.cops_cb.operator_name[HFCLIENT_OPERATOR_NAME_LEN] = '\0';
    linuxbt_send_msg(&btmw_msg);
}

static void linuxbt_hfclient_call_cb(bthf_client_call_t call)
{
    BT_DBG_INFO(BT_DEBUG_HFP, "call=%d\n", call);

    tBTMW_MSG btmw_msg = {0};
    btmw_msg.hdr.event = BTMW_HFCLIENT_IND_CALL_CB_EVT;

    btmw_msg.data.hfclient_msg.data.call_cb.call = call;

    linuxbt_send_msg(&btmw_msg);
}

static void linuxbt_hfclient_callsetup_cb(bthf_client_callsetup_t callsetup)
{
    BT_DBG_INFO(BT_DEBUG_HFP, "callsetup=%d\n", callsetup);

    tBTMW_MSG btmw_msg = {0};
    btmw_msg.hdr.event = BTMW_HFCLIENT_IND_CALLSETUP_CB_EVT;

    btmw_msg.data.hfclient_msg.data.callsetup_cb.callsetup = callsetup;

    linuxbt_send_msg(&btmw_msg);
}

static void linuxbt_hfclient_callheld_cb(bthf_client_callheld_t callheld)
{
    BT_DBG_INFO(BT_DEBUG_HFP, "callheld=%d\n", callheld);

    tBTMW_MSG btmw_msg = {0};
    btmw_msg.hdr.event = BTMW_HFCLIENT_IND_CALLHELD_CB_EVT;

    btmw_msg.data.hfclient_msg.data.callheld_cb.callheld = callheld;

    linuxbt_send_msg(&btmw_msg);
}

static void linuxbt_hfclient_resp_and_hold_cb(bthf_client_resp_and_hold_t resp_and_hold)
{
    BT_DBG_INFO(BT_DEBUG_HFP, "resp_and_hold=%d\n", resp_and_hold);

    tBTMW_MSG btmw_msg = {0};
    btmw_msg.hdr.event = BTMW_HFCLIENT_BTRH_CB_EVT;

    btmw_msg.data.hfclient_msg.data.btrh_cb.resp_and_hold = resp_and_hold;

    linuxbt_send_msg(&btmw_msg);
}

static void linuxbt_hfclient_clip_cb(const char *number)
{
    BT_DBG_INFO(BT_DEBUG_HFP, "number=%s\n", number);

    tBTMW_MSG btmw_msg = {0};
    btmw_msg.hdr.event = BTMW_HFCLIENT_CLIP_CB_EVT;

    memset(btmw_msg.data.hfclient_msg.data.clip_cb.number, 0, HFCLIENT_NUMBER_LEN + 1);
    strncpy(btmw_msg.data.hfclient_msg.data.clip_cb.number, number, HFCLIENT_NUMBER_LEN + 1);
    btmw_msg.data.hfclient_msg.data.clip_cb.number[HFCLIENT_NUMBER_LEN] = '\0';

    linuxbt_send_msg(&btmw_msg);
}

static void linuxbt_hfclient_call_waiting_cb(const char *number)
{
    BT_DBG_INFO(BT_DEBUG_HFP, "number=%s\n", number);

    tBTMW_MSG btmw_msg = {0};
    btmw_msg.hdr.event = BTMW_HFCLIENT_CCWA_CB_EVT;

    memset(btmw_msg.data.hfclient_msg.data.ccwa_cb.number, 0, HFCLIENT_NUMBER_LEN + 1);
    strncpy(btmw_msg.data.hfclient_msg.data.ccwa_cb.number, number, HFCLIENT_NUMBER_LEN + 1);
    btmw_msg.data.hfclient_msg.data.ccwa_cb.number[HFCLIENT_NUMBER_LEN] = '\0';

    linuxbt_send_msg(&btmw_msg);
}

static void linuxbt_hfclient_current_calls_cb(int index, bthf_client_call_direction_t dir,
                                           bthf_client_call_state_t state,
                                           bthf_client_call_mpty_type_t mpty,
                                           const char *number)
{
    BT_DBG_INFO(BT_DEBUG_HFP, "index=%d, dir=%d, state=%d, mpty=%d, number=%s\n",
                              index, dir, state, mpty, number);

    tBTMW_MSG btmw_msg = {0};
    btmw_msg.hdr.event = BTMW_HFCLIENT_CLCC_CB_EVT;

    btmw_msg.data.hfclient_msg.data.clcc_cb.index = index;
    btmw_msg.data.hfclient_msg.data.clcc_cb.dir = (BT_HFCLIENT_CALL_DIRECTION_T)dir;
    btmw_msg.data.hfclient_msg.data.clcc_cb.state = (BT_HFCLIENT_CALL_STATE_T)state;
    btmw_msg.data.hfclient_msg.data.clcc_cb.mpty = (BT_HFCLIENT_CALL_MPTY_TYPE_T)mpty;

    memset(btmw_msg.data.hfclient_msg.data.clcc_cb.number, 0, HFCLIENT_NUMBER_LEN + 1);
    strncpy(btmw_msg.data.hfclient_msg.data.clcc_cb.number, number, HFCLIENT_NUMBER_LEN + 1);
    btmw_msg.data.hfclient_msg.data.clcc_cb.number[HFCLIENT_NUMBER_LEN] = '\0';

    linuxbt_send_msg(&btmw_msg);
}

static void linuxbt_hfclient_volume_change_cb(bthf_client_volume_type_t type, int volume)
{
    BT_DBG_INFO(BT_DEBUG_HFP, "type=%d, volume=%d\n", type, volume);

    tBTMW_MSG btmw_msg = {0};
    btmw_msg.hdr.event = BTMW_HFCLIENT_VGM_VGS_CB_EVT;

    btmw_msg.data.hfclient_msg.data.volume_cb.type = type;
    btmw_msg.data.hfclient_msg.data.volume_cb.volume = volume;

    linuxbt_send_msg(&btmw_msg);
}

static void linuxbt_hfclient_cmd_complete_cb(bthf_client_cmd_complete_t type, int cme)
{
    BT_DBG_INFO(BT_DEBUG_HFP, "type=%d, cme=%d\n", type, cme);

    tBTMW_MSG btmw_msg = {0};
    btmw_msg.hdr.event = BTMW_HFCLIENT_CMD_COMPLETE_CB_EVT;

    btmw_msg.data.hfclient_msg.data.cmd_complete_cb.type = type;
    btmw_msg.data.hfclient_msg.data.cmd_complete_cb.cme = cme;

    linuxbt_send_msg(&btmw_msg);
}

static void linuxbt_hfclient_subscriber_info_cb(const char *number,
                                                       bthf_client_subscriber_service_type_t type)
{
    BT_DBG_INFO(BT_DEBUG_HFP, "number=%s, type=%d\n", number, type);

    tBTMW_MSG btmw_msg = {0};
    btmw_msg.hdr.event = BTMW_HFCLIENT_CNUM_CB_EVT;

    memset(btmw_msg.data.hfclient_msg.data.cnum_cb.number, 0, HFCLIENT_NUMBER_LEN + 1);
    strncpy(btmw_msg.data.hfclient_msg.data.cnum_cb.number, number, HFCLIENT_NUMBER_LEN + 1);
    btmw_msg.data.hfclient_msg.data.cnum_cb.number[HFCLIENT_NUMBER_LEN] = '\0';
    btmw_msg.data.hfclient_msg.data.cnum_cb.type = type;

    linuxbt_send_msg(&btmw_msg);
}

static void linuxbt_hfclient_in_band_ring_tone_cb(bthf_client_in_band_ring_state_t state)
{
    BT_DBG_INFO(BT_DEBUG_HFP, "state=%d\n", state);

    tBTMW_MSG btmw_msg = {0};
    btmw_msg.hdr.event = BTMW_HFCLIENT_BSIR_CB_EVT;

    btmw_msg.data.hfclient_msg.data.bsir_cb.state = (BT_HFCLIENT_IN_BAND_RING_STATE_T)state;

    linuxbt_send_msg(&btmw_msg);
}

static void linuxbt_hfclient_last_voice_tag_number_cb(const char *number)
{
    BT_DBG_INFO(BT_DEBUG_HFP, "number=%s\n", number);

    tBTMW_MSG btmw_msg = {0};
    btmw_msg.hdr.event = BTMW_HFCLIENT_BINP_CB_EVT;

    memset(btmw_msg.data.hfclient_msg.data.binp_cb.number, 0, HFCLIENT_NUMBER_LEN + 1);
    strncpy(btmw_msg.data.hfclient_msg.data.binp_cb.number, number, HFCLIENT_NUMBER_LEN + 1);
    btmw_msg.data.hfclient_msg.data.binp_cb.number[HFCLIENT_NUMBER_LEN] = '\0';

    linuxbt_send_msg(&btmw_msg);
}

static void linuxbt_hfclient_ring_indication_cb(void)
{
    BT_DBG_INFO(BT_DEBUG_HFP, "\n");

    tBTMW_MSG btmw_msg = {0};
    btmw_msg.hdr.event = BTMW_HFCLIENT_RING_IND_CB_EVT;

    linuxbt_send_msg(&btmw_msg);
}


