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
#include "wifi_setting.h"
#include "c_net_config.h"
#include "u_app_thread.h"
#include "u_acfg.h"
#include "u_app_def.h"
#ifdef CONFIG_ADAPTOR_APP_CTRL
#include "app_ctrl_wifi.h" 
#endif /* CONFIG_ADAPTOR_APP_CTRL */
#ifdef CONFIG_BLE_SUPPORT
#include "ble.h" 
#endif

#include <time.h>

/*-----------------------------------------------------------------------------
 * structure, constants, macro definitions
 *---------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------
 * variable declarations
 *---------------------------------------------------------------------------*/
WIFI_OBJ_T t_g_this_obj = {0};
extern CHAR wifi_ConnectSsid[NET_802_11_MAX_LEN_SSID+1];
extern CHAR wifi_ConnectPwd[WLAN_KEY_MAX_LENGTH+1];
extern INT32 wifi_AuthMode;

#if CONFIG_APP_SUPPORT_SMART_CONNECTION
BOOL   			  SETUP_WIFI_SMART_CONNECTION_IS_ON  = TRUE;
HANDLE_T		  h_start_smart_connection = NULL_HANDLE;
#endif

INT32 wifi_send_msg(const WIFI_MSG_T * pt_event )
{
	WIFI_SETTING_FUNCTION_BEGIN;

    INT32 i4_ret;

    if (t_g_this_obj.b_g_is_init == FALSE)
	{
        return WIFI_FAIL;
    }

    i4_ret = u_app_send_msg (t_g_this_obj.h_app,
							 E_APP_MSG_TYPE_WIFI_SETTING,
							 pt_event,
							 sizeof(WIFI_MSG_T),
							 NULL,
							 NULL);

	WIFI_SETTING_FUNCTION_END;

    return (i4_ret == AEER_OK ? WIFI_OK : WIFI_FAIL);
}

VOID _wifi_setting_process_timer_msg(VOID * pv_msg)
{
	WIFI_SETTING_FUNCTION_BEGIN;

    WIFI_SETTING_TIMER_MSG_E e_msg = *(WIFI_SETTING_TIMER_MSG_E *)pv_msg;
    switch(e_msg)
    {
        case TIMER_MSG_WIFI_SCAN:
			_setup_wlan_scan_timer_cb();
            break;

		case TIMER_MSG_WIFI_ASSCOCIATE:
			_setup_wlan_associate_timer_cb();
            break;

        default:
            break;
    }

	WIFI_SETTING_FUNCTION_END;

	return ;
}

VOID _wifi_setting_process_user_msg(VOID * pv_msg)
{
	WIFI_SETTING_FUNCTION_BEGIN;

	INT32 i4_ret=0;
    APPMSG_T * pt_msg = (APPMSG_T *) pv_msg;

    printf("<WIFI_SETTING> Press key to wifi_open_smart_connection pt_msg->ui4_msg_type = %d\n",pt_msg->ui4_msg_type);

    switch(pt_msg->ui4_msg_type)
    {
        case START_SMART_CONNECTION:
#if CONFIG_APP_SUPPORT_SMART_CONNECTION
            printf("<WIFI_SETTING> Press key to wifi_open_smart_connection\n");
            wifi_open_smart_connection(1);
#endif
            break;

        default:
            break;
    }

	WIFI_SETTING_FUNCTION_END;

	return ;
}


VOID _wifi_setting_process_misc_msg(VOID * pv_msg)
{
	WIFI_SETTING_FUNCTION_BEGIN;

	INT32 i4_ret=0;
    MISC_MSG_T * e_misc_msg = (MISC_MSG_T *)pv_msg;

    switch(e_misc_msg->ui4_msg)
    {
        case CONNECT_FAVORATE_AP:
			i4_ret = setup_wlan_try_connect_favorite_ap_as_dongle_in( );
			if (0 != i4_ret)
			{
			#if 0 /*switch*/
				printf("<WIFI_SETTING> connection favorite ap fail \n");
				_setup_wlan_prepare_to_scan();
			#endif
			}
            break;

        default:
            break;
    }

	WIFI_SETTING_FUNCTION_END;

	return ;
}

#ifdef CONFIG_BLE_SUPPORT
VOID _wifi_setting_process_app_ctrl_msg(VOID * pv_msg)
{
    APPMSG_T * pt_msg = (APPMSG_T *) pv_msg;

    WIFI_SETTING_FUNCTION_BEGIN;

    if (pv_msg == NULL) {
        WIFI_ERR("<APP_CTRL_WIFI> pv_msg is NULL");
        return;
    }

    WIFI_MSG("<APP_CTRL_WIFI> ui4_msg_type = %d",pt_msg->ui4_msg_type);

    switch(pt_msg->ui4_msg_type) {
        case BLE_TO_WIFI_CMD_WIFI_CONNECT:
            wifi_configure_station_connect((CONNECT_AP_INFO *)pt_msg->p_usr_msg);
            break;
        case BLE_TO_WIFI_CMD_WIFI_DISCONNECT:
            wifi_configure_station_disconnect();
            break;
        default:
            break;
    }

	WIFI_SETTING_FUNCTION_END;

	return;
}
#endif /*CONFIG_BLE_SUPPORT*/

/*-----------------------------------------------------------------------------
 * private methods implementations
 *---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------
 * Name
 *      _wifi_start
 * Description      -
 * Input arguments  -
 * Output arguments -
 * Returns          -
 *---------------------------------------------------------------------------*/
static INT32 _wifi_start (const CHAR * ps_name,HANDLE_T  h_app)
{
	WIFI_SETTING_FUNCTION_BEGIN;

    /* local variables declaration must be on the top of the fucntion */
    INT32 i4_ret;

    /* init application structue */
    memset(&t_g_this_obj, 0, sizeof(WIFI_OBJ_T));
    t_g_this_obj.h_app = h_app;

    t_g_this_obj.b_g_is_init = TRUE;
#if 0
	i4_ret = u_timer_create(&h_timer_wlan);
    if (i4_ret != OSR_OK)
    {
        DBG_ERROR(("<OHAYO_WIFI> ERROR: create h_timer_wlan fail %d\n", i4_ret));
        return AEER_FAIL;
    }
#else
	i4_ret = u_timer_create(&t_g_this_obj.t_wlan_timer.h_timer);
    if (i4_ret != OSR_OK)
    {
        WIFI_ERR("create h_timer_wlan fail %d\n", i4_ret);
        goto WLAN_TIMER_ERR;
    }

	i4_ret = u_timer_create(&t_g_this_obj.t_scan_timer.h_timer);
	if (i4_ret != OSR_OK)
	{
		WIFI_ERR("create t_scan_timer fail %d\n", i4_ret);
        goto SCAN_TIMER_ERR;
	}

	i4_ret = u_timer_create(&t_g_this_obj.t_associate_timer.h_timer);
	if (i4_ret != OSR_OK)
	{
		WIFI_ERR("create t_associate_timer fail %d\n", i4_ret);
        goto ASSOCIATE_TIMER_ERR;
	}
#endif

#ifdef CLI_SUPPORT
	i4_ret = wifi_cli_attach_cmd_tbl();
	if(i4_ret != CLIR_NOT_INIT && i4_ret != CLIR_OK)
	{
		WIFI_ERR("Err: wifi_cli_attach_cmd_tbl() failed, ret=%ld\r\n", i4_ret);
		return AEER_FAIL;
	}
#endif

    i4_ret = pthread_mutex_init(&t_g_this_obj.t_mutex, NULL);
    if (i4_ret)
    {
        WIFI_ERR("mutex init failed!\n");
        goto MUTEX_INIT_ERR;
    }

    pthread_condattr_t condattr;

    i4_ret = pthread_condattr_init(&condattr);
    if (i4_ret)
    {
        WIFI_ERR("condattr init failed!\n");
        goto ATTR_INIT_ERR;
    }

    i4_ret = pthread_condattr_setclock(&condattr, CLOCK_MONOTONIC);
    if (i4_ret)
    {
        WIFI_ERR("condattr setclock failed!\n");
        goto ATTR_SET_ERR;
    }

    i4_ret = pthread_cond_init(&t_g_this_obj.t_cond, &condattr);
    if (i4_ret)
    {
        WIFI_ERR("cond init failed!\n");
        goto COND_INIT_ERR;
    }

	WIFI_SETTING_FUNCTION_END;

    return AEER_OK;

COND_INIT_ERR:
ATTR_SET_ERR:
    pthread_condattr_destroy(&condattr);
ATTR_INIT_ERR:
    pthread_mutex_destroy(&t_g_this_obj.t_mutex);
MUTEX_INIT_ERR:
    u_timer_delete(t_g_this_obj.t_associate_timer.h_timer);
ASSOCIATE_TIMER_ERR:
    u_timer_delete(t_g_this_obj.t_scan_timer.h_timer);
SCAN_TIMER_ERR:
    u_timer_delete(t_g_this_obj.t_wlan_timer.h_timer);
WLAN_TIMER_ERR:
    return AEER_FAIL;
}
/*---------------------------------------------------------------------------
 * Name             _wifi_exit
 * Description      -
 * Input arguments  -
 * Output arguments -
 * Returns          -
 *---------------------------------------------------------------------------*/
static INT32 _wifi_exit (HANDLE_T h_app,APP_EXIT_MODE_T  e_exit_mode)
{
	WIFI_SETTING_FUNCTION_BEGIN;

    if (t_g_this_obj.b_g_is_init == FALSE)
	{
        return AEER_FAIL;
    }

	if ((t_g_this_obj.t_scan_timer.h_timer != (HANDLE_T)NULL)&&
		(t_g_this_obj.t_associate_timer.h_timer != (HANDLE_T)NULL))
    {
 #if 0
        u_timer_stop(h_timer_wlan);
        u_timer_delete(h_timer_wlan);
#else
		u_timer_stop(t_g_this_obj.t_scan_timer.h_timer);
		u_timer_delete(t_g_this_obj.t_scan_timer.h_timer);
		u_timer_stop(t_g_this_obj.t_associate_timer.h_timer);
		u_timer_delete(t_g_this_obj.t_associate_timer.h_timer);
#endif
    }


    t_g_this_obj.b_g_is_init = FALSE;

	WIFI_SETTING_FUNCTION_END;

    return AEER_OK;
}

/*---------------------------------------------------------------------------
 * Name
 *      _wifi_process_msg
 * Description      -
 * Input arguments  -
 * Output arguments -
 * Returns          -
 *---------------------------------------------------------------------------*/
static INT32 _wifi_process_msg (HANDLE_T     h_app,
					                 UINT32       ui4_type,
					                 const VOID * pv_msg,
					                 SIZE_T       z_msg_len,
					                 BOOL         b_paused)
{

	WIFI_SETTING_FUNCTION_BEGIN;

    if (t_g_this_obj.b_g_is_init == FALSE)
	{
        return AEER_FAIL;
    }

    if (ui4_type < AMB_BROADCAST_OFFSET)
	{
        /* private message */
        printf("<WIFI_SETTING> Press key to wifi_open_smart_connection ui4_type = %d\n",ui4_type);

		switch(ui4_type)
		{
            case E_APP_MSG_TYPE_TIMER:
			{
                _wifi_setting_process_timer_msg(pv_msg);
            }
            break;

			case E_APP_MSG_TYPE_MISC:
			{
                _wifi_setting_process_misc_msg(pv_msg);
            }
            break;

            case E_APP_MSG_TYPE_WIFI_SETTING:
			{
				WIFI_MSG_T* pt_msg = (WIFI_MSG_T*) pv_msg;
				switch(pt_msg->ui4_msg_id)
				{
					case WIFI_WLAN_MSG:
					{
						_wifi_setting_wlan_notify_msg_proc(pv_msg);
					}
					break;

					case WIFI_SOFTAP_MSG:
					{
					}
					break;

					default:
						break;
				}
            }
            break;

			case E_APP_MSG_TYPE_ASSISTANT_STUB:
			{
				APPMSG_T * pt_msg = (APPMSG_T *) pv_msg;
				switch(pt_msg->ui4_sender_id)
				{
					case MSG_FROM_ASSISTANT_STUB:
					{
						_wifi_setting_handle_assistant_stub_msg(pt_msg);
					}
					break;

					default:
						break;
				}
            }
            break;

			case E_APP_MSG_TYPE_USER_CMD:
			{
                _wifi_setting_process_user_msg(pv_msg);
            }
            break;

#ifdef CONFIG_ADAPTOR_APP_CTRL
			case E_APP_MSG_TYPE_APP_CTRL:
			{
                _wifi_setting_process_app_ctrl_msg(pv_msg);
            }
            break;
#endif /* CONFIG_ADAPTOR_APP_CTRL */

#ifdef CONFIG_BLE_SUPPORT
			case E_APP_MSG_TYPE_BLE:
			{
                _wifi_setting_process_app_ctrl_msg(pv_msg);
            }
            break;
#endif /* CONFIG_BLE_SUPPORT */

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


	WIFI_SETTING_FUNCTION_END;

    return AEER_OK;
}

VOID a_wifi_setting_register(AMB_REGISTER_INFO_T* pt_reg)
{
	WIFI_SETTING_FUNCTION_BEGIN;

    if (t_g_this_obj.b_g_is_init == TRUE)
	{
        return;
    }

    memset(pt_reg->s_name, 0, sizeof(CHAR)*(APP_NAME_MAX_LEN + 1));
	strncpy(pt_reg->s_name, WIFI_SETTING_THREAD_NAME,APP_NAME_MAX_LEN);
    pt_reg->t_fct_tbl.pf_init                   = _wifi_start;
    pt_reg->t_fct_tbl.pf_exit                   = _wifi_exit;
    pt_reg->t_fct_tbl.pf_process_msg            = _wifi_process_msg;

    pt_reg->t_desc.ui8_flags                    = ~((UINT64)0);
    pt_reg->t_desc.t_thread_desc.z_stack_size   = WIFI_SETTING_STACK_SZ;
    pt_reg->t_desc.t_thread_desc.ui1_priority   = WIFI_SETTING_THREAD_PRIORITY;
    pt_reg->t_desc.t_thread_desc.ui2_num_msgs   = WIFI_SETTING_NUM_MSGS;

    pt_reg->t_desc.ui2_msg_count                = WIFI_SETTING_MSGS_COUNT;
    pt_reg->t_desc.ui2_max_msg_size             = WIFI_SETTING_MAX_MSGS_SIZE;

	WIFI_SETTING_FUNCTION_END;

    return ;
}


