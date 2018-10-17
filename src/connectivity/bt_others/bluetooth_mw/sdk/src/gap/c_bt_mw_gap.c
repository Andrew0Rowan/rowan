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

/* FILE NAME:  c_bt_mw_gap.c
 * PURPOSE:
 *  {1. What is covered in this file - function and scope.}
 *  {2. Related documents or hardware information}
 * NOTES:
 *  {Something must be known or noticed}
 *  {1. How to use these functions - Give an example.}
 *  {2. Sequence of messages if applicable.}
 *  {3. Any design limitation}
 *  {4. Any performance limitation}
 *  {5. Is it a reusable component}
 *
 */

#include "bluetooth.h"
#include "bt_mw_common.h"
#include "bt_mw_gap.h"
#include "c_bt_mw_gap.h"

EXPORT_SYMBOL INT32 c_btm_bt_base_init(BT_APP_CB_FUNC *func)
{
    FUNC_ENTRY;
    return bt_mw_base_init(func);
}

EXPORT_SYMBOL INT32 c_btm_bluetooth_on_off(BOOL fg_on)
{
    FUNC_ENTRY;
    return bt_mw_on_off(fg_on);
}

EXPORT_SYMBOL INT32 c_btm_get_local_dev_info(BT_LOCAL_DEV* local_dev)
{
    FUNC_ENTRY;
    return bt_mw_get_local_dev_info(local_dev);
}

EXPORT_SYMBOL INT32 c_btm_bluetooth_factory_reset(VOID)
{
    FUNC_ENTRY;
    return bt_mw_factory_reset();
}

EXPORT_SYMBOL INT32 c_btm_set_dbg_level(BT_DEBUG_LAYER_NAME_T layer, INT32 level)
{
    FUNC_ENTRY;
    bt_set_dbg_level(layer, level);
    return BT_SUCCESS;
}

EXPORT_SYMBOL INT32 c_btm_set_local_name(CHAR *name)
{
    FUNC_ENTRY;
    return bt_mw_set_local_dev_name(name);
}

EXPORT_SYMBOL INT32 c_btm_pair(CHAR *addr, int transport)
{
    FUNC_ENTRY;
    return bt_mw_pair(addr, transport);
}

EXPORT_SYMBOL INT32 c_btm_unpaire(CHAR *addr)
{
    BT_DBG_NORMAL(BT_DEBUG_GAP, "the MAC is: %s", addr);
    return bt_mw_unpair(addr);
}

EXPORT_SYMBOL INT32 c_btm_get_rssi(CHAR *address, INT16 *rssi_value)
{
    FUNC_ENTRY
    bt_mw_get_rssi(address, rssi_value);
    return BT_SUCCESS;
}

EXPORT_SYMBOL INT32 c_btm_start_inquiry_scan(UINT32 ui4_filter_type)
{
    FUNC_ENTRY;
    return bt_mw_scan(ui4_filter_type);
}

EXPORT_SYMBOL INT32 c_btm_stop_inquiry_scan(VOID)
{
    FUNC_ENTRY;
    return bt_mw_stop_scan();
}

EXPORT_SYMBOL INT32 c_btm_set_connectable_and_discoverable(BOOL fg_conn, BOOL fg_disc)
{
    FUNC_ENTRY;
    return bt_mw_set_connectable_and_discoverable(fg_conn, fg_disc);
}

EXPORT_SYMBOL INT32 c_btm_set_virtual_sniffer(INT32 enable)
{
    FUNC_ENTRY;
    return bt_mw_set_virtual_sniffer(enable);
}

EXPORT_SYMBOL VOID c_btm_send_hci(CHAR *buffer)
{
    FUNC_ENTRY;
    bt_mw_send_hci(buffer);
}

EXPORT_SYMBOL INT32 c_btm_get_dev_info(BLUETOOTH_DEVICE* dev_info, CHAR* bd_addr)
{
    FUNC_ENTRY;
    return bt_mw_gap_get_device_info(dev_info, bd_addr);
}



