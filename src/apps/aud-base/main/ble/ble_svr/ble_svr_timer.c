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

#include "u_os.h"
#include "u_common.h"
#include "u_amb.h"
#include "u_am.h"
#include "u_common.h"
#include "u_timerd.h"
#include "u_app_thread.h"

#include "ble.h"
#include "ble_svr_timer.h"

VOID *ble_server_start_timer_init(VOID)
{
    TIMER_TYPE_T *pTimer = NULL;

    pTimer = (TIMER_TYPE_T *)malloc(sizeof (TIMER_TYPE_T));
    if (pTimer == NULL) {
        BLE_LOG(BLE_LOG_ERR,"*timerData is NULL.\r\n");
        return NULL;
    }

    pTimer->e_flags = X_TIMER_FLAG_ONCE;
    pTimer->ui4_delay = BLE_SERVER_START_TIME;
    if (u_timer_create(&pTimer->h_timer) != OSR_OK) {
        BLE_LOG(BLE_LOG_ERR,"Failed to creat ble server start timer");
        free(pTimer);
        return NULL;
    }

    BLE_LOG(BLE_LOG_ERR,"ble server start timer init success.\r\n");

    return (VOID *)pTimer;
}

INT32 ble_server_start_timer_deinit(VOID *timerData)
{
    TIMER_TYPE_T *pTimer = (TIMER_TYPE_T *)timerData;

    if (pTimer == NULL) {
        BLE_LOG(BLE_LOG_ERR,"malloc MEM fail\r\n");
        return BLE_FAIL;
    }

    if (u_timer_delete(pTimer->h_timer) != OSR_OK) {
        BLE_LOG(BLE_LOG_ERR,"Failed to deinit ble server start timer");
            return BLE_FAIL;
    }

    free(pTimer);

    BLE_LOG(BLE_LOG_ERR,"ble server start timer deinit success.\r\n");

    return BLE_OK;
}

INT32 ble_server_start_timer_start(VOID *timerData)
{
    TIMER_TYPE_T *pTimer = (TIMER_TYPE_T *)timerData;
    UINT32 event = 0;
    HANDLE_T h_app = NULL_HANDLE;

    if (pTimer == NULL) {
        BLE_LOG(BLE_LOG_ERR,"pTimer is NULL.\r\n");
        return BLE_FAIL;
    }

    if(pTimer->h_timer == NULL_HANDLE)
    {
        BLE_LOG(BLE_LOG_ERR,"server start timer hasn't created.\r\n");
        return BLE_FAIL;
    }

    if(u_am_get_app_handle_from_name(&h_app, BLE_THREAD_NAME) != AMR_OK) {
       BLE_LOG(BLE_LOG_ERR,"get handle fail!\n");
       return BLE_FAIL;
    }

    event = BLE_SERVER_TIMER_EVENT_START_TIMEOUT;

    if (u_timer_start(h_app,pTimer,
                  (void *)&event,sizeof(event)) != OSR_OK) {
        BLE_LOG(BLE_LOG_ERR,"Failed to start ble server start timer");
        return BLE_FAIL;
    }

    BLE_LOG(BLE_LOG_ERR,"ble server start timer start success.\r\n");

    return BLE_OK;
}

INT32 ble_server_start_timer_stop(VOID *timerData)
{
    TIMER_TYPE_T *pTimer = (TIMER_TYPE_T *)timerData;

    if (pTimer == NULL) {
        BLE_LOG(BLE_LOG_ERR,"pTimer is NULL.\r\n");
        return BLE_FAIL;
    }

    if(pTimer->h_timer == NULL_HANDLE)
    {
        BLE_LOG(BLE_LOG_ERR,"server start timer hasn't created.\r\n");
        return BLE_FAIL;
    }

    if (u_timer_stop(pTimer->h_timer) != OSR_OK) {
        BLE_LOG(BLE_LOG_ERR,"Failed to stop ble server start timer");
        return BLE_FAIL;
    }

    BLE_LOG(BLE_LOG_ERR,"ble server start timer stop success.\r\n");

    return BLE_OK;
}

VOID *ble_server_stop_timer_init(VOID)
{
    TIMER_TYPE_T *pTimer = NULL;

    pTimer = (TIMER_TYPE_T *)malloc(sizeof (TIMER_TYPE_T));
    if (pTimer == NULL) {
        BLE_LOG(BLE_LOG_ERR,"*malloc MEM fail.\r\n");
        return NULL;
    }

    pTimer->e_flags = X_TIMER_FLAG_ONCE;
    pTimer->ui4_delay = BLE_SERVER_STOP_TIME;
    if (u_timer_create(&pTimer->h_timer) != OSR_OK) {
        BLE_LOG(BLE_LOG_ERR,"Failed to creat ble server stop timer");
        free(pTimer);
        return NULL;
    }

    BLE_LOG(BLE_LOG_ERR,"ble server stop timer init success.\r\n");

    return (VOID *)pTimer;
}

INT32 ble_server_stop_timer_deinit(VOID *timerData)
{
    TIMER_TYPE_T *pTimer = (TIMER_TYPE_T *)timerData;

    if (pTimer == NULL) {
        BLE_LOG(BLE_LOG_ERR,"pTimer is NULL.\r\n");
        return BLE_FAIL;
    }

    if (u_timer_delete(pTimer->h_timer) != OSR_OK) {
        BLE_LOG(BLE_LOG_ERR,"Failed to deinit ble server stop timer");
            return BLE_FAIL;
    }

    free(pTimer);

    BLE_LOG(BLE_LOG_ERR,"ble server stop timer deinit success.\r\n");

    return BLE_OK;
}

INT32 ble_server_stop_timer_start(VOID *timerData)
{
    TIMER_TYPE_T *pTimer = (TIMER_TYPE_T *)timerData;
    UINT32 event = 0;
    HANDLE_T h_app = NULL_HANDLE;

    if (pTimer == NULL) {
        BLE_LOG(BLE_LOG_ERR,"pTimer is NULL.\r\n");
        return BLE_FAIL;
    }

    if(pTimer->h_timer == NULL_HANDLE)
    {
        BLE_LOG(BLE_LOG_ERR,"server stop timer hasn't created.\r\n");
        return BLE_FAIL;
    }

    if(u_am_get_app_handle_from_name(&h_app, BLE_THREAD_NAME) != AMR_OK) {
       BLE_LOG(BLE_LOG_ERR,"get handle fail!\n");
       return BLE_FAIL;
    }

    event = BLE_SERVER_TIMER_EVENT_STOP_TIMEOUT;

    if (u_timer_start(h_app,pTimer,
                  (void *)&event,sizeof(event)) != OSR_OK) {
        BLE_LOG(BLE_LOG_ERR,"Failed to start ble server stop timer");
        return BLE_FAIL;
    }

    BLE_LOG(BLE_LOG_ERR,"ble server stop timer start success.\r\n");

    return BLE_OK;
}

INT32 ble_server_stop_timer_stop(VOID *timerData)
{
    TIMER_TYPE_T *pTimer = (TIMER_TYPE_T *)timerData;

    if (pTimer == NULL) {
        BLE_LOG(BLE_LOG_ERR,"pTimer is NULL.\r\n");
        return BLE_FAIL;
    }

    if(pTimer->h_timer == NULL_HANDLE)
    {
        BLE_LOG(BLE_LOG_ERR,"server stop timer hasn't created.\r\n");
        return BLE_FAIL;
    }

    if (u_timer_stop(pTimer->h_timer) != OSR_OK) {
        BLE_LOG(BLE_LOG_ERR,"Failed to stop ble server stop timer");
        return BLE_FAIL;
    }

    BLE_LOG(BLE_LOG_ERR,"ble server stop timer stop success.\r\n");

    return BLE_OK;
}

