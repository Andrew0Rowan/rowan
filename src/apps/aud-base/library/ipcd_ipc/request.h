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


#ifndef __REQUEST_H__
#define __REQUEST_H__


/*******************************************************************************/
/* REQUEST definitions                                                         */
/*******************************************************************************/
#define IPCD_REQUEST_MAGIC			0x8530
#define IPCD_REQUEST_MAX_SIZE		4196
#define IPCD_SERV_PORT 58530

#define IPCD_UNIX_DOMAIN_PATH "/tmp/ipcd"

#define IPCD_USE_UNIX_SOCKET 1

#define NETINFD_REQUEST_MAGIC			0x8530
#define NETINFD_REQUEST_MAX_SIZE		4096
#define NETINFD_SERV_PORT 55050
/*******************************************************************************/
/* REQUEST prototypes                                                          */
/*******************************************************************************/
enum {
	IPCD_REQUEST_TYPE_EXEC = 0,
	IPCD_REQUEST_TYPE_EXEC_ASYNC,
	
	IPCD_REQUEST_TYPE_SETENV,
	IPCD_REQUEST_TYPE_GETENV,

	IPCD_REQUEST_TYPE_RESERVED	
};


struct ipcd_request {
	unsigned int magic;
	unsigned int type;
	int status;
	int reservedLen;
	char *cmd;
	char *priv;
	char string[IPCD_REQUEST_MAX_SIZE];	/* enough ?? */
};

enum {
	NETINFD_REQUEST_TYPE_GET = 0,
	NETINFD_REQUEST_TYPE_SET,
	NETINFD_REQUEST_TYPE_DHCP,
	NETINFD_REQUEST_TYPE_IFCONF,
	NETINFD_REQUEST_TYPE_NIENABLE,
	NETINFD_REQUEST_TYPE_DNSLOOKUP,
	NETINFD_REQUEST_TYPE_RESERVED
};


struct netinfd_request {
	unsigned int magic;
	unsigned int type;
	int status;
	int reservedLen;
	char *key;
	char *value;
	char string[NETINFD_REQUEST_MAX_SIZE];	/* enough ?? */
};


/*******************************************************************************/
/* REQUEST variables                                                           */
/*******************************************************************************/

/*******************************************************************************/
/* REQUEST functions                                                           */
/*******************************************************************************/
extern int pack_request(struct ipcd_request *request);
extern int unpack_request(struct ipcd_request *request, int size);
extern int connect_request(void);
extern int accept_request(int sockfd);
extern int request_for_ipcd(struct ipcd_request *request);

extern int pack_request_netinf(struct netinfd_request *request);
extern int unpack_request_netinf(struct netinfd_request *request, int size);
extern int connect_request_netinf(void);
extern int request_for_netinfd(struct netinfd_request *request);
#endif	// __REQUEST_H__
