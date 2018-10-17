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

#include "mtk_bt_service_gatts_wrapper.h"

#include "u_common.h"

#include "ble.h"
#include "ble_msg.h"
#include "ble_gatt.h"
#include "ble_gatts.h"

static VOID ble_gatts_register_server_callback(BT_GATTS_REG_SERVER_RST_T *bt_gatts_reg_server,
                                                                       VOID* pv_tag)
{
    BLE_MSG msg;
    BLE_GATT_SERVER_MSG *pBleSvrMsg = NULL;

    if (NULL == bt_gatts_reg_server) {
        BLE_LOG(BLE_LOG_ERR,"bt_gatts_reg_server is NULL");
        return;
    }

    BLE_LOG(BLE_LOG_INFO,"server_if = %d app_uuid=%s",bt_gatts_reg_server->server_if,
        bt_gatts_reg_server->app_uuid);

    gGattsMwInfo.server_if = bt_gatts_reg_server->server_if;

    /*add service*/
    msg.hdr.event = BLE_SERVER_EVENT;
    msg.hdr.len = sizeof(BLE_GATT_SERVER_MSG) - BLE_GATT_SERVER_MSG_VALUE_MAX_lEN;
    pBleSvrMsg = (BLE_GATT_SERVER_MSG *)&msg.data;
    pBleSvrMsg->type = BLE_GATT_SERVER_START_MSG;
    pBleSvrMsg->event = BLE_GATT_SERVER_START_OP_ADD_SERVICE;

    if (ble_send_msg(&msg) != BLE_OK) {
        BLE_LOG(BLE_LOG_ERR,"ble_send_msg fail");
        return;
    }

    return;
}

static VOID ble_gatts_event_callback(BT_GATTS_EVENT_T bt_gatts_event,
                                                           VOID* pv_tag)
{
    BLE_MSG msg;
    BLE_GATT_SERVER_MSG *pBleSvrMsg = NULL;

    BLE_LOG(BLE_LOG_INFO,"bt_gatts_event =%d", bt_gatts_event);

    if (bt_gatts_event == BT_GATTS_CONNECT) {
        gGattsMwInfo.isConnected = TRUE;

        msg.hdr.event = BLE_SERVER_EVENT;
        msg.hdr.len = sizeof(BLE_GATT_SERVER_MSG) - BLE_GATT_SERVER_MSG_VALUE_MAX_lEN;
        pBleSvrMsg = (BLE_GATT_SERVER_MSG *)&msg.data;
        pBleSvrMsg->type = BLE_GATT_SERVER_MISC_MSG;
        pBleSvrMsg->event = BLE_GATT_SERVER_MISC_OP_GET_CONN_INFO;
        
        if (ble_send_msg(&msg) != BLE_OK) {
            BLE_LOG(BLE_LOG_ERR,"ble_send_msg fail");
            return;
        }
    } else if (bt_gatts_event == BT_GATTS_DISCONNECT) {
        gGattsMwInfo.isConnected = FALSE;
    }

    return;
}

static VOID ble_gatts_add_srvc_callback(BT_GATTS_ADD_SRVC_RST_T *bt_gatts_add_srvc,
                                                               VOID* pv_tag)
{
    BLE_MSG msg;
    BLE_GATT_SERVER_MSG *pBleSvrMsg = NULL;

    if (NULL == bt_gatts_add_srvc) {
        BLE_LOG(BLE_LOG_ERR,"bt_gatts_add_srvc is NULL\n");
        return;
    }

    BLE_LOG(BLE_LOG_INFO,"srvc_handle =%d, uuid = %s, is_primary=%d, server_if=%d",
        bt_gatts_add_srvc->srvc_handle, bt_gatts_add_srvc->srvc_id.id.uuid,
        bt_gatts_add_srvc->srvc_id.is_primary,bt_gatts_add_srvc->server_if);

    gGattsMwInfo.servHandle = bt_gatts_add_srvc->srvc_handle;

    msg.hdr.event = BLE_SERVER_EVENT;
    msg.hdr.len = sizeof(BLE_GATT_SERVER_MSG) - BLE_GATT_SERVER_MSG_VALUE_MAX_lEN;
    pBleSvrMsg = (BLE_GATT_SERVER_MSG *)&msg.data;
    pBleSvrMsg->type = BLE_GATT_SERVER_START_MSG;
    pBleSvrMsg->event = BLE_GATT_SERVER_START_OP_ADD_CHAR;

    if (ble_send_msg(&msg) != BLE_OK) {
        BLE_LOG(BLE_LOG_ERR,"ble_send_msg fail");
        return;
    }

    return;
}

static VOID ble_gatts_add_incl_callback(BT_GATTS_ADD_INCL_RST_T *bt_gatts_add_incl,
                                                              VOID* pv_tag)
{
    if (NULL == bt_gatts_add_incl) {
        BLE_LOG(BLE_LOG_ERR,"bt_gatts_add_incl is NULL");
        return;
    }

    BLE_LOG(BLE_LOG_INFO,"bt_gatts_add_incl->incl_srvc_handle =%d",bt_gatts_add_incl->incl_srvc_handle);
}

static VOID ble_gatts_add_char_callback(BT_GATTS_ADD_CHAR_RST_T *bt_gatts_add_char,
                                                               VOID* pv_tag)
{
    BLE_MSG msg;
    BLE_GATT_SERVER_MSG *pBleSvrMsg = NULL;

    if (NULL == bt_gatts_add_char) {
        BLE_LOG(BLE_LOG_ERR,"bt_gatts_add_CHAR is NULL");
        return;
    }

    BLE_LOG(BLE_LOG_INFO,"char_handle =%d, uuid = %s server_if=%d srvc_handle=%d",
        bt_gatts_add_char->char_handle,
        bt_gatts_add_char->uuid,bt_gatts_add_char->server_if,
        bt_gatts_add_char->srvc_handle);

    gGattsMwInfo.charHandle = bt_gatts_add_char->char_handle;

    msg.hdr.event = BLE_SERVER_EVENT;
    msg.hdr.len = sizeof(BLE_GATT_SERVER_MSG) - BLE_GATT_SERVER_MSG_VALUE_MAX_lEN;
    pBleSvrMsg = (BLE_GATT_SERVER_MSG *)&msg.data;
    pBleSvrMsg->type = BLE_GATT_SERVER_START_MSG;
    pBleSvrMsg->event = BLE_GATT_SERVER_START_OP_ADD_DESC;

    if (ble_send_msg(&msg) != BLE_OK) {
        BLE_LOG(BLE_LOG_ERR,"ble_send_msg fail");
        return;
    }

    return;
}

static VOID ble_gatts_add_desc_callback(BT_GATTS_ADD_DESCR_RST_T *bt_gatts_add_desc,
                                                                VOID* pv_tag)
{
    BLE_MSG msg;
    BLE_GATT_SERVER_MSG * pBleSvrMsg = NULL;

    if (NULL == bt_gatts_add_desc) {
        BLE_LOG(BLE_LOG_ERR,"bt_gatts_add_char is NULL");
        return;
    }

    BLE_LOG(BLE_LOG_INFO,"descr_handle =%d, uuid = %s server_if=%d srvc_handle=%d",
        bt_gatts_add_desc->descr_handle, bt_gatts_add_desc->uuid,
        bt_gatts_add_desc->server_if,bt_gatts_add_desc->srvc_handle);

    gGattsMwInfo.descHandle = bt_gatts_add_desc->descr_handle;

    msg.hdr.event = BLE_SERVER_EVENT;
    msg.hdr.len = sizeof(BLE_GATT_SERVER_MSG) - BLE_GATT_SERVER_MSG_VALUE_MAX_lEN;
    pBleSvrMsg = (BLE_GATT_SERVER_MSG *)&msg.data;
    pBleSvrMsg->type = BLE_GATT_SERVER_START_MSG;
    pBleSvrMsg->event = BLE_GATT_SERVER_START_OP_START_SERVER;

    if (ble_send_msg(&msg) != BLE_OK) {
        BLE_LOG(BLE_LOG_ERR,"ble_send_msg fail");
        return;
    }

}

static VOID ble_gatts_op_srvc_callback(BT_GATTS_SRVC_OP_TYPE_T op_type,
                                                              BT_GATTS_SRVC_RST_T *bt_gatts_srvc,
                                                              VOID* pv_tag)
{
    BLE_MSG msg;
    BLE_GATT_SERVER_MSG *pBleSvrMsg = NULL;

    if (NULL == bt_gatts_srvc) {
        BLE_LOG(BLE_LOG_ERR,"bt_gatts_srvc is NULL");
        return;
    }

    BLE_LOG(BLE_LOG_INFO,"op_type =%d, srvc_handle = %d",op_type,bt_gatts_srvc->srvc_handle);

    if (op_type == BT_GATTS_START_SRVC) {
        msg.hdr.event = BLE_SERVER_EVENT;
        msg.hdr.len = sizeof(BLE_GATT_SERVER_MSG) - BLE_GATT_SERVER_MSG_VALUE_MAX_lEN;
        pBleSvrMsg = (BLE_GATT_SERVER_MSG *)&msg.data;
        pBleSvrMsg->type = BLE_GATT_SERVER_START_MSG;
        pBleSvrMsg->event = BLE_GATT_SERVER_START_OP_ENBALE_ADV;
    } else if (op_type == BT_GATTS_STOP_SRVC) {
        msg.hdr.event = BLE_SERVER_EVENT;
        msg.hdr.len = sizeof(BLE_GATT_SERVER_MSG) - BLE_GATT_SERVER_MSG_VALUE_MAX_lEN;
        pBleSvrMsg = (BLE_GATT_SERVER_MSG *)&msg.data;
        pBleSvrMsg->type = BLE_GATT_SERVER_STOP_MSG;
        pBleSvrMsg->event = BLE_GATT_SERVER_STOP_OP_DELETET_SERVER;
    } else if (op_type == BT_GATTS_DEL_SRVC) {
        msg.hdr.event = BLE_SERVER_EVENT;
        msg.hdr.len = sizeof(BLE_GATT_SERVER_MSG) - BLE_GATT_SERVER_MSG_VALUE_MAX_lEN;
        pBleSvrMsg = (BLE_GATT_SERVER_MSG *)&msg.data;
        pBleSvrMsg->type = BLE_GATT_SERVER_STOP_MSG;
        pBleSvrMsg->event = BLE_GATT_SERVER_STOP_OP_UNREG;
    } else {
        BLE_LOG(BLE_LOG_ERR,"err op_type");
        return;
    }

    if (ble_send_msg(&msg) != BLE_OK) {
        BLE_LOG(BLE_LOG_ERR,"ble_send_msg fail");
        return;
    }
}

static VOID ble_gatts_req_read_callback(BT_GATTS_REQ_READ_RST_T *bt_gatts_read,
                                                               VOID* pv_tag)
{
    BLE_MSG msg;
    BLE_GATT_SERVER_MSG *pBleSvrMsg = NULL;
    BLE_SERVER_READ_REQUEST *read_request = NULL;

    if (NULL == bt_gatts_read) {
        BLE_LOG(BLE_LOG_ERR,"bt_gatts_read is NULL");
        return;
    }
    BLE_LOG(BLE_LOG_INFO,"attr_handle =%d,conn_id=%d,trans_id=%d,offset=%d is_long = %d",
                       bt_gatts_read->attr_handle,bt_gatts_read->conn_id,
                       bt_gatts_read->trans_id,bt_gatts_read->offset,bt_gatts_read->is_long);

    msg.hdr.event = BLE_SERVER_EVENT;
    msg.hdr.len = sizeof(BLE_GATT_SERVER_MSG) - BLE_GATT_SERVER_MSG_VALUE_MAX_lEN + sizeof (BLE_SERVER_READ_REQUEST);
    pBleSvrMsg = (BLE_GATT_SERVER_MSG *)&msg.data;
    pBleSvrMsg->type = BLE_GATT_SERVER_MISC_MSG;
    pBleSvrMsg->event = BLE_GATT_SERVER_MISC_OP_READ_REQUEST;
    read_request = (BLE_SERVER_READ_REQUEST *)pBleSvrMsg->value;

    read_request->conn_id = bt_gatts_read->conn_id;
    read_request->trans_id = bt_gatts_read->trans_id;
    read_request->attr_handle = bt_gatts_read->attr_handle;
    read_request->offset = bt_gatts_read->offset;
    read_request->is_long = bt_gatts_read->is_long;
    memcpy(read_request->btaddr,bt_gatts_read->btaddr,BLE_MAC_lEN);

    if (ble_send_msg(&msg) != BLE_OK) {
        BLE_LOG(BLE_LOG_ERR,"ble_send_msg fail");
        return;
    }
}

static VOID ble_gatts_req_write_callback(BT_GATTS_REQ_WRITE_RST_T *bt_gatts_write,
                                                                VOID* pv_tag)
{
    BLE_MSG msg;
    BLE_GATT_SERVER_MSG *pBleSvrMsg = NULL;
    BLE_SERVER_WRITE_REQUEST *write_request = NULL;
    INT32 i = 0;

    if (NULL == bt_gatts_write) {
        BLE_LOG(BLE_LOG_ERR,"bt_gatts_write is NULL\n");
        return;
    }
    BLE_LOG(BLE_LOG_INFO,"attr_handle =%d, offset=%d , trans_id=%d need_rsp=%d is_prep = %d, btaddr = %s value=%s length=%d",
        bt_gatts_write->attr_handle, bt_gatts_write->offset, bt_gatts_write->trans_id, bt_gatts_write->need_rsp, bt_gatts_write->is_prep,
        bt_gatts_write->btaddr, bt_gatts_write->value,bt_gatts_write->length);

    for (i=0;i<bt_gatts_write->length;i++)
       BLE_LOG(BLE_LOG_INFO,"0x%x",bt_gatts_write->value[i]);

    msg.hdr.event = BLE_SERVER_EVENT;
    msg.hdr.len = sizeof(BLE_GATT_SERVER_MSG) - BLE_GATT_SERVER_MSG_VALUE_MAX_lEN 
        + sizeof (BLE_SERVER_WRITE_REQUEST) - BT_GATT_MAX_ATTR_LEN + bt_gatts_write->length;
    pBleSvrMsg = (BLE_GATT_SERVER_MSG *)&msg.data;
    pBleSvrMsg->type = BLE_GATT_SERVER_MISC_MSG;
    pBleSvrMsg->event = BLE_GATT_SERVER_MISC_OP_WRITE_REQUEST;
    write_request = (BLE_SERVER_WRITE_REQUEST *)pBleSvrMsg->value;

    write_request->attr_handle = bt_gatts_write->attr_handle;
    write_request->offset = bt_gatts_write->offset;
    write_request->trans_id = bt_gatts_write->trans_id;
    write_request->need_rsp = bt_gatts_write->need_rsp;
    write_request->is_prep = bt_gatts_write->is_prep;
    write_request->length = bt_gatts_write->length;
    memcpy(write_request->btaddr,bt_gatts_write->btaddr,BLE_MAC_lEN);
    memcpy(write_request->value,bt_gatts_write->value,bt_gatts_write->length);

    if (ble_send_msg(&msg) != BLE_OK) {
        BLE_LOG(BLE_LOG_ERR,"ble_send_msg fail");
        return;
    }

}

static VOID ble_gatts_ind_sent_callback(INT32 conn_id,
                                                               INT32 status,
                                                               VOID* pv_tag)
{
    BLE_LOG(BLE_LOG_INFO,"status =%d, conn_id = %d",status, conn_id);
}

INT32 ble_gatts_send_indication(CHAR *data,INT32 length) {

    if (data == NULL) {
        BLE_LOG(BLE_LOG_ERR, "data is NULL");
        return BLE_FAIL;
    }

    if (a_mtkapi_bt_gatts_send_indication(gGattsMwInfo.server_if, gGattsMwInfo.charHandle, 
               gGattsMwInfo.connId, 0, data, length) == -1) {
        BLE_LOG(BLE_LOG_ERR, "send indication fail");
        return BLE_FAIL;
    }

    return BLE_OK;
}

INT32 ble_gatts_server_add_service(CHAR *service_uuid) {

    if (service_uuid == NULL) {
        BLE_LOG(BLE_LOG_ERR,"service_uuid is NULL.");
        return BLE_FAIL;
    }

    BLE_LOG(BLE_LOG_INFO,"gatt server add service(%d,%s).\n",
              gGattsMwInfo.servHandle,service_uuid);

    if (a_mtkapi_bt_gatts_add_service(gGattsMwInfo.server_if,
                 service_uuid,1,20) == -1) {
        BLE_LOG(BLE_LOG_ERR,"add service fail.");
        return BLE_FAIL;
    }

    return BLE_OK;
}

INT32 ble_gatts_server_add_char(CHAR *char_uuid) {

    if (char_uuid == NULL) {
        BLE_LOG(BLE_LOG_ERR,"char_uuid is NULL.");
        return BLE_FAIL;
    }

    BLE_LOG(BLE_LOG_INFO,"gatt server add charac(%d,%d %s).\n",
              gGattsMwInfo.servHandle,gGattsMwInfo.server_if,char_uuid);

    if (a_mtkapi_bt_gatts_add_char(gGattsMwInfo.server_if,
                          gGattsMwInfo.servHandle,char_uuid,0x1e,0x11) == -1) {
        BLE_LOG(BLE_LOG_ERR,"add char fail.");
        return BLE_FAIL;
    }

    return BLE_OK;
}

INT32 ble_gatts_server_add_desc(CHAR *desc_uuid) {

    if (desc_uuid == NULL) {
        BLE_LOG(BLE_LOG_ERR,"desc_uuid is NULL.");
        return BLE_FAIL;
    }

    BLE_LOG(BLE_LOG_INFO,"gatt server add description(%d,%d %s).\n",
              gGattsMwInfo.servHandle,gGattsMwInfo.server_if,desc_uuid);

    if (a_mtkapi_bt_gatts_add_desc(gGattsMwInfo.server_if,
                          gGattsMwInfo.servHandle,desc_uuid,0x11) == -1) {
        BLE_LOG(BLE_LOG_ERR,"add desc fail.");
        return BLE_FAIL;
    }

    return BLE_OK;
}

INT32 ble_gatts_server_start(VOID) {

    BLE_LOG(BLE_LOG_INFO,"start gatt server(%d,%d).\n",
              gGattsMwInfo.servHandle,gGattsMwInfo.server_if);

    /*stop gatt server*/
    if (a_mtkapi_bt_gatts_start_service(gGattsMwInfo.server_if,gGattsMwInfo.servHandle,0) == -1){
        BLE_LOG(BLE_LOG_ERR,"start gatt server fail.\n");
        return BLE_FAIL;
    }

    return BLE_OK;
}

INT32 ble_gatts_server_get_conn_info(VOID) {
    BT_GATTS_CONNECT_RST_T connect_rst_info;

    BLE_LOG(BLE_LOG_INFO,"gatt server get conn info(%d,%d).\n",
              gGattsMwInfo.servHandle,gGattsMwInfo.server_if);

    a_mtkapi_bt_gatts_get_connect_result_info(&connect_rst_info);

    gGattsMwInfo.connId = connect_rst_info.conn_id;
    memcpy(gGattsMwInfo.btAddr,connect_rst_info.btaddr,BLE_MAC_lEN);
    BLE_LOG(BLE_LOG_ERR,"get connection info(conn_id=%d %02x:%02x:%02x:%02x:%02x:%02x).",
                    gGattsMwInfo.connId,gGattsMwInfo.btAddr[0],gGattsMwInfo.btAddr[1],
                    gGattsMwInfo.btAddr[2],gGattsMwInfo.btAddr[3],gGattsMwInfo.btAddr[4],
                    gGattsMwInfo.btAddr[5]);

    return BLE_OK;
}

INT32 ble_gatts_server_read_response(INT32 handle,INT32 transId,VOID *readData,UINT32 len,INT32 status) {

    BLE_LOG(BLE_LOG_INFO,"gatt server request read(%d %d %d %d %d).\n",
              gGattsMwInfo.servHandle,gGattsMwInfo.server_if,gGattsMwInfo.connId,
              transId,handle,len);

    if (a_mtkapi_bt_gatts_send_response(gGattsMwInfo.connId,
          transId,0,handle,
          readData,len,0) == -1) {
        BLE_LOG(BLE_LOG_ERR,"a_mtkapi_bt_gatts_send_response fail");
        return BLE_FAIL;
    }

    return BLE_OK;
}

INT32 ble_gatts_server_write_response(INT32 handle,INT32 transId,VOID *writeData,UINT32 len,INT32 status) {

    BLE_LOG(BLE_LOG_INFO,"gatt server request write(%d %d %d %d %d).\n",
              gGattsMwInfo.servHandle,gGattsMwInfo.server_if,gGattsMwInfo.connId,
              transId,handle,len);

    if (a_mtkapi_bt_gatts_send_response(gGattsMwInfo.connId,
                      transId,0,handle,
                      writeData,len,0) == -1) {
        BLE_LOG(BLE_LOG_ERR,"a_mtkapi_bt_gatts_send_response fail");
        return BLE_FAIL;
    }

    return BLE_OK;
}

INT32 ble_gatts_server_connection_close(VOID) {

    BLE_LOG(BLE_LOG_INFO,"close gatt server(%d,%d,%02x:%02x:%02x:%02x:%02x:%02x,%d).\n",
              gGattsMwInfo.servHandle,gGattsMwInfo.server_if,gGattsMwInfo.btAddr[0],
              gGattsMwInfo.btAddr[1],gGattsMwInfo.btAddr[2],gGattsMwInfo.btAddr[3],
              gGattsMwInfo.btAddr[4],gGattsMwInfo.btAddr[5],gGattsMwInfo.connId);

    /*close gatt server*/
    if (a_mtkapi_bt_gatts_close(gGattsMwInfo.server_if,gGattsMwInfo.btAddr,gGattsMwInfo.connId) == -1){
        BLE_LOG(BLE_LOG_ERR,"close gatt server connection fail.\n");
        return BLE_FAIL;
    }

    return BLE_OK;
}

INT32 ble_gatts_server_stop(VOID) {

    BLE_LOG(BLE_LOG_INFO,"stop gatt server(%d,%d).\n",
              gGattsMwInfo.servHandle,gGattsMwInfo.server_if);

    /*stop gatt server*/
    if (a_mtkapi_bt_gatts_stop_service(gGattsMwInfo.server_if,gGattsMwInfo.servHandle) == -1){
        BLE_LOG(BLE_LOG_ERR,"stop gatt server fail.\n");
        return BLE_FAIL;
    }

    return BLE_OK;
}

INT32 ble_gatts_server_delete(VOID) {

    BLE_LOG(BLE_LOG_INFO,"delete gatt server(%d,%d).\n",
              gGattsMwInfo.servHandle,gGattsMwInfo.server_if);

    /*delete gatt server*/
    if (a_mtkapi_bt_gatts_delete_service(gGattsMwInfo.server_if,gGattsMwInfo.servHandle) == -1){
        BLE_LOG(BLE_LOG_ERR,"delete gatt server fail.\n");
        return BLE_FAIL;
    }

    return BLE_OK;
}

INT32 ble_gatts_init(VOID) {
    MTKRPCAPI_BT_APP_GATTS_CB_FUNC_T func;

    memset(&func, 0, sizeof(MTKRPCAPI_BT_APP_GATTS_CB_FUNC_T));
    func.bt_gatts_reg_server_cb = ble_gatts_register_server_callback;
    func.bt_gatts_event_cb = ble_gatts_event_callback;
    func.bt_gatts_add_srvc_cb = ble_gatts_add_srvc_callback;
    func.bt_gatts_add_incl_cb = ble_gatts_add_incl_callback;
    func.bt_gatts_add_char_cb = ble_gatts_add_char_callback;
    func.bt_gatts_add_desc_cb = ble_gatts_add_desc_callback;
    func.bt_gatts_op_srvc_cb = ble_gatts_op_srvc_callback;
    func.bt_gatts_req_read_cb = ble_gatts_req_read_callback;
    func.bt_gatts_req_write_cb = ble_gatts_req_write_callback;
    func.bt_gatts_ind_sent_cb = ble_gatts_ind_sent_callback;

    if (a_mtkapi_bt_gatts_base_init(&func, NULL) == -1){
        BLE_LOG(BLE_LOG_ERR,"bt gatts init fail.");
        return BLE_FAIL;
    }

    BLE_LOG(BLE_LOG_INFO,"bt gatts init success.");

    return BLE_OK;
}

INT32 ble_gatts_register(CHAR *server_uuid) {

    if (a_mtkapi_bt_gatts_register_server(server_uuid) == -1) {
        BLE_LOG(BLE_LOG_ERR,"register server fail.");
        return BLE_FAIL;
    }

    BLE_LOG(BLE_LOG_INFO,"bt gatts register success.");

    return BLE_OK;
}

INT32 ble_gatts_unregister(VOID) {

    if (a_mtkapi_bt_gatts_unregister_server(gGattsMwInfo.server_if) == -1){
        BLE_LOG(BLE_LOG_ERR,"bt gatts destroy fail.");
        return BLE_FAIL;
    }

    BLE_LOG(BLE_LOG_INFO,"bt gatts destroy success.");

    return BLE_OK;
}