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


/* FILE NAME:  u_bt_mw_common.h
 * AUTHOR: Hongliang Hu
 * PURPOSE:
 *      It provides bluetooth common structure to APP.
 * NOTES:
 */


#ifndef _U_BT_MW_COMMON_H_
#define _U_BT_MW_COMMON_H_

/*-----------------------------------------------------------------------------
                    include files
-----------------------------------------------------------------------------*/
#include "u_bt_mw_types.h"

/*-----------------------------------------------------------------------------
                    macros, defines, typedefs, enums
 ----------------------------------------------------------------------------*/

#define MAX_BDADDR_LEN                   ((UINT32)     18)
#define MAX_NAME_LEN                     ((UINT32)    249)
//#define MAX_INQU_NUM                     ((UINT32)     32)
//#define MAX_STR_LEN                      ((UINT32)    128)   // only use in gatt
//#define             MAX_DEV_NUM                       ((UINT32)     10)

typedef enum
{
    BT_DEBUG_COMM  = 0,
    BT_DEBUG_GAP   = 1,
    BT_DEBUG_A2DP  = 2,
    BT_DEBUG_AVRCP = 3,
    BT_DEBUG_HID   = 4,
    BT_DEBUG_SPP   = 5,
    BT_DEBUG_GATT  = 6,
    BT_DEBUG_HFP   = 7,
    BT_DEBUG_PB    = 8,
    BT_DEBUG_UPL   = 9,
    BT_DEBUG_MAX
}BT_DEBUG_LAYER_NAME_T;

typedef enum
{
    BT_SUCCESS = 0,
    BT_ERR_STATUS_FAIL = -1,
    BT_ERR_STATUS_NOT_READY = -2,
    BT_ERR_STATUS_NOMEM = -3,
    BT_ERR_STATUS_BUSY = -4,
    BT_ERR_STATUS_DONE = -5,        /* request already completed */
    BT_ERR_STATUS_UNSUPPORTED = -6,
    BT_ERR_STATUS_PARM_INVALID = -7,
    BT_ERR_STATUS_UNHANDLED = -8,
    BT_ERR_STATUS_AUTH_FAILURE = -9,
    BT_ERR_STATUS_RMT_DEV_DOWN = -10,
    BT_ERR_STATUS_INVALID_PARM_NUMS = -11,
    BT_ERR_STATUS_NULL_POINTER = -12,
    BT_ERR_STATUS_END = -100
} BT_ERR_STATUS_T;

typedef enum
{
   BT_DEVICE_LOCAL ,
   BT_DEVICE_SCAN ,
   BT_DEVICE_BONDED ,
}BTMW_DEVICE_KIND;

typedef struct _BLUETOOTH_DEVICE
{
    CHAR            bdAddr[MAX_BDADDR_LEN];/* Bluetooth Address */
    CHAR            name[MAX_NAME_LEN];/* Name of device */
    UINT32          cod;
    INT16           rssi;
    UINT32          devicetype;
}BLUETOOTH_DEVICE;

#endif /*  _U_BT_MW_COMMON_H_ */

