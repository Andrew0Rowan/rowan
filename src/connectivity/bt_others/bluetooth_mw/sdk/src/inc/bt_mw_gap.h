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

#ifndef _BT_MW_GAP_H_
#define _BT_MW_GAP_H_

#include "bluetooth.h"
#include "u_bt_mw_gap.h"
#include "bt_mw_common.h"

typedef struct  {
  void (*init)(void);
  void (*deinit)(void);
  void (*notify_acl_state)(tBTMW_GAP_STATE *gap_state);
  void (*facotry_reset)(void);
  void (*rm_dev)(CHAR *addr);
} profile_operator_t;

/*----------------------------------------------------------------------------
                    functions declarations
 ----------------------------------------------------------------------------*/
INT32 bt_mw_factory_reset(VOID);
INT32 bt_mw_on_off(BOOL fg_on);
INT32 bt_mw_base_init(BT_APP_CB_FUNC *func);
INT32 bt_mw_set_local_dev_name(CHAR *name);
INT32 bt_mw_get_properties(VOID);
INT32 bt_mw_set_power(BOOL fg_on);
INT32 bt_mw_set_power_sync(BOOL fg_on);
INT32 bt_mw_set_connectable_and_discoverable(BOOL fg_conn, BOOL fg_disc);
INT32 bt_mw_set_scanmode_sync(BOOL fg_conn, BOOL fg_disc);
INT32 bt_mw_scan(UINT32 ui4_filter_type);
INT32 bt_mw_get_rssi(CHAR *address, INT16 *rssi_value);
INT32 bt_mw_stop_scan(VOID);
INT32 bt_mw_pair(CHAR *addr, int transport);
INT32 bt_mw_unpair(CHAR *addr);
INT32 bt_mw_gap_init(VOID);
INT32 bt_mw_gap_deinit(VOID);
INT32 bt_mw_set_virtual_sniffer(INT32 enable);
VOID bt_gap_get_rssi_result_cb(INT16 rssi_value);
INT32 bt_mw_send_hci(CHAR* buffer);
INT32 bt_mw_get_bonded_device(VOID);
VOID bt_gap_get_pin_code_cb(bt_pin_code_t *pin, UINT8 *fg_accept);
VOID bt_gap_get_passkey_cb(UINT32 passkey, UINT8 *fg_accept);
void bt_mw_register_profile(UINT8 id, profile_operator_t* operator);
INT32 bt_mw_get_local_dev_info(BT_LOCAL_DEV* local_dev);
INT32 bt_mw_gap_get_device_info(BLUETOOTH_DEVICE* dev_info, CHAR* bd_addr);

#if defined(MTK_LINUX_GAP) && (MTK_LINUX_GAP == TRUE)
INT32 bt_mw_database_add(UINT16 feature, bt_bdaddr_t *remote_bd_addr, size_t len);
INT32 bt_mw_database_clear(VOID);
#endif

#endif /*  _BT_MW_GAP_H_ */

