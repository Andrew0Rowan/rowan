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

/********************************************************************************************
 *     LEGAL DISCLAIMER
 *
 *     (Header of MediaTek Software/Firmware Release or Documentation)
 *
 *     BY OPENING OR USING THIS FILE, BUYER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 *     THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE") RECEIVED
 *     FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO BUYER ON AN "AS-IS" BASIS
 *     ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES, EXPRESS OR IMPLIED,
 *     INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR
 *     A PARTICULAR PURPOSE OR NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY
 *     WHATSOEVER WITH RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 *     INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND BUYER AGREES TO LOOK
 *     ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. MEDIATEK SHALL ALSO
 *     NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE RELEASES MADE TO BUYER'S SPECIFICATION
 *     OR TO CONFORM TO A PARTICULAR STANDARD OR OPEN FORUM.
 *
 *     BUYER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND CUMULATIVE LIABILITY WITH
 *     RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION,
 *     TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE
 *     FEES OR SERVICE charGE PAID BY BUYER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 *     THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE WITH THE LAWS
 *     OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF LAWS PRINCIPLES.
 ************************************************************************************************/

#ifndef _MTK_BT_SERVICE_GATTC_WRAPPER_H_
#define _MTK_BT_SERVICE_GATTC_WRAPPER_H_

#include "u_rpcipc_types.h"
#include "u_bt_mw_gattc.h"

#ifdef  __cplusplus
extern "C" {
#endif

typedef VOID (*mtkrpcapi_BtAppGATTCEventCbk)(BT_GATTC_EVENT_T bt_gatt_event, void* pv_tag);
typedef VOID (*mtkrpcapi_BtAppGATTCRegClientCbk)(BT_GATTC_REG_CLIENT_T *pt_reg_client_result, void* pv_tag);
typedef VOID (*mtkrpcapi_BtAppGATTCScanCbk)(BT_GATTC_SCAN_RST_T *pt_scan_result, void* pv_tag);
typedef VOID (*mtkrpcapi_BtAppGATTCGetGattDbCbk)(BT_GATTC_GET_GATT_DB_T *pt_get_gatt_db_result, void* pv_tag);
typedef VOID (*mtkrpcapi_BtAppGATTCGetRegNotiCbk)(BT_GATTC_GET_REG_NOTI_RST_T *pt_get_reg_noti_result, void* pv_tag);
typedef VOID (*mtkrpcapi_BtAppGATTCNotifyCbk)(BT_GATTC_GET_NOTIFY_T *pt_notify, void* pv_tag);
typedef VOID (*mtkrpcapi_BtAppGATTCReadCharCbk)(BT_GATTC_READ_CHAR_RST_T *pt_read_char, void* pv_tag);
typedef VOID (*mtkrpcapi_BtAppGATTCWriteCharCbk)(BT_GATTC_WRITE_CHAR_RST_T *pt_write_char, void* pv_tag);
typedef VOID (*mtkrpcapi_BtAppGATTCReadDescCbk)(BT_GATTC_READ_DESCR_RST_T *pt_read_desc, void* pv_tag);
typedef VOID (*mtkrpcapi_BtAppGATTCWriteDescCbk)(BT_GATTC_WRITE_DESCR_RST_T *pt_write_desc, void* pv_tag);
typedef VOID (*mtkrpcapi_BtAppGATTScanFilterParamCbk)(BT_GATTC_SCAN_FILTER_PARAM_T *pt_scan_filter_param, void* pv_tag);
typedef VOID (*mtkrpcapi_BtAppGATTScanFilterStatusCbk)(BT_GATTC_SCAN_FILTER_STATUS_T *pt_scan_filter_status, void* pv_tag);
typedef VOID (*mtkrpcapi_BtAppGATTScanFilterCfgCbk)(BT_GATTC_SCAN_FILTER_CFG_T *pt_scan_filter_cfg, void* pv_tag);
typedef VOID (*mtkrpcapi_BtAppGATTCAdvEnableCbk)(int status, void* pv_tag);

typedef struct
{
    mtkrpcapi_BtAppGATTCEventCbk bt_gattc_event_cb;
    mtkrpcapi_BtAppGATTCRegClientCbk bt_gattc_reg_client_cb;
    mtkrpcapi_BtAppGATTCScanCbk bt_gattc_scan_cb;
    mtkrpcapi_BtAppGATTCGetGattDbCbk bt_gattc_get_gatt_db_cb;
    mtkrpcapi_BtAppGATTCGetRegNotiCbk bt_gattc_get_reg_noti_cb;
    mtkrpcapi_BtAppGATTCNotifyCbk bt_gattc_notify_cb;
    mtkrpcapi_BtAppGATTCReadCharCbk bt_gattc_read_char_cb;
    mtkrpcapi_BtAppGATTCWriteCharCbk bt_gattc_write_char_cb;
    mtkrpcapi_BtAppGATTCReadDescCbk bt_gattc_read_desc_cb;
    mtkrpcapi_BtAppGATTCWriteDescCbk bt_gattc_write_desc_cb;
    mtkrpcapi_BtAppGATTScanFilterParamCbk bt_gattc_scan_filter_param_cb;
    mtkrpcapi_BtAppGATTScanFilterStatusCbk bt_gattc_scan_filter_status_cb;
    mtkrpcapi_BtAppGATTScanFilterCfgCbk bt_gattc_scan_filter_cfg_cb;
    mtkrpcapi_BtAppGATTCAdvEnableCbk bt_gattc_adv_enable_cb;
}MTKRPCAPI_BT_APP_GATTC_CB_FUNC_T;

extern INT32 a_mtkapi_bt_gattc_base_init(MTKRPCAPI_BT_APP_GATTC_CB_FUNC_T *func,
                                                 void* pv_tag);
extern INT32 a_mtkapi_bt_gattc_register_app(CHAR * app_uuid);
extern INT32 a_mtkapi_bt_gattc_unregister_app(INT32 client_if);
extern INT32 a_mtkapi_bt_gattc_scan();
extern INT32 a_mtkapi_bt_gattc_stop_scan();
extern INT32 a_mtkapi_bt_gattc_unregister_callback();
extern INT32 a_mtkapi_bt_gattc_open(INT32 client_if, CHAR *bt_addr,
                                            UINT8 is_direct, INT32 transport);
extern INT32 a_mtkapi_bt_gattc_close(INT32 client_if, CHAR *bt_addr,
                                             INT32 conn_id);
extern INT32 a_mtkapi_bt_gattc_listen(INT32 client_if);
extern INT32 a_mtkapi_bt_gattc_refresh(INT32 client_if, CHAR *bt_addr);
extern INT32 a_mtkapi_bt_gattc_search_service(INT32 conn_id, CHAR *uuid);
extern INT32 a_mtkapi_bt_gattc_get_gatt_db(INT32 conn_id);
extern INT32 a_mtkapi_bt_gattc_read_char(INT32 conn_id, INT32 char_handle,
                                                   INT32 auth_req);
extern INT32 a_mtkapi_bt_gattc_read_descr(INT32 conn_id, INT32 descr_handle,
                                                    INT32 auth_req);
extern INT32 a_mtkapi_bt_gattc_write_char(INT32 conn_id, INT32 char_handle,
                                                   INT32 write_type, INT32 len,
                                                   INT32 auth_req, CHAR *value);
extern INT32 a_mtkapi_bt_gattc_write_descr(INT32 conn_id, INT32 descr_handle,
                                                     INT32 write_type, INT32 len,
                                                     INT32 auth_req, CHAR *value);
extern INT32 a_mtkapi_bt_gattc_execute_write(INT32 conn_id, INT32 execute);
extern INT32 a_mtkapi_bt_gattc_reg_noti(INT32 client_if, CHAR *bt_addr,
                                                INT32 char_handle);
extern INT32 a_mtkapi_bt_gattc_dereg_noti(INT32 client_if, CHAR *bt_addr,
                                                    INT32 char_handle);
extern INT32 a_mtkapi_bt_gattc_read_rssi(INT32 client_if, CHAR *bt_addr);
extern INT32 a_mtkapi_bt_gattc_scan_filter_param_setup(BT_GATTC_FILT_PARAM_SETUP_T *scan_filt_param);
extern INT32 a_mtkapi_bt_gattc_scan_filter_enable(INT32 client_if);
extern INT32 a_mtkapi_bt_gattc_scan_filter_disable(INT32 client_if);
extern INT32 a_mtkapi_bt_gattc_scan_filter_add_remove(INT32 client_if,
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
extern INT32 a_mtkapi_bt_gattc_scan_filter_clear(INT32 client_if, INT32 filt_index);
extern INT32 a_mtkapi_bt_gattc_get_device_type(CHAR *bd_addr);

extern INT32 a_mtkapi_bt_gattc_configure_mtu(INT32 conn_id, INT32 mtu);
extern INT32 a_mtkapi_bt_gattc_conn_parameter_update(CHAR *bt_addr,
                                                                    INT32 min_interval,
                                                                    INT32 max_interval,
                                                                    INT32 latency,
                                                                    INT32 timeout);
extern INT32 a_mtkapi_bt_gattc_set_scan_parameters(INT32 client_if,
                                                                 INT32 scan_interval,
                                                                 INT32 scan_window);
extern INT32 a_mtkapi_bt_gattc_multi_adv_enable(INT32 client_if,
                                                            INT32 min_interval,
                                                            INT32 max_interval,
                                                            INT32 adv_type,
                                                            INT32 chnl_map,
                                                            INT32 tx_power,
                                                            INT32 timeout_s);
extern INT32 a_mtkapi_bt_gattc_set_disc_mode(INT32 client_if, INT32 disc_mode);

extern INT32 a_mtkapi_bt_gattc_multi_adv_update(INT32 client_if,
                                                            INT32 min_interval,
                                                            INT32 max_interval,
                                                            INT32 adv_type,
                                                            INT32 chnl_map,
                                                            INT32 tx_power,
                                                            INT32 timeout_s);
extern INT32 a_mtkapi_bt_gattc_multi_adv_setdata(INT32 client_if,
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
extern INT32 a_mtkapi_bt_gattc_multi_adv_disable(INT32 client_if);
extern INT32 a_mtkapi_bt_gattc_batchscan_cfg_storage(INT32 client_if,
                                                                   INT32 batch_scan_full_max,
                                                                   INT32 batch_scan_trunc_max,
                                                                   INT32 batch_scan_notify_threshold);
extern INT32 a_mtkapi_bt_gattc_batchscan_enb_batch_scan(INT32 client_if,
                                                                        INT32 scan_mode,
                                                                        INT32 scan_interval,
                                                                        INT32 scan_window,
                                                                        INT32 addr_type,
                                                                        INT32 discard_rule);
extern INT32 a_mtkapi_bt_gattc_batchscan_dis_batch_scan(INT32 client_if);
extern INT32 a_mtkapi_bt_gattc_batchscan_read_reports(INT32 client_if,
                                                                    INT32 scan_mode);
extern INT32 a_mtkapi_bt_gattc_set_local_le_name(INT32 client_if,
                                                             CHAR *le_name);
extern VOID a_mtkapi_bt_gattc_get_connect_result_info(BT_GATTC_CONNECT_RST_T *connect_rst_info);
extern VOID a_mtkapi_bt_gattc_get_disconnect_result_info(BT_GATTC_CONNECT_RST_T *disconnect_rst_info);
extern VOID a_mtkapi_bt_gattc_read_rssi_result_info(BT_GATTC_GET_REMOTE_RSSI_T *get_remote_rssi_info);
extern INT32 c_rpc_reg_mtk_bt_service_gattc_cb_hndlrs(VOID);

#ifdef  __cplusplus
}
#endif
#endif
