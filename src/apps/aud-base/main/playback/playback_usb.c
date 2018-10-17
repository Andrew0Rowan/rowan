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
#include "u_sm.h"
#include "u_dm.h"

/* private */
#include "playback_usb.h"
#include "playback_usb_cli.h"

/* public */
#include "u_wav_parser.h"
#include "u_alsa_interface.h"
#include "u_key_def.h"
#include "u_acfg.h"

/*-----------------------------------------------------------------------------
 * structure, constants, macro definitions
 *---------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------
 * variable declarations
 *---------------------------------------------------------------------------*/
Wav_Play_List *p_playlist = NULL;
Wav_Play_List *g_pt_playlist_head = NULL;
PCMContainer_t g_playback_pcm;
/*-----------------------------------------------------------------------------
 * static variable declarations
 *---------------------------------------------------------------------------*/
static PLAYBACK_USB_OBJ_T t_g_pb_usb    = {0};

static BOOL   g_b_usb_auto_play = FALSE;
static UINT16 ui2_g_playback_usb_dbg_level = DBG_INIT_LEVEL_APP_PLAYBACK_USB;

static char  filename[MAX_WAV_FILE_LEN] = {0};

static HANDLE_T  h_sema_usb_pb = NULL_HANDLE;
static HANDLE_T  h_sema_usb_switch = NULL_HANDLE;

static off64_t_b written = 0;
static off64_t_b count = 0;
static int load = 0;

static int g_fd = -1;
/*-----------------------------------------------------------------------------
 * private methods declarations
 *---------------------------------------------------------------------------*/
static INT32 _playback_usb_init (
                    const CHAR*                 ps_name,
                    HANDLE_T                    h_app
                    );
static INT32 _playback_usb_exit (
                    HANDLE_T                    h_app,
                    APP_EXIT_MODE_T             e_exit_mode
                    );
static INT32 _playback_usb_process_msg (
                    HANDLE_T                    h_app,
                    UINT32                      ui4_type,
                    const VOID*                 pv_msg,
                    SIZE_T                      z_msg_len,
                    BOOL                        b_paused
                    );

INT32 playback_usb_alsa_msg_process(PB_USB_MSG_T  t_pb_usb_msg);

static INT32 _playback_usb_send_pb_status_to_sm(SM_MSG_BDY_E PB_BODY);
static INT32 _playback_usb_send_play_request_to_sm(void);
SM_MSG_BDY_E u_playback_usb_get_playback_status(void);
/*---------------------------------------------------------------------------
 * Name
 *      a_playback_usb_register
 * Description      -
 * Input arguments  -
 * Output arguments -
 * Returns          -
 *---------------------------------------------------------------------------*/

VOID a_playback_usb_register(
                AMB_REGISTER_INFO_T*            pt_reg
                )
{
    if (TRUE == t_g_pb_usb.b_app_init_ok) {
        return;
    }

    memset(pt_reg->s_name, 0, sizeof(CHAR)*(APP_NAME_MAX_LEN + 1));
    strncpy(pt_reg->s_name, PB_USB_THREAD_NAME, APP_NAME_MAX_LEN);

    pt_reg->t_fct_tbl.pf_init                   = _playback_usb_init;
    pt_reg->t_fct_tbl.pf_exit                   = _playback_usb_exit;
    pt_reg->t_fct_tbl.pf_process_msg            = _playback_usb_process_msg;
    pt_reg->t_desc.ui8_flags                    = ~((UINT64)0);
    pt_reg->t_desc.t_thread_desc.z_stack_size   = PB_USB_STACK_SIZE;
    pt_reg->t_desc.t_thread_desc.ui1_priority   = PB_USB_THREAD_PRIORITY;
    pt_reg->t_desc.t_thread_desc.ui2_num_msgs   = PB_USB_NUM_MSGS;
    pt_reg->t_desc.ui2_msg_count                = PB_USB_MSGS_COUNT;
    pt_reg->t_desc.ui2_max_msg_size             = PB_MAX_MSGS_SIZE;

    return;
}


UINT16 playback_usb_get_dbg_level(VOID)
{
    return (ui2_g_playback_usb_dbg_level | DBG_LAYER_APP);
}

VOID playback_usb_set_dbg_level(UINT16 ui2_db_level)
{
    ui2_g_playback_usb_dbg_level = ui2_db_level;
}

BOOL playback_usb_get_auto_play(VOID)
{
    return g_b_usb_auto_play;
}

VOID playback_usb_set_auto_play(BOOL b_auto_play)
{
    g_b_usb_auto_play = b_auto_play;
}

BOOL playback_usb_get_pause_flag(void)
{
    return t_g_pb_usb.b_isPause;
}

static void playback_usb_set_pause_flag(BOOL flag)
{
    /*only set by main thread, so i think do not need add mutex to protect*/
    t_g_pb_usb.b_isPause = flag;
}

BOOL playback_usb_get_pb_thread_running_flag(void)
{
    return t_g_pb_usb.b_pb_thread_running;
}

static void playback_usb_set_pb_thread_running_flag(BOOL flag)
{
    /*only set by main thread, so i think do not need add mutex to protect*/
    t_g_pb_usb.b_pb_thread_running = flag;
}

BOOL playback_usb_get_switch_flag(void)
{
    return t_g_pb_usb.b_isSwitch;
}

static void playback_usb_set_switch_flag(BOOL flag)
{
    /*only set by main thread, so i think do not need add mutex to protect*/
    t_g_pb_usb.b_isSwitch = flag;
}


INT32 _playback_usb_create_pb_sema(VOID)
{
    if (NULL_HANDLE == h_sema_usb_pb)
    {
        if (u_sema_create(&h_sema_usb_pb, X_SEMA_TYPE_BINARY, X_SEMA_STATE_LOCK)  != OSR_OK)
        {
            DBG_ERROR((PBUSB_TAG"ERR: _playback_usb_create_sema error\n"));
            return AEER_FAIL;
        }
    }
    return AEER_OK;
}


INT32 _playback_usb_destroy_pb_sema(VOID)
{
    return u_sema_delete(h_sema_usb_pb);
}


INT32 playback_usb_lock_pb_sema(VOID)
{
    return u_sema_lock(h_sema_usb_pb, X_SEMA_OPTION_WAIT);
}


INT32 playback_usb_unlock_pb_sema(VOID)
{
    return u_sema_unlock(h_sema_usb_pb);
}


INT32 _playback_usb_create_switch_sema(VOID)
{
    if (NULL_HANDLE == h_sema_usb_switch)
    {
        if (u_sema_create(&h_sema_usb_switch, X_SEMA_TYPE_BINARY, X_SEMA_STATE_UNLOCK)  != OSR_OK)
        {
            DBG_ERROR((PBUSB_TAG"ERR: _playback_usb_create_switch_sema error\n"));
            return AEER_FAIL;
        }
    }
    return AEER_OK;
}


INT32 _playback_usb_destroy_switch_sema(VOID)
{
    return u_sema_delete(h_sema_usb_switch);
}


INT32 playback_usb_lock_switch_sema(VOID)
{
    return u_sema_lock(h_sema_usb_switch, X_SEMA_OPTION_WAIT);
}


INT32 playback_usb_unlock_switch_sema(VOID)
{
    return u_sema_unlock(h_sema_usb_switch);
}


VOID _playback_usb_continue_playback_next_wav(VOID)
{
    PB_USB_MSG_T t_alsa_msg = {0};

    playback_usb_set_switch_flag(TRUE);

    memset(&t_alsa_msg,0,sizeof(PB_USB_MSG_T));
    t_alsa_msg.ui4_msg_id = ALSA_USB_PB_NEXT;
    u_playback_usb_send_msg(&t_alsa_msg);
}


VOID playback_usb_stop_pcm_playback(VOID)
{
    if(NULL != g_playback_pcm.handle)
        snd_pcm_drain(g_playback_pcm.handle);

    if(g_fd > 0)
    {
        close(g_fd);
        g_fd = -1;
    }

    if(NULL != g_playback_pcm.data_buf)
    {
        free(g_playback_pcm.data_buf);
        g_playback_pcm.data_buf = NULL;
    }

    if(NULL != g_playback_pcm.handle)
    {
        snd_pcm_close(g_playback_pcm.handle);
        g_playback_pcm.handle = NULL;
    }
}


VOID playback_usb_reset_next_wav_params(VOID)
{
    load = 0;
    written = 0;
    count = LE_INT(g_playback_pcm.chunk_count);

    if(count > 0)
    {
        playback_usb_set_pb_thread_running_flag(TRUE);
        playback_usb_set_pause_flag(FALSE);
        //to be done,notify play msg to ss
    }
    else
    {
        playback_usb_set_pb_thread_running_flag(FALSE);
        playback_usb_set_pause_flag(TRUE);
    }
}

static INT32 _playback_usb_init (
                    const CHAR*                 ps_name,
                    HANDLE_T                    h_app
                    )
{
    INT32 i4_ret;

    memset(&t_g_pb_usb, 0, sizeof(PLAYBACK_USB_OBJ_T));
    t_g_pb_usb.h_app = h_app;


    if (t_g_pb_usb.b_app_init_ok)
    {
        return AEER_OK;
    }

#ifdef CLI_SUPPORT
    i4_ret = playback_usb_cli_attach_cmd_tbl();
    if ((i4_ret != CLIR_NOT_INIT) && (i4_ret != CLIR_OK))
    {
        DBG_ERROR((PBUSB_TAG"Err: playback_usb_cli_attach_cmd_tbl() failed, ret=%ld\r\n",
            i4_ret ));
        return AEER_FAIL;
    }
    playback_usb_set_dbg_level(DBG_INIT_LEVEL_APP_PLAYBACK_USB);
#endif/* CLI_SUPPORT */

    _playback_usb_create_pb_sema();
    _playback_usb_create_switch_sema();

    t_g_pb_usb.b_app_init_ok = TRUE;

    return AEER_OK;

}

static INT32 _playback_usb_exit (
                    HANDLE_T                    h_app,
                    APP_EXIT_MODE_T             e_exit_mode
                    )
{
    if (FALSE == t_g_pb_usb.b_app_init_ok)
    {
        return AEER_FAIL;
    }

    _playback_usb_destroy_pb_sema();
    _playback_usb_destroy_switch_sema();

    t_g_pb_usb.b_app_init_ok = FALSE;

    return AEER_OK;
}


ssize_t _playback_usb_saferead(int fd, void *buf, size_t count)
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


void _playback_usb_alsa_playback_thread(void)
{
    int ret;
    off64_t_b c;

    load = 0;
    written = 0;
    count = LE_INT(g_playback_pcm.chunk_count);

    if(count > 0)
    {
        playback_usb_set_pb_thread_running_flag(TRUE);
        playback_usb_set_pause_flag(FALSE);
        //to be done,notify play msg to ss
    }
    else
    {
        playback_usb_set_pb_thread_running_flag(FALSE);
        playback_usb_set_pause_flag(TRUE);
    }

    DBG_ERROR((PBUSB_TAG"%s pid is [%d] ,tid is [%d]\n",__FUNCTION__,getpid(),syscall(SYS_gettid)));

    while (TRUE == playback_usb_get_pb_thread_running_flag())
    {
        if(TRUE == playback_usb_get_pause_flag() || TRUE == playback_usb_get_switch_flag())
        {
            sleep(1);/*need to sleep for wait for resume*/
            continue;
        }

        if(written >= count)
        {
            /*current wav playback finish, auto switch to next wav*/
            _playback_usb_continue_playback_next_wav();
            continue;
        }

        playback_usb_lock_switch_sema();

        /* Must read [chunk_bytes] bytes data enough. */
        do {
            c = count - written;
            if (c > g_playback_pcm.chunk_bytes)
                c = g_playback_pcm.chunk_bytes;
            c -= load;

            if (c == 0)
                break;
            ret = _playback_usb_saferead(g_fd, g_playback_pcm.data_buf + load, c);
            if (ret < 0) {
                DBG_ERROR((PBUSB_TAG"Error _playback_usb_saferead\n"));
                break;
            }
            if (ret == 0)
                break;
            load += ret;
        } while ((size_t)load < g_playback_pcm.chunk_bytes);

        /* Transfer to size frame */
        load = load * 8 / g_playback_pcm.bits_per_frame;
        ret = u_alsa_write_pcm(&g_playback_pcm, load);

        playback_usb_unlock_switch_sema();

        if((ret == -1) && (TRUE == playback_usb_get_pause_flag()))
        {
            continue;/*do not exit, because when pause, call pcm drop here will return -1*/
        }

        if (ret != load && ret != g_playback_pcm.chunk_size)
            DBG_ERROR((PBUSB_TAG"Error write pcm frame number is %d but load is %d\n",ret,load));

        ret = ret * g_playback_pcm.bits_per_frame / 8;
        written += ret;

        if(written == g_playback_pcm.chunk_bytes)
        {
            //to be done, send pb start msg to ss
            _playback_usb_send_pb_status_to_sm(SM_BODY_PLAY);
        }
        load = 0;
    }

    playback_usb_stop_pcm_playback();

    DBG_ERROR((PBUSB_TAG"playback_usb wav file finished\n"));

    playback_usb_unlock_pb_sema();

    pthread_exit(NULL);
}


static void _playback_usb_show_wav_list(Wav_Play_List *head)
{
    Wav_Play_List *p,*s;
    p = head;

    while(p->pre != NULL)
    {
        p = p->pre;
    }/*go to head postion*/

    while(p)
    {
        DBG_ERROR((PBUSB_TAG"index is [%d] : %s \n",p->index,p->data));
        s = p;
        p = p->next;
    }
}


static void _playback_usb_delete_wav_play_list(Wav_Play_List **pp_list)
{
    Wav_Play_List *p_list,*temp;
    p_list = *pp_list;

    while(p_list->pre != NULL)
    {
        p_list = p_list->pre;
    }/*go to head postion*/

    while(p_list != NULL)
    {
        DBG_ERROR((PBUSB_TAG"free data %s\n",p_list->data));
        free(p_list->data);
        p_list->data = NULL;
        p_list->pre  = NULL;
        temp = p_list->next;
        p_list->next  = NULL;
        free(p_list);
        p_list = temp;
    }
    *pp_list = NULL;
}


static Wav_Play_List *_playback_usb_create_wav_play_list(UINT32 *pfile_num)
{
    Wav_Play_List *head,*newnode,*p_node;

    char tar[MAX_WAV_FILE_LEN];
    FILE * fp;
    char *ch;
    int filenum = 0;

    /*create a head node firstly*/
    head = (Wav_Play_List *)malloc(sizeof(Wav_Play_List));
    head->index = 0;
    head->next = NULL;
    head->pre  = NULL;

    p_node = head;


    fp = fopen(PLAYLIST_PATH, "r");
    if(fp < 0)
    {
        free(head);
        head = NULL;
        DBG_ERROR((PBUSB_TAG"USB Wav Playback List create fail at line\n",__LINE__));
    }

    while(fgets(tar,MAX_WAV_FILE_LEN,fp)!=NULL)
    {
        ch = strtok(tar,"\n");
        DBG_INFO((PBUSB_TAG"%s,str length is %d\n",ch,strlen(ch)+1));
        if(0 == filenum)
        {
            /*cp head node data*/
            p_node->data = (char *)malloc(strlen(ch)+1);
            strcpy(p_node->data,ch);
            p_node->index = filenum + 1;
        }
        else
        {
            newnode = (Wav_Play_List*)malloc(sizeof(Wav_Play_List));
            newnode->data = (char *)malloc(strlen(ch)+1);
            strcpy(newnode->data,ch);
            newnode->index = filenum + 1;

            p_node->next = newnode;
            newnode->pre = p_node;
            newnode->next = NULL;

            p_node = p_node->next;
        }
        filenum ++;
    }
    *pfile_num = filenum;
    DBG_ERROR((PBUSB_TAG"USB Wav Playback List Length is %d\n",filenum));
    if(0 == filenum)
    {
        free(head);
        head = NULL;
        DBG_ERROR((PBUSB_TAG"USB Wav Playback List create fail at line\n",__LINE__));
    }
    fclose(fp);
    return head;
}



static INT32 _playback_usb_parser_file(void)
{
    int ret = PB_USB_FAIL;

    strcpy(filename,p_playlist->data);

    t_g_pb_usb.cur_file_index = p_playlist->index;

    DBG_ERROR((PBUSB_TAG"set filename is %s\n",filename));

    g_fd = open(filename, O_RDONLY);

    if (g_fd < 0)
    {
        DBG_ERROR((PBUSB_TAG"open file %s fail\n",filename));
        return PB_USB_FAIL;
    }
    else
    {
        DBG_INFO((PBUSB_TAG"open file %s ok\n",filename));
    }

    if (u_wave_file_get_pcm_container(g_fd,&g_playback_pcm) < 0)
    {
        DBG_ERROR((PBUSB_TAG"parser usb wav file %s fail\n",filename));
        ret = PB_USB_INV_ARG;
    }
    else
    {
        DBG_ERROR((PBUSB_TAG"parser usb wav file %s ok\n",filename));
        ret = PB_USB_OK;
    }
    return ret;
}

static INT32 _playback_usb_update_wav_file(VOID)
{
    INT32 i4_ret;
    SIZE_T i4_len;
    FM_MNT_INFO_T t_mnt_info;
    CHAR *ps_find_cmd = "find ";
    CHAR *ps_file_cmd = " -maxdepth 1 -name \"*.wav\" > /tmp/wav.list";
    CHAR *ps_cmd = NULL;

    i4_ret = u_dm_get_mnt_info(&t_mnt_info);
    if (DMR_OK != i4_ret)
    {
        DBG_ERROR((PBUSB_TAG"get mnt info failed(ret:%d)!\n", i4_ret));
        return PB_USB_FAIL;
    }

    i4_len = strlen(t_mnt_info.s_mnt_path) + strlen(ps_find_cmd) + strlen(ps_file_cmd) + 1;
    ps_cmd = (CHAR *)calloc(i4_len, sizeof(CHAR));
    if (NULL == ps_cmd)
    {
        DBG_ERROR((PBUSB_TAG"malloc cmd failed(size:%d)!\n", i4_len));
        return PB_USB_FAIL;
    }

    i4_ret = snprintf(ps_cmd, i4_len, "%s%s%s", ps_find_cmd, t_mnt_info.s_mnt_path, ps_file_cmd);
    DBG_INFO((PBUSB_TAG"update cmd:%s\n", ps_cmd));
    if ((i4_ret + 1) != i4_len) //i4_ret + 1:snprintf return value not include '\0'
    {
        free(ps_cmd);
        DBG_ERROR((PBUSB_TAG"snprintf cmd failed(i4_ret:%d i4_len:%d)!\n", i4_ret, i4_len));
        return PB_USB_FAIL;
    }

    system(ps_cmd);

    free(ps_cmd);
    return PB_USB_OK;
}

static INT32 _playback_usb_update_playback_list(void)
{
    INT32 ret = PB_USB_FAIL;

    ret = _playback_usb_update_wav_file();
    if (PB_USB_OK != ret)
    {
        return ret;
    }

    if(NULL != p_playlist)
    {
        _playback_usb_delete_wav_play_list(&p_playlist);
        g_pt_playlist_head = NULL;
    }

    g_pt_playlist_head = _playback_usb_create_wav_play_list(&t_g_pb_usb.wav_num);
    p_playlist = g_pt_playlist_head;

    DBG_INFO((PBUSB_TAG"wav playback list num is %d\n",t_g_pb_usb.wav_num));

    if(NULL != p_playlist)
    {
        _playback_usb_show_wav_list(p_playlist);
        ret = PB_USB_OK;
    }
    else
    {
        ret = PB_USB_FAIL;
    }
    return ret;
}


/*-----------------------------------------------------------------------------
 * Name: playback_usb_alsa_msg_process()
 *
 * Description:alsa playback msg process function
 *
 * Input:
 *
 * Output :
 *
 * Returns: 0                          Routine successful.
 *              Any other value     Routine failed.
 *----------------------------------------------------------------------------*/
INT32 playback_usb_alsa_msg_process(PB_USB_MSG_T  t_pb_usb_msg)
{
    INT32 ret = PB_USB_OK;
    PB_USB_MSG_T t_alsa_msg = {0};

    DBG_ERROR((PBUSB_TAG"%s is called, msg id is %d\n", __FUNCTION__,t_pb_usb_msg.ui4_msg_id));

    switch (t_pb_usb_msg.ui4_msg_id)
    {
        case ALSA_USB_PB_START:
        {
            DBG_INFO((PBUSB_TAG"ALSA_USB_PB_START at line %d\n",__LINE__));
            //when recieve the USB start play request. please check the wav file firstly.
            #if 0
            memset(&t_alsa_msg,0,sizeof(PB_USB_MSG_T));
            t_alsa_msg.ui4_msg_id = ALSA_USB_PB_CREATE_PB_LIST;
            ret = u_playback_usb_send_msg(&t_alsa_msg);
            #else
            //send play request to sm
            _playback_usb_send_play_request_to_sm();
            #endif
        }
        break;

        case ALSA_USB_PB_CREATE_PB_LIST:
        {
            DBG_INFO((PBUSB_TAG"ALSA_USB_PB_CREATE_PB_LIST at line %d\n",__LINE__));

            ret = _playback_usb_update_playback_list();
            if(PB_USB_OK == ret)
            {
                memset(&t_alsa_msg,0,sizeof(PB_USB_MSG_T));
                t_alsa_msg.ui4_msg_id = ALSA_USB_PB_PARSER_FILE;
                ret = u_playback_usb_send_msg(&t_alsa_msg);
            }
            else
            {
                _playback_usb_send_pb_status_to_sm(SM_BODY_STOP);
            }
        }
        break;

        case ALSA_USB_PB_PARSER_FILE:
        {
            DBG_INFO((PBUSB_TAG"ALSA_USB_PB_PARSER_FILE at line %d\n",__LINE__));
            memset(&t_alsa_msg,0,sizeof(PB_USB_MSG_T));

            ret = _playback_usb_parser_file();
            if(PB_USB_OK == ret)
            {
                t_alsa_msg.ui4_msg_id = ALSA_USB_PB_SET_HW_PARAMS;

                if(1 == t_pb_usb_msg.ui4_data1)
                    t_alsa_msg.ui4_data1 = 1;

                ret = u_playback_usb_send_msg(&t_alsa_msg);
            }
            else if(ret == PB_USB_INV_ARG)
            {
                /*goto playback next wav*/
                DBG_ERROR((PBUSB_TAG"goto playback next wav\n"));
                if(0 == t_pb_usb_msg.ui4_data1)
                    t_alsa_msg.ui4_msg_id = ALSA_USB_PB_NEXT;
                else
                    t_alsa_msg.ui4_msg_id = ALSA_USB_PB_PREVIOUS;

                u_playback_usb_send_msg(&t_alsa_msg);
            }

        }
        break;

        case ALSA_USB_PB_SET_HW_PARAMS:
        {
            UINT32 ui4_retry_count = 0;
            DBG_INFO((PBUSB_TAG"ALSA_USB_PB_SET_HW_PARAMS at line %d\n",__LINE__));
            memset(&t_alsa_msg,0,sizeof(PB_USB_MSG_T));

            if(NULL != g_playback_pcm.handle)
            {
                snd_pcm_close(g_playback_pcm.handle);
                g_playback_pcm.handle = NULL;
            }

            while (1)
            {
                if (snd_pcm_open(&g_playback_pcm.handle,MAIN_PCM_DEVICE_NAME,SND_PCM_STREAM_PLAYBACK,0) < 0)
                {
                    ui4_retry_count++;
                    DBG_ERROR((PBUSB_TAG"snd_pcm_open handle %s fail, count:%lu\n",MAIN_PCM_DEVICE_NAME, ui4_retry_count));
                }
                else
                {
                    break;
                }

                if (ui4_retry_count >= 10)
                {
                    DBG_ERROR((PBUSB_TAG"retry count:%lu, exit!\n",MAIN_PCM_DEVICE_NAME, ui4_retry_count));
                    _playback_usb_send_pb_status_to_sm(SM_BODY_STOP);
                    return PB_USB_FAIL;
                }
                usleep(20 * 1000);
            }

            DBG_ERROR((PBUSB_TAG"snd_pcm_open handle %s ok\n",MAIN_PCM_DEVICE_NAME));
            if (u_alsa_set_hw_params(&g_playback_pcm, 200000) < 0)
            {
                DBG_ERROR((PBUSB_TAG"set alsa hw params fail\n"));
                ret = PB_USB_FAIL;
                /*goto playback next wav*/
                DBG_ERROR((PBUSB_TAG"goto playback next wav\n"));
                if(0 == t_pb_usb_msg.ui4_data1)
                    t_alsa_msg.ui4_msg_id = ALSA_USB_PB_NEXT;
                else
                    t_alsa_msg.ui4_msg_id = ALSA_USB_PB_PREVIOUS;

                u_playback_usb_send_msg(&t_alsa_msg);
            }
            else
            {
                DBG_ERROR((PBUSB_TAG"set alsa hw params ok\n"));
                if(TRUE == playback_usb_get_pb_thread_running_flag())
                {
                    playback_usb_reset_next_wav_params();
                    playback_usb_set_switch_flag(FALSE);
                }
                else
                {
                    t_alsa_msg.ui4_msg_id = ALSA_USB_PB_CREATE_PB_THREAD;
                    ret = u_playback_usb_send_msg(&t_alsa_msg);
                }
            }
        }
        break;

        case ALSA_USB_PB_CREATE_PB_THREAD:
        {
            DBG_INFO((PBUSB_TAG"ALSA_USB_PB_CREATE_PB_THREAD at line %d\n",__LINE__));
            pthread_t alsa_play_thread;
            pthread_attr_t attr;
            pthread_attr_init(&attr);
            /*set detached system will auto release resource*/
            pthread_attr_setdetachstate(&attr,PTHREAD_CREATE_DETACHED);
            pthread_create(&alsa_play_thread,&attr,(void *)_playback_usb_alsa_playback_thread,NULL);
            DBG_ERROR((PBUSB_TAG"create alsa playback thread finished,pthread_t id is %u\n",alsa_play_thread));
        }
        break;

        case ALSA_USB_PB_PLAY:
        {
            DBG_INFO((PBUSB_TAG"ALSA_USB_PB_PLAY at line %d\n",__LINE__));
            if(FALSE == playback_usb_get_pb_thread_running_flag())
            {
                break;
            }

            if(TRUE == playback_usb_get_pause_flag())
            {
                /*current is pause status, need to resume play*/
                if(u_alsa_resume(g_playback_pcm.handle) != PB_USB_OK)
                {
                    ret = PB_USB_FAIL;
                }
                else
                {
                    playback_usb_set_pause_flag(FALSE);
                    //to be done,notify play msg to ss
                    _playback_usb_send_pb_status_to_sm(SM_BODY_PLAY);
                }
            }
        }
        break;

        case ALSA_USB_PB_PAUSE:
        case ALSA_USB_PB_PLAY_PAUSE:
        {
            DBG_INFO((PBUSB_TAG"ALSA_USB_PB_PAUSE/ALSA_USB_PB_PLAY_PAUSE at line %d\n",__LINE__));
            if(FALSE == playback_usb_get_pb_thread_running_flag())
            {
                break;
            }

            if(FALSE == playback_usb_get_pause_flag())
            {
                /*current is play status, need to pause play*/
                playback_usb_set_pause_flag(TRUE);
                //to be done,notify pause msg to ss
                _playback_usb_send_pb_status_to_sm(SM_BODY_PAUSE);
                playback_usb_lock_switch_sema();
                if(u_alsa_pause(g_playback_pcm.handle) != PB_USB_OK)
                {
                    ret = PB_USB_FAIL;
                }
                playback_usb_unlock_switch_sema();
            }
            else
            {
                /*current is pause status, need to resume play*/
                if(u_alsa_resume(g_playback_pcm.handle) != PB_USB_OK)
                {
                    DBG_ERROR((PBUSB_TAG"alsa resume from pause failed!\n"));
                    ret = PB_USB_FAIL;
                }
                else
                {
                    playback_usb_set_pause_flag(FALSE);
                    //to be done,notify play msg to ss
                    _playback_usb_send_pb_status_to_sm(SM_BODY_PLAY);
                }
            }
        }
        break;

        case ALSA_USB_PB_STOP:
        {
            DBG_INFO((PBUSB_TAG"ALSA_USB_PB_STOP at line %d\n",__LINE__));

            if(TRUE == playback_usb_get_pb_thread_running_flag())
            {
                DBG_ERROR((PBUSB_TAG"alsa stop starting\n"));
                playback_usb_set_pb_thread_running_flag(FALSE);
                playback_usb_lock_pb_sema();
            }
            //to be done,notify stop msg to ss
            _playback_usb_send_pb_status_to_sm(SM_BODY_STOP);
            DBG_ERROR((PBUSB_TAG"alsa stop finished\n"));
        }
        break;

        case ALSA_USB_PB_DELET_PB_LIST:
        {
            DBG_INFO((PBUSB_TAG"ALSA_USB_PB_DELET_PB_LIST at line %d\n",__LINE__));
            if(NULL != p_playlist)
            {
                _playback_usb_delete_wav_play_list(&p_playlist);
                g_pt_playlist_head = NULL;
            }
        }
        break;

        case ALSA_USB_PB_NEXT:
        {
            DBG_INFO((PBUSB_TAG"ALSA_USB_PB_NEXT at line %d\n",__LINE__));
            memset(&t_alsa_msg,0,sizeof(PB_USB_MSG_T));

            if(NULL == p_playlist)
            {
                DBG_ERROR((PBUSB_TAG"Wav playback list delet or error\n"));
                break;
            }

            if(NULL != p_playlist->next)
            {
                p_playlist = p_playlist->next;
            }
            else /*next=NULL means tail postion,switch to head*/
            {
                p_playlist = g_pt_playlist_head;
            }

            t_alsa_msg.ui4_data1= 0; /*next*/

            DBG_ERROR((PBUSB_TAG"Next playback file name is %s\n",p_playlist->data));

            if(TRUE == playback_usb_get_pb_thread_running_flag())
            {
                playback_usb_set_switch_flag(TRUE);
            }

            playback_usb_lock_switch_sema();

            playback_usb_stop_pcm_playback();

            t_alsa_msg.ui4_msg_id = ALSA_USB_PB_PARSER_FILE;

            ret = u_playback_usb_send_msg(&t_alsa_msg);

            playback_usb_unlock_switch_sema();
        }
        break;

        case ALSA_USB_PB_PREVIOUS:
        {
            DBG_INFO((PBUSB_TAG"ALSA_USB_PB_PREVIOUS at line %d\n",__LINE__));
            memset(&t_alsa_msg,0,sizeof(PB_USB_MSG_T));

#if 0
            if(FALSE == playback_usb_get_pb_thread_running_flag())
            {
                break;
            }
#endif

            if(NULL == p_playlist)
            {
                DBG_ERROR((PBUSB_TAG"Wav playback list delet or error\n"));
                break;
            }

            if(NULL != p_playlist->pre)
            {
                p_playlist = p_playlist->pre;
            }
            else /*pre=NULL means head postion,switch to tail*/
            {
                while (NULL != p_playlist->next)
                {
                    p_playlist = p_playlist->next;
                }
            }

            t_alsa_msg.ui4_data1= 1; /*previous*/

            DBG_ERROR((PBUSB_TAG"Next playback file name is %s\n",p_playlist->data));

            if(TRUE == playback_usb_get_pb_thread_running_flag())
            {
                playback_usb_set_switch_flag(TRUE);
            }

            playback_usb_lock_switch_sema();

            playback_usb_stop_pcm_playback();

            t_alsa_msg.ui4_msg_id = ALSA_USB_PB_PARSER_FILE;

            ret = u_playback_usb_send_msg(&t_alsa_msg);

            playback_usb_unlock_switch_sema();
        }
        break;

        case ALSA_USB_PB_PCM_STATE_CHECK:
        {
            DBG_INFO((PBUSB_TAG"ALSA_USB_PB_PCM_STATE_CHECK at line %d\n",__LINE__));
            u_alsa_get_pcm_state(g_playback_pcm.handle);
            DBG_INFO((PBUSB_TAG"current playback status is %d\n",u_playback_usb_get_playback_status()));
            #if 1
            system("ls -l");
            #endif
        }
        break;

        default:
        break;
    }

    return ret;
}




static INT32 playback_usb_sm_msg_process(APPMSG_T* pv_msg)
{
    PB_USB_MSG_T t_pb_usb_msg = {0};

    switch(SM_MSG_GRP(pv_msg->ui4_msg_type))
    {
        case SM_REQUEST_GRP:
        case SM_PERMIT_GRP:
        case SM_INFORM_GRP:
        {
            switch(SM_MSG_BDY(pv_msg->ui4_msg_type))
            {
                case SM_BODY_PLAY:
                {
                    memset(&t_pb_usb_msg,0,sizeof(PB_USB_MSG_T));
                    t_pb_usb_msg.ui4_msg_id = ALSA_USB_PB_CREATE_PB_LIST;
                    u_playback_usb_send_msg(&t_pb_usb_msg);
                }
                break;

                case SM_BODY_PAUSE:
                {
                    memset(&t_pb_usb_msg,0,sizeof(PB_USB_MSG_T));
                    t_pb_usb_msg.ui4_msg_id = ALSA_USB_PB_PAUSE;
                    u_playback_usb_send_msg(&t_pb_usb_msg);
                }
                break;

                case SM_BODY_STOP:
                {
                    memset(&t_pb_usb_msg,0,sizeof(PB_USB_MSG_T));
                    t_pb_usb_msg.ui4_msg_id = ALSA_USB_PB_STOP;
                    u_playback_usb_send_msg(&t_pb_usb_msg);
                }
                break;

                default:
                break;
            }
        }
        break;

        case SM_FORBID_GRP:
        {
            switch(SM_MSG_BDY(pv_msg->ui4_msg_type))
            {
                case SM_BODY_PLAY:
                {
                    DBG_ERROR((PBUSB_TAG"Forbid SM_BODY_PLAY by SM\n"));
                }
                break;

                case SM_BODY_PAUSE:
                {
                    DBG_ERROR((PBUSB_TAG"Forbid SM_BODY_PAUSE by SM\n"));
                }
                break;

                case SM_BODY_STOP:
                {
                    DBG_ERROR((PBUSB_TAG"Forbid SM_BODY_STOP by SM\n"));
                }
                break;

                default:
                break;
            }
        }
        break;

        default:
        break;
    }
}

static VOID _playback_usb_state_process(BOOL b_mnt)
{
    INT32 i4_ret;
    PB_USB_MSG_T  t_pb_usb_msg;

    memset(&t_pb_usb_msg, 0, sizeof(PB_USB_MSG_T));
    t_pb_usb_msg.ui4_msg_id = (TRUE == b_mnt) ? ALSA_USB_PB_START : ALSA_USB_PB_STOP;
    i4_ret = u_playback_usb_send_msg(&t_pb_usb_msg);
    if (PB_USB_OK != i4_ret)
    {
        DBG_ERROR((PBUSB_TAG"playback send dm mnt msg failed!\n"));
    }
}

static VOID _playback_usb_dm_msg_process(const VOID* pv_msg, const SIZE_T z_msg_len)
{
    DM_MNT_MSG_T *pt_mnt_msg = (DM_MNT_MSG_T *)pv_msg;

    if (z_msg_len != sizeof(DM_MNT_MSG_T))
    {
        DBG_ERROR((PBUSB_TAG"dm's msg size[%d] is wrong, should be [%d]!\n", z_msg_len, sizeof(DM_MNT_MSG_T)));
        return;
    }
    DBG_INFO((PBUSB_TAG"[dm's broadcast]event type:%d\n", pt_mnt_msg->e_dm_event));

    switch(pt_mnt_msg->e_dm_event)
    {
        case DM_DEV_EVT_MOUNTED:
            if (g_b_usb_auto_play)
            {
                _playback_usb_state_process(TRUE);
            }
            break;
        case DM_DEV_EVT_UNMOUNTED:
            _playback_usb_state_process(FALSE);
            break;
        default:
            break;
    }
}

static VOID _playback_process_ui_mute_key(VOID)
{
    BOOL b_vol_mute;
    UINT8 ui1_value;
    INT32 i4_ret;

    DBG_INFO((PBUSB_TAG"receive volume mute key!\n"));
    i4_ret = u_acfg_get_mute(&b_vol_mute);
    if (APP_CFGR_OK != i4_ret)
    {
        DBG_ERROR((PBUSB_TAG"u_acfg_get_mute fail ret:%d\n", i4_ret));
        return;
    }

    b_vol_mute = !b_vol_mute;

    i4_ret = u_acfg_set_mute(b_vol_mute);
    if (APP_CFGR_OK != i4_ret)
    {
        DBG_ERROR((PBUSB_TAG"u_acfg_set_mute fail ret:%d\n", i4_ret));
        return;
    }

    if (!b_vol_mute)
    {
        i4_ret = u_acfg_get_volume(&ui1_value);
        if (APP_CFGR_OK != i4_ret)
        {
            DBG_ERROR((PBUSB_TAG"u_acfg_get_volume fail ret:%d\n", i4_ret));
            return;
        }

        i4_ret = u_acfg_set_volume(ui1_value);
        if (APP_CFGR_OK != i4_ret)
        {
            DBG_ERROR((PBUSB_TAG"u_acfg_set_volume fail ret:%d\n", i4_ret));
            return;
        }
    }
}

static VOID _playback_process_ui_volume_up_key(VOID)
{
    INT32 i4_ret;

    DBG_INFO((PBUSB_TAG"receive volume up key!\n"));
    i4_ret = u_acfg_increase_volume();
    if (APP_CFGR_OK != i4_ret)
    {
        DBG_ERROR((PBUSB_TAG"u_acfg_increase_volume fail ret:%d\n", i4_ret));
    }
}

static VOID _playback_process_ui_volume_down_key(VOID)
{
    INT32 i4_ret;

    DBG_INFO((PBUSB_TAG"receive volume down key!\n"));
    i4_ret = u_acfg_reduce_volume();
    if (APP_CFGR_OK != i4_ret)
    {
        DBG_ERROR((PBUSB_TAG"u_acfg_increase_volume fail ret:%d\n", i4_ret));
    }
}

VOID u_playback_ui_msg_process(const VOID *pv_msg, SIZE_T z_msg_len)
{
    APPMSG_T *t_msg = (APPMSG_T *)pv_msg;

    if (z_msg_len != sizeof(*t_msg))
    {
        DBG_ERROR((PBUSB_TAG"ui's msg size[%d] is wrong, should be [%d]!\n", z_msg_len, sizeof(APPMSG_T)));
        return;
    }

    DBG_INFO((PBUSB_TAG"receive ui key value:%d!\n", t_msg->ui4_msg_type));

    switch(t_msg->ui4_msg_type)
    {
        case KEY_MUTE:
            _playback_process_ui_mute_key();
            break;
        case KEY_VOLUMEUP:
            _playback_process_ui_volume_up_key();
            break;
        case KEY_VOLUMEDOWN:
            _playback_process_ui_volume_down_key();
            break;
        default:
            break;
    }
}

static INT32 _playback_usb_send_pb_status_to_sm(SM_MSG_BDY_E PB_BODY)
{
    INT32 i4_ret = PB_USB_FAIL;
    APPMSG_T t_msg = {0};
    HANDLE_T h_app = NULL_HANDLE;

    t_msg.ui4_sender_id = MSG_FROM_PB;

    t_msg.ui4_msg_type = SM_MAKE_MSG(SM_INFORM_GRP,PB_BODY,0);

    u_am_get_app_handle_from_name(&h_app,SM_THREAD_NAME);

    i4_ret = u_app_send_msg(h_app,
                            E_APP_MSG_TYPE_STATE_MNGR,
                            &t_msg,
                            sizeof(APPMSG_T),
                            NULL,
                            NULL);

    if (AEER_OK != i4_ret) {
        DBG_ERROR((PBUSB_TAG"_playback_usb_send_play_request_to_sm failed, i4_ret[%ld]\n",i4_ret));
    }
    DBG_INFO((PBUSB_TAG"playback send %d status to sm!\n", PB_BODY));

    return (i4_ret == AEER_OK ? PB_USB_OK : PB_USB_FAIL);
}


static INT32 _playback_usb_send_play_request_to_sm(void)
{
    INT32 i4_ret = PB_USB_FAIL;
    APPMSG_T t_msg = {0};
    HANDLE_T h_app = NULL_HANDLE;

    t_msg.ui4_sender_id = MSG_FROM_PB;

    t_msg.ui4_msg_type = SM_MAKE_MSG(SM_REQUEST_GRP,SM_BODY_PLAY,0);

    u_am_get_app_handle_from_name(&h_app,SM_THREAD_NAME);

    i4_ret = u_app_send_msg(h_app,
                            E_APP_MSG_TYPE_STATE_MNGR,
                            &t_msg,
                            sizeof(APPMSG_T),
                            NULL,
                            NULL);

    if (AEER_OK != i4_ret) {
        DBG_ERROR((PBUSB_TAG"_playback_usb_send_play_request_to_sm failed, i4_ret[%ld]\n",i4_ret));
    }

    return (i4_ret == AEER_OK ? PB_USB_OK : PB_USB_FAIL);
}


static INT32 _playback_usb_process_msg (
                    HANDLE_T                    h_app,
                    UINT32                      ui4_type,
                    const VOID*                 pv_msg,
                    SIZE_T                      z_msg_len,
                    BOOL                        b_paused
                    )
{

    PB_USB_MSG_T   t_pb_usb_msg = {0};


    if (FALSE == t_g_pb_usb.b_app_init_ok) {
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
                DBG_ERROR((PBUSB_TAG"E_APP_PRI_ALSA_PB_MSG\n"));
                memcpy(
                         &t_pb_usb_msg,
                         (PB_USB_MSG_T*)pv_msg,
                         sizeof(PB_USB_MSG_T));

                playback_usb_alsa_msg_process(t_pb_usb_msg);
                break;
            }
            case E_APP_MSG_TYPE_USER_CMD:
            {
                u_playback_ui_msg_process(pv_msg, z_msg_len);
            }
            break;
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
                playback_usb_sm_msg_process((APPMSG_T *)pv_msg);
            }
            break;

            case E_APP_MSG_TYPE_DUT_STATE:
            {
                // should get power on/off msg here
            }
            break;

            case E_APP_MSG_TYPE_USB_DEV:
            {
                _playback_usb_dm_msg_process(pv_msg, z_msg_len);
            }
            break;
            default:
            break;
        }
    }

    return AEER_OK;
}


/*-------------------------------------------------------------------------
 * Name
 *      u_playback_usb_send_msg
 * Description      -
 * Input arguments  -
 * Output arguments -
 * Returns          -
 *------------------------------------------------------------------------*/
INT32 u_playback_usb_send_msg(const PB_USB_MSG_T* pt_event)
{
    INT32 i4_ret;

    if (FALSE == t_g_pb_usb.b_app_init_ok)
    {
        return PB_USB_FAIL;
    }

    i4_ret = u_app_send_msg(t_g_pb_usb.h_app,
                            E_APP_PRI_ALSA_PB_MSG,
                            pt_event,
                            sizeof(PB_USB_MSG_T),
                            NULL,
                            NULL);

    if (AEER_OK != i4_ret) {
        DBG_ERROR((PBUSB_TAG"_app_send_msg sent E_APP_PRI_ALSA_PB_MSG failed, i4_ret[%ld]\n",i4_ret));
    }

    return (i4_ret == AEER_OK ? PB_USB_OK : PB_USB_FAIL);
}


/*-------------------------------------------------------------------------
 * Name
 *      u_playback_usb_send_msg
 * Description      -
 * Input arguments  -
 * Output arguments -
 * Returns          -
 *------------------------------------------------------------------------*/
SM_MSG_BDY_E u_playback_usb_get_playback_status(void)
{
    SM_MSG_BDY_E ret = SM_BODY_STOP;

    if(TRUE == playback_usb_get_pb_thread_running_flag())
    {
        if(TRUE == playback_usb_get_pause_flag())
        {
            ret = SM_BODY_PAUSE;
        }
        else
        {
            ret = SM_BODY_PLAY;
        }
    }
    else/*pb thread running == FALSE*/
    {
            ret = SM_BODY_STOP;
    }
    return ret;
}
