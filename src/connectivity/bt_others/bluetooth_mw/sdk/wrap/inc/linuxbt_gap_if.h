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

#ifndef __LINUXBT_GAP_IF_H__
#define __LINUXBT_GAP_IF_H__


int linuxbt_gap_init(void);
int linuxbt_gap_deinit(void);
int linuxbt_gap_deinit_profiles(void);
int linuxbt_gap_enable_handler(void);
int linuxbt_gap_disable_handler(void);
int linuxbt_gap_start_discovery_handler(void);
int linuxbt_gap_cancel_discovery_handler(void);
int linuxbt_gap_create_bond_handler(char *pbt_addr, int transport);
int linuxbt_gap_remove_bond_handler(char *pbt_addr);
int linuxbt_gap_set_scan_mode(int mode);
int linuxbt_gap_set_device_name_handler(char *pname);
int linuxbt_gap_get_adapter_properties_handler(void);
int linuxbt_gap_get_rssi_handler(char *pbt_addr);
int linuxbt_gap_config_hci_snoop_log_handler(unsigned char enable);

int linuxbtgap_send_hci_handler(char *ptr);
int linuxbt_gap_set_bt_wifi_ratio(uint8_t bt_ratio, uint8_t wifi_ratio);
const void *linuxbt_gap_get_profile_interface(const char *profile_id);

#if defined(MTK_LINUX_GAP) && (MTK_LINUX_GAP == TRUE)
int linuxbt_interop_database_add(uint16_t feature, bt_bdaddr_t *remote_bd_addr,size_t len);
int linuxbt_interop_database_clear();
//int linuxbt_ble_pair(char *pbt_addr);
#endif

#endif /* __LINUXBT_GAP_IF_H__ */
