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



#ifndef U_NET_UTILITY_H
#define U_NET_UTILITY_H

#include "u_net_common.h"

/**
 * Frame packet type
 */
#define X_IP_PACKET           0x0800
#define X_ARP_PACKET          0x0806
#define X_RARP_PACKET         0x8035

/**
 * http://www.iana.org/assignments/protocol-numbers
 */
#define X_PROTOCOL_ICMP       1
#define X_PROTOCOL_IGMP       2
#define X_PROTOCOL_TCP        6
#define X_PROTOCOL_EGP        8
#define X_PROTOCOL_IGP        9
#define X_PROTOCOL_UDP        17
#define X_PROTOCOL_IPv6       41
#define X_PROTOCOL_IPv6_ICMP  58
#define X_PROTOCOL_L2TP       115

/**
 * http://www.iana.org/assignments/port-numbers
 */
#define X_PORT_DNS               53
#define X_PORT_BOOTPS          67
#define X_PORT_BOOTPC         68
#define X_PORT_TFTP              69
#define X_PORT_HTTP              80

/**
 * Header length
 */
#define X_ETHERNET_HEADER_LENGTH       14
#define X_UDP_HEADER_LENGTH                 8
#define X_TCP_HEADER_LENGTH                 20 /* not include option*/

/**
 * Frame type
 */
#define X_MAC_FRAME_TYPE_ETHERNET_II  1
#define X_MAC_FRAME_TYPE_802_2_LLC    2
#define X_MAC_FRAME_TYPE_802_2_SNAP   3

/**
 * IP header offset
 */
#define X_IP_TOS_OFFSET              1
#define X_IP_TOTAL_LENGTH_OFFSET     2
#define X_IP_IDENTIFICATION_OFFSET   4
#define X_IP_FLAG_OFFSET             6
#define X_IP_TTL_OFFSET              8
#define X_IP_PROTOCOL_OFFSET         9
#define X_IP_HDR_CHKSUM_OFFSET       10
#define X_IP_SRC_IP_OFFSET           12
#define X_IP_DST_IP_OFFSET           16

/**
 * TCP header offset
 */
#define X_TCP_DST_PORT_OFFSET        2
#define X_TCP_SEQUENCE_ID_OFFSET     4
#define X_TCP_ACK_ID_OFFSET          8
#define X_TCP_HDR_LEN_OFFSET         12
#define X_TCP_FLAG_OFFSET            13
#define X_TCP_WINDOW_SIZE_OFFSET     14
#define X_TCP_CHKSUM_OFFSET          16

/**
 * UDP header offset
 */
#define X_UDP_DST_PORT_OFFSET        2
#define X_UDP_LEN_OFFSET             4
#define X_UDP_CHKSUM_OFFSET          6

/**
 * ARP header offset
 */
#define X_ARP_PROT_TYPE_OFFSET       2
#define X_ARP_HARD_SIZE_OFFSET       4
#define X_ARP_PROT_SIZE_OFFSET       5
#define X_ARP_OP_OFFSET              6
#define X_ARP_SENDER_MAC_OFFSET      8
#define X_ARP_SENDER_IP_OFFSET      14
#define X_ARP_TGT_ETH_ADDR_OFFSET   18
#define X_ARP_TGT_IP_OFFSET         24

#define ENABLE_RAW_DUMP  ((UINT32) 1 << 31)
#define ENABLE_ARP_DUMP  ((UINT32) 1 << 30)
#define ENABLE_IP_DUMP   ((UINT32) 1 << 29)
#define ENABLE_UDP_DUMP  ((UINT32) 1 << 28)
#define ENABLE_TCP_DUMP  ((UINT32) 1 << 27)
#define ENABLE_ICMP_DUMP  ((UINT32) 1 << 26)
#define ENABLE_DNS_DUMP  ((UINT32) 1 << 25)
#define ENABLE_DHCP_DUMP  ((UINT32) 1 << 24)
#define ENABLE_TFTP_DUMP  ((UINT32) 1 << 23)
#define ENABLE_HTTP_DUMP  ((UINT32) 1 << 22)

/**
 * dump packet
 */
/* Filter tx transfer in my mac (DA = my mac) */
#define FILTER_MAC_MYIN  ((UINT32) 1 << 0)
/* Filter tx transfer out from my mac (SA = my mac) */
#define FILTER_MAC_MYOUT ((UINT32) 1 << 1)
/* Filter tx transfer in (or out from) my mac (DA = my mac or SA = my mac) */
#define FILTER_MAC_MYIO  (FILTER_MAC_MYIN | FILTER_MAC_MYOUT)

#define FILTER_NI_RX ((UINT32) 1 << 2)
#define FILTER_NI_TX ((UINT32) 1 << 3)
#define FILTER_NI_RTX  (FILTER_NI_RX | FILTER_NI_TX)

#define FILTER_NI_ETH0  ((UINT32) 1 << 4)
#define FILTER_NI_WIFI0 ((UINT32) 1 << 5)
#define FILTER_NI_IP0   ((UINT32) 1 << 6)
#define FILTER_NI_ALL  (FILTER_NI_ETH0 | FILTER_NI_WIFI0 | FILTER_NI_IP0)

#define ENABLE_ALL_DUMP (ENABLE_ARP_DUMP | ENABLE_IP_DUMP | ENABLE_UDP_DUMP | ENABLE_TCP_DUMP | \
                         ENABLE_ICMP_DUMP | ENABLE_DNS_DUMP | ENABLE_DHCP_DUMP | ENABLE_TFTP_DUMP | \
                         ENABLE_HTTP_DUMP)

#define DISABLE_ALL_DUMP ((UINT32) 0)

typedef struct FrameInfo_s
{
    /**
     * Frame info
     */
    INT16   i2_frame_type;
    INT16   i2_frame_len;
    UCHAR   *pDaMacAddress;
    UCHAR   *pSaMacAddress;
    /**
     * Packet info
     */
    INT16   i2_pkt_type;
    INT16   i2_pkt_len;
    INT16   i2_pkt_protocol;
    /**
     * IP/ARP/RARP hdr start
     */
    UCHAR   *pPktHdrAddr;
    /**
     * TCP/UDP/ICMP hdr start
     */
    UCHAR   *pProtocolHdrAddr;
    /**
     * TFTP/DNS/DHCP/HTTP hdr start
     */    
    UCHAR   *pAppHdrAddr;     
} FrameInfo_T;

#endif


