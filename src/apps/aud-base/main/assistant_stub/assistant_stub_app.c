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
 * $RCSfile: assistant_stub_app.c $
 * $Revision:0.1
 * $Date: 2017/01/05
 * $Author:pingan.liu
 * $CCRevision: $
 * $SWAuthor:  $
 * $MD5HEX: $
 *
 * Description:assistance stub app thread


 *---------------------------------------------------------------------------*/
/*-----------------------------------------------------------------------------
                    include files
-----------------------------------------------------------------------------*/
#include "assistant_stub_app.h"
#include "assistant_stub_app_json_recv.h"
#include "assistant_stub_app_tts_recv.h"
#include "assistant_stub_app_cli.h"
#include "u_acfg.h"
#include "u_dbg.h"
//#include "../wifi_setting/wifi_setting.h"
//#include "c_net_wlan.h"

static ASSISTANT_STUB_APP_OBJ_T t_g_assistant_stub_app = {0};
extern BOOL   need_respose_wifi_connect_rpc;

extern struct hub_ctx CTX;
extern int    speaker_id;
extern BOOL   fg_wifi_connect_status; /*0==disconnect   1==connect*/
extern BOOL   fg_wifi_setup_status; /*1 == setting , 0 == other*/

#define CMD_STR_MAP(cmd) {ASSISTANT_STUB_CMD_##cmd, STRING_##cmd}

ASSISTANT_STUB_CMD_MAP_T assistant_map[ASSISTANT_STUB_CMD_MAX]=
{
    CMD_STR_MAP(PLAY),
    CMD_STR_MAP(PLAY_VOICE_PROMPT),
    CMD_STR_MAP(PLAY_TTS),
    CMD_STR_MAP(PLAY_PREV_AUDIO),
    CMD_STR_MAP(PLAY_NEXT_AUDIO),
    CMD_STR_MAP(SET_VOLUME),
    CMD_STR_MAP(SET_SYSTEM_STATUS),
    CMD_STR_MAP(SET_BT_NAME),
    CMD_STR_MAP(START_BT_PAIR),
    CMD_STR_MAP(DEL_BT_PAIRED),
    CMD_STR_MAP(BT_POWER_ON),
    CMD_STR_MAP(BT_POWER_OFF),
    CMD_STR_MAP(PLAY_BT_MUSIC),
    CMD_STR_MAP(BT_DISCONNECT),
    CMD_STR_MAP(GET_AP_LIST),
    CMD_STR_MAP(WIFI_CONNECT),
    CMD_STR_MAP(WIFI_CONNECT_OVER),
    CMD_STR_MAP(WIFI_SETUP_RESULT),
    CMD_STR_MAP(SPEECH_START),
    CMD_STR_MAP(SPEECH_PROCESS),
    CMD_STR_MAP(SPEECH_FEEDBACK),
    CMD_STR_MAP(SPEECH_FINISH),
    CMD_STR_MAP(GET_SPEAKER_STATUS),
    CMD_STR_MAP(PAUSE),
    CMD_STR_MAP(RESUME),
    CMD_STR_MAP(OTA_UPGRADE),
    CMD_STR_MAP(ADJUST_PROGRESS),
    CMD_STR_MAP(STOP),
    CMD_STR_MAP(FACTORY_RESET_RESULT),
    CMD_STR_MAP(HFP_FREE_MIC_RESULT),

    CMD_STR_MAP(PLAY_DONE),
    CMD_STR_MAP(PLAY_TTS_DONE),
    CMD_STR_MAP(SYSTEM_STATUS_CHANGE),
    CMD_STR_MAP(PLAYER_STATUS_CHANGE),
    CMD_STR_MAP(NETWORK_STATUS_CHANGE),
    CMD_STR_MAP(BLUETOOTH_STATUS_CHANGE),
    CMD_STR_MAP(BUTTON),
    CMD_STR_MAP(OTA_PROGRESS),
    CMD_STR_MAP(HFP_STATUS_CHANGE),

    {ASSISTANT_STUB_CMD_MAX,NULL}

};

INT32 _assistant_stub_cli_button(INT32 i4_argc, const CHAR ** pps_argv)
{
    FUNCTION_BEGIN
    char * out = NULL;
    char send_buffer[HUB_CMD_LENGTH_MAX+1] = {0};
    ASSISTANT_STUB_BUTTON_T button = {0};

    strncpy(button.name,pps_argv[1],ASSISTANT_STUB_NAME);

    printf("<ASSISTANT_STUB_APP> button.name is %s \n",button.name);


    /*create cjson dada*/
    cJSON * params = cJSON_CreateObject();
    cJSON * notificatioon = cJSON_CreateObject();

    /*add params && child object of params*/
    cJSON_AddItemToObject(notificatioon,ASSISTANT_STUB_PARAMS,params);
    cJSON_AddItemToObject(params,
                          ASSISTANT_STUB_NAME,
                          cJSON_CreateString(button.name));

    /*transfer cjson to string and send to assistance server*/
    out = cJSON_Print(notificatioon);
    strncpy(send_buffer,out,HUB_CMD_LENGTH_MAX);
    printf("<ASSISTANT_STUB_APP> send msg is %s\n",send_buffer);
    extern int hub_send(struct hub_ctx * ctx, char * buffer, size_t len);
    hub_send(&CTX,send_buffer,strlen(send_buffer));
    /*delete cjson object*/
    cJSON_Delete(notificatioon);
    free(out);
    out = NULL;

    FUNCTION_END
}

static VOID _assistant_stub_json_handle_play_done(VOID * p_usr_msg)
{
    FUNCTION_BEGIN
    char * out = NULL;
    char send_buffer[HUB_CMD_LENGTH_MAX+1] = {0};
    ASSISTANT_STUB_PLAY_DONE_T play_done = {0};
    memcpy(&play_done,
          (ASSISTANT_STUB_PLAY_DONE_T *)p_usr_msg,
           sizeof(ASSISTANT_STUB_PLAY_DONE_T));

    printf("<ASSISTANT_STUB_APP> command is %s \n",play_done.command);
    printf("<ASSISTANT_STUB_APP> uri is %s \n",play_done.uri);
    printf("<ASSISTANT_STUB_APP> code is %d \n",play_done.code);
    printf("<ASSISTANT_STUB_APP> status is %d \n",play_done.status);

    /*create cjson object*/
    cJSON * error  = cJSON_CreateObject();
    cJSON * params = cJSON_CreateObject();
    cJSON * notificatioon = cJSON_CreateObject();
    /*add  jsonrpc && command*/
    cJSON_AddItemToObject(notificatioon,
                          ASSISTANT_STUB_COMMAND,
                          cJSON_CreateString(play_done.command));
    /*add params && child object of params*/
    cJSON_AddItemToObject(notificatioon,ASSISTANT_STUB_PARAMS,params);
    cJSON_AddItemToObject(params,
                          ASSISTANT_STUB_URI,
                          cJSON_CreateString(play_done.uri));
    cJSON_AddItemToObject(params,
                          ASSISTANT_STUB_STATUS,
                          cJSON_CreateNumber(play_done.status));
    /*add error && child object of error*/
    cJSON_AddItemToObject(params,ASSISTANT_STUB_ERROR,error);
    cJSON_AddItemToObject(error,
                          ASSISTANT_STUB_CODE,
                          cJSON_CreateNumber(play_done.code));
    /*transfer cjson to string and send to assistance server*/
    out = cJSON_Print(notificatioon);
    strncpy(send_buffer,out,HUB_CMD_LENGTH_MAX);
    printf("<ASSISTANT_STUB_APP> send msg is %s\n",send_buffer);
    hub_send(&CTX,send_buffer,strlen(send_buffer));
    /*delete cjson object*/
    cJSON_Delete(notificatioon);
    free(out);
    out = NULL;

    FUNCTION_END
}

static VOID _assistant_stub_json_handle_ota_progress(VOID * p_usr_msg)
{
    FUNCTION_BEGIN
    char * out = NULL;
    char send_buffer[HUB_CMD_LENGTH_MAX+1] = {0};
    ASSISTANT_STUB_OTA_PROGRESS_T ota_progress = {0};
    memcpy(&ota_progress,
          (ASSISTANT_STUB_OTA_PROGRESS_T *)p_usr_msg,
           sizeof(ASSISTANT_STUB_OTA_PROGRESS_T));

    printf("<ASSISTANT_STUB_APP> command is %s \n",ota_progress.command);
    printf("<ASSISTANT_STUB_APP> progress is %d\n",ota_progress.progress);

    /*create cjson object*/
    cJSON * params = cJSON_CreateObject();
    cJSON * notificatioon = cJSON_CreateObject();
    /*add  command*/
    cJSON_AddItemToObject(notificatioon,
                          ASSISTANT_STUB_COMMAND,
                          cJSON_CreateString(ota_progress.command));

    /*add params && child object of params*/
    cJSON_AddItemToObject(notificatioon,ASSISTANT_STUB_PARAMS,params);
    cJSON_AddItemToObject(params,
                          ASSISTANT_STUB_PROGRESS,
                          cJSON_CreateNumber(ota_progress.progress));
    /*transfer cjson to string and send to assistance server*/
    out = cJSON_Print(notificatioon);
    strncpy(send_buffer,out,HUB_CMD_LENGTH_MAX);
    printf("<ASSISTANT_STUB_APP> send msg is %s\n",send_buffer);
    hub_send(&CTX,send_buffer,strlen(send_buffer));
    /*delete cjson object*/
    cJSON_Delete(notificatioon);
    free(out);
    out = NULL;

    FUNCTION_END
}

static VOID _assistant_stub_json_handle_play_tts_done(VOID * p_usr_msg)
{
    FUNCTION_BEGIN
    char * out = NULL;
    char send_buffer[HUB_CMD_LENGTH_MAX+1] = {0};
    ASSISTANT_STUB_PLAY_TTS_DONE_T play_tts_done = {0};
    memcpy(&play_tts_done,
          (ASSISTANT_STUB_PLAY_TTS_DONE_T *)p_usr_msg,
          sizeof(ASSISTANT_STUB_PLAY_TTS_DONE_T));

    printf("<ASSISTANT_STUB_APP> command is %s \n",play_tts_done.command);
    printf("<ASSISTANT_STUB_APP> uri is %d \n",play_tts_done.tts_id);
    printf("<ASSISTANT_STUB_APP> status is %d\n",play_tts_done.status);

    /*create cjson dada*/
    cJSON * params = cJSON_CreateObject();
    cJSON * notificatioon = cJSON_CreateObject();

    /*add  jsonrpc && command*/
    cJSON_AddItemToObject(notificatioon,
                          ASSISTANT_STUB_COMMAND,
                          cJSON_CreateString(play_tts_done.command));
    /*add params && child object of params*/
    cJSON_AddItemToObject(notificatioon,ASSISTANT_STUB_PARAMS,params);
    cJSON_AddItemToObject(params,
                          ASSISTANT_STUB_TTS_ID,
                          cJSON_CreateNumber(play_tts_done.tts_id));
    cJSON_AddItemToObject(params,
                          ASSISTANT_STUB_STATUS,
                          cJSON_CreateNumber(play_tts_done.status));
    /*transfer cjson to string and send to assistance server*/
    out = cJSON_Print(notificatioon);
    strncpy(send_buffer,out,HUB_CMD_LENGTH_MAX);
    printf("<ASSISTANT_STUB_APP> send msg is %s\n",send_buffer);
    hub_send(&CTX,send_buffer,strlen(send_buffer));
    /*delete cjson object*/
    cJSON_Delete(notificatioon);
    free(out);
    out = NULL;

    FUNCTION_END
}

static VOID _assistant_stub_json_handle_respose_speaker_status(VOID *p_usr_msg)
{
    FUNCTION_BEGIN
    char *out = NULL;
    ASSISTANT_STUB_GET_SPEAKER_STATUS_RESPONSE_T *cmd = p_usr_msg;

    /*create json dada*/
    cJSON *bluetooth = cJSON_CreateObject();
    cJSON *network   = cJSON_CreateObject();
    cJSON *power     = cJSON_CreateObject();
    cJSON *player    = cJSON_CreateObject();
    cJSON *system    = cJSON_CreateObject();
    cJSON *result    = cJSON_CreateObject();
    cJSON *notificatioon = cJSON_CreateObject();

    cJSON_AddItemToObject(notificatioon, ASSISTANT_STUB_COMMAND,
                          cJSON_CreateString(STRING_GET_SPEAKER_STATUS));

    /*add system and child of system*/
    printf("cmd->system.status is %s \n",cmd->system.status);
    cJSON_AddItemToObject(system,
                          ASSISTANT_STUB_STATUS,
                          cJSON_CreateString(cmd->system.status));

    printf("cmd->system.wifi_mac is %s \n",cmd->system.wifi_mac);
    cJSON_AddItemToObject(system,
                          ASSISTANT_STUB_WIFI_MAC,
                          cJSON_CreateString(cmd->system.wifi_mac));

	printf("cmd->system.bt_mac is %s \n",cmd->system.bt_mac);
    cJSON_AddItemToObject(system,
                          ASSISTANT_STUB_BT_MAC,
                          cJSON_CreateString(cmd->system.bt_mac));

	printf("cmd->system.device_sn is %s \n",cmd->system.device_sn);
    cJSON_AddItemToObject(system,
                          ASSISTANT_STUB_DEVICE_SN,
                          cJSON_CreateString(cmd->system.device_sn));
    cJSON_AddItemToObject(result,ASSISTANT_STUB_SYSTEM,system);

    /*add player and child of player*/
    printf(" cmd->player.volume is %d \n",cmd->player.volume);
    cJSON_AddItemToObject(player,
                          ASSISTANT_STUB_VOLUME,
                          cJSON_CreateNumber(cmd->player.volume));
    printf("cmd->player.status is %s \n",cmd->player.status);
    cJSON_AddItemToObject(player,
                          ASSISTANT_STUB_STATUS,
                          cJSON_CreateString(cmd->player.status));
    printf(" cmd->player.source is %s\n",cmd->player.source);
    cJSON_AddItemToObject(player,
                          ASSISTANT_STUB_SOURCE,
                          cJSON_CreateString(cmd->player.source));

    printf(" cmd->player.audioId is %s\n",cmd->player.audioId);
    cJSON_AddItemToObject(player,
                          ASSISTANT_STUB_AUDIO_ID,
                          cJSON_CreateString(cmd->player.audioId));

    printf(" cmd->player.audioSource is %s\n",cmd->player.audioSource);
    cJSON_AddItemToObject(player,
                          ASSISTANT_STUB_AUDIO_SOURCE,
                          cJSON_CreateString(cmd->player.audioSource));

    printf(" cmd->player.audioName is %s\n",cmd->player.audioName);
    cJSON_AddItemToObject(player,
                          ASSISTANT_STUB_AUDIO_NAME,
                          cJSON_CreateString(cmd->player.audioName));

    printf(" cmd->player.audioAnchor is %s\n",cmd->player.audioAnchor);
    cJSON_AddItemToObject(player,
                          ASSISTANT_STUB_AUDIO_ANCHOR,
                          cJSON_CreateString(cmd->player.audioAnchor));

    printf(" cmd->player.audioAlbum is %s\n",cmd->player.audioAlbum);
    cJSON_AddItemToObject(player,
                          ASSISTANT_STUB_AUDIO_ALBUM,
                          cJSON_CreateString(cmd->player.audioAlbum));

    printf(" cmd->player.progress is %d\n",cmd->player.progress);
    cJSON_AddItemToObject(player,
                          ASSISTANT_STUB_PROGRESS,
                          cJSON_CreateNumber(cmd->player.progress));
    cJSON_AddItemToObject(result,ASSISTANT_STUB_PLAYER,player);

    /*add power and child of power*/
    printf(" cmd->power.quantity is %d\n",cmd->power.quantity);
    cJSON_AddItemToObject(power,
                          ASSISTANT_STUB_QUANTITY,
                          cJSON_CreateNumber(cmd->power.quantity));

    printf("cmd->power.status is %s \n",cmd->power.status);
    cJSON_AddItemToObject(power,
                          ASSISTANT_STUB_STATUS,
                          cJSON_CreateString(cmd->power.status));
    cJSON_AddItemToObject(result,ASSISTANT_STUB_POWER,power);

    /*add network and child of network*/

    printf(" cmd->network.quantity is %d \n",cmd->network.quantity);
    cJSON_AddItemToObject(network,
                          ASSISTANT_STUB_QUANTITY,
                          cJSON_CreateNumber(cmd->network.quantity));

    printf("cmd->network.status is %s \n",cmd->network.status);
    cJSON_AddItemToObject(network,
                          ASSISTANT_STUB_STATUS,
                          cJSON_CreateString(cmd->network.status));

	printf("cmd->network.ssid is %s \n",cmd->network.ssid);
    cJSON_AddItemToObject(network,
                          ASSISTANT_STUB_SSID,
                          cJSON_CreateString(cmd->network.ssid));

    printf("cmd->network.bssid is %s \n",cmd->network.bssid);
    cJSON_AddItemToObject(network,
                          ASSISTANT_STUB_BSSID,
                          cJSON_CreateString(cmd->network.bssid));
    cJSON_AddItemToObject(result,ASSISTANT_STUB_NETWORK,network);

    /*add bluetooth and child of bluetooth*/

    printf(" cmd->bluetooth.status is %s\n",cmd->bluetooth.status);
    cJSON_AddItemToObject(bluetooth,
                          ASSISTANT_STUB_STATUS,
                          cJSON_CreateString(cmd->bluetooth.status));

    printf(" cmd->bluetooth.name is %s\n",cmd->bluetooth.name);
    cJSON_AddItemToObject(bluetooth,
                          ASSISTANT_STUB_NAME,
                          cJSON_CreateString(cmd->bluetooth.name));

	printf(" cmd->bluetooth.bt_paired_name is %s\n",cmd->bluetooth.bt_paired_name);
    cJSON_AddItemToObject(bluetooth,
                          ASSISTANT_STUB_BT_PAIRED_NAME,
                          cJSON_CreateString(cmd->bluetooth.bt_paired_name));
    cJSON_AddItemToObject(result,ASSISTANT_STUB_BLUETOOTH,bluetooth);

    cJSON_AddItemToObject(notificatioon,ASSISTANT_STUB_RESULT,result);

    cJSON_AddItemToObject(notificatioon,ASSISTANT_STUB_ID,cJSON_CreateNumber(cmd->id));

    /*transfer cjson to string and send to assistance server*/
    out = cJSON_Print(notificatioon);
    printf("<ASSISTANT_STUB_APP> send msg is %s\n",out);
    hub_send(&CTX, out, strlen(out));
    /*delete cjson object*/
    cJSON_Delete(notificatioon);
    free(out);

    FUNCTION_END
}

static VOID _assistant_stub_json_handle_system_status_change(VOID * p_usr_msg)
{
    FUNCTION_BEGIN
    char * out = NULL;
    char send_buffer[HUB_CMD_LENGTH_MAX+1] = {0};
    ASSISTANT_STUB_SYSTEM_STATUS_CHANGE_T system_status_change = {0};
    memcpy(&system_status_change,
          (ASSISTANT_STUB_SYSTEM_STATUS_CHANGE_T *)p_usr_msg,
           sizeof(ASSISTANT_STUB_SYSTEM_STATUS_CHANGE_T));

    printf("<ASSISTANT_STUB_APP> command is %s \n",system_status_change.command);
    printf("<ASSISTANT_STUB_APP> status is %s \n",system_status_change.status);

    /*create cjson object*/
    cJSON * params = cJSON_CreateObject();
    cJSON * notificatioon = cJSON_CreateObject();
    /*add  jsonrpc && command*/
    cJSON_AddItemToObject(notificatioon,
                          ASSISTANT_STUB_COMMAND,
                          cJSON_CreateString(system_status_change.command));
    /*add params && status*/
    cJSON_AddItemToObject(notificatioon,ASSISTANT_STUB_PARAMS,params);
    cJSON_AddItemToObject(params,
                          ASSISTANT_STUB_STATUS,
                          cJSON_CreateString(system_status_change.status));
    /*transfer cjson to string and send to assistance server*/
    out = cJSON_Print(notificatioon);
    strncpy(send_buffer,out,HUB_CMD_LENGTH_MAX);
    printf("<ASSISTANT_STUB_APP> send msg is %s\n",send_buffer);
    hub_send(&CTX,send_buffer,strlen(send_buffer));
    /*delete cjson object*/
    cJSON_Delete(notificatioon);
    free(out);
    out = NULL;

    FUNCTION_END
}

static VOID _assistant_stub_json_handle_player_status_change(VOID * p_usr_msg)
{
    FUNCTION_BEGIN
    char * out = NULL;
    char send_buffer[HUB_CMD_LENGTH_MAX+1] = {0};
    ASSISTANT_STUB_PLAYER_STATUS_CHANGE_T player_status_change = {0};
    memcpy(&player_status_change,
          (ASSISTANT_STUB_PLAYER_STATUS_CHANGE_T *)p_usr_msg,
           sizeof(ASSISTANT_STUB_PLAYER_STATUS_CHANGE_T));

    printf("<ASSISTANT_STUB_APP> source is %s \n",player_status_change.player.source);
    printf("<ASSISTANT_STUB_APP> status is %s \n",player_status_change.player.status);
    printf("<ASSISTANT_STUB_APP> volume is %d \n",player_status_change.player.volume);

    /*create cjson object*/
    cJSON * params = cJSON_CreateObject();
    cJSON * notificatioon = cJSON_CreateObject();
    /*add  jsonrpc && command*/
    cJSON_AddItemToObject(notificatioon,
                          ASSISTANT_STUB_COMMAND,
                          cJSON_CreateString(player_status_change.command));
    /*add params && child object of params*/
    cJSON_AddItemToObject(notificatioon,ASSISTANT_STUB_PARAMS,params);
    cJSON_AddItemToObject(params,
                          ASSISTANT_STUB_VOLUME,
                          cJSON_CreateNumber(player_status_change.player.volume));
    cJSON_AddItemToObject(params,
                          ASSISTANT_STUB_STATUS,
                          cJSON_CreateString(player_status_change.player.status));
    cJSON_AddItemToObject(params,
                          ASSISTANT_STUB_SOURCE,
                          cJSON_CreateString(player_status_change.player.source));
    cJSON_AddItemToObject(params,
                          ASSISTANT_STUB_AUDIO_ID,
                          cJSON_CreateString(player_status_change.player.audioId));
    cJSON_AddItemToObject(params,
                          ASSISTANT_STUB_AUDIO_UID,
                          cJSON_CreateString(player_status_change.player.audioUid));
    cJSON_AddItemToObject(params,
                          ASSISTANT_STUB_AUDIO_SOURCE,
                          cJSON_CreateString(player_status_change.player.audioSource));
    cJSON_AddItemToObject(params,
                          ASSISTANT_STUB_AUDIO_NAME,
                          cJSON_CreateString(player_status_change.player.audioName));
    cJSON_AddItemToObject(params,
                          ASSISTANT_STUB_AUDIO_ANCHOR,
                          cJSON_CreateString(player_status_change.player.audioAnchor));
    cJSON_AddItemToObject(params,
                          ASSISTANT_STUB_AUDIO_ALBUM,
                          cJSON_CreateString(player_status_change.player.audioAlbum));
    cJSON_AddItemToObject(params,
                          ASSISTANT_STUB_PROGRESS,
                          cJSON_CreateNumber(player_status_change.player.progress));
    cJSON_AddItemToObject(params,
                          ASSISTANT_STUB_AUDIO_EXT,
                          cJSON_CreateString(player_status_change.player.audioExt));
    /*transfer cjson to string and send to assistance server*/
    out = cJSON_Print(notificatioon);
    strncpy(send_buffer,out,HUB_CMD_LENGTH_MAX);
    printf("<ASSISTANT_STUB_APP> send msg is %s\n",send_buffer);
    hub_send(&CTX,send_buffer,strlen(send_buffer));
    /*delete cjson object*/
    cJSON_Delete(notificatioon);
    free(out);
    out = NULL ;

    FUNCTION_END
}

static VOID _assistant_stub_json_handle_src_avrcp_cmd(VOID * p_usr_msg)
{
    FUNCTION_BEGIN
    char * out = NULL;
    char send_buffer[HUB_CMD_LENGTH_MAX+1] = {0};
    ASSISTANT_STUB_BT_SRC_AVRCP_CMD_T src_avrcp_cmd = {0};
    memcpy(&src_avrcp_cmd,
          (ASSISTANT_STUB_BT_SRC_AVRCP_CMD_T *)p_usr_msg,
           sizeof(ASSISTANT_STUB_BT_SRC_AVRCP_CMD_T));

    printf("<ASSISTANT_STUB_APP> request is %s \n",src_avrcp_cmd.cmd.request);

    /*create cjson object*/
    cJSON * params = cJSON_CreateObject();
    cJSON * notificatioon = cJSON_CreateObject();
    /*add  jsonrpc && command*/
    cJSON_AddItemToObject(notificatioon,
                          ASSISTANT_STUB_COMMAND,
                          cJSON_CreateString(src_avrcp_cmd.command));
    /*add params && child object of params*/
    cJSON_AddItemToObject(notificatioon,ASSISTANT_STUB_PARAMS,params);
    cJSON_AddItemToObject(params,
                          ASSISTANT_STUB_STATUS,
                          cJSON_CreateString(src_avrcp_cmd.cmd.request));

    /*transfer cjson to string and send to assistance server*/
    out = cJSON_Print(notificatioon);
    strncpy(send_buffer,out,HUB_CMD_LENGTH_MAX);
    printf("<ASSISTANT_STUB_APP> send msg is %s\n",send_buffer);
    hub_send(&CTX,send_buffer,strlen(send_buffer));
    /*delete cjson object*/
    cJSON_Delete(notificatioon);
    free(out);
    out = NULL ;

    FUNCTION_END


}
static VOID _assistant_stub_json_handle_network_status_change(VOID * p_usr_msg)
{
    FUNCTION_BEGIN
    char *out = NULL;
    char send_buffer[HUB_CMD_LENGTH_MAX+1] = {0};
    ASSISTANT_STUB_NETWORK_STATUS_CHANGE_T network_status_change = {0};
    memcpy(&network_status_change, p_usr_msg, sizeof(network_status_change));

    printf("<ASSISTANT_STUB_APP> command is %s \n",network_status_change.command);
    printf("<ASSISTANT_STUB_APP> quantity is %d \n",network_status_change.quantity);
    printf("<ASSISTANT_STUB_APP> status is %s\n",network_status_change.status);
    printf("<ASSISTANT_STUB_APP> ssid is %s\n",network_status_change.ssid);
    printf("<ASSISTANT_STUB_APP> bssid is %s\n",network_status_change.bssid);

    /*create cjson dada*/
    cJSON * params = cJSON_CreateObject();
    cJSON * notificatioon = cJSON_CreateObject();

    /*add command*/
    cJSON_AddItemToObject(notificatioon,
                          ASSISTANT_STUB_COMMAND,
                          cJSON_CreateString(network_status_change.command));
    /*add params && child object of params*/
    cJSON_AddItemToObject(notificatioon,ASSISTANT_STUB_PARAMS,params);
    cJSON_AddItemToObject(params,
                          ASSISTANT_STUB_QUANTITY,
                          cJSON_CreateNumber(network_status_change.quantity));
    cJSON_AddItemToObject(params,
                          ASSISTANT_STUB_STATUS,
                          cJSON_CreateString(network_status_change.status));
	cJSON_AddItemToObject(params,
                          ASSISTANT_STUB_SSID,
                          cJSON_CreateString(network_status_change.ssid));
    cJSON_AddItemToObject(params,
                          ASSISTANT_STUB_BSSID,
                          cJSON_CreateString(network_status_change.bssid));
    /*transfer cjson to string and send to assistance server*/
    out = cJSON_Print(notificatioon);
    strncpy(send_buffer,out,HUB_CMD_LENGTH_MAX);
    printf("<ASSISTANT_STUB_APP> send msg is %s\n",send_buffer);
    hub_send(&CTX,send_buffer,strlen(send_buffer));
    /*delete cjson object*/
    cJSON_Delete(notificatioon);
    free(out);

    FUNCTION_END
}

static VOID _assistant_stub_json_handle_hfp_status_change(VOID * p_usr_msg)
{
    FUNCTION_BEGIN
    char * out = NULL;
    char send_buffer[HUB_CMD_LENGTH_MAX+1] = {0};
    ASSISTANT_STUB_HFP_STATUS_CHANGE_T hfp_status_change = {0};
    memcpy(&hfp_status_change,
          (ASSISTANT_STUB_HFP_STATUS_CHANGE_T *)p_usr_msg,
           sizeof(ASSISTANT_STUB_HFP_STATUS_CHANGE_T));

    printf("<ASSISTANT_STUB_APP> command is %s \n",hfp_status_change.command);
    printf("<ASSISTANT_STUB_APP> status is %s \n",hfp_status_change.status);

    /*create cjson object*/
    cJSON * params = cJSON_CreateObject();
    cJSON * notificatioon = cJSON_CreateObject();
    /*add  jsonrpc && command*/
    cJSON_AddItemToObject(notificatioon,
                          ASSISTANT_STUB_COMMAND,
                          cJSON_CreateString(hfp_status_change.command));
    /*add params && status*/
    cJSON_AddItemToObject(notificatioon,ASSISTANT_STUB_PARAMS,params);
    cJSON_AddItemToObject(params,
                          ASSISTANT_STUB_STATUS,
                          cJSON_CreateString(hfp_status_change.status));
    /*transfer cjson to string and send to assistance server*/
    out = cJSON_Print(notificatioon);
    strncpy(send_buffer,out,HUB_CMD_LENGTH_MAX);
    printf("<ASSISTANT_STUB_APP> send msg is %s\n",send_buffer);
    hub_send(&CTX,send_buffer,strlen(send_buffer));
    /*delete cjson object*/
    cJSON_Delete(notificatioon);
    free(out);
    out = NULL;

    FUNCTION_END
}

static VOID _assistant_stub_json_handle_bluetooth_status_change(VOID * p_usr_msg)
{
    FUNCTION_BEGIN
    char * out = NULL;
    char send_buffer[HUB_CMD_LENGTH_MAX+1] = {0};
    ASSISTANT_STUB_BLUETOOTH_STATUS_CHANGE_T bluetooth_status_change = {0};
    memcpy(&bluetooth_status_change,
          (ASSISTANT_STUB_BLUETOOTH_STATUS_CHANGE_T *)p_usr_msg,
          sizeof(ASSISTANT_STUB_BLUETOOTH_STATUS_CHANGE_T));

    printf("<ASSISTANT_STUB_APP> command is %s \n",bluetooth_status_change.command);
    printf("<ASSISTANT_STUB_APP> status is %d \n",bluetooth_status_change.status);
    printf("<ASSISTANT_STUB_APP> name is %s\n",bluetooth_status_change.name);
    printf("<ASSISTANT_STUB_APP> bt_paired_name is %s\n",bluetooth_status_change.bt_paired_name);

    /*create cjson dada*/
    cJSON * params = cJSON_CreateObject();
    cJSON * notificatioon = cJSON_CreateObject();

    /*add  command*/
    cJSON_AddItemToObject(notificatioon,
                          ASSISTANT_STUB_COMMAND,
                          cJSON_CreateString(bluetooth_status_change.command));
    /*add params && child object of params*/
    cJSON_AddItemToObject(notificatioon,ASSISTANT_STUB_PARAMS,params);
    cJSON_AddItemToObject(params,
                          ASSISTANT_STUB_STATUS,
                          cJSON_CreateString(bluetooth_status_change.status));
    cJSON_AddItemToObject(params,
                          ASSISTANT_STUB_NAME,
                          cJSON_CreateString(bluetooth_status_change.name));
	cJSON_AddItemToObject(params,
                          ASSISTANT_STUB_BT_PAIRED_NAME,
                          cJSON_CreateString(bluetooth_status_change.bt_paired_name));
    /*transfer cjson to string and send to assistance server*/
    out = cJSON_Print(notificatioon);
    strncpy(send_buffer,out,HUB_CMD_LENGTH_MAX);
    printf("<ASSISTANT_STUB_APP> send msg is %s\n",send_buffer);
    hub_send(&CTX,send_buffer,strlen(send_buffer));
    /*delete cjson object*/
    cJSON_Delete(notificatioon);
    free(out);
    out = NULL;

    FUNCTION_END
}

static VOID _assistant_stub_json_handle_get_ap_list_response(VOID * p_usr_msg)
{
    FUNCTION_BEGIN
    char *out = NULL;
    ASSISTANT_STUB_GET_AP_LIST_RESPONSE_T *cmd = p_usr_msg;

    cJSON *root = cJSON_CreateObject();
    cJSON_AddStringToObject(root, ASSISTANT_STUB_COMMAND, cmd->command);
    cJSON_AddNumberToObject(root, ASSISTANT_STUB_ID, cmd->id);
    cJSON_AddNumberToObject(root, ASSISTANT_STUB_FINISH, cmd->finish);

    cJSON *list = cJSON_CreateArray();
    cJSON_AddItemToObject(root, ASSISTANT_STUB_LIST, list);
    cJSON *param;
    for (INT32 i=0; i < cmd->list_num; i++) {
        cJSON_AddItemToArray(list, param = cJSON_CreateObject());
        cJSON_AddStringToObject(param, ASSISTANT_STUB_SSID, cmd->ap_info[i].ssid);
        cJSON_AddStringToObject(param, ASSISTANT_STUB_BSSID, cmd->ap_info[i].bssid);
        cJSON_AddNumberToObject(param, ASSISTANT_STUB_AUTH_MODE, cmd->ap_info[i].auth_mode);
        cJSON_AddNumberToObject(param, ASSISTANT_STUB_LEVEL, cmd->ap_info[i].level);
        cJSON_AddNumberToObject(param, ASSISTANT_STUB_FREQUENCY, cmd->ap_info[i].frequency);
    }

    /*transfer cjson to string and send to assistance server*/
    out = cJSON_Print(root);
    printf("<ASSISTANT_STUB_APP> send msg is %s\n",out);
    hub_send(&CTX, out, strlen(out));

    /*delete cjson object*/
    cJSON_Delete(root);
    free(out);

    FUNCTION_END
}

static VOID _assistant_stub_json_handle_wifi_status(VOID * p_usr_msg)
{
    FUNCTION_BEGIN
    char * out = NULL;
    char send_buffer[HUB_CMD_LENGTH_MAX+1] = {0};
    ASSISTANT_STUB_WIFI_CONNECT_RESPONSE_T wifi_status = {0};
    memcpy(&wifi_status, p_usr_msg, sizeof(wifi_status));

    printf("<ASSISTANT_STUB_APP> id is %d \n",wifi_status.id);
    printf("<ASSISTANT_STUB_APP> wifi_status is %d \n",wifi_status.wifi_status);

    /*create cjson dada*/
    cJSON *notificatioon = cJSON_CreateObject();
    cJSON *result = cJSON_CreateObject();

    /*add params && child object of params*/
    cJSON_AddStringToObject(notificatioon, ASSISTANT_STUB_COMMAND, wifi_status.command);
    cJSON_AddItemToObject(notificatioon,ASSISTANT_STUB_RESULT,result);
    cJSON_AddItemToObject(result,
                          ASSISTANT_STUB_WIFI_STATUS,
                          cJSON_CreateNumber(wifi_status.wifi_status));
    cJSON_AddItemToObject(notificatioon,
                          ASSISTANT_STUB_ID,
                          cJSON_CreateNumber(wifi_status.id));
    /*transfer cjson to string and send to assistance server*/
    out = cJSON_Print(notificatioon);
    strncpy(send_buffer,out,HUB_CMD_LENGTH_MAX);
    printf("<ASSISTANT_STUB_APP> send msg is %s\n",send_buffer);
    hub_send(&CTX,send_buffer,strlen(send_buffer));
    /*delete cjson object*/
    cJSON_Delete(notificatioon);
    free(out);
    out = NULL;

    FUNCTION_END
}

static VOID _assistant_stub_json_handle_button(VOID * p_usr_msg)
{
    FUNCTION_BEGIN
    char * out = NULL;
    char send_buffer[HUB_CMD_LENGTH_MAX+1] = {0};
    ASSISTANT_STUB_BUTTON_T button = {0};
    INT32 i4_ret;
    HANDLE_T h_app = NULL_HANDLE;

    memcpy(&button,
          (ASSISTANT_STUB_BUTTON_T *)p_usr_msg,
           sizeof(ASSISTANT_STUB_BUTTON_T));

    printf("<ASSISTANT_STUB_APP> button.name is %s \n",button.name);

    /*create cjson dada*/
    cJSON * params = cJSON_CreateObject();
    cJSON * notificatioon = cJSON_CreateObject();

    /*add command*/
    cJSON_AddItemToObject(notificatioon,
                          ASSISTANT_STUB_COMMAND,
                          cJSON_CreateString(button.command));

    /*add params && child object of params*/
    cJSON_AddItemToObject(notificatioon,ASSISTANT_STUB_PARAMS,params);
    cJSON_AddItemToObject(params,
                          ASSISTANT_STUB_NAME,
                          cJSON_CreateString(button.name));

    /*transfer cjson to string and send to assistance server*/
    out = cJSON_Print(notificatioon);
    strncpy(send_buffer,out,HUB_CMD_LENGTH_MAX);
    printf("<ASSISTANT_STUB_APP> send msg is %s\n",send_buffer);
    hub_send(&CTX,send_buffer,strlen(send_buffer));
    /*delete cjson object*/
    cJSON_Delete(notificatioon);
    free(out);
    out = NULL;

#if 0
    /*follow smart box behavior ,when config wifi need ignore wifi_setup key*/
    printf("<ASSISTANT_STUB_APP> fg_wifi_setup_status is %d\n",fg_wifi_setup_status);
    if((FALSE == fg_wifi_setup_status)&&(0 == strncmp(button.name,"wifi_setup",strlen(button.name))))
    {
        fg_wifi_setup_status = TRUE;
        printf("<ASSISTANT_STUB_APP> need disconnected wifi:c_net_wlan_disassociate!\n");
        wlan_scan_connect_status.ui4_wlan_timer_counter = 32;//32==WLAN_TIMER_ASSOCIATE_TIMEOUT
        c_net_wlan_disassociate( );

        printf("<ASSISTANT_STUB_APP> fg_wifi_connect_status is %d line = %d\n",fg_wifi_connect_status, __LINE__);
        if(1 == fg_wifi_connect_status)
        {
            fg_wifi_connect_status = 0;
            printf("<ASSISTANT_STUB_APP> ifconfig wlan0 0.0.0.0\n");
            system("ifconfig wlan0 0.0.0.0");
        }

        printf("echo 0 > /dev/wmtWifi\n");
        system("echo 0 > /dev/wmtWifi");

        printf("echo 1 > /dev/wmtWifi \n");
        system("echo 1 > /dev/wmtWifi");

        ASSISTANT_STUB_NETWORK_STATUS_CHANGE_T network_status_change = {0};
		NET_802_11_ASSOCIATE_T _rSavedWlanAssociate={{0}};
		a_cfg_get_wlan_ap(&_rSavedWlanAssociate);
        strncpy(network_status_change.command,"/system/network_status_change",ASSISTANT_STUB_COMMAND_MAX_LENGTH);
        network_status_change.quantity = 0;
        strncpy(network_status_change.status,"disconnect",ASSISTANT_STUB_STATUS_MAX_LENGTH);
        strncpy(network_status_change.ssid,
				_rSavedWlanAssociate.t_Ssid.ui1_aSsid,
				NET_802_11_MAX_LEN_SSID);

#if 0
        /*send  network status change  assistant_stub */
        i4_ret = u_am_get_app_handle_from_name(&h_app, ALI_STUB_THREAD_NAME);
        if(0 != i4_ret)
        {
            printf("<MISC_DHCP> get handle fail!,i4_ret=%ld\n",i4_ret);
        }
        i4_ret = u_app_send_appmsg(h_app,
                                   E_APP_MSG_TYPE_ALI_STUB,
                                   MSG_FROM_WIFI_SETTING,
                                   ALI_STUB_CMD_NETWORK_STATUS_CHANGE,
                                   &network_status_change,
                                   sizeof(ALI_STUB_NETWORK_STATUS_CHANGE_T));
        if(0 == i4_ret)
        {
            printf("<MISC_DHCP> u_app_send_appmsg success !!!\n");
        }
        else
        {
            printf("<MISC_DHCP> u_app_send_appmsg fail !!!\n");
        }
#endif
        /*send  network status change to sm */
        i4_ret = u_am_get_app_handle_from_name(&h_app, SM_THREAD_NAME);
        if(0 != i4_ret)
        {
            printf("<MISC_DHCP> get handle fail!,i4_ret=%ld\n",i4_ret);
        }
        i4_ret = u_app_send_appmsg(h_app,
                                   E_APP_MSG_TYPE_MISC,
                                   MSG_FROM_WIFI_SETTING,
                                   ASSISTANT_STUB_CMD_NETWORK_STATUS_CHANGE,
                                   &network_status_change,
                                   sizeof(ASSISTANT_STUB_NETWORK_STATUS_CHANGE_T));
        if(0 == i4_ret)
        {
            printf("<MISC_DHCP> u_app_send_appmsg success !!!\n");
        }
        else
        {
            printf("<MISC_DHCP> u_app_send_appmsg fail !!!\n");
        }
    }
#endif
    FUNCTION_END
}

static VOID _assistant_stub_json_handle_sm_msg(APPMSG_T * t_app_msg)
{
    FUNCTION_BEGIN
    switch(t_app_msg->ui4_msg_type)
    {
        case ASSISTANT_STUB_CMD_SYSTEM_STATUS_CHANGE:
        {
            _assistant_stub_json_handle_system_status_change((VOID *)(t_app_msg->p_usr_msg));
        }
        break;

        case ASSISTANT_STUB_CMD_GET_SPEAKER_STATUS:
        {
            _assistant_stub_json_handle_respose_speaker_status((VOID *)(t_app_msg->p_usr_msg));
        }
        break;

        case  ASSISTANT_STUB_CMD_NETWORK_STATUS_CHANGE:
        {
            _assistant_stub_json_handle_network_status_change((VOID *)(t_app_msg->p_usr_msg));
        }
        break;
        
        case  ASSISTANT_STUB_CMD_HFP_STATUS_CHANGE:
        {
            _assistant_stub_json_handle_hfp_status_change((VOID *)(t_app_msg->p_usr_msg));
        }
        break;

        default:
        {
            printf("<ASSISTANT_STUB_APP> ERROR SM MSG TYPE!\n");
        }
        break;

    }
    FUNCTION_END
}

static VOID _assistant_stub_json_handle_uri_msg(APPMSG_T * t_app_msg)
{
    FUNCTION_BEGIN
    switch(t_app_msg->ui4_msg_type)
    {
        case ASSISTANT_STUB_CMD_PLAY_DONE:
        {
            _assistant_stub_json_handle_play_done((VOID *)(t_app_msg->p_usr_msg));
        }
        break;

        case ASSISTANT_STUB_CMD_PLAYER_STATUS_CHANGE:
        {
            _assistant_stub_json_handle_player_status_change((VOID *)(t_app_msg->p_usr_msg));
        }
        break;

        default:
        {
            printf("<ASSISTANT_STUB_APP> ERROR URI MSG TYPE!\n");
        }
        break;

    }
    FUNCTION_END
}

static VOID _assistant_stub_json_handle_upg_msg(APPMSG_T * t_app_msg)
{
    FUNCTION_BEGIN
    switch(t_app_msg->ui4_msg_type)
    {
        case ASSISTANT_STUB_CMD_OTA_PROGRESS:
        {
            _assistant_stub_json_handle_ota_progress((VOID *)(t_app_msg->p_usr_msg));
        }
        break;

        default:
        {
            printf("<ASSISTANT_STUB_APP> ERROR URI MSG TYPE!\n");
        }
        break;

    }
    FUNCTION_END
}

static VOID _assistant_stub_json_handle_assistant_stub_msg(APPMSG_T * t_app_msg)
{
    FUNCTION_BEGIN
    switch(t_app_msg->ui4_msg_type)
    {
        default:
        {
            printf("<ASSISTANT_STUB_APP> ERROR ASSISTANT STUB MSG TYPE!\n");
        }
        break;

    }
    FUNCTION_END
}

static VOID _assistant_stub_json_handle_dm_msg(APPMSG_T * t_app_msg)
{
    FUNCTION_BEGIN
    FUNCTION_END
}

static VOID _assistant_stub_json_handle_tts_msg(APPMSG_T * t_app_msg)
{
    FUNCTION_BEGIN
    switch(t_app_msg->ui4_msg_type)
    {
        case  ASSISTANT_STUB_CMD_PLAY_TTS_DONE:
        {
            _assistant_stub_json_handle_play_tts_done((VOID *)(t_app_msg->p_usr_msg));
        }
        break;

        default:
        {
            printf("<ASSISTANT_STUB_APP> ERROR TTS MSG TYPE!\n");
        }
        break;

    }
    FUNCTION_END
}

static VOID _assistant_stub_json_handle_bt_msg(APPMSG_T * t_app_msg)
{
    FUNCTION_BEGIN
    switch(t_app_msg->ui4_msg_type)
    {
        case  ASSISTANT_STUB_CMD_BLUETOOTH_STATUS_CHANGE:
        {
            _assistant_stub_json_handle_bluetooth_status_change((VOID *)(t_app_msg->p_usr_msg));
        }
        break;

        case ASSISTANT_STUB_CMD_PLAYER_STATUS_CHANGE:
        {
            _assistant_stub_json_handle_player_status_change((VOID *)(t_app_msg->p_usr_msg));
        }
        break;

        case ASSISTANT_STUB_CMD_BT_SRC_AVRCP_CMD:
        {
            _assistant_stub_json_handle_src_avrcp_cmd((VOID *)(t_app_msg->p_usr_msg));
        }
        break;

        default:
        {
            printf("<ASSISTANT_STUB_APP> ERROR BT MSG TYPE!\n");
        }
        break;

    }
    FUNCTION_END
}

static VOID _assistant_stub_json_handle_wifi_setting_msg(APPMSG_T * t_app_msg)
{
    FUNCTION_BEGIN
    switch(t_app_msg->ui4_msg_type)
    {
        case ASSISTANT_STUB_CMD_GET_AP_LIST:
            _assistant_stub_json_handle_get_ap_list_response(t_app_msg->p_usr_msg);
            break;
        case  ASSISTANT_STUB_CMD_WIFI_CONNECT:
            _assistant_stub_json_handle_wifi_status((VOID *)(t_app_msg->p_usr_msg));
            break;
        default:
            printf("<ASSISTANT_STUB_APP> ERROR WIFI SETTING MSG TYPE!\n");
            break;

    }
    FUNCTION_END
}

static VOID _assistant_stub_json_handle_user_interface_msg(APPMSG_T * t_app_msg)
{
    FUNCTION_BEGIN
    switch(t_app_msg->ui4_msg_type)
    {
        case ASSISTANT_STUB_CMD_BUTTON:
        {
            _assistant_stub_json_handle_button((VOID *)(t_app_msg->p_usr_msg));
        }
        break;

        default:
        {
            printf("<ASSISTANT_STUB_APP> ERROR USER INTERFACE MSG TYPE!\n");
        }
        break;

    }
    FUNCTION_END
}

static INT32 _assistant_stub_app_init(const CHAR* ps_name, HANDLE_T h_app)
{
    FUNCTION_BEGIN

    INT32    i4_ret;
    HANDLE_T h_thread = NULL_HANDLE;
    UINT8    volume;

    memset(&t_g_assistant_stub_app, 0, sizeof(ASSISTANT_STUB_APP_OBJ_T));

    t_g_assistant_stub_app.h_app=h_app;

    if (t_g_assistant_stub_app.b_app_init_ok)
    {
        return AEER_OK;
    }

    /*create assistance json data recv thread*/
    i4_ret = u_thread_create(&h_thread,
                             ASSISTANT_STUB_JSON_RECV_THREAD_NAME,
                             ASSISTANT_STUB_JSON_RECV_THREAD_STACK_SIZE,
                             ASSISTANT_STUB_JSON_RECV_THREAD_PRIORATY,
                             _assistant_stub_json_recv_thread,
                             0,
                             NULL);
    if (ASSISTANT_STUB_APPR_OK != i4_ret)
    {
        printf("<C4A_STUB_APP> ERR: create _assistant_stub_json_recv_thread failed, i4_ret [%ld],at L%d\r\n", i4_ret, __LINE__);
        return AEER_FAIL;
    }

#ifdef CLI_SUPPORT
    i4_ret = _assistant_stub_cli_attach_cmd_tbl();
    if ((CLIR_NOT_INIT != i4_ret) && (CLIR_OK != i4_ret))
    {
        printf("Err: _assistant_stub_cli_attach_cmd_tbl() failed, ret=%ld\r\n",i4_ret);
        return AEER_FAIL;
    }
    _assistant_stub_set_dbg_level(DBG_INIT_LEVEL_APP_ASSISTANT_STUB);
#endif/* CLI_SUPPORT */

    t_g_assistant_stub_app.b_app_init_ok=TRUE;

    FUNCTION_END

    return ASSISTANT_STUB_APPR_OK;
}

static INT32 _assistant_stub_app_process_msg(HANDLE_T     h_app,
                                               UINT32       ui4_type,
                                               const VOID*  pv_msg,
                                               SIZE_T       z_msg_len,
                                               BOOL         b_paused)
{
    FUNCTION_BEGIN

    UCHAR*       puc_name;
    INT32        i4_ret = ASSISTANT_STUB_APPR_OK;
    APPMSG_T *   app_msg = (APPMSG_T * )pv_msg;

    if (!t_g_assistant_stub_app.b_app_init_ok)
    {
        return AEER_FAIL;
    }

    if (ui4_type < AMB_BROADCAST_OFFSET)
    {
        /* private message */
        switch(ui4_type)
        {
            case E_APP_MSG_TYPE_ASSISTANT_STUB:
            {
                printf("<ASSISTANT_STUB_APP> E_APP_MSG_TYPE_ASSISTANT_STUB\n");
                switch(app_msg->ui4_sender_id)
                {
                    case MSG_FROM_SM:
                    {
                        printf("<ASSISTANT_STUB_APP> MSG_FROM_SM\n");
                        _assistant_stub_json_handle_sm_msg(app_msg);
                    }
                    break;

                    case MSG_FROM_URI:
                    {
                        printf("<ASSISTANT_STUB_APP> MSG_FROM_URI\n");
                        _assistant_stub_json_handle_uri_msg(app_msg);
                    }
                    break;

                    case MSG_FROM_PROMPT:
                    {
                        printf("<ASSISTANT_STUB_APP> MSG_FROM_PROMPT\n");
                        _assistant_stub_json_handle_uri_msg(app_msg);
                    }
                    break;

                    case MSG_FROM_DM:
                    {
                        printf("<ASSISTANT_STUB_APP> MSG_FROM_DM\n");
                        _assistant_stub_json_handle_dm_msg(app_msg);
                    }
                    break;

                    case MSG_FROM_TTS:
                    {
                        printf("<ASSISTANT_STUB_APP> MSG_FROM_TTS\n");
                        _assistant_stub_json_handle_tts_msg(app_msg);
                    }
                    break;

                    case MSG_FROM_BT:
                    {
                        printf("<ASSISTANT_STUB_APP> MSG_FROM_BT\n");
                        _assistant_stub_json_handle_bt_msg(app_msg);
                    }
                    break;

                    case MSG_FROM_WIFI_SETTING:
                    {
                        printf("<ASSISTANT_STUB_APP> MSG_FROM_WIFI_SETTING\n");
                        _assistant_stub_json_handle_wifi_setting_msg(app_msg);
                    }
                    break;

                    case MSG_FROM_UI:
                    {
                        printf("<ASSISTANT_STUB_APP> MSG_FROM_UI\n");
                        _assistant_stub_json_handle_user_interface_msg(app_msg);
                    }
                    break;

                    case MSG_FROM_UPG:
                    {
                        printf("<ASSISTANT_STUB_APP> MSG_FROM_UPG\n");
                        _assistant_stub_json_handle_upg_msg(app_msg);
                    }
                    break;

                    case MSG_FROM_ASSISTANT_STUB:
                    {
                        printf("<ASSISTANT_STUB_APP> MSG_FROM_ASSISTANT_STUB\n");
                        _assistant_stub_json_handle_assistant_stub_msg(app_msg);
                    }
                    break;

                    default:
                    {
                        printf("<ASSISTANT_STUB_APP> ERROR sender id\n");
                    }
                    break;
                }
            }
            break;

            default:
            break;
        }
    }
    else
    {
        switch(ui4_type)
        {
            case E_APP_MSG_TYPE_STATE_MNGR:
            {
                printf("<ASSISTANT_STUB_APP> E_APP_MSG_TYPE_STATE_MNGR\n");
            }
            break;

            default:
            break;
        }
    }

    FUNCTION_END

    return ASSISTANT_STUB_APPR_OK;
}

static INT32 _assistant_stub_app_exit(HANDLE_T h_app, APP_EXIT_MODE_T e_exit_mode)
{
    FUNCTION_BEGIN
    t_g_assistant_stub_app.b_app_init_ok=FALSE;
    FUNCTION_END

    return ASSISTANT_STUB_APPR_OK;
}

VOID a_assistant_stub_register(AMB_REGISTER_INFO_T* pt_reg)
{
    FUNCTION_BEGIN

    if (t_g_assistant_stub_app.b_app_init_ok)
    {
        printf("<ASSISTANT_STUB_APP> a_assistant_stub_register done,just return\n");
        return;
    }

    strncpy(pt_reg->s_name, ASSISTANT_STUB_THREAD_NAME,sizeof(ASSISTANT_STUB_THREAD_NAME));
    pt_reg->t_fct_tbl.pf_init                   = _assistant_stub_app_init;
    pt_reg->t_fct_tbl.pf_exit                   = _assistant_stub_app_exit;
    pt_reg->t_fct_tbl.pf_process_msg            = _assistant_stub_app_process_msg;
    pt_reg->t_desc.ui8_flags = AEE_FLAG_WRITE_CONFIG|AEE_FLAG_WRITE_FLM|AEE_FLAG_WRITE_TSL|AEE_FLAG_WRITE_SVL;
    pt_reg->t_desc.t_thread_desc.z_stack_size = ASSISTANT_STUB_STACK_SZ;
    pt_reg->t_desc.t_thread_desc.ui1_priority = ASSISTANT_STUB_THREAD_PRIORITY;
    pt_reg->t_desc.t_thread_desc.ui2_num_msgs = ASSISTANT_STUB_THREAD_MSG_NUM;
    pt_reg->t_desc.ui4_app_group_id = 0;
    pt_reg->t_desc.ui4_app_id = 0;
    pt_reg->t_desc.ui2_msg_count = ASSISTANT_STUB_MSGS_COUNT;
    pt_reg->t_desc.ui2_max_msg_size = ASSISTANT_STUB_MAX_MSGS_SIZE;

    FUNCTION_END
}
