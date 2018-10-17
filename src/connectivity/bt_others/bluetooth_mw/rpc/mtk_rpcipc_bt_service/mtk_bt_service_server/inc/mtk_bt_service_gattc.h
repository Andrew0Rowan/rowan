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


#ifndef _MTK_BT_SERVICE_GATTC_H_
#define _MTK_BT_SERVICE_GATTC_H_

#include "mtk_bt_service_gattc_wrapper.h"

INT32 x_mtkapi_bt_gattc_register_callback(MTKRPCAPI_BT_APP_GATTC_CB_FUNC_T *func,
                                                     void *pv_tag);
INT32 mtkrpcapi_bt_gattc_register_callback(MTKRPCAPI_BT_APP_GATTC_CB_FUNC_T *func,
                                                     void *pv_tag);
INT32 mtkrpcapi_btm_gattc_reg_cbk_fct(MTKRPCAPI_BT_APP_GATTC_CB_FUNC_T *func,
                                                 void *pv_tag);
INT32 x_mtkapi_bt_gattc_register_app(char * app_uuid);
INT32 x_mtkapi_bt_gattc_unregister_app(INT32 client_if);
INT32 x_mtkapi_bt_gattc_scan(BOOL start);
INT32 x_mtkapi_bt_gattc_unregister_callback();
INT32 x_mtkapi_bt_gattc_connect(INT32 client_if, CHAR *bt_addr,
                                         UINT8 is_direct, INT32 transport);
INT32 x_mtkapi_bt_gattc_disconnect(INT32 client_if, CHAR *bt_addr,
                                             INT32 conn_id);
INT32 x_mtkapi_bt_gattc_listen(INT32 client_if);
INT32 x_mtkapi_bt_gattc_refresh(INT32 client_if, CHAR *bt_addr);
INT32 x_mtkapi_bt_gattc_search_service(INT32 conn_id, CHAR *uuid);
INT32 x_mtkapi_bt_gattc_get_gatt_db(INT32 conn_id);
INT32 x_mtkapi_bt_gattc_read_char(INT32 conn_id, INT32 char_handle,
                                            INT32 auth_req);
INT32 x_mtkapi_bt_gattc_read_descr(INT32 conn_id, INT32 descr_handle,
                                             INT32 auth_req);
INT32 x_mtkapi_bt_gattc_write_char(INT32 conn_id, INT32 char_handle,
                                            INT32 write_type, INT32 len,
                                            INT32 auth_req, CHAR *value);
INT32 x_mtkapi_bt_gattc_write_descr(INT32 conn_id, INT32 descr_handle,
                                              INT32 write_type, INT32 len,
                                              INT32 auth_req, CHAR *value);
INT32 x_mtkapi_bt_gattc_execute_write(INT32 conn_id, INT32 execute);
INT32 x_mtkapi_bt_gattc_set_reg_noti(INT32 client_if, CHAR *bt_addr,
                                               INT32 char_handle, BOOL enable);
INT32 x_mtkapi_bt_gattc_read_rssi(INT32 client_if, CHAR *bt_addr);
INT32 x_mtkapi_bt_gattc_scan_filter_param_setup(BT_GATTC_FILT_PARAM_SETUP_T *scan_filt_param);
INT32 x_mtkapi_bt_gattc_scan_filter_enable(INT32 client_if, BOOL enable);
INT32 x_mtkapi_bt_gattc_scan_filter_add_remove(INT32 client_if,
                                                             INT32 action,
                                                             INT32 filt_type,
                                                             INT32 filt_index,
                                                             INT32 company_id,
                                                             INT32 company_id_mask,
                                                             const CHAR *p_uuid,
                                                             const CHAR *p_uuid_mask,
                                                             const CHAR *bd_addr,
                                                             CHAR addr_type,
                                                             INT32 data_len,
                                                             CHAR *p_data,
                                                             INT32 mask_len,
                                                             CHAR *p_mask);
INT32 x_mtkapi_bt_gattc_scan_filter_clear(INT32 client_if,
                                                    INT32 filt_index);
INT32 x_mtkapi_bt_gattc_get_device_type(CHAR *bd_addr);
INT32 x_mtkapi_bt_gattc_configure_mtu(INT32 conn_id, INT32 mtu);
INT32 x_mtkapi_bt_gattc_conn_parameter_update(CHAR *bt_addr,
                                                              INT32 min_interval,
                                                              INT32 max_interval,
                                                              INT32 latency,
                                                              INT32 timeout);
INT32 x_mtkapi_bt_gattc_set_scan_parameters(INT32 client_if,
                                                          INT32 scan_interval,
                                                          INT32 scan_window);
INT32 x_mtkapi_bt_gattc_multi_adv_enable(INT32 client_if,
                                                     INT32 min_interval,
                                                     INT32 max_interval,
                                                     INT32 adv_type,
                                                     INT32 chnl_map,
                                                     INT32 tx_power,
                                                     INT32 timeout_s);
INT32 x_mtkapi_bt_gattc_set_disc_mode(INT32 client_if, INT32 disc_mode);

INT32 x_mtkapi_bt_gattc_multi_adv_update(INT32 client_if,
                                                     INT32 min_interval,
                                                     INT32 max_interval,
                                                     INT32 adv_type,
                                                     INT32 chnl_map,
                                                     INT32 tx_power,
                                                     INT32 timeout_s);
INT32 x_mtkapi_bt_gattc_multi_adv_setdata(INT32 client_if,
                                                      UINT8 set_scan_rsp,
                                                      UINT8 include_name,
                                                      UINT8 include_txpower,
                                                      INT32 appearance,
                                                      INT32 manufacturer_len,
                                                      CHAR* manufacturer_data,
                                                      INT32 service_data_len,
                                                      CHAR* service_data,
                                                      INT32 service_uuid_len,
                                                      CHAR* service_uuid);
INT32 x_mtkapi_bt_gattc_multi_adv_disable(INT32 client_if);
INT32 x_mtkapi_bt_gattc_batchscan_cfg_storage(INT32 client_if,
                                                            INT32 batch_scan_full_max,
                                                            INT32 batch_scan_trunc_max,
                                                            INT32 batch_scan_notify_threshold);
INT32 x_mtkapi_bt_gattc_batchscan_enb_batch_scan(INT32 client_if,
                                                                  INT32 scan_mode,
                                                                  INT32 scan_interval,
                                                                  INT32 scan_window,
                                                                  INT32 addr_type,
                                                                  INT32 discard_rule);
INT32 x_mtkapi_bt_gattc_batchscan_dis_batch_scan(INT32 client_if);
INT32 x_mtkapi_bt_gattc_batchscan_read_reports(INT32 client_if,
                                                             INT32 scan_mode);

#if MTK_LINUX_GATTC_LE_NAME
INT32 x_mtkapi_bt_gattc_set_local_le_name(INT32 client_if, CHAR *le_name);
#endif
VOID x_mtkapi_bt_gattc_get_connect_result_info(BT_GATTC_CONNECT_RST_T *connect_rst_info);
VOID x_mtkapi_bt_gattc_get_disconnect_result_info(BT_GATTC_CONNECT_RST_T *disconnect_rst_info);
VOID x_mtkapi_bt_gattc_read_rssi_result_info(BT_GATTC_GET_REMOTE_RSSI_T *get_remote_rssi_info);
#endif
