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


#ifndef _X_NET_WLAN_CTRL_H
#define _X_NET_WLAN_CTRL_H
#include "x_net_wlan.h"

typedef struct
{
    INT32  rssi;
    INT32  linkSpeed;
    INT32  noise;
    INT32  freq;
} SIGNAL_DATA_T;

INT32 x_net_wlan_open_connection(const char *ifname);
INT32 x_net_wlan_open_connection_per_path(const char *ifname, char *ctrl_path, char *moni_path);
INT32 x_net_wlan_close_connection(void);
INT32 x_net_wps_generate_pin_code(char *wps_pin_code);
INT32 x_net_wlan_read_e2p(int e2pAddr, UINT32 *e2pValue);
INT32 x_net_get_wlan_freq_offset(int addr1, int addr2, int addr3, int *freqoffset);
INT32 x_net_wlan_get_mt7662_freq_offset(int *freqoffset);
INT32 x_net_wlan_get_mac_addr(UINT8 *pMacAddr);
INT32 x_net_wlan_scan_with_ssid(x_net_wlan_notify_fct pfNotify, char * ssid, int ssid_len );
INT32 x_net_wlan_get_scan_result(NET_802_11_SCAN_RESULT_T *pScanResult);
INT32 x_net_wlan_get_ess_list(NET_802_11_ESS_LIST_T *pEssList_result);
INT32 x_net_wlan_disassociate(VOID);
INT32 x_net_wlan_wpa_signal_poll(SIGNAL_DATA_T *pSignal);
INT32 x_net_wlan_get_curr_bss(NET_802_11_BSS_INFO_T *pCurrBss);
INT32 x_net_wlan_wpa_reg_cbk(x_net_wlan_notify_fct pfNotify);
INT32 x_net_wlan_wpa_unreg_cbk(x_net_wlan_notify_fct pfNotify);
INT32 x_net_wlan_set_send_true(void);
INT32 x_net_wlan_get_rssi(NET_802_11_WIFI_RSSI_T *pRSSI);

INT32 _get_STA_bandwidth(INT32 *bw);
INT32 _wpa_determine_wireless_type(INT32 freq, INT32 bw, char *bitrate_str, NET_802_11_T *wireless_type);
int wlan_set_reconnect_network(NET_802_11_ASSOCIATE_T *pAssociate);
int wlan_set_enable_network(NET_802_11_ASSOCIATE_T *pAssociate);

#endif
