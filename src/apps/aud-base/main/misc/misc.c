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



/*-----------------------------------------------------------------------------
                    include files
-----------------------------------------------------------------------------*/
/* middleware level*/
#include    "u_appman.h"
#include    "u_app_thread.h"
#include    "u_aee.h"
#include    "u_am.h"
#include    "u_amb.h"
#include    "u_cli.h"
#include    "u_dbg.h"
#include    "u_os.h"
#include    "c_net_config.h"
#include    "c_net_wlan.h"
#include    <stdlib.h>
#include    "misc.h"

MISC_OBJ_T misc_this_obj = {0};

static VOID _misc_handle_timer_msg(VOID*  pv_msg)
{
	MISC_FUNCTION_BEGIN

    MISC_TIMER_MSG_E e_msg = *(MISC_TIMER_MSG_E *)pv_msg;
    switch(e_msg)
    {
        case TIMER_MSG_AUTO_IP:
            _misc_process_auto_ip_timer_msg();
            break;
        default:
            break;
    }

	MISC_FUNCTION_END

	return ;
}

static VOID _misc_handle_wifi_setting_msg(APPMSG_T * t_app_msg)
{
	MISC_FUNCTION_BEGIN

	switch(t_app_msg->ui4_msg_type)
	{
		case MISC_PRI_DHCP_MSG:
		{
			MISC_NOTIFY_MSG_T * t_notify_msg =
							    (MISC_NOTIFY_MSG_T*)(t_app_msg->p_usr_msg);

			misc_dhcp_msg_process(t_notify_msg);
		}
		break;

		case MISC_PRI_DEVICE_ACTIVE_MSG:
		{
			MISC_DEVICE_ACTIVE_MSG_T * t_device_active_msg =
									   (MISC_DEVICE_ACTIVE_MSG_T*)(t_app_msg->p_usr_msg);

			misc_device_active_msg_process(t_device_active_msg);
		}
		break;

		default:
		{
			printf("<ASSISTANT_STUB_APP> ERROR SM MSG TYPE!\n");
		}
		break;

	}

	MISC_FUNCTION_END
}

static INT32 _misc_app_init(const CHAR* ps_name, HANDLE_T h_app)
{
	MISC_FUNCTION_BEGIN

	INT32    i4_ret;
    HANDLE_T h_thread = NULL_HANDLE;
	UINT8    volume;

	memset(&misc_this_obj, 0, sizeof(MISC_OBJ_T));

	misc_this_obj.h_app=h_app;

	if (misc_this_obj.b_app_init_ok)
    {
        return AEER_OK;
    }

	i4_ret = u_timer_create(&misc_this_obj.auto_ip_timer.h_timer);
	if (i4_ret != AEER_OK)
	{
		printf("<MISC> ERROR: create auto_ip_timer fail %d\n", i4_ret);
		return AEER_FAIL;
	}

#ifdef CLI_SUPPORT
	i4_ret = _misc_cli_attach_cmd_tbl();
	if ((CLIR_NOT_INIT != i4_ret) && (CLIR_OK != i4_ret))
	{
		printf("Err: _misc_cli_attach_cmd_tbl() failed, ret=%ld\r\n",i4_ret);
		return AEER_FAIL;
	}
	_misc_set_dbg_level(DBG_INIT_LEVEL_APP_MISC);
#endif/* CLI_SUPPORT */

	a_network_init_wlan();

	misc_this_obj.b_app_init_ok=TRUE;

	MISC_FUNCTION_END

	return AEER_OK;
}

static INT32 _misc_app_process_msg(HANDLE_T     h_app,
										   UINT32       ui4_type,
										   const VOID*  pv_msg,
										   SIZE_T       z_msg_len,
										   BOOL         b_paused)
{
	MISC_FUNCTION_BEGIN

	INT32	   i4_ret = 0;
	UCHAR*	   puc_name = NULL;
	APPMSG_T * t_app_msg= (APPMSG_T*)pv_msg;

	if (!misc_this_obj.b_app_init_ok)
	{
		return AEER_FAIL;
	}

	if (ui4_type < AMB_BROADCAST_OFFSET)
	{
			/* private message */
		 switch (ui4_type)
		 {
		 	case E_APP_MSG_TYPE_TIMER:
			{
                _misc_handle_timer_msg(pv_msg);
			}
            break;

			case E_APP_MSG_TYPE_MISC:
			{
				printf("<MISC> E_APP_MSG_TYPE_MISC\n");
				switch(t_app_msg->ui4_sender_id)
				{
					case MSG_FROM_WIFI_SETTING:
					{
						printf("<MISC> MSG_FROM_WIFI_SETTING\n");
						_misc_handle_wifi_setting_msg(t_app_msg);
					}
					break;

					default:
					{
						printf("<MISC> ERROR sender id\n");
					}
					break;
				}
            }
			break;

			default:
				printf("<MISC> ERROR ui4_type\n");
				break;
		 }
	}

	else if (ui4_type < AMB_BROADCAST_MISC)
	{
		/* AM broadcast message */
		switch (ui4_type)
		{
			case AM_BRDCST_MSG_SYSTEM_KEY:
			{
				printf("<MISC>AM_BRDCST_MSG_SYSTEM_KEY\n");
				break;
			}

			default:
				printf("<MISC>other AM broadcast message\n");
				break;
		}
	}
	else
	{
		printf("<MISC>other message\n");
	}

	MISC_FUNCTION_END

	return AEER_OK;
}


static INT32 _misc_app_exit(HANDLE_T h_app, APP_EXIT_MODE_T e_exit_mode)
{
	MISC_FUNCTION_BEGIN

	misc_this_obj.b_app_init_ok=FALSE;

	MISC_FUNCTION_END

	return AEER_OK;
}

VOID a_misc_register(AMB_REGISTER_INFO_T* pt_reg)
{
	MISC_FUNCTION_BEGIN

    if (misc_this_obj.b_app_init_ok)
    {
		printf("<MISC> a_misc_register done,just return\n");
        return;
    }

    strncpy(pt_reg->s_name, MISC_THREAD_NAME,sizeof(MISC_THREAD_NAME));
    pt_reg->t_fct_tbl.pf_init                 = _misc_app_init;
    pt_reg->t_fct_tbl.pf_exit                 = _misc_app_exit;
    pt_reg->t_fct_tbl.pf_process_msg          = _misc_app_process_msg;

	pt_reg->t_desc.ui8_flags 				  = ~((UINT64)0);
    pt_reg->t_desc.t_thread_desc.z_stack_size = MISC_STACK_SZ;
    pt_reg->t_desc.t_thread_desc.ui1_priority = MISC_THREAD_PRIORITY;
    pt_reg->t_desc.t_thread_desc.ui2_num_msgs = MISC_THREAD_MSG_NUM;

    pt_reg->t_desc.ui2_msg_count 			  = MISC_MSGS_COUNT;
    pt_reg->t_desc.ui2_max_msg_size 		  = MISC_MAX_MSGS_SIZE;

	MISC_FUNCTION_END

}
