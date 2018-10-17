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

#ifndef _U_RPC_H_
#define _U_RPC_H_

/*-----------------------------------------------------------------------------
                    include files
 ----------------------------------------------------------------------------*/
#include "u_rpcipc_types.h"

#ifndef RPC_DEBUG
#define RPC_DEBUG
#endif


typedef UINT32 ARG_TYPE_T;



typedef INT32 RPC_ID_T;


#define RPC_DEFAULT_ID  ((RPC_ID_T)0)


typedef struct _ARG_DESC_T
{
    ARG_TYPE_T  e_type;

    union
    {
        CHAR     c_arg;
        INT8     i1_arg;
        INT16    i2_arg;
        INT32    i4_arg;
        INT64    i8_arg;
        UINT8    ui1_arg;
        UINT16   ui2_arg;
        UINT32   ui4_arg;
        UINT64   ui8_arg;
        SIZE_T   z_arg;
        BOOL     b_arg;

        VOID *   pv_arg;
        CHAR *   pc_arg;
        INT8 *   pi1_arg;
        INT16 *  pi2_arg;
        INT32 *  pi4_arg;
        INT64 *  pi8_arg;
        UINT8 *  pui1_arg;
        UINT16 * pui2_arg;
        UINT32 * pui4_arg;
        UINT64 * pui8_arg;
        SIZE_T * pz_arg;
        BOOL *   pb_arg;

        CHAR *   ps_str;
        VOID *   pv_func;

        VOID *   pv_desc;
    }   u;


}   ARG_DESC_T;



/* RPC description structure. */
typedef struct _RPC_DESC_ENTRY_T
{
    ARG_TYPE_T  e_type;

    const struct _RPC_DESC_T*  pt_desc;

    UINT32  ui4_num_entries;

    union
    {
        SIZE_T  z_offs;

        const CHAR*  ps_field_name;
    }   u;
}   RPC_DESC_ENTRY_T;


typedef struct _RPC_DESC_T
{
    ARG_TYPE_T  e_type;

    SIZE_T  z_size;

    UINT32  ui4_num_entries;

    RPC_DESC_ENTRY_T  at_desc_entries [];
}   RPC_DESC_T;


typedef VOID (*os_main_fct) (VOID*  pv_arg);

typedef INT32 (*os_thread_create_fct) (
    os_main_fct  pf_main,
    SIZE_T       z_stack_size,
    UINT8        ui1_priority,
    SIZE_T       z_arg_size,
    VOID*        pv_arg);

typedef struct _OS_FNCT_T
{
    os_thread_create_fct  pf_thread_create;
}   OS_FNCT_T;

typedef INT32 (*rpc_op_hndlr_fct) (
    RPC_ID_T     t_rpc_id,
    const CHAR*  ps_op,
    UINT32       ui4_num_args,
    ARG_DESC_T*  pt_args,
    ARG_DESC_T*  pt_return);

typedef INT32 (*rpc_cb_hndlr_fct) (
    RPC_ID_T     t_rpc_id,
    const CHAR*  ps_cb_type,
    VOID*        pv_cb_addr,
    UINT32       ui4_num_args,
    ARG_DESC_T*  pt_args,
    ARG_DESC_T*  pt_return);

typedef void (*rpc_log_fct)(INT32 i4_level, char * ps_string);


#define RPCR_OK          ((INT32)0)
#define RPCR_ERROR    ((INT32)-1)
#define RPCR_INV_IPC_ID  ((INT32)-2)
#define RPCR_INV_ARGS    ((INT32)-3)
#define RPCR_INV_SERVER  ((INT32)-4)
#define RPCR_CNN_FAIL    ((INT32)-5)
#define RPCR_NOT_OPENED  ((INT32)-6)
#define RPCR_TIMEOUT     ((INT32)-7)
#define RPCR_OSERR       ((INT32)-8)
#define RPCR_OUTOFMEMORY ((INT32)-9)
#define RPCR_RCVD_ERR    ((INT32)-10)
#define RPCR_NOT_FOUND   ((INT32)-11)
#define RPCR_EXIST       ((INT32)-12)

#define ARG_MASK_DIR            ((ARG_TYPE_T) 0xc0)
#define ARG_MASK_TYPE           ((ARG_TYPE_T) 0x3f)


#define ARG_DIR_INP             ((ARG_TYPE_T) 0x40)
#define ARG_DIR_OUT             ((ARG_TYPE_T) 0x80)
#define ARG_DIR_IO              (ARG_DIR_INP | ARG_DIR_OUT)

#define ARG_TYPE_VOID           ((ARG_TYPE_T)  0)
#define ARG_TYPE_CHAR           ((ARG_TYPE_T)  1)
#define ARG_TYPE_INT8           ((ARG_TYPE_T)  2)
#define ARG_TYPE_INT16          ((ARG_TYPE_T)  3)
#define ARG_TYPE_INT32          ((ARG_TYPE_T)  4)
#define ARG_TYPE_INT64          ((ARG_TYPE_T)  5)
#define ARG_TYPE_UINT8          ((ARG_TYPE_T)  6)
#define ARG_TYPE_UINT16         ((ARG_TYPE_T)  7)
#define ARG_TYPE_UINT32         ((ARG_TYPE_T)  8)
#define ARG_TYPE_UINT64         ((ARG_TYPE_T)  9)
#define ARG_TYPE_SIZE_T         ((ARG_TYPE_T) 10)
#define ARG_TYPE_BOOL           ((ARG_TYPE_T) 11)

#define ARG_TYPE_REF_VOID       ((ARG_TYPE_T) 16)
#define ARG_TYPE_REF_CHAR       ((ARG_TYPE_T) 17)
#define ARG_TYPE_REF_INT8       ((ARG_TYPE_T) 18)
#define ARG_TYPE_REF_INT16      ((ARG_TYPE_T) 19)
#define ARG_TYPE_REF_INT32      ((ARG_TYPE_T) 20)
#define ARG_TYPE_REF_INT64      ((ARG_TYPE_T) 21)
#define ARG_TYPE_REF_UINT8      ((ARG_TYPE_T) 22)
#define ARG_TYPE_REF_UINT16     ((ARG_TYPE_T) 23)
#define ARG_TYPE_REF_UINT32     ((ARG_TYPE_T) 24)
#define ARG_TYPE_REF_UINT64     ((ARG_TYPE_T) 25)
#define ARG_TYPE_REF_SIZE_T     ((ARG_TYPE_T) 26)
#define ARG_TYPE_REF_BOOL       ((ARG_TYPE_T) 27)

#define ARG_TYPE_REF_STR        ((ARG_TYPE_T) 32)
#define ARG_TYPE_REF_FUNC       ((ARG_TYPE_T) 33)

#define ARG_TYPE_DESC           ((ARG_TYPE_T) 48)
#define ARG_TYPE_REF_DESC       ((ARG_TYPE_T) 49)
#define ARG_TYPE_VARIABLE       ((ARG_TYPE_T) 50)
#define ARG_TYPE_STRUCT         ((ARG_TYPE_T) 51)
#define ARG_TYPE_UNION          ((ARG_TYPE_T) 52)

#define RPC_LOG_NONE           ((INT32)0)
#define RPC_LOG_INFO        ((INT32)1)
#define RPC_LOG_WARNING         ((INT32)2)
#define RPC_LOG_ERROR        ((INT32)4)
#define RPC_LOG_FAILURE      ((INT32)8)
#define RPC_LOG_NORMAL       ((INT32)16)

#define RPC_LOG_ALL      ((INT32)(RPC_LOG_INFO|RPC_LOG_WARNING|RPC_LOG_ERROR|RPC_LOG_FAILURE))


#endif  /* _U_RPC_H_  */


