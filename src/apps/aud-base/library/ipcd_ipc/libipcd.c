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
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <net/if.h>
#include <netinet/in.h>

#include "request.h"
#include "utility.h"
#include "u_dbg.h"
#include "u_ipcd.h"
#include <pthread.h>
/*******************************************************************************/
/* REQUEST local definitions                                                   */
/*******************************************************************************/
//#define DEBUG
#ifdef DEBUG
#define ASSERT_CHECK(x) assert(x)
#define DEBUG_PRINT(str, args...) \
    do { \
    printf("[IPC] "); \
    printf(str, ##args); \
    printf("\n"); \
    } while(0)
#else
#define ASSERT_CHECK(x)
#define DEBUG_PRINT(str, args...)
#endif
/*******************************************************************************/
/* IPCD local definitions                                                     */
/*******************************************************************************/

/*******************************************************************************/
/* IPCD local prototypes                                                      */
/*******************************************************************************/

/*******************************************************************************/
/* IPCD local variables                                                       */
/*******************************************************************************/

/*******************************************************************************/
/* IPCD local functions                                                       */
/*******************************************************************************/

/*******************************************************************************/
/* IPCD functions                                                             */
/*******************************************************************************/
static void ipcd_free_request(struct ipcd_request *request)
{
    if (NULL != request)
    {
        if (NULL != request->cmd)
        {
            free(request->cmd);
            request->cmd = NULL;
        }
        if (NULL != request->priv)
        {
            free(request->priv);
            request->priv = NULL;
        }
        free(request);
        request = NULL;
    }
}

int ipcd_set_env(char* envName, char* envVal)
{
	struct ipcd_request *request;
	int rc=0;

	assert(envName);
	assert(envVal);
	request = (struct ipcd_request *)malloc(sizeof(struct ipcd_request));
    if (NULL == request)
    {
    	DEBUG_PRINT("[IPCD] Malloc fail!\n");
        return -1;
    }

	bzero(request, sizeof(struct ipcd_request));
	
	
    DEBUG_PRINT("[IPCD]SET env: %s = %s \n", envName, envVal);

	request->magic = IPCD_REQUEST_MAGIC;
	request->type = IPCD_REQUEST_TYPE_SETENV;
	request->cmd = envName;
    request->priv = envVal;


	rc = request_for_ipcd(request);
	rc = (rc != 0)?-1:0;
	request->cmd = NULL;
	request->priv = NULL;
	
	ipcd_free_request(request);
	return rc;
}


int ipcd_get_env(char* envName, char* envVal)
{
	struct ipcd_request *request;	
	int rc=0;

	assert(envName);
	assert(envVal);

	request = (struct ipcd_request *)malloc(sizeof(struct ipcd_request));
    if (NULL == request)
    {
    	DEBUG_PRINT("[IPCD] Malloc fail!\n");
        return -1;
    }
	bzero(request, sizeof(struct ipcd_request));
	
    DEBUG_PRINT("[IPCD]GET env: %s \n", envName);

	request->magic = IPCD_REQUEST_MAGIC;
	request->type = IPCD_REQUEST_TYPE_GETENV;
	request->cmd = envName;

	rc = request_for_ipcd(request);
	rc = (rc != 0)?-1:0;
	if(!rc)
		strncpy(envVal, request->priv, strlen(request->priv));
	request->cmd = NULL;
	request->priv = NULL;
	ipcd_free_request(request);
	return 0;
}


int ipcd_exec(char *cmd, char *priv)
{
    struct ipcd_request *request;
	char *resCmd = NULL;
	char *resPriv = NULL;
	int rc = 0;
	
    request = (struct ipcd_request *)malloc(sizeof(struct ipcd_request));
    if (NULL == request)
    {
    	DEBUG_PRINT("[IPCD] Malloc fail!\n");
        return -1;
    }

	bzero(request, sizeof(struct ipcd_request));

    DEBUG_PRINT("[IPCD]CMD: %s\n", cmd);

	request->magic = IPCD_REQUEST_MAGIC;
	request->type = IPCD_REQUEST_TYPE_EXEC;

    if (NULL == cmd)
    {
        ipcd_free_request(request);
        return -1;
    }else{
    	request->cmd = (char *)malloc(strlen(cmd)+1);
        if (NULL == request->cmd)
        {
            ipcd_free_request(request);
            return -1;
        }
        bzero(request->cmd, strlen(cmd)+1);
        strncpy(request->cmd, cmd, strlen(cmd));
    }

    if (NULL != priv)
    {
        request->priv = (char *)malloc(strlen(priv)+1);
        if (NULL == request->priv)
        {
            ipcd_free_request(request);
            return -1;
        }
        bzero(request->priv, strlen(priv)+1);
        strncpy(request->priv, priv, strlen(priv));
    }
    
	resCmd = request->cmd;/*Jason : cmd & priv pointer has changed in flow, so reserve this position*/
	resPriv = request->priv;
	rc = request_for_ipcd(request);
	rc = (rc != 0)?-1:0;
	request->cmd = resCmd;
	request->priv = resPriv;
	
	ipcd_free_request(request);
	return rc;
}
int ipcd_exec_security(char *cmd, char *priv)
{
    struct ipcd_request *request;
	char *resCmd = NULL;
	char *resPriv = NULL;
	int rc = 0;
	
    request = (struct ipcd_request *)malloc(sizeof(struct ipcd_request));
    if (NULL == request)
    {
    	DEBUG_PRINT("[IPCD] Malloc fail!\n");
        return -1;
    }

	bzero(request, sizeof(struct ipcd_request));

    //printf("[IPC]CMD: %s\n", cmd);

	request->magic = IPCD_REQUEST_MAGIC;
	request->type = IPCD_REQUEST_TYPE_EXEC;

    if (NULL == cmd)
    {
        ipcd_free_request(request);
		DEBUG_PRINT("[IPCD] Empty command!\n");
        return -1;
    }else{
    	request->cmd = (char *)malloc(strlen(cmd)+1);
        if (NULL == request->cmd)
        {
            ipcd_free_request(request);
			DEBUG_PRINT("[IPCD] Memory error!\n");
            return -1;
        }
        bzero(request->cmd, strlen(cmd)+1);
        strncpy(request->cmd, cmd, strlen(cmd));
    }

    if (NULL != priv)
    {
        request->priv = (char *)malloc(strlen(priv)+1);
        if (NULL == request->priv)
        {
            ipcd_free_request(request);
			DEBUG_PRINT("[IPCD] Memory error!\n");
            return -1;
        }
        bzero(request->priv, strlen(priv)+1);
        strncpy(request->priv, priv, strlen(priv));
    }
    
	resCmd = request->cmd;/*Jason : cmd & priv pointer has changed in flow, so reserve this position*/
	resPriv = request->priv;
	rc = request_for_ipcd(request);
	request->cmd = resCmd;
	request->priv = resPriv;
	
	ipcd_free_request(request);

	//printf("[IPCD] ipcd_exec_security return:%d !\n", rc);
	return rc;
}



static void *ipcd_exec_async_thread(void * async_req)
{
    struct ipcd_request *request;
    IPCD_EXEC_CB_FP callback;

    //for storing allocated cmd pointer
    //because in the request_for_ipcd, request->cmd would lost its original pointer
    //please refer to pack_request_string() / unpack_request_string()
    char  *cmd_addr = NULL;

    request = (struct ipcd_request *)async_req;
    callback = (IPCD_EXEC_CB_FP) request->reservedLen;
    request->reservedLen = 0;
    cmd_addr = request->cmd;

    if (request_for_ipcd(request) != 0)
    {
        (*callback)(IPCD_EXEC_FAILED);
        request->cmd = cmd_addr;
        ipcd_free_request(request);
        return NULL;
    }

    (*callback)(IPCD_EXEC_SUCESS);
    request->cmd = cmd_addr;
    ipcd_free_request(request);
    return NULL;
}


int ipcd_exec_async(char *cmd, char *priv, IPCD_EXEC_CB_FP callback)
{
	struct ipcd_request *request;

    int ret;
    pthread_attr_t attr;
    pthread_t thread_id;

    request = (struct ipcd_request *)malloc(sizeof(struct ipcd_request));
    DEBUG_PRINT("[IPCD]Async CMD: %s\n", cmd);
	
    if (NULL == request)
    {
    	DEBUG_PRINT("[IPCD] Malloc req fail! %s\n", strerror(errno));    
        return -1;
    }
	bzero(request, sizeof(struct ipcd_request));

	request->magic = IPCD_REQUEST_MAGIC;
	request->type  = IPCD_REQUEST_TYPE_EXEC_ASYNC;

    if (NULL == cmd)
    {
        ipcd_free_request(request);
        return -1;
    }else{
    	request->cmd = (char *)malloc(strlen(cmd)+1);
        if (NULL == request->cmd)
        {    
	    	DEBUG_PRINT("[IPCD] Malloc cmd string fail! %s\n", strerror(errno));    
            ipcd_free_request(request);
            return -1;
        }
        bzero(request->cmd, strlen(cmd)+1);
        strncpy(request->cmd, cmd, strlen(cmd));
    }

    if (NULL != priv)
    {
        request->priv = (char *)malloc(strlen(priv)+1);
        if (NULL == request->priv)
        {    
			DEBUG_PRINT("[IPCD] Malloc priv string fail! %s\n", strerror(errno));    
            ipcd_free_request(request);
            return -1;
        }
        bzero(request->priv, strlen(priv)+1);
        strncpy(request->priv, priv, strlen(priv));
    }

    if (NULL == callback)
    {
		DEBUG_PRINT("[IPCD]callback is NULL!\n");	
        ipcd_free_request(request);
        return -1;
    }else{
        request->reservedLen = (int)callback;
    }

    /* Initialize and set thread detached attribute */
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

    ret = pthread_create(&thread_id, &attr, ipcd_exec_async_thread, (void *) request);

    if (ret)
    {
    	DEBUG_PRINT("[IPCD] Async create thread fail! - %s\n", strerror(errno));    
        ipcd_free_request(request);
        pthread_attr_destroy(&attr);
        return -1;
	}

    pthread_attr_destroy(&attr);
	return 0;
}

#define MAXINTERFACES 16


int net_get_validMac(char* pmac)
{
    register int fd, interface;
    struct ifreq buf[MAXINTERFACES] = {{{{0}}}};
    struct ifconf ifc;
	char defMac[6] = {0x00, 0x0C, 0xE7, 0x06, 0x00, 0x00};

    if ((fd = socket(AF_INET, SOCK_DGRAM, 0)) >= 0) {
		memcpy(buf[0].ifr_name, "eth0", 5);
		if (!(ioctl(fd, SIOCGIFHWADDR, (char *) &buf[0]))) {
			memcpy(pmac, buf[0].ifr_hwaddr.sa_data, 6);
			if(memcmp(pmac, defMac, 6)){
				DEBUG_PRINT("Find eth0 HW address:");
				DEBUG_PRINT("%02x%02x%02x%02x%02x%02x \n",
						(unsigned char)pmac[0],
						(unsigned char)pmac[1],
						(unsigned char)pmac[2],
						(unsigned char)pmac[3],
						(unsigned char)pmac[4],
						(unsigned char)pmac[5]); 
                close(fd);
				return 0;
			}
		}

		
        ifc.ifc_len = sizeof buf;
        ifc.ifc_buf = (caddr_t) buf;
        if (!ioctl(fd, SIOCGIFCONF, (char *) &ifc)) {
            interface = ifc.ifc_len / sizeof(struct ifreq);
            DEBUG_PRINT("interface num is interface=%d\n\n", interface);
            while (interface-- > 0) {
				if((!strcmp("lo", buf[interface].ifr_name))
				|| (!strcmp("eth0", buf[interface].ifr_name)))
									    continue;
				
                DEBUG_PRINT("net device %s\n", buf[interface].ifr_name);
/*Get HW ADDRESS of the net card */
                if (!(ioctl(fd, SIOCGIFHWADDR, (char *) &buf[interface]))) {
                    memcpy(pmac, buf[interface].ifr_hwaddr.sa_data, 6);
                    DEBUG_PRINT("HW address is:");
                    DEBUG_PRINT("%02x%02x%02x%02x%02x%02x \n",
                            (unsigned char)pmac[0],
                            (unsigned char)pmac[1],
                            (unsigned char)pmac[2],
                            (unsigned char)pmac[3],
                            (unsigned char)pmac[4],
                            (unsigned char)pmac[5]); 
                    close(fd);
                    return 0;        
                }

                else {
                    char str[256];

                    sprintf(str, "cpm: ioctl device %s",
                            buf[interface].ifr_name);
                    perror(str);
                }
            }                   //end of while
        } else
            perror("cpm: ioctl");

    } else
        perror("cpm: socket");

    close(fd);
    return -1;
}
