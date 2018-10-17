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
#include <stdlib.h>
#include <stdio.h>

#include "adaptor_log.h"
#include "adaptor.h"
#include "u_common.h"
#include "types.h"
#include "gap_handler.h"
#include "gatt_handler.h"
#include "gatts_handler.h"
#include "bt_app.h"

static int gBtDeviceIsOpened = 0;

EXPORT_SYMBOL int ble_send_packet(char *data,int length) {
    
    if (!bt_app_gatts_send_indication(data,length)) {
        APDATOR_LOG(INFO,"ble send indication fail");
        return ADAPTOR_FAIL;
    }
    
    APDATOR_LOG(INFO,"ble send indication success");
    
    return ADAPTOR_OK;
}

EXPORT_SYMBOL int ble_server_open(BT_APP_START_BLE_SERVER *pBleServer) {

    if (!bt_app_start_gatt_server(pBleServer)) {
        APDATOR_LOG(INFO,"ble server open fail");
        return ADAPTOR_FAIL;
    }

    APDATOR_LOG(INFO,"ble server open success");

    return ADAPTOR_OK;
}

EXPORT_SYMBOL int ble_server_close(void) {

    if (!bt_app_stop_gatt_server()) {
        APDATOR_LOG(INFO,"ble server close fail");
        return ADAPTOR_FAIL;
    }

    APDATOR_LOG(INFO,"ble server close success");

    return ADAPTOR_OK;
}

EXPORT_SYMBOL int bt_status_get(void) {

    APDATOR_LOG(INFO,"status = %d",gBtDeviceIsOpened);

    return gBtDeviceIsOpened;
}

EXPORT_SYMBOL int bt_setup(void) {
#if 0
    /*setup bt stack*/
    if (system("btservice &") == -1) {
        APDATOR_LOG(ERROR,"start bt stack fail.");
        return ADAPTOR_FAIL;
    }
#endif
    /*setup bt mw*/
    if (!bt_service_init()) {
        APDATOR_LOG(ERROR,"init bt mw fail.");
        return ADAPTOR_FAIL;
    }

    /*setup bt profile*/
    if (!bt_app_gap_init()) {
        APDATOR_LOG(ERROR,"init bt gap fail.");
        return ADAPTOR_FAIL;
    }

    if (!bt_app_gatt_init()) {
        APDATOR_LOG(ERROR,"init bt gatt fail.");
        return ADAPTOR_FAIL;
    }

    if (bt_app_queue_init_new() != 0) {
        APDATOR_LOG(ERROR,"init bt app msg queue fail.");
        return ADAPTOR_FAIL;
    }

    gBtDeviceIsOpened = 1;

    return ADAPTOR_OK;
}

EXPORT_SYMBOL int bt_destroy(void) {
#if 0
    /*kill bt service*/
    if (system("killall btservice") == -1) {
        APDATOR_LOG(ERROR,"kill bt stack fail.");
        return ADAPTOR_FAIL;
    }
#endif
    /*destroy msg queue*/
    bt_app_msg_queue_destroy();

    gBtDeviceIsOpened = 0;

    return ADAPTOR_OK;
}

