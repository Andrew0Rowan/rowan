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

#include "mtk_bt_service_gap_wrapper.h"
#include "mtk_bt_service_gap_ipcrpc_struct.h"
#include "client_common.h"


#define BT_RW_LOG(_stmt...) \
        do{ \
            if(0){    \
                printf("[Client]Func:%s Line:%d--->: ", __FUNCTION__, __LINE__);   \
                printf(_stmt); \
                printf("\n"); \
            }        \
        }   \
        while(0)

static INT32 _hndlr_bt_app_gap_inquiry_response_cbk(
    RPC_ID_T     t_rpc_id,
    const CHAR*  ps_cb_type,
    void          *pv_cb_addr,
    UINT32       ui4_num_args,
    ARG_DESC_T*  pt_args,
    ARG_DESC_T*  pt_return)
{
    BT_RW_LOG("bt_app_gap_inquiry_response_cbk, pv_cb_addr = %p", pv_cb_addr);

    if(ui4_num_args != 2)
    {
        return RPCR_INV_ARGS;
    }
    pt_return->e_type   = ARG_TYPE_VOID;

    ((mtkrpcapi_BtAppGapInquiryResponseCbk)pv_cb_addr)((tBTMW_GAP_DEVICE_INFO *)pt_args[0].u.pv_desc, pt_args[1].u.pv_arg);
    return RPCR_OK;
}

static INT32 _hndlr_bt_app_gap_event_cbk(
    RPC_ID_T     t_rpc_id,
    const CHAR*  ps_cb_type,
    void          *pv_cb_addr,
    UINT32       ui4_num_args,
    ARG_DESC_T*  pt_args,
    ARG_DESC_T*  pt_return)
{
    BT_RW_LOG("bt_app_gap_event_cbk, pv_cb_addr = %p", pv_cb_addr);

    if(ui4_num_args != 2)
    {
        return RPCR_INV_ARGS;
    }
    pt_return->e_type   = ARG_TYPE_VOID;

    ((mtkrpcapi_BtAppGapEventCbk)pv_cb_addr)((tBTMW_GAP_STATE*)pt_args[0].u.pv_desc, pt_args[1].u.pv_arg);
    return RPCR_OK;
}

static INT32 _hndlr_bt_app_gap_get_pairing_key_cbk(
    RPC_ID_T     t_rpc_id,
    const CHAR*  ps_cb_type,
    void          *pv_cb_addr,
    UINT32       ui4_num_args,
    ARG_DESC_T*  pt_args,
    ARG_DESC_T*  pt_return)
{
    BT_RW_LOG("bt_app_gap_get_pairing_key_cbk, pv_cb_addr = %p", pv_cb_addr);

    if(ui4_num_args != 3)
    {
        return RPCR_INV_ARGS;
    }
    pt_return->e_type   = ARG_TYPE_VOID;

    ((mtkrpcapi_BtAppGapGetPairingKeyCbk)pv_cb_addr)((pairing_key_value_t *)pt_args[0].u.pv_desc, pt_args[1].u.pui1_arg, pt_args[2].u.pv_arg);
    return RPCR_OK;
}

EXPORT_SYMBOL INT32 a_mtkapi_gap_bt_base_init(MTKRPCAPI_BT_APP_CB_FUNC * func, VOID* pv_tag)
{
    BT_RW_LOG("a_mtkapi_gap_bt_base_init");
    RPC_CLIENT_DECL(2, INT32);
    RPC_CLIENT_CHECK(bt_rpc_add_ref_desc(RPC_DEFAULT_ID,
                    func,
                    RPC_DESC_MTKRPCAPI_BT_APP_CB_FUNC,
                    NULL));
    RPC_CLIENT_ARG_INP(ARG_TYPE_REF_DESC, func);
    RPC_CLIENT_ARG_INP(ARG_TYPE_REF_VOID, pv_tag);
    RPC_BT_SERVICE_CLIENT_DO_OP("x_mtkapi_bt_base_init");
    RPC_CLIENT_RETURN(ARG_TYPE_INT32, -1);
    return RPCR_OK;
}

EXPORT_SYMBOL INT32 a_mtkapi_bt_gap_set_dbg_level(BT_DEBUG_LAYER_NAME_T layer, INT32 level)
{
    RPC_CLIENT_DECL(2, INT32);
    RPC_CLIENT_ARG_INP(ARG_TYPE_INT32, layer);
    RPC_CLIENT_ARG_INP(ARG_TYPE_INT32, level);
    RPC_BT_SERVICE_CLIENT_DO_OP("x_mtkapi_bt_gap_set_dbg_level");
    RPC_CLIENT_RETURN(ARG_TYPE_INT32, -1);
}

EXPORT_SYMBOL INT32 a_mtkapi_bt_gap_on_off(BOOL fg_on)
{
    RPC_CLIENT_DECL(1, INT32);
    RPC_CLIENT_ARG_INP(ARG_TYPE_BOOL, fg_on);
    RPC_BT_SERVICE_CLIENT_DO_OP("x_mtkapi_bt_gap_on_off");
    RPC_CLIENT_RETURN(ARG_TYPE_INT32, -1);
}

EXPORT_SYMBOL INT32 a_mtkapi_bt_bluetooth_factory_reset(VOID)
{
    //Adding a invalid value for passing IPC/RPC, no other use
    INT32 i4_InvalidValue = 0;
    RPC_CLIENT_DECL(1, INT32);
    RPC_CLIENT_ARG_INP(ARG_TYPE_INT32, i4_InvalidValue);
    RPC_BT_SERVICE_CLIENT_DO_OP("x_mtkapi_bt_bluetooth_factory_reset");
    RPC_CLIENT_RETURN(ARG_TYPE_INT32, -1);
}

EXPORT_SYMBOL INT32 a_mtkapi_bt_mw_log_setStackLevel(const char *trc_name, int level)
{
    //Adding a invalid value for passing IPC/RPC, no other use
    RPC_CLIENT_DECL(2, INT32);
    RPC_CLIENT_ARG_INP(ARG_TYPE_REF_STR, trc_name);
    RPC_CLIENT_ARG_INP(ARG_TYPE_INT32, level);
    RPC_BT_SERVICE_CLIENT_DO_OP("x_mtkapi_bt_mw_log_setStackLevel");
    RPC_CLIENT_RETURN(ARG_TYPE_INT32, -1);
}

EXPORT_SYMBOL INT32 a_mtkapi_bt_gap_set_name(CHAR *name)
{
    RPC_CLIENT_DECL(1, INT32);
    RPC_CLIENT_ARG_INP(ARG_TYPE_REF_STR, name);
    RPC_BT_SERVICE_CLIENT_DO_OP("x_mtkapi_bt_gap_set_name");
    RPC_CLIENT_RETURN(ARG_TYPE_INT32, -1);
}

EXPORT_SYMBOL INT32 a_mtkapi_bt_gap_set_connectable_and_discoverable(BOOL fg_conn, BOOL fg_disc)
{
    RPC_CLIENT_DECL(2, INT32);
    RPC_CLIENT_ARG_INP(ARG_TYPE_BOOL, fg_conn);
    RPC_CLIENT_ARG_INP(ARG_TYPE_BOOL, fg_disc);
    RPC_BT_SERVICE_CLIENT_DO_OP("x_mtkapi_bt_gap_set_connectable_and_discoverable");
    RPC_CLIENT_RETURN(ARG_TYPE_INT32, -1);
}

EXPORT_SYMBOL INT32 a_mtkapi_bt_gap_get_dev_info(BLUETOOTH_DEVICE* dev_info, CHAR* bd_addr)
{
    RPC_CLIENT_DECL(2, INT32);
    RPC_CLIENT_CHECK(bt_rpc_add_ref_desc(RPC_DEFAULT_ID,
                    dev_info,
                    RPC_DESC_BLUETOOTH_DEVICE,
                    NULL));
    RPC_CLIENT_ARG_OUT(ARG_TYPE_REF_DESC, dev_info);
    RPC_CLIENT_ARG_INP(ARG_TYPE_REF_STR, bd_addr);
    RPC_BT_SERVICE_CLIENT_DO_OP("x_mtkapi_bt_gap_get_dev_info");
    RPC_CLIENT_RETURN(ARG_TYPE_INT32, -1);
}

EXPORT_SYMBOL INT32 a_mtkapi_bt_gap_get_local_dev_info(BT_LOCAL_DEV *ps_dev_info)
{
    RPC_CLIENT_DECL(1, INT32);
    RPC_CLIENT_CHECK(bt_rpc_add_ref_desc(RPC_DEFAULT_ID,
                    ps_dev_info,
                    RPC_DESC_BT_LOCAL_DEV,
                    NULL));
    RPC_CLIENT_ARG_OUT(ARG_TYPE_REF_DESC, ps_dev_info);
    RPC_BT_SERVICE_CLIENT_DO_OP("x_mtkapi_bt_gap_get_local_dev_info");
    RPC_CLIENT_RETURN(ARG_TYPE_INT32, -1);
}

EXPORT_SYMBOL INT32 a_mtkapi_bt_gap_start_inquiry_scan(UINT32 ui4_filter_type)
{
    RPC_CLIENT_DECL(1, INT32);
    RPC_CLIENT_ARG_INP(ARG_TYPE_UINT32, ui4_filter_type);
    RPC_BT_SERVICE_CLIENT_DO_OP("x_mtkapi_bt_gap_start_inquiry_scan");
    RPC_CLIENT_RETURN(ARG_TYPE_INT32, -1);
}

EXPORT_SYMBOL INT32 a_mtkapi_bt_gap_stop_inquiry_scan(VOID)
{
    //Adding a invalid value for passing IPC/RPC, no other use
    INT32 i4_InvalidValue = 0;
    RPC_CLIENT_DECL(1, INT32);
    RPC_CLIENT_ARG_INP(ARG_TYPE_INT32, i4_InvalidValue);
    RPC_BT_SERVICE_CLIENT_DO_OP("x_mtkapi_bt_gap_stop_inquiry_scan");
    RPC_CLIENT_RETURN(ARG_TYPE_INT32, -1);
}

EXPORT_SYMBOL INT32 a_mtkapi_bt_gap_pair(CHAR *addr, int transport)
{
    RPC_CLIENT_DECL(2, INT32);
    RPC_CLIENT_ARG_INP(ARG_TYPE_REF_STR, addr);
    RPC_CLIENT_ARG_INP(ARG_TYPE_INT32, transport);
    RPC_BT_SERVICE_CLIENT_DO_OP("x_mtkapi_bt_gap_pair");
    RPC_CLIENT_RETURN(ARG_TYPE_INT32, -1);
}

EXPORT_SYMBOL INT32 a_mtkapi_bt_gap_unpair(CHAR *addr)
{
    RPC_CLIENT_DECL(1, INT32);
    RPC_CLIENT_ARG_INP(ARG_TYPE_REF_STR, addr);
    RPC_BT_SERVICE_CLIENT_DO_OP("x_mtkapi_bt_gap_unpair");
    RPC_CLIENT_RETURN(ARG_TYPE_INT32, -1);
}

EXPORT_SYMBOL INT32 a_mtkapi_bt_gap_get_rssi(CHAR *address, INT16 *rssi_value)
{
    RPC_CLIENT_DECL(2, INT32);
    RPC_CLIENT_ARG_INP(ARG_TYPE_REF_STR, address);
    RPC_CLIENT_ARG_OUT(ARG_TYPE_REF_INT16, rssi_value);
    RPC_BT_SERVICE_CLIENT_DO_OP("x_mtkapi_bt_gap_get_rssi");
    RPC_CLIENT_RETURN(ARG_TYPE_INT32, -1);
}

EXPORT_SYMBOL INT32 a_mtkapi_bt_gap_set_virtual_sniffer(INT32 enable)
{
    RPC_CLIENT_DECL(1, INT32);
    RPC_CLIENT_ARG_INP(ARG_TYPE_INT32, enable);
    RPC_BT_SERVICE_CLIENT_DO_OP("x_mtkapi_bt_gap_set_virtual_sniffer");
    RPC_CLIENT_RETURN(ARG_TYPE_INT32, -1);
}

EXPORT_SYMBOL INT32 a_mtkapi_bt_gap_send_hci(CHAR *buffer)
{
    RPC_CLIENT_DECL(1, INT32);
    RPC_CLIENT_ARG_INP(ARG_TYPE_REF_STR, buffer);
    RPC_BT_SERVICE_CLIENT_DO_OP("x_mtkapi_bt_gap_send_hci");
    RPC_CLIENT_RETURN(ARG_TYPE_INT32, -1);
}

INT32 c_rpc_reg_mtk_bt_service_gap_cb_hndlrs(VOID)
{
    int i4_ret = 0;
    RPC_REG_CB_HNDLR(bt_app_gap_inquiry_response_cbk);
    RPC_REG_CB_HNDLR(bt_app_gap_event_cbk);
    RPC_REG_CB_HNDLR(bt_app_gap_get_pairing_key_cbk);
    return RPCR_OK;
}


