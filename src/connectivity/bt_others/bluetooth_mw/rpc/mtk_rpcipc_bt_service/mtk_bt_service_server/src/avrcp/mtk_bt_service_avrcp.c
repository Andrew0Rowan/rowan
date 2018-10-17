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

#include "mtk_bt_service_avrcp.h"
#include "c_bt_mw_avrcp.h"
#include "ri_common.h"


#define BT_AVRCP_RC_LOG(_stmt, ...)                         \
    do{                                                     \
        if(1){                                              \
            printf("[AVRCP-Server][%s@%d]"_stmt"\n",        \
                __FUNCTION__, __LINE__, ## __VA_ARGS__);    \
        }                                                   \
    }                                                       \
    while(0)

static void *g_avrcp_app_pvtag = NULL;

static mtkrpcapi_BtAppAvrcpCbk mtkrpcapi_BtAvrcpEventCbk = NULL;

static VOID MWBtAvrcpEventCbk(BT_AVRCP_EVENT_PARAM *param)
{
    if (mtkrpcapi_BtAvrcpEventCbk)
    {
        mtkrpcapi_BtAvrcpEventCbk(param, g_avrcp_app_pvtag);
    }
    else
    {
        BT_AVRCP_RC_LOG("mtkrpcapi_BtAvrcpEventCbk is null\n");
    }
}


INT32 x_mtkapi_avrcp_send_passthrough_cmd(CHAR *addr,
    BT_AVRCP_CMD_TYPE cmd_type, BT_AVRCP_KEY_STATE key_state)
{
    BT_AVRCP_RC_LOG("");
    return c_btm_avrcp_send_passthrough_cmd(addr, cmd_type, key_state);
}

INT32 x_mtkapi_avrcp_send_vendor_unique_cmd(CHAR *addr,
    UINT8 key, BT_AVRCP_KEY_STATE key_state)
{
    BT_AVRCP_RC_LOG("");
    return c_btm_avrcp_send_vendor_unique_cmd(addr, key, key_state);
}


INT32 x_mtkapi_avrcp_change_volume(CHAR *addr, UINT8 volume)
{
    BT_AVRCP_RC_LOG("");
    return c_btm_avrcp_change_volume(addr, volume);
}

INT32 x_mtkapi_avrcp_update_player_status(BT_AVRCP_PLAYER_STATUS *player_status)
{
    BT_AVRCP_RC_LOG("");
    return c_btm_avrcp_update_player_status(player_status);
}

INT32 x_mtkapi_avrcp_update_player_media_info(BT_AVRCP_PLAYER_MEDIA_INFO *player_media_info)
{
    BT_AVRCP_RC_LOG("");
    return c_btm_avrcp_update_player_media_info(player_media_info);
}

INT32 x_mtkapi_avrcp_update_absolute_volume(const UINT8 abs_volume)
{
    BT_AVRCP_RC_LOG("");
    return c_btm_avrcp_update_absolute_volume(abs_volume);
}


INT32 x_mtkapi_avrcp_register_callback(mtkrpcapi_BtAppAvrcpCbk func, void *pv_tag)
{
    INT32 i4_ret = 0;

    if (NULL != func)
    {
        g_avrcp_app_pvtag = pv_tag;
        mtkrpcapi_BtAvrcpEventCbk = func;

        i4_ret = c_btm_avrcp_register_callback(MWBtAvrcpEventCbk);
    }
    else
    {
        if (NULL != g_avrcp_app_pvtag)
        {
            ri_free_cb_tag(g_avrcp_app_pvtag);
            g_avrcp_app_pvtag = NULL;
        }
        i4_ret = c_btm_avrcp_register_callback(NULL);
    }
    if (i4_ret != 0)
    {
        BT_AVRCP_RC_LOG("fail\n");
    }
    else
    {
        BT_AVRCP_RC_LOG("success\n");
    }

    return i4_ret;
}

