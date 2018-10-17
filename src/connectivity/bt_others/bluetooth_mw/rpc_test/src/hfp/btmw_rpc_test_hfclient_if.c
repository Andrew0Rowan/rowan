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

#include "btmw_rpc_test_cli.h"
#include "btmw_rpc_test_debug.h"
#include "btmw_rpc_test_hfclient_if.h"
#include "mtk_bt_service_hfclient_wrapper.h"

#define MAX_HFCLIENT_NUMBER_LEN 32
#define BTMW_RPC_HFCLIENT_CASE_RETURN_STR(const) case const: return #const;

static int btmw_rpc_test_hfclient_enable(int argc, char **argv);
static int btmw_rpc_test_hfclient_disable(int argc, char **argv);
static int btmw_rpc_test_hfclient_connect(int argc, char **argv);
static int btmw_rpc_test_hfclient_disconnect(int argc, char **argv);
static int btmw_rpc_test_hfclient_connect_audio(int argc, char **argv);
static int btmw_rpc_test_hfclient_disconnect_audio(int argc, char **argv);
static int btmw_rpc_test_hfclient_start_vr(int argc, char **argv);
static int btmw_rpc_test_hfclient_stop_vr(int argc, char **argv);
static int btmw_rpc_test_hfclient_volume_control(int argc, char **argv);
static int btmw_rpc_test_hfclient_dial(int argc, char **argv);
static int btmw_rpc_test_hfclient_dial_memory(int argc, char **argv);
static int btmw_rpc_test_hfclient_call_action(int argc, char **argv);
static int btmw_rpc_test_hfclient_query_calls(int argc, char **argv);
static int btmw_rpc_test_hfclient_query_operator(int argc, char **argv);
static int btmw_rpc_test_hfclient_retrieve_subscriber(int argc, char **argv);
static int btmw_rpc_test_hfclient_dtmf(int argc, char **argv);
static int btmw_rpc_test_hfclient_request_voice_tag(int argc, char **argv);
static int btmw_rpc_test_hfclient_atcmd(int argc, char **argv);
static int btmw_rpc_test_hfclient_read_pb_entries(int argc, char **argv);

static BTMW_RPC_TEST_CLI btmw_rpc_test_hfclient_cli_commands[] =
{
    {(const char *)"enable",               btmw_rpc_test_hfclient_enable,               (const char *)" = enable"},
    {(const char *)"disable",              btmw_rpc_test_hfclient_disable,              (const char *)" = disable"},
    {(const char *)"connect",              btmw_rpc_test_hfclient_connect,              (const char *)" = connect <addr>"},
    {(const char *)"disconnect",           btmw_rpc_test_hfclient_disconnect,           (const char *)" = disconnect <addr>"},
    {(const char *)"connect_audio",        btmw_rpc_test_hfclient_connect_audio,        (const char *)" = connect_audio <addr>"},
    {(const char *)"disconnect_audio",     btmw_rpc_test_hfclient_disconnect_audio,     (const char *)" = disconnect_audio <addr>"},
    {(const char *)"start_vr",             btmw_rpc_test_hfclient_start_vr,             (const char *)" = start_vr"},
    {(const char *)"stop_vr",              btmw_rpc_test_hfclient_stop_vr,              (const char *)" = stop_vr"},
    {(const char *)"volume_control",       btmw_rpc_test_hfclient_volume_control,       (const char *)" = volume_control <type> <volume>"},
    {(const char *)"dial",                 btmw_rpc_test_hfclient_dial,                 (const char *)" = dial [<number>]"},
    {(const char *)"dial_memory",          btmw_rpc_test_hfclient_dial_memory,          (const char *)" = dial_memory <location>"},
    {(const char *)"call_action",          btmw_rpc_test_hfclient_call_action,          (const char *)" = call_action <action> <idx>"},
    {(const char *)"query_calls",          btmw_rpc_test_hfclient_query_calls,          (const char *)" = query_calls"},
    {(const char *)"query_operator",       btmw_rpc_test_hfclient_query_operator,       (const char *)" = query_operator"},
    {(const char *)"retrieve_subscriber",  btmw_rpc_test_hfclient_retrieve_subscriber,  (const char *)" = retrieve_subscriber"},
    {(const char *)"dtmf",                 btmw_rpc_test_hfclient_dtmf,                 (const char *)" = dtmf <code>"},
    {(const char *)"request_voice_tag",    btmw_rpc_test_hfclient_request_voice_tag,    (const char *)" = request_voice_tag"},
    {(const char *)"atcmd",                btmw_rpc_test_hfclient_atcmd,                (const char *)" = atcmd <cmd> <val1> <val2> [<arg>]"},
    {(const char *)"read_pb",              btmw_rpc_test_hfclient_read_pb_entries,      (const char *)" = read_pb"},
    {NULL, NULL, NULL},
};

int btmw_rpc_test_hfclient_cmd_handler(int argc, char **argv)
{
    BTMW_RPC_TEST_CLI *cmd, *match = NULL;
    int ret = 0;
    int count;

    count = 0;
    cmd = btmw_rpc_test_hfclient_cli_commands;

    BTMW_RPC_TEST_Logi("[HFCLIENT] argc: %d, argv[0]: %s\n", argc, argv[0]);
    if (NULL == argv[0])
    {
        BTMW_RPC_TEST_Logi("[HFCLIENT] Unknown command\n");
        btmw_rpc_test_print_cmd_help(BTMW_RPC_TEST_CMD_KEY_HFCLIENT, btmw_rpc_test_hfclient_cli_commands);
        return -1;
    }

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
        BTMW_RPC_TEST_Logi("[HFCLIENT] Unknown command '%s'\n", argv[0]);
        btmw_rpc_test_print_cmd_help(BTMW_RPC_TEST_CMD_KEY_HFCLIENT, btmw_rpc_test_hfclient_cli_commands);
        ret = -1;
    }
    else
    {
        match->handler(argc - 1, &argv[1]);
    }

    return ret;
}

static VOID btmw_rpc_test_hfclient_show_connection_state(BT_HFCLIENT_CONNECTION_CB_DATA_T *connect_cb)
{
    BTMW_RPC_TEST_Logd("state=%d, peer_feat=%d, chld_feat=%d, bt_addr=%s\n",
        connect_cb->state,
        connect_cb->peer_feat,
        connect_cb->chld_feat,
        connect_cb->addr);
    switch (connect_cb->state)
    {
    case BT_HFCLIENT_CONNECTION_STATE_DISCONNECTED:
        BTMW_RPC_TEST_Logd("HFP DISCONNECTED");
        break;
    case BT_HFCLIENT_CONNECTION_STATE_SLC_CONNECTED:
        BTMW_RPC_TEST_Logd("HFP CONNECTED");
        break;
    default:
        break;
    }
}


static VOID btmw_rpc_test_hfclient_show_audio_connection_state(BT_HFCLIENT_AUDIO_CONNECTION_CB_DATA_T *audio_connect_cb)
{
    BTMW_RPC_TEST_Logd("state=%d, bt_addr=%s\n",
        audio_connect_cb->state,
        audio_connect_cb->addr);

    switch (audio_connect_cb->state)
    {
        case BT_HFCLIENT_AUDIO_STATE_DISCONNECTED:
            BTMW_RPC_TEST_Logd("HFP AUDIO DISCONNECTED");
            break;
        case BT_HFCLIENT_AUDIO_STATE_CONNECTED:
            BTMW_RPC_TEST_Logd("HFP AUDIO CONNECTED CVSD");
            break;
        case BT_HFCLIENT_AUDIO_STATE_CONNECTED_MSBC:
            BTMW_RPC_TEST_Logd("HFP AUDIO CONNECTED MSBC");
            break;
        default:
            break;
    }
}

static VOID btmw_rpc_test_hfclient_show_vr_cmd(BT_HFCLIENT_BVRA_CB_DATA_T *bvra_cb)
{
    BTMW_RPC_TEST_Logd("vr_state=%d\n", bvra_cb->vr_state);
}

static VOID btmw_rpc_test_hfclient_show_network_status(BT_HFCLIENT_IND_SERVICE_CB_DATA_T *service_cb)
{
    BTMW_RPC_TEST_Logd("network_state=%d\n", service_cb->network_state);
}

static VOID btmw_rpc_test_hfclient_show_network_roaming(BT_HFCLIENT_IND_ROAM_CB_DATA_T *roam_cb)
{
    BTMW_RPC_TEST_Logd("service_type=%d\n", roam_cb->service_type);
}

static VOID btmw_rpc_test_hfclient_show_network_signal(BT_HFCLIENT_IND_SIGNAL_CB_DATA_T *signal_cb)
{
    BTMW_RPC_TEST_Logd("signal_strength=%d\n", signal_cb->signal_strength);
}

static VOID btmw_rpc_test_hfclient_show_battery_level(BT_HFCLIENT_IND_BATTCH_CB_DATA_T *battery_cb)
{
    BTMW_RPC_TEST_Logd("battery_level=%d\n", battery_cb->battery_level);
}

static VOID btmw_rpc_test_hfclient_show_current_operator(BT_HFCLIENT_COPS_CB_DATA_T *cops_cb)
{
    BTMW_RPC_TEST_Logd("operator_name=%s\n", cops_cb->operator_name);
}

static VOID btmw_rpc_test_hfclient_show_call(BT_HFCLIENT_IND_CALL_CB_DATA_T  *call_cb)
{
    BTMW_RPC_TEST_Logd("call=%d\n", call_cb->call);
}

static VOID btmw_rpc_test_hfclient_show_callsetup(BT_HFCLIENT_IND_CALLSETUP_CB_DATA_T *callsetup_cb)
{
    BTMW_RPC_TEST_Logd("callsetup=%d\n", callsetup_cb->callsetup);
}

static VOID btmw_rpc_test_hfclient_show_callheld(BT_HFCLIENT_IND_CALLHELD_CB_DATA_T *callheld_cb)
{
    BTMW_RPC_TEST_Logd("callheld=%d\n", callheld_cb->callheld);
}

static VOID btmw_rpc_test_hfclient_show_resp_and_hold(BT_HFCLIENT_BTRH_CB_DATA_T *btrh_cb)
{
    BTMW_RPC_TEST_Logd("resp_and_hold=%d\n", btrh_cb->resp_and_hold);
}

static VOID btmw_rpc_test_hfclient_show_clip(BT_HFCLIENT_CLIP_CB_DATA_T *clip_cb)
{
    BTMW_RPC_TEST_Logd("number=%s\n", clip_cb->number);
}

static VOID btmw_rpc_test_hfclient_show_call_waiting(BT_HFCLIENT_CCWA_CB_DATA_T *ccwa_cb)
{
    BTMW_RPC_TEST_Logd("number=%s\n", ccwa_cb->number);
}

static VOID btmw_rpc_test_hfclient_show_current_calls(BT_HFCLIENT_CLCC_CB_DATA_T *clcc_cb)
{
    BTMW_RPC_TEST_Logd("index=%d, dir=%d, state=%d, mpty=%d, number=%s\n",
                   clcc_cb->index, clcc_cb->dir, clcc_cb->mpty, clcc_cb->number);
}

static VOID btmw_rpc_test_hfclient_show_volume_change(BT_HFCLIENT_VGM_VGS_CB_DATA_T *volume_cb)
{
    BTMW_RPC_TEST_Logd("type=%d, volume=%d\n", volume_cb->type, volume_cb->volume);
}

static VOID btmw_rpc_test_hfclient_show_cmd_complete(BT_HFCLIENT_CMD_COMPLETE_CB_DATA_T *cmd_complete_cb)
{
    BTMW_RPC_TEST_Logd("type=%d, cme=%d\n", cmd_complete_cb->type, cmd_complete_cb->cme);
}

static VOID btmw_rpc_test_hfclient_show_subscriber_info(BT_HFCLIENT_CNUM_CB_DATA_T *cnum_cb)
{
    BTMW_RPC_TEST_Logd("number=%s, type=%d\n", cnum_cb->number, cnum_cb->type);
}

static VOID btmw_rpc_test_hfclient_show_in_band_ring_tone(BT_HFCLIENT_BSIR_CB_DATA_T *bsir_cb)
{
    BTMW_RPC_TEST_Logd("state=%d\n", bsir_cb->state);
}

static VOID btmw_rpc_test_hfclient_show_last_voice_tag_number(BT_HFCLIENT_BINP_CB_DATA_T *binp_cb)
{
    BTMW_RPC_TEST_Logd("number=%s\n", binp_cb->number);
}

static VOID btmw_rpc_test_hfclient_show_ring_indication(VOID)
{
    BTMW_RPC_TEST_Logd("ring indication!\n");
}

static VOID btmw_rpc_test_hfclient_show_mw_pb_entry_ready(VOID)
{
    BTMW_RPC_TEST_Logd("MW phonebook entries ready!\n");
}

static VOID btmw_rpc_test_hfclient_show_read_pb_entry_done(VOID)
{
    BTMW_RPC_TEST_Logd("MW callbaci phonebook entries done!\n");
}

static VOID btmw_rpc_test_hfclient_show_pb_entry_info(BT_HFCLIENT_PB_ENTRY_APP_DATA_T *pb_entry_app)
{
    BTMW_RPC_TEST_Logd("app pb entry, name=%s number=%s\n", pb_entry_app->name, pb_entry_app->number);
}

static CHAR* btmw_rpc_test_hfclient_app_event(BTAPP_HFCLIENT_CB_EVENT event)
{
    switch((int)event)
    {
        BTMW_RPC_HFCLIENT_CASE_RETURN_STR(BTAPP_HFCLIENT_CONNECTION_CB_EVT)
        BTMW_RPC_HFCLIENT_CASE_RETURN_STR(BTAPP_HFCLIENT_AUDIO_CONNECTION_CB_EVT)
        BTMW_RPC_HFCLIENT_CASE_RETURN_STR(BTAPP_HFCLIENT_BVRA_CB_EVT)
        BTMW_RPC_HFCLIENT_CASE_RETURN_STR(BTAPP_HFCLIENT_IND_SERVICE_CB_EVT)
        BTMW_RPC_HFCLIENT_CASE_RETURN_STR(BTAPP_HFCLIENT_IND_ROAM_CB_EVT)
        BTMW_RPC_HFCLIENT_CASE_RETURN_STR(BTAPP_HFCLIENT_IND_SIGNAL_CB_EVT)
        BTMW_RPC_HFCLIENT_CASE_RETURN_STR(BTAPP_HFCLIENT_IND_BATTCH_CB_EVT)
        BTMW_RPC_HFCLIENT_CASE_RETURN_STR(BTAPP_HFCLIENT_COPS_CB_EVT)
        BTMW_RPC_HFCLIENT_CASE_RETURN_STR(BTAPP_HFCLIENT_IND_CALL_CB_EVT)
        BTMW_RPC_HFCLIENT_CASE_RETURN_STR(BTAPP_HFCLIENT_IND_CALLSETUP_CB_EVT)
        BTMW_RPC_HFCLIENT_CASE_RETURN_STR(BTAPP_HFCLIENT_IND_CALLHELD_CB_EVT)
        BTMW_RPC_HFCLIENT_CASE_RETURN_STR(BTAPP_HFCLIENT_BTRH_CB_EVT)
        BTMW_RPC_HFCLIENT_CASE_RETURN_STR(BTAPP_HFCLIENT_CLIP_CB_EVT)
        BTMW_RPC_HFCLIENT_CASE_RETURN_STR(BTAPP_HFCLIENT_CCWA_CB_EVT)
        BTMW_RPC_HFCLIENT_CASE_RETURN_STR(BTAPP_HFCLIENT_CLCC_CB_EVT)
        BTMW_RPC_HFCLIENT_CASE_RETURN_STR(BTAPP_HFCLIENT_VGM_VGS_CB_EVT)
        BTMW_RPC_HFCLIENT_CASE_RETURN_STR(BTAPP_HFCLIENT_CMD_COMPLETE_CB_EVT)
        BTMW_RPC_HFCLIENT_CASE_RETURN_STR(BTAPP_HFCLIENT_CNUM_CB_EVT)
        BTMW_RPC_HFCLIENT_CASE_RETURN_STR(BTAPP_HFCLIENT_BSIR_CB_EVT)
        BTMW_RPC_HFCLIENT_CASE_RETURN_STR(BTAPP_HFCLIENT_BINP_CB_EVT)
        BTMW_RPC_HFCLIENT_CASE_RETURN_STR(BTAPP_HFCLIENT_RING_IND_CB_EVT)
        BTMW_RPC_HFCLIENT_CASE_RETURN_STR(BTAPP_HFCLIENT_CPBR_ENTRY_EVT)
        BTMW_RPC_HFCLIENT_CASE_RETURN_STR(BTAPP_HFCLIENT_CPBR_READY_EVT)
        BTMW_RPC_HFCLIENT_CASE_RETURN_STR(BTAPP_HFCLIENT_CPBR_DONE_EVT)
        default: return "UNKNOWN_HFCLIENT_EVENT";
   }
}

static VOID btmw_rpc_test_hfclient_app_cbk(BT_HFCLIENT_EVENT_PARAM *param, VOID *pv_tag)
{
    if (NULL == param)
    {
        BTMW_RPC_TEST_Loge("param is NULL\n");
        return;
    }

    BTMW_RPC_TEST_Logd("event=%d %s\n", param->event,
                        btmw_rpc_test_hfclient_app_event(param->event));
    switch (param->event)
    {
        case BTAPP_HFCLIENT_CONNECTION_CB_EVT:
            btmw_rpc_test_hfclient_show_connection_state(&param->data.connect_cb);
            break;
        case BTAPP_HFCLIENT_AUDIO_CONNECTION_CB_EVT:
            btmw_rpc_test_hfclient_show_audio_connection_state(&param->data.auido_connect_cb);
            break;
        case BTAPP_HFCLIENT_BVRA_CB_EVT:
            btmw_rpc_test_hfclient_show_vr_cmd(&param->data.bvra_cb);
            break;
        case BTAPP_HFCLIENT_IND_SERVICE_CB_EVT:
            btmw_rpc_test_hfclient_show_network_status(&param->data.service_cb);
            break;
        case BTAPP_HFCLIENT_IND_ROAM_CB_EVT:
            btmw_rpc_test_hfclient_show_network_roaming(&param->data.roam_cb);
            break;
        case BTAPP_HFCLIENT_IND_SIGNAL_CB_EVT:
            btmw_rpc_test_hfclient_show_network_signal(&param->data.signal_cb);
            break;
        case BTAPP_HFCLIENT_IND_BATTCH_CB_EVT:
            btmw_rpc_test_hfclient_show_battery_level(&param->data.battery_cb);
            break;
        case BTAPP_HFCLIENT_COPS_CB_EVT:
            btmw_rpc_test_hfclient_show_current_operator(&param->data.cops_cb);
            break;
        case BTAPP_HFCLIENT_IND_CALL_CB_EVT:
            btmw_rpc_test_hfclient_show_call(&param->data.call_cb);
            break;
        case BTAPP_HFCLIENT_IND_CALLSETUP_CB_EVT:
            btmw_rpc_test_hfclient_show_callsetup(&param->data.callsetup_cb);
            break;
        case BTAPP_HFCLIENT_IND_CALLHELD_CB_EVT:
            btmw_rpc_test_hfclient_show_callheld(&param->data.callheld_cb);
            break;
        case BTAPP_HFCLIENT_BTRH_CB_EVT:
            btmw_rpc_test_hfclient_show_resp_and_hold(&param->data.btrh_cb);
            break;
        case BTAPP_HFCLIENT_CLIP_CB_EVT:
            btmw_rpc_test_hfclient_show_clip(&param->data.clip_cb);
            break;
        case BTAPP_HFCLIENT_CCWA_CB_EVT:
            btmw_rpc_test_hfclient_show_call_waiting(&param->data.ccwa_cb);
            break;
        case BTAPP_HFCLIENT_CLCC_CB_EVT:
            btmw_rpc_test_hfclient_show_current_calls(&param->data.clcc_cb);
            break;
        case BTAPP_HFCLIENT_VGM_VGS_CB_EVT:
            btmw_rpc_test_hfclient_show_volume_change(&param->data.volume_cb);
            break;
        case BTAPP_HFCLIENT_CMD_COMPLETE_CB_EVT:
            btmw_rpc_test_hfclient_show_cmd_complete(&param->data.cmd_complete_cb);
            break;
        case BTAPP_HFCLIENT_CNUM_CB_EVT:
            btmw_rpc_test_hfclient_show_subscriber_info(&param->data.cnum_cb);
            break;
        case BTAPP_HFCLIENT_BSIR_CB_EVT:
            btmw_rpc_test_hfclient_show_in_band_ring_tone(&param->data.bsir_cb);
            break;
        case BTAPP_HFCLIENT_BINP_CB_EVT:
            btmw_rpc_test_hfclient_show_last_voice_tag_number(&param->data.binp_cb);
            break;
        case BTAPP_HFCLIENT_RING_IND_CB_EVT:
            btmw_rpc_test_hfclient_show_ring_indication();
            break;
        case BTAPP_HFCLIENT_CPBR_ENTRY_EVT:
            btmw_rpc_test_hfclient_show_pb_entry_info(&param->data.pb_entry_app);
            break;
        case BTAPP_HFCLIENT_CPBR_READY_EVT:
            btmw_rpc_test_hfclient_show_mw_pb_entry_ready();
            break;
        case BTAPP_HFCLIENT_CPBR_DONE_EVT:
            btmw_rpc_test_hfclient_show_read_pb_entry_done();
            break;
        default:
            break;
    }
}

int btmw_rpc_test_hfclient_init()
{
    int ret = 0;
    BTMW_RPC_TEST_MOD hfclient_mod = {0};

    hfclient_mod.mod_id = BTMW_RPC_TEST_MOD_HFCLIENT;
    strncpy(hfclient_mod.cmd_key, BTMW_RPC_TEST_CMD_KEY_HFCLIENT, sizeof(hfclient_mod.cmd_key));
    hfclient_mod.cmd_handler = btmw_rpc_test_hfclient_cmd_handler;
    hfclient_mod.cmd_tbl = btmw_rpc_test_hfclient_cli_commands;

    ret = btmw_rpc_test_register_mod(&hfclient_mod);
    BTMW_RPC_TEST_Logd("[HFCLIENT] btmw_rpc_test_register_mod() returns: %d\n", ret);
    if (!g_cli_pts_mode)
    {
        a_mtkapi_bt_hfclient_register_callback(btmw_rpc_test_hfclient_app_cbk, NULL);
    }
    return ret;
}

int btmw_rpc_test_hfclient_deinit()
{
    BTMW_RPC_TEST_Logi("%s", __func__);
    return 0;
}

static int btmw_rpc_test_hfclient_enable(int argc, char **argv)
{
    BTMW_RPC_TEST_Logi("[HFCLIENT] %s()\n", __func__);
    return a_mtkapi_bt_hfclient_enable();
}

static int btmw_rpc_test_hfclient_disable(int argc, char **argv)
{
    BTMW_RPC_TEST_Logi("[HFCLIENT] %s()\n", __func__);
    return a_mtkapi_bt_hfclient_disable();
}

static int btmw_rpc_test_hfclient_connect(int argc, char **argv)
{
    CHAR bt_addr[MAX_BDADDR_LEN] = {0};

    BTMW_RPC_TEST_Logi("[HFCLIENT] %s()\n", __func__);

    if (argc < 1)
    {
        BTMW_RPC_TEST_Logi("Usage :\n");
        BTMW_RPC_TEST_Logi("  HFCLIENT connect <addr>\n");
        return -1;
    }

    strncpy(bt_addr, argv[0], MAX_BDADDR_LEN);
    bt_addr[MAX_BDADDR_LEN - 1] = '\0';
    return a_mtkapi_bt_hfclient_connect(bt_addr);
}

static int btmw_rpc_test_hfclient_disconnect(int argc, char **argv)
{
    CHAR bt_addr[MAX_BDADDR_LEN] = {0};

    BTMW_RPC_TEST_Logi("[HFCLIENT] %s()\n", __func__);

    if (argc < 1)
    {
        BTMW_RPC_TEST_Logi("Usage :\n");
        BTMW_RPC_TEST_Logi("  HFCLIENT disconnect <addr>\n");
        return -1;
    }

    strncpy(bt_addr, argv[0], MAX_BDADDR_LEN);
    bt_addr[MAX_BDADDR_LEN - 1] = '\0';
    return a_mtkapi_bt_hfclient_disconnect(bt_addr);
}

static int btmw_rpc_test_hfclient_connect_audio(int argc, char **argv)
{
    CHAR bt_addr[MAX_BDADDR_LEN] = {0};

    BTMW_RPC_TEST_Logi("[HFCLIENT] %s()\n", __func__);

    if (argc < 1)
    {
        BTMW_RPC_TEST_Logi("Usage :\n");
        BTMW_RPC_TEST_Logi("  HFCLIENT connect_audio <addr>\n");
        return -1;
    }

    strncpy(bt_addr, argv[0], MAX_BDADDR_LEN);
    bt_addr[MAX_BDADDR_LEN - 1] = '\0';
    return a_mtkapi_bt_hfclient_connect_audio(bt_addr);
}

static int btmw_rpc_test_hfclient_disconnect_audio(int argc, char **argv)
{
    CHAR bt_addr[MAX_BDADDR_LEN] = {0};

    BTMW_RPC_TEST_Logi("[HFCLIENT] %s()\n", __func__);

    if (argc < 1)
    {
        BTMW_RPC_TEST_Logi("Usage :\n");
        BTMW_RPC_TEST_Logi("  HFCLIENT disconnect_audio <addr>\n");
        return -1;
    }

    strncpy(bt_addr, argv[0], MAX_BDADDR_LEN);
    bt_addr[MAX_BDADDR_LEN - 1] = '\0';
    return a_mtkapi_bt_hfclient_disconnect_audio(bt_addr);
}

static int btmw_rpc_test_hfclient_start_vr(int argc, char **argv)
{
    BTMW_RPC_TEST_Logi("[HFCLIENT] %s()\n", __func__);
    return a_mtkapi_bt_hfclient_start_voice_recognition();
}

static int btmw_rpc_test_hfclient_stop_vr(int argc, char **argv)
{
    BTMW_RPC_TEST_Logi("[HFCLIENT] %s()\n", __func__);
    return a_mtkapi_bt_hfclient_stop_voice_recognition();
}

static int btmw_rpc_test_hfclient_volume_control(int argc, char **argv)
{
    INT32 type = 0;
    INT32 volume = 0;

    BTMW_RPC_TEST_Logi("[HFCLIENT] %s()\n", __func__);

    if (argc != 2)
    {
        BTMW_RPC_TEST_Logi("Usage :\n");
        BTMW_RPC_TEST_Logi("  HFCLIENT volume_control <type> <volume>\n");
        return -1;
    }

    type = atoi(argv[0]);
    volume = atoi(argv[1]);
    return a_mtkapi_bt_hfclient_volume_control(type, volume);
}

static int btmw_rpc_test_hfclient_dial(int argc, char **argv)
{
    CHAR number[MAX_HFCLIENT_NUMBER_LEN + 1] = {0};

    BTMW_RPC_TEST_Logi("[HFCLIENT] %s()\n", __func__);

    if (0 == argc)
    {
        return a_mtkapi_bt_hfclient_dial(NULL);
    }

    if (argc != 1)
    {
        BTMW_RPC_TEST_Logi("Usage :\n");
        BTMW_RPC_TEST_Logi("  HFCLIENT dial <number>\n");
        return -1;
    }

    strncpy(number, argv[0], MAX_HFCLIENT_NUMBER_LEN);
    number[MAX_HFCLIENT_NUMBER_LEN] = '\0';
    return a_mtkapi_bt_hfclient_dial(number);
}

static int btmw_rpc_test_hfclient_dial_memory(int argc, char **argv)
{
    INT32 location = 0;

    BTMW_RPC_TEST_Logi("[HFCLIENT] %s()\n", __func__);

    if (argc != 1)
    {
        BTMW_RPC_TEST_Logi("Usage :\n");
        BTMW_RPC_TEST_Logi("  HFCLIENT dial_memory <location>\n");
        return -1;
    }

    location = atoi(argv[0]);
    return a_mtkapi_bt_hfclient_dial_memory(location);
}

static int btmw_rpc_test_hfclient_call_action(int argc, char **argv)
{
    INT32 action = 0;
    INT32 idx = 0;

    BTMW_RPC_TEST_Logi("[HFCLIENT] %s()\n", __func__);

    if (argc != 2)
    {
        BTMW_RPC_TEST_Logi("Usage :\n");
        BTMW_RPC_TEST_Logi("  HFCLIENT call_action <action> <idx>\n");
        return -1;
    }

    action = atoi(argv[0]);
    idx = atoi(argv[1]);
    return a_mtkapi_bt_hfclient_handle_call_action(action, idx);
}

static int btmw_rpc_test_hfclient_query_calls(int argc, char **argv)
{
    BTMW_RPC_TEST_Logi("[HFCLIENT] %s()\n", __func__);
    return a_mtkapi_bt_hfclient_query_current_calls();
}

static int btmw_rpc_test_hfclient_query_operator(int argc, char **argv)
{
    BTMW_RPC_TEST_Logi("[HFCLIENT] %s()\n", __func__);
    return a_mtkapi_bt_hfclient_query_current_operator_name();
}

static int btmw_rpc_test_hfclient_retrieve_subscriber(int argc, char **argv)
{
    BTMW_RPC_TEST_Logi("[HFCLIENT] %s()\n", __func__);
    return a_mtkapi_bt_hfclient_retrieve_subscriber_info();
}

static int btmw_rpc_test_hfclient_dtmf(int argc, char **argv)
{
    CHAR code = 0;

    BTMW_RPC_TEST_Logi("[HFCLIENT] %s()\n", __func__);

    if (argc != 1)
    {
        BTMW_RPC_TEST_Logi("Usage :\n");
        BTMW_RPC_TEST_Logi("  HFCLIENT dtmf <code>\n");
        return -1;
    }

    code = argv[0][0];
    return a_mtkapi_bt_hfclient_send_dtmf(code);
}

static int btmw_rpc_test_hfclient_request_voice_tag(int argc, char **argv)
{
    BTMW_RPC_TEST_Logi("[HFCLIENT] %s()\n", __func__);
    return a_mtkapi_bt_hfclient_request_last_voice_tag_number();
}

static int btmw_rpc_test_hfclient_atcmd(int argc, char **argv)
{
    INT32 len = 0;
    INT32 cmd = 0;
    INT32 val1 = 0;
    INT32 val2 = 0;
    CHAR arg[MAX_HFCLIENT_NUMBER_LEN + 1] = {0};

    BTMW_RPC_TEST_Logi("[HFCLIENT] %s()\n", __func__);

    if ((argc != 3) && (argc != 4))
    {
        BTMW_RPC_TEST_Logi("Usage :\n");
        BTMW_RPC_TEST_Logi("  HFCLIENT atcmd <cmd> <val1> <val2> [<arg>]\n");
        return -1;
    }

    cmd = atoi(argv[0]);
    val1 = atoi(argv[1]);
    val2 = atoi(argv[2]);
    if (argv[3])
    {
        len = strlen(argv[3]);
        if (len > MAX_HFCLIENT_NUMBER_LEN)
        {
            BTMW_RPC_TEST_Logi("  HFCLIENT max <arg> length is %d\n", MAX_HFCLIENT_NUMBER_LEN);
            return -1;
        }
        strncpy(arg, argv[3], len);
        arg[len] = '\0';
    }

    return a_mtkapi_bt_hfclient_send_at_cmd(cmd, val1, val2, arg);
}

static int btmw_rpc_test_hfclient_read_pb_entries(int argc, char **argv)
{
    BTMW_RPC_TEST_Loge("[HFCLIENT] %s()\n", __func__);
    return a_mtkapi_bt_hfclient_read_pb_entries();
}

