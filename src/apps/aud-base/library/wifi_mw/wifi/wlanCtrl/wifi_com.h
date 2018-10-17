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


/*****************************************************************************
 *
 * Filename:
 * ---------
 *   $Workfile: wlan_common.h $
 *
 * Project:
 * --------
 *
 *
 * Description:
 * ------------
 *   Common structure, macro and constant for all WLAN modules
 *
 * Author:
 * -------
 *   Hs Huang
 *
 * Last changed:
 * -------------
 * $Author:
 *
 * $Modtime:
 *
 * $Revision:
****************************************************************************/

#ifndef _WIFI_COM_H
#define _WIFI_COM_H

#include "u_common.h"
#include "u_os.h"
//#include "x_dbg.h"
#include <net/if.h>
#include <netinet/in.h>

#include <errno.h>
#include <unistd.h>
#include <sys/types.h>

#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/wait.h>

#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <pthread.h>
#include <signal.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>

#include <dirent.h>


//#include "libc/stdio.h"
#ifdef __linux__
#include "sys/time.h"
#else
#ifndef _USE_LINUX
#include "libc/time.h"
#else
#include <time.h>
#endif
#endif

#ifndef ETH_ALEN
#define ETH_ALEN 6
#endif

#ifndef MAC_STR_LEN
#define MAC_STR_LEN 17
#endif

/*******************************************************************************
*
*  Constant
*
*******************************************************************************/

/* HT Cap related defines */
/* ----- HT Capabilities IE ----- */
#define HTCAP_IE_ID         45
#define HTCAP_IE_MIN_LEN    26 // excluding ie_id and len
    /* .. HT Capabilities Info field .. */
    #define BITMASK_HTCAP_INFO_CH_WIDTH     (1<<1)
    #define BITMASK_HTCAP_INFO_SGI_20       (1<<5)
    #define BITMASK_HTCAP_INFO_SGI_40       (1<<6)
    /* .. Supported MCS Set field .. */
    #define BITMAS_MCS_SET                  (0xffff)
        #define BITMASK_RX_MCS_IDX0             (1<<0)    
        #define BITMASK_RX_MCS_IDX1             (1<<1)    
        #define BITMASK_RX_MCS_IDX2             (1<<2)    
        #define BITMASK_RX_MCS_IDX3             (1<<3)    
        #define BITMASK_RX_MCS_IDX4             (1<<4)    
        #define BITMASK_RX_MCS_IDX5             (1<<5)    
        #define BITMASK_RX_MCS_IDX6             (1<<6)    
        #define BITMASK_RX_MCS_IDX7             (1<<7)    
        #define BITMASK_RX_MCS_IDX8             (1<<8)    
        #define BITMASK_RX_MCS_IDX9             (1<<9)    
        #define BITMASK_RX_MCS_IDX10            (1<<10)    
        #define BITMASK_RX_MCS_IDX11            (1<<11)    
        #define BITMASK_RX_MCS_IDX12            (1<<12)    
        #define BITMASK_RX_MCS_IDX13            (1<<13)    
        #define BITMASK_RX_MCS_IDX14            (1<<14)    
        #define BITMASK_RX_MCS_IDX15            (1<<15)
    #define BYTE_OFFSET_TX_MAX_SPATIAL_STREAM   (12)
        #define BITMASK_TX_MAX_SPATIAL_STREAM   (0xc)

/*******************************************************************************
*
*  Enums
*
*******************************************************************************/


/*******************************************************************************
*
*  Structure
*
*******************************************************************************/


/*******************************************************************************
*
*  Macros
*
*******************************************************************************/
/* The printf macro is depended on platform to change it */
#define size_t      SIZE_T

#ifdef BDP_ENV
#define atoi        StrToInt
#else
#define StrToInt atoi
#define StrToHex atoi
#endif
//#define UNUSED(x)   (void)x

#ifndef MAC2STR
#define MAC2STR(a) (a)[0], (a)[1], (a)[2], (a)[3], (a)[4], (a)[5]
#define MACSTR "%02X:%02X:%02X:%02X:%02X:%02X"
#endif


#define IF_BUF_SIZE             512
#define WIFI_DIRECT_DEV_NUM     4
#define WIFI_INTERFACE_NAME     0
#define WIFI_INTERFACE_PATH     1 
#define WIFI_INTERFACE_P2P_NAME 2
#define WIFI_INTERFACE_SUPPLICANT_PATH 3
#define WIFI_COL_NUM            4

#define  WLAN_MAX_IE_LENGTH      (384)
#define  WLAN_MAX_KEY_LENGTH     (64)
#define  WLAN_MAC_ADDR_LENGTH    (6)
#define  WLAN_PIN_LENGTH         (8)
#define  WLAN_MAX_SCAN_RESULT    (160)
#define  WLAN_MAX_EEPROM_LENGTH  (4)
#define  WLAN_WLAN_IF_MAX_LEN    (15)

#if PLATFORM_IS_TV
#define WPA_SUPPLICANT_PATH "/3rd/bin/wpa_supplicant"
#else
#define WPA_SUPPLICANT_PATH "/usr/bin"
#endif

#define MAX_CBK_NUM 10


struct wpa_ctrl
{
    int s;
    struct sockaddr_un local;
    struct sockaddr_un dest;
};

#if CONFIG_WLAN_SW_FOR_CUSTOM_JSN
#define WLAN_STATUS_WITH_MATCHED_SSID 1
#else
#define WLAN_STATUS_WITH_MATCHED_SSID 0
#endif

#if CONFIG_WLAN_SW_FOR_CUSTOM_JSN
#define WLAN_MULTI_CREDENTIAL_SELECTION 1
#else
#define WLAN_MULTI_CREDENTIAL_SELECTION 0
#endif

#if WLAN_MULTI_CREDENTIAL_SELECTION
#define  WLAN_MAX_WPS_CRED_NUM (4)
#endif

#if PLATFORM_IS_BDP
#define WLAN_SUPPORT_P2P_CFG80211 
#endif

/* KLG wants to use the original signal level value, so don't process the signal level value, just return */
#if CONFIG_MW_CUSTOM_KLG
#define WLAN_SUPPORT_ORIGINAL_SIGNAL 1
#else
#define WLAN_SUPPORT_ORIGINAL_SIGNAL 0
#endif

#endif /* end of _WIFI_COM_H */
