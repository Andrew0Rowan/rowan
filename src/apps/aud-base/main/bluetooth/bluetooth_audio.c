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

#include <string.h>
#include<unistd.h>
#include <pthread.h>

/* application level */
#include "u_appman.h"
#include "u_aee.h"
#include "u_cli.h"
#include "u_amb.h"
#include "u_am.h"
#include "u_app_thread.h"
#include "u_dbg.h"
#include "u_os.h"
#include "u_acfg.h"
#include "u_key_def.h"
#include "u_dm.h"
#include "u_timerd.h"
#include "u_assistant_stub.h"

/*   btmw   */
#include "mtk_bt_service_gap_wrapper.h"
#include "mtk_bt_service_a2dp_wrapper.h"
#include "mtk_bt_service_avrcp_wrapper.h"
#include "rw_init_mtk_bt_service.h"
#include "mtk_bt_service_hfclient_wrapper.h"

/* private */
#include "bluetooth_audio_proc.h"
#include "u_bluetooth_audio.h"
#include "bluetooth_audio.h"
#include "bluetooth_audio_key_proc.h"
//#include "bluetooth_audio_key_proc.h"
#if CONFIG_SUPPORT_BT_HFP
#include "bluetooth_hfp.h"
#include "bluetooth_hfp_proc.h"
#endif

/*-----------------------------------------------------------------------------
 * structure, constants, macro definitions
 *---------------------------------------------------------------------------*/


/*-----------------------------------------------------------------------------
 * variable declarations
 *---------------------------------------------------------------------------*/
BT_AUD_OBJ_T g_t_bt_aud = {0};
extern BT_AUD_BLUETOOTH_STATUS_T g_t_bt_aud_sta;
extern BT_A2DP_DEVICE_LIST g_t_bt_pair_dev_list;
static UINT16 ui2_bt_aud_dbg_level = DBG_INIT_LEVEL_APP_BT_AUD;
static pthread_t t_bt_aud_init_thread;

/*-----------------------------------------------------------------------------
 * private methods declarations
 *---------------------------------------------------------------------------*/
static INT32 _bluetooth_audio_app_init (const CHAR* ps_name,
                                            HANDLE_T h_app);
static INT32 _bluetooth_audio_app_exit (HANDLE_T h_app,
                                            APP_EXIT_MODE_T e_exit_mode);
static INT32 _bluetooth_audio_app_process_msg (HANDLE_T h_app,
                                                        UINT32 ui4_type,
                                                        const VOID* pv_msg,
                                                        SIZE_T z_msg_len,
                                                        BOOL b_paused);
static VOID _bluetooth_send_btkey(UINT32 ui4_keysta);
static INT32 _bluetooth_bt_init_thread_creat(VOID);
static VOID _bluetooth_init_thread_exit(VOID); 


/*-----------------------------------------------------------------------------
 * extern methods declarations
 *---------------------------------------------------------------------------*/
extern INT32 _bluetooth_cli_attach_cmd_tbl(VOID);
extern INT32 _bluetooth_cli_detach_cmd_tbl(VOID);



VOID a_bluetooth_register(AMB_REGISTER_INFO_T* pt_reg)
{
    if (g_t_bt_aud.b_app_init_ok == TRUE)
    {
        DBG_ERROR((BTAUD_TAG"Err: bluetooth already init.\r\n"));
        return;
    }

    /* Application can only use middleware's c_ API */
    memset(pt_reg->s_name, 0, sizeof(CHAR)*(APP_NAME_MAX_LEN + 1));
    strncpy(pt_reg->s_name, BLUETOOTH_THREAD_NAME, APP_NAME_MAX_LEN);
    pt_reg->t_fct_tbl.pf_init                   = _bluetooth_audio_app_init;
    pt_reg->t_fct_tbl.pf_exit                   = _bluetooth_audio_app_exit;
    pt_reg->t_fct_tbl.pf_process_msg            = _bluetooth_audio_app_process_msg;
    pt_reg->t_desc.ui8_flags                    = AEE_FLAG_WRITE_CONFIG|AEE_FLAG_WRITE_FLM|AEE_FLAG_WRITE_TSL|AEE_FLAG_WRITE_SVL;
    pt_reg->t_desc.t_thread_desc.z_stack_size   = BLUETOOTH_STACK_SIZE;
    pt_reg->t_desc.t_thread_desc.ui1_priority   = BLUETOOTH_THREAD_PRIORITY;
    pt_reg->t_desc.t_thread_desc.ui2_num_msgs   = BLUETOOTH_NUM_MSGS;//20;
    pt_reg->t_desc.ui2_msg_count                = BLUETOOTH_MSGS_COUNT;//20;
    pt_reg->t_desc.ui2_max_msg_size             = BLUETOOTH_MAX_MSGS_SIZE;//4096;

    /* init application structue */
    memset(&g_t_bt_aud, 0, sizeof(BT_AUD_OBJ_T));
    g_t_bt_aud.h_app = NULL_HANDLE;

    return;
}


UINT16 _bluetooth_get_dbg_level(VOID)
{
    return (ui2_bt_aud_dbg_level | DBG_LAYER_APP);
}

VOID _bluetooth_set_dbg_level(UINT16 ui2_db_level)
{
    ui2_bt_aud_dbg_level = ui2_db_level;
}

static INT32 _bluetooth_audio_app_init (const CHAR* ps_name,
                                        HANDLE_T h_app)
{
    INT32 i4_ret;
//	BT_DM_TIMER_MSG_E e_bt_dm_msg;

    DBG_API((BTAUD_TAG"_bluetooth_audio_app_init.\r\n"));

    if (g_t_bt_aud.b_app_init_ok)
    {
        DBG_API((BTAUD_TAG"bluetooth already init.\r\n"));
        return AEER_OK;
    }
    g_t_bt_aud.h_app = h_app;

#ifdef CLI_SUPPORT
    i4_ret = _bluetooth_cli_attach_cmd_tbl();
    if ((i4_ret != CLIR_NOT_INIT) && (i4_ret != CLIR_OK))
    {
        DBG_ERROR((BTAUD_TAG"Err: _bluetooth_cli_attach_cmd_tbl() failed, ret=%ld\r\n",i4_ret ));
        return AEER_FAIL;
    }
#endif/* CLI_SUPPORT */

    _bluetooth_set_dbg_level(DBG_INIT_LEVEL_APP_BT_AUD);

    _bluetooth_timer_create();


    u_cfg_set_bluetooth_enable(BLUETOOTH_ENABLE);   //make sure bluetooth always enable

    g_t_bt_aud.b_app_init_ok = TRUE;
//    sleep(2);
    do{
       i4_ret = access(DONGLE_NODE_PATH,0);
	   DBG_ERROR((BTAUD_TAG" access DONGLE_NODE_PATH \n"));
	}
	while(i4_ret);

    DBG_ERROR((BTAUD_TAG" _bluetooth_bt_init_thread_creat... \n"));
    i4_ret = _bluetooth_bt_init_thread_creat();
    BT_AUD_CHK_FAIL(_bluetooth_bt_init_thread_creat, i4_ret);

    return AEER_OK;
}

static INT32 _bluetooth_audio_app_exit (HANDLE_T h_app,
                                        APP_EXIT_MODE_T e_exit_mode)
{
    DBG_API((BTAUD_TAG"_bluetooth_audio_app_exit.\r\n"));

    if (g_t_bt_aud.b_app_init_ok == FALSE)
    {
        DBG_ERROR((BTAUD_TAG"Err: bluetooth audio doesn't init!\r\n"));
        return AEER_FAIL;
    }

    _bluetooth_init_thread_exit();

    _bluetooth_bt_close(FALSE);

//    _bluetooth_cli_detach_cmd_tbl();

    _bluetooth_timer_delete();

    g_t_bt_aud.b_app_init_ok = FALSE;

    return AEER_OK;
}

static INT32 _bluetooth_itself_msg_process(APPMSG_T *pv_msg)
{
    INT32 i4_ret = BT_AUD_FAIL;
	BT_AUD_MSG_T* pt_bt_gap_msg = (BT_AUD_MSG_T*)pv_msg;

	BT_AUD_FUNC_ENTRY();


	DBG_ERROR((BTAUD_TAG"BLUETOOTH msg come.\r\n"));
	
	BT_AUD_MSG_T *pt_bt_msg = (BT_AUD_MSG_T *)pv_msg;

	switch(pt_bt_msg->ui4_msg_type)
	{
		DBG_ERROR((BTAUD_TAG"BLUETOOTH pt_bt_msg->ui4_msg_type is : %d \r\n",pt_bt_msg->ui4_msg_type));
	
		case BT_AUD_PTE_MSG_BT:
		i4_ret = _bluetooth_profile_msg_proc(pt_bt_msg);
		BT_AUD_CHK_FAIL_RET(_bluetooth_profile_msg_proc, i4_ret, AEER_FAIL);
		break;
#if CONFIG_APP_SUPPORT_BT_SRC_MODE
		case BT_AUD_PTE_MSG_SRC:
		i4_ret = _bluetooth_src_itself_msg_proc(&pt_bt_msg->btmedialnfo);
		BT_AUD_CHK_FAIL(_bluetooth_src_itself_msg_proc, i4_ret);
		break;
#endif /*CONFIG_APP_SUPPORT_BT_SRC_MODE*/	
		/*PLAY_CMD	message*/
		case BT_AUD_PTE_MSG_KEY:
		i4_ret = bt_aud_key_msg_proc(pt_bt_msg);
		BT_AUD_CHK_FAIL_RET(bt_aud_key_msg_proc, i4_ret, AEER_FAIL);
		break;
	
		default:
			DBG_ERROR((BTAUD_TAG"Err: unkown BTLUETOOTH private msg, msgid=%d.\r\n", pt_bt_msg->ui4_msg_id));
			break;	
	}

	BT_AUD_FUNC_EXIT();
    return BT_AUD_OK;

}


static INT32 _bluetooth_audio_app_process_msg (HANDLE_T h_app,
                                                UINT32 ui4_type,
                                                const VOID* pv_msg,
                                                SIZE_T z_msg_len,
                                                BOOL b_paused)
{
    INT32 i4_ret = BT_AUD_FAIL;

    DBG_INFO((BTAUD_TAG"_bluetooth_audio_app_process_msg, msgtype=%d.\r\n",ui4_type));
    if (g_t_bt_aud.b_app_init_ok == FALSE)
    {
        DBG_ERROR((BTAUD_TAG"Err: bluetooth audio doesn't init!\r\n"));
        return AEER_FAIL;
    }

    if (ui4_type < AMB_BROADCAST_OFFSET)
    {
        //DBG_INFO((BTAUD_TAG"private msg come.\r\n"));
        /* private message */
        switch(ui4_type)
        {
		    case E_APP_MSG_TYPE_TIMER:
                _bluetooth_process_timer_msg(pv_msg, z_msg_len);
                break;

            case E_APP_MSG_TYPE_USER_CMD:
            {
                _bluetooth_ui_msg_process(pv_msg, z_msg_len);
            }
            break;

			case E_APP_MSG_TYPE_BLUETOOTH:
			{
				_bluetooth_itself_msg_process(pv_msg);
			}
			break;

			case E_APP_MSG_TYPE_URI:
			{
            	_bluetooth_playback_msg_process((APPMSG_T *)pv_msg);
			}
			break;

			/*state manager message*/
			case E_APP_MSG_TYPE_STATE_MNGR:
			{
				_bluetooth_sm_msg_process((APPMSG_T *)pv_msg);
			}
			break;

            /*ALI_STUB message*/
			case E_APP_MSG_TYPE_ASSISTANT_STUB:
			{
				APPMSG_T *pt_notify_msg;

                DBG_INFO((BTAUD_TAG"ASSISTANT_STUB msg come.\r\n"));

                pt_notify_msg = (APPMSG_T*)pv_msg;

                if (MSG_FROM_ASSISTANT_STUB == pt_notify_msg->ui4_sender_id)
                {
                    i4_ret = _bluetooth_assistant_stub_msg_proc(pt_notify_msg);
                    BT_AUD_CHK_FAIL_RET(_bluetooth_assistant_stub_msg_proc, i4_ret, AEER_FAIL);
#if 0
					if(ALI_STUB_CMD_SET_BT_NAME == pt_notify_msg->ui4_msg_type)
					{
                        DBG_ERROR((BTAUD_TAG"ALI_STUB set BT name \n"));
						i4_ret = _bluetooth_set_name_assistant_stub(pt_notify_msg->p_usr_msg);
						BT_AUD_CHK_FAIL_RET(_bluetooth_assistant_stub_msg_proc, i4_ret, BT_AUD_FAIL);
					}
#endif
                }
			}
			break;

            case E_APP_MSG_TYPE_WIFI_SETTING:
            {
                APPMSG_T *pt_notify_msg;

                DBG_ERROR((BTAUD_TAG"WIFI_SETTING msg come.\r\n"));

                pt_notify_msg = (APPMSG_T*)pv_msg;

                i4_ret = _bluetooth_wifi_status_msg_proc(pt_notify_msg);
                BT_AUD_CHK_FAIL_RET(_bluetooth_wifi_status_msg_proc, i4_ret, AEER_FAIL);
            }
            break;

            case E_APP_MSG_TYPE_MISC:
            {
                APPMSG_T *pt_notify_msg;

                DBG_ERROR((BTAUD_TAG"MISC msg come.\r\n"));

                pt_notify_msg = (APPMSG_T*)pv_msg;

                i4_ret = _bluetooth_wifi_status_msg_proc(pt_notify_msg);
                BT_AUD_CHK_FAIL_RET(_bluetooth_wifi_status_msg_proc, i4_ret, AEER_FAIL);
            }
            break;

            default:
                DBG_ERROR((BTAUD_TAG"Err: unkown private msg, msgtype=%d.\r\n", ui4_type));
                break;
        }

    }
    else
    {
        DBG_ERROR((BTAUD_TAG"AMB_BROADCAST msg come.\r\n"));
    }


    DBG_ERROR((BTAUD_TAG"_bluetooth_audio_app_process_msg done.\r\n"));
    return AEER_OK;
}


INT32 _bluetooth_send_msg_to_itself(UINT32 ui4_msgtype, BT_AUD_MSG_T* pt_bt_msg)
{
    INT32 i4_ret = 0;

    //send msg to bt thread
    if (NULL_HANDLE == g_t_bt_aud.h_app)
    {
        DBG_ERROR((BTAUD_TAG"bluetooth audio app has a NULL_HANDLE.\r\n"));
        return BT_AUD_FAIL;
    }

    DBG_INFO((BTAUD_TAG"send msg to BT thread, pt_bt_msg->ui4_msg_id=%d.\r\n", pt_bt_msg->ui4_msg_id));

    i4_ret = u_app_send_msg(g_t_bt_aud.h_app,
                    ui4_msgtype,
                    pt_bt_msg,
                    sizeof(BT_AUD_MSG_T),
                    NULL,
                    NULL);
    BT_AUD_CHK_FAIL_RET(u_app_send_msg, i4_ret, BT_AUD_FAIL);

    return BT_AUD_OK;

}

/**-----------------------------------------------------------------------------
  * Name          - _bluetooth_set_bt_power
  * Description  - set the power of bluetooth device.
  * Inputs        - b_on power is set to on or off
  * Returns       -
  *---------------------------------------------------------------------------*/
INT32 _bluetooth_set_bt_power(BOOL b_power_on)
{
    INT32 i4_ret = BT_AUD_OK;

    //DBG_API((BTAUD_TAG"set bt power=%d.\r\n", b_power_on));
    DBG_ERROR((BTAUD_TAG" set bt power=%d.\r\n", b_power_on));

    if(g_t_bt_aud.b_bt_has_power_on == b_power_on)
    {
        DBG_ERROR((BTAUD_TAG"bt power no need change.\r\n"));
        return BT_AUD_OK;
    }

    if(b_power_on)
    {
        i4_ret = a_mtkapi_bt_gap_on_off(TRUE);    
		DBG_ERROR((BTAUD_TAG" set bt power on. \r\n"));
    }
    else
    {
        i4_ret = a_mtkapi_bt_gap_on_off(FALSE);   
		DBG_ERROR((BTAUD_TAG" set bt power off. \r\n"));
    }

   g_t_bt_aud.b_bt_has_power_on = b_power_on;

    return i4_ret;
}

/**-----------------------------------------------------------------------------
  * Name          - _bluetooth_is_bt_power_on
  * Description  - the power of bluetooth device is on or off
  * Returns       - TRUE: the power of bluetooth device is on, FALSE: the power of bluetooth device is off
  *---------------------------------------------------------------------------*/
BOOL _bluetooth_is_bt_power_on(VOID)
{
    return g_t_bt_aud.b_bt_has_power_on;
}
/**-----------------------------------------------------------------------------
  * Name          - _bluetooth_is_bt_power_setting_on
  * Description  - bluetooth power setting is on or off
  * Returns       - TRUE: bluetooth power setting is on, FALSE: bluetooth power setting is off
  *---------------------------------------------------------------------------*/
BOOL _bluetooth_is_bt_power_setting_on(VOID)
{
    UINT8 ui1_value = 0;

    u_cfg_get_bluetooth_enable(&ui1_value);
    if(ui1_value == BLUETOOTH_ENABLE)
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

static VOID _bluetooth_send_btkey(UINT32 ui4_keysta)
{
    BT_AUD_MSG_T t_msg = {0};
    INT32 i4_ret;

    DBG_INFO((BTAUD_TAG"send btkey to bluetooth audio, keysta=0x%x.\r\n",ui4_keysta));

    t_msg.ui4_msg_type = BT_AUD_PTE_MSG_KEY;
	t_msg.ui4_msg_id = BT_AUD_MSG_KEY_BTKEY;
    t_msg.ui4_data1 = ui4_keysta;
    t_msg.ui4_data2= BT_AUD_KEY_BLUETOOTH;
	
    i4_ret = _bluetooth_send_msg_to_itself(E_APP_MSG_TYPE_BLUETOOTH, &t_msg);
    BT_AUD_CHK_FAIL(_bluetooth_send_msg_to_itself, i4_ret);
}

INT32 a_bluetooth_btkey_handle(UINT32 ui4_keysta, UINT32 ui4_keyval)
{
    static UINT32 ui4_last_keysta = BT_AUD_KEYSTA_UP;

    if (ui4_keyval != BT_AUD_KEY_BLUETOOTH)
    {
        DBG_ERROR((BTAUD_TAG"Err: not BLUETOOTH key.\r\n"));
        return APPUTILR_FAIL;
    }

    switch(ui4_keysta)
    {
        case BT_AUD_KEYSTA_DOWN:
            DBG_INFO((BTAUD_TAG"BTkey DOWN.\r\n"));
            break;

        case BT_AUD_KEYSTA_UP:
            DBG_INFO((BTAUD_TAG"BTkey UP.\r\n"));
            if (ui4_last_keysta == BT_AUD_KEYSTA_DOWN)
            {
                _bluetooth_send_btkey(BT_AUD_KEY_SRC_SWITCH);
            }
            break;

        case BT_AUD_KEYSTA_REPEAT:
            DBG_INFO((BTAUD_TAG"BTkey REPEAT.\r\n"));
            _bluetooth_send_btkey(BT_AUD_KEY_FORCED_PAIRING);
            break;

        default:
            DBG_ERROR((BTAUD_TAG"Err: unkown BTkey keysta.\r\n"));
            return APPUTILR_FAIL;
    }

    ui4_last_keysta = ui4_keysta;
    return APPUTILR_OK;
}


VOID _bluetooth_bt_init_thread(VOID)
{
    INT32 i4_ret;

    pthread_detach(pthread_self());

    BT_AUD_FUNC_ENTRY();

#ifndef CONFIG_ADAPTOR_APP_CTRL
    /***      RPC init      ***/
    DBG_ERROR((BTAUD_TAG"---------IPC/RPC initialize----------- \r\n"));
    a_mtk_bt_service_init();
    sleep(3);
    DBG_ERROR((BTAUD_TAG"---------IPC/RPC init Done!!!--------- \r\n"));
#endif

    i4_ret = _bluetooth_bt_init();//  Bluetooth init
    BT_AUD_CHK_FAIL_RET(_bluetooth_bt_init, i4_ret, BT_AUD_FAIL);

    pause();

    BT_AUD_FUNC_EXIT();
}

static INT32 _bluetooth_bt_init_thread_creat(VOID)
{
    INT32 i4_ret;

    BT_AUD_FUNC_ENTRY();

    i4_ret = pthread_create(&t_bt_aud_init_thread, NULL, _bluetooth_bt_init_thread, NULL);
    if (0 != i4_ret)
    {
        DBG_ERROR((BTAUD_TAG"bt aud init thread create failed!\r\n"));
        return BT_AUD_FAIL;
    }

//    pthread_join(t_bt_aud_init_thread, NULL);

    BT_AUD_FUNC_EXIT();

    return BT_AUD_OK;
}

static VOID _bluetooth_init_thread_exit(VOID)
{
    INT32 i4_ret;

    BT_AUD_FUNC_ENTRY();

    i4_ret = pthread_cancel(t_bt_aud_init_thread);
    if (0 != i4_ret)
    {
        DBG_ERROR((BTAUD_TAG"bt aud init thread cancle failed!\r\n"));
    }

    BT_AUD_FUNC_EXIT();
}


