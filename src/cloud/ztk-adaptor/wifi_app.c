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

#include "u_common.h"
#include "u_appman.h"
#include "u_app_thread.h"
#include "c_net_wlan.h"

#include "types.h"
#include "adaptor_log.h"
#include "adaptor.h"
#include "app_ctrl.h"
#include "app_ctrl_wifi.h"
#include "u_wifi_setting.h"

EXPORT_SYMBOL void mac_int_convert_to_str(char *mac_int,char *buf,int buf_len) {

    if (mac_int == NULL || buf == NULL || buf_len < 18) {
        APDATOR_LOG(ERROR,"mac_addr OR buf is NULL, OR buf_len(%d) < 18!"
            ,buf_len);
        return;
    }

    snprintf(buf,buf_len,"%02x:%02x:%02x:%02x:%02x:%02x",
        mac_int[0],mac_int[1],mac_int[2],mac_int[3],mac_int[4],mac_int[5]);

    return;
}

EXPORT_SYMBOL void mac_str_convert_to_int(char *mac_str,char *buf,int buf_len) {
    char *str = NULL,ptr = NULL;
    char savePtr_buf[18] = {0};

    if (mac_str == NULL || buf == NULL || buf_len < 6) {
        APDATOR_LOG(ERROR,"mac_addr OR buf is NULL, OR buf_len(%d) < 6!"
            ,buf_len);
        return;
    }

    str = mac_str;
    ptr = strtok_r(str, ":",savePtr_buf);

    while (ptr != NULL) {
        buf[0] = atoi(ptr);
        ptr = strtok_r(NULL, ":",savePtr_buf);
    }

    return;
}

EXPORT_SYMBOL int wifi_get_bssid(char *buf) {

    NET_802_11_BSS_INFO_T strCurrBss;

    APDATOR_LOG(ERROR,"wifi_get_bssid!");

    memset(&strCurrBss, 0, sizeof(NET_802_11_BSS_INFO_T));

    if (c_net_wlan_get_curr_bss(&strCurrBss) != 0) {
        APDATOR_LOG(ERROR,"c_net_wlan_get_curr_bss failed");
        return FALSE;
    }

    memcpy(buf,strCurrBss.t_Bssid,sizeof (strCurrBss.t_Bssid));

    APDATOR_LOG(INFO,"bssid: 0x%02x:0x%02x:0x%02x:0x%02x:0x%02x:0x%02x",
        buf[0],buf[1],buf[2],buf[3],buf[4],buf[5]);

    return ADAPTOR_OK;

}

EXPORT_SYMBOL int wifi_station_connect(const char *ssid,const char *key) {

    APP_CTRL_WIFI_INFO_T wifiInfo;

    APDATOR_LOG(INFO,"wifi_station_connect(ssid=%s key=%s)",ssid,key);

    if (ssid == NULL || key == NULL) {
        APDATOR_LOG(INFO,"wifi_station_connect");
        return ADAPTOR_FAIL;
    }

    memset(&wifiInfo,0,sizeof (APP_CTRL_WIFI_INFO_T));

    if ((strlen(ssid) + 1) > APP_CTRL_WIFI_SSID_MAX_LEN) {
        memcpy(wifiInfo.ssid,ssid,APP_CTRL_WIFI_SSID_MAX_LEN - 1);
    } else {
        memcpy(wifiInfo.ssid,ssid,strlen(ssid) + 1);
    }

    if ((strlen(key) + 1) > APP_CTRL_WIFI_PASSWORD_MAX_LEN) {
        memcpy(wifiInfo.password,key,APP_CTRL_WIFI_PASSWORD_MAX_LEN - 1);
    } else {
        memcpy(wifiInfo.password,key,strlen(key) + 1);
    }

    wifiInfo.auth_mode = AUTH_MODE_WPA;

    return ((app_ctrl_send_msg((void *)&wifiInfo,sizeof (wifiInfo), 
            APP_CTRL_CMD_WIFI_CONNECT, E_APP_MSG_TYPE_APP_CTRL,
            WIFI_SETTING_THREAD_NAME)
            == APP_CTRL_OK) ? ADAPTOR_OK: ADAPTOR_FAIL);

}

EXPORT_SYMBOL int wifi_station_disconnect(void) {

    APDATOR_LOG(INFO,"wifi_station_disconnect");

    return ((app_ctrl_send_msg(NULL,0, APP_CTRL_CMD_WIFI_DISCONNECT,
            E_APP_MSG_TYPE_APP_CTRL,WIFI_SETTING_THREAD_NAME) 
            == APP_CTRL_OK) ? ADAPTOR_OK: ADAPTOR_FAIL);

}
