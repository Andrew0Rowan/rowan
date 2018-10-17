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

/*----------------------------------------------------------------------------*/
/*! @file x_net_p2p_ctl.h
 *  $RCSfile: x_net_p2p_ctl.c,v $
 *  $Revision: #3 $
 *  $Date: 2013/11/15 $
 *  $Author: yali.wu $
 *
 *  @par Description:
 *         This file contains the prototype of control API of Wifi Driver for Application.
 */
/*----------------------------------------------------------------------------*/

#ifndef _X_NET_P2P_CTRL_H
#define _X_NET_P2P_CTRL_H

/*----------------------------------------------------------------------------*/
/*! @addtogroup groupMW_INET_WIFI
 *  @{
 */
/*----------------------------------------------------------------------------*/

#include "x_net_wlan.h"
#include "wifi_com.h"
#include <sys/stat.h>

#if PLATFORM_IS_BDP
#define DHCPD_EXEC_PATH         WPA_SUPPLICANT_PATH"/dhcpd"
#else
#define DHCPD_EXEC_PATH         WPA_SUPPLICANT_PATH"/dhcpd/dhcpd"
#endif
#define DHCPD_LEASE_FILE_PATH   "/tmp/dhcpd.leases"
#if PLATFORM_IS_BDP
#if CONFIG_WLAN_SW_FOR_CUSTOM_JSN
#define DHCPD_CONFIG_FILE_PATH "/usr/bin/dhcpd_sony.conf"
#else
#define DHCPD_CONFIG_FILE_PATH "/usr/bin/dhcpd.conf"
#endif
#else
#define DHCPD_CONFIG_FILE_PATH  WPA_SUPPLICANT_PATH"/dhcpd/dhcpd.conf"
#endif
#if PLATFORM_IS_BDP
#define P2P_SERVER_IP_FILE_PATH "/tmp/wpa_supplicant/p2p_s_ip"
#else
#define P2P_SERVER_IP_FILE_PATH "/tmp/p2p_s_ip"
#endif
#define WIFI_SOCKET_PATH        "/tmp/wpa_supplicant/"
#define DHCPD_KILL_SCRIPT_PATH  WPA_SUPPLICANT_PATH"/dhcpd/kill.script"

#define WFD_CMD_BUF_LEN             128
#if PLATFORM_IS_BDP
#define WFD_DEFAULT_DEVICE_NAME     "MTK-BDP-Sink"
#else
#define WFD_DEFAULT_DEVICE_NAME     "MTK-DTV-Sink"
#endif
#define WFD_DEFAULT_OP_CHANNEL      1

#define WFD_CMD_VAL_DEV_TYPE_SINK       1
#define WFD_CMD_VAL_DEV_TYPE_SOURCE     0
#define WFD_CMD_VAL_SOURCE_COUPLED      0
#define WFD_CMD_VAL_SINK_COUPLED        0
#define WFD_CMD_VAL_SESSION_AVAILABLE   1
#define WFD_CMD_VAL_RTSP_PORT           554 /* Default RTSP port */
#define WFD_CMD_VAL_MAX_THROUGHPUT      20
#define WFD_CMD_VAL_ENABLE_WFD          1
#define WFD_CMD_VAL_DISABLE_WFD         0
#define WFD_CMD_VAL_ENABLE_CONTENT_PROTECT          1
#define WFD_CMD_VAL_DISABLE_CONTENT_PROTECT         0

enum
{
    WFD_P2P_CMD_DEV_TYPE = 0
    ,WFD_P2P_CMD_SOURCE_COUPLED
    ,WFD_P2P_CMD_SINK_COUPLED
    ,WFD_P2P_CMD_SESSION_AVAILABLE
    ,WFD_P2P_CMD_RTSP_PORT
    ,WFD_P2P_CMD_MAX_THROUGHPUT
    #if PLATFORM_IS_TV
    ,WFD_P2P_CMD_DEVICE_NAME 
    #endif    
    //,WFD_P2P_CMD_SET_P2P_OP_CHANNEL
    ,WFD_P2P_CMD_SET_CONTENT_PROTECT
//        ,WFD_P2P_CMD_ENABLE_WFD
    ,WFD_P2P_CMD_MAX
};



INT32 x_net_wlan_p2p_set_peer_ip(CHAR * ip , int len);
INT32 x_net_wlan_p2p_set_wfd_dev_info_subelem(void);

INT32 x_net_wlan_p2p_enable_wfd_mode(void);

INT32 x_net_wlan_p2p_peer(char *reply, int relpyLen, char *deviceAddress);
INT32 x_net_wlan_parse_p2p_peer_info(char *infoStr, NET_802_11_P2P_PEER_INFO *p2pPeerInfo);
INT32 x_net_wlan_get_p2p_peer_info(NET_802_11_P2P_PEER_INFO *peerInfo, char *deviceAddress);
INT32 x_net_wlan_p2p_getInterfaceAddress(char *interfaceAddress, char *deviceAddress);

INT32 x_net_wlan_p2p_getDeviceAddress(char *deviceAddress);

INT32 x_net_wlan_p2p_get_go_ssid(char *gossid);

INT32 x_net_wlan_p2p_get_WPA_key(char *key);

INT32 x_net_wlan_cancel_moniter_ip(void);

#if PLATFORM_IS_TV
INT32 x_net_p2p_get_current_ip_info(char *name, NW_IP_INFO_T* pt_ip_info);
INT32 x_net_p2p_start_dhcpc(char *name);
INT32 x_net_p2p_restart_dhcpc(char *name);
INT32 x_net_p2p_stop_dhcpc(char *name);
#endif
INT32 get_file_stat(char * file_path , struct stat * stat_val);
BOOL is_file_changed(struct stat stat_o , struct stat stat_d);

INT32 x_net_wlan_clean_file (char* path);
INT32 x_net_wlan_monitor_dhcpd_lease(void* pIn);
char * _get_ip_from_mac (const char * str_mac,const char * file_path);

INT32 _P2P_Get_Random_SSID(char *ssid, int buflen);
INT32 _P2P_Get_Random_Key(char *key, int buflen);
INT32 x_net_stop_dhcpd(void);

/*add for service discovery*/
INT32 x_net_p2p_serv_disc_enable(BOOL enable);

INT32 x_net_p2p_serv_disc_resq_pre(INT16 protocolType,INT16 version, char *requestData, char *responseData);

INT32 x_net_p2p_serv_disc_req(char *mac, INT16 protocolType,INT16 version, char *requestData);
/*add for service discovery*/

/*------------------------------------------------------------------*/
/*! @brief get persistent group network id for CFG802.11 mode.
 *
 *  @param [in]   networkId_buf    the buffer to store persistent group network id.
 *  @param [in]   networkIdbuflen    the length of buffer to store persistent group network id.
 *  @param [out]   networkNum    the buffer to store persistent group network total number.
 *  @return     result
 *  @retval     0 -success
 *  @retval     otherwise -failure
 */
/*------------------------------------------------------------------*/
INT32 x_net_wlan_cfg80211_get_persistent_network_id(int *networkId_buf, int networkIdbuflen, int *networkNum);

#endif
