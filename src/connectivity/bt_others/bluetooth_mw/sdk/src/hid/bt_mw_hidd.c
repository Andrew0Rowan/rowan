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


/* FILE NAME:  bt_mw_hidd.c
 * AUTHOR: zwei chen
 * PURPOSE:
 *      It provides hid  interface to c_bt_mw_hidd.c.
 * NOTES:
 */


#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>

#include "bt_mw_common.h"
#include "bt_mw_hidd.h"
#include "bluetooth.h"
//#include "bluetooth_sync.h"
#include "linuxbt_hidd_if.h"


extern BtAppEventCbk      BtAppCbk;


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
INT32 bluetooth_hidd_activate(VOID)
{
    FUNC_ENTRY;
    INT32 ret = BT_SUCCESS;
    ret = linuxbt_hidd_activate_handler();
    return ret;
}

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
INT32 bluetooth_hidd_deactivate(VOID)
{
    FUNC_ENTRY;
    INT32 ret = BT_SUCCESS;
    ret = linuxbt_hidd_deactivate_handler();
    return ret;
}

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
INT32 bluetooth_hidd_init(VOID)
{
    FUNC_ENTRY;
    INT32 ret = BT_SUCCESS;
    ret = linuxbt_hidd_init();
    return ret;
}


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
INT32 bluetooth_hidd_deinit(VOID)
{
    FUNC_ENTRY;
    INT32 ret = BT_SUCCESS;
    ret = linuxbt_hidd_deinit();
    return ret;
}

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
INT32 bluetooth_hidd_connect(CHAR *pbt_addr)
{
    FUNC_ENTRY;
    INT32 ret = BT_SUCCESS;
    ret = linuxbt_hidd_connect_handler(pbt_addr);
    return ret;
}

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
INT32 bluetooth_hidd_disconnect(CHAR *pbt_addr)
{
    FUNC_ENTRY;
    INT32 ret = BT_SUCCESS;
    ret = linuxbt_hidd_disconnect_handler(pbt_addr);
    return ret;
}

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
INT32 bluetooth_hidd_send_keyboard_data(char *ptData, int dataSize)
{
    FUNC_ENTRY;
    INT32 ret = BT_SUCCESS;
    int i;
    for(i=0;(i < dataSize);i++)
    {
        BT_DBG_NORMAL(BT_DEBUG_HID, "hex values= %02X ",ptData[i]);
    }
    BT_DBG_NORMAL(BT_DEBUG_HID, "data Size:%d",dataSize);

    unsigned char hex_buf[200] = {0};
    hex_buf[0]= 01;
    memcpy(hex_buf + 1, ptData, dataSize);

    for(i=0; i < (dataSize+1); i++)
    {
       BT_DBG_NORMAL(BT_DEBUG_HID, "hex values= %02X ",hex_buf[i]);
    }
    ret = linuxbt_hidd_send_data_handler((char*)hex_buf, dataSize+1);
    return ret;
}

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
INT32 bluetooth_hidd_send_mouse_data(char *ptData, int dataSize)
{
    FUNC_ENTRY;
    INT32 ret = BT_SUCCESS;
    int i;
    for(i=0;(i < dataSize);i++)
    {
        BT_DBG_NORMAL(BT_DEBUG_HID, "hex values= %02X ",ptData[i]);
    }
    BT_DBG_NORMAL(BT_DEBUG_HID, "data Size:%d", dataSize);

    unsigned char hex_buf[200] = {0};

    hex_buf[0]= 02;
    memcpy(hex_buf + 1, ptData, dataSize);

    for(i=0;i<(dataSize+1);i++)
    {
       BT_DBG_NORMAL(BT_DEBUG_HID, "hex values= %02X ",hex_buf[i]);
    }
    ret = linuxbt_hidd_send_data_handler((char*)hex_buf, dataSize+1);
    return ret;
}

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
INT32 bluetooth_hidd_send_consumer_data(char *ptData, int dataSize)
{
    FUNC_ENTRY;
    INT32 ret = BT_SUCCESS;
    int i;
    for(i=0;(i<dataSize);i++)
    {
       BT_DBG_NORMAL(BT_DEBUG_HID, "hex values= %02X ",ptData[i]);
    }
    BT_DBG_NORMAL(BT_DEBUG_HID, "data Size:%d", dataSize);

    unsigned char hex_buf[200] = {0};

    hex_buf[0]= 03;
    memcpy(hex_buf + 1,ptData, dataSize);

    for(i=0;i<(dataSize+1);i++)
    {
       BT_DBG_NORMAL(BT_DEBUG_HID, "hex values= %02X ",hex_buf[i]);
    }
    ret = linuxbt_hidd_send_data_handler((char*)hex_buf, dataSize+1);
    return ret;
}

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
INT32 bluetooth_hidd_send_data(char *ptData, int dataSize)
{
    FUNC_ENTRY;
    INT32 ret = BT_SUCCESS;
    ret = linuxbt_hidd_send_data_handler(ptData, dataSize);
    return ret;
}

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
INT32 bluetooth_handle_hidd_connect_cb(bt_bdaddr_t *bd_addr)
{
    FUNC_ENTRY;
    INT32 ret = BT_SUCCESS;
    if(NULL == bd_addr)
    {
        BT_DBG_ERROR(BT_DEBUG_HID, "null pointer of bd_addr");
        return BT_ERR_STATUS_FAIL;
    }

    if(BtAppCbk != NULL)
    {
        BtAppCbk(BT_HIDD_CONNECTION_SUCCESS);
    }
    return ret;
}

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
INT32 bluetooth_handle_hidd_connect_fail_cb(bt_bdaddr_t *bd_addr)
{
    FUNC_ENTRY;
    INT32 ret = BT_SUCCESS;
    if(NULL == bd_addr)
    {
        BT_DBG_ERROR(BT_DEBUG_HID, "null pointer of bd_addr");
        return BT_ERR_STATUS_FAIL;
    }

    if(BtAppCbk != NULL)
    {
        BtAppCbk(BT_HIDD_CONNECTION_FAIL);
    }
    return ret;
}


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
INT32 bluetooth_handle_hidd_disconnect_cb(bt_bdaddr_t *bd_addr)
{
    FUNC_ENTRY;
    INT32 ret = BT_SUCCESS;
    if(NULL == bd_addr)
    {
        BT_DBG_ERROR(BT_DEBUG_HID, "null pointer of bd_addr");
        return BT_ERR_STATUS_FAIL;
    }

    if(BtAppCbk != NULL)
    {
        BtAppCbk(BT_HIDD_DISCONNECTION_SUCCESS);
    }
    return ret;
}

