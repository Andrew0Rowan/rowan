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


#ifndef _BLUETOOTH_AUDIO_PTE_H_
#define _BLUETOOTH_AUDIO_PTE_H_

#include "u_common.h"
#include "u_handle.h"
#include "u_timerd.h"
#define BT_AUD_HANDLE_VOLUME_KEY        0   //handle volume key by itself

#define BT_AUD_NAME                     "BLUETOOTH_AUDIO"
#define DONGLE_NODE_PATH                "/dev/stpbt"
#define BT_AUD_STR_MAX_LEN              20
#define STICKY_PAIRING_MAX_TIME         (60*1000) //ms

#define BTAUD_TAG                       "<BT_AUD> "
#define BTCLI_TAG                       "<BT_CLI> "
#define DBG_INIT_LEVEL_APP_BT_AUD       (DBG_LEVEL_API | DBG_LEVEL_ERROR | DBG_LEVEL_INFO | DBG_LAYER_APP)
#undef  DBG_LEVEL_MODULE
#define DBG_LEVEL_MODULE                _bluetooth_get_dbg_level()

#define DBG_BT_AUD_INFO(fmt, args...) do{DBG_INFO(("%s"fmt, BTAUD_TAG, ##args));}while(0)
#define DBG_BT_AUD_API(fmt, args...)  do{DBG_API(("%s"fmt, BTAUD_TAG, ##args));}while(0)
#define DBG_BT_AUD_ERR(fmt, args...)  do{DBG_ERROR(("%s"fmt, BTAUD_TAG, ##args));}while(0)

#define BT_AUD_FUNC_ENTRY()   DBG_INFO((BTAUD_TAG"---Enter:Func=%s, Line=%d---\r\n",__FUNCTION__, __LINE__))
#define BT_AUD_FUNC_EXIT()    DBG_INFO((BTAUD_TAG"---Exit:Func=%s, Line=%d---\r\n",__FUNCTION__, __LINE__))

#define BT_AUD_CHK_FAIL_GOTO(_func, _ret, _label)   \
do{if (_ret != 0){DBG_ERROR((BTAUD_TAG"Err:"#_func" return err: ret=%d.\r\n", _ret));goto _label;}}while(0)
#define BT_AUD_CHK_FAIL_RET(_func, _ret, _ret_on_err)   \
do{if (_ret != 0){DBG_ERROR((BTAUD_TAG"Err:"#_func" return err: ret=%d.\r\n", _ret));return _ret_on_err;}}while(0)
#define BT_AUD_CHK_FAIL(_func, _ret)    \
do{if (_ret != 0){DBG_ERROR((BTAUD_TAG"Err:"#_func" return err: ret=%d.\r\n", _ret));}}while(0)


typedef enum{
    BT_AUD_PTE_MSG_BT = 1,          //bluetooth msg
    BT_AUD_PTE_MSG_DM,              //DM msg          
    BT_AUD_PTE_MSG_KEY,             //key msg
    BT_AUD_PTE_MSG_SRC,             //src mode msg

    BT_AUD_PTE_MSG_MAX
}BT_AUD_PTE_MSG_TYPE;

typedef enum{
    BT_AUD_MSG_BT_GAP_EVENT = 1,    //GAP EVENT
    BT_AUD_MSG_BT_INQUIRY_RESP,     //inquiry response device info
    BT_AUD_MSG_BT_A2DP_EVENT,       //A2DP EVENT
    BT_AUD_MSG_BT_AVRCP_EVENT,      //AVRCP EVENT
    BT_AUD_MSG_BT_PLAY_STATUS,      //PLAY STATUS
    BT_AUD_MSG_BT_VOLUME_CHG,       //VOLUME CHANGE
    BT_AUD_MSG_BT_TRACK_CHG,        //TRACK CHANGE
    BT_AUD_MSG_BT_MEDIAINFO_PLAYSTATUS,//mediainfo playstatus event
    BT_AUD_MSG_KEY_PLAY_CTRL,       //PLAY CONTROL KEY
    BT_AUD_MSG_KEY_BTKEY,           //BTKEY
    BT_SRC_MSG_MEDIAINFO_EVENT,
    
    BT_AUD_MSG_MAX
}BT_AUD_CB_MSG_ID;

typedef enum{
    BT_AUD_MSG_BT_GAP_PROFILE = 1,    //GAP EVENT
	BT_AUD_MSG_BT_A2DP_PROFILE,
	BT_AUD_MSG_BT_AVRCP_PROFILE,
	BT_AUD_MSG_BT_HFP_PROFILE,

    BT_AUD_MSG_TPYE_PROFILE_MAX
}BT_AUD_CB_MSG_TYPE_PROFILE;

typedef enum
{
    BT_AUD_MSG_A2DP_EVENT_CONNECTED,        /* A2DP connected    */
    BT_AUD_MSG_EVENT_DISCONNECTED,     /* A2DP disconnected */
    BT_AUD_MSG_EVENT_STREAM_SUSPEND,   /* A2DP suspend      */
    BT_AUD_MSG_EVENT_STREAM_START,     /* A2DP start        */
    BT_AUD_MSG_EVENT_CONNECT_COMING,   /* A2DP connect comming */
    BT_AUD_MSG_EVENT_PLAYER_EVENT,     /* A2DP Local playback status */
    BT_AUD_MSG_EVENT_MAX
} BT_AUD_MSG_A2DP_EVENT;

/* application structure */
typedef struct _BT_AUD_OBJ_T
{
    HANDLE_T    h_app;                      //app handle
    BOOL        b_app_init_ok;              //app init ok flag
    BOOL        b_bt_has_power_on;          // bluetooth device RF turn on flag
    TIMER_TYPE_T            t_stream_chk_timer;  
    TIMER_TYPE_T            t_sticky_pairing_timer;  // sticky pairing timer for connection dropped abruptly.
    TIMER_TYPE_T            t_key_delay_timer;     //for key simulator
    TIMER_TYPE_T            t_enter_pairing_timer;     //for enter paring mode
}BT_AUD_OBJ_T;

//the struct of BT audio applicatio's message
typedef struct _BT_AUD_MSG_T
{
	UINT32          ui4_msg_type;
	UINT8           ui4_msg_type_profile;          
	UINT32          ui4_msg_id;
    UINT32          ui4_data1;
    UINT8           ui4_data2;
    UINT32          ui4_data3;
	INT16           ui4_data4;
	CHAR*           ui4_str1[BT_AUD_STR_MAX_LEN];
	//CHAR*			ui4_str2[BT_AUD_STR_MAX_LEN];
#if CONFIG_APP_SUPPORT_BT_SRC_MODE
	BT_SRC_PLAYER_STATUS_T btmedialnfo;
#endif /*CONFIG_APP_SUPPORT_BT_SRC_MODE*/
} BT_AUD_MSG_T;


/*  the struct of BT key */  
typedef struct{
    CHAR* pc_keystr;
    UINT32 ui4_keymsg;
}BT_AUD_KEY2MSG_T;


/*------------------------------------------------------------------------------
                                            funcitons declarations
------------------------------------------------------------------------------*/
extern UINT16 _bluetooth_get_dbg_level(VOID);
extern VOID _bluetooth_set_dbg_level(UINT16 ui2_db_level);
extern INT32 _bluetooth_send_msg_to_itself(UINT32 ui4_msgtype, BT_AUD_MSG_T* pt_bt_msg);
extern INT32 _bluetooth_set_bt_power(BOOL b_power_on);
extern BOOL _bluetooth_is_bt_power_on(VOID);
extern BOOL _bluetooth_is_bt_power_setting_on(VOID);
//extern BOOL bt_aud_is_bt_dongle_connect(VOID);  



#endif  //_BLUETOOTH_AUDIO_PTE_H_