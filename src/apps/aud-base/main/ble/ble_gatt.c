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

#include "mtk_bt_service_gap_wrapper.h"
#include "rw_init_mtk_bt_service.h"

#include "u_common.h"

#include "ble.h"
#include "ble_svr_timer.h"
#include "ble_svr_wifi_info_get.h"
#include "ble_msg.h"
#include "ble_gatt.h"
#include "ble_gattc.h"
#include "ble_gatts.h"

/*
2018/4/20 yeqin
1. optimize code architecture 
2. add state machine
3. timer control wait for the next optimizition
*/

#define CHAR_CONTENT_WIFI_INFO_CMD "DATA"
#define CHAR_CONTENT_RESET_CMD "RESET"

static BLE_GATTS_INFO gGattsInfo;
BLE_GATTS_MW_ABOUT_INFO gGattsMwInfo;

INT32 ble_gatt_server_init(VOID)
{

    memset(&gGattsInfo,0,sizeof (BLE_GATTS_INFO));

    strcpy(gGattsInfo.app_uuid,"00008888");
    strcpy(gGattsInfo.server_uuid,"00006666");
    strcpy(gGattsInfo.service_uuid,"00001111");
    strcpy(gGattsInfo.char_uuid,"00002222");
    strcpy(gGattsInfo.desc_uuid,"00002902");

/*
        2018-4-20 yeqin
        the content of attri handle should be in a file.
        And this function will be realized in the next optimizition
*/

    /*get service content mem*/
    gGattsInfo.pSeviContent = malloc(1024);
    if (gGattsInfo.pSeviContent == NULL) {
        BLE_LOG(BLE_LOG_ERR,"malloc MEM for pSeviConten fail");
        goto MALLOC_SEVICE_CONTENT_FAIL;
    }
    strcpy(gGattsInfo.pSeviContent,"zelustek support");
    gGattsInfo.service_content_len = 1024;

    /*get char content mem*/
    gGattsInfo.pCharContent = malloc(1024);
    if (gGattsInfo.pCharContent == NULL) {
        BLE_LOG(BLE_LOG_ERR,"malloc MEM for pSeviConten fail");
        goto MALLOC_CHAR_CONTENT_FAIL;
    }
    strcpy(gGattsInfo.pCharContent,"zelustek wifi info");
    gGattsInfo.char_content_len = 1024;

    /*get desc content mem*/
    gGattsInfo.pDescContent = malloc(1024);
    if (gGattsInfo.pDescContent == NULL) {
        BLE_LOG(BLE_LOG_ERR,"malloc MEM for pSeviConten fail");
        goto MALLOC_DESC_CONTENT_FAIL;
    }
    strcpy(gGattsInfo.pDescContent,"made in china,and made in hangzhou");
    gGattsInfo.desc_content_len = 1024;

    BLE_LOG(BLE_LOG_INFO,"app_uuid %s.",gGattsInfo.app_uuid);
    BLE_LOG(BLE_LOG_INFO,"server_uuid %s.",gGattsInfo.server_uuid);
    BLE_LOG(BLE_LOG_INFO,"service_uuid %s.",gGattsInfo.service_uuid);
    BLE_LOG(BLE_LOG_INFO,"char_uuid %s.",gGattsInfo.char_uuid);
    BLE_LOG(BLE_LOG_INFO,"desc_uuid %s.",gGattsInfo.desc_uuid);
    BLE_LOG(BLE_LOG_INFO,"service_content(%d) %s.",gGattsInfo.service_content_len,gGattsInfo.pSeviContent);
    BLE_LOG(BLE_LOG_INFO,"char_content(%d) %s.",gGattsInfo.char_content_len,gGattsInfo.pCharContent);
    BLE_LOG(BLE_LOG_INFO,"desc_content(%d) %s.",gGattsInfo.desc_content_len,gGattsInfo.pDescContent);

    /*creat server start timer*/
    if ((gGattsInfo.bleSvrStartTimer = ble_server_start_timer_init()) == NULL) {
        BLE_LOG(BLE_LOG_ERR,"Failed to creat ble server start timer");
        goto CREAT_SERVER_START_TIMER_FAIL;
    }

    /*creat server stop timer*/
    if ((gGattsInfo.bleSvrStopTimer = ble_server_stop_timer_init()) == NULL) {
        BLE_LOG(BLE_LOG_ERR,"Failed to creat ble server start timer");
        goto CREAT_SERVER_STOP_TIMER_FAIL;
    }

    if (ble_gattc_init() != BLE_OK) {
        BLE_LOG(BLE_LOG_ERR,"Failed to init GATT client interface");
        goto GATT_INIT_FAIL;
    }
    if (ble_gatts_init() != BLE_OK) {
        BLE_LOG(BLE_LOG_ERR,"Failed to init GATT server interface");
        goto GATT_INIT_FAIL;
    }

    /*init server status*/
    gGattsInfo.bleSvrStartStatus = BLE_GATT_SERVER_START_IDLE;
    gGattsInfo.bleSvrStopStatus = BLE_GATT_SERVER_STOP_IDLE;
    gGattsInfo.bleSvrIsOpened = FALSE;

    BLE_LOG(BLE_LOG_ERR,"init GATT server success");

    return BLE_OK;

GATT_INIT_FAIL:
    ble_server_stop_timer_deinit(gGattsInfo.bleSvrStopTimer);
CREAT_SERVER_STOP_TIMER_FAIL:
    ble_server_start_timer_deinit(gGattsInfo.bleSvrStartTimer);
CREAT_SERVER_START_TIMER_FAIL:
    free(gGattsInfo.pDescContent);
MALLOC_DESC_CONTENT_FAIL:
    free(gGattsInfo.pCharContent);
MALLOC_CHAR_CONTENT_FAIL:
    free(gGattsInfo.pSeviContent);
MALLOC_SEVICE_CONTENT_FAIL:
    BLE_LOG(BLE_LOG_ERR,"Fail to init GATT server success");
    return BLE_FAIL;
}

INT32 ble_gatt_server_deinit(VOID)
{
    ble_server_stop_timer_deinit(gGattsInfo.bleSvrStopTimer);
    ble_server_start_timer_deinit(gGattsInfo.bleSvrStartTimer);

    free(gGattsInfo.pDescContent);
    free(gGattsInfo.pCharContent);
    free(gGattsInfo.pSeviContent);

    memset(&gGattsInfo,0,sizeof (BLE_GATTS_INFO));

    BLE_LOG(BLE_LOG_ERR,"deinit GATT server success");

    return BLE_OK;
}

INT32 ble_gatt_server_handle_content_get(UINT32 handle,BLE_SERVER_CONTENT_GET *pData) {

    BLE_LOG(BLE_LOG_INFO,"handle = %d",handle);

    if (pData == NULL) {
        BLE_LOG(BLE_LOG_ERR,"pData is NULL.");
        return BLE_FAIL;
    }

    if (handle == gGattsMwInfo.charHandle) {
        pData->content_str = gGattsInfo.pCharContent;
        pData->length = gGattsInfo.char_content_len;
        pData->pos = &gGattsInfo.char_content_pos;

        BLE_LOG(BLE_LOG_INFO,"charHandle(%s)(%d)",
            gGattsInfo.pCharContent,gGattsInfo.char_content_pos);
    } else if (handle == gGattsMwInfo.servHandle) {
        pData->content_str = gGattsInfo.pSeviContent;
        pData->length = gGattsInfo.service_content_len;
        pData->pos = &gGattsInfo.service_content_pos;

        BLE_LOG(BLE_LOG_INFO,"servHandle(%s)(%d)",
            gGattsInfo.pSeviContent,gGattsInfo.service_content_pos);
    } else if (handle == gGattsMwInfo.descHandle) {
        pData->content_str = gGattsInfo.pDescContent;
        pData->length = gGattsInfo.desc_content_len;
        pData->pos = &gGattsInfo.desc_content_pos;

        BLE_LOG(BLE_LOG_INFO,"descHandle(%s)(%d)",
            gGattsInfo.pDescContent,gGattsInfo.desc_content_pos);
    } else {
        BLE_LOG(BLE_LOG_ERR,"err handle(%d).",handle);
        return BLE_FAIL;
    }

    return BLE_OK;
}

CHAR *ble_gatt_server_get_char_handle_mem(VOID) {

    BLE_LOG(BLE_LOG_ERR,"!!!");

    return gGattsInfo.pCharContent;
}

UINT32 *ble_gatt_server_get_char_handle_mem_len(VOID) {

    BLE_LOG(BLE_LOG_ERR,"!!!");

    return (UINT32 *)&gGattsInfo.char_content_len;
}

INT32 ble_gatt_server_get_status(VOID) {

    BLE_LOG(BLE_LOG_ERR,"opened %d.",
        ((gGattsInfo.bleSvrIsOpened == TRUE) ? 1 : 0));

    return gGattsInfo.bleSvrIsOpened;
}

INT32 ble_gatt_server_get_conn_status(VOID) {

    BLE_LOG(BLE_LOG_ERR,"isConnected %d.",
        ((gGattsMwInfo.isConnected == TRUE) ? 1 : 0));

    return gGattsMwInfo.isConnected;
}

VOID *ble_gatt_server_get_start_timer(VOID) {

    BLE_LOG(BLE_LOG_INFO,"start timer");

    return gGattsInfo.bleSvrStartTimer;
}

VOID *ble_gatt_server_get_stop_timer(VOID) {

    BLE_LOG(BLE_LOG_INFO,"stop timer");

    return gGattsInfo.bleSvrStopTimer;
}

static VOID ble_gatt_server_start_status_machine(UINT32 event,VOID *data) {

    BLE_LOG(BLE_LOG_INFO, " event %d status %d opened %d",
        event,gGattsInfo.bleSvrStartStatus,((gGattsInfo.bleSvrIsOpened == TRUE) ? 1 : 0));

    switch(event) {
        case BLE_GATT_SERVER_START_OP_REG_APP:
            if ((gGattsInfo.bleSvrStopStatus != BLE_GATT_SERVER_STOP_IDLE) ||
                (gGattsInfo.bleSvrIsOpened != FALSE)) {
                BLE_LOG(BLE_LOG_ERR,"ble server has already started or is closing now (status=%d opened %d)",
                    gGattsInfo.bleSvrStopStatus,((gGattsInfo.bleSvrIsOpened == TRUE) ? 1 : 0));
                return;
            }

            if (gGattsInfo.bleSvrStartStatus != BLE_GATT_SERVER_START_IDLE) {
                BLE_LOG(BLE_LOG_ERR,"err operation (event=%d status=%d)",
                    event,gGattsInfo.bleSvrStartStatus);
                return;
            }

            if (ble_gattc_register(gGattsInfo.app_uuid) != BLE_OK) {
                BLE_LOG(BLE_LOG_ERR,"ble_gattc_register fail");
                return BLE_FAIL;
            } else {
                gGattsInfo.bleSvrStartStatus = BLE_GATT_SERVER_START_APP_REGED;
                BLE_LOG(BLE_LOG_INFO,"bleSvrSartStatus = %d",gGattsInfo.bleSvrStartStatus);
            }
            break;
        case BLE_GATT_SERVER_START_OP_REG_SERVER:
            if (gGattsInfo.bleSvrStartStatus != BLE_GATT_SERVER_START_APP_REGED) {
                BLE_LOG(BLE_LOG_ERR,"err operation (event=%d status=%d)",
                    event,gGattsInfo.bleSvrStartStatus);
                return;
            }
            
            if (ble_gatts_register(gGattsInfo.server_uuid) != BLE_OK) {
                BLE_LOG(BLE_LOG_ERR,"Failed to init GATT server interface");
                return BLE_FAIL;
            } else {
                gGattsInfo.bleSvrStartStatus = BLE_GATT_SERVER_START_SERVER_REGED;
                BLE_LOG(BLE_LOG_INFO,"bleSvrSartStatus = %d",gGattsInfo.bleSvrStartStatus);
            }
            break;

        case BLE_GATT_SERVER_START_OP_ADD_SERVICE:
            if (gGattsInfo.bleSvrStartStatus != BLE_GATT_SERVER_START_SERVER_REGED) {
                BLE_LOG(BLE_LOG_ERR,"err operation (event=%d status=%d)",
                    event,gGattsInfo.bleSvrStartStatus);
                return;
            }
            
            if (ble_gatts_server_add_service(gGattsInfo.service_uuid) != BLE_OK){
                BLE_LOG(BLE_LOG_ERR,"ble_gatts_server_add_service fail.");
                return;
            } else {
                gGattsInfo.bleSvrStartStatus = BLE_GATT_SERVER_START_SERVICE_ADDED;
                BLE_LOG(BLE_LOG_INFO,"bleSvrSartStatus = %d",gGattsInfo.bleSvrStartStatus);
            }
            break;

        case BLE_GATT_SERVER_START_OP_ADD_CHAR:
            if (gGattsInfo.bleSvrStartStatus != BLE_GATT_SERVER_START_SERVICE_ADDED) {
                BLE_LOG(BLE_LOG_ERR,"err operation (event=%d status=%d)",
                    event,gGattsInfo.bleSvrStartStatus);
                return;
            }
            
            if (ble_gatts_server_add_char(gGattsInfo.char_uuid) != BLE_OK){
                BLE_LOG(BLE_LOG_ERR,"ble_gatts_server_add_char fail.");
                return;
            } else {
                gGattsInfo.bleSvrStartStatus = BLE_GATT_SERVER_START_CHAR_ADDED;
                BLE_LOG(BLE_LOG_INFO,"bleSvrSartStatus = %d",gGattsInfo.bleSvrStartStatus);
            }
            break;

        case BLE_GATT_SERVER_START_OP_ADD_DESC:
            if (gGattsInfo.bleSvrStartStatus != BLE_GATT_SERVER_START_CHAR_ADDED) {
                BLE_LOG(BLE_LOG_ERR,"err operation (event=%d status=%d)",
                    event,gGattsInfo.bleSvrStartStatus);
                return;
            }
            
            if (ble_gatts_server_add_desc(gGattsInfo.desc_uuid) != BLE_OK){
                BLE_LOG(BLE_LOG_ERR,"ble_gatts_server_add_desc fail.");
                return;
            } else {
                gGattsInfo.bleSvrStartStatus = BLE_GATT_SERVER_START_DESC_ADDED;
                BLE_LOG(BLE_LOG_INFO,"bleSvrSartStatus = %d",gGattsInfo.bleSvrStartStatus);
            }
            break;

        case BLE_GATT_SERVER_START_OP_START_SERVER:
            if (gGattsInfo.bleSvrStartStatus != BLE_GATT_SERVER_START_DESC_ADDED) {
                BLE_LOG(BLE_LOG_ERR,"err operation (event=%d status=%d)",
                    event,gGattsInfo.bleSvrStartStatus);
                return;
            }
            
            if (ble_gatts_server_start() != BLE_OK){
                BLE_LOG(BLE_LOG_ERR,"ble_gatts_server_start fail.");
                return;
            } else {
                gGattsInfo.bleSvrStartStatus = BLE_GATT_SERVER_START_SERVER_STARTED;
                BLE_LOG(BLE_LOG_INFO,"bleSvrSartStatus = %d",gGattsInfo.bleSvrStartStatus);
            }
            break;

        case BLE_GATT_SERVER_START_OP_ENBALE_ADV:
            if (gGattsInfo.bleSvrStartStatus != BLE_GATT_SERVER_START_SERVER_STARTED) {
                BLE_LOG(BLE_LOG_ERR,"err operation (event=%d status=%d)",
                    event,gGattsInfo.bleSvrStartStatus);
                return;
            }
            
            if (ble_gattc_adv_enable() != BLE_OK){
                BLE_LOG(BLE_LOG_ERR,"ble_gattc_adv_enable fail.");
                return;
            } else {
                gGattsInfo.bleSvrStartStatus = BLE_GATT_SERVER_START_ADV_ENABLED;
                BLE_LOG(BLE_LOG_INFO,"bleSvrSartStatus = %d",gGattsInfo.bleSvrStartStatus);
            }
            break;

        case BLE_GATT_SERVER_START_OP_SET_ADVDATA:
            if (gGattsInfo.bleSvrStartStatus != BLE_GATT_SERVER_START_ADV_ENABLED) {
                BLE_LOG(BLE_LOG_ERR,"err operation (event=%d status=%d)",
                    event,gGattsInfo.bleSvrStartStatus);
                return;
            }
            
            if (ble_gattc_adv_set_data(gGattsInfo.service_uuid) != BLE_OK){
                BLE_LOG(BLE_LOG_ERR,"ble_gattc_adv_set_data fail.");
                return;
            } else {
                gGattsInfo.bleSvrStartStatus = BLE_GATT_SERVER_START_IDLE;
                gGattsInfo.bleSvrIsOpened = TRUE;
                ble_server_start_timer_stop(gGattsInfo.bleSvrStartTimer);
                BLE_LOG(BLE_LOG_INFO,"bleSvrSartStatus = %d",gGattsInfo.bleSvrStartStatus);
            }
            break;

        default:
            BLE_LOG(BLE_LOG_ERR, " unknown event:%d",event);
            break;
    }

    return;

}

static VOID ble_gatt_server_stop_status_machine(UINT32 event,VOID *data) {

    BLE_LOG(BLE_LOG_INFO, " event %d status %d opened %d",
        event,gGattsInfo.bleSvrStopStatus,((gGattsInfo.bleSvrIsOpened == TRUE) ? 1 : 0));

    switch(event) {
        case BLE_GATT_SERVER_STOP_OP_STOP_SERVER:
            if (gGattsInfo.bleSvrStopStatus != BLE_GATT_SERVER_STOP_IDLE) {
                BLE_LOG(BLE_LOG_ERR,"err operation (event=%d status=%d)",
                    event,gGattsInfo.bleSvrStopStatus);
                return;
            }

            if (ble_gatts_server_connection_close() != BLE_OK){
                BLE_LOG(BLE_LOG_ERR,"ble_gatts_server_connection_close fail.");
                return;
            }

            if (ble_gatts_server_stop() != BLE_OK){
                BLE_LOG(BLE_LOG_ERR,"ble_gatts_server_stop fail.");
                return;
            } else {
                gGattsInfo.bleSvrStopStatus = BLE_GATT_SERVER_STOP_STOPPED;
                BLE_LOG(BLE_LOG_INFO,"gBleSvrStopStatus = %d",gGattsInfo.bleSvrStopStatus);
            }
            break;

        case BLE_GATT_SERVER_STOP_OP_DELETET_SERVER:
            if (gGattsInfo.bleSvrStopStatus != BLE_GATT_SERVER_STOP_STOPPED) {
                BLE_LOG(BLE_LOG_ERR,"err operation (event=%d status=%d)",
                    event,gGattsInfo.bleSvrStopStatus);
                return;
            }

            if (ble_gatts_server_delete() != BLE_OK){
                BLE_LOG(BLE_LOG_ERR,"ble_gatts_server_delete fail.");
                return;
            } else {
                gGattsInfo.bleSvrStopStatus = BLE_GATT_SERVER_STOP_DELETETED;
                BLE_LOG(BLE_LOG_INFO,"gBleSvrStopStatus = %d",gGattsInfo.bleSvrStopStatus);
            }
            break;

        case BLE_GATT_SERVER_STOP_OP_UNREG:
            if (gGattsInfo.bleSvrStopStatus != BLE_GATT_SERVER_STOP_DELETETED) {
                BLE_LOG(BLE_LOG_ERR,"err operation (event=%d status=%d)",
                    event,gGattsInfo.bleSvrStopStatus);
                return;
            }

            if (ble_gattc_unregister() != BLE_OK) {
                BLE_LOG(BLE_LOG_ERR,"Failed to unregister GATT client interface");
                return;
            }

            if (ble_gatts_unregister() != BLE_OK) {
                BLE_LOG(BLE_LOG_ERR,"Failed to unregister GATT server interface");
                return;
            }

            gGattsInfo.bleSvrStopStatus = BLE_GATT_SERVER_STOP_IDLE;
            gGattsInfo.bleSvrIsOpened = FALSE;
            ble_server_stop_timer_stop(gGattsInfo.bleSvrStopTimer);
            BLE_LOG(BLE_LOG_INFO,"gBleSvrStopStatus = %d",gGattsInfo.bleSvrStopStatus);
            break;

        default:
            BLE_LOG(BLE_LOG_ERR, " unknown event:%d",event);
            break;
    }

    return BLE_OK;

}

static VOID ble_gatt_server_misc_msg_handle(UINT32 event,VOID *data) {
    BLE_SERVER_READ_REQUEST *pReadRequest = NULL;
    BLE_SERVER_CONTENT_GET server_content = {0};
    BLE_SERVER_WRITE_REQUEST *pWriteRequest = NULL;
    INT32 status = 0;
    UINT32 wifi_info_cmd_len = strlen(CHAR_CONTENT_WIFI_INFO_CMD);

    BLE_LOG(BLE_LOG_INFO, " event %d status %d opened %d",
        event,gGattsInfo.bleSvrStopStatus,((gGattsInfo.bleSvrIsOpened == TRUE) ? 1 : 0));

    switch(event) {
        case BLE_GATT_SERVER_MISC_OP_READ_REQUEST:
            pReadRequest = (BLE_SERVER_READ_REQUEST *)data;

            if (pReadRequest == NULL) {
                BLE_LOG(BLE_LOG_ERR,"pReadRequest is NULL.");
                return;
            }

            if (ble_gatt_server_handle_content_get(pReadRequest->attr_handle,&server_content) != BLE_OK){
                BLE_LOG(BLE_LOG_ERR,"ble_gatt_server_handle_content_get fail.");
                status = -1;
            }

            if (pReadRequest->offset >= server_content.length) {
                BLE_LOG(BLE_LOG_ERR,"read override.");
                status = -1;
            }

            BLE_LOG(BLE_LOG_INFO, " GATT server READ REQUEST(%s)(%d %d)",
                server_content.content_str,pReadRequest->offset,server_content.length);

            if (ble_gatts_server_read_response(pReadRequest->attr_handle,pReadRequest->trans_id,
                (server_content.content_str + pReadRequest->offset),
                strlen(server_content.content_str + pReadRequest->offset),status) != BLE_OK){
                BLE_LOG(BLE_LOG_ERR,"ble_gatts_server_read_response fail.");
                return;
            }
            break;

        case BLE_GATT_SERVER_MISC_OP_WRITE_REQUEST:
            pWriteRequest = (BLE_SERVER_WRITE_REQUEST *)data;

            if (pWriteRequest == NULL) {
                BLE_LOG(BLE_LOG_ERR,"pReadRequest is NULL.");
                return;
            }

            if (ble_gatt_server_handle_content_get(pWriteRequest->attr_handle,&server_content) != BLE_OK){
                BLE_LOG(BLE_LOG_ERR,"ble_gatt_server_handle_content_get fail.");
                status = -1;
            }

            BLE_LOG(BLE_LOG_INFO, " GATT server WRITE REQUEST(%s)(%s)(%d %d)",
                server_content.content_str,pWriteRequest->value,pWriteRequest->offset,pWriteRequest->length);

            if (strncmp(pWriteRequest->value,CHAR_CONTENT_WIFI_INFO_CMD,
                    wifi_info_cmd_len) == 0) {
                if (pWriteRequest->offset == 0) {
                    BLE_LOG(BLE_LOG_INFO,"offset is 0.");
                    if ((pWriteRequest->length + *(server_content.pos)) >= server_content.length) {
                            BLE_LOG(BLE_LOG_ERR,"write override(%d %d).",
                                *(server_content.pos),pWriteRequest->length);
                            status = -1;
                    } else {
                        BLE_LOG(BLE_LOG_INFO,"before write data(%s)(%s)(%d) in.",server_content.content_str,
                            (pWriteRequest->value + wifi_info_cmd_len),
                            *(server_content.pos));

                        memcpy((server_content.content_str + *(server_content.pos)),
                            (pWriteRequest->value + wifi_info_cmd_len),
                            (pWriteRequest->length - wifi_info_cmd_len));

                        *(server_content.pos) = *(server_content.pos) + 
                            pWriteRequest->length - wifi_info_cmd_len;

                        BLE_LOG(BLE_LOG_INFO,"after write data(%s)(%s)(%d) in.",server_content.content_str,
                            (pWriteRequest->value + wifi_info_cmd_len),*(server_content.pos));
                    }
                } else {
                    BLE_LOG(BLE_LOG_INFO,"offset is not 0.");
                    if ((pWriteRequest->length + pWriteRequest->offset) >= server_content.length) {
                            BLE_LOG(BLE_LOG_ERR,"write override(%d %d).",
                                pWriteRequest->offset,pWriteRequest->length);
                            status = -1;
                    } else {
                        BLE_LOG(BLE_LOG_INFO,"before write data(%s)(%s)(%d) in.",server_content.content_str,
                            (pWriteRequest->value + wifi_info_cmd_len),*(server_content.pos));

                        memcpy((server_content.content_str + pWriteRequest->offset),
                            (pWriteRequest->value + strlen(CHAR_CONTENT_WIFI_INFO_CMD)),
                            (pWriteRequest->length - strlen(CHAR_CONTENT_WIFI_INFO_CMD)));

                        *(server_content.pos) = pWriteRequest->offset + 
                            pWriteRequest->length - wifi_info_cmd_len;

                        BLE_LOG(BLE_LOG_INFO,"after write data(%s)(%s)(%d) in.",server_content.content_str,
                            (pWriteRequest->value + wifi_info_cmd_len),*(server_content.pos));
                    }
                }

                ble_server_wifi_info_trigger_read();
            } else if (strncmp(pWriteRequest->value,CHAR_CONTENT_RESET_CMD,
                    strlen(CHAR_CONTENT_RESET_CMD)) == 0) {
                memset(gGattsInfo.pCharContent,0,gGattsInfo.char_content_len);
                gGattsInfo.char_content_pos = 0;
            }

            if (pWriteRequest->need_rsp && 
                ble_gatts_server_write_response(pWriteRequest->attr_handle,
                    pWriteRequest->trans_id,NULL,0,status) != BLE_OK){
                BLE_LOG(BLE_LOG_ERR,"ble_gatts_server_write_response fail.");
                return;
            } 

            break;

        case BLE_GATT_SERVER_MISC_OP_GET_CONN_INFO:
            BLE_LOG(BLE_LOG_INFO, " GATT server get conn info");
            if (ble_gatts_server_get_conn_info() != BLE_OK){
                BLE_LOG(BLE_LOG_ERR,"ble_gatts_server_get_conn_info fail.");
                return;
            } 
            break;

        default:
            BLE_LOG(BLE_LOG_ERR, " unknown event:%d",event);
            break;
    }

    return BLE_OK;

}

INT32 ble_gatt_server_msg_handle(BLE_GATT_SERVER_MSG *data) {

    BLE_GATT_SERVER_MSG *pMsg = (BLE_GATT_SERVER_MSG *)data;

    if (pMsg == NULL) {
        BLE_LOG(BLE_LOG_ERR, "msg is NULL");
        return BLE_FAIL;
    }

    BLE_LOG(BLE_LOG_INFO, " event:%d type:%d ", pMsg->event,pMsg->type);

    switch(pMsg->type) {
        case BLE_GATT_SERVER_START_MSG:
            BLE_LOG(BLE_LOG_INFO, " GATT server start");
            ble_gatt_server_start_status_machine(pMsg->event,pMsg->value);
            break;

        case BLE_GATT_SERVER_STOP_MSG:
            BLE_LOG(BLE_LOG_INFO, " GATT server close");
            ble_gatt_server_stop_status_machine(pMsg->event,pMsg->value);
            break;

        case BLE_GATT_SERVER_MISC_MSG:
            BLE_LOG(BLE_LOG_INFO, " GATT server misc msg");
            ble_gatt_server_misc_msg_handle(pMsg->event,pMsg->value);
            break;

        default:
            BLE_LOG(BLE_LOG_ERR, " unknown type:%d",pMsg->type);
            break;
    }

    return BLE_OK;
}

