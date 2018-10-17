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


/* FILE NAME:  linuxbt_gatts_if.c
 * AUTHOR: Xuemei Yang
 * PURPOSE:
 *      It provides GATTS operation interface and callback function implementation to MW higher layer.
 * NOTES:
 */


/*-----------------------------------------------------------------------------
                    include files
 ----------------------------------------------------------------------------*/

#include <string.h>
#include "bluetooth.h"
#include "u_bt_mw_common.h"
#include "bt_mw_common.h"
#include "u_bt_mw_gatt.h"
#include "bt_mw_log.h"
#include "linuxbt_gatts_if.h"
#include "linuxbt_common.h"

#define CHECK_GATTS_INTERFACE() if (NULL == linuxbt_gatts_interface)\
{\
    BT_DBG_ERROR(BT_DEBUG_GATT, "[GATTS] linuxbt_gatts_interface not init.");\
    return BT_ERR_STATUS_PARM_INVALID;\
}

#define CHECK_GATTS_BT_ADDR(bt_addr) if (NULL == bt_addr)\
{\
    BT_DBG_ERROR(BT_DEBUG_GATT,"[GATTS] null pointer");\
    return BT_ERR_STATUS_PARM_INVALID;\
}

static int g_linuxbt_server_if = -1;
static const btgatt_server_interface_t *linuxbt_gatts_interface = NULL;

static void linuxbt_gatts_register_server_callback(int status,
                                                             int server_if,
                                                             bt_uuid_t *app_uuid);

static void linuxbt_gatts_connection_callback(int conn_id, int server_if,
                                                        int connected, bt_bdaddr_t *bda);

static void linuxbt_gatts_service_added_callback(int status,
                                                            int server_if,
                                                            btgatt_srvc_id_t *srvc_id,
                                                            int srvc_handle);

static void linuxbt_gatts_included_service_added_callback(int status,
                                                                         int server_if,
                                                                         int srvc_handle,
                                                                         int incl_srvc_handle);

static void linuxbt_gatts_characteristic_added_callback(int status,
                                                                    int server_if,
                                                                    bt_uuid_t *uuid,
                                                                    int srvc_handle,
                                                                    int char_handle);

static void linuxbt_gatts_descriptor_added_callback(int status,
                                                                int server_if,
                                                                bt_uuid_t *uuid,
                                                                int srvc_handle,
                                                                int descr_handle);

static void linuxbt_gatts_service_started_callback(int status,
                                                             int server_if,
                                                             int srvc_handle);

static void linuxbt_gatts_service_stopped_callback(int status,
                                                               int server_if,
                                                               int srvc_handle);

static void linuxbt_gatts_service_deleted_callback(int status,
                                                              int server_if,
                                                              int srvc_handle);

static void linuxbt_gatts_request_read_callback(int conn_id,
                                                           int trans_id,
                                                           bt_bdaddr_t *bda,
                                                           int attr_handle,
                                                           int offset,
                                                           bool is_long);

static void linuxbt_gatts_request_write_callback(int conn_id,
                                                           int trans_id,
                                                           bt_bdaddr_t *bda,
                                                           int attr_handle,
                                                           int offset,
                                                           int length,
                                                           bool need_rsp,
                                                           bool is_prep,
                                                           uint8_t* value);

static void linuxbt_gatts_request_exec_write_callback(int conn_id,
                                                                   int trans_id,
                                                                   bt_bdaddr_t *bda,
                                                                   int exec_write);

static void linuxbt_gatts_response_confirmation_callback(int status,
                                                                       int handle);

static void linuxbt_gatts_indication_sent_callback(int conn_id, int status);

static void linuxbt_gatts_mtu_changed_callback(int conn_id, int mtu);

const btgatt_server_callbacks_t linuxbt_gatts_callbacks =
{
    linuxbt_gatts_register_server_callback,
    linuxbt_gatts_connection_callback,
    linuxbt_gatts_service_added_callback,
    linuxbt_gatts_included_service_added_callback,
    linuxbt_gatts_characteristic_added_callback,
    linuxbt_gatts_descriptor_added_callback,
    linuxbt_gatts_service_started_callback,
    linuxbt_gatts_service_stopped_callback,
    linuxbt_gatts_service_deleted_callback,
    linuxbt_gatts_request_read_callback,
    linuxbt_gatts_request_write_callback,
    linuxbt_gatts_request_exec_write_callback,
    linuxbt_gatts_response_confirmation_callback,
    linuxbt_gatts_indication_sent_callback,
    NULL,
    linuxbt_gatts_mtu_changed_callback,
};

int linuxbt_gatts_init(const btgatt_server_interface_t *pt_interface)
{
    linuxbt_gatts_interface = pt_interface;
    bt_uuid_t uuid;
    linuxbt_uuid_stoh(LINUXBT_GATTS_SERVER_UUID, &uuid);
    return BT_SUCCESS;
}

int linuxbt_gatts_deinit(void)
{
    return BT_SUCCESS;
}

int linuxbt_gatts_register_server(char *uuid)
{
    bt_status_t ret= BT_STATUS_SUCCESS;
    CHECK_GATTS_INTERFACE();
    if (NULL == linuxbt_gatts_interface->register_server)
    {
        BT_DBG_ERROR(BT_DEBUG_GATT,"[GATTS] null pointer of register_server");
        return BT_ERR_STATUS_PARM_INVALID;
    }
    bt_uuid_t app_uuid;
    linuxbt_uuid_stoh(uuid, &app_uuid);
    ret = linuxbt_gatts_interface->register_server(&app_uuid);
    BT_DBG_MINOR(BT_DEBUG_GATT,"ret = %d", ret);
    return linuxbt_return_value_convert(ret);
}

int linuxbt_gatts_unregister_server(int server_if)
{
    bt_status_t ret= BT_STATUS_SUCCESS;
    CHECK_GATTS_INTERFACE();
    ret = linuxbt_gatts_interface->unregister_server(server_if);
    BT_DBG_MINOR(BT_DEBUG_GATT,"ret = %d", ret);
    return linuxbt_return_value_convert(ret);
}

int linuxbt_gatts_connect(int server_if, char *bt_addr, uint8_t is_direct,
                                int transport)
{
    bt_status_t ret= BT_STATUS_SUCCESS;
    bt_bdaddr_t bdaddr;
    CHECK_GATTS_INTERFACE();
    CHECK_GATTS_BT_ADDR(bt_addr);

    /*GATTS open BD_ADDR [isDirect:true|false] [transport]*/
    linuxbt_btaddr_stoh(bt_addr, &bdaddr);
    BT_DBG_INFO(BT_DEBUG_GATT,"GATTS connect to %02X:%02X:%02X:%02X:%02X:%02X ,sever_if:%d",
                bdaddr.address[0], bdaddr.address[1], bdaddr.address[2],
                bdaddr.address[3], bdaddr.address[4], bdaddr.address[5], server_if);

    ret = linuxbt_gatts_interface->connect(server_if,&bdaddr,is_direct,transport);
    BT_DBG_MINOR(BT_DEBUG_GATT,"ret = %d", ret);
    return linuxbt_return_value_convert(ret);

}

int linuxbt_gatts_disconnect(int server_if, char *bt_addr, int conn_id)
{
    bt_status_t ret= BT_STATUS_SUCCESS;
    bt_bdaddr_t bdaddr;
    CHECK_GATTS_INTERFACE();
    CHECK_GATTS_BT_ADDR(bt_addr);
    linuxbt_btaddr_stoh(bt_addr, &bdaddr);
    BT_DBG_INFO(BT_DEBUG_GATT,"GATTS disconnect to %02X:%02X:%02X:%02X:%02X:%02X ,server_if:%d",
                bdaddr.address[0], bdaddr.address[1], bdaddr.address[2],
                bdaddr.address[3], bdaddr.address[4], bdaddr.address[5], server_if);

    ret = linuxbt_gatts_interface->disconnect(server_if,&bdaddr,conn_id);
    BT_DBG_MINOR(BT_DEBUG_GATT,"ret = %d", ret);
    return linuxbt_return_value_convert(ret);
}

int linuxbt_gatts_add_service(int server_if, char *pt_service_uuid,
                                      uint8_t is_primary, int number)
{
    bt_status_t ret= BT_STATUS_SUCCESS;
    CHECK_GATTS_INTERFACE();
    if (NULL == pt_service_uuid)
    {
        BT_DBG_ERROR(BT_DEBUG_GATT,"[GATTS] null pointer");
        return BT_ERR_STATUS_PARM_INVALID;
    }
    /* one service,one char and no desc*/
    /*GATTS add_service uuid [is_primary:true|false] [number_of_handles]*/
    btgatt_srvc_id_t srvc_id;
    bt_uuid_t svc_uuid;
    linuxbt_uuid_stoh(pt_service_uuid, &svc_uuid);
    memcpy(&srvc_id.id.uuid, &svc_uuid, sizeof(bt_uuid_t));
    srvc_id.is_primary = is_primary;

    ret = linuxbt_gatts_interface->add_service(server_if,&srvc_id,number);
    BT_DBG_MINOR(BT_DEBUG_GATT,"ret = %d", ret);
    return linuxbt_return_value_convert(ret);
}

int linuxbt_gatts_add_char(int server_if, int service_handle, char *uuid,
                                  int properties, int permissions)
{
    bt_status_t ret= BT_STATUS_SUCCESS;
    bt_uuid_t char_uuid;
    CHECK_GATTS_INTERFACE();
    linuxbt_uuid_stoh(uuid, &char_uuid);
    /*GATTS add_char service_handle uuid [properties] [permissions]*/
    ret = linuxbt_gatts_interface->add_characteristic(server_if,
            service_handle,
            &char_uuid,
            properties,
            permissions);
    BT_DBG_MINOR(BT_DEBUG_GATT,"ret = %d", ret);
    return linuxbt_return_value_convert(ret);
}

int linuxbt_gatts_add_desc(int server_if, int service_handle,
                                   char *uuid, int permissions)
{
    bt_status_t ret= BT_STATUS_SUCCESS;
    bt_uuid_t desc_uuid;
    CHECK_GATTS_INTERFACE();
    /*GATTS add_desc service_handle uuid [permissions]*/
    linuxbt_uuid_stoh(uuid, &desc_uuid);
    ret = linuxbt_gatts_interface->add_descriptor(server_if,service_handle,&desc_uuid,permissions);
    BT_DBG_MINOR(BT_DEBUG_GATT,"ret = %d", ret);
    return linuxbt_return_value_convert(ret);
}

int linuxbt_gatts_add_included_service(int server_if,
                                                  int service_handle,
                                                  int included_handle)
{
    bt_status_t ret= BT_STATUS_SUCCESS;
    CHECK_GATTS_INTERFACE();
    /*GATTS add_included_service service_handle included_handle*/
    ret = linuxbt_gatts_interface->add_included_service(server_if,service_handle,included_handle);
    BT_DBG_MINOR(BT_DEBUG_GATT,"ret = %d", ret);
    return linuxbt_return_value_convert(ret);
}

int linuxbt_gatts_start_service(int server_if, int service_handle, int transport)
{
    bt_status_t ret= BT_STATUS_SUCCESS;
    CHECK_GATTS_INTERFACE();
    ret = linuxbt_gatts_interface->start_service(server_if,service_handle,transport);
    BT_DBG_MINOR(BT_DEBUG_GATT,"ret = %d", ret);
    return linuxbt_return_value_convert(ret);
}

int linuxbt_gatts_stop_service(int server_if, int service_handle)
{
    bt_status_t ret= BT_STATUS_SUCCESS;
    CHECK_GATTS_INTERFACE();
    ret = linuxbt_gatts_interface->stop_service(server_if, service_handle);
    BT_DBG_MINOR(BT_DEBUG_GATT,"ret = %d", ret);
    return linuxbt_return_value_convert(ret);
}

int linuxbt_gatts_delete_service(int server_if, int service_handle)
{
    bt_status_t ret= BT_STATUS_SUCCESS;
    CHECK_GATTS_INTERFACE();
    ret = linuxbt_gatts_interface->delete_service(server_if,service_handle);
    BT_DBG_MINOR(BT_DEBUG_GATT,"ret = %d", ret);
    return linuxbt_return_value_convert(ret);

}

int linuxbt_gatts_send_indication(int server_if,
                                          int attribute_handle,
                                          int conn_id,
                                          int fg_confirm,
                                          char* p_value,
                                          int value_len)
{
    bt_status_t ret= BT_STATUS_SUCCESS;
    CHECK_GATTS_INTERFACE();
    if (NULL == p_value)
    {
        BT_DBG_ERROR(BT_DEBUG_GATT,"[GATTS] null pointer");
        return BT_ERR_STATUS_PARM_INVALID;
    }
    /*GATTS send_indi attribute_handle conn_id [confirm] value*/
    ret = linuxbt_gatts_interface->send_indication(server_if,
            attribute_handle,
            conn_id,
            value_len,
            fg_confirm,
            p_value);
    BT_DBG_MINOR(BT_DEBUG_GATT,"ret = %d", ret);
    return linuxbt_return_value_convert(ret);

}

int linuxbt_gatts_send_response(int conn_id,
                                          int trans_id,
                                          int status,
                                          int handle,
                                          char *p_value,
                                          int value_len,
                                          int auth_req)
{
    bt_status_t ret= BT_STATUS_SUCCESS;
    btgatt_response_t response;
    CHECK_GATTS_INTERFACE();
    /*GATTS send_response conn_id trans_id status handle [auth_req] value*/
    if (value_len > 0)
    {
        memcpy(response.attr_value.value, p_value, value_len);
    }
    response.attr_value.handle = handle;
    response.attr_value.offset = 0;
    response.attr_value.len = value_len;
    response.attr_value.auth_req = auth_req;

    ret = linuxbt_gatts_interface->send_response(conn_id,trans_id,status,&response);
    BT_DBG_MINOR(BT_DEBUG_GATT,"ret = %d", ret);
    return linuxbt_return_value_convert(ret);

}

static void linuxbt_gatts_register_server_callback(int status,
                                                             int server_if,
                                                             bt_uuid_t *app_uuid)
{
    BT_DBG_INFO(BT_DEBUG_GATT,"[GATTS] Register server callback server_if:%d, status:%d",
                 server_if,status);
    if (0 == status)
    {
        g_linuxbt_server_if = server_if;
    }
    char uuid[BT_GATT_MAX_UUID_LEN] = {0};
    linuxbt_print_uuid(app_uuid, uuid);
    bluetooth_gatts_register_server_callback(status, server_if, uuid);
}

static void linuxbt_gatts_connection_callback(int conn_id, int server_if,
                                                        int connected, bt_bdaddr_t *bda)
{
    BT_DBG_INFO(BT_DEBUG_GATT, "%02X:%02X:%02X:%02X:%02X:%02X\n connected = %d, conn_id = %d",
                bda->address[0], bda->address[1], bda->address[2],
                bda->address[3], bda->address[4], bda->address[5],connected,conn_id);
    char btaddr_s[MAX_BDADDR_LEN];
    linuxbt_btaddr_htos(bda, btaddr_s);
    bluetooth_gatts_connection_callback(conn_id, server_if, connected, btaddr_s);

#if MTK_LINUX_GATTC_PTS_TEST
    if (connected == 1)
    {
        linuxbt_gatts_connect(server_if, btaddr_s, 1, 2);
    }
#endif
}

static void linuxbt_gatts_service_added_callback(int status,
                                                            int server_if,
                                                            btgatt_srvc_id_t *srvc_id,
                                                            int srvc_handle)
{
    bt_uuid_t uuid = srvc_id->id.uuid;
    char uuid_s[BT_GATT_MAX_UUID_LEN] = {0};
    BT_GATT_SRVC_ID_T bt_srvc_id;
    linuxbt_print_uuid(&uuid,uuid_s);

    BT_DBG_INFO(BT_DEBUG_GATT,"add service uuid:%s handle = %d, status = %d",
                uuid_s, srvc_handle, status);
    bt_srvc_id.is_primary = srvc_id->is_primary;
    bt_srvc_id.id.inst_id = srvc_id->id.inst_id;
    linuxbt_print_uuid(&uuid,bt_srvc_id.id.uuid);
    bluetooth_gatts_service_added_callback(status, server_if, &bt_srvc_id, srvc_handle);
}

static void linuxbt_gatts_included_service_added_callback(int status,
                                                                         int server_if,
                                                                         int srvc_handle,
                                                                         int incl_srvc_handle)
{
    BT_DBG_INFO(BT_DEBUG_GATT, "add included service:%d in service: %d, status = %d",
                incl_srvc_handle, srvc_handle, status);
    bluetooth_gatts_included_service_added_callback(status, server_if, srvc_handle,
                                                    incl_srvc_handle);
}

static void linuxbt_gatts_characteristic_added_callback(int status,
                                                                    int server_if,
                                                                    bt_uuid_t *uuid,
                                                                    int srvc_handle,
                                                                    int char_handle)
{
    char uuid_s[BT_GATT_MAX_UUID_LEN] = {0};
    linuxbt_print_uuid(uuid,uuid_s);

    BT_DBG_INFO(BT_DEBUG_GATT, "add char uuid:%s in service:%d handle = %d, status = %d",
                uuid_s, srvc_handle, char_handle, status);
    bluetooth_gatts_characteristic_added_callback(status, server_if, uuid_s,
                                                  srvc_handle, char_handle);
}

static void linuxbt_gatts_descriptor_added_callback(int status,
                                                                int server_if,
                                                                bt_uuid_t *uuid,
                                                                int srvc_handle,
                                                                int descr_handle)
{
    char uuid_s[BT_GATT_MAX_UUID_LEN] = {0};
    linuxbt_print_uuid(uuid,uuid_s);

    BT_DBG_INFO(BT_DEBUG_GATT,"add descriptor uuid:%s in service:%d handle = %d, status = %d",
                uuid_s, srvc_handle, descr_handle, status);
    bluetooth_gatts_descriptor_added_callback(status, server_if, uuid_s,
                                              srvc_handle, descr_handle);
}

static void linuxbt_gatts_service_started_callback(int status,
                                                             int server_if,
                                                             int srvc_handle)
{
    BT_DBG_INFO(BT_DEBUG_GATT, "service started handle = %d, status = %d",
                srvc_handle, status);
    bluetooth_gatts_service_started_callback(status, server_if, srvc_handle);
}

static void linuxbt_gatts_service_stopped_callback(int status,
                                                               int server_if,
                                                               int srvc_handle)
{
    BT_DBG_INFO(BT_DEBUG_GATT,"service stopped handle = %d, status = %d",
                srvc_handle, status);
    bluetooth_gatts_service_stopped_callback(status, server_if, srvc_handle);
}

static void linuxbt_gatts_service_deleted_callback(int status,
                                                              int server_if,
                                                              int srvc_handle)
{
    BT_DBG_INFO(BT_DEBUG_GATT,"service stopped handle = %d, status = %d",
                srvc_handle, status);
    bluetooth_gatts_service_deleted_callback(status, server_if, srvc_handle);
}

static void linuxbt_gatts_request_read_callback(int conn_id,
                                                           int trans_id,
                                                           bt_bdaddr_t *bda,
                                                           int attr_handle,
                                                           int offset,
                                                           bool is_long)
{
    BT_DBG_INFO(BT_DEBUG_GATT, "%02X:%02X:%02X:%02X:%02X:%02X\n request read, trans_id = %d , attr_handle = %d",
                bda->address[0], bda->address[1], bda->address[2],
                bda->address[3], bda->address[4], bda->address[5],trans_id,attr_handle);
    char btaddr_s[MAX_BDADDR_LEN];
    linuxbt_btaddr_htos(bda, btaddr_s);
    bluetooth_gatts_request_read_callback(conn_id, trans_id, btaddr_s,
                                          attr_handle, offset, is_long);
}

static void linuxbt_gatts_request_write_callback(int conn_id,
                                                           int trans_id,
                                                           bt_bdaddr_t *bda,
                                                           int attr_handle,
                                                           int offset,
                                                           int length,
                                                           bool need_rsp,
                                                           bool is_prep,
                                                           uint8_t* value)
{
    BT_DBG_INFO(BT_DEBUG_GATT, "%02X:%02X:%02X:%02X:%02X:%02X\n request write, need_respond = %d , trans_id = %d , attr_handle = %d, value:%s",
                bda->address[0], bda->address[1], bda->address[2],
                bda->address[3], bda->address[4], bda->address[5],need_rsp,trans_id,attr_handle,value);
    char btaddr_s[MAX_BDADDR_LEN];
    linuxbt_btaddr_htos(bda, btaddr_s);
    bluetooth_gatts_request_write_callback(conn_id,
                                           trans_id,
                                           btaddr_s,
                                           attr_handle,
                                           offset,
                                           length,
                                           need_rsp,
                                           is_prep,
                                           value);
}

static void linuxbt_gatts_request_exec_write_callback(int conn_id,
                                                                   int trans_id,
                                                                   bt_bdaddr_t *bda,
                                                                   int exec_write)
{
    BT_DBG_INFO(BT_DEBUG_GATT,"%02X:%02X:%02X:%02X:%02X:%02X\n request exec write, conn_id = %d,trans_id = %d , exec_write = %d",
                bda->address[0], bda->address[1], bda->address[2],
                bda->address[3], bda->address[4], bda->address[5],conn_id,trans_id,exec_write);

#if !MTK_LINUX_GATTC_PTS_TEST
    btgatt_response_t response;
    response.handle = conn_id;
    response.attr_value.len = 0;
    if (NULL == linuxbt_gatts_interface)
    {
        BT_DBG_ERROR(BT_DEBUG_GATT,"[GATTS] linuxbt_gatts_interface not init.");
        return;
    }
    linuxbt_gatts_interface->send_response(conn_id,trans_id,BT_STATUS_SUCCESS,&response);
#endif
    char btaddr_s[MAX_BDADDR_LEN];
    linuxbt_btaddr_htos(bda, btaddr_s);
    bluetooth_gatts_request_exec_write_callback(conn_id, trans_id, btaddr_s, exec_write);
}

static void linuxbt_gatts_response_confirmation_callback(int status,
                                                                       int handle)
{
    BT_DBG_INFO(BT_DEBUG_GATT,"response confirmation handle = %d, status = %d",
                handle, status);
}

static void linuxbt_gatts_indication_sent_callback(int conn_id, int status)
{
    BT_DBG_INFO(BT_DEBUG_GATT, "indication sent conn_id = %d, status = %d",
                conn_id, status);
    bluetooth_gatts_indication_sent_callback(conn_id, status);
}

static void linuxbt_gatts_mtu_changed_callback(int conn_id, int mtu)
{
    BT_DBG_INFO(BT_DEBUG_GATT, "conn_id = %d, mtu = %d",conn_id,mtu);
    bluetooth_gatts_mtu_changed_callback(conn_id,mtu);
}

