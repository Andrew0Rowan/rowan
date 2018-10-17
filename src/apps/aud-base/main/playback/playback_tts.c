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
#include <sys/types.h>
#include <sys/stat.h>
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
#include "u_playback_tts.h"
#include "u_assistant_stub.h"
#include "u_datalist.h"
#include "Interface.h"
/* private */
#include "playback_tts.h"

/*-----------------------------------------------------------------------------
 * structure, constants, macro definitions
 *---------------------------------------------------------------------------*/
#define TTS_ASSERT(cond, action, ...)       \
    do {                                    \
        if (cond)                           \
        {                                   \
            TTS_ERR(__VA_ARGS__);           \
            action;                         \
        }                                   \
    }                                       \
    while (0)

#define MAX_BUFFER_LEN   2048
#define SERVER_FIFO_NAME "/tmp/pipe_fifo"
#define SAVE_FILE_NAME   "/tmp/tmp.ogg"

enum
{
    TTS_BIN_MSG_START = 0,
    TTS_BIN_MSG_SENDING,
    TTS_BIN_MSG_FINISH
};

typedef struct
{
    int msgType;
    int requestId;
    int needMix;
    int dataLen;
    char buffer[MAX_BUFFER_LEN];
}bin_msg;

/*-----------------------------------------------------------------------------
 * variable declarations
 *---------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------
 * static variable declarations
 *---------------------------------------------------------------------------*/
static UINT16 g_ui2_playback_tts_dbg_level = DBG_INIT_LEVEL_APP_PLAYBACK_TTS;
static PLAYBACK_TTS_OBJ_T g_t_tts = {0};
static DATALIST_T g_t_datalist;
static pthread_t t_write_thread = 0;
/*-----------------------------------------------------------------------------
* private methods declarations
*---------------------------------------------------------------------------*/
static VOID _playback_tts_process_start_play(VOID);
static VOID _playback_tts_process_start(UINT16 id);
static VOID _playback_tts_process_pause(UINT16 id);
static VOID _playback_tts_process_resume(UINT16 id);
static VOID _playback_tts_process_stop(UINT16 id);

static BOOL _playback_set_tts_running_flag(BOOL flag)
{
    g_t_tts.b_running = flag;
}

static BOOL _playback_get_tts_running_flag(void)
{
    return g_t_tts.b_running;
}

static void playback_set_tts_prepare_flag(BOOL flag)
{
    g_t_tts.b_prepare = flag;
}

static BOOL playback_get_tts_prepare_flag(void)
{
    return g_t_tts.b_prepare;
}

static void playback_set_tts_pause_flag(BOOL flag)
{
    g_t_tts.b_pause = flag;
}

static BOOL playback_get_tts_pause_flag(void)
{
    return g_t_tts.b_pause;
}

static VOID _playback_tts_play(VOID)
{
    playback_set_tts_pause_flag(FALSE);
    _playback_set_tts_running_flag(TRUE);
}

static VOID _playback_tts_stop(VOID)
{
    playback_set_tts_pause_flag(FALSE);
    _playback_set_tts_running_flag(FALSE);
}

UINT16 playback_tts_get_dbg_level(VOID)
{
    return (g_ui2_playback_tts_dbg_level | DBG_LAYER_APP);
}

VOID playback_tts_set_dbg_level(UINT16 ui2_db_level)
{
    g_ui2_playback_tts_dbg_level = ui2_db_level;
}

static INT32 _playback_tts_send_msg_to_assistant_stub(ASSISTANT_STUB_CMD_INDEX_E e_cmd, VOID *pt_send_msg, SIZE_T t_size)
{
    INT32 i4_ret = PB_TTS_FAIL;
    HANDLE_T h_app = NULL_HANDLE;

    u_am_get_app_handle_from_name(&h_app,ASSISTANT_STUB_THREAD_NAME);

    i4_ret = u_app_send_appmsg(h_app,
                                E_APP_MSG_TYPE_ASSISTANT_STUB,
                                MSG_FROM_TTS,
                                e_cmd,
                                pt_send_msg,
                                t_size);
    if (AEER_OK != i4_ret)
    {
        TTS_ERR("_playback_tts_send_play_request_to_assistant_stub failed, i4_ret[%ld]\n", i4_ret);
    }
    return (i4_ret == AEER_OK ? PB_TTS_OK : PB_TTS_FAIL);
}

static VOID _playback_tts_upload_finish_state(VOID)
{
    ASSISTANT_STUB_PLAY_TTS_DONE_T t_play_msg;

    memset(&t_play_msg, 0, sizeof(t_play_msg));
    strncpy(t_play_msg.command, "/playback/play_tts_done", ASSISTANT_STUB_COMMAND_MAX_LENGTH);

    t_play_msg.tts_id = g_t_tts.t_play_msg.t_tts_info.tts_id;

    if (TTS_PLAY_NORMAL == g_t_tts.t_play_msg.e_play_result)
    {
        t_play_msg.status = 0;
    }
    else
    {
        t_play_msg.status = 1;
    }
    _playback_tts_send_msg_to_assistant_stub(ASSISTANT_STUB_CMD_PLAY_TTS_DONE, &t_play_msg, sizeof(t_play_msg));
    TTS_INFO("tts send play tts done, needmix:%d!\n", g_t_tts.t_play_msg.t_tts_info.need_mix);
}

static INT32 _playback_tts_inform_playback_status_to_sm(SM_MSG_BDY_E PB_BODY)
{
    INT32 i4_ret = PB_TTS_FAIL;
    HANDLE_T h_app = NULL_HANDLE;
    SM_PARAM_T t_sm_param = {0};
    t_sm_param.speech_enable = g_t_tts.t_play_msg.t_tts_info.speech_enable;

    u_am_get_app_handle_from_name(&h_app,SM_THREAD_NAME);

    i4_ret = u_app_send_appmsg(h_app,
                                E_APP_MSG_TYPE_STATE_MNGR,
                                MSG_FROM_TTS,
                                SM_MAKE_MSG(SM_INFORM_GRP,PB_BODY,g_t_tts.t_play_msg.ui2_id),
                                &t_sm_param,
                                sizeof(t_sm_param));
    if (AEER_OK != i4_ret)
    {
        TTS_ERR("_playback_tts_inform_playback_status_to_sm failed, i4_ret[%ld]\n",i4_ret);
    }
    TTS_INFO("playback send %d status to sm!\n", PB_BODY);

    return (i4_ret == AEER_OK ? PB_TTS_OK : PB_TTS_FAIL);
}

static VOID _playback_tts_send_self_msg(UINT32 ui4_msg_id, UINT32 ui4_data1)
{
    PB_TTS_MSG_T pb_tts_msg;
    memset(&pb_tts_msg, 0x00, sizeof(PB_TTS_MSG_T));
    pb_tts_msg.ui4_msg_id = ui4_msg_id;
    pb_tts_msg.ui4_data1 = ui4_data1;
    u_playback_tts_send_msg(&pb_tts_msg);
}

static int _playback_tts_open_fifo(VOID)
{
    int server_fifo_fd;

    if(-1 == access(SERVER_FIFO_NAME, F_OK))
    {
        if (-1 == mkfifo(SERVER_FIFO_NAME, 0777))
        {
            TTS_ERR("create server fifo error!\n");
            return PB_TTS_FAIL;
        }
    }

    server_fifo_fd = open(SERVER_FIFO_NAME, O_RDONLY);
    if (-1 == server_fifo_fd)
    {
        TTS_ERR("open server fifo error!\n");
        return PB_TTS_FAIL;
    }

    return server_fifo_fd;
}

void *_playback_tts_recv_data_thread(void* arg)
{
    int ret;
    int write_size;
    int i4_fifo_fd;
    bin_msg msg;

    pthread_detach(pthread_self());

    while (1)
    {
        i4_fifo_fd = _playback_tts_open_fifo();
        if (-1 == i4_fifo_fd)
        {
            TTS_ERR("tts fifo open failed!\n");
            return NULL;
        }

        while (read(i4_fifo_fd, &msg, sizeof(msg)) > 0)
        {
            TTS_ERR("msgType is %d, dataLen is %d, requestId=%d\n", msg.msgType, msg.dataLen, msg.requestId);

            if ((msg.dataLen > 0) && (msg.dataLen <= MAX_BUFFER_LEN))
            {
                pthread_mutex_lock(&g_t_tts.t_data_mutex);
                while ((ret = u_datalist_enqueue(&g_t_datalist, msg.buffer, msg.dataLen, msg.requestId)) != DATALIST_SUCCESS);
                pthread_mutex_unlock(&g_t_tts.t_data_mutex);
            }
            else if (msg.dataLen > MAX_BUFFER_LEN)
            {
                TTS_ERR("dataLen[%d] is too big, discard!\n", msg.dataLen);
                continue;
            }

            if(TTS_BIN_MSG_FINISH == msg.msgType)
            {
                pthread_mutex_lock(&g_t_tts.t_data_mutex);
                while ((ret = u_datalist_enqueue(&g_t_datalist, NULL, 0, msg.requestId)) != DATALIST_SUCCESS);
                pthread_mutex_unlock(&g_t_tts.t_data_mutex);
            }
        }
        close(i4_fifo_fd);
    }
    TTS_ERR("_playback_tts_recv_data_thread exit \n");
    return NULL;
}

void *_playback_tts_write_data_thread(void* arg)
{
    INT64 i8_player_size = 0;
    INT64 i8_data_size = 0;
    DATANODE_T *pt_node = NULL;

    TTS_MSG("_playback_tts_write_data_thread start!\n");

    //pthread_detach(pthread_self());

    while (_playback_get_tts_running_flag())
    {
        if (!i8_data_size)
        {
            u_datanode_free(pt_node);
            pthread_mutex_lock(&g_t_tts.t_data_mutex);
            pt_node = u_datalist_dequeue(&g_t_datalist);
            pthread_mutex_unlock(&g_t_tts.t_data_mutex);
            if (!pt_node)
            {
                usleep(5 * 1000);
                continue;
            }
            else
            {
                i8_data_size = pt_node->size;
            }
        }

        if (pt_node->id != g_t_tts.t_play_msg.t_tts_info.tts_id)
        {
            i8_data_size = 0;
            TTS_ERR("[%d] is error id, new id is [%d]!\n", pt_node->id, g_t_tts.t_play_msg.t_tts_info.tts_id);
            continue;
        }

        if (!pt_node->size)
        {
            TTS_MSG("tts data finish!\n");
            break;
        }

        if (g_t_tts.t_play_msg.t_tts_info.pcm)
        {
            i8_player_size = 2048;//getAvailiableSize(g_t_tts.h_player); 
        }
        else
        {
            i8_player_size = getAvailiableSize(g_t_tts.h_player); 
        }
        if (!i8_player_size)
        {
            TTS_MSG("getAvailiableSize 0!\n");
            usleep(5 * 1000);
            continue;
        }
        else
        {
            i8_player_size = i8_player_size < i8_data_size ? i8_player_size : i8_data_size;
            TTS_MSG("Write Data:%d!\n", i8_player_size);
            if (g_t_tts.t_play_msg.t_tts_info.pcm)
            {
                writePCMData(g_t_tts.h_player, pt_node->data + pt_node->size - i8_data_size, i8_player_size);
                i8_data_size -= i8_player_size;
            }
            else
            {
                writeData(g_t_tts.h_player, pt_node->data + pt_node->size - i8_data_size, i8_player_size);
                i8_data_size -= i8_player_size;
            }
        }
    }
    u_datanode_free(pt_node);
    TTS_MSG("Write Data With EOS!\n");
    if (g_t_tts.t_play_msg.t_tts_info.pcm)
    {
        writePCMDatawithEOS(g_t_tts.h_player, NULL, 0);
    }
    else
    {
        writeDatawithEOS(g_t_tts.h_player, NULL, 0);
    }
    TTS_MSG("_playback_tts_write_data_thread exit!\n");
    TTS_MSG("running flag:[%d]!\n", _playback_get_tts_running_flag());

    return NULL;
}

static VOID _playback_tts_process_start_play(VOID)
{
    TTS_ERR("_playback_tts_process_start_play enter: PCMData=%d!\n", g_t_tts.t_play_msg.t_tts_info.pcm);
    TTS_ERR("_playback_tts_process_start_play enter: mSampleRate=%d!\n", g_t_tts.t_play_msg.t_tts_info.mSampleRate);
    TTS_ERR("_playback_tts_process_start_play enter: mChannels=%d!\n", g_t_tts.t_play_msg.t_tts_info.mChannels);
    INT32 i4_ret;
    AudioFormat t_output_format = {0};
    t_output_format.mChannelLayout = AV_CH_LAYOUT_STEREO;
    t_output_format.mSampleFmt = AV_SAMPLE_FMT_S16;
    t_output_format.mSampleRate = g_t_tts.t_play_msg.t_tts_info.mSampleRate;
    t_output_format.mChannels = g_t_tts.t_play_msg.t_tts_info.mChannels;

    if (g_t_tts.t_play_msg.t_tts_info.pcm)
    {
        memcpy(t_output_format.mDeviceName, TTS_DEVICE_NAME, strlen(TTS_DEVICE_NAME));
        setPlaybackSettings(g_t_tts.h_player, &t_output_format);
        setPCMMode(g_t_tts.h_player, g_t_tts.t_play_msg.ui2_id);
        start(g_t_tts.h_player);
    }
    else
    {
        memcpy(t_output_format.mDeviceName, TTS_DEVICE_NAME, strlen(TTS_DEVICE_NAME));
        setPlaybackSettings(g_t_tts.h_player, &t_output_format);
    i4_ret = setDataSource(g_t_tts.h_player, g_t_tts.t_play_msg.ui2_id);
    TTS_ASSERT(i4_ret, return, "<player>setDataSource failed, ret:[%d]!\n", i4_ret);

    i4_ret = prepareAsync(g_t_tts.h_player);
    TTS_ASSERT(i4_ret, return, "<player>prepare failed, ret:[%d]!\n", i4_ret);
    }

    playback_set_tts_prepare_flag(TRUE);

    _playback_tts_play();

    i4_ret = pthread_create(&t_write_thread, NULL, _playback_tts_write_data_thread, NULL);
    if (i4_ret)
    {
        TTS_ERR("write data thread create failed, ret:[%d]!\n", i4_ret);
        _playback_tts_process_stop(g_t_tts.t_play_msg.ui2_id);
    }
    else
    {
        _playback_tts_inform_playback_status_to_sm(SM_BODY_PLAY);
    }
    g_t_tts.b_switch = FALSE;
}

static VOID _playback_tts_process_start(UINT16 id)
{
    if (id != g_t_tts.t_play_msg.ui2_id)
    {
        TTS_INFO("old id:[%d], new id:[%d]\n", id, g_t_tts.t_play_msg.ui2_id);
        return;
    }

    start(g_t_tts.h_player);
    _playback_tts_inform_playback_status_to_sm(SM_BODY_PLAY);
}

static VOID _playback_tts_process_pause(UINT16 id)
{
    if (id != g_t_tts.t_play_msg.ui2_id)
    {
        TTS_INFO("old id:[%d], new id:[%d]\n", id, g_t_tts.t_play_msg.ui2_id);
        return;
    }

    if (!_playback_get_tts_running_flag())
    {
        TTS_INFO("player not running!\n");
        return;
    }

    if (playback_get_tts_pause_flag())
    {
        TTS_INFO("player is pause!\n");
        return;
    }

    INT32 i4_ret = pause_l(g_t_tts.h_player);
    TTS_ASSERT(i4_ret, return, "<player>pause failed, ret:[%d]!\n", i4_ret);

    playback_set_tts_pause_flag(TRUE);
    _playback_tts_inform_playback_status_to_sm(SM_BODY_PAUSE);
}

static VOID _playback_tts_process_resume(UINT16 id)
{
    if (id != g_t_tts.t_play_msg.ui2_id)
    {
        TTS_INFO("old id:[%d], new id:[%d]\n", id, g_t_tts.t_play_msg.ui2_id);
        return;
    }

    if (!_playback_get_tts_running_flag())
    {
        TTS_INFO("player not running!\n");
        return;
    }

    if (FALSE == playback_get_tts_pause_flag())
    {
        TTS_INFO("player is playing!\n");
        return;
    }

    INT32 i4_ret = start(g_t_tts.h_player);
    TTS_ASSERT(i4_ret, return, "<player>start failed, ret:[%d]!\n", i4_ret);

    playback_set_tts_pause_flag(FALSE);
    _playback_tts_inform_playback_status_to_sm(SM_BODY_PLAY);
}

static VOID _playback_tts_process_stop(UINT16 id)
{
    if (id != g_t_tts.t_play_msg.ui2_id)
    {
        TTS_INFO("old id:[%d], new id:[%d]\n", id, g_t_tts.t_play_msg.ui2_id);
        return;
    }

    _playback_tts_stop();
    if (t_write_thread)
    {
        pthread_join(t_write_thread, NULL);
        t_write_thread = 0;
        TTS_INFO("tts write data thread join!\n");
    }

    INT32 i4_ret = reset(g_t_tts.h_player);
    TTS_ASSERT(i4_ret, return, "<player>stop failed, ret:[%d]!\n", i4_ret);

    malloc_trim(0);

    _playback_tts_upload_finish_state();
    g_t_tts.t_play_msg.e_play_result = TTS_PLAY_NORMAL;
    _playback_tts_inform_playback_status_to_sm(SM_BODY_STOP);
}

static VOID _playback_tts_send_self_start(UINT32 ui4_id)
{
    _playback_tts_send_self_msg(TTS_START, ui4_id);
}

static VOID _playback_tts_send_self_pause(UINT32 ui4_id)
{
    _playback_tts_send_self_msg(TTS_PAUSE, ui4_id);
}

static VOID _playback_tts_send_self_resume(UINT32 ui4_id)
{
    _playback_tts_send_self_msg(TTS_RESUME, ui4_id);
}

static VOID _playback_tts_send_self_stop(UINT32 ui4_id)
{
    _playback_tts_send_self_msg(TTS_STOP, ui4_id);
}

INT32 playback_tts_process_self_msg(APPMSG_T* pv_msg)
{
    PB_TTS_MSG_T *pt_tts_msg = (PB_TTS_MSG_T *)pv_msg;
    UINT16 ui2_id = (UINT16)pt_tts_msg->ui4_data1;
    TTS_INFO("tts msg id [%d]\n", pt_tts_msg->ui4_msg_id);

    switch (pt_tts_msg->ui4_msg_id)
    {
       case TTS_START:
            _playback_tts_process_start(ui2_id);
            break;
        case TTS_PAUSE:
            _playback_tts_process_pause(ui2_id);
            break;
        case TTS_RESUME:
            _playback_tts_process_resume(ui2_id);
            break;
        case TTS_STOP:
            _playback_tts_process_stop(ui2_id);
            break;
        default:
            break;
    }
    return AEER_OK;
}


static VOID _playback_tts_process_sm_play(unsigned char *pt_msg, UINT32 ui2_id)
{
    INT32 i4_ret;
    _playback_tts_stop();
    if (t_write_thread)
    {
        pthread_join(t_write_thread, NULL);
        t_write_thread = 0;
        TTS_INFO("tts write data thread join!\n");
    }

    g_t_tts.b_switch = TRUE;
    g_t_tts.t_play_msg.ui2_id = ui2_id;
    memcpy(&(g_t_tts.t_play_msg.t_tts_info), pt_msg, sizeof(g_t_tts.t_play_msg.t_tts_info));

    i4_ret = reset(g_t_tts.h_player);
    TTS_ASSERT(i4_ret, return, "<player>reset failed, ret:[%d]!\n", i4_ret);

    _playback_tts_process_start_play();
}

static VOID _playback_tts_process_sm_pause(VOID)
{
    if (_playback_get_tts_running_flag() && !playback_get_tts_pause_flag())
    {
        if (playback_get_tts_prepare_flag())
        {
            playback_set_tts_pause_flag(TRUE);
        }
        else
        {
            _playback_tts_process_pause(g_t_tts.t_play_msg.ui2_id);
        }
    }
}

static VOID _playback_tts_process_sm_resume(VOID)
{
    if (_playback_get_tts_running_flag() && playback_get_tts_pause_flag())
    {
        if (playback_get_tts_prepare_flag())
        {
            playback_set_tts_pause_flag(FALSE);
        }
        else
        {
            _playback_tts_process_resume(g_t_tts.t_play_msg.ui2_id);
        }
    }
}

static VOID _playback_tts_process_sm_request_permit(APPMSG_T* pv_msg)
{
    switch(SM_MSG_BDY(pv_msg->ui4_msg_type))
    {
        case SM_BODY_PLAY:
            _playback_tts_process_sm_play(pv_msg->p_usr_msg, SM_MSG_ID(pv_msg->ui4_msg_type));
            break;
        case SM_BODY_PAUSE:
            _playback_tts_process_sm_pause();
            break;
        case SM_BODY_RESUME:
            _playback_tts_process_sm_resume();
            break;
        case SM_BODY_STOP:
            _playback_tts_process_stop(g_t_tts.t_play_msg.ui2_id);
           break;
        default:
            break;
    }
}

INT32 playback_tts_process_sm_msg(APPMSG_T* pv_msg)
{
    TTS_INFO("sm msg grp:[%d] bdy:[%d] id:[%d]\n", SM_MSG_GRP(pv_msg->ui4_msg_type), SM_MSG_BDY(pv_msg->ui4_msg_type), SM_MSG_ID(pv_msg->ui4_msg_type));
    switch(SM_MSG_GRP(pv_msg->ui4_msg_type))
    {
        case SM_REQUEST_GRP:
        case SM_PERMIT_GRP:
            _playback_tts_process_sm_request_permit(pv_msg);
            break;
        case SM_FORBID_GRP:
            TTS_ERR("forbid msg body[%d]\n", SM_MSG_BDY(pv_msg->ui4_msg_type));
            break;
        default:
            break;
    }
    return AEER_OK;
}

static INT32 _playback_tts_process_msg (
        HANDLE_T                    h_app,
        UINT32                      ui4_type,
        const VOID*                 pv_msg,
        SIZE_T                      z_msg_len,
        BOOL                        b_paused
        )
{
    APPMSG_T *pt_app_msg = pv_msg;

    if (FALSE == g_t_tts.b_app_init_ok) {
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
            case E_APP_MSG_TYPE_TTS:
                playback_tts_process_self_msg(pt_app_msg);
                break;
            case E_APP_MSG_TYPE_STATE_MNGR:
                playback_tts_process_sm_msg(pt_app_msg);
                break;
            default:
                break;
        }
    }
    return AEER_OK;
}

static void _player_msg_callback(int msg, int id, int ext1, int ext2)
{
    TTS_MSG("<player>callback msg:[%d] id:[%d] ext1:[%d] ext2:[%d]\n", msg, id, ext1, ext2);
    switch (msg)
    {
        case MEDIA_PLAYBACK_COMPLETE:
        case MEDIA_ERROR:
            if (g_t_tts.b_switch)
            {
                break;
            }
            if (MEDIA_ERROR == msg)
            {
                g_t_tts.t_play_msg.e_play_result = TTS_PLAY_ERROR;
            }
            else
            {
                g_t_tts.t_play_msg.e_play_result = TTS_PLAY_NORMAL;
            }
            _playback_tts_send_self_stop((UINT32)id);
            break;
        case MEDIA_PREPARED:
            playback_set_tts_prepare_flag(FALSE);
            if (playback_get_tts_pause_flag())
            {
                _playback_tts_inform_playback_status_to_sm(SM_BODY_PAUSE);
            }
            else
            {
                _playback_tts_send_self_start((UINT32)id);
            }
            break;
        default:
            break;
    }
}

static INT32 _playback_tts_player_init(VOID)
{
    //AudioFormat t_output_format = {0};

    g_t_tts.h_player = player_init();
    TTS_ASSERT(!g_t_tts.h_player, return PB_TTS_FAIL, "player init failed!\n");

/*
    t_output_format.mChannelLayout = AV_CH_LAYOUT_STEREO;
    t_output_format.mSampleFmt = AV_SAMPLE_FMT_S16;
    t_output_format.mSampleRate = PLAYBACK_TTS_SAMPLERATE;
    t_output_format.mChannels = PLAYBACK_TTS_CHANNLE;
    memcpy(t_output_format.mDeviceName, TTS_DEVICE_NAME, strlen(TTS_DEVICE_NAME));
    setPlaybackSettings(g_t_tts.h_player, &t_output_format);
    //start(g_t_tts.h_player);
*/
    registerCallback(g_t_tts.h_player, _player_msg_callback);

    return PB_TTS_OK;
}

static INT32 _playback_tts_init(
        const CHAR*                 ps_name,
        HANDLE_T                    h_app
        )
{
    INT32 i4_ret;
    pthread_t t_data_thread;

    memset(&g_t_tts, 0, sizeof(PLAYBACK_TTS_OBJ_T));
    g_t_tts.h_app = h_app;

    if (g_t_tts.b_app_init_ok)
    {
        return AEER_OK;
    }

#ifdef CLI_SUPPORT
    i4_ret = playback_tts_cli_attach_cmd_tbl();
    if ((i4_ret != CLIR_NOT_INIT) && (i4_ret != CLIR_OK))
    {
        TTS_ERR("Err: playback_tts_cli_attach_cmd_tbl() failed, ret=%ld\r\n", i4_ret);
        return AEER_FAIL;
    }
#endif/* CLI_SUPPORT */

    i4_ret = _playback_tts_player_init();
    TTS_ASSERT(i4_ret, return AEER_FAIL, "_playback_tts_player_init failed");

    u_datalist_init(&g_t_datalist);

    i4_ret = pthread_mutex_init(&g_t_tts.t_data_mutex, NULL);
    TTS_ASSERT(i4_ret, return AEER_FAIL, "pthread_mutex_init failed, ret=%ld\n", i4_ret);

    i4_ret = pthread_create(&t_data_thread, NULL, _playback_tts_recv_data_thread, NULL);
    TTS_ASSERT(i4_ret, return AEER_FAIL, "data pthread create failed, ret=%ld\n", i4_ret);

    g_t_tts.b_app_init_ok = TRUE;

    return AEER_OK;
}

static INT32 _playback_tts_exit (
        HANDLE_T                    h_app,
        APP_EXIT_MODE_T             e_exit_mode
        )
{
    if (FALSE == g_t_tts.b_app_init_ok)
    {
        return AEER_FAIL;
    }

    pthread_mutex_destroy(&g_t_tts.t_data_mutex);
    u_datalist_destroy(&g_t_datalist);
    player_deinit(g_t_tts.h_player);

    g_t_tts.b_app_init_ok = FALSE;

    return AEER_OK;
}

INT32 u_playback_tts_send_msg(const PB_TTS_MSG_T* pt_event)
{
    INT32 i4_ret;

    if (FALSE == g_t_tts.b_app_init_ok)
    {
        return PB_TTS_FAIL;
    }

    i4_ret = u_app_send_msg(g_t_tts.h_app,
                            E_APP_MSG_TYPE_TTS,
                            pt_event,
                            sizeof(PB_TTS_MSG_T),
                            NULL,
                            NULL);
    TTS_ASSERT(AEER_OK != i4_ret, , "_app_send_msg sent E_APP_MSG_TYPE_TTS failed, i4_ret[%ld]\n",i4_ret);
    return (i4_ret == AEER_OK ? PB_TTS_OK : PB_TTS_FAIL);
}

VOID a_playback_tts_register(AMB_REGISTER_INFO_T* pt_reg)
{
    if (TRUE == g_t_tts.b_app_init_ok)
    {
        return;
    }

    memset(pt_reg->s_name, 0, sizeof(CHAR)*(APP_NAME_MAX_LEN + 1));
    strncpy(pt_reg->s_name, PB_TTS_THREAD_NAME, APP_NAME_MAX_LEN);

    pt_reg->t_fct_tbl.pf_init                   = _playback_tts_init;
    pt_reg->t_fct_tbl.pf_exit                   = _playback_tts_exit;
    pt_reg->t_fct_tbl.pf_process_msg            = _playback_tts_process_msg;
    pt_reg->t_desc.ui8_flags                    = ~((UINT64)0);
    pt_reg->t_desc.t_thread_desc.z_stack_size   = PB_TTS_STACK_SIZE;
    pt_reg->t_desc.t_thread_desc.ui1_priority   = PB_TTS_THREAD_PRIORITY;
    pt_reg->t_desc.t_thread_desc.ui2_num_msgs   = PB_TTS_NUM_MSGS;
    pt_reg->t_desc.ui2_msg_count                = PB_TTS_MSGS_COUNT;
    pt_reg->t_desc.ui2_max_msg_size             = PB_TTS_MAX_MSGS_SIZE;
}

