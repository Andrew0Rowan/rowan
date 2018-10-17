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


#include "CmdDebug.h"
#include "AssistantDef.h"
#include "AssistantCmd.h"

extern "C"
{
#include "string.h"
}

using std::make_pair;

extern "C"
{
static void callback(ASSISTANT_CMD_E cmd, char *msg,void *pri_data)
{
    printf("notify cmd:[%d]!\n", cmd);
}
}

CmdDebug::CmdDebug() {
    mCmdMap.insert(make_pair(STRING_PLAY, &CmdDebug::SendPlayCmd));
    mCmdMap.insert(make_pair(STRING_PAUSE, &CmdDebug::SendPauseCmd));
    mCmdMap.insert(make_pair(STRING_STOP, &CmdDebug::SendStopCmd));
    mCmdMap.insert(make_pair(STRING_RESUME, &CmdDebug::SendResumeCmd));
    mCmdMap.insert(make_pair(STRING_SET_VOLUME, &CmdDebug::SendSetVolumeCmd));
    mCmdMap.insert(make_pair(STRING_ADJUST_PROGRESS, &CmdDebug::SendAdjustProgressCmd));
    mCmdMap.insert(make_pair(STRING_SET_SYSTEM_STATUS, &CmdDebug::SendSetSystemStatusCmd));
    mCmdMap.insert(make_pair(STRING_SET_BT_NAME, &CmdDebug::SendSetBtNameCmd));
    mCmdMap.insert(make_pair(STRING_START_BT_PAIR, &CmdDebug::SendStartBtPairCmd));
    mCmdMap.insert(make_pair(STRING_DEL_BT_PAIRED, &CmdDebug::SendDelBtPairedCmd));
    mCmdMap.insert(make_pair(STRING_BT_POWER_OFF, &CmdDebug::SendBtPowerOffCmd));
    mCmdMap.insert(make_pair(STRING_PLAY_BT_MUSIC, &CmdDebug::SendPlayBtMusicCmd));
    mCmdMap.insert(make_pair(STRING_BT_DISCONNECT, &CmdDebug::SendBtDisconnectCmd));
    mCmdMap.insert(make_pair(STRING_PLAY_PREV_AUDIO, &CmdDebug::SendPlayPrevAudio));
    mCmdMap.insert(make_pair(STRING_PLAY_NEXT_AUDIO, &CmdDebug::SendPlayNextAudio));
    mCmdMap.insert(make_pair(STRING_OTA_UPGRADE, &CmdDebug::SendOtaUpgradeCmd));
    mCmdMap.insert(make_pair(STRING_GET_AP_LIST, &CmdDebug::SendGetApListCmd));
    mCmdMap.insert(make_pair(STRING_WIFI_CONNECT, &CmdDebug::SendWifiConnectCmd));
    mCmdMap.insert(make_pair(STRING_WIFI_CONNECT_OVER, &CmdDebug::SendWifiConnectOverCmd));

    mCmdHub.Init(callback);
}

CmdDebug::~CmdDebug() {
    mCmdHub.DeInit();
}

void CmdDebug::SendDebugCmd(int argc, char *argv[]) {
    if (argc < 2) {
        AST_ERR("AssistantCenter debug must have parameters!\n");
        AST_ERR("like: ./AssistantCenter [parameters]!\n");
        return;
    }

    AST_INFO("Cmd is [%s]!\n", argv[1]);
    CmdFunc cmdFunc = mCmdMap[argv[1]];
    if (cmdFunc) {
        (this->*cmdFunc)(argc, argv);
    }
}

void CmdDebug::SendPlayCmd(int argc, char *argv[]) {
    if (argc < 3) {
        AST_ERR("/playback/play_tts error!\n");
        AST_ERR("usage: ./AssistantCenter /playback/play_tts [url]!\n");
        return;
    }

	ASSISTANT_CMD_PLAY_T play;
    memset(&play, 0, sizeof(play));
    strncpy(play.uri, argv[2], ASSISTANT_CMD_URI_MAX_LENGTH);
    int32 ret = mCmdHub.SendCmd(ASSISTANT_CMD_PLAY, (char *)&play, sizeof(play));
    if (ret < 0) {
        AST_INFO("[play] cmd send fail!\n");
    } else {
        AST_INFO("[play] cmd send success!\n");
    }
}

void CmdDebug::SendPauseCmd(int argc, char *argv[]) {
    ASSISTANT_CMD_PAUSE_T pause;

    memset(&pause, 0, sizeof(pause));
    int32 ret = mCmdHub.SendCmd(ASSISTANT_CMD_PAUSE, (char *)&pause, sizeof(pause));
    if (ret < 0) {
        AST_INFO("[pause] cmd send fail!\n");
    } else {
        AST_INFO("[pause] cmd send success!\n");
    }
}

void CmdDebug::SendStopCmd(int argc, char *argv[]) {
    ASSISTANT_CMD_STOP_T stop;

    memset(&stop, 0, sizeof(stop));
    int32 ret = mCmdHub.SendCmd(ASSISTANT_CMD_STOP, (char *)&stop, sizeof(stop));
    if (ret < 0) {
        AST_INFO("[stop] cmd send fail!\n");
    } else {
        AST_INFO("[stop] cmd send success!\n");
    }
}

void CmdDebug::SendResumeCmd(int argc, char *argv[]) {
    ASSISTANT_CMD_RESUME_T resume;

    memset(&resume, 0, sizeof(resume));
    int32 ret = mCmdHub.SendCmd(ASSISTANT_CMD_RESUME, (char *)&resume, sizeof(resume));
    if (ret < 0) {
        AST_INFO("[resume] cmd send fail!\n");
    } else {
        AST_INFO("[resume] cmd send success!\n");
    }
}

void CmdDebug::SendSetVolumeCmd(int argc, char *argv[]) {
    if (argc < 3) {
        AST_ERR("/system/set_volume error!\n");
        AST_ERR("usage: ./AssistantCenter /system/set_volume [0-100]!\n");
        return;
    }

    ASSISTANT_CMD_SET_VOLUME_T setVolume;
    memset(&setVolume, 0, sizeof(setVolume));
    setVolume.volume = atoi(argv[2]);
    int32 ret = mCmdHub.SendCmd(ASSISTANT_CMD_SET_VOLUME, (char *)&setVolume, sizeof(setVolume));
    if (ret < 0) {
        AST_INFO("[setVolume] cmd send fail!\n");
    } else {
        AST_INFO("[setVolume] cmd send success!\n");
    }
}

void CmdDebug::SendAdjustProgressCmd(int argc, char *argv[]) {
    if (argc < 3) {
        AST_ERR("/playback/adjust_progress error!\n");
        AST_ERR("usage: ./AssistantCenter /playback/adjust_progress [0-~]!\n");
        return;
    }

    ASSISTANT_CMD_ADJUST_PROGRESS_T adjustProgress;
    memset(&adjustProgress, 0, sizeof(adjustProgress));
    adjustProgress.progress = atoi(argv[2]);
    int32 ret = mCmdHub.SendCmd(ASSISTANT_CMD_ADJUST_PROGRESS, (char *)&adjustProgress, sizeof(adjustProgress));
    if (ret < 0) {
        AST_INFO("[adjustProgress] cmd send fail!\n");
    } else {
        AST_INFO("[adjustProgress] cmd send success!\n");
    }
}

void CmdDebug::SendSetSystemStatusCmd(int argc, char *argv[]) {
    if (argc < 3) {
        AST_ERR("/system/set_system_status error!\n");
        AST_ERR("usage: ./AssistantCenter /system/set_system_status [standby/normal]!\n");
        return;
    }

    ASSISTANT_CMD_SET_SYSTEM_STATUS_T setSystemStatus;
    memset(&setSystemStatus, 0, sizeof(setSystemStatus));
    strncpy(setSystemStatus.status, argv[2], ASSISTANT_CMD_STATUS_MAX_LENGTH);
    int32 ret = mCmdHub.SendCmd(ASSISTANT_CMD_SET_SYSTEM_STATUS, (char *)&setSystemStatus, sizeof(setSystemStatus));
    if (ret < 0) {
        AST_INFO("[setSystemStatus] cmd send fail!\n");
    } else {
        AST_INFO("[setSystemStatus] cmd send success!\n");
    }
}

void CmdDebug::SendSetBtNameCmd(int argc, char *argv[]) {
    if (argc < 3) {
        AST_ERR("/bluetooth/set_bt_name error!\n");
        AST_ERR("usage: ./AssistantCenter /bluetooth/set_bt_name [bt_name]!\n");
        return;
    }

    ASSISTANT_CMD_SET_BT_NAME_T setBtName;
    memset(&setBtName, 0, sizeof(setBtName));
    strncpy(setBtName.name, argv[2], ASSISTANT_CMD_BT_NAME_MAX_LENGTH);
    int32 ret = mCmdHub.SendCmd(ASSISTANT_CMD_SET_BT_NAME, (char *)&setBtName, sizeof(setBtName));
    if (ret < 0) {
        AST_INFO("[setBtName] cmd send fail!\n");
    } else {
        AST_INFO("[setBtName] cmd send success!\n");
    }
}

void CmdDebug::SendStartBtPairCmd(int argc, char *argv[]) {
    ASSISTANT_CMD_START_BT_PAIR_T startBtPair;

    memset(&startBtPair, 0, sizeof(startBtPair));
    int32 ret = mCmdHub.SendCmd(ASSISTANT_CMD_START_BT_PAIR, (char *)&startBtPair, sizeof(startBtPair));
    if (ret < 0) {
        AST_INFO("[startBtPair] cmd send fail!\n");
    } else {
        AST_INFO("[startBtPair] cmd send success!\n");
    }
}

void CmdDebug::SendDelBtPairedCmd(int argc, char *argv[]) {
    if (argc < 2) {
    AST_ERR("input error!\n");
    AST_ERR("usage: ./AssistantCenter /bluetooth/del_bt_paired!\n");
    return;
    }

    ASSISTANT_CMD_DEL_BT_PAIRED_T delBtPaired;
    memset(&delBtPaired, 0, sizeof(delBtPaired));
    int32 ret = mCmdHub.SendCmd(ASSISTANT_CMD_DEL_BT_PAIRED, (char *)&delBtPaired, sizeof(delBtPaired));
    if (ret < 0) {
        AST_INFO("[delBtPaired] cmd send fail!\n");
    } else {
        AST_INFO("[delBtPaired] cmd send success!\n");
    }
}

void CmdDebug::SendBtPowerOffCmd(int argc, char *argv[]) {
    if (argc < 2) {
    AST_ERR("input error!\n");
    AST_ERR("usage: ./AssistantCenter /bluetooth/power_off!\n");
    return;
    }

    ASSISTANT_CMD_BT_POWER_OFF_T BtPoweroff;
    memset(&BtPoweroff, 0, sizeof(BtPoweroff));
    int32 ret = mCmdHub.SendCmd(ASSISTANT_CMD_BT_POWER_OFF, (char *)&BtPoweroff, sizeof(BtPoweroff));
    if (ret < 0) {
    AST_INFO("[BtPoweroff] cmd send fail!\n");
    } else {
    AST_INFO("[BtPoweroff] cmd send success!\n");
    }
}

void CmdDebug::SendPlayBtMusicCmd(int argc, char *argv[]) {
    ASSISTANT_CMD_PLAY_BT_MUSIC_T playBtMusic;

    memset(&playBtMusic, 0, sizeof(playBtMusic));
    int32 ret = mCmdHub.SendCmd(ASSISTANT_CMD_PLAY_BT_MUSIC, (char *)&playBtMusic, sizeof(playBtMusic));
    if (ret < 0) {
        AST_INFO("[playBtMusic] cmd send fail!\n");
    } else {
        AST_INFO("[playBtMusic] cmd send success!\n");
    }
}

void CmdDebug::SendBtDisconnectCmd(int argc, char *argv[]) {
    ASSISTANT_CMD_BT_DISCONNECT_T btDisconnect;

    memset(&btDisconnect, 0, sizeof(btDisconnect));
    int32 ret = mCmdHub.SendCmd(ASSISTANT_CMD_BT_DISCONNECT, (char *)&btDisconnect, sizeof(btDisconnect));
    if (ret < 0) {
        AST_INFO("[btDisconnect] cmd send fail!\n");
    } else {
        AST_INFO("[btDisconnect] cmd send success!\n");
    }
}

void CmdDebug::SendPlayPrevAudio(int argc, char *argv[]) {
    ASSISTANT_CMD_PLAY_PREV_AUDIO_T playPrevAudio;

    memset(&playPrevAudio, 0, sizeof(playPrevAudio));
    int32 ret = mCmdHub.SendCmd(ASSISTANT_CMD_PLAY_PREV_AUDIO, (char *)&playPrevAudio, sizeof(playPrevAudio));
    if (ret < 0) {
        AST_INFO("[playPrevAudio] cmd send fail!\n");
    } else {
        AST_INFO("[playPrevAudio] cmd send success!\n");
    }
}

void CmdDebug::SendPlayNextAudio(int argc, char *argv[]) {
    ASSISTANT_CMD_PLAY_NEXT_AUDIO_T playNextAudio;

    memset(&playNextAudio, 0, sizeof(playNextAudio));
    int32 ret = mCmdHub.SendCmd(ASSISTANT_CMD_PLAY_NEXT_AUDIO, (char *)&playNextAudio, sizeof(playNextAudio));
    if (ret < 0) {
        AST_INFO("[playNextAudio] cmd send fail!\n");
    } else {
        AST_INFO("[playNextAudio] cmd send success!\n");
    }
}

void CmdDebug::SendOtaUpgradeCmd(int argc, char *argv[]) {
    if (argc < 3) {
        AST_ERR("/system/ota_upgrade error!\n");
        AST_ERR("usage: ./AssistantCenter /system/ota_upgrade [ota_url]!\n");
        return;
    }

    ASSISTANT_CMD_OTA_UPGRADE_T otaUpgrade;
    memset(&otaUpgrade, 0, sizeof(otaUpgrade));
    strncpy(otaUpgrade.ota_url, argv[2], ASSISTANT_CMD_URL_MAX_LENGTH);
    int32 ret = mCmdHub.SendCmd(ASSISTANT_CMD_OTA_UPGRADE, (char *)&otaUpgrade, sizeof(otaUpgrade));
    if (ret < 0) {
        AST_INFO("[otaUpgrade] cmd send fail!\n");
    } else {
        AST_INFO("[otaUpgrade] cmd send success!\n");
    }
}

void CmdDebug::SendGetApListCmd(int argc, char *argv[]) {
    ASSISTANT_CMD_GET_AP_LIST_T cmd = {0};
    int32 ret = mCmdHub.SendCmd(ASSISTANT_CMD_GET_AP_LIST, (char *)&cmd, sizeof(cmd));
    if (ret < 0) {
        AST_INFO("[GetApList] cmd send fail!\n");
    } else {
        AST_INFO("[GetApList] cmd send success!\n");
    }
}

void CmdDebug::SendWifiConnectCmd(int argc, char *argv[]) {
    if (argc < 5) {
        AST_ERR("/wifi/wifi_connect error!\n");
        AST_ERR("usage: ./AssistantCenter /wifi/wifi_connect [ssid] [password] [authmode]!\n");
        return;
    }

    ASSISTANT_CMD_WIFI_CONNECT_T cmd = {0};
    strncpy(cmd.ssid, argv[2], ASSISTANT_CMD_SSID_MAX_LENGTH);
    strncpy(cmd.password, argv[3], ASSISTANT_CMD_PASSWORD_MAX_LENGTH);
    cmd.auth_mode = atoi(argv[4]);
    int32 ret = mCmdHub.SendCmd(ASSISTANT_CMD_WIFI_CONNECT, (char *)&cmd, sizeof(cmd));
    if (ret < 0) {
        AST_INFO("[WifiConnect] cmd send fail!\n");
    } else {
        AST_INFO("[WifiConnect] cmd send success!\n");
    }
}

void CmdDebug::SendWifiConnectOverCmd(int argc, char *argv[]) {
    ASSISTANT_CMD_WIFI_CONNECT_OVER_T cmd = {0};
    int32 ret = mCmdHub.SendCmd(ASSISTANT_CMD_WIFI_CONNECT_OVER, (char *)&cmd, sizeof(cmd));
    if (ret < 0) {
        AST_INFO("[WifiConnectOver] cmd send fail!\n");
    } else {
        AST_INFO("[WifiConnectOver] cmd send success!\n");
    }
}

