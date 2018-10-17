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

#ifndef _MTK_CUSTOM_WLAN_PROBE_H_
#define _MTK_CUSTOM_WLAN_PROBE_H_

/*******************************************************************************
*                         C O M P I L E R   F L A G S
********************************************************************************
*/

/*******************************************************************************
*                    E X T E R N A L   R E F E R E N C E S
********************************************************************************
*/
#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <linux/genetlink.h>
#include <string.h>
#include <linux/if_packet.h>
#include <pthread.h>
#include <stdint.h>


/******************************************************************************
*                              C O N S T A N T S
*******************************************************************************
*/
#define MTK_CUSTOM_SAVE_PCAP_FILE               0
#define MTK_CUSTOM_PRINT_FRAME                  0

/* Address 4 excluded */
#define WLAN_MAC_MGMT_HEADER_LEN                24
/* 7.3.1.10 Timestamp field */
#define TIMESTAMP_FIELD_LEN                         8
/* 7.3.1.3 Beacon Interval field */
#define BEACON_INTERVAL_FIELD_LEN                   2
/* 7.3.1.4 Capability Information field */
#define CAP_INFO_FIELD_LEN                          2
/* 7.3.2 Element IDs of information elements */
#define ELEM_HDR_LEN                                2

/* Information Element IDs */
#define WLAN_EID_SSID 0
#define WLAN_EID_SUPP_RATES 1
#define WLAN_EID_FH_PARAMS 2
#define WLAN_EID_DS_PARAMS 3
#define WLAN_EID_CF_PARAMS 4
#define WLAN_EID_TIM 5
#define WLAN_EID_IBSS_PARAMS 6
#define WLAN_EID_COUNTRY 7
#define WLAN_EID_BSS_LOAD 11
#define WLAN_EID_CHALLENGE 16
#define WLAN_EID_ERP_INFO 42
#define WLAN_EID_EXT_SUPP_RATES 50
#define WLAN_EID_VENDOR_SPECIFIC 221

#define GL_CUSTOM_PROBE_RESP_SSID ("MTK_Probe_SSID")

#define WLAN_CAPABILITY_ESS ((unsigned int) 1U << 0)

#define WLAN_FC_TYPE_MGMT		0
#define WLAN_FC_TYPE_CTRL		1
#define WLAN_FC_TYPE_DATA		2

/* management */
#define WLAN_FC_STYPE_ASSOC_REQ		0
#define WLAN_FC_STYPE_ASSOC_RESP	1
#define WLAN_FC_STYPE_REASSOC_REQ	2
#define WLAN_FC_STYPE_REASSOC_RESP	3
#define WLAN_FC_STYPE_PROBE_REQ		4
#define WLAN_FC_STYPE_PROBE_RESP	5
#define WLAN_FC_STYPE_BEACON		8
#define WLAN_FC_STYPE_ATIM			9
#define WLAN_FC_STYPE_DISASSOC		10
#define WLAN_FC_STYPE_AUTH			11
#define WLAN_FC_STYPE_DEAUTH		12
#define WLAN_FC_STYPE_ACTION		13

#define IEEE80211_FC(type, stype) ((type << 2) | (stype << 4))

#define MAX_PROBERESP_LEN 1024

#define MAX_CUSTOM_VENDOR_LEN 256

#ifndef ETH_ALEN
#define ETH_ALEN 6
#endif

#ifndef MAC2STR
#define MAC2STR(a) (a)[0], (a)[1], (a)[2], (a)[3], (a)[4], (a)[5]
#define MACSTR "%02x:%02x:%02x:%02x:%02x:%02x"
#endif

#ifdef __GNUC__
#define STRUCT_PACKED __attribute__ ((packed))
#else
#define STRUCT_PACKED
#endif

/******************************************************************************
*                             D A T A   T Y P E S
*******************************************************************************
*/
typedef uint64_t u64;
typedef uint32_t u32;
typedef uint16_t u16;
typedef uint8_t u8;
typedef int64_t s64;
typedef int32_t s32;
typedef int16_t s16;
typedef int8_t s8;

typedef u16 be16;
typedef u16 le16;
typedef u32 be32;
typedef u32 le32;
typedef u64 be64;
typedef u64 le64;

struct ieee80211_hdr {
	le16 frame_control;
	le16 duration_id;
	u8 addr1[6];
	u8 addr2[6];
	u8 addr3[6];
	le16 seq_ctrl;
	/* followed by 'u8 addr4[6];' if ToDS and FromDS is set in data frame
	 */
} STRUCT_PACKED;

#define IEEE80211_HDRLEN (sizeof(struct ieee80211_hdr))

struct ieee80211_mgmt {
	le16 frame_control;
	le16 duration;
	u8 da[6];
	u8 sa[6];
	u8 bssid[6];
	le16 seq_ctrl;
	union {
		struct {
			/* only variable items: SSID, Supported rates */
			u8 variable[0];
		} STRUCT_PACKED probe_req;
		struct {
			u8 timestamp[8];
			le16 beacon_int;
			le16 capab_info;
			/* followed by some of SSID, Supported rates,
			 * FH Params, DS Params, CF Params, IBSS Params */
			u8 variable[0];
		} STRUCT_PACKED probe_resp;
	} u;
} STRUCT_PACKED;

struct pcap_pkthdr
{
	int tv_sec;
	int tv_usec;
	int caplen;
	int len;
};

struct pcap_file_header
{
	int   magic;
	short version_major;
	short version_minor;
	int   thiszone;   /* gmt to local correction */
	int   sigfigs;    /* accuracy of timestamps */
	int   snaplen;    /* max length saved portion of each pkt */
	int   linktype;   /* data link type (LINKTYPE_*) */
};

/**
* custom_probe_req_handler - custom probe request callback type
* @probe_req: probe request context data
* @req_len: probe request data length
*/
typedef void (*custom_probe_req_handler)(void *probe_req, int req_len);

/******************************************************************************
*                                 M A C R O S
*******************************************************************************
*/
#ifndef ALOGI
#define ALOGI printf
#endif
#ifndef ALOGE
#define ALOGE printf
#endif
#ifndef ALOGD
#define ALOGD printf
#endif


/*******************************************************************************
*                  F U N C T I O N   D E C L A R A T I O N S
********************************************************************************
*/

/*----------------------------------------------------------------------------*/
/*!
 * @brief This function is to init get wifi probe request/send probe response
 *        function
 *
 * @retval 0      Success to init
 * @retval -1     Fail to init
 */
/*----------------------------------------------------------------------------*/
int mtk_custom_init_communicate(void);

/*----------------------------------------------------------------------------*/
/*!
 * @brief This function is to deinit get wifi probe request/send probe response
 *        function
 *
 * @retval 0      Success to deinit
 * @retval -1     Fail to deinit
 */
/*----------------------------------------------------------------------------*/
int mtk_custom_deinit_communicate(void);

/*----------------------------------------------------------------------------*/
/*!
 * @brief This function is to register probe request packet callback handling
 *
 * @param handler         Pointer to custom_probe_req_handler type API
 *
 */
/*----------------------------------------------------------------------------*/
void mtk_custom_reg_probe_request_handling(custom_probe_req_handler handler);

/*----------------------------------------------------------------------------*/
/*!
 * @brief This function is to send probe response packet with specific vendor IE
 *
 * @param vendor_ie         Pointer to vendor IE buffer
 * @param vendor_len        Vendor IE buffer length
 * @param probe_req         Probe request frame buffer
 *
 * @retval 0      Success to send probe response
 * @retval -1     Fail to send probe response
 */
/*----------------------------------------------------------------------------*/
int mtk_custom_send_probe_response(void *vendor_ie, int vendor_len, void *probe_req);

/*----------------------------------------------------------------------------*/
/*!
 * @brief This function is to send probe request packet with specific vendor IE
 *
 * @param vendor_ie         Pointer to vendor IE buffer
 * @param vendor_len        Vendor IE buffer length
 *
 * @retval 0      Success to send probe response
 * @retval -1     Fail to send probe response
 */
/*----------------------------------------------------------------------------*/
int mtk_custom_send_probe_request(void *vendor_ie, int vendor_len);

#endif
