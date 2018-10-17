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

#ifndef __BLE_GATT_H__
#define __BLE_GATT_H__

#define BT_GATT_MAX_ATTR_LEN 600

#define BT_TRANSPORT_INVALID   0
#define BT_TRANSPORT_BR_EDR    1
#define BT_TRANSPORT_LE        2

#define GATT_TRANSPORT_LE           BT_TRANSPORT_LE
#define GATT_TRANSPORT_BR_EDR       BT_TRANSPORT_BR_EDR
#define GATT_TRANSPORT_LE_BR_EDR    (BT_TRANSPORT_LE|BT_TRANSPORT_BR_EDR)

#define BTM_BLE_ADV_CHNL_37    (0x01 << 0)
#define BTM_BLE_ADV_CHNL_38    (0x01 << 1)
#define BTM_BLE_ADV_CHNL_39    (0x01 << 2)

#define BTA_BLE_ADV_CHNL_37 BTM_BLE_ADV_CHNL_37
#define BTA_BLE_ADV_CHNL_38 BTM_BLE_ADV_CHNL_38
#define BTA_BLE_ADV_CHNL_39 BTM_BLE_ADV_CHNL_39

#define BTA_BLE_ADV_TX_POWER_MIN        0           /* minimum tx power */
#define BTA_BLE_ADV_TX_POWER_LOW        1           /* low tx power     */
#define BTA_BLE_ADV_TX_POWER_MID        2           /* middle tx power  */
#define BTA_BLE_ADV_TX_POWER_UPPER      3           /* upper tx power   */
#define BTA_BLE_ADV_TX_POWER_MAX        4           /* maximum tx power */

#define BTM_BLE_CONNECT_EVT     0x00   /* 0x00-0x04 only used for set advertising
                                            parameter command */
#define BTM_BLE_CONNECT_DIR_EVT 0x01   /* Connectable directed advertising */
#define BTM_BLE_DISCOVER_EVT    0x02  /* Scannable undirected advertising */
#define BTM_BLE_NON_CONNECT_EVT 0x03  /* Non connectable undirected advertising */
#define BTM_BLE_CONNECT_LO_DUTY_DIR_EVT 0x04        /* Connectable low duty
                                                       cycle directed advertising  */
    /* 0x00 - 0x05 can be received on adv event type */
#define BTM_BLE_SCAN_RSP_EVT    0x04
#define BTM_BLE_SCAN_REQ_EVT    0x05
#define BTM_BLE_UNKNOWN_EVT     0xff

#define BTM_BLE_UNKNOWN_EVT     0xff

#define BTA_BLE_CONNECT_EVT         BTM_BLE_CONNECT_EVT     /* Connectable undirected advertising */
#define BTA_BLE_CONNECT_DIR_EVT     BTM_BLE_CONNECT_DIR_EVT /* Connectable directed advertising */
#define BTA_BLE_DISCOVER_EVT        BTM_BLE_DISCOVER_EVT    /* Scannable undirected advertising */
#define BTA_BLE_NON_CONNECT_EVT     BTM_BLE_NON_CONNECT_EVT /* Non connectable undirected advertising */

#define BLE_MAC_lEN 18

typedef enum
{
    BLE_GATT_SERVER_START_IDLE = 0,
    BLE_GATT_SERVER_START_APP_REGED,
    BLE_GATT_SERVER_START_SERVER_REGED,
    BLE_GATT_SERVER_START_SERVICE_ADDED,
    BLE_GATT_SERVER_START_CHAR_ADDED,
    BLE_GATT_SERVER_START_DESC_ADDED,
    BLE_GATT_SERVER_START_SERVER_STARTED,
    BLE_GATT_SERVER_START_ADV_ENABLED,
    BLE_GATT_SERVER_START_STATUS_MAX
} BLE_GATT_SERVER_START_STATUS;

typedef enum
{
    BLE_GATT_SERVER_STOP_IDLE = 0,
    BLE_GATT_SERVER_STOP_STOPPED,
    BLE_GATT_SERVER_STOP_DELETETED,
    BLE_GATT_SERVER_STOP_STATUS_MAX
} BLE_GATT_SERVER_STOP_STATUS;

typedef enum
{
    BLE_GATT_SERVER_START_OP_REG_APP = 1,
    BLE_GATT_SERVER_START_OP_REG_SERVER,
    BLE_GATT_SERVER_START_OP_ADD_SERVICE,
    BLE_GATT_SERVER_START_OP_ADD_CHAR,
    BLE_GATT_SERVER_START_OP_ADD_DESC,
    BLE_GATT_SERVER_START_OP_START_SERVER,
    BLE_GATT_SERVER_START_OP_ENBALE_ADV,
    BLE_GATT_SERVER_START_OP_SET_ADVDATA,
    BLE_GATT_SERVER_START_OP_MAX
} BLE_GATT_SERVER_START_OP;

typedef enum
{
    BLE_GATT_SERVER_STOP_OP_STOP_SERVER = 1,
    BLE_GATT_SERVER_STOP_OP_DELETET_SERVER,
    BLE_GATT_SERVER_STOP_OP_UNREG,
    BLE_GATT_SERVER_STOP_OP_MAX
} BLE_GATT_SERVER_STOP_OP;

typedef enum
{
    BLE_GATT_SERVER_MISC_OP_READ_REQUEST = 1,
    BLE_GATT_SERVER_MISC_OP_WRITE_REQUEST,
    BLE_GATT_SERVER_MISC_OP_GET_CONN_INFO,
    BLE_GATT_SERVER_MISC_OP_MAX
} BLE_GATT_SERVER_MISC_OP;

typedef enum
{
    BLE_GATT_SERVER_START_MSG = 0,
    BLE_GATT_SERVER_STOP_MSG,
    BLE_GATT_SERVER_MISC_MSG,
    BLE_GATT_SERVER_MAX_MSG
} BLE_GATT_SERVER_MSG_TYPE;

typedef struct
{
    CHAR app_uuid[BT_LEN_UUID_LEN];
    CHAR server_uuid[BT_LEN_UUID_LEN];
    CHAR service_uuid[BT_LEN_UUID_LEN];
    CHAR char_uuid[BT_LEN_UUID_LEN];
    CHAR desc_uuid[BT_LEN_UUID_LEN];
    CHAR *pSeviContent;
    UINT32 service_content_len;
    UINT32 service_content_pos;
    CHAR *pCharContent;
    UINT32 char_content_len;
    UINT32 char_content_pos;
    CHAR *pDescContent;
    UINT32 desc_content_len;
    UINT32 desc_content_pos;
    VOID *bleSvrStartTimer;
    VOID *bleSvrStopTimer;
    UINT32 bleSvrStartStatus;
    UINT32 bleSvrStopStatus;
    INT32 bleSvrIsOpened;
}BLE_GATTS_INFO;

typedef struct
{
    INT32 server_if;
    INT32 client_if;
    INT32 connId;
    INT32 servHandle;
    INT32 charHandle;
    INT32 descHandle;
    INT32 transId;
    BOOL isConnected;
    CHAR btAddr[BLE_MAC_lEN];
}BLE_GATTS_MW_ABOUT_INFO;

typedef struct
{
    INT32 conn_id;
    INT32 trans_id;
    CHAR btaddr[BLE_MAC_lEN];
    INT32 attr_handle;
    INT32 offset;
    UINT8 is_long;
}BLE_SERVER_READ_REQUEST;

typedef struct
{
    INT32 conn_id;
    INT32 trans_id;
    CHAR btaddr[BLE_MAC_lEN];
    INT32 attr_handle;
    INT32 offset;
    INT32 length;
    UINT8 need_rsp;
    UINT8 is_prep;
    UINT8 value[BT_GATT_MAX_ATTR_LEN];
}BLE_SERVER_WRITE_REQUEST;

typedef struct
{
    CHAR *content_str;
    UINT32 length;
    UINT32 *pos;
}BLE_SERVER_CONTENT_GET;

extern BLE_GATTS_MW_ABOUT_INFO gGattsMwInfo;

INT32 ble_gatt_server_init(VOID);
INT32 ble_gatt_server_deinit(VOID);
INT32 ble_gatt_server_get_status(VOID);
VOID *ble_gatt_server_get_start_timer(VOID);
VOID *ble_gatt_server_get_stop_timer(VOID);
CHAR *ble_gatt_server_get_char_handle_mem(VOID);
UINT32 *ble_gatt_server_get_char_handle_mem_len(VOID);

#endif /* __BLE_GATT_H__ */

