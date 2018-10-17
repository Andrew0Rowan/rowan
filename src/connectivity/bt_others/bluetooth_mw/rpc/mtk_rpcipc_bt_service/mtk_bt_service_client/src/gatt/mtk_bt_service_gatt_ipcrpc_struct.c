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

#include "u_bt_mw_gattc.h"
#include "u_bt_mw_gatts.h"
#include "mtk_bt_service_gattc_wrapper.h"
#include "mtk_bt_service_gatts_wrapper.h"
#include "mtk_bt_service_gatt_ipcrpc_struct.h"



/* End of source pre-amble file 'src_header_file.h'. */

static const RPC_DESC_T t_rpc_decl_BLUETOOTH_DEVICE;
static const RPC_DESC_T t_rpc_decl_BT_GATT_ID_T;
static const RPC_DESC_T t_rpc_decl_BT_GATT_SRVC_ID_T;
static const RPC_DESC_T t_rpc_decl_BT_GATTC_REG_CLIENT_T;
static const RPC_DESC_T t_rpc_decl_BT_GATTC_SCAN_RST_T;
static const RPC_DESC_T t_rpc_decl_BT_GATTC_CONNECT_RST_T;
static const RPC_DESC_T t_rpc_decl_BT_GATTC_GET_REG_NOTI_RST_T;
static const RPC_DESC_T t_rpc_decl_BT_GATTC_NOTI_PARAMS_T;
static const RPC_DESC_T t_rpc_decl_BT_GATTC_GET_NOTIFY_T;
static const RPC_DESC_T t_rpc_decl_BT_GATTC_UNFORMATTED_VALUE_T;
static const RPC_DESC_T t_rpc_decl_BT_GATTC_READ_PARAMS_T;
static const RPC_DESC_T t_rpc_decl_BT_GATTC_READ_CHAR_RST_T;
static const RPC_DESC_T t_rpc_decl_BT_GATTC_WRITE_CHAR_RST_T;
static const RPC_DESC_T t_rpc_decl_BT_GATTC_READ_DESCR_RST_T;
static const RPC_DESC_T t_rpc_decl_BT_GATTC_WRITE_DESCR_RST_T;
static const RPC_DESC_T t_rpc_decl_BT_GATTC_GET_REMOTE_RSSI_T;
static const RPC_DESC_T t_rpc_decl_BT_GATTC_SCAN_FILTER_PARAM_T;
static const RPC_DESC_T t_rpc_decl_BT_GATTC_SCAN_FILTER_STATUS_T;
static const RPC_DESC_T t_rpc_decl_BT_GATTC_SCAN_FILTER_CFG_T;
static const RPC_DESC_T t_rpc_decl_BT_GATTC_FILT_PARAM_SETUP_T;
static const RPC_DESC_T t_rpc_decl_BT_GATTC_DB_ELEMENT_T;
static const RPC_DESC_T t_rpc_decl_BT_GATTC_GET_GATT_DB_T;
static const RPC_DESC_T t_rpc_decl_BT_GATTC_TEST_PARAMS_T;
static const RPC_DESC_T t_rpc_decl_BT_APP_GATTC_CB_FUNC_T;
static const RPC_DESC_T t_rpc_decl_MTKRPCAPI_BT_APP_GATTC_CB_FUNC_T;
static const RPC_DESC_T t_rpc_decl_BT_GATTS_REG_SERVER_RST_T;
static const RPC_DESC_T t_rpc_decl_BT_GATTS_CONNECT_RST_T;
static const RPC_DESC_T t_rpc_decl_BT_GATTS_ADD_SRVC_RST_T;
static const RPC_DESC_T t_rpc_decl_BT_GATTS_ADD_INCL_RST_T;
static const RPC_DESC_T t_rpc_decl_BT_GATTS_ADD_CHAR_RST_T;
static const RPC_DESC_T t_rpc_decl_BT_GATTS_ADD_DESCR_RST_T;
static const RPC_DESC_T t_rpc_decl_BT_GATTS_SRVC_RST_T;
static const RPC_DESC_T t_rpc_decl_BT_GATTS_REQ_READ_RST_T;
static const RPC_DESC_T t_rpc_decl_BT_GATTS_REQ_WRITE_RST_T;
static const RPC_DESC_T t_rpc_decl_BT_GATTS_EXEC_WRITE_RST_T;
static const RPC_DESC_T t_rpc_decl_BT_GATTS_SENT_IND_RST_T;
static const RPC_DESC_T t_rpc_decl_BT_APP_GATTS_CB_FUNC_T;
static const RPC_DESC_T t_rpc_decl_MTKRPCAPI_BT_APP_GATTS_CB_FUNC_T;



static const RPC_DESC_T t_rpc_decl_BLUETOOTH_DEVICE =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BLUETOOTH_DEVICE),
    .ui4_num_entries = 0
};

static const RPC_DESC_T t_rpc_decl_BT_GATT_ID_T =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_GATT_ID_T),
    .ui4_num_entries = 0
};

static const RPC_DESC_T t_rpc_decl_BT_GATT_SRVC_ID_T =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_GATT_SRVC_ID_T),
    .ui4_num_entries = 0
};

static const RPC_DESC_T t_rpc_decl_BT_GATTC_REG_CLIENT_T =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_GATTC_REG_CLIENT_T),
    .ui4_num_entries = 0
};

static const RPC_DESC_T t_rpc_decl_BT_GATTC_SCAN_RST_T =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_GATTC_SCAN_RST_T),
    .ui4_num_entries = 0
};

static const RPC_DESC_T t_rpc_decl_BT_GATTC_CONNECT_RST_T =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_GATTC_CONNECT_RST_T),
    .ui4_num_entries = 0
};

static const RPC_DESC_T t_rpc_decl_BT_GATTC_GET_REG_NOTI_RST_T =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_GATTC_GET_REG_NOTI_RST_T),
    .ui4_num_entries = 0
};

static const RPC_DESC_T t_rpc_decl_BT_GATTC_NOTI_PARAMS_T =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_GATTC_NOTI_PARAMS_T),
    .ui4_num_entries = 0
};

static const RPC_DESC_T t_rpc_decl_BT_GATTC_GET_NOTIFY_T =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_GATTC_GET_NOTIFY_T),
    .ui4_num_entries = 0
};

static const RPC_DESC_T t_rpc_decl_BT_GATTC_UNFORMATTED_VALUE_T =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_GATTC_UNFORMATTED_VALUE_T),
    .ui4_num_entries = 0
};

static const RPC_DESC_T t_rpc_decl_BT_GATTC_READ_PARAMS_T =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_GATTC_READ_PARAMS_T),
    .ui4_num_entries = 0
};

static const RPC_DESC_T t_rpc_decl_BT_GATTC_READ_CHAR_RST_T =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_GATTC_READ_CHAR_RST_T),
    .ui4_num_entries = 0
};

static const RPC_DESC_T t_rpc_decl_BT_GATTC_WRITE_CHAR_RST_T =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_GATTC_WRITE_CHAR_RST_T),
    .ui4_num_entries = 0
};

static const RPC_DESC_T t_rpc_decl_BT_GATTC_READ_DESCR_RST_T =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_GATTC_READ_DESCR_RST_T),
    .ui4_num_entries = 0
};

static const RPC_DESC_T t_rpc_decl_BT_GATTC_WRITE_DESCR_RST_T =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_GATTC_WRITE_DESCR_RST_T),
    .ui4_num_entries = 0
};

static const RPC_DESC_T t_rpc_decl_BT_GATTC_GET_REMOTE_RSSI_T =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_GATTC_GET_REMOTE_RSSI_T),
    .ui4_num_entries = 0
};

static const RPC_DESC_T t_rpc_decl_BT_GATTC_SCAN_FILTER_PARAM_T =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_GATTC_SCAN_FILTER_PARAM_T),
    .ui4_num_entries = 0
};

static const RPC_DESC_T t_rpc_decl_BT_GATTC_SCAN_FILTER_STATUS_T =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_GATTC_SCAN_FILTER_STATUS_T),
    .ui4_num_entries = 0
};

static const RPC_DESC_T t_rpc_decl_BT_GATTC_SCAN_FILTER_CFG_T =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_GATTC_SCAN_FILTER_CFG_T),
    .ui4_num_entries = 0
};

static const RPC_DESC_T t_rpc_decl_BT_GATTC_FILT_PARAM_SETUP_T =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_GATTC_FILT_PARAM_SETUP_T),
    .ui4_num_entries = 0
};

static const RPC_DESC_T t_rpc_decl_BT_GATTC_DB_ELEMENT_T =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_GATTC_DB_ELEMENT_T),
    .ui4_num_entries = 0
};

static const RPC_DESC_T t_rpc_decl_BT_GATTC_GET_GATT_DB_T =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_GATTC_GET_GATT_DB_T),
    .ui4_num_entries = 1,
    {
        {
            .e_type          = ARG_TYPE_REF_DESC,
            .pt_desc         = &t_rpc_decl_BT_GATTC_DB_ELEMENT_T,
            .ui4_num_entries = 1,

            {
                .z_offs = offsetof (BT_GATTC_GET_GATT_DB_T, gatt_db_element)
            }
        }
    }
};

static const RPC_DESC_T t_rpc_decl_BT_GATTC_TEST_PARAMS_T =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_GATTC_TEST_PARAMS_T),
    .ui4_num_entries = 2,
    {
        {
            .e_type          = ARG_TYPE_REF_DESC,
            .pt_desc         = NULL,
            .ui4_num_entries = 1,

            {
                .z_offs = offsetof (BT_GATTC_TEST_PARAMS_T, bda1)
            }
        },
        {
            .e_type          = ARG_TYPE_REF_DESC,
            .pt_desc         = NULL,
            .ui4_num_entries = 1,

            {
                .z_offs = offsetof (BT_GATTC_TEST_PARAMS_T, uuid1)
            }
        }
    }
};

static const RPC_DESC_T t_rpc_decl_BT_APP_GATTC_CB_FUNC_T =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_APP_GATTC_CB_FUNC_T),
    .ui4_num_entries = 0
};

static const RPC_DESC_T t_rpc_decl_MTKRPCAPI_BT_APP_GATTC_CB_FUNC_T =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (MTKRPCAPI_BT_APP_GATTC_CB_FUNC_T),
    .ui4_num_entries = 0
};

static const RPC_DESC_T t_rpc_decl_BT_GATTS_REG_SERVER_RST_T =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_GATTS_REG_SERVER_RST_T),
    .ui4_num_entries = 0
};

static const RPC_DESC_T t_rpc_decl_BT_GATTS_CONNECT_RST_T =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_GATTS_CONNECT_RST_T),
    .ui4_num_entries = 0
};

static const RPC_DESC_T t_rpc_decl_BT_GATTS_ADD_SRVC_RST_T =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_GATTS_ADD_SRVC_RST_T),
    .ui4_num_entries = 0
};

static const RPC_DESC_T t_rpc_decl_BT_GATTS_ADD_INCL_RST_T =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_GATTS_ADD_INCL_RST_T),
    .ui4_num_entries = 0
};

static const RPC_DESC_T t_rpc_decl_BT_GATTS_ADD_CHAR_RST_T =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_GATTS_ADD_CHAR_RST_T),
    .ui4_num_entries = 0
};

static const RPC_DESC_T t_rpc_decl_BT_GATTS_ADD_DESCR_RST_T =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_GATTS_ADD_DESCR_RST_T),
    .ui4_num_entries = 0
};

static const RPC_DESC_T t_rpc_decl_BT_GATTS_SRVC_RST_T =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_GATTS_SRVC_RST_T),
    .ui4_num_entries = 0
};

static const RPC_DESC_T t_rpc_decl_BT_GATTS_REQ_READ_RST_T =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_GATTS_REQ_READ_RST_T),
    .ui4_num_entries = 0
};

static const RPC_DESC_T t_rpc_decl_BT_GATTS_REQ_WRITE_RST_T =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_GATTS_REQ_WRITE_RST_T),
    .ui4_num_entries = 0
};

static const RPC_DESC_T t_rpc_decl_BT_GATTS_EXEC_WRITE_RST_T =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_GATTS_EXEC_WRITE_RST_T),
    .ui4_num_entries = 0
};

static const RPC_DESC_T t_rpc_decl_BT_GATTS_SENT_IND_RST_T =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_GATTS_SENT_IND_RST_T),
    .ui4_num_entries = 0
};

static const RPC_DESC_T t_rpc_decl_BT_APP_GATTS_CB_FUNC_T =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (BT_APP_GATTS_CB_FUNC_T),
    .ui4_num_entries = 0
};

static const RPC_DESC_T t_rpc_decl_MTKRPCAPI_BT_APP_GATTS_CB_FUNC_T =
{
    .e_type          = ARG_TYPE_STRUCT,
    .z_size          = sizeof (MTKRPCAPI_BT_APP_GATTS_CB_FUNC_T),
    .ui4_num_entries = 0
};


static const RPC_DESC_T* at_rpc_desc_list [] =
{
    &t_rpc_decl_BLUETOOTH_DEVICE,
    &t_rpc_decl_BT_GATT_ID_T,
    &t_rpc_decl_BT_GATT_SRVC_ID_T,
    &t_rpc_decl_BT_GATTC_REG_CLIENT_T,
    &t_rpc_decl_BT_GATTC_SCAN_RST_T,
    &t_rpc_decl_BT_GATTC_CONNECT_RST_T,
    &t_rpc_decl_BT_GATTC_GET_REG_NOTI_RST_T,
    &t_rpc_decl_BT_GATTC_NOTI_PARAMS_T,
    &t_rpc_decl_BT_GATTC_GET_NOTIFY_T,
    &t_rpc_decl_BT_GATTC_UNFORMATTED_VALUE_T,
    &t_rpc_decl_BT_GATTC_READ_PARAMS_T,
    &t_rpc_decl_BT_GATTC_READ_CHAR_RST_T,
    &t_rpc_decl_BT_GATTC_WRITE_CHAR_RST_T,
    &t_rpc_decl_BT_GATTC_READ_DESCR_RST_T,
    &t_rpc_decl_BT_GATTC_WRITE_DESCR_RST_T,
    &t_rpc_decl_BT_GATTC_GET_REMOTE_RSSI_T,
    &t_rpc_decl_BT_GATTC_SCAN_FILTER_PARAM_T,
    &t_rpc_decl_BT_GATTC_SCAN_FILTER_STATUS_T,
    &t_rpc_decl_BT_GATTC_SCAN_FILTER_CFG_T,
    &t_rpc_decl_BT_GATTC_FILT_PARAM_SETUP_T,
    &t_rpc_decl_BT_GATTC_DB_ELEMENT_T,
    &t_rpc_decl_BT_GATTC_GET_GATT_DB_T,
    &t_rpc_decl_BT_GATTC_TEST_PARAMS_T,
    &t_rpc_decl_BT_APP_GATTC_CB_FUNC_T,
    &t_rpc_decl_MTKRPCAPI_BT_APP_GATTC_CB_FUNC_T,
    &t_rpc_decl_BT_GATTS_REG_SERVER_RST_T,
    &t_rpc_decl_BT_GATTS_CONNECT_RST_T,
    &t_rpc_decl_BT_GATTS_ADD_SRVC_RST_T,
    &t_rpc_decl_BT_GATTS_ADD_INCL_RST_T,
    &t_rpc_decl_BT_GATTS_ADD_CHAR_RST_T,
    &t_rpc_decl_BT_GATTS_ADD_DESCR_RST_T,
    &t_rpc_decl_BT_GATTS_SRVC_RST_T,
    &t_rpc_decl_BT_GATTS_REQ_READ_RST_T,
    &t_rpc_decl_BT_GATTS_REQ_WRITE_RST_T,
    &t_rpc_decl_BT_GATTS_EXEC_WRITE_RST_T,
    &t_rpc_decl_BT_GATTS_SENT_IND_RST_T,
    &t_rpc_decl_BT_APP_GATTS_CB_FUNC_T,
    &t_rpc_decl_MTKRPCAPI_BT_APP_GATTS_CB_FUNC_T
};

EXPORT_SYMBOL const RPC_DESC_T* __rpc_get_gatt_desc__ (UINT32  ui4_idx)
{
  return ((ui4_idx < 38) ? at_rpc_desc_list [ui4_idx] : NULL);
}


