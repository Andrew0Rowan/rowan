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

#ifndef __IPC_H_
#define __IPC_H_

#include "u_rpc.h"

#define IPCR_OK          ((INT32)RPCR_OK)
#define IPCR_INV_IPC_ID  ((INT32)RPCR_INV_IPC_ID)
#define IPCR_INV_ARGS    ((INT32)RPCR_INV_ARGS)
#define IPCR_INV_SERVER  ((INT32)RPCR_INV_SERVER)
#define IPCR_CNN_FAIL    ((INT32)RPCR_CNN_FAIL)
#define IPCR_NOT_OPENED  ((INT32)RPCR_NOT_OPENED)
#define IPCR_TIMEOUT     ((INT32)RPCR_TIMEOUT)
#define IPCR_OSERR       ((INT32)RPCR_OSERR)
#define IPCR_OUTOFMEMORY ((INT32)RPCR_OUTOFMEMORY)
#define IPCR_RCVD_ERR    ((INT32)RPCR_RCVD_ERR)



/* IPC Message */
#define IPCM_MASK_TYPE         ((UINT32) 0xfe)


#define IPCM_TYPE_OP           ((UINT32) 0x02)
#define IPCM_TYPE_CB           ((UINT32) 0x04)
#define IPCM_TYPE_RET          ((UINT32) 0x06)
#define IPCM_TYPE_ERROR        ((UINT32) 0x08)


typedef INT32  IPC_ID_T;

#define IPC_DEFAULT_ID ((IPC_ID_T)0)


typedef RPC_DESC_T* (*ipc_get_rpc_desc_fct) (
    VOID*    pv_addr,
    VOID*    pv_tag,
    VOID**   ppv_start_addr);

typedef INT32 (*ipc_proc_msg_fct) (
    const VOID*  pv_msg,
    SIZE_T       z_size);

typedef VOID (*ipc_close_fct) (
    VOID*  pv_tag);


typedef INT32 (*ipc_proc_rpc_cb_fct) (
    IPC_ID_T        t_ipc_id,
    const CHAR*     ps_cb_type,
    VOID*           pv_cb_addr,
    UINT32          ui4_num_args,
    ARG_DESC_T*     pt_args,
    ARG_DESC_T*     pt_return);

typedef INT32 (*ipc_proc_rpc_op_fct) (
    IPC_ID_T        t_ipc_id,
    const CHAR*     ps_op,
    UINT32          ui4_num_args,
    ARG_DESC_T*     pt_args,
    ARG_DESC_T*     pt_return);

/*IPC_ID_T ipc_get_cur_id();*/

extern IPC_ID_T ipc_open_client (
    const CHAR*                ps_server,
    VOID *                     pv_tag,
    ipc_close_fct              pf_close,
    ipc_proc_msg_fct           pf_proc_msg  ,
    ipc_proc_rpc_cb_fct        pf_proc_rpc_cb);

extern IPC_ID_T ipc_open_server (
    const CHAR*          ps_server,
    VOID *               pv_tag,
    ipc_close_fct        pf_close,
    ipc_proc_msg_fct     pf_proc_msg,
    ipc_proc_rpc_op_fct  pf_rpc_op);

extern INT32 ipc_close_client (IPC_ID_T  t_ipc_id, BOOL bAuto);

extern INT32 ipc_close_server (IPC_ID_T  t_ipc_id);


extern INT32 ipc_do_cb(
    IPC_ID_T                  t_ipc_id,
    const CHAR*               ps_cb_type,
    VOID*                     pv_cb_addr,
    UINT32                    ui4_num_args,
    ARG_DESC_T*               pt_args,
    ARG_DESC_T*               pt_return,
    UINT32                    ui4_timeout,
    ipc_get_rpc_desc_fct      pf_get_rpc_desc,
    VOID*                     pv_tag);


extern INT32 ipc_do_op (
    IPC_ID_T                  t_ipc_id,
    const CHAR*               ps_op,
    UINT32                    ui4_num_args,
    ARG_DESC_T*               pt_args,
    ARG_DESC_T*               pt_return,
    UINT32                    ui4_timeout,
    ipc_get_rpc_desc_fct      pf_get_rpc_desc,
    VOID*                     pv_tag);

extern INT32 ipc_init(
    UINT32              ui4_max_id,
    const OS_FNCT_T*    pt_os_fnct);

extern INT32 ipc_set_close_fct(IPC_ID_T t_ipc_id, void * pv_tag, ipc_close_fct pf_close);

extern VOID ipc_uninit();

VOID ipc_uninit();

VOID ipc_push_logger(IPC_ID_T t_ipc_id);
VOID ipc_pop_logger(IPC_ID_T t_ipc_id);
VOID ipc_update_logger(IPC_ID_T t_ipc_id);

#endif
