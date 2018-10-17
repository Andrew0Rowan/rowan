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
#include <pthread.h>
#include <sys/prctl.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <netdb.h>
#include <arpa/inet.h>


#include <unistd.h>
#include <sys/types.h>
#include <sys/time.h>

#include "request.h"
#include "utility.h"
#include "netinfd.h"
#include "x_mw_config.h"

#undef   DEBUG
#define DEBUG
/* NETINFD local definitions                                                    */

#ifdef DEBUG
#define ASSERT_CHECK(x)	assert(x)
#define DEBUG_PRINT(str, args...) \
	do { \
	printf("[NETINFD] "); \
	printf(str, ##args); \
	printf("\n"); \
	} while(0)
#else
#define ASSERT_CHECK(x)
#define DEBUG_PRINT(str, args...)
#endif

#ifndef PID_FILE
#define PID_FILE				"/var/run/NETINFD.pid"
#endif


/* NETINFD local prototypes                                                     */

typedef struct config {
  int  buse;
	int  linebufsz;
	char *key;
	char *value;
	char *linebuf;
}mtk_netconfig;


struct req {
	int	sock;
	struct netinfd_request request;
};

#define CONFIG_SIZE		16

/* NETINFD local variables                                                      */
mtk_netconfig ConfigHashTable[CONFIG_SIZE];
static struct sockaddr_in RequestSockAddr;
static struct req ReqList;
/*
static pthread_t ThreadReq;
static pthread_mutex_t MutexReqList, MutexConfig;
static pthread_cond_t ConditionReqList;
*/

/* NETINFD variables                                                            */
// DHCP
char *dhcpscript = "/sbin/dhcpc.script";
char *dhcpinfo = "/var/dhcpinfo";
// DNS
char *dnsscript = "/sbin/dns.script";
// Ifconfig
char *ifscript = "/sbin/ifconfig.script";
#define ip_addr_len 16
/* NETINFD local functions                                                      */
void reapchild(int sig)
{
	int status;

	while (waitpid(-1, &status, WNOHANG) > 0) {
		/* do nothing, just prevent from zombie children */
	}
}

int pidfile_acquire(void)
{
	int pidfd;

	pidfd = open(PID_FILE, O_CREAT | O_WRONLY, 0644);
	if (pidfd < 0) {
		DEBUG_PRINT("Can't open pidfile %s : %s !!", PID_FILE, strerror(errno));
	}
	else {
		lockf(pidfd, F_LOCK, 0);
	}

	return pidfd;
}


void pidfile_write_release(int pidfd)
{
	FILE *out;

	if (pidfd < 0) {
		return;
	}

	if ((out = fdopen(pidfd, "w")) != NULL) {
		fprintf(out, "%d\n", getpid());
		fclose(out);
	}

	lockf(pidfd, F_UNLCK, 0);
	close(pidfd);
}
/*
static void req_enqueue(struct req *req)
{
	pthread_mutex_lock(&MutexReqList);

	ReqList.sock = req->sock;
  memcpy(&ReqList.request, &req->request,  sizeof(struct netinfd_request));
	pthread_cond_signal(&ConditionReqList);
	pthread_mutex_unlock(&MutexReqList);
  DEBUG_PRINT("EnQ cond signal !!\n");
}


static void req_dequeue(struct req *preq)
{
	pthread_mutex_lock(&MutexReqList);
	if (ReqList.sock == -1)
  {
    DEBUG_PRINT("DeQ cond wait !!\n");
		pthread_cond_wait(&ConditionReqList, &MutexReqList);
	}
  preq->sock = ReqList.sock;
  memcpy(&preq->request, &ReqList.request, sizeof(struct netinfd_request));
  ReqList.sock = -1;

	pthread_mutex_unlock(&MutexReqList);

  DEBUG_PRINT("DeQ return the req !!\n");
}
*/

int hash_config(char *key)
{
	int i, v = 0;
	char *c;

	for (i = 31, c = key; i != 0 && *c != '\0'; i--, c++) {
		v += *c;
	}

	return (v % CONFIG_SIZE);
}

static mtk_netconfig *find_config(char *key)
{
  int i = 0;
  mtk_netconfig *pEmptyConfSlot = NULL;
  DEBUG_PRINT("--- Find config !!\n");

	for (i = 0; i < CONFIG_SIZE; i ++)
	{
    if (ConfigHashTable[i].buse == 1)
    {
      if (strcmp(key, ConfigHashTable[i].key) == 0)
      {
        return &ConfigHashTable[i];
      }
    }
    else
    {
      //assign the 1st empty slot.
      if (pEmptyConfSlot == NULL)
        pEmptyConfSlot = &ConfigHashTable[i];
    }
  }

	return pEmptyConfSlot;
}

static int config_set_runtime(char *key, char *value, mtk_netconfig *pEmpConf)
{
	size_t kLen, vLen, linebufsz;
	char *linebuf;

	kLen = strlen(key);
	vLen = strlen(value);
	linebufsz = kLen + vLen + 3;

	linebuf = malloc(linebufsz);
	if (!linebuf) {
		DEBUG_PRINT("Can't malloc for line of runtime configuration !!");
		return -1;
	}
  DEBUG_PRINT("--- Runtime linebuf malloc ok !!\n");

	pEmpConf->linebufsz = linebufsz;
	pEmpConf->key = linebuf;
	pEmpConf->value = &linebuf[kLen+2];
	strncpy(pEmpConf->key, key, kLen);
	strncpy(pEmpConf->value, value, vLen);
	pEmpConf->linebuf = linebuf;
  pEmpConf->buse = 1;
  DEBUG_PRINT("--- Runtime config assign ok !!\n");

	return 0;
}

static int config_set(char *key, char *value)
{
	char *reline;
	size_t kLen, vLen;
	mtk_netconfig *pConf = find_config(key);
  if (pConf == NULL)
  {
    DEBUG_PRINT("--- No empty and matched slot !!\n");
    return (-1);
  }
	else if (pConf && pConf->buse == 0)
  {
		/*
		 * runtime configuration,
		 * does not belong to non-volitile configurations
		 */
  	DEBUG_PRINT("--- Enter set runtime !!\n");
		return (config_set_runtime(key, value, pConf));
	}
  DEBUG_PRINT("--- Enter set config !!\n");

	kLen = strlen(key);
	vLen = strlen(value);
	ASSERT_CHECK(kLen == strlen(pConf->key));

	if (pConf->linebufsz > (kLen + vLen + 3)) {
    DEBUG_PRINT("--- Set config : copy in enough buffer!!\n");
		strncpy(pConf->value, value, vLen);
	}
	else {
		reline = realloc(pConf->linebuf, (kLen + vLen + 3));
		if (!reline) {
			DEBUG_PRINT("Can't realloc for set configuration !!");
			return -1;
		}
		pConf->linebufsz = kLen + vLen + 3;
		pConf->linebuf = reline;
		pConf->key = reline;
		pConf->value = &reline[kLen + 2];
		strncpy(pConf->value, value, vLen);
    DEBUG_PRINT("--- Set config : copy in reallocated buffer!!\n");
	}
	return 0;
}

static char *config_get(char *key)
{
	mtk_netconfig *pConf;

	pConf = find_config(key);
	if (!pConf || (pConf && pConf->buse == 0))
  {
		return NULL;
	}

	return (pConf->value);
}

int config_init(void)
{
	int i;

	/* initialize list and hash table */
	for (i = 0; i < CONFIG_SIZE; i++) {
		ConfigHashTable[i].buse = 0;
    ConfigHashTable[i].linebufsz = 0;
    ConfigHashTable[i].key = NULL;
    ConfigHashTable[i].value = NULL;
    ConfigHashTable[i].linebuf = NULL;
	}
	return 0;
}

/*
static void *request_thread(void *arg)
{
	int size, sock;
	struct req preq = {0};
	struct netinfd_request *request;

  char *argp [6];
  int pid, wpid, wstatus;
  int ret = 0;

	for (;;) {
		req_dequeue(&preq);
		if (preq.sock == -1) {
      DEBUG_PRINT("Thr --- No new req !!\n");
			continue;
		}

		sock = preq.sock;
		request = &preq.request;
    DEBUG_PRINT("Thr --- Req sock %d, type: %d magic: %d status: %d key %s !!\n", sock, request->type, request->magic, request->status, request->key);

		switch (request->type) {
	    	case NETINFD_REQUEST_TYPE_GET:

          DEBUG_PRINT("DeQ --- Get !!\n");
          pthread_mutex_lock(&MutexConfig);
          request->value = config_get(request->key);
          request->status = 0;

          size = pack_request(request);
          pthread_mutex_unlock(&MutexConfig);
	    		if (size < 0) {
	    			request->status = -1;
	    			request->value = NULL;
	    			size = pack_request(request);
	    		}
          DEBUG_PRINT("DeQ --- Get Write :%d !!\n", sizeof(struct netinfd_request));
				  safe_write(sock, request, sizeof(struct netinfd_request));
	    		close(sock);

	    		break;

	    	case NETINFD_REQUEST_TYPE_SET:

          DEBUG_PRINT("DeQ --- Set !!\n");
          pthread_mutex_lock(&MutexConfig);
          DEBUG_PRINT("DeQ --- Set key %s val %s!!\n", request->key, request->value);
          request->status = config_set(request->key, request->value);
          DEBUG_PRINT("DeQ --- Set ok : %d!!\n", request->status);
          pthread_mutex_unlock(&MutexConfig);
          DEBUG_PRINT("DeQ --- Set pack request!!\n");

	    		size = pack_request(request);
          DEBUG_PRINT("DeQ --- Set Write :%d !!\n", sizeof(struct netinfd_request));
          safe_write(sock, request, sizeof(struct netinfd_request));
	    		close(sock);

	    		break;

        case NETINFD_REQUEST_TYPE_DHCP:

          DEBUG_PRINT("DeQ --- DHCP !!\n");
          argp [0] = script;
          argp [1] = request->key;
          argp [2] = request->value;
          argp [3] = dhcpinfo;
          argp [4] = (char *)"0";
          argp [5] = NULL;

          pid = fork ();
          printf ("fork pid: %d\n", pid);
          if (pid < 0)
          {
            printf ("fork: %d\n", errno);
            wstatus = 0;
          }
          else if (pid)
          { //parent
            do {
              wpid = wait (&wstatus);
            } while (wpid != pid && wpid > 0);
            printf ("child %d terminated\n", wpid);
            printf ("child status : %d\n", WEXITSTATUS(wstatus));

            if (wpid < 0)
            {
              printf("wait: %d, err: %s\n", wpid, strerror(errno));
              wstatus = 0;
              request->status = -1;
            }
            else if (wstatus != 0)
            {
              request->status = -1;
            }
            else
            {
              request->status = 0;
            }
            pthread_mutex_lock(&MutexConfig);
            size = pack_request(request);
            pthread_mutex_unlock(&MutexConfig);

        		if (size < 0) {
        			request->status = -1;
        			request->value = NULL;
        			size = pack_request(request);
        		}
            DEBUG_PRINT("DeQ --- DHCP Write :%d !!\n", sizeof(struct netinfd_request));
    			  safe_write(sock, request, sizeof(struct netinfd_request));
        		close(sock);
          }
          else
          { // child
            ret = execv (script, argp);
            printf ("execve result = %d\n", ret);
            if (ret == -1)
            {
              printf("execve errno = %s\n", strerror(errno));
            }
            exit(0);
          }

	    		break;

		    default:
	    		close(sock);
	    		break;
	    }
	}

	pthread_exit(NULL);
}
*/
#define MAX_THREAD 10
pthread_t NetinfdTd[MAX_THREAD] = {0};
static int totalUsedThd = 0;
pthread_mutex_t lock_thrdParam; 

void FreeThisThread(void *In)
{
	pthread_t myThd = pthread_self();
	int i = 0;
	/*Set use_bit to 0, and total_num to*/
	for(; i<MAX_THREAD; i++)
	{
		if(NetinfdTd[i] == myThd)
		{
			NetinfdTd[i] = 0;
			totalUsedThd --;
			break;
		}
	}
}
void* request_handle(void* InParam)
{
	int size, acpt_sock;
	struct req *req;
 	struct netinfd_request *request;
  char s_command[64];

  char *argp [8];
  int pid, wpid, wstatus;
  int ret = 0;
  int autoip = 0;

  char s_ip[ip_addr_len];
  char s_mask[ip_addr_len];
  char s_gw[ip_addr_len];

  int count;
  struct hostent *pt_hostnet;
  char * server_url;

  prctl(PR_SET_NAME,"netinfd_handle",0,0,0);
  pthread_cleanup_push(FreeThisThread,NULL);
  pthread_detach(pthread_self());
	acpt_sock = *(int*)InParam;	
	pthread_mutex_unlock(&lock_thrdParam);	
	printf("Accept the request : %d !! & unlock \n", acpt_sock);
	if (acpt_sock < 0) {
		printf("Can't accept request socket : %s !!", strerror(errno));
		return NULL;
	}

	req = malloc(sizeof(struct req));
	if (!req) {
		printf("Can't malloc for pending request : %s !!", strerror(errno));
		close(acpt_sock);
		return NULL;
	}
	req->sock = acpt_sock;
	request = &req->request;

	size = safe_read(acpt_sock, request, sizeof(struct netinfd_request));
	if (size < 0) {
		printf("Can't read request : %s !!", strerror(errno));
		if (req != NULL)
        {
		    free(req);
        }		
		close(acpt_sock);
		return NULL;
  }

  printf("Safe read: %d !!\n",size);

	if (unpack_request_netinf(request, size) < 0) {
		printf("Can't unpack request !!");
		if (req != NULL)
        {
		    free(req);
        }
		close(acpt_sock);
		return NULL;
  }
  printf("Unpack req type %d !!\n", request->type);

    switch (request->type) {
    	case NETINFD_REQUEST_TYPE_GET:
        printf("DeQ --- Get !!\n");

        request->value = config_get(request->key);
        request->status = 0;

        size = pack_request_netinf(request);

    		if (size < 0) {
    			request->status = -1;
    			request->value = NULL;
    			size = pack_request_netinf(request);
    		}
        printf("DeQ --- Get Write :%d !!\n", sizeof(struct netinfd_request));
			  safe_write(acpt_sock, request, sizeof(struct netinfd_request));
    		close(acpt_sock);
        break;
    	case NETINFD_REQUEST_TYPE_SET:
        printf("DeQ --- Set key %s val %s!!\n", request->key, request->value);
        request->status = config_set(request->key, request->value);
        printf("DeQ --- Set ok : %d!!\n", request->status);
    		size = pack_request_netinf(request);
        printf("DeQ --- Set Write :%d !!\n", sizeof(struct netinfd_request));
        safe_write(acpt_sock, request, sizeof(struct netinfd_request));
    	close(acpt_sock);
    		//req_enqueue(req);
        //DEBUG_PRINT("Req ENQ %d!!\n", request->type);
		  break;

      case NETINFD_REQUEST_TYPE_DHCP:
        argp [0] = dhcpscript;
        argp [1] = request->key;
        argp [2] = request->value;
        argp [3] = dhcpinfo;
        argp [4] = (char *)"0";
        argp [5] = NULL;

        pid = fork ();
        printf ("fork pid: %d\n", pid);
        if (pid < 0)
        {
          printf ("fork: %d\n", errno);
          wstatus = 0;
        }
        else if (pid)
        { //parent

          do {
            wpid = waitpid (pid, &wstatus, 0);
          } while (wpid >= 0 && wpid != pid);

          printf ("child %d terminated\n", wpid);
          printf ("child status : %d\n", WEXITSTATUS(wstatus));

          if (wpid < 0)
          {
            printf("wait: %d, err: %s\n", wpid, strerror(errno));
            wstatus = 0;
            request->status = -1;
          }
          else if (wstatus != 0)
          {
            request->status = -1;
          }
          else
          {
            request->status = 0;
          }

          size = pack_request_netinf(request);

      		if (size < 0) {
      			request->status = -1;
      			request->value = NULL;
      			size = pack_request_netinf(request);
      		}
          DEBUG_PRINT("DHCP Write :%d !!\n", sizeof(struct netinfd_request));
  			  safe_write(acpt_sock, request, sizeof(struct netinfd_request));
      		close(acpt_sock);
        }
        else
        { // child
          ret = execv (dhcpscript, argp);
          printf ("execve result = %d\n", ret);
          if (ret == -1)
          {
            printf("execve errno = %s\n", strerror(errno));
          }
          exit(0);
        }
      break;

      case NETINFD_REQUEST_TYPE_IFCONF:
        argp[0] = ifscript;
        sscanf(request->value, "%16s %16s %16s %d",
             s_ip,  s_mask, s_gw, &autoip);
        argp[1] = (char *)"ip_mask";
        if (autoip == 1)
          argp[1] = (char *)"auto";

        argp[2] = (char *)request->key;
        DEBUG_PRINT("ifconf value : %s\n", request->value);
        argp [3] = (char *)s_ip;
        argp [4] = (char *)s_mask;
        argp [6] = (char *)s_gw;
        argp [5] = (char *)"gw";
        argp [7] = NULL;
        DEBUG_PRINT("ip: %s, mask: %s, gw: %s\n", argp[3], argp[4], argp[6]);

        pid = fork ();
        if (pid < 0)
        {
          printf ("fork: %d\n", errno);
          wstatus = 0;
        }
        else if (pid)
        { //parent

          do {
            wpid = waitpid (pid, &wstatus, 0);
          } while (wpid >= 0 && wpid != pid);

          printf ("child %d terminated\n", wpid);
          printf ("child status : %d\n", WEXITSTATUS(wstatus));

          if (wpid < 0)
          {
            printf("wait: %d, err: %s\n", wpid, strerror(errno));
            wstatus = 0;
            request->status = -1;
          }
          else if (wstatus != 0)
          {
            request->status = -1;
          }
          else
          {
            request->status = 0;
          }

          size = pack_request_netinf(request);

      		if (size < 0) {
      			request->status = -1;
      			request->value = NULL;
      			size = pack_request_netinf(request);
      		}
          DEBUG_PRINT("IFCONF Write :%d !!\n", sizeof(struct netinfd_request));
  			  safe_write(acpt_sock, request, sizeof(struct netinfd_request));
      		close(acpt_sock);
        }
        else
        { // child
          ret = execv (ifscript, argp);
          printf ("execve result = %d\n", ret);
          if (ret == -1)
          {
            printf("execve errno = %s\n", strerror(errno));
          }
          exit (0);
        }

      break;

      case NETINFD_REQUEST_TYPE_NIENABLE:
        argp [0] = request->key; // Net interface

        // Enable : 1, Disable : 0
        if (strcmp((const char *)request->value, (const char*)"y") == 0)
        {
          snprintf(s_command, 64, "ifconfig %s up", argp [0]);
        }
        else if (strcmp((const char *)request->value, (const char*)"n") == 0)
        {
          snprintf(s_command, 64, "ifconfig %s down", argp [0]);
        }
        else
        {
          close(acpt_sock);
          break;
        }

        printf("%s \n", s_command);
        system(s_command);

        ret = system(s_command);

        if (ret == 0)
        {
            request->status = 0;
        }
        else
        {
            request->status = -1;
        }

        request->status = 0;

        size = pack_request_netinf(request);

    		if (size < 0) {
    			request->status = -1;
    			size = pack_request_netinf(request);
    		}
        DEBUG_PRINT("Req reply :%d !!\n", sizeof(struct netinfd_request));
			  safe_write(acpt_sock, request, sizeof(struct netinfd_request));
        close(acpt_sock);

        break;

    case NETINFD_REQUEST_TYPE_DNSLOOKUP:
        count = atoi(request->value) * 100;
        server_url = request->key;
        pt_hostnet = NULL;
		printf("<--netinfd--> DNS lookup, time is %d\n", count);
        pid = fork ();
        printf ("fork pid: %d\n", pid);
        if (pid < 0)
        {
          printf ("fork: %d\n", errno);
          wstatus = 0;
        }
        else if (pid)
        { //parent
          do {
            wpid = waitpid (pid, &wstatus, WNOHANG);
            if(wpid != pid)
            {
                if(0 == count)
                {
                    printf("Timeout, kill childpid: %d\n", pid);
                    //kill child
                    kill(pid, SIGKILL);
                    usleep(5000);
                    waitpid (pid, &wstatus, WNOHANG);
                    wstatus = 1;
                    break;
                }
                else
                {
                    usleep(1000);
                    count = count - 1;
                }
            }
          } while (wpid != pid);
          printf ("child %d terminated\n", wpid);
          printf ("child status : %d\n", WEXITSTATUS(wstatus));

          if (wpid < 0)
          {
            printf("wait: %d, err: %s\n", wpid, strerror(errno));
            wstatus = 0;
            request->status = -1;
          }
          else if (0 != wstatus)
          {
            request->status = -1;
          }
          else
          {
            request->status = 0;
          }

          size = pack_request_netinf(request);

      		if (size < 0) {
      			request->status = -1;
      			request->value = NULL;
      			size = pack_request_netinf(request);
      		}
          DEBUG_PRINT("DNSLOOKUP Write :%d !!\n", sizeof(struct netinfd_request));
  			  safe_write(acpt_sock, request, sizeof(struct netinfd_request));
      		close(acpt_sock);
        }
        else
        { // child
          printf("Child: start to gethostbyname\n");
          pt_hostnet = gethostbyname(server_url);
          printf("Child: end gethostbyname\n");
          if (NULL == pt_hostnet)
          {
            exit(1);
          }
          exit(0);
        }
        break;

	    default:
    		close(acpt_sock);
    		break;
    }	
	printf("netinfd handle over !!\n");
    
	if (req) {
		DEBUG_PRINT("[netinfd] free pointer %p !!\n",req);
		free(req);
		
	}
    else
    {
        DEBUG_PRINT("[netinfd] NULL pointer %p !!\n",req);
    }
	pthread_cleanup_pop(1);
	return NULL;
}

static int request_init(void)
{
  	int reqfd;
	  const int reuse = 1;

    bzero(&RequestSockAddr, sizeof(RequestSockAddr));
    RequestSockAddr.sin_family = AF_INET;
    RequestSockAddr.sin_port = htons(NETINFD_SERV_PORT);
    //RequestSockAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    printf("Socket Addr ok !!\n");

    reqfd = inet_pton(AF_INET, "127.0.0.1", &RequestSockAddr.sin_addr);

    if (reqfd == 0)
    {
      printf("Src doesn't contain a valid network address !!\n");
      return -1;
    }
    else if (reqfd < 0)
    {
      printf("Address convert fail : %s !!\n", strerror(errno));
      return -1;
    }

    reqfd = socket(AF_INET, SOCK_STREAM, 0);
    if (reqfd == -1) {
    	DEBUG_PRINT("Can't create socket : %s !!", strerror(errno));
    	return -1;
    }

  	if (setsockopt(reqfd, SOL_SOCKET, SO_REUSEADDR, (const void *)&reuse, sizeof(reuse)) < 0) {
      	DEBUG_PRINT("Can't setsockopt SO_REUSEADDR : %s !!", strerror(errno));
  		  close(reqfd);
      	return -1;
  	}
    DEBUG_PRINT("Socket option Reuseaddr ok !!\n");

  	if (bind(reqfd, (struct sockaddr *)&RequestSockAddr,
  		  sizeof(RequestSockAddr)) < 0) {
       	DEBUG_PRINT("Can't bind : %s !!", strerror(errno));
  		  close(reqfd);
  		  return -1;
  	}
    DEBUG_PRINT("Bind OK, port: %d !!\n", NETINFD_SERV_PORT);

  	if (listen(reqfd, 5) < 0) {
        DEBUG_PRINT("Can't listen : %s !!", strerror(errno));
        close(reqfd);
  		  return -1;
  	}
    DEBUG_PRINT("Listen OK !!\n");

  	/* initialize request list */
    memset(&ReqList.request, 0, sizeof(struct netinfd_request));
    ReqList.sock = -1;

	  return reqfd;
}


/* SNCFGD functions                                                            */
int GetFreeThread(pthread_t **pp_freeThrd)
{
	int i=0;
	/*search a free thread - not complished yet*/
	for(i=0;i<MAX_THREAD;i++)
	{
		if(!NetinfdTd[i])
		{			
			*pp_freeThrd = NetinfdTd+i;
			totalUsedThd++;
			break;
		}
	}
	if(i == MAX_THREAD)
	{
		printf("<MW> Error, Netinfd no enough thread!!\n");
		return -1;
	}
	/*trace remaining connections*/
	printf("<MW> Use %d th netinfd thread in total %d connections !\n", i,totalUsedThd);
	return 0;
}


int main(int argc, char **argv)
{
  int reqfd;
  int ndes, maxfd;
  int acceptsock = 0;
  fd_set readfds;
  pthread_t* p_thread = NULL;
  int ret = 0;
  
#if CONFIG_SUPPORT_MW_NETWORK_CLOSE
  return 0;
#endif
  
  pthread_mutex_init(&lock_thrdParam,NULL);
  	
  //struct sched_param sched;

	/* realtime priority */
	/*sched.sched_priority = 89;
	if (sched_setscheduler(0, SCHED_FIFO, &sched) == -1) {
		DEBUG_PRINT("Can't set realtime scheduler : %s !!", strerror(errno));
	}*/
  printf("<netinfd>cfg daemon begins!!\n");

	/* initialize request interface */
	if ((reqfd = request_init()) < 0) {
		exit(-1);
	}

	/* it's time to become a daemon */
	/*
	signal(SIGCHLD, reapchild);
	pidfd = pidfile_acquire();
	pid = fork();
	if (pid < 0) {
		DEBUG_PRINT("Can't fork to become a daemon : %s  !!", strerror(errno));
		exit(-1);
	}
	if (pid != 0) {
		exit(0);
	}
	pidfile_write_release(pidfd);
  DEBUG_PRINT("Fork to become a daemon !!\n");
  */

	/* request processing thread */
  /*
	pthread_mutex_init(&MutexConfig, NULL);
	pthread_mutex_init(&MutexReqList, NULL);
	pthread_cond_init(&ConditionReqList, NULL);

	if (pthread_create(&ThreadReq, NULL, request_thread, (void *)NULL) != 0) {
		DEBUG_PRINT("Can't create thread for processing request : %s !!", strerror(errno));
		close(reqfd);
		exit(-1);
	}
	*/

  DEBUG_PRINT("Create the processing thread !!");

  /*
  signal(SIGPIPE, SIG_IGN);
  signal(SIGCHLD, SIG_IGN);
  signal(SIGINT, SIG_IGN);
  */

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
					printf("[MW]--- create thread fail, reason : %d \n----", ret);
			}
		}
	}

	return 0;
}


