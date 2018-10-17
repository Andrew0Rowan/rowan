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


/* middleware level*/
#include "u_appman.h"
#include "u_aee.h"
#include "u_cli.h"
#include "u_misc.h"

#include "u_dbg.h"
#include "u_os.h"

/* application level */
#include "u_amb.h"
#include "u_am.h"
#include "u_common.h"
/* private */
#include "app_ctrl.h"
#include "c_net_config.h"
#include "u_app_thread.h"
#include "u_acfg.h"
#include "u_app_def.h"

#include <time.h>

/*-----------------------------------------------------------------------------
 * structure, constants, macro definitions
 *---------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------
 * variable declarations
 *---------------------------------------------------------------------------*/
APP_CTRL_OBJ_T t_g_app_ctrl_obj = {0};

VOID app_ctrl_timer_msg_process(APP_CTRL_IMER_INDEX_E timerType) {
    APP_CTRL_MSG(ERROR,"timerType = %d",timerType);
    switch (timerType) {
        default:
        APP_CTRL_MSG(ERROR," short press INVALID");
        break;
    }

}

INT32 app_ctrl_send_msg(VOID *data,SIZE_T len, int event, int app_uid,char *app_name) {
    INT32 i4_ret;
    HANDLE_T h_app = NULL_HANDLE;

    APP_CTRL_FUNCTION_BEGIN;

    if (t_g_app_ctrl_obj.b_g_is_init == FALSE) {
        return APP_CTRL_FAIL;
    }

    i4_ret = u_am_get_app_handle_from_name(&h_app, app_name);
    if(0 != i4_ret) {
       APP_CTRL_MSG(ERROR,"[%s %d] get handle fail!,i4_ret=%ld\n",__func__,__LINE__,i4_ret);
       return APP_CTRL_FAIL;
    }

    i4_ret = u_app_send_appmsg(h_app,
                              app_uid,
                              MSG_FROM_APP_CTRL,
                              event,
                              data,
                              len);
    if(0 == i4_ret) {
       APP_CTRL_MSG(INFO,"success!");
    } else {
       APP_CTRL_MSG(ERROR,"fail(%d)!\n",i4_ret);
       return APP_CTRL_FAIL;
    }

	APP_CTRL_FUNCTION_END;

    return APP_CTRL_OK;
}

/*-----------------------------------------------------------------------------
 * private methods implementations
 *---------------------------------------------------------------------------*/

static INT32 _app_ctrl_start (const CHAR * ps_name,HANDLE_T  h_app)
{
	APP_CTRL_FUNCTION_BEGIN;

    /* local variables declaration must be on the top of the fucntion */
    INT32 i4_ret;

    /* init application structue */
    memset(&t_g_app_ctrl_obj, 0, sizeof(APP_CTRL_OBJ_T));
    t_g_app_ctrl_obj.h_app = h_app;

    t_g_app_ctrl_obj.b_g_is_init = TRUE;

#ifdef CLI_SUPPORT
	i4_ret = app_ctrl_cli_attach_cmd_tbl();
	if(i4_ret != CLIR_NOT_INIT && i4_ret != CLIR_OK)
	{
		APP_CTRL_MSG(ERROR,"Err: wifi_cli_attach_cmd_tbl() failed, ret=%ld\r\n", i4_ret);
		return AEER_FAIL;
	}
#endif

    i4_ret = pthread_mutex_init(&t_g_app_ctrl_obj.t_mutex, NULL);
    if (i4_ret)
    {
        APP_CTRL_MSG(ERROR,"mutex init failed!\n");
        goto MUTEX_INIT_ERR;
    }

    pthread_condattr_t condattr;

    i4_ret = pthread_condattr_init(&condattr);
    if (i4_ret)
    {
        APP_CTRL_MSG(ERROR,"condattr init failed!\n");
        goto ATTR_INIT_ERR;
    }

    i4_ret = pthread_condattr_setclock(&condattr, CLOCK_MONOTONIC);
    if (i4_ret)
    {
        APP_CTRL_MSG(ERROR,"condattr setclock failed!\n");
        goto ATTR_SET_ERR;
    }

    i4_ret = pthread_cond_init(&t_g_app_ctrl_obj.t_cond, &condattr);
    if (i4_ret)
    {
        APP_CTRL_MSG(ERROR,"cond init failed!\n");
        goto COND_INIT_ERR;
    }

	APP_CTRL_FUNCTION_END;

    return AEER_OK;

COND_INIT_ERR:
ATTR_SET_ERR:
    pthread_condattr_destroy(&condattr);
ATTR_INIT_ERR:
    pthread_mutex_destroy(&t_g_app_ctrl_obj.t_mutex);
MUTEX_INIT_ERR:
    return AEER_FAIL;
}

static INT32 _app_ctrl_exit (HANDLE_T h_app,APP_EXIT_MODE_T  e_exit_mode)
{
	APP_CTRL_FUNCTION_BEGIN;

    if (t_g_app_ctrl_obj.b_g_is_init == FALSE)
	{
        return AEER_FAIL;
    }

    t_g_app_ctrl_obj.b_g_is_init = FALSE;

	APP_CTRL_FUNCTION_END;

    return AEER_OK;
}

static INT32 _app_ctrl_process_msg (HANDLE_T     h_app,
					                 UINT32       ui4_type,
					                 const VOID * pv_msg,
					                 SIZE_T       z_msg_len,
					                 BOOL         b_paused)
{
    APPMSG_T *   app_msg = (APPMSG_T * )pv_msg;
	APP_CTRL_FUNCTION_BEGIN;

    if (t_g_app_ctrl_obj.b_g_is_init == FALSE) {
        return AEER_FAIL;
    }

    if (t_g_app_ctrl_obj.b_g_is_init == NULL) {
        return AEER_FAIL;
    }

    if (ui4_type < AMB_BROADCAST_OFFSET) {
        /* private message */
        switch(ui4_type) {
            case E_APP_MSG_TYPE_USER_CMD:
                printf("<APP_CTRL> E_APP_MSG_TYPE_USER_CMD\n");
                key_app_event_process(app_msg->ui4_msg_type,app_msg->p_usr_msg);
            break;

            case E_APP_MSG_TYPE_MISC:
                printf("<APP_CTRL> E_APP_MSG_TYPE_MISC\n");
                ntp_app_msg_process(app_msg->ui4_msg_type,app_msg->p_usr_msg);
            break;

            case E_APP_MSG_TYPE_TIMER:
                printf("<APP_CTRL> E_APP_MSG_TYPE_TIMER\n");
                app_ctrl_timer_msg_process(*(APP_CTRL_IMER_INDEX_E *)pv_msg);
            break;

            default:
            break;
        }
    }
	else
	{
        /* system message */
        switch(ui4_type)
		{
            case AM_BRDCST_MSG_POWER_ON:
                break;
            case AM_BRDCST_MSG_POWER_OFF:
                break;
            default:
                break;
        }
    }

	APP_CTRL_FUNCTION_END;

    return AEER_OK;
}

VOID a_app_ctrl_setting_register(AMB_REGISTER_INFO_T* pt_reg)
{
	APP_CTRL_FUNCTION_BEGIN;

    if (t_g_app_ctrl_obj.b_g_is_init == TRUE) {
        return;
    }

    memset(pt_reg->s_name, 0, sizeof(CHAR)*(APP_NAME_MAX_LEN + 1));
    strncpy(pt_reg->s_name, APP_CTRL_THREAD_NAME,APP_NAME_MAX_LEN);
    pt_reg->t_fct_tbl.pf_init                   = _app_ctrl_start;
    pt_reg->t_fct_tbl.pf_exit                   = _app_ctrl_exit;
    pt_reg->t_fct_tbl.pf_process_msg            = _app_ctrl_process_msg;

    pt_reg->t_desc.ui8_flags                    = ~((UINT64)0);
    pt_reg->t_desc.t_thread_desc.z_stack_size   = APP_CTRL_STACK_SZ;
    pt_reg->t_desc.t_thread_desc.ui1_priority   = APP_CTRL_THREAD_PRIORITY;
    pt_reg->t_desc.t_thread_desc.ui2_num_msgs   = APP_CTRL_NUM_MSGS;
    pt_reg->t_desc.ui4_app_group_id = 0;
    pt_reg->t_desc.ui4_app_id = 0;
    pt_reg->t_desc.ui2_msg_count                = APP_CTRL_MSGS_COUNT;
    pt_reg->t_desc.ui2_max_msg_size             = APP_CTRL_MAX_MSGS_SIZE;

    APP_CTRL_FUNCTION_END;

    return ;
}

