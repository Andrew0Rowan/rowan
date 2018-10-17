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


#ifndef _WIFI_LOG_H
#define _WIFI_LOG_H
//#include <stdio.h>
#include <stdarg.h>	
#include "u_common.h"
#include "u_dbg.h"

//#define WIFI_STARDARD_DBG

#ifndef WIFI_STARDARD_DBG

#else
#define DBG_LEVEL_MODULE            wlan_get_dbg_level()
#define WIFI_LOG(level,fmt...) DBG_LOG_MW((1<<level),(fmt));DBG_LOG_MW((1<<level),("\n"))
#endif


/* Set WLAN share library debug level */
/* 
 * fg_enable: 1: enable, 0: disable
 * level: the debug level which want to enable or disable (LogDefault < level < LogAll)
 * Returns: 0: success, otherwise failure
 */
int wlan_set_dbg_level(int fg_enable, int level);

int wlan_get_dbg_level(void);


typedef enum
{
    WIFI_ERROR           = 0,
	WIFI_WARNING,	
	WIFI_API,
	WIFI_DEBUG,	
	WIFI_INFO,
	WIFI_LOGALL,
    WPALogLevel_Cnt,
} WPA_LogLevelDef;


#define LogDefault WIFI_DEBUG
#define LogInfo WIFI_INFO


extern unsigned int wifi_dbg_level;

int wifi_dbg_log (const char *  ps_format, ...);
extern const char *wpa_get_dbg_level_string(int level);

#define WIFI_LOG_IS_ENABLED(level) \
             (wifi_dbg_level & (1u << level))
	      

 
#define WIFI_LOG(level, args) \
         do { \
             if (WIFI_LOG_IS_ENABLED(level)) { \
                 wifi_dbg_log("[WIFI_MW] "); \
                 wifi_dbg_log args; \
			 	 wifi_dbg_log("\n"); \
             } \
         } while (0)
 
 
#define WIFI_LOG_ENTRY(level, args) \
         do { \
             if (WIFI_LOG_IS_ENABLED( level)) { \
                 wifi_dbg_log("[WIFI_MW][Enter]%s : ", __PRETTY_FUNCTION__); \
                 wifi_dbg_log args; \
                 wifi_dbg_log("\n"); \
             } \
         } while (0) 


#define WIFI_LOG_EXIT(level, args) \
         do { \
             if (WIFI_LOG_IS_ENABLED(level)) { \
                 wifi_dbg_log("[WIFI_MW][Exit]%s [Line%d] : ", __PRETTY_FUNCTION__, __LINE__); \
                 wifi_dbg_log args; \
			 	 wifi_dbg_log("\n"); \
             } \
         } while (0)

#define WIFI_CHK_FAIL(_ret)	\
			 do{ \
			 int	ret = _ret;  \
			 if(ret <0)  \
			 {	 \
				 WIFI_LOG(WIFI_ERROR, (" Return fail:%s:%d:%d\r\n", __FILE__, __LINE__, ret)); \
				 return  ret;	 \
			 }	 \
			 }while(FALSE)

#endif
