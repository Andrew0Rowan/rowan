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



#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <sys/types.h>

#include <unistd.h>
#include <netdb.h>
#include <errno.h>
#include <string.h>

#include "u_dbg.h"

int i4_NDM;
#define dbg_mw_printf printf
struct hostent *c_net_dbg_gethostbyname(const char *name)
{

    struct hostent *pt_hostnet;
    struct in_addr ip_addr = {0};

    pt_hostnet = gethostbyname(name);

    if (i4_NDM)
    {
        if (pt_hostnet)
        {
            ip_addr = *(struct in_addr*)(pt_hostnet->h_addr_list[0]);
            dbg_mw_printf("<NDM> gethostbyname (thread id = %d) hostname = %s, address = %s \n",
                (int)getpid(), name, inet_ntoa(ip_addr));
        }
        else
        {
            dbg_mw_printf("<NDM> gethostbyname (thread id = %d) fail. hostname = %s, err: %s %d\n",
                (int)getpid(), name, hstrerror(h_errno), h_errno);
        }
    }

    return pt_hostnet;
}


struct hostent *c_net_dbg_gethostbyaddr(const char *addr, int len, int type)
{
    struct hostent *pt_hostnet;

    pt_hostnet = gethostbyaddr(addr, len, type);

    if (i4_NDM)
    {

        if (pt_hostnet)
        {
            dbg_mw_printf("<NDM> gethostbyaddr (thread id = %d) hostname = %s, address = %s \n",
                (int)getpid(), pt_hostnet->h_name, inet_ntoa(*(struct in_addr *)addr));

        }
        else
        {
            dbg_mw_printf("<NDM> gethostbyaddr (thread id = %d) address = %s, err: %s %d\n",
                (int)getpid(), inet_ntoa(*(struct in_addr *)addr), hstrerror(h_errno), h_errno);
        }
    }

    return pt_hostnet;

}


int c_net_dbg_connect(int sockfd, const struct sockaddr *serv_addr, socklen_t addr_len)
{
    int i4_ConnRet;

    if (i4_NDM)
    {
        dbg_mw_printf("<NDM> connect (thread id = %d) sockfd = %d, dest_addr = %s, dest_port = %d  \n", (int)getpid(), sockfd,
                          inet_ntoa(((struct sockaddr_in *)serv_addr)->sin_addr), ((struct sockaddr_in *)serv_addr)->sin_port);
    }

    i4_ConnRet = connect(sockfd, serv_addr, addr_len);

    if (i4_NDM)
    {
        if (i4_ConnRet)
        {
            dbg_mw_printf("<NDM> connect (thread id = %d) err: %s %d\n", (int)getpid(), strerror(errno), errno);
        }
    }
    return i4_ConnRet;
}


int c_net_dbg_bind(int sockfd, struct sockaddr *my_addr, socklen_t addrlen)
{
    int i4_ret;

    if (i4_NDM)
    {
        dbg_mw_printf("<NDM> bind (thread id = %d) sockfd = %d, serv_address = %s, serv_port = %s \n", (int)getpid(), sockfd,
                          inet_ntoa(((struct sockaddr_in *)my_addr)->sin_addr), ((struct sockaddr_in *)my_addr)->sin_port);
    }

    i4_ret = bind(sockfd, my_addr, addrlen);

    if (i4_NDM)
    {
        if (i4_ret)
        {
            dbg_mw_printf("<NDM> connect (thread id = %d) err: %s %d\n", (int)getpid(), strerror(errno), errno);
        }
    }

    return i4_ret;
}


ssize_t c_net_dbg_send(int sockfd, const void *buf, size_t len, int flags)
{
    ssize_t size;

    size = send(sockfd, buf, len, flags);

    if(i4_NDM)
    {
        dbg_mw_printf("<NDM> send (thread id = %d) Out: sockfd = %d, buffer = 0x%x, nbytes = %d, flags = %d, sned = %d \n",
            (int)getpid(), sockfd, buf, len, flags, size);
        if ( -1 == size )
        {
            dbg_mw_printf("<NDM> send (thread id = %d) err: %s %d\n", (int)getpid(), strerror(errno), errno);
        }
    }

    return size;
}


ssize_t c_net_dbg_sendto(int sockfd, const void *buf, size_t len,
               int flags, const struct sockaddr *dest_addr, socklen_t dest_len)
{
    ssize_t size;

    size = sendto(sockfd, buf, len, flags, dest_addr, dest_len);

    if (i4_NDM)
    {
        dbg_mw_printf("<NDM> sendto (thread id = %d) Out: sockfd = %d, buffer = 0x%x, nbytes = %d, flags = %d, sned = %d, to_addr = %s, to_port = %d \n",
            (int)getpid(), sockfd, buf, len, flags, size, inet_ntoa(((struct sockaddr_in *)dest_addr)->sin_addr), ((struct sockaddr_in *)dest_addr)->sin_port);

        if ( -1 == size )
        {
            dbg_mw_printf("<NDM> sendto (thread id = %d) err: %s %d\n", (int)getpid(), strerror(errno), errno);
        }
    }

    return size;
}


ssize_t c_net_dbg_recv(int sockfd, void *buf, size_t len, int flags)
{
    ssize_t size;

    size = recv(sockfd, buf, len, flags);

    if(i4_NDM){
        dbg_mw_printf("<NDM> recv (thread id= %d) In: sockfd = %d, buffer = 0x%x, nbytes = %d, flags = %d, recv = %d \n",
            (int)getpid(), sockfd, buf, len, flags, size);

        if ( -1 == size )
        {
            dbg_mw_printf("<NDM> recv (thread id = %d) err: %s %d\n", (int)getpid(), strerror(errno), errno);
        }
    }

    return size;
}


ssize_t c_net_dbg_recvfrom(int sockfd, void *buf, size_t len,
              int flags, struct sockaddr *address, socklen_t *address_len)
{
    ssize_t size;

    size = recvfrom(sockfd, buf, len, flags, address, address_len);

    if (i4_NDM)
    {
        dbg_mw_printf("<NDM> recvfrom (thread id= %d) In: sockfd = %d, buffer = 0x%x, nbytes = %d, flags = %d, recv = %d, from_addr = %s, from_port = %d \n",
            (int)getpid(), sockfd, buf, len, flags, size, inet_ntoa(((struct sockaddr_in *)address)->sin_addr), ((struct sockaddr_in *)address)->sin_port);

        if ( -1 == size )
        {
            dbg_mw_printf("<NDM> recvfrom (thread id = %d) err: %s %d\n", (int)getpid(), strerror(errno), errno);
        }
    }

    return 0;
}



