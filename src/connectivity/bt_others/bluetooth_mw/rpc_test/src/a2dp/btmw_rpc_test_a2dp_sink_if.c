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

#include <string.h>
#include <stdlib.h>

#include "btmw_rpc_test_cli.h"
#include "btmw_rpc_test_debug.h"
#include "btmw_rpc_test_a2dp_sink_if.h"
#include "mtk_bt_service_a2dp_wrapper.h"

#define BTMW_RPC_TEST_CMD_KEY_A2DP_SINK        "MW_RPC_A2DP_SINK"

#define BTMW_RPC_A2DP_CASE_RETURN_STR(const) case const: return #const;

// CLI handler
static int btmw_rpc_test_a2dp_sink_connect_int_handler(int argc, char *argv[]);
static int btmw_rpc_test_a2dp_sink_disconnect_handler(int argc, char *argv[]);
static int btmw_rpc_test_a2dp_sink_start_play(int argc, char *argv[]);
static int btmw_rpc_test_a2dp_sink_stop_play(int argc, char *argv[]);
static int btmw_rpc_test_a2dp_sink_active_handler(int argc, char *argv[]);
static int btmw_rpc_test_a2dp_get_src_dev_list_handler(int argc, char *argv[]);
static int btmw_rpc_test_a2dp_codec_enable_handler(int argc, char *argv[]);

static BTMW_RPC_TEST_CLI btmw_rpc_test_a2dp_sink_cli_commands[] =
{
    {"connect",         btmw_rpc_test_a2dp_sink_connect_int_handler, " = connect <addr>"},
    {"disconnect",      btmw_rpc_test_a2dp_sink_disconnect_handler,  " = disconnect <addr>"},
    {"start_play",      btmw_rpc_test_a2dp_sink_start_play,          " = start_play "},
    {"stop_play",       btmw_rpc_test_a2dp_sink_stop_play,           " = stop_play"},
    {"active_sink",     btmw_rpc_test_a2dp_sink_active_handler,      " = active_sink <1:enable|0:disable>"},
    {"get_paired_dev",  btmw_rpc_test_a2dp_get_src_dev_list_handler, " = get_paired_dev"},
    {"codec_enable",    btmw_rpc_test_a2dp_codec_enable_handler,     " = codec_enable <codec> <1/0>"},
    {NULL, NULL, NULL},
};

static CHAR* btmw_rpc_test_a2dp_sink_app_role(BT_A2DP_ROLE local_role)
{
    switch((int)local_role)
    {
        BTMW_RPC_A2DP_CASE_RETURN_STR(BT_A2DP_ROLE_SRC)
        BTMW_RPC_A2DP_CASE_RETURN_STR(BT_A2DP_ROLE_SINK)
        default: return "UNKNOWN_ROLE";
   }
}

static CHAR* btmw_rpc_test_a2dp_sink_player_event_str(BT_A2DP_PLAYER_EVENT player_event)
{
    switch((int)player_event)
    {
        BTMW_RPC_A2DP_CASE_RETURN_STR(BT_A2DP_ALSA_PB_EVENT_STOP)
        BTMW_RPC_A2DP_CASE_RETURN_STR(BT_A2DP_ALSA_PB_EVENT_STOP_FAIL)
        BTMW_RPC_A2DP_CASE_RETURN_STR(BT_A2DP_ALSA_PB_EVENT_START)
        BTMW_RPC_A2DP_CASE_RETURN_STR(BT_A2DP_ALSA_PB_EVENT_START_FAIL)
        default: return "UNKNOWN_EVENT";
   }
}

static CHAR* btmw_rpc_test_a2dp_get_codec_str(UINT8 codec_tpye)
{
     switch(codec_tpye)
     {
         BTMW_RPC_A2DP_CASE_RETURN_STR(BT_A2DP_CODEC_TYPE_SBC)
         BTMW_RPC_A2DP_CASE_RETURN_STR(BT_A2DP_CODEC_TYPE_MP3)
         BTMW_RPC_A2DP_CASE_RETURN_STR(BT_A2DP_CODEC_TYPE_AAC)
         BTMW_RPC_A2DP_CASE_RETURN_STR(BT_A2DP_CODEC_TYPE_LDAC)
         default: return "UNKNOWN_CODEC_TYPE";
    }
}

CHAR g_rpc_a2dp_addr_test[18];
BT_A2DP_STREAM_STATE g_rpc_a2dp_stream_state = BT_A2DP_STREAM_STATE_SUSPEND;

static int btmw_rpc_test_a2dp_sink_start_play(int argc, char *argv[])
{
    a_mtkapi_a2dp_sink_start_player();

    return 0;
}

static int btmw_rpc_test_a2dp_sink_stop_play(int argc, char *argv[])
{
    a_mtkapi_a2dp_sink_stop_player();

    return 0;
}

static int btmw_rpc_test_a2dp_sink_active_handler(int argc, char *argv[])
{
    UINT8 u1_enable = 0;
    BTMW_RPC_TEST_Logi("%s()\n", __func__);

    if (argc != 1)
    {
        BTMW_RPC_TEST_Loge("[USAGE] active_sink [1:enable|0:disable]");
        return -1;
    }

    u1_enable = atoi(argv[0]);
    a_mtkapi_a2dp_sink_enable(u1_enable);

    return 0;
}

static int btmw_rpc_test_a2dp_get_src_dev_list_handler(int argc, char *argv[])
{
    BT_A2DP_DEVICE_LIST device_list;
    INT32 ret = 0;
    BTMW_RPC_TEST_Logi("%s()\n", __func__);

    memset((void*)&device_list, 0, sizeof(device_list));

    ret = a_mtkapi_a2dp_sink_get_dev_list(&device_list);
    BTMW_RPC_TEST_Logi("get paired src device list result:\n", __func__);
    BTMW_RPC_TEST_Logi("======================================\n");
    if(BT_SUCCESS == ret)
    {
        if (0 == device_list.dev_num)
        {
            BTMW_RPC_TEST_Logi("no paired src device\n");
        }
        else
        {
            INT32 i = 0;
            for(i=0;i<device_list.dev_num;i++)
            {
                BTMW_RPC_TEST_Logi("device[%d]: %s, name:%s, role:%s\n",
                    i, device_list.dev[i].addr, device_list.dev[i].name,
                    btmw_rpc_test_a2dp_sink_app_role(device_list.dev[i].role));
                BTMW_RPC_TEST_Logi("======================================\n");
            }
        }
    }
    else
    {
        BTMW_RPC_TEST_Logi("get paired sink device failed: %d\n", ret);
    }

    return 0;
}


static int btmw_rpc_test_a2dp_codec_enable_handler(int argc, char *argv[])
{
    CHAR *codec = NULL;
    INT32 codec_type = 0xFF;
    UINT8 u1_enable = 0;

    BTMW_RPC_TEST_Logi("%s()\n", __func__);

    if (argc != 2)
    {
        BTMW_RPC_TEST_Loge("[USAGE] codec_enable <aac/aptx> <1/0>");
        return -1;
    }

    codec = argv[0];
    u1_enable = atoi(argv[1]);
    /*
    if (!((strncmp(codec, "aac", sizeof("aac")) == 0) || \
        (strncmp(codec, "aptx", sizeof("aptx")) == 0)))
    {
        BTMW_RPC_TEST_Logi("error codec");
        BTMW_RPC_TEST_Loge("[USAGE] codec_enable <aac/aptx> <1/0>");
        return -1;
    }
*/
    BTMW_RPC_TEST_Logi("codec: %s\n", codec);
    if (strncmp(codec, "aac", sizeof("aac")) == 0)
    {
        codec_type = BT_A2DP_CODEC_TYPE_AAC;
    }
    else
    {
        BTMW_RPC_TEST_Loge("currently not support this codec");
    }

    BTMW_RPC_TEST_Logi("to %s %s(%ld) ......\n", u1_enable ? "enable" : "disable", codec, codec_type);

    a_mtkapi_a2dp_codec_enable(codec_type, u1_enable);

    return 0;
}

static int btmw_rpc_test_a2dp_sink_connect_int_handler(int argc, char *argv[])
{
    CHAR *ptr = NULL;

    BTMW_RPC_TEST_Logi("%s()\n", __func__);

    if (argc != 1)
    {
        BTMW_RPC_TEST_Loge("[USAGE] connect <addr>");
        return -1;
    }
    if (strlen(argv[0]) < 17)
    {
        BTMW_RPC_TEST_Loge("<addr> invalid. Good example is \"AA:BB:CC:DD:EE:FF\"");
        BTMW_RPC_TEST_Loge("[USAGE] connect <addr>");
        return -1;
    }

    ptr = argv[0];
    BTMW_RPC_TEST_Logi("A2DP connected to %s\n", ptr);
    a_mtkapi_a2dp_connect(ptr, BT_A2DP_ROLE_SINK);

    return 0;
}

static int btmw_rpc_test_a2dp_sink_disconnect_handler(int argc, char *argv[])
{
    CHAR *ptr;
    BTMW_RPC_TEST_Logi("%s()\n", __func__);

    if (argc != 1)
    {
        BTMW_RPC_TEST_Loge("[USAGE] disconnect <addr>");
        return -1;
    }
    if (strlen(argv[0]) < 17)
    {
        BTMW_RPC_TEST_Loge("<addr> invalid. Good example is \"AA:BB:CC:DD:EE:FF\"");
        BTMW_RPC_TEST_Loge("[USAGE] disconnect <addr>");
    }

    ptr = argv[0];
    BTMW_RPC_TEST_Logi("A2DP disconnected to %s\n", ptr);
    a_mtkapi_a2dp_disconnect(ptr);
    return 0;
}

static int  btmw_rpc_test_a2dp_sink_cmd_handler(int argc, char *argv[])
{
    BTMW_RPC_TEST_CLI *cmd, *match = NULL;
    INT32 ret = 0;
    INT32 count = 0;

    BTMW_RPC_TEST_Logd("%s argc: %d, argv[0]: %s\n", __func__, argc, argv[0]);

    cmd = btmw_rpc_test_a2dp_sink_cli_commands;
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
        BTMW_RPC_TEST_Logd("Unknown command '%s'\n", argv[0]);

        btmw_rpc_test_print_cmd_help(BTMW_RPC_TEST_CMD_KEY_A2DP_SINK, btmw_rpc_test_a2dp_sink_cli_commands);
        ret = -1;
    }
    else
    {
        match->handler(argc - 1, &argv[1]);
    }

    return ret;
}

static CHAR* btmw_rpc_test_a2dp_sink_app_event(BT_A2DP_EVENT event)
{
    switch((int)event)
    {
        BTMW_RPC_A2DP_CASE_RETURN_STR(BT_A2DP_EVENT_CONNECTED)
        BTMW_RPC_A2DP_CASE_RETURN_STR(BT_A2DP_EVENT_DISCONNECTED)
        BTMW_RPC_A2DP_CASE_RETURN_STR(BT_A2DP_EVENT_CONNECT_TIMEOUT)
        BTMW_RPC_A2DP_CASE_RETURN_STR(BT_A2DP_EVENT_STREAM_SUSPEND)
        BTMW_RPC_A2DP_CASE_RETURN_STR(BT_A2DP_EVENT_STREAM_START)
        BTMW_RPC_A2DP_CASE_RETURN_STR(BT_A2DP_EVENT_CONNECT_COMING)
        BTMW_RPC_A2DP_CASE_RETURN_STR(BT_A2DP_EVENT_PLAYER_EVENT)
        BTMW_RPC_A2DP_CASE_RETURN_STR(BT_A2DP_EVENT_ROLE_CHANGED)
        default: return "UNKNOWN_EVENT";
   }
}


static VOID btmw_rpc_test_a2dp_sink_app_cbk(BT_A2DP_EVENT_PARAM *param, VOID *pv_tag)
{
    UINT8 codec_type = 0;

    if (NULL == param)
    {
        BTMW_RPC_TEST_Loge("param is NULL\n");
        return;
    }

    BTMW_RPC_TEST_Logd("addr=%s, event=%d, %s\n", param->addr, param->event,
        btmw_rpc_test_a2dp_sink_app_event(param->event));
    switch (param->event)
    {
        case BT_A2DP_EVENT_CONNECTED:
            BTMW_RPC_TEST_Logd("A2DP connected(%s)\n", param->addr);
            BTMW_RPC_TEST_Logd("local_role=%s, sample rate=%d, channel num=%d\n",
                btmw_rpc_test_a2dp_sink_app_role(param->data.connected_data.local_role),
                param->data.connected_data.sample_rate,
                param->data.connected_data.channel_num);
            strcpy(g_rpc_a2dp_addr_test, param->addr);

            codec_type = param->data.connected_data.config.codec_type;
            BTMW_RPC_TEST_Logd("current codec is:%s(%d)\n", \
                btmw_rpc_test_a2dp_get_codec_str(codec_type), \
                codec_type);
            switch (codec_type)
            {
                case BT_A2DP_CODEC_TYPE_SBC: //A2D_MEDIA_CT_SBC
                {
                    BT_A2DP_SBC_CONF *local_sbc = &param->data.connected_data.config.codec_conf.sbc_conf;
                    BTMW_RPC_TEST_Logd("min_bitpool:%d\n", local_sbc->min_bitpool);
                    BTMW_RPC_TEST_Logd("max_bitpool:%d\n", local_sbc->max_bitpool);
                    BTMW_RPC_TEST_Logd("block_len:%d\n", local_sbc->block_len);
                    BTMW_RPC_TEST_Logd("num_subbands:%d\n", local_sbc->num_subbands);
                    BTMW_RPC_TEST_Logd("alloc_mthd:%d\n", local_sbc->alloc_mthd);
                    BTMW_RPC_TEST_Logd("samp_freq:%ld\n", local_sbc->samp_freq);
                    BTMW_RPC_TEST_Logd("ch_mode:%d\n", local_sbc->ch_mode);

                    break;
                }
                case BT_A2DP_CODEC_TYPE_AAC:
                {
                    BT_A2DP_AAC_CONF *local_aac = &param->data.connected_data.config.codec_conf.aac_conf;
                    BTMW_RPC_TEST_Logd("object_type:%d\n", local_aac->object_type);
                    BTMW_RPC_TEST_Logd("sample_rate:%ld\n", local_aac->samp_freq);
                    BTMW_RPC_TEST_Logd("channels:%d\n", local_aac->channels);
                    BTMW_RPC_TEST_Logd("VBR supported? (%s)\n", local_aac->vbr ? "TRUE" : "FALSE");
                    BTMW_RPC_TEST_Logd("bit_rate:%d\n", local_aac->bitrate);

                    break;
                }
                default:
                    BTMW_RPC_TEST_Logd("invalid codec type:%d\n", codec_type);
                    break;
            }
            break;
        case BT_A2DP_EVENT_DISCONNECTED:
            BTMW_RPC_TEST_Logd("A2DP disconnected(%s)\n", param->addr);
            g_rpc_a2dp_addr_test[0] = 0;
            break;
        case BT_A2DP_EVENT_CONNECT_TIMEOUT:
            BTMW_RPC_TEST_Logd("A2DP Connect Timeout(%s)\n", param->addr);
            g_rpc_a2dp_addr_test[0] = 0;
            break;
        case BT_A2DP_EVENT_STREAM_SUSPEND:
            g_rpc_a2dp_stream_state = BT_A2DP_STREAM_STATE_SUSPEND;
            break;
        case BT_A2DP_EVENT_STREAM_START:
            g_rpc_a2dp_stream_state = BT_A2DP_STREAM_STATE_PLAYING;
            break;
        case BT_A2DP_EVENT_CONNECT_COMING:
            break;
        case BT_A2DP_EVENT_PLAYER_EVENT:
            BTMW_RPC_TEST_Logd("player event %s(%d)\n",
                btmw_rpc_test_a2dp_sink_player_event_str(param->data.player_event),
                param->data.player_event);
            break;
        case BT_A2DP_EVENT_ROLE_CHANGED:
            BTMW_RPC_TEST_Logd("%s role change %s\n",
                btmw_rpc_test_a2dp_sink_app_role(param->data.role_change.role),
                param->data.role_change.enable?"enable":"disable");
            break;
        default:
            break;
    }
    return;
}

INT32 btmw_rpc_test_a2dp_sink_init(VOID)
{
    INT32 ret = 0;
    BTMW_RPC_TEST_MOD a2dp_sink_mod = {0};

    a2dp_sink_mod.mod_id = BTMW_RPC_TEST_MOD_A2DP_SINK;
    strncpy(a2dp_sink_mod.cmd_key, BTMW_RPC_TEST_CMD_KEY_A2DP_SINK, sizeof(a2dp_sink_mod.cmd_key));
    a2dp_sink_mod.cmd_handler = btmw_rpc_test_a2dp_sink_cmd_handler;
    a2dp_sink_mod.cmd_tbl = btmw_rpc_test_a2dp_sink_cli_commands;

    ret = btmw_rpc_test_register_mod(&a2dp_sink_mod);
    BTMW_RPC_TEST_Logd("btmw_rpc_test_register_mod() for SINK returns: %d\n", ret);

    if (!g_cli_pts_mode)
    {
        a_mtkapi_a2dp_register_callback(btmw_rpc_test_a2dp_sink_app_cbk, NULL);
    }

    return ret;
}


