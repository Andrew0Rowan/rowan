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


#ifndef __ASSISTANT_CMD_H__
#define __ASSISTANT_CMD_H__

#ifdef __cplusplus
extern "C"
{
#endif

#include "AssistantDef.h"

#define HUB_CMD_LENGTH_MAX       (4096)

#define ASSISTANT_CMD_COMMAND_MAX_LENGTH       (32)
#define ASSISTANT_CMD_TYPE_LENGTH              (16)
#define ASSISTANT_CMD_ALGORITHM_LENGTH         (16)
#define ASSISTANT_CMD_DATA_MAX_SIZE            (512)
#define ASSISTANT_CMD_URI_MAX_LENGTH           (256)
#define ASSISTANT_CMD_URL_MAX_LENGTH           (256)
#define ASSISTANT_CMD_TITLE_MAX_LENGTH         (128)
#define ASSISTANT_CMD_ARTIST_MAX_LENGTH        (32)
#define ASSISTANT_CMD_SOURCE_MAX_LENGTH        (16)
#define ASSISTANT_CMD_SSID_MAX_LENGTH          (32)
#define ASSISTANT_CMD_BSSID_MAX_LENGTH         (30)
#define ASSISTANT_CMD_PASSWORD_MAX_LENGTH      (64)
#define ASSISTANT_CMD_ACTION_MAX_LENGTH        (16)
#define ASSISTANT_CMD_STATUS_MAX_LENGTH        (16)
#define ASSISTANT_CMD_AUDIO_TYPE_MAX_LENGTH    (16)
#define ASSISTANT_CMD_AUDIO_NAME_MAX_LENGTH    (256)
#define ASSISTANT_CMD_AUDIO_ANCHOR_MAX_LENGTH  (256)
#define ASSISTANT_CMD_AUDIO_SOURCE_MAX_LENGTH  (32)
#define ASSISTANT_CMD_AUDIO_ALBUM_MAX_LENGTH   (256)
#define ASSISTANT_CMD_AUDIO_ID_MAX_LENGTH      (48)
#define ASSISTANT_CMD_AUDIO_UID_MAX_LENGTH     (64)
#define ASSISTANT_CMD_AUDIO_EXT_MAX_LENGTH     (512)
#define ASSISTANT_CMD_BT_NAME_MAX_LENGTH       (96)
#define ASSISTANT_CMD_BT_PAIRED_NAME_MAX_LENGTH (96)
#define ASSISTANT_CMD_CRONTAB_MAX_LENGTH       (96)
#define ASSISTANT_CMD_AUDIOURL_MAX_LENGTH      (256)
#define ASSISTANT_CMD_WIFI_MAC_MAX_LENGTH      (32)
#define ASSISTANT_CMD_BT_MAC_MAX_LENGTH        (32)
#define ASSISTANT_CMD_DEVICE_SN_MAX_LENGTH     (32)
#define ASSISTANT_CMD_BUTTON_NAME_MAX_LENGTH   (32)
#define ASSISTANT_CMD_AP_LIST_MAX              (10)
#define ASSISTANT_CMD_ANCHOR_PIC_MAX_LENGTH    (256)
#define ASSISTANT_CMD_IMG_MAX_LENGTH           (256)
#define ASSISTANT_CMD_LRCURL_MAX_LENGTH        (256)

//add by sliver
#define ASSISTANT_CMD_OUTPUT_MAX_LENGTH 	   (512)


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
#define STRING_BT_POWER_OFF             "/bluetooth/power_off"
#define STRING_PLAY_BT_MUSIC            "/bluetooth/play_bt_music"
#define STRING_BT_DISCONNECT            "/bluetooth/bt_disconnect"
#define STRING_BT_SRC_AVRCP_CMD         "/bluetooth/avrcp_src_cmd"
#define STRING_SPEECH_START             "/speech/start"
#define STRING_SPEECH_PROCESS           "/speech/process"
#define STRING_SPEECH_FEEDBACK          "/speech/feedback"
#define STRING_SPEECH_FINISH            "/speech/finish"
#define STRING_WIFI_SETUP_RESULT        "/wifi/wifi_setup_result"
#define STRING_OTA_UPGRADE              "/system/ota_upgrade"
#define STRING_FACTORY_RESET_RESULT     "/system/factory_reset_result"
#define STRING_HFP_FREE_MIC_RESULT      "/system/hfp_free_mic_result"
#define STRING_GET_SPEAKER_STATUS       "/system/get_speaker_status"
#define STRING_GET_AP_LIST              "/wifi/get_ap_list"
#define STRING_WIFI_CONNECT             "/wifi/wifi_connect"
#define STRING_WIFI_CONNECT_OVER        "/wifi/wifi_connect_over"
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

#define STRING_COMMAND           "command"
#define STRING_PARAMS            "params"
#define STRING_TYPE              "type"
#define STRING_DATA              "data"
#define STRING_ALGORITHM         "algorithm"
#define STRING_STATUS            "status"
#define STRING_VOLUME            "volume"
#define STRING_SOURCE            "source"
#define STRING_AUDIO_TYPE        "audio_type"
#define STRING_AUDIO_NAME        "audioName"
#define STRING_AUDIO_ID          "audioId"
#define STRING_AUDIO_UID         "audioUid"
#define STRING_AUDIO_ANCHOR      "audioAnchor"
#define STRING_AUDIO_ALBUM       "audioAlbum"
#define STRING_AUDIO_SOURCE      "audioSource"
#define STRING_PROGRESS          "progress"
#define STRING_AUDIO_EXT         "audioExt"
#define STRING_URI               "uri"
#define STRING_ERROR             "error"
#define STRING_CODE              "code"
#define STRING_RESULT            "result"
#define STRING_WIFI_STATUS       "wifi_status"
#define STRING_ID                "id"
#define STRING_FINISH            "finish"
#define STRING_LIST              "list"
#define STRING_ON                "on"
#define STRING_EXPIRES_IN_MS     "expires_in_ms"
#define STRING_LED_ID            "led_id"
#define STRING_SET_LED           "set_led"
#define STRING_ACTION            "action"
#define STRING_TTS_ID            "tts_id"
#define STRING_SYSTEM            "system"
#define STRING_PLAYER            "player"
#define STRING_POWER             "power"
#define STRING_NETWORK           "network"
#define STRING_BLUETOOTH         "bluetooth"
#define STRING_QUANTITY          "quantity"
#define STRING_CRONTAB           "crontab"
#define STRING_AUDIOURL          "audioUrl"
#define STRING_WIFI_MAC          "wifi_mac"
#define STRING_BT_MAC            "bt_mac"
#define STRING_DEVICE_SN         "device_sn"
#define STRING_NAME              "name"
#define STRING_BT_PAIRED_NAME    "bt_paired_name"
#define STRING_TITLE             "title"
#define STRING_ARTIST            "artist"
#define STRING_FEEDBACK          "feedback"
#define STRING_SPEECH_ENABLE     "speech_enable"
#define STRING_NEED_MIX          "need_mix"
#define STRING_PCM               "pcm"
#define STRING_SAMPLERATE        "samplerate"
#define STRING_CHANNEL           "channels"
#define STRING_SSID              "ssid"
#define STRING_BSSID             "bssid"
#define STRING_PASSWORD          "password"
#define STRING_AUTH_MODE         "auth_mode"
#define STRING_OTA_URL           "ota_url"
#define STRING_LEVEL             "level"
#define STRING_FREQUENCY         "frequency"

typedef struct
{
    char command[ASSISTANT_CMD_COMMAND_MAX_LENGTH+1];
    char uri[ASSISTANT_CMD_URI_MAX_LENGTH+1];
    char audioId[ASSISTANT_CMD_AUDIO_ID_MAX_LENGTH+1];
    char audioUid[ASSISTANT_CMD_AUDIO_UID_MAX_LENGTH+1];
    char audioSource[ASSISTANT_CMD_AUDIO_SOURCE_MAX_LENGTH+1];
    char audioName[ASSISTANT_CMD_AUDIO_NAME_MAX_LENGTH+1];
    char audioAnchor[ASSISTANT_CMD_AUDIO_ANCHOR_MAX_LENGTH+1];
    char audioAlbum[ASSISTANT_CMD_AUDIO_ALBUM_MAX_LENGTH+1];
    int  progress;
    char audioExt[ASSISTANT_CMD_AUDIO_EXT_MAX_LENGTH+1];
    char anchorPic[ASSISTANT_CMD_ANCHOR_PIC_MAX_LENGTH+1];
    char img[ASSISTANT_CMD_IMG_MAX_LENGTH+1];
    char lrcurl[ASSISTANT_CMD_LRCURL_MAX_LENGTH+1];
} ASSISTANT_CMD_PLAY_T;

typedef struct
{
    int msgType;
    int requestId;
    int needMix;
    int dataLen;
    char buffer[2048];
}BIN_MSG;

typedef struct
{
    char command[ASSISTANT_CMD_COMMAND_MAX_LENGTH+1];
    char uri[ASSISTANT_CMD_URI_MAX_LENGTH+1];
    int  volume;
    char type[ASSISTANT_CMD_TYPE_LENGTH+1];
    BOOL feedback;
} ASSISTANT_CMD_PLAY_VOICE_PROMPT_T;

typedef struct
{
    char command[ASSISTANT_CMD_COMMAND_MAX_LENGTH+1];
    int  tts_id;
    BOOL speech_enable;
    BOOL need_mix;
    BOOL pcm;
    int mSampleRate;
    int mChannels;
} ASSISTANT_CMD_PLAY_TTS_T;

typedef struct
{
    char command[ASSISTANT_CMD_COMMAND_MAX_LENGTH+1];
} ASSISTANT_CMD_PAUSE_T;

typedef struct
{
    char command[ASSISTANT_CMD_COMMAND_MAX_LENGTH+1];
} ASSISTANT_CMD_RESUME_T;

typedef struct
{
    char command[ASSISTANT_CMD_COMMAND_MAX_LENGTH+1];
    int  volume;
} ASSISTANT_CMD_SET_VOLUME_T;

typedef struct
{
    char command[ASSISTANT_CMD_COMMAND_MAX_LENGTH+1];
    int  id;
} ASSISTANT_CMD_GET_SPEAKER_STATUS_T;

typedef struct
{
    char status[ASSISTANT_CMD_STATUS_MAX_LENGTH+1];
    char wifi_mac[ASSISTANT_CMD_WIFI_MAC_MAX_LENGTH+1];
    char bt_mac[ASSISTANT_CMD_BT_MAC_MAX_LENGTH+1];
    char device_sn[ASSISTANT_CMD_DEVICE_SN_MAX_LENGTH+1];
} ASSISTANT_CMD_SYSTEM_T;

typedef struct
{
    int  volume;
    char status[ASSISTANT_CMD_STATUS_MAX_LENGTH+1];
    char source[ASSISTANT_CMD_SOURCE_MAX_LENGTH+1];
    char audioId[ASSISTANT_CMD_AUDIO_ID_MAX_LENGTH+1];
    char audioUid[ASSISTANT_CMD_AUDIO_UID_MAX_LENGTH+1];
    char audioSource[ASSISTANT_CMD_AUDIO_SOURCE_MAX_LENGTH+1];
    char audioName[ASSISTANT_CMD_AUDIO_NAME_MAX_LENGTH+1];
    char audioAnchor[ASSISTANT_CMD_AUDIO_ANCHOR_MAX_LENGTH+1];
    char audioAlbum[ASSISTANT_CMD_AUDIO_ALBUM_MAX_LENGTH+1];
    int  progress;
    char audioExt[ASSISTANT_CMD_AUDIO_EXT_MAX_LENGTH+1];
} ASSISTANT_CMD_PLAYER_T;

typedef struct
{
    int  volume;
    char request[ASSISTANT_CMD_STATUS_MAX_LENGTH+1];
} ASSISTANT_CMD_AVRCP_T;


typedef struct
{
    int  quantity;
    char status[ASSISTANT_CMD_STATUS_MAX_LENGTH+1];
} ASSISTANT_CMD_POWER_T;

typedef struct
{
    int  quantity;
    char status[ASSISTANT_CMD_STATUS_MAX_LENGTH+1];
    char ssid[ASSISTANT_CMD_SSID_MAX_LENGTH+1];
    char bssid[ASSISTANT_CMD_BSSID_MAX_LENGTH+1];
} ASSISTANT_CMD_NETWORK_T;

typedef struct
{
    char status[ASSISTANT_CMD_STATUS_MAX_LENGTH+1];
    char name[ASSISTANT_CMD_BT_NAME_MAX_LENGTH+1];
    char bt_paired_name[ASSISTANT_CMD_BT_PAIRED_NAME_MAX_LENGTH+1];
} ASSISTANT_CMD_BLUETOOTH_T;

/*need to add detail infor*/
typedef struct
{
    char command[ASSISTANT_CMD_COMMAND_MAX_LENGTH+1];
    ASSISTANT_CMD_SYSTEM_T        system;
    ASSISTANT_CMD_PLAYER_T        player;
    ASSISTANT_CMD_POWER_T         power;
    ASSISTANT_CMD_NETWORK_T       network;
    ASSISTANT_CMD_BLUETOOTH_T     bluetooth;
    int                           id;
} ASSISTANT_CMD_GET_SPEAKER_STATUS_RESPONSE_T;

typedef struct
{
    char command[ASSISTANT_CMD_COMMAND_MAX_LENGTH+1];
    int  id;
} ASSISTANT_CMD_GET_AP_LIST_T;

typedef struct
{
    char ssid[ASSISTANT_CMD_SSID_MAX_LENGTH+1];
    char bssid[ASSISTANT_CMD_BSSID_MAX_LENGTH+1];
    int  auth_mode;
    int  level;
    int  frequency;
} ASSISTANT_CMD_AP_INFO_T;

typedef struct
{
    char command[ASSISTANT_CMD_COMMAND_MAX_LENGTH+1];
    int  id;
    int  finish;
    int  list_num;
    ASSISTANT_CMD_AP_INFO_T ap_info[ASSISTANT_CMD_AP_LIST_MAX];
} ASSISTANT_CMD_GET_AP_LIST_RESPONSE_T;

typedef struct
{
    char command[ASSISTANT_CMD_COMMAND_MAX_LENGTH+1];
    char ssid[ASSISTANT_CMD_SSID_MAX_LENGTH+1];
	char bssid[ASSISTANT_CMD_BSSID_MAX_LENGTH+1];
    char password[ASSISTANT_CMD_PASSWORD_MAX_LENGTH+1];
    int  auth_mode;
    int  id;
} ASSISTANT_CMD_WIFI_CONNECT_T;

typedef struct
{
    char command[ASSISTANT_CMD_COMMAND_MAX_LENGTH+1];
    int wifi_status;
    int id;
} ASSISTANT_CMD_WIFI_CONNECT_RESPONSE_T;

typedef struct
{
    char command[ASSISTANT_CMD_COMMAND_MAX_LENGTH+1];
    char uri[ASSISTANT_CMD_URI_MAX_LENGTH+1];
    int  status;
    int  code;
} ASSISTANT_CMD_PLAY_DONE_T;

typedef struct
{
    char command[ASSISTANT_CMD_COMMAND_MAX_LENGTH+1];
    int  tts_id;
    int  status;
} ASSISTANT_CMD_PLAY_TTS_DONE_T;

typedef struct
{
    char command[ASSISTANT_CMD_COMMAND_MAX_LENGTH+1];
    char status[ASSISTANT_CMD_STATUS_MAX_LENGTH+1];
} ASSISTANT_CMD_SYSTEM_STATUS_CHANGE_T;

typedef struct
{
    char command[ASSISTANT_CMD_COMMAND_MAX_LENGTH+1];
    ASSISTANT_CMD_PLAYER_T player;

} ASSISTANT_CMD_PLAYER_STATUS_CHANGE_T;

typedef struct
{
    char command[ASSISTANT_CMD_COMMAND_MAX_LENGTH+1];
    ASSISTANT_CMD_AVRCP_T cmd;

}ASSISTANT_CMD_BT_SRC_AVRCP_CMD_T;


typedef struct
{
    char command[ASSISTANT_CMD_COMMAND_MAX_LENGTH+1];
    char status[ASSISTANT_CMD_STATUS_MAX_LENGTH+1];
} ASSISTANT_CMD_SET_SYSTEM_STATUS_T;

typedef struct
{
    char command[ASSISTANT_CMD_COMMAND_MAX_LENGTH+1];
} ASSISTANT_CMD_PLAY_PREV_AUDIO_T;

typedef struct
{
    char command[ASSISTANT_CMD_COMMAND_MAX_LENGTH+1];
} ASSISTANT_CMD_PLAY_NEXT_AUDIO_T;

typedef struct
{
    char command[ASSISTANT_CMD_COMMAND_MAX_LENGTH+1];
    char name[ASSISTANT_CMD_BT_NAME_MAX_LENGTH+1];
} ASSISTANT_CMD_SET_BT_NAME_T;

typedef struct
{
    char command[ASSISTANT_CMD_COMMAND_MAX_LENGTH+1];
} ASSISTANT_CMD_START_BT_PAIR_T;

typedef struct
{
    char command[ASSISTANT_CMD_COMMAND_MAX_LENGTH+1];
} ASSISTANT_CMD_DEL_BT_PAIRED_T;

typedef struct
{
    char command[ASSISTANT_CMD_COMMAND_MAX_LENGTH+1];
} ASSISTANT_CMD_BT_POWER_OFF_T;

typedef struct
{
    char command[ASSISTANT_CMD_COMMAND_MAX_LENGTH+1];
} ASSISTANT_CMD_PLAY_BT_MUSIC_T;

typedef struct
{
    char command[ASSISTANT_CMD_COMMAND_MAX_LENGTH+1];
} ASSISTANT_CMD_BT_DISCONNECT_T;

typedef struct
{
    char command[ASSISTANT_CMD_COMMAND_MAX_LENGTH+1];
} ASSISTANT_CMD_SPEECH_START_T;

typedef struct
{
    char command[ASSISTANT_CMD_COMMAND_MAX_LENGTH+1];
} ASSISTANT_CMD_SPEECH_PROCESS_T;

typedef struct
{
    char command[ASSISTANT_CMD_COMMAND_MAX_LENGTH+1];
} ASSISTANT_CMD_SPEECH_FEEDBACK_T;

typedef struct
{
    char command[ASSISTANT_CMD_COMMAND_MAX_LENGTH+1];
} ASSISTANT_CMD_SPEECH_FINISH_T;

typedef struct
{
    char command[ASSISTANT_CMD_COMMAND_MAX_LENGTH+1];
} ASSISTANT_CMD_WIFI_CONNECT_OVER_T;

typedef struct
{
    char command[ASSISTANT_CMD_COMMAND_MAX_LENGTH+1];
    int  result;
} ASSISTANT_CMD_WIFI_SETUP_RESULT_T;

typedef struct
{
    char command[ASSISTANT_CMD_COMMAND_MAX_LENGTH+1];
    int  result;
} ASSISTANT_CMD_SWITCH_MIC_RESULT_T;

typedef struct
{
    char command[ASSISTANT_CMD_COMMAND_MAX_LENGTH+1];
    char ota_url[ASSISTANT_CMD_URL_MAX_LENGTH+1];
} ASSISTANT_CMD_OTA_UPGRADE_T;

typedef struct
{
    char command[ASSISTANT_CMD_COMMAND_MAX_LENGTH+1];
    int  progress;
} ASSISTANT_CMD_ADJUST_PROGRESS_T;

typedef struct
{
    char command[ASSISTANT_CMD_COMMAND_MAX_LENGTH+1];
} ASSISTANT_CMD_STOP_T;

typedef struct
{
    char command[ASSISTANT_CMD_COMMAND_MAX_LENGTH+1];
    int  result;
} ASSISTANT_CMD_FACTORY_RESET_RESULT_T;

typedef struct
{
    char command[ASSISTANT_CMD_COMMAND_MAX_LENGTH+1];
    int  result;
} ASSISTANT_CMD_HFP_FREE_MIC_RESULT_T;

typedef struct
{
    char command[ASSISTANT_CMD_COMMAND_MAX_LENGTH+1];
    int  quantity;
    char status[ASSISTANT_CMD_STATUS_MAX_LENGTH+1];
    char ssid[ASSISTANT_CMD_SSID_MAX_LENGTH+1];
    char bssid[ASSISTANT_CMD_BSSID_MAX_LENGTH+1];
} ASSISTANT_CMD_NETWORK_STATUS_CHANGE_T;

typedef struct
{
    char command[ASSISTANT_CMD_COMMAND_MAX_LENGTH+1];
    char status[ASSISTANT_CMD_STATUS_MAX_LENGTH+1];
    char name[ASSISTANT_CMD_BT_NAME_MAX_LENGTH+1];
    char bt_paired_name[ASSISTANT_CMD_BT_PAIRED_NAME_MAX_LENGTH+1];
} ASSISTANT_CMD_BLUETOOTH_STATUS_CHANGE_T;

typedef struct
{
    char command[ASSISTANT_CMD_COMMAND_MAX_LENGTH+1];
    char name[ASSISTANT_CMD_BUTTON_NAME_MAX_LENGTH+1];
} ASSISTANT_CMD_BUTTON_T;

typedef struct
{
    char command[ASSISTANT_CMD_COMMAND_MAX_LENGTH+1];
    char status[ASSISTANT_CMD_STATUS_MAX_LENGTH+1];
} ASSISTANT_CMD_HFP_STATUS_CHANGE_T;

typedef enum
{
    //Assistant Center send to appmainprog
    ASSISTANT_CMD_PLAY = 0,
    ASSISTANT_CMD_PLAY_VOICE_PROMPT,
    ASSISTANT_CMD_PLAY_TTS,
    ASSISTANT_CMD_PLAY_PREV_AUDIO,
    ASSISTANT_CMD_PLAY_NEXT_AUDIO,
    ASSISTANT_CMD_SET_VOLUME = 5,
    ASSISTANT_CMD_SET_SYSTEM_STATUS,
    ASSISTANT_CMD_SET_BT_NAME,
    ASSISTANT_CMD_START_BT_PAIR,
    ASSISTANT_CMD_DEL_BT_PAIRED,
	ASSISTANT_CMD_BT_POWER_ON,
    ASSISTANT_CMD_BT_POWER_OFF,
    ASSISTANT_CMD_PLAY_BT_MUSIC,
    ASSISTANT_CMD_BT_DISCONNECT,
    ASSISTANT_CMD_GET_AP_LIST,
    ASSISTANT_CMD_WIFI_CONNECT,
    ASSISTANT_CMD_WIFI_CONNECT_OVER,
    ASSISTANT_CMD_WIFI_SETUP_RESULT,
    ASSISTANT_CMD_SPEECH_START,
    ASSISTANT_CMD_SPEECH_PROCESS,
    ASSISTANT_CMD_SPEECH_FEEDBACK,
    ASSISTANT_CMD_SPEECH_FINISH,
    ASSISTANT_CMD_GET_SPEAKER_STATUS,
    ASSISTANT_CMD_PAUSE,
    ASSISTANT_CMD_RESUME,
    ASSISTANT_CMD_OTA_UPGRADE,
    ASSISTANT_CMD_ADJUST_PROGRESS,
    ASSISTANT_CMD_STOP,
    ASSISTANT_CMD_FACTORY_RESET_RESULT,
    ASSISTANT_CMD_HFP_FREE_MIC_RESULT,

    //appmainprog send to Assistant Center
    ASSISTANT_CMD_PLAY_DONE,
    ASSISTANT_CMD_PLAY_TTS_DONE,
    ASSISTANT_CMD_SYSTEM_STATUS_CHANGE,
    ASSISTANT_CMD_PLAYER_STATUS_CHANGE,
    ASSISTANT_CMD_NETWORK_STATUS_CHANGE,
    ASSISTANT_CMD_BLUETOOTH_STATUS_CHANGE,
    ASSISTANT_CMD_BUTTON,
    ASSISTANT_CMD_OTA_PROGRESS,
    ASSISTANT_CMD_HFP_STATUS_CHANGE,
    ASSISTANT_CMD_BT_SRC_AVRCP_CMD,
    ASSISTANT_CMD_MAX
} ASSISTANT_CMD_E;

#ifdef __cplusplus
}
#endif

#endif
