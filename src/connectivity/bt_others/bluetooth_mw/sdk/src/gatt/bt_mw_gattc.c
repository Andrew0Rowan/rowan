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


/* FILE NAME:  bt_mw_gattc.c
 * AUTHOR: Xuemei Yang
 * PURPOSE:
 *      It provides GATTC andAPI to c_bt_mw_gattc and other mw layer modules.
 * NOTES:
 */

/*-----------------------------------------------------------------------------
                    include files
 ----------------------------------------------------------------------------*/
#include <stddef.h>
#include "linuxbt_gattc_if.h"
#include "bt_mw_gatt.h"
#include "bt_mw_log.h"

static BtAppGATTCEventCbk BtGattcAppCbk = NULL;
static BtAppGATTCRegClientCbk BtGattcRegClientCbk = NULL;
static BtAppGATTCScanCbk BtGattcScanCbk = NULL;
static BtAppGATTCGetGattDbCbk BtGattcGetGattDbCbk = NULL;
static BtAppGATTCGetRegNotiCbk BtGattcGetRegNotiCbk = NULL;
static BtAppGATTCNotifyCbk BtGattcNotifyCbk = NULL;
static BtAppGATTCReadCharCbk BtGattcReadCharCbk = NULL;
static BtAppGATTCWriteCharCbk BtGattcWriteCharCbk = NULL;
static BtAppGATTCReadDescCbk BtGattcReadDescCbk = NULL;
static BtAppGATTCWriteDescCbk BtGattcWriteDescCbk = NULL;
static BtAppGATTCScanFilterParamCbk BtGattcScanFilterParamCbk = NULL;
static BtAppGATTCScanFilterStatusCbk BtGattcScanFilterStatusCbk = NULL;
static BtAppGATTCScanFilterCfgCbk BtGattcScanFilterCfgCbk = NULL;
static BtAppGATTCAdvEnableCbk BtGattAdvEnableCbk = NULL;


static BT_GATTC_CONNECT_RST_T g_gattc_connect_rst_info = {0};
static BT_GATTC_CONNECT_RST_T g_gattc_disconnect_rst_info = {0};
static BT_GATTC_GET_REMOTE_RSSI_T g_gattc_remote_rssi_info = {0};

static INT32 g_client_if = 0;
static BOOL g_fg_gattc_reg_done = FALSE;
static BOOL g_fg_gattc_adv_enable = FALSE;
static BOOL g_fg_gattc_adv_data = FALSE;

static INT32 bluetooth_gattc_reg_cbk_fct(BT_APP_GATTC_CB_FUNC_T *func)
{
    BT_ERR_STATUS_T ret = BT_SUCCESS;
    BT_DBG_NORMAL(BT_DEBUG_GATT, "start bluetooth_gattc_reg_cbk_fct");

    if (NULL == func)
    {
        BT_DBG_ERROR(BT_DEBUG_GATT, "callback func is null!");
        return BT_ERR_STATUS_PARM_INVALID;
    }
    /*1*/
    if (func->bt_gattc_event_cb)
    {
        BtGattcAppCbk = func->bt_gattc_event_cb;
    }
    else
    {
        BT_DBG_NOTICE(BT_DEBUG_GATT,"gatt event callback func is null!");
    }
    /*2*/
    if (func->bt_gattc_reg_client_cb)
    {
        BtGattcRegClientCbk = func->bt_gattc_reg_client_cb;
    }
    else
    {
        BT_DBG_NOTICE(BT_DEBUG_GATT,"gatt register client callback func is null!");
    }

    /*3*/
    if (func->bt_gattc_scan_cb)
    {
        BtGattcScanCbk = func->bt_gattc_scan_cb;
    }
    else
    {
        BT_DBG_NOTICE(BT_DEBUG_GATT,"gatt scan callback func is null!");
    }
    /*4*/
    if (func->bt_gattc_get_reg_noti_cb)
    {
        BtGattcGetRegNotiCbk = func->bt_gattc_get_reg_noti_cb;
    }
    else
    {
        BT_DBG_NOTICE(BT_DEBUG_GATT,"gatt register notify callback func is null!");
    }
    /*5*/
    if (func->bt_gattc_notify_cb)
    {
        BtGattcNotifyCbk = func->bt_gattc_notify_cb;
    }
    else
    {
        BT_DBG_NOTICE(BT_DEBUG_GATT,"gatt notify callback func is null!");
    }
    /*6*/
    if (func->bt_gattc_read_char_cb)
    {
        BtGattcReadCharCbk = func->bt_gattc_read_char_cb;
    }
    else
    {
        BT_DBG_NOTICE(BT_DEBUG_GATT,"gatt read characteristic callback func is null!");
    }
    /*7*/
    if (func->bt_gattc_read_desc_cb)
    {
        BtGattcReadDescCbk = func->bt_gattc_read_desc_cb;
    }
    else
    {
        BT_DBG_NOTICE(BT_DEBUG_GATT,"gatt read descripor callback func is null!");
    }
    /*8*/
    if (func->bt_gattc_write_char_cb)
    {
        BtGattcWriteCharCbk = func->bt_gattc_write_char_cb;
    }
    else
    {
        BT_DBG_NOTICE(BT_DEBUG_GATT,"gatt write characteristic callback func is null!");
    }
    /*9*/
    if (func->bt_gattc_write_desc_cb)
    {
        BtGattcWriteDescCbk = func->bt_gattc_write_desc_cb;
    }
    else
    {
        BT_DBG_NOTICE(BT_DEBUG_GATT,"gatt write descripor callback func is null!");
    }
    /*10*/
    if (func->bt_gattc_scan_filter_param_cb)
    {
        BtGattcScanFilterParamCbk = func->bt_gattc_scan_filter_param_cb;
    }
    else
    {
        BT_DBG_NOTICE(BT_DEBUG_GATT,"gatt scan filter param callback func is null!");
    }

    /*11*/
    if (func->bt_gattc_scan_filter_status_cb)
    {
        BtGattcScanFilterStatusCbk = func->bt_gattc_scan_filter_status_cb;
    }
    else
    {
        BT_DBG_NOTICE(BT_DEBUG_GATT,"gatt scan filter status callback func is null!");
    }
    /*12*/
    if (func->bt_gattc_scan_filter_cfg_cb)
    {
        BtGattcScanFilterCfgCbk = func->bt_gattc_scan_filter_cfg_cb;
    }
    else
    {
        BT_DBG_NOTICE(BT_DEBUG_GATT,"gatt scan filter cfg callback func is null!");
    }
    /*13*/
    if (func->bt_gattc_get_gatt_db_cb)
    {
        BtGattcGetGattDbCbk = func->bt_gattc_get_gatt_db_cb;
    }
    else
    {
        BT_DBG_NOTICE(BT_DEBUG_GATT,"get gatt db callback func is null!");
    }
    /*14*/
    if (func->bt_gattc_adv_enable_cb)
    {
        BtGattAdvEnableCbk = func->bt_gattc_adv_enable_cb;
    }
    else
    {
        BT_DBG_NOTICE(BT_DEBUG_GATT,"gatt advertisment enable callback func is null!");
    }
    return ret;
}


INT32 bluetooth_gattc_register_callback(BT_APP_GATTC_CB_FUNC_T *func)
{
    BT_ERR_STATUS_T ret = BT_SUCCESS;
    g_fg_gattc_reg_done = FALSE;
    g_fg_gattc_adv_enable = FALSE;
    g_fg_gattc_adv_data = FALSE;
    ret = bluetooth_gattc_reg_cbk_fct(func);
    if (BT_SUCCESS != ret)
    {
        BT_DBG_ERROR(BT_DEBUG_GATT,"bluetooth_gattc_reg_cbk_fct failed");
        return ret;
    }
    ret = bluetooth_gatt_init();
    if (BT_SUCCESS != ret)
    {
        BT_DBG_ERROR(BT_DEBUG_GATT,"bluetooth_gatt_init failed");
    }
    return ret;
}

INT32 bluetooth_gattc_unregister_callback()
{
    BT_DBG_NORMAL(BT_DEBUG_GATT,"[GATT] bluetooth_gattc_unregister_callback\n");
    
    BtGattcAppCbk = NULL;
    BtGattcRegClientCbk = NULL;
    BtGattcScanCbk = NULL;
    BtGattcGetGattDbCbk = NULL;
    BtGattcGetRegNotiCbk = NULL;
    BtGattcNotifyCbk = NULL;
    BtGattcReadCharCbk = NULL;
    BtGattcWriteCharCbk = NULL;
    BtGattcReadDescCbk = NULL;
    BtGattcWriteDescCbk = NULL;
    BtGattcScanFilterParamCbk = NULL;
    BtGattcScanFilterStatusCbk = NULL;
    BtGattcScanFilterCfgCbk = NULL;
    BtGattAdvEnableCbk = NULL;
    return BT_SUCCESS;
}

INT32 bluetooth_gattc_register_app_sync(CHAR *pt_uuid, INT32 *client_if)
{
    INT32 i4_loop = 300;
    BT_ERR_STATUS_T ret = BT_SUCCESS;
    g_fg_gattc_reg_done = FALSE;
    ret = linuxbt_gattc_register_app(pt_uuid);
    while (0 < i4_loop && !g_fg_gattc_reg_done)
    {
        BT_DBG_INFO(BT_DEBUG_GATT, "Wait gattc register app finish:%ld", (long)i4_loop);
        usleep(10*1000);
        i4_loop--;
    }
    if (g_fg_gattc_reg_done)
    {
        *client_if = g_client_if;
    }
    else
    {
        BT_DBG_ERROR(BT_DEBUG_GATT,"gattc register app failed");
        *client_if = -1;
        ret = BT_ERR_STATUS_FAIL;
    }
    return ret;
}

INT32 bluetooth_gattc_register_app(CHAR *pt_uuid)
{
    return linuxbt_gattc_register_app(pt_uuid);
}

INT32 bluetooth_gattc_unregister_app(INT32 client_if)
{
    return linuxbt_gattc_unregister_app(client_if);
}

INT32 bluetooth_gattc_scan(BOOL start)
{
    return linuxbt_gattc_scan(start);
}

INT32 bluetooth_gattc_connect(INT32 client_if, CHAR *bt_addr,
                                      UINT8 is_direct, INT32 transport)
{
    return linuxbt_gattc_connect(client_if, bt_addr, is_direct, transport);
}

INT32 bluetooth_gattc_disconnect(INT32 client_if, CHAR *bt_addr,
                                          INT32 conn_id)
{
    return linuxbt_gattc_disconnect(client_if, bt_addr, conn_id);
}

INT32 bluetooth_gattc_listen(INT32 client_if)
{
    return linuxbt_gattc_listen(client_if);
}

INT32 bluetooth_gattc_refresh(INT32 client_if, CHAR *bt_addr)
{
    return linuxbt_gattc_refresh(client_if, bt_addr);
}


INT32 bluetooth_gattc_search_service(INT32 conn_id, CHAR *pt_uuid)
{
    return linuxbt_gattc_search_service(conn_id, pt_uuid);
}

INT32 bluetooth_gattc_get_gatt_db(INT32 conn_id)
{
    return linuxbt_gattc_get_gatt_db(conn_id);
}

INT32 bluetooth_gattc_read_char(INT32 conn_id,
                                        INT32 char_handle,
                                        INT32 auth_req)
{
    return linuxbt_gattc_read_char(conn_id, char_handle, auth_req);
}

INT32 bluetooth_gattc_read_descr(INT32 conn_id, INT32 descr_handle,
                                          INT32 auth_req)
{
    return linuxbt_gattc_read_descr(conn_id, descr_handle, auth_req);
}

INT32 bluetooth_gattc_write_char(INT32 conn_id, INT32 char_handle,
                                         INT32 write_type, INT32 len,
                                         INT32 auth_req, CHAR *value)
{
    return linuxbt_gattc_write_char(conn_id, char_handle, write_type, len,
                                    auth_req, value);
}

INT32 bluetooth_gattc_execute_write(INT32 conn_id, INT32 execute)
{
    return linuxbt_gattc_execute_write(conn_id, execute);
}

INT32 bluetooth_gattc_write_descr(INT32 conn_id, INT32 descr_handle,
                                          INT32 write_type, INT32 len,
                                          INT32 auth_req, CHAR *value)
{
    return linuxbt_gattc_write_descr(conn_id, descr_handle, write_type, len,
                                     auth_req, value);
}


INT32 bluetooth_gattc_set_reg_noti(INT32 client_if, CHAR *bt_addr,
                                            INT32 char_handle, BOOL enable)
{
    return linuxbt_gattc_set_reg_noti(client_if, bt_addr, char_handle, enable);
}

INT32 bluetooth_gattc_read_rssi(INT32 client_if, CHAR *bt_addr)
{
    return linuxbt_gattc_read_rssi(client_if, bt_addr);
}

VOID bluetooth_gattc_get_connect_result_info(BT_GATTC_CONNECT_RST_T *connect_rst_info)
{
    if (NULL == connect_rst_info)
    {
        BT_DBG_ERROR(BT_DEBUG_GATT,"[GATT] null pointer");
        return;
    }
    memcpy(connect_rst_info, &g_gattc_connect_rst_info, sizeof(BT_GATTC_CONNECT_RST_T));
}

VOID bluetooth_gattc_get_disconnect_result_info(BT_GATTC_CONNECT_RST_T *disconnect_rst_info)
{
    if (NULL == disconnect_rst_info)
    {
        BT_DBG_ERROR(BT_DEBUG_GATT,"[GATT] null pointer");
        return;
    }
    memcpy(disconnect_rst_info, &g_gattc_disconnect_rst_info, sizeof(BT_GATTC_CONNECT_RST_T));
}

VOID bluetooth_gattc_read_rssi_result_info(BT_GATTC_GET_REMOTE_RSSI_T *get_remote_rssi_info)
{
    if (NULL == get_remote_rssi_info)
    {
        BT_DBG_ERROR(BT_DEBUG_GATT,"[GATT] null pointer");
        return;
    }
    memcpy(get_remote_rssi_info, &g_gattc_remote_rssi_info, sizeof(BT_GATTC_GET_REMOTE_RSSI_T));
}

INT32 bluetooth_gattc_conn_parameter_update(CHAR *pbt_addr,
                                                          INT32 min_interval,
                                                          INT32 max_interval,
                                                          INT32 latency,
                                                          INT32 timeout)
{
    return linuxbt_gattc_conn_parameter_update(pbt_addr, min_interval,
                                               max_interval, latency, timeout);
}

INT32 bluetooth_gattc_set_scan_parameters(INT32 client_if,
                                                       INT32 scan_interval,
                                                       INT32 scan_window)
{
    return linuxbt_gattc_set_scan_parameters(client_if, scan_interval, scan_window);
}

INT32 bluetooth_gattc_multi_adv_enable(INT32 client_if,
                                                  INT32 min_interval,
                                                  INT32 max_interval,
                                                  INT32 adv_type,
                                                  INT32 chnl_map,
                                                  INT32 tx_power,
                                                  INT32 timeout_s)
{
    return linuxbt_gattc_multi_adv_enable(client_if, min_interval, max_interval,
                                          adv_type, chnl_map, tx_power, timeout_s);
}

INT32 bluetooth_gattc_multi_adv_enable_sync(INT32 client_if,
                                                         INT32 min_interval,
                                                         INT32 max_interval,
                                                         INT32 adv_type,
                                                         INT32 chnl_map,
                                                         INT32 tx_power,
                                                         INT32 timeout_s)
{
    INT32 i4_loop = 300;
    BT_ERR_STATUS_T ret = BT_SUCCESS;
    g_fg_gattc_adv_enable = FALSE;
    ret = linuxbt_gattc_multi_adv_enable(client_if, min_interval, max_interval,
                                         adv_type, chnl_map, tx_power, timeout_s);
    while (0 < i4_loop && !g_fg_gattc_adv_enable)
    {
        BT_DBG_INFO(BT_DEBUG_GATT, "Wait gattc adv enable finish:%ld", (long)i4_loop);
        usleep(10*1000);
        i4_loop--;
    }
    if (g_fg_gattc_adv_enable)
    {
        return ret;
    }
    else
    {
        BT_DBG_ERROR(BT_DEBUG_GATT, "gattc adb enable failed");
        return BT_ERR_STATUS_FAIL;
    }
}

INT32 bluetooth_gattc_multi_adv_update(INT32 client_if,
                                                  INT32 min_interval,
                                                  INT32 max_interval,
                                                  INT32 adv_type,
                                                  INT32 chnl_map,
                                                  INT32 tx_power,
                                                  INT32 timeout_s)
{
    return linuxbt_gattc_multi_adv_update(client_if, min_interval, max_interval,
                                          adv_type, chnl_map, tx_power, timeout_s);
}

INT32 bluetooth_gattc_multi_adv_setdata(INT32 client_if,
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
    return linuxbt_gattc_multi_adv_setdata(client_if, set_scan_rsp,
                                           include_name, include_txpower,
                                           appearance, manufacturer_len,
                                           manufacturer_data, service_data_len,
                                           service_data, service_uuid_len,
                                           service_uuid);

}

INT32 bluetooth_gattc_multi_adv_setdata_sync(INT32 client_if,
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
    INT32 i4_loop = 300;
    BT_ERR_STATUS_T ret = BT_SUCCESS;
    g_fg_gattc_adv_data = FALSE;
    ret = linuxbt_gattc_multi_adv_setdata(client_if, set_scan_rsp, include_name,
                                          include_txpower, appearance,
                                          manufacturer_len, manufacturer_data,
                                          service_data_len, service_data,
                                          service_uuid_len, service_uuid);
    while (0<i4_loop && !g_fg_gattc_adv_data)
    {
        BT_DBG_INFO(BT_DEBUG_GATT, "Wait gattc adv data finish:%ld", (long)i4_loop);
        usleep(10*1000);
        i4_loop--;
    }
    if (g_fg_gattc_adv_data)
    {
        return ret;
    }
    else
    {
        BT_DBG_ERROR(BT_DEBUG_GATT, "gattc adb data failed");
        return BT_ERR_STATUS_FAIL;
    }
}

INT32 bluetooth_gattc_multi_adv_disable(INT32 client_if)
{
    return linuxbt_gattc_multi_adv_disable(client_if);
}

INT32 bluetooth_gattc_batchscan_cfg_storage(INT32 client_if,
                                                        INT32 batch_scan_full_max,
                                                        INT32 batch_scan_trunc_max,
                                                        INT32 batch_scan_notify_threshold)
{
    return linuxbt_gattc_batchscan_cfg_storage(client_if, batch_scan_full_max,
                                               batch_scan_trunc_max,
                                               batch_scan_notify_threshold);
}

INT32 bluetooth_gattc_batchscan_enb_batch_scan(INT32 client_if,
                                                              INT32 scan_mode,
                                                              INT32 scan_interval,
                                                              INT32 scan_window,
                                                              INT32 addr_type,
                                                              INT32 discard_rule)
{
    return linuxbt_gattc_batchscan_enb_batch_scan(client_if, scan_mode,
                                                  scan_interval, scan_window,
                                                  addr_type, discard_rule);
}

INT32 bluetooth_gattc_batchscan_dis_batch_scan(INT32 client_if)
{
    return linuxbt_gattc_batchscan_dis_batch_scan(client_if);
}


INT32 bluetooth_gattc_batchscan_read_reports(INT32 client_if,
                                                          INT32 scan_mode)
{
    return linuxbt_gattc_batchscan_read_reports(client_if, scan_mode);
}

INT32 bluetooth_gattc_scan_filter_param_setup(BT_GATTC_FILT_PARAM_SETUP_T scan_filt_param)
{
    return linuxbt_gattc_scan_filter_param_setup(scan_filt_param);
}

INT32 bluetooth_gattc_scan_filter_enable(INT32 client_if, BOOL enable)
{
    return linuxbt_gattc_scan_filter_enable(client_if, enable);
}

INT32 bluetooth_gattc_scan_filter_add_remove(INT32 client_if,
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
                                                          CHAR* p_data,
                                                          INT32 mask_len,
                                                          CHAR* p_mask)
{
    return linuxbt_gattc_scan_filter_add_remove(client_if, action, filt_type,
                                                filt_index, company_id, company_id_mask,
                                                p_uuid, p_uuid_mask, bd_addr,
                                                addr_type, data_len, p_data,
                                                mask_len, p_mask);
}
INT32 bluetooth_gattc_scan_filter_clear(INT32 client_if, INT32 filt_index)
{
    return linuxbt_gattc_scan_filter_clear(client_if,filt_index);
}

INT32 bluetooth_gattc_get_device_type(CHAR *bt_addr)
{
    return linuxbt_gattc_get_device_type(bt_addr);
}

INT32 bluetooth_gattc_configure_mtu(INT32 conn_id, INT32 mtu)
{
    return linuxbt_gattc_configure_mtu(conn_id, mtu);
}

#if MTK_LINUX_GATTC_LE_NAME
INT32 bluetooth_gattc_set_local_le_name(INT32 client_if, CHAR *name)
{
    return linuxbt_gattc_set_local_le_name(client_if, name);
}
#endif

#if MTK_LINUX_GATTC_RPA
INT32 bluetooth_gattc_get_local_adv_rpa(INT32 client_if)
{
    BT_DBG_NORMAL(BT_DEBUG_GATT, "<<< call bt_getlocal_adv_rpa >>>");
    return linuxbt_gattc_get_local_adv_rpa(client_if);
}

#endif

#if MTK_LINUX_GATTC_DISC_MODE
INT32 bluetooth_gattc_set_local_disc_mode(INT32 client_if, INT32 disc_mode)
{
    BT_DBG_NORMAL(BT_DEBUG_GATT, "bluetooth_gattc_set_local_disc_mode client_if= %d,disc_mode= %d", client_if, disc_mode);
    return linuxbt_gattc_set_local_disc_mode(client_if, disc_mode);
}
#endif

#if MTK_LINUX_GATTC_PTS_TEST
INT32 bluetooth_gattc_read_using_char_uuid(INT32 conn_id,
                                                 INT32 start_handle,
                                                 INT32 end_handle,
                                                 CHAR *uuid,
                                                 INT32 auth_req)
{
    BT_DBG_NORMAL(BT_DEBUG_GATT, "<<< call bt_read_using_char_uuid >>>");
    return linuxbt_gattc_read_using_char_uuid(conn_id, start_handle, end_handle, uuid, auth_req);
}

INT32 bluetooth_gattc_read_long_characteristic(INT32 conn_id,
                                                 INT32 handle,
                                                 INT32 offset,
                                                 INT32 auth_req)
{
    BT_DBG_NORMAL(BT_DEBUG_GATT, "<<< call bt_read_long_characteristic >>>");
    return linuxbt_gattc_read_long_characteristic(conn_id, handle, offset, auth_req);
}

INT32 bluetooth_gattc_read_multi_characteristic(INT32 conn_id,
                                                 INT32 num_attr,
                                                 UINT16 *handles,
                                                 INT32 auth_req)
{
    BT_DBG_NORMAL(BT_DEBUG_GATT, "<<< call bt_read_multi_characteristic >>>");
    return linuxbt_gattc_read_multi_characteristic(conn_id, num_attr, handles, auth_req);
}

INT32 bluetooth_gattc_read_long_descr(INT32 conn_id,
                                                 INT32 handle,
                                                 INT32 offset,
                                                 INT32 auth_req)
{
    BT_DBG_NORMAL(BT_DEBUG_GATT, "<<< call bt_read_long_characteristic >>>");
    return linuxbt_gattc_read_long_descr(conn_id, handle, offset, auth_req);
}

INT32 bluetooth_gattc_write_long_characteristic(INT32 conn_id,
                                                         INT32 char_handle,
                                                         INT32 write_type,
                                                         INT32 len,
                                                         INT32 offset,
                                                         INT32 auth_req,
                                                         CHAR *value)
{
    BT_DBG_NORMAL(BT_DEBUG_GATT, "<<< call bt_write_long_characteristic >>>");
    return linuxbt_gattc_write_long_characteristic(conn_id, char_handle, write_type,
        len, offset, auth_req, value);
}

INT32 bluetooth_gattc_write_long_descr(INT32 conn_id,
                                                     INT32 descr_handle,
                                                     INT32 write_type,
                                                     INT32 len,
                                                     INT32 offset,
                                                     INT32 auth_req,
                                                     CHAR *value)
{
    BT_DBG_NORMAL(BT_DEBUG_GATT, "<<< call bt_write_long_char_descr >>>");
    return linuxbt_gattc_write_long_descr(conn_id, descr_handle, write_type,
        len, offset, auth_req, value);
}

INT32 bluetooth_gattc_set_pts_flag(UINT8 pts_flag)
{
    BT_DBG_NORMAL(BT_DEBUG_GATT, "<<< call bt_set_pts_flag >>>");
    return linuxbt_gattc_set_pts_flag(pts_flag);
}
#endif

INT32 bluetooth_gattc_test_command(INT32 command, BT_GATTC_TEST_PARAMS_T test_params)
{
    BT_DBG_NORMAL(BT_DEBUG_GATT, "<<< call gattc test command >>>");
    return linuxbt_gattc_test_command(command, test_params);
}

/************gattc  callback *************/

static VOID bt_mw_gattc_notify_app(tBT_MW_GATTC_MSG *gattc_msg)
{
    tBTMW_GATT_MSG msg;
    msg.hdr.event = BTMW_GATTC_EVENT;
    msg.hdr.len = sizeof(tBT_MW_GATTC_MSG);
    memcpy((void*)&msg.data.gattc_msg, gattc_msg, sizeof(tBT_MW_GATTC_MSG));
    bt_mw_gatt_nty_send_msg(&msg);
}

VOID bt_mw_gattc_nty_handle(tBT_MW_GATTC_MSG *gattc_msg)
{
    BT_DBG_INFO(BT_DEBUG_GATT, "bt_mw_gatt_nty_handle: event:%d", gattc_msg->event);

    switch(gattc_msg->event)
    {
        case BTMW_GATTC_REG_CLIENT:
            if (BtGattcRegClientCbk)
            {
                BT_DBG_INFO(BT_DEBUG_GATT,"[GATT] client_if = %d",
                            gattc_msg->gattc_data.gattc_reg_client.client_if);
                BtGattcRegClientCbk(&gattc_msg->gattc_data.gattc_reg_client);
            }
            break;

        case BTMW_GATTC_SCAN_RST:
            if (BtGattcScanCbk)
            {
                BtGattcScanCbk(&gattc_msg->gattc_data.gattc_scan_rst);
            }
            break;
        case BTMW_GATTC_CONNECT:
            if (BtGattcAppCbk)
            {
                BtGattcAppCbk(BT_GATTC_CONNECT);
            }
            break;
        case BTMW_GATTC_DISCONNECT:
            if (BtGattcAppCbk)
            {
                BtGattcAppCbk(BT_GATTC_DISCONNECT);
            }
            break;
        case BTMW_GATTC_GATT_DB:
            if (BtGattcGetGattDbCbk)
            {
                BtGattcGetGattDbCbk(&gattc_msg->gattc_data.gattc_gatt_db);
                free(gattc_msg->gattc_data.gattc_gatt_db.gatt_db_element);

            }
            break;
        case BTMW_GATTC_REG_NTY:
            if (BtGattcGetRegNotiCbk)
            {
                BtGattcGetRegNotiCbk(&gattc_msg->gattc_data.gattc_reg_noti_rst);
            }
            break;
        case BTMW_GATTC_NOTIFY:
            if (BtGattcNotifyCbk)
            {
                BtGattcNotifyCbk(&gattc_msg->gattc_data.gattc_notify);
            }
            break;
        case BTMW_GATTC_READ_CHAR:
            if (BtGattcReadCharCbk)
            {
                BtGattcReadCharCbk(&gattc_msg->gattc_data.gattc_read_char_rst);
            }
            break;
        case BTMW_GATTC_WRITE_CHAR:
            if (BtGattcWriteCharCbk)
            {
                BtGattcWriteCharCbk(&gattc_msg->gattc_data.gattc_write_char_rst);
            }
            break;
        case BTMW_GATTC_READ_DESC:
            if (BtGattcReadDescCbk)
            {
                BtGattcReadDescCbk(&gattc_msg->gattc_data.gattc_read_desc_rst);
            }
            break;
        case BTMW_GATTC_WRITE_DESC:
            if (BtGattcWriteDescCbk)
            {
                BtGattcWriteDescCbk(&gattc_msg->gattc_data.gattc_write_desc_rst);
            }
            break;
        case BTMW_GATTC_REM_RSSI:
            if (BtGattcAppCbk)
            {
                BtGattcAppCbk(BT_GATTC_GET_RSSI_DONE);
            }
            break;
        case BTMW_GATTC_SCAN_FILT_CFG:
            if (BtGattcScanFilterCfgCbk) //need app to implement
            {
                BtGattcScanFilterCfgCbk(&gattc_msg->gattc_data.gattc_scan_filter_cfg);
            }
            break;
        case BTMW_GATTC_SCAN_FILT_PARAM:
            if (BtGattcScanFilterParamCbk) //need app to implement
            {
                BtGattcScanFilterParamCbk(&gattc_msg->gattc_data.gattc_scan_filter_param);
            }
            break;
        case BTMW_GATTC_SCAN_FILT_STATUS:
            if (BtGattcScanFilterStatusCbk) //need app to implement
            {
                BtGattcScanFilterStatusCbk(&gattc_msg->gattc_data.gattc_scan_filter_status);
            }
            break;
        default:
            break;
    }
}

VOID bluetooth_gattc_register_client_cbk(INT32 status, INT32 client_if,
                                                   CHAR *app_uuid)
{
    tBT_MW_GATTC_MSG gattc_msg = {0};
    g_fg_gattc_reg_done = TRUE;
    g_client_if = client_if;
    gattc_msg.event = BTMW_GATTC_REG_CLIENT;
    gattc_msg.gattc_data.gattc_reg_client.client_if = client_if;
    memcpy(gattc_msg.gattc_data.gattc_reg_client.uuid, app_uuid, strlen(app_uuid)+1);
    BT_DBG_INFO(BT_DEBUG_GATT, "client_if is: %d", gattc_msg.gattc_data.gattc_reg_client.client_if);
    BT_DBG_INFO(BT_DEBUG_GATT, "uuid is: %s", gattc_msg.gattc_data.gattc_reg_client.uuid);

    bt_mw_gattc_notify_app(&gattc_msg);
}

VOID bluetooth_gattc_scan_result_cbk(CHAR* bda, INT32 rssi, UINT8* adv_data)
{
    tBT_MW_GATTC_MSG gattc_msg = {0};
    gattc_msg.event = BTMW_GATTC_SCAN_RST;
    memcpy(gattc_msg.gattc_data.gattc_scan_rst.btaddr, bda, strlen(bda)+ 1);
    gattc_msg.gattc_data.gattc_scan_rst.rssi = rssi;
    memcpy(gattc_msg.gattc_data.gattc_scan_rst.adv_data, adv_data, BT_GATT_MAX_ATTR_LEN);
    BT_DBG_INFO(BT_DEBUG_GATT, "rssi is: %d", gattc_msg.gattc_data.gattc_scan_rst.rssi);
    BT_DBG_INFO(BT_DEBUG_GATT, "btaddr is: %s", gattc_msg.gattc_data.gattc_scan_rst.btaddr);

    bt_mw_gattc_notify_app(&gattc_msg);
}
VOID bluetooth_gattc_connect_cbk(INT32 conn_id, INT32 status, INT32 client_if, CHAR* bda)
{
    tBT_MW_GATTC_MSG gattc_msg = {0};
    gattc_msg.event = BTMW_GATTC_CONNECT;
    memset(&g_gattc_connect_rst_info, 0x0, sizeof(BT_GATTC_CONNECT_RST_T));
    g_gattc_connect_rst_info.conn_id = conn_id;
    g_gattc_connect_rst_info.client_if = client_if;
    memcpy(g_gattc_connect_rst_info.btaddr, bda, strlen(bda) + 1);

    bt_mw_gattc_notify_app(&gattc_msg);
}

VOID bluetooth_gattc_disconnect_cbk(INT32 conn_id, INT32 status, INT32 client_if, CHAR* bda)
{
    tBT_MW_GATTC_MSG gattc_msg = {0};
    gattc_msg.event = BTMW_GATTC_DISCONNECT;
    memset(&g_gattc_disconnect_rst_info, 0x0, sizeof(BT_GATTC_CONNECT_RST_T));
    g_gattc_disconnect_rst_info.conn_id = conn_id;
    g_gattc_disconnect_rst_info.client_if = client_if;
    memcpy(g_gattc_disconnect_rst_info.btaddr, bda, strlen(bda) + 1);

    bt_mw_gattc_notify_app(&gattc_msg);
}

VOID bluetooth_gattc_get_gatt_db_cbk(INT32 conn_id,
                                                BT_GATTC_DB_ELEMENT_T *gatt_db,
                                                INT32 count)
{
    tBT_MW_GATTC_MSG gattc_msg = {0};
    gattc_msg.event = BTMW_GATTC_GATT_DB;
    gattc_msg.gattc_data.gattc_gatt_db.conn_id = conn_id;
    gattc_msg.gattc_data.gattc_gatt_db.count = count;
    gattc_msg.gattc_data.gattc_gatt_db.gatt_db_element = gatt_db;

    bt_mw_gattc_notify_app(&gattc_msg);
}

VOID bluetooth_gattc_register_for_notification_cbk(INT32 conn_id,
                                                              INT32 registered,
                                                              INT32 status,
                                                              UINT16 handle)
{
    tBT_MW_GATTC_MSG gattc_msg = {0};
    gattc_msg.event = BTMW_GATTC_REG_NTY;
    gattc_msg.gattc_data.gattc_reg_noti_rst.conn_id = conn_id;
    gattc_msg.gattc_data.gattc_reg_noti_rst.registered = registered;
    gattc_msg.gattc_data.gattc_reg_noti_rst.handle = handle;

    bt_mw_gattc_notify_app(&gattc_msg);
}

VOID bluetooth_gattc_notify_cbk(INT32 conn_id, BT_GATTC_NOTI_PARAMS_T *p_data)
{
    tBT_MW_GATTC_MSG gattc_msg = {0};
    gattc_msg.event = BTMW_GATTC_NOTIFY;
    gattc_msg.gattc_data.gattc_notify.conn_id = conn_id;
    memcpy(&gattc_msg.gattc_data.gattc_notify.notify_data, p_data, sizeof(BT_GATTC_NOTI_PARAMS_T));

    bt_mw_gattc_notify_app(&gattc_msg);
}

VOID bluetooth_gattc_read_characteristic_cbk(INT32 conn_id, INT32 status,
                                                         BT_GATTC_READ_PARAMS_T *p_data)
{
    tBT_MW_GATTC_MSG gattc_msg = {0};
    gattc_msg.event = BTMW_GATTC_READ_CHAR;
    gattc_msg.gattc_data.gattc_read_char_rst.conn_id = conn_id;
    memcpy(&gattc_msg.gattc_data.gattc_read_char_rst.read_data, p_data, sizeof(BT_GATTC_READ_PARAMS_T));

    bt_mw_gattc_notify_app(&gattc_msg);
}

VOID bluetooth_gattc_write_characteristic_cbk(INT32 conn_id, INT32 status,
                                                         UINT16 handle)
{
    tBT_MW_GATTC_MSG gattc_msg = {0};
    gattc_msg.event = BTMW_GATTC_WRITE_CHAR;
    gattc_msg.gattc_data.gattc_write_char_rst.conn_id = conn_id;
    gattc_msg.gattc_data.gattc_write_char_rst.status = status;
    gattc_msg.gattc_data.gattc_write_char_rst.handle = handle;

    bt_mw_gattc_notify_app(&gattc_msg);
}

VOID bluetooth_gattc_read_descriptor_cbk(INT32 conn_id, INT32 status,
                                                     BT_GATTC_READ_PARAMS_T *p_data)
{
    tBT_MW_GATTC_MSG gattc_msg = {0};
    gattc_msg.event = BTMW_GATTC_READ_DESC;
    gattc_msg.gattc_data.gattc_read_desc_rst.conn_id = conn_id;
    memcpy(&gattc_msg.gattc_data.gattc_read_desc_rst.read_data, p_data, sizeof(BT_GATTC_READ_PARAMS_T));

    bt_mw_gattc_notify_app(&gattc_msg);
}

VOID bluetooth_gattc_write_descriptor_cbk(INT32 conn_id, INT32 status,
                                                     UINT16 handle)
{
    tBT_MW_GATTC_MSG gattc_msg = {0};
    gattc_msg.event = BTMW_GATTC_WRITE_DESC;
    gattc_msg.gattc_data.gattc_write_desc_rst.conn_id = conn_id;
    gattc_msg.gattc_data.gattc_write_desc_rst.conn_id = status;
    gattc_msg.gattc_data.gattc_write_desc_rst.handle = handle;

    bt_mw_gattc_notify_app(&gattc_msg);
}

VOID bluetooth_gattc_read_remote_rssi_cbk(INT32 client_if, CHAR* bda,
                                                       INT32 rssi, INT32 status)
{
    tBT_MW_GATTC_MSG gattc_msg = {0};
    gattc_msg.event = BTMW_GATTC_REM_RSSI;
    memset(&g_gattc_remote_rssi_info, 0x0, sizeof(BT_GATTC_GET_REMOTE_RSSI_T));
    g_gattc_remote_rssi_info.client_if = client_if;
    g_gattc_remote_rssi_info.rssi= rssi;
    memcpy(g_gattc_remote_rssi_info.btaddr, bda, strlen(bda) + 1);

    bt_mw_gattc_notify_app(&gattc_msg);
}

VOID bluetooth_gattc_scan_filter_cfg_cbk(INT32 action, INT32 client_if,
                                                   INT32 status, INT32 filt_type,
                                                   INT32 avbl_space)
{
    tBT_MW_GATTC_MSG gattc_msg = {0};
    gattc_msg.event = BTMW_GATTC_SCAN_FILT_CFG;
    gattc_msg.gattc_data.gattc_scan_filter_cfg.client_if = client_if;
    gattc_msg.gattc_data.gattc_scan_filter_cfg.action = action;
    gattc_msg.gattc_data.gattc_scan_filter_cfg.filt_type = filt_type;
    gattc_msg.gattc_data.gattc_scan_filter_cfg.avbl_space = avbl_space;

    bt_mw_gattc_notify_app(&gattc_msg);
}

VOID bluetooth_gattc_scan_filter_param_cbk(INT32 action, INT32 client_if,
                                                       INT32 status, INT32 avbl_space)
{
    tBT_MW_GATTC_MSG gattc_msg = {0};
    gattc_msg.event = BTMW_GATTC_SCAN_FILT_PARAM;
    gattc_msg.gattc_data.gattc_scan_filter_param.client_if = client_if;
    gattc_msg.gattc_data.gattc_scan_filter_param.action = action;
    gattc_msg.gattc_data.gattc_scan_filter_param.avbl_space = avbl_space;

    bt_mw_gattc_notify_app(&gattc_msg);
}
VOID bluetooth_gattc_scan_filter_status_cbk(INT32 enable, INT32 client_if, INT32 status)
{
    tBT_MW_GATTC_MSG gattc_msg = {0};
    gattc_msg.event = BTMW_GATTC_SCAN_FILT_STATUS;
    gattc_msg.gattc_data.gattc_scan_filter_status.client_if = client_if;
    gattc_msg.gattc_data.gattc_scan_filter_status.enable= enable;

    bt_mw_gattc_notify_app(&gattc_msg);
}

VOID bluetooth_gattc_multi_adv_enable_cbk(INT32 client_if, INT32 status)
{
    g_fg_gattc_adv_enable = TRUE;

    if(NULL != BtGattAdvEnableCbk)
    {
        BtGattAdvEnableCbk(status);
    }
    else
    {
        BT_DBG_ERROR(BT_DEBUG_GATT,"[GATT] null pointer of BtGattAdvEnableCbk");
    }
}

VOID bluetooth_gattc_multi_adv_data_cbk(INT32 client_if, INT32 status)
{
    g_fg_gattc_adv_data = TRUE;
}
