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

#ifndef __LINUXBT_HFCLIENT_IF_H__
#define __LINUXBT_HFCLIENT_IF_H__

#include "bluetooth.h"
#include "bt_mw_common.h"
#include "bt_hf_client.h"

#if defined(MTK_LINUX_HFP) && (MTK_LINUX_HFP == TRUE)
int linuxbt_hfclient_enable(void);
int linuxbt_hfclient_disable(void);
#endif
int linuxbt_hfclient_init(void);
int linuxbt_hfclient_deinit(void);
int linuxbt_hfclient_connect(char *bt_addr);
int linuxbt_hfclient_disconnect(char *bt_addr);
int linuxbt_hfclient_connect_audio(char *bt_addr);
int linuxbt_hfclient_disconnect_audio(char *bt_addr);
int linuxbt_hfclient_start_voice_recognition(void);
int linuxbt_hfclient_stop_voice_recognition(void);
int linuxbt_hfclient_volume_control(bthf_client_volume_type_t type, int volume);
int linuxbt_hfclient_dial(const char *number);
int linuxbt_hfclient_dial_memory(int location);
int linuxbt_hfclient_handle_call_action(bthf_client_call_action_t action, int idx);
int linuxbt_hfclient_query_current_calls(void);
int linuxbt_hfclient_query_current_operator_name(void);
int linuxbt_hfclient_retrieve_subscriber_info(void);
int linuxbt_hfclient_send_dtmf(char code);
int linuxbt_hfclient_request_last_voice_tag_number(void);
int linuxbt_hfclient_send_at_cmd(int cmd, int val1, int val2, const char *arg);
#if defined(MTK_LINUX_HFP_PHONEBOOK) && (MTK_LINUX_HFP_PHONEBOOK == TRUE)
int linuxbt_hfclient_select_pb_storage();
int linuxbt_hfclient_set_pb_storage(UINT8 storage_idx);
int linuxbt_hfclient_set_charset(const char* charset);
int linuxbt_hfclient_test_pb_entry(void);
int linuxbt_hfclient_read_pb_entry(UINT16 idx_min, UINT16 idx_max);
#endif

#endif /* __LINUXBT_HFCLIENT_IF_H__ */
