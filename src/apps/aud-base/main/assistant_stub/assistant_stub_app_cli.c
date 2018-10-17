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
 * $RCSfile: assistant_stub_app_cli.c $
 * $Revision:0.1
 * $Date: 2017/01/09
 * $Author:pingan.liu
 * $CCRevision: $
 * $SWAuthor:  $
 * $MD5HEX: $
 *
 * Description:assistance stub app cli cmd


 *---------------------------------------------------------------------------*/
/*-----------------------------------------------------------------------------
                    include files
-----------------------------------------------------------------------------*/
#ifdef CLI_SUPPORT
#include "assistant_stub_app_cli.h"
#include "u_app_def.h"

static UINT16 ui2_g_assistant_stub_dbg_level = DBG_INIT_LEVEL_APP_ASSISTANT_STUB;

UINT16 _assistant_stub_get_dbg_level(VOID)
{
    return (ui2_g_assistant_stub_dbg_level | DBG_LAYER_APP);
}

VOID _assistant_stub_set_dbg_level(UINT16 ui2_db_level)
{
    ui2_g_assistant_stub_dbg_level = ui2_db_level;
}

static INT32 _assistant_stub_cli_set_dbg_level (INT32 i4_argc, const CHAR ** pps_argv)
{
    UINT16 ui2_dbg_level;
    INT32  i4_ret;

    i4_ret = u_cli_parse_dbg_level(i4_argc, pps_argv, &ui2_dbg_level);
    if (CLIR_OK == i4_ret)
	{
        _assistant_stub_set_dbg_level(ui2_dbg_level);
    }

    return i4_ret;
}

static INT32 _assistant_stub_cli_get_dbg_level (INT32 i4_argc, const CHAR ** pps_argv)
{
    INT32  i4_ret;

    i4_ret = u_cli_show_dbg_level(_assistant_stub_get_dbg_level());

    return i4_ret;
}

static INT32 _assistant_stub_cli_play(INT32 i4_argc, const CHAR ** pps_argv)
{
	FUNCTION_BEGIN
	INT32	 i4_ret;
	HANDLE_T h_app = NULL_HANDLE;
	ASSISTANT_STUB_PLAY_T play={0};

	strncpy(play.command,pps_argv[1],ASSISTANT_STUB_COMMAND_MAX_LENGTH);
	strncpy(play.uri,pps_argv[2],ASSISTANT_STUB_URI_MAX_LENGTH);
	strncpy(play.audioId,pps_argv[3],ASSISTANT_STUB_AUDIO_ID_MAX_LENGTH);
	strncpy(play.audioSource,pps_argv[4],ASSISTANT_STUB_AUDIO_SOURCE_MAX_LENGTH);
	strncpy(play.audioName,pps_argv[5],ASSISTANT_STUB_AUDIO_NAME_MAX_LENGTH);
	strncpy(play.audioAnchor,pps_argv[6],ASSISTANT_STUB_AUDIO_ANCHOR_MAX_LENGTH);
	strncpy(play.audioAlbum,pps_argv[7],ASSISTANT_STUB_AUDIO_ALBUM_MAX_LENGTH);
	play.progress = atoi(pps_argv[8]);

	i4_ret = u_am_get_app_handle_from_name(&h_app, PB_URI_THREAD_NAME);
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

static INT32 _assistant_stub_cli_play_voice_prompt(INT32 i4_argc, const CHAR ** pps_argv)
{
	FUNCTION_BEGIN
	INT32	 i4_ret;
	HANDLE_T h_app = NULL_HANDLE;
	ASSISTANT_STUB_PLAY_VOICE_PROMPT_T play_voice_prompt={0};

	strncpy(play_voice_prompt.command,pps_argv[1],ASSISTANT_STUB_COMMAND_MAX_LENGTH);
	strncpy(play_voice_prompt.uri,pps_argv[2],ASSISTANT_STUB_URI_MAX_LENGTH);
	play_voice_prompt.volume= atoi(pps_argv[3]);
	strncpy(play_voice_prompt.type,pps_argv[4],ASSISTANT_STUB_TYPE_LENGTH);
	play_voice_prompt.feedback = atoi(pps_argv[5]);

	i4_ret = u_am_get_app_handle_from_name(&h_app, PB_URI_THREAD_NAME);
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


static INT32 _assistant_stub_cli_play_tts(INT32 i4_argc, const CHAR ** pps_argv)
{
	FUNCTION_BEGIN
	INT32	 i4_ret;
	HANDLE_T h_app = NULL_HANDLE;
	ASSISTANT_STUB_PLAY_TTS_T play_tts={0};

	play_tts.tts_id = 1;
	play_tts.need_mix= atoi(pps_argv[1]);

	i4_ret = u_am_get_app_handle_from_name(&h_app, PB_TTS_THREAD_NAME);
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

static INT32 _assistant_stub_cli_play_prev_audio(INT32 i4_argc, const CHAR ** pps_argv)
{
	FUNCTION_BEGIN
	INT32	 i4_ret;
	HANDLE_T h_app = NULL_HANDLE;

	ASSISTANT_STUB_PLAY_PREV_AUDIO_T play_prev_audio={0};

	strncpy(play_prev_audio.command,pps_argv[1],ASSISTANT_STUB_COMMAND_MAX_LENGTH);

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


static INT32 _assistant_stub_cli_play_next_audio(INT32 i4_argc, const CHAR ** pps_argv)
{
	FUNCTION_BEGIN
	INT32	 i4_ret;
	HANDLE_T h_app = NULL_HANDLE;

	ASSISTANT_STUB_PLAY_NEXT_AUDIO_T play_next_audio={0};

	strncpy(play_next_audio.command,pps_argv[1],ASSISTANT_STUB_COMMAND_MAX_LENGTH);

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


static INT32 _assistant_stub_cli_set_volume(INT32 i4_argc, const CHAR ** pps_argv)
{
	FUNCTION_BEGIN
	INT32	 i4_ret;
	HANDLE_T h_app = NULL_HANDLE;
	ASSISTANT_STUB_SET_VOLUME_T set_volume={0};

	strncpy(set_volume.command,ASSISTANT_STUB_SET_VOLUME,ASSISTANT_STUB_COMMAND_MAX_LENGTH);

	set_volume.volume= atoi(pps_argv[1]);
	printf("<ASSISTANT_STUB_APP> set_volume is %d",set_volume.volume);

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
                                sizeof(set_volume));
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

static INT32 _assistant_stub_cli_test_led(INT32 i4_argc, const CHAR ** pps_argv)
{
	FUNCTION_BEGIN

	int action=0;
	action = atoi(pps_argv[1]);
	printf("<ASSISTANT_STUB_APP> action is %d \n",action);

#if CONFIG_SUPPORT_APP_TONLY_LED
	u_set_led_type(action);
#endif

	FUNCTION_END
}

static INT32 _assistant_stub_cli_set_system_status(INT32 i4_argc, const CHAR ** pps_argv)
{
	FUNCTION_BEGIN
	INT32	 i4_ret;
	HANDLE_T h_app = NULL_HANDLE;
	ASSISTANT_STUB_SET_SYSTEM_STATUS_T set_system_status={0};

	strncpy(set_system_status.command,pps_argv[1],ASSISTANT_STUB_COMMAND_MAX_LENGTH);
	strncpy(set_system_status.status,pps_argv[2],ASSISTANT_STUB_STATUS_MAX_LENGTH);

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


static INT32 _assistant_stub_cli_set_bt_name(INT32 i4_argc, const CHAR ** pps_argv)
{
	FUNCTION_BEGIN
	INT32	 i4_ret;
	HANDLE_T h_app = NULL_HANDLE;
	ASSISTANT_STUB_SET_BT_NAME_T set_bt_name={0};

	strncpy(set_bt_name.command,pps_argv[1],ASSISTANT_STUB_COMMAND_MAX_LENGTH);
	strncpy(set_bt_name.name,pps_argv[2],ASSISTANT_STUB_BT_NAME_MAX_LENGTH);

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

static INT32 _assistant_stub_cli_wifi_connect(INT32 i4_argc, const CHAR ** pps_argv)
{
	FUNCTION_BEGIN
	INT32	 i4_ret;
	HANDLE_T h_app = NULL_HANDLE;
	ASSISTANT_STUB_WIFI_CONNECT_T wifi_connect={0};

	strncpy(wifi_connect.command,pps_argv[1],ASSISTANT_STUB_COMMAND_MAX_LENGTH);
	strncpy(wifi_connect.ssid,pps_argv[2],ASSISTANT_STUB_SSID_MAX_LENGTH);
	strncpy(wifi_connect.password,pps_argv[3],ASSISTANT_STUB_PASSWORD_MAX_LENGTH);
    wifi_connect.auth_mode = atoi(pps_argv[4]);
	wifi_connect.id = atoi(pps_argv[5]);

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

static INT32 _assistant_stub_cli_wifi_setup_result(INT32 i4_argc, const CHAR ** pps_argv)
{
	FUNCTION_BEGIN
	INT32	 i4_ret;
	HANDLE_T h_app = NULL_HANDLE;
	ASSISTANT_STUB_WIFI_SETUP_RESULT_T wifi_setup_result={0};

	strncpy(wifi_setup_result.command,pps_argv[1],ASSISTANT_STUB_COMMAND_MAX_LENGTH);
	wifi_setup_result.result = atoi(pps_argv[2]);

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


static INT32 _assistant_stub_cli_speech_start(INT32 i4_argc, const CHAR ** pps_argv)
{
	FUNCTION_BEGIN
	INT32	 i4_ret;
	HANDLE_T h_app = NULL_HANDLE;
	ASSISTANT_STUB_SPEECH_START_T speech_start={0};

	strncpy(speech_start.command,pps_argv[1],ASSISTANT_STUB_COMMAND_MAX_LENGTH);
	FUNCTION_END
}

static INT32 _assistant_stub_cli_speech_process(INT32 i4_argc, const CHAR ** pps_argv)
{
	FUNCTION_BEGIN
	INT32	 i4_ret;
	HANDLE_T h_app = NULL_HANDLE;
	ASSISTANT_STUB_SPEECH_PROCESS_T speech_process={0};

	strncpy(speech_process.command,pps_argv[1],ASSISTANT_STUB_COMMAND_MAX_LENGTH);
	FUNCTION_END
}


static INT32 _assistant_stub_cli_speech_feedback(INT32 i4_argc, const CHAR ** pps_argv)
{
	FUNCTION_BEGIN
	INT32	 i4_ret;
	HANDLE_T h_app = NULL_HANDLE;
	ASSISTANT_STUB_SPEECH_FEEDBACK_T speech_feedback={0};

	strncpy(speech_feedback.command,pps_argv[1],ASSISTANT_STUB_COMMAND_MAX_LENGTH);
	FUNCTION_END
}

static INT32 _assistant_stub_cli_speech_finish(INT32 i4_argc, const CHAR ** pps_argv)
{
	FUNCTION_BEGIN
	INT32	 i4_ret;
	HANDLE_T h_app = NULL_HANDLE;
	ASSISTANT_STUB_SPEECH_FINISH_T speech_finish={0};

	strncpy(speech_finish.command,pps_argv[1],ASSISTANT_STUB_COMMAND_MAX_LENGTH);
	FUNCTION_END
}


static INT32 _assistant_stub_cli_pause(INT32 i4_argc, const CHAR ** pps_argv)
{
	FUNCTION_BEGIN
	INT32	 i4_ret;
	HANDLE_T h_app = NULL_HANDLE;
	ASSISTANT_STUB_PAUSE_T pause={0};

	strncpy(pause.command,pps_argv[1],ASSISTANT_STUB_COMMAND_MAX_LENGTH);

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


static INT32 _assistant_stub_cli_resume(INT32 i4_argc, const CHAR ** pps_argv)
{
	FUNCTION_BEGIN
	INT32	 i4_ret;
	HANDLE_T h_app = NULL_HANDLE;
	ASSISTANT_STUB_RESUME_T resume={0};

	strncpy(resume.command,pps_argv[1],ASSISTANT_STUB_COMMAND_MAX_LENGTH);

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


static INT32 _assistant_stub_cli_get_speaker_status(INT32 i4_argc, const CHAR ** pps_argv)
{
	FUNCTION_BEGIN
	INT32	 i4_ret;
	HANDLE_T h_app = NULL_HANDLE;
	ASSISTANT_STUB_GET_SPEAKER_STATUS_T get_speaker_status={0};

	strncpy(get_speaker_status.command,pps_argv[1],ASSISTANT_STUB_COMMAND_MAX_LENGTH);
	get_speaker_status.id = atoi(pps_argv[2]);

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

static INT32 _assistant_stub_cli_ota_upgrade(INT32 i4_argc, const CHAR ** pps_argv)
{
	FUNCTION_BEGIN
	INT32	 i4_ret;
	HANDLE_T h_app = NULL_HANDLE;
	ASSISTANT_STUB_OTA_UPGRADE_T ota_upgrade={0};

	strncpy(ota_upgrade.command,pps_argv[1],ASSISTANT_STUB_COMMAND_MAX_LENGTH);

	strncpy(ota_upgrade.ota_url,pps_argv[2],ASSISTANT_STUB_URL_MAX_LENGTH);

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

static VOID _assistant_stub_cli_adjust_progress(INT32 i4_argc, const CHAR ** pps_argv)
{
	FUNCTION_BEGIN
	INT32	 i4_ret;
	HANDLE_T h_app = NULL_HANDLE;
	ASSISTANT_STUB_ADJUST_PROGRESS_T adjust_progress={0};

	strncpy(adjust_progress.command, pps_argv[1], ASSISTANT_STUB_COMMAND_MAX_LENGTH);

	adjust_progress.progress = atoi(pps_argv[2]);

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

static VOID _assistant_stub_cli_stop(INT32 i4_argc, const CHAR ** pps_argv)
{
	FUNCTION_BEGIN
	INT32	 i4_ret;
	HANDLE_T h_app = NULL_HANDLE;
	ASSISTANT_STUB_STOP_T stop={0};

	strncpy(stop.command, pps_argv[1], ASSISTANT_STUB_COMMAND_MAX_LENGTH);

	i4_ret = u_am_get_app_handle_from_name(&h_app, SM_THREAD_NAME);
	if(ASSISTANT_STUB_APPR_OK != i4_ret)
	{
		printf("<ASSISTANT_STUB_APP> get handle fail!,i4_ret=%ld\n",i4_ret);
	}
    i4_ret = u_app_send_appmsg(h_app,
                               E_APP_MSG_TYPE_ASSISTANT_STUB,
                               MSG_FROM_ASSISTANT_STUB,
                               ASSISTANT_STUB_CMD_ADJUST_PROGRESS,
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

extern INT32 _assistant_stub_cli_button(INT32 i4_argc, const CHAR ** pps_argv);

/* command table */
static CLI_EXEC_T _assistant_stub_cmd_tbl[] =
{
    {
        CLI_GET_DBG_LVL_STR,
        NULL,
        _assistant_stub_cli_get_dbg_level,
        NULL,
        CLI_GET_DBG_LVL_HELP_STR,
        CLI_GUEST
    },
    {
        CLI_SET_DBG_LVL_STR,
        NULL,
        _assistant_stub_cli_set_dbg_level,
        NULL,
        CLI_SET_DBG_LVL_HELP_STR,
        CLI_GUEST
    },
    {
        "play",
        "play",
        _assistant_stub_cli_play,
        NULL,
        "[assistant_stub]_assistant_stub_cli_play",
        CLI_GUEST
    },
    {
        "play_voice_prompt",
        "play_voice_prompt",
        _assistant_stub_cli_play_voice_prompt,
        NULL,
        "[assistant_stub]_assistant_stub_cli_play_voice_prompt",
        CLI_GUEST
    },
    {
        "play_tts",
        "playtts",
        _assistant_stub_cli_play_tts,
        NULL,
        "[assistant_stub]_assistant_stub_cli_play_tts",
        CLI_GUEST
    },
    {
        "play_prev_audio",
        "play_prev_audio",
        _assistant_stub_cli_play_prev_audio,
        NULL,
        "[assistant_stub]_assistant_stub_cli_play_prev_audio",
        CLI_GUEST
    },
    {
        "play_next_audio",
        "play_next_audio",
        _assistant_stub_cli_play_next_audio,
        NULL,
        "[assistant_stub]_assistant_stub_cli_play_next_audio",
        CLI_GUEST
    },
    {
        "set_volume",
        "setvolume",
        _assistant_stub_cli_set_volume,
        NULL,
        "[assistant_stub]_assistant_stub_cli_set_volume",
        CLI_GUEST
    },
    {
        "test_led",
        "testled",
        _assistant_stub_cli_test_led,
        NULL,
        "[assistant_stub]_assistant_stub_cli_test_led",
        CLI_GUEST
    },
    {
        "set_system_status",
        "set_system_status",
        _assistant_stub_cli_set_system_status,
        NULL,
        "[assistant_stub]_assistant_stub_cli_set_system_status",
        CLI_GUEST
    },
    {
        "set_bt_name",
        "set_bt_name",
        _assistant_stub_cli_set_bt_name,
        NULL,
        "[assistant_stub]_assistant_stub_cli_set_bt_name",
        CLI_GUEST
    },
    {
        "wifi_connect",
        "wifi_connect",
        _assistant_stub_cli_wifi_connect,
        NULL,
        "[assistant_stub]_assistant_stub_cli_wifi_connect",
        CLI_GUEST
    },
    {
        "wifi_setup_result",
        "wifi_setup_result",
        _assistant_stub_cli_wifi_setup_result,
        NULL,
        "[assistant_stub]_assistant_stub_cli_wifi_setup_result",
        CLI_GUEST
    },
    {
        "speech_start",
        "speech_start",
        _assistant_stub_cli_speech_start,
        NULL,
        "[assistant_stub]_assistant_stub_cli_speech_start",
        CLI_GUEST
    },
    {
        "speech_process",
        "speech_process",
        _assistant_stub_cli_speech_process,
        NULL,
        "[assistant_stub]_assistant_stub_cli_speech_process",
        CLI_GUEST
    },
    {
        "speech_feedback",
        "speech_feedback",
        _assistant_stub_cli_speech_feedback,
        NULL,
        "[assistant_stub]_assistant_stub_cli_speech_feedback",
        CLI_GUEST
    },
    {
        "speech_finish",
        "speech_finish",
        _assistant_stub_cli_speech_finish,
        NULL,
        "[assistant_stub]_assistant_stub_cli_speech_finish",
        CLI_GUEST
    },
    {
        "pause",
        "pause",
        _assistant_stub_cli_pause,
        NULL,
        "[assistant_stub]_assistant_stub_cli_pause",
        CLI_GUEST
    },
    {
        "resume",
        "resume",
        _assistant_stub_cli_resume,
        NULL,
        "[assistant_stub]_assistant_stub_cli_resume",
        CLI_GUEST
    },
    {
        "get_speaker_status",
        "get_speaker_status",
        _assistant_stub_cli_get_speaker_status,
        NULL,
        "[assistant_stub]_assistant_stub_cli_get_speaker_status",
        CLI_GUEST
    },
    {
        "ota_upgrade",
        "ota_upgrade",
        _assistant_stub_cli_ota_upgrade,
        NULL,
        "[assistant_stub]_assistant_stub_cli_ota_upgrade",
        CLI_GUEST
    },
    {
        "adjust_progress",
        "adjust_progress",
        _assistant_stub_cli_adjust_progress,
        NULL,
        "[assistant_stub]_assistant_stub_cli_adjust_progress",
        CLI_GUEST
    },
    {
        "button",
        "button",
        _assistant_stub_cli_button,
        NULL,
        "[assistant_stub]_assistant_stub_cli_button",
        CLI_GUEST
    },
    {
        "stop",
        "stop",
        _assistant_stub_cli_stop,
        NULL,
        "[assistant_stub]_assistant_stub_cli_stop",
        CLI_GUEST
    },
    END_OF_CLI_CMD_TBL
};

static CLI_EXEC_T _assistant_stub_root_cmd_tbl[] =
{
    {
		"assistant_stub",
        "assistantstub",
        NULL,
        _assistant_stub_cmd_tbl,
        "assistant_stub commands",
        CLI_GUEST
    },
    END_OF_CLI_CMD_TBL
};

INT32 _assistant_stub_cli_attach_cmd_tbl(VOID)
{
    return (u_cli_attach_cmd_tbl(_assistant_stub_root_cmd_tbl, CLI_CAT_APP, CLI_GRP_GUI));
}

#endif
