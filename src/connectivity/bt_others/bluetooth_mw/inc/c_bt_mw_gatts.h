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


/* FILE NAME:  c_bt_mw_gatts.h
 * AUTHOR: Xuemei Yang
 * PURPOSE:
 *      It provides GATTS API to APP.
 * NOTES:
 */


#ifndef _C_BT_MW_GATTS_H_
#define _C_BT_MW_GATTS_H_

/* INCLUDE FILE DECLARATIONS
 */

#include "u_bt_mw_gatts.h"

/* NAMING CONSTANT DECLARATIONS
 */

/* MACRO FUNCTION DECLARATIONS
 */


/* DATA TYPE DECLARATIONS
 */

/******************** gatts ********************/
/**
 * FUNCTION NAME: c_btm_gatts_register_callback
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
extern INT32 c_btm_gatts_register_callback(BT_APP_GATTS_CB_FUNC_T *func);

/**
 * FUNCTION NAME: c_btm_gatts_register_server_sync
 * PURPOSE:
 *      The function is used to synchronous register gatt server APP.
 * INPUT:
 *      uuid               -- app uuid
 * OUTPUT:
 *      server_if          -- registered app identifer
 * RETURN:
 *      BT_SUCCESS                        -- Operate success.
 *      BT_ERR_STATUS_FAIL                -- Operate fail.
 *      BT_ERR_STATUS_PARM_INVALID        -- paramter is invalid.
 * NOTES:
 *      None
 */
extern INT32 c_btm_gatts_register_server_sync(CHAR *uuid, INT32 *server_if);

/**
 * FUNCTION NAME: c_btm_gatts_register_server
 * PURPOSE:
 *      The function is used to register gatt server APP.
 * INPUT:
 *      uuid               -- app uuid
 * OUTPUT:
 *      None
 * RETURN:
 *      BT_SUCCESS                        -- Operate success.
 *      BT_ERR_STATUS_FAIL                -- Operate fail.
 *      BT_ERR_STATUS_PARM_INVALID        -- paramter is invalid.
 * NOTES:
 *      None
 */
extern INT32 c_btm_gatts_register_server(CHAR *uuid);

/**
 * FUNCTION NAME: c_btm_gatts_unregister_server
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
extern INT32 c_btm_gatts_unregister_server(INT32 server_if);

/**
 * FUNCTION NAME: c_btm_gatts_connect
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
extern INT32 c_btm_gatts_connect(INT32 server_if, CHAR *bt_addr,
                                         UINT8 is_direct, INT32 transport);

/**
 * FUNCTION NAME: c_btm_gatts_disconnect
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
extern INT32 c_btm_gatts_disconnect(INT32 server_if, CHAR *bt_addr, INT32 conn_id);

/**
 * FUNCTION NAME: c_btm_gatts_add_service_sync
 * PURPOSE:
 *      The function is used for gatt server synchronous add service to database.
 * INPUT:
 *      server_if              -- registered app identifer
 *      service_uuid           -- service uuid
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
extern INT32 c_btm_gatts_add_service_sync(INT32 server_if, CHAR *service_uuid,
                                                     UINT8 is_primary, INT32 number,
                                                     INT32 *srvc_handle);

/**
 * FUNCTION NAME: c_btm_gatts_add_service
 * PURPOSE:
 *      The function is used for gatt server add service to database.
 * INPUT:
 *      server_if              -- registered app identifer
 *      service_uuid           -- service uuid
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
extern INT32 c_btm_gatts_add_service(INT32 server_if, CHAR *service_uuid,
                                              UINT8 is_primary, INT32 number);

/**
 * FUNCTION NAME: c_btm_gatts_add_included_service
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
extern INT32 c_btm_gatts_add_included_service(INT32 server_if,
                                                          INT32 service_handle,
                                                          INT32 included_handle);

/**
 * FUNCTION NAME: c_btm_gatts_add_char_sync
 * PURPOSE:
 *      The function is used for gatt server synchronous add characteristic to database.
 * INPUT:
 *      server_if              -- registered app identifer
 *      service_handle         -- added service start handle
 *      char_uuid              -- characteristic uuid
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
extern INT32 c_btm_gatts_add_char_sync(INT32 server_if, INT32 service_handle,
                                                  CHAR *char_uuid, INT32 properties,
                                                  INT32 permissions, INT32 *char_handle);

/**
 * FUNCTION NAME: c_btm_gatts_add_char
 * PURPOSE:
 *      The function is used for gatt server add characteristic to database.
 * INPUT:
 *      server_if              -- registered app identifer
 *      service_handle         -- added service start handle
 *      char_uuid              -- characteristic uuid
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
extern INT32 c_btm_gatts_add_char(INT32 server_if, INT32 service_handle,
                                          CHAR *uuid, INT32 properties, INT32 permissions);

/**
 * FUNCTION NAME: c_btm_gatts_add_desc
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
extern INT32 c_btm_gatts_add_desc(INT32 server_if, INT32 service_handle,
                                           CHAR *uuid, INT32 permissions);

/**
 * FUNCTION NAME: c_btm_gatts_start_service
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
extern INT32 c_btm_gatts_start_service(INT32 server_if, INT32 service_handle,
                                               INT32 transport);

/**
 * FUNCTION NAME: c_btm_gatts_start_service_sync
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
extern INT32 c_btm_gatts_start_service_sync(INT32 server_if, INT32 service_handle,
                                                       INT32 transport);

/**
 * FUNCTION NAME: c_btm_gatts_stop_service
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
extern INT32 c_btm_gatts_stop_service(INT32 server_if, INT32 service_handle);

/**
 * FUNCTION NAME: c_btm_gatts_delete_service
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
extern INT32 c_btm_gatts_delete_service(INT32 server_if, INT32 service_handle);

/**
 * FUNCTION NAME: c_btm_gatts_send_indication
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
extern INT32 c_btm_gatts_send_indication(INT32 server_if,
                                                  INT32 attribute_handle,
                                                  INT32 conn_id,
                                                  INT32 fg_confirm,
                                                  CHAR *p_value,
                                                  INT32 value_len);

/**
 * FUNCTION NAME: c_btm_gatts_send_response
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
extern INT32 c_btm_gatts_send_response(INT32 conn_id,
                                                  INT32 trans_id,
                                                  INT32 status,
                                                  INT32 handle,
                                                  CHAR *p_value,
                                                  INT32 value_len,
                                                  INT32 auth_req);

/**
 * FUNCTION NAME: c_btm_gatts_get_connect_result_info
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
extern VOID c_btm_gatts_get_connect_result_info(BT_GATTS_CONNECT_RST_T *connect_rst_info);

/**
 * FUNCTION NAME: c_btm_gatts_get_disconnect_result_info
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
extern VOID c_btm_gatts_get_disconnect_result_info(BT_GATTS_CONNECT_RST_T *disconnect_rst_info);
#endif /*  _C_BT_MW_GATTS_H_  */

