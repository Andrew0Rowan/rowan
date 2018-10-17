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


#ifdef __cplusplus
extern "C"
{
#endif

/* public */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dlfcn.h>
#include <pthread.h>
#include <sys/syscall.h>
/* application level */
#include "u_am.h"
#include "u_amb.h"
#include "u_common.h"
#include "u_app_thread.h"
#include "u_app_def.h"
#include "u_cli.h"
#include "u_os.h"
#include "u_timerd.h"
#include "u_assert.h"
#include "u_acfg.h"
#include "u_sm.h"
#include "u_dm.h"
#include "u_playback_uri.h"
#include "u_assistant_stub.h"
#include "u_bluetooth_audio.h"
#include "Interface.h"
/* private */

#ifdef __cplusplus
}
#endif
#include "playback_uri.h"
#include "playback_uri_cli.h"

static UINT16 g_ui2_playback_uri_dbg_level = DBG_INIT_LEVEL_APP_PLAYBACK_URI;
PlaybackUri *UriPlay = NULL;

#ifdef __cplusplus
extern "C"
{
#endif

extern "C"
{
#include <malloc.h>
}

UINT16 playback_uri_get_dbg_level(VOID) {
    return (g_ui2_playback_uri_dbg_level | DBG_LAYER_APP);
}

VOID playback_uri_set_dbg_level(UINT16 ui2_db_level) {
    g_ui2_playback_uri_dbg_level = ui2_db_level;
}

VOID u_playback_uri_get_player_status(ASSISTANT_STUB_PLAYER_STATUS_CHANGE_T *pt_status) {
    if (!UriPlay) {
        URI_ERR("playback uri not init!\n");
        return;
    }
    UriPlay->get_player_status(pt_status);
}

BOOL u_playback_uri_get_running_flag(VOID)
{
    if (!UriPlay) {
        URI_ERR("playback uri not init!\n");
        return false;
    }
    return UriPlay->get_running_flag();
}

INT32 _playback_uri_init(const CHAR*    ps_name,
                            HANDLE_T        h_app) {
    INT32 ret;

    if (UriPlay) {
        return AEER_OK;
    }

    UriPlay = new PlaybackUri(h_app);
    if (!UriPlay) {
        URI_ERR("new PlaybackUri failed!\n");
        return AEER_FAIL;
    }

#ifdef CLI_SUPPORT
    ret = playback_uri_cli_attach_cmd_tbl();
    if ((ret != CLIR_NOT_INIT) && (ret != CLIR_OK)) {
        URI_ERR("Err: playback_uri_cli_attach_cmd_tbl() failed, ret=%ld\r\n", ret);
        return AEER_FAIL;
    }
#endif/* CLI_SUPPORT */

    return AEER_OK;
}

INT32 _playback_uri_exit(HANDLE_T       mAppHandle,
                            APP_EXIT_MODE_T e_exit_mode) {
    if (!UriPlay) {
        URI_ERR("playback uri not init!\n");
        return AEER_FAIL;
    }

    delete UriPlay;
    UriPlay = NULL;
    return AEER_OK;
}

INT32 _playback_uri_process_msg(HANDLE_T    mAppHandle,
                                        UINT32      ui4_type,
                                        const VOID* pv_msg,
                                        SIZE_T      z_msg_len,
                                        BOOL        mPaused) {
    APPMSG_T *appMsg = (APPMSG_T *)pv_msg;

    if (!UriPlay) {
        return AEER_FAIL;
    }

    if (!pv_msg) {
        return AEER_FAIL;
    }

    if (ui4_type < AMB_BROADCAST_OFFSET) {
        switch(ui4_type) {
            case E_APP_MSG_TYPE_URI:
                UriPlay->process_uri_msg(appMsg);
                break;
            case E_APP_MSG_TYPE_ASSISTANT_STUB:
                UriPlay->process_assistant_msg(appMsg);
                break;
            case E_APP_MSG_TYPE_STATE_MNGR:
                UriPlay->process_sm_msg(appMsg);
                break;
            default:
                break;
        }
    }
    return AEER_OK;
}

VOID a_playback_uri_register(AMB_REGISTER_INFO_T* pt_reg) {
    memset(pt_reg->s_name, 0, sizeof(CHAR)*(APP_NAME_MAX_LEN + 1));
    strncpy(pt_reg->s_name, PB_URI_THREAD_NAME, APP_NAME_MAX_LEN);

    pt_reg->t_fct_tbl.pf_init                   = _playback_uri_init;
    pt_reg->t_fct_tbl.pf_exit                   = _playback_uri_exit;
    pt_reg->t_fct_tbl.pf_process_msg            = _playback_uri_process_msg;
    pt_reg->t_desc.ui8_flags                    = ~((UINT64)0);
    pt_reg->t_desc.t_thread_desc.z_stack_size   = PB_URI_STACK_SIZE;
    pt_reg->t_desc.t_thread_desc.ui1_priority   = PB_URI_THREAD_PRIORITY;
    pt_reg->t_desc.t_thread_desc.ui2_num_msgs   = PB_URI_NUM_MSGS;
    pt_reg->t_desc.ui2_msg_count                = PB_URI_MSGS_COUNT;
    pt_reg->t_desc.ui2_max_msg_size             = PB_URI_MAX_MSGS_SIZE;
}

#ifdef __cplusplus
}
#endif

void player_msg_callback(int msg, int id, int ext1, int ext2) {
    if (UriPlay) {
        UriPlay->player_msg_process(msg, id, ext1, ext2);
    }
}

PlaybackUri::PlaybackUri(HANDLE_T h_app) {
    AudioFormat outputFormat = {0};

    mAppHandle = h_app;

    mPlayer = player_init();
    if (!mPlayer) {
        URI_ERR("player init failed!\n");
        return;
    }

    outputFormat.mChannelLayout = AV_CH_LAYOUT_STEREO;
    outputFormat.mSampleFmt = AV_SAMPLE_FMT_S16;
    outputFormat.mSampleRate = URI_SAMPLERATE;
    outputFormat.mChannels = URI_CHANNLE;
    memcpy(outputFormat.mDeviceName, URI_DEVICE_NAME, strlen(URI_DEVICE_NAME));
    setPlaybackSettings(mPlayer, &outputFormat);

    registerCallback(mPlayer, player_msg_callback);
}

PlaybackUri::~PlaybackUri() {
    player_deinit(mPlayer);
}

INT32 PlaybackUri::send_msg_to_assistant_stub(ASSISTANT_STUB_CMD_INDEX_E e_cmd, VOID *pt_send_msg, SIZE_T t_size) {
    INT32 ret = PB_URI_FAIL;
    HANDLE_T mAppHandle = NULL_HANDLE;

    u_am_get_app_handle_from_name(&mAppHandle,ASSISTANT_STUB_THREAD_NAME);

    ret = u_app_send_appmsg(mAppHandle,
                            E_APP_MSG_TYPE_ASSISTANT_STUB,
                            MSG_FROM_URI,
                            e_cmd,
                            pt_send_msg,
                            t_size);
    if (AEER_OK != ret) {
        URI_ERR("send_play_request_to_sm failed, ret[%ld]\n", ret);
    }

    return (ret == AEER_OK ? PB_URI_OK : PB_URI_FAIL);
}

INT32 PlaybackUri::send_status_change_to_assistant_stub(VOID) {
    ASSISTANT_STUB_PLAYER_STATUS_CHANGE_T player_status_change = {0};
    u_playback_uri_get_player_status(&player_status_change);
    return send_msg_to_assistant_stub(ASSISTANT_STUB_CMD_PLAYER_STATUS_CHANGE, &player_status_change, sizeof(player_status_change));
}

VOID PlaybackUri::send_player_status_to_bluetooth(VOID)
{
    ASSISTANT_STUB_PLAYER_STATUS_CHANGE_T player_status_change = {0};
#if (CONFIG_SUPPORT_BT_APP && CONFIG_APP_SUPPORT_BT_SRC_MODE)
    BT_SRC_PLAYER_STATUS_T media_info = {0};
#endif /*CONFIG_SUPPORT_BT_APP && CONFIG_APP_SUPPORT_BT_SRC_MODE*/
    int i4_duration = 0;  //song_length
    u_playback_uri_get_player_status(&player_status_change);
    get_current_duration(&i4_duration); //ms

    URI_INFO("player_status_change.player.audioAlbum:%s\n",player_status_change.player.audioAlbum);
    URI_INFO("player_status_change.player.audioAnchor:%s\n",player_status_change.player.audioAnchor);
    URI_INFO("player_status_change.player.audioName:%s\n",player_status_change.player.audioName);
    URI_INFO("player_status_change.player.audioSource:%s\n",player_status_change.player.audioSource);
    URI_INFO("player_status_change.player.source:%s\n",player_status_change.player.source);
    URI_INFO("player_status_change.player.progress:%d\n",player_status_change.player.progress);
    URI_INFO("song_length:%d ms, or %d:%d mins\n",i4_duration, i4_duration/60000, i4_duration/1000%60);

#if (CONFIG_SUPPORT_BT_APP && CONFIG_APP_SUPPORT_BT_SRC_MODE)
    strncpy(media_info.audioAlbum, player_status_change.player.audioAlbum, ASSISTANT_STUB_AUDIO_ALBUM_MAX_LENGTH);
    strncpy(media_info.audioAnchor, player_status_change.player.audioAnchor, ASSISTANT_STUB_AUDIO_ANCHOR_MAX_LENGTH);
    strncpy(media_info.audioName, player_status_change.player.audioName, ASSISTANT_STUB_AUDIO_NAME_MAX_LENGTH);
    strncpy(media_info.audioSource, player_status_change.player.audioSource, ASSISTANT_STUB_AUDIO_SOURCE_MAX_LENGTH);
    strncpy(media_info.source, player_status_change.player.source, ASSISTANT_STUB_SOURCE_MAX_LENGTH);
    media_info.progress = player_status_change.player.progress;
	media_info.song_length = i4_duration;  //ms

    u_bluetooth_src_request_mediaInfo_from_url(&media_info);  
#endif /*CONFIG_SUPPORT_BT_APP && CONFIG_APP_SUPPORT_BT_SRC_MODE*/
}

VOID PlaybackUri::upload_finish_state_to_assistant_stub(VOID) {
    ASSISTANT_STUB_PLAY_DONE_T t_play_msg;

    memset(&t_play_msg, 0, sizeof(t_play_msg));
    strncpy(t_play_msg.command, "/playback/play_done", ASSISTANT_STUB_COMMAND_MAX_LENGTH);

    strncpy(t_play_msg.uri, mCurrentPlay.uri, ASSISTANT_STUB_URI_MAX_LENGTH);

    if (URI_PLAY_NORMAL == mResult)
    {
        t_play_msg.status = 0;
    }
    else
    {
        t_play_msg.status = 1;
        // TODO: error code
    }

    send_msg_to_assistant_stub(ASSISTANT_STUB_CMD_PLAY_DONE, &t_play_msg, sizeof(t_play_msg));
}

INT32 PlaybackUri::send_status_inform_to_sm(SM_MSG_BDY_E PB_BODY) {
    INT32 ret = PB_URI_FAIL;
    HANDLE_T mAppHandle = NULL_HANDLE;
    SM_PARAM_T t_sm_param = {0};

    u_am_get_app_handle_from_name(&mAppHandle,SM_THREAD_NAME);

    ret = u_app_send_appmsg(mAppHandle,
                            E_APP_MSG_TYPE_STATE_MNGR,
                            MSG_FROM_URI,
                            SM_MAKE_MSG(SM_INFORM_GRP,PB_BODY,mUriId),
                            &t_sm_param,
                            sizeof(t_sm_param));
    if (AEER_OK != ret) {
        URI_ERR("send_status_inform_to_sm failed, ret[%ld]\n",ret);
    }
    URI_INFO("playback send %d status to sm!\n", PB_BODY);

    return (ret == AEER_OK ? PB_URI_OK : PB_URI_FAIL);
}

INT32 PlaybackUri::send_status_inform_to_bt(PB_MSG_BDY_E PB_BODY) {
    INT32 ret = PB_URI_FAIL;
    HANDLE_T mAppHandle = NULL_HANDLE;
    SM_PARAM_T t_sm_param = {0};

    u_am_get_app_handle_from_name(&mAppHandle,BLUETOOTH_THREAD_NAME);

    ret = u_app_send_appmsg(mAppHandle,
                            E_APP_MSG_TYPE_URI,
                            MSG_FROM_URI,
                            PB_BODY,
                            &t_sm_param,
                            sizeof(t_sm_param));
    if (AEER_OK != ret) {
        URI_ERR("send_status_inform_to_bt failed, ret[%ld]\n",ret);
    }
    URI_INFO("playback send %d status to bt!\n", PB_BODY);

    return (ret == AEER_OK ? PB_URI_OK : PB_URI_FAIL);
}

INT32 PlaybackUri::send_play_request_to_sm(void) {
    INT32 ret = PB_URI_FAIL;
    HANDLE_T mAppHandle = NULL_HANDLE;

    u_am_get_app_handle_from_name(&mAppHandle,SM_THREAD_NAME);

    ret = u_app_send_appmsg(mAppHandle,
                            E_APP_MSG_TYPE_STATE_MNGR,
                            MSG_FROM_URI,
                            SM_MAKE_MSG(SM_REQUEST_GRP,SM_BODY_PLAY,0),
                            NULL,
                            0);
    if (AEER_OK != ret) {
        URI_ERR("send_play_request_to_sm failed, ret[%ld]\n", ret);
    }

    return (ret == AEER_OK ? PB_URI_OK : PB_URI_FAIL);
}

VOID PlaybackUri::process_start_request(VOID) {
    INT32 ret;

    URI_INFO("process_start_request start\n");

    ret = setDataSource_url(mPlayer, mCurrentPlay.uri, mUriId);
    URI_ASSERT(0 != ret, return, "<player>setDataSource_url failed, ret:[%d]!\n", ret);

    ret = prepareAsync(mPlayer);
    URI_ASSERT(0 != ret, return, "<player>prepare failed, ret:[%d]!\n", ret);

    set_prepare_flag(TRUE);

    uri_play_set_flag();
    URI_MSG(" process_start_request end!\n");
}

VOID PlaybackUri::send_uri_start_msg(UINT32 ui4_id) {
    send_uri_msg_to_self(URI_START, ui4_id);
}

VOID PlaybackUri::send_uri_pause_msg(UINT32 ui4_id) {
    send_uri_msg_to_self(URI_PAUSE, ui4_id);
}

VOID PlaybackUri::send_uri_resume_msg(UINT32 ui4_id) {
    send_uri_msg_to_self(URI_RESUME, ui4_id);
}

VOID PlaybackUri::send_uri_stop_msg(UINT32 ui4_id) {
    send_uri_msg_to_self(URI_STOP, ui4_id);
}

VOID PlaybackUri::process_start(UINT16 ui2_id) {
    if (ui2_id != mUriId) {
        URI_INFO("old id:[%d], new id:[%d]\n", ui2_id, mUriId);
        return;
    }

    start(UriPlay->mPlayer);
    send_status_change_to_assistant_stub();
    send_status_inform_to_sm(SM_BODY_PLAY);
    #if CONFIG_APP_SUPPORT_BT_SRC_MODE
    send_status_inform_to_bt(PB_BODY_PLAY);
    send_player_status_to_bluetooth();
    #endif
}

VOID PlaybackUri::process_pause(UINT16 ui2_id) {
    if (ui2_id != mUriId) {
        URI_INFO("old id:[%d], new id:[%d]\n", ui2_id, mUriId);
        return;
    }

    if (!get_running_flag()) {
        URI_MSG("player not running!\n");
        return;
    }

    if (get_pause_flag()) {
        URI_MSG("player is pause!\n");
        return;
    }

    INT32 ret = pause_l(mPlayer);
    URI_ASSERT(0 != ret, return, "<player>pause failed, ret:[%d]!\n", ret);
    set_pause_flag(TRUE);
    send_status_change_to_assistant_stub();
    send_status_inform_to_sm(SM_BODY_PAUSE);
    send_status_inform_to_bt(PB_BODY_PAUSE);
}

VOID PlaybackUri::process_resume(UINT16 ui2_id) {
    if (ui2_id != mUriId) {
        URI_INFO("old id:[%d], new id:[%d]\n", ui2_id, mUriId);
        return;
    }

    if (!get_running_flag()) {
        URI_MSG("player not running!\n");
        return;
    }

    if (false == get_pause_flag()) {
        URI_MSG("player is playing!\n");
        return;
    }

    INT32 ret = start(mPlayer);
    URI_ASSERT(0 != ret, return, "<player>start failed, ret:[%d]!\n", ret);
    set_pause_flag(FALSE);
    send_status_change_to_assistant_stub();
    send_status_inform_to_sm(SM_BODY_PLAY);
    send_status_inform_to_bt(PB_BODY_PLAY);
}

VOID PlaybackUri::process_stop(UINT16 ui2_id) {
    if (ui2_id != mUriId) {
        URI_INFO("old id:[%d], new id:[%d]\n", ui2_id, mUriId);
        return;
    }

    URI_INFO("process_stop start\n");
    URI_INFO("switch flag:[%d] prepare flag:[%d]\n", mSwitch, get_prepare_flag());

    uri_stop_set_flag();
    send_status_change_to_assistant_stub();
    INT32 ret = reset(mPlayer);
    URI_ASSERT(0 != ret, return, "<player>stop failed, ret:[%d]!\n", ret);

    malloc_trim(0);

    if (mSwitch) {
        mSwitch = FALSE;
        process_start_request();
    }
    else {
        send_status_inform_to_sm(SM_BODY_STOP);
        send_status_inform_to_bt(PB_BODY_STOP);
    }

}

INT32 PlaybackUri::process_uri_msg(APPMSG_T* pv_msg) {
    PB_URI_MSG_T *uriMsg = (PB_URI_MSG_T *)pv_msg;
    UINT16 ui2_id = (UINT16)uriMsg->ui4_data1;
    URI_INFO("uri msg id [%d]\n", uriMsg->ui4_msg_id);

    switch (uriMsg->ui4_msg_id) {
        case URI_START:
            process_start(ui2_id);
            break;
        case URI_PAUSE:
            process_pause(ui2_id);
            break;
        case URI_RESUME:
            process_resume(ui2_id);
            break;
        case URI_STOP:
            process_stop(ui2_id);
            break;
        default:
            break;
    }
    return AEER_OK;
}

VOID PlaybackUri::set_seek(int i4_seek_ms) {
    seekTo(mPlayer, i4_seek_ms*MS_TO_US);
}

VOID PlaybackUri::change_to_next_uri(INT8 *pt_msg) {
    memcpy(&mOriPlay, &mCurrentPlay, sizeof(mOriPlay));
    memcpy(&mCurrentPlay, pt_msg, sizeof(mCurrentPlay));
    process_stop(mUriId);
}

VOID PlaybackUri::set_new_uri(INT8 *pt_msg) {
    memcpy(&mCurrentPlay, pt_msg, sizeof(mCurrentPlay));
    mSwitch = FALSE;
    process_start_request();
}

VOID PlaybackUri::set_current_play(INT8 *pt_msg, UINT32 uriId) {
    mUriId = uriId;
    mSwitch = TRUE;

    if (get_running_flag()) {
        change_to_next_uri(pt_msg);
    }
    else {
        set_new_uri(pt_msg);
    }
}

VOID PlaybackUri::process_assistant_seek(INT8 *pt_msg) {
    ASSISTANT_STUB_ADJUST_PROGRESS_T *seekMsg = (ASSISTANT_STUB_ADJUST_PROGRESS_T *)pt_msg;
    set_seek(seekMsg->progress);
}

VOID PlaybackUri::process_assistant_msg(APPMSG_T* pv_msg) {
    URI_INFO("uri msg id [%d]\n", pv_msg->ui4_msg_type);
    switch (pv_msg->ui4_msg_type) {
        case ASSISTANT_STUB_CMD_ADJUST_PROGRESS:
            process_assistant_seek(pv_msg->p_usr_msg);
            break;
        default:
            URI_ERR("error msg type[%d] from assistant_stub!\n", pv_msg->ui4_msg_type);
            break;
    }
}

VOID PlaybackUri::set_current_pause(VOID) {
    if (get_running_flag() && !get_pause_flag()) {
        if (get_prepare_flag()) {
            URI_INFO("send pause when preparing!\n");
            set_pause_flag(TRUE);
        }
        else {
            process_pause(mUriId);
        }
    }
}

VOID PlaybackUri::set_current_resume(VOID) {
    if (get_running_flag() && get_pause_flag()) {
        if (get_prepare_flag()) {
            URI_INFO("send resume when preparing!\n");
            set_pause_flag(FALSE);
        }
        else {
            process_resume(mUriId);
        }
    }
}

VOID PlaybackUri::process_request_permit_msg(APPMSG_T* pv_msg) {
    URI_INFO("sm msg body:%d\n", SM_MSG_BDY(pv_msg->ui4_msg_type));
    switch(SM_MSG_BDY(pv_msg->ui4_msg_type)) {
        case SM_BODY_PLAY:
            set_current_play(pv_msg->p_usr_msg, SM_MSG_ID(pv_msg->ui4_msg_type));
            break;
        case SM_BODY_PAUSE:
            set_current_pause();
            break;
        case SM_BODY_RESUME:
            set_current_resume();
            break;
        case SM_BODY_STOP:
            mSwitch = FALSE;
            process_stop(mUriId);
            break;
        default:
            break;
    }
}

INT32 PlaybackUri::process_sm_msg(APPMSG_T* pv_msg) {
    UINT32 msgType = pv_msg->ui4_msg_type;
    URI_INFO("sm msg type:%d\n", SM_MSG_GRP(msgType));
    switch(SM_MSG_GRP(msgType)) {
        case SM_REQUEST_GRP:
        case SM_PERMIT_GRP:
            process_request_permit_msg(pv_msg);
            break;
        case SM_FORBID_GRP:
            URI_ERR("forbid msg body[%d]\n", SM_MSG_BDY(msgType));
            break;
        default:
            break;
    }
    return AEER_OK;
}

void PlaybackUri::player_msg_process(int msg, int id, int ext1, int ext2) {
    URI_MSG("<player>callback msg:[%d] id:[%d] ext1:[%d] ext2:[%d]\n", msg, id, ext1, ext2);
    if (id != mUriId) {
        URI_MSG("<player>callback notify old id:[%d] new id:[%d]!\n", id, mUriId);
        return;
    }
    switch (msg) {
        case MEDIA_PLAYBACK_COMPLETE:
            send_uri_stop_msg((UINT32)id);
            upload_finish_state_to_assistant_stub();
            break;
        case MEDIA_ERROR:
            send_uri_stop_msg((UINT32)id);
            mResult = URI_ERROR_UNKOWN;
            upload_finish_state_to_assistant_stub();
            mResult = URI_PLAY_NORMAL;
            break;
        case MEDIA_PREPARED:
            set_prepare_flag(FALSE);
            set_seek(mCurrentPlay.progress);
            if (get_pause_flag()) {
                URI_INFO("prepare->pause!\n");
                send_status_change_to_assistant_stub();
                send_status_inform_to_sm(SM_BODY_PAUSE);
                send_status_inform_to_bt(PB_BODY_PAUSE);
            } else {
                URI_INFO("prepare->start!\n");
                send_uri_start_msg((UINT32)id);
            }
            break;
        default:
            break;
    }
}

VOID PlaybackUri::send_uri_msg_to_self(UINT32 ui4_msg_id, UINT32 ui4_id) {
    PB_URI_MSG_T uriMsg;
    memset(&uriMsg, 0x00, sizeof(PB_URI_MSG_T));
    uriMsg.ui4_msg_id = ui4_msg_id;
    uriMsg.ui4_data1 = ui4_id;
    send_msg_to_self(&uriMsg);
}

INT32 PlaybackUri::send_msg_to_self(const PB_URI_MSG_T* pt_event) {
    INT32 ret;

    if (!UriPlay) {
        return PB_URI_FAIL;
    }

    ret = u_app_send_msg(mAppHandle,
                        E_APP_MSG_TYPE_URI,
                        pt_event,
                        sizeof(PB_URI_MSG_T),
                        NULL,
                        NULL);
    URI_ASSERT(AEER_OK != ret, , "_app_send_msg sent E_APP_MSG_TYPE_URI failed, ret[%ld]\n",ret);
    return (ret == AEER_OK ? PB_URI_OK : PB_URI_FAIL);
}
VOID PlaybackUri::get_current_progress(INT32 *pi4_ms) {
    if (NULL == pi4_ms) {
        return;
    }

    if (get_running_flag()) {
        off64_t us;
        getCurrentPosition(mPlayer, &us);
        *pi4_ms = us / 1000;
    } else {
        *pi4_ms = 0;
    }
}

VOID PlaybackUri::get_current_duration(INT32 *pi4_ms) {
    if (NULL == pi4_ms) {
            return;
        }

    if (get_running_flag()) {
        off64_t us;
        getDuration(mPlayer, &us);
        *pi4_ms = us / 1000;
    } else {
        *pi4_ms = 0;
    }
}

VOID PlaybackUri::get_player_info(URI_STATUS_E e_status,
                                     ASSISTANT_STUB_PLAYER_STATUS_CHANGE_T *pt_status,
                                     ASSISTANT_STUB_PLAY_T *pt_play) {
    BOOL b_mute;

    if (NULL == pt_status) {
        URI_ERR("pt_status can't be NULL!\n");
        return;
    }

    memcpy(pt_status->command, STRING_PLAYER_STATUS_CHANGE, ASSISTANT_STUB_COMMAND_MAX_LENGTH);

    //volume
    if (!u_acfg_get_mute(&b_mute) && b_mute) {
        pt_status->player.volume = 0;
    }
    else {
        u_acfg_get_volume((UINT8*)&pt_status->player.volume);
    }

    //status
    switch(e_status) {
        case SM_BODY_STOP:
            memcpy(pt_status->player.status, "stop", ASSISTANT_STUB_STATUS_MAX_LENGTH);
            break;
        case SM_BODY_PLAY:
            memcpy(pt_status->player.status, "play", ASSISTANT_STUB_STATUS_MAX_LENGTH);
            break;
        case SM_BODY_PAUSE:
            memcpy(pt_status->player.status, "pause", ASSISTANT_STUB_STATUS_MAX_LENGTH);
            break;
        default:
            break;
    }

    //source
    memcpy(pt_status->player.source, "cloud", ASSISTANT_STUB_SOURCE_MAX_LENGTH);
    //audioId
    memcpy(pt_status->player.audioId, pt_play->audioId, ASSISTANT_STUB_AUDIO_ID_MAX_LENGTH);
    //audioUid
    memcpy(pt_status->player.audioUid, pt_play->audioUid, ASSISTANT_STUB_AUDIO_UID_MAX_LENGTH);
    //audioSource
    memcpy(pt_status->player.audioSource, pt_play->audioSource, ASSISTANT_STUB_AUDIO_SOURCE_MAX_LENGTH);
    //audioName
    memcpy(pt_status->player.audioName, pt_play->audioName, ASSISTANT_STUB_AUDIO_NAME_MAX_LENGTH);
    //audioAnchor
    memcpy(pt_status->player.audioAnchor, pt_play->audioAnchor, ASSISTANT_STUB_AUDIO_ANCHOR_MAX_LENGTH);
    //audioAlbum
    memcpy(pt_status->player.audioAlbum, pt_play->audioAlbum, ASSISTANT_STUB_AUDIO_ALBUM_MAX_LENGTH);
    //progress
    get_current_progress(&(pt_status->player.progress));
    //audioExt
    memcpy(pt_status->player.audioExt, pt_play->audioExt, ASSISTANT_STUB_AUDIO_EXT_MAX_LENGTH);
}

VOID PlaybackUri::get_player_status(ASSISTANT_STUB_PLAYER_STATUS_CHANGE_T *pt_status) {
    URI_STATUS_E e_status = URI_STATUS_STOP;
    ASSISTANT_STUB_PLAY_T *pt_play = NULL;

    if (get_running_flag()) {
        e_status = URI_STATUS_PLAY;
        if (get_pause_flag()) {
            e_status = URI_STATUS_PAUSE;
        }
    }
    if (mSwitch) {
        pt_play = &mOriPlay;
    }
    else {
        pt_play = &mCurrentPlay;
    }
    get_player_info(e_status, pt_status, pt_play);
}


