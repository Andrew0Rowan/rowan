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



#ifndef _U_ASSISTANT_STUB_H_
#define _U_ASSISTANT_STUB_H_

/*-----------------------------------------------------------------------------
                    include files
-----------------------------------------------------------------------------*/
#include "u_amb.h"

#define ASSISTANT_STUB_APPR_OK                  (0)
#define ASSISTANT_STUB_APPR_FAIL                (-1)
#define ASSISTANT_STUB_COMMAND_MAX_LENGTH       (32)
#define ASSISTANT_STUB_TYPE_LENGTH              (16)
#define ASSISTANT_STUB_ALGORITHM_LENGTH         (16)
#define ASSISTANT_STUB_DATA_MAX_SIZE            (512)
#define ASSISTANT_STUB_URI_MAX_LENGTH           (256)
#define ASSISTANT_STUB_URL_MAX_LENGTH           (256)
#define ASSISTANT_STUB_TITLE_MAX_LENGTH         (128)
#define ASSISTANT_STUB_ARTIST_MAX_LENGTH        (32)
#define ASSISTANT_STUB_SOURCE_MAX_LENGTH        (16)
#define ASSISTANT_STUB_SSID_MAX_LENGTH          (32)
#define ASSISTANT_STUB_BSSID_MAX_LENGTH         (30)
#define ASSISTANT_STUB_PASSWORD_MAX_LENGTH      (64)
#define ASSISTANT_STUB_ACTION_MAX_LENGTH        (16)
#define ASSISTANT_STUB_STATUS_MAX_LENGTH        (16)
#define ASSISTANT_STUB_AUDIO_TYPE_MAX_LENGTH    (16)
#define ASSISTANT_STUB_AUDIO_NAME_MAX_LENGTH    (256)
#define ASSISTANT_STUB_AUDIO_ANCHOR_MAX_LENGTH  (256)
#define ASSISTANT_STUB_AUDIO_SOURCE_MAX_LENGTH  (32)
#define ASSISTANT_STUB_AUDIO_ALBUM_MAX_LENGTH   (256)
#define ASSISTANT_STUB_AUDIO_ID_MAX_LENGTH      (48)
#define ASSISTANT_STUB_AUDIO_UID_MAX_LENGTH     (64)
#define ASSISTANT_STUB_AUDIO_EXT_MAX_LENGTH     (512)
#define ASSISTANT_STUB_BT_NAME_MAX_LENGTH       (96)
#define ASSISTANT_STUB_BT_PAIRED_NAME_MAX_LENGTH (96)
#define ASSISTANT_STUB_CRONTAB_MAX_LENGTH       (96)
#define ASSISTANT_STUB_AUDIOURL_MAX_LENGTH      (256)
#define ASSISTANT_STUB_ALAEM_NUM_MAX            (8)
#define ASSISTANT_STUB_WIFI_MAC_MAX_LENGTH      (32)
#define ASSISTANT_STUB_BT_MAC_MAX_LENGTH        (32)
#define ASSISTANT_STUB_DEVICE_SN_MAX_LENGTH     (32)
#define ASSISTANT_STUB_BUTTON_NAME_MAX_LENGTH   (32)
#define ASSISTANT_STUB_AP_LIST_MAX              (10)

#define STRING_PLAY                     "/playback/play"
#define STRING_PLAY_VOICE_PROMPT        "/playback/play_voice_prompt"
#define STRING_PLAY_TTS                 "/playback/play_tts"
#define STRING_PAUSE                    "/playback/pause"
#define STRING_STOP                     "/playback/stop"
#define STRING_RESUME                   "/playback/resume"
#define STRING_SET_VOLUME               "/system/set_volume"
#define STRING_ADJUST_PROGRESS          "/playback/adjust_progress"
#define STRING_SET_SYSTEM_STATUS        "/system/set_system_status"
#define STRING_PLAY_PREV_AUDIO          "/bluetooth/play_prev_audio"
#define STRING_PLAY_NEXT_AUDIO          "/bluetooth/play_next_audio"
#define STRING_SET_BT_NAME              "/bluetooth/set_bt_name"
#define STRING_START_BT_PAIR            "/bluetooth/start_bt_pair"
#define STRING_DEL_BT_PAIRED            "/bluetooth/del_bt_paired"
#define STRING_BT_POWER_ON              "/bluetooth/power_on"
#define STRING_BT_POWER_OFF             "/bluetooth/power_off"
#define STRING_PLAY_BT_MUSIC            "/bluetooth/play_bt_music"
#define STRING_BT_DISCONNECT            "/bluetooth/bt_disconnect"
#define STRING_SPEECH_START             "/speech/start"
#define STRING_SPEECH_PROCESS           "/speech/process"
#define STRING_SPEECH_FEEDBACK          "/speech/feedback"
#define STRING_SPEECH_FINISH            "/speech/finish"
#define STRING_WIFI_CONNECT_OVER        "/wifi/wifi_connect_over"
#define STRING_WIFI_SETUP_RESULT        "/wifi/wifi_setup_result"
#define STRING_OTA_UPGRADE              "/system/ota_upgrade"
#define STRING_OTA_PROGRESS             "/system/ota_progress"
#define STRING_FACTORY_RESET_RESULT     "/system/factory_reset_result"
#define STRING_HFP_FREE_MIC_RESULT      "/system/hfp_free_mic_result"
#define STRING_GET_SPEAKER_STATUS       "/system/get_speaker_status"
#define STRING_GET_AP_LIST              "/wifi/get_ap_list"
#define STRING_WIFI_CONNECT             "/wifi/wifi_connect"
#define STRING_PLAY_DONE                "/playback/play_done"
#define STRING_PLAY_TTS_DONE            "/playback/play_tts_done"
#define STRING_SYSTEM_STATUS_CHANGE     "/system/system_status_change"
#define STRING_PLAYER_STATUS_CHANGE     "/playback/player_status_change"
#define STRING_NETWORK_STATUS_CHANGE    "/system/network_status_change"
#define STRING_BLUETOOTH_STATUS_CHANGE  "/system/bluetooth_status_change"
#define STRING_BUTTON                   "button"
#define STRING_VOLUME_INC               "volume_inc"
#define STRING_VOLUME_DEC               "volume_dec"
#define STRING_WIFI_SETUP               "wifi_setup"
#define STRING_FACTORY_RESET            "factory_reset"
#define STRING_MIC_MUTE                 "mic_mute"
#define STRING_BT_RECONNECT             "bt_reconnect"
#define STRING_SELF_TEST                "self_test"
#define STRING_FACTORY_OTA              "factory_ota"
#define STRING_HFP_STATUS_CHANGE        "/system/hfp_status_change"

#define ASSISTANT_STUB_JSONRPC           "jsonrpc"
#define ASSISTANT_STUB_COMMAND           "command"
#define ASSISTANT_STUB_PARAMS            "params"
#define ASSISTANT_STUB_TYPE              "type"
#define ASSISTANT_STUB_DATA              "data"
#define ASSISTANT_STUB_ALGORITHM         "algorithm"
#define ASSISTANT_STUB_STATUS            "status"
#define ASSISTANT_STUB_VOLUME            "volume"
#define ASSISTANT_STUB_SOURCE            "source"
#define ASSISTANT_STUB_AUDIO_TYPE        "audio_type"
#define ASSISTANT_STUB_AUDIO_NAME        "audioName"
#define ASSISTANT_STUB_AUDIO_ID          "audioId"
#define ASSISTANT_STUB_AUDIO_UID         "audioUid"
#define ASSISTANT_STUB_AUDIO_ANCHOR      "audioAnchor"
#define ASSISTANT_STUB_AUDIO_ALBUM       "audioAlbum"
#define ASSISTANT_STUB_AUDIO_SOURCE      "audioSource"
#define ASSISTANT_STUB_PROGRESS          "progress"
#define ASSISTANT_STUB_AUDIO_EXT         "audioExt"
#define ASSISTANT_STUB_URI               "uri"
#define ASSISTANT_STUB_ERROR             "error"
#define ASSISTANT_STUB_CODE              "code"
#define ASSISTANT_STUB_RESULT            "result"
#define ASSISTANT_STUB_WIFI_STATUS       "wifi_status"
#define ASSISTANT_STUB_ID                "id"
#define ASSISTANT_STUB_FINISH            "finish"
#define ASSISTANT_STUB_LIST              "list"
#define ASSISTANT_STUB_ON                "on"
#define ASSISTANT_STUB_EXPIRES_IN_MS     "expires_in_ms"
#define ASSISTANT_STUB_SET_VOLUME        "set_volume"
#define ASSISTANT_STUB_ACTION            "action"
#define ASSISTANT_STUB_TTS_ID            "tts_id"
#define ASSISTANT_STUB_SYSTEM            "system"
#define ASSISTANT_STUB_PLAYER            "player"
#define ASSISTANT_STUB_POWER             "power"
#define ASSISTANT_STUB_NETWORK           "network"
#define ASSISTANT_STUB_BLUETOOTH         "bluetooth"
#define ASSISTANT_STUB_QUANTITY          "quantity"
#define ASSISTANT_STUB_CRONTAB           "crontab"
#define ASSISTANT_STUB_AUDIOURL          "audioUrl"
#define ASSISTANT_STUB_WIFI_MAC          "wifi_mac"
#define ASSISTANT_STUB_BT_MAC            "bt_mac"
#define ASSISTANT_STUB_DEVICE_SN         "device_sn"
#define ASSISTANT_STUB_NAME              "name"
#define ASSISTANT_STUB_BT_PAIRED_NAME    "bt_paired_name"
#define ASSISTANT_STUB_TITLE             "title"
#define ASSISTANT_STUB_ARTIST            "artist"
#define ASSISTANT_STUB_FEEDBACK          "feedback"
#define ASSISTANT_STUB_SPEECH_ENABLE     "speech_enable"
#define ASSISTANT_STUB_NEED_MIX          "need_mix"
#define ASSISTANT_STUB_PCM               "pcm"
#define ASSISTANT_STUB_SAMPLERATE        "samplerate"
#define ASSISTANT_STUB_CHANNEL           "channels"
#define ASSISTANT_STUB_SSID              "ssid"
#define ASSISTANT_STUB_PASSWORD          "password"
#define ASSISTANT_STUB_AUTH_MODE         "auth_mode"
#define ASSISTANT_STUB_BSSID             "bssid"
#define ASSISTANT_STUB_LEVEL             "level"
#define ASSISTANT_STUB_FREQUENCY         "frequency"
#define ASSISTANT_STUB_OTA_URL           "ota_url"

#define FUNCTION_BEGIN printf("<ASSISTANT_STUB_APP> %s %d line begin \n",__FUNCTION__,__LINE__);
#define FUNCTION_END   printf("<ASSISTANT_STUB_APP> %s %d line end \n",__FUNCTION__,__LINE__);

typedef struct _ASSISTANT_STUB_PLAY_T
{
    char command[ASSISTANT_STUB_COMMAND_MAX_LENGTH+1];
    char uri[ASSISTANT_STUB_URI_MAX_LENGTH+1];
    char audioId[ASSISTANT_STUB_AUDIO_ID_MAX_LENGTH+1];
    char audioUid[ASSISTANT_STUB_AUDIO_UID_MAX_LENGTH+1];
    char audioSource[ASSISTANT_STUB_AUDIO_SOURCE_MAX_LENGTH+1];
    char audioName[ASSISTANT_STUB_AUDIO_NAME_MAX_LENGTH+1];
    char audioAnchor[ASSISTANT_STUB_AUDIO_ANCHOR_MAX_LENGTH+1];
    char audioAlbum[ASSISTANT_STUB_AUDIO_ALBUM_MAX_LENGTH+1];
    int  progress;
    char audioExt[ASSISTANT_STUB_AUDIO_EXT_MAX_LENGTH+1];
}ASSISTANT_STUB_PLAY_T;

typedef struct _ASSISTANT_STUB_PLAY_VOICE_PROMPT_T
{
    char command[ASSISTANT_STUB_COMMAND_MAX_LENGTH+1];
    char uri[ASSISTANT_STUB_URI_MAX_LENGTH+1];
    int  volume;
    char type[ASSISTANT_STUB_TYPE_LENGTH+1];
    BOOL feedback;
}ASSISTANT_STUB_PLAY_VOICE_PROMPT_T;

typedef struct _ASSISTANT_STUB_PLAY_TTS_T
{
    char command[ASSISTANT_STUB_COMMAND_MAX_LENGTH+1];
    int  tts_id;
    BOOL speech_enable;
    BOOL need_mix;
    BOOL pcm;
    int mSampleRate;
    int mChannels;
}ASSISTANT_STUB_PLAY_TTS_T;

typedef struct _ASSISTANT_STUB_PAUSE_T
{
    char command[ASSISTANT_STUB_COMMAND_MAX_LENGTH+1];
}ASSISTANT_STUB_PAUSE_T;

typedef struct _ASSISTANT_STUB_RESUME_T
{
    char command[ASSISTANT_STUB_COMMAND_MAX_LENGTH+1];
}ASSISTANT_STUB_RESUME_T;

typedef struct _ASSISTANT_STUB_SET_VOLUME_T
{
    char command[ASSISTANT_STUB_COMMAND_MAX_LENGTH+1];
    int  volume;
}ASSISTANT_STUB_SET_VOLUME_T;

typedef struct _ASSISTANT_STUB_GET_SPEAKER_STATUS_T
{
    char command[ASSISTANT_STUB_COMMAND_MAX_LENGTH+1];
    int  id;
}ASSISTANT_STUB_GET_SPEAKER_STATUS_T;

typedef struct _ASSISTANT_STUB_SYSTEM_T
{
    char status[ASSISTANT_STUB_STATUS_MAX_LENGTH+1];
    char wifi_mac[ASSISTANT_STUB_WIFI_MAC_MAX_LENGTH+1];
    char bt_mac[ASSISTANT_STUB_BT_MAC_MAX_LENGTH+1];
    char device_sn[ASSISTANT_STUB_DEVICE_SN_MAX_LENGTH+1];
}ASSISTANT_STUB_SYSTEM_T;

typedef struct _ASSISTANT_STUB_PLAYER_T
{
    int  volume;
    char status[ASSISTANT_STUB_STATUS_MAX_LENGTH+1];
    char source[ASSISTANT_STUB_SOURCE_MAX_LENGTH+1];
    char audioId[ASSISTANT_STUB_AUDIO_ID_MAX_LENGTH+1];
    char audioUid[ASSISTANT_STUB_AUDIO_UID_MAX_LENGTH+1];
    char audioSource[ASSISTANT_STUB_AUDIO_SOURCE_MAX_LENGTH+1];
    char audioName[ASSISTANT_STUB_AUDIO_NAME_MAX_LENGTH+1];
    char audioAnchor[ASSISTANT_STUB_AUDIO_ANCHOR_MAX_LENGTH+1];
    char audioAlbum[ASSISTANT_STUB_AUDIO_ALBUM_MAX_LENGTH+1];
    int  progress;
    char audioExt[ASSISTANT_STUB_AUDIO_EXT_MAX_LENGTH+1];
}ASSISTANT_STUB_PLAYER_T;

typedef struct _ASSISTANT_STUB_AVRCP_CMD_T
{
    int  volume;
    char request[ASSISTANT_STUB_STATUS_MAX_LENGTH+1];
}ASSISTANT_STUB_AVRCP_CMD_T;

typedef struct _ASSISTANT_STUB_POWER_T
{
    int  quantity;
    char status[ASSISTANT_STUB_STATUS_MAX_LENGTH+1];
}ASSISTANT_STUB_POWER_T;

typedef struct _ASSISTANT_STUB_NETWORK_T
{
    int  quantity;
    char status[ASSISTANT_STUB_STATUS_MAX_LENGTH+1];
    char ssid[ASSISTANT_STUB_SSID_MAX_LENGTH+1];
    char bssid[ASSISTANT_STUB_BSSID_MAX_LENGTH+1];
}ASSISTANT_STUB_NETWORK_T;

typedef struct _ASSISTANT_STUB_BLUETOOTH_T
{
    char status[ASSISTANT_STUB_STATUS_MAX_LENGTH+1];
    char name[ASSISTANT_STUB_BT_NAME_MAX_LENGTH+1];
    char bt_paired_name[ASSISTANT_STUB_BT_PAIRED_NAME_MAX_LENGTH+1];
}ASSISTANT_STUB_BLUETOOTH_T;

typedef struct
{
    ASSISTANT_STUB_SYSTEM_T        system;
    ASSISTANT_STUB_PLAYER_T        player;
    ASSISTANT_STUB_POWER_T         power;
    ASSISTANT_STUB_NETWORK_T       network;
    ASSISTANT_STUB_BLUETOOTH_T     bluetooth;
    int                            id;
}ASSISTANT_STUB_GET_SPEAKER_STATUS_RESPONSE_T;

typedef struct
{
    char command[ASSISTANT_STUB_COMMAND_MAX_LENGTH+1];
    int  id;
}ASSISTANT_STUB_GET_AP_LIST_T;

typedef struct
{
    char ssid[ASSISTANT_STUB_SSID_MAX_LENGTH+1];
    char bssid[ASSISTANT_STUB_BSSID_MAX_LENGTH+1];
    int  auth_mode;
    int  level;
    int  frequency;
}ASSISTANT_STUB_AP_INFO_T;

typedef struct
{
    char command[ASSISTANT_STUB_COMMAND_MAX_LENGTH+1];
    int  id;
    int  finish;
    int  list_num;
    ASSISTANT_STUB_AP_INFO_T ap_info[ASSISTANT_STUB_AP_LIST_MAX];
}ASSISTANT_STUB_GET_AP_LIST_RESPONSE_T;

typedef struct _ASSISTANT_STUB_WIFI_CONNECT_T
{
    char command[ASSISTANT_STUB_COMMAND_MAX_LENGTH+1];
    char ssid[ASSISTANT_STUB_SSID_MAX_LENGTH+1];
	char bssid[ASSISTANT_STUB_BSSID_MAX_LENGTH+1];
    char password[ASSISTANT_STUB_PASSWORD_MAX_LENGTH+1];
    int  auth_mode;
    int  id;
}ASSISTANT_STUB_WIFI_CONNECT_T;

typedef struct
{
    char command[ASSISTANT_STUB_COMMAND_MAX_LENGTH+1];
    int  wifi_status;
    int  id;
}ASSISTANT_STUB_WIFI_CONNECT_RESPONSE_T;

typedef struct _ASSISTANT_STUB_PLAY_DONE_T
{
    char command[ASSISTANT_STUB_COMMAND_MAX_LENGTH+1];
    char uri[ASSISTANT_STUB_URI_MAX_LENGTH+1];
    int  status;
    int  code;
}ASSISTANT_STUB_PLAY_DONE_T;

typedef struct _ASSISTANT_STUB_OTA_PROGRESS_T
{
    char command[ASSISTANT_STUB_COMMAND_MAX_LENGTH+1];
    int  progress;
}ASSISTANT_STUB_OTA_PROGRESS_T;


typedef struct _ASSISTANT_STUB_PLAY_TTS_DONE_T
{
    char command[ASSISTANT_STUB_COMMAND_MAX_LENGTH+1];
    int  tts_id;
    int  status;
}ASSISTANT_STUB_PLAY_TTS_DONE_T;

typedef struct _ASSISTANT_STUB_SYSTEM_STATUS_CHANGE_T
{
    char command[ASSISTANT_STUB_COMMAND_MAX_LENGTH+1];
    char status[ASSISTANT_STUB_STATUS_MAX_LENGTH+1];
}ASSISTANT_STUB_SYSTEM_STATUS_CHANGE_T;

typedef struct _ASSISTANT_STUB_PLAYER_STATUS_CHANGE_T
{
    char command[ASSISTANT_STUB_COMMAND_MAX_LENGTH+1];
    ASSISTANT_STUB_PLAYER_T player;

}ASSISTANT_STUB_PLAYER_STATUS_CHANGE_T;
typedef struct _ASSISTANT_STUB_BT_SRC_AVRCP_CMD_T
{
    char command[ASSISTANT_STUB_COMMAND_MAX_LENGTH+1];
    ASSISTANT_STUB_AVRCP_CMD_T cmd;

}ASSISTANT_STUB_BT_SRC_AVRCP_CMD_T;

typedef struct _ASSISTANT_STUB_SET_SYSTEM_STATUS_T
{
    char command[ASSISTANT_STUB_COMMAND_MAX_LENGTH+1];
    char status[ASSISTANT_STUB_STATUS_MAX_LENGTH+1];
}ASSISTANT_STUB_SET_SYSTEM_STATUS_T;

typedef struct _ASSISTANT_STUB_PLAY_PREV_AUDIO_T
{
    char command[ASSISTANT_STUB_COMMAND_MAX_LENGTH+1];
}ASSISTANT_STUB_PLAY_PREV_AUDIO_T;

typedef struct _ASSISTANT_STUB_PLAY_NEXT_AUDIO_T
{
    char command[ASSISTANT_STUB_COMMAND_MAX_LENGTH+1];
}ASSISTANT_STUB_PLAY_NEXT_AUDIO_T;

typedef struct _ASSISTANT_STUB_SET_BT_NAME_T
{
    char command[ASSISTANT_STUB_COMMAND_MAX_LENGTH+1];
    char name[ASSISTANT_STUB_BT_NAME_MAX_LENGTH+1];
}ASSISTANT_STUB_SET_BT_NAME_T;

typedef struct _ASSISTANT_STUB_START_BT_PAIR_T
{
    char command[ASSISTANT_STUB_COMMAND_MAX_LENGTH+1];
}ASSISTANT_STUB_START_BT_PAIR_T;

typedef struct _ASSISTANT_STUB_DEL_BT_PAIRED_T
{
    char command[ASSISTANT_STUB_COMMAND_MAX_LENGTH+1];
}ASSISTANT_STUB_DEL_BT_PAIRED_T;

typedef struct _ASSISTANT_STUB_BT_POWER_ON_T
{
    char command[ASSISTANT_STUB_COMMAND_MAX_LENGTH+1];
}ASSISTANT_STUB_BT_POWER_ON_T;

typedef struct _ASSISTANT_STUB_BT_POWER_OFF_T
{
    char command[ASSISTANT_STUB_COMMAND_MAX_LENGTH+1];
}ASSISTANT_STUB_BT_POWER_OFF_T;

typedef struct _ASSISTANT_STUB_PLAY_BT_MUSIC_T
{
    char command[ASSISTANT_STUB_COMMAND_MAX_LENGTH+1];
}ASSISTANT_STUB_PLAY_BT_MUSIC_T;

typedef struct _ASSISTANT_STUB_BT_DISCONNECT_T
{
    char command[ASSISTANT_STUB_COMMAND_MAX_LENGTH+1];
}ASSISTANT_STUB_BT_DISCONNECT_T;

typedef struct _ASSISTANT_STUB_SPEECH_START_T
{
    char command[ASSISTANT_STUB_COMMAND_MAX_LENGTH+1];
}ASSISTANT_STUB_SPEECH_START_T;

typedef struct _ASSISTANT_STUB_SPEECH_PROCESS_T
{
    char command[ASSISTANT_STUB_COMMAND_MAX_LENGTH+1];
}ASSISTANT_STUB_SPEECH_PROCESS_T;

typedef struct _ASSISTANT_STUB_SPEECH_FEEDBACK_T
{
    char command[ASSISTANT_STUB_COMMAND_MAX_LENGTH+1];
}ASSISTANT_STUB_SPEECH_FEEDBACK_T;

typedef struct _ASSISTANT_STUB_SPEECH_FINISH_T
{
    char command[ASSISTANT_STUB_COMMAND_MAX_LENGTH+1];
}ASSISTANT_STUB_SPEECH_FINISH_T;

typedef struct
{
    char command[ASSISTANT_STUB_COMMAND_MAX_LENGTH+1];
}ASSISTANT_STUB_WIFI_CONNECT_OVER_T;

typedef struct _ASSISTANT_STUB_WIFI_SETUP_RESULT_T
{
    char command[ASSISTANT_STUB_COMMAND_MAX_LENGTH+1];
    int  result;
}ASSISTANT_STUB_WIFI_SETUP_RESULT_T;

typedef struct _ASSISTANT_STUB_OTA_UPGRADE_T
{
    char command[ASSISTANT_STUB_COMMAND_MAX_LENGTH+1];
    char ota_url[ASSISTANT_STUB_URL_MAX_LENGTH+1];
}ASSISTANT_STUB_OTA_UPGRADE_T;

typedef struct _ASSISTANT_STUB_ADJUST_PROGRESS_T
{
    char command[ASSISTANT_STUB_COMMAND_MAX_LENGTH+1];
    int  progress;
}ASSISTANT_STUB_ADJUST_PROGRESS_T;

typedef struct _ASSISTANT_STUB_STOP_T
{
    char command[ASSISTANT_STUB_COMMAND_MAX_LENGTH+1];
}ASSISTANT_STUB_STOP_T;

typedef struct _ASSISTANT_STUB_FACTORY_RESET_RESULT_T
{
    char command[ASSISTANT_STUB_COMMAND_MAX_LENGTH+1];
    int  result;
}ASSISTANT_STUB_FACTORY_RESET_RESULT_T;

typedef struct
{
    char command[ASSISTANT_STUB_COMMAND_MAX_LENGTH+1];
    int  result;
}ASSISTANT_STUB_HFP_FREE_MIC_RESULT_T;

typedef struct _ASSISTANT_STUB_NETWORK_STATUS_CHANGE_T
{
    char command[ASSISTANT_STUB_COMMAND_MAX_LENGTH+1];
    int  quantity;
    char status[ASSISTANT_STUB_STATUS_MAX_LENGTH+1];
    char ssid[ASSISTANT_STUB_SSID_MAX_LENGTH+1];
    char bssid[ASSISTANT_STUB_BSSID_MAX_LENGTH+1];
}ASSISTANT_STUB_NETWORK_STATUS_CHANGE_T;

typedef struct _ASSISTANT_STUB_BLUETOOTH_STATUS_CHANGE_T
{
    char command[ASSISTANT_STUB_COMMAND_MAX_LENGTH+1];
    char status[ASSISTANT_STUB_STATUS_MAX_LENGTH+1];
    char name[ASSISTANT_STUB_BT_NAME_MAX_LENGTH+1];
    char bt_paired_name[ASSISTANT_STUB_BT_PAIRED_NAME_MAX_LENGTH+1];
}ASSISTANT_STUB_BLUETOOTH_STATUS_CHANGE_T;

typedef struct _ASSISTANT_STUB_BUTTON_T
{
    char command[ASSISTANT_STUB_COMMAND_MAX_LENGTH+1];
    char name[ASSISTANT_STUB_BUTTON_NAME_MAX_LENGTH+1];
}ASSISTANT_STUB_BUTTON_T;

typedef struct
{
    char command[ASSISTANT_STUB_COMMAND_MAX_LENGTH+1];
    char status[ASSISTANT_STUB_STATUS_MAX_LENGTH+1];
}ASSISTANT_STUB_HFP_STATUS_CHANGE_T;

typedef enum _ASSISTANT_STUB_CMD_INDEX_E
{
    ASSISTANT_STUB_CMD_PLAY = 0,
    ASSISTANT_STUB_CMD_PLAY_VOICE_PROMPT,
    ASSISTANT_STUB_CMD_PLAY_TTS,
    ASSISTANT_STUB_CMD_PLAY_PREV_AUDIO,
    ASSISTANT_STUB_CMD_PLAY_NEXT_AUDIO,
    ASSISTANT_STUB_CMD_SET_VOLUME,  //5
    ASSISTANT_STUB_CMD_SET_SYSTEM_STATUS,
    ASSISTANT_STUB_CMD_SET_BT_NAME,
    ASSISTANT_STUB_CMD_START_BT_PAIR,
    ASSISTANT_STUB_CMD_DEL_BT_PAIRED,
    ASSISTANT_STUB_CMD_BT_POWER_ON,  //10
    ASSISTANT_STUB_CMD_BT_POWER_OFF,
    ASSISTANT_STUB_CMD_PLAY_BT_MUSIC,
    ASSISTANT_STUB_CMD_BT_DISCONNECT,
    ASSISTANT_STUB_CMD_GET_AP_LIST,
    ASSISTANT_STUB_CMD_WIFI_CONNECT,   //15
    ASSISTANT_STUB_CMD_WIFI_CONNECT_OVER,
    ASSISTANT_STUB_CMD_WIFI_SETUP_RESULT,
    ASSISTANT_STUB_CMD_SPEECH_START,
    ASSISTANT_STUB_CMD_SPEECH_PROCESS,
    ASSISTANT_STUB_CMD_SPEECH_FEEDBACK,  //20
    ASSISTANT_STUB_CMD_SPEECH_FINISH,
    ASSISTANT_STUB_CMD_GET_SPEAKER_STATUS,
    ASSISTANT_STUB_CMD_PAUSE,
    ASSISTANT_STUB_CMD_RESUME,
    ASSISTANT_STUB_CMD_OTA_UPGRADE,   //25
    ASSISTANT_STUB_CMD_ADJUST_PROGRESS,
    ASSISTANT_STUB_CMD_STOP,
    ASSISTANT_STUB_CMD_FACTORY_RESET_RESULT,
    ASSISTANT_STUB_CMD_HFP_FREE_MIC_RESULT,
    ASSISTANT_STUB_CMD_PLAY_DONE,  //30
    ASSISTANT_STUB_CMD_PLAY_TTS_DONE,
    ASSISTANT_STUB_CMD_SYSTEM_STATUS_CHANGE,
    ASSISTANT_STUB_CMD_PLAYER_STATUS_CHANGE,
    ASSISTANT_STUB_CMD_BT_SRC_AVRCP_CMD,
    ASSISTANT_STUB_CMD_NETWORK_STATUS_CHANGE,  //35
    ASSISTANT_STUB_CMD_BLUETOOTH_STATUS_CHANGE,
    ASSISTANT_STUB_CMD_BUTTON,
    ASSISTANT_STUB_CMD_OTA_PROGRESS,
    ASSISTANT_STUB_CMD_HFP_STATUS_CHANGE,
    ASSISTANT_STUB_CMD_MAX  //40
}ASSISTANT_STUB_CMD_INDEX_E;

typedef struct _ASSISTANT_STUB_CMD_MAP_T
{
    ASSISTANT_STUB_CMD_INDEX_E  index;
    char *                command;
}ASSISTANT_STUB_CMD_MAP_T;

extern VOID a_assistant_stub_register(AMB_REGISTER_INFO_T* pt_reg);
extern BOOL u_assistant_stub_is_tts_stream_finish(VOID);

#endif /* _U_ASSISTANT_STUB_H_ */
