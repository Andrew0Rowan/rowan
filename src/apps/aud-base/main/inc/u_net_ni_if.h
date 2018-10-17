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



#ifndef U_NET_NI_IF_H
#define U_NET_NI_IF_H

#include "u_net_common.h"
#include "u_net_drv_if.h"

#define MAX_NI_INTERFACE    16   /* To define maximum network interface support in system */
#define MAX_LEARNING_ENTRY  16   /* To define how many learning entry support in learning table */

#define MAX_IP_INSTANCE     4

/**
 * Interface define
 * Note,
 *      The maximum length of type is 4 bytes.
 */
#define NI_UNKNOW      "uwn"
#define NI_LOOPBACK    "l0"     /* software loopback*/
#define NI_ETHERNET_0  "eth0"   /* etheret */
#define NI_ETHERNET_1  "eth1"
#define NI_ETHERNET_2  "eth2"
#define NI_ETHERNET_3  "eth3"
#define NI_USB_0       "usb0"   /* usb */
#define NI_USB_1       "usb1"
#define NI_USB_2       "usb2"
#define NI_USB_3       "usb3"
#define NI_ATH0             "ath0"
#define NI_WIRELESS_0  "wi0"    /* wireless */
#define NI_WIRELESS_1  "wi1"
#define NI_WIRELESS_2  "wi2"
#define NI_WIRELESS_3  "wi3"
#define NI_IP_0        "ip0"    /* ip stack */
#define NI_IP_1        "ip1"
#define NI_IP_2        "ip2"
#define NI_IP_3        "ip3"


/* Forward declare */

typedef struct IF_TABLE_S       IF_TABLE_T;
typedef struct NET_NI_S         NET_NI_T;
typedef struct LearningEntry_S  LearningTable_T;
typedef struct L2SWITCH_S       L2_SWITCH_T;

/* following type is defined in http://www.iana.org/assignments/ianaiftype-mib */
#define IF_TYPE_OTHER              1
#define IF_TYPE_ETHERNET           6
#define IF_TYPE_PPP                23
#define IF_TYPE_SOFTWARE_LOOPBACK  24
#define IF_TYPE_MODEM              48
#define IF_TYPE_IEEE80211          71

/* Our internal type */
#define IF_TYPE_IP              32768

/* notify api */
typedef VOID (*ni_ev_notify_fnct) (NI_DRV_EV_T e_ev);

/**
 * interface information
 */
struct IF_TABLE_S
{
    INT32   i4_ifIndex;
    INT32   i4_ifType;
    INT32   i4_ifMtu;
    UINT64  ui8_ifSpeed;
    INT32   i4_ifOperStatus;
    UINT32  ui4_ifLastChange;
    INT32   i4_ifLinkUpDownTrapEnable;
    INT32   i4_ifPromiscuousMode;
    UINT64  ui8_ifInOctets;
    UINT64  ui8_ifInUcastPkts;
    UINT32  ui4_ifInDiscards;
    UINT32  ui4_ifInErrors;
    UINT32  ui4_ifInUnknownProtos;
    UINT64  ui8_ifOutOctets;
    UINT64  ui8_ifOutUcastPkts;
    UINT32  ui4_ifOutDiscards;
    UINT32  ui4_ifOutErrors;
    UINT64  ui8_ifInMulticastPkts;
    UINT64  ui8_ifInBroadcastPkts;
    UINT64  ui8_ifOutMulticastPkts;
    UINT64  ui8_ifOutBroadcastPkts;

    /* IP address */
    INT32   i4_ip_version;
    union   {
        UCHAR   uac_ipv4[4];
        UINT32  ui4_ipv4;
        UCHAR   uac_ipv6[16];
    }ip;

    union   {
        UCHAR   uac_v4[4];
        UINT32  ui4_v4;
        UCHAR   uac_v6[16];
    }mask;

    union   {
        UCHAR   uac_v4[4];
        UINT32  ui4_v4;
        UCHAR   uac_v6[16];
    }gateway;

    INT32   i4_dhcpc_handle;
    CHAR    sz_ifDescr[256];
    CHAR    sz_ifName[8];
    CHAR    ac_ifPhysAddress[6];
};

/**
 *  Layer 2 switch to dispatch packet to proper network device
 */
struct LearningEntry_S
{
	CHAR    ac_SaMac[6];           /* MAC address */
	INT16   i2_AgentOutSupport;    /* default value is 0 */
	UINT32 ui4_AgentOutTime;       /* agent out time in second */
	INT32   i4_NiHandle;           /* the handle to network interface */
	LearningTable_T *pt_next;  /* point to next entry in same hash value */
};

struct L2SWITCH_S
{
    CHAR              sz_name[8];
    INT32             i4_Lt_entry;					       /* the entry cnt */
    LearningTable_T   t_lt[MAX_LEARNING_ENTRY];            /* the default entry is 256 */
    INT32             i4_NiCnt;                            /* ni cnt */
    INT32             ai4_NiHandle[MAX_NI_INTERFACE];      /* the network interface handle */
    INT32             i4_DefaultNi;                        /* the default ni */
    struct L2SWITCH_S *pt_next;
};

/**
 * packet sniffer hook array(list)
 */
#define NET_PKT_FILTER_CONTINUE  ((INT32) 0)
#define NET_PKT_FILTER_DISCARD   ((INT32) 1)


typedef INT32 (*net_pkt_hook_fn) (PKT_BUFF_T *pt_buff);
typedef struct pkt_hook_s
{
    INT32              i4_priority;
    INT32              i4_niHandle;
    net_pkt_hook_fn    pf_sniffer;
    struct pkt_hook_s *pt_next;
}PKT_HOOK_T;

/**
 * net_ni_receive_pkt
 * @param pt_ni
 * @param pt_buff
 *
 * @return INT32
 */
typedef INT32 (*net_ni_receive_pkt) (NET_NI_T *pt_ni, PKT_BUFF_T *pt_buff);

/**
 * net_ni_transmit_pkt
 * @param pt_ni
 * @param pt_buff
 *
 * @return INT32
 */
typedef INT32 (*net_ni_transmit_pkt) (NET_NI_T *pt_ni, PKT_BUFF_T *pt_buff);

/**
 * Hardware abstract layer, the upper layer use this type to
 * communication with low layer driver.
 */
 
struct NET_NI_S
{
    NET_DRV_IF_T            t_drv_if;

	net_ni_transmit_pkt     pf_TransmitPacket;

	net_ni_receive_pkt      pf_ReceivePacket;

    L2_SWITCH_T             *pt_switch;

    PKT_HOOK_T              *pt_pktHook;

    IF_TABLE_T              t_if_Table;

    struct NET_NI_S        *pt_next;
};

/* Packet flow
 * Incoming packet -> IRQ -> Drv.ni_indicate_fnct -> SourceNi->pf_ReceivePacket ->
 * x_Switch_TransmitPacket(pt_switch, packet, srcIf) -> DestinationNi->pf_TransmitPacket ->
 * case 1: to IP stack,
 *         DestinationNi equal to IPNi
 *         pf_TransmitPacket call pf_ReceivePacket
 *         pf_ReceivePacket copy packet to Access internal API
 *         call ni_tx_complete_fnct to release packet.
 *         finished.
 * case 2: to other network interface
 *         pf_TransmitPacket call drv->netdrv_txmit_fnct
 *         drv->netdrv_txmit_fnct send packet out,
 *         call ni_tx_complete_fnct to release packet.
 */

/* Ni StartUp flow
 * ---------------
 * 1. create new swithc for dispatch packet
 *    pt_NewSwitch = x_Switch_New()
 * 2. create ni for each physical network interface
 *    pt_EthNi = x_Ni_New("eth0");
 * 3. Register ni to switch
 *    x_Switch_AddDefaultNi(pt_NewSwitch, (INT32) pt_EthNi);
 * 4. Register drv to ni
 *    x_Ni_InstallDrvEntry(pt_EthNi, EthDrvEntry); --> EthDrvEntry is a callback provided by driver
 * 5. Ni Start
 *    x_Ni_Start(pt_EthNi);
 */

/* c_net_set_ni_speed, see ioctl_mdio_cmd */
typedef enum
{
    ETH_SPEED_AUTO = 0,
    ETH_SPEED_10M_HALF,
    ETH_SPEED_10M_FULL,
    ETH_SPEED_100M_HALF,
    ETH_SPEED_100M_FULL,

}NI_SPEED_T;
#define SIOC_MDIO_CMD   (SIOCDEVPRIVATE+8)//see ioctl_mdio_cmd
#define SIOC_SET_SPEED_CMD   (SIOCDEVPRIVATE+9)
#define SIOC_PHY_CTRL_CMD         (SIOCDEVPRIVATE+10)//see ioctl_mdio_cmd
#define SIOC_GET_SPEED_DUPLEX_CMD   (SIOCDEVPRIVATE+11)//get current speed and mode

//For SIOC_ETH_MAC_CMD
typedef struct txDesc_s	/* Tx Ring */
{
	UINT32  buffer;		/* Tx segment data pointer */
	UINT32  ctrlLen;	/* Tx control and length */
		#define TX_COWN					(1 << 31)	/* Tx descriptor Own bit; 1: CPU own */
		#define TX_EOR					(1 << 30)	/* End of Tx descriptor ring */
		#define TX_FS					(1 << 29)	/* First Segment descriptor */
		#define TX_LS					(1 << 28)	/* Last Segment descriptor */
		#define TX_INT					(1 << 27)	/* Tx complete interrupt enable (when set, DMA generate interrupt after tx sending out pkt) */
		#define TX_INSV					(1 << 26)	/* Insert VLAN Tag in the following word (in tdes2) */
		#define TX_ICO					(1 << 25)	/* Enable IP checksum generation offload */
		#define TX_UCO					(1 << 24)	/* Enable UDP checksum generation offload */
		#define TX_TCO					(1 << 23)	/* Enable TCP checksum generation offload */
		#define TX_LEN_MASK				(0xffff)	/* Tx Segment Data length */
		#define TX_LEN_OFFSET			(0)
	UINT32  vtag;
		#define TX_EPID_MASK			(0xffff)	/* VLAN Tag EPID */
		#define TX_EPID_OFFSET			(16)
		#define TX_PRI_MASK				(0x7)		/* VLNA Tag Priority */
		#define TX_PRI_OFFSET			(13)
		#define TX_CFI					(1 << 12)	/* VLAN Tag CFI (Canonical Format Indicator) */
		#define TX_VID_MASK				(0xfff)		/* VLAN Tag VID */
		#define TX_VID_OFFSET			(0)
	UINT32  reserve;	/* Tx pointer for external management usage */
} TxDesc;

typedef struct rxDesc_s	/* Rx Ring */
{
	UINT32  buffer;		/* RX segment data pointer */
	UINT32  ctrlLen;	/* Rx control and length */
		#define RX_COWN					(1 << 31)	/* RX descriptor Own bit; 1: CPU own */
		#define RX_EOR					(1 << 30)	/* End of Rx descriptor ring */
		#define RX_FS					(1 << 29)	/* First Segment descriptor */
		#define RX_LS					(1 << 28)	/* Last Segment descriptor */
		#define RX_OSIZE				(1 << 25)	/* Rx packet is oversize */
		#define RX_CRCERR				(1 << 24)	/* Rx packet is CRC Error */
		#define RX_RMC					(1 << 23)	/* Rx packet DMAC is Reserved Multicast Address */
		#define RX_HHIT					(1 << 22)	/* Rx packet DMAC is hit in hash table */
		#define RX_MYMAC				(1 << 21)	/* Rx packet DMAC is My_MAC */
		#define RX_VTAG					(1 << 20)	/* VLAN Tagged int the following word */
		#define RX_PROT_MASK			(0x3)
		#define RX_PROT_OFFSET			(18)
			#define RX_PROT_IP			(0x0)		/* Protocol: IPV4 */
			#define RX_PROT_UDP			(0x1)		/* Protocol: UDP */
			#define RX_PROT_TCP			(0x2)		/* Protocol: TCP */
			#define RX_PROT_OTHERS		(0x3)		/* Protocol: Others */
		#define RX_IPF					(1 << 17)	/* IP checksum fail (meaningful when PROT is IPV4) */
		#define RX_L4F					(1 << 16)	/* Layer-4 checksum fail (meaningful when PROT is UDP or TCP) */
		#define RX_LEN_MASK				(0xffff)	/* Segment Data length(FS=0) / Whole Packet Length(FS=1) */
		#define RX_LEN_OFFSET			(0)
	UINT32  vtag;
		#define RX_EPID_MASK			(0xffff)	/* VLAN Tag EPID */
		#define RX_EPID_OFFSET			(16)
		#define RX_PRI_MASK				(0x7)		/* VLAN Tag Priority */
		#define RX_PRI_OFFSET			(13)
		#define RX_CFI					(1 << 12)	/* VLAN Tag CFI (Canonical Format Indicator) */
		#define RX_VID_MASK				(0xfff)		/* VLAN Tag VID */
		#define RX_VID_OFFSET			(0)
	UINT32  reserve;	/* Rx pointer for external management usage */
} RxDesc;
struct reg_par
{
    UINT32 addr;    /* reg addr */
    UINT32 val;    /* value to write, or value been read */
};


struct tx_descriptor_par
{
    UINT32 index;    /* tx descriptor index*/
    TxDesc *prTxDesc;
};

struct rx_descriptor_par
{
    UINT32 index;    /* tx descriptor index*/
    RxDesc *prRxDesc;
};


struct up_down_queue_par
{
    UINT32 up;

};

struct ioctl_eth_mac_cmd
{

    UINT32 eth_cmd; /* see  ETH_MAC_CMD_TYPE*/
    union {
	struct reg_par reg;
    struct tx_descriptor_par tx_desc;
    struct rx_descriptor_par rx_desc;
    struct up_down_queue_par up_down_queue;


	} ifr_ifru;
};
struct ioctl_mdio_cmd
{
    UINT32 wr_cmd; /* 1: write, 0: read */
    UINT32 reg;    /* reg addr */
    UINT16 val;    /* value to write, or value been read */
    UINT16 rsvd;
};
struct ioctl_phy_ctrl_cmd
{
    UINT32 wr_cmd; /* 1: write, 0: read */
    UINT32 Prm;    /* prm */
    UINT32 val;    /* value to write, or value been read */
    UINT32 rsvd;
};

#if CONFIG_MW_CUSTOM_JSN

typedef enum {
    NET_IP_TYPE_UNKNOWN = 0,
    NET_IP_TYPE_V6_UNSPEC = 0x1,
    NET_IP_TYPE_V6_LOOPBACK = 0x2,
    NET_IP_TYPE_V6_MULTICAST = 0x4,
    NET_IP_TYPE_V6_LINKLOCAL = 0x8,
	NET_IP_TYPE_V6_SITELOCAL = 0x10,
    
    NET_IP_TYPE_V6_GLOBAL_PERNAMENT = 0x20,
    NET_IP_TYPE_V6_GLOBAL_TEMP = 0x40,		
    NET_IP_TYPE_V6_GLOBAL_DYNAMIC =0x80,	
    NET_IP_TYPE_V6_GLOBAL_UNIQUE =0x100,
    
	NET_IP_TYPE_V6_ALLCONFIG = 0x20 | 0x40 | 0x80 | 0x100, 
	NET_IP_TYPE_V6_GLOBAL = NET_IP_TYPE_V6_GLOBAL_PERNAMENT | NET_IP_TYPE_V6_GLOBAL_DYNAMIC, /*For App use*/
    
} NET_IP_TYPE_V6;

#else

typedef enum {
    NET_IP_TYPE_UNKNOWN = 0,
    NET_IP_TYPE_V6_UNSPEC = 0x1,
    NET_IP_TYPE_V6_LOOPBACK = 0x2,
    NET_IP_TYPE_V6_MULTICAST = 0x4,
    NET_IP_TYPE_V6_LINKLOCAL = 0x8,
	NET_IP_TYPE_V6_SITELOCAL = 0x10,
    
    NET_IP_TYPE_V6_GLOBAL_PERNAMENT = 0x20,
    NET_IP_TYPE_V6_GLOBAL_TEMP = 0x40,		
    NET_IP_TYPE_V6_GLOBAL_DYNAMIC =0x80,	
	NET_IP_TYPE_V6_GLOBAL = NET_IP_TYPE_V6_GLOBAL_PERNAMENT | NET_IP_TYPE_V6_GLOBAL_TEMP | NET_IP_TYPE_V6_GLOBAL_DYNAMIC,
    NET_IP_TYPE_V6_UNIQUE =0x100,
    
	NET_IP_TYPE_V6_ALLCONFIG = NET_IP_TYPE_V6_GLOBAL | NET_IP_TYPE_V6_UNIQUE, 
	NET_IP_TYPE_V6_VALIDIP = NET_IP_TYPE_V6_GLOBAL_PERNAMENT | NET_IP_TYPE_V6_GLOBAL_DYNAMIC | NET_IP_TYPE_V6_UNIQUE, /*For App use*/

} NET_IP_TYPE_V6;

#endif


#endif

