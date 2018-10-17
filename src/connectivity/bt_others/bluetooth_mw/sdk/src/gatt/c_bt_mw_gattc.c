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


/* FILE NAME:  c_bt_mw_gattc.c
 * AUTHOR: Xuemei Yang
 * PURPOSE:
 *      It provides GATTC API to APP.
 * NOTES:
 */

/*-----------------------------------------------------------------------------
                    include files
 ----------------------------------------------------------------------------*/
#include "c_bt_mw_gattc.h"
#include "bt_mw_gattc.h"
#include "bt_mw_log.h"

/******************** gattc ********************/
EXPORT_SYMBOL INT32 c_btm_gattc_register_callback(BT_APP_GATTC_CB_FUNC_T *func)
{
    return bluetooth_gattc_register_callback(func);
}

EXPORT_SYMBOL INT32 c_btm_bt_gattc_unregister_callback()
{
    BT_DBG_NORMAL(BT_DEBUG_GATT, "[GATTS] c_btm_bt_gattc_unregister_callback\n");
    return bluetooth_gattc_unregister_callback();
}

EXPORT_SYMBOL INT32 c_btm_gattc_register_app_sync(CHAR *uuid, INT32 *client_if)
{
    return bluetooth_gattc_register_app_sync(uuid, client_if);
}

EXPORT_SYMBOL INT32 c_btm_gattc_register_app(CHAR *uuid)
{
    return bluetooth_gattc_register_app(uuid);
}

EXPORT_SYMBOL INT32 c_btm_gattc_unregister_app(INT32 client_if)
{
    return bluetooth_gattc_unregister_app(client_if);
}

EXPORT_SYMBOL INT32 c_btm_gattc_scan(BOOL start)
{
    return bluetooth_gattc_scan(start);
}

EXPORT_SYMBOL INT32 c_btm_gattc_connect(INT32 client_if, CHAR *bt_addr,
                                                UINT8 is_direct, INT32 transport)
{
    return bluetooth_gattc_connect(client_if, bt_addr, is_direct, transport);
}

EXPORT_SYMBOL INT32 c_btm_gattc_disconnect(INT32 client_if, CHAR *bt_addr, INT32 conn_id)
{
    return bluetooth_gattc_disconnect(client_if, bt_addr, conn_id);
}

EXPORT_SYMBOL INT32 c_btm_gattc_listen(INT32 client_if)
{
    return bluetooth_gattc_listen(client_if);
}

EXPORT_SYMBOL INT32 c_btm_gattc_refresh(INT32 client_if, CHAR *bt_addr)
{
    return bluetooth_gattc_refresh(client_if, bt_addr);
}

EXPORT_SYMBOL INT32 c_btm_gattc_search_service(INT32 conn_id, CHAR *uuid)
{
    return bluetooth_gattc_search_service(conn_id, uuid);
}

EXPORT_SYMBOL INT32 c_btm_gattc_get_gatt_db(INT32 conn_id)
{
    return bluetooth_gattc_get_gatt_db(conn_id);
}

EXPORT_SYMBOL INT32 c_btm_gattc_read_char(INT32 conn_id, INT32 char_handle,
                                                  INT32 auth_req)
{
    return bluetooth_gattc_read_char(conn_id, char_handle, auth_req);
}

EXPORT_SYMBOL INT32 c_btm_gattc_read_descr(INT32 conn_id, INT32 descr_handle,
                                                    INT32 auth_req)
{
    return bluetooth_gattc_read_descr(conn_id, descr_handle, auth_req);
}

EXPORT_SYMBOL INT32 c_btm_gattc_write_char(INT32 conn_id, INT32 char_handle,
                                                   INT32 write_type, INT32 len,
                                                   INT32 auth_req, CHAR *value)
{
    return bluetooth_gattc_write_char(conn_id, char_handle,
                                      write_type, len, auth_req, value);
}

EXPORT_SYMBOL INT32 c_btm_gattc_execute_write(INT32 conn_id, INT32 execute)
{
    return bluetooth_gattc_execute_write(conn_id, execute);
}

EXPORT_SYMBOL INT32 c_btm_gattc_write_descr(INT32 conn_id, INT32 descr_handle,
                                                    INT32 write_type, INT32 len,
                                                    INT32 auth_req, CHAR *value)
{
    return bluetooth_gattc_write_descr(conn_id, descr_handle, write_type, len, auth_req, value);
}

EXPORT_SYMBOL INT32 c_btm_gattc_set_reg_noti(INT32 client_if, CHAR *bt_addr,
                                                     INT32 char_handle, BOOL enable)
{
    return bluetooth_gattc_set_reg_noti(client_if, bt_addr, char_handle, enable);
}

EXPORT_SYMBOL INT32 c_btm_gattc_read_rssi(INT32 client_if, CHAR *bt_addr)
{
    return bluetooth_gattc_read_rssi(client_if, bt_addr);
}

EXPORT_SYMBOL VOID c_btm_gattc_get_connect_result_info(BT_GATTC_CONNECT_RST_T *connect_rst_info)
{
    bluetooth_gattc_get_connect_result_info(connect_rst_info);
}

EXPORT_SYMBOL VOID c_btm_gattc_get_disconnect_result_info(BT_GATTC_CONNECT_RST_T *disconnect_rst_info)
{
    bluetooth_gattc_get_disconnect_result_info(disconnect_rst_info);
}

EXPORT_SYMBOL VOID c_btm_gattc_read_rssi_result_info(BT_GATTC_GET_REMOTE_RSSI_T *get_remote_rssi_info)
{
    bluetooth_gattc_read_rssi_result_info(get_remote_rssi_info);
}

EXPORT_SYMBOL INT32 c_btm_gattc_conn_parameter_update(CHAR *bt_addr,
                                                                    INT32 min_interval,
                                                                    INT32 max_interval,
                                                                    INT32 latency,
                                                                    INT32 timeout)
{
    return bluetooth_gattc_conn_parameter_update(bt_addr, min_interval, max_interval, latency, timeout);
}

EXPORT_SYMBOL INT32 c_btm_gattc_set_scan_parameters(INT32 client_if,
                                                          INT32 scan_interval,
                                                          INT32 scan_window)
{
    return bluetooth_gattc_set_scan_parameters(client_if, scan_interval, scan_window);
}

EXPORT_SYMBOL INT32 c_btm_gattc_multi_adv_enable(INT32 client_if, INT32 min_interval,
                                                            INT32 max_interval, INT32 adv_type,
                                                            INT32 chnl_map, INT32 tx_power,
                                                            INT32 timeout_s)
{
    return bluetooth_gattc_multi_adv_enable(client_if, min_interval, max_interval,
                                            adv_type, chnl_map, tx_power, timeout_s);
}

EXPORT_SYMBOL INT32 c_btm_gattc_multi_adv_enable_sync(INT32 client_if,
                                                                   INT32 min_interval,
                                                                   INT32 max_interval,
                                                                   INT32 adv_type,
                                                                   INT32 chnl_map,
                                                                   INT32 tx_power,
                                                                   INT32 timeout_s)
{
    return bluetooth_gattc_multi_adv_enable_sync(client_if, min_interval, max_interval,
            adv_type, chnl_map, tx_power, timeout_s);
}

EXPORT_SYMBOL INT32 c_btm_gattc_multi_adv_update(INT32 client_if, INT32 min_interval,
                                                     INT32 max_interval, INT32 adv_type,
                                                     INT32 chnl_map, INT32 tx_power,
                                                     INT32 timeout_s)
{
    return bluetooth_gattc_multi_adv_update(client_if, min_interval, max_interval,
                                            adv_type, chnl_map, tx_power, timeout_s);
}

EXPORT_SYMBOL INT32 c_btm_gattc_multi_adv_setdata(INT32 client_if,
                                                             UINT8 set_scan_rsp,
                                                             UINT8 include_name,
                                                             UINT8 include_txpower,
                                                             INT32 appearance,
                                                             INT32 manufacturer_len,
                                                             CHAR* manufacturer_data,
                                                             INT32 service_data_len,
                                                             CHAR* service_data,
                                                             INT32 service_uuid_len,
                                                             CHAR* service_uuid)
{
    return bluetooth_gattc_multi_adv_setdata(client_if, set_scan_rsp, include_name,
            include_txpower, appearance, manufacturer_len, manufacturer_data,
            service_data_len, service_data, service_uuid_len, service_uuid);
}

EXPORT_SYMBOL INT32 c_btm_gattc_multi_adv_setdata_sync(INT32 client_if,
                                                                    UINT8 set_scan_rsp,
                                                                    UINT8 include_name,
                                                                    UINT8 include_txpower,
                                                                    INT32 appearance,
                                                                    INT32 manufacturer_len,
                                                                    CHAR* manufacturer_data,
                                                                    INT32 service_data_len,
                                                                    CHAR* service_data,
                                                                    INT32 service_uuid_len,
                                                                    CHAR* service_uuid)
{
    return bluetooth_gattc_multi_adv_setdata_sync(client_if, set_scan_rsp, include_name,
            include_txpower, appearance, manufacturer_len, manufacturer_data,
            service_data_len, service_data, service_uuid_len, service_uuid);
}

EXPORT_SYMBOL INT32 c_btm_gattc_multi_adv_disable(INT32 client_if)
{
    return bluetooth_gattc_multi_adv_disable(client_if);
}

EXPORT_SYMBOL INT32 c_btm_gattc_batchscan_cfg_storage(INT32 client_if,
                                                                   INT32 batch_scan_full_max,
                                                                   INT32 batch_scan_trunc_max,
                                                                   INT32 batch_scan_notify_threshold)
{
    return bluetooth_gattc_batchscan_cfg_storage(client_if, batch_scan_full_max,
                                                 batch_scan_trunc_max, batch_scan_notify_threshold);
}

EXPORT_SYMBOL INT32 c_btm_gattc_batchscan_enb_batch_scan(INT32 client_if,
                                                                        INT32 scan_mode,
                                                                        INT32 scan_interval,
                                                                        INT32 scan_window,
                                                                        INT32 addr_type,
                                                                        INT32 discard_rule)
{
    return bluetooth_gattc_batchscan_enb_batch_scan(client_if, scan_mode,
                        scan_interval, scan_window, addr_type, discard_rule);
}

EXPORT_SYMBOL INT32 c_btm_gattc_batchscan_dis_batch_scan(INT32 client_if)
{
    return bluetooth_gattc_batchscan_dis_batch_scan(client_if);
}

EXPORT_SYMBOL INT32 c_btm_gattc_batchscan_read_reports(INT32 client_if,
                                                                    INT32 scan_mode)
{
    return bluetooth_gattc_batchscan_read_reports(client_if, scan_mode);
}

EXPORT_SYMBOL INT32 c_btm_gattc_scan_filter_param_setup(BT_GATTC_FILT_PARAM_SETUP_T scan_filt_param)
{
    return bluetooth_gattc_scan_filter_param_setup(scan_filt_param);
}

EXPORT_SYMBOL INT32 c_btm_gattc_scan_filter_enable(INT32 client_if, BOOL enable)
{
    return bluetooth_gattc_scan_filter_enable(client_if, enable);
}

EXPORT_SYMBOL INT32 c_btm_gattc_scan_filter_add_remove(INT32 client_if,
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
                                                                   CHAR *p_mask)
{
    return bluetooth_gattc_scan_filter_add_remove(client_if, action,
            filt_type, filt_index, company_id,
            company_id_mask, p_uuid,
            p_uuid_mask, bd_addr,
            addr_type, data_len, p_data, mask_len,
            p_mask);
}

EXPORT_SYMBOL INT32 c_btm_gattc_scan_filter_clear(INT32 client_if, INT32 filt_index)
{
    return bluetooth_gattc_scan_filter_clear(client_if, filt_index);
}

EXPORT_SYMBOL INT32 c_btm_gattc_get_device_type(CHAR *bt_addr)
{
    return bluetooth_gattc_get_device_type(bt_addr);
}

EXPORT_SYMBOL INT32 c_btm_gattc_set_local_le_name(INT32 client_if, CHAR *le_name)

{
#if MTK_LINUX_GATTC_LE_NAME //== TRUE
    return bluetooth_gattc_set_local_le_name(client_if, le_name);
#else
    return BT_SUCCESS;
#endif
}

EXPORT_SYMBOL INT32 c_btm_gattc_set_local_disc_mode(INT32 client_if, INT32 disc_mode)
{
    FUNC_ENTRY;
#if MTK_LINUX_GATTC_DISC_MODE
    return bluetooth_gattc_set_local_disc_mode(client_if, disc_mode);
#else
    return BT_SUCCESS;
#endif
}

EXPORT_SYMBOL INT32 c_btm_gattc_get_local_adv_rpa(INT32 client_if)
{
#if MTK_LINUX_GATTC_RPA
    return bluetooth_gattc_get_local_adv_rpa(client_if);
#else
    return BT_SUCCESS;
#endif
}

EXPORT_SYMBOL INT32 c_btm_gattc_configure_mtu(INT32 conn_id, INT32 mtu)
{
    return bluetooth_gattc_configure_mtu(conn_id, mtu);
}

/*for test mode interface*/
EXPORT_SYMBOL INT32 c_btm_gattc_test_command(INT32 command, BT_GATTC_TEST_PARAMS_T test_params)
{
    return bluetooth_gattc_test_command(command, test_params);
}

/* for pts test start */
EXPORT_SYMBOL INT32 c_btm_gattc_read_using_char_uuid(INT32 conn_id,
                                                                 INT32 start_handle,
                                                                 INT32 end_handle,
                                                                 CHAR *uuid,
                                                                 INT32 auth_req)
{
#if MTK_LINUX_GATTC_PTS_TEST
    return bluetooth_gattc_read_using_char_uuid(conn_id, start_handle, end_handle, uuid, auth_req);
#else
    return BT_SUCCESS;
#endif
}

EXPORT_SYMBOL INT32 c_btm_gattc_read_long_characteristic(INT32 conn_id,
                                                                    INT32 handle,
                                                                    INT32 offset,
                                                                    INT32 auth_req)
{
#if MTK_LINUX_GATTC_PTS_TEST
    return bluetooth_gattc_read_long_characteristic(conn_id, handle, offset, auth_req);
#else
    return 0;
#endif
}

EXPORT_SYMBOL INT32 c_btm_gattc_read_long_descr(INT32 conn_id,
                                                          INT32 handle,
                                                          INT32 offset,
                                                          INT32 auth_req)
{
#if MTK_LINUX_GATTC_PTS_TEST
    return bluetooth_gattc_read_long_descr(conn_id, handle, offset, auth_req);
#else
    return BT_SUCCESS;
#endif
}

EXPORT_SYMBOL INT32 c_btm_gattc_write_long_characteristic(INT32 conn_id,
                                                                     INT32 char_handle,
                                                                     INT32 write_type,
                                                                     INT32 len,
                                                                     INT32 offset,
                                                                     INT32 auth_req,
                                                                     CHAR *value)
{
#if MTK_LINUX_GATTC_PTS_TEST
    return bluetooth_gattc_write_long_characteristic(conn_id, char_handle, write_type,
        len, offset, auth_req, value);
#else
    return BT_SUCCESS;
#endif
}

EXPORT_SYMBOL INT32 c_btm_gattc_write_long_descr(INT32 conn_id,
                                                           INT32 descr_handle,
                                                           INT32 write_type,
                                                           INT32 len,
                                                           INT32 offset,
                                                           INT32 auth_req,
                                                           CHAR *value)
{
#if MTK_LINUX_GATTC_PTS_TEST
    return bluetooth_gattc_write_long_descr(conn_id, descr_handle, write_type,
        len, offset, auth_req, value);
#else
    return BT_SUCCESS;
#endif
}

EXPORT_SYMBOL INT32 c_btm_gattc_set_pts_flag(UINT8 pts_flag)
{
#if MTK_LINUX_GATTC_PTS_TEST
    return bluetooth_gattc_set_pts_flag(pts_flag);
#else
    return BT_SUCCESS;
#endif
}
EXPORT_SYMBOL INT32 c_btm_gattc_read_multi_characteristic(INT32 conn_id,
                                                 INT32 num_attr,
                                                 UINT16 *handles,
                                                 INT32 auth_req)
{
#if MTK_LINUX_GATTC_PTS_TEST
    return bluetooth_gattc_read_multi_characteristic(conn_id, num_attr, handles, auth_req);
#else
    return BT_SUCCESS;
#endif
}
/*for pts test end*/
