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

#include <stddef.h>
#include <pthread.h>
#include <string.h>
#include <errno.h>
#include <sys/prctl.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>

#include "u_os.h"
#include "u_common.h"
#include "u_amb.h"
#include "u_am.h"
#include "u_common.h"
#include "u_timerd.h"
#include "u_app_thread.h"
#include "wifi_setting.h"

#include "ble.h"
#include "ble_gatt.h"

#define WIFI_SSID_PWD_INFO_PREAMBLE "1111("
#define WIFI_SSID_PWD_INFO_TAIL ")2222"

#define WIFI_SSID_FORMAT "SSID:"
#define WIFI_PWD_FORMAT "PWD:"
#define WIFI_SSID_PWD_INFO_SEPARATOR ";"

#define WIFI_SSID_lEN (NET_802_11_MAX_LEN_SSID+1)
#define WIFI_PWD_lEN (WLAN_KEY_MAX_LEN)

static CHAR *pWiFiSsidMem = NULL;
static CHAR *pWiFiPwdMem = NULL;
static pthread_t s_ble_svr_wifi_info_reading_thread;
static pthread_mutex_t s_ble_svr_wifi_info_reading_mutex;
static pthread_cond_t s_ble_svr_wifi_info_reading_cond;

extern INT32 ble_app_send_msg_to_others(VOID *data,SIZE_T len, int event, int app_uid,char *app_name);

static INT32 ble_server_wifi_ssid_pwd_resolve(CHAR *pStr,CHAR *pSsid,CHAR *pPwd) {

    CHAR *tmp_str = pStr;
    CHAR *ssid_str = NULL,*pwd_str = NULL,*tail_str = NULL,*saveptr = NULL;

    if (tmp_str == NULL) {
        BLE_LOG(BLE_LOG_ERR,"pStr is NULL.");
        return BLE_FAIL;
    }

    BLE_LOG(BLE_LOG_INFO,"pStr %s.",tmp_str);

    if (strncmp(tmp_str,WIFI_SSID_PWD_INFO_PREAMBLE,strlen(WIFI_SSID_PWD_INFO_PREAMBLE)) != 0) {
        BLE_LOG(BLE_LOG_ERR,"pStr is INVALID(%s).",tmp_str);
        return BLE_FAIL;
    }

    if ((tail_str = strstr(tmp_str,WIFI_SSID_PWD_INFO_TAIL)) == NULL) {
        BLE_LOG(BLE_LOG_ERR,"pStr is INVALID(%s).",tmp_str);
        return BLE_FAIL;
    }

    ssid_str = strtok_r((tmp_str + strlen(WIFI_SSID_PWD_INFO_PREAMBLE)),
        WIFI_SSID_PWD_INFO_SEPARATOR,&saveptr);

    if ((ssid_str != NULL) && (saveptr != NULL)) {
        BLE_LOG(BLE_LOG_INFO,"ssid_str(%s).pwd_str(%s)",ssid_str,saveptr);

        if (strncmp(ssid_str,WIFI_SSID_FORMAT,strlen(WIFI_SSID_FORMAT)) == 0) {
            if (strlen(ssid_str+strlen(WIFI_SSID_FORMAT)) >= WIFI_SSID_lEN) {
                BLE_LOG(BLE_LOG_INFO,"SSID STRING is too long(%s).",ssid_str);
                return BLE_FAIL;
            }

            strcpy(pSsid,(ssid_str+strlen(WIFI_SSID_FORMAT)));
            BLE_LOG(BLE_LOG_INFO,"pSsid %s.",pSsid);
        } else {
            BLE_LOG(BLE_LOG_INFO,"INVALID SSID STRING(%s).",ssid_str);
            return BLE_FAIL;
        }

        if (strncmp(saveptr,WIFI_PWD_FORMAT,strlen(WIFI_PWD_FORMAT)) == 0) {

            pwd_str = strtok_r(NULL,WIFI_SSID_PWD_INFO_SEPARATOR,&saveptr);

            if (strlen(pwd_str+strlen(WIFI_PWD_FORMAT)) >= WIFI_PWD_lEN) {
                BLE_LOG(BLE_LOG_INFO,"PWD STRING is too long(%s).",pwd_str);
                return BLE_FAIL;
            }

            strcpy(pPwd,(pwd_str+strlen(WIFI_PWD_FORMAT)));
            BLE_LOG(BLE_LOG_INFO,"pPwd %s.",pPwd);
        } else {
            BLE_LOG(BLE_LOG_INFO,"INVALID PWD STRING(%s).",saveptr);
            return BLE_FAIL;
        }
    } else {
        BLE_LOG(BLE_LOG_INFO,"INVALID SSID STRING(%s).",tmp_str);
        return BLE_FAIL;
    }

    return BLE_OK;

}

static INT32 ble_server_wifi_station_connect(const CHAR *ssid,const CHAR *key) {

    CONNECT_AP_INFO wifiInfo;

    BLE_LOG(BLE_LOG_INFO,"ble_server_wifi_station_connect(ssid=%s key=%s)",ssid,key);

    if (ssid == NULL || key == NULL) {
        BLE_LOG(BLE_LOG_ERR,"ssid or key is NULL");
        return BLE_FAIL;
    }

    memset(&wifiInfo,0,sizeof (CONNECT_AP_INFO));

    strcpy(wifiInfo.ap_name,ssid);
    strcpy(wifiInfo.ap_password,key);

    wifiInfo.ap_authmode = AUTH_MODE_WPA;

    return ble_app_send_msg_to_others((VOID *)&wifiInfo,sizeof (wifiInfo),
            BLE_TO_WIFI_CMD_WIFI_CONNECT, E_APP_MSG_TYPE_BLE,
            WIFI_SETTING_THREAD_NAME);

}

static INT32 ble_server_wifi_info_handle(CHAR *pData) {

    if (pData == NULL) {
        BLE_LOG(BLE_LOG_ERR,"pStr is NULL.");
        return BLE_FAIL;
    }

    BLE_LOG(BLE_LOG_INFO,"enter !!!");

    if (ble_server_wifi_ssid_pwd_resolve(pData,pWiFiSsidMem,pWiFiPwdMem) != BLE_OK) {
        BLE_LOG(BLE_LOG_ERR,"ble_server_wifi_ssid_pwd_resolve FAIL.");
        return BLE_FAIL;
    }

    ble_server_wifi_station_connect(pWiFiSsidMem,pWiFiPwdMem);

    BLE_LOG(BLE_LOG_INFO,"SUCCSS !!!");

    return BLE_OK;

}

static INT32 ble_server_wifi_info_reading(VOID) {

    BLE_LOG(BLE_LOG_INFO,"ble_server_wifi_info_reading start!");

    while (1) {

        pthread_mutex_lock(&s_ble_svr_wifi_info_reading_mutex);

        pthread_cond_wait(&s_ble_svr_wifi_info_reading_cond, &s_ble_svr_wifi_info_reading_mutex);

        ble_server_wifi_info_handle(ble_gatt_server_get_char_handle_mem());

        pthread_mutex_unlock(&s_ble_svr_wifi_info_reading_mutex);

    }

    BLE_LOG(BLE_LOG_INFO,"ble_server_wifi_info_reading over!");

    return AEER_OK;
}

static INT32 ble_server_wifi_info_reading_thread(VOID)
{
    INT32 i4_ret;
    pthread_attr_t t_attr;

    i4_ret = pthread_attr_init(&t_attr);
    if(0 != i4_ret) {
        BLE_LOG(BLE_LOG_ERR,"pthread_attr_init error!\n");
        goto ATTR_INIT_ERR;
    }

    i4_ret = pthread_attr_setdetachstate(&t_attr, PTHREAD_CREATE_DETACHED);
    if(0 != i4_ret) {
        BLE_LOG(BLE_LOG_ERR,"setdetachstate fail !\n");
        goto MUTEX_INIT_ERR;
    }

    i4_ret = pthread_mutex_init(&s_ble_svr_wifi_info_reading_mutex, NULL);
    if(0 != i4_ret) {
        BLE_LOG(BLE_LOG_ERR,"mutex init failed!\n");
        goto MUTEX_INIT_ERR;
    }

    i4_ret = pthread_cond_init(&s_ble_svr_wifi_info_reading_cond, NULL);
    if(0 != i4_ret) {
        BLE_LOG(BLE_LOG_ERR,"cond init failed!\n");
        goto COND_INIT_ERR;
    }

    i4_ret = pthread_create(&s_ble_svr_wifi_info_reading_thread, &t_attr, ble_server_wifi_info_reading, NULL);
    if(0 != i4_ret) {
        BLE_LOG(BLE_LOG_ERR,"pthread_create error!\n");
        goto ATTR_SET_ERR;
    }

    pthread_attr_destroy(&t_attr);
    return BLE_OK;

ATTR_SET_ERR:
    pthread_cond_destroy(&s_ble_svr_wifi_info_reading_cond);
COND_INIT_ERR:
    pthread_mutex_destroy(&s_ble_svr_wifi_info_reading_mutex);
MUTEX_INIT_ERR:
    pthread_attr_destroy(&t_attr);
ATTR_INIT_ERR:

    return BLE_FAIL;
}

VOID ble_server_wifi_info_trigger_read(VOID) {

    BLE_LOG(BLE_LOG_INFO,"enter !!!");

    pthread_cond_signal(&s_ble_svr_wifi_info_reading_cond);

    return;

}

INT32 ble_server_wifi_info_handler_init(VOID) {

    INT32 i4_ret = 0;
    pthread_attr_t attr;

    BLE_LOG(BLE_LOG_INFO,"!!!");

    pWiFiSsidMem = malloc(WIFI_SSID_lEN);
    if (pWiFiSsidMem == NULL) {
        BLE_LOG(BLE_LOG_INFO,"pWiFiSsidMem is NULL");
        return BLE_FAIL;
    }

    pWiFiPwdMem = malloc(WIFI_PWD_lEN);
    if (pWiFiPwdMem == NULL) {
        BLE_LOG(BLE_LOG_INFO,"pWiFiPwdMem is NULL");
        free(pWiFiSsidMem);
        return BLE_FAIL;
    }

    memset(pWiFiSsidMem,0,WIFI_SSID_lEN);
    memset(pWiFiPwdMem,0,WIFI_PWD_lEN);

    if (ble_server_wifi_info_reading_thread() != BLE_OK) {
        BLE_LOG(BLE_LOG_INFO,"ble_server_wifi_info_reading_thread FAIL");
        free(pWiFiSsidMem);
        free(pWiFiPwdMem);
        return BLE_FAIL;
    }

    BLE_LOG(BLE_LOG_INFO,"success !!!");

    return BLE_OK;

}

INT32 ble_server_wifi_info_handler_deinit(VOID) {

    BLE_LOG(BLE_LOG_INFO,"!!!");

    free(pWiFiSsidMem);
    free(pWiFiPwdMem);

    pthread_cancel(s_ble_svr_wifi_info_reading_thread);
    pthread_cond_destroy(&s_ble_svr_wifi_info_reading_cond);
    pthread_mutex_destroy(&s_ble_svr_wifi_info_reading_mutex);
    
    BLE_LOG(BLE_LOG_INFO,"success !!!");

    return BLE_OK;

}
