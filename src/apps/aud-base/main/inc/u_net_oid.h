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




#ifndef U_NET_OID_H
#define U_NET_OID_H

#include "u_net_common.h"
/*
    The following OID is use to manage low level driver,
    in system, the OID is 4 bytes length and the first 2 byets
    is mask as driver group.
*/
#define GET_OID_GROUP(X)    (X&0xffff0000)

#define OID_GROUP_CMN_IF    0x00010000
#define OID_GROUP_ETHER     0x00020000
#define OID_GROUP_802_11    0x00040000

typedef enum
{
    OID_CMN_OID_START        =   OID_GROUP_CMN_IF,
    OID_CMN_IF_INDEX,                               /* param : INT32 */
    OID_CMN_IF_DESCR,                               /* param : CHAR[256] */
    OID_CMN_IF_TYPE,                                /* param : UINT32 */
    OID_CMN_IF_MTU,                                 /* param : UINT32 */
    OID_CMN_IF_SPEED,                               /* param : UINT32 */
    OID_CMN_IF_PHYADDRESS,                          /* param : MAC_ADDRESS_T */
    OID_CMN_IF_MULTICASTADDRESSLIST,                /* param : NET_MAC_ADDRESS_LIST_T */
    OID_CMN_IF_MULTICASTADDRESS,                    /* param : MAC_ADDRESS_T */
    OID_CMN_IF_ADMIN_STATUS,                        /* param : UINT32 */
    OID_CMN_IF_OPERSTATUS,                          /* param : UINT32 */
    OID_CMN_IF_LASTCHANGE,                          /* param : UINT32 */
    OID_CMN_IF_NAME,                                /* param : CHAR[256] */
    OID_CMN_IF_PROMISCUOUS_MODE,                    /* param : UINT32 */
    /* the following info will record in net_dev */
    OID_CMN_IF_IN_OCTETS,                           /* param : UINT64 */
    OID_CMN_IF_IN_UCASTPKT,                         /* param : UINT64 */
    OID_CMN_IF_IN_DISCARDS,                         /* param : UINT64 */
    OID_CMN_IF_IN_ERRORS,                           /* param : UINT64 */
    OID_CMN_IF_IN_UNKNOWN_PROTOS,                   /* param : UINT64 */
    OID_CMN_IF_OUT_OCTETS,                          /* param : UINT64 */
    OID_CMN_IF_OUT_UCASTPKTS,                       /* param : UINT64 */
    OID_CMN_IF_OUT_DISCARDS,                        /* param : UINT64 */
    OID_CMN_IF_OUT_ERRORS,                          /* param : UINT64 */
    OID_CMN_IF_IN_MULTICAST_PKTS,                   /* param : UINT64 */
    OID_CMN_IF_IN_BROADCAST_PKTS,                   /* param : UINT64 */
    OID_CMN_IF_OUT_MULTICAST_PKTS,                  /* param : UINT64 */
    OID_CMN_IF_OUT_BROADCAST_PKTS,                  /* param : UINT64 */
    OID_CMN_IF_IP_ADDRESS,                          /* param : UINT32, IP address (get only) */
    OID_CMN_IF_MULTICASTADDRESS_DEL,                /* remove multicast addr */
    OID_CMN_IF_CONNECT_STATUS,                      /* param : UINT32 */
    OID_CMN_IF_NIC_STATUS,                          /* param : UINT32 */
    OID_CMN_IF_CURRNET_SPEED_INFO,                  /* param : UINT32 */
    /* -------------------------------------------------------------- */
    OID_ETHER_OID_START     =   OID_GROUP_ETHER,
    OID_ETHER_ALIGNMENT_ERRORS,                     /* param : UINT32 */
    OID_ETHER_FCSERRORS,                            /* param : UINT32 */
    OID_ETHER_SINGLECOLLISIONFRAMES,                /* param : UINT32 */
    OID_ETHER_MULTIPLECOLLISIONFRAMES,              /* param : UINT32 */
    OID_ETHER_DEFERREDTRANSMISSIONS,                /* param : UINT32 */
    OID_ETHER_LATECOLLISIONS,                       /* param : UINT32 */
    OID_ETHER_EXCESSIVECOLLISIONS,                  /* param : UINT32 */
    OID_ETHER_INTERNALMACTRANSMITERRORS,            /* param : UINT32 */
    OID_ETHER_FRAMETOOLONGS,                        /* param : UINT32 */
    OID_ETHER_INTERNALMACRECEIVEERRORS,             /* param : UINT32 */
    OID_ETHER_SYMBOLERRORS,                         /* param : UINT32 */
    OID_ETHER_DUPLEXSTATUS,                         /* param : UINT32 */
    /* -------------------------------------------------------------- */
    OID_802_11_OID_START    =   OID_GROUP_802_11,
    OID_802_11_SSID,                                /* param : NET_802_11_SSID_T */
    OID_802_11_SCAN_,                                /* param : NULL */
    OID_802_11_BSSID_LIST_SCAN,                     /* param : NET_802_11_BSSID_LIST_T */
    OID_802_11_INFRASTRUCTURE_MODE,                 /* param : NET_802_11_NETWORK_INFRASTRUCTURE_T */
    OID_802_11_AUTHENTICATION_MODE,                 /* param : NET_802_11_AUTH_MODE_T */
    OID_802_11_DISASSOCIATE_,                        /* param : NULL */
    OID_802_11_TX_POWER_LEVEL,                      /* param : NET_802_11_TX_POWER_LEVEL_T */
    OID_802_11_POWER_MODE,                          /* param : NET_802_11_POWER_MODE_T */
    OID_802_11_RSSI,                                /* param : NET_802_11_RSSI_T */
    OID_802_11_ENCRYPTION_STATUS                    /* param : NET_802_11_ENCRYPTION_STATUS_T*/
    /* -------------------------------------------------------------- */

}   NET_OID_T;

typedef struct _NET_MAC_ADDRESS_LIST_S
{
    UINT32                ui4_NumberOfItems;
    MAC_ADDRESS_T         t_MulticastList [1];
}   NET_MAC_ADDRESS_LIST_T;

/* Received signal strength indication (in unit of dBm) */
typedef INT32          NET_802_11_RSSI_T;

/*--------------------------------------------------------------*/
/* Set/Query data rates.                                        */
/*--------------------------------------------------------------*/
#define NET_802_11_MAX_LEN_RATES                     8
typedef UINT8   NET_802_11_RATES_T[NET_802_11_MAX_LEN_RATES];

/*--------------------------------------------------------------*/
/* Set/Query SSID setting                                       */
/*--------------------------------------------------------------*/
/* Network types include OFDM 5G(a) and 2.4G(g) */
typedef enum
{
    IEEE_802_11_FH,
    IEEE_802_11_DS,
    IEEE_802_11_OFDM5,
    IEEE_802_11_OFDM24,
    IEEE_802_11_AUTOMODE,
    IEEE_802_11_NETWORK_TYPE_MAX                    /* Upper bound, not real case */
}   NET_802_11_NETWORK_TYPE_T;

typedef struct _NET_802_11_CONFIG_FH_S
{
    UINT32             ui4_Length;             /* Length of structure */
    UINT32             ui4_HopPattern;         /* Defined as 802.11 */
    UINT32             ui4_HopSet;             /* to one if non-802.11 */
    UINT32             ui4_DwellTime;          /* In unit of Kusec */
}   NET_802_11_CONFIG_FH_T;

typedef struct _NET_802_11_CONFIG_S
{
    UINT32                          ui4_Length;             /* Length of structure */
    UINT32                          ui4_BeaconPeriod;       /* In unit of Kusec */
    UINT32                          ui4_ATIMWindow;         /* In unit of Kusec */
    UINT32                          ui4_DSConfig;           /* Channel frequency in unit of kHz */
    NET_802_11_CONFIG_FH_T          t_FHConfig;
}   NET_802_11_CONFIG_T;


/* Set/Query power saving mode */
typedef enum
{
    IEEE_802_11_POWER_MODE_CAM,
    IEEE_802_11_POWER_MODE_MAX_PSP,
    IEEE_802_11_POWER_MODE_FAST_PSP,
    IEEE_802_11_POWER_MODE_MAX                      /* Upper bound, not real case */
}   NET_802_11_POWER_MODE_T;

/* In unit of milliwatts */
typedef UINT32         NET_802_11_TX_POWER_LEVEL_T;

/* Encryption types */
typedef enum
{
    IEEE_802_11_WEP_ENABLED,
    IEEE_802_11_ENCRYPTION1_ENABLED = IEEE_802_11_WEP_ENABLED,
    IEEE_802_11_WEP_DISABLED,
    IEEE_802_11_ENCRYPTION_DISABLED = IEEE_802_11_WEP_DISABLED,
    IEEE_802_11_WEP_KEY_ABSENT,
    IEEE_802_11_ENCRYPTION1_KEY_ABSENT = IEEE_802_11_WEP_KEY_ABSENT,
    IEEE_802_11_WEP_NOT_SUPPORTED,
    IEEE_802_11_ENCRYPTION_NOT_SUPPORTED = IEEE_802_11_WEP_NOT_SUPPORTED,
    IEEE_802_11_ENCRYPTION2_ENABLED,
    IEEE_802_11_ENCRYPTION2_KEY_ABSENT,
    IEEE_802_11_ENCRYPTION3_ENABLED,
    IEEE_802_11_ENCRYPTION3_KEY_ABSENT
}   NET_802_11_ENCRYPTION_STATUS_T;

/* Set/Query network operation mode */
typedef enum
{
    IEEE_802_11_NET_TYPE_IBSS,                      /* Ad Hoc */
    IEEE_802_11_NET_TYPE_INFRA,                     /* Infrastructure STA */
    IEEE_802_11_NET_TYPE_AUTO_UNKNOWN,              /* Automatic to join BSS */
    IEEE_802_11_NET_TYPE_INFRA_MAX                  /* Upper bound, not real case */
}   NET_802_11_NETWORK_INFRASTRUCTURE_T;

typedef enum
{
    IEEE_802_11_A,
    IEEE_802_11_B,
    IEEE_802_11_G,
    IEEE_802_11_N
}   NET_802_11_PROTOCOL_T;

#endif
