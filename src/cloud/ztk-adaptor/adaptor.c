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
 * MediaTek Inc. (C) 2016-2017. All rights reserved.
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
#include <unistd.h>
#include <malloc.h>
#include <fcntl.h>
#include <sys/prctl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "u_common.h"
#include "u_amb.h"
#include "u_am.h"
#include "u_assert.h"
#include "u_cli.h"
#include "u_dbg.h"
#include "u_app_thread.h"
#include "u_c4a_stub.h"
#include "u_assistant_stub.h"
#include "u_app_def.h"
#include "u_acfg.h"
#include "mas_lib.h"
#include "led.h"
#include "fl3236_led.h"

#include "types.h"
#include "adaptor_log.h"
#include "bt_app.h"
#include "led_app.h"
#include "adaptor.h"
#include "mas_lib.h"
/*-----------------------------------------------------------------------------
 * macro definitions
 *---------------------------------------------------------------------------*/

UINT32 ui4_enable_all_log = 1;
void* g_mas_handle = NULL;

/*-----------------------------------------------------------------------------
 * structure definitions
 *---------------------------------------------------------------------------*/

typedef struct _APP_MNGR_INIT_SEQ_T {
    HANDLE_T  h_sema;  /* Semaphore handle for the application manager sequencing. */
    BOOL  b_ok;        /* Boolean state indicating application manager init successfull. */
}APP_MNGR_INIT_SEQ_T;


typedef VOID (*app_register_fct)(AMB_REGISTER_INFO_T * );


typedef struct{
	app_register_fct _regfct;
	AMB_REGISTER_INFO_T _reginfo;
	char * app_name;
}APP_REG;

/*-----------------------------------------------------------------------------
 * extern function declarations
 *---------------------------------------------------------------------------*/
#if CONFIG_SUPPORT_DM_APP
extern VOID a_dm_register(AMB_REGISTER_INFO_T* pt_reg);
#endif
extern VOID a_button_sound_register(AMB_REGISTER_INFO_T* pt_reg);
extern VOID a_timerd_register(AMB_REGISTER_INFO_T* pt_reg);
extern VOID a_user_interface_register(AMB_REGISTER_INFO_T*  pt_reg);
extern VOID a_wifi_setting_register(AMB_REGISTER_INFO_T* pt_reg);
extern VOID a_misc_register(AMB_REGISTER_INFO_T* pt_reg);
extern VOID a_upg_register(AMB_REGISTER_INFO_T* pt_reg);
extern VOID a_upg_control_register(AMB_REGISTER_INFO_T* pt_reg);
extern VOID a_app_ctrl_setting_register(AMB_REGISTER_INFO_T* pt_reg);
extern INT32 app_ctrl_led_init();

/*-----------------------------------------------------------------------------
 * private function declarations
 *---------------------------------------------------------------------------*/
VOID adaptor_appl_init (VOID);


/*-----------------------------------------------------------------------------
 * variable declarations
 *---------------------------------------------------------------------------*/

static APP_REG app_to_reg[] = {
        {a_timerd_register,{0},TIMERD_THREAD_NAME},
        {a_wifi_setting_register,{0},WIFI_SETTING_THREAD_NAME},
        {a_misc_register,{0},MISC_THREAD_NAME},
        {a_upg_register,{0},UPG_THREAD_NAME},
#if CONFIG_SUPPORT_DM_APP
        {a_dm_register,{0},DM_THREAD_NAME},
#endif
        {a_user_interface_register,{0},USER_INTERFACE_THREAD_NAME},
        {a_upg_control_register, {0}, UPG_CONTROL_THREAD_NAME},
        {a_app_ctrl_setting_register, {0}, APP_CTRL_THREAD_NAME},
        {NULL,{0},NULL}
};

BspReportEventCbk gAdaptorReportEvent;

static VOID adaptor_callback_register(ADAPTOR_CB_FUNC_T *func) {

    if (func == NULL) {
        APDATOR_LOG(ERROR,"func is NULL!");
        return;
    }

    APDATOR_LOG(INFO,"START REGISTER");

    if (func->adaptor_report_event_call_back) {
        gAdaptorReportEvent = func->adaptor_report_event_call_back;
        APDATOR_LOG(INFO,"gCloudAppBleRecvPacket register!");
    }

    return;
}

/*---------------------------------------------------------------------------
 * Name
 *      app_mngr_nfy_fct
 * Description      -
 * Input arguments  -
 * Output arguments -
 * Returns          -
 *---------------------------------------------------------------------------*/
static BOOL adaptor_app_mngr_nfy_fct (HANDLE_T          h_app_mngr,
                              VOID*             pv_tag,
                              APP_NFY_REASON_T  e_nfy_reason) {
    if ((pv_tag != NULL)                           &&
        ((e_nfy_reason == APP_NFY_INIT_OK)       ||
         (e_nfy_reason == APP_NFY_INIT_FAILED)))
    {
        if (e_nfy_reason == APP_NFY_INIT_OK)
        {
            ((APP_MNGR_INIT_SEQ_T*) pv_tag)->b_ok = TRUE;
        }

        ASSERT(u_sema_unlock (((APP_MNGR_INIT_SEQ_T*) pv_tag)->h_sema) == OSR_OK);
    }

    return (FALSE);
}

/*---------------------------------------------------------------------------
 * Name
 *      x_appl_init
 * Description      -
 * Input arguments  -
 * Output arguments -
 * Returns          -
 *---------------------------------------------------------------------------*/
VOID adaptor_appl_init (VOID) {
    HANDLE_T              h_app_mngr;
    APP_MNGR_INIT_SEQ_T   t_app_mngr_init_seq;
    AMB_REGISTER_INFO_T   t_amb_reg_info;

    /* Create the application manager synchronization semaphore. */
    ASSERT(u_sema_create (&t_app_mngr_init_seq.h_sema, X_SEMA_TYPE_BINARY, X_SEMA_STATE_LOCK) == OSR_OK);


    /* Initialize application manager */
    u_am_init (& t_amb_reg_info);


    t_app_mngr_init_seq.b_ok = FALSE;
    /* Start application manager */
    ASSERT (u_app_start (&(t_amb_reg_info.t_fct_tbl),
                     &(t_amb_reg_info.t_desc),
                     t_amb_reg_info.s_name,
                     adaptor_app_mngr_nfy_fct,
                     ((VOID*) &t_app_mngr_init_seq),
                     &h_app_mngr) == AEER_OK);

    /* And now wait until the application manager has signaled that it */
    /* has successfully started.                                       */
    ASSERT(u_sema_lock (t_app_mngr_init_seq.h_sema, X_SEMA_OPTION_WAIT) == OSR_OK);
    /* If the application manager could not start successfully, abort.*/
    ASSERT ((t_app_mngr_init_seq.b_ok));
    /* Free the application manager sequencing semaphore. */
    ASSERT (u_sema_delete (t_app_mngr_init_seq.h_sema) == OSR_OK);


    return;
}

EXPORT_SYMBOL int adaptor_init(ADAPTOR_CB_FUNC_T *func) {
    int ret = AMBR_OK;
    int status;
    pid_t pid;
    int file;

    /*regitster callback*/
    adaptor_callback_register(func);

    /*coredump function*/

    if(0 == access("/data/enable_coredump", 0)) {
        system("ulimit -c 200000");
        system("mkdir /data/coredump");
        system("echo /data/coredump/core.%e.%p.%t > /proc/sys/kernel/core_pattern");
    } else {
        system("ulimit -c 0");
        system("echo /data/coredump/core.%e.%p.%t > /proc/sys/kernel/core_pattern");
    }

#if CONFIG_SUPPORT_APPMAIN_RESTART
    restart:

    pid = fork();

    if(0 > pid) {
        APDATOR_LOG(INFO,"fork child process error at line");
    } else if(0 < pid) {
        /*parent process*/
        APDATOR_LOG(INFO,"parent process pid is %d, child process pid is %d",getpid(),pid);
        int quit_pid = waitpid(pid,&status,0);
        APDATOR_LOG(INFO,"quit pid is %d, exit status is %d",quit_pid,WEXITSTATUS(status));

        APDATOR_LOG(INFO,"kill btservice");
        /*kill main btservice process*/
        system("killall -9 btservice");

        /*need to remove the alsa device handle used flag*/
        system("rm -f /tmp/mtkac0_opened");
        system("rm -f /tmp/mtkac1_opened");
        system("rm -f /tmp/mtkac2_opened");

        /*restart bt service*/
        system("/usr/bin/btservice&");
        goto restart;
    } else
#endif
    {
        /*init output log type*/
        if(0 == access("/data/log_all",0)){
            ui4_enable_all_log = 0;
            APDATOR_LOG(INFO,"disable all ouput in appmainprog!!");
        }
        APDATOR_LOG(INFO,"**********************************************");
        APDATOR_LOG(INFO,"child process id is %d",getpid());
        APDATOR_LOG(INFO,"Appcliation Init Begin");

        APDATOR_LOG(INFO,"OS thread Init");
        CHECK_ASSERT(os_thread_init());

#if CLI_SUPPORT
        APDATOR_LOG(INFO,"Cli Init");
        CHECK_ASSERT(u_cli_init());
#endif

        APDATOR_LOG(INFO,"Dbg Backtrace Init");
        CHECK_ASSERT(dbg_init());

        APDATOR_LOG(INFO,"AEE Init");
        CHECK_ASSERT(aee_init());

        APDATOR_LOG(INFO,"Handle Usr Init");
        CHECK_ASSERT(u_handle_usr_init(256));

        APDATOR_LOG(INFO,"Am Init");
        adaptor_appl_init();

        int i = 0;
        APP_REG * app = app_to_reg;
        while(app->_regfct != NULL) {
            APDATOR_LOG(INFO,"%d.%s Init Begin",i+1,app->app_name);
            app->_regfct(&app->_reginfo);
            if(AMBR_OK == (ret = u_amb_register_app(&app->_reginfo))) {
                if(AMBR_OK == (ret = u_amb_sync_start_app(app->app_name))) {
                    APDATOR_LOG(INFO,"%d.%s Init Successful",i+1,app->app_name);
                } else {
                    APDATOR_LOG(INFO,"%d.%s sync start fail ret %d",i+1,app->app_name,ret);
                }
            } else {
                APDATOR_LOG(INFO,"%d.%s register fail ret %d",i+1,app->app_name,ret);
            }
            app = &app_to_reg[++i];
        }
        APDATOR_LOG(INFO,"Appcliation Init Finish\n");
        APDATOR_LOG(INFO,"**********************************************\n");
    }

    led_system_power_on(LED_ON);

    CHECK_ASSERT(mas_create(&g_mas_handle));

#if CONFIG_SUPPORT_BT_HFP_APP
    //if custom's SPK only support 48k, please set fix sample rate to MAS
    CHECK_ASSERT(mas_set_out_samplerate_rule(g_mas_handle,MAS_OUT_SR_FIX,48000));
#endif
    CHECK_ASSERT(mas_start(g_mas_handle));

    return ADAPTOR_OK;
}

