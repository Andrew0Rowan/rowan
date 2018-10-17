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


/* FILE NAME:  bt_mw_hidd.h
 * AUTHOR: zwei chen
 * PURPOSE:
 *      It provides hid  interface to c_bt_mw_hidd.c.
 * NOTES:
 */

#ifndef __BT_MW_HID_H__
#define __BT_MW_HID_H__

#include "bluetooth.h"

/****************************************************
 * FUNCTION NAME: bluetooth_hidd_activate
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
INT32 bluetooth_hidd_activate(VOID);

/****************************************************
 * FUNCTION NAME: bluetooth_hidd_deactivate
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
INT32 bluetooth_hidd_deactivate(VOID);


/****************************************************
 * FUNCTION NAME: bluetooth_hidd_init
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
INT32 bluetooth_hidd_init(VOID);



/****************************************************
 * FUNCTION NAME: bluetooth_hidd_deinit
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
INT32 bluetooth_hidd_deinit(VOID);


/****************************************************
 * FUNCTION NAME: bluetooth_hidd_connect
 * PURPOSE:
 *      The function is used to connect  hid device
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
INT32 bluetooth_hidd_connect(CHAR *pbt_addr);


/****************************************************
 * FUNCTION NAME: bluetooth_hidd_disconnect
 * PURPOSE:
 *      The function is used to disconnect  hid device
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
INT32 bluetooth_hidd_disconnect(CHAR *pbt_addr);


/****************************************************
 * FUNCTION NAME: bluetooth_hidd_send_keyboard_data
 * PURPOSE:
 *      The function is used to sent  keyboard data
 * INPUT:
 *      ptData:keyboard data
 *      dataSize: data size
 *
 * OUTPUT:
 *      None
 * RETURN:
 *      INT32:error code
 * NOTES:
 *      None
 */
INT32 bluetooth_hidd_send_keyboard_data(char *ptData, int dataSize);

/****************************************************
 * FUNCTION NAME: bluetooth_hidd_send_mouse_data
 * PURPOSE:
 *      The function is used to sent  mouse data
 * INPUT:
 *      ptData:keyboard data
 *      dataSize: data size
 *
 * OUTPUT:
 *      None
 * RETURN:
 *      INT32:error code
 * NOTES:
 *      None
 */
INT32 bluetooth_hidd_send_mouse_data(char *ptData, int dataSize);


/****************************************************
 * FUNCTION NAME: bluetooth_hidd_send_consumer_data
 * PURPOSE:
 *      The function is used to sent  consumer data
 * INPUT:
 *      ptData:keyboard data
 *      dataSize: data size
 *
 * OUTPUT:
 *      None
 * RETURN:
 *      INT32:error code
 * NOTES:
 *      None
 */
INT32 bluetooth_hidd_send_consumer_data(char *ptData, int dataSize);


/****************************************************
 * FUNCTION NAME: bluetooth_hidd_send_data
 * PURPOSE:
 *      The function is used to sent   data
 * INPUT:
 *      ptdata:keyboard data
 *      dataSize: data size
 *
 * OUTPUT:
 *      None
 * RETURN:
 *      INT32:error code
 * NOTES:
 *      None
 */
INT32 bluetooth_hidd_send_data(char *ptData, int dataSize);

/****************************************************
 * FUNCTION NAME: bluetooth_handle_hidd_connect_cb
 * PURPOSE:
 *      The function is used to notify app connected
 * INPUT:
 *      bd_addr:  address
 *
 *
 * OUTPUT:
 *      None
 * RETURN:
 *      INT32:error code
 * NOTES:
 *      None
 */
INT32 bluetooth_handle_hidd_connect_cb(bt_bdaddr_t *bd_addr);

/****************************************************
 * FUNCTION NAME: bluetooth_handle_hidd_connect_cb
 * PURPOSE:
 *      The function is used to notify app connect failed
 * INPUT:
 *      bd_addr:  address
 *
 *
 * OUTPUT:
 *      None
 * RETURN:
 *      INT32:error code
 * NOTES:
 *      None
 */
INT32 bluetooth_handle_hidd_connect_fail_cb(bt_bdaddr_t *bd_addr);

/****************************************************
 * FUNCTION NAME: bluetooth_handle_hidd_disconnect_cb
 * PURPOSE:
 *      The function is used to notify app disconnected
 * INPUT:
 *      bd_addr:  address
 *
 *
 * OUTPUT:
 *      None
 * RETURN:
 *      INT32:error code
 * NOTES:
 *      None
 */
INT32 bluetooth_handle_hidd_disconnect_cb(bt_bdaddr_t *bd_addr);


#endif


