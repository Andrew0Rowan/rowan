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



#ifndef C_NET_CONFIG_H
#define C_NET_CONFIG_H

#include "u_common.h"
#include "u_net_common.h"
#include "u_dhcpc_api.h"
#include "u_net_ni_if.h"
//#include "u_http_firmware_upgrade.h"
#include "u_ipcd.h"


typedef struct _NET_SPEED_T
{
    INT32 rx_speed;
    INT32 tx_speed;
}NET_SPEED_T;

typedef enum
{
    NET_AF_IPV4 = 0,
    NET_AF_IPV6,   
} NET_ADDR_FAMILY_TYPE;

typedef struct
{
    NET_ADDR_FAMILY_TYPE eAddrFamily;
    char *ps_server_url;    
} NET_ADDR_INFO_LOOK_UP;
typedef VOID (*get_ni_speed_cb_fp)(NET_SPEED_T * speed);
typedef VOID (*connection_test_cb_fp)(INT32 result);





extern INT32    c_net_network_init(INT32 i4IfType);
extern INT32    c_net_network_deinit(VOID);
extern INT32    c_net_get_wifi_ni_name(CHAR *psz_ni_name);
extern INT32    c_net_get_mac_by_ip(const CHAR * ps_ip, UCHAR * puac_mac);
extern INT32    c_net_get_ni_speed(const CHAR * ifname, get_ni_speed_cb_fp cb_fp);

extern INT32    c_net_ip_config(CHAR *psz_name, UINT32 ui4_address, UINT32 ui4_netmask, UINT32 ui4_gw);
extern INT32    c_net_dns_config(UINT32 ui4_dns1, UINT32 ui4_dns2);
extern VOID     c_net_set_hostname(CHAR *ps_hostname);
extern VOID     c_net_clean_occupied_socket(INT32 h_thread_id);
extern INT32    c_net_autoip_config(CHAR *psz_name, CHAR *pc_mac);
#if 1// CONFIG_SUPPORT_SS
extern INT32 c_dhcpc_en_dns(void);
extern INT32 c_dhcpc_dis_dns(void);
extern INT32 c_dhcpc_get_dns_status(void);
extern struct hostent *    c_net_gethostbyname_ex(const CHAR *ps_server_url); // mtk70199_0913
extern struct hostent *    c_net_gethostbyname_timeout(const CHAR *ps_server_url,INT32 timeout);

#endif
/* Network diagnosis */

/**
 * c_net_dns_lookup
 *
 * dns lookup utility, user may use this API to verify dns
 * @param ps_server_url, dot format IPv4 address, ex "192.168.1.1", or dot domain name, ex "www.google.com"
 *
 * @return NET_OK if success, NET_FAILED if fail
 */
extern INT32    c_net_dns_lookup(const NET_ADDR_INFO_LOOK_UP *ptAddrinfolookup);

/**
 * c_net_proxy_connection
 *
 * proxy connection utility, user may use this API to check proxy connection
 * @param ps_server_url, dot format IPv4 address, ex: "192.168.1.1", or dot domain name, ex: "www.google.com"
 * @param ui2_port, port number, ex: 80
 * @return NET_OK if success, NET_FAILED if lookup fail or connection fail
 */
extern INT32    c_net_proxy_connection(const CHAR *ps_server_url, UINT16 ui2_port);
extern INT32    c_net_test_connection(const CHAR *ps_server_url, UINT16 ui2_port, UINT16 ui2_timeout);
extern INT32    c_net_test_connection_async(const CHAR *ps_server_url, UINT16 ui2_port, UINT16 ui2_timeout, connection_test_cb_fp cb_fp);

#ifdef _USE_LINUX
extern struct hostent *    c_net_gethostbyname(const CHAR *ps_server_url);
extern struct hostent *    c_net_gethostbyname_ex(const CHAR *ps_server_url); // samsung
extern struct hostent *    c_net_gethostbyname_timeout(const CHAR *ps_server_url,INT32 timeout);

extern INT32               c_net_pinghostbyname(const CHAR * ps_server_url);
extern INT32               c_net_module_load(VOID);
extern INT32               c_net_module_unload(VOID);
extern INT32               c_net_set_mac_to_sys(CHAR *pc_mac);
extern INT32               c_net_get_mac_from_sys(CHAR *pac_mac);
extern INT32               c_net_ip_conflict(CHAR *psz_name, UINT32 ui4_address, VOID (*notify_fn) (BOOL fgConflict));
extern int isSummerTime();
extern void c_set_time_zone();
#endif

extern INT32 c_net_ni_get_subnet(char *dev, unsigned long *ip_addr);
extern INT32 c_net_ni_get_gateway(char *dev, unsigned long *ip_addr);
extern INT32 c_net_ni_get_dns(unsigned long *dns1, unsigned long *dns2);

/* MPTool Server */
typedef VOID (* server_notify_fn)(INT32 evt);
extern INT32 c_net_mptool_server_startup(server_notify_fn cb_fn);

/* DLNA DHCP client */
extern INT32   c_dhcpc_init(VOID);
extern INT32   c_dhcpc_deinit(VOID);
extern INT32   c_dhcpc_start(CHAR *ps_ni_name, x_dhcpc_nfy_fct fn);
extern INT32   c_dhcpc_notify_restart(CHAR *ps_ni_name, x_dhcpc_nfy_fct fn);
extern INT32   c_dhcpc_get_info(CHAR *ps_ni_name, MT_DHCP4_INFO_T *pt_info);
extern INT32   c_dhcpc_restart(CHAR *ps_ni_name);
extern INT32   c_dhcpc_stop(CHAR *ps_ni_name);
extern INT32   c_net_ni_get_ip(char *dev, unsigned long *ip_addr);

//add for ipv6 test
extern int icmp_RS(char *iface);
extern INT32   _net_get_RA_bit(char* dev, int* ip_state, int* dns_state);



extern VOID c_net_ping_v4(CHAR * ps_ip,
                          INT32  i4_len,
                          INT16  i2_wait,
                          VOID (*notify_fn) (INT16 i2_rsp_time));

extern VOID c_dlna_net_ping_v4(CHAR * ps_ip,
                               INT16  i2_len,
                               INT16  i2_wait,
                               VOID* dlnaPingHandle,
                               VOID (*notify_fn)(VOID* h_dlna_notify, INT16 i2_rsp_time));

/* PPPoE functions */
typedef enum
{
    PPPOE_EVENT_SUCCESS     = 1,
    PPPOE_EVENT_FAILURE     = 2
} PPPOE_EV_T;

/* DNS functions */
typedef enum
{
    DHCP_EN_DNS     = 1,
    DHCP_DIS_DNS    = 0
} DHCP_DNS_STAT;



typedef VOID (*x_pppoe_nfy_fct) (PPPOE_EV_T event);

extern INT32 c_pppoe_start(CHAR * ifname, CHAR * username, CHAR * password, x_pppoe_nfy_fct cb_fn);
extern INT32 c_pppoe_stop();
extern INT32 c_pppoe_restart();
extern INT32 c_pppoe_get_info(CHAR *ps_ni_name, MT_DHCP4_INFO_T *pt_info);

/* NTP time sync functions */
typedef enum
{
    NTP_SYNC_TIME_SUCCESS     = IPCD_EXEC_SUCESS,
    NTP_SYNC_TIME_FAILURE     = IPCD_EXEC_FAILED
} NTP_EV_T;

typedef VOID (*x_ntp_nfy_fct) (NTP_EV_T event);
extern INT32 c_ntp_sync_time(CHAR * time_server, x_ntp_nfy_fct cb_fn);
extern INT32 c_time_sync(CHAR * time_server, x_ntp_nfy_fct cb_fn );

/* configure socket block mode */
extern int c_net_socket_nonblocking(int sock);
extern int c_net_socket_blocking(int sock);
extern int c_net_get_socket_mode(int sock);

/* NET NI C_api */
extern L2_SWITCH_T *c_net_switch_new(CHAR *ps_name);
extern INT32 c_net_switch_add_default_ni(L2_SWITCH_T *pt_Switch, INT32 i4_NiHandle);
extern INT32 c_net_switch_add_ni(L2_SWITCH_T *pt_Switch, INT32 i4_NiHandle);
extern INT32 c_net_switch_add_learn_entry (L2_SWITCH_T *pt_Switch, UINT8 *pac_Mac, INT32 i4_NiHandle);
extern INT32 c_net_swtich_find_learn_entry(L2_SWITCH_T *pt_Switch, UINT8 *pac_Mac);
extern INT32 c_net_switch_transmit_packet(L2_SWITCH_T *pt_Switch, PKT_BUFF_T *pt_packet, INT32 i4_NiHandle);
extern NET_NI_T *c_net_ni_new(CHAR *ps_name, INT32 i4_ifType);
extern INT32 c_net_ni_install_drv_entry(NET_NI_T *pt_ni, NetworkDriverEntry pf_fn);
extern INT32 c_net_ni_start(NET_NI_T *pt_ni);
extern INT32 c_net_ni_install_transmit_packet_callback(NET_NI_T *pt_ni, net_ni_transmit_pkt pf_fn);
extern INT32 c_net_ni_install_receive_packet_callback(NET_NI_T *pt_ni, net_ni_receive_pkt pf_fn);
extern INT32 c_net_ni_packet_indication(NET_DRV_IF_T *pt_drv, PKT_BUFF_T *pt_pkb);
extern INT32 c_net_ni_tx_completed(NET_DRV_IF_T *pt_drv, PKT_BUFF_T *pt_pkb);
extern INT32 c_net_ni_rx_packet(NET_NI_T *pt_ni, PKT_BUFF_T *pt_buff);
extern INT32 c_net_ni_tx_packet(NET_NI_T *pt_ni, PKT_BUFF_T *pt_buff);
extern NET_NI_T *c_net_ni_get_by_name(CHAR *ps_name);
extern VOID c_net_ni_list_reset(VOID);
extern INT32 c_net_ni_enable(CHAR *psz_name, INT8 u1EnableNi);
extern INT32 c_net_ni_set_mac(CHAR *psz_name, CHAR *pc_mac);
extern INT32 c_net_ni_get_mac(CHAR *psz_name, CHAR *pac_mac);
extern INT32 c_net_ni_set_if_type(NET_NI_T *pt_ni, INT32 i4_type);
extern NET_NI_T *c_net_ni_list_get(VOID);
extern INT32 c_net_ni_add_assocate_ni(NET_NI_T *pt_ni1, NET_NI_T *pt_ni2);
extern INT32 c_net_ni_get(CHAR *ps_niName, INT32 i4_cmd, VOID *pv_param);
extern INT32 c_net_ni_set(CHAR *ps_niName, INT32 i4_cmd, VOID *pv_param);
extern INT32 c_net_if_join(const CHAR* ps_niName, UINT32 ui4_ip);
extern INT32 c_net_if_leave(const CHAR* ps_niName, UINT32 ui4_ip);
extern INT32 c_net_if_multi_list(const CHAR* ps_niName, UINT32 *paui4_ip, INT32 i4_size);
extern INT32 c_net_ni_reg_ev_notify(CHAR *ps_ni_name, NI_DRV_EV_T e_ev, ni_ev_notify_fnct pf_notify);
extern INT32 c_net_ni_unreg_ev_notify(CHAR *ps_ni_name, NI_DRV_EV_T e_ev, ni_ev_notify_fnct pf_notify);
extern INT32 c_net_ni_ev_send(NET_NI_T *pt_ni, NI_DRV_EV_T e_ev);
extern INT32 c_net_ni_set_speed(CHAR * psz_name, NI_SPEED_T ni_speed);
extern INT32 c_net_ni_get_speed_duplex(CHAR *psz_name, NI_SPEED_T *pni_speed);


extern INT32 c_net_ni_get_subnet(char *dev, unsigned long *ip_addr);
extern INT32 c_net_ni_get_gateway(char *dev, unsigned long *ip_addr);
extern INT32 c_net_ni_get_dns(unsigned long *dns1, unsigned long *dns2);
extern INT32 c_net_mdio_ctrl( UINT32 fgSet, UINT32 reg, UINT16 *value);
extern 	INT32 c_net_socket_to_ifname(int socket, char *ifname);
extern 	INT32 c_net_ip_to_ifname(int inaddr, char* ifname);
extern VOID c_net_clean_dns_cache(VOID);
extern int c_net_getNationCode(char* Account,  char* IPAddr,  char *CountryCode);

extern INT32 c_net_phy_ctrl(const CHAR *ifname, UINT16 fgSet, UINT16 Prm, UINT16 value);

/*
extern VOID                         c_http_cmd_abort(HTTP_CTX_T *pt_ctx);
extern VOID                         c_http_cmd_del_ctx(HTTP_CTX_T *pt_ctx);
extern UINT32                       c_http_cmd_get_content_length(HTTP_CTX_T *pt_ctx);
extern CHAR *                       c_http_cmd_get_ctx_tag_result(HTTP_CTX_T *pt_ctx, CHAR *ps_tag_name);
extern HTTP_RET_CODE_T              c_http_cmd_get_method(HTTP_CTX_T *pt_ctx);
extern UINT32                       c_http_cmd_get_recvd_size(HTTP_CTX_T *pt_ctx);
#if CONFIG_MW_CUSTOM_KLG
extern HTTP_CTX_T *                 c_http_cmd_new_ctx(CHAR *ps_url, x_http_fw_dn_read_nfy fn, HTTP_DN_POOL *pool);
#else
extern HTTP_CTX_T *                 c_http_cmd_new_ctx(CHAR *ps_url, x_http_fw_dn_read_nfy fn, CHAR *pac_buffer, INT32 i4_ac_buffer_size);
#endif
extern HTTP_RET_CODE_T              c_http_cmd_post_method(HTTP_CTX_T *pt_ctx, CHAR *ps_request_content);
extern VOID                         c_http_cmd_upgrade_module_init(VOID);
extern INT32                        c_http_cmd_fix_non_base64_char(CHAR* pv_base64_input, INT32 i4_len);
extern HTTP_RET_CODE_T              c_http_cmd_parse_xml(HTTP_CTX_T *pt_ctx, VOID *pv_in_xml, INT32 i4_in_xml_size);
extern HTTP_RESPONSE_STATUS_CODE_T  c_http_cmd_get_rsp_status(HTTP_CTX_T *pt_ctx);
*/
/**
 * Base64 utility
 */
extern INT32 c_base64_encode(const VOID *pv_in_data, INT32 i4_pv_in_data_size, VOID *pac_out_data, INT32 i4_pac_out_data_size);
extern INT32 c_base64_decode(const VOID* ps_in_str, INT32 i4_ps_in_str_len, VOID *pv_out_data, INT32 i4_pv_out_data_size);

/**
 * Query software version
 */
extern CHAR *c_ave_tcp_tag(VOID);
extern CHAR *c_nflc_dlna_tag(VOID);

extern INT32 c_net_get_mac_by_ip (const CHAR *ps_ip, UCHAR *puac_mac);
extern INT32 c_net_ip_to_mac(char* in_addr, char* out_mac);

extern int c_net_get_netstack_type(void);



/*******
 * for ipv6 -- app use
 *******/
extern INT32 c_net_ni_set_ip_v6(char *dev, char* ipsrc, int v_prefix);
extern INT32 c_net_ni_set_gw_v6(char* dev, char* gwaddr);
extern INT32 c_net_ni_start_dhcpv6(int ip_state, int dns_state, char *dev);
extern INT32 c_net_ni_rmv6_gw(char* dev);
extern INT32 c_net_ni_rmv6config_all(char* dev);
extern INT32 c_net_ni_rmv6config_SLAAC(char* dev);


/*******
 * for ipv6 -- internal use
 *******/
extern INT32 c_net_is_RA_rcvd(char* dev);
extern INT32  c_net_ni_get_ip_v6(char *dev, int addr_type, char* ip_asc, int* prefix);
extern INT32  c_net_ni_get_gw_v6(char* dev, char* gwaddr);
extern INT32  c_net_get_dns_v6(char* dns1, char* dns2);

extern INT32 c_net_ni_stop_dhcpv6(char *dev);
extern INT32 c_net_ni_rmv6config_manual(char* dev);
extern INT32 c_net_ni_start_config_ipv6(char *dev, void* pIn);

extern INT32 c_net_ni_disable_ipv6(char *dev);
extern INT32 c_net_ni_enable_ipv6(char *dev);
extern INT32 c_net_ni_ipv6_setKernDisable(char *dev);


extern INT32 c_net_IsPreferIpv6(int flag);
extern INT32 c_net_proxy_connection_v6(const CHAR *ps_server_url, UINT16 ui2_port);
extern INT32 c_net_test_connection_v6(const CHAR *ps_server_url, UINT16 ui2_port, UINT16 ui2_timeout);
extern 	int c_net_getaddrinfo(char* url, char* ipstr);

extern VOID c_net_ping_v6(
                  CHAR *ps_interface_name,
                  CHAR *ps_dest_ip,
                  INT32  i2_len,
                  INT16  i2_wait,
                  VOID (*notify_fn) (INT16 i2_rsp_time));

extern int _net_IsValidIpv6Addr(char* ipstr);


extern int c_net_IsValidIpv6Addr(char* ipstr);
extern int c_net_IsValidIpv6Ip(char* ipstr);

extern INT32 c_net_get_ipv6_info(CHAR *ps_ni_name, MT_IPV6_INFO_T *pt_info);

/**
 * Wake On Lan
 */
typedef VOID (*wol_notify_cb_fn) (VOID);
 
INT32 c_net_start_wol_thread(CHAR *psz_name, wol_notify_cb_fn cb_fn);
INT32 c_net_stop_wol_thread(VOID);
VOID c_net_get_netlink_message(void*  pv_arg);
INT32 c_net_start_get_netlink_thread(VOID);
INT32 c_net_stop_get_netlink_thread(VOID);

#if CONFIG_MW_CUSTOM_SFLP
extern BOOL mpserver_is_pctool_upgrade(VOID);
extern VOID mpserver_set_pctool_upgrade(BOOL flag);
extern INT32 pctool_upg_register(x_pctool_upg_nfy pf_nfy);
#endif

#endif
