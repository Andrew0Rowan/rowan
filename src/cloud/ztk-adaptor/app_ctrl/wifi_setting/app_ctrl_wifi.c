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
/* middleware level*/
#include "u_appman.h"
#include "u_aee.h"
#include "u_cli.h"
#include "u_misc.h"

#include "u_dbg.h"
#include "u_os.h"

/* application level */
#include "u_amb.h"
#include "u_am.h"
#include "u_common.h"
/* private */
#include "wifi_setting.h"
#include "c_net_config.h"
#include "u_app_thread.h"
#include "u_acfg.h"
#include "u_app_def.h"

#include "app_ctrl.h"
#include "app_ctrl_wifi.h"

APP_CTRL_WIFI_INFO_T wifiInfo;
extern CHAR   wifi_ConnectSsid[NET_802_11_MAX_LEN_SSID+1];
extern CHAR   wifi_ConnectBssid[NET_802_11_MAX_LEN_BSSID+1];
extern CHAR   wifi_ConnectPwd[WLAN_KEY_MAX_LEN+1];
extern INT32  wifi_AuthMode;
extern INT32  fg_wifi_connect_id;


extern INT32 start_wifi_connect(void);
extern void wifi_connect_over(void);

static VOID app_ctrl_wifi_send_event(VOID *data,SIZE_T len,UINT32 msgType) {
    HANDLE_T h_app = NULL_HANDLE;
    INT32 i4_ret = 0;

    if(0 != (i4_ret = u_am_get_app_handle_from_name(&h_app, APP_CTRL_THREAD_NAME))) {
       WIFI_ERR("<APP_CTRL_WIFI> get handle fail!,i4_ret=%ld",i4_ret);
       return;
    }

    i4_ret = u_app_send_appmsg(h_app,
                              E_APP_MSG_TYPE_WIFI_SETTING,
                              MSG_FROM_WIFI_SETTING,
                              msgType,
                              data,
                              len);
    if(0 == i4_ret) {
       WIFI_MSG("<APP_CTRL_WIFI> success!");
    } else {
       WIFI_ERR("<APP_CTRL_WIFI> fail!",i4_ret);
    }

    return;
}

static VOID app_ctrl_wifi_connection(APP_CTRL_WIFI_INFO_T *wifi_connect)
{
    WIFI_SETTING_PROC_FUNCTION_BEGIN;

    WIFI_MSG("<APP_CTRL_WIFI> wifi_connect->ssid is %s",wifi_connect->ssid);
    WIFI_MSG("<APP_CTRL_WIFI> wifi_connect->password is %s",wifi_connect->password);
    WIFI_MSG("<APP_CTRL_WIFI> wifi_connect->auth_mode is %d", wifi_connect->auth_mode);

    strncpy(wifi_ConnectSsid,wifi_connect->ssid,NET_802_11_MAX_LEN_SSID);
    strncpy(wifi_ConnectPwd,wifi_connect->password,WLAN_KEY_MAX_LEN);
    strncpy(wifi_ConnectBssid, "FF:FF:FF:FF:FF:FF", NET_802_11_MAX_LEN_BSSID);

    switch (wifi_connect->auth_mode) {
        case AUTH_MODE_NONE:
            wifi_AuthMode = IEEE_802_11_AUTH_MODE_NONE;
            break;
        case AUTH_MODE_WEP:
            wifi_AuthMode = IEEE_802_11_AUTH_MODE_WEPAUTO;
            break;
        case AUTH_MODE_WPA:
            wifi_AuthMode = IEEE_802_11_AUTH_MODE_WPAPSK_OR_WPA2PSK;
            break;
        default:
            wifi_AuthMode = -1;
            break;
    }

    fg_wifi_connect_id = 0;

    start_wifi_connect();

    WIFI_SETTING_PROC_FUNCTION_END;
}

static VOID app_ctrl_wifi_disconnection(VOID)
{
    WIFI_SETTING_PROC_FUNCTION_BEGIN;
    wifi_connect_over();
    WIFI_SETTING_PROC_FUNCTION_END;
}

VOID _wifi_setting_process_app_ctrl_msg(VOID * pv_msg)
{
    APPMSG_T * pt_msg = (APPMSG_T *) pv_msg;

    WIFI_SETTING_FUNCTION_BEGIN;

    if (pv_msg == NULL) {
        WIFI_ERR("<APP_CTRL_WIFI> pv_msg is NULL");
        return;
    }

    WIFI_MSG("<APP_CTRL_WIFI> ui4_msg_type = %d",pt_msg->ui4_msg_type);

    switch(pt_msg->ui4_msg_type) {
        case APP_CTRL_CMD_WIFI_CONNECT:
            app_ctrl_wifi_connection(pt_msg->p_usr_msg);
            break;
        case APP_CTRL_CMD_WIFI_DISCONNECT:
            app_ctrl_wifi_disconnection();
            break;
        default:
            break;
    }

	WIFI_SETTING_FUNCTION_END;

	return;
}


INT32 a_cfg_get_wlan_key(CHAR *pc_value)
{

    if (pc_value == NULL) {
        return WIFI_INV_ARG;
    }

    return WIFI_OK;
}

INT32 a_cfg_get_wlan_ap(void* data)
{
    NET_802_11_ASSOCIATE_T* t_wifi_get = (NET_802_11_ASSOCIATE_T *)data;


    if (t_wifi_get == NULL) {
        return WIFI_INV_ARG;
    }

    return WIFI_OK;
}
