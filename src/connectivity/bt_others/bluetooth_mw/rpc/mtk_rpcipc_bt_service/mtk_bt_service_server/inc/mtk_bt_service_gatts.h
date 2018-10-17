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


#ifndef _MTK_BT_SERVICE_GATTS_H_
#define _MTK_BT_SERVICE_GATTS_H_

#include "mtk_bt_service_gatts_wrapper.h"

INT32 x_mtkapi_bt_gatts_register_callback(MTKRPCAPI_BT_APP_GATTS_CB_FUNC_T *func,
                                                    void *pv_tag);
INT32 x_mtkapi_bt_gatts_register_server(char * app_uuid);
INT32 x_mtkapi_bt_gatts_unregister_server(INT32 server_if);
INT32 x_mtkapi_bt_gatts_connect(INT32 server_if, CHAR *bt_addr,
                                         UINT8 is_direct, INT32 transport);
INT32 x_mtkapi_bt_gatts_disconnect(INT32 server_if, CHAR *bt_addr,
                                             INT32 conn_id);
INT32 x_mtkapi_bt_gatts_add_service(INT32 server_if, CHAR *service_uuid,
                                               UINT8 is_primary, INT32 number);
INT32 x_mtkapi_bt_gatts_add_included_service(INT32 server_if,
                                                           INT32 service_handle,
                                                           INT32 included_handle);
INT32 x_mtkapi_bt_gatts_add_char(INT32 server_if, INT32 service_handle,
                                          CHAR *uuid, INT32 properties,
                                          INT32 permissions);
INT32 x_mtkapi_bt_gatts_add_desc(INT32 server_if, INT32 service_handle,
                                           CHAR *uuid, INT32 permissions);
INT32 x_mtkapi_bt_gatts_start_service(INT32 server_if,
                                                INT32 service_handle,
                                                INT32 transport);
INT32 x_mtkapi_bt_gatts_stop_service(INT32 server_if, INT32 service_handle);
INT32 x_mtkapi_bt_gatts_delete_service(INT32 server_if, INT32 service_handle);
INT32 x_mtkapi_bt_gatts_send_indication(INT32 server_if, INT32 attribute_handle,
                                                   INT32 conn_id, INT32 fg_confirm,
                                                   CHAR *p_value, INT32 value_len);
INT32 x_mtkapi_bt_gatts_send_response(INT32 conn_id, INT32 trans_id,
                                                  INT32 status, INT32 handle,
                                                  CHAR *p_value, INT32 value_len,
                                                  INT32 auth_req);
VOID x_mtkapi_bt_gatts_get_connect_result_info(BT_GATTS_CONNECT_RST_T *connect_rst_info);
VOID x_mtkapi_bt_gatts_get_disconnect_result_info(BT_GATTS_CONNECT_RST_T *disconnect_rst_info);

#endif
