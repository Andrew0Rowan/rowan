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
/*! @file c_net_wlan.h
 * $RCSfile: c_net_wlan.h,v $
 * $Revision: #1 $
 * $Date: 2015/12/08 $
 * $Author: bdbm01 $
 *
 *  @par Description:
 *         This file contains the prototype of control API of Wifi Driver for Application.
 */
/*----------------------------------------------------------------------------*/

#ifndef _C_NET_WLAN_H
#define _C_NET_WLAN_H

/*----------------------------------------------------------------------------*/
/*! @addtogroup groupMW_INET_WIFI
 *  @{
 */
/*----------------------------------------------------------------------------*/

#include "u_net_wlan.h"

INT32 c_net_wlan_set_debug_level(int fg_enable, int level);
INT32 c_net_wlan_get_debug_level();

//#if CONFIG_WIFI_MW_DIRECT

#if CONFIG_MW_CUSTOM_KLG
INT32 c_net_wlan_p2p_find(x_net_wlan_notify_fct pfNotify);
INT32 c_net_wlan_p2p_set_driver_auto_connect(BOOL flag);
#else
INT32 c_net_wlan_p2p_find(x_net_wlan_notify_fct pfNotify,BOOL for_listen);
#endif // CONFIG_MMW_CUSTOM_KLG

INT32 c_net_wlan_p2p_get_mac_addr(UINT8 *pMacAddr);
INT32 c_net_wlan_p2p_set_auto_channel_selection(char *iface, int auto_ch_select);
INT32 c_net_wlan_p2p_cfg80211_auto_channel_selection(int auto_ch_select);
INT32 c_net_wlan_p2p_get_operation_channel(int *channel);
INT32 c_net_wlan_p2p_stop_find(void);
INT32 c_net_wlan_p2p_get_random_ssid(char *ssid, int buflen);
INT32 c_net_wlan_p2p_get_random_key(char *key, int buflen);
#if 0
INT32 c_net_wlan_calculate_PSK_from_ssid_passphrase(char *ssid, int ssid_len,
    char *passphrase, int passphrase_len, char *PSK, int PSK_len);
#endif
INT32 c_net_wlan_p2p_connect(INT32 mode , const char * mac , const char * pin);
/*------------------------------------------------------------------*/
/*! @brief: Connect to a P2P Device with a specific network id (start a persistent group)
*  @param [in] addr -peer device address
*  @param [in] pin - PIN code
*  @param [in] mode - the connection mode, such as "pbc", "display" or "keypad" 
*  @param [in] network_id - the network id of P2P persistent group
*  @return     result
*  @retval     0 -success
*  @retval     otherwise -failure 
*/
/*------------------------------------------------------------------*/
INT32 c_net_wlan_p2p_cfg_accept_persistent_connection(char *addr, char *pin, INT32 mode, int network_id);


INT32 c_net_wlan_p2p_disconnect();
INT32 c_net_wlan_enable_wifi_direct(const char *ifname);
INT32 c_net_wlan_disable_wifi_direct(void);
/* for wpa_supplicant-2.4, enable/disable wifi display by command "SET wifi_display 1/0" to wpa_supplicant */
INT32 c_net_wlan_p2p_enable_wifi_display(int enable);
/* for wpa_supplicant-2.4, Set WFD device information subelements by command "WFD_SUBELEM_SET 0 00060111022a0014" */
INT32 c_net_wlan_p2p_set_wfd_dev_info_subelem(void);
INT32 c_net_wlan_get_p2p_peer_info(NET_802_11_P2P_PEER_INFO *peerInfo, char *deviceAddress);
INT32 c_net_wlan_p2p_reject(char *deviceAddress);
INT32 c_net_wlan_p2p_display_port_getpin(const char * mac );
INT32 c_net_wlan_p2p_keypad_port_displaypin(const char * mac);
INT32 c_net_wlan_p2p_keypad_port_pinconnect(const char * mac, const char * pin);
INT32 c_net_wlan_p2p_display_pin_connect(const char * mac, const char * pin);
INT32 c_net_wlan_p2p_pin_display_accept();
INT32 c_net_wlan_p2p_up_do(void);
INT32 c_net_wlan_p2p_set_dev_name( const char * dev_name );
INT32 c_net_wlan_p2p_set_dev_type( const char * dev_type );
INT32 c_net_wlan_p2p_set_go_intent_value(UINT8 intent);
BOOL c_net_wlan_p2p_set_go_ssid(char *ssid, char *passphrase);
INT32 c_net_wlan_p2p_listen(int timeout);
//stop p2p listen
INT32 c_net_wlan_p2p_stop_listen(void);
INT32 c_net_wlan_p2p_setListenchannel(INT16 channel);
INT32 c_net_wlan_p2p_set_HDCP(BOOL flag);
#if 0 //CONFIG_MW_SUPPORT_MR
INT32 c_net_wlan_p2p_associate_table(void);
#else
INT32 c_net_wlan_p2p_associate_table(P2P_ASSOC_ENTRY_T *AssocTab);
#endif


INT32 c_net_wlan_p2p_groupAdd();
INT32 c_net_wlan_p2p_groupAdd_withssid(const char *ssid,const char *passphrase);

/*------------------------------------------------------------------*/
/*! @brief: Statrt a new P2P Group or an exist one with specific network id
*  @param [in] network_id - the network id of P2P network, -1: start a new P2P Group,
                                              other: an exist P2P Group
*  @param [in] freq -the frequency which P2P Group start at, 0: not specify frequency,
                                                other: specify a frequency
*  @return     result
*  @retval     0 -success
*  @retval     otherwise -failure 
*/
/*------------------------------------------------------------------*/
INT32 c_net_wlan_p2p_cfg_groupAdd_Persistent(int network_id,
        int freq);

INT32 c_net_wlan_p2p_groupRemove(char *iface);

/*------------------------------------------------------------------*/
/*! @brief: Reconfigure the P2P by new configuration
*  @param  - none
*  @return     result
*  @retval     0 -success
*  @retval     otherwise -failure 
*/
/*------------------------------------------------------------------*/
INT32 c_net_wlan_p2p_cfg_reconfigure(void);

INT32 c_net_wlan_p2p_pbc_accept(const char * mac);

INT32 c_net_wlan_p2p_reg_cbk(x_net_wlan_notify_fct pfNotify);
INT32 c_net_wlan_p2p_unreg_cbk(x_net_wlan_notify_fct pfNotify);

INT32 c_net_wlan_p2p_wpsPbc(const char * mac);
INT32 c_net_wlan_p2p_wpsPin_withMac(const char * mac,const char *pin);
INT32 c_net_wlan_p2p_wpsPin(const char *pin);
INT32 c_net_wlan_p2p_invite(char *groupInterface, char *groupOwnerDeviceAddress, char *deviceAddress);
INT32 c_net_wlan_p2p_wpsCancel(void);

#if CONFIG_MW_CUSTOM_KLG
INT32 c_net_p2p_get_other_ip(void);
#else
INT32 c_net_p2p_get_other_ip(char *IP, UINT32 *port);
#endif // CONFIG_MW_CUSTOM_KLG

INT32 c_net_wlan_p2p_set_as_wfd_mode(BOOL wfd_mode);
INT32 c_net_wlan_wfd_setwfdsessionavail(BOOL enabled);
INT32 c_net_wlan_p2p_check_peer_channel(int value);

INT32 c_net_wlan_p2p_set_persistent(BOOL enable);
INT32 c_net_wlan_setPersistentTable(PERSISTENT_ENTRY_T* per_entry);
INT32 c_net_wlan_getPersistentTable(char* mac,PERSISTENT_ENTRY_T* per_entry);
INT32 c_net_wlan_deletePersistentEntry(UINT8 *mac);

INT32 c_net_wlan_set_auto_accept(BOOL enable);
INT32 c_net_wlan_p2p_set_del_client(const char * mac);
/* cut P2P connection for P2P GO */
INT32 c_net_wlan_p2p_disassociate(const char *mac);

INT32 c_net_wlan_get_curr_connection_info(NET_802_11_BSS_INFO_T *pCurrBss);
INT32 c_net_wlan_get_curr_wireless_type(NET_802_11_T *wireless_type);

INT32 c_net_wlan_p2p_enable_nat( const char * if_name , const char * dns);
INT32 c_net_wlan_p2p_disable_nat(void);

/*------------------------------------------------------------------*/
/*! @brief: Get all network ids of persistent networks
*  @param [in] networkId_buf - the address of buffer stored network_id
*  @param [in] networkIdbuflen - the length of the buffer stored network_id
*  @param [out] networkNum - the address of total number of persistent table
*  @return     result
*  @retval     0 -success
*  @retval     otherwise -failure 
*/
/*------------------------------------------------------------------*/
INT32 c_net_wlan_p2p_cfg80211_get_persistent_network_ids(int *networkId_buf, 
    int networkIdbuflen, int *networkNum);

/*------------------------------------------------------------------*/
/*! @brief: Get mode of P2P network with specified network id
*  @param [in] network_id - the network id of P2P network which want to get the mode
*  @param [out] mode - the network mode, 3: GO
*  @return     result
*  @retval     0 -success
*  @retval     otherwise -failure 
*/
/*------------------------------------------------------------------*/
INT32 c_net_wlan_p2p_cfg_get_network_mode(int network_id, int *mode);


/*------------------------------------------------------------------*/
/*! @brief: Get p2p client list of a P2P network
*  @param [in] network_id - the network id of specified P2P network
*  @param [out] client_list - the buffer of p2p client list, the length is 180 or more.
*  @return     result
*  @retval     0 -success
*  @retval     otherwise -failure 
*/
/*------------------------------------------------------------------*/
INT32 c_net_wlan_p2p_cfg_get_client_list(int network_id, 
            char *client_list);


/*------------------------------------------------------------------*/
/*! @brief: Set priority of P2P and STA, when P2P connection is conflict with STA
*  @param [in] iface - the interface prefer to keep when there is a conflict, 0: STA, 1: P2P
*  @return     result
*  @retval     0 -success
*  @retval     otherwise -failure 
*/
/*------------------------------------------------------------------*/
INT32 c_net_wlan_p2p_cfg_set_connection_prefer(int iface);


/*------------------------------------------------------------------*/
/*! @brief: Remove all P2P network with specified network id
*  @param [in] network_id - the pointer of network id, ("0", "1", "2" ... or "all")
*  @return     result
*  @retval     0 -success
*  @retval     otherwise -failure 
*/
/*------------------------------------------------------------------*/
INT32 c_net_wlan_p2p_cfg_remove_network(char *network_id);

/*------------------------------------------------------------------*/
/*! @brief: Save configuration for wpa_supplicant
*  @param - none
*  @return     result
*  @retval     0 -success
*  @retval     otherwise -failure 
*/
/*------------------------------------------------------------------*/
INT32 c_net_wlan_p2p_cfg_save_conf(void);


/*------------------------------------------------------------------*/
/*! @brief enable/disable mutli channel support.
 *
 *  @param [in]   BOOL enable: true/false
 *  @return     result
 *  @retval     0 -success
 *  @retval     otherwise -failure
 */
/*------------------------------------------------------------------*/

INT32 c_net_wlan_set_multi_channel(BOOL enable);


/*------------------------------------------------------------------*/
/*! @brief reset (stop and then start)dhcpd server.
 *
 *  @param [in]   dev    the device which dhcpd server on .
 *  @return     result
 *  @retval     0 -success
 *  @retval     otherwise -failure
 */
/*------------------------------------------------------------------*/
INT32 c_net_wlan_start_p2p_dhcpd_sever(char * dev , NET_DHCP_CONFIG dhcps_cfg );
/*------------------------------------------------------------------*/
/*! @brief start dhcpd server.
 *
 *  @param [in]   dev    the device which dhcpd server on .
 *  @return     result
 *  @retval     0 -success
 *  @retval     otherwise -failure
 */
/*------------------------------------------------------------------*/


INT32 c_net_wlan_start_p2p_dhcpd_sever_only(char * dev , NET_DHCP_CONFIG dhcps_cfg );


/*------------------------------------------------------------------*/
/*! @brief stop dhcpd server.
 *
 *  @param [in]   void
 *  @return     result
 *  @retval     0 -success
 *  @retval     otherwise -failure
 */
/*------------------------------------------------------------------*/
INT32 c_net_wlan_stop_p2p_dhcpd_sever(void );

/*------------------------------------------------------------------*/
/*! @brief set ap as auto go mode .
 *
 *  @return     result
 *  @retval     0 -success
 *  @retval     otherwise -failure
 */
/*------------------------------------------------------------------*/

INT32 c_net_wlan_set_auto_go(int fgEnableAutoGo, const char * ssid, const char * psk);

/*------------------------------------------------------------------*/
/*! @brief push button menu .
 *
 *  @return     result
 *  @retval     0 -success
 *  @retval     otherwise -failure
 */
/*------------------------------------------------------------------*/


INT32 c_net_wlan_press_push_button(void);

//#endif

/*------------------------------------------------------------------*/
/*! @brief Create wlan task. This routine initializes the wlan task.
 *              It must be called before calling c_net_network_init
 *  @return result
 *  @retval     0 -success
 *  @retval     otherwise -failure
 */
/*------------------------------------------------------------------*/
INT32 c_net_wlan_task_create(void);


/*------------------------------------------------------------------*/
/*! @brief Set Country region ABand.
 *  @return result
 *  @retval     0 -success
 *  @retval     otherwise -failure
 */
/*------------------------------------------------------------------*/
INT32 c_net_wlan_wpa_set_region_ABand(int i4ABand);


/*------------------------------------------------------------------*/
/*! @brief Get signal srength, freq and so on after Infra connection established.
 *  @param [out] pSignal -the signal data of connection
 *  @return result
 *  @retval    0 -success
 *  @retval    otherwise -failure
 */
/*------------------------------------------------------------------*/
INT32 c_net_wlan_wpa_signal_poll(NET_802_11_SIGNAL_DATA_T *pSignal);


/*------------------------------------------------------------------*/
/*! @brief Get WPA status. This routine retrieves the WPA status.
 *  @param [out] pStatus -WPAStatus "WPA Status"
 *  @return result
 *  @retval    0 -success
 *  @retval    otherwise -failure
 */
/*------------------------------------------------------------------*/


INT32 c_net_wlan_wpa_get_status(INT32 *pStatus,NET_802_11_ASSOCIATE_T *pAssociate);



/*------------------------------------------------------------------*/
/*! @brief Transform channel to frequency.
 *  @param [in] channel - channel number which need to be transformed.
 *  @return     frequency value
 *  @retval     -1 -failure
 *  @retval     otherwise -success
*/
/*------------------------------------------------------------------*/
INT32 c_net_wlan_channel_to_frequency(int channel);


/*------------------------------------------------------------------*/
/*! @brief Transform frequency to channel.
 *  @param [in] freq - frequency value which need to be transformed.
 *  @return     channel number
 *  @retval     -1 -failure
 *  @retval     otherwise -success
*/
/*------------------------------------------------------------------*/
INT32 c_net_wlan_frequency_to_channel(int freq);


/*------------------------------------------------------------------*/
/*! @brief Set the time in seconds between scans to find suitable AP.
 *  @param [in] scan_interval -The scan interval in seconds will be set.
 *  @return     result
 *  @retval     0 -success
 *  @retval     otherwise -failure
*/
/*------------------------------------------------------------------*/
INT32 c_net_wlan_set_scan_interval(INT32 scan_interval);


/*------------------------------------------------------------------*/
/*! @brief Scan network.This routine scans infrastructure and adhoc networks.
 *  @param [in] pfNotify -callback function to notify scan completion.
 *  @return     result
 *  @retval     0 -success
 *  @retval     otherwise -failure
*/
/*------------------------------------------------------------------*/
INT32 c_net_wlan_scan(x_net_wlan_notify_fct pfNotify);


/*------------------------------------------------------------------*/
/*! @brief When P2P is active, Infra will perform scan by interval 2s, 4s, 8s, 16s, 30s
 *  @param - none
 *  @return     result
 *  @retval     0 -success
 *  @retval     otherwise -failure
*/
/*------------------------------------------------------------------*/
INT32 c_net_wlan_set_auto_scan(void);


/*------------------------------------------------------------------*/
/*! @brief Scan network.This routine scans infrastructure and adhoc networks 
with specify.
 *  @param [in] pfNotify -callback function to notify scan completion.
 *  @param [in] ssid -the ssid than you want to scan.
 *  @return     result
 *  @retval     0 -success
 *  @retval     otherwise -failure
 */
/*------------------------------------------------------------------*/
INT32 c_net_wlan_scan_with_ssid(x_net_wlan_notify_fct pfNotify, char * ssid, int ssid_len );

/*------------------------------------------------------------------*/
/*! @brief Get scan result.This routine is called to get the scan result.All the found BSSs are listed in the list.
 *  @param [in,out] pScanResult -list of scan result(BSSs)
 *  @return     result
 *  @retval     0 -success
 *  @retval     otherwise -failure
 */
/*------------------------------------------------------------------*/
INT32 c_net_wlan_get_scan_result(NET_802_11_SCAN_RESULT_T *pScanResult);

/*------------------------------------------------------------------*/
/*! @brief Get a list of ESSs.This routine lists the ESSs enties and sorts in descending order
 *  of the signal strength. The stealth networks are attached the BSSIDs.
 *  Adhoc networks are not listed in the table.
 *  @param [in,out]  pEssList -list of ESSs
 *  @return     result
 *  @retval     0 -success
 *  @retval     otherwise -failure
 */
/*------------------------------------------------------------------*/
INT32 c_net_wlan_get_ess_list(NET_802_11_ESS_LIST_T *pEssList);


/*------------------------------------------------------------------*/
/*! @brief Set ssid to driver ( for associate with a hidde ssid )
 *  @param [in]  iface -the reference of interface, such as "ra0"
 *  @param [in]  ssid -the reference of ssid
 *  @return     result
 *  @retval     0 -success
 *  @retval     otherwise -failure
 */
/*------------------------------------------------------------------*/
INT32 c_net_wlan_set_ssid_to_driver(char *iface, char *ssid);


INT32 c_net_wlan_enable_network(void);


/*------------------------------------------------------------------*/
/*! @brief Associate with a wireless network.
 *  This routine triggers attempts of connecting to a wireless network according
 *  to the specified settings. After calling this routine, the WLAN
 *  software continues to maintain a network connection according
 *  to the specified settings until c_net_wlan_disassociate() is called.
 *  @param [in] pAssociate -association settings
 *  @param [in] pfNotify -callback function to notify the association status.
 *  @return     result
 *  @retval     0 -success
 *  @retval     otherwise -failure
 */
/*------------------------------------------------------------------*/
INT32 c_net_wlan_associate(NET_802_11_ASSOCIATE_T *pAssociate,
                           x_net_wlan_notify_fct pfNotify);



/*------------------------------------------------------------------*/
/*! @brief Enable/disable STA automatic reconnection.
 *  @param [in] enableFlag - the flag of enable or disable, 1: enable, 0: disable
 *  @return     result
 *  @retval     -1 -failure
 *  @retval     otherwise -success
*/
/*------------------------------------------------------------------*/
INT32 c_net_wlan_set_sta_auto_reconnect(int enableFlag);



/*------------------------------------------------------------------*/
/*! @brief Start STA reconnection.
 *  @return     result
 *  @retval     -1 -failure
 *  @retval     otherwise -success
*/
/*------------------------------------------------------------------*/
INT32 c_net_wlan_sta_reconnect(void);


/*------------------------------------------------------------------*/
/*! @brief Disassociate a wireless network connection.
 *  @return     result
 *  @retval     0 -success
 *  @retval     otherwise -failure
 */
/*------------------------------------------------------------------*/
INT32 c_net_wlan_disassociate(void);


/*------------------------------------------------------------------*/
/*! @brief Get currently WEP authentication mode.
 *  This routine gets the currently WEP authentication mode (SHARED or OPEN).
 *  this routine should be called after connection has been established.
 *  @param [in,out] pWepAuthMode -WEP authentication mode
 *  @return     result
 *  @retval     0 -success
 *  @retval     otherwise -failure
 */
/*------------------------------------------------------------------*/
INT32 c_net_wlan_get_wep_auth_mode(NET_802_11_AUTH_MODE_T *pWepAuthMode);


/*------------------------------------------------------------------*/
/*! @brief Get the currently associated BSS information.
 *  This routine gets the information of the associated BSS.
 *  If this routine is called while connection is not established,
 *  zero-BSSID (00:00:00:00:00:00) is returned.
 *  @param [in,out] pCurrBss -associated BSS information
 *  @return     result
 *  @retval     0 -success
 *  @retval     otherwise -failure
 */
/*------------------------------------------------------------------*/
INT32 c_net_wlan_get_curr_bss(NET_802_11_BSS_INFO_T *pCurrBss);


/*------------------------------------------------------------------*/
/*! @brief Get the currently connection information.
 *  This routine gets the information of currently connection.
 *  @param [in,out] pCurrBss -associated BSS information
 *  @return     result
 *  @retval     0 -success
 *  @retval     otherwise -failure
 */
/*------------------------------------------------------------------*/
INT32 c_net_wlan_wpa_get_connection_info(NET_802_11_BSS_INFO_T * pCurrBss);



/*------------------------------------------------------------------*/
/*! @brief Get WLAN MAC address.
 *  This routine gets MAC address of WLAN interface described in binary form.
 *  @param [out] pMacAddr -buffer to store MAC address
 *  @return     result
 *  @retval     0 -success
 *  @retval     otherwise -failure
 */
/*------------------------------------------------------------------*/
INT32 c_net_wlan_get_mac_addr(UINT8 *pMacAddr);



/*------------------------------------------------------------------*/
/*! @brief Get current AP 's Rssi.
 *  This routine get current connecting AP's Raw RSSI(-dbm).
 *  @param [int8_t] pRssi -buffer to Raw Rssi
 *  @return     result
 *  @retval     0 -success
 *  @retval     otherwise -failure

 */
/*------------------------------------------------------------------*/
INT32 c_net_wlan_get_rssi(NET_802_11_WIFI_RSSI_T *pRssi);



/*------------------------------------------------------------------*/
/*! @brief Set WLAN MAC address.
 *  This routine sets MAC address of WLAN interface described in binary form.
 *  @param [in] pMacAddr -buffer to store MAC address
 *  @return     result
 *  @retval     0 -success
 *  @retval     otherwise -failure

 */
/*------------------------------------------------------------------*/
INT32 c_net_wlan_set_mac_addr(UINT8 *pMacAddr);


/*------------------------------------------------------------------*/
/*! @brief Set Country Region, ABand Region, enable/disable channel list seperate for P2P.
 *  This routine sets Country Region, ABand Region, enable/disable channel list seperate for P2P.
 *  @param [in] iface -buffer to store interface, ra0 or p2p0.
 *  @param [in] pCountryRegion -buffer to store Country Region
 *  @param [in] pABandRegion -buffer to store ABand Region
 *  @param [in] pFgChListSepe -buffer to store Channel List Seperate enable/disable flag, 0 or 1.
 *  @param [in] pPassiveScanABand -buffer to store ABand Passive Scan enable/disable flag, 0 or 1.
 *  @return     result
 *  @retval     0 -success
 *  @retval     otherwise -failure

 */
/*------------------------------------------------------------------*/
INT32 c_net_wlan_set_region_channel_seperate(char *iface, INT32 *pCountryRegion, 
                INT32 *pABandRegion, INT32 *pFgChListSepe, INT32 *pPassiveScanABand);



/*------------------------------------------------------------------*/
/*! @brief Get STA bandwidth (ra0 and p2p0).
 *  This routine gets STA bandwidth.
 *  @param [out] pBW -buffer to store bandwidth, 20: 20MHz, 40: 40MHz.
 *  @return     result
 *  @retval     0 -success
 *  @retval     otherwise -failure

 */
/*------------------------------------------------------------------*/
INT32 c_net_wlan_get_STA_bandwidth(INT32 *pBW);




/*------------------------------------------------------------------*/
/*! @brief Get WLAN EEPROM data.
 *  This routine gets the EEPROM data of WLAN device.
 *  @param [in,out] pEeprom -EEPROM address and data buffer.
 *  @return     result
 *  @retval     0 -success
 *  @retval     otherwise -failure
 */
/*------------------------------------------------------------------*/
INT32 c_net_wlan_get_eeprom(NET_802_11_EEPORM_T *pEeprom);

/*------------------------------------------------------------------*/
/*! @brief Register WPA callback function.
 *  @param [in] pfNotify -callback function to notify the association status.
 *  @return     result
 *  @retval     0 -success
 *  @retval     otherwise -failure
 */
/*------------------------------------------------------------------*/
INT32 c_net_wlan_wpa_reg_cbk(x_net_wlan_notify_fct pfNotify);
/*------------------------------------------------------------------*/
/*! @brief WPS registration and association.
 *  This routine obtains association settings from a WPS registrer,
 *  save it in the disk and associate with the network.
 *  @param [in] eWpsMode -WPSOpMode "WPS Mode" WPS registration method.
 *  @param [out] pfNotify -callback function to notify WPS status.
 *  @return     result
 *  @retval     0 -success
 *  @retva      otherwise -failure
 */
/*------------------------------------------------------------------*/
INT32 c_net_wlan_wps_connect(NET_802_11_WPS_ASSOC_MODE_T eWpsMode, char *pin, UINT8 *bssid, 
                                         x_net_wlan_notify_fct pfNotify);

/*------------------------------------------------------------------*/
/*! @brief WPS registration and association.
 *  This routine obtains association settings from a WPS registrer,
 *  save it in the disk and associate with the network.
 *  @param [in] eWpsMode -WPSOpMode "WPS Mode" WPS registration method.
 *  @param [out] pfNotify -callback function to notify WPS status.
 *  @return     result
 *  @retval     0 -success
 *  @retva      otherwise -failure
 */
/*------------------------------------------------------------------*/
INT32 c_net_wlan_wps_associate(NET_802_11_WPS_ASSOC_MODE_T eWpsMode,
                               x_net_wlan_notify_fct pfNotify);


/*------------------------------------------------------------------*/
/*! @brief Get the status of a WPS process.
 *  This routine gets the current WPS status.
 *  @param [out] status -WPSStatus "WPS status"
 *  @return    result
 *  @retval     0 -success
 *  @retval     otherwise -failure
 */
/*------------------------------------------------------------------*/
INT32 c_net_wlan_wps_get_status(INT32 *status);

/*------------------------------------------------------------------*/
/*! @brief Cancel WPS registration process.
 *  This routine tiriggers cancellation of WPS registration process.
 *  @return     result
 *  @retval     0 -success
 *  @retval     otherwise -failure
 */
/*------------------------------------------------------------------*/
INT32 c_net_wlan_wps_cancel(void);

/*------------------------------------------------------------------*/
/*! @brief Refresh WPS PIN code.
 *  This routine refreshes WPS PIN code.
 *  @return     result
 *  @retval     0 -success
 *  @retval     otherwise -failure
 */
/*------------------------------------------------------------------*/
INT32 c_net_wlan_wps_refresh_pin(void);

/*------------------------------------------------------------------*/
/*! @brief Get WPS PIN code.
 *  This routine gets WPS PIN code. The PIN code is not refreshed
 *  by calling this routine.
 *  @param [in]   len -length of the buffer
 *  @param [out] pin -buffer of PIN code
 *  @return     result
 *  @retval     0 -success
 *  @retval     otherwise -failure
 */
/*------------------------------------------------------------------*/
INT32 c_net_wlan_wps_get_pin(char *pin, UINT32 len);

/*------------------------------------------------------------------*/
/*! @brief Open Wlan connection for ifname.
 *  This routine is to open connection for specific ifname.
 *  @param [in]   ifname -interface name
 *  @return     result
 *  @retval     0 -success
 *  @retval     otherwise -failure
 */
/*------------------------------------------------------------------*/
INT32 c_net_wlan_open_connection(const char *ifname);

INT32 c_net_wlan_open_connection_per_path(const char *ifname, char *ctrl_path, char *moni_path);


/*------------------------------------------------------------------*/
/*! @brief Close Wlan connection.
 *  This routine is to close connection.
 *  @return     result
 *  @retval     0 -success
 *  @retval     otherwise -failure
 */
/*------------------------------------------------------------------*/
INT32 c_net_wlan_close_connection(void);

/*------------------------------------------------------------------*/
/*! @brief Get Wlan init state from wpaprog.
 *  This routine is to get Wlan init state when the net type is ethernet during Power-on .
 *  @return     result
 *  @retval     0 -init success
 *  @retval     otherwise -failure
 */
/*------------------------------------------------------------------*/
INT32 c_net_wlan_check_init_state(void);
/*--------------------------------------------------------------------*/
/*  @brief Generic RSSI Types
 *  This routing is to get rssi for every chain.
 *  @return      result
    @retval      0 get success
    @retval      otherwise -failure
*/

INT32 c_net_wlan_get_chain_rssi(NET_802_11_CHAIN_RSSI_T *pRssi);

/*--------------------------------------------------------------------*/
/*  @brief Custom data
 *  This routing is to get custom data
 *  @return      result
    @retval      0 get success
    @retval      otherwise -failure
*/
INT32 c_net_wlan_get_custdata(NET_802_11_ACFG_CUSTDAT_T * pCustdata);
/*----------------------------------------------------------------------------*/
/*! @brief Get Driver Version.
 *  This routine is to get driver version.
 *  @return     result
 *  @retval     0 -init success
 *  @retval     otherwise -failure
 */
/*------------------------------------------------------------------*/
INT32 c_net_wlan_get_wlan_version(void);
/*--------------------------------------------------------------------*/
/*
 *  This routing is set usb suspend
 *  @input  TRUE - enable usb suspend . FALSE - disable usb suspend
 *  @return      result
    @retval      0 get success
    @retval      otherwise -failure
*/

INT32 c_net_wlan_set_usb_suspend(BOOL enable);


/*----------------------------------------------------------------------------*/
/* @ API to start Iperf server (foxconn only)
  * @  param - none
  * @  retval - none
*/
/*----------------------------------------------------------------------------*/
#if 1//add API for Foxconn start Iperf server 
extern INT32 c_start_iperf_server(void);
extern INT32 c_start_iperf_client(const char *argv1, const char *argv2, const char *argv3);
#endif

/*--------------------------------------------------------------------*/
/*
 *  This routing is enable/disable wac config
 *  @input  TRUE - enable wac config . FALSE - disable wac config
 *  @return      result
    @retval      0 get success
    @retval      otherwise -failure
*/
INT32 c_net_wlan_enable_wac_config(BOOL enable);


/*----------------------------------------------------------------------------*/
/* @ API to monitor dhcpd lease time
  * @  param - Input callback function.
  * @  retval -
*/
/*----------------------------------------------------------------------------*/
INT32 c_net_wlan_monitor_dhcpd_lease(void* pIn);


/*------------------------------------------------------------------*/
/*! @brief: select WLAN antenna
*  @param [in] iface - the pointer of interface, such as "ra0"
*  @param [in] option - the option of antenna selection, 
*      0: Antenna Diversity Disable
*      1: Antenna Diversity Enable
*      2: Antenna CON1 2.4G, CON2 5G/BT
*      3: Antenna CON1 5G/BT, CON2 2.4G
*      4: Software Antenna Diversity Enable (Default)
*  @return     result
*  @retval     0 -success
*  @retval     otherwise -failure 
*/
/*------------------------------------------------------------------*/
INT32 c_net_wlan_select_antenna(char *iface, int option);


/*------------------------------------------------------------------*/
/*! @brief: start or stop ATE
*  @param [in] iface - the pointer of interface, such as "ra0"
*  @param [in] flag - the flag to start or stop ATE, 0: stop, 1: start
*  @return     result
*  @retval     0 -success
*  @retval     otherwise -failure 
*/
/*------------------------------------------------------------------*/
INT32 c_net_wlan_ate_control( char *iface, int flag );


/*------------------------------------------------------------------*/
/*! @brief: Set parameters for TX
*  @param [in] iface - the pointer of interface, such as "ra0"
*  @param [in] parame - the pointer of parameter structure
*  @return     result
*  @retval     0 -success
*  @retval     otherwise -failure 
*/
/*------------------------------------------------------------------*/
INT32 c_net_wlan_ate_start_Tx( char *iface, NET_802_11_ATE_PARAMETER_T *param);


/*------------------------------------------------------------------*/
/*! @brief: Set parameters for RX
*  @param [in] iface - the pointer of interface, such as "ra0"
*  @param [in] parame - the pointer of parameter structure
*  @return     result
*  @retval     0 -success
*  @retval     otherwise -failure 
*/
/*------------------------------------------------------------------*/
INT32 c_net_wlan_ate_start_Rx( char *iface, NET_802_11_ATE_PARAMETER_T *param);


/*------------------------------------------------------------------*/
/*! @brief: Get statistics data of RX
*  @return     result
*  @retval     0 -success
*  @retval     otherwise -failure 
*/
/*------------------------------------------------------------------*/
INT32 c_net_wlan_ate_get_Rx_state(char *iface);


/*------------------------------------------------------------------*/
/*! @brief: initialize ATE for RF test
*  @param [in] iface - the pointer of interface, such as "ra0"
*  @return     result
*  @retval     0 -success
*  @retval     otherwise -failure 
*/
/*------------------------------------------------------------------*/
INT32 c_net_wlan_RF_initialize(char *iface);

/*------------------------------------------------------------------*/
/*! @brief: Set parameters for TX
*  @param [in] iface - the pointer of interface, such as "ra0"
*  @param [in] parame - the pointer of parameter structure
*  @return     result
*  @retval     0 -success
*  @retval     otherwise -failure
*/
/*------------------------------------------------------------------*/
INT32 c_net_wlan_RF_start_Tx( char *iface, NET_802_11_ATE_PARAMETER_T *param);

/*------------------------------------------------------------------*/
/*! @brief: Stop Tx
*  @param [in] iface - the pointer of interface, such as "ra0"
*  @return     result
*  @retval     0 -success
*  @retval     otherwise -failure
*/
/*------------------------------------------------------------------*/
INT32 c_net_wlan_RF_stop_Tx(char *iface);

/*------------------------------------------------------------------*/
/*! @brief: Set parameters for Rx
*  @param [in] iface - the pointer of interface, such as "ra0"
*  @param [in] parame - the pointer of parameter structure
*  @return     result
*  @retval     0 -success
*  @retval     otherwise -failure
*/
/*------------------------------------------------------------------*/
INT32 c_net_wlan_RF_start_Rx( char *iface, NET_802_11_ATE_PARAMETER_T *param);

/*------------------------------------------------------------------*/
/*! @brief: Stop Rx and get statistics data of Rx
*  @param [in] iface - the pointer of interface, such as "ra0"
*  @return     result
*  @retval     0 -success
*  @retval     otherwise -failure
*/
/*------------------------------------------------------------------*/
INT32 c_net_wlan_RF_stop_Rx(char *iface);

/** ======================== TDLS Functions ======================== **/

/*------------------------------------------------------------------*/
/*! @brief: Send TDLS discovery request.
*  @param [in] iface - the mac address of TDLS entry which want to discovery
*  @return     result
*  @retval     0 -success
*  @retval     otherwise -failure 
*/
/*------------------------------------------------------------------*/
INT32 c_net_wlan_tdls_discovery(WLAN_TDLS_Device_Identifier_t device);

/*------------------------------------------------------------------*/
/*! @brief: Send TDLS setup request.
*  @param [in] iface - the mac address of TDLS entry which want to setup
*  @return     result
*  @retval     0 -success
*  @retval     otherwise -failure 
*/
/*------------------------------------------------------------------*/
INT32 c_net_wlan_tdls_setup(WLAN_TDLS_Device_Identifier_t device);

/*------------------------------------------------------------------*/
/*! @brief: Send TDLS teardown request.
*  @param [in] iface - the mac address of TDLS entry which want to teardown
*  @return     result
*  @retval     0 -success
*  @retval     otherwise -failure 
*/
/*------------------------------------------------------------------*/
INT32 c_net_wlan_tdls_teardown(WLAN_TDLS_Device_Identifier_t device);

/*------------------------------------------------------------------*/
/*! @brief: Get current TDLS WLAN status synchronously.
*  @param [out] iface - the buffer of storing TDLS Entry
*  @return     result
*  @retval     0 -success
*  @retval     otherwise -failure 
*/
/*------------------------------------------------------------------*/
INT32 c_net_wlan_tdls_get_status(WLAN_TDLS_Status_t *tdlsStatus);

/*------------------------------------------------------------------*/
/*! @brief cancel associate a wireless network connection.
 *  @return     result
 *  @retval     0 -success
 *  @retval     otherwise -failure 
 */
/*------------------------------------------------------------------*/
INT32 c_net_wlan_cancel_associate(void);


#endif /* _C_NET_WLAN_H */
