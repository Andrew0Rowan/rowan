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
#include "btmw_rpc_test_gap_if.h"
#include "mtk_bt_service_gap_wrapper.h"

#define BLUETOOTH_RPC_TEST_NAME             "audio_bt"
#define BT_NAME_SUF_RPC_TEST_LEN              5

static void btmw_rpc_test_gap_event_callback(tBTMW_GAP_STATE bt_event, void* pv_tag)
{
    //BTMW_RPC_TEST_Logi("[GAP] %s(), bt_gap_event =%d\n",  __func__, bt_event.state);
    switch (bt_event.state)
     {
     case GAP_STATE_ON:
        BTMW_RPC_TEST_Logi("[GAP] %s(), bt_gap_event = state on\n",  __func__);
        break;
     case GAP_STATE_OFF:
        BTMW_RPC_TEST_Logi("[GAP] %s(), bt_gap_event = state off\n",  __func__);
        break;

     case GAP_STATE_ACL_CONNECTED:
        BTMW_RPC_TEST_Logi("[GAP] %s(), bt_gap_event = acl connected:%s\n",  __func__, bt_event.bd_addr);
        break;

     case GAP_STATE_ACL_DISCONNECTED:
        if(bt_event.reason)
        {
             if (0x08 == bt_event.reason)
             {
                 BTMW_RPC_TEST_Logi("[GAP] %s(), bt_gap_event = connect lost\n",  __func__);
             }
        }
        else
        {
            BTMW_RPC_TEST_Logi("[GAP] %s(), bt_gap_event = acl is disconnected:%s\n",  __func__, bt_event.bd_addr);
        }
        break;

     case GAP_STATE_BONDED:
        break;
     case GAP_STATE_BONDING:
        break;
     case GAP_STATE_NO_BOND:
        break;
     default:
        BTMW_RPC_TEST_Logi("[GAP] %s(), undefined bt_gap_event \n",  __func__);
        break;
     }

    return;
}

static void btmw_rpc_test_gap_get_pairing_key_callback(pairing_key_value_t *bt_pairing_key, UINT8 *fg_accept, void* pv_tag)
{
    BTMW_RPC_TEST_Logi("[GAP] %s(), bt_pairing_key->pin_code =%s,  key_value = %lu, fg_accept = %u\n",  __func__, bt_pairing_key->pin_code, bt_pairing_key->key_value, *fg_accept);
    return;
}

static void btmw_rpc_test_gap_app_inquiry_callback(tBTMW_GAP_DEVICE_INFO *pt_result, void* pv_tag)
{
    if (NULL == pt_result)
    {
        return;
    }

    BTMW_RPC_TEST_Logi("[GAP] %s() \n", __func__);
    BTMW_RPC_TEST_Logi("device_kind:     %d\n", pt_result->device_kind);
    BTMW_RPC_TEST_Logi("name:     %s\n", pt_result->device.name);
    BTMW_RPC_TEST_Logi("cod:      0x%2lx\n", pt_result->device.cod);
    BTMW_RPC_TEST_Logi("bdAddr:   %s\n", pt_result->device.bdAddr);
}


int btmw_rpc_test_gap_bt_base_init(MTKRPCAPI_BT_APP_CB_FUNC *func, void *pv_tag)
{
    BTMW_RPC_TEST_Logi("[GAP] btmw_rpc_test_gap_bt_base_init\n");
    return a_mtkapi_gap_bt_base_init(func, pv_tag);
}

int btmw_rpc_test_gap_set_local_name(VOID)
{
    //INT32 i4_ret;
    //CHAR ac_bdAddr[MAX_BDADDR_LEN]={0};
    //CHAR ac_name[MAX_NAME_LEN] = {0};
    //CHAR bt_mac_suf[MAX_BDADDR_LEN] = {0};
    //BT_GAP_LOCAL_PROPERTIES_RESULT t_local_info;
    //memset(&t_local_info, 0, sizeof(BT_GAP_LOCAL_PROPERTIES_RESULT));

    //get local mac address
    //i4_ret = a_mtkapi_bt_gap_get_local_dev_info(&t_local_info);
   // strncpy(ac_bdAddr, t_local_info.bdAddr, MAX_BDADDR_LEN);

     //only use the last 5 characters of the bluetooth device mac address
    //strncpy(bt_mac_suf, ac_bdAddr + (strlen(ac_bdAddr)-BT_NAME_SUF_RPC_TEST_LEN), BT_NAME_SUF_RPC_TEST_LEN);

    //strncpy(ac_name, BLUETOOTH_RPC_TEST_NAME, strlen(BLUETOOTH_RPC_TEST_NAME));

    //the bluetooth name: ac_name = BLUETOOTH_RPC_TEST_NAME + (bt_mac_suf)
    //strcat(ac_name,"(");
    //strcat(ac_name,bt_mac_suf);
    //strcat(ac_name,")");
    //a_mtkapi_bt_gap_set_name(ac_name);
    //return i4_ret;
}

int btmw_rpc_test_gap_set_dbg_level_cli(int argc, char **argv)
{
    if (argc != 2)
    {
        BTMW_RPC_TEST_Loge("Usage :\n");
        BTMW_RPC_TEST_Loge("layer <0~8> means set target layer to level\n");
        BTMW_RPC_TEST_Loge("if layer is 9 means set all layer to level\n");
        BTMW_RPC_TEST_Loge("level <0~5>\n");
        return BT_ERR_STATUS_INVALID_PARM_NUMS;
    }

    a_mtkapi_bt_gap_set_dbg_level(atoi(argv[0]), atoi(argv[1]));
    return BT_SUCCESS;
}

int btmw_rpc_test_gap_set_power_cli(int argc, char **argv)
{
    if (argc != 1)
    {
        BTMW_RPC_TEST_Loge("0 means off, 1 means on\n");
        return BT_ERR_STATUS_INVALID_PARM_NUMS;
    }

    if (0 == strcmp("0" , argv[0]))
    {
        return a_mtkapi_bt_gap_on_off(FALSE);
    }
    else if (0 == strcmp("1" , argv[0]))
    {
        return a_mtkapi_bt_gap_on_off(TRUE);
    }
    else
    {
        return BT_ERR_STATUS_PARM_INVALID;
    }

    return BT_SUCCESS;
}

int btmw_rpc_test_gap_set_name_cli(int argc, char **argv)
{
    INT32 i4_ret = BT_SUCCESS;

    if (argc != 1)
    {
        BTMW_RPC_TEST_Logd("please attach name after commond just append one string\n");
        return BT_ERR_STATUS_INVALID_PARM_NUMS;
    }
    if (NULL == argv[0])
    {
        BTMW_RPC_TEST_Logd("please attach name after commond just append one string\n");
        return BT_ERR_STATUS_NULL_POINTER;
    }

    BTMW_RPC_TEST_Logd("name is: %s\n", argv[0]);
    i4_ret = a_mtkapi_bt_gap_set_name(argv[0]);
    if (BT_SUCCESS == i4_ret)
    {
        BTMW_RPC_TEST_Logd("set name ok!\n");
    }

    return BT_SUCCESS;
}

int btmw_rpc_test_gap_set_conn_disc_cli(int argc, char **argv)
{
    if (2 == argc)
    {
        if (0 == strcmp("1" , argv[0]))
        {
            if (0 == strcmp("1" , argv[1]))
            {
                return a_mtkapi_bt_gap_set_connectable_and_discoverable(TRUE, TRUE);
            }
            else if (0 == strcmp("0" , argv[1]))
            {
                return a_mtkapi_bt_gap_set_connectable_and_discoverable(TRUE, FALSE);
            }
            else
            {
                BTMW_RPC_TEST_Loge("input error, 0 means off, 1 means on\n");
                return BT_ERR_STATUS_PARM_INVALID;
            }
        }
        else if (0 == strcmp("0" , argv[0]))
        {
            if (0 == strcmp("1" , argv[1]))
            {
                return a_mtkapi_bt_gap_set_connectable_and_discoverable(FALSE, TRUE);
            }
            else if (0 == strcmp("0" , argv[1]))
            {
                return a_mtkapi_bt_gap_set_connectable_and_discoverable(FALSE, FALSE);
            }
            else
            {
                BTMW_RPC_TEST_Loge("input error, 0 means off, 1 means on\n");
                return BT_ERR_STATUS_PARM_INVALID;
            }
        }
    }
    else
    {
        BTMW_RPC_TEST_Loge("0 means off, other integer:on");
        return BT_ERR_STATUS_INVALID_PARM_NUMS;
    }
    return BT_SUCCESS;
}

int btmw_rpc_test_gap_get_dev_info_cli(int argc, char **argv)
{
    BLUETOOTH_DEVICE dev_info;
    INT32 i4_ret = BT_SUCCESS;

    if (argc != 1)
    {
        BTMW_RPC_TEST_Loge("no parameter in this command\n");
        return BT_ERR_STATUS_INVALID_PARM_NUMS;
    }

    if (NULL == argv[0])
    {
        BTMW_RPC_TEST_Logd("addr is null \n");
        return BT_ERR_STATUS_NULL_POINTER;
    }
    if (17 != strlen(argv[0]))
    {
        BTMW_RPC_TEST_Logd("mac length should be 17\n");
        return BT_ERR_STATUS_PARM_INVALID;
    }

    i4_ret = a_mtkapi_bt_gap_get_dev_info(&dev_info, argv[0]);

    if(i4_ret == BT_SUCCESS)
    {
        BTMW_RPC_TEST_Logd("get device info ok!\n");
        BTMW_RPC_TEST_Logd("dev_info.bdAddr:      %s\n", dev_info.bdAddr);
        BTMW_RPC_TEST_Logd("ps_dev_info.name:        %s\n", dev_info.name);
    }
    return BT_SUCCESS;
}

int btmw_rpc_test_gap_get_local_dev_info_cli(int argc, char **argv)
{
    BT_LOCAL_DEV ps_dev_info;
    INT32 i4_ret = BT_SUCCESS;
    if (argc != 0)
    {
        BTMW_RPC_TEST_Loge("no parameter in this command\n");
        return BT_ERR_STATUS_INVALID_PARM_NUMS;
    }

    a_mtkapi_bt_gap_get_local_dev_info(&ps_dev_info);

    if (BT_SUCCESS == i4_ret)
    {
        BTMW_RPC_TEST_Logd("get local device info ok!\n");
        BTMW_RPC_TEST_Logd("ps_dev_info.bdAddr:      %s\n", ps_dev_info.bdAddr);
        BTMW_RPC_TEST_Logd("ps_dev_info.name:        %s\n", ps_dev_info.name);
        BTMW_RPC_TEST_Logd("ps_dev_info.powered:     %s\n", (ps_dev_info.state == GAP_STATE_ON) ? "on" : "off");
    }
    else
    {
        BTMW_RPC_TEST_Logd("get local device info failed!\n");
    }

    return BT_SUCCESS;
}

int btmw_rpc_test_gap_inquiry_cli(int argc, char **argv)
{
    UINT32 ui4_filter_type = BT_INQUIRY_FILTER_TYPE_ALL;
    if (argc != 1)
    {
        BTMW_RPC_TEST_Loge("no parameter in this command\n");
        return BT_ERR_STATUS_INVALID_PARM_NUMS;
    }

    /* <all|src|sink|hfp|hid> */
    if (0 == strcmp(argv[0], "all"))
    {
        ui4_filter_type = BT_INQUIRY_FILTER_TYPE_ALL;
    }
    else if (0 == strcmp(argv[0], "src"))
    {
        ui4_filter_type = BT_INQUIRY_FILTER_TYPE_A2DP_SRC;
    }
    else if (0 == strcmp(argv[0], "sink"))
    {
        ui4_filter_type = BT_INQUIRY_FILTER_TYPE_A2DP_SNK;
    }
    else if (0 == strcmp(argv[0], "hfp"))
    {
        ui4_filter_type = BT_INQUIRY_FILTER_TYPE_HFP;
    }
    else if (0 == strcmp(argv[0], "hid"))
    {
        ui4_filter_type = BT_INQUIRY_FILTER_TYPE_HID;
    }

    a_mtkapi_bt_gap_start_inquiry_scan(ui4_filter_type);
    return 0;
}

int btmw_rpc_test_gap_stop_inquiry_cli(int argc, char **argv)
{
    if (argc != 0)
    {
        BTMW_RPC_TEST_Loge("no parameter in this command\n");
        return BT_ERR_STATUS_INVALID_PARM_NUMS;
    }

    return a_mtkapi_bt_gap_stop_inquiry_scan();
}

int btmw_rpc_test_gap_pair_cli (int argc, char **argv)
{
    UINT32 transport;

    if (argc != 2)
    {
        BTMW_RPC_TEST_Logd("parameter num error,please enter two parameter: addr and transport \n");
        return BT_ERR_STATUS_INVALID_PARM_NUMS;
    }
    if (NULL == argv[0])
    {
        BTMW_RPC_TEST_Logd("addr is null \n");
        return BT_ERR_STATUS_NULL_POINTER;
    }

    if (17 != strlen(argv[0]))
    {
        BTMW_RPC_TEST_Logd("mac length should be 17\n");
        return BT_ERR_STATUS_PARM_INVALID;
    }
    transport = atol(argv[1]);
    return a_mtkapi_bt_gap_pair(argv[0], transport);

}

int btmw_rpc_test_gap_remove_paired_dev_cli(int argc, char **argv)
{
    if (argc != 1)
    {
        BTMW_RPC_TEST_Logd("please attach MAC after commond just append one string\n");
        return BT_ERR_STATUS_INVALID_PARM_NUMS;
    }
    if (NULL == argv[0])
    {
        BTMW_RPC_TEST_Logd("please attach MAC after commond just append one string\n");
        return BT_ERR_STATUS_NULL_POINTER;
    }
    if (17 != strlen(argv[0]))
    {
        BTMW_RPC_TEST_Logd("mac length should be 17\n");
        return BT_ERR_STATUS_PARM_INVALID;
    }

    return a_mtkapi_bt_gap_unpair(argv[0]);
}

int btmw_rpc_test_gap_get_rssi_cli (int argc, char **argv)
{
    CHAR ps_target_mac[18];
    INT16 rssi_value;

    if (1 == argc)
    {
        memset(ps_target_mac, 0, sizeof(ps_target_mac));
        if (17 != strlen(argv[0]))
        {
            BTMW_RPC_TEST_Logd("mac length should be 17\n");
            return BT_ERR_STATUS_PARM_INVALID;
        }
        strncpy(ps_target_mac,argv[0],sizeof(ps_target_mac)-1);
    }
    else
    {
        BTMW_RPC_TEST_Logd("please input get_rssi [MAC address]\n");
        return BT_ERR_STATUS_INVALID_PARM_NUMS;
    }

    a_mtkapi_bt_gap_get_rssi(ps_target_mac, &rssi_value);
    return BT_SUCCESS;
}

int btmw_rpc_test_gap_set_virtual_sniffer_cli(int argc, char **argv)
{

    INT32 flag;

    if (argc != 1)
    {
        BTMW_RPC_TEST_Logd("A parameter is required!\n");
        return BT_ERR_STATUS_INVALID_PARM_NUMS;
    }

    flag = atoi(argv[0]);
    a_mtkapi_bt_gap_set_virtual_sniffer(flag);

    return BT_SUCCESS;
}

int btmw_rpc_test_gap_send_hci(int argc, char **argv)
{
    CHAR *hci_cmd;

    if (argc != 1)
        return BT_ERR_STATUS_INVALID_PARM_NUMS;

    hci_cmd = (CHAR *)argv[0];
    a_mtkapi_bt_gap_send_hci(hci_cmd);
    return BT_SUCCESS;
}

int btmw_rpc_test_gap_factory_reset(int argc, char **argv)
{
    if (argc != 0)
        return BT_ERR_STATUS_INVALID_PARM_NUMS;

    a_mtkapi_bt_bluetooth_factory_reset();
    return BT_SUCCESS;
}

INT32 btmw_rpc_test_gap_enable_stack_log(int argc, char *argv[])
{
    CHAR *section_name;
    UINT8 trace_level = 0;

    if (argc != 2)
    {
        return BT_ERR_STATUS_INVALID_PARM_NUMS;
    }

    section_name = (CHAR *)argv[0];
    trace_level = atoi(argv[1]);
    a_mtkapi_bt_mw_log_setStackLevel((const char*)section_name, trace_level);
    return BT_SUCCESS;
}


static BTMW_RPC_TEST_CLI btmw_rpc_test_gap_cli_commands[] = {
    { (const char *)"dbg_level",            btmw_rpc_test_gap_set_dbg_level_cli,          (const char *)" = dbg_level <0~9> <0~5>"},
    { (const char *)"power",                btmw_rpc_test_gap_set_power_cli,              (const char *)" = power_on local device, 0:off, 1:on"},
    { (const char *)"name",                 btmw_rpc_test_gap_set_name_cli,               (const char *)" = bt set local dev name, name <xxx>"},
    { (const char *)"set_conn_disc",        btmw_rpc_test_gap_set_conn_disc_cli,          (const char *)" = set device connectable and discoverable, set_conn_disc <1|0> <1|0>, 1:enable, 0: unable"},
    { (const char *)"get_dev_info",         btmw_rpc_test_gap_get_dev_info_cli,           (const char *)" = get device info "},
    { (const char *)"info",                 btmw_rpc_test_gap_get_local_dev_info_cli,     (const char *)" = info <local|update>"},
    { (const char *)"inquiry",              btmw_rpc_test_gap_inquiry_cli,                (const char *)" = start device discovery <all|src|sink|hfp|hid>"},
    { (const char *)"stop_inquiry",         btmw_rpc_test_gap_stop_inquiry_cli,           (const char *)" = stop device discovery"},
    { (const char *)"pair",                 btmw_rpc_test_gap_pair_cli,                   (const char *)" = pair a remote device <addr> <transport> 0:unknown,1:BR/EDR,2:BLE"},
    { (const char *)"unpair",               btmw_rpc_test_gap_remove_paired_dev_cli,      (const char *)" = remove paired device <addr>"},
    { (const char *)"get_rssi",             btmw_rpc_test_gap_get_rssi_cli,               (const char *)" = get_rssi <addr>"},
    { (const char *)"enable_hci_log",       btmw_rpc_test_gap_set_virtual_sniffer_cli,    (const char *)" = enable_hci_log <1|0>. save bluetooth hci snoop log to btsnoop_hci.log"},
    { (const char *)"send_hci",             btmw_rpc_test_gap_send_hci,                   (const char *)" = send_hci <buffer>"},
    { (const char *)"factory_reset",        btmw_rpc_test_gap_factory_reset,              (const char *)" = delete bluetooth files for factory reset"},
    { (const char *)"enable_stack_log",     btmw_rpc_test_gap_enable_stack_log,           (const char *)" = enable_stack_log <trace_name> <level:0~6>"},

    { NULL, NULL, NULL }
};

int btmw_rpc_test_gap_cmd_handler(int argc, char **argv)
{
    BTMW_RPC_TEST_CLI *cmd, *match = NULL;
    int ret = 0;
    int count;

    count = 0;
    cmd = btmw_rpc_test_gap_cli_commands;

    BTMW_RPC_TEST_Logi("[GAP] argc: %d, argv[0]: %s\n", argc, argv[0]);

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
        btmw_rpc_test_print_cmd_help(BTMW_RPC_TEST_CMD_KEY_GAP, btmw_rpc_test_gap_cli_commands);
        ret = -1;
    }
    else
    {
        match->handler(argc - 1, &argv[1]);
    }

    return ret;
}

int btmw_rpc_test_gap_init()
{
    int ret = 0;
    BTMW_RPC_TEST_MOD btmw_rpc_test_gap_mod = {0};

    btmw_rpc_test_gap_mod.mod_id = BTMW_RPC_TEST_MOD_GAP;
    strncpy(btmw_rpc_test_gap_mod.cmd_key, BTMW_RPC_TEST_CMD_KEY_GAP, sizeof(btmw_rpc_test_gap_mod.cmd_key));
    btmw_rpc_test_gap_mod.cmd_handler = btmw_rpc_test_gap_cmd_handler;
    btmw_rpc_test_gap_mod.cmd_tbl = btmw_rpc_test_gap_cli_commands;

    ret = btmw_rpc_test_register_mod(&btmw_rpc_test_gap_mod);
    BTMW_RPC_TEST_Logi("[GAP] btmw_rpc_test_register_mod() returns: %d\n", ret);

    if (!g_cli_pts_mode)
    {
        MTKRPCAPI_BT_APP_CB_FUNC func;
        char pv_tag[2] = {0};
        memset(&func, 0, sizeof(MTKRPCAPI_BT_APP_CB_FUNC));
        func.bt_event_cb = btmw_rpc_test_gap_event_callback;
        func.bt_get_pairing_key_cb = btmw_rpc_test_gap_get_pairing_key_callback;
        func.bt_dev_info_cb = btmw_rpc_test_gap_app_inquiry_callback;
        btmw_rpc_test_gap_bt_base_init(&func, (void *)pv_tag);
        //a_mtkapi_bt_gap_on_off(TRUE);
        //btmw_rpc_test_gap_set_local_name();
        //a_mtkapi_bt_gap_set_connectable_and_discoverable(TRUE, TRUE);
    }
    return 0;
}

