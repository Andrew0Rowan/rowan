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


#include <stdio.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/ioctl.h>
#include <linux/if.h>     /* for struct ifreq */
#include <net/if_arp.h> /* for ARPHRD_ETHER */
#include <fcntl.h>
#include <netinet/ip_icmp.h> // for ping
//#include <nand_ptt.h>
#include <time.h>

#include <sys/un.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <linux/if_ether.h>
#include <linux/if_packet.h>
#include <linux/if_addr.h>
#include <linux/netlink.h>
#include <linux/rtnetlink.h>
#include <pthread.h>
#include <sys/prctl.h>

#include <ifaddrs.h>
#include "curl/curl.h"
#include <net/route.h>
#include <linux/icmpv6.h>
#include <poll.h>
#include "common.h"
#include "u_os.h"
//#include "x_fm.h"
#include "u_common.h"
#include "u_dbg.h"
#include "u_net_common.h"
#include "u_net_util.h"
//#include "u_net_drv_if.h"
#include "u_net_ni_if.h"
#include "u_net_oid.h"   // for OID

#include "u_dhcpc_api.h"
//#include "u_http_firmware_upgrade.h"
#include "u_ipcd.h"
#include "x_net_ni_if.h"
//#include "x_net_wlan.h"
//#include "x_modutil.h"
//#include "c_dm.h"
//#include "x_mw_config.h"
#include "c_net_config.h"
//#include "x_mw_config.h"
#include "x_bsp_linux.h"
//#include "sys_config.h"

#include <sys/socket.h>
#include <netinet/in.h>

#if CONFIG_PHILIPS_WMDRM_KEY
#include "philips_front_api.h"
#include "diversity_if.h"
#endif

#define NIFNAMELEN 15
#define NETINFO_DAEMON 0
#define FORK2NETINFD 0
#define LINUX_NETWORK_MAC_ADDRE_PATH   "/misc/network.ini"
#define LINUX_MISC_FOLDER              "/misc"
#define GETHOSTBYNAME_TIMEOUT_SEC     30      /* waiting time for c_net_gethostbyname    */
#define SOCKET_CONNECTION_TIMEOUT_SEC 30      /* waiting time for c_net_proxy_connection */
#define PINGHOSTBYNAME_TIMEOUT_SEC    3      /* waiting time for c_net_pinghostbyname   */

#define NET_INADDR_LOOPBACK         ((UINT32) 0x7f000001)
#define NET_INADDR_ANY              ((UINT32) 0x00000000)
#define NET_INADDR_BROADCAST        ((UINT32) 0xffffffff)

// DHCP
char *dhcpscript = "/sbin/dhcpc.script";
char *dhcpinfo = "/var/dhcpinfo";
// DNS
char *dnsscript = "/sbin/dns.script";
// Ifconfig
char *ifscript = "/sbin/ifconfig.script";

#if (NETINFO_DAEMON)
//#include "utility.h"
#include "request.h"
#include "netinfd.h"
#endif

#if !CONFIG_EMMC_BOOT
#define BLOCK_DEV "/dev/mtd%d"
#else
#define BLOCK_DEV "/dev/mmcblk0p%d"
#endif
#define ERROR_RET(ret, x...) \
{ \
	if(ret){  \
		printf("[NW]Error happen in %s - %d", __FUNCTION__, __LINE__); \
		printf(x);  \
		return ret; \
	}\
}

#define NET_ERROR_RET(ret, fd, x...) \
{ \
	if(ret){  \
		printf("[NW]Error happen in %s - %d, errno : %d", __FUNCTION__, __LINE__, errno); \
		printf(x); \
		close(fd); \
		return ret;\
	}\
}

#define CHECK_NULL_PTR(x) \
{ \
	if(NULL == x){  \
		printf("[NW]Input NULL pointer in %s - %d, errno : %d", __FUNCTION__, __LINE__, errno); \
		return -1;\
	}\
}

static int g_net_ipv6_is_print=1;
#define NET_LOG(X...) \
	if(g_net_ipv6_is_print == 1)\
	{ \
		printf("[NW]"X); \
	}
#define NET_ERR_LOG(X...) \
    if(g_net_ipv6_is_print == 1)\
    { \
        printf("[NW]ERR"X); \
    }

#define dbg_mw_printf printf

/***** NIC Monitor *****/
HANDLE_T h_ni_monitor_thread;
BOOL     fg_NiMonitor_Thrd_Create = FALSE;

#if (MW_SUPPORT_ADDON_HOA || MW_SUPPORT_APP_FRAMEWORK_HOA)
#include "../../../mediamw/inc/hoa_aoi_wrapper.h"
INT32 (* SUB_AOI_ADDONHOST_SendEventToApp_mw_fn)(INT32 evt, UINT32 param) = NULL;
#endif


typedef struct ni_lx_monitor_list_s
{
    CHAR                        sz_NifName[NIFNAMELEN];
    NI_DRV_EV_T                 e_event;
    ni_ev_notify_fnct           pf_notify;
    BOOL                        fgNeedNotify;
    struct ni_lx_monitor_list_s *pt_next;
} NI_LX_MONITOR_LIST_T;

static NI_LX_MONITOR_LIST_T *pt_lx_ni_monitor_list_root = NULL;
/***** NIC Monitor *****/

/***** Ping *****/

enum {
	DefaultDataLen = 56,
	IpMaxLen    = 60,
	IcmpMaxLen  = 76,
	MaxBufSz    = 1500
};

enum{
	icmp6_echo_offset = 8,
	icmp6_least_len = 16,
	icmp6_echo = 128,
	icmp6_reply = 129,
};

/***** Ping *****/

/***** DHCP Monitor *****/

HANDLE_T h_dhcp_monitor_thread;
BOOL     fg_dhcp_thread_running = FALSE;
int      g_dhcp_IsDhcpNeedStop = FALSE;

BOOL fg_ntp_sync_ready = FALSE;


typedef enum
{
    DHCP_START = 0x0,
    DHCP_RESTART = 0x1
} DHCP_CMD;

typedef struct dhcp_lx_monitor_s
{
    CHAR                        sz_NifName[NIFNAMELEN];
    x_dhcpc_nfy_fct             pf_notify;
    DHCP_CMD                    eDhcpCmd;
} DHCP_LX_MONITOR_T;

typedef struct ntp_para_t
{    
    x_ntp_nfy_fct             	pf_notify;
    CHAR                    	ac_command_name[70];
} NTP_PARA_T;


static DHCP_LX_MONITOR_T t_lx_dhcp_monitor_g = {{0}};

/***** DHCP Monitor *****/

/***** PPPPoE *****/
typedef struct pppoe_conf_t
{
    CHAR * ifname;
    CHAR * username;
    CHAR * password;
    x_pppoe_nfy_fct cb_fn;
} PPPOE_CONF_T;

static PPPOE_CONF_T pppoe_config = {0};
/***** PPPPoE *****/


/***** MPTool Server ****/
HANDLE_T h_MPTOOL_SERVER_thread;
BOOL     fg_MPTOOL_SERVER_Thrd_Create = FALSE;
#define MPSPORT "8530"
#define BACKLOG 10
/*function declare*/
INT32 c_net_mptool_server_startup();
INT32 c_net_ni_get_mac(CHAR *psz_name, CHAR *pac_mac);
INT32 get_ifconfig(struct ifconf **conf);
INT32 build_etheraddr(struct ifreq *ifr, struct sockaddr_ll *out_etheraddr);
INT32 build_arp_req_packet(struct ifreq *ifr, unsigned long in_inet_Dst_Addr, unsigned char** out_arp_req_packet);
INT32 exe_arp_request(struct sockaddr_ll *in_etheraddr, unsigned char *in_arp_req_packet, unsigned char * out_arp_res_packet);
INT32 get_src_mac_from_arp_packet(unsigned char *in_arp_rsp_packet, char *out_mac);
int print_if_config_info(struct ifconf *conf);

extern BOOL x_cpsm_kb_write
(
UINT8* pui1_upg_kb,
UINT32 ui4_upg_kb_sz,
UINT8* pui1_ins_ck_st
);

//by suma suo for LG optioncode download 20140926
#if CONFIG_MW_CUSTOM_KLG
extern INT32 x_meq_upg(UINT8*	puData,	UINT32 u4Size);
extern INT32 x_optioncode_upg(UINT8 * puData,UINT32 u4Size);
#endif
//end


#if CONFIG_SUPPORT_NET_MPTOOL
#if CONFIG_SUPPORT_SS
const char* opt_file_name = "/cust_part_1/model_option.dat";
INT32 Sumsung_opcode_write_fn(UINT8 * puData, UINT32 u4Size)
{
	/*open file*/
	FILE* fd = fopen(opt_file_name, "w");
	int wlen = 0;
	
  wlen = fwrite(puData, 1, u4Size, fd);
  
  printf("<MW Debug> Write file in path : %s, %s - %ld, wlen : %d", opt_file_name,puData,u4Size, wlen);

  fclose(fd);
	return (wlen - u4Size);	
}


INT32 (* x_upg_opcode_write_fn)(UINT8 * puData, UINT32 u4Size) = Sumsung_opcode_write_fn;
#elif CONFIG_MW_CUSTOM_KLG //by suma suo for LG optioncode download 20140926
INT32 (* x_upg_opcode_write_fn)(UINT8 * puData, UINT32 u4Size) = x_optioncode_upg;
#else
INT32 (* x_upg_opcode_write_fn)(UINT8 * puData, UINT32 u4Size) = NULL;
#endif

#if CONFIG_SUPPORT_BE_IN_DRAM
INT32 (* upg_from_mptool_Step1_fn)(UCHAR * be_file_part, INT32 be_part_len) = NULL;
INT32 (* upg_from_mptool_step2_fn)(UCHAR * be_file_part, INT32 be_part_len) = NULL;
#else
INT32 (* upg_from_mptool_Step1_fn)(CHAR * be_mnt_path, CHAR * be_file_path) = NULL;
INT32 (* upg_from_mptool_step2_fn)(CHAR * be_mnt_path, CHAR * be_file_path) = NULL;
#endif

#endif

#if CONFIG_MW_CUSTOM_SFLP
static x_pctool_upg_nfy g_pctool_upg_fn= NULL;
static BOOL b_mpserver_pctool_upg=FALSE;

BOOL mpserver_is_pctool_upgrade(VOID)
{
	return b_mpserver_pctool_upg;
}

VOID mpserver_set_pctool_upgrade(BOOL flag)
{
	b_mpserver_pctool_upg=flag;
}
	

INT32 pctool_upg_register(x_pctool_upg_nfy pf_nfy)
{
    g_pctool_upg_fn=pf_nfy;

    return 0;
}
#endif

enum MPTOOL_ACTION_CMD_T {  //new
    MPTOOL_SEND_KEYBLOCK = 1,
    MPTOOL_INSTALL_KEYBLOCK,
    MPTOOL_SEND_MAC,
    MPTOOL_INSTALL_MAC,
    MPTOOL_SEND_OPT,
    MPTOOL_INSTALL_OPT,
    MPTOOL_DISCONNECT,
    MPTOOL_SEND_FE,
    MPTOOL_INSTALL_FE,
    MPTOOL_CHECK_KEY,
    MPTOOL_CHECK_MAC,
    MPTOOL_CLOSE_SERVER,
    MPTOOL_SEND_BE_START,
    MPTOOL_SEND_BE,
    MPTOOL_SEND_BE_END,
    MPTOOL_INSTALL_BE,
    MPTOOL_SYNC_TIME,
    MPTOOL_GET_MODEL_NAME,
    MPTOOL_SEND_CUST_DATA,
    MPTOOL_INSTALL_CUST_DATA,
    MPTOOL_SEND_MEQ,
    MPTOOL_INSTALL_MEQ
};
volatile static INT32 i4_fe_progress;
INT32 c_dhcpc_restart(CHAR *ps_ni_name);
INT32 c_dhcpc_start(CHAR *ps_ni_name, x_dhcpc_nfy_fct fn);
INT32 c_dhcpc_stop(CHAR * ps_ni_name);
INT32 c_dhcpc_get_info(CHAR * ps_ni_name, MT_DHCP4_INFO_T * pt_info);
/***** MPTool Server ****/


//gethostbyname
HANDLE_T h_host_monitor_thread;
BOOL     fg_host_thread_running = FALSE;
struct hostent *pt_hostnet = NULL;
struct hostent *    c_net_gethostbyname(const CHAR *ps_server_url);
//gethostbyname
struct hostent *    c_net_gethostbyname_ex(const CHAR *ps_server_url);// samsung
struct hostent *    c_net_gethostbyname_timeout(const CHAR *ps_server_url,INT32 timeout);// samsung

//getaddrinfo
HANDLE_T h_addrinfo_monitor_thread;
BOOL     fg_addrinfo_thread_running = FALSE;
struct addrinfo *answer = NULL;
struct addrinfo *c_net_get_addrinfo_by_addrtype(const NET_ADDR_INFO_LOOK_UP *ptAddrInfoLookup);
//getaddrinfo
/***** pinghostbyname *****/
HANDLE_T h_ping_host_thread;
BOOL     fg_ping_host_thread_running = FALSE;
INT32    ping_host_result;
INT32    c_net_pinghostbyname(const CHAR * ps_server_url);
/***** pinghostbyname *****/

//net cmd thread
typedef enum
{
    NET_ARPING = 0x0
} NET_CMD_TYPE;

#define net_cmd_len 256
HANDLE_T h_net_cmd_thread;
BOOL     fg_net_cmd_thread_running = FALSE;
typedef struct net_cmd_s
{
    CHAR                        s_cmd[net_cmd_len];
    NET_CMD_TYPE                eCmdType;
    void                       (*pf_conflict_notify)(BOOL fgConflict);
} NET_CMD_T;

//net cmd thread

/* c_net_get_mac_by_ip */
INT32 c_net_get_mac_from_sys(CHAR * pac_mac);
INT32 c_dhcpc_get_info(CHAR * ps_ni_name, MT_DHCP4_INFO_T * pt_info);
extern INT32 c_net_wlan_get_mac_addr(UINT8 *pMacAddr);
/* c_net_get_mac_by_ip */

/* c_net_get_ni_speed */


typedef struct _SPEED_REQUEST_T
{
    CHAR if_name[8];
    get_ni_speed_cb_fp cb_fp;

}SPEED_REQUEST_T;

NET_SPEED_T net_speed;
BOOL fg_ni_speed_thread_running = FALSE;
HANDLE_T h_ni_speed_thread;
/* c_net_get_ni_speed */

/* c_net_test_connection_async */
typedef struct _CONN_TEST_REQ_T
{
    CHAR *ps_server_url;
    UINT16 ui2_port;
    UINT16 ui2_timeout;
    connection_test_cb_fp cb_fp;
}CONN_TEST_REQ_T;
BOOL fg_connection_test_thread_running = FALSE;
HANDLE_T h_connection_test_thread;
/* c_net_test_connection_async */

/* Wake-On-Lan */
typedef enum {
    ETH_GET_WOL_STATUS = 0,
    ETH_SET_WOL
}ETH_WOL_CMD;

typedef enum{
    ETH_WOL_OFF = 0,
    ETH_WOL_ON
}ETH_WOL_STATUS;

struct magic_p_mac {
    UCHAR mac1[6];
    UCHAR mac2[6];
    UCHAR mac3[6];
    UCHAR mac4[6];
    UCHAR mac5[6];
    UCHAR mac6[6];
    UCHAR mac7[6];
    UCHAR mac8[6];
    UCHAR mac9[6];
    UCHAR mac10[6];
    UCHAR mac11[6];
    UCHAR mac12[6];
    UCHAR mac13[6];
    UCHAR mac14[6];
    UCHAR mac15[6];
    UCHAR mac16[6];
};

typedef struct _WOL_REQ_T
{
    CHAR if_name[16];
    wol_notify_cb_fn wol_cb;
}WOL_REQ_T;

HANDLE_T h_wol_monitor_thread;
BOOL     fg_wol_thread_running = FALSE;
/* Wake-On-Lan */

static CHAR      s_hostname[64] = "MT8520-1";
static INT32     g_i4CurIfType = IF_TYPE_OTHER;
NET_DRV_IF_T rDev = {0};
#define ip_addr_len 16

#define MTD_MAC_ADDR_1                           _mtd_mac_addr_1
#define MTD_MAC_ADDR_1_NAME                      "mac_addr_1"
#define MTD_MAC_ADDR_2                           _mtd_mac_addr_2
#define MTD_MAC_ADDR_2_NAME                      "mac_addr_2"
char _mtd_mac_addr_1[16]     = {0};
char _mtd_mac_addr_2[16]     = {0};

/*arp*/
struct arp_packet
{
   struct ethhdr ethd;
   struct arphdr arphd;
   unsigned char eth_src_addr[6];
   unsigned char inet_src_addr[4];
   unsigned char eth_dst_addr[6];
   unsigned char inet_dst_addr[4];
   unsigned char padding[18];
};

void print_ARPMsg(struct arp_packet *packet);
void print_data(unsigned char *buf);
unsigned char *BuildArpRequestPacket(unsigned char* Eth_Src_Addr, unsigned long Inet_Src_Addr, unsigned long Inet_Dst_Addr);
//int get_local_mac(char *dev, unsigned char *mac);
//int get_local_ip(char *dev, unsigned long *ip_addr);
int c_net_get_ifindex(char *dev, int *index);

INT32 c_net_reboot(void)
{
    INT32 i4fd = open("/dev/bsp",O_RDWR);
    if (i4fd < 0)
    {
        printf("[MW] Reboot failed No1!\n");
        return -1;
    }

    if (ioctl(i4fd, IOCTL_BSP_REBOOT) < 0)
    {
        close(i4fd);
        printf("[MW] Reboot failed No2!\n");
        return -1;
    }

    close(i4fd);

    printf("[UPGPROC] 0703 Reboot Sucess!\n");
    return 0;
}

void print_ARPMsg(struct arp_packet *packet)
{
   unsigned char tmp[7];

   memset(tmp, 0, 7);
   memcpy(tmp, packet->eth_src_addr, 6);
   printf("[DLNA]eth src addr:%x %x %x %x %x %x \n", tmp[0], tmp[1], tmp[2], tmp[3], tmp[4], tmp[5]);

   memset(tmp, 0, 7);
   memcpy(tmp, packet->inet_src_addr, 4);
   printf("[DLNA]inet src addr:%x %x %x %x\n", tmp[0], tmp[1], tmp[2], tmp[3]);

   memset(tmp, 0, 7);
   memcpy(tmp, packet->eth_dst_addr, 6);
   printf("[DLNA]eth dst addr:%x %x %x %x %x %x\n", tmp[0], tmp[1], tmp[2], tmp[3], tmp[4], tmp[5]);

   memset(tmp, 0, 7);
   memcpy(tmp, packet->inet_dst_addr, 4);
   printf("[DLNA]inet dst addr:%x %x %x %x\n", tmp[0], tmp[1], tmp[2], tmp[3]);

   return;
}

void print_data(unsigned char *buf)
{
   int i;
   if(buf != NULL)
   {
      for(i = 0; i < 60; i++)
      {
         if(i != 0 && i%16 == 0)
         {
            printf("\n");
         }
         printf("%.2X", buf[i]);
      }
      printf("\n");
   }

   return;
}

unsigned char *BuildArpRequestPacket(unsigned char* Eth_Src_Addr, unsigned long Inet_Src_Addr, unsigned long Inet_Dst_Addr)
{
   static struct arp_packet packet;
//   unsigned long netul;

   memset(packet.ethd.h_dest, 0xff, ETH_ALEN);
   memcpy(packet.ethd.h_source, Eth_Src_Addr, ETH_ALEN);
   packet.ethd.h_proto = htons(ETH_P_ARP);
   memset(&(packet.arphd), 0, sizeof(struct arphdr));
   packet.arphd.ar_hrd = htons(ARPHRD_ETHER);
   packet.arphd.ar_pro = htons(ETH_P_IP);
   packet.arphd.ar_hln = ETH_ALEN;
   packet.arphd.ar_pln = 4;
   packet.arphd.ar_op = htons(ARPOP_REQUEST);

   memcpy(packet.eth_src_addr, Eth_Src_Addr, 6);
   //printf("[DLNA]inet_src_addr:0x%lx\n", Inet_Src_Addr);
  // netul = (unsigned long)htonl(Inet_Src_Addr);
   //printf("[DLNA]inet_src_addr netul:0x%lx\n", netul);
//   memcpy(packet.inet_src_addr, &netul, 4);
memcpy(packet.inet_src_addr, &Inet_Src_Addr, 4);

   memset(packet.eth_dst_addr, 0, 6);
   //printf("[DLNA]dst addr:0x%lx\n", Inet_Dst_Addr);
   //netul = (unsigned long)htonl(Inet_Dst_Addr);
   //printf("[DLNA]inet_src_addr netul:0x%lx\n", netul);
   memcpy(packet.inet_dst_addr, &Inet_Dst_Addr, 4);

   memset(packet.padding, 0, 18);

   return (unsigned char*)&packet;
}

/*
int get_local_mac(char *dev, unsigned char *mac)
{
   int s = socket(AF_INET, SOCK_DGRAM, 0);
   struct ifreq req;
   int err;

   strcpy(req.ifr_name, dev);
   err = ioctl(s, SIOCGIFHWADDR, &req);
   close(s);

   if(err == -1)
   {
      return err;
   }

   memcpy(mac, req.ifr_hwaddr.sa_data, 6);

   return 0;
}
*/
INT32 c_net_ni_get_ip(char *dev, unsigned long *ip_addr)
{
   int s=0;
   struct ifreq req={{{0}}};
   int err=0;

   strncpy(req.ifr_name, dev, strlen(dev));
   if((s = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
   {
      printf("[DLNA]get local ip error\n");
      return -1;
   }
   err = ioctl(s, SIOCGIFADDR, &req);
   close(s);

   if(err == -1)
   {
	  printf("<IP>ioctl get fail!errno = %d\n", errno);   
      return err;
   }
   *ip_addr= ((struct sockaddr_in*)&req.ifr_addr)->sin_addr.s_addr;
   printf("<IP>IP = %s\n", inet_ntoa(*(struct in_addr*)ip_addr));   

   return 0;
}

int c_net_get_ifindex(char *dev, int *index)
{
   int s;
   struct ifreq req={{{0}}};
   int err;

   strncpy(req.ifr_name, dev, strlen(dev));
   if((s = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
   {
      printf("[DLNA]get ifindex error\n");
      return -1;
   }

   err = ioctl(s, SIOCGIFINDEX, &req);
   close(s);

   if(err == -1)
   {
      return err;
   }
   *index = req.ifr_ifindex;
   return 0;
}

#define NET_IF_NUMBER 8

INT32 build_etheraddr(struct ifreq *ifr, struct sockaddr_ll *out_etheraddr)
{
   int ret = 0;
   int my_ifindex = 0;

   memset(out_etheraddr, 0, sizeof(struct sockaddr_ll));
   out_etheraddr->sll_family = AF_PACKET;
   out_etheraddr->sll_protocol = htons(ETH_P_ARP);
   ret = c_net_get_ifindex(ifr->ifr_ifrn.ifrn_name, &my_ifindex);
   if(ret != 0)
   {
      ret = -1;
      goto PGM_END;
   }
   out_etheraddr->sll_ifindex = my_ifindex;

PGM_END:
   //dbg_mw_printf("[DLNA]%s ret code:%d\n", __FUNCTION__, ret);
   return ret;
}

INT32 get_ifconfig(struct ifconf **conf)
{
   int length = 0;
   int ret = 0;
   int sockfd = 0;

   sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
   if (sockfd <= 0)
   {
      ret = -1;
      goto PGM_END;
   }

   *conf = (struct ifconf *)malloc(sizeof(struct ifconf) + 1);
   if((*conf) == NULL)
   {
      ret = -2;
      goto PGM_END;
   }

   length = NET_IF_NUMBER * sizeof(struct ifreq);
   (*conf)->ifc_ifcu.ifcu_buf = malloc(length + 1);
   if((*conf)->ifc_ifcu.ifcu_buf == NULL)
   {
      ret = -3;
      goto PGM_END;
   }
   memset((*conf)->ifc_ifcu.ifcu_buf, 0, length + 1);
   (*conf)->ifc_len = length + 1;

   ret = ioctl(sockfd, SIOCGIFCONF, *conf);
   if (ret < 0)
   {
      ret = -4 ;
      goto PGM_END;
   }

PGM_END:
   dbg_mw_printf("[DLNA]%s ret code:%d\n", __FUNCTION__, ret);
   if(!ret) //success
   {
      //do nothing
   }
   else //fail
   {
      if(*conf)
      {
         if((*conf)->ifc_ifcu.ifcu_buf)
         {
            free((*conf)->ifc_ifcu.ifcu_buf);
            (*conf)->ifc_ifcu.ifcu_buf = NULL;
         }
         free(*conf);
         *conf = NULL;
      }
   }

   if(sockfd > 0)
   {
      close(sockfd);
      sockfd = 0;
   }
   return ret;
}

VOID free_ifconfig(struct ifconf *conf)
{
   if(conf)
   {
      if(conf->ifc_ifcu.ifcu_buf)
      {
         free(conf->ifc_ifcu.ifcu_buf);
         conf->ifc_ifcu.ifcu_buf = NULL;
      }
      free(conf);
      conf = NULL;
   }

   return;
}
INT32 exe_arp_request(struct sockaddr_ll *in_etheraddr, unsigned char *in_arp_req_packet, unsigned char * out_arp_res_packet)
{
   int sockfd = 0;
   int ret = 0;
   struct timeval time_out;
   fd_set readset,writeset;
   int nfound = 0;
   struct arp_packet *req_packet = NULL;
   struct arp_packet *rsp_packet = NULL;

   req_packet = (struct arp_packet *)in_arp_req_packet;

   sockfd = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_ARP));
   if(sockfd <= 0)
   {
      ret = -1;
      goto PGM_END;
   }

   if(bind(sockfd, (struct sockaddr*)in_etheraddr, sizeof(struct sockaddr_ll)) != 0)
   {
      ret = -2;
      goto PGM_END;
   }


   //printf("[DLNA]send Arp pcket:\n");
   //print_data(in_arp_req_packet);
   //print_ARPMsg((struct arp_packet*)in_arp_req_packet);

   ret = sendto(sockfd, in_arp_req_packet, sizeof(struct arp_packet), 0, (struct sockaddr*)in_etheraddr, sizeof(struct sockaddr_ll));

   if (ret < 0)
   {
      ret = -3;
      
      goto PGM_END;
   }
   else
   {
    printf("[DLNA]%s:%d:ret=%d\n", __FUNCTION__, __LINE__, ret);
    
   }

   time_out.tv_sec = 0;
   time_out.tv_usec =500000; //500ms

   while(1)
   {
      FD_ZERO(&readset);
      FD_ZERO(&writeset);
      FD_SET(sockfd, &readset);

      nfound = select(sockfd+1, &readset, &writeset, NULL, &time_out);
      if(nfound == 0)
      {
         ret = -4;
         goto PGM_END;
      }
      else if(nfound < 0)
      {
         ret = -5;
         goto PGM_END;
      }

      if(FD_ISSET(sockfd, &readset))
      {
         memset(out_arp_res_packet, 0, 80);
         recvfrom(sockfd, out_arp_res_packet, 80, 0, NULL, 0);
         //printf("[DLNA]recv arp packet\n");
         //print_data(out_arp_res_packet);
         //print_ARPMsg((struct arp_packet*)out_arp_res_packet);
         rsp_packet = (struct arp_packet*)(out_arp_res_packet);
         if(ntohs(rsp_packet->arphd.ar_op) != 2) //if not arp respond
         {
            continue;
         }

         //dbg_mw_printf("[DLNA]dst_ip:[%x %x %x %x]\n", req_packet->inet_src_addr[0], req_packet->inet_src_addr[1], req_packet->inet_src_addr[2], req_packet->inet_src_addr[3]);
         //dbg_mw_printf("[DLNA]rsp_ip:[%x %x %x %x]\n", rsp_packet->inet_src_addr[0], rsp_packet->inet_src_addr[1], rsp_packet->inet_src_addr[2], rsp_packet->inet_src_addr[3]);
         if(memcmp(req_packet->inet_dst_addr, rsp_packet->inet_src_addr, 4) != 0)
         {
            continue;
         }
         else
         {
            break;
         }
      }
   }

   PGM_END:
   //dbg_mw_printf("[DLNA]%s ret code:%d\n", __FUNCTION__, ret);
   if(sockfd > 0)
   {
      close(sockfd);
   }

   return ret;
}

INT32 build_arp_req_packet(struct ifreq *ifr, unsigned long in_inet_Dst_Addr, unsigned char** out_arp_req_packet)
{
   int ret = 0;
   static struct arp_packet packet;
   char if_mac[7];
   unsigned long ul_if_ip;

   memset(if_mac, 0, 7);
   memset(&packet, 0, sizeof(struct arp_packet));
   memset(packet.ethd.h_dest, 0xff, ETH_ALEN);
   if(c_net_ni_get_mac(ifr->ifr_ifrn.ifrn_name, if_mac) != 0)
   {
      ret = -1;
      goto PGM_END;
   }
   memcpy(packet.ethd.h_source, if_mac, ETH_ALEN);
   packet.ethd.h_proto = htons(ETH_P_ARP);
   packet.arphd.ar_hrd = htons(ARPHRD_ETHER);
   packet.arphd.ar_pro = htons(ETH_P_IP);
   packet.arphd.ar_hln = ETH_ALEN;
   packet.arphd.ar_pln = 4;
   packet.arphd.ar_op = htons(ARPOP_REQUEST);

   memcpy(packet.eth_src_addr, if_mac, 6);
   if(c_net_ni_get_ip(ifr->ifr_ifrn.ifrn_name, &ul_if_ip) != 0)
   {
      ret = -2;
      goto PGM_END;
   }

   memcpy(packet.inet_src_addr, &ul_if_ip, 4);
   memcpy(packet.inet_dst_addr, &in_inet_Dst_Addr, 4);

PGM_END:
   //dbg_mw_printf("[DLNA]%s ret code:%d\n", __FUNCTION__, ret);
   if(!ret)
   {
      *out_arp_req_packet = (unsigned char*)(&packet);
   }

   return ret;
}

INT32 get_src_mac_from_arp_packet(unsigned char *in_arp_rsp_packet, char *out_mac)
{
   struct arp_packet *rsp_packet = NULL;
   int ret = 0;

   if(!in_arp_rsp_packet || !out_mac)
   {
      ret = -1;
      goto PGM_END;
   }
   rsp_packet = (struct arp_packet*)in_arp_rsp_packet;
   memcpy(out_mac, rsp_packet->eth_src_addr, 6);

PGM_END:
   //dbg_mw_printf("[DLNA]%s ret code:%d\n", __FUNCTION__, ret);
   return ret;
}
int print_if_config_info(struct ifconf *conf)
{
   int ret = 0;
   unsigned int if_num = 0;
   struct ifreq *ifr = NULL;
   unsigned long ul_if_ip;
   unsigned char mac_addr[7];
   struct in_addr inaddr;

   if(!conf)
   {
      ret = -1;
      goto PGM_END;
   }

   for(if_num = 0; if_num < NET_IF_NUMBER; if_num++)
   {
      ret = 0;
      memset(mac_addr, 0, 7);
      memset(&inaddr, 0, sizeof(struct in_addr));

      dbg_mw_printf("[DLNA]interface num:[%d/8]\n", if_num+1);

      ifr = &(conf->ifc_ifcu.ifcu_req[if_num]);
      dbg_mw_printf("[DLNA]interface name:[%s]\n", ifr->ifr_ifrn.ifrn_name);

      if(c_net_ni_get_ip(ifr->ifr_ifrn.ifrn_name, &ul_if_ip) != 0)
      {
         ret = -2;
         continue;
      }
      memcpy(&inaddr, &ul_if_ip, 4);
      dbg_mw_printf("[DLNA]interface IP:[%s]\n", inet_ntoa(inaddr));

      if(c_net_ni_get_mac(ifr->ifr_ifrn.ifrn_name, (char*) mac_addr) != 0)
      {
         ret = -3;
         continue;
      }
      dbg_mw_printf("[DLNA]interface addr:[%x %x %x %x %x %x]\n", mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[6]);
   }

PGM_END:
   dbg_mw_printf("[DLNA]%s ret code:%d\n", __FUNCTION__, ret);

   return ret;
}

/************************
******return 0 - OK *******
******only  ipv4***r*******
*************************/
INT32 c_net_ip_to_ifname(int inaddr, char* ifname)
{
	struct ifconf* conf = NULL;
	struct ifreq *ifr = NULL;				/*point to one interface of device*/
	int if_addr = 0;
	int if_num = 0;
	int ret = 1;

	if(NULL == ifname)
	{
	   printf("<MW>Input string null, error!\n");
	   goto PGM_END;
	}		
	
	if(get_ifconfig(&conf) != 0)
	{
	   printf("<MW>Get ifconf error\n");
	   goto PGM_END;
	}
    for(if_num = 0; if_num < 8; if_num++)
    {	
		ifr = &(conf->ifc_req[if_num]);    
		if (strlen(ifr->ifr_name) == 0)
		{
		   break;
		}
		
		if_addr = ((struct sockaddr_in*)(&(ifr->ifr_addr)))->sin_addr.s_addr;
		if((if_addr == NET_INADDR_BROADCAST) ||
		   (if_addr == NET_INADDR_ANY) 	  ||
		   (if_addr == NET_INADDR_LOOPBACK))
		{
		   continue;
		}
		if(if_addr == inaddr)
		{
			ret = 0;
			strncpy(ifname,ifr->ifr_name,20);
			break;
		}
    }

	
PGM_END:
	if(conf)
	{
	   free_ifconfig(conf);
	}
	if(ret)		
		printf("<MW>Get ifname by ip fail!\n");
	
	return ret;

}

INT32 c_net_socket_to_ifname(int socket, char *ifname)
{
	struct sockaddr Myname = {0};
	int Mylen=sizeof(Myname);
	int ret = 0;
	
	ret = getsockname(socket, &Myname, (socklen_t*)&Mylen);
	if(ret)
	{
		printf("<MW>Get socket bind ip error: %d\n",errno);
		return -1;
	}
	else
		printf("<MW>Socket bind ip is: %s\n", inet_ntoa(((struct sockaddr_in *)&Myname)->sin_addr));
	ret = c_net_ip_to_ifname(((struct sockaddr_in *)&Myname)->sin_addr.s_addr, ifname);
	if(ret)
		printf("<MW>Ip translate to ifname error: %d\n",errno);
	else
		printf("<MW>Get socket ifname : %s\n",ifname);
	
	return ret;
}



INT32 c_net_ip_to_mac(char* in_addr, char* out_mac)
{
   int ret = 0;
   int internal_ret = 0;
   unsigned int if_num;
   struct ifconf *conf = NULL;             /*store informations of device all interfaces which have ip addresss*/
   struct ifreq *ifr = NULL;               /*point to one interface of device*/
   struct sockaddr_ll etheraddr;
   unsigned char* arp_req_packet = NULL;   /*pointer to static char array*/
   static unsigned char arp_rsp_packet[80];/*use frequencly, set as static*/
   unsigned int addr;


   if(get_ifconfig(&conf) != 0)
   {
      //dbg_mw_printf("[DLNA]get ifconf error\n");
      ret = -1;
      goto PGM_END;
   }

   /*dbg msg, should remove at release*/
   //print_if_config_info(conf);
   printf("[DEBUG] dlna send arp for address : %s \n", in_addr);
   
   for(if_num = 0; if_num < NET_IF_NUMBER; if_num++)
   {
      ret = 0;
      ifr = &(conf->ifc_ifcu.ifcu_req[if_num]);

	  if(-1 == inet_addr(in_addr))
	  {
		  printf("[DEBUG] arp address illegal !\n");	 		  
		  ret = -3;
		  break;
	  }

      //dbg_mw_printf("[DLNA]interface num:[%d]\n", if_num+1);

      if (strlen(ifr->ifr_ifrn.ifrn_name) == 0)
      {
         //dbg_mw_printf("[DLNA]interface name null\n");
         continue;
      }

      //dbg_mw_printf("[DLNA]interface name:[%s]\n", ifr->ifr_ifrn.ifrn_name);
      addr = ntohl(((struct sockaddr_in*)(&(ifr->ifr_ifru.ifru_addr)))->sin_addr.s_addr);
      if((addr == NET_INADDR_BROADCAST) ||
         (addr == NET_INADDR_ANY)       ||
         (addr == NET_INADDR_LOOPBACK))
      {
         //dbg_mw_printf("[DLNA]interface broadcast/any/loopback\n");
         continue;
      }
	

      if(build_etheraddr(ifr, &etheraddr) != 0)
      {
         ret = -1;
         continue;
      }

      if(build_arp_req_packet(ifr, (unsigned long)inet_addr(in_addr), &arp_req_packet) != 0)
      {
         ret = -2;
         continue;
      }

      internal_ret = exe_arp_request(&etheraddr, arp_req_packet, arp_rsp_packet);
      if(internal_ret == -4)
      {
         ret = -3;
         break;
      }

      if(get_src_mac_from_arp_packet(arp_rsp_packet, out_mac) != 0)
      {
         ret = -4;
         continue;
      }
      else
      {
         break;
      }
   }


PGM_END:
   //dbg_mw_printf("[DLNA]%s ret code:%d\n", __FUNCTION__, ret);
   if(conf)
   {
      free_ifconfig(conf);
   }

   return ret;
}

INT32 c_net_iptomac (CHAR* in_addr, CHAR* out_mac)
{
    /* Get MAC address form IP use ARP */
    /* TODO: implement linux version here */
   int sockfd = 0;
   struct sockaddr_ll my_etheraddr;
   unsigned char *pktbuf;
   unsigned char ackbuf[80];
   unsigned char my_mac[7];
   unsigned long my_ip;
   int           my_ifindex;
   int           nfound;
   struct timeval time_out;
   fd_set readset,writeset;
   struct arp_packet *arp_packet;

   unsigned long dst_ip;
   unsigned char ip_addr[5];

   int ret = 0;

   if(!in_addr)
   {
      //dbg_mw_printf("[DLNA]in_addr is nil\n");
      return -1;
   }

   if(!out_mac)
   {
      //dbg_mw_printf("[DLNA] out mac is null\n");
      return -1;
   }

   ret = c_net_ni_get_mac("eth0", (CHAR*)my_mac);
   if(ret != 0)
   {
      //printf("[DLNA]get mac error\n");
      ret = -8;
      goto PGM_END;
   }

   ret = c_net_ni_get_ip("eth0", &my_ip);
   if(ret != 0)
   {
      //printf("[DLNA]get ip error\n");
      ret = -7;
      goto PGM_END;
   }

   memset(&my_etheraddr, 0, sizeof(struct sockaddr_ll));
   my_etheraddr.sll_family = AF_PACKET;
   my_etheraddr.sll_protocol = htons(ETH_P_ARP);
   ret = c_net_get_ifindex("eth0", &my_ifindex);
   if(ret != 0)
   {
      //printf("[DLNA]get ifindex error\n");
      ret = -6;
      goto PGM_END;
   }
   my_etheraddr.sll_ifindex = my_ifindex;

   sockfd = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_ARP));
   if(sockfd < 0)
   {
      //printf("[DLNA]create socket error\n");
      ret = -5;
      goto PGM_END;
   }

   if(bind(sockfd, (struct sockaddr*)&my_etheraddr, sizeof(struct sockaddr_ll)) != 0)
   {
      //printf("[DLNA]bind socket error\n");
      ret = -4;
      goto PGM_END;
   }

   pktbuf = BuildArpRequestPacket(my_mac, my_ip, inet_addr((const char*)in_addr));

   //printf("[DLNA]send Arp pcket:\n");
//   print_data(pktbuf);
  // print_ARPMsg((struct arp_packet*)pktbuf);

   if(sendto(sockfd, pktbuf, 80, 0, (struct sockaddr*)&my_etheraddr, sizeof(struct sockaddr_ll)) <= 0)
   {
      //printf("[DLNA]send packet error\n");
      //printf("[DLNA]error no:%d error str:%s\n", errno, strerror(errno));
      ret = -3;
      goto PGM_END;
   }

   time_out.tv_sec = 0;
   time_out.tv_usec = 500000; //500ms

   while(1)
   {
      FD_ZERO(&readset);
      FD_ZERO(&writeset);
      FD_SET(sockfd, &readset);

      nfound = select(sockfd+1, &readset, &writeset, NULL, &time_out);
      if(nfound == 0)
      {
         //printf("[DLNA]select error, time out,err code:%d\n", errno);
         ret = -1;
         goto PGM_END;
      }
      else if(nfound < 0)
      {
         //printf("[DLNA]select error, err code:%d\n", nfound);
         ret = -2;
         goto PGM_END;
      }

      if(FD_ISSET(sockfd, &readset))
      {
         memset(ackbuf, 0, 80);
         recvfrom(sockfd, ackbuf, 80, 0, NULL, 0);
         //printf("[DLNA]recv arp packet\n");
         //print_data(ackbuf);
         //print_ARPMsg((struct arp_packet*)ackbuf);
         arp_packet = (struct arp_packet*)ackbuf;
         if(ntohs(arp_packet->arphd.ar_op) != 2) //if not arp respond
         {
            //dbg_mw_printf("[DLNA]arp op is not respond\n");
            continue;
         }

         dst_ip = inet_addr((const char*)in_addr);
         memset(ip_addr, 0, 5);
         memcpy(ip_addr, &dst_ip, 4);
         //dbg_mw_printf("[DLNA]dst ip:%s\n", in_addr);
         //dbg_mw_printf("[DLNA]dst_ip:[%x %x %x %x]\n", ip_addr[0], ip_addr[1], ip_addr[2], ip_addr[3]);
         //dbg_mw_printf("[DLNA]rsp_ip:[%x %x %x %x]\n", arp_packet->inet_src_addr[0], arp_packet->inet_src_addr[1], arp_packet->inet_src_addr[2], arp_packet->inet_src_addr[3]);
         if(memcmp(ip_addr, arp_packet->inet_src_addr, 4) != 0)
         {
            //dbg_mw_printf("[DLNA]dst ip isn't match inet_src_addr.\n");
            continue;
         }
         else
         {
            //dbg_mw_printf("[DLNA]dst ip match inet_src_addr.\n");
         memcpy(out_mac, arp_packet->eth_src_addr, 6);
         break;
      }
      }
      else
      {
      //printf("[DLNA]cannt get arp respond\n");
   }
   }

PGM_END:
   if(sockfd > 0)
   {
      close(sockfd);
   }

    return ret;
}
/*
INT32 c_net_module_load(VOID)
{
    CHAR ps_cmd[64];
    INT32 i4_sys_ret;
    strncpy(ps_cmd, "af_packet.ko", 64);

    i4_sys_ret = insert_kernel_module(NULL, ps_cmd, "", 0);

    if (i4_sys_ret == 0)
    {
        //dbg_mw_printf("insmod af_packet.ko ok\n");
    }
    else
    {
        dbg_mw_printf("insmod af_packet.ko failed\n");
        return NET_FAILED;
    }

    return 0;
}
*/
INT32 c_net_get_wifi_ni_name(CHAR *psz_ni_name)
{
    INT32 sock = 0;
    INT32 ret = 0;
    struct ifreq ifr;

    memset(&ifr, 0, sizeof(struct ifreq));

    dbg_mw_printf(" C_MT_LX_NET_CFG:>>> c_net_get_wifi_ni_name() is called\n");

    sock = socket(AF_INET, SOCK_DGRAM, 0);
    if(sock < 0) {
        dbg_mw_printf("<%s> Fail to create socket!\n", __FUNCTION__);
        return NET_FAILED;
    }

    /* first check ath0 */
	strncpy(ifr.ifr_name, "ath0", 5);
    ret = ioctl(sock, SIOCGIFFLAGS, &ifr);
    if (!ret)
    {
        //return ath0
        strncpy(psz_ni_name, "ath0", 5);
    }
    else
    {
        //test ra0
        strncpy(ifr.ifr_name, "ra0", 4);
        ret = ioctl(sock, SIOCGIFFLAGS, &ifr);
        if (!ret)
        {
            strncpy(psz_ni_name, "ra0", 4);
        }
        else
        {
            strncpy(ifr.ifr_name, "rea0", 4);
            ret = ioctl(sock, SIOCGIFFLAGS, &ifr); //For real dongle.
            if (!ret)
            {
                strncpy(psz_ni_name, "rea0", 4);
            }
            else
            {
                strncpy(ifr.ifr_name, "mlan0", 6);
                ret = ioctl(sock, SIOCGIFFLAGS, &ifr); //For marvell wifi.
                if (!ret)
                {
                    strncpy(psz_ni_name, "mlan0", 6);
                }
                else
                {
                    close(sock);
                    return NET_FAILED;
                }
            }
        }
    }

    close(sock);
    return NET_OK;
}
/*
INT32 c_net_module_unload(VOID)
{
    CHAR ps_cmd[64];
    INT32 i4_sys_ret;
    strncpy(ps_cmd, "af_packet", 64);

    i4_sys_ret = remove_kernel_module(ps_cmd,0);

    if (i4_sys_ret == 0)
    {
        dbg_mw_printf("remove_kernel_module af_packet.ko ok\n");
    }
    else
    {
        dbg_mw_printf("remove_kernel_module af_packet.ko failed\n");
        return NET_FAILED;
    }

    return 0;
}
*/

INT32 c_net_network_init(INT32 i4IfType)
{
    g_i4CurIfType = i4IfType;

    // WLAN
#ifdef CONFIG_MW_NET_WIFI
    if (IF_TYPE_IEEE80211 == i4IfType)
    {
        x_net_wlan_start(&rDev);
    }
    else
    {
        x_net_wlan_stop(&rDev);
    }
#endif
    {
     //   extern BOOL x_cpsm_imtk_pd_chk(VOID);  // audio project

    //    x_cpsm_imtk_pd_chk();
        //dbg_mw_printf("network_init for imtk\n"); // audio project

    }

#if MW_SUPPORT_ADDON_HOA
    //HOA_ADDON_Notification
    //SUB_AOI_ADDONHOST_SendEventToApp(SUB_HOST_EVT_NETWORK_INET_ENABLED, 0);
    if (SUB_AOI_ADDONHOST_SendEventToApp_mw_fn != NULL)
    {
        dbg_mw_printf("[HOA]Send SUB_HOST_EVT_NETWORK_INET_ENABLED event.\n");
        (* SUB_AOI_ADDONHOST_SendEventToApp_mw_fn) (SUB_HOST_EVT_NETWORK_INET_ENABLED, 0);
    }
#endif

    return 0;
}

INT32 c_net_network_deinit(VOID)
{
    // WLAN
#ifdef CONFIG_MW_NET_WIFI
    if (IF_TYPE_IEEE80211 == g_i4CurIfType)
    {
      x_net_wlan_stop(&rDev);
      x_net_wlan_deinit();
    }
#endif
    g_i4CurIfType = IF_TYPE_OTHER;

#if MW_SUPPORT_ADDON_HOA
    //HOA_ADDON_Notification
    //SUB_AOI_ADDONHOST_SendEventToApp(SUB_HOST_EVT_NETWORK_INET_DISABLED, 0);
    if (SUB_AOI_ADDONHOST_SendEventToApp_mw_fn != NULL)
    {
        dbg_mw_printf("[HOA]Send SUB_HOST_EVT_NETWORK_INET_DISABLED event.\n");
        (* SUB_AOI_ADDONHOST_SendEventToApp_mw_fn) (SUB_HOST_EVT_NETWORK_INET_DISABLED, 0);
    }
#endif

    return 0;
}

VOID c_net_set_hostname(CHAR *ps_hostname)
{
    CHAR cmd[net_cmd_len];
    dbg_mw_printf("<c_net_set_hostname>ps_hostname: %s\n", ps_hostname);

    if (ps_hostname != NULL)
    {
        strncpy(s_hostname, ps_hostname, 64);
        sethostname(ps_hostname, strlen(ps_hostname));
        snprintf(cmd, net_cmd_len, "echo \"%s\" > /etc/hostname", ps_hostname);
        dbg_mw_printf("<c_net_set_hostname>CMD: %s\n", cmd);
        system(cmd);
    }
}

INT32 parse_ni_statistics(FILE * file, const CHAR * ifname, INT64 * long_rx, INT64 * long_tx)
{
    CHAR buf[100];
    INT32 bytes_read;
    INT32 i;
    CHAR *rx, *tx;
    INT32 skip = 0;

    bytes_read = fscanf(file, "%99s", buf);

    while ( -1 !=  bytes_read)
    {
        if ( 0 == strncmp(ifname, buf, strlen(ifname)))
        {
            //dbg_mw_printf("read string: %s\n", buf);
            if (strlen(buf) > (strlen(ifname) + 1))
            {
                for (i = 0; i<100 ; i++)
                {
                    if(':' == buf[i])
                    {
                        skip = 8;
                        rx = &buf[i+1];
                        //dbg_mw_printf("(1)Recieve bytes: %s, idx= %d\n", rx, i);
                        *long_rx = strtol(rx, NULL, 10);
                        break;
                    }
                }
            }else{
                bytes_read = fscanf(file, "%99s", buf);
                rx = buf;
                //dbg_mw_printf("(2)Recieve bytes: %s\n", rx);
                *long_rx = strtol(rx, NULL, 10);
            }

            for (i = 0; i < 8; i++)
                bytes_read = fscanf(file, "%99s", buf);

            tx = buf;
            //dbg_mw_printf("Transmit bytes: %s\n", tx);
            *long_tx = strtol(tx, NULL, 10);
        }

        bytes_read = fscanf(file, "%99s", buf);
    }

    return NET_OK;
}


VOID c_net_get_ni_speed_thread(VOID * req)
{
    FILE *file = NULL;
    INT64 t1_rx = 0, t1_tx = 0, t2_rx = 0, t2_tx = 0;
    struct timespec t1, t2;  //CLOCK_MONOTONIC
    DOUBLE micro_sec;
    SPEED_REQUEST_T * request = (SPEED_REQUEST_T *)(*(int *)req);

    clock_gettime(CLOCK_MONOTONIC, &t1);
    file = fopen("/proc/net/dev", "r");
    if (NULL == file)
    {
        dbg_mw_printf("/proc/net/dev open fail\n");
        free(request);
        u_thread_exit();
    }
    parse_ni_statistics(file, request->if_name, &t1_rx, &t1_tx);
    fclose(file);

    sleep(2);

    clock_gettime(CLOCK_MONOTONIC, &t2);
    file = fopen("/proc/net/dev", "r");
    if (NULL == file)
    {
        dbg_mw_printf("/proc/net/dev open fail\n");
        free(request);
        u_thread_exit();
    }
    parse_ni_statistics(file, request->if_name, &t2_rx, &t2_tx);
    fclose(file);

    micro_sec = (t2.tv_sec - t1.tv_sec) + (t2.tv_nsec - t1.tv_nsec) /1000000000;
    //dbg_mw_printf("sleep %f seconds\n", micro_sec);

    //dbg_mw_printf("t1_rx= %d, t2_rx= %d \n", t1_rx, t2_rx);
    net_speed.rx_speed = (t2_rx - t1_rx) / micro_sec;
    net_speed.tx_speed = (t2_tx - t1_tx) / micro_sec;

    /* For debug the ocassionally minus value*/
    if (0 > net_speed.rx_speed || 0 > net_speed.tx_speed)
    {
        dbg_mw_printf("Minus speed value detected\n");
        dbg_mw_printf("t2_rx= %d, t1_rx= %d, micro_sec= %d\n", t2_rx, t1_rx, micro_sec);
        dbg_mw_printf("t2_tx= %d, t1_tx= %d, micro_sec= %d\n", t2_tx, t1_tx, micro_sec);
        system("cat /proc/net/dev");
    }


    request->cb_fp(&net_speed);

    fg_ni_speed_thread_running = FALSE;
    free(request);
    u_thread_exit();

}

INT32 c_net_get_ni_speed(const CHAR * ifname, get_ni_speed_cb_fp cb_fp)
{
    INT32 i4_status;
    SPEED_REQUEST_T * request = NULL;

    if (!fg_ni_speed_thread_running)
    {
        //start get ni speed thread
        fg_ni_speed_thread_running = TRUE;

        request = (SPEED_REQUEST_T *)malloc(sizeof(SPEED_REQUEST_T));
        if (NULL == request)
        {
            dbg_mw_printf("<%s>:%d malloc memory failed\n", __FUNCTION__, __LINE__);
            return NET_FAILED;
        }
        strncpy(request->if_name, ifname, strlen(ifname));
        request->if_name[strlen(ifname)] = 0;
        request->cb_fp = cb_fp;

        i4_status = u_thread_create (&h_ni_speed_thread,
                              "NISPEED",
                              8*1024,
                              NI_MONITOR_THREAD_PRIORITY,
                              c_net_get_ni_speed_thread,
                              sizeof(VOID *),
                              (VOID *)&request);

          if (i4_status != OSR_OK)
          {
              fg_ni_speed_thread_running = FALSE;
              /* Bad news, some other error. Well, simply return a NULL_HANDLE. */
              dbg_mw_printf("Create get ni speed thread failed, ret = %d\n", (int)i4_status);
              free(request);
              return NET_FAILED;
          }

    }else{
        //give result of last time
        (*cb_fp)(&net_speed);
    }

    return NET_OK;
}


static VOID net_cmd_thread(VOID *cmd)
{
      NET_CMD_T *net_cmd = (NET_CMD_T *)cmd;
      INT32 i4_net_ret;
      dbg_mw_printf("net_cmd = %s\n", net_cmd->s_cmd);

      i4_net_ret = system(net_cmd->s_cmd);

      if (i4_net_ret == 0)
      {
          if (net_cmd->eCmdType == NET_ARPING)
          {
            dbg_mw_printf("IP isn't conflicted.\n");
            if (net_cmd->pf_conflict_notify != NULL)
            {
              net_cmd->pf_conflict_notify(FALSE);
            }
          }
      }
      else
      {
          if (net_cmd->eCmdType == NET_ARPING)
          {
            CHAR s_cmd[net_cmd_len];

            snprintf(s_cmd, net_cmd_len, "%s | grep reply | awk '{print $5}' > /var/ipconflict", net_cmd->s_cmd);
            //dbg_mw_printf("net_cmd = %s\n", s_cmd);
            i4_net_ret = system(s_cmd);

            if (i4_net_ret == 0)
            {
              dbg_mw_printf("conflict MAC is output.\n");
            }
            else
            {
              dbg_mw_printf("conflict MAC is not output.\n");
            }

            dbg_mw_printf("IP is conflicted.\n");
            if (net_cmd->pf_conflict_notify != NULL)
            {
              net_cmd->pf_conflict_notify(TRUE);
            }
          }
      }

      fg_net_cmd_thread_running = FALSE;
      u_thread_exit();
}

INT32 c_net_ip_conflict(CHAR *psz_name, UINT32 ui4_address, VOID (*notify_fn) (BOOL fgConflict))
{
      struct in_addr t_addr;
      NET_CMD_T net_cmd = {{0}};
      INT32          i4_status;

      t_addr.s_addr = ui4_address;
      snprintf(net_cmd.s_cmd, net_cmd_len, "arping -D %s -w 5 -U -I %s", (char *)inet_ntoa(t_addr), psz_name);
      net_cmd.pf_conflict_notify = notify_fn;
      net_cmd.eCmdType = NET_ARPING;

      if (!fg_net_cmd_thread_running)
      {
          fg_net_cmd_thread_running = TRUE;
          // Create the polling thread first
          i4_status = u_thread_create (&h_net_cmd_thread,
                              "NETCMDD",
                              1024,
                              NI_MONITOR_THREAD_PRIORITY,
                              net_cmd_thread,
                              sizeof(net_cmd),
                              (VOID *)&net_cmd);

          if (i4_status != OSR_OK)
          {
              fg_net_cmd_thread_running = FALSE;
              /* Bad news, some other error. Well, simply return a NULL_HANDLE. */
              dbg_mw_printf("Create net cmd thread failed, ret = %d\n", (int)i4_status);
              return NET_FAILED;
          }
      }

      return NET_OK;

}

INT32 hex2int(CHAR c)
{
	if (c >= '0' && c <= '9')
		return c - '0';
	if (c >= 'a' && c <= 'f')
		return c - 'a' + 10;
	if (c >= 'A' && c <= 'F')
		return c - 'A' + 10;
	return -1;
}


INT32 arp_search_mac_by_ip( UINT32 ui4_address, UCHAR *puac_mac)
{
      struct in_addr t_addr;
      NET_CMD_T net_cmd = {{0}};
      INT32          i4_status;
      CHAR *mac_addr = "/var/ipconflict";
      CHAR buff[20] = {0};
      BOOL start_cp = FALSE;
      UCHAR *mac_ptr = puac_mac;
      FILE * fd = NULL;
      UINT8 idx = 0;
      CHAR mac[2] = {0};

      t_addr.s_addr = ui4_address;
      snprintf(net_cmd.s_cmd, net_cmd_len, "arping -D %s -w 5 -U", (char *)inet_ntoa(t_addr));
      net_cmd.pf_conflict_notify = NULL;
      net_cmd.eCmdType = NET_ARPING;

      i4_status = system(net_cmd.s_cmd);

      if (i4_status == 0)
      {
          if (net_cmd.eCmdType == NET_ARPING)
          {
            dbg_mw_printf("No ARP replied.\n");
            return NET_FAILED;
          }
      }
      else
      {
          if (net_cmd.eCmdType == NET_ARPING)
          {
            CHAR s_cmd[net_cmd_len];

            snprintf(s_cmd, net_cmd_len, "%s | grep reply | awk '{print $5}' > /var/ipconflict", net_cmd.s_cmd);

            i4_status = system(s_cmd);

            if (0 == i4_status)
            {
                fd = fopen(mac_addr, "rt");
                if (fd == NULL)
                {
                  return NET_FAILED;
                }

                fscanf(fd, "%19s", buff);

                for(i4_status = 0; i4_status < 20; i4_status++)
                {
                  if('[' == buff[i4_status])
                  {
                      start_cp = TRUE;
                      continue;
                  }
                  if(']' == buff[i4_status])
                  {
                     if(1 == idx)
                     {
                        *mac_ptr++ = hex2int(mac[0]);
                     }
                     else // idx = 2
                     {
                        *mac_ptr++ = hex2int(mac[0]) * 16 + hex2int(mac[1]);
                     }
                     //*mac_ptr = '\0';
                     break;
                  }
                  if (':' == buff[i4_status])
                  {
                     if(1 == idx)
                     {
                        *mac_ptr++ = hex2int(mac[0]);
                     }
                     else // idx = 2
                     {
                        *mac_ptr++ = hex2int(mac[0]) * 16 + hex2int(mac[1]);
                     }
                     idx = 0;
                     continue;
                  }

                  if(start_cp)
                  {
                    //if something goes wrong, return fail
                    if(idx > 1)
                    {
                        fclose(fd);
                        return NET_FAILED;
                    }

                    mac[idx++] = buff[i4_status];
                  }

                }
                fclose(fd);
            }
          }
      }

      if (FALSE == start_cp)
        return NET_FAILED;

      return NET_OK;

}


INT32 c_net_get_mac_by_ip (const CHAR *ps_ip, UCHAR *puac_mac)
{
    UINT32 ui4_ip;
    INT32 i4_same_subnet = 0;
    MT_DHCP4_INFO_T pt_info = {0};

#ifdef CONFIG_MW_NET_WIFI
    INT32 sockfd;
    struct ifreq ifr;
#endif

    if (ps_ip == NULL || puac_mac == NULL)
    {
        return NET_INV_ARG;
    }

    ui4_ip = inet_addr (ps_ip);
    if (!ui4_ip)
    {
        return NET_INV_ARG;
    }

    /* if is our self */
    c_dhcpc_get_info("eth0", &pt_info);

    if (pt_info.ui4_ipaddr == ui4_ip)
    {

#ifdef CONFIG_MW_NET_WIFI
        sockfd = socket(AF_INET, SOCK_DGRAM, 0);
        bzero(&ifr, sizeof(ifr));
        strncpy(ifr.ifr_name, "eth0", 5);

        if (ioctl(sockfd, SIOCGIFADDR, &ifr)<0 || ((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr.s_addr == 0)
        {
            dbg_mw_printf("Get wifi mac\n");

            c_net_wlan_get_mac_addr((UINT8 *)puac_mac);
        }
        else
        {
            dbg_mw_printf("Get ethernet mac\n");
            c_net_get_mac_from_sys((CHAR *)puac_mac);
        }

        close (sockfd);
#else
        dbg_mw_printf("Get ethernet mac\n");
        c_net_get_mac_from_sys((CHAR *)puac_mac);
#endif

        return NET_OK;
    }

    /* check same subnet */
    {
        UINT32 ui4_mask = pt_info.ui4_subnet;

        if ((i4_same_subnet == 0) &&
            ((pt_info.ui4_ipaddr & ui4_mask) == (ui4_ip & ui4_mask)))
        {
            i4_same_subnet = 1;
        }
    }

    /* from arp table */
    if (arp_search_mac_by_ip(ui4_ip, puac_mac) == 0)
    {
        return NET_OK;
    }

    if (i4_same_subnet == 1)
    {
        return NET_FAILED; /* not in ARP table */
    }

    return -1; /* not same subnet */
}


/**
* Configure IP address of a network interface according to it's MAC address.
*
* @param psz_name name of network interface.
* @param pc_mac   mac address.
*
* @return NET_OK      configure success.
*         NET_FAILED  configure fail
*/
INT32 c_net_autoip_config(CHAR *psz_name, CHAR *pc_mac)
{
      struct in_addr t_addr;
      CHAR s_ip[ip_addr_len];
      CHAR s_mask[ip_addr_len];
      CHAR s_gw[ip_addr_len];

#if (FORK2NETINFD)
      CHAR s_ifconf[ip_addr_len * 4];
#endif
      unsigned int randv;
      int addr;

      randv = (*(pc_mac+2) << 24) + (*(pc_mac+3) << 16) + (*(pc_mac+4) << 8) + *(pc_mac+5);
      randv += rand();

      randv = randv % 0xFE00;  /* 169.254.0.1 to 169.254.254.255 */
      printf("autoip randv: 0x%x\n", randv);
      randv += 0xA9FE0001;     /* 169.254.0.1 */

      addr = htonl(randv);

      //ifconfig eth0 s_ip
      t_addr.s_addr = addr;
      snprintf(s_ip, ip_addr_len, "%s", (char *)inet_ntoa(t_addr));

      //ifconfig eth0 netmask s_mask
      randv = 0xFFFF0000;     /* 255.255.0.0 */
      t_addr.s_addr = htonl(randv);
      snprintf(s_mask, ip_addr_len, "%s", (char *)inet_ntoa(t_addr));

      //route add default gw s_gw
      randv = 0x00000000;     /* 0.0.0.0 */
      t_addr.s_addr = htonl(randv);
      snprintf(s_gw, ip_addr_len, "%s", (char *)inet_ntoa(t_addr));

#if (FORK2NETINFD)
      snprintf(s_ifconf, ip_addr_len * 4, "%s %s %s 1", s_ip, s_mask, s_gw);

      if (netinfd_ifconf(psz_name, (char *)s_ifconf) < 0)
      {
        return NET_FAILED;
      }
      else
      {
        //CRIT_STATE_T   t_crit;
        //t_crit = u_crit_start();
        NI_LX_MONITOR_LIST_T *pt_new_item = pt_lx_ni_monitor_list_root;

        /* search current list for this new item */

        while (pt_new_item)
        {
            if (pt_new_item->e_event == NI_DRV_EV_IP_ADDRESS_CHANGED &&
                (strncmp(pt_new_item->sz_NifName, psz_name,NIFNAMELEN) == 0) &&
                pt_new_item->pf_notify != NULL)
            {
                pt_new_item->fgNeedNotify = TRUE;
                //u_crit_end(t_crit);
                break;
            }
            pt_new_item = pt_new_item->pt_next;
        }
        //u_crit_end(t_crit);
      }

      return NET_OK;
#else
    char *argp[8];
    int pid, wpid, wstatus;
    int rc = -1;

    argp[0] = ifscript;
    argp[1] = (char *)"ip_mask";
    argp[2] = psz_name;
    argp[3] = (char *)s_ip;
    argp[4] = (char *)s_mask;
    argp[5] = (char *)"gw";
    argp[6] = (char *)s_gw;
    argp[7] = NULL;
    printf("ip: %s, mask: %s, gw: %s\n", argp[3], argp[4], argp[6]);

     
     pid = fork();
 
     if (pid < 0) {
         NET_LOG("fork() error: %d, %s\n", errno, strerror(errno));
         return -1;
     } else if (pid == 0) {
         /* child */
         execvp(argp[0], argp);
         NET_LOG("execvp() error: %d, %s\n", errno, strerror(errno));
         exit(-1);
     }
 
     do {
         /* parent */
         waitpid(pid, &rc, 0);
 
         if (WIFEXITED(rc)) {
             rc = WEXITSTATUS(rc);
             break;
         } else if (WIFSIGNALED(rc)) {
             rc = WTERMSIG(rc);
             NET_LOG("proc %d term by signal: %d, %s\n", pid, rc, strsignal(rc));
             break;
         } else if (WIFSTOPPED(rc)) {
             rc = WSTOPSIG(rc);
             NET_LOG("proc %d stop by signal: %d, %s\n", pid, rc, strsignal(rc));
             continue;
         } else if (WIFCONTINUED(rc)) {
             NET_LOG("proc %d continued\n", pid);
             continue;
         } else {
             NET_LOG("proc %d unknown status: 0x%08x\n", pid, rc);
             break;
         }
 
     } while (1);
   
  return rc;
#endif
}


/**
* Configure network interface with specific ip addressm netmask and gateway.
*
* @param psz_name      name of network interface.
* @param ui4_address   IPv4 address.
* @param ui4_netmask   Netmask.
* @param ui4_gw        Default gateway.
*
* @return NET_OK      configure success.
*         NET_FAILED  configure fail
*/
INT32 c_net_ip_config(CHAR *psz_name, UINT32 ui4_address, UINT32 ui4_netmask, UINT32 ui4_gw)
{
    struct in_addr t_addr;
    CHAR s_ip[ip_addr_len];
    CHAR s_mask[ip_addr_len];
    CHAR s_gw[ip_addr_len];
    char *argp [8];
#if (!FORK2NETINFD)
    int pid, wpid, wstatus;
    int ret = 0;
#endif
    char *script = "/sbin/ifconfig.script";
#if (FORK2NETINFD)
    CHAR s_ifconf[ip_addr_len * 4];
#endif

    argp [0] = script;

    //ifconfig eth0 s_ip
    t_addr.s_addr = ui4_address;
    snprintf(s_ip, ip_addr_len, "%s", (char *)inet_ntoa(t_addr));

    //ifconfig eth0 netmask s_mask
    t_addr.s_addr = ui4_netmask;
    snprintf(s_mask, ip_addr_len, "%s", (char *)inet_ntoa(t_addr));

    //route add default gw s_gw
    t_addr.s_addr = ui4_gw;
    snprintf(s_gw, ip_addr_len, "%s", (char *)inet_ntoa(t_addr));
    printf("[NW] c_net_ip_config s_ip[%s]s_mask[%s]s_gw[%s]psz_name[%s]!\n", s_ip, s_mask, s_gw, psz_name);

    argp [1] = (char *)"ip_mask";
    argp [2] = (char *)psz_name;
    argp [3] = (char *)s_ip;
    argp [4] = (char *)s_mask;
    argp [5] = (char *)"gw";
    argp [6] = (char *)s_gw;
    argp [7] = NULL;
#if (FORK2NETINFD)
    snprintf(s_ifconf, ip_addr_len * 4, "%s %s %s 0", s_ip, s_mask, s_gw);

    if (netinfd_ifconf(argp [2], (char *)s_ifconf) < 0)
    {
      return NET_FAILED;
    }
    else
    {
      //CRIT_STATE_T   t_crit;
      //t_crit = u_crit_start();
      NI_LX_MONITOR_LIST_T *pt_new_item = pt_lx_ni_monitor_list_root;

      /* search current list for this new item */

      while (pt_new_item)
      {
          if (pt_new_item->e_event == NI_DRV_EV_IP_ADDRESS_CHANGED &&
              (strncmp(pt_new_item->sz_NifName, psz_name,NIFNAMELEN) == 0) &&
              pt_new_item->pf_notify != NULL)
          {
              pt_new_item->fgNeedNotify = TRUE;
              //u_crit_end(t_crit);
              break;
          }
          pt_new_item = pt_new_item->pt_next;
      }
      //u_crit_end(t_crit);
    }
#else
    pid = fork ();
    if (pid < 0)
    {
      dbg_mw_printf ("fork: %d\n", errno);
      wstatus = 0;
    }
    else if (pid)
    { //parent
      do {
        wpid = wait (&wstatus);
      } while (wpid != pid && wpid > 0);
      dbg_mw_printf ("child %d terminated\n", wpid);
      dbg_mw_printf ("child status : %d\n", WEXITSTATUS(wstatus));

      if (wpid < 0)
      {
        dbg_mw_printf("wait: %d, err: %s\n", wpid, strerror(errno));
        wstatus = 0;
      }
      if (wstatus != 0)
      {
        return NET_FAILED;
      }

      {
        CRIT_STATE_T   t_crit;
        t_crit = u_crit_start();
        NI_LX_MONITOR_LIST_T *pt_new_item = pt_lx_ni_monitor_list_root;

        /* search current list for this new item */

        while (pt_new_item)
        {
            if (pt_new_item->e_event == NI_DRV_EV_IP_ADDRESS_CHANGED &&
                (strcmp(pt_new_item->sz_NifName, psz_name) == 0) &&
                pt_new_item->pf_notify != NULL)
            {
                pt_new_item->fgNeedNotify = TRUE;
                u_crit_end(t_crit);
                break;
            }
            pt_new_item = pt_new_item->pt_next;
        }
        u_crit_end(t_crit);

      }
    }
    else
    { // child
      ret = execv (script, argp);
      dbg_mw_printf ("execve result = %d\n", ret);
      if (ret == -1)
      {
        dbg_mw_printf("execve errno = %s\n", strerror(errno));
      }
      exit (0);
    }
#endif

    return NET_OK;
}


/**
* Set DNS server for network system
*
* @param ui4_dns1     IP address of first DNS server.
* @param ui4_dns2     IP address of second DNS server.
*
* @return NET_OK      configure success.
*         NET_FAILED  configure fail
*/
INT32 c_net_dns_config(UINT32 ui4_dns1, UINT32 ui4_dns2)
{
    /*
       run script : 1. clean /etc/resolv.conf (cat /dev/null > /etc/resolv.conf)
                    2. update with new info
    */
    struct in_addr t_addr;
    CHAR s_dnsip[2][ip_addr_len] = {{0}};
    int i4dnsNum = 1;
	char cmd[100] = {0};
    FILE * fd;
    char *dhcpinfo = "/var/dhcpinfo";
    char buf[20];

    if (ui4_dns1 != 0 && ui4_dns1 != 0xFFFFFFFF)
    {
      i4dnsNum ++;
      t_addr.s_addr = ui4_dns1;
      snprintf(s_dnsip[0], ip_addr_len, "%s", (char *)inet_ntoa(t_addr));
    }
    if (ui4_dns2 != 0 && ui4_dns2 != 0xFFFFFFFF)
    {
      t_addr.s_addr = ui4_dns2;
      snprintf(s_dnsip[1], ip_addr_len, "%s", (char *)inet_ntoa(t_addr));
      i4dnsNum ++;
    }
	if(3 == i4dnsNum)
	{
		printf("[NW] -----config dns1 && dns2--------!\n");
		snprintf(cmd, 100, "/sbin/dns.script set %s %s", s_dnsip[0], s_dnsip[1]);
	}
	else if(2 == i4dnsNum)
	{
        //dns1 is invalid, copy dns2 to s_dnsip[0]
        if (0 == ui4_dns1 || 0xFFFFFFFF == ui4_dns1)
        {
            strncpy(s_dnsip[0], s_dnsip[1], ip_addr_len);
        }
		printf("[NW] -----config 2 dns--------!\n");
		snprintf(cmd, 100, "/sbin/dns.script set %s", s_dnsip[0]);
	}
	system(cmd);

    /* Update DNS information in /var/dhcpinfo */
    fd = fopen(dhcpinfo, "r+b");
    if (fd == NULL)
      return 0;

    while (EOF != fscanf(fd, "%15s", buf))
    {
      if (strcmp("IP", buf) == 0)
      {
          if (EOF == fscanf(fd, "%15s", buf))
          {
            fclose(fd);
            return NET_FAILED;
          }
          if (inet_aton(buf, &t_addr) == 0)
          {
            dbg_mw_printf ("IP %s is invalid\n", buf);
          }
          dbg_mw_printf("IP: %s\n", buf);
      }
      else if (strcmp("Subnet", buf) == 0)
      {
          if (EOF == fscanf(fd, "%15s", buf))
          {
            fclose(fd);
            return NET_FAILED;
          }
          if (inet_aton(buf, &t_addr) == 0)
          {
            dbg_mw_printf ("Subnet %s is invalid\n", buf);
          }
          dbg_mw_printf("Subnet: %s\n", buf);
      }
      else if (strcmp("router", buf) == 0)
      {
          if (EOF == fscanf(fd, "%15s", buf))
          {
            fclose(fd);
            return NET_FAILED;
          }
          if (inet_aton(buf, &t_addr) == 0)
          {
            dbg_mw_printf ("router %s is invalid\n", buf);
          }
          dbg_mw_printf("router: %s\n", buf);
          break;
      }
      else if (strcmp("DNS", buf) == 0)
      {
          fseek(fd, -3, SEEK_CUR);
          break;
      }
    }

    if (ui4_dns1 != 0 && ui4_dns1 != 0xFFFFFFFF)
    {
      fprintf(fd, "\nDNS %s\n", s_dnsip[0]);
    }
    else
    {
        fprintf(fd, "\nDNS %s\n", "0.0.0.0");
    }
    if (ui4_dns2 != 0 && ui4_dns2 != 0xFFFFFFFF)
    {
      fprintf(fd, "DNS %s\n", s_dnsip[1]);
    }
    else
    {
        fprintf(fd, "\nDNS %s\n", "0.0.0.0");
    }
    fclose (fd);
    return NET_OK;
}


/**
* Resolve server name into IP address.
*
* @param  ps_server_url     The server name to be looked up.
*
* @return NET_OK      Look up success.
*         NET_FAILED  Look up fail.
*/
INT32 c_net_dns_lookup(const NET_ADDR_INFO_LOOK_UP *ptAddrinfoLookup)
{
    struct addrinfo *ptAddrInfo = NULL;
    struct addrinfo *ptCurrAddrinfo = NULL;
    answer = NET_OK;
    
    if (NULL == ptAddrinfoLookup)
    {
        NET_LOG("<dns lookup>invalid parameter!\n");  
        return NET_FAILED;
    }

    if(NULL == ptAddrinfoLookup->ps_server_url)
    {
        NET_LOG("<dns lookup>invalid parameter!\n");  
        return NET_FAILED;
    }
    NET_LOG("<dns lookup>url=%s,eAddrFamily=%d,fg_addrinfo_thread_running=%d!\n", 
        ptAddrinfoLookup->ps_server_url, ptAddrinfoLookup->eAddrFamily,
        fg_addrinfo_thread_running);
    
    ptAddrInfo = c_net_get_addrinfo_by_addrtype(ptAddrinfoLookup);

    if (ptAddrInfo)
    {
        if (ptAddrinfoLookup->eAddrFamily == NET_AF_IPV4)
        {
            char ip4[INET_ADDRSTRLEN] = {0};
            for (ptCurrAddrinfo = ptAddrInfo; ptCurrAddrinfo != NULL; ptCurrAddrinfo = ptCurrAddrinfo->ai_next) 
            {
                inet_ntop(AF_INET,&(((struct sockaddr_in *)(ptCurrAddrinfo->ai_addr))->sin_addr),ip4, sizeof(ip4));
                NET_LOG("<dns lookup>ipv4 add is-%s\n", ip4);                
            }
        }
        else if (ptAddrinfoLookup->eAddrFamily == NET_AF_IPV6)
        {
            char ip6[INET6_ADDRSTRLEN] = {0};
            for (ptCurrAddrinfo = ptAddrInfo; ptCurrAddrinfo != NULL; ptCurrAddrinfo = ptCurrAddrinfo->ai_next) 
            {
                inet_ntop(AF_INET6,&(((struct sockaddr_in *)(ptCurrAddrinfo->ai_addr))->sin_addr),ip6, sizeof(ip6));
                NET_LOG("<dns lookup>ip6 add is-%s\n", ip6); 
            }
        }
    }
    else{
        NET_LOG("<get addr info>DNS lookup Name unknown!\n");        
        return NET_FAILED;
    }
    if (!fg_addrinfo_thread_running)
    {
        if(NULL != answer)
        {
            freeaddrinfo(answer);
            answer = NULL;
        }
        ptAddrInfo = NULL;
    }
    
    return NET_OK;   
}



INT32 timed_tcp_test(const CHAR *ps_server_url, UINT16 ui2_port, UINT16 timeout)
{
    int sock_fd, so_error;
    struct sockaddr_in dst;
    struct in_addr ip_addr = {0};
    struct hostent *hp;
    struct timeval tv;
    fd_set writefds;
    socklen_t len = sizeof so_error;

    sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (sock_fd <0)
    {
        dbg_mw_printf("create socket fail !");
        return NET_FAILED;
    }

    memset(&dst,0,sizeof(dst));

    if (inet_pton(AF_INET, ps_server_url, &dst.sin_addr) == 1)
    {
      // URL: Digit IP
      dbg_mw_printf("IP = %s\n", inet_ntoa(dst.sin_addr));
    }
    else
    {
      // URL: Domain name
      hp = c_net_gethostbyname(ps_server_url);
      if (hp)
      {
          ip_addr = *(struct in_addr*)(hp->h_addr_list[0]);
          dbg_mw_printf("IP = %s\n", inet_ntoa(ip_addr));
      }
      else
      {
          dbg_mw_printf("gethostbyname failed\n");
          close (sock_fd);
          return NET_FAILED;
      }
      dst.sin_addr.s_addr = ip_addr.s_addr;
    }

    dst.sin_family = AF_INET;
    dst.sin_port = htons(ui2_port);

    fcntl(sock_fd, F_SETFL, O_NONBLOCK);
    connect(sock_fd, (struct sockaddr *) &dst, (socklen_t)sizeof(dst));

    tv.tv_sec  = timeout;
    tv.tv_usec = 0;

    FD_ZERO(&writefds);
    FD_SET(sock_fd, &writefds);

    if(select(sock_fd + 1, NULL, &writefds, NULL, &tv))
    {

        getsockopt(sock_fd, SOL_SOCKET, SO_ERROR, &so_error, &len);

        if(0 == so_error)
        {
            dbg_mw_printf("connect success\n");
        }
        else
        {
            dbg_mw_printf("connect fail, error code: %s\n", strerror(so_error));
            close (sock_fd);
            return NET_FAILED;
        }
    }
    else
    {
        dbg_mw_printf("connect timeout!\n");
        close (sock_fd);
        return NET_FAILED;
    }

    close (sock_fd);
    return NET_OK;

}


INT32 c_net_test_connection(const CHAR *ps_server_url, UINT16 ui2_port, UINT16 ui2_timeout)
{
    INT32 i4_ret;
    #if !CONFIG_MW_CUSTOM_KLG    
    //Test specific url
    i4_ret = timed_tcp_test(ps_server_url, ui2_port, ui2_timeout);
    if (NET_OK == i4_ret)
    	return NET_OK;
    else
    	  return c_net_test_connection_v6(ps_server_url, ui2_port, ui2_timeout);
#else
    CHAR timeout[4];
    int testCount = 3;
    snprintf(timeout, 4, "%3d", ui2_timeout/2);

    //Test if DNS exists

    while(testCount--)
    {//try 3 times, if still fail, then fail.
    	if (!netinfd_dnslookup((char *)ps_server_url, timeout))
    	{
        	break;
    	}
    	printf("<MW>Gethostbyname fail in 5 secs! time: %d\n", testCount);
    	if(0 >= testCount)
    		return NET_FAILED;
    }

    //Test specific url
    i4_ret = timed_tcp_test(ps_server_url, ui2_port, ui2_timeout/10);
    if (NET_OK != i4_ret)
    {
        dbg_mw_printf("<%s> Connect to %s:%d failed in %d seconds. \n", __FUNCTION__, ps_server_url, ui2_port, ui2_timeout);
        return NET_FAILED;
    }
    return NET_OK;
#endif
}

INT32 c_net_phy_ctrl(const CHAR *ifname, UINT16 fgSet, UINT16 Prm, UINT16 value)
{
		int sockfd;
		struct ifreq ifr;
		struct ioctl_phy_ctrl_cmd *pc_cmd = NULL;

		if (NULL == ifname)
		{
			dbg_mw_printf("No device name.\n");
			return NET_FAILED;
		}

		sockfd = socket(AF_INET, SOCK_DGRAM, 0);
		bzero(&ifr, sizeof(ifr));
		strncpy(ifr.ifr_name, ifname, strlen(ifname));

		pc_cmd = (struct ioctl_phy_ctrl_cmd *)&ifr.ifr_data;
		pc_cmd->wr_cmd = fgSet;
		pc_cmd->Prm    = Prm ;
		pc_cmd->val = value;

		if (ioctl(sockfd, SIOC_PHY_CTRL_CMD , &ifr) < 0)
		{
			dbg_mw_printf("SIOC_PHY_CTRL_CMD  ioctrl command failed, err : %s\n", strerror(errno));
			close (sockfd);
			return NET_FAILED;
		}

		if(!fgSet)
		{
          dbg_mw_printf("result = %d \n", pc_cmd->val);
		}

		close (sockfd);
		return NET_OK;

}

INT32 c_net_mdio_ctrl( UINT32 fgSet, UINT32 reg, UINT16 *value)
{
		int sockfd;
		struct ifreq ifr;
		struct ioctl_mdio_cmd *pc_cmd = NULL;


		sockfd = socket(AF_INET, SOCK_DGRAM, 0);
		bzero(&ifr, sizeof(ifr));
		strncpy(ifr.ifr_name, "eth0", strlen("eth0"));

		pc_cmd = (struct ioctl_mdio_cmd *)&ifr.ifr_data;
		pc_cmd->wr_cmd = fgSet;
		pc_cmd->reg    = reg ;
		pc_cmd->val = *value;

		if (ioctl(sockfd, SIOC_MDIO_CMD , &ifr) < 0)
		{
			dbg_mw_printf("SIOC_MDIO_CMD  ioctrl command failed, err : %s\n", strerror(errno));
			close (sockfd);
			return NET_FAILED;
		}

		if(!fgSet)
		{
		  *value=pc_cmd->val;
         // dbg_mw_printf("result = 0x%x , val=0x%x \n", pc_cmd->val,*value);
		}

		close (sockfd);
		return NET_OK;

}

static VOID free_conn_test_req(CONN_TEST_REQ_T * conn_req)
{
    if (NULL != conn_req)
    {
        if (NULL != conn_req->ps_server_url) free(conn_req->ps_server_url);
        free(conn_req);
    }
}

static VOID c_net_test_connection_thread(VOID * req)
{
    CONN_TEST_REQ_T * conn_req = (CONN_TEST_REQ_T *)(*(int *)req);
    INT32 i4_ret;

    //dbg_mw_printf("<%s> Request: %p, Connect to %s:%d, timeout:%d seconds, callback: %p. \n", __FUNCTION__,
    //        conn_req, conn_req->ps_server_url, conn_req->ui2_port, conn_req->ui2_timeout, conn_req->cb_fp);

    i4_ret = c_net_test_connection(conn_req->ps_server_url, conn_req->ui2_port, conn_req->ui2_timeout);

    conn_req->cb_fp(i4_ret);

    free_conn_test_req(conn_req);
    fg_connection_test_thread_running = FALSE;
}


INT32 c_net_test_connection_async(const CHAR *ps_server_url, UINT16 ui2_port, UINT16 ui2_timeout, connection_test_cb_fp cb_fp)
{
    INT32 i4_status;
    CONN_TEST_REQ_T * conn_req = NULL;

    if (NULL == ps_server_url || NULL == cb_fp)
    {
        return NET_FAILED;
    }

    if (!fg_connection_test_thread_running)
    {
        //start connection test thread
        fg_connection_test_thread_running = TRUE;

        conn_req = (CONN_TEST_REQ_T *)malloc(sizeof(CONN_TEST_REQ_T));
        if ( NULL == conn_req ) return NET_FAILED;

        conn_req->ps_server_url = (CHAR *)malloc(strlen(ps_server_url)+1);
        if (NULL == conn_req->ps_server_url)
        {
            free_conn_test_req(conn_req);
            return NET_FAILED;
        }
        strncpy(conn_req->ps_server_url, ps_server_url, strlen(ps_server_url)+1);
        conn_req->ui2_port      = ui2_port;
        conn_req->ui2_timeout   = ui2_timeout;
        conn_req->cb_fp         = cb_fp;

        //dbg_mw_printf("<%s> Request: %p, Connect to %s:%d, timeout:%d seconds, callback: %p. \n", __FUNCTION__,
        //    conn_req, conn_req->ps_server_url, conn_req->ui2_port, conn_req->ui2_timeout, conn_req->cb_fp);

        i4_status = u_thread_create (&h_connection_test_thread,
                              "CONNTEST",
                              32*1024,
                              NI_MONITOR_THREAD_PRIORITY,
                              c_net_test_connection_thread,
                              sizeof(VOID *),
                              (VOID *)&conn_req);

        if (i4_status != OSR_OK)
        {
            fg_connection_test_thread_running = FALSE;
            /* Bad news, some other error. Well, simply return a NULL_HANDLE. */
            dbg_mw_printf("Create connection test thread failed, ret = %d\n", (int)i4_status);
            free_conn_test_req(conn_req);
            return NET_FAILED;
        }
    }
    return NET_OK;
}

/**
* Test if a proxy server still open by trying to make a connection to the server.
*
* @param  ps_server_url     URL or IP address of the proxy server.
* @param  ui2_port          Server port of the proxy server.
*
* @return NET_OK      Connection test success.
*         NET_FAILED  Connection test fail.
*/
INT32    c_net_proxy_connection(const CHAR *ps_server_url, UINT16 ui2_port)
{
    if(NET_OK == timed_tcp_test(ps_server_url, ui2_port, SOCKET_CONNECTION_TIMEOUT_SEC/2))
    	return NET_OK;
    else 
    	return c_net_test_connection_v6(ps_server_url, ui2_port, SOCKET_CONNECTION_TIMEOUT_SEC/2);
}


/**
* Open MTDs and parse their MAC address.
*
* @return fgMacFoundMtd    TRUE:    Find MTD devices successful.
*                          FALSE:   Find MTD devices fail.
*/
BOOL fgMwMacParseMtdTable(void)
{
    BOOL  fgMacFoundMtd = FALSE;

    INT32 i4fd;

    i4fd = open("/proc/mtd", O_RDONLY | O_NONBLOCK, 0);

    if (i4fd< 0)
    {
        dbg_mw_printf("[MAC]<open /proc/mtd> failed !!\n");
        return FALSE;
    }
    else
    {
        UINT8  *file_buf;
        UINT32 rd_length;
        UINT32 line_start = 0, line_end = 0;
        UINT32 dev_len;
        UINT32 dev_name_start, dev_name_len;
        int temp = 0;

        file_buf = malloc((SIZE_T) 2048);
        if (NULL == file_buf)
        {
            dbg_mw_printf("%s malloc() fail at line %d", __FUNCTION__, __LINE__);
            close(i4fd);
            return FALSE;
        }

        memset(file_buf, 0, (SIZE_T) 2048);
        rd_length = read(i4fd, file_buf, 2048);

        //dbg_mw_printf("[MAC]Analyze /proc/mtd rd_length = %d !!\n", rd_length);
        dbg_mw_printf("[zy]dump buffer begin\n");
        for (temp = 0; temp <= rd_length; temp++)
        {
            dbg_mw_printf("%2x", file_buf[temp]);
            
        }
        dbg_mw_printf("\n");

        while (line_end < rd_length)
        {
          while ((file_buf[line_end] == 0x0A) || (file_buf[line_end] == 0x0D))
          { 
            line_end ++;
            if (line_end == 2048)
            {
                dbg_mw_printf("[MAC]1<open /proc/mtd> failed line_end[%d]!!\n", line_end);
                break;
            }
            
          }
          //dbg_mw_printf("[MAC]2<open /proc/mtd> failed line_end[%d]!!\n", line_end);
          if (line_end >= rd_length)
            break;

          line_start = line_end;
          while (((file_buf[line_end] != 0x0A) && (file_buf[line_end] != 0x0D)) ||
                 (line_end >= rd_length))
            line_end ++;

          dev_len = 0;
          while (file_buf[line_start + dev_len] != ':')
            dev_len ++;
          file_buf[line_start + dev_len] = 0;

          dev_name_start = dev_len + 1;
          while (file_buf[line_start + dev_name_start] != '\"')
            dev_name_start ++;

          dev_name_start ++;
          dev_name_len = 0;
          while (file_buf[line_start + dev_name_start + dev_name_len] != '\"')
            dev_name_len ++;

          if (strncmp(MTD_MAC_ADDR_1_NAME, (const CHAR *)&(file_buf[line_start + dev_name_start]), dev_name_len) == 0)
          {
            snprintf(MTD_MAC_ADDR_1, 16, "/dev/%s", &(file_buf[line_start]));
            fgMacFoundMtd = TRUE;
          }
          else if (strncmp(MTD_MAC_ADDR_2_NAME, (const CHAR *)&(file_buf[line_start + dev_name_start]), dev_name_len) == 0)
          {
            snprintf(MTD_MAC_ADDR_2, 16, "/dev/%s", &(file_buf[line_start]));
          }
        }

        //dbg_mw_printf("%s - %s\n",  MTD_MAC_ADDR_1_NAME, MTD_MAC_ADDR_1);
        //dbg_mw_printf("%s - %s\n",  MTD_MAC_ADDR_2_NAME, MTD_MAC_ADDR_2);

        free(file_buf);

        close(i4fd);

        return fgMacFoundMtd;
    }
    return FALSE;
}


/**
* Create a folder according to the parameter path.
* If the folder doesn't exist, then create it, else do nothing.
*
* @return NET_OK        Create folder successfully.
*         NET_FAILED    Fail to create folder.
*/
    /*

INT32 i4mw_create_folder(const CHAR *path)
{
    HANDLE_T h_dir = NULL_HANDLE;
    INT32 i4_ret = 0;

    //check the folder if exist, if no then create it
    i4_ret = x_fm_open_dir(FM_ROOT_HANDLE, path, &h_dir);
    if (i4_ret != FMR_OK)
    {
        //if misc folder not exist, create one
        dbg_mw_printf("<MwNetCfg>NO /misc folder\n");
        //create buda folder
        i4_ret = x_fm_create_dir(FM_ROOT_HANDLE, path, 0777);
        if (i4_ret != FMR_OK)
        {
            dbg_mw_printf("<MwNetCfg>create /misc folder failed, err code = [%d]\n", i4_ret);
            return NET_FAILED;
        }

        i4_ret = x_fm_open_dir(FM_ROOT_HANDLE, path, &h_dir);
        if (i4_ret != FMR_OK)
        {
            dbg_mw_printf("<MwNetCfg>creat /misc folder fail\n");
            return i4_ret;
        }
    }
    //close the folder
    x_fm_close(h_dir);

    return NET_OK;
}
*/

INT32 c_net_set_mac_to_sys(CHAR *pc_mac)
{
    INT32    i4_ret = 0;
    /*
    HANDLE_T h_file = NULL_HANDLE;
    UINT32   ui4_size = 0;

    #if CONFIG_MW_USB_BOOTUP
        BOOL   fgMacMtd = FALSE;
    #else       
        BOOL   fgMacMtd = fgMwMacParseMtdTable();
    #endif

    if (NULL == pc_mac)
    {
        return NET_INV_ARG;
    }

    if (fgMacMtd)
    {
      UINT32 u4DevId = 0;
      dbg_mw_printf("<MwNetCfg> SET MAC in RAW Mode.\n");

      x_sscanf(MTD_MAC_ADDR_1, BLOCK_DEV, &u4DevId);
      i4_ret = i4NFBPartitionWrite(u4DevId, 0, (UINT32)pc_mac, 6);
      if (i4_ret < 0)
        return NET_FAILED;

      x_sscanf(MTD_MAC_ADDR_2, BLOCK_DEV, &u4DevId);
      i4_ret = i4NFBPartitionWrite(u4DevId, 0, (UINT32)pc_mac, 6);
      if (i4_ret < 0)
        return NET_FAILED;

      dbg_mw_printf("[[MwNetCfg]SET mac addr: %2x-%2x-%2x-%2x-%2x-%2x in RAW Mode OK\n",
                             pc_mac[0],pc_mac[1],pc_mac[2],pc_mac[3],pc_mac[4],pc_mac[5]);
      return NET_OK;
    }
    else
    {
      dbg_mw_printf("<MwNetCfg> SET MAC in File Mode.\n");
      //check if the file exist, if no then create it
      i4_ret = x_fm_open(NULL_HANDLE,
                         (const CHAR*)(LINUX_NETWORK_MAC_ADDRE_PATH),
                         FM_OPEN_CREATE | FM_READ_WRITE | FM_OPEN_SYNC ,
                         0777,
                         FALSE,
                         &h_file);
      if (i4_ret)
      {
          dbg_mw_printf("<MwNetCfg> x_fm_open fail: %s, %d, [file name] is%s\r\n", __FILE__, __LINE__, LINUX_NETWORK_MAC_ADDRE_PATH);

          //first check if the folder is exist, if no then create it
          i4_ret = i4mw_create_folder((const CHAR*)(LINUX_MISC_FOLDER));
          if (i4_ret)
          {
               dbg_mw_printf("<MwNetCfg> create_folder fail: %s, %d, [file name] is%s\r\n", __FILE__, __LINE__, LINUX_MISC_FOLDER);
               return  NET_FAILED;
          }

          //check the file if exist, if no then create it
          i4_ret = x_fm_open(NULL_HANDLE,
                         (const CHAR*)(LINUX_NETWORK_MAC_ADDRE_PATH),
                         FM_OPEN_CREATE | FM_READ_WRITE,
                         0777,
                         FALSE,
                         &h_file);
          if (i4_ret)
          {
              dbg_mw_printf("<MwNetCfg> x_fm_open fail: %s, %d, [file name] is%s\r\n", __FILE__, __LINE__, LINUX_NETWORK_MAC_ADDRE_PATH);
              return NET_FAILED;
          }
      }

      //write mac addr to the file
      i4_ret = x_fm_write(h_file,(const VOID *) pc_mac, sizeof(MAC_ADDRESS_T), &ui4_size);
      if ((FMR_OK != i4_ret) || (ui4_size != sizeof(MAC_ADDRESS_T)))
      {
          dbg_mw_printf("<MwNetCfg> x_fm_write fail: %s, %d, i4_ret=[%ld], ui4_size=[%ld]\r\n", __FILE__, __LINE__, i4_ret, ui4_size);
      }

      x_fm_close(h_file);
      dbg_mw_printf("[[MwNetCfg]SET mac addr: %2x-%2x-%2x-%2x-%2x-%2x in File Mode OK\n",
                             pc_mac[0],pc_mac[1],pc_mac[2],pc_mac[3],pc_mac[4],pc_mac[5]);

      return NET_OK;
    }
    */

    return i4_ret;
}

INT32 c_net_get_mac_from_sys(CHAR *pac_mac)
{

    INT32    i4_ret = 0;
    /*
    HANDLE_T h_file = NULL_HANDLE;
    UINT32   ui4_size = 0;

    #if CONFIG_MW_USB_BOOTUP
        BOOL   fgMacMtd = FALSE;
    #else       
        BOOL   fgMacMtd = fgMwMacParseMtdTable();
    #endif

    if (NULL == pac_mac)
    {
        return NET_INV_ARG;
    }

    if (fgMacMtd)
    {
      UINT32 u4DevId = 0;
      //dbg_mw_printf("<MwNetCfg> GET MAC in RAW Mode.\n");
      x_sscanf(MTD_MAC_ADDR_1, BLOCK_DEV, &u4DevId);
      i4_ret = i4NFBPartitionRead(u4DevId, 0, (UINT32)pac_mac, 6);
      if (i4_ret < 0)
      {
        x_sscanf(MTD_MAC_ADDR_2, BLOCK_DEV, &u4DevId);
        i4_ret = i4NFBPartitionRead(u4DevId, 0, (UINT32)pac_mac, 6);
        if (i4_ret < 0)
          return NET_FAILED;
      }
      dbg_mw_printf("[[MwNetCfg]:mac addr: %2x-%2x-%2x-%2x-%2x-%2x \n",
                       pac_mac[0],pac_mac[1],pac_mac[2],pac_mac[3],pac_mac[4],pac_mac[5]);

      return NET_OK;
    }
    else
    {
      UCHAR    amac[sizeof(MAC_ADDRESS_T)] = {0};
      dbg_mw_printf("<MwNetCfg> GET MAC in File Mode.\n");
      //open the file, if error, return failed, else read the mac address according to psz_name
      i4_ret = x_fm_open(NULL_HANDLE,
                         (const CHAR*)(LINUX_NETWORK_MAC_ADDRE_PATH),
                         FM_READ_ONLY,
                         0777,
                         FALSE,
                         &h_file);
      if (i4_ret)
      {
          dbg_mw_printf("<MwNetCfg> x_fm_open fail: %s, %d, [file name] is%s\r\n", __FILE__, __LINE__, LINUX_NETWORK_MAC_ADDRE_PATH);
          return NET_FAILED;
      }

      //read file
      i4_ret = x_fm_read(h_file, amac, sizeof(MAC_ADDRESS_T), &ui4_size);
      if ((FMR_OK != i4_ret) || (ui4_size != sizeof(MAC_ADDRESS_T)))
      {
          dbg_mw_printf("<MwNetCfg> x_fm_read fail: %s, %d, i4_ret=[%ld], ui4_size=[%ld]\r\n", __FILE__, __LINE__, i4_ret, ui4_size);
      }

      memcpy((VOID *) pac_mac, (const VOID *) amac, sizeof(MAC_ADDRESS_T));

      x_fm_close(h_file);
      dbg_mw_printf("[[MwNetCfg]GET mac addr: %2x-%2x-%2x-%2x-%2x-%2x in File Mode OK\n",
                       pac_mac[0],pac_mac[1],pac_mac[2],pac_mac[3],pac_mac[4],pac_mac[5]);

      return NET_OK;
    }
    */

    return i4_ret;
}


int isSummerTime()
{
	time_t now_t = time(0);
    struct tm *now = localtime(&now_t);
        /*count */
	int start_hour = (31 - ((now->tm_year * 5)/4 + 4)%7) * 24 + 1;
	int end_hour = (31 -((now->tm_year * 5)/4 + 1)%7) * 24 + 1;
	int totalHour = now->tm_mday*24 + now->tm_hour;
	
	if(now->tm_mon > 3 && now->tm_mon < 10) 
	{
		printf("summer time match - 1!\n");
		return 1;
	}
	else if(now->tm_mon == 3)
	{
		if(totalHour >= start_hour)
      		{
			printf("summer time match - 2!\n");
			return 1;
		}
	}
	else if(now->tm_mon == 10)
	{
		if(totalHour < end_hour)
      		{
			printf("summer time match - 3!\n");
			return 1;
		}

	}
	printf("not in summer time!\n");
	return 0;
}


void c_set_time_zone(int i_zone)
{/*zone should be -12 <=> 11, and should reverse*/
	i_zone = -1*(((i_zone+12) % 24) - 12);
	char cmd[100] = {0};
	char fileName[10] = "GMT";
	if(i_zone < 0)
		snprintf(fileName, 9, "GMT%d", i_zone);
	else if(i_zone > 0)
		snprintf(fileName, 9, "GMT+%d", i_zone);		

	snprintf(cmd, 100, "cp -rf /usr/share/zoneinfo/%s /etc/localtime", fileName);
	system(cmd);
}

static VOID addrinfo_mon_thread(VOID *arg_host)
{
    struct addrinfo hint;
    int int4_Ret = NET_OK;
    NET_ADDR_INFO_LOOK_UP *ptAddrInfoLookup = (NET_ADDR_INFO_LOOK_UP *)arg_host;
    if (NULL == ptAddrInfoLookup)
    {
        NET_LOG("<addrinfo_mon_thread>invalid parameter!\n");
        fg_addrinfo_thread_running = FALSE;
        u_thread_exit();
    }

    if(NULL == ptAddrInfoLookup->ps_server_url)
    {
        NET_LOG("<addrinfo_mon_thread>invalid parameter!\n");
        fg_addrinfo_thread_running = FALSE;
        u_thread_exit();
    }

    NET_LOG("<addrinfo_mon_thread>url=%s,eAddrFamily=%d!\n", 
        ptAddrInfoLookup->ps_server_url, ptAddrInfoLookup->eAddrFamily);
    if (ptAddrInfoLookup->eAddrFamily == NET_AF_IPV4)
    { 
        NET_LOG("<addrinfo_mon_thread>NET_AF_IPV4\n");
        bzero(&hint, sizeof(hint));
        hint.ai_family = AF_INET;
        hint.ai_socktype = SOCK_STREAM;
        
        int4_Ret = getaddrinfo(ptAddrInfoLookup->ps_server_url, NULL, &hint, &answer);
    
        if (int4_Ret != 0) 
        {            
            NET_LOG("<addrinfo_mon_thread>getaddrinfo: %s!\n", gai_strerror(int4_Ret));
        }     
    }
    else if (ptAddrInfoLookup->eAddrFamily == NET_AF_IPV6)
    { 
        NET_LOG("<addrinfo_mon_thread>NET_AF_IPV6\n");
        bzero(&hint, sizeof(hint));
        hint.ai_family = AF_INET6;
        hint.ai_socktype = SOCK_STREAM;
        
        int4_Ret = getaddrinfo(ptAddrInfoLookup->ps_server_url, NULL, &hint, &answer);
    
        if (int4_Ret != 0) 
        {
            NET_LOG("<addrinfo_mon_thread>getaddrinfo: %s!\n", gai_strerror(int4_Ret));
        }    
        
    }

    fg_addrinfo_thread_running = FALSE;
    u_thread_exit();
}

/**
* Thread function body for gethostbyname
*
* @param   arg_host    The server name which is going to be looked up.
*
* @return
*
*/
static VOID host_mon_thread(VOID *arg_host)
{
      struct in_addr ip_addr = {0};
      dbg_mw_printf("Param host %s\n", (const char*)arg_host);
      pt_hostnet = gethostbyname((const char*)arg_host);

      if (pt_hostnet)
      {
          dbg_mw_printf("gethostbyname ok\n");
          ip_addr = *(struct in_addr*)(pt_hostnet->h_addr_list[0]);
          dbg_mw_printf("IP = %s\n", inet_ntoa(ip_addr));
      }
      else
      {
          dbg_mw_printf("err: %s %d\n", hstrerror(h_errno), h_errno);
      }
      fg_host_thread_running = FALSE;
      u_thread_exit();
      /*
       const char *serv = "80";
       struct addrinfo hints;
       struct addrinfo* res = NULL;
       int error;

       memset(&hints, 0, sizeof(hints));
       hints.ai_family = AF_INET;
       hints.ai_socktype = SOCK_DGRAM;
       hints.ai_flags = AI_CANONNAME;

       error = getaddrinfo(net_station1, serv, &hints, &res);

       if (error) {
           DBG_INFO(("getaddrinfo: %s error=%d: %s\n",
                   net_station1, error, gai_strerror(error)));
           *b_status = FALSE;
       }
       else
       {
           DBG_INFO(("getaddrinfo ok: %s\n",
                         net_station1));//, inet_ntoa(((struct sockaddr_in *)res->ai_addr)->sin_addr)));
           *b_status = TRUE;
       }
       freeaddrinfo(res);
      */
}

struct hostent *    c_net_gethostbyname_ex(const CHAR *ps_server_url)// mtk70199_0913
{
  INT32          i4_status;
  if (!fg_host_thread_running)
  {
      dbg_mw_printf("%s, param = %s %d\n", __FUNCTION__ ,ps_server_url, strlen(ps_server_url));
      fg_host_thread_running = TRUE;
      usleep(50000);
      // Create the polling thread first
      i4_status = u_thread_create (&h_host_monitor_thread,
                          "HOSTD",
                          1024 * 32,
                          NI_MONITOR_THREAD_PRIORITY,
                          host_mon_thread,
                          strlen(ps_server_url),
                          (VOID *)ps_server_url);

      if (i4_status != OSR_OK)
      {
          fg_host_thread_running = FALSE;
          /* Bad news, some other error. Well, simply return a NULL_HANDLE. */
          dbg_mw_printf("Create HOST thread failed, ret = %d\n", (int)i4_status);
          return NULL;
      }
  }
  /* SH Lin@20100517 avoid waiting 25 secs for gethostbyname thread timeout. */
  i4_status = 0;
  dbg_mw_printf("gethostbyname time out 1111.\n");
  while (fg_host_thread_running && i4_status < (1000)) //(GETHOSTBYNAME_TIMEOUT_SEC/2*1000))
  {
    u_thread_delay(2);
    i4_status++;
  }
  dbg_mw_printf("gethostbyname time out.\n");
  if (TRUE == fg_host_thread_running)
  {
    dbg_mw_printf("gethostbyname time out.\n");
    return NULL;
  }

  return pt_hostnet;
}


struct hostent *    c_net_gethostbyname_timeout(const CHAR *ps_server_url,INT32 timeout)
{
  INT32          i4_status;
  if (!fg_host_thread_running)
  {
     
      fg_host_thread_running = TRUE;
      usleep(50000);
      // Create the polling thread first
      i4_status = u_thread_create (&h_host_monitor_thread,
                          "HOSTD",
                          1024 * 32,
                          NI_MONITOR_THREAD_PRIORITY,
                          host_mon_thread,
                          strlen(ps_server_url),
                          (VOID *)ps_server_url);

      if (i4_status != OSR_OK)
      {
          fg_host_thread_running = FALSE;
          /* Bad news, some other error. Well, simply return a NULL_HANDLE. */
          dbg_mw_printf("Create HOST thread failed, ret = %d\n", (int)i4_status);
          return NULL;
      }
  }
 
  i4_status = 0;
  dbg_mw_printf("gethostbyname time out 1111.\n");
  while (fg_host_thread_running && i4_status < (timeout*1000)) //(GETHOSTBYNAME_TIMEOUT_SEC/2*1000))
  {
    u_thread_delay(2);
    i4_status++;
  }

  if (TRUE == fg_host_thread_running)
  {
    dbg_mw_printf("gethostbyname time out.\n");
    return NULL;
  }

  return pt_hostnet;
}



/**
* Create a separately thread to invoke gethostbyname() proivided by system library.
* Because gethostbyname() keeps the result of last time we inquired for same server name
* in thread-level granularity, hence we create a new thread to enforce gethostbyname()
* return the latest result from dns server.
*
* @param  ps_server_url    The server name which is going to be looked up.
*
* @return pt_hostnet       A stracture contains ip address of the server name we looked up.
*         NULL             Lookup fail.
*
*/
struct hostent *    c_net_gethostbyname(const CHAR *ps_server_url)
{
  INT32          i4_status;
  if (!fg_host_thread_running)
  {
      dbg_mw_printf("%s, param = %s %d\n", __FUNCTION__ ,ps_server_url, strlen(ps_server_url));
      fg_host_thread_running = TRUE;
      // Create the polling thread first
      i4_status = u_thread_create (&h_host_monitor_thread,
                          "HOSTD",
                          1024 * 32,
                          NI_MONITOR_THREAD_PRIORITY,
                          host_mon_thread,
                          strlen(ps_server_url),
                          (VOID *)ps_server_url);

      if (i4_status != OSR_OK)
      {
          fg_host_thread_running = FALSE;
          /* Bad news, some other error. Well, simply return a NULL_HANDLE. */
          dbg_mw_printf("Create HOST thread failed, ret = %d\n", (int)i4_status);
          return NULL;
      }
  }
  /* SH Lin@20100517 avoid waiting 25 secs for gethostbyname thread timeout. */
  i4_status = 0;
  while (fg_host_thread_running && i4_status < (GETHOSTBYNAME_TIMEOUT_SEC/2*1000))
  {
    u_thread_delay(2);
    i4_status++;
  }
  if (TRUE == fg_host_thread_running)
  {
    dbg_mw_printf("gethostbyname time out.\n");
    return NULL;
  }

  return pt_hostnet;
}

struct addrinfo *c_net_get_addrinfo_by_addrtype(const NET_ADDR_INFO_LOOK_UP *ptAddrInfoLookup)
{
    INT32          i4_status;
    if (!fg_addrinfo_thread_running)
    {
        NET_LOG("<dns lookup>url=%s,eAddrFamily=%d\n", 
        ptAddrInfoLookup->ps_server_url, ptAddrInfoLookup->eAddrFamily);
        fg_addrinfo_thread_running = TRUE;
        // Create the polling thread first
        i4_status = u_thread_create (&h_addrinfo_monitor_thread,
                              "ADDRINFOD",
                              1024 * 32,
                              NI_MONITOR_THREAD_PRIORITY,
                              addrinfo_mon_thread,
                              sizeof(NET_ADDR_INFO_LOOK_UP),
                              (VOID *)ptAddrInfoLookup);

        if (i4_status != OSR_OK)
        {
            fg_addrinfo_thread_running = FALSE;
            /* Bad news, some other error. Well, simply return a NULL_HANDLE. */            
            NET_LOG("<get_addrinfo_by_addrtype>Create HOST thread failed[%d]!\n", (int)i4_status);
            return NULL;
        }
    }
    /* SH Lin@20100517 avoid waiting 25 secs for gethostbyname thread timeout. */
    i4_status = 0;
    while (fg_addrinfo_thread_running && i4_status < (GETHOSTBYNAME_TIMEOUT_SEC/2*1000))
    {
        u_thread_delay(2);
        i4_status++;
    }
    if (TRUE == fg_addrinfo_thread_running)
    {
        NET_LOG("<get_addrinfo_by_addrtype>get addr info failed!\n");
        return NULL;
    }

  return answer;
}
VOID ping_host_thread(VOID *arg_host)
{
  INT32  i4_status;
  CHAR cmd[128] = {0};

  dbg_mw_printf("Param host %s\n", (const char*)arg_host);

  snprintf(cmd, 128,"ping -c 1 %s", (const char*)arg_host);
  i4_status = system(cmd);

  if (0 == i4_status)
  {
      dbg_mw_printf("ping hostname ok\n");
      ping_host_result = NET_OK;
  }
  else
  {
      dbg_mw_printf("ping hostname fail\n");
      ping_host_result = NET_FAILED;
  }
  fg_ping_host_thread_running = FALSE;
  u_thread_exit();
}

INT32 c_net_pinghostbyname(const CHAR *ps_server_url)
{
  INT32  i4_status;

  if (!fg_ping_host_thread_running)
  {
      dbg_mw_printf("%s, param = %s %d\n", __FUNCTION__ ,ps_server_url, strlen(ps_server_url));
      fg_ping_host_thread_running = TRUE;
      ping_host_result = NET_FAILED;

      // Create the polling thread first
      i4_status = u_thread_create (&h_ping_host_thread,
                          "PINGD",
                          1024 * 20,
                          NI_MONITOR_THREAD_PRIORITY,
                          ping_host_thread,
                          strlen(ps_server_url),
                          (VOID *)ps_server_url);

      if (i4_status != OSR_OK)
      {
          fg_ping_host_thread_running = FALSE;
          /* Bad news, some other error. Well, simply return a NULL_HANDLE. */
          dbg_mw_printf("Create PING thread failed, ret = %d\n", (int)i4_status);
          return NET_FAILED;
      }
  }

  i4_status = 0;
  while (fg_ping_host_thread_running && (i4_status < (PINGHOSTBYNAME_TIMEOUT_SEC*1000/2)))
  {
    u_thread_delay(2);
    i4_status++;
  }
  if (TRUE == fg_ping_host_thread_running)
  {
    dbg_mw_printf("%s time out.\n", __FUNCTION__);
    return NET_FAILED;
  }

  return ping_host_result;
}


VOID c_net_clean_dns_cache(VOID)
{
#if (FORK2NETINFD)
    char *dnsconfig = "/etc/resolv.conf";
    FILE * fd;

    fd = fopen(dnsconfig, "wt");
    if (fd != NULL)
      fclose (fd);
#else
    char *argp [3];
    int i4ret = 0;
    int pid, wpid, wstatus;
    char *script = "/sbin/dns.script";

    argp [0] = script;
    argp [1] = (char *)"clean";
    argp [2] = NULL;

    pid = fork ();
    if (pid < 0)
    {
      dbg_mw_printf ("fork: %d\n", errno);
      wstatus = 0;
    }
    else if (pid)
    { //parent
      do {
        wpid = wait (&wstatus);
      } while (wpid != pid && wpid > 0);
      dbg_mw_printf ("child %d terminated\n", wpid);
      dbg_mw_printf ("child status : %d\n", WEXITSTATUS(wstatus));

      if (wpid < 0)
      {
        dbg_mw_printf("wait: %d, err: %s\n", wpid, strerror(errno));
        wstatus = 0;
      }
    }
    else
    { // child
      i4ret = execv (script, argp);
      dbg_mw_printf ("execve result = %d\n", i4ret);
      if (i4ret == -1)
      {
        dbg_mw_printf("execve errno = %s\n", strerror(errno));
      }
      exit (0);
    }
#endif
}

static int ping_chksum(unsigned short *buf, int len)
{
	int nlength = len;
	INT32 i2sum = 0;
	unsigned short *b = buf;
	unsigned short ret = 0;

	while (nlength > 1)
  {
		i2sum += *b++;
		nlength -= 2;
	}

	if (nlength == 1)
  {
		*(unsigned char *) (&ret) = *(unsigned char *) b;
		i2sum += ret;
	}

	i2sum = (i2sum >> 16) + (i2sum & 0xFFFF);
	i2sum += (i2sum >> 16);
	ret = ~i2sum;
	return ret;
}

static INT32 wait_reply_timeout(INT32 sockfd, INT32 waitsec)
{
  fd_set rset;
  struct timeval tv;

  FD_ZERO(&rset);
  FD_SET(sockfd, &rset);

  tv.tv_sec = waitsec;
  tv.tv_usec = 0;
  return (select(sockfd + 1, &rset, NULL, NULL, &tv));
}


static INT32 wait_reply_timeout_ms(INT32 sockfd, INT32 waitms)
{
  fd_set rset;
  struct timeval tv;

  FD_ZERO(&rset);
  FD_SET(sockfd, &rset);

  tv.tv_sec = waitms/1000;
  tv.tv_usec = (waitms%1000)*1000;
  return (select(sockfd + 1, &rset, NULL, NULL, &tv));
}


/**
 * c_net_ping_v4
 *
 * ping utility, user may use this API to verify remote host
 * @param ps_ip, dot format IPv4 address, ex "192.168.1.1"
 * @param i2_len, payload size
 * @param i2_wait, maximum wait time
 * @param notify_fn, notify callback
 *
 * @return VOID
 */
#define PING_MS 0x8000
VOID c_net_ping_v4(CHAR * ps_ip,
                  INT32  i2_len,
                  INT16  i2_wait,
                  VOID (*notify_fn) (INT16 i2_rsp_time))
{
    struct sockaddr_in *pingServeraddr = NULL;
    struct in_addr ip_addr;
    struct icmp *pkt;
    INT32  pingsock, ret;
    UINT32 diff;
    char packet[MaxBufSz];
    int len, sendLen;
    pid_t ping_pid;
    struct addrinfo addrhint;
    struct addrinfo *pingaddrinfo = NULL;
    BOOL needFree = FALSE;

    memset(&ip_addr, 0, sizeof (struct in_addr));
	bzero(&addrhint, sizeof(struct addrinfo));
    addrhint.ai_flags  = AI_CANONNAME;
    addrhint.ai_family = 0;
    addrhint.ai_socktype = 0;

	
	if(0 == inet_aton(ps_ip, &ip_addr))/*Not ip, maybe URL*/
	{		
		if (getaddrinfo(ps_ip, NULL, &addrhint, &pingaddrinfo) != 0)
		{
		  dbg_mw_printf("Addr not found, please enter dot format IPv4 address\n");
		  notify_fn(-1);
		  return;
		}
        pingServeraddr = (struct sockaddr_in *)(pingaddrinfo->ai_addr);
        sendLen = pingaddrinfo->ai_addrlen;
	}
    else
    {
      /* malloc memory for pingServeraddr,and turn on free memory flag */
      pingServeraddr = (struct sockaddr_in *) malloc(sizeof(struct sockaddr_in));
      if (NULL == pingServeraddr)
      {
        dbg_mw_printf("Malloc memory for pingServeraddr fail\n");
        return;
      }
      else
      {
        needFree = TRUE;
      }
      pingServeraddr->sin_family = AF_INET;
      pingServeraddr->sin_addr = ip_addr;
      sendLen = sizeof(struct sockaddr_in);
    }

  	pkt = (struct icmp *) packet;
  	memset(pkt, 0, sizeof(packet));
    /*
     * make up ICMP header.
     */
    pkt->icmp_type = ICMP_ECHO; //echo request, 8
    pkt->icmp_code = 0;
    ping_pid = getpid();
    pkt->icmp_id = ping_pid;
    gettimeofday((struct timeval *)pkt->icmp_data, NULL);
	
    len = 8 + DefaultDataLen;//sizeof(packet)
    if (i2_len > 0)
    {
        i2_len = i2_len < MaxBufSz ? i2_len :  MaxBufSz;
        len    = len < i2_len ? i2_len :  len;
    }
	  pkt->icmp_cksum = ping_chksum((unsigned short *) pkt, len);

    pingsock = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);    

    ret = sendto(pingsock, packet, len, 0,
                (struct sockaddr *) pingServeraddr, sendLen);

    /* free pingServeraddr if the memory allocate here*/
    if (needFree)
    {
        free(pingServeraddr);
    }else{
        freeaddrinfo(pingaddrinfo);
    }

    if (ret < 0)
    {
		printf("send fail:%s", strerror(errno));
      close(pingsock);
      notify_fn(-1);
      return;
    }

    struct sockaddr_in replyAddr;
    socklen_t replylen = sizeof(struct sockaddr_in);
    INT32 recvSize = 60 * 1024; // in case of many replies
    setsockopt(pingsock, SOL_SOCKET, SO_RCVBUF, &recvSize, sizeof(recvSize));

recvagain:
    /* wait to listen for replies */
	if(i2_wait & PING_MS)
	   i2_wait &= ~PING_MS;
	else
	   i2_wait *= 1000;
    if (wait_reply_timeout_ms(pingsock, i2_wait) == 0)
    {
      printf("socket timeout\n");
      close(pingsock);
      notify_fn(-1);
      return;
    }
    else
    {
        dbg_mw_printf("before pingsock=%d, replylen=%d\n", pingsock, replylen);
      ret = recvfrom(pingsock, packet, sizeof(packet), 0,
                     (struct sockaddr *) &replyAddr, &replylen);
      dbg_mw_printf("after pingsock=%d, replylen=%d\n", pingsock, replylen);

      if (ret < 0)
      {
        dbg_mw_printf("recvfrom error:%s\n", strerror(errno));
        close(pingsock);
        notify_fn(-1);
        return;
      }

      // unpack as the icmp packet
      if (ret >= IcmpMaxLen) /* ip + icmp */
      {
        struct iphdr *iphdr = (struct iphdr *) packet;
        pkt = (struct icmp *) (packet + (iphdr->ihl << 2));	/* skip ip hdr */
      }
      else
      {
        pkt = (struct icmp *) packet;
      }

      // check the pid
      if (pkt->icmp_id != ping_pid)
      {
        dbg_mw_printf("reply id %d is wrong with %d, recv again.\n", pkt->icmp_id, ping_pid);
        goto recvagain;
      }

      if (pkt->icmp_type == ICMP_ECHOREPLY)
      {
        struct timeval tmRecvPing = {0};
        struct timeval * tmSendPing = (struct timeval *)pkt->icmp_data;
        gettimeofday(&tmRecvPing, NULL);

        if (tmRecvPing.tv_usec < tmSendPing->tv_usec)
        {
          tmRecvPing.tv_sec --;
          tmRecvPing.tv_usec += 1000000;
        }

        tmRecvPing.tv_sec  -= tmSendPing->tv_sec;
        tmRecvPing.tv_usec -= tmSendPing->tv_usec;
        diff = (tmRecvPing.tv_sec * 1000) + (tmRecvPing.tv_usec / 1000);
        close(pingsock);
        notify_fn(diff);
      }else{
        dbg_mw_printf("reply icmp_type= %d is wrong with %d, recv again.\n", pkt->icmp_type, ICMP_ECHOREPLY);
        goto recvagain;
      }
    }
    close(pingsock);
    return;
}

VOID c_dlna_net_ping_v4(CHAR * ps_ip,
                               INT16  i2_len,
                               INT16  i2_wait,
                               VOID* dlnaPingHandle,
                               VOID (*notify_fn)(VOID* h_dlna_notify, INT16 i2_rsp_time))
{
    struct sockaddr_in *pingServeraddr = NULL;
    struct in_addr ip_addr;
    struct icmp *pkt;
    INT32  pingsock, ret;
    UINT32 diff;
    char packet[MaxBufSz];
    int len, sendLen;
    pid_t ping_pid;
    struct addrinfo addrhint;
    struct addrinfo *pingaddrinfo = NULL;
    BOOL needFree = FALSE;

    memset(&ip_addr, 0, sizeof (struct in_addr));
    inet_aton(ps_ip, &ip_addr);

    bzero(&addrhint, sizeof(struct addrinfo));
    addrhint.ai_flags  = AI_CANONNAME;
    addrhint.ai_family = 0;
    addrhint.ai_socktype = 0;

    if (getaddrinfo(ps_ip, NULL, &addrhint, &pingaddrinfo) != 0)
    {
      pingaddrinfo = NULL;
    }

    if (pingaddrinfo != NULL)
    {
      pingServeraddr = (struct sockaddr_in *)(pingaddrinfo->ai_addr);
      sendLen = pingaddrinfo->ai_addrlen;
    }
    else
    {
      if ((UINT32)ip_addr.s_addr == 0)
      {
        dbg_mw_printf("Addr not found, please enter dot format IPv4 address\n");
        notify_fn(dlnaPingHandle,-1);
        return;
      }

      /* malloc memory for pingServeraddr,and turn on free memory flag */
      pingServeraddr = (struct sockaddr_in *) malloc(sizeof(struct sockaddr_in));
      if (NULL == pingServeraddr)
      {
        dbg_mw_printf("Malloc memory for pingServeraddr fail\n");
        return;
      }
      else
      {
        needFree = TRUE;
      }

      pingServeraddr->sin_family = AF_INET;
      pingServeraddr->sin_addr = ip_addr;
      sendLen = sizeof(struct sockaddr_in);
    }

  	pkt = (struct icmp *) packet;
  	memset(pkt, 0, sizeof(packet));
    /*
     * make up ICMP header.
     */
    pkt->icmp_type = ICMP_ECHO; //echo request, 8
    pkt->icmp_code = 0;
    ping_pid = getpid();
    pkt->icmp_id = ping_pid;
    gettimeofday((struct timeval *)pkt->icmp_data, NULL);

    len = 8 + DefaultDataLen;//sizeof(packet)
    if (i2_len > 0)
    {
        i2_len = i2_len < MaxBufSz ? i2_len :  MaxBufSz;
        len    = len < i2_len ? i2_len :  len;
    }
	  pkt->icmp_cksum = ping_chksum((unsigned short *) pkt, len);

    pingsock = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);

    ret = sendto(pingsock, packet, len, 0,
                (struct sockaddr *) pingServeraddr, sendLen);

    /* free pingServeraddr if the memory allocate here*/
    if (needFree)
    {
        free(pingServeraddr);
    }else{
        freeaddrinfo(pingaddrinfo);
    }

    if (ret < 0)
    {
      close(pingsock);
      notify_fn(dlnaPingHandle, -1);
      return;
    }

    /* wait to listen for replies */
    if (wait_reply_timeout(pingsock, i2_wait) == 0)
    {
      dbg_mw_printf("socket timeout\n");
      close(pingsock);
      notify_fn(dlnaPingHandle, -1);
      return;
    }
    else
    {
      struct sockaddr_in replyAddr;
      socklen_t replylen = sizeof(struct sockaddr_in);
      INT32 recvSize = 60 * 1024; // in case of many replies

      setsockopt(pingsock, SOL_SOCKET, SO_RCVBUF, &recvSize, sizeof(recvSize));

      ret = recvfrom(pingsock, packet, sizeof(packet), 0,
                     (struct sockaddr *) &replyAddr, &replylen);

      if (ret < 0)
      {
        dbg_mw_printf("recvfrom error\n");
        close(pingsock);
        notify_fn(dlnaPingHandle, -1);
        return;
      }

      // unpack as the icmp packet
      if (ret >= IcmpMaxLen) /* ip + icmp */
      {
        struct iphdr *iphdr = (struct iphdr *) packet;
        pkt = (struct icmp *) (packet + (iphdr->ihl << 2));	/* skip ip hdr */
      }
      else
      {
        pkt = (struct icmp *) packet;
      }

      // check the pid
      if (pkt->icmp_id != ping_pid)
      {
        dbg_mw_printf("reply id %d is wrong with %d, recv again.\n", pkt->icmp_id, ping_pid);
        close(pingsock);
        notify_fn(dlnaPingHandle, -1);
        return;
      }

      if (pkt->icmp_type == ICMP_ECHOREPLY)
      {
        struct timeval tmRecvPing = {0};
        struct timeval * tmSendPing = (struct timeval *)pkt->icmp_data;
        gettimeofday(&tmRecvPing, NULL);

        if (tmRecvPing.tv_usec < tmSendPing->tv_usec)
        {
          tmRecvPing.tv_sec --;
          tmRecvPing.tv_usec += 1000000;
        }

        tmRecvPing.tv_sec  -= tmSendPing->tv_sec;
        tmRecvPing.tv_usec -= tmSendPing->tv_usec;
        diff = (tmRecvPing.tv_sec * 1000) + (tmRecvPing.tv_usec / 1000);
        close(pingsock);
        notify_fn(dlnaPingHandle, diff);
      }
    }
    close(pingsock);
    return;
}

/**
 * c_net_socket_nonblocking
 *
 * configure socket as nonblocking mode
 *
 * @param sock, socket to config
 *
 * @return int
 * 0: ok
 * -1: failed
 */
int c_net_socket_nonblocking(int sock)
{
    int flags;

    dbg_mw_printf("set sock: %d nonblocking\n", sock);
    flags= fcntl(sock, F_GETFL, 0);
    if (fcntl(sock, F_SETFL, flags | O_NONBLOCK) != 0)
    {
        dbg_mw_printf("Set socket non-blocking failed, err : %s\n", strerror(errno));
        return NET_FAILED;
    }
    return NET_OK;
}

/**
 * c_net_socket_blocking
 *
 * configure socket as blocking mode
 *
 * @param sock, socket to config
 *
 * @return int
 * 0: ok
 * -1: failed
 */
int c_net_socket_blocking(int sock)
{
    int flags;

    dbg_mw_printf("set sock: %d nonblocking\n", sock);
    flags = fcntl(sock, F_GETFL, 0);
    flags &= (~O_NONBLOCK);
    if (fcntl(sock, F_SETFL, flags) != 0)
    {
        dbg_mw_printf("Set socket blocking failed, err : %s\n", strerror(errno));
        return NET_FAILED;
    }
    return NET_OK;
}

/**
 * c_net_get_socket_mode
 *
 * query current socket mode
 *
 * @param sock
 *
 * @return int
 * 0: blocking
 * 1: nonblocking
 */
int c_net_get_socket_mode(int sock)
{
    return fcntl(sock, F_GETFL, 0);
}

/**
 * NIC API for Application
 */
INT32 c_net_ni_enable(CHAR *psz_name, INT8 u1EnableNi)
{
  char *argp [1];
  if (u1EnableNi == 0)
  {
    argp [0] = (char *)"n";
  }
  else
  {
    argp [0] = (char *)"y";
  }
  if (netinfd_nienable((char *)psz_name, argp [0]) < 0)
  {
    return NET_FAILED;
  }
  return NET_OK;
}

INT32 c_net_ni_get(CHAR *ps_niName, INT32 i4_cmd, VOID *pv_param)
{
    return x_net_ni_get(ps_niName, i4_cmd, pv_param);
}

INT32 c_net_ni_set(CHAR *ps_niName, INT32 i4_cmd, VOID *pv_param)
{
    return x_net_ni_set(ps_niName, i4_cmd, pv_param);
}


INT32 c_net_ni_set_mac(CHAR *psz_name, CHAR *pc_mac)
{
    int sockfd;
    struct ifreq ifr;

    c_net_ni_enable(psz_name, 0);

    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    bzero(&ifr, sizeof(ifr));
    strncpy(ifr.ifr_name, psz_name, 16);
    memcpy(&ifr.ifr_hwaddr.sa_data, pc_mac, 6);
    ifr.ifr_hwaddr.sa_family = ARPHRD_ETHER;

    if (ioctl(sockfd, SIOCSIFHWADDR, &ifr) < 0)
    {
        dbg_mw_printf("SIOCSIFHWADDR ioctrl Connect failed, err : %s\n", strerror(errno));
        close (sockfd);
        if (!c_net_ni_enable(psz_name, 1))
          return NET_FAILED;
        return NET_FAILED;
    }

    if (c_net_ni_enable(psz_name, 1) < 0)
    {
      dbg_mw_printf("%s enable failed\n", psz_name);
      close (sockfd);
      return NET_FAILED;
    }
    close (sockfd);

    return NET_OK;
}

INT32 c_net_ni_get_mac(CHAR *psz_name, CHAR *pac_mac)
{
    int sockfd;
    struct ifreq ifr;

    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    bzero(&ifr, sizeof(ifr));
    strncpy(ifr.ifr_name, psz_name, 16);

    if (ioctl(sockfd, SIOCGIFHWADDR, &ifr) < 0)
    {
        dbg_mw_printf("SIOCGIFHWADDR ioctrl Connect failed, err : %s\n", strerror(errno));
        close (sockfd);
        return NET_FAILED;
    }
    close (sockfd);
    memcpy(pac_mac, ifr.ifr_hwaddr.sa_data, 6);
    return NET_OK;
}

INT32 c_net_ni_set_speed(CHAR *psz_name, NI_SPEED_T ni_speed)
{
    int sockfd;
    struct ifreq ifr;
    struct ioctl_eth_mac_cmd *mac_cmd = NULL;

    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    bzero(&ifr, sizeof(ifr));
    strncpy(ifr.ifr_name, psz_name, strlen(psz_name));

    //if expend more speed define, add mapping to each kind of speed type
    mac_cmd = (struct ioctl_eth_mac_cmd *)&ifr.ifr_data;
    mac_cmd->eth_cmd = ni_speed;

    dbg_mw_printf("Ready to set net speed by ioctl SIOC_SET_SPEED_CMD.\n");
    if (ioctl(sockfd, SIOC_SET_SPEED_CMD, &ifr) < 0)
    {
        dbg_mw_printf("SIOC_SET_SPEED_CMD ioctrl command failed, err : %s\n", strerror(errno));
        close (sockfd);
        return NET_FAILED;
    }
    dbg_mw_printf("ioctl SIOC_SET_SPEED_CMD command finished.\n");
    close (sockfd);
    return NET_OK;
}


INT32 c_net_ni_get_speed_duplex(CHAR *psz_name, NI_SPEED_T *pni_speed)
{
    int sockfd;
    struct ifreq ifr;
    struct ioctl_eth_mac_cmd *mac_cmd = NULL;

    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    bzero(&ifr, sizeof(ifr));
    strncpy(ifr.ifr_name, psz_name, strlen(psz_name));

    //if expend more speed define, add mapping to each kind of speed type
    mac_cmd = (struct ioctl_eth_mac_cmd *)&ifr.ifr_data;
    mac_cmd->eth_cmd = *pni_speed;

    dbg_mw_printf("Ready to get net speed and duplex by ioctl SIOC_GET_SPEED_DUPLEX_CMD.\n");
    if (ioctl(sockfd, SIOC_GET_SPEED_DUPLEX_CMD, &ifr) < 0)
    {
        dbg_mw_printf("SIOC_GET_SPEED_DUPLEX_CMD ioctrl command failed, err : %s\n", strerror(errno));
        close (sockfd);
        return NET_FAILED;
    }
    dbg_mw_printf("ioctl SIOC_GET_SPEED_DUPLEX_CMD command finished, speed and duplex is %d.\n", mac_cmd->eth_cmd);
    close (sockfd);
    return NET_OK;
}


static NI_LX_MONITOR_LIST_T* find_monitor_item_by_evt_type(CHAR *ps_ni_name, NI_DRV_EV_T ev_t)
{
  CRIT_STATE_T t_crit;

  t_crit = u_crit_start();
  NI_LX_MONITOR_LIST_T *pt_new_item = pt_lx_ni_monitor_list_root;

  while (pt_new_item)
  {
      if (pt_new_item->e_event == ev_t &&
          (strcmp(pt_new_item->sz_NifName, ps_ni_name) == 0))
      {
          u_crit_end(t_crit);
          return pt_new_item;
      }
      pt_new_item = pt_new_item->pt_next;
  }
  u_crit_end(t_crit);
  return NULL;
}


static NI_LX_MONITOR_LIST_T* find_inactive_monitor_item_by_evt_type(CHAR *ps_ni_name, NI_DRV_EV_T ev_t)
{
  CRIT_STATE_T t_crit;

  t_crit = u_crit_start();
  NI_LX_MONITOR_LIST_T *pt_new_item = pt_lx_ni_monitor_list_root;

  while (pt_new_item)
  {
      if (pt_new_item->e_event == ev_t &&
          (strcmp(pt_new_item->sz_NifName, ps_ni_name) == 0) &&
          pt_new_item->fgNeedNotify == FALSE)
      {
          u_crit_end(t_crit);
          return pt_new_item;
      }
      pt_new_item = pt_new_item->pt_next;
  }
  u_crit_end(t_crit);
  return NULL;
}

int s_IsNtpSuccess = 0;
void _net_ntp_cb(NTP_EV_T rt)
{
	printf("[MW]Time sysnc result : %d", rt);
	if(rt == NTP_SYNC_TIME_SUCCESS)
		s_IsNtpSuccess = 1;
}	


static VOID ni_mon_thread(VOID *nothing)
{
    CRIT_STATE_T   t_crit;
    int sockfd, sleep;
    struct ifreq ifr;

    do
    {
        t_crit = u_crit_start();
        NI_LX_MONITOR_LIST_T *pt_new_item = pt_lx_ni_monitor_list_root;
        NI_LX_MONITOR_LIST_T *pt_chg_item = NULL;
        sleep = 500;

        while (pt_new_item)
        {
          if (pt_new_item->fgNeedNotify)
          {
            switch (pt_new_item->e_event)
            {
              case NI_DRV_EV_ETHERNET_PLUGIN:
                 sockfd = socket(AF_INET, SOCK_DGRAM, 0);
                 bzero(&ifr, sizeof(ifr));
                 strncpy(ifr.ifr_name, pt_new_item->sz_NifName, 16);

                 if (ioctl(sockfd, SIOCGIFFLAGS, &ifr)<0)
                 {
                     //dbg_mw_printf("ioctl flag failed :%s\n", pt_new_item->sz_NifName);
                     close (sockfd);
                     break;
                 }
                 if (ifr.ifr_flags & IFF_RUNNING) /* interface running and carrier ok */
                 {
                    pt_new_item->pf_notify(pt_new_item->e_event);
                    pt_new_item->fgNeedNotify = FALSE;
                    dbg_mw_printf("<ni_mon_thread> %s is plug!\n", pt_new_item->sz_NifName);

                    #if MW_SUPPORT_ADDON_HOA
                    //HOA_ADDON_Notification
                    //SUB_AOI_ADDONHOST_SendEventToApp(SUB_HOST_EVT_NETWORK_DISCONNECTED, 0);
                    if (SUB_AOI_ADDONHOST_SendEventToApp_mw_fn != NULL)
                    {
                        dbg_mw_printf("[ni_mon_thread]Send SUB_HOST_EVT_NETWORK_CONNECTED event.\n");
                        (* SUB_AOI_ADDONHOST_SendEventToApp_mw_fn) (SUB_HOST_EVT_NETWORK_CONNECTED, 0);
                    }
                    #elif MW_SUPPORT_APP_FRAMEWORK_HOA
                    if (SUB_AOI_ADDONHOST_SendEventToApp_mw_fn != NULL)
                    {
                        dbg_mw_printf("[ni_mon_thread]Send SUB_HOA_HOST_EVT_NETWORK_CONNECTED event 111.\n");
                        (* SUB_AOI_ADDONHOST_SendEventToApp_mw_fn) (SUB_HOA_HOST_EVT_NETWORK_CONNECTED, 0);
                    }
                    #endif  


                    if ((pt_chg_item = find_monitor_item_by_evt_type(pt_new_item->sz_NifName, NI_DRV_EV_ETHERNET_UNPLUG)) != NULL)
                    {
                      //workaround for waiting for unstable ethernet link status
                      sleep = sleep + 1500;
                      pt_chg_item->fgNeedNotify = TRUE;
                    }
                 }

                 close (sockfd);

              break;

              case NI_DRV_EV_ETHERNET_UNPLUG:
                 sockfd = socket(AF_INET, SOCK_DGRAM, 0);
                 bzero(&ifr, sizeof(ifr));
                 strncpy(ifr.ifr_name, pt_new_item->sz_NifName, 16);

                 if (ioctl(sockfd, SIOCGIFFLAGS, &ifr) < 0)
                 {
                     dbg_mw_printf("ioctl flag failed\n");
                     close (sockfd);
                     break;
                 }
                 if (!(ifr.ifr_flags & IFF_RUNNING))
                 {
                    pt_new_item->pf_notify(pt_new_item->e_event);
                    pt_new_item->fgNeedNotify = FALSE;
                    dbg_mw_printf("<ni_mon_thread> %s is unplug!\n", pt_new_item->sz_NifName);


                    #if MW_SUPPORT_ADDON_HOA
                    //HOA_ADDON_Notification
                    //SUB_AOI_ADDONHOST_SendEventToApp(SUB_HOST_EVT_NETWORK_DISCONNECTED, 0);
                    if (SUB_AOI_ADDONHOST_SendEventToApp_mw_fn != NULL)
                    {
                        dbg_mw_printf("[ni_mon_thread]Send SUB_HOST_EVT_NETWORK_DISCONNECTED event.\n");
                        (* SUB_AOI_ADDONHOST_SendEventToApp_mw_fn) (SUB_HOST_EVT_NETWORK_DISCONNECTED, 0);
                    }
                    #elif MW_SUPPORT_APP_FRAMEWORK_HOA
                    if (SUB_AOI_ADDONHOST_SendEventToApp_mw_fn != NULL)
                    {
                        dbg_mw_printf("[ni_mon_thread]Send SUB_HOA_HOST_EVT_NETWORK_DISCONNECTED event 111.\n");
                        (* SUB_AOI_ADDONHOST_SendEventToApp_mw_fn) (SUB_HOA_HOST_EVT_NETWORK_DISCONNECTED, 0);
                    }
                    #endif  

                    while ((pt_chg_item = find_inactive_monitor_item_by_evt_type(pt_new_item->sz_NifName, NI_DRV_EV_ETHERNET_PLUGIN)) != NULL)
                    {
                      pt_chg_item->fgNeedNotify = TRUE;
                    }
                 }

                 close (sockfd);
              break;

              case NI_DRV_EV_IP_ADDRESS_CHANGED:
                  if (pt_new_item->pf_notify != NULL)
                  {
                    pt_new_item->pf_notify(pt_new_item->e_event);
                  }
                  pt_new_item->fgNeedNotify = FALSE;
                  dbg_mw_printf("%s 's IP Address is changed !\n", pt_new_item->sz_NifName);
				  //if(!s_IsNtpSuccess)  ---for playing BD disc issue.---
				  //	c_ntp_sync_time(NULL, _net_ntp_cb);
                  #if MW_SUPPORT_ADDON_HOA
                  //HOA_ADDON_Notification
                  //SUB_AOI_ADDONHOST_SendEventToApp(SUB_HOST_EVT_NETWORK_SETTINGCHANGED, 0);
                  if (SUB_AOI_ADDONHOST_SendEventToApp_mw_fn != NULL)
                  {
                      dbg_mw_printf("[HOA]Send SUB_HOST_EVT_NETWORK_SETTINGCHANGED event.\n");
                      (* SUB_AOI_ADDONHOST_SendEventToApp_mw_fn) (SUB_HOST_EVT_NETWORK_SETTINGCHANGED, 0);
                  }
                  #endif
              break;
			  case NI_DRV_EV_IP_ADDRESSv6_CHANGED:
				  if (pt_new_item->pf_notify != NULL)
				  {
					pt_new_item->pf_notify(pt_new_item->e_event);
				  }
				  pt_new_item->fgNeedNotify = FALSE;
				  dbg_mw_printf("%s 's IPv6 Address is changed !\n", pt_new_item->sz_NifName);
			  break;

              default:
              break;
            }
          }
          pt_new_item = pt_new_item->pt_next;

        }
        u_crit_end(t_crit);
        u_thread_delay(sleep);
    } while (1);
}


INT32 c_net_ni_reg_ev_notify(CHAR *ps_ni_name, NI_DRV_EV_T e_ev, ni_ev_notify_fnct pf_notify)
{
    CRIT_STATE_T   t_crit;
    INT32          i4_status;

    if (!fg_NiMonitor_Thrd_Create)
    {
      // Create the polling thread first
      i4_status = u_thread_create (&h_ni_monitor_thread,
                          NI_MONITOR_THREAD_NAME,
                          NI_MONITOR_THREAD_STACK_SIZE,
                          NI_MONITOR_THREAD_PRIORITY,
                          ni_mon_thread,
                          0,
                          NULL);

      if (i4_status != OSR_OK)
      {
          /* Bad news, some other error. Well, simply return a NULL_HANDLE. */
          dbg_mw_printf("Create NI polling thread failed, ret = %d\n", (int)i4_status);
          return NET_FAILED;
      }
      fg_NiMonitor_Thrd_Create = TRUE;
    }

    NI_LX_MONITOR_LIST_T *pt_new_item = pt_lx_ni_monitor_list_root;

    /* search current list for this new item */
    t_crit = u_crit_start();
    while (pt_new_item)
    {
        if (pt_new_item->e_event == e_ev &&
            (strcmp(pt_new_item->sz_NifName, ps_ni_name) == 0) &&
            pt_new_item->pf_notify == pf_notify)
        {
            u_crit_end(t_crit);
            return NET_NI_OK; /* already exist */
        }
        pt_new_item = pt_new_item->pt_next;
    }
    u_crit_end(t_crit);

    /* allocate memory for this new item */
    pt_new_item = malloc(sizeof(NI_LX_MONITOR_LIST_T));

    if (pt_new_item)
    {
        /* assign new item */
        pt_new_item->e_event = e_ev;
        strncpy(pt_new_item->sz_NifName, ps_ni_name, NIFNAMELEN);
        pt_new_item->pf_notify = pf_notify;
        pt_new_item->fgNeedNotify = TRUE;
        pt_new_item->pt_next = NULL;
        if (pt_new_item->e_event == NI_DRV_EV_IP_ADDRESS_CHANGED)
        { //avoid too early ip change notify
          pt_new_item->fgNeedNotify = FALSE;
        }
        if (pt_new_item->e_event == NI_DRV_EV_ETHERNET_UNPLUG)
        { //avoid too early unplug notify
          pt_new_item->fgNeedNotify = FALSE;
        }

        /* inseart this new item to the list */
        t_crit = u_crit_start();

        if (pt_lx_ni_monitor_list_root)
        {
            pt_new_item->pt_next = pt_lx_ni_monitor_list_root;
        }
        pt_lx_ni_monitor_list_root = pt_new_item;

        u_crit_end(t_crit);
    }
    else
    {
      return NET_NI_ALLOC_MEM_FAILED;
    }
    return NET_OK;
}

INT32 c_net_ni_unreg_ev_notify(CHAR *ps_ni_name, NI_DRV_EV_T e_ev, ni_ev_notify_fnct pf_notify)
{
    if (fg_NiMonitor_Thrd_Create)
    {
      CRIT_STATE_T t_crit;
      NI_LX_MONITOR_LIST_T *pt_del_item = pt_lx_ni_monitor_list_root;
      NI_LX_MONITOR_LIST_T *pt_tmp_item = NULL;
      BOOL fg_Found = FALSE;

      /* search current list for this new item */
      t_crit = u_crit_start();
      while (pt_del_item)
      {
          if (pt_del_item->e_event == e_ev &&
              (strcmp(pt_del_item->sz_NifName, ps_ni_name) == 0) &&
              pt_del_item->pf_notify == pf_notify)
          {
              fg_Found = TRUE;
              break;
          }
          pt_tmp_item = pt_del_item;
          pt_del_item = pt_del_item->pt_next;
      }

      if (fg_Found)
      {
          /* del item locate at front */
          if (pt_del_item == pt_lx_ni_monitor_list_root)
          {
              pt_lx_ni_monitor_list_root = pt_lx_ni_monitor_list_root->pt_next;
          }
          else
          {
              pt_tmp_item->pt_next = pt_del_item->pt_next;
          }
      }
      u_crit_end(t_crit);

      if (fg_Found)
      {
          free(pt_del_item);
      }
    }
    return NET_OK;
}

INT32 c_net_ni_ev_send(NET_NI_T *pt_ni, NI_DRV_EV_T e_ev)
{
    return NET_OK;
}

#if CONFIG_SUPPORT_NET_MPTOOL

static VOID fe_upgrade_progress(HANDLE_T      h_ioc,
                                VOID          *pv_tag,
                                VOID          *pv_data,
                                UINT32        ui4_data_sz,
                                DM_IOC_COND_T t_ioc_cond,
                                UINT32        ui4_prg)
{
    i4_fe_progress = ui4_prg;

    return;
}

static INT32 front_end_upgrade(const UINT8 * fe, INT32 len)
{
    INT32   i4_ret = 0;

    DM_DEV_SPEC_T t_spec;
    HANDLE_T h_ldr, h_ioc;

    DM_DEV_ODD_UPGRADE_T    upg_odd_t;

    t_spec.t_hw_type   = DM_HW_TYPE_IDE;
    t_spec.t_dev_type  = DM_DEV_SKT_TYPE_OPTI_DRV;
    t_spec.ui4_unit_id = 0;

    i4_ret = c_dm_get_device(&t_spec, &h_ldr);

    if (i4_ret != 0)
    {
        dbg_mw_printf("[MPServer]front end upgrade error @ file %s\tline %d\n", __FILE__, __LINE__);
        dbg_mw_printf("[MPServer]get dev return %d\n", i4_ret);

        return  i4_ret;
    }

    upg_odd_t.pv_buf      = (VOID*)fe;
    upg_odd_t.ui4_buf_len = len;

    i4_ret  =   c_dm_odd_upgrade(h_ldr,
                                 &(upg_odd_t),
                                 NULL,
                                 &(fe_upgrade_progress),
                                 &h_ioc);

    if (i4_ret != 0)
    {
        dbg_mw_printf("[MPServer]front end upgrade error @ file %s\tline %d\n", __FILE__, __LINE__);
        dbg_mw_printf("[MPServer]odd upgrade return %d\n", i4_ret);
    }

    //block until write finish
    while (i4_fe_progress < 100)   ;

    return  i4_ret;

}
#endif

static VOID mptool_server_thread(VOID *server_notify_fn)
{
#if CONFIG_SUPPORT_NET_MPTOOL

    struct sockaddr_storage their_addr;
    socklen_t addr_size;
    struct addrinfo hints, *res;
    INT32 sockfd, new_fd, ret, yes = 1, numbytes, svr_status = 1;
    INT32 cmd, cmd_idx, len_idx;
    char buf[100], *ack;
    UINT16 i2_checksum, i2_serv_checksum;
    //UINT8 u8KeyStatus;
    VOID (*fp)(int);

    UINT8 * keyblock = NULL;
    UINT8 * mac      = NULL;
    UINT8 * optfile  = NULL;
#if CONFIG_MW_CUSTOM_KLG
    UINT8 * meqfile = NULL;
    INT32 meqfile_len = 0; 
#endif
    UINT8 * fe       = NULL;
    UINT8 * str_time = NULL;
    UINT8 * cust_data = NULL;
    INT32 kb_len = 0, mac_len = 0, optfile_len = 0, fe_len = 0, time_len = 0, cust_data_len = 0;

    long long system_time = 0;
    CHAR timeStamp[17] = {0};
    struct tm *temptm;
    time_t temptime;

	/** for BE upgrade **/
#if CONFIG_SUPPORT_BE_IN_DRAM
    UCHAR * be_file_part = NULL;
    INT32 be_part_len = 0, recv_len;
#else
    FILE * be_file = NULL;
    CHAR * be_path = "/mnt/nand_06_0/upg.bin";
    CHAR * be_mnt_path = "/mnt/nand_06_0";
    CHAR * be_file_path = "/upg.bin";
    UCHAR * be_file_part = NULL;
    INT32 be_part_len = 0, recv_len;
#endif
    #if (CONFIG_MW_CUSTOM_KLG && CONFIG_MW_SUPPORT_MR)
	INT32 i4_revtimeout = 600000;//Add by suma 2014/02/20
    #endif



#if CONFIG_MW_CUSTOM_SFLP
    UINT32 ip_addr=0;
    INT32  i4_ret=0;
    MT_DHCP4_INFO_T t_ip_info={0};
    UCHAR uc_ip_addr[32]={0};

	mpserver_set_pctool_upgrade(TRUE);
	
    while(1)
    {
    	
        i4_ret=c_dhcpc_get_info(NI_ETHERNET_0,&t_ip_info);
        if(NET_OK!=i4_ret)
        {
            u_thread_delay(1000);
            dbg_mw_printf("[PCTool]Delay 1s for couldn't get ip now.\n");
        }
        else
        {
            ip_addr = htonl(t_ip_info.ui4_ipaddr);

            snprintf((CHAR*)uc_ip_addr,
               16,
               "%d.%d.%d.%d",
               (ip_addr&0xFF000000)>>24,
               (ip_addr&0xFF0000)>>16,
               (ip_addr&0xFF00)>>8,
               ip_addr&0xFF);
        
            if(x_strstr((CHAR *)uc_ip_addr,"169.254")!=NULL)
            {
                dbg_mw_printf("[PCTool]current ip is 169.254.so break loop.\n");
                break;
            }
            else
            {
                u_thread_delay(1000);
                dbg_mw_printf("[PCTool]current ip is not auto ip.so continue loop.\n");
            }
        }
    }
	
    i4_ret=c_net_ip_config("eth0",(UINT32)inet_addr("192.168.0.2"),(UINT32)inet_addr("255.255.255.0"),(UINT32)inet_addr("192.168.0.1"));
    if (i4_ret != NET_OK)
    {
        dbg_mw_printf("[PCTool]ip config failed! error code=%d \n",i4_ret);
        return;
    }
#endif
    fp = (VOID *)(*((int *)server_notify_fn));

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    getaddrinfo(NULL, MPSPORT, &hints, &res);

    sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (-1 == sockfd)
    {
        dbg_mw_printf("[PCTool]socket");
        //strerror
        u_thread_exit();
    }

    ret = setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));
    if (-1 == ret)
    {
        dbg_mw_printf("[PCTool]setsockopt");
    }

    //Add by suma 2014/02/20
    #if (CONFIG_MW_CUSTOM_KLG && CONFIG_MW_SUPPORT_MR)
    ret = setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &i4_revtimeout, sizeof(int));
    if (-1 == ret)
    {
	    perror("setsockopt");
    }
    #endif

    ret = bind(sockfd, res->ai_addr, res->ai_addrlen);
    if (-1 == ret)
    {
        dbg_mw_printf("[PCTool]bind");
        u_thread_exit();
    }
    ret = listen(sockfd, BACKLOG);
    if (-1 == ret)
    {
        dbg_mw_printf("[PCTool]listen");
        u_thread_exit();
    }

    freeaddrinfo(res);

    dbg_mw_printf("[PCTool]Waiting for MPTool...\n");

    while(svr_status){

        addr_size = sizeof(their_addr);
        new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &addr_size);
        if (-1 == new_fd)
        {
            dbg_mw_printf("[PCTool]accept");
            u_thread_exit();
        }

        dbg_mw_printf("[PCTool]Connection accept, fd= %d \n", new_fd);
        (*fp)(0);

        while(1){
            numbytes = recv(new_fd, buf, 100-1, 0);
            dbg_mw_printf("[PCTool]numbytes: %d \n", numbytes);
            if (-1 == numbytes)
            {
                dbg_mw_printf("[PCTool]recv");
                 /* Using perror() here result in an Invalid Memory Reference error
                                 * sometimes, the reason is under investigation.
                                 */
                 //perror("recv");
                dbg_mw_printf("recv: %s\n", strerror(errno));
                break;
            }
            if(numbytes)
            {
                buf[numbytes] = '\0';
                dbg_mw_printf("[PCTool]recieve: %s\n", buf);
                if (!strncmp("CMD:", (char *)&buf, 4))
                {
                    for (cmd_idx = 4; cmd_idx <numbytes; cmd_idx++)
                    {
                        if(',' == buf[cmd_idx])
                        {
                            buf[cmd_idx] = '\0';
                            break;
                        }
                    }
                    cmd = atoi(&buf[4]);

                    switch(cmd){

                    case MPTOOL_SYNC_TIME:
                        dbg_mw_printf("syncServerTime cmd\n");

                        dbg_mw_printf("Initial time buff\n");
                        for (len_idx = cmd_idx + 1; len_idx < numbytes; len_idx++)
                        {
                            if('#' == buf[len_idx])
                            {
                                buf[len_idx]='\0';
                                break;
                            }
                        }
                        time_len = atoi(&buf[cmd_idx+1]);
                        i2_checksum = atoi(&buf[len_idx+1]);
                        if (str_time != NULL) free(str_time);
                        str_time = (UINT8 *)malloc(time_len);
                        if (NULL == str_time)
                        {
                            dbg_mw_printf("time buffer malloc fail, size: %d.\n", time_len);
                            u_thread_exit();
                        }
                        dbg_mw_printf("time buffer initial sucess, size: %d\n", time_len);

                        dbg_mw_printf("Send OK back to client\n");
                        numbytes = send(new_fd, "OK", 3, 0);
                        if (-1 == numbytes)
                        {
                            perror("send ok");
                        }

                        dbg_mw_printf("Start to recieve time\n");
                        numbytes = recv(new_fd, str_time, time_len, MSG_WAITALL);
                        if (-1 == numbytes)
                        {
                            perror("recieve time");
                        }

                        i2_serv_checksum = ping_chksum((unsigned short *)str_time, numbytes);
                        if (i2_checksum == i2_serv_checksum)
                        {
                            dbg_mw_printf("time recieved, %d bytes, server checksum: 0x%x\n",
                                        numbytes, i2_serv_checksum);
                            //Set system time
                            system_time = strtoll((CHAR *)str_time, NULL, 10);
                            if (0 == stime((time_t *)&system_time))
                            {
                                ack = "OK";
                            }else{
                                dbg_mw_printf("sync system time error. \n");
                                ack = "RSND";
                            }
                        }else{
                            dbg_mw_printf("checksum fail, receive %d bytes, server checksum: 0x%x, client checksum: 0x%x\n",
                                        numbytes, i2_serv_checksum, i2_checksum);
                            free(str_time);
                            str_time = NULL;
                            ack = "RSND";
                        }

                        dbg_mw_printf("Send Result back to client: %s\n", ack);
                        numbytes = send(new_fd, ack, strlen(ack), 0);
                        if (-1 == numbytes)
                        {
                            perror("send checksum result");
                        }

                        break;

                    case MPTOOL_SEND_KEYBLOCK:
                        dbg_mw_printf("[PCTool]sendKeyBlock cmd\n");

                        dbg_mw_printf("[PCTool]Initial keyblock buff\n");
                        for (len_idx = cmd_idx + 1; len_idx < numbytes; len_idx++)
                        {
                            if('#' == buf[len_idx])
                            {
                                buf[len_idx]='\0';
                                break;
                            }
                        }
                        kb_len = atoi(&buf[cmd_idx+1]);
                        i2_checksum = atoi(&buf[len_idx+1]);
                        if (keyblock != NULL) free(keyblock);
                        keyblock = (UINT8 *)malloc(kb_len);
                        if (NULL == keyblock)
                        {
                            dbg_mw_printf("[PCTool]keyblock buffer malloc fail, size: %d.\n", kb_len);
                            u_thread_exit();
                        }
                        dbg_mw_printf("[PCTool]keyblock buffer initial sucess, size: %d\n", kb_len);

                        dbg_mw_printf("[PCTool]Send OK back to client\n");
                        numbytes = send(new_fd, "OK", 3, 0);
                        if (-1 == numbytes)
                        {
                            dbg_mw_printf("[PCTool]send ok");
                        }

                        dbg_mw_printf("[MPServer]Start to recieve keyblock\n");
                        numbytes = recv(new_fd, keyblock, kb_len, MSG_WAITALL);
                        if (-1 == numbytes)
                        {
                            dbg_mw_printf("[PCTool]recieve keyblock");
                        }

                        i2_serv_checksum = ping_chksum((unsigned short *)keyblock, numbytes);
                        if (i2_checksum == i2_serv_checksum)
                        {
                            dbg_mw_printf("[PCTool]keyblock recieved, %d bytes, server checksum: 0x%x\n",
                                        numbytes, i2_serv_checksum);
                            ack = "OK";
                        }else{
                            dbg_mw_printf("[PCTool]checksum fail, receive %d bytes, server checksum: 0x%x, client checksum: 0x%x\n",
                                        numbytes, i2_serv_checksum, i2_checksum);
                            free(keyblock);
                            keyblock = NULL;
                            ack = "RSND";
                        }

                        dbg_mw_printf("[PCTool]Send Result back to client: %s\n", ack);
                        numbytes = send(new_fd, ack, strlen(ack), 0);
                        if (-1 == numbytes)
                        {
                            dbg_mw_printf("[PCTool]send checksum result");
                        }

                        break;

                    case MPTOOL_INSTALL_KEYBLOCK:

                        dbg_mw_printf("[PCTool]install keyblock cmd\n");

                        temptime = time(NULL);
                        temptm = localtime(&temptime);
                        snprintf(timeStamp, 17, "%04d%02d%02d%02d%02d%02d%02d", temptm->tm_year + 1900, temptm->tm_mon + 1, temptm->tm_mday,
                             temptm->tm_hour, temptm->tm_min, temptm->tm_sec, 0);
                        dbg_mw_printf("timeStamp: %s.\n", timeStamp);

                        if (NULL == keyblock)
                        {
                            dbg_mw_printf("[PCTool]No keyblock in server\n");
                            ack = "NO_KB";
                        }else{

                            dbg_mw_printf("[PCTool]Call x_cpsm_kb_write()\n");

                            if (TRUE == x_cpsm_kb_write(keyblock, kb_len, (UINT8 *)timeStamp))
                            //if (1)
                            {
                                dbg_mw_printf("[PCTool]write key sucess\n");
                                ack = "OK";

                                //cleanup buffer
                                //free(keyblock);
                                //kb_len = 0;

                            }else{
                                dbg_mw_printf("write key fail\n");
                                ack = "W_FAIL";

                            }
                        }

                        dbg_mw_printf("[PCTool]Send Result back to client: %s\n", ack);
                        numbytes = send(new_fd, ack, strlen(ack), 0);
                        if (-1 == numbytes)
                        {
                        dbg_mw_printf("[PCTool]send checksum result\n");
                        }


                        break;

                    case MPTOOL_SEND_MAC:
                        dbg_mw_printf("[PCTool]sendMAC cmd\n");

                        dbg_mw_printf("[PCTool]Initial mac buff\n");
                        for (len_idx = cmd_idx + 1; len_idx < numbytes; len_idx++)
                        {
                            if('#' == buf[len_idx])
                            {
                                buf[len_idx]='\0';
                                break;
                            }
                        }
                        mac_len = atoi(&buf[cmd_idx+1]);
                        i2_checksum = atoi(&buf[len_idx+1]);
                        if (mac != NULL) free(mac);
                        mac = (UINT8 *)malloc(mac_len);
                        if (NULL == mac)
                        {
                            dbg_mw_printf("[PCTool]mac buffer malloc fail, size: %d.\n", mac_len);
                            u_thread_exit();
                        }
                        dbg_mw_printf("[PCTool]mac buffer initial sucess, size: %d\n", mac_len);

                        dbg_mw_printf("[PCTool]Send OK back to client\n");
                        numbytes = send(new_fd, "OK", 3, 0);
                        if (-1 == numbytes)
                        {
                        
                        dbg_mw_printf("[PCTool]send ok\n");
                       //     perror("send ok");
                        }

                        dbg_mw_printf("[PCTool]Start to recieve mac\n");
                        numbytes = recv(new_fd, mac, mac_len, MSG_WAITALL);
                        if (-1 == numbytes)
                        {
                            dbg_mw_printf("[PCTool]recieve mac");
                        }

                        i2_serv_checksum = ping_chksum((unsigned short *)mac, numbytes);
                        if (i2_checksum == i2_serv_checksum)
                        {
                            dbg_mw_printf("MAC recieved, %d bytes, server checksum: 0x%x\n",
                                        numbytes, i2_serv_checksum);
                            dbg_mw_printf("[PCTool]MAC: %02x-%02x-%02x-%02x-%02x-%02x\n",mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
                            ack = "OK";
                        }else{
                            dbg_mw_printf("[PCTool]checksum fail, receive %d bytes, server checksum: 0x%x, client checksum: 0x%x\n",
                                        numbytes, i2_serv_checksum, i2_checksum);
                            free(mac);
                            mac = NULL;
                            ack = "RSND";
                        }

                        dbg_mw_printf("[PCTool]Send Result back to client: %s\n", ack);
                        numbytes = send(new_fd, ack, strlen(ack), 0);
                        if (-1 == numbytes)
                        {
                            dbg_mw_printf("[PCTool]send checksum result");
                        }
                        break;

                    case MPTOOL_INSTALL_MAC:
                        dbg_mw_printf("[PCTool]install mac cmd\n");
                        if (NULL == mac)
                        {
                            dbg_mw_printf("[PCTool]No MAC in server\n");
                            ack = "NO_MAC";
                        }else{

                            dbg_mw_printf("[PCTool]Call c_net_set_mac_to_sys()\n");

                            if (NET_OK == c_net_set_mac_to_sys((CHAR *)mac))
                            //if (1)
                            {
                                dbg_mw_printf("[PCTool]write MAC sucess\n");
                                ack = "OK";

                                //cleanup buffer
                                //free(mac);
                                //mac_len = 0;

                            }else{
                                dbg_mw_printf("[PCTool]write MAC fail\n");
                                ack = "W_FAIL";

                            }
                        }

                        dbg_mw_printf("[PCTool]Send Result back to client: %s\n", ack);
                        numbytes = send(new_fd, ack, strlen(ack), 0);
                        if (-1 == numbytes)
                        {
                            dbg_mw_printf("[PCTool]send checksum result");
                        }


                        break;

                    case MPTOOL_SEND_OPT:
                        dbg_mw_printf("[PCTool]send Option File\n");

                        dbg_mw_printf("[PCTool]Initial optfile buff\n");
                        for (len_idx = cmd_idx + 1; len_idx < numbytes; len_idx++)
                        {
                            if('#' == buf[len_idx])
                            {
                                buf[len_idx]='\0';
                                break;
                            }
                        }
                        optfile_len = atoi(&buf[cmd_idx+1]);
                        i2_checksum = atoi(&buf[len_idx+1]);
                        if (optfile != NULL) free(optfile);
                        optfile = (UINT8 *)malloc(optfile_len);
                        if (NULL == optfile)
                        {
                            dbg_mw_printf("[PCTool]optfile buffer malloc fail, size: %d.\n", optfile_len);
                            u_thread_exit();;
                        }
                        dbg_mw_printf("[PCTool]optfile buffer initial sucess, size: %d\n", optfile_len);

                        dbg_mw_printf("[PCTool]Send OK back to client\n");
                        numbytes = send(new_fd, "OK", 3, 0);
                        if (-1 == numbytes)
                        {
                            dbg_mw_printf("[PCTool]send ok");
                        }

                        dbg_mw_printf("[PCTool]Start to recieve optfile\n");
                        numbytes = recv(new_fd, optfile, optfile_len, MSG_WAITALL);
                        if (-1 == numbytes)
                        {
                            dbg_mw_printf("[PCTool]recieve optfile");
                        }

                        i2_serv_checksum = ping_chksum((unsigned short *)optfile, numbytes);
                        if (i2_checksum == i2_serv_checksum)
                        {
                            dbg_mw_printf("[PCTool]OptFile recieved, %d bytes, server checksum: 0x%x\n",
                                        numbytes, i2_serv_checksum);
                            ack = "OK";
                        }else{
                            dbg_mw_printf("[PCTool]checksum fail, receive %d bytes, server checksum: 0x%x, client checksum: 0x%x\n",
                                        numbytes, i2_serv_checksum, i2_checksum);
                            free(optfile);
                            optfile = NULL;
                            ack = "RSND";
                        }

                        dbg_mw_printf("[PCTool]Send Result back to client: %s\n", ack);
                        numbytes = send(new_fd, ack, strlen(ack), 0);
                        if (-1 == numbytes)
                        {
                            dbg_mw_printf("[PCTool]send checksum result");
                        }
                        break;

                    case MPTOOL_INSTALL_OPT:
                        dbg_mw_printf("[PCTool]install option file\n");
                        if (NULL == optfile)
                        {
                            dbg_mw_printf("[PCTool]No optfile in server\n");
                            ack = "NO_OPTFILE";
                        }else{

                            dbg_mw_printf("[PCTool]Call x_upg_opcode_write()\n");

                            if ( x_upg_opcode_write_fn != NULL)
                            {
                                //if (32 == optfile_len && 0 == x_upg_opcode_write(optfile, optfile_len))
                                if (0 == (* x_upg_opcode_write_fn)(optfile, optfile_len))
                                {
                                    dbg_mw_printf("[PCTool]write OptFile sucess\n");
                                    ack = "OK";

                                    //cleanup buffer
                                    //free(mac);
                                    //mac_len = 0;

                                }else{
                                    dbg_mw_printf("[PCTool]write OptFile fail\n");
                                    ack = "W_FAIL";

                                }
                            }else{
                                dbg_mw_printf("[PCTool]No x_upg_opcode_write_fn registered, write option stopped\n");
                                ack = "W_FAIL";
                            }
                        }

                        dbg_mw_printf("[PCTool]Send Result back to client: %s\n", ack);
                        numbytes = send(new_fd, ack, strlen(ack), 0);
                        if (-1 == numbytes)
                        {
                            dbg_mw_printf("[PCTool]send checksum result");
                        }

                        break;
#if CONFIG_MW_CUSTOM_KLG
#if !CONFIG_MW_SUPPORT_LG_MICROBE
                    case MPTOOL_SEND_MEQ:
                        dbg_mw_printf("send Meq File\n");

                        dbg_mw_printf("Initial meqfile buff\n");
                        for (len_idx = cmd_idx + 1; len_idx < numbytes; len_idx++)
                        {
                            if('#' == buf[len_idx])
                            {
                                buf[len_idx]='\0';
                                break;
                            }
                        }
                        meqfile_len = atoi(&buf[cmd_idx+1]);
                        i2_checksum = atoi(&buf[len_idx+1]);
                        if (meqfile != NULL) free(meqfile);
                        meqfile = (UINT8 *)malloc(meqfile_len);
                        if (NULL == meqfile)
                        {
                            dbg_mw_printf("meqfile buffer malloc fail, size: %d.\n", meqfile_len);
                            u_thread_exit();
                        }
                        dbg_mw_printf("meqfile buffer initial sucess, size: %d\n", meqfile_len);

                        dbg_mw_printf("Send OK back to client\n");
                        numbytes = send(new_fd, "OK", 3, 0);
                        if (-1 == numbytes)
                        {
                            perror("send ok");
                        }

                        dbg_mw_printf("Start to recieve meqfile\n");
                        numbytes = recv(new_fd, meqfile, meqfile_len, MSG_WAITALL);
                        if (-1 == numbytes)
                        {
                            perror("recieve meqfile");
                        }

                        i2_serv_checksum = ping_chksum((unsigned short *)meqfile, numbytes);
                        if (i2_checksum == i2_serv_checksum)
                        {
                            dbg_mw_printf("Meqfile recieved, %d bytes, server checksum: 0x%x\n",
                                        numbytes, i2_serv_checksum);
                            ack = "OK";
                        }else{
                            dbg_mw_printf("checksum fail, receive %d bytes, server checksum: 0x%x, client checksum: 0x%x\n",
                                        numbytes, i2_serv_checksum, i2_checksum);
                            free(meqfile);
                            meqfile = NULL;
                            ack = "RSND";
                        }

                        dbg_mw_printf("Send Result back to client: %s\n", ack);
                        numbytes = send(new_fd, ack, strlen(ack), 0);
                        if (-1 == numbytes)
                        {
                            perror("send checksum result");
                        }
                        break;

                    case MPTOOL_INSTALL_MEQ:
                        dbg_mw_printf("install meq file\n");
                        if (NULL == meqfile)
                        {
                            dbg_mw_printf("No meqfile in server\n");
                            ack = "NO_MEQFILE";
                        }
                        else
                            {

                            dbg_mw_printf("Call x_meq_upg()\n");
                                                     
  
                                if ( FALSE != x_meq_upg(meqfile, meqfile_len) )
                                {
                                    dbg_mw_printf("write MeqFile sucess\n");
                                    ack = "OK";                

                                }else{
                                    dbg_mw_printf("write MeqFile fail\n");
                                    ack = "W_FAIL";

                                }
                            }
                                                 
                        dbg_mw_printf("Send Result back to client: %s\n", ack);
                        numbytes = send(new_fd, ack, strlen(ack), 0);
                        if (-1 == numbytes)
                        {
                            perror("send checksum result");
                        }

                        break;
#endif
#endif
                    case MPTOOL_SEND_FE:
                        dbg_mw_printf("[PCTool]send Front End File\n");

                        dbg_mw_printf("[PCTool]Initial fe buff\n");
                        for (len_idx = cmd_idx + 1; len_idx < numbytes; len_idx++)
                        {
                            if('#' == buf[len_idx])
                            {
                                buf[len_idx]='\0';
                                break;
                            }
                        }
                        fe_len = atoi(&buf[cmd_idx+1]);
                        i2_checksum = atoi(&buf[len_idx+1]);
                        if (fe != NULL) free(fe);
                        fe = (UINT8 *)malloc(fe_len);
                        if (NULL == fe)
                        {
                            dbg_mw_printf("[PCTool]fe buffer malloc fail, size: %d.\n", fe_len);
                            u_thread_exit();;
                        }
                        dbg_mw_printf("[PCTool]fe buffer initial sucess, size: %d\n", fe_len);

                        dbg_mw_printf("[PCTool]Send OK back to client\n");
                        numbytes = send(new_fd, "OK", 3, 0);
                        if (-1 == numbytes)
                        {
                            dbg_mw_printf("[PCTool]send ok");
                        }

                        dbg_mw_printf("[PCTool]Start to recieve optfile\n");
                        numbytes = recv(new_fd, fe, fe_len, MSG_WAITALL);
                        if (-1 == numbytes)
                        {
                            dbg_mw_printf("[PCTool]recieve fe");
                        }

                        i2_serv_checksum = ping_chksum((unsigned short *)fe, numbytes);
                        if (i2_checksum == i2_serv_checksum)
                        {
                            dbg_mw_printf("[PCTool]Front end recieved, %d bytes, server checksum: 0x%x\n",
                                        numbytes, i2_serv_checksum);
                            ack = "OK";
                        }else{
                            dbg_mw_printf("[PCTool]checksum fail, receive %d bytes, server checksum: 0x%x, client checksum: 0x%x\n",
                                        numbytes, i2_serv_checksum, i2_checksum);
                            free(fe);
                            fe = NULL;
                            ack = "RSND";
                        }

                        dbg_mw_printf("[PCTool]Send Result back to client: %s\n", ack);
                        numbytes = send(new_fd, ack, strlen(ack), 0);
                        if (-1 == numbytes)
                        {
                            dbg_mw_printf("[PCTool]send checksum result");
                        }

                        break;

                    case MPTOOL_INSTALL_FE:
                        dbg_mw_printf("[PCTool]install front end\n");
                        if (NULL == fe)
                        {
                            dbg_mw_printf("[PCTool]No fe in server\n");
                            ack = "NO_FE";
                        }else{

                            dbg_mw_printf("[PCTool]Call c_dm_odd_upgrade()\n");

                            if (0 == front_end_upgrade(fe, fe_len))
                            //if (1)
                            {
                                dbg_mw_printf("[PCTool]write FE sucess\n");
                                ack = "OK";

                                //cleanup buffer
                                //free(mac);
                                //mac_len = 0;

                            }else{
                                dbg_mw_printf("[PCTool]write FE fail\n");
                                ack = "W_FAIL";

                            }
                        }

                        dbg_mw_printf("[PCTool]Send Result back to client: %s\n", ack);
                        numbytes = send(new_fd, ack, strlen(ack), 0);
                        if (-1 == numbytes)
                        {
                            dbg_mw_printf("[PCTool]send checksum result");
                        }

                        break;
                    case MPTOOL_CHECK_KEY:
                        dbg_mw_printf("[PCTool]Check WMDRM Key \n");
#if CONFIG_PHILIPS_WMDRM_KEY

                        UINT32 ct_size;

                        if (fgGetPhilipsFrontDataSize((uint32_t *)&ct_size)){
                        ack = "OK";
                        }else{
                            ack = "NO_WMDRM_KEY";
                        }
#else
                        ack = "NO_WMDRM_KEY";
#endif

                        dbg_mw_printf("[PCTool]Send Result back to client: %s\n", ack);
                        numbytes = send(new_fd, ack, strlen(ack), 0);
                        if (-1 == numbytes)
                        {
                            dbg_mw_printf("[PCTool]send checksum result");
                        }

                        break;

                    case MPTOOL_GET_MODEL_NAME:
                        dbg_mw_printf("[PCTool]Get Platform model name \n");
#if CONFIG_PHILIPS_WMDRM_KEY

                        ack = DIVERSITY_S(DIV_STR_DISPLAY_NAME);
                        if (NULL == ack)
                        {
                            ack = "NO_MODEL_NAME";
                        }
                        dbg_mw_printf("[PCTool]Model Name: %s\n", ack);

#else
                        ack = s_hostname;
#endif

                        dbg_mw_printf("[PCTool]Send Result back to client: %s\n", ack);
                        numbytes = send(new_fd, ack, strlen(ack), 0);
                        if (-1 == numbytes)
                        {
                            dbg_mw_printf("[PCTool]send model name");
                        }
                        break;

                    case MPTOOL_SEND_BE_START:
                        dbg_mw_printf("[PCTool]initial be file tranfer\n");

#if CONFIG_SUPPORT_BE_IN_DRAM
                        /* Store BE in DRAM */
                        ack = "OK";
#else
                        /* Store BE in File */
                        //open file for storing
                        if(NULL != be_file) fclose(be_file);

                        be_file = fopen(be_path, "w+b");
                        if (NULL == be_file)
                        {
                            dbg_mw_printf("[PCTool]Create BE file error\n");
                            ack = "BE_FILE_FAIL";
                        }else{
                            ack = "OK";
                        }
#endif


                        dbg_mw_printf("[PCTool]Send Result back to client: %s\n", ack);
                        numbytes = send(new_fd, ack, strlen(ack), 0);
                        if (-1 == numbytes)
                        {
                            dbg_mw_printf("[PCTool]send checksum result");
                        }
                        break;

                    case MPTOOL_SEND_BE:
                        dbg_mw_printf("[PCTool]BD memory clean begin\n");

					#if CONFIG_MW_CUSTOM_SFLP
                        if(g_pctool_upg_fn)
                        {
                            (* g_pctool_upg_fn)();
                        }
                        dbg_mw_printf("[PCTool]BD memory clean finish.\n");  
                        dbg_mw_printf("send BE File\n");

                        dbg_mw_printf("[PCTool]Initial be_file buff\n");
					#endif
					
                        for (len_idx = cmd_idx + 1; len_idx < numbytes; len_idx++)
                        {
                            //printf("len_idx: %d\n", len_idx);
                            if('#' == buf[len_idx])
                            {
                                buf[len_idx]='\0';
                                break;
                            }
                        }
                        be_part_len = atoi(&buf[cmd_idx+1]);
                        i2_checksum = atoi(&buf[len_idx+1]);
                        if (be_file_part != NULL) free(be_file_part);
                        be_file_part = (UCHAR *)malloc(be_part_len);
                        if (NULL == be_file_part)
                        {
                            dbg_mw_printf("[PCTool]be_file_part buffer malloc fail, size: %d.\n", be_part_len);
                            break;
                        }
                        dbg_mw_printf("be_file_part buffer initial sucess, size: %d\n", be_part_len);

                        dbg_mw_printf("Send OK back to client\n");
                        numbytes = send(new_fd, "OK", 3, 0);
                        if (-1 == numbytes)
                        {
                            dbg_mw_printf("[PCTool]send ok");
                        }

                        dbg_mw_printf("Start to recieve be file\n");
                        recv_len = 0;
                        while (recv_len < be_part_len)
                        {
                            numbytes = recv(new_fd, be_file_part + recv_len, be_part_len - recv_len, MSG_WAITALL);
                            if (-1 == numbytes)
                            {
                                dbg_mw_printf("[PCTool]recieve be_file_part");
                            }
                            dbg_mw_printf("numbytes: %d\n", numbytes);
                            recv_len += numbytes;
                        }

                        i2_serv_checksum = ping_chksum((unsigned short *)be_file_part, (int)recv_len);
                        if (i2_checksum == i2_serv_checksum)
                        {
                            dbg_mw_printf("be_file_part recieved, %d bytes, server checksum: 0x%x\n",
                                        recv_len, i2_serv_checksum);
#if CONFIG_SUPPORT_BE_IN_DRAM
                            ack = "OK";

#else
                            //Write be_file_part into file
                            if (NULL == be_file)
                            {
                                ack = "NO_BE_FILE";
                            }else
                            {
                                fwrite(be_file_part, 1, numbytes, be_file);
                                ack = "OK";
                            }
#endif
                        }else{
                            dbg_mw_printf("checksum fail, receive %d bytes, server checksum: 0x%x, client checksum: 0x%x\n",
                                        recv_len, i2_serv_checksum, i2_checksum);
                            free(be_file_part);
                            be_file_part = NULL;
                            ack = "RSND";
                        }

                        dbg_mw_printf("Send Result back to client: %s\n", ack);
                        numbytes = send(new_fd, ack, strlen(ack), 0);
                        if (-1 == numbytes)
                        {
                        
                        dbg_mw_printf("[PCTool]send checksum result\n");
                        }


                        break;

                    case MPTOOL_SEND_BE_END:
                        dbg_mw_printf("finalize be file tranfer\n");
#if CONFIG_SUPPORT_BE_IN_DRAM
                        ack = "OK";

#else
                        //close be file
                        if (NULL == be_file)
                        {
                            ack = "NO_BE_FILE";
                        }else
                        {
                            fclose(be_file);
                            be_file = NULL;
                            ack = "OK";
                        }

                        //Verify BE file
                        if (upg_from_mptool_Step1_fn != NULL)
                        {
                            dbg_mw_printf("[MPServer]Start to verify BE file\n");
                            ret = (* upg_from_mptool_Step1_fn)(be_mnt_path, be_file_path);
                            if ( 0 != ret )
                            {
                                ack = "BE_VER_FAIL";
                            }
                        }else{
                            dbg_mw_printf("[MPServer]No BE verify function registered, upgrade stopped\n");
                            ack = "NO_BE_VER_FN";
                        }
#endif
                        dbg_mw_printf("Send Result back to client: %s\n", ack);
                        numbytes = send(new_fd, ack, strlen(ack), 0);
                        if (-1 == numbytes)
                        {
                        
                        dbg_mw_printf("[PCTool]send checksum result\n");
                            //perror("[MPServer]send checksum result");
                        }

                        break;

                    case MPTOOL_INSTALL_BE:
                        dbg_mw_printf("[MPServer]install be file\n");

#if CONFIG_SUPPORT_BE_IN_DRAM
                        if (upg_from_mptool_step2_fn != NULL)
                        {
                            dbg_mw_printf("[MPServer]Start to write BE into flash\n");
                            if ( NULL != be_file_part && 0 != be_part_len)
                            {
                                ret = (* upg_from_mptool_step2_fn)(be_file_part, be_part_len);
                                if (0 == ret)
                                {
                                    ack = "OK";
                                }else{
                                    ack = "BE_UPG_FAILED";
                                }
                            }else{
                                dbg_mw_printf("[MPServer]No BE exists in DRAM\n");
                                ack = "NO_BE_DRAM";
                            }
                        }else{
                            dbg_mw_printf("[MPServer]No BE upgrade function registered, upgrade stopped\n");
                            ack = "NO_BE_UPG_FN";
                        }

#else

                        if (upg_from_mptool_step2_fn != NULL)
                        {
                            dbg_mw_printf("[MPServer]Start to write BE into flash\n");
                            ret = (* upg_from_mptool_step2_fn)(be_mnt_path, be_file_path);
                            if (0 == ret)
                            {
                                ack = "OK";
                            }else{
                                ack = "BE_UPG_FAILED";
                            }
                        }else{
                            dbg_mw_printf("[MPServer]No BE upgrade function registered, upgrade stopped\n");
                            ack = "NO_BE_UPG_FN";
                        }
#endif

                        dbg_mw_printf("Send Result back to client: %s\n", ack);
                        numbytes = send(new_fd, ack, strlen(ack), 0);
                        if (-1 == numbytes)
                        {
                            perror("send checksum result");
                        }

                        break;

                    case MPTOOL_SEND_CUST_DATA:
                        dbg_mw_printf("Send Custom Data File\n");

                        dbg_mw_printf("Initial cust_data buff\n");
                        for (len_idx = cmd_idx + 1; len_idx < numbytes; len_idx++)
                        {
                            if('#' == buf[len_idx])
                            {
                                buf[len_idx]='\0';
                                break;
                            }
                        }
                        cust_data_len = atoi(&buf[cmd_idx+1]);
                        i2_checksum = atoi(&buf[len_idx+1]);
                        if (cust_data != NULL) free(cust_data);
                        cust_data = (UINT8 *)malloc(cust_data_len);
                        if (NULL == cust_data)
                        {
                            dbg_mw_printf("cust_data buffer malloc fail, size: %d.\n", cust_data_len);
                            u_thread_exit();;
                        }
                        dbg_mw_printf("cust_data buffer initial sucess, size: %d\n", cust_data_len);

                        dbg_mw_printf("Send OK back to client\n");
                        numbytes = send(new_fd, "OK", 3, 0);
                        if (-1 == numbytes)
                        {
                            perror("send ok");
                        }

                        dbg_mw_printf("Start to recieve cust_data\n");
                        numbytes = recv(new_fd, cust_data, cust_data_len, MSG_WAITALL);
                        if (-1 == numbytes)
                        {
                            perror("recieve cust_data");
                        }

                        i2_serv_checksum = ping_chksum((unsigned short *)cust_data, numbytes);
                        if (i2_checksum == i2_serv_checksum)
                        {
                            dbg_mw_printf("cust_data recieved, %d bytes, server checksum: 0x%x\n",
                                        numbytes, i2_serv_checksum);
                            ack = "OK";
                        }else{
                            dbg_mw_printf("checksum fail, receive %d bytes, server checksum: 0x%x, client checksum: 0x%x\n",
                                        numbytes, i2_serv_checksum, i2_checksum);
                            free(cust_data);
                            cust_data = NULL;
                            ack = "RSND";
                        }

                        dbg_mw_printf("Send Result back to client: %s\n", ack);
                        numbytes = send(new_fd, ack, strlen(ack), 0);
                        if (-1 == numbytes)
                        {
                            perror("send checksum result");
                        }

                        break;

                    case MPTOOL_INSTALL_CUST_DATA:
                        dbg_mw_printf("Install Custom Data\n");
                        if (NULL == cust_data)
                        {
                            dbg_mw_printf("No cust_data in server\n");
                            ack = "NO_FE";
                        }else{

                            dbg_mw_printf("Call i4MPDataWrite()\n");

                            if (0 == i4MPDataWrite((char *)cust_data, (unsigned int)cust_data_len))
                            //if (1)
                            {
                                dbg_mw_printf("write cust_data sucess\n");
                                ack = "OK";

                                //cleanup buffer
                                //free(mac);
                                //mac_len = 0;

                            }else{
                                dbg_mw_printf("write cust_data fail\n");
                                ack = "W_FAIL";

                            }
                        }

                        dbg_mw_printf("Send Result back to client: %s\n", ack);
                        numbytes = send(new_fd, ack, strlen(ack), 0);
                        if (-1 == numbytes)
                        {
                            perror("send checksum result");
                        }

                        break;

                    case MPTOOL_CLOSE_SERVER:
#ifdef  CONFIG_AMW_ROBUSTNESS_FINAL_SOLUTION_EN                   	
                    	  i4NFBEnableUart(0);
#endif                    	  
                        dbg_mw_printf("Close Server\n");
                        svr_status = 0;
                        //continue to process MPTOOL_DISCONNECT part, so no break here.

                    case MPTOOL_DISCONNECT:
                        dbg_mw_printf("Disconnect\n");


                        //TODO cleaup all kinds of buffer
                        if (NULL != keyblock)
                        {
                            free(keyblock);
                            kb_len = 0;
                            keyblock = NULL;
                        }
                        if (NULL != mac)
                        {
                            free(mac);
                            mac_len = 0;
                            mac = NULL;
                        }
                        if (NULL != optfile)
                        {
                            free(optfile);
                            optfile_len = 0;
                            optfile = NULL;
                        }
#if CONFIG_MW_CUSTOM_KLG
                         if (NULL != meqfile)
                        {
                            free(meqfile);
                            meqfile_len = 0;
                            meqfile = NULL;
                        }
#endif

                        close(new_fd);
                        dbg_mw_printf("sockfd %d closed\n", new_fd);
                        new_fd = 0;
 #if CONFIG_SUPPORT_SS
 						dbg_mw_printf("Defined Sumsung configure!\n");
                        c_net_reboot();
 #endif
                        break;

                    default:
                        dbg_mw_printf("Unknow cmd: %d\n", cmd);
                        break;
                    }

                    dbg_mw_printf("\n");
                }

            }
            else
            {
                if (0 != new_fd)
                    close(new_fd);
                dbg_mw_printf("inner: sockfd %d closed\n", new_fd);
                break;
            }
        }
        if (0 != new_fd)
            close(new_fd);
        dbg_mw_printf("outter: sockfd %d closed\n", new_fd);
    }
    close(sockfd);
    fg_MPTOOL_SERVER_Thrd_Create = FALSE;
    (*fp)(1);
#endif

    //reset ip to DHCP
    //c_dhcpc_start("eth0", NULL);
    u_thread_exit();
}


INT32 c_net_mptool_server_startup(server_notify_fn cb_fn)
{

    INT32 i4_status;
    //CHAR * ifname = "eth0";
    //CHAR * cmd = "kill `ps | grep \"/sbin/udhcpc -i\" | awk '{print $1}' | head -n 1`";
    //stop DHCP process
    //ipcd_exec(cmd,NULL);

    //Change platform's IP address
    /*
    UINT32 ip      = inet_addr("192.168.0.2");
    UINT32 netmask = inet_addr("255.255.255.0");
    UINT32 gateway = inet_addr("192.168.0.1");
    c_net_ip_config(ifname, ip, netmask, gateway);
    */

    if (!fg_MPTOOL_SERVER_Thrd_Create)
    {
      // Create MPTool Server
      i4_status = u_thread_create (&h_MPTOOL_SERVER_thread,
                          MPTOOL_SERVER_THREAD_NAME,
                          MPTOOL_SERVER_THREAD_STACK_SIZE,
                          MPTOOL_SERVER_THREAD_PRIORITY,
                          mptool_server_thread,
                          sizeof(VOID *),
                          &cb_fn);

      if (i4_status != OSR_OK)
      {
          /* Bad news, some other error. Well, simply return a NULL_HANDLE. */
          dbg_mw_printf("Create MPTool Server thread failed, ret = %d\n", (int)i4_status);
          return NET_FAILED;
      }
      fg_MPTOOL_SERVER_Thrd_Create = TRUE;
    }

    return NET_OK;
}


/**
 * DHCP API for Application
 */
static int g_IsDnsEnable= 1;

INT32 c_dhcpc_en_dns(void)
{
	FILE * fd;
	char *dns_opt = "/var/EnDns.info";

	g_IsDnsEnable = DHCP_EN_DNS;
	
	fd = fopen(dns_opt, "wb");
	if (NULL == fd)
	{
	  printf("<dhcpc>oOpen dns file fail!\n");
	  return -1;
	}
	
	fprintf(fd, "%d", g_IsDnsEnable);
	fclose(fd);
	
	printf("<dhcpc>Now enable dns!\n");
	return 0;	
}

INT32 c_dhcpc_dis_dns(void)
{
	FILE * fd;
	char *dns_opt = "/var/EnDns.info";

	g_IsDnsEnable = DHCP_DIS_DNS;
	
	fd = fopen(dns_opt, "wb");
	if (NULL == fd)
	{
	  printf("<dhcpc>oOpen dns file fail!\n");
	  return -1;
	}

	fprintf(fd, "%d", g_IsDnsEnable);
	fclose(fd);
	printf("<dhcpc>Now disable dns!\n");	
	return 0;
}

INT32 c_dhcpc_get_dns_status(void)
{
	return g_IsDnsEnable;
}



INT32 c_dhcpc_init(VOID)
{
    return NET_OK;
}

INT32 c_dhcpc_deinit(VOID)
{
    return NET_OK;
}

INT32 c_dhcpc_get_info(CHAR *ps_ni_name, MT_DHCP4_INFO_T *pt_info)
{
    pt_info->ui4_ipaddr = 0;
    pt_info->ui4_subnet = 0;
    pt_info->ui4_router = 0;
    pt_info->ui4_dns1 = 0;
    pt_info->ui4_dns2 = 0;

#if  (FORK2NETINFD)
    char key[ip_addr_len] = "IP";
    char *value;
    struct in_addr t_addr;

    if (netinfd_get(key, &value) < 0)
    {
      return NET_OK;
    }
    else
    {
      if (value != NULL)
      {
        dbg_mw_printf("%s      = \"%s\"\n", key, value);
        if (inet_aton(value, &t_addr) == 0)
        {
          dbg_mw_printf ("IP %s is invalid\n", value);
        }
        pt_info->ui4_ipaddr = t_addr.s_addr;
        free(value);
      }
    }

    strncpy(key, "DNS1", 4);
    if (netinfd_get(key, &value) < 0)
    {
      dbg_mw_printf("%s=\n", key);
      return NET_OK;
    }
    else
    {
      if (value != NULL)
      {
        dbg_mw_printf("%s    = \"%s\"\n", key, value);
        if (inet_aton(value, &t_addr) == 0)
        {
          dbg_mw_printf ("DNS1 %s is invalid\n", value);
        }
        pt_info->ui4_dns1 = t_addr.s_addr;
        free(value);
      }
    }

    strncpy(key, "DNS2", 4);
    if (netinfd_get(key, &value) < 0)
    {
      dbg_mw_printf("%s=\n", key);
      return NET_OK;
    }
    else
    {
      if (value != NULL)
      {
        dbg_mw_printf("%s  = \"%s\"\n", key, value);
        if (inet_aton(value, &t_addr) == 0)
        {
          dbg_mw_printf ("DNS2 %s is invalid\n", value);
        }
        pt_info->ui4_dns2 = t_addr.s_addr;
        free(value);
      }
    }

    strncpy(key, "Subnet", 6);
    if (netinfd_get(key, &value) < 0)
    {
      dbg_mw_printf("%s=\n", key);
      return NET_OK;
    }
    else
    {
      if (value != NULL)
      {
        dbg_mw_printf("%s  = \"%s\"\n", key, value);
        if (inet_aton(value, &t_addr) == 0)
        {
          dbg_mw_printf ("Subnet %s is invalid\n", value);
        }
        pt_info->ui4_subnet = t_addr.s_addr;
        free(value);
      }
    }

    strncpy(key, "router", 6);
    if (netinfd_get(key, &value) < 0)
    {
      dbg_mw_printf("%s=\n", key);
      return NET_OK;
    }
    else
    {
      if (value != NULL)
      {
        dbg_mw_printf("%s  = \"%s\"\n", key, value);
        if (inet_aton(value, &t_addr) == 0)
        {
          dbg_mw_printf ("router %s is invalid\n", value);
        }
        pt_info->ui4_router = t_addr.s_addr;
        free(value);
      }
    }

#else

    // After the script done, we need to parse the temp dhcp info file to retrieve info.
    {
	c_net_ni_get_ip(ps_ni_name, &(pt_info->ui4_ipaddr));
	c_net_ni_get_subnet(ps_ni_name, &(pt_info->ui4_subnet));
	c_net_ni_get_gateway(ps_ni_name, &(pt_info->ui4_router));
	c_net_ni_get_dns(&(pt_info->ui4_dns1), &(pt_info->ui4_dns2));
	
    return NET_OK;
    }

#endif //NETINFO_DAEMON
    return NET_OK;
}

static VOID ntp_mon_thread(VOID *arg_dhcp)
{	
	INT32 i4_ret = 0;
    NTP_PARA_T *pt_ntp_para = (NTP_PARA_T *)arg_dhcp;
	i4_ret = system(pt_ntp_para->ac_command_name);
	if (i4_ret == 0)
	{
		fg_ntp_sync_ready = TRUE;
		dbg_mw_printf ("{Network MW}sync success\n");
		if(pt_ntp_para->pf_notify != NULL)
		{			
			pt_ntp_para->pf_notify(IPCD_EXEC_SUCESS);	
		}
	}
	else
	{
		fg_ntp_sync_ready = FALSE;
		dbg_mw_printf ("{Network MW}sync fail\n");
		if(pt_ntp_para->pf_notify != NULL)
		{			
			pt_ntp_para->pf_notify(IPCD_EXEC_FAILED);	
		}
	}	
	u_thread_exit();   
}


static VOID dhcp_mon_thread(VOID *arg_dhcp)
{
    DHCP_LX_MONITOR_T *pt_lx_dhcp_monitor = (DHCP_LX_MONITOR_T *)arg_dhcp;

    do
    {
        if (pt_lx_dhcp_monitor != NULL)
        {
              char *argp [6];
#if (!FORK2NETINFD)
              int pid, wpid, wstatus;
              int ret = 0;
              char *dhcpinfo = "/var/dhcpinfo";
#endif
              char *script = "/sbin/dhcpc.script";

              argp [0] = script;
              argp [1] = (char *)"";

              if (DHCP_START == pt_lx_dhcp_monitor->eDhcpCmd)
              {
                MT_DHCP4_INFO_T t_dhcp_info = {0};
                c_dhcpc_get_info((char *)pt_lx_dhcp_monitor->sz_NifName, &t_dhcp_info);
#if 0
                if (t_dhcp_info.ui4_ipaddr != 0 && t_dhcp_info.ui4_subnet != 0 &&
                    t_dhcp_info.ui4_router != 0 && t_dhcp_info.ui4_dns1 != 0)
                {
                  if (pt_lx_dhcp_monitor->pf_notify != NULL)
                  {
                    pt_lx_dhcp_monitor->pf_notify(DHCPC_EVENT_SUCCESS_DHCPv4);
                  }
                  {
                    NI_LX_MONITOR_LIST_T *pt_new_item = pt_lx_ni_monitor_list_root;

                    /* search current list for this new item */

                    while (pt_new_item)
                    {
                        if (pt_new_item->e_event == NI_DRV_EV_IP_ADDRESS_CHANGED &&
                            (strcmp(pt_new_item->sz_NifName, pt_lx_dhcp_monitor->sz_NifName) == 0) &&
                            pt_new_item->pf_notify != NULL)
                        {
                            pt_new_item->fgNeedNotify = TRUE;
                            break;
                        }
                        pt_new_item = pt_new_item->pt_next;
                    }
                  }
                  fg_dhcp_thread_running = FALSE;
                  u_thread_exit();
                }
#endif
                argp [1] = (char *)"start";
              }
              else if (DHCP_RESTART == pt_lx_dhcp_monitor->eDhcpCmd)
              {
                argp [1] = (char *)"restart";
              }

              argp [2] = (char *)pt_lx_dhcp_monitor->sz_NifName;
#if (FORK2NETINFD)
              if (netinfd_dhcp(argp[1], argp[2]) < 0)
              {
                if ((pt_lx_dhcp_monitor->pf_notify != NULL) && (FALSE == g_dhcp_IsDhcpNeedStop))
                {
                  pt_lx_dhcp_monitor->pf_notify(DHCPC_EVENT_FAILURE_DHCPv4);
                }
              }
              else
              {
                if (pt_lx_dhcp_monitor->pf_notify != NULL)
                {
                  pt_lx_dhcp_monitor->pf_notify(DHCPC_EVENT_SUCCESS_DHCPv4);
                }

                if (DHCP_START == pt_lx_dhcp_monitor->eDhcpCmd ||
                    DHCP_RESTART == pt_lx_dhcp_monitor->eDhcpCmd)
                {
                  NI_LX_MONITOR_LIST_T *pt_new_item = pt_lx_ni_monitor_list_root;

                  /* search current list for this new item */

                  while (pt_new_item)
                  {
                      if (pt_new_item->e_event == NI_DRV_EV_IP_ADDRESS_CHANGED &&
                          (strcmp(pt_new_item->sz_NifName, pt_lx_dhcp_monitor->sz_NifName) == 0) &&
                          pt_new_item->pf_notify != NULL)
                      {
                          pt_new_item->fgNeedNotify = TRUE;
                          break;
                      }
                      pt_new_item = pt_new_item->pt_next;
                  }
                }
              }
              fg_dhcp_thread_running = FALSE;
              u_thread_exit();
#else

              argp [3] = dhcpinfo;

              #if  (NETINFO_DAEMON)
              argp [4] = (char *)"1";
              #else
              argp [4] = (char *)"0";
              #endif
              argp [5] = NULL;

              pid = fork ();
              if (pid < 0)
              {
                dbg_mw_printf ("fork: %d\n", errno);
                wstatus = 0;
              }
              else if (pid)
              { //parent
                do {
                  wpid = wait (&wstatus);
                } while (wpid != pid && wpid > 0);
                dbg_mw_printf ("child %d terminated\n", wpid);
                dbg_mw_printf ("child status : %d\n", WEXITSTATUS(wstatus));

                if (wpid < 0)
                {
                  dbg_mw_printf("wait: %d, err: %s\n", wpid, strerror(errno));
                  wstatus = 0;
                  if (pt_lx_dhcp_monitor->pf_notify != NULL)
                  {
                    pt_lx_dhcp_monitor->pf_notify(DHCPC_EVENT_FAILURE_DHCPv4);
                  }
                }
                else if (wstatus != 0)
                {
                  if (pt_lx_dhcp_monitor->pf_notify != NULL)
                  {
                    pt_lx_dhcp_monitor->pf_notify(DHCPC_EVENT_FAILURE_DHCPv4);
                  }
                }
                else
                {
                  if (pt_lx_dhcp_monitor->pf_notify != NULL)
                  {
                    pt_lx_dhcp_monitor->pf_notify(DHCPC_EVENT_SUCCESS_DHCPv4);
                  }

                  if (DHCP_START == pt_lx_dhcp_monitor->eDhcpCmd ||
                      DHCP_RESTART == pt_lx_dhcp_monitor->eDhcpCmd)
                  {
                    NI_LX_MONITOR_LIST_T *pt_new_item = pt_lx_ni_monitor_list_root;

                    /* search current list for this new item */

                    while (pt_new_item)
                    {
                        if (pt_new_item->e_event == NI_DRV_EV_IP_ADDRESS_CHANGED &&
                            (strcmp(pt_new_item->sz_NifName, pt_lx_dhcp_monitor->sz_NifName) == 0) &&
                            pt_new_item->pf_notify != NULL)
                        {
                            pt_new_item->fgNeedNotify = TRUE;
                            break;
                        }
                        pt_new_item = pt_new_item->pt_next;
                    }
                  }
                }
                fg_dhcp_thread_running = FALSE;
                dbg_mw_printf ("dhcp_mon_thread fg_dhcp_thread_running= %d\n", fg_dhcp_thread_running);
                u_thread_exit();
              }
              else
              { // child
                ret = execv (script, argp);
                dbg_mw_printf ("execve result = %d\n", ret);
                if (ret == -1)
                {
                  dbg_mw_printf("execve errno = %s\n", strerror(errno));
                }
                exit(0);
              }
#endif
        }
        u_thread_delay(5);
    } while (1);
}

INT32 c_dhcpc_start(CHAR *ps_ni_name, x_dhcpc_nfy_fct fn)
{
    CRIT_STATE_T   t_crit;
    INT32          i4_status;
    DHCP_LX_MONITOR_T lx_dhcp_monitor = {{0}};
    INT32   i4_time = 100;
    char cmd[60] = {0};
    
	snprintf(cmd,60, "echo 1 > /proc/sys/net/ipv4/conf/%s/arp_ignore", ps_ni_name);
	system(cmd);

	dbg_mw_printf("[nw]c_dhcpc_start fg_dhcp_thread_running = %d\n", fg_dhcp_thread_running);

	if(fg_dhcp_thread_running)
	{
		sleep(1);
	}

	dbg_mw_printf("[nw]c_dhcpc_start2 fg_dhcp_thread_running = %d\n", fg_dhcp_thread_running);

    t_crit = u_crit_start();

	g_dhcp_IsDhcpNeedStop = FALSE;

    strncpy(lx_dhcp_monitor.sz_NifName, ps_ni_name, NIFNAMELEN);
    lx_dhcp_monitor.pf_notify = fn;
    lx_dhcp_monitor.eDhcpCmd = DHCP_START;

    if (!fg_dhcp_thread_running)
    {
        
        while (i4_time--)
        {
            usleep(10000);           
            // Create the polling thread first
            i4_status = u_thread_create (&h_dhcp_monitor_thread,
                                "DHCPMON",
                                1024,
                                NI_MONITOR_THREAD_PRIORITY,
                                dhcp_mon_thread,
                                sizeof(DHCP_LX_MONITOR_T),
                                (VOID *)&lx_dhcp_monitor);

            if (i4_status != OSR_OK)
            {                
                /* Bad news, some other error. Well, simply return a NULL_HANDLE. */
                dbg_mw_printf("Create DHCP starting thread failed, ret[%d], i4_time[%d]\n", (int)i4_status, i4_time);       		
            }
            else
            {
                fg_dhcp_thread_running = TRUE;
                dbg_mw_printf("[nw]%s:%d:\n", __FUNCTION__, __LINE__);
                break;
            }
        }
    }
    if (i4_time > 0)
    {
        memcpy(&t_lx_dhcp_monitor_g, &lx_dhcp_monitor, sizeof(DHCP_LX_MONITOR_T));
    }

    u_crit_end(t_crit);

    return NET_OK;
}


INT32 c_dhcpc_restart(CHAR *ps_ni_name)
{
    CRIT_STATE_T   t_crit;
    INT32          i4_status;
    DHCP_LX_MONITOR_T lx_dhcp_monitor = {{0}};
    INT32   i4_time = 100;
    char cmd[60] = {0};
    
	snprintf(cmd,60, "echo 1 > /proc/sys/net/ipv4/conf/%s/arp_ignore", ps_ni_name);
	system(cmd);

    t_crit = u_crit_start();

	g_dhcp_IsDhcpNeedStop = FALSE;

    strncpy(lx_dhcp_monitor.sz_NifName, ps_ni_name, NIFNAMELEN);
    lx_dhcp_monitor.pf_notify = t_lx_dhcp_monitor_g.pf_notify;
    lx_dhcp_monitor.eDhcpCmd = DHCP_RESTART;

    if (!fg_dhcp_thread_running)
    {
        while (i4_time--)
    	{
            usleep(10000);
            // Create the polling thread first
            i4_status = u_thread_create (&h_dhcp_monitor_thread,
                                "DHCPMON",
                                1024,
                                NI_MONITOR_THREAD_PRIORITY,
                                dhcp_mon_thread,
                                sizeof(DHCP_LX_MONITOR_T),
                                (VOID *)&lx_dhcp_monitor);

            if (i4_status != OSR_OK)
            {                
                /* Bad news, some other error. Well, simply return a NULL_HANDLE. */
                dbg_mw_printf("Create DHCP starting thread failed, ret[%d], i4_time[%d]\n", (int)i4_status, i4_time);       		
            }
            else
            {
                fg_dhcp_thread_running = TRUE;
                dbg_mw_printf("[nw]%s:%d:\n", __FUNCTION__, __LINE__);
                break;
            }
        }
    }
    if (i4_time > 0)
    {
        memcpy(&t_lx_dhcp_monitor_g, &lx_dhcp_monitor, sizeof(DHCP_LX_MONITOR_T));
    }

    u_crit_end(t_crit);

    return NET_OK;
}


INT32 c_dhcpc_notify_restart(CHAR *ps_ni_name, x_dhcpc_nfy_fct fn)
{

    INT32 i4_ret;

    t_lx_dhcp_monitor_g.pf_notify = fn;   //just replace with new callback function
    i4_ret = c_dhcpc_restart(ps_ni_name);

    return i4_ret;
}


INT32 c_dhcpc_stop(CHAR *ps_ni_name)
{
/////////////////////////
    char cmd[256];

    sprintf(cmd,"%s stop %s %s",dhcpscript,ps_ni_name,dhcpinfo);

    dbg_mw_printf("c_dhcpc_stop cmd : %s\n",cmd);

    g_dhcp_IsDhcpNeedStop = TRUE;

    system(cmd);

    dbg_mw_printf("c_dhcpc_stop finish!\n");

     return NET_OK;
/////////////////////////////
/*
    char *argp [5];

#if (!FORK2NETINFD)
    int pid, wpid, wstatus;
    int ret = 0;
#endif
    char *script = "/sbin/dhcpc.script";
    char *dhcpinfo = "/var/dhcpinfo";

    argp [0] = script;

    argp [1] = (char *)"stop";
    argp [2] = (char *)ps_ni_name;
    argp [3] = dhcpinfo;
    argp [4] = NULL;

	g_dhcp_IsDhcpNeedStop = TRUE;

    if (netinfd_dhcp(argp[1], argp[2]) < 0)
    {
      while(fg_dhcp_thread_running)
		usleep(10000);
      return NET_FAILED;
    }
	while(fg_dhcp_thread_running)
	  usleep(10000);
    return NET_OK;      */
}

/* PPPoE API for Application */

static VOID cleanup_pppoe_conf(PPPOE_CONF_T *pppoe_conf)
{
    if ( NULL != pppoe_conf )
    {
        if ( NULL != pppoe_conf->ifname)
        {
            free(pppoe_conf->ifname);
            pppoe_conf->ifname = NULL;
        }
        if ( NULL != pppoe_conf->username)
        {
            free(pppoe_conf->username);
            pppoe_conf->username = NULL;
        }
        if ( NULL != pppoe_conf->password)
        {
            free(pppoe_conf->password);
            pppoe_conf->password = NULL;
        }
        if ( NULL != pppoe_conf->cb_fn)
            pppoe_conf->cb_fn = NULL;
    }
}

static void _c_pppoe_start_notify_callback (IPCD_EVENT_T event)
{
    switch (event)
    {
        case IPCD_EXEC_SUCESS:

            //dbg_mw_printf("PPPoE start sucessed\n");
            if ( NULL != pppoe_config.cb_fn)
                pppoe_config.cb_fn(PPPOE_EVENT_SUCCESS);

            break;

        case IPCD_EXEC_FAILED:
            break;
        default:

            //dbg_mw_printf("PPPoE start failed\n");
            if ( NULL != pppoe_config.cb_fn)
                pppoe_config.cb_fn(PPPOE_EVENT_FAILURE);

            break;
    }
}

INT32 c_pppoe_start(CHAR * ifname, CHAR * username, CHAR * password, x_pppoe_nfy_fct cb_fn)
{
    CHAR cmd[64] = {0};

    if ( NULL == ifname || NULL == username || NULL == password || NULL == cb_fn)
    {
        dbg_mw_printf("<%s> Invalid arguemnt: ifname=%s, username=%s, password=%s, cb_fn=%p\n", __FUNCTION__,ifname, username, password, cb_fn);
        return NET_FAILED;
    }

    cleanup_pppoe_conf(&pppoe_config);

    pppoe_config.ifname = (CHAR *)malloc(strlen(ifname)+1);

    if(!pppoe_config.ifname)
    {
        dbg_mw_printf("[mw]memory alloc fail\n");
        return NET_FAILED;
    }
    strncpy(pppoe_config.ifname, ifname, strlen(ifname)+1);

    pppoe_config.username = (CHAR *)malloc(strlen(username)+1);
    
    if(!pppoe_config.username)
    {
        if (pppoe_config.ifname)
        {
            free(pppoe_config.ifname);
            pppoe_config.ifname = NULL;
        }
        dbg_mw_printf("[mw]memory alloc fail\n");
        return NET_FAILED;
    }
    strncpy(pppoe_config.username, username, strlen(username)+1);

    pppoe_config.password = (CHAR *)malloc(strlen(password)+1);
    
    if(!pppoe_config.username)
    {
        if (pppoe_config.ifname)
        {
            free(pppoe_config.ifname);
            pppoe_config.ifname = NULL;
        }

        if (pppoe_config.username)
        {
            free(pppoe_config.username);
            pppoe_config.username = NULL;
        }
        dbg_mw_printf("[mw]memory alloc fail\n");
        return NET_FAILED;
    }
    strncpy(pppoe_config.password, password, strlen(password)+1);

    pppoe_config.cb_fn = cb_fn;

    snprintf(cmd, sizeof(cmd), "pppoe.script start %s %s %s", pppoe_config.ifname, pppoe_config.username, pppoe_config.password);

    ipcd_exec_async(cmd, NULL, _c_pppoe_start_notify_callback);

    return NET_OK;
}

INT32 c_pppoe_stop()
{
    system("pppoe.script stop");

    cleanup_pppoe_conf(&pppoe_config);

    return NET_OK;
}

INT32 c_pppoe_restart()
{
    CHAR cmd[64] = {0};

    if ( NULL == pppoe_config.cb_fn)
    {
        dbg_mw_printf("<%s> No previous PPPoE config exists\n");
        return NET_FAILED;
    }

    snprintf(cmd, sizeof(cmd), "pppoe.script restart %s %s %s", pppoe_config.ifname, pppoe_config.username, pppoe_config.password);

    ipcd_exec_async(cmd, NULL, _c_pppoe_start_notify_callback);

    return NET_OK;
}

INT32 c_pppoe_get_info(CHAR *ps_ni_name, MT_DHCP4_INFO_T *pt_info)
{
    pt_info->ui4_ipaddr = 0;
    pt_info->ui4_subnet = 0;
    pt_info->ui4_router = 0;
    pt_info->ui4_dns1 = 0;
    pt_info->ui4_dns2 = 0;

    char *pppoeinfo = "/var/pppoeinfo";

    // After the script done, we need to parse the temp dhcp info file to retrieve info.
      char buf[ip_addr_len];
      FILE * fd;
      struct in_addr t_addr;
      UINT8 u1_dnscnt = 0;

      fd = fopen(pppoeinfo, "rt");
      if (NULL == fd)
        return NET_OK;

      while (EOF != fscanf(fd, "%15s", buf))
      {
        if (strcmp("IP", buf) == 0)
        {
            if (EOF == fscanf(fd, "%15s", buf))
            {
              fclose (fd);
              return NET_OK;
            }
            if (inet_aton(buf, &t_addr) == 0)
            {
              dbg_mw_printf ("IP %s is invalid\n", buf);
            }
            pt_info->ui4_ipaddr = t_addr.s_addr;
            dbg_mw_printf("IP: %s\n", buf);
        }
        else if (strcmp("Subnet", buf) == 0)
        {
            if (EOF == fscanf(fd, "%15s", buf))
            {
              fclose (fd);
              return NET_OK;
            }
            if (inet_aton(buf, &t_addr) == 0)
            {
              dbg_mw_printf ("Subnet %s is invalid\n", buf);
            }
            pt_info->ui4_subnet = t_addr.s_addr;
            dbg_mw_printf("Subnet: %s\n", buf);
        }
        else if (strcmp("router", buf) == 0)
        {
            if (EOF == fscanf(fd, "%15s", buf))
            {
              fclose (fd);
              return NET_OK;
            }
            if (inet_aton(buf, &t_addr) == 0)
            {
              dbg_mw_printf ("router %s is invalid\n", buf);
            }
            pt_info->ui4_router = t_addr.s_addr;
            dbg_mw_printf("router: %s\n", buf);
        }
        else if (strcmp("DNS", buf) == 0)
        {
            if (EOF == fscanf(fd, "%15s", buf))
            {
              fclose (fd);
              return NET_OK;
            }
            if (inet_aton(buf, &t_addr) == 0)
            {
              dbg_mw_printf ("DNS %s is invalid\n", buf);
            }
            if (u1_dnscnt == 0)
            {
              pt_info->ui4_dns1 = t_addr.s_addr;
            }
            else if (u1_dnscnt == 1)
            {
              pt_info->ui4_dns2 = t_addr.s_addr;
            }
            dbg_mw_printf("DNS: %s\n", buf);
            u1_dnscnt ++;
        }
      }
      fclose (fd);
      return NET_OK;
}


static VOID magic_packet_monitor_thread(VOID * WOLReq)
{
    INT32 sock, ret, i, counter = 0;
    WOL_REQ_T * wol_req = (WOL_REQ_T *)(*(int *)WOLReq);

    UCHAR local_mac[6] = {0};
    UCHAR bcast_mac[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
    UCHAR packet[1500] = {0};
    //struct sockaddr_in replyAddr;
    //socklen_t replylen;
    struct ifreq ethreq;

    struct magic_p_mac *mpm;

    dbg_mw_printf("Magic packet monitor thread started!\n");

    c_net_ni_get_mac(wol_req->if_name, (CHAR *)local_mac);
    //c_net_get_mac_from_sys((CHAR *)local_mac);
    dbg_mw_printf("Machine mac: %02x-%02x-%02x-%02x-%02x-%02x\n", local_mac[0], local_mac[1], local_mac[2], local_mac[3], local_mac[4], local_mac[5]);

    sock = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
    if (sock < 0)
    {
        perror("create socket");
        free(wol_req);
        fg_wol_thread_running = FALSE;
        u_thread_exit();
        return;
    }

    /* Set the network card in promiscuos mode */
    strncpy(ethreq.ifr_name, wol_req->if_name, strlen(wol_req->if_name)+1);
    if (ioctl(sock, SIOCGIFFLAGS, &ethreq)==-1)
    {
        perror("ioctl (SIOCGIFCONF) 1");
        close(sock);
        free(wol_req);
        fg_wol_thread_running = FALSE;
        u_thread_exit();
        return;
    }

    ethreq.ifr_flags|=IFF_PROMISC;
    if (ioctl(sock, SIOCSIFFLAGS, &ethreq)==-1)
    {
        perror("ioctl (SIOCGIFCONF) 2");
        close(sock);
        free(wol_req);
        fg_wol_thread_running = FALSE;
        u_thread_exit();
        return;
    }

    while(fg_wol_thread_running){
        if (wait_reply_timeout(sock, 1) == 0)
        {
            //dbg_mw_printf("No packet recieved, check flag to decide keep waiting or out\n");

        }else{

            ret = recvfrom(sock, packet, sizeof(packet), 0, NULL, NULL);

            if (0 > ret)
            {
                perror("recvfrom error");

            }else{
                memset(local_mac, 0, 6*sizeof(UCHAR));
                c_net_ni_get_mac(wol_req->if_name, (CHAR *)local_mac);

                //Check Destination MAC address
                if (memcmp(packet, local_mac, 6) && memcmp(packet, bcast_mac, 6))
                {
                    continue;
                }
                //dbg_mw_printf("Data recved: %d byte\n", ret);
                //dbg_mw_printf("Destination MAC Matched\n");

                for (i=0; i<(INT32)(ret - sizeof(struct magic_p_mac)); i++)
                {
                    //dbg_mw_printf("%02x ", packet[i]);

                    //Search for synchronization stream
                    if (0xFF == packet[i]) counter++;
                    else counter = 0;

                    if (6 == counter)
                    {
                        dbg_mw_printf("synchronization stream found\n");
                        mpm = (struct magic_p_mac *)&packet[i+1];
                        counter = 0;

                        //compare magic packet mac with local mac
                        if (memcmp(mpm->mac1, local_mac, 6))
                        {
                            i-=5;
                            continue;
                        }
                        dbg_mw_printf("Fisrt magic MAC address matched\n");

                        if ((memcmp(mpm->mac1, mpm->mac2, 6) ||
                             memcmp(mpm->mac1, mpm->mac3, 6) ||
                             memcmp(mpm->mac1, mpm->mac4, 6) ||
                             memcmp(mpm->mac1, mpm->mac5, 6) ||
                             memcmp(mpm->mac1, mpm->mac6, 6) ||
                             memcmp(mpm->mac1, mpm->mac7, 6) ||
                             memcmp(mpm->mac1, mpm->mac8, 6) ||
                             memcmp(mpm->mac1, mpm->mac9, 6) ||
                             memcmp(mpm->mac1, mpm->mac10, 6) ||
                             memcmp(mpm->mac1, mpm->mac11, 6) ||
                             memcmp(mpm->mac1, mpm->mac12, 6) ||
                             memcmp(mpm->mac1, mpm->mac13, 6) ||
                             memcmp(mpm->mac1, mpm->mac14, 6) ||
                             memcmp(mpm->mac1, mpm->mac15, 6) ||
                             memcmp(mpm->mac1, mpm->mac16, 6) ))
                        {
                            dbg_mw_printf("The fowllowing 16 mac not matched, skip this synchronization stream.\n");
                            i -= 5;
                            continue;
                        }

                        //dbg_mw_printf("Machine mac: %02x-%02x-%02x-%02x-%02x-%02x\n", local_mac[0], local_mac[1], local_mac[2], local_mac[3], local_mac[4], local_mac[5]);
                        //dbg_mw_printf("Wake up mac: %02x-%02x-%02x-%02x-%02x-%02x\n", mpm->mac1[0], mpm->mac1[1], mpm->mac1[2], mpm->mac1[3], mpm->mac1[4], mpm->mac1[5]);

                        //MAC address hit, prepare to power on
                        dbg_mw_printf("MAC address match!! Power On! \n");
                        wol_req->wol_cb();

                        close(sock);
                        free(wol_req);
                        fg_wol_thread_running = FALSE;
                        dbg_mw_printf("magic_packet_monitor_thread closed due to magic packet detected.\n");
                        u_thread_exit();
                        return;

                    }

                    //if (i!=0 && 0 == (i+1)%10) dbg_mw_printf("\n");

                }
                //dbg_mw_printf("\n");
            }
        }
    }

    close(sock);
    free(wol_req);
    fg_wol_thread_running = FALSE;
    dbg_mw_printf("magic_packet_monitor_thread closed due to stop function.\n");
    u_thread_exit();
    return;

}

INT32 c_net_start_wol_thread(CHAR *psz_name, wol_notify_cb_fn cb_fn)
{
      CRIT_STATE_T   t_crit;
      INT32 i4_status = -1;
      WOL_REQ_T *wol_req = NULL;
      INT32   i4_time = 100;

      if (NULL == psz_name || NULL == cb_fn)
      {
          dbg_mw_printf("<%s>Wrong parameter, please check the pass in parameter\n", __FUNCTION__);
          return NET_FAILED;
      }else{
            wol_req = (WOL_REQ_T *) malloc(sizeof(WOL_REQ_T));
            if (NULL == wol_req)
            {
                dbg_mw_printf("<%s>WOL_REQ malloc failed\n", __FUNCTION__);
                return NET_FAILED;
            }
            bzero(wol_req, sizeof(WOL_REQ_T));
            if (strlen(psz_name) < 16)
            {
                strncpy(wol_req->if_name, psz_name, strlen(psz_name)+1);
            }else{
                dbg_mw_printf("<%s>Wrong ifname, please check the pass in ifname: %s\n", __FUNCTION__, psz_name);
                free(wol_req);
                return NET_FAILED;
            }
            //dbg_mw_printf("wol_req->if_name: %s\n", wol_req->if_name);
            wol_req->wol_cb = cb_fn;
      }

      t_crit = u_crit_start();

      if (!fg_wol_thread_running)
      {
          while (i4_time-- && (i4_status != OSR_OK))
          {
              fg_wol_thread_running = TRUE;
              // Create the polling thread first
              i4_status = u_thread_create (&h_wol_monitor_thread,
                                  "WOLMNT",
                                  48 * 1024,
                                  NI_MONITOR_THREAD_PRIORITY,
                                  magic_packet_monitor_thread,
                                  sizeof(VOID *),
                                  (VOID *)&wol_req);

              if (i4_status != OSR_OK)
              {
                  fg_wol_thread_running = FALSE;
                  usleep(10000);                
                  /* Bad news, some other error. Well, simply return a NULL_HANDLE. */
                  dbg_mw_printf("Create WOL monitor thread failed, ret=%d, i4_time=%d\n", (int)i4_status, i4_time);
                  
              }
              else{
                fg_wol_thread_running = TRUE;
              }
          }

          if (i4_status != OSR_OK)
          {
            if (wol_req)
                free(wol_req);
            u_crit_end(t_crit);
            return  NET_FAILED;
          }
      }else {

          free(wol_req);
      }

      u_crit_end(t_crit);
      return NET_OK;

}

INT32 c_net_stop_wol_thread(VOID)
{
    fg_wol_thread_running = FALSE;

    return NET_OK;
}

INT32 c_ntp_sync_time(CHAR * time_server, x_ntp_nfy_fct cb_fn )
{
    CHAR cmd[64] = {0};

    if ( NULL == time_server)
    {
        time_server = "pool.ntp.org";
    }

    snprintf(cmd, sizeof(cmd), "ntpdate %s ", time_server);

    ipcd_exec_async(cmd, NULL, (IPCD_EXEC_CB_FP)cb_fn);

    return NET_OK;
}

HANDLE_T h_ntp_monitor_thread;
INT32 c_time_sync(CHAR * time_server, x_ntp_nfy_fct cb_fn )
{
	CRIT_STATE_T   t_crit;
	INT32		   i4_status;
	NTP_PARA_T ntp; 	
	t_crit = u_crit_start();	
	bzero(&ntp, sizeof(NTP_PARA_T));
	
	if ( NULL == time_server)
	{
		time_server = "pool.ntp.org";		
	}	
	snprintf(ntp.ac_command_name, sizeof(ntp.ac_command_name), "ntpdate %s", time_server); 
		
	dbg_mw_printf("{Network MW}%s,ntp command = %s\n", __FUNCTION__, ntp.ac_command_name);
	ntp.pf_notify = cb_fn;
		
	if (!fg_ntp_sync_ready)
	{				
		// Create the polling thread first		
		i4_status = u_thread_create (&h_ntp_monitor_thread,
							"NTPMON",
							1024,
							NI_MONITOR_THREAD_PRIORITY,
							ntp_mon_thread,
							sizeof(NTP_PARA_T),
							(VOID *)&ntp);
	
		if (i4_status != OSR_OK)
		{
			/* Bad news, some other error. Well, simply return a NULL_HANDLE. */
			dbg_mw_printf("{Network MW}Create ntp thread failed, ret = %d\n", (int)i4_status);
			u_crit_end(t_crit);
			return NET_FAILED;
		}
	}	
	else
	{
		dbg_mw_printf ("{Network MW}sync success\n");
		if(cb_fn != NULL)
		{			
			cb_fn(IPCD_EXEC_SUCESS);	
		}
	}
	u_crit_end(t_crit);
	return NET_OK;
}








#if 0
/**
 * HTTP API for application
 */

VOID c_http_cmd_abort(HTTP_CTX_T *pt_ctx)
{
}

VOID c_http_cmd_del_ctx(HTTP_CTX_T *pt_ctx)
{
}

UINT32 c_http_cmd_get_content_length(HTTP_CTX_T *pt_ctx)
{
    return 0;//x_http_cmd_get_content_length(pt_ctx);
}

CHAR * c_http_cmd_get_ctx_tag_result(HTTP_CTX_T *pt_ctx, CHAR *ps_tag_name)
{
    return 0;//x_http_cmd_get_ctx_tag_result(pt_ctx, ps_tag_name);
}

HTTP_RET_CODE_T c_http_cmd_get_method(HTTP_CTX_T *pt_ctx)
{
    return 0;//x_http_cmd_get_method(pt_ctx);
}

UINT32 c_http_cmd_get_recvd_size(HTTP_CTX_T *pt_ctx)
{
    return 0;//x_http_cmd_get_recvd_size(pt_ctx);
}

HTTP_CTX_T *c_http_cmd_new_ctx(CHAR *ps_url, x_http_fw_dn_read_nfy fn, CHAR *pac_buffer, INT32 i4_ac_buffer_size)
{
    return 0;//x_http_cmd_new_ctx(ps_url, fn, pac_buffer, i4_ac_buffer_size);
}

HTTP_RET_CODE_T c_http_cmd_post_method(HTTP_CTX_T *pt_ctx, CHAR *ps_request_content)
{
    return 0;//x_http_cmd_post_method(pt_ctx, ps_request_content);
}

VOID c_http_cmd_upgrade_module_init(VOID)
{
    return;
}

INT32 c_http_cmd_fix_non_base64_char(CHAR *ps_base64_input, INT32 i4_len)
{
    return 0;//x_http_cmd_fix_non_base64_char(ps_base64_input, i4_len);
}

HTTP_RET_CODE_T c_http_cmd_parse_xml(HTTP_CTX_T *pt_ctx, VOID *pv_in_xml, INT32 i4_in_xml_size)
{
    return 0;//x_http_cmd_parse_xml(pt_ctx, pv_in_xml, i4_in_xml_size);
}

HTTP_RESPONSE_STATUS_CODE_T c_http_cmd_get_rsp_status(HTTP_CTX_T *pt_ctx)
{
    return 0;//x_http_cmd_get_rsp_status(pt_ctx);
}
#endif


INT32 c_net_ni_get_subnet(char *dev, unsigned long *ip_addr)
{
   int s=0;
   struct ifreq req = {{{0}}};
   int err=0;

   strncpy(req.ifr_name, dev, strlen(dev));
   if((s = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
   {
      printf("<Subnet>create socket fail!\n");
      return -1;
   }
   err = ioctl(s, SIOCGIFNETMASK, &req);
   close(s);

   if(err == -1)
   {
      printf("<Subnet>ioctl get fail!errno = %d\n", errno);
      return err;
   }

   *ip_addr= ((struct sockaddr_in*)&req.ifr_addr)->sin_addr.s_addr;
   printf("<subnet>subnet = %s\n", inet_ntoa(*(struct in_addr*)ip_addr));
   return 0;
}


INT32 c_net_ni_get_gateway(char* ifname, unsigned long *gateway)
{
	FILE *fp;
	char buf[256];
	unsigned long dest_addr, gate_addr;
	char rd_ifname[30+1] = {0};
		
	fp = fopen("/proc/net/route","r");
	
	if(NULL == fp)
	{
		printf("<route table>open file fail!\n");
		return -1;
	}
	fgets(buf, sizeof(buf), fp);
	while(fgets(buf, sizeof(buf), fp))
	{
		if(sscanf(buf, "%30s\t%lx\t%lx", rd_ifname, &dest_addr, &gate_addr) != 3 || dest_addr != 0)
				continue;
		*gateway = gate_addr;
		break;
	}
	printf("<route table>gateway = %s\n", inet_ntoa(*(struct in_addr*)gateway));
	fclose(fp);
	return 0;
}

INT32 c_net_ni_get_dns(unsigned long *dns1, unsigned long *dns2)
{
	FILE *fp;
	char *var = "nameserver";
	char tmp[30+1] = {0};
	char tmp2[30+1] = {0};
	char buf[256] = {0};
	int cnt = 0;
	fp = fopen("/etc/resolv.conf","r");
	if(NULL == fp)
	{
		printf("<DNS>open file fail!\n");
		return -1;
	}
	while(fgets(buf, sizeof(buf), fp))
	{
		memset(tmp, 0, sizeof(tmp));
		memset(tmp2, 0, sizeof(tmp2));
		if(sscanf(buf, "%30s\t%30s", tmp, tmp2) != 2 )
			continue;
		if(0 != strcmp(tmp, var))
			continue;
		if(NULL != strchr(tmp2, ':')) /*consider ipv6 as ':' included*/ 
			continue;
		if(cnt == 0)
		{
			printf("<DNS> dns1 = %s\n", tmp2);
			*dns1 = inet_addr(tmp2);
			cnt ++;
			continue;
		}
		else
		{
			printf("<DNS> dns2 = %s\n", tmp2);
			*dns2 = inet_addr(tmp2);
			break;
		}
	}
	
	fclose(fp);
	return 0;
}

int c_net_get_netstack_type(void)
{
   int s=0;
   int rc = 0;
   struct ifreq req = {{{0}}};
   char *dev = "lo";

   strncpy(req.ifr_name, dev, strlen(dev));
   if((s = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
   {
      printf("<Subnet>create socket fail!\n");
      printf("Do not support Ipv4! \n");
   }else{
   		rc |= 0x01;
   		printf("Support Ipv4! \n");
		close(s);		
   }

   if((s = socket(AF_INET6, SOCK_DGRAM, 0)) < 0)
   {
      printf("<Subnet>create socket fail!\n");
      printf("Do not support Ipv6! \n");
   }else{
   		rc |= 0x02;
   		printf("Support Ipv6! \n");
		close(s);		
   }
   return rc;
}


static size_t curl_cb(char* data, size_t size, size_t num, void* tag)
{
	int tlsize = size * num;
	char *c = (char *)tag;	
	char *start, *end;

	start = strstr(data, "iso_code");
	if(!start)
	{
	     printf("[MW]--Error! Maxmind return : %s--\n",data);
			 return 0;
	}	
	start = strstr(start, ":");
	end = strstr(start+2, "\"");
	memcpy(c, start+2, end-start-2);
	
	return tlsize;
}

int c_net_getNationCode(char* Account,  char* IPAddr,  char *CountryCode)
{
	/*Input Valid?*/
	int ret = 0;
  char retBuf[128]={0};
	if(CountryCode == NULL)
	{
		printf("[MW]--%s input param invalid!--\n", __FUNCTION__);
		return -1;
	}
	CURL *c =  curl_easy_init();	
	/*printf("[MW]--SN : %s, IP: %s--\n", Account, IPAddr);
	char Url[200] = "http://geoip.maxmind.com/a?l=";
	strncat(Url,Account,50);
	strncat(Url,"&i=",4);
	strncat(Url,IPAddr,46);

  struct curl_slist * headers = NULL;
  char* refStr = "Referer: http://www.samsung.com";
  headers = curl_slist_append(headers, refStr);
	curl_easy_setopt(c, CURLOPT_HTTPHEADER, headers);*/
	
	curl_easy_setopt(c, CURLOPT_URL, "https://geoip.maxmind.com/geoip/v2.0/country/me");	
	curl_easy_setopt(c, CURLOPT_USERNAME, "66127");
	curl_easy_setopt(c, CURLOPT_PASSWORD, "eXbF9sANYJNd");
	curl_easy_setopt(c, CURLOPT_SSL_VERIFYPEER, 0);		
	curl_easy_setopt(c, CURLOPT_SSL_VERIFYHOST, 0);		
	curl_easy_setopt(c, CURLOPT_CONNECTTIMEOUT, 5);
	curl_easy_setopt(c, CURLOPT_TIMEOUT, 10);
	curl_easy_setopt(c, CURLOPT_WRITEDATA, (void* )retBuf);
	curl_easy_setopt(c, CURLOPT_WRITEFUNCTION, curl_cb);

	ret = curl_easy_perform(c); 
	curl_easy_cleanup(c);    
	if(CURLE_OK != ret)
	{
		printf("[MW]--%s CURL fail : %d!--\n", __FUNCTION__, ret);
		return ret;
	}
	
	printf("[MW]--Nation code reply: %s--\n", retBuf);
	if(strlen(retBuf)>3)
		return -3;
	memmove(CountryCode, retBuf, strlen(retBuf));
	return 0; 

}




/************************************************
 * For IPV6 
 * -- basic function
 ************************************************/


#if 1 

/*Should be defined in if_inet6.h*/
#define IF_RA_OTHERCONF	0x80
#define IF_RA_MANAGED	0x40
#define IF_RA_RCVD	0x20
#define IF_RS_SENT	0x10
#define IF_READY	0x80000000

struct in6_ifreq{
	struct in6_addr ifr6_addr;
	int ifr6_prefixlen;
	int ifr6_ifindex;
};

IPV6_CONFIG_INFO_T g_Ipv6SettingInfo;	 /*Ip setting info set from app*/
pthread_t g_ipv6MonThread_id ={0};		 /*Ipv6_mon_thread id*/
pthread_t g_ipv6RdnssThread_id ={0};		 /*Ipv6_mon_thread id*/
static BOOL s_isRdnssThreadOpen = FALSE;

int g_Is_ipv6MonThread_running = 0;      /*For debug, if ipv6_mon_thread is running*/
char *s_tmpResolvPath = "/tmp/resolv";

void net_strtolower(char* str)
{	
	int i;
	for(i=0; str[i]!=0; i++)
	{
		if(str[i]>='A' && str[i] <='Z')
		str[i] = str[i] + 'a' - 'A';
	}
}

void _net_cton_v6(char* _char, char* _n)
{
	int i=0;
	char tmpc[3] = {0};
	for(;i<16;i++)
	{
		memcpy(tmpc, _char+i*2, 2);
		_n[i] = strtol(tmpc,NULL,IPV6_NETADDR_LEN);
	}
}

void _net_ctop_v6(char* _char,char* _p)
{
	char _n[17] = {0};
	_net_cton_v6( _char, _n);
	inet_ntop(AF_INET6,_n, _p,IPV6_PTRADDR_LEN);
}

static NET_IP_TYPE_V6 _net_ip_scope_v6(const char* ip_in)
{	
    if (ip_in == NULL) return NET_IP_TYPE_UNKNOWN;
    if (strchr(ip_in, ':') == NULL) return NET_IP_TYPE_UNKNOWN;
    if (strcmp(ip_in, "::") == 0) return NET_IP_TYPE_V6_UNSPEC;
    if (strcmp(ip_in, "::1") == 0) return NET_IP_TYPE_V6_LOOPBACK;
    if (!strncmp(ip_in, "2", 1) || !strncmp(ip_in, "3", 1))  return NET_IP_TYPE_V6_GLOBAL;

    if (strncmp(ip_in, "ff00", 4) == 0) return NET_IP_TYPE_V6_MULTICAST;
    if (strncmp(ip_in, "fe80", 4) == 0) return NET_IP_TYPE_V6_LINKLOCAL;
    if (strncmp(ip_in, "fec0", 4) == 0) return NET_IP_TYPE_V6_SITELOCAL;
    if (!strncmp(ip_in, "fc", 2) || !strncmp(ip_in, "fd", 2))  return NET_IP_TYPE_V6_UNIQUE;

    return NET_IP_TYPE_UNKNOWN;
}

static NET_IP_TYPE_V6 _net_ip_type_v6(int scope, int ifa_flags)
{	
    if(NET_IP_TYPE_V6_GLOBAL != scope)
		return scope;

	if(ifa_flags & IFA_F_PERMANENT)
		return NET_IP_TYPE_V6_GLOBAL_PERNAMENT;

	if(ifa_flags & IFA_F_SECONDARY)
		return NET_IP_TYPE_V6_GLOBAL_TEMP;

	return NET_IP_TYPE_V6_GLOBAL_DYNAMIC;
}



int _mask2prefix_v6(char* mask)
{
	int idx =15, prefix=128;	
	char ch = mask[idx];
	
	while(!(ch % 2))
	{
		prefix--;
		ch = ch/2;
		if(!(prefix%8))
		{
			if(!idx)
				break;
			ch = mask[--idx];	
		}
	}
	NET_LOG("[MW]prefix is %d\n", prefix);
	return prefix;
}

/*********** ipv6 utility functions **********/
INT32 _net_get_nlipv6flags(char* dev, int *flags)
{
	struct {
		  struct nlmsghdr n;
		  struct rtgenmsg r;
	} req;
	int status;
	//char buf[16384];
	char *buf = NULL;
	struct nlmsghdr *nlmp;
	struct ifinfomsg *rtmp;
	struct rtattr *rtatp;
	int rtattrlen, ifidx;
	int fd = socket(PF_NETLINK, SOCK_DGRAM, NETLINK_ROUTE);

    if (fd == -1)
    {
        dbg_mw_printf("_net_get_nlipv6flags:create socket fail: %s\n", strerror(errno));
        return -1;
    }
	buf = (char*)malloc(sizeof(char) * 16384);
	if(buf == NULL) 
	{
		perror("malloc fail, oom");
		close(fd);
		return -1;
	}
	memset(buf, 0, sizeof(char) * 16384);
    
	int ret =0;
	int found = -1;
	
	memset(&req, 0, sizeof(req));
	req.n.nlmsg_len = NLMSG_LENGTH(sizeof(struct rtgenmsg));
	req.n.nlmsg_flags = NLM_F_REQUEST | NLM_F_DUMP | NLM_F_MULTI;
	req.n.nlmsg_type = RTM_GETLINK;

	req.r.rtgen_family = AF_INET6;/*Request for ipv6 link-msg only*/	
	status = send(fd, &req, req.n.nlmsg_len, 0);

	if (status <= 0) {
		  perror("_net_get_nlipv6flags:send");
		  close(fd);
		  if(buf != NULL) 
		  {
		      free(buf);
		  }
		  return -1;
	}

	ret = c_net_get_ifindex(dev,&ifidx);
	if(0 != ret)
	{
		NET_LOG("wpf:c_net_get_ifindex return error\n");
        if(buf != NULL) 
		{
		    free(buf);
		}    
	}
	NET_ERROR_RET(ret,fd, "Get if-index by if-name error!");
	 
	/* Typically the message is stored in buf, so we need to parse the message to *
	* get the required data for our display. */
	int rcvEnd = 0;
	while(!rcvEnd)
	{
		status = recv(fd, buf, 16384, 0);
		if (status < 0) {
			perror("recv");
			close(fd);
			if(buf != NULL) 
		    {
		        free(buf);
		    }
			return -1;
		}

		for(nlmp = (struct nlmsghdr *)buf;
		 	status > sizeof(*nlmp);
			status -= NLMSG_ALIGN(nlmp->nlmsg_len),nlmp = (struct nlmsghdr*)((char*)nlmp + NLMSG_ALIGN(nlmp->nlmsg_len))
			){
			if(nlmp->nlmsg_type == NLMSG_DONE)
			{
				rcvEnd = 1;
				break;
			}
			if(nlmp->nlmsg_type != RTM_NEWLINK)/*Basicly would not be here~*/
				continue;
			
			rtmp = (struct ifinfomsg *)NLMSG_DATA(nlmp);
			rtatp = (struct rtattr *)IFLA_RTA(rtmp);	
			rtattrlen = IFLA_PAYLOAD(nlmp);

			if(ifidx != rtmp->ifi_index)
				continue;
			
			for (; RTA_OK(rtatp, rtattrlen); rtatp = RTA_NEXT(rtatp, rtattrlen)) {  
				if(rtatp->rta_type == IFLA_PROTINFO)
				{
					int innlen = rtatp->rta_len;
					struct rtattr *fla_in  = rtatp+1;
					for (; RTA_OK(fla_in, innlen); fla_in = RTA_NEXT(fla_in, innlen)){	
						if(fla_in->rta_type == IFLA_INET6_FLAGS)
						{
							*flags = *((int*)(fla_in+1));
							found = 0;
							break;
						}
					}	   

				}					
			}
		}
	}
	close(fd);
	if(buf != NULL) 
	{
        free(buf);
    }
	return found;
}

#if 0 /*RDNSS operation is moved to isolate file.*/
int _net_get_nlipv6Rndss(char* dev)
{
      int status;
      char buf[16384];
      struct nlmsghdr *nlmp;
	  struct nduseroptmsg *ndmsg;	  
      int rtattrlen;

      int fd = socket(PF_NETLINK, SOCK_DGRAM, NETLINK_ROUTE);
      int group = RTNLGRP_ND_USEROPT;
      setsockopt(fd, 270, NETLINK_ADD_MEMBERSHIP, &group, sizeof(group));/*270 = SOL_NETLINK*/
      /* Typically the message is stored in buf, so we need to parse the message to *
        * get the required data for our display. */
      int rcvEnd = 0;
      while(!rcvEnd)
      {
	      status = recv(fd, buf, sizeof(buf), 0);
	      if (status < 0) {
		      perror("recv");
		      return -1;
	      }
		for(nlmp = (struct nlmsghdr *)buf;
		 	status > sizeof(*nlmp);
			status -= NLMSG_ALIGN(nlmp->nlmsg_len),nlmp = (struct nlmsghdr*)((char*)nlmp + NLMSG_ALIGN(nlmp->nlmsg_len))
			){

			if(nlmp->nlmsg_type == NLMSG_DONE)
			{
				rcvEnd = 1;
				break;
			}
			if(nlmp->nlmsg_type != RTM_NEWNDUSEROPT)/*Basicly would not be here~*/
				continue;

			ndmsg = (struct nduseroptmsg *)NLMSG_DATA(nlmp);
			if(	AF_INET6 != ndmsg->nduseropt_family || 
				134		 != ndmsg->nduseropt_icmp_type ||)
				return -2;

			struct nd_opt_rdnss{ /*Icmpv6 ND option*/
				char nd_opt_rdnss_type;
				char nd_opt_rdnss_len;
				short nd_opt_rdnss_reserved;
				int nd_opt_rdnss_lifetime;
			};

			struct nd_opt_rdnss* rdnss_opt = (struct nd_opt_rdnss*)(ndmsg+1);

			if(rdnss_opt->nd_opt_rdnss_type == 25)/*ND_OPT_RDNSS*/
			{/*dump address & other config*/
				printf("rndss_len : %d!\n",rdnss_opt->nd_opt_rdnss_len * 8);
				printf("rndss_lifetime : %d!\n",htonl(rdnss_opt->nd_opt_rdnss_lifetime));				
			}
			int ndatalen = rdnss_opt->nd_opt_rdnss_len * 8;
			struct in6_addr* addr = (struct in6_addr*)(rdnss_opt+1);
			while(ndatalen > sizeof(struct in6_addr))
			{
				char ipstr[46];
				ndatalen -= sizeof(struct in6_addr);
				printf("--------dns addr is %s--------\n", inet_ntop(AF_INET6,addr, ipstr, 46));
				_net_renew_Rdnss(ipstr,rdnss_opt->nd_opt_rdnss_lifetime);
				addr++;
			}     
		}
	}
	close(fd);
	return 0;
}
#endif

int icmp_RS(char *iface)
{
    int len = sizeof(struct icmp6hdr) + 8;
	char ip_str[46] = {0};
	int if_idx;
	int prefix = 0;
    int retry = 3;
    struct sockaddr_in6 srcaddr = {0};	
    struct sockaddr_in6 dstaddr = {0};	
	
    char mac[6];
    struct icmp6hdr* pkt  = (struct icmp6hdr* )malloc(len);

	if(NULL == pkt)
	{
		NET_LOG("icmp_RS:Error, RS malloc fail!");
		return -1;
	}

	c_net_ni_get_ip_v6(iface, NET_IP_TYPE_V6_LINKLOCAL, ip_str, &prefix);
	c_net_get_ifindex(iface, &if_idx);
	
	NET_LOG("LINK_LOCAL Address: %s - if_idx: %d!\n", ip_str, if_idx);
    inet_pton(AF_INET6, ip_str, &(srcaddr.sin6_addr));
	srcaddr.sin6_family = AF_INET6;
	srcaddr.sin6_scope_id = if_idx;

    c_net_ni_get_mac(iface, mac);
    inet_pton(AF_INET6, "ff02::2", &(dstaddr.sin6_addr));
    memset(pkt, 0, len);
    pkt->icmp6_type = 133;
    

    char* icmp_option = (char*)pkt+8;
    *icmp_option = 1;
    *(icmp_option+1) = 1;
    memcpy(icmp_option+2,mac,6);

    int ttl = 255;	
    int sock = socket(AF_INET6, SOCK_RAW, IPPROTO_ICMPV6);

    if (sock == -1)
    {
        dbg_mw_printf("icmp_RS:create socket fail: %s\n", strerror(errno));
        free(pkt);
        return -1;
    }
    
    setsockopt(sock, IPPROTO_IPV6, IPV6_MULTICAST_HOPS, (char*)&ttl, sizeof(ttl));

	if(bind(sock, (struct sockaddr*)&srcaddr, sizeof(struct sockaddr_in6)) < 0)
	{
	    free(pkt);
		close(sock);
		perror("icmp_RS:RS bind fail!");		
    	return -1;
	}
    
	while(retry > 0)
	{
        retry --;
        if (sendto(sock, pkt, len, 0, (struct sockaddr*)&dstaddr, sizeof(struct sockaddr_in6)) < 0)
        {
		    perror("icmp_RS:RS send fail!");
        }else{
            NET_LOG("icmp_RS:send [%d] RS\n", 3-retry);
        }      

        usleep(50000);
	}
    free(pkt);
	close(sock);
    return 0;
}

int pre_flags = 0;

INT32 _net_get_RA_bit(char* dev, int* ip_state, int* dns_state)
{	/*Temp solution : if no RA msg in timeout,  both config as stateful*/
	int flags = 0;
	int timeout = 10; /*Max wait time, else ret directly*/    

	NET_LOG("<v6>1 - start get RA message\n");
    

	while(timeout)
	{
		if (_net_get_nlipv6flags(dev, &flags) == 0)
		{
            pre_flags = flags;
            NET_LOG("[Get RA bit]set pre_flags[%d]\n", pre_flags);
		}
		if(flags & IF_RA_RCVD)
			break;
		sleep(1);
		timeout--;
	}
	NET_LOG("[Get RA bit]flags[%d]\n", flags);
	if(timeout<=0)
	{
        NET_LOG("[Get RA bit]timed out\n");
        if (!pre_flags)
        {
    		*ip_state = NET_V6_STATEFUL;

            //for auto get IP, get dns manually
            if (*dns_state != NET_V6_MANUAL)
            {
    		    *dns_state =NET_V6_STATEFUL;
            }
        }else
        {
            NET_LOG("[Get RA bit]use pre_flags[%d]\n", pre_flags);
            *ip_state  =(pre_flags & IF_RA_MANAGED)? NET_V6_STATEFUL:NET_V6_STATELESS;

            //for auto get IP, get dns manually
            if (*dns_state != NET_V6_MANUAL)
            {
    		    *dns_state =(pre_flags & IF_RA_OTHERCONF)?NET_V6_STATEFUL:NET_V6_STATELESS;
            }
        }
	}
	else
    {
		*ip_state  =(flags & IF_RA_MANAGED)? NET_V6_STATEFUL:NET_V6_STATELESS;

        //for auto get IP, get dns manually
        if (*dns_state != NET_V6_MANUAL)
        {
		    *dns_state =(flags & IF_RA_OTHERCONF)?NET_V6_STATEFUL:NET_V6_STATELESS;
        }
	}
    NET_LOG("[Get RA bit]ip_state[%d],dns_state[%d]\n", *ip_state , *dns_state);
	NET_LOG("==End get RA address ==\n");
	return 0;
}


INT32 _net_store_ipv6_setting_info(void* pIn)
{
	IPV6_CONFIG_INFO_T* pInfo = pIn;

	CHECK_NULL_PTR(pInfo);	
	CHECK_NULL_PTR(pInfo->cb);
	ERROR_RET((pInfo->RA_mode != IPV6_MODE_IGNORE_RA && pInfo->RA_mode != IPV6_MODE_RA), "[MW]Ipv6 mode invalid!\n");

	if(IPV6_SETTING_MANUAL == pInfo->ip_mode && pInfo->RA_mode == IPV6_MODE_IGNORE_RA)
	{
		CHECK_NULL_PTR(pInfo->ip_addr);
		ERROR_RET((pInfo->ip_prefix<0 || pInfo->ip_prefix>128), "[MW]Ipv6 Prefix invalid!\n");		
	}
	
	NET_LOG("<v6>0 RA_mode is %d, ip_mode is %d, dns_mode is %d\n", pInfo->RA_mode,pInfo->ip_mode,pInfo->dns_mode);
	extern IPV6_CONFIG_INFO_T g_Ipv6SettingInfo;
	memmove(&g_Ipv6SettingInfo,pInfo,sizeof(IPV6_CONFIG_INFO_T));
	
	return 0;
}


static ssize_t
_icmpv6_recvfromLL (int fd, void *buf, size_t len, int flags,
		struct sockaddr_in6 *addr)
{
	char cbuf[CMSG_SPACE (sizeof (int))];
	struct iovec iov =
	{
		.iov_base = buf,
		.iov_len = len
	};
	struct msghdr hdr =
	{
		.msg_name = addr,
		.msg_namelen = sizeof (*addr),
		.msg_iov = &iov,
		.msg_iovlen = 1,
		.msg_control = cbuf,
		.msg_controllen = sizeof (cbuf)
	};

	ssize_t val = recvmsg (fd, &hdr, flags);
	if (val == -1)
	{
		NET_LOG("recv error(%d)\n", errno);
		return val;
	}

	/* ensures the hop limit is 255 */
	struct cmsghdr *cmsg = CMSG_FIRSTHDR (&hdr);
	for (; cmsg != NULL; cmsg = CMSG_NXTHDR (&hdr, cmsg))
	{
		if ((cmsg->cmsg_level == IPPROTO_IPV6)
				&& (cmsg->cmsg_type == IPV6_HOPLIMIT))
		{
			if (255 != *(int *)CMSG_DATA (cmsg))
			{
				// pretend to be a spurious wake-up
				errno = EAGAIN;
				return -1;
			}
		}
	}

	return val;
}

static int g_is_RA_rcvd = FALSE;

static ssize_t
_icmpv6_recvadv (int fd, unsigned if_idx, unsigned wait_ms, int *ip_conf, int *dns_conf)
{
#include <netinet/icmp6.h>

	struct timespec end;
	unsigned responses = 0;

	/* computes deadline time */
	clock_gettime(CLOCK_MONOTONIC, &end);
	{
		div_t d;

		d = div (wait_ms, 1000);
		end.tv_sec += d.quot;
		end.tv_nsec += d.rem * 1000000;
	}

	/* receive loop */
	for (;;)
	{
		/* waits for reply until deadline */
		struct timespec now;
		ssize_t val = 0;

		clock_gettime(CLOCK_MONOTONIC, &now);
		if (end.tv_sec >= now.tv_sec)
		{
			val = (end.tv_sec - now.tv_sec) * 1000
				+ (int)((end.tv_nsec - now.tv_nsec) / 1000000);
			if (val < 0)
				val = 0;
		}

		val = poll (&(struct pollfd){ .fd = fd, .events = POLLIN }, 1, val);
		if (val < 0)
		{
			NET_LOG("poll return < 0\n");
			break;
		}

		if (val == 0)
		{
			NET_LOG("poll return = 0\n");
			return responses;
		}

		/* receives an ICMPv6 packet */
		union
		{
			uint8_t  b[1460];
			uint64_t align;
		} buf;
		struct sockaddr_in6 addr;

		val = _icmpv6_recvfromLL (fd, &buf, sizeof (buf), MSG_DONTWAIT, &addr);
		if (val == -1)
		{
			if (errno != EAGAIN)
				NET_LOG("Receiving ICMPv6 packet\n");
			continue;
		}

		/* ensures the response came through the right interface */
		if (addr.sin6_scope_id
				&& (addr.sin6_scope_id != if_idx))
		{
			NET_LOG("scope id does not match\n");
			continue;
		}

		NET_LOG("parse msg...\n");
		{
			const struct nd_router_advert *ra = (const struct nd_router_advert *)buf.b;
			if (((size_t)val < sizeof (struct nd_router_advert))
					|| (ra->nd_ra_type != ND_ROUTER_ADVERT)
					|| (ra->nd_ra_code != 0))
			{
				NET_LOG("It's not RA response\n");
				continue;
			}

			unsigned int flag = ra->nd_ra_flags_reserved;
			*ip_conf = (flag & ND_RA_FLAG_MANAGED)?1:0;
			*dns_conf = (flag & ND_RA_FLAG_OTHER)?1:0;
            g_is_RA_rcvd = TRUE;
			NET_LOG("RA result(%X), addr(%X), oth(%X)\n",
					flag, *ip_conf, *dns_conf );

			if (responses < INT_MAX)
				responses++;

			//NET_LOG("Only need single response, return\n");
			return 1 /* = responses */;
		}
	}

	return -1; /* error */
}


int _icmpv6_get_ra(char * ifname, int *ip_conf, int *dns_conf)
{
/*
#include <netinet/icmp6.h>
workaroud due to this header file cannot be used
*/
    struct icmp6_hdr 
    {
        uint8_t     icmp6_type;   /* type field */
        uint8_t     icmp6_code;   /* code field */
        uint16_t    icmp6_cksum;  /* checksum field */
        union {
            uint32_t  icmp6_un_data32[1];/* type-specific field */
            uint16_t  icmp6_un_data16[2]; /* type-specific field */	
            uint8_t   icmp6_un_data8[4];  /* type-specific field */ 
        } icmp6_dataun;  
    };
    
    struct nd_router_solicit      /* router solicitation */ 
    {
        struct icmp6_hdr  nd_rs_hdr;    /* could be followed by options */
    };

    struct icmp6_filter {
        uint32_t icmp6_filt[8];  
    };
    
	typedef struct {
		struct nd_router_solicit header;
		char option_type[1];
		char option_len[1];
		char option_val[6];
	} rs_pkt;

	rs_pkt packet;
	struct sockaddr_in6 src_addr = {0};	
	struct sockaddr_in6 dst_addr = {0};	

	int fd = socket (PF_INET6, SOCK_RAW, IPPROTO_ICMPV6);
	if (fd < 0)
	{
		NET_ERR_LOG("create socket error.\n");
		return -1;
	}

	/* setsockopt */
	{
		/* set ICMPv6 filter */
		struct icmp6_filter f;

		ICMP6_FILTER_SETBLOCKALL (&f);
		ICMP6_FILTER_SETPASS (ND_ROUTER_ADVERT, &f);
		setsockopt (fd, IPPROTO_ICMPV6, ICMP6_FILTER, &f, sizeof (f));

		/* sets Hop-by-hop limit to 255 */
		int value;
		value = 255;
		setsockopt (fd, IPPROTO_IPV6, IPV6_MULTICAST_HOPS, &value, sizeof (value));
		setsockopt (fd, IPPROTO_IPV6, IPV6_UNICAST_HOPS, &value, sizeof (value));
		value = 1;
		setsockopt (fd, IPPROTO_IPV6, IPV6_RECVHOPLIMIT, &value, sizeof (value));
	}

	/* bind */
	int rebind_times = 10;
bind_sk:
	{
		//Prepare source address
		char ip_str[46] = {0};
		int if_idx, prefix = 0;

		c_net_ni_get_ip_v6(ifname, NET_IP_TYPE_V6_LINKLOCAL, ip_str, &prefix);
		c_net_get_ifindex(ifname, &if_idx);

		NET_LOG("LINK_LOCAL Address: %s - if_idx: %d!\n", ip_str, if_idx);
		inet_pton(AF_INET6, ip_str, &(src_addr.sin6_addr));
		src_addr.sin6_family = AF_INET6;
		src_addr.sin6_scope_id = if_idx;

		if(bind(fd, (struct sockaddr*)&src_addr, sizeof(src_addr)) < 0)
		{
			NET_ERR_LOG("bind socket error(%d):%s\n", errno, strerror(errno));
			if (rebind_times-- > 0) 
			{
    			/* Sometime linklocal address is used by kernel, just rebind after a while */
        		usleep(500000); //500 ms
			    goto bind_sk;
			}
    	    else
    			goto error;
		}
	}

	/* Prepare destination address and packet content */
	{
		inet_pton(AF_INET6, "ff02::2", &(dst_addr.sin6_addr));

		memset(&packet, 0, sizeof(packet));
		/*packet header*/
		packet.header.nd_rs_type = ND_ROUTER_SOLICIT;
		packet.header.nd_rs_code = 0;
		packet.header.nd_rs_cksum = 0; /* computed by the kernel */
		packet.header.nd_rs_reserved = 0;
		/*packet option*/
		packet.option_type[0] = 1; //source link-layer address
		packet.option_len[0] = 1;
		{
			char mac[6];
			c_net_ni_get_mac(ifname, mac);
			memcpy(packet.option_val, mac, 6);
		}
	}

	/* send rs then receive ra */
	{
		int retry = 4;
		ssize_t plen = sizeof (packet);
		while (retry > 0)
		{
			/* sends a Solitication */
			if (sendto (fd, &packet, plen, 0,
						(const struct sockaddr *)&dst_addr,
						sizeof (dst_addr)) != plen)
			{
				NET_ERR_LOG("failed to send rs packet.\n");
				goto error;
			}
			retry--;

			/* receives an Advertisement */
			ssize_t val = _icmpv6_recvadv (fd, src_addr.sin6_scope_id, 2000, ip_conf, dns_conf);
			if (val > 0)
			{
				//NET_ERR_LOG("get ra\n");
				close (fd);
				return 1;
			} else if (val == 0) {
				NET_ERR_LOG("Timed out.");
			} else
				goto error;
		}
	}

	close (fd);
	return 0;

error:
	NET_ERR_LOG("exit with error\n");
	close (fd);
	return -1;
}


/* predicted */
INT32 c_net_ni_get_ip_v6_ioctl(char *dev, int addr_type, char* ip_asc, int* prefix)
{
    struct ifaddrs *ifaddr = NULL, *ifa = NULL;
    int ip_type = NET_IP_TYPE_UNKNOWN;

    if (dev == NULL || ip_asc == NULL) {
        NET_LOG("invalid argument\n");
        return -1;
    }

    if (getifaddrs(&ifaddr) != 0) {
        NET_LOG("call getifaddrs error: %d, %s\n", errno, strerror(errno));
        return -1;
    }

    for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) {
    	void *ipsrc = NULL, *masksrc = NULL;
        if (ifa->ifa_addr->sa_family != AF_INET6) continue;
        if (strcmp(ifa->ifa_name, dev) != 0) continue;

        ipsrc = &((struct sockaddr_in6*)ifa->ifa_addr)->sin6_addr;
        masksrc = &((struct sockaddr_in6*)ifa->ifa_netmask)->sin6_addr;
	
		inet_ntop(AF_INET6,ipsrc, ip_asc,IPV6_PTRADDR_LEN);
		*prefix = _mask2prefix_v6(masksrc);

		/*Get ip type == addr_type*/
        ip_type = _net_ip_scope_v6(ip_asc);
        NET_LOG("==ip: %s, prefix: %d, type: %d==\n", ip_asc, *prefix, ip_type);
        if (ip_type == addr_type)
			break;
    }

    freeifaddrs(ifaddr);
	ERROR_RET((ip_type != addr_type),"No match ipv6 address as specific type! \n");	
    return 0;
}


struct{
	  struct nlmsghdr n;
	  struct ifaddrmsg r;
}nl_rt_req;

INT32 c_net_is_RA_rcvd(char* dev)
{	
	int flags = 0;	
	int ip_conf = 0;
    int dns_conf = 0;

    if(TRUE == g_is_RA_rcvd)
    {
        NET_LOG("c_net_is_RA_rcvd[%d]", g_is_RA_rcvd);
        return TRUE;
    }

    
	_net_get_nlipv6flags(dev, &flags);
	if(flags & IF_RA_RCVD)
	{
        g_is_RA_rcvd = TRUE;
        NET_LOG("c_net_is_RA_rcvd:g_is_RA_rcvd[%d]flags[%d]", g_is_RA_rcvd, flags);
		return TRUE;
	}
	if(_icmpv6_get_ra(dev, &ip_conf, &dns_conf) == 1)
	{
        g_is_RA_rcvd = TRUE;
        NET_LOG("c_net_is_RA_rcvd1[%d]", g_is_RA_rcvd);
		return TRUE;
	}
	else
	{
        NET_LOG("c_net_is_RA_rcvd2[%d]", g_is_RA_rcvd);
		return FALSE;
	}
}

INT32 c_net_ni_get_ip_v6(char *dev, int addr_type, char* ip_asc, int* prefix)
{
	int status;
	//char buf[16384];
	char *buf = NULL;
	struct nlmsghdr *nlmp;
	int nlmsgLen;
	struct ifaddrmsg *rt_ifmsg;
	struct rtattr *rta;
	int rtattrlen;
	struct in6_addr *in6p;
	int ifidx;
	int ip_scope, ip_type=0;
	struct{
		  struct nlmsghdr n;
		  struct ifaddrmsg r;
	}req;

	buf = (char*)malloc(sizeof(char) * 16384);
	if(buf == NULL) 
	{
		perror("malloc fail, oom");
		return -1;
	}
	memset(buf, 0, sizeof(char) * 16384);
    NET_LOG("[Get IP v6][mw]dev[%s], ip_asc[%s], prefix[%d], addr_type[%u]\n", dev, ip_asc, *prefix, addr_type);
	status = c_net_get_ifindex(dev, &ifidx);
	if(0 != status)
	{	
		free(buf);
	ERROR_RET(status, "Invalid ifindex!\n");
	}
	int fd = socket(PF_NETLINK, SOCK_DGRAM, NETLINK_ROUTE);

	memset(&req, 0, sizeof(req));
	req.n.nlmsg_len = NLMSG_LENGTH(sizeof(struct ifaddrmsg));
	req.n.nlmsg_flags = NLM_F_REQUEST | NLM_F_ROOT;
	req.n.nlmsg_type = RTM_GETADDR;
	req.r.ifa_family = AF_INET6;

	/* Time to send and recv the message from kernel */
	status = send(fd, &req, req.n.nlmsg_len, 0);
	if (status < 0) {
		perror("netlink get ip - send");
		close(fd);		
		if(buf != NULL) 
		{
			free(buf);
		}
		return -1;
	}

	status = recv(fd, buf, 16384, 0);
	if (status <= 0) {
		perror("netlink get ip - recv");
		close(fd);		
		if(buf != NULL)
		{
			free(buf);
		}
		return -1;
	}

	/* Typically the message is stored in buf, so we need to parse the message to get the required data*/
	for(nlmp = (struct nlmsghdr *)buf; status > sizeof(struct nlmsghdr);)
	{
		nlmsgLen = nlmp->nlmsg_len;
		if (!NLMSG_OK(nlmp, status))
		{
            NET_LOG("NLMSG not OK\n");			
			close(fd);
			if(buf != NULL) 
			{
				free(buf);
			}
			return 1;
		} 
		rt_ifmsg = (struct ifaddrmsg *)NLMSG_DATA(nlmp);
		rta = (struct rtattr *)IFA_RTA(rt_ifmsg);
		rtattrlen = IFA_PAYLOAD(nlmp);
		
		status -= NLMSG_ALIGN(nlmsgLen);
		nlmp = (struct nlmsghdr*)((char*)nlmp + NLMSG_ALIGN(nlmsgLen));		
		/*Device not match, to next*/
		if(ifidx != rt_ifmsg->ifa_index)
			continue;

		for (; RTA_OK(rta, rtattrlen); rta = RTA_NEXT(rta, rtattrlen))
		{
			if(rta->rta_type == IFA_CACHEINFO)
				continue;
			if(rta->rta_type == IFA_LOCAL||rta->rta_type == IFA_BROADCAST||rta->rta_type == IFA_ANYCAST)
				NET_LOG("[MW]Find curious ipv6-address!\n");
			if(rta->rta_type == IFA_ADDRESS)
			{
				in6p = (struct in6_addr *)RTA_DATA(rta);
				inet_ntop(AF_INET6,in6p, ip_asc,IPV6_PTRADDR_LEN);
				/*Get type match address*/
				ip_scope = _net_ip_scope_v6(ip_asc);
                NET_LOG("==ip:ip_scope[%d]\n", ip_scope);
				ip_type  = _net_ip_type_v6(ip_scope, rt_ifmsg->ifa_flags); 
                NET_LOG("==ip:ip_type[%d]\n", ip_type);
				*prefix =  rt_ifmsg->ifa_prefixlen;
				NET_LOG("==ip: %s, prefix: %d, type: %d==\n", ip_asc, *prefix, ip_type);
				if(ip_type & addr_type)/*addr type is we need, and may have a fix of several. Just return the first valid*/
				{
					close(fd);
                    NET_LOG("== return success\n");
					if(buf != NULL)
					{
						free(buf);
					}
					return 0;
				}
			}
		}
	}	
	close(fd);
	if(buf != NULL) 
	{
		free(buf);
	}
    NET_LOG("== return fail\n");
	return -1;
}


INT32 c_net_ni_set_ip_v6(char *dev, char* ipsrc, int v_prefix)
{
	/*set the first ipv6-address of dev*/
	struct in6_ifreq v6_ifreq={{{{0}}}};
	struct ifreq ifreq={{{0}}};
	int fd;
	int ret;
  
    NET_LOG("==set ip: %s, dev: %s, prefix: %d==\n", ipsrc, dev, v_prefix);	
	fd = socket(AF_INET6, SOCK_DGRAM,0);
	ERROR_RET((fd==-1),"socketv6 create error!\n");	

	strncpy(ifreq.ifr_name, dev, IFNAMSIZ);
    ret = ioctl(fd, SIOCGIFINDEX, &ifreq);
	if(ret == -1)
	{
		NET_LOG("ioctl get if_index error!\n");
		NET_LOG("Error happen in %s - %d, errno : %d", __FUNCTION__, __LINE__, errno); 		
		close(fd);
		return -1;
	}
	
	ret = inet_pton(AF_INET6, ipsrc, &v6_ifreq.ifr6_addr.s6_addr);
	if(ret != 1)
	{
		NET_LOG("inet_pton error!\n");		
		NET_LOG("Error happen in %s - %d, errno : %d", __FUNCTION__, __LINE__, errno); 
		close(fd);
		return -1;
	}
	
	v6_ifreq.ifr6_ifindex = ifreq.ifr_ifindex;
	v6_ifreq.ifr6_prefixlen = v_prefix;

	ret = ioctl(fd, SIOCSIFADDR, &v6_ifreq);
	if(ret == -1)
	{
		NET_LOG("ioctl get ip addr error!\n");
		NET_LOG("Error happen in %s - %d, errno : %d", __FUNCTION__, __LINE__, errno); 		
		close(fd);
		return -1;
	}	

	/*Notify ipv6 change*/
	{
		NI_LX_MONITOR_LIST_T* ni_mon_item = NULL;
		ni_mon_item = find_monitor_item_by_evt_type(dev, NI_DRV_EV_IP_ADDRESSv6_CHANGED);
		if(NULL != ni_mon_item)
			ni_mon_item->fgNeedNotify = TRUE;
	}

	close(fd);
	return 0;
}



INT32 c_net_ni_delete_ip_v6(char* dev, int v_prefix, char* ipstr)
{
	/*set the first ipv6-address of dev*/
	struct in6_ifreq v6_ifreq={{{{0}}}};
	struct ifreq ifreq={{{0}}};
	int fd=0;
	int ret;
  
    NET_LOG("==delete ip: %s, dev: %s, prefix: %d==\n", ipstr, dev, v_prefix);	
	fd = socket(AF_INET6, SOCK_DGRAM,0);
	ERROR_RET((fd == -1),"socketv6 create error!\n");	

	strncpy(ifreq.ifr_name, dev, IFNAMSIZ);
    ret = ioctl(fd, SIOCGIFINDEX, &ifreq);
	NET_ERROR_RET((ret == -1), fd, "ioctl get if_index error!\n");
	
	ret = inet_pton(AF_INET6, ipstr, &v6_ifreq.ifr6_addr.s6_addr);
	NET_ERROR_RET((ret != 1), fd, "inet_pton error!\n");
	
	v6_ifreq.ifr6_ifindex = ifreq.ifr_ifindex;
	v6_ifreq.ifr6_prefixlen = v_prefix;

	ret = ioctl(fd, SIOCDIFADDR, &v6_ifreq);
	NET_ERROR_RET((-1 == ret), fd, "ip ioctl error!\n");

	/*Notify ipv6 change*/
	{
		NI_LX_MONITOR_LIST_T* ni_mon_item = NULL;
		ni_mon_item = find_monitor_item_by_evt_type(dev, NI_DRV_EV_IP_ADDRESSv6_CHANGED);
		if(NULL != ni_mon_item)
			ni_mon_item->fgNeedNotify = TRUE;
	}

	close(fd);
	return 0;

}

/*multi set would not replace, just add*/
INT32 c_net_ni_set_gw_v6(char* dev, char* gwaddr)
{
	struct in6_rtmsg rtm={{{{0}}}};
	struct ifreq ifreq = {{{0}}};
	int fd = socket(AF_INET6,SOCK_DGRAM,0);
	int ret =0;

    NET_LOG("==set gateway: %s, dev: %s==\n", gwaddr, dev);		
	ret = inet_pton(AF_INET6, gwaddr, &rtm.rtmsg_gateway);
	NET_ERROR_RET((ret != 1), fd, "gateway msg error\n");
	strncpy(ifreq.ifr_name, dev, IFNAMSIZ);
    ret = ioctl(fd, SIOCGIFINDEX, &ifreq);
	NET_ERROR_RET((ret == -1), fd, "ioctl get ifindex error!\n");

	rtm.rtmsg_dst_len = 0;
	rtm.rtmsg_flags = RTF_UP;	
	rtm.rtmsg_metric = 1;	
	rtm.rtmsg_flags |= RTF_GATEWAY;
	rtm.rtmsg_ifindex = ifreq.ifr_ifindex;
	ret = ioctl(fd, SIOCADDRT, &rtm);
	NET_ERROR_RET((ret == -1), fd, "ioctl gw error!\n");
	rtm.rtmsg_ifindex = ifreq.ifr_ifindex;
	close(fd);

	return 0;

}

INT32 c_net_ni_delete_gw_v6(char* dev, char* gwaddr)
{
/*
	struct in6_rtmsg rtm={{{{0}}}};
	struct ifreq ifreq = {{{0}}};
	int fd = socket(AF_INET6,SOCK_DGRAM,0);
	int ret =0;

    NET_LOG("==del gateway: %s, dev: %s==\n", gwaddr, dev);		
	ret = inet_pton(AF_INET6, gwaddr, &rtm.rtmsg_gateway);
	NET_ERROR_RET((ret != 1), fd, "gateway msg error\n");
	strncpy(ifreq.ifr_name, dev, 20);
	ret = ioctl(fd, SIOCGIFINDEX, &ifreq);
	NET_ERROR_RET((ret == -1), fd, "ioctl get ifindex error!\n");

	rtm.rtmsg_dst_len = 0;
	rtm.rtmsg_flags = RTF_UP;	
	rtm.rtmsg_metric = 1;	
	rtm.rtmsg_flags |= RTF_GATEWAY;
	rtm.rtmsg_ifindex = ifreq.ifr_ifindex;
	ret = ioctl(fd, SIOCDELRT, &rtm);
	NET_ERROR_RET((ret == -1), fd, "ioctl del gw error!\n");
	close(fd);
	return ret;
 */ 
	char cmd[100] = {0};
	snprintf(cmd, 100, "ip -6 route del default dev %s", dev);
	return system(cmd);
}

INT32 c_net_ni_get_gw_v6(char* dev, char* gwaddr)
{
	FILE *fp;
	char buf[512];
	char dst[2*IPV6_NETADDR_LEN +1], stmp[2*IPV6_NETADDR_LEN +1], gw_c[2*IPV6_NETADDR_LEN +1],ifname[20]; 
	int x[6];

	const char dst_dft[] = "00000000000000000000000000000000";   
	
	fp = fopen("/proc/net/ipv6_route", "r");
	ERROR_RET((NULL == fp),"error in open ipv6 route table!\n");
	while(fgets(buf, sizeof(buf), fp))
	{

		if(sscanf(buf, "%32s %x %32s %x %32s %x %x %x %x %19s", dst, x, stmp, x+1, gw_c, x+2, x+3, x+4, x+5, ifname) == 10   && 
			strncmp(dev, ifname, 20) == 0               && 
			strncmp(dst, dst_dft, 2*IPV6_NETADDR_LEN) == 0) 
		{
			_net_ctop_v6(gw_c, gwaddr);
			//NET_LOG("Get route:  %s\n", gwaddr);
			fclose(fp);
			return 0;
		}
	}
	NET_LOG("Do not find route !\n");
	fclose(fp);	
	return -1;
}



INT32 c_net_set_dns_v6(char* dns1, char* dns2)
{
	char cmd[200] = {0};
    char tmp[100] = {0};

	
	snprintf(cmd, 50, "/sbin/dns.script setv6 ");
	
	/*insert ipv6 dns*/
	if(dns1 != NULL)
	{
		snprintf(tmp,50,"%s ", dns1);
		strncat(cmd, tmp, 50);
	}	
	if(dns2 != NULL)
	{
		snprintf(tmp,50,"%s ", dns2);
		strncat(cmd, tmp, 50);
	}
	system(cmd);
	return 0;
}

INT32 c_net_delete_dnsv6()
{
	/*rm v6 dns*/
	system("/sbin/dns.script rmv6");
	return 0;
}


INT32 c_net_get_dns_v6(char *dns1, char *dns2)
{
	FILE *fp;
	char *var = "nameserver";
	char tmp[30+1] = {0};
	char tmp2[46+1] = {0};
	char buf[256] = {0};
	int cnt = 0;
	fp = fopen("/etc/resolv.conf","r");
	if(NULL == fp)
	{
		NET_LOG("<DNS>open file fail!\n");
		return -1;
	}
	while(fgets(buf, sizeof(buf), fp))
	{
		memset(tmp, 0, sizeof(tmp));
		memset(tmp2, 0, sizeof(tmp2));
		if(sscanf(buf, "%30s\t%46s", tmp, tmp2) != 2 )
			continue;
		if(0 != strcmp(tmp, var))
			continue;
		if(NULL == strchr(tmp2, ':')) /*consider ipv6 as ':' included*/ 
			continue;
		
		if(cnt == 0)
		{
			NET_LOG("<DNS> dnsv6_1 = %s\n", tmp2);
			strncpy(dns1, tmp2, 46);
			cnt ++;
			continue;
		}
		else
		{
			NET_LOG("<DNS> dnsv6_2 = %s\n", tmp2);
			strncpy(dns2, tmp2, 46);
			break;
		}
	}
	
	fclose(fp);
	return !cnt;
}


INT32 c_net_ni_rmv6_gw(char* dev)
{
	int ret=0;
	char ipstr[IPV6_PTRADDR_LEN]={0};
	while(1)
	{/*rm all default gateway*/
		if(c_net_ni_get_gw_v6(dev, ipstr))
			break;
		ret = c_net_ni_delete_gw_v6(dev, ipstr);
		ERROR_RET(ret,"Delete gwv6 error!\n");
	}
	return 0;
}

/*get ip&dns by dhcp*/
INT32 c_net_ni_start_dhcpv6(int ip_state, int dns_state, char *dev)
{	
    system("mkdir -p /var/lib/dibbler");
    system("mkdir -p /etc/dibbler");
	char cmd[100] = {0};
	int ret = 0;
	
	/*Get ipv6_state*/
	if(NET_V6_STATEFUL == ip_state)
	{
		if(NET_V6_STATEFUL == dns_state)
		{
			NET_LOG("<v6>2 - DHCPv6 : IP & DNS STATE all statefull\n");
			snprintf(cmd, 100,"/sbin/dhcpv6.script %s ipdns", dev);
			ret = system(cmd);
		}
		else
		{
			NET_LOG("IP STATE stateful\n");
			snprintf(cmd, 100,"/sbin/dhcpv6.script %s ip", dev);
			ret = system(cmd);
		}
	}
	else
	{
		if(NET_V6_STATEFUL == dns_state)
		{
			NET_LOG("<v6>2 - DHCPv6 : DNS STATE stateful\n");
			snprintf(cmd, 100,"/sbin/dhcpv6.script %s dns", dev);
			ret = system(cmd);
		}
		else
			NET_LOG("<v6>2 - DHCPv6 : IP STATE & DNS STATE statelss\n");
	}
	return ret;
}

/*Means we are receiving msg from RDNSS-thread,  need to refresh DNS-file*/
void _net_rdnss_set_dns(char* tmpPath)
{
	/*If dns-mode is stateless, change ipv6 dns config to rdnss*/
	FILE *fp;
	char *var = "nameserver";
	char tmp[30+1] = {0};
	char tmp2[46+1] = {0};
	char buf[256] = {0};
	char dns1[46]={0}, dns2[46]={0};
	int cnt = 0;
	fp = fopen(tmpPath,"r");
	if(NULL == fp)
	{
		NET_LOG("<DNS>open file fail!\n");
		return;
	}
	
	while(fgets(buf, sizeof(buf), fp))
	{
		memset(tmp, 0, sizeof(tmp));
		memset(tmp2, 0, sizeof(tmp2));
		if(sscanf(buf, "%30s\t%46s", tmp, tmp2) != 2 )
			continue;
		if(0 != strcmp(tmp, var))
			continue;
		if(NULL == strchr(tmp2, ':')) /*consider ipv6 as ':' included*/ 
			continue;
		
		if(cnt == 0)
		{
			strncpy(dns1, tmp2, 46);
			cnt++;
			continue;
		}
		else
		{
			strncpy(dns2, tmp2, 46);
			break;
		}
	}	
	fclose(fp);	
	c_net_set_dns_v6(dns1,dns2);
	NET_LOG("[rdnss]set rdnss info\n");
}

void* _net_rdnssd_thread(void* pv_para)
{

    pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS,NULL);
    s_isRdnssThreadOpen = TRUE;    
    system("/sbin/rdnss &");    
	return NULL;
}

INT32 c_net_ni_stop_rdnss()
{    
       
	if(FALSE != s_isRdnssThreadOpen)
	{
		system("kill $(cat /tmp/rdnss.pid)");
		pthread_cancel(g_ipv6RdnssThread_id);
		pthread_join(g_ipv6RdnssThread_id,NULL);
		s_isRdnssThreadOpen = FALSE;
	}
    
	return 0;
}


INT32 c_net_ni_start_rdnss(char *resolvPath)
{
	int ret = 0;
	char cmd[100];
	snprintf(cmd, 100, "rm -rf %s", resolvPath);
	system(cmd);

	if(FALSE != s_isRdnssThreadOpen)
	{
		pthread_cancel(g_ipv6RdnssThread_id);
		pthread_join(g_ipv6RdnssThread_id,NULL);
        c_net_ni_stop_rdnss();
		s_isRdnssThreadOpen = FALSE;
	}
	NET_LOG("<v6>1:1 - start rdnss monitor \n");
	ret = pthread_create(&g_ipv6RdnssThread_id, NULL,_net_rdnssd_thread, NULL);
    ERROR_RET(ret, "rdnss create thread error - %d!\n", ret);
    //ret = pthread_join(g_ipv6RdnssThread_id, &status);
    ERROR_RET(ret, "rdnss join thread error - %d!\n", ret);
    s_isRdnssThreadOpen = TRUE;

	return 0;
}


INT32 c_net_ni_manual_config_ipv6(int ip_state, int dns_state, char *dev, IPV6_CONFIG_INFO_T ipInfo)
{
	int ret = 0;
	NET_LOG("<v6>4 - manual config...");
	if(ip_state == NET_V6_MANUAL)
	{	
		ret |= c_net_ni_set_ip_v6(dev, ipInfo.ip_addr, ipInfo.ip_prefix);
		ret |= c_net_ni_set_gw_v6(dev, ipInfo.gw_addr);	
	}
	if(dns_state == NET_V6_MANUAL)
		c_net_set_dns_v6(ipInfo.dns1, ipInfo.dns2);
	return ret;

}

INT32 c_net_ni_ipv6_setKernDisable(char *dev)
{
	char cmd[100];
	snprintf(cmd,100, "echo 1 > /proc/sys/net/ipv6/conf/%s/disable_ipv6",dev);
	system(cmd);		
	return 0;
}

INT32 c_net_ni_ipv6_setKernEnable(char *dev)
{
	char cmd[100];
	snprintf(cmd, 100,"echo 0 > /proc/sys/net/ipv6/conf/%s/disable_ipv6",dev);
	system(cmd);		
	return 0;
}


INT32 c_net_ni_no_SLAAC(char *dev)
{
	char cmd[100];
	snprintf(cmd, 100, "echo 0 > /proc/sys/net/ipv6/conf/%s/autoconf",dev);
	system(cmd);		
	/*remove SLAAC ip*/
	return 0;
}

INT32 c_net_ni_accept_SLAAC(char *dev)
{
	char cmd[100];
	snprintf(cmd, 100, "echo 1 > /proc/sys/net/ipv6/conf/%s/autoconf",dev);
	system(cmd);	
	return 0;
}

INT32 c_net_ni_accept_RA(char *dev)
{
	char cmd[100];
	snprintf(cmd, 100, "echo 1 > /proc/sys/net/ipv6/conf/%s/accept_ra",dev);
	system(cmd);	
	return 0;
}

INT32 c_net_ni_no_RA(char *dev)
{
	char cmd[100];
	snprintf(cmd, 100, "echo 0 > /proc/sys/net/ipv6/conf/%s/accept_ra",dev);
	system(cmd);	
	return 0;
}



INT32 c_net_ni_start_SLAAC(int ip_state, int dns_state, char *dev)
{
	int ret= 0;
	if(NET_V6_STATELESS == ip_state)	
		c_net_ni_accept_SLAAC(dev);
	if(NET_V6_STATELESS == dns_state)
		ret = c_net_ni_start_rdnss(s_tmpResolvPath);
	if(NET_V6_MANUAL != ip_state)
		c_net_ni_accept_RA(dev);
	icmp_RS(dev);	
	return ret;
}

INT32 c_net_ni_accept_v6Temp(char *dev)
{
	char cmd[100];
	snprintf(cmd, 100, "echo 1 > /proc/sys/net/ipv6/conf/%s/use_tempaddr",dev);
	system(cmd);	
	return 0;
}


/**************************/
/***** Provide api to app*****/
/**************************/
INT32 c_net_ni_rmv6config_SLAAC(char* dev)
{	
	char ipstr[IPV6_PTRADDR_LEN]={0};
	int prefix = 0;
	int ret = 0;

	while(1)
	{/*rm all dynamic address*/
		if(c_net_ni_get_ip_v6(dev, NET_IP_TYPE_V6_GLOBAL_DYNAMIC | NET_IP_TYPE_V6_GLOBAL_TEMP, ipstr, &prefix))
			break;
		ret = c_net_ni_delete_ip_v6(dev, prefix, ipstr);
		ERROR_RET(ret, "Delete ipv6 address error!\n");
	}
	ret = c_net_ni_rmv6_gw(dev);
	//ERROR_RET(ret, "Delete ipv6 gw error!\n");/*If default gateway is from RA_msg, sometimes delete error*/
	
	c_net_ni_stop_rdnss();
	c_net_ni_no_SLAAC(dev);	
	c_net_ni_no_RA(dev);		
	return 0;
}	

INT32 c_net_ni_rmv6config_Dhcpv6(char* dev)
{	
	return c_net_ni_stop_dhcpv6(dev);
}	



INT32 c_net_ni_rmv6config_manual(char* dev)
{	
	char ipstr[IPV6_PTRADDR_LEN]={0};
	int prefix = 0;
	int ret = 0;

	while(1)
	{/*Only rm manual ip address*/
		if(c_net_ni_get_ip_v6(dev, NET_IP_TYPE_V6_GLOBAL_PERNAMENT, ipstr, &prefix))
			break;
		ret = c_net_ni_delete_ip_v6(dev, prefix, ipstr);
		ERROR_RET(ret, "Delete ipv6 address error!\n");
	}
	ret = c_net_ni_rmv6_gw(dev);
	//ERROR_RET(ret, "Delete ipv6 gw error!\n");/*If default gateway is from RA-msg, sometimes delete error*/	
	c_net_delete_dnsv6(); 	
	return 0;
}	


INT32 c_net_ni_rmv6config_all(char* dev)
{	
	c_net_ni_rmv6config_Dhcpv6(dev);
	c_net_ni_rmv6config_SLAAC(dev);
	c_net_ni_rmv6config_manual(dev);	
	return 0;
}	


INT32 c_net_ni_stop_dhcpv6(char *dev)
{	
	char cmd[100] = {0};
	snprintf(cmd, 100,"/sbin/dhcpv6.script stop");
	system(cmd);	
	
	return 0;
}



INT32 c_net_ni_disable_ipv6(char *dev)
{
    g_is_RA_rcvd = FALSE;
    NET_LOG("c_net_ni_disable_ipv6 g_is_RA_rcvd[%d]",g_is_RA_rcvd);
	c_net_ni_rmv6config_all(dev);
	c_net_ni_ipv6_setKernDisable(dev);
	return 0;
}


INT32 c_net_ni_enable_ipv6(char *dev)
{
	c_net_ni_ipv6_setKernEnable(dev);
	return 0;
}


INT32 c_net_ni_get_SLAAC_info(int ip_state, int dns_state,char* dev)
{  
	int cnt = 30;
	char ipstr[IPV6_PTRADDR_LEN];
	char dns1[IPV6_PTRADDR_LEN];	
	char dns2[IPV6_PTRADDR_LEN];
	int  prefix;
	int ret = 0;

	NET_LOG("<v6>3 - Wait for slaac info...");
	/*Address are acquired in back-mode,  here only detect ip & dns state*/	
	while(cnt--)
	{
		ret = 0;
		if(ip_state == NET_V6_STATELESS)
			ret |= c_net_ni_get_ip_v6(dev,NET_IP_TYPE_V6_GLOBAL_DYNAMIC,ipstr,&prefix);
		if(dns_state == NET_V6_STATELESS)
			ret |= c_net_get_dns_v6(dns1, dns2);
		if(!ret)
			return 0;
		sleep(1);
		if(!(cnt % 3))
			icmp_RS(dev);			
	}
	return -1;/*timeout*/
}

void _Ipv6MonExit(void*  pin)
{
	g_Is_ipv6MonThread_running = 0;
}

void* Ipv6addr_mon_thread(void* pIn)
{

    prctl(PR_SET_NAME,"ipv6_mon_thread",0,0,0);
	IPV6_CONFIG_INFO_T ipInfo={0};
	char dev[20+1] = {0};
	int ip_state = 0; 
    int dns_state = 0;
	int ret = 0;
    char cmd[100] = {0};
	
	
	strncpy(dev, pIn, 20);	
	memmove(&ipInfo, &g_Ipv6SettingInfo, sizeof(IPV6_CONFIG_INFO_T));	
	NET_LOG("<v6>0 - start monitor ipv6 address : dev[%s],RA_mode[%d],ip_mode[%d],dns_mode[%d]\n", 
        dev, ipInfo.RA_mode,
        ipInfo.ip_mode, ipInfo.dns_mode);
    //this two variable must be initialized by what the app set
    ip_state = ipInfo.ip_mode;
    dns_state = ipInfo.dns_mode;

	pthread_cleanup_push(_Ipv6MonExit,NULL);
	g_Is_ipv6MonThread_running = 1;
	
	pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS,NULL);

    //disable DAD for link local address
	system("echo 0 > /proc/sys/net/ipv6/conf/all/accept_dad");
	snprintf(cmd,100, "echo 0 > /proc/sys/net/ipv6/conf/%s/accept_dad", dev);
	system(cmd);
    
	c_net_ni_rmv6config_all(dev);	
	c_net_ni_accept_v6Temp(dev);
	c_net_ni_ipv6_setKernDisable("wifi0");/*For sony USB wifi work around*/
	if(ipInfo.RA_mode == IPV6_MODE_RA)
	{				/*RA - mode*/
	    /*Send RS packets*/		
		c_net_ni_accept_RA(dev);
		/*
		icmp_RS(dev);
		ret = _net_get_RA_bit(dev, &ip_state, &dns_state);
		if(ret)
			goto errRet; */
		
		/* save previous result in case of getting ra failed */
		static int _def_ip_ra = 0;
        static int _def_dns_ra = 0;
        _def_ip_ra = ipInfo.ip_mode;
        _def_ip_ra = ipInfo.dns_mode;
		int ip_ra = ipInfo.ip_mode, dns_ra = ipInfo.dns_mode;
		ret = _icmpv6_get_ra(dev, &ip_ra, &dns_ra);
		if (ret > 0 )
		{
			ip_state = ip_ra ? NET_V6_STATEFUL:NET_V6_STATELESS;
			if (dns_state != NET_V6_MANUAL)
				dns_state = dns_ra ? NET_V6_STATEFUL:NET_V6_STATELESS;

			/* save it as default */
            NET_LOG("<v6>get ra result by ra message\n");
			_def_ip_ra = ip_state;
			_def_dns_ra = dns_state;
		}
		else
		{
			ip_state = NET_V6_STATEFUL;
			if (dns_state != NET_V6_MANUAL)
				dns_state = NET_V6_STATEFUL;
			NET_LOG("<v6>get ra result failed, use default\n");
            _def_ip_ra = ip_state;
			_def_dns_ra = dns_state;
		}
	}
	else
	{				/*non-RA - mode*/
		ip_state = (ipInfo.ip_mode == IPV6_SETTING_AUTO)?NET_V6_STATEFUL:NET_V6_MANUAL;
		dns_state = (ipInfo.dns_mode == IPV6_SETTING_AUTO)?NET_V6_STATEFUL:NET_V6_MANUAL;
	}

    NET_LOG("<v6>0 ip_state[%d],dns_state[%d]\n", ip_state , dns_state);
	c_net_ni_start_SLAAC( ip_state, dns_state,dev);
	ret = c_net_ni_start_dhcpv6(ip_state,dns_state,dev);
	if(ret)
	{
		NET_LOG("DHCPv6 are wrong here!!");
		goto errRet;
	}
	
	ret = c_net_ni_get_SLAAC_info(ip_state,dns_state,dev);
	if(ret)
		goto errRet;
	
	ret = c_net_ni_manual_config_ipv6(ip_state, dns_state, dev, ipInfo);	
	if(ret)
		goto errRet;
	
	NET_LOG("<v6>OK - IPV6 address acquire is success!");
	ipInfo.cb(DHCPC_EVENT_SUCCESS_AUTOv6);	
	goto OKRet;
	
errRet:	
	
	NET_LOG("<v6>Error - IPV6 address acquire failed!");
	ipInfo.cb(DHCPC_EVENT_FAILURE_AUTOv6);
OKRet:
	pthread_cleanup_pop(0);
	g_Is_ipv6MonThread_running = 0;		
	return NULL;
}


INT32 c_net_ni_start_config_ipv6(char *dev, void* pIn)
{
	int ret = 0;
	static char sdev[20] = {0};
	
	strncpy(sdev, dev, 20);
	ret = _net_store_ipv6_setting_info(pIn);
	ERROR_RET(ret, "\n");	
    if(0 != (int)g_ipv6MonThread_id)
	{
		pthread_cancel(g_ipv6MonThread_id);
		pthread_join(g_ipv6MonThread_id,NULL);	
	}
	
	ret = pthread_create(&g_ipv6MonThread_id, NULL,Ipv6addr_mon_thread,sdev);
	ERROR_RET(ret, "Ipv6 monitor create thread error - %d!\n", ret);  
	return 0;
}

INT32 c_net_IsPreferIpv6(int flag)
{
	if(flag)
		system("cp -rf /etc/gai.conf.v6  /etc/gai.conf");		
	else
		system("cp -rf /etc/gai.conf.v4  /etc/gai.conf");				
	return 0;
}

INT32 c_net_get_ipv6_info(CHAR *ps_ni_name, MT_IPV6_INFO_T *pt_info)
{
	int v_pfx = 0;
	memset(pt_info->ac_ipaddr, 0, IPV6_MAX_ADDR_LEN);
	memset(pt_info->ac_llipaddr, 0, IPV6_MAX_ADDR_LEN);
	memset(pt_info->ac_router, 0, IPV6_MAX_ADDR_LEN);
	memset(pt_info->ac_dns1, 0, IPV6_MAX_ADDR_LEN);
	memset(pt_info->ac_dns2, 0, IPV6_MAX_ADDR_LEN);
	pt_info->ui1_prefix = 0;
	
	if(c_net_ni_get_ip_v6(ps_ni_name, NET_IP_TYPE_V6_VALIDIP, pt_info->ac_ipaddr, &v_pfx))
	{
		NET_LOG("v6 - No Valid Ipv6 address!\n");
		memset(pt_info->ac_ipaddr, 0, IPV6_MAX_ADDR_LEN);		
	}
	else
		pt_info->ui1_prefix = v_pfx;
	
	c_net_ni_get_ip_v6(ps_ni_name, NET_IP_TYPE_V6_LINKLOCAL, pt_info->ac_llipaddr, &v_pfx);	
	
	c_net_ni_get_gw_v6(ps_ni_name, pt_info->ac_router);
	c_net_get_dns_v6(pt_info->ac_dns1, pt_info->ac_dns2);
	return NET_OK;
}



/****************************/
/*Implent  ipv6 net debug api      */
/****************************/

typedef struct{
char* url;
char* ipv6;
int flag;
}THREAD_IN_DATA;

void* GetaddrFn(void *ps_data)
{
	THREAD_IN_DATA *p_data = ps_data;
	struct addrinfo hints={0}, *res;
	struct sockaddr_in6 *in6;

    prctl(PR_SET_NAME,"get_addrinfo_thread",0,0,0);

	memset(&hints,0,sizeof(hints));
	hints.ai_family = AF_INET6;
	hints.ai_socktype = SOCK_STREAM;
	if(0 != getaddrinfo(p_data->url, NULL, &hints, &res))
	{
		p_data->flag = -1;
        NET_LOG("h_errno: %d\n", h_errno);
		herror("hostname");	
		return NULL;
	}
	in6 = (struct sockaddr_in6 *)res->ai_addr;
	inet_ntop(AF_INET6, &(in6->sin6_addr), p_data->ipv6, 46);
	NET_LOG("url is %s \n", p_data->ipv6);
	p_data->flag = 1;
	freeaddrinfo(res);
	return NULL;
}

int c_net_getaddrinfo(char* url, char* ipstr)
{
	pthread_t mon_task_t = {0};
	int status = 0;
	const int interval = 100000;
	int timeout = 1000000*30;/*30s*/
	THREAD_IN_DATA in_data={0};
	char ipv6[46] = {0};	

	in_data.url = url;
	in_data.flag = 0;
	in_data.ipv6 = ipv6;
	status = pthread_create(&mon_task_t,NULL,GetaddrFn,&in_data);

	while(1)
	{
		usleep(interval);
		/*If ok? return.*/
		if(1 == in_data.flag)
		{
			NET_LOG("Getaddrinfo OK exit!\n");
			if(NULL != ipstr)
				strncpy(ipstr, ipv6, 46);
			return 0;
		}
		if(-1 == in_data.flag)
		{
			NET_LOG("Getaddrinfo Fail exit!\n");
			return -1;			
		}
		/*Timeout, exit*/
                timeout -= interval;
		if(timeout <= 0)
		{
			NET_LOG("Time out!\n");
			break;
		}
	}
	
	return -2; /*Time out*/
}



/**
* Test if a proxy server still open by trying to make a connection to the server.
*
* @param  ps_server_url     URL or IP address of the proxy server.
* @param  ui2_port          Server port of the proxy server.
*
* @return NET_OK      Connection test success.
*         NET_FAILED  Connection test fail.
*/

INT32 timed_tcp_test_v6(const CHAR *ps_server_url, UINT16 ui2_port, UINT16 timeout)
{
    int sock_fd, so_error;
	int ret = 0;
    struct sockaddr_in6 dst;
	char ipstr[46] = {0};
	int rc = 0;
	
    struct timeval tv;
    fd_set writefds;
	
    socklen_t len = sizeof(so_error);	
	
	memset(&dst, 0, sizeof(struct sockaddr_in6));	
    if (inet_pton(AF_INET6, ps_server_url, &(dst.sin6_addr)) == 1)
    {
      // URL: Digit IP
      dbg_mw_printf("IPv6 = %s\n", ps_server_url);
    }
	else
    {
		ret = c_net_getaddrinfo((char* )ps_server_url,ipstr);
		ERROR_RET(ret,"c_net_getaddrinfo error!\n");
		inet_pton(AF_INET6, ipstr, &(dst.sin6_addr));
    }
	
    dst.sin6_family = AF_INET6;
    dst.sin6_port = htons(ui2_port);

	sock_fd = socket(AF_INET6, SOCK_STREAM, 0);
    if (sock_fd <0)
    {
        dbg_mw_printf("create socketv6 fail !");
        return NET_FAILED;
    }
    fcntl(sock_fd, F_SETFL, O_NONBLOCK);
    rc = connect(sock_fd, (struct sockaddr *) &dst, (socklen_t)sizeof(struct sockaddr_in6));
	if(rc != 0 && errno != EINPROGRESS)
	{
		perror("connect fail");
		close(sock_fd);		
        return NET_FAILED;
	}

	tv.tv_sec  = timeout;
    tv.tv_usec = 0;
    FD_ZERO(&writefds);
    FD_SET(sock_fd, &writefds);
    if(select(sock_fd + 1, NULL, &writefds, NULL, &tv))
    {

        getsockopt(sock_fd, SOL_SOCKET, SO_ERROR, &so_error, &len);

        if(0 == so_error)
        {
            dbg_mw_printf("connect success\n");
        }
        else
        {
            dbg_mw_printf("connect fail, error code: %s\n", strerror(so_error));
            close (sock_fd);
            return NET_FAILED;
        }
    }
    else
    {
        dbg_mw_printf("connect timeout!\n");
        close (sock_fd);
        return NET_FAILED;
    }
    close (sock_fd);
    return NET_OK;
}

INT32 c_net_proxy_connection_v6(const CHAR *ps_server_url, UINT16 ui2_port)
{
    return timed_tcp_test_v6(ps_server_url, ui2_port, SOCKET_CONNECTION_TIMEOUT_SEC);
}

INT32 c_net_test_connection_v6(const CHAR *ps_server_url, UINT16 ui2_port, UINT16 ui2_timeout)
{
    return timed_tcp_test_v6(ps_server_url, ui2_port, ui2_timeout);
}


int csum_ipv6_header(struct in6_addr* src, struct in6_addr* dst, int len, int nexthdr)
{
	char ps_ipheader[40];
	memcpy(ps_ipheader, src, sizeof(struct in6_addr));
	memcpy(ps_ipheader+16, dst, sizeof(struct in6_addr));	
	memcpy(ps_ipheader+32, &len, 4);
	memcpy(ps_ipheader+36, &nexthdr, 4);
	return ping_chksum((UINT16*)ps_ipheader, 40);
}



int _net_icmp6_echo(struct sockaddr_in6* dst_addr, int len, int sock)
{
	struct sockaddr_in6 src_addr={0};
    struct icmp6hdr* pkt=(struct icmp6hdr* )malloc(len);
	char *pkt_data = (char *)pkt + icmp6_echo_offset;/*echo pkt data is at 8.*/
    char ipsrc[46] = {0};
	int ret  =0;

	if(NULL == pkt)
	{
		NET_ERR_LOG("Error in malloc icmpv6 packet!\n");
		return -1;
	}
	//int prefix = 0;	
    //c_net_ni_get_ip_v6("eth0",NET_IP_TYPE_V6_GLOBAL,ipsrc,&prefix); 
    inet_pton(AF_INET6,ipsrc, &(src_addr.sin6_addr));

    gettimeofday((struct timeval *)pkt_data, NULL);	
    pkt->icmp6_type = icmp6_echo;
    pkt->icmp6_code = 0;
    pkt->icmp6_identifier = getpid();
    //pkt->icmp6_cksum = ping_chksum((char*)pkt, sizeof(struct icmp6hdr) + pkt_size);
    //pkt->icmp6_cksum += csum_ipv6_header(&(src_addr.sin6_addr), &(dst_addr->sin6_addr), sizeof(struct icmp6hdr) + pkt_size, 0x3a);	
    ret = sendto(sock, pkt,len,0,(struct sockaddr*)dst_addr, sizeof(struct sockaddr_in6));	
    if(ret == -1)
    {
		NET_ERR_LOG("Send ipv6 pkt error:%s\n", strerror(errno));
    }
    if (pkt)
    {
        free(pkt);
    }
	return ret;
}

int _net_icmp6_rcv(int wr_sock, char* in_data)
{
	int ret = 0;
	struct sockaddr_in6 reply;
	socklen_t addrlen = sizeof(struct sockaddr_in6);
	
	ret = recvfrom(wr_sock, in_data, 1500, 0, (struct sockaddr *)&reply, &addrlen);
	if(ret == -1)
	{
		NET_ERR_LOG("Rcv icmpv6-reply error:%s\n", strerror(errno));
		return -1;
	}
	return 0;
}


VOID c_net_ping_v6(
                  CHAR *ps_interface_name,
                  CHAR *ps_dest_ip,
                  INT32  i2_len,
                  INT16  i2_wait,
                  VOID (*notify_fn) (INT16 i2_rsp_time))
{
    struct sockaddr_in6 dst = {0};    
    struct ifreq interface;
    int wr_sock = 0;
    char buff[1500];    
	int ret= 0;
	int len = 0;
	int diff = 0;

    if  ((NULL == ps_interface_name)
       ||(NULL == ps_dest_ip))
    {
        notify_fn(-1);
        return ;        
    }
    NET_LOG("%s:%d:ipv6 test bin:interface name=%s,i2_wait = %d\n", __FUNCTION__, __LINE__, 
                    ps_interface_name,i2_wait);
    /*Url to addr*/
    if (inet_pton(AF_INET6, ps_dest_ip, &(dst.sin6_addr)) == 1)
    {
      // URL: Digit IP
      NET_LOG("Ping6 :dest ip:%s\n", ps_dest_ip);
    }	
	else{
		char ipstr[46];
		struct addrinfo hints={0}, *res;
		memset(&hints,0,sizeof(hints));
		hints.ai_family = AF_INET6;
		hints.ai_socktype = SOCK_RAW;
		if(0 != getaddrinfo(ps_dest_ip, NULL, &hints, &res))
		{
	        NET_ERR_LOG("h_errno: %d\n", h_errno);
			herror("hostname");					
			notify_fn(-1);
			return ;
		}
		memcpy(&dst,res->ai_addr,sizeof(struct sockaddr_in6));
		inet_ntop(AF_INET6, &(dst.sin6_addr), ipstr, 46);
		NET_LOG("dst url ip is %s \n", ipstr);
		freeaddrinfo(res);        
    }	

   
    
    
    
    /*send request*/
	wr_sock = socket(AF_INET6, SOCK_RAW, IPPROTO_ICMPV6);
    if (wr_sock == -1)
    {
        NET_ERR_LOG("socket create error:%s\n", strerror(errno));
        close(wr_sock);
        notify_fn(-1);
        return;
    }

    /* if there are two interface, ra0 and eth0. wr_sock needs to 
    bind to a source ip to specify a interface.
    */
    /*ret = bind(wr_sock, (struct sockaddr*)&source, sizeof(source));

    if (-1 == ret)
    {
        NET_LOG("Ping6 :bind icmp socket\n");
        notify_fn(-1);
        return ;
    }*/
    memset(&interface, 0, sizeof(interface));
    strncpy(interface.ifr_ifrn.ifrn_name, ps_interface_name, strlen(ps_interface_name));    
    
    if (setsockopt(wr_sock, SOL_SOCKET, SO_BINDTODEVICE, &interface, sizeof(interface)) < 0)
    {
        NET_ERR_LOG("setsockopt error:%s\n", strerror(errno));
        close(wr_sock);
        notify_fn(-1);
        return;        
    } 
    
    
	len = (i2_len<icmp6_least_len)?icmp6_least_len:i2_len;
    ret = _net_icmp6_echo(&dst, len, wr_sock);
	if(ret<=0)
	{
        NET_ERR_LOG("_net_icmp6_echo error(%d)\n", ret);
		notify_fn(-1);
		close(wr_sock);
		return;
	}
    /* wait to listen for replies */
	struct icmp6hdr *recvpkt = (struct icmp6hdr *)buff;	
	while(1)
	{
	    if (wait_reply_timeout(wr_sock, i2_wait) == 0)
	    {
	      NET_ERR_LOG("socket timeout\n");
	      close(wr_sock);
	      notify_fn(-1);
	      return;
	    }
		ret = _net_icmp6_rcv(wr_sock, buff);
		if(ret)
		{	
            NET_ERR_LOG("socket recive error(%d)\n", ret);
			close(wr_sock);
			notify_fn(-1);
			return ;
		}
		if(recvpkt->icmp6_identifier == getpid() && recvpkt->icmp6_type == icmp6_reply)
		{
	        struct timeval tmRecvPing = {0};
	        struct timeval * tmSendPing = (struct timeval *)((char*)recvpkt + icmp6_echo_offset);
	        gettimeofday(&tmRecvPing, NULL);

	        if (tmRecvPing.tv_usec < tmSendPing->tv_usec)
	        {
	          tmRecvPing.tv_sec --;
	          tmRecvPing.tv_usec += 1000000;
	        }

	        tmRecvPing.tv_sec  -= tmSendPing->tv_sec;
	        tmRecvPing.tv_usec -= tmSendPing->tv_usec;
	        diff = (tmRecvPing.tv_sec * 1000) + (tmRecvPing.tv_usec / 1000);		
			notify_fn(diff);
			break;
		}	
    }
    close(wr_sock);
    return;
}

int _net_IsValidIpv6Addr(char* ipstr)
{
	struct in6_addr netaddr;
	return inet_pton(AF_INET6, ipstr, &netaddr);
}

int c_net_IsValidIpv6Addr(char* ipstr)
{
	struct in6_addr netaddr;
	return inet_pton(AF_INET6, ipstr, &netaddr);
}

int c_net_IsValidIpv6Ip(char* ipstr)
{
	struct in6_addr netaddr;
	if(!inet_pton(AF_INET6, ipstr, &netaddr))
		return FALSE;
	if(NET_IP_TYPE_V6_UNIQUE == _net_ip_scope_v6(ipstr) || NET_IP_TYPE_V6_GLOBAL == _net_ip_scope_v6(ipstr))
		return 1;
	return FALSE;
}

#endif
/************************************************
 * NET NI C_api (Ignore part)
 ************************************************/
L2_SWITCH_T *c_net_switch_new(CHAR *ps_name)
{
    return 0;//x_net_switch_new(ps_name);
}

INT32 c_net_switch_add_default_ni(L2_SWITCH_T *pt_Switch, INT32 i4_NiHandle)
{
    return 0;//x_net_switch_add_default_ni(pt_Switch, i4_NiHandle);
}

INT32 c_net_switch_add_ni(L2_SWITCH_T *pt_Switch, INT32 i4_NiHandle)
{
    return 0;//x_net_switch_add_ni(pt_Switch, i4_NiHandle);
}

INT32 c_net_switch_add_learn_entry (L2_SWITCH_T *pt_Switch, UINT8 *pac_Mac, INT32 i4_NiHandle)
{
    return 0;//x_net_switch_add_learn_entry(pt_Switch, pac_Mac, i4_NiHandle);
}

INT32 c_net_swtich_find_learn_entry(L2_SWITCH_T *pt_Switch, UINT8 *pac_Mac)
{
    return 0;//x_net_swtich_find_learn_entry(pt_Switch, pac_Mac);
}

INT32 c_net_switch_transmit_packet(L2_SWITCH_T *pt_Switch, PKT_BUFF_T *pt_packet, INT32 i4_NiHandle)
{
    return 0;//x_net_switch_transmit_packet(pt_Switch, pt_packet, i4_NiHandle);
}

NET_NI_T *c_net_ni_new(CHAR *ps_name, INT32 i4_ifType)
{
    return 0;//x_net_ni_new(ps_name, i4_ifType);
}

INT32 c_net_ni_install_drv_entry(NET_NI_T *pt_ni, NetworkDriverEntry pf_fn)
{
    return 0;//x_net_ni_install_drv_entry(pt_ni, pf_fn);
}

INT32  c_net_ni_start(NET_NI_T *pt_ni)
{
    return 0;//x_net_ni_start(pt_ni);
}

INT32 c_net_ni_install_transmit_packet_callback(NET_NI_T *pt_ni, net_ni_transmit_pkt pf_fn)
{
    return 0;//x_net_ni_install_transmit_packet_callback(pt_ni, pf_fn);
}

INT32 c_net_ni_install_receive_packet_callback(NET_NI_T *pt_ni, net_ni_receive_pkt pf_fn)
{
    return 0;//x_net_ni_install_receive_packet_callback(pt_ni, pf_fn);
}

INT32 c_net_ni_packet_indication(NET_DRV_IF_T *pt_drv, PKT_BUFF_T *pt_pkb)
{
    return 0;//x_net_ni_packet_indication(pt_drv, pt_pkb);
}

INT32 c_net_ni_tx_completed(NET_DRV_IF_T *pt_drv, PKT_BUFF_T *pt_pkb)
{
    return 0;//x_net_ni_tx_completed(pt_drv, pt_pkb);
}

INT32 c_net_ni_rx_packet(NET_NI_T *pt_ni, PKT_BUFF_T *pt_buff)
{
    return 0;//x_net_ni_rx_packet(pt_ni, pt_buff);
}

INT32 c_net_ni_tx_packet(NET_NI_T *pt_ni, PKT_BUFF_T *pt_buff)
{
    return 0;//x_net_ni_tx_packet(pt_ni, pt_buff);
}

NET_NI_T *c_net_ni_get_by_name(CHAR *ps_name)
{
    return 0;//x_net_ni_get_by_name(ps_name);
}

VOID c_net_ni_list_reset(VOID)
{
    //return x_net_ni_list_reset();
}

INT32 c_net_ni_set_if_type(NET_NI_T *pt_ni, INT32 i4_type)
{
    return 0;//x_net_ni_set_if_type(pt_ni, i4_type);
}

NET_NI_T *c_net_ni_list_get(VOID)
{
    return 0;//x_net_ni_list_get();
}

INT32 c_net_ni_add_assocate_ni(NET_NI_T *pt_ni1, NET_NI_T *pt_ni2)
{
    return 0;//x_net_ni_add_assocate_ni(pt_ni1, pt_ni2);
}

INT32 c_net_if_join(const CHAR* ps_niName, UINT32 ui4_ip)
{
    return 0;//x_net_if_join(ps_niName, ui4_ip);
}

INT32 c_net_if_leave(const CHAR* ps_niName, UINT32 ui4_ip)
{
    return 0;//x_net_if_leave(ps_niName, ui4_ip);
}

INT32 c_net_if_multi_list(const CHAR* ps_niName, UINT32 *paui4_ip, INT32 i4_size)
{
    return 0;//x_net_if_multi_list(ps_niName, paui4_ip, i4_size);
}

