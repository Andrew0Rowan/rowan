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


/* FILE NAME:  c_bt_mw_hidd.c
 * AUTHOR: zwei chen
 * PURPOSE:
 *      It provides HID device  API to APP.
 * NOTES:
 */

#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include "bt_mw_common.h"
#include "bluetooth.h"
#include "c_bt_mw_hidd.h"
//#include "bluetooth_sync.h"
#include "bt_mw_hidd.h"


/****************************************************
 * FUNCTION NAME: c_bt_hidd_activate
 * PURPOSE:
 *      The function is used to activate hid device
 * INPUT:
 *      None
 *
 * OUTPUT:
 *      None
 * RETURN:
 *      INT32:error code
 * NOTES:
 *      None
 */
EXPORT_SYMBOL INT32 c_bt_hidd_activate(VOID)
{
    FUNC_ENTRY;
    return bluetooth_hidd_activate();
}

/****************************************************
 * FUNCTION NAME: c_bt_hidd_deactivate
 * PURPOSE:
 *      The function is used to deactivate hid device
 * INPUT:
 *      None
 *
 * OUTPUT:
 *      None
 * RETURN:
 *      INT32:error code
 * NOTES:
 *      None
 */
EXPORT_SYMBOL INT32 c_bt_hidd_deactivate(VOID)
{
    FUNC_ENTRY;
    return bluetooth_hidd_deactivate();
}


/****************************************************
 * FUNCTION NAME: c_bt_hidd_init
 * PURPOSE:
 *      The function is used to init hid profile for hid device
 * INPUT:
 *      None
 *
 * OUTPUT:
 *      None
 * RETURN:
 *      INT32:error code
 * NOTES:
 *      None
 */
EXPORT_SYMBOL INT32 c_bt_hidd_init(VOID)
{
    FUNC_ENTRY;
    return bluetooth_hidd_init();
}

/****************************************************
 * FUNCTION NAME: c_bt_hidd_deinit
 * PURPOSE:
 *      The function is used to deinit hid profile for hid device
 * INPUT:
 *      None
 *
 * OUTPUT:
 *      None
 * RETURN:
 *      INT32:error code
 * NOTES:
 *      None
 */
EXPORT_SYMBOL INT32 c_bt_hidd_deinit(VOID)
{
    FUNC_ENTRY;
    return bluetooth_hidd_deinit();
}

/****************************************************
 * FUNCTION NAME: c_bt_hidd_connect
 * PURPOSE:
 *      The function is used to connect  hid host
 * INPUT:
 *      pbt_addr:device address
 *
 * OUTPUT:
 *      None
 * RETURN:
 *      INT32:error code
 * NOTES:
 *      None
 */
EXPORT_SYMBOL INT32 c_bt_hidd_connect(char *pbt_addr)
{
    FUNC_ENTRY;
    BT_DBG_NORMAL(BT_DEBUG_HID, "pbt_addr:%s",pbt_addr);
    return bluetooth_hidd_connect(pbt_addr);
}

/****************************************************
 * FUNCTION NAME: c_bt_hid_disconnect
 * PURPOSE:
 *      The function is used to disconnect  hid host
 * INPUT:
 *      pbt_addr:device address
 *
 * OUTPUT:
 *      None
 * RETURN:
 *      INT32:error code
 * NOTES:
 *      None
 */
EXPORT_SYMBOL INT32 c_bt_hidd_disconnect(CHAR *pbt_addr)
{
    FUNC_ENTRY;
    BT_DBG_NORMAL(BT_DEBUG_HID, "pbt_addr:%s",pbt_addr);
    return bluetooth_hidd_disconnect(pbt_addr);;
}

/****************************************************
 * FUNCTION NAME: c_bt_hidd_send_keyboard_data
 * PURPOSE:
 *      The function is used to sent  keyboard data
 * INPUT:
 *      data:keyboard data
 *      dataSize: data size
 *
 * OUTPUT:
 *      None
 * RETURN:
 *      INT32:error code
 * NOTES:
 *      None
 */
EXPORT_SYMBOL INT32 c_bt_hidd_send_keyboard_data(char *data, int dataSize)
{
    FUNC_ENTRY;
    return bluetooth_hidd_send_keyboard_data(data, dataSize);
}

/****************************************************
 * FUNCTION NAME: c_bt_hidd_send_mouse_data
 * PURPOSE:
 *      The function is used to sent  mouse data
 * INPUT:
 *      data:keyboard data
 *      dataSize: data size
 *
 * OUTPUT:
 *      None
 * RETURN:
 *      INT32:error code
 * NOTES:
 *      None
 */
EXPORT_SYMBOL INT32 c_bt_hidd_send_mouse_data(char *data, int dataSize)
{
    FUNC_ENTRY;


    return bluetooth_hidd_send_mouse_data(data, dataSize);
}

/****************************************************
 * FUNCTION NAME: c_bt_hidd_send_consumer_data
 * PURPOSE:
 *      The function is used to sent  consumer data
 * INPUT:
 *      data:keyboard data
 *      dataSize: data size
 *
 * OUTPUT:
 *      None
 * RETURN:
 *      INT32:error code
 * NOTES:
 *      None
 */
EXPORT_SYMBOL INT32 c_bt_hidd_send_consumer_data(char *data, int dataSize)
{
    FUNC_ENTRY;
    return bluetooth_hidd_send_consumer_data(data, dataSize);
}

/****************************************************
 * FUNCTION NAME: c_bt_hidd_send_data
 * PURPOSE:
 *      The function is used to sent   data
 * INPUT:
 *      data:keyboard data
 *      dataSize: data size
 *
 * OUTPUT:
 *      None
 * RETURN:
 *      INT32:error code
 * NOTES:
 *      None
 */
EXPORT_SYMBOL INT32 c_bt_hidd_send_data(char *data, int dataSize)
{
    FUNC_ENTRY;
    return bluetooth_hidd_send_data(data, dataSize);
}

