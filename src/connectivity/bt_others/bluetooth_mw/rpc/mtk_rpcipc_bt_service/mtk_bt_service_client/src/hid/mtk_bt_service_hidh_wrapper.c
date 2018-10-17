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

#include "mtk_bt_service_hidh_wrapper.h"
#include "mtk_bt_service_hidh_ipcrpc_struct.h"
#include "client_common.h"

#define BT_HIDH_LOG(_stmt...) \
        do{ \
            if(0){    \
                printf("[HIDH]Func:%s Line:%d--->: ", __FUNCTION__, __LINE__);   \
                printf(_stmt); \
                printf("\n"); \
            }        \
        }   \
        while(0)


EXPORT_SYMBOL INT32 a_mtkapi_hidh_register_callback(mtkrpcapi_BtAppHidhCbk hidh_handle, VOID* pv_tag)
{
    RPC_CLIENT_DECL(2, INT32);

    RPC_CLIENT_ARG_INP(ARG_TYPE_REF_FUNC, hidh_handle);
    RPC_CLIENT_ARG_INP(ARG_TYPE_REF_VOID, pv_tag);
    BT_HIDH_LOG("hidh_handle=%p, pv_tag=%p", hidh_handle, pv_tag);

    RPC_BT_SERVICE_CLIENT_DO_OP("x_mtkapi_hidh_register_callback");
    RPC_CLIENT_RETURN(ARG_TYPE_INT32, -1);
}



EXPORT_SYMBOL INT32 a_mtkapi_hidh_connect(CHAR *pbt_addr)
{
    BT_HIDH_LOG("a_mtkapi_hidh_connect [%s]", pbt_addr);
    RPC_CLIENT_DECL(1, INT32);
    RPC_CLIENT_ARG_INP(ARG_TYPE_REF_STR, pbt_addr);
    RPC_BT_SERVICE_CLIENT_DO_OP("x_mtkapi_hidh_connect");
    RPC_CLIENT_RETURN(ARG_TYPE_INT32, -1);
}

EXPORT_SYMBOL INT32 a_mtkapi_hidh_disconnect(CHAR *pbt_addr)
{
    BT_HIDH_LOG("a_mtkapi_hidh_disconnect [%s]", pbt_addr);
    RPC_CLIENT_DECL(1, INT32);
    RPC_CLIENT_ARG_INP(ARG_TYPE_REF_STR, pbt_addr);
    RPC_BT_SERVICE_CLIENT_DO_OP("x_mtkapi_hidh_disconnect");
    RPC_CLIENT_RETURN(ARG_TYPE_INT32, -1);
}

EXPORT_SYMBOL INT32 a_mtkapi_hidh_set_output_report(CHAR *pbt_addr, CHAR *preport_data)
{
    BT_HIDH_LOG("a_mtkapi_hidh_set_output_report");
    RPC_CLIENT_DECL(2, INT32);
    RPC_CLIENT_ARG_INP(ARG_TYPE_REF_STR, pbt_addr);
    RPC_CLIENT_ARG_INP(ARG_TYPE_REF_STR, preport_data);
    RPC_BT_SERVICE_CLIENT_DO_OP("x_mtkapi_hidh_set_output_report");
    RPC_CLIENT_RETURN(ARG_TYPE_INT32, -1);
}

EXPORT_SYMBOL INT32 a_mtkapi_hidh_get_input_report(CHAR *pbt_addr, UINT8 reportId, INT32 bufferSize)
{
    BT_HIDH_LOG("a_mtkapi_hidh_get_input_report");
    RPC_CLIENT_DECL(3, INT32);
    RPC_CLIENT_ARG_INP(ARG_TYPE_REF_STR, pbt_addr);
    RPC_CLIENT_ARG_INP(ARG_TYPE_UINT8, reportId);
    RPC_CLIENT_ARG_INP(ARG_TYPE_INT32, bufferSize);
    RPC_BT_SERVICE_CLIENT_DO_OP("x_mtkapi_hidh_get_input_report");
    RPC_CLIENT_RETURN(ARG_TYPE_INT32, -1);
}

EXPORT_SYMBOL INT32 a_mtkapi_hidh_get_output_report(CHAR *pbt_addr, UINT8 reportId, INT32 bufferSize)
{
    BT_HIDH_LOG("a_mtkapi_hidh_get_output_report");
    RPC_CLIENT_DECL(3, INT32);
    RPC_CLIENT_ARG_INP(ARG_TYPE_REF_STR, pbt_addr);
    RPC_CLIENT_ARG_INP(ARG_TYPE_UINT8, reportId);
    RPC_CLIENT_ARG_INP(ARG_TYPE_INT32, bufferSize);
    RPC_BT_SERVICE_CLIENT_DO_OP("x_mtkapi_hidh_get_output_report");
    RPC_CLIENT_RETURN(ARG_TYPE_INT32, -1);
}

EXPORT_SYMBOL INT32 a_mtkapi_hidh_set_input_report(CHAR *pbt_addr, CHAR *preport_data)
{
    BT_HIDH_LOG("a_mtkapi_hidh_set_input_report");
    RPC_CLIENT_DECL(2, INT32);
    RPC_CLIENT_ARG_INP(ARG_TYPE_REF_STR, pbt_addr);
    RPC_CLIENT_ARG_INP(ARG_TYPE_REF_STR, preport_data);
    RPC_BT_SERVICE_CLIENT_DO_OP("x_mtkapi_hidh_set_input_report");
    RPC_CLIENT_RETURN(ARG_TYPE_INT32, -1);
}

EXPORT_SYMBOL INT32 a_mtkapi_hidh_get_feature_report(CHAR *pbt_addr, UINT8 reportId, INT32 bufferSize)
{
    BT_HIDH_LOG("a_mtkapi_hidh_get_feature_report");
    RPC_CLIENT_DECL(3, INT32);
    RPC_CLIENT_ARG_INP(ARG_TYPE_REF_STR, pbt_addr);
    RPC_CLIENT_ARG_INP(ARG_TYPE_UINT8, reportId);
    RPC_CLIENT_ARG_INP(ARG_TYPE_INT32, bufferSize);
    RPC_BT_SERVICE_CLIENT_DO_OP("x_mtkapi_hidh_get_feature_report");
    RPC_CLIENT_RETURN(ARG_TYPE_INT32, -1);
}

EXPORT_SYMBOL INT32 a_mtkapi_hidh_set_feature_report(CHAR *pbt_addr, CHAR *preport_data)
{
    BT_HIDH_LOG("a_mtkapi_hidh_set_feature_report");
    RPC_CLIENT_DECL(2, INT32);
    RPC_CLIENT_ARG_INP(ARG_TYPE_REF_STR, pbt_addr);
    RPC_CLIENT_ARG_INP(ARG_TYPE_REF_STR, preport_data);
    RPC_BT_SERVICE_CLIENT_DO_OP("x_mtkapi_hidh_set_feature_report");
    RPC_CLIENT_RETURN(ARG_TYPE_INT32, -1);
}

EXPORT_SYMBOL INT32 a_mtkapi_hidh_get_protocol(CHAR *pbt_addr)
{
    BT_HIDH_LOG("a_mtkapi_hidh_get_protocol");
    RPC_CLIENT_DECL(1, INT32);
    RPC_CLIENT_ARG_INP(ARG_TYPE_REF_STR, pbt_addr);
    RPC_BT_SERVICE_CLIENT_DO_OP("x_mtkapi_hidh_get_protocol");
    RPC_CLIENT_RETURN(ARG_TYPE_INT32, -1);
}

EXPORT_SYMBOL INT32 a_mtkapi_hidh_set_protocol(CHAR *pbt_addr, UINT8 protocol_mode)
{
    BT_HIDH_LOG("a_mtkapi_hidh_set_protocol");
    RPC_CLIENT_DECL(2, INT32);
    RPC_CLIENT_ARG_INP(ARG_TYPE_REF_STR, pbt_addr);
    RPC_CLIENT_ARG_INP(ARG_TYPE_UINT8, protocol_mode);
    RPC_BT_SERVICE_CLIENT_DO_OP("x_mtkapi_hidh_set_protocol");
    RPC_CLIENT_RETURN(ARG_TYPE_INT32, -1);
}

EXPORT_SYMBOL INT32 a_mtkapi_hidh_send_control(CHAR *pbt_addr, UINT8 pcontrol_mode)
{
    BT_HIDH_LOG("a_mtkapi_hidh_send_control");
    RPC_CLIENT_DECL(2, INT32);
    RPC_CLIENT_ARG_INP(ARG_TYPE_REF_STR, pbt_addr);
    RPC_CLIENT_ARG_INP(ARG_TYPE_UINT8, pcontrol_mode);
    RPC_BT_SERVICE_CLIENT_DO_OP("x_mtkapi_hidh_send_control");
    RPC_CLIENT_RETURN(ARG_TYPE_INT32, -1);
}

EXPORT_SYMBOL INT32 a_mtkapi_bluetooth_hidh_set_output_report(CHAR *pbt_addr, CHAR *preport_data)
{
    BT_HIDH_LOG("a_mtkapi_bluetooth_hidh_set_output_report");
    RPC_CLIENT_DECL(2, INT32);
    RPC_CLIENT_ARG_INP(ARG_TYPE_REF_STR, pbt_addr);
    RPC_CLIENT_ARG_INP(ARG_TYPE_REF_STR, preport_data);
    RPC_BT_SERVICE_CLIENT_DO_OP("x_mtkapi_bluetooth_hidh_set_output_report");
    RPC_CLIENT_RETURN(ARG_TYPE_INT32, -1);
}


EXPORT_SYMBOL INT32 a_mtkapi_hidh_disconnect_all(VOID)
{
    BT_HIDH_LOG("a_mtkapi_hidh_disconnect_all");
    //Adding a invalid value for passing IPC/RPC, no other use
    INT32 i4_InvalidValue = 0;
    RPC_CLIENT_DECL(1, INT32);
    RPC_CLIENT_ARG_INP(ARG_TYPE_INT32, i4_InvalidValue);
    RPC_BT_SERVICE_CLIENT_DO_OP("x_mtkapi_hidh_disconnect_all");
    RPC_CLIENT_RETURN(ARG_TYPE_INT32, -1);
}

static INT32 _hndlr_bt_app_hidh_event_cbk(
    RPC_ID_T     t_rpc_id,
    const CHAR*  ps_cb_type,
    void          *pv_cb_addr,
    UINT32       ui4_num_args,
    ARG_DESC_T*  pt_args,
    ARG_DESC_T*  pt_return)
{
    BT_HIDH_LOG("_hndlr_bt_app_hidh_event_cbk ui4_num_args = %d\n", ui4_num_args);
    if(ui4_num_args != 2)
    {
        return RPCR_INV_ARGS;
    }
    pt_return->e_type   = ARG_TYPE_VOID;

    ((mtkrpcapi_BtAppHidhCbk)pv_cb_addr)((BT_HIDH_CBK_STRUCT*)pt_args[0].u.pv_desc, pt_args[1].u.pv_arg);
    return RPCR_OK;
}

INT32 c_rpc_reg_mtk_bt_service_hidh_cb_hndlrs(VOID)
{
    int i4_ret = 0;
    RPC_REG_CB_HNDLR(bt_app_hidh_event_cbk);
    return RPCR_OK;
}

