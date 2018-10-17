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
#include "btmw_rpc_test_hid_if.h"
#include "mtk_bt_service_hidh_wrapper.h"

typedef unsigned char U8;
typedef unsigned short U16;
//extern void *btmw_rpc_test_gap_get_profile_interface(const char *profile_id);

static int btmw_rpc_test_hid_connect_handler(int argc, char *argv[]);
static int btmw_rpc_test_hid_disconnect_handler(int argc, char *argv[]);
static int btmw_rpc_test_hid_set_output_report_handler(int argc, char *argv[]);
static int btmw_rpc_test_hid_get_input_report_handler(int argc, char *argv[]);
static int btmw_rpc_test_hid_set_feature_report_handler(int argc, char *argv[]);
static int btmw_rpc_test_hid_get_feature_report_handler(int argc, char *argv[]);
static int btmw_rpc_test_hid_set_protocol_handler(int argc, char *argv[]);
static int btmw_rpc_test_hid_get_protocol_handler(int argc, char *argv[]);
static int btmw_rpc_test_hid_get_output_report_handler(int argc, char *argv[]);
static int btmw_rpc_test_hid_set_input_report_handler(int argc, char *argv[]);

#if 0
static void btmw_rpc_test_hid_connection_state_cb(bt_bdaddr_t *bd_addr, bthh_connection_state_t state)
{
    BTMW_RPC_TEST_Logd("[HID] %s()\n", __func__);
    switch (state)
    {
        case BTHH_CONN_STATE_CONNECTED:
            BTMW_RPC_TEST_Logi("[HID] Connection State : connected\n");
            break;
        case BTHH_CONN_STATE_CONNECTING:
            BTMW_RPC_TEST_Logi("[HID] Connection State : connecting\n");
            break;
        case BTHH_CONN_STATE_DISCONNECTED:
            BTMW_RPC_TEST_Logi("[HID] Connection State : disconnected\n");
            break;
        case BTHH_CONN_STATE_DISCONNECTING:
            BTMW_RPC_TEST_Logi("[HID] Connection State : disconnecting\n");
            break;
        default:
            BTMW_RPC_TEST_Logi("[HID] Connection State : '%d'\n", state);
    }
}

static void btmw_rpc_test_hid_virtual_unplug_cb(bt_bdaddr_t *bd_addr, bthh_status_t hh_status)
{
    BTMW_RPC_TEST_Logd("[HID] %s() state: %s(%d)\n", __func__, (hh_status == 0) ? "SUCCESS" : "FAILED", hh_status);
}

static void btmw_rpc_test_hid_info_cb(bt_bdaddr_t *bd_addr, bthh_hid_info_t hid_info)
{
    BTMW_RPC_TEST_Logd("[HID] %s() \n", __func__);
    BTMW_RPC_TEST_Logd("[HID] attr_mask = 0x%x\n", hid_info.attr_mask);
    BTMW_RPC_TEST_Logd("[HID] sub_class = 0x%x\n", hid_info.sub_class);
    BTMW_RPC_TEST_Logd("[HID] app_id = 0x%x\n", hid_info.app_id);
    BTMW_RPC_TEST_Logd("[HID] vendor_id = 0x%x\n", hid_info.vendor_id);
    BTMW_RPC_TEST_Logd("[HID] product_id = 0x%x\n", hid_info.product_id);
    BTMW_RPC_TEST_Logd("[HID] version = %d\n", hid_info.version);
    BTMW_RPC_TEST_Logd("[HID] ctry_code = %d\n", hid_info.ctry_code);
    BTMW_RPC_TEST_Logd("[HID] dl_len = %d\n", hid_info.dl_len);
    BTMW_RPC_TEST_Logd("[HID] dsc_list = %s\n", hid_info.dsc_list);
}

static void btmw_rpc_test_hid_protocol_mode_cb(bt_bdaddr_t *bd_addr, bthh_status_t hh_status, bthh_protocol_mode_t mode)
{
    BTMW_RPC_TEST_Logd("[HID] %s() state: %s\n", __func__, (hh_status == 0) ? "SUCCESS" : "FAILED");
    BTMW_RPC_TEST_Logi("[HID] mode = %s(%d)\n", (mode == BTHH_REPORT_MODE) ? "REPORT_MODE" : "BOOT_MODE", mode);
}

static void btmw_rpc_test_hid_idle_time_cb(bt_bdaddr_t *bd_addr, bthh_status_t hh_status, int idle_rate)
{
    BTMW_RPC_TEST_Logd("[HID] %s() state: %s\n", __func__, (hh_status == 0) ? "SUCCESS" : "FAILED");
}

static void btmw_rpc_test_hid_get_report_cb(bt_bdaddr_t *bd_addr, bthh_status_t hh_status, uint8_t* rpt_data, int rpt_size)
{
    BTMW_RPC_TEST_Logd("[HID] %s() state: %s\n", __func__, (hh_status == 0) ? "SUCCESS" : "FAILED");
    BTMW_RPC_TEST_Logi("Data Len = %d", rpt_size);

    BTMW_RPC_TEST_Logi("Data = 0x%02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X ",
                rpt_size>0?rpt_data[0]:0,
                rpt_size>1?rpt_data[1]:0,
                rpt_size>2?rpt_data[2]:0,
                rpt_size>3?rpt_data[3]:0,
                rpt_size>4?rpt_data[4]:0,
                rpt_size>5?rpt_data[5]:0,
                rpt_size>6?rpt_data[6]:0,
                rpt_size>7?rpt_data[7]:0,
                rpt_size>9?rpt_data[8]:0,
                rpt_size>10?rpt_data[9]:0,
                rpt_size>11?rpt_data[10]:0,
                rpt_size>12?rpt_data[11]:0,
                rpt_size>13?rpt_data[12]:0,
                rpt_size>14?rpt_data[13]:0,
                rpt_size>15?rpt_data[14]:0,
                rpt_size>16?rpt_data[15]:0);

    if (rpt_data[0] == 0)
    {
        BTMW_RPC_TEST_Logd("Report ID is NULL.Report ID cannot be NULL. Invalid HID report recieved \n");
    }
}

static bthh_interface_t *g_bt_hid_interface = NULL;
static bthh_callbacks_t g_bt_hid_callbacks =
{
    sizeof(bthh_callbacks_t),
    btmw_rpc_test_hid_connection_state_cb,
    btmw_rpc_test_hid_info_cb,
    btmw_rpc_test_hid_protocol_mode_cb,
    btmw_rpc_test_hid_idle_time_cb,
    btmw_rpc_test_hid_get_report_cb,
    btmw_rpc_test_hid_virtual_unplug_cb,
};
#endif


static BTMW_RPC_TEST_CLI btmw_rpc_test_hid_cli_commands[] =
{
    {"connect",             btmw_rpc_test_hid_connect_handler,        " = connect <addr>"},
    {"disconnect",          btmw_rpc_test_hid_disconnect_handler,         " = disconnect <addr>"},
    {"get_input_report",    btmw_rpc_test_hid_get_input_report_handler,   " = get input report <addr> <report id> <max buffer size>"},
    {"get_feature_report",  btmw_rpc_test_hid_get_feature_report_handler, " = get feature report <addr> <report id> <max buffer size>"},
    {"get_output_report",   btmw_rpc_test_hid_get_output_report_handler,  " = get output report <addr> <report id> <max buffer size>"},
    {"set_feature_report",  btmw_rpc_test_hid_set_feature_report_handler, " = set feature report <addr> <report data(hex)>"},
    {"set_output_report",   btmw_rpc_test_hid_set_output_report_handler,  " = set output report <addr> <report data(hex)>"},
    {"set_input_report",    btmw_rpc_test_hid_set_input_report_handler,   " = set input report <addr> <report data(hex)>"},
    {"set_protocol",        btmw_rpc_test_hid_set_protocol_handler,       " = set protocol <addr> <protocol(0:boot, 1:report)>"},
    {"get_protocol",        btmw_rpc_test_hid_get_protocol_handler,       " = get protocol <addr>"},
    {NULL, NULL, NULL},
};

static int btmw_rpc_test_hid_connect_handler(int argc, char *argv[])
{
    BTMW_RPC_TEST_Logi("[HID] %s()\n", __func__);
    CHAR ps_addr[MAX_BDADDR_LEN];

    if (argc != 1)
    {
        BTMW_RPC_TEST_Loge("[HID] Usage : connect ([addr])\n", __func__);
        return -1;
    }

    strncpy(ps_addr,argv[0], MAX_BDADDR_LEN);
    ps_addr[MAX_BDADDR_LEN - 1] = '\0';

    return a_mtkapi_hidh_connect(ps_addr);
}

static int btmw_rpc_test_hid_disconnect_handler(int argc, char *argv[])
{
    BTMW_RPC_TEST_Logi("[HID] %s()\n", __func__);
    CHAR ps_addr[MAX_BDADDR_LEN];

    if (argc != 1)
    {
        BTMW_RPC_TEST_Loge("[HID] Usage : disconnect ([addr])\n", __func__);
        return -1;
    }
    strncpy(ps_addr,argv[0], MAX_BDADDR_LEN);
    ps_addr[MAX_BDADDR_LEN - 1] = '\0';

    return a_mtkapi_hidh_disconnect(ps_addr);
}

static int btmw_rpc_test_hid_set_input_report_handler(int argc, char *argv[])
{
    BTMW_RPC_TEST_Logi("[HID] %s()\n", __func__);
    CHAR ps_addr[MAX_BDADDR_LEN];
    CHAR *report_data;  //lyyang todo

    if (argc != 2)
    {
        BTMW_RPC_TEST_Loge("[HID] Usage : set_output_report ([addr][report data(hex)])\n", __func__);
        return -1;
    }
    strncpy(ps_addr,argv[0], MAX_BDADDR_LEN);
    ps_addr[MAX_BDADDR_LEN - 1] = '\0';
    report_data = argv[1];

    return a_mtkapi_hidh_set_input_report(ps_addr, report_data);
}

static int btmw_rpc_test_hid_set_output_report_handler(int argc, char *argv[])
{
    BTMW_RPC_TEST_Logi("[HID] %s()\n", __func__);
    CHAR ps_addr[MAX_BDADDR_LEN];
    CHAR *report_data;

    if (argc != 2)
    {
        BTMW_RPC_TEST_Loge("[HID] Usage : set_output_report ([addr][report data(hex)])\n", __func__);
        return -1;
    }
    strncpy(ps_addr,argv[0], MAX_BDADDR_LEN);
    ps_addr[MAX_BDADDR_LEN - 1] = '\0';
    report_data = argv[1];

    return a_mtkapi_hidh_set_output_report(ps_addr, report_data);
}

static int btmw_rpc_test_hid_get_output_report_handler(int argc, char *argv[])
{
    BTMW_RPC_TEST_Logi("[HID] %s()\n", __func__);
    CHAR ps_addr[MAX_BDADDR_LEN];
    char *ptr1;
    char *ptr2;
    int reportId;
    int bufferSize;
    if (argc != 3)
    {
        BTMW_RPC_TEST_Loge("[HID] Usage : get_input_report ([addr][report id][max buffer size])\n", __func__);
        return -1;
    }

    strncpy(ps_addr,argv[0], MAX_BDADDR_LEN);
    ps_addr[MAX_BDADDR_LEN - 1] = '\0';  /* bluetooth address of remote device */
    ptr1 = argv[1];  /* report id*/
    ptr2 = argv[2];  /* buffer size */
    reportId = atoi(ptr1);
    bufferSize = atoi(ptr2);

    return a_mtkapi_hidh_get_output_report(ps_addr, reportId, bufferSize);
}

static int btmw_rpc_test_hid_get_input_report_handler(int argc, char *argv[])
{
    BTMW_RPC_TEST_Logi("[HID] %s()\n", __func__);
    CHAR ps_addr[MAX_BDADDR_LEN];
    char *ptr1;
    char *ptr2;
    int reportId;
    int bufferSize;
    if (argc != 3)
    {
        BTMW_RPC_TEST_Loge("[HID] Usage : get_input_report ([addr][report id][max buffer size])\n", __func__);
        return -1;
    }

    strncpy(ps_addr,argv[0], MAX_BDADDR_LEN);
    ps_addr[MAX_BDADDR_LEN - 1] = '\0';   /* bluetooth address of remote device */
    ptr1 = argv[1];  /* report id*/
    ptr2 = argv[2];  /* buffer size */
    reportId = atoi(ptr1);
    bufferSize = atoi(ptr2);

    return a_mtkapi_hidh_get_input_report(ps_addr, reportId, bufferSize);
}

static int btmw_rpc_test_hid_get_feature_report_handler(int argc, char *argv[])
{
    BTMW_RPC_TEST_Logi("[HID] %s()\n", __func__);
    CHAR ps_addr[MAX_BDADDR_LEN];
    int reportId;
    int bufferSize;

    BTMW_RPC_TEST_Logi("[HID] %s()\n", __func__);
    if (argc != 3)
    {
        BTMW_RPC_TEST_Loge("[HID] Usage : get_feature_report ([addr][report id][max buffer size])\n", __func__);
        return -1;
    }

    strncpy(ps_addr,argv[0], MAX_BDADDR_LEN);
    ps_addr[MAX_BDADDR_LEN - 1] = '\0';            /* bluetooth address of remote device */
    reportId = atoi(argv[1]); /* report id */
    bufferSize = atoi(argv[2]); /* max buffer size */

    return a_mtkapi_hidh_get_feature_report(ps_addr, reportId, bufferSize);
}
static int btmw_rpc_test_hid_set_protocol_handler(int argc, char *argv[])
{
    BTMW_RPC_TEST_Logi("[HID] %s()\n", __func__);
    CHAR ps_addr[MAX_BDADDR_LEN];
    int protocol_mode;

    if (argc != 2)
    {
        BTMW_RPC_TEST_Loge("[HID] Usage : set_protocol ([addr][protocol, 1:boot protocol | 0:report protocol])\n", __func__);
        return -1;
    }
    strncpy(ps_addr,argv[0], MAX_BDADDR_LEN);
    ps_addr[MAX_BDADDR_LEN - 1] = '\0';                   /* bluetooth address of remote device */
    protocol_mode = atoi(argv[1]);  /* protocol mode ( 0:boot protocol, 1:report protocol)  */

    return a_mtkapi_hidh_set_protocol(ps_addr, protocol_mode);
}

static int btmw_rpc_test_hid_get_protocol_handler(int argc, char *argv[])
{
    BTMW_RPC_TEST_Logi("[HID] %s()\n", __func__);
    CHAR ps_addr[MAX_BDADDR_LEN];

    if (argc != 1)
    {
        BTMW_RPC_TEST_Loge("[HID] Usage : get_protocol ([addr])\n", __func__);
        return -1;
    }

    strncpy(ps_addr,argv[0], MAX_BDADDR_LEN);
    ps_addr[MAX_BDADDR_LEN - 1] = '\0';              /* bluetooth address of remote device */

    return a_mtkapi_hidh_get_protocol(ps_addr);
}

static int btmw_rpc_test_hid_set_feature_report_handler(int argc, char *argv[])
{
    BTMW_RPC_TEST_Logi("[HID] %s()\n", __func__);
    CHAR ps_addr[MAX_BDADDR_LEN];
    CHAR *report_data;

    if (argc != 2)
    {
        BTMW_RPC_TEST_Loge("[HID] Usage : set_feature_report ([addr][report data(hex)])\n", __func__);
        return -1;
    }

    strncpy(ps_addr,argv[0], MAX_BDADDR_LEN);
    ps_addr[MAX_BDADDR_LEN - 1] = '\0';   /* bluetooth address of remote device */
    report_data = argv[1];

    return a_mtkapi_hidh_set_feature_report(ps_addr, report_data);
}

// For handling incoming commands from CLI.
int btmw_rpc_test_hid_cmd_handler(int argc, char **argv)
{
    BTMW_RPC_TEST_CLI *cmd, *match = NULL;
    int ret = 0;
    int count;

    count = 0;
    cmd = btmw_rpc_test_hid_cli_commands;

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
        btmw_rpc_test_print_cmd_help(BTMW_RPC_TEST_CMD_KEY_HID, btmw_rpc_test_hid_cli_commands);
        ret = -1;
    }
    else
    {
        match->handler(argc - 1, &argv[1]);
    }

    return ret;
}


#define BTMW_RPC_HIDH_CASE_RETURN_STR(const) case const: return #const;

static CHAR* btmw_rpc_test_hidh_app_event(BT_HIDH_EVENT event)
{
    switch((int)event)
    {
        BTMW_RPC_HIDH_CASE_RETURN_STR(BT_HIDH_CONNECTED)
        BTMW_RPC_HIDH_CASE_RETURN_STR(BT_HIDH_DISCONNECTED)
        BTMW_RPC_HIDH_CASE_RETURN_STR(BT_HIDH_CONNECT_FAIL)
        default: return "UNKNOWN_EVENT";
   }
}

static VOID btmw_rpc_test_hidh_app_cbk(BT_HIDH_CBK_STRUCT *param, VOID *pv_tag)
{
    if (NULL == param)
    {
        BTMW_RPC_TEST_Loge("btmw_rpc_test_hidh_app_cbk: param is NULL\n");
        return;
    }

    BTMW_RPC_TEST_Loge("addr=%s, event=%d, %s\n", param->addr, param->event,
        btmw_rpc_test_hidh_app_event(param->event));
    switch (param->event)
    {
        default:
            break;
    }
    return;
}


int btmw_rpc_test_hid_init()
{
    int ret = 0;
    BTMW_RPC_TEST_MOD hid_mod = {0};
    BTMW_RPC_TEST_Logi("[HID] btmw_rpc_test_hid_init() init\n");

    // Register command to CLI
    hid_mod.mod_id = BTMW_RPC_TEST_MOD_HID;
    strncpy(hid_mod.cmd_key, BTMW_RPC_TEST_CMD_KEY_HID, sizeof(hid_mod.cmd_key));
    hid_mod.cmd_handler = btmw_rpc_test_hid_cmd_handler;
    hid_mod.cmd_tbl = btmw_rpc_test_hid_cli_commands;

    ret = btmw_rpc_test_register_mod(&hid_mod);
    BTMW_RPC_TEST_Logi("[HID] btmw_rpc_test_register_mod() returns: %d\n", ret);
    if (!g_cli_pts_mode)
    {
        a_mtkapi_hidh_register_callback(btmw_rpc_test_hidh_app_cbk, NULL);
        BTMW_RPC_TEST_Logi("[HID] a_mtkapi_hidh_register_callback() \n");
    }
    return ret;
}

int btmw_rpc_test_hid_deinit()
{
    return 0;
}
