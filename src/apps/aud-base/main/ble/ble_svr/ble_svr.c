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

#include "u_common.h"

#include "ble.h"
#include "ble_msg.h"

#include "ble_gatt.h"
#include "ble_svr.h"
#include "ble_svr_timer.h"

static UINT32 g_ble_server_init = FALSE;

INT32 ble_server_init(VOID) {

    if (g_ble_server_init != FALSE) {
        BLE_LOG(BLE_LOG_ERR,"ble server has been inited");
        goto BLE_GATT_SERVER_INIT_FAIL;
    }

    /*setup ble server*/
    if (ble_gatt_server_init() != BLE_OK) {
        BLE_LOG(BLE_LOG_ERR,"init bt gatt fail.");
        goto BLE_GATT_SERVER_INIT_FAIL;
    }

    if (ble_queue_init_new() != 0) {
        BLE_LOG(BLE_LOG_ERR,"init bt app msg queue fail.");
        goto BLE_MSG_QUEUE_INIT_FAIL;
    }

    if (ble_server_wifi_info_handler_init() != BLE_OK) {
        BLE_LOG(BLE_LOG_ERR,"ble_server_wifi_info_handler_init fail.");
        goto BLE_WIFI_INFO_HANDLER_INIT_FAIL;
    }

    BLE_LOG(BLE_LOG_INFO,"ble server init success");

    g_ble_server_init = TRUE;

    return BLE_OK;

BLE_WIFI_INFO_HANDLER_INIT_FAIL:
    ble_msg_queue_destroy();
BLE_MSG_QUEUE_INIT_FAIL:
    ble_gatt_server_deinit();
BLE_GATT_SERVER_INIT_FAIL:
    return BLE_FAIL;
}

INT32 ble_server_deinit(VOID) {

    if (g_ble_server_init != TRUE) {
        BLE_LOG(BLE_LOG_INFO,"ble server has been destroy");
        return BLE_OK;
    }

    /*setup ble server*/
    if (ble_gatt_server_deinit() != BLE_OK) {
        BLE_LOG(BLE_LOG_ERR,"init bt gatt fail.");
        return BLE_FAIL;
    }

    ble_msg_queue_destroy();

    ble_server_wifi_info_handler_deinit();

    BLE_LOG(BLE_LOG_INFO,"ble server deinit success");

    g_ble_server_init = FALSE;

    return BLE_OK;
}

INT32 ble_server_open(VOID) {
    BLE_MSG msg;
    BLE_GATT_SERVER_MSG *pBleSvrMsg = NULL;

    /*add app*/
    msg.hdr.event = BLE_SERVER_EVENT;
    msg.hdr.len = sizeof(BLE_GATT_SERVER_MSG) - BLE_GATT_SERVER_MSG_VALUE_MAX_lEN;
    pBleSvrMsg = (BLE_GATT_SERVER_MSG *)&msg.data;
    pBleSvrMsg->type = BLE_GATT_SERVER_START_MSG;
    pBleSvrMsg->event = BLE_GATT_SERVER_START_OP_REG_APP;

    if (ble_send_msg(&msg) != BLE_OK) {
        BLE_LOG(BLE_LOG_ERR,"ble_send_msg fail");
        return;
    }
    BLE_LOG(BLE_LOG_INFO,"ble server open success");

    ble_server_start_timer_start(ble_gatt_server_get_start_timer());

    return BLE_OK;
}

INT32 ble_server_close(VOID) {

    BLE_MSG msg;
    BLE_GATT_SERVER_MSG *pBleSvrMsg = NULL;

    /*close conn*/
    msg.hdr.event = BLE_SERVER_EVENT;
    msg.hdr.len = sizeof(BLE_GATT_SERVER_MSG) - BLE_GATT_SERVER_MSG_VALUE_MAX_lEN;
    pBleSvrMsg = (BLE_GATT_SERVER_MSG *)&msg.data;
    pBleSvrMsg->type = BLE_GATT_SERVER_STOP_MSG;
    pBleSvrMsg->event = BLE_GATT_SERVER_STOP_OP_STOP_SERVER;

    if (ble_send_msg(&msg) != BLE_OK) {
        BLE_LOG(BLE_LOG_ERR,"ble_send_msg fail");
        return;
    }
    BLE_LOG(BLE_LOG_INFO,"ble server close success");

    ble_server_stop_timer_start(ble_gatt_server_get_stop_timer());

    return BLE_OK;
}

BLE_SERVER_STATUS ble_server_status_get(VOID) {

    return ((ble_gatt_server_get_status() == TRUE) ? BLE_SERVER_STATUS_OPEN
        : BLE_SERVER_STATUS_CLOSE);
}

