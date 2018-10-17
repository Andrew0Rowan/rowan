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

#include "mtk_bt_service_gap.h"
#include "mtk_bt_service_gap_ipcrpc_struct.h"
#include "mtk_bt_service_gap_handle.h"
#include "ri_common.h"

#define BT_RH_LOG(_stmt...) \
        do{ \
            if(0){    \
                printf("[Handle]Func:%s Line:%d--->: ", __FUNCTION__, __LINE__);   \
                printf(_stmt); \
                printf("\n"); \
            }        \
        }   \
        while(0)

void *g_gap_ptag = NULL;

static VOID bt_app_gap_inquiry_response_cbk_wrapper(tBTMW_GAP_DEVICE_INFO* pt_result, void* pv_tag)
{

    RPC_DECL_VOID(2);
    RPC_CB_NFY_TAG_T *pt_nfy_tag = (RPC_CB_NFY_TAG_T*)pv_tag;
    RPC_CHECK(bt_rpc_add_ref_desc(RPC_DEFAULT_ID, pt_result, RPC_DESC_tBTMW_GAP_DEVICE_INFO, NULL));
    RPC_ARG_INP(ARG_TYPE_REF_DESC, pt_result);
    RPC_ARG_INP(ARG_TYPE_REF_VOID, pt_nfy_tag->pv_tag);

    RPC_DO_CB(pt_nfy_tag->t_id, "bt_app_gap_inquiry_response_cbk", pt_nfy_tag->apv_cb_addr_ex[APP_GAP_INQUIRY_RESPONSE_CB_IDX]);
    RPC_RETURN_VOID;
}

static VOID bt_app_gap_event_cbk_wrapper(tBTMW_GAP_STATE *bt_event, void* pv_tag)
{
    RPC_DECL_VOID(2);
    RPC_CB_NFY_TAG_T  *pt_nfy_tag = (RPC_CB_NFY_TAG_T*)pv_tag;
    RPC_CHECK(bt_rpc_add_ref_desc(RPC_DEFAULT_ID, bt_event, RPC_DESC_tBTMW_GAP_STATE, NULL));
    RPC_ARG_INP(ARG_TYPE_REF_DESC, bt_event);
    RPC_ARG_INP(ARG_TYPE_REF_VOID, pt_nfy_tag->pv_tag);

    RPC_DO_CB(pt_nfy_tag->t_id, "bt_app_gap_event_cbk", pt_nfy_tag->apv_cb_addr_ex[APP_EVENT_CB_IDX]);

    RPC_RETURN_VOID;
}

static VOID bt_app_gap_get_pairing_key_cbk_wrapper(pairing_key_value_t *bt_pairing_key, UINT8 *fg_accept, void* pv_tag)
{

    RPC_DECL_VOID(3);
    RPC_CB_NFY_TAG_T *pt_nfy_tag = (RPC_CB_NFY_TAG_T*)pv_tag;
    RPC_CHECK(bt_rpc_add_ref_desc(RPC_DEFAULT_ID, bt_pairing_key, RPC_DESC_pairing_key_value_t, NULL));
    RPC_ARG_INP(ARG_TYPE_REF_DESC, bt_pairing_key);
    RPC_ARG_IO(ARG_TYPE_REF_UINT8, fg_accept);
    RPC_ARG_INP(ARG_TYPE_REF_VOID, pt_nfy_tag->pv_tag);

    BT_RH_LOG("[_hndlr_]bt_app_gap_get_pairing_key_cbk_wrapper, key_type = %d, pt_nfy_tag->apv_cb_addr_ex[%d] = %p\n",
        bt_pairing_key->key_type, APP_GAP_GET_PAIRING_KEY_CB_IDX, pt_nfy_tag->apv_cb_addr_ex[APP_GAP_GET_PAIRING_KEY_CB_IDX]);

    RPC_DO_CB(pt_nfy_tag->t_id, "bt_app_gap_get_pairing_key_cbk", pt_nfy_tag->apv_cb_addr_ex[APP_GAP_GET_PAIRING_KEY_CB_IDX]);


    RPC_RETURN_VOID;
}

static INT32 _hndlr_x_mtkapi_bt_base_init(
                         RPC_ID_T     t_rpc_id,
                         const CHAR*  ps_cb_type,
                         UINT32       ui4_num_args,
                         ARG_DESC_T*  pt_args,
                         ARG_DESC_T*  pt_return)
{
    BT_RH_LOG("[_hndlr_]bt_gap_base_init , arg_1 = %ld\n", pt_args[0].u.i4_arg);

    MTKRPCAPI_BT_APP_CB_FUNC *p_bt_app_cb_func = NULL;
    MTKRPCAPI_BT_APP_CB_FUNC bt_app_cb_func;
    RPC_CB_NFY_TAG_T * pt_nfy_tag = NULL;
    VOID * apv_cb_addr[APP_CB_IDX_NUM] = {0};

    memset(&bt_app_cb_func,0,sizeof(MTKRPCAPI_BT_APP_CB_FUNC));

    BT_RH_LOG("bt_base_init, pt_args[0].u.pv_desc = %p\n", pt_args[0].u.pv_desc);

    if(ui4_num_args != 2)
    {
        return RPCR_INV_ARGS;
    }
    p_bt_app_cb_func = (MTKRPCAPI_BT_APP_CB_FUNC*)pt_args[0].u.pv_desc;

    if(p_bt_app_cb_func->bt_event_cb != NULL)
    {
        apv_cb_addr[APP_EVENT_CB_IDX] = p_bt_app_cb_func->bt_event_cb;
        bt_app_cb_func.bt_event_cb = bt_app_gap_event_cbk_wrapper;
    }

    if(p_bt_app_cb_func->bt_get_pairing_key_cb != NULL)
    {
        apv_cb_addr[APP_GAP_GET_PAIRING_KEY_CB_IDX] = p_bt_app_cb_func->bt_get_pairing_key_cb;
        bt_app_cb_func.bt_get_pairing_key_cb = bt_app_gap_get_pairing_key_cbk_wrapper;
    }

    if(p_bt_app_cb_func->bt_dev_info_cb != NULL)
    {
        apv_cb_addr[APP_GAP_INQUIRY_RESPONSE_CB_IDX] = p_bt_app_cb_func->bt_dev_info_cb;
        bt_app_cb_func.bt_dev_info_cb = bt_app_gap_inquiry_response_cbk_wrapper;
    }

    pt_nfy_tag  = ri_create_cb_tag(t_rpc_id, apv_cb_addr, APP_CB_IDX_NUM, pt_args[1].u.pv_arg);

    pt_return->e_type   = ARG_TYPE_INT32;
    pt_return->u.i4_arg = x_mtkapi_bt_base_init(&bt_app_cb_func, pt_nfy_tag);
    if (pt_return->u.i4_arg && pt_nfy_tag != NULL)
    {
        ri_free_cb_tag(pt_nfy_tag);
    }
    return RPCR_OK;

}

static INT32 _hndlr_x_mtkapi_bt_gap_set_dbg_level(
                         RPC_ID_T     t_rpc_id,
                         const CHAR*  ps_cb_type,
                         UINT32       ui4_num_args,
                         ARG_DESC_T*  pt_args,
                         ARG_DESC_T*  pt_return)
{
    BT_RH_LOG("[_hndlr_]bt_gap_set_dbg_layer , arg_1 = %ld\n", pt_args[0].u.i4_arg);
    BT_RH_LOG("[_hndlr_]bt_gap_set_dbg_level , arg_2 = %ld\n", pt_args[1].u.i4_arg);

    if (ui4_num_args != 2)
    {
        BT_RH_LOG("Invalid ARGS: %d\n", ui4_num_args);
        return RPCR_INV_ARGS;
    }

    pt_return->e_type   = ARG_TYPE_INT32;
    pt_return->u.i4_arg = x_mtkapi_bt_gap_set_dbg_level(pt_args[0].u.i4_arg, pt_args[1].u.i4_arg);

    return RPCR_OK;
}

static INT32 _hndlr_x_mtkapi_bt_gap_on_off(
                         RPC_ID_T     t_rpc_id,
                         const CHAR*  ps_cb_type,
                         UINT32       ui4_num_args,
                         ARG_DESC_T*  pt_args,
                         ARG_DESC_T*  pt_return)
{
    BT_RH_LOG("[_hndlr_]bt_gap_on_off , arg_1 = %d\n", pt_args[0].u.b_arg);

    if (ui4_num_args != 1)
    {
        BT_RH_LOG("Invalid ARGS: %d\n", ui4_num_args);
        return RPCR_INV_ARGS;
    }

    pt_return->e_type   = ARG_TYPE_INT32;
    pt_return->u.i4_arg = x_mtkapi_bt_gap_on_off(pt_args[0].u.b_arg);

    return RPCR_OK;
}

static INT32 _hndlr_x_mtkapi_bt_bluetooth_factory_reset(
                         RPC_ID_T     t_rpc_id,
                         const CHAR*  ps_cb_type,
                         UINT32       ui4_num_args,
                         ARG_DESC_T*  pt_args,
                         ARG_DESC_T*  pt_return)
{
    BT_RH_LOG("[_hndlr_]bt_factory_reset\n");

    pt_return->e_type   = ARG_TYPE_INT32;
    pt_return->u.i4_arg = x_mtkapi_bt_bluetooth_factory_reset();

    return RPCR_OK;
}

static INT32 _hndlr_x_mtkapi_bt_mw_log_setStackLevel(
                         RPC_ID_T     t_rpc_id,
                         const CHAR*  ps_cb_type,
                         UINT32       ui4_num_args,
                         ARG_DESC_T*  pt_args,
                         ARG_DESC_T*  pt_return)
{
    BT_RH_LOG("[_hndlr_]bt_gap_btstart , arg_1 = %s, arg_2 = %lu\n", pt_args[0].u.ps_str, pt_args[1].u.ui4_arg);

    if (ui4_num_args != 2)
    {
        BT_RH_LOG("Invalid ARGS: %d\n", ui4_num_args);
        return RPCR_INV_ARGS;
    }

    pt_return->e_type   = ARG_TYPE_INT32;
    pt_return->u.i4_arg = x_mtkapi_bt_mw_log_setStackLevel(pt_args[0].u.ps_str,
                                                  pt_args[1].u.ui4_arg);

    return RPCR_OK;
}

static INT32 _hndlr_x_mtkapi_bt_gap_set_name(
                         RPC_ID_T     t_rpc_id,
                         const CHAR*  ps_cb_type,
                         UINT32       ui4_num_args,
                         ARG_DESC_T*  pt_args,
                         ARG_DESC_T*  pt_return)
{
    BT_RH_LOG("[_hndlr_]bt_gap_set_name , arg_1 = %s\n", pt_args[0].u.ps_str);

    if (ui4_num_args != 1)
    {
        BT_RH_LOG("Invalid ARGS: %d\n", ui4_num_args);
        return RPCR_INV_ARGS;
    }

    pt_return->e_type   = ARG_TYPE_INT32;
    pt_return->u.i4_arg = x_mtkapi_bt_gap_set_name(pt_args[0].u.ps_str);

    return RPCR_OK;
}

static INT32 _hndlr_x_mtkapi_bt_gap_set_connectable_and_discoverable(
                         RPC_ID_T     t_rpc_id,
                         const CHAR*  ps_cb_type,
                         UINT32       ui4_num_args,
                         ARG_DESC_T*  pt_args,
                         ARG_DESC_T*  pt_return)
{
    BT_RH_LOG("[_hndlr_]bt_gap_set_connectable_and_discoverable , arg_1 = %d, arg_2 = %d\n", pt_args[0].u.b_arg, pt_args[1].u.b_arg);

    if (ui4_num_args != 2)
    {
        BT_RH_LOG("Invalid ARGS: %d\n", ui4_num_args);
        return RPCR_INV_ARGS;
    }

    pt_return->e_type   = ARG_TYPE_INT32;
    pt_return->u.i4_arg = x_mtkapi_bt_gap_set_connectable_and_discoverable(pt_args[0].u.b_arg, pt_args[1].u.b_arg);

    return RPCR_OK;
}

static INT32 _hndlr_x_mtkapi_bt_gap_get_dev_info(
                         RPC_ID_T     t_rpc_id,
                         const CHAR*  ps_cb_type,
                         UINT32       ui4_num_args,
                         ARG_DESC_T*  pt_args,
                         ARG_DESC_T*  pt_return)
{
    BT_RH_LOG("[_hndlr_]bt_gap_get_dev_info\n");

    if (ui4_num_args != 2)
    {
        BT_RH_LOG("Invalid ARGS: %d\n", ui4_num_args);
        return RPCR_INV_ARGS;
    }

    pt_return->e_type   = ARG_TYPE_INT32;
    pt_return->u.i4_arg = x_mtkapi_bt_gap_get_dev_info(pt_args[0].u.pv_desc, pt_args[1].u.ps_str);

    return RPCR_OK;
}

static INT32 _hndlr_x_mtkapi_bt_gap_get_local_dev_info(
                         RPC_ID_T     t_rpc_id,
                         const CHAR*  ps_cb_type,
                         UINT32       ui4_num_args,
                         ARG_DESC_T*  pt_args,
                         ARG_DESC_T*  pt_return)
{
    BT_RH_LOG("[_hndlr_]bt_gap_get_local_dev_info , arg_1 = %p\n", pt_args[0].u.pv_desc);

    if (ui4_num_args != 1)
    {
        BT_RH_LOG("Invalid ARGS: %d\n", ui4_num_args);
        return RPCR_INV_ARGS;
    }

    pt_return->e_type   = ARG_TYPE_INT32;
    pt_return->u.i4_arg = x_mtkapi_bt_gap_get_local_dev_info(pt_args[0].u.pv_desc);

    return RPCR_OK;
}

static INT32 _hndlr_x_mtkapi_bt_gap_start_inquiry_scan(
                         RPC_ID_T     t_rpc_id,
                         const CHAR*  ps_cb_type,
                         UINT32       ui4_num_args,
                         ARG_DESC_T*  pt_args,
                         ARG_DESC_T*  pt_return)
{
    BT_RH_LOG("[_hndlr_]bt_gap_start_inquiry_scan\n");

    if (ui4_num_args != 1)
    {
        BT_RH_LOG("Invalid ARGS: %d\n", ui4_num_args);
        return RPCR_INV_ARGS;
    }

    pt_return->e_type   = ARG_TYPE_INT32;
    pt_return->u.i4_arg = x_mtkapi_bt_gap_start_inquiry_scan(pt_args[0].u.ui4_arg);

    return RPCR_OK;
}

static INT32 _hndlr_x_mtkapi_bt_gap_stop_inquiry_scan(
                         RPC_ID_T     t_rpc_id,
                         const CHAR*  ps_cb_type,
                         UINT32       ui4_num_args,
                         ARG_DESC_T*  pt_args,
                         ARG_DESC_T*  pt_return)
{
    BT_RH_LOG("[_hndlr_]bt_gap_stop_inquiry_scan\n");

    pt_return->e_type   = ARG_TYPE_INT32;
    pt_return->u.i4_arg = x_mtkapi_bt_gap_stop_inquiry_scan();

    if (pt_return->u.i4_arg == 0)
    {
        return RPCR_OK;
    }
    else
    {
        return RPCR_ERROR;
    }

}

static INT32 _hndlr_x_mtkapi_bt_gap_pair(
                         RPC_ID_T     t_rpc_id,
                         const CHAR*  ps_cb_type,
                         UINT32       ui4_num_args,
                         ARG_DESC_T*  pt_args,
                         ARG_DESC_T*  pt_return)
{
    BT_RH_LOG("[_hndlr_]bt_gap_pair , arg_1 = %s, arg_2 = %ld\n", pt_args[0].u.ps_str, pt_args[1].u.i4_arg);

    if (ui4_num_args != 2)
    {
        BT_RH_LOG("Invalid ARGS: %d\n", ui4_num_args);
        return RPCR_INV_ARGS;
    }

    pt_return->e_type   = ARG_TYPE_INT32;
    pt_return->u.i4_arg = x_mtkapi_bt_gap_pair(pt_args[0].u.ps_str, pt_args[1].u.i4_arg);

    return RPCR_OK;
}

static INT32 _hndlr_x_mtkapi_bt_gap_unpair(
                         RPC_ID_T     t_rpc_id,
                         const CHAR*  ps_cb_type,
                         UINT32       ui4_num_args,
                         ARG_DESC_T*  pt_args,
                         ARG_DESC_T*  pt_return)
{
    BT_RH_LOG("[_hndlr_]bt_gap_paired_dev_erase , arg_1 = %s\n", pt_args[0].u.ps_str);

    if (ui4_num_args != 1)
    {
        BT_RH_LOG("Invalid ARGS: %d\n", ui4_num_args);
        return RPCR_INV_ARGS;
    }

    pt_return->e_type   = ARG_TYPE_INT32;
    pt_return->u.i4_arg = x_mtkapi_bt_gap_unpair(pt_args[0].u.ps_str);

    return RPCR_OK;
}

static INT32 _hndlr_x_mtkapi_bt_gap_get_rssi(
                         RPC_ID_T     t_rpc_id,
                         const CHAR*  ps_cb_type,
                         UINT32       ui4_num_args,
                         ARG_DESC_T*  pt_args,
                         ARG_DESC_T*  pt_return)
{
    BT_RH_LOG("[_hndlr_]bt_gap_get_rssi , arg_1 = %s, arg_2 = %p\n", pt_args[0].u.ps_str, pt_args[1].u.pi2_arg);

    if (ui4_num_args != 2)
    {
        BT_RH_LOG("Invalid ARGS: %d\n", ui4_num_args);
        return RPCR_INV_ARGS;
    }

    pt_return->e_type   = ARG_TYPE_INT32;
    pt_return->u.i4_arg = x_mtkapi_bt_gap_get_rssi(pt_args[0].u.ps_str, pt_args[1].u.pi2_arg);

    return RPCR_OK;
}

static INT32 _hndlr_x_mtkapi_bt_gap_set_virtual_sniffer(
                         RPC_ID_T     t_rpc_id,
                         const CHAR*  ps_cb_type,
                         UINT32       ui4_num_args,
                         ARG_DESC_T*  pt_args,
                         ARG_DESC_T*  pt_return)
{
    BT_RH_LOG("[_hndlr_]bt_gap_set_virtual_sniffer , arg_1 = %ld\n", pt_args[0].u.i4_arg);

    if (ui4_num_args != 1)
    {
        BT_RH_LOG("Invalid ARGS: %d\n", ui4_num_args);
        return RPCR_INV_ARGS;
    }

    pt_return->e_type   = ARG_TYPE_INT32;
    pt_return->u.i4_arg = x_mtkapi_bt_gap_set_virtual_sniffer(pt_args[0].u.i4_arg);

    return RPCR_OK;
}

static INT32 _hndlr_x_mtkapi_bt_gap_send_hci(
                         RPC_ID_T     t_rpc_id,
                         const CHAR*  ps_cb_type,
                         UINT32       ui4_num_args,
                         ARG_DESC_T*  pt_args,
                         ARG_DESC_T*  pt_return)
{
    BT_RH_LOG("[_hndlr_]bt_gap_send_hci , arg_1 = %s\n", pt_args[0].u.ps_str);

    if (ui4_num_args != 1)
    {
        BT_RH_LOG("Invalid ARGS: %d\n", ui4_num_args);
        return RPCR_INV_ARGS;
    }

    pt_return->e_type   = ARG_TYPE_INT32;
    pt_return->u.i4_arg = x_mtkapi_bt_gap_send_hci(pt_args[0].u.ps_str);

    return RPCR_OK;
}


INT32 c_rpc_reg_mtk_bt_service_gap_op_hndlrs(VOID)
{
    RPC_REG_OP_HNDLR(x_mtkapi_bt_base_init);
    RPC_REG_OP_HNDLR(x_mtkapi_bt_gap_set_dbg_level);
    RPC_REG_OP_HNDLR(x_mtkapi_bt_gap_on_off);
    RPC_REG_OP_HNDLR(x_mtkapi_bt_bluetooth_factory_reset);
    RPC_REG_OP_HNDLR(x_mtkapi_bt_gap_set_name);
    RPC_REG_OP_HNDLR(x_mtkapi_bt_gap_set_connectable_and_discoverable);
    RPC_REG_OP_HNDLR(x_mtkapi_bt_gap_get_dev_info);
    RPC_REG_OP_HNDLR(x_mtkapi_bt_gap_get_local_dev_info);
    RPC_REG_OP_HNDLR(x_mtkapi_bt_gap_start_inquiry_scan);
    RPC_REG_OP_HNDLR(x_mtkapi_bt_gap_stop_inquiry_scan);
    RPC_REG_OP_HNDLR(x_mtkapi_bt_gap_pair);
    RPC_REG_OP_HNDLR(x_mtkapi_bt_gap_unpair);
    RPC_REG_OP_HNDLR(x_mtkapi_bt_gap_get_rssi);
    RPC_REG_OP_HNDLR(x_mtkapi_bt_gap_set_virtual_sniffer);
    RPC_REG_OP_HNDLR(x_mtkapi_bt_gap_send_hci);
    return RPCR_OK;
}


