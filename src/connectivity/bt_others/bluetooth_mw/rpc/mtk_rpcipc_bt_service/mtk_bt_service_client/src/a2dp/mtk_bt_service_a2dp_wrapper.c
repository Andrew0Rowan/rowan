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


/*-----------------------------------------------------------------------------
                        include files
-----------------------------------------------------------------------------*/
#include <stdio.h>

#include "mtk_bt_service_a2dp_wrapper.h"
#include "mtk_bt_service_a2dp_ipcrpc_struct.h"
#include "client_common.h"


#define BT_A2DP_WRAPPER_LOG(_stmt, ...)                 \
    do{                                                 \
        printf("[A2DP-WRAPPER][%s@%d]"_stmt"\n",        \
            __FUNCTION__, __LINE__, ## __VA_ARGS__);    \
    }                                                   \
    while(0)

static INT32 _hndlr_bt_app_a2dp_event_cbk(
    RPC_ID_T     t_rpc_id,
    const CHAR*  ps_cb_type,
    void          *pv_cb_addr,
    UINT32       ui4_num_args,
    ARG_DESC_T*  pt_args,
    ARG_DESC_T*  pt_return)
{
    if(ui4_num_args != 2)
    {
        return RPCR_INV_ARGS;
    }
    pt_return->e_type   = ARG_TYPE_VOID;

    ((mtkrpcapi_BtAppA2dpCbk)pv_cb_addr)
        ((BT_A2DP_EVENT_PARAM*)pt_args[0].u.pv_desc, pt_args[1].u.pv_arg);
    return RPCR_OK;
}



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
 *      BT_ERR_STATUS_NOT_READY -- a2dp role is not enable or power if off or role is changing.
 *      BT_ERR_STATUS_BUSY -- device is not disconnected status.
 *      others      -- connection is sent out fail.
 * NOTES:
 *      when this API return, it does not mean the connection is OK. It just
 *  indicates a A2DP connection is sent out. Caller need wait a async
 *  event:BT_A2DP_EVENT_CONNECTED.
 */
EXPORT_SYMBOL INT32 a_mtkapi_a2dp_connect(CHAR *addr, BT_A2DP_ROLE local_role)
{
    BT_A2DP_WRAPPER_LOG("");
    RPC_CLIENT_DECL(2, INT32);

    RPC_CLIENT_ARG_INP(ARG_TYPE_REF_STR, addr);
    RPC_CLIENT_ARG_INP(ARG_TYPE_INT32, local_role);

    RPC_BT_SERVICE_CLIENT_DO_OP("x_mtkapi_a2dp_connect");
    RPC_CLIENT_RETURN(ARG_TYPE_INT32, -1);
}

/* FUNCTION NAME: a_mtkapi_a2dp_disconnect
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
EXPORT_SYMBOL INT32 a_mtkapi_a2dp_disconnect(char *addr)
{
    BT_A2DP_WRAPPER_LOG("");
    RPC_CLIENT_DECL(1, INT32);
    RPC_CLIENT_ARG_INP(ARG_TYPE_REF_STR, addr);

    RPC_BT_SERVICE_CLIENT_DO_OP("x_mtkapi_a2dp_disconnect");
    RPC_CLIENT_RETURN(ARG_TYPE_INT32, -1);
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
EXPORT_SYMBOL
INT32 a_mtkapi_a2dp_register_callback(mtkrpcapi_BtAppA2dpCbk a2dp_handle,
    VOID* pv_tag)
{
    RPC_CLIENT_DECL(2, INT32);

    RPC_CLIENT_ARG_INP(ARG_TYPE_REF_FUNC, a2dp_handle);
    RPC_CLIENT_ARG_INP(ARG_TYPE_REF_VOID, pv_tag);
    BT_A2DP_WRAPPER_LOG("a2dp_handle=%p, pv_tag=%p", a2dp_handle, pv_tag);

    RPC_BT_SERVICE_CLIENT_DO_OP("x_mtkapi_a2dp_register_callback");
    RPC_CLIENT_RETURN(ARG_TYPE_INT32, -1);
}

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
EXPORT_SYMBOL INT32 a_mtkapi_a2dp_sink_adjust_buf_time(UINT32 buffer_time)
{
    BT_A2DP_WRAPPER_LOG("");
    RPC_CLIENT_DECL(1, INT32);

    RPC_CLIENT_ARG_INP(ARG_TYPE_UINT32, buffer_time);

    RPC_BT_SERVICE_CLIENT_DO_OP("x_mtkapi_a2dp_sink_adjust_buf_time");
    RPC_CLIENT_RETURN(ARG_TYPE_INT32, -1);

}

/* FUNCTION NAME: a_mtkapi_a2dp_sink_get_dev_list
 * PURPOSE:
 *      it is used to get source device list which have connected before. The
 *  lastest device is the last one.
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
EXPORT_SYMBOL
INT32 a_mtkapi_a2dp_sink_get_dev_list(BT_A2DP_DEVICE_LIST *dev_list)
{
    BT_A2DP_WRAPPER_LOG("");
    RPC_CLIENT_DECL(1, INT32);
    RPC_CLIENT_CHECK(bt_rpc_add_ref_desc(RPC_DEFAULT_ID,
                    dev_list,
                    RPC_DESC_BT_A2DP_DEVICE_LIST,
                    NULL));
    RPC_CLIENT_ARG_OUT(ARG_TYPE_REF_DESC, dev_list);
    RPC_BT_SERVICE_CLIENT_DO_OP("x_mtkapi_a2dp_sink_get_dev_list");
    RPC_CLIENT_RETURN(ARG_TYPE_INT32, -1);
}

/* FUNCTION NAME: a_mtkapi_a2dp_sink_enable
 * PURPOSE:
 *      enable A2DP sink function. Then it can connect to A2DP source device.
 * INPUT:
 *      enable  -- TRUE: enable A2DP sink function
 *                 FALSE: disable A2DP sink function
 * OUTPUT:
 *      N/A
 * RETURN:
 *      BT_SUCCESS         -- set successfully
 *      BT_ERR_STATUS_BUSY -- there is connection, pending it.
*      others              -- set fail
 * NOTES:
 *      If there is no connection to source device, disable operation will
 * return with BT_SUCCESS.
 *      If there is connections to source device, disable will pending and BT MW
 * will disconnect all A2DP Source connections. When all A2DP source connectons
 * are disconnected, then BT MW will disable/enable sink role and report
 * BT_A2DP_EVENT_ROLE_CHANGED event to APP.
 *      Please set scan mode as 0 to protect disable operation. After this, APP
 * can restore scan mode.
 */
EXPORT_SYMBOL INT32 a_mtkapi_a2dp_sink_enable(BOOL enable)
{
    BT_A2DP_WRAPPER_LOG("");
    RPC_CLIENT_DECL(1, UINT8);

    RPC_CLIENT_ARG_INP(ARG_TYPE_UINT8, enable);

    RPC_BT_SERVICE_CLIENT_DO_OP("x_mtkapi_a2dp_sink_enable");
    RPC_CLIENT_RETURN(ARG_TYPE_INT32, -1);

}

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
EXPORT_SYMBOL INT32 a_mtkapi_a2dp_sink_start_player(VOID)
{
    BT_A2DP_WRAPPER_LOG("");
    INT32 tmp = 0;
    RPC_CLIENT_DECL(1, INT32);
    RPC_CLIENT_ARG_INP(ARG_TYPE_INT32, tmp);
    RPC_BT_SERVICE_CLIENT_DO_OP("x_mtkapi_a2dp_sink_start_player");
    RPC_CLIENT_RETURN(ARG_TYPE_INT32, -1);
}

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
EXPORT_SYMBOL INT32 a_mtkapi_a2dp_sink_stop_player(VOID)
{
    BT_A2DP_WRAPPER_LOG("");
    INT32 tmp = 0;
    RPC_CLIENT_DECL(1, INT32);
    RPC_CLIENT_ARG_INP(ARG_TYPE_INT32, tmp);
    RPC_BT_SERVICE_CLIENT_DO_OP("x_mtkapi_a2dp_sink_stop_player");
    RPC_CLIENT_RETURN(ARG_TYPE_INT32, -1);
}

/* FUNCTION NAME: a_mtkapi_a2dp_src_get_dev_list
 * PURPOSE:
 *      it is used to get sink device list which have connected before. The
 *  lastest device is the last one.
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
EXPORT_SYMBOL INT32 a_mtkapi_a2dp_src_get_dev_list(BT_A2DP_DEVICE_LIST *dev_list)
{
    BT_A2DP_WRAPPER_LOG("");
    RPC_CLIENT_DECL(1, INT32);
    RPC_CLIENT_CHECK(bt_rpc_add_ref_desc(RPC_DEFAULT_ID,
                    dev_list,
                    RPC_DESC_BT_A2DP_DEVICE_LIST,
                    NULL));
    RPC_CLIENT_ARG_OUT(ARG_TYPE_REF_DESC, dev_list);
    RPC_BT_SERVICE_CLIENT_DO_OP("x_mtkapi_a2dp_src_get_dev_list");
    RPC_CLIENT_RETURN(ARG_TYPE_INT32, -1);
}

/* FUNCTION NAME: a_mtkapi_a2dp_codec_enable
 * PURPOSE:
 *      it is used to enable/disable A2DP codec
 * INPUT:
 *      codec_type --    codec type
 *      enable     --    true: enable the codec, false: disable the codec
 * NOTES:
 *
 */
EXPORT_SYMBOL INT32 a_mtkapi_a2dp_codec_enable(BT_A2DP_CODEC_TYPE codec_type, BOOL enable)
{
    BT_A2DP_WRAPPER_LOG("");
    RPC_CLIENT_DECL(2, INT32);

    RPC_CLIENT_ARG_INP(ARG_TYPE_INT32, codec_type);
    RPC_CLIENT_ARG_INP(ARG_TYPE_UINT8, enable);

    RPC_BT_SERVICE_CLIENT_DO_OP("x_mtkapi_a2dp_codec_enable");
    RPC_CLIENT_RETURN(ARG_TYPE_INT32, -1);
}

/* FUNCTION NAME: a_mtkapi_a2dp_src_enable
 * PURPOSE:
 *      enable  -- TRUE: enable A2DP source function
 *                 FALSE: disable A2DP source function
 * OUTPUT:
 *      N/A
 * RETURN:
 *      BT_SUCCESS         -- set successfully
 *      BT_ERR_STATUS_BUSY -- there is connection, pending it.
 *      others             -- set fail
 * NOTES:
 *      If there is no connection to sink device, disable operation will
 * return with BT_SUCCESS.
 *      If there is connections to source device, disable will pending and BT MW
 * will disconnect all A2DP sink connections. When all A2DP sink connectons
 * are disconnected, then BT MW will disable/enable source role and report
 * BT_A2DP_EVENT_ROLE_CHANGED event to APP.
 *      Please set scan mode as 0 to protect disable operation. After this, APP
 * can restore scan mode.
 */
EXPORT_SYMBOL INT32 a_mtkapi_a2dp_src_enable(BOOL enable)
{
    BT_A2DP_WRAPPER_LOG("");
    RPC_CLIENT_DECL(1, UINT8);

    RPC_CLIENT_ARG_INP(ARG_TYPE_UINT8, enable);

    RPC_BT_SERVICE_CLIENT_DO_OP("x_mtkapi_a2dp_src_enable");
    RPC_CLIENT_RETURN(ARG_TYPE_INT32, -1);

}

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
EXPORT_SYMBOL INT32 a_mtkapi_a2dp_set_dbg_flag(BT_A2DP_DBG_FLAG flag,
    BT_A2DP_DBG_PARAM *param)
{
    BT_A2DP_WRAPPER_LOG("");
    RPC_CLIENT_DECL(2, INT32);
    RPC_CLIENT_ARG_INP(ARG_TYPE_INT32, flag);
    RPC_CLIENT_CHECK(bt_rpc_add_ref_desc(RPC_DEFAULT_ID,
                    param,
                    RPC_DESC_BT_A2DP_DBG_PARAM,
                    NULL));
    RPC_CLIENT_ARG_INP(ARG_TYPE_REF_DESC, param);
    RPC_BT_SERVICE_CLIENT_DO_OP("x_mtkapi_a2dp_set_dbg_flag");
    RPC_CLIENT_RETURN(ARG_TYPE_INT32, -1);
}

INT32 c_rpc_reg_mtk_bt_service_a2dp_cb_hndlrs(VOID)
{
    int i4_ret = 0;
    RPC_REG_CB_HNDLR(bt_app_a2dp_event_cbk);
    return RPCR_OK;
}

