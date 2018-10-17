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

#ifndef __LINUXBT_HID_IF_H__
#define __LINUXBT_HID_IF_H__

//int linuxbt_hid_activate_handler(void);
//int linuxbt_hid_deactivate_handler(void);
int linuxbt_hid_init(void);
int linuxbt_hid_deinit(void);
int linuxbt_hid_connect_int_handler(char *pbt_addr);
int linuxbt_hid_disconnect_handler(char *pbt_addr);
int linuxbt_hid_set_output_report_handler(char *pbt_addr, char *preport_data);
int linuxbt_hid_get_input_report_handler(char *pbt_addr, int reportId, int bufferSize);
int linuxbt_hid_get_feature_report_handler(char *pbt_addr, int reportId, int bufferSize);
int linuxbt_hid_set_feature_report_handler(char *pbt_addr, char *preport_data);
int linuxbt_hid_set_protocol_handler(char *pbt_addr, int protocol_mode);
//int linuxbt_hid_send_control_handler(char *pbt_addr, int pcontrol_mode);
int linuxbt_hid_get_protocol_handler(char *pbt_addr);
int linuxbt_hid_get_output_report_handler(char *pbt_addr, int reportId, int bufferSize);
int linuxbt_hid_set_input_report_handler(char *pbt_addr, char *preport_data);
int linuxbt_hid_virtual_unplug_handler(char *pbt_addr);
int linuxbt_hid_send_data_handler(char *pbt_addr,char *data);
#endif /* __LINUXBT_HID_IF_H__ */
