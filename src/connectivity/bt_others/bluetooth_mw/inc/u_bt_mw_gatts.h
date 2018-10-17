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


/* FILE NAME:  u_bt_mw_gatts.h
 * AUTHOR: Xuemei Yang
 * PURPOSE:
 *      It provides GATTS structure to APP.
 * NOTES:
 */


#ifndef _U_BT_MW_GATTS_H_
#define _U_BT_MW_GATTS_H_

/*-----------------------------------------------------------------------------
                    include files
-----------------------------------------------------------------------------*/
#include "u_bt_mw_common.h"
#include "u_bt_mw_gatt.h"

/*GATTS struct*/
typedef enum
{
    BT_GATTS_REGISTER_SERVER = 0,
    BT_GATTS_CONNECT,
    BT_GATTS_DISCONNECT,
    BT_GATTS_GET_RSSI_DONE,

    BT_GATTS_EVENT_MAX
}BT_GATTS_EVENT_T;

typedef enum
{
    BT_GATTS_START_SRVC = 0,
    BT_GATTS_STOP_SRVC,
    BT_GATTS_DEL_SRVC,

    BT_GATTS_SRVC_OP_MAX
}BT_GATTS_SRVC_OP_TYPE_T;

typedef struct
{
    INT32 server_if;
    CHAR app_uuid[BT_GATT_MAX_UUID_LEN];
} BT_GATTS_REG_SERVER_RST_T;

typedef struct
{
    INT32 conn_id;
    INT32 server_if;
    CHAR btaddr[MAX_BDADDR_LEN];
} BT_GATTS_CONNECT_RST_T;

typedef struct
{
    INT32 server_if;
    BT_GATT_SRVC_ID_T srvc_id;
    INT32 srvc_handle;
} BT_GATTS_ADD_SRVC_RST_T;

typedef struct
{
    INT32 server_if;
    INT32 srvc_handle;
    INT32 incl_srvc_handle;
} BT_GATTS_ADD_INCL_RST_T;

typedef struct
{
    INT32 server_if;
    CHAR uuid[BT_GATT_MAX_UUID_LEN];
    INT32 srvc_handle;
    INT32 char_handle;
} BT_GATTS_ADD_CHAR_RST_T ;

typedef struct
{
    INT32 server_if;
    CHAR uuid[BT_GATT_MAX_UUID_LEN];
    INT32 srvc_handle;
    INT32 descr_handle;
} BT_GATTS_ADD_DESCR_RST_T;

typedef struct
{
    INT32 server_if;
    INT32 srvc_handle;
} BT_GATTS_SRVC_RST_T;

typedef struct
{
    INT32 conn_id;
    INT32 trans_id;
    CHAR btaddr[MAX_BDADDR_LEN];
    INT32 attr_handle;
    INT32 offset;
    UINT8 is_long;
} BT_GATTS_REQ_READ_RST_T;

typedef struct
{
    INT32 conn_id;
    INT32 trans_id;
    CHAR btaddr[MAX_BDADDR_LEN];
    INT32 attr_handle;
    INT32 offset;
    INT32 length;
    UINT8 need_rsp;
    UINT8 is_prep;
    UINT8 value[BT_GATT_MAX_ATTR_LEN];
} BT_GATTS_REQ_WRITE_RST_T;

typedef struct
{
    INT32 conn_id;
    INT32 trans_id;
    CHAR btaddr[MAX_BDADDR_LEN];
    INT32 exec_write;
} BT_GATTS_EXEC_WRITE_RST_T;

typedef struct
{
    INT32 conn_id;
    INT32 status;
}BT_GATTS_SENT_IND_RST_T;

typedef VOID (*BtAppGATTSEventCbk)(BT_GATTS_EVENT_T bt_gatts_event);
typedef VOID (*BtAppGATTSRegServerCbk)(BT_GATTS_REG_SERVER_RST_T *bt_gatts_reg_server);
typedef VOID (*BtAppGATTSAddSrvcCbk)(BT_GATTS_ADD_SRVC_RST_T *bt_gatts_add_srvc);
typedef VOID (*BtAppGATTSAddInclCbk)(BT_GATTS_ADD_INCL_RST_T *bt_gatts_add_incl);
typedef VOID (*BtAppGATTSAddCharCbk)(BT_GATTS_ADD_CHAR_RST_T *bt_gatts_add_char);
typedef VOID (*BtAppGATTSAddDescCbk)(BT_GATTS_ADD_DESCR_RST_T *bt_gatts_add_desc);
typedef VOID (*BtAppGATTSOpSrvcCbk)(BT_GATTS_SRVC_OP_TYPE_T op_type, BT_GATTS_SRVC_RST_T *bt_gatts_srvc);
typedef VOID (*BtAppGATTSReqReadCbk)(BT_GATTS_REQ_READ_RST_T *bt_gatts_read);
typedef VOID (*BtAppGATTSReqWriteCbk)(BT_GATTS_REQ_WRITE_RST_T *bt_gatts_write);
typedef VOID (*BtAppGATTSIndSentCbk)(INT32 conn_id, INT32 status);

typedef struct _BT_APP_GATTS_CB_FUNC
{
    BtAppGATTSEventCbk bt_gatts_event_cb;
    BtAppGATTSRegServerCbk bt_gatts_reg_server_cb;
    BtAppGATTSAddSrvcCbk bt_gatts_add_srvc_cb;
    BtAppGATTSAddInclCbk bt_gatts_add_incl_cb;
    BtAppGATTSAddCharCbk bt_gatts_add_char_cb;
    BtAppGATTSAddDescCbk bt_gatts_add_desc_cb;
    BtAppGATTSOpSrvcCbk bt_gatts_op_srvc_cb;
    BtAppGATTSReqReadCbk bt_gatts_req_read_cb;
    BtAppGATTSReqWriteCbk bt_gatts_req_write_cb;
    BtAppGATTSIndSentCbk bt_gatts_ind_sent_cb;

}BT_APP_GATTS_CB_FUNC_T;

#endif /*  _U_BT_MW_GATTS_H_ */




