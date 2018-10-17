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


/* FILE NAME:
 * AUTHOR:
 * PURPOSE:
 *      It provides bluetooth common structure to MSG.
 * NOTES:
 */

#ifndef __BT_MW_MESSAGE_QUEUE_H__
#define __BT_MW_MESSAGE_QUEUE_H__

#include "bluetooth.h"
#include "bt_mw_common.h"
#include "u_bt_mw_gap.h"
#include "bt_mw_a2dp_common.h"
#include "bt_mw_avrcp.h"
#include "bt_mw_hidh.h"
#include "bt_mw_spp.h"
#include "bt_mw_hfclient.h"

enum
{
    /* device manager local device API events */
    BTMW_GAP_STATE_EVT = BTMW_EVT_START(BTWM_ID_GAP),
    BTMW_GAP_DEVICE_INFO_EVT,

    BTMW_GAP_MAX_EVT,
};  // event

typedef struct
{
    UINT32          event;
    UINT32          len;    /* data lenght, don't include HDR */
} BTMW_HDR;

/* union of all data types */
typedef struct
{
    BTMW_HDR              hdr;
    union
    {
        tBTMW_GAP_STATE       gap_state;
        tBTMW_GAP_DEVICE_INFO device_info;
        tBT_MW_A2DP_MSG       a2dp_msg;
        BT_A2DP_EVENT_PARAM   a2dp_event;
        tBT_MW_AVRCP_MSG      avrcp_msg;
        BT_AVRCP_EVENT_PARAM  avrcp_event;
        tBT_MW_HIDH_MSG       hidh_msg;
        BT_SPP_CBK_STRUCT     spp_msg;
        tBT_MW_HFCLIENT_MSG   hfclient_msg;
        BT_HFCLIENT_EVENT_PARAM hfclient_event;
    }data;
} tBTMW_MSG;

typedef struct
{
    long tMsgType;   //IPC_MSG_TYPE
    tBTMW_MSG body;
}tBTMW_MSG_T;

typedef VOID (tBTMW_EVENT_HDR)(tBTMW_MSG *p_msg);

void linuxbt_hdl_register(UINT8 id, tBTMW_EVENT_HDR *p_reg);
INT32 linuxbt_send_msg(tBTMW_MSG* msg);
INT32 linuxbt_msg_queue_init_new(VOID);
void bt_mw_nty_hdl_register(UINT8 id, tBTMW_EVENT_HDR *p_reg);
INT32 bt_mw_nty_send_msg(tBTMW_MSG* msg);
INT32 bt_mw_nty_queue_init_new(VOID);


#endif/*__BT_MW_MESSAGE_QUEUE_H__*/

