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

#include "mtk_bt_service_hidh.h"
#include "c_bt_mw_hidh.h"

#define BT_RC_LOG(_stmt...) \
        do{ \
            if(1){    \
                printf("[HIDH IPC Server]Func:%s Line:%d--->: ", __FUNCTION__, __LINE__);   \
                printf(_stmt); \
                printf("\n"); \
            }        \
        }   \
        while(0)



static void *g_hidh_app_pvtag = NULL;
static mtkrpcapi_BtAppHidhCbk mtkrpcapi_BtHidhEventCbk = NULL;


VOID MWBtHidhEventCbk(BT_HIDH_CBK_STRUCT *param)
{
    BT_RC_LOG("MWBtHidhEventCbk enter!\n");
    if (mtkrpcapi_BtHidhEventCbk)
    {
        BT_RC_LOG("will call mtkrpcapi_BtHidhEventCbk \n");
        mtkrpcapi_BtHidhEventCbk(param, g_hidh_app_pvtag);
    }
    else
    {
        BT_RC_LOG("mtkrpcapi_BtHidhEventCbk is null\n");
    }
}


INT32 x_mtkapi_hidh_register_callback(mtkrpcapi_BtAppHidhCbk func, void *pv_tag)
{
    INT32 i4_ret = 0;

    if (NULL != func)
    {
        g_hidh_app_pvtag = pv_tag;
        mtkrpcapi_BtHidhEventCbk = func;

        i4_ret = c_btm_hid_register_callback(MWBtHidhEventCbk);
    }
    else
    {
        if (NULL != g_hidh_app_pvtag)
        {
            ri_free_cb_tag(g_hidh_app_pvtag);
            g_hidh_app_pvtag = NULL;
        }
        i4_ret = c_btm_hid_register_callback(NULL);
    }

    if (i4_ret != 0)
    {
        BT_RC_LOG("x_mtkapi_hidh_register_callback fail\n");
    }

    return i4_ret;
}



INT32 x_mtkapi_hidh_connect(CHAR *pbt_addr)
{
    BT_RC_LOG("[.c][%s] begin\n", __FUNCTION__);
    INT32 i4_ret = 0;
    i4_ret = c_btm_hid_connect(pbt_addr);
    if (i4_ret != 0)
    {
        BT_RC_LOG("[.c][%s]c_btm_hid_connect fail\n", __FUNCTION__);
    }
    else
    {
        BT_RC_LOG("[.c][%s]c_btm_hid_connect success\n", __FUNCTION__);
    }

    return i4_ret;
}

INT32 x_mtkapi_hidh_disconnect(CHAR *pbt_addr)
{
    BT_RC_LOG("[.c][%s] begin\n", __FUNCTION__);
    INT32 i4_ret = 0;
    i4_ret = c_btm_hid_disconnect(pbt_addr);
    if (i4_ret != 0)
    {
        BT_RC_LOG("[.c][%s]c_btm_hid_disconnect fail\n", __FUNCTION__);
    }
    else
    {
        BT_RC_LOG("[.c][%s]c_btm_hid_disconnect success\n", __FUNCTION__);
    }

    return i4_ret;
}

INT32 x_mtkapi_hidh_set_output_report(CHAR *pbt_addr, CHAR *preport_data)
{
    BT_RC_LOG("[.c][%s] begin\n", __FUNCTION__);
    INT32 i4_ret = 0;
    i4_ret = c_btm_hid_set_output_report(pbt_addr, preport_data);
    if (i4_ret != 0)
    {
        BT_RC_LOG("[.c][%s]c_btm_hid_set_output_report fail\n", __FUNCTION__);
    }
    else
    {
        BT_RC_LOG("[.c][%s]c_btm_hid_set_output_report success\n", __FUNCTION__);
    }

    return i4_ret;
}

INT32 x_mtkapi_hidh_get_input_report(CHAR *pbt_addr, UINT8 reportId, INT32 bufferSize)
{
    BT_RC_LOG("[.c][%s] begin\n", __FUNCTION__);
    INT32 i4_ret = 0;
    i4_ret = c_btm_hid_get_input_report(pbt_addr, reportId, bufferSize);
    if (i4_ret != 0)
    {
        BT_RC_LOG("[.c][%s]c_btm_hid_get_input_report fail\n", __FUNCTION__);
    }
    else
    {
        BT_RC_LOG("[.c][%s]c_btm_hid_get_input_report success\n", __FUNCTION__);
    }

    return i4_ret;
}

INT32 x_mtkapi_hidh_get_output_report(CHAR *pbt_addr, UINT8 reportId, INT32 bufferSize)
{
    BT_RC_LOG("[.c][%s] begin\n", __FUNCTION__);
    INT32 i4_ret = 0;
    i4_ret = c_btm_hid_get_output_report(pbt_addr, reportId, bufferSize);
    if (i4_ret != 0)
    {
        BT_RC_LOG("[.c][%s]c_btm_hid_get_output_report fail\n", __FUNCTION__);
    }
    else
    {
        BT_RC_LOG("[.c][%s]c_btm_hid_get_output_report success\n", __FUNCTION__);
    }

    return i4_ret;
}

INT32 x_mtkapi_hidh_set_input_report(CHAR *pbt_addr, CHAR *preport_data)
{
    BT_RC_LOG("[.c][%s] begin\n", __FUNCTION__);
    INT32 i4_ret = 0;
    i4_ret = c_btm_hid_set_input_report(pbt_addr, preport_data);
    if (i4_ret != 0)
    {
        BT_RC_LOG("[.c][%s]c_btm_hid_set_input_report fail\n", __FUNCTION__);
    }
    else
    {
        BT_RC_LOG("[.c][%s]c_btm_hid_set_input_report success\n", __FUNCTION__);
    }

    return i4_ret;
}

INT32 x_mtkapi_hidh_get_feature_report(CHAR *pbt_addr, UINT8 reportId, INT32 bufferSize)
{
    BT_RC_LOG("[.c][%s] begin\n", __FUNCTION__);
    INT32 i4_ret = 0;
    i4_ret = c_btm_hid_get_feature_report(pbt_addr, reportId, bufferSize);
    if (i4_ret != 0)
    {
        BT_RC_LOG("[.c][%s]c_btm_hid_get_feature_report fail\n", __FUNCTION__);
    }
    else
    {
        BT_RC_LOG("[.c][%s]c_btm_hid_get_feature_report success\n", __FUNCTION__);
    }

    return i4_ret;
}

INT32 x_mtkapi_hidh_set_feature_report(CHAR *pbt_addr, CHAR *preport_data)
{
    BT_RC_LOG("[.c][%s] begin\n", __FUNCTION__);
    INT32 i4_ret = 0;
    i4_ret = c_btm_hid_set_feature_report(pbt_addr, preport_data);
    if (i4_ret != 0)
    {
        BT_RC_LOG("[.c][%s]c_btm_hid_set_feature_report fail\n", __FUNCTION__);
    }
    else
    {
        BT_RC_LOG("[.c][%s]c_btm_hid_set_feature_report success\n", __FUNCTION__);
    }

    return i4_ret;
}

INT32 x_mtkapi_hidh_get_protocol(CHAR *pbt_addr)
{
    BT_RC_LOG("[.c][%s] begin\n", __FUNCTION__);
    INT32 i4_ret = 0;
    i4_ret = c_btm_hid_get_protocol(pbt_addr);
    if (i4_ret != 0)
    {
        BT_RC_LOG("[.c][%s]c_btm_hid_get_protocol fail\n", __FUNCTION__);
    }
    else
    {
        BT_RC_LOG("[.c][%s]c_btm_hid_get_protocol success\n", __FUNCTION__);
    }

    return i4_ret;
}

INT32 x_mtkapi_hidh_set_protocol(CHAR *pbt_addr, UINT8 protocol_mode)
{
    BT_RC_LOG("[.c][%s] begin\n", __FUNCTION__);
    INT32 i4_ret = 0;
    i4_ret = c_btm_hid_set_protocol(pbt_addr, protocol_mode);
    if (i4_ret != 0)
    {
        BT_RC_LOG("[.c][%s]c_btm_hid_set_protocol fail\n", __FUNCTION__);
    }
    else
    {
        BT_RC_LOG("[.c][%s]c_btm_hid_set_protocol success\n", __FUNCTION__);
    }

    return i4_ret;
}

INT32 x_mtkapi_hidh_send_control(CHAR *pbt_addr, UINT8 pcontrol_mode)
{
    INT32 i4_ret = 0;
    return i4_ret;
}

INT32 x_mtkapi_bluetooth_hidh_set_output_report(CHAR *pbt_addr, CHAR *preport_data)
{
    BT_RC_LOG("[.c][%s] begin\n", __FUNCTION__);
    INT32 i4_ret = 0;
    i4_ret = c_btm_hid_set_output_report(pbt_addr, preport_data);
    if (i4_ret != 0)
    {
        BT_RC_LOG("[.c][%s]c_btm_hid_set_output_report fail\n", __FUNCTION__);
    }
    else
    {
        BT_RC_LOG("[.c][%s]c_btm_hid_set_output_report success\n", __FUNCTION__);
    }

    return i4_ret;
}


INT32 x_mtkapi_hidh_disconnect_all(VOID)
{
    BT_RC_LOG("[.c][%s] begin\n", __FUNCTION__);
    INT32 i4_ret = 0;
    i4_ret = c_btm_hid_disconnect_all();
    if (i4_ret != 0)
    {
        BT_RC_LOG("[.c][%s]c_btm_hid_auto_disconnection fail\n", __FUNCTION__);
    }
    else
    {
        BT_RC_LOG("[.c][%s]c_btm_hid_auto_disconnection success\n", __FUNCTION__);
    }

    return i4_ret;
}

