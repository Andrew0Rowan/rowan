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


/* FILE NAME:  bt_mw_gatts.c
 * AUTHOR: Xuemei Yang
 * PURPOSE:
 *      It provides GATTS API to c_bt_mw_gatt and other mw layer modules.
 * NOTES:
 */

/*-----------------------------------------------------------------------------
                    include files
 ----------------------------------------------------------------------------*/
#include <stddef.h>
#include "linuxbt_gatts_if.h"
#include "bt_mw_gatt.h"
#include "bt_mw_log.h"

/*GATT server*/

#if MTK_LINUX_GATTC_PTS_TEST
#define PTS_TEST_MTU 23
#define PTS_MAX_LENGTH 6
static CHAR *g_pts_test_value = NULL;
static INT32 g_pts_write_offset = 0;
static INT32 g_pts_exe_write = 1;
static CHAR g_pts_test_read_value[BT_GATT_MAX_ATTR_LEN] = {0};
static CHAR *g_pts_test_value_invalid = NULL;
static CHAR g_pts_test_read_value_invalid[BT_GATT_MAX_ATTR_LEN] = {0};
static INT32 g_pts_invalid_length = 0;
#endif

static BtAppGATTSEventCbk BtGattsAppCbk = NULL;
static BtAppGATTSRegServerCbk BtGattsRegSeverCbk = NULL;
static BtAppGATTSAddSrvcCbk BtGattsAddSrvcCbk = NULL;
static BtAppGATTSAddInclCbk BtGattsAddInclCbk = NULL;
static BtAppGATTSAddCharCbk BtGattsAddCharCbk = NULL;
static BtAppGATTSAddDescCbk BtGattsAddDescCbk = NULL;
static BtAppGATTSOpSrvcCbk BtGattsOpSrvcCbk = NULL;
static BtAppGATTSReqReadCbk BtGattsReqReadCbk = NULL;
static BtAppGATTSReqWriteCbk BtGattsReqWriteCbk = NULL;
static BtAppGATTSIndSentCbk BtGattsIndSentCbk = NULL;


static BT_GATTS_SRVC_RST_T g_gatts_srvc_info = {0};
static BT_GATTS_CONNECT_RST_T g_gatts_connect_rst_info = {0};
static BT_GATTS_CONNECT_RST_T g_gatts_disconnect_rst_info = {0};

static INT32 g_server_if = 0;
static INT32 g_srvc_handle = 0;
static INT32 g_char_handle = 0;
static BOOL g_fg_gatts_reg_done = FALSE;
static BOOL g_fg_gatts_add_service = FALSE;
static BOOL g_fg_gatts_add_char = FALSE;
static BOOL g_fg_gatts_start_service = FALSE;

/*GATT server*/
static INT32 bluetooth_gatts_reg_cbk_fct(BT_APP_GATTS_CB_FUNC_T *func)
{
    BT_ERR_STATUS_T ret = BT_SUCCESS;
    BT_DBG_NORMAL(BT_DEBUG_GATT, "start bluetooth_gatts_register_cbk_fct");

    if (NULL == func)
    {
        BT_DBG_ERROR(BT_DEBUG_GATT, "callback func is null!");
        return BT_ERR_STATUS_PARM_INVALID;
    }
    /*1*/
    if (func->bt_gatts_event_cb)
    {
        BtGattsAppCbk = func->bt_gatts_event_cb;
    }
    else
    {
        BT_DBG_NOTICE(BT_DEBUG_GATT, "gatts event callback func is null!");
        BtGattsAppCbk = NULL;
    }
    /*2*/
    if (func->bt_gatts_reg_server_cb)
    {
        BtGattsRegSeverCbk = func->bt_gatts_reg_server_cb;
    }
    else
    {
        BT_DBG_NOTICE(BT_DEBUG_GATT, "gatts register server callback func is null!");
        BtGattsRegSeverCbk = NULL;
    }
    /*3*/
    if (func->bt_gatts_add_srvc_cb)
    {
        BtGattsAddSrvcCbk = func->bt_gatts_add_srvc_cb;
    }
    else
    {
        BT_DBG_NOTICE(BT_DEBUG_GATT, "gatts add service callback func is null!");
        BtGattsAddSrvcCbk = NULL;
    }
    /*4*/
    if (func->bt_gatts_add_incl_cb)
    {
        BtGattsAddInclCbk = func->bt_gatts_add_incl_cb;
    }
    else
    {
        BT_DBG_NOTICE(BT_DEBUG_GATT, "gatts add include callback func is null!");
        BtGattsAddInclCbk = NULL;
    }
    /*5*/
    if (func->bt_gatts_add_char_cb)
    {
        BtGattsAddCharCbk = func->bt_gatts_add_char_cb;
    }
    else
    {
        BT_DBG_NOTICE(BT_DEBUG_GATT, "gatts add characteristic callback func is null!");
        BtGattsAddCharCbk = NULL;
    }
    /*6*/
    if (func->bt_gatts_add_desc_cb)
    {
        BtGattsAddDescCbk = func->bt_gatts_add_desc_cb;
    }
    else
    {
        BT_DBG_NOTICE(BT_DEBUG_GATT, "gatts add descripor callback func is null!");
        BtGattsAddDescCbk = NULL;
    }
    /*7*/
    if (func->bt_gatts_op_srvc_cb)
    {
        BtGattsOpSrvcCbk = func->bt_gatts_op_srvc_cb;
    }
    else
    {
        BT_DBG_NOTICE(BT_DEBUG_GATT, "gatts operate service callback func is null!");
        BtGattsOpSrvcCbk = NULL;
    }
    /*8*/
    if (func->bt_gatts_req_read_cb)
    {
        BtGattsReqReadCbk = func->bt_gatts_req_read_cb;
    }
    else
    {
        BT_DBG_NOTICE(BT_DEBUG_GATT, "gatts request read callback func is null!");
        BtGattsReqReadCbk = NULL;
    }
    /*9*/
    if (func->bt_gatts_req_write_cb)
    {
        BtGattsReqWriteCbk = func->bt_gatts_req_write_cb;
    }
    else
    {
        BT_DBG_NOTICE(BT_DEBUG_GATT, "gatts request read callback func is null!");
        BtGattsReqWriteCbk = NULL;
    }
    /*10*/
    if (func->bt_gatts_ind_sent_cb)
    {
        BtGattsIndSentCbk = func->bt_gatts_ind_sent_cb;
    }
    else
    {
        BT_DBG_NOTICE(BT_DEBUG_GATT, "gatts indication sent callback func is null!");
        BtGattsIndSentCbk = NULL;
    }


    return ret;
}

INT32 bluetooth_gatts_register_callback(BT_APP_GATTS_CB_FUNC_T *func)
{
    g_fg_gatts_reg_done = FALSE;
    g_fg_gatts_add_service = FALSE;
    g_fg_gatts_add_char = FALSE;
    g_fg_gatts_start_service = FALSE;
    return  bluetooth_gatts_reg_cbk_fct(func);
}

INT32 bluetooth_gatts_register_app_sync(CHAR *pt_uuid, INT32 *server_if)
{
    INT32 i4_loop = 300;
    BT_ERR_STATUS_T ret = BT_SUCCESS;
    g_fg_gatts_reg_done = FALSE;
    ret = linuxbt_gatts_register_server(pt_uuid);
    while (0 < i4_loop && !g_fg_gatts_reg_done)
    {
        BT_DBG_INFO(BT_DEBUG_GATT, "Wait gatts register server finish:%ld", (long)i4_loop);
        usleep(10*1000);
        i4_loop--;
    }
    if (g_fg_gatts_reg_done)
    {
        *server_if = g_server_if;
    }
    else
    {
        BT_DBG_ERROR(BT_DEBUG_GATT, "gatts register server failed");
        *server_if = -1;
        return BT_ERR_STATUS_FAIL;
    }
    return ret;
}

INT32 bluetooth_gatts_register_server(CHAR *pt_uuid)
{
    return linuxbt_gatts_register_server(pt_uuid);
}

INT32 bluetooth_gatts_unregister_server(INT32 server_if)
{
    return linuxbt_gatts_unregister_server(server_if);
}

INT32 bluetooth_gatts_connect(INT32 server_if, CHAR *bt_addr,
                                      UINT8 is_direct, INT32 transport)
{
    return linuxbt_gatts_connect(server_if, bt_addr, is_direct, transport);
}

INT32 bluetooth_gatts_disconnect(INT32 server_if, CHAR *bt_addr, INT32 conn_id)
{
    return linuxbt_gatts_disconnect(server_if, bt_addr, conn_id);
}

INT32 bluetooth_gatts_add_service_sync(INT32 server_if, CHAR *pt_service_uuid,
                                                   UINT8 is_primary, INT32 number,
                                                   INT32 *srvc_handle)
{
    INT32 i4_loop = 300;
    BT_ERR_STATUS_T ret = BT_SUCCESS;
    g_fg_gatts_add_service= FALSE;
    ret = linuxbt_gatts_add_service(server_if, pt_service_uuid, is_primary, number);
    while (0<i4_loop && !g_fg_gatts_add_service)
    {
        BT_DBG_INFO(BT_DEBUG_GATT, "Wait gatts add service finish:%ld", (long)i4_loop);
        usleep(10*1000);
        i4_loop--;
    }
    if (g_fg_gatts_add_service)
    {
        *srvc_handle = g_srvc_handle;
    }
    else
    {
        BT_DBG_ERROR(BT_DEBUG_GATT, "gatts register server failed");
        *srvc_handle = -1;
        return BT_ERR_STATUS_FAIL;
    }
    return ret;
}

INT32 bluetooth_gatts_add_service(INT32 server_if, CHAR *pt_service_uuid,
                                           UINT8 is_primary, INT32 number)
{
    return linuxbt_gatts_add_service(server_if, pt_service_uuid, is_primary, number);
}

INT32 bluetooth_gatts_add_included_service(INT32 server_if,
                                                       INT32 service_handle,
                                                       INT32 included_handle)
{
    return linuxbt_gatts_add_included_service(server_if, service_handle, included_handle);
}

INT32 bluetooth_gatts_add_char_sync(INT32 server_if, INT32 service_handle,
                                               CHAR *uuid, INT32 properties,
                                               INT32 permissions, INT32 *char_handle)
{
    INT32 i4_loop = 300;
    BT_ERR_STATUS_T ret = BT_SUCCESS;
    g_fg_gatts_add_char = FALSE;
    ret = linuxbt_gatts_add_char(server_if, service_handle, uuid, properties, permissions);
    while (0<i4_loop && !g_fg_gatts_add_char)
    {
        BT_DBG_INFO(BT_DEBUG_GATT, "Wait gatts add char finish:%ld", (long)i4_loop);
        usleep(10*1000);
        i4_loop--;
    }
    if (g_fg_gatts_add_char)
    {
        *char_handle = g_char_handle;
    }
    else
    {
        BT_DBG_ERROR(BT_DEBUG_GATT, "gatts register server failed");
        *char_handle = -1;
        return BT_ERR_STATUS_FAIL;
    }
    return ret;
}

INT32 bluetooth_gatts_add_char(INT32 server_if, INT32 service_handle,
                                       CHAR *uuid, INT32 properties,
                                       INT32 permissions)
{
    return linuxbt_gatts_add_char(server_if, service_handle, uuid, properties,
                                  permissions);
}

INT32 bluetooth_gatts_add_desc(INT32 server_if, INT32 service_handle,
                                        CHAR *uuid, INT32 permissions)
{
    return linuxbt_gatts_add_desc(server_if, service_handle, uuid, permissions);
}

INT32 bluetooth_gatts_start_service(INT32 server_if, INT32 service_handle,
                                            INT32 transport)
{
    return linuxbt_gatts_start_service(server_if, service_handle, transport);
}

INT32 bluetooth_gatts_start_service_sync(INT32 server_if,
                                                    INT32 service_handle,
                                                    INT32 transport)
{
    INT32 i4_loop = 300;
    BT_ERR_STATUS_T ret = BT_SUCCESS;
    g_fg_gatts_start_service = FALSE;
    ret = linuxbt_gatts_start_service(server_if, service_handle, transport);
    while (0<i4_loop && !g_fg_gatts_start_service)
    {
        BT_DBG_INFO(BT_DEBUG_GATT, "Wait gatts start Service finish:%ld", (long)i4_loop);
        usleep(10*1000);
        i4_loop--;
    }
    if (g_fg_gatts_start_service)
    {
        return ret;
    }
    else
    {
        BT_DBG_ERROR(BT_DEBUG_GATT, "gatts start service failed");
        return BT_ERR_STATUS_FAIL;
    }
}

INT32 bluetooth_gatts_delete_service(INT32 server_if, INT32 service_handle)
{
    return linuxbt_gatts_delete_service(server_if, service_handle);
}

INT32 bluetooth_gatts_stop_service(INT32 server_if, INT32 service_handle)
{
    return linuxbt_gatts_stop_service(server_if, service_handle);
}

INT32 bluetooth_gatts_send_indication(INT32 server_if,
                                               INT32 attribute_handle,
                                               INT32 conn_id,
                                               INT32 fg_confirm,
                                               CHAR* p_value,
                                               INT32 value_len)
{
    return linuxbt_gatts_send_indication(server_if, attribute_handle, conn_id,
                                         fg_confirm, p_value, value_len);
}

INT32 bluetooth_gatts_send_response(INT32 conn_id, INT32 trans_id,
                                               INT32 status, INT32 handle,
                                               CHAR *p_value, INT32 value_len,
                                               INT32 auth_req)
{
    return linuxbt_gatts_send_response(conn_id, trans_id, status, handle, p_value,
                                       value_len, auth_req);
}

VOID bluetooth_gatts_get_connect_result_info(BT_GATTS_CONNECT_RST_T *connect_rst_info)
{
    if (NULL == connect_rst_info)
    {
        BT_DBG_ERROR(BT_DEBUG_GATT, "[GATT] null pointer");
        return;
    }
    memcpy(connect_rst_info, &g_gatts_connect_rst_info, sizeof(BT_GATTS_CONNECT_RST_T));
}

VOID bluetooth_gatts_get_disconnect_result_info(BT_GATTS_CONNECT_RST_T *disconnect_rst_info)
{
    if (NULL == disconnect_rst_info)
    {
        BT_DBG_ERROR(BT_DEBUG_GATT, "[GATT] null pointer");
        return;
    }
    memcpy(disconnect_rst_info, &g_gatts_disconnect_rst_info, sizeof(BT_GATTS_CONNECT_RST_T));
}


/*********GATTS callback**************/

static VOID bt_mw_gatts_notify_app(tBT_MW_GATTS_MSG *gatts_msg)
{
    tBTMW_GATT_MSG msg;
    msg.hdr.event = BTMW_GATTS_EVENT;
    msg.hdr.len = sizeof(tBT_MW_GATTS_MSG);
    memcpy((void*)&msg.data.gatts_msg, gatts_msg, sizeof(tBT_MW_GATTS_MSG));
    bt_mw_gatt_nty_send_msg(&msg);
}

VOID bt_mw_gatts_nty_handle(tBT_MW_GATTS_MSG *gatts_msg)
{
    BT_DBG_INFO(BT_DEBUG_GATT, "bt_mw_gatt_nty_handle: event:%d", gatts_msg->event);

    switch(gatts_msg->event)
    {
        case BTMW_GATTS_REG_SERVER:
            if (BtGattsRegSeverCbk)
            {
                BtGattsRegSeverCbk(&gatts_msg->gatts_data.gatts_reg_server);
            }
            break;
        case BTMW_GATTS_ADD_SRVC:
            if (BtGattsAddSrvcCbk)
            {
                BtGattsAddSrvcCbk(&gatts_msg->gatts_data.gatts_add_srvc);
            }
            break;
        case BTMW_GATTS_CONNECT:
            if (BtGattsAppCbk)
            {
                BtGattsAppCbk(BT_GATTS_CONNECT);
            }
            break;
        case BTMW_GATTS_DISCONNECT:
            if (BtGattsAppCbk)
            {
                BtGattsAppCbk(BT_GATTS_DISCONNECT);
            }
            break;
        case BTMW_GATTS_ADD_INCL:
            if (BtGattsAddInclCbk)
            {
                BtGattsAddInclCbk(&gatts_msg->gatts_data.gatts_add_incl);
            }
            break;
        case BTMW_GATTS_ADD_CHAR:
            if (BtGattsAddCharCbk)
            {
                BtGattsAddCharCbk(&gatts_msg->gatts_data.gatts_add_char);
            }
            break;
        case BTMW_GATTS_ADD_DESC:
            if (BtGattsAddDescCbk)
            {
                BtGattsAddDescCbk(&gatts_msg->gatts_data.gatts_add_desc);
            }
            break;
        case BTMW_GATTS_REQ_READ:
            if (BtGattsReqReadCbk)
            {
                BtGattsReqReadCbk(&gatts_msg->gatts_data.gatts_req_read);
            }
            break;
        case BTMW_GATTS_REQ_WRITE:
            if (BtGattsReqWriteCbk)
            {
                BtGattsReqWriteCbk(&gatts_msg->gatts_data.gatts_req_write);
            }
            break;
        case BTMW_GATTS_SENT_IND:
            if (BtGattsIndSentCbk)
            {
                BtGattsIndSentCbk(gatts_msg->gatts_data.gatts_sent_ind.conn_id,
                                  gatts_msg->gatts_data.gatts_sent_ind.status);
            }
            break;
        case BTMW_GATTS_START_SRVC:
            if (BtGattsOpSrvcCbk)
            {
                BtGattsOpSrvcCbk(BT_GATTS_START_SRVC, &gatts_msg->gatts_data.gatts_srvc_info);
            }
            break;
        case BTMW_GATTS_STOP_SRVC:
            if (BtGattsOpSrvcCbk)
            {
                BtGattsOpSrvcCbk(BT_GATTS_STOP_SRVC, &gatts_msg->gatts_data.gatts_srvc_info);
            }
            break;
        case BTMW_GATTS_DEL_SRVC:
            if (BtGattsOpSrvcCbk)
            {
                BtGattsOpSrvcCbk(BT_GATTS_DEL_SRVC, &gatts_msg->gatts_data.gatts_srvc_info);
            }
            break;
        default:
            break;
    }
}

VOID bluetooth_gatts_register_server_callback(INT32 status,
                                                          INT32 server_if,
                                                          CHAR *app_uuid)
{
    tBT_MW_GATTS_MSG gatts_msg = {0};
    g_fg_gatts_reg_done = TRUE;
    g_server_if = server_if;
    gatts_msg.event = BTMW_GATTS_REG_SERVER;
    gatts_msg.gatts_data.gatts_reg_server.server_if = server_if;
    memcpy(gatts_msg.gatts_data.gatts_reg_server.app_uuid, app_uuid, strlen(app_uuid) + 1);
    BT_DBG_INFO(BT_DEBUG_GATT, "server_if is: %d", gatts_msg.gatts_data.gatts_reg_server.server_if);
    BT_DBG_INFO(BT_DEBUG_GATT, "uuid is: %s", gatts_msg.gatts_data.gatts_reg_server.app_uuid);

    bt_mw_gatts_notify_app(&gatts_msg);
}

VOID bluetooth_gatts_service_added_callback(INT32 status,
                                                         INT32 server_if,
                                                         BT_GATT_SRVC_ID_T *srvc_id,
                                                         INT32 srvc_handle)
{
    tBT_MW_GATTS_MSG gatts_msg = {0};
    g_fg_gatts_add_service = TRUE;
    g_srvc_handle = srvc_handle;
    gatts_msg.event = BTMW_GATTS_ADD_SRVC;
    gatts_msg.gatts_data.gatts_add_srvc.server_if = server_if;
    gatts_msg.gatts_data.gatts_add_srvc.srvc_handle = srvc_handle;
    memcpy(&gatts_msg.gatts_data.gatts_add_srvc.srvc_id, srvc_id, sizeof(BT_GATT_SRVC_ID_T));

    bt_mw_gatts_notify_app(&gatts_msg);
}

VOID bluetooth_gatts_connection_callback(INT32 conn_id, INT32 server_if,
                                                    INT32 connected, CHAR *bda)
{
    tBT_MW_GATTS_MSG gatts_msg = {0};
    if (0 == connected)
    {
        gatts_msg.event = BTMW_GATTS_DISCONNECT;
        g_gatts_disconnect_rst_info.conn_id = conn_id;
        g_gatts_disconnect_rst_info.server_if = server_if;
        memcpy(g_gatts_disconnect_rst_info.btaddr, bda, strlen(bda) + 1);
    }
    else
    {
        gatts_msg.event = BTMW_GATTS_CONNECT;
        g_gatts_connect_rst_info.conn_id = conn_id;
        g_gatts_connect_rst_info.server_if = server_if;
        memcpy(g_gatts_connect_rst_info.btaddr, bda, strlen(bda) + 1);

    }
    bt_mw_gatts_notify_app(&gatts_msg);
}

VOID bluetooth_gatts_included_service_added_callback(INT32 status,
                                                                     INT32 server_if,
                                                                     INT32 srvc_handle,
                                                                     INT32 incl_srvc_handle)
{
    tBT_MW_GATTS_MSG gatts_msg = {0};
    gatts_msg.event = BTMW_GATTS_ADD_INCL;
    gatts_msg.gatts_data.gatts_add_incl.server_if = server_if;
    gatts_msg.gatts_data.gatts_add_incl.srvc_handle = srvc_handle;
    gatts_msg.gatts_data.gatts_add_incl.incl_srvc_handle = incl_srvc_handle;

    bt_mw_gatts_notify_app(&gatts_msg);
}

VOID bluetooth_gatts_characteristic_added_callback(INT32 status,
                                                                INT32 server_if,
                                                                CHAR *uuid,
                                                                INT32 srvc_handle,
                                                                INT32 char_handle)
{
    tBT_MW_GATTS_MSG gatts_msg = {0};
    g_fg_gatts_add_char = TRUE;
    g_char_handle = char_handle;
    gatts_msg.event = BTMW_GATTS_ADD_CHAR;
    gatts_msg.gatts_data.gatts_add_char.server_if = server_if;
    gatts_msg.gatts_data.gatts_add_char.srvc_handle = srvc_handle;
    gatts_msg.gatts_data.gatts_add_char.char_handle = char_handle;
    memcpy(gatts_msg.gatts_data.gatts_add_char.uuid, uuid, strlen(uuid) + 1);

    bt_mw_gatts_notify_app(&gatts_msg);
}

VOID bluetooth_gatts_descriptor_added_callback(INT32 status,
                                                            INT32 server_if,
                                                            CHAR *uuid,
                                                            INT32 srvc_handle,
                                                            INT32 descr_handle)
{
    tBT_MW_GATTS_MSG gatts_msg = {0};
    gatts_msg.event = BTMW_GATTS_ADD_DESC;
    gatts_msg.gatts_data.gatts_add_desc.server_if = server_if;
    gatts_msg.gatts_data.gatts_add_desc.srvc_handle = srvc_handle;
    gatts_msg.gatts_data.gatts_add_desc.descr_handle = descr_handle;
    memcpy(gatts_msg.gatts_data.gatts_add_desc.uuid, uuid, strlen(uuid) + 1);

    bt_mw_gatts_notify_app(&gatts_msg);
}

#if MTK_LINUX_GATTC_PTS_TEST
static VOID bluetooth_gatts_req_read_invalid_attr_length(INT32 conn_id,
                                                                     INT32 trans_id,
                                                                     INT32 attr_handle)
{
    if (attr_handle == 48 || attr_handle == 49) //for invalid attribute  length
    {
        if (NULL != g_pts_test_value_invalid)
        {
            free(g_pts_test_value_invalid);
            g_pts_test_value_invalid = NULL;
        }
        g_pts_test_value_invalid = malloc(PTS_MAX_LENGTH * sizeof(CHAR));

        if (!g_pts_test_value_invalid)
        {
            BT_DBG_ERROR(BT_DEBUG_GATT, "[GATT] malloc pts_test_value fail!");
            return;
        }
        else
        {
            memset(g_pts_test_value_invalid, 0xaa, PTS_MAX_LENGTH * sizeof(CHAR));
            g_pts_test_value_invalid[PTS_MAX_LENGTH - 1] = '\0';
        }
        if (g_pts_exe_write == 1)
        {
            memcpy(g_pts_test_read_value_invalid, g_pts_test_value_invalid,
                   PTS_MAX_LENGTH * sizeof(CHAR));
            bluetooth_gatts_send_response(conn_id, trans_id, 0, attr_handle,
                                          g_pts_test_value_invalid,
                                          strlen(g_pts_test_value_invalid), 0);
        }
        else
        {
            bluetooth_gatts_send_response(conn_id, trans_id, 0, attr_handle,
                                          g_pts_test_read_value_invalid,
                                          strlen(g_pts_test_read_value_invalid), 0);
        }
    }
    else //for other case
    {
        if (g_pts_test_value == NULL)
        {
            g_pts_test_value = malloc((PTS_TEST_MTU + 10) * sizeof(CHAR));

            if (!g_pts_test_value)
            {
                BT_DBG_ERROR(BT_DEBUG_GATT, "[GATT] malloc pts_test_value fail!");
                return;
            }
            else
            {
                memset(g_pts_test_value, 0x12, (PTS_TEST_MTU + 10) * sizeof(CHAR));
                g_pts_test_value[(PTS_TEST_MTU + 9)] = '\0';
            }
        }

        if (g_pts_exe_write == 1)
        {
            memcpy(g_pts_test_read_value, g_pts_test_value, (PTS_TEST_MTU + 10) * sizeof(CHAR));
            bluetooth_gatts_send_response(conn_id, trans_id, 0, attr_handle,
                                          g_pts_test_value, strlen(g_pts_test_value), 0);
        }
        else
        {
            bluetooth_gatts_send_response(conn_id, trans_id, 0, attr_handle,
                                          g_pts_test_read_value,
                                          strlen(g_pts_test_read_value), 0);
        }
    }
}

static VOID bluetooth_gatts_pts_test_req_read_cb(INT32 conn_id,
                                                             INT32 trans_id,
                                                             INT32 attr_handle,
                                                             INT32 offset)
{
    if (offset == 0)
    {
        bluetooth_gatts_req_read_invalid_attr_length(conn_id, trans_id, attr_handle);
    }
    else if(offset < (PTS_TEST_MTU + 10))
    {

        CHAR *p_value = NULL;
        INT32 len = 0;
        if (g_pts_exe_write == 1)
        {
            p_value = g_pts_test_value + offset;
            len = strlen(g_pts_test_value) - offset + 1;
        }
        else
        {
            p_value = g_pts_test_read_value + offset;
            len = strlen(g_pts_test_read_value) - offset + 1;
        }
        bluetooth_gatts_send_response(conn_id, trans_id, 0, attr_handle, p_value, len, 0);
    }
    else if (offset == (PTS_TEST_MTU + 10))
    {
        bluetooth_gatts_send_response(conn_id, trans_id, 0, attr_handle, NULL, 0, 0);
    }
    else
    {
        //invalid offset
        bluetooth_gatts_send_response(conn_id, trans_id, 0x07, attr_handle, NULL, 0, 0);
    }
}

static VOID bluetooth_gatts_req_write_invalid_attr_length(INT32 conn_id,
                                                                      INT32 trans_id,
                                                                      INT32 attr_handle,
                                                                      INT32 offset,
                                                                      INT32 length,
                                                                      BOOL need_rsp,
                                                                      BOOL is_prep,
                                                                      UINT8* value)
{
    if (attr_handle == 48 || attr_handle == 49)
    {
        memcpy(g_pts_test_value_invalid, value, length);
        g_pts_test_value_invalid[length] = '\0';
        if (length >= PTS_MAX_LENGTH)//for invalid attribute  length
        {
            if (need_rsp)
            {
                if (is_prep)
                {
                    bluetooth_gatts_send_response(conn_id, trans_id, 0x0d, attr_handle, NULL, 0, 0);
                }
                else
                {
                    g_pts_exe_write = 0;
                    bluetooth_gatts_send_response(conn_id, trans_id, 0x0d, attr_handle, NULL, 0, 0);
                }
            }
        }
        else
        {
            if (need_rsp)
            {
                if (is_prep)
                {
                    bluetooth_gatts_send_response(conn_id, trans_id, BT_STATUS_SUCCESS, attr_handle, g_pts_test_value_invalid, length, 0);
                }
                else
                {
                    g_pts_exe_write = 1;
                    bluetooth_gatts_send_response(conn_id, trans_id, BT_STATUS_SUCCESS, attr_handle, NULL, 0, 0);
                }
            }
        }
     }
    else  //for other case
    {
        memcpy(g_pts_test_value, value, length);
        g_pts_test_value[length] = '\0';
        if (need_rsp)
        {
            if (is_prep)
            {
                bluetooth_gatts_send_response(conn_id, trans_id, BT_STATUS_SUCCESS, attr_handle, g_pts_test_value, length, 0);
            }
            else
            {
                g_pts_exe_write = 1;
                bluetooth_gatts_send_response(conn_id, trans_id, BT_STATUS_SUCCESS, attr_handle, NULL, 0, 0);
            }
        }
    }
}

static VOID bluetooth_gatts_pts_test_req_write_cb(INT32 conn_id,
                                                             INT32 trans_id,
                                                             INT32 attr_handle,
                                                             INT32 offset,
                                                             INT32 length,
                                                             BOOL need_rsp,
                                                             BOOL is_prep,
                                                             UINT8* value)
{
    g_pts_write_offset = offset;
    if (g_pts_test_value == NULL)
    {
        g_pts_test_value = malloc((PTS_TEST_MTU + 10) * sizeof(CHAR));

        if (!g_pts_test_value)
        {
            BT_DBG_ERROR(BT_DEBUG_GATT, "[GATT] malloc pts_test_value fail!");
            return;
        }
    }
    if (offset == 0)
    {
        bluetooth_gatts_req_write_invalid_attr_length(conn_id, trans_id, attr_handle,
                                                      offset, length, need_rsp,
                                                      is_prep, value);
    }
        else if (offset < (PTS_TEST_MTU + 10))
        {
            if(offset + length > (PTS_TEST_MTU + 10))
            {
                g_pts_invalid_length = offset + length;
                memcpy(g_pts_test_value + offset, value, PTS_TEST_MTU + 10 - offset);
                g_pts_test_value[PTS_TEST_MTU + 9] = '\0';
            }
            else
            {
                memcpy(g_pts_test_value + offset, value, length);
                g_pts_test_value[offset + length] = '\0';
            }
            if (need_rsp && is_prep)
            {
                bluetooth_gatts_send_response(conn_id, trans_id, BT_STATUS_SUCCESS, attr_handle, g_pts_test_value + offset, length, 0);
            }
        }
        else
        {
            if (need_rsp && is_prep)
            {
                bluetooth_gatts_send_response(conn_id, trans_id, BT_STATUS_SUCCESS, attr_handle, (CHAR *)value, length, 0);
            }
            //invalid offset
            //bluetooth_gatts_send_response(conn_id, trans_id, 0x07, attr_handle, NULL, 0, 0);
        }
}
#endif

VOID bluetooth_gatts_request_read_callback(INT32 conn_id,
                                                       INT32 trans_id,
                                                       CHAR *bda,
                                                       INT32 attr_handle,
                                                       INT32 offset,
                                                       BOOL is_long)
{
    tBT_MW_GATTS_MSG gatts_msg = {0};
    gatts_msg.event = BTMW_GATTS_REQ_READ;
    gatts_msg.gatts_data.gatts_req_read.conn_id = conn_id;
    gatts_msg.gatts_data.gatts_req_read.trans_id = trans_id;
    gatts_msg.gatts_data.gatts_req_read.attr_handle = attr_handle;
    gatts_msg.gatts_data.gatts_req_read.offset = offset;
    gatts_msg.gatts_data.gatts_req_read.is_long = is_long;
    memcpy(gatts_msg.gatts_data.gatts_req_read.btaddr, bda, strlen(bda) + 1);

#if MTK_LINUX_GATTC_PTS_TEST
    bluetooth_gatts_pts_test_req_read_cb(conn_id, trans_id, attr_handle, offset);
#endif

    bt_mw_gatts_notify_app(&gatts_msg);
}


VOID bluetooth_gatts_request_write_callback(INT32 conn_id,
                                                        INT32 trans_id,
                                                        CHAR *bda,
                                                        INT32 attr_handle,
                                                        INT32 offset,
                                                        INT32 length,
                                                        BOOL need_rsp,
                                                        BOOL is_prep,
                                                        UINT8* value)
{
    tBT_MW_GATTS_MSG gatts_msg = {0};
    gatts_msg.event = BTMW_GATTS_REQ_WRITE;
    gatts_msg.gatts_data.gatts_req_write.conn_id = conn_id;
    gatts_msg.gatts_data.gatts_req_write.trans_id = trans_id;
    gatts_msg.gatts_data.gatts_req_write.attr_handle = attr_handle;
    gatts_msg.gatts_data.gatts_req_write.offset = offset;
    gatts_msg.gatts_data.gatts_req_write.length = length;
    gatts_msg.gatts_data.gatts_req_write.need_rsp = need_rsp;
    gatts_msg.gatts_data.gatts_req_write.is_prep = is_prep;
    memcpy(gatts_msg.gatts_data.gatts_req_write.btaddr, bda, strlen(bda) + 1);
    memset(gatts_msg.gatts_data.gatts_req_write.value, 0, BT_GATT_MAX_ATTR_LEN);
    BT_DBG_INFO(BT_DEBUG_GATT,"[GATT] length = %ld", (long)length);
    if ((0 != length) && (length <= BT_GATT_MAX_ATTR_LEN))
    {
        memcpy(gatts_msg.gatts_data.gatts_req_write.value, value, length);
#if MTK_LINUX_GATTC_PTS_TEST
    bluetooth_gatts_pts_test_req_write_cb(conn_id, trans_id, attr_handle, offset,
                                          length, need_rsp, is_prep, value);
#endif
    }

    bt_mw_gatts_notify_app(&gatts_msg);
}

VOID bluetooth_gatts_request_exec_write_callback(INT32 conn_id,
        INT32 trans_id,
        CHAR *bda,
        INT32 exec_write)
{
#if MTK_LINUX_GATTC_PTS_TEST
    g_pts_exe_write = exec_write;
    if ((g_pts_write_offset < (PTS_TEST_MTU + 10)) && (g_pts_invalid_length <= (PTS_TEST_MTU + 10)))
    {
         bluetooth_gatts_send_response(conn_id, trans_id, BT_STATUS_SUCCESS, conn_id, NULL, 0, 0);
    }
    else if (g_pts_invalid_length > (PTS_TEST_MTU + 10))
    {
        bluetooth_gatts_send_response(conn_id, trans_id, 0x0d, conn_id, NULL, 0, 0);
    }
    else
    {
        //invalid offset
        bluetooth_gatts_send_response(conn_id, trans_id, 0x07, conn_id, NULL, 0, 0);
    }
#endif
}

VOID bluetooth_gatts_indication_sent_callback(INT32 conn_id, INT32 status)
{
    tBT_MW_GATTS_MSG gatts_msg = {0};
    gatts_msg.event = BTMW_GATTS_SENT_IND;
    gatts_msg.gatts_data.gatts_sent_ind.conn_id = conn_id;
    gatts_msg.gatts_data.gatts_sent_ind.status = status;

    bt_mw_gatts_notify_app(&gatts_msg);
}

VOID bluetooth_gatts_mtu_changed_callback(INT32 conn_id, INT32 mtu)
{
#if MTK_LINUX_GATTC_PTS_TEST
    PTS_TEST_MTU = mtu;
    g_pts_test_value = malloc((PTS_TEST_MTU + 10) * sizeof(CHAR));

    if (!g_pts_test_value)
    {
        BT_DBG_ERROR(BT_DEBUG_GATT, "[GATT] malloc pts_test_value fail!");
        return;
    }
    else
    {
        memset(g_pts_test_value, 0x12, (PTS_TEST_MTU + 10) * sizeof(CHAR));
        g_pts_test_value[(PTS_TEST_MTU + 9)] = '\0';
    }
#endif
}

VOID bluetooth_gatts_service_started_callback(INT32 status,
                                                          INT32 server_if,
                                                          INT32 srvc_handle)
{
    tBT_MW_GATTS_MSG gatts_msg = {0};
    g_fg_gatts_start_service = TRUE;
    gatts_msg.event = BTMW_GATTS_START_SRVC;
    gatts_msg.gatts_data.gatts_srvc_info.server_if = server_if;
    gatts_msg.gatts_data.gatts_srvc_info.srvc_handle = srvc_handle;

    bt_mw_gatts_notify_app(&gatts_msg);
}

VOID bluetooth_gatts_service_stopped_callback(INT32 status,
                                                           INT32 server_if,
                                                           INT32 srvc_handle)
{
    tBT_MW_GATTS_MSG gatts_msg = {0};
    gatts_msg.event = BTMW_GATTS_STOP_SRVC;
    gatts_msg.gatts_data.gatts_srvc_info.server_if = server_if;
    gatts_msg.gatts_data.gatts_srvc_info.srvc_handle = srvc_handle;

    bt_mw_gatts_notify_app(&gatts_msg);
}

VOID bluetooth_gatts_service_deleted_callback(INT32 status,
                                                           INT32 server_if,
                                                           INT32 srvc_handle)
{
    tBT_MW_GATTS_MSG gatts_msg = {0};
    gatts_msg.event = BTMW_GATTS_DEL_SRVC;
    gatts_msg.gatts_data.gatts_srvc_info.server_if = server_if;
    gatts_msg.gatts_data.gatts_srvc_info.srvc_handle = srvc_handle;

    bt_mw_gatts_notify_app(&gatts_msg);
}
