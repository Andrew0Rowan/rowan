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


/*-----------------------------------------------------------------------------
 * $RCSfile: assistant_stub_app_json_recv.c $
 * $Revision:0.1
 * $Date: 2017/01/06
 * $Author:pingan.liu
 * $CCRevision: $
 * $SWAuthor:  $
 * $MD5HEX: $
 *
 * Description:assistance stub app json recv thread


 *---------------------------------------------------------------------------*/
/*-----------------------------------------------------------------------------
                    include files
-----------------------------------------------------------------------------*/
#include "assistant_stub_app_json_recv.h"

static char * hub_app_filename = "/tmp/hub_send.sock";
static char * hub_host_filename = "/tmp/hub_recv.sock";

int speaker_id=0;

struct hub_ctx CTX = {0};

extern ASSISTANT_STUB_CMD_MAP_T assistant_map[];
extern BOOL need_respose_wifi_connect_rpc;

int hub_init(struct hub_ctx * ctx, int type)
{
    FUNCTION_BEGIN
    /* start server side*/
    struct sockaddr_un * server_addr = &ctx->server_addr;
    memset(server_addr, 0, sizeof(struct sockaddr_un));
    server_addr->sun_family = AF_UNIX;
    strcpy(server_addr->sun_path,(HUB_APP == type) ? hub_app_filename : hub_host_filename);
    strcpy(ctx->filename, server_addr->sun_path);
    unlink(ctx->filename);

    /*create server socket*/
    ctx->server_sockfd = socket(AF_UNIX, SOCK_DGRAM, 0);
    if (-1 == ctx->server_sockfd)
    {
        perror("<ASSISTANT_STUB_APP> Fail to create server socket.\n");
        perror(strerror(errno));
        return -1;
    }

    /*bind server socket*/
    int res = bind(ctx->server_sockfd,(struct sockaddr *)server_addr,sizeof(struct sockaddr_un));
    if (res)
    {
        perror("<ASSISTANT_STUB_APP> Fail to start hub server.\n");
        perror(strerror(errno));
        return res;
    }

    /*initialize client side*/
    struct sockaddr_un *client_addr = &ctx->client_addr;
    client_addr->sun_family = AF_UNIX;
    strcpy(client_addr->sun_path,(HUB_APP == type) ? hub_host_filename : hub_app_filename);

    /*create client socket*/
    ctx->client_sockfd = socket(AF_UNIX, SOCK_DGRAM, 0);
    if (-1 == ctx->client_sockfd)
    {
        perror("<ASSISTANT_STUB_APP> Fail to create client socket.\n");
        perror(strerror(errno));
        return -1;
    }
    FUNCTION_END

    return 0;

}

void hub_destroy(struct hub_ctx * ctx)
{
    close(ctx->client_sockfd);
    close(ctx->server_sockfd);
    unlink(ctx->filename);
}

int hub_recv(struct hub_ctx * ctx, char * buffer, size_t len)
{
    return recv(ctx->server_sockfd, buffer, len, 0);
}

int hub_send(struct hub_ctx * ctx, char * buffer, size_t len)
{
    return sendto(ctx->client_sockfd,
                  buffer,
                  len,
                  0,
                  (struct sockaddr *)&ctx->client_addr,
                  sizeof(struct sockaddr_un));
}

void _assistant_stub_json_parse_uri(char *uri)
{
    char *into = uri;
    while (*uri)
    {
        if (*uri== '\\') /* if (*uri== 92)*/
        {
            uri++;
        }
        else
        {
            *into++=*uri++;         /* All other characters. */
        }
    }
    *into=0;    /* and null-terminate. */
}

static VOID _assistant_stub_json_handle_play(cJSON * assistant_cmd)
{
    FUNCTION_BEGIN
    INT32    i4_ret;
    HANDLE_T h_app = NULL_HANDLE;
    cJSON *  params=NULL;
    ASSISTANT_STUB_PLAY_T play={0};

    strncpy(play.command,
            cJSON_GetObjectItem(assistant_cmd,ASSISTANT_STUB_COMMAND)->valuestring,
            ASSISTANT_STUB_COMMAND_MAX_LENGTH);

    params = cJSON_GetObjectItem(assistant_cmd,ASSISTANT_STUB_PARAMS);
    strncpy(play.uri,
            cJSON_GetObjectItem(params,ASSISTANT_STUB_URI)->valuestring,
            ASSISTANT_STUB_URI_MAX_LENGTH);
    printf("<ASSISTANT_STUB_APP> play.uri 1 is %s\n",play.uri);
#if 0
    cJSON_Minify(play.uri);
#else
    _assistant_stub_json_parse_uri(play.uri);
#endif
    printf("<ASSISTANT_STUB_APP> play.uri 2 is %s\n",play.uri);

    strncpy(play.uri,
            cJSON_GetObjectItem(params,ASSISTANT_STUB_URI)->valuestring,
            ASSISTANT_STUB_URI_MAX_LENGTH);
    strncpy(play.audioId,
            cJSON_GetObjectItem(params,ASSISTANT_STUB_AUDIO_ID)->valuestring,
            ASSISTANT_STUB_AUDIO_ID_MAX_LENGTH);
    strncpy(play.audioUid,
            cJSON_GetObjectItem(params,ASSISTANT_STUB_AUDIO_UID)->valuestring,
            ASSISTANT_STUB_AUDIO_UID_MAX_LENGTH);
    strncpy(play.audioSource,
            cJSON_GetObjectItem(params,ASSISTANT_STUB_AUDIO_SOURCE)->valuestring,
            ASSISTANT_STUB_AUDIO_SOURCE_MAX_LENGTH);
    strncpy(play.audioName,
            cJSON_GetObjectItem(params,ASSISTANT_STUB_AUDIO_NAME)->valuestring,
            ASSISTANT_STUB_AUDIO_NAME_MAX_LENGTH);
    strncpy(play.audioAnchor,
            cJSON_GetObjectItem(params,ASSISTANT_STUB_AUDIO_ANCHOR)->valuestring,
            ASSISTANT_STUB_AUDIO_ANCHOR_MAX_LENGTH);
    strncpy(play.audioAlbum,
            cJSON_GetObjectItem(params,ASSISTANT_STUB_AUDIO_ALBUM)->valuestring,
            ASSISTANT_STUB_AUDIO_ALBUM_MAX_LENGTH);

    play.progress = cJSON_GetObjectItem(params,ASSISTANT_STUB_PROGRESS)->valueint;
    strncpy(play.audioExt,
            cJSON_GetObjectItem(params,ASSISTANT_STUB_AUDIO_EXT)->valuestring,
            ASSISTANT_STUB_AUDIO_EXT_MAX_LENGTH);

    i4_ret = u_am_get_app_handle_from_name(&h_app, SM_THREAD_NAME);
    if(ASSISTANT_STUB_APPR_OK != i4_ret)
    {
        printf("<ASSISTANT_STUB_APP> get handle fail!,i4_ret=%ld\n",i4_ret);
    }
    i4_ret = u_app_send_appmsg(h_app,
                               E_APP_MSG_TYPE_ASSISTANT_STUB,
                               MSG_FROM_ASSISTANT_STUB,
                               ASSISTANT_STUB_CMD_PLAY,
                               &play,
                               sizeof(ASSISTANT_STUB_PLAY_T));
    if(ASSISTANT_STUB_APPR_OK != i4_ret)
    {
        printf("<ASSISTANT_STUB_APP> send msg fail!\n");
    }
    else
    {
        printf("<ASSISTANT_STUB_APP>send msg suc line=%d!\n",__LINE__);
    }
    FUNCTION_END
}

static VOID _assistant_stub_json_handle_play_voice_prompt(cJSON * assistant_cmd)
{
    FUNCTION_BEGIN
    INT32    i4_ret;
    HANDLE_T h_app = NULL_HANDLE;
    cJSON *  params=NULL;
    ASSISTANT_STUB_PLAY_VOICE_PROMPT_T play_voice_prompt={0};

    strncpy(play_voice_prompt.command,
            cJSON_GetObjectItem(assistant_cmd,ASSISTANT_STUB_COMMAND)->valuestring,
            ASSISTANT_STUB_COMMAND_MAX_LENGTH);

    params = cJSON_GetObjectItem(assistant_cmd,ASSISTANT_STUB_PARAMS);
    strncpy(play_voice_prompt.uri,
            cJSON_GetObjectItem(params,ASSISTANT_STUB_URI)->valuestring,
            ASSISTANT_STUB_URI_MAX_LENGTH);

    printf("<ASSISTANT_STUB_APP> play_voice_prompt.uri 1 is %s\n",play_voice_prompt.uri);
#if 0
    cJSON_Minify(play_voice_prompt.uri);
#else
    _assistant_stub_json_parse_uri(play_voice_prompt.uri);
#endif
    printf("<ASSISTANT_STUB_APP> play_voice_prompt.uri 2 is %s\n",play_voice_prompt.uri);

    play_voice_prompt.volume= cJSON_GetObjectItem(params,ASSISTANT_STUB_VOLUME)->valueint;
    printf("<ASSISTANT_STUB_APP> volume is %d\n",play_voice_prompt.volume);

    strncpy(play_voice_prompt.type,
            cJSON_GetObjectItem(params,ASSISTANT_STUB_TYPE)->valuestring,
            ASSISTANT_STUB_TYPE_LENGTH);
    printf("<ASSISTANT_STUB_APP> type is %s\n",play_voice_prompt.type);

    play_voice_prompt.feedback = (BOOL)(cJSON_GetObjectItem(params,ASSISTANT_STUB_FEEDBACK)->type);

    i4_ret = u_am_get_app_handle_from_name(&h_app, SM_THREAD_NAME);
    if(ASSISTANT_STUB_APPR_OK != i4_ret)
    {
        printf("<ASSISTANT_STUB_APP> get handle fail!,i4_ret=%ld\n",i4_ret);
    }
    i4_ret = u_app_send_appmsg(h_app,
                               E_APP_MSG_TYPE_ASSISTANT_STUB,
                               MSG_FROM_ASSISTANT_STUB,
                               ASSISTANT_STUB_CMD_PLAY_VOICE_PROMPT,
                               &play_voice_prompt,
                               sizeof(ASSISTANT_STUB_PLAY_VOICE_PROMPT_T));
    if(ASSISTANT_STUB_APPR_OK != i4_ret)
    {
        printf("<ASSISTANT_STUB_APP> send msg fail!\n");
    }
    else
    {
        printf("<ASSISTANT_STUB_APP>send msg suc line=%d!\n",__LINE__);
    }
    FUNCTION_END
}

static VOID _assistant_stub_json_handle_play_tts(cJSON * assistant_cmd)
{
    FUNCTION_BEGIN
    INT32    i4_ret;
    HANDLE_T h_app = NULL_HANDLE;
    cJSON *  params=NULL;
    ASSISTANT_STUB_PLAY_TTS_T play_tts={0};

    strncpy(play_tts.command,
            cJSON_GetObjectItem(assistant_cmd,ASSISTANT_STUB_COMMAND)->valuestring,
            ASSISTANT_STUB_COMMAND_MAX_LENGTH);

    params = cJSON_GetObjectItem(assistant_cmd,ASSISTANT_STUB_PARAMS);
    play_tts.tts_id = cJSON_GetObjectItem(params,ASSISTANT_STUB_TTS_ID)->valueint;
    play_tts.speech_enable = (BOOL)(cJSON_GetObjectItem(params,ASSISTANT_STUB_SPEECH_ENABLE)->valueint);
    play_tts.need_mix = (BOOL)(cJSON_GetObjectItem(params,ASSISTANT_STUB_NEED_MIX)->valueint);
    play_tts.pcm = (BOOL)(cJSON_GetObjectItem(params,ASSISTANT_STUB_PCM)->valueint);    
    play_tts.mSampleRate = (cJSON_GetObjectItem(params,ASSISTANT_STUB_SAMPLERATE)->valueint);
    play_tts.mChannels= (cJSON_GetObjectItem(params,ASSISTANT_STUB_CHANNEL)->valueint);
    printf("<ASSISTANT_STUB_APP> get PCM!,PCMData=%d\n",play_tts.pcm);
    printf("<ASSISTANT_STUB_APP> get samplerate!,Samplerate=%d\n",play_tts.mSampleRate);
    printf("<ASSISTANT_STUB_APP> get mChannels!,mChannels=%d\n",play_tts.mChannels);

    i4_ret = u_am_get_app_handle_from_name(&h_app, SM_THREAD_NAME);
    if(ASSISTANT_STUB_APPR_OK != i4_ret)
    {
        printf("<ASSISTANT_STUB_APP> get handle fail!,i4_ret=%ld\n",i4_ret);
    }
    i4_ret = u_app_send_appmsg(h_app,
                               E_APP_MSG_TYPE_ASSISTANT_STUB,
                               MSG_FROM_ASSISTANT_STUB,
                               ASSISTANT_STUB_CMD_PLAY_TTS,
                               &play_tts,
                               sizeof(ASSISTANT_STUB_PLAY_TTS_T));
    if(ASSISTANT_STUB_APPR_OK != i4_ret)
    {
        printf("<ASSISTANT_STUB_APP> send msg fail!\n");
    }
    else
    {
        printf("<ASSISTANT_STUB_APP>send msg suc line=%d!\n",__LINE__);
    }
    FUNCTION_END
}

static VOID _assistant_stub_json_handle_play_prev_audio(cJSON * assistant_cmd)
{
    FUNCTION_BEGIN
    INT32    i4_ret;
    HANDLE_T h_app = NULL_HANDLE;

    ASSISTANT_STUB_PLAY_PREV_AUDIO_T play_prev_audio={0};

    strncpy(play_prev_audio.command,
            cJSON_GetObjectItem(assistant_cmd,ASSISTANT_STUB_COMMAND)->valuestring,
            ASSISTANT_STUB_COMMAND_MAX_LENGTH);

    i4_ret = u_am_get_app_handle_from_name(&h_app, SM_THREAD_NAME);
    if(ASSISTANT_STUB_APPR_OK != i4_ret)
    {
        printf("<ASSISTANT_STUB_APP> get handle fail!,i4_ret=%ld\n",i4_ret);
    }
    i4_ret = u_app_send_appmsg(h_app,
                               E_APP_MSG_TYPE_ASSISTANT_STUB,
                               MSG_FROM_ASSISTANT_STUB,
                               ASSISTANT_STUB_CMD_PLAY_PREV_AUDIO,
                               &play_prev_audio,
                               sizeof(ASSISTANT_STUB_PLAY_PREV_AUDIO_T));
    if(ASSISTANT_STUB_APPR_OK != i4_ret)
    {
        printf("<ASSISTANT_STUB_APP> send msg fail!\n");
    }
    else
    {
        printf("<ASSISTANT_STUB_APP>send msg suc line=%d!\n",__LINE__);
    }
    FUNCTION_END
}

static VOID _assistant_stub_json_handle_play_next_audio(cJSON * assistant_cmd)
{
    FUNCTION_BEGIN
    INT32    i4_ret;
    HANDLE_T h_app = NULL_HANDLE;

    ASSISTANT_STUB_PLAY_NEXT_AUDIO_T play_next_audio={0};

    strncpy(play_next_audio.command,
            cJSON_GetObjectItem(assistant_cmd,ASSISTANT_STUB_COMMAND)->valuestring,
            ASSISTANT_STUB_COMMAND_MAX_LENGTH);

    i4_ret = u_am_get_app_handle_from_name(&h_app, SM_THREAD_NAME);
    if(ASSISTANT_STUB_APPR_OK != i4_ret)
    {
        printf("<ASSISTANT_STUB_APP> get handle fail!,i4_ret=%ld\n",i4_ret);
    }
    i4_ret = u_app_send_appmsg(h_app,
                               E_APP_MSG_TYPE_ASSISTANT_STUB,
                               MSG_FROM_ASSISTANT_STUB,
                               ASSISTANT_STUB_CMD_PLAY_NEXT_AUDIO,
                               &play_next_audio,
                               sizeof(ASSISTANT_STUB_PLAY_NEXT_AUDIO_T));
    if(ASSISTANT_STUB_APPR_OK != i4_ret)
    {
        printf("<ASSISTANT_STUB_APP> send msg fail!\n");
    }
    else
    {
        printf("<ASSISTANT_STUB_APP>send msg suc line=%d!\n",__LINE__);
    }
    FUNCTION_END
}

static VOID _assistant_stub_json_handle_play_bt_music(cJSON * assistant_cmd)
{
    FUNCTION_BEGIN
    INT32    i4_ret;
    HANDLE_T h_app = NULL_HANDLE;

    ASSISTANT_STUB_PLAY_BT_MUSIC_T play_bt_music={0};

    strncpy(play_bt_music.command,
            cJSON_GetObjectItem(assistant_cmd,ASSISTANT_STUB_COMMAND)->valuestring,
            ASSISTANT_STUB_COMMAND_MAX_LENGTH);

    i4_ret = u_am_get_app_handle_from_name(&h_app, SM_THREAD_NAME);
    if(ASSISTANT_STUB_APPR_OK != i4_ret)
    {
        printf("<ASSISTANT_STUB_APP> get handle fail!,i4_ret=%ld\n",i4_ret);
    }
    i4_ret = u_app_send_appmsg(h_app,
                               E_APP_MSG_TYPE_ASSISTANT_STUB,
                               MSG_FROM_ASSISTANT_STUB,
                               ASSISTANT_STUB_CMD_PLAY_BT_MUSIC,
                               &play_bt_music,
                               sizeof(ASSISTANT_STUB_PLAY_BT_MUSIC_T));
    if(ASSISTANT_STUB_APPR_OK != i4_ret)
    {
        printf("<ASSISTANT_STUB_APP> send msg fail!\n");
    }
    else
    {
        printf("<ASSISTANT_STUB_APP>send msg suc line=%d!\n",__LINE__);
    }
    FUNCTION_END
}

static VOID _assistant_stub_json_handle_set_volume(cJSON * assistant_cmd)
{
    FUNCTION_BEGIN
    INT32    i4_ret;
    HANDLE_T h_app = NULL_HANDLE;
    cJSON *  params=NULL;
    ASSISTANT_STUB_SET_VOLUME_T set_volume={0};

    strncpy(set_volume.command,
            cJSON_GetObjectItem(assistant_cmd,ASSISTANT_STUB_COMMAND)->valuestring,
            ASSISTANT_STUB_COMMAND_MAX_LENGTH);

    params = cJSON_GetObjectItem(assistant_cmd,ASSISTANT_STUB_PARAMS);
    set_volume.volume= cJSON_GetObjectItem(params,ASSISTANT_STUB_VOLUME)->valueint;

    i4_ret = u_am_get_app_handle_from_name(&h_app, SM_THREAD_NAME);
    if(ASSISTANT_STUB_APPR_OK != i4_ret)
    {
        printf("<ASSISTANT_STUB_APP> get handle fail!,i4_ret=%ld\n",i4_ret);
    }
    i4_ret = u_app_send_appmsg(h_app,
                               E_APP_MSG_TYPE_ASSISTANT_STUB,
                               MSG_FROM_ASSISTANT_STUB,
                               ASSISTANT_STUB_CMD_SET_VOLUME,
                               &set_volume,
                               sizeof(ASSISTANT_STUB_SET_VOLUME_T));
    if(ASSISTANT_STUB_APPR_OK != i4_ret)
    {
        printf("<ASSISTANT_STUB_APP> send msg fail!\n");
    }
    else
    {
        printf("<ASSISTANT_STUB_APP>send msg suc line=%d!\n",__LINE__);
    }
    FUNCTION_END
}

static VOID _assistant_stub_json_handle_set_system_status(cJSON * assistant_cmd)
{
    FUNCTION_BEGIN
    INT32    i4_ret;
    HANDLE_T h_app = NULL_HANDLE;
    cJSON *  params=NULL;
    ASSISTANT_STUB_SET_SYSTEM_STATUS_T set_system_status={0};

    strncpy(set_system_status.command,cJSON_GetObjectItem(assistant_cmd,ASSISTANT_STUB_COMMAND)->valuestring,ASSISTANT_STUB_COMMAND_MAX_LENGTH);
    params = cJSON_GetObjectItem(assistant_cmd,ASSISTANT_STUB_PARAMS);
    strncpy(set_system_status.status,cJSON_GetObjectItem(params,ASSISTANT_STUB_STATUS)->valuestring,ASSISTANT_STUB_STATUS_MAX_LENGTH);

    i4_ret = u_am_get_app_handle_from_name(&h_app, SM_THREAD_NAME);
    if(ASSISTANT_STUB_APPR_OK != i4_ret)
    {
        printf("<ASSISTANT_STUB_APP> get handle fail!,i4_ret=%ld\n",i4_ret);
    }
    i4_ret = u_app_send_appmsg(h_app,
                               E_APP_MSG_TYPE_ASSISTANT_STUB,
                               MSG_FROM_ASSISTANT_STUB,
                               ASSISTANT_STUB_CMD_SET_SYSTEM_STATUS,
                               &set_system_status,
                               sizeof(ASSISTANT_STUB_SET_SYSTEM_STATUS_T));
    if(ASSISTANT_STUB_APPR_OK != i4_ret)
    {
        printf("<ASSISTANT_STUB_APP> send msg fail!\n");
    }
    else
    {
        printf("<ASSISTANT_STUB_APP>send msg suc line=%d!\n",__LINE__);
    }
    FUNCTION_END
}

static VOID _assistant_stub_json_handle_set_bt_name(cJSON * assistant_cmd)
{
    FUNCTION_BEGIN
    INT32    i4_ret;
    HANDLE_T h_app = NULL_HANDLE;
    cJSON *  params=NULL;
    ASSISTANT_STUB_SET_BT_NAME_T set_bt_name={0};

    strncpy(set_bt_name.command,cJSON_GetObjectItem(assistant_cmd,ASSISTANT_STUB_COMMAND)->valuestring,ASSISTANT_STUB_COMMAND_MAX_LENGTH);
    params = cJSON_GetObjectItem(assistant_cmd,ASSISTANT_STUB_PARAMS);
    strncpy(set_bt_name.name,cJSON_GetObjectItem(params,ASSISTANT_STUB_NAME)->valuestring,ASSISTANT_STUB_BT_NAME_MAX_LENGTH);

    i4_ret = u_am_get_app_handle_from_name(&h_app, BLUETOOTH_THREAD_NAME);
    if(ASSISTANT_STUB_APPR_OK != i4_ret)
    {
        printf("<ASSISTANT_STUB_APP> get handle fail!,i4_ret=%ld\n",i4_ret);
    }
    i4_ret = u_app_send_appmsg(h_app,
                               E_APP_MSG_TYPE_ASSISTANT_STUB,
                               MSG_FROM_ASSISTANT_STUB,
                               ASSISTANT_STUB_CMD_SET_BT_NAME,
                               &set_bt_name,
                               sizeof(ASSISTANT_STUB_SET_BT_NAME_T));
    if(ASSISTANT_STUB_APPR_OK != i4_ret)
    {
        printf("<ASSISTANT_STUB_APP> send msg fail!\n");
    }
    else
    {
        printf("<ASSISTANT_STUB_APP>send msg suc line=%d!\n",__LINE__);
    }
    FUNCTION_END
}

static VOID _assistant_stub_json_handle_get_ap_list(cJSON * assistant_cmd)
{
    FUNCTION_BEGIN
    INT32    i4_ret;
    HANDLE_T h_app = NULL_HANDLE;
    ASSISTANT_STUB_GET_AP_LIST_T cmd = {0};

    strncpy(cmd.command,cJSON_GetObjectItem(assistant_cmd,ASSISTANT_STUB_COMMAND)->valuestring,ASSISTANT_STUB_COMMAND_MAX_LENGTH);
    cmd.id = cJSON_GetObjectItem(assistant_cmd, ASSISTANT_STUB_ID)->valueint;

    i4_ret = u_am_get_app_handle_from_name(&h_app, WIFI_SETTING_THREAD_NAME);
    if(i4_ret)
    {
        printf("<ASSISTANT_STUB_APP> get handle fail!,i4_ret=%ld\n",i4_ret);
    }

    i4_ret = u_app_send_appmsg(h_app,
                               E_APP_MSG_TYPE_ASSISTANT_STUB,
                               MSG_FROM_ASSISTANT_STUB,
                               ASSISTANT_STUB_CMD_GET_AP_LIST,
                               &cmd,
                               sizeof(cmd));
    if (i4_ret)
    {
        printf("<ASSISTANT_STUB_APP> send msg fail!\n");
    }
    else
    {
        printf("<ASSISTANT_STUB_APP>send msg suc line=%d!\n",__LINE__);
    }
    FUNCTION_END
}

static VOID _assistant_stub_json_handle_wifi_connect(cJSON * assistant_cmd)
{
    FUNCTION_BEGIN
    INT32    i4_ret;
    HANDLE_T h_app = NULL_HANDLE;
    cJSON *  params=NULL;
    ASSISTANT_STUB_WIFI_CONNECT_T wifi_connect={0};

    strncpy(wifi_connect.command,cJSON_GetObjectItem(assistant_cmd,ASSISTANT_STUB_COMMAND)->valuestring,ASSISTANT_STUB_COMMAND_MAX_LENGTH);
    printf("wifi_connect.command is %s\n",wifi_connect.command);
    wifi_connect.id = cJSON_GetObjectItem(assistant_cmd,ASSISTANT_STUB_ID)->valueint;
    printf("wifi_connect.id is %d\n",wifi_connect.id);

    params = cJSON_GetObjectItem(assistant_cmd,ASSISTANT_STUB_PARAMS);
    strncpy(wifi_connect.ssid,
            cJSON_GetObjectItem(params,ASSISTANT_STUB_SSID)->valuestring,
            ASSISTANT_STUB_SSID_MAX_LENGTH);
    printf("wifi_connect.ssid is %s\n",wifi_connect.ssid);

	strncpy(wifi_connect.bssid,
		cJSON_GetObjectItem(params,ASSISTANT_STUB_BSSID)->valuestring,
		ASSISTANT_STUB_BSSID_MAX_LENGTH);
	printf("wifi_connect.bssid is %s\n",wifi_connect.bssid);
	
    strncpy(wifi_connect.password,
            cJSON_GetObjectItem(params,ASSISTANT_STUB_PASSWORD)->valuestring,
            ASSISTANT_STUB_PASSWORD_MAX_LENGTH);
    printf("wifi_connect.password is %s\n",wifi_connect.password);

    wifi_connect.auth_mode = cJSON_GetObjectItem(params, ASSISTANT_STUB_AUTH_MODE)->valueint;
    printf("wifi_connect.auth_mode is %d\n", wifi_connect.auth_mode);

    need_respose_wifi_connect_rpc = TRUE;
    printf("<ASSISTANT_STUB_APP> need_respose_wifi_connect_rpc is %d\n",need_respose_wifi_connect_rpc);

    i4_ret = u_am_get_app_handle_from_name(&h_app, WIFI_SETTING_THREAD_NAME);
    if(ASSISTANT_STUB_APPR_OK != i4_ret)
    {
        printf("<ASSISTANT_STUB_APP> get handle fail!,i4_ret=%ld\n",i4_ret);
    }
    i4_ret = u_app_send_appmsg(h_app,
                               E_APP_MSG_TYPE_ASSISTANT_STUB,
                               MSG_FROM_ASSISTANT_STUB,
                               ASSISTANT_STUB_CMD_WIFI_CONNECT,
                               &wifi_connect,
                               sizeof(ASSISTANT_STUB_WIFI_CONNECT_T));
    if(ASSISTANT_STUB_APPR_OK != i4_ret)
    {
        printf("<ASSISTANT_STUB_APP> send msg fail!\n");
    }
    else
    {
        printf("<ASSISTANT_STUB_APP>send msg suc line=%d!\n",__LINE__);
    }
    FUNCTION_END
}

static VOID _assistant_stub_json_handle_wifi_setup_result(cJSON * assistant_cmd)
{
    FUNCTION_BEGIN
    INT32    i4_ret;
    HANDLE_T h_app = NULL_HANDLE;
    cJSON *  params=NULL;
    APPMSG_T t_msg = {0};
    ASSISTANT_STUB_WIFI_SETUP_RESULT_T wifi_setup_result={0};

    strncpy(wifi_setup_result.command,cJSON_GetObjectItem(assistant_cmd,ASSISTANT_STUB_COMMAND)->valuestring,ASSISTANT_STUB_COMMAND_MAX_LENGTH);
    params = cJSON_GetObjectItem(assistant_cmd,ASSISTANT_STUB_PARAMS);
    wifi_setup_result.result = cJSON_GetObjectItem(params,ASSISTANT_STUB_RESULT)->valueint;

    i4_ret = u_am_get_app_handle_from_name(&h_app, WIFI_SETTING_THREAD_NAME);
    if(ASSISTANT_STUB_APPR_OK != i4_ret)
    {
        printf("<ASSISTANT_STUB_APP> get handle fail!,i4_ret=%ld\n",i4_ret);
    }
    i4_ret = u_app_send_appmsg(h_app,
                               E_APP_MSG_TYPE_ASSISTANT_STUB,
                               MSG_FROM_ASSISTANT_STUB,
                               ASSISTANT_STUB_CMD_WIFI_SETUP_RESULT,
                               &wifi_setup_result,
                               sizeof(ASSISTANT_STUB_WIFI_SETUP_RESULT_T));
    if(ASSISTANT_STUB_APPR_OK != i4_ret)
    {
        printf("<ASSISTANT_STUB_APP> send msg fail!\n");
    }
    else
    {
        printf("<ASSISTANT_STUB_APP>send msg suc line=%d!\n",__LINE__);
    }
    FUNCTION_END
}

static VOID _assistant_stub_json_handle_wifi_connect_over(cJSON * assistant_cmd)
{
    FUNCTION_BEGIN
    INT32    i4_ret;
    HANDLE_T h_app = NULL_HANDLE;
    APPMSG_T t_msg = {0};
    ASSISTANT_STUB_WIFI_CONNECT_OVER_T cmd = {0};

    strncpy(cmd.command,cJSON_GetObjectItem(assistant_cmd,ASSISTANT_STUB_COMMAND)->valuestring,ASSISTANT_STUB_COMMAND_MAX_LENGTH);

    i4_ret = u_am_get_app_handle_from_name(&h_app, WIFI_SETTING_THREAD_NAME);
    if (i4_ret)
    {
        printf("<ASSISTANT_STUB_APP> get handle fail!,i4_ret=%ld\n",i4_ret);
    }
    i4_ret = u_app_send_appmsg(h_app,
                               E_APP_MSG_TYPE_ASSISTANT_STUB,
                               MSG_FROM_ASSISTANT_STUB,
                               ASSISTANT_STUB_CMD_WIFI_CONNECT_OVER,
                               &cmd,
                               sizeof(cmd));
    if(i4_ret)
    {
        printf("<ASSISTANT_STUB_APP> send msg fail!\n");
    }
    else
    {
        printf("<ASSISTANT_STUB_APP>send msg suc line=%d!\n",__LINE__);
    }
    FUNCTION_END
}

static VOID _assistant_stub_json_handle_speech_start(cJSON * assistant_cmd)
{
    FUNCTION_BEGIN
    INT32    i4_ret;
    HANDLE_T h_app = NULL_HANDLE;
    cJSON *  params=NULL;
    ASSISTANT_STUB_SPEECH_START_T speech_start={0};

    strncpy(speech_start.command,cJSON_GetObjectItem(assistant_cmd,ASSISTANT_STUB_COMMAND)->valuestring,ASSISTANT_STUB_COMMAND_MAX_LENGTH);
    i4_ret = u_am_get_app_handle_from_name(&h_app, SM_THREAD_NAME);
    if(ASSISTANT_STUB_APPR_OK != i4_ret)
    {
        printf("<ASSISTANT_STUB_APP> get handle fail!,i4_ret=%ld\n",i4_ret);
    }
    i4_ret = u_app_send_appmsg(h_app,
                               E_APP_MSG_TYPE_ASSISTANT_STUB,
                               MSG_FROM_ASSISTANT_STUB,
                               ASSISTANT_STUB_CMD_SPEECH_START,
                               &speech_start,
                               sizeof(ASSISTANT_STUB_SPEECH_START_T));
    if(ASSISTANT_STUB_APPR_OK != i4_ret)
    {
        printf("<ASSISTANT_STUB_APP> send msg fail!\n");
    }
    else
    {
        printf("<ASSISTANT_STUB_APP>send msg suc line=%d!\n",__LINE__);
    }
    FUNCTION_END
}

static VOID _assistant_stub_json_handle_speech_process(cJSON * assistant_cmd)
{
    FUNCTION_BEGIN
    INT32    i4_ret;
    HANDLE_T h_app = NULL_HANDLE;
    cJSON *  params=NULL;
    ASSISTANT_STUB_SPEECH_PROCESS_T speech_process={0};

    strncpy(speech_process.command,cJSON_GetObjectItem(assistant_cmd,ASSISTANT_STUB_COMMAND)->valuestring,ASSISTANT_STUB_COMMAND_MAX_LENGTH);
    FUNCTION_END
}

static VOID _assistant_stub_json_handle_speech_feedback(cJSON * assistant_cmd)
{
    FUNCTION_BEGIN
    INT32    i4_ret;
    HANDLE_T h_app = NULL_HANDLE;
    cJSON *  params=NULL;
    ASSISTANT_STUB_SPEECH_FEEDBACK_T speech_feedback={0};

    strncpy(speech_feedback.command,cJSON_GetObjectItem(assistant_cmd,ASSISTANT_STUB_COMMAND)->valuestring,ASSISTANT_STUB_COMMAND_MAX_LENGTH);
    FUNCTION_END
}

static VOID _assistant_stub_json_handle_speech_finish(cJSON * assistant_cmd)
{
    FUNCTION_BEGIN
    INT32    i4_ret;
    HANDLE_T h_app = NULL_HANDLE;
    cJSON *  params=NULL;
    ASSISTANT_STUB_SPEECH_FINISH_T speech_finish={0};

    strncpy(speech_finish.command,cJSON_GetObjectItem(assistant_cmd,ASSISTANT_STUB_COMMAND)->valuestring,ASSISTANT_STUB_COMMAND_MAX_LENGTH);
    i4_ret = u_am_get_app_handle_from_name(&h_app, SM_THREAD_NAME);
    if(ASSISTANT_STUB_APPR_OK != i4_ret)
    {
        printf("<ASSISTANT_STUB_APP> get handle fail!,i4_ret=%ld\n",i4_ret);
    }
    i4_ret = u_app_send_appmsg(h_app,
                               E_APP_MSG_TYPE_ASSISTANT_STUB,
                               MSG_FROM_ASSISTANT_STUB,
                               ASSISTANT_STUB_CMD_SPEECH_FINISH,
                               &speech_finish,
                               sizeof(ASSISTANT_STUB_SPEECH_FINISH_T));
    if(ASSISTANT_STUB_APPR_OK != i4_ret)
    {
        printf("<ASSISTANT_STUB_APP> send msg fail!\n");
    }
    else
    {
        printf("<ASSISTANT_STUB_APP>send msg suc line=%d!\n",__LINE__);
    }
    FUNCTION_END
}

static VOID _assistant_stub_json_handle_pause(cJSON * assistant_cmd)
{
    FUNCTION_BEGIN
    INT32    i4_ret;
    HANDLE_T h_app = NULL_HANDLE;
    cJSON *  params=NULL;
    ASSISTANT_STUB_PAUSE_T pause={0};

    strncpy(pause.command,
            cJSON_GetObjectItem(assistant_cmd,ASSISTANT_STUB_COMMAND)->valuestring,
            ASSISTANT_STUB_COMMAND_MAX_LENGTH);

    i4_ret = u_am_get_app_handle_from_name(&h_app, SM_THREAD_NAME);
    if(ASSISTANT_STUB_APPR_OK != i4_ret)
    {
        printf("<ASSISTANT_STUB_APP> get handle fail!,i4_ret=%ld\n",i4_ret);
    }
    i4_ret = u_app_send_appmsg(h_app,
                               E_APP_MSG_TYPE_ASSISTANT_STUB,
                               MSG_FROM_ASSISTANT_STUB,
                               ASSISTANT_STUB_CMD_PAUSE,
                               &pause,
                               sizeof(ASSISTANT_STUB_PAUSE_T));
    if(ASSISTANT_STUB_APPR_OK != i4_ret)
    {
        printf("<ASSISTANT_STUB_APP> send msg fail!\n");
    }
    else
    {
        printf("<ASSISTANT_STUB_APP>send msg suc line=%d!\n",__LINE__);
    }
    FUNCTION_END
}

static VOID _assistant_stub_json_handle_resume(cJSON * assistant_cmd)
{
    FUNCTION_BEGIN
    INT32    i4_ret;
    HANDLE_T h_app = NULL_HANDLE;
    cJSON *  params=NULL;
    ASSISTANT_STUB_RESUME_T resume={0};

    strncpy(resume.command,
            cJSON_GetObjectItem(assistant_cmd,ASSISTANT_STUB_COMMAND)->valuestring,
            ASSISTANT_STUB_COMMAND_MAX_LENGTH);

    i4_ret = u_am_get_app_handle_from_name(&h_app, SM_THREAD_NAME);
    if(ASSISTANT_STUB_APPR_OK != i4_ret)
    {
        printf("<ASSISTANT_STUB_APP> get handle fail!,i4_ret=%ld\n",i4_ret);
    }
    i4_ret = u_app_send_appmsg(h_app,
                               E_APP_MSG_TYPE_ASSISTANT_STUB,
                               MSG_FROM_ASSISTANT_STUB,
                               ASSISTANT_STUB_CMD_RESUME,
                               &resume,
                               sizeof(ASSISTANT_STUB_RESUME_T));
    if(ASSISTANT_STUB_APPR_OK != i4_ret)
    {
        printf("<ASSISTANT_STUB_APP> send msg fail!\n");
    }
    else
    {
        printf("<ASSISTANT_STUB_APP>send msg suc line=%d!\n",__LINE__);
    }
    FUNCTION_END
}

static VOID _assistant_stub_json_handle_get_speaker_status(cJSON * assistant_cmd)
{
    FUNCTION_BEGIN
    INT32    i4_ret;
    HANDLE_T h_app = NULL_HANDLE;
    cJSON *  params=NULL;
    ASSISTANT_STUB_GET_SPEAKER_STATUS_T get_speaker_status={0};

    strncpy(get_speaker_status.command,
            cJSON_GetObjectItem(assistant_cmd,ASSISTANT_STUB_COMMAND)->valuestring,
            ASSISTANT_STUB_COMMAND_MAX_LENGTH);
    get_speaker_status.id = cJSON_GetObjectItem(assistant_cmd,ASSISTANT_STUB_ID)->valueint;

    speaker_id=get_speaker_status.id;

    i4_ret = u_am_get_app_handle_from_name(&h_app, SM_THREAD_NAME);
    if(ASSISTANT_STUB_APPR_OK != i4_ret)
    {
        printf("<ASSISTANT_STUB_APP> get handle fail!,i4_ret=%ld\n",i4_ret);
    }
    i4_ret = u_app_send_appmsg(h_app,
                               E_APP_MSG_TYPE_ASSISTANT_STUB,
                               MSG_FROM_ASSISTANT_STUB,
                               ASSISTANT_STUB_CMD_GET_SPEAKER_STATUS,
                               &get_speaker_status,
                               sizeof(ASSISTANT_STUB_GET_SPEAKER_STATUS_T));
    if(ASSISTANT_STUB_APPR_OK != i4_ret)
    {
        printf("<ASSISTANT_STUB_APP> send msg fail!\n");
    }
    else
    {
        printf("<ASSISTANT_STUB_APP>send msg suc line=%d!\n",__LINE__);
    }
    FUNCTION_END
}

static VOID _assistant_stub_json_handle_ota_upgrade(cJSON * assistant_cmd)
{
    FUNCTION_BEGIN
    INT32    i4_ret;
    HANDLE_T h_app = NULL_HANDLE;
    cJSON *  params=NULL;
    ASSISTANT_STUB_OTA_UPGRADE_T ota_upgrade={0};

    strncpy(ota_upgrade.command,
            cJSON_GetObjectItem(assistant_cmd,ASSISTANT_STUB_COMMAND)->valuestring,
            ASSISTANT_STUB_COMMAND_MAX_LENGTH);

    params = cJSON_GetObjectItem(assistant_cmd,ASSISTANT_STUB_PARAMS);
    strncpy(ota_upgrade.ota_url,
            cJSON_GetObjectItem(params,ASSISTANT_STUB_OTA_URL)->valuestring,
            ASSISTANT_STUB_URL_MAX_LENGTH);

    i4_ret = u_am_get_app_handle_from_name(&h_app, UPG_THREAD_NAME);
    if(ASSISTANT_STUB_APPR_OK != i4_ret)
    {
        printf("<ASSISTANT_STUB_APP> get handle fail!,i4_ret=%ld\n",i4_ret);
    }
    i4_ret = u_app_send_appmsg(h_app,
                               E_APP_MSG_TYPE_ASSISTANT_STUB,
                               MSG_FROM_ASSISTANT_STUB,
                               ASSISTANT_STUB_CMD_OTA_UPGRADE,
                               &ota_upgrade,
                               sizeof(ASSISTANT_STUB_OTA_UPGRADE_T));
    if(ASSISTANT_STUB_APPR_OK != i4_ret)
    {
        printf("<ASSISTANT_STUB_APP> send msg fail!\n");
    }
    else
    {
        printf("<ASSISTANT_STUB_APP>send msg suc line=%d!\n",__LINE__);
    }
    FUNCTION_END
}

static VOID _assistant_stub_json_handle_adjust_progress(cJSON * assistant_cmd)
{
    FUNCTION_BEGIN
    INT32    i4_ret;
    HANDLE_T h_app = NULL_HANDLE;
    cJSON *  params=NULL;
    ASSISTANT_STUB_ADJUST_PROGRESS_T adjust_progress={0};

    strncpy(adjust_progress.command,
            cJSON_GetObjectItem(assistant_cmd,ASSISTANT_STUB_COMMAND)->valuestring,
            ASSISTANT_STUB_COMMAND_MAX_LENGTH);

    params = cJSON_GetObjectItem(assistant_cmd,ASSISTANT_STUB_PARAMS);
    adjust_progress.progress = cJSON_GetObjectItem(params,ASSISTANT_STUB_PROGRESS)->valueint;

    i4_ret = u_am_get_app_handle_from_name(&h_app, PB_URI_THREAD_NAME);
    if(ASSISTANT_STUB_APPR_OK != i4_ret)
    {
        printf("<ASSISTANT_STUB_APP> get handle fail!,i4_ret=%ld\n",i4_ret);
    }
    i4_ret = u_app_send_appmsg(h_app,
                               E_APP_MSG_TYPE_ASSISTANT_STUB,
                               MSG_FROM_ASSISTANT_STUB,
                               ASSISTANT_STUB_CMD_ADJUST_PROGRESS,
                               &adjust_progress,
                               sizeof(ASSISTANT_STUB_ADJUST_PROGRESS_T));
    if(ASSISTANT_STUB_APPR_OK != i4_ret)
    {
        printf("<ASSISTANT_STUB_APP> send msg fail!\n");
    }
    else
    {
        printf("<ASSISTANT_STUB_APP>send msg suc line=%d!\n",__LINE__);
    }

    FUNCTION_END
}

static VOID _assistant_stub_json_handle_stop(cJSON * assistant_cmd)
{
    FUNCTION_BEGIN
    INT32    i4_ret;
    HANDLE_T h_app = NULL_HANDLE;
    cJSON *  params=NULL;
    ASSISTANT_STUB_STOP_T stop={0};

    strncpy(stop.command,
            cJSON_GetObjectItem(assistant_cmd,ASSISTANT_STUB_COMMAND)->valuestring,
            ASSISTANT_STUB_COMMAND_MAX_LENGTH);

    i4_ret = u_am_get_app_handle_from_name(&h_app, SM_THREAD_NAME);
    if(ASSISTANT_STUB_APPR_OK != i4_ret)
    {
        printf("<ASSISTANT_STUB_APP> get handle fail!,i4_ret=%ld\n",i4_ret);
    }
    i4_ret = u_app_send_appmsg(h_app,
                               E_APP_MSG_TYPE_ASSISTANT_STUB,
                               MSG_FROM_ASSISTANT_STUB,
                               ASSISTANT_STUB_CMD_STOP,
                               &stop,
                               sizeof(ASSISTANT_STUB_STOP_T));
    if(ASSISTANT_STUB_APPR_OK != i4_ret)
    {
        printf("<ASSISTANT_STUB_APP> send msg fail!\n");
    }
    else
    {
        printf("<ASSISTANT_STUB_APP>send msg suc line=%d!\n",__LINE__);
    }

    FUNCTION_END
}

static VOID _assistant_stub_json_handle_factory_reset_result(cJSON * assistant_cmd)
{
    FUNCTION_BEGIN
    INT32    i4_ret;
    HANDLE_T h_app = NULL_HANDLE;
    cJSON *  params=NULL;
    ASSISTANT_STUB_FACTORY_RESET_RESULT_T factory_reset_result={0};

    strncpy(factory_reset_result.command,
            cJSON_GetObjectItem(assistant_cmd,ASSISTANT_STUB_COMMAND)->valuestring,
            ASSISTANT_STUB_COMMAND_MAX_LENGTH);

    params = cJSON_GetObjectItem(assistant_cmd,ASSISTANT_STUB_PARAMS);

    factory_reset_result.result = cJSON_GetObjectItem(params,ASSISTANT_STUB_RESULT)->valueint;

    i4_ret = u_am_get_app_handle_from_name(&h_app, SM_THREAD_NAME);
    if(ASSISTANT_STUB_APPR_OK != i4_ret)
    {
        printf("<ASSISTANT_STUB_APP> get handle fail!,i4_ret=%ld\n",i4_ret);
    }
    i4_ret = u_app_send_appmsg(h_app,
                               E_APP_MSG_TYPE_ASSISTANT_STUB,
                               MSG_FROM_ASSISTANT_STUB,
                               ASSISTANT_STUB_CMD_FACTORY_RESET_RESULT,
                               &factory_reset_result,
                               sizeof(ASSISTANT_STUB_FACTORY_RESET_RESULT_T));
    if(ASSISTANT_STUB_APPR_OK != i4_ret)
    {
        printf("<ASSISTANT_STUB_APP> send msg fail!\n");
    }
    else
    {
        printf("<ASSISTANT_STUB_APP>send msg suc line=%d!\n",__LINE__);
    }

    FUNCTION_END
}

static VOID _assistant_stub_json_handle_hfp_free_mic_result(cJSON * assistant_cmd)
{
    FUNCTION_BEGIN
    INT32    i4_ret;
    HANDLE_T h_app = NULL_HANDLE;
    cJSON *  params=NULL;
    ASSISTANT_STUB_HFP_FREE_MIC_RESULT_T hfp_free_mic_result={0};

    strncpy(hfp_free_mic_result.command,
            cJSON_GetObjectItem(assistant_cmd,ASSISTANT_STUB_COMMAND)->valuestring,
            ASSISTANT_STUB_COMMAND_MAX_LENGTH);

    params = cJSON_GetObjectItem(assistant_cmd,ASSISTANT_STUB_PARAMS);

    hfp_free_mic_result.result = cJSON_GetObjectItem(params,ASSISTANT_STUB_RESULT)->valueint;

    i4_ret = u_am_get_app_handle_from_name(&h_app, SM_THREAD_NAME);
    if(ASSISTANT_STUB_APPR_OK != i4_ret)
    {
        printf("<ASSISTANT_STUB_APP> get handle fail!,i4_ret=%ld\n",i4_ret);
    }
    i4_ret = u_app_send_appmsg(h_app,
                               E_APP_MSG_TYPE_ASSISTANT_STUB,
                               MSG_FROM_ASSISTANT_STUB,
                               ASSISTANT_STUB_CMD_HFP_FREE_MIC_RESULT,
                               &hfp_free_mic_result,
                               sizeof(ASSISTANT_STUB_HFP_FREE_MIC_RESULT_T));
    if(ASSISTANT_STUB_APPR_OK != i4_ret)
    {
        printf("<ASSISTANT_STUB_APP> send msg fail!\n");
    }
    else
    {
        printf("<ASSISTANT_STUB_APP>send msg suc line=%d!\n",__LINE__);
    }

    FUNCTION_END
}

static VOID _assistant_stub_json_handle_start_bt_pair(cJSON * assistant_cmd)
{
    FUNCTION_BEGIN
    INT32    i4_ret;
    HANDLE_T h_app = NULL_HANDLE;

    ASSISTANT_STUB_START_BT_PAIR_T start_bt_pair ={0};

    strncpy(start_bt_pair.command,
            cJSON_GetObjectItem(assistant_cmd,ASSISTANT_STUB_COMMAND)->valuestring,
            ASSISTANT_STUB_COMMAND_MAX_LENGTH);

    i4_ret = u_am_get_app_handle_from_name(&h_app, BLUETOOTH_THREAD_NAME);
    if(ASSISTANT_STUB_APPR_OK != i4_ret)
    {
        printf("<ASSISTANT_STUB_APP> get handle fail!,i4_ret=%ld\n",i4_ret);
    }
    i4_ret = u_app_send_appmsg(h_app,
                               E_APP_MSG_TYPE_ASSISTANT_STUB,
                               MSG_FROM_ASSISTANT_STUB,
                               ASSISTANT_STUB_CMD_START_BT_PAIR,
                               &start_bt_pair,
                               sizeof(ASSISTANT_STUB_START_BT_PAIR_T));
    if(ASSISTANT_STUB_APPR_OK != i4_ret)
    {
        printf("<ASSISTANT_STUB_APP> send msg fail!\n");
    }
    else
    {
        printf("<ASSISTANT_STUB_APP>send msg suc line=%d!\n",__LINE__);
    }
    FUNCTION_END
}

static VOID _assistant_stub_json_handle_del_bt_paired(cJSON * assistant_cmd)
{
    FUNCTION_BEGIN
    INT32    i4_ret;
    HANDLE_T h_app = NULL_HANDLE;

    ASSISTANT_STUB_DEL_BT_PAIRED_T del_bt_paired ={0};

    strncpy(del_bt_paired.command,
            cJSON_GetObjectItem(assistant_cmd,ASSISTANT_STUB_COMMAND)->valuestring,
            ASSISTANT_STUB_COMMAND_MAX_LENGTH);

    i4_ret = u_am_get_app_handle_from_name(&h_app, BLUETOOTH_THREAD_NAME);
    if(ASSISTANT_STUB_APPR_OK != i4_ret)
    {
        printf("<ASSISTANT_STUB_APP> get handle fail!,i4_ret=%ld\n",i4_ret);
    }
    i4_ret = u_app_send_appmsg(h_app,
                               E_APP_MSG_TYPE_ASSISTANT_STUB,
                               MSG_FROM_ASSISTANT_STUB,
                               ASSISTANT_STUB_CMD_DEL_BT_PAIRED,
                               &del_bt_paired,
                               sizeof(ASSISTANT_STUB_DEL_BT_PAIRED_T));
    if(ASSISTANT_STUB_APPR_OK != i4_ret)
    {
        printf("<ASSISTANT_STUB_APP> send msg fail!\n");
    }
    else
    {
        printf("<ASSISTANT_STUB_APP>send msg suc line=%d!\n",__LINE__);
    }
    FUNCTION_END
}

static VOID _assistant_stub_json_handle_bt_power_on(cJSON * assistant_cmd)
{
    FUNCTION_BEGIN
    INT32    i4_ret;
    HANDLE_T h_app = NULL_HANDLE;

    ASSISTANT_STUB_BT_POWER_ON_T bt_power_on ={0};

    strncpy(bt_power_on.command,
            cJSON_GetObjectItem(assistant_cmd,ASSISTANT_STUB_COMMAND)->valuestring,
            ASSISTANT_STUB_COMMAND_MAX_LENGTH);

    i4_ret = u_am_get_app_handle_from_name(&h_app, BLUETOOTH_THREAD_NAME);
    if(ASSISTANT_STUB_APPR_OK != i4_ret)
    {
        printf("<ASSISTANT_STUB_APP> get handle fail!,i4_ret=%ld\n",i4_ret);
    }
    i4_ret = u_app_send_appmsg(h_app,
                               E_APP_MSG_TYPE_ASSISTANT_STUB,
                               MSG_FROM_ASSISTANT_STUB,
                               ASSISTANT_STUB_CMD_BT_POWER_ON,
                               &bt_power_on,
                               sizeof(ASSISTANT_STUB_BT_POWER_ON_T));
    if(ASSISTANT_STUB_APPR_OK != i4_ret)
    {
        printf("<ASSISTANT_STUB_APP> send msg fail!\n");
    }
    else
    {
        printf("<ASSISTANT_STUB_APP>send msg suc line=%d!\n",__LINE__);
    }
    FUNCTION_END
}

static VOID _assistant_stub_json_handle_bt_power_off(cJSON * assistant_cmd)
{
    FUNCTION_BEGIN
    INT32    i4_ret;
    HANDLE_T h_app = NULL_HANDLE;

    ASSISTANT_STUB_BT_POWER_OFF_T bt_power_off ={0};

    strncpy(bt_power_off.command,
            cJSON_GetObjectItem(assistant_cmd,ASSISTANT_STUB_COMMAND)->valuestring,
            ASSISTANT_STUB_COMMAND_MAX_LENGTH);

    i4_ret = u_am_get_app_handle_from_name(&h_app, BLUETOOTH_THREAD_NAME);
    if(ASSISTANT_STUB_APPR_OK != i4_ret)
    {
        printf("<ASSISTANT_STUB_APP> get handle fail!,i4_ret=%ld\n",i4_ret);
    }
    i4_ret = u_app_send_appmsg(h_app,
                               E_APP_MSG_TYPE_ASSISTANT_STUB,
                               MSG_FROM_ASSISTANT_STUB,
                               ASSISTANT_STUB_CMD_BT_POWER_OFF,
                               &bt_power_off,
                               sizeof(ASSISTANT_STUB_BT_POWER_OFF_T));
    if(ASSISTANT_STUB_APPR_OK != i4_ret)
    {
        printf("<ASSISTANT_STUB_APP> send msg fail!\n");
    }
    else
    {
        printf("<ASSISTANT_STUB_APP>send msg suc line=%d!\n",__LINE__);
    }
    FUNCTION_END
}

static VOID _assistant_stub_json_handle_bt_disconnect(cJSON * assistant_cmd)
{
    FUNCTION_BEGIN
    INT32    i4_ret;
    HANDLE_T h_app = NULL_HANDLE;

    ASSISTANT_STUB_BT_DISCONNECT_T bt_disconnect ={0};

    strncpy(bt_disconnect.command,
            cJSON_GetObjectItem(assistant_cmd,ASSISTANT_STUB_COMMAND)->valuestring,
            ASSISTANT_STUB_COMMAND_MAX_LENGTH);

    i4_ret = u_am_get_app_handle_from_name(&h_app, BLUETOOTH_THREAD_NAME);
    if(ASSISTANT_STUB_APPR_OK != i4_ret)
    {
        printf("<ASSISTANT_STUB_APP> get handle fail!,i4_ret=%ld\n",i4_ret);
    }
    i4_ret = u_app_send_appmsg(h_app,
                               E_APP_MSG_TYPE_ASSISTANT_STUB,
                               MSG_FROM_ASSISTANT_STUB,
                               ASSISTANT_STUB_CMD_BT_DISCONNECT,
                               &bt_disconnect,
                               sizeof(ASSISTANT_STUB_BT_DISCONNECT_T));
    if(ASSISTANT_STUB_APPR_OK != i4_ret)
    {
        printf("<ASSISTANT_STUB_APP> send msg fail!\n");
    }
    else
    {
        printf("<ASSISTANT_STUB_APP>send msg suc line=%d!\n",__LINE__);
    }
    FUNCTION_END
}

static VOID _assistant_stub_json_handle_dispatch_playback_cmd(cJSON * assistant_cmd)
{
    FUNCTION_BEGIN
    char cmd[ASSISTANT_STUB_COMMAND_MAX_LENGTH+1] = {0};

    strncpy(cmd,
            cJSON_GetObjectItem(assistant_cmd,ASSISTANT_STUB_COMMAND)->valuestring,
            ASSISTANT_STUB_COMMAND_MAX_LENGTH);

    if((strlen(cmd) == strlen(assistant_map[ASSISTANT_STUB_CMD_PLAY].command))&&
      (!strcmp(cmd,assistant_map[ASSISTANT_STUB_CMD_PLAY].command)))
    {
        _assistant_stub_json_handle_play(assistant_cmd);
    }
    else if((strlen(cmd) == strlen(assistant_map[ASSISTANT_STUB_CMD_PLAY_VOICE_PROMPT].command))&&
           (!strcmp(cmd,assistant_map[ASSISTANT_STUB_CMD_PLAY_VOICE_PROMPT].command)))
    {
        _assistant_stub_json_handle_play_voice_prompt(assistant_cmd);
    }
    else if((strlen(cmd) == strlen(assistant_map[ASSISTANT_STUB_CMD_PLAY_TTS].command))&&
           (!strcmp(cmd,assistant_map[ASSISTANT_STUB_CMD_PLAY_TTS].command)))
    {
        _assistant_stub_json_handle_play_tts(assistant_cmd);
    }
	else if((strlen(cmd) == strlen(assistant_map[ASSISTANT_STUB_CMD_PAUSE].command))&&
           (!strcmp(cmd,assistant_map[ASSISTANT_STUB_CMD_PAUSE].command)))
    {
        _assistant_stub_json_handle_pause(assistant_cmd);
    }
    else if((strlen(cmd) == strlen(assistant_map[ASSISTANT_STUB_CMD_RESUME].command))&&
           (!strcmp(cmd,assistant_map[ASSISTANT_STUB_CMD_RESUME].command)))
    {
        _assistant_stub_json_handle_resume(assistant_cmd);
    }
	else if((strlen(cmd) == strlen(assistant_map[ASSISTANT_STUB_CMD_STOP].command))&&
           (!strcmp(cmd,assistant_map[ASSISTANT_STUB_CMD_STOP].command)))
    {
        _assistant_stub_json_handle_stop(assistant_cmd);
    }
	else if((strlen(cmd) == strlen(assistant_map[ASSISTANT_STUB_CMD_ADJUST_PROGRESS].command))&&
           (!strcmp(cmd,assistant_map[ASSISTANT_STUB_CMD_ADJUST_PROGRESS].command)))
    {
        _assistant_stub_json_handle_adjust_progress(assistant_cmd);
    }
    else
    {
        printf("<ASSISTANT_STUB_APP> error cmd %s\n",cmd);
    }
    FUNCTION_END

}

static VOID _assistant_stub_json_handle_dispatch_system_cmd(cJSON * assistant_cmd)
{
    FUNCTION_BEGIN
    char cmd[ASSISTANT_STUB_COMMAND_MAX_LENGTH+1] = {0};

    strncpy(cmd,
            cJSON_GetObjectItem(assistant_cmd,ASSISTANT_STUB_COMMAND)->valuestring,
            ASSISTANT_STUB_COMMAND_MAX_LENGTH);

    if((strlen(cmd) == strlen(assistant_map[ASSISTANT_STUB_CMD_SET_VOLUME].command))&&
      (!strcmp(cmd,assistant_map[ASSISTANT_STUB_CMD_SET_VOLUME].command)))
    {
        _assistant_stub_json_handle_set_volume(assistant_cmd);
    }
	else if((strlen(cmd) == strlen(assistant_map[ASSISTANT_STUB_CMD_SET_SYSTEM_STATUS].command))&&
           (!strcmp(cmd,assistant_map[ASSISTANT_STUB_CMD_SET_SYSTEM_STATUS].command)))
    {
        _assistant_stub_json_handle_set_system_status(assistant_cmd);
    }
	else if((strlen(cmd) == strlen(assistant_map[ASSISTANT_STUB_CMD_OTA_UPGRADE].command))&&
           (!strcmp(cmd,assistant_map[ASSISTANT_STUB_CMD_OTA_UPGRADE].command)))
    {
        _assistant_stub_json_handle_ota_upgrade(assistant_cmd);
    }
	else if((strlen(cmd) == strlen(assistant_map[ASSISTANT_STUB_CMD_FACTORY_RESET_RESULT].command))&&
           (!strcmp(cmd,assistant_map[ASSISTANT_STUB_CMD_FACTORY_RESET_RESULT].command)))
    {
        _assistant_stub_json_handle_factory_reset_result(assistant_cmd);
    }
    else if((strlen(cmd) == strlen(assistant_map[ASSISTANT_STUB_CMD_HFP_FREE_MIC_RESULT].command))&&
           (!strcmp(cmd,assistant_map[ASSISTANT_STUB_CMD_HFP_FREE_MIC_RESULT].command)))
    {
        _assistant_stub_json_handle_hfp_free_mic_result(assistant_cmd);
    }
	else if((strlen(cmd) == strlen(assistant_map[ASSISTANT_STUB_CMD_GET_SPEAKER_STATUS].command))&&
           (!strcmp(cmd,assistant_map[ASSISTANT_STUB_CMD_GET_SPEAKER_STATUS].command)))
    {
        _assistant_stub_json_handle_get_speaker_status(assistant_cmd);
    }
    else
    {
        printf("<ASSISTANT_STUB_APP> error cmd %s\n",cmd);
    }
    FUNCTION_END

}

static VOID _assistant_stub_json_handle_dispatch_bluetooth_cmd(cJSON * assistant_cmd)
{
    FUNCTION_BEGIN
    char cmd[ASSISTANT_STUB_COMMAND_MAX_LENGTH+1] = {0};

    strncpy(cmd,
            cJSON_GetObjectItem(assistant_cmd,ASSISTANT_STUB_COMMAND)->valuestring,
            ASSISTANT_STUB_COMMAND_MAX_LENGTH);

    if((strlen(cmd) == strlen(assistant_map[ASSISTANT_STUB_CMD_PLAY_PREV_AUDIO].command))&&
           (!strcmp(cmd,assistant_map[ASSISTANT_STUB_CMD_PLAY_PREV_AUDIO].command)))
    {
        _assistant_stub_json_handle_play_prev_audio(assistant_cmd);
    }
    else if((strlen(cmd) == strlen(assistant_map[ASSISTANT_STUB_CMD_PLAY_NEXT_AUDIO].command))&&
           (!strcmp(cmd,assistant_map[ASSISTANT_STUB_CMD_PLAY_NEXT_AUDIO].command)))
    {
        _assistant_stub_json_handle_play_next_audio(assistant_cmd);
    }
	else if((strlen(cmd) == strlen(assistant_map[ASSISTANT_STUB_CMD_SET_BT_NAME].command))&&
           (!strcmp(cmd,assistant_map[ASSISTANT_STUB_CMD_SET_BT_NAME].command)))
    {
        _assistant_stub_json_handle_set_bt_name(assistant_cmd);
    }
	else if((strlen(cmd) == strlen(assistant_map[ASSISTANT_STUB_CMD_START_BT_PAIR].command))&&
           (!strcmp(cmd,assistant_map[ASSISTANT_STUB_CMD_START_BT_PAIR].command)))
    {
        _assistant_stub_json_handle_start_bt_pair(assistant_cmd);
    }
    else if((strlen(cmd) == strlen(assistant_map[ASSISTANT_STUB_CMD_DEL_BT_PAIRED].command))&&
           (!strcmp(cmd,assistant_map[ASSISTANT_STUB_CMD_DEL_BT_PAIRED].command)))
    {
        _assistant_stub_json_handle_del_bt_paired(assistant_cmd);
    }
    else if((strlen(cmd) == strlen(assistant_map[ASSISTANT_STUB_CMD_BT_POWER_ON].command))&&
           (!strcmp(cmd,assistant_map[ASSISTANT_STUB_CMD_BT_POWER_ON].command)))
    {
        _assistant_stub_json_handle_bt_power_on(assistant_cmd);
    }
    else if((strlen(cmd) == strlen(assistant_map[ASSISTANT_STUB_CMD_BT_POWER_OFF].command))&&
           (!strcmp(cmd,assistant_map[ASSISTANT_STUB_CMD_BT_POWER_OFF].command)))
    {
        _assistant_stub_json_handle_bt_power_off(assistant_cmd);
    }
    else if((strlen(cmd) == strlen(assistant_map[ASSISTANT_STUB_CMD_BT_DISCONNECT].command))&&
           (!strcmp(cmd,assistant_map[ASSISTANT_STUB_CMD_BT_DISCONNECT].command)))
    {
        _assistant_stub_json_handle_bt_disconnect(assistant_cmd);
    }
	else if((strlen(cmd) == strlen(assistant_map[ASSISTANT_STUB_CMD_PLAY_BT_MUSIC].command))&&
           (!strcmp(cmd,assistant_map[ASSISTANT_STUB_CMD_PLAY_BT_MUSIC].command)))
    {
        _assistant_stub_json_handle_play_bt_music(assistant_cmd);
    }
    else
    {
        printf("<ASSISTANT_STUB_APP> error cmd %s\n",cmd);
    }
    FUNCTION_END

}

static VOID _assistant_stub_json_handle_dispatch_wifi_setting_cmd(cJSON * assistant_cmd)
{
    FUNCTION_BEGIN
    char cmd[ASSISTANT_STUB_COMMAND_MAX_LENGTH+1] = {0};

    strncpy(cmd,
            cJSON_GetObjectItem(assistant_cmd,ASSISTANT_STUB_COMMAND)->valuestring,
            ASSISTANT_STUB_COMMAND_MAX_LENGTH);

    if((strlen(cmd) == strlen(assistant_map[ASSISTANT_STUB_CMD_GET_AP_LIST].command))&&
      (!strcmp(cmd,assistant_map[ASSISTANT_STUB_CMD_GET_AP_LIST].command)))
    {
        _assistant_stub_json_handle_get_ap_list(assistant_cmd);
    }
    else if((strlen(cmd) == strlen(assistant_map[ASSISTANT_STUB_CMD_WIFI_CONNECT].command))&&
      (!strcmp(cmd,assistant_map[ASSISTANT_STUB_CMD_WIFI_CONNECT].command)))
    {
        _assistant_stub_json_handle_wifi_connect(assistant_cmd);
    }
    else if((strlen(cmd) == strlen(assistant_map[ASSISTANT_STUB_CMD_WIFI_CONNECT_OVER].command))&&
           (!strcmp(cmd,assistant_map[ASSISTANT_STUB_CMD_WIFI_CONNECT_OVER].command)))
    {
        _assistant_stub_json_handle_wifi_connect_over(assistant_cmd);
    }
	else if((strlen(cmd) == strlen(assistant_map[ASSISTANT_STUB_CMD_WIFI_SETUP_RESULT].command))&&
           (!strcmp(cmd,assistant_map[ASSISTANT_STUB_CMD_WIFI_SETUP_RESULT].command)))
    {
        _assistant_stub_json_handle_wifi_setup_result(assistant_cmd);
    }
    else
    {
        printf("<ASSISTANT_STUB_APP> error cmd %s\n",cmd);
    }
    FUNCTION_END

}
static VOID _assistant_stub_json_handle_dispatch_speech_cmd(cJSON * assistant_cmd)
{
    FUNCTION_BEGIN
    char cmd[ASSISTANT_STUB_COMMAND_MAX_LENGTH+1] = {0};

    strncpy(cmd,
            cJSON_GetObjectItem(assistant_cmd,ASSISTANT_STUB_COMMAND)->valuestring,
            ASSISTANT_STUB_COMMAND_MAX_LENGTH);
    if((strlen(cmd) == strlen(assistant_map[ASSISTANT_STUB_CMD_SPEECH_START].command))&&
      (!strcmp(cmd,assistant_map[ASSISTANT_STUB_CMD_SPEECH_START].command)))
    {
        _assistant_stub_json_handle_speech_start(assistant_cmd);
    }
    else if((strlen(cmd) == strlen(assistant_map[ASSISTANT_STUB_CMD_SPEECH_PROCESS].command))&&
           (!strcmp(cmd,assistant_map[ASSISTANT_STUB_CMD_SPEECH_PROCESS].command)))
    {
        _assistant_stub_json_handle_speech_process(assistant_cmd);
    }
    else if((strlen(cmd) == strlen(assistant_map[ASSISTANT_STUB_CMD_SPEECH_FEEDBACK].command))&&
           (!strcmp(cmd,assistant_map[ASSISTANT_STUB_CMD_SPEECH_FEEDBACK].command)))
    {
        _assistant_stub_json_handle_speech_feedback(assistant_cmd);
    }
    else if((strlen(cmd) == strlen(assistant_map[ASSISTANT_STUB_CMD_SPEECH_FINISH].command))&&
           (!strcmp(cmd,assistant_map[ASSISTANT_STUB_CMD_SPEECH_FINISH].command)))
    {
        _assistant_stub_json_handle_speech_finish(assistant_cmd);
    }
    else
    {
        printf("<ASSISTANT_STUB_APP> error cmd %s\n",cmd);
    }

    FUNCTION_END

}

static VOID _assistant_stub_json_handle_dispatch_other_cmd(cJSON * assistant_cmd)
{
    FUNCTION_BEGIN

    char cmd[ASSISTANT_STUB_COMMAND_MAX_LENGTH+1] = {0};

    strncpy(cmd,
            cJSON_GetObjectItem(assistant_cmd,ASSISTANT_STUB_COMMAND)->valuestring,
            ASSISTANT_STUB_COMMAND_MAX_LENGTH);

    printf("<ASSISTANT_STUB_APP> error cmd %s\n",cmd);

    FUNCTION_END

}


static VOID _assistant_stub_json_recv_thread_parse_json(cJSON * assistant_cmd)
{
    FUNCTION_BEGIN
    char cmd[ASSISTANT_STUB_COMMAND_MAX_LENGTH+1] = {0};

    strncpy(cmd,
            cJSON_GetObjectItem(assistant_cmd,ASSISTANT_STUB_COMMAND)->valuestring,
            ASSISTANT_STUB_COMMAND_MAX_LENGTH);

    if(NULL != strstr(cmd,"/playback/"))
    {
        _assistant_stub_json_handle_dispatch_playback_cmd(assistant_cmd);
    }
	else if (NULL != strstr(cmd,"/system/"))
    {
        _assistant_stub_json_handle_dispatch_system_cmd(assistant_cmd);
    }
    else if (NULL != strstr(cmd,"/bluetooth/"))
    {
        _assistant_stub_json_handle_dispatch_bluetooth_cmd(assistant_cmd);
    }
    else if (NULL != strstr(cmd,"/wifi/"))
    {
        _assistant_stub_json_handle_dispatch_wifi_setting_cmd(assistant_cmd);
    }
    else if (NULL != strstr(cmd,"/speech/"))
    {
        _assistant_stub_json_handle_dispatch_speech_cmd(assistant_cmd);
    }
    else
    {
        _assistant_stub_json_handle_dispatch_other_cmd(assistant_cmd);
    }

    FUNCTION_END
}

VOID _assistant_stub_json_recv_thread(VOID * arg)
{
    FUNCTION_BEGIN
    char cmd[HUB_CMD_LENGTH_MAX+1] = {0};

    hub_init(&CTX, HUB_APP);

    while(1)
    {
        memset(cmd, 0, sizeof(cmd));
        hub_recv(&CTX, cmd, sizeof(cmd));
        printf("<ASSISTANT_STUB_APP> json recv:%s\n",cmd);

        cJSON * assistant_cmd = cJSON_Parse(cmd);
        if (NULL == assistant_cmd)
        {
            cJSON_Delete(assistant_cmd);
            printf("<ASSISTANT_STUB_APP> error , assistant_cmd is NULL\n");
            return ASSISTANT_STUB_APPR_FAIL;
        }

        _assistant_stub_json_recv_thread_parse_json(assistant_cmd);

        cJSON_Delete(assistant_cmd);
    }

    hub_destroy(&CTX);
    FUNCTION_END

    pthread_exit(NULL);
}

