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
                   include files
-----------------------------------------------------------------------------*/
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <pthread.h>
#include <sys/time.h>


/* application level */
#include "u_amb.h"
#include "u_am.h"
#include "u_acfg.h"
#include "u_timerd.h"
#include "u_assistant_stub.h"
#include "Interface.h"
#include "u_aee.h"
#include "u_cli.h"
#include "u_dbg.h"
#include "u_os.h"
#include "u_appman.h"
#include "u_sm.h"
#include "u_app_thread.h"

/*    btmw   */
#include "c_bt_mw_a2dp_snk.h"
#include "bt_a2dp_alsa_playback.h"
#include "c_bt_mw_gap.h"
#include "u_bt_mw_common.h"
#include "mtk_bt_service_gap_wrapper.h"
#include "mtk_bt_service_a2dp_wrapper.h"
#include "mtk_bt_service_avrcp_wrapper.h"
#include "mtk_bt_service_hfclient_wrapper.h"

/* private */
#include "u_bluetooth_audio.h"
#include "bluetooth_audio_proc.h"
#include "bluetooth_audio_key_proc.h"
#include "bluetooth_audio.h"
#include "bluetooth_audio_avrcp.h"
#if CONFIG_SUPPORT_BT_HFP
#include "bluetooth_hfp.h"
#include "bluetooth_hfp_proc.h"
#endif /*CONFIG_SUPPORT_BT_HFP*/

#if (CONFIG_APP_SUPPORT_BLE_GATT_CLIENT || CONFIG_APP_SUPPORT_BLE_GATT_SERVER)
#include "bluetooth_gatt.h"
#endif


/*-----------------------------------------------------------------------------
 * structure, constants, macro definitions
 *---------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------
 * variable declarations
 *---------------------------------------------------------------------------*/
CHAR g_cur_avrcp_addr[18];
BT_AVRCP_MEDIA_INFO g_t_cur_media_info;
BT_AVRCP_PLAYER_STATUS g_t_src_player_status = {0};
#if CONFIG_APP_SUPPORT_BT_SRC_MODE
BT_AVRCP_PLAYER_MEDIA_INFO g_t_src_medialnfo = {0};
ASSISTANT_STUB_BT_SRC_AVRCP_CMD_T  g_t_bt_src_avrcp_cmd;
#endif /*CONFIG_APP_SUPPORT_BT_SRC_MODE*/

extern BT_AUD_BLUETOOTH_STATUS_T g_t_bt_aud_sta;
extern BT_A2DP_ROLE g_cur_a2dp_role;
extern BT_AUD_OBJ_T g_t_bt_aud;
extern ASSISTANT_STUB_BLUETOOTH_STATUS_CHANGE_T g_t_bt_aud_bt_status;
#if CONFIG_SUPPORT_BT_HFP
extern BT_HFP_STATUS_T g_t_bt_hfp_sta;
extern BT_HFP_OBJ_T g_t_g_bt_hfp;
extern BT_HFP_CBK_STATUS_T g_t_bt_hfp_cbk_sta;
#endif /*CONFIG_SUPPORT_BT_HFP*/

/*-----------------------------------------------------------------------------
 * private methods declarations
 *---------------------------------------------------------------------------*/
INT32 _bluetooth_set_absolute_volume(UINT8 ui1_value);
INT32 _bluetooth_set_play_status(BT_AVRCP_PLAY_STATUS e_playstatus);


 VOID _bluetooth_avrcp_event_cbk_fct(BT_AVRCP_EVENT_PARAM *avrcp_event)
 {
	 BT_AUD_MSG_T t_msg = {0};
	 BT_AUD_FUNC_ENTRY();

	 if (NULL == avrcp_event)
	 {
		 DBG_ERROR((BTAUD_TAG"avrcp_event is NULL\n"));
		 return;
	 }

	 DBG_ERROR((BTAUD_TAG"AVRCP handle cb, avrcp_event = %d.\r\n",
						 avrcp_event->event));

	 switch (avrcp_event->event)
	 {
		 case BT_AVRCP_EVENT_CONNECTED: 	  /* AVRCP connected */
		 case BT_AVRCP_EVENT_DISCONNECTED:	  /* AVRCP disconnected */
		 {
			 strncpy(g_cur_avrcp_addr,
					 avrcp_event->addr,
					 MAX_BDADDR_LEN);

			 strncpy(t_msg.ui4_str1,
					 avrcp_event->addr,
					 MAX_BDADDR_LEN);
		 }
		 break;

		 case BT_AVRCP_EVENT_TRACK_CHANGE:	  /* remote trach change */
		 {
			 memset(&g_t_cur_media_info, 0x0, sizeof(BT_AVRCP_MEDIA_INFO));
			 memcpy(&g_t_cur_media_info, &(avrcp_event->data.track_change.element_attr), sizeof(BT_AVRCP_MEDIA_INFO));
		 }
		 break;

		 case BT_AVRCP_EVENT_POS_CHANGE:	  /* remote position change */
		 {
			 t_msg.ui4_data1= avrcp_event->data.pos_change.song_len;
			 t_msg.ui4_data3 = avrcp_event->data.pos_change.song_pos;
		 }
		 break;

		 case BT_AVRCP_EVENT_PLAY_STATUS_CHANGE:	  /* remote play status change */
		 {
			 t_msg.ui4_data1 = avrcp_event->data.play_status_change.play_status;
		 }
		 break;

		 case BT_AVRCP_EVENT_VOLUME_CHANGE: 	/* remote absolute volume change */
		 {
			 t_msg.ui4_data2 = avrcp_event->data.volume_change.abs_volume;
		 }
		 break;

		 case BT_AVRCP_EVENT_SET_VOLUME_REQ:	/* remote set local absolute volume */
		 {
			 t_msg.ui4_data2 = avrcp_event->data.set_vol_req.abs_volume;
		 }
		 break;
 // 		 btmw_rpc_test_avrcp_handle_set_volume_req(&avrcp_event->data.set_vol_req);
 // 		 break;
		 case BT_AVRCP_EVENT_PASSTHROUGH_CMD_REQ:	/* remote send passthrough command */
		 {
			 t_msg.ui4_data1 = avrcp_event->data.passthrough_cmd_req.cmd_type;
			 t_msg.ui4_data2 = avrcp_event->data.passthrough_cmd_req.action;
		 }
		 break;

		 default:
			 break;
	 }

	 t_msg.ui4_msg_type  = BT_AUD_PTE_MSG_BT;
	 t_msg.ui4_msg_type_profile = BT_AUD_MSG_BT_AVRCP_PROFILE;
	 t_msg.ui4_msg_id	 = avrcp_event->event;

	 _bluetooth_send_msg_to_itself(E_APP_MSG_TYPE_BLUETOOTH, &t_msg);
 }

INT32 _bluetooth_avrcp_init(VOID)
 {
	 INT32 i4_ret = BT_AUD_FAIL;
	 char pv_tag[2]={0};

	 BT_AUD_FUNC_ENTRY();

	 i4_ret = a_mtkapi_avrcp_register_callback(_bluetooth_avrcp_event_cbk_fct,(void*)pv_tag);
	 DBG_ERROR((BTAUD_TAG"avrcp register callback complete. \n"));
	 BT_AUD_CHK_FAIL_RET(a_mtkapi_avrcp_register_callback, i4_ret, BT_AUD_FAIL);

	 BT_AUD_FUNC_EXIT();
	 return BT_AUD_OK;
 }

 INT32 _bluetooh_send_avrcp_pause_proc(VOID)
 {
	 INT32 i4_ret = BT_AUD_OK;

	 DBG_ERROR((BTAUD_TAG"send AVRCP(PAUSE) cmd to MW.\r\n"));
	 BT_AUD_FUNC_ENTRY();

	 if (TRUE == g_t_bt_aud_sta.b_bt_open_player)
	 {
		i4_ret = a_mtkapi_a2dp_sink_stop_player();
	 	BT_AUD_CHK_FAIL(a_mtkapi_a2dp_sink_start_player, i4_ret);

		g_t_bt_aud_sta.b_bt_open_player = FALSE;
	 }

	 BT_AVRCP_KEY_STATE action;
	 action = BT_AVRCP_KEY_STATE_PRESS;
	 i4_ret = a_mtkapi_avrcp_send_passthrough_cmd(g_cur_avrcp_addr, BT_AVRCP_CMD_TYPE_PAUSE, action);
	 BT_AUD_CHK_FAIL_RET(a_mtkapi_avrcp_send_passthrough_cmd, i4_ret, BT_AUD_FAIL);

	 //sleep(1);

	 action = BT_AVRCP_KEY_STATE_RELEASE;
	 i4_ret = a_mtkapi_avrcp_send_passthrough_cmd(g_cur_avrcp_addr, BT_AVRCP_CMD_TYPE_PAUSE, action);
	 BT_AUD_CHK_FAIL_RET(a_mtkapi_avrcp_send_passthrough_cmd, i4_ret, BT_AUD_FAIL);

	 BT_AUD_FUNC_EXIT();
	 return BT_AUD_OK;
 }

 INT32 _bluetooth_send_avrcp_play_proc(VOID)
 {
	 INT32	  i4_ret = BT_AUD_OK;
	 BT_AVRCP_KEY_STATE action;

	 DBG_ERROR((BTAUD_TAG"send AVRCP(PLAY) cmd to MW.\r\n"));
	 BT_AUD_FUNC_ENTRY();

	 if (FALSE == g_t_bt_aud_sta.b_bt_open_player)
	 {
		i4_ret = a_mtkapi_a2dp_sink_start_player();
	 	BT_AUD_CHK_FAIL(a_mtkapi_a2dp_sink_start_player, i4_ret);

		g_t_bt_aud_sta.b_bt_open_player = TRUE;
	 }

	 action = BT_AVRCP_KEY_STATE_PRESS;
	 i4_ret = a_mtkapi_avrcp_send_passthrough_cmd(g_cur_avrcp_addr, BT_AVRCP_CMD_TYPE_PLAY, action);
	 BT_AUD_CHK_FAIL_RET(a_mtkapi_avrcp_send_passthrough_cmd, i4_ret, BT_AUD_FAIL);

	 //sleep(1);

	 action = BT_AVRCP_KEY_STATE_RELEASE;
	 i4_ret = a_mtkapi_avrcp_send_passthrough_cmd(g_cur_avrcp_addr, BT_AVRCP_CMD_TYPE_PLAY, action);
	 BT_AUD_CHK_FAIL_RET(a_mtkapi_avrcp_send_passthrough_cmd, i4_ret, BT_AUD_FAIL);

	 BT_AUD_FUNC_EXIT();
	 return BT_AUD_OK;
 }
 INT32 _bluetooth_send_avrcp_next_proc(VOID)
 {
	 INT32	  i4_ret = BT_AUD_OK;
	 BT_AVRCP_KEY_STATE action;

	 DBG_ERROR((BTAUD_TAG"send AVRCP(NEXT) cmd to MW.\r\n"));
	 BT_AUD_FUNC_ENTRY();

	 if (FALSE == g_t_bt_aud_sta.b_bt_open_player)
	 {
		i4_ret = a_mtkapi_a2dp_sink_start_player();
	 	BT_AUD_CHK_FAIL(a_mtkapi_a2dp_sink_start_player, i4_ret);

		g_t_bt_aud_sta.b_bt_open_player = TRUE;
	 }

	 action = BT_AVRCP_KEY_STATE_PRESS;
	 i4_ret = a_mtkapi_avrcp_send_passthrough_cmd(g_cur_avrcp_addr, BT_AVRCP_CMD_TYPE_FWD, action);
	 BT_AUD_CHK_FAIL_RET(a_mtkapi_avrcp_send_passthrough_cmd, i4_ret, BT_AUD_FAIL);

	 //sleep(1);

	 action = BT_AVRCP_KEY_STATE_RELEASE;
	 i4_ret = a_mtkapi_avrcp_send_passthrough_cmd(g_cur_avrcp_addr, BT_AVRCP_CMD_TYPE_FWD, action);
	 BT_AUD_CHK_FAIL_RET(a_mtkapi_avrcp_send_passthrough_cmd, i4_ret, BT_AUD_FAIL);

	 BT_AUD_FUNC_EXIT();
	 return BT_AUD_OK;
 }

 INT32 _bluetooth_send_avrcp_prev_proc(VOID)
 {
	 INT32	  i4_ret = BT_AUD_OK;
	 BT_AVRCP_KEY_STATE action;

	 DBG_ERROR((BTAUD_TAG"send AVRCP(PREV) cmd to MW.\r\n"));
	 BT_AUD_FUNC_ENTRY();

	 if (FALSE == g_t_bt_aud_sta.b_bt_open_player)
	 {
		i4_ret = a_mtkapi_a2dp_sink_start_player();
	 	BT_AUD_CHK_FAIL(a_mtkapi_a2dp_sink_start_player, i4_ret);

		g_t_bt_aud_sta.b_bt_open_player = TRUE;
	 }

	 action = BT_AVRCP_KEY_STATE_PRESS;
	 i4_ret = a_mtkapi_avrcp_send_passthrough_cmd(g_cur_avrcp_addr, BT_AVRCP_CMD_TYPE_BWD, action);
	 BT_AUD_CHK_FAIL_RET(a_mtkapi_avrcp_send_passthrough_cmd, i4_ret, BT_AUD_FAIL);

	 //sleep(1);

	 action = BT_AVRCP_KEY_STATE_RELEASE;
	 i4_ret = a_mtkapi_avrcp_send_passthrough_cmd(g_cur_avrcp_addr, BT_AVRCP_CMD_TYPE_BWD, action);
	 BT_AUD_CHK_FAIL_RET(a_mtkapi_avrcp_send_passthrough_cmd, i4_ret, BT_AUD_FAIL);

	 BT_AUD_FUNC_EXIT();
	 return BT_AUD_OK;
 }


static INT32 _bluetooth_avrcp_track_change_prco(BT_AVRCP_MEDIA_INFO *t_cur_media_info)
{
	INT32 i4_ret = BT_AUD_FAIL;
	
	BT_AUD_FUNC_ENTRY();

	if (g_t_bt_aud_sta.ui4_bt_track_idx != t_cur_media_info->current_track_number)
	{
		g_t_bt_aud_sta.ui4_bt_track_idx = t_cur_media_info->current_track_number;
		DBG_API((BTAUD_TAG"Not the same track.\r\n"));

		//fix IOS device play next/prev no sound by voice 
		if ((BT_AUD_SM_REQ_PREV == g_t_bt_aud_sta.e_bt_sm_cmd_sta)
			|| (BT_AUD_SM_REQ_NEXT == g_t_bt_aud_sta.e_bt_sm_cmd_sta))
		{
			if (FALSE == g_t_bt_aud_sta.b_bt_play)
			{
				DBG_ERROR((BTAUD_TAG"A2DP stream is suspend, here need to send AVRCP(PLAY) to remote.\r\n"));
				
				i4_ret = _bluetooth_send_avrcp_play_proc();
				BT_AUD_CHK_FAIL_RET(_bluetooth_send_avrcp_play_proc, i4_ret, BT_AUD_FAIL);
			}
			else
			{
				DBG_ERROR((BTAUD_TAG"A2DP stream is started.\r\n"));
			}
		}
		else
		{
			DBG_ERROR((BTAUD_TAG"Track changed is not controled by SM.\r\n"));
		}
	}
	else
	{
		DBG_API((BTAUD_TAG"Same track.\r\n"));
	}

	DBG_ERROR((BTAUD_TAG"cur_mediaInfo.title %s \r\n",t_cur_media_info->title));
	DBG_ERROR((BTAUD_TAG"cur_mediaInfo.artist %s \r\n",t_cur_media_info->artist));
	DBG_ERROR((BTAUD_TAG"cur_mediaInfo.album %s \r\n",t_cur_media_info->album));
	DBG_ERROR((BTAUD_TAG"cur_mediaInfo.current_track_number %d \r\n",t_cur_media_info->current_track_number));

	BT_AUD_FUNC_EXIT();
}

 static VOID _bluetooth_avrcp_position_change_proc(BT_AVRCP_POS_CHANGE *position_change)
 {
	 BT_AUD_FUNC_ENTRY();

	 DBG_ERROR((BTAUD_TAG"cur_song_len %d \r\n",position_change->song_len));
	 DBG_ERROR((BTAUD_TAG"cur_song_pos %s \r\n",position_change->song_pos));

	 BT_AUD_FUNC_EXIT();
 }

 INT32 _bluetooth_set_play_status(BT_AVRCP_PLAY_STATUS e_playstatus)
 {
	 g_t_bt_aud_sta.b_play_pause_down = FALSE;//clear it when get play status respond

	 BT_AUD_FUNC_ENTRY();

	 switch(e_playstatus)
	 {
		 case AVRCP_PLAY_STATUS_STOPPED:
		 case AVRCP_PLAY_STATUS_PAUSED:
			 DBG_API((BTAUD_TAG"set play flag as FALSE.\r\n"));
			 g_t_bt_aud_sta.b_bt_play = FALSE;
			 break;

		 case AVRCP_PLAY_STATUS_PLAYING:
		 case AVRCP_PLAY_STATUS_FORWARDSEEK:
		 case AVRCP_PLAY_STATUS_REWINDSEEK:
			 DBG_API((BTAUD_TAG"set play flag as TRUE.\r\n"));
			 g_t_bt_aud_sta.b_bt_play = TRUE;
			 break;

		 default:
			 DBG_ERROR((BTAUD_TAG"can't set play status, status=%d.\r\n", e_playstatus));
			 break;
	 }

	 return BT_AUD_OK;
	 BT_AUD_FUNC_EXIT();
 }

 static INT32 _bluetooth_avrcp_playstatus_proc(UINT32 ui4_playstatus)
 {
	  BT_AVRCP_PLAY_STATUS e_playsta;
	 INT32 i4_ret;
	 BT_AUD_FUNC_ENTRY();


	 switch (ui4_playstatus)
	 {
		 case AVRCP_PLAY_STATUS_PLAYING:	   /* Playing */
		 {
			 e_playsta = AVRCP_PLAY_STATUS_PLAYING;
			 DBG_API((BTAUD_TAG"BT_PLAY_STATE_PLAY.\r\n"));

			 i4_ret = _bluetooth_state_play_proc();
			 BT_AUD_CHK_FAIL_RET(_bluetooth_state_play_proc, i4_ret, BT_AUD_FAIL);
		 }
		 break;

		 case AVRCP_PLAY_STATUS_PAUSED: 	 /* Paused	*/
		 {
			 e_playsta = AVRCP_PLAY_STATUS_PAUSED;
			 DBG_API((BTAUD_TAG"BT_PLAY_STATE_PAUSE.\r\n"));

			 i4_ret = _bluetooth_state_pause_proc();
			 BT_AUD_CHK_FAIL_RET(_bluetooth_state_pause_proc, i4_ret, BT_AUD_FAIL);
		 }
		 break;

		 case AVRCP_PLAY_STATUS_STOPPED:	/* Stopped */
		 {
			 e_playsta = AVRCP_PLAY_STATUS_STOPPED;
			 DBG_API((BTAUD_TAG"AVRCP_PLAY_STATUS_STOPPED.\r\n"));

			 i4_ret = _bluetooth_state_pause_proc();
			 BT_AUD_CHK_FAIL_RET(_bluetooth_state_pause_proc, i4_ret, BT_AUD_FAIL);
		 }
		 break;

		 case AVRCP_PLAY_STATUS_FORWARDSEEK:	/* Fwd Seek*/
		 {
			 e_playsta = AVRCP_PLAY_STATUS_FORWARDSEEK;
			 DBG_API((BTAUD_TAG"AVRCP_PLAY_STATUS_FORWARDSEEK.\r\n"));
		 }
		 break;

		 case AVRCP_PLAY_STATUS_REWINDSEEK:    /* Rev Seek*/
		 {
			 e_playsta = AVRCP_PLAY_STATUS_REWINDSEEK;
			 DBG_API((BTAUD_TAG"AVRCP_PLAY_STATUS_REWINDSEEK.\r\n"));
		 }
		 break;

		 default:
			 DBG_ERROR((BTAUD_TAG"Err:No such play status id=%d.\r\n", ui4_playstatus));
			 return BT_AUD_FAIL;
	 }

	 _bluetooth_set_play_status(e_playsta);

	 BT_AUD_FUNC_EXIT();
	 return BT_AUD_OK;
 }


 INT32 _bluetooth_avrcp_absoulte_volume_update(const UINT8 ui4_volume)
 {
	 INT32 i4_ret = BT_AUD_FAIL;

	 BT_AUD_FUNC_ENTRY();

	 if (_bluetooth_is_bt_ready() == FALSE)
	 {
		 DBG_INFO((BTAUD_TAG"BT isn't the current input source, ignore volume sync.\r\n"));
	 }
	 else	//update local volume to MW
	 {
		 DBG_API((BTAUD_TAG"local volume update to MW, localvol=%d \r\n",
							 ui4_volume));
		 i4_ret = a_mtkapi_avrcp_update_absolute_volume(ui4_volume);
		 BT_AUD_CHK_FAIL_RET(a_mtkapi_avrcp_update_absolute_volume, i4_ret, BT_AUD_FAIL);
	 }

	 BT_AUD_FUNC_EXIT();
	 return BT_AUD_OK;
 }


 INT32 _bluetooth_avrcp_change_remote_volume(const UINT8 ui4_volume)
 {
	 INT32 i4_ret = BT_AUD_FAIL;

	 BT_AUD_FUNC_ENTRY();

	 if (_bluetooth_is_bt_connected() == FALSE)
	 {
		 DBG_INFO((BTAUD_TAG"BT disconnected, ignore volume sync.\r\n"));
	 }
	 else	//update local volume to MW
	 {
		 DBG_API((BTAUD_TAG"local volume update to MW, localvol=%d \r\n",
							 ui4_volume));
		 i4_ret = a_mtkapi_avrcp_change_volume(g_cur_avrcp_addr, ui4_volume);
		 BT_AUD_CHK_FAIL_RET(a_mtkapi_avrcp_update_absolute_volume, i4_ret, BT_AUD_FAIL);
	 }

	 BT_AUD_FUNC_EXIT();
	 return BT_AUD_OK;
 }


 INT32 _bluetooth_avrcp_volume_sync(const UINT8 ui4_volume)
 {
	 INT32 i4_ret = BT_AUD_FAIL;

	 BT_AUD_FUNC_ENTRY();

	 if (BT_A2DP_ROLE_SRC == g_cur_a2dp_role)
	 {
		 DBG_INFO((BTAUD_TAG"current local A2DP role is SRC.\r\n"));

		 if(_bluetooth_is_bt_connected() == FALSE)
		 {

		 }

		 i4_ret = _bluetooth_avrcp_change_remote_volume(ui4_volume);
		 BT_AUD_CHK_FAIL_RET(_bluetooth_avrcp_change_remote_volume, i4_ret, BT_AUD_FAIL);
	 }
	 else if (BT_A2DP_ROLE_SINK == g_cur_a2dp_role)  //set	local volume to remote
	 {
		 DBG_INFO((BTAUD_TAG"current local A2DP role is SINK.\r\n"));
		 i4_ret = _bluetooth_avrcp_absoulte_volume_update(ui4_volume);
		 BT_AUD_CHK_FAIL_RET(_bluetooth_avrcp_absoulte_volume_update, i4_ret, BT_AUD_FAIL);
	 }

	 BT_AUD_FUNC_EXIT();
	 return BT_AUD_OK;
 }


 INT32 _bluetooth_avrcp_set_volume_req_proc(UINT8 ui4_volume)
 {
	 INT32 i4_ret;
	 UINT8 ui1_setval = 0, ui1_cur_vol = 0;
	 BT_AUD_FUNC_ENTRY();

	 if (_bluetooth_is_bt_ready() == FALSE)
	 {
		 DBG_INFO((BTAUD_TAG"BT isn't the current input source, ignore volume sync.\r\n"));
	 }
	 else if (g_t_bt_aud_sta.e_bt_sta != BT_AUD_BT_STA_PLAYING)
	 {
		 DBG_API((BTAUD_TAG"BT isn't under playing mode, ignore volume sync.\r\n"));
	 }
	 else	 //sync volume when BT is current source
	 {
		 //change remote volume to local type before set it to the platform
		 ui1_setval = ((ui4_volume * BT_AUD_LOCAL_MAX_VOLUME)
						 + (BT_AUD_REMOTE_DEV_MAX_VOLUME - 1))
						 / BT_AUD_REMOTE_DEV_MAX_VOLUME;

		 i4_ret = u_acfg_get_volume(&ui1_cur_vol);
		 if (APP_CFGR_OK != i4_ret)
		 {
			 DBG_ERROR((BTAUD_TAG"Err: get local volume fail.\r\n"));
			 return i4_ret;
		 }
#if CONFIG_APP_SUPPORT_XPL
		 //In case volume turns up to max volume abruptly, only allow volume turns up 25% once
		 if ((ui1_setval > ui1_cur_vol)
			 && ((ui1_setval - ui1_cur_vol) > (BT_AUD_LOCAL_MAX_VOLUME >> 2)))
		 {
			 DBG_API((BTAUD_TAG"volume turns up abruptly, limit the variation.\r\n"));
			 ui1_setval = ui1_cur_vol + (BT_AUD_LOCAL_MAX_VOLUME >> 2);
		 }
		 else if (ui1_setval == ui1_cur_vol)//remote volume no change or change slightly, then ignore this action
		 {
			 DBG_ERROR((BTAUD_TAG"remote volume is the same with local, ignore this action.\r\n"));
			 return BT_AUD_OK;
		 }
#endif/* CONFIG_APP_SUPPORT_XPL */
		 DBG_ERROR((BTAUD_TAG"remote volume sync to local, localvol=%d, set_vol=%d, send_vol=%d.\r\n",
				 ui1_cur_vol, ui1_setval, ui4_volume));

		 i4_ret = _bluetooth_set_absolute_volume(ui1_setval);
		 BT_AUD_CHK_FAIL_RET(_bluetooth_set_absolute_volume, i4_ret, BT_AUD_FAIL);
	 }

	 BT_AUD_FUNC_EXIT();
	 return BT_AUD_OK;
 }


 static INT32 _bluetooth_avrcp_passthrough_volume_up_proc(VOID)
 {
	 BT_AUD_FUNC_ENTRY();

	 UINT8 ui1_cur_vol,ui1_cur_vol_new;
	 INT32 i4_ret;

	 DBG_API((BTAUD_TAG"BT_AVRCP_CMD_TYPE_VOL_UP.\r\n"));

	 //change remote volume to local type before set it to the platform
	 i4_ret = u_acfg_get_volume(&ui1_cur_vol);
	 if (APP_CFGR_OK != i4_ret)
	  {
			 DBG_ERROR((BTAUD_TAG"Err: get local volume fail.\r\n"));
			 return i4_ret;
	  }
	 if(ui1_cur_vol>=0&& ui1_cur_vol<=BT_AUD_LOCAL_MAX_VOLUME)
	 {
		 //change VOLUME_CHANGE_STEP once
		 ui1_cur_vol_new = ui1_cur_vol + BT_AUD_VOLUME_CHANGE_STEP;

		 if(ui1_cur_vol_new>BT_AUD_LOCAL_MAX_VOLUME) //local volume is the max
		 {
			 ui1_cur_vol_new = BT_AUD_LOCAL_MAX_VOLUME;
			 DBG_ERROR((BTAUD_TAG"local volume is the max, ignore this action.\r\n"));
		 }
	 }
	 else
	 {
		 //In case current local volume error
		 DBG_ERROR((BTAUD_TAG"current local volume is ERROR.\r\n"));
		 ui1_cur_vol_new = 50;
		 return BT_AUD_OK;
	 }

	 DBG_API((BTAUD_TAG"remote volume sync to local, localvol=%d, set_vol=%d.\r\n",
				 ui1_cur_vol, ui1_cur_vol_new));

	 i4_ret = _bluetooth_set_absolute_volume(ui1_cur_vol_new);
	 BT_AUD_CHK_FAIL_RET(_bluetooth_set_absolute_volume, i4_ret, BT_AUD_FAIL);

	 BT_AUD_FUNC_EXIT();
	 return BT_AUD_OK;
 }


 static INT32 _bluetooth_avrcp_passthrough_volume_down_proc(VOID)
 {
	 BT_AUD_FUNC_ENTRY();

	 UINT8 ui1_cur_vol,ui1_cur_vol_new;
	 INT32 i4_ret;

	 DBG_API((BTAUD_TAG"BT_AVRCP_CMD_TYPE_VOL_DOWN.\r\n"));

	 //change remote volume to local type before set it to the platform
	 i4_ret = u_acfg_get_volume(&ui1_cur_vol);
	 if (APP_CFGR_OK != i4_ret)
	  {
			 DBG_ERROR((BTAUD_TAG"Err: get local volume fail.\r\n"));
			 return i4_ret;
	  }
	 if(ui1_cur_vol>=0&& ui1_cur_vol<=BT_AUD_LOCAL_MAX_VOLUME)
	 {
		 //change VOLUME_CHANGE_STEP once
		 if(ui1_cur_vol<=BT_AUD_VOLUME_CHANGE_STEP) //local volume is the minus
		 {
			 ui1_cur_vol_new = 0;
			 DBG_ERROR((BTAUD_TAG"local volume is less than BT_AUD_VOLUME_CHANGE_STEP, ignore this action.\r\n"));
		 }
		 else
		 {
			 //change VOLUME_CHANGE_STEP once
			 ui1_cur_vol_new = ui1_cur_vol - BT_AUD_VOLUME_CHANGE_STEP;
		 }
	 }
	 else
	 {
		 //In case current local volume error
		 DBG_ERROR((BTAUD_TAG"current local volume is ERROR.\r\n"));
		 ui1_cur_vol_new = 50;

		 return BT_AUD_OK;
	 }

	 DBG_API((BTAUD_TAG"remote volume sync to local, localvol=%d, set_vol=%d.\r\n",
				 ui1_cur_vol, ui1_cur_vol_new));

	 i4_ret = _bluetooth_set_absolute_volume(ui1_cur_vol_new);
	 BT_AUD_CHK_FAIL_RET(_bluetooth_set_absolute_volume, i4_ret, BT_AUD_FAIL);

	 BT_AUD_FUNC_EXIT();
	 return BT_AUD_OK;
 }


 static INT32 _bluetooth_src_avrcp_volume_req_proc(UINT8 cmd_type)
 {
	 INT32 i4_ret = BT_AUD_FAIL;

	 BT_AUD_FUNC_ENTRY();

	 switch (cmd_type)
	 {
		 case BT_AVRCP_CMD_TYPE_VOL_UP:
		 {
			 DBG_API((BTAUD_TAG"BT_AVRCP_CMD_TYPE_VOL_UP.\r\n"));

			 i4_ret = _bluetooth_avrcp_passthrough_volume_up_proc();
			 BT_AUD_CHK_FAIL(_bluetooth_avrcp_passthrough_volume_up_proc, i4_ret);
		 }
		 break;

		 case BT_AVRCP_CMD_TYPE_VOL_DOWN:
		 {
			 DBG_API((BTAUD_TAG"BT_AVRCP_CMD_TYPE_VOL_DOWN.\r\n"));

			 i4_ret = _bluetooth_avrcp_passthrough_volume_down_proc();
			 BT_AUD_CHK_FAIL(_bluetooth_avrcp_passthrough_volume_down_proc, i4_ret);
		 }
		 break;

		 default:
			 DBG_ERROR((BTAUD_TAG"Err:No such volume type = %d.\r\n", cmd_type));
			 return BT_AUD_FAIL;
	 }

	 BT_AUD_FUNC_EXIT();
	 return BT_AUD_OK;
 }


 static INT32 _bluetooth_avrcp_passthrough_cmd_req_proc(UINT8 cmd_type,UINT32 key_state)
 {
	 INT32 i4_ret = BT_AUD_FAIL;

	 BT_AUD_FUNC_ENTRY();

	 if (BT_AVRCP_KEY_STATE_PRESS != key_state)
	 {
		 DBG_ERROR((BTAUD_TAG"key state is not PRESS, ignore cmd!"));
		 return BT_AUD_OK;
	 }

	 switch (cmd_type)
	 {
		 case BT_AVRCP_CMD_TYPE_VOL_UP:

		 case BT_AVRCP_CMD_TYPE_VOL_DOWN:
		 {
			 i4_ret = _bluetooth_src_avrcp_volume_req_proc(cmd_type);
			 BT_AUD_CHK_FAIL(_bluetooth_avrcp_passthrough_volume_up_proc, i4_ret);
		 }
		 break;
#if CONFIG_APP_SUPPORT_BT_SRC_MODE
		 case BT_AVRCP_CMD_TYPE_PLAY:
		 case BT_AVRCP_CMD_TYPE_PAUSE:
		 case BT_AVRCP_CMD_TYPE_FWD:
		 case BT_AVRCP_CMD_TYPE_BWD:
		 case BT_AVRCP_CMD_TYPE_FFWD:
		 case BT_AVRCP_CMD_TYPE_RWD:
		 case BT_AVRCP_CMD_TYPE_STOP:
		 {
			 i4_ret = _bluetooth_src_avrcp_cmd_proc(cmd_type);
			 BT_AUD_CHK_FAIL(_bluetooth_src_avrcp_cmd_proc, i4_ret);
		 }
		 break;
#endif /*CONFIG_APP_SUPPORT_BT_SRC_MODE*/
		 default:
			 DBG_ERROR((BTAUD_TAG"Err:No such volume type = %d.\r\n", cmd_type));
			 break;
	 }

	 BT_AUD_FUNC_EXIT();
	 return BT_AUD_OK;
 }

INT32 _bluetooth_avrcp_event_proc(const VOID *pv_msg)
 {
	 INT32 i4_ret;
	 BT_AUD_MSG_T* pt_bt_avrcp_msg = (BT_AUD_MSG_T*)pv_msg;

	 BT_AUD_FUNC_ENTRY();

	 DBG_ERROR((BTAUD_TAG"AVRCP event msg come : (%d)\r\n",
						 pt_bt_avrcp_msg->ui4_msg_id));

	 switch(pt_bt_avrcp_msg->ui4_msg_id)
	 {
		 case BT_AVRCP_EVENT_CONNECTED:
		 {
			 DBG_ERROR((BTAUD_TAG"AVRCP connect success (device addr : %s). \r\n",
								 pt_bt_avrcp_msg->ui4_str1));
		 }
		 break;

		 case BT_AVRCP_EVENT_DISCONNECTED:
		 {
			 DBG_ERROR((BTAUD_TAG"AVRCP disconnected (device addr : %s). \r\n",
								 pt_bt_avrcp_msg->ui4_str1));
			 g_t_bt_aud_sta.b_bt_avrcp_connect = FALSE;
		 }
		 break;

		 case BT_AVRCP_EVENT_TRACK_CHANGE:
		 {
			 DBG_ERROR((BTAUD_TAG"BT_AVRCP_EVENT_TRACK_CHANGE \r\n"));

			 _bluetooth_avrcp_track_change_prco(&g_t_cur_media_info);
		 }
		 break;

		 case BT_AVRCP_EVENT_POS_CHANGE:
		 {
			 BT_AVRCP_POS_CHANGE position_change;

			 DBG_ERROR((BTAUD_TAG"BT_A2DP_EVENT_STREAM_START (device addr : %s). \r\n",
								 pt_bt_avrcp_msg->ui4_str1));

			 position_change.song_len = pt_bt_avrcp_msg->ui4_data1;
			 position_change.song_pos = pt_bt_avrcp_msg->ui4_data3;

			 _bluetooth_avrcp_position_change_proc(&position_change);
		 }
		 break;

		 case BT_AVRCP_EVENT_PLAY_STATUS_CHANGE:
		 {
			 DBG_ERROR((BTAUD_TAG"BT_AVRCP_EVENT_PLAY_STATUS_CHANGE , status: %d. \r\n",
								 pt_bt_avrcp_msg->ui4_data1));

			 i4_ret = _bluetooth_avrcp_playstatus_proc(pt_bt_avrcp_msg->ui4_data1);
			 BT_AUD_CHK_FAIL_RET(_bluetooth_avrcp_playstatus_proc, i4_ret, BT_AUD_FAIL);
		 }
		 break;

		 case BT_AVRCP_EVENT_VOLUME_CHANGE:
		 {
			 DBG_ERROR((BTAUD_TAG"BT_AVRCP_EVENT_VOLUME_CHANGE , volume = %d. \r\n",
								 pt_bt_avrcp_msg->ui4_data2));
		 }
		 break;

		 case BT_AVRCP_EVENT_SET_VOLUME_REQ:
		 {
			 DBG_ERROR((BTAUD_TAG"BT_AVRCP_EVENT_SET_VOLUME_REQ. \r\n"));

			 i4_ret = _bluetooth_avrcp_set_volume_req_proc(pt_bt_avrcp_msg->ui4_data2);
			 BT_AUD_CHK_FAIL_RET(_bluetooth_avrcp_set_volume_req_proc, i4_ret, BT_AUD_FAIL);
		 }
		 break;

		 case BT_AVRCP_EVENT_PASSTHROUGH_CMD_REQ:
		 {
			 DBG_ERROR((BTAUD_TAG"BT_AVRCP_EVENT_PASSTHROUGH_CMD_REQ. \r\n"));

			 i4_ret = _bluetooth_avrcp_passthrough_cmd_req_proc(pt_bt_avrcp_msg->ui4_data1,pt_bt_avrcp_msg->ui4_data2);
			 BT_AUD_CHK_FAIL_RET(_bluetooth_avrcp_passthrough_cmd_req_proc, i4_ret, BT_AUD_FAIL);
		 }
		 break;

		 default:
			 DBG_ERROR((BTAUD_TAG"Err:can't recognize the AVRCP event %d.\r\n",
								 pt_bt_avrcp_msg->ui4_msg_id));
			 break;
	 }

	 BT_AUD_FUNC_EXIT();
	 return BT_AUD_OK;
 }

#if CONFIG_APP_SUPPORT_BT_SRC_MODE
INT32 _bluetooth_src_send_msg_to_itself(UINT32 ui4_msgtype, BT_AUD_MSG_T* pt_bt_msg)
{
    INT32 i4_ret = 0;

    //send msg to bt thread
    if (NULL_HANDLE == g_t_bt_aud.h_app)
    {
        DBG_ERROR((BTAUD_TAG"bluetooth audio has a NULL_HANDLE.\r\n"));
        return BT_AUD_FAIL;
    }

    i4_ret = u_app_send_msg(g_t_bt_aud.h_app,
                    ui4_msgtype,
                    pt_bt_msg,
                    sizeof(BT_AUD_MSG_T),
                    NULL,
                    NULL);
    BT_AUD_CHK_FAIL_RET(u_app_send_msg, i4_ret, BT_AUD_FAIL);

    return BT_AUD_OK;

}

INT32 _bluetooth_src_avrcp_cmd_proc(UINT8 avrcp_cmd)
{
    BT_AUD_FUNC_ENTRY();
    switch(avrcp_cmd)
    {
        case BT_AVRCP_CMD_TYPE_PLAY:    //  get play information.
            DBG_API((BTAUD_TAG"get play/resume information.\r\n"));
            _bluetooth_send_src_avrcp_cmd_to_assistant_stub(STRING_BT_PLAY);
            break;

        case BT_AVRCP_CMD_TYPE_PAUSE:   //  get pause information.
            DBG_API((BTAUD_TAG"get pause information.\r\n"));
            _bluetooth_send_src_avrcp_cmd_to_assistant_stub(STRING_BT_PAUSE);
            break;

        case BT_AVRCP_CMD_TYPE_FWD:     //   get forward information.
            DBG_API((BTAUD_TAG"get forward information.\r\n"));
            _bluetooth_send_src_avrcp_cmd_to_assistant_stub(STRING_BT_FORWARD);
            break;

        case BT_AVRCP_CMD_TYPE_BWD:     //  get backward information.
            DBG_API((BTAUD_TAG"get backward information.\r\n"));
            _bluetooth_send_src_avrcp_cmd_to_assistant_stub(STRING_BT_BACKWARD);
            break;

        case BT_AVRCP_CMD_TYPE_FFWD:   //  get fast_forward information.
            DBG_API((BTAUD_TAG"get fast_forward information.\r\n"));
            break;

        case BT_AVRCP_CMD_TYPE_RWD:    //  get rewind information.
            DBG_API((BTAUD_TAG"get rewind information.\r\n"));
            break;

        case BT_AVRCP_CMD_TYPE_STOP:   //  get  stop information.
            DBG_API((BTAUD_TAG"get stop information.\r\n"));
            _bluetooth_send_src_avrcp_cmd_to_assistant_stub(STRING_BT_STOP);
            break;

        default:
            DBG_ERROR((BTAUD_TAG"Err: unsupported src avrcp cmd msg.\r\n"));
        break;
    }
    BT_AUD_FUNC_EXIT();
    return BT_AUD_OK;

}

INT32 u_bluetooth_src_request_mediaInfo_from_url(BT_SRC_PLAYER_STATUS_T *medialnfo)
{
    INT32 i4_ret;
    BT_SRC_PLAYER_STATUS_T bt_medialnfo;
    BT_AUD_MSG_T t_msg = {0};

    BT_AUD_FUNC_ENTRY();

    DBG_API((BTAUD_TAG"[SRC MODE]bt aud request medialnfo from url\r\n"));

    memset(&bt_medialnfo,0,sizeof(BT_SRC_PLAYER_STATUS_T));
    memcpy(&bt_medialnfo,medialnfo,sizeof(BT_SRC_PLAYER_STATUS_T));

    t_msg.ui4_msg_type  = BT_AUD_PTE_MSG_SRC;
    t_msg.ui4_msg_id    = BT_SRC_MSG_MEDIAINFO_EVENT;
    t_msg.btmedialnfo   = bt_medialnfo;
    _bluetooth_src_send_msg_to_itself(E_APP_MSG_TYPE_BLUETOOTH, &t_msg);

    return BT_AUD_OK;
}
INT32 _bluetooth_src_itself_msg_proc(BT_SRC_PLAYER_STATUS_T *medialnfo)
{
    BT_AUD_FUNC_ENTRY();
	INT32 i4_ret;
	
    if(NULL == medialnfo)
    {
        DBG_ERROR((BTAUD_TAG"medialnfo is NULL.\r\n"));
        return;
    }
    memset(&g_t_src_medialnfo,0,sizeof(BT_AVRCP_PLAYER_MEDIA_INFO));

    memcpy(&g_t_src_medialnfo.media_info.title, medialnfo->audioName, BT_AUDIO_NAME_MAX_LENGTH);
    memcpy(&g_t_src_medialnfo.media_info.album, medialnfo->audioAlbum, BT_AUDIO_ALBUM_MAX_LENGTH);
    memcpy(&g_t_src_medialnfo.media_info.artist, medialnfo->audioAnchor, BT_AUDIO_ANCHOR_MAX_LENGTH);
    g_t_src_medialnfo.media_info.position = medialnfo->progress;
	g_t_src_player_status.song_len = medialnfo->song_length;
	g_t_src_player_status.song_pos = medialnfo->progress;

    if(TRUE == u_playback_uri_get_running_flag())
    {
        memset(&g_t_src_medialnfo.track,0,BT_AVRCP_UID_SIZE);
        i4_ret = _bluetooth_src_update_mediainfo_to_mw();
        if (BT_AUD_OK != i4_ret)
        {
            DBG_ERROR(("<BT_AUD>_bluetooth_src_update_mediainfo_to_mw err, %s\n", __FUNCTION__));
            return BT_AUD_FAIL;
        }
    }
    else
    {
        memset(&g_t_src_medialnfo.track,0xff,BT_AVRCP_UID_SIZE);
    }

    DBG_ERROR((BTAUD_TAG"g_t_src_medialnfo.media_info.title : %s , g_t_src_player_status.song_len: %d ,"
		                "g_t_src_medialnfo.media_info.position: %d, g_t_src_medialnfo.track :%s\n",
		                g_t_src_medialnfo.media_info.title,g_t_src_player_status.song_len,
		                g_t_src_medialnfo.media_info.position,g_t_src_medialnfo.track));

	BT_AUD_FUNC_EXIT();
	return BT_AUD_OK;
}

INT32 _bluetooth_src_update_mediainfo_to_mw(VOID)
{
    INT32 i4_ret;

    BT_AUD_FUNC_ENTRY();

    DBG_API((BTAUD_TAG"[SRC MODE]bt aud send mediainfo to mw\r\n"));

    i4_ret = a_mtkapi_avrcp_update_player_media_info(&g_t_src_medialnfo);
    BT_AUD_CHK_FAIL(a_mtkapi_avrcp_update_player_media_info, i4_ret);

	BT_AUD_FUNC_EXIT();
	return BT_AUD_OK;
}

ASSISTANT_STUB_BT_SRC_AVRCP_CMD_T* u_bluetooth_src_set_player_status(char *avrcp_cmd)
{
    INT32 i4_ret;
	BT_AUD_FUNC_ENTRY();

	strncpy(g_t_bt_src_avrcp_cmd.command,
			"/bluetooth/avrcp_src_cmd",
			ASSISTANT_STUB_COMMAND_MAX_LENGTH);
    strncpy(g_t_bt_src_avrcp_cmd.cmd.request,
			avrcp_cmd,
			ASSISTANT_STUB_SOURCE_MAX_LENGTH);

	DBG_API((BTAUD_TAG" ====ASSISTANT_STUB=====g_t_bt_src_avrcp_cmd.command : %s \r\n",g_t_bt_src_avrcp_cmd.command));
    DBG_API((BTAUD_TAG" ====ASSISTANT_STUB=====g_t_bt_src_avrcp_cmd.cmd.request : %s \r\n",g_t_bt_src_avrcp_cmd.cmd.request));

	return &g_t_bt_src_avrcp_cmd;
}


INT32 _bluetooth_send_src_avrcp_cmd_to_assistant_stub(char *avrcp_cmd)
{
	BT_AUD_FUNC_ENTRY();

	INT32 i4_ret = BT_AUD_FAIL;
    HANDLE_T h_app = NULL_HANDLE;
    ASSISTANT_STUB_BT_SRC_AVRCP_CMD_T* bt_src_avrcp_cmd = {0};

    bt_src_avrcp_cmd = u_bluetooth_src_set_player_status(avrcp_cmd);
    DBG_API((BTAUD_TAG" ====ASSISTANT_STUB=====bt_src_avrcp_cmd.command : %s \r\n",bt_src_avrcp_cmd->command));
	DBG_API((BTAUD_TAG" ====ASSISTANT_STUB=====bt_src_avrcp_cmd.cmd.request : %s \r\n",bt_src_avrcp_cmd->cmd.request));

    u_am_get_app_handle_from_name(&h_app,ASSISTANT_STUB_THREAD_NAME);

    i4_ret = u_app_send_appmsg(h_app,
                               E_APP_MSG_TYPE_ASSISTANT_STUB,
                               MSG_FROM_BT,
                               ASSISTANT_STUB_CMD_BT_SRC_AVRCP_CMD,
                               bt_src_avrcp_cmd,
                               sizeof(ASSISTANT_STUB_BT_SRC_AVRCP_CMD_T));
    if (AEER_OK != i4_ret)
    {
        DBG_ERROR((BTAUD_TAG"\n bluetooth send avrcp passthrough cmd to assistant_stub failed, i4_ret[%ld]\n",i4_ret));
    }
    else
    {
        DBG_ERROR((BTAUD_TAG"\n bluetooth send avrcp passthrough cmd to assistant_stub success, i4_ret[%ld]\n",i4_ret));
    }

	BT_AUD_FUNC_EXIT();
    return (i4_ret == AEER_OK ? BT_AUD_OK : BT_AUD_FAIL);
}

INT32 _bluetooth_src_update_player_status_to_mw(BT_AVRCP_PLAY_STATUS status)
{
	INT32 i4_ret = BT_AUD_FAIL;

	BT_AUD_FUNC_ENTRY();

	g_t_src_player_status.play_status = status;

    i4_ret = a_mtkapi_avrcp_update_player_status(&g_t_src_player_status);
    BT_AUD_CHK_FAIL(a_mtkapi_avrcp_update_player_status, i4_ret);
    DBG_API((BTAUD_TAG"play_status=%d, song_length=%d, song_position=%d.\r\n",
        				g_t_src_player_status.play_status,
        				g_t_src_player_status.song_len,
        				g_t_src_player_status.song_pos));

   BT_AUD_FUNC_EXIT();
   return BT_AUD_OK;
}


#endif /*CONFIG_APP_SUPPORT_BT_SRC_MODE*/


