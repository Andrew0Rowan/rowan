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

#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "btut_avrcp_if.h"
#include "util.h"

extern const void *btut_gap_get_profile_interface(const char *profile_id);

static void btut_rc_passthrough_rsp_cb(int id, int key_state);
static void btut_rc_groupnavigation_rsp_cb(int id, int key_state);
static void btut_rc_connection_state_cb(bool state, bt_bdaddr_t *bd_addr);
static void btut_rc_getrcfeatures_cb(bt_bdaddr_t *bd_addr, int features);
static void btut_rc_setplayerappsetting_rsp_cb(bt_bdaddr_t *bd_addr, uint8_t accepted);
static void btut_rc_playerapplicationsetting_cb(bt_bdaddr_t *bd_addr,
                                                                 uint8_t num_attr,
                                                                 btrc_player_app_attr_t *app_attrs,
                                                                 uint8_t num_ext_attr,
                                                                 btrc_player_app_ext_attr_t *ext_attrs);
static void btut_rc_playerapplicationsetting_changed_cb(bt_bdaddr_t *bd_addr,
                                                                          btrc_player_settings_t *p_vals);
static void btut_rc_setabsvol_cmd_cb(bt_bdaddr_t *bd_addr, uint8_t abs_vol, uint8_t label);
static void btut_rc_registernotification_absvol_cb(bt_bdaddr_t *bd_addr, uint8_t label);
static void btut_rc_track_changed_cb(bt_bdaddr_t *bd_addr, uint8_t num_attr,
                                                     btrc_element_attr_val_t *p_attrs);
static void btut_rc_play_position_changed_cb(bt_bdaddr_t *bd_addr,
                                                              uint32_t song_len, uint32_t song_pos);
static void btut_rc_play_status_changed_cb(bt_bdaddr_t *bd_addr,
                                                            btrc_play_status_t play_status);

static int btut_rc_send_play_handler(int argc, char *argv[]);
static int btut_rc_send_pause_handler(int argc, char *argv[]);
static int btut_rc_send_stop_handler(int argc, char *argv[]);
static int btut_rc_send_fwd_handler(int argc, char *argv[]);
static int btut_rc_send_bwd_handler(int argc, char *argv[]);
static int btut_rc_send_ffwd_handler(int argc, char *argv[]);
static int btut_rc_send_rwd_handler(int argc, char *argv[]);
static int btut_rc_send_volumeup_handler(int argc, char *argv[]);
static int btut_rc_send_volumedown_handler(int argc, char *argv[]);
static int btut_rc_send_next_group_handler(int argc, char *argv[]);
static int btut_rc_send_prev_group_handler(int argc, char *argv[]);
static int btut_rc_send_pass_through_cmd_handler(int argc, char *argv[]);

static btrc_ctrl_interface_t *g_bt_rc_ct_interface = NULL;
static btrc_ctrl_callbacks_t g_bt_rc_callbacks =
{
    sizeof(btrc_ctrl_callbacks_t),
    btut_rc_passthrough_rsp_cb,
    btut_rc_groupnavigation_rsp_cb,
    btut_rc_connection_state_cb,
    btut_rc_getrcfeatures_cb,
    btut_rc_setplayerappsetting_rsp_cb,
    btut_rc_playerapplicationsetting_cb,
    btut_rc_playerapplicationsetting_changed_cb,
    btut_rc_setabsvol_cmd_cb,
    btut_rc_registernotification_absvol_cb,
    btut_rc_track_changed_cb,
    btut_rc_play_position_changed_cb,
    btut_rc_play_status_changed_cb,
};

static BTUT_CLI bt_rc_cli_commands[] =
{
    {"play", btut_rc_send_play_handler,                                   " = send play command [addr]"},
    {"pause", btut_rc_send_pause_handler,                                 " = send pause command [addr]"},
    {"stop", btut_rc_send_stop_handler,                                   " = send stop command [addr]"},
    {"fwd", btut_rc_send_fwd_handler,                                     " = send fwd command [addr]"},
    {"bwd", btut_rc_send_bwd_handler,                                     " = send bwd command [addr]"},
    {"ffwd", btut_rc_send_ffwd_handler,                                   " = send ffwd command [addr]"},
    {"rwd", btut_rc_send_rwd_handler,                                     " = send rwd command [addr]"},
    {"volume_up", btut_rc_send_volumeup_handler,                          " = send volume up command [addr]"},
    {"volume_down", btut_rc_send_volumedown_handler,                      " = send volume down command [addr]"},
    {"next_group", btut_rc_send_next_group_handler,                       " = send next group command [addr]"},
    {"prev_group", btut_rc_send_prev_group_handler,                       " = send previous group command [addr]"},
    {"pass_through_cmd", btut_rc_send_pass_through_cmd_handler ,          " = pass_through_cmd [addr] [key_code] enter 0 for Next and 1 for Previous navigation command"},
    {NULL, NULL, NULL},
};

static void btut_rc_passthrough_rsp_cb(int id, int key_state)
{
    BTUT_Logd("[AVRCP] %s() id = 0x%x, key state = 0x%x\n", __func__, id, key_state);
}

static void btut_rc_groupnavigation_rsp_cb(int id, int key_state)
{
    BTUT_Logd("[AVRCP] %s() id = 0x%x, key state = 0x%x\n", __func__, id, key_state);
}

static void btut_rc_connection_state_cb(bool state, bt_bdaddr_t *bd_addr)
{
    if (NULL == bd_addr)
    {
        return;
    }

    BTUT_Logd("[AVRCP] %s() state = 0x%x, bd_addr = %02x:%02x:%02x:%02x:%02x:%02x\n", __func__, state,
        bd_addr->address[0], bd_addr->address[1], bd_addr->address[2],
        bd_addr->address[3], bd_addr->address[4], bd_addr->address[5]);
}

static void btut_rc_getrcfeatures_cb(bt_bdaddr_t *bd_addr, int features)
{
    if (NULL == bd_addr)
    {
        return;
    }

    BTUT_Logd("[AVRCP] %s() features = 0x%x, bd_addr = %02x:%02x:%02x:%02x:%02x:%02x\n", __func__, features,
        bd_addr->address[0], bd_addr->address[1], bd_addr->address[2],
        bd_addr->address[3], bd_addr->address[4], bd_addr->address[5]);
}

static void btut_rc_setplayerappsetting_rsp_cb(bt_bdaddr_t *bd_addr, uint8_t accepted)
{
    if (NULL == bd_addr)
    {
        return;
    }

    BTUT_Logd("[AVRCP] %s() accepted = 0x%x, bd_addr = %02x:%02x:%02x:%02x:%02x:%02x\n", __func__, accepted,
        bd_addr->address[0], bd_addr->address[1], bd_addr->address[2],
        bd_addr->address[3], bd_addr->address[4], bd_addr->address[5]);
}

static void btut_rc_playerapplicationsetting_cb(bt_bdaddr_t *bd_addr,
                                                                 uint8_t num_attr,
                                                                 btrc_player_app_attr_t *app_attrs,
                                                                 uint8_t num_ext_attr,
                                                                 btrc_player_app_ext_attr_t *ext_attrs)
{
    if (NULL == bd_addr)
    {
        return;
    }

    BTUT_Logd("[AVRCP] %s() num_attr = 0x%x, num_ext_attr = 0x%x, bd_addr = %02x:%02x:%02x:%02x:%02x:%02x\n",
        __func__, num_attr, num_ext_attr,
        bd_addr->address[0], bd_addr->address[1], bd_addr->address[2],
        bd_addr->address[3], bd_addr->address[4], bd_addr->address[5]);
}

static void btut_rc_playerapplicationsetting_changed_cb(bt_bdaddr_t *bd_addr,
                                                                          btrc_player_settings_t *p_vals)
{
    if (NULL == bd_addr)
    {
        return;
    }

    BTUT_Logd("[AVRCP] %s() bd_addr = %02x:%02x:%02x:%02x:%02x:%02x\n", __func__,
        bd_addr->address[0], bd_addr->address[1], bd_addr->address[2],
        bd_addr->address[3], bd_addr->address[4], bd_addr->address[5]);
}

static void btut_rc_setabsvol_cmd_cb(bt_bdaddr_t *bd_addr, uint8_t abs_vol, uint8_t label)
{
    if (NULL == bd_addr)
    {
        return;
    }

    BTUT_Logd("[AVRCP] %s() abs_vol = 0x%x, label = 0x%x, bd_addr = %02x:%02x:%02x:%02x:%02x:%02x\n",
        __func__, abs_vol, label,
        bd_addr->address[0], bd_addr->address[1], bd_addr->address[2],
        bd_addr->address[3], bd_addr->address[4], bd_addr->address[5]);
}

static void btut_rc_registernotification_absvol_cb(bt_bdaddr_t *bd_addr, uint8_t label)
{
    if (NULL == bd_addr)
    {
        return;
    }

    BTUT_Logd("[AVRCP] %s() label = 0x%x, bd_addr = %02x:%02x:%02x:%02x:%02x:%02x\n", __func__, label,
        bd_addr->address[0], bd_addr->address[1], bd_addr->address[2],
        bd_addr->address[3], bd_addr->address[4], bd_addr->address[5]);
}

static void btut_rc_track_changed_cb(bt_bdaddr_t *bd_addr, uint8_t num_attr,
                                                     btrc_element_attr_val_t *p_attrs)
{
    if (NULL == bd_addr)
    {
        return;
    }

    BTUT_Logd("[AVRCP] %s() num_attr = 0x%x, bd_addr = %02x:%02x:%02x:%02x:%02x:%02x\n", __func__, num_attr,
        bd_addr->address[0], bd_addr->address[1], bd_addr->address[2],
        bd_addr->address[3], bd_addr->address[4], bd_addr->address[5]);
}

static void btut_rc_play_position_changed_cb(bt_bdaddr_t *bd_addr,
                                                              uint32_t song_len, uint32_t song_pos)
{
    if (NULL == bd_addr)
    {
        return;
    }

    BTUT_Logd("[AVRCP] %s() song_len = 0x%x, song_pos = 0x%x, bd_addr = %02x:%02x:%02x:%02x:%02x:%02x\n",
        __func__, song_len, song_pos,
        bd_addr->address[0], bd_addr->address[1], bd_addr->address[2],
        bd_addr->address[3], bd_addr->address[4], bd_addr->address[5]);
}

static void btut_rc_play_status_changed_cb(bt_bdaddr_t *bd_addr,
                                                            btrc_play_status_t play_status)
{
    if (NULL == bd_addr)
    {
        return;
    }

    BTUT_Logd("[AVRCP] %s() play_status = 0x%x, bd_addr = %02x:%02x:%02x:%02x:%02x:%02x\n",
        __func__, play_status,
        bd_addr->address[0], bd_addr->address[1], bd_addr->address[2],
        bd_addr->address[3], bd_addr->address[4], bd_addr->address[5]);
}

static int btut_rc_cmd_handler(int argc, char **argv)
{
    BTUT_CLI *cmd, *match = NULL;
    int ret = 0;
    int count = 0;

    BTUT_Logi("[AVRCP] CT argc: %d, argv[0]: %s\n", argc, argv[0]);

    cmd = bt_rc_cli_commands;
    while (cmd->cmd)
    {
        if (!strcmp(cmd->cmd, argv[0]))
        {
            match = cmd;
            count = 1;
            break;
        }
        cmd++;
    }

    if (count == 0)
    {
        btut_print_cmd_help(CMD_KEY_RC_CT, bt_rc_cli_commands);
        ret = -1;
    }
    else
    {
        match->handler(argc - 1, &argv[1]);
    }

    return ret;
}

int btut_rc_init()
{
    int ret = 0;
    BTUT_MOD rc_mod = {0};

    BTUT_Logi("[AVRCP] %s() \n", __func__);

    g_bt_rc_ct_interface = (btrc_ctrl_interface_t *)btut_gap_get_profile_interface(BT_PROFILE_AV_RC_CTRL_ID);

    if (g_bt_rc_ct_interface == NULL)
    {
        BTUT_Loge("[AVRCP] Failed to get AVRCP CT interface\n");
        return -1;
    }

    if (g_bt_rc_ct_interface->init(&g_bt_rc_callbacks) != BT_STATUS_SUCCESS)
    {
        BTUT_Loge("[AVRCP] Failed to init AVRCP CT interface\n");
        return -1;
    }

    rc_mod.mod_id = BTUT_MOD_AVRCP_CT;
    strncpy(rc_mod.cmd_key, CMD_KEY_RC_CT, sizeof(rc_mod.cmd_key));
    rc_mod.cmd_handler = btut_rc_cmd_handler;
    rc_mod.cmd_tbl = bt_rc_cli_commands;

    ret = btut_register_mod(&rc_mod);
    BTUT_Logi("[AVRCP] btut_register_mod() fro CT returns: %d\n", ret);

    return ret;
}

int btut_rc_deinit()
{
    BTUT_Logi("[AVRCP] %s() \n", __func__);

    if (g_bt_rc_ct_interface == NULL)
    {
        BTUT_Loge("[AVRCP] Failed to get AVRCP CT interface\n");
        return -1;
    }

    g_bt_rc_ct_interface->cleanup();
    return 0;
}

static int btut_rc_send_play_handler(int argc, char *argv[])
{
    char *ptr;
    bt_bdaddr_t bdaddr;

    BTUT_Logi("[AVRCP] %s() \n", __func__);

    if (g_bt_rc_ct_interface == NULL)
    {
        BTUT_Loge("[AVRCP] Failed to get AVRCP CT interface\n");
        return -1;
    }

    ptr = argv[0];
    util_btaddr_stoh(ptr, &bdaddr);

    BTUT_Logi("AVRCP send play to %02X:%02X:%02X:%02X:%02X:%02X\n",
    bdaddr.address[0], bdaddr.address[1], bdaddr.address[2],
    bdaddr.address[3], bdaddr.address[4], bdaddr.address[5]);

    g_bt_rc_ct_interface->send_pass_through_cmd(&bdaddr, AVRCP_POP_PLAY, AVRCP_STATE_PRESSED);
    sleep(1);
    g_bt_rc_ct_interface->send_pass_through_cmd(&bdaddr, AVRCP_POP_PLAY, AVRCP_STATE_RELEASED);

    return 0;
}

static int btut_rc_send_pause_handler(int argc, char *argv[])
{
    char *ptr;
    bt_bdaddr_t bdaddr;

    BTUT_Logi("[AVRCP] %s() \n", __func__);

    if (g_bt_rc_ct_interface == NULL)
    {
        BTUT_Loge("[AVRCP] Failed to get AVRCP CT interface\n");
        return -1;
    }

    ptr = argv[0];
    util_btaddr_stoh(ptr, &bdaddr);

    BTUT_Logi("AVRCP send pause to %02X:%02X:%02X:%02X:%02X:%02X\n",
    bdaddr.address[0], bdaddr.address[1], bdaddr.address[2],
    bdaddr.address[3], bdaddr.address[4], bdaddr.address[5]);

    g_bt_rc_ct_interface->send_pass_through_cmd(&bdaddr, AVRCP_POP_PAUSE, AVRCP_STATE_PRESSED);
    sleep(1);
    g_bt_rc_ct_interface->send_pass_through_cmd(&bdaddr, AVRCP_POP_PAUSE, AVRCP_STATE_RELEASED);

    return 0;
}

static int btut_rc_send_stop_handler(int argc, char *argv[])
{
    char *ptr;
    bt_bdaddr_t bdaddr;

    BTUT_Logi("[AVRCP] %s() \n", __func__);

    if (g_bt_rc_ct_interface == NULL)
    {
        BTUT_Loge("[AVRCP] Failed to get AVRCP CT interface\n");
        return -1;
    }

    ptr = argv[0];
    util_btaddr_stoh(ptr, &bdaddr);

    BTUT_Logi("AVRCP send stop to %02X:%02X:%02X:%02X:%02X:%02X\n",
    bdaddr.address[0], bdaddr.address[1], bdaddr.address[2],
    bdaddr.address[3], bdaddr.address[4], bdaddr.address[5]);

    g_bt_rc_ct_interface->send_pass_through_cmd(&bdaddr, AVRCP_POP_STOP, AVRCP_STATE_PRESSED);
    sleep(1);
    g_bt_rc_ct_interface->send_pass_through_cmd(&bdaddr, AVRCP_POP_STOP, AVRCP_STATE_RELEASED);

    return 0;
}

static int btut_rc_send_fwd_handler(int argc, char *argv[])
{
    char *ptr;
    bt_bdaddr_t bdaddr;

    BTUT_Logi("[AVRCP] %s() \n", __func__);

    if (g_bt_rc_ct_interface == NULL)
    {
        BTUT_Loge("[AVRCP] Failed to get AVRCP CT interface\n");
        return -1;
    }

    ptr = argv[0];
    util_btaddr_stoh(ptr, &bdaddr);

    BTUT_Logi("AVRCP send fwd to %02X:%02X:%02X:%02X:%02X:%02X\n",
    bdaddr.address[0], bdaddr.address[1], bdaddr.address[2],
    bdaddr.address[3], bdaddr.address[4], bdaddr.address[5]);

    g_bt_rc_ct_interface->send_pass_through_cmd(&bdaddr, AVRCP_POP_FORWARD, AVRCP_STATE_PRESSED);
    sleep(1);
    g_bt_rc_ct_interface->send_pass_through_cmd(&bdaddr, AVRCP_POP_FORWARD, AVRCP_STATE_RELEASED);

    return 0;
}

static int btut_rc_send_bwd_handler(int argc, char *argv[])
{
    char *ptr;
    bt_bdaddr_t bdaddr;

    BTUT_Logi("[AVRCP] %s() \n", __func__);

    if (g_bt_rc_ct_interface == NULL)
    {
        BTUT_Loge("[AVRCP] Failed to get AVRCP CT interface\n");
        return -1;
    }

    ptr = argv[0];
    util_btaddr_stoh(ptr, &bdaddr);

    BTUT_Logi("AVRCP send bwd to %02X:%02X:%02X:%02X:%02X:%02X\n",
    bdaddr.address[0], bdaddr.address[1], bdaddr.address[2],
    bdaddr.address[3], bdaddr.address[4], bdaddr.address[5]);

    g_bt_rc_ct_interface->send_pass_through_cmd(&bdaddr, AVRCP_POP_BACKWARD, AVRCP_STATE_PRESSED);
    sleep(1);
    g_bt_rc_ct_interface->send_pass_through_cmd(&bdaddr, AVRCP_POP_BACKWARD, AVRCP_STATE_RELEASED);

    return 0;
}

static int btut_rc_send_ffwd_handler(int argc, char *argv[])
{
    char *ptr;
    bt_bdaddr_t bdaddr;

    BTUT_Logi("[AVRCP] %s() \n", __func__);

    if (g_bt_rc_ct_interface == NULL)
    {
        BTUT_Loge("[AVRCP] Failed to get AVRCP CT interface\n");
        return -1;
    }

    ptr = argv[0];
    util_btaddr_stoh(ptr, &bdaddr);

    BTUT_Logi("AVRCP send ffwd to %02X:%02X:%02X:%02X:%02X:%02X\n",
    bdaddr.address[0], bdaddr.address[1], bdaddr.address[2],
    bdaddr.address[3], bdaddr.address[4], bdaddr.address[5]);

    g_bt_rc_ct_interface->send_pass_through_cmd(&bdaddr, AVRCP_POP_FAST_FORWARD, AVRCP_STATE_PRESSED);
    sleep(1);
    g_bt_rc_ct_interface->send_pass_through_cmd(&bdaddr, AVRCP_POP_FAST_FORWARD, AVRCP_STATE_RELEASED);

    return 0;
}

static int btut_rc_send_rwd_handler(int argc, char *argv[])
{
    char *ptr;
    bt_bdaddr_t bdaddr;

    BTUT_Logi("[AVRCP] %s() \n", __func__);

    if (g_bt_rc_ct_interface == NULL)
    {
        BTUT_Loge("[AVRCP] Failed to get AVRCP CT interface\n");
        return -1;
    }

    ptr = argv[0];
    util_btaddr_stoh(ptr, &bdaddr);

    BTUT_Logi("AVRCP send rwd to %02X:%02X:%02X:%02X:%02X:%02X\n",
    bdaddr.address[0], bdaddr.address[1], bdaddr.address[2],
    bdaddr.address[3], bdaddr.address[4], bdaddr.address[5]);

    g_bt_rc_ct_interface->send_pass_through_cmd(&bdaddr, AVRCP_POP_REWIND, AVRCP_STATE_PRESSED);
    sleep(1);
    g_bt_rc_ct_interface->send_pass_through_cmd(&bdaddr, AVRCP_POP_REWIND, AVRCP_STATE_RELEASED);

    return 0;
}

static int btut_rc_send_volumeup_handler(int argc, char *argv[])
{
    char *ptr;
    bt_bdaddr_t bdaddr;

    BTUT_Logi("[AVRCP] %s() \n", __func__);

    if (g_bt_rc_ct_interface == NULL)
    {
        BTUT_Loge("[AVRCP] Failed to get AVRCP CT interface\n");
        return -1;
    }

    ptr = argv[0];
    util_btaddr_stoh(ptr, &bdaddr);

    BTUT_Logi("AVRCP send volume up to %02X:%02X:%02X:%02X:%02X:%02X\n",
    bdaddr.address[0], bdaddr.address[1], bdaddr.address[2],
    bdaddr.address[3], bdaddr.address[4], bdaddr.address[5]);

    g_bt_rc_ct_interface->send_pass_through_cmd(&bdaddr, AVRCP_POP_VOLUME_UP, AVRCP_STATE_PRESSED);
    sleep(1);
    g_bt_rc_ct_interface->send_pass_through_cmd(&bdaddr, AVRCP_POP_VOLUME_UP, AVRCP_STATE_RELEASED);

    return 0;
}

static int btut_rc_send_volumedown_handler(int argc, char *argv[])
{
    char *ptr;
    bt_bdaddr_t bdaddr;

    BTUT_Logi("[AVRCP] %s() \n", __func__);

    if (g_bt_rc_ct_interface == NULL)
    {
        BTUT_Loge("[AVRCP] Failed to get AVRCP CT interface\n");
        return -1;
    }

    ptr = argv[0];
    util_btaddr_stoh(ptr, &bdaddr);

    BTUT_Logi("AVRCP send volume down to %02X:%02X:%02X:%02X:%02X:%02X\n",
    bdaddr.address[0], bdaddr.address[1], bdaddr.address[2],
    bdaddr.address[3], bdaddr.address[4], bdaddr.address[5]);

    g_bt_rc_ct_interface->send_pass_through_cmd(&bdaddr, AVRCP_POP_VOLUME_DOWN, AVRCP_STATE_PRESSED);
    sleep(1);
    g_bt_rc_ct_interface->send_pass_through_cmd(&bdaddr, AVRCP_POP_VOLUME_DOWN, AVRCP_STATE_RELEASED);

    return 0;
}

static int btut_rc_send_next_group_handler(int argc, char *argv[])
{
    char *ptr;
    bt_bdaddr_t bdaddr;

    BTUT_Logi("[AVRCP] %s() \n", __func__);

    if (g_bt_rc_ct_interface == NULL)
    {
        BTUT_Loge("[AVRCP] Failed to get AVRCP CT interface\n");
        return -1;
    }

    ptr = argv[0];
    util_btaddr_stoh(ptr, &bdaddr);

    BTUT_Logi("AVRCP send next group to %02X:%02X:%02X:%02X:%02X:%02X\n",
    bdaddr.address[0], bdaddr.address[1], bdaddr.address[2],
    bdaddr.address[3], bdaddr.address[4], bdaddr.address[5]);

    g_bt_rc_ct_interface->send_pass_through_cmd(&bdaddr, AVRCP_POP_NEXT_GROUP, AVRCP_STATE_PRESSED);
    sleep(1);
    g_bt_rc_ct_interface->send_pass_through_cmd(&bdaddr, AVRCP_POP_NEXT_GROUP, AVRCP_STATE_RELEASED);

    return 0;
}

static int btut_rc_send_prev_group_handler(int argc, char *argv[])
{
    char *ptr;
    bt_bdaddr_t bdaddr;

    BTUT_Logi("[AVRCP] %s() \n", __func__);

    if (g_bt_rc_ct_interface == NULL)
    {
        BTUT_Loge("[AVRCP] Failed to get AVRCP CT interface\n");
        return -1;
    }

    ptr = argv[0];
    util_btaddr_stoh(ptr, &bdaddr);

    BTUT_Logi("AVRCP send prev group to %02X:%02X:%02X:%02X:%02X:%02X\n",
    bdaddr.address[0], bdaddr.address[1], bdaddr.address[2],
    bdaddr.address[3], bdaddr.address[4], bdaddr.address[5]);

    g_bt_rc_ct_interface->send_pass_through_cmd(&bdaddr, AVRCP_POP_PREV_GROUP, AVRCP_STATE_PRESSED);
    sleep(1);
    g_bt_rc_ct_interface->send_pass_through_cmd(&bdaddr, AVRCP_POP_PREV_GROUP, AVRCP_STATE_RELEASED);

    return 0;
}

static int btut_rc_send_pass_through_cmd_handler(int argc, char *argv[])
{
    int local_value;
    char *ptr;
    bt_bdaddr_t bdaddr;

    if (g_bt_rc_ct_interface == NULL)
    {
        return -1;
    }

   ptr = argv[0];
   util_btaddr_stoh(ptr, &bdaddr);

   BTUT_Logi("AVRCP send pass through cmd to %02X:%02X:%02X:%02X:%02X:%02X\n",
   bdaddr.address[0], bdaddr.address[1], bdaddr.address[2],
   bdaddr.address[3], bdaddr.address[4], bdaddr.address[5]);

    local_value = atoi(argv[1]);
    g_bt_rc_ct_interface->send_pass_through_cmd(&bdaddr, local_value, AVRCP_STATE_PRESSED);
    sleep(1);
    g_bt_rc_ct_interface->send_pass_through_cmd(&bdaddr, local_value, AVRCP_STATE_RELEASED);

    return 0;
}
