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


#include "wifi_log.h"

#ifndef WIFI_STARDARD_DBG

#else
#include "dbg/def_dbg_level_mw.h"
static UINT16 ui2_wifi_dbg_level = DBG_INIT_LEVEL_MW_WIFI;
#endif


unsigned int wifi_dbg_level = (1u << WIFI_DEBUG) |(1u << WIFI_ERROR) | (1u << WIFI_API);

static const char *wlan_LogLevelNames[WPALogLevel_Cnt] =
{
    "0 Error",
    "1 Warning",    
    "2 Api",
    "3 Debug",
    "4 Info",
    "5 LogAll"
};

const char *wpa_get_dbg_level_string(int level)
{
    if (level <= WIFI_LOGALL)
    {
        return (const char *)wlan_LogLevelNames[level];
    }
    else
    {
        return "";
    }
}

int wifi_dbg_log (const char *  ps_format, ...)
{
    VA_LIST arg_list;

    int i4_len = -1;

    VA_START (arg_list, ps_format);
    i4_len = vprintf((const char *)ps_format, arg_list);
    /* Create debug statement string and send it to output device. */
    VA_END (arg_list);

    return 0;
}

int wlan_get_dbg_level()
{
    wifi_dbg_log("Current WLAN debug level=%08x, (", (int)wifi_dbg_level);
    int i = 0;
    for (; i < WPALogLevel_Cnt; i++)
    {
        if (wifi_dbg_level & (1u << i))
        {
            wifi_dbg_log(" %s |", wpa_get_dbg_level_string(i));
        }
    }
    wifi_dbg_log(")\n");
    return wifi_dbg_level;
}


int wlan_set_dbg_level(int fg_enable, int level)
{
    wifi_dbg_log("  Enter %s \n", __FUNCTION__);
    wifi_dbg_log("Current WLAN debug level=%08x, (", (int)wifi_dbg_level);
    int i = 0;
    for (; i < WPALogLevel_Cnt; i++)
    {
        if (wifi_dbg_level & (1u << i))
        {
            wifi_dbg_log(" %s |", wpa_get_dbg_level_string(i));
        }
    }
    wifi_dbg_log(")\n");

    if (level < WIFI_ERROR || level > WIFI_LOGALL)
    {
        wifi_dbg_log("Invalid debug level, level=%d @ %s\n", level, __FUNCTION__);
        wifi_dbg_log("  [level] -\n");
        wifi_dbg_log("  WIFI_ERROR      = 0 \n");
        wifi_dbg_log("  WIFI_WARNING    = 1 \n");
        wifi_dbg_log("  WIFI_API        = 2 \n");
        wifi_dbg_log("  WIFI_DEBUG      = 3 \n");
        wifi_dbg_log("  WIFI_Info       = 4 \n");
        wifi_dbg_log("  WIFI_LogAll     = 5 \n");
        return -1;
    }

    if (fg_enable == 1)
    {
        if (level != WIFI_LOGALL)
        {
            wifi_dbg_level |= (1u << level);
        }
        else
        {
            wifi_dbg_level = 0xFFFFFFFF;
        }
    }
    else if (fg_enable == 0)
    {
        if (level != WIFI_LOGALL)
        {
            wifi_dbg_level &= ~(1u << level);
        }
        else
        {
            wifi_dbg_level = 0;
        }
    }
    else
    {
        wifi_dbg_log("Invalid log flag, flag=%d @ %s\n", __FUNCTION__, fg_enable);
        wifi_dbg_log("  [flag] -\n");
        wifi_dbg_log("    Enable log      = 1 \n");
        wifi_dbg_log("    Disable log     = 0 \n");
        return -1;
    }

    wifi_dbg_log(" Set WLAN debug level successfully\n");
    wifi_dbg_log("New WLAN debug level=%08x, (", (int)wifi_dbg_level);
    for (i = 0; i < WPALogLevel_Cnt; i++)
    {
        if (wifi_dbg_level & (1u << i))
        {
            wifi_dbg_log(" %s |", wpa_get_dbg_level_string(i));
        }
    }
    wifi_dbg_log(")\n");

    return 0;
}


