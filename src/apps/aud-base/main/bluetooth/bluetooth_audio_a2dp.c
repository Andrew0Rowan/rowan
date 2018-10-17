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
#include<unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <pthread.h>
#include <sys/time.h>


/* application level */
#include "u_aee.h"
#include "u_cli.h"
#include "u_dbg.h"
#include "u_os.h"
#include "u_appman.h"
#include "u_sm.h"
#include "u_app_thread.h"
#include "u_amb.h"
#include "u_am.h"
#include "u_acfg.h"
#include "u_timerd.h"
#include "u_assistant_stub.h"
#include "Interface.h"
#if CONFIG_APP_SUPPORT_BT_SRC_MODE
#include "mas_lib.h"
#endif /*CONFIG_APP_SUPPORT_BT_SRC_MODE*/


/*    btmw   */
#include "c_bt_mw_a2dp_snk.h"
#include "bt_a2dp_alsa_playback.h"
#include "c_bt_mw_gap.h"
#include "u_bt_mw_common.h"
#include "mtk_bt_service_gap_wrapper.h"
#include "mtk_bt_service_a2dp_wrapper.h"
#include "mtk_bt_service_hfclient_wrapper.h"

/* private */
#include "u_bluetooth_audio.h"
#include "bluetooth_audio_proc.h"
#include "bluetooth_audio_key_proc.h"
#include "bluetooth_audio.h"
#include "bluetooth_audio_a2dp.h"
#include "bluetooth_audio_gap.h"
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
CHAR g_cur_a2dp_addr[18]={0};
BT_A2DP_ROLE g_cur_a2dp_role;

extern BT_AUD_BLUETOOTH_STATUS_T g_t_bt_aud_sta;
extern BT_AUD_OBJ_T g_t_bt_aud;
extern ASSISTANT_STUB_BLUETOOTH_STATUS_CHANGE_T g_t_bt_aud_bt_status;
extern UINT8 g_cur_gap_scan_mode;

#if CONFIG_APP_SUPPORT_BT_SRC_MODE
extern void* g_mas_handle;
#endif /*CONFIG_APP_SUPPORT_BT_SRC_MODE*/
#if CONFIG_SUPPORT_BT_HFP
extern BT_HFP_STATUS_T g_t_bt_hfp_sta;
extern BT_HFP_OBJ_T g_t_g_bt_hfp;
extern BT_HFP_CBK_STATUS_T g_t_bt_hfp_cbk_sta;
extern BOOL g_b_hfp_ability_enable;
extern CHAR g_cur_hfp_addr[18];
#endif /*CONFIG_SUPPORT_BT_HFP*/

/*-----------------------------------------------------------------------------
 * private methods declarations
 *---------------------------------------------------------------------------*/




 VOID _bluetooth_a2dp_event_cbk_fct(BT_A2DP_EVENT_PARAM *a2dp_event)
 {
	 BT_AUD_MSG_T t_msg = {0};
	 BT_AUD_FUNC_ENTRY();
 
	 if (NULL == a2dp_event)
	 {
		 DBG_ERROR((BTAUD_TAG"a2dp_event is NULL\n"));
		 return;
	 }
	 
	 DBG_ERROR((BTAUD_TAG"A2DP handle cb, a2dp_event = %d.\r\n",a2dp_event->event));
 
	 switch (a2dp_event->event)
	 {
		 case BT_A2DP_EVENT_CONNECTED:
		 {
			DBG_ERROR((BTAUD_TAG"A2DP connected (%s), sample rate=%d, channel num=%d \r\n",
							 a2dp_event->addr,
							 a2dp_event->data.connected_data.sample_rate,
							 a2dp_event->data.connected_data.channel_num));

			g_cur_a2dp_role = a2dp_event->data.connected_data.local_role;
			strncpy(g_cur_a2dp_addr,
					a2dp_event->addr,
					MAX_BDADDR_LEN);
			 
			strncpy(t_msg.ui4_str1,
					a2dp_event->addr,
					MAX_BDADDR_LEN);		 
		 }
		 break;
		 
		 case BT_A2DP_EVENT_DISCONNECTED:
		 {
			 DBG_ERROR((BTAUD_TAG"A2DP disconnected (device addr : %s)\n", a2dp_event->addr));
 
			 strncpy(g_cur_a2dp_addr,
					 a2dp_event->addr,
					 MAX_BDADDR_LEN);
 
			 strncpy(t_msg.ui4_str1,
					 a2dp_event->addr,
					 MAX_BDADDR_LEN);
		 }
		 break;
		 
		 case BT_A2DP_EVENT_CONNECT_TIMEOUT:
		 case BT_A2DP_EVENT_STREAM_SUSPEND: 	 
		 case BT_A2DP_EVENT_STREAM_START:
		 {		 
			 strncpy(t_msg.ui4_str1,
					 a2dp_event->addr,
					 MAX_BDADDR_LEN);
		 }
		 break;
				 
		 case BT_A2DP_EVENT_PLAYER_EVENT:
		 {
			 DBG_ERROR((BTAUD_TAG"player event(%d)\n", a2dp_event->data.player_event));
			 t_msg.ui4_data1 = a2dp_event->data.player_event;
		 }
		 break;

		 case BT_A2DP_EVENT_ROLE_CHANGED:
		 {
			 DBG_ERROR((BTAUD_TAG"BT_A2DP_EVENT_ROLE_CHANGED(role: %d, enable: %d).\n", 
			 					a2dp_event->data.role_change.role,
			 					a2dp_event->data.role_change.enable));
			 t_msg.ui4_data1 = a2dp_event->data.role_change.role;
			 t_msg.ui4_data2 = a2dp_event->data.role_change.enable;
		 }
		 break;
		 
		 case BT_A2DP_EVENT_CONNECT_COMING:
			 break;
 
		 default:
			 break;
	 }
 
	 t_msg.ui4_msg_type 		= BT_AUD_PTE_MSG_BT;
	 t_msg.ui4_msg_type_profile = BT_AUD_MSG_BT_A2DP_PROFILE;
	 t_msg.ui4_msg_id			= a2dp_event->event;
	 
	 _bluetooth_send_msg_to_itself(E_APP_MSG_TYPE_BLUETOOTH, &t_msg);
 }
 
 
 INT32 _bluetooth_a2dp_init(VOID)
 {
	 INT32 i4_ret = BT_AUD_FAIL;
	 char pv_tag[2]={0}; 
	 
	 BT_AUD_FUNC_ENTRY();
 
	 i4_ret = a_mtkapi_a2dp_register_callback(_bluetooth_a2dp_event_cbk_fct,(void*)pv_tag);
	 DBG_ERROR((BTAUD_TAG"a2dp register callback complete. \n"));
	 BT_AUD_CHK_FAIL_RET(a_mtkapi_a2dp_register_callback, i4_ret, BT_AUD_FAIL);
	 
	 BT_AUD_FUNC_EXIT();
	 return BT_AUD_OK;
 }
 
#if CONFIG_APP_SUPPORT_BT_SRC_MODE
 INT32 _bluetooth_a2dp_src_mas_stop_proc(VOID)
 {
	 int ret ;
 
	 ret = mas_set_device_disconnection(g_mas_handle,MT_AUDIO_OUT_BT_A2DP);
	 if(ret != 0)
	 {
		 DBG_ERROR((BTAUD_TAG"Err:mas_set_device_disconnection fail!\r\n"));
		 return BT_AUD_FAIL;
	 }
 
	 return BT_AUD_OK;
 }
 
 INT32 _bluetooth_a2dp_src_mas_start_proc(VOID)
 {
	 int ret ;
	 mas_device_config_ext_t config;
	 mas_chmap_desc_t hw_spk;
	 memset(&config,0,sizeof(config));
	 hw_spk.channels = 2;
	 hw_spk.chmap[0]= MAS_CHMAP_FL;
	 hw_spk.chmap[1]= MAS_CHMAP_FR;
	 config.rate_flags = MAS_SAMPLE_RATE_48000;
	 config.bits = 16;
	 config.hw_spk_chmap = &hw_spk;
 
	 ret = mas_set_device_connection_ext(g_mas_handle,MT_AUDIO_OUT_BT_A2DP,&config);
	 if(ret != 0)
	 {
		 DBG_ERROR((BTAUD_TAG"Err:mas_set_device_conection_ext fail!\r\n"));
		 return BT_AUD_FAIL;
	 }
 
	 return BT_AUD_OK;
 
 }
#endif /*CONFIG_APP_SUPPORT_BT_SRC_MODE*/

BT_A2DP_ROLE _bluetooth_a2dp_get_role(VOID)
{
	DBG_ERROR((BTAUD_TAG"_bluetooth_a2dp_get_role, current A2DP role is %s mode\n", 
							g_cur_a2dp_role==BT_A2DP_ROLE_SRC?"src":(g_cur_a2dp_role==BT_A2DP_ROLE_SINK?"sink":"unknown")));
	return g_cur_a2dp_role;
}

 static INT32 _bluetooth_a2dp_connect_proc(const CHAR *device_addr)
{
	INT32 i4_ret = BT_AUD_FAIL;
	CHAR device_name[MAX_NAME_LEN]={0};

	BT_AUD_FUNC_ENTRY();

	DBG_ERROR((BTAUD_TAG"_bluetooth_a2dp_connect_proc.\r\n"));

	g_t_bt_aud_sta.b_bt_a2dp_connect = TRUE;
	g_t_bt_aud_sta.e_bt_sta = BT_AUD_BT_STA_CONNECTED;

	if(TRUE==g_t_bt_aud_sta.b_bt_init_auto_connect)
	{
		DBG_ERROR((BTAUD_TAG"init auto connect succuss,touch /tmp/bt_ok.\r\n"));

		g_t_bt_aud_sta.b_bt_init_auto_connect = FALSE;
		system("touch /tmp/bt_ok");
	}

	strncpy(g_t_bt_aud_bt_status.status,
		 "connect",
		 ASSISTANT_STUB_STATUS_MAX_LENGTH);
#if 0	 
	strncpy(g_t_bt_aud_bt_status.bt_paired_name,
		 device_name,
		 ASSISTANT_STUB_STATUS_MAX_LENGTH);
#endif
	DBG_ERROR((BTAUD_TAG"A2DP connect success----send notification to ASSISTANT_STUB \n"));
	i4_ret = _bluetooth_send_bt_status_to_assistant_stub("connect");
	BT_AUD_CHK_FAIL(_bluetooth_send_bt_status_to_assistant_stub, i4_ret);

	g_t_bt_aud_sta.b_continue_connect = FALSE;//stop autoconnect when already connected with dev

	i4_ret = _bluetooth_get_remote_device_name(device_name,device_addr);
	BT_AUD_CHK_FAIL(_bluetooth_get_remote_device_name, i4_ret);

	strncpy(g_t_bt_aud_bt_status.bt_paired_name, 
			device_name,
			ASSISTANT_STUB_STATUS_MAX_LENGTH);

#if CONFIG_APP_SUPPORT_BT_BACKGROUND_CONNECTABLE
	if (TRUE == _bluetooth_is_connectable_background())
	{
		DBG_API((BTAUD_TAG"background connection, send request to SS.\r\n"));
		//bt_aud_set_bt_wob(TRUE);
	}
	else
#endif/* CONFIG_APP_SUPPORT_BT_BACKGROUND_CONNECTABLE */
	{
		//disable connect and discover
		i4_ret = a_mtkapi_bt_gap_set_connectable_and_discoverable(FALSE, FALSE);
		BT_AUD_CHK_FAIL(a_mtkapi_bt_gap_set_connectable_and_discoverable, i4_ret);

		g_cur_gap_scan_mode = 0;
	}

#if CONFIG_APP_SUPPORT_BT_STICKY_PAIRING
	if(g_t_bt_aud_sta.b_sticky_pairing_enable == TRUE)
	{
		DBG_API((BTAUD_TAG"connect with others, stop sticky pairing.\r\n"));
		_bluetooth_stop_sticky_pairing_timer();
	}
#endif/* CONFIG_APP_SUPPORT_BT_STICKY_PAIRING */	
#if CONFIG_APP_SUPPORT_BT_SRC_MODE
	/* BT in SRC Mode */
	if(BT_A2DP_ROLE_SRC == g_cur_a2dp_role)
	{
        DBG_ERROR((BTAUD_TAG BTHFP_TAG"Current A2DP role is SRC. \r\n"));
        
#if CONFIG_APP_SUPPORT_A2DP_SINK_SRC_BOTH
        if (g_t_bt_hfp_sta.b_bt_hfp_connect)
        {
            DBG_ERROR((BTAUD_TAG BTHFP_TAG"HFP is connected, start disconnect HDP device : %s .\r\n",
                                  g_cur_hfp_addr));

            i4_ret = a_mtkapi_bt_hfclient_disconnect(g_cur_hfp_addr);
            BT_AUD_CHK_FAIL_RET(a_mtkapi_bt_hfclient_disconnect, i4_ret, BT_HFP_FAIL);
        }
        else
        {
            DBG_ERROR((BTAUD_TAG BTHFP_TAG"HFP is disconnected.\r\n"));
        }
#endif /*CONFIG_APP_SUPPORT_A2DP_SINK_SRC_BOTH*/    
		g_t_bt_aud_sta.b_bt_mas_mode = BT_A2DP_MAS_OPEN;
		DBG_API((BTAUD_TAG"mas open a BT A2DP device.\r\n"));
		//mas interface for A2DP
		i4_ret = _bluetooth_a2dp_src_mas_start_proc();
		BT_AUD_CHK_FAIL(_bluetooth_a2dp_src_mas_start_proc, i4_ret);
	}
#endif /*CONFIG_APP_SUPPORT_BT_SRC_MODE*/

	BT_AUD_FUNC_EXIT();
	return BT_AUD_OK;
}

static INT32 _bluetooth_a2dp_disconnect_proc(CHAR *device_addr)
{
	INT32 i4_ret = BT_AUD_FAIL;
	CHAR device_name[MAX_NAME_LEN]={0};

	BT_AUD_FUNC_ENTRY();

	DBG_ERROR((BTAUD_TAG"_bluetooth_a2dp_disconnect_proc.\r\n"));

	_bluetooth_clear_connect_info(FALSE);
	_bluetooth_clear_status();

	strncpy(g_t_bt_aud_bt_status.status,
		  "disconnect",
		  ASSISTANT_STUB_STATUS_MAX_LENGTH);
#if 0
	strncpy(g_t_bt_aud_bt_status.bt_paired_name,
		  device_name,
		  ASSISTANT_STUB_BT_PAIRED_NAME_MAX_LENGTH);
#endif
	if(TRUE == g_t_bt_aud_sta.b_bt_source)
	{
		g_t_bt_aud_sta.b_bt_source = FALSE;

		DBG_ERROR((BTAUD_TAG"BT src is stop, send inform to SM \n"));

		i4_ret = _bluetooth_send_inform_stop_to_sm();
		BT_AUD_CHK_FAIL(_bluetooth_send_inform_stop_to_sm, i4_ret);
	}

	DBG_ERROR((BTAUD_TAG"A2DP_DISCONNECTED----send notification to ASSISTANT_STUB \n"));
	i4_ret = _bluetooth_send_bt_status_to_assistant_stub("disconnect");
	BT_AUD_CHK_FAIL(_bluetooth_send_bt_status_to_assistant_stub, i4_ret);

	i4_ret = _bluetooth_get_remote_device_name(device_name,device_addr);
	BT_AUD_CHK_FAIL(_bluetooth_get_remote_device_name, i4_ret);

	strncpy(g_t_bt_aud_bt_status.bt_paired_name, 
			device_name,
			ASSISTANT_STUB_STATUS_MAX_LENGTH);

	if (TRUE == _bluetooth_is_bt_ready())
	{
#if CONFIG_APP_SUPPORT_BT_STICKY_PAIRING
		if (g_t_bt_aud_sta.b_sticky_pairing_enable == TRUE)
		{
			//connect missing msg has come early, now change to sticky pairing
			DBG_API((BTAUD_TAG"start sticky pairing.\r\n"));

			g_t_bt_aud_sta.b_continue_connect = FALSE;
			i4_ret = _bluetooth_auto_connect(1, TRUE);
			if (i4_ret == BT_AUD_OK)
			{
			  return BT_AUD_OK;
			}

			DBG_API((BTAUD_TAG"connect fail, stop sticky pairing.\r\n"));
			_bluetooth_stop_sticky_pairing_timer();
	  	}
	  	else//no dropped abruptly, change to pairing mode
#endif/* CONFIG_APP_SUPPORT_BT_STICKY_PAIRING */
		{
			i4_ret = _bluetooth_start_pairing();
			BT_AUD_CHK_FAIL(_bluetooth_start_pairing, i4_ret);
	  	}
	}	
#if CONFIG_APP_SUPPORT_BT_SRC_MODE	
	//BT SRC mode,close MAS
	if(BT_A2DP_MAS_OPEN == g_t_bt_aud_sta.b_bt_mas_mode)
	{
		DBG_API((BTAUD_TAG"mas close a BT A2DP device.\r\n"));
		i4_ret = _bluetooth_a2dp_src_mas_stop_proc();
		BT_AUD_CHK_FAIL(_bluetooth_a2dp_src_mas_stop_proc, i4_ret);
		g_t_bt_aud_sta.b_bt_mas_mode = BT_A2DP_MAS_CLOSE;
	}
#endif /*CONFIG_APP_SUPPORT_BT_SRC_MODE*/

	BT_AUD_FUNC_EXIT();
	return BT_AUD_OK;
}

static INT32 _bluetooth_a2dp_connect_timeout_proc(const CHAR *device_name)
{
	INT32 i4_ret = BT_AUD_FAIL;

	BT_AUD_FUNC_ENTRY();

	if(TRUE==g_t_bt_aud_sta.b_bt_init_auto_connect)
	{
		 DBG_ERROR((BTAUD_TAG"init auto connect time out.\r\n"));
		 g_t_bt_aud_sta.b_bt_init_auto_connect = FALSE;
	}

#if CONFIG_APP_SUPPORT_BT_STICKY_PAIRING
	if(g_t_bt_aud_sta.b_sticky_pairing_enable == TRUE)
	{
		 DBG_API((BTAUD_TAG"continue sticky pairing.\r\n"));
		 //if sticky pairing enable, keep connect untill time out or connected with others
		 g_t_bt_aud_sta.b_continue_connect = FALSE;
		 i4_ret = _bluetooth_auto_connect(1, TRUE);
		 if (i4_ret == BT_AUD_OK)
		 {
			 return i4_ret;
		 }

		 //connect fail, stop sticky pairing
		 DBG_API((BTAUD_TAG"connect fail, stop sticky pairing.\r\n"));
		 _bluetooth_stop_sticky_pairing_timer();
	}
#endif/* CONFIG_APP_SUPPORT_BT_STICKY_PAIRING */
	if (g_t_bt_aud_sta.b_continue_connect == TRUE)//continue autoconnect,exhaust the list
	{
		 i4_ret = _bluetooth_auto_connect(0, FALSE);
		 if (i4_ret != BT_AUD_OK)	 //autoconnect fail, goto pairing mode
		 {
			 g_t_bt_aud_sta.b_continue_connect = FALSE;
		 }
	}

	if ((g_t_bt_aud_sta.b_continue_connect == FALSE)
	 	&& (FALSE == _bluetooth_is_bt_connected()))
	{
		 //g_t_bt_aud_sta.e_bt_sta = BT_AUD_BT_STA_IDLE;
		 i4_ret = _bluetooth_start_pairing();//auto connect time out then goto pairing mode
		 BT_AUD_CHK_FAIL(_bluetooth_start_pairing, i4_ret);
	}	 

	BT_AUD_FUNC_EXIT();
	return BT_AUD_OK;
}
 
INT32 _bluetooth_a2dp_player_data_come_proc(VOID)
{
	INT32 i4_ret;

	BT_AUD_FUNC_ENTRY();

	if ((BT_AUD_SYSTEM_FAKE_STANDBY == g_t_bt_aud_sta.e_bt_system_sta) 
		 ||(BT_AUD_SYSTEM_SUSPEND_STANDBY == g_t_bt_aud_sta.e_bt_system_sta))
	{
		DBG_ERROR((BTAUD_TAG"system is standby: %d , ignore data come event.\n",g_t_bt_aud_sta.e_bt_system_sta));
		g_t_bt_aud_sta.e_bt_system_sta = BT_AUD_SYSTEM_NORMAL;
	
		return BT_AUD_OK;
	}
	
	i4_ret = _bluetooth_state_play_proc();
	BT_AUD_CHK_FAIL_RET(_bluetooth_state_play_proc, i4_ret, BT_AUD_FAIL);

	BT_AUD_FUNC_EXIT();
	return BT_AUD_OK;
}

 
INT32 _bluetooth_a2dp_player_event_msg_proc(const VOID *pv_msg)
{
	INT32 i4_ret = BT_AUD_FAIL;
	BT_AUD_MSG_T* pt_a2dp_msg = (BT_AUD_MSG_T*)pv_msg;

	BT_AUD_FUNC_ENTRY();

	DBG_ERROR((BTAUD_TAG"player event msg come : (%d)\r\n", 
					 pt_a2dp_msg->ui4_data1));	 

	switch (pt_a2dp_msg->ui4_data1)
	{
		case BT_A2DP_ALSA_PB_EVENT_STOP:  /* local player STOPED	   */
		{
			DBG_ERROR((BTAUD_TAG"BT_A2DP_ALSA_PB_EVENT_STOP. \r\n"));
		}
		break;

		case BT_A2DP_ALSA_PB_EVENT_STOP_FAIL:	  /* local player stop fail   */
		{
			DBG_ERROR((BTAUD_TAG"BT_A2DP_ALSA_PB_EVENT_STOP_FAIL. \r\n"));
		}
		break;

		case BT_A2DP_ALSA_PB_EVENT_START:	  /* local player STARTED	  */
		{
			DBG_ERROR((BTAUD_TAG"BT_A2DP_ALSA_PB_EVENT_START. \r\n"));
		}
		break;

		case BT_A2DP_ALSA_PB_EVENT_START_FAIL:   /* local player start fail  */
		{
			DBG_ERROR((BTAUD_TAG"BT_A2DP_ALSA_PB_EVENT_START_FAIL. \r\n"));
		}
		break;

		case BT_A2DP_ALSA_PB_EVENT_DATA_COME:
		{
			DBG_ERROR((BTAUD_TAG"BT_A2DP_ALSA_PB_EVENT_DATA_COME. \r\n"));
			
			i4_ret = _bluetooth_a2dp_player_data_come_proc();
			BT_AUD_CHK_FAIL_RET(_bluetooth_a2dp_player_data_come_proc, i4_ret, BT_AUD_FAIL);
		}
		break;

		default:
			DBG_ERROR((BTAUD_TAG"Err:can't recognize the player event %d.\r\n",
							 pt_a2dp_msg->ui4_data1));
		 	break;
	}

	BT_AUD_FUNC_EXIT();
	return BT_AUD_OK;
}

#if !CONFIG_APP_SUPPORT_A2DP_SINK_SRC_BOTH
INT32 _bluetooth_switch_to_sink_mode_proc(VOID)
{
	INT32 i4_ret;

	BT_AUD_FUNC_ENTRY();

	if (BT_AUD_A2DP_SINK_MODE == g_t_bt_aud_sta.e_bt_a2dp_mode_switch)
	{
		DBG_ERROR((BTAUD_TAG"A2DP role(SINK) switch is going, ignore switch action. \n"));
		return BT_AUD_OK;
	}

	g_t_bt_aud_sta.e_bt_a2dp_mode_switch = BT_AUD_A2DP_SINK_MODE;

    DBG_ERROR((BTAUD_TAG" enable HFP protocol. \n"));
    i4_ret = a_mtkapi_bt_hfclient_enable();
    BT_AUD_CHK_FAIL(a_mtkapi_bt_hfclient_enable, i4_ret);

	DBG_ERROR((BTAUD_TAG" disable SRC role. \n"));
	i4_ret = a_mtkapi_a2dp_src_enable(FALSE);
	BT_AUD_CHK_FAIL(a_mtkapi_a2dp_src_enable, i4_ret);

	//u_thread_delay(300);

	DBG_ERROR((BTAUD_TAG" switch to SINK role. \n"));
	a_mtkapi_a2dp_sink_enable(TRUE);
	BT_AUD_CHK_FAIL(a_mtkapi_a2dp_sink_enable, i4_ret);

	g_cur_a2dp_role = BT_A2DP_ROLE_SINK;
#if 0
	//make bluetooth stay in discoverable and connectable after switch role
    i4_ret = a_mtkapi_bt_gap_set_connectable_and_discoverable(TRUE, TRUE);
	DBG_ERROR((BTAUD_TAG"set BT connectable and discoverable.\r\n"));
    BT_AUD_CHK_FAIL(a_mtkapi_bt_gap_set_connectable_and_discoverable, i4_ret);

	g_cur_gap_scan_mode = 2;
#endif
	BT_AUD_FUNC_EXIT();
	return BT_AUD_OK;
}

INT32 _bluetooth_switch_to_source_mode_proc(VOID)
{
	INT32 i4_ret;

	BT_AUD_FUNC_ENTRY();

	if (BT_AUD_A2DP_SOURCE_MODE == g_t_bt_aud_sta.e_bt_a2dp_mode_switch)
	{
		DBG_ERROR((BTAUD_TAG"A2DP role(SRC) switch is going, ignore switch action. \n"));
		return BT_AUD_OK;
	}

	g_t_bt_aud_sta.e_bt_a2dp_mode_switch = BT_AUD_A2DP_SOURCE_MODE;

	DBG_ERROR((BTAUD_TAG" disable HFP protocol. \n"));
	i4_ret = a_mtkapi_bt_hfclient_disable();
	BT_AUD_CHK_FAIL(a_mtkapi_bt_hfclient_disable, i4_ret);

	DBG_ERROR((BTAUD_TAG" disable SINK role. \n"));
	i4_ret = a_mtkapi_a2dp_sink_enable(FALSE);
	BT_AUD_CHK_FAIL(a_mtkapi_a2dp_sink_enable, i4_ret);

	//u_thread_delay(300);

	DBG_ERROR((BTAUD_TAG" switch to SRC role. \n"));
	i4_ret = a_mtkapi_a2dp_src_enable(TRUE);
	BT_AUD_CHK_FAIL(a_mtkapi_a2dp_src_enable, i4_ret);

	g_cur_a2dp_role = BT_A2DP_ROLE_SRC;
#if 0
	//make bluetooth stay in discoverable and connectable after switch role
    i4_ret = a_mtkapi_bt_gap_set_connectable_and_discoverable(TRUE, TRUE);
	DBG_ERROR((BTAUD_TAG"set BT connectable and discoverable.\r\n"));
    BT_AUD_CHK_FAIL(a_mtkapi_bt_gap_set_connectable_and_discoverable, i4_ret);

	g_cur_gap_scan_mode = 2;
#endif	

	BT_AUD_FUNC_EXIT();
	return BT_AUD_OK;
}

INT32 _bluetooth_reset_a2dp_mode_proc(VOID)
{
	INT32 i4_ret;
	BT_A2DP_ROLE role;

	BT_AUD_FUNC_ENTRY();

	role = _bluetooth_a2dp_get_role();

	//make bluetooth stay in no discoverable,no pairable,no connectable before switch role
	i4_ret = a_mtkapi_bt_gap_set_connectable_and_discoverable(FALSE, FALSE);
	DBG_ERROR((BTAUD_TAG"set BT no connectable and no discoverable.\r\n"));
	BT_AUD_CHK_FAIL(a_mtkapi_bt_gap_set_connectable_and_discoverable(FALSE, FALSE), i4_ret);

	g_cur_gap_scan_mode = 0;

	if(role == BT_A2DP_ROLE_SINK)
	{
		i4_ret = _bluetooth_switch_to_source_mode_proc();
		BT_AUD_CHK_FAIL(_bluetooth_switch_to_sink_mode_proc, i4_ret);
	}
	else if (role == BT_A2DP_ROLE_SRC)
	{
        i4_ret = _bluetooth_switch_to_sink_mode_proc();
        BT_AUD_CHK_FAIL(_bluetooth_switch_to_source_mode_proc, i4_ret);
	}

	role = _bluetooth_a2dp_get_role();

	BT_AUD_FUNC_EXIT();
	return BT_AUD_OK;
}
  
INT32 _bluetooth_a2dp_sink_enable_proc(VOID)
{
	INT32 i4_ret = BT_AUD_FAIL;

	BT_AUD_FUNC_ENTRY();
	 
	if (g_b_hfp_ability_enable)
	{
		if (BT_AUD_A2DP_SINK_MODE == g_t_bt_aud_sta.e_bt_a2dp_mode_switch)
		{
			g_t_bt_aud_sta.e_bt_a2dp_mode_switch = BT_AUD_A2DP_UNKNOW_MODE;
		}
		
		DBG_ERROR((BTAUD_TAG"Enable HFP successful, set BT connectable and discoverable.  \r\n"));

		//make bluetooth stay in discoverable and connectable after switch role
		i4_ret = a_mtkapi_bt_gap_set_connectable_and_discoverable(TRUE, TRUE);
		BT_AUD_CHK_FAIL(a_mtkapi_bt_gap_set_connectable_and_discoverable, i4_ret);

		g_cur_gap_scan_mode = 2;
	}
	else
	{
		DBG_ERROR((BTAUD_TAG"HFP is still disable, BT scan mode should be set later. \r\n"));
	}
		
	BT_AUD_FUNC_EXIT();
	return BT_AUD_OK;
}
 
INT32 _bluetooth_a2dp_source_enable_proc(VOID)
{
	INT32 i4_ret = BT_AUD_FAIL;

	BT_AUD_FUNC_ENTRY();

	if (!g_b_hfp_ability_enable)
	{
		if (BT_AUD_A2DP_SOURCE_MODE == g_t_bt_aud_sta.e_bt_a2dp_mode_switch)
		{
			g_t_bt_aud_sta.e_bt_a2dp_mode_switch = BT_AUD_A2DP_UNKNOW_MODE;
		}

		DBG_ERROR((BTAUD_TAG"Disable HFP successful, set BT connectable and discoverable.  \r\n"));

		//make bluetooth stay in discoverable and connectable after switch role
		i4_ret = a_mtkapi_bt_gap_set_connectable_and_discoverable(TRUE, TRUE);
		BT_AUD_CHK_FAIL(a_mtkapi_bt_gap_set_connectable_and_discoverable, i4_ret);

		g_cur_gap_scan_mode = 2;
	}
	else
	{
		DBG_ERROR((BTAUD_TAG"HFP is not enable, BT scan mode should be set later. \r\n"));
	}

	BT_AUD_FUNC_EXIT();
	return BT_AUD_OK;
}
#endif /*CONFIG_APP_SUPPORT_A2DP_SINK_SRC_BOTH*/

INT32 _bluetooth_a2dp_role_change_event_msg_proc(const VOID *pv_msg)
{
	INT32 i4_ret = BT_AUD_FAIL;
	BT_AUD_MSG_T* pt_a2dp_msg = (BT_AUD_MSG_T*)pv_msg;

	BT_AUD_FUNC_ENTRY();

	DBG_ERROR((BTAUD_TAG"a2dp role change event msg come : (%d)\r\n", 
				  pt_a2dp_msg->ui4_data1)); 
	 
	switch (pt_a2dp_msg->ui4_data1)
	{
		case BT_A2DP_ROLE_SRC:    /* Source role */
		{
			if (pt_a2dp_msg->ui4_data2)
			{
				DBG_ERROR((BTAUD_TAG"Enable SRC role successful. \r\n"));
#if !CONFIG_APP_SUPPORT_A2DP_SINK_SRC_BOTH              
				i4_ret = _bluetooth_a2dp_source_enable_proc();
				BT_AUD_CHK_FAIL(_bluetooth_a2dp_source_enable_proc, i4_ret);
#endif /*CONFIG_APP_SUPPORT_A2DP_SINK_SRC_BOTH*/
			}
			else
			{
				DBG_ERROR((BTAUD_TAG"SRC role is disable. \r\n"));
			}
		}
		break;

		case BT_A2DP_ROLE_SINK:    /* Sink role   */
		{
			if (pt_a2dp_msg->ui4_data2)
			{
				DBG_ERROR((BTAUD_TAG"Enable SINK role successful. \r\n"));
#if !CONFIG_APP_SUPPORT_A2DP_SINK_SRC_BOTH
				i4_ret = _bluetooth_a2dp_sink_enable_proc();
				BT_AUD_CHK_FAIL(_bluetooth_a2dp_sink_enable_proc, i4_ret);
#endif /*CONFIG_APP_SUPPORT_A2DP_SINK_SRC_BOTH*/
			}
			else
			{
				DBG_ERROR((BTAUD_TAG"SINK role is disable. \r\n"));
			}
		}
		break;

		default:
		 DBG_ERROR((BTAUD_TAG"Err:can't recognize the a2dp role disable event %d.\r\n",
						  pt_a2dp_msg->ui4_data1));
		 break;
	}

	BT_AUD_FUNC_EXIT();
	return BT_AUD_OK;
}

INT32 _bluetooth_a2dp_event_proc(const VOID *pv_msg)
{
	INT32 i4_ret = BT_AUD_FAIL;
	BT_AUD_MSG_T* pt_bt_a2dp_msg = (BT_AUD_MSG_T*)pv_msg;
 
	BT_AUD_FUNC_ENTRY();
 
	DBG_ERROR((BTAUD_TAG"A2DP event msg come : (%d)\r\n", 
						 pt_bt_a2dp_msg->ui4_msg_id));	 
	 
	switch(pt_bt_a2dp_msg->ui4_msg_id)
	{
		case BT_A2DP_EVENT_CONNECTED:
		{
			DBG_ERROR((BTAUD_TAG"BT_A2DP_EVENT_CONNECTED (device addr : %s). \r\n",
								 pt_bt_a2dp_msg->ui4_str1));
			 
			i4_ret = _bluetooth_a2dp_connect_proc(pt_bt_a2dp_msg->ui4_str1);
			BT_AUD_CHK_FAIL_RET(_bluetooth_a2dp_connect_proc, i4_ret, BT_AUD_FAIL);
		}
		break;
		 
		case BT_A2DP_EVENT_DISCONNECTED:
		{
			DBG_ERROR((BTAUD_TAG"BT_A2DP_EVENT_DISCONNECTED (device addr : %s). \r\n",
								 pt_bt_a2dp_msg->ui4_str1));
			 
			i4_ret = _bluetooth_a2dp_disconnect_proc(pt_bt_a2dp_msg->ui4_str1);
			BT_AUD_CHK_FAIL_RET(_bluetooth_a2dp_disconnect_proc, i4_ret, BT_AUD_FAIL);
		}
		break;
 
		case BT_A2DP_EVENT_CONNECT_TIMEOUT:
		{
			DBG_ERROR((BTAUD_TAG"BT_A2DP_EVENT_CONNECT_TIMEOUT. \r\n"));
			i4_ret = _bluetooth_a2dp_connect_timeout_proc(pt_bt_a2dp_msg->ui4_str1);
			BT_AUD_CHK_FAIL_RET(_bluetooth_state_play_proc, i4_ret, BT_AUD_FAIL);
		}
		break;
		 
		case BT_A2DP_EVENT_STREAM_SUSPEND:
		{
			DBG_ERROR((BTAUD_TAG"BT_A2DP_EVENT_STREAM_SUSPEND (device addr : %s). \r\n",
								 pt_bt_a2dp_msg->ui4_str1));
			 
			i4_ret = _bluetooth_state_pause_proc();
			BT_AUD_CHK_FAIL_RET(_bluetooth_state_play_proc, i4_ret, BT_AUD_FAIL);
		}
		break;
		 
		case BT_A2DP_EVENT_STREAM_START:
		{
			DBG_ERROR((BTAUD_TAG"BT_A2DP_EVENT_STREAM_START (device addr : %s). \r\n",
								 pt_bt_a2dp_msg->ui4_str1));
			 
			i4_ret = _bluetooth_state_play_proc();
			BT_AUD_CHK_FAIL_RET(_bluetooth_state_play_proc, i4_ret, BT_AUD_FAIL);
		}
		break;
 
		case BT_A2DP_EVENT_PLAYER_EVENT:
		{
			DBG_ERROR((BTAUD_TAG"BT_A2DP_EVENT_PLAYER_EVENT. \r\n"));
								 
			i4_ret = _bluetooth_a2dp_player_event_msg_proc(pt_bt_a2dp_msg);
			BT_AUD_CHK_FAIL_RET(_bluetooth_a2dp_player_event_msg_proc, i4_ret, BT_AUD_FAIL);
		}
		break;
 
		case BT_A2DP_EVENT_CONNECT_COMING:
		{
			DBG_ERROR((BTAUD_TAG"BT_A2DP_EVENT_CONNECT_COMING. \r\n"));
		}
		break;

		case BT_A2DP_EVENT_ROLE_CHANGED:
		{
			DBG_ERROR((BTAUD_TAG"BT_A2DP_EVENT_ROLE_CHANGED. \r\n"));
			 
			i4_ret = _bluetooth_a2dp_role_change_event_msg_proc(pt_bt_a2dp_msg);
			BT_AUD_CHK_FAIL_RET(_bluetooth_a2dp_role_change_event_msg_proc, i4_ret, BT_AUD_FAIL);
		}
		break;		 
 
		default:
			DBG_ERROR((BTAUD_TAG"Err:can't recognize the A2DP event %d.\r\n", 
								 pt_bt_a2dp_msg->ui4_msg_id));
			break;
	}
 
	BT_AUD_FUNC_EXIT();
	return BT_AUD_OK;
}

