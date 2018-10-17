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
#include "mtk_bt_service_hfclient.h"
#include "mtk_bt_service_hfclient_ipcrpc_struct.h"
#include "mtk_bt_service_hfclient_handle.h"
#include "u_rpc.h"
#include "ri_common.h"

#define BT_RH_LOG(_stmt...) \
        do{ \
            if(0){    \
                printf("Func:%s Line:%d--->: ", __FUNCTION__, __LINE__);   \
                printf(_stmt); \
                printf("\n"); \
            }        \
        }   \
        while(0)


static VOID bt_app_hfclient_event_cbk_wrapper(BT_HFCLIENT_EVENT_PARAM *param,
    void* pv_tag)
{
    RPC_DECL_VOID(2);
    RPC_CB_NFY_TAG_T  *pt_nfy_tag = (RPC_CB_NFY_TAG_T*)pv_tag;
    RPC_CHECK(bt_rpc_add_ref_desc(RPC_DEFAULT_ID, param,
        RPC_DESC_BT_HFCLIENT_EVENT_PARAM, NULL));
    RPC_ARG_INP(ARG_TYPE_REF_DESC, param);
    RPC_ARG_INP(ARG_TYPE_REF_VOID, pt_nfy_tag->pv_tag);

    RPC_DO_CB(pt_nfy_tag->t_id, "bt_app_hfclient_event_cbk", pt_nfy_tag->pv_cb_addr);

    RPC_RETURN_VOID;
}

static INT32 _hndlr_x_mtkapi_bt_hfclient_enable(
                         RPC_ID_T     t_rpc_id,
                         const CHAR*  ps_cb_type,
                         UINT32       ui4_num_args,
                         ARG_DESC_T*  pt_args,
                         ARG_DESC_T*  pt_return)
{
    BT_RH_LOG("\n");
    pt_return->e_type   = ARG_TYPE_INT32;
    pt_return->u.i4_arg = x_mtkapi_bt_hfclient_enable();

    return RPCR_OK;
}

static INT32 _hndlr_x_mtkapi_bt_hfclient_disable(
                         RPC_ID_T     t_rpc_id,
                         const CHAR*  ps_cb_type,
                         UINT32       ui4_num_args,
                         ARG_DESC_T*  pt_args,
                         ARG_DESC_T*  pt_return)
{
    BT_RH_LOG("\n");
    pt_return->e_type   = ARG_TYPE_INT32;
    pt_return->u.i4_arg = x_mtkapi_bt_hfclient_disable();

    return RPCR_OK;
}


static INT32 _hndlr_x_mtkapi_bt_hfclient_connect(
                         RPC_ID_T     t_rpc_id,
                         const CHAR*  ps_cb_type,
                         UINT32       ui4_num_args,
                         ARG_DESC_T*  pt_args,
                         ARG_DESC_T*  pt_return)
{
    if (ui4_num_args != 1)
    {
        BT_RH_LOG("Invalid ARGS: %d\n", ui4_num_args);
        return RPCR_INV_ARGS;
    }

    BT_RH_LOG("arg_1 = %s\n", pt_args[0].u.pc_arg);
    pt_return->e_type   = ARG_TYPE_INT32;
    pt_return->u.i4_arg = x_mtkapi_bt_hfclient_connect(pt_args[0].u.pc_arg);

    return RPCR_OK;
}

static INT32 _hndlr_x_mtkapi_bt_hfclient_disconnect(
                         RPC_ID_T     t_rpc_id,
                         const CHAR*  ps_cb_type,
                         UINT32       ui4_num_args,
                         ARG_DESC_T*  pt_args,
                         ARG_DESC_T*  pt_return)
{
    if (ui4_num_args != 1)
    {
        BT_RH_LOG("Invalid ARGS: %d\n", ui4_num_args);
        return RPCR_INV_ARGS;
    }

    BT_RH_LOG("arg_1 = %s\n", pt_args[0].u.pc_arg);
    pt_return->e_type   = ARG_TYPE_INT32;
    pt_return->u.i4_arg = x_mtkapi_bt_hfclient_disconnect(pt_args[0].u.pc_arg);

    return RPCR_OK;
}

static INT32 _hndlr_x_mtkapi_bt_hfclient_connect_audio(
                         RPC_ID_T     t_rpc_id,
                         const CHAR*  ps_cb_type,
                         UINT32       ui4_num_args,
                         ARG_DESC_T*  pt_args,
                         ARG_DESC_T*  pt_return)
{
    if (ui4_num_args != 1)
    {
        BT_RH_LOG("Invalid ARGS: %d\n", ui4_num_args);
        return RPCR_INV_ARGS;
    }

    BT_RH_LOG("arg_1 = %s\n", pt_args[0].u.pc_arg);
    pt_return->e_type   = ARG_TYPE_INT32;
    pt_return->u.i4_arg = x_mtkapi_bt_hfclient_connect_audio(pt_args[0].u.pc_arg);

    return RPCR_OK;
}

static INT32 _hndlr_x_mtkapi_bt_hfclient_disconnect_audio(
                         RPC_ID_T     t_rpc_id,
                         const CHAR*  ps_cb_type,
                         UINT32       ui4_num_args,
                         ARG_DESC_T*  pt_args,
                         ARG_DESC_T*  pt_return)
{
    if (ui4_num_args != 1)
    {
        BT_RH_LOG("Invalid ARGS: %d\n", ui4_num_args);
        return RPCR_INV_ARGS;
    }

    BT_RH_LOG("arg_1 = %s\n", pt_args[0].u.pc_arg);
    pt_return->e_type   = ARG_TYPE_INT32;
    pt_return->u.i4_arg = x_mtkapi_bt_hfclient_disconnect_audio(pt_args[0].u.pc_arg);

    return RPCR_OK;
}

static INT32 _hndlr_x_mtkapi_bt_hfclient_start_voice_recognition(
                         RPC_ID_T     t_rpc_id,
                         const CHAR*  ps_cb_type,
                         UINT32       ui4_num_args,
                         ARG_DESC_T*  pt_args,
                         ARG_DESC_T*  pt_return)
{
    BT_RH_LOG("\n");
    pt_return->e_type   = ARG_TYPE_INT32;
    pt_return->u.i4_arg = x_mtkapi_bt_hfclient_start_voice_recognition();

    return RPCR_OK;
}

static INT32 _hndlr_x_mtkapi_bt_hfclient_stop_voice_recognition(
                         RPC_ID_T     t_rpc_id,
                         const CHAR*  ps_cb_type,
                         UINT32       ui4_num_args,
                         ARG_DESC_T*  pt_args,
                         ARG_DESC_T*  pt_return)
{
    BT_RH_LOG("\n");
    pt_return->e_type   = ARG_TYPE_INT32;
    pt_return->u.i4_arg = x_mtkapi_bt_hfclient_stop_voice_recognition();

    return RPCR_OK;
}

static INT32 _hndlr_x_mtkapi_bt_hfclient_volume_control(
                         RPC_ID_T     t_rpc_id,
                         const CHAR*  ps_cb_type,
                         UINT32       ui4_num_args,
                         ARG_DESC_T*  pt_args,
                         ARG_DESC_T*  pt_return)
{
    if (ui4_num_args != 2)
    {
        BT_RH_LOG("Invalid ARGS: %d\n", ui4_num_args);
        return RPCR_INV_ARGS;
    }

    BT_RH_LOG("arg_1 = %ld, arg_2 = %ld\n", pt_args[0].u.i4_arg, pt_args[1].u.i4_arg);
    pt_return->e_type   = ARG_TYPE_INT32;
    pt_return->u.i4_arg = x_mtkapi_bt_hfclient_volume_control(pt_args[0].u.i4_arg, pt_args[1].u.i4_arg);

    return RPCR_OK;
}

static INT32 _hndlr_x_mtkapi_bt_hfclient_dial(
                         RPC_ID_T     t_rpc_id,
                         const CHAR*  ps_cb_type,
                         UINT32       ui4_num_args,
                         ARG_DESC_T*  pt_args,
                         ARG_DESC_T*  pt_return)
{
    if (ui4_num_args != 1)
    {
        BT_RH_LOG("Invalid ARGS: %d\n", ui4_num_args);
        return RPCR_INV_ARGS;
    }

    BT_RH_LOG("arg_1 = %s\n", pt_args[0].u.pc_arg);
    pt_return->e_type   = ARG_TYPE_INT32;
    pt_return->u.i4_arg = x_mtkapi_bt_hfclient_dial(pt_args[0].u.pc_arg);

    return RPCR_OK;
}

static INT32 _hndlr_x_mtkapi_bt_hfclient_dial_memory(
                         RPC_ID_T     t_rpc_id,
                         const CHAR*  ps_cb_type,
                         UINT32       ui4_num_args,
                         ARG_DESC_T*  pt_args,
                         ARG_DESC_T*  pt_return)
{
    if (ui4_num_args != 1)
    {
        BT_RH_LOG("Invalid ARGS: %d\n", ui4_num_args);
        return RPCR_INV_ARGS;
    }

    BT_RH_LOG("arg_1 = %ld\n", pt_args[0].u.i4_arg);
    pt_return->e_type   = ARG_TYPE_INT32;
    pt_return->u.i4_arg = x_mtkapi_bt_hfclient_dial_memory(pt_args[0].u.i4_arg);

    return RPCR_OK;
}

static INT32 _hndlr_x_mtkapi_bt_hfclient_handle_call_action(
                         RPC_ID_T     t_rpc_id,
                         const CHAR*  ps_cb_type,
                         UINT32       ui4_num_args,
                         ARG_DESC_T*  pt_args,
                         ARG_DESC_T*  pt_return)
{
    if (ui4_num_args != 2)
    {
        BT_RH_LOG("Invalid ARGS: %d\n", ui4_num_args);
        return RPCR_INV_ARGS;
    }

    BT_RH_LOG("arg_1 = %ld, arg_2 = %ld\n", pt_args[0].u.i4_arg, pt_args[1].u.i4_arg);
    pt_return->e_type   = ARG_TYPE_INT32;
    pt_return->u.i4_arg = x_mtkapi_bt_hfclient_handle_call_action(pt_args[0].u.i4_arg, pt_args[1].u.i4_arg);

    return RPCR_OK;
}

static INT32 _hndlr_x_mtkapi_bt_hfclient_query_current_calls(
                         RPC_ID_T     t_rpc_id,
                         const CHAR*  ps_cb_type,
                         UINT32       ui4_num_args,
                         ARG_DESC_T*  pt_args,
                         ARG_DESC_T*  pt_return)
{
    BT_RH_LOG("\n");
    pt_return->e_type   = ARG_TYPE_INT32;
    pt_return->u.i4_arg = x_mtkapi_bt_hfclient_query_current_calls();

    return RPCR_OK;
}

static INT32 _hndlr_x_mtkapi_bt_hfclient_query_current_operator_name(
                         RPC_ID_T     t_rpc_id,
                         const CHAR*  ps_cb_type,
                         UINT32       ui4_num_args,
                         ARG_DESC_T*  pt_args,
                         ARG_DESC_T*  pt_return)
{
    BT_RH_LOG("\n");
    pt_return->e_type   = ARG_TYPE_INT32;
    pt_return->u.i4_arg = x_mtkapi_bt_hfclient_query_current_operator_name();

    return RPCR_OK;
}

static INT32 _hndlr_x_mtkapi_bt_hfclient_retrieve_subscriber_info(
                         RPC_ID_T     t_rpc_id,
                         const CHAR*  ps_cb_type,
                         UINT32       ui4_num_args,
                         ARG_DESC_T*  pt_args,
                         ARG_DESC_T*  pt_return)
{
    BT_RH_LOG("\n");
    pt_return->e_type   = ARG_TYPE_INT32;
    pt_return->u.i4_arg = x_mtkapi_bt_hfclient_retrieve_subscriber_info();

    return RPCR_OK;
}

static INT32 _hndlr_x_mtkapi_bt_hfclient_send_dtmf(
                         RPC_ID_T     t_rpc_id,
                         const CHAR*  ps_cb_type,
                         UINT32       ui4_num_args,
                         ARG_DESC_T*  pt_args,
                         ARG_DESC_T*  pt_return)
{
    if (ui4_num_args != 1)
    {
        BT_RH_LOG("Invalid ARGS: %d\n", ui4_num_args);
        return RPCR_INV_ARGS;
    }

    BT_RH_LOG("arg_1 = %d\n", pt_args[0].u.c_arg);
    pt_return->e_type   = ARG_TYPE_INT32;
    pt_return->u.i4_arg = x_mtkapi_bt_hfclient_send_dtmf(pt_args[0].u.c_arg);

    return RPCR_OK;
}

static INT32 _hndlr_x_mtkapi_bt_hfclient_request_last_voice_tag_number(
                         RPC_ID_T     t_rpc_id,
                         const CHAR*  ps_cb_type,
                         UINT32       ui4_num_args,
                         ARG_DESC_T*  pt_args,
                         ARG_DESC_T*  pt_return)
{
    BT_RH_LOG("\n");
    pt_return->e_type   = ARG_TYPE_INT32;
    pt_return->u.i4_arg = x_mtkapi_bt_hfclient_request_last_voice_tag_number();

    return RPCR_OK;
}

static INT32 _hndlr_x_mtkapi_bt_hfclient_send_at_cmd(
                         RPC_ID_T     t_rpc_id,
                         const CHAR*  ps_cb_type,
                         UINT32       ui4_num_args,
                         ARG_DESC_T*  pt_args,
                         ARG_DESC_T*  pt_return)
{
    if (ui4_num_args != 4)
    {
        BT_RH_LOG("Invalid ARGS: %d\n", ui4_num_args);
        return RPCR_INV_ARGS;
    }

    BT_RH_LOG("arg_1 = %ld, arg_2 = %ld, arg_3 = %ld, arg_4 = %s\n",
        pt_args[0].u.i4_arg, pt_args[1].u.i4_arg, pt_args[2].u.i4_arg, pt_args[3].u.pc_arg);
    pt_return->e_type   = ARG_TYPE_INT32;
    pt_return->u.i4_arg = x_mtkapi_bt_hfclient_send_at_cmd(pt_args[0].u.i4_arg, pt_args[1].u.i4_arg,
                                                           pt_args[2].u.i4_arg, pt_args[3].u.pc_arg);

    return RPCR_OK;
}


static INT32 _hndlr_x_mtkapi_bt_hfclient_read_pb_entries(
                         RPC_ID_T     t_rpc_id,
                         const CHAR*  ps_cb_type,
                         UINT32       ui4_num_args,
                         ARG_DESC_T*  pt_args,
                         ARG_DESC_T*  pt_return)
{
    BT_RH_LOG("\n");
    pt_return->e_type   = ARG_TYPE_INT32;
    pt_return->u.i4_arg = x_mtkapi_bt_hfclient_read_pb_entries();

    return RPCR_OK;
}


static INT32 _hndlr_x_mtkapi_bt_hfclient_register_callback(
                         RPC_ID_T     t_rpc_id,
                         const CHAR*  ps_cb_type,
                         UINT32       ui4_num_args,
                         ARG_DESC_T*  pt_args,
                         ARG_DESC_T*  pt_return)
{
    BT_RH_LOG("\n");

    mtkrpcapi_BtAppHfclientCbk p_bt_hfclient_app_cb_func = NULL;
    RPC_CB_NFY_TAG_T * pt_nfy_tag = NULL;
    VOID * apv_cb_addr[1] = {0};

    if(ui4_num_args != 2)
    {
        return RPCR_INV_ARGS;
    }

    p_bt_hfclient_app_cb_func = (mtkrpcapi_BtAppHfclientCbk)pt_args[0].u.pv_func;
    if (NULL != p_bt_hfclient_app_cb_func)
    {
        apv_cb_addr[0] = (VOID*)p_bt_hfclient_app_cb_func;
        pt_nfy_tag  = ri_create_cb_tag(t_rpc_id, apv_cb_addr, 1,
            pt_args[1].u.pv_arg);

        BT_RH_LOG("pv_func=%p, pv_arg=%p, pt_nfy_tag=%p",
            p_bt_hfclient_app_cb_func, pt_args[1].u.pv_arg, pt_nfy_tag);

        pt_return->e_type   = ARG_TYPE_INT32;
        pt_return->u.i4_arg =
            x_mtkapi_bt_hfclient_register_callback(bt_app_hfclient_event_cbk_wrapper,
            pt_nfy_tag);
        if (pt_return->u.i4_arg && pt_nfy_tag != NULL)
        {
            ri_free_cb_tag(pt_nfy_tag);
        }
    }
    else
    {
        pt_return->u.i4_arg = x_mtkapi_bt_hfclient_register_callback(NULL, NULL);
    }
    return RPCR_OK;

}


INT32 c_rpc_reg_mtk_bt_service_hfclient_op_hndlrs(VOID)
{
    RPC_REG_OP_HNDLR(x_mtkapi_bt_hfclient_enable);
    RPC_REG_OP_HNDLR(x_mtkapi_bt_hfclient_disable);
    RPC_REG_OP_HNDLR(x_mtkapi_bt_hfclient_connect);
    RPC_REG_OP_HNDLR(x_mtkapi_bt_hfclient_disconnect);
    RPC_REG_OP_HNDLR(x_mtkapi_bt_hfclient_connect_audio);
    RPC_REG_OP_HNDLR(x_mtkapi_bt_hfclient_disconnect_audio);
    RPC_REG_OP_HNDLR(x_mtkapi_bt_hfclient_start_voice_recognition);
    RPC_REG_OP_HNDLR(x_mtkapi_bt_hfclient_stop_voice_recognition);
    RPC_REG_OP_HNDLR(x_mtkapi_bt_hfclient_volume_control);
    RPC_REG_OP_HNDLR(x_mtkapi_bt_hfclient_dial);
    RPC_REG_OP_HNDLR(x_mtkapi_bt_hfclient_dial_memory);
    RPC_REG_OP_HNDLR(x_mtkapi_bt_hfclient_handle_call_action);
    RPC_REG_OP_HNDLR(x_mtkapi_bt_hfclient_query_current_calls);
    RPC_REG_OP_HNDLR(x_mtkapi_bt_hfclient_query_current_operator_name);
    RPC_REG_OP_HNDLR(x_mtkapi_bt_hfclient_retrieve_subscriber_info);
    RPC_REG_OP_HNDLR(x_mtkapi_bt_hfclient_send_dtmf);
    RPC_REG_OP_HNDLR(x_mtkapi_bt_hfclient_request_last_voice_tag_number);
    RPC_REG_OP_HNDLR(x_mtkapi_bt_hfclient_send_at_cmd);
    RPC_REG_OP_HNDLR(x_mtkapi_bt_hfclient_read_pb_entries);
    RPC_REG_OP_HNDLR(x_mtkapi_bt_hfclient_register_callback);
    return RPCR_OK;
}
