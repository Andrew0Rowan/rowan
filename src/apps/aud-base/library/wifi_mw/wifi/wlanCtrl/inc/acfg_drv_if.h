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


#ifndef __ACFG_DRV_INTERFACE_H
#define __ACFG_DRV_INTERFACE_H


#ifndef IFNAMSIZ
#define IFNAMSIZ 16
#endif

#define MAX_P2P_DEVICE_NAME_LEN 32


#define SIOCIWFIRSTPRIV	0x8BE0
#define RT_PRIV_IOCTL                   (SIOCIWFIRSTPRIV + 0x01)
#define RTPRIV_IOCTL_SET                (SIOCIWFIRSTPRIV + 0x02)
#define RTPRIV_IOCTL_E2P                (SIOCIWFIRSTPRIV + 0x07)    //ioctl to read/write eeprom
#define OID_GET_SET_TOGGLE              0x8000
#define OID_802_11_P2P_PERSISTENT_TABLE     (0x0846)


#define OID_DELETE_PERSISTENT_TABLE     (0x0847)
#define OID_DELETE_PERSISTENT_ENTRY    (0x0854)


#define MAC_ADDR_LEN    6
#define NDIS_802_11_LENGTH_SSID 32

#if CONFIG_WLAN_SW_FOR_CUSTOM_JSN
#define P2P_DEVICENAME_IN_PERSISTAB 1
#else
#define P2P_DEVICENAME_IN_PERSISTAB 0
#endif

#if P2P_DEVICENAME_IN_PERSISTAB
#define MAX_P2P_TABLE_SIZE		9 /*  Save Presistent entry, change from 8 to 9 as Sony's request */
#else
#define MAX_P2P_TABLE_SIZE		8 /*  Save Presistent entry */
#endif

#define IWSC_SUPPORT    1

/* START ---- For persistent group table ---- */
typedef struct _NDIS_802_11_SSID {
	unsigned int SsidLength;	/* length of SSID field below, in bytes; */
	/* this can be zero. */
	unsigned char Ssid[NDIS_802_11_LENGTH_SSID];	/* SSID information field */
} NDIS_802_11_SSID, *PNDIS_802_11_SSID;

#ifdef IWSC_SUPPORT
#define IWSC_MAX_SUB_MASK_LIST_COUNT	3
#endif /* IWSC_SUPPORT */

/* WSC configured credential */
typedef struct _WSC_CREDENTIAL {
	NDIS_802_11_SSID SSID;	/* mandatory */
	unsigned short AuthType;	/* mandatory, 1: open, 2: wpa-psk, 4: shared, 8:wpa, 0x10: wpa2, 0x20: wpa2-psk */
	unsigned short EncrType;	/* mandatory, 1: none, 2: wep, 4: tkip, 8: aes */
	unsigned char Key[64];		/* mandatory, Maximum 64 byte */
	unsigned short KeyLength;
	unsigned char MacAddr[MAC_ADDR_LEN];	/* mandatory, AP MAC address */
	unsigned char KeyIndex;		/* optional, default is 1 */
	unsigned char bFromUPnP;	/* TRUE: This credential is from external UPnP registrar */
	unsigned char Rsvd[2];		/* Make alignment */
#ifdef IWSC_SUPPORT
    unsigned short              IpConfigMethod;
    unsigned int              RegIpv4Addr;
    unsigned int              Ipv4SubMask;
    unsigned int              EnrIpv4Addr;
    unsigned int              AvaIpv4SubmaskList[IWSC_MAX_SUB_MASK_LIST_COUNT];
#endif /* IWSC_SUPPORT */
} WSC_CREDENTIAL, *PWSC_CREDENTIAL;

typedef struct _RT_P2P_PERSISTENT_ENTRY
{
	unsigned char		bValid;
	unsigned char		MyRule;		/* My rule is GO or Client  */
	unsigned char		Addr[MAC_ADDR_LEN];		/* this addr is to distinguish this persistent entry is for which mac addr   */
#if P2P_DEVICENAME_IN_PERSISTAB
    unsigned char        DeviceName[MAX_P2P_DEVICE_NAME_LEN + 1];    /* add device name as Sony's request */
#endif
	WSC_CREDENTIAL	Profile;				/*  profile's bssid is always the GO's bssid. */
} RT_P2P_PERSISTENT_ENTRY, *PRT_P2P_PERSISTENT_ENTRY;

/****************************** TYPES ******************************/


typedef __uint32_t __u32;
typedef __int32_t __s32;
typedef __uint16_t __u16;
typedef __int16_t __s16;
typedef __uint8_t __u8;
#ifndef __user
#define __user
#endif /* __user */

/* --------------------------- SUBTYPES --------------------------- */
/*
 *	Generic format for most parameters that fit in an int
 */
struct	iw_param
{
  __s32		value;		/* The value of the parameter itself */
  __u8		fixed;		/* Hardware should not use auto select */
  __u8		disabled;	/* Disable the feature */
  __u16		flags;		/* Various specifc flags (if any) */
};

/*
 *	For all data larger than 16 octets, we need to use a
 *	pointer to memory allocated in user space.
 */
struct	iw_point
{
  void __user	*pointer;	/* Pointer to the data  (in user space) */
  __u16		length;		/* number of fields or size in bytes */
  __u16		flags;		/* Optional params */
};
/*
 *	A frequency
 *	For numbers lower than 10^9, we encode the number in 'm' and
 *	set 'e' to 0
 *	For number greater than 10^9, we divide it by the lowest power
 *	of 10 to get 'm' lower than 10^9, with 'm'= f / (10^'e')...
 *	The power of 10 is in 'e', the result of the division is in 'm'.
 */
struct	iw_freq
{
	__s32		m;		/* Mantissa */
	__s16		e;		/* Exponent */
	__u8		i;		/* List index (when in range struct) */
	__u8		flags;		/* Flags (fixed/auto) */
};

/*
 *	Quality of the link
 */
struct	iw_quality
{
	__u8		qual;		/* link quality (%retries, SNR,
					   %missed beacons or better...) */
	__u8		level;		/* signal level (dBm) */
	__u8		noise;		/* noise level (dBm) */
	__u8		updated;	/* Flags to know if updated */
};

union	iwreq_data
{
	/* Config - generic */
	char		name[IFNAMSIZ];
	/* Name : used to verify the presence of  wireless extensions.
	 * Name of the protocol/provider... */

	struct iw_point	essid;		/* Extended network name */
	struct iw_param	nwid;		/* network id (or domain - the cell) */
	struct iw_freq	freq;		/* frequency or channel :
					 * 0-1000 = channel
					 * > 1000 = frequency in Hz */

	struct iw_param	sens;		/* signal level threshold */
	struct iw_param	bitrate;	/* default bit rate */
	struct iw_param	txpower;	/* default transmit power */
	struct iw_param	rts;		/* RTS threshold threshold */
	struct iw_param	frag;		/* Fragmentation threshold */
	__u32		mode;		/* Operation mode */
	struct iw_param	retry;		/* Retry limits & lifetime */

	struct iw_point	encoding;	/* Encoding stuff : tokens */
	struct iw_param	power;		/* PM duration/timeout */
	struct iw_quality qual;		/* Quality part of statistics */

	struct sockaddr	ap_addr;	/* Access point address */
	struct sockaddr	addr;		/* Destination address (hw/mac) */

	struct iw_param	param;		/* Other small parameters */
	struct iw_point	data;		/* Other large parameters */
};

/*
 * The structure to exchange data for ioctl.
 * This structure is the same as 'struct ifreq', but (re)defined for
 * convenience...
 * Do I need to remind you about structure size (32 octets) ?
 */
struct	iwreq 
{
	union
	{
		char	ifrn_name[IFNAMSIZ];	/* if name, e.g. "eth0" */
	} ifr_ifrn;

	/* Data part (defined just above) */
	union	iwreq_data	u;
};




/* END ---- For persistent group table ---- */

/*Start ------ For get p2p associate table*/
typedef struct _P2P_ETHER_ADDR
{
	unsigned char octet[6];
} P2P_ETHER_ADDR, *PP2P_ETHER_ADDR;

typedef struct _P2P_PEER_SSID
{
	unsigned char ssid[32+1];
} P2P_PEER_SSID, *PP2P_PEER_SSID;

typedef struct _P2P_PEER_DEV_TYPE
{
	unsigned char dev_type[8];
} P2P_PEER_DEV_TYPE, *PP2P_PEER_DEV_TYPE;

typedef struct _P2P_STA_ASSOC_LIST
{
	P2P_ETHER_ADDR	maclist[3];
	unsigned int			maclist_count;
	P2P_PEER_SSID	device_name[3];
	P2P_PEER_DEV_TYPE	device_type[3];
	unsigned char			is_p2p[3];
	unsigned char                      rssi[3];
	unsigned char			is_wfd[3];
	unsigned short			rtsp_port[3];
} P2P_STA_ASSOC_LIST, *PP2P_STA_ASSOC_LIST;

/*End ------ For get p2p associate table*/

#endif
