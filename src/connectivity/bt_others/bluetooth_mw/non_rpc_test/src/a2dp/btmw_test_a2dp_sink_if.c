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
#include <pthread.h>
#include <stdio.h>
#include <string.h>

#include "btmw_test_cli.h"
#include "btmw_test_debug.h"
#include "c_bt_mw_a2dp_common.h"
#include "c_bt_mw_a2dp_snk.h"
#include "bt_a2dp_alsa_playback.h"

#define BTMW_TEST_CMD_KEY_A2DP_SINK        "MW_A2DP_SINK"

#define BTMW_TEST_A2DP_CASE_RETURN_STR(const) case const: return #const;

// CLI handler
static int btmw_test_a2dp_sink_connect_int_handler(int argc, char *argv[]);
static int btmw_test_a2dp_sink_disconnect_handler(int argc, char *argv[]);
static int btmw_test_a2dp_sink_start_play(int argc, char *argv[]);
static int btmw_test_a2dp_sink_stop_play(int argc, char *argv[]);
static int btmw_test_a2dp_sink_active_handler(int argc, char *argv[]);
static int btmw_test_a2dp_get_src_dev_list_handler(int argc, char *argv[]);


static BTMW_TEST_CLI btmw_test_a2dp_sink_cli_commands[] =
{
    {"connect",         btmw_test_a2dp_sink_connect_int_handler, " = connect <addr>"},
    {"disconnect",      btmw_test_a2dp_sink_disconnect_handler,  " = disconnect <addr>"},
    {"start_play",      btmw_test_a2dp_sink_start_play,          " = start_play "},
    {"stop_play",       btmw_test_a2dp_sink_stop_play,           " = stop_play"},
    {"active_sink",     btmw_test_a2dp_sink_active_handler,      " = active_sink <1:enable|0:disable>"},
    {"get_paired_dev",  btmw_test_a2dp_get_src_dev_list_handler, " = get_paired_dev"},
    {NULL, NULL, NULL},
};

CHAR g_a2dp_addr_test[18];
BT_A2DP_STREAM_STATE g_a2dp_stream_state = BT_A2DP_STREAM_STATE_SUSPEND;

static int btmw_test_a2dp_sink_start_play(int argc, char *argv[])
{
    c_btm_a2dp_sink_start_player();

    return 0;
}

static int btmw_test_a2dp_sink_stop_play(int argc, char *argv[])
{
    c_btm_a2dp_sink_stop_player();

    return 0;
}

static int btmw_test_a2dp_sink_active_handler(int argc, char *argv[])
{
    UINT8 u1_enable = 0;
    BTMW_TEST_Logi("%s()\n", __func__);

    if (argc != 1)
    {
        BTMW_TEST_Loge("[USAGE] active_sink [1:enable|0:disable]");
        return -1;
    }

    u1_enable = atoi(argv[0]);
    c_btm_a2dp_sink_enable(u1_enable);

    return 0;
}




static int btmw_test_a2dp_get_src_dev_list_handler(int argc, char *argv[])
{
    BT_A2DP_DEVICE_LIST device_list;
    INT32 ret = 0;
    BTMW_TEST_Logi("%s()\n", __func__);

    memset((void*)&device_list, 0, sizeof(device_list));

    ret = c_btm_a2dp_sink_get_dev_list(&device_list);
    BTMW_TEST_Logi("get paired src device list result:\n", __func__);
    if(BT_SUCCESS == ret)
    {
        if (0 == device_list.dev_num)
        {
            BTMW_TEST_Logi("no paired src device\n");
        }
        else
        {
            INT32 i = 0;
            for(i=0;i<device_list.dev_num;i++)
            {
                BTMW_TEST_Logi("device[%d]: %s\n", i, device_list.dev[i].addr);
                BTMW_TEST_Logi("======================================\n");
            }
        }
    }
    else
    {
        BTMW_TEST_Logi("get paired sink device failed: %d\n", ret);
    }

    return 0;
}




static int btmw_test_a2dp_sink_connect_int_handler(int argc, char *argv[])
{
    CHAR *ptr = NULL;

    BTMW_TEST_Logi("%s()\n", __func__);

    if (argc != 1)
    {
        BTMW_TEST_Loge("[USAGE] connect <addr>");
        return -1;
    }
    if (strlen(argv[0]) < 17)
    {
        BTMW_TEST_Loge("<addr> invalid. Good example is \"AA:BB:CC:DD:EE:FF\"");
        BTMW_TEST_Loge("[USAGE] connect <addr>");
        return -1;
    }

    ptr = argv[0];
    BTMW_TEST_Logi("A2DP connected to %s\n", ptr);
    c_btm_a2dp_connect(ptr, BT_A2DP_ROLE_SINK);

    return 0;
}

static int btmw_test_a2dp_sink_disconnect_handler(int argc, char *argv[])
{
    CHAR *ptr;
    BTMW_TEST_Logi("%s()\n", __func__);

    if (argc != 1)
    {
        BTMW_TEST_Loge("[USAGE] disconnect <addr>");
        return -1;
    }
    if (strlen(argv[0]) < 17)
    {
        BTMW_TEST_Loge("<addr> invalid. Good example is \"AA:BB:CC:DD:EE:FF\"");
        BTMW_TEST_Loge("[USAGE] disconnect <addr>");
    }

    ptr = argv[0];
    BTMW_TEST_Logi("A2DP disconnected to %s\n", ptr);
    c_btm_a2dp_disconnect(ptr);
    return 0;
}

static int  btmw_test_a2dp_sink_cmd_handler(int argc, char *argv[])
{
    BTMW_TEST_CLI *cmd, *match = NULL;
    INT32 ret = 0;
    INT32 count = 0;

    BTMW_TEST_Logd("%s argc: %d, argv[0]: %s\n", __func__, argc, argv[0]);

    cmd = btmw_test_a2dp_sink_cli_commands;
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
        BTMW_TEST_Logd("Unknown command '%s'\n", argv[0]);

        btmw_test_print_cmd_help(BTMW_TEST_CMD_KEY_A2DP_SINK, btmw_test_a2dp_sink_cli_commands);
        ret = -1;
    }
    else
    {
        match->handler(argc - 1, &argv[1]);
    }

    return ret;
}

static CHAR* btmw_test_a2dp_sink_app_event(BT_A2DP_EVENT event)
{
    switch((int)event)
    {
        BTMW_TEST_A2DP_CASE_RETURN_STR(BT_A2DP_EVENT_CONNECTED)
        BTMW_TEST_A2DP_CASE_RETURN_STR(BT_A2DP_EVENT_DISCONNECTED)
        BTMW_TEST_A2DP_CASE_RETURN_STR(BT_A2DP_EVENT_STREAM_SUSPEND)
        BTMW_TEST_A2DP_CASE_RETURN_STR(BT_A2DP_EVENT_STREAM_START)
        BTMW_TEST_A2DP_CASE_RETURN_STR(BT_A2DP_EVENT_CONNECT_COMING)
        BTMW_TEST_A2DP_CASE_RETURN_STR(BT_A2DP_EVENT_PLAYER_EVENT)
        default: return "UNKNOWN_EVENT";
   }
}

static VOID btmw_test_a2dp_sink_app_cbk(BT_A2DP_EVENT_PARAM *param)
{
    if (NULL == param)
    {
        BTMW_TEST_Loge("param is NULL\n");
        return;
    }

    BTMW_TEST_Logd("addr=%s, event=%d, %s\n", param->addr, param->event,
        btmw_test_a2dp_sink_app_event(param->event));
    switch (param->event)
    {
        case BT_A2DP_EVENT_CONNECTED:
            BTMW_TEST_Logd("A2DP connected(%s)\n", param->addr);
            BTMW_TEST_Logd("sample rate=%d, channel num=%d\n",
                param->data.connected_data.sample_rate,
                param->data.connected_data.channel_num);
            strcpy(g_a2dp_addr_test, param->addr);
            break;
        case BT_A2DP_EVENT_DISCONNECTED:
            BTMW_TEST_Logd("A2DP disconnected(%s)\n", param->addr);
            g_a2dp_addr_test[0] = 0;
            break;
        case BT_A2DP_EVENT_STREAM_SUSPEND:
            g_a2dp_stream_state = BT_A2DP_STREAM_STATE_SUSPEND;
            break;
        case BT_A2DP_EVENT_STREAM_START:
            g_a2dp_stream_state = BT_A2DP_STREAM_STATE_PLAYING;
            break;
        case BT_A2DP_EVENT_CONNECT_COMING:
            break;
        case BT_A2DP_EVENT_PLAYER_EVENT:
            BTMW_TEST_Logd("player event(%d)\n", param->data.player_event);
            break;
    }
    return;
}

INT32 btmw_test_a2dp_sink_init(VOID)
{
    INT32 ret = 0;
    BTMW_TEST_MOD a2dp_sink_mod = {0};

    a2dp_sink_mod.mod_id = BTMW_TEST_MOD_A2DP_SINK;
    strncpy(a2dp_sink_mod.cmd_key, BTMW_TEST_CMD_KEY_A2DP_SINK, sizeof(a2dp_sink_mod.cmd_key));
    a2dp_sink_mod.cmd_handler = btmw_test_a2dp_sink_cmd_handler;
    a2dp_sink_mod.cmd_tbl = btmw_test_a2dp_sink_cli_commands;

    ret = btmw_test_register_mod(&a2dp_sink_mod);
    BTMW_TEST_Logd("btmw_test_register_mod() for SINK returns: %d\n", ret);
    c_btm_a2dp_sink_register_player_cb(bt_a2dp_alsa_get_player());

    c_btm_a2dp_register_callback(btmw_test_a2dp_sink_app_cbk);


    return ret;
}


