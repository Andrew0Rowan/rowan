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

#include "btmw_test_cli.h"
#include "btmw_test_debug.h"
#include "btmw_test_gatt_if.h"
#include "btmw_test_gatts_if.h"
#include "c_bt_mw_gatts.h"

static INT32 btmw_test_gatts_register_server(INT32 argc, CHAR **argv)
{
    CHAR pt_service_uuid[130];
    BTMW_TEST_Logi("[GATTS] %s()\n", __func__);

    if (argc < 1)
    {
        return -1;
    }
    strncpy(pt_service_uuid,argv[0], strlen(argv[0]));
    pt_service_uuid[strlen(argv[0])] = '\0';
    return c_btm_gatts_register_server(pt_service_uuid);
}

static INT32 btmw_test_gatts_unregister_server(INT32 argc, CHAR **argv)
{
    BTMW_TEST_Logi("[GATTS] %s()\n", __func__);
    if (argc < 1)
    {
        BTMW_TEST_Logi("Usage :\n");
        BTMW_TEST_Logi("  GATTS unregister_server <server_if>\n");
        return -1;
    }
    INT32 server_if = 0;
    server_if = atoi(argv[0]);
    return c_btm_gatts_unregister_server(server_if);
}

static INT32 btmw_test_gatts_open(INT32 argc, CHAR **argv)
{
    BTMW_TEST_Logi("[GATTS] %s()\n", __func__);

    CHAR ps_addr[MAX_BDADDR_LEN];
    UINT8 is_direct = 0;
    INT32 transport = 0;
    INT32 server_if = 0;
    if (argc < 2)
    {
        BTMW_TEST_Logi("Usage :\n");
        BTMW_TEST_Logi("  GATTS open <server_if> <addr> [isDirect <true|false> [<transport>]]\n");
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
    return c_btm_gatts_connect(server_if, ps_addr, is_direct, transport);
}

static INT32 btmw_test_gatts_close(INT32 argc, CHAR **argv)
{
    BTMW_TEST_Logi("[GATTS] %s()\n", __func__);
    CHAR ps_addr[MAX_BDADDR_LEN];
    INT32 server_if = 0;
    INT32 conn_id = 0;

    if (argc < 3)
    {
        BTMW_TEST_Logi("Usage :\n");
        BTMW_TEST_Logi("  GATTS close <server_if> <addr> <conn_id>\n");
        return -1;
    }
    server_if = atoi(argv[0]);
    strncpy(ps_addr,argv[1], MAX_BDADDR_LEN);
    ps_addr[MAX_BDADDR_LEN - 1] = '\0';
    conn_id = atoi(argv[2]);
    return c_btm_gatts_disconnect(server_if, ps_addr, conn_id);
}

static INT32 btmw_test_gatts_add_service (INT32 argc, CHAR **argv)
{
    BTMW_TEST_Logi("[GATTS] %s()\n", __func__);
    CHAR pt_uuid[130];
    INT32 number = 2;
    UINT8 is_primary = 1;
    INT32 server_if = 0;

    if (argc < 2)
    {
        BTMW_TEST_Logi("Usage :\n");
        BTMW_TEST_Logi("  GATTS add_service <server_if> <uuid> [is_primary <true|false> [<number_of_handles>]]\n");
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
    return c_btm_gatts_add_service(server_if, pt_uuid, is_primary, number);
}

static INT32 btmw_test_gatts_add_included_service(INT32 argc, CHAR **argv)
{
    BTMW_TEST_Logi("[GATTS] %s()\n", __func__);
    INT32 service_handle = 0;
    INT32 included_handle = 0;
    INT32 server_if = 0;
    if (argc < 3)
    {
        BTMW_TEST_Logi("Usage :\n");
        BTMW_TEST_Logi("  GATTS add_included_service <server_if> <service_handle> <included_handle>\n");
        return -1;
    }
    server_if = atoi(argv[0]);
    service_handle = atoi(argv[1]);
    included_handle = atoi(argv[2]);
    return c_btm_gatts_add_included_service(server_if,service_handle,included_handle);
}

static INT32 btmw_test_gatts_add_char (INT32 argc, CHAR **argv)
{
    BTMW_TEST_Logi("[GATTS] %s()\n", __func__);
    CHAR pt_uuid[130];
    INT32 service_handle = 0;
    INT32 properties = 6;
    INT32 permissions = 17;
    INT32 server_if = 0;
    if (argc < 3)
    {
        BTMW_TEST_Logi("Usage :\n");
        BTMW_TEST_Logi("  GATTS add_char <server_if> <service_handle> <uuid> [<properties> [<permissions>]]\n");
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
    return c_btm_gatts_add_char(server_if, service_handle, pt_uuid, properties, permissions);
}

static INT32 btmw_test_gatts_add_desc (INT32 argc, CHAR **argv)
{
    BTMW_TEST_Logi("[GATTS] %s()\n", __func__);
    CHAR pt_uuid[130];
    INT32 service_handle = 0;
    INT32 permissions = 0;
    INT32 server_if = 0;

    if (argc < 3)
    {
        BTMW_TEST_Logi("Usage :\n");
        BTMW_TEST_Logi("  GATTS add_desc <server_if> <service_handle> <uuid> [<permissions>]\n");
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
    return c_btm_gatts_add_desc(server_if, service_handle, pt_uuid, permissions);
}

static INT32 btmw_test_gatts_start_service (INT32 argc, CHAR **argv)
{
    BTMW_TEST_Logi("[GATTS] %s()\n", __func__);
    INT32 service_handle = 0;
    INT32 transport = 0;
    INT32 server_if = 0;
    if (argc < 2)
    {
        BTMW_TEST_Logi("Usage :\n");
        BTMW_TEST_Logi("  GATTS start_service <server_if> <service_handle> [<transport>]\n");
        return -1;
    }
    server_if = atoi(argv[0]);
    service_handle = atoi(argv[1]);
    if (argc > 2)
    {
        CHAR *temp = argv[2];
        transport = atoi(temp);
    }
    return c_btm_gatts_start_service(server_if, service_handle, transport);
}

static INT32 btmw_test_gatts_stop_service (INT32 argc, CHAR **argv)
{
    BTMW_TEST_Logi("[GATTS] %s()\n", __func__);
    INT32 service_handle = 0;
    INT32 server_if = 0;
    if (argc < 2)
    {
        BTMW_TEST_Logi("Usage :\n");
        BTMW_TEST_Logi("  GATTS stop_service <server_if> <service_handle>\n");
        return -1;
    }
    server_if = atoi(argv[0]);
    service_handle = atoi(argv[1]);
    return c_btm_gatts_stop_service(server_if, service_handle);
}

static INT32 btmw_test_gatts_delete_service (INT32 argc, CHAR **argv)
{
    BTMW_TEST_Logi("[GATTS] %s()\n", __func__);
    INT32 service_handle = 0;
    INT32 server_if = 0;
    if (argc < 2)
    {
        BTMW_TEST_Logi("Usage :\n");
        BTMW_TEST_Logi("  GATTS delete_service <server_if> <service_handle>\n");
        return -1;
    }
    server_if = atoi(argv[0]);
    service_handle = atoi(argv[1]);
    return c_btm_gatts_delete_service(server_if, service_handle);
}

static INT32 btmw_test_gatts_send_indication (INT32 argc, CHAR **argv)
{
    BTMW_TEST_Logi("[GATTS] %s()\n", __func__);
    CHAR p_value[260];
    INT32 server_if = 0;
    INT32 attribute_handle = 0;
    INT32 conn_id = 0;
    INT32 confirm = 0;
    INT32 value_len = 0;
    if (argc < 4)
    {
        BTMW_TEST_Logi("Usage :\n");
        BTMW_TEST_Logi("  GATTS send_indi <server_if> <attribute_handle> <conn_id> [<confirm>] <value>\n");
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
    return c_btm_gatts_send_indication(server_if, attribute_handle, conn_id, confirm, p_value, value_len);
}

static INT32 btmw_test_gatts_send_response (INT32 argc, CHAR **argv)
{
    BTMW_TEST_Logi("[GATTS] %s()\n", __func__);
    CHAR p_value[260];
    INT32 conn_id = 0;
    INT32 trans_id = 0;
    INT32 status = 0;
    INT32 handle = 0;
    INT32 value_len = 0;
    INT32 auth_req = 0;

    if (argc < 5)
    {
        BTMW_TEST_Logi("Usage :\n");
        BTMW_TEST_Logi("  GATTS send_response <conn_id> <trans_id> <status> <handle> [<auth_req>] <value>\n");
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
    return c_btm_gatts_send_response(conn_id, trans_id, status, handle, p_value,
                                     value_len, auth_req);
}

static BTMW_TEST_CLI btmw_test_gatts_cli_commands[] =
{
    {"register_server",     btmw_test_gatts_register_server,     " = register_server <uuid>"},
    {"unregister_server",   btmw_test_gatts_unregister_server,   " = unregister_server <server_if>"},
    {"open",                btmw_test_gatts_open,                " = open <server_if> <addr> [isDirect <true|false> [<transport>]]"},
    {"close",               btmw_test_gatts_close,               " = close <server_if> <addr> <conn_id>"},
    {"add_service",         btmw_test_gatts_add_service,         " = add_service <server_if> <uuid> [is_primary <true|false> [<number_of_handles>]]"},
    {"add_included_service",btmw_test_gatts_add_included_service," = add_included_service <server_if> <service_handle> <included_handle>"},
    {"add_char",            btmw_test_gatts_add_char,            " = add_char <server_if> <service_handle> <uuid> [<properties> [<permissions>]]"},
    {"add_desc",            btmw_test_gatts_add_desc,            " = add_desc <server_if> <service_handle> <uuid> [<permissions>]"},
    {"start_service",       btmw_test_gatts_start_service,       " = start_service <server_if> <service_handle> [<transport>]"},
    {"stop_service",        btmw_test_gatts_stop_service,        " = stop_service <server_if> <service_handle>"},
    {"delete_service",      btmw_test_gatts_delete_service,      " = delete_service <server_if> <service_handle>"},
    {"send_indi",           btmw_test_gatts_send_indication,     " = send_indi <server_if> <attribute_handle> <conn_id> [<confirm>] <value>"},
    {"send_response",       btmw_test_gatts_send_response,       " = send_response <conn_id> <trans_id> <status> <handle> [<auth_req>] <value>"},
    {NULL, NULL, NULL},
};


#if 0
static void btmw_test_gatts_register_server_callback(int status, int server_if, bt_uuid_t *app_uuid)
{
    BTMW_TEST_Logi("[GATTS] %s()\n", __func__);
    BTMW_TEST_Logi("[GATTS] Register server callback :'%d' server_if = %d\n", status ,server_if);
    if (status == 0)
    {
        btmw_test_server_if = server_if;
        //LE scan units(0.625ms)
        /*int scan_window = SCAN_MODE_BALANCED_WINDOW_MS*1000/625;
        int scan_interval = SCAN_MODE_BALANCED_INTERVAL_MS*1000/625;
        btmw_test_gattc_interface->set_scan_parameters(scan_interval,scan_window);*/
    }
}

static void btmw_test_gatts_connection_callback(int conn_id, int server_if, int connected, bt_bdaddr_t *bda)
{
    BTMW_TEST_Logi("[GATTS] %s()\n", __func__);
    BTMW_TEST_Logi("%02X:%02X:%02X:%02X:%02X:%02X\n connected = %d, conn_id = %d\n",
        bda->address[0], bda->address[1], bda->address[2],
        bda->address[3], bda->address[4], bda->address[5],connected,conn_id);
}

static void btmw_test_gatts_service_added_callback(int status, int server_if, btgatt_srvc_id_t *srvc_id, int srvc_handle)
{
    BTMW_TEST_Logi("[GATTS] %s()\n", __func__);
    bt_uuid_t uuid = srvc_id->id.uuid;
    char uuid_s[37];
    btmw_test_gatt_print_uuid(&uuid,uuid_s);

    BTMW_TEST_Logi("add service uuid:%s handle = %d, status = %d\n",uuid_s,srvc_handle,status);
}

static void btmw_test_gatts_included_service_added_callback(int status, int server_if, int srvc_handle, int incl_srvc_handle)
{
    BTMW_TEST_Logi("[GATTS] %s()\n", __func__);
    BTMW_TEST_Logi("add included service:%s in service: %d, status = %d\n",incl_srvc_handle,status);
}

static void btmw_test_gatts_characteristic_added_callback(int status, int server_if, bt_uuid_t *uuid, int srvc_handle, int char_handle)
{
    BTMW_TEST_Logi("[GATTS] %s()\n", __func__);
    char uuid_s[37];
    btmw_test_gatt_print_uuid(uuid,uuid_s);

    BTMW_TEST_Logi("add char uuid:%s in service:%d handle = %d, status = %d\n",uuid_s,srvc_handle,char_handle,status);
}

static void btmw_test_gatts_descriptor_added_callback(int status, int server_if, bt_uuid_t *uuid, int srvc_handle, int descr_handle)
{
    BTMW_TEST_Logi("[GATTS] %s()\n", __func__);
    char uuid_s[37];
    btmw_test_gatt_print_uuid(uuid,uuid_s);

    BTMW_TEST_Logi("add descriptor uuid:%s in service:%d handle = %d, status = %d\n",uuid_s,srvc_handle,descr_handle,status);
}

static void btmw_test_gatts_service_started_callback(int status, int server_if, int srvc_handle)
{
    BTMW_TEST_Logi("[GATTS] %s()\n", __func__);
    BTMW_TEST_Logi("service started handle = %d, status = %d\n",srvc_handle,status);
}

static void btmw_test_gatts_service_stopped_callback(int status, int server_if, int srvc_handle)
{
    BTMW_TEST_Logi("[GATTS] %s()\n", __func__);
    BTMW_TEST_Logi("service stopped handle = %d, status = %d\n",srvc_handle,status);
}

static void btmw_test_gatts_service_deleted_callback(int status, int server_if, int srvc_handle)
{
    BTMW_TEST_Logi("[GATTS] %s()\n", __func__);
    BTMW_TEST_Logi("service stopped handle = %d, status = %d\n",srvc_handle,status);
}

static void btmw_test_gatts_request_read_callback(int conn_id, int trans_id, bt_bdaddr_t *bda, int attr_handle, int offset, bool is_long)
{
    BTMW_TEST_Logi("[GATTS] %s()\n", __func__);
    BTMW_TEST_Logi("%02X:%02X:%02X:%02X:%02X:%02X\n request read, trans_id = %d , attr_handle = %d\n",
        bda->address[0], bda->address[1], bda->address[2],
        bda->address[3], bda->address[4], bda->address[5],trans_id,attr_handle);
    BTMW_TEST_Logd("Call btmw_test_gatts_send_response() to send data.\n");
}

static void btmw_test_gatts_request_write_callback(int conn_id, int trans_id, bt_bdaddr_t *bda, int attr_handle, int offset, int length,
                                       bool need_rsp, bool is_prep, uint8_t* value)
{
    BTMW_TEST_Logi("[GATTS] %s()\n", __func__);
    BTMW_TEST_Logi("%02X:%02X:%02X:%02X:%02X:%02X\n request write, need_respond = %d , trans_id = %d , attr_handle = %d,\n value:%s",
        bda->address[0], bda->address[1], bda->address[2],
        bda->address[3], bda->address[4], bda->address[5],need_rsp,trans_id,attr_handle,value);
    if (need_rsp)
    {
        btgatt_response_t response;
        response.handle = attr_handle;
        response.attr_value.len = 0;
        btmw_test_gatts_interface->send_response(conn_id,trans_id,BT_STATUS_SUCCESS,&response);
    }
}

static void btmw_test_gatts_request_exec_write_callback(int conn_id, int trans_id, bt_bdaddr_t *bda, int exec_write)
{
    BTMW_TEST_Logi("[GATTS] %s()\n", __func__);
    BTMW_TEST_Logi("%02X:%02X:%02X:%02X:%02X:%02X\n request exec write, trans_id = %d , exec_write = %d\n",
        bda->address[0], bda->address[1], bda->address[2],
        bda->address[3], bda->address[4], bda->address[5],conn_id,trans_id,exec_write);
}

static void btmw_test_gatts_response_confirmation_callback(int status, int handle)
{
    BTMW_TEST_Logi("[GATTS] %s()\n", __func__);
    BTMW_TEST_Logi("response confirmation handle = %d, status = %d\n",handle,status);
}

static void btmw_test_gatts_indication_sent_callback(int conn_id, int status)
{
    BTMW_TEST_Logi("[GATTS] %s()\n", __func__);
    BTMW_TEST_Logi("indication sent conn_id = %d, status = %d\n",conn_id,status);
}

static void btmw_test_gatts_congestion_callback(int conn_id, bool congested)
{
    BTMW_TEST_Logi("[GATTS] %s()\n", __func__);
    BTMW_TEST_Logi("conn_id = %d, congestion = %d\n",conn_id,congested);
}

static void btmw_test_gatts_mtu_changed_callback(int conn_id, int mtu)
{
    BTMW_TEST_Logi("[GATTS] %s()\n", __func__);
    BTMW_TEST_Logi("conn_id = %d, mtu = %d\n",conn_id,mtu);
}

const btgatt_server_callbacks_t btmw_test_gatts_callbacks =
{
    btmw_test_gatts_register_server_callback,
    btmw_test_gatts_connection_callback,
    btmw_test_gatts_service_added_callback,
    btmw_test_gatts_included_service_added_callback,
    btmw_test_gatts_characteristic_added_callback,
    btmw_test_gatts_descriptor_added_callback,
    btmw_test_gatts_service_started_callback,
    btmw_test_gatts_service_stopped_callback,
    btmw_test_gatts_service_deleted_callback,
    btmw_test_gatts_request_read_callback,
    btmw_test_gatts_request_write_callback,
    btmw_test_gatts_request_exec_write_callback,
    btmw_test_gatts_response_confirmation_callback,
    btmw_test_gatts_indication_sent_callback,
    btmw_test_gatts_congestion_callback,
    btmw_test_gatts_mtu_changed_callback,
};

btgatt_server_interface_t *btmw_test_gatts_interface;
#endif
#if defined(MTK_LINUX_GAP_PTS_TEST) && (MTK_LINUX_GAP_PTS_TEST == TRUE)
static VOID btmw_test_gatt_req_read_cbk(BT_GATTS_REQ_READ_RST_T *bt_gatts_read)
{
    //FUNC_ENTRY;
    BTMW_TEST_Logd("GAP PTS [GATTS] btmw_test_gatt_req_read_cbk, conn_id is %d, trans_id is %d",
                   bt_gatts_read->conn_id, bt_gatts_read->trans_id);
    INT32 length = 5;
    CHAR p_value[5] = "cccc";
    c_btm_gatts_send_response(bt_gatts_read->conn_id,
                              bt_gatts_read->trans_id,
                              0,
                              bt_gatts_read->attr_handle,
                              p_value,
                              length,
                              0);

    return;
}
#endif
// For handling incoming commands from CLI.
int btmw_test_gatts_cmd_handler(int argc, char **argv)
{
    BTMW_TEST_CLI *cmd, *match = NULL;
    int ret = 0;
    int count;
    count = 0;
    cmd = btmw_test_gatts_cli_commands;
    BTMW_TEST_Logd("[GATTS] argc: %d, argv[0]: %s\n", argc, argv[0]);

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
        BTMW_TEST_Logd("[GATTS] Unknown command '%s'\n", argv[0]);

        btmw_test_print_cmd_help(BTMW_TEST_CMD_KEY_GATTS, btmw_test_gatts_cli_commands);
        ret = -1;
    }
    else
    {
        match->handler(argc - 1, &argv[1]);
    }
    return ret;
}

int btmw_test_gatts_init()
{
    BTMW_TEST_Logi("%s", __func__);
    int ret = 0;
    BTMW_TEST_MOD hid_mod = {0};

    // Register command to CLI
    hid_mod.mod_id = BTMW_TEST_MOD_GATT_SERVER;
    strncpy(hid_mod.cmd_key, BTMW_TEST_CMD_KEY_GATTS, sizeof(hid_mod.cmd_key));
    hid_mod.cmd_handler = btmw_test_gatts_cmd_handler;
    hid_mod.cmd_tbl = btmw_test_gatts_cli_commands;

    ret = btmw_test_register_mod(&hid_mod);
    BTMW_TEST_Logd("[GATTS] btmw_test_register_mod() returns: %d\n", ret);
#if defined(MTK_LINUX_GAP_PTS_TEST) && (MTK_LINUX_GAP_PTS_TEST == TRUE)
    BT_APP_GATTS_CB_FUNC_T gatts_func;
    memset(&gatts_func, 0, sizeof(BT_APP_GATTS_CB_FUNC_T));
    gatts_func.bt_gatts_req_read_cb = btmw_test_gatt_req_read_cbk;
    ret = c_btm_gatts_register_callback(&gatts_func);
    if (BT_SUCCESS != ret)
    {
        BTMW_TEST_Logd("GAP PTS [GATT] Register BT_APP_GATTS_CB_FUNC failed.\n");
        return ret;
    }
#endif
#if 0
    if (0 == c_btm_bt_gatts_register_server(BTMW_TEST_GATTS_SERVER_UUID))
    {
        BTMW_TEST_Logi("[GATTS] Register server uuid:'%s'\n", BTMW_TEST_GATTS_SERVER_UUID);
    }
#endif
    return ret;
}

int btmw_test_gatts_deinit()
{
    BTMW_TEST_Logi("%s", __func__);
    return 0;
}


