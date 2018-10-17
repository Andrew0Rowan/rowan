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
#include <semaphore.h>
#include <sys/ipc.h>
#include <sys/sem.h>

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
#include "Interface.h"
#include "u_user_interface.h"


/*    btmw   */
#include "c_bt_mw_a2dp_snk.h"
#include "bt_a2dp_alsa_playback.h"
#include "c_bt_mw_gap.h"
#include "mtk_bt_service_gap_wrapper.h"
#include "mtk_bt_service_a2dp_wrapper.h"
#include "mtk_bt_service_avrcp_wrapper.h"
#include "mtk_bt_service_hfclient_wrapper.h"

/* private */
#include "u_bluetooth_audio.h"
#include "bluetooth_audio_proc.h"
#include "bluetooth_audio_key_proc.h"
#include "bluetooth_audio.h"
#include "bluetooth_hfp.h"
#include "bluetooth_hfp_proc.h"

#if (CONFIG_APP_SUPPORT_BLE_GATT_CLIENT || CONFIG_APP_SUPPORT_BLE_GATT_SERVER)
#include "bluetooth_gatt.h"
#endif


/*-----------------------------------------------------------------------------
 * structure, constants, macro definitions
 *---------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------
 * variable declarations
 *---------------------------------------------------------------------------*/
BT_HFP_CBK_STATUS_T g_t_bt_hfp_cbk_sta = {0};
CHAR g_cur_hfp_addr[18]={0};
UINT32 g_pb_cur_index = 0;
INT32 g_i_cur_call = 0;
UINT16 g_bt_hfp_sm_parameter_id = 0;
BOOL g_b_hfp_ability_enable = FALSE;


extern BT_HFP_STATUS_T g_t_bt_hfp_sta;
extern BT_HFP_OBJ_T g_t_g_bt_hfp;
extern BT_AUD_BLUETOOTH_STATUS_T g_t_bt_aud_sta;
extern UINT8 g_cur_gap_scan_mode;
extern BT_A2DP_ROLE g_cur_a2dp_role;

#if 0

union semun{
    int val;
	struct semid_ds *buf;
	unsigned short *array;
};

union semun hfp_arg;
key_t hfp_key = 0;
int hfp_semid;

#endif

/*-----------------------------------------------------------------------------
 * private methods declarations
 *---------------------------------------------------------------------------*/




INT32 _bluetooth_hfp_init(VOID)
 {
	 INT32 i4_ret = BT_AUD_FAIL;
	 char pv_tag[2]={0}; 
	 
	 BT_AUD_FUNC_ENTRY();
	 
    if (!g_t_bt_hfp_sta.b_bt_hfp_init)
    {
		 i4_ret = a_mtkapi_bt_hfclient_register_callback(_bt_hfp_event_cbk_fct,(void*)pv_tag);
		 DBG_ERROR((BTAUD_TAG"hfp register callback complete. \n"));
		 BT_AUD_CHK_FAIL_RET(a_mtkapi_bt_hfclient_register_callback, i4_ret, BT_AUD_FAIL);

		 memset(&g_t_bt_hfp_sta, 0, sizeof(BT_HFP_STATUS_T));
		 g_t_bt_hfp_sta.b_bt_hfp_init = TRUE;
		 DBG_ERROR((BTAUD_TAG"_bt_hfp_init done. b_bt_hfp_init=%d.\r\n", g_t_bt_hfp_sta.b_bt_hfp_init));
    }
	
	BT_AUD_FUNC_EXIT();
	return BT_AUD_OK;
 }

VOID _bt_hfp_event_cbk_fct(BT_HFCLIENT_EVENT_PARAM *hfp_event, VOID *pv_tag)
{
	BT_HFP_MSG_T t_msg = {0};
    BT_HFP_FUNC_ENTRY();

	if (NULL == hfp_event)
	{
		DBG_ERROR((BTAUD_TAG"hfp_event is NULL\n"));
		return;
	}

	DBG_ERROR((BTHFP_TAG"hfpclient cb , hf_event = %d.\r\n",hfp_event->event));

	switch (hfp_event->event)
	{
		case BTAPP_HFCLIENT_ABILITY_CB_EVT:          /*HFP interface enable/disable event*/
		{
            t_msg.ui4_data1 = hfp_event->data.ability_cb.state;					 
		}
		break;

		case BTAPP_HFCLIENT_CONNECTION_CB_EVT:
		{
            t_msg.ui4_data1 = hfp_event->data.connect_cb.state;					 
			strcpy(t_msg.ui4_str,hfp_event->data.connect_cb.addr);
		}
		break;
		
		case BTAPP_HFCLIENT_AUDIO_CONNECTION_CB_EVT:
		{
            t_msg.ui4_data1 = hfp_event->data.auido_connect_cb.state;
		}
		break;

		case BTAPP_HFCLIENT_BVRA_CB_EVT:
		{
			t_msg.ui4_data1 = hfp_event->data.bvra_cb.vr_state;
		}
		break;

		case BTAPP_HFCLIENT_IND_SERVICE_CB_EVT:
		{
			t_msg.ui4_data1 = hfp_event->data.service_cb.network_state;
			g_t_bt_hfp_cbk_sta.network_state = hfp_event->data.service_cb.network_state;
		}
		break;

		case BTAPP_HFCLIENT_IND_ROAM_CB_EVT:
		{
			t_msg.ui4_data1 = hfp_event->data.roam_cb.service_type;
			g_t_bt_hfp_cbk_sta.service_type = hfp_event->data.roam_cb.service_type;
		}
		break;

		case BTAPP_HFCLIENT_IND_SIGNAL_CB_EVT:
		{
			t_msg.ui4_data1 = hfp_event->data.signal_cb.signal_strength;
			g_t_bt_hfp_cbk_sta.signal_strength = hfp_event->data.signal_cb.signal_strength;
		}
		break;

		case BTAPP_HFCLIENT_IND_BATTCH_CB_EVT:
		{
			t_msg.ui4_data1 = hfp_event->data.battery_cb.battery_level;
			g_t_bt_hfp_cbk_sta.battery_level = hfp_event->data.battery_cb.battery_level;
		}
		break;

		case BTAPP_HFCLIENT_COPS_CB_EVT:
		{
			strcpy(t_msg.ui4_str, hfp_event->data.cops_cb.operator_name);
			strcpy(g_t_bt_hfp_cbk_sta.operator_name, hfp_event->data.cops_cb.operator_name);
		}
		break;

		case BTAPP_HFCLIENT_IND_CALL_CB_EVT:
		{
			t_msg.ui4_data1 = hfp_event->data.call_cb.call;
			g_t_bt_hfp_cbk_sta.call = hfp_event->data.call_cb.call;
		}
		break;

		case BTAPP_HFCLIENT_IND_CALLSETUP_CB_EVT:
		{
			t_msg.ui4_data1 = hfp_event->data.callsetup_cb.callsetup;
			g_t_bt_hfp_cbk_sta.callsetup = hfp_event->data.callsetup_cb.callsetup;
		}
		break;

		case BTAPP_HFCLIENT_IND_CALLHELD_CB_EVT:
		{
			t_msg.ui4_data1 = hfp_event->data.callheld_cb.callheld;
			g_t_bt_hfp_cbk_sta.callheld = hfp_event->data.callheld_cb.callheld;
		}
		break;

		case BTAPP_HFCLIENT_BTRH_CB_EVT:
		{
			t_msg.ui4_data1 = hfp_event->data.btrh_cb.resp_and_hold;
			g_t_bt_hfp_cbk_sta.resp_and_hold = hfp_event->data.btrh_cb.resp_and_hold;
		}
		break;

		case BTAPP_HFCLIENT_CLIP_CB_EVT:
		{
			strcpy(t_msg.ui4_str, hfp_event->data.clip_cb.number);
			strcpy(g_t_bt_hfp_cbk_sta.clip_number, hfp_event->data.clip_cb.number);
		}
		break;

		case BTAPP_HFCLIENT_CCWA_CB_EVT:
		{
			strcpy(t_msg.ui4_str, hfp_event->data.ccwa_cb.number);
			strcpy(g_t_bt_hfp_cbk_sta.call_waiting_number, hfp_event->data.ccwa_cb.number);
		}
		break;

		case BTAPP_HFCLIENT_CLCC_CB_EVT:
		{
			DBG_ERROR((BTHFP_TAG"g_i_cur_call : %d .\r\n",g_i_cur_call));

            g_t_bt_hfp_sta.b_bt_hfp_query_calls = TRUE ;
			memcpy(&(g_t_bt_hfp_cbk_sta.current_calls[g_i_cur_call]), 
					&(hfp_event->data.clcc_cb), 
					sizeof(BT_HFCLIENT_CLCC_CB_DATA_T));
			
			g_i_cur_call++;
		}
		break;

		case BTAPP_HFCLIENT_VGM_VGS_CB_EVT:
		{
			t_msg.ui4_data1 = hfp_event->data.volume_cb.type;
			t_msg.ui4_data2 = hfp_event->data.volume_cb.volume;
			g_t_bt_hfp_cbk_sta.volume_change.type = hfp_event->data.volume_cb.type;
			g_t_bt_hfp_cbk_sta.volume_change.volume = hfp_event->data.volume_cb.volume;			
		}
		break;

		case BTAPP_HFCLIENT_CMD_COMPLETE_CB_EVT:
		{
			DBG_ERROR((BTHFP_TAG"BTAPP_HFCLIENT_CMD_COMPLETE_CB_EVT.\r\n"));
		}
		break;
		
		case BTAPP_HFCLIENT_CNUM_CB_EVT:
		{
			t_msg.ui4_data1 = hfp_event->data.cnum_cb.type;					 
			strcpy(t_msg.ui4_str,hfp_event->data.cnum_cb.number);
			g_t_bt_hfp_cbk_sta.subscriber_info.type = hfp_event->data.cnum_cb.type;	
			strcpy(g_t_bt_hfp_cbk_sta.subscriber_info.number, hfp_event->data.cnum_cb.number);
			
		}
		break;

		case BTAPP_HFCLIENT_BSIR_CB_EVT:
		{
			t_msg.ui4_data1 = hfp_event->data.bsir_cb.state;
			g_t_bt_hfp_cbk_sta.inband_ring_state = hfp_event->data.bsir_cb.state;
		}
		break;

		case BTAPP_HFCLIENT_BINP_CB_EVT:
		{
			strcpy(t_msg.ui4_str, hfp_event->data.binp_cb.number);
			strcpy(g_t_bt_hfp_cbk_sta.voice_tag_number, hfp_event->data.binp_cb.number);
		}
		break;

		case BTAPP_HFCLIENT_RING_IND_CB_EVT:
		break;
#if CONFIG_SUPPORT_PHONE_BOOK
		case BTAPP_HFCLIENT_CPBR_ENTRY_EVT:
		{
			if(!g_pb_cur_index)
			{
				DBG_ERROR((BTHFP_TAG"BT_HFCLIENT_CPBR_ENTRY.\r\n"));
			}
			else
			{
				DBG_ERROR(("<HFP_APP_PB> BT_HFCLIENT_CPBR_ENTRY.\r\n"));
			}
//			t_msg.ui4_msg_type	= BT_HFP_PTE_MSG_HFP;
//			   t_msg.ui4_msg_id    = BT_HFP_MSG_CPBR_ENTRY;

//			DBG_ERROR((BTHFP_TAG"pb_cur_index: %d .\r\n",pb_cur_index));

			g_t_bt_hfp_cbk_sta.pb_info.index = g_pb_cur_index;
//			strcpy(g_t_bt_hfp_cbk_sta.pb_info.hfp_pb_entry_info.name,pt_hfclient_struct->u.pb_entry_info.name);
//			strcpy(g_t_bt_hfp_cbk_sta.pb_info.hfp_pb_entry_info.number,pt_hfclient_struct->u.pb_entry_info.number);

			//TODO:here should send each phonebook record to ali-stub

			g_pb_cur_index++;
			
			DBG_ERROR(("<HFP_APP_PB> phonebook current index info:\r\n"));
			DBG_ERROR(("<HFP_APP_PB> index : %d , name : %s , number : %s .\r\n",
					 			g_t_bt_hfp_cbk_sta.pb_info.index,
								hfp_event->data.pb_entry_app.name,
								hfp_event->data.pb_entry_app.number)); 
#if 0
			DBG_ERROR((BTHFP_TAG"<HFP_APP_PB> index : %d , name : %s , number : %s .\r\n",
								 g_t_bt_hfp_cbk_sta.pb_info.index,
								 g_t_bt_hfp_cbk_sta.pb_info.hfp_pb_entry_info.name,
								 g_t_bt_hfp_cbk_sta.pb_info.hfp_pb_entry_info.number));	
#endif
		}
		break;

		case BTAPP_HFCLIENT_CPBR_READY_EVT:
	    {		
			DBG_ERROR((BTHFP_TAG"BTAPP_HFCLIENT_CPBR_READY_EVT.\r\n"));

			INT32 i4_ret = BT_HFP_FAIL;
			//MW phonebook is ready , then APP read phonebook from MW
			i4_ret = a_mtkapi_bt_hfclient_read_pb_entries();
			DBG_ERROR((BTHFP_TAG"a_mtkapi_bt_hfclient_read_pb_entries \r\n"));
			BT_HFP_CHK_FAIL_RET(a_mtkapi_bt_hfclient_read_pb_entries, i4_ret, BT_HFP_FAIL);
		}
		break;
		
		case BTAPP_HFCLIENT_CPBR_DONE_EVT:
		{
			DBG_ERROR((BTHFP_TAG"BT_HFCLIENT_CPBR_DONE.\r\n"));
			DBG_ERROR(("<HFP_APP_PB> total phonebook index : %d .\r\n",g_pb_cur_index));
		}
		break;
#endif
		default:
			break;
	}
	
	t_msg.ui4_msg_type  = BT_HFP_PTE_MSG_HFP;
	t_msg.ui4_msg_id    = hfp_event->event;

    _bt_hfp_send_msg_to_itself(E_APP_MSG_TYPE_BLUETOOTH_HFP, &t_msg);

	BT_HFP_FUNC_EXIT();
}

BT_HFP_CBK_STATUS_T* _bt_hfp_get_cbk_status(VOID)
{
    BT_HFP_FUNC_ENTRY();
	
	return &g_t_bt_hfp_cbk_sta;
	
	BT_HFP_FUNC_EXIT();
}

INT32 _bt_hfp_cbk_status_print(VOID)
{
    BT_HFP_CBK_STATUS_T *hfp_cbk_sta;

    CHAR* conn_state[]={"HFP_DISCONNECTED","HFP_CONNCETED"};
    CHAR* audio_state[]={"AUDIO_DISCONNECTED","AUDIO_CONNCETED(CVSD)","AUDIO_CONNCETED(MSBC)"};
    CHAR* vr_state[]={"STOPPED", "STARTED"};
    CHAR* network_state[]={"NOT_AVAILABLE", "AVAILABLE"};
    CHAR* service_type[]={"HOME", "ROAMING"};
    CHAR* call[]={"NO_CALLS_IN_PROGRESS","CALLS_IN_PROGRESS"};
    CHAR* callheld[]={"NONE", "HOLD_AND_ACTIVE", "HOLD"};
    CHAR* callsetup[]={"NONE", "INCOMING", "OUTGOING", "ALERTING"};
    CHAR* resp_and_hold[]={"HELD", "ACCEPT", "REJECT"};
    CHAR* current_calls_dir[]={"OUTGOING", "INCOMING"};
    CHAR* current_calls_state[]={"ACTIVE", "HELD", "DIALING", "ALERTING", "INCOMING", "WAITING", "HELD_BY_RESP_HOLD"};
    CHAR* current_calls_mpty[]={"SINGLE", "MULTI"};
    CHAR* volume_change_type[]={"SPK", "MIC"};
    CHAR* cmd_complete_type[]={"OK", "ERROR", "NO_CARRIER", "BUSY", "NO_ANSWER", "DELAYED", "BLACKLISTED", "CME"};
    CHAR* subscriber_info_type[]={"UNKNOWN", "VOICE", "FAX"};
    CHAR* inband_ring_state[]={"NOT_PROVIDED", "PROVIDED"};

	BT_HFP_FUNC_ENTRY();

	hfp_cbk_sta = _bt_hfp_get_cbk_status();

	DBG_ERROR((BTHFP_TAG"hfp_cbk_sta : \r\n"));

    DBG_ERROR((BTHFP_TAG"conn_state=%s.\r\n",conn_state[hfp_cbk_sta->conn_state]));
    DBG_ERROR((BTHFP_TAG"audio_state=%s.\r\n",audio_state[hfp_cbk_sta->audio_state]));
	DBG_ERROR((BTHFP_TAG"vr_state=%s.\r\n",vr_state[hfp_cbk_sta->vr_state]));
	DBG_ERROR((BTHFP_TAG"network_state=%s.\r\n",network_state[hfp_cbk_sta->network_state]));
	DBG_ERROR((BTHFP_TAG"service_type=%s.\r\n",service_type[hfp_cbk_sta->service_type]));
    DBG_ERROR((BTHFP_TAG"signal_strength(0~5)=%d.\r\n",hfp_cbk_sta->signal_strength));
    DBG_ERROR((BTHFP_TAG"battery_level(0~5)=%d.\r\n",hfp_cbk_sta->battery_level));     //5
	DBG_ERROR((BTHFP_TAG"operator_name=%s.\r\n",hfp_cbk_sta->operator_name));
	DBG_ERROR((BTHFP_TAG"call=%s.\r\n",call[hfp_cbk_sta->call]));
	DBG_ERROR((BTHFP_TAG"callheld=%s.\r\n",callheld[hfp_cbk_sta->callheld]));
	DBG_ERROR((BTHFP_TAG"callsetup=%s.\r\n",callsetup[hfp_cbk_sta->callsetup]));
	DBG_ERROR((BTHFP_TAG"resp_and_hold=%s.\r\n",resp_and_hold[hfp_cbk_sta->resp_and_hold]));    //10
	DBG_ERROR((BTHFP_TAG"clip_number=%s.\r\n",hfp_cbk_sta->clip_number));
	DBG_ERROR((BTHFP_TAG"call_waiting_number=%s.\r\n",hfp_cbk_sta->call_waiting_number));

    if(TRUE == g_t_bt_hfp_sta.b_bt_hfp_query_calls)
    {
        g_t_bt_hfp_sta.b_bt_hfp_query_calls = FALSE;

        DBG_ERROR((BTHFP_TAG"current_calls.index=%d, current_calls.dir=%s, current_calls.state=%s, current_calls.mpty=%s, current_calls.number=%s.\r\n",
		                     hfp_cbk_sta->current_calls[g_i_cur_call-1].index,
		                     current_calls_dir[hfp_cbk_sta->current_calls[g_i_cur_call-1].dir],
		                     current_calls_state[hfp_cbk_sta->current_calls[g_i_cur_call-1].state],
		                     current_calls_mpty[hfp_cbk_sta->current_calls[g_i_cur_call-1].mpty],
		                     hfp_cbk_sta->current_calls[g_i_cur_call-1].number));
    }

    DBG_ERROR((BTHFP_TAG"volume_spk(0~15)=%d.\r\n",
		                hfp_cbk_sta->spk_volume));
	DBG_ERROR((BTHFP_TAG"volume_mic(0~15)=%d.\r\n",
		                hfp_cbk_sta->mic_volume));
//	DBG_ERROR((BTHFP_TAG"cmd_complete.type=%s, cmd_complete.type=%s.\r\n",
//		                cmd_complete_type[hfp_cbk_sta->cmd_complete.type],
//		                hfp_cbk_sta->cmd_complete.cme));    //15
	DBG_ERROR((BTHFP_TAG"subscriber_info.number=%s, subscriber_info.type=%s.\r\n",
		                hfp_cbk_sta->subscriber_info.number,
		                subscriber_info_type[hfp_cbk_sta->subscriber_info.type]));
	DBG_ERROR((BTHFP_TAG"inband_ring_state=%s.\r\n",inband_ring_state[hfp_cbk_sta->inband_ring_state]));
	DBG_ERROR((BTHFP_TAG"voice_tag_number=%s.\r\n",hfp_cbk_sta->voice_tag_number));
//	DBG_ERROR((BTHFP_TAG"BT_HFCLIENT_RING_INDICATION.\r\n"));

	BT_HFP_FUNC_EXIT();
	return BT_HFP_OK;

}

static void _bt_hfp_player_msg_callback(int msg, int ext1, int ext2)
{
    INT32 i4_ret;

	BT_HFP_FUNC_ENTRY();

    DBG_ERROR((BTHFP_TAG"callback msg:[%d] ext1:[%d] ext2:[%d]\n", msg, ext1, ext2));
    switch (msg)
    {
        case MEDIA_PLAYBACK_COMPLETE:
        {
            DBG_ERROR((BTHFP_TAG"MEDIA_PLAYBACK_COMPLETE \r\n"));
		}
        break;

        case MEDIA_ERROR:
        {
            DBG_ERROR((BTHFP_TAG"MEDIA_ERROR \r\n"));

			g_t_bt_hfp_sta.b_bt_hfp_media_prepared = FALSE;
		}
        break;

        case MEDIA_PREPARED:
		{
            DBG_ERROR((BTHFP_TAG"MEDIA_PREPARED \r\n"));

			g_t_bt_hfp_sta.b_bt_hfp_media_prepared = TRUE;

            i4_ret = start(g_t_g_bt_hfp.h_player);
	        BT_HFP_CHK_FAIL(start, i4_ret);

            g_t_bt_hfp_sta.b_bt_hfp_media_play = TRUE;
            DBG_ERROR((BTAUD_TAG BTHFP_TAG"g_t_bt_hfp_sta.b_bt_hfp_media_play : %d \r\n",g_t_bt_hfp_sta.b_bt_hfp_media_play));

		}
        break;
#if 0
            start(g_t_prompt.h_player);
            _playback_prompt_inform_playback_status_to_sm(SM_BODY_PLAY);
            break;
#endif

        case MEDIA_STOPPED:
        {
            DBG_ERROR((BTHFP_TAG"MEDIA_STOPPED \r\n"));

			g_t_bt_hfp_sta.b_bt_hfp_media_stop = TRUE;
		}
        break;

        default:
            break;
    }

	BT_HFP_FUNC_EXIT();
}
INT32 _bt_hfp_player_init(VOID)
{
    AudioFormat t_hfp_output_format = {0};

	BT_HFP_FUNC_ENTRY();

    g_t_g_bt_hfp.h_player = player_init();
	if (NULL == g_t_g_bt_hfp.h_player)
	{
		DBG_ERROR((BTHFP_TAG"player init failed! \r\n"));
		return BT_HFP_FAIL;
	}

    t_hfp_output_format.mChannelLayout = AV_CH_LAYOUT_STEREO;
    t_hfp_output_format.mSampleFmt = AV_SAMPLE_FMT_S16;
    t_hfp_output_format.mSampleRate = 44100;
    t_hfp_output_format.mChannels = 2;
    memcpy(t_hfp_output_format.mDeviceName, BT_HFP_DEVICE_NAME, strlen(BT_HFP_DEVICE_NAME));
    setPlaybackSettings(g_t_g_bt_hfp.h_player, &t_hfp_output_format);

    registerCallback(g_t_g_bt_hfp.h_player, _bt_hfp_player_msg_callback);

	BT_HFP_FUNC_EXIT();

    return BT_HFP_OK;
}

VOID _bt_hfp_player_deinit(VOID)
{
    BT_HFP_FUNC_ENTRY();

	reset(g_t_g_bt_hfp.h_player);
    player_deinit(g_t_g_bt_hfp.h_player);

	BT_HFP_FUNC_EXIT();
}

INT32 _bt_hfp_player_stop(VOID)
{
    INT32 i4_ret;

	BT_HFP_FUNC_ENTRY();

    DBG_ERROR((BTHFP_TAG"_bt_hfp_player_stop \r\n"));
    g_t_bt_hfp_sta.b_bt_hfp_media_play =FALSE;
    DBG_ERROR((BTAUD_TAG BTHFP_TAG"g_t_bt_hfp_sta.b_bt_hfp_media_play : %d \r\n",
									g_t_bt_hfp_sta.b_bt_hfp_media_play));

    i4_ret = stop(g_t_g_bt_hfp.h_player);  //stop media play
    BT_HFP_CHK_FAIL(stop, i4_ret);

    i4_ret = reset(g_t_g_bt_hfp.h_player);  //stop media play
	BT_HFP_CHK_FAIL(reset, i4_ret);

    player_deinit(g_t_g_bt_hfp.h_player);;  //stop media play

	BT_HFP_FUNC_EXIT();
    return BT_HFP_OK;

}

INT32 u_bt_hfp_set_absolute_volume(UINT8 ui1_value)
{
    INT32 i4_ret = 0;
    BOOL  b_vol_mute = FALSE;

	BT_HFP_FUNC_EXIT();

    /*call acfg api to set absolute volume*/
    DBG_INFO((BTHFP_TAG"set absolute volume, volume=%d\n",ui1_value));

    /* get current mute status*/
    i4_ret = u_acfg_get_mute(&b_vol_mute);
    if (APP_CFGR_OK != i4_ret)
    {
        DBG_ERROR((BTHFP_TAG"a_cfg_get_mute error in function %s\n", __FUNCTION__));
        return BT_HFP_FAIL;
    }

    if (b_vol_mute)
    {
        i4_ret = u_acfg_set_mute(FALSE);
        if (APP_CFGR_OK != i4_ret)
        {
            DBG_ERROR((BTHFP_TAG"u_acfg_set_mute error in function %s\n", __FUNCTION__));
            return BT_HFP_FAIL;
        }
    }

    /*change volume value, set to SCC*/
    i4_ret = u_acfg_set_volume((UINT8)ui1_value,TRUE);
    if (APP_CFGR_OK != i4_ret)
    {
        DBG_ERROR((BTHFP_TAG"u_acfg_set_volume error in function %s\n", __FUNCTION__));
        return BT_HFP_FAIL;
    }

	BT_HFP_FUNC_EXIT();
    return BT_HFP_OK;
}

#if !CONFIG_APP_SUPPORT_A2DP_SINK_SRC_BOTH
INT32 _bt_hfp_event_hfclient_enable_proc(VOID)
{
    INT32 i4_ret;
	
    BT_HFP_FUNC_ENTRY();

	g_b_hfp_ability_enable = TRUE;

	if (BT_A2DP_ROLE_SRC== g_t_bt_aud_sta.e_bt_a2dp_role_disable)
	{
		if (BT_AUD_A2DP_SINK_MODE == g_t_bt_aud_sta.e_bt_a2dp_mode_switch)
		{
			g_t_bt_aud_sta.e_bt_a2dp_mode_switch = BT_AUD_A2DP_UNKNOW_MODE;
		}
				
		DBG_ERROR((BTAUD_TAG BTHFP_TAG"Disable A2DP SRC role successful, set BT connectable and discoverable.  \r\n"));

		//make bluetooth stay in discoverable and connectable after switch role
		i4_ret = a_mtkapi_bt_gap_set_connectable_and_discoverable(TRUE, TRUE);
		BT_AUD_CHK_FAIL(a_mtkapi_bt_gap_set_connectable_and_discoverable, i4_ret);
		
		g_cur_gap_scan_mode = 2;
	}
	else
	{
		DBG_ERROR((BTAUD_TAG BTHFP_TAG"A2DP SRC role disable is not ready, BT scan mode should be set later. \r\n"));
	}

    BT_HFP_FUNC_EXIT();
    return BT_HFP_OK;
}

INT32 _bt_hfp_event_hfclient_disable_proc(VOID)
{
    INT32 i4_ret;
	
    BT_HFP_FUNC_ENTRY();

	g_b_hfp_ability_enable = FALSE;

	if(BT_A2DP_ROLE_SINK == g_t_bt_aud_sta.e_bt_a2dp_role_disable)
	{
		if (BT_AUD_A2DP_SOURCE_MODE == g_t_bt_aud_sta.e_bt_a2dp_mode_switch)
		{
			g_t_bt_aud_sta.e_bt_a2dp_mode_switch = BT_AUD_A2DP_UNKNOW_MODE;
		}
				
		DBG_ERROR((BTAUD_TAG BTHFP_TAG"Disable A2DP SINK role successful, set BT connectable and discoverable.  \r\n"));

		//make bluetooth stay in discoverable and connectable after switch role
		i4_ret = a_mtkapi_bt_gap_set_connectable_and_discoverable(TRUE, TRUE);
		BT_AUD_CHK_FAIL(a_mtkapi_bt_gap_set_connectable_and_discoverable, i4_ret);
		
		g_cur_gap_scan_mode = 2;
	}
	else
	{
		DBG_ERROR((BTAUD_TAG BTHFP_TAG"A2DP SRC role disable is not ready, BT scan mode should be set later.  \r\n"));
	}

    BT_HFP_FUNC_EXIT();
    return BT_HFP_OK;
}
#endif /*CONFIG_APP_SUPPORT_A2DP_SINK_SRC_BOTH*/

INT32 _bt_hfp_event_hfclient_ability_proc(BT_HFP_MSG_T *pv_msg)
{
    INT32 i4_ret;
    BT_HFP_MSG_T* pt_hfp_msg = (BT_HFP_MSG_T*)pv_msg;
	
    BT_HFP_FUNC_ENTRY();

    switch (pt_hfp_msg->ui4_data1)
    {
#if CONFIG_SUPPORT_BT_HFP
        case BT_HFCLIENT_ENABLE:
        {
			DBG_ERROR((BTAUD_TAG BTHFP_TAG"BT_HFCLIENT_ENABLE \r\n"));
#if !CONFIG_APP_SUPPORT_A2DP_SINK_SRC_BOTH
			i4_ret = _bt_hfp_event_hfclient_enable_proc();
			BT_AUD_CHK_FAIL(_bt_hfp_event_hfclient_enable_proc, i4_ret);
#endif /*CONFIG_APP_SUPPORT_A2DP_SINK_SRC_BOTH*/
        }
        break;

        case BT_HFCLIENT_DISABLE:    //HFP
        {
            DBG_ERROR((BTAUD_TAG BTHFP_TAG"BT_HFCLIENT_DISABLE \r\n"));
#if !CONFIG_APP_SUPPORT_A2DP_SINK_SRC_BOTH
			i4_ret = _bt_hfp_event_hfclient_disable_proc();
			BT_AUD_CHK_FAIL(_bt_hfp_event_hfclient_enable_proc, i4_ret);
#endif /*CONFIG_APP_SUPPORT_A2DP_SINK_SRC_BOTH*/
        }
        break;			

        default:
        	break;
#endif /*CONFIG_SUPPORT_BT_HFP*/
    }

    BT_HFP_FUNC_EXIT();
    return BT_HFP_OK;
}

INT32 _bt_hfp_hfclient_state_disconnected_proc(BT_HFP_MSG_T *pv_msg)
{
    INT32 i4_ret;
    BT_HFP_MSG_T* pt_hfp_msg = (BT_HFP_MSG_T*)pv_msg;
	
    BT_HFP_FUNC_ENTRY();

	g_t_bt_hfp_cbk_sta.conn_state = 0; 
	g_pb_cur_index = 0;
	strcpy(g_cur_hfp_addr,pt_hfp_msg->ui4_str);			 
	g_t_bt_hfp_sta.b_bt_hfp_play = FALSE;

	//hfclient disconnected while reject a incomming call without callsetup_none event
	i4_ret = _bt_hfp_stop_ring_alert_proc();
	BT_HFP_CHK_FAIL(_bt_hfp_stop_ring_alert_proc, i4_ret);

    BT_HFP_FUNC_EXIT();
    return BT_HFP_OK;
}

INT32 _bt_hfp_hfclient_state_connected_proc(BT_HFP_MSG_T *pv_msg)
{
    INT32 i4_ret;
    BT_HFP_MSG_T* pt_hfp_msg = (BT_HFP_MSG_T*)pv_msg;
	
    BT_HFP_FUNC_ENTRY();

    g_t_bt_hfp_sta.b_bt_hfp_connect = TRUE;
    g_t_bt_hfp_cbk_sta.conn_state = 1 ;
	strcpy(g_cur_hfp_addr,pt_hfp_msg->ui4_str);	

#if CONFIG_APP_SUPPORT_A2DP_SINK_SRC_BOTH
	if (g_t_bt_aud_sta.b_bt_a2dp_connect)
	{
		DBG_ERROR((BTAUD_TAG BTHFP_TAG"A2DP role is disconnected.\r\n"));
		
		if (BT_A2DP_ROLE_SRC == g_cur_a2dp_role)
		{
			DBG_ERROR((BTAUD_TAG BTHFP_TAG"Current A2DP role is SRC, start disconnect HFP device : %s .\r\n",
								  g_cur_hfp_addr));
			
			i4_ret = a_mtkapi_bt_hfclient_disconnect(g_cur_hfp_addr);
			BT_AUD_CHK_FAIL_RET(a_mtkapi_bt_hfclient_disconnect, i4_ret, BT_HFP_FAIL);
		}
		else
		{
			DBG_ERROR((BTAUD_TAG BTHFP_TAG"Current A2DP role is SINK.\r\n"));
		}
	}
	else
	{
		DBG_ERROR((BTAUD_TAG BTHFP_TAG"A2DP role is disconnected.\r\n"));
	}
#endif /*CONFIG_APP_SUPPORT_A2DP_SINK_SRC_BOTH*/

    BT_HFP_FUNC_EXIT();
    return BT_HFP_OK;
}

INT32 _bt_hfp_event_hfclient_connection_proc(BT_HFP_MSG_T *pv_msg)
{
    INT32 i4_ret;
    BT_HFP_MSG_T* pt_hfp_msg = (BT_HFP_MSG_T*)pv_msg;
	
    BT_HFP_FUNC_ENTRY();

    switch (pt_hfp_msg->ui4_data1)
    {
#if CONFIG_SUPPORT_BT_HFP
        case BT_HFCLIENT_CONNECTION_STATE_DISCONNECTED:
        {
			DBG_ERROR((BTAUD_TAG BTHFP_TAG"BT_HFCLIENT_CONNECTION_STATE_DISCONNECTED, addr: %s\r\n",
											pt_hfp_msg->ui4_str));

			i4_ret = _bt_hfp_hfclient_state_disconnected_proc(pt_hfp_msg);
			BT_HFP_CHK_FAIL(_bt_hfp_hfclient_state_disconnected_proc, i4_ret);
        }
        break;

        case BT_HFCLIENT_CONNECTION_STATE_CONNECTED:    //HFP
        {
            DBG_ERROR((BTAUD_TAG BTHFP_TAG"BT_HFCLIENT_CONNECTION_STATE_CONNECTED, addr: %s.\r\n",
											pt_hfp_msg->ui4_str));

			i4_ret = _bt_hfp_hfclient_state_connected_proc(pt_hfp_msg);
			BT_HFP_CHK_FAIL(_bt_hfp_hfclient_state_connected_proc, i4_ret);
        }
        break;			

        default:
        	break;
#endif /*CONFIG_SUPPORT_BT_HFP*/
    }

    BT_HFP_FUNC_EXIT();
    return BT_HFP_OK;
}

INT32 _bt_hfp_play_pre_proc(VOID)
{
    INT32 i4_ret;
	
    BT_HFP_FUNC_ENTRY();
          
	if (TRUE == g_t_bt_aud_sta.b_bt_play)
	{
		DBG_ERROR((BTAUD_TAG BTHFP_TAG"A2DP is still playing."));
		if (TRUE == g_t_bt_aud_sta.b_bt_open_player)
		{
			DBG_ERROR((BTAUD_TAG BTHFP_TAG"A2DP player is opened, stop player firstly."));
#if 0  //fix a2dp play no sound after hfp play finish
			i4_ret = _bluetooh_send_avrcp_pause_proc();
			BT_AUD_CHK_FAIL_RET(_bluetooh_send_avrcp_pause_proc, i4_ret, BT_AUD_FAIL);
#endif		
			i4_ret = a_mtkapi_a2dp_sink_stop_player();
			BT_AUD_CHK_FAIL(a_mtkapi_a2dp_sink_stop_player, i4_ret);
			g_t_bt_aud_sta.b_bt_open_player = FALSE;

			i4_ret = _bluetooth_send_inform_stop_to_sm();
        	BT_AUD_CHK_FAIL_RET(_bluetooth_send_inform_play_to_sm, i4_ret, BT_AUD_FAIL);

			if (g_t_bt_aud_sta.b_bt_source)
			{
				g_t_bt_aud_sta.b_bt_source = FALSE; //not BT source
			}

			if(g_t_bt_aud_sta.b_bt_play_pause_proc)
			{
				g_t_bt_aud_sta.b_bt_play_pause_proc = FALSE; //avoid inform stop SM again while user stop A2DP

			}
		}
		else
		{
			DBG_ERROR((BTAUD_TAG BTHFP_TAG"A2DP player is stopped."));
		}
	}
	else
	{
		DBG_ERROR((BTAUD_TAG BTHFP_TAG"A2DP is already paused."));
	}
		
    BT_HFP_FUNC_EXIT();
    return BT_HFP_OK;
}

INT32 _bt_hfp_restart_a2dp_player_proc(VOID)
{
    INT32 i4_ret;
	
    BT_HFP_FUNC_ENTRY();

	DBG_ERROR((BTAUD_TAG BTHFP_TAG"b_bt_play: %d,"
								"b_bt_open_player: %d,"
								"b_bt_source: %d",
								"b_bt_play_pause_proc: %d"));
          
	if (TRUE == g_t_bt_aud_sta.b_bt_play)
	{
		DBG_ERROR((BTAUD_TAG BTHFP_TAG"A2DP is in play status ."));
		if (FALSE == g_t_bt_aud_sta.b_bt_open_player)
		{
			DBG_ERROR((BTAUD_TAG BTHFP_TAG"A2DP player is stopped, here send request PLAY for A2DP play resuming."));

			i4_ret = _bluetooth_send_request_play_to_sm();
        	BT_AUD_CHK_FAIL_RET(_bluetooth_send_request_play_to_sm, i4_ret, BT_AUD_FAIL);

			if(!g_t_bt_aud_sta.b_bt_play_pause_proc)
			{
				g_t_bt_aud_sta.b_bt_play_pause_proc = TRUE; //inform stop SM while user stop A2DP
			}
		}
		else
		{
			DBG_ERROR((BTAUD_TAG BTHFP_TAG"A2DP player is stopped."));
		}
	}
	else
	{
		DBG_ERROR((BTAUD_TAG BTHFP_TAG"A2DP is already paused."));
	}
		
    BT_HFP_FUNC_EXIT();
    return BT_HFP_OK;
}

INT32 _bt_hfp_audio_play_proc(VOID)
{
    INT32 i4_ret;
	
    BT_HFP_FUNC_ENTRY();

	DBG_ERROR((BTAUD_TAG BTHFP_TAG"b_bt_hfp_play: %d, b_bt_play: %d, b_bt_open_player: %d. \r\n",
				g_t_bt_hfp_sta.b_bt_hfp_play,g_t_bt_aud_sta.b_bt_play,
				g_t_bt_aud_sta.b_bt_open_player));
          
    if (!g_t_bt_hfp_sta.b_bt_hfp_play)
    {
		DBG_ERROR((BTAUD_TAG BTHFP_TAG"inband ring phone !\r\n"));

		i4_ret = _bt_hfp_play_pre_proc();
		BT_AUD_CHK_FAIL(_bt_hfp_play_pre_proc, i4_ret);
		
		//send request to SM
        i4_ret = _bt_hfp_send_request_play_to_sm();
        BT_AUD_CHK_FAIL(_bt_hfp_send_request_play_to_sm, i4_ret);
    }
	else
	{
		DBG_ERROR((BTAUD_TAG BTHFP_TAG"HFP is playing , no need to request again.\r\n"));			
		
        if (TRUE == g_t_bt_hfp_sta.b_bt_hfp_player_thread_create)
        {
        	g_t_bt_hfp_sta.b_bt_hfp_player_thread_create = FALSE;
        }
        DBG_ERROR((BTAUD_TAG BTHFP_TAG"g_t_bt_hfp_sta.b_bt_hfp_player_thread_create: %d \r\n",
									g_t_bt_hfp_sta.b_bt_hfp_player_thread_create));

        if (TRUE == g_t_bt_hfp_sta.b_bt_hfp_media_play)
        {
            i4_ret = _bt_hfp_player_stop();
            BT_HFP_CHK_FAIL(_bt_hfp_player_stop, i4_ret);

            i4_ret = _bt_hfp_mic_spk_thread_create();
            BT_HFP_CHK_FAIL(_bt_hfp_mic_spk_thread_create, i4_ret);
        }
	}

    BT_HFP_FUNC_EXIT();
    return BT_HFP_OK;
}

INT32 _bt_hfp_event_audio_connected_proc(BOOL set_cvsd_msbc_type)
{
    INT32 i4_ret;

    BT_HFP_FUNC_ENTRY();

	g_t_bt_hfp_sta.b_bt_hfp_audio_connect = TRUE; 
	DBG_ERROR((BTAUD_TAG BTHFP_TAG"g_t_bt_hfp_sta.b_bt_hfp_audio_connect: %d \r\n",
								g_t_bt_hfp_sta.b_bt_hfp_audio_connect));
	
	if (TRUE == set_cvsd_msbc_type)
	{
		g_t_bt_hfp_cbk_sta.audio_state = 1;		
	}
	else
	{
		g_t_bt_hfp_cbk_sta.audio_state = 2; 	
	}

	//inform audio of codec type
	_bt_hfp_set_cvsd_msbc_type(set_cvsd_msbc_type);

	i4_ret = _bt_hfp_audio_play_proc();
	BT_AUD_CHK_FAIL(_bt_hfp_audio_play_proc, i4_ret);

    BT_HFP_FUNC_EXIT();
    return BT_HFP_OK;
}

INT32 _bt_hfp_event_audio_disconnected_proc(VOID)
{
    INT32 i4_ret;

    BT_HFP_FUNC_ENTRY();
		
	g_t_bt_hfp_sta.b_bt_hfp_play = FALSE;

    i4_ret = _bt_hfp_mic_spk_thread_exit();
    BT_HFP_CHK_FAIL(_bt_hfp_mic_spk_thread_exit, i4_ret);

    g_t_bt_hfp_sta.b_bt_hfp_audio_connect = FALSE;
    DBG_ERROR((BTAUD_TAG BTHFP_TAG"g_t_bt_hfp_sta.b_bt_hfp_audio_connect: %d \r\n",
									g_t_bt_hfp_sta.b_bt_hfp_audio_connect));

	g_t_bt_hfp_cbk_sta.audio_state = 0;
#if 1
    //send inform to SM
    i4_ret = _bt_hfp_send_inform_stop_to_sm();
    BT_AUD_CHK_FAIL(_bt_hfp_send_inform_stop_to_sm, i4_ret);
#endif

	i4_ret = _bt_hfp_restart_a2dp_player_proc();
	BT_HFP_CHK_FAIL(_bt_hfp_restart_a2dp_player_proc, i4_ret);

    BT_HFP_FUNC_EXIT();
    return BT_HFP_OK;
}

INT32 _bt_hfp_event_audio_connection_proc(BT_HFP_MSG_T *pv_msg)
{
    INT32 i4_ret;
    BT_HFP_MSG_T* pt_hfp_msg = (BT_HFP_MSG_T*)pv_msg;
    BT_HFP_FUNC_ENTRY();

    switch (pt_hfp_msg->ui4_data1)
    {
#if CONFIG_SUPPORT_BT_HFP		
        case BT_HFCLIENT_AUDIO_STATE_CONNECTED:   
        {
            DBG_ERROR((BTAUD_TAG BTHFP_TAG"BT_HFCLIENT_AUDIO_STATE_CONNECTED\r\n"));
            DBG_ERROR((BTAUD_TAG BTHFP_TAG"audio connect ok !\r\n"));

			i4_ret = _bt_hfp_event_audio_connected_proc(TRUE);
			BT_AUD_CHK_FAIL(_bt_hfp_event_audio_connected_proc, i4_ret);
        }
		break;

        case BT_HFCLIENT_AUDIO_STATE_CONNECTED_MSBC:   
        {
            DBG_ERROR((BTAUD_TAG BTHFP_TAG"BT_HFCLIENT_AUDIO_STATE_CONNECTED_MSBC \r\n"));
            DBG_ERROR((BTAUD_TAG BTHFP_TAG"audio connect ok !\r\n"));
			
			i4_ret = _bt_hfp_event_audio_connected_proc(FALSE);
			BT_AUD_CHK_FAIL(_bt_hfp_event_audio_connected_proc, i4_ret);
        }
		break;
		
        case BT_HFCLIENT_AUDIO_STATE_DISCONNECTED:  //HFP
        {
            DBG_ERROR((BTAUD_TAG BTHFP_TAG"BT_HFCLIENT_AUDIO_STATE_DISCONNECTED \r\n"));
			DBG_ERROR((BTAUD_TAG BTHFP_TAG"audio disconnect !!\r\n"));
			
			i4_ret = _bt_hfp_event_audio_disconnected_proc();
			BT_AUD_CHK_FAIL(_bt_hfp_event_audio_disconnected_proc, i4_ret);			
        }
        break;

        default:
			break;
#endif /*CONFIG_SUPPORT_BT_HFP*/
    }

    BT_HFP_FUNC_EXIT();
    return BT_HFP_OK;
}

INT32 _bt_hfp_event_vr_state_proc(BT_HFP_MSG_T *pv_msg)
{
    INT32 i4_ret;
    BT_HFP_MSG_T* pt_hfp_msg = (BT_HFP_MSG_T*)pv_msg;
	
    BT_HFP_FUNC_ENTRY();

	switch(pt_hfp_msg->ui4_data1)
	{
		case BT_HFCLIENT_VR_STATE_STOPPED:
		{
			DBG_ERROR((BTHFP_TAG"BT_HFCLIENT_VR_STATE_STOPPED.\r\n"));
			g_t_bt_hfp_cbk_sta.vr_state = FALSE;
		}
		break;

		case BT_HFCLIENT_VR_STATE_STARTED:
		{
			DBG_ERROR((BTHFP_TAG"BT_HFCLIENT_VR_STATE_STARTED.\r\n"));
			g_t_bt_hfp_cbk_sta.vr_state = TRUE;
		}
		break;

		default:
			break;
	}

    BT_HFP_FUNC_EXIT();
    return BT_HFP_OK;
}

INT32 _bt_hfp_event_call_proc(BT_HFP_MSG_T *pv_msg)
{
    INT32 i4_ret;
    BT_HFP_MSG_T* pt_hfp_msg = (BT_HFP_MSG_T*)pv_msg;
	
    BT_HFP_FUNC_ENTRY();

	switch(pt_hfp_msg->ui4_data1)
	{
		case BT_HFCLIENT_CALL_CALLS_IN_PROGRESS:
		{
		   DBG_ERROR((BTHFP_TAG"BT_HFCLIENT_CALL_CALLS_IN_PROGRESS.\r\n"));
		   g_t_bt_hfp_sta.b_bt_hfp_calls_in_process = TRUE;
		   //then inform SM
		}
		break;

		case BT_HFCLIENT_CALL_NO_CALLS_IN_PROGRESS:
		{
		   DBG_ERROR((BTHFP_TAG"BT_HFCLIENT_CALL_NO_CALLS_IN_PROGRESS.\r\n"));
		   g_t_bt_hfp_sta.b_bt_hfp_calls_in_process = FALSE;
		   //then inform SM
		}
		break;

		default:
			break;
	}

    BT_HFP_FUNC_EXIT();
    return BT_HFP_OK;
}

INT32 _bt_hfp_stop_ring_alert_proc(VOID)
{
    INT32 i4_ret;
	
    BT_HFP_FUNC_ENTRY();

	if(TRUE == g_t_bt_hfp_sta.b_bt_hfp_player_thread_create)
	{
		g_t_bt_hfp_sta.b_bt_hfp_player_thread_create = FALSE;
	}
	
	if(TRUE == g_t_bt_hfp_sta.b_bt_hfp_media_play)
	{
		//stop alert
		i4_ret = _bt_hfp_player_stop();
		BT_HFP_CHK_FAIL(_bt_hfp_player_stop, i4_ret);
	
		g_t_bt_hfp_sta.b_bt_hfp_media_play = FALSE;
	
		//ring alert but audio stay disconnected
		if(FALSE == g_t_bt_hfp_sta.b_bt_hfp_audio_connect)
		{
			g_t_bt_hfp_sta.b_bt_hfp_play = FALSE;
			
			//send inform stop to SM
			i4_ret = _bt_hfp_send_inform_stop_to_sm();
			BT_AUD_CHK_FAIL(_bt_hfp_send_inform_stop_to_sm, i4_ret);

			i4_ret = _bt_hfp_restart_a2dp_player_proc();
			BT_HFP_CHK_FAIL(_bt_hfp_restart_a2dp_player_proc, i4_ret);
		}
	}
	DBG_ERROR((BTAUD_TAG BTHFP_TAG"g_t_bt_hfp_sta.b_bt_hfp_player_thread_create: %d ",
								 "g_t_bt_hfp_sta.b_bt_hfp_media_play : %d \r\n",
								 g_t_bt_hfp_sta.b_bt_hfp_player_thread_create,
								 g_t_bt_hfp_sta.b_bt_hfp_media_play));


    BT_HFP_FUNC_EXIT();
    return BT_HFP_OK;
}

INT32 _bt_hfp_callsetup_none_proc(VOID)
{
    INT32 i4_ret;
	
    BT_HFP_FUNC_ENTRY();

	g_t_bt_hfp_sta.b_bt_hfp_call_setup = HFP_CALLSETUP_NONE;
	
	i4_ret = _bt_hfp_stop_ring_alert_proc();
	BT_HFP_CHK_FAIL(_bt_hfp_stop_ring_alert_proc, i4_ret);

    BT_HFP_FUNC_EXIT();
    return BT_HFP_OK;
}

INT32 _bt_hfp_event_callsetup_proc(BT_HFP_MSG_T *pv_msg)
{
    INT32 i4_ret;
    BT_HFP_MSG_T* pt_hfp_msg = (BT_HFP_MSG_T*)pv_msg;
	
    BT_HFP_FUNC_ENTRY();

	switch(pt_hfp_msg->ui4_data1)
	{
		case BT_HFCLIENT_CALLSETUP_NONE:
		{
			DBG_ERROR((BTHFP_TAG"BT_HFCLIENT_CALLSETUP_NONE.\r\n"));
			i4_ret = _bt_hfp_callsetup_none_proc();
			BT_HFP_CHK_FAIL(_bt_hfp_callsetup_none_proc, i4_ret);
		}
		break;

		case BT_HFCLIENT_CALLSETUP_INCOMING:
		{
		   DBG_ERROR((BTHFP_TAG"BT_HFCLIENT_CALLSETUP_INCOMING.\r\n"));

		   g_t_bt_hfp_sta.b_bt_hfp_call_setup = HFP_CALLSETUP_INCOMING;
		}
		break;

		case BT_HFCLIENT_CALLSETUP_OUTGOING:
		{
		   DBG_ERROR((BTHFP_TAG"BT_HFCLIENT_CALLSETUP_OUTGOING.\r\n"));

		   g_t_bt_hfp_sta.b_bt_hfp_call_setup = HFP_CALLSETUP_OUTGOING;
		}
		break;

		case BT_HFCLIENT_CALLSETUP_ALERTING:
		{
		   DBG_ERROR((BTHFP_TAG"BT_HFCLIENT_CALLSETUP_ALERTING.\r\n"));

		   g_t_bt_hfp_sta.b_bt_hfp_call_setup = HFP_CALLSETUP_ALERTING;
		}
		break;

		default:
			DBG_ERROR((BTHFP_TAG"Err:can't recognize the BT_HFP_MSG_CALLSETUP msgid %d.\r\n", pt_hfp_msg->ui4_msg_id));
			break;
	}

    BT_HFP_FUNC_EXIT();
    return BT_HFP_OK;
}

INT32 _bt_hfp_event_volume_change_proc(BT_HFP_MSG_T *pv_msg)
{
    INT32 i4_ret;
    BT_HFP_MSG_T* pt_hfp_msg = (BT_HFP_MSG_T*)pv_msg;
	int ui1_setval = 0, ui1_cur_vol = 0;
	int ui4_volume = pt_hfp_msg->ui4_data2;
	
    BT_HFP_FUNC_ENTRY();

	switch(pt_hfp_msg->ui4_data1)
	{
		case BT_HFCLIENT_VOLUME_TYPE_SPK:	

	//	case BT_HFCLIENT_VOLUME_TYPE_MIC:	
		{
			//change remote volume to local type before set it to the platform
			ui1_setval = ((ui4_volume * BT_AUD_LOCAL_MAX_VOLUME)
						+ (BT_HFP_REMOTE_DEV_MAX_VOLUME - 1))
						/ BT_HFP_REMOTE_DEV_MAX_VOLUME;

			i4_ret = u_acfg_get_volume(&ui1_cur_vol);
			if (APP_CFGR_OK != i4_ret)
			{
				 DBG_ERROR((BTHFP_TAG"Err: get local volume fail.\r\n"));
				 break;
			}
#if CONFIG_APP_SUPPORT_XPL
			//In case volume turns up to max volume abruptly, only allow volume turns up 25% once
			if ((ui1_setval > ui1_cur_vol)
				 && ((ui1_setval - ui1_cur_vol) > (BT_AUD_LOCAL_MAX_VOLUME >> 2)))
			{
				DBG_API((BTHFP_TAG"volume turns up abruptly, limit the variation.\r\n"));
				ui1_setval = ui1_cur_vol + (BT_AUD_LOCAL_MAX_VOLUME >> 2);
			}
			else if (ui1_setval == ui1_cur_vol)//remote volume no change or change slightly, then ignore this action
			{
				DBG_ERROR((BTHFP_TAG"remote volume is the same with local, ignore this action.\r\n"));
				break;
			}
#endif/* CONFIG_APP_SUPPORT_XPL */
			DBG_API((BTHFP_TAG"remote volume sync to local, localvol=%d, set_vol=%d, send_vol=%d.\r\n",
					 ui1_cur_vol, ui1_setval, ui4_volume));

			i4_ret = u_bt_hfp_set_absolute_volume(ui1_setval);
			BT_AUD_CHK_FAIL_RET(u_bt_hfp_set_absolute_volume, i4_ret, BT_HFP_FAIL);
		}
		break;

		default:
			DBG_ERROR((BTHFP_TAG"Err:can't recognize the BT_HFP_MSG_VOLUME_CHANGE msgid %d.\r\n", pt_hfp_msg->ui4_msg_id));
			break;
	}

    BT_HFP_FUNC_EXIT();
    return BT_HFP_OK;
}

INT32 _bt_hfp_event_ring_indicate_proc(BT_HFP_MSG_T *pv_msg)
{
    INT32 i4_ret;
    BT_HFP_MSG_T* pt_hfp_msg = (BT_HFP_MSG_T*)pv_msg;
	
    BT_HFP_FUNC_ENTRY();

	DBG_ERROR((BTAUD_TAG BTHFP_TAG"b_bt_hfp_audio_connect: %d ,"
		                          "b_bt_hfp_player_thread_create: %d,"
		                          "b_bt_play: %d,"
		                          "b_bt_open_player: %d.\r\n",
		                          g_t_bt_hfp_sta.b_bt_hfp_audio_connect,
		                          g_t_bt_hfp_sta.b_bt_hfp_player_thread_create,
		                          g_t_bt_aud_sta.b_bt_play,
		                          g_t_bt_aud_sta.b_bt_open_player));

	i4_ret = _bt_hfp_play_pre_proc();
	BT_AUD_CHK_FAIL(_bt_hfp_play_pre_proc, i4_ret);
	
	if ((FALSE == g_t_bt_hfp_sta.b_bt_hfp_audio_connect)
		&& (FALSE == g_t_bt_hfp_sta.b_bt_hfp_player_thread_create))   //non-iband-ring
	{
        g_t_bt_hfp_sta.b_bt_hfp_non_inband = TRUE;
		
		i4_ret = _bt_hfp_send_request_play_to_sm();
        BT_HFP_CHK_FAIL(_bt_hfp_send_request_play_to_sm, i4_ret);
	}
	else
	{
        g_t_bt_hfp_sta.b_bt_hfp_non_inband = FALSE;
	}

    BT_HFP_FUNC_EXIT();
    return BT_HFP_OK;
}

INT32 _bt_hfp_event_msg_proc(const VOID *pv_msg)
{
    INT32 i4_ret;
    BT_HFP_MSG_T* pt_hfp_msg = (BT_HFP_MSG_T*)pv_msg;
	
    BT_HFP_FUNC_ENTRY();

#if 0
    if((BT_HFCLIENT_CALLSETUP_NONE == g_t_bt_hfp_cbk_sta.callsetup)&&(BT_HFCLIENT_CALL_NO_CALLS_IN_PROGRESS== g_t_bt_hfp_cbk_sta.call))
    {
        g_t_bt_hfp_sta.b_bt_hfp_call_ongoing = FALSE;
    }
    else
    {
        g_t_bt_hfp_sta.b_bt_hfp_call_ongoing = TRUE;
    }

    DBG_ERROR((BTHFP_TAG"g_t_bt_hfp_sta.b_bt_hfp_call_ongoing : %d .\r\n",
							g_t_bt_hfp_sta.b_bt_hfp_call_ongoing));
#endif

    switch (pt_hfp_msg->ui4_msg_id)
    {
        DBG_ERROR((BTHFP_TAG"BLUETOOTH_HFP pt_hfp_msg->ui4_msg_id is : %d \r\n",pt_hfp_msg->ui4_msg_id));

		case BTAPP_HFCLIENT_ABILITY_CB_EVT:		 /*HFP interface enable/disable event*/
        {
            DBG_ERROR((BTHFP_TAG"BTAPP_HFCLIENT_ABILITY_CB_EVT.\r\n"));
			
            i4_ret = _bt_hfp_event_hfclient_ability_proc(pt_hfp_msg);
            BT_HFP_CHK_FAIL(_bt_hfp_event_hfclient_ability_proc, i4_ret);
        }
        break;

        case BTAPP_HFCLIENT_CONNECTION_CB_EVT:		 /*HFP Connection status*/
        {
            DBG_ERROR((BTHFP_TAG"BT_HFP_MSG_GAP_EVENT.\r\n"));
			
            i4_ret = _bt_hfp_event_hfclient_connection_proc(pt_hfp_msg);
            BT_HFP_CHK_FAIL(_bt_hfp_event_hfclient_connection_proc, i4_ret);
        }
        break;

		case BTAPP_HFCLIENT_AUDIO_CONNECTION_CB_EVT:		 /*HFP Connection status*/
        {
            DBG_ERROR((BTHFP_TAG"BTAPP_HFCLIENT_AUDIO_CONNECTION_CB_EVT.\r\n"));
			
            i4_ret = _bt_hfp_event_audio_connection_proc(pt_hfp_msg);
            BT_HFP_CHK_FAIL(_bt_hfp_event_hfclient_connection_proc, i4_ret);
        }
        break;

		case BTAPP_HFCLIENT_BVRA_CB_EVT:			 /*AG changed voice recognition setting*/
        {
            DBG_ERROR((BTHFP_TAG"BT_HFP_MSG_GAP_EVENT.\r\n"));
			
            i4_ret = _bt_hfp_event_vr_state_proc(pt_hfp_msg);
            BT_HFP_CHK_FAIL(_bt_hfp_event_hfclient_connection_proc, i4_ret);
        }
        break;

        case BTAPP_HFCLIENT_IND_SERVICE_CB_EVT:		 /*network status*/
		{
            DBG_ERROR((BTHFP_TAG"BT_HFP_MSG_NETWORK_STATE.\r\n"));
		}
		break;

        case BTAPP_HFCLIENT_IND_ROAM_CB_EVT: 		 /*network roaming status*/
		{
            DBG_ERROR((BTHFP_TAG"BT_HFP_MSG_NETWORK_ROAMING.\r\n"));
		}
		break;

        case BTAPP_HFCLIENT_IND_SIGNAL_CB_EVT:		 /*network signal strength*/
	    {
            DBG_ERROR((BTHFP_TAG"BT_HFP_MSG_NETWORK_SIGNAL.\r\n"));
		}
		break;

        case BTAPP_HFCLIENT_IND_BATTCH_CB_EVT:		 /*battery level*/
        {
            DBG_ERROR((BTHFP_TAG"BT_HFP_MSG_BATTERY_LEVEL.\r\n"));
		}
		break;

        case BTAPP_HFCLIENT_COPS_CB_EVT: 			 /*current operator name*/
		{
            DBG_ERROR((BTHFP_TAG"BT_HFP_MSG_CURRENT_OPERATOR.\r\n"));
		}
		break;

        case BTAPP_HFCLIENT_IND_CALL_CB_EVT: 		 /*call*/
		{
            DBG_ERROR((BTHFP_TAG"BTAPP_HFCLIENT_IND_CALL_CB_EVT.\r\n"));
			
			i4_ret = _bt_hfp_event_call_proc(pt_hfp_msg);
            BT_HFP_CHK_FAIL(_bt_hfp_event_call_proc, i4_ret);
		}
		break;

	    case BTAPP_HFCLIENT_IND_CALLSETUP_CB_EVT:	 /*callsetup*/
	    {
            DBG_ERROR((BTHFP_TAG"BTAPP_HFCLIENT_IND_CALLSETUP_CB_EVT.\r\n"));
			
			i4_ret = _bt_hfp_event_callsetup_proc(pt_hfp_msg);
            BT_HFP_CHK_FAIL(_bt_hfp_event_callsetup_proc, i4_ret);			
		}
		break;

	    case BTAPP_HFCLIENT_IND_CALLHELD_CB_EVT: 	 /*callheld*/
		{
            DBG_ERROR(("BTAPP_HFCLIENT_IND_CALLHELD_CB_EVT.\r\n"));
		}
		break;

	    case BTAPP_HFCLIENT_BTRH_CB_EVT: 			 /*bluetooth response and hold event*/
	    {
            DBG_ERROR((BTHFP_TAG"BTAPP_HFCLIENT_BTRH_CB_EVT.\r\n"));
		}
		break;

	    case BTAPP_HFCLIENT_CLIP_CB_EVT: 			 /*Calling line identification event*/
		{
            DBG_ERROR((BTHFP_TAG"BTAPP_HFCLIENT_CLIP_CB_EVT.\r\n"));
		}
		break;

	    case BTAPP_HFCLIENT_CCWA_CB_EVT: 			 /*Call waiting notification*/
		{
            DBG_ERROR((BTHFP_TAG"BTAPP_HFCLIENT_CCWA_CB_EVT.\r\n"));
		}
		break;

	    case BTAPP_HFCLIENT_CLCC_CB_EVT: 			 /*current call event*/
		{
            DBG_ERROR((BTHFP_TAG"BTAPP_HFCLIENT_CLCC_CB_EVT.\r\n"));
		}
		break;

	    case BTAPP_HFCLIENT_VGM_VGS_CB_EVT:			 /*volume change*/
		{
            DBG_ERROR((BTHFP_TAG"BTAPP_HFCLIENT_VGM_VGS_CB_EVT(VOLUME CHANGE).\r\n"));
			
			i4_ret = _bt_hfp_event_volume_change_proc(pt_hfp_msg);
            BT_HFP_CHK_FAIL(_bt_hfp_event_volume_change_proc, i4_ret);				
		}
		break;

	    case BTAPP_HFCLIENT_CMD_COMPLETE_CB_EVT: 	 /*command complete*/
        {
            DBG_ERROR((BTHFP_TAG"BTAPP_HFCLIENT_CMD_COMPLETE_CB_EVT.\r\n"));
		}
		break;

	    case BTAPP_HFCLIENT_CNUM_CB_EVT: 			 /*subscriber information event*/
		{
            DBG_ERROR((BTHFP_TAG"BTAPP_HFCLIENT_CNUM_CB_EVT.\r\n"));
		}
		break;

	    case BTAPP_HFCLIENT_BSIR_CB_EVT: 			 /*in-band ring tone setting changed event*/
		{
            DBG_ERROR((BTHFP_TAG"BTAPP_HFCLIENT_BSIR_CB_EVT(in-band ring tone setting).\r\n"));
		}
		break;

	    case BTAPP_HFCLIENT_BINP_CB_EVT: 			 /*last voice tag number*/
		{
            DBG_ERROR((BTHFP_TAG"BTAPP_HFCLIENT_BINP_CB_EVT(last voice tag number: %s).\r\n",
								pt_hfp_msg->ui4_str));
		}
		break;

	    case BTAPP_HFCLIENT_RING_IND_CB_EVT: 		 /*HF Client ring indication */
		{
            DBG_ERROR((BTHFP_TAG"BTAPP_HFCLIENT_RING_IND_CB_EVT.\r\n"));
			
			i4_ret = _bt_hfp_event_ring_indicate_proc(pt_hfp_msg);
            BT_HFP_CHK_FAIL(_bt_hfp_event_ring_indicate_proc, i4_ret);				           
		}
		break;

		default:
			break;
    }
#if 0
	i4_ret = _bt_hfp_cbk_status_print();   //print cbd status
	BT_HFP_CHK_FAIL(_bt_hfp_cbk_status_print, i4_ret);
#endif
    BT_HFP_FUNC_EXIT();
    return BT_HFP_OK;
}

INT32 u_bt_hfp_volume_sync_to_remote(UINT8 ui4_volume)
{
    INT32 i4_ret;
	UINT8 ui1_setval = 0, ui1_cur_vol = 0;
	
    BT_HFP_FUNC_ENTRY();

	//change local volume to remote type before send it out
    ui1_setval = (ui4_volume * BT_AUD_REMOTE_DEV_MAX_VOLUME) / BT_AUD_LOCAL_MAX_VOLUME;
    DBG_API((BTAUD_TAG"local volume sync to remote, localvol=%d, send_vol=%d.\r\n",
            ui4_volume, ui1_setval));
    i4_ret = a_mtkapi_bt_hfclient_volume_control(BT_HFCLIENT_VOLUME_TYPE_SPK,ui1_setval);
    DBG_ERROR((BTHFP_TAG"a_mtkapi_bt_hfclient_volume_control type=%d , volume=%d\r\n"));
    BT_HFP_CHK_FAIL_RET(a_mtkapi_bt_hfclient_volume_control, i4_ret, BT_HFP_FAIL);

    BT_HFP_FUNC_EXIT();
    return APPUTILR_OK;
}

INT32 _bt_hfp_acfg_increase_volume(VOID)
{
    UINT8 ui1_volume;
    INT32 i4_ret;

	BT_HFP_FUNC_ENTRY();

    DBG_INFO(("u_acfg_increase_volume\n"));

    i4_ret = u_acfg_set_mute(FALSE);
    if (APP_CFGR_OK != i4_ret)
    {
        return i4_ret;
    }

    i4_ret = u_acfg_get_volume(&ui1_volume);
    if (APP_CFGR_OK != i4_ret)
    {
        return i4_ret;
    }

    ui1_volume += BT_VOLUME_STEP;
    if (ui1_volume > VOLUME_MAX)
    {
        ui1_volume = VOLUME_MAX;
    }

    i4_ret = u_acfg_set_volume(ui1_volume,TRUE);
    if (APP_CFGR_OK != i4_ret)
    {
        return i4_ret;
    }

	u_bt_hfp_volume_sync_to_remote(ui1_volume);

	BT_HFP_FUNC_EXIT();
    return APP_CFGR_OK;
}

INT32 _bt_hfp_acfg_reduce_volume(VOID)
{
    UINT8 ui1_volume;
    INT32 i4_ret;

	BT_HFP_FUNC_ENTRY();

    DBG_INFO(("u_acfg_reduce_volume\n"));

    i4_ret = u_acfg_set_mute(FALSE);
    if (APP_CFGR_OK != i4_ret)
    {
        return i4_ret;
    }

    i4_ret = u_acfg_get_volume(&ui1_volume);
    if (APP_CFGR_OK != i4_ret)
    {
        return i4_ret;
    }

    if (ui1_volume < BT_VOLUME_STEP)
    {
        ui1_volume = 0;
    }
    else
    {
        ui1_volume -= BT_VOLUME_STEP;
    }

    i4_ret = u_acfg_set_volume(ui1_volume,TRUE);
    if (APP_CFGR_OK != i4_ret)
    {
        return i4_ret;
    }

	u_bt_hfp_volume_sync_to_remote(ui1_volume);

	BT_HFP_FUNC_EXIT();
	return APP_CFGR_OK;
}

static VOID _bt_hfp_process_ui_mute_key(VOID)
{
    BOOL b_vol_mute;
    UINT8 ui1_value;
    INT32 i4_ret;

	BT_HFP_FUNC_ENTRY();

    DBG_INFO((BTHFP_TAG"receive volume mute key!\n"));
    i4_ret = u_acfg_get_mute(&b_vol_mute);
    if (APP_CFGR_OK != i4_ret)
    {
        DBG_ERROR((BTAUD_TAG"u_acfg_get_mute fail ret:%d\n", i4_ret));
        return;
    }

    b_vol_mute = !b_vol_mute;

    i4_ret = u_acfg_set_mute(b_vol_mute);
    if (APP_CFGR_OK != i4_ret)
    {
        DBG_ERROR((BTHFP_TAG"u_acfg_set_mute fail ret:%d\n", i4_ret));
        return;
    }

    if (!b_vol_mute)
    {
        i4_ret = u_acfg_get_volume(&ui1_value);
        if (APP_CFGR_OK != i4_ret)
        {
            DBG_ERROR((BTHFP_TAG"u_acfg_get_volume fail ret:%d\n", i4_ret));
            return;
        }

        i4_ret = u_acfg_set_volume(ui1_value,TRUE);
        if (APP_CFGR_OK != i4_ret)
        {
            DBG_ERROR((BTHFP_TAG"u_acfg_set_volume fail ret:%d\n", i4_ret));
            return;
        }
    }

	u_bt_hfp_volume_sync_to_remote(ui1_value);

	BT_HFP_FUNC_EXIT();
}

static VOID _bt_hfp_process_ui_volume_up_key(VOID)
{
    INT32 i4_ret;

	BT_HFP_FUNC_ENTRY();

    DBG_INFO((BTHFP_TAG"receive volume up key!\n"));
    i4_ret = _bt_hfp_acfg_increase_volume();
    if (APP_CFGR_OK != i4_ret)
    {
        DBG_ERROR((BTHFP_TAG"u_acfg_increase_volume fail ret:%d\n", i4_ret));
    }

	BT_HFP_FUNC_EXIT();
}

static VOID _bt_hfp_process_ui_volume_down_key(VOID)
{
    INT32 i4_ret;

	BT_HFP_FUNC_ENTRY();

    DBG_INFO((BTHFP_TAG"receive volume down key!\n"));
    i4_ret = _bt_hfp_acfg_reduce_volume();
    if (APP_CFGR_OK != i4_ret)
    {
        DBG_ERROR((BTHFP_TAG"u_acfg_increase_volume fail ret:%d\n", i4_ret));
    }

	BT_HFP_FUNC_EXIT();
}

INT32 _bt_hfp_answer_call(VOID)
{
    INT32 i4_ret;

    BT_HFP_FUNC_ENTRY();

    DBG_ERROR((BTHFP_TAG"Handle user_interface msg : answer an incomming call \n"));
	i4_ret = a_mtkapi_bt_hfclient_handle_call_action(BT_HFCLIENT_CALL_ACTION_ATA,0);
	BT_AUD_CHK_FAIL_RET(a_mtkapi_bt_hfclient_handle_call_action, i4_ret, BT_AUD_FAIL);

    BT_AUD_FUNC_EXIT();
    return BT_HFP_OK;
}

INT32 _bt_hfp_hange_up_call(VOID)
{
    INT32 i4_ret;

    BT_HFP_FUNC_ENTRY();

    DBG_ERROR((BTHFP_TAG"Handle user_interface msg : terminate an ongoing call \n"));
    i4_ret = a_mtkapi_bt_hfclient_handle_call_action(BT_HFCLIENT_CALL_ACTION_CHUP,0);
    BT_AUD_CHK_FAIL_RET(a_mtkapi_bt_hfclient_handle_call_action, i4_ret, BT_AUD_FAIL);


    BT_HFP_FUNC_EXIT();
    return BT_HFP_OK;
}

INT32 _bt_hfp_reject_call(VOID)
{
    INT32 i4_ret;

    BT_HFP_FUNC_ENTRY();

    DBG_ERROR((BTHFP_TAG"Handle user_interface msg : reject an incoming call \n"));
    i4_ret = a_mtkapi_bt_hfclient_handle_call_action(BT_HFCLIENT_CALL_ACTION_CHUP,0);
    BT_AUD_CHK_FAIL_RET(a_mtkapi_bt_hfclient_handle_call_action, i4_ret, BT_AUD_FAIL);

    BT_HFP_FUNC_EXIT();
    return BT_HFP_OK;
}

VOID _bt_hfp_ui_msg_process(const VOID *pv_msg, SIZE_T z_msg_len)
{
    APPMSG_T *t_msg = (APPMSG_T *)pv_msg;

	BT_HFP_FUNC_ENTRY();

    if (z_msg_len != sizeof(*t_msg))
    {
        DBG_ERROR((BTHFP_TAG"ui's msg size[%d] is wrong, should be [%d]!\n", z_msg_len, sizeof(APPMSG_T)));
        return;
    }

    DBG_INFO((BTHFP_TAG"receive ui key value:%d!\n", t_msg->ui4_msg_type));

    switch(t_msg->ui4_msg_type)
    {
        case KEY_MUTE:
            _bt_hfp_process_ui_mute_key();
            break;

        case KEY_VOLUMEUP:
            _bt_hfp_process_ui_volume_up_key();
            break;

        case KEY_VOLUMEDOWN:
            _bt_hfp_process_ui_volume_down_key();
            break;
#if 0
        case E_USERCMD_BT_HFP_ANSWER_CALL:
            _bt_hfp_answer_call();
            break;

        case E_USERCMD_BT_HFP_HANGUP_CALL:
            _bt_hfp_hange_up_call();
            break;
#endif
        default:
            DBG_INFO((BTHFP_TAG"Err: unrecognize receive ui key value:%d!\n", t_msg->ui4_msg_type));
            break;
    }

	BT_HFP_FUNC_EXIT();
}

//send play request to sm
INT32 _bt_hfp_send_request_play_to_sm(void)
{
    DBG_ERROR(("<BT_HFP> %s\n",__FUNCTION__));

    INT32 i4_ret = BT_AUD_FAIL;
    HANDLE_T h_app = NULL_HANDLE;

    u_am_get_app_handle_from_name(&h_app,SM_THREAD_NAME);

    i4_ret = u_app_send_appmsg(h_app,
                                E_APP_MSG_TYPE_STATE_MNGR,
                                MSG_FROM_BT_HFP,
                                SM_MAKE_MSG(SM_REQUEST_GRP,SM_BODY_PLAY,0),
                                NULL,
                                0);
    if (AEER_OK != i4_ret) {
        DBG_ERROR((BTAUD_TAG"_bluetooth_hfp_send_play_request_to_sm failed, i4_ret[%ld]\n",i4_ret));
    }

	BT_HFP_FUNC_EXIT();
    return (i4_ret == AEER_OK ? BT_AUD_OK : BT_AUD_FAIL);
}


//send inform play to sm
INT32 _bt_hfp_send_inform_play_to_sm(void)
{
    DBG_ERROR(("<BT_HFP> %s\n",__FUNCTION__));

	INT32 i4_ret = BT_HFP_FAIL;
    HANDLE_T h_app = NULL_HANDLE;
    SM_PARAM_T t_sm_param = {0};

    DBG_ERROR((BTHFP_TAG"g_bt_hfp_sm_parameter_id : %d \n",g_bt_hfp_sm_parameter_id));

    u_am_get_app_handle_from_name(&h_app,SM_THREAD_NAME);

    i4_ret = u_app_send_appmsg(h_app,
                                E_APP_MSG_TYPE_STATE_MNGR,
                                MSG_FROM_BT_HFP,
                                SM_MAKE_MSG(SM_INFORM_GRP,SM_BODY_PLAY,g_bt_hfp_sm_parameter_id),
                                &t_sm_param,
                                sizeof(t_sm_param));
    if (AEER_OK != i4_ret) {
        DBG_ERROR((BTHFP_TAG" _bt_hfp_send_inform_play_to_sm failed, i4_ret[%ld]\n",i4_ret));
    }

	BT_HFP_FUNC_EXIT();
    return (i4_ret == AEER_OK ? BT_HFP_OK : BT_HFP_FAIL);
}

//send inform stop to sm
INT32 _bt_hfp_send_inform_stop_to_sm(void)
{
    DBG_ERROR(("<BT_HFP> %s\n",__FUNCTION__));

    INT32 i4_ret = BT_HFP_FAIL;
    HANDLE_T h_app = NULL_HANDLE;
    SM_PARAM_T t_sm_param = {0};

    DBG_ERROR((BTHFP_TAG"g_bt_hfp_sm_parameter_id : %d \n",g_bt_hfp_sm_parameter_id));

    u_am_get_app_handle_from_name(&h_app,SM_THREAD_NAME);

    i4_ret = u_app_send_appmsg(h_app,
                                E_APP_MSG_TYPE_STATE_MNGR,
                                MSG_FROM_BT_HFP,
                                SM_MAKE_MSG(SM_INFORM_GRP,SM_BODY_STOP,g_bt_hfp_sm_parameter_id),
                                &t_sm_param,
                                sizeof(t_sm_param));
    if (AEER_OK != i4_ret) {
        DBG_ERROR((BTHFP_TAG" _bt_hfp_send_inform_play_to_sm failed, i4_ret[%ld]\n",i4_ret));
    }

	BT_HFP_FUNC_EXIT();
    return (i4_ret == AEER_OK ? BT_HFP_OK : BT_HFP_FAIL);
}

//bluetooth_sm_msg_process
INT32 _bt_hfp_sm_permit_play_proc(APPMSG_T* pv_msg)
{
    INT32 i4_ret;
    BT_AUD_FUNC_ENTRY();

    DBG_ERROR((BTAUD_TAG BTHFP_TAG"g_t_bt_hfp_sta.b_bt_hfp_audio_connect: %d \r\n",
									g_t_bt_hfp_sta.b_bt_hfp_audio_connect));

    g_bt_hfp_sm_parameter_id = SM_MSG_ID(pv_msg->ui4_msg_type);  //SM PARAMETER ID
    DBG_ERROR((BTAUD_TAG"g_bt_hfp_sm_parameter_id : %d \n",g_bt_hfp_sm_parameter_id));

	g_t_bt_hfp_sta.b_bt_hfp_play = TRUE;
	
    if ((FALSE == g_t_bt_hfp_sta.b_bt_hfp_audio_connect)
		&&(TRUE == g_t_bt_hfp_sta.b_bt_hfp_non_inband))  //non-in-band ring
    {
		g_t_bt_hfp_sta.b_bt_hfp_player_thread_create= TRUE;

		i4_ret = _bt_hfp_media_player_thread_creat();//create thread to play ring tone
		BT_HFP_CHK_FAIL_RET(_bt_hfp_media_player_thread_creat, i4_ret, BT_HFP_FAIL);

        DBG_ERROR((BTHFP_TAG"HFP src is playing, send inform to SM \n"));

		//inform SM
        //i4_ret = _bt_hfp_send_inform_play_to_sm();
        //BT_HFP_CHK_FAIL(_bt_hfp_send_inform_play_to_sm, i4_ret);;
	}
	else if (TRUE == g_t_bt_hfp_sta.b_bt_hfp_audio_connect)
	{
		//read data from mic and spk
	    i4_ret = _bt_hfp_mic_spk_thread_create();
        BT_HFP_CHK_FAIL(_bt_hfp_mic_spk_thread_create, i4_ret);	
	}

	//inform SM
	DBG_ERROR((BTHFP_TAG"HFP src is playing, send inform to SM \n"));						
    i4_ret = _bt_hfp_send_inform_play_to_sm();
    BT_HFP_CHK_FAIL(_bt_hfp_send_inform_play_to_sm, i4_ret);
              
    BT_AUD_FUNC_EXIT();
    return BT_AUD_OK;

}

//bluetooth_sm_msg_process
INT32 _bt_hfp_sm_request_stop_proc(APPMSG_T* pv_msg)
{
    INT32 i4_ret;
    BT_AUD_FUNC_ENTRY();

    g_t_bt_hfp_sta.b_bt_hfp_play = FALSE;

	//audio disconnect
	i4_ret = a_mtkapi_bt_hfclient_disconnect_audio(g_cur_hfp_addr);
    DBG_ERROR((BTHFP_TAG"a_mtkapi_bt_hfclient_disconnect_audio \r\n",g_cur_hfp_addr));
    BT_HFP_CHK_FAIL_RET(a_mtkapi_bt_hfclient_disconnect_audio, i4_ret, BT_HFP_FAIL);

	//stop reading data from mic and spk
	i4_ret = _bt_hfp_mic_spk_thread_exit();
    BT_AUD_CHK_FAIL(_bt_hfp_mic_spk_thread_exit, i4_ret);
#if 1
	DBG_ERROR((BTAUD_TAG"BT src is stop, send inform to SM \n"));
    i4_ret = _bt_hfp_send_inform_stop_to_sm();
    BT_AUD_CHK_FAIL(_bt_hfp_send_inform_stop_to_sm, i4_ret);
#endif

    BT_AUD_FUNC_EXIT();
    return BT_AUD_OK;
}

//bluetooth_sm_msg_process
INT32 _bt_hfp_sm_msg_process(APPMSG_T* pv_msg)
{
    INT32 i4_ret;
    BT_AUD_FUNC_ENTRY();

    DBG_ERROR((BTHFP_TAG"SM msg come!!! \r\n"));
    switch(SM_MSG_GRP(pv_msg->ui4_msg_type))
    {
        DBG_ERROR((BTHFP_TAG"SM_MSG_GRP is %d \r\n",SM_MSG_GRP(pv_msg->ui4_msg_type)));

		case SM_PERMIT_GRP:
        {
            switch(SM_MSG_BDY(pv_msg->ui4_msg_type))
            {
                DBG_ERROR((BTHFP_TAG"SM_MSG_BDY is %d \r\n",SM_MSG_BDY(pv_msg->ui4_msg_type)));

				case SM_BODY_PLAY:
                {
                    DBG_ERROR((BTHFP_TAG"SM permit HFP source play!!! \n"));

			        i4_ret = _bt_hfp_sm_permit_play_proc(pv_msg);
			        BT_HFP_CHK_FAIL(_bt_hfp_sm_permit_play_proc, i4_ret);
                }
                break;

                case SM_BODY_PAUSE:
                {
                    DBG_ERROR((BTAUD_TAG"Ignore--- Permite PAUSE by SM\n"));
                }
                break;

                case SM_BODY_STOP:
                {
                    DBG_ERROR((BTAUD_TAG"Ignore--- Permite PAUSE by SM\n"));
                }
                break;

                default:
                break;
            }
        }
        break;
		
        case SM_FORBID_GRP:
        {
            switch(SM_MSG_BDY(pv_msg->ui4_msg_type))
            {
                DBG_ERROR((BTHFP_TAG"SM_MSG_BDY is %d \r\n",SM_MSG_BDY(pv_msg->ui4_msg_type)));

				case SM_BODY_PLAY:
                {
                    DBG_ERROR((BTHFP_TAG"Ignore--- Forbid PLAY by SM\n"));
                }
                break;

                case SM_BODY_PAUSE:
                {
                    DBG_ERROR((BTHFP_TAG"Ignore--- Forbid PAUSE by SM\n"));
                }
                break;

                case SM_BODY_STOP:
                {
                    DBG_ERROR((BTHFP_TAG"Ignore--- Forbid STOP by SM\n"));
                }
                break;

                default:
                break;
            }
        }
        break;
		
		case SM_REQUEST_GRP:
        {
            switch(SM_MSG_BDY(pv_msg->ui4_msg_type))
            {
                DBG_ERROR((BTHFP_TAG"SM_MSG_BDY is %d \r\n",SM_MSG_BDY(pv_msg->ui4_msg_type)));

				case SM_BODY_PLAY:
				{
                    DBG_ERROR((BTHFP_TAG"REQUEST PLAY by SM\n"));
				}
                break;
#if 0
				case SM_BODY_RESUME:
                {
                    DBG_ERROR((BTHFP_TAG"REQUEST RESUME by SM\n"));
                }
                break;
#endif
                case SM_BODY_PAUSE:
				{
                    DBG_ERROR((BTHFP_TAG"REQUEST PAUSE by SM\n"));
                }
                break;

				case SM_BODY_STOP:
                {
                    DBG_ERROR((BTHFP_TAG"REQUEST STOP by SM\n"));
					
			        i4_ret = _bt_hfp_sm_request_stop_proc(pv_msg);
			        BT_AUD_CHK_FAIL(_bt_hfp_sm_request_stop_proc, i4_ret);
                }
                break;

			    default:
			        break;
            }
        }
	    break;
    case SM_BT_HFP_ANSWER_CALL:
        _bt_hfp_answer_call();
        break;
    case SM_BT_HFP_HANGUP_CALL:
        _bt_hfp_hange_up_call();
        break;
	default:
	    DBG_ERROR((BTAUD_TAG"Err:can't recognize the SM msgtype .\r\n"));
	    return BT_AUD_FAIL;
    }

    BT_AUD_FUNC_EXIT();
    return BT_AUD_OK;

}


