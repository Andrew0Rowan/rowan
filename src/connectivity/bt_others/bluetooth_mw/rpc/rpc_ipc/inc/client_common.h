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

#ifndef _CLIENT_COMMON_H_
#define _CLIENT_COMMON_H_

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "init_mtk_bt_service_client.h"
#include "rpc.h"
#include "_rpc_ipc_util.h"

/*
 * These are some helper functions for RPC caller.
 */
#define RPC_CLIENT_DECL(num, ret_type)  \
    ARG_DESC_T __at_args[num];   \
    UINT32     __ui4_idx = 0;    \
    UINT32     __num_check = num;\
    ARG_DESC_T __t_ret;          \
    __t_ret.e_type = ARG_TYPE_VOID;\
    ret_type   __ret;            \
    INT32      __rpc_ret = RPCR_OK

#define RPC_CLIENT_DECL_VOID(num)       \
    ARG_DESC_T __at_args[num];   \
    UINT32     __ui4_idx = 0;    \
    UINT32     __num_check = num;\
    ARG_DESC_T __t_ret;          \
    INT32      __rpc_ret = RPCR_OK

#define RPC_CLIENT_DECL_NO_ARG          \
    UINT32     __num_check = num;\
    ARG_DESC_T __t_ret           \
    INT32      __rpc_ret = RPCR_OK

/*
 * C stadard says that all union fields' start addresses are the same.
 */
#define RPC_CLIENT_ARG(e_arg_type, arg_var)                                \
do {                                                                \
    __at_args[(__ui4_idx)].e_type = (e_arg_type);                   \
    memcpy(&__at_args[(__ui4_idx) ++].u, &arg_var, sizeof(arg_var));\
}while(0)

#define RPC_CLIENT_ARG_INP(e_arg_type, arg_var) RPC_CLIENT_ARG(e_arg_type | ARG_DIR_INP, arg_var)
#define RPC_CLIENT_ARG_OUT(e_arg_type, arg_var) RPC_CLIENT_ARG(e_arg_type | ARG_DIR_OUT, arg_var)
#define RPC_CLIENT_ARG_IO(e_arg_type, arg_var)  RPC_CLIENT_ARG(e_arg_type | ARG_DIR_IO,  arg_var)

/*Fix this;*/
#define RPC_CLIENT_FAIL __rpc_ret = RPCR_INV_ARGS

#ifndef RPC_CLIENT_DEBUG
#define RPC_CLIENT_DEBUG
#endif


/*TODO: Refactoring: delete RPC_CLIENT_ERROR*/
#ifdef RPC_CLIENT_DEBUG

#define RPC_CLIENT_ERROR printf
#define RI_CLIENT_LOG printf
#define RI_CLIENT_ERR printf
#define RI_CLIENT_INF   printf

#else

#define RPC_CLIENT_ERROR(...)
#define RI_CLIENT_LOG(...)
#define RI_CLIENT_ERR(...)
#define RI_CLIENT_INF(...)

#endif

/* Any RPC operation should check __rpc_ret first and fill return value into __rpc_ret */

#define RPC_CLIENT_CHECK(stmt)                                           \
do                                                                \
{                                                                 \
    if(__rpc_ret == RPCR_OK)                                      \
    {\
        if((__rpc_ret = (stmt)) != RPCR_OK)                           \
        {                                                             \
            RPC_CLIENT_ERROR("RPC Failure:%s #%d\n", __FUNCTION__, __LINE__);\
        }                                                             \
    }                                                                 \
}while(0)

#define RPC_CLIENT_DO_OP(ps_op)                                              \
do                                                                    \
{                                                                     \
    if(__rpc_ret == RPCR_OK)                                          \
    {\
        if(__num_check == __ui4_idx)                                  \
        {\
            __rpc_ret = bt_rpc_do_op(RPC_DEFAULT_ID,                     \
                                  (ps_op),                            \
                                  __ui4_idx,                          \
                                  __at_args,                          \
                                  &__t_ret,                           \
                                  0xffffffff);                        \
            if(__rpc_ret == RPCR_NOT_OPENED)                          \
            {\
                printf("Open client:%s #%d try to auto connect \n", __FUNCTION__, __LINE__);\
                __rpc_ret = c_rpc_start_client();                     \
                if(__rpc_ret > RPCR_OK)\
                {\
                    __rpc_ret = bt_rpc_do_op(RPC_DEFAULT_ID,             \
                                      (ps_op),                        \
                                      __ui4_idx,                      \
                                      __at_args,                      \
                                      &__t_ret,                       \
                                      0xffffffff);\
                }\
            }\
            if(__rpc_ret != RPCR_OK)\
            {                                                         \
                printf("RPC Failure:%s #%d, ret %d\n", __FUNCTION__, __LINE__, (int)__rpc_ret);\
            }                                                         \
            else\
            {\
                RPC_INFO("RPC Succeed:%s #%d, ret %d\n", __FUNCTION__, __LINE__, (int)__rpc_ret);\
            }\
        }\
        else\
        {                                                             \
            printf("RPC Failure arg check:%s #%d\n", __FUNCTION__, __LINE__);\
        }                                                             \
    }                                                                 \
}while(0)

#define RPC_SND_CLIENT_DO_OP(ps_op)                                              \
do                                                                    \
{                                                                     \
    if(__rpc_ret == RPCR_OK)                                          \
    {\
        if(__num_check == __ui4_idx)                                  \
        {\
            __rpc_ret = bt_rpc_do_op(RPC_DEFAULT_ID,                     \
                                  (ps_op),                            \
                                  __ui4_idx,                          \
                                  __at_args,                          \
                                  &__t_ret,                           \
                                  0xffffffff);                        \
            if(__rpc_ret == RPCR_NOT_OPENED)                          \
            {\
                printf("Open client:%s #%d try to auto connect \n", __FUNCTION__, __LINE__);\
                __rpc_ret = c_rpc_start_snd_client();                     \
                if(__rpc_ret > RPCR_OK)\
                {\
                    __rpc_ret = bt_rpc_do_op(RPC_DEFAULT_ID,             \
                                      (ps_op),                        \
                                      __ui4_idx,                      \
                                      __at_args,                      \
                                      &__t_ret,                       \
                                      0xffffffff);\
                }\
            }\
            if(__rpc_ret != RPCR_OK)\
            {                                                         \
                printf("RPC Failure:%s #%d, ret %d\n", __FUNCTION__, __LINE__, (int)__rpc_ret);\
            }                                                         \
            else\
            {\
                RPC_INFO("RPC Succeed:%s #%d, ret %d\n", __FUNCTION__, __LINE__, (int)__rpc_ret);\
            }\
        }\
        else\
        {                                                             \
            printf("RPC Failure arg check:%s #%d\n", __FUNCTION__, __LINE__);\
        }                                                             \
    }                                                                 \
}while(0)

#define RPC_MISC_CLIENT_DO_OP(ps_op)                                              \
do                                                                    \
{                                                                     \
    if(__rpc_ret == RPCR_OK)                                          \
    {\
        if(__num_check == __ui4_idx)                                  \
        {\
            __rpc_ret = bt_rpc_do_op(RPC_DEFAULT_ID,                     \
                                  (ps_op),                            \
                                  __ui4_idx,                          \
                                  __at_args,                          \
                                  &__t_ret,                           \
                                  0xffffffff);                        \
            if(__rpc_ret == RPCR_NOT_OPENED)                          \
            {\
                printf("Open client:%s #%d try to auto connect \n", __FUNCTION__, __LINE__);\
                __rpc_ret = c_rpc_start_misc_client();                     \
                if(__rpc_ret > RPCR_OK)\
                {\
                    __rpc_ret = bt_rpc_do_op(RPC_DEFAULT_ID,             \
                                      (ps_op),                        \
                                      __ui4_idx,                      \
                                      __at_args,                      \
                                      &__t_ret,                       \
                                      0xffffffff);\
                }\
            }\
            if(__rpc_ret != RPCR_OK)\
            {                                                         \
                printf("RPC Failure:%s #%d, ret %d\n", __FUNCTION__, __LINE__, (int)__rpc_ret);\
            }                                                         \
            else\
            {\
                RPC_INFO("RPC Succeed:%s #%d, ret %d\n", __FUNCTION__, __LINE__, (int)__rpc_ret);\
            }\
        }\
        else\
        {                                                             \
            printf("RPC Failure arg check:%s #%d\n", __FUNCTION__, __LINE__);\
        }                                                             \
    }                                                                 \
}while(0)

#define RPC_SDP_CLIENT_DO_OP(ps_op)                                              \
do                                                                    \
{                                                                     \
    if(__rpc_ret == RPCR_OK)                                          \
    {\
        if(__num_check == __ui4_idx)                                  \
        {\
            __rpc_ret = bt_rpc_do_op(RPC_DEFAULT_ID,                     \
                                  (ps_op),                            \
                                  __ui4_idx,                          \
                                  __at_args,                          \
                                  &__t_ret,                           \
                                  0xffffffff);                        \
            if(__rpc_ret == RPCR_NOT_OPENED)                          \
            {\
                printf("Open client:%s #%d try to auto connect \n", __FUNCTION__, __LINE__);\
                __rpc_ret = c_rpc_init_sdp_client();                     \
                if(__rpc_ret > RPCR_OK)\
                {\
                    __rpc_ret = bt_rpc_do_op(RPC_DEFAULT_ID,             \
                                      (ps_op),                        \
                                      __ui4_idx,                      \
                                      __at_args,                      \
                                      &__t_ret,                       \
                                      0xffffffff);\
                }\
            }\
            if(__rpc_ret != RPCR_OK)\
            {                                                         \
                printf("RPC Failure:%s #%d, ret %d\n", __FUNCTION__, __LINE__, (int)__rpc_ret);\
            }                                                         \
            else\
            {\
                IPC_INFO("RPC Succeed:%s #%d, ret %d\n", __FUNCTION__, __LINE__, (int)__rpc_ret);\
            }\
        }\
        else\
        {                                                             \
            printf("RPC Failure arg check:%s #%d\n", __FUNCTION__, __LINE__);\
        }                                                             \
    }                                                                 \
}while(0)

#define RPC_BT_SERVICE_CLIENT_DO_OP(ps_op)                                                               \
do                                                                                                \
{                                                                                                 \
    if(__rpc_ret == RPCR_OK)                                                                      \
    {                                                                                             \
        if(__num_check == __ui4_idx)                                                              \
        {                                                                                         \
            __rpc_ret = bt_rpc_do_op(RPC_DEFAULT_ID,                                                 \
                                  (ps_op),                                                        \
                                  __ui4_idx,                                                      \
                                  __at_args,                                                      \
                                  &__t_ret,                                                       \
                                  0xffffffff);                                                    \
            if(__rpc_ret == RPCR_NOT_OPENED)                                                      \
            {                                                                                     \
                printf("Open client:%s #%d try to auto connect \n", __FUNCTION__, __LINE__);      \
                __rpc_ret = c_rpc_start_mtk_bt_service_client();                                             \
                if(__rpc_ret > RPCR_OK)                                                           \
                {                                                                                 \
                    __rpc_ret = bt_rpc_do_op(RPC_DEFAULT_ID,                                         \
                                      (ps_op),                                                    \
                                      __ui4_idx,                                                  \
                                      __at_args,                                                  \
                                      &__t_ret,                                                   \
                                      0xffffffff);                                                \
                }                                                                                 \
            }                                                                                     \
            if(__rpc_ret != RPCR_OK)                                                              \
            {                                                                                     \
                printf("RPC Failure:%s #%d, ret %d\n", __FUNCTION__, __LINE__, (int)__rpc_ret);   \
            }                                                                                     \
            else                                                                                  \
            {                                                                                     \
                RPC_INFO("RPC Succeed:%s #%d, ret %d\n", __FUNCTION__, __LINE__, (int)__rpc_ret); \
            }                                                                                     \
        }                                                                                         \
        else                                                                                      \
        {                                                                                         \
            printf("RPC Failure arg check:%s #%d\n", __FUNCTION__, __LINE__);                     \
        }                                                                                         \
    }                                                                                             \
}while(0)

#define RPC_CLIENT_RETURN(e_ret_type, fail_ret)            \
do {                                                \
    if(__rpc_ret == RPCR_OK && __t_ret.e_type == (e_ret_type)) \
    {                                               \
        memcpy(&__ret, &__t_ret.u, sizeof(__ret));  \
        bt_rpc_del(RPC_DEFAULT_ID);                    \
        return __ret;                               \
    }                                               \
    else                                            \
    {                                               \
        RPC_CLIENT_ERROR("RPC Failure ret type:%s #%d, RPC ret:%d, T:%d\n", \
            __FUNCTION__, __LINE__, __rpc_ret, (int)__t_ret.e_type);\
        bt_rpc_del(RPC_DEFAULT_ID);                    \
        return fail_ret;                            \
    }                                               \
}while(0)

#define RPC_CLIENT_RETURN_NO_LOG(e_ret_type, fail_ret)            \
        do {                                                      \
                if(__rpc_ret == RPCR_OK && __t_ret.e_type == (e_ret_type)) \
                {                                                          \
                        memcpy(&__ret, &__t_ret.u, sizeof(__ret));         \
                        bt_rpc_del(RPC_DEFAULT_ID);                           \
                        return __ret;                                      \
                }                                                          \
                else                                                       \
                {                                                          \
                        bt_rpc_del(RPC_DEFAULT_ID);                           \
                        return fail_ret;                                   \
                }                                                          \
           }while(0)

#define RPC_REG_CB_HNDLR(name) \
do{\
    RI_CLIENT_INF("REG CB HNDLR %s, 0x%p\n", #name, (void *)_hndlr_##name);\
    i4_ret = bt_rpc_reg_cb_hndlr(#name, _hndlr_##name);\
    if (i4_ret < 0)                                                                               \
    {                                                                                           \
        printf("<CLIENT> log on fail: file = %s, line = %d, reason = %d\r\n", __FILE__, __LINE__, (int)i4_ret);        \
        i4_ret = 0;                          \
    }\
}while(0)


#endif
