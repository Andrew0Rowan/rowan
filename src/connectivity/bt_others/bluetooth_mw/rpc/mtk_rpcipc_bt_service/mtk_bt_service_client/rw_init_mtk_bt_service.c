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


#include "rpc.h"
#include "rw_init_mtk_bt_service.h"
#include "init_mtk_bt_service_client.h"
#include "_rpc_ipc_util.h"
#include "mtk_bt_service_gattc_wrapper.h"
#include "mtk_bt_service_gatts_wrapper.h"
#include "mtk_bt_service_a2dp_wrapper.h"
#include "mtk_bt_service_avrcp_wrapper.h"
#include "mtk_bt_service_gap_wrapper.h"
#include "mtk_bt_service_spp_wrapper.h"
#include "mtk_bt_service_hfclient_wrapper.h"

static BOOL b_imtk_bt_service_client_sys_init = FALSE;

EXPORT_SYMBOL RPC_ID_T c_rpc_start_mtk_bt_service_client(void)
{
  return bt_rpc_open_client("mtk_bt_service");
}

EXPORT_SYMBOL RPC_ID_T c_rpc_init_mtk_bt_service_client(void)
{
  return c_rpc_start_mtk_bt_service_client();
}

INT32 c_rpc_uninit_mtk_bt_service_client(RPC_ID_T t_rpc_id)
{
  bt_rpc_close_client(t_rpc_id);
  bt_rpc_del(t_rpc_id);
  return RPCR_OK;
}

EXPORT_SYMBOL void a_mtk_bt_service_init(void)
{
    if (!b_imtk_bt_service_client_sys_init)
    {
        b_imtk_bt_service_client_sys_init = TRUE;
        bt_rpc_init(NULL);
        c_rpc_init_mtk_bt_service_client();
        bt_rpcu_tl_log_start();
        c_rpc_reg_mtk_bt_service_gattc_cb_hndlrs();
        c_rpc_reg_mtk_bt_service_gatts_cb_hndlrs();
        c_rpc_reg_mtk_bt_service_a2dp_cb_hndlrs();
        c_rpc_reg_mtk_bt_service_avrcp_cb_hndlrs();
        c_rpc_reg_mtk_bt_service_gap_cb_hndlrs();
        c_rpc_reg_mtk_bt_service_spp_cb_hndlrs();
        c_rpc_reg_mtk_bt_service_hidh_cb_hndlrs();
        c_rpc_reg_mtk_bt_service_hfclient_cb_hndlrs();
    }
}

void a_mtk_bt_service_terminate(void)
{
    if (b_imtk_bt_service_client_sys_init)
    {
        b_imtk_bt_service_client_sys_init = FALSE;
        bt_rpcu_tl_log_end();
    }
}

