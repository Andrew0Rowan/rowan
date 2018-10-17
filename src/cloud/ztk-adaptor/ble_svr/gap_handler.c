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

#include <string.h>
#include "u_bt_mw_gap.h"
#include "mtk_bt_service_gap_wrapper.h"
#include "adaptor_log.h"
#include "bt_msg.h"

void bt_app_gap_event_callback(tBTMW_GAP_STATE *bt_event, void* pv_tag)
{
    switch (bt_event->state)
     {
     case GAP_STATE_ON:
        APDATOR_LOG(INFO,"bt_gap_event = state on");
        break;
     case GAP_STATE_OFF:
        APDATOR_LOG(INFO,"bt_gap_event = state off");
        break;

     case GAP_STATE_ACL_CONNECTED:
        APDATOR_LOG(INFO,"bt_gap_event = acl connected");
        break;

     case GAP_STATE_ACL_DISCONNECTED:
        if(bt_event->reason)
        {
             if (0x08 == bt_event->reason)
             {
                 APDATOR_LOG(INFO,"bt_gap_event = connect lost");
             }
        }
        else
        {
            APDATOR_LOG(INFO,"bt_gap_event = acl is disconnected");
        }
        break;

     case GAP_STATE_BONDED:
        break;
     case GAP_STATE_BONDING:
        break;
     case GAP_STATE_NO_BOND:
        break;
     default:
        APDATOR_LOG(INFO,"undefined bt_gap_event ");
        break;
     }

    return;
}

void bt_app_gap_get_pairing_key_callback(pairing_key_value_t *bt_pairing_key, UINT8 *fg_accept, void* pv_tag)
{
    APDATOR_LOG(INFO,"bt_pairing_key->pin_code =%s,  key_value = %u, fg_accept = %u", bt_pairing_key->pin_code, bt_pairing_key->key_value, *fg_accept);
    return;
}

void bt_app_gap_inquiry_callback(tBTMW_GAP_DEVICE_INFO *pt_result, void* pv_tag)
{
    if (NULL == pt_result)
    {
        return;
    }

    APDATOR_LOG(INFO,"[GAP]");
    APDATOR_LOG(INFO,"device_kind:     %d", pt_result->device_kind);
    APDATOR_LOG(INFO,"name:     %s", pt_result->device.name);
    APDATOR_LOG(INFO,"cod:      0x%x", pt_result->device.cod);
    APDATOR_LOG(INFO,"bdAddr:   %s", pt_result->device.bdAddr);
}

BOOL bt_app_gap_init(void) {
    MTKRPCAPI_BT_APP_CB_FUNC func;

    memset(&func, 0, sizeof(MTKRPCAPI_BT_APP_CB_FUNC));
    func.bt_event_cb = bt_app_gap_event_callback;
    func.bt_get_pairing_key_cb = bt_app_gap_get_pairing_key_callback;
    func.bt_dev_info_cb = bt_app_gap_inquiry_callback;

    if (a_mtkapi_gap_bt_base_init(&func, NULL) != 0) {
        APDATOR_LOG(ERROR,"btmw_rpc_test_gap_bt_base_init fail");
        return FALSE;
    }

    if (a_mtkapi_bt_gap_on_off(TRUE) != 0) {
        APDATOR_LOG(ERROR,"a_mtkapi_bt_gap_on_off set TRUE in fail");
        return FALSE;
    }

    APDATOR_LOG(INFO,"bt gap init success");

    return TRUE;
}

