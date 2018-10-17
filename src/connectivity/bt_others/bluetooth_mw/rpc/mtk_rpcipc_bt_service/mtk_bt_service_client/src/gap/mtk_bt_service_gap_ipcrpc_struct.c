/* This source file was automatically created by the */
/* tool 'MTK RPC Description tool', 'Version 1.10' on 'Tue Dec 26 15:05:37 2017'. */
/* Do NOT modify this source file. */



/* Start of source pre-amble file 'src_header_file.h'. */

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

#include "mtk_bt_service_gap_wrapper.h"
#include "mtk_bt_service_gap_ipcrpc_struct.h"


/* End of source pre-amble file 'src_header_file.h'. */

static const RPC_DESC_T t_rpc_decl_BLUETOOTH_DEVICE;
static const RPC_DESC_T t_rpc_decl_tBTMW_GAP_STATE;
static const RPC_DESC_T t_rpc_decl_tBTMW_GAP_DEVICE_INFO;
static const RPC_DESC_T t_rpc_decl_BT_LOCAL_DEV;
static const RPC_DESC_T t_rpc_decl_pairing_key_value_t;
static const RPC_DESC_T t_rpc_decl_BT_APP_CB_FUNC;
static const RPC_DESC_T t_rpc_decl_MTKRPCAPI_BT_APP_CB_FUNC;



static const RPC_DESC_T t_rpc_decl_BLUETOOTH_DEVICE =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BLUETOOTH_DEVICE),
    .ui4_num_entries = 0
};

static const RPC_DESC_T t_rpc_decl_tBTMW_GAP_STATE =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (tBTMW_GAP_STATE),
    .ui4_num_entries = 0
};

static const RPC_DESC_T t_rpc_decl_tBTMW_GAP_DEVICE_INFO =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (tBTMW_GAP_DEVICE_INFO),
    .ui4_num_entries = 0
};

static const RPC_DESC_T t_rpc_decl_BT_LOCAL_DEV =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_LOCAL_DEV),
    .ui4_num_entries = 0
};

static const RPC_DESC_T t_rpc_decl_pairing_key_value_t =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (pairing_key_value_t),
    .ui4_num_entries = 0
};

static const RPC_DESC_T t_rpc_decl_BT_APP_CB_FUNC =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_APP_CB_FUNC),
    .ui4_num_entries = 0
};

static const RPC_DESC_T t_rpc_decl_MTKRPCAPI_BT_APP_CB_FUNC =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (MTKRPCAPI_BT_APP_CB_FUNC),
    .ui4_num_entries = 0
};


static const RPC_DESC_T* at_rpc_desc_list [] =
{
    &t_rpc_decl_BLUETOOTH_DEVICE,
    &t_rpc_decl_tBTMW_GAP_STATE,
    &t_rpc_decl_tBTMW_GAP_DEVICE_INFO,
    &t_rpc_decl_BT_LOCAL_DEV,
    &t_rpc_decl_pairing_key_value_t,
    &t_rpc_decl_BT_APP_CB_FUNC,
    &t_rpc_decl_MTKRPCAPI_BT_APP_CB_FUNC
};

EXPORT_SYMBOL const RPC_DESC_T* __rpc_get_gap_desc__ (UINT32  ui4_idx)
{
  return ((ui4_idx < 7) ? at_rpc_desc_list [ui4_idx] : NULL);
}


