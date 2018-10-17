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

#include "mtk_bt_service_gattc_wrapper.h"
#include "mtk_bt_service_gatt_ipcrpc_struct.h"
#include "client_common.h"
#include "ri_common.h"

#define BT_RW_LOG(_stmt...) \
        do{ \
            if(0){    \
                printf("[Client]Func:%s Line:%d--->: ", __FUNCTION__, __LINE__);   \
                printf(_stmt); \
                printf("\n"); \
            }        \
        }   \
        while(0)

static INT32 _hndlr_bt_app_gattc_event_cbk(RPC_ID_T     t_rpc_id,
                                                      const CHAR*  ps_cb_type,
                                                      void          *pv_cb_addr,
                                                      UINT32       ui4_num_args,
                                                      ARG_DESC_T*  pt_args,
                                                      ARG_DESC_T*  pt_return)
{
    BT_RW_LOG("bt_app_gattc_event_cbk, pv_cb_addr = %p", pv_cb_addr);
    if(ui4_num_args != 2)
    {
        return RPCR_INV_ARGS;
    }
    pt_return->e_type   = ARG_TYPE_VOID;
    ((mtkrpcapi_BtAppGATTCEventCbk)pv_cb_addr)((BT_GATTC_EVENT_T)pt_args[0].u.i4_arg,
                                                pt_args[1].u.pv_arg);
    return RPCR_OK;
}

static INT32 _hndlr_bt_app_gattc_reg_client_cbk(RPC_ID_T     t_rpc_id,
                                                           const CHAR*  ps_cb_type,
                                                           void          *pv_cb_addr,
                                                           UINT32       ui4_num_args,
                                                           ARG_DESC_T*  pt_args,
                                                           ARG_DESC_T*  pt_return)
{
    BT_RW_LOG("bt_app_gattc_reg_client_cbk, pv_cb_addr = %p, pt_args[0].u.pv_desc = %p",
              pv_cb_addr, pt_args[0].u.pv_desc);
    if(ui4_num_args != 2)
    {
        return RPCR_INV_ARGS;
    }
    pt_return->e_type   = ARG_TYPE_VOID;
    ((mtkrpcapi_BtAppGATTCRegClientCbk)pv_cb_addr)((BT_GATTC_REG_CLIENT_T *)pt_args[0].u.pv_desc,
                                                    pt_args[1].u.pv_arg);
    return RPCR_OK;
}

static INT32 _hndlr_bt_app_gattc_scan_result_cbk(RPC_ID_T     t_rpc_id,
                                                             const CHAR*  ps_cb_type,
                                                             void          *pv_cb_addr,
                                                             UINT32       ui4_num_args,
                                                             ARG_DESC_T*  pt_args,
                                                             ARG_DESC_T*  pt_return)
{
    BT_RW_LOG("bt_app_gattc_scan_result_cbk, pv_cb_addr = %p, pt_args[0].u.pv_desc = %p",
              pv_cb_addr, pt_args[0].u.pv_desc);
    if(ui4_num_args != 2)
    {
        return RPCR_INV_ARGS;
    }
    pt_return->e_type   = ARG_TYPE_VOID;
    ((mtkrpcapi_BtAppGATTCScanCbk)pv_cb_addr)((BT_GATTC_SCAN_RST_T *)pt_args[0].u.pv_desc,
                                               pt_args[1].u.pv_arg);
    return RPCR_OK;
}

static INT32 _hndlr_bt_app_gattc_get_gatt_db_cbk(RPC_ID_T     t_rpc_id,
                                                             const CHAR*  ps_cb_type,
                                                             void          *pv_cb_addr,
                                                             UINT32       ui4_num_args,
                                                             ARG_DESC_T*  pt_args,
                                                             ARG_DESC_T*  pt_return)
{
    BT_RW_LOG("bt_app_gattc_get_gatt_db_cbk, pv_cb_addr = %p, pt_args[0].u.pv_desc = %p",
              pv_cb_addr, pt_args[0].u.pv_desc);
    if(ui4_num_args != 3)
    {
        return RPCR_INV_ARGS;
    }
    pt_return->e_type   = ARG_TYPE_VOID;
    BT_GATTC_GET_GATT_DB_T get_gatt_db;
    BT_GATTC_GET_GATT_DB_T *ptr_get_gatt_db = (BT_GATTC_GET_GATT_DB_T *)pt_args[0].u.pv_desc;
    get_gatt_db.conn_id = ptr_get_gatt_db->conn_id;
    get_gatt_db.count = ptr_get_gatt_db->count;
    get_gatt_db.gatt_db_element = (BT_GATTC_DB_ELEMENT_T *)pt_args[2].u.pv_desc;
    ((mtkrpcapi_BtAppGATTCGetGattDbCbk)pv_cb_addr)(&get_gatt_db, pt_args[1].u.pv_arg);
    return RPCR_OK;
}

static INT32 _hndlr_bt_app_gattc_get_reg_noti_cbk(RPC_ID_T     t_rpc_id,
                                                              const CHAR*  ps_cb_type,
                                                              void          *pv_cb_addr,
                                                              UINT32       ui4_num_args,
                                                              ARG_DESC_T*  pt_args,
                                                              ARG_DESC_T*  pt_return)
{
    BT_RW_LOG("bt_app_gattc_get_reg_noti_cbk, pv_cb_addr = %p, pt_args[0].u.pv_desc = %p",
              pv_cb_addr, pt_args[0].u.pv_desc);
    if(ui4_num_args != 2)
    {
        return RPCR_INV_ARGS;
    }
    pt_return->e_type   = ARG_TYPE_VOID;
    ((mtkrpcapi_BtAppGATTCGetRegNotiCbk)pv_cb_addr)((BT_GATTC_GET_REG_NOTI_RST_T *)pt_args[0].u.pv_desc,
                                                     pt_args[1].u.pv_arg);
    return RPCR_OK;
}

static INT32 _hndlr_bt_app_gattc_notify_cbk(RPC_ID_T     t_rpc_id,
                                                     const CHAR*  ps_cb_type,
                                                     void          *pv_cb_addr,
                                                     UINT32       ui4_num_args,
                                                     ARG_DESC_T*  pt_args,
                                                     ARG_DESC_T*  pt_return)
{
    BT_RW_LOG("bt_app_gattc_notify_cbk, pv_cb_addr = %p, pt_args[0].u.pv_desc = %p",
              pv_cb_addr, pt_args[0].u.pv_desc);
    if(ui4_num_args != 2)
    {
        return RPCR_INV_ARGS;
    }
    pt_return->e_type   = ARG_TYPE_VOID;
    ((mtkrpcapi_BtAppGATTCNotifyCbk)pv_cb_addr)((BT_GATTC_GET_NOTIFY_T *)pt_args[0].u.pv_desc,
                                                 pt_args[1].u.pv_arg);
    return RPCR_OK;
}

static INT32 _hndlr_bt_app_gattc_read_char_cbk(RPC_ID_T     t_rpc_id,
                                                           const CHAR*  ps_cb_type,
                                                           void          *pv_cb_addr,
                                                           UINT32       ui4_num_args,
                                                           ARG_DESC_T*  pt_args,
                                                           ARG_DESC_T*  pt_return)
{
    BT_RW_LOG("bt_app_gattc_read_char_cbk, pv_cb_addr = %p, pt_args[0].u.pv_desc = %p",
              pv_cb_addr, pt_args[0].u.pv_desc);
    if(ui4_num_args != 2)
    {
        return RPCR_INV_ARGS;
    }
    pt_return->e_type   = ARG_TYPE_VOID;
    ((mtkrpcapi_BtAppGATTCReadCharCbk)pv_cb_addr)((BT_GATTC_READ_CHAR_RST_T *)pt_args[0].u.pv_desc,
                                                   pt_args[1].u.pv_arg);
    return RPCR_OK;
}

static INT32 _hndlr_bt_app_gattc_write_char_cbk(RPC_ID_T     t_rpc_id,
                                                            const CHAR*  ps_cb_type,
                                                            void          *pv_cb_addr,
                                                            UINT32       ui4_num_args,
                                                            ARG_DESC_T*  pt_args,
                                                            ARG_DESC_T*  pt_return)
{

    BT_RW_LOG("bt_app_gattc_write_char_cbk, pv_cb_addr = %p, pt_args[0].u.pv_desc = %p",
              pv_cb_addr, pt_args[0].u.pv_desc);
    if(ui4_num_args != 2)
    {
        return RPCR_INV_ARGS;
    }
    pt_return->e_type   = ARG_TYPE_VOID;
    ((mtkrpcapi_BtAppGATTCWriteCharCbk)pv_cb_addr)((BT_GATTC_WRITE_CHAR_RST_T *)pt_args[0].u.pv_desc,
                                                    pt_args[1].u.pv_arg);
    return RPCR_OK;
}

static INT32 _hndlr_bt_app_gattc_read_desc_cbk(RPC_ID_T     t_rpc_id,
                                                           const CHAR*  ps_cb_type,
                                                           void          *pv_cb_addr,
                                                           UINT32       ui4_num_args,
                                                           ARG_DESC_T*  pt_args,
                                                           ARG_DESC_T*  pt_return)
{
    BT_RW_LOG("bt_app_gattc_read_desc_cbk, pv_cb_addr = %p, pt_args[0].u.pv_desc = %p",
              pv_cb_addr, pt_args[0].u.pv_desc);
    if(ui4_num_args != 2)
    {
        return RPCR_INV_ARGS;
    }
    pt_return->e_type   = ARG_TYPE_VOID;
    ((mtkrpcapi_BtAppGATTCReadDescCbk)pv_cb_addr)((BT_GATTC_READ_DESCR_RST_T *)pt_args[0].u.pv_desc,
                                                   pt_args[1].u.pv_arg);
    return RPCR_OK;
}

static INT32 _hndlr_bt_app_gattc_write_desc_cbk(RPC_ID_T     t_rpc_id,
                                                            const CHAR*  ps_cb_type,
                                                            void          *pv_cb_addr,
                                                            UINT32       ui4_num_args,
                                                            ARG_DESC_T*  pt_args,
                                                            ARG_DESC_T*  pt_return)
{
    BT_RW_LOG("bt_app_gattc_write_desc_cbk, pv_cb_addr = %p, pt_args[0].u.pv_desc = %p",
              pv_cb_addr, pt_args[0].u.pv_desc);
    if(ui4_num_args != 2)
    {
        return RPCR_INV_ARGS;
    }
    pt_return->e_type   = ARG_TYPE_VOID;
    ((mtkrpcapi_BtAppGATTCWriteDescCbk)pv_cb_addr)((BT_GATTC_WRITE_DESCR_RST_T *)pt_args[0].u.pv_desc,
                                                    pt_args[1].u.pv_arg);
    return RPCR_OK;
}

static INT32 _hndlr_bt_app_gattc_scan_filter_param_cbk(RPC_ID_T     t_rpc_id,
                                                                    const CHAR*  ps_cb_type,
                                                                    void          *pv_cb_addr,
                                                                    UINT32       ui4_num_args,
                                                                    ARG_DESC_T*  pt_args,
                                                                    ARG_DESC_T*  pt_return)
{
    BT_RW_LOG("bt_app_gattc_scan_filter_param_cbk, pv_cb_addr = %p, pt_args[0].u.pv_desc = %p",
              pv_cb_addr, pt_args[0].u.pv_desc);
    if(ui4_num_args != 2)
    {
        return RPCR_INV_ARGS;
    }
    pt_return->e_type   = ARG_TYPE_VOID;
    ((mtkrpcapi_BtAppGATTScanFilterParamCbk)pv_cb_addr)((BT_GATTC_SCAN_FILTER_PARAM_T *)pt_args[0].u.pv_desc,
                                                         pt_args[1].u.pv_arg);
    return RPCR_OK;
}

static INT32 _hndlr_bt_app_gattc_scan_filter_status_cbk(RPC_ID_T     t_rpc_id,
                                                                    const CHAR*  ps_cb_type,
                                                                    void          *pv_cb_addr,
                                                                    UINT32       ui4_num_args,
                                                                    ARG_DESC_T*  pt_args,
                                                                    ARG_DESC_T*  pt_return)
{
    BT_RW_LOG("bt_app_gattc_scan_filter_status_cbk, pv_cb_addr = %p, pt_args[0].u.pv_desc = %p",
              pv_cb_addr, pt_args[0].u.pv_desc);
    if(ui4_num_args != 2)
    {
        return RPCR_INV_ARGS;
    }
    pt_return->e_type   = ARG_TYPE_VOID;
    ((mtkrpcapi_BtAppGATTScanFilterStatusCbk)pv_cb_addr)((BT_GATTC_SCAN_FILTER_STATUS_T *)pt_args[0].u.pv_desc,
                                                          pt_args[1].u.pv_arg);
    return RPCR_OK;
}

static INT32 _hndlr_bt_app_gattc_scan_filter_cfg_cbk(RPC_ID_T     t_rpc_id,
                                                                 const CHAR*  ps_cb_type,
                                                                 void          *pv_cb_addr,
                                                                 UINT32       ui4_num_args,
                                                                 ARG_DESC_T*  pt_args,
                                                                 ARG_DESC_T*  pt_return)
{
    BT_RW_LOG("bt_app_gattc_scan_filter_cfg_cbk, pv_cb_addr = %p, pt_args[0].u.pv_desc = %p",
              pv_cb_addr, pt_args[0].u.pv_desc);
    if(ui4_num_args != 2)
    {
        return RPCR_INV_ARGS;
    }
    pt_return->e_type   = ARG_TYPE_VOID;
    ((mtkrpcapi_BtAppGATTScanFilterCfgCbk)pv_cb_addr)((BT_GATTC_SCAN_FILTER_CFG_T *)pt_args[0].u.pv_desc,
                                                       pt_args[1].u.pv_arg);
    return RPCR_OK;
}

static INT32 _hndlr_bt_app_gattc_adv_enable_cbk(RPC_ID_T     t_rpc_id,
                                                             const CHAR*  ps_cb_type,
                                                             void          *pv_cb_addr,
                                                             UINT32       ui4_num_args,
                                                             ARG_DESC_T*  pt_args,
                                                             ARG_DESC_T*  pt_return)
{
    BT_RW_LOG("bt_app_gattc_adv_enable_cbk, pv_cb_addr = %p, pt_args[0].u.pc_arg = %s",
              pv_cb_addr, pt_args[0].u.pc_arg);
    if(ui4_num_args != 2)
    {
        return RPCR_INV_ARGS;
    }
    pt_return->e_type   = ARG_TYPE_VOID;
    ((mtkrpcapi_BtAppGATTCAdvEnableCbk)pv_cb_addr)(pt_args[0].u.i8_arg, pt_args[1].u.pv_arg);
    return RPCR_OK;
}

EXPORT_SYMBOL INT32 a_mtkapi_bt_gattc_base_init(MTKRPCAPI_BT_APP_GATTC_CB_FUNC_T *func,
                                                        void* pv_tag)
{
    BT_RW_LOG("a_mtkapi_bt_gattc_base_init");
    RPC_CLIENT_DECL(2, INT32);
    RPC_CLIENT_CHECK(bt_rpc_add_ref_desc(RPC_DEFAULT_ID, func,
                                         RPC_DESC_MTKRPCAPI_BT_APP_GATTC_CB_FUNC_T,
                                         NULL));
    RPC_CLIENT_ARG_INP(ARG_TYPE_REF_DESC, func);
    RPC_CLIENT_ARG_INP(ARG_TYPE_REF_VOID, pv_tag);
    RPC_BT_SERVICE_CLIENT_DO_OP("x_mtkapi_bt_gattc_register_callback");
    RPC_CLIENT_RETURN(ARG_TYPE_INT32, -1);
}

EXPORT_SYMBOL INT32 a_mtkapi_bt_gattc_register_app(char *app_uuid)
{
    BT_RW_LOG("a_mtkapi_bt_gattc_register_app");
    RPC_CLIENT_DECL(1, INT32);
    RPC_CLIENT_ARG_INP(ARG_TYPE_REF_STR, app_uuid);
    RPC_BT_SERVICE_CLIENT_DO_OP("x_mtkapi_bt_gattc_register_app");
    RPC_CLIENT_RETURN(ARG_TYPE_INT32, -1);
}

EXPORT_SYMBOL INT32 a_mtkapi_bt_gattc_unregister_app(INT32 client_if)
{
    BT_RW_LOG("a_mtkapi_bt_gattc_unregister_app");
    RPC_CLIENT_DECL(1, INT32);
    RPC_CLIENT_ARG_INP(ARG_TYPE_INT32, client_if);
    RPC_BT_SERVICE_CLIENT_DO_OP("x_mtkapi_bt_gattc_unregister_app");
    RPC_CLIENT_RETURN(ARG_TYPE_INT32, -1);
}

EXPORT_SYMBOL INT32 a_mtkapi_bt_gattc_scan()
{
    BT_RW_LOG("a_mtkapi_bt_gattc_scan");
    BOOL b_start = TRUE;
    RPC_CLIENT_DECL(1, INT32);
    RPC_CLIENT_ARG_INP(ARG_TYPE_BOOL, b_start);
    RPC_BT_SERVICE_CLIENT_DO_OP("x_mtkapi_bt_gattc_scan");
    RPC_CLIENT_RETURN(ARG_TYPE_INT32, -1);
}
EXPORT_SYMBOL INT32 a_mtkapi_bt_gattc_stop_scan()
{
    BT_RW_LOG("a_mtkapi_bt_gattc_stop_scan");
    BOOL b_start = FALSE;
    RPC_CLIENT_DECL(1, INT32);
    RPC_CLIENT_ARG_INP(ARG_TYPE_BOOL, b_start);
    RPC_BT_SERVICE_CLIENT_DO_OP("x_mtkapi_bt_gattc_scan");
    RPC_CLIENT_RETURN(ARG_TYPE_INT32, -1);
}

EXPORT_SYMBOL INT32 a_mtkapi_bt_gattc_open(INT32 client_if, CHAR *bt_addr,
                                                   UINT8 is_direct, INT32 transport)
{
    BT_RW_LOG("a_mtkapi_bt_gattc_open");
    RPC_CLIENT_DECL(4, INT32);
    RPC_CLIENT_ARG_INP(ARG_TYPE_INT32, client_if);
    RPC_CLIENT_ARG_INP(ARG_TYPE_REF_STR, bt_addr);
    RPC_CLIENT_ARG_INP(ARG_TYPE_UINT8, is_direct);
    RPC_CLIENT_ARG_INP(ARG_TYPE_INT32, transport);
    RPC_BT_SERVICE_CLIENT_DO_OP("x_mtkapi_bt_gattc_connect");
    RPC_CLIENT_RETURN(ARG_TYPE_INT32, -1);
}

EXPORT_SYMBOL INT32 a_mtkapi_bt_gattc_unregister_callback()
{
    BT_RW_LOG("a_mtkapi_bt_gattc_unregister_callback");
    //Adding a invalid value for passing IPC/RPC, no other use
    INT32 i4_InvalidValue = 0;
    RPC_CLIENT_DECL(1, INT32);
    RPC_CLIENT_ARG_INP(ARG_TYPE_INT32, i4_InvalidValue);
    RPC_BT_SERVICE_CLIENT_DO_OP("x_mtkapi_bt_gattc_unregister_callback");
    RPC_CLIENT_RETURN(ARG_TYPE_INT32, -1);
}

EXPORT_SYMBOL INT32 a_mtkapi_bt_gattc_close(INT32 client_if, CHAR *bt_addr,
                                                    INT32 conn_id)
{
    BT_RW_LOG("a_mtkapi_bt_gattc_close");
    RPC_CLIENT_DECL(3, INT32);
    RPC_CLIENT_ARG_INP(ARG_TYPE_INT32, client_if);
    RPC_CLIENT_ARG_INP(ARG_TYPE_REF_STR, bt_addr);
    RPC_CLIENT_ARG_INP(ARG_TYPE_INT32, conn_id);
    RPC_BT_SERVICE_CLIENT_DO_OP("x_mtkapi_bt_gattc_disconnect");
    RPC_CLIENT_RETURN(ARG_TYPE_INT32, -1);
}
EXPORT_SYMBOL INT32 a_mtkapi_bt_gattc_listen(INT32 client_if)
{
    BT_RW_LOG("a_mtkapi_bt_gattc_listen");
    RPC_CLIENT_DECL(1, INT32);
    RPC_CLIENT_ARG_INP(ARG_TYPE_INT32, client_if);
    RPC_BT_SERVICE_CLIENT_DO_OP("x_mtkapi_bt_gattc_listen");
    RPC_CLIENT_RETURN(ARG_TYPE_INT32, -1);
}

EXPORT_SYMBOL INT32 a_mtkapi_bt_gattc_refresh(INT32 client_if, CHAR *bt_addr)
{
    BT_RW_LOG("a_mtkapi_bt_gattc_refresh");
    RPC_CLIENT_DECL(2, INT32);
    RPC_CLIENT_ARG_INP(ARG_TYPE_INT32, client_if);
    RPC_CLIENT_ARG_INP(ARG_TYPE_REF_STR, bt_addr);
    RPC_BT_SERVICE_CLIENT_DO_OP("x_mtkapi_bt_gattc_refresh");
    RPC_CLIENT_RETURN(ARG_TYPE_INT32, -1);
}

EXPORT_SYMBOL INT32 a_mtkapi_bt_gattc_search_service(INT32 conn_id, CHAR *uuid)
{
    BT_RW_LOG("a_mtkapi_bt_gattc_search_service");
    RPC_CLIENT_DECL(2, INT32);
    RPC_CLIENT_ARG_INP(ARG_TYPE_INT32, conn_id);
    RPC_CLIENT_ARG_INP(ARG_TYPE_REF_STR, uuid);
    RPC_BT_SERVICE_CLIENT_DO_OP("x_mtkapi_bt_gattc_search_service");
    RPC_CLIENT_RETURN(ARG_TYPE_INT32, -1);
}

EXPORT_SYMBOL INT32 a_mtkapi_bt_gattc_get_gatt_db(INT32 conn_id)
{
    BT_RW_LOG("a_mtkapi_bt_gattc_get_gatt_db");
    RPC_CLIENT_DECL(1, INT32);
    RPC_CLIENT_ARG_INP(ARG_TYPE_INT32, conn_id);
    RPC_BT_SERVICE_CLIENT_DO_OP("x_mtkapi_bt_gattc_get_gatt_db");
    RPC_CLIENT_RETURN(ARG_TYPE_INT32, -1);
}

EXPORT_SYMBOL INT32 a_mtkapi_bt_gattc_read_char(INT32 conn_id,
                                                         INT32 char_handle,
                                                         INT32 auth_req)
{
    BT_RW_LOG("a_mtkapi_bt_gattc_read_char");
    RPC_CLIENT_DECL(3, INT32);
    RPC_CLIENT_ARG_INP(ARG_TYPE_INT32, conn_id);
    RPC_CLIENT_ARG_INP(ARG_TYPE_INT32, char_handle);
    RPC_CLIENT_ARG_INP(ARG_TYPE_INT32, auth_req);
    RPC_BT_SERVICE_CLIENT_DO_OP("x_mtkapi_bt_gattc_read_char");
    RPC_CLIENT_RETURN(ARG_TYPE_INT32, -1);
}

EXPORT_SYMBOL INT32 a_mtkapi_bt_gattc_read_descr(INT32 conn_id,
                                                           INT32 descr_handle,
                                                           INT32 auth_req)
{
    BT_RW_LOG("a_mtkapi_bt_gattc_read_descr");
    RPC_CLIENT_DECL(3, INT32);
    RPC_CLIENT_ARG_INP(ARG_TYPE_INT32, conn_id);
    RPC_CLIENT_ARG_INP(ARG_TYPE_INT32, descr_handle);
    RPC_CLIENT_ARG_INP(ARG_TYPE_INT32, auth_req);
    RPC_BT_SERVICE_CLIENT_DO_OP("x_mtkapi_bt_gattc_read_descr");
    RPC_CLIENT_RETURN(ARG_TYPE_INT32, -1);
}

EXPORT_SYMBOL INT32 a_mtkapi_bt_gattc_write_char(INT32 conn_id,
                                                          INT32 char_handle,
                                                          INT32 write_type,
                                                          INT32 len,
                                                          INT32 auth_req,
                                                          CHAR *value)
{
    BT_RW_LOG("a_mtkapi_bt_gattc_write_char");
    RPC_CLIENT_DECL(6, INT32);
    RPC_CLIENT_ARG_INP(ARG_TYPE_INT32, conn_id);
    RPC_CLIENT_ARG_INP(ARG_TYPE_INT32, char_handle);
    RPC_CLIENT_ARG_INP(ARG_TYPE_INT32, write_type);
    RPC_CLIENT_ARG_INP(ARG_TYPE_INT32, len);
    RPC_CLIENT_ARG_INP(ARG_TYPE_INT32, auth_req);
    RPC_CLIENT_CHECK(bt_rpc_add_ref_buff(RPC_DEFAULT_ID, value,len));
    RPC_CLIENT_ARG_INP(ARG_TYPE_REF_DESC, value);
    RPC_BT_SERVICE_CLIENT_DO_OP("x_mtkapi_bt_gattc_write_char");
    RPC_CLIENT_RETURN(ARG_TYPE_INT32, -1);
}

EXPORT_SYMBOL INT32 a_mtkapi_bt_gattc_write_descr(INT32 conn_id,
                                                            INT32 descr_handle,
                                                            INT32 write_type,
                                                            INT32 len,
                                                            INT32 auth_req,
                                                            CHAR *value)
{
    BT_RW_LOG("a_mtkapi_bt_gattc_write_descr");
    RPC_CLIENT_DECL(6, INT32);
    RPC_CLIENT_ARG_INP(ARG_TYPE_INT32, conn_id);
    RPC_CLIENT_ARG_INP(ARG_TYPE_INT32, descr_handle);
    RPC_CLIENT_ARG_INP(ARG_TYPE_INT32, write_type);
    RPC_CLIENT_ARG_INP(ARG_TYPE_INT32, len);
    RPC_CLIENT_ARG_INP(ARG_TYPE_INT32, auth_req);
    RPC_CLIENT_CHECK(bt_rpc_add_ref_buff(RPC_DEFAULT_ID, value,len));
    RPC_CLIENT_ARG_INP(ARG_TYPE_REF_DESC, value);
    RPC_BT_SERVICE_CLIENT_DO_OP("x_mtkapi_bt_gattc_write_descr");
    RPC_CLIENT_RETURN(ARG_TYPE_INT32, -1);
}

EXPORT_SYMBOL INT32 a_mtkapi_bt_gattc_execute_write(INT32 conn_id,
                                                               INT32 execute)
{
    BT_RW_LOG("a_mtkapi_bt_gattc_execute_write");
    RPC_CLIENT_DECL(2, INT32);
    RPC_CLIENT_ARG_INP(ARG_TYPE_INT32, conn_id);
    RPC_CLIENT_ARG_INP(ARG_TYPE_INT32, execute);
    RPC_BT_SERVICE_CLIENT_DO_OP("x_mtkapi_bt_gattc_execute_write");
    RPC_CLIENT_RETURN(ARG_TYPE_INT32, -1);
}

EXPORT_SYMBOL INT32 a_mtkapi_bt_gattc_reg_noti(INT32 client_if,
                                                       CHAR *bt_addr,
                                                       INT32 char_handle)
{
    BT_RW_LOG("a_mtkapi_bt_gattc_reg_noti");
    BOOL b_enable = TRUE;
    RPC_CLIENT_DECL(4, INT32);
    RPC_CLIENT_ARG_INP(ARG_TYPE_INT32, client_if);
    RPC_CLIENT_ARG_INP(ARG_TYPE_REF_STR, bt_addr);
    RPC_CLIENT_ARG_INP(ARG_TYPE_INT32, char_handle);
    RPC_CLIENT_ARG_INP(ARG_TYPE_BOOL, b_enable);
    RPC_BT_SERVICE_CLIENT_DO_OP("x_mtkapi_bt_gattc_set_reg_noti");
    RPC_CLIENT_RETURN(ARG_TYPE_INT32, -1);
}

EXPORT_SYMBOL INT32 a_mtkapi_bt_gattc_dereg_noti(INT32 client_if,
                                                          CHAR *bt_addr,
                                                          INT32 char_handle)
{
    BT_RW_LOG("a_mtkapi_bt_gattc_dereg_noti");
    BOOL b_enable = FALSE;
    RPC_CLIENT_DECL(4, INT32);
    RPC_CLIENT_ARG_INP(ARG_TYPE_INT32, client_if);
    RPC_CLIENT_ARG_INP(ARG_TYPE_REF_STR, bt_addr);
    RPC_CLIENT_ARG_INP(ARG_TYPE_INT32, char_handle);
    RPC_CLIENT_ARG_INP(ARG_TYPE_BOOL, b_enable);
    RPC_BT_SERVICE_CLIENT_DO_OP("x_mtkapi_bt_gattc_set_reg_noti");
    RPC_CLIENT_RETURN(ARG_TYPE_INT32, -1);
}

EXPORT_SYMBOL INT32 a_mtkapi_bt_gattc_read_rssi(INT32 client_if, CHAR *bt_addr)
{
    BT_RW_LOG("a_mtkapi_bt_gattc_read_rssi");
    RPC_CLIENT_DECL(2, INT32);
    RPC_CLIENT_ARG_INP(ARG_TYPE_INT32, client_if);
    RPC_CLIENT_ARG_INP(ARG_TYPE_REF_STR, bt_addr);
    RPC_BT_SERVICE_CLIENT_DO_OP("x_mtkapi_bt_gattc_read_rssi");
    RPC_CLIENT_RETURN(ARG_TYPE_INT32, -1);
    RPC_CLIENT_RETURN(ARG_TYPE_INT32, -1);
}

EXPORT_SYMBOL INT32
a_mtkapi_bt_gattc_scan_filter_param_setup(BT_GATTC_FILT_PARAM_SETUP_T *scan_filt_param)
{
    BT_RW_LOG("a_mtkapi_bt_gattc_read_rssi");
    RPC_CLIENT_DECL(1, INT32);
    RPC_CLIENT_CHECK(bt_rpc_add_ref_desc(RPC_DEFAULT_ID, scan_filt_param,
                                         RPC_DESC_BT_GATTC_FILT_PARAM_SETUP_T,
                                         NULL));
    RPC_CLIENT_ARG_INP(ARG_TYPE_REF_DESC, scan_filt_param);
    RPC_BT_SERVICE_CLIENT_DO_OP("x_mtkapi_bt_gattc_scan_filter_param_setup");
    RPC_CLIENT_RETURN(ARG_TYPE_INT32, -1);
}

EXPORT_SYMBOL INT32 a_mtkapi_bt_gattc_scan_filter_enable(INT32 client_if)
{
    BT_RW_LOG("a_mtkapi_bt_gattc_scan_filter_enable");
    BOOL b_enable = TRUE;
    RPC_CLIENT_DECL(2, INT32);
    RPC_CLIENT_ARG_INP(ARG_TYPE_INT32, client_if);
    RPC_CLIENT_ARG_INP(ARG_TYPE_BOOL, b_enable);
    RPC_BT_SERVICE_CLIENT_DO_OP("x_mtkapi_bt_gattc_scan_filter_enable");
    RPC_CLIENT_RETURN(ARG_TYPE_INT32, -1);
}

EXPORT_SYMBOL INT32 a_mtkapi_bt_gattc_scan_filter_disable(INT32 client_if)
{
    BT_RW_LOG("a_mtkapi_bt_gattc_scan_filter_disable");
    BOOL b_enable = FALSE;
    RPC_CLIENT_DECL(2, INT32);
    RPC_CLIENT_ARG_INP(ARG_TYPE_INT32, client_if);
    RPC_CLIENT_ARG_INP(ARG_TYPE_BOOL, b_enable);
    RPC_BT_SERVICE_CLIENT_DO_OP("x_mtkapi_bt_gattc_scan_filter_enable");
    RPC_CLIENT_RETURN(ARG_TYPE_INT32, -1);
}

EXPORT_SYMBOL INT32 a_mtkapi_bt_gattc_scan_filter_add_remove(INT32 client_if,
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
    BT_RW_LOG("a_mtkapi_bt_gattc_scan_filter_add_remove");
    RPC_CLIENT_DECL(14, INT32);
    RPC_CLIENT_ARG_INP(ARG_TYPE_INT32, client_if);
    RPC_CLIENT_ARG_INP(ARG_TYPE_INT32, action);
    RPC_CLIENT_ARG_INP(ARG_TYPE_INT32, filt_type);
    RPC_CLIENT_ARG_INP(ARG_TYPE_INT32, filt_index);
    RPC_CLIENT_ARG_INP(ARG_TYPE_INT32, company_id);
    RPC_CLIENT_ARG_INP(ARG_TYPE_INT32, company_id_mask);
    RPC_CLIENT_ARG_INP(ARG_TYPE_REF_STR, p_uuid);
    RPC_CLIENT_ARG_INP(ARG_TYPE_REF_STR, p_uuid_mask);
    RPC_CLIENT_ARG_INP(ARG_TYPE_REF_STR, bd_addr);
    RPC_CLIENT_ARG_INP(ARG_TYPE_CHAR, addr_type);
    RPC_CLIENT_ARG_INP(ARG_TYPE_INT32, data_len);
    RPC_CLIENT_ARG_INP(ARG_TYPE_REF_STR, p_data);
    RPC_CLIENT_ARG_INP(ARG_TYPE_INT32, mask_len);
    RPC_CLIENT_ARG_INP(ARG_TYPE_REF_STR, p_mask);
    RPC_BT_SERVICE_CLIENT_DO_OP("x_mtkapi_bt_gattc_scan_filter_add_remove");
    RPC_CLIENT_RETURN(ARG_TYPE_INT32, -1);
}

EXPORT_SYMBOL INT32 a_mtkapi_bt_gattc_scan_filter_clear(INT32 client_if,
                                                                  INT32 filt_index)
{
    BT_RW_LOG("a_mtkapi_bt_gattc_scan_filter_clear");
    RPC_CLIENT_DECL(2, INT32);
    RPC_CLIENT_ARG_INP(ARG_TYPE_INT32, client_if);
    RPC_CLIENT_ARG_INP(ARG_TYPE_INT32, filt_index);
    RPC_BT_SERVICE_CLIENT_DO_OP("x_mtkapi_bt_gattc_scan_filter_clear");
    RPC_CLIENT_RETURN(ARG_TYPE_INT32, -1);
}

EXPORT_SYMBOL INT32 a_mtkapi_bt_gattc_get_device_type(CHAR *bd_addr)
{
    BT_RW_LOG("a_mtkapi_bt_gattc_get_device_type");
    RPC_CLIENT_DECL(1, INT32);
    RPC_CLIENT_ARG_INP(ARG_TYPE_REF_STR, bd_addr);
    RPC_BT_SERVICE_CLIENT_DO_OP("x_mtkapi_bt_gattc_get_device_type");
    RPC_CLIENT_RETURN(ARG_TYPE_INT32, -1);
}

EXPORT_SYMBOL INT32 a_mtkapi_bt_gattc_configure_mtu(INT32 conn_id,
                                                               INT32 mtu)
{
    BT_RW_LOG("a_mtkapi_bt_gattc_configure_mtu");
    RPC_CLIENT_DECL(2, INT32);
    RPC_CLIENT_ARG_INP(ARG_TYPE_INT32, conn_id);
    RPC_CLIENT_ARG_INP(ARG_TYPE_INT32, mtu);
    RPC_BT_SERVICE_CLIENT_DO_OP("x_mtkapi_bt_gattc_configure_mtu");
    RPC_CLIENT_RETURN(ARG_TYPE_INT32, -1);
}

EXPORT_SYMBOL INT32 a_mtkapi_bt_gattc_conn_parameter_update(CHAR *bt_addr,
                                                                           INT32 min_interval,
                                                                           INT32 max_interval,
                                                                           INT32 latency,
                                                                           INT32 timeout)
{
    BT_RW_LOG("a_mtkapi_bt_gattc_conn_parameter_update");
    RPC_CLIENT_DECL(5, INT32);
    RPC_CLIENT_ARG_INP(ARG_TYPE_REF_STR, bt_addr);
    RPC_CLIENT_ARG_INP(ARG_TYPE_INT32, min_interval);
    RPC_CLIENT_ARG_INP(ARG_TYPE_INT32, max_interval);
    RPC_CLIENT_ARG_INP(ARG_TYPE_INT32, latency);
    RPC_CLIENT_ARG_INP(ARG_TYPE_INT32, timeout);
    RPC_BT_SERVICE_CLIENT_DO_OP("x_mtkapi_bt_gattc_conn_parameter_update");
    RPC_CLIENT_RETURN(ARG_TYPE_INT32, -1);
}

EXPORT_SYMBOL INT32 a_mtkapi_bt_gattc_set_scan_parameters(INT32 client_if,
                                                                        INT32 scan_interval,
                                                                        INT32 scan_window)
{
    BT_RW_LOG("a_mtkapi_bt_gattc_set_scan_parameters");
    RPC_CLIENT_DECL(3, INT32);
    RPC_CLIENT_ARG_INP(ARG_TYPE_INT32, client_if);
    RPC_CLIENT_ARG_INP(ARG_TYPE_INT32, scan_interval);
    RPC_CLIENT_ARG_INP(ARG_TYPE_INT32, scan_window);
    RPC_BT_SERVICE_CLIENT_DO_OP("x_mtkapi_bt_gattc_set_scan_parameters");
    RPC_CLIENT_RETURN(ARG_TYPE_INT32, -1);
}

EXPORT_SYMBOL INT32 a_mtkapi_bt_gattc_multi_adv_enable(INT32 client_if,
                                                                   INT32 min_interval,
                                                                   INT32 max_interval,
                                                                   INT32 adv_type,
                                                                   INT32 chnl_map,
                                                                   INT32 tx_power,
                                                                   INT32 timeout_s)
{
    BT_RW_LOG("a_mtkapi_bt_gattc_multi_adv_enable");
    RPC_CLIENT_DECL(7, INT32);
    RPC_CLIENT_ARG_INP(ARG_TYPE_INT32, client_if);
    RPC_CLIENT_ARG_INP(ARG_TYPE_INT32, min_interval);
    RPC_CLIENT_ARG_INP(ARG_TYPE_INT32, max_interval);
    RPC_CLIENT_ARG_INP(ARG_TYPE_INT32, adv_type);
    RPC_CLIENT_ARG_INP(ARG_TYPE_INT32, chnl_map);
    RPC_CLIENT_ARG_INP(ARG_TYPE_INT32, tx_power);
    RPC_CLIENT_ARG_INP(ARG_TYPE_INT32, timeout_s);
    RPC_BT_SERVICE_CLIENT_DO_OP("x_mtkapi_bt_gattc_multi_adv_enable");
    RPC_CLIENT_RETURN(ARG_TYPE_INT32, -1);
}

EXPORT_SYMBOL INT32 a_mtkapi_bt_gattc_set_disc_mode(INT32 client_if,
                                                                INT32 disc_mode)
{
    BT_RW_LOG("a_mtkapi_bt_gattc_set_disc_mode");
    RPC_CLIENT_DECL(2, INT32);
    RPC_CLIENT_ARG_INP(ARG_TYPE_INT32, client_if);
    RPC_CLIENT_ARG_INP(ARG_TYPE_INT32, disc_mode);
    RPC_BT_SERVICE_CLIENT_DO_OP("x_mtkapi_bt_gattc_set_disc_mode");
    RPC_CLIENT_RETURN(ARG_TYPE_INT32, -1);
}


EXPORT_SYMBOL INT32 a_mtkapi_bt_gattc_multi_adv_update(INT32 client_if,
                                                                   INT32 min_interval,
                                                                   INT32 max_interval,
                                                                   INT32 adv_type,
                                                                   INT32 chnl_map,
                                                                   INT32 tx_power,
                                                                   INT32 timeout_s)
{
    BT_RW_LOG("a_mtkapi_bt_gattc_multi_adv_update");
    RPC_CLIENT_DECL(7, INT32);
    RPC_CLIENT_ARG_INP(ARG_TYPE_INT32, client_if);
    RPC_CLIENT_ARG_INP(ARG_TYPE_INT32, min_interval);
    RPC_CLIENT_ARG_INP(ARG_TYPE_INT32, max_interval);
    RPC_CLIENT_ARG_INP(ARG_TYPE_INT32, adv_type);
    RPC_CLIENT_ARG_INP(ARG_TYPE_INT32, chnl_map);
    RPC_CLIENT_ARG_INP(ARG_TYPE_INT32, tx_power);
    RPC_CLIENT_ARG_INP(ARG_TYPE_INT32, timeout_s);
    RPC_BT_SERVICE_CLIENT_DO_OP("x_mtkapi_bt_gattc_multi_adv_update");
    RPC_CLIENT_RETURN(ARG_TYPE_INT32, -1);
}

EXPORT_SYMBOL INT32 a_mtkapi_bt_gattc_multi_adv_setdata(INT32 client_if,
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
    BT_RW_LOG("a_mtkapi_bt_gattc_multi_adv_setdata");
    RPC_CLIENT_DECL(11, INT32);
    RPC_CLIENT_ARG_INP(ARG_TYPE_INT32, client_if);
    RPC_CLIENT_ARG_INP(ARG_TYPE_UINT8, set_scan_rsp);
    RPC_CLIENT_ARG_INP(ARG_TYPE_UINT8, include_name);
    RPC_CLIENT_ARG_INP(ARG_TYPE_UINT8, include_txpower);
    RPC_CLIENT_ARG_INP(ARG_TYPE_INT32, appearance);
    RPC_CLIENT_ARG_INP(ARG_TYPE_INT32, manufacturer_len);
    RPC_CLIENT_CHECK(bt_rpc_add_ref_buff(RPC_DEFAULT_ID, manufacturer_data, manufacturer_len));
    RPC_CLIENT_ARG_INP(ARG_TYPE_REF_DESC, manufacturer_data);
    RPC_CLIENT_ARG_INP(ARG_TYPE_INT32, service_data_len);
    RPC_CLIENT_CHECK(bt_rpc_add_ref_buff(RPC_DEFAULT_ID, service_data, service_data_len));
    RPC_CLIENT_ARG_INP(ARG_TYPE_REF_DESC, service_data);
    RPC_CLIENT_ARG_INP(ARG_TYPE_INT32, service_uuid_len);
    RPC_CLIENT_CHECK(bt_rpc_add_ref_buff(RPC_DEFAULT_ID, service_uuid, service_uuid_len));
    RPC_CLIENT_ARG_INP(ARG_TYPE_REF_DESC, service_uuid);
    RPC_BT_SERVICE_CLIENT_DO_OP("x_mtkapi_bt_gattc_multi_adv_setdata");
    RPC_CLIENT_RETURN(ARG_TYPE_INT32, -1);
}

EXPORT_SYMBOL INT32 a_mtkapi_bt_gattc_multi_adv_disable(INT32 client_if)
{
    BT_RW_LOG("a_mtkapi_bt_gattc_multi_adv_disable");
    RPC_CLIENT_DECL(1, INT32);
    RPC_CLIENT_ARG_INP(ARG_TYPE_INT32, client_if);
    RPC_BT_SERVICE_CLIENT_DO_OP("x_mtkapi_bt_gattc_multi_adv_disable");
    RPC_CLIENT_RETURN(ARG_TYPE_INT32, -1);
}

EXPORT_SYMBOL INT32 a_mtkapi_bt_gattc_batchscan_cfg_storage(INT32 client_if,
                                                                          INT32 batch_scan_full_max,
                                                                          INT32 batch_scan_trunc_max,
                                                                          INT32 batch_scan_notify_threshold)
{
    BT_RW_LOG("a_mtkapi_bt_gattc_batchscan_cfg_storage");
    RPC_CLIENT_DECL(4, INT32);
    RPC_CLIENT_ARG_INP(ARG_TYPE_INT32, client_if);
    RPC_CLIENT_ARG_INP(ARG_TYPE_INT32, batch_scan_full_max);
    RPC_CLIENT_ARG_INP(ARG_TYPE_INT32, batch_scan_trunc_max);
    RPC_CLIENT_ARG_INP(ARG_TYPE_INT32, batch_scan_notify_threshold);
    RPC_BT_SERVICE_CLIENT_DO_OP("x_mtkapi_bt_gattc_batchscan_cfg_storage");
    RPC_CLIENT_RETURN(ARG_TYPE_INT32, -1);
}

EXPORT_SYMBOL INT32 a_mtkapi_bt_gattc_batchscan_enb_batch_scan(INT32 client_if,
                                                                                INT32 scan_mode,
                                                                                INT32 scan_interval,
                                                                                INT32 scan_window,
                                                                                INT32 addr_type,
                                                                                INT32 discard_rule)
{
    BT_RW_LOG("a_mtkapi_bt_gattc_batchscan_enb_batch_scan");
    RPC_CLIENT_DECL(6, INT32);
    RPC_CLIENT_ARG_INP(ARG_TYPE_INT32, client_if);
    RPC_CLIENT_ARG_INP(ARG_TYPE_INT32, scan_mode);
    RPC_CLIENT_ARG_INP(ARG_TYPE_INT32, scan_interval);
    RPC_CLIENT_ARG_INP(ARG_TYPE_INT32, scan_window);
    RPC_CLIENT_ARG_INP(ARG_TYPE_INT32, addr_type);
    RPC_CLIENT_ARG_INP(ARG_TYPE_INT32, discard_rule);
    RPC_BT_SERVICE_CLIENT_DO_OP("x_mtkapi_bt_gattc_batchscan_enb_batch_scan");
    RPC_CLIENT_RETURN(ARG_TYPE_INT32, -1);
}

EXPORT_SYMBOL INT32 a_mtkapi_bt_gattc_batchscan_dis_batch_scan(INT32 client_if)
{
    BT_RW_LOG("a_mtkapi_bt_gattc_batchscan_dis_batch_scan");
    RPC_CLIENT_DECL(1, INT32);
    RPC_CLIENT_ARG_INP(ARG_TYPE_INT32, client_if);
    RPC_BT_SERVICE_CLIENT_DO_OP("x_mtkapi_bt_gattc_batchscan_dis_batch_scan");
    RPC_CLIENT_RETURN(ARG_TYPE_INT32, -1);
}

EXPORT_SYMBOL INT32 a_mtkapi_bt_gattc_batchscan_read_reports(INT32 client_if,
                                                                           INT32 scan_mode)
{
    BT_RW_LOG("a_mtkapi_bt_gattc_batchscan_read_reports");
    RPC_CLIENT_DECL(2, INT32);
    RPC_CLIENT_ARG_INP(ARG_TYPE_INT32, client_if);
    RPC_CLIENT_ARG_INP(ARG_TYPE_INT32, scan_mode);
    RPC_BT_SERVICE_CLIENT_DO_OP("x_mtkapi_bt_gattc_batchscan_read_reports");
    RPC_CLIENT_RETURN(ARG_TYPE_INT32, -1);
}


EXPORT_SYMBOL INT32 a_mtkapi_bt_gattc_set_local_le_name(INT32 client_if,
                                                                     CHAR *le_name)
{
#if MTK_LINUX_GATTC_LE_NAME
    BT_RW_LOG("a_mtkapi_bt_gattc_set_local_le_name");
    RPC_CLIENT_DECL(2, INT32);
    RPC_CLIENT_ARG_INP(ARG_TYPE_INT32, client_if);
    RPC_CLIENT_ARG_INP(ARG_TYPE_REF_STR, le_name);
    RPC_BT_SERVICE_CLIENT_DO_OP("x_mtkapi_bt_gattc_set_local_le_name");
    RPC_CLIENT_RETURN(ARG_TYPE_INT32, -1);
#else
    return RPCR_OK;
#endif
}

EXPORT_SYMBOL VOID a_mtkapi_bt_gattc_get_connect_result_info(BT_GATTC_CONNECT_RST_T *connect_rst_info)
{
    BT_RW_LOG("a_mtkapi_bt_gattc_get_connect_result_info");
    RPC_CLIENT_DECL_VOID(1);
    RPC_CLIENT_CHECK(bt_rpc_add_ref_desc(RPC_DEFAULT_ID, connect_rst_info,
                                         RPC_DESC_BT_GATTC_CONNECT_RST_T, NULL));
    RPC_CLIENT_ARG_IO(ARG_TYPE_REF_DESC, connect_rst_info);
    RPC_BT_SERVICE_CLIENT_DO_OP("x_mtkapi_bt_gattc_get_connect_result_info");
    RPC_RETURN_VOID;
}

EXPORT_SYMBOL VOID a_mtkapi_bt_gattc_get_disconnect_result_info(BT_GATTC_CONNECT_RST_T *disconnect_rst_info)
{
    BT_RW_LOG("a_mtkapi_bt_gattc_get_disconnect_result_info");
    RPC_CLIENT_DECL_VOID(1);
    RPC_CLIENT_CHECK(bt_rpc_add_ref_desc(RPC_DEFAULT_ID, disconnect_rst_info,
                                         RPC_DESC_BT_GATTC_CONNECT_RST_T, NULL));
    RPC_CLIENT_ARG_IO(ARG_TYPE_REF_DESC, disconnect_rst_info);
    RPC_BT_SERVICE_CLIENT_DO_OP("x_mtkapi_bt_gattc_get_disconnect_result_info");
    RPC_RETURN_VOID;
}

EXPORT_SYMBOL VOID a_mtkapi_bt_gattc_read_rssi_result_info(BT_GATTC_GET_REMOTE_RSSI_T *get_remote_rssi_info)
{
    BT_RW_LOG("a_mtkapi_bt_gattc_read_rssi_result_info");
    RPC_CLIENT_DECL_VOID(1);
    RPC_CLIENT_CHECK(bt_rpc_add_ref_desc(RPC_DEFAULT_ID, get_remote_rssi_info,
                                         RPC_DESC_BT_GATTC_GET_REMOTE_RSSI_T, NULL));
    RPC_CLIENT_ARG_IO(ARG_TYPE_REF_DESC, get_remote_rssi_info);
    RPC_BT_SERVICE_CLIENT_DO_OP("x_mtkapi_bt_gattc_read_rssi_result_info");
    RPC_RETURN_VOID;
}

INT32 c_rpc_reg_mtk_bt_service_gattc_cb_hndlrs(VOID)
{
    int i4_ret = 0;
    RPC_REG_CB_HNDLR(bt_app_gattc_event_cbk);
    RPC_REG_CB_HNDLR(bt_app_gattc_reg_client_cbk);
    RPC_REG_CB_HNDLR(bt_app_gattc_scan_result_cbk);
    RPC_REG_CB_HNDLR(bt_app_gattc_get_gatt_db_cbk);
    RPC_REG_CB_HNDLR(bt_app_gattc_get_reg_noti_cbk);
    RPC_REG_CB_HNDLR(bt_app_gattc_notify_cbk);
    RPC_REG_CB_HNDLR(bt_app_gattc_read_char_cbk);
    RPC_REG_CB_HNDLR(bt_app_gattc_write_char_cbk);
    RPC_REG_CB_HNDLR(bt_app_gattc_read_desc_cbk);
    RPC_REG_CB_HNDLR(bt_app_gattc_write_desc_cbk);
    RPC_REG_CB_HNDLR(bt_app_gattc_scan_filter_param_cbk);
    RPC_REG_CB_HNDLR(bt_app_gattc_scan_filter_status_cbk);
    RPC_REG_CB_HNDLR(bt_app_gattc_scan_filter_cfg_cbk);
    RPC_REG_CB_HNDLR(bt_app_gattc_adv_enable_cbk);
    return RPCR_OK;
}

