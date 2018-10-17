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

#include "mtk_bt_service_a2dp.h"
#include "c_bt_mw_a2dp_src.h"
#include "c_bt_mw_a2dp_snk.h"
#include "c_bt_mw_a2dp_common.h"
#include "ri_common.h"

#define BT_A2DP_RC_LOG(_stmt, ...) \
        do{ \
            if(1){    \
                printf("[A2DP-Server][%s@%d]"_stmt"\n", __FUNCTION__, __LINE__, ## __VA_ARGS__);   \
            }        \
        }   \
        while(0)


static void *g_a2dp_app_pvtag = NULL;

static mtkrpcapi_BtAppA2dpCbk mtkrpcapi_BtA2dpEventCbk = NULL;

VOID MWBtA2dpEventCbk(BT_A2DP_EVENT_PARAM *param)
{
    if (mtkrpcapi_BtA2dpEventCbk)
    {
        mtkrpcapi_BtA2dpEventCbk(param, g_a2dp_app_pvtag);
    }
    else
    {
        BT_A2DP_RC_LOG("mtkrpcapi_BtA2dpEventCbk is null\n");
    }
}




INT32 x_mtkapi_a2dp_connect(CHAR *addr, BT_A2DP_ROLE local_role)
{
    BT_A2DP_RC_LOG("");
    return c_btm_a2dp_connect(addr, local_role);
}

INT32 x_mtkapi_a2dp_disconnect(char *addr)
{
    BT_A2DP_RC_LOG("");
    return c_btm_a2dp_disconnect(addr);
}



INT32 x_mtkapi_a2dp_register_callback(mtkrpcapi_BtAppA2dpCbk func, void *pv_tag)
{
    INT32 i4_ret = 0;

    if (NULL != func)
    {
        g_a2dp_app_pvtag = pv_tag;
        mtkrpcapi_BtA2dpEventCbk = func;

        i4_ret = c_btm_a2dp_register_callback(MWBtA2dpEventCbk);
    }
    else
    {
        if (NULL != g_a2dp_app_pvtag)
        {
            ri_free_cb_tag(g_a2dp_app_pvtag);
            g_a2dp_app_pvtag = NULL;
        }
        i4_ret = c_btm_a2dp_register_callback(NULL);
    }

    if (i4_ret != 0)
    {
        BT_A2DP_RC_LOG("x_mtkapi_a2dp_register_callback fail\n");
    }

    return i4_ret;
}


INT32 x_mtkapi_a2dp_sink_adjust_buf_time(UINT32 buffer_time)
{
    BT_A2DP_RC_LOG("");
    return c_btm_a2dp_sink_adjust_buf_time(buffer_time);
}

INT32 x_mtkapi_a2dp_sink_enable(BOOL enable)
{
    BT_A2DP_RC_LOG("");
    return c_btm_a2dp_sink_enable(enable);
}

INT32 x_mtkapi_a2dp_sink_start_player(VOID)
{
    BT_A2DP_RC_LOG("");
    return c_btm_a2dp_sink_start_player();
}

INT32 x_mtkapi_a2dp_sink_stop_player(VOID)
{
    BT_A2DP_RC_LOG("");
    return c_btm_a2dp_sink_stop_player();
}

INT32 x_mtkapi_a2dp_sink_get_dev_list(BT_A2DP_DEVICE_LIST *dev_list)
{
    BT_A2DP_RC_LOG("");
    return c_btm_a2dp_sink_get_dev_list(dev_list);
}

INT32 x_mtkapi_a2dp_src_enable(BOOL enable)
{
    BT_A2DP_RC_LOG("");
    return c_btm_a2dp_src_enable(enable);
}

INT32 x_mtkapi_a2dp_src_get_dev_list(BT_A2DP_DEVICE_LIST *dev_list)
{
    BT_A2DP_RC_LOG("");
    return c_btm_a2dp_src_get_dev_list(dev_list);
}

INT32 x_mtkapi_a2dp_codec_enable(INT32 codec_type, BOOL enable)
{
    BT_A2DP_RC_LOG("");
    return c_btm_a2dp_codec_enable(codec_type, enable);
}

INT32 x_mtkapi_a2dp_set_dbg_flag(BT_A2DP_DBG_FLAG flag,
    BT_A2DP_DBG_PARAM *param)
{
    BT_A2DP_RC_LOG("");
    return c_btm_a2dp_set_dbg_flag(flag, param);
}


