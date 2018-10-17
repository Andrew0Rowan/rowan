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


   /*-----------------------------------------------------------------------------
                       include files
    ----------------------------------------------------------------------------*/
#include <stdio.h>
#include <string.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>
#include <stdlib.h>
#include <math.h>

#include "c_net_config.h"
#include "u_cli.h"
#include "u_os.h"
#include "u_ipcd.h"
//#include "net_define.h"
//#include "x_mw_config.h"

   /*-----------------------------------------------------------------------------
                       macros, defines, typedefs, enums
    ----------------------------------------------------------------------------*/

#ifdef CLI_SUPPORT

   /*-----------------------------------------------------------------------------
                       functions declaraions
    ----------------------------------------------------------------------------*/
   static VOID net_dhcpok_cb(DLNA_DHCPC_EV_T e_event);
   static INT32 cli_c_net_ip_config(INT32 i4_argc, const CHAR**  pps_argv);
   static INT32 cli_c_net_set_hostname(INT32 i4_argc, const CHAR**  pps_argv);
   static INT32 cli_c_net_dns_config(INT32 i4_argc, const CHAR**  pps_argv);
   static INT32 cli_c_net_dns_lookup(INT32 i4_argc, const CHAR**  pps_argv);
   static INT32 cli_c_net_proxy_connection(INT32 i4_argc, const CHAR**  pps_argv);
   static INT32 cli_c_dhcpc_start(INT32 i4_argc, const CHAR**  pps_argv);
   static INT32 cli_c_dhcpc_stop(INT32 i4_argc, const CHAR**  pps_argv);
   static INT32 cli_c_dhcpc_restart(INT32 i4_argc, const CHAR**  pps_argv);
   static INT32 cli_c_dhcpc_get_info(INT32 i4_argc, const CHAR**  pps_argv);
   static INT32 cli_ping(INT32 i4_argc, const CHAR**  pps_argv);
   static INT32 ui_ni_monitor_init(INT32 i4_argc, const CHAR**  pps_argv);
   static INT32 cli_clean_dns_cache_debug(INT32 i4_argc, const CHAR**  pps_argv);
   static INT32 cli_c_net_ni_set_mac(INT32 i4_argc, const CHAR**  pps_argv);
   static INT32 cli_c_net_ni_get_mac(INT32 i4_argc, const CHAR**  pps_argv);
   static INT32 cli_c_net_ni_set_mac_sys(INT32 i4_argc, const CHAR**  pps_argv);
   static INT32 cli_c_net_ni_get_mac_sys(INT32 i4_argc, const CHAR**  pps_argv);
   static INT32 cli_c_net_ni_invoke_telnetd(INT32 i4_argc, const CHAR**  pps_argv);
   static INT32 cli_get_local_host_ip(INT32 i4_argc, const CHAR**  pps_argv);
   static INT32 cli_get_host_by_name(INT32 i4_argc, const CHAR**  pps_argv);
   static INT32 cli_c_net_autoip_config(INT32 i4_argc, const CHAR**  pps_argv);
   static INT32 cli_c_net_ip_conflict(INT32 i4_argc, const CHAR**  pps_argv);
   static INT32 cli_c_net_dbg_msg_toggle(INT32 i4_argc, const CHAR** pps_argv);
   static INT32 cli_c_net_get_wifi_ni_name(INT32 i4_argc, const CHAR**  pps_argv);
   static INT32 cli_c_net_mptool_server_startup(INT32 i4_argc, const CHAR**  pps_argv);
   static INT32 cli_c_net_pinghostbyname(INT32 i4_argc, const CHAR**  pps_argv);
   static INT32 cli_c_net_test_connection(INT32 i4_argc, const CHAR**  pps_argv);
   static INT32 cli_c_net_get_mac_by_ip(INT32 i4_argc, const CHAR**  pps_argv);
   static INT32 cli_c_net_get_ni_speed(INT32 i4_argc, const CHAR**  pps_argv);
   static INT32 cli_c_net_phy_ctrl(INT32 i4_argc, const CHAR**  pps_argv);
   static INT32 cli_c_net_start_wol_thread(INT32 i4_argc, const CHAR**  pps_argv);
   static INT32 cli_c_net_stop_wol_thread(INT32 i4_argc, const CHAR**  pps_argv);

   static INT32 cli_c_pppoe_start(INT32 i4_argc, const CHAR**  pps_argv);
   static INT32 cli_c_pppoe_stop(INT32 i4_argc, const CHAR**  pps_argv);
   static INT32 cli_c_debug_print_switch(INT32 i4_argc, const CHAR**  pps_argv);
   static INT32 cli_c_ntp_sync_time(INT32 i4_argc, const CHAR**  pps_argv);
   static INT32 cli_c_set_ipcd(INT32 i4_argc,  const CHAR**	pps_argv);
	   static INT32 cli_c_get_ipcd(INT32 i4_argc,  const CHAR**	pps_argv);

   
   static INT32 cli_icmp_RS(INT32 i4_argc, const CHAR**  pps_argv);
   static INT32 cli_net_get_RA_bit(INT32 i4_argc, const CHAR**  pps_argv);

   //ipv6
   static INT32 cli_c_net_read_snr(INT32 i4_argc, const CHAR**  pps_argv);
   static INT32 cli_c_get_validMac(INT32 i4_argc, const CHAR**  pps_argv);
   
   static INT32 cli_c_net_get_speed_duplex(INT32 i4_argc, const CHAR**  pps_argv);

static INT32 cli_test_up(INT32 i4_argc, const CHAR**  pps_argv)
{
	putchar(0xf9);
	return 0;
}


   
   static INT32 cli_dhcp6_start(INT32 i4_argc, const CHAR**  pps_argv);
   static INT32 cli_dhcp6_stop(INT32 i4_argc, const CHAR**  pps_argv);

   static VOID ping_notify(INT16 rsptime)
	{
		printf("Received ping : %d ms\n", rsptime);
	}

	  
   static INT32 cli_ip6_show(INT32 i4_argc, const CHAR**  pps_argv)
   {
   		 printf("=============IP info============\n");
       system("ip -6 addr");
       printf("=============ROUTE info============\n");
       system("ip -6 route");
       printf("=============DNS info============\n");     
       system("cat /etc/resolv.conf");
       return CLIR_OK;
   }

   
   static INT32 cli_ip6_set(INT32 i4_argc, const CHAR**  pps_argv)
   {
	   char dev[20] = "eth0";
	   int prefix = 0;

       if (i4_argc < 3 )
       {
           printf("Usage: ip6_set ip prefix dev\n");
           return CLIR_OK;
       }
	   prefix = atoi(pps_argv[2]);
	   
       if (4 ==  i4_argc)
	   	   strncpy(dev, pps_argv[3],20);
	   	   
	   c_net_ni_set_ip_v6(dev, (char *)pps_argv[1], prefix);

       return CLIR_OK;
   }
   
   static INT32 cli_tracepath6(INT32 i4_argc, const CHAR**  pps_argv)
   {
	   char ipCmd[256] = {0};
	   
       if (i4_argc < 2)
       {
           printf("Usage: tracepath6 ip \n");
           return CLIR_OK;
       }
	   snprintf(ipCmd, 256, "tracepath6  %s", pps_argv[1]);
	   
       system(ipCmd);

       return CLIR_OK;
   }
   
  static INT32 cli_route6_set(INT32 i4_argc, const CHAR**  pps_argv)
   {
       if (i4_argc < 3)
       {
           printf("Usage: route6_set gw dev\n");
           return CLIR_OK;
       }
	   c_net_ni_set_gw_v6((char *)pps_argv[2], (char *)pps_argv[1]);

       return CLIR_OK;
   }

   static INT32 cli_route6_del(INT32 i4_argc, const CHAR**  pps_argv)
   {
     if (i4_argc < 2)
     {
           printf("Usage: route6_del dev\n");
           return CLIR_OK;
     }
	 c_net_ni_rmv6_gw((char *)pps_argv[1]);
     return CLIR_OK;
   }


   static INT32 cli_dhcp6_start(INT32 i4_argc, const CHAR**  pps_argv)
   {
		 int ip_auto = 0;
		 int dns_auto = 0;
		 if (i4_argc < 4)
		 {
			   printf("Usage: dhcp6_start b_ipauto b_dnsauto dev  (0=manual, 2=auto)\n");
			   return CLIR_OK;
		 }
		 ip_auto = atoi(pps_argv[1]);
		 dns_auto = atoi(pps_argv[2]);
		 c_net_ni_start_dhcpv6(ip_auto, dns_auto, (char *)pps_argv[3]);
		 return CLIR_OK;
   }
   
   static INT32 cli_dhcp6_stop(INT32 i4_argc, const CHAR**	pps_argv)
   {
		 c_net_ni_stop_dhcpv6("eth0");
		 return CLIR_OK;
   }
   
   static INT32 cli_start_config_ipv6(INT32 i4_argc, const CHAR**	pps_argv)
   {
		 IPV6_CONFIG_INFO_T tempInfo = {0};	
		 
		 if (i4_argc < 5)
		 {
			   printf("Usage: start_config6 dev RA-mode ip_mode dns_mode  (0=manual, 1=auto)\n");
			   return CLIR_OK;
		 }		
		 tempInfo.RA_mode = atoi(pps_argv[2]);
		 tempInfo.ip_mode = atoi(pps_argv[3]);
		 tempInfo.dns_mode = atoi(pps_argv[4]);
		 tempInfo.ip_prefix = 96;
		 strncpy(tempInfo.ip_addr, "2001::8888", 46);
		 strncpy(tempInfo.gw_addr, "2001::100", 46);
		 strncpy(tempInfo.dns1, "2001::1:1", 46);
		 strncpy(tempInfo.dns2, "2001::2:2", 46);
		 tempInfo.cb = net_dhcpok_cb;
		 
		 c_net_ni_start_config_ipv6((char *)pps_argv[1], &tempInfo);
		 return CLIR_OK;
   }
	
    static INT32 cli_disable_ipv6(INT32 i4_argc, const CHAR**	pps_argv)
   {
		if (i4_argc < 3)
		{
			  printf("Usage: disable6 dev flag (0 = enable, 1 = disable)\n");
			  return CLIR_OK;
		}
		if(1 == atoi(pps_argv[2]))
			c_net_ni_disable_ipv6((char *)pps_argv[1]);
		else
			c_net_ni_enable_ipv6((char *)pps_argv[1]);
		return CLIR_OK;
   }  

	
   static INT32 cli_rm_config_ipv6(INT32 i4_argc, const CHAR**	pps_argv)
   {
	   if (i4_argc < 3)
	   {
			 printf("Usage: rmcfg6 dev flag (0 = all, 1 = manual, 2 = slaac)\n");
			 return CLIR_OK;
	   }
	   if(0 == atoi(pps_argv[2]))
   		  c_net_ni_rmv6config_all((char *)pps_argv[1]);
	   if(1 == atoi(pps_argv[2]))
   		  c_net_ni_rmv6config_manual((char *)pps_argv[1]);
	   if(2 == atoi(pps_argv[2]))
   		  c_net_ni_rmv6config_SLAAC((char *)pps_argv[1]);	   
		 return CLIR_OK;
   }  
   static INT32 cli_set_prio_ipv6(INT32 i4_argc, const CHAR**	pps_argv)
   {
   		 int flag = 0;
   		 if (i4_argc < 2)
		 {
			   printf("Usage: prefer_ipv6 b_flag (1 = ipv6, 0 = ipv4)\n");
			   return CLIR_OK;
		 }
		 flag = atoi(pps_argv[1]);
		 c_net_IsPreferIpv6(flag);
		 return CLIR_OK;
   }   

   static INT32 cli_c_net_is_RA_rcvd(INT32 i4_argc, const CHAR**	pps_argv)
   {
        if (1);
        {
            printf("c_net_ni_accept_RA invalid parameter(%d)\n", (int)i4_argc);			
        }
        if (c_net_is_RA_rcvd((char *)pps_argv[1]))
        {
            printf("RA rcvd\n");
        }
        else{
            printf("RA not revd\n");
        }

        return 0;
   }
   
   static INT32 cli_test_connection_v6(INT32 i4_argc, const CHAR**	pps_argv)
   {
   		 int port = 0;
		 int timeout = 0;
   		 if (i4_argc < 4)
		 {
			   printf("Usage: test_conv6 url port timeout\n");
			   return CLIR_OK;
		 }
		 port = atoi(pps_argv[2]);
		 timeout = atoi(pps_argv[3]);		 
		 c_net_test_connection_v6((char *)pps_argv[1],port,timeout);
		 return CLIR_OK;
   } 	

   static INT32 cli_ping6(INT32 i4_argc, const CHAR**  pps_argv)
   {
       int len = 58, waittimeout = 10;
       if (i4_argc < 2)
       {
           printf("Usage: ping6 ip len timeout\n");
           return CLIR_OK;
       }
       if (i4_argc >= 4)
       {
           sscanf(pps_argv[3],"%d",&len);
       }
       if (i4_argc == 5)
       {
           sscanf(pps_argv[4],"%d",&waittimeout);
       }
       printf("ping len: %d bytes, waittm: %d sec\n", len, waittimeout);
	   if(len < 0 || len > 2000 || waittimeout < 0)
	   		return CLIR_OK;
       c_net_ping_v6((CHAR *)pps_argv[1], (CHAR *)pps_argv[2], len, waittimeout, ping_notify);

       return CLIR_OK;
   }

   static INT32 cli_get_ipv6(INT32 i4_argc, const CHAR**  pps_argv)
   {	  
   	   int type =0;
	   char ipstr[46] = {0};
	   int prefix = 0;
	   
       if (i4_argc < 3)
       {
           printf("Usage: getipv6 dev type!\n");
           return CLIR_OK;
       }

	   type = atoi(pps_argv[2]);

       c_net_ni_get_ip_v6((char *)pps_argv[1], type, ipstr, &prefix);
	   printf("CLI get ipv6 address : %s - %d\n", ipstr, prefix);
       return CLIR_OK;
   }

   static INT32 cli_get_gwv6(INT32 i4_argc, const CHAR**  pps_argv)
   {	  
	   char ipstr[46] = {0};
       if (i4_argc < 2)
       {
           printf("Usage: getgwv6 dev!\n");
           return CLIR_OK;
       }

       c_net_ni_get_gw_v6((char *)pps_argv[1], ipstr);
	   printf("CLI get gateway address : %s \n", ipstr);
	  
       return CLIR_OK;
   }

  
   /*-----------------------------------------------------------------------------
                       data declarations
    ----------------------------------------------------------------------------*/
   /* Download module command table */

   static CLI_EXEC_T sub_cmd_tbl[] =
   {
       /*     full, brief,         command func,  next command tbl, help description, access level */
	   { "test_up",  "tup", 	cli_test_up,	 NULL, "test if cli is up", CLI_GUEST},	   
       { "hostname",       "hn",     cli_c_net_set_hostname,       NULL, "set/get hostname", CLI_GUEST},
       { "ip_config",      "ip",     cli_c_net_ip_config,          NULL, "ip_config ip_address netmask gateway", CLI_GUEST },
       { "dns_config",     "dns",    cli_c_net_dns_config,         NULL, "dns_config dns1 dns2", CLI_GUEST },
       { "dns_lookup",     "dns_lk", cli_c_net_dns_lookup,         NULL, "dns_lookup server_url", CLI_GUEST },
       { "proxy_connection", "proxy",cli_c_net_proxy_connection,   NULL, "proxy_connection server_url port", CLI_GUEST },
       { "dhcpc_start",    "dn",     cli_c_dhcpc_start,            NULL, "dhcp_on", CLI_GUEST },
       { "dhcpc_stop",     "df",     cli_c_dhcpc_stop,             NULL, "dhcp_off", CLI_GUEST },
       { "dhcpc_restart",  "dr",     cli_c_dhcpc_restart,          NULL, "dhcp_restart", CLI_GUEST },
       { "dhcpc_get_info", "ip_info",cli_c_dhcpc_get_info,         NULL, "show dhcp ip address", CLI_GUEST },
       { "ping",           "p",      cli_ping,                     NULL, "ping dst_ip size interval(second)\n", CLI_GUEST },
       { "nimon",          "nim",    ui_ni_monitor_init,           NULL, "reg ni monitor event", CLI_GUEST },
       { "clean_dns",      "clean_dns", cli_clean_dns_cache_debug, NULL, "clean dns cache", CLI_GUEST },
       { "setMac2drv",        "smac",   cli_c_net_ni_set_mac,         NULL, "set_mac nif 6byte-MacAddress", CLI_GUEST },
       { "getMacfromdrv",     "gmac",   cli_c_net_ni_get_mac,         NULL, "get_mac nif", CLI_GUEST },
       { "setMac2sys",        "syssmac",   cli_c_net_ni_set_mac_sys,         NULL, "set_mac 6byte-MacAddress", CLI_GUEST },
       { "getMacfromsys",     "sysgmac",   cli_c_net_ni_get_mac_sys,         NULL, "get_mac", CLI_GUEST },
       { "invoke_telnetd", "td",     cli_c_net_ni_invoke_telnetd,  NULL, "td", CLI_GUEST },
       { "local_ip",     "lip",     cli_get_local_host_ip,     NULL, "lip: to get real host ip", CLI_GUEST},
       { "get_host_by_name",  "gh",     cli_get_host_by_name,     NULL, "gh urlname(please input valid format)", CLI_GUEST},
       { "autoip",  "auto",     cli_c_net_autoip_config,     NULL, "auto if macaddr", CLI_GUEST},
       { "ip conflict",  "conflict",     cli_c_net_ip_conflict,     NULL, "conflict ip", CLI_GUEST},
       { "network_dbg_msg", "ndm",   cli_c_net_dbg_msg_toggle, NULL, "Taggle network debugging information",  CLI_GUEST},
       { "get_ni_name",  "getwlni",     cli_c_net_get_wifi_ni_name,     NULL, "get wireless ni name", CLI_GUEST},
       { "MPTool Server",  "mpserver",     cli_c_net_mptool_server_startup,     NULL, "daemon for MPTool", CLI_GUEST},
       { "pinghostbyname",  "p_host",     cli_c_net_pinghostbyname,     NULL, "ping host by domain name", CLI_GUEST},
       { "test connection",  "test",     cli_c_net_test_connection,     NULL, "test <host domain name> <port>", CLI_GUEST},
       { "get_mac_by_ip",  "ip_mac",     cli_c_net_get_mac_by_ip,     NULL, "ip_mac ip_addr", CLI_GUEST},
       { "get_ni_speed",  "ni_sp",     cli_c_net_get_ni_speed,     NULL, "ni_sp ni_name", CLI_GUEST},
       { "Phy ctrl",  "pc",	 cli_c_net_phy_ctrl, 	NULL, "pc <eth0> <get/set> <10Amp> <value>", CLI_GUEST},
       { "start_wol_monitor",  "start_wol",   cli_c_net_start_wol_thread,     NULL, "Start WOL monitor thread", CLI_GUEST},
       { "stop_wol_monitor",  "stop_wol",     cli_c_net_stop_wol_thread,     NULL, "Stop WOL monitor thread", CLI_GUEST},
       { "pppoe_start",  "pppoe_start",     cli_c_pppoe_start,     NULL, "pppoe_start <if name> <username> <password>", CLI_GUEST},
       { "pppoe_stop",  "pppoe_stop",     cli_c_pppoe_stop,     NULL, "pppoe_stop", CLI_GUEST},
       { "debugPrintShift",  "dbg",     cli_c_debug_print_switch,     NULL, "mw debug print on-off", CLI_GUEST},
       { "ntp_sync_time",  "tsync",     cli_c_ntp_sync_time,     NULL, "tsync <ntp_time_server>", CLI_GUEST},

       { "set_ipcd_env",  "senv",     cli_c_set_ipcd,     NULL, "set_ipcd_env", CLI_GUEST},
		{ "get_ipcd_env",  "genv", 	cli_c_get_ipcd,	 NULL, "get_ipcd_env", CLI_GUEST},
		{ "get_valid_mac",  "gvm",  cli_c_get_validMac,	NULL, "get_valid_mac", CLI_GUEST},


	   { "net_read_snr",  "nrs", 	cli_c_net_read_snr,	 NULL, "read internet Phy gain value; read the SRN(Signial to Noise Ratio)", CLI_GUEST},
	          { "get speed dup", "gsd", cli_c_net_get_speed_duplex, NULL, "get speed and duplex", CLI_GUEST  },
	          
       { "ping6",          "p6",      cli_ping6,     NULL, "ping dst_ip size interval(second)\n", CLI_GUEST },
	   { "ip6_add",		   "ip6a", 	 cli_ip6_set,					   NULL, "add ipv6 address to if\n", CLI_GUEST },
	   { "ip6_show",	   "ip6s", 	 cli_ip6_show,					   NULL, "show all ipv6 address\n", CLI_GUEST },
	   { "tracepath6",	   "trace6", 	 cli_tracepath6,					   NULL, "trace an ip address\n", CLI_GUEST },
	   { "route6_set",	   "rt6s", 	 cli_route6_set,		NULL, "set gateway entry\n", CLI_GUEST },
	   { "route6_del",	   "rt6d", 	 cli_route6_del,		NULL, "del gateway entry\n", CLI_GUEST },
	   { "dhcp6_start",	   "d6s", 	 cli_dhcp6_start,		NULL, "start dhcpv6\n", CLI_GUEST },
	   { "dhcp6_stop",	   "d6f", 	 cli_dhcp6_stop,		NULL, "stop dhcpv6\n", CLI_GUEST },
	   { "start_config6",	   "autov6",	 cli_start_config_ipv6,		NULL, "start ipv6 ip-address config\n", CLI_GUEST },			   
	   { "disablev6",	   "dis6",	 cli_disable_ipv6,	NULL, "enable/disable ipv6\n", CLI_GUEST }, 		   

	   { "rm_config_v6",   "rmcfg6", cli_rm_config_ipv6,	NULL, "remove current ipv6 config\n", CLI_GUEST },			   

	   { "priority_v6",	   "sp6",	 cli_set_prio_ipv6, 	NULL, "set ipv6/ipv4 priority\n", CLI_GUEST },			   
	   { "test_con6",	   "tc6",	 cli_test_connection_v6, 	NULL, "test connection of ipv6\n", CLI_GUEST },		
       { "test_accept_RA", "accept_RA",	 cli_c_net_is_RA_rcvd, 	NULL, "test interface accept RA\n", CLI_GUEST },      
	   { "getipv6",	   	   "getipv6",	 cli_get_ipv6,	NULL, "get ipv6 address \n", CLI_GUEST }, 		   
	   { "getgwv6",	       "getgwv6",	 cli_get_gwv6,	NULL, "get gateway address\n", CLI_GUEST },
       { "sendrs",	       "sendrs",	 cli_icmp_RS,	NULL, "send rs message\n", CLI_GUEST },
       { "getRAbit",	   "getra",	    cli_net_get_RA_bit,	NULL, "get ra bit\n", CLI_GUEST },
       END_OF_CLI_CMD_TBL
   };

   /* Download module root command table */
   static CLI_EXEC_T root_cmd_tbl[] =
   {
       {
           "ave_tcp",
           NULL,
           NULL,
           sub_cmd_tbl,
           "ave_tcp ut",
           CLI_GUEST
       },
       END_OF_CLI_CMD_TBL
   };

   /*-----------------------------------------------------------------------------
                       Internal functions implementations
    ----------------------------------------------------------------------------*/

   static VOID ip_ni_notify(NI_DRV_EV_T e_ev)
   {
       printf("Received ni notify -> %d\n", e_ev);
   }

   extern VOID ni_monitor_init(VOID);
   static INT32 ui_ni_monitor_init(INT32 i4_argc, const CHAR**  pps_argv)
   {
       if (c_net_ni_reg_ev_notify(NI_ETHERNET_0, NI_DRV_EV_ETHERNET_PLUGIN, ip_ni_notify) != NET_NI_OK)
       {
           printf("register ni ev notify failed\n");
           return CLIR_OK;
       }

       if (c_net_ni_reg_ev_notify(NI_ETHERNET_0, NI_DRV_EV_ETHERNET_UNPLUG, ip_ni_notify) != NET_NI_OK)
       {
           printf("register ni ev notify failed\n");
           return CLIR_OK;
       }

       if (c_net_ni_reg_ev_notify(NI_ETHERNET_0, NI_DRV_EV_IP_ADDRESS_CHANGED, ip_ni_notify) != NET_NI_OK)
       {
           printf("register ni ev notify failed\n");
           return CLIR_OK;
       }

       return CLIR_OK;
   }

   static INT32 cli_c_net_ip_config(INT32 i4_argc, const CHAR**  pps_argv)
   {
       INT32 i4_rt;
       in_addr_t t_ip, t_net_mask, t_gateway;

       if (i4_argc < 3)
       {
           printf("Usage: ip_config ip netmask gateway\n");
           return CLIR_OK;
       }

       /* ip address */
       t_ip       = inet_addr(pps_argv[1]); /* The return of inet_addr is network endian */
       t_net_mask = inet_addr(pps_argv[2]);
       t_gateway  = inet_addr(pps_argv[3]);

       i4_rt = c_net_ip_config(NI_ETHERNET_0, (UINT32) t_ip, (UINT32) t_net_mask, (UINT32) t_gateway);

       if (i4_rt != NET_OK)
       {
           printf("ip config failed! error code=%d \n", (int)i4_rt);
#if CONFIG_SUPPORT_SS
         putchar(0xfd);
       }
       else
       {
	       putchar(0xfc);
#endif
       }

       return(CLIR_OK);
   }

   static INT32 cli_c_net_set_hostname(INT32 i4_argc, const CHAR**  pps_argv)
   {
       if (i4_argc == 1)
       {
           c_net_set_hostname((CHAR*) NULL);
       }
       else
       {
           c_net_set_hostname((CHAR*) pps_argv[1]);
       }

       return CLIR_OK;
   }

   static INT32 cli_c_net_dns_config(INT32 i4_argc, const CHAR**  pps_argv)
   {
       INT32 i4_rt;
       in_addr_t t_dns1, t_dns2;

       if (i4_argc < 2)
       {
           printf("Usage: dns_config DNS_1 DNS_2\n");
           return CLIR_OK;
       }

       /* ip address */
       t_dns1 = inet_addr(pps_argv[1]); /* The return of inet_addr is network endian */
       t_dns2 = inet_addr(pps_argv[2]);

       i4_rt = c_net_dns_config((UINT32) t_dns1, (UINT32) t_dns2);

       if (i4_rt != NET_OK)
       {
           printf("DNS config failed! error code=%d \n", (int)i4_rt);
       }

       return CLIR_OK;
   }

   static INT32 cli_c_net_dns_lookup(INT32 i4_argc, const CHAR**  pps_argv)
   {
       INT32 i4_rt;       
       NET_ADDR_INFO_LOOK_UP tAddrInfoLookup = {0};
       if (i4_argc < 1)
       {
           printf("Usage: dns_lookup server_url\n");
           return CLIR_OK;
       }
    
       tAddrInfoLookup.eAddrFamily = (NET_ADDR_FAMILY_TYPE)atoi(pps_argv[1]);
       tAddrInfoLookup.ps_server_url = "www.google.com";

       i4_rt = c_net_dns_lookup(&tAddrInfoLookup);

       if (i4_rt != NET_OK)
       {
           printf("DNS lookup failed! error code=%d \n", (int)i4_rt);
       }

       return CLIR_OK;
   }

   static INT32 cli_c_net_proxy_connection(INT32 i4_argc, const CHAR**  pps_argv)
   {
       INT32 i4_rt;
       CHAR *c;
       UINT16 tmp = 0;

       if (i4_argc < 2)
       {
           printf("Usage: proxy_connection server_url port\n");
           return CLIR_OK;
       }

       for (c = (CHAR *)pps_argv[2]; *c; c++)
       {
         tmp *= 10;
         tmp += (UINT16)(*c - '0');
       }

       i4_rt = c_net_proxy_connection(pps_argv[1], (UINT16)(tmp));

       if (i4_rt != NET_OK)
       {
           printf("Proxy connection failed! error code=%d \n", (int)i4_rt);
       }

       return CLIR_OK;
   }

   static VOID net_dhcpok_cb(DLNA_DHCPC_EV_T e_event)
   {
     switch (e_event)
     {
       case DHCPC_EVENT_SUCCESS_DHCPv4:
       case DHCPC_EVENT_NEW_ADDRESS_DHCPv4:
       {
         printf("DHCP start OK.\n");
         break;
       }
       case DHCPC_EVENT_FAILURE_DHCPv4:
       {
         printf("DHCP start FAIL.\n");
         break;
       }
       default:
         printf("Rcv notify : %d.\n", e_event);
         break;
     }
  }


   static INT32 cli_c_dhcpc_start(INT32 i4_argc, const CHAR**  pps_argv)
   {
       if (c_dhcpc_init() != DHCPC_R_OK)
       {
           printf("Init autoip failed\n");
           return CLIR_OK;
       }

       if (c_dhcpc_start(NI_ETHERNET_0, net_dhcpok_cb) != DHCPC_R_OK)
       {
           printf("request ip failed\n");
           return CLIR_OK;
       }
       return CLIR_OK;
   }

static INT32 cli_icmp_RS(INT32 i4_argc, const CHAR**  pps_argv)
{
    int i4_ret = 0;
    if (i4_argc < 2)
    {
        printf("icmp_RS: pls enter interface\n");
        return CLIR_OK;
    }

    i4_ret = icmp_RS((CHAR *)pps_argv[1]);
    if (i4_ret != 0)
    {
        printf("icmp_RS: error return[%d]\n", i4_ret);
    }

    return i4_ret;
}

  static INT32 cli_net_get_RA_bit(INT32 i4_argc, const CHAR**  pps_argv)
   {
       int i4_ret = 0;
       int ip_state = 1; 
       int dns_state = 1;
       if (i4_argc < 2)
       {
           printf("_net_get_RA_bit: pls enter interface\n");
           return CLIR_OK;
       }

       if (i4_argc >= 4)
       {
            ip_state = atoi(pps_argv[2]);
            dns_state = atoi(pps_argv[3]);
       }
   
       i4_ret = _net_get_RA_bit((CHAR *)pps_argv[1], &ip_state, &dns_state);
       if (i4_ret != 0)
       {
           printf("_net_get_RA_bit: error return[%d]\n", i4_ret);
       }

       return i4_ret;
   }



   static INT32 cli_c_dhcpc_stop(INT32 i4_argc, const CHAR**  pps_argv)
   {
       if (c_dhcpc_stop(NI_ETHERNET_0) != DHCPC_R_OK)
       {
           printf("stop autoip failed\n");
           return CLIR_OK;
       }

       if (c_dhcpc_deinit() != DHCPC_R_OK)
       {
           printf("de-init autoip failed\n");
           return CLIR_OK;
       }

       return CLIR_OK;
   }

   static INT32 cli_c_dhcpc_restart(INT32 i4_argc, const CHAR**  pps_argv)
   {
       if (c_dhcpc_restart(NI_ETHERNET_0) != DHCPC_R_OK)
       {
           printf("request ip failed\n");
           return CLIR_OK;
       }
       return CLIR_OK;
   }

   static INT32 cli_c_dhcpc_get_info(INT32 i4_argc, const CHAR**  pps_argv)
   {
       MT_DHCP4_INFO_T t_info;
       struct in_addr t_addr;

       if (c_dhcpc_get_info(NI_ETHERNET_0, &t_info) != DHCPC_R_OK)
       {
           printf("c_dhcpc_get_info failed\n");
           return CLIR_OK;
       }

       t_addr.s_addr = t_info.ui4_ipaddr;
       printf("ipaddr            : %s\n", (char *)inet_ntoa(t_addr));

       t_addr.s_addr = t_info.ui4_subnet;
       printf("subnet            : %s\n", (char *)inet_ntoa(t_addr));

       t_addr.s_addr = t_info.ui4_router;
       printf("router            : %s\n", (char *)inet_ntoa(t_addr));

       t_addr.s_addr = t_info.ui4_dns1;
       printf("dns1              : %s\n", (char *)inet_ntoa(t_addr));

       t_addr.s_addr = t_info.ui4_dns2;
       printf("dns2              : %s\n", (char *)inet_ntoa(t_addr));

       return CLIR_OK;
   }

   static INT32 cli_ping(INT32 i4_argc, const CHAR**  pps_argv)
   {
       int len = 58, waittimeout = 10;
       if (i4_argc < 2)
       {
           printf("Usage: ping ip len timeout\n");
           return CLIR_OK;
       }
       if (i4_argc >= 3)
       {
           sscanf(pps_argv[2],"%d",&len);
       }
       if (i4_argc == 4)
       {
           sscanf(pps_argv[3],"%d",&waittimeout);
       }
       printf("ping len: %d bytes, waittm: %d sec\n", len, waittimeout);

       c_net_ping_v4((CHAR *)pps_argv[1], len, waittimeout, ping_notify);

       return CLIR_OK;
   }

   static INT32 cli_clean_dns_cache_debug(INT32 i4_argc, const CHAR**  pps_argv)
   {
       c_net_clean_dns_cache();

       return CLIR_OK;
   }


   static INT32 cli_c_net_ni_set_mac(INT32 i4_argc, const CHAR**  pps_argv)
   {
      MAC_ADDRESS_T           t_MacAddress = {0};

      if (i4_argc != 3)
      {
          printf("Usage: smac ifName xx:xx:xx:xx:xx:xx\n");
          return CLIR_OK;
      }

      sscanf(pps_argv[2], "%hhx:%hhx:%hhx:%hhx:%hhx:%hhx",
             &(t_MacAddress[0]),
             &(t_MacAddress[1]),
             &(t_MacAddress[2]),
             &(t_MacAddress[3]),
             &(t_MacAddress[4]),
             &(t_MacAddress[5]));
      c_net_ni_set_mac((CHAR*)pps_argv[1], (VOID *)(&t_MacAddress));
      c_net_ni_get_mac((CHAR*)pps_argv[1], (CHAR*)(&t_MacAddress));
      printf("Ethernet MAC = %02x:%02x:%02x:%02x:%02x:%02x\n", t_MacAddress[0],t_MacAddress[1],t_MacAddress[2],t_MacAddress[3],t_MacAddress[4],t_MacAddress[5]);
      return CLIR_OK;
   }

   static INT32 cli_c_net_ni_get_mac(INT32 i4_argc, const CHAR**  pps_argv)
   {
      MAC_ADDRESS_T           t_MacAddress = {0};

      if (i4_argc != 2)
      {
          printf("Usage: gmac ifName\n");
          return CLIR_OK;
      }

      c_net_ni_get_mac((CHAR*)pps_argv[1], (CHAR*)(&t_MacAddress));
      printf("Ethernet MAC = %02x:%02x:%02x:%02x:%02x:%02x\n", t_MacAddress[0],t_MacAddress[1],t_MacAddress[2],t_MacAddress[3],t_MacAddress[4],t_MacAddress[5]);

      return CLIR_OK;
   }

   static INT32 cli_c_net_ni_set_mac_sys(INT32 i4_argc, const CHAR**  pps_argv)
   {
      MAC_ADDRESS_T           t_MacAddress = {0};

      if (i4_argc != 2)
      {
          printf("Usage: syssmac xx:xx:xx:xx:xx:xx\n");
          return CLIR_OK;
      }

      sscanf(pps_argv[1], "%hhx:%hhx:%hhx:%hhx:%hhx:%hhx",
             &(t_MacAddress[0]),
             &(t_MacAddress[1]),
             &(t_MacAddress[2]),
             &(t_MacAddress[3]),
             &(t_MacAddress[4]),
             &(t_MacAddress[5]));
      c_net_set_mac_to_sys((CHAR *)(&t_MacAddress));

      c_net_get_mac_from_sys((CHAR*)(&t_MacAddress));
      printf("Ethernet MAC = %02x:%02x:%02x:%02x:%02x:%02x\n", t_MacAddress[0],t_MacAddress[1],t_MacAddress[2],t_MacAddress[3],t_MacAddress[4],t_MacAddress[5]);
      return CLIR_OK;
   }

   static INT32 cli_c_net_ni_get_mac_sys(INT32 i4_argc, const CHAR**  pps_argv)
   {
      MAC_ADDRESS_T           t_MacAddress = {0};

      if (i4_argc != 1)
      {
          printf("Usage: sysgmac\n");
          return CLIR_OK;
      }

      c_net_get_mac_from_sys((CHAR*)(&t_MacAddress));
      printf("Ethernet MAC = %02x:%02x:%02x:%02x:%02x:%02x\n", t_MacAddress[0],t_MacAddress[1],t_MacAddress[2],t_MacAddress[3],t_MacAddress[4],t_MacAddress[5]);

      return CLIR_OK;
   }

   static INT32 cli_c_net_ni_invoke_telnetd(INT32 i4_argc, const CHAR**  pps_argv)
   {
      CHAR ps_cmd[64];
      INT32 i4_sys_ret;
      strncpy(ps_cmd, "/usr/sbin/inetd &", 64);

      i4_sys_ret = system(ps_cmd);

      if (i4_sys_ret == 0)
      {
          printf("telnetd invoked ok\n");
      }
      else
      {
          printf("telnetd invoked failed\n");
      }

      return CLIR_OK;
   }

   static INT32 cli_get_local_host_ip(INT32 i4_argc, const CHAR**  pps_argv)
  {
    char localname[1024];
    struct hostent *pt_hostnet;

    if (i4_argc != 1)
    {
      printf("Usage: lip\n");
      return CLIR_OK;
    }

    if(gethostname(localname, sizeof(localname)) < 0)
    {
      printf("gethostname failed\n");
      return CLIR_OK;

    }
    printf("hostname: %s\n", localname);

    pt_hostnet = c_net_gethostbyname(localname);

    if (pt_hostnet && (pt_hostnet->h_addr_list[0] != NULL))
    {
        printf ("Host Name= %s, IP = %s\n", localname, (char *)inet_ntoa(*( struct in_addr*)( pt_hostnet->h_addr_list[0])));
    }
    else
    {
        printf ("Host lookup Name unknown!\n");
        return CLIR_OK;
    }

    return CLIR_OK;
  }

  static INT32 cli_get_host_by_name(INT32 i4_argc, const CHAR**  pps_argv)
  {
    struct hostent *pt_hostnet;

    if (i4_argc != 2)
    {
      printf("Usage: gh urlname\n");
      return CLIR_OK;
    }

    printf("urlname: %s\n", (CHAR*)pps_argv[1]);

    pt_hostnet = c_net_gethostbyname((CHAR*)pps_argv[1]);

    if (pt_hostnet && (pt_hostnet->h_addr_list[0] != NULL))
    {
        printf ("Host IP = %s\n", (char *)inet_ntoa(*( struct in_addr*)( pt_hostnet->h_addr_list[0])));
    }
    else
    {
        printf ("Host lookup Name unknown!\n");
        return CLIR_OK;
    }

    return CLIR_OK;
  }

  static INT32 cli_c_net_autoip_config(INT32 i4_argc, const CHAR**  pps_argv)
   {
       INT32 i4_rt;

       if (i4_argc < 2)
       {
           printf("Usage: if macaddr\n");
           return CLIR_OK;
       }

       i4_rt = c_net_autoip_config((CHAR*)pps_argv[1], (CHAR*)pps_argv[2]);

       if (i4_rt != NET_OK)
       {
           printf("autoip config failed! error code=%d \n", (int)i4_rt);
       }

       return(CLIR_OK);
   }

  static VOID conflict_notify(BOOL fgConflict)
   {
       if (fgConflict)
         printf("IP is conflicted\n");
       else
        printf("IP is not conflicted\n");
   }

  static INT32 cli_c_net_ip_conflict(INT32 i4_argc, const CHAR**  pps_argv)
   {
       INT32 i4_rt;
       in_addr_t t_ip;

       if (i4_argc < 2)
       {
           printf("Usage: conflict ip\n");
           return CLIR_OK;
       }
       t_ip       = inet_addr(pps_argv[1]);

       i4_rt = c_net_ip_conflict("eth0", (UINT32)t_ip, conflict_notify);

       if (i4_rt != NET_OK)
       {
           printf("ip conflict failed! error code=%d \n", (int)i4_rt);
       }

       return(CLIR_OK);
   }

   static INT32 cli_c_net_dbg_msg_toggle(INT32 i4_argc, const CHAR** pps_argv)
   {
        extern INT32 i4_NDM;

        if (i4_NDM)
        {
            i4_NDM = 0;
        }
        else
        {
            i4_NDM = 1;
        }

        return CLIR_OK;
   }

   static INT32 cli_c_net_get_wifi_ni_name(INT32 i4_argc, const CHAR**  pps_argv)
   {
        INT32 i4_ret;
        CHAR ni_name[10]={0};

        i4_ret = c_net_get_wifi_ni_name(ni_name);
        if (NET_OK != i4_ret)
        {
             printf("get wifi ni name failed! error code=%d \n", (int)i4_ret);
             return CLIR_CMD_EXEC_ERROR;
        }

        printf("Wireless network ni name: %s\n", ni_name);

        return CLIR_OK;
   }
  static VOID mpserver_notify(INT32 evt)
  {
        if (0 == evt)
        {
            printf("<CLI>MPServer Upgrade Start\n");
        }
        else
        {
            printf("<CLI>MPServer upgrade Finished\n");
        }
  }

  static INT32 cli_c_net_mptool_server_startup(INT32 i4_argc, const CHAR * * pps_argv)
  {
        INT32 i4_ret;

        i4_ret = c_net_mptool_server_startup(mpserver_notify);

        return CLIR_OK;
  }

  static INT32 cli_c_net_pinghostbyname(INT32 i4_argc, const CHAR**  pps_argv)
  {
        INT32 i4_ret;

        if (i4_argc < 2)
        {
            printf("Usage: p_host <host domain name>\n");
            return CLIR_CMD_EXEC_ERROR;
        }

        i4_ret = c_net_pinghostbyname(pps_argv[1]);

        printf("ping result: %d", (int)i4_ret);

        return CLIR_OK;
  }

  INT32 cli_c_net_test_connection(INT32 i4_argc, const CHAR**  pps_argv)
  {
        INT32 i4_ret;

        if (i4_argc < 4)
        {
            printf("Usage: test <host domain name> <port> <timeout>\n");
            return CLIR_CMD_EXEC_ERROR;
        }

        i4_ret = c_net_test_connection(pps_argv[1], atoi(pps_argv[2]), atoi(pps_argv[3]));

        if (NET_OK == i4_ret)
        {
            printf("Connection test to %s:%s success.\n", pps_argv[1], pps_argv[2]);
        }
        else
        {
            printf("Connection test to %s:%s fail.\n", pps_argv[1], pps_argv[2]);
        }

        return CLIR_OK;
  }

  static INT32 cli_c_net_get_mac_by_ip(INT32 i4_argc, const CHAR * * pps_argv)
  {
        INT32 i4_ret;
        UCHAR mac_addr[20];

        if (i4_argc < 2)
        {
           printf("Usage: ip_mac ip\n");
           return CLIR_INV_ARG;
        }

        i4_ret = c_net_get_mac_by_ip(pps_argv[1], mac_addr);

        if (NET_OK == i4_ret)
        {
            printf("MTKSTUB_OK=> %s's MAC address: %02x:%02x:%02x:%02x:%02x:%02x\n", pps_argv[1], mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);
        }
        else if (NET_FAILED == i4_ret)
        {
            printf("MTKSTUB_ARP_NO_ENTRY \n");
        }
        else
        {
            printf("MTKSTUB_ARP_NOT_SAMESEG \n");
        }

        return CLIR_OK;

  }


  static VOID get_ni_speed_cb(NET_SPEED_T * net_speed)
  {
    printf("RX: %d bytes, TX: %d bytes\n", (int)net_speed->rx_speed, (int)net_speed->tx_speed);
  }

  static INT32 cli_c_net_get_ni_speed(INT32 i4_argc, const CHAR * * pps_argv)
  {
          INT32 i4_ret;


          if (i4_argc < 2)
          {
             printf("Usage: ni_sp <network interface name>\n");
             return CLIR_INV_ARG;
          }
          i4_ret = c_net_get_ni_speed(pps_argv[1], (get_ni_speed_cb_fp)get_ni_speed_cb);
          if (NET_OK != i4_ret)
          {
              printf("Get network interface speed fail \n");
              return CLIR_CMD_EXEC_ERROR;
          }
          return CLIR_OK;

   }

  INT32 cli_c_net_phy_ctrl(INT32 i4_argc, const CHAR**  pps_argv)
  {
        INT32 i4_ret;
		INT32 fgSet=0;

        if (i4_argc < 4)
        {
            printf("Usage: pc <eth0> <set/get> <0:100Amp/1:10Amp/2:InBias/3:OutBias/4:FedbakCap/5:slew/6:eye/7:BW50> <value>\n");
            return CLIR_CMD_EXEC_ERROR;
        }
        else if((pps_argv[2][0]=='s'||pps_argv[2][0]=='S') &&  i4_argc<5)
        {
             printf("Usage: pc <eth0> <set/get> <0:100Amp/1:10Amp/2:InBias/3:OutBias/4:FedbakCap/5:slew/6:eye/7:BW50> <value>\n");
              return CLIR_CMD_EXEC_ERROR;
        }

		if(pps_argv[2][0]=='s' || pps_argv[2][0]=='S')
		{
            fgSet =1 ;
		}
		else if(pps_argv[2][0]=='g' || pps_argv[2][0]=='G')
		{
            fgSet =0 ;
		}
		else
		{
			printf("Usage: pc <eth0> <set/get> <0:100Amp/1:10Amp/2:InBias/3:OutBias/4:FedbakCap/5:slew/6:eye/7:BW50> <value>\n");
			 return CLIR_CMD_EXEC_ERROR;
		}

        i4_ret = c_net_phy_ctrl(pps_argv[1], fgSet, atoi(pps_argv[3]),(i4_argc<=4)?0: atoi(pps_argv[4]));

        return CLIR_OK;
  }

  static VOID _cli_c_net_start_wol_thread_cb(VOID)
  {
        printf("System Power On from WOL\n");
  }

  static INT32 cli_c_net_start_wol_thread(INT32 i4_argc, const CHAR**  pps_argv)
  {
        INT32 i4_ret;

        if (i4_argc < 2)
        {
           printf("Usage: start_wol <ifname>\n");
           return CLIR_INV_ARG;
        }

        i4_ret = c_net_start_wol_thread((CHAR *)pps_argv[1], _cli_c_net_start_wol_thread_cb);
        if (NET_OK != i4_ret)
        {
            printf("WOL Monitor start fail \n");
            return CLIR_CMD_EXEC_ERROR;
        }
        return CLIR_OK;

  }

  static INT32 cli_c_net_stop_wol_thread(INT32 i4_argc, const CHAR**  pps_argv)
  {
        INT32 i4_ret;
        i4_ret = c_net_stop_wol_thread();

        return CLIR_OK;
  }


  static VOID pppoe_start_cb(PPPOE_EV_T event)
  {
        if ( PPPOE_EVENT_SUCCESS == event )
        {
            printf("<CLI> pppoe_start sucessfully\n");
        } else {
            printf("<CLI> pppoe_start failed\n");
        }

  }

  static INT32 cli_c_pppoe_start(INT32 i4_argc, const CHAR**  pps_argv)
  {
        INT32 i4_ret;

        if (i4_argc < 4)
        {
            printf("Usage: pppoe_start <interface> <username> <password>\n");
            return CLIR_INV_ARG;
        }

        i4_ret = c_pppoe_start((CHAR *)pps_argv[1], (CHAR *)pps_argv[2], (CHAR *)pps_argv[3], pppoe_start_cb);

        if (NET_OK != i4_ret)
            return CLIR_CMD_EXEC_ERROR;

        return CLIR_OK;

  }

  static INT32 cli_c_pppoe_stop(INT32 i4_argc, const CHAR**  pps_argv)
  {
        INT32 i4_ret;

        i4_ret = c_pppoe_stop();

        return CLIR_OK;
  }


   static INT32 cli_c_debug_print_switch(INT32 i4_argc, const CHAR**  pps_argv)
   {
		 return CLIR_OK;
   }

 static void _c_ntp_sync_time_notify_callback (NTP_EV_T event)
 {
    switch (event)
    {
        case NTP_SYNC_TIME_SUCCESS:

            printf("NTP time sync sucessed\n");			
			system("date");
            break;

        case NTP_SYNC_TIME_FAILURE:
        default:

            printf("NTP time sync failed\n");
			system("date");
            break;
    }
 }

  static INT32 cli_c_ntp_sync_time(INT32 i4_argc, const CHAR**  pps_argv)
  {
        INT32 i4_ret;

        if (i4_argc >1)
        {
            i4_ret = c_ntp_sync_time( (CHAR *)pps_argv[1], _c_ntp_sync_time_notify_callback);
            
        }else{
            i4_ret = c_ntp_sync_time(NULL, _c_ntp_sync_time_notify_callback);         
        }

        return CLIR_OK;


  }

static INT32 cli_c_set_ipcd(INT32 i4_argc,  const CHAR**  pps_argv)
{
	  INT32 i4_ret;
	  char name[24];
	  char value[24];

	  if (i4_argc != 3)
	  	  printf("Usage: senv name value\n");
	  else
	  {
	  	  strncpy(name, pps_argv[1],20);
	  	  strncpy(value, pps_argv[2],20);
		  i4_ret = ipcd_set_env(name, value); 		
	  }

	  return CLIR_OK;


}

static INT32 cli_c_get_ipcd(INT32 i4_argc, const CHAR**  pps_argv)
{
	  INT32 i4_ret;
	  char name[24]={0};
	  char value[24]={0};
	  
	  if (i4_argc != 2)
	  	  printf("Usage: genv name\n");
	  else
	  {
	  	  strncpy(name, pps_argv[1],20);
		  i4_ret = ipcd_get_env(name, value); 		
		  if(!i4_ret)
		  	printf("%s value is %s \n", name, value);
	  }
	  return CLIR_OK;


}


static INT32 cli_c_get_validMac(INT32 i4_argc, const CHAR**  pps_argv)
{
	//char mac[6]={0};
	//net_get_validMac(mac);
	return CLIR_OK;
}



static INT32 cli_c_net_read_snr(INT32 i4_argc, const CHAR**  pps_argv)
   {
		INT32 i4Ret=0,i4Idx;
		UINT32 u4ReadCount,slc_mse_exta,gain;
		double div;
		double mse=0,snr,maxsnr=0,minsnr=100,meansnr=0;
		
		UINT16 data=0;
		UINT16 tokenDatalow =0,tokenDataHi=0;

#define REG_WRITE 1
#define REG_READ 0
		
		if ((i4_argc < 2) || (pps_argv == NULL) || (pps_argv[1] == NULL))
		{
			printf("Arg: read iteration\n");
			return 0;
		}
	
		u4ReadCount = (UINT32) atoi(pps_argv[1]);
		if(u4ReadCount<1)
			{
				printf("invaild input parmater!need input>0\n");
				return 0;
			}
		//get the gain value
		data=0x52b5;
		c_net_mdio_ctrl(REG_WRITE,0x1f,&data);   //change Token ring
		data=0xafa4;
		c_net_mdio_ctrl(REG_WRITE,0x10,&data);   // read 
		
		c_net_mdio_ctrl(REG_READ,0x11,&tokenDatalow);   //read low data
		//printf("(%d)tokenDatalow=0x%x\n",__LINE__,(int)tokenDatalow);
		c_net_mdio_ctrl(REG_READ,0x12,&tokenDataHi);
		//printf("(%d)tokenDataHi=0x%x\n",__LINE__,(int)tokenDataHi);
		gain=(tokenDataHi&0xfe)>>1;
		data=0;
		c_net_mdio_ctrl(REG_WRITE,0x1f,&data);   //change main mode
	   // printf("gain=%d,tokenDataHi=0x%x,tokenDatalow=0x%x\n",(int)gain,(int)tokenDataHi,(int)tokenDatalow);
		
		for (i4Idx=0;i4Idx<u4ReadCount;i4Idx++)
			{
				data=0x52b5;
				c_net_mdio_ctrl(REG_WRITE,0x1f,&data);   //change Token ring
				data=0xa3d0;
				c_net_mdio_ctrl(REG_WRITE,0x10,&data);   // read token ring data_addr=0x28 value;
				c_net_mdio_ctrl(REG_READ,0x11,&tokenDatalow);   //read low data
				//printf("(%d)tokenDatalow=0x%x\n",__LINE__,(int)tokenDatalow);
				c_net_mdio_ctrl(REG_READ,0x12,&tokenDataHi);
				//printf("(%d)tokenDataHi=0x%x\n",__LINE__,(int)tokenDataHi);
				
				slc_mse_exta=(((tokenDataHi&0x7f))<<16)+tokenDatalow;
				//printf("(%d)slc_mse_exta=0x%x\n",__LINE__,(int)slc_mse_exta);
				data=0;
				c_net_mdio_ctrl(REG_WRITE,0x1f,&data);   //change main mode
	
				div=1024*2048;           //div= 2^21
				if(slc_mse_exta>0)
					mse= (div/slc_mse_exta);
				snr=20*log10(mse);
				if(snr>maxsnr)
					 maxsnr=snr;
				if(snr<minsnr)
					minsnr=snr;
				meansnr+=snr;
				//printf("tokenDataHi=0x%x,tokenDatalow=0x%0x,slc_mse_exta=0x%x,mse=%f,snr(DB)=%f\n",(int)tokenDataHi,(int)tokenDatalow,(int)slc_mse_exta,mse,snr);
			}

			meansnr= meansnr/u4ReadCount;

			printf("Gain(%d)\n",(int)gain);
			printf("SNR: min:max(%f: %f),meansnr(%f)\n",minsnr,maxsnr, meansnr);
		return (i4Ret);
  
   
   
   }

   
static INT32 cli_c_net_get_speed_duplex(INT32 i4_argc, const CHAR**  pps_argv)
{
    
    NI_SPEED_T t_ni_speed = 0;
    return c_net_ni_get_speed_duplex((CHAR*)pps_argv[1], &t_ni_speed);
}

   /*-----------------------------------------------------------------------------
                       External functions implementations
    ----------------------------------------------------------------------------*/
   INT32 linuxnet_cli_init(VOID)
   {
       INT32   i4_ret;

       i4_ret = u_cli_attach_cmd_tbl( root_cmd_tbl, CLI_CAT_MW, CLI_GRP_PIPE );
       if (i4_ret != CLIR_OK && i4_ret != CLIR_NOT_INIT)
       {
           return CLIR_NOT_INIT;
       }

       return CLIR_OK;
   }

#endif /* CLI_SUPPORT */

#ifdef HTTP_FM_CACHE_ENABLE
   extern VOID x_net_http_reg_cli(VOID);
#endif

   VOID x_net_ip_reg_cli(VOID)
   {
#ifdef CLI_SUPPORT
       linuxnet_cli_init();
#if LPCH_SUPPORT_CINEMANOW
      //extern INT32 wv_cli_init(VOID);
      //wv_cli_init();
#endif

#ifdef CONFIG_MW_NET_WIFI

    extern INT32 x_wifi_cli_init(VOID);
    x_wifi_cli_init();
#endif
#ifdef HTTP_FM_CACHE_ENABLE
        x_net_http_reg_cli();
#endif

#else
       /* cli is disable */
#endif
   }


