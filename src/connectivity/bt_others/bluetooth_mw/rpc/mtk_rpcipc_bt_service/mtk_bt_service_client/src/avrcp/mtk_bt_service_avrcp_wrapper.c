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
#include "mtk_bt_service_avrcp_wrapper.h"
#include "mtk_bt_service_avrcp_ipcrpc_struct.h"
#include "client_common.h"


#define BT_AVRCP_WRAPPER_LOG(_stmt, ...) \
    do{ \
        if(1){    \
            printf("[AVRCP-WRAPPER][%s@%d]"_stmt"\n", __FUNCTION__, __LINE__, ## __VA_ARGS__);   \
        }        \
    }   \
    while(0)


static INT32 _hndlr_bt_app_avrcp_event_cbk(
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

    ((mtkrpcapi_BtAppAvrcpCbk)pv_cb_addr)
        ((BT_AVRCP_EVENT_PARAM*)pt_args[0].u.pv_desc, pt_args[1].u.pv_arg);
    return RPCR_OK;
}

/* FUNCTION NAME: a_mtkapi_avrcp_send_passthrough_cmd
 * PURPOSE:
 *      it's used to send passthrough command to the remote device.
 * INPUT:
 *      addr        -- peer device address
 *      cmd_type    -- command type
 *      key_state   -- key state
 * OUTPUT:
 *      N/A
 * RETURN:
 *      BT_SUCCESS -- send passthrough command succesfully
 *      Others     -- send failed
 * NOTES:
 *
 */
EXPORT_SYMBOL INT32 a_mtkapi_avrcp_send_passthrough_cmd(CHAR *addr,
    BT_AVRCP_CMD_TYPE cmd_type, BT_AVRCP_KEY_STATE key_state)
{
    BT_AVRCP_WRAPPER_LOG("addr=%s", addr);
    RPC_CLIENT_DECL(3, INT32);
    RPC_CLIENT_ARG_INP(ARG_TYPE_REF_STR, addr);
    RPC_CLIENT_ARG_INP(ARG_TYPE_INT32, cmd_type);
    RPC_CLIENT_ARG_INP(ARG_TYPE_INT32, key_state);
    RPC_BT_SERVICE_CLIENT_DO_OP("x_mtkapi_avrcp_send_passthrough_cmd");
    RPC_CLIENT_RETURN(ARG_TYPE_INT32, -1);
}

/* FUNCTION NAME: a_mtkapi_avrcp_send_vendor_unique_cmd
 * PURPOSE:
 *      it's used to send vendor unique command to the remote device.
 * INPUT:
 *      addr        -- peer device address
 *      key         -- vendor unique key
 *      key_state   -- key state
 * OUTPUT:
 *      N/A
 * RETURN:
 *      BT_SUCCESS -- send vendor unique succesfully
 *      Others     -- send failed
 * NOTES:
 *
 */
EXPORT_SYMBOL INT32 a_mtkapi_avrcp_send_vendor_unique_cmd(CHAR *addr,
    UINT8 key, BT_AVRCP_KEY_STATE key_state)
{
    BT_AVRCP_WRAPPER_LOG("addr=%s", addr);
    RPC_CLIENT_DECL(3, INT32);
    RPC_CLIENT_ARG_INP(ARG_TYPE_REF_STR, addr);
    RPC_CLIENT_ARG_INP(ARG_TYPE_UINT8, key);
    RPC_CLIENT_ARG_INP(ARG_TYPE_INT32, key_state);
    RPC_BT_SERVICE_CLIENT_DO_OP("x_mtkapi_avrcp_send_vendor_unique_cmd");
    RPC_CLIENT_RETURN(ARG_TYPE_INT32, -1);
}


/* FUNCTION NAME: a_mtkapi_avrcp_change_volume
 * PURPOSE:
 *      change the remote sink device's absolute volume
 * INPUT:
 *      addr        -- peer device address
 *      volume      -- the absolute volume, 0~100 => 0%~100%
 * OUTPUT:
 *      N/A
 * RETURN:
 *      BT_SUCCESS -- set absolute volume succesfully
 *      Others     -- set failed
 * NOTES:
 *
 */
EXPORT_SYMBOL INT32 a_mtkapi_avrcp_change_volume(CHAR *addr, UINT8 volume)
{
    BT_AVRCP_WRAPPER_LOG("addr=%s", addr);
    RPC_CLIENT_DECL(2, INT32);
    RPC_CLIENT_ARG_INP(ARG_TYPE_REF_STR, addr);
    RPC_CLIENT_ARG_INP(ARG_TYPE_UINT8, volume);
    RPC_BT_SERVICE_CLIENT_DO_OP("x_mtkapi_avrcp_change_volume");
    RPC_CLIENT_RETURN(ARG_TYPE_INT32, -1);

}

/* FUNCTION NAME: a_mtkapi_avrcp_update_player_status
 * PURPOSE:
 *      it's used to update player status in the MW. When player status change
 *  BT WM will trigger PLAY_STATUS_CHANGED event or PLAYBACK_POS_CHANGED event.
 * INPUT:
 *      player_status  -- player status.
 * OUTPUT:
 *      N/A
 * RETURN:
 *      BT_SUCCESS -- update succesfully
 *      Others     -- update failed
 * NOTES:
 *      This function can be called without AVRCP connection. When Player's
 *  status changes, user should call this function to update MW player status.
 */
EXPORT_SYMBOL
INT32 a_mtkapi_avrcp_update_player_status(BT_AVRCP_PLAYER_STATUS *player_status)
{
    RPC_CLIENT_DECL(1, INT32);
    RPC_CLIENT_CHECK(bt_rpc_add_ref_desc(RPC_DEFAULT_ID,
                    player_status,
                    RPC_DESC_BT_AVRCP_PLAYER_STATUS,
                    NULL));
    RPC_CLIENT_ARG_INP(ARG_TYPE_REF_DESC, player_status);
    RPC_BT_SERVICE_CLIENT_DO_OP("x_mtkapi_avrcp_update_player_status");
    RPC_CLIENT_RETURN(ARG_TYPE_INT32, -1);
}

/* FUNCTION NAME: a_mtkapi_avrcp_update_player_media_info
 * PURPOSE:
 *      it's used to update player media information in the MW. When player
 *  status change BT WM will trigger TRACK_CHANGED event. When the remote device
 *  get element attribute, BT WM will use this information to resposne it.
 * INPUT:
 *      player_media_info  -- player media information, like: songs title, ...
 * OUTPUT:
 *      N/A
 * RETURN:
 *      BT_SUCCESS -- update succesfully
 *      Others     -- update failed
 * NOTES:
 *      This function can be called without AVRCP connection. When Player's
 *  media(song) changes, user should call this function to update MW player
 *  media information.
 */
EXPORT_SYMBOL
INT32 a_mtkapi_avrcp_update_player_media_info(BT_AVRCP_PLAYER_MEDIA_INFO *player_media_info)
{
    RPC_CLIENT_DECL(1, INT32);
    RPC_CLIENT_CHECK(bt_rpc_add_ref_desc(RPC_DEFAULT_ID,
                    player_media_info,
                    RPC_DESC_BT_AVRCP_PLAYER_MEDIA_INFO,
                    NULL));
    RPC_CLIENT_ARG_INP(ARG_TYPE_REF_DESC, player_media_info);
    RPC_BT_SERVICE_CLIENT_DO_OP("x_mtkapi_avrcp_update_player_media_info");
    RPC_CLIENT_RETURN(ARG_TYPE_INT32, -1);
}

/* FUNCTION NAME: a_mtkapi_avrcp_update_absolute_volume
 * PURPOSE:
 *      it's used to update absolute volume in the MW. When local volume changes
 * user should call this function to update BT MW absolute volume. It will
 * trigger VOLUME_CHANGED event if volume changes.
 * INPUT:
 *      abs_volume  -- local volume, unit: %, 0~100%
 * OUTPUT:
 *      N/A
 * RETURN:
 *      BT_SUCCESS -- update succesfully
 *      Others     -- update failed
 * NOTES:
 *      This function can be called without AVRCP connection. When local volume
 * is changed, APP should call this function to update the volume in BT MW.
 */
EXPORT_SYMBOL
INT32 a_mtkapi_avrcp_update_absolute_volume(const UINT8 abs_volume)
{
    RPC_CLIENT_DECL(1, INT32);
    RPC_CLIENT_ARG_INP(ARG_TYPE_UINT8, abs_volume);
    RPC_BT_SERVICE_CLIENT_DO_OP("x_mtkapi_avrcp_update_absolute_volume");
    RPC_CLIENT_RETURN(ARG_TYPE_INT32, -1);
}

/* FUNCTION NAME: a_mtkapi_avrcp_register_callback
 * PURPOSE:
 *      it's used by user to register a AVRCP event callback. User can receive
 *  AVRCP event by this handler.
 * INPUT:
 *      avrcp_handle  -- AVRCP event handler implemented by user
 *      pv_tag        -- it will be passed to avrcp_handle.
 * OUTPUT:
 *      N/A
 * RETURN:
 *      BT_SUCCESS -- register succesfully
 *      Others     -- register failed
 * NOTES:
 *
 */
EXPORT_SYMBOL
INT32 a_mtkapi_avrcp_register_callback(mtkrpcapi_BtAppAvrcpCbk avrcp_handle,
    VOID* pv_tag)
{
    RPC_CLIENT_DECL(2, INT32);

    RPC_CLIENT_ARG_INP(ARG_TYPE_REF_FUNC, avrcp_handle);
    RPC_CLIENT_ARG_INP(ARG_TYPE_REF_VOID, pv_tag);
    BT_AVRCP_WRAPPER_LOG("avrcp_handle=%p, pv_tag=%p", avrcp_handle, pv_tag);

    RPC_BT_SERVICE_CLIENT_DO_OP("x_mtkapi_avrcp_register_callback");
    RPC_CLIENT_RETURN(ARG_TYPE_INT32, -1);
}


INT32 c_rpc_reg_mtk_bt_service_avrcp_cb_hndlrs(VOID)
{
    int i4_ret = 0;
    RPC_REG_CB_HNDLR(bt_app_avrcp_event_cbk);
    return RPCR_OK;
}


