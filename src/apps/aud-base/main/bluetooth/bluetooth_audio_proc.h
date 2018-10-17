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


#ifndef _BLUETOOTH_AUDIO_PROC_H_
#define _BLUETOOTH_AUDIO_PROC_H_

#include "u_common.h"
#include "u_handle.h"
#include "u_acfg.h"
#include "u_assistant_stub.h"
#include <string.h>
#include "u_bt_mw_a2dp.h"
#if CONFIG_APP_SUPPORT_BT_SRC_MODE
#include "mas_lib.h"
#endif /*CONFIG_APP_SUPPORT_BT_SRC_MODE*/


//#include "u_appman.h"

/* return value */
#define BT_AUD_OK                            ((INT32)0)
#define BT_AUD_FAIL                          ((INT32)-1) /* abnormal return must < 0 */

#define BT_AUD_PAIRING_TIMEOUT     0        //s,  0 means forever
#define BT_AUD_KEY_DELAY_TIMEOUT   150      //ms, widget will send key event after 200ms,so time out value must less than it
#define BT_AUD_STREAM_CHK_TIMEOUT           1000     //ms
#define BT_AUD_STREAM_FIRST_CHK_TIMEOUT     10000     //ms
#define BT_AUD_ENTER_PARING_TIMEOUT         (2*60*1000)
#define BT_AUD_REMOTE_DEV_MAX_VOLUME    0x7F    //remote device max volume value
#define BT_AUD_LOCAL_MAX_VOLUME         VOLUME_MAX    //local max volume value
#define BT_AUD_VOLUME_CHANGE_STEP       5       //volume change step
#define SECONDS_BT_DM_COUNT   500
#define BT_AUD_MAX_STR_LEN    255

#define BLUETOOTH_NAME_SAVE_PATH       "/data/misc/bluetooth_name.txt" /*bluetooth_name path*/
#define BLUETOOTH_DEVNAME_MAXSIZE      80
#define BLUETOOTH_NAME_PRE             "MT8516"
#define BT_NAME_SUF_LEN                    5
#define BT_LOACL_NAME_LEN              (strlen(BLUETOOTH_NAME_PRE)+BT_NAME_SUF_LEN)


enum{
    BT_AUD_STOP,
    BT_AUD_PLAY
};

typedef enum{
    BT_AUD_BT_STA_IDLE,
    BT_AUD_BT_STA_PAIRING,
    BT_AUD_BT_STA_CONNECTED,
    BT_AUD_BT_STA_PLAYING,
    BT_AUD_BT_STA_AUTO_CONNECTING,
    BT_AUD_BT_STA_DISCONNECT,

    BT_AUD_BT_STA_MAX
}BT_AUD_BT_STA_T;

typedef enum{
    BT_AUD_A2DP_SINK_MODE =1,
    BT_AUD_A2DP_SOURCE_MODE,
    BT_AUD_A2DP_UNKNOW_MODE,
}BT_AUD_A2DP_MODE_T;

enum{
    BT_A2DP_MAS_CLOSE,
    BT_A2DP_MAS_OPEN
};

typedef enum{
	BT_AUD_SM_IDLE = 1,
	BT_AUD_SM_PMT_PLAY,
	BT_AUD_SM_FBD_PLAY,
    BT_AUD_SM_REQ_PLAY,
    BT_AUD_SM_REQ_PAUSE,
    BT_AUD_SM_REQ_RESUME,
   	BT_AUD_SM_REQ_STOP,
    BT_AUD_SM_REQ_NEXT,
	BT_AUD_SM_REQ_PREV,
	BT_AUD_SM_REQ_RECNT,
	BT_AUD_SM_REQ_DISCNT,
	BT_AUD_SM_REQ_PAIR,

    BT_AUD_SM_CMD_MAX
}BT_AUD_SM_CMD_T;

typedef enum{
    BT_AUD_SRC_BP_STA_STOP = 0,
    BT_AUD_SRC_BP_STA_PLAY,
    BT_AUD_SRC_BP_STA_PAUSE,
    BT_AUD_SRC_BP_STA_RESUME,

    BT_AUD_SRC_BP_STA_MAX
}BT_AUD_SRC_BP_STA_T;

typedef enum{
    BT_AUD_SYSTEM_NORMAL = 0,
    BT_AUD_SYSTEM_FAKE_STANDBY,
    BT_AUD_SYSTEM_SUSPEND_STANDBY,
    
    BT_AUD_SYSTEM_STA_MAX
}BT_AUD_SYSTEM_STA_T;


typedef struct{
    BOOL b_bt_init;                 //init only do one time
    BOOL b_bt_open;                 //current input src
    BT_AUD_BT_STA_T e_bt_sta;       //bluetooth audio status
    BT_AUD_SM_CMD_T e_bt_sm_cmd_sta;   //blue
    BOOL b_bt_forced_pairing;       //forced pairing mode
    BOOL b_bt_play;                 //play status
    UINT32 ui4_bt_track_idx;        //track idx
    BOOL b_continue_connect;        //continue to connect paired list
    BOOL b_keep_connect;            //keep connect even if BT isn't current input source
    BOOL b_sticky_pairing_enable;   //sticky pairing  flag.
    BOOL b_enter_pairing_enable;   //enter pairing  flag.

    BOOL b_play_pause_down;         //play/pause key press down
    BOOL b_connectable_background;   //conenctable background

	BOOL b_bt_source;                //bt source
	BOOL b_bt_play_pause_proc;       //play/pasue proc
	BOOL b_bt_a2dp_connect;			 //a2dp connect
	BOOL b_bt_avrcp_connect;         //avrcp connect
#if CONFIG_SUPPORT_BT_HFP
	BOOL b_bt_hfp_connect;           //hfpclient connect
#endif /*CONFIG_SUPPORT_BT_HFP*/
	BOOL b_bt_init_auto_connect;     //init auto connect
	BOOL b_bt_state_play_pause_proc;       //play pause proc
	BOOL b_bt_reconnect_for_wifi;    //reconnect for wifi
	BOOL b_bt_open_player;
	BOOL b_bt_mas_mode;
	BT_AUD_SRC_BP_STA_T e_bt_src_playback_state;    //for source mode record playback playstatus
	BT_A2DP_ROLE e_bt_a2dp_role_disable;
	BT_AUD_A2DP_MODE_T  e_bt_a2dp_mode_switch;
	BT_AUD_SYSTEM_STA_T e_bt_system_sta;       //for system status
	//BOOL b_bt_forbbid_play_by_hfp;    //forbid bt play while hfp playing
}BT_AUD_BLUETOOTH_STATUS_T;

typedef enum
{
	BT_TIMER_MSG_STREAM_CHK=1,
	BT_TIMER_MSG_STICKY_PAIRING,
	BT_TIMER_MSG_KEY_DELAY,
	BT_TIMER_MSG_ENTER_PARING,

    BT_TIMER_MSG_MAX
} BT_TIMER_MSG_E;


/*------------------------------------------------------------------------------
                                            funcitons declarations
------------------------------------------------------------------------------*/
extern INT32 _bluetooth_bt_init(VOID);
extern INT32 _bluetooth_bt_close(BOOL b_keep_connect);
extern INT32 _bluetooth_bt_open(VOID);
extern INT32 _bluetooth_bt_disconnect(BOOL b_keep_connect);
extern INT32 _bluetooth_start_pairing(VOID);
extern INT32 _bluetooth_auto_connect(UINT32 ui4_devnum, BOOL b_start_init);
extern INT32 _bluetooth_connect(const CHAR *pc_mac_addr);
extern VOID _bluetooth_ui_msg_process(const VOID *pv_msg, SIZE_T z_msg_len);
extern VOID _bluetooth_process_timer_msg(const VOID* pv_msg, SIZE_T z_msg_len);
extern INT32 _bluetooth_send_request_play_to_sm(void);
extern INT32 bluetooth_app_pre_play_init(VOID);
extern VOID _bluetooth_clear_connect_info(BOOL b_keep_connect);
extern INT32 u_bluetooth_set_bt_name(const CHAR *setname);
extern INT32 _bluetooth_set_name_assistant_stub(BYTE *uc_usr_msg);
extern INT32 _bluetooth_send_bt_status_to_assistant_stub(CHAR *bt_status);
extern INT32 _bluetooth_bt_reconnect_assistant(VOID);
extern ASSISTANT_STUB_BLUETOOTH_STATUS_CHANGE_T* u_bluetooth_get_bt_status_assistant(VOID);
extern ASSISTANT_STUB_PLAYER_STATUS_CHANGE_T* u_bluetooth_get_player_status(VOID);
extern INT32 _bluetooth_send_player_status_to_assistant_stub(VOID);
extern INT32 _bluetooth_wifi_status_msg_proc(APPMSG_T* pv_msg);
extern INT32 _bluetooth_disconnect_assistant_stub_msg_proc(BOOL b_keep_connect);
extern INT32 _bluetooth_start_play_assistant_stub_msg_proc(VOID);
extern INT32 _bluetooth_start_pairing_assistant_stub_msg_proc(VOID);
extern INT32 _bluetooth_assistant_stub_msg_proc(APPMSG_T* pv_msg);



/*  timer cb  */
extern VOID _bluetooth_sticky_pairing_timer_cb(VOID);
extern VOID _bluetooth_key_delay_timer_cb(VOID);
extern VOID _bluetooth_enter_pairing_timer_cb(VOID);


#if CONFIG_APP_SUPPORT_BT_STICKY_PAIRING
extern INT32 _bluetooth_start_sticky_pairing_timer(VOID);
extern INT32 _bluetooth_stop_sticky_pairing_timer(VOID);
#endif/* CONFIG_APP_SUPPORT_BT_STICKY_PAIRING */


extern INT32 _bluetooth_enter_pairing_key_proc(VOID);
extern INT32 _bluetooth_break_link(VOID);
extern BT_AUD_BLUETOOTH_STATUS_T* _bluetooth_get_bt_status(VOID);
extern INT32 _bluetooth_paired_dev_list_update(BT_A2DP_DEVICE_LIST *pt_dev_list);
extern INT32 _bluetooth_set_local_name(const CHAR* pc_btname);
extern BOOL _bluetooth_is_bt_profile_connected(VOID);
extern BOOL _bluetooth_is_bt_connected(VOID);
extern BOOL _bluetooth_is_bt_ready(VOID);
extern INT32 _bluetooth_key_cmd_proc (const CHAR *key_cmd);
extern INT32 _bluetooth_timer_create(VOID);
extern INT32 _bluetooth_timer_delete(VOID);

#if CONFIG_APP_SUPPORT_BT_BACKGROUND_CONNECTABLE
extern BOOL _bluetooth_is_connectable_background(VOID);
extern INT32 _bluetooth_set_connectable_background(BOOL b_connectable);
#endif/* CONFIG_APP_SUPPORT_BT_BACKGROUND_CONNECTABLE */

/**------------------- -----API about SM ----------------------------------------**/
extern INT32 _bluetooth_send_inform_play_to_sm(void);
extern INT32 _bluetooth_send_inform_stop_to_sm(void);
extern INT32 _bluetooth_send_inform_pause_to_sm(void);
extern INT32 _bluetooth_send_pairing_result_to_sm(void);
//-----------------------------------------------------------------------------



#endif  //_BLUETOOTH_AUDIO_PROC_H_
