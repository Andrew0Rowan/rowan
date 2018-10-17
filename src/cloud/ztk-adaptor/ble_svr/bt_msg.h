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

#ifndef __BT_MSG_H__
#define __BT_MSG_H__

#include "u_common.h"
#include "types.h"

enum
{
    BT_APP_GATTC_EVENT = 0,
    BT_APP_GATTS_EVENT,
    BT_APP_GATT_EVENT,
    BT_APP_EVENT_MAX,
};  // event

typedef struct
{
    int          event;
    int          len;    /* data lenght, don't include HDR */
} BT_APP_MSG_HDR;

typedef enum
{
    /* GATTS events */
    BT_APP_GATTS_REG_SERVER = 0,
    BT_APP_GATTS_ADD_SRVC,
    BT_APP_GATTS_CONNECT,
    BT_APP_GATTS_DISCONNECT,
    BT_APP_GATTS_GET_CONNECT_INFO,
    BT_APP_GATTS_ADD_INCL,
    BT_APP_GATTS_ADD_CHAR,
    BT_APP_GATTS_ADD_DESC,
    BT_APP_GATTS_REQ_READ,
    BT_APP_GATTS_REQ_WRITE,
    BT_APP_GATTS_SENT_IND,
    BT_APP_GATTS_START_SRVC,
    BT_APP_GATTS_STOP_SRVC,
    BT_APP_GATTS_DEL_SRVC,
    BT_APP_GATTS_EVENT_MAX,
} BT_APP_GATTS_NOTIFY_EVENT;  // event

typedef enum
{
    /* GATTC events */
    BT_APP_GATTC_REG_CLIENT = 0,
    BT_APP_GATTC_MULTI_ADV_ENABLE,
    BT_APP_GATTC_MULTI_ADV_SETDATA,
    BT_APP_GATTC_EVENT_MAX,
} BT_APP_GATTC_NOTIFY_EVENT;  // event

typedef enum
{
    /* GATT events */
    BT_APP_GATT_START_SERVER = 0,
    BT_APP_GATT_STOP_SERVER = 0,
    BT_APP_GATT_START_SERVER_SUCCESS,
    BT_APP_GATT_START_SERVER_FAIL,
    BT_APP_GATT_EVENT_MAX,
} BT_APP_GATT_NOTIFY_EVENT;  // event

typedef struct
{
    BT_APP_GATT_NOTIFY_EVENT event;
} BT_APP_GATT_MSG_HDR;

typedef struct
{
    int trans_id;
    int attr_handle;
    int conn_id;
} BT_APP_GATTS_WRITE_CB_HDR;

typedef struct
{
    int trans_id;
    int attr_handle;
    int conn_id;
    int offset;
} BT_APP_GATTS_READ_CB_HDR;

typedef struct
{
    BT_APP_GATTS_NOTIFY_EVENT event;
    union
    {
       BT_APP_GATTS_WRITE_CB_HDR       writeCb_msg;
       BT_APP_GATTS_READ_CB_HDR       readCb_msg;
    }data;
} BT_APP_GATTS_MSG_HDR;

typedef struct
{
    BT_APP_GATTC_NOTIFY_EVENT event;
} BT_APP_GATTC_MSG_HDR;

/* union of all data types */
typedef struct
{
    BT_APP_MSG_HDR             hdr;
    union
    {
       BT_APP_GATTC_MSG_HDR       gattc_msg;
       BT_APP_GATTS_MSG_HDR       gatts_msg;
       BT_APP_GATT_MSG_HDR       gatt_msg;
    }data;
} BT_APP_MSG;

typedef struct
{
    long tMsgType;   //IPC_MSG_TYPE
    BT_APP_MSG body;
}BT_APP_IPC_MSG;


BOOL bt_app_send_msg(BT_APP_MSG *msg);
int bt_app_queue_init_new(void);
void bt_app_msg_queue_destroy();


#endif /* __BT_MSG_H__ */

