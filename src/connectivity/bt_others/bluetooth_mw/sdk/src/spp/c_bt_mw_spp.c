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


#include "c_mw_config.h"
#include "bt_mw_common.h"
#include "bt_mw_spp.h"

/**
 * FUNCTION NAME: c_btm_spp_register_callback
 * PURPOSE:
 *      The function is used to register spp callback.
 * INPUT:
 *      spp_cb -- spp callback function
 * OUTPUT:
 *      None
 * RETURN:
 *      BT_SUCCESS           -- Operate success.
 *      BT_ERR_STATUS_NULL_POINTER   -- Input parameter is null pointer.
 * NOTES:
 *      None
 */
INT32 c_btm_spp_register_callback(BtAppSppCbk spp_cb)
{
    return bt_mw_spp_register_callback(spp_cb);
}

/**
 * FUNCTION NAME: c_btm_spp_connect
 * PURPOSE:
 *      The function is used to SPP connect with uuid.
 * INPUT:
 *      addr -- the device address to connect.
 *      uuid -- the uuid to connect.
 * OUTPUT:
 *      None
 * RETURN:
 *      BT_SUCCESS           -- Operate success.
 *      BT_ERR_STATUS_FAIL   -- Operate fail.
 * NOTES:
 *      None
 */
EXPORT_SYMBOL INT32 c_btm_spp_connect(CHAR *addr, CHAR *uuid)
{
    FUNC_ENTRY;

    BT_DBG_INFO(BT_DEBUG_SPP, "connect MAC is: %s", addr);
    BT_DBG_INFO(BT_DEBUG_SPP, "connect UUID is: %s", uuid);

    return bt_mw_spp_connect(addr, uuid);
}

/**
 * FUNCTION NAME: c_btm_spp_disconnect
 * PURPOSE:
 *      The function is used to SPP disconnect.
 * INPUT:
 *      addr -- the device address to disconnect.
 *      uuid -- the uuid to disconnect.
 * OUTPUT:
 *      None
 * RETURN:
 *      BT_SUCCESS           -- Operate success.
 *      BT_ERR_STATUS_FAIL   -- Operate fail.
 * NOTES:
 *      None
 */
EXPORT_SYMBOL INT32 c_btm_spp_disconnect(CHAR *addr, CHAR *uuid)
{
    FUNC_ENTRY;

    BT_DBG_INFO(BT_DEBUG_SPP, "disconnect MAC is: %s", addr);
    BT_DBG_INFO(BT_DEBUG_SPP, "disconnect UUID is: %s", uuid);

    return bt_mw_spp_disconnect(addr, uuid);
}

/**
 * FUNCTION NAME: c_btm_spp_send_data
 * PURPOSE:
 *      The function is used to send data.
 * INPUT:
 *      bd_addr -- the device address to send.
 *      uuid -- the uuid to send.
 *      str -- the string to send.
 *      len -- the string length to send.
 * OUTPUT:
 *      None
 * RETURN:
 *      BT_SUCCESS           -- Operate success.
 *      BT_ERR_STATUS_FAIL   -- Operate fail.
 * NOTES:
 *      None
 */
EXPORT_SYMBOL INT32 c_btm_spp_send_data(CHAR* bd_addr, CHAR *uuid, CHAR* str, INT32 len)
{
    FUNC_ENTRY;
    return bt_mw_spp_send_data(bd_addr, uuid, str, len);
}

/**
 * FUNCTION NAME: c_btm_spp_start_server
 * PURPOSE:
 *      The function is used to enable deviceB, the platform is deviceB.
 * INPUT:
 *      servername -- the deviceB's server name.
 *      uuid -- the uuid to connect/send/disconnect.
 * OUTPUT:
 *      None
 * RETURN:
 *      BT_SUCCESS           -- Operate success.
 *      BT_ERR_STATUS_FAIL   -- Operate fail.
 * NOTES:
 *      None
 */
EXPORT_SYMBOL INT32 c_btm_spp_start_server(CHAR *servername, CHAR* uuid)
{
    FUNC_ENTRY;

    BT_DBG_INFO(BT_DEBUG_SPP, "SPP enable devb servername:%s", servername);
    BT_DBG_INFO(BT_DEBUG_SPP, "SPP enable devb uuid:%s", uuid);

    return bt_mw_spp_start_server(servername, uuid);
}

/**
 * FUNCTION NAME: c_btm_spp_stop_server
 * PURPOSE:
 *      The function is used to disable deviceB, the platform is deviceB.
 * INPUT:
 *      uuid -- the uuid to disable.
 * OUTPUT:
 *      None
 * RETURN:
 *      BT_SUCCESS           -- Operate success.
 *      BT_ERR_STATUS_FAIL   -- Operate fail.
 * NOTES:
 *      When have connection using this service, after disable devb, the connection also abort.
 */
EXPORT_SYMBOL INT32 c_btm_spp_stop_server(CHAR *uuid)
{
    FUNC_ENTRY;

    BT_DBG_INFO(BT_DEBUG_SPP, "SPP disable devb uuid:%s", uuid);

    return bt_mw_spp_stop_server(uuid);
}

