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

/******************************************************************************
*                         C O M P I L E R   F L A G S
*******************************************************************************
*/

/******************************************************************************
*                    E X T E R N A L   R E F E R E N C E S
*******************************************************************************
*/
#include "mtk_custom_wlan_probe.h"

/******************************************************************************
*                              C O N S T A N T S
*******************************************************************************
*/
/* Microsoft (also used in Wi-Fi specs) 00:50:F2 */
#define OUI_MICROSOFT 0x0050f2
#define OUI_BROADCOM 0x00904c /* Broadcom (Epigram) */
#define VENDOR_HT_CAPAB_OUI_TYPE 0x33 /* 00-90-4c:0x33 */

#define WMM_OUI_TYPE 2
#define WMM_OUI_SUBTYPE_INFORMATION_ELEMENT 0
#define WMM_OUI_SUBTYPE_PARAMETER_ELEMENT 1
#define WMM_OUI_SUBTYPE_TSPEC_ELEMENT 2
#define WMM_VERSION 1

#define OUI_WFA 0x506f9a
#define P2P_OUI_TYPE 9
#define WFD_OUI_TYPE 10
#define HS20_INDICATION_OUI_TYPE 16
#define HS20_OSEN_OUI_TYPE 18

#define OUI_MEDIATEK 0x000ce7 /* MediaTek Information Element */

/******************************************************************************
*                             D A T A   T Y P E S
*******************************************************************************
*/
/* Parsed Information Elements */
struct ieee802_11_elems {
	const u8 *ssid;
	const u8 *supp_rates;
	const u8 *ds_params;
	const u8 *challenge;
	const u8 *erp_info;
	const u8 *ext_supp_rates;
	const u8 *wpa_ie;
	const u8 *rsn_ie;
	const u8 *wmm; /* WMM Information or Parameter Element */
	const u8 *wmm_tspec;
	const u8 *wps_ie;
	const u8 *supp_channels;
	const u8 *mdie;
	const u8 *ftie;
	const u8 *timeout_int;
	const u8 *ht_capabilities;
	const u8 *ht_operation;
	const u8 *vht_capabilities;
	const u8 *vht_operation;
	const u8 *vht_opmode_notif;
	const u8 *vendor_ht_cap;
	const u8 *p2p;
	const u8 *wfd;
	const u8 *link_id;
	const u8 *interworking;
	const u8 *qos_map_set;
	const u8 *hs20;
	const u8 *ext_capab;
	const u8 *bss_max_idle_period;
	const u8 *ssid_list;
	const u8 *osen;

	const u8 *vendor_custom;

	u8 ssid_len;
	u8 supp_rates_len;
	u8 ds_params_len;
	u8 challenge_len;
	u8 erp_info_len;
	u8 ext_supp_rates_len;
	u8 wpa_ie_len;
	u8 rsn_ie_len;
	u8 wmm_len; /* 7 = WMM Information; 24 = WMM Parameter */
	u8 wmm_tspec_len;
	u8 wps_ie_len;
	u8 supp_channels_len;
	u8 mdie_len;
	u8 ftie_len;
	u8 timeout_int_len;
	u8 ht_capabilities_len;
	u8 ht_operation_len;
	u8 vht_capabilities_len;
	u8 vht_operation_len;
	u8 vendor_ht_cap_len;
	u8 p2p_len;
	u8 wfd_len;
	u8 interworking_len;
	u8 qos_map_set_len;
	u8 hs20_len;
	u8 ext_capab_len;
	u8 ssid_list_len;
	u8 osen_len;

	u8 vendor_custom_len;
};

/******************************************************************************
*                            P U B L I C   D A T A
*******************************************************************************
*/


/******************************************************************************
*                           P R I V A T E   D A T A
*******************************************************************************
*/
static u8 *g_vendor_request = "Tell me AP info";
static u8 *g_vendor_reply = "SSID:Test PSK:12345678";

/******************************************************************************
*                                 M A C R O S
*******************************************************************************
*/

/******************************************************************************
*                   F U N C T I O N   D E C L A R A T I O N S
*******************************************************************************
*/


/******************************************************************************
*                              F U N C T I O N S
*******************************************************************************
*/
static inline u32 WPA_GET_BE24(const u8 *a)
{
	return (a[0] << 16) | (a[1] << 8) | a[2];
}

static inline void WPA_PUT_BE24(u8 *a, u32 val)
{
	a[0] = (val >> 16) & 0xff;
	a[1] = (val >> 8) & 0xff;
	a[2] = val & 0xff;
}

/*----------------------------------------------------------------------------*/
/*!
 * @brief This function is to parse vendor specific element
 *
 * @param pos        Pointer to the start of vendor specific information element
 * @param len        Length of vendor specific IE in octets
 * @param elems      Data structure for parsed elements
 *
 * @retval 0      Success to parse elems
 * @retval -1     Fail to parse elems
 */
/*----------------------------------------------------------------------------*/
int ieee802_11_parse_vendor_specific(const u8 *pos, size_t elen,
	struct ieee802_11_elems *elems)
{
	unsigned int oui;

	/* first 3 bytes in vendor specific information element are the IEEE
	* OUI of the vendor. The following byte is used a vendor specific
	* sub-type.
	*/
	if (elen < 4) {
		ALOGE("short vendor specific information element ignored (len=%lu)\n",
			(unsigned long) elen);
		return -1;
	}

	oui = WPA_GET_BE24(pos);
	switch (oui) {
	case OUI_MEDIATEK:
		elems->vendor_custom = pos;
		elems->vendor_custom_len = elen;
		break;
	case OUI_MICROSOFT:
		/* Microsoft/Wi-Fi information elements are further typed and subtyped */
		switch (pos[3]) {
		case 1:
			/* Microsoft OUI (00:50:F2) with OUI Type 1:
			* real WPA information element
			*/
			elems->wpa_ie = pos;
			elems->wpa_ie_len = elen;
			break;
		case WMM_OUI_TYPE:
			/* WMM information element */
			if (elen < 5) {
				ALOGE("short WMM information element ignored (len=%lu)\n",
					(unsigned long) elen);
				return -1;
			}
			switch (pos[4]) {
			case WMM_OUI_SUBTYPE_INFORMATION_ELEMENT:
			case WMM_OUI_SUBTYPE_PARAMETER_ELEMENT:
				/*
				* Share same pointer since only one of these
				* is used and they start with same data.
				* Length field can be used to distinguish the
				* IEs.
				*/
				elems->wmm = pos;
				elems->wmm_len = elen;
				break;
			case WMM_OUI_SUBTYPE_TSPEC_ELEMENT:
				elems->wmm_tspec = pos;
				elems->wmm_tspec_len = elen;
				break;
			default:
				ALOGE("unknown WMM information element ignored (subtype=%d len=%lu)\n",
					pos[4], (unsigned long) elen);
				return -1;
			}
			break;
		case 4:
			/* Wi-Fi Protected Setup (WPS) IE */
			elems->wps_ie = pos;
			elems->wps_ie_len = elen;
			break;
		default:
			ALOGE("Unknown Microsoft information element ignored (type=%d len=%lu)\n",
				pos[3], (unsigned long) elen);
			return -1;
		}
		break;

	case OUI_WFA:
		switch (pos[3]) {
		case P2P_OUI_TYPE:
			/* Wi-Fi Alliance - P2P IE */
			elems->p2p = pos;
			elems->p2p_len = elen;
			break;
		case WFD_OUI_TYPE:
			/* Wi-Fi Alliance - WFD IE */
			elems->wfd = pos;
			elems->wfd_len = elen;
			break;
		case HS20_INDICATION_OUI_TYPE:
			/* Hotspot 2.0 */
			elems->hs20 = pos;
			elems->hs20_len = elen;
			break;
		case HS20_OSEN_OUI_TYPE:
			/* Hotspot 2.0 OSEN */
			elems->osen = pos;
			elems->osen_len = elen;
			break;
		default:
			ALOGE("Unknown WFA information element ignored (type=%d len=%lu)\n",
				pos[3], (unsigned long) elen);
			return -1;
		}
		break;

	case OUI_BROADCOM:
		switch (pos[3]) {
		case VENDOR_HT_CAPAB_OUI_TYPE:
			elems->vendor_ht_cap = pos;
			elems->vendor_ht_cap_len = elen;
			break;
		default:
			ALOGE("Unknown Broadcom information element ignored (type=%d len=%lu)\n",
				pos[3], (unsigned long) elen);
			return -1;
		}
		break;

	default:
		ALOGE("unknown vendor specific information element ignored (vendor OUI "
			"%02x:%02x:%02x len=%lu)\n",
			pos[0], pos[1], pos[2], (unsigned long) elen);
		return -1;
	}

	return 0;
}

/*----------------------------------------------------------------------------*/
/*!
 * @brief This function is to parse information elements in management frames
 *
 * @param start      Pointer to the start of IEs
 * @param len        Length of IE buffer in octets
 * @param elems      Data structure for parsed elements
 *
 * @retval 0      Success to parse elems
 * @retval -1     Fail to parse elems
 */
/*----------------------------------------------------------------------------*/
int ieee802_11_parse_elems(const u8 *start, size_t len,
	struct ieee802_11_elems *elems)
{
	size_t left = len;
	const u8 *pos = start;
	int unknown = 0;

	memset(elems, 0, sizeof(*elems));

	while (left >= 2) {
		u8 id, elen;

		id = *pos++;
		elen = *pos++;
		left -= 2;

		if (elen > left) {
			ALOGE("IEEE 802.11 element parse failed (id=%d elen=%d "
				"left=%lu)", id, elen, (unsigned long) left);
			return -1;
		}

		switch (id) {
		case WLAN_EID_SSID:
			elems->ssid = pos;
			elems->ssid_len = elen;
			break;
		case WLAN_EID_SUPP_RATES:
			elems->supp_rates = pos;
			elems->supp_rates_len = elen;
			break;
		case WLAN_EID_DS_PARAMS:
			elems->ds_params = pos;
			elems->ds_params_len = elen;
			break;
		case WLAN_EID_CF_PARAMS:
		case WLAN_EID_TIM:
			break;
		case WLAN_EID_CHALLENGE:
			elems->challenge = pos;
			elems->challenge_len = elen;
			break;
		case WLAN_EID_ERP_INFO:
			elems->erp_info = pos;
			elems->erp_info_len = elen;
			break;
		case WLAN_EID_EXT_SUPP_RATES:
			elems->ext_supp_rates = pos;
			elems->ext_supp_rates_len = elen;
			break;
		case WLAN_EID_VENDOR_SPECIFIC:
			if (ieee802_11_parse_vendor_specific(pos, elen, elems))
				unknown++;
			break;
		default:
			unknown++;
			break;
		}

		left -= elen;
		pos += elen;
	}

	if (left)
	{
		ALOGE("elem_len=%d, left=%d\n", len, left);
		return -1;
	}

	return 0;
}

/*----------------------------------------------------------------------------*/
/*!
 * @brief This function is to genarate vendor specific element
 *
 * @param vendor_ie      Pointer to the element buffer
 *
 * @retval element length
 */
/*----------------------------------------------------------------------------*/
int mtk_custom_gen_vendor_specific(u8 *vendor_ie, u8 *content, int content_len)
{
	u8 *pos, *len;

	if(vendor_ie == NULL)
		return 0;

	pos = vendor_ie;
	len = vendor_ie;
	*pos++ = WLAN_EID_VENDOR_SPECIFIC;
	len = pos++; /* to be filled */
	WPA_PUT_BE24(pos, OUI_MEDIATEK);
	pos += 3;

	memcpy(pos, content, content_len);
	pos += content_len;

	*len = pos -len -1; /* fill vendor len */

	return pos - vendor_ie;
}

/*
* 1.Customer parse probe request frame and get IE
* 2.Decide if it is wanted probe request frame
* 3.Prepare vendor IE which will send in probe response
* 4.Call mtk_custom_send_probe_response() to send probe response
*/
void custom_probe_req_demo(void *probe_req, int req_len)
{
	struct ieee80211_mgmt *mgmt;
	const u8 *ie;
	size_t ie_len;
	struct ieee802_11_elems elems;
	u8 vendor_ie[MAX_CUSTOM_VENDOR_LEN];
	int vendor_len = 0;
	u8 content[MAX_CUSTOM_VENDOR_LEN];

	mgmt = (struct ieee80211_mgmt *)probe_req;

	if(mgmt->frame_control != IEEE80211_FC(WLAN_FC_TYPE_MGMT, WLAN_FC_STYPE_PROBE_REQ))
	{
		//ALOGE("[Demo]Got packet from " MACSTR " type=%d is not probe-request\n",
		//	MAC2STR(mgmt->sa), mgmt->frame_control);
		return;
	}

	ie = mgmt->u.probe_req.variable;

	if (req_len < IEEE80211_HDRLEN + sizeof(mgmt->u.probe_req))
		return;

	ie_len = req_len - (IEEE80211_HDRLEN + sizeof(mgmt->u.probe_req));

	ALOGE("[Demo]Got ProbeReq from " MACSTR " len=%d\n",
		MAC2STR(mgmt->sa), req_len);

	/* 1 parse probe request ie fields */
	if (ieee802_11_parse_elems(ie, ie_len, &elems) != 0) {
		ALOGE("Could not parse ProbeReq from " MACSTR "\n", MAC2STR(mgmt->sa));
		return;
	}

	/* 2 Decide if it is wanted probe request frame from elems.vendor_custom */
	/* elems.vendor_custom is alibaba vendor now */
	memset(content, 0, MAX_CUSTOM_VENDOR_LEN);
	if(elems.vendor_custom != NULL && elems.vendor_custom_len > 3)
	{
		memcpy(content, elems.vendor_custom + 3, elems.vendor_custom_len - 3);
		ALOGI("custom_probe_req_demo[INFO] %s\n", content);
	}

	/* vondor content check */
	if(memcmp(content, g_vendor_request, strlen(g_vendor_request)) == 0)
	{
		/* 3.Prepare vendor IE which will send in probe response */
		vendor_len = mtk_custom_gen_vendor_specific(vendor_ie,
			g_vendor_reply, strlen(g_vendor_reply));

		/* 4 Call mtk_custom_send_probe_response() to send probe response */
		mtk_custom_send_probe_response(vendor_ie, vendor_len, probe_req);
	}
}

/*
* 1.Customer parse probe response frame and get IE
* 2.Decide if it is wanted probe request frame
* 3.Prepare vendor IE which will send in probe response
*/
void custom_probe_response_demo(void *probe_res, int resp_len)
{
	struct ieee80211_mgmt *mgmt;
	const u8 *ie;
	size_t ie_len;
	struct ieee802_11_elems elems;
	u8 content[MAX_CUSTOM_VENDOR_LEN];

	mgmt = (struct ieee80211_mgmt *)probe_res;

	if(mgmt->frame_control != IEEE80211_FC(WLAN_FC_TYPE_MGMT, WLAN_FC_STYPE_PROBE_RESP))
	{
		//ALOGE("[Demo]Got packet from " MACSTR " type=%d is not probe-response\n",
		//	MAC2STR(mgmt->sa), mgmt->frame_control);
		return;
	}

	ie = mgmt->u.probe_resp.variable;

	if (resp_len < IEEE80211_HDRLEN + sizeof(mgmt->u.probe_resp))
		return;

	ie_len = resp_len - (IEEE80211_HDRLEN + sizeof(mgmt->u.probe_resp));

	ALOGE("[Demo]Got ProbeResp from " MACSTR " len=%d\n",
		MAC2STR(mgmt->sa), resp_len);

	/* 1 parse probe response ie fields */
	if (ieee802_11_parse_elems(ie, ie_len, &elems) != 0) {
		ALOGE("Could not parse ProbeResp from " MACSTR "\n", MAC2STR(mgmt->sa));
		return;
	}

	/* 2 Decide if it is wanted probe request frame from elems.vendor_custom */
	//TODO elems.vendor_custom
	if(elems.vendor_custom != NULL && elems.vendor_custom_len > 3)
	{
		memset(content, 0, MAX_CUSTOM_VENDOR_LEN);
		memcpy(content, elems.vendor_custom + 3, elems.vendor_custom_len - 3);
		ALOGI("custom_probe_response_demo()[INFO] %s\n", content);
	}
}

void custom_send_probe_request(void)
{
	u8 vendor_ie[MAX_CUSTOM_VENDOR_LEN];
	int vendor_len;

	ALOGI("custom_send_probe_request() info:%s\n", g_vendor_request);

	vendor_len = mtk_custom_gen_vendor_specific(vendor_ie,
		g_vendor_request, strlen(g_vendor_request));

	/* Call mtk_custom_send_probe_request() to send probe request */
	mtk_custom_send_probe_request(vendor_ie, vendor_len);

	usleep(50*1000);
	system("wpa_cli -iwlan0 -p/tmp/wpa_supplicant scan");
}

/** The main for test !**/
int main(int argc, char *argv[]) {
	char cmd[10] = {0};

	if(!strcmp(argv[1], "recv-req-demo"))
	{
		mtk_custom_reg_probe_request_handling(custom_probe_req_demo);
	}
	else if(!strcmp(argv[1], "send-req-demo"))
	{
		mtk_custom_reg_probe_request_handling(custom_probe_response_demo);
	}
	else
	{
		printf("Unknown cmd!\n");
		return -1;
	}

	mtk_custom_init_communicate();

	printf("++main++\n");

	while(1)
	{
		printf("support cmd:");
		memset(cmd, 0, 10);
		scanf("%s", cmd);
		printf("\n input cmd=%s\n", cmd);
		if(!strcmp(cmd, "pkt_off"))
		{
			mtk_custom_deinit_communicate();
		}
		else if(!strcmp(cmd, "request"))
		{
			custom_send_probe_request();
		}
		else if(!strcmp(cmd, "exit"))
		{
			break;
		}
	}

	printf("--main--\n");

	return 0;
}

