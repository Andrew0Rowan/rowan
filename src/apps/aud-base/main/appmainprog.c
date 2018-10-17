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

/*-----------------------------------------------------------------------------
 * macro definitions
 *---------------------------------------------------------------------------*/
#define APPMAIN_TAG "<appmain>"
#define APPMAIN_DEBUG(x) printf x
#define APPMAIN_OK 0
#define APPMAIN_FAIL -1

UINT32 ui4_enable_all_log = 1;

/*-----------------------------------------------------------------------------
 * structure definitions
 *---------------------------------------------------------------------------*/

typedef struct _APP_MNGR_INIT_SEQ_T
{
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
extern VOID a_hellotest_register(AMB_REGISTER_INFO_T* pt_reg);
#if CONFIG_SUPPORT_DM_APP
extern VOID a_dm_register(AMB_REGISTER_INFO_T* pt_reg);
#endif
extern VOID a_sm_register(AMB_REGISTER_INFO_T* pt_reg);
extern VOID a_playback_uri_register(AMB_REGISTER_INFO_T* pt_reg);
extern VOID a_playback_tts_register(AMB_REGISTER_INFO_T* pt_reg);
extern VOID a_playback_prompt_register(AMB_REGISTER_INFO_T* pt_reg);
extern VOID a_button_sound_register(AMB_REGISTER_INFO_T* pt_reg);
extern VOID a_timerd_register(AMB_REGISTER_INFO_T* pt_reg);
extern VOID a_acfg_register(AMB_REGISTER_INFO_T*  pt_reg);
extern VOID a_user_interface_register(AMB_REGISTER_INFO_T*  pt_reg);
#if CONFIG_SUPPORT_BT_APP
extern VOID a_bluetooth_register(AMB_REGISTER_INFO_T* pt_reg);
#endif
#if CONFIG_SUPPORT_DLNA_APP
extern VOID a_playback_dlna_register(AMB_REGISTER_INFO_T* pt_reg);
#endif
extern VOID a_wifi_setting_register(AMB_REGISTER_INFO_T* pt_reg);
extern VOID a_misc_register(AMB_REGISTER_INFO_T* pt_reg);
extern VOID a_upg_register(AMB_REGISTER_INFO_T* pt_reg);
extern VOID a_upg_control_register(AMB_REGISTER_INFO_T* pt_reg);
#if CONFIG_SUPPORT_BT_HFP_APP
extern VOID a_bluetooth_hfp_register(AMB_REGISTER_INFO_T* pt_reg);
#endif
#ifdef CONFIG_BLE_SUPPORT
extern VOID a_ble_setting_register(AMB_REGISTER_INFO_T* pt_reg);
#endif /* CONFIG_BLE_SUPPORT */

/*-----------------------------------------------------------------------------
 * private function declarations
 *---------------------------------------------------------------------------*/
VOID x_appl_init (VOID);


/*-----------------------------------------------------------------------------
 * variable declarations
 *---------------------------------------------------------------------------*/
void* g_mas_handle = NULL;

static APP_REG app_to_reg[] = {
		{a_timerd_register,{0},TIMERD_THREAD_NAME},
        {a_acfg_register,{0},ACFG_THREAD_NAME},
		{a_sm_register,{0},SM_THREAD_NAME},
        {a_assistant_stub_register,{0},ASSISTANT_STUB_THREAD_NAME},
	    {a_wifi_setting_register,{0},WIFI_SETTING_THREAD_NAME},
	    {a_misc_register,{0},MISC_THREAD_NAME},
		{a_upg_register,{0},UPG_THREAD_NAME},
        {a_playback_uri_register,{0},PB_URI_THREAD_NAME},
        {a_playback_tts_register,{0},PB_TTS_THREAD_NAME},
        {a_playback_prompt_register,{0},PB_PROMPT_THREAD_NAME},
#if CONFIG_SUPPORT_BUTTON_SOUND_APP
        {a_button_sound_register,{0},BTN_SOUND_THREAD_NAME},
#endif
#if CONFIG_SUPPORT_DM_APP
		{a_dm_register,{0},DM_THREAD_NAME},
#endif
#ifdef CONFIG_BLE_SUPPORT
        {a_ble_setting_register,{0},BLE_THREAD_NAME},
#endif
	    {a_user_interface_register,{0},USER_INTERFACE_THREAD_NAME},
#if CONFIG_SUPPORT_BT_APP
	    {a_bluetooth_register,{0},BLUETOOTH_THREAD_NAME},
#endif
#if CONFIG_SUPPORT_DLNA_APP
	    {a_playback_dlna_register,{0},PB_DLNA_THREAD_NAME},
#endif
#if CONFIG_SUPPORT_BT_HFP_APP
        {a_bluetooth_hfp_register,{0},BLUETOOTH_HFP_THREAD_NAME},
#endif
        {a_upg_control_register, {0}, UPG_CONTROL_THREAD_NAME},
		{NULL,{0},NULL}
};

/*---------------------------------------------------------------------------
 * Name
 *      app_mngr_nfy_fct
 * Description      -
 * Input arguments  -
 * Output arguments -
 * Returns          -
 *---------------------------------------------------------------------------*/
static BOOL app_mngr_nfy_fct (HANDLE_T          h_app_mngr,
                              VOID*             pv_tag,
                              APP_NFY_REASON_T  e_nfy_reason)
{
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
VOID x_appl_init (VOID)
{
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
                     app_mngr_nfy_fct,
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

/*---------------------------------------------------------------------------
 * Name
 *      app main
 * Description      -
 * Input arguments  -
 * Output arguments -
 * Returns          -
 *---------------------------------------------------------------------------*/
int main(int argc, char **argv)
{
	int ret = APPMAIN_OK;
	int status;
	pid_t pid;
	int file;

	/*mtk demo board led show function*/
	/*fd_led = open(LED_FD,0);
	if(fd_led < 0)
	{
		printf("open device LED_FD fail \n");
	}
	led_horse_race(100,COLOR_BLUE);
	led_light_stop();*/

	/*coredump function*/
	if(0 == access("/data/enable_coredump", 0))
	{
		system("ulimit -c 200000");
		system("mkdir /data/coredump");
		system("echo /data/coredump/core.%e.%p.%t > /proc/sys/kernel/core_pattern");
	}
	else
	{
		system("ulimit -c 0");
		system("echo /data/coredump/core.%e.%p.%t > /proc/sys/kernel/core_pattern");
	}

#if CONFIG_SUPPORT_APPMAIN_RESTART
	restart:

	pid = fork();

	if(0 > pid)
	{
		APPMAIN_DEBUG((APPMAIN_TAG"fork child process error at line %d\n",__LINE__));
	}
	else if(0 < pid)
	{
		/*parent process*/
		APPMAIN_DEBUG((APPMAIN_TAG"parent process pid is %d, child process pid is %d\n",getpid(),pid));
		int quit_pid = waitpid(pid,&status,0);
		APPMAIN_DEBUG((APPMAIN_TAG"quit pid is %d, exit status is %d\n",quit_pid,WEXITSTATUS(status)));

		APPMAIN_DEBUG((APPMAIN_TAG"kill btservice\n"));
		/*kill main btservice process*/
		system("killall -9 btservice");

		/*need to remove the alsa device handle used flag*/
		system("rm -f /tmp/mtkac0_opened");
		system("rm -f /tmp/mtkac1_opened");
		system("rm -f /tmp/mtkac2_opened");

		/*restart bt service*/
		system("/usr/bin/btservice&");
		goto restart;
	}
	else
#endif
	{
		/*init output log type*/
		if(0 == access("/data/log_all",0)){
			ui4_enable_all_log = 0;
			APPMAIN_DEBUG((APPMAIN_TAG"disable all ouput in appmainprog!!\n"));
		}
		APPMAIN_DEBUG((APPMAIN_TAG"**********************************************\n"));
		APPMAIN_DEBUG((APPMAIN_TAG"child process id is %d\n",getpid()));
		APPMAIN_DEBUG((APPMAIN_TAG"Application Init Begin\n"));
		APPMAIN_DEBUG((APPMAIN_TAG"Audio Mas process Init\n"));
		CHECK_ASSERT(mas_create(&g_mas_handle));
#if MT8516_SOM
		//I2S 1 playback
		system("amixer -c 0 cset name='O03 I05 Switch' 1");
    	system("amixer -c 0 cset name='O04 I06 Switch' 1");
		system("amixer -c 0 cset name='I2S O03_O04 Switch' 1");
		//I2S 2 loopback
		system("amixer -c 0 cset name='O09 I03 Switch' 1");
    	system("amixer -c 0 cset name='O10 I04 Switch' 1");
		system("amixer -c 0 cset name='AIN Mux' 1");

		//select hw:0,6 to playback
		mas_device_config_ext_t config;    
		mas_chmap_desc_t hw_spk;    
		mas_chmap_desc_t usr_spk;    
		char *outputaddr = "hw:0,6";
		
		memset(&config,0,sizeof(config));    
		hw_spk.channels = 2;    
		hw_spk.chmap[0] = MAS_CHMAP_FL;    
		hw_spk.chmap[1] = MAS_CHMAP_FR;    
		usr_spk.channels = 2;    
		usr_spk.chmap[0] = MAS_CHMAP_FL;    
		usr_spk.chmap[1] = MAS_CHMAP_FR;    
		config.hw_spk_chmap = &hw_spk;    
		config.user_spk_chmap = &usr_spk;    
		config.bits = 32;    
		config.addr = outputaddr;    
		ret = mas_set_device_desc_ext(g_mas_handle, MT_AUDIO_OUT_SPEAKER, &config);    
		if(ret != 0)    {        
			APPMAIN_DEBUG((APPMAIN_TAG"%s,%d,error!\n",__FUNCTION__,__LINE__));           
		}
#endif

#if CONFIG_SUPPORT_BT_HFP_APP
		//if custom's SPK only support 48k, please set fix sample rate to MAS
		CHECK_ASSERT(mas_set_out_samplerate_rule(g_mas_handle,MAS_OUT_SR_FIX,48000));
#endif
		CHECK_ASSERT(mas_start(g_mas_handle));

		APPMAIN_DEBUG((APPMAIN_TAG"OS thread Init\n"));
		CHECK_ASSERT(os_thread_init());

#if CLI_SUPPORT
		APPMAIN_DEBUG((APPMAIN_TAG"Cli Init\n"));
		CHECK_ASSERT(u_cli_init());
#endif

		APPMAIN_DEBUG((APPMAIN_TAG"Dbg Backtrace Init\n"));
		CHECK_ASSERT(dbg_init());

		APPMAIN_DEBUG((APPMAIN_TAG"AEE Init\n"));
		CHECK_ASSERT(aee_init());

		APPMAIN_DEBUG((APPMAIN_TAG"Handle Usr Init\n"));
		CHECK_ASSERT(u_handle_usr_init(256));

		APPMAIN_DEBUG((APPMAIN_TAG"Am Init\n"));
		x_appl_init();

		int i = 0;
		APP_REG * app = app_to_reg;
		while(app->_regfct != NULL)
		{
			APPMAIN_DEBUG((APPMAIN_TAG"%d.%s Init Begin\n",i+1,app->app_name));
			app->_regfct(&app->_reginfo);
			if(APPMAIN_OK == (ret = u_amb_register_app(&app->_reginfo)))
			{
				if(APPMAIN_OK == (ret = u_amb_sync_start_app(app->app_name)))
					APPMAIN_DEBUG((APPMAIN_TAG"%d.%s Init Successful\n",i+1,app->app_name));
				else
					APPMAIN_DEBUG((APPMAIN_TAG"%d.%s sync start fail ret %d\n",i+1,app->app_name,ret));
			}
			else
			{
				APPMAIN_DEBUG((APPMAIN_TAG"%d.%s register fail ret %d\n",i+1,app->app_name,ret));
			}
			app = &app_to_reg[++i];
		}
		APPMAIN_DEBUG((APPMAIN_TAG"Application Init Finish\n"));
		APPMAIN_DEBUG((APPMAIN_TAG"**********************************************\n"));

        #if CONFIG_SUPPORT_ALGO_WENZHI
            system("WenzhiDemo /data/wenzhi/model");
        #endif

		/*mtk demo board led show function*/
		/*led_blink(100,3,COLOR_BLUE);
		led_light_stop();
		led_light(100,COLOR_BLUE);*/

		while(1)
		{
			pid = waitpid(-1,&status,WNOHANG);
			if(pid > 0)
				APPMAIN_DEBUG((APPMAIN_TAG"waitpid pid %d,status = %08x\n",pid,status));
			sleep(1);
		}
	}
}
