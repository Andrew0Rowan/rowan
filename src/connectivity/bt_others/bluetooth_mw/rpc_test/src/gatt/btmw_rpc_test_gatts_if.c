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
#include <stdio.h>
#include <stdlib.h>

#include "btmw_rpc_test_cli.h"
#include "btmw_rpc_test_debug.h"
#include "btmw_rpc_test_gatts_if.h"
#include "mtk_bt_service_gatts_wrapper.h"

extern INT32 g_btm_rpc_client_if;

static void btmw_rpc_test_gatts_register_server_callback(BT_GATTS_REG_SERVER_RST_T *bt_gatts_reg_server,
                                                                       void* pv_tag)
{
    if (NULL == bt_gatts_reg_server)
    {
        BTMW_RPC_TEST_Loge("[GATTS] %s(), bt_gatts_reg_server is NULL\n", __func__);
        return;
    }
    BTMW_RPC_TEST_Logi("[GATTS] %s(), server_if = %ld\n", __func__, (long)bt_gatts_reg_server->server_if);
    return;
}

static void btmw_rpc_test_gatts_event_callback(BT_GATTS_EVENT_T bt_gatts_event,
                                                           void* pv_tag)
{
    BTMW_RPC_TEST_Logi("[GATTS] %s(), bt_gatts_event =%d\n",  __func__, bt_gatts_event);
#if 0
    INT32 ret = 0;
    if (BT_GATTS_CONNECT == bt_gatts_event)
    {
        ret = a_mtkapi_bt_gattc_multi_adv_disable(g_btm_rpc_client_if);
        if (ret != BT_SUCCESS)
        {
            BTMW_RPC_TEST_Loge("[GATTC] disable advertisement failed %d\n", ret);
        }
    }
    else if (BT_GATTS_DISCONNECT == bt_gatts_event)
    {
        ret = a_mtkapi_bt_gattc_multi_adv_enable(g_btm_rpc_client_if, 2048, 2048, 0, 0, 1, 0);
        if (ret != BT_SUCCESS)
        {
            BTMW_RPC_TEST_Loge("[GATTC] enable advertisement failed %d\n", ret);
        }
    }
#endif
    return;
}

static void btmw_rpc_test_gatts_add_srvc_callback(BT_GATTS_ADD_SRVC_RST_T *bt_gatts_add_srvc,
                                                               void* pv_tag)
{
    if (NULL == bt_gatts_add_srvc)
    {
        BTMW_RPC_TEST_Loge("[GATTS] %s(), bt_gatts_add_srvc is NULL\n", __func__);
        return;
    }
    BTMW_RPC_TEST_Logi("[GATTS] %s(), bt_gatts_add_srvc->srvc_handle =%ld, uuid = %s\n",
        __func__, (long)bt_gatts_add_srvc->srvc_handle, bt_gatts_add_srvc->srvc_id.id.uuid);
}

static void btmw_rpc_test_gatts_add_incl_callback(BT_GATTS_ADD_INCL_RST_T *bt_gatts_add_incl,
                                                              void* pv_tag)
{
    if (NULL == bt_gatts_add_incl)
    {
        BTMW_RPC_TEST_Loge("[GATTS] %s(), bt_gatts_add_incl is NULL\n", __func__);
        return;
    }
    BTMW_RPC_TEST_Logi("[GATTS] %s(), bt_gatts_add_incl->incl_srvc_handle =%ld\n",
                       __func__, (long)bt_gatts_add_incl->incl_srvc_handle);
}

static void btmw_rpc_test_gatts_add_char_callback(BT_GATTS_ADD_CHAR_RST_T *bt_gatts_add_char,
                                                               void* pv_tag)
{
    if (NULL == bt_gatts_add_char)
    {
        BTMW_RPC_TEST_Loge("[GATTS] %s(), bt_gatts_add_char is NULL\n", __func__);
        return;
    }
    BTMW_RPC_TEST_Logi("[GATTS] %s(), bt_gatts_add_char->char_handle =%ld, uuid = %s\n",
        __func__, (long)bt_gatts_add_char->char_handle, bt_gatts_add_char->uuid);
}

static void btmw_rpc_test_gatts_add_desc_callback(BT_GATTS_ADD_DESCR_RST_T *bt_gatts_add_desc,
                                                                void* pv_tag)
{
    if (NULL == bt_gatts_add_desc)
    {
        BTMW_RPC_TEST_Loge("[GATTS] %s(), bt_gatts_add_desc is NULL\n", __func__);
        return;
    }
    BTMW_RPC_TEST_Logi("[GATTS] %s(), bt_gatts_add_desc->descr_handle =%ld, uuid = %s\n",
        __func__, (long)bt_gatts_add_desc->descr_handle, bt_gatts_add_desc->uuid);
}

static void btmw_rpc_test_gatts_op_srvc_callback(BT_GATTS_SRVC_OP_TYPE_T op_type,
                                                              BT_GATTS_SRVC_RST_T *bt_gatts_srvc,
                                                              void* pv_tag)
{
    if (NULL == bt_gatts_srvc)
    {
        BTMW_RPC_TEST_Loge("[GATTS] %s(), bt_gatts_srvc is NULL\n", __func__);
        return;
    }
    BTMW_RPC_TEST_Logi("[GATTS] %s(), op_type =%ld, srvc_handle = %ld\n",
                       __func__, (long)op_type, (long)bt_gatts_srvc->srvc_handle);
}

static void btmw_rpc_test_gatts_req_read_callback(BT_GATTS_REQ_READ_RST_T *bt_gatts_read,
                                                               void* pv_tag)
{
    if (NULL == bt_gatts_read)
    {
        BTMW_RPC_TEST_Loge("[GATTS] %s(), bt_gatts_read is NULL\n", __func__);
        return;
    }
    BTMW_RPC_TEST_Logi("[GATTS] %s(), attr_handle =%ld, is_long = %d, btaddr = %s\n",
                       __func__, (long)bt_gatts_read->attr_handle,
                       bt_gatts_read->is_long, bt_gatts_read->btaddr);
}

static void btmw_rpc_test_gatts_req_write_callback(BT_GATTS_REQ_WRITE_RST_T *bt_gatts_write,
                                                                void* pv_tag)
{
    if (NULL == bt_gatts_write)
    {
        BTMW_RPC_TEST_Loge("[GATTS] %s(), bt_gatts_write is NULL\n", __func__);
        return;
    }
    BTMW_RPC_TEST_Logi("[GATTS] %s(), attr_handle =%ld, is_prep = %d, btaddr = %s, value = %s\n",
        __func__, (long)bt_gatts_write->attr_handle, bt_gatts_write->is_prep,
        bt_gatts_write->btaddr, bt_gatts_write->value);
}

static void btmw_rpc_test_gatts_ind_sent_callback(INT32 conn_id,
                                                               INT32 status,
                                                               void* pv_tag)
{
    BTMW_RPC_TEST_Logi("[GATTS] %s(), status =%ld, conn_id = %ld\n",
                       __func__, (long)status, (long)conn_id);
}

static INT32 btmw_rpc_test_gatts_base_init(MTKRPCAPI_BT_APP_GATTS_CB_FUNC_T *func,
                                                     void *pv_tag)
{
    BTMW_RPC_TEST_Logi("[GATTC] btmw_rpc_test_gatts_base_init\n");
    return a_mtkapi_bt_gatts_base_init(func, pv_tag);
}

static int btmw_rpc_test_gatts_register_server(int argc, char **argv)
{
    CHAR pt_service_uuid[130];
    BTMW_RPC_TEST_Logi("[GATTS] %s()\n", __func__);

    if (argc < 1)
    {
        return -1;
    }
    strncpy(pt_service_uuid,argv[0], strlen(argv[0]));
    pt_service_uuid[strlen(argv[0])] = '\0';
    return a_mtkapi_bt_gatts_register_server(pt_service_uuid);
}

static int btmw_rpc_test_gatts_unregister_server(int argc, char **argv)
{
    BTMW_RPC_TEST_Logi("[GATTS] %s()\n", __func__);
    if (argc < 1)
    {
        BTMW_RPC_TEST_Logi("Usage :\n");
        BTMW_RPC_TEST_Logi("  GATTS unregister_server <server_if>\n");
        return -1;
    }
    INT32 server_if = 0;
    server_if = atoi(argv[0]);
    return a_mtkapi_bt_gatts_unregister_server(server_if);
}

static int btmw_rpc_test_gatts_open(int argc, char **argv)
{
    BTMW_RPC_TEST_Logi("[GATTS] %s()\n", __func__);

    CHAR ps_addr[MAX_BDADDR_LEN];
    UINT8 is_direct = 0;
    INT32 transport = 0;
    INT32 server_if = 0;

    if (argc < 2)
    {
        BTMW_RPC_TEST_Logi("Usage :\n");
        BTMW_RPC_TEST_Logi("  GATTS open <server_if> <addr> [isDirect <true|false> [<transport>]]\n");
        return -1;
    }
    server_if = atoi(argv[0]);
    strncpy(ps_addr, argv[1], MAX_BDADDR_LEN);
    ps_addr[MAX_BDADDR_LEN - 1] = '\0';

    if (argc >= 3) { // set isDirect, opt.
        CHAR *temp = argv[2];
        if (strcmp(temp,"1")||strcmp(temp,"true")||strcmp(temp,"TRUE"))
        {
            is_direct = 1;
        }
        else
        {
            is_direct = 0;
        }
    }
    if (argc >= 4)
    {
        // set transport, opt.
        CHAR *temp = argv[3];
        transport = atoi(temp);
    }
    return a_mtkapi_bt_gatts_open(server_if, ps_addr, is_direct, transport);
}

static int btmw_rpc_test_gatts_close(int argc, char **argv)
{
    BTMW_RPC_TEST_Logi("[GATTS] %s()\n", __func__);
    CHAR ps_addr[MAX_BDADDR_LEN];
    INT32 server_if = 0;
    INT32 conn_id = 0;

    if (argc < 3)
    {
        BTMW_RPC_TEST_Logi("Usage :\n");
        BTMW_RPC_TEST_Logi("  GATTS close <server_if> <addr> <conn_id>\n");
        return -1;
    }
    server_if = atoi(argv[0]);
    strncpy(ps_addr,argv[1], MAX_BDADDR_LEN);
    ps_addr[MAX_BDADDR_LEN - 1] = '\0';
    conn_id = atoi(argv[2]);
    return a_mtkapi_bt_gatts_close(server_if, ps_addr, conn_id);
}

static int btmw_rpc_test_gatts_add_service (int argc, char **argv)
{

    BTMW_RPC_TEST_Logi("[GATTS] %s()\n", __func__);
    CHAR pt_uuid[130];
    INT32 number = 2;
    UINT8 is_primary = 1;
    INT32 server_if = 0;

    if (argc < 2)
    {
        BTMW_RPC_TEST_Logi("Usage :\n");
        BTMW_RPC_TEST_Logi("  GATTS add_service <server_if> <uuid> [is_primary <true|false> [<number_of_handles>]]\n");
        return -1;
    }
    server_if = atoi(argv[0]);
    strncpy(pt_uuid, argv[1], strlen(argv[1]));
    pt_uuid[strlen(argv[1])] = '\0';

    if (argc >= 3)
    {
        // set is_primary, opt.
        CHAR *temp = argv[2];
        if (strcmp(temp,"1")||strcmp(temp,"true")||strcmp(temp,"TRUE"))
        {
            is_primary = 1;
        }
        else
        {
            is_primary = 0;
        }
    }
    if (argc >= 4)
    {
         // set number_of_handles, opt.
         CHAR *temp = argv[3];
         number = atoi(temp);
    }
    return a_mtkapi_bt_gatts_add_service(server_if, pt_uuid, is_primary, number);
}

static int btmw_rpc_test_gatts_add_included_service(int argc, char **argv)
{
    BTMW_RPC_TEST_Logi("[GATTS] %s()\n", __func__);
    INT32 service_handle = 0;
    INT32 included_handle = 0;
    INT32 server_if = 0;
    if (argc < 3)
    {
        BTMW_RPC_TEST_Logi("Usage :\n");
        BTMW_RPC_TEST_Logi("  GATTS add_included_service <server_if> <service_handle> <included_handle>\n");
        return -1;
    }
    server_if = atoi(argv[0]);
    service_handle = atoi(argv[1]);
    included_handle = atoi(argv[2]);
    return a_mtkapi_bt_gatts_add_included_service(server_if,service_handle,included_handle);
}

static int btmw_rpc_test_gatts_add_char (int argc, char **argv)
{
    BTMW_RPC_TEST_Logi("[GATTS] %s()\n", __func__);
    CHAR pt_uuid[130];
    INT32 service_handle = 0;
    INT32 properties = 6;
    INT32 permissions = 17;
    INT32 server_if = 0;
    if (argc < 3)
    {
        BTMW_RPC_TEST_Logi("Usage :\n");
        BTMW_RPC_TEST_Logi("  GATTS add_char <server_if> <service_handle> <uuid> [<properties> [<permissions>]]\n");
        return -1;
    }
    server_if = atoi(argv[0]);
    service_handle = atoi(argv[1]);
    strncpy(pt_uuid,argv[2], strlen(argv[2]));
    pt_uuid[strlen(argv[2])] = '\0';
    if (argc > 3)
    {
        // set properties, opt.
        properties = atoi(argv[3]);
    }
    if (argc > 4)
    {
        // set permissions, opt.
        permissions = atoi(argv[4]);
    }
    return a_mtkapi_bt_gatts_add_char(server_if, service_handle, pt_uuid, properties, permissions);
}

static int btmw_rpc_test_gatts_add_desc (int argc, char **argv)
{
    BTMW_RPC_TEST_Logi("[GATTS] %s()\n", __func__);
    CHAR pt_uuid[130];
    INT32 service_handle = 0;
    INT32 permissions = 0;
    INT32 server_if = 0;

    if (argc < 3)
    {
        BTMW_RPC_TEST_Logi("Usage :\n");
        BTMW_RPC_TEST_Logi("  GATTS add_desc <server_if> <service_handle> <uuid> [<permissions>]\n");
        return -1;
    }
    server_if = atoi(argv[0]);
    service_handle = atoi(argv[1]);
    strncpy(pt_uuid,argv[2], strlen(argv[2]));
    pt_uuid[strlen(argv[2])] = '\0';
    if (argc > 3)
    {
        CHAR *temp = argv[3];
        permissions = atoi(temp);
    }
    return a_mtkapi_bt_gatts_add_desc(server_if, service_handle, pt_uuid, permissions);
}

static int btmw_rpc_test_gatts_start_service (int argc, char **argv)
{
    BTMW_RPC_TEST_Logi("[GATTS] %s()\n", __func__);
    INT32 service_handle = 0;
    INT32 transport = 0;
    INT32 server_if = 0;
    if (argc < 2)
    {
        BTMW_RPC_TEST_Logi("Usage :\n");
        BTMW_RPC_TEST_Logi("  GATTS start_service <server_if> <service_handle> [<transport>]\n");
        return -1;
    }
    server_if = atoi(argv[0]);
    service_handle = atoi(argv[1]);
    if (argc > 2)
    {
        CHAR *temp = argv[2];
        transport = atoi(temp);
    }
    return a_mtkapi_bt_gatts_start_service(server_if,service_handle,transport);
}

static int btmw_rpc_test_gatts_stop_service (int argc, char **argv)
{
    BTMW_RPC_TEST_Logi("[GATTS] %s()\n", __func__);
    INT32 service_handle = 0;
    INT32 server_if = 0;
    if (argc < 2)
    {
        BTMW_RPC_TEST_Logi("Usage :\n");
        BTMW_RPC_TEST_Logi("  GATTS stop_service <server_if> <service_handle>\n");
        return -1;
    }
    server_if = atoi(argv[0]);
    service_handle = atoi(argv[1]);
    return a_mtkapi_bt_gatts_stop_service(server_if,service_handle);
}

static int btmw_rpc_test_gatts_delete_service (int argc, char **argv)
{
    BTMW_RPC_TEST_Logi("[GATTS] %s()\n", __func__);
    INT32 service_handle = 0;
    INT32 server_if = 0;
    if (argc < 2)
    {
        BTMW_RPC_TEST_Logi("Usage :\n");
        BTMW_RPC_TEST_Logi("  GATTS delete_service <server_if> <service_handle>\n");
        return -1;
    }
    server_if = atoi(argv[0]);
    service_handle = atoi(argv[1]);
    return a_mtkapi_bt_gatts_delete_service(server_if,service_handle);
}

static int btmw_rpc_test_gatts_send_indication (int argc, char **argv)
{
    BTMW_RPC_TEST_Logi("[GATTS] %s()\n", __func__);
    CHAR p_value[260];
    INT32 server_if = 0;
    INT32 attribute_handle = 0;
    INT32 conn_id = 0;
    INT32 confirm = 0;
    INT32 value_len = 0;
    if (argc < 4)
    {
        BTMW_RPC_TEST_Logi("Usage :\n");
        BTMW_RPC_TEST_Logi("  GATTS send_indi <server_if> <attribute_handle> <conn_id> [<confirm>] <value>\n");
        return -1;
    }
    server_if = atoi(argv[0]);
    attribute_handle = atoi(argv[1]);
    conn_id = atoi(argv[2]);
    if (argc == 5)
    {
        char *temp = argv[3];
        if (strcmp(temp,"1")||strcmp(temp,"true")||strcmp(temp,"TRUE"))
        {
            confirm = 1;
        }
        else
        {
            confirm = 0;
        }
        strncpy(p_value,argv[4], strlen(argv[4]));
        p_value[strlen(argv[4])] = '\0';
        value_len = strlen(p_value);
    }
    else
    {
        strncpy(p_value,argv[3], strlen(argv[3]));
        p_value[strlen(argv[3])] = '\0';
        value_len = strlen(p_value);
    }
    return a_mtkapi_bt_gatts_send_indication(server_if, attribute_handle, conn_id, confirm, p_value, value_len);
}

static int btmw_rpc_test_gatts_send_response (int argc, char **argv)
{
    BTMW_RPC_TEST_Logi("[GATTS] %s()\n", __func__);
    CHAR p_value[260];
    INT32 conn_id = 0;
    INT32 trans_id = 0;
    INT32 status = 0;
    INT32 handle = 0;
    INT32 value_len = 0;
    INT32 auth_req = 0;

    if (argc < 5)
    {
        BTMW_RPC_TEST_Logi("Usage :\n");
        BTMW_RPC_TEST_Logi("  GATTS send_response <conn_id> <trans_id> <status> <handle> [<auth_req>] <value>\n");
        return -1;
    }
    conn_id = atoi(argv[0]);
    trans_id = atoi(argv[1]);
    status = atoi(argv[2]);
    handle = atoi(argv[3]);
    if (argc == 6)
    {
        auth_req = atoi(argv[4]);
        strncpy(p_value,argv[5], strlen(argv[5]));
        p_value[strlen(argv[5])] = '\0';
        value_len = strlen(p_value);
    }
    else
    {
        strncpy(p_value,argv[4], strlen(argv[4]));
        p_value[strlen(argv[4])] = '\0';
        value_len = strlen(p_value);
    }
    return a_mtkapi_bt_gatts_send_response(conn_id, trans_id, status, handle, p_value, value_len, auth_req);
}

static BTMW_RPC_TEST_CLI btmw_rpc_test_gatts_cli_commands[] =
{
    {(const char *)"register_server",     btmw_rpc_test_gatts_register_server,     (const char *)" = register_server <uuid>"},
    {(const char *)"unregister_server",   btmw_rpc_test_gatts_unregister_server,   (const char *)" = unregister_server <server_if>"},
    {(const char *)"open",                btmw_rpc_test_gatts_open,                (const char *)" = open <server_if> <addr> [isDirect <true|false> [<transport>]]"},
    {(const char *)"close",               btmw_rpc_test_gatts_close,               (const char *)" = close <server_if> <addr> <conn_id>"},
    {(const char *)"add_service",         btmw_rpc_test_gatts_add_service,         (const char *)" = add_service <server_if> <uuid> [is_primary <true|false> [<number_of_handles>]]"},
    {(const char *)"add_included_service",btmw_rpc_test_gatts_add_included_service,(const char *)" = add_included_service <server_if> <service_handle> <included_handle>"},
    {(const char *)"add_char",            btmw_rpc_test_gatts_add_char,            (const char *)" = add_char <server_if> <service_handle> <uuid> [<properties> [<permissions>]]"},
    {(const char *)"add_desc",            btmw_rpc_test_gatts_add_desc,            (const char *)" = add_desc <server_if> <service_handle> <uuid> [<permissions>]"},
    {(const char *)"start_service",       btmw_rpc_test_gatts_start_service,       (const char *)" = start_service <server_if> <service_handle> [<transport>]"},
    {(const char *)"stop_service",        btmw_rpc_test_gatts_stop_service,        (const char *)" = stop_service <server_if> <service_handle>"},
    {(const char *)"delete_service",      btmw_rpc_test_gatts_delete_service,      (const char *)" = delete_service <server_if> <service_handle>"},
    {(const char *)"send_indi",           btmw_rpc_test_gatts_send_indication,     (const char *)" = send_indi <server_if> <attribute_handle> <conn_id> [<confirm>] <value>"},
    {(const char *)"send_response",       btmw_rpc_test_gatts_send_response,       (const char *)" = send_response <conn_id> <trans_id> <status> <handle> [<auth_req>] <value>"},
    {NULL, NULL, NULL},
};

// For handling incoming commands from CLI.
int btmw_rpc_test_gatts_cmd_handler(int argc, char **argv)
{
    BTMW_RPC_TEST_CLI *cmd, *match = NULL;
    int ret = 0;
    int count;
    count = 0;
    cmd = btmw_rpc_test_gatts_cli_commands;

    BTMW_RPC_TEST_Logd("[GATTS] argc: %d, argv[0]: %s\n", argc, argv[0]);

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
        BTMW_RPC_TEST_Logd("[GATTS] Unknown command '%s'\n", argv[0]);

        btmw_rpc_test_print_cmd_help(BTMW_RPC_TEST_CMD_KEY_GATTS, btmw_rpc_test_gatts_cli_commands);
        ret = -1;
    }
    else
    {
        match->handler(argc - 1, &argv[1]);
    }
    return ret;
}

int btmw_rpc_test_gatts_init()
{
    BTMW_RPC_TEST_Logi("%s", __func__);
    int ret = 0;
    BTMW_RPC_TEST_MOD gatts_mod = {0};

    // Register command to CLI
    gatts_mod.mod_id = BTMW_RPC_TEST_MOD_GATT_SERVER;
    strncpy(gatts_mod.cmd_key, BTMW_RPC_TEST_CMD_KEY_GATTS, sizeof(gatts_mod.cmd_key));
    gatts_mod.cmd_handler = btmw_rpc_test_gatts_cmd_handler;
    gatts_mod.cmd_tbl = btmw_rpc_test_gatts_cli_commands;
    ret = btmw_rpc_test_register_mod(&gatts_mod);
    BTMW_RPC_TEST_Logd("[GATTS] btmw_rpc_test_register_mod() returns: %d\n", ret);

    if (!g_cli_pts_mode)
    {
        MTKRPCAPI_BT_APP_GATTS_CB_FUNC_T func;
        char pv_tag[2] = {0};
        memset(&func, 0, sizeof(MTKRPCAPI_BT_APP_GATTS_CB_FUNC_T));
        func.bt_gatts_reg_server_cb = btmw_rpc_test_gatts_register_server_callback;
        func.bt_gatts_event_cb = btmw_rpc_test_gatts_event_callback;
        func.bt_gatts_add_srvc_cb = btmw_rpc_test_gatts_add_srvc_callback;
        func.bt_gatts_add_incl_cb = btmw_rpc_test_gatts_add_incl_callback;
        func.bt_gatts_add_char_cb = btmw_rpc_test_gatts_add_char_callback;
        func.bt_gatts_add_desc_cb = btmw_rpc_test_gatts_add_desc_callback;
        func.bt_gatts_op_srvc_cb = btmw_rpc_test_gatts_op_srvc_callback;
        func.bt_gatts_req_read_cb = btmw_rpc_test_gatts_req_read_callback;
        func.bt_gatts_req_write_cb = btmw_rpc_test_gatts_req_write_callback;
        func.bt_gatts_ind_sent_cb = btmw_rpc_test_gatts_ind_sent_callback;
        btmw_rpc_test_gatts_base_init(&func, (void *)pv_tag);
    }
    return ret;
}

int btmw_rpc_test_gatts_deinit()
{
    BTMW_RPC_TEST_Logi("%s", __func__);
    return 0;
}


