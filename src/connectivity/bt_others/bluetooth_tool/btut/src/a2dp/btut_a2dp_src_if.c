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
#include <hardware/bluetooth.h>
#include <hardware/bt_av.h>
#include <hardware/bt_rc.h>

#include "btut_cli.h"
#include "btut_debug.h"
#include "btut_a2dp_src_if.h"
#include "btut_gap_if.h"
#include "util.h"

extern const void *btut_gap_get_profile_interface(const char *profile_id);

// Callback functions
static void btut_a2dp_src_connection_state_cb(btav_connection_state_t state, bt_bdaddr_t *bd_addr);
static void btut_a2dp_src_audio_state_cb(btav_audio_state_t state, bt_bdaddr_t *bd_addr);
static void btut_a2dp_src_audio_config_cb(bt_bdaddr_t *bd_addr, uint32_t sample_rate, uint8_t channel_count);

// CLI handler
static int btut_a2dp_src_connect_int_handler(int argc, char *argv[]);
static int btut_a2dp_src_disconnect_handler(int argc, char *argv[]);

static btav_interface_t *g_bt_a2dp_src_interface = NULL;
static btav_callbacks_t g_bt_a2dp_src_callbacks =
{
    sizeof(btav_callbacks_t),
    btut_a2dp_src_connection_state_cb,
    btut_a2dp_src_audio_state_cb,
    btut_a2dp_src_audio_config_cb,
};

static BTUT_CLI bt_a2dp_src_cli_commands[] =
{
    {"connect",                      btut_a2dp_src_connect_int_handler,                     " = connect ([addr])"},
    {"disconnect",                btut_a2dp_src_disconnect_handler,                       " = disconnect ([addr])"},
    {NULL, NULL, NULL},
};

static void btut_a2dp_src_connection_state_cb(btav_connection_state_t state, bt_bdaddr_t *bd_addr)
{
    BTUT_Logd("%s", __func__);
    BTUT_Logi("[A2DP] state: %s(%d) ",
            (state == BTAV_CONNECTION_STATE_DISCONNECTED) ? "SIGNALLING CHANNEL DISCONNECTED" :
            (state == BTAV_CONNECTION_STATE_CONNECTING) ? "CONNECTING" :
            (state == BTAV_CONNECTION_STATE_CONNECTED) ? "SIGNALLING CHANNEL CONNECTED" :
            (state == BTAV_CONNECTION_STATE_DISCONNECTING) ? "DISCONNECTING" : "UNKNOWN",
            state);
}

static void btut_a2dp_src_audio_state_cb(btav_audio_state_t state, bt_bdaddr_t *bd_addr)
{
    BTUT_Logd("%s", __func__);
    BTUT_Logi("[A2DP] state: %s(%d) ",
            (state == BTAV_AUDIO_STATE_STARTED) ? "STARTED" :
            (state == BTAV_AUDIO_STATE_STOPPED) ? "STOPPED" :
            (state == BTAV_AUDIO_STATE_REMOTE_SUSPEND) ? "SUSPEND" : "UNKNOWN",
            state);
    BTUT_Logi("BDADDR = %02X:%02X:%02X:%02X:%02X:%02X\n",
        bd_addr->address[0], bd_addr->address[1], bd_addr->address[2],
        bd_addr->address[3], bd_addr->address[4], bd_addr->address[5]);
}

static void btut_a2dp_src_audio_config_cb(bt_bdaddr_t *bd_addr, uint32_t sample_rate, uint8_t channel_mode)
{
    BTUT_Logd("%s", __func__);
    BTUT_Logi("[A2DP] sample_rate: %d, channel_mode: %d", sample_rate, channel_mode);

    if (sample_rate & 0x10)
    {
        BTUT_Logi(" 48000Hz");
    }
    if (sample_rate & 0x20)
    {
        BTUT_Logi(" 44100Hz");
    }
    if (channel_mode& 0x1)
    {
        BTUT_Logi(" stereo");
    }
    if (channel_mode& 0x2)
    {
        BTUT_Logi(" dual channel");
    }
    if (channel_mode & 0x4)
    {
        BTUT_Logi(" mono");
    }
}

static int btut_a2dp_src_connect_int_handler(int argc, char *argv[])
{
    char *ptr;
    bt_bdaddr_t bdaddr;

    BTUT_Logi("[A2DP] %s()\n", __func__);

    if (argc < 1)
    {
        BTUT_Loge("[A2DP] [USAGE] A2DP_connect [BD_ADDR] ");
        return -1;
    }

    ptr = argv[0];
    util_btaddr_stoh(ptr, &bdaddr);
    BTUT_Logi("A2DP connected to %02X:%02X:%02X:%02X:%02X:%02X\n",
            bdaddr.address[0], bdaddr.address[1], bdaddr.address[2],
            bdaddr.address[3], bdaddr.address[4], bdaddr.address[5]);

    g_bt_a2dp_src_interface->connect(&bdaddr);

    return 0;
}

static int btut_a2dp_src_disconnect_handler(int argc, char *argv[])
{
    char *ptr;
    bt_bdaddr_t bdaddr;

    BTUT_Logi("[A2DP] %s()\n", __func__);

    if (argc < 1)
    {
        return -1;
    }

    ptr = argv[0];
    util_btaddr_stoh(ptr, &bdaddr);
    BTUT_Logi("A2DP disconnected to %02X:%02X:%02X:%02X:%02X:%02X\n",
            bdaddr.address[0], bdaddr.address[1], bdaddr.address[2],
            bdaddr.address[3], bdaddr.address[4], bdaddr.address[5]);

    g_bt_a2dp_src_interface->disconnect(&bdaddr);
    return 0;
}

int btut_a2dp_src_cmd_handler(int argc, char **argv)
{
    BTUT_CLI *cmd, *match = NULL;
    int ret = 0;
    int count = 0;

    BTUT_Logd("[A2DP] %s argc: %d, argv[0]: %s\n", __func__, argc, argv[0]);

    cmd = bt_a2dp_src_cli_commands;
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
        BTUT_Logd("[A2DP] Unknown command '%s'\n", argv[0]);

        btut_print_cmd_help(CMD_KEY_A2DP_SRC, bt_a2dp_src_cli_commands);
        ret = -1;
    }
    else
    {
        match->handler(argc - 1, &argv[1]);
    }

    return ret;
}

int btut_a2dp_src_init()
{
    int ret = 0;
    BTUT_MOD a2dp_src_mod = {0};

    g_bt_a2dp_src_interface = (btav_interface_t *) btut_gap_get_profile_interface(BT_PROFILE_ADVANCED_AUDIO_ID);
    if (g_bt_a2dp_src_interface == NULL)
    {
        BTUT_Loge("[A2DP] Failed to get A2DP SRC interface\n");
        return -1;
    }

    if (g_bt_a2dp_src_interface->init(&g_bt_a2dp_src_callbacks) != BT_STATUS_SUCCESS)
    {
        BTUT_Loge("[A2DP] Failed to init A2DP SRC interface\n");
        return -1;
    }

    a2dp_src_mod.mod_id = BTUT_MOD_A2DP_SRC;
    strncpy(a2dp_src_mod.cmd_key, CMD_KEY_A2DP_SRC, sizeof(a2dp_src_mod.cmd_key));
    a2dp_src_mod.cmd_handler = btut_a2dp_src_cmd_handler;
    a2dp_src_mod.cmd_tbl = bt_a2dp_src_cli_commands;

    ret = btut_register_mod(&a2dp_src_mod);
    BTUT_Logd("[A2DP] btut_register_mod() for SRC returns: %d\n", ret);

    return ret;
}

int btut_a2dp_src_deinit()
{
    if (g_bt_a2dp_src_interface != NULL)
    {
        g_bt_a2dp_src_interface->cleanup();
    }

    return 0;
}
