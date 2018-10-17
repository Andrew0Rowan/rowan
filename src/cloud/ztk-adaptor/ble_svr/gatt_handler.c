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
#include <sys/stat.h>

#include "adaptor_log.h"
#include "mtk_bt_service_gap_wrapper.h"
#include "rw_init_mtk_bt_service.h"
#include "bt_msg.h"
#include "bt_app.h"
#include "gattc_handler.h"
#include "gatts_handler.h"


BT_APP_START_BLE_SERVER gBleSvrInfo;

BOOL bt_app_gatt_init()
{
    if (!bt_app_gattc_init()) {
        APDATOR_LOG(ERROR,"Failed to init GATT client interface");
        return FALSE;
    }
    if (!bt_app_gatts_init()) {
        APDATOR_LOG(ERROR,"Failed to init GATT server interface");
        return FALSE;
    }

    return TRUE;
}

BOOL bt_service_init() {
    chmod("/tmp/mtk_bt_service",
          S_IRUSR|S_IWUSR|S_IXUSR|S_IRGRP|S_IWGRP|S_IXGRP|S_IROTH|S_IWOTH|S_IXOTH);

    a_mtk_bt_service_init();

    sleep(3);

    APDATOR_LOG(INFO,"bt server initialize");

    return TRUE;
}

BOOL bt_app_start_gatt_server(void *data) {
    BT_APP_MSG msg;
    BT_APP_GATTC_MSG_HDR gattc_msg;
    BT_APP_START_BLE_SERVER *pBleServer = (BT_APP_START_BLE_SERVER *)data;

    if (pBleServer == NULL) {
        APDATOR_LOG(ERROR,"pBleServer is NULL.");
        return FALSE;
    }

    memset((void *)&gBleSvrInfo,0,sizeof (gBleSvrInfo));
    memcpy(gBleSvrInfo.app_uuid,pBleServer->app_uuid,BT_LEN_UUID_LEN);
    memcpy(gBleSvrInfo.server_uuid,pBleServer->server_uuid,BT_LEN_UUID_LEN);
    memcpy(gBleSvrInfo.service_uuid,pBleServer->service_uuid,BT_LEN_UUID_LEN);
    memcpy(gBleSvrInfo.char_uuid,pBleServer->char_uuid,BT_LEN_UUID_LEN);
    memcpy(gBleSvrInfo.desc_uuid,pBleServer->desc_uuid,BT_LEN_UUID_LEN);
    memcpy(gBleSvrInfo.device_name,pBleServer->device_name,BT_DEVICE_NAME_LEN);

    APDATOR_LOG(INFO,"app_uuid %s.",gBleSvrInfo.app_uuid);
    APDATOR_LOG(INFO,"server_uuid %s.",gBleSvrInfo.server_uuid);
    APDATOR_LOG(INFO,"service_uuid %s.",gBleSvrInfo.service_uuid);
    APDATOR_LOG(INFO,"char_uuid %s.",gBleSvrInfo.char_uuid);
    APDATOR_LOG(INFO,"desc_uuid %s.",gBleSvrInfo.desc_uuid);
    APDATOR_LOG(INFO,"device_name %s.",gBleSvrInfo.device_name);
#if 1
    /*set device name*/
    if (a_mtkapi_bt_gap_set_name(gBleSvrInfo.device_name) == -1) {
        APDATOR_LOG(ERROR,"set device name fail.");
        return FALSE;
    }
#endif
    /*set client uuid*/
    msg.hdr.event = BT_APP_GATTC_EVENT;
    msg.hdr.len = sizeof(BT_APP_GATTC_MSG_HDR);
    gattc_msg.event = BT_APP_GATTC_REG_CLIENT;
    memcpy((void*)&msg.data.gattc_msg, &gattc_msg, sizeof(BT_APP_GATTC_MSG_HDR));
    
    if (!bt_app_send_msg(&msg)) {
        APDATOR_LOG(ERROR,"bt_app_send_msg fail\n");
        return FALSE;
    }

    return TRUE;
}

BOOL bt_app_stop_gatt_server(void) {

    APDATOR_LOG(ERROR,"stop gatt server.");


    /*stop gatt server*/
    if (!bt_app_gatts_server_stop()){
        APDATOR_LOG(ERROR,"stop gatt server fail.");
        return FALSE;
    }

    memset(&gBleSvrInfo,0,sizeof (gBleSvrInfo));

    return TRUE;
}


BOOL bt_app_gatt_msg_handle(void *data) {

    BT_APP_GATT_MSG_HDR *gatt_msg = (BT_APP_GATT_MSG_HDR *)data;

    if (gatt_msg == NULL) {
        APDATOR_LOG(ERROR, "msg is NULL");
        return FALSE;
    }

    APDATOR_LOG(INFO, " event:%d", gatt_msg->event);

    switch(gatt_msg->event) {
        case BT_APP_GATT_START_SERVER_SUCCESS:
            APDATOR_LOG(INFO, " GATT server start success");
            break;

        case BT_APP_GATT_START_SERVER_FAIL:
            APDATOR_LOG(INFO, " GATT server start fail");
            break;

        default:
            APDATOR_LOG(ERROR, " unknown event:%d", gatt_msg->event);
            break;
    }

    return TRUE;
}

