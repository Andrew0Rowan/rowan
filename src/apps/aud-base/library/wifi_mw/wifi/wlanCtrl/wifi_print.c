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


/*----------------------------------------------------------------------------*
 * $RCSfile: wifi_printf.c,v $
 * $Revision:
 * $Date:
 * $Author: yali.wu $
 *
 * Description:
 *         This file contains control API of Wifi Driver for Application.
 *---------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------
                    include files
-----------------------------------------------------------------------------*/

#include "wifi_print.h"
#include "wifi_log.h"

#define MAC2STR(a) (a)[0], (a)[1], (a)[2], (a)[3], (a)[4], (a)[5]

void print_ess_info (INT32 num,NET_802_11_ESS_INFO_T *p_EssInfo) 
{
	   NET_802_11_ESS_INFO_T * ptr_EssInfo = p_EssInfo;
	
	   if(ptr_EssInfo == NULL)
	   {
		   WIFI_LOG(WIFI_ERROR,("Ess Info handle is NULL\n"));
		   return;
	   }
	   WIFI_LOG(WIFI_INFO,("Ess %03d :[%02x:%02x:%02x:%02x:%02x:%02x] [%02d]\
 [%02d] [%03d] [%d] [%d] [%s]\t[%02d]",
		   num,MAC2STR(ptr_EssInfo->t_Bssid),ptr_EssInfo->e_AuthMode,ptr_EssInfo->e_AuthCipher,
		   ptr_EssInfo->i2_Level,ptr_EssInfo->i4_isConnected,ptr_EssInfo->is_wps_support,
		   ptr_EssInfo->t_Ssid.ui1_aSsid,ptr_EssInfo->t_Ssid.ui4_SsidLen));
	   return;

	return;
}

void print_esslist (NET_802_11_ESS_LIST_T *p_essList_result)
{
	NET_802_11_ESS_LIST_T *ptr_essList_result = p_essList_result;
	NET_802_11_ESS_INFO_T    *ptr_EssInfo = NULL;
	INT32 i = 0;
	if(ptr_essList_result == NULL)
	{
		WIFI_LOG(WIFI_ERROR,("Ess List result struct is NULL"));
		return;
	}
	
	if((ptr_essList_result->ui4_NumberOfItems== 0))
	{
		WIFI_LOG(WIFI_ERROR,("Ess List result ui4_NumberOfItems is 0"));
		return;
	}
	
	WIFI_LOG(WIFI_INFO,
		(">>>>Esslist: bssid Authmode AuthCipher Level isCon wps_support ssid ssid.len <<<<"));
	for(i = 0; i < ptr_essList_result->ui4_NumberOfItems;i++)
	{
		ptr_EssInfo = (NET_802_11_ESS_INFO_T *) & ptr_essList_result->p_EssInfo[i];
		print_ess_info(i+1,ptr_EssInfo);
	}
	return;
}

void print_associate (NET_802_11_ASSOCIATE_T *p_associate)
{
	NET_802_11_ASSOCIATE_T * ptr_associate = p_associate;
	if(ptr_associate == NULL)
	{
		WIFI_LOG(WIFI_ERROR,("associate struct is NULL"));
		return ;
	}
	WIFI_LOG(WIFI_INFO,("associate bssid [%02x:%02x:%02x:%02x:%02x:%02x]",MAC2STR(ptr_associate->t_Bssid)));
	WIFI_LOG(WIFI_INFO,("associate ssid [%s] len [%d]",ptr_associate->t_Ssid.ui1_aSsid,ptr_associate->t_Ssid.ui4_SsidLen));
	WIFI_LOG(WIFI_INFO,("associate e_AssocCase [%d]",ptr_associate->e_AssocCase));
	WIFI_LOG(WIFI_INFO,("associate e_AuthMode [%d]",ptr_associate->e_AuthMode));	
	WIFI_LOG(WIFI_INFO,("associate e_AuthCipher [%d]",ptr_associate->e_AuthCipher));
	WIFI_LOG(WIFI_INFO,("associate t_Key.b_IsAscii [%d] pui1_PassPhrase [%s] keybody [%s]",
		ptr_associate->t_Key.b_IsAscii,ptr_associate->t_Key.pui1_PassPhrase,ptr_associate->t_Key.key_body));
	return;
}

void  print_bss_info (INT32 num,NET_802_11_BSS_INFO_T *p_bss_info)
{
	NET_802_11_BSS_INFO_T * ptr_BssInfo = p_bss_info;

	if(ptr_BssInfo == NULL)
	{
		WIFI_LOG(WIFI_ERROR,("Bss Info handle is NULL\n"));
		return;
	}

	WIFI_LOG(WIFI_INFO,("Bss:%03d :[%02x:%02x:%02x:%02x:%02x:%02x] [%d]\
 [%s] [%d] [%d] [%d] [%d] [%d] [%d] [%d] [%d] [%d] [%d] [%d]",
		num,MAC2STR(ptr_BssInfo->t_Bssid),ptr_BssInfo->t_Ssid.ui4_SsidLen,
		ptr_BssInfo->t_Ssid.ui1_aSsid,ptr_BssInfo->e_AuthMode,ptr_BssInfo->e_AuthCipher,
		ptr_BssInfo->i2_Freq,ptr_BssInfo->i2_Channel,ptr_BssInfo->i2_Quality,
		ptr_BssInfo->i2_Noise,ptr_BssInfo->i2_Level,ptr_BssInfo->i4_MaxRate,
		ptr_BssInfo->u2_Caps,ptr_BssInfo->is_wps_support,ptr_BssInfo->u8_wpa_status));
	return;
}
void print_scan_result (NET_802_11_SCAN_RESULT_T *p_scan_result)
{
	NET_802_11_SCAN_RESULT_T *ptr_scan_result = p_scan_result;
	NET_802_11_BSS_INFO_T    *ptr_BssInfo = NULL;
	INT32 i = 0;
	if(ptr_scan_result == NULL)
	{
		WIFI_LOG(WIFI_ERROR,("scan result struct is NULL"));
		return;
	}
	
	if((ptr_scan_result->ui4_NumberOfItems == 0))
	{
		WIFI_LOG(WIFI_ERROR,("Scan result ui4_NumberOfItems is 0"));
		return;
	}
	
	WIFI_LOG(WIFI_INFO,(">>>>bssid	ssid.len ssid  Authmode AuthCipher Freq Channel Quality\
	Noise Level MaxRate Caps wps_support wpa_status<<<<"));
	for(i = 0; i < ptr_scan_result->ui4_NumberOfItems;i++)
	{
		ptr_BssInfo = (NET_802_11_BSS_INFO_T *) & ptr_scan_result->p_BssInfo[i];
		print_bss_info(i+1,ptr_BssInfo);
	}
	return;
}

