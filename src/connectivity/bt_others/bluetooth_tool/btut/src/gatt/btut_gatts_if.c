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
#include <hardware/bluetooth.h>
#include <hardware/bt_gatt.h>
#include <hardware/bt_gatt_server.h>

#include "btut_cli.h"
#include "btut_debug.h"
#include "btut_gatt_if.h"
#include "btut_gatts_if.h"

static int btut_server_if = -1;
const btgatt_server_interface_t *btut_gatts_interface;

static int btut_gatts_register_server(int argc, char **argv)
{
    char *ptr;
    bt_uuid_t uuid;

    BTUT_Logi("[GATTS] %s()\n", __func__);

    if (argc < 1)
    {
        return -1;
    }

    if (btut_server_if != -1)
    {
        btut_gatts_interface->unregister_server(btut_server_if);
    }

    ptr = argv[0];
    btut_gatt_uuid_stoh(ptr, &uuid);
    btut_gatts_interface->register_server(&uuid);

    return 0;
}

static int btut_gatts_unregister_server(int argc, char **argv)
{
    BTUT_Logi("[GATTS] %s()\n", __func__);
#if 0
    if (btut_server_if == -1)
    {
        BTUT_Logi("[GATTS] Unregister client : no client app need to unregister.\n");
        return 0;
    }
#endif
    if (argc < 1)
    {
        BTUT_Logi("Usage :\n");
        BTUT_Logi("  GATTS unregister_server server_if\n");
        return -1;
    }
    int server_if = 0;
    server_if = atoi(argv[0]);
    btut_gatts_interface->unregister_server(server_if);
    btut_server_if = -1;
    return 0;
}

static int btut_gatts_open(int argc, char **argv)
{
    BTUT_Logi("[GATTS] %s()\n", __func__);
    /*if (btut_server_if == -1)
    {
        BTUT_Logi("[GATTS] no server registered now.\n");
        return 0;
    }*/
    char *ptr;
    bool is_direct = true;
    int transport = 0;
    int server_if = 0;
    bt_bdaddr_t bdaddr;

    if (argc < 2)
    {
        BTUT_Logi("Usage :\n");
        BTUT_Logi("  GATTS open server_if BD_ADDR [isDirect:true|false] [transport]\n");
        return -1;
    }
    server_if = atoi(argv[0]);
    ptr = argv[1];
    if (argc >= 3) { // set isDirect, opt.
        char *temp = argv[2];
        if (strcmp(temp,"1")||strcmp(temp,"true")||strcmp(temp,"TRUE"))
        {
            is_direct = true;
        }
        else
        {
            is_direct = false;
        }
    }

    if (argc >= 4)
    {
        // set transport, opt.
        char *temp = argv[3];
        transport = atoi(temp);
    }

    btut_gatt_btaddr_stoh(ptr, &bdaddr);
    BTUT_Logi("GATTS connect to %02X:%02X:%02X:%02X:%02X:%02X ,\n",
        bdaddr.address[0], bdaddr.address[1], bdaddr.address[2],
        bdaddr.address[3], bdaddr.address[4], bdaddr.address[5]);

    btut_gatts_interface->connect(server_if,&bdaddr,is_direct,transport);

    return 0;
}

static int btut_gatts_close(int argc, char **argv)
{
    BTUT_Logi("[GATTS] %s()\n", __func__);
    /*if (btut_server_if == -1)
    {
        BTUT_Logi("[GATTS] no server registered now.\n");
        return 0;
    }*/
    char *ptr;
    int conn_id = 0;
    bt_bdaddr_t bdaddr;
    int server_if = 0;
    if (argc < 3)
    {
        BTUT_Logi("Usage :\n");
        BTUT_Logi("  GATTS close server_if BD_ADDR conn_id \n");
        return -1;
    }
    server_if = atoi(argv[0]);
    ptr = argv[1];
    conn_id = atoi(argv[2]);

    btut_gatt_btaddr_stoh(ptr, &bdaddr);
    BTUT_Logi("GATTS disconnect to %02X:%02X:%02X:%02X:%02X:%02X ,\n",
        bdaddr.address[0], bdaddr.address[1], bdaddr.address[2],
        bdaddr.address[3], bdaddr.address[4], bdaddr.address[5]);

    btut_gatts_interface->disconnect(server_if,&bdaddr,conn_id);

    return 0;
}

static int btut_gatts_add_service (int argc, char **argv)
{
    BTUT_Logi("[GATTS] %s()\n", __func__);
    /*if (btut_server_if == -1)
    {
        BTUT_Logi("[GATTS] no server registered now.\n");
        return 0;
    }*/
    char *ptr;
    int number = 2; // one service,one char and no desc
    int server_if = 0;
    btgatt_srvc_id_t srvc_id;

    if (argc < 2)
    {
        BTUT_Logi("Usage :\n");
        BTUT_Logi("  GATTS add_service server_if uuid [is_primary:true|false] [number_of_handles] \n");
        return -1;
    }
    server_if = atoi(argv[0]);
    ptr = argv[1];
    btut_gatt_uuid_stoh(ptr, &(srvc_id.id.uuid));
    srvc_id.is_primary = true;

    if (argc >= 3)
    {
        // set is_primary, opt.
        char *temp = argv[2];
        if (strcmp(temp,"1")||strcmp(temp,"true")||strcmp(temp,"TRUE"))
        {
            srvc_id.is_primary = true;
        }
        else
        {
            srvc_id.is_primary = false;
        }
    }

    if (argc >= 4)
    {
         // set number_of_handles, opt.
         char *temp = argv[3];
         number = atoi(temp);
    }

    btut_gatts_interface->add_service(server_if,&srvc_id,number);
    return 0;
}

static int btut_gatts_add_included_service(int argc, char **argv)
{
    BTUT_Logi("[GATTS] %s()\n", __func__);
    /*if (btut_server_if == -1)
    {
        BTUT_Logi("[GATTS] no server registered now.\n");
        return 0;
    }*/
    int service_handle = 0;
    int included_handle = 0;
    int server_if = 0;
    if (argc < 3)
    {
        BTUT_Logi("Usage :\n");
        BTUT_Logi("  GATTS add_included_service server_if service_handle included_handle \n");
        return -1;
    }
    server_if = atoi(argv[0]);
    service_handle = atoi(argv[1]);
    included_handle = atoi(argv[2]);

    btut_gatts_interface->add_included_service(server_if,service_handle,included_handle);
    return 0;
}

static int btut_gatts_add_char (int argc, char **argv)
{
    BTUT_Logi("[GATTS] %s()\n", __func__);
    /*if (btut_server_if == -1)
    {
        BTUT_Logi("[GATTS] no server registered now.\n");
        return 0;
    }*/
    char *ptr;
    bt_uuid_t uuid;
    int service_handle = 0;
    int properties = 0;
    int permissions = 0;
    int server_if = 0;
    char *temp;
    if (argc < 3)
    {
        BTUT_Logi("Usage :\n");
        BTUT_Logi("  GATTS add_char server_if service_handle uuid [properties] [permissions] \n");
        return -1;
    }
    server_if = atoi(argv[0]);
    service_handle = atoi(argv[1]);
    ptr = argv[2];
    btut_gatt_uuid_stoh(ptr, &uuid);

    if (argc > 3)
    {
        // set properties, opt.
        temp = argv[3];
        properties = atoi(temp);
    }

    if (argc > 4)
    {
        // set permissions, opt.
        temp = argv[4];
        permissions = atoi(temp);
    }

    btut_gatts_interface->add_characteristic(server_if,service_handle,&uuid,properties,permissions);
    return 0;
}

static int btut_gatts_add_desc (int argc, char **argv)
{
    BTUT_Logi("[GATTS] %s()\n", __func__);
    /*if (btut_server_if == -1)
    {
        BTUT_Logi("[GATTS] no server registered now.\n");
        return 0;
    }*/
    char *ptr;
    bt_uuid_t uuid;
    int service_handle = 0;
    int permissions = 0;
    int server_if = 0;
    if (argc < 3)
    {
        BTUT_Logi("Usage :\n");
        BTUT_Logi("  GATTS add_desc server_if service_handle uuid [permissions] \n");
        return -1;
    }
    server_if = atoi(argv[0]);
    service_handle = atoi(argv[1]);
    ptr = argv[2];
    btut_gatt_uuid_stoh(ptr, &uuid);

    if (argc > 3)
    {
        char *temp = argv[3];
        permissions = atoi(temp);
    }

    btut_gatts_interface->add_descriptor(server_if,service_handle,&uuid,permissions);
    return 0;
}

static int btut_gatts_start_service (int argc, char **argv)
{
    BTUT_Logi("[GATTS] %s()\n", __func__);
    /* (btut_server_if == -1)
    {
        BTUT_Logi("[GATTS] no server registered now.\n");
        return 0;
    }*/
    int service_handle = 0;
    int transport = 0;
    int server_if = 0;
    if (argc < 2)
    {
        BTUT_Logi("Usage :\n");
        BTUT_Logi("  GATTS start_service server_if service_handle [transport] \n");
        return -1;
    }
    server_if = atoi(argv[0]);
    service_handle = atoi(argv[1]);

    if (argc > 2)
    {
        char *temp = argv[2];
        transport = atoi(temp);
    }

    btut_gatts_interface->start_service(server_if,service_handle,transport);
    return 0;
}

static int btut_gatts_stop_service (int argc, char **argv)
{
    BTUT_Logi("[GATTS] %s()\n", __func__);
    /*if (btut_server_if == -1)
    {
        BTUT_Logi("[GATTS] no server registered now.\n");
        return 0;
    }*/
    int service_handle = 0;
    int server_if = 0;
    if (argc < 2)
    {
        BTUT_Logi("Usage :\n");
        BTUT_Logi("  GATTS stop_service server_if service_handle \n");
        return -1;
    }
    server_if = atoi(argv[0]);
    service_handle = atoi(argv[1]);
    btut_gatts_interface->stop_service(server_if,service_handle);
    return 0;
}

static int btut_gatts_delete_service (int argc, char **argv)
{
    BTUT_Logi("[GATTS] %s()\n", __func__);
    /*if (btut_server_if == -1)
    {
        BTUT_Logi("[GATTS] no server registered now.\n");
        return 0;
    }*/
    int service_handle= 0;
    int server_if = 0;
    if (argc < 2)
    {
        BTUT_Logi("Usage :\n");
        BTUT_Logi("  GATTS delete_service server_if service_handle \n");
        return -1;
    }

    server_if = atoi(argv[0]);
    service_handle = atoi(argv[1]);
    btut_gatts_interface->delete_service(server_if,service_handle);
    return 0;
}

static int btut_gatts_send_indication (int argc, char **argv)
{
    BTUT_Logi("[GATTS] %s()\n", __func__);
    /*if (btut_server_if == -1)
    {
        BTUT_Logi("[GATTS] no server registered now.\n");
        return 0;
    }*/
    int attribute_handle = 0;
    int conn_id = 0;
    bool confirm = false;
    char *value;
    int server_if = 0;
    if (argc < 4)
    {
        BTUT_Logi("Usage :\n");
        BTUT_Logi("  GATTS send_indi server_if attribute_handle conn_id [confirm] value \n");
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
            confirm = true;
        }
        else
        {
            confirm = false;
        }
        value= argv[4];
    }
    else
    {
        value= argv[3];
    }

    btut_gatts_interface->send_indication(server_if,attribute_handle,conn_id,strlen(value),confirm,value);
    return 0;
}

static int btut_gatts_send_response (int argc, char **argv)
{
    BTUT_Logi("[GATTS] %s()\n", __func__);
    /*if (btut_server_if == -1)
    {
        BTUT_Logi("[GATTS] no server registered now.\n");
        return 0;
    }*/
    int conn_id = 0;
    int trans_id = 0;
    int status = 0;
    int handle = 0;
    int auth_req = 0;
    char *value;

    if (argc < 5)
    {
        BTUT_Logi("Usage :\n");
        BTUT_Logi("  GATTS send_response conn_id trans_id status handle [auth_req] value \n");
        return -1;
    }

    char *temp = argv[0];
    conn_id = atoi(temp);
    temp = argv[1];
    trans_id = atoi(temp);
    temp = argv[2];
    status = atoi(temp);
    temp = argv[3];
    handle = atoi(temp);

    if (argc == 6)
    {
        temp = argv[4];
        auth_req = atoi(temp);
        value= argv[5];
    }
    else
    {
        value= argv[4];
    }
    btgatt_response_t response;
    strcpy((char *)response.attr_value.value,value);
    response.attr_value.handle = handle;
    response.attr_value.offset = 0;
    response.attr_value.len = strlen(value);
    response.attr_value.auth_req = auth_req;

    btut_gatts_interface->send_response(conn_id,trans_id,status,&response);
    return 0;
}

static BTUT_CLI bt_gatts_cli_commands[] =
{
    {"register_server",     btut_gatts_register_server,     " = activate"},
    {"unregister_server",   btut_gatts_unregister_server,   " = deactivate"},
    {"open",                btut_gatts_open,                " = open server_if BD_ADDR [isDirect:true|false] [transport]"},
    {"close",               btut_gatts_close,               " = close server_if BD_ADDR conn_id"},
    {"add_service",         btut_gatts_add_service,         " = add_service server_if uuid [is_primary:true|false] [number_of_handles]"},
    {"add_included_service",btut_gatts_add_included_service," = add_included_service server_if service_handle included_handle"},
    {"add_char",            btut_gatts_add_char,            " = add_char server_if service_handle uuid [properties] [permissions]"},
    {"add_desc",            btut_gatts_add_desc,            " = add_desc server_if service_handle uuid [permissions]"},
    {"start_service",       btut_gatts_start_service,       " = start_service server_if service_handle [transport]"},
    {"stop_service",        btut_gatts_stop_service,        " = stop_service server_if service_handle"},
    {"delete_service",      btut_gatts_delete_service,      " = delete_service server_if service_handle"},
    {"send_indi",           btut_gatts_send_indication,     " = send_indi server_if attribute_handle conn_id [confirm] value"},
    {"send_response",       btut_gatts_send_response,       " = send_response conn_id trans_id status handle [auth_req] value"},
    {NULL, NULL, NULL},
};

static void btut_gatts_register_server_callback(int status, int server_if, bt_uuid_t *app_uuid)
{
    BTUT_Logi("[GATTS] %s()\n", __func__);
    BTUT_Logi("[GATTS] Register server callback :'%d' server_if = %d\n", status ,server_if);
    if (status == 0)
    {
        btut_server_if = server_if;
        //LE scan units(0.625ms)
        /*int scan_window = SCAN_MODE_BALANCED_WINDOW_MS*1000/625;
        int scan_interval = SCAN_MODE_BALANCED_INTERVAL_MS*1000/625;
        btut_gattc_interface->set_scan_parameters(scan_interval,scan_window);*/
    }
}

static void btut_gatts_connection_callback(int conn_id, int server_if, int connected, bt_bdaddr_t *bda)
{
    BTUT_Logi("[GATTS] %s()\n", __func__);
    BTUT_Logi("%02X:%02X:%02X:%02X:%02X:%02X\n connected = %d, conn_id = %d\n",
        bda->address[0], bda->address[1], bda->address[2],
        bda->address[3], bda->address[4], bda->address[5],connected,conn_id);
}

static void btut_gatts_service_added_callback(int status, int server_if, btgatt_srvc_id_t *srvc_id, int srvc_handle)
{
    BTUT_Logi("[GATTS] %s()\n", __func__);
    bt_uuid_t uuid = srvc_id->id.uuid;
    char uuid_s[37];
    btut_gatt_print_uuid(&uuid,uuid_s);

    BTUT_Logi("add service uuid:%s handle = %d, status = %d\n",uuid_s,srvc_handle,status);
}

static void btut_gatts_included_service_added_callback(int status, int server_if, int srvc_handle, int incl_srvc_handle)
{
    BTUT_Logi("[GATTS] %s()\n", __func__);
    BTUT_Logi("add included service:%d in service: %d, status = %d\n",incl_srvc_handle,srvc_handle, status);
}

static void btut_gatts_characteristic_added_callback(int status, int server_if, bt_uuid_t *uuid, int srvc_handle, int char_handle)
{
    BTUT_Logi("[GATTS] %s()\n", __func__);
    char uuid_s[37];
    btut_gatt_print_uuid(uuid,uuid_s);

    BTUT_Logi("add char uuid:%s in service:%d handle = %d, status = %d\n",uuid_s,srvc_handle,char_handle,status);
}

static void btut_gatts_descriptor_added_callback(int status, int server_if, bt_uuid_t *uuid, int srvc_handle, int descr_handle)
{
    BTUT_Logi("[GATTS] %s()\n", __func__);
    char uuid_s[37];
    btut_gatt_print_uuid(uuid,uuid_s);

    BTUT_Logi("add descriptor uuid:%s in service:%d handle = %d, status = %d\n",uuid_s,srvc_handle,descr_handle,status);
}

static void btut_gatts_service_started_callback(int status, int server_if, int srvc_handle)
{
    BTUT_Logi("[GATTS] %s()\n", __func__);
    BTUT_Logi("service started handle = %d, status = %d\n",srvc_handle,status);
}

static void btut_gatts_service_stopped_callback(int status, int server_if, int srvc_handle)
{
    BTUT_Logi("[GATTS] %s()\n", __func__);
    BTUT_Logi("service stopped handle = %d, status = %d\n",srvc_handle,status);
}

static void btut_gatts_service_deleted_callback(int status, int server_if, int srvc_handle)
{
    BTUT_Logi("[GATTS] %s()\n", __func__);
    BTUT_Logi("service stopped handle = %d, status = %d\n",srvc_handle,status);
}

static void btut_gatts_request_read_callback(int conn_id, int trans_id, bt_bdaddr_t *bda, int attr_handle, int offset, bool is_long)
{
    BTUT_Logi("[GATTS] %s()\n", __func__);
    BTUT_Logi("%02X:%02X:%02X:%02X:%02X:%02X\n request read, trans_id = %d , attr_handle = %d\n",
        bda->address[0], bda->address[1], bda->address[2],
        bda->address[3], bda->address[4], bda->address[5],trans_id,attr_handle);
    BTUT_Logd("Call btut_gatts_send_response() to send data.\n");
}

static void btut_gatts_request_write_callback(int conn_id, int trans_id, bt_bdaddr_t *bda, int attr_handle, int offset, int length,
                                       bool need_rsp, bool is_prep, uint8_t* value)
{
    BTUT_Logi("[GATTS] %s()\n", __func__);
    BTUT_Logi("%02X:%02X:%02X:%02X:%02X:%02X\n request write, need_respond = %d , trans_id = %d , attr_handle = %d,\n value:%s",
        bda->address[0], bda->address[1], bda->address[2],
        bda->address[3], bda->address[4], bda->address[5],need_rsp,trans_id,attr_handle,value);
    if (need_rsp)
    {
        btgatt_response_t response;
        response.handle = attr_handle;
        response.attr_value.len = 0;
        btut_gatts_interface->send_response(conn_id,trans_id,BT_STATUS_SUCCESS,&response);
    }
}

static void btut_gatts_request_exec_write_callback(int conn_id, int trans_id, bt_bdaddr_t *bda, int exec_write)
{
    BTUT_Logi("[GATTS] %s()\n", __func__);
    BTUT_Logi("%02X:%02X:%02X:%02X:%02X:%02X\n request exec write, trans_id = %d , exec_write = %d\n",
        bda->address[0], bda->address[1], bda->address[2],
        bda->address[3], bda->address[4], bda->address[5],conn_id,trans_id,exec_write);
}

static void btut_gatts_response_confirmation_callback(int status, int handle)
{
    BTUT_Logi("[GATTS] %s()\n", __func__);
    BTUT_Logi("response confirmation handle = %d, status = %d\n",handle,status);
}

static void btut_gatts_indication_sent_callback(int conn_id, int status)
{
    BTUT_Logi("[GATTS] %s()\n", __func__);
    BTUT_Logi("indication sent conn_id = %d, status = %d\n",conn_id,status);
}

static void btut_gatts_congestion_callback(int conn_id, bool congested)
{
    BTUT_Logi("[GATTS] %s()\n", __func__);
    BTUT_Logi("conn_id = %d, congestion = %d\n",conn_id,congested);
}

static void btut_gatts_mtu_changed_callback(int conn_id, int mtu)
{
    BTUT_Logi("[GATTS] %s()\n", __func__);
    BTUT_Logi("conn_id = %d, mtu = %d\n",conn_id,mtu);
}

const btgatt_server_callbacks_t btut_gatts_callbacks =
{
    btut_gatts_register_server_callback,
    btut_gatts_connection_callback,
    btut_gatts_service_added_callback,
    btut_gatts_included_service_added_callback,
    btut_gatts_characteristic_added_callback,
    btut_gatts_descriptor_added_callback,
    btut_gatts_service_started_callback,
    btut_gatts_service_stopped_callback,
    btut_gatts_service_deleted_callback,
    btut_gatts_request_read_callback,
    btut_gatts_request_write_callback,
    btut_gatts_request_exec_write_callback,
    btut_gatts_response_confirmation_callback,
    btut_gatts_indication_sent_callback,
    btut_gatts_congestion_callback,
    btut_gatts_mtu_changed_callback,
};

// For handling incoming commands from CLI.
int btut_gatts_cmd_handler(int argc, char **argv)
{
    BTUT_CLI *cmd, *match = NULL;
    int ret = 0;
    int count;

    count = 0;
    cmd = bt_gatts_cli_commands;

    BTUT_Logd("[GATTS] argc: %d, argv[0]: %s\n", argc, argv[0]);

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
        BTUT_Logd("[GATTS] Unknown command '%s'\n", argv[0]);

        btut_print_cmd_help(CMD_KEY_GATTS, bt_gatts_cli_commands);
        ret = -1;
    }
    else
    {
        match->handler(argc - 1, &argv[1]);
    }

    return ret;
}

int btut_gatts_init(const btgatt_server_interface_t *interface)
{
    BTUT_Logi("%s", __func__);
    int ret = 0;
    BTUT_MOD hid_mod = {0};

    // Register command to CLI
    hid_mod.mod_id = BTUT_MOD_GATT_SERVER;
    strncpy(hid_mod.cmd_key, CMD_KEY_GATTS, sizeof(hid_mod.cmd_key));
    hid_mod.cmd_handler = btut_gatts_cmd_handler;
    hid_mod.cmd_tbl = bt_gatts_cli_commands;

    ret = btut_register_mod(&hid_mod);
    BTUT_Logd("[GATTS] btut_register_mod() returns: %d\n", ret);
    btut_gatts_interface = interface;

    bt_uuid_t uuid;
    btut_gatt_uuid_stoh(BTUT_GATTS_SERVER_UUID, &uuid);

    if (btut_gatts_interface->register_server(&uuid))
    {
        BTUT_Logi("[GATTS] Register server uuid:'%s'\n", BTUT_GATTS_SERVER_UUID);
    }
    return ret;
}

int btut_gatts_deinit()
{
    BTUT_Logi("%s", __func__);
    /*if (pxpm_callback != NULL)
    {
        btut_pxpm_deinit();
    }*/
    return 0;
}


