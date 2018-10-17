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

#include "mtk_bt_service_gap_wrapper.h"
#include "mtk_bt_service_gap.h"
#include "c_bt_mw_gap.h"

#define BT_RC_LOG(_stmt...) \
        do{ \
            if(1){    \
                printf("Func:%s Line:%d--->: ", __FUNCTION__, __LINE__);   \
                printf(_stmt); \
                printf("\n"); \
            }        \
        }   \
        while(0)

void *g_gap_pvtag = NULL;

mtkrpcapi_BtAppGapEventCbk mtkrpcapi_BtEventCbk = NULL;
mtkrpcapi_BtAppGapGetPairingKeyCbk mtkrpcapi_BtGapGetPairingKeyCbk = NULL;
mtkrpcapi_BtAppGapInquiryResponseCbk mtkrpcapi_BtGapInquiryResponseCbk = NULL;

VOID MWBtAppInquiryResponseCbk(tBTMW_GAP_DEVICE_INFO *device_info)
{
    BT_RC_LOG("Enter MWBtAppInquiryResponseCbk\n");
    if (mtkrpcapi_BtGapInquiryResponseCbk)
    {
        mtkrpcapi_BtGapInquiryResponseCbk(device_info, g_gap_pvtag);
    }
    else
    {
        BT_RC_LOG("mtkrpcapi_BtEventCbk is null\n");
    }
}


VOID MWBtAppEventCbk(tBTMW_GAP_STATE *bt_event)
{
    if (mtkrpcapi_BtEventCbk)
    {
        mtkrpcapi_BtEventCbk(bt_event, g_gap_pvtag);
    }
    else
    {
        BT_RC_LOG("mtkrpcapi_BtEventCbk is null\n");
    }
}

VOID MWBtAppGapGetPairingKeyCbk(pairing_key_value_t *bt_pairing_key, UINT8 *fg_accept)
{
    if ((NULL == bt_pairing_key) || (NULL == fg_accept))
    {
        if (NULL == bt_pairing_key)
        {
            BT_RC_LOG("bt_pairing_key is null\n");
        }
        if (NULL == fg_accept)
        {
            BT_RC_LOG("fg_accept is null\n");
        }
        return;
    }
    BT_RC_LOG("pin_code = %s, key_value= %u, fg_accept=%u\n", bt_pairing_key->pin_code, bt_pairing_key->key_value, *fg_accept);
    if (mtkrpcapi_BtGapGetPairingKeyCbk)
    {
        mtkrpcapi_BtGapGetPairingKeyCbk(bt_pairing_key, fg_accept, g_gap_pvtag);
    }
    else
    {
        BT_RC_LOG("mtkrpcapi_BtGapGetPairingKeyCbk is null\n");
    }
}


/*register APP callback function*/
INT32 mtkrpcapi_btm_gap_register_cbk_fct(MTKRPCAPI_BT_APP_CB_FUNC *func, BT_APP_CB_FUNC *app_func, void *pv_tag)
{
    INT32 i4_ret = 0;

    g_gap_pvtag = pv_tag;
    if(NULL == func)
    {
        BT_RC_LOG(("callback func is null!\n"));
        return BT_ERR_STATUS_NULL_POINTER;
    }
    if(func->bt_event_cb)
    {
        mtkrpcapi_BtEventCbk = func->bt_event_cb;
        app_func->bt_gap_event_cb = MWBtAppEventCbk;
    }
    else
    {
        BT_RC_LOG(("event callback func is null!\n"));
        app_func->bt_gap_event_cb = NULL;
    }

    if(func->bt_dev_info_cb)
    {
        mtkrpcapi_BtGapInquiryResponseCbk = func->bt_dev_info_cb;
        app_func->bt_dev_info_cb = MWBtAppInquiryResponseCbk;
    }
    else
    {
        BT_RC_LOG(("dev_info func is null!\n"));
        app_func->bt_dev_info_cb = NULL;
    }

    if(func->bt_get_pairing_key_cb)
    {
        mtkrpcapi_BtGapGetPairingKeyCbk = func->bt_get_pairing_key_cb;
        app_func->bt_get_pairing_key_cb = MWBtAppGapGetPairingKeyCbk;
    }
    else
    {
        BT_RC_LOG(("bt_get_pairing_key_cb callback func is null!\n"));
        app_func->bt_get_pairing_key_cb = NULL;
    }

    return i4_ret;
}


INT32 x_mtkapi_bt_base_init(MTKRPCAPI_BT_APP_CB_FUNC* func, void *pv_tag)
{
    INT32 i4_ret = 0;

    BT_APP_CB_FUNC app_func;
    memset(&app_func, 0, sizeof(BT_APP_CB_FUNC));

    i4_ret = mtkrpcapi_btm_gap_register_cbk_fct(func, &app_func, pv_tag);
    if(0 != i4_ret)
    {
        BT_RC_LOG(("x_mtkapi_bt_base_init Error.\n"));
        return i4_ret;
    }

    i4_ret = c_btm_bt_base_init(&app_func);
    if (i4_ret != 0)
    {
        BT_RC_LOG("c_btm_bt_base_init fail\n");
    }
    else
    {
        BT_RC_LOG("c_btm_bt_base_init success\n");
    }

    return i4_ret;
}

INT32 x_mtkapi_bt_gap_set_dbg_level(BT_DEBUG_LAYER_NAME_T layer, INT32 level)
{
    INT32 i4_ret = 0;
    i4_ret = c_btm_set_dbg_level(layer, level);
    if (i4_ret != 0)
    {
        BT_RC_LOG("c_btm_set_dbg_level fail\n");
    }
    else
    {
        BT_RC_LOG("c_btm_set_dbg_level success\n");
    }
    return i4_ret;
}

INT32 x_mtkapi_bt_gap_on_off(BOOL fg_on)
{
    INT32 i4_ret = 0;
    i4_ret = c_btm_bluetooth_on_off(fg_on);
    if (i4_ret != 0)
    {
        BT_RC_LOG("c_btm_bluetooth_on_off fail\n");
    }
    else
    {
        BT_RC_LOG("c_btm_bluetooth_on_off success\n");
    }
    return i4_ret;
}

INT32 x_mtkapi_bt_bluetooth_factory_reset()
{
    INT32 i4_ret = 0;
    i4_ret = c_btm_bluetooth_factory_reset();
    if (i4_ret != 0)
    {
        BT_RC_LOG("c_btm_bluetooth_factory_reset fail\n");
    }
    else
    {
        BT_RC_LOG("c_btm_bluetooth_factory_reset success\n");
    }
    return i4_ret;
}

INT32 x_mtkapi_bt_mw_log_setStackLevel(const char *trc_name, int level)
{
    INT32 i4_ret = 0;
    i4_ret = bt_mw_log_setStackLevel(trc_name, level);
    if (i4_ret != 0)
    {
        BT_RC_LOG("bt_mw_log_setStackLevel fail\n");
    }
    else
    {
        BT_RC_LOG("bt_mw_log_setStackLevel success\n");
    }
    return i4_ret;
}

INT32 x_mtkapi_bt_gap_set_name(CHAR *name)
{
    INT32 i4_ret = 0;
    i4_ret = c_btm_set_local_name(name);
    if (i4_ret != 0)
    {
        BT_RC_LOG("c_btm_set_local_name fail\n");
    }
    else
    {
        BT_RC_LOG("c_btm_set_local_name success\n");
    }
    return i4_ret;
}

INT32 x_mtkapi_bt_gap_set_connectable_and_discoverable(BOOL fg_conn, BOOL fg_disc)
{
    INT32 i4_ret = 0;
    i4_ret = c_btm_set_connectable_and_discoverable(fg_conn, fg_disc);
    if (i4_ret != 0)
    {
        BT_RC_LOG("c_btm_set_connectable_and_discoverable fail\n");
    }
    else
    {
        BT_RC_LOG("c_btm_set_connectable_and_discoverable success\n");
    }
    return i4_ret;
}

INT32 x_mtkapi_bt_gap_get_dev_info(BLUETOOTH_DEVICE* dev_info, CHAR* bd_addr)
{
    INT32 i4_ret = 0;
    i4_ret = c_btm_get_dev_info(dev_info, bd_addr);
    if (i4_ret != 0)
    {
        BT_RC_LOG("c_btm_get_dev_info fail\n");
    }
    else
    {
        BT_RC_LOG("c_btm_get_dev_info success\n");
    }
    return i4_ret;
}

INT32 x_mtkapi_bt_gap_get_local_dev_info(BT_LOCAL_DEV *ps_dev_info)
{
    INT32 i4_ret = 0;
    i4_ret = c_btm_get_local_dev_info(ps_dev_info);
    if (i4_ret != 0)
    {
        BT_RC_LOG("c_btm_get_local_dev_info fail\n");
    }
    else
    {
        BT_RC_LOG("c_btm_get_local_dev_info success\n");
    }
    return i4_ret;
}

INT32 x_mtkapi_bt_gap_start_inquiry_scan(UINT32 ui4_filter_type)
{
    return c_btm_start_inquiry_scan(ui4_filter_type);
}

INT32 x_mtkapi_bt_gap_stop_inquiry_scan()
{
    INT32 i4_ret = 0;
    i4_ret = c_btm_stop_inquiry_scan();
    if (i4_ret != 0)
    {
        BT_RC_LOG("c_btm_start_inquiry_scan fail\n");
    }
    else
    {
        BT_RC_LOG("c_btm_start_inquiry_scan success\n");
    }
    return i4_ret;
}

INT32 x_mtkapi_bt_gap_pair(CHAR *addr, int transport)
{
    INT32 i4_ret = 0;
    i4_ret = c_btm_pair(addr, transport);
    if (i4_ret != 0)
    {
        BT_RC_LOG("c_btm_pair fail\n");
    }
    else
    {
        BT_RC_LOG("c_btm_pair success\n");
    }
    return i4_ret;
}

INT32 x_mtkapi_bt_gap_unpair(CHAR *addr)
{
    INT32 i4_ret = 0;
    i4_ret = c_btm_unpaire(addr);
    if (i4_ret != 0)
    {
        BT_RC_LOG("c_btm_unpaire fail\n");
    }
    else
    {
        BT_RC_LOG("c_btm_unpaire success\n");
    }
    return i4_ret;
}

INT32 x_mtkapi_bt_gap_get_rssi(CHAR *address, INT16 *rssi_value)
{
    INT32 i4_ret = 0;
    i4_ret = c_btm_get_rssi(address, rssi_value);
    if (i4_ret != 0)
    {
        BT_RC_LOG("c_btm_get_rssi fail\n");
    }
    else
    {
        BT_RC_LOG("c_btm_get_rssi success\n");
    }
    return i4_ret;
}

INT32 x_mtkapi_bt_gap_set_virtual_sniffer(INT32 enable)
{
    INT32 i4_ret = 0;
    i4_ret = c_btm_set_virtual_sniffer(enable);
    if (i4_ret != 0)
    {
        BT_RC_LOG("c_btm_set_virtual_sniffer fail\n");
    }
    else
    {
        BT_RC_LOG("c_btm_set_virtual_sniffer success\n");
    }
    return i4_ret;
}

INT32 x_mtkapi_bt_gap_send_hci(CHAR *buffer)
{
    INT32 i4_ret = 0;
    c_btm_send_hci(buffer);
    return i4_ret;
}


