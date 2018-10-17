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

#include "mtk_bt_service_hfclient_wrapper.h"
#include "mtk_bt_service_hfclient_ipcrpc_struct.h"
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

static INT32 _hndlr_bt_app_hfclient_event_cbk(
    RPC_ID_T     t_rpc_id,
    const CHAR*  ps_cb_type,
    void          *pv_cb_addr,
    UINT32       ui4_num_args,
    ARG_DESC_T*  pt_args,
    ARG_DESC_T*  pt_return)
{
    if (ui4_num_args != 2)
    {
        return RPCR_INV_ARGS;
    }
    pt_return->e_type   = ARG_TYPE_VOID;

    ((mtkrpcapi_BtAppHfclientCbk)pv_cb_addr)
        ((BT_HFCLIENT_EVENT_PARAM*)pt_args[0].u.pv_desc, pt_args[1].u.pv_arg);
    return RPCR_OK;
}

EXPORT_SYMBOL INT32 a_mtkapi_bt_hfclient_enable(VOID)
{
    INT32 i4_InvalidValue = 0;
    BT_RW_LOG("a_mtkapi_bt_hfclient_enable");
    RPC_CLIENT_DECL(1, INT32);
    RPC_CLIENT_ARG_INP(ARG_TYPE_INT32, i4_InvalidValue);
    RPC_BT_SERVICE_CLIENT_DO_OP("x_mtkapi_bt_hfclient_enable");
    RPC_CLIENT_RETURN(ARG_TYPE_INT32, -1);
}

EXPORT_SYMBOL INT32 a_mtkapi_bt_hfclient_disable(VOID)
{
    INT32 i4_InvalidValue = 0;
    BT_RW_LOG("a_mtkapi_bt_hfclient_disable");
    RPC_CLIENT_DECL(1, INT32);
    RPC_CLIENT_ARG_INP(ARG_TYPE_INT32, i4_InvalidValue);
    RPC_BT_SERVICE_CLIENT_DO_OP("x_mtkapi_bt_hfclient_disable");
    RPC_CLIENT_RETURN(ARG_TYPE_INT32, -1);
}

EXPORT_SYMBOL INT32 a_mtkapi_bt_hfclient_connect(CHAR *bt_addr)
{
    BT_RW_LOG("a_mtkapi_bt_hfclient_connect");
    RPC_CLIENT_DECL(1, INT32);
    RPC_CLIENT_ARG_INP(ARG_TYPE_REF_STR, bt_addr);
    RPC_BT_SERVICE_CLIENT_DO_OP("x_mtkapi_bt_hfclient_connect");
    RPC_CLIENT_RETURN(ARG_TYPE_INT32, -1);
}

EXPORT_SYMBOL INT32 a_mtkapi_bt_hfclient_disconnect(CHAR *bt_addr)
{
    BT_RW_LOG("a_mtkapi_bt_hfclient_disconnect");
    RPC_CLIENT_DECL(1, INT32);
    RPC_CLIENT_ARG_INP(ARG_TYPE_REF_STR, bt_addr);
    RPC_BT_SERVICE_CLIENT_DO_OP("x_mtkapi_bt_hfclient_disconnect");
    RPC_CLIENT_RETURN(ARG_TYPE_INT32, -1);
}

EXPORT_SYMBOL INT32 a_mtkapi_bt_hfclient_connect_audio(CHAR *bt_addr)
{
    BT_RW_LOG("a_mtkapi_bt_hfclient_connect_audio");
    RPC_CLIENT_DECL(1, INT32);
    RPC_CLIENT_ARG_INP(ARG_TYPE_REF_STR, bt_addr);
    RPC_BT_SERVICE_CLIENT_DO_OP("x_mtkapi_bt_hfclient_connect_audio");
    RPC_CLIENT_RETURN(ARG_TYPE_INT32, -1);
}

EXPORT_SYMBOL INT32 a_mtkapi_bt_hfclient_disconnect_audio(CHAR *bt_addr)
{
    BT_RW_LOG("a_mtkapi_bt_hfclient_disconnect_audio");
    RPC_CLIENT_DECL(1, INT32);
    RPC_CLIENT_ARG_INP(ARG_TYPE_REF_STR, bt_addr);
    RPC_BT_SERVICE_CLIENT_DO_OP("x_mtkapi_bt_hfclient_disconnect_audio");
    RPC_CLIENT_RETURN(ARG_TYPE_INT32, -1);
}

EXPORT_SYMBOL INT32 a_mtkapi_bt_hfclient_start_voice_recognition(VOID)
{
    INT32 i4_InvalidValue = 0;
    BT_RW_LOG("a_mtkapi_bt_hfclient_start_voice_recognition");
    RPC_CLIENT_DECL(1, INT32);
    RPC_CLIENT_ARG_INP(ARG_TYPE_INT32, i4_InvalidValue);
    RPC_BT_SERVICE_CLIENT_DO_OP("x_mtkapi_bt_hfclient_start_voice_recognition");
    RPC_CLIENT_RETURN(ARG_TYPE_INT32, -1);
}

EXPORT_SYMBOL INT32 a_mtkapi_bt_hfclient_stop_voice_recognition(VOID)
{
    INT32 i4_InvalidValue = 0;
    BT_RW_LOG("a_mtkapi_bt_hfclient_stop_voice_recognition");
    RPC_CLIENT_DECL(1, INT32);
    RPC_CLIENT_ARG_INP(ARG_TYPE_INT32, i4_InvalidValue);
    RPC_BT_SERVICE_CLIENT_DO_OP("x_mtkapi_bt_hfclient_stop_voice_recognition");
    RPC_CLIENT_RETURN(ARG_TYPE_INT32, -1);
}

EXPORT_SYMBOL INT32 a_mtkapi_bt_hfclient_volume_control(BT_HFCLIENT_VOLUME_TYPE_T type, INT32 volume)
{
    BT_RW_LOG("a_mtkapi_bt_hfclient_volume_control");
    RPC_CLIENT_DECL(2, INT32);
    RPC_CLIENT_ARG_INP(ARG_TYPE_INT32, type);
    RPC_CLIENT_ARG_INP(ARG_TYPE_INT32, volume);
    RPC_BT_SERVICE_CLIENT_DO_OP("x_mtkapi_bt_hfclient_volume_control");
    RPC_CLIENT_RETURN(ARG_TYPE_INT32, -1);
}

EXPORT_SYMBOL INT32 a_mtkapi_bt_hfclient_dial(const CHAR *number)
{
    BT_RW_LOG("a_mtkapi_bt_hfclient_dial");
    RPC_CLIENT_DECL(1, INT32);
    RPC_CLIENT_ARG_INP(ARG_TYPE_REF_STR, number);
    RPC_BT_SERVICE_CLIENT_DO_OP("x_mtkapi_bt_hfclient_dial");
    RPC_CLIENT_RETURN(ARG_TYPE_INT32, -1);
}

EXPORT_SYMBOL INT32 a_mtkapi_bt_hfclient_dial_memory(INT32 location)
{
    BT_RW_LOG("a_mtkapi_bt_hfclient_dial_memory");
    RPC_CLIENT_DECL(1, INT32);
    RPC_CLIENT_ARG_INP(ARG_TYPE_INT32, location);
    RPC_BT_SERVICE_CLIENT_DO_OP("x_mtkapi_bt_hfclient_dial_memory");
    RPC_CLIENT_RETURN(ARG_TYPE_INT32, -1);
}

EXPORT_SYMBOL INT32 a_mtkapi_bt_hfclient_handle_call_action(BT_HFCLIENT_CALL_ACTION_T action, INT32 idx)
{
    BT_RW_LOG("a_mtkapi_bt_hfclient_handle_call_action");
    RPC_CLIENT_DECL(2, INT32);
    RPC_CLIENT_ARG_INP(ARG_TYPE_INT32, action);
    RPC_CLIENT_ARG_INP(ARG_TYPE_INT32, idx);
    RPC_BT_SERVICE_CLIENT_DO_OP("x_mtkapi_bt_hfclient_handle_call_action");
    RPC_CLIENT_RETURN(ARG_TYPE_INT32, -1);
}

EXPORT_SYMBOL INT32 a_mtkapi_bt_hfclient_query_current_calls(VOID)
{
    INT32 i4_InvalidValue = 0;
    BT_RW_LOG("a_mtkapi_bt_hfclient_query_current_calls");
    RPC_CLIENT_DECL(1, INT32);
    RPC_CLIENT_ARG_INP(ARG_TYPE_INT32, i4_InvalidValue);
    RPC_BT_SERVICE_CLIENT_DO_OP("x_mtkapi_bt_hfclient_query_current_calls");
    RPC_CLIENT_RETURN(ARG_TYPE_INT32, -1);
}

EXPORT_SYMBOL INT32 a_mtkapi_bt_hfclient_query_current_operator_name(VOID)
{
    INT32 i4_InvalidValue = 0;
    BT_RW_LOG("a_mtkapi_bt_hfclient_query_current_operator_name");
    RPC_CLIENT_DECL(1, INT32);
    RPC_CLIENT_ARG_INP(ARG_TYPE_INT32, i4_InvalidValue);
    RPC_BT_SERVICE_CLIENT_DO_OP("x_mtkapi_bt_hfclient_query_current_operator_name");
    RPC_CLIENT_RETURN(ARG_TYPE_INT32, -1);
}

EXPORT_SYMBOL INT32 a_mtkapi_bt_hfclient_retrieve_subscriber_info(VOID)
{
    INT32 i4_InvalidValue = 0;
    BT_RW_LOG("a_mtkapi_bt_hfclient_retrieve_subscriber_info");
    RPC_CLIENT_DECL(1, INT32);
    RPC_CLIENT_ARG_INP(ARG_TYPE_INT32, i4_InvalidValue);
    RPC_BT_SERVICE_CLIENT_DO_OP("x_mtkapi_bt_hfclient_retrieve_subscriber_info");
    RPC_CLIENT_RETURN(ARG_TYPE_INT32, -1);
}

EXPORT_SYMBOL INT32 a_mtkapi_bt_hfclient_send_dtmf(CHAR code)
{
    BT_RW_LOG("a_mtkapi_bt_hfclient_send_dtmf");
    RPC_CLIENT_DECL(1, INT32);
    RPC_CLIENT_ARG_INP(ARG_TYPE_CHAR, code);
    RPC_BT_SERVICE_CLIENT_DO_OP("x_mtkapi_bt_hfclient_send_dtmf");
    RPC_CLIENT_RETURN(ARG_TYPE_INT32, -1);
}

EXPORT_SYMBOL INT32 a_mtkapi_bt_hfclient_request_last_voice_tag_number(VOID)
{
    INT32 i4_InvalidValue = 0;
    BT_RW_LOG("a_mtkapi_bt_hfclient_request_last_voice_tag_number");
    RPC_CLIENT_DECL(1, INT32);
    RPC_CLIENT_ARG_INP(ARG_TYPE_INT32, i4_InvalidValue);
    RPC_BT_SERVICE_CLIENT_DO_OP("x_mtkapi_bt_hfclient_request_last_voice_tag_number");
    RPC_CLIENT_RETURN(ARG_TYPE_INT32, -1);
}

EXPORT_SYMBOL INT32 a_mtkapi_bt_hfclient_send_at_cmd(INT32 cmd, INT32 val1, INT32 val2, const CHAR *arg)
{
    BT_RW_LOG("a_mtkapi_bt_hfclient_send_at_cmd");
    RPC_CLIENT_DECL(4, INT32);
    RPC_CLIENT_ARG_INP(ARG_TYPE_INT32, cmd);
    RPC_CLIENT_ARG_INP(ARG_TYPE_INT32, val1);
    RPC_CLIENT_ARG_INP(ARG_TYPE_INT32, val2);
    RPC_CLIENT_ARG_INP(ARG_TYPE_REF_STR, arg);
    RPC_BT_SERVICE_CLIENT_DO_OP("x_mtkapi_bt_hfclient_send_at_cmd");
    RPC_CLIENT_RETURN(ARG_TYPE_INT32, -1);
}

EXPORT_SYMBOL INT32 a_mtkapi_bt_hfclient_read_pb_entries(VOID)
{
    BT_RW_LOG("a_mtkapi_bt_hfclient_read_pb_entries");
    INT32 i4_InvalidValue = 0;
    RPC_CLIENT_DECL(1, INT32);
    RPC_CLIENT_ARG_INP(ARG_TYPE_INT32, i4_InvalidValue);
    RPC_BT_SERVICE_CLIENT_DO_OP("x_mtkapi_bt_hfclient_read_pb_entries");
    RPC_CLIENT_RETURN(ARG_TYPE_INT32, -1);
}

/*******************************************************************************
**
** Function         a_mtkapi_bt_hfclient_register_callback
**
** Description      The function is used by user to register a HFCLIENT event callback.
**                      User can receive HFCLIENT event by this handler.
** Returns          BT_SUCCESS -- register succesfully
**                  Others     -- register failed
*******************************************************************************/
EXPORT_SYMBOL INT32 a_mtkapi_bt_hfclient_register_callback(mtkrpcapi_BtAppHfclientCbk hfclient_handle, VOID* pv_tag)
{
    BT_RW_LOG("a_mtkapi_bt_hfclient_register_callback");
    RPC_CLIENT_DECL(2, INT32);
    RPC_CLIENT_ARG_INP(ARG_TYPE_REF_FUNC, hfclient_handle);
    RPC_CLIENT_ARG_INP(ARG_TYPE_REF_VOID, pv_tag);
    RPC_BT_SERVICE_CLIENT_DO_OP("x_mtkapi_bt_hfclient_register_callback");
    RPC_CLIENT_RETURN(ARG_TYPE_INT32, -1);
}

EXPORT_SYMBOL INT32 c_rpc_reg_mtk_bt_service_hfclient_cb_hndlrs(VOID)
{
    int i4_ret = 0;
    RPC_REG_CB_HNDLR(bt_app_hfclient_event_cbk);
    return RPCR_OK;
}
