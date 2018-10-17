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
#include "mtk_bt_service_gatts_wrapper.h"
#include "bt_msg.h"
#include "bt_app.h"
#include "gatt_handler.h"
#include "gatts_handler.h"
#include "adaptor.h"
#include "adaptor_cbk.h"

BT_APP_GATTS_INFO gGattsInfo;
extern BT_APP_START_BLE_SERVER gBleSvrInfo;

static void bt_app_gatts_register_server_callback(BT_GATTS_REG_SERVER_RST_T *bt_gatts_reg_server,
                                                                       void* pv_tag)
{
    BT_APP_MSG msg;
    BT_APP_GATTS_MSG_HDR gatts_msg;

    if (NULL == bt_gatts_reg_server) {
        APDATOR_LOG(ERROR,"bt_gatts_reg_server is NULL");
        return;
    }

    APDATOR_LOG(INFO,"server_if = %d app_uuid=%s",bt_gatts_reg_server->server_if,
        bt_gatts_reg_server->app_uuid);

    gGattsInfo.server_if = bt_gatts_reg_server->server_if;

    msg.hdr.event = BT_APP_GATTS_EVENT;
    msg.hdr.len = sizeof(BT_APP_GATTS_MSG_HDR);
    gatts_msg.event = BT_APP_GATTS_ADD_SRVC;
    memcpy((void*)&msg.data.gatts_msg, &gatts_msg, sizeof(BT_APP_GATTS_MSG_HDR));

    if (!bt_app_send_msg(&msg)) {
        APDATOR_LOG(ERROR,"bt_app_send_msg fail");
        return;
    }

    return;
}

static void bt_app_gatts_event_callback(BT_GATTS_EVENT_T bt_gatts_event,
                                                           void* pv_tag)
{
    BT_APP_MSG msg;
    BT_APP_GATTS_MSG_HDR gatts_msg;

    APDATOR_LOG(INFO,"bt_gatts_event =%d", bt_gatts_event);

    if (bt_gatts_event == BT_GATTS_CONNECT) {
        gGattsInfo.isConnected = TRUE;

        msg.hdr.event = BT_APP_GATTS_EVENT;
        msg.hdr.len = sizeof(BT_APP_GATTS_MSG_HDR);
        gatts_msg.event = BT_APP_GATTS_GET_CONNECT_INFO;
        memcpy((void*)&msg.data.gatts_msg, &gatts_msg, sizeof(BT_APP_GATTS_MSG_HDR));
        
        if (TRUE != bt_app_send_msg(&msg)) {
            APDATOR_LOG(ERROR,"bt_app_send_msg fail");
            return;
        }
    } else if (bt_gatts_event == BT_GATTS_DISCONNECT) {
        gGattsInfo.isConnected = FALSE;
        gGattsInfo.connId = 0;
        memset(gGattsInfo.btAddr,0,BT_APP_MAC_lEN);
    }

    return;
}

static void bt_app_gatts_add_srvc_callback(BT_GATTS_ADD_SRVC_RST_T *bt_gatts_add_srvc,
                                                               void* pv_tag)
{
    BT_APP_MSG msg;
    BT_APP_GATTS_MSG_HDR gatts_msg;

    if (NULL == bt_gatts_add_srvc) {
        APDATOR_LOG(ERROR,"bt_gatts_add_srvc is NULL\n");
        return;
    }

    APDATOR_LOG(INFO,"srvc_handle =%d, uuid = %s, is_primary=%d, server_if=%d",
        bt_gatts_add_srvc->srvc_handle, bt_gatts_add_srvc->srvc_id.id.uuid,
        bt_gatts_add_srvc->srvc_id.is_primary,bt_gatts_add_srvc->server_if);

    gGattsInfo.servHandle = bt_gatts_add_srvc->srvc_handle;

    msg.hdr.event = BT_APP_GATTS_EVENT;
    msg.hdr.len = sizeof(BT_APP_GATTS_MSG_HDR);
    gatts_msg.event = BT_APP_GATTS_ADD_CHAR;
    memcpy((void*)&msg.data.gatts_msg, &gatts_msg, sizeof(BT_APP_GATTS_MSG_HDR));

    if (!bt_app_send_msg(&msg)) {
        APDATOR_LOG(ERROR,"bt_app_send_msg fail");
        return;
    }

    return;
}

static void bt_app_gatts_add_incl_callback(BT_GATTS_ADD_INCL_RST_T *bt_gatts_add_incl,
                                                              void* pv_tag)
{
    if (NULL == bt_gatts_add_incl) {
        APDATOR_LOG(ERROR,"bt_gatts_add_incl is NULL");
        return;
    }

    APDATOR_LOG(INFO,"bt_gatts_add_incl->incl_srvc_handle =%d",bt_gatts_add_incl->incl_srvc_handle);
}

static void bt_app_gatts_add_char_callback(BT_GATTS_ADD_CHAR_RST_T *bt_gatts_add_char,
                                                               void* pv_tag)
{
    BT_APP_MSG msg;
    BT_APP_GATTS_MSG_HDR gatts_msg;

    if (NULL == bt_gatts_add_char) {
        APDATOR_LOG(ERROR,"bt_gatts_add_char is NULL");
        return;
    }

    APDATOR_LOG(INFO,"char_handle =%d, uuid = %s server_if=%d srvc_handle=%d",
        bt_gatts_add_char->char_handle,
        bt_gatts_add_char->uuid,bt_gatts_add_char->server_if,
        bt_gatts_add_char->srvc_handle);

    gGattsInfo.charHandle = bt_gatts_add_char->char_handle;

    msg.hdr.event = BT_APP_GATTS_EVENT;
    msg.hdr.len = sizeof(BT_APP_GATTS_MSG_HDR);
    gatts_msg.event = BT_APP_GATTS_ADD_DESC;
    memcpy((void*)&msg.data.gatts_msg, &gatts_msg, sizeof(BT_APP_GATTS_MSG_HDR));

    if (!bt_app_send_msg(&msg)) {
        APDATOR_LOG(ERROR,"bt_app_send_msg fail");
        return;
    }

    return;
}

static void bt_app_gatts_add_desc_callback(BT_GATTS_ADD_DESCR_RST_T *bt_gatts_add_desc,
                                                                void* pv_tag)
{
    BT_APP_MSG msg;
    BT_APP_GATTS_MSG_HDR gatts_msg;

    if (NULL == bt_gatts_add_desc) {
        APDATOR_LOG(ERROR,"bt_gatts_add_char is NULL");
        return;
    }

    APDATOR_LOG(INFO,"descr_handle =%d, uuid = %s server_if=%d srvc_handle=%d",
        bt_gatts_add_desc->descr_handle, bt_gatts_add_desc->uuid,
        bt_gatts_add_desc->server_if,bt_gatts_add_desc->srvc_handle);

    gGattsInfo.descHandle = bt_gatts_add_desc->descr_handle;

    msg.hdr.event = BT_APP_GATTS_EVENT;
    msg.hdr.len = sizeof(BT_APP_GATTS_MSG_HDR);
    gatts_msg.event = BT_APP_GATTS_START_SRVC;
    memcpy((void*)&msg.data.gatts_msg, &gatts_msg, sizeof(BT_APP_GATTS_MSG_HDR));

    if (!bt_app_send_msg(&msg)) {
        APDATOR_LOG(ERROR,"bt_app_send_msg fail");
        return;
    }

}

static void bt_app_gatts_op_srvc_callback(BT_GATTS_SRVC_OP_TYPE_T op_type,
                                                              BT_GATTS_SRVC_RST_T *bt_gatts_srvc,
                                                              void* pv_tag)
{
    if (NULL == bt_gatts_srvc) {
        APDATOR_LOG(ERROR,"bt_gatts_srvc is NULL");
        return;
    }

    APDATOR_LOG(INFO,"op_type =%d, srvc_handle = %d",op_type,bt_gatts_srvc->srvc_handle);
}

static void bt_app_gatts_req_read_callback(BT_GATTS_REQ_READ_RST_T *bt_gatts_read,
                                                               void* pv_tag)
{
    BT_APP_MSG msg;
    BT_APP_GATTS_MSG_HDR gatts_msg;

    if (NULL == bt_gatts_read) {
        APDATOR_LOG(ERROR,"bt_gatts_read is NULL");
        return;
    }
    APDATOR_LOG(INFO,"attr_handle =%d, offset=%d is_long = %d, btaddr = %s",bt_gatts_read->attr_handle,
                       bt_gatts_read->offset,bt_gatts_read->is_long, bt_gatts_read->btaddr);

    msg.hdr.event = BT_APP_GATTS_EVENT;
    msg.hdr.len = sizeof(BT_APP_GATTS_MSG_HDR);
    gatts_msg.event = BT_APP_GATTS_REQ_READ;
    memcpy((void*)&msg.data.gatts_msg, &gatts_msg, sizeof(BT_APP_GATTS_MSG_HDR));
    
    if (!bt_app_send_msg(&msg)) {
        APDATOR_LOG(ERROR,"bt_app_send_msg fail");
        return;
    }
}

static void bt_app_gatts_req_write_callback(BT_GATTS_REQ_WRITE_RST_T *bt_gatts_write,
                                                                void* pv_tag)
{
    BT_APP_MSG msg;
    BT_APP_GATTS_MSG_HDR gatts_msg;
    int i = 0;

    if (NULL == bt_gatts_write) {
        APDATOR_LOG(ERROR,"bt_gatts_write is NULL\n");
        return;
    }
    APDATOR_LOG(INFO,"attr_handle =%d, offset=%d , trans_id=%d need_rsp=%d is_prep = %d, btaddr = %s value=%s length=%d",
        bt_gatts_write->attr_handle, bt_gatts_write->offset, bt_gatts_write->trans_id, bt_gatts_write->need_rsp, bt_gatts_write->is_prep,
        bt_gatts_write->btaddr, bt_gatts_write->value,bt_gatts_write->length);

    for (i=0;i<bt_gatts_write->length;i++)
       APDATOR_LOG(INFO,"0x%x\n",bt_gatts_write->value[i]);

    if (gAdaptorReportEvent) {
        gAdaptorReportEvent(BSP_BLE_SERVER_RECV,bt_gatts_write->value,bt_gatts_write->length);
    }

    if (bt_gatts_write->need_rsp) {
        msg.hdr.event = BT_APP_GATTS_EVENT;
        msg.hdr.len = sizeof(BT_APP_GATTS_MSG_HDR);
        gatts_msg.event = BT_APP_GATTS_REQ_WRITE;
        gatts_msg.data.writeCb_msg.attr_handle = bt_gatts_write->attr_handle;
        gatts_msg.data.writeCb_msg.trans_id = bt_gatts_write->trans_id;
        gatts_msg.data.writeCb_msg.conn_id = bt_gatts_write->conn_id;
        memcpy((void*)&msg.data.gatts_msg, &gatts_msg, sizeof(BT_APP_GATTS_MSG_HDR));
        
        if (!bt_app_send_msg(&msg)) {
            APDATOR_LOG(ERROR,"bt_app_send_msg fail");
            return;
        }
    }

}

static void bt_app_gatts_ind_sent_callback(int conn_id,
                                                               int status,
                                                               void* pv_tag)
{
    APDATOR_LOG(INFO,"status =%d, conn_id = %d",status, conn_id);
}

BOOL bt_app_gatts_send_indication(char *data,int length) {

    if (data == NULL) {
        APDATOR_LOG(ERROR, "data is NULL");
        return FALSE;
    }

    if (a_mtkapi_bt_gatts_send_indication(gGattsInfo.server_if, gGattsInfo.charHandle, 
               gGattsInfo.connId, 0, data, length) == -1) {
        APDATOR_LOG(ERROR, "send indication fail");
        return FALSE;
    }

    return TRUE;
}

BOOL bt_app_gatts_msg_handle(void *data) {

    BT_APP_MSG msg;
    BT_APP_GATTC_MSG_HDR gattc_msg;
    BT_APP_GATTS_MSG_HDR *gatts_msg = (BT_APP_GATTS_MSG_HDR *)data;
    BT_APP_GATT_MSG_HDR gatt_msg;
    BT_GATTS_CONNECT_RST_T connect_rst_info;
    char readData[]="1234";

    if (gatts_msg == NULL) {
        APDATOR_LOG(ERROR, "msg is NULL");
        return FALSE;
    }

    APDATOR_LOG(INFO, " event:%d", gatts_msg->event);

    switch(gatts_msg->event) {
        case BT_APP_GATTS_REG_SERVER:
            APDATOR_LOG(INFO,"BT_APP_GATTS_REG_SERVER:server_uuid=%s",gBleSvrInfo.server_uuid);
            if (a_mtkapi_bt_gatts_register_server(gBleSvrInfo.server_uuid) == -1) {
                APDATOR_LOG(ERROR,"register server fail.");
                msg.hdr.event = BT_APP_GATT_EVENT;
                msg.hdr.len = sizeof(BT_APP_GATT_MSG_HDR);
                gatt_msg.event = BT_APP_GATT_START_SERVER_FAIL;
                memcpy((void*)&msg.data.gatt_msg, &gatt_msg, sizeof(BT_APP_GATTS_MSG_HDR));

                if (!bt_app_send_msg(&msg)) {
                    APDATOR_LOG(ERROR,"bt_app_send_msg fail");
                    return FALSE;
                }
            }
            APDATOR_LOG(INFO,"register server success.");
            break;

        case BT_APP_GATTS_ADD_SRVC:
            APDATOR_LOG(INFO,"BT_APP_GATTS_ADD_SRVC:server_if=%d service_uuid=%s",
                  gGattsInfo.server_if,gBleSvrInfo.service_uuid);
            if (a_mtkapi_bt_gatts_add_service(gGattsInfo.server_if,
                         gBleSvrInfo.service_uuid,TRUE,20) == -1) {
                APDATOR_LOG(ERROR,"add service fail.");
                msg.hdr.event = BT_APP_GATT_EVENT;
                msg.hdr.len = sizeof(BT_APP_GATT_MSG_HDR);
                gatt_msg.event = BT_APP_GATT_START_SERVER_FAIL;
                memcpy((void*)&msg.data.gatt_msg, &gatt_msg, sizeof(BT_APP_GATTS_MSG_HDR));

                if (!bt_app_send_msg(&msg)) {
                    APDATOR_LOG(ERROR,"bt_app_send_msg fail");
                    return FALSE;
                }
            }
            APDATOR_LOG(INFO,"add service success.");
            break;

        case BT_APP_GATTS_ADD_CHAR:
            APDATOR_LOG(INFO,"BT_APP_GATTS_ADD_CHAR:server_if=%d char_uuid=%s",
                  gGattsInfo.server_if,gBleSvrInfo.char_uuid);
            if (a_mtkapi_bt_gatts_add_char(gGattsInfo.server_if,
                          gGattsInfo.servHandle,gBleSvrInfo.char_uuid,0x1e,0x11) == -1) {
                APDATOR_LOG(ERROR,"add charactristic fail.");
                msg.hdr.event = BT_APP_GATT_EVENT;
                msg.hdr.len = sizeof(BT_APP_GATT_MSG_HDR);
                gatt_msg.event = BT_APP_GATT_START_SERVER_FAIL;
                memcpy((void*)&msg.data.gatt_msg, &gatt_msg, sizeof(BT_APP_GATTS_MSG_HDR));

                if (!bt_app_send_msg(&msg)) {
                    APDATOR_LOG(ERROR,"bt_app_send_msg fail");
                    return FALSE;
                }
            }
            APDATOR_LOG(INFO,"add charactristic success.");
            break;

        case BT_APP_GATTS_ADD_DESC:
            APDATOR_LOG(INFO,"BT_APP_GATTS_ADD_DESC:server_if=%d servHandle=%d,desc_uuid=%s",
                  gGattsInfo.server_if,gGattsInfo.servHandle,gBleSvrInfo.desc_uuid);
            if (a_mtkapi_bt_gatts_add_desc(gGattsInfo.server_if,
                          gGattsInfo.servHandle,gBleSvrInfo.desc_uuid,0x11) == -1) {
                APDATOR_LOG(ERROR,"add descriptor fail.");
                msg.hdr.event = BT_APP_GATT_EVENT;
                msg.hdr.len = sizeof(BT_APP_GATT_MSG_HDR);
                gatt_msg.event = BT_APP_GATT_START_SERVER_FAIL;
                memcpy((void*)&msg.data.gatt_msg, &gatt_msg, sizeof(BT_APP_GATTS_MSG_HDR));
                
                if (!bt_app_send_msg(&msg)) {
                     APDATOR_LOG(ERROR,"bt_app_send_msg fail");
                     return FALSE;
                }
            }
            APDATOR_LOG(INFO,"add descriptor success.");
            break;

        case BT_APP_GATTS_GET_CONNECT_INFO:
            a_mtkapi_bt_gatts_get_connect_result_info(&connect_rst_info);
            gGattsInfo.connId = connect_rst_info.conn_id;
            memcpy(gGattsInfo.btAddr,connect_rst_info.btaddr,BT_APP_MAC_lEN);
            APDATOR_LOG(ERROR,"get connection info(conn_id=%d %02x:%02x:%02x:%02x:%02x:%02x).",
                            gGattsInfo.connId,gGattsInfo.btAddr[0],gGattsInfo.btAddr[1],
                            gGattsInfo.btAddr[2],gGattsInfo.btAddr[3],gGattsInfo.btAddr[4],
                            gGattsInfo.btAddr[5]);
            break;

         case BT_APP_GATTS_START_SRVC:
            APDATOR_LOG(INFO,"BT_APP_GATTS_START_SRVC:server_if=%d servHandle=%d",
                  gGattsInfo.server_if,gGattsInfo.servHandle);
             if (a_mtkapi_bt_gatts_start_service(gGattsInfo.server_if,
                          gGattsInfo.servHandle,0) == -1) {
                 APDATOR_LOG(ERROR,"start service fail.");
                 msg.hdr.event = BT_APP_GATT_EVENT;
                 msg.hdr.len = sizeof(BT_APP_GATT_MSG_HDR);
                 gatt_msg.event = BT_APP_GATT_START_SERVER_FAIL;
                 memcpy((void*)&msg.data.gatt_msg, &gatt_msg, sizeof(BT_APP_GATTS_MSG_HDR));
                 
                 if (!bt_app_send_msg(&msg)) {
                     APDATOR_LOG(ERROR,"bt_app_send_msg fail");
                     return FALSE;
                 }
             } else {
                 APDATOR_LOG(INFO,"start service success.");
                 msg.hdr.event = BT_APP_GATTC_EVENT;
                 msg.hdr.len = sizeof(BT_APP_GATT_MSG_HDR);
                 gattc_msg.event = BT_APP_GATTC_MULTI_ADV_ENABLE;
                 memcpy((void*)&msg.data.gattc_msg, &gattc_msg, sizeof(BT_APP_GATTC_MSG_HDR));
                 
                 if (!bt_app_send_msg(&msg)) {
                     APDATOR_LOG(ERROR,"bt_app_send_msg fail\n");
                     return FALSE;
                 }
             }
             break;

          case BT_APP_GATTS_REQ_READ:
              APDATOR_LOG(INFO,"receive read request.");
              if (a_mtkapi_bt_gatts_send_response(gGattsInfo.connId,
                      gGattsInfo.transId,0,gGattsInfo.charHandle,
                      readData,sizeof (readData),0) == -1) {
                  APDATOR_LOG(ERROR,"a_mtkapi_bt_gatts_send_response fail");
              }

          case BT_APP_GATTS_REQ_WRITE:
              APDATOR_LOG(INFO,"receive write request(conn_id=%d,trans_id=%d,attr_handle=%d)",
                                gatts_msg->data.writeCb_msg.conn_id,gatts_msg->data.writeCb_msg.trans_id,
                                gatts_msg->data.writeCb_msg.attr_handle);
              if (a_mtkapi_bt_gatts_send_response(gatts_msg->data.writeCb_msg.conn_id,
                      gatts_msg->data.writeCb_msg.trans_id,0,gatts_msg->data.writeCb_msg.attr_handle,
                      NULL,0,0) == -1) {
                  APDATOR_LOG(ERROR,"a_mtkapi_bt_gatts_send_response fail");
              }

              break;

        default:
            APDATOR_LOG(ERROR, " unknown event:%d", gatts_msg->event);
            break;
    }

    return TRUE;
}

BOOL bt_app_gatts_server_stop(void) {

    APDATOR_LOG(INFO,"stop gatt server(%d,%d,%02x:%02x:%02x:%02x:%02x:%02x,%d).\n",
              gGattsInfo.servHandle,gGattsInfo.server_if,gGattsInfo.btAddr[0],
              gGattsInfo.btAddr[1],gGattsInfo.btAddr[2],gGattsInfo.btAddr[3],
              gGattsInfo.btAddr[4],gGattsInfo.btAddr[5],gGattsInfo.connId);

    /*stop gatt server*/
    if (a_mtkapi_bt_gatts_close(gGattsInfo.server_if,gGattsInfo.btAddr,gGattsInfo.connId) == -1){
        APDATOR_LOG(ERROR,"stop gatt server fail.\n");
        return FALSE;
    }

    memset(&gGattsInfo,0,sizeof (gGattsInfo));

    return TRUE;
}

BOOL bt_app_gatts_init(void) {
    MTKRPCAPI_BT_APP_GATTS_CB_FUNC_T func;

    memset(&func, 0, sizeof(MTKRPCAPI_BT_APP_GATTS_CB_FUNC_T));
    func.bt_gatts_reg_server_cb = bt_app_gatts_register_server_callback;
    func.bt_gatts_event_cb = bt_app_gatts_event_callback;
    func.bt_gatts_add_srvc_cb = bt_app_gatts_add_srvc_callback;
    func.bt_gatts_add_incl_cb = bt_app_gatts_add_incl_callback;
    func.bt_gatts_add_char_cb = bt_app_gatts_add_char_callback;
    func.bt_gatts_add_desc_cb = bt_app_gatts_add_desc_callback;
    func.bt_gatts_op_srvc_cb = bt_app_gatts_op_srvc_callback;
    func.bt_gatts_req_read_cb = bt_app_gatts_req_read_callback;
    func.bt_gatts_req_write_cb = bt_app_gatts_req_write_callback;
    func.bt_gatts_ind_sent_cb = bt_app_gatts_ind_sent_callback;

    if (a_mtkapi_bt_gatts_base_init(&func, NULL) == -1){
        APDATOR_LOG(ERROR,"bt gatts init fail.");
        return FALSE;
    }

    memset(&gGattsInfo,0,sizeof (gGattsInfo));

    APDATOR_LOG(INFO,"bt gatts init success.");

    return TRUE;
}

