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

/* FILE NAME:  c_bt_mw_a2dp_common.c
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
 *
 *
 */
/* INCLUDE FILE DECLARATIONS
 */
#include "bt_mw_a2dp_common.h"
#if ENABLE_A2DP_SINK
#include "bt_mw_a2dp_snk.h"
#endif
#if ENABLE_A2DP_SRC
#include "bt_mw_a2dp_src.h"
#endif

/* NAMING CONSTANT DECLARATIONS
 */
/* MACRO FUNCTION DECLARATIONS
 */
/* DATA TYPE DECLARATIONS
 */
/* GLOBAL VARIABLE DECLARATIONS
 */
/* LOCAL SUBPROGRAM DECLARATIONS
 */
/* STATIC VARIABLE DECLARATIONS
 */
/* EXPORTED SUBPROGRAM BODIES
 */
/* LOCAL SUBPROGRAM BODIES
 */


/* FUNCTION NAME: c_btm_a2dp_connect
 * PURPOSE:
 *      connect A2DP to a specified device.
 * INPUT:
 *      addr        -- connect to this device
 *      local_role  -- local device role
 * OUTPUT:
 *      N/A
 * RETURN:
 *      BT_SUCCESS  -- connection is sent out successfully
 *      BT_ERR_STATUS_NOT_READY -- a2dp role is not enable or power if off or role is changing.
 *      BT_ERR_STATUS_BUSY -- device is not disconnected status.
 *      others      -- connection is sent out fail.
 * NOTES:
 *      when this API return, it does not mean the connection is OK. It just
 *  indicates a A2DP connection is sent out. Caller need wait a async
 *  event:BT_A2DP_EVENT_CONNECTED.
 */
EXPORT_SYMBOL INT32 c_btm_a2dp_connect(CHAR *addr, BT_A2DP_ROLE local_role)
{
    INT32 ret = BT_ERR_STATUS_UNSUPPORTED;

    BT_CHECK_POINTER(BT_DEBUG_A2DP, addr);

    BT_MW_FUNC_ENTER(BT_DEBUG_A2DP, "addr=%s, local_role=%d", addr, local_role);

    if (BT_A2DP_ROLE_SRC == local_role)
    {
        ret = bt_mw_a2dp_connect(addr, BT_MW_A2DP_ROLE_SRC);
    }
    else if (BT_A2DP_ROLE_SINK == local_role)
    {
        ret = bt_mw_a2dp_connect(addr, BT_MW_A2DP_ROLE_SINK);
    }

    return ret;
}

/* FUNCTION NAME: c_btm_a2dp_disconnect
 * PURPOSE:
 *      disconnect A2DP connection.
 * INPUT:
 *      addr        -- disconnect to this device
 * OUTPUT:
 *      N/A
 * RETURN:
 *      BT_SUCCESS         -- disconnect is sent out successfully
 *      BT_ERR_STATUS_BUSY -- device is disconnecting or connecting.
 *      others             -- disconnect is sent out fail
 * NOTES:
 *      when this API return, it does not mean the disconnection is OK. It just
 *  indicates a A2DP disconnect request is sent out. Caller need wait a async
 *  event:BT_A2DP_EVENT_DISCONNECTED.
 */
EXPORT_SYMBOL INT32 c_btm_a2dp_disconnect(char *addr)
{
    BT_MW_FUNC_ENTER(BT_DEBUG_A2DP, "addr=%s", addr);
    return bt_mw_a2dp_disconnect(addr);
}

/* FUNCTION NAME: a_mtkapi_a2dp_register_callback
 * PURPOSE:
 *      it is used to register an event callback function. The A2DP MW event
 *  will be reported by this callback function.
 * INPUT:
 *      a2dp_handle  -- non NULL: enable A2DP function
 *                      NULL: disable A2DP function
 * OUTPUT:
 *      N/A
 * RETURN:
 *      BT_SUCCESS         -- register successfully
 *      BT_ERR_STATUS_BUSY -- there is connection, pending it.
 *      others             -- register fail
 * NOTES:
 *      If there is connection before deregister callback(disable A2DP), it will
 * be an async function. BT MW will disconnect all A2DP connection and after all
 * connections are disconnected, BT MW will release other resource.
 */
EXPORT_SYMBOL INT32 c_btm_a2dp_register_callback(BT_A2DP_EVENT_HANDLE_CB a2dp_handle)
{
    BT_MW_FUNC_ENTER(BT_DEBUG_A2DP, "a2dp_handle=%p", a2dp_handle);
    return bt_mw_a2dp_register_callback(a2dp_handle);
}

/* FUNCTION NAME: c_btm_a2dp_codec_enable
 * PURPOSE:
 *      it is used to enable/disable A2DP codec
 * INPUT:
 *      codec_type --    codec type
 *      enable     --    true: enable the codec, false: disable the codec
 * NOTES:
 *
 */
EXPORT_SYMBOL INT32 c_btm_a2dp_codec_enable(BT_A2DP_CODEC_TYPE codec_type, BOOL enable)
{
    BT_MW_FUNC_ENTER(BT_DEBUG_A2DP, "codec_type = %d, enable = %d", codec_type, enable);
    return bt_mw_a2dp_codec_enable(codec_type, enable);
}

/* FUNCTION NAME: c_btm_a2dp_set_dbg_flag
 * PURPOSE:
 *      it is used to set some debug flag internally.
 * INPUT:
 *      flag  -- debug flag, indicats which debug flag should be set
 *      param -- the debug parameter, some debug flag will be set as the param.
 * OUTPUT:
 *      N/A
 * RETURN:
 *      BT_SUCCESS  -- set successfully
 *      others      -- set fail
 * NOTES:
 *
 */
EXPORT_SYMBOL INT32 c_btm_a2dp_set_dbg_flag(BT_A2DP_DBG_FLAG flag,
    BT_A2DP_DBG_PARAM *param)
{
    BT_MW_FUNC_ENTER(BT_DEBUG_A2DP, "flag=%d", flag);
    return bt_mw_a2dp_set_dbg_flag(flag, param);
}

