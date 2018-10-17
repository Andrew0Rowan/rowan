/* This header file was automatically created by the */
/* tool 'MTK RPC Description tool', 'Version 1.10' on 'Tue Dec 26 15:05:37 2017'. */
/* Do NOT modify this header file. */

#ifndef _MTK_BT_SERVICE_GATT_IPCRPC_STRUCT__H_
#define _MTK_BT_SERVICE_GATT_IPCRPC_STRUCT__H_




/* Start of header pre-amble file 'preamble_file.h'. */

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

#include "u_rpc.h"


/* End of header pre-amble file 'preamble_file.h'. */


#define RPC_DESC_BLUETOOTH_DEVICE  (__rpc_get_gatt_desc__ (0))


#define RPC_DESC_BT_GATT_ID_T  (__rpc_get_gatt_desc__ (1))


#define RPC_DESC_BT_GATT_SRVC_ID_T  (__rpc_get_gatt_desc__ (2))


#define RPC_DESC_BT_GATTC_REG_CLIENT_T  (__rpc_get_gatt_desc__ (3))


#define RPC_DESC_BT_GATTC_SCAN_RST_T  (__rpc_get_gatt_desc__ (4))


#define RPC_DESC_BT_GATTC_CONNECT_RST_T  (__rpc_get_gatt_desc__ (5))


#define RPC_DESC_BT_GATTC_GET_REG_NOTI_RST_T  (__rpc_get_gatt_desc__ (6))


#define RPC_DESC_BT_GATTC_NOTI_PARAMS_T  (__rpc_get_gatt_desc__ (7))


#define RPC_DESC_BT_GATTC_GET_NOTIFY_T  (__rpc_get_gatt_desc__ (8))


#define RPC_DESC_BT_GATTC_UNFORMATTED_VALUE_T  (__rpc_get_gatt_desc__ (9))


#define RPC_DESC_BT_GATTC_READ_PARAMS_T  (__rpc_get_gatt_desc__ (10))


#define RPC_DESC_BT_GATTC_READ_CHAR_RST_T  (__rpc_get_gatt_desc__ (11))


#define RPC_DESC_BT_GATTC_WRITE_CHAR_RST_T  (__rpc_get_gatt_desc__ (12))


#define RPC_DESC_BT_GATTC_READ_DESCR_RST_T  (__rpc_get_gatt_desc__ (13))


#define RPC_DESC_BT_GATTC_WRITE_DESCR_RST_T  (__rpc_get_gatt_desc__ (14))


#define RPC_DESC_BT_GATTC_GET_REMOTE_RSSI_T  (__rpc_get_gatt_desc__ (15))


#define RPC_DESC_BT_GATTC_SCAN_FILTER_PARAM_T  (__rpc_get_gatt_desc__ (16))


#define RPC_DESC_BT_GATTC_SCAN_FILTER_STATUS_T  (__rpc_get_gatt_desc__ (17))


#define RPC_DESC_BT_GATTC_SCAN_FILTER_CFG_T  (__rpc_get_gatt_desc__ (18))


#define RPC_DESC_BT_GATTC_FILT_PARAM_SETUP_T  (__rpc_get_gatt_desc__ (19))


#define RPC_DESC_BT_GATTC_DB_ELEMENT_T  (__rpc_get_gatt_desc__ (20))


#define RPC_DESC_BT_GATTC_GET_GATT_DB_T  (__rpc_get_gatt_desc__ (21))


#define RPC_DESC_BT_GATTC_TEST_PARAMS_T  (__rpc_get_gatt_desc__ (22))


#define RPC_DESC_BT_APP_GATTC_CB_FUNC_T  (__rpc_get_gatt_desc__ (23))


#define RPC_DESC_MTKRPCAPI_BT_APP_GATTC_CB_FUNC_T  (__rpc_get_gatt_desc__ (24))


#define RPC_DESC_BT_GATTS_REG_SERVER_RST_T  (__rpc_get_gatt_desc__ (25))


#define RPC_DESC_BT_GATTS_CONNECT_RST_T  (__rpc_get_gatt_desc__ (26))


#define RPC_DESC_BT_GATTS_ADD_SRVC_RST_T  (__rpc_get_gatt_desc__ (27))


#define RPC_DESC_BT_GATTS_ADD_INCL_RST_T  (__rpc_get_gatt_desc__ (28))


#define RPC_DESC_BT_GATTS_ADD_CHAR_RST_T  (__rpc_get_gatt_desc__ (29))


#define RPC_DESC_BT_GATTS_ADD_DESCR_RST_T  (__rpc_get_gatt_desc__ (30))


#define RPC_DESC_BT_GATTS_SRVC_RST_T  (__rpc_get_gatt_desc__ (31))


#define RPC_DESC_BT_GATTS_REQ_READ_RST_T  (__rpc_get_gatt_desc__ (32))


#define RPC_DESC_BT_GATTS_REQ_WRITE_RST_T  (__rpc_get_gatt_desc__ (33))


#define RPC_DESC_BT_GATTS_EXEC_WRITE_RST_T  (__rpc_get_gatt_desc__ (34))


#define RPC_DESC_BT_GATTS_SENT_IND_RST_T  (__rpc_get_gatt_desc__ (35))


#define RPC_DESC_BT_APP_GATTS_CB_FUNC_T  (__rpc_get_gatt_desc__ (36))


#define RPC_DESC_MTKRPCAPI_BT_APP_GATTS_CB_FUNC_T  (__rpc_get_gatt_desc__ (37))



extern const RPC_DESC_T* __rpc_get_gatt_desc__ (UINT32  ui4_idx);


#endif

