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
#include <pthread.h>
#include <hardware/bluetooth.h>
#include <hardware/bt_hh.h>

#include "btut_cli.h"
#include "btut_debug.h"
#include "btut_hid_if.h"
#include "util.h"
#include "btif_util.h"

typedef unsigned char U8;
typedef unsigned short U16;
extern const void *btut_gap_get_profile_interface(const char *profile_id);

static int btut_hid_connect_int_handler(int argc, char *argv[]);
static int btut_hid_disconnect_handler(int argc, char *argv[]);
static int btut_hid_set_output_report_handler(int argc, char *argv[]);
static int btut_hid_get_input_report_handler(int argc, char *argv[]);
static int btut_hid_set_feature_report_handler(int argc, char *argv[]);
static int btut_hid_get_feature_report_handler(int argc, char *argv[]);
static int btut_hid_set_protocol_handler(int argc, char *argv[]);
static int btut_hid_get_protocol_handler(int argc, char *argv[]);
static int btut_hid_get_output_report_handler(int argc, char *argv[]);
static int btut_hid_set_input_report_handler(int argc, char *argv[]);

static void btut_hid_connection_state_cb(bt_bdaddr_t *bd_addr, bthh_connection_state_t state)
{
    BTUT_Logd("[HID] %s()\n", __func__);
    switch (state)
    {
        case BTHH_CONN_STATE_CONNECTED:
            BTUT_Logi("[HID] Connection State : connected\n");
            break;
        case BTHH_CONN_STATE_CONNECTING:
            BTUT_Logi("[HID] Connection State : connecting\n");
            break;
        case BTHH_CONN_STATE_DISCONNECTED:
            BTUT_Logi("[HID] Connection State : disconnected\n");
            break;
        case BTHH_CONN_STATE_DISCONNECTING:
            BTUT_Logi("[HID] Connection State : disconnecting\n");
            break;
        default:
            BTUT_Logi("[HID] Connection State : '%d'\n", state);
    }
}

static void btut_hid_virtual_unplug_cb(bt_bdaddr_t *bd_addr, bthh_status_t hh_status)
{
    BTUT_Logd("[HID] %s() state: %s(%d)\n", __func__, (hh_status == 0) ? "SUCCESS" : "FAILED", hh_status);
}

static void btut_hid_info_cb(bt_bdaddr_t *bd_addr, bthh_hid_info_t hid_info)
{
    BTUT_Logd("[HID] %s() \n", __func__);
    BTUT_Logd("[HID] attr_mask = 0x%x\n", hid_info.attr_mask);
    BTUT_Logd("[HID] sub_class = 0x%x\n", hid_info.sub_class);
    BTUT_Logd("[HID] app_id = 0x%x\n", hid_info.app_id);
    BTUT_Logd("[HID] vendor_id = 0x%x\n", hid_info.vendor_id);
    BTUT_Logd("[HID] product_id = 0x%x\n", hid_info.product_id);
    BTUT_Logd("[HID] version = %d\n", hid_info.version);
    BTUT_Logd("[HID] ctry_code = %d\n", hid_info.ctry_code);
    BTUT_Logd("[HID] dl_len = %d\n", hid_info.dl_len);
    BTUT_Logd("[HID] dsc_list = %s\n", hid_info.dsc_list);
}

static void btut_hid_protocol_mode_cb(bt_bdaddr_t *bd_addr, bthh_status_t hh_status, bthh_protocol_mode_t mode)
{
    BTUT_Logd("[HID] %s() state: %s\n", __func__, (hh_status == 0) ? "SUCCESS" : "FAILED");
    BTUT_Logi("[HID] mode = %s(%d)\n", (mode == BTHH_REPORT_MODE) ? "REPORT_MODE" : "BOOT_MODE", mode);
}

static void btut_hid_idle_time_cb(bt_bdaddr_t *bd_addr, bthh_status_t hh_status, int idle_rate)
{
    BTUT_Logd("[HID] %s() state: %s\n", __func__, (hh_status == 0) ? "SUCCESS" : "FAILED");
}

static void btut_hid_get_report_cb(bt_bdaddr_t *bd_addr, bthh_status_t hh_status, uint8_t* rpt_data, int rpt_size)
{
    BTUT_Logd("[HID] %s() state: %s\n", __func__, (hh_status == 0) ? "SUCCESS" : "FAILED");
    BTUT_Logi("Data Len = %d", rpt_size);

    BTUT_Logi("Data = 0x%02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X ",
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
        BTUT_Logd("Report ID is NULL.Report ID cannot be NULL. Invalid HID report recieved \n");
    }
}

static bthh_interface_t *g_bt_hid_interface = NULL;
static bthh_callbacks_t g_bt_hid_callbacks =
{
    sizeof(bthh_callbacks_t),
    btut_hid_connection_state_cb,
    btut_hid_info_cb,
    btut_hid_protocol_mode_cb,
    btut_hid_idle_time_cb,
    btut_hid_get_report_cb,
    btut_hid_virtual_unplug_cb,
};

static BTUT_CLI bt_hid_cli_commands[] =
{
    {"connect",             btut_hid_connect_int_handler,       " = connect"},
    {"disconnect",          btut_hid_disconnect_handler,        " = disconnect"},
    {"get_input_report",    btut_hid_get_input_report_handler,  " = get input report ([addr][report id][max buffer size])"},
    {"get_feature_report",  btut_hid_get_feature_report_handler," = get feature report ([addr][report id][max buffer size])"},
    {"get_output_report",   btut_hid_get_output_report_handler, " = get output report ([addr][report id][max buffer size])"},
    {"set_feature_report",  btut_hid_set_feature_report_handler," = set feature report [addr][report data(hex)]"},
    {"set_output_report",   btut_hid_set_output_report_handler, " = set output report [addr][report data(hex)]"},
    {"set_input_report",    btut_hid_set_input_report_handler,  " = set input report [addr][report data(hex)]"},
    {"set_protocol",        btut_hid_set_protocol_handler,      " = send hid control ([addr][protocol, 1:boot protocol | 0:report protocol]"},
    {"get_protocol",        btut_hid_get_protocol_handler,      " = HID get_protocol = get hid control ([addr])"},
    {NULL, NULL, NULL},
};

static int btut_hid_connect_int_handler(int argc, char *argv[])
{
    char *ptr;
    bt_bdaddr_t bdaddr;

    if (argc != 1)
    {
        BTUT_Loge("[HID] Usage : connect ([addr])\n", __func__);
        return -1;
    }

    ptr = argv[0];
    util_btaddr_stoh(ptr, &bdaddr);
    BTUT_Logi("HID connect to %02X:%02X:%02X:%02X:%02X:%02X\n",
        bdaddr.address[0], bdaddr.address[1], bdaddr.address[2],
        bdaddr.address[3], bdaddr.address[4], bdaddr.address[5]);

    g_bt_hid_interface->connect(&bdaddr);

    return 0;
}

static int btut_hid_disconnect_handler(int argc, char *argv[])
{
    char *ptr;
    bt_bdaddr_t bdaddr;

    if (argc != 1)
    {
        BTUT_Loge("[HID] Usage : disconnect ([addr])\n", __func__);
        return -1;
    }

    ptr = argv[0];
    util_btaddr_stoh(ptr, &bdaddr);
    BTUT_Logi("HID disconnect %02X:%02X:%02X:%02X:%02X:%02X\n",
        bdaddr.address[0], bdaddr.address[1], bdaddr.address[2],
        bdaddr.address[3], bdaddr.address[4], bdaddr.address[5]);

    g_bt_hid_interface->disconnect(&bdaddr);

    return 0;
}

static int btut_hid_set_input_report_handler(int argc, char *argv[])
{
    char *ptr;
    bt_bdaddr_t bdaddr;
    int i = 0;
    U16 rpt_size = strlen(argv[1]);
    int hex_bytes_filled;
    U8 hex_buf[200] = {0};
    U16 hex_len = (strlen(argv[1]) + 1) / 2;

    if (argc != 2)
    {
        BTUT_Loge("[HID] Usage : set_output_report ([addr][report data(hex)])\n", __func__);
        return -1;
    }
    ptr = argv[0];  /*bt address */
    util_btaddr_stoh(ptr, &bdaddr);
    BTUT_Logi("HID set_report %02X:%02X:%02X:%02X:%02X:%02X\n",
            bdaddr.address[0], bdaddr.address[1], bdaddr.address[2],
            bdaddr.address[3], bdaddr.address[4], bdaddr.address[5]);
    BTUT_Logi("%s:rpt_size, hex_len=%d", __func__, rpt_size, hex_len);
    hex_bytes_filled = ascii_2_hex(argv[1], hex_len, hex_buf);
    BTUT_Logi("%s:hex_bytes_filled=%d", __func__, hex_bytes_filled);
    for (i = 0; i < hex_len; i++)
    {
       BTUT_Logi("hex values= %02X", hex_buf[i]);
    }
    if (hex_bytes_filled)
    {
        g_bt_hid_interface->set_report(&bdaddr, BTHH_INPUT_REPORT, (char*)hex_buf);
        BTUT_Logi("set_output_report|");
        return 0;
    }
    else
    {
        BTUT_Logi("%s:hex_bytes_filled <= 0", __func__);
        return -1;
    }
    return 0;
}

static int btut_hid_set_output_report_handler(int argc, char *argv[])
{
    char *ptr;
    bt_bdaddr_t bdaddr;
    int i = 0;
    U16 rpt_size = strlen(argv[1]);
    int hex_bytes_filled;
    U8 hex_buf[200] = {0};
    U16 hex_len = (strlen(argv[1]) + 1) / 2;

    if (argc != 2)
    {
        BTUT_Loge("[HID] Usage : set_output_report ([addr][report data(hex)])\n", __func__);
        return -1;
    }
    ptr = argv[0];  /*bt address */
    util_btaddr_stoh(ptr, &bdaddr);
    BTUT_Logi("HID set_report %02X:%02X:%02X:%02X:%02X:%02X\n",
            bdaddr.address[0], bdaddr.address[1], bdaddr.address[2],
            bdaddr.address[3], bdaddr.address[4], bdaddr.address[5]);
    BTUT_Logi("%s:rpt_size, hex_len=%d", __func__, rpt_size, hex_len);
    hex_bytes_filled = ascii_2_hex(argv[1], hex_len, hex_buf);
    BTUT_Logi("%s:hex_bytes_filled=%d", __func__, hex_bytes_filled);
    for (i = 0; i < hex_len; i++)
    {
       BTUT_Logi("hex values= %02X", hex_buf[i]);
    }
    if (hex_bytes_filled)
    {
        g_bt_hid_interface->set_report(&bdaddr, BTHH_OUTPUT_REPORT, (char*)hex_buf);
        BTUT_Logi("set_output_report|");
        return 0;
    }
    else
    {
        BTUT_Logi("%s:hex_bytes_filled <= 0", __func__);
        return -1;
    }
    return 0;
}

static int btut_hid_get_output_report_handler(int argc, char *argv[])
{
    char *ptr;
    char *ptr1;
    char *ptr2;
    int reportId;
    int bufferSize;
    bt_bdaddr_t bdaddr;

    if (argc != 3)
    {
        BTUT_Loge("[HID] Usage : get_input_report ([addr][report id][max buffer size])\n", __func__);
        return -1;
    }

    ptr = argv[0];  /* bluetooth address of remote device */
    ptr1 = argv[1];  /* report id*/
    ptr2 = argv[2];  /* buffer size */
    reportId = atoi(ptr1);
    bufferSize = atoi(ptr2);
    util_btaddr_stoh(ptr, &bdaddr);
    BTUT_Logi("HID get_report %02X:%02X:%02X:%02X:%02X:%02X\n",
                bdaddr.address[0], bdaddr.address[1], bdaddr.address[2],
                bdaddr.address[3], bdaddr.address[4], bdaddr.address[5]);
    g_bt_hid_interface->get_report(&bdaddr,BTHH_OUTPUT_REPORT,reportId,bufferSize);
    return 0;
}

static int btut_hid_get_input_report_handler(int argc, char *argv[])
{
    char *ptr;
    char *ptr1;
    char *ptr2;
    int reportId;
    int bufferSize;
    bt_bdaddr_t bdaddr;

    if (argc != 3)
    {
        BTUT_Loge("[HID] Usage : get_input_report ([addr][report id][max buffer size])\n", __func__);
        return -1;
    }

    ptr = argv[0];  /* bluetooth address of remote device */
    ptr1 = argv[1];  /* report id*/
    ptr2 = argv[2];  /* buffer size */
    reportId = atoi(ptr1);
    bufferSize = atoi(ptr2);
    util_btaddr_stoh(ptr, &bdaddr);
    BTUT_Logi("HID get_report %02X:%02X:%02X:%02X:%02X:%02X\n",
                bdaddr.address[0], bdaddr.address[1], bdaddr.address[2],
                bdaddr.address[3], bdaddr.address[4], bdaddr.address[5]);
    g_bt_hid_interface->get_report(&bdaddr,BTHH_INPUT_REPORT,reportId,bufferSize);
    return 0;
}

static int btut_hid_get_feature_report_handler(int argc, char *argv[])
{
    char *ptr;
    int reportId;
    int bufferSize;
    bt_bdaddr_t bdaddr;

    BTUT_Logi("[HID] %s()\n", __func__);
    if (argc != 3)
    {
        BTUT_Loge("[HID] Usage : get_feature_report ([addr][report id][max buffer size])\n", __func__);
        return -1;
    }

    ptr = argv[0];              /* bluetooth address of remote device */
    reportId = atoi(argv[1]); /* report id */
    bufferSize = atoi(argv[2]); /* max buffer size */
    util_btaddr_stoh(ptr, &bdaddr);
    BTUT_Logi("HID get_report %02X:%02X:%02X:%02X:%02X:%02X\n",
                bdaddr.address[0], bdaddr.address[1], bdaddr.address[2],
                bdaddr.address[3], bdaddr.address[4], bdaddr.address[5]);
    g_bt_hid_interface->get_report(&bdaddr,BTHH_FEATURE_REPORT,reportId,bufferSize);
    return 0;
}
static int btut_hid_set_protocol_handler(int argc, char *argv[])
{
    char *ptr;
    int protocol_mode;
    bt_bdaddr_t bdaddr;
    BTUT_Logi("[HID] %s()\n", __func__);
    if (argc != 2)
    {
        BTUT_Loge("[HID] Usage : set_protocol ([addr][protocol, 1:boot protocol | 0:report protocol])\n", __func__);
        return -1;
    }
    ptr = argv[0];                  /* bluetooth address of remote device */
    protocol_mode = atoi(argv[1]);  /* protocol mode ( 0:boot protocol, 1:report protocol)  */
    util_btaddr_stoh(ptr, &bdaddr);
    BTUT_Logi("HID set_protocol %02X:%02X:%02X:%02X:%02X:%02X\n",
                bdaddr.address[0], bdaddr.address[1], bdaddr.address[2],
                bdaddr.address[3], bdaddr.address[4], bdaddr.address[5]);
    g_bt_hid_interface->set_protocol(&bdaddr,protocol_mode);
    return 0;
}

static int btut_hid_get_protocol_handler(int argc, char *argv[])
{
    char *ptr;
    bt_bdaddr_t bdaddr;
    BTUT_Logi("[HID] %s()\n", __func__);
    if (argc != 1)
    {
        BTUT_Loge("[HID] Usage : get_protocol ([addr])\n", __func__);
        return -1;
    }
    ptr = argv[0];              /* bluetooth address of remote device */
    util_btaddr_stoh(ptr, &bdaddr);
    BTUT_Logi("HID get_protocol %02X:%02X:%02X:%02X:%02X:%02X\n",
                bdaddr.address[0], bdaddr.address[1], bdaddr.address[2],
                bdaddr.address[3], bdaddr.address[4], bdaddr.address[5]);
    g_bt_hid_interface->get_protocol(&bdaddr,0);
    return 0;
}

static int btut_hid_set_feature_report_handler(int argc, char *argv[])
{
    char *ptr;
    bt_bdaddr_t bdaddr;
    int i;
    U16 rpt_size = strlen(argv[1]);
    int hex_bytes_filled;
    U8 hex_buf[200] = {0};
    U16 hex_len = (strlen(argv[1]) + 1) / 2;

    if (argc != 2)
    {
        BTUT_Loge("[HID] Usage : set_feature_report ([addr][report data(hex)])\n", __func__);
        return -1;
    }

    ptr = argv[0];  /* bluetooth address of remote device */
    util_btaddr_stoh(ptr, &bdaddr);
    BTUT_Logi("HID set_report %02X:%02X:%02X:%02X:%02X:%02X\n",
                bdaddr.address[0], bdaddr.address[1], bdaddr.address[2],
                bdaddr.address[3], bdaddr.address[4], bdaddr.address[5]);
    BTUT_Logi("%s:rpt_size, hex_len=%d", __func__, rpt_size, hex_len);
    hex_bytes_filled = ascii_2_hex(argv[1], hex_len, hex_buf);
    BTUT_Logi("%s:hex_bytes_filled=%d", __func__, hex_bytes_filled);
    for (i = 0; i < hex_len; i++)
    {
        BTUT_Logi("hex values= %02X",hex_buf[i]);
    }
    if (hex_bytes_filled)
    {
        g_bt_hid_interface->set_report(&bdaddr, BTHH_FEATURE_REPORT, (char*)hex_buf);
        BTUT_Logi("set_feature_report");
        return 0;
    }
    else
    {
        BTUT_Logi("%s:hex_bytes_filled <= 0", __func__);
        return 0;
    }
    return 0;
}

// For handling incoming commands from CLI.
int btut_hid_cmd_handler(int argc, char **argv)
{
    BTUT_CLI *cmd, *match = NULL;
    int ret = 0;
    int count;

    count = 0;
    cmd = bt_hid_cli_commands;

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
        btut_print_cmd_help(CMD_KEY_HID, bt_hid_cli_commands);
        ret = -1;
    }
    else
    {
        match->handler(argc - 1, &argv[1]);
    }

    return ret;
}

int btut_hid_init()
{
    int ret = 0;
    BTUT_MOD hid_mod = {0};

    // Get HID interface
    g_bt_hid_interface = (bthh_interface_t *) btut_gap_get_profile_interface(BT_PROFILE_HIDHOST_ID);
    if (g_bt_hid_interface == NULL)
    {
        BTUT_Loge("[HID] Failed to get HID interface\n");
        return -1;
    }

     // Init HID interface
    if (g_bt_hid_interface->init(&g_bt_hid_callbacks) != BT_STATUS_SUCCESS)
    {
        BTUT_Loge("[HID] Failed to init HID interface\n");
        return -1;
    }

    // Register command to CLI
    hid_mod.mod_id = BTUT_MOD_HID;
    strncpy(hid_mod.cmd_key, CMD_KEY_HID, sizeof(hid_mod.cmd_key));
    hid_mod.cmd_handler = btut_hid_cmd_handler;
    hid_mod.cmd_tbl = bt_hid_cli_commands;

    ret = btut_register_mod(&hid_mod);
    BTUT_Logi("[HID] btut_register_mod() returns: %d\n", ret);

    return ret;
}

int btut_hid_deinit()
{
    // Deinit HID interface
    if (g_bt_hid_interface != NULL)
    {
        g_bt_hid_interface->cleanup();
    }

    return 0;
}
