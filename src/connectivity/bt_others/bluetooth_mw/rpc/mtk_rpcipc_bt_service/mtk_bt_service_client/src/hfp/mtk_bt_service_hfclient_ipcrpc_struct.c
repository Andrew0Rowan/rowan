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

#include "u_bt_mw_hfclient.h"
#include "mtk_bt_service_hfclient_ipcrpc_struct.h"

/* End of source pre-amble file 'src_header_file.h'. */

static const RPC_DESC_T t_rpc_decl_BLUETOOTH_DEVICE;
static const RPC_DESC_T t_rpc_decl_BT_HFCLIENT_ABILITY_CB_DATA_T;
static const RPC_DESC_T t_rpc_decl_BT_HFCLIENT_CONNECTION_CB_DATA_T;
static const RPC_DESC_T t_rpc_decl_BT_HFCLIENT_AUDIO_CONNECTION_CB_DATA_T;
static const RPC_DESC_T t_rpc_decl_BT_HFCLIENT_BVRA_CB_DATA_T;
static const RPC_DESC_T t_rpc_decl_BT_HFCLIENT_IND_SERVICE_CB_DATA_T;
static const RPC_DESC_T t_rpc_decl_BT_HFCLIENT_IND_ROAM_CB_DATA_T;
static const RPC_DESC_T t_rpc_decl_BT_HFCLIENT_IND_SIGNAL_CB_DATA_T;
static const RPC_DESC_T t_rpc_decl_BT_HFCLIENT_IND_BATTCH_CB_DATA_T;
static const RPC_DESC_T t_rpc_decl_BT_HFCLIENT_COPS_CB_DATA_T;
static const RPC_DESC_T t_rpc_decl_BT_HFCLIENT_IND_CALL_CB_DATA_T;
static const RPC_DESC_T t_rpc_decl_BT_HFCLIENT_IND_CALLSETUP_CB_DATA_T;
static const RPC_DESC_T t_rpc_decl_BT_HFCLIENT_IND_CALLHELD_CB_DATA_T;
static const RPC_DESC_T t_rpc_decl_BT_HFCLIENT_BTRH_CB_DATA_T;
static const RPC_DESC_T t_rpc_decl_BT_HFCLIENT_CLIP_CB_DATA_T;
static const RPC_DESC_T t_rpc_decl_BT_HFCLIENT_CCWA_CB_DATA_T;
static const RPC_DESC_T t_rpc_decl_BT_HFCLIENT_CLCC_CB_DATA_T;
static const RPC_DESC_T t_rpc_decl_BT_HFCLIENT_VGM_VGS_CB_DATA_T;
static const RPC_DESC_T t_rpc_decl_BT_HFCLIENT_CMD_COMPLETE_CB_DATA_T;
static const RPC_DESC_T t_rpc_decl_BT_HFCLIENT_CNUM_CB_DATA_T;
static const RPC_DESC_T t_rpc_decl_BT_HFCLIENT_BSIR_CB_DATA_T;
static const RPC_DESC_T t_rpc_decl_BT_HFCLIENT_BINP_CB_DATA_T;
static const RPC_DESC_T t_rpc_decl_BT_HFCLIENT_PHONEBOOK_ENTRY_T;
static const RPC_DESC_T t_rpc_decl_BT_HFCLIENT_PB_ENTRY_APP_DATA_T;
static const RPC_DESC_T t_rpc_decl_BT_HFCLIENT_CPBS_CB_DATA_T;
static const RPC_DESC_T t_rpc_decl_BT_HFCLIENT_CPBR_COUNT_CB_DATA_T;
static const RPC_DESC_T t_rpc_decl_BT_HFCLIENT_CPBR_ENTRY_CB_DATA_T;
static const RPC_DESC_T t_rpc_decl_BT_HFCLIENT_CB_DATA;
static const RPC_DESC_T t_rpc_decl_BT_HFCLIENT_EVENT_PARAM;



static const RPC_DESC_T t_rpc_decl_BLUETOOTH_DEVICE =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BLUETOOTH_DEVICE),
    .ui4_num_entries = 0
};

static const RPC_DESC_T t_rpc_decl_BT_HFCLIENT_ABILITY_CB_DATA_T =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_HFCLIENT_ABILITY_CB_DATA_T),
    .ui4_num_entries = 0
};

static const RPC_DESC_T t_rpc_decl_BT_HFCLIENT_CONNECTION_CB_DATA_T =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_HFCLIENT_CONNECTION_CB_DATA_T),
    .ui4_num_entries = 0
};

static const RPC_DESC_T t_rpc_decl_BT_HFCLIENT_AUDIO_CONNECTION_CB_DATA_T =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_HFCLIENT_AUDIO_CONNECTION_CB_DATA_T),
    .ui4_num_entries = 0
};

static const RPC_DESC_T t_rpc_decl_BT_HFCLIENT_BVRA_CB_DATA_T =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_HFCLIENT_BVRA_CB_DATA_T),
    .ui4_num_entries = 0
};

static const RPC_DESC_T t_rpc_decl_BT_HFCLIENT_IND_SERVICE_CB_DATA_T =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_HFCLIENT_IND_SERVICE_CB_DATA_T),
    .ui4_num_entries = 0
};

static const RPC_DESC_T t_rpc_decl_BT_HFCLIENT_IND_ROAM_CB_DATA_T =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_HFCLIENT_IND_ROAM_CB_DATA_T),
    .ui4_num_entries = 0
};

static const RPC_DESC_T t_rpc_decl_BT_HFCLIENT_IND_SIGNAL_CB_DATA_T =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_HFCLIENT_IND_SIGNAL_CB_DATA_T),
    .ui4_num_entries = 0
};

static const RPC_DESC_T t_rpc_decl_BT_HFCLIENT_IND_BATTCH_CB_DATA_T =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_HFCLIENT_IND_BATTCH_CB_DATA_T),
    .ui4_num_entries = 0
};

static const RPC_DESC_T t_rpc_decl_BT_HFCLIENT_COPS_CB_DATA_T =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_HFCLIENT_COPS_CB_DATA_T),
    .ui4_num_entries = 0
};

static const RPC_DESC_T t_rpc_decl_BT_HFCLIENT_IND_CALL_CB_DATA_T =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_HFCLIENT_IND_CALL_CB_DATA_T),
    .ui4_num_entries = 0
};

static const RPC_DESC_T t_rpc_decl_BT_HFCLIENT_IND_CALLSETUP_CB_DATA_T =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_HFCLIENT_IND_CALLSETUP_CB_DATA_T),
    .ui4_num_entries = 0
};

static const RPC_DESC_T t_rpc_decl_BT_HFCLIENT_IND_CALLHELD_CB_DATA_T =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_HFCLIENT_IND_CALLHELD_CB_DATA_T),
    .ui4_num_entries = 0
};

static const RPC_DESC_T t_rpc_decl_BT_HFCLIENT_BTRH_CB_DATA_T =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_HFCLIENT_BTRH_CB_DATA_T),
    .ui4_num_entries = 0
};

static const RPC_DESC_T t_rpc_decl_BT_HFCLIENT_CLIP_CB_DATA_T =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_HFCLIENT_CLIP_CB_DATA_T),
    .ui4_num_entries = 0
};

static const RPC_DESC_T t_rpc_decl_BT_HFCLIENT_CCWA_CB_DATA_T =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_HFCLIENT_CCWA_CB_DATA_T),
    .ui4_num_entries = 0
};

static const RPC_DESC_T t_rpc_decl_BT_HFCLIENT_CLCC_CB_DATA_T =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_HFCLIENT_CLCC_CB_DATA_T),
    .ui4_num_entries = 0
};

static const RPC_DESC_T t_rpc_decl_BT_HFCLIENT_VGM_VGS_CB_DATA_T =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_HFCLIENT_VGM_VGS_CB_DATA_T),
    .ui4_num_entries = 0
};

static const RPC_DESC_T t_rpc_decl_BT_HFCLIENT_CMD_COMPLETE_CB_DATA_T =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_HFCLIENT_CMD_COMPLETE_CB_DATA_T),
    .ui4_num_entries = 0
};

static const RPC_DESC_T t_rpc_decl_BT_HFCLIENT_CNUM_CB_DATA_T =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_HFCLIENT_CNUM_CB_DATA_T),
    .ui4_num_entries = 0
};

static const RPC_DESC_T t_rpc_decl_BT_HFCLIENT_BSIR_CB_DATA_T =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_HFCLIENT_BSIR_CB_DATA_T),
    .ui4_num_entries = 0
};

static const RPC_DESC_T t_rpc_decl_BT_HFCLIENT_BINP_CB_DATA_T =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_HFCLIENT_BINP_CB_DATA_T),
    .ui4_num_entries = 0
};

static const RPC_DESC_T t_rpc_decl_BT_HFCLIENT_PHONEBOOK_ENTRY_T =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_HFCLIENT_PHONEBOOK_ENTRY_T),
    .ui4_num_entries = 0
};

static const RPC_DESC_T t_rpc_decl_BT_HFCLIENT_PB_ENTRY_APP_DATA_T =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_HFCLIENT_PB_ENTRY_APP_DATA_T),
    .ui4_num_entries = 0
};

static const RPC_DESC_T t_rpc_decl_BT_HFCLIENT_CPBS_CB_DATA_T =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_HFCLIENT_CPBS_CB_DATA_T),
    .ui4_num_entries = 0
};

static const RPC_DESC_T t_rpc_decl_BT_HFCLIENT_CPBR_COUNT_CB_DATA_T =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_HFCLIENT_CPBR_COUNT_CB_DATA_T),
    .ui4_num_entries = 0
};

static const RPC_DESC_T t_rpc_decl_BT_HFCLIENT_CPBR_ENTRY_CB_DATA_T =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_HFCLIENT_CPBR_ENTRY_CB_DATA_T),
    .ui4_num_entries = 0
};

static const RPC_DESC_T t_rpc_decl_BT_HFCLIENT_CB_DATA =
{
    .e_type          = ARG_TYPE_UNION,
    .z_size          = sizeof (BT_HFCLIENT_CB_DATA),
    .ui4_num_entries = 0
};

static const RPC_DESC_T t_rpc_decl_BT_HFCLIENT_EVENT_PARAM =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_HFCLIENT_EVENT_PARAM),
    .ui4_num_entries = 0
};


static const RPC_DESC_T* at_rpc_desc_list [] =
{
    &t_rpc_decl_BLUETOOTH_DEVICE,
    &t_rpc_decl_BT_HFCLIENT_ABILITY_CB_DATA_T,
    &t_rpc_decl_BT_HFCLIENT_CONNECTION_CB_DATA_T,
    &t_rpc_decl_BT_HFCLIENT_AUDIO_CONNECTION_CB_DATA_T,
    &t_rpc_decl_BT_HFCLIENT_BVRA_CB_DATA_T,
    &t_rpc_decl_BT_HFCLIENT_IND_SERVICE_CB_DATA_T,
    &t_rpc_decl_BT_HFCLIENT_IND_ROAM_CB_DATA_T,
    &t_rpc_decl_BT_HFCLIENT_IND_SIGNAL_CB_DATA_T,
    &t_rpc_decl_BT_HFCLIENT_IND_BATTCH_CB_DATA_T,
    &t_rpc_decl_BT_HFCLIENT_COPS_CB_DATA_T,
    &t_rpc_decl_BT_HFCLIENT_IND_CALL_CB_DATA_T,
    &t_rpc_decl_BT_HFCLIENT_IND_CALLSETUP_CB_DATA_T,
    &t_rpc_decl_BT_HFCLIENT_IND_CALLHELD_CB_DATA_T,
    &t_rpc_decl_BT_HFCLIENT_BTRH_CB_DATA_T,
    &t_rpc_decl_BT_HFCLIENT_CLIP_CB_DATA_T,
    &t_rpc_decl_BT_HFCLIENT_CCWA_CB_DATA_T,
    &t_rpc_decl_BT_HFCLIENT_CLCC_CB_DATA_T,
    &t_rpc_decl_BT_HFCLIENT_VGM_VGS_CB_DATA_T,
    &t_rpc_decl_BT_HFCLIENT_CMD_COMPLETE_CB_DATA_T,
    &t_rpc_decl_BT_HFCLIENT_CNUM_CB_DATA_T,
    &t_rpc_decl_BT_HFCLIENT_BSIR_CB_DATA_T,
    &t_rpc_decl_BT_HFCLIENT_BINP_CB_DATA_T,
    &t_rpc_decl_BT_HFCLIENT_PHONEBOOK_ENTRY_T,
    &t_rpc_decl_BT_HFCLIENT_PB_ENTRY_APP_DATA_T,
    &t_rpc_decl_BT_HFCLIENT_CPBS_CB_DATA_T,
    &t_rpc_decl_BT_HFCLIENT_CPBR_COUNT_CB_DATA_T,
    &t_rpc_decl_BT_HFCLIENT_CPBR_ENTRY_CB_DATA_T,
    &t_rpc_decl_BT_HFCLIENT_CB_DATA,
    &t_rpc_decl_BT_HFCLIENT_EVENT_PARAM
};

EXPORT_SYMBOL const RPC_DESC_T* __rpc_get_hfclient_desc__ (UINT32  ui4_idx)
{
  return ((ui4_idx < 29) ? at_rpc_desc_list [ui4_idx] : NULL);
}


