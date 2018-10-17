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


#ifndef _WIFI_PARSE_H
#define _WIFI_PARSE_H
#include "x_net_wlan.h"  


int frequency_to_channel(int freq);
int channel_to_frequency(int channel);
int get_protocal_type(NET_802_11_T * p_e_802_11_type, char * p80211type);
int  tok_scan_bssid_convert(char *s_scan, NET_802_11_BSS_INFO_T *p_bss_info, char* s_scan_header);
INT32 wpa_get_scan_results_from_ret_buf(char * buf,NET_802_11_SCAN_RESULT_T *pScanResult);
INT32 wpa_get_status_from_ret_buf(char * buf,NET_802_11_BSS_INFO_T *pCurrBss);
int proc_signal_level_ra0 (INT16 *i2_level, char* str);
int proc_rate_ra0 (INT32 *i4_max_rate, char* str);
int proc_rate_ath0 (INT32 *i4_max_rate, char* str);
int proc_signal_level_ath0(INT16 *i2_level, char* str);
int proc_wpa_state(UINT8 *p_wpa_state, char *str);
int  proc_pairwise_cipher_str(NET_802_11_AUTH_CIPHER_T  *e_auth_ciper, char * str);
int proc_key_mgmt_str(NET_802_11_AUTH_MODE_T *e_auth_mode, char *str);

INT32 wps_proc_cred(char * cred, NET_802_11_ASSOCIATE_T *assoc);
INT32 wps_get_cred_mac(char * cred, unsigned char *mac_addr);
INT32 wps_get_cred_network_key(char * cred, char * key_body);
INT32 wps_get_cred_netkeyidx(char * cred, INT32 * key_idx);
INT32 wps_get_cred_encypt_type(char * cred, INT32 * encypt_type);
INT32 wps_get_cred_auth_type(char * cred, INT32 * auth_type);
INT32 wps_get_cred_ssid(char * cred, char * ssid);
INT32 wps_get_cred_network_id(char * cred, INT32 * network_id);
INT32 wps_get_cred_context(char * cred);
INT32 wps_get_cred_RF_band(char * cred, unsigned char *prf_band);

INT32 str_to_mac(char * str, UINT8 * bssid);
unsigned char str2mac(char*s);



#endif
