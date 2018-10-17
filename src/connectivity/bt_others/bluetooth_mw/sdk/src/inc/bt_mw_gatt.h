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


/* FILE NAME:  bt_mw_gatt.h
 * AUTHOR: Xuemei Yang
 * PURPOSE:
 *      It provides GATT common operation interface to MW.
 * NOTES:
 */


#ifndef __BT_MW_GATT_H__
#define __BT_MW_GATT_H__

#include "u_bt_mw_types.h"
#include "bt_mw_gattc.h"
#include "bt_mw_gatts.h"
#include "bt_mw_common.h"

enum
{
    BTMW_GATTC_EVENT = BTMW_EVT_START(BTWM_ID_GATT),
    BTMW_GATTS_EVENT,
    BTMW_GATT_MAX_EVT,
};  // event

typedef struct
{
    UINT32          event;
    UINT32          len;    /* data lenght, don't include HDR */
} BTMW_GATT_HDR;

/* union of all data types */
typedef struct
{
    BTMW_GATT_HDR             hdr;
    union
    {
       tBT_MW_GATTC_MSG       gattc_msg;
       tBT_MW_GATTS_MSG       gatts_msg;
    }data;
} tBTMW_GATT_MSG;

typedef struct
{
    long tMsgType;   //IPC_MSG_TYPE
    tBTMW_GATT_MSG body;
}tBTMW_GATT_MSG_T;

typedef VOID (tBTMW_GATT_EVENT_HDR)(tBTMW_GATT_MSG *p_msg);

/**
 * FUNCTION NAME: bluetooth_gatt_init
 * PURPOSE:
 *      The function is used to register init function to gap profile.
 * INPUT:
 *      None
 * OUTPUT:
 *      None
 * RETURN:
 *      BT_SUCCESS                    -- Operate success.
 * NOTES:
 *      None
 */
INT32 bluetooth_gatt_init();

/**
 * FUNCTION NAME: bt_mw_gatt_nty_send_msg
 * PURPOSE:
 *      The function is used to send msg to message queue.
 * INPUT:
 *      None
 * OUTPUT:
 *      None
 * RETURN:
 *      BT_SUCCESS                    -- Operate success.
 * NOTES:
 *      None
 */
INT32 bt_mw_gatt_nty_send_msg(tBTMW_GATT_MSG* msg);

/**
 * FUNCTION NAME: bt_gatt_deinit_profile
 * PURPOSE:
 *      The function is used to deinit gatt profile.
 * INPUT:
 *      None
 * OUTPUT:
 *      None
 * RETURN:
 *      BT_SUCCESS                    -- Operate success.
 * NOTES:
 *      None
 */
INT32 bt_gatt_deinit_profile();

/**
 * FUNCTION NAME: bt_gatt_init_profile
 * PURPOSE:
 *      The function is used to init gatt profile.
 * INPUT:
 *      None
 * OUTPUT:
 *      None
 * RETURN:
 *      BT_SUCCESS                    -- Operate success.
 * NOTES:
 *      None
 */
INT32 bt_gatt_init_profile();




#endif/* __BT_MW_GATT_H__ */
