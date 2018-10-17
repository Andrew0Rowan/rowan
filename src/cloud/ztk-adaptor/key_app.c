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
#include <stdio.h>
#include <string.h>

#include "types.h"
#include "adaptor_log.h"
#include "adaptor.h"
#include "adaptor_cbk.h"
#include "app_ctrl.h"
#include "app_ctrl_key.h"

int mute_flag;

INT32 key_app_event_process(UINT32 keyType,INT8 *data) {

    APP_CTRL_KEY_MSG_T *keyData = (APP_CTRL_KEY_MSG_T *)data;
    UINT32 cloud_key_code = BSP_EVENT_MAX;
    UINT32 income_key_code = 0;

    if (keyData == NULL) {
        APP_CTRL_MSG(ERROR,"keyData is NULL!");
        return ADAPTOR_FAIL;
    }

    APP_CTRL_MSG(ERROR,"keyType = %d key_ratio = %d key_value =%d",
        keyType,keyData->key_ratio,keyData->key_value);

    income_key_code = keyData->key_ratio * keyType;

    switch (income_key_code) {

        case KEY_SHORT_PRESS_KEY_MUTE:
              APP_CTRL_MSG(ERROR," KEY_SHORT_PRESS_KEY_MUTE");
              cloud_key_code = BSP_KEY_VOLUME_MUTE;
              break;
        case KEY_LONG_PRESS_KEY_MUTE:
              APP_CTRL_MSG(ERROR," KEY_LONG_PRESS_KEY_MUTE");
              cloud_key_code = BSP_KEY_VOLUME_MUTE;
              break;
        case KEY_SHORT_PRESS_KEY_MICMUTE:
            {
              APP_CTRL_MSG(ERROR," KEY_SHORT_PRESS_KEY_MICMUTE");
              APP_CTRL_MSG(ERROR," LED MUTE_FLAG = %d\n", mute_flag);
              if (mute_flag == 0) {
                cloud_key_code = BSP_KEY_MICMUTE;
                mute_flag = 1;
              } else {
                cloud_key_code = BSP_KEY_MICUNMUTE;
                mute_flag = 0;
              }
              
              break;
            }
        case KEY_LONG_PRESS_KEY_MICMUTE:
              APP_CTRL_MSG(ERROR," KEY_LONG_PRESS_KEY_MICMUTE");
              cloud_key_code = BSP_KEY_MICMUTE;
              break;
        case KEY_SHORT_PRESS_KEY_SOURCE:
              APP_CTRL_MSG(ERROR," KEY_SHORT_PRESS_KEY_SOURCE");
              cloud_key_code = BSP_KEY_ENTER_AP;
              break;
        case KEY_LONG_PRESS_KEY_SOURCE:
              APP_CTRL_MSG(ERROR," KEY_LONG_PRESS_KEY_SOURCE");
              cloud_key_code = BSP_KEY_EXIT_AP;
              break;
        case KEY_SHORT_PRESS_KEY_VOLUMEDOWN:
              APP_CTRL_MSG(ERROR," KEY_SHORT_PRESS_KEY_VOLUMEDOWN");
              cloud_key_code = BSP_KEY_VOLUME_DOWN;
              break;
        case KEY_LONG_PRESS_KEY_VOLUMEDOWN:
              APP_CTRL_MSG(ERROR," KEY_LONG_PRESS_KEY_VOLUMEDOWN");
              cloud_key_code = BSP_KEY_VOLUME_DOWN;
              break;
        case KEY_SHORT_PRESS_KEY_VOLUMEUP:
              APP_CTRL_MSG(ERROR," KEY_SHORT_PRESS_KEY_VOLUMEUP");
              cloud_key_code = BSP_KEY_VOLUME_UP;
              break;
        case KEY_LONG_PRESS_KEY_VOLUMEUP:
              APP_CTRL_MSG(ERROR," KEY_LONG_PRESS_KEY_VOLUMEUP");
              cloud_key_code = BSP_KEY_VOLUME_UP;
              break;
        case KEY_SHORT_PRESS_KEY_POWER:
              APP_CTRL_MSG(ERROR," KEY_SHORT_PRESS_KEY_POWER");
              cloud_key_code = BSP_KEY_TAP_TO_TALK;
              break;
        case KEY_LONG_PRESS_KEY_POWER:
              APP_CTRL_MSG(ERROR," KEY_LONG_PRESS_KEY_POWER");
              cloud_key_code = BSP_KEY_HOLD_TO_TALK;
        default:
              APP_CTRL_MSG(ERROR," short press INVALID");
              break;
    }

    if (gAdaptorReportEvent) {
        APP_CTRL_MSG(ERROR," gAdaptorReportEvent=%p", gAdaptorReportEvent);
        gAdaptorReportEvent(cloud_key_code,&keyData->key_value,sizeof (UINT32));
    }

    return ADAPTOR_OK;

}

