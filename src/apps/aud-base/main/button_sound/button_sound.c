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


#include <stdio.h>
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

/* private */
#include "button_sound.h"
#include "button_sound_cli.h"

/* public */
#include "u_wav_parser.h"
#include "u_alsa_interface.h"
#include "u_acfg.h"

/*-----------------------------------------------------------------------------
                    macros, defines, typedefs, enums
-----------------------------------------------------------------------------*/
#define BTN_SND_FILE_DIR   "/data/sda1/"

#define BTN_SND_POWER_ON_FILE_PATH             BTN_SND_FILE_DIR"POWER_ON.wav"
#define BTN_SND_POWER_OFF_FILE_PATH            BTN_SND_FILE_DIR"POWER_OFF.wav"
#define BTN_SND_VOLUME_UP_FILE_PATH            BTN_SND_FILE_DIR"VOLUME_UP.wav"
#define BTN_SND_VOLUME_DOWN_FILE_PATH          BTN_SND_FILE_DIR"VOLUME_DOWN.wav"
#define BTN_SND_NEXT_TRACK_FILE_PATH           BTN_SND_FILE_DIR"NEXT_TRACK.wav"
#define BTN_SND_CHARGING_IN_DOCK_FILE_PATH     BTN_SND_FILE_DIR"CHARGING_IN_DOCK.wav"
#define BTN_SND_BATTERY_LOW_FILE_PATH          BTN_SND_FILE_DIR"BATTERY_LOW.wav"
#define BTN_SND_TAP_TO_PAIR_SUCCESS_FILE_PATH  BTN_SND_FILE_DIR"TAP_TO_PAIR_SUCCESS.wav"
#define BTN_SND_BT_WAITING_FILE_PATH           BTN_SND_FILE_DIR"BLUETOOTH_WAITING.wav"
#define BTN_SND_BT_CONNECTED_FILE_PATH         BTN_SND_FILE_DIR"BLUETOOTH_CONNECTED.wav"
#define BTN_SND_BT_DISCONNECTED_FILE_PATH      BTN_SND_FILE_DIR"BLUETOOTH_DISCONNECTED.wav"

typedef struct
{
    BTN_SND_TYPE_E e_snd;
    const CHAR* filepath;
}BTN_SND_PLAY_FILE_TYPE_T;

/*-----------------------------------------------------------------------------
 * extern variable declarations
 *---------------------------------------------------------------------------*/
extern void* g_mas_handle;
/*-----------------------------------------------------------------------------
 * variable declarations
 *---------------------------------------------------------------------------*/
PCMContainer_t g_buttonsound_pcm;
/*-----------------------------------------------------------------------------
 * static variable declarations
 *---------------------------------------------------------------------------*/
static BUTTON_SOUND_OBJ_T t_g_buttonsound   = {0};

static UINT16 ui2_g_button_sound_dbg_level = DBG_INIT_LEVEL_APP_BUTTON_SOUND;

static HANDLE_T  h_sema_bsound_stop = NULL_HANDLE;
static HANDLE_T  h_sema_bsound_switch = NULL_HANDLE;

static off64_t_b written = 0;
static off64_t_b count = 0;
static int load = 0;

static int g_fd_bsound = -1;

BTN_SND_PLAY_FILE_TYPE_T at_g_btn_snd_file_list[] = {
    {BTN_SND_POWER_ON,             BTN_SND_POWER_ON_FILE_PATH},
    {BTN_SND_POWER_OFF,            BTN_SND_POWER_OFF_FILE_PATH},
    {BTN_SND_VOLUME_UP,            BTN_SND_VOLUME_UP_FILE_PATH},
    {BTN_SND_VOLUME_DOWN,          BTN_SND_VOLUME_DOWN_FILE_PATH},
    {BTN_SND_NEXT_TRACK,           BTN_SND_NEXT_TRACK_FILE_PATH},
    {BTN_SND_CHARGING_IN_DOCK,     BTN_SND_CHARGING_IN_DOCK_FILE_PATH},
    {BTN_SND_BATTERY_LOW,          BTN_SND_BATTERY_LOW_FILE_PATH},
    {BTN_SND_TAP_TO_PAIR_SUCCESS,  BTN_SND_TAP_TO_PAIR_SUCCESS_FILE_PATH},
    {BTN_SND_BT_WAITING,           BTN_SND_BT_WAITING_FILE_PATH},
    {BTN_SND_BT_CONNECTED,         BTN_SND_BT_CONNECTED_FILE_PATH},
    {BTN_SND_BT_DISCONNECTED,      BTN_SND_BT_DISCONNECTED_FILE_PATH},
    {BTN_SND_MAX, NULL}
};
/*-----------------------------------------------------------------------------
 * private methods declarations
 *---------------------------------------------------------------------------*/
static INT32 _button_sound_init (
                    const CHAR*                 ps_name,
                    HANDLE_T                    h_app
                    );
static INT32 _button_sound_exit (
                    HANDLE_T                    h_app,
                    APP_EXIT_MODE_T             e_exit_mode
                    );
static INT32 _button_sound_process_msg (
                    HANDLE_T                    h_app,
                    UINT32                      ui4_type,
                    const VOID*                 pv_msg,
                    SIZE_T                      z_msg_len,
                    BOOL                        b_paused
                    );

INT32 button_sound_alsa_msg_process(BUTTON_SOUND_MSG_T  t_btn_sound_msg);

/*---------------------------------------------------------------------------
 * Name
 *      a_button_sound_register
 * Description      -
 * Input arguments  -
 * Output arguments -
 * Returns          -
 *---------------------------------------------------------------------------*/

VOID a_button_sound_register(
                AMB_REGISTER_INFO_T*            pt_reg
                )
{
    if (TRUE == t_g_buttonsound.b_app_init_ok) {
        return;
    }

    memset(pt_reg->s_name, 0, sizeof(CHAR)*(APP_NAME_MAX_LEN + 1));
    strncpy(pt_reg->s_name, BTN_SOUND_THREAD_NAME, APP_NAME_MAX_LEN);

    pt_reg->t_fct_tbl.pf_init                   = _button_sound_init;
    pt_reg->t_fct_tbl.pf_exit                   = _button_sound_exit;
    pt_reg->t_fct_tbl.pf_process_msg            = _button_sound_process_msg;
    pt_reg->t_desc.ui8_flags                    = ~((UINT64)0);
    pt_reg->t_desc.t_thread_desc.z_stack_size   = BTN_SOUND_STACK_SIZE;
    pt_reg->t_desc.t_thread_desc.ui1_priority   = BTN_SOUND_THREAD_PRIORITY;
    pt_reg->t_desc.t_thread_desc.ui2_num_msgs   = BTN_SOUND_NUM_MSGS;
    pt_reg->t_desc.ui2_msg_count                = BTN_SOUND_MSGS_COUNT;
    pt_reg->t_desc.ui2_max_msg_size             = BTN_SOUND_MAX_MSGS_SIZE;

    return;
}


UINT16 button_sound_get_dbg_level(VOID)
{
    return (ui2_g_button_sound_dbg_level | DBG_LAYER_APP);
}


VOID button_sound_set_dbg_level(UINT16 ui2_db_level)
{
    ui2_g_button_sound_dbg_level = ui2_db_level;
}


BOOL button_sound_get_pb_running_flag(void)
{
    return t_g_buttonsound.b_pb_running;
}

static void button_sound_set_pb_running_flag(BOOL flag)
{
    /*only set by main thread, so i think do not need add mutex to protect*/
    t_g_buttonsound.b_pb_running = flag;
}

BOOL button_sound_get_switch_flag(void)
{
    return t_g_buttonsound.b_isSwitch;
}

static void button_sound_set_switch_flag(BOOL flag)
{
    /*only set by main thread, so i think do not need add mutex to protect*/
    t_g_buttonsound.b_isSwitch = flag;
}


INT32 _button_sound_create_stop_sema(VOID)
{
    if (NULL_HANDLE == h_sema_bsound_stop)
    {
        if (u_sema_create(&h_sema_bsound_stop, X_SEMA_TYPE_BINARY, X_SEMA_STATE_LOCK)  != OSR_OK)
        {
            DBG_ERROR((BTNSOUND_TAG"ERR: _button_sound_create_stop_sema error\n"));
            return AEER_FAIL;
        }
    }
    return AEER_OK;
}


INT32 _button_sound_destroy_stop_sema(VOID)
{
    return u_sema_delete(h_sema_bsound_stop);
}


INT32 button_sound_lock_stop_sema(VOID)
{
    return u_sema_lock(h_sema_bsound_stop, X_SEMA_OPTION_WAIT);
}


INT32 button_sound_unlock_stop_sema(VOID)
{
    return u_sema_unlock(h_sema_bsound_stop);
}


INT32 _button_sound_create_switch_sema(VOID)
{
    if (NULL_HANDLE == h_sema_bsound_switch)
    {
        if (u_sema_create(&h_sema_bsound_switch, X_SEMA_TYPE_BINARY, X_SEMA_STATE_UNLOCK)  != OSR_OK)
        {
            DBG_ERROR((BTNSOUND_TAG"ERR: _button_sound_create_switch_sema error\n"));
            return AEER_FAIL;
        }
    }
    return AEER_OK;
}


INT32 _button_sound_destroy_switch_sema(VOID)
{
    return u_sema_delete(h_sema_bsound_switch);
}


INT32 button_sound_lock_switch_sema(VOID)
{
    return u_sema_lock(h_sema_bsound_switch, X_SEMA_OPTION_WAIT);
}


INT32 button_sound_unlock_switch_sema(VOID)
{
    return u_sema_unlock(h_sema_bsound_switch);
}


VOID button_sound_stop_pcm_playback(VOID)
{
    if(NULL != g_buttonsound_pcm.handle)
        snd_pcm_drain(g_buttonsound_pcm.handle);

    if(g_fd_bsound > 0)
    {
        close(g_fd_bsound);
        g_fd_bsound = -1;
    }

    if(NULL != g_buttonsound_pcm.data_buf)
    {
        free(g_buttonsound_pcm.data_buf);
        g_buttonsound_pcm.data_buf = NULL;
    }

    if(NULL != g_buttonsound_pcm.handle)
    {
        snd_pcm_close(g_buttonsound_pcm.handle);
        g_buttonsound_pcm.handle = NULL;
    }
}


VOID button_sound_reset_next_wav_params(VOID)
{
    load = 0;
    written = 0;
    count = LE_INT(g_buttonsound_pcm.chunk_count);

    if(count > 0)
    {
        button_sound_set_pb_running_flag(TRUE);
    }
    else
    {
        button_sound_set_pb_running_flag(FALSE);
    }
}

static INT32 _button_sound_init (
                    const CHAR*                 ps_name,
                    HANDLE_T                    h_app
                    )
{
    INT32 i4_ret;

    memset(&t_g_buttonsound, 0, sizeof(BUTTON_SOUND_OBJ_T));
    t_g_buttonsound.h_app = h_app;


    if (t_g_buttonsound.b_app_init_ok)
    {
        return AEER_OK;
    }

#ifdef CLI_SUPPORT
    i4_ret = button_sound_cli_attach_cmd_tbl();
    if ((i4_ret != CLIR_NOT_INIT) && (i4_ret != CLIR_OK))
    {
        DBG_ERROR((BTNSOUND_TAG"Err: button_sound_cli_attach_cmd_tbl() failed, ret=%ld\r\n",
            i4_ret ));
        return AEER_FAIL;
    }
    button_sound_set_dbg_level(DBG_INIT_LEVEL_APP_BUTTON_SOUND);
#endif/* CLI_SUPPORT */

    _button_sound_create_stop_sema();
    _button_sound_create_switch_sema();

    t_g_buttonsound.b_app_init_ok = TRUE;

    return AEER_OK;

}

static INT32 _button_sound_exit (
                    HANDLE_T                    h_app,
                    APP_EXIT_MODE_T             e_exit_mode
                    )
{
    if (FALSE == t_g_buttonsound.b_app_init_ok)
    {
        return AEER_FAIL;
    }

    _button_sound_destroy_stop_sema();
    _button_sound_destroy_switch_sema();

    t_g_buttonsound.b_app_init_ok = FALSE;

    return AEER_OK;
}


ssize_t _button_sound_saferead(int fd, void *buf, size_t count)
{
    ssize_t result = 0, res;

    while (count > 0) {
        if ((res = read(fd, buf, count)) == 0)
            break;
        if (res < 0)
            return result > 0 ? result : res;
        count -= res;
        result += res;
        buf = (char *)buf + res;
    }
    return result;
}


void _button_sound_alsa_playback_thread(void)
{
    UINT8 ui1_volume;
    int ret;
    off64_t_b c;

    load = 0;
    written = 0;
    count = LE_INT(g_buttonsound_pcm.chunk_count);

    if(count > 0)
    {
        button_sound_set_pb_running_flag(TRUE);
    }
    else
    {
        button_sound_set_pb_running_flag(FALSE);
    }

    DBG_ERROR((BTNSOUND_TAG"%s pid is [%d] ,tid is [%d]\n",__FUNCTION__,getpid(),syscall(SYS_gettid)));

    while (TRUE == button_sound_get_pb_running_flag())
    {
        if((TRUE == button_sound_get_switch_flag()) || (written >= count))
        {
            sleep(1);/*need to sleep for wait for resume*/
            continue;
        }

        button_sound_lock_switch_sema();

        /* Must read [chunk_bytes] bytes data enough. */
        do {
            c = count - written;
            if (c > g_buttonsound_pcm.chunk_bytes)
                c = g_buttonsound_pcm.chunk_bytes;
            c -= load;

            if (c == 0)
                break;
            ret = _button_sound_saferead(g_fd_bsound, g_buttonsound_pcm.data_buf + load, c);
            if (ret < 0) {
                DBG_ERROR((BTNSOUND_TAG"Error _button_sound_saferead\n"));
                break;
            }
            if (ret == 0)
                break;
            load += ret;
        } while ((size_t)load < g_buttonsound_pcm.chunk_bytes);

        /* Transfer to size frame */
        load = load * 8 / g_buttonsound_pcm.bits_per_frame;
        ret = u_alsa_write_pcm(&g_buttonsound_pcm, load);

        button_sound_unlock_switch_sema();

        if(ret == -1)
        {
            continue;/*do not exit, because when pause, call pcm drop here will return -1*/
        }

        if (ret != load && ret != g_buttonsound_pcm.chunk_size)
            DBG_ERROR((BTNSOUND_TAG"Error write pcm frame number is %d but load is %d\n",ret,load));

        ret = ret * g_buttonsound_pcm.bits_per_frame / 8;
        written += ret;
        load = 0;

        if(written == g_buttonsound_pcm.chunk_bytes)
        {
            u_acfg_get_volume(&ui1_volume);
        }

        if(written >= count)
        {
            //means playback finished
            button_sound_stop_pcm_playback();
            t_g_buttonsound.b_play_done = TRUE;
        }
    }

    button_sound_stop_pcm_playback();

    DBG_ERROR((BTNSOUND_TAG"button sound wav file finished\n"));

    button_sound_unlock_stop_sema();

    pthread_exit(NULL);
}


static int _buttton_sound_parser_file(BTN_SND_TYPE_E play_type)
{
    int ret = BUTTON_SOUND_FAIL;

    if(play_type >= BTN_SND_MAX)
    {
        return BUTTON_SOUND_FAIL;
    }

    DBG_ERROR((BTNSOUND_TAG"set filename is %s\n",at_g_btn_snd_file_list[play_type].filepath));

    g_fd_bsound = open(at_g_btn_snd_file_list[play_type].filepath, O_RDONLY);

    if (g_fd_bsound < 0)
    {
        DBG_ERROR((BTNSOUND_TAG"open file %s fail\n",at_g_btn_snd_file_list[play_type].filepath));
        return BUTTON_SOUND_FAIL;
    }
    else
    {
        DBG_INFO((BTNSOUND_TAG"open file %s ok\n",at_g_btn_snd_file_list[play_type].filepath));
    }

    if (u_wave_file_get_pcm_container(g_fd_bsound,&g_buttonsound_pcm) < 0)
    {
        DBG_ERROR((BTNSOUND_TAG"parser usb wav file %s fail\n",at_g_btn_snd_file_list[play_type].filepath));
        ret = BUTTON_SOUND_FAIL;
    }
    else
    {
        DBG_ERROR((BTNSOUND_TAG"parser usb wav file %s ok\n",at_g_btn_snd_file_list[play_type].filepath));
        ret = BUTTON_SOUND_OK;
    }
    return ret;
}


static INT32 _button_sound_process_msg (
                    HANDLE_T                    h_app,
                    UINT32                      ui4_type,
                    const VOID*                 pv_msg,
                    SIZE_T                      z_msg_len,
                    BOOL                        b_paused
                    )
{

    BUTTON_SOUND_MSG_T   t_btn_sound_msg = {0};


    if (FALSE == t_g_buttonsound.b_app_init_ok) {
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
            case E_APP_MSG_TYPE_BUTTON:
            {
                DBG_ERROR((BTNSOUND_TAG"E_APP_MSG_TYPE_BUTTON\n"));
                memcpy(
                         &t_btn_sound_msg,
                         (BUTTON_SOUND_MSG_T*)pv_msg,
                         sizeof(BUTTON_SOUND_MSG_T));

                button_sound_alsa_msg_process(t_btn_sound_msg);
                break;
            }
            default:
            break;
        }

    }
    else
    {
        switch(ui4_type)
        {
            case E_APP_MSG_TYPE_DUT_STATE:
            {
                // should get power on/off msg here
            }
            break;

            case E_APP_MSG_TYPE_USB_DEV:
            {
                // should get usb plug in /out, file system mount/unmount msg.
            }
            break;

            default:
            break;
        }
    }

    return AEER_OK;
}


/*-----------------------------------------------------------------------------
 * Name: button_sound_alsa_msg_process()
 *
 * Description:button sound playback msg process function
 *
 * Input:
 *
 * Output :
 *
 * Returns: 0                          Routine successful.
 *              Any other value     Routine failed.
 *----------------------------------------------------------------------------*/
INT32 button_sound_alsa_msg_process(BUTTON_SOUND_MSG_T  t_btn_sound_msg)
{
    INT32 ret = BUTTON_SOUND_OK;
    BUTTON_SOUND_MSG_T t_alsa_msg = {0};

    DBG_ERROR((BTNSOUND_TAG"%s is called, msg id is %d\n", __FUNCTION__,t_btn_sound_msg.ui4_msg_id));

    switch (t_btn_sound_msg.ui4_msg_id)
    {
        case BTN_SOUND_PB_PLAY:
        {
            DBG_INFO((BTNSOUND_TAG"BTN_SOUND_PB_PLAY at line %d\n",__LINE__));
            if(FALSE == button_sound_get_pb_running_flag())
            {
                memset(&t_alsa_msg,0,sizeof(BUTTON_SOUND_MSG_T));
                t_alsa_msg.ui4_msg_id = BTN_SOUND_PB_PARSER_FILE;
                t_alsa_msg.ui4_data1  = t_btn_sound_msg.ui4_data1;
                ret = u_button_sound_send_msg(&t_alsa_msg);
            }
            else
            {
                button_sound_set_switch_flag(TRUE);

                button_sound_lock_switch_sema();

                button_sound_stop_pcm_playback();

                memset(&t_alsa_msg,0,sizeof(BUTTON_SOUND_MSG_T));
                t_alsa_msg.ui4_msg_id = BTN_SOUND_PB_PARSER_FILE;
                t_alsa_msg.ui4_data1  = t_btn_sound_msg.ui4_data1;
                ret = u_button_sound_send_msg(&t_alsa_msg);

                button_sound_unlock_switch_sema();
            }
        }
        break;

        case BTN_SOUND_PB_PARSER_FILE:
        {
            DBG_INFO((BTNSOUND_TAG"BTN_SOUND_PB_PARSER_FILE at line %d\n",__LINE__));
            memset(&t_alsa_msg,0,sizeof(BUTTON_SOUND_MSG_T));

            ret = _buttton_sound_parser_file(t_btn_sound_msg.ui4_data1);

            if(ret == BUTTON_SOUND_OK)
            {
                t_alsa_msg.ui4_msg_id = BTN_SOUND_PB_SET_HW_PARAMS;
                ret = u_button_sound_send_msg(&t_alsa_msg);
            }

        }
        break;

        case BTN_SOUND_PB_SET_HW_PARAMS:
        {
            DBG_INFO((BTNSOUND_TAG"BTN_SOUND_PB_SET_HW_PARAMS at line %d\n",__LINE__));
            memset(&t_alsa_msg,0,sizeof(BUTTON_SOUND_MSG_T));

            if (snd_pcm_open(&g_buttonsound_pcm.handle,BUTTON_SOUND_PCM_DEVICE_NAME,SND_PCM_STREAM_PLAYBACK,0) < 0)
            {
                DBG_ERROR((BTNSOUND_TAG"snd_pcm_open handle %s fail\n",BUTTON_SOUND_PCM_DEVICE_NAME));
                ret = BUTTON_SOUND_FAIL;
            }
            else
            {
                DBG_ERROR((BTNSOUND_TAG"snd_pcm_open handle %s ok\n",BUTTON_SOUND_PCM_DEVICE_NAME));
                if (u_alsa_set_hw_params(&g_buttonsound_pcm, 200000) < 0)
                {
                    DBG_ERROR((BTNSOUND_TAG"set alsa hw params fail\n"));
                    ret = BUTTON_SOUND_FAIL;
                }
                else
                {
                    DBG_ERROR((BTNSOUND_TAG"set alsa hw params ok\n"));
                    if(TRUE == button_sound_get_pb_running_flag())
                    {
                        button_sound_reset_next_wav_params();
                        button_sound_set_switch_flag(FALSE);
                    }
                    else
                    {
                        t_alsa_msg.ui4_msg_id = BTN_SOUND_PB_CREATE_PB_THREAD;
                        ret = u_button_sound_send_msg(&t_alsa_msg);
                    }
                }
            }
        }
        break;

        case BTN_SOUND_PB_CREATE_PB_THREAD:
        {
            DBG_INFO((BTNSOUND_TAG"BTN_SOUND_PB_CREATE_PB_THREAD at line %d\n",__LINE__));
            pthread_t btn_play_thread;
            pthread_attr_t attr;
            pthread_attr_init(&attr);
            /*set detached system will auto release resource*/
            pthread_attr_setdetachstate(&attr,PTHREAD_CREATE_DETACHED);
            pthread_create(&btn_play_thread,&attr,(void *)_button_sound_alsa_playback_thread,NULL);
            DBG_ERROR((BTNSOUND_TAG"create alsa playback thread finished,pthread_t id is %u\n",btn_play_thread));
        }
        break;


        case BTN_SOUND_PB_STOP:
        {
            DBG_INFO((BTNSOUND_TAG"BTN_SOUND_PB_STOP at line %d\n",__LINE__));

            if(TRUE == button_sound_get_pb_running_flag())
            {
                DBG_ERROR((BTNSOUND_TAG"alsa stop starting\n"));
                button_sound_set_pb_running_flag(FALSE);
                button_sound_lock_stop_sema();
            }

            DBG_ERROR((BTNSOUND_TAG"alsa stop finished\n"));
        }
        break;

        default:
        break;
    }

    return ret;
}


/*-------------------------------------------------------------------------
 * Name
 *      u_button_sound_send_msg
 * Description      -
 * Input arguments  -
 * Output arguments -
 * Returns          -
 *------------------------------------------------------------------------*/
INT32 u_button_sound_send_msg(const BUTTON_SOUND_MSG_T* pt_event)
{
    INT32 i4_ret;

    if (FALSE == t_g_buttonsound.b_app_init_ok)
    {
        return BUTTON_SOUND_FAIL;
    }

    i4_ret = u_app_send_msg(t_g_buttonsound.h_app,
                            E_APP_MSG_TYPE_BUTTON,
                            pt_event,
                            sizeof(BUTTON_SOUND_MSG_T),
                            NULL,
                            NULL);

    if (AEER_OK != i4_ret) {
        DBG_ERROR((BTNSOUND_TAG"u_button_sound_send_msg sent msg failed, i4_ret[%ld]\n",i4_ret));
    }

    return (i4_ret == AEER_OK ? BUTTON_SOUND_OK : BUTTON_SOUND_FAIL);
}


/*-------------------------------------------------------------------------
 * Name
 *      u_button_sound_play
 * Description      -
 * Input arguments  - BTN_SND_TYPE_E
 * Output arguments - NA
 * Returns          -
 *------------------------------------------------------------------------*/
INT32 u_button_sound_play(BTN_SND_TYPE_E play_type)
{
    INT32 i4_ret;
    BUTTON_SOUND_MSG_T snd_msg;

    if (FALSE == t_g_buttonsound.b_app_init_ok)
    {
        return BUTTON_SOUND_FAIL;
    }

    snd_msg.ui4_msg_id = BTN_SOUND_PB_PLAY;
    snd_msg.ui4_data1  = play_type;
    snd_msg.ui4_data2  = 0;
    snd_msg.ui4_data3  = 0;

    i4_ret = u_app_send_msg(t_g_buttonsound.h_app,
                            E_APP_MSG_TYPE_BUTTON,
                            &snd_msg,
                            sizeof(BUTTON_SOUND_MSG_T),
                            NULL,
                            NULL);

    if (AEER_OK != i4_ret) {
        DBG_ERROR((BTNSOUND_TAG"u_button_sound_play sent msg failed, i4_ret[%ld]\n",i4_ret));
    }

    return (i4_ret == AEER_OK ? BUTTON_SOUND_OK : BUTTON_SOUND_FAIL);
}

