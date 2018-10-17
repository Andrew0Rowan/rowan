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
CHAR g_cur_gap_addr[18];
tBTMW_GAP_DEVICE_INFO g_cur_target_device_info = {0};
UINT8 g_cur_gap_scan_mode = 0;

extern CHAR g_cur_a2dp_addr[18];
extern BT_AUD_BLUETOOTH_STATUS_T g_t_bt_aud_sta;
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


/*-----------------------------------------------------------------------------
 * extern methods declarations
 *---------------------------------------------------------------------------*/




 VOID _bluetooth_gap_event_cbk_fct(tBTMW_GAP_STATE *bt_event)
 {
	 BT_AUD_MSG_T t_msg = {0};
	 BT_AUD_FUNC_ENTRY();
 
	 if (NULL == bt_event)
	 {
		 DBG_ERROR((BTAUD_TAG"GAP event cb, bt_event is NULL !!\r\n"));
		 return;
	 }
 
	 DBG_ERROR((BTAUD_TAG"GAP event cb, bt_event.state = %d.\r\n", bt_event->state));
 
	 switch(bt_event->state)
	 {
		 case GAP_STATE_ON:
		 case GAP_STATE_OFF:
		 case GAP_STATE_BONDING:
		 case GAP_STATE_DISCOVERY_STARTED:
		 case GAP_STATE_DISCOVERY_STOPED:
		 break;
 
		 case GAP_STATE_ACL_CONNECTED:
		 case GAP_STATE_BONDED:
		 case GAP_STATE_NO_BOND:
		 {
			 strncpy(g_cur_gap_addr,
					 bt_event->bd_addr,
					 MAX_BDADDR_LEN);
			 strncpy(t_msg.ui4_str1,
					 bt_event->bd_addr,
					 MAX_BDADDR_LEN);
		 }
		 break;
		 
		 case GAP_STATE_ACL_DISCONNECTED:
		 {
			 t_msg.ui4_data2 = bt_event->reason;
			 strncpy(g_cur_gap_addr,
					 bt_event->bd_addr,
					 MAX_BDADDR_LEN);
			 strncpy(t_msg.ui4_str1,
					 bt_event->bd_addr,
					 MAX_BDADDR_LEN);
		 }
		 break;
 
		 default:
			 break; 	 
	 }
 
	 t_msg.ui4_msg_type 		= BT_AUD_PTE_MSG_BT;
	 t_msg.ui4_msg_type_profile = BT_AUD_MSG_BT_GAP_PROFILE;
	 t_msg.ui4_msg_id			= BT_AUD_MSG_BT_GAP_EVENT;
	 t_msg.ui4_data1			= bt_event->state;
	 
	 _bluetooth_send_msg_to_itself(E_APP_MSG_TYPE_BLUETOOTH, &t_msg);
 }
 
 VOID _bluetooth_gap_inquiry_dev_info_cbk_fct(tBTMW_GAP_DEVICE_INFO* pt_result)
{
	BT_AUD_MSG_T t_msg = {0};
	BT_AUD_FUNC_ENTRY();
	DBG_ERROR((BTAUD_TAG"inquiry device response cb, device_kind = %d.\r\n",pt_result->device_kind));

	memcpy(&g_cur_target_device_info,pt_result,sizeof(tBTMW_GAP_DEVICE_INFO));
	strcpy(g_t_bt_aud_bt_status.bt_paired_name,pt_result->device.name);
	strncpy(t_msg.ui4_str1,
	 		pt_result->device.bdAddr,
	 		MAX_BDADDR_LEN);
#if 0
	strncpy(t_msg.ui4_str2,
			pt_result->device.name,
			MAX_BDADDR_LEN);
#endif
	t_msg.ui4_msg_type 		= BT_AUD_PTE_MSG_BT;
	t_msg.ui4_msg_type_profile = BT_AUD_MSG_BT_GAP_PROFILE;
	t_msg.ui4_msg_id			= BT_AUD_MSG_BT_INQUIRY_RESP;
	t_msg.ui4_data1			= pt_result->device_kind;
	t_msg.ui4_data3			= pt_result->device.devicetype;
	t_msg.ui4_data4			= pt_result->device.rssi;

	_bluetooth_send_msg_to_itself(E_APP_MSG_TYPE_BLUETOOTH, &t_msg);
 }

VOID _bluetooth_get_cur_target_device_info(tBTMW_GAP_DEVICE_INFO* pt_device_info)
{
	if (NULL == pt_device_info)
	{
		DBG_ERROR((BTAUD_TAG"Err: pt_device_info is NULL!!\r\n"));
		return ;
	}

	memcpy(pt_device_info,&g_cur_target_device_info,sizeof(tBTMW_GAP_DEVICE_INFO));
}

 INT32 _bluetooth_gap_base_init(VOID)
 {
	 INT32 i4_ret = BT_AUD_FAIL;
	 MTKRPCAPI_BT_APP_CB_FUNC t_bt_cbk_func; 
	 char pv_tag[2]={0}; 
	 
	 BT_AUD_FUNC_ENTRY();
 
	 memset(&t_bt_cbk_func, 0x0, sizeof(BT_APP_CB_FUNC));
	 t_bt_cbk_func.bt_event_cb				= _bluetooth_gap_event_cbk_fct;
	 t_bt_cbk_func.bt_dev_info_cb			= _bluetooth_gap_inquiry_dev_info_cbk_fct;
	 
	 i4_ret = a_mtkapi_gap_bt_base_init(&t_bt_cbk_func,(void*)pv_tag);
	 DBG_ERROR((BTAUD_TAG"gap bt base_init complete. \n"));
	 BT_AUD_CHK_FAIL_RET(a_mtkapi_gap_bt_base_init, i4_ret, BT_AUD_FAIL);
 
	 
	 BT_AUD_FUNC_EXIT();
	 return BT_AUD_OK;
 }

 //get remote device name which connected with the platform
 INT32 _bluetooth_get_remote_device_name(CHAR *device_name, CHAR *device_addr)
{
	 INT32 i4_ret = BT_AUD_FAIL;
	 BLUETOOTH_DEVICE pt_target_dev_info;

	 BT_AUD_FUNC_ENTRY();

	 if (NULL == device_addr)
	 {
		DBG_ERROR((BTAUD_TAG"device addr is NULL!! \r\n"));
		return BT_AUD_FAIL;
	 }

	 if(17 != strlen(device_addr))
	 {
		 DBG_ERROR((BTAUD_TAG"device addr length should be 17 !! \r\n"));
		 return BT_AUD_FAIL;
	 }

	 DBG_ERROR((BTAUD_TAG"get remote device name \r\n"));

	 memset(&pt_target_dev_info, 0, sizeof(BLUETOOTH_DEVICE));
	 i4_ret = a_mtkapi_bt_gap_get_dev_info(&pt_target_dev_info,device_addr);
	 BT_AUD_CHK_FAIL(a_mtkapi_bt_gap_get_dev_info, i4_ret);

	 strcpy(device_name, pt_target_dev_info.name);
	 DBG_ERROR((BTAUD_TAG"the remote device name is : %s\r\n", device_name));

	 BT_AUD_FUNC_EXIT();
	 return BT_AUD_OK;
}

//bt power on proc
  static INT32 _bluetooth_gap_state_on_proc(VOID)
{
	INT32 i4_ret = BT_AUD_FAIL;

	BT_AUD_FUNC_ENTRY();

	DBG_API((BTAUD_TAG"BT_POWER_ON.\r\n"));
	strcpy(g_t_bt_aud_bt_status.status,"power_on");

	DBG_ERROR((BTAUD_TAG"POWER_ON----send notification to ASSISTANT_STUB \n"));
	i4_ret = _bluetooth_send_bt_status_to_assistant_stub("power_on");
	BT_AUD_CHK_FAIL(_bluetooth_send_bt_status_to_assistant_stub, i4_ret);

	 BT_AUD_FUNC_EXIT();
	 return BT_AUD_OK;
}
 
 //bt power off proc
static INT32 _bluetooth_gap_state_off_proc(VOID)
{
	INT32 i4_ret = BT_AUD_FAIL;

	BT_AUD_FUNC_ENTRY();

	DBG_API((BTAUD_TAG"BT_POWER_OFF.\r\n"));
	strcpy(g_t_bt_aud_bt_status.status,"power_off");

	DBG_ERROR((BTAUD_TAG"POWER_OFF----send notification to ASSISTANT_STUB \n"));
	i4_ret = _bluetooth_send_bt_status_to_assistant_stub("power_off");
	BT_AUD_CHK_FAIL(_bluetooth_send_bt_status_to_assistant_stub, i4_ret);

	BT_AUD_FUNC_EXIT();
	return BT_AUD_OK;
}

 static INT32 _bluetooth_gap_connect_lost_proc(const CHAR *device_addr)
{
	INT32 i4_ret = BT_AUD_FAIL;

	BT_AUD_FUNC_ENTRY();

	DBG_ERROR((BTAUD_TAG"bluetooth_gap_connect_loss_proc.\r\n"));	 

	//connection dropped abruptly, need to try to reconnect
	if (TRUE == _bluetooth_is_bt_ready())
	{
		if (FALSE == _bluetooth_is_bt_connected())
		{
			 DBG_API((BTAUD_TAG"Disconnect msg has come early, here need to enter sticky pairing.\r\n"));

			 i4_ret = _bluetooth_connect(device_addr);
			 BT_AUD_CHK_FAIL_RET(_bluetooth_connect, i4_ret, BT_AUD_FAIL);

			 if (i4_ret == BT_AUD_OK)
			 {
				 _bluetooth_start_sticky_pairing_timer();
				 return i4_ret;
			 }

			 DBG_API((BTAUD_TAG"connect fail, stop sticky pairing.\r\n"));
			 _bluetooth_stop_sticky_pairing_timer();
			}
			else
			{
			    //start sticky pairing timer, and then start auto connect when disconnect msg comes
			    if (g_t_bt_aud_sta.b_sticky_pairing_enable == FALSE)
			    {
			        _bluetooth_start_sticky_pairing_timer();
			    }
			}
	}

	BT_AUD_FUNC_EXIT();
	return BT_AUD_OK;
}

 static INT32 _bluetooth_gap_state_bonded_proc(const CHAR *device_addr)
{
	INT32 i4_ret = BT_AUD_FAIL;
	CHAR device_name[MAX_NAME_LEN]={0};

	BT_AUD_FUNC_ENTRY();

	DBG_ERROR((BTAUD_TAG"bluetooth_gap_state_bonded_proc.\r\n"));

	strcpy(g_t_bt_aud_bt_status.status,"pair_success");

	if(TRUE ==g_t_bt_aud_sta.b_enter_pairing_enable)
	{
	 i4_ret = _bluetooth_send_pairing_result_to_sm();
	 BT_AUD_CHK_FAIL(_bluetooth_send_pairing_result_to_sm, i4_ret);
	}

	DBG_ERROR((BTAUD_TAG"PAIRING_SUCCESS----send notification to ASSISTANT_STUB \n"));

	i4_ret = _bluetooth_send_bt_status_to_assistant_stub("pair_success");
	BT_AUD_CHK_FAIL(_bluetooth_send_bt_status_to_assistant_stub, i4_ret);
#if 0
	strncpy(g_t_bt_aud_bt_status.bt_paired_name,
		 device_name,
		 ASSISTANT_STUB_STATUS_MAX_LENGTH);
#endif

	i4_ret = _bluetooth_get_remote_device_name(device_name,device_addr);
	BT_AUD_CHK_FAIL(_bluetooth_get_remote_device_name, i4_ret);

	strncpy(g_t_bt_aud_bt_status.bt_paired_name, 
			device_name,
			ASSISTANT_STUB_STATUS_MAX_LENGTH);

	BT_AUD_FUNC_EXIT();
	return BT_AUD_OK;
}
 
 static INT32 _bluetooth_gap_state_nobond_proc(const CHAR *device_addr)
{
	INT32 i4_ret = BT_AUD_FAIL;
	CHAR device_name[MAX_NAME_LEN]={0};

	BT_AUD_FUNC_ENTRY();

	DBG_ERROR((BTAUD_TAG"bluetooth_gap_state_nobond_proc.\r\n"));

	strcpy(g_t_bt_aud_bt_status.status,"pair_failed");
#if 0
	strncpy(g_t_bt_aud_bt_status.bt_paired_name,
		 device_name,
		 ASSISTANT_STUB_STATUS_MAX_LENGTH);
#endif
	if(TRUE ==g_t_bt_aud_sta.b_enter_pairing_enable)
	{
	 i4_ret = _bluetooth_send_pairing_result_to_sm();
	 BT_AUD_CHK_FAIL(_bluetooth_send_pairing_result_to_sm, i4_ret);
	}

	DBG_ERROR((BTAUD_TAG"PAIR_FAILED----send notification to ASSISTANT_STUB \n"));

	i4_ret = _bluetooth_send_bt_status_to_assistant_stub("pair_failed");
	BT_AUD_CHK_FAIL(_bluetooth_send_bt_status_to_assistant_stub, i4_ret);	

	i4_ret = _bluetooth_get_remote_device_name(device_name,device_addr);
	BT_AUD_CHK_FAIL(_bluetooth_get_remote_device_name, i4_ret);

	strncpy(g_t_bt_aud_bt_status.bt_paired_name, 
			device_name,
			ASSISTANT_STUB_STATUS_MAX_LENGTH);

	BT_AUD_FUNC_EXIT();
	return BT_AUD_OK;
}
 
 static INT32 _bluetooth_gap_event_proc(const VOID *pv_msg)
 {
	 INT32 i4_ret;
	 BT_AUD_MSG_T* pt_bt_gap_msg = (BT_AUD_MSG_T*)pv_msg;
 
	 BT_AUD_FUNC_ENTRY();
	 
	 switch(pt_bt_gap_msg->ui4_data1)
	 {
		 case GAP_STATE_ON:
		 {
			 DBG_ERROR((BTAUD_TAG"GAP_STATE_ON.\r\n"));
			 i4_ret = _bluetooth_gap_state_on_proc();
			 BT_AUD_CHK_FAIL_RET(_bluetooth_gap_state_on_proc, i4_ret, BT_AUD_FAIL);
		 }
		 break;
 
		 case GAP_STATE_OFF:
		 {
			 DBG_ERROR((BTAUD_TAG"GAP_STATE_OFF.\r\n"));
			 i4_ret = _bluetooth_gap_state_off_proc();
			 BT_AUD_CHK_FAIL_RET(_bluetooth_gap_state_off_proc, i4_ret, BT_AUD_FAIL);
		 }
		 break;
 
		 case GAP_STATE_ACL_CONNECTED:
		 {
			 DBG_ERROR((BTAUD_TAG"GAP_STATE_ACL_CONNECTED.\r\n"));
		 }
		 break;
 
		 case GAP_STATE_ACL_DISCONNECTED:
		 {
			 DBG_ERROR((BTAUD_TAG"GAP_STATE_ACL_DISCONNECTED , reason = %d.\r\n",
						 pt_bt_gap_msg->ui4_data2));
						 
			 if(0x08 == pt_bt_gap_msg->ui4_data2)
			 {
				 DBG_ERROR((BTAUD_TAG"BT connect lost , reason = %d.\r\n",
									 pt_bt_gap_msg->ui4_data2));
#if CONFIG_APP_SUPPORT_BT_STICKY_PAIRING
				 i4_ret = _bluetooth_gap_connect_lost_proc(g_cur_a2dp_addr);
				 BT_AUD_CHK_FAIL_RET(_bluetooth_gap_connect_lost_proc, i4_ret, BT_AUD_FAIL);
#endif /*CONFIG_APP_SUPPORT_BT_STICKY_PAIRING*/
			 }
			 else if (BT_AUD_BT_STA_PAIRING != g_t_bt_aud_sta.e_bt_sta)
			 {				
				i4_ret = _bluetooth_start_pairing();
				BT_AUD_CHK_FAIL(_bluetooth_start_pairing, i4_ret);
			 }
		 }
		 break;
 
		 case GAP_STATE_BONDED:
		 {
			 DBG_ERROR((BTAUD_TAG"GAP_STATE_BONDED.\r\n"));
			 i4_ret = _bluetooth_gap_state_bonded_proc(pt_bt_gap_msg->ui4_str1);
			 BT_AUD_CHK_FAIL_RET(_bluetooth_gap_state_bonded_proc, i4_ret, BT_AUD_FAIL);
		 }
		 break;
		 
		 case GAP_STATE_NO_BOND:
		 {
			 DBG_ERROR((BTAUD_TAG"GAP_STATE_NO_BOND.\r\n"));
			 i4_ret = _bluetooth_gap_state_nobond_proc(pt_bt_gap_msg->ui4_str1);
			 BT_AUD_CHK_FAIL_RET(_bluetooth_gap_state_nobond_proc, i4_ret, BT_AUD_FAIL);
		 }
		 break;
 
		 case GAP_STATE_BONDING:
		 {
			 DBG_ERROR((BTAUD_TAG"GAP_STATE_BONDING.\r\n"));		 
		 }
		 break;
 
		 case GAP_STATE_DISCOVERY_STARTED:
		 {
			 DBG_ERROR((BTAUD_TAG"GAP_STATE_DISCOVERY_STARTED.\r\n"));			 
		 }
		 break;  
 
		 case GAP_STATE_DISCOVERY_STOPED:
		 {
			 DBG_ERROR((BTAUD_TAG"GAP_STATE_DISCOVERY_STOPED.\r\n"));			 
		 }
		 break;
 
		 default:
			 DBG_ERROR((BTAUD_TAG"Err:can't recognize the bt_event->state %d.\r\n", 
								 pt_bt_gap_msg->ui4_data1));
			 break;  
	 }
 
	 BT_AUD_FUNC_EXIT();
	 return BT_AUD_OK;
 }
 
 static INT32 _bluetooth_gap_inquiry_resp_proc(const VOID *pv_msg)
 {
	 INT32 i4_ret = BT_AUD_FAIL;
	 BT_AUD_MSG_T* pt_iquiry_bt_msg = (BT_AUD_MSG_T*)pv_msg; 
	 CHAR* device_kind[]={"LOCAL","SCAN","BONDED"};
	 
	 BT_AUD_FUNC_ENTRY();
 
	 DBG_ERROR((BTAUD_TAG"catch a device. Device info:\r\n"));
	 DBG_ERROR(("\t\tdeice_kind:%s\n",device_kind[pt_iquiry_bt_msg->ui4_data1]));
	 //DBG_ERROR(("\t\tname:%s\n",pt_iquiry_bt_msg->ui4_str2));
	 DBG_ERROR(("\t\tBluetooth addr:%s\n",pt_iquiry_bt_msg->ui4_str1));
	 DBG_ERROR(("\t\tRSSI:%d\n",pt_iquiry_bt_msg->ui4_data4));
 
	 BT_AUD_FUNC_EXIT();
	 return BT_AUD_OK;
 }
 
 INT32 _bluetooth_gap_msg_proc(const VOID *pv_msg)
 {
	 INT32 i4_ret = BT_AUD_FAIL;
	 BT_AUD_MSG_T* pt_bt_msg = (BT_AUD_MSG_T*)pv_msg;
	 
	 BT_AUD_FUNC_ENTRY();
 
	 switch (pt_bt_msg->ui4_msg_id)
	 {
		 case BT_AUD_MSG_BT_GAP_EVENT:
		 {
			 i4_ret = _bluetooth_gap_event_proc(pt_bt_msg);
			 BT_AUD_CHK_FAIL(_bluetooth_gap_event_proc, i4_ret);
		 }
		 break;
 
		 case BT_AUD_MSG_BT_INQUIRY_RESP:
		 {
			 i4_ret = _bluetooth_gap_inquiry_resp_proc(pt_bt_msg);
			 BT_AUD_CHK_FAIL(_bluetooth_gap_inquiry_resp_proc, i4_ret);
		 }
		 break;
 
		 default:
			 DBG_ERROR((BTAUD_TAG"Err:can't recognize the bluetooth msgtype %d.\r\n",
								 pt_bt_msg->ui4_msg_id));
			 break;
	 }
 
	 BT_AUD_FUNC_EXIT();
	 return BT_AUD_OK;
 }



