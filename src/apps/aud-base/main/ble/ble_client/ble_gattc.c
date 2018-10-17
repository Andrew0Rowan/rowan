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
#include <string.h>

#include "mtk_bt_service_gattc_wrapper.h"

#include "u_common.h"

#include "ble.h"
#include "ble_msg.h"
#include "ble_gatt.h"
#include "ble_gatts.h"

static VOID ble_gattc_register_client_callback(BT_GATTC_REG_CLIENT_T *pt_reg_client_result ,
                                                                      VOID* pv_tag)
{
    BLE_MSG msg;
    BLE_GATT_SERVER_MSG *pBleSvrMsg = NULL;

    if (NULL == pt_reg_client_result) {
        BLE_LOG(BLE_LOG_ERR,"pt_reg_client_result is NULL");
        return;
    }

    BLE_LOG(BLE_LOG_INFO,"client_if = %d uuid=%s",
                   pt_reg_client_result->client_if,pt_reg_client_result->uuid);

    gGattsMwInfo.client_if = pt_reg_client_result->client_if;

    /*add server*/
    msg.hdr.event = BLE_SERVER_EVENT;
    msg.hdr.len = sizeof(BLE_GATT_SERVER_MSG) - BLE_GATT_SERVER_MSG_VALUE_MAX_lEN;
    pBleSvrMsg = (BLE_GATT_SERVER_MSG *)&msg.data;
    pBleSvrMsg->type = BLE_GATT_SERVER_START_MSG;
    pBleSvrMsg->event = BLE_GATT_SERVER_START_OP_REG_SERVER;

    if (ble_send_msg(&msg) != BLE_OK) {
        BLE_LOG(BLE_LOG_ERR,"ble_send_msg fail");
        return;
    }

    return;
}

static VOID ble_gattc_event_callback(BT_GATTC_EVENT_T bt_gatt_event,
                                                           VOID* pv_tag)
{
    BLE_LOG(BLE_LOG_INFO,"bt_gattc_event =%d", bt_gatt_event);
    return;
}

static VOID ble_gattc_scan_result_callback(BT_GATTC_SCAN_RST_T *pt_scan_result,
                                                                  VOID* pv_tag)
{
    if (NULL == pt_scan_result) {
        BLE_LOG(BLE_LOG_ERR,"pt_scan_result is NULL");
        return;
    }
    BLE_LOG(BLE_LOG_INFO,"pt_scan_result->btaddr =%s",pt_scan_result->btaddr);
    BLE_LOG(BLE_LOG_INFO,"adv_data =%s",pt_scan_result->adv_data);

}

static VOID ble_gattc_get_gatt_db_callback(BT_GATTC_GET_GATT_DB_T *pt_get_gatt_db_result,
                                                                   VOID* pv_tag)
{
    BT_GATTC_DB_ELEMENT_T *curr_db_ptr = pt_get_gatt_db_result->gatt_db_element;
    INT32 i = 0;

    BLE_LOG(BLE_LOG_INFO,"count =%d\n",pt_get_gatt_db_result->count);
    for (i = 0; i < pt_get_gatt_db_result->count; i++) {
        curr_db_ptr->type = pt_get_gatt_db_result->gatt_db_element->type;
        curr_db_ptr->attribute_handle = pt_get_gatt_db_result->gatt_db_element->attribute_handle;
        curr_db_ptr->start_handle = pt_get_gatt_db_result->gatt_db_element->start_handle;
        curr_db_ptr->end_handle = pt_get_gatt_db_result->gatt_db_element->end_handle;
        curr_db_ptr->id = pt_get_gatt_db_result->gatt_db_element->id;
        curr_db_ptr->properties = pt_get_gatt_db_result->gatt_db_element->properties;
        memcpy(pt_get_gatt_db_result->gatt_db_element->uuid, curr_db_ptr->uuid, BT_GATT_MAX_UUID_LEN);

        BLE_LOG(BLE_LOG_INFO,"type = %d, attribute_handle = %d",curr_db_ptr->type, curr_db_ptr->attribute_handle);
        BLE_LOG(BLE_LOG_INFO,"start_handle = %d, end_handle = %d",curr_db_ptr->start_handle, curr_db_ptr->end_handle);
        BLE_LOG(BLE_LOG_INFO,"id = %d, properties = %d\n",curr_db_ptr->id, curr_db_ptr->properties);
        BLE_LOG(BLE_LOG_INFO,"uuid = %s",curr_db_ptr->uuid);
        BLE_LOG(BLE_LOG_INFO,"\n");
        curr_db_ptr++;
        pt_get_gatt_db_result->gatt_db_element++;
    }
}

static VOID ble_gattc_get_reg_noti_callback(BT_GATTC_GET_REG_NOTI_RST_T *pt_get_reg_noti_result,
                                                                    VOID* pv_tag)
{
    BLE_LOG(BLE_LOG_INFO,"registered = %d, attribute_handle = %d",
                        pt_get_reg_noti_result->registered, pt_get_reg_noti_result->handle);
}

static VOID ble_gattc_notify_callback(BT_GATTC_GET_NOTIFY_T *pt_notify,
                                                           VOID* pv_tag)
{
    BLE_LOG(BLE_LOG_INFO,"handle = %d, bda = %s",pt_notify->notify_data.handle, pt_notify->notify_data.bda);
}

static VOID ble_gattc_read_char_callback(BT_GATTC_READ_CHAR_RST_T *pt_read_char,
                                                                VOID* pv_tag)
{
    BLE_LOG(BLE_LOG_INFO,"handle = %d, value = %s",pt_read_char->read_data.handle, pt_read_char->read_data.value.value);
}

static VOID ble_gattc_write_char_callback(BT_GATTC_WRITE_CHAR_RST_T *pt_write_char,
                                                                 VOID* pv_tag)
{
    BLE_LOG(BLE_LOG_INFO,"handle = %d",pt_write_char->handle);
}

static VOID ble_gattc_read_desc_callback(BT_GATTC_READ_DESCR_RST_T *pt_read_desc,
                                                                 VOID* pv_tag)
{
    BLE_LOG(BLE_LOG_INFO,"handle = %d, value = %s",
                       pt_read_desc->read_data.handle,
                       pt_read_desc->read_data.value.value);
}

static VOID ble_gattc_write_desc_callback(BT_GATTC_WRITE_DESCR_RST_T *pt_write_desc,
                                                                  VOID* pv_tag)
{
    BLE_LOG(BLE_LOG_INFO,"handle = %d",pt_write_desc->handle);
}

static VOID ble_gattc_scan_filter_param_callback(BT_GATTC_SCAN_FILTER_PARAM_T *pt_scan_filter_param,
                                                                          VOID* pv_tag)
{
    BLE_LOG(BLE_LOG_INFO,"action = %d", pt_scan_filter_param->action);
}

static VOID ble_gattc_scan_filter_status_callback(BT_GATTC_SCAN_FILTER_STATUS_T *pt_scan_filter_status,
                                                                          VOID* pv_tag)
{
    BLE_LOG(BLE_LOG_INFO,"enable = %d",pt_scan_filter_status->enable);
}

static VOID ble_gattc_scan_filter_cfg_callback(BT_GATTC_SCAN_FILTER_CFG_T *pt_scan_filter_cfg,
                                                                       VOID* pv_tag)
{
    BLE_LOG(BLE_LOG_INFO,"action = %d",pt_scan_filter_cfg->action);
}

static VOID ble_gattc_adv_enable_callback(INT32 status, VOID* pv_tag)
{
    BLE_MSG msg;
    BLE_GATT_SERVER_MSG *pBleSvrMsg = NULL;

    BLE_LOG(BLE_LOG_INFO,"status = %d.",status);

    msg.hdr.event = BLE_SERVER_EVENT;
    msg.hdr.len = sizeof(BLE_GATT_SERVER_MSG) - BLE_GATT_SERVER_MSG_VALUE_MAX_lEN;
    pBleSvrMsg = (BLE_GATT_SERVER_MSG *)&msg.data;
    pBleSvrMsg->type = BLE_GATT_SERVER_START_MSG;
    pBleSvrMsg->event = BLE_GATT_SERVER_START_OP_SET_ADVDATA;

    if (ble_send_msg(&msg) != BLE_OK) {
        BLE_LOG(BLE_LOG_ERR,"ble_send_msg fail");
        return;
    }

    return;

}

INT32 ble_gattc_start_scan(VOID) {

    if (a_mtkapi_bt_gattc_scan() == -1) {
        BLE_LOG(BLE_LOG_ERR,"ble_gattc_start_scan fail");
        return BLE_FAIL;
    } else {
        BLE_LOG(BLE_LOG_INFO,"ble_gattc_start_scan success");
    }

    return BLE_OK;
}

INT32 ble_gattc_stop_scan(VOID) {

    if (a_mtkapi_bt_gattc_stop_scan() == -1) {
        BLE_LOG(BLE_LOG_ERR,"a_mtkapi_bt_gattc_stop_scan fail");
        return BLE_FAIL;
    } else {
        BLE_LOG(BLE_LOG_INFO,"a_mtkapi_bt_gattc_stop_scan success");
    }

    return BLE_OK;
}

INT32 ble_gattc_adv_enable(VOID) {

    BLE_LOG(BLE_LOG_INFO,"client_if=%d",gGattsMwInfo.client_if);

    if (a_mtkapi_bt_gattc_multi_adv_enable(gGattsMwInfo.client_if,(1000*1000/625),
        (1000*1000/625),BTA_BLE_CONNECT_EVT,
        (BTA_BLE_ADV_CHNL_37|BTA_BLE_ADV_CHNL_38|BTA_BLE_ADV_CHNL_39),
        BTA_BLE_ADV_TX_POWER_LOW,0) == -1) {
        BLE_LOG(BLE_LOG_ERR,"bt gattc adv enable fail.");
        return BLE_FAIL;
    }

    BLE_LOG(BLE_LOG_INFO,"bt gattc adv enable success.");

    return BLE_OK;
}

INT32 ble_gattc_adv_set_data(CHAR *service_uuid) {

    if (service_uuid == NULL) {
        BLE_LOG(BLE_LOG_ERR,"service_uuid is NULL.");
        return BLE_FAIL;
    }

    BLE_LOG(BLE_LOG_INFO,"client_if=%d service_uuid=%s",
        gGattsMwInfo.client_if,service_uuid);

    if (a_mtkapi_bt_gattc_multi_adv_setdata(gGattsMwInfo.client_if,0,1,0,0,0,
                                             NULL,0,NULL,strlen(service_uuid)
                                             ,service_uuid) == -1) {
        BLE_LOG(BLE_LOG_ERR,"bt gattc adv setdata fail.");
        return BLE_FAIL;
    }

    BLE_LOG(BLE_LOG_INFO,"bt gattc adv setdata success.");

    return BLE_OK;
}

INT32 ble_gattc_init(VOID) {
    MTKRPCAPI_BT_APP_GATTC_CB_FUNC_T func;

    memset(&func, 0, sizeof(MTKRPCAPI_BT_APP_GATTC_CB_FUNC_T));
    func.bt_gattc_reg_client_cb = ble_gattc_register_client_callback;
    func.bt_gattc_event_cb = ble_gattc_event_callback;
#ifdef BT_GATT_TEST_SUPPORT
    func.bt_gattc_scan_cb = ble_test_gattc_scan_result_callback;
#else
    func.bt_gattc_scan_cb = ble_gattc_scan_result_callback;
#endif
    func.bt_gattc_get_gatt_db_cb = ble_gattc_get_gatt_db_callback;
    func.bt_gattc_get_reg_noti_cb = ble_gattc_get_reg_noti_callback;
    func.bt_gattc_notify_cb = ble_gattc_notify_callback;
    func.bt_gattc_read_char_cb = ble_gattc_read_char_callback;
    func.bt_gattc_write_char_cb = ble_gattc_write_char_callback;
    func.bt_gattc_read_desc_cb = ble_gattc_read_desc_callback;
    func.bt_gattc_write_desc_cb = ble_gattc_write_desc_callback;
    func.bt_gattc_scan_filter_param_cb = ble_gattc_scan_filter_param_callback;
    func.bt_gattc_scan_filter_status_cb = ble_gattc_scan_filter_status_callback;
    func.bt_gattc_scan_filter_cfg_cb = ble_gattc_scan_filter_cfg_callback;
    func.bt_gattc_adv_enable_cb = ble_gattc_adv_enable_callback;

    if (a_mtkapi_bt_gattc_base_init(&func, NULL) == -1){
        BLE_LOG(BLE_LOG_ERR,"bt gattc init fail.");
        return BLE_FAIL;
    }

    BLE_LOG(BLE_LOG_INFO,"bt gattc init success.");

    return BLE_OK;
}

INT32 ble_gattc_register(CHAR *app_uuid) {

    if (a_mtkapi_bt_gattc_register_app(app_uuid) == -1) {
        BLE_LOG(BLE_LOG_ERR,"bt gattc register fail.");
        return BLE_FAIL;
    }
    BLE_LOG(BLE_LOG_INFO,"bt gattc register success.");

    return BLE_OK;
}

INT32 ble_gattc_unregister(VOID) {

    if (a_mtkapi_bt_gattc_unregister_app(gGattsMwInfo.client_if) == -1){
        BLE_LOG(BLE_LOG_ERR,"bt gattc destroy fail.");
        return BLE_FAIL;
    }

    BLE_LOG(BLE_LOG_INFO,"bt gattc destroy success.");

    return BLE_OK;
}
