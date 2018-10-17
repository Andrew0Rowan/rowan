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
#include <assert.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>

#include <unistd.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <arpa/inet.h>

#include "request.h"
#include "utility.h"
#include "u_ipcd.h"
#include "pthread.h"
#include "ipcd.h"
//#include "sys_config.h"

/* IPCD local prototypes                                                     */

struct req {
	int	sock;
	struct ipcd_request request;
};

/* IPCD local variables                                                      */
static struct sockaddr_in RequestSockAddr;
/* IPCD variables                                                            */

#define ip_addr_len 16
/* IPCD local functions                                                      */
/*
static void reapchild(int sig)
{
	int status;

	while (waitpid(-1, &status, WNOHANG) > 0) {
		// do nothing, just prevent from zombie children
	}
}
*/


void *async_exec_thread(void * pt_req)
{
    struct req *req;
 	struct ipcd_request *request;
 	int i4_system_ret;
 	int size, acpt_sock;

 	req = (struct req *) pt_req;
 	request = &req->request;
 	acpt_sock = req->sock;

 	i4_system_ret = system(request->cmd);

    if (i4_system_ret == 0)
    {
        request->status = 0;
    }
    else
    {
        request->status = -1;
    }

    size = pack_request(request);

    if (size < 0) {
    	request->status = -1;
    	size = pack_request(request);
    }
    DEBUG_PRINT("Req reply :%d !!\n", sizeof(struct ipcd_request));
    DEBUG_PRINT("acpt_sock:%d, request:%p, size:%d", acpt_sock, &request, sizeof(struct ipcd_request));
	safe_write(acpt_sock, request, sizeof(struct ipcd_request));
    close(acpt_sock);
    free(req);
    return NULL;

}

#define MAX_THREAD 20
pthread_t IpcdTd[MAX_THREAD] = {0};
static int totalUsedThd = 0;
pthread_mutex_t lock_thrdParam; 


int GetFreeThread(pthread_t **pp_freeThrd)
{
	int i=0;
	/*search a free thread - not complished yet*/
	for(i=0;i<MAX_THREAD;i++)
	{
		if(!IpcdTd[i])
		{			
			*pp_freeThrd = IpcdTd+i;
			totalUsedThd++;
			break;
		}
	}
	if(i == MAX_THREAD)
	{
		DEBUG_PRINT("<MW> Error, ipcd no enough thread!!\n");
		return -1;
	}
	/*trace remaining connections*/
	DEBUG_PRINT("<MW> Use %d th ipcd thread in total %d connections !\n", i,totalUsedThd);
	return 0;
}



void FreeThisThread(void *In)
{
	pthread_t myThd = pthread_self();
	int i = 0;
	/*Set use_bit to 0, and total_num to*/
	for(; i<MAX_THREAD; i++)
	{
		if(IpcdTd[i] == myThd)
		{
			IpcdTd[i] = 0;
			totalUsedThd --;
			break;
		}
	}
}


void* request_handle(void* InParam)
{
	int size, acpt_sock;
	struct req *req;
 	struct ipcd_request *request;
	char *real_cmd_line = NULL;

    //char *argp [8];
    //int pid, wpid, wstatus;
 //   int ret = 0;
    int i4_system_ret;

	pthread_cleanup_push(FreeThisThread,NULL);
	pthread_detach(pthread_self());
	acpt_sock = *(int*)InParam; 
	pthread_mutex_unlock(&lock_thrdParam);  

	if (acpt_sock < 0)
    {
		DEBUG_ERROR("Can't accept request socket : %s !!", strerror(errno));
		return NULL;
	}
    DEBUG_PRINT("Accept the request : %d !!\n", acpt_sock);

	req = malloc(sizeof(struct req));
	if (!req) 
    {
		DEBUG_ERROR("Can't malloc for pending request : %s !!", strerror(errno));
		close(acpt_sock);
		return NULL;
	}
	req->sock = acpt_sock;
	request = &req->request;

	size = safe_read(acpt_sock, request, sizeof(struct ipcd_request));
	if (size < 0) 
    {
		DEBUG_ERROR("Can't read request : %s !!", strerror(errno));
		free(req);
		close(acpt_sock);
		return NULL;
    }

    DEBUG_PRINT("Safe read, size: %d, fd: %d!!\n", size, acpt_sock);

	if (unpack_request(request, size) < 0) 
    {
		DEBUG_ERROR("Can't unpack request !!");
		free(req);
		close(acpt_sock);
		return NULL;
    }
    DEBUG_PRINT("Unpack req type %d !!\n", request->type);

    switch (request->type) 
    {
    	case IPCD_REQUEST_TYPE_EXEC:
		case IPCD_REQUEST_TYPE_EXEC_ASYNC:
        /*
              * execute cmd, ex: system(request->cmd) or fork()
              * please add your code here, if cmd execution fails, also set request->status = -1.
              *
              */
			real_cmd_line = get_real_command(request->cmd);
			if(!real_cmd_line){
				DEBUG_ERROR("system() exec fail\n");
				request->status = -1;
				goto IPC_RESPONSE;
			}

			DEBUG_PRINT("[IPCD] system(%s)\n", real_cmd_line);
			
			i4_system_ret = system(real_cmd_line);

			free(real_cmd_line);

			if(i4_system_ret == -1 || !WIFEXITED(i4_system_ret))
			{
				DEBUG_ERROR("system() exec fail\n");
				request->status = -1;
				goto IPC_RESPONSE;
			}

			if(0 == WEXITSTATUS(i4_system_ret))
			{
				DEBUG_ERROR("shell excute successfully\n");
				request->status = 0;
			}
			else
			{
				DEBUG_ERROR("shell excute fail:%d\n", WEXITSTATUS(i4_system_ret));
				request->status = WEXITSTATUS(i4_system_ret);
			}

IPC_RESPONSE:
            size = pack_request(request);
        	if (size < 0) 
            {
        		request->status = -1;
        		size = pack_request(request);
        	}
            DEBUG_PRINT("Req reply :%d !!\n", i4_system_ret);
			safe_write(acpt_sock, request, sizeof(struct ipcd_request));
            close(acpt_sock);
            free(req);
            break;
			
		case IPCD_REQUEST_TYPE_SETENV:
            /*	Set enviroment virable of ipcd.	*/
            i4_system_ret = setenv(request->cmd, request->priv, 1);
			request->status = i4_system_ret?-1:0;
			
            size = pack_request(request);
        	if (size < 0)
            {
        		request->status = -1;
        		size = pack_request(request);
        	}
			safe_write(acpt_sock, request, sizeof(struct ipcd_request));
            close(acpt_sock);
            free(req);
            break;

		case IPCD_REQUEST_TYPE_GETENV:
            /*	Get enviroment virable of ipcd.	*/
			
            request->priv = getenv(request->cmd);
			request->status = request->priv?0:-1;
			DEBUG_PRINT("env value = %s\n", request->priv);
            size = pack_request(request);
			
			safe_write(acpt_sock, request, sizeof(struct ipcd_request));
            close(acpt_sock);
            free(req);
            break;

	    default:
    		close(acpt_sock);
    		free(req);
    		break;
    }	
	pthread_cleanup_pop(1);
	return NULL;
}

#if IPCD_USE_UNIX_SOCKET
#include <sys/un.h>
static int request_init(void)
{
	int ret;
  	int reqfd;
	struct sockaddr_un selfAddr;
	

	DEBUG_PRINT("Unix Socket Addr ok !!\n");

	unlink(IPCD_UNIX_DOMAIN_PATH);
	reqfd = socket(PF_UNIX, SOCK_STREAM, 0);
    if (reqfd < 0) {
    	DEBUG_PRINT("Can't create socket : %s !!", (char *)strerror(errno));
    	return -1;
    }

	memset(&selfAddr, 0, sizeof(selfAddr));
	selfAddr.sun_family = AF_UNIX;

	strncpy(selfAddr.sun_path, IPCD_UNIX_DOMAIN_PATH, sizeof(selfAddr.sun_path) -1);
	ret = bind(reqfd, (struct sockaddr *)&selfAddr, sizeof(selfAddr));
	if (ret == -1) {
		DEBUG_PRINT("Can't bind : %s !!", (char *)strerror(errno));
		  close(reqfd);
		  return -1;
	}

	DEBUG_PRINT("Bind %s OK!!\n", IPCD_UNIX_DOMAIN_PATH);
  	if (listen(reqfd, 5) < 0) {
        DEBUG_PRINT("Can't listen : %s !!", (char *)strerror(errno));
        close(reqfd);
  		  return -1;
  	}
    DEBUG_PRINT("Listen OK !!\n");

	return reqfd;

}
#else
static int request_init(void)
{
  	int reqfd;
	const int reuse = 1;

    bzero(&RequestSockAddr, sizeof(RequestSockAddr));
    RequestSockAddr.sin_family = AF_INET;
    RequestSockAddr.sin_port = htons(IPCD_SERV_PORT);
    //RequestSockAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    DEBUG_PRINT("Socket Addr ok !!\n");

    reqfd = inet_pton(AF_INET, "127.0.0.1", &RequestSockAddr.sin_addr);

    if (reqfd == 0)
    {
      DEBUG_PRINT("Src doesn't contain a valid network address !!\n");
      return -1;
    }
    else if (reqfd < 0)
    {
      DEBUG_PRINT("Address convert fail : %s !!\n", (char *)strerror(errno));
      return -1;
    }

    reqfd = socket(AF_INET, SOCK_STREAM, 0);
    if (reqfd == -1) {
    	DEBUG_PRINT("Can't create socket : %s !!", (char *)strerror(errno));
    	return -1;
    }

  	if (setsockopt(reqfd, SOL_SOCKET, SO_REUSEADDR, (const void *)&reuse, sizeof(reuse)) < 0) {
      	DEBUG_PRINT("Can't setsockopt SO_REUSEADDR : %s !!", (char *)strerror(errno));
  		  close(reqfd);
      	return -1;
  	}
    DEBUG_PRINT("Socket option Reuseaddr ok !!\n");

  	if (bind(reqfd, (struct sockaddr *)&RequestSockAddr,
  		  sizeof(RequestSockAddr)) < 0) {
       	DEBUG_PRINT("Can't bind : %s !!", (char *)strerror(errno));
  		  close(reqfd);
  		  return -1;
  	}
    DEBUG_PRINT("Bind OK, port: %d !!\n", IPCD_SERV_PORT);

  	if (listen(reqfd, 5) < 0) {
        DEBUG_PRINT("Can't listen : %s !!", (char *)strerror(errno));
        close(reqfd);
  		  return -1;
  	}
    DEBUG_PRINT("Listen OK !!\n");

	return reqfd;
}
#endif

/* IPCD functions                                                            */
int main(int argc, char **argv)
{
    int reqfd;
	//int reqfd, pidfd;
	//pid_t pid;
    int ndes, maxfd;
    int ret;
    fd_set readfds;    
    int acceptsock = 0;
    pthread_t* p_thread = NULL;

    #if CONFIG_SYS_BDP_DROP_ROOT
        #if CONFIG_SYS_SUPPORT_MP_SECURITY
        umask(022); //set default for process umask for MP mode.
        #else
        umask(002); //set default for process umask for none MP mode.
        #endif
    #endif

    pthread_mutex_init(&lock_thrdParam,NULL);
    DEBUG_PRINT("ipcd begins!!");

	/* initialize request interface */
	if ((reqfd = request_init()) < 0) {
		exit(-1);
	}

    maxfd = 0;
    FD_ZERO(&readfds);

    FD_SET(reqfd, &readfds);

    DEBUG_PRINT("Readset add : %d !!", reqfd);

    if (maxfd < reqfd)
    {
        maxfd = reqfd;
    }

	for (;;) {
		ndes = select((maxfd + 1), &readfds, NULL, NULL, NULL);
		if (ndes == -1) {
			/* select failed, may be interrupted, try again */
			continue;
		}
		else if (ndes == 0) {
			/* select timeout, should not be */
			continue;
		}
		else {
			if (FD_ISSET(reqfd, &readfds))	{
				GetFreeThread(&p_thread);
        		DEBUG_PRINT("Got incoming request & lock!!");
				pthread_mutex_lock(&lock_thrdParam);
				acceptsock = accept_request(reqfd);
				ret = pthread_create(p_thread,NULL,request_handle,&acceptsock);		
				if(ret != 0)
					DEBUG_PRINT("[MW]--- create thread fail, reason : %d \n----", ret);
			}
		}
	}

	return 0;
}


