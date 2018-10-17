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


#ifndef _U_USER_INTERFACE_H_
#define _U_USER_INTERFACE_H_
/*-----------------------------------------------------------------------------
                    include files
-----------------------------------------------------------------------------*/
#include "u_common.h"
#include "u_dbg.h"
/*-----------------------------------------------------------------------------
	user event : user trigger to ctrl the device.
				these events may come from phone msg/IR key/Panel key,
			 	key event include key up/key down/long press/key combination.

			 	These is a kind of event which can directly map to user cmd, which we named "Directly cmd".
			 	This can be defined just the same as phone's cmd, and easy used for cli debugging.

	user cmd : map user event to certain single ctrl cmd which the device can response directly, like pause playback, start bt pairing.

	example : customer may define long press mute key as BT key.
			  event will be : mute_down.mute_press.....mute_up
			  this event will be map to user cmd : E_USERCMD_TYPE_BT_CTRL.E_BT_CTRL_TYPE_PAIR.
-----------------------------------------------------------------------------*/
#if 0
typedef enum _E_USEREVENT_TYPE{
	E_USEREVENT_TYPE_DIRECT_CMD,
	E_USEREVENT_TYPE_KEYEVENT,
	E_USEREVENT_TYPE_MAX,
}E_USEREVENT_TYPE;

typedef struct _T_USEREVENT{
	E_USEREVENT_TYPE event_type;
	void *pv_event;
}T_USEREVENT;

/**
	user cmd
*/
typedef enum _E_USERCMD_TYPE{
	E_USERCMD_TYPE_DEV_CTRL,
	E_USERCMD_TYPE_PB_CTRL,
	E_USERCMD_TYPE_AUDIO_PP,
	E_USERCMD_TYPE_BT_CTRL,
	E_USERCMD_TYPE_MAX,
}E_USERCMD_TYPE;

typedef enum _E_DEV_CTRL_TYPE{
	E_DEV_CTRL_TYPE_POWERKEY,
	E_DEV_CTRL_TYPE_AUTOTIMER_SET,
	E_DEV_CTRL_TYPE_MAX,
}E_DEV_CTRL_TYPE;

typedef enum _E_PB_CTRL_TYPE{
	E_PB_CTRL_TYPE_PAUSE,
	E_PB_CTRL_TYPE_PLAY,
	E_PB_CTRL_TYPE_NEXT,
	E_PB_CTRL_TYPE_PREV,
	E_PB_CTRL_TYPE_MAX,
}E_PB_CTRL_TYPE;

typedef enum _E_AUDIO_PP_TYPE{
	E_AUDIO_PP_TYPE_VOL_PLUS,
	E_AUDIO_PP_TYPE_VOL_REDUCE,
	E_AUDIO_PP_TYPE_MUTE,
	E_AUDIO_PP_TYPE_MAX,
}E_AUDIO_PP_TYPE;

typedef struct _T_USERCMD{
	E_USERCMD_TYPE cmd_type;
	union{
		E_DEV_CTRL_TYPE dev_ctrl_cmd;
		E_PB_CTRL_TYPE pb_ctrl_cmd;
		E_AUDIO_PP_TYPE audio_pp_cmd;
	}cmd;
	void *pargs;
}T_USERCMD;
#endif
extern BOOL u_ui_get_factory_reset_flag(VOID);
extern VOID u_ui_set_factory_reset_flag(BOOL flag);
extern VOID u_ui_set_wifi_setup_flag(BOOL flag);
extern BOOL u_ui_get_wifi_setup_flag(VOID);
extern VOID u_ui_set_bt_paring_flag(BOOL flag);
extern BOOL u_ui_get_bt_paring_flag(VOID);
#endif /* _U_USER_INTERFACE_H_ */
