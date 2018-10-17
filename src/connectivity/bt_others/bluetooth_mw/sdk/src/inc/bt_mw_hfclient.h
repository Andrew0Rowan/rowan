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

#ifndef _BT_MW_HFCLIENT_H_
#define _BT_MW_HFCLIENT_H_

#include "bt_mw_common.h"
#include "u_bt_mw_hfclient.h"

/*****************************************************************************
**  Callback Event used in tBTMW_MSG.BTMW_HDR.event
*****************************************************************************/
enum
{
    BTMW_HFCLIENT_CONNECTION_CB_EVT = BTMW_EVT_START(BTWM_ID_HFP),  /*HFP Connection status*/
    BTMW_HFCLIENT_AUDIO_CONNECTION_CB_EVT,      /*HFP audio connection status*/
    BTMW_HFCLIENT_BVRA_CB_EVT,                  /*AG changed voice recognition setting*/
    BTMW_HFCLIENT_IND_SERVICE_CB_EVT,           /*network status*/
    BTMW_HFCLIENT_IND_ROAM_CB_EVT,              /*network roaming status*/
    BTMW_HFCLIENT_IND_SIGNAL_CB_EVT,            /*network signal strength*/
    BTMW_HFCLIENT_IND_BATTCH_CB_EVT,            /*battery level*/
    BTMW_HFCLIENT_COPS_CB_EVT,                  /*current operator name*/
    BTMW_HFCLIENT_IND_CALL_CB_EVT,              /*call*/
    BTMW_HFCLIENT_IND_CALLSETUP_CB_EVT,         /*callsetup*/
    BTMW_HFCLIENT_IND_CALLHELD_CB_EVT,          /*callheld*/
    BTMW_HFCLIENT_BTRH_CB_EVT,                  /*bluetooth response and hold event*/
    BTMW_HFCLIENT_CLIP_CB_EVT,                  /*Calling line identification event*/
    BTMW_HFCLIENT_CCWA_CB_EVT,                  /*Call waiting notification*/
    BTMW_HFCLIENT_CLCC_CB_EVT,                  /*current call event*/
    BTMW_HFCLIENT_VGM_VGS_CB_EVT,               /*volume change*/
    BTMW_HFCLIENT_CMD_COMPLETE_CB_EVT,          /*command complete*/
    BTMW_HFCLIENT_CNUM_CB_EVT,                  /*subscriber information event*/
    BTMW_HFCLIENT_BSIR_CB_EVT,                  /*in-band ring tone setting changed event*/
    BTMW_HFCLIENT_BINP_CB_EVT,                  /*last voice tag number*/
    BTMW_HFCLIENT_RING_IND_CB_EVT,              /*HF Client ring indication */
    BTMW_HFCLIENT_ABILITY_CB_EVT,               /*HFP Interface enable/disable event*/
#if defined(MTK_LINUX_HFP_PHONEBOOK) && (MTK_LINUX_HFP_PHONEBOOK == TRUE)
    BTMW_HFCLIENT_CPBS_CB_EVT,                  /*BTIF event to BTMW*/
    BTMW_HFCLIENT_CPBR_COUNT_CB_EVT,            /*BTIF event to BTMW*/
    BTMW_HFCLIENT_CPBR_ENTRY_CB_EVT,            /*BTIF event to BTMW*/
    BTMW_HFCLIENT_CPBR_COMPLETE_CB_EVT,         /*BTIF event to BTMW*/
    BTMW_HFCLIENT_CPBR_ENTRY_EVT,               /*BTMW event itself*/
    BTMW_HFCLIENT_CPBR_READY_EVT,               /*BTMW event itself*/
    BTMW_HFCLIENT_CPBR_DONE_EVT,                /*BTMW event itself*/
#endif
};

typedef struct
{
    BT_HFCLIENT_CB_DATA    data;
}tBT_MW_HFCLIENT_MSG;

INT32 bluetooth_hfclient_enable(VOID);
INT32 bluetooth_hfclient_disable(VOID);
INT32 bluetooth_hfclient_connect(CHAR *bt_addr);
INT32 bluetooth_hfclient_disconnect(CHAR *bt_addr);
INT32 bluetooth_hfclient_connect_audio(CHAR *bt_addr);
INT32 bluetooth_hfclient_disconnect_audio(CHAR *bt_addr);
INT32 bluetooth_hfclient_start_voice_recognition(VOID);
INT32 bluetooth_hfclient_stop_voice_recognition(VOID);
INT32 bluetooth_hfclient_volume_control(BT_HFCLIENT_VOLUME_TYPE_T type, INT32 volume);
INT32 bluetooth_hfclient_dial(const CHAR *number);
INT32 bluetooth_hfclient_dial_memory(INT32 location);
INT32 bluetooth_hfclient_handle_call_action(BT_HFCLIENT_CALL_ACTION_T action, INT32 idx);
INT32 bluetooth_hfclient_query_current_calls(VOID);
INT32 bluetooth_hfclient_query_current_operator_name(VOID);
INT32 bluetooth_hfclient_retrieve_subscriber_info(VOID);
INT32 bluetooth_hfclient_send_dtmf(CHAR code);
INT32 bluetooth_hfclient_request_last_voice_tag_number(VOID);
INT32 bluetooth_hfclient_send_at_cmd(INT32 cmd, INT32 val1, INT32 val2, const CHAR *arg);
#if defined(MTK_LINUX_HFP_PHONEBOOK) && (MTK_LINUX_HFP_PHONEBOOK == TRUE)
INT32 bluetooth_hfclient_read_pb_entries(VOID);
#endif
INT32 bluetooth_hfclient_register_callback(BT_HFCLIENT_EVENT_HANDLE_CB hfclient_handle);

#endif /* _BT_MW_HFCLIENT_H_ */
