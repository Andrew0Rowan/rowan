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
 * MediaTek Inc. (C) 2014. All rights reserved.                                
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


#ifndef _WIFI_SETTING_H_
#define _WIFI_SETTING_H_
/*-----------------------------------------------------------------------------
                    include files
-----------------------------------------------------------------------------*/
#include "u_dbg.h"
#include "u_timerd.h"
#include "c_net_wlan.h"
#include "c_net_config.h"
#include "u_wifi_setting.h"
#include "u_app_def.h"
#include "u_net_wlan.h"
#include "u_acfg.h"

#undef  DBG_LEVEL_MODULE
#define DBG_LEVEL_MODULE        wifi_get_dbg_level()

#define WIFI_TAG                "<WIFI>"
#define WIFI_ERR(fmt, args...)  do{printf("%s[%s:%d]:"fmt, WIFI_TAG, __FUNCTION__, __LINE__, ##args);}while(0)
#define WIFI_MSG(fmt, args...)  do{printf("%s[%s:%d]:"fmt, WIFI_TAG, __FUNCTION__, __LINE__, ##args);}while(0)

/*-----------------------------------------------------------------------------
                    macros, defines, typedefs, enums
-----------------------------------------------------------------------------*/
//#define APP_NAME                            "wifi_setting"    /* max length = 16*/

/* return value */
#define WIFI_OK                         ((INT32)0)
#define WIFI_FAIL                       ((INT32)-1) /* abnormal return must < 0 */
#define WIFI_INV_ARG                    ((INT32)-2)

/* message */
#define WIFI_KEY_MSG                    ((UINT32) 0)
#define WIFI_WLAN_MSG                   ((UINT32) 1)
#define WIFI_SOFTAP_MSG                 ((UINT32) 2)

#define SETUP_WIFI_KEY_LEN              (64)


//MUST BE  SETUP_WLAN_SCAN_RESULT_SHOW_NUM=SETUP_WIFI_SCAN_RESULT_SHOW_NUM=160
#define SETUP_WIFI_SCAN_RESULT_SHOW_NUM 160
//#define MISC_MESH_SCAN_DB                        "/tmp/scan_mesh/wireless_scan_result.db"

#define SETUP_WIFI_APP_ADDR_LEN         64
#define STCN_DRV_DATA_BUFFER_LEN        512
#define WIFI_AP_SWITCH_STATUS_LEN       32

#define  WIFI_CHK_FAIL(_ret, _ret_on_err)                      \
    do {                                                                    \
    if(_ret < 0)                                                            \
        {                                                                   \
        DBG_ERROR (("<WIFI_SETTING> ERR: file = %s, line = %d, reason = %d\n\r",      \
        __FILE__, __LINE__, _ret));                                         \
        return _ret_on_err;                                                 \
        }                                                                   \
    } while (0)


#if CONFIG_APP_SUPPORT_SMART_CONNECTION
typedef struct
{
    int  mesh_chanel;
    char server_ip[20];
    char mesh_id[32];
    char mesh_key[32];
}SMART_CONNECT_MESH_INFO;

typedef struct
{
    char   ap_name[NET_802_11_MAX_LEN_SSID+1];
    char   ap_password[WLAN_KEY_MAX_LEN];
    char   app_addr[SETUP_WIFI_APP_ADDR_LEN];
    INT32  ap_authmode;
}CONNECT_AP_INFO;
#endif

typedef enum
{
    SCAN_AP_STAT_NONE        = 0,
    SCAN_AP_STAT_FAILE       = 1,
    SCAN_AP_STAT_OK          = 2,
    CONNECT_AP_STAT_OK       = 3,
    CONNECT_AP_STAT_FAILE
}SCAN_AP_STAT_T;

typedef enum
{
    TIMER_MSG_WIFI_SCAN,
    TIMER_MSG_WIFI_ASSCOCIATE,
    TIMER_MSG_MAX
} WIFI_SETTING_TIMER_MSG_E;

typedef struct _WIFI_MSG_T
{
    UINT32          ui4_msg_id;
    UINT32          ui4_data1;
    UINT32          ui4_data2;
    UINT32          ui4_data3;
} WIFI_MSG_T;

/* application structure */
typedef struct _WIFI_OBJ_T
{
    HANDLE_T        h_app;
    BOOL            b_g_is_init;
    BOOL            b_connect_over;
    TIMER_TYPE_T    t_wlan_timer;
    TIMER_TYPE_T    t_associate_timer;
    TIMER_TYPE_T    t_scan_timer;
    pthread_t       t_thread;
    pthread_mutex_t t_mutex;
    pthread_cond_t  t_cond;
}WIFI_OBJ_T;

enum
{
    SETUP_AP_ENCRYPT_AES,
    SETUP_AP_ENCRYPT_TRIP,
    SETUP_AP_ENCRYPT_WEP,
    SETUP_AP_ENCRYPT_NONE,
};

enum
{
    SETUP_AP_AUTHEN_WPA2_PSK,
    SETUP_AP_AUTHEN_WPA_PSK,
};


typedef struct WIFI_SETTING_WLAN_STT_FLAG
{
    BOOL  b_scan_request_from_pin_code;
    BOOL  b_user_cancel;
    UINT8 ui1_wlan_notify_type;
    UINT32 ui4_wlan_timer_counter; //timer counter.
    UINT16 ui1_wlan_cur_idx;
    UINT16 ui1_wlan_total_num;
    CHAR  wlan_ssid[NET_802_11_MAX_LEN_SSID+1];  //record the input of ssid by editer frame
    CHAR  wlan_wpa_wep[3][SETUP_WIFI_KEY_LEN+1];      //record the input of wap wep by editer frame.

    NET_802_11_BSS_INFO_T cur_show_result[SETUP_WIFI_SCAN_RESULT_SHOW_NUM];
    UINT8 ui1_ap_count;                          // only for WLAN_NOTIFY_WPS_ASSOC_SETTING
    NET_802_11_ASSOCIATE_T _rCurWlanAssociate[3];

    BOOL  b_ap_search_again;
    UINT8 ui1_ap_scan_initiator;
    UINT8 ui1_ap_connect_initiator;
    UINT8 ui1_ap_encrypt_type;/*AP ecnryption type*/
    UINT8 ui1_ap_auth_type;   /*AP Authentication type*/
    UINT16 wlan_user_input_key[SETUP_WIFI_KEY_LEN];

    char  networklist[1024];
}   WIFI_SETTING_WLAN_STT_T;

typedef enum _SETUP_WLAN_FM_OWNER_T
{
    WLAN_FM_OWNER_WIRED_WIRELESS_SWITCH, //0
    WLAN_FM_OWNER_CHOOSE_WIRED,
    WLAN_FM_OWNER_CHOOSE_WIRELESS,
    WLAN_FM_OWNER_DONGLE_CABLE_PLUG_IN,
    WLAN_FM_OWNER_DONGLE_CABLE_PLUG_OUT,
    WLAN_FM_OWNER_AP_SCANING,//5
    WLAN_FM_OWNER_MANUAL_INPUT_SSID,
    WLAN_FM_OWNER_MANUAL_INPUT_SECURITY,
    WLAN_FM_OWNER_MANUAL_INPUT_PASSWORD,
    WLAN_FM_OWNER_AUTO_INPUT_PASSWORD,
    WLAN_FM_OWNER_SCAN_ASSOCIATE_SECURITY_NONE,//10
    WLAN_FM_OWNER_SCAN_ASSOCIATE_SECURITY_WPS,//
    WLAN_FM_OWNER_MANUAL_ASSOCIATE_SECURITY_NONE,
    WLAN_FM_OWNER_MANUAL_ASSOCIATE_SECURITY_WPS,
    WLAN_FM_OWNER_WPS_PBC_ASSOCIATE,
    WLAN_FM_OWNER_WPS_PIN_ASSOCIATE,
    WLAN_FM_OWNER_SCAN_FAILED,
    WLAN_FM_OWNER_ASSOCIATE_FAILED,//17
    WLAN_FM_OWNER_ASSOCIATE_OK,
    WLAN_FM_OWNER_SHOW_IP_INPUT_PAGE,
    WLAN_FM_OWNER_WAITING_NEW,
    WLAN_FM_OWNER_WPS_ASSOCIATE_FAILED,
    WLAN_FM_OWNER_WARNING_WEP_NEW,
    WLAN_FM_OWNER_WARNING_WAP_NEW,
    WLAN_FM_OWNER_GET_NO_RESULT_NEW,
    WLAN_FM_OWNER_WIRELESS_SETTING_QUESTION_NEW,
    WLAN_FM_OWNER_SHOW_CONNECT_STATUS,
    WLAN_FM_OWNER_SHOW_SCAN_LIST,
    WLAN_FM_OWNER_MANUAL_ASSOCIATE_FAVORITE_AP,
    WLAN_FM_OWNER_MANUAL_ASSOCIATE_FROM_DONGLE_IN,
    WLAN_FM_OWNER_MAX_NEW//24
}SETUP_WLAN_FM_OWNER_T;

typedef enum _SETUP_WLAN_SETTING_STATUS_T
{
    WLAN_SET_NOT_INITED,
    WLAN_SET_FINISHED,
    WLAN_SET_IN_EASY_SETTING,
    WLAN_SET_IN_NORMAL_SETTING,
    WLAN_SET_CONNECT_FAVORVITE_AP,
    WLAN_SET_CONNECT_FAVORVITE_AP_OK,
    WLAN_SET_CONNECT_FAVORVITE_AP_FAILED,
    WLAN_SET_CONNECT_FAVORVITE_AP_INTERRUPT,
    WLAN_SET_CONNECT_AP_OK,
    WLAN_SET_MAX
}SETUP_WLAN_SETTING_STATUS_T;

/*-----------------------------------------------------------------------------
                    functions declarations
-----------------------------------------------------------------------------*/
#ifdef CLI_SUPPORT
extern INT32 wifi_cli_attach_cmd_tbl(VOID);
#endif

extern INT32 _setup_wlan_prepare_to_scan(VOID);
extern VOID _setup_wlan_associate_timer_cb(VOID);
extern VOID _setup_wlan_scan_timer_cb(VOID);
extern INT32 setup_wlan_try_connect_favorite_ap_as_dongle_in(VOID);
extern INT32 _wifi_setting_wlan_notify_msg_proc(const VOID *pv_msg);
extern signed int wifi_open_smart_connection(unsigned char ui1IfType);
extern VOID wifi_configure_station_connect(CONNECT_AP_INFO *wifi_connect);
extern VOID wifi_configure_station_disconnect(VOID);
#endif /* _WIFI_SETTING_H_ */
