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

#ifndef _RPC_H_
#define _RPC_H_


#include "u_rpc.h"

#define RPC_KW_LOG_FUNCTION "-logfunction"
#define RPC_KW_LOG_LEVEL	"-loglevel"

#define RPC_ADD_KW(_kw)		" "_kw

extern INT32 bt_rpc_add_ref_buff (
    RPC_ID_T        t_rpc_id,
    const VOID*     pv_buff,
    SIZE_T          z_size);

extern INT32 bt_rpc_add_ref_desc (
    RPC_ID_T            t_rpc_id,
    const VOID *        pv_obj,
    const RPC_DESC_T*   pt_desc,
    const CHAR*         ps_field_name);

extern INT32 bt_rpc_add_ref_desc_arr (
    RPC_ID_T            t_rpc_id,
    UINT32              ui4_num_entries,
    const VOID*         pv_obj,
    const RPC_DESC_T*   pt_desc,
    const CHAR*         ps_field_name);


extern INT32 bt_rpc_add_ref_str (
    RPC_ID_T    t_rpc_id,
    const CHAR* ps_str);

extern INT32 bt_rpc_add_ref_val (
    RPC_ID_T     t_rpc_id,
    const VOID*  pv_ref);


extern INT32 bt_rpc_add_arg_type (
    RPC_ID_T    t_rpc_id,
    UINT32      ui4_arg_idx,
    ARG_TYPE_T  e_type);

extern ARG_TYPE_T bt_rpc_arg_type (
    RPC_ID_T  t_rpc_id,
    UINT32    ui4_arg_idx);


extern INT32 bt_rpc_close_client (
    RPC_ID_T  t_rpc_id);

extern INT32 bt_rpc_close_server (
    RPC_ID_T  t_rpc_id);

extern INT32 bt_rpc_del (
    RPC_ID_T  t_rpc_id);

extern INT32 bt_rpc_do_cb (
    RPC_ID_T     t_rpc_id,
    const CHAR*  ps_cb_type,
    VOID*        pv_cb_addr,
    UINT32       ui4_num_args,
    ARG_DESC_T*  pt_args,
    ARG_DESC_T*  pt_return,
    UINT32       ui4_timeout);

extern INT32 bt_rpc_do_op (
    RPC_ID_T     t_rpc_id,
    const CHAR*  ps_op,
    UINT32       ui4_num_args,
    ARG_DESC_T*  pt_args,
    ARG_DESC_T*  pt_return,
    UINT32       ui4_timeout);

extern RPC_ID_T bt_rpc_open_client (const CHAR*  ps_server,...);

extern INT32 bt_rpc_close_client (RPC_ID_T  t_ipc_id);

extern RPC_ID_T bt_rpc_open_server (const CHAR*  ps_server,...);

extern INT32 bt_rpc_close_server (RPC_ID_T  t_ipc_id);

extern INT32 bt_rpc_init(
    const OS_FNCT_T*  pt_os_fnct);

extern INT32 bt_rpc_set_opt(RPC_ID_T t_id, const CHAR * ps_opt, ...);

extern VOID bt_rpc_uninit();

extern VOID bt_rpc_simple_uninit();

extern INT32 bt_rpc_reg_op_hndlr (
    const CHAR*       ps_op,
    rpc_op_hndlr_fct  pf_op_hndlr);

extern INT32 bt_rpc_reg_cb_hndlr (
    const CHAR*       ps_cb_type,
    rpc_cb_hndlr_fct  pf_cb_hndlr);

extern INT32 bt_rpc_get_op_hndlr(
    const CHAR *       ps_op,
    rpc_op_hndlr_fct * ppf_handler);

extern INT32 bt_rpc_get_cb_hndlr(
    const CHAR *       ps_cb_type,
    rpc_cb_hndlr_fct * ppf_handler);

extern INT32 bt_rpc_handler_init();

extern VOID bt_rpc_handler_uninit();

extern BOOL bt_rpcu_tl_log_is_on();

extern VOID bt_rpcipc_set_log_status(BOOL b_on);

extern VOID bt_rpcu_default_output_log(INT32 i4_level, CHAR* ps_string);

#endif /* _RPC_H_*/

