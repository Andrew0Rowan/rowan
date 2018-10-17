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
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <assert.h>
#include <unistd.h>
#include "netinfd.h"
#include "request.h"
#include "utility.h"

/* NETINFD local definitions                                                     */

/* NETINFD local prototypes                                                      */

/* NETINFD local variables                                                       */

/* NETINFD local functions                                                       */

/* NETINFD functions                                                             */
int netinfd_get(char *key, char **value)
{
	struct netinfd_request request;

	bzero(&request, sizeof(struct netinfd_request));

	request.magic = NETINFD_REQUEST_MAGIC;
	request.type = NETINFD_REQUEST_TYPE_GET;
	request.key = key;

	if (request_for_netinfd(&request) < 0 || request.key == NULL) {
		*value = NULL;
		return -1;
	}

	if (request.value == NULL) {
		*value = NULL;
	}
	else {
		*value = strdup(request.value);
	}

	return 0;
}

int netinfd_set(char *key, char *value)
{
	struct netinfd_request request;

	bzero(&request, sizeof(struct netinfd_request));

	request.magic = NETINFD_REQUEST_MAGIC;
	request.type = NETINFD_REQUEST_TYPE_SET;
	request.key = key;
	request.value = value;

	if (request_for_netinfd(&request) < 0) {
		return -1;
	}

	return 0;
}

int netinfd_dhcp(char *key, char *value)
{
	struct netinfd_request request;

	bzero(&request, sizeof(struct netinfd_request));

	request.magic = NETINFD_REQUEST_MAGIC;
	request.type = NETINFD_REQUEST_TYPE_DHCP;
	request.key = key;
	request.value = value;
	printf("DHCP cmd : %s, nif : %s\n", key, value);

	if (request_for_netinfd(&request) < 0) {
		return -1;
	}

	return 0;
}

int netinfd_ifconf(char *key, char *value)
{
	struct netinfd_request request;

	bzero(&request, sizeof(struct netinfd_request));

	request.magic = NETINFD_REQUEST_MAGIC;
	request.type = NETINFD_REQUEST_TYPE_IFCONF;
	request.key = key;
	request.value = value;
	printf("IFCONF nif : %s, info: %s\n", key, value);

	if (request_for_netinfd(&request) < 0) {
		return -1;
	}

	return 0;
}

int netinfd_nienable(char *key, char *value)
{
	struct netinfd_request request;
	printf("__%s:%d__\n",__FUNCTION__,__LINE__);

	bzero(&request, sizeof(struct netinfd_request));
	printf("__%s:%d__\n",__FUNCTION__,__LINE__);

	request.magic = NETINFD_REQUEST_MAGIC;
	request.type = NETINFD_REQUEST_TYPE_NIENABLE;
	
	printf("__%s:%d__\n",__FUNCTION__,__LINE__);
	request.key = key;
	request.value = value;
	printf("__%s:%d__\n",__FUNCTION__,__LINE__);
	if (request_for_netinfd(&request) < 0) {
		return -1;
	}

	return 0;
}

int netinfd_dnslookup(char *key, char *value)
{
	struct netinfd_request request;

	bzero(&request, sizeof(struct netinfd_request));

	request.magic = NETINFD_REQUEST_MAGIC;
	request.type =  NETINFD_REQUEST_TYPE_DNSLOOKUP;
	request.key = key;
	request.value = value;
	printf("DNS Lookup URL : %s, timeout : %s\n", key, value);

	if (request_for_netinfd(&request) < 0) {
		return -1;
	}

	return 0;
}

