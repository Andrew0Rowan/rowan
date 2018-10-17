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

#ifndef _RI_COMMON_H_
#define _RI_COMMON_H_

#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include "rpc.h"

extern INT32 c_rpc_server_init(VOID);

extern RPC_ID_T c_rpc_start_server(VOID);
extern RPC_ID_T c_rpc_start_snd_server(VOID);
extern RPC_ID_T c_rpc_start_misc_server(VOID);

/*
 * These are some helper functions for RPC caller.
 */
#define RPC_DECL(num, ret_type)  \
    ARG_DESC_T __at_args[num];   \
    UINT32     __ui4_idx = 0;    \
    UINT32     __num_check = num;\
    ARG_DESC_T __t_ret;          \
    ret_type   __ret;            \
    INT32      __rpc_ret = RPCR_OK; \
    __t_ret.e_type = ARG_TYPE_VOID

#define RPC_DECL_VOID(num)       \
    ARG_DESC_T __at_args[num];   \
    UINT32     __ui4_idx = 0;    \
    UINT32     __num_check = num;\
    ARG_DESC_T __t_ret;          \
    INT32      __rpc_ret = RPCR_OK; \
    __t_ret.e_type = ARG_TYPE_VOID

#define RPC_DECL_NO_ARG          \
    UINT32     __num_check = num;\
    ARG_DESC_T __t_ret;           \
    INT32      __rpc_ret = RPCR_OK; \
    __t_ret.e_type = ARG_TYPE_VOID

/*
 * C stadard says that all union fields' start addresses are the same.
 */
#define RPC_ARG(e_arg_type, arg_var)                                \
do {                                                                \
    __at_args[(__ui4_idx)].e_type = (e_arg_type);                   \
    memcpy(&__at_args[(__ui4_idx) ++].u, &arg_var, sizeof(arg_var));\
}while(0)

#define RPC_ARG_INP(e_arg_type, arg_var) RPC_ARG(e_arg_type | ARG_DIR_INP, arg_var)
#define RPC_ARG_OUT(e_arg_type, arg_var) RPC_ARG(e_arg_type | ARG_DIR_OUT, arg_var)
#define RPC_ARG_IO(e_arg_type, arg_var)  RPC_ARG(e_arg_type | ARG_DIR_IO,  arg_var)

/*Fix this;*/
#define RPC_FAIL __rpc_ret = RPCR_INV_ARGS

#ifndef RPC_DEBUG
#define RPC_DEBUG
#endif


/*TODO: Refactoring: delete RPC_ERROR*/
#ifdef RPC_DEBUG

#define RPC_ERROR printf
#define RI_LOG printf
#define RI_ERR printf
#define RI_INF   printf

#else

#define RPC_ERROR(...)
#define RI_LOG(...)
#define RI_ERR(...)
#define RI_INF(...)

#endif

/* Any RPC operation should check __rpc_ret first and fill return value into __rpc_ret */

#define RPC_CHECK(stmt)                                           \
do                                                                \
{                                                                 \
    if(__rpc_ret == RPCR_OK)                                      \
    {\
        if((__rpc_ret = (stmt)) != RPCR_OK)                           \
        {                                                             \
            RPC_ERROR("RPC Failure:%s #%d\n", __FUNCTION__, __LINE__);\
        }                                                             \
    }                                                                 \
}while(0)


#define RPC_DO_CB(t_id, ps_cb, pv_cb_addr)                        \
do                                                                \
{                                                                 \
    if(__rpc_ret == RPCR_OK)                                      \
    {\
        if(__num_check == __ui4_idx)                                  \
        {\
            if((__rpc_ret = bt_rpc_do_cb(                                    \
                            t_id,                                         \
                            (ps_cb),                                      \
                            (pv_cb_addr),                                 \
                            __ui4_idx,                                    \
                            __at_args,                                    \
                            &__t_ret,                                     \
                            0xffffffff)) != RPCR_OK)                      \
            {                                                             \
                if (RPCR_TIMEOUT == __rpc_ret)    \
                {    \
                    usleep(50*1000);                          \
                    if((__rpc_ret = bt_rpc_do_cb(                                    \
                            t_id,                                         \
                            (ps_cb),                                      \
                            (pv_cb_addr),                                 \
                            __ui4_idx,                                    \
                            __at_args,                                    \
                            &__t_ret,                                     \
                            0xffffffff)) != RPCR_OK)                      \
                    {                                                             \
                        RPC_ERROR("RPC Retry Failure:%s again #%d\n", __FUNCTION__, __LINE__);\
                    }  \
                }  \
                else   \
                {    \
                RPC_ERROR("RPC Failure:%s #%d\n", __FUNCTION__, __LINE__);\
            }                                                             \
        }\
        }\
        else\
        {                                                             \
            RPC_ERROR("RPC Failure arg check %s #%d\n", __FUNCTION__, __LINE__);\
            bt_rpc_del(RPC_DEFAULT_ID);                                  \
        }                                                             \
    }                                                                 \
}while(0)

#define RPC_CHECK_RETURN_VAL(e_ret_type)            \
do {                                                \
    if(__rpc_ret == RPCR_OK && __t_ret.e_type == (e_ret_type)) \
    {                                               \
        return TRUE                                 \
    }                                               \
    else                                            \
    {                                               \
        return FALSE                                \
    }                                               \
}while(0)

#define RPC_RETURN(e_ret_type, fail_ret)            \
do {                                                \
    if(__rpc_ret == RPCR_OK && __t_ret.e_type == (e_ret_type)) \
    {                                               \
        memcpy(&__ret, &__t_ret.u, sizeof(__ret));  \
        bt_rpc_del(RPC_DEFAULT_ID);                    \
        return __ret;                               \
    }                                               \
    else                                            \
    {                                               \
        RPC_ERROR("RPC Failure ret type:%s #%d\n", __FUNCTION__, __LINE__);\
        bt_rpc_del(RPC_DEFAULT_ID);                    \
        return fail_ret;                            \
    }                                               \
}while(0)


#define RPC_RETURN_VOID     \
do{                         \
    bt_rpc_del(RPC_DEFAULT_ID);\
    return;                 \
}while(0)

/* Decrecated !! */
#define RPC_CB_TAG(t_rpc_id, pt_nfy_tag, pv_func, pv_tag) \
   pt_nfy_tag = ri_create_cb_tag(t_rpc_id, &pv_func, 1, pv_tag)

/* Decrecated !! */
#define RPC_CB_TAG_EX(t_rpc_id, pt_nfy_tag, apv_func, z_num, pv_tag) \
   pt_nfy_tag = ri_create_cb_tag(t_rpc_id, apv_func, z_num, pv_tag)

#define RPC_CB_TAG_FREE(pt_nfy_tag) \
    ri_free_cb_tag(pt_nfy_tag)

/* This could help register handler with prefix __hndlr_...*/
#define RPC_REG_OP_HNDLR(name) \
do{RI_INF("REG OP HNDLR %s, 0x%p\n", #name, (void *)_hndlr_##name);bt_rpc_reg_op_hndlr(#name, _hndlr_##name);}while(0)



typedef struct _RPC_CB_NFY_TAG_T
{
    RPC_ID_T        t_id;
    VOID *          pv_cb_addr;
    VOID *          apv_cb_addr_ex[25];

    VOID *          pv_tag;
}RPC_CB_NFY_TAG_T;


typedef struct _EXT_RPC_CB_NFY_TAG_T
{
    RPC_ID_T        t_id;
    UINT32           ui4_pb_handle;
    VOID *          pv_cb_addr;
    VOID *          apv_cb_addr_ex[25];

    VOID *          pv_tag;
}EXT_RPC_CB_NFY_TAG_T;


RPC_CB_NFY_TAG_T * ri_create_cb_tag(
    RPC_ID_T t_rpc_id,
    VOID *   apv_cb_addr_ex[],
    SIZE_T   z_num,
    VOID *   pv_tag);

EXT_RPC_CB_NFY_TAG_T * ri_create_ext_cb_tag(
    RPC_ID_T t_rpc_id,
    VOID *   apv_cb_addr_ex[],
    SIZE_T   z_num,
    VOID *   pv_tag,
    UINT32    ui4_pb_handle);

VOID ri_free_cb_tag(RPC_CB_NFY_TAG_T *   pt_nfy_tag);

VOID ri_free_ext_cb_tag(EXT_RPC_CB_NFY_TAG_T *   pt_ext_nfy_tag);

#endif
