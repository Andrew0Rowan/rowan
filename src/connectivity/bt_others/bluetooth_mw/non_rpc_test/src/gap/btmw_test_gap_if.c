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

#include "btmw_test_debug.h"
#include "btmw_test_gap_if.h"
#include "btmw_test_cli.h"
#include "c_bt_mw_gap.h"

#define BLUETOOTH_TEST_NAME             "audio_bt"
#define BT_NAME_SUF_TEST_LEN              5

/*-----------------------------------------------------------------------------
                            variable declarations
 -----------------------------------------------------------------------------*/
static BT_APP_CB_FUNC g_btmw_test_gap_callbacks;

extern VOID btmw_test_send_audio_data(VOID);
extern void bt_mw_log_setStackLevel(const char *trc_name, int level);

VOID btmw_test_gap_app_event_callback(tBTMW_GAP_STATE *gap_event)
{
    //BTMW_TEST_Logd("bt_event: %s\n", print_the_event(bt_event));
#if 0
    switch (bt_event)
    {
    case BT_CONNECT_SUCCESS:
        break;
    case BT_DISCONNECTED:
        g_stream_status = 0;
        g_new_PCM_file_flag = 0;
        g_a2dp_connection_established = 0;
        strncpy(btmw_test_default_pcm_file, "/data/usb/music/48000/input_48000.pcm", 127);
        btmw_test_default_pcm_file[127] = 0;/*
        if (btmw_test_stream_handle)
        {
            pthread_kill(btmw_test_stream_handle, 0);
            btmw_test_stream_handle = -1;
        }*/
        c_btm_set_connectable_and_discoverable(TRUE,TRUE);
        break;
    case BT_EXIT:
        break;
    default:
        break;
    }
#endif
}


VOID btmw_test_get_pairing_key_callback(pairing_key_value_t *bt_pairing_key, UINT8 *fg_accept)
{
    //FUNC_ENTRY;
    if (bt_pairing_key->key_type == PIN_CODE)
    {
        BTMW_TEST_Logv("PIN CODE\n");
        BTMW_TEST_Logv("pin code:%s\n", bt_pairing_key->pin_code);
    }
    else if (bt_pairing_key->key_type == PASSKEY)
    {
        BTMW_TEST_Logv("PASS KEY\n");
        BTMW_TEST_Logv("pass key:%d\n", bt_pairing_key->key_value);
    }
    *fg_accept = 1;
    BTMW_TEST_Logv("%s\n", *fg_accept ? "accept" : "reject");
}

VOID btmw_test_gap_app_inquiry_callback(tBTMW_GAP_DEVICE_INFO *pt_result)
{
    if (NULL == pt_result)
    {
        return;
    }

    BTMW_TEST_Logv("device_kind:     %s\n", pt_result->device_kind);
    BTMW_TEST_Logv("name:     %s\n", pt_result->device.name);
    BTMW_TEST_Logv("cod:      0x%2lx\n", pt_result->device.cod);
    BTMW_TEST_Logv("bdAddr:   %s\n", pt_result->device.bdAddr);
}

INT32 btmw_test_gap_set_dbg_level_cli(int argc, char *argv[])
{
    if (argc != 2)
    {
        BTMW_TEST_Loge("Usage :\n");
        BTMW_TEST_Loge("layer <0~8> means set target layer to level\n");
        BTMW_TEST_Loge("if layer is 9 means set all layer to level\n");
        BTMW_TEST_Loge("level <0~5>\n");
        return -1;
    }
    c_btm_set_dbg_level(atoi(argv[0]), atoi(argv[1]));
    return 0;
}

INT32 btmw_test_gap_set_power_cli(int argc, char *argv[])
{
    if (argc != 1)
    {
        BTMW_TEST_Loge("arg num should be 1 \n");
        return -1;
    }

    if (0 == strcmp("0", argv[0]))
    {
        return c_btm_bluetooth_on_off(FALSE);
    }
    else if (0 == strcmp("1", argv[0]))
    {
        return c_btm_bluetooth_on_off(TRUE);
    }

    return 0;
}

INT32 btmw_test_gap_set_name_cli(int argc, char *argv[])
{
    INT32 i4_ret;

    if (argc != 1)
    {
        BTMW_TEST_Logd("arg num should be 1\n");
    }
    if (NULL == argv[0])
    {
        BTMW_TEST_Logd("please attach name after commond just append one string\n");
    }

    BTMW_TEST_Logd("name is: %s\n", argv[0]);
    i4_ret = c_btm_set_local_name(argv[0]);
    if (BT_SUCCESS == i4_ret)
    {
        BTMW_TEST_Logd("set name ok!\n");
    }

    return BT_SUCCESS;
}

INT32 btmw_test_gap_set_conn_disc_cli(int argc, char *argv[])
{
    if (2 == argc)
    {
        if (0 == strcmp("1" , argv[0]))
        {
            if (0 == strcmp("1" , argv[1]))
            {
                return c_btm_set_connectable_and_discoverable(TRUE, TRUE);
            }
            else if (0 == strcmp("0" , argv[1]))
            {
                return c_btm_set_connectable_and_discoverable(TRUE, FALSE);
            }
            else
            {
                BTMW_TEST_Loge("input error, 0 means off, 1 means on\n");
                return -1;
            }
        }
        else if (0 == strcmp("0" , argv[0]))
        {
            if (0 == strcmp("1" , argv[1]))
            {
                return c_btm_set_connectable_and_discoverable(FALSE, TRUE);
            }
            else if (0 == strcmp("0" , argv[1]))
            {
                return c_btm_set_connectable_and_discoverable(FALSE, FALSE);
            }
            else
            {
                BTMW_TEST_Loge("input error, 0 means off, 1 means on\n");
                return BT_ERR_STATUS_PARM_INVALID;
            }
        }
    }
    else
    {
        BTMW_TEST_Loge("arg num should be 2 \n");
        return BT_ERR_STATUS_INVALID_PARM_NUMS;
    }
    return BT_SUCCESS;
}

INT32 btmw_test_gap_get_paired_device_info_cli(int argc, char *argv[])
{
    if (argc != 0)
    {
        BTMW_TEST_Loge("no args \n");
        return -1;
    }

    //c_btm_bluetooth_get_bonded_device();
    return BT_SUCCESS;
}

INT32 btmw_test_gap_get_local_dev_info_cli(int argc, char *argv[])
{
    BT_LOCAL_DEV ps_dev_info;;
    INT32 i4_ret;
    if (argc != 0)
    {
        BTMW_TEST_Loge("no args \n");
        return -1;
    }

    i4_ret = c_btm_get_local_dev_info(&ps_dev_info);

    if (BT_SUCCESS == i4_ret)
    {
        BTMW_TEST_Logd("get local device info ok!\n");
        BTMW_TEST_Logd("ps_dev_info.bdAddr:      %s\n", ps_dev_info.bdAddr);
        BTMW_TEST_Logd("ps_dev_info.name:        %s\n", ps_dev_info.name);
        BTMW_TEST_Logd("ps_dev_info.powered:     %s\n", (ps_dev_info.state == GAP_STATE_ON) ? "on" : "off");
    }
    else
    {
        BTMW_TEST_Logd("get local device info failed!\n");
    }

    return BT_SUCCESS;
}

INT32 btmw_test_gap_inquiry_cli(int argc, char *argv[])
{
    if (argc != 0)
    {
        BTMW_TEST_Loge("no args \n");
        return -1;
    }

    c_btm_start_inquiry_scan(0xff);
    return 0;
}

INT32 btmw_test_gap_stop_inquiry_cli(int argc, char *argv[])
{
    if (argc != 0)
    {
        BTMW_TEST_Loge("no args \n");
        return -1;
    }

    return c_btm_stop_inquiry_scan();
}

INT32 btmw_test_gap_pair_cli(int argc, char *argv[])
{
    UINT32 transport;

    if (argc != 2)
    {
        BTMW_TEST_Logd("parameter num error,please enter two parameter: addr and transport \n");
        return BT_ERR_STATUS_INVALID_PARM_NUMS;
    }
    if (NULL == argv[0])
    {
        BTMW_TEST_Logd("addr is null \n");
        return BT_ERR_STATUS_NULL_POINTER;
    }

    transport = atol(argv[1]);
    if (transport >= 3)
    {
        BTMW_TEST_Logd("transport error should < 3\n");
        return BT_ERR_STATUS_PARM_INVALID;
    }
    return c_btm_pair(argv[0], transport);
}

INT32 btmw_test_gap_remove_paired_dev_cli(int argc, char *argv[])
{
    if (argc != 1)
    {
        BTMW_TEST_Logd("arg num should be 1\n");
        return BT_ERR_STATUS_INVALID_PARM_NUMS;
    }
    if (NULL == argv[0])
    {
        BTMW_TEST_Logd("please attach MAC after commond just append one string\n");
        return BT_ERR_STATUS_NULL_POINTER;
    }

    return c_btm_unpaire(argv[0]);
}

INT32 btmw_test_gap_get_rssi_cli(int argc, char *argv[])
{
    CHAR ps_target_mac[18];
    INT16 rssi_value;

    if (1 == argc)
    {
        memset(ps_target_mac, 0, sizeof(ps_target_mac));
        if (17 > strlen(argv[0]))
        {
            BTMW_TEST_Logd("mac length should be 17\n");
            return BT_ERR_STATUS_PARM_INVALID;
        }
        strncpy(ps_target_mac,argv[0],sizeof(ps_target_mac)-1);
    }
    else
    {
        BTMW_TEST_Logd("arg num should be 1! \n");
        return BT_ERR_STATUS_INVALID_PARM_NUMS;
    }

    return c_btm_get_rssi(ps_target_mac, &rssi_value);;
}

INT32 btmw_test_gap_set_virtual_sniffer_cli(int argc, char *argv[])
{
    INT32 flag;

    if (argc != 1)
    {
        BTMW_TEST_Logd("A parameter is required!\n");
        return BT_ERR_STATUS_INVALID_PARM_NUMS;
    }

    flag = atoi(argv[0]);

    if (flag != 1 || flag != 0)
    {
        BTMW_TEST_Logd("flag should be 1 or 0 !\n");
        return BT_ERR_STATUS_PARM_INVALID;
    }
    c_btm_set_virtual_sniffer(flag);
    return BT_SUCCESS;
}

INT32 btmw_test_gap_send_hci(int argc, char *argv[])
{
    CHAR *hci_cmd;

    if (argc != 1)
        return BT_ERR_STATUS_INVALID_PARM_NUMS;

    hci_cmd = (CHAR *)argv[0];
    c_btm_send_hci(hci_cmd);
    return BT_SUCCESS;
}

INT32 btmw_test_gap_enable_stack_log(int argc, char *argv[])
{
    CHAR *section_name;
    UINT8 trace_level = 0;

    if (argc != 2)
    {
        return BT_ERR_STATUS_INVALID_PARM_NUMS;
    }

    section_name = (CHAR *)argv[0];
    trace_level = atoi(argv[1]);
    bt_mw_log_setStackLevel((const char*)section_name, trace_level);
    return BT_SUCCESS;
}


static BTMW_TEST_CLI btmw_test_gap_cli_commands[] = {
    { "dbg_level",            btmw_test_gap_set_dbg_level_cli,          " = dbg_level <0~9> <0~5>"},
    { "power",                btmw_test_gap_set_power_cli,              " = power_on local device, 0:off, 1:on"},
    { "name",                 btmw_test_gap_set_name_cli,               " = bt set local dev name, name <xxx>"},
    { "set_conn_disc",        btmw_test_gap_set_conn_disc_cli,          " = set device connectable and discoverable, set_conn_disc <1|0> <1|0>, 1:enable, 0: unable"},
    { "get_device_list",      btmw_test_gap_get_paired_device_info_cli, " = get paired device list "},
    { "info",                 btmw_test_gap_get_local_dev_info_cli,     " = info <local|update>"},
    { "inquiry",              btmw_test_gap_inquiry_cli,                " = start device discovery"},
    { "stop_inquiry",         btmw_test_gap_stop_inquiry_cli,           " = Stop device discovery"},
    { "pair",                 btmw_test_gap_pair_cli,                   " = Pair a remote device <addr> <transport> 0:unknown,1:BR/EDR,2:BLE"},
    { "unpair",               btmw_test_gap_remove_paired_dev_cli,      " = remove paired device <addr>"},
    { "get_rssi",             btmw_test_gap_get_rssi_cli,               " = get_rssi <addr>"},
    { "enable_hci_log",       btmw_test_gap_set_virtual_sniffer_cli,    " = enable_hci_log <1|0>. save bluetooth hci snoop log to btsnoop_hci.log"},
    { "send_hci",             btmw_test_gap_send_hci,                   " = send_hci <buffer>"},
    { "enable_stack_log",     btmw_test_gap_enable_stack_log,           " = enable_stack_log <trace_name> <level:0~6>"},

    { NULL, NULL, NULL }
};

int btmw_test_gap_cmd_handler(int argc, char **argv)
{
    BTMW_TEST_CLI *cmd, *match = NULL;
    int ret = 0;
    int count;

    count = 0;
    cmd = btmw_test_gap_cli_commands;

    BTMW_TEST_Logi("[GAP] argc: %d, argv[0]: %s\n", argc, argv[0]);

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
        btmw_test_print_cmd_help(BTMW_TEST_CMD_KEY_GAP, btmw_test_gap_cli_commands);
        ret = -1;
    }
    else
    {
        match->handler(argc - 1, &argv[1]);
    }

    return ret;
}

int btmw_test_gap_set_local_name()
{
    /*
    INT32 i4_ret;
    CHAR ac_bdAddr[MAX_BDADDR_LEN]={0};
    CHAR ac_name[MAX_NAME_LEN] = {0};
    CHAR bt_mac_suf[MAX_BDADDR_LEN] = {0};
    BT_GAP_LOCAL_PROPERTIES_RESULT t_local_info;
    memset(&t_local_info, 0, sizeof(BT_GAP_LOCAL_PROPERTIES_RESULT));

    //get local mac address
    i4_ret = c_btm_get_local_dev_info(&t_local_info);
    strncpy(ac_bdAddr, t_local_info.bdAddr, MAX_BDADDR_LEN);

     //only use the last 5 characters of the bluetooth device mac address
    strncpy(bt_mac_suf, ac_bdAddr + (strlen(ac_bdAddr)-BT_NAME_SUF_TEST_LEN), BT_NAME_SUF_TEST_LEN);

    strncpy(ac_name, BLUETOOTH_TEST_NAME, strlen(BLUETOOTH_TEST_NAME));

    //the bluetooth name: ac_name = BLUETOOTH_TEST_NAME + (bt_mac_suf)
    strcat(ac_name,"(");
    strcat(ac_name,bt_mac_suf);
    strcat(ac_name,")");
    c_btm_set_local_name(ac_name);
    return i4_ret;
    */
}

int btmw_test_gap_reg(void)
{
    int ret = 0;
    BTMW_TEST_MOD btmw_test_gap_mod = {0};

    btmw_test_gap_mod.mod_id = BTMW_TEST_MOD_GAP;
    strncpy(btmw_test_gap_mod.cmd_key, BTMW_TEST_CMD_KEY_GAP, sizeof(btmw_test_gap_mod.cmd_key));
    btmw_test_gap_mod.cmd_handler = btmw_test_gap_cmd_handler;
    btmw_test_gap_mod.cmd_tbl = btmw_test_gap_cli_commands;

    ret = btmw_test_register_mod(&btmw_test_gap_mod);
    BTMW_TEST_Logi("[GAP] btmw_test_register_mod() returns: %d\n", ret);
    if (ret != 0)
    {
        return ret;
    }

    return 0;
}


int btmw_test_gap_init()
{
    btmw_test_gap_reg();

    memset(&g_btmw_test_gap_callbacks, 0, sizeof(BT_APP_CB_FUNC));
    g_btmw_test_gap_callbacks.bt_gap_event_cb = btmw_test_gap_app_event_callback;
    g_btmw_test_gap_callbacks.bt_get_pairing_key_cb = btmw_test_get_pairing_key_callback;
    g_btmw_test_gap_callbacks.bt_dev_info_cb = btmw_test_gap_app_inquiry_callback;

    c_btm_bt_base_init(&g_btmw_test_gap_callbacks);
    //register_player_func();
    //c_btm_bluetooth_on_off(TRUE);
    //btmw_test_gap_set_local_name();
    //c_btm_set_connectable_and_discoverable(TRUE, TRUE);

    return 0;
}

