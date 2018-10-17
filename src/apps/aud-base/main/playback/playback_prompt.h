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


#ifndef _PLAYBACK_PROMPT_H_
#define _PLAYBACK_PROMPT_H_

#include "u_handle.h"
#include "u_dbg.h"
#include "u_alsa_interface.h"
#include "u_ringbuf.h"
#include "u_assistant_stub.h"
#include <alsa/asoundlib.h>

#define DBG_INIT_LEVEL_APP_PLAYBACK_PROMPT (DBG_LEVEL_ALL|DBG_LAYER_APP)

#undef  DBG_LEVEL_MODULE
#define DBG_LEVEL_MODULE playback_prompt_get_dbg_level()

#define PROMPT_TAG            "<prompt>"
#define PROMPT_INFO(fmt, args...) do{DBG_INFO(("%s[%s:%d]:"fmt, PROMPT_TAG, __FUNCTION__, __LINE__, ##args));}while(0)
#define PROMPT_ERR(fmt, args...)  do{DBG_ERROR(("%s[%s:%d]:"fmt, PROMPT_TAG, __FUNCTION__, __LINE__, ##args));}while(0)
#define PROMPT_MSG(fmt, args...)  do{DBG_ERROR(("%s[%s:%d]:"fmt, PROMPT_TAG, __FUNCTION__, __LINE__, ##args));}while(0)

#define PROMPT_DEVICE_NAME              "sub1"
#define PLAYBACK_PROMPT_SAMPLERATE      44100
#define PLAYBACK_PROMPT_CHANNLE         2
#define PLAYBACK_PROMPT_BITWIDTH        16

typedef enum
{
    PROMPT_PLAY_NORMAL = 0,
    PROMPT_HTTP_ERR_403,
    PROMPT_HTTP_ERR_404,
    PROMPT_HTTP_TIME_OUT,
    PROMPT_FILE_NOT_EXIST,
    PROMPT_FILE_NOT_SUPPORT,
    PROMPT_ERROR_UNKOWN,
} PLAYBACK_PROMPT_PLAY_RESULT_E;

typedef enum
{
    PROMPT_STATUS_STOP = 0,
    PROMPT_STATUS_PLAY,
    PROMPT_STATUS_PAUSE
} PLAYBACK_PROMPT_STATUS_E;

typedef struct _PLAYBACK_PROMPT_PLAY_MSG_T
{
    BOOL                                b_next_prompt;
    UINT16                              ui2_id;
    PLAYBACK_PROMPT_PLAY_RESULT_E       e_play_result;
    ASSISTANT_STUB_PLAY_VOICE_PROMPT_T        t_voice_prompt;
    ASSISTANT_STUB_PLAY_VOICE_PROMPT_T        t_next_prompt;
} PLAYBACK_PROMPT_PLAY_MSG_T;

/* application structure */
typedef struct _PLAYBACK_PROMPT_OBJ_T
{
    HANDLE_T                    h_app;
    BOOL                        b_app_init_ok;
    BOOL                        b_prepare;
    BOOL                        b_pause;
    BOOL                        b_running;
    INT32                       i4_volume;
    VOID*                       h_player;
    PLAYBACK_PROMPT_PLAY_MSG_T  t_play_msg;
} PLAYBACK_PROMPT_OBJ_T;

extern VOID playback_prompt_process_sm_play(unsigned char *pt_msg, UINT32 ui2_id);

#endif
