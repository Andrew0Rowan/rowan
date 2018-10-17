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


#ifndef _SM_H_
#define _SM_H_
/*-----------------------------------------------------------------------------
                    include files
-----------------------------------------------------------------------------*/
#include "u_common.h"
#include "u_dbg.h"
#include "u_appman.h"
#include "u_sm.h"
#include "u_timerd.h"
#include "u_assistant_stub.h"

#undef   DBG_LEVEL_MODULE
#define  DBG_LEVEL_MODULE       sm_get_dbg_level()

#define SM_ASSERT(err) \
    do{                 \
        if(err<0){      \
            DBG_ERROR(("<SM><ASSERT> %s L%d err:%d\n",__FUNCTION__,__LINE__,err)); \
            while(1);   \
        }               \
    }while(0)

#define SM_TAG "<SM>"
#define SM_INFO(fmt, args...) do{DBG_INFO(("%s[%s:%d]:"fmt, SM_TAG, __FUNCTION__, __LINE__, ##args));}while(0)
#define SM_ERR(fmt, args...)  do{DBG_ERROR(("%s[%s:%d]:"fmt, SM_TAG, __FUNCTION__, __LINE__, ##args));}while(0)
#define SM_MSG(fmt, args...)  do{DBG_ERROR(("%s[%s:%d]:"fmt, SM_TAG, __FUNCTION__, __LINE__, ##args));}while(0)

/*app private  msg*/


//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
/* application structure */
typedef enum
{
    URI_NOT_SUSPEND = 0,
    URI_SUSPEND_BY_SWITCH,
} URI_SUSPEND_MODE_E;

typedef enum
{
    BT_NOT_PERMIT = 0,
    BT_WAIT_PERMIT,
    BT_WAIT_PREV,
    BT_WAIT_NEXT,
} BT_PERMIT_E;

typedef struct
{
    BOOL                    b_wait_permit;
    UINT16                  ui2_id;
    SM_PLAYBACK_SOURCE_E    e_tts_source;
    ASSISTANT_STUB_PLAY_TTS_T     t_tts_play;
} TTS_SOURCE_T;

typedef struct
{
    BOOL                            b_wait_permit;
    UINT16                          ui2_id;
    SM_PLAYBACK_SOURCE_E            e_prompt_source;
    ASSISTANT_STUB_PLAY_VOICE_PROMPT_T    t_prompt_play;
} PROMPT_SOURCE_T;

typedef struct
{
    BOOL                    b_wait_permit;
    UINT16                  ui2_uri_id;
    UINT32                  ui4_status;
    SM_PLAYBACK_SOURCE_E    e_uri_source;
    URI_SUSPEND_MODE_E      e_suspend_mode;
    ASSISTANT_STUB_PLAY_T         t_uri_play;
} URI_SOURCE_T;

typedef struct
{
    BOOL                    b_suspend;
    UINT16                  ui2_bt_id;
    UINT32                  ui4_status;
    BT_PERMIT_E             e_wait_permit;
    SM_PLAYBACK_SOURCE_E    e_bt_source;
} BT_SOURCE_T;

typedef struct
{
    UINT16                  ui2_bt_hfp_id;
    UINT32                  ui4_status;
    BOOL                    b_wait_permit;
    SM_PLAYBACK_SOURCE_E    e_bt_hfp_source;
} BT_HFP_SOURCE_T;

typedef struct
{
    BOOL                    b_suspend;
    UINT16                  ui2_dlna_id;
    UINT32                  ui4_status;
    BOOL                    b_wait_permit;
    SM_PLAYBACK_SOURCE_E    e_dlna_source;
} DLNA_SOURCE_T;

typedef struct _SM_OBJ_T
{
    HANDLE_T                h_app;
    BOOL                    b_app_init;
    BOOL                    b_fake_standby;
    BOOL                    b_suspend_standby;
    BOOL                    b_network_connect;
    UINT32                  ui4_standby_count;
    TIMER_TYPE_T            t_standby_timer;

    BOOL                    b_speech_start;
    PROMPT_SOURCE_T         t_prompt_source;
    TTS_SOURCE_T            t_tts_source;
    URI_SOURCE_T            t_uri_source;
    BT_SOURCE_T             t_bt_source;
    BT_HFP_SOURCE_T         t_bt_hfp_source;
    DLNA_SOURCE_T           t_dlna_source;
}SM_OBJ_T;

/*------------------------------------------------------------------------------
                                            funcitons declarations
------------------------------------------------------------------------------*/
INT32 _sm_send_msg(UINT32 ui4_sender_id, UINT32 ui4_msg_type,const CHAR* ps_name);
extern BOOL _sm_get_ecopwer_flag(VOID);
extern VOID _sm_set_ecopwer_flag(BOOL flag);
#endif /* _HELLOTEST_H_ */
