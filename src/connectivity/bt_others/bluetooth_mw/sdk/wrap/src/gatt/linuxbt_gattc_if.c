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


/* FILE NAME:  linuxbt_gattc_if.c
 * AUTHOR: Xuemei Yang
 * PURPOSE:
 *      It provides GATTC operation interface and callback function implementation to MW higher layer.
 * NOTES:
 */


/*-----------------------------------------------------------------------------
                    include files
 ----------------------------------------------------------------------------*/
#include <string.h>
#include "bluetooth.h"
#include "bt_mw_common.h"
#include "linuxbt_gattc_if.h"
#include "linuxbt_common.h"

#define ADVERTISING_CHANNEL_ALL 7   //ADVERTISING_CHANNEL_37 | ADVERTISING_CHANNEL_38 | ADVERTISING_CHANNEL_39;

#define CHECK_GATTC_INTERFACE() if (NULL == linuxbt_gattc_interface)\
{\
    BT_DBG_ERROR(BT_DEBUG_GATT, "[GATTC] linuxbt_gattc_interface not init.");\
    return BT_ERR_STATUS_PARM_INVALID;\
}

#define CHECK_GATTC_EX_INTERFACE() if (NULL == linuxbt_gattc_ex_interface)\
{\
    BT_DBG_ERROR(BT_DEBUG_GATT, "[GATTC] linuxbt_gattc_ex_interface not init.");\
    return BT_ERR_STATUS_NOT_READY;\
}

#define CHECK_GATTC_CLIENT_IF(client_if) if (-1 == client_if)\
{\
    BT_DBG_ERROR(BT_DEBUG_GATT, "[GATTC] Close : no client app registered now.");\
    return BT_ERR_STATUS_PARM_INVALID;\
}

#define CHECK_GATTC_BT_ADDR(bt_addr) if (NULL == bt_addr)\
{\
    BT_DBG_ERROR(BT_DEBUG_GATT,"[GATTC] null pointer");\
    return BT_ERR_STATUS_PARM_INVALID;\
}

static void linuxbt_gattc_get_adv_rpa_callback(int client_if,
                                                          int status,
                                                          bt_bdaddr_t* rpa);

#if defined(MTK_LINUX_GATT) && (MTK_LINUX_GATT == TRUE)
static const btgatt_ex_client_interface_t *linuxbt_gattc_ex_interface = NULL;
const btgatt_ex_client_callbacks_t linuxbt_gattc_ex_callbacks =
{
    linuxbt_gattc_get_adv_rpa_callback,
};
#endif

static int g_linuxbt_client_if = -1;
static const btgatt_client_interface_t *linuxbt_gattc_interface = NULL;

// Callback functions
static void linuxbt_gattc_register_client_callback(int status, int client_if,
                                                            bt_uuid_t *app_uuid);

static void linuxbt_gattc_scan_result_callback(bt_bdaddr_t* bda, int rssi,
                                                        uint8_t* adv_data);

static void linuxbt_gattc_connect_callback(int conn_id, int status,
                                                    int client_if, bt_bdaddr_t* bda);

static void linuxbt_gattc_disconnect_callback(int conn_id, int status,
                                                        int client_if, bt_bdaddr_t* bda);

static void linuxbt_gattc_search_complete_callback(int conn_id, int status);

static void linuxbt_gattc_register_for_notification_callback(int conn_id,
                                                                        int registered,
                                                                        int status,
                                                                        uint16_t handle);

static void linuxbt_gattc_notify_callback(int conn_id, btgatt_notify_params_t *p_data);

static void linuxbt_gattc_read_characteristic_callback(int conn_id,
                                                                  int status,
                                                                  btgatt_read_params_t *p_data);

static void linuxbt_gattc_write_characteristic_callback(int conn_id,
                                                                   int status,
                                                                   uint16_t handle);

static void linuxbt_gattc_read_descriptor_callback(int conn_id,
                                                              int status,
                                                              btgatt_read_params_t *p_data);

static void linuxbt_gattc_write_descriptor_callback(int conn_id,
                                                               int status,
                                                               uint16_t handle);

static void linuxbt_gattc_execute_write_callback(int conn_id, int status);

static void linuxbt_gattc_read_remote_rssi_callback(int client_if,
                                                                bt_bdaddr_t* bda,
                                                                int rssi,
                                                                int status);

static void linuxbt_gattc_configure_mtu_callback(int conn_id,
                                                            int status,
                                                            int mtu);

static void linuxbt_gattc_scan_filter_cfg_callback(int action,
                                                            int client_if,
                                                            int status,
                                                            int filt_type,
                                                            int avbl_space);

static void linuxbt_gattc_scan_filter_param_callback(int action,
                                                                int client_if,
                                                                int status,
                                                                int avbl_space);

static void linuxbt_gattc_scan_filter_status_callback(int enable,
                                                                int client_if,
                                                                int status);

static void linuxbt_gattc_multi_adv_enable_callback(int client_if,
                                                                int status);

static void linuxbt_gattc_multi_adv_update_callback(int client_if,
                                                                int status);

static void linuxbt_gattc_multi_adv_data_callback(int client_if,
                                                             int status);

static void linuxbt_gattc_multi_adv_disable_callback(int client_if,
                                                                 int status);

static void linuxbt_gattc_batchscan_cfg_storage_callback(int client_if,
                                                                       int status);

static void linuxbt_gattc_batchscan_enable_disable_callback(int action,
                                                                           int client_if,
                                                                           int status);
static void linuxbt_gattc_batchscan_reports_callback(int client_if,
                                                                 int status,
                                                                 int report_format,
                                                                 int num_records,
                                                                 int data_len,
                                                                 uint8_t* rep_data);

static void linuxbt_gattc_batchscan_threshold_callback(int client_if);

static void linuxbt_gattc_scan_parameter_setup_completed_callback(int client_if,
    btgattc_error_t status);

static void linuxbt_gattc_get_gatt_db_callback(int conn_id,
                                                         btgatt_db_element_t *db,
                                                         int count);
#if MTK_LINUX_GATTC_RPA
static void linuxbt_gattc_get_adv_rpa_callback(int client_if,
                                                          int status,
                                                          bt_bdaddr_t* rpa);
#endif

const btgatt_client_callbacks_t linuxbt_gattc_callbacks =
{
    linuxbt_gattc_register_client_callback,
    linuxbt_gattc_scan_result_callback,
    linuxbt_gattc_connect_callback,
    linuxbt_gattc_disconnect_callback,
    linuxbt_gattc_search_complete_callback,
    linuxbt_gattc_register_for_notification_callback,
    linuxbt_gattc_notify_callback,
    linuxbt_gattc_read_characteristic_callback,
    linuxbt_gattc_write_characteristic_callback,
    linuxbt_gattc_read_descriptor_callback,
    linuxbt_gattc_write_descriptor_callback,
    linuxbt_gattc_execute_write_callback,
    linuxbt_gattc_read_remote_rssi_callback,
    NULL,
    linuxbt_gattc_configure_mtu_callback,
    linuxbt_gattc_scan_filter_cfg_callback,
    linuxbt_gattc_scan_filter_param_callback,
    linuxbt_gattc_scan_filter_status_callback,
    linuxbt_gattc_multi_adv_enable_callback,
    linuxbt_gattc_multi_adv_update_callback,
    linuxbt_gattc_multi_adv_data_callback,
    linuxbt_gattc_multi_adv_disable_callback,
    NULL,
    linuxbt_gattc_batchscan_cfg_storage_callback,
    linuxbt_gattc_batchscan_enable_disable_callback,
    linuxbt_gattc_batchscan_reports_callback,
    linuxbt_gattc_batchscan_threshold_callback,
    NULL,
    linuxbt_gattc_scan_parameter_setup_completed_callback,
    linuxbt_gattc_get_gatt_db_callback,
    NULL,
    NULL,
};

#if defined(MTK_LINUX_GATT) && (MTK_LINUX_GATT == TRUE)
int linuxbt_gattc_ex_init(const btgatt_ex_client_interface_t *pt_ex_interface)
{
    linuxbt_gattc_ex_interface = pt_ex_interface;
    return BT_SUCCESS;
}
#endif

int linuxbt_gattc_init(const btgatt_client_interface_t *pt_interface)
{
    BT_MW_FUNC_ENTER(BT_DEBUG_GATT, "");
    BT_ERR_STATUS_T ret = BT_SUCCESS;
    bt_uuid_t uuid;

    linuxbt_gattc_interface = pt_interface;
    linuxbt_uuid_stoh(LINUXBT_GATTC_APP_UUID, &uuid);
    return ret;
}

int linuxbt_gattc_deinit(void)
{
    return BT_SUCCESS;
}

int linuxbt_gattc_register_app(char *uuid)
{
    bt_uuid_t app_uuid;
    bt_status_t ret= BT_STATUS_SUCCESS;
    CHECK_GATTC_INTERFACE();
    if (NULL == linuxbt_gattc_interface->register_client)
    {
        BT_DBG_ERROR(BT_DEBUG_GATT, "[GATTC] null pointer of register_client");
        return BT_ERR_STATUS_PARM_INVALID;
    }
    linuxbt_uuid_stoh(uuid, &app_uuid);
    ret = linuxbt_gattc_interface->register_client(&app_uuid);
    BT_DBG_MINOR(BT_DEBUG_GATT,"ret = %d", ret);
    return linuxbt_return_value_convert(ret);
}

int linuxbt_gattc_unregister_app(int client_if)
{
    bt_status_t ret = BT_STATUS_SUCCESS;
    CHECK_GATTC_INTERFACE();
    if (client_if < 0 && g_linuxbt_client_if == -1)
    {
        BT_DBG_ERROR(BT_DEBUG_GATT, "[GATTC] Unregister client : no client app need to unregister.");
        return BT_ERR_STATUS_PARM_INVALID;
    }
    ret = linuxbt_gattc_interface->unregister_client(client_if);
    BT_DBG_MINOR(BT_DEBUG_GATT,"ret = %d", ret);
    g_linuxbt_client_if = -1;
    return linuxbt_return_value_convert(ret);
}

int linuxbt_gattc_scan(bool start)
{
    bt_status_t ret = BT_STATUS_SUCCESS;
    CHECK_GATTC_INTERFACE();
    ret = linuxbt_gattc_interface->scan(start);
    BT_DBG_MINOR(BT_DEBUG_GATT,"ret = %d", ret);
    return linuxbt_return_value_convert(ret);
}

int linuxbt_gattc_connect(int client_if, char *bt_addr, bool is_direct, int transport)
{
    bt_bdaddr_t bdaddr;
    bt_status_t ret = BT_STATUS_SUCCESS;

    CHECK_GATTC_INTERFACE();
    CHECK_GATTC_CLIENT_IF(client_if);
    CHECK_GATTC_BT_ADDR(bt_addr);
    linuxbt_btaddr_stoh(bt_addr, &bdaddr);
    BT_DBG_INFO(BT_DEBUG_GATT, "GATTC connect to %02X:%02X:%02X:%02X:%02X:%02X, client_if:%d",
                bdaddr.address[0], bdaddr.address[1], bdaddr.address[2],
                bdaddr.address[3], bdaddr.address[4], bdaddr.address[5], client_if);

    ret = linuxbt_gattc_interface->connect(client_if,&bdaddr,is_direct,transport);
    BT_DBG_MINOR(BT_DEBUG_GATT,"ret = %d", ret);
    return linuxbt_return_value_convert(ret);

}

int linuxbt_gattc_disconnect(int client_if, char *bt_addr, int conn_id)
{
    bt_bdaddr_t bdaddr;
    bt_status_t ret = BT_STATUS_SUCCESS;
    CHECK_GATTC_INTERFACE();
    CHECK_GATTC_CLIENT_IF(client_if);
    linuxbt_btaddr_stoh(bt_addr, &bdaddr);
    BT_DBG_INFO(BT_DEBUG_GATT, "GATTC disconnect to %02X:%02X:%02X:%02X:%02X:%02X, client_if:%d",
                bdaddr.address[0], bdaddr.address[1], bdaddr.address[2],
                bdaddr.address[3], bdaddr.address[4], bdaddr.address[5], client_if);

    ret = linuxbt_gattc_interface->disconnect(client_if,&bdaddr,conn_id);
    BT_DBG_MINOR(BT_DEBUG_GATT,"ret = %d", ret);
    return linuxbt_return_value_convert(ret);

}

int linuxbt_gattc_listen(int client_if)
{
    bt_status_t ret = BT_STATUS_SUCCESS;
    CHECK_GATTC_INTERFACE();
    CHECK_GATTC_CLIENT_IF(client_if);
    BT_DBG_INFO(BT_DEBUG_GATT, "[GATTC] Listen client_if: %d", client_if);
    ret = linuxbt_gattc_interface->listen(client_if,true);
    BT_DBG_MINOR(BT_DEBUG_GATT,"ret = %d", ret);
    return linuxbt_return_value_convert(ret);

}

int linuxbt_gattc_refresh(int client_if, char *bt_addr)
{
    bt_bdaddr_t bdaddr;
    bt_status_t ret = BT_STATUS_SUCCESS;
    CHECK_GATTC_INTERFACE();
    CHECK_GATTC_CLIENT_IF(client_if);
    linuxbt_btaddr_stoh(bt_addr, &bdaddr);
    BT_DBG_INFO(BT_DEBUG_GATT, "GATTC refresh %02X:%02X:%02X:%02X:%02X:%02X , client_if:%d",
                bdaddr.address[0], bdaddr.address[1], bdaddr.address[2],
                bdaddr.address[3], bdaddr.address[4], bdaddr.address[5], client_if);
    ret = linuxbt_gattc_interface->refresh(client_if,&bdaddr);
    BT_DBG_MINOR(BT_DEBUG_GATT,"ret = %d", ret);
    return linuxbt_return_value_convert(ret);
}

int linuxbt_gattc_search_service(int conn_id, char *pt_uuid)
{
    bt_status_t ret = BT_STATUS_SUCCESS;
    bt_uuid_t uuid;
    bt_uuid_t *uuid_ptr = NULL;
    CHECK_GATTC_INTERFACE();
    if (NULL == pt_uuid)
    {
        BT_DBG_ERROR(BT_DEBUG_GATT, "[GATTC] pt_uuid is null.");
        uuid_ptr = NULL;
    }
    else
    {
        linuxbt_uuid_stoh(pt_uuid, &uuid);
        uuid_ptr = &uuid;
    }
    ret = linuxbt_gattc_interface->search_service(conn_id, uuid_ptr);
    BT_DBG_MINOR(BT_DEBUG_GATT,"ret = %d", ret);
    return linuxbt_return_value_convert(ret);
}

int linuxbt_gattc_get_gatt_db(int conn_id)
{
    FUNC_ENTRY;
    bt_status_t ret = BT_STATUS_SUCCESS;
    CHECK_GATTC_INTERFACE();
    ret = linuxbt_gattc_interface->get_gatt_db(conn_id);
    BT_DBG_MINOR(BT_DEBUG_GATT,"ret = %d", ret);
    return linuxbt_return_value_convert(ret);
}

int linuxbt_gattc_read_char(int conn_id, int char_handle, int auth_req)
{
    FUNC_ENTRY;
    bt_status_t ret = BT_STATUS_SUCCESS;
    CHECK_GATTC_INTERFACE();
    ret = linuxbt_gattc_interface->read_characteristic(conn_id,char_handle,auth_req);
    BT_DBG_MINOR(BT_DEBUG_GATT,"ret = %d", ret);
    return linuxbt_return_value_convert(ret);


}

int linuxbt_gattc_read_descr(int conn_id, int descr_handle, int auth_req)
{
    FUNC_ENTRY;
    bt_status_t ret = BT_STATUS_SUCCESS;
    CHECK_GATTC_INTERFACE();
    ret = linuxbt_gattc_interface->read_descriptor(conn_id,descr_handle,auth_req);
    BT_DBG_MINOR(BT_DEBUG_GATT,"ret = %d", ret);
    return linuxbt_return_value_convert(ret);

}

int linuxbt_gattc_write_char(int conn_id, int char_handle,int write_type,
                                    int len, int auth_req, char *value)
{
    FUNC_ENTRY;
    bt_status_t ret = BT_STATUS_SUCCESS;
    CHECK_GATTC_INTERFACE();
    //WRITE_TYPE_DEFAULT = 2, WRITE_TYPE_NO_RESPONSE = 1, WRITE_TYPE_SIGNED = 4
    ret = linuxbt_gattc_interface->write_characteristic(conn_id,char_handle,write_type,len,auth_req,value);
    BT_DBG_MINOR(BT_DEBUG_GATT,"ret = %d", ret);
    return linuxbt_return_value_convert(ret);
}

int linuxbt_gattc_write_descr(int conn_id, int descr_handle, int write_type,
                                     int len, int auth_req, char *value)
{
    FUNC_ENTRY;
    bt_status_t ret = BT_STATUS_SUCCESS;
    CHECK_GATTC_INTERFACE();
    //WRITE_TYPE_DEFAULT = 2, WRITE_TYPE_NO_RESPONSE = 1, WRITE_TYPE_SIGNED = 4
    ret = linuxbt_gattc_interface->write_descriptor(conn_id,descr_handle,write_type,len,auth_req,value);
    BT_DBG_MINOR(BT_DEBUG_GATT,"ret = %d", ret);
    return linuxbt_return_value_convert(ret);
}

int linuxbt_gattc_execute_write(int conn_id, int execute)
{
    bt_status_t ret = BT_STATUS_SUCCESS;
    CHECK_GATTC_INTERFACE();
    CHECK_GATTC_CLIENT_IF(g_linuxbt_client_if);

    /* GATTC execute_write conn_id execute */
    ret = linuxbt_gattc_interface->execute_write(conn_id,execute);
    BT_DBG_MINOR(BT_DEBUG_GATT,"ret = %d", ret);
    return linuxbt_return_value_convert(ret);
}

int linuxbt_gattc_set_reg_noti(int client_if, char *bt_addr,
                                      int char_handle, bool enable)
{
    bt_bdaddr_t bdaddr;
    bt_status_t ret = BT_STATUS_SUCCESS;
    CHECK_GATTC_INTERFACE();
    CHECK_GATTC_CLIENT_IF(client_if);
    linuxbt_btaddr_stoh(bt_addr, &bdaddr);
    if (enable)
    {
        ret = linuxbt_gattc_interface->register_for_notification(client_if,&bdaddr,char_handle);
    }
    else
    {
        ret = linuxbt_gattc_interface->deregister_for_notification(client_if,&bdaddr,char_handle);
    }
    BT_DBG_MINOR(BT_DEBUG_GATT,"ret = %d", ret);
    return linuxbt_return_value_convert(ret);

}

int linuxbt_gattc_read_rssi(int client_if, char *bt_addr)
{
    bt_status_t ret = BT_STATUS_SUCCESS;
    bt_bdaddr_t bdaddr;
    CHECK_GATTC_INTERFACE();
    CHECK_GATTC_CLIENT_IF(client_if);
    CHECK_GATTC_BT_ADDR(bt_addr);
    /* GATTC read_rssi BD_ADDR */
    linuxbt_btaddr_stoh(bt_addr, &bdaddr);
    ret = linuxbt_gattc_interface->read_remote_rssi(client_if,&bdaddr);
    BT_DBG_MINOR(BT_DEBUG_GATT,"ret = %d", ret);
    return linuxbt_return_value_convert(ret);
}

int linuxbt_gattc_conn_parameter_update(char *pbt_addr,
                                                     int min_interval,
                                                     int max_interval,
                                                     int latency,
                                                     int timeout)
{
    bt_status_t ret = BT_STATUS_SUCCESS;
    bt_bdaddr_t bd_addr;
    CHECK_GATTC_INTERFACE();
    CHECK_GATTC_BT_ADDR(pbt_addr);
    linuxbt_btaddr_stoh(pbt_addr, &bd_addr);
    BT_DBG_INFO(BT_DEBUG_GATT, "[GATTC] %02X:%02X:%02X:%02X:%02X:%02X ,",
                bd_addr.address[0], bd_addr.address[1], bd_addr.address[2],
                bd_addr.address[3], bd_addr.address[4], bd_addr.address[5]);
    ret = linuxbt_gattc_interface->conn_parameter_update(&bd_addr, min_interval,
                                                         max_interval, latency,
                                                         timeout);
    BT_DBG_MINOR(BT_DEBUG_GATT,"ret = %d", ret);
    return linuxbt_return_value_convert(ret);

}

int linuxbt_gattc_set_scan_parameters(int client_if, int scan_interval,
                                                 int scan_window)
{
    bt_status_t ret = BT_STATUS_SUCCESS;
    CHECK_GATTC_INTERFACE();
    CHECK_GATTC_CLIENT_IF(client_if);
    ret = linuxbt_gattc_interface->set_scan_parameters(client_if, scan_interval,
                                                       scan_window);
    BT_DBG_MINOR(BT_DEBUG_GATT,"ret = %d", ret);
    return linuxbt_return_value_convert(ret);

}

int linuxbt_gattc_multi_adv_enable(int client_if,
                                             int min_interval,
                                             int max_interval,
                                             int adv_type,
                                             int chnl_map,
                                             int tx_power,
                                             int timeout_s)
{
    bt_status_t ret = BT_STATUS_SUCCESS;
    CHECK_GATTC_INTERFACE();
    CHECK_GATTC_CLIENT_IF(client_if);
    BT_DBG_INFO(BT_DEBUG_GATT, "client_if:%d,min_interval:%d ,max_interval:%d, adv_type:%d,chnl_map:%d,tx_power:%d,timeout_s:%d",
                client_if,min_interval,max_interval,adv_type,ADVERTISING_CHANNEL_ALL,tx_power,timeout_s);
    /*GATTC adv_enable min_interval max_interval adv_type tx_power timeout*/
    ret = linuxbt_gattc_interface->multi_adv_enable(client_if, min_interval,
                                                    max_interval, adv_type,
                                                    ADVERTISING_CHANNEL_ALL,
                                                    tx_power, timeout_s);
    BT_DBG_MINOR(BT_DEBUG_GATT,"ret = %d", ret);
    return linuxbt_return_value_convert(ret);

}

int linuxbt_gattc_multi_adv_update(int client_if, int min_interval,
                                             int max_interval, int adv_type,
                                             int chnl_map, int tx_power,
                                             int timeout_s)
{
    bt_status_t ret = BT_STATUS_SUCCESS;
    CHECK_GATTC_INTERFACE();
    CHECK_GATTC_CLIENT_IF(client_if);
    BT_DBG_INFO(BT_DEBUG_GATT, "client_if:%d,min_interval:%d ,max_interval:%d, adv_type:%d,chnl_map:%d,tx_power:%d,timeout_s:%d",
                client_if,min_interval,max_interval,adv_type,ADVERTISING_CHANNEL_ALL,tx_power,timeout_s);

    /*GATTC adv_update min_interval max_interval adv_type tx_power timeout*/
    ret = linuxbt_gattc_interface->multi_adv_update(client_if, min_interval,
                                                    max_interval, adv_type,
                                                    ADVERTISING_CHANNEL_ALL,
                                                    tx_power, timeout_s);
    BT_DBG_MINOR(BT_DEBUG_GATT,"ret = %d", ret);
    return linuxbt_return_value_convert(ret);
}

int linuxbt_gattc_multi_adv_setdata(int client_if,
                                             uint8_t set_scan_rsp,
                                             uint8_t include_name,
                                             uint8_t include_txpower,
                                             int appearance,
                                             int manufacturer_len,
                                             char* manufacturer_data,
                                             int service_data_len,
                                             char* service_data,
                                             int service_uuid_len,
                                             char* service_uuid)
{
    bt_status_t ret = BT_STATUS_SUCCESS;
    char uuid[BT_GATT_UUID_ARRAY_SIZE] = {0};
    CHECK_GATTC_INTERFACE();
    CHECK_GATTC_CLIENT_IF(client_if);

    /*GATTC adv_update_data [set_scan_rsp true|false] [include_name true|false] [incl_txpower true|false] [appearance int] [manufacturer_data hex_string] [service_data hex_string] [service_uuid hex_string]*/
    BT_DBG_INFO(BT_DEBUG_GATT, "set_scan_rsp : %d" ,set_scan_rsp);
    BT_DBG_INFO(BT_DEBUG_GATT, "include_name : %d" ,include_name);
    BT_DBG_INFO(BT_DEBUG_GATT, "include_txpower : %d" ,include_txpower);
    BT_DBG_INFO(BT_DEBUG_GATT, "appearance : %d" ,appearance);
    BT_DBG_INFO(BT_DEBUG_GATT, "manufacturer_len : %d" ,manufacturer_len);
    BT_DBG_INFO(BT_DEBUG_GATT, "service_data_len : %d" ,service_data_len);
    BT_DBG_INFO(BT_DEBUG_GATT, "service_uuid_len : %d" ,service_uuid_len);
    if (service_uuid)
    {
        bluetooth_uuid_stoh(service_uuid, uuid);
        ret = linuxbt_gattc_interface->multi_adv_set_inst_data(client_if,
                                                               set_scan_rsp,
                                                               include_name,
                                                               include_txpower,
                                                               appearance,
                                                               manufacturer_len,
                                                               manufacturer_data,
                                                               service_data_len,
                                                               service_data,
                                                               BT_GATT_UUID_ARRAY_SIZE,
                                                               uuid);
    }
    else
    {

        ret = linuxbt_gattc_interface->multi_adv_set_inst_data(client_if,
                                                               set_scan_rsp,
                                                               include_name,
                                                               include_txpower,
                                                               appearance,
                                                               manufacturer_len,
                                                               manufacturer_data,
                                                               service_data_len,
                                                               service_data,
                                                               service_uuid_len,
                                                               service_uuid);
    }
    BT_DBG_MINOR(BT_DEBUG_GATT,"ret = %d", ret);
    return linuxbt_return_value_convert(ret);
}

int linuxbt_gattc_multi_adv_disable(int client_if)
{
    bt_status_t ret = BT_STATUS_SUCCESS;
    CHECK_GATTC_INTERFACE();
    CHECK_GATTC_CLIENT_IF(client_if);
    ret = linuxbt_gattc_interface->multi_adv_disable(client_if);
    BT_DBG_MINOR(BT_DEBUG_GATT,"ret = %d", ret);
    return linuxbt_return_value_convert(ret);
}

int linuxbt_gattc_batchscan_cfg_storage(int client_if,
                                                   int batch_scan_full_max,
                                                   int batch_scan_trunc_max,
                                                   int batch_scan_notify_threshold)
{
    FUNC_ENTRY;
    bt_status_t ret = BT_STATUS_SUCCESS;
    CHECK_GATTC_INTERFACE();
    CHECK_GATTC_CLIENT_IF(client_if);
    ret = linuxbt_gattc_interface->batchscan_cfg_storage(client_if,
                batch_scan_full_max, batch_scan_trunc_max, batch_scan_notify_threshold);
    BT_DBG_MINOR(BT_DEBUG_GATT,"ret = %d", ret);
    return linuxbt_return_value_convert(ret);
}

int linuxbt_gattc_batchscan_enb_batch_scan(int client_if,
                                                         int scan_mode,
                                                         int scan_interval,
                                                         int scan_window,
                                                         int addr_type,
                                                         int discard_rule)
{
    FUNC_ENTRY;
    bt_status_t ret = BT_STATUS_SUCCESS;
    CHECK_GATTC_INTERFACE();
    CHECK_GATTC_CLIENT_IF(client_if);
    ret = linuxbt_gattc_interface->batchscan_enb_batch_scan(client_if,
                scan_mode, scan_interval, scan_window, addr_type, discard_rule);
    BT_DBG_MINOR(BT_DEBUG_GATT,"ret = %d", ret);
    return linuxbt_return_value_convert(ret);

}

int linuxbt_gattc_batchscan_dis_batch_scan(int client_if)
{
    FUNC_ENTRY;
    bt_status_t ret = BT_STATUS_SUCCESS;
    CHECK_GATTC_INTERFACE();
    CHECK_GATTC_CLIENT_IF(client_if);
    ret = linuxbt_gattc_interface->batchscan_dis_batch_scan(client_if);
    BT_DBG_MINOR(BT_DEBUG_GATT,"ret = %d", ret);
    return linuxbt_return_value_convert(ret);

}

int linuxbt_gattc_batchscan_read_reports(int client_if, int scan_mode)
{
    FUNC_ENTRY;
    bt_status_t ret = BT_STATUS_SUCCESS;
    CHECK_GATTC_INTERFACE();
    CHECK_GATTC_CLIENT_IF(client_if);
    ret = linuxbt_gattc_interface->batchscan_read_reports(client_if, scan_mode);
    BT_DBG_MINOR(BT_DEBUG_GATT,"ret = %d", ret);
    return linuxbt_return_value_convert(ret);
}

int linuxbt_gattc_scan_filter_param_setup(BT_GATTC_FILT_PARAM_SETUP_T scan_filt_param)
{
    bt_status_t ret = BT_STATUS_SUCCESS;
    CHECK_GATTC_INTERFACE();
    CHECK_GATTC_CLIENT_IF(scan_filt_param.client_if);

    /*GATTC adv_enable min_interval max_interval adv_type tx_power timeout*/
    btgatt_filt_param_setup_t filt_param;
    filt_param.client_if = scan_filt_param.client_if;
    filt_param.action = scan_filt_param.action;
    filt_param.filt_index = scan_filt_param.filt_index;
    filt_param.feat_seln = scan_filt_param.feat_seln;
    filt_param.list_logic_type = scan_filt_param.list_logic_type;
    filt_param.filt_logic_type = scan_filt_param.filt_logic_type;
    filt_param.rssi_high_thres = scan_filt_param.rssi_high_thres;
    filt_param.rssi_low_thres = scan_filt_param.rssi_low_thres;
    filt_param.dely_mode = scan_filt_param.dely_mode;
    filt_param.found_timeout = scan_filt_param.found_timeout;
    filt_param.lost_timeout = scan_filt_param.lost_timeout;
    filt_param.found_timeout_cnt = scan_filt_param.found_timeout_cnt;
    filt_param.num_of_tracking_entries = scan_filt_param.num_of_tracking_entries;
    ret = linuxbt_gattc_interface->scan_filter_param_setup(filt_param);
    BT_DBG_MINOR(BT_DEBUG_GATT,"ret = %d", ret);
    return linuxbt_return_value_convert(ret);
}

int linuxbt_gattc_scan_filter_enable(int client_if, bool enable)
{
    bt_status_t ret = BT_STATUS_SUCCESS;
    CHECK_GATTC_INTERFACE();
    CHECK_GATTC_CLIENT_IF(client_if);
    BT_DBG_NORMAL(BT_DEBUG_GATT,"client_if:%d",client_if);
    /*GATTC adv_enable min_interval max_interval adv_type tx_power timeout*/
    ret = linuxbt_gattc_interface->scan_filter_enable(client_if, enable);
    BT_DBG_MINOR(BT_DEBUG_GATT,"ret = %d", ret);
    return linuxbt_return_value_convert(ret);
}

int linuxbt_gattc_scan_filter_add_remove(int client_if,
                                                     int action,
                                                     int filt_type,
                                                     int filt_index,
                                                     int company_id,
                                                     int company_id_mask,
                                                     const char *p_uuid,
                                                     const char *p_uuid_mask,
                                                     const char *bd_addr,
                                                     char addr_type,
                                                     int data_len,
                                                     char* p_data,
                                                     int mask_len,
                                                     char* p_mask)
{
    bt_status_t ret = BT_STATUS_SUCCESS;
    bt_uuid_t uuid;
    bt_uuid_t uuid_mask;
    bt_bdaddr_t bdaddr;
    CHECK_GATTC_INTERFACE();
    CHECK_GATTC_CLIENT_IF(client_if);

    if (NULL == p_uuid)
    {
        BT_DBG_NOTICE(BT_DEBUG_GATT, "[GATTC] p_uuid is null.");
    }
    if (NULL == p_uuid_mask)
    {
        BT_DBG_NOTICE(BT_DEBUG_GATT, "[GATTC] p_uuid is null.");
    }
    if (NULL == bd_addr)
    {
        BT_DBG_NOTICE(BT_DEBUG_GATT,"[GATTC] null pointer");
    }

    linuxbt_uuid_stoh((CHAR*)p_uuid, &uuid);
    linuxbt_uuid_stoh((CHAR*)p_uuid_mask, &uuid_mask);
    linuxbt_btaddr_stoh((CHAR*)bd_addr, &bdaddr);

    ret = linuxbt_gattc_interface->scan_filter_add_remove(client_if,action,filt_type,
            filt_index,company_id,company_id_mask,&uuid,
            &uuid_mask,&bdaddr,addr_type,data_len,p_data,mask_len,p_mask);
    BT_DBG_MINOR(BT_DEBUG_GATT,"ret = %d", ret);
    return linuxbt_return_value_convert(ret);
}

int linuxbt_gattc_scan_filter_clear(int client_if, int filt_index)
{
    bt_status_t ret = BT_STATUS_SUCCESS;
    CHECK_GATTC_INTERFACE();
    CHECK_GATTC_CLIENT_IF(client_if);
    ret = linuxbt_gattc_interface->scan_filter_clear(client_if,filt_index);
    BT_DBG_MINOR(BT_DEBUG_GATT,"ret = %d", ret);
    return linuxbt_return_value_convert(ret);
}

int linuxbt_gattc_get_device_type(char *bt_addr)
{
    bt_bdaddr_t bdaddr;
    CHECK_GATTC_INTERFACE();
    CHECK_GATTC_BT_ADDR(bt_addr);
    linuxbt_btaddr_stoh(bt_addr, &bdaddr);
    return linuxbt_gattc_interface->get_device_type(&bdaddr);
}

int linuxbt_gattc_configure_mtu(int conn_id, int mtu)
{
    bt_status_t ret = BT_STATUS_SUCCESS;
    CHECK_GATTC_INTERFACE();
    if ((mtu < BT_GATTC_MIN_MTU_SIZE) || (mtu > BT_GATTC_MAX_MTU_SIZE))
    {
        BT_DBG_ERROR(BT_DEBUG_GATT,"[GATTC] invalid mtu size %d.", mtu);
        return BT_ERR_STATUS_PARM_INVALID;
    }
    BT_DBG_INFO(BT_DEBUG_GATT, "gattc config mtu : %d", mtu);
    ret = linuxbt_gattc_interface->configure_mtu(conn_id, mtu);
    BT_DBG_MINOR(BT_DEBUG_GATT,"ret = %d", ret);
    return linuxbt_return_value_convert(ret);
}

#if MTK_LINUX_GATTC_LE_NAME
int linuxbt_gattc_set_local_le_name(int client_if, char *name)
{
    bt_status_t ret = BT_STATUS_SUCCESS;
    CHECK_GATTC_EX_INTERFACE();
    CHECK_GATTC_CLIENT_IF(client_if);
    if (NULL == name)
    {
        BT_DBG_ERROR(BT_DEBUG_GATT,"[GATTC] input name is NULL.");
        return BT_ERR_STATUS_PARM_INVALID;
    }
    ret = linuxbt_gattc_ex_interface->set_local_le_name(client_if,name);
    BT_DBG_MINOR(BT_DEBUG_GATT,"ret = %d", ret);
    return linuxbt_return_value_convert(ret);
}
#endif

#if MTK_LINUX_GATTC_RPA
int linuxbt_gattc_get_local_adv_rpa(int client_if)
{
    bt_status_t ret = BT_STATUS_SUCCESS;
    CHECK_GATTC_EX_INTERFACE();
    CHECK_GATTC_CLIENT_IF(client_if);
    ret = linuxbt_gattc_ex_interface->get_adv_rpa(client_if);
    BT_DBG_MINOR(BT_DEBUG_GATT,"ret = %d", ret);
    return linuxbt_return_value_convert(ret);
}
#endif

#if MTK_LINUX_GATTC_DISC_MODE
int linuxbt_gattc_set_local_disc_mode(int client_if, int disc_mode)
{
    bt_status_t ret = BT_STATUS_SUCCESS;
    CHECK_GATTC_EX_INTERFACE();
    CHECK_GATTC_CLIENT_IF(client_if);
    ret = linuxbt_gattc_ex_interface->set_discoverable_mode(client_if,disc_mode);
    BT_DBG_MINOR(BT_DEBUG_GATT,"ret = %d", ret);
    return linuxbt_return_value_convert(ret);
}
#endif

#if MTK_LINUX_GATTC_PTS_TEST
int linuxbt_gattc_read_using_char_uuid(int conn_id,
                                                  int start_handle,
                                                  int end_handle,
                                                  CHAR *uuid,
                                                  int auth_req)
{
    FUNC_ENTRY;
    bt_status_t ret = BT_STATUS_SUCCESS;
    bt_uuid_t char_uuid;
    CHECK_GATTC_EX_INTERFACE();
    linuxbt_uuid_stoh(uuid, &char_uuid);
    ret = linuxbt_gattc_ex_interface->read_using_characteristic_uuid(conn_id,
                                                                     start_handle,
                                                                     end_handle,
                                                                     &char_uuid,
                                                                     auth_req);
    BT_DBG_MINOR(BT_DEBUG_GATT,"ret = %d", ret);
    return linuxbt_return_value_convert(ret);
}

int linuxbt_gattc_read_long_characteristic(int conn_id, uint16_t handle,
                                                     uint16_t offset, int auth_req)
{
    FUNC_ENTRY;
    bt_status_t ret = BT_STATUS_SUCCESS;
    CHECK_GATTC_EX_INTERFACE();
    ret = linuxbt_gattc_ex_interface->read_long_characteristic(conn_id, handle,
                                                               offset, auth_req);
    BT_DBG_MINOR(BT_DEBUG_GATT,"ret = %d", ret);
    return linuxbt_return_value_convert(ret);
}

int linuxbt_gattc_read_multi_characteristic(int conn_id, uint8_t num_attr,
                                                      uint16_t *handles, int auth_req)
{
    FUNC_ENTRY;
    bt_status_t ret = BT_STATUS_SUCCESS;
    CHECK_GATTC_EX_INTERFACE();
    if (NULL == handles)
    {
        BT_DBG_ERROR(BT_DEBUG_GATT,"[GATTC] handles is NULL.");
        return BT_ERR_STATUS_PARM_INVALID;
    }
    ret = linuxbt_gattc_ex_interface->read_multi_characteristic(conn_id, num_attr,
                                                                handles, auth_req);
    BT_DBG_MINOR(BT_DEBUG_GATT,"ret = %d", ret);
    return linuxbt_return_value_convert(ret);
}

int linuxbt_gattc_read_long_descr(int conn_id, uint16_t handle,
                                           uint16_t offset, int auth_req)
{
    FUNC_ENTRY;
    bt_status_t ret = BT_STATUS_SUCCESS;
    CHECK_GATTC_EX_INTERFACE();
    ret = linuxbt_gattc_ex_interface->read_long_char_descr(conn_id, handle, offset, auth_req);
    BT_DBG_MINOR(BT_DEBUG_GATT,"ret = %d", ret);
    return linuxbt_return_value_convert(ret);
}

int linuxbt_gattc_write_long_characteristic(int conn_id, uint16_t handle,
                                                      int write_type, int len,
                                                      uint16_t offset, int auth_req,
                                                      char* p_value)
{
    FUNC_ENTRY;
    bt_status_t ret = BT_STATUS_SUCCESS;
    CHECK_GATTC_EX_INTERFACE();
    ret = linuxbt_gattc_ex_interface->write_long_char(conn_id, handle, write_type,
                                                      len, offset, auth_req, p_value);
    BT_DBG_MINOR(BT_DEBUG_GATT, "ret = %d", ret);
    return linuxbt_return_value_convert(ret);
}

int linuxbt_gattc_write_long_descr(int conn_id, uint16_t handle,
                                            int write_type, int len,
                                            uint16_t offset, int auth_req,
                                            char* p_value)
{
    FUNC_ENTRY;
    bt_status_t ret = BT_STATUS_SUCCESS;
    CHECK_GATTC_EX_INTERFACE();
    ret = linuxbt_gattc_ex_interface->write_long_char_descr(conn_id, handle, write_type,
                                                            len, offset, auth_req,
                                                            p_value);
    BT_DBG_MINOR(BT_DEBUG_GATT, "ret = %d", ret);
    return linuxbt_return_value_convert(ret);
}

int linuxbt_gattc_set_pts_flag(uint8_t pts_flag)
{
    FUNC_ENTRY;
    bt_status_t ret = BT_STATUS_SUCCESS;
    CHECK_GATTC_EX_INTERFACE();
    ret = linuxbt_gattc_ex_interface->set_pts_test_flag(pts_flag);
    BT_DBG_MINOR(BT_DEBUG_GATT, "ret = %d", ret);
    return linuxbt_return_value_convert(ret);
}
#endif

/*for test mode interface*/
int linuxbt_gattc_test_command(int command, BT_GATTC_TEST_PARAMS_T test_params)
{
    FUNC_ENTRY;
    bt_status_t ret = BT_STATUS_SUCCESS;
    CHECK_GATTC_INTERFACE();
    btgatt_test_params_t params;
    params.u1 = test_params.u1;
    params.u2 = test_params.u2;
    params.u3 = test_params.u3;
    params.u4 = test_params.u4;
    params.u5 = test_params.u5;
    linuxbt_btaddr_stoh(test_params.bda1, params.bda1);
    linuxbt_uuid_stoh(test_params.uuid1, params.uuid1);
    ret = linuxbt_gattc_interface->test_command(command, &params);
    BT_DBG_MINOR(BT_DEBUG_GATT, "ret = %d", ret);
    return linuxbt_return_value_convert(ret);
}

static void linuxbt_gattc_register_client_callback(int status, int client_if,
                                                            bt_uuid_t *app_uuid)
{
    BT_DBG_INFO(BT_DEBUG_GATT, "[GATTC] Register client callback client_if:%d, status:%d", client_if,status);
    if (0 == status)
    {
        g_linuxbt_client_if = client_if;
    }

    char uuid[BT_GATT_MAX_UUID_LEN] = {0};
    linuxbt_print_uuid(app_uuid, uuid);
    bluetooth_gattc_register_client_cbk(status, client_if, uuid);
}

static void linuxbt_gattc_scan_result_callback(bt_bdaddr_t* bda, int rssi,
                                                        uint8_t* adv_data)
{
    BT_DBG_INFO(BT_DEBUG_GATT, "Find %02X:%02X:%02X:%02X:%02X:%02X\n , rssi : %d",
                bda->address[0], bda->address[1], bda->address[2],
                bda->address[3], bda->address[4], bda->address[5],rssi);

    char btaddr_s[MAX_BDADDR_LEN];
    linuxbt_btaddr_htos(bda, btaddr_s);
    bluetooth_gattc_scan_result_cbk(btaddr_s, rssi, adv_data);
}

static void linuxbt_gattc_connect_callback(int conn_id, int status,
                                                    int client_if, bt_bdaddr_t* bda)
{
    BT_DBG_NORMAL(BT_DEBUG_GATT, "%02X:%02X:%02X:%02X:%02X:%02X\n connected, conn_id = %d , status = %d",
                bda->address[0], bda->address[1], bda->address[2],
                bda->address[3], bda->address[4], bda->address[5],conn_id,status);
    char btaddr_s[MAX_BDADDR_LEN];
    linuxbt_btaddr_htos(bda, btaddr_s);
    bluetooth_gattc_connect_cbk(conn_id, status, client_if, btaddr_s);
}

static void linuxbt_gattc_disconnect_callback(int conn_id, int status,
                                                        int client_if, bt_bdaddr_t* bda)
{
    BT_DBG_INFO(BT_DEBUG_GATT, "%02X:%02X:%02X:%02X:%02X:%02X\n disconnected, conn_id = %d , status = %d",
                bda->address[0], bda->address[1], bda->address[2],
                bda->address[3], bda->address[4], bda->address[5],conn_id,status);
    char btaddr_s[MAX_BDADDR_LEN];
    linuxbt_btaddr_htos(bda, btaddr_s);
    bluetooth_gattc_disconnect_cbk(conn_id, status, client_if, btaddr_s);
}

static void linuxbt_gattc_search_complete_callback(int conn_id, int status)
{
    BT_DBG_INFO(BT_DEBUG_GATT,"search complete status = %d",status);
}

static void linuxbt_gattc_register_for_notification_callback(int conn_id,
                                                                        int registered,
                                                                        int status,
                                                                        uint16_t handle)
{
    FUNC_ENTRY;
    bluetooth_gattc_register_for_notification_cbk(conn_id, registered, status, handle);

}

static void linuxbt_gattc_notify_callback(int conn_id, btgatt_notify_params_t *p_data)
{
    FUNC_ENTRY;
    BT_GATTC_NOTI_PARAMS_T noti_params_data;
    memcpy(noti_params_data.value, p_data->value, BT_GATT_MAX_ATTR_LEN);
    linuxbt_btaddr_htos(&(p_data->bda), noti_params_data.bda);
    noti_params_data.handle = p_data->handle;
    noti_params_data.len = p_data->len;
    noti_params_data.is_notify = p_data->is_notify;
    bluetooth_gattc_notify_cbk(conn_id, &noti_params_data);
}

static void linuxbt_gattc_read_characteristic_callback(int conn_id,
                                                                  int status,
                                                                  btgatt_read_params_t *p_data)
{
    BT_GATTC_READ_PARAMS_T read_params_data;
    memset(&read_params_data, 0 , sizeof(BT_GATTC_READ_PARAMS_T));
    read_params_data.handle = p_data->handle;
    read_params_data.status = p_data->status;
    read_params_data.value_type = p_data->value_type;
    read_params_data.value.len = p_data->value.len;
    memcpy(read_params_data.value.value, p_data->value.value, p_data->value.len);

    BT_DBG_INFO(BT_DEBUG_GATT,"handle = %d", read_params_data.handle);
    BT_DBG_INFO(BT_DEBUG_GATT,"status = %d", read_params_data.status);
    BT_DBG_INFO(BT_DEBUG_GATT,"value_type = %d", read_params_data.value_type);
    BT_DBG_INFO(BT_DEBUG_GATT,"value len = %d", read_params_data.value.len);
    BT_DBG_INFO(BT_DEBUG_GATT,"value = %s", read_params_data.value.value);
    bluetooth_gattc_read_characteristic_cbk(conn_id, status, &read_params_data);
}

static void linuxbt_gattc_write_characteristic_callback(int conn_id,
                                                                   int status,
                                                                   uint16_t handle)
{
    BT_DBG_INFO(BT_DEBUG_GATT,"handle = %d", handle);
    bluetooth_gattc_write_characteristic_cbk(conn_id, status, handle);
}

static void linuxbt_gattc_read_descriptor_callback(int conn_id,
                                                              int status,
                                                              btgatt_read_params_t *p_data)
{
    BT_GATTC_READ_PARAMS_T read_params_data;
    memset(&read_params_data, 0 , sizeof(BT_GATTC_READ_PARAMS_T));
    read_params_data.handle = p_data->handle;
    read_params_data.status = p_data->status;
    read_params_data.value_type = p_data->value_type;
    read_params_data.value.len = p_data->value.len;
    memcpy(read_params_data.value.value, p_data->value.value, p_data->value.len);
    BT_DBG_INFO(BT_DEBUG_GATT,"handle = %d", read_params_data.handle);
    BT_DBG_INFO(BT_DEBUG_GATT,"status = %d", read_params_data.status);
    BT_DBG_INFO(BT_DEBUG_GATT,"value_type = %d", read_params_data.value_type);
    BT_DBG_INFO(BT_DEBUG_GATT,"value len = %d", read_params_data.value.len);
    BT_DBG_INFO(BT_DEBUG_GATT,"value = %s", read_params_data.value.value);
    bluetooth_gattc_read_descriptor_cbk(conn_id, status, &read_params_data);
}

static void linuxbt_gattc_write_descriptor_callback(int conn_id,
                                                               int status,
                                                               uint16_t handle)
{
    BT_DBG_INFO(BT_DEBUG_GATT,"handle = %d", handle);
    bluetooth_gattc_write_descriptor_cbk(conn_id, status, handle);
}

static void linuxbt_gattc_execute_write_callback(int conn_id, int status)
{
    BT_DBG_INFO(BT_DEBUG_GATT, "execute write status = %d",status);
}

static void linuxbt_gattc_read_remote_rssi_callback(int client_if,
                                                                bt_bdaddr_t* bda,
                                                                int rssi,
                                                                int status)
{
    BT_DBG_INFO(BT_DEBUG_GATT, "read %02X:%02X:%02X:%02X:%02X:%02X\n rssi = %d , status = %d",
                bda->address[0], bda->address[1], bda->address[2],
                bda->address[3], bda->address[4], bda->address[5],rssi,status);
    char btaddr_s[MAX_BDADDR_LEN];
    linuxbt_btaddr_htos(bda, btaddr_s);
    bluetooth_gattc_read_remote_rssi_cbk(client_if, btaddr_s, rssi, status);
}

static void linuxbt_gattc_configure_mtu_callback(int conn_id,
                                                            int status,
                                                            int mtu)
{
    BT_DBG_INFO(BT_DEBUG_GATT,"configure mtu = %d, status = %d",mtu,status);
}

static void linuxbt_gattc_scan_filter_cfg_callback(int action,
                                                            int client_if,
                                                            int status,
                                                            int filt_type,
                                                            int avbl_space)
{
    BT_DBG_INFO(BT_DEBUG_GATT,"scan_filter_cfg action = %d, client_if = %d, filt_type = %d, status = %d, avbl_space = %d",
                action,client_if,filt_type,status,avbl_space);
    bluetooth_gattc_scan_filter_cfg_cbk(action, client_if, status, filt_type, avbl_space);
}

static void linuxbt_gattc_scan_filter_param_callback(int action,
                                                                int client_if,
                                                                int status,
                                                                int avbl_space)
{
    BT_DBG_INFO(BT_DEBUG_GATT,"scan_filter_param action = %d, status = %d, avbl_space = %d",
                action, status, avbl_space);
    bluetooth_gattc_scan_filter_param_cbk(action, client_if, status, avbl_space);
}

static void linuxbt_gattc_scan_filter_status_callback(int enable,
                                                                int client_if,
                                                                int status)
{
    BT_DBG_INFO(BT_DEBUG_GATT,"scan_filter_status %d ,status = %d",enable,status);
    bluetooth_gattc_scan_filter_status_cbk(enable, client_if, status);
}

static void linuxbt_gattc_multi_adv_enable_callback(int client_if,
                                                                int status)
{
    BT_DBG_INFO(BT_DEBUG_GATT,"status = %d",status);
    bluetooth_gattc_multi_adv_enable_cbk(client_if, status);
}

static void linuxbt_gattc_multi_adv_update_callback(int client_if,
                                                                int status)
{
    BT_DBG_INFO(BT_DEBUG_GATT, "status = %d",status);
}

static void linuxbt_gattc_multi_adv_data_callback(int client_if,
                                                             int status)
{
    BT_DBG_INFO(BT_DEBUG_GATT,"status = %d",status);
    bluetooth_gattc_multi_adv_data_cbk(client_if, status);
}

static void linuxbt_gattc_multi_adv_disable_callback(int client_if,
                                                                 int status)
{
    BT_DBG_INFO(BT_DEBUG_GATT,"status = %d",status);
}

static void linuxbt_gattc_batchscan_cfg_storage_callback(int client_if,
                                                                       int status)
{
    BT_DBG_INFO(BT_DEBUG_GATT,"status = %d",status);
}

static void linuxbt_gattc_batchscan_enable_disable_callback(int action,
                                                                           int client_if,
                                                                           int status)
{
    BT_DBG_INFO(BT_DEBUG_GATT,"action= %d, status = %d", action, status);
}

static void linuxbt_gattc_batchscan_reports_callback(int client_if,
                                                                 int status,
                                                                 int report_format,
                                                                 int num_records,
                                                                 int data_len,
                                                                 uint8_t* rep_data)
{
    BT_DBG_INFO(BT_DEBUG_GATT,"report_format= %d, status = %d, num_records = %d, data_len = %d",
                report_format, status, num_records, data_len);
    for (int i = 0; i < data_len; i += 6)
    {
        if ((data_len - i) > 6)
        {
            BT_DBG_INFO(BT_DEBUG_GATT,"rep_data= %02X %02X %02X %02X %02X %02X ",
                rep_data[i], rep_data[i+1], rep_data[i+2], rep_data[i+3], rep_data[i+4], rep_data[i+5]);
        }
    }
}

static void linuxbt_gattc_batchscan_threshold_callback(int client_if)
{
    BT_DBG_INFO(BT_DEBUG_GATT,"client_if= %d", client_if);
}

static void linuxbt_gattc_scan_parameter_setup_completed_callback(int client_if,
    btgattc_error_t status)
{
    BT_DBG_INFO(BT_DEBUG_GATT,"client_if = %d, status = %d", client_if, status);
}

static void linuxbt_gattc_get_gatt_db_callback(int conn_id,
                                                         btgatt_db_element_t *db,
                                                         int count)
{
    BT_GATTC_DB_ELEMENT_T *gatt_db = malloc(count * sizeof(BT_GATTC_DB_ELEMENT_T));
    memset(gatt_db, 0, count * sizeof(BT_GATTC_DB_ELEMENT_T));
    BT_GATTC_DB_ELEMENT_T *curr_db_ptr = gatt_db;
    int i = 0;
    for (i = 0; i < count; i++)
    {
        curr_db_ptr->type = db->type;
        curr_db_ptr->attribute_handle = db->attribute_handle;
        curr_db_ptr->start_handle = db->start_handle;
        curr_db_ptr->end_handle = db->end_handle;
        curr_db_ptr->id = db->id;
        curr_db_ptr->properties = db->properties;
        linuxbt_print_uuid(&(db->uuid), curr_db_ptr->uuid);

        BT_DBG_INFO(BT_DEBUG_GATT,"type = %d, attribute_handle = %d",curr_db_ptr->type, curr_db_ptr->attribute_handle);
        BT_DBG_INFO(BT_DEBUG_GATT,"start_handle = %d, end_handle = %d",curr_db_ptr->start_handle, curr_db_ptr->end_handle);
        BT_DBG_INFO(BT_DEBUG_GATT,"id = %d, properties = %d",curr_db_ptr->id, curr_db_ptr->properties);
        BT_DBG_INFO(BT_DEBUG_GATT,"uuid = %s",curr_db_ptr->uuid);
        BT_DBG_INFO(BT_DEBUG_GATT,"\n");
        curr_db_ptr++;
        db++;
    }
    bluetooth_gattc_get_gatt_db_cbk(conn_id, gatt_db, count);
}

#if MTK_LINUX_GATTC_RPA
static void linuxbt_gattc_get_adv_rpa_callback(int client_if,
                                                          int status,
                                                          bt_bdaddr_t* rpa)
{
    BT_DBG_INFO(BT_DEBUG_GATT, "get_adv_rpa %02X:%02X:%02X:%02X:%02X:%02X\n , status : %d, client_if : %d",
                rpa->address[0], rpa->address[1], rpa->address[2],
                rpa->address[3], rpa->address[4], rpa->address[5],status, client_if);
}
#endif
