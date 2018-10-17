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

/********************************************************************************************
 *     LEGAL DISCLAIMER
 *
 *     (Header of MediaTek Software/Firmware Release or Documentation)
 *
 *     BY OPENING OR USING THIS FILE, BUYER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 *     THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE") RECEIVED
 *     FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO BUYER ON AN "AS-IS" BASIS
 *     ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES, EXPRESS OR IMPLIED,
 *     INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR
 *     A PARTICULAR PURPOSE OR NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY
 *     WHATSOEVER WITH RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 *     INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND BUYER AGREES TO LOOK
 *     ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. MEDIATEK SHALL ALSO
 *     NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE RELEASES MADE TO BUYER'S SPECIFICATION
 *     OR TO CONFORM TO A PARTICULAR STANDARD OR OPEN FORUM.
 *
 *     BUYER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND CUMULATIVE LIABILITY WITH
 *     RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION,
 *     TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE
 *     FEES OR SERVICE charGE PAID BY BUYER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 *     THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE WITH THE LAWS
 *     OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF LAWS PRINCIPLES.
 ************************************************************************************************/

#ifndef _MTK_BT_SERVICE_A2DP_WRAPPER_H_
#define _MTK_BT_SERVICE_A2DP_WRAPPER_H_

#include "u_rpcipc_types.h"
#include "u_bt_mw_a2dp.h"

#ifdef  __cplusplus
extern "C" {
#endif

typedef VOID (*mtkrpcapi_BtAppA2dpCbk)(BT_A2DP_EVENT_PARAM *param, VOID *pv_tag);

/* FUNCTION NAME: a_mtkapi_a2dp_connect
 * PURPOSE:
 *      connect A2DP to a specified device.
 * INPUT:
 *      addr        -- connect to this device
 *      local_role  -- local device role
 * OUTPUT:
 *      N/A
 * RETURN:
 *      BT_SUCCESS  -- connection is sent out successfully
 *      BT_ERR_STATUS_NOT_READY -- A2DP is not ready.
 *      BT_ERR_STATUS_BUSY -- it's not disconnected device or role is changing.
 *      BT_ERR_STATUS_UNSUPPORTED -- it's a wrong role
 *      others      -- connection is sent out fail.
 * NOTES:
 *      when this API return, it does not mean the connection is OK. It just
 *  indicates a A2DP connection is sent out. Caller need wait a async
 *  event:BT_A2DP_EVENT_CONNECTED.
 */
extern INT32 a_mtkapi_a2dp_connect(CHAR *addr, BT_A2DP_ROLE local_role);

/* FUNCTION NAME: a_mtkapi_a2dp_disconnect
 * PURPOSE:
 *      disconnect A2DP connection.
 * INPUT:
 *      addr        -- disconnect to this device
 * OUTPUT:
 *      N/A
 * RETURN:
 *      BT_SUCCESS  -- disconnect is sent out successfully
 *      others      -- disconnect is sent out fail
 * NOTES:
 *      when this API return, it does not mean the disconnection is OK. It just
 *  indicates a A2DP disconnect request is sent out. Caller need wait a async
 *  event:BT_A2DP_EVENT_DISCONNECTED.
 */
extern INT32 a_mtkapi_a2dp_disconnect(char *addr);

/* FUNCTION NAME: a_mtkapi_a2dp_register_callback
 * PURPOSE:
 *      it is used to register an event callback function. The A2DP MW event
 *  will be reported by this callback function. When user call this function
 *  will enable/disable A2DP function.
 * INPUT:
 *      a2dp_handle  -- non NULL: enable A2DP function
 *                      NULL: disable A2DP function
 * OUTPUT:
 *      N/A
 * RETURN:
 *      BT_SUCCESS  -- register successfully
 *      others      -- register fail
 * NOTES:
 *
 */
extern INT32 a_mtkapi_a2dp_register_callback(mtkrpcapi_BtAppA2dpCbk a2dp_handle,
    VOID* pv_tag);

/* FUNCTION NAME: a_mtkapi_a2dp_sink_adjust_buf_time
 * PURPOSE:
 *      adjust the buffer time in player. After buffer some data and then start
 *  to play the streaming data. It is used to avoid choppy sound in bad
 *  environment.
 * INPUT:
 *      buffer_time  -- the buffer time, unit: ms
 * OUTPUT:
 *      N/A
 * RETURN:
 *      BT_SUCCESS  -- set successfully
 *      others      -- set fail
 * NOTES:
 *
 */
extern INT32 a_mtkapi_a2dp_sink_adjust_buf_time(UINT32 buffer_time);

/* FUNCTION NAME: a_mtkapi_a2dp_sink_get_dev_list
 * PURPOSE:
 *      it is used to get source device list which have connected before. The
 *  lastest device is the first one in dev[].
 * INPUT:
 *      N/A
 * OUTPUT:
 *      dev_list  -- the source device list
 * RETURN:
 *      BT_SUCCESS  -- get successfully
 *      others      -- get fail
 * NOTES:
 *
 */
extern INT32 a_mtkapi_a2dp_sink_get_dev_list(BT_A2DP_DEVICE_LIST *dev_list);

/* FUNCTION NAME: a_mtkapi_a2dp_sink_enable
 * PURPOSE:
 *      enable A2DP sink function. Then it can connect to A2DP source device.
 * INPUT:
 *      enable  -- TRUE: enable A2DP sink function
 *                 FALSE: disable A2DP sink function
 * OUTPUT:
 *      N/A
 * RETURN:
 *      BT_SUCCESS  -- set successfully
 *      BT_ERR_STATUS_BUSY -- there is still connection
 *      others      -- set fail
 * NOTES:
 *
 */
extern INT32 a_mtkapi_a2dp_sink_enable(BOOL enable);

/* FUNCTION NAME: a_mtkapi_a2dp_sink_start_player
 * PURPOSE:
 *      when A2DP is connected, it is used to start player to prepare playing.
 * INPUT:
 *      N/A
 * OUTPUT:
 *      N/A
 * RETURN:
 *      BT_SUCCESS  -- set successfully
 *      others      -- set fail
 * NOTES:
 *      this is a async function. Caller should wait BT_A2DP_EVENT_PLAYER_EVENT
 *  to check if player is started.
 */
extern INT32 a_mtkapi_a2dp_sink_start_player(VOID);

/* FUNCTION NAME: a_mtkapi_a2dp_sink_stop_player
 * PURPOSE:
 *      when user want to stop player, it can be used to stop player event A2DP
 *  is connected.
 * INPUT:
 *      N/A
 * OUTPUT:
 *      N/A
 * RETURN:
 *      BT_SUCCESS  -- set successfully
 *      others      -- set fail
 * NOTES:
 *      this is a async function. Caller should wait BT_A2DP_EVENT_PLAYER_EVENT
 *  to check if player is stopped.
 */
extern INT32 a_mtkapi_a2dp_sink_stop_player(VOID);

/* FUNCTION NAME: a_mtkapi_a2dp_src_get_dev_list
 * PURPOSE:
 *      it is used to get sink device list which have connected before. The
 *  lastest device is the first one in dev[].
 * INPUT:
 *      N/A
 * OUTPUT:
 *      dev_list  -- the sink device list
 * RETURN:
 *      BT_SUCCESS  -- get successfully
 *      others      -- get fail
 * NOTES:
 *
 */
extern INT32 a_mtkapi_a2dp_src_get_dev_list(BT_A2DP_DEVICE_LIST *dev_list);

/* FUNCTION NAME: a_mtkapi_a2dp_codec_enable
 * PURPOSE:
 *      it is used to enable/disable A2DP codec
 * INPUT:
 *      codec_type --    codec , the codec is defined in the header file "u_bt_mw_a2dp.h", as follows:
 *      enable     --    true: enable the codec, false: disable the codec
 * RETURN:
 *      BT_SUCCESS  -- enable/disable successfully
 *      others      -- enable/disable fail
 * NOTES:
 *
 */
extern INT32 a_mtkapi_a2dp_codec_enable(BT_A2DP_CODEC_TYPE codec_type, BOOL enable);

/* FUNCTION NAME: a_mtkapi_a2dp_src_enable
 * PURPOSE:
 *      enable  -- TRUE: enable A2DP source function
 *                 FALSE: disable A2DP source function
 * OUTPUT:
 *      N/A
 * RETURN:
 *      BT_SUCCESS  -- set successfully
 *      BT_ERR_STATUS_BUSY -- there is still connection
 *      others      -- set fail
 * NOTES:
 *
 */
extern INT32 a_mtkapi_a2dp_src_enable(BOOL enable);

/* FUNCTION NAME: a_mtkapi_a2dp_set_dbg_flag
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
extern INT32 a_mtkapi_a2dp_set_dbg_flag(BT_A2DP_DBG_FLAG flag,
    BT_A2DP_DBG_PARAM *param);

extern INT32 c_rpc_reg_mtk_bt_service_a2dp_cb_hndlrs(VOID);

#ifdef  __cplusplus
}
#endif
#endif
