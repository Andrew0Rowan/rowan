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

#include "adaptor_log.h"
#include "mtk_bt_service_gattc_wrapper.h"
#include "bt_msg.h"
#include "bt_app.h"
#include "gatt_handler.h"
#include "gatts_handler.h"


extern BT_APP_GATTS_INFO gGattsInfo;
extern BT_APP_START_BLE_SERVER gBleSvrInfo;

static void bt_app_gattc_register_client_callback(BT_GATTC_REG_CLIENT_T *pt_reg_client_result ,
                                                                      void* pv_tag)
{
    BT_APP_MSG msg;
    BT_APP_GATTS_MSG_HDR gatts_msg;

    if (NULL == pt_reg_client_result) {
        APDATOR_LOG(ERROR,"pt_reg_client_result is NULL");
        return;
    }

    APDATOR_LOG(INFO,"client_if = %d uuid=%s",
                   pt_reg_client_result->client_if,pt_reg_client_result->uuid);

    gGattsInfo.client_if = pt_reg_client_result->client_if;

    msg.hdr.event = BT_APP_GATTS_EVENT;
    msg.hdr.len = sizeof(BT_APP_GATTS_MSG_HDR);
    gatts_msg.event = BT_APP_GATTS_REG_SERVER;
    memcpy((void*)&msg.data.gatts_msg, &gatts_msg, sizeof(BT_APP_GATTS_MSG_HDR));

    if (!bt_app_send_msg(&msg)) {
        APDATOR_LOG(ERROR,"bt_app_send_msg fail");
        return;
    }

    return;
}

static void bt_app_gattc_event_callback(BT_GATTC_EVENT_T bt_gatt_event,
                                                           void* pv_tag)
{
    APDATOR_LOG(INFO,"bt_gattc_event =%d", bt_gatt_event);
    return;
}

static void bt_app_gattc_scan_result_callback(BT_GATTC_SCAN_RST_T *pt_scan_result,
                                                                  void* pv_tag)
{
    if (NULL == pt_scan_result) {
        APDATOR_LOG(ERROR,"pt_scan_result is NULL");
        return;
    }
    APDATOR_LOG(INFO,"pt_scan_result->btaddr =%s",pt_scan_result->btaddr);

}

static void bt_app_gattc_get_gatt_db_callback(BT_GATTC_GET_GATT_DB_T *pt_get_gatt_db_result,
                                                                   void* pv_tag)
{
    BT_GATTC_DB_ELEMENT_T *curr_db_ptr = pt_get_gatt_db_result->gatt_db_element;
    int i = 0;

    APDATOR_LOG(INFO,"count =%d\n",pt_get_gatt_db_result->count);
    for (i = 0; i < pt_get_gatt_db_result->count; i++) {
        curr_db_ptr->type = pt_get_gatt_db_result->gatt_db_element->type;
        curr_db_ptr->attribute_handle = pt_get_gatt_db_result->gatt_db_element->attribute_handle;
        curr_db_ptr->start_handle = pt_get_gatt_db_result->gatt_db_element->start_handle;
        curr_db_ptr->end_handle = pt_get_gatt_db_result->gatt_db_element->end_handle;
        curr_db_ptr->id = pt_get_gatt_db_result->gatt_db_element->id;
        curr_db_ptr->properties = pt_get_gatt_db_result->gatt_db_element->properties;
        memcpy(pt_get_gatt_db_result->gatt_db_element->uuid, curr_db_ptr->uuid, BT_GATT_MAX_UUID_LEN);

        APDATOR_LOG(INFO,"type = %d, attribute_handle = %d",curr_db_ptr->type, curr_db_ptr->attribute_handle);
        APDATOR_LOG(INFO,"start_handle = %d, end_handle = %d",curr_db_ptr->start_handle, curr_db_ptr->end_handle);
        APDATOR_LOG(INFO,"id = %d, properties = %d\n",curr_db_ptr->id, curr_db_ptr->properties);
        APDATOR_LOG(INFO,"uuid = %s",curr_db_ptr->uuid);
        APDATOR_LOG(INFO,"\n");
        curr_db_ptr++;
        pt_get_gatt_db_result->gatt_db_element++;
    }
}

static void bt_app_gattc_get_reg_noti_callback(BT_GATTC_GET_REG_NOTI_RST_T *pt_get_reg_noti_result,
                                                                    void* pv_tag)
{
    APDATOR_LOG(INFO,"registered = %d, attribute_handle = %d",
                        pt_get_reg_noti_result->registered, pt_get_reg_noti_result->handle);
}

static void bt_app_gattc_notify_callback(BT_GATTC_GET_NOTIFY_T *pt_notify,
                                                           void* pv_tag)
{
    APDATOR_LOG(INFO,"handle = %d, bda = %s",pt_notify->notify_data.handle, pt_notify->notify_data.bda);
}

static void bt_app_gattc_read_char_callback(BT_GATTC_READ_CHAR_RST_T *pt_read_char,
                                                                void* pv_tag)
{
    APDATOR_LOG(INFO,"handle = %d, value = %s",pt_read_char->read_data.handle, pt_read_char->read_data.value.value);
}

static void bt_app_gattc_write_char_callback(BT_GATTC_WRITE_CHAR_RST_T *pt_write_char,
                                                                 void* pv_tag)
{
    APDATOR_LOG(INFO,"handle = %d",pt_write_char->handle);
}

static void bt_app_gattc_read_desc_callback(BT_GATTC_READ_DESCR_RST_T *pt_read_desc,
                                                                 void* pv_tag)
{
    APDATOR_LOG(INFO,"handle = %d, value = %s",
                       pt_read_desc->read_data.handle,
                       pt_read_desc->read_data.value.value);
}

static void bt_app_gattc_write_desc_callback(BT_GATTC_WRITE_DESCR_RST_T *pt_write_desc,
                                                                  void* pv_tag)
{
    APDATOR_LOG(INFO,"handle = %d",pt_write_desc->handle);
}

static void bt_app_gattc_scan_filter_param_callback(BT_GATTC_SCAN_FILTER_PARAM_T *pt_scan_filter_param,
                                                                          void* pv_tag)
{
    APDATOR_LOG(INFO,"action = %d", pt_scan_filter_param->action);
}

static void bt_app_gattc_scan_filter_status_callback(BT_GATTC_SCAN_FILTER_STATUS_T *pt_scan_filter_status,
                                                                          void* pv_tag)
{
    APDATOR_LOG(INFO,"enable = %d",pt_scan_filter_status->enable);
}

static void bt_app_gattc_scan_filter_cfg_callback(BT_GATTC_SCAN_FILTER_CFG_T *pt_scan_filter_cfg,
                                                                       void* pv_tag)
{
    APDATOR_LOG(INFO,"action = %d",pt_scan_filter_cfg->action);
}

static void bt_app_gattc_adv_enable_callback(int status, void* pv_tag)
{
    BT_APP_MSG msg;
    BT_APP_GATTC_MSG_HDR gattc_msg;

    APDATOR_LOG(INFO,"status = %d.",status);

    msg.hdr.event = BT_APP_GATTC_EVENT;
    msg.hdr.len = sizeof(BT_APP_GATTC_MSG_HDR);
    gattc_msg.event = BT_APP_GATTC_MULTI_ADV_SETDATA;
    memcpy((void*)&msg.data.gattc_msg, &gattc_msg, sizeof(BT_APP_GATTC_MSG_HDR));

    if (!bt_app_send_msg(&msg)) {
        APDATOR_LOG(ERROR,"bt_app_send_msg fail");
        return;
    }

    return;

}

BOOL bt_app_gattc_msg_handle(void *data) {

    BT_APP_MSG msg;
    BT_APP_GATTC_MSG_HDR *gattc_msg = (BT_APP_GATTC_MSG_HDR *)data;
    BT_APP_GATT_MSG_HDR gatt_msg;

    if (gattc_msg == NULL) {
        APDATOR_LOG(ERROR, "msg is NULL");
        return FALSE;
    }

    APDATOR_LOG(INFO, " event:%d", gattc_msg->event);

    switch(gattc_msg->event) {
        case BT_APP_GATTC_MULTI_ADV_ENABLE:
            APDATOR_LOG(INFO,"BT_APP_GATTC_MULTI_ADV_ENABLE:client_if=%d",gGattsInfo.client_if);
            if (a_mtkapi_bt_gattc_multi_adv_enable(gGattsInfo.client_if,(1000*1000/625),(1000*1000/625),BTA_BLE_CONNECT_EVT,
        (BTA_BLE_ADV_CHNL_37|BTA_BLE_ADV_CHNL_38|BTA_BLE_ADV_CHNL_39),BTA_BLE_ADV_TX_POWER_LOW,0) == -1) {
                APDATOR_LOG(ERROR,"multi adv enable fail.");
                msg.hdr.event = BT_APP_GATT_EVENT;
                msg.hdr.len = sizeof(BT_APP_GATT_MSG_HDR);
                gatt_msg.event = BT_APP_GATT_START_SERVER_FAIL;
                memcpy((void*)&msg.data.gatt_msg, &gatt_msg, sizeof(BT_APP_GATTS_MSG_HDR));

                if (!bt_app_send_msg(&msg)) {
                    APDATOR_LOG(ERROR,"bt_app_send_msg fail");
                    return FALSE;
                }
            }
            break;

        case BT_APP_GATTC_REG_CLIENT:
            APDATOR_LOG(INFO,"BT_APP_GATTC_REG_CLIENT:app_uuid=%s",gBleSvrInfo.app_uuid);
            if (a_mtkapi_bt_gattc_register_app(gBleSvrInfo.app_uuid) == -1) {
                APDATOR_LOG(ERROR,"register client fail.");
                msg.hdr.event = BT_APP_GATT_EVENT;
                msg.hdr.len = sizeof(BT_APP_GATT_MSG_HDR);
                gatt_msg.event = BT_APP_GATT_START_SERVER_FAIL;
                memcpy((void*)&msg.data.gatt_msg, &gatt_msg, sizeof(BT_APP_GATTS_MSG_HDR));

                if (!bt_app_send_msg(&msg)) {
                    APDATOR_LOG(ERROR,"bt_app_send_msg fail");
                    return FALSE;
                }
            }
            break;

        case BT_APP_GATTC_MULTI_ADV_SETDATA:
            APDATOR_LOG(INFO,"BT_APP_GATTC_MULTI_ADV_SETDATA:client_if=%d",gGattsInfo.client_if);
            if (a_mtkapi_bt_gattc_multi_adv_setdata(gGattsInfo.client_if,FALSE,TRUE,FALSE,0,0,
                                             NULL,0,NULL,strlen(gBleSvrInfo.service_uuid)
                                             ,gBleSvrInfo.service_uuid) == -1) {
                APDATOR_LOG(ERROR,"multi adv setdata fail.");
                msg.hdr.event = BT_APP_GATT_EVENT;
                msg.hdr.len = sizeof(BT_APP_GATT_MSG_HDR);
                gatt_msg.event = BT_APP_GATT_START_SERVER_FAIL;
                memcpy((void*)&msg.data.gatt_msg, &gatt_msg, sizeof(BT_APP_GATTS_MSG_HDR));

                if (!bt_app_send_msg(&msg)) {
                    APDATOR_LOG(ERROR,"bt_app_send_msg fail");
                    return FALSE;
                }
            } else {
                APDATOR_LOG(ERROR,"start server success.");
                msg.hdr.event = BT_APP_GATT_EVENT;
                msg.hdr.len = sizeof(BT_APP_GATT_MSG_HDR);
                gatt_msg.event = BT_APP_GATT_START_SERVER_SUCCESS;
                memcpy((void*)&msg.data.gatt_msg, &gatt_msg, sizeof(BT_APP_GATTS_MSG_HDR));

                if (!bt_app_send_msg(&msg)) {
                    APDATOR_LOG(ERROR,"bt_app_send_msg fail");
                    return FALSE;
                }
            }
            break;

        default:
            APDATOR_LOG(ERROR, " unknown event:%d", gattc_msg->event);
            break;
    }

    return TRUE;
}

BOOL bt_app_gattc_init() {
    MTKRPCAPI_BT_APP_GATTC_CB_FUNC_T func;

    memset(&func, 0, sizeof(MTKRPCAPI_BT_APP_GATTC_CB_FUNC_T));
    func.bt_gattc_reg_client_cb = bt_app_gattc_register_client_callback;
    func.bt_gattc_event_cb = bt_app_gattc_event_callback;
    func.bt_gattc_scan_cb = bt_app_gattc_scan_result_callback;
    func.bt_gattc_get_gatt_db_cb = bt_app_gattc_get_gatt_db_callback;
    func.bt_gattc_get_reg_noti_cb = bt_app_gattc_get_reg_noti_callback;
    func.bt_gattc_notify_cb = bt_app_gattc_notify_callback;
    func.bt_gattc_read_char_cb = bt_app_gattc_read_char_callback;
    func.bt_gattc_write_char_cb = bt_app_gattc_write_char_callback;
    func.bt_gattc_read_desc_cb = bt_app_gattc_read_desc_callback;
    func.bt_gattc_write_desc_cb = bt_app_gattc_write_desc_callback;
    func.bt_gattc_scan_filter_param_cb = bt_app_gattc_scan_filter_param_callback;
    func.bt_gattc_scan_filter_status_cb = bt_app_gattc_scan_filter_status_callback;
    func.bt_gattc_scan_filter_cfg_cb = bt_app_gattc_scan_filter_cfg_callback;
    func.bt_gattc_adv_enable_cb = bt_app_gattc_adv_enable_callback;

    if (a_mtkapi_bt_gattc_base_init(&func, NULL) == -1){
        APDATOR_LOG(ERROR,"bt gattc init fail.");
        return FALSE;
    }

    APDATOR_LOG(INFO,"bt gattc init success.");

    return TRUE;
}

