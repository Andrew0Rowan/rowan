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


/* public */
#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <string.h>
#include <dlfcn.h>
#include <pthread.h>
#include <sys/syscall.h>
/* application level */
#include "u_amb.h"
#include "u_common.h"
#include "u_app_thread.h"
#include "u_cli.h"
#include "u_os.h"
#include "u_timerd.h"
#include "u_assert.h"
#include "u_sm.h"
#include "u_dm.h"
#include "u_playback_prompt.h"
#include "u_assistant_stub.h"
#include "Interface.h"
/* private */
#include "playback_prompt.h"

/*-----------------------------------------------------------------------------
 * structure, constants, macro definitions
 *---------------------------------------------------------------------------*/
#define PROMPT_ASSERT(cond, action, ...)       \
    do {                                    \
        if (cond)                           \
        {                                   \
            PROMPT_ERR(__VA_ARGS__);           \
            action;                         \
        }                                   \
    }                                       \
    while (0)

/*-----------------------------------------------------------------------------
 * variable declarations
 *---------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------
 * static variable declarations
 *---------------------------------------------------------------------------*/
static UINT16 g_ui2_playback_prompt_dbg_level = DBG_INIT_LEVEL_APP_PLAYBACK_PROMPT;
static PLAYBACK_PROMPT_OBJ_T g_t_prompt = {0};
static char g_url[ASSISTANT_STUB_URI_MAX_LENGTH] = "/data/sda1/test3.mp3";
/*-----------------------------------------------------------------------------
 * private methods declarations
 *---------------------------------------------------------------------------*/
static VOID _playback_prompt_process_start_play(VOID);
static VOID _playback_prompt_process_start(UINT16 id);
static VOID _playback_prompt_process_pause(UINT16 id);
static VOID _playback_prompt_process_resume(UINT16 id);
static VOID _playback_prompt_process_stop(UINT16 id);

static BOOL _playback_set_prompt_running_flag(BOOL flag)
{
    g_t_prompt.b_running = flag;
}

static BOOL _playback_get_prompt_running_flag(void)
{
    return g_t_prompt.b_running;
}

static void playback_set_prompt_prepare_flag(BOOL flag)
{
    g_t_prompt.b_prepare = flag;
}

static BOOL playback_get_prompt_prepare_flag(void)
{
    return g_t_prompt.b_prepare;
}

static void playback_set_prompt_pause_flag(BOOL flag)
{
    g_t_prompt.b_pause = flag;
}

static BOOL playback_get_prompt_pause_flag(void)
{
    return g_t_prompt.b_pause;
}

static VOID _playback_prompt_play(VOID)
{
    playback_set_prompt_pause_flag(FALSE);
    _playback_set_prompt_running_flag(TRUE);
}

static VOID _playback_prompt_stop(VOID)
{
    playback_set_prompt_pause_flag(FALSE);
    _playback_set_prompt_running_flag(FALSE);
}

UINT16 playback_prompt_get_dbg_level(VOID)
{
    return (g_ui2_playback_prompt_dbg_level | DBG_LAYER_APP);
}

VOID playback_prompt_set_dbg_level(UINT16 ui2_db_level)
{
    g_ui2_playback_prompt_dbg_level = ui2_db_level;
}

static INT32 _playback_prompt_send_msg_to_assistant_stub(ASSISTANT_STUB_CMD_INDEX_E e_cmd, VOID *pt_send_msg, SIZE_T t_size)
{
    INT32 i4_ret = PB_PROMPT_FAIL;
    HANDLE_T h_app = NULL_HANDLE;

    u_am_get_app_handle_from_name(&h_app,ASSISTANT_STUB_THREAD_NAME);

    i4_ret = u_app_send_appmsg(h_app,
            E_APP_MSG_TYPE_ASSISTANT_STUB,
            MSG_FROM_PROMPT,
            e_cmd,
            pt_send_msg,
            t_size);
    if (AEER_OK != i4_ret)
    {
        PROMPT_ERR("_playback_prompt_send_play_request_to_sm failed, i4_ret[%ld]\n", i4_ret);
    }

    return (i4_ret == AEER_OK ? PB_PROMPT_OK : PB_PROMPT_FAIL);
}

static VOID _playback_prompt_upload_finish_state(VOID)
{
    if (g_t_prompt.t_play_msg.t_voice_prompt.feedback)
    {
        ASSISTANT_STUB_PLAY_DONE_T t_play_msg;

        memset(&t_play_msg, 0, sizeof(t_play_msg));
        strncpy(t_play_msg.command, "/playback/play_done", ASSISTANT_STUB_COMMAND_MAX_LENGTH);

        strncpy(t_play_msg.uri, g_t_prompt.t_play_msg.t_voice_prompt.uri, ASSISTANT_STUB_URI_MAX_LENGTH);

        if (PROMPT_PLAY_NORMAL == g_t_prompt.t_play_msg.e_play_result)
        {
            t_play_msg.status = 0;
        }
        else
        {
            // TODO: error code
        }
        // for public branch "add prompt tast"
        //_playback_prompt_send_msg_to_assistant_stub(ASSISTANT_STUB_CMD_PLAY_DONE, &t_play_msg, sizeof(t_play_msg));
    }
}

static INT32 _playback_prompt_inform_playback_status_to_sm(SM_MSG_BDY_E PB_BODY)
{
    INT32 i4_ret = PB_PROMPT_FAIL;
    HANDLE_T h_app = NULL_HANDLE;
    SM_PARAM_T t_sm_param = {0};

    u_am_get_app_handle_from_name(&h_app,SM_THREAD_NAME);

    i4_ret = u_app_send_appmsg(h_app,
                                E_APP_MSG_TYPE_STATE_MNGR,
                                MSG_FROM_PROMPT,
                                SM_MAKE_MSG(SM_INFORM_GRP,PB_BODY,g_t_prompt.t_play_msg.ui2_id),
                                &t_sm_param,
                                sizeof(t_sm_param));
    if (AEER_OK != i4_ret)
    {
        PROMPT_ERR("_playback_prompt_inform_playback_status_to_sm failed, i4_ret[%ld]\n",i4_ret);
    }
    PROMPT_INFO("playback send %d status to sm!\n", PB_BODY);

    return (i4_ret == AEER_OK ? PB_PROMPT_OK : PB_PROMPT_FAIL);
}

static INT32 _playback_prompt_send_play_request_to_sm(void)
{
    INT32 i4_ret = PB_PROMPT_FAIL;
    HANDLE_T h_app = NULL_HANDLE;

    u_am_get_app_handle_from_name(&h_app,SM_THREAD_NAME);

    i4_ret = u_app_send_appmsg(h_app,
                                E_APP_MSG_TYPE_STATE_MNGR,
                                MSG_FROM_PROMPT,
                                SM_MAKE_MSG(SM_REQUEST_GRP,SM_BODY_PLAY,0),
                                NULL,
                                0);
    if (AEER_OK != i4_ret)
    {
        PROMPT_ERR("_playback_prompt_send_play_request_to_sm failed, i4_ret[%ld]\n", i4_ret);
    }

    return (i4_ret == AEER_OK ? PB_PROMPT_OK : PB_PROMPT_FAIL);
}

static VOID _playback_prompt_process_start_play(VOID)
{
    INT32 i4_ret;

    i4_ret = setDataSource_url(g_t_prompt.h_player, g_t_prompt.t_play_msg.t_voice_prompt.uri, g_t_prompt.t_play_msg.ui2_id);
    PROMPT_ASSERT(0 != i4_ret, return, "<player>setDataSource_url failed, ret:[%d]!\n", i4_ret);

    i4_ret = prepareAsync(g_t_prompt.h_player);
    PROMPT_ASSERT(0 != i4_ret, return, "<player>prepare failed, ret:[%d]!\n", i4_ret);

    playback_set_prompt_prepare_flag(TRUE);

    _playback_prompt_play();
}

static VOID _playback_prompt_process_start(UINT16 id)
{
    if ((FALSE == g_t_prompt.t_play_msg.b_next_prompt) && (id != g_t_prompt.t_play_msg.ui2_id))
    {
        PROMPT_INFO("old id:[%d], new id:[%d]\n", id, g_t_prompt.t_play_msg.ui2_id);
        return;
    }
    start(g_t_prompt.h_player);
    _playback_prompt_inform_playback_status_to_sm(SM_BODY_PLAY);
}

static VOID _playback_prompt_process_pause(UINT16 id)
{
    if ((FALSE == g_t_prompt.t_play_msg.b_next_prompt) && (id != g_t_prompt.t_play_msg.ui2_id))
    {
        PROMPT_INFO("old id:[%d], new id:[%d]\n", id, g_t_prompt.t_play_msg.ui2_id);
        return;
    }
    if (!_playback_get_prompt_running_flag())
    {
        PROMPT_INFO("player not running!\n");
        return;
    }
    if (playback_get_prompt_pause_flag())
    {
        PROMPT_INFO("player is pause!\n");
        return;
    }

    INT32 i4_ret = pause_l(g_t_prompt.h_player);
    PROMPT_ASSERT(0 != i4_ret, return, "<player>pause failed, ret:[%d]!\n", i4_ret);

    playback_set_prompt_pause_flag(TRUE);
    _playback_prompt_inform_playback_status_to_sm(SM_BODY_PAUSE);
}

static VOID _playback_prompt_process_resume(UINT16 id)
{
    if ((FALSE == g_t_prompt.t_play_msg.b_next_prompt) && (id != g_t_prompt.t_play_msg.ui2_id))
    {
        PROMPT_INFO("old id:[%d], new id:[%d]\n", id, g_t_prompt.t_play_msg.ui2_id);
        return;
    }
    if (!_playback_get_prompt_running_flag())
    {
        PROMPT_MSG("player not running!\n");
        return;
    }
    if (FALSE == playback_get_prompt_pause_flag())
    {
        PROMPT_INFO("player is playing!\n");
        return;
    }

    INT32 i4_ret = start(g_t_prompt.h_player);
    PROMPT_ASSERT(0 != i4_ret, return, "<player>start failed, ret:[%d]!\n", i4_ret);

    playback_set_prompt_pause_flag(FALSE);
    _playback_prompt_inform_playback_status_to_sm(SM_BODY_PLAY);
}

static VOID _playback_prompt_process_stop(UINT16 id)
{
    if ((FALSE == g_t_prompt.t_play_msg.b_next_prompt) && (id != g_t_prompt.t_play_msg.ui2_id))
    {
        PROMPT_INFO("old id:[%d], new id:[%d]\n", id, g_t_prompt.t_play_msg.ui2_id);
        return;
    }

    PROMPT_INFO("b_next_prompt:[%d]\n", g_t_prompt.t_play_msg.b_next_prompt);

    _playback_prompt_stop();
    INT32 i4_ret = reset(g_t_prompt.h_player);
    PROMPT_ASSERT(0 != i4_ret, return, "<player>stop failed, ret:[%d]!\n", i4_ret);

    malloc_trim(0);

    if (g_t_prompt.i4_volume >= 0)
    {
        u_acfg_set_volume(g_t_prompt.i4_volume, FALSE);
        g_t_prompt.i4_volume = -1;
    }

    if (g_t_prompt.t_play_msg.b_next_prompt)
    {
        g_t_prompt.t_play_msg.b_next_prompt = FALSE;
        memcpy(&(g_t_prompt.t_play_msg.t_voice_prompt),
                &(g_t_prompt.t_play_msg.t_next_prompt),
                sizeof(g_t_prompt.t_play_msg.t_voice_prompt));
        _playback_prompt_process_start_play();
    }
    else
    {
        _playback_prompt_inform_playback_status_to_sm(SM_BODY_STOP);
    }
}

static VOID _playback_prompt_send_self_msg(UINT32 ui4_msg_id, UINT32 ui4_data1)
{
    PB_PROMPT_MSG_T pb_prompt_msg;
    memset(&pb_prompt_msg, 0x00, sizeof(PB_PROMPT_MSG_T));
    pb_prompt_msg.ui4_msg_id = ui4_msg_id;
    pb_prompt_msg.ui4_data1 = ui4_data1;
    u_playback_prompt_send_msg(&pb_prompt_msg);
}

static VOID _playback_prompt_send_prompt_start(UINT32 ui4_id)
{
    _playback_prompt_send_self_msg(PROMPT_START, ui4_id);
}

static VOID _playback_prompt_send_prompt_pause(UINT32 ui4_id)
{
    _playback_prompt_send_self_msg(PROMPT_PAUSE, ui4_id);
}

static VOID _playback_prompt_send_prompt_resume(UINT32 ui4_id)
{
    _playback_prompt_send_self_msg(PROMPT_RESUME, ui4_id);
}

static VOID _playback_prompt_send_prompt_stop(UINT32 ui4_id)
{
    _playback_prompt_send_self_msg(PROMPT_STOP, ui4_id);
}

INT32 _playback_prompt_process_self_msg(APPMSG_T* pv_msg)
{
    PB_PROMPT_MSG_T *pt_prompt_msg = (PB_PROMPT_MSG_T *)pv_msg;
    UINT16 ui2_id = (UINT16)pt_prompt_msg->ui4_data1;
    PROMPT_INFO("prompt msg id [%d]\n", pt_prompt_msg->ui4_msg_id);

    switch (pt_prompt_msg->ui4_msg_id)
    {
        case PROMPT_START:
            _playback_prompt_process_start(ui2_id);
            break;
        case PROMPT_PAUSE:
            _playback_prompt_process_pause(ui2_id);
            break;
        case PROMPT_RESUME:
            _playback_prompt_process_resume(ui2_id);
            break;
        case PROMPT_STOP:
            _playback_prompt_process_stop(ui2_id);
            break;
        default:
            break;
    }
    return AEER_OK;
}

VOID playback_prompt_process_sm_play(unsigned char *pt_msg, UINT32 ui2_id)
{
    g_t_prompt.t_play_msg.ui2_id = ui2_id;
    if (!_playback_get_prompt_running_flag())
    {
        PROMPT_INFO("first prompt play request!\n");
        memcpy(&(g_t_prompt.t_play_msg.t_voice_prompt), pt_msg, sizeof(g_t_prompt.t_play_msg.t_voice_prompt));
        _playback_prompt_process_start_play();
    }
    else
    {
        PROMPT_INFO("enqueue prompt play!\n");
        g_t_prompt.t_play_msg.b_next_prompt = TRUE;
        memcpy(&(g_t_prompt.t_play_msg.t_next_prompt), pt_msg, sizeof(g_t_prompt.t_play_msg.t_next_prompt));
    }
}

static VOID _playback_prompt_process_sm_pause(VOID)
{
    if (_playback_get_prompt_running_flag() && !playback_get_prompt_pause_flag())
    {
        if (playback_get_prompt_prepare_flag())
        {
            playback_set_prompt_pause_flag(TRUE);
        }
        else
        {
            _playback_prompt_process_pause(g_t_prompt.t_play_msg.ui2_id);
        }
    }
}

static VOID _playback_prompt_process_sm_resume(VOID)
{
    if (_playback_get_prompt_running_flag() && playback_get_prompt_pause_flag())
    {
        if (playback_get_prompt_prepare_flag())
        {
            playback_set_prompt_pause_flag(FALSE);
        }
        else
        {
            _playback_prompt_process_resume(g_t_prompt.t_play_msg.ui2_id);
        }
    }
}

static VOID _playback_prompt_process_sm_request_permit(APPMSG_T* pv_msg)
{
    switch (SM_MSG_BDY(pv_msg->ui4_msg_type))
    {
        case SM_BODY_PLAY:
            playback_prompt_process_sm_play(pv_msg->p_usr_msg, SM_MSG_ID(pv_msg->ui4_msg_type));
            break;
        case SM_BODY_PAUSE:
            _playback_prompt_process_sm_pause();
            break;
        case SM_BODY_RESUME:
            _playback_prompt_process_sm_resume();
            break;
        case SM_BODY_STOP:
            g_t_prompt.t_play_msg.b_next_prompt = FALSE;
            _playback_prompt_process_stop(g_t_prompt.t_play_msg.ui2_id);
            break;
        default:
            break;
    }
}

static INT32 _playback_prompt_process_sm_msg(APPMSG_T* pv_msg)
{
    PROMPT_INFO("sm msg grp:[%d] bdy:[%d] id:[%d]\n", SM_MSG_GRP(pv_msg->ui4_msg_type), SM_MSG_BDY(pv_msg->ui4_msg_type), SM_MSG_ID(pv_msg->ui4_msg_type));
    switch(SM_MSG_GRP(pv_msg->ui4_msg_type))
    {
        case SM_REQUEST_GRP:
        case SM_PERMIT_GRP:
            _playback_prompt_process_sm_request_permit(pv_msg);
            break;
        case SM_FORBID_GRP:
            PROMPT_ERR("forbid msg body[%d]\n", SM_MSG_BDY(pv_msg->ui4_msg_type));
            break;
        default:
            break;
    }
    return AEER_OK;
}

static VOID _playback_prompt_process_end(int id, PLAYBACK_PROMPT_PLAY_RESULT_E e_play_result)
{
    PROMPT_INFO("prompt end, volume:[%d] b_next_prompt:[%d]\n", g_t_prompt.i4_volume, g_t_prompt.t_play_msg.b_next_prompt);
    if (g_t_prompt.i4_volume >= 0)
    {
        u_acfg_set_volume(g_t_prompt.i4_volume, FALSE);
        g_t_prompt.i4_volume = -1;
    }

    if (FALSE == g_t_prompt.t_play_msg.b_next_prompt)
    {
        g_t_prompt.t_play_msg.e_play_result = e_play_result;
        _playback_prompt_upload_finish_state();
        g_t_prompt.t_play_msg.e_play_result = PROMPT_PLAY_NORMAL;
    }
    _playback_prompt_send_prompt_stop((UINT32)id);
}

static void _player_msg_callback(int msg, int id, int ext1, int ext2)
{
    PROMPT_MSG("<player>callback msg:[%d] id:[%d] ext1:[%d] ext2:[%d]\n", msg, id, ext1, ext2);
    switch (msg)
    {
        case MEDIA_PLAYBACK_COMPLETE:
            _playback_prompt_process_end(id, PROMPT_PLAY_NORMAL);
            break;
        case MEDIA_ERROR:
            _playback_prompt_process_end(id, PROMPT_ERROR_UNKOWN);
            break;
        case MEDIA_PREPARED:
            playback_set_prompt_prepare_flag(FALSE);

            g_t_prompt.i4_volume = 0;
            u_acfg_get_volume(&(g_t_prompt.i4_volume));
            u_acfg_set_volume(g_t_prompt.t_play_msg.t_voice_prompt.volume, FALSE);

            if (playback_get_prompt_pause_flag())
            {
                _playback_prompt_inform_playback_status_to_sm(SM_BODY_PAUSE);
            }
            else
            {
                _playback_prompt_send_prompt_start(id);
            }
            break;
        default:
            break;
    }
}

static INT32 _playback_prompt_player_init(VOID)
{
    AudioFormat t_output_format = {0};

    g_t_prompt.h_player = player_init();
    PROMPT_ASSERT(!g_t_prompt.h_player, return PB_PROMPT_FAIL, "player init failed!\n");

    t_output_format.mChannelLayout = AV_CH_LAYOUT_STEREO;
    t_output_format.mSampleFmt = AV_SAMPLE_FMT_S16;
    t_output_format.mSampleRate = 44100;
    t_output_format.mChannels = 2;
    memcpy(t_output_format.mDeviceName, PROMPT_DEVICE_NAME, strlen(PROMPT_DEVICE_NAME));
    setPlaybackSettings(g_t_prompt.h_player, &t_output_format);

    registerCallback(g_t_prompt.h_player, _player_msg_callback);

    return PB_PROMPT_OK;
}

static INT32 _playback_prompt_init(
        const CHAR*                 ps_name,
        HANDLE_T                    h_app
        )
{
    INT32 i4_ret;
    AudioFormat t_output_format = {0};

    memset(&g_t_prompt, 0, sizeof(PLAYBACK_PROMPT_OBJ_T));
    g_t_prompt.h_app = h_app;

    if (g_t_prompt.b_app_init_ok)
    {
        return AEER_OK;
    }

#ifdef CLI_SUPPORT
    i4_ret = playback_prompt_cli_attach_cmd_tbl();
    if ((i4_ret != CLIR_NOT_INIT) && (i4_ret != CLIR_OK))
    {
        PROMPT_ERR("Err: playback_prompt_cli_attach_cmd_tbl() failed, ret=%ld\r\n", i4_ret);
        return AEER_FAIL;
    }
#endif/* CLI_SUPPORT */

    i4_ret = _playback_prompt_player_init();
    PROMPT_ASSERT(i4_ret, return AEER_FAIL, "_playback_prompt_player_init failed");

    g_t_prompt.i4_volume = -1;

    g_t_prompt.b_app_init_ok = TRUE;

    return AEER_OK;

}

static INT32 _playback_prompt_exit (
        HANDLE_T                    h_app,
        APP_EXIT_MODE_T             e_exit_mode
        )
{
    if (FALSE == g_t_prompt.b_app_init_ok)
    {
        return AEER_FAIL;
    }

    player_deinit(g_t_prompt.h_player);

    g_t_prompt.b_app_init_ok = FALSE;

    return AEER_OK;
}

static INT32 _playback_prompt_process_msg (
        HANDLE_T                    h_app,
        UINT32                      ui4_type,
        const VOID*                 pv_msg,
        SIZE_T                      z_msg_len,
        BOOL                        b_paused
        )
{
    APPMSG_T *pt_app_msg = pv_msg;

    if (FALSE == g_t_prompt.b_app_init_ok) {
        return AEER_FAIL;
    }

    if (NULL == pv_msg)
    {
        return AEER_FAIL;
    }

    if (ui4_type < AMB_BROADCAST_OFFSET)
    {
        /* private message */
        switch(ui4_type)
        {
            case E_APP_MSG_TYPE_PROMPT:
                _playback_prompt_process_self_msg(pt_app_msg);
                break;
            case E_APP_MSG_TYPE_STATE_MNGR:
                _playback_prompt_process_sm_msg(pt_app_msg);
                break;
            default:
                break;
        }
    }
    return AEER_OK;
}

INT32 u_playback_prompt_send_msg(const PB_PROMPT_MSG_T* pt_event)
{
    INT32 i4_ret;

    if (FALSE == g_t_prompt.b_app_init_ok)
    {
        return PB_PROMPT_FAIL;
    }

    i4_ret = u_app_send_msg(g_t_prompt.h_app,
                            E_APP_MSG_TYPE_PROMPT,
                            pt_event,
                            sizeof(PB_PROMPT_MSG_T),
                            NULL,
                            NULL);
    PROMPT_ASSERT(AEER_OK != i4_ret, , "_app_send_msg sent E_APP_MSG_TYPE_PROMPT failed, i4_ret[%ld]\n",i4_ret);
    return (i4_ret == AEER_OK ? PB_PROMPT_OK : PB_PROMPT_FAIL);
}

static VOID _playback_prompt_get_player_info(PLAYBACK_PROMPT_STATUS_E e_status,
        ASSISTANT_STUB_PLAYER_STATUS_CHANGE_T *pt_status)
{
    BOOL b_mute;

    if (NULL == pt_status)
    {
        PROMPT_ERR("pt_status can't be NULL!\n");
        return;
    }

    memcpy(pt_status->command, "/playback/player_status_change", ASSISTANT_STUB_COMMAND_MAX_LENGTH);

    memcpy(pt_status->player.source, "none", ASSISTANT_STUB_SOURCE_MAX_LENGTH);
    //volume
    if (!u_acfg_get_mute(&b_mute) && b_mute)
    {
        pt_status->player.volume = 0;
    }
    else
    {
        u_acfg_get_volume(&pt_status->player.volume);
    }

    //status
    switch(e_status)
    {
        case SM_BODY_STOP:
            memcpy(pt_status->player.status, "stop", ASSISTANT_STUB_STATUS_MAX_LENGTH);
            return;
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
    memcpy(pt_status->player.source, "sdk", ASSISTANT_STUB_SOURCE_MAX_LENGTH);

}

VOID u_playback_prompt_get_player_status(ASSISTANT_STUB_PLAYER_STATUS_CHANGE_T *pt_status)
{
    PLAYBACK_PROMPT_STATUS_E e_status = PROMPT_STATUS_STOP;

    if (_playback_get_prompt_running_flag())
    {
        e_status = PROMPT_STATUS_PLAY;
        if (playback_get_prompt_pause_flag())
        {
            e_status = PROMPT_STATUS_PAUSE;
        }
    }
    _playback_prompt_get_player_info(e_status, pt_status);
}

VOID a_playback_prompt_register(AMB_REGISTER_INFO_T* pt_reg)
{
    if (TRUE == g_t_prompt.b_app_init_ok)
    {
        return;
    }

    memset(pt_reg->s_name, 0, sizeof(CHAR)*(APP_NAME_MAX_LEN + 1));
    strncpy(pt_reg->s_name, PB_PROMPT_THREAD_NAME, APP_NAME_MAX_LEN);

    pt_reg->t_fct_tbl.pf_init                   = _playback_prompt_init;
    pt_reg->t_fct_tbl.pf_exit                   = _playback_prompt_exit;
    pt_reg->t_fct_tbl.pf_process_msg            = _playback_prompt_process_msg;
    pt_reg->t_desc.ui8_flags                    = ~((UINT64)0);
    pt_reg->t_desc.t_thread_desc.z_stack_size   = PB_PROMPT_STACK_SIZE;
    pt_reg->t_desc.t_thread_desc.ui1_priority   = PB_PROMPT_THREAD_PRIORITY;
    pt_reg->t_desc.t_thread_desc.ui2_num_msgs   = PB_PROMPT_NUM_MSGS;
    pt_reg->t_desc.ui2_msg_count                = PB_PROMPT_MSGS_COUNT;
    pt_reg->t_desc.ui2_max_msg_size             = PB_PROMPT_MAX_MSGS_SIZE;
}

