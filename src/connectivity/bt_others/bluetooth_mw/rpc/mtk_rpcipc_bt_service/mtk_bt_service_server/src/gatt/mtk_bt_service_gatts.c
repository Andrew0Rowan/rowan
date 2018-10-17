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


/*-----------------------------------------------------------------------------
                            include files
-----------------------------------------------------------------------------*/
#include <stdio.h>
#include "mtk_bt_service_gatts_wrapper.h"
#include "mtk_bt_service_gatts.h"
#include "c_bt_mw_gatts.h"

#define BT_RC_LOG(_stmt...) \
        do{ \
            if(0){    \
                printf("Func:%s Line:%d--->: ", __FUNCTION__, __LINE__);   \
                printf(_stmt); \
                printf("\n"); \
            }        \
        }   \
        while(0)


static void *g_gatts_pvtag = NULL;

static mtkrpcapi_BtAppGATTSEventCbk mtkrpcapi_BtGATTSEventCbk = NULL;
static mtkrpcapi_BtAppGATTSRegServerCbk mtkrpcapi_BtGATTSRegServerCbk = NULL;
static mtkrpcapi_BtAppGATTSAddSrvcCbk mtkrpcapi_BtGATTSAddSrvcCbk = NULL;
static mtkrpcapi_BtAppGATTSAddInclCbk mtkrpcapi_BtGATTSAddInclCbk = NULL;
static mtkrpcapi_BtAppGATTSAddCharCbk mtkrpcapi_BtGATTSAddCharCbk = NULL;
static mtkrpcapi_BtAppGATTSAddDescCbk mtkrpcapi_BtGATTSAddDescCbk = NULL;
static mtkrpcapi_BtAppGATTSOpSrvcCbk mtkrpcapi_BtGATTSOpSrvcCbk = NULL;
static mtkrpcapi_BtAppGATTSReqReadCbk mtkrpcapi_BtGATTSReqReadCbk = NULL;
static mtkrpcapi_BtAppGATTSReqWriteCbk mtkrpcapi_BtGATTSReqWriteCbk = NULL;
static mtkrpcapi_BtAppGATTSIndSentCbk mtkrpcapi_BtGATTSIndSentCbk = NULL;

VOID MWBtAppGATTSEventCbk(BT_GATTS_EVENT_T bt_gatts_event)
{
    BT_RC_LOG("[.c][%s], bt_gatts_event= %d\n", __FUNCTION__, bt_gatts_event);
    if (mtkrpcapi_BtGATTSEventCbk)
    {
        mtkrpcapi_BtGATTSEventCbk(bt_gatts_event, g_gatts_pvtag);
    }
}

VOID MWBtAppGATTSRegServerCbk(BT_GATTS_REG_SERVER_RST_T *bt_gatts_reg_server)
{
    if (NULL == bt_gatts_reg_server)
    {
        return;
    }
    BT_RC_LOG("[.c][%s], server_if = %ld\n", __FUNCTION__, (long)bt_gatts_reg_server->server_if);
    if (mtkrpcapi_BtGATTSRegServerCbk)
    {
        mtkrpcapi_BtGATTSRegServerCbk(bt_gatts_reg_server, g_gatts_pvtag);
    }
}

VOID MWBtAppGATTSAddSrvcCbk(BT_GATTS_ADD_SRVC_RST_T *bt_gatts_add_srvc)
{
    if (NULL == bt_gatts_add_srvc)
    {
        return;
    }
    BT_RC_LOG("[.c][%s], uuid = %s\n", __FUNCTION__,
    bt_gatts_add_srvc->srvc_id.id.uuid);
    if (mtkrpcapi_BtGATTSAddSrvcCbk)
    {
        mtkrpcapi_BtGATTSAddSrvcCbk(bt_gatts_add_srvc, g_gatts_pvtag);
    }
}

VOID MWBtAppGATTSAddInclCbk(BT_GATTS_ADD_INCL_RST_T *bt_gatts_add_incl)
{
    if (NULL == bt_gatts_add_incl)
    {
        return;
    }
    BT_RC_LOG("[.c][%s], incl_srvc_handle = %ld\n", __FUNCTION__,
              (long)bt_gatts_add_incl->incl_srvc_handle);
    if (mtkrpcapi_BtGATTSAddInclCbk)
    {
        mtkrpcapi_BtGATTSAddInclCbk(bt_gatts_add_incl, g_gatts_pvtag);
    }
}

VOID MWBtAppGATTSAddCharCbk(BT_GATTS_ADD_CHAR_RST_T *bt_gatts_add_char)
{
    if (NULL == bt_gatts_add_char)
    {
        return;
    }
    BT_RC_LOG("[.c][%s], uuid = %s\n", __FUNCTION__, bt_gatts_add_char->uuid);
    if (mtkrpcapi_BtGATTSAddCharCbk)
    {
        mtkrpcapi_BtGATTSAddCharCbk(bt_gatts_add_char, g_gatts_pvtag);
    }
}

VOID MWBtAppGATTSAddDescCbk(BT_GATTS_ADD_DESCR_RST_T *bt_gatts_add_desc)
{
    if (NULL == bt_gatts_add_desc)
    {
        return;
    }
    BT_RC_LOG("[.c][%s], uuid = %s\n", __FUNCTION__, bt_gatts_add_desc->uuid);
    if (mtkrpcapi_BtGATTSAddDescCbk)
    {
        mtkrpcapi_BtGATTSAddDescCbk(bt_gatts_add_desc, g_gatts_pvtag);
    }
}

VOID MWBtAppGATTSOpSrvcCbk(BT_GATTS_SRVC_OP_TYPE_T op_type,
                                         BT_GATTS_SRVC_RST_T *bt_gatts_srvc)
{
    if (NULL == bt_gatts_srvc)
    {
        return;
    }
    BT_RC_LOG("[.c][%s], op_type = %ld, srvc_handle = %ld\n", __FUNCTION__,
               (long)op_type, (long)bt_gatts_srvc->srvc_handle);
    if (mtkrpcapi_BtGATTSOpSrvcCbk)
    {
        mtkrpcapi_BtGATTSOpSrvcCbk(op_type, bt_gatts_srvc, g_gatts_pvtag);
    }
}

VOID MWBtAppGATTSReqReadCbk(BT_GATTS_REQ_READ_RST_T *bt_gatts_read)
{
    if (NULL == bt_gatts_read)
    {
        return;
    }
    BT_RC_LOG("[.c][%s], attr_handle = %ld\n", __FUNCTION__,
             (long)bt_gatts_read->attr_handle);
    if (mtkrpcapi_BtGATTSReqReadCbk)
    {
        mtkrpcapi_BtGATTSReqReadCbk(bt_gatts_read, g_gatts_pvtag);
    }
}

VOID MWBtAppGATTSReqWriteCbk(BT_GATTS_REQ_WRITE_RST_T *bt_gatts_write)
{
    if (NULL == bt_gatts_write)
    {
        return;
    }
    BT_RC_LOG("[.c][%s], attr_handle = %ld\n", __FUNCTION__,
              (long)bt_gatts_write->attr_handle);
    if (mtkrpcapi_BtGATTSReqWriteCbk)
    {
        mtkrpcapi_BtGATTSReqWriteCbk(bt_gatts_write, g_gatts_pvtag);
    }
}

VOID MWBtAppGATTSIndSentCbk(INT32 conn_id, INT32 status)
{
    BT_RC_LOG("[.c][%s], conn_id = %ld, status = %ld\n", __FUNCTION__,
              (long)conn_id, (long)status);
    if (mtkrpcapi_BtGATTSIndSentCbk)
    {
        mtkrpcapi_BtGATTSIndSentCbk(conn_id, status, g_gatts_pvtag);
    }
}


/*register APP callback function*/
INT32 mtkrpcapi_btm_gatts_reg_cbk_fct(MTKRPCAPI_BT_APP_GATTS_CB_FUNC_T *func,
                                                 void *pv_tag)
{
    INT32 i4_ret = 0;

    g_gatts_pvtag = pv_tag;
    if(NULL == func)
    {
        BT_RC_LOG(("callback func is null!\n"));
        return BT_ERR_STATUS_NULL_POINTER;
    }
    if(func->bt_gatts_event_cb)
    {
        mtkrpcapi_BtGATTSEventCbk = func->bt_gatts_event_cb;
    }
    else
    {
        BT_RC_LOG(("event callback func is null!\n"));
        i4_ret = BT_ERR_STATUS_NULL_POINTER;
    }

    if(func->bt_gatts_reg_server_cb)
    {
        mtkrpcapi_BtGATTSRegServerCbk = func->bt_gatts_reg_server_cb;
    }
    else
    {
        BT_RC_LOG(("bt_gatts_reg_server_cb callback func is null!\n"));
        i4_ret = BT_ERR_STATUS_NULL_POINTER;
    }
    if(func->bt_gatts_add_srvc_cb)
    {
        mtkrpcapi_BtGATTSAddSrvcCbk = func->bt_gatts_add_srvc_cb;
    }
    else
    {
        BT_RC_LOG(("bt_gatts_add_srvc_cb callback func is null!\n"));
        i4_ret = BT_ERR_STATUS_NULL_POINTER;
    }

    if(func->bt_gatts_add_incl_cb)
    {
        mtkrpcapi_BtGATTSAddInclCbk = func->bt_gatts_add_incl_cb;
    }
    else
    {
        BT_RC_LOG(("bt_gatts_add_incl_cb callback func is null!\n"));
        i4_ret = BT_ERR_STATUS_NULL_POINTER;
    }

    if(func->bt_gatts_add_char_cb)
    {
        mtkrpcapi_BtGATTSAddCharCbk = func->bt_gatts_add_char_cb;
    }
    else
    {
        BT_RC_LOG(("bt_gatts_add_char_cb callback func is null!\n"));
        i4_ret = BT_ERR_STATUS_NULL_POINTER;
    }

    if(func->bt_gatts_add_desc_cb)
    {
        mtkrpcapi_BtGATTSAddDescCbk = func->bt_gatts_add_desc_cb;
    }
    else
    {
        BT_RC_LOG(("bt_gatts_add_desc_cb callback func is null!\n"));
        i4_ret = BT_ERR_STATUS_NULL_POINTER;
    }

    if(func->bt_gatts_op_srvc_cb)
    {
        mtkrpcapi_BtGATTSOpSrvcCbk = func->bt_gatts_op_srvc_cb;
    }
    else
    {
        BT_RC_LOG(("bt_gatts_op_srvc_cb callback func is null!\n"));
        i4_ret = BT_ERR_STATUS_NULL_POINTER;
    }

    if(func->bt_gatts_req_read_cb)
    {
        mtkrpcapi_BtGATTSReqReadCbk = func->bt_gatts_req_read_cb;
    }
    else
    {
        BT_RC_LOG(("bt_gatts_req_read_cb callback func is null!\n"));
        i4_ret = BT_ERR_STATUS_NULL_POINTER;
    }

    if(func->bt_gatts_req_write_cb)
    {
        mtkrpcapi_BtGATTSReqWriteCbk = func->bt_gatts_req_write_cb;
    }
    else
    {
        BT_RC_LOG(("bt_gatts_req_write_cb callback func is null!\n"));
        i4_ret = BT_ERR_STATUS_NULL_POINTER;
    }

    if(func->bt_gatts_ind_sent_cb)
    {
        mtkrpcapi_BtGATTSIndSentCbk = func->bt_gatts_ind_sent_cb;
    }
    else
    {
        BT_RC_LOG(("bt_gatts_ind_sent_cb callback func is null!\n"));
        i4_ret = BT_ERR_STATUS_NULL_POINTER;
    }
    return i4_ret;
}

INT32 mtkrpcapi_bt_gatts_register_callback(MTKRPCAPI_BT_APP_GATTS_CB_FUNC_T *func,
                                                     void *pv_tag)
{
    BT_RC_LOG("[.c][%s]\n", __FUNCTION__);
    INT32 i4_ret = 0;
    BT_APP_GATTS_CB_FUNC_T app_func;
    memset(&app_func, 0, sizeof(BT_APP_GATTS_CB_FUNC_T));
    app_func.bt_gatts_event_cb = MWBtAppGATTSEventCbk;
    app_func.bt_gatts_reg_server_cb = MWBtAppGATTSRegServerCbk;
    app_func.bt_gatts_add_srvc_cb = MWBtAppGATTSAddSrvcCbk;
    app_func.bt_gatts_add_incl_cb = MWBtAppGATTSAddInclCbk;
    app_func.bt_gatts_add_char_cb = MWBtAppGATTSAddCharCbk;
    app_func.bt_gatts_add_desc_cb = MWBtAppGATTSAddDescCbk;
    app_func.bt_gatts_op_srvc_cb = MWBtAppGATTSOpSrvcCbk;
    app_func.bt_gatts_req_read_cb = MWBtAppGATTSReqReadCbk;
    app_func.bt_gatts_req_write_cb = MWBtAppGATTSReqWriteCbk;
    app_func.bt_gatts_ind_sent_cb = MWBtAppGATTSIndSentCbk;
    i4_ret = mtkrpcapi_btm_gatts_reg_cbk_fct(func, pv_tag);
    if(0 != i4_ret)
    {
        BT_RC_LOG(("mtkrpcapi_bt_gatts_base_init Error.\n"));
        return i4_ret;
    }

    return c_btm_gatts_register_callback(&app_func);
}

INT32 x_mtkapi_bt_gatts_register_callback(MTKRPCAPI_BT_APP_GATTS_CB_FUNC_T *func,
                                                     void *pv_tag)
{
    BT_RC_LOG("[.c][%s]\n", __FUNCTION__);
    return mtkrpcapi_bt_gatts_register_callback(func, pv_tag);

}

INT32 x_mtkapi_bt_gatts_register_server(char * app_uuid)
{
    INT32 i4_ret = 0;
    i4_ret = c_btm_gatts_register_server(app_uuid);
    if (i4_ret != 0)
    {
        BT_RC_LOG("[.c][%s]c_btm_gatts_register_server fail\n", __FUNCTION__);
    }
    else
    {
        BT_RC_LOG("[.c][%s]c_btm_gatts_register_server success\n", __FUNCTION__);
    }
    return i4_ret;
}
INT32 x_mtkapi_bt_gatts_unregister_server(INT32 server_if)
{
    INT32 i4_ret = 0;
    i4_ret = c_btm_gatts_unregister_server(server_if);
    if (i4_ret != 0)
    {
        BT_RC_LOG("[.c][%s]c_btm_gatts_unregister_server fail\n", __FUNCTION__);
    }
    else
    {
        BT_RC_LOG("[.c][%s]c_btm_gatts_unregister_server success\n", __FUNCTION__);
    }
    return i4_ret;
}

INT32 x_mtkapi_bt_gatts_connect(INT32 server_if, CHAR *bt_addr,
                                         UINT8 is_direct, INT32 transport)
{
    INT32 i4_ret = 0;
    i4_ret = c_btm_gatts_connect(server_if, bt_addr, is_direct, transport);
    if (i4_ret != 0)
    {
        BT_RC_LOG("[.c][%s]c_btm_gatts_connect fail\n", __FUNCTION__);
    }
    else
    {
        BT_RC_LOG("[.c][%s]c_btm_gatts_connect success\n", __FUNCTION__);
    }
    return i4_ret;
}

INT32 x_mtkapi_bt_gatts_disconnect(INT32 server_if, CHAR *bt_addr,
                                             INT32 conn_id)
{
    INT32 i4_ret = 0;
    i4_ret = c_btm_gatts_disconnect(server_if, bt_addr, conn_id);
    if (i4_ret != 0)
    {
        BT_RC_LOG("[.c][%s]c_btm_gatts_disconnect fail\n", __FUNCTION__);
    }
    else
    {
        BT_RC_LOG("[.c][%s]c_btm_gatts_disconnect success\n", __FUNCTION__);
    }
    return i4_ret;
}

INT32 x_mtkapi_bt_gatts_add_service(INT32 server_if, CHAR *service_uuid,
                                               UINT8 is_primary, INT32 number)
{
    INT32 i4_ret = 0;
    i4_ret = c_btm_gatts_add_service(server_if, service_uuid, is_primary, number);
    if (i4_ret != 0)
    {
        BT_RC_LOG("[.c][%s]c_btm_gatts_add_service fail\n", __FUNCTION__);
    }
    else
    {
        BT_RC_LOG("[.c][%s]c_btm_gatts_add_service success\n", __FUNCTION__);
    }
    return i4_ret;
}

INT32 x_mtkapi_bt_gatts_add_included_service(INT32 server_if,
                                                           INT32 service_handle,
                                                           INT32 included_handle)
{
    INT32 i4_ret = 0;
    i4_ret = c_btm_gatts_add_included_service(server_if, service_handle, included_handle);
    if (i4_ret != 0)
    {
        BT_RC_LOG("[.c][%s]c_btm_gatts_add_included_service fail\n", __FUNCTION__);
    }
    else
    {
        BT_RC_LOG("[.c][%s]c_btm_gatts_add_included_service success\n", __FUNCTION__);
    }
    return i4_ret;
}

INT32 x_mtkapi_bt_gatts_add_char(INT32 server_if, INT32 service_handle,
                                           CHAR *uuid, INT32 properties,
                                           INT32 permissions)
{
    INT32 i4_ret = 0;
    i4_ret = c_btm_gatts_add_char(server_if, service_handle, uuid, properties, permissions);
    if (i4_ret != 0)
    {
        BT_RC_LOG("[.c][%s]c_btm_gatts_add_char fail\n", __FUNCTION__);
    }
    else
    {
        BT_RC_LOG("[.c][%s]c_btm_gatts_add_char success\n", __FUNCTION__);
    }
    return i4_ret;
}

INT32 x_mtkapi_bt_gatts_add_desc(INT32 server_if, INT32 service_handle,
                                           CHAR *uuid, INT32 permissions)
{
    INT32 i4_ret = 0;
    i4_ret = c_btm_gatts_add_desc(server_if, service_handle, uuid, permissions);
    if (i4_ret != 0)
    {
        BT_RC_LOG("[.c][%s]c_btm_gatts_add_desc fail\n", __FUNCTION__);
    }
    else
    {
        BT_RC_LOG("[.c][%s]c_btm_gatts_add_desc success\n", __FUNCTION__);
    }
    return i4_ret;
}

INT32 x_mtkapi_bt_gatts_start_service(INT32 server_if, INT32 service_handle,
                                                INT32 transport)
{
    INT32 i4_ret = 0;
    i4_ret = c_btm_gatts_start_service(server_if, service_handle, transport);
    if (i4_ret != 0)
    {
        BT_RC_LOG("[.c][%s]c_btm_gatts_start_service fail\n", __FUNCTION__);
    }
    else
    {
        BT_RC_LOG("[.c][%s]c_btm_gatts_start_service success\n", __FUNCTION__);
    }
    return i4_ret;
}

INT32 x_mtkapi_bt_gatts_stop_service(INT32 server_if, INT32 service_handle)
{
    INT32 i4_ret = 0;
    i4_ret = c_btm_gatts_stop_service(server_if, service_handle);
    if (i4_ret != 0)
    {
        BT_RC_LOG("[.c][%s]c_btm_gatts_stop_service fail\n", __FUNCTION__);
    }
    else
    {
        BT_RC_LOG("[.c][%s]c_btm_gatts_stop_service success\n", __FUNCTION__);
    }
    return i4_ret;
}

INT32 x_mtkapi_bt_gatts_delete_service(INT32 server_if, INT32 service_handle)
{
    INT32 i4_ret = 0;
    i4_ret = c_btm_gatts_delete_service(server_if, service_handle);
    if (i4_ret != 0)
    {
        BT_RC_LOG("[.c][%s]c_btm_gatts_delete_service fail\n", __FUNCTION__);
    }
    else
    {
        BT_RC_LOG("[.c][%s]c_btm_gatts_delete_service success\n", __FUNCTION__);
    }
    return i4_ret;
}

INT32 x_mtkapi_bt_gatts_send_indication(INT32 server_if, INT32 attribute_handle,
                                                   INT32 conn_id, INT32 fg_confirm,
                                                   CHAR *p_value, INT32 value_len)
{
    INT32 i4_ret = 0;
    i4_ret = c_btm_gatts_send_indication(server_if, attribute_handle, conn_id,
                                            fg_confirm, p_value, value_len);
    if (i4_ret != 0)
    {
        BT_RC_LOG("[.c][%s]c_btm_gatts_send_indication fail\n", __FUNCTION__);
    }
    else
    {
        BT_RC_LOG("[.c][%s]c_btm_gatts_send_indication success\n", __FUNCTION__);
    }
    return i4_ret;
}

INT32 x_mtkapi_bt_gatts_send_response(INT32 conn_id, INT32 trans_id,
                                                  INT32 status, INT32 handle,
                                                  CHAR *p_value, INT32 value_len,
                                                  INT32 auth_req)
{
    INT32 i4_ret = 0;
    i4_ret = c_btm_gatts_send_response(conn_id, trans_id, status, handle,
                                       p_value, value_len, auth_req);
    if (i4_ret != 0)
    {
        BT_RC_LOG("[.c][%s]c_btm_gatts_send_response fail\n", __FUNCTION__);
    }
    else
    {
        BT_RC_LOG("[.c][%s]c_btm_gatts_send_response success\n", __FUNCTION__);
    }
    return i4_ret;
}

VOID x_mtkapi_bt_gatts_get_connect_result_info(BT_GATTS_CONNECT_RST_T *connect_rst_info)
{
    c_btm_gatts_get_connect_result_info(connect_rst_info);
}

VOID x_mtkapi_bt_gatts_get_disconnect_result_info(BT_GATTS_CONNECT_RST_T *disconnect_rst_info)
{
    c_btm_gatts_get_disconnect_result_info(disconnect_rst_info);
}

