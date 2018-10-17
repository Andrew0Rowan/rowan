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


#ifndef __PLAYBACK_URI_H__
#define __PLAYBACK_URI_H__

#ifdef __cplusplus
extern "C"
{
#endif

#include "u_handle.h"
#include "u_dbg.h"
#include "u_alsa_interface.h"
#include "u_ringbuf.h"
#include "u_assistant_stub.h"
#include "u_sm.h"
#include "u_bluetooth_audio.h"
#include <alsa/asoundlib.h>

#ifdef __cplusplus
}
#endif

#define DBG_INIT_LEVEL_APP_PLAYBACK_URI (DBG_LEVEL_ALL|DBG_LAYER_APP)

#undef  DBG_LEVEL_MODULE
#define DBG_LEVEL_MODULE playback_uri_get_dbg_level()

#define URI_TAG "<uri>"
#define URI_INFO(fmt, args...) do{DBG_INFO(("%s[%s:%d]:" fmt, URI_TAG, __FUNCTION__, __LINE__, ##args));}while(0)
#define URI_ERR(fmt, args...)  do{DBG_ERROR(("%s[%s:%d]:" fmt, URI_TAG, __FUNCTION__, __LINE__, ##args));}while(0)
#define URI_MSG(fmt, args...)  do{DBG_ERROR(("%s[%s:%d]:" fmt, URI_TAG, __FUNCTION__, __LINE__, ##args));}while(0)

#define URI_ASSERT(cond, action, ...)       \
    do {                                    \
        if (cond)                           \
        {                                   \
            URI_ERR(__VA_ARGS__);           \
            action;                         \
        }                                   \
    }                                       \
    while (0)

#ifdef __cplusplus
extern "C"
{
#endif

UINT16 playback_uri_get_dbg_level(VOID);
VOID playback_uri_set_dbg_level(UINT16 ui2_db_level);
INT32 _playback_uri_init(const CHAR*    ps_name,
                            HANDLE_T        h_app);
INT32 _playback_uri_exit(HANDLE_T       mAppHandle,
                        APP_EXIT_MODE_T e_exit_mode);
INT32 _playback_uri_process_msg (HANDLE_T        mAppHandle,
                                        UINT32          ui4_type,
                                        const VOID*     pv_msg,
                                        SIZE_T          z_msg_len,
                                        BOOL            mPaused);

#ifdef __cplusplus
}
#endif

class PlaybackUri
{
public:
    PlaybackUri(HANDLE_T mAppHandle);
    ~PlaybackUri();
    VOID set_seek(int i4_seek_ms);
    INT32 send_msg_to_self(const PB_URI_MSG_T* pt_event);
    VOID set_current_play(INT8 *pt_msg, UINT32 mUriId);
    VOID get_current_progress(INT32 *pi4_ms);
    VOID get_current_duration(INT32 *pi4_ms);
    friend BOOL u_playback_uri_get_running_flag(VOID);
    friend VOID u_playback_uri_get_player_status(ASSISTANT_STUB_PLAYER_STATUS_CHANGE_T *pt_status);
    VOID player_msg_process(int msg, int id, int ext1, int ext2);
    friend INT32 _playback_uri_process_msg (HANDLE_T        mAppHandle,
                                            UINT32          ui4_type,
                                            const VOID*     pv_msg,
                                            SIZE_T          z_msg_len,
                                            BOOL            mPaused);

private:
    #define US_TO_MS           (1/1000)
    #define MS_TO_US           1000

    #define URI_DEVICE_NAME    "sub0"
    #define URI_SAMPLERATE     44100
    #define URI_CHANNLE        2
    #define URI_BITWIDTH       16

    enum URI_PLAY_RESULT_E {
        URI_PLAY_NORMAL = 0,
        URI_HTTP_ERR_403,
        URI_HTTP_ERR_404,
        URI_HTTP_TIME_OUT,
        URI_FILE_NOT_EXIST,
        URI_FILE_NOT_SUPPORT,
        URI_ERROR_UNKOWN,
    };

    enum URI_STATUS_E {
        URI_STATUS_STOP = 0,
        URI_STATUS_PLAY,
        URI_STATUS_PAUSE
    };

    HANDLE_T            mAppHandle;
    VOID*               mPlayer;
    BOOL                mPrepare;
    BOOL                mSwitch;
    BOOL                mPause;
    BOOL                mRunning;
    UINT16              mUriId;
    ASSISTANT_STUB_PLAY_T     mCurrentPlay;
    ASSISTANT_STUB_PLAY_T     mOriPlay;
    URI_PLAY_RESULT_E   mResult;

    BOOL set_running_flag(BOOL flag) {
        mRunning = flag;
    }

    BOOL get_running_flag(void) {
        return mRunning;
    }

    void set_prepare_flag(BOOL flag) {
        mPrepare = flag;
    }

    BOOL get_prepare_flag(void) {
        return mPrepare;
    }

    void set_pause_flag(BOOL flag) {
        mPause = flag;
    }

    BOOL get_pause_flag(void) {
        return mPause;
    }

    VOID uri_play_set_flag(VOID) {
        set_pause_flag(FALSE);
        set_running_flag(TRUE);
    }

    VOID uri_stop_set_flag(VOID) {
        set_prepare_flag(FALSE);
        set_pause_flag(FALSE);
        set_running_flag(FALSE);
    }

    INT32 process_uri_msg(APPMSG_T* pv_msg);
    INT32 process_sm_msg(APPMSG_T* pv_msg);
    VOID process_assistant_msg(APPMSG_T* pv_msg);
    VOID process_request_permit_msg(APPMSG_T* pv_msg);
    VOID process_start_request(VOID);
    VOID process_start(UINT16 ui2_id);
    VOID process_pause(UINT16 ui2_id);
    VOID process_resume(UINT16 ui2_id);
    VOID process_stop(UINT16 ui2_id);
    VOID send_uri_start_msg(UINT32 ui4_id);
    VOID send_uri_pause_msg(UINT32 ui4_id);
    VOID send_uri_resume_msg(UINT32 ui4_id);
    VOID send_uri_stop_msg(UINT32 ui4_id);
    INT32 send_msg_to_assistant_stub(ASSISTANT_STUB_CMD_INDEX_E e_cmd, VOID *pt_send_msg, SIZE_T t_size);
    VOID upload_finish_state_to_assistant_stub(VOID);
    INT32 send_status_change_to_assistant_stub(VOID);
    VOID send_player_status_to_bluetooth(VOID);
    INT32 send_status_inform_to_sm(SM_MSG_BDY_E PB_BODY);
    INT32 send_status_inform_to_bt(PB_MSG_BDY_E PB_BODY);
    INT32 send_play_request_to_sm(void);
    VOID change_to_next_uri(INT8 *pt_msg);
    VOID set_new_uri(INT8 *pt_msg);
    VOID set_current_pause(VOID);
    VOID set_current_resume(VOID);
    VOID process_assistant_seek(INT8 *pt_msg);
    VOID send_uri_msg_to_self(UINT32 ui4_msg_id, UINT32 ui4_id);
    VOID get_player_info(URI_STATUS_E e_status,
        ASSISTANT_STUB_PLAYER_STATUS_CHANGE_T *pt_status, ASSISTANT_STUB_PLAY_T *pt_play);
    VOID get_player_status(ASSISTANT_STUB_PLAYER_STATUS_CHANGE_T *pt_status);
};

#endif
