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
#include <stddef.h>
#include <string.h>
#include <errno.h>
#include <assert.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ether.h>
#include <arpa/inet.h>
#include <sys/un.h>
#include "utility.h"
#include "request.h"



/*******************************************************************************/
/* REQUEST local definitions                                                   */
/*******************************************************************************/
//#define DEBUG
#ifdef DEBUG
#define ASSERT_CHECK(x) assert(x)
#define DEBUG_PRINT(str, args...) \
    do { \
    SYS_Printf("[IPC] "); \
    SYS_Printf(str, ##args); \
    SYS_Printf("\n"); \
    } while(0)
#else
#define ASSERT_CHECK(x)
#define DEBUG_PRINT(str, args...)
#endif

//#define DEBUG_NETINF
#ifdef DEBUG_NETINF
#define DEBUGN_PRINT(str, args...) \
    do { \
    SYS_Printf("[NETINFC] "); \
    SYS_Printf(str, ##args); \
    SYS_Printf("\n"); \
    } while(0)
#else
#define DEBUGN_PRINT(str, args...)
#endif

#define SYS_Printf printf
/*******************************************************************************/
/* REQUEST local prototypes                                                    */
/*******************************************************************************/


/*******************************************************************************/
/* REQUEST local variables                                                     */
/*******************************************************************************/


/*******************************************************************************/
/* REQUEST local functions                                                     */
/*******************************************************************************/
static int pack_request_string(char **p, char **next, const char *roof)
{
    if (*p == NULL)
  {
        return 0;
    }
    else
  {
        size_t len = strlen(*p) + 1;

        if ((roof - *next) < (ptrdiff_t)len)
    {
            return -1;
        }
        else
    {
            //strcpy(*next, *p);
            strncpy(*next, *p, len);
            *next += len;
            *p = (char *)0x12348530;
            return 0;
        }
    }
}


static int unpack_request_string(char **p, char **next, const char *roof)
{
    if (*p == NULL)
  {
        return 0;
    }
    else if (*p == (char *)0x12348530)
  {
        char *end = memchr(*next, '\0', roof - *next);

        if (end == NULL)
    {
            return -1;
        }
        else
    {
            *p = *next;
            *next = end + 1;
            return 0;
        }
    }
    return -1;
}

/*******************************************************************************/
/* REQUEST functions                                                           */
/*******************************************************************************/
int pack_request(struct ipcd_request *request)
{
    int size;
    char *next, *roof;
  int type, magic;

  next = request->string + request->reservedLen;
  roof = next + sizeof(request->string);

    if (request->magic != IPCD_REQUEST_MAGIC)
  {
    SYS_Printf("pack magic bad\n");
        return -1;
    }

  magic = request->magic;
  request->magic = htonl(magic);

    if (request->type >= IPCD_REQUEST_TYPE_RESERVED)
  {
    SYS_Printf("pack type bad\n");
        return -1;
    }

  type = request->type;
  request->type = htonl(type);

    if (pack_request_string(&request->cmd, &next, roof) < 0 ||
        pack_request_string(&request->priv, &next, roof) < 0)
  {
        SYS_Printf("pack cmd priv bad\n");
        return -1;
    }

    size = next - (char *)request;
    return size;
}


int unpack_request(struct ipcd_request *request, int size)
{
    char *next, *roof;
  int type, magic;

    next = request->string + request->reservedLen;
    roof = (char *)request + size;

    if (roof < next)
  {
    SYS_Printf("unpack roof smaller\n");
        return -1;
    }

  magic = request->magic;
  request->magic = ntohl(magic);

    if (request->magic != IPCD_REQUEST_MAGIC)
  {
    SYS_Printf("unpack magic bad\n");
        return -1;
    }

  type = request->type;
  request->type = ntohl(type);

    if (request->type >= IPCD_REQUEST_TYPE_RESERVED)
  {
    SYS_Printf("unpack type bad\n");
        return -1;
    }

    if (unpack_request_string(&request->cmd, &next, roof) < 0 ||
          unpack_request_string(&request->priv, &next, roof) < 0)
    {
        SYS_Printf("unpack cmd priv bad\n");
        return -1;
    }

    return 0;
}

int pack_request_netinf(struct netinfd_request *request)
{
    int size;
    char *next, *roof;
  int type, magic;

  next = request->string + request->reservedLen;
  roof = next + sizeof(request->string);

    if (request->magic != NETINFD_REQUEST_MAGIC)
  {
    SYS_Printf("pack magic bad\n");
        return -1;
    }

  magic = request->magic;
  request->magic = htonl(magic);

    if (request->type >= NETINFD_REQUEST_TYPE_RESERVED)
  {
    SYS_Printf("pack type bad\n");
        return -1;
    }

  type = request->type;
  request->type = htonl(type);

    if (pack_request_string(&request->key, &next, roof) < 0 ||
        pack_request_string(&request->value, &next, roof) < 0)
  {
        SYS_Printf("pack key value bad\n");
        return -1;
    }

    size = next - (char *)request;
    return size;
}


int unpack_request_netinf(struct netinfd_request *request, int size)
{
    char *next, *roof;
  int type, magic;

    next = request->string + request->reservedLen;
    roof = (char *)request + size;

    if (roof < next)
  {
    SYS_Printf("unpack roof smaller\n");
        return -1;
    }

  magic = request->magic;
  request->magic = ntohl(magic);

    if (request->magic != NETINFD_REQUEST_MAGIC)
  {
    SYS_Printf("unpack magic bad\n");
        return -1;
    }

  type = request->type;
  request->type = ntohl(type);

    if (request->type >= NETINFD_REQUEST_TYPE_RESERVED)
  {
    SYS_Printf("unpack type bad\n");
        return -1;
    }

    if (unpack_request_string(&request->key, &next, roof) < 0 ||
          unpack_request_string(&request->value, &next, roof) < 0)
    {
        SYS_Printf("unpack key value bad\n");
        return -1;
    }

    return 0;
}

#if IPCD_USE_UNIX_SOCKET
#include <sys/un.h>
int connect_request(void)
{
    int sock;
    struct sockaddr_un dstAddr;


    sock = socket(PF_UNIX, SOCK_STREAM, 0);
    if (sock == -1)
    {
        SYS_Printf("Socket create fail : %s !!\n", (char *)strerror(errno));
        return -1;
    }
	
    DEBUG_PRINT("Socket create ok: %d!!", sock);

    bzero(&dstAddr, sizeof(dstAddr));
    dstAddr.sun_family = AF_UNIX;
	strncpy(dstAddr.sun_path, IPCD_UNIX_DOMAIN_PATH, sizeof(dstAddr.sun_path) - 1);
	

    if (connect(sock, (struct sockaddr *)&dstAddr, sizeof(dstAddr)) < 0)
    {
        SYS_Printf("Connect to daemon fail : %s !!\n", (char *)strerror(errno));
        close(sock);
        return -1;
    }
    DEBUG_PRINT("Connect to daemon(%s) ok !!", IPCD_UNIX_DOMAIN_PATH);

    return sock;
}
#else
int connect_request(void)
{
    int sock;
    struct sockaddr_in dstaddr;

    bzero(&dstaddr, sizeof(dstaddr));
    dstaddr.sin_family = AF_INET;
    dstaddr.sin_port = htons(IPCD_SERV_PORT);
    //dstaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    sock = inet_pton(AF_INET, "127.0.0.1", &dstaddr.sin_addr);
    if (sock == 0)
    {
        SYS_Printf("Src doesn't contain a valid network address !!\n");
        return -1;
    }
    else if (sock < 0)
    {
        SYS_Printf("Address convert fail : %s !!\n", (char *)strerror(errno));
        return -1;
    }

    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == -1)
    {
        SYS_Printf("Socket create fail : %s !!\n", (char *)strerror(errno));
        return -1;
    }
    DEBUG_PRINT("Socket create ok: %d!!", sock);

    if (connect(sock, (struct sockaddr *)&dstaddr, sizeof(dstaddr)) < 0)
    {
        SYS_Printf("Connect to daemon fail : %s !!\n", (char *)strerror(errno));
        close(sock);
        return -1;
    }
    DEBUG_PRINT("Connect to daemon ok !!");

    return sock;
}
#endif

int connect_request_netinf(void)
{
    int sock;
    struct sockaddr_in dstaddr;

  bzero(&dstaddr, sizeof(dstaddr));
  dstaddr.sin_family = AF_INET;
  dstaddr.sin_port = htons(NETINFD_SERV_PORT);
  //dstaddr.sin_addr.s_addr = htonl(INADDR_ANY);
  sock = inet_pton(AF_INET, "127.0.0.1", &dstaddr.sin_addr);
  if (sock == 0)
  {
    SYS_Printf("Src doesn't contain a valid network address !!\n");
    return -1;
  }
  else if (sock < 0)
  {
    SYS_Printf("Address convert fail : %s !!\n", (char *)strerror(errno));
    return -1;
  }

    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == -1)
  {
    SYS_Printf("Socket create fail : %s !!\n", (char *)strerror(errno));
        return -1;
    }
  DEBUG_PRINT("Socket create ok: %d!!", sock);

    if (connect(sock, (struct sockaddr *)&dstaddr, sizeof(dstaddr)) < 0)
  {
    SYS_Printf("Connect to daemon fail : %s !!\n", (char *)strerror(errno));
        close(sock);
        return -1;
    }
  DEBUG_PRINT("Connect to daemon ok !!");

    return sock;
}

int accept_request(int sockfd)
{
    int sock;
    socklen_t addrlen;
    struct sockaddr_un addr;

    addrlen = sizeof(addr);
    sock = accept(sockfd, (struct sockaddr *)&addr, &addrlen);
    if (sock == -1)
    {
        SYS_Printf("Socket accept fail : %s !!\n", (char *)strerror(errno));
        return -1;
    }
    DEBUG_PRINT("Socket accept ok, fd : %d !!", sock);

    return sock;
}

int request_for_ipcd(struct ipcd_request *request)
{
    int sock, size;

    DEBUG_PRINT("claim req type: %d", request->type);

    size = pack_request(request);
    if (size < 0)
    {
        SYS_Printf("pack req fail\n");
        return -1;
    }
    DEBUG_PRINT("pack req ok");

    sock = connect_request();
    if (sock < 0)
    {
        SYS_Printf("connect fail\n");
        return -1;
    }
    DEBUG_PRINT("connect ok, fd: %d", sock);

    if (safe_write(sock, request, sizeof(struct ipcd_request)) != sizeof(struct ipcd_request))
    {
        SYS_Printf("safe write fail\n");
        close(sock);
        return -1;
    }
    //SYS_Printf("[IPC]client req safe write ok, fd: %d\n", sock);

    size = safe_read(sock, request, sizeof(struct ipcd_request));

	if (size < 0 || unpack_request(request, size) < 0)
    {
        SYS_Printf("safe read fail, size: %d status: %d\n", size, request->status);
        /* request processing failed */
        close(sock);
        return -1;
    }

	//SYS_Printf("[IPC]commond excuted, ret:%d\n", request->status);

    close(sock);
    return request->status;
}

int request_for_netinfd(struct netinfd_request *request)
{
    int sock, size;

  DEBUGN_PRINT("claim req type: %d", request->type);

    size = pack_request_netinf(request);
    if (size < 0)
  {
    SYS_Printf("pack req fail\n");
        return -1;
    }
  DEBUGN_PRINT("pack req ok");

    sock = connect_request_netinf();
    if (sock < 0)
  {
    SYS_Printf("connect fail\n");
        return -1;
    }
  DEBUGN_PRINT("connect ok %d", sock);


    if (safe_write(sock, request, sizeof(struct netinfd_request)) != sizeof(struct netinfd_request))
  {
    SYS_Printf("safe write fail\n");
        close(sock);
        return -1;
    }
  DEBUGN_PRINT("client req safe write ok");

    size = safe_read(sock, request, sizeof(struct netinfd_request));

    if (size < 0 ||
        unpack_request_netinf(request, size) < 0 ||
          request->status < 0)
  {
        SYS_Printf("safe read fail, size: %d status: %d\n", size, request->status);
        /* request processing failed */
        close(sock);
        return -1;
    }
  DEBUGN_PRINT("client req safe read ok: %d", size);

    close(sock);
    return 0;
}
