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

#include "u_bt_mw_types.h"
#include "u_bt_mw_common.h"
#include "btmw_test_cli.h"
#include "btmw_test_debug.h"
#include "btmw_test_spp_if.h"
#include "c_bt_mw_spp.h"

typedef unsigned char U8;
typedef unsigned short U16;

CHAR* print_spp_event(BT_SPP_EVENT bt_event)
{
    switch (bt_event)
    {
        case BT_SPP_CONNECT:
            return "BT_SPP_CONNECT";
        case BT_SPP_DISCONNECT:
            return "BT_SPP_DISCONNECT";
        case BT_SPP_RECV_DATA:
            return "BT_SPP_RECV_DATA";
        case BT_SPP_CONNECT_FAIL:
            return "BT_SPP_CONNECT_FAIL";
        case BT_SPP_DISCONNECT_FAIL:
            return "BT_SPP_DISCONNECT_FAIL";
        default:
            break;
    }

    return "";
}

VOID btmw_test_spp_callback(BT_SPP_CBK_STRUCT *pt_spp_struct)
{
    BTMW_TEST_Logd("[SPP] event: %s\n", print_spp_event(pt_spp_struct->event));
    BTMW_TEST_Logv("[SPP]bd_addr:%s\n", pt_spp_struct->bd_addr);
    BTMW_TEST_Logv("[SPP]uuid:%s\n", pt_spp_struct->uuid);
    BTMW_TEST_Logv("[SPP]spp_data:%s\n", pt_spp_struct->spp_data);
    BTMW_TEST_Logv("[SPP]uuid_len:%d\n", pt_spp_struct->uuid_len);
    BTMW_TEST_Logv("[SPP]spp_data_len:%d\n", pt_spp_struct->spp_data_len);
}


static int btmw_test_spp_connect_int_handler(int argc, char *argv[]);
static int btmw_test_spp_disconnect_handler(int argc, char *argv[]);
static int btmw_test_spp_send_data_handler(int argc, char *argv[]);
static int btmw_test_spp_enable_devb_handler(int argc, char *argv[]);
static int btmw_test_spp_disable_devb_handler(int argc, char *argv[]);

static BTMW_TEST_CLI btmw_test_spp_cli_commands[] =
{
    {"connect",             btmw_test_spp_connect_int_handler,        " = connect <addr> <uuid>"},
    {"disconnect",          btmw_test_spp_disconnect_handler,         " = disconnect <addr> <uuid>"},
    {"send_data",           btmw_test_spp_send_data_handler,          " = send data <addr> <uuid> <str>"},
    {"enable_devb",         btmw_test_spp_enable_devb_handler,        " = enable devb <server name> <uuid>"},
    {"disable_devb",        btmw_test_spp_disable_devb_handler,       " = disable devb <uuid>"},
    {NULL, NULL, NULL},
};

static int btmw_test_spp_connect_int_handler(int argc, char *argv[])
{
    CHAR *pbt_addr;
    CHAR *uuid;
    if (argc != 2)
    {
        BTMW_TEST_Loge("[SPP] Usage : connect ([addr][uuid])\n", __func__);
        return -1;
    }
    pbt_addr = argv[0];
    uuid = argv[1];
    c_btm_spp_connect(pbt_addr, uuid);
    return 0;
}

static int btmw_test_spp_disconnect_handler(int argc, char *argv[])
{
    CHAR *pbt_addr;
    CHAR *uuid;
    if (argc != 2)
    {
        BTMW_TEST_Loge("[SPP] Usage : disconnect ([addr][uuid])\n", __func__);
        return -1;
    }
    pbt_addr = argv[0];
    uuid = argv[1];
    c_btm_spp_disconnect(pbt_addr, uuid);
    return 0;
}

static int btmw_test_spp_send_data_handler(int argc, char *argv[])
{
    CHAR *pbt_addr;
    CHAR *uuid;
    CHAR *str;
    INT32 len = 0;
    if (argc != 3)
    {
        BTMW_TEST_Loge("[SPP] Usage : send data ([addr][uuid][str])\n", __func__);
        return -1;
    }
    pbt_addr = argv[0];
    uuid = argv[1];
    str = argv[2];
    len = strlen(str);
    c_btm_spp_send_data(pbt_addr, uuid, str, len);
    return 0;
}

static int btmw_test_spp_enable_devb_handler(int argc, char *argv[])
{
    CHAR *servername;
    CHAR *uuid;
    if (argc != 2)
    {
        BTMW_TEST_Loge("[SPP] Usage : enable devb ([servername][uuid])\n", __func__);
        return -1;
    }
    servername = argv[0];
    uuid = argv[1];
    c_btm_spp_start_server(servername, uuid);
    return 0;
}

static int btmw_test_spp_disable_devb_handler(int argc, char *argv[])
{
    CHAR *uuid;
    if (argc != 2)
    {
        BTMW_TEST_Loge("[SPP] Usage : disable devb ([uuid])\n", __func__);
        return -1;
    }
    uuid = argv[0];
    c_btm_spp_stop_server(uuid);
    return 0;
}


// For handling incoming commands from CLI.
int btmw_test_spp_cmd_handler(int argc, char **argv)
{
    BTMW_TEST_CLI *cmd, *match = NULL;
    int ret = 0;
    int count;

    count = 0;
    cmd = btmw_test_spp_cli_commands;

    BTMW_TEST_Logd("[SPP] argc: %d, argv[0]: %s\n", argc, argv[0]);

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
        btmw_test_print_cmd_help(BTMW_TEST_CMD_KEY_SPP, btmw_test_spp_cli_commands);
        ret = -1;
    }
    else
    {
        match->handler(argc - 1, &argv[1]);
    }

    return ret;
}

int btmw_test_spp_init()
{
    BTMW_TEST_Logd("[SPP] %s() \n", __func__);
    int ret = 0;
    BTMW_TEST_MOD spp_mod = {0};

    // Register command to CLI
    spp_mod.mod_id = BTMW_TEST_MOD_SPP;
    strncpy(spp_mod.cmd_key, BTMW_TEST_CMD_KEY_SPP, sizeof(spp_mod.cmd_key));
    spp_mod.cmd_handler = btmw_test_spp_cmd_handler;
    spp_mod.cmd_tbl = btmw_test_spp_cli_commands;

    ret = btmw_test_register_mod(&spp_mod);
    BTMW_TEST_Logd("[SPP] btmw_test_register_mod() returns: %d\n", ret);

    return ret;
}

int btmw_test_spp_deinit()
{
    return 0;
}
