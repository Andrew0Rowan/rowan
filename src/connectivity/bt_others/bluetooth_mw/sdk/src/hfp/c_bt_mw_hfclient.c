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

 
#include "bt_mw_hfclient.h"

EXPORT_SYMBOL INT32 c_btm_bt_hfclient_enable(VOID)
{
    return bluetooth_hfclient_enable();
}

EXPORT_SYMBOL INT32 c_btm_bt_hfclient_disable(VOID)
{
    return bluetooth_hfclient_disable();
}

EXPORT_SYMBOL INT32 c_btm_bt_hfclient_connect(CHAR *bt_addr)
{
    return bluetooth_hfclient_connect(bt_addr);
}

EXPORT_SYMBOL INT32 c_btm_bt_hfclient_disconnect(CHAR *bt_addr)
{
    return bluetooth_hfclient_disconnect(bt_addr);
}

EXPORT_SYMBOL INT32 c_btm_bt_hfclient_connect_audio(CHAR *bt_addr)
{
    return bluetooth_hfclient_connect_audio(bt_addr);
}

EXPORT_SYMBOL INT32 c_btm_bt_hfclient_disconnect_audio(CHAR *bt_addr)
{
    return bluetooth_hfclient_disconnect_audio(bt_addr);
}

EXPORT_SYMBOL INT32 c_btm_bt_hfclient_start_voice_recognition(VOID)
{
    return bluetooth_hfclient_start_voice_recognition();
}

EXPORT_SYMBOL INT32 c_btm_bt_hfclient_stop_voice_recognition(VOID)
{
    return bluetooth_hfclient_stop_voice_recognition();
}

EXPORT_SYMBOL INT32 c_btm_bt_hfclient_volume_control(BT_HFCLIENT_VOLUME_TYPE_T type, INT32 volume)
{
    return bluetooth_hfclient_volume_control(type, volume);
}

EXPORT_SYMBOL INT32 c_btm_bt_hfclient_dial(const CHAR *number)
{
    return bluetooth_hfclient_dial(number);
}

EXPORT_SYMBOL INT32 c_btm_bt_hfclient_dial_memory(INT32 location)
{
    return bluetooth_hfclient_dial_memory(location);
}

EXPORT_SYMBOL INT32 c_btm_bt_hfclient_handle_call_action(BT_HFCLIENT_CALL_ACTION_T action, INT32 idx)
{
    return bluetooth_hfclient_handle_call_action(action, idx);
}

EXPORT_SYMBOL INT32 c_btm_bt_hfclient_query_current_calls(VOID)
{
    return bluetooth_hfclient_query_current_calls();
}

EXPORT_SYMBOL INT32 c_btm_bt_hfclient_query_current_operator_name(VOID)
{
    return bluetooth_hfclient_query_current_operator_name();
}

EXPORT_SYMBOL INT32 c_btm_bt_hfclient_retrieve_subscriber_info(VOID)
{
    return bluetooth_hfclient_retrieve_subscriber_info();
}

EXPORT_SYMBOL INT32 c_btm_bt_hfclient_send_dtmf(CHAR code)
{
    return bluetooth_hfclient_send_dtmf(code);
}

EXPORT_SYMBOL INT32 c_btm_bt_hfclient_request_last_voice_tag_number(VOID)
{
    return bluetooth_hfclient_request_last_voice_tag_number();
}

EXPORT_SYMBOL INT32 c_btm_bt_hfclient_send_at_cmd(INT32 cmd, INT32 val1, INT32 val2, const CHAR *arg)
{
    return bluetooth_hfclient_send_at_cmd(cmd, val1, val2, arg);
}

EXPORT_SYMBOL INT32 c_btm_bt_hfclient_read_pb_entries(VOID)
{
#if defined(MTK_LINUX_HFP_PHONEBOOK) && (MTK_LINUX_HFP_PHONEBOOK == TRUE)
    return bluetooth_hfclient_read_pb_entries();
#else
    return -1;
#endif
}

EXPORT_SYMBOL INT32 c_btm_bt_hfclient_register_callback(BT_HFCLIENT_EVENT_HANDLE_CB hfclient_handle)
{
    return bluetooth_hfclient_register_callback(hfclient_handle);
}
