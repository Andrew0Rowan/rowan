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


/* FILE NAME:  bt_mw_gatts.h
 * AUTHOR: Xuemei Yang
 * PURPOSE:
 *      It provides GATTS API to c_bt_mw_gatts and other mw layer modules.
 * NOTES:
 */


#ifndef __BT_MW_GATTS_H__
#define __BT_MW_GATTS_H__

#include "u_bt_mw_gatts.h"


typedef enum
{
    /* GATTS API events */
    BTMW_GATTS_REG_SERVER = 0,
    BTMW_GATTS_ADD_SRVC,
    BTMW_GATTS_CONNECT,
    BTMW_GATTS_DISCONNECT,
    BTMW_GATTS_ADD_INCL,
    BTMW_GATTS_ADD_CHAR,
    BTMW_GATTS_ADD_DESC,
    BTMW_GATTS_REQ_READ,
    BTMW_GATTS_REQ_WRITE,
    BTMW_GATTS_SENT_IND,
    BTMW_GATTS_START_SRVC,
    BTMW_GATTS_STOP_SRVC,
    BTMW_GATTS_DEL_SRVC,
    BTMW_GATTS_CB_MAX_EVT,
} BTMW_GATTS_CB_EVENT;  // event

typedef struct
{
    BTMW_GATTS_CB_EVENT event;
    union
    {
        BT_GATTS_REG_SERVER_RST_T gatts_reg_server;
        BT_GATTS_ADD_SRVC_RST_T gatts_add_srvc;
        BT_GATTS_ADD_INCL_RST_T gatts_add_incl;
        BT_GATTS_ADD_CHAR_RST_T gatts_add_char;
        BT_GATTS_ADD_DESCR_RST_T gatts_add_desc;
        BT_GATTS_REQ_READ_RST_T gatts_req_read;
        BT_GATTS_REQ_WRITE_RST_T gatts_req_write;
        BT_GATTS_SENT_IND_RST_T gatts_sent_ind;
        BT_GATTS_SRVC_RST_T gatts_srvc_info;
    }gatts_data;
}tBT_MW_GATTS_MSG;

/**
 * FUNCTION NAME: bt_mw_gatts_nty_handle
 * PURPOSE:
 *      The function is used to handle notify  APP callback function.
 * INPUT:
 *      gatts_msg           -- gatt server message
 * OUTPUT:
 *      None
 * RETURN:
 *      None
 * NOTES:
 *      None
 */
VOID bt_mw_gatts_nty_handle(tBT_MW_GATTS_MSG *gatts_msg);

/*GATT server*/
/**
 * FUNCTION NAME: bluetooth_gatts_register_callback
 * PURPOSE:
 *      The function is used to gatt server register APP callback function.
 * INPUT:
 *      func               -- gatt server app callback function structure
 * OUTPUT:
 *      None
 * RETURN:
 *      BT_SUCCESS                    -- Operate success.
 *      BT_ERR_STATUS_PARM_INVALID    -- paramter invalid.
 * NOTES:
 *      None
 */
INT32 bluetooth_gatts_register_callback(BT_APP_GATTS_CB_FUNC_T *func);

/**
 * FUNCTION NAME: bluetooth_gatts_register_app_sync
 * PURPOSE:
 *      The function is used to synchronous register gatt server APP.
 * INPUT:
 *      pt_uuid            -- app uuid
 * OUTPUT:
 *      server_if          -- registered app identifer
 * RETURN:
 *      BT_SUCCESS                        -- Operate success.
 *      BT_ERR_STATUS_FAIL                -- Operate fail.
 *      BT_ERR_STATUS_PARM_INVALID        -- paramter is invalid.
 * NOTES:
 *      None
 */
INT32 bluetooth_gatts_register_app_sync(CHAR *pt_uuid, INT32 *server_if);

/**
 * FUNCTION NAME: bluetooth_gatts_register_server
 * PURPOSE:
 *      The function is used to register gatt server APP.
 * INPUT:
 *      pt_uuid            -- app uuid
 * OUTPUT:
 *      None
 * RETURN:
 *      BT_SUCCESS                        -- Operate success.
 *      BT_ERR_STATUS_FAIL                -- Operate fail.
 *      BT_ERR_STATUS_PARM_INVALID        -- paramter is invalid.
 * NOTES:
 *      None
 */
INT32 bluetooth_gatts_register_server(CHAR *pt_uuid);

/**
 * FUNCTION NAME: bluetooth_gatts_unregister_server
 * PURPOSE:
 *      The function is used to deregister gatt server APP.
 * INPUT:
 *      server_if              -- registered app identifer
 * OUTPUT:
 *      None
 * RETURN:
 *      BT_SUCCESS                        -- Operate success.
 *      BT_ERR_STATUS_FAIL                -- Operate fail.
 *      BT_ERR_STATUS_PARM_INVALID        -- paramter is invalid.
 * NOTES:
 *      None
 */
INT32 bluetooth_gatts_unregister_server(INT32 server_if);

/**
 * FUNCTION NAME: bluetooth_gatts_connect
 * PURPOSE:
 *      The function is used for gatt server connect with remote device.
 * INPUT:
 *      server_if              -- registered app identifer
 *      bt_addr                -- remote device bt address
 *      is_direct              -- is direct connection or background connection
 *      transport              -- transport type:(0 : auto, 1 : BREDR, 2 : LE)
 * OUTPUT:
 *      None
 * RETURN:
 *      BT_SUCCESS                        -- Operate success.
 *      BT_ERR_STATUS_FAIL                -- Operate fail.
 *      BT_ERR_STATUS_PARM_INVALID        -- paramter is invalid.
 * NOTES:
 *      None
 */
INT32 bluetooth_gatts_connect(INT32 server_if, CHAR *bt_addr,
                                      UINT8 is_direct, INT32 transport);

/**
 * FUNCTION NAME: bluetooth_gatts_disconnect
 * PURPOSE:
 *      The function is used for gatt server disconnect with remote device.
 * INPUT:
 *      server_if              -- registered app identifer
 *      bt_addr                -- remote device bt address
 *      conn_id                -- connection id
 * OUTPUT:
 *      None
 * RETURN:
 *      BT_SUCCESS                        -- Operate success.
 *      BT_ERR_STATUS_FAIL                -- Operate fail.
 *      BT_ERR_STATUS_PARM_INVALID        -- paramter is invalid.
 * NOTES:
 *      None
 */
INT32 bluetooth_gatts_disconnect(INT32 server_if, CHAR *bt_addr, INT32 conn_id);

/**
 * FUNCTION NAME: bluetooth_gatts_add_service_sync
 * PURPOSE:
 *      The function is used for gatt server synchronous add service to database.
 * INPUT:
 *      server_if              -- registered app identifer
 *      pt_service_uuid        -- service uuid
 *      is_primary             -- is primary service or not
 *      number                 -- handle number
 * OUTPUT:
 *      srvc_handle            -- added service start handle
 * RETURN:
 *      BT_SUCCESS                        -- Operate success.
 *      BT_ERR_STATUS_FAIL                -- Operate fail.
 *      BT_ERR_STATUS_PARM_INVALID        -- paramter is invalid.
 * NOTES:
 *      None
 */
INT32 bluetooth_gatts_add_service_sync(INT32 server_if, CHAR *pt_service_uuid,
                                                   UINT8 is_primary, INT32 number,
                                                   INT32 *srvc_handle);

/**
 * FUNCTION NAME: bluetooth_gatts_add_service
 * PURPOSE:
 *      The function is used for gatt server add service to database.
 * INPUT:
 *      server_if              -- registered app identifer
 *      pt_service_uuid        -- service uuid
 *      is_primary             -- is primary service or not
 *      number                 -- handle number
 * OUTPUT:
 *      None
 * RETURN:
 *      BT_SUCCESS                        -- Operate success.
 *      BT_ERR_STATUS_FAIL                -- Operate fail.
 *      BT_ERR_STATUS_PARM_INVALID        -- paramter is invalid.
 * NOTES:
 *      None
 */
INT32 bluetooth_gatts_add_service(INT32 server_if, CHAR *pt_service_uuid,
                                           UINT8 is_primary, INT32 number);

/**
 * FUNCTION NAME: bluetooth_gatts_add_included_service
 * PURPOSE:
 *      The function is used for gatt server add included service to database.
 * INPUT:
 *      server_if              -- registered app identifer
 *      service_handle         -- added service start handle
 *      included_handle        -- include service handle
 * OUTPUT:
 *      None
 * RETURN:
 *      BT_SUCCESS                        -- Operate success.
 *      BT_ERR_STATUS_FAIL                -- Operate fail.
 *      BT_ERR_STATUS_PARM_INVALID        -- paramter is invalid.
 * NOTES:
 *      None
 */
INT32 bluetooth_gatts_add_included_service(INT32 server_if,
                                                       INT32 service_handle,
                                                       INT32 included_handle);

/**
 * FUNCTION NAME: bluetooth_gatts_add_char_sync
 * PURPOSE:
 *      The function is used for gatt server synchronous add characteristic to database.
 * INPUT:
 *      server_if              -- registered app identifer
 *      service_handle         -- added service start handle
 *      uuid                   -- characteristic uuid
 *      properties             -- characteristic properties
 *      permissions            -- access characteristic permissions
 * OUTPUT:
 *      char_handle            -- added characteristic handle
 * RETURN:
 *      BT_SUCCESS                        -- Operate success.
 *      BT_ERR_STATUS_FAIL                -- Operate fail.
 *      BT_ERR_STATUS_PARM_INVALID        -- paramter is invalid.
 * NOTES:
 *      None
 */
INT32 bluetooth_gatts_add_char_sync(INT32 server_if,
                                              INT32 service_handle,
                                              CHAR *uuid,
                                              INT32 properties,
                                              INT32 permissions,
                                              INT32 *char_handle);

/**
 * FUNCTION NAME: bluetooth_gatts_add_char
 * PURPOSE:
 *      The function is used for gatt server add characteristic to database.
 * INPUT:
 *      server_if              -- registered app identifer
 *      service_handle         -- added service start handle
 *      uuid                   -- characteristic uuid
 *      properties             -- characteristic properties
 *      permissions            -- access characteristic permissions
 * OUTPUT:
 *      None
 * RETURN:
 *      BT_SUCCESS                        -- Operate success.
 *      BT_ERR_STATUS_FAIL                -- Operate fail.
 *      BT_ERR_STATUS_PARM_INVALID        -- paramter is invalid.
 * NOTES:
 *      None
 */
INT32 bluetooth_gatts_add_char(INT32 server_if, INT32 service_handle,
                                       CHAR *uuid, INT32 properties,
                                       INT32 permissions);

/**
 * FUNCTION NAME: bluetooth_gatts_add_desc
 * PURPOSE:
 *      The function is used for gatt server add char description to database.
 * INPUT:
 *      server_if              -- registered app identifer
 *      service_handle         -- added service start handle
 *      uuid                   -- char description uuid
 *      permissions            -- access char description permissions
 * OUTPUT:
 *      None
 * RETURN:
 *      BT_SUCCESS                        -- Operate success.
 *      BT_ERR_STATUS_FAIL                -- Operate fail.
 *      BT_ERR_STATUS_PARM_INVALID        -- paramter is invalid.
 * NOTES:
 *      None
 */
INT32 bluetooth_gatts_add_desc(INT32 server_if, INT32 service_handle,
                                        CHAR *uuid, INT32 permissions);

/**
 * FUNCTION NAME: bluetooth_gatts_start_service
 * PURPOSE:
 *      The function is used for gatt server start service.
 * INPUT:
 *      server_if              -- registered app identifer
 *      service_handle         -- added service start handle
 *      transport              -- transport type:(0 : auto, 1 : BREDR, 2 : LE)
 * OUTPUT:
 *      None
 * RETURN:
 *      BT_SUCCESS                        -- Operate success.
 *      BT_ERR_STATUS_FAIL                -- Operate fail.
 *      BT_ERR_STATUS_PARM_INVALID        -- paramter is invalid.
 * NOTES:
 *      None
 */
INT32 bluetooth_gatts_start_service(INT32 server_if, INT32 service_handle,
                                            INT32 transport);

/**
 * FUNCTION NAME: bluetooth_gatts_start_service_sync
 * PURPOSE:
 *      The function is used for gatt server start service.
 * INPUT:
 *      server_if              -- registered app identifer
 *      service_handle         -- added service start handle
 *      transport              -- transport type:(0 : auto, 1 : BREDR, 2 : LE)
 * OUTPUT:
 *      None
 * RETURN:
 *      BT_SUCCESS                        -- Operate success.
 *      BT_ERR_STATUS_FAIL                -- Operate fail.
 *      BT_ERR_STATUS_PARM_INVALID        -- paramter is invalid.
 * NOTES:
 *      None
 */
INT32 bluetooth_gatts_start_service_sync(INT32 server_if, INT32 service_handle,
                                                    INT32 transport);

/**
 * FUNCTION NAME: bluetooth_gatts_stop_service
 * PURPOSE:
 *      The function is used for gatt server stop service.
 * INPUT:
 *      server_if              -- registered app identifer
 *      service_handle         -- added service start handle
 * OUTPUT:
 *      None
 * RETURN:
 *      BT_SUCCESS                        -- Operate success.
 *      BT_ERR_STATUS_FAIL                -- Operate fail.
 *      BT_ERR_STATUS_PARM_INVALID        -- paramter is invalid.
 * NOTES:
 *      None
 */
INT32 bluetooth_gatts_stop_service(INT32 server_if, INT32 service_handle);

/**
 * FUNCTION NAME: bluetooth_gatts_delete_service
 * PURPOSE:
 *      The function is used for gatt server delete service.
 * INPUT:
 *      server_if              -- registered app identifer
 *      service_handle         -- added service start handle
 * OUTPUT:
 *      None
 * RETURN:
 *      BT_SUCCESS                        -- Operate success.
 *      BT_ERR_STATUS_FAIL                -- Operate fail.
 *      BT_ERR_STATUS_PARM_INVALID        -- paramter is invalid.
 * NOTES:
 *      None
 */
INT32 bluetooth_gatts_delete_service(INT32 server_if, INT32 service_handle);

/**
 * FUNCTION NAME: bluetooth_gatts_send_indication
 * PURPOSE:
 *      The function is used for gatt server send indication to remote device.
 * INPUT:
 *      server_if              -- registered app identifer
 *      attribute_handle       -- send indication attribute handle
 *      conn_id                -- connection id
 *      fg_confirm             -- is need confirmation or not
 *      p_value                -- send indication value
 *      value_len              -- send indication value length
 * OUTPUT:
 *      None
 * RETURN:
 *      BT_SUCCESS                        -- Operate success.
 *      BT_ERR_STATUS_FAIL                -- Operate fail.
 *      BT_ERR_STATUS_PARM_INVALID        -- paramter is invalid.
 * NOTES:
 *      None
 */
INT32 bluetooth_gatts_send_indication(INT32 server_if,
                                               INT32 attribute_handle,
                                               INT32 conn_id,
                                               INT32 fg_confirm,
                                               CHAR* p_value,
                                               INT32 value_len);

/**
 * FUNCTION NAME: bluetooth_gatts_send_response
 * PURPOSE:
 *      The function is used for gatt server send response to remote device.
 * INPUT:
 *      conn_id                -- connection id
 *      trans_id               -- transaction id
 *      status                 -- send response status
 *      handle                 -- send response handle
 *      p_value                -- send response value
 *      value_len              -- send response value length
 *      auth_req               -- authentication request
 * OUTPUT:
 *      None
 * RETURN:
 *      BT_SUCCESS                        -- Operate success.
 *      BT_ERR_STATUS_FAIL                -- Operate fail.
 *      BT_ERR_STATUS_PARM_INVALID        -- paramter is invalid.
 * NOTES:
 *      None
 */
INT32 bluetooth_gatts_send_response(INT32 conn_id,
                                               INT32 trans_id,
                                               INT32 status,
                                               INT32 handle,
                                               CHAR *p_value,
                                               INT32 value_len,
                                               INT32 auth_req);

/**
 * FUNCTION NAME: bluetooth_gatts_get_connect_result_info
 * PURPOSE:
 *      The function is used for gatt server to get connection result information.
 * INPUT:
 *      None
 * OUTPUT:
 *      connect_rst_info       -- connection result information
 * RETURN:
 *      None
 * NOTES:
 *      None
 */
VOID bluetooth_gatts_get_connect_result_info(BT_GATTS_CONNECT_RST_T *connect_rst_info);

/**
 * FUNCTION NAME: bluetooth_gatts_get_disconnect_result_info
 * PURPOSE:
 *      The function is used for gatt server to get disconnection result information.
 * INPUT:
 *      None
 * OUTPUT:
 *      disconnect_rst_info     -- disconnection result information
 * RETURN:
 *      None
 * NOTES:
 *      None
 */
VOID bluetooth_gatts_get_disconnect_result_info(BT_GATTS_CONNECT_RST_T *disconnect_rst_info);

/*********GATTS callback**************/
/**
 * FUNCTION NAME: bluetooth_gatts_register_server_callback
 * PURPOSE:
 *      The function is callback function of gatt server register app
 * INPUT:
 *      status              -- gatt error code status
 *      server_if           -- registered server app identifier
 *      app_uuid            -- registered server app uuid
 * OUTPUT:
 *      None
 * RETURN:
 *      None
 * NOTES:
 *      None
 */
VOID bluetooth_gatts_register_server_callback(INT32 status,
                                                          INT32 server_if,
                                                          CHAR *app_uuid);

/**
 * FUNCTION NAME: bluetooth_gatts_service_added_callback
 * PURPOSE:
 *      The function is callback function of gatt server add service
 * INPUT:
 *      status              -- gatt error code status
 *      server_if           -- registered server app identifier
 *      srvc_id              -- service information
 *      srvc_handle          -- added service handle
 * OUTPUT:
 *      None
 * RETURN:
 *      None
 * NOTES:
 *      None
 */
VOID bluetooth_gatts_service_added_callback(INT32 status,
                                                         INT32 server_if,
                                                         BT_GATT_SRVC_ID_T *srvc_id,
                                                         INT32 srvc_handle);

/**
 * FUNCTION NAME: bluetooth_gatts_included_service_added_callback
 * PURPOSE:
 *      The function is callback function of gatt server add included service
 * INPUT:
 *      status              -- gatt error code status
 *      server_if           -- registered server app identifier
 *      srvc_handle         -- added service handle
 *      incl_srvc_handle    -- added included service handle
 * OUTPUT:
 *      None
 * RETURN:
 *      None
 * NOTES:
 *      None
 */
VOID bluetooth_gatts_included_service_added_callback(INT32 status,
                                                                     INT32 server_if,
                                                                     INT32 srvc_handle,
                                                                     INT32 incl_srvc_handle);

/**
 * FUNCTION NAME: bluetooth_gatts_characteristic_added_callback
 * PURPOSE:
 *      The function is callback function of gatt server add characteristic
 * INPUT:
 *      status              -- gatt error code status
 *      server_if           -- registered server app identifier
 *      uuid                -- added characteristic uuid
 *      srvc_handle         -- added service handle
 *      char_handle         -- added characteristic handle
 * OUTPUT:
 *      None
 * RETURN:
 *      None
 * NOTES:
 *      None
 */
VOID bluetooth_gatts_characteristic_added_callback(INT32 status,
                                                                 INT32 server_if,
                                                                 CHAR *uuid,
                                                                 INT32 srvc_handle,
                                                                 INT32 char_handle);

/**
 * FUNCTION NAME: bluetooth_gatts_descriptor_added_callback
 * PURPOSE:
 *      The function is callback function of gatt server add characteristic descriptor
 * INPUT:
 *      status              -- gatt error code status
 *      server_if           -- registered server app identifier
 *      uuid                -- added characteristic descriptor uuid
 *      srvc_handle         -- added service handle
 *      descr_handle        -- added characteristic descriptor handle
 * OUTPUT:
 *      None
 * RETURN:
 *      None
 * NOTES:
 *      None
 */
VOID bluetooth_gatts_descriptor_added_callback(INT32 status,
                                                            INT32 server_if,
                                                            CHAR *uuid,
                                                            INT32 srvc_handle,
                                                            INT32 descr_handle);

/**
 * FUNCTION NAME: bluetooth_gatts_connection_callback
 * PURPOSE:
 *      The function is callback function of gatt server connection with remote device
 * INPUT:
 *      conn_id              -- connection id
 *      server_if            -- registered server app identifier
 *      connected            -- connected or not
 *      bda                  -- remote device bt address
 * OUTPUT:
 *      None
 * RETURN:
 *      None
 * NOTES:
 *      None
 */
VOID bluetooth_gatts_connection_callback(INT32 conn_id, INT32 server_if,
                                                    INT32 connected, CHAR *bda);


/**
 * FUNCTION NAME: bluetooth_gatts_request_read_callback
 * PURPOSE:
 *      The function is callback function of gatt server request read operation
 * INPUT:
 *      conn_id             -- connection id
 *      trans_id            -- transaction id
 *      bda                 -- remote device address
 *      attr_handle         -- request read attribute handle
 *      offset              -- read offset
 *      is_long             -- is long characteristic/descriptor or not
 * OUTPUT:
 *      None
 * RETURN:
 *      None
 * NOTES:
 *      None
 */
VOID bluetooth_gatts_request_read_callback(INT32 conn_id,
                                                       INT32 trans_id,
                                                       CHAR *bda,
                                                       INT32 attr_handle,
                                                       INT32 offset,
                                                       BOOL is_long);

/**
 * FUNCTION NAME: bluetooth_gatts_request_write_callback
 * PURPOSE:
 *      The function is callback function of gatt server request write operation
 * INPUT:
 *      conn_id             -- connection id
 *      trans_id            -- transaction id
 *      bda                 -- remote device address
 *      attr_handle         -- request read attribute handle
 *      offset              -- read offset
 *      length              -- the value length need to write
 *      need_rsp            -- gatt client need response or not
 *      is_prep             -- is prepare write or not
 *      value               -- the value need to write
 * OUTPUT:
 *      None
 * RETURN:
 *      None
 * NOTES:
 *      None
 */
VOID bluetooth_gatts_request_write_callback(INT32 conn_id,
                                                        INT32 trans_id,
                                                        CHAR *bda,
                                                        INT32 attr_handle,
                                                        INT32 offset,
                                                        INT32 length,
                                                        BOOL need_rsp,
                                                        BOOL is_prep,
                                                        UINT8* value);

/**
 * FUNCTION NAME: bluetooth_gatts_request_exec_write_callback
 * PURPOSE:
 *      The function is callback function of gatt server execute write operation
 * INPUT:
 *      conn_id             -- connection id
 *      trans_id            -- transaction id
 *      bda                 -- remote device address
 *      exec_write          -- execute write or not
 * OUTPUT:
 *      None
 * RETURN:
 *      None
 * NOTES:
 *      None
 */
VOID bluetooth_gatts_request_exec_write_callback(INT32 conn_id,
                                                               INT32 trans_id,
                                                               CHAR *bda,
                                                               INT32 exec_write);

/**
 * FUNCTION NAME: bluetooth_gatts_indication_sent_callback
 * PURPOSE:
 *      The function is callback function of gatt server send indication operation
 * INPUT:
 *      conn_id             -- connection id
 *      status              -- gatt error code status
 * OUTPUT:
 *      None
 * RETURN:
 *      None
 * NOTES:
 *      None
 */
VOID bluetooth_gatts_indication_sent_callback(INT32 conn_id, INT32 status);

/**
 * FUNCTION NAME: bluetooth_gatts_mtu_changed_callback
 * PURPOSE:
 *      The function is callback function of gatt server mtu changed operation
 * INPUT:
 *      conn_id             -- connection id
 *      mtu                 -- mtu
 * OUTPUT:
 *      None
 * RETURN:
 *      None
 * NOTES:
 *      None
 */
VOID bluetooth_gatts_mtu_changed_callback(INT32 conn_id, INT32 mtu);

/**
 * FUNCTION NAME: bluetooth_gatts_service_started_callback
 * PURPOSE:
 *      The function is callback function of gatt server start service operation
 * INPUT:
 *      status              -- gatt error code status
 *      server_if           -- registered server app identifier
 *      srvc_handle         -- start service handle
 * OUTPUT:
 *      None
 * RETURN:
 *      None
 * NOTES:
 *      None
 */
VOID bluetooth_gatts_service_started_callback(INT32 status,
                                                          INT32 server_if,
                                                          INT32 srvc_handle);

/**
 * FUNCTION NAME: bluetooth_gatts_service_stopped_callback
 * PURPOSE:
 *      The function is callback function of gatt server stop service operation
 * INPUT:
 *      status              -- gatt error code status
 *      server_if           -- registered server app identifier
 *      srvc_handle         -- start service handle
 * OUTPUT:
 *      None
 * RETURN:
 *      None
 * NOTES:
 *      None
 */
VOID bluetooth_gatts_service_stopped_callback(INT32 status,
                                                           INT32 server_if,
                                                           INT32 srvc_handle);

/**
 * FUNCTION NAME: bluetooth_gatts_service_deleted_callback
 * PURPOSE:
 *      The function is callback function of gatt server delete service operation
 * INPUT:
 *      status              -- gatt error code status
 *      server_if           -- registered server app identifier
 *      srvc_handle         -- start service handle
 * OUTPUT:
 *      None
 * RETURN:
 *      None
 * NOTES:
 *      None
 */
VOID bluetooth_gatts_service_deleted_callback(INT32 status,
                                                          INT32 server_if,
                                                          INT32 srvc_handle);


#endif/* __BT_MW_GATTS_H__ */
