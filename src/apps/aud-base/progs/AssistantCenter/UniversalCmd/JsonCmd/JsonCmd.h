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


#ifndef __JSONCMD_H__
#define __JSONCMD_H__

#include "UniversalCmd.h"
#include <iostream>
#include <map>
extern "C"
{
#include "AssistantDef.h"
#include "cjson/cJSON.h"
#include "AssistantCmd.h"
}

using std::map;
using std::string;

class JsonCmd : public UniversalCmd {
public:
    JsonCmd() {}
    virtual ~JsonCmd() {}

    int32 Init(void);
    void  DeInit(void);
    shared_ptr<char> ConstructCmd(ASSISTANT_CMD_E cmd, char *data, size_t dataSize);
    shared_ptr<char> DeconstructCmd(char *data, ASSISTANT_CMD_E &cmd);

private:
    //convert struct to json
    typedef shared_ptr<char> (JsonCmd::*ConstructFunc)(char *data, size_t dataSize);
    //convert json to struct
    typedef shared_ptr<char> (JsonCmd::*DeconstructFunc)(char *data, ASSISTANT_CMD_E &cmd);

    template <typename T>
    shared_ptr<T> ConvertPointerToShared(T *point, size_t size);

    map<ASSISTANT_CMD_E, ConstructFunc> mConstructFuncMap;
    map<string, DeconstructFunc> mDeconstructFuncMap;
    shared_ptr<char> ConstructPlayCmd(char *data, size_t dataSize);
    shared_ptr<char> ConstructPlayVoicePromptCmd(char *data, size_t dataSize);
    shared_ptr<char> ConstructPlayTtsCmd(char *data, size_t dataSize);
    shared_ptr<char> ConstructPlayPrevAudioCmd(char *data, size_t dataSize);
    shared_ptr<char> ConstructPlayNextAudioCmd(char *data, size_t dataSize);
    shared_ptr<char> ConstructSetVolumeCmd(char *data, size_t dataSize);
    shared_ptr<char> ConstructSetSystemStatusCmd(char *data, size_t dataSize);
    shared_ptr<char> ConstructSetBtNameCmd(char *data, size_t dataSize);
    shared_ptr<char> ConstructStartBtPairCmd(char *data, size_t dataSize);
    shared_ptr<char> ConstructBtDelPairedCmd(char *data, size_t dataSize);
    shared_ptr<char> ConstructBtPowerOffCmd(char *data, size_t dataSize);
    shared_ptr<char> ConstructPlayBtMusicCmd(char *data, size_t dataSize);
    shared_ptr<char> ConstructBtDisconnectCmd(char *data, size_t dataSize);
    shared_ptr<char> ConstructGetApListCmd(char *data, size_t dataSize);
    shared_ptr<char> ConstructWifiConnectCmd(char *data, size_t dataSize);
    shared_ptr<char> ConstructWifiConnectOverCmd(char *data, size_t dataSize);
    shared_ptr<char> ConstructWifiSetupResultCmd(char *data, size_t dataSize);
    shared_ptr<char> ConstructSpeechStartCmd(char *data, size_t dataSize);
    shared_ptr<char> ConstructSpeechProcessCmd(char *data, size_t dataSize);
    shared_ptr<char> ConstructSpeechFeedbackCmd(char *data, size_t dataSize);
    shared_ptr<char> ConstructSpeechFinishCmd(char *data, size_t dataSize);
    shared_ptr<char> ConstructGetSpeakerStatusCmd(char *data, size_t dataSize);
    shared_ptr<char> ConstructPauseCmd(char *data, size_t dataSize);
    shared_ptr<char> ConstructResumeCmd(char *data, size_t dataSize);
    shared_ptr<char> ConstructOtaUpgradeCmd(char *data, size_t dataSize);
    shared_ptr<char> ConstructAdjustProgressCmd(char *data, size_t dataSize);
    shared_ptr<char> ConstructButtonCmd(char *data, size_t dataSize);
    shared_ptr<char> ConstructStopCmd(char *data, size_t dataSize);
    shared_ptr<char> ConstructFactoryResetResultCmd(char *data, size_t dataSize);
    shared_ptr<char> ConstructHfpFreeMicResultCmd(char *data, size_t dataSize);

    shared_ptr<char> DeconstructPlayDoneResponse(char *data, ASSISTANT_CMD_E &cmd);
    shared_ptr<char> DeconstructPlayTtsDoneResponse(char *data, ASSISTANT_CMD_E &cmd);
    shared_ptr<char> DeconstructSpeakerStatusResponse(char *data, ASSISTANT_CMD_E &cmd);
    shared_ptr<char> DeconstructSystemStatusChangeResponse(char *data, ASSISTANT_CMD_E &cmd);
    shared_ptr<char> DeconstructPlayerStatusChangeResponse(char *data, ASSISTANT_CMD_E &cmd);
    shared_ptr<char> DeconstructNetworkStatusChangeResponse(char *data, ASSISTANT_CMD_E &cmd);
    shared_ptr<char> DeconstructBluetoothStatusChangeResponse(char *data, ASSISTANT_CMD_E &cmd);
    shared_ptr<char> DeconstructBluetoothSrcAvrcpCmd(char *data, ASSISTANT_CMD_E &cmd);
    shared_ptr<char> DeconstructWifiStatusResponse(char *data, ASSISTANT_CMD_E &cmd);
    shared_ptr<char> DeconstructGetApListResponse(char *data, ASSISTANT_CMD_E &cmd);
    shared_ptr<char> DeconstructHfpStatusChangeResponse(char *data, ASSISTANT_CMD_E &cmd);
	shared_ptr<char> DeconstructDeconstructRecevButton(char *data, ASSISTANT_CMD_E &cmd);
};

#endif
