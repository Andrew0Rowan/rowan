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



#ifndef    _U_WIFI_SETTING_H_
#define    _U_WIFI_SETTING_H_
/*-----------------------------------------------------------------------------
                    include files
-----------------------------------------------------------------------------*/
#define WIFI_SETTING_FUNCTION_BEGIN         printf("<WIFI_SETTING> %s %d line begin \n",__FUNCTION__,__LINE__)
#define WIFI_SETTING_FUNCTION_END           printf("<WIFI_SETTING> %s %d line end \n",__FUNCTION__,__LINE__)
#define WIFI_SETTING_PROC_FUNCTION_BEGIN    printf("<WIFI_SETTING_PROC> %s %d line begin \n",__FUNCTION__,__LINE__)
#define WIFI_SETTING_PROC_FUNCTION_END      printf("<WIFI_SETTING_PROC> %s %d line end \n",__FUNCTION__,__LINE__)

typedef enum
{
	SCAN_AP, // 0
	SCAN_STOP,  // 1
    CONNECT_FAVORATE_AP,  // 2
    CONNECT_SETTING_AP, // 3
    START_SMART_CONNECTION,  //4
    CLEAN_AP_INFO,
    GET_AP_INFO_LIST,
    WIFI_TASK_MAX,
} WIFI_WIRED_TASK_T;

typedef enum
{
	OPEN_MONITOR,
	RECV_80211_FRAME,
	CHANNEL_SWITCH,
	CLOSE_MONITOR,
    WIFI_ELIAN_MAX,
} WIFI_ELIAN_TASK_T;

typedef enum {
    CONNECT_SUCCESS = 0,
    PASSWORD_ERROR,
    SCAN_AP_FAIL,
    ASSOCIATE_TIMEDOUT,
    GET_IP_FAIL,
    UNKNOWN_ERROR = 10,
} WIFI_CONNECT_RESULT_E;

typedef enum {
    AUTH_MODE_NONE = 0,
    AUTH_MODE_WEP,
    AUTH_MODE_WPA,
} WIFI_AUTH_MODE_E;

extern INT32 wifi_task_realize(UINT8 i_task, const CHAR *ssid, const CHAR *bssid, const CHAR *password,INT32 authmode);
extern INT32 wifi_elian_task(UINT8 i_task, const CHAR primary_channel , const CHAR secondary_channel,CHAR *bssid);
extern void  wifi_setting_notify_bt_wifi_setup_result(void);
extern INT32 u_wifi_get_current_info(CHAR *ssid, CHAR *bssid, WIFI_AUTH_MODE_E *auth_mode);
extern VOID str_to_strhex(const CHAR* byte, CHAR* strhex, int byte_len);
extern VOID convert_encoding_format(const CHAR* source, CHAR* result);
#endif

