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


#ifndef _U_SM_H_
#define _U_SM_H_

#include "u_common.h"
#include "u_amb.h"
#include "u_c4a_stub.h"
#include "u_assistant_stub.h"

#define SMR_OK                  ((INT32)    0)        /* OK */
#define SMR_FAIL                ((INT32)   -1)        /* Something error? */
#define SMR_INV_ARG             ((INT32)   -2)        /* Invalid arguemnt. */

#define SM_MSG_GRP_OFFSET       ((INT32)   24)                  /*   32 <GRP>24<MSG BODY>16<ID>0*/
#define SM_MSG_GRP_MSK          ((INT32)   0X000000FF)          /*   32 <GRP>24<MSG BODY>16<ID>0*/

#define SM_MSG_BDY_OFFSET       ((INT32)   16)                  /*   32 <GRP>24<MSG BODY>16<ID>0*/
#define SM_MSG_BDY_MSK          ((INT32)   0X000000FF)          /*   32 <GRP>24<MSG BODY>16<ID>0*/

#define SM_MSG_ID_OFFSET        ((INT32)   0)                   /*   32 <GRP>24<MSG BODY>16<ID>0*/
#define SM_MSG_ID_MSK           ((INT32)   0X0000FFFF)          /*   32 <GRP>24<MSG BODY>16<ID>0*/


#define SM_MSG_GRP(msg)     (((INT32)msg>>SM_MSG_GRP_OFFSET)&SM_MSG_GRP_MSK)
#define SM_MSG_BDY(msg)     (((INT32)msg>>SM_MSG_BDY_OFFSET)&SM_MSG_BDY_MSK)
#define SM_MSG_ID(msg)   (((INT32)msg>>SM_MSG_ID_OFFSET)&SM_MSG_ID_MSK)

#define SM_MAKE_MSG(grp,bdy,id)  (((((INT32)grp)&SM_MSG_GRP_MSK)<<SM_MSG_GRP_OFFSET)|((((INT32)bdy)&SM_MSG_BDY_MSK)<<SM_MSG_BDY_OFFSET)|((((INT32)id)&SM_MSG_ID_MSK)<<SM_MSG_ID_OFFSET))

typedef enum
{
    SM_REQUEST_GRP=0,
    SM_PERMIT_GRP,
    SM_FORBID_GRP,
    SM_INFORM_GRP,
    SM_UPDATE_GRP,// TODO: broadcast something
}SM_MSG_GRP_E;

typedef enum
{
    // ------PB GRP----------//
    SM_BODY_STOP = 0,
    SM_BODY_PLAY,
    SM_BODY_PAUSE,
    SM_BODY_RESUME,
    SM_BODY_PREV,
    SM_BODY_NEXT,
    SM_BODY_SOURCE,

    //-------BT GRP -----------//
    SM_BODY_BT_RECON,
    SM_BODY_BT_DISCON,
    SM_BODY_BT_PAIRING,

    //-------UPG GRP -----------//
    SM_BODY_UPG_START,
    SM_BODY_UPG_STOP,

    //-------UI GRP -----------//
    SM_BODY_UI_POWER,
    SM_BODY_UI_BT,
    SM_BODY_UI_MUTE,//add by yuyun for yocto
    SM_BODY_UI_LONG_MICMUTE,//add by yuyun for yocto
    SM_BODY_UI_LONG_POWER,
    SM_BODY_UI_LONG_SOURCE,
    SM_BODY_UI_LONG_MUTE,
    SM_BODY_UI_LONG_VOLUMEUP,
    SM_BODY_UI_LONG_VOLUMEDOWN,

    //-------IR GRP --------//
    SM_BODY_IR_POWER,
    SM_BODY_IR_SOURCE,
    SM_BODY_IR_BT,
    SM_BODY_IR_REQUEST,
    SM_BODY_IR_MICMUTE,
    SM_BODY_IR_PLAYPAUSE,
    SM_BODY_IR_PRE,
    SM_BODY_IR_NEXT,
    SM_BODY_IR_SURROUND,
    SM_BODY_IR_LINE,
    SM_BODY_IR_USB,
    //-------END -------------//
}SM_MSG_BDY_E;

typedef enum
{
    SM_BT_PARING_SUCESS = 0,
    SM_BT_PARING_FAIL,
    SM_BT_CONNECTED,
    SM_BT_DISCONNECTED,
    SM_BT_FINISH_PARING,
}SM_MSG_BT_E;

typedef enum
{
    SOURCE_NONE = 0,
    SOURCE_URI,
    SOURCE_BT,
    SOURCE_TTS,
    SOURCE_PROMPT,
    SOURCE_BT_HFP,
    SOURCE_DLNA,
    SOURCE_MAX
}SM_PLAYBACK_SOURCE_E;

typedef struct
{
    BOOL speech_enable;
} SM_PARAM_T;

typedef struct _SOURCE_INFO_T
{
    SM_PLAYBACK_SOURCE_E e_source;
    char sz_thread_name[APP_NAME_MAX_LEN + 1];
} SOURCE_INFO_T;

typedef enum 
{
	SM_BT_HFP_ANSWER_CALL = 5,
    SM_BT_HFP_HANGUP_CALL,
	SM_TYPE_MAX,
}SM_BT_HFP_TYPE_E;

extern BOOL u_sm_is_current_playing(VOID);
extern BOOL u_sm_is_network_connect(VOID);
extern BOOL u_sm_is_speech_start(VOID);
extern BOOL u_sm_is_enter_fake_standby(VOID);
extern VOID u_sm_set_fake_standby(BOOL b_enter_fake_standby);
extern VOID u_sm_set_suspend_standby(BOOL b_enter_normal_standby);
extern BOOL u_sm_is_enter_suspend_standby(VOID);
extern SM_PLAYBACK_SOURCE_E u_sm_get_current_source(VOID);
extern INT32 u_sm_get_current_playback_status(VOID);
extern C4aStub_CastState u_sm_get_cast_status(VOID);
extern BOOL u_sm_get_tts_mix_flag(VOID);
extern VOID u_sm_set_tts_mix_flag(BOOL flag);
extern VOID u_sm_send_stop_to_all_source(VOID);
#endif
