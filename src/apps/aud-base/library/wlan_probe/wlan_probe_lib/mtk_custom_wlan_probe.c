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
#define GL_CUSTOM_FAMILY_NAME      "WIFI_NL_CUSTOM"
#define MAX_CUSTOM_PKT_LENGTH      (2048)

enum {
	__GL_CUSTOM_ATTR_INVALID,
	GL_CUSTOM_ATTR_MSG,	/* message */
	__GL_CUSTOM_ATTR_MAX,
};
#define GL_CUSTOM_ATTR_MAX       (__GL_CUSTOM_ATTR_MAX - 1)

enum {
	__GL_CUSTOM_COMMAND_INVALID,
	GL_CUSTOM_COMMAND_BIND,	/* bind */
	GL_CUSTOM_COMMAND_SEND,	/* user -> kernel */
	GL_CUSTOM_COMMAND_RECV,	/* kernel -> user */
	__GL_CUSTOM_COMMAND_MAX,
};
#define GL_CUSTOM_COMMAND_MAX    (__GL_CUSTOM_COMMAND_MAX - 1)

#define GENLMSG_DATA(glh) ((void *)((long)NLMSG_DATA(glh) + GENL_HDRLEN))
#define GENLMSG_PAYLOAD(glh) (NLMSG_PAYLOAD(glh, 0) - GENL_HDRLEN)
#define NLA_DATA(na) ((void *)((char*)(na) + NLA_HDRLEN))
#define NLA_LEN(na) (((struct nlattr *)na)->nla_len - NLA_HDRLEN)

#define GL_CUSTOM_BIND ("BIND")
#define GL_CUSTOM_UNBIND ("UNBIND")

/******************************************************************************
*                             D A T A   T Y P E S
*******************************************************************************
*/
typedef struct _tagGenericNetlinkPacket {
	struct nlmsghdr n;
	struct genlmsghdr g;
	char buf[MAX_CUSTOM_PKT_LENGTH];
} GENERIC_NETLINK_PACKET, *P_GENERIC_NETLINK_PACKET;

/******************************************************************************
*                            P U B L I C   D A T A
*******************************************************************************
*/


/******************************************************************************
*                           P R I V A T E   D A T A
*******************************************************************************
*/
static int g_nl_sock;
static int g_nl_family_id;
static custom_probe_req_handler g_probe_req_handler = NULL;
static int g_thread_flag;
static u8 g_own_mac[ETH_ALEN];

#if MTK_CUSTOM_SAVE_PCAP_FILE
static FILE *gfile = NULL;
#endif

/******************************************************************************
*                                 M A C R O S
*******************************************************************************
*/

/******************************************************************************
*                   F U N C T I O N   D E C L A R A T I O N S
*******************************************************************************
*/
static int mtk_custom_send_cmd(int sk, __u16 nlmsg_type, __u32 nlmsg_pid,
	__u8 genl_cmd, __u16 nla_type, void* nla_data, int nla_len);

static int mtk_custom_get_family_id(int sk, const char *family_name);

static void mtk_custom_own_wlan_mac(void);

/* genarate probe response frame */
static u8 * mtk_custom_gen_probe_resp(void *vendor_ie, int vendor_len,
	const struct ieee80211_mgmt *probe_req, int *resp_len);

static u8 * mtk_custom_gen_probe_req(void *vendor_ie, int vendor_len, int *req_len);

#if MTK_CUSTOM_SAVE_PCAP_FILE
static void mtk_custom_save_one_frame(FILE * f, char * frame, int len);
#endif

static int mtk_custom_recv_one_wlan_frame(void);

static void mtk_custom_recv_wlan_frames(void);

static void mtk_custom_recv_packet_thread(void);

static void mtk_custom_dumpMemory8(char *buf, int len);


/******************************************************************************
*                              F U N C T I O N S
*******************************************************************************
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
int mtk_custom_init_communicate(void)
{
	struct sockaddr_nl nladdr;

	memset(&nladdr, 0, sizeof(nladdr));
	nladdr.nl_family = AF_NETLINK;
	nladdr.nl_pid = getpid();

	g_nl_sock = socket(AF_NETLINK, SOCK_RAW, NETLINK_GENERIC);
	if (g_nl_sock < 0)
	{
		ALOGE("Unable to create socket: %s\n", strerror(errno));
		return -1;
	}

	if (bind(g_nl_sock, (struct sockaddr *) &nladdr, sizeof(nladdr)) < 0)
	{
		ALOGE("Unable to bind socket: %d:%s\n", errno, strerror(errno));
		return -1;
	}

	g_nl_family_id = mtk_custom_get_family_id(g_nl_sock, GL_CUSTOM_FAMILY_NAME);
	if(!g_nl_family_id)
	{
		ALOGE("Unable to get family id: %s\n", strerror(errno));
		return -1;
	}
	ALOGI("get family id: %d\n", g_nl_family_id);

	mtk_custom_send_cmd(g_nl_sock, g_nl_family_id, getpid(), GL_CUSTOM_COMMAND_BIND,
		GL_CUSTOM_ATTR_MSG, GL_CUSTOM_BIND, strlen(GL_CUSTOM_BIND)+1);

	/* open wifi fw probe request function */
	system("iwpriv wlan0 set_mcr 2011 2011");
	ALOGI("sys cmd is: iwpriv wlan0 set_mcr 2011 2011\n");
	system("iwpriv wlan0 set_mcr 0x11111113 1");
	ALOGI("sys cmd is: iwpriv wlan0 set_mcr 0x11111113 1\n");

	mtk_custom_own_wlan_mac();

	/* create recv probe request thread */
	mtk_custom_recv_packet_thread();

	ALOGI("mtk_custom_init_communicate success\n");
	return 0;
}

/*----------------------------------------------------------------------------*/
/*!
 * @brief This function is to deinit get wifi probe request/send probe response
 *        function
 *
 * @retval 0      Success to deinit
 * @retval -1     Fail to deinit
 */
/*----------------------------------------------------------------------------*/
int mtk_custom_deinit_communicate(void)
{
	/* set recv thread exit flag */
	g_thread_flag = 0;
	
	mtk_custom_send_cmd(g_nl_sock, g_nl_family_id, getpid(), GL_CUSTOM_COMMAND_BIND,
		GL_CUSTOM_ATTR_MSG, GL_CUSTOM_UNBIND, strlen(GL_CUSTOM_UNBIND)+1);

	close(g_nl_sock);

	/* close wifi fw probe request function */
	system("iwpriv wlan0 set_mcr 2011 2011");
	ALOGI("sys cmd is: iwpriv wlan0 set_mcr 2011 2011\n");
	system("iwpriv wlan0 set_mcr 0x11111113 0");
	ALOGI("sys cmd is: iwpriv wlan0 set_mcr 0x11111113 0\n");

	ALOGI("mtk_custom_deinit_communicate success\n");
	return 0;
}

/*----------------------------------------------------------------------------*/
/*!
 * @brief This function is to register probe request packet callback handling
 *
 * @param handler         Pointer to custom_probe_req_handler type API
 *
 */
/*----------------------------------------------------------------------------*/
void mtk_custom_reg_probe_request_handling(custom_probe_req_handler handler)
{
	g_probe_req_handler = handler;
}

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
int mtk_custom_send_probe_response(void *vendor_ie, int vendor_len, void *probe_req)
{
	u8 *resp_frame = NULL;
	int resp_len = 0;

	if(vendor_len > MAX_CUSTOM_VENDOR_LEN || vendor_ie == NULL)
	{
		ALOGE("Input vendor ie invalid!\n");
		return -1;
	}

	resp_frame = mtk_custom_gen_probe_resp(vendor_ie, vendor_len, probe_req, &resp_len);
	if(resp_frame == NULL)
	{
		ALOGE("Genarate probe response frame error\n");
		return -1;
	}

#if MTK_CUSTOM_PRINT_FRAME
	ALOGI("[MTK]Send probe response frame:\n");
	mtk_custom_dumpMemory8(resp_frame, resp_len);
#endif

	mtk_custom_send_cmd(g_nl_sock, g_nl_family_id, getpid(), GL_CUSTOM_COMMAND_SEND,
		GL_CUSTOM_ATTR_MSG, resp_frame, resp_len);

	free(resp_frame);

	return 0;
}

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
int mtk_custom_send_probe_request(void *vendor_ie, int vendor_len)
{
#if 0
	u8 *req_frame = NULL;
	int req_len = 0;

	if(vendor_len > MAX_CUSTOM_VENDOR_LEN || vendor_ie == NULL)
	{
		ALOGE("Input vendor ie invalid!\n");
		return -1;
	}

	req_frame = mtk_custom_gen_probe_req(vendor_ie, vendor_len, &req_len);
	if(req_frame == NULL)
	{
		ALOGE("Genarate probe response frame error\n");
		return -1;
	}

#if MTK_CUSTOM_PRINT_FRAME
	ALOGI("[MTK]Send probe request frame:\n");
	mtk_custom_dumpMemory8(req_frame, req_len);
#endif

	mtk_custom_send_cmd(g_nl_sock, g_nl_family_id, getpid(), GL_CUSTOM_COMMAND_SEND,
		GL_CUSTOM_ATTR_MSG, req_frame, req_len);

	free(req_frame);
#endif

	u8 *req_frame = NULL;
	int req_len = 0;

	if(vendor_len > MAX_CUSTOM_VENDOR_LEN || vendor_ie == NULL)
	{
		ALOGE("Input vendor ie invalid!\n");
		return -1;
	}

	req_frame = malloc(vendor_len+10);
	if(req_frame == NULL)
	{
		ALOGE("Genarate probe response frame error\n");
		return -1;
	}

	memset(req_frame, 0, vendor_len+10);
	req_frame[0] = 0x40;
	req_frame[1] = 0x00;
	req_frame[2] = 0x00;
	req_frame[3] = 0x00;
	memcpy(req_frame+4, vendor_ie, vendor_len);
	req_len = vendor_len+4;

#if MTK_CUSTOM_PRINT_FRAME
	ALOGI("[MTK]Send probe request frame:\n");
	mtk_custom_dumpMemory8(req_frame, req_len);
#endif

	mtk_custom_send_cmd(g_nl_sock, g_nl_family_id, getpid(), GL_CUSTOM_COMMAND_SEND,
		GL_CUSTOM_ATTR_MSG, req_frame, req_len);

	free(req_frame);

	return 0;
}

/*
* send cmd to driver
*/
static int mtk_custom_send_cmd(int sk, __u16 nlmsg_type, __u32 nlmsg_pid,
	__u8 genl_cmd, __u16 nla_type, void* nla_data, int nla_len)
{
	struct nlattr *na;
	struct sockaddr_nl nladdr;
	int r, buflen;
	char *buf;
	GENERIC_NETLINK_PACKET msg;

	/* Fill msg attr */
	msg.n.nlmsg_len = NLMSG_LENGTH(GENL_HDRLEN);
	msg.n.nlmsg_type = nlmsg_type;
	msg.n.nlmsg_flags = NLM_F_REQUEST;
	msg.n.nlmsg_seq = 0;
	msg.n.nlmsg_pid = getpid();
	msg.g.cmd = genl_cmd;
	msg.g.version = 0x1;

	na = (struct nlattr *) GENLMSG_DATA(&msg);
	na->nla_type = nla_type;
	na->nla_len = nla_len + NLA_HDRLEN;
	memcpy(NLA_DATA(na), nla_data, nla_len);
	msg.n.nlmsg_len += NLMSG_ALIGN(na->nla_len);

	buf = (char *)&msg;
	buflen = msg.n.nlmsg_len;
	memset(&nladdr, 0, sizeof(nladdr));
	nladdr.nl_family = AF_NETLINK;

	while ((r = sendto(sk, buf, buflen, 0, (struct sockaddr *) &nladdr,
			sizeof(nladdr))) < buflen)
	{
		if (r > 0)
		{
			buf += r;
			buflen -= r;
		}
		else if (errno != EAGAIN)
		{
			ALOGE("%s failed\n", __func__);
			return -1;
		}
	}

	return 0;
}

/*
* Probe the controller in genetlink to find the family id
*/
static int mtk_custom_get_family_id(int sk, const char *family_name)
{
	struct nlattr *na;
	int rep_len;
	int id = -1;
	GENERIC_NETLINK_PACKET ans;

	if (mtk_custom_send_cmd(sk, GENL_ID_CTRL, getpid(), CTRL_CMD_GETFAMILY,
		CTRL_ATTR_FAMILY_NAME, (void *)family_name, strlen(family_name)+1) < 0)
	{
		ALOGE("%s failed\n", __func__);
		return -1;
	}

	rep_len = recv(sk, &ans, sizeof(ans), 0);
	if (rep_len < 0)
	{
		ALOGE("no response\n");
		return -1;
	}
	/* Validate response message */
	else if (!NLMSG_OK((&ans.n), (unsigned int)rep_len))
	{
		ALOGE("invalid reply message\n");
		return -1;
	}
	else if (ans.n.nlmsg_type == NLMSG_ERROR)
	{ /* error */
		ALOGE("received error\n");
		return -1;
	}

	na = (struct nlattr *) GENLMSG_DATA(&ans);
	na = (struct nlattr *) ((char *) na + NLA_ALIGN(na->nla_len));
	if (na->nla_type == CTRL_ATTR_FAMILY_ID)
	{
		id = *(__u16 *) NLA_DATA(na);
	}

	return id;
}

/*
* Get wlan0 mac address for probe response frame
*/
static void mtk_custom_own_wlan_mac(void)
{
	FILE *fp;
	u8 file_buffer[24];
	int i_file_size;
	
	system("cat /sys/class/net/wlan0/address > /tmp/mac_addr.tmp");

	fp = fopen("/tmp/mac_addr.tmp", "r");
	if(fp == NULL)
	{
		ALOGE("ERR:%s:%d: mac_addr.tmp open failed\n",__FUNCTION__, __LINE__);
		return;
	}

	fseek(fp, 0L, SEEK_END);
	i_file_size = ftell(fp);
	fseek(fp, 0L, SEEK_SET);

	memset(file_buffer, 0, 24);
	fread(file_buffer, i_file_size, 1, fp);

	fclose(fp);
	fp = NULL;

	sscanf(file_buffer, "%02x:%02x:%02x:%02x:%02x:%02x",
		&g_own_mac[0], &g_own_mac[1], &g_own_mac[2],
		&g_own_mac[3], &g_own_mac[4], &g_own_mac[5]);

	ALOGI("Get Mac %02x:%02x:%02x:%02x:%02x:%02x\n",
		g_own_mac[0], g_own_mac[1], g_own_mac[2],
		g_own_mac[3], g_own_mac[4], g_own_mac[5]);
}

/*
* Genarate probe response frame
*/
static u8 * mtk_custom_gen_probe_resp(void *vendor_ie, int vendor_len,
	const struct ieee80211_mgmt *probe_req, int *resp_len)
{
	struct ieee80211_mgmt *resp = NULL;
	u8 *pos = NULL;
	int buflen = MAX_PROBERESP_LEN;
	char *ssid = GL_CUSTOM_PROBE_RESP_SSID;
	int ssid_len = strlen(GL_CUSTOM_PROBE_RESP_SSID);
	u8 supp_rate[4] = {0x02, 0x04, 0x0b, 0x16};

	if (vendor_ie == NULL || vendor_len == 0 || probe_req == NULL || resp_len == 0)
	{
		ALOGE("mtk_custom_gen_probe_resp() param error\n");
		return NULL;
	}

	/* alloc buffer for probe response */
	resp = malloc(buflen);
	if (resp == NULL)
	{
		ALOGE("malloc buffer fail\n");
		return NULL;
	}

	memset(resp, 0, buflen);

	resp->frame_control = IEEE80211_FC(WLAN_FC_TYPE_MGMT,
		WLAN_FC_STYPE_PROBE_RESP);

	memcpy(resp->da, probe_req->sa, ETH_ALEN);
	memcpy(resp->sa, g_own_mac, ETH_ALEN);
	memcpy(resp->bssid, g_own_mac, ETH_ALEN);
	resp->seq_ctrl = 0;
	resp->u.probe_resp.timestamp[0] = 0;
	resp->u.probe_resp.beacon_int = 100;
	resp->u.probe_resp.capab_info = WLAN_CAPABILITY_ESS;

	pos = resp->u.probe_resp.variable;

	/* SSID */
	*pos++ = WLAN_EID_SSID;
	*pos++ = ssid_len;
	memcpy(pos, ssid, ssid_len);
	pos += ssid_len;

	/* Supported rates */
	*pos++ = WLAN_EID_SUPP_RATES;
	*pos++ = 4;
	memcpy(pos, supp_rate, 4);
	pos += 4;

#if 0
	/* DS Params */
	*pos++ = WLAN_EID_DS_PARAMS;
	*pos++ = 1;
	*pos++ = 13; /* channel 13*/
#endif

	/* Vendor specific */
	if (vendor_ie != NULL && vendor_len > 0) {
		memcpy(pos, vendor_ie, vendor_len);
		pos += vendor_len;
	}

	*resp_len = pos - (u8 *)resp;

	return (u8 *) resp;
}

/*
* Genarate probe request frame
*/
static u8 * mtk_custom_gen_probe_req(void *vendor_ie, int vendor_len, int *req_len)
{
	struct ieee80211_mgmt *resp = NULL;
	u8 *pos = NULL;
	int buflen = MAX_PROBERESP_LEN;
	char *ssid = NULL;
	int ssid_len = 0;
	u8 supp_rate[4] = {0x02, 0x04, 0x0b, 0x16};
	u8 da_mac[ETH_ALEN] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

	if (vendor_ie == NULL || vendor_len == 0 || req_len == 0)
	{
		ALOGE("mtk_custom_gen_probe_req() param error\n");
		return NULL;
	}

	/* alloc buffer for probe response */
	resp = malloc(buflen);
	if (resp == NULL)
	{
		ALOGE("malloc buffer fail\n");
		return NULL;
	}

	memset(resp, 0, buflen);

	resp->frame_control = IEEE80211_FC(WLAN_FC_TYPE_MGMT,
		WLAN_FC_STYPE_PROBE_REQ);

	memcpy(resp->da, da_mac, ETH_ALEN);
	memcpy(resp->sa, g_own_mac, ETH_ALEN);
	memcpy(resp->bssid, da_mac, ETH_ALEN);
	resp->seq_ctrl = 0;

	pos = resp->u.probe_req.variable;

	/* SSID */
	*pos++ = WLAN_EID_SSID;
	*pos++ = ssid_len;
	memcpy(pos, ssid, ssid_len);
	pos += ssid_len;

	/* Supported rates */
	*pos++ = WLAN_EID_SUPP_RATES;
	*pos++ = 4;
	memcpy(pos, supp_rate, 4);
	pos += 4;

#if 0
	/* DS Params */
	*pos++ = WLAN_EID_DS_PARAMS;
	*pos++ = 1;
	*pos++ = 13; /* channel 13*/
#endif

	/* Vendor specific */
	if (vendor_ie != NULL && vendor_len > 0) {
		memcpy(pos, vendor_ie, vendor_len);
		pos += vendor_len;
	}

	*req_len = pos - (u8 *)resp;

	return (u8 *) resp;
}

#if MTK_CUSTOM_SAVE_PCAP_FILE
/*
* For debug save probe request
*/
static void mtk_custom_save_one_frame(FILE * f, char * frame, int len)
{
	struct pcap_pkthdr hdr;

	memset(&hdr, 0, sizeof(hdr));
	hdr.caplen = hdr.len = len;
	fwrite(&hdr, sizeof(hdr), 1, f);
	fwrite(frame, len, 1, f);
}
#endif

static int mtk_custom_recv_one_wlan_frame(void)
{
	int fd = g_nl_sock;
	int rep_len, ret;
	fd_set rfds;
	struct timeval tv;
	GENERIC_NETLINK_PACKET msg;
	struct nlattr *na;

	u8 *probe_req;
	int probe_req_len;

	FD_ZERO(&rfds);
	FD_SET(fd, &rfds);

	/* Select timeout 1s */
	tv.tv_sec = 1;
	tv.tv_usec = 0;

	ret = select(fd + 1, &rfds, NULL, NULL, &tv);
	if (ret == 0)
	{
		return ret;
	}
	if(ret < 0)
	{
		perror("mtk_custom_recv_one_wlan_frame() select:");
		return ret;
	}

	rep_len = recv(g_nl_sock, &msg, sizeof(msg), 0);
	if(rep_len < 0 || msg.n.nlmsg_type == NLMSG_ERROR
		|| !NLMSG_OK((&msg.n), (unsigned int)rep_len))
		ALOGE("received error\n");

	rep_len = GENLMSG_PAYLOAD(&msg.n);
	na = (struct nlattr*) GENLMSG_DATA(&msg);

	switch(na->nla_type) {
	case GL_CUSTOM_ATTR_MSG:
		probe_req = (char *)NLA_DATA(na);
		probe_req_len = NLA_LEN(na);

#if MTK_CUSTOM_SAVE_PCAP_FILE
		mtk_custom_save_one_frame(gfile, probe_req, probe_req_len);
#endif

#if MTK_CUSTOM_PRINT_FRAME
		ALOGI("[MTK]Receive Probe request:\n");
		mtk_custom_dumpMemory8(probe_req, probe_req_len);
#endif

		if(g_probe_req_handler)
			g_probe_req_handler(probe_req, probe_req_len);

		break;
	default:
		ALOGE("unknown nla_type %d\n", na->nla_type);
	}


	ret = 1;

	return ret;
}

static void mtk_custom_recv_wlan_frames(void)
{
#if MTK_CUSTOM_SAVE_PCAP_FILE
	struct pcap_file_header file_hdr;

	memset(&file_hdr, 0, sizeof(file_hdr));
	file_hdr.magic = 0xa1b2c3d4;
	file_hdr.version_major = 2;
	file_hdr.version_minor = 4;
	file_hdr.snaplen = 0xFFFF;
	file_hdr.linktype = 105;
	
	gfile = fopen("/tmp/probe_request.pcap", "w");
	fwrite(&file_hdr, sizeof(file_hdr), 1, gfile);
#endif

	ALOGI("++mtk_custom_recv_wlan_frames++\n");
	while(g_thread_flag)
	{
		mtk_custom_recv_one_wlan_frame();
	}

	ALOGI("--mtk_custom_recv_wlan_frames--\n");

#if MTK_CUSTOM_SAVE_PCAP_FILE
	fclose(gfile);
#endif
}

static void mtk_custom_recv_packet_thread(void)
{
	pthread_t thread_id;
	pthread_attr_t attr;
	int ret;

	ALOGI("++mtk_custom_recv_packet_thread++\n");
	
	g_thread_flag = 1;

	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

	ret = pthread_create(&thread_id, &attr, (void*)mtk_custom_recv_wlan_frames, NULL);
	if(ret != 0)
	{
		ALOGE("[err] create thread error!\n");
		return;
	}

	ALOGI("--mtk_custom_recv_packet_thread--\n");
}

static void mtk_custom_dumpMemory8(char *buf, int len)
{
	printf("DUMP8 ADDRESS: %x, Length: %d\n", (int)buf, len);

	while (len > 0)
	{
		if (len >= 16)
		{
			printf("%02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x\n",
				buf[0], buf[1],buf[2], buf[3], buf[4], buf[5],buf[6], buf[7],
				buf[8], buf[9],buf[10], buf[11],buf[12],buf[13], buf[14], buf[15]);

			len -= 16;
			
			buf += 16;
		}
		else
		{
			switch (len)
			{
				case 1:
				{
					printf("%02x\n", buf[0]);
				}
				break;
				
				case 2:
				{
					printf("%02x %02x\n", buf[0], buf[1]);
				}
				break;
				
				case 3:
				{
					printf("%02x %02x %02x\n",buf[0], buf[1], buf[2]);
				}
				break;
				
				case 4:
				{
					printf("%02x %02x %02x %02x\n",buf[0], buf[1], buf[2], buf[3]);
				}
				break;
				
				case 5:
				{
					printf("%02x %02x %02x %02x %02x\n",buf[0], buf[1], buf[2],buf[3],buf[4]);
				}
				break;
				
				case 6:
				{
					printf("%02x %02x %02x %02x %02x %02x\n",
						buf[0], buf[1], buf[2],buf[3], buf[4], buf[5]);
				}
				break;
				
				case 7:
				{
					printf("%02x %02x %02x %02x %02x %02x %02x\n",
						buf[0], buf[1], buf[2], buf[3],buf[4], buf[5], buf[6]);
				}
				break;
				
				case 8:
				{
					printf("%02x %02x %02x %02x %02x %02x %02x %02x\n",
						buf[0], buf[1], buf[2], buf[3],buf[4], buf[5], buf[6], buf[7]);
				}
				break;
				
				case 9:
				{
					printf("%02x %02x %02x %02x %02x %02x %02x %02x %02x\n",
						buf[0], buf[1], buf[2], buf[3],buf[4],
						buf[5], buf[6], buf[7],buf[8]);
				}
				break;
				
				case 10:
				{
					printf("%02x %02x %02x %02x %02x %02x %02x %02x %02x %02x\n",
						buf[0], buf[1], buf[2], buf[3],buf[4],
						buf[5], buf[6], buf[7], buf[8],buf[9]);
				}
				break;
				
				case 11:
				{
					printf("%02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x\n",
						buf[0], buf[1], buf[2], buf[3],buf[4], buf[5],
						buf[6], buf[7], buf[8], buf[9],buf[10]);
				}
				break;
				
				case 12:
				{
					printf("%02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x\n",
						buf[0], buf[1], buf[2], buf[3],buf[4], buf[5],
						buf[6], buf[7], buf[8], buf[9],buf[10],buf[11]);
				}
				break;
				
				case 13:
				{
					printf("%02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x\n",
						buf[0], buf[1], buf[2], buf[3],buf[4], buf[5],buf[6],
						buf[7], buf[8], buf[9], buf[10],buf[11],buf[12]);
				}
				break;
				
				case 14:
				{
					printf("%02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x\n",
						buf[0], buf[1], buf[2], buf[3],buf[4], buf[5], buf[6], buf[7],
						buf[8], buf[9], buf[10], buf[11],buf[12], buf[13]);
				}
				break;
				
				case 15:
				{
					printf("%02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x\n",
						buf[0], buf[1], buf[2], buf[3],buf[4], buf[5], buf[6], buf[7],
						buf[8], buf[9], buf[10], buf[11],buf[12], buf[13], buf[14]);
				}
				break;
				
				default:
					break;
			}
			
			len = 0;
		}
	}

	printf("\n");
}

