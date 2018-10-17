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

#ifndef __ADAPTOR_H__
#define __ADAPTOR_H__

#ifdef  __cplusplus
extern "C" {
#endif

#define ADAPTOR_OK 0
#define ADAPTOR_FAIL -1

typedef void (*BspReportEventCbk)(int event, void *data, int len);

typedef struct _ADAPTOR_CB_FUNC
{
    BspReportEventCbk adaptor_report_event_call_back;
}ADAPTOR_CB_FUNC_T;

/* input event */
enum {
    // for speaker volume control
    BSP_KEY_VOLUME_UP = 0,
    BSP_KEY_VOLUME_DOWN,
    BSP_KEY_VOLUME_MUTE,
    BSP_KEY_MICMUTE,
    BSP_KEY_MICUNMUTE,

    /* tap to talk and hold to talk*/
    BSP_KEY_TAP_TO_TALK,   // TAP to TALK
    BSP_KEY_HOLD_TO_TALK, 

    /* enter/exit AP connect */
    BSP_KEY_ENTER_AP,
    BSP_KEY_EXIT_AP,

    //ble
    BSP_BLE_CLIENT_CONNECT,
    BSP_BLE_CLIENT_DISCONNECT,
    BSP_BLE_SERVER_RECV,

    //ntp
    BSP_NTP_SERVICE_START,

    BSP_EVENT_MAX = 0xff
};

int adaptor_init(ADAPTOR_CB_FUNC_T *func);

#ifdef  __cplusplus
}
#endif

#endif /* __ADAPTOR_H__ */

