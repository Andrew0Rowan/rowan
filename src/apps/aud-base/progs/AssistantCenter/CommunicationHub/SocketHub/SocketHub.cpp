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


#include "SocketHub.h"
extern "C"
{
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
}

int32 SocketHub::ServerInit(void) {
    memset(&mServerAddr, 0, sizeof(mServerAddr));
    mServerAddr.sun_family = AF_UNIX;
    strcpy(mServerAddr.sun_path, SERVER_HUB_PATH);
    mServerPathName = mServerAddr.sun_path;
    unlink(mServerPathName.c_str());

    mServerSockfd = socket(AF_UNIX, SOCK_DGRAM, 0);
    if (-1 == mServerSockfd) {
        perror("Fail to create server socket.\n");
        perror(strerror(errno));
        return -1;
    }

    /*bind server socket*/
    int32 ret = bind(mServerSockfd, (struct sockaddr*)&mServerAddr, sizeof(mServerAddr));
    if (ret)
    {
        perror("Fail to start hub server.\n");
        perror(strerror(errno));
        return ret;
    }
    AST_INFO("Server Hub Init Success!\n");
    return 0;
}

int32 SocketHub::ClientInit(void) {
    mClientAddr.sun_family = AF_UNIX;
    strcpy(mClientAddr.sun_path, CLIENT_HUB_PATH);

    /*create client socket*/
    mClientSockfd = socket(AF_UNIX, SOCK_DGRAM, 0);
    if (-1 == mClientSockfd)
    {
        perror("Fail to create client socket.\n");
        perror(strerror(errno));
        return -1;
    }
    AST_INFO("Client Hub Init Success!\n");
    return 0;
}

int32 SocketHub::Init(void) {
    if (ServerInit() || ClientInit()) {
        return -1;
    }
    return 0;
}

ssize_t SocketHub::RecvMsg(void *buf, size_t maxLen) {
    return recv(mServerSockfd, buf, maxLen, 0);
}

ssize_t SocketHub::SendMsg(const void *buf, size_t bufLen) {
    return sendto(mClientSockfd,
                  buf,
                  bufLen,
                  0,
                  (struct sockaddr *)&mClientAddr,
                  sizeof(mClientAddr));
}

void SocketHub::DeInit(void) {
    shutdown(mServerSockfd, SHUT_RD);
    close(mClientSockfd);
    close(mServerSockfd);
    unlink(mServerPathName.c_str());
}


