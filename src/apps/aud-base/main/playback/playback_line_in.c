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
#include <string.h>
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
#include "u_playback_usb.h"
#include "u_playback_line_in.h"

/* private */
#include "playback_line_in.h"

/*-----------------------------------------------------------------------------
 * structure, constants, macro definitions
 *---------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------
 * variable declarations
 *---------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------
 * static variable declarations
 *---------------------------------------------------------------------------*/
static UINT16 g_ui2_playback_line_in_dbg_level = DBG_INIT_LEVEL_APP_PLAYBACK_LINE_IN;
static PLAYBACK_LINE_IN_OBJ_T g_t_pb_line_in = {0};

static pthread_t t_record;
static pthread_t t_play;
static volatile INT32 g_line_in_play_flag = FALSE;

/*-----------------------------------------------------------------------------
 * private methods declarations
 *---------------------------------------------------------------------------*/
static INT32 _playback_line_in_init(
                        const CHAR*                 ps_name,
                        HANDLE_T                    h_app
                        );
static INT32 _playback_line_in_exit (
                    HANDLE_T                    h_app,
                    APP_EXIT_MODE_T             e_exit_mode
                    );
static INT32 _playback_line_in_process_msg (
                    HANDLE_T                    h_app,
                    UINT32                      ui4_type,
                    const VOID*                 pv_msg,
                    SIZE_T                      z_msg_len,
                    BOOL                        b_paused
                    );

static INT32 _playback_line_in_create_thread(VOID);
static INT32 _playback_line_in_send_pb_status_to_sm(SM_MSG_BDY_E PB_BODY);

static VOID _playback_line_in_play(VOID)
{
    g_line_in_play_flag = TRUE;
}

static VOID _playback_line_in_stop(VOID)
{
    g_line_in_play_flag = FALSE;
}

static BOOL _playback_get_line_in_thread_running_flag(void)
{
    return g_line_in_play_flag;
}

static BOOL playback_get_line_in_thread_pause_flag(void)
{
    return g_t_pb_line_in.b_pause;
}

static void playback_set_line_in_thread_pause_flag(BOOL flag)
{
    g_t_pb_line_in.b_pause = flag;
}

VOID a_playback_line_in_register(AMB_REGISTER_INFO_T* pt_reg)
{
    if (TRUE == g_t_pb_line_in.b_app_init_ok)
    {
        return;
    }

    memset(pt_reg->s_name, 0, sizeof(CHAR)*(APP_NAME_MAX_LEN + 1));
    strncpy(pt_reg->s_name, PB_LINE_IN_THREAD_NAME, APP_NAME_MAX_LEN);

    pt_reg->t_fct_tbl.pf_init                   = _playback_line_in_init;
    pt_reg->t_fct_tbl.pf_exit                   = _playback_line_in_exit;
    pt_reg->t_fct_tbl.pf_process_msg            = _playback_line_in_process_msg;
    pt_reg->t_desc.ui8_flags                    = ~((UINT64)0);
    pt_reg->t_desc.t_thread_desc.z_stack_size   = PB_LINE_IN_STACK_SIZE;
    pt_reg->t_desc.t_thread_desc.ui1_priority   = PB_LINE_IN_THREAD_PRIORITY;
    pt_reg->t_desc.t_thread_desc.ui2_num_msgs   = PB_LINE_IN_NUM_MSGS;
    pt_reg->t_desc.ui2_msg_count                = PB_LINE_IN_MSGS_COUNT;
    pt_reg->t_desc.ui2_max_msg_size             = PB_LINE_IN_MAX_MSGS_SIZE;
}

UINT16 playback_line_in_get_dbg_level(VOID)
{
    return (g_ui2_playback_line_in_dbg_level | DBG_LAYER_APP);
}

VOID playback_line_in_set_dbg_level(UINT16 ui2_db_level)
{
    g_ui2_playback_line_in_dbg_level = ui2_db_level;
}

static VOID* _playback_line_in_record_process(VOID *arg)
{
    INT32 i4_ret;
    INT32 write_data_remain;
    INT32 write_data_done;
    snd_pcm_t *handle;
    PCMContainer_t *pcm_params;
    RINGBUF_H h_ringbuf = (RINGBUF_H)arg;

    pcm_params = (PCMContainer_t *)malloc(sizeof(PCMContainer_t));
    if(NULL == pcm_params)
    {
        LINE_IN_ERR("malloc pcm_params failed!\n");
        goto MALLOC_ERR;
    }

    memset(pcm_params, 0x00, sizeof(PCMContainer_t));
    pcm_params->format = SND_PCM_FORMAT_S16_LE;
    pcm_params->handle = handle;
    pcm_params->sample_rate = PLAYBACK_LINE_IN_SAMPLERATE;//44.1kHZs
    pcm_params->channels = PLAYBACK_LINE_IN_CHANNLE;

    //set mixer
    i4_ret = system("amixer cset name='O05 I00 Switch' 1");
    if (0 != i4_ret)
    {
        LINE_IN_ERR("ipcd 'O05 I00 Switch' failed!\n");
        //goto SET_UP_ERR;
    }

    i4_ret = system("amixer cset name='O06 I01 Switch' 1");
    if (-1 == i4_ret)
    {
        LINE_IN_ERR("ipcd 'O06 I01 Switch' failed!\n");
        //goto SET_UP_ERR;
    }

    i4_ret = snd_pcm_open(&pcm_params->handle, LINE_IN_DEVICE_NAME, SND_PCM_STREAM_CAPTURE, 0);
    if (0 != i4_ret)
    {
        LINE_IN_ERR("snd_pcm_open failed!\n");
        goto SET_UP_ERR;
    }
    i4_ret = u_alsa_set_hw_params(pcm_params, 400000);
    if(0 != i4_ret)
    {
        LINE_IN_ERR("u_alsa_set_hw_params failed!\n");
        goto SET_UP_ERR;
    }

    while(_playback_get_line_in_thread_running_flag())
    {
        if (playback_get_line_in_thread_pause_flag())
        {
            sleep(1);
            continue;
        }

        i4_ret = u_alsa_read_pcm(pcm_params, pcm_params->chunk_size);
        //printf("u_alsa_read_pcm i4_ret = %d\n", i4_ret);
        //printf("u_alsa_read_pcm pcm_params->chunk_size = %d\n", pcm_params->chunk_bytes/4);
        if(i4_ret == pcm_params->chunk_size)
        {
            write_data_remain = pcm_params->chunk_bytes;
            write_data_done = 0;
            while(write_data_remain > 0)
            {
                i4_ret = u_ringbuf_write(h_ringbuf, pcm_params->data_buf + write_data_done, write_data_remain);
                write_data_remain -= i4_ret;
                write_data_done += i4_ret;
            }
        }
    }

    snd_pcm_drain(pcm_params->handle);
    if(NULL != pcm_params->data_buf)
    {
        free(pcm_params->data_buf);
    }
    snd_pcm_close(pcm_params->handle);
    free(pcm_params);
    return NULL;

SET_UP_ERR:
    free(pcm_params);
MALLOC_ERR:
    _playback_line_in_stop();
    return NULL;
}

static VOID* _playback_line_in_play_process(VOID *arg)
{
    BOOL  b_prev_pause = FALSE;
    INT32 i4_ret;
    INT32 read_data_remain;
    INT32 read_data_done;
    snd_pcm_t *handle;
    PCMContainer_t *pcm_params;
    RINGBUF_H h_ringbuf = (RINGBUF_H)arg;

    pcm_params = (PCMContainer_t *)malloc(sizeof(PCMContainer_t));
    if(NULL == pcm_params)
    {
        LINE_IN_ERR("malloc failed!\n");
        goto MALLOC_ERR;
    }
    memset(pcm_params, 0, sizeof(PCMContainer_t));
    pcm_params->format = SND_PCM_FORMAT_S16_LE;
    pcm_params->handle = handle;
    pcm_params->sample_rate = PLAYBACK_LINE_IN_SAMPLERATE;
    pcm_params->channels = PLAYBACK_LINE_IN_CHANNLE;

    i4_ret = snd_pcm_open(&pcm_params->handle, MAIN_PCM_DEVICE_NAME, SND_PCM_STREAM_PLAYBACK, 0);
    if (0 != i4_ret)
    {
        LINE_IN_ERR("snd_pcm_open failed!, i4_ret = %d\n", i4_ret);
        goto SET_UP_ERR;
    }

    sleep(1);
    i4_ret = u_alsa_set_hw_params(pcm_params, 400000);
    if(0 != i4_ret)
    {
        LINE_IN_ERR("u_alsa_set_hw_params failed!\n");
        goto SET_UP_ERR;
    }

    while(_playback_get_line_in_thread_running_flag())
    {
        if (playback_get_line_in_thread_pause_flag())
        {
            b_prev_pause = TRUE;
            u_ringbuf_clear(h_ringbuf);
            sleep(1);
            continue;
        }

        if(b_prev_pause)
        {
            b_prev_pause = FALSE;
            sleep(1);
        }
        read_data_remain = pcm_params->chunk_bytes;
        read_data_done = 0;
        while(read_data_remain > 0)
        {
            i4_ret = u_ringbuf_read(h_ringbuf, pcm_params->data_buf + read_data_done, read_data_remain);
            read_data_remain -= i4_ret;
            read_data_done += i4_ret;
        }
        i4_ret = u_alsa_write_pcm(pcm_params, pcm_params->chunk_size);
        //printf("u_alsa_write_pcm i4_ret = %d\n", i4_ret);
        //printf("u_alsa_write_pcm pcm_params->chunk_size = %d\n", pcm_params->chunk_bytes/4);
#if 0
        if(i4_ret == read_data_done)
        {
            LINE_IN_INFO("u_alsa_write_pcm %d\n", i4_ret);
        }
        while (i4_ret--)
        {
            printf("%x ", *(pcm_params->data_buf + read_data_done - i4_ret - 1));
        }
        printf("\n");
#endif
    }

    snd_pcm_drain(pcm_params->handle);
    if(NULL != pcm_params->data_buf)
    {
        free(pcm_params->data_buf);
    }
    snd_pcm_close(pcm_params->handle);
    free(pcm_params);
    return NULL;

SET_UP_ERR:
    free(pcm_params);
MALLOC_ERR:
    _playback_line_in_stop();
    return NULL;
}

INT32 _playback_line_in_thread(VOID)
{
    INT32 i4_ret;
    RINGBUF_H h_ringbuf;
    pthread_attr_t t_attr;
    struct sched_param t_schedule;

    h_ringbuf = u_ringbuf_malloc(PLAYBACK_LINE_IN_SAMPLERATE * PLAYBACK_LINE_IN_CHANNLE * PLAYBACK_LINE_IN_BITWIDTH / 8);
    if (NULL == h_ringbuf)
    {
        LINE_IN_ERR("ringbuf malloc failed!\n");
        goto RINGBUF_ERR;
    }

    i4_ret = pthread_attr_init(&t_attr);
    if (0 != i4_ret)
    {
        LINE_IN_ERR("pthread_attr_init failed!\n");
        goto ATTR_INIT_ERR;
    }

    i4_ret = pthread_attr_setinheritsched(&t_attr, PTHREAD_EXPLICIT_SCHED);
    if (0 != i4_ret)
    {
        LINE_IN_ERR("pthread_attr_setinheritsched failed!\n");
        goto ATTR_SET_ERR;
    }

    //i4_ret = pthread_attr_setschedpolicy(&t_attr, SCHED_RR);
    if (0 != i4_ret)
    {
        LINE_IN_ERR("pthread_attr_setschedpolicy failed!\n");
        goto ATTR_SET_ERR;
    }

    t_schedule.sched_priority = 99;
    //i4_ret = pthread_attr_setschedparam(&t_attr, &t_schedule);
    if (0 != i4_ret)
    {
        LINE_IN_ERR("pthread_attr_setschedparam failed!\n");
        goto ATTR_SET_ERR;
    }

    _playback_line_in_play();

    i4_ret = pthread_create(&t_record, &t_attr, _playback_line_in_record_process, h_ringbuf);
    if (0 != i4_ret)
    {
        LINE_IN_ERR("line_in record thread create failed!\n");
        goto LINE_IN_THREAD_ERR;
    }

    i4_ret = pthread_create(&t_play, &t_attr, _playback_line_in_play_process, h_ringbuf);
    if (0 != i4_ret)
    {
        LINE_IN_ERR("line_in play thread create failed!\n");
        goto PLAY_THREAD_ERR;
    }

    pthread_join(t_record, NULL);
    pthread_join(t_play, NULL);

    pthread_attr_destroy(&t_attr);
    u_ringbuf_free(h_ringbuf);
    _playback_line_in_send_pb_status_to_sm(SM_BODY_STOP);
    return NULL;

PLAY_THREAD_ERR:
    pthread_cancel(t_record);
LINE_IN_THREAD_ERR:
ATTR_SET_ERR:
    pthread_attr_destroy(&t_attr);
ATTR_INIT_ERR:
    u_ringbuf_free(h_ringbuf);
RINGBUF_ERR:
    _playback_line_in_stop();
    _playback_line_in_send_pb_status_to_sm(SM_BODY_STOP);
    return NULL;
}

static INT32 _playback_line_in_create_thread(VOID)
{
    INT32 i4_ret;
    pthread_t ntid;
    pthread_attr_t t_attr;

    i4_ret = pthread_attr_init(&t_attr);
    if(0 != i4_ret)
    {
        LINE_IN_ERR("pthread_attr_init error!\n");
        goto ATTR_INIT_ERR;
    }

    i4_ret = pthread_attr_setdetachstate(&t_attr, PTHREAD_CREATE_DETACHED);
    if(0 != i4_ret)
    {
        LINE_IN_ERR("pthread_attr_setdetachstate error!\n");
        goto ATTR_SET_ERR;
    }

    i4_ret = pthread_create(&ntid, &t_attr, _playback_line_in_thread, NULL);
    if(0 != i4_ret)
    {
        LINE_IN_ERR("pthread_create error!\n");
        goto ATTR_SET_ERR;
    }

    pthread_attr_destroy(&t_attr);
    return PB_LINE_IN_OK;

ATTR_SET_ERR:
    pthread_attr_destroy(&t_attr);
ATTR_INIT_ERR:
    _playback_line_in_send_pb_status_to_sm(SM_BODY_STOP);
    return PB_LINE_IN_FAIL;
}

static INT32 _playback_line_in_init(
                    const CHAR*                 ps_name,
                    HANDLE_T                    h_app
                    )
{
    INT32 i4_ret;

    memset(&g_t_pb_line_in, 0, sizeof(PLAYBACK_LINE_IN_OBJ_T));
    g_t_pb_line_in.h_app = h_app;


    if (g_t_pb_line_in.b_app_init_ok)
    {
        return AEER_OK;
    }

#ifdef CLI_SUPPORT
    i4_ret = playback_line_in_cli_attach_cmd_tbl();
    if ((i4_ret != CLIR_NOT_INIT) && (i4_ret != CLIR_OK))
    {
        LINE_IN_ERR("Err: playback_line_in_cli_attach_cmd_tbl() failed, ret=%ld\r\n", i4_ret);
        return AEER_FAIL;
    }
#endif/* CLI_SUPPORT */

    g_t_pb_line_in.b_app_init_ok = TRUE;

    return AEER_OK;

}

static INT32 _playback_line_in_exit (
                    HANDLE_T                    h_app,
                    APP_EXIT_MODE_T             e_exit_mode
                    )
{
    if (FALSE == g_t_pb_line_in.b_app_init_ok)
    {
        return AEER_FAIL;
    }

    g_t_pb_line_in.b_app_init_ok = FALSE;

    return AEER_OK;
}

static INT32 _playback_line_in_send_pb_status_to_sm(SM_MSG_BDY_E PB_BODY)
{
    INT32 i4_ret = PB_LINE_IN_FAIL;
    APPMSG_T t_msg = {0};
    HANDLE_T h_app = NULL_HANDLE;

    t_msg.ui4_sender_id = MSG_FROM_LINEIN;

    t_msg.ui4_msg_type = SM_MAKE_MSG(SM_INFORM_GRP,PB_BODY,0);

    u_am_get_app_handle_from_name(&h_app,SM_THREAD_NAME);

    i4_ret = u_app_send_msg(h_app,
                            E_APP_MSG_TYPE_STATE_MNGR,
                            &t_msg,
                            sizeof(APPMSG_T),
                            NULL,
                            NULL);

    if (AEER_OK != i4_ret)
    {
        LINE_IN_ERR("_playback_line_in_send_pb_status_to_sm failed, i4_ret[%ld]\n",i4_ret);
    }
    LINE_IN_INFO("playback send %d status to sm!\n", PB_BODY);

    return (i4_ret == AEER_OK ? PB_LINE_IN_OK : PB_LINE_IN_FAIL);
}


static INT32 _playback_line_in_send_play_request_to_sm(void)
{
    INT32 i4_ret = PB_LINE_IN_FAIL;
    APPMSG_T t_msg = {0};
    HANDLE_T h_app = NULL_HANDLE;

    t_msg.ui4_sender_id = MSG_FROM_LINEIN;

    t_msg.ui4_msg_type = SM_MAKE_MSG(SM_REQUEST_GRP,SM_BODY_PLAY,0);

    u_am_get_app_handle_from_name(&h_app,SM_THREAD_NAME);

    i4_ret = u_app_send_msg(h_app,
                            E_APP_MSG_TYPE_STATE_MNGR,
                            &t_msg,
                            sizeof(APPMSG_T),
                            NULL,
                            NULL);

    if (AEER_OK != i4_ret)
    {
        LINE_IN_ERR("_playback_line_in_send_play_request_to_sm failed, i4_ret[%ld]\n", i4_ret);
    }

    return (i4_ret == AEER_OK ? PB_LINE_IN_OK : PB_LINE_IN_FAIL);
}

INT32 playback_line_in_alsa_msg_process(PB_LINE_IN_MSG_T t_pb_line_in_msg)
{
    INT32 i4_ret = PB_LINE_IN_OK;
    PB_LINE_IN_MSG_T t_alsa_msg = {0};

    LINE_IN_INFO("msg id [%d]\n", t_pb_line_in_msg.ui4_msg_id);

    switch (t_pb_line_in_msg.ui4_msg_id)
    {
        case LINE_IN_REQUEST_PLAY:
            _playback_line_in_send_play_request_to_sm();
            break;
        case LINE_IN_CREATE_THREAD:
            if(_playback_get_line_in_thread_running_flag())
            {
                LINE_IN_MSG("thread is running!!!\n");
                break;
            }
            _playback_line_in_create_thread();
            break;
        case LINE_IN_PLAY_PAUSE:
            if (_playback_get_line_in_thread_running_flag())
            {
                playback_set_line_in_thread_pause_flag(!g_t_pb_line_in.b_pause);
                _playback_line_in_send_pb_status_to_sm(g_t_pb_line_in.b_pause ? SM_BODY_PAUSE : SM_BODY_PLAY);
            }
            else
            {
                LINE_IN_ERR("thread not running!\n");
            }

            break;
        case LINE_IN_STOP:
            _playback_line_in_stop();
            break;
        default:
            break;
    }

    return AEER_OK;
}

INT32 playback_line_in_sm_msg_process(APPMSG_T* pv_msg)
{
    PB_LINE_IN_MSG_T pb_line_in_msg;
    memset(&pb_line_in_msg, 0x00, sizeof(PB_LINE_IN_MSG_T));

    switch(SM_MSG_GRP(pv_msg->ui4_msg_type))
    {
        case SM_REQUEST_GRP:
        case SM_PERMIT_GRP:
        {
            switch(SM_MSG_BDY(pv_msg->ui4_msg_type))
            {
                case SM_BODY_PLAY:
                    pb_line_in_msg.ui4_msg_id = LINE_IN_CREATE_THREAD;
                    u_playback_line_in_send_msg(&pb_line_in_msg);
                    break;
                case SM_BODY_PAUSE:
                    //pause
                    break;
                case SM_BODY_STOP:
                    pb_line_in_msg.ui4_msg_id = LINE_IN_STOP;
                    u_playback_line_in_send_msg(&pb_line_in_msg);
                    break;
                default:
                    break;
            }
        }
            break;
        case SM_FORBID_GRP:
            LINE_IN_ERR("forbid msg body[%d]\n", SM_MSG_BDY(pv_msg->ui4_msg_type));
            break;
        default:
            break;
    }
    return AEER_OK;
}

static INT32 _playback_line_in_process_msg (
                    HANDLE_T                    h_app,
                    UINT32                      ui4_type,
                    const VOID*                 pv_msg,
                    SIZE_T                      z_msg_len,
                    BOOL                        b_paused
                    )
{

    PB_LINE_IN_MSG_T   t_pb_line_in_msg = {0};

    if (FALSE == g_t_pb_line_in.b_app_init_ok) {
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
          case E_APP_PRI_ALSA_PB_MSG:
            {
                LINE_IN_INFO("E_APP_PRI_ALSA_PB_MSG\n");
                memcpy(&t_pb_line_in_msg,
                       (PB_LINE_IN_MSG_T*)pv_msg,
                       sizeof(PB_LINE_IN_MSG_T));

                playback_line_in_alsa_msg_process(t_pb_line_in_msg);
                break;
            }
            case E_APP_MSG_TYPE_USER_CMD:
            {
                u_playback_ui_msg_process(pv_msg, z_msg_len);
            }
            default:
            break;
        }

    }
    else
    {
        switch(ui4_type)
        {
            case E_APP_MSG_TYPE_STATE_MNGR:
            {
                LINE_IN_INFO("E_APP_MSG_TYPE_STATE_MNGR\n");
                playback_line_in_sm_msg_process((APPMSG_T *)pv_msg);
            }
            break;
            default:
            break;
        }
    }

    return AEER_OK;
}

INT32 u_playback_line_in_send_msg(const PB_LINE_IN_MSG_T* pt_event)
{
    INT32 i4_ret;

    if (FALSE == g_t_pb_line_in.b_app_init_ok)
    {
        return PB_LINE_IN_FAIL;
    }

    i4_ret = u_app_send_msg(g_t_pb_line_in.h_app,
                            E_APP_PRI_ALSA_PB_MSG,
                            pt_event,
                            sizeof(PB_LINE_IN_MSG_T),
                            NULL,
                            NULL);

    if (AEER_OK != i4_ret) {
        LINE_IN_ERR("_app_send_msg sent E_APP_PRI_ALSA_PB_MSG failed, i4_ret[%ld]\n",i4_ret);
    }

    return (i4_ret == AEER_OK ? PB_LINE_IN_OK : PB_LINE_IN_FAIL);
}

