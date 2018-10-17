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

/*standard library*/
#include <time.h>

/* middleware level*/
#include "u_appman.h"
#include "u_app_thread.h"
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
#include "ble.h"
#include "ble_svr.h"
#include "ble_svr_timer.h"
#include "ble_client.h"

/*-----------------------------------------------------------------------------
 * structure, constants, macro definitions
 *---------------------------------------------------------------------------*/
typedef struct _BLE_OBJ_T
{
    HANDLE_T        h_app;
    BOOL            b_g_is_init;
    pthread_t       t_thread;
    pthread_mutex_t t_mutex;
    pthread_cond_t  t_cond;
}BLE_OBJ_T;

/*-----------------------------------------------------------------------------
 * variable declarations
 *---------------------------------------------------------------------------*/
static BLE_OBJ_T t_g_ble_obj = {0};

INT32 ble_app_send_msg_to_others(VOID *data,SIZE_T len, int event, int app_uid,char *app_name) {
    INT32 i4_ret;
    HANDLE_T h_app = NULL_HANDLE;

    BLE_FUNCTION_BEGIN;

    if (t_g_ble_obj.b_g_is_init == FALSE) {
        return BLE_FAIL;
    }

    i4_ret = u_am_get_app_handle_from_name(&h_app, app_name);
    if(0 != i4_ret) {
       BLE_LOG(BLE_LOG_INFO,"get handle fail!\n");
        return BLE_FAIL;
    }

    i4_ret = u_app_send_appmsg(h_app,
                              app_uid,
                              MSG_FROM_BLE,
                              event,
                              data,
                              len);
    if(0 == i4_ret) {
       BLE_LOG(BLE_LOG_INFO,"success!");
    } else {
       BLE_LOG(BLE_LOG_ERR,"fail(%d)!\n",i4_ret);
       return BLE_FAIL;
    }

    BLE_FUNCTION_END;

    return BLE_OK;
}

INT32 ble_app_send_msg_to_ble(UINT32  sender_id,VOID *data,SIZE_T len, BLE_TO_BLE_MSG_TYPE event) {
    INT32 i4_ret;
    HANDLE_T h_app = NULL_HANDLE;

    BLE_FUNCTION_BEGIN;

    if (t_g_ble_obj.b_g_is_init == FALSE) {
        return BLE_FAIL;
    }

    i4_ret = u_app_send_appmsg(t_g_ble_obj.h_app,
                              E_APP_MSG_TYPE_BLE,
                              sender_id,
                              (UINT32)event,
                              data,
                              len);
    if(0 == i4_ret) {
       BLE_LOG(BLE_LOG_INFO,"success!");
    } else {
       BLE_LOG(BLE_LOG_ERR,"fail(%d)!\n",i4_ret);
       return BLE_FAIL;
    }

    BLE_FUNCTION_END;

    return BLE_OK;
}

/*-----------------------------------------------------------------------------
 * private methods implementations
 *---------------------------------------------------------------------------*/
static VOID _ble_timer_msg_process(UINT32 timerType) {

    BLE_LOG(BLE_LOG_INFO,"timerType = %d",timerType);

    switch (timerType) {

        case BLE_SERVER_TIMER_EVENT_START_TIMEOUT:
              BLE_LOG(BLE_LOG_INFO," BLE_SERVER_TIMER_EVENT_START_TIMEOUT");
              ble_server_close();
              break;
        case BLE_SERVER_TIMER_EVENT_STOP_TIMEOUT:
              BLE_LOG(BLE_LOG_INFO," BLE_SERVER_TIMER_EVENT_STOP_TIMEOUT");
              ble_server_deinit();
              break;
        default:
              BLE_LOG(BLE_LOG_INFO," short press INVALID");
              break;
    }

}

static VOID _ble_key_process(VOID)
{
    BLE_LOG(BLE_LOG_INFO,"b_g_is_init = %d\n",t_g_ble_obj.b_g_is_init);

    if (t_g_ble_obj.b_g_is_init == FALSE)
	{
        return AEER_FAIL;
    }

    if (ble_server_status_get() == BLE_SERVER_STATUS_OPEN) {
        BLE_LOG(BLE_LOG_INFO,"Close BLE SERVER\n");
        ble_server_close();
    } else {
        BLE_LOG(BLE_LOG_INFO,"Open BLE SERVER\n");
        ble_server_open();
    }

    return;
}

static VOID _ble_itself_msg_process(UINT32 msg_type,VOID *data)
{
    if (t_g_ble_obj.b_g_is_init == FALSE)
    {
        BLE_LOG(BLE_LOG_ERR,"b_g_is_init = %d\n",t_g_ble_obj.b_g_is_init);
        return AEER_FAIL;
    }

    switch(msg_type)
    {
        case BLE_TO_BLE_MSG_INIT_AND_OPEN_SVR:
            BLE_LOG(BLE_LOG_INFO,"Open and init ble server\n");
            if (ble_server_init() != BLE_OK) {
                BLE_LOG(BLE_LOG_INFO,"init ble server fail\n");
                return;
            }
            ble_server_open();
            break;

        case BLE_TO_BLE_MSG_INIT_SVR:
            BLE_LOG(BLE_LOG_INFO,"init ble server\n");
            ble_server_init();
            break;

        case BLE_TO_BLE_MSG_OPEN_SVR:
            BLE_LOG(BLE_LOG_INFO,"open ble server\n");
            ble_server_open();
            break;

        case BLE_TO_BLE_MSG_CLOSE_SVR:
            BLE_LOG(BLE_LOG_INFO,"close ble server\n");
            ble_server_close();
            break;

        default:
            BLE_LOG(BLE_LOG_ERR,"ERR MSG(%d)\n",msg_type);
            break;
	}

    return;
}

static INT32 _ble_start (const CHAR * ps_name,HANDLE_T  h_app)
{
	BLE_FUNCTION_BEGIN;

    /* local variables declaration must be on the top of the fucntion */
    INT32 i4_ret;

    /* init application structue */
    memset(&t_g_ble_obj, 0, sizeof(BLE_OBJ_T));
    t_g_ble_obj.h_app = h_app;

    t_g_ble_obj.b_g_is_init = TRUE;

    i4_ret = pthread_mutex_init(&t_g_ble_obj.t_mutex, NULL);
    if (i4_ret)
    {
        BLE_LOG(BLE_LOG_ERR,"mutex init failed!\n");
        goto MUTEX_INIT_ERR;
    }

    pthread_condattr_t condattr;

    i4_ret = pthread_condattr_init(&condattr);
    if (i4_ret)
    {
        BLE_LOG(BLE_LOG_ERR,"condattr init failed!\n");
        goto ATTR_INIT_ERR;
    }

    i4_ret = pthread_condattr_setclock(&condattr, CLOCK_MONOTONIC);
    if (i4_ret)
    {
        BLE_LOG(BLE_LOG_ERR,"condattr setclock failed!\n");
        goto ATTR_SET_ERR;
    }

    i4_ret = pthread_cond_init(&t_g_ble_obj.t_cond, &condattr);
    if (i4_ret)
    {
        BLE_LOG(BLE_LOG_ERR,"cond init failed!\n");
        goto COND_INIT_ERR;
    }

	BLE_FUNCTION_END;

    return AEER_OK;

COND_INIT_ERR:
ATTR_SET_ERR:
    pthread_condattr_destroy(&condattr);
ATTR_INIT_ERR:
    pthread_mutex_destroy(&t_g_ble_obj.t_mutex);
MUTEX_INIT_ERR:
    return AEER_FAIL;
}

static INT32 _ble_exit (HANDLE_T h_app,APP_EXIT_MODE_T  e_exit_mode)
{
	BLE_FUNCTION_BEGIN;

    if (t_g_ble_obj.b_g_is_init == FALSE)
	{
        return AEER_FAIL;
    }

    t_g_ble_obj.b_g_is_init = FALSE;

	BLE_FUNCTION_END;

    return AEER_OK;
}

static INT32 _ble_process_msg (HANDLE_T     h_app,
					                 UINT32       ui4_type,
					                 const VOID * pv_msg,
					                 SIZE_T       z_msg_len,
					                 BOOL         b_paused)
{
    APPMSG_T *   app_msg = (APPMSG_T * )pv_msg;
	BLE_FUNCTION_BEGIN;

    if (t_g_ble_obj.b_g_is_init == FALSE) {
        return AEER_FAIL;
    }

    if (app_msg == NULL) {
        return AEER_FAIL;
    }

    if (ui4_type < AMB_BROADCAST_OFFSET) {
        /* private message */
        switch(ui4_type) {
            case E_APP_MSG_TYPE_USER_CMD:
                BLE_LOG(BLE_LOG_INFO,"E_APP_MSG_TYPE_USER_CMD\n");
                _ble_key_process();
            break;

            case E_APP_MSG_TYPE_BLE:
                BLE_LOG(BLE_LOG_INFO,"E_APP_MSG_TYPE_BLE\n");
                _ble_itself_msg_process(app_msg->ui4_msg_type,app_msg->p_usr_msg);
            break;

            case E_APP_MSG_TYPE_TIMER:
                BLE_LOG(BLE_LOG_INFO,"E_APP_MSG_TYPE_TIMER\n");
                _ble_timer_msg_process(*(UINT32 *)pv_msg);
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

	BLE_FUNCTION_END;

    return AEER_OK;
}

VOID a_ble_setting_register(AMB_REGISTER_INFO_T* pt_reg)
{
	BLE_FUNCTION_BEGIN;

    if (t_g_ble_obj.b_g_is_init == TRUE) {
        return;
    }

    memset(pt_reg->s_name, 0, sizeof(CHAR)*(APP_NAME_MAX_LEN + 1));
    strncpy(pt_reg->s_name, BLE_THREAD_NAME,APP_NAME_MAX_LEN);
    pt_reg->t_fct_tbl.pf_init                   = _ble_start;
    pt_reg->t_fct_tbl.pf_exit                   = _ble_exit;
    pt_reg->t_fct_tbl.pf_process_msg            = _ble_process_msg;

    pt_reg->t_desc.ui8_flags                    = ~((UINT64)0);
    pt_reg->t_desc.t_thread_desc.z_stack_size   = BLE_STACK_SZ;
    pt_reg->t_desc.t_thread_desc.ui1_priority   = BLE_THREAD_PRIORITY;
    pt_reg->t_desc.t_thread_desc.ui2_num_msgs   = BLE_NUM_MSGS;
    pt_reg->t_desc.ui4_app_group_id = 0;
    pt_reg->t_desc.ui4_app_id = 0;
    pt_reg->t_desc.ui2_msg_count                = BLE_MSGS_COUNT;
    pt_reg->t_desc.ui2_max_msg_size             = BLE_MAX_MSGS_SIZE;

    BLE_FUNCTION_END;

    return ;
}

