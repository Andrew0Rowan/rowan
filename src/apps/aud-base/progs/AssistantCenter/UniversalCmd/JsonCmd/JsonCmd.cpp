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


#include "JsonCmd.h"

extern "C"
{
#include <stdio.h>
#include <string.h>
}

using std::make_pair;

int32 JsonCmd::Init(void) {
    mConstructFuncMap.insert(make_pair(ASSISTANT_CMD_PLAY, &JsonCmd::ConstructPlayCmd));
    mConstructFuncMap.insert(make_pair(ASSISTANT_CMD_PLAY_VOICE_PROMPT, &JsonCmd::ConstructPlayVoicePromptCmd));
    mConstructFuncMap.insert(make_pair(ASSISTANT_CMD_PLAY_TTS, &JsonCmd::ConstructPlayTtsCmd));
    mConstructFuncMap.insert(make_pair(ASSISTANT_CMD_PLAY_PREV_AUDIO, &JsonCmd::ConstructPlayPrevAudioCmd));
    mConstructFuncMap.insert(make_pair(ASSISTANT_CMD_PLAY_NEXT_AUDIO, &JsonCmd::ConstructPlayNextAudioCmd));
    mConstructFuncMap.insert(make_pair(ASSISTANT_CMD_SET_VOLUME, &JsonCmd::ConstructSetVolumeCmd));
    mConstructFuncMap.insert(make_pair(ASSISTANT_CMD_SET_SYSTEM_STATUS, &JsonCmd::ConstructSetSystemStatusCmd));
    mConstructFuncMap.insert(make_pair(ASSISTANT_CMD_SET_BT_NAME, &JsonCmd::ConstructSetBtNameCmd));
    mConstructFuncMap.insert(make_pair(ASSISTANT_CMD_START_BT_PAIR, &JsonCmd::ConstructStartBtPairCmd));
	mConstructFuncMap.insert(make_pair(ASSISTANT_CMD_DEL_BT_PAIRED, &JsonCmd::ConstructBtDelPairedCmd));
	mConstructFuncMap.insert(make_pair(ASSISTANT_CMD_BT_POWER_OFF, &JsonCmd::ConstructBtPowerOffCmd));
    mConstructFuncMap.insert(make_pair(ASSISTANT_CMD_PLAY_BT_MUSIC, &JsonCmd::ConstructPlayBtMusicCmd));
    mConstructFuncMap.insert(make_pair(ASSISTANT_CMD_BT_DISCONNECT, &JsonCmd::ConstructBtDisconnectCmd));
    mConstructFuncMap.insert(make_pair(ASSISTANT_CMD_GET_AP_LIST, &JsonCmd::ConstructGetApListCmd));
    mConstructFuncMap.insert(make_pair(ASSISTANT_CMD_WIFI_CONNECT, &JsonCmd::ConstructWifiConnectCmd));
    mConstructFuncMap.insert(make_pair(ASSISTANT_CMD_WIFI_CONNECT_OVER, &JsonCmd::ConstructWifiConnectOverCmd));
    mConstructFuncMap.insert(make_pair(ASSISTANT_CMD_WIFI_SETUP_RESULT, &JsonCmd::ConstructWifiSetupResultCmd));
    mConstructFuncMap.insert(make_pair(ASSISTANT_CMD_SPEECH_START, &JsonCmd::ConstructSpeechStartCmd));
    mConstructFuncMap.insert(make_pair(ASSISTANT_CMD_SPEECH_PROCESS, &JsonCmd::ConstructSpeechProcessCmd));
    mConstructFuncMap.insert(make_pair(ASSISTANT_CMD_SPEECH_FEEDBACK, &JsonCmd::ConstructSpeechFeedbackCmd));
    mConstructFuncMap.insert(make_pair(ASSISTANT_CMD_SPEECH_FINISH, &JsonCmd::ConstructSpeechFinishCmd));
    mConstructFuncMap.insert(make_pair(ASSISTANT_CMD_GET_SPEAKER_STATUS, &JsonCmd::ConstructGetSpeakerStatusCmd));
    mConstructFuncMap.insert(make_pair(ASSISTANT_CMD_PAUSE, &JsonCmd::ConstructPauseCmd));
    mConstructFuncMap.insert(make_pair(ASSISTANT_CMD_RESUME, &JsonCmd::ConstructResumeCmd));
    mConstructFuncMap.insert(make_pair(ASSISTANT_CMD_OTA_UPGRADE, &JsonCmd::ConstructOtaUpgradeCmd));
    mConstructFuncMap.insert(make_pair(ASSISTANT_CMD_ADJUST_PROGRESS, &JsonCmd::ConstructAdjustProgressCmd));
    mConstructFuncMap.insert(make_pair(ASSISTANT_CMD_BUTTON, &JsonCmd::ConstructButtonCmd));
    mConstructFuncMap.insert(make_pair(ASSISTANT_CMD_STOP, &JsonCmd::ConstructStopCmd));
    mConstructFuncMap.insert(make_pair(ASSISTANT_CMD_FACTORY_RESET_RESULT, &JsonCmd::ConstructFactoryResetResultCmd));
	mConstructFuncMap.insert(make_pair(ASSISTANT_CMD_HFP_FREE_MIC_RESULT, &JsonCmd::ConstructHfpFreeMicResultCmd));

    mDeconstructFuncMap.insert(make_pair(STRING_PLAY_DONE, &JsonCmd::DeconstructPlayDoneResponse));
    mDeconstructFuncMap.insert(make_pair(STRING_PLAY_TTS_DONE, &JsonCmd::DeconstructPlayTtsDoneResponse));
    mDeconstructFuncMap.insert(make_pair(STRING_GET_SPEAKER_STATUS, &JsonCmd::DeconstructSpeakerStatusResponse));
    mDeconstructFuncMap.insert(make_pair(STRING_SYSTEM_STATUS_CHANGE, &JsonCmd::DeconstructSystemStatusChangeResponse));
    mDeconstructFuncMap.insert(make_pair(STRING_PLAYER_STATUS_CHANGE, &JsonCmd::DeconstructPlayerStatusChangeResponse));
    mDeconstructFuncMap.insert(make_pair(STRING_NETWORK_STATUS_CHANGE, &JsonCmd::DeconstructNetworkStatusChangeResponse));
    mDeconstructFuncMap.insert(make_pair(STRING_BLUETOOTH_STATUS_CHANGE, &JsonCmd::DeconstructBluetoothStatusChangeResponse));
	mDeconstructFuncMap.insert(make_pair(STRING_BT_SRC_AVRCP_CMD, &JsonCmd::DeconstructBluetoothSrcAvrcpCmd));
    mDeconstructFuncMap.insert(make_pair(STRING_GET_AP_LIST, &JsonCmd::DeconstructGetApListResponse));
    mDeconstructFuncMap.insert(make_pair(STRING_WIFI_CONNECT, &JsonCmd::DeconstructWifiStatusResponse));
	mDeconstructFuncMap.insert(make_pair(STRING_HFP_STATUS_CHANGE, &JsonCmd::DeconstructHfpStatusChangeResponse));
	mDeconstructFuncMap.insert(make_pair(STRING_BUTTON, &JsonCmd::DeconstructDeconstructRecevButton));
    AST_INFO("JsonCmd Init Success!\n");
    return AST_OK;
}

void  JsonCmd::DeInit(void) {

}

shared_ptr<char> JsonCmd::ConstructCmd(ASSISTANT_CMD_E cmd, char *data, size_t dataSize) {
    ConstructFunc funcPoint = mConstructFuncMap[cmd];
    if (funcPoint) {
        return (this->*funcPoint)(data, dataSize);
    }
    return nullptr;
}

shared_ptr<char> JsonCmd::DeconstructCmd(char *data, ASSISTANT_CMD_E &cmd) {
    cJSON *root = cJSON_Parse((const char *)data);
    if (NULL == root) {
        return nullptr;
    }

    cJSON *command = cJSON_GetObjectItem(root, STRING_COMMAND);
    if (NULL == command) {
        cJSON_Delete(root);
        return nullptr;
    }

    string commandString = command->valuestring;

    cJSON_Delete(root);

    DeconstructFunc funcPoint = mDeconstructFuncMap[commandString];
    if (funcPoint) {
        return (this->*funcPoint)(data, cmd);
    }
    return nullptr;
}

template <typename T>
shared_ptr<T> JsonCmd::ConvertPointerToShared(T *point, size_t size) {
    shared_ptr<T> sp (new char[size], [](char *p){delete []p;});
    memcpy(sp.get(), point, size);
    return sp;
}

shared_ptr<char> JsonCmd::ConstructPlayCmd(char *data, size_t dataSize) {
    ASSISTANT_CMD_PLAY_T *tmp = (ASSISTANT_CMD_PLAY_T *)data;

    if (dataSize != sizeof(*tmp)) {
        AST_ERR("error dataSize:[%lu], correct:[%lu]\n", dataSize, sizeof(*tmp));
        return NULL;
    }

    cJSON *root = cJSON_CreateObject();
    if (NULL == root) {
        return NULL;
    }

    cJSON_AddStringToObject(root, STRING_COMMAND, STRING_PLAY);

    cJSON *params = cJSON_CreateObject();
    if (NULL == params) {
        cJSON_Delete(root);
        return NULL;
    }

    cJSON_AddItemToObject(root, STRING_PARAMS, params);
    cJSON_AddStringToObject(params, STRING_URI, tmp->uri);
    cJSON_AddStringToObject(params, STRING_AUDIO_ID, tmp->audioId);
    cJSON_AddStringToObject(params, STRING_AUDIO_UID, tmp->audioUid);
    cJSON_AddStringToObject(params, STRING_AUDIO_SOURCE, tmp->audioSource);
    cJSON_AddStringToObject(params, STRING_AUDIO_NAME, tmp->audioName);
    cJSON_AddStringToObject(params, STRING_AUDIO_ANCHOR, tmp->audioAnchor);
    cJSON_AddStringToObject(params, STRING_AUDIO_ALBUM, tmp->audioAlbum);
    cJSON_AddNumberToObject(params, STRING_PROGRESS, tmp->progress);
    cJSON_AddStringToObject(params, STRING_AUDIO_EXT, tmp->audioExt);

    char *str = cJSON_Print(root);
    printf("%s", str);

    shared_ptr<char> point = ConvertPointerToShared<char>(str, strlen(str) + 1);

    free(str);
    cJSON_Delete(root);

    return point;
}

shared_ptr<char> JsonCmd::ConstructPlayVoicePromptCmd(char *data, size_t dataSize) {
    ASSISTANT_CMD_PLAY_VOICE_PROMPT_T *tmp = (ASSISTANT_CMD_PLAY_VOICE_PROMPT_T *)data;

    if (dataSize != sizeof(*tmp)) {
        AST_ERR("error dataSize:[%lu], correct:[%lu]\n", dataSize, sizeof(*tmp));
        return NULL;
    }

    cJSON *root = cJSON_CreateObject();
    if (NULL == root) {
        return NULL;
    }

    cJSON_AddStringToObject(root, STRING_COMMAND, STRING_PLAY_VOICE_PROMPT);

    cJSON *params = cJSON_CreateObject();
    if (NULL == params) {
        cJSON_Delete(root);
        return NULL;
    }

    cJSON_AddItemToObject(root, STRING_PARAMS, params);
    cJSON_AddStringToObject(params, STRING_URI, tmp->uri);
    cJSON_AddNumberToObject(params, STRING_VOLUME, tmp->volume);
    cJSON_AddStringToObject(params, STRING_TYPE, tmp->type);
    cJSON_AddBoolToObject(params, STRING_FEEDBACK, tmp->feedback);

    char *str = cJSON_Print(root);
    printf("%s", str);

    shared_ptr<char> point = ConvertPointerToShared<char>(str, strlen(str) + 1);

    free(str);
    cJSON_Delete(root);

    return point;
}

shared_ptr<char> JsonCmd::ConstructPlayTtsCmd(char *data, size_t dataSize) {
    ASSISTANT_CMD_PLAY_TTS_T *tmp = (ASSISTANT_CMD_PLAY_TTS_T *)data;

    if (dataSize != sizeof(*tmp)) {
        AST_ERR("error dataSize:[%lu], correct:[%lu]\n", dataSize, sizeof(*tmp));
        return NULL;
    }

    cJSON *root = cJSON_CreateObject();
    if (NULL == root) {
        return NULL;
    }

    cJSON_AddStringToObject(root, STRING_COMMAND, STRING_PLAY_TTS);

    cJSON *params = cJSON_CreateObject();
    if (NULL == params) {
        cJSON_Delete(root);
        return NULL;
    }

    cJSON_AddItemToObject(root, STRING_PARAMS, params);
    cJSON_AddNumberToObject(params, STRING_TTS_ID, tmp->tts_id);
    cJSON_AddBoolToObject(params, STRING_SPEECH_ENABLE, tmp->speech_enable);
    cJSON_AddBoolToObject(params, STRING_NEED_MIX, tmp->need_mix);
    cJSON_AddBoolToObject(params, STRING_PCM, tmp->pcm);
    cJSON_AddNumberToObject(params, STRING_SAMPLERATE, tmp->mSampleRate);	
    cJSON_AddNumberToObject(params, STRING_CHANNEL, tmp->mChannels);
    AST_ERR("Json PCMData:[%d]\n", tmp->pcm);

    char *str = cJSON_Print(root);
    printf("%s", str);

    shared_ptr<char> point = ConvertPointerToShared<char>(str, strlen(str) + 1);

    free(str);
    cJSON_Delete(root);

    return point;
}

shared_ptr<char> JsonCmd::ConstructPauseCmd(char *data, size_t dataSize) {
    ASSISTANT_CMD_PAUSE_T *tmp = (ASSISTANT_CMD_PAUSE_T *)data;

    if (dataSize != sizeof(*tmp)) {
        AST_ERR("error dataSize:[%lu], correct:[%lu]\n", dataSize, sizeof(*tmp));
        return NULL;
    }

    cJSON *root = cJSON_CreateObject();
    if (NULL == root) {
        return NULL;
    }

    cJSON_AddStringToObject(root, STRING_COMMAND, STRING_PAUSE);

    char *str = cJSON_Print(root);
    printf("%s", str);

    shared_ptr<char> point = ConvertPointerToShared<char>(str, strlen(str) + 1);

    free(str);
    cJSON_Delete(root);

    return point;
}

shared_ptr<char> JsonCmd::ConstructResumeCmd(char *data, size_t dataSize) {
    ASSISTANT_CMD_RESUME_T *tmp = (ASSISTANT_CMD_RESUME_T *)data;

    if (dataSize != sizeof(*tmp)) {
        AST_ERR("error dataSize:[%lu], correct:[%lu]\n", dataSize, sizeof(*tmp));
        return NULL;
    }

    cJSON *root = cJSON_CreateObject();
    if (NULL == root) {
        return NULL;
    }

    cJSON_AddStringToObject(root, STRING_COMMAND, STRING_RESUME);

    char *str = cJSON_Print(root);
    printf("%s", str);

    shared_ptr<char> point = ConvertPointerToShared<char>(str, strlen(str) + 1);

    free(str);
    cJSON_Delete(root);

    return point;
}

shared_ptr<char> JsonCmd::ConstructStopCmd(char *data, size_t dataSize) {
    ASSISTANT_CMD_STOP_T *tmp = (ASSISTANT_CMD_STOP_T *)data;

    if (dataSize != sizeof(*tmp)) {
        AST_ERR("error dataSize:[%lu], correct:[%lu]\n", dataSize, sizeof(*tmp));
        return NULL;
    }

    cJSON *root = cJSON_CreateObject();
    if (NULL == root) {
        return NULL;
    }

    cJSON_AddStringToObject(root, STRING_COMMAND, STRING_STOP);

    char *str = cJSON_Print(root);
    printf("%s", str);

    shared_ptr<char> point = ConvertPointerToShared<char>(str, strlen(str) + 1);

    free(str);
    cJSON_Delete(root);

    return point;
}

shared_ptr<char> JsonCmd::ConstructPlayPrevAudioCmd(char *data, size_t dataSize) {
    ASSISTANT_CMD_PLAY_PREV_AUDIO_T *tmp = (ASSISTANT_CMD_PLAY_PREV_AUDIO_T *)data;

    if (dataSize != sizeof(*tmp)) {
        AST_ERR("error dataSize:[%lu], correct:[%lu]\n", dataSize, sizeof(*tmp));
        return NULL;
    }

    cJSON *root = cJSON_CreateObject();
    if (NULL == root) {
        return NULL;
    }

    cJSON_AddStringToObject(root, STRING_COMMAND, STRING_PLAY_PREV_AUDIO);

    char *str = cJSON_Print(root);
    printf("%s", str);

    shared_ptr<char> point = ConvertPointerToShared<char>(str, strlen(str) + 1);

    free(str);
    cJSON_Delete(root);

    return point;
}

shared_ptr<char> JsonCmd::ConstructPlayNextAudioCmd(char *data, size_t dataSize) {
    ASSISTANT_CMD_PLAY_NEXT_AUDIO_T *tmp = (ASSISTANT_CMD_PLAY_NEXT_AUDIO_T *)data;

    if (dataSize != sizeof(*tmp)) {
        AST_ERR("error dataSize:[%lu], correct:[%lu]\n", dataSize, sizeof(*tmp));
        return NULL;
    }

    cJSON *root = cJSON_CreateObject();
    if (NULL == root) {
        return NULL;
    }

    cJSON_AddStringToObject(root, STRING_COMMAND, STRING_PLAY_NEXT_AUDIO);

    char *str = cJSON_Print(root);
    printf("%s", str);

    shared_ptr<char> point = ConvertPointerToShared<char>(str, strlen(str) + 1);

    free(str);
    cJSON_Delete(root);

    return point;
}

shared_ptr<char> JsonCmd::ConstructSetVolumeCmd(char *data, size_t dataSize) {
    ASSISTANT_CMD_SET_VOLUME_T *tmp = (ASSISTANT_CMD_SET_VOLUME_T *)data;

    if (dataSize != sizeof(*tmp)) {
        AST_ERR("error dataSize:[%lu], correct:[%lu]\n", dataSize, sizeof(*tmp));
        return NULL;
    }

    cJSON *root = cJSON_CreateObject();
    if (NULL == root) {
        return NULL;
    }

    cJSON_AddStringToObject(root, STRING_COMMAND, STRING_SET_VOLUME);

    cJSON *params = cJSON_CreateObject();
    if (NULL == params) {
        cJSON_Delete(root);
        return NULL;
    }

    cJSON_AddItemToObject(root, STRING_PARAMS, params);
    cJSON_AddNumberToObject(params, STRING_VOLUME, tmp->volume);

    char *str = cJSON_Print(root);
    printf("%s", str);

    shared_ptr<char> point = ConvertPointerToShared<char>(str, strlen(str) + 1);

    free(str);
    cJSON_Delete(root);

    return point;
}

shared_ptr<char> JsonCmd::ConstructSetSystemStatusCmd(char *data, size_t dataSize) {
    ASSISTANT_CMD_SET_SYSTEM_STATUS_T *tmp = (ASSISTANT_CMD_SET_SYSTEM_STATUS_T *)data;

    if (dataSize != sizeof(*tmp)) {
        AST_ERR("error dataSize:[%lu], correct:[%lu]\n", dataSize, sizeof(*tmp));
        return NULL;
    }

    cJSON *root = cJSON_CreateObject();
    if (NULL == root) {
        return NULL;
    }

    cJSON_AddStringToObject(root, STRING_COMMAND, STRING_SET_SYSTEM_STATUS);

    cJSON *params = cJSON_CreateObject();
    if (NULL == params) {
        cJSON_Delete(root);
        return NULL;
    }

    cJSON_AddItemToObject(root, STRING_PARAMS, params);
    cJSON_AddStringToObject(params, STRING_STATUS, tmp->status);

    char *str = cJSON_Print(root);
    printf("%s", str);

    shared_ptr<char> point = ConvertPointerToShared<char>(str, strlen(str) + 1);

    free(str);
    cJSON_Delete(root);

    return point;
}

shared_ptr<char> JsonCmd::ConstructSetBtNameCmd(char *data, size_t dataSize) {
    ASSISTANT_CMD_SET_BT_NAME_T *tmp = (ASSISTANT_CMD_SET_BT_NAME_T *)data;

    if (dataSize != sizeof(*tmp)) {
        AST_ERR("error dataSize:[%lu], correct:[%lu]\n", dataSize, sizeof(*tmp));
        return NULL;
    }

    cJSON *root = cJSON_CreateObject();
    if (NULL == root) {
        return NULL;
    }

    cJSON_AddStringToObject(root, STRING_COMMAND, STRING_SET_BT_NAME);

    cJSON *params = cJSON_CreateObject();
    if (NULL == params) {
        cJSON_Delete(root);
        return NULL;
    }

    cJSON_AddItemToObject(root, STRING_PARAMS, params);
    cJSON_AddStringToObject(params, STRING_NAME, tmp->name);

    char *str = cJSON_Print(root);
    printf("%s", str);

    shared_ptr<char> point = ConvertPointerToShared<char>(str, strlen(str) + 1);

    free(str);
    cJSON_Delete(root);

    return point;
}

shared_ptr<char> JsonCmd::ConstructStartBtPairCmd(char *data, size_t dataSize) {
    ASSISTANT_CMD_START_BT_PAIR_T *tmp = (ASSISTANT_CMD_START_BT_PAIR_T *)data;

    if (dataSize != sizeof(*tmp)) {
        AST_ERR("error dataSize:[%lu], correct:[%lu]\n", dataSize, sizeof(*tmp));
        return NULL;
    }

    cJSON *root = cJSON_CreateObject();
    if (NULL == root) {
        return NULL;
    }

    cJSON_AddStringToObject(root, STRING_COMMAND, STRING_START_BT_PAIR);

    char *str = cJSON_Print(root);
    printf("%s", str);

    shared_ptr<char> point = ConvertPointerToShared<char>(str, strlen(str) + 1);

    free(str);
    cJSON_Delete(root);

    return point;
}

shared_ptr<char> JsonCmd::ConstructBtDelPairedCmd(char *data, size_t dataSize) {
    ASSISTANT_CMD_DEL_BT_PAIRED_T *tmp = (ASSISTANT_CMD_DEL_BT_PAIRED_T *)data;

    if (dataSize != sizeof(*tmp)) {
        AST_ERR("error dataSize:[%lu], correct:[%lu]\n", dataSize, sizeof(*tmp));
        return NULL;
    }

    cJSON *root = cJSON_CreateObject();
    if (NULL == root) {
        return NULL;
    }

    cJSON_AddStringToObject(root, STRING_COMMAND, STRING_DEL_BT_PAIRED);

    char *str = cJSON_Print(root);
    printf("%s", str);

    shared_ptr<char> point = ConvertPointerToShared<char>(str, strlen(str) + 1);

    free(str);
    cJSON_Delete(root);

    return point;
}


shared_ptr<char> JsonCmd::ConstructBtPowerOffCmd(char *data, size_t dataSize) {
    ASSISTANT_CMD_BT_POWER_OFF_T *tmp = (ASSISTANT_CMD_BT_POWER_OFF_T *)data;

    if (dataSize != sizeof(*tmp)) {
        AST_ERR("error dataSize:[%lu], correct:[%lu]\n", dataSize, sizeof(*tmp));
        return NULL;
    }

    cJSON *root = cJSON_CreateObject();
    if (NULL == root) {
        return NULL;
    }

    cJSON_AddStringToObject(root, STRING_COMMAND, STRING_BT_POWER_OFF);

    char *str = cJSON_Print(root);
    printf("%s", str);
    shared_ptr<char> point = ConvertPointerToShared<char>(str, strlen(str) + 1);

    free(str);
    cJSON_Delete(root);

    return point;
}

shared_ptr<char> JsonCmd::ConstructPlayBtMusicCmd(char *data, size_t dataSize) {
    ASSISTANT_CMD_PLAY_BT_MUSIC_T *tmp = (ASSISTANT_CMD_PLAY_BT_MUSIC_T *)data;

    if (dataSize != sizeof(*tmp)) {
        AST_ERR("error dataSize:[%lu], correct:[%lu]\n", dataSize, sizeof(*tmp));
        return NULL;
    }

    cJSON *root = cJSON_CreateObject();
    if (NULL == root) {
        return NULL;
    }

    cJSON_AddStringToObject(root, STRING_COMMAND, STRING_PLAY_BT_MUSIC);

    char *str = cJSON_Print(root);
    printf("%s", str);

    shared_ptr<char> point = ConvertPointerToShared<char>(str, strlen(str) + 1);

    free(str);
    cJSON_Delete(root);

    return point;
}

shared_ptr<char> JsonCmd::ConstructBtDisconnectCmd(char *data, size_t dataSize) {
    ASSISTANT_CMD_BT_DISCONNECT_T *tmp = (ASSISTANT_CMD_BT_DISCONNECT_T *)data;

    if (dataSize != sizeof(*tmp)) {
        AST_ERR("error dataSize:[%lu], correct:[%lu]\n", dataSize, sizeof(*tmp));
        return NULL;
    }

    cJSON *root = cJSON_CreateObject();
    if (NULL == root) {
        return NULL;
    }

    cJSON_AddStringToObject(root, STRING_COMMAND, STRING_BT_DISCONNECT);

    char *str = cJSON_Print(root);
    printf("%s", str);

    shared_ptr<char> point = ConvertPointerToShared<char>(str, strlen(str) + 1);

    free(str);
    cJSON_Delete(root);

    return point;
}

shared_ptr<char> JsonCmd::ConstructGetApListCmd(char *data, size_t dataSize) {
    ASSISTANT_CMD_GET_AP_LIST_T *tmp = (ASSISTANT_CMD_GET_AP_LIST_T *)data;

    if (dataSize != sizeof(*tmp)) {
        AST_ERR("error dataSize:[%lu], correct:[%lu]\n", dataSize, sizeof(*tmp));
        return NULL;
    }

    cJSON *root = cJSON_CreateObject();
    if (NULL == root) {
        return NULL;
    }

    cJSON_AddStringToObject(root, STRING_COMMAND, STRING_GET_AP_LIST);
    cJSON_AddNumberToObject(root, STRING_ID, tmp->id);

    char *str = cJSON_Print(root);
    printf("%s", str);

    shared_ptr<char> point = ConvertPointerToShared<char>(str, strlen(str) + 1);

    free(str);
    cJSON_Delete(root);

    return point;
}

shared_ptr<char> JsonCmd::ConstructWifiConnectCmd(char *data, size_t dataSize) {
    ASSISTANT_CMD_WIFI_CONNECT_T *tmp = (ASSISTANT_CMD_WIFI_CONNECT_T *)data;

    if (dataSize != sizeof(*tmp)) {
        AST_ERR("error dataSize:[%lu], correct:[%lu]\n", dataSize, sizeof(*tmp));
        return NULL;
    }

    cJSON *root = cJSON_CreateObject();
    if (NULL == root) {
        return NULL;
    }

    cJSON_AddStringToObject(root, STRING_COMMAND, STRING_WIFI_CONNECT);

    cJSON *params = cJSON_CreateObject();
    if (NULL == params) {
        cJSON_Delete(root);
        return NULL;
    }

    cJSON_AddItemToObject(root, STRING_PARAMS, params);
    cJSON_AddStringToObject(params, STRING_SSID, tmp->ssid);
	cJSON_AddStringToObject(params, STRING_BSSID, tmp->bssid);
    cJSON_AddStringToObject(params, STRING_PASSWORD, tmp->password);
    cJSON_AddNumberToObject(params, STRING_AUTH_MODE, tmp->auth_mode);
    cJSON_AddNumberToObject(root, STRING_ID, tmp->id);

    char *str = cJSON_Print(root);
    printf("%s", str);

    shared_ptr<char> point = ConvertPointerToShared<char>(str, strlen(str) + 1);

    free(str);
    cJSON_Delete(root);

    return point;
}

shared_ptr<char> JsonCmd::ConstructWifiConnectOverCmd(char *data, size_t dataSize) {
    ASSISTANT_CMD_WIFI_CONNECT_OVER_T *tmp = (ASSISTANT_CMD_WIFI_CONNECT_OVER_T *)data;

    if (dataSize != sizeof(*tmp)) {
        AST_ERR("error dataSize:[%lu], correct:[%lu]\n", dataSize, sizeof(*tmp));
        return NULL;
    }

    cJSON *root = cJSON_CreateObject();
    if (NULL == root) {
        return NULL;
    }

    cJSON_AddStringToObject(root, STRING_COMMAND, STRING_WIFI_CONNECT_OVER);

    char *str = cJSON_Print(root);
    printf("%s", str);

    shared_ptr<char> point = ConvertPointerToShared<char>(str, strlen(str) + 1);

    free(str);
    cJSON_Delete(root);

    return point;
}

shared_ptr<char> JsonCmd::ConstructWifiSetupResultCmd(char *data, size_t dataSize) {
    ASSISTANT_CMD_WIFI_SETUP_RESULT_T *tmp = (ASSISTANT_CMD_WIFI_SETUP_RESULT_T *)data;

    if (dataSize != sizeof(*tmp)) {
        AST_ERR("error dataSize:[%lu], correct:[%lu]\n", dataSize, sizeof(*tmp));
        return NULL;
    }

    cJSON *root = cJSON_CreateObject();
    if (NULL == root) {
        return NULL;
    }

    cJSON_AddStringToObject(root, STRING_COMMAND, STRING_WIFI_SETUP_RESULT);

    cJSON *params = cJSON_CreateObject();
    if (NULL == params) {
        cJSON_Delete(root);
        return NULL;
    }

    cJSON_AddItemToObject(root, STRING_PARAMS, params);
    cJSON_AddNumberToObject(params, STRING_RESULT, tmp->result);

    char *str = cJSON_Print(root);
    printf("%s", str);

    shared_ptr<char> point = ConvertPointerToShared<char>(str, strlen(str) + 1);

    free(str);
    cJSON_Delete(root);

    return point;
}

shared_ptr<char> JsonCmd::ConstructSpeechStartCmd(char *data, size_t dataSize) {
    ASSISTANT_CMD_SPEECH_START_T *tmp = (ASSISTANT_CMD_SPEECH_START_T *)data;

    if (dataSize != sizeof(*tmp)) {
        AST_ERR("error dataSize:[%lu], correct:[%lu]\n", dataSize, sizeof(*tmp));
        return NULL;
    }

    cJSON *root = cJSON_CreateObject();
    if (NULL == root) {
        return NULL;
    }

    cJSON_AddStringToObject(root, STRING_COMMAND, STRING_SPEECH_START);

    char *str = cJSON_Print(root);
	//printf("GS test\n");
    printf("%s", str);
	//printf("GS test end\n");

    shared_ptr<char> point = ConvertPointerToShared<char>(str, strlen(str) + 1);

    free(str);
    cJSON_Delete(root);

    return point;
}

shared_ptr<char> JsonCmd::ConstructSpeechProcessCmd(char *data, size_t dataSize) {
    ASSISTANT_CMD_SPEECH_PROCESS_T *tmp = (ASSISTANT_CMD_SPEECH_PROCESS_T *)data;

    if (dataSize != sizeof(*tmp)) {
        AST_ERR("error dataSize:[%lu], correct:[%lu]\n", dataSize, sizeof(*tmp));
        return NULL;
    }

    cJSON *root = cJSON_CreateObject();
    if (NULL == root) {
        return NULL;
    }

    cJSON_AddStringToObject(root, STRING_COMMAND, STRING_SPEECH_PROCESS);

    char *str = cJSON_Print(root);
    printf("%s", str);

    shared_ptr<char> point = ConvertPointerToShared<char>(str, strlen(str) + 1);

    free(str);
    cJSON_Delete(root);

    return point;
}

shared_ptr<char> JsonCmd::ConstructSpeechFeedbackCmd(char *data, size_t dataSize) {
    ASSISTANT_CMD_SPEECH_FEEDBACK_T *tmp = (ASSISTANT_CMD_SPEECH_FEEDBACK_T *)data;

    if (dataSize != sizeof(*tmp)) {
        AST_ERR("error dataSize:[%lu], correct:[%lu]\n", dataSize, sizeof(*tmp));
        return NULL;
    }

    cJSON *root = cJSON_CreateObject();
    if (NULL == root) {
        return NULL;
    }

    cJSON_AddStringToObject(root, STRING_COMMAND, STRING_SPEECH_FEEDBACK);

    char *str = cJSON_Print(root);
    printf("%s", str);

    shared_ptr<char> point = ConvertPointerToShared<char>(str, strlen(str) + 1);

    free(str);
    cJSON_Delete(root);

    return point;
}

shared_ptr<char> JsonCmd::ConstructSpeechFinishCmd(char *data, size_t dataSize) {
    ASSISTANT_CMD_SPEECH_FINISH_T *tmp = (ASSISTANT_CMD_SPEECH_FINISH_T *)data;

    if (dataSize != sizeof(*tmp)) {
        AST_ERR("error dataSize:[%lu], correct:[%lu]\n", dataSize, sizeof(*tmp));
        return NULL;
    }

    cJSON *root = cJSON_CreateObject();
    if (NULL == root) {
        return NULL;
    }

    cJSON_AddStringToObject(root, STRING_COMMAND, STRING_SPEECH_FINISH);

    char *str = cJSON_Print(root);
    printf("%s", str);

    shared_ptr<char> point = ConvertPointerToShared<char>(str, strlen(str) + 1);

    free(str);
    cJSON_Delete(root);

    return point;
}

shared_ptr<char> JsonCmd::ConstructGetSpeakerStatusCmd(char *data, size_t dataSize) {
    ASSISTANT_CMD_GET_SPEAKER_STATUS_T *tmp = (ASSISTANT_CMD_GET_SPEAKER_STATUS_T *)data;

    if (dataSize != sizeof(*tmp)) {
        AST_ERR("error dataSize:[%lu], correct:[%lu]\n", dataSize, sizeof(*tmp));
        return NULL;
    }

    cJSON *root = cJSON_CreateObject();
    if (NULL == root) {
        return NULL;
    }

    cJSON_AddStringToObject(root, STRING_COMMAND, STRING_GET_SPEAKER_STATUS);
    cJSON_AddNumberToObject(root, STRING_ID, tmp->id);

    char *str = cJSON_Print(root);
    printf("%s", str);

    shared_ptr<char> point = ConvertPointerToShared<char>(str, strlen(str) + 1);

    free(str);
    cJSON_Delete(root);

    return point;
}

shared_ptr<char> JsonCmd::ConstructOtaUpgradeCmd(char *data, size_t dataSize) {
    ASSISTANT_CMD_OTA_UPGRADE_T *tmp = (ASSISTANT_CMD_OTA_UPGRADE_T *)data;

    if (dataSize != sizeof(*tmp)) {
        AST_ERR("error dataSize:[%lu], correct:[%lu]\n", dataSize, sizeof(*tmp));
        return NULL;
    }

    cJSON *root = cJSON_CreateObject();
    if (NULL == root) {
        return NULL;
    }

    cJSON_AddStringToObject(root, STRING_COMMAND, STRING_OTA_UPGRADE);

    cJSON *params = cJSON_CreateObject();
    if (NULL == params) {
        cJSON_Delete(root);
        return NULL;
    }

    cJSON_AddItemToObject(root, STRING_PARAMS, params);
    cJSON_AddStringToObject(params, STRING_OTA_URL, tmp->ota_url);

    char *str = cJSON_Print(root);
    printf("%s", str);

    shared_ptr<char> point = ConvertPointerToShared<char>(str, strlen(str) + 1);

    free(str);
    cJSON_Delete(root);

    return point;
}

shared_ptr<char> JsonCmd::ConstructAdjustProgressCmd(char *data, size_t dataSize) {
    ASSISTANT_CMD_ADJUST_PROGRESS_T *tmp = (ASSISTANT_CMD_ADJUST_PROGRESS_T *)data;

    if (dataSize != sizeof(*tmp)) {
        AST_ERR("error dataSize:[%lu], correct:[%lu]\n", dataSize, sizeof(*tmp));
        return NULL;
    }

    cJSON *root = cJSON_CreateObject();
    if (NULL == root) {
        return NULL;
    }

    cJSON_AddStringToObject(root, STRING_COMMAND, STRING_ADJUST_PROGRESS);

    cJSON *params = cJSON_CreateObject();
    if (NULL == params) {
        cJSON_Delete(root);
        return NULL;
    }

    cJSON_AddItemToObject(root, STRING_PARAMS, params);
    cJSON_AddNumberToObject(params, STRING_PROGRESS, tmp->progress);

    char *str = cJSON_Print(root);
    printf("%s", str);

    shared_ptr<char> point = ConvertPointerToShared<char>(str, strlen(str) + 1);

    free(str);
    cJSON_Delete(root);

    return point;
}

shared_ptr<char> JsonCmd::ConstructButtonCmd(char *data, size_t dataSize) {
    ASSISTANT_CMD_BUTTON_T *tmp = (ASSISTANT_CMD_BUTTON_T *)data;

    if (dataSize != sizeof(*tmp)) {
        AST_ERR("error dataSize:[%lu], correct:[%lu]\n", dataSize, sizeof(*tmp));
        return NULL;
    }

    cJSON *root = cJSON_CreateObject();
    if (NULL == root) {
        return NULL;
    }

    cJSON_AddStringToObject(root, STRING_COMMAND, STRING_BUTTON);

    cJSON *params = cJSON_CreateObject();
    if (NULL == params) {
        cJSON_Delete(root);
        return NULL;
    }

    cJSON_AddItemToObject(root, STRING_PARAMS, params);
    cJSON_AddStringToObject(params, STRING_NAME, tmp->name);

    char *str = cJSON_Print(root);
    printf("%s", str);

    shared_ptr<char> point = ConvertPointerToShared<char>(str, strlen(str) + 1);

    free(str);
    cJSON_Delete(root);

    return point;
}

shared_ptr<char> JsonCmd::ConstructFactoryResetResultCmd(char *data, size_t dataSize) {
    ASSISTANT_CMD_FACTORY_RESET_RESULT_T *tmp = (ASSISTANT_CMD_FACTORY_RESET_RESULT_T *)data;

    if (dataSize != sizeof(*tmp)) {
        AST_ERR("error dataSize:[%lu], correct:[%lu]\n", dataSize, sizeof(*tmp));
        return NULL;
    }

    cJSON *root = cJSON_CreateObject();
    if (NULL == root) {
        return NULL;
    }

    cJSON_AddStringToObject(root, STRING_COMMAND, STRING_FACTORY_RESET_RESULT);

    cJSON *params = cJSON_CreateObject();
    if (NULL == params) {
        cJSON_Delete(root);
        return NULL;
    }

    cJSON_AddItemToObject(root, STRING_PARAMS, params);
    cJSON_AddNumberToObject(params, STRING_RESULT, tmp->result);

    char *str = cJSON_Print(root);
    printf("%s", str);

    shared_ptr<char> point = ConvertPointerToShared<char>(str, strlen(str) + 1);

    free(str);
    cJSON_Delete(root);

    return point;
}

shared_ptr<char> JsonCmd::ConstructHfpFreeMicResultCmd(char *data, size_t dataSize) {
    ASSISTANT_CMD_HFP_FREE_MIC_RESULT_T *tmp = (ASSISTANT_CMD_HFP_FREE_MIC_RESULT_T *)data;

    if (dataSize != sizeof(*tmp)) {
        AST_ERR("error dataSize:[%lu], correct:[%lu]\n", dataSize, sizeof(*tmp));
        return NULL;
    }

    cJSON *root = cJSON_CreateObject();
    if (NULL == root) {
        return NULL;
    }

    cJSON_AddStringToObject(root, STRING_COMMAND, STRING_HFP_FREE_MIC_RESULT);

    cJSON *params = cJSON_CreateObject();
    if (NULL == params) {
        cJSON_Delete(root);
        return NULL;
    }

    cJSON_AddItemToObject(root, STRING_PARAMS, params);
    cJSON_AddNumberToObject(params, STRING_RESULT, tmp->result);

    char *str = cJSON_Print(root);
    printf("%s", str);

    shared_ptr<char> point = ConvertPointerToShared<char>(str, strlen(str) + 1);

    free(str);
    cJSON_Delete(root);

    return point;
}

shared_ptr<char> JsonCmd::DeconstructPlayDoneResponse(char *data, ASSISTANT_CMD_E &cmd) {
    cJSON *root = cJSON_Parse((const char *)data);
    if (NULL == root) {
        return nullptr;
    }

    shared_ptr<char> sp (new char[sizeof(ASSISTANT_CMD_PLAY_DONE_T)], [](char *p){delete []p;});
    ASSISTANT_CMD_PLAY_DONE_T *response = (ASSISTANT_CMD_PLAY_DONE_T *)sp.get();

    cJSON *command = cJSON_GetObjectItem(root, STRING_COMMAND);
    memcpy(response->command, command->valuestring, ASSISTANT_CMD_COMMAND_MAX_LENGTH);

    cJSON *params = cJSON_GetObjectItem(root, STRING_PARAMS);
    cJSON *uri = cJSON_GetObjectItem(params, STRING_URI);
    memcpy(response->uri, uri->valuestring, ASSISTANT_CMD_URI_MAX_LENGTH);
    cJSON *status = cJSON_GetObjectItem(params, STRING_STATUS);
    response->status = status->valueint;
	cJSON *error = cJSON_GetObjectItem(params, STRING_ERROR);
    cJSON *code = cJSON_GetObjectItem(error, STRING_CODE);
    response->code = code->valueint;

    cJSON_Delete(root);

    cmd = ASSISTANT_CMD_PLAY_DONE;
    return sp;
}

shared_ptr<char> JsonCmd::DeconstructPlayTtsDoneResponse(char *data, ASSISTANT_CMD_E &cmd) {
    cJSON *root = cJSON_Parse((const char *)data);
    if (NULL == root) {
        return nullptr;
    }

    shared_ptr<char> sp (new char[sizeof(ASSISTANT_CMD_PLAY_TTS_DONE_T)], [](char *p){delete []p;});
    ASSISTANT_CMD_PLAY_TTS_DONE_T *response = (ASSISTANT_CMD_PLAY_TTS_DONE_T *)sp.get();

    cJSON *command = cJSON_GetObjectItem(root, STRING_COMMAND);
    memcpy(response->command, command->valuestring, ASSISTANT_CMD_COMMAND_MAX_LENGTH);

    cJSON *params = cJSON_GetObjectItem(root, STRING_PARAMS);
    cJSON *ttsId = cJSON_GetObjectItem(params, STRING_TTS_ID);
    response->tts_id = ttsId->valueint;
    cJSON *status = cJSON_GetObjectItem(params, STRING_STATUS);
    response->status = status->valueint;

    cJSON_Delete(root);

    cmd = ASSISTANT_CMD_PLAY_TTS_DONE;
    return sp;
}

shared_ptr<char> JsonCmd::DeconstructSpeakerStatusResponse(char *data, ASSISTANT_CMD_E &cmd) {
    cJSON *root = cJSON_Parse((const char *)data);
    if (NULL == root) {
        return nullptr;
    }

    shared_ptr<char> sp (new char[sizeof(ASSISTANT_CMD_GET_SPEAKER_STATUS_RESPONSE_T)], [](char *p){delete []p;});
    ASSISTANT_CMD_GET_SPEAKER_STATUS_RESPONSE_T *response = (ASSISTANT_CMD_GET_SPEAKER_STATUS_RESPONSE_T *)sp.get();

    cJSON *command = cJSON_GetObjectItem(root, STRING_COMMAND);
    memcpy(response->command, command->valuestring, ASSISTANT_CMD_COMMAND_MAX_LENGTH);

    //result
    cJSON *result = cJSON_GetObjectItem(root, STRING_RESULT);

    //system
    {
        cJSON *system = cJSON_GetObjectItem(result, STRING_SYSTEM);
        cJSON *status = cJSON_GetObjectItem(system, STRING_STATUS);
        memcpy(response->system.status, status->valuestring, ASSISTANT_CMD_STATUS_MAX_LENGTH);
        cJSON *wifi_mac = cJSON_GetObjectItem(system, STRING_WIFI_MAC);
        memcpy(response->system.wifi_mac, wifi_mac->valuestring, ASSISTANT_CMD_WIFI_MAC_MAX_LENGTH);
        cJSON *bt_mac = cJSON_GetObjectItem(system, STRING_BT_MAC);
        memcpy(response->system.bt_mac, bt_mac->valuestring, ASSISTANT_CMD_BT_MAC_MAX_LENGTH);
        cJSON *device_sn = cJSON_GetObjectItem(system, STRING_DEVICE_SN);
        memcpy(response->system.device_sn, device_sn->valuestring, ASSISTANT_CMD_DEVICE_SN_MAX_LENGTH);
    }

    //player
    {
        cJSON *player = cJSON_GetObjectItem(result, STRING_PLAYER);
        cJSON *volume = cJSON_GetObjectItem(player, STRING_VOLUME);
        response->player.volume = volume->valueint;
        cJSON *status = cJSON_GetObjectItem(player, STRING_STATUS);
        memcpy(response->player.status, status->valuestring, ASSISTANT_CMD_STATUS_MAX_LENGTH);
        cJSON *source = cJSON_GetObjectItem(player, STRING_SOURCE);
        memcpy(response->player.source, source->valuestring, ASSISTANT_CMD_SOURCE_MAX_LENGTH);
        cJSON *audioId = cJSON_GetObjectItem(player, STRING_AUDIO_ID);
        memcpy(response->player.audioId, audioId->valuestring, ASSISTANT_CMD_AUDIO_ID_MAX_LENGTH);
        cJSON *audioSource = cJSON_GetObjectItem(player, STRING_AUDIO_SOURCE);
        memcpy(response->player.audioSource, audioSource->valuestring, ASSISTANT_CMD_AUDIO_SOURCE_MAX_LENGTH);
        cJSON *audioName = cJSON_GetObjectItem(player, STRING_AUDIO_NAME);
        memcpy(response->player.audioName, audioName->valuestring, ASSISTANT_CMD_AUDIO_NAME_MAX_LENGTH);
        cJSON *audioAnchor = cJSON_GetObjectItem(player, STRING_AUDIO_ANCHOR);
        memcpy(response->player.audioAnchor, audioAnchor->valuestring, ASSISTANT_CMD_AUDIO_ANCHOR_MAX_LENGTH);
        cJSON *audioAlbum = cJSON_GetObjectItem(player, STRING_AUDIO_ALBUM);
        memcpy(response->player.audioAlbum, audioAlbum->valuestring, ASSISTANT_CMD_AUDIO_ALBUM_MAX_LENGTH);
        cJSON *progress = cJSON_GetObjectItem(player, STRING_PROGRESS);
        response->player.progress = progress->valueint;
    }

    //network
    {
        cJSON *network = cJSON_GetObjectItem(result, STRING_NETWORK);
        cJSON *quantity = cJSON_GetObjectItem(network, STRING_QUANTITY);
        response->network.quantity = quantity->valueint;
        cJSON *status = cJSON_GetObjectItem(network, STRING_STATUS);
        memcpy(response->network.status, status->valuestring, ASSISTANT_CMD_STATUS_MAX_LENGTH);
        cJSON *ssid = cJSON_GetObjectItem(network, STRING_SSID);
        memcpy(response->network.ssid, ssid->valuestring, ASSISTANT_CMD_SSID_MAX_LENGTH);
        cJSON *bssid = cJSON_GetObjectItem(network, STRING_BSSID);
        memcpy(response->network.bssid, bssid->valuestring, ASSISTANT_CMD_BSSID_MAX_LENGTH);
    }

    //bluetooth
    {
        cJSON *bluetooth = cJSON_GetObjectItem(result, STRING_BLUETOOTH);
        cJSON *status = cJSON_GetObjectItem(bluetooth, STRING_STATUS);
        memcpy(response->bluetooth.status, status->valuestring, ASSISTANT_CMD_STATUS_MAX_LENGTH);
        cJSON *name = cJSON_GetObjectItem(bluetooth, STRING_NAME);
        memcpy(response->bluetooth.name, name->valuestring, ASSISTANT_CMD_BT_NAME_MAX_LENGTH);
        cJSON *bt_paired_name = cJSON_GetObjectItem(bluetooth, STRING_BT_PAIRED_NAME);
        memcpy(response->bluetooth.bt_paired_name, bt_paired_name->valuestring, ASSISTANT_CMD_BT_PAIRED_NAME_MAX_LENGTH);
    }

    //id
    cJSON *id = cJSON_GetObjectItem(root, STRING_ID);
    response->id = id->valueint;

    cJSON_Delete(root);

    cmd = ASSISTANT_CMD_GET_SPEAKER_STATUS;
    return sp;
}

shared_ptr<char> JsonCmd::DeconstructSystemStatusChangeResponse(char *data, ASSISTANT_CMD_E &cmd) {
    cJSON *root = cJSON_Parse((const char *)data);
    if (NULL == root) {
        return nullptr;
    }

    shared_ptr<char> sp (new char[sizeof(ASSISTANT_CMD_SYSTEM_STATUS_CHANGE_T)], [](char *p){delete []p;});
    ASSISTANT_CMD_SYSTEM_STATUS_CHANGE_T *response = (ASSISTANT_CMD_SYSTEM_STATUS_CHANGE_T *)sp.get();

    cJSON *command = cJSON_GetObjectItem(root, STRING_COMMAND);
    memcpy(response->command, command->valuestring, ASSISTANT_CMD_COMMAND_MAX_LENGTH);

    cJSON *params = cJSON_GetObjectItem(root, STRING_PARAMS);
    cJSON *status = cJSON_GetObjectItem(params, STRING_STATUS);
    memcpy(response->status, status->valuestring, ASSISTANT_CMD_STATUS_MAX_LENGTH);

    cJSON_Delete(root);

    cmd = ASSISTANT_CMD_SYSTEM_STATUS_CHANGE;
    return sp;
}

shared_ptr<char> JsonCmd::DeconstructPlayerStatusChangeResponse(char *data, ASSISTANT_CMD_E &cmd) {
    cJSON *root = cJSON_Parse((const char *)data);
    if (NULL == root) {
        return nullptr;
    }

    shared_ptr<char> sp (new char[sizeof(ASSISTANT_CMD_PLAYER_STATUS_CHANGE_T)], [](char *p){delete []p;});
    ASSISTANT_CMD_PLAYER_STATUS_CHANGE_T *response = (ASSISTANT_CMD_PLAYER_STATUS_CHANGE_T *)sp.get();

    cJSON *command = cJSON_GetObjectItem(root, STRING_COMMAND);
    memcpy(response->command, command->valuestring, ASSISTANT_CMD_COMMAND_MAX_LENGTH);

    cJSON *params = cJSON_GetObjectItem(root, STRING_PARAMS);
    cJSON *volume = cJSON_GetObjectItem(params, STRING_VOLUME);
    response->player.volume = volume->valueint;
    cJSON *status = cJSON_GetObjectItem(params, STRING_STATUS);
    memcpy(response->player.status, status->valuestring, ASSISTANT_CMD_STATUS_MAX_LENGTH);
    cJSON *source = cJSON_GetObjectItem(params, STRING_SOURCE);
    memcpy(response->player.source, source->valuestring, ASSISTANT_CMD_SOURCE_MAX_LENGTH);
    cJSON *audioId = cJSON_GetObjectItem(params, STRING_AUDIO_ID);
    memcpy(response->player.audioId, audioId->valuestring, ASSISTANT_CMD_AUDIO_ID_MAX_LENGTH);
    cJSON *audioUid = cJSON_GetObjectItem(params, STRING_AUDIO_UID);
    memcpy(response->player.audioUid, audioUid->valuestring, ASSISTANT_CMD_AUDIO_UID_MAX_LENGTH);
    cJSON *audioSource = cJSON_GetObjectItem(params, STRING_AUDIO_SOURCE);
    memcpy(response->player.audioSource, audioSource->valuestring, ASSISTANT_CMD_AUDIO_SOURCE_MAX_LENGTH);
    cJSON *audioName = cJSON_GetObjectItem(params, STRING_AUDIO_NAME);
    memcpy(response->player.audioName, audioName->valuestring, ASSISTANT_CMD_AUDIO_NAME_MAX_LENGTH);
    cJSON *audioAnchor = cJSON_GetObjectItem(params, STRING_AUDIO_ANCHOR);
    memcpy(response->player.audioAnchor, audioAnchor->valuestring, ASSISTANT_CMD_AUDIO_ANCHOR_MAX_LENGTH);
    cJSON *audioAlbum = cJSON_GetObjectItem(params, STRING_AUDIO_ALBUM);
    memcpy(response->player.audioAlbum, audioAlbum->valuestring, ASSISTANT_CMD_AUDIO_ALBUM_MAX_LENGTH);
    cJSON *progress = cJSON_GetObjectItem(params, STRING_PROGRESS);
    response->player.progress = progress->valueint;
    cJSON *audioExt = cJSON_GetObjectItem(params, STRING_AUDIO_EXT);
    memcpy(response->player.audioExt, audioExt->valuestring, ASSISTANT_CMD_AUDIO_EXT_MAX_LENGTH);

    cJSON_Delete(root);

    cmd = ASSISTANT_CMD_PLAYER_STATUS_CHANGE;
    return sp;
}

shared_ptr<char> JsonCmd::DeconstructBluetoothSrcAvrcpCmd(char *data, ASSISTANT_CMD_E &cmd) {
    cJSON *root = cJSON_Parse((const char *)data);
    if (NULL == root) {
        return nullptr;
    }

    shared_ptr<char> sp (new char[sizeof(ASSISTANT_CMD_BT_SRC_AVRCP_CMD_T)], [](char *p){delete []p;});
    ASSISTANT_CMD_BT_SRC_AVRCP_CMD_T *response = (ASSISTANT_CMD_BT_SRC_AVRCP_CMD_T *)sp.get();

	cJSON *command = cJSON_GetObjectItem(root, STRING_COMMAND);
    memcpy(response->command, command->valuestring, ASSISTANT_CMD_COMMAND_MAX_LENGTH);

    cJSON *params = cJSON_GetObjectItem(root, STRING_PARAMS);

    cJSON *request = cJSON_GetObjectItem(params, STRING_STATUS);
    memcpy(response->cmd.request, request->valuestring, ASSISTANT_CMD_STATUS_MAX_LENGTH);

    cJSON_Delete(root);

    cmd = ASSISTANT_CMD_BT_SRC_AVRCP_CMD;
    return sp;
}

shared_ptr<char> JsonCmd::DeconstructNetworkStatusChangeResponse(char *data, ASSISTANT_CMD_E &cmd) {
    cJSON *root = cJSON_Parse((const char *)data);
    if (NULL == root) {
        return nullptr;
    }

    shared_ptr<char> sp (new char[sizeof(ASSISTANT_CMD_NETWORK_STATUS_CHANGE_T)], [](char *p){delete []p;});
    ASSISTANT_CMD_NETWORK_STATUS_CHANGE_T *response = (ASSISTANT_CMD_NETWORK_STATUS_CHANGE_T *)sp.get();

    cJSON *command = cJSON_GetObjectItem(root, STRING_COMMAND);
    memcpy(response->command, command->valuestring, ASSISTANT_CMD_COMMAND_MAX_LENGTH);

    cJSON *params = cJSON_GetObjectItem(root, STRING_PARAMS);
    cJSON *quantity = cJSON_GetObjectItem(params, STRING_QUANTITY);
    response->quantity = quantity->valueint;
    cJSON *status = cJSON_GetObjectItem(params, STRING_STATUS);
    memcpy(response->status, status->valuestring, ASSISTANT_CMD_STATUS_MAX_LENGTH);
    cJSON *ssid = cJSON_GetObjectItem(params, STRING_SSID);
    memcpy(response->ssid, ssid->valuestring, ASSISTANT_CMD_SSID_MAX_LENGTH);
    cJSON *bssid = cJSON_GetObjectItem(params, STRING_BSSID);
    memcpy(response->bssid, bssid->valuestring, ASSISTANT_CMD_BSSID_MAX_LENGTH);

    cJSON_Delete(root);

    cmd = ASSISTANT_CMD_NETWORK_STATUS_CHANGE;
    return sp;
}

shared_ptr<char> JsonCmd::DeconstructBluetoothStatusChangeResponse(char *data, ASSISTANT_CMD_E &cmd) {
    cJSON *root = cJSON_Parse((const char *)data);
    if (NULL == root) {
        return nullptr;
    }

    shared_ptr<char> sp (new char[sizeof(ASSISTANT_CMD_BLUETOOTH_STATUS_CHANGE_T)], [](char *p){delete []p;});
    ASSISTANT_CMD_BLUETOOTH_STATUS_CHANGE_T *response = (ASSISTANT_CMD_BLUETOOTH_STATUS_CHANGE_T *)sp.get();

    cJSON *command = cJSON_GetObjectItem(root, STRING_COMMAND);
    memcpy(response->command, command->valuestring, ASSISTANT_CMD_COMMAND_MAX_LENGTH);

    cJSON *params = cJSON_GetObjectItem(root, STRING_PARAMS);
    cJSON *status = cJSON_GetObjectItem(params, STRING_STATUS);
    memcpy(response->status, status->valuestring, ASSISTANT_CMD_STATUS_MAX_LENGTH);
    cJSON *name = cJSON_GetObjectItem(params, STRING_NAME);
    memcpy(response->name, name->valuestring, ASSISTANT_CMD_BT_NAME_MAX_LENGTH);
    cJSON *bt_paired_name = cJSON_GetObjectItem(params, STRING_BT_PAIRED_NAME);
    memcpy(response->bt_paired_name, bt_paired_name->valuestring, ASSISTANT_CMD_BT_PAIRED_NAME_MAX_LENGTH);

    cJSON_Delete(root);

    cmd = ASSISTANT_CMD_BLUETOOTH_STATUS_CHANGE;
    return sp;
}

shared_ptr<char> JsonCmd::DeconstructWifiStatusResponse(char *data, ASSISTANT_CMD_E &cmd) {
    cJSON *root = cJSON_Parse((const char *)data);
    if (NULL == root) {
        return nullptr;
    }

    shared_ptr<char> sp (new char[sizeof(ASSISTANT_CMD_WIFI_CONNECT_RESPONSE_T)], [](char *p){delete []p;});
    ASSISTANT_CMD_WIFI_CONNECT_RESPONSE_T *response = (ASSISTANT_CMD_WIFI_CONNECT_RESPONSE_T *)sp.get();

    cJSON *command = cJSON_GetObjectItem(root, STRING_COMMAND);
    memcpy(response->command, command->valuestring, ASSISTANT_CMD_COMMAND_MAX_LENGTH);

    cJSON *result = cJSON_GetObjectItem(root, STRING_RESULT);
    cJSON *wifi_status = cJSON_GetObjectItem(result, STRING_WIFI_STATUS);
    response->wifi_status = wifi_status->valueint;
    cJSON *id = cJSON_GetObjectItem(root, STRING_ID);
    response->id = id->valueint;

    cJSON_Delete(root);

    cmd = ASSISTANT_CMD_WIFI_CONNECT;
    return sp;
}

shared_ptr<char> JsonCmd::DeconstructGetApListResponse(char *data, ASSISTANT_CMD_E &cmd) {
    cJSON *root = cJSON_Parse((const char *)data);
    if (NULL == root) {
        return nullptr;
    }

    shared_ptr<char> sp (new char[sizeof(ASSISTANT_CMD_GET_AP_LIST_RESPONSE_T)], [](char *p){delete []p;});
    ASSISTANT_CMD_GET_AP_LIST_RESPONSE_T *response = (ASSISTANT_CMD_GET_AP_LIST_RESPONSE_T *)sp.get();

    cJSON *command = cJSON_GetObjectItem(root, STRING_COMMAND);
    memcpy(response->command, command->valuestring, ASSISTANT_CMD_COMMAND_MAX_LENGTH);

    cJSON *id = cJSON_GetObjectItem(root, STRING_ID);
    response->id = id->valueint;

    cJSON *finish = cJSON_GetObjectItem(root, STRING_FINISH);
    response->finish = finish->valueint;

    cJSON *list = cJSON_GetObjectItem(root, STRING_LIST);
    response->list_num = cJSON_GetArraySize(list);

    for (int32 i=0; i < response->list_num; i++) {
        cJSON *array = cJSON_GetArrayItem(list, i);
        cJSON *ssid = cJSON_GetObjectItem(array, STRING_SSID);
        memcpy(response->ap_info[i].ssid, ssid->valuestring, ASSISTANT_CMD_SSID_MAX_LENGTH);
        cJSON *bssid = cJSON_GetObjectItem(array, STRING_BSSID);
        memcpy(response->ap_info[i].bssid, bssid->valuestring, ASSISTANT_CMD_BSSID_MAX_LENGTH);
        cJSON *auth_mode = cJSON_GetObjectItem(array, STRING_AUTH_MODE);
        response->ap_info[i].auth_mode = auth_mode->valueint;

        cJSON *level = cJSON_GetObjectItem(array, STRING_LEVEL);
        response->ap_info[i].level = level->valueint;

        cJSON *frequency = cJSON_GetObjectItem(array, STRING_FREQUENCY);
        response->ap_info[i].frequency = frequency->valueint;
    }

    cJSON_Delete(root);

    cmd = ASSISTANT_CMD_GET_AP_LIST;
    return sp;
}

shared_ptr<char> JsonCmd::DeconstructHfpStatusChangeResponse(char *data, ASSISTANT_CMD_E &cmd) {
    cJSON *root = cJSON_Parse((const char *)data);
    if (NULL == root) {
        return nullptr;
    }

    shared_ptr<char> sp (new char[sizeof(ASSISTANT_CMD_HFP_STATUS_CHANGE_T)], [](char *p){delete []p;});
    ASSISTANT_CMD_HFP_STATUS_CHANGE_T *response = (ASSISTANT_CMD_HFP_STATUS_CHANGE_T *)sp.get();

    cJSON *command = cJSON_GetObjectItem(root, STRING_COMMAND);
    memcpy(response->command, command->valuestring, ASSISTANT_CMD_COMMAND_MAX_LENGTH);

    cJSON *params = cJSON_GetObjectItem(root, STRING_PARAMS);
    cJSON *status = cJSON_GetObjectItem(params, STRING_STATUS);
    memcpy(response->status, status->valuestring, ASSISTANT_CMD_STATUS_MAX_LENGTH);

    cJSON_Delete(root);

    cmd = ASSISTANT_CMD_HFP_STATUS_CHANGE;
    return sp;
}
shared_ptr<char> JsonCmd::DeconstructDeconstructRecevButton(char *data, ASSISTANT_CMD_E &cmd) {
    cJSON *root = cJSON_Parse((const char *)data);
    if (NULL == root) {
        return nullptr;
    }

    shared_ptr<char> sp (new char[sizeof(ASSISTANT_CMD_BUTTON_T)], [](char *p){delete []p;});
    ASSISTANT_CMD_BUTTON_T *response = (ASSISTANT_CMD_BUTTON_T *)sp.get();

    cJSON *command = cJSON_GetObjectItem(root, STRING_COMMAND);
    memcpy(response->command, command->valuestring, ASSISTANT_CMD_COMMAND_MAX_LENGTH);

    cJSON *params = cJSON_GetObjectItem(root, STRING_PARAMS);
    cJSON *name = cJSON_GetObjectItem(params, STRING_NAME);
    memcpy(response->name, name->valuestring, ASSISTANT_CMD_STATUS_MAX_LENGTH);

    cJSON_Delete(root);

    cmd = ASSISTANT_CMD_BUTTON;
    return sp;
}

