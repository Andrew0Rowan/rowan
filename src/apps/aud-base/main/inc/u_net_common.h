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



#ifndef U_NET_COMMON_H
#define U_NET_COMMON_H

#include "u_common.h"
//#include "section.h" // for CONFIG_SECTION_BUILD_LINUX_PROG
//#include "drv_def.h" // for CONFIG_BUILD_DRV_EMULATION

/* BSD-style types */
#ifndef __USERDRV__
#ifndef ANDROID
#ifndef __KERNEL__
typedef unsigned char       u_char;
typedef unsigned short      u_short;
typedef unsigned int        u_int;
typedef unsigned long       u_long;

typedef signed char         int8_t;
typedef unsigned char       u_int8_t;
typedef unsigned char       uint8_t;
typedef short               int16_t;
typedef unsigned short      u_int16_t;
typedef unsigned short      uint16_t;
typedef int                 int32_t;
typedef unsigned int        u_int32_t;
typedef unsigned int        uint32_t;
//typedef long long           int64_t;
//typedef unsigned long long  uint64_t;
#endif
#endif
#endif


/* no monitor notify */
typedef enum
{
    NI_DRV_EV_UNKNOW,
    /* ethernet */
    NI_DRV_EV_ETHERNET_PLUGIN,
    NI_DRV_EV_ETHERNET_UNPLUG,

    /* wlan */
    NI_DRV_EV_WLAN_ASSOC = NI_DRV_EV_ETHERNET_PLUGIN,
    NI_DRV_EV_WLAN_DISASSOC = NI_DRV_EV_ETHERNET_UNPLUG,

    /* ip0 */
    NI_DRV_EV_IP_ADDRESS_CHANGED,
    NI_DRV_EV_IP_ADDRESS_EXPIRED,
    NI_DRV_EV_IP_ADDRESSv6_CHANGED,
} NI_DRV_EV_T;

/**
 * NiHal retuen code
 * Network Interface Hardware Abstract Layer Interface
 */
enum
{
    NET_OK = 0,
    NET_FAILED = -65535,
    NET_CREATE_THREAD_FAILED,
    NET_INITIATED,
    NET_NOT_INITIATED,
    NET_INV_ARG,
    NET_IP_STACK_ERROR,
    NET_DRIVER_ERROR
};

enum
{
    NET_NI_OK = 0,
    NET_NI_ERROR = -65535,
    NET_NI_INV_ARG,
    NET_NI_NOT_EXIST,
    NET_NI_ALLOC_MEM_FAILED,
    NET_NI_MSG_QUEUE_FAILED
};

/**
 * DrvHal return code
 */
enum 
{
    NET_DRV_OK = 0,
    NET_DRV_ERROR = -65535,
    NET_DRV_INV_ARG,
    NET_DRV_INV_OID,
    NET_DRV_PKT_ERROR,
    NET_DRV_PKT_TOO_SMALL,
    NET_DRV_PKT_TOO_BIG,
    NET_DRV_HW_ERROR,
    NET_DRV_NOT_INIT,
    NET_DRV_NOT_START,
    NET_DRV_ALREADY_INIT,
    NET_DRV_ALREADY_START,
    NET_DRV_ALREADY_STOP,
    NET_DRV_NAME_DUPLICATE,
    NET_DRV_REG_FULL,
    NET_DRV_PKT_BUFF_ERROR,
    NET_DRV_NO_RESOURCE,
    NET_DRV_NO_ASSOCICATION
};

/* Layer 2 switch return code */
enum
{
    NET_SWITCH_OK = 0,
    NET_SWITCH_INVALID = -65535,
    NET_SWITCH_MALLOC_FAILED
};

/**
 * Loopback
 */
#define NET_INADDR_LOOPBACK         ((UINT32) 0x7f000001)

/**
 * Packet header checksum indicator
 */
typedef enum
{
    CHKSUM_NONE = 0,
    CHKSUM_HW,
    CHKSUM_UNNECESSARY
} NET_CHKSUM_FLAG_T;

/**
 *
  ifOperStatus, base on RFC 2233
  The current operational state of the interface.
  The testing(3) state indicates that no operational packets can be passed.
  If ifAdminStatus is down(2) then ifOperStatus should be down(2).
  If ifAdminStatus is changed to up(1)
  then ifOperStatus should change to up(1) if the interface is ready to transmit and
  receive network traffic; it should change to dormant(5) if the interface is
  waiting for external actions (such as a serial line waiting for an incoming
  connection); it should remain in the down(2) state if and only if there is a
  fault that prevents it from going to the up(1) state; it should remain in the
  notPresent(6) state if the interface has missing (typically, hardware) components.
 */
#define NET_IF_STATE_UP                 ((INT32) 1)
#define NET_IF_STATE_DOWN               ((INT32) 2)
#define NET_IF_STATE_TESTING            ((INT32) 3)
#define NET_IF_STATE_UNKNOWN            ((INT32) 4)
#define NET_IF_STATE_DORMANT            ((INT32) 5)
#define NET_IF_STATE_NOTPRESENT         ((INT32) 6)
#define NET_IF_STATE_LOWERLAYERDOWN     ((INT32) 7)

/**
 * Common type define
 * MAC_ADDRESS_T
 */
typedef UCHAR MAC_ADDRESS_T[6];

typedef struct _NET_PHY_ADDRESS_S
{
    UINT32                  ui4_SetToFlash;  /* TRUE: set to flash and register, FALSE: set to register only */
    MAC_ADDRESS_T           t_MacAddress;
}   NET_PHY_ADDRESS_T;

/**
 * Frame Checksum
 */
typedef struct _HW_FRAME_CHKSUM_S
{
    NET_CHKSUM_FLAG_T       e_chksum_flag;
    BOOL                    b_ip_header_chksum;
    UINT16                  ui2_ip_payload_chksum;
}   NET_CHKSUM_T;

#if (!CONFIG_SECTION_BUILD_LINUX_PROG)

/* define byte endian */
#ifdef _CPU_LITTLE_ENDIAN_
    #define htons(s) \
        (UINT16)((((UINT16)(s)<<8)&0xff00U)|(((UINT16)(s)>>8)&0x00ffU))

    #define htonl(l) \
        (UINT32)((((UINT32)(l)>>24)&0x00ffU)|(((UINT32)(l)>>8)&0xff00U) \
        |(((UINT32)(l)<<8)&0xff0000U)|(((UINT32)(l)<<24)&0xff000000U))

#else
    #define htons(s) (s)
    #define htonl(l) (l)
#endif

#define ntohs(s) htons(s)
#define ntohl(l) htonl(l)

#endif

/* MT_DHCP4_INFO_T.i2_status */
#define X_DHCP4_ZERO             0
#define X_DHCP4_STAT_INIT        1
#define X_DHCP4_STAT_SELECTING   2
#define X_DHCP4_STAT_REQUESTING  3
#define X_DHCP4_STAT_BOUND       4
#define X_DHCP4_STAT_RENEWING    5
#define X_DHCP4_STAT_REBINDING   6
#define X_DHCP4_STAT_ERR         7

/* MT_DHCP4_INFO_T.i2_lastevent or
 * MT_DHCPC_MSG_T.i4_event */
#define X_DHCP4_EVENT_RECV_OFFER     1
#define X_DHCP4_EVENT_RECV_ACK       2
#define X_DHCP4_EVENT_RECV_NAK       3
#define X_DHCP4_EVENT_SEND_BOUNDREQ  4
#define X_DHCP4_EVENT_SEND_RELEASE   5
#define X_DHCP4_EVENT_SEND_ERR       6
#define X_DHCP4_EVENT_CHANGE_INFO    7
#define X_DHCP4_EVENT_NOT_YET        8
#define X_DHCP4_EVENT_TIMEOUT        9

#define MAX_DHCP4_DOMAIN_LENGTH  256

typedef struct _MT_DHCP4_INFO_S
{
    INT16   i2_status;             /* DHCP Status */
    INT16   i2_lastevent;          /* occurd last event */
    UINT32  ui4_get_time;          /* system time of receiving DHCPACK */
    UINT32  ui4_remain_renewal;    /* remain time of renewal   */
    UINT32  ui4_remain_rebinding;  /* remain time of rebinding */
    UINT32  ui4_remain_lease;      /* remain time of lease     */
    UINT32  ui4_renewal;           /* renewal time   */
    UINT32  ui4_rebinding;         /* rebinding time */
    UINT32  ui4_lease;             /* lease time     */
    UINT32  ui4_server;            /* DHCP server IP address */
    UINT32  ui4_ipaddr;            /* My IP address */
    UINT32  ui4_subnet;            /* IP subnet address */
    UINT32  ui4_router;            /* default router IP address */
    UINT32  ui4_dns1;              /* primary DNS IP address */
    UINT32  ui4_dns2;              /* secondary DNS IP address */
    CHAR    ac_domain[MAX_DHCP4_DOMAIN_LENGTH];  /* domain name */
    INT16   i2_domain_len;
    UINT16  ui2_change_flag;
} MT_DHCP4_INFO_T;

#ifndef IPV6_MAX_ADDR_LEN
#define IPV6_MAX_ADDR_LEN 46
#endif

typedef struct _MT_IPV6_INFO_S
{
    INT16   i2_status;             /* ipv6 Status */
    INT16   i2_lastevent;          /* occurd last event */
    UINT32  ui4_get_time;          /* system time of receiving DHCPACK */
    UINT32  ui4_remain_renewal;    /* remain time of renewal   */
    UINT32  ui4_remain_rebinding;  /* remain time of rebinding */
    UINT32  ui4_remain_lease;      /* remain time of lease     */
    UINT32  ui4_renewal;           /* renewal time   */
    UINT32  ui4_rebinding;         /* rebinding time */
    UINT32  ui4_lease;             /* lease time     */
    CHAR  ac_server[IPV6_MAX_ADDR_LEN];             /* DHCP server IP address */

    CHAR  ac_ipaddr[IPV6_MAX_ADDR_LEN];            /* My IP address */	
    CHAR  ac_llipaddr[IPV6_MAX_ADDR_LEN];            /* Link-local IP address */
    UINT8 ui1_prefix;            /* ipv6 prefix */
    CHAR  ac_router[IPV6_MAX_ADDR_LEN];            /* default router IP address */
    CHAR  ac_dns1[IPV6_MAX_ADDR_LEN];              /* primary DNS IP address */
    CHAR  ac_dns2[IPV6_MAX_ADDR_LEN];              /* secondary DNS IP address */
    CHAR  ac_domain[MAX_DHCP4_DOMAIN_LENGTH];  /* domain name */
    INT16   i2_domain_len;
    UINT16  ui2_change_flag;
} MT_IPV6_INFO_T;



typedef struct _MT_NET_CFG_DEV_INFO_S
{
	INT32   i4_ifType;
	INT32   i4_status;
    UINT64  ui8_ifSpeed;
	CHAR    sz_ifName[8];
    CHAR    ac_ifPhysAddress[6];
} MT_NET_CFG_DEV_INFO;

typedef struct _MT_NET_CFG_DEV_LIST_S
{
    UINT32 ui4_num;
    MT_NET_CFG_DEV_INFO *pt_dev_list;
} MT_NET_CFG_DEV_LIST;

#endif

