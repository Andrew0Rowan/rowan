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
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <pthread.h>
/* application level */
#include "u_amb.h"
#include "u_common.h"
#include "u_app_thread.h"
#include "u_cli.h"
#include "u_os.h"
#include "u_assert.h"
#include "u_dm.h"
#include "u_acfg.h"
#include "u_user_interface.h"
#include "u_bluetooth_audio.h"
/* private */
#include "sm.h"
#include "u_sm.h"
#include "sm_cli.h"
#include "NvRAMUtils.h"
#include "led.h"

#define SECONDS_PER_STANDBY_COUNT   3000    //ms
#define MINUTES_TO_ENTER_STANDBY    5
#define COUNTS_TO_ENTER_STANDBY     (MINUTES_TO_ENTER_STANDBY * 60 * 1000 / SECONDS_PER_STANDBY_COUNT)
#define CONFIG_AUTO_SLEEP_FOR_SUSPEND 1
extern void* g_mas_handle;

/*-----------------------------------------------------------------------------
                    private variable definition
-----------------------------------------------------------------------------*/
typedef enum
{
    TIMER_MSG_STANDBY = 0,
    TIMER_MSG_MAX
} SM_TIMER_MSG_E;

SOURCE_INFO_T g_t_source_list[SOURCE_MAX] =
{
    {SOURCE_NONE,       NULL},
    {SOURCE_URI,        PB_URI_THREAD_NAME},
    {SOURCE_BT,         BLUETOOTH_THREAD_NAME},
    {SOURCE_TTS,        PB_TTS_THREAD_NAME},
    {SOURCE_PROMPT,     PB_PROMPT_THREAD_NAME},
    {SOURCE_BT_HFP,     BLUETOOTH_HFP_THREAD_NAME},
    {SOURCE_DLNA,       PB_DLNA_THREAD_NAME},
};

static HANDLE_T g_h_status_sema = NULL;
static SM_OBJ_T g_t_sm = {0};
static UINT16 ui2_g_sm_dbg_level = DBG_INIT_LEVEL_APP_SM;

static int network_quality = 0;
static char network_status[ASSISTANT_STUB_STATUS_MAX_LENGTH+1] = "disconnect";
static BOOL f_is_custom_clear_down = FALSE;
static BOOL f_is_set_system_status = FALSE;
static BOOL f_ecopwer = FALSE;
/*-----------------------------------------------------------------------------
 * private methods declarations
 *---------------------------------------------------------------------------*/
static INT32 _sm_start (
                    const CHAR*                 ps_name,
                    HANDLE_T                    h_app
                    );
static INT32 _sm_exit (
                    HANDLE_T                    h_app,
                    APP_EXIT_MODE_T             e_exit_mode
                    );
static INT32 _sm_process_msg (
                    HANDLE_T                    h_app,
                    UINT32                      ui4_type,
                    const VOID*                 pv_msg,
                    SIZE_T                      z_msg_len,
                    BOOL                        b_paused
                    );
static INT32 _sm_send_msg_to_assistant_stub(ASSISTANT_STUB_CMD_INDEX_E e_cmd, VOID *pt_send_msg,UINT32 msg_size);
static INT32 _sm_send_msg_to_upg_control(SM_MSG_BDY_E UPG_BODY);
static VOID _sm_process_pause_msg(VOID);
static VOID _sm_process_resume_msg(VOID);
static VOID _sm_enter_fake_standby_mode(VOID);
static VOID _sm_exit_fake_standby_mode(VOID);
static VOID _sm_enter_suspend_standby_mode();
static VOID _sm_exit_suspend_standby_mode();
static VOID _sm_process_long_volumedown_msg(VOID);
BOOL _sm_is_factory_reset_down(VOID);
VOID _sm_set_factory_reset(BOOL flag);
VOID _sm_set_factory_reset(BOOL flag);
BOOL _sm_is_set_system_status(VOID);
VOID u_sm_send_stop_to_all_source(VOID);
BOOL _sm_is_process_exist(const char * process_name);
/*---------------------------------------------------------------------------
 * Name
 *      a_app_set_registration
 * Description      -
 * Input arguments  -
 * Output arguments -
 * Returns          -
 *---------------------------------------------------------------------------*/
VOID a_sm_register(
                AMB_REGISTER_INFO_T*            pt_reg
                )
{
    if (g_t_sm.b_app_init == TRUE)
    {
        return;
    }


    memset(pt_reg->s_name, 0, sizeof(CHAR)*(APP_NAME_MAX_LEN + 1));
    strncpy(pt_reg->s_name, SM_THREAD_NAME, APP_NAME_MAX_LEN);
    pt_reg->t_fct_tbl.pf_init                   = _sm_start;
    pt_reg->t_fct_tbl.pf_exit                   = _sm_exit;
    pt_reg->t_fct_tbl.pf_process_msg            = _sm_process_msg;
    pt_reg->t_desc.ui8_flags                    = ~((UINT64)0);
    pt_reg->t_desc.t_thread_desc.z_stack_size   = SM_STACK_SIZE;
    pt_reg->t_desc.t_thread_desc.ui1_priority   = SM_THREAD_PRIORITY;
    pt_reg->t_desc.t_thread_desc.ui2_num_msgs   = SM_NUM_MSGS;

    pt_reg->t_desc.ui2_msg_count                = SM_MSGS_COUNT;
    pt_reg->t_desc.ui2_max_msg_size             = SM_MAX_MSGS_SIZE;
}

UINT16 sm_get_dbg_level(VOID)
{
    return (ui2_g_sm_dbg_level | DBG_LAYER_APP);
}

VOID sm_set_dbg_level(UINT16 ui2_db_level)
{
    ui2_g_sm_dbg_level = ui2_db_level;
}

static inline BOOL _sm_is_prompt_exist(VOID)
{
    return !!(g_t_sm.t_prompt_source.e_prompt_source);
}

static inline BOOL _sm_is_prompt_wait_permit(VOID)
{
    return _sm_is_prompt_exist() && g_t_sm.t_prompt_source.b_wait_permit;
}

static inline BOOL _sm_is_prompt_playing(VOID)
{
    return _sm_is_prompt_exist() && !_sm_is_prompt_wait_permit();
}

static inline BOOL _sm_is_tts_exist(VOID)
{
    return !!(g_t_sm.t_tts_source.e_tts_source);
}

static inline BOOL _sm_is_tts_wait_permit(VOID)
{
    return _sm_is_tts_exist() && g_t_sm.t_tts_source.b_wait_permit;
}

static inline BOOL _sm_is_tts_need_mix(VOID)
{
    return (_sm_is_tts_exist() && !!(g_t_sm.t_tts_source.t_tts_play.need_mix));
}

static inline BOOL _sm_is_tts_playing(VOID)
{
    return _sm_is_tts_exist() && !_sm_is_tts_wait_permit();
}

static inline BOOL _sm_is_bt_exist(VOID)
{
    return !!(g_t_sm.t_bt_source.e_bt_source);
}

static inline BOOL _sm_is_bt_wait_permit(VOID)
{
    return _sm_is_bt_exist() && g_t_sm.t_bt_source.e_wait_permit;
}

static inline BOOL _sm_is_bt_playing(VOID)
{
    return _sm_is_bt_exist()
            && !_sm_is_bt_wait_permit()
            && (g_t_sm.t_bt_source.ui4_status == SM_BODY_PLAY);
}

static inline BOOL _sm_is_bt_pause(VOID)
{
    return _sm_is_bt_exist()
            && !_sm_is_bt_wait_permit()
            && (g_t_sm.t_bt_source.ui4_status == SM_BODY_PAUSE);
}

static inline BOOL _sm_is_bt_suspend(VOID)
{
    return _sm_is_bt_pause() && g_t_sm.t_bt_source.b_suspend;
}

static inline BOOL _sm_is_bt_hfp_exist(VOID)
{
    return !!(g_t_sm.t_bt_hfp_source.e_bt_hfp_source);
}

static inline BOOL _sm_is_bt_hfp_wait_permit(VOID)
{
    return _sm_is_bt_hfp_exist() && g_t_sm.t_bt_hfp_source.b_wait_permit;
}

static inline BOOL _sm_is_bt_hfp_playing(VOID)
{
    return _sm_is_bt_hfp_exist()
            && !_sm_is_bt_hfp_wait_permit()
            && (g_t_sm.t_bt_hfp_source.ui4_status == SM_BODY_PLAY);
}

static inline BOOL _sm_is_dlna_exist(VOID)
{
    return !!(g_t_sm.t_dlna_source.e_dlna_source);
}

static inline BOOL _sm_is_dlna_wait_permit(VOID)
{
    return _sm_is_dlna_exist() && g_t_sm.t_dlna_source.b_wait_permit;
}

static inline BOOL _sm_is_dlna_playing(VOID)
{
    return _sm_is_dlna_exist()
            && !_sm_is_dlna_wait_permit()
            && (g_t_sm.t_dlna_source.ui4_status == SM_BODY_PLAY);
}

static inline BOOL _sm_is_dlna_pause(VOID)
{
    return _sm_is_dlna_exist()
            && !_sm_is_dlna_wait_permit()
            && (g_t_sm.t_dlna_source.ui4_status == SM_BODY_PAUSE);
}

static inline BOOL _sm_is_dlna_suspend(VOID)
{
    return _sm_is_dlna_pause() && g_t_sm.t_dlna_source.b_suspend;
}

static inline BOOL _sm_is_uri_exist(VOID)
{
    return !!(g_t_sm.t_uri_source.e_uri_source);
}

static inline BOOL _sm_is_uri_wait_permit(VOID)
{
    return _sm_is_uri_exist() && g_t_sm.t_uri_source.b_wait_permit;
}

static inline BOOL _sm_is_uri_playing(VOID)
{
    return _sm_is_uri_exist()
            && !_sm_is_uri_wait_permit()
            && (g_t_sm.t_uri_source.ui4_status == SM_BODY_PLAY);
}

static inline BOOL _sm_is_uri_pause(VOID)
{
    return _sm_is_uri_exist()
            && !_sm_is_uri_wait_permit()
            && (g_t_sm.t_uri_source.ui4_status == SM_BODY_PAUSE);
}

static inline BOOL _sm_is_uri_suspend(VOID)
{
    return _sm_is_uri_pause() && !!(g_t_sm.t_uri_source.e_suspend_mode);
}

static INT32 _sm_create_standby_timer(VOID)
{
    INT32 i4_ret;

    g_t_sm.t_standby_timer.e_flags = X_TIMER_FLAG_REPEAT;
    g_t_sm.t_standby_timer.ui4_delay = SECONDS_PER_STANDBY_COUNT;//3s

    i4_ret = u_timer_create(&g_t_sm.t_standby_timer.h_timer);
    if (OSR_OK != i4_ret)
    {
        SM_ERR("standby timer create failed!\n");
        return SMR_FAIL;
    }
    return SMR_OK;
}

static INT32 _sm_start_standby_timer(VOID)
{
    INT32 i4_ret;
    SM_TIMER_MSG_E e_standby_msg = TIMER_MSG_STANDBY;

    i4_ret = u_timer_start(g_t_sm.h_app, &g_t_sm.t_standby_timer, (void *)&e_standby_msg, sizeof(SM_TIMER_MSG_E));
    if (OSR_OK != i4_ret)
    {
        SM_ERR("standby timer start failed!\n");
        return SMR_FAIL;
    }
    return SMR_OK;
}

static INT32 _sm_start (const CHAR *ps_name,HANDLE_T h_app)
{
    INT32 i4_ret;

    memset(&g_t_sm, 0, sizeof(SM_OBJ_T));
    g_t_sm.h_app = h_app;


    if (g_t_sm.b_app_init)
    {
        return AEER_OK;
    }

#ifdef CLI_SUPPORT
    i4_ret = sm_cli_attach_cmd_tbl();
    if ((i4_ret != CLIR_NOT_INIT) && (i4_ret != CLIR_OK))
    {
        return AEER_FAIL;
    }
    sm_set_dbg_level(DBG_INIT_LEVEL_APP_SM);
#endif/* CLI_SUPPORT */

    _sm_create_standby_timer();
    _sm_start_standby_timer();

    g_t_sm.b_app_init = TRUE;

    return AEER_OK;
}

static INT32 _sm_exit (HANDLE_T h_app,APP_EXIT_MODE_T e_exit_mode)
{
    INT32 i4_ret;
    u_sema_delete(g_h_status_sema);
    u_timer_delete(g_t_sm.t_standby_timer.h_timer);
    g_t_sm.b_app_init = FALSE;
    return AEER_OK;
}

INT32 _sm_send_msg(UINT32 ui4_sender_id, UINT32 ui4_msg_type,const CHAR* ps_name)
{
    INT32 i4_ret = SMR_OK;
    HANDLE_T h_app = NULL_HANDLE;

    i4_ret = u_am_get_app_handle_from_name(&h_app, ps_name);
    if (0 != i4_ret)
    {
        SM_ERR("u_am_get_app_handle_from_name failed! ps_name:%s\n", ps_name);
    }

    i4_ret = u_app_send_appmsg(h_app,
                            E_APP_MSG_TYPE_STATE_MNGR,
                            ui4_sender_id,
                            ui4_msg_type,
                            NULL,
                            0);
    if (0 != i4_ret)
    {
        SM_ERR("u_app_send_msg failed! h_app:%p\n", h_app);
    }

    return i4_ret;
}


static VOID _sm_send_permit_to_source(SM_PLAYBACK_SOURCE_E e_source, UINT32 ui2_id, void *msg, size_t msg_len)
{
    INT32 i4_ret = SMR_OK;
    HANDLE_T h_app = NULL_HANDLE;

    if (SOURCE_NONE == e_source)
    {
        return;
    }

    i4_ret = u_am_get_app_handle_from_name(&h_app, g_t_source_list[e_source].sz_thread_name);
    if (0 != i4_ret)
    {
        SM_ERR("u_am_get_app_handle_from_name failed! ps_name:%s\n", g_t_source_list[e_source].sz_thread_name);
    }

    i4_ret = u_app_send_appmsg(h_app,
                            E_APP_MSG_TYPE_STATE_MNGR,
                            MSG_FROM_SM,
                            SM_MAKE_MSG(SM_PERMIT_GRP, SM_BODY_PLAY, ui2_id),
                            msg,
                            msg_len);
    if (0 != i4_ret)
    {
        SM_ERR("u_app_send_msg failed! h_app:%p\n", h_app);
    }
}

static VOID _sm_send_forbid_to_source(SM_PLAYBACK_SOURCE_E e_source)
{
    INT32 i4_ret = SMR_OK;
    HANDLE_T h_app = NULL_HANDLE;

    if (SOURCE_NONE == e_source)
    {
        return;
    }

    i4_ret = u_am_get_app_handle_from_name(&h_app, g_t_source_list[e_source].sz_thread_name);
    if (0 != i4_ret)
    {
        SM_ERR("u_am_get_app_handle_from_name failed! ps_name:%s\n", g_t_source_list[e_source].sz_thread_name);
    }

    i4_ret = u_app_send_appmsg(h_app,
                            E_APP_MSG_TYPE_STATE_MNGR,
                            MSG_FROM_SM,
                            SM_MAKE_MSG(SM_FORBID_GRP, SM_BODY_PLAY, 0),
                            NULL,
                            0);
    if (0 != i4_ret)
    {
        SM_ERR("u_app_send_msg failed! h_app:%p\n", h_app);
    }
}

static VOID _sm_send_play_to_source(SM_PLAYBACK_SOURCE_E e_source, UINT32 ui2_id)
{
    APPMSG_T t_msg;

    if (SOURCE_NONE == e_source)
    {
        return;
    }

    t_msg.ui4_sender_id = MSG_FROM_SM;
    t_msg.ui4_msg_type = SM_MAKE_MSG(SM_REQUEST_GRP, SM_BODY_PLAY, ui2_id);
    _sm_send_msg(t_msg.ui4_sender_id,t_msg.ui4_msg_type, g_t_source_list[e_source].sz_thread_name);
}

static VOID _sm_send_stop_to_source(SM_PLAYBACK_SOURCE_E e_source)
{
    APPMSG_T t_msg;

    if (SOURCE_NONE == e_source)
    {
        return;
    }

    t_msg.ui4_sender_id = MSG_FROM_SM;
    t_msg.ui4_msg_type = SM_MAKE_MSG(SM_REQUEST_GRP, SM_BODY_STOP, 0);
    _sm_send_msg(t_msg.ui4_sender_id,t_msg.ui4_msg_type, g_t_source_list[e_source].sz_thread_name);
}

static VOID _sm_send_pause_to_source(SM_PLAYBACK_SOURCE_E e_source)
{
    APPMSG_T t_msg;

    t_msg.ui4_sender_id = MSG_FROM_SM;
    t_msg.ui4_msg_type = SM_MAKE_MSG(SM_REQUEST_GRP, SM_BODY_PAUSE, 0);

    _sm_send_msg(t_msg.ui4_sender_id,
                t_msg.ui4_msg_type,
                g_t_source_list[e_source].sz_thread_name);

}

static VOID _sm_send_resume_to_source(SM_PLAYBACK_SOURCE_E e_source)
{
    APPMSG_T t_msg;

    t_msg.ui4_sender_id = MSG_FROM_SM;
    t_msg.ui4_msg_type = SM_MAKE_MSG(SM_REQUEST_GRP, SM_BODY_RESUME, 0);

    _sm_send_msg(t_msg.ui4_sender_id,
                t_msg.ui4_msg_type,
                g_t_source_list[e_source].sz_thread_name);
}

static VOID _sm_send_permit_to_prompt(VOID)
{
    SM_INFO("permit prompt play, id:[%d]!\n", g_t_sm.t_prompt_source.ui2_id);

    g_t_sm.t_prompt_source.b_wait_permit = FALSE;
    _sm_send_permit_to_source(SOURCE_PROMPT,
                                g_t_sm.t_prompt_source.ui2_id,
                                &(g_t_sm.t_prompt_source.t_prompt_play),
                                sizeof(g_t_sm.t_prompt_source.t_prompt_play));
}

static VOID _sm_send_permit_to_tts(VOID)
{
    SM_INFO("permit tts play, id:[%d], needMix:[%d]!\n",    \
        g_t_sm.t_tts_source.ui2_id, g_t_sm.t_tts_source.t_tts_play.need_mix);

    g_t_sm.t_tts_source.b_wait_permit = FALSE;
    _sm_send_permit_to_source(SOURCE_TTS,
                                g_t_sm.t_tts_source.ui2_id,
                                &(g_t_sm.t_tts_source.t_tts_play),
                                sizeof(g_t_sm.t_tts_source.t_tts_play));
}

static VOID _sm_send_permit_to_bt(VOID)
{
    UINT16 ui2_id = g_t_sm.t_bt_source.ui2_bt_id;

    SM_INFO("permit bt[%d] play!\n", ui2_id);

    g_t_sm.t_bt_source.e_wait_permit = BT_NOT_PERMIT;
    g_t_sm.t_bt_source.b_suspend = FALSE;
    _sm_send_permit_to_source(SOURCE_BT, ui2_id, NULL, 0);
    g_t_sm.t_bt_source.ui4_status = SM_BODY_PLAY;
}

static VOID _sm_send_permit_to_bt_hfp(VOID)
{
    UINT16 ui2_id = g_t_sm.t_bt_hfp_source.ui2_bt_hfp_id;

    SM_INFO("permit bt hfp[%d] play!\n", ui2_id);

    g_t_sm.t_bt_hfp_source.b_wait_permit = FALSE;
    _sm_send_permit_to_source(SOURCE_BT_HFP, ui2_id, NULL, 0);
}

static VOID _sm_send_permit_to_dlna(VOID)
{
    UINT16 ui2_id = g_t_sm.t_dlna_source.ui2_dlna_id;

    SM_INFO("permit dlna play, dlna id: %d!\n", ui2_id);

    g_t_sm.t_dlna_source.b_wait_permit = FALSE;
    _sm_send_permit_to_source(SOURCE_DLNA, ui2_id, NULL, 0);
}

static VOID _sm_send_prev_to_bt(UINT32 ui2_id)
{
    APPMSG_T t_msg;
    t_msg.ui4_sender_id = MSG_FROM_SM;
    t_msg.ui4_msg_type = SM_MAKE_MSG(SM_REQUEST_GRP, SM_BODY_PREV, ui2_id);
    _sm_send_msg(t_msg.ui4_sender_id,t_msg.ui4_msg_type, BLUETOOTH_THREAD_NAME);
}

static VOID _sm_send_next_to_bt(UINT32 ui2_id)
{
    APPMSG_T t_msg;
    t_msg.ui4_sender_id = MSG_FROM_SM;
    t_msg.ui4_msg_type = SM_MAKE_MSG(SM_REQUEST_GRP, SM_BODY_NEXT, ui2_id);
    _sm_send_msg(t_msg.ui4_sender_id,t_msg.ui4_msg_type, BLUETOOTH_THREAD_NAME);
}

static VOID _sm_send_play_to_bt(VOID)
{
    UINT16 ui2_id = g_t_sm.t_bt_source.ui2_bt_id;

    SM_INFO("send bt[%d] e_wait_permit[%d]!\n", ui2_id, g_t_sm.t_bt_source.e_wait_permit);

    switch (g_t_sm.t_bt_source.e_wait_permit)
    {
        case BT_WAIT_PERMIT:
            _sm_send_play_to_source(SOURCE_BT, ui2_id);
            break;
        case BT_WAIT_PREV:
            _sm_send_prev_to_bt(ui2_id);
            break;
        case BT_WAIT_NEXT:
            _sm_send_next_to_bt(ui2_id);
            break;
        default:
            SM_ERR("error e_wait_permit:[%d]\n", g_t_sm.t_bt_source.e_wait_permit);
            break;
    }

    g_t_sm.t_bt_source.e_wait_permit = BT_NOT_PERMIT;
    g_t_sm.t_bt_source.b_suspend = FALSE;
    g_t_sm.t_bt_source.ui4_status = SM_BODY_PLAY;
}

static VOID _sm_send_forbid_to_bt(VOID)
{
    SM_INFO("forbid bt play!\n");
    _sm_send_forbid_to_source(SOURCE_BT);
}

static VOID _sm_send_forbid_to_bt_hfp(VOID)
{
    SM_INFO("forbid bt play!\n");
    _sm_send_forbid_to_source(SOURCE_BT_HFP);
}

static VOID _sm_send_forbid_to_dlna(VOID)
{
    SM_INFO("forbid dlna play!\n");
    _sm_send_forbid_to_source(SOURCE_DLNA);
}

static VOID _sm_send_permit_to_uri(VOID)
{
    UINT16 ui2_id = g_t_sm.t_uri_source.ui2_uri_id;

    SM_INFO("permit uri[%d] play!\n", ui2_id);

    g_t_sm.t_uri_source.b_wait_permit = FALSE;
    g_t_sm.t_uri_source.e_suspend_mode = URI_NOT_SUSPEND;
    _sm_send_permit_to_source(SOURCE_URI,
                                ui2_id,
                                &(g_t_sm.t_uri_source.t_uri_play),
                                sizeof(g_t_sm.t_uri_source.t_uri_play));
    g_t_sm.t_uri_source.ui4_status = SM_BODY_PLAY;
}

static VOID _sm_send_pause_to_bt(VOID)
{
    g_t_sm.t_bt_source.b_suspend = FALSE;
    if (!_sm_is_bt_playing())
    {
        return;
    }

    SM_INFO("send pause to bt\n");

    _sm_send_pause_to_source(SOURCE_BT);
    g_t_sm.t_bt_source.ui4_status = SM_BODY_PAUSE;
}

static VOID _sm_send_pause_to_dlna(VOID)
{
    g_t_sm.t_dlna_source.b_suspend = FALSE;
    if (!_sm_is_dlna_playing())
    {
        return;
    }

    SM_INFO("send pause to dlna!\n");

    _sm_send_pause_to_source(SOURCE_DLNA);
    g_t_sm.t_dlna_source.ui4_status = SM_BODY_PAUSE;
}

static VOID _sm_send_pause_to_uri(VOID)
{
    g_t_sm.t_uri_source.e_suspend_mode = URI_NOT_SUSPEND;
    if (!_sm_is_uri_playing())
    {
        return;
    }

    SM_INFO("send pause to uri\n");

    _sm_send_pause_to_source(SOURCE_URI);
    g_t_sm.t_uri_source.ui4_status = SM_BODY_PAUSE;
}

static VOID _sm_send_suspend_to_bt(VOID)
{
    if (!_sm_is_bt_playing())
    {
        return;
    }

    SM_INFO("send suspend to bt\n");

    _sm_send_pause_to_source(SOURCE_BT);
    g_t_sm.t_bt_source.b_suspend = TRUE;
    g_t_sm.t_bt_source.ui4_status = SM_BODY_PAUSE;
}

static VOID _sm_send_suspend_to_dlna(VOID)
{
    if (!_sm_is_dlna_playing())
    {
        return;
    }

    SM_INFO("send suspend to dlna!\n");

    _sm_send_pause_to_source(SOURCE_DLNA);
    g_t_sm.t_dlna_source.b_suspend = TRUE;
    g_t_sm.t_dlna_source.ui4_status = SM_BODY_PAUSE;
}

static VOID _sm_send_suspend_to_uri(URI_SUSPEND_MODE_E e_suspend_mode)
{
    if (!_sm_is_uri_playing())
    {
        return;
    }

    SM_INFO("send suspend to uri\n");

    _sm_send_pause_to_source(SOURCE_URI);
    g_t_sm.t_uri_source.e_suspend_mode = e_suspend_mode; //URI_SUSPEND_BY_SWITCH;
    g_t_sm.t_uri_source.ui4_status = SM_BODY_PAUSE;
}

static VOID _sm_send_resume_to_bt(VOID)
{
    if (!g_t_sm.t_bt_source.e_bt_source)
    {
        return;
    }

    SM_INFO("send resume to bt!\n");

    _sm_send_resume_to_source(SOURCE_BT);
    g_t_sm.t_bt_source.b_suspend = FALSE;
    g_t_sm.t_bt_source.ui4_status = SM_BODY_PLAY;
}

static VOID _sm_send_resume_to_dlna(VOID)
{
    if (!g_t_sm.t_dlna_source.e_dlna_source)
    {
        return;
    }

    SM_INFO("send resume to dlna!\n");

    _sm_send_resume_to_source(SOURCE_DLNA);
    g_t_sm.t_dlna_source.b_suspend = FALSE;
    g_t_sm.t_dlna_source.ui4_status = SM_BODY_PLAY;
}

static VOID _sm_send_resume_to_uri(VOID)
{
    if (!g_t_sm.t_uri_source.e_uri_source)
    {
        return;
    }

    SM_INFO("send resume to uri!\n");

    _sm_send_resume_to_source(SOURCE_URI);
    g_t_sm.t_uri_source.e_suspend_mode = URI_NOT_SUSPEND;
    g_t_sm.t_uri_source.ui4_status = SM_BODY_PLAY;
}

static VOID _sm_clear_tts(VOID)
{
    if (_sm_is_tts_exist())
    {
        if (_sm_is_tts_wait_permit())
        {
            SM_INFO("stop tts permit!\n");
            g_t_sm.t_tts_source.e_tts_source = SOURCE_NONE;
            g_t_sm.t_tts_source.b_wait_permit = FALSE;
        }
        else
        {
            SM_INFO("stop tts!\n");
            _sm_send_stop_to_source(SOURCE_TTS);
        }
    }
}

static VOID _sm_clear_prompt(VOID)
{
    if (_sm_is_prompt_exist())
    {
        if (_sm_is_prompt_wait_permit())
        {
            SM_INFO("stop prompt permit!\n");
            g_t_sm.t_prompt_source.e_prompt_source = SOURCE_NONE;
            g_t_sm.t_prompt_source.b_wait_permit = FALSE;
        }
        else
        {
            SM_INFO("stop prompt!\n");
            _sm_send_stop_to_source(SOURCE_PROMPT);
        }
    }
}

static VOID _sm_clear_bt(VOID)
{
    if (_sm_is_bt_exist())
    {
        if (_sm_is_bt_wait_permit())
        {
            SM_INFO("stop bt permit!\n");
            g_t_sm.t_bt_source.e_bt_source = SOURCE_NONE;
            g_t_sm.t_bt_source.e_wait_permit = BT_NOT_PERMIT;
        }
        else
        {
            SM_INFO("stop bt!\n");
            _sm_send_stop_to_source(SOURCE_BT);
        }
    }
}

static VOID _sm_clear_bt_hfp(VOID)
{
    if (_sm_is_bt_hfp_exist())
    {
        if (_sm_is_bt_hfp_wait_permit())
        {
            SM_INFO("stop bt hfp permit!\n");
            g_t_sm.t_bt_hfp_source.e_bt_hfp_source = SOURCE_NONE;
            g_t_sm.t_bt_hfp_source.b_wait_permit = FALSE;
        }
        else
        {
            SM_INFO("stop bt hfp!\n");
            _sm_send_stop_to_source(SOURCE_BT_HFP);
        }
    }
}

static VOID _sm_clear_dlna(VOID)
{
    if (_sm_is_dlna_exist())
    {
        if (_sm_is_dlna_wait_permit())
        {
            SM_INFO("stop dlna permit!\n");
            g_t_sm.t_dlna_source.e_dlna_source = SOURCE_NONE;
            g_t_sm.t_dlna_source.b_wait_permit = FALSE;
        }
        else
        {
            SM_INFO("stop dlna!\n");
            _sm_send_stop_to_source(SOURCE_DLNA);
        }
    }
}

static VOID _sm_clear_uri(VOID)
{
    if (_sm_is_uri_exist())
    {
        if (_sm_is_uri_wait_permit())
        {
            SM_INFO("stop uri permit!\n");
            g_t_sm.t_uri_source.e_uri_source = SOURCE_NONE;
            g_t_sm.t_uri_source.b_wait_permit = FALSE;
        }
        else
        {
            SM_INFO("stop uri!\n");
            _sm_send_stop_to_source(SOURCE_URI);
        }
    }
}

static VOID _sm_send_play_pause_to_source(SM_PLAYBACK_SOURCE_E e_source)
{
    APPMSG_T t_msg;

    if (SOURCE_NONE == e_source)
    {
        return;
    }

    t_msg.ui4_sender_id = MSG_FROM_SM;
    t_msg.ui4_msg_type = SM_MAKE_MSG(SM_REQUEST_GRP, SM_BODY_PAUSE, 0);
    _sm_send_msg(t_msg.ui4_sender_id,t_msg.ui4_msg_type, g_t_source_list[e_source].sz_thread_name);
}

static VOID _sm_send_next_to_source(SM_PLAYBACK_SOURCE_E e_source)
{
    APPMSG_T t_msg;

    if (SOURCE_NONE == e_source)
    {
        return;
    }

    t_msg.ui4_sender_id = MSG_FROM_SM;
    t_msg.ui4_msg_type = SM_MAKE_MSG(SM_REQUEST_GRP, SM_BODY_NEXT, 0);
    _sm_send_msg(t_msg.ui4_sender_id,t_msg.ui4_msg_type, g_t_source_list[e_source].sz_thread_name);
}
static VOID _sm_send_prev_to_source(SM_PLAYBACK_SOURCE_E e_source)
{
    APPMSG_T t_msg;

    if (SOURCE_NONE == e_source)
    {
        return;
    }

    t_msg.ui4_sender_id = MSG_FROM_SM;
    t_msg.ui4_msg_type = SM_MAKE_MSG(SM_REQUEST_GRP, SM_BODY_PREV, 0);
    _sm_send_msg(t_msg.ui4_sender_id,t_msg.ui4_msg_type, g_t_source_list[e_source].sz_thread_name);
}

UINT32 _sm_get_msg_from_source(SM_PLAYBACK_SOURCE_E e_source)
{
    switch(e_source)
    {
        case SOURCE_URI :
            return MSG_FROM_URI;
        case SOURCE_BT:
            return MSG_FROM_BT;
        case SOURCE_TTS:
            return MSG_FROM_TTS;
        case SOURCE_PROMPT:
            return MSG_FROM_PROMPT;
        case SOURCE_BT_HFP:
            return MSG_FROM_BT_HFP;
        case SOURCE_DLNA:
            return MSG_FROM_DLNA;
        default:
            break;
    }
    return MSG_FROM_SM;
}

static SM_PLAYBACK_SOURCE_E _sm_get_source_from_msg(UINT32 ui4_sender_id)
{
    switch(ui4_sender_id)
    {
        case MSG_FROM_URI:
            return SOURCE_URI;
        case MSG_FROM_BT:
            return SOURCE_BT;
        case MSG_FROM_TTS:
            return SOURCE_TTS;
        case MSG_FROM_PROMPT:
            return SOURCE_PROMPT;
        case MSG_FROM_BT_HFP:
            return SOURCE_BT_HFP;
        case MSG_FROM_DLNA:
            return SOURCE_DLNA;
        default:
            break;
    }
    return SOURCE_NONE;
}

static INT32 _sm_send_msg_to_assistant_stub(ASSISTANT_STUB_CMD_INDEX_E e_cmd, VOID *pt_send_msg,UINT32 msg_size)
{
    INT32 i4_ret = SMR_FAIL;
    HANDLE_T h_app = NULL_HANDLE;

    u_am_get_app_handle_from_name(&h_app,ASSISTANT_STUB_THREAD_NAME);

    i4_ret = u_app_send_appmsg(h_app,
                                E_APP_MSG_TYPE_ASSISTANT_STUB,
                                MSG_FROM_SM,
                                e_cmd,
                                pt_send_msg,
                                msg_size);
    if (AEER_OK != i4_ret)
    {
        SM_ERR("_sm_send_msg_to_assistant_stub failed, i4_ret[%ld]\n", i4_ret);
    }

    return (i4_ret == AEER_OK ? SMR_OK : SMR_FAIL);
}

static INT32 _sm_send_msg_to_upg_control(SM_MSG_BDY_E UPG_BODY)
{
    INT32 i4_ret = SMR_FAIL;
    HANDLE_T h_app = NULL_HANDLE;
    
    u_am_get_app_handle_from_name(&h_app, UPG_CONTROL_THREAD_NAME);

    i4_ret = u_app_send_appmsg(h_app,
                                E_APP_MSG_TYPE_STATE_MNGR,
                                MSG_FROM_SM,
                                UPG_BODY,
                                NULL,
                                0);
    if (AEER_OK != i4_ret)
    {
        SM_ERR("_sm_send_msg_to_upg_control failed, i4_ret[%ld]\n", i4_ret);
    }
    
    return (i4_ret == AEER_OK ? SMR_OK : SMR_FAIL);
}

INT32 _sm_factory_reset_process(VOID)
{
    INT8 i4_ret = 0;
    if(u_ui_get_factory_reset_flag())
    {
        SM_ERR("%s\n",__FUNCTION__);

        while(!_sm_is_factory_reset_down())
        {
            sleep(1);
            SM_ERR("wait custom clear down.\n");
        }
        _sm_set_factory_reset(FALSE);
        u_ui_set_factory_reset_flag(FALSE);
        i4_ret = u_acfg_factory_reset();
        if(0 != i4_ret)
        {
            SM_ERR("u_acfg_factory_reset failed, i4_ret[%d]\n", i4_ret);
        }
    }
    return SMR_OK;
}

static INT32 _sm_actory_reset_thread(VOID)
{
    INT32 i4_ret;
    pthread_t ntid;
    pthread_attr_t t_attr;

    i4_ret = pthread_attr_init(&t_attr);
    if(0 != i4_ret)
    {
        SM_ERR("pthread_attr_init error!\n");
        goto ATTR_INIT_ERR;
    }

    i4_ret = pthread_attr_setdetachstate(&t_attr, PTHREAD_CREATE_DETACHED);
    if(0 != i4_ret)
    {
        SM_ERR("pthread_attr_setdetachstate error!\n");
        goto ATTR_SET_ERR;
    }

    i4_ret = pthread_create(&ntid, &t_attr, _sm_factory_reset_process, NULL);
    if(0 != i4_ret)
    {
        SM_ERR("pthread_create error!\n");
        goto ATTR_SET_ERR;
    }

    pthread_attr_destroy(&t_attr);
    return SMR_OK;

ATTR_SET_ERR:
    pthread_attr_destroy(&t_attr);
ATTR_INIT_ERR:

    return SMR_FAIL;
}

static BOOL _sm_process_prompt_priority_play(VOID)
{
    if (_sm_is_prompt_wait_permit())
    {
        SM_INFO("permit prompt play!\n");
        _sm_send_permit_to_prompt();
        return TRUE;
    }
    return FALSE;
}

static BOOL _sm_process_tts_priority_play(VOID)
{
    if (_sm_is_tts_wait_permit())
    {
        SM_INFO("permit tts play!\n");
        _sm_send_permit_to_tts();
        if (!g_t_sm.t_tts_source.t_tts_play.need_mix)
        {
            return TRUE;
        }
    }
    return FALSE;
}

static BOOL _sm_process_bt_priority_play(VOID)
{
    //if bt playing, bt clear prompt or tts
    if (_sm_is_bt_playing())
    {
        return TRUE;
    }

    //if wait permit, send permit
    if (_sm_is_bt_wait_permit())
    {
        _sm_send_play_to_bt();
        return TRUE;
    }

    //if suspend, send resume
    if (_sm_is_bt_suspend())
    {
        _sm_send_resume_to_bt();
        return TRUE;
    }

    return FALSE;
}

static BOOL _sm_process_bt_hfp_priority_play(VOID)
{
    //if bt hfp playing, return
    if (_sm_is_bt_hfp_playing())
    {
        return TRUE;
    }

    //if wait permit, send permit
    if (_sm_is_bt_hfp_wait_permit())
    {
        _sm_send_permit_to_bt_hfp();
        return TRUE;
    }

    return FALSE;
}

static BOOL _sm_process_uri_priority_play(VOID)
{
    //if wait permit, send permit
    if (_sm_is_uri_wait_permit())
    {
        _sm_send_permit_to_uri();
        return TRUE;
    }

    //if suspend, send resume
    if (_sm_is_uri_suspend())
    {
        _sm_send_resume_to_uri();
        return TRUE;
    }
    return FALSE;
}

static BOOL _sm_process_dlna_priority_play(VOID)
{
    //if dlna playing, dlna clear prompt or tts
    if (_sm_is_dlna_playing())
    {
        return TRUE;
    }

    //if wait permit, send permit
    if (_sm_is_dlna_wait_permit())
    {
        _sm_send_permit_to_dlna();
        return TRUE;
    }

    //if suspend, send resume
    if (_sm_is_dlna_suspend())
    {
        _sm_send_resume_to_dlna();
        return TRUE;
    }

    return FALSE;
}

static VOID _sm_process_priority_play(BOOL speech_enable)
{
    if (_sm_is_bt_hfp_playing())
    {
        SM_INFO("bt HFP running, return!\n");
        return;
    }

    if (g_t_sm.b_speech_start)
    {
        SM_INFO("speech starting, return!\n");
        return;
    }

    if (_sm_is_prompt_playing() || _sm_is_tts_playing())
    {
        SM_INFO("prompt or tts playing!\n");
        return;
    }

    //return TRUE means: permit bt hfp play
    if (_sm_process_bt_hfp_priority_play())
    {
        SM_INFO("priority bt hfp play!\n");
        return;
    }

    //return TRUE means: permit prompt play
    if (_sm_process_prompt_priority_play())
    {
        SM_INFO("priority prompt play!\n");
        return;
    }

    //return TRUE means: permit no need mix tts play
    if (_sm_process_tts_priority_play())
    {
        SM_INFO("priority tts play!\n");
        return;
    }

    if (speech_enable)
    {
        SM_INFO("speech enable, don't resume or permit bt/uri!\n");
        return;
    }

    //return TRUE means: permit bt play || resume bt play
    if (_sm_process_bt_priority_play())
    {
        SM_INFO("priority bt play!\n");
        return;
    }

     //return TRUE means: permit dlna play || resume dlna play
    if (_sm_process_dlna_priority_play())
    {
        SM_INFO("priority dlna play!\n");
        return;
    }

    //return TRUE means: permit uri play || resume uri play
    if (_sm_process_uri_priority_play())
    {
        SM_INFO("priority uri play!\n");
        return;
    }
    SM_INFO("priority nothing play!\n");
}

static VOID _sm_process_bt_request_play(VOID)
{
    SM_MSG("bt request play!\n");

    _sm_exit_fake_standby_mode();
    _sm_exit_suspend_standby_mode();

    if(u_ui_get_factory_reset_flag())
    {
        SM_INFO("factory reset going, return!\n");
        _sm_send_forbid_to_bt();
        return;
    }

    if (g_t_sm.b_speech_start)
    {
        SM_INFO("speech starting, forbid bt request!\n");
        _sm_send_forbid_to_bt();
        return;
    }

    g_t_sm.t_bt_source.e_bt_source = SOURCE_BT;
    g_t_sm.t_bt_source.ui2_bt_id++;
    g_t_sm.t_bt_source.e_wait_permit = BT_WAIT_PERMIT;

    if(_sm_is_dlna_playing()||
        _sm_is_uri_playing()||
        _sm_is_tts_playing()||
        _sm_is_prompt_playing())
    {
        SM_INFO("[source switch]uri/tts/prompt/dlna is playing now! First, stop.\n");
        _sm_clear_tts();
        _sm_clear_prompt();
        _sm_clear_uri();
        _sm_clear_dlna();
        return;
    }

    _sm_send_permit_to_bt();    //question?two permit? different between permit and play
}

static VOID _sm_process_dlna_request_play(VOID)
{
    SM_MSG("dlna request play!\n");

    _sm_exit_fake_standby_mode();
    _sm_exit_suspend_standby_mode();

    if(u_ui_get_factory_reset_flag())
    {
        SM_INFO("factory reset going, return!\n");
        _sm_send_forbid_to_dlna();
        return;
    }

    if (g_t_sm.b_speech_start)
    {
        SM_INFO("speech starting, forbid dlna request!\n");
        _sm_send_forbid_to_dlna();
        return;
    }

    g_t_sm.t_dlna_source.e_dlna_source = SOURCE_DLNA;
    g_t_sm.t_dlna_source.ui2_dlna_id = 0;
    g_t_sm.t_dlna_source.b_wait_permit = TRUE;
 
    if(_sm_is_bt_playing()||
        _sm_is_uri_playing()||
        _sm_is_tts_playing()||
        _sm_is_prompt_playing())
    {
        SM_INFO("[source switch]uri/tts/prompt/bt is playing now! First, stop.\n");
        _sm_clear_tts();
        _sm_clear_prompt();
        _sm_clear_uri();
        _sm_clear_bt();
        return;
    }
    
    _sm_send_permit_to_dlna();
}

static VOID _sm_process_bt_hfp_request_play(VOID)
{
    SM_MSG("<%s>enter!\n",__FUNCTION__);

    if(u_ui_get_factory_reset_flag())
    {
        SM_INFO("factory reset going, return!\n");
        _sm_send_forbid_to_bt_hfp();
        return;
    }

    g_t_sm.t_bt_hfp_source.e_bt_hfp_source = SOURCE_BT_HFP;
    g_t_sm.t_bt_hfp_source.ui2_bt_hfp_id++;
    g_t_sm.t_bt_hfp_source.b_wait_permit = TRUE;

    if(_sm_is_dlna_playing()||
        _sm_is_uri_playing()||
        _sm_is_tts_playing()||
        _sm_is_prompt_playing())
    {
        SM_INFO("[source switch]uri/tts/prompt/dlna is playing now! First, stop.\n");
        _sm_clear_tts();
        _sm_clear_prompt();
        _sm_clear_uri();
        _sm_clear_dlna();
        return;
    }

    if (_sm_is_bt_playing())
    {
        SM_INFO("[source switch]bt is playing now!Don't need to process bt hfp.\n");
    }
    _sm_send_permit_to_bt_hfp();
    return;
}

static VOID _sm_process_pri_bt_hfp_request_play(VOID)
{
    ASSISTANT_STUB_HFP_STATUS_CHANGE_T hfp_status_change = {0};

    SM_MSG("<%s>enter!\n",__FUNCTION__);
    _sm_exit_fake_standby_mode();

    SM_MSG("<%s>wait Assistantcenter free MIC!\n",__FUNCTION__);
    strncpy(hfp_status_change.command,"/system/hfp_status_change",ASSISTANT_STUB_COMMAND_MAX_LENGTH);
    strncpy(hfp_status_change.status,"hfp_request",ASSISTANT_STUB_STATUS_MAX_LENGTH);
    _sm_send_msg_to_assistant_stub(ASSISTANT_STUB_CMD_HFP_STATUS_CHANGE, &hfp_status_change,sizeof(ASSISTANT_STUB_HFP_STATUS_CHANGE_T));

    return;
}
static VOID _sm_process_request_play_msg(UINT32 ui4_sender_id)
{
    switch (ui4_sender_id)
    {
        case MSG_FROM_BT:
            _sm_process_bt_request_play();
            break;
        case MSG_FROM_BT_HFP:
            if(_sm_is_process_exist("Aispeech"))
            {
                //after Assistant auto start, use pri API
                _sm_process_pri_bt_hfp_request_play();
            }
            else
            {
                _sm_process_bt_hfp_request_play();
            }
            break;
        case MSG_FROM_DLNA:
            _sm_process_dlna_request_play();
            break;
        default:
            SM_ERR("[%d] is invalid play request id!\n", ui4_sender_id);
            break;
    }
}

static VOID _sm_process_request_pause_msg(UINT32 ui4_sender_id)
{   
    switch (ui4_sender_id)
        {
            case MSG_FROM_DLNA:
                _sm_send_pause_to_dlna();
                break;
            default:
                SM_ERR("[%d] is invalid pause request id!\n", ui4_sender_id);
                break;
        }
}

static VOID _sm_process_request_stop_msg(UINT32 ui4_sender_id)
{   
    switch (ui4_sender_id)
        {
            case MSG_FROM_DLNA:
                _sm_clear_dlna();
                break;
            default:
                SM_ERR("[%d] is invalid stop request id!\n", ui4_sender_id);
                break;
        }
}

static VOID _sm_process_inform_play_msg(UINT32 ui4_sender_id, UINT32 msg_id)
{
    SM_PLAYBACK_SOURCE_E e_source;

    e_source = _sm_get_source_from_msg(ui4_sender_id);
    if (SOURCE_NONE == e_source)
    {
        SM_ERR("%d is not valid sender id!\n", ui4_sender_id);
        return;
    }

    switch (e_source)
    {
        case SOURCE_BT:
            if (!_sm_is_bt_exist())
            {
                return;
            }
            if (g_t_sm.t_bt_source.ui2_bt_id == msg_id)
            {
                g_t_sm.t_bt_source.ui4_status = SM_BODY_PLAY;
                SM_INFO("[source switch]bt is playing now!\n");
            }
            break;
        case SOURCE_URI:
            if (!_sm_is_uri_exist())
            {
                return;
            }
            if (g_t_sm.t_uri_source.ui2_uri_id == msg_id)
            {
                g_t_sm.t_uri_source.ui4_status = SM_BODY_PLAY;
                SM_INFO("[source switch]uri is playing now!\n");
            }
            break;
         case SOURCE_BT_HFP:
             if (!_sm_is_bt_hfp_exist())
             {
                 SM_INFO("[source switch]bt hfp does not exist!\n");
                 return;
             }
             if (g_t_sm.t_bt_hfp_source.ui2_bt_hfp_id == msg_id)
             {
                 g_t_sm.t_bt_hfp_source.ui4_status = SM_BODY_PLAY;
                 SM_INFO("[source switch]bt hfp is playing now!\n");
             }
             break;
         case SOURCE_DLNA:
             if (!_sm_is_dlna_exist())
             {
                 SM_INFO("[source switch]dlna does not exist!\n");
                 return;
             }
             if (g_t_sm.t_dlna_source.ui2_dlna_id == msg_id)
             {
                 g_t_sm.t_dlna_source.ui4_status = SM_BODY_PLAY;
                 SM_INFO("[source switch]dlna is playing now!\n");
             }
             break;
    }
}

static VOID _sm_process_inform_pause_msg(UINT32 ui4_sender_id, UINT32 msg_id)
{
    SM_PLAYBACK_SOURCE_E e_source;

    e_source = _sm_get_source_from_msg(ui4_sender_id);
    if (SOURCE_NONE == e_source)
    {
        SM_ERR("%d is not valid sender id!\n", ui4_sender_id);
        return;
    }

    switch (e_source)
    {
        case SOURCE_BT:
            if (!_sm_is_bt_exist())
            {
                return;
            }
            if (g_t_sm.t_bt_source.ui2_bt_id == msg_id)
            {
                g_t_sm.t_bt_source.ui4_status = SM_BODY_PAUSE;
                SM_INFO("[source switch]bt is pause now!\n");
            }
            break;
        case SOURCE_URI:
            if (!_sm_is_uri_exist())
            {
                return;
            }
            if (g_t_sm.t_uri_source.ui2_uri_id == msg_id)
            {
                g_t_sm.t_uri_source.ui4_status = SM_BODY_PAUSE;
                SM_INFO("[source switch]uri is pause now!\n");
            }
            break;
        case SOURCE_DLNA:
            if (!_sm_is_dlna_exist())
            {
                return;
            }
            if (g_t_sm.t_dlna_source.ui2_dlna_id == msg_id)
            {
                g_t_sm.t_dlna_source.ui4_status = SM_BODY_PAUSE;
                SM_INFO("[source switch]dlna is pause now!\n");
            }
            break;
        default:
            SM_ERR("%d is not valid sender pause id!\n", ui4_sender_id);
            return;
    }

    _sm_process_priority_play(FALSE);
}

static VOID _sm_process_inform_stop_msg(UINT32 ui4_sender_id, UINT32 msg_id, INT8 *msg_param)
{
    BOOL speech_enable = FALSE;
    SM_PLAYBACK_SOURCE_E e_source;
    ASSISTANT_STUB_HFP_STATUS_CHANGE_T hfp_status_change = {0};

    SM_INFO("stop msg comming, sender id:[%d] msg id:[%d]\n", ui4_sender_id, msg_id);

    e_source = _sm_get_source_from_msg(ui4_sender_id);
    if (SOURCE_NONE == e_source)
    {
        SM_ERR("%d is not valid sender id!\n", ui4_sender_id);
        return;
    }

    switch (e_source)
    {
        case SOURCE_PROMPT:
            if (msg_id == g_t_sm.t_prompt_source.ui2_id)
            {
                SM_INFO("[source switch]prompt is stop now!\n", e_source);
                g_t_sm.t_prompt_source.e_prompt_source = SOURCE_NONE;
                g_t_sm.t_prompt_source.b_wait_permit = FALSE;

                //_sm_actory_reset_thread();
            }
            break;
        case SOURCE_TTS:
            if (msg_id == g_t_sm.t_tts_source.ui2_id)
            {
                SM_INFO("[source switch]tts is stop now!\n", e_source);
                g_t_sm.t_tts_source.e_tts_source = SOURCE_NONE;
                g_t_sm.t_tts_source.b_wait_permit = FALSE;
                speech_enable = ((SM_PARAM_T *)msg_param)->speech_enable;
            }
            break;
        case SOURCE_BT:
            if (msg_id == g_t_sm.t_bt_source.ui2_bt_id)
            {
                SM_INFO("[source switch]bt is stop now!\n", e_source);
                g_t_sm.t_bt_source.e_wait_permit = BT_NOT_PERMIT;
                g_t_sm.t_bt_source.b_suspend = FALSE;
                g_t_sm.t_bt_source.e_bt_source = SOURCE_NONE;
                g_t_sm.t_bt_source.ui4_status = SM_BODY_STOP;
            }
            break;
        case SOURCE_URI:
            if (msg_id == g_t_sm.t_uri_source.ui2_uri_id)
            {
                SM_INFO("[source switch]uri is stop now!\n", e_source);
                g_t_sm.t_uri_source.b_wait_permit = FALSE;
                g_t_sm.t_uri_source.e_suspend_mode = FALSE;
                g_t_sm.t_uri_source.e_uri_source = SOURCE_NONE;
                g_t_sm.t_uri_source.ui4_status = SM_BODY_STOP;
            }
            break;
        case SOURCE_BT_HFP:
            if (msg_id == g_t_sm.t_bt_hfp_source.ui2_bt_hfp_id)
            {
                SM_INFO("[source switch]bt hfp is stop now!\n");
                g_t_sm.t_bt_hfp_source.e_bt_hfp_source = SOURCE_NONE;
                g_t_sm.t_bt_hfp_source.ui4_status = SM_BODY_STOP;
                g_t_sm.t_bt_hfp_source.b_wait_permit = FALSE;
                   
                strncpy(hfp_status_change.command,"/system/hfp_status_change",ASSISTANT_STUB_COMMAND_MAX_LENGTH);
                strncpy(hfp_status_change.status,"hfp_hangup",ASSISTANT_STUB_STATUS_MAX_LENGTH);
                _sm_send_msg_to_assistant_stub(ASSISTANT_STUB_CMD_HFP_STATUS_CHANGE, &hfp_status_change,sizeof(ASSISTANT_STUB_HFP_STATUS_CHANGE_T));
            }
            break;
        case SOURCE_DLNA:
            if (msg_id == g_t_sm.t_dlna_source.ui2_dlna_id)
            {
                SM_INFO("[source switch]dlna is stop now!\n", e_source);
                g_t_sm.t_dlna_source.b_wait_permit = FALSE;
                g_t_sm.t_dlna_source.b_suspend = FALSE;
                g_t_sm.t_dlna_source.e_dlna_source = SOURCE_NONE;
                g_t_sm.t_dlna_source.ui4_status = SM_BODY_STOP;
            }
            break;
        default:
            SM_ERR("unsupport source:[%d]!\n", e_source);
            return;
    }

    if(u_sm_is_enter_suspend_standby())
    {
        _sm_enter_suspend_standby_mode();
    }
    if(u_sm_is_enter_fake_standby())
    {
        _sm_enter_fake_standby_mode();
    }
    
    _sm_process_priority_play(speech_enable);
}

static VOID _sm_process_inform_ui_source_msg(SM_PLAYBACK_SOURCE_E e_source)
{
    INT32 i4_ret = SMR_FAIL;
    HANDLE_T h_app = NULL_HANDLE;

    u_am_get_app_handle_from_name(&h_app,SM_THREAD_NAME);

    i4_ret = u_app_send_appmsg(h_app,
                                E_APP_MSG_TYPE_STATE_MNGR,
                                _sm_get_msg_from_source(e_source),
                                SM_MAKE_MSG(SM_REQUEST_GRP, SM_BODY_PLAY, 0),
                                NULL,
                                0);
    if (AEER_OK != i4_ret)
    {
        SM_ERR("_sm_process_inform_ui_source_msg failed, i4_ret:%ld\n",i4_ret);
    }
}

static VOID _sm_process_inform_long_power_msg(UINT32 ui4_sender_id)
{
    if (MSG_FROM_UI != ui4_sender_id)
    {
        SM_ERR("only ui can send power msg!\n");
        return;
    }
}

static VOID _sm_process_inform_long_source_msg(UINT32 ui4_sender_id)
{
    INT8 i4_ret = 0;

    if (MSG_FROM_UI != ui4_sender_id)
    {
        SM_ERR("only ui can send power msg!\n");
        return;
    }

    //do factory reset
    i4_ret = u_acfg_factory_reset();
    if(0 != i4_ret)
    {
        SM_ERR("execute factory reset fail!\n");
    }
}

static VOID _sm_process_inform_long_mute_msg(UINT32 ui4_sender_id)
{
    if (MSG_FROM_UI != ui4_sender_id)
    {
        SM_ERR("only ui can send power msg!\n");
        return;
    }

    //open or close the Aispeech
}

static VOID _sm_process_inform_long_volumeup_msg(UINT32 ui4_sender_id)
{
    if (MSG_FROM_UI != ui4_sender_id)
    {
        SM_ERR("only ui can send power msg!\n");
        return;
    }

    //enter the Aispeech mode
}

VOID _sm_process_request_msg(APPMSG_T *pMsg)
{
    switch(SM_MSG_BDY(pMsg->ui4_msg_type))
    {
        case SM_BODY_PLAY:
            _sm_process_request_play_msg(pMsg->ui4_sender_id);
            break;
        case SM_BODY_PAUSE:
            _sm_process_request_pause_msg(pMsg->ui4_sender_id);
            break;
        case SM_BODY_STOP:
            _sm_process_request_stop_msg(pMsg->ui4_sender_id);
            break;
        default:
            break;
    }
}

VOID _sm_process_inform_msg(APPMSG_T *pMsg)
{
    SM_MSG_BDY_E e_msg_body = SM_MSG_BDY(pMsg->ui4_msg_type);

    switch(e_msg_body)
    {
        case SM_BODY_PLAY:
            _sm_process_inform_play_msg(pMsg->ui4_sender_id, SM_MSG_ID(pMsg->ui4_msg_type));
            break;
        case SM_BODY_PAUSE:
            _sm_process_inform_pause_msg(pMsg->ui4_sender_id, SM_MSG_ID(pMsg->ui4_msg_type));
            break;
        case SM_BODY_STOP:
            _sm_process_inform_stop_msg(pMsg->ui4_sender_id, SM_MSG_ID(pMsg->ui4_msg_type), pMsg->p_usr_msg);
            break;
        default:
            break;
    }
}

static VOID _sm_process_network_msg(const VOID* pv_msg, SIZE_T z_msg_len)
{
    INT32 i4_ret = 0;
    HANDLE_T h_app = NULL_HANDLE;
    ASSISTANT_STUB_NETWORK_STATUS_CHANGE_T *network_status_change = ((APPMSG_T *)pv_msg)->p_usr_msg;

    SM_INFO("[network]quantity:%d status:%s\n", network_status_change->quantity,network_status_change->status);

    network_quality = network_status_change->quantity;
    memset(network_status,0,ASSISTANT_STUB_STATUS_MAX_LENGTH);
    strncpy(network_status,network_status_change->status,ASSISTANT_STUB_STATUS_MAX_LENGTH);

    //wifi setup is done, reset the flag for standby
    u_ui_set_wifi_setup_flag(FALSE);

    /*send  network status change to assistant_stub */
    i4_ret = u_am_get_app_handle_from_name(&h_app, ASSISTANT_STUB_THREAD_NAME);
    if(AEER_OK != i4_ret)
    {
       SM_INFO("get handle fail!,i4_ret=%ld\n",i4_ret);
    }
    i4_ret = u_app_send_appmsg(h_app,
                              E_APP_MSG_TYPE_ASSISTANT_STUB,
                              MSG_FROM_SM,
                              ASSISTANT_STUB_CMD_NETWORK_STATUS_CHANGE,
                              network_status_change,
                              sizeof(ASSISTANT_STUB_NETWORK_STATUS_CHANGE_T));
    if(0 == i4_ret)
    {
        SM_INFO("u_app_send_appmsg success !\n");
    }
    else
    {
        SM_INFO("u_app_send_appmsg fail !\n");
    }
}

static VOID _sm_process_wifi_setting_msg(const VOID* pv_msg, SIZE_T z_msg_len)
{
    u_ui_set_wifi_setup_flag(FALSE);
    SM_INFO("wifi setup finish[%d].\n", u_ui_get_wifi_setup_flag());
}

static VOID _sm_process_bluetooth_msg(const VOID* pv_msg, SIZE_T z_msg_len)
{
    APPMSG_T *pt_msg = pv_msg;

    switch(pt_msg->ui4_msg_type)
    {
        case SM_BT_FINISH_PARING:
            u_ui_set_bt_paring_flag(FALSE);
            SM_INFO("%s,bt paring finish[%d].\n",__FUNCTION__,u_ui_get_bt_paring_flag());
            break;
        default:
            break;
    }
}

static VOID _sm_process_long_volumedown_msg(VOID)
{
    SM_INFO("sm process long volumedown msg!\n");

    if(g_t_sm.b_fake_standby)
    {
        _sm_exit_fake_standby_mode();
    }
    else if(g_t_sm.b_suspend_standby)
    {
        _sm_exit_suspend_standby_mode();
    }
    else
    {
        
    }

    if (_sm_is_bt_hfp_playing())
    {
        SM_INFO("BT HFP running, long volumedown key down command is invalid!\n");
        return;
    }
    if (g_t_sm.b_speech_start)
    {
        SM_INFO("speech start, long volumedown key down command is invalid!\n");
        return;
    }

    //if source play , pause source!
    if(_sm_is_uri_playing()||
        _sm_is_tts_playing()||
        _sm_is_prompt_playing()||
        _sm_is_bt_playing()||
        _sm_is_dlna_playing())
    {
        _sm_process_pause_msg();
        return;
    }

    //if no source play, resume one source according to priority
    _sm_process_resume_msg();
}

static VOID _sm_process_key_msg(const VOID* pv_msg, SIZE_T z_msg_len)
{
    APPMSG_T *pt_msg = pv_msg;
    SM_MSG("pt_msg->ui4_msg_type = %d\n", pt_msg->ui4_msg_type);

    switch(pt_msg->ui4_msg_type)
    {
        case SM_BODY_UI_POWER:
            if (g_t_sm.b_fake_standby || g_t_sm.b_suspend_standby){
                SM_MSG("Exit standby mode, send start message to upg control!\n");
                _sm_send_msg_to_upg_control(SM_BODY_UPG_START);
            }else{
                SM_MSG("Enter fake or suspend standby, send stop message to upg control!\n");
                _sm_send_msg_to_upg_control(SM_BODY_UPG_STOP);
            }
            
            if(g_t_sm.b_fake_standby)
            {
                _sm_exit_fake_standby_mode();
            }
            else if(g_t_sm.b_suspend_standby)
            {
                _sm_exit_suspend_standby_mode();
            }
            else
            {
                if(_sm_get_ecopwer_flag())
                {
                    _sm_enter_suspend_standby_mode();
                }
                else
                {
                    _sm_enter_fake_standby_mode();
                }
            }
            break;
        case SM_BODY_UI_LONG_VOLUMEUP:
            break;
        case SM_BODY_UI_LONG_VOLUMEDOWN:
            _sm_process_long_volumedown_msg();
            break;
        default:
            break;
    }
}

static VOID _sm_process_standby_timer_msg(VOID)
{
    if (_sm_is_prompt_exist()
        || _sm_is_tts_exist()
        || _sm_is_bt_playing()
        || _sm_is_uri_playing()
        || _sm_is_dlna_playing()
        || _sm_is_bt_hfp_playing()
        || u_ui_get_wifi_setup_flag()
        || u_ui_get_bt_paring_flag()
        || g_t_sm.b_fake_standby
        || g_t_sm.b_suspend_standby)
    {
        g_t_sm.ui4_standby_count = 0;
        return;
    }

    g_t_sm.ui4_standby_count++;

    if (g_t_sm.ui4_standby_count >= COUNTS_TO_ENTER_STANDBY)
    {
        _sm_enter_fake_standby_mode();
        g_t_sm.ui4_standby_count = 0;
    }
}

static VOID _sm_process_timer_msg(const VOID* pv_msg, SIZE_T z_msg_len)
{
    SM_TIMER_MSG_E e_msg = *(SM_TIMER_MSG_E *)pv_msg;

    if (z_msg_len != sizeof(e_msg))
    {
        SM_ERR("timer's msg size[%d] is wrong, should be [%d]!\n", z_msg_len, sizeof(e_msg));
        return;
    }

    //SM_INFO("[timer] event:%d\n", e_msg);

    switch(e_msg)
    {
        case TIMER_MSG_STANDBY:
            _sm_process_standby_timer_msg();
            break;
        default:
            break;
    }
}

static VOID _sm_process_state_msg(VOID* pv_msg, SIZE_T z_msg_len)
{
    APPMSG_T *pt_msg = pv_msg;

    switch(SM_MSG_GRP(pt_msg->ui4_msg_type))
    {
        case SM_REQUEST_GRP:
            _sm_process_request_msg(pt_msg);
            break;
        case SM_INFORM_GRP:
            _sm_process_inform_msg(pt_msg);
            break;
        default:
            break;
    }
}

static VOID _sm_process_pause_msg(VOID)
{
    SM_MSG("sm process pause msg!\n");
    _sm_clear_prompt();
    _sm_clear_tts();
    _sm_send_pause_to_bt();
    _sm_send_pause_to_uri();
    _sm_send_pause_to_dlna();
}

static VOID _sm_process_resume_msg(VOID)
{
    _sm_clear_prompt();
    _sm_clear_tts();

    if (_sm_is_bt_pause())
    {
        _sm_exit_fake_standby_mode();
        _sm_send_resume_to_bt();
        return;
    }

    if (_sm_is_dlna_pause())
    {
        _sm_exit_fake_standby_mode();
        _sm_send_resume_to_dlna();
        return;
    }

    if (_sm_is_uri_pause())
    {
        _sm_exit_fake_standby_mode();
        _sm_send_resume_to_uri();
    }
}

static VOID _sm_process_stop_msg(VOID)
{
    _sm_clear_prompt();
    _sm_clear_tts();
    _sm_clear_bt();
    _sm_clear_uri();
    _sm_clear_bt_hfp();
    _sm_clear_dlna();
}

INT32 _sm_process_set_volume(ASSISTANT_STUB_SET_VOLUME_T * ui1_volume)
{
    INT32 i4_ret = 0;

    SM_ERR("cmd,%s,%d\n",ui1_volume->command,ui1_volume->volume);

    i4_ret = u_acfg_set_mute(FALSE);
    if (AEER_OK != i4_ret)
    {
        return i4_ret;
    }

    i4_ret = u_acfg_set_volume(ui1_volume->volume, TRUE);
    if (AEER_OK != i4_ret)
    {
        return i4_ret;
    }

    return 0;
}

void sm_get_playback_status(ASSISTANT_STUB_PLAYER_T * player)
{
    UINT8 b_mute;
    UINT8 volume = 0;
    ASSISTANT_STUB_PLAYER_STATUS_CHANGE_T player_status = {0};

    u_acfg_get_mute(&b_mute);
    if (b_mute)
    {
        volume = 0;
    }
    else
    {
        u_acfg_get_volume(&volume);
    }

    switch(u_sm_get_current_source())
    {
        case SOURCE_URI:
        {
            u_playback_uri_get_player_status(&player_status);
            memcpy(player,&(player_status.player),sizeof(ASSISTANT_STUB_PLAYER_T));
            break;
        }
        case SOURCE_BT:
        {
#if CONFIG_SUPPORT_BT_APP
            memcpy(&player_status,u_bluetooth_get_player_status(),sizeof(ASSISTANT_STUB_PLAYER_STATUS_CHANGE_T));
            memcpy(player,&(player_status.player),sizeof(ASSISTANT_STUB_PLAYER_T));
#endif /*CONFIG_SUPPORT_BT_APP*/
            break;
        }
        default:
        {
            player->volume = volume;
            strncpy(player->status,"stop",ASSISTANT_STUB_STATUS_MAX_LENGTH);
            strncpy(player->source,"none",ASSISTANT_STUB_SOURCE_MAX_LENGTH);
            break;
        }
    }
    SM_ERR("volume:%d,status:%s.\n",player->volume,player->status,player->source);
}

VOID _sm_clear_bt_before_enter_standby(VOID)
{
    if (_sm_is_bt_exist())
    {
        if (_sm_is_bt_wait_permit())
        {
            SM_INFO("stop bt permit!\n");
            g_t_sm.t_bt_source.e_bt_source = SOURCE_NONE;
            g_t_sm.t_bt_source.e_wait_permit = BT_NOT_PERMIT;
        }
        else
        {
            SM_INFO("stop bt before enter standby!\n");
 
            //send STOP BT message to BT, in order to allow BT to distinguish
            //this special situation, here send special message with extra information
            INT32 i4_ret = 0;
            HANDLE_T h_app = NULL_HANDLE;
            CHAR bt_fake[20] = {0};

            if(TRUE == g_t_sm.b_fake_standby)
            {
                strncpy(bt_fake, "FAKE_STANDBY", sizeof("FAKE_STANDBY"));
            }
            else if(TRUE == g_t_sm.b_suspend_standby)
            {
                strncpy(bt_fake, "SUSPEND_STANDBY", sizeof("SUSPEND_STANDBY"));
            }
            else
            {
                 //
            }
           
            i4_ret = u_am_get_app_handle_from_name(&h_app, BLUETOOTH_THREAD_NAME);
            if(AEER_OK != i4_ret)
            {
               SM_INFO("get handle fail!,i4_ret=%ld\n",i4_ret);
            }
            i4_ret = u_app_send_appmsg(h_app,
                                      E_APP_MSG_TYPE_STATE_MNGR,
                                      MSG_FROM_SM,
                                      SM_MAKE_MSG(SM_REQUEST_GRP, SM_BODY_STOP, 0),
                                      bt_fake,
                                      sizeof(bt_fake));
            if(0 == i4_ret)
            {
                SM_INFO("sm send stop to bt success !\n");
            }
            else
            {
                SM_INFO("sm send stop to bt fail !\n");
            }
        }
    }
}

VOID _sm_send_stop_before_enter_standby(VOID)
{
    _sm_clear_tts();
    _sm_clear_prompt();
    _sm_clear_bt_before_enter_standby();
    _sm_clear_uri();
    _sm_clear_bt_hfp();
    _sm_clear_dlna();
}

static VOID _sm_enter_fake_standby_mode(VOID)
{
    ASSISTANT_STUB_SYSTEM_STATUS_CHANGE_T system_status_change = {0};
    g_t_sm.b_fake_standby = TRUE;
    g_t_sm.b_suspend_standby = FALSE;

    if(_sm_is_bt_exist()
        ||_sm_is_uri_exist()
        ||_sm_is_tts_exist()
        ||_sm_is_prompt_exist()
        ||_sm_is_bt_hfp_exist()
        ||_sm_is_dlna_exist())
    {
        SM_MSG("current some source is playing, wait stop!\n");
        _sm_send_stop_before_enter_standby();
        return;
    }

    system("echo test > /sys/power/wake_lock");
    system("echo mem > /sys/power/autosleep");

    SM_MSG("enter fake standby mode!\n");

    strncpy(system_status_change.command,"/system/system_status_change",ASSISTANT_STUB_COMMAND_MAX_LENGTH);
    strncpy(system_status_change.status,"standby",ASSISTANT_STUB_STATUS_MAX_LENGTH);
    _sm_send_msg_to_assistant_stub(ASSISTANT_STUB_CMD_SYSTEM_STATUS_CHANGE, &system_status_change,sizeof(ASSISTANT_STUB_SYSTEM_STATUS_CHANGE_T));
}

VOID _sm_exit_fake_standby_mode(VOID)
{
    ASSISTANT_STUB_SYSTEM_STATUS_CHANGE_T system_status_change = {0};
    if(FALSE == g_t_sm.b_fake_standby)
    {
        //SM_MSG("current mode is not standby.\n");
        return;
    }

    system("echo off > /sys/power/autosleep");
    system("echo test > /sys/power/wake_unlock");

    SM_MSG("exit fake standby mode!\n");
    g_t_sm.b_fake_standby = FALSE;
    g_t_sm.b_suspend_standby = FALSE;

    strncpy(system_status_change.command,"/system/system_status_change",ASSISTANT_STUB_COMMAND_MAX_LENGTH);
    strncpy(system_status_change.status,"normal",ASSISTANT_STUB_STATUS_MAX_LENGTH);
    _sm_send_msg_to_assistant_stub(ASSISTANT_STUB_CMD_SYSTEM_STATUS_CHANGE, &system_status_change,sizeof(ASSISTANT_STUB_SYSTEM_STATUS_CHANGE_T));
}

VOID _sm_exec_wifi_suspend()
{
    SM_MSG("iwpriv wlan0 driver \"SETSUSPENDMODE 1\"!\n");
    system("iwpriv wlan0 driver \"SETSUSPENDMODE 1\"");

    SM_MSG("Sleep 300 milliseconds!\n");
    usleep(300000);
}
VOID _sm_exec_wifi_resume()
{
    SM_MSG("iwpriv wlan0 driver \"SETSUSPENDMODE 0\"!\n");
    system("iwpriv wlan0 driver \"SETSUSPENDMODE 0\"");
}

static VOID _sm_process_suspend_standby(VOID)
{
    SM_MSG("enter suspend standby mode!\n");
    g_t_sm.ui4_standby_count = 0;
#if CONFIG_AUTO_SLEEP_FOR_SUSPEND
    _sm_exec_wifi_suspend();
    system("echo mem > /sys/power/autosleep");
#else
    _sm_exec_wifi_suspend();
    //after execute the following cmd, sytem will hung
    system("echo mem > /sys/power/state");

    //if power on, system will restart from here
    system("echo on > /sys/power/state");
    SM_MSG("exit suspend standby mode!\n");
    _sm_exec_wifi_resume();
    g_t_sm.b_fake_standby = FALSE;
    g_t_sm.b_suspend_standby = FALSE;
#endif
}

static VOID _sm_enter_suspend_standby_mode()
{
    ASSISTANT_STUB_HFP_STATUS_CHANGE_T hfp_status_change = {0};

    g_t_sm.b_fake_standby = FALSE;
    g_t_sm.b_suspend_standby = TRUE;

    if(_sm_is_bt_exist()
        ||_sm_is_uri_exist()
        ||_sm_is_tts_exist()
        ||_sm_is_prompt_exist()
        ||_sm_is_bt_hfp_exist()
        ||_sm_is_dlna_exist())
    {
        SM_MSG("current some source is playing, wait stop!\n");
        _sm_send_stop_before_enter_standby();
        return;
    }
    
    if(_sm_is_process_exist("Aispeech"))
    {
        SM_MSG("<%s>wait Assistantcenter free MIC!\n",__FUNCTION__);
        strncpy(hfp_status_change.command,"/system/hfp_status_change",ASSISTANT_STUB_COMMAND_MAX_LENGTH);
        strncpy(hfp_status_change.status,"hfp_request",ASSISTANT_STUB_STATUS_MAX_LENGTH);
        _sm_send_msg_to_assistant_stub(ASSISTANT_STUB_CMD_HFP_STATUS_CHANGE, &hfp_status_change,sizeof(ASSISTANT_STUB_HFP_STATUS_CHANGE_T));
    }  
    else
    {
        _sm_process_suspend_standby();
    }
}

static VOID _sm_exit_suspend_standby_mode()
{
    ASSISTANT_STUB_HFP_STATUS_CHANGE_T hfp_status_change = {0};

    if(FALSE == g_t_sm.b_suspend_standby)
    {
        SM_MSG("current mode is not suspend standby.\n");
        return;
    }

    #if CONFIG_AUTO_SLEEP_FOR_SUSPEND
    system("echo off > /sys/power/autosleep");
    SM_MSG("exit suspend standby mode!\n");
    _sm_exec_wifi_resume();
    g_t_sm.b_fake_standby = FALSE;
    g_t_sm.b_suspend_standby = FALSE;
    #else
    //do nothing
    #endif
    strncpy(hfp_status_change.command,"/system/hfp_status_change",ASSISTANT_STUB_COMMAND_MAX_LENGTH);
    strncpy(hfp_status_change.status,"hfp_hangup",ASSISTANT_STUB_STATUS_MAX_LENGTH);
    _sm_send_msg_to_assistant_stub(ASSISTANT_STUB_CMD_HFP_STATUS_CHANGE, &hfp_status_change,sizeof(ASSISTANT_STUB_HFP_STATUS_CHANGE_T));
}

INT32 _sm_process_set_system_status(ASSISTANT_STUB_SET_SYSTEM_STATUS_T * t_status)
{
    SM_ERR("cmd,%s,%s\n",t_status->command,t_status->status);

    if(!strcmp(t_status->status,"standby"))
    {
        
        SM_MSG("Enter fake standby, send stop message to upg control!\n");
        _sm_send_msg_to_upg_control(SM_BODY_UPG_STOP);
        
        _sm_set_system_status(TRUE);
        _sm_enter_fake_standby_mode();
    }
    else
    {
        _sm_exit_fake_standby_mode();
    }
    return 0;
}

INT32 _sm_process_get_speaker_status(ASSISTANT_STUB_GET_SPEAKER_STATUS_T * t_status)
{
    ASSISTANT_STUB_GET_SPEAKER_STATUS_RESPONSE_T respose_speaker_status = {0};
#if CONFIG_SUPPORT_BT_APP
    ASSISTANT_STUB_BLUETOOTH_STATUS_CHANGE_T * bt_status = NULL;
#endif /*CONFIG_SUPPORT_BT_APP*/
    UINT8 mac_def[6] = {0};
    char mac[20] = {0};
    char bt_mac[20] = {0};
    char sn[64] = {0};
    struct BT_CFG_STRUCT bt_mac_def = {0};

    SM_ERR("cmd,%s,%d\n",t_status->command,t_status->id);

    strncpy(respose_speaker_status.system.status,u_sm_is_enter_fake_standby()?"standby":"normal",ASSISTANT_STUB_STATUS_MAX_LENGTH);

    a_cfg_get_wlan_mac(mac);
    SM_INFO("mac:%s\n",mac);
    if(strlen(mac) == 0)
    {
        c_net_wlan_get_mac_addr(mac_def);
        snprintf(mac,19,"%02x:%0x:%02x:%02x:%02x:%02x",mac_def[0],mac_def[1],mac_def[2],mac_def[3],mac_def[4],mac_def[5]);
        SM_INFO("mac:%s,%d\n",mac,__LINE__);
        a_cfg_set_wlan_mac(mac);
    }
    strncpy(respose_speaker_status.system.wifi_mac,mac,ASSISTANT_STUB_WIFI_MAC_MAX_LENGTH);

    r_BT_CFG(&bt_mac_def);
    snprintf(bt_mac,19,"%02x:%0x:%02x:%02x:%02x:%02x",bt_mac_def.mac[0],bt_mac_def.mac[1],bt_mac_def.mac[2],bt_mac_def.mac[3],bt_mac_def.mac[4],bt_mac_def.mac[5]);
    SM_INFO("bt_mac:%s\n",bt_mac);
    strncpy(respose_speaker_status.system.bt_mac,bt_mac,ASSISTANT_STUB_BT_MAC_MAX_LENGTH);
    r_SN(&sn);
    strncpy(respose_speaker_status.system.device_sn,sn,ASSISTANT_STUB_DEVICE_SN_MAX_LENGTH);
    SM_INFO("sn:%s\n",sn);

    sm_get_playback_status(&respose_speaker_status.player);

    respose_speaker_status.power.quantity = 0;
    strncpy(respose_speaker_status.power.status,"electricity",ASSISTANT_STUB_STATUS_MAX_LENGTH);

#if CONFIG_SUPPORT_BT_APP
    bt_status = u_bluetooth_get_bt_status_assistant();
    //memcpy(&bt_status,,sizeof(ALI_STUB_BLUETOOTH_STATUS_CHANGE_T));
    if(NULL == bt_status)
    {
        SM_ERR("get bt status fail.\n");
    }
    else
    {
        SM_ERR("get bt status,status=%s,name =%s,paired_name=%s.\n",bt_status->status,bt_status->name,bt_status->bt_paired_name);
        strncpy(respose_speaker_status.bluetooth.status,bt_status->status,ASSISTANT_STUB_STATUS_MAX_LENGTH);
        strncpy(respose_speaker_status.bluetooth.name,bt_status->name,ASSISTANT_STUB_BT_NAME_MAX_LENGTH);
        strncpy(respose_speaker_status.bluetooth.bt_paired_name,bt_status->bt_paired_name,ASSISTANT_STUB_BT_PAIRED_NAME_MAX_LENGTH);
    }
#endif /*CONFIG_SUPPORT_BT_APP*/

    respose_speaker_status.network.quantity = network_quality;
    strncpy(respose_speaker_status.network.status,network_status,ASSISTANT_STUB_STATUS_MAX_LENGTH);

    u_wifi_get_current_info(respose_speaker_status.network.ssid, respose_speaker_status.network.bssid, NULL);

    respose_speaker_status.id = t_status->id;

    _sm_send_msg_to_assistant_stub(ASSISTANT_STUB_CMD_GET_SPEAKER_STATUS, &respose_speaker_status,sizeof(ASSISTANT_STUB_GET_SPEAKER_STATUS_RESPONSE_T));

    return 0;
}

INT32 _sm_process_factory_reset(VOID)
{
    //SM_MSG("cmd,%s,%d\n",t_status->command,t_status->result);

    //_sm_set_factory_reset(TRUE);

    INT8 i4_ret = 0;
    i4_ret = u_acfg_factory_reset();
    if(0 != i4_ret)
    {
        SM_ERR("u_acfg_factory_reset failed, i4_ret[%d]\n", i4_ret);
    }
    else
    {
        SM_INFO("u_acfg_factory_reset success!\n");
    }

    return 0;
}

INT32 _sm_process_hfp_free_mic(ASSISTANT_STUB_HFP_FREE_MIC_RESULT_T * t_status)
{
    SM_MSG("cmd,%s,%d\n",t_status->command,t_status->result);

    if(u_sm_is_enter_suspend_standby())
    {
        _sm_process_suspend_standby();
        return 0;
    }

    _sm_process_bt_hfp_request_play();

    return 0;
}

static VOID _sm_process_speech_start(VOID)
{
    g_t_sm.b_speech_start = TRUE;

    _sm_exit_fake_standby_mode();

    _sm_clear_tts();
    _sm_clear_prompt();
    _sm_send_suspend_to_bt();
    _sm_send_suspend_to_dlna();
    _sm_send_suspend_to_uri(URI_SUSPEND_BY_SWITCH);
}

static VOID _sm_process_speech_finish(VOID)
{
    g_t_sm.b_speech_start = FALSE;

    _sm_process_priority_play(FALSE);
}

static VOID _sm_process_uri_play(INT8 *p_msg)
{
    g_t_sm.t_uri_source.e_uri_source = SOURCE_URI;
    g_t_sm.t_uri_source.b_wait_permit = TRUE;
    g_t_sm.t_uri_source.ui2_uri_id++;
    memcpy(&g_t_sm.t_uri_source.t_uri_play, p_msg, sizeof(g_t_sm.t_uri_source.t_uri_play));

    _sm_exit_fake_standby_mode();

    if (_sm_is_bt_exist())
    {
        if (_sm_is_bt_wait_permit())
        {
            _sm_clear_bt();
        }
        else
        {
            _sm_clear_bt();
            return;
        }
    }

    if (_sm_is_dlna_exist())
    {
        if (_sm_is_dlna_wait_permit())
        {
            _sm_clear_dlna();
        }
        else
        {
            _sm_clear_dlna();
            return;
        }
    }

    if(u_ui_get_factory_reset_flag())
    {
        SM_INFO("factory reset going, return!\n");
        return;
    }

    if (g_t_sm.b_speech_start)
    {
        SM_INFO("speech starting, wait end!\n");
        return;
    }

    //prompt exist || no need mix tts exist
    if (_sm_is_prompt_exist() || (_sm_is_tts_exist() && !_sm_is_tts_need_mix()))
    {
        SM_INFO("prompt or tts exist, wait end!\n");
        return;
    }

    _sm_send_permit_to_uri();
}

static VOID _sm_process_normal_prompt(INT8 *p_msg)
{
    g_t_sm.t_prompt_source.e_prompt_source = SOURCE_PROMPT;
    g_t_sm.t_prompt_source.b_wait_permit = TRUE;
    g_t_sm.t_prompt_source.ui2_id++;
    memcpy(&g_t_sm.t_prompt_source.t_prompt_play, p_msg, sizeof(g_t_sm.t_prompt_source.t_prompt_play));

    if (g_t_sm.b_speech_start)
    {
        _sm_send_permit_to_prompt();
        return;
    }

    if (_sm_is_tts_exist())
    {
        SM_INFO("tts exist, wait end!\n");
        return;
    }

    if (_sm_is_bt_playing())
    {
        _sm_send_suspend_to_bt();
        return;
    }

    if (_sm_is_dlna_playing())
    {
        _sm_send_suspend_to_dlna();
        return;
    }

    if (_sm_is_uri_playing())
    {
        _sm_send_suspend_to_uri(URI_SUSPEND_BY_SWITCH);
        return;
    }

    _sm_send_permit_to_prompt();
}

static INT32 _sm_is_current_any_playing(VOID)
{
    if (_sm_is_prompt_playing()
        || _sm_is_tts_playing()
        || _sm_is_bt_playing()
        || _sm_is_dlna_playing()
        || _sm_is_uri_playing())
    {
        return TRUE;
    }
    return FALSE;
}

static VOID _sm_process_alarm_prompt(INT8 *p_msg)
{
    BOOL b_any_playing = _sm_is_current_any_playing();

    _sm_clear_prompt();
    _sm_clear_tts();
    _sm_send_suspend_to_bt();
    _sm_send_suspend_to_dlna();
    _sm_send_suspend_to_uri(URI_SUSPEND_BY_SWITCH);

    g_t_sm.t_prompt_source.e_prompt_source = SOURCE_PROMPT;
    g_t_sm.t_prompt_source.b_wait_permit = TRUE;
    g_t_sm.t_prompt_source.ui2_id++;
    memcpy(&g_t_sm.t_prompt_source.t_prompt_play, p_msg, sizeof(g_t_sm.t_prompt_source.t_prompt_play));

    if (FALSE == b_any_playing)
    {
        _sm_send_permit_to_prompt();
    }
}

static VOID _sm_process_reset_prompt(INT8 *p_msg)
{
    g_t_sm.t_prompt_source.e_prompt_source = SOURCE_PROMPT;
    g_t_sm.t_prompt_source.b_wait_permit = TRUE;
    g_t_sm.t_prompt_source.ui2_id++;
    memcpy(&g_t_sm.t_prompt_source.t_prompt_play, p_msg, sizeof(g_t_sm.t_prompt_source.t_prompt_play));

    if(u_ui_get_factory_reset_flag())
    {
        _sm_send_permit_to_prompt();
    }
}

static VOID _sm_process_voice_prompt(INT8 *p_msg)
{
    ASSISTANT_STUB_PLAY_VOICE_PROMPT_T * pr_msg = p_msg;

    if(!_sm_is_set_system_status())
    {
        _sm_exit_fake_standby_mode();
    }
    _sm_set_system_status(FALSE);

    if (strncmp(pr_msg->type, "normal", ASSISTANT_STUB_TYPE_LENGTH) == 0)
    {
        _sm_process_normal_prompt(p_msg);
    }
    else if (strncmp(pr_msg->type, "alarm", ASSISTANT_STUB_TYPE_LENGTH) == 0)
    {
        _sm_process_alarm_prompt(p_msg);
    }
    else if (strncmp(pr_msg->type, "reset", ASSISTANT_STUB_TYPE_LENGTH) == 0)
    {
        _sm_process_reset_prompt(p_msg);
    }
}

static VOID _sm_process_tts_play(INT8 *p_msg)
{
    if(u_ui_get_factory_reset_flag())
    {
        SM_INFO("factory reset going, return!\n");
        return;
    }

    if(!_sm_is_set_system_status())
    {
        _sm_exit_fake_standby_mode();
    }
    _sm_set_system_status(FALSE);

    g_t_sm.t_tts_source.e_tts_source = SOURCE_TTS;
    g_t_sm.t_tts_source.b_wait_permit = TRUE;
    g_t_sm.t_tts_source.ui2_id++;
    memcpy(&g_t_sm.t_tts_source.t_tts_play, p_msg, sizeof(g_t_sm.t_tts_source.t_tts_play));

    if (g_t_sm.b_speech_start)
    {
        SM_INFO("speech start, wait end!\n");
        return;
    }

    if (_sm_is_prompt_exist())
    {
        SM_INFO("prompt exist, wait end!\n");
        return;
    }

    if (g_t_sm.t_tts_source.t_tts_play.need_mix)
    {
        _sm_send_permit_to_tts();
        return;
    }

    if (_sm_is_bt_playing())
    {
        _sm_send_suspend_to_bt();
        return;
    }

    if (_sm_is_dlna_playing())
    {
        _sm_send_suspend_to_dlna();
        return;
    }
    
    if (_sm_is_uri_playing())
    {
        _sm_send_suspend_to_uri(URI_SUSPEND_BY_SWITCH);
        return;
    }

    _sm_send_permit_to_tts();
}

VOID _sm_process_prev_bt_audio(VOID)
{
    SM_MSG("bt prev audio request!\n");

    if(u_ui_get_factory_reset_flag())
    {
        SM_INFO("factory reset going, return!\n");
        return;
    }

    g_t_sm.t_bt_source.e_bt_source = SOURCE_BT;
    g_t_sm.t_bt_source.ui2_bt_id++;
    g_t_sm.t_bt_source.e_wait_permit = BT_WAIT_PREV;

    _sm_clear_uri();
    _sm_clear_dlna();

    if (g_t_sm.b_speech_start)
    {
        SM_INFO("speech starting, bt request enqueue!\n");
        return;
    }

    //prompt exist || no need mix tts exist
    if (_sm_is_prompt_exist() || (_sm_is_tts_exist() && !_sm_is_tts_need_mix()))
    {
        SM_INFO("prompt or tts exist, wait end!\n");
        return;
    }

    _sm_send_play_to_bt();

    return;
}

VOID _sm_process_next_bt_audio(VOID)
{
    SM_MSG("bt next audio request!\n");

    if(u_ui_get_factory_reset_flag())
    {
        SM_INFO("factory reset going, return!\n");
        return;
    }

    g_t_sm.t_bt_source.e_bt_source = SOURCE_BT;
    g_t_sm.t_bt_source.ui2_bt_id++;
    g_t_sm.t_bt_source.e_wait_permit = BT_WAIT_NEXT;

    _sm_clear_uri();
    _sm_clear_dlna();

    if (g_t_sm.b_speech_start)
    {
        SM_INFO("speech starting, bt request enqueue!\n");
        return;
    }

    //prompt exist || no need mix tts exist
    if (_sm_is_prompt_exist() || (_sm_is_tts_exist() && !_sm_is_tts_need_mix()))
    {
        SM_INFO("prompt or tts exist, wait end!\n");
        return;
    }

    _sm_send_play_to_bt();
}

VOID _sm_process_play_bt_music(VOID)
{
    SM_MSG("speech play bt music!\n");

    if(u_ui_get_factory_reset_flag())
    {
        SM_INFO("factory reset going, return!\n");
        return;
    }

    g_t_sm.t_bt_source.e_bt_source = SOURCE_BT;
    g_t_sm.t_bt_source.ui2_bt_id++;
    g_t_sm.t_bt_source.e_wait_permit = BT_WAIT_PERMIT;

    _sm_clear_uri();
    _sm_clear_dlna();

    if (g_t_sm.b_speech_start)
    {
        SM_INFO("speech starting, bt request enqueue!\n");
        return;
    }

    //prompt exist || no need mix tts exist
    if (_sm_is_prompt_exist() || (_sm_is_tts_exist() && !_sm_is_tts_need_mix()))
    {
        SM_INFO("prompt or tts exist, wait end!\n");
        return;
    }

    _sm_send_play_to_bt();
}

static VOID _sm_process_assistant_msg(APPMSG_T* p_msg)
{
    SM_INFO("assistant msg type:[%d]\n", p_msg->ui4_msg_type);
    switch (p_msg->ui4_msg_type)
    {
        case ASSISTANT_STUB_CMD_SPEECH_START:
            _sm_process_speech_start();
            break;
        case ASSISTANT_STUB_CMD_SPEECH_FINISH:
            _sm_process_speech_finish();
            break;
        case ASSISTANT_STUB_CMD_PLAY:
            _sm_process_uri_play(p_msg->p_usr_msg);
            break;
        case ASSISTANT_STUB_CMD_PLAY_VOICE_PROMPT:
            _sm_process_voice_prompt(p_msg->p_usr_msg);
            break;
        case ASSISTANT_STUB_CMD_PLAY_TTS:
            _sm_process_tts_play(p_msg->p_usr_msg);
            break;
        case ASSISTANT_STUB_CMD_PLAY_PREV_AUDIO:
            _sm_process_prev_bt_audio();
            break;
        case ASSISTANT_STUB_CMD_PLAY_NEXT_AUDIO:
            _sm_process_next_bt_audio();
            break;
        case ASSISTANT_STUB_CMD_PLAY_BT_MUSIC:
            _sm_process_play_bt_music();
            break;
        case ASSISTANT_STUB_CMD_PAUSE:
            _sm_process_pause_msg();
            break;
        case ASSISTANT_STUB_CMD_RESUME:
            _sm_process_resume_msg();
            break;
        case ASSISTANT_STUB_CMD_STOP:
            _sm_process_stop_msg();
            break;
        case ASSISTANT_STUB_CMD_SET_VOLUME:
            _sm_process_set_volume((ASSISTANT_STUB_SET_VOLUME_T *)(p_msg->p_usr_msg));
            break;
        case ASSISTANT_STUB_CMD_SET_SYSTEM_STATUS:
            _sm_process_set_system_status((ASSISTANT_STUB_SET_SYSTEM_STATUS_T *)(p_msg->p_usr_msg));
            break;
        case ASSISTANT_STUB_CMD_GET_SPEAKER_STATUS:
            _sm_process_get_speaker_status((ASSISTANT_STUB_GET_SPEAKER_STATUS_T *)(p_msg->p_usr_msg));
            break;
        case ASSISTANT_STUB_CMD_FACTORY_RESET_RESULT:
            _sm_process_factory_reset();
            break;
        case ASSISTANT_STUB_CMD_HFP_FREE_MIC_RESULT:
            _sm_process_hfp_free_mic((ASSISTANT_STUB_HFP_FREE_MIC_RESULT_T *)(p_msg->p_usr_msg));
            break;
        default:
            break;
    }
}

static INT32 _sm_process_msg (
                    HANDLE_T                    h_app,
                    UINT32                      ui4_type,
                    const VOID*                 pv_msg,
                    SIZE_T                      z_msg_len,
                    BOOL                        b_paused
                    )
{
    UCHAR* pc_name;
    UINT32             i4_ret;
    const CHAR *pc_keysta, *pc_keyval;
    APPMSG_T *pMsg = pv_msg;

    if (g_t_sm.b_app_init == FALSE) {
        return AEER_FAIL;
    }
    if (ui4_type < AMB_BROADCAST_OFFSET)
    {
        switch(ui4_type)
        {
            case E_APP_MSG_TYPE_ASSISTANT_STUB:
                _sm_process_assistant_msg(pMsg);
                break;
            case E_APP_MSG_TYPE_STATE_MNGR:
                _sm_process_state_msg(pv_msg, z_msg_len);
                break;
            case E_APP_MSG_TYPE_MISC:
                _sm_process_network_msg(pv_msg, z_msg_len);
                break;
            case E_APP_MSG_TYPE_TIMER:
                _sm_process_timer_msg(pv_msg, z_msg_len);
                break;
            case E_APP_MSG_TYPE_WIFI_SETTING:
                _sm_process_wifi_setting_msg(pv_msg, z_msg_len);
                break;
            case E_APP_MSG_TYPE_BLUETOOTH:
                _sm_process_bluetooth_msg(pv_msg, z_msg_len);
                break;
            case E_APP_MSG_TYPE_USER_CMD:
                _sm_process_key_msg(pv_msg, z_msg_len);
                break;
            default:
                break;
        }
    }
    return AEER_OK;
}

BOOL _sm_is_factory_reset_down(VOID)
{
    return f_is_custom_clear_down;
}

VOID _sm_set_factory_reset(BOOL flag)
{
    f_is_custom_clear_down = flag;
}

BOOL _sm_is_set_system_status(VOID)
{
    return f_is_set_system_status;
}

VOID _sm_set_system_status(BOOL flag)
{
    f_is_set_system_status = flag;
}

BOOL _sm_get_ecopwer_flag(VOID)
{
    return f_ecopwer;
}

VOID _sm_set_ecopwer_flag(BOOL flag)
{
    f_ecopwer = flag;
}

INT32 u_sm_get_current_playback_status(VOID)
{
    SM_PLAYBACK_SOURCE_E e_source = u_sm_get_current_source();

    switch (e_source)
    {
        case SOURCE_BT:
            return g_t_sm.t_bt_source.ui4_status;
        case SOURCE_URI:
            return g_t_sm.t_uri_source.ui4_status;
        default:
            break;
    }

    return SM_BODY_STOP;
}

SM_PLAYBACK_SOURCE_E u_sm_get_current_source(VOID)
{
    if (_sm_is_bt_exist())
    {
        return SOURCE_BT;
    }
    else if (_sm_is_uri_exist())
    {
        return SOURCE_URI;
    }

    return SOURCE_NONE;
}

BOOL u_sm_is_network_connect(VOID)
{
    return g_t_sm.b_network_connect;
}

BOOL u_sm_is_speech_start(VOID)
{
    return g_t_sm.b_speech_start;
}

BOOL u_sm_is_enter_fake_standby(VOID)
{
    return g_t_sm.b_fake_standby;
}

VOID u_sm_set_fake_standby(BOOL b_enter_fake_standby)
{
    g_t_sm.b_fake_standby = b_enter_fake_standby;
}

BOOL u_sm_is_enter_suspend_standby(VOID)
{
    return g_t_sm.b_suspend_standby;
}

VOID u_sm_send_stop_to_all_source(VOID)
{
    _sm_clear_tts();
    _sm_clear_prompt();
    _sm_clear_bt();
    _sm_clear_uri();
    _sm_clear_bt_hfp();
    _sm_clear_dlna();
}

VOID u_sm_set_suspend_standby(BOOL b_enter_normal_standby)
{
    g_t_sm.b_suspend_standby = b_enter_normal_standby;
}

BOOL _sm_is_process_exist(const char * process_name)
{
    char command[150];
    FILE *fp;
    int count = 0;
    char buf[3];
    
    sprintf(command,"ps | grep %s | wc -l",process_name);

    if((fp = popen(command,"r")) != NULL)
    {
        if((fgets(buf,3,fp)) != NULL)
        {
            printf("%s,buf=%s\n",__FUNCTION__,buf);
            count = atoi(buf);
            if(count > 2)
                return TRUE;
        }
    }
    return FALSE;
}
