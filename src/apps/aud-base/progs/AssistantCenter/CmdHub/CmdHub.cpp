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


#include "CmdHub.h"
extern "C"
{
#include <string.h>
#include <pthread.h>
}

void *RecvCmdThread(void *arg) {
    ((CmdHub *)arg)->RecvCmd();
    return NULL;
}

int32 CmdHub::Init(CmdNotifyFunc func, HubType hubType, CmdType cmdType) {
    int32 ret;

    if (nullptr == func) {
        AST_ERR("CmdNotifyFunc is nullptr!\n");
        return AST_FAIL;
    }
    mNotifyCallback = func;

    mUniversalCmd = UniversalCmdFactory::GetUniversalCmd(cmdType);
    if (nullptr == mUniversalCmd) {
        AST_ERR("UniversalCmd is nullptr!\n");
        return AST_FAIL;
    }
    mCommunicationHub = CommunicationHubFactory::GetCommunicationHub(hubType);
    if (nullptr == mCommunicationHub) {
        AST_ERR("CommunicationHub is nullptr!\n");
        return AST_FAIL;
    }

    ret = mUniversalCmd->Init();
    if (ret) {
        AST_ERR("UniversalCmd init failed[%d]!\n", ret);
        goto CMD_INIT_ERR;
    }
    ret = mCommunicationHub->Init();
    if (ret) {
        AST_ERR("UniversalCmd init failed[%d]!\n", ret);
        goto HUB_INIT_ERR;
    }

    mRecvThreadExit = false;
    ret = pthread_create(&mRecvThread, NULL, RecvCmdThread, this);
    if (ret) {
        AST_ERR("CmdReceive Thread Create Failed[%d]!\n", ret);
        goto PTHREAD_ERR;
    }

    return AST_OK;

PTHREAD_ERR:
    mCommunicationHub->DeInit();
CMD_INIT_ERR:
    mUniversalCmd->DeInit();
HUB_INIT_ERR:
    return AST_FAIL;
}

void CmdHub::DeInit(void) {
    mCommunicationHub->DeInit();
    mUniversalCmd->DeInit();
    mRecvThreadExit = true;
    pthread_join(mRecvThread, NULL);
}

int32 CmdHub::SendCmd(ASSISTANT_CMD_E cmd, char *data, size_t dataSize) {
    shared_ptr<char> jsonCmd = mUniversalCmd->ConstructCmd(cmd, data, dataSize);
    if (nullptr == jsonCmd) {
        AST_ERR("UniversalCmd ConstructCmd failed!\n");
        return AST_FAIL;
    }

    return mCommunicationHub->SendMsg(jsonCmd.get(), strlen(jsonCmd.get()));
}

void CmdHub::RecvCmd(void) {
    ssize_t ret;
    ASSISTANT_CMD_E cmd;
    shared_ptr<char> cmdBuf;

    while (false == mRecvThreadExit) {
        ret = mCommunicationHub->RecvMsg(mRecvBuf, HUB_CMD_LENGTH_MAX);
        if (ret > 0) {
            cmdBuf = mUniversalCmd->DeconstructCmd(mRecvBuf, cmd);
            if (mNotifyCallback) {
                mNotifyCallback(cmd, cmdBuf.get(),this->pri_data);
            }
        }
    }
}


