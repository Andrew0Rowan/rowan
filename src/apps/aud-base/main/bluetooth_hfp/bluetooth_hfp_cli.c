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
#include<unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

/* middleware level*/
#include "c_bt_mw_hidh.h"
#include "u_appman.h"
#include "u_aee.h"
#include "u_cli.h"
#include "u_app_thread.h"
#include "u_dbg.h"
#include "u_os.h"

/*    btmw   */
#include "c_bt_mw_gap.h"
#include "mtk_bt_service_gap_wrapper.h"
#include "mtk_bt_service_a2dp_wrapper.h"
#include "mtk_bt_service_avrcp_wrapper.h"
#include "mtk_bt_service_hidh_wrapper.h"
#include "mtk_bt_service_hfclient_wrapper.h"


/* application level */
#include "u_amb.h"
#include "u_am.h"
#include "u_sm.h"

/* private */
#include "u_bluetooth_audio.h"
#include "bluetooth_audio_proc.h"
#include "bluetooth_audio.h"
#include "bluetooth_hfp.h"
#include "bluetooth_hfp_proc.h"

/*-----------------------------------------------------------------------------
 * structure, constants, macro definitions
 *---------------------------------------------------------------------------*/


/*-----------------------------------------------------------------------------
 * variable declarations
 *---------------------------------------------------------------------------*/
extern BT_HFP_CBK_STATUS_T g_t_bt_hfp_cbk_sta;
extern INT32 g_i_cur_call;


/*-----------------------------------------------------------------------------
 * private methods declarations
 *---------------------------------------------------------------------------*/

//=====================================================================

/*-----------------------------------------------------------------------------
 * Name: _bt_hfp_cli_get_dbg_level
 *
 * Description: This API gets the current debug level.
 *
 * Inputs:  i4_argc         Contains the argument count.
 *          pps_argv        Contains the arguments.
 *
 * Outputs: -
 *
 * Returns: CLIR_OK         Routine successful.
 ----------------------------------------------------------------------------*/
static INT32 _bt_hfp_cli_get_dbg_level (INT32 i4_argc, const CHAR** pps_argv)
{
    INT32  i4_ret;

    i4_ret = u_cli_show_dbg_level(_bt_hfp_get_dbg_level());

    return i4_ret;
}
/*-----------------------------------------------------------------------------
 * Name: _bt_hfp_cli_set_dbg_level
 *
 * Description: This API sets the debug level.
 *
 * Inputs:  i4_argc         Contains the argument count.
 *          pps_argv        Contains the arguments.
 *
 * Outputs: -
 *
 * Returns: CLIR_OK         Routine successful.
 ----------------------------------------------------------------------------*/
static INT32 _bt_hfp_cli_set_dbg_level (INT32 i4_argc, const CHAR** pps_argv)
{
    UINT16 ui2_dbg_level;
    INT32  i4_ret;

    i4_ret = u_cli_parse_dbg_level(i4_argc, pps_argv, &ui2_dbg_level);

    if (i4_ret == CLIR_OK){
        _bt_hfp_set_dbg_level(ui2_dbg_level);
    }

    return i4_ret;
}
#if CONFIG_SUPPORT_BT_HFP
//---------------------------------------------------------------------------------------

static INT32 _bt_hfp_cli_connect (INT32 i4_argc, const CHAR** pps_argv)
{
    INT32  i4_ret;

    if (i4_argc != 2)
    {
        DBG_ERROR((BTHFP_TAG"Err: invalid param!\r\n"));
        return CLIR_INV_ARG;
    }

    i4_ret = a_mtkapi_bt_hfclient_connect(pps_argv[1]);
	DBG_ERROR((BTHFP_TAG"a_mtkapi_bt_hfclient_connect %s \r\n",pps_argv[1]));
    BT_HFP_CHK_FAIL_RET(a_mtkapi_bt_hfclient_connect, i4_ret, BT_HFP_FAIL);

    return CLIR_OK;
}

static INT32 _bt_hfp_cli_disconnect (INT32 i4_argc, const CHAR** pps_argv)
{
    INT32  i4_ret;

    if (i4_argc != 2)
    {
        DBG_ERROR((BTHFP_TAG"Err: invalid param!\r\n"));
        return CLIR_INV_ARG;
    }

    i4_ret = a_mtkapi_bt_hfclient_disconnect(pps_argv[1]);
	DBG_ERROR((BTHFP_TAG"a_mtkapi_bt_hfclient_disconnect %s \r\n",pps_argv[1]));
    BT_HFP_CHK_FAIL_RET(a_mtkapi_bt_hfclient_disconnect, i4_ret, BT_HFP_FAIL);

    return CLIR_OK;
}

static INT32 _bt_hfp_cli_connect_audio (INT32 i4_argc, const CHAR** pps_argv)
{
    INT32  i4_ret;

    if (i4_argc != 2)
    {
        DBG_ERROR((BTHFP_TAG"Err: invalid param!\r\n"));
        return CLIR_INV_ARG;
    }

    i4_ret = a_mtkapi_bt_hfclient_connect_audio(pps_argv[1]);
	DBG_ERROR((BTHFP_TAG"a_mtkapi_bt_hfclient_connect_audio %s \r\n",pps_argv[1]));
    BT_HFP_CHK_FAIL_RET(a_mtkapi_bt_hfclient_connect_audio, i4_ret, BT_HFP_FAIL);

    return CLIR_OK;
}

static INT32 _bt_hfp_cli_disconnect_audio (INT32 i4_argc, const CHAR** pps_argv)
{
    INT32  i4_ret;

    if (i4_argc != 2)
    {
        DBG_ERROR((BTHFP_TAG"Err: invalid param!\r\n"));
        return CLIR_INV_ARG;
    }

    i4_ret = a_mtkapi_bt_hfclient_disconnect_audio(pps_argv[1]);
	DBG_ERROR((BTHFP_TAG"a_mtkapi_bt_hfclient_disconnect_audio \r\n",pps_argv[1]));
    BT_HFP_CHK_FAIL_RET(a_mtkapi_bt_hfclient_disconnect_audio, i4_ret, BT_HFP_FAIL);

    return CLIR_OK;
}

static INT32 _bt_hfp_cli_start_voice_recognition (INT32 i4_argc, const CHAR** pps_argv)    //5
{
    INT32  i4_ret;

    i4_ret = a_mtkapi_bt_hfclient_start_voice_recognition();
	DBG_ERROR((BTHFP_TAG"a_mtkapi_bt_hfclient_start_voice_recognition \r\n"));
    BT_HFP_CHK_FAIL_RET(a_mtkapi_bt_hfclient_start_voice_recognition, i4_ret, BT_HFP_FAIL);

    return CLIR_OK;
}

static INT32 _bt_hfp_cli_stop_voice_recognition (INT32 i4_argc, const CHAR** pps_argv)
{
    INT32  i4_ret;

    i4_ret = a_mtkapi_bt_hfclient_stop_voice_recognition();
	DBG_ERROR((BTHFP_TAG"a_mtkapi_bt_hfclient_stop_voice_recognition \r\n"));
    BT_HFP_CHK_FAIL_RET(a_mtkapi_bt_hfclient_stop_voice_recognition, i4_ret, BT_HFP_FAIL);

    return CLIR_OK;
}

static INT32 _bt_hfp_cli_volume_control (INT32 i4_argc, const CHAR** pps_argv)
{
    INT32  i4_ret;
	BT_HFCLIENT_VOLUME_TYPE_T type;
	INT32 volume;

    if (i4_argc != 3)
    {
        DBG_ERROR((BTHFP_TAG"Err: invalid param!\r\n"));
        return CLIR_INV_ARG;
    }

	type = atoi(pps_argv[1]);
	volume = atoi(pps_argv[2]);

	switch(type)
	{
        case 0:
		{
            i4_ret = a_mtkapi_bt_hfclient_volume_control(BT_HFCLIENT_VOLUME_TYPE_SPK,volume);
	        DBG_ERROR((BTHFP_TAG"a_mtkapi_bt_hfclient_volume_control type=%d , volume=%d\r\n"));
            BT_HFP_CHK_FAIL_RET(a_mtkapi_bt_hfclient_volume_control, i4_ret, BT_HFP_FAIL);
	    }
		break;

		case 1:
		{
            i4_ret = a_mtkapi_bt_hfclient_volume_control(BT_HFCLIENT_VOLUME_TYPE_MIC,volume);
	        DBG_ERROR((BTHFP_TAG"a_mtkapi_bt_hfclient_volume_control type=%d , volume=%d\r\n"));
            BT_HFP_CHK_FAIL_RET(a_mtkapi_bt_hfclient_volume_control, i4_ret, BT_HFP_FAIL);
	    }
		break;

		default:
			DBG_ERROR((BTHFP_TAG"Err:invalid TYPE,please input 0 or 1\r\n"));
			break;

	}

    return CLIR_OK;
}

static INT32 _bt_hfp_cli_dail (INT32 i4_argc, const CHAR** pps_argv)
{
    INT32  i4_ret;


    if (1 == i4_argc)
    {
        return a_mtkapi_bt_hfclient_dial(NULL);
    }

    if (i4_argc != 2)
    {
        DBG_ERROR((BTHFP_TAG"Err: invalid param!\r\n"));
        return CLIR_INV_ARG;
    }

    i4_ret = a_mtkapi_bt_hfclient_dial(pps_argv[1]);
	DBG_ERROR((BTHFP_TAG"a_mtkapi_bt_hfclient_dial %s \r\n",pps_argv[1]));
    BT_HFP_CHK_FAIL_RET(a_mtkapi_bt_hfclient_dial, i4_ret, BT_HFP_FAIL);

    return CLIR_OK;
}

static INT32 _bt_hfp_cli_dail_memory (INT32 i4_argc, const CHAR** pps_argv)
{
    INT32  i4_ret;
	INT32 location;

    if (i4_argc != 2)
    {
        DBG_ERROR((BTHFP_TAG"Err: invalid param!\r\n"));
        return CLIR_INV_ARG;
    }

    location = atoi(pps_argv[1]);

    i4_ret = a_mtkapi_bt_hfclient_dial_memory(location);
	DBG_ERROR((BTAUD_TAG"a_mtkapi_bt_hfclient_dial_memory %d \r\n",location));
    BT_HFP_CHK_FAIL_RET(a_mtkapi_bt_hfclient_dial_memory, i4_ret, BT_HFP_FAIL);

    return CLIR_OK;
}

static INT32 _bt_hfp_cli_call_action (INT32 i4_argc, const CHAR** pps_argv)     //10
{
    INT32  i4_ret;
	INT32 idx;
	BT_HFCLIENT_CALL_ACTION_T call_action ;
	UINT32 ui4_i = 0;
    BT_HFP_CALL_ACTION_T at_call[] = {
        {"CHLD_0",   BT_HFCLIENT_CALL_ACTION_CHLD_0},
        {"CHLD_1",   BT_HFCLIENT_CALL_ACTION_CHLD_1},
        {"CHLD_2",   BT_HFCLIENT_CALL_ACTION_CHLD_2},
        {"CHLD_3",   BT_HFCLIENT_CALL_ACTION_CHLD_3},
        {"CHLD_4",   BT_HFCLIENT_CALL_ACTION_CHLD_4},
        {"CHLD_1x",  BT_HFCLIENT_CALL_ACTION_CHLD_1x},
        {"CHLD_2x",  BT_HFCLIENT_CALL_ACTION_CHLD_2x},
        {"ATA",      BT_HFCLIENT_CALL_ACTION_ATA},
        {"CHUP",     BT_HFCLIENT_CALL_ACTION_CHUP},
        {"BTRH_0",   BT_HFCLIENT_CALL_ACTION_BTRH_0},
        {"BTRH_1",   BT_HFCLIENT_CALL_ACTION_BTRH_1},
        {"BTRH_2",   BT_HFCLIENT_CALL_ACTION_BTRH_2},

        {NULL, 0},
    };
    if (i4_argc != 3)
    {
        DBG_ERROR((BTHFP_TAG"Err: invalid param! usage: cmd [action] [idx]\r\n"));
        return CLIR_INV_ARG;
    }

	idx = atoi(pps_argv[2]);

    //match the support call action
    for (ui4_i = 0; at_call[ui4_i].action_str != NULL ;ui4_i++)
    {
        if(strcmp(at_call[ui4_i].action_str, pps_argv[1]) == 0)
        {
            call_action = at_call[ui4_i].ui4_call_action;
            break;
        }
    }

    if (at_call[ui4_i].action_str == NULL)
    {
        DBG_ERROR((BTAUD_TAG"Err: invalid keyval!, valid keyval is:\r\n\t"));
        for (ui4_i = 0; at_call[ui4_i].action_str != NULL ;ui4_i++)
        {
            DBG_ERROR(("%s  ",at_call[ui4_i].action_str));
        }
        DBG_ERROR(("\r\n"));
        return CLIR_INV_ARG;
    }
	else
    {
        i4_ret = a_mtkapi_bt_hfclient_handle_call_action(call_action,idx);
	    DBG_ERROR((BTHFP_TAG"a_mtkapi_bt_hfclient_handle_call_action call_cation=%d , idx=%d \r\n",call_action,idx));
        BT_HFP_CHK_FAIL_RET(a_mtkapi_bt_hfclient_handle_call_action, i4_ret, BT_HFP_FAIL);
	}

    return CLIR_OK;
}

static INT32 _bt_hfp_cli_query_current_calls (INT32 i4_argc, const CHAR** pps_argv)
{
    INT32  i4_ret;

	g_i_cur_call = 0;
	memset(g_t_bt_hfp_cbk_sta.current_calls, 0, sizeof(g_t_bt_hfp_cbk_sta.current_calls));

	i4_ret = a_mtkapi_bt_hfclient_query_current_calls();
	DBG_ERROR((BTHFP_TAG"a_mtkapi_bt_hfclient_query_current_calls \r\n"));
    BT_HFP_CHK_FAIL_RET(a_mtkapi_bt_hfclient_query_current_calls, i4_ret, BT_HFP_FAIL);

    return CLIR_OK;
}

static INT32 _bt_hfp_cli_query_current_operator_name (INT32 i4_argc, const CHAR** pps_argv)
{
    INT32  i4_ret;

    i4_ret = a_mtkapi_bt_hfclient_query_current_operator_name();
	DBG_ERROR((BTHFP_TAG"a_mtkapi_bt_hfclient_query_current_operator_name \r\n"));
    BT_HFP_CHK_FAIL_RET(a_mtkapi_bt_hfclient_query_current_operator_name, i4_ret, BT_HFP_FAIL);

    return CLIR_OK;
}

static INT32 _bt_hfp_cli_retrieve_subscriber_info (INT32 i4_argc, const CHAR** pps_argv)
{
    INT32  i4_ret;

    i4_ret = a_mtkapi_bt_hfclient_retrieve_subscriber_info();
	DBG_ERROR((BTHFP_TAG"a_mtkapi_bt_hfclient_retrieve_subscriber_info \r\n"));
    BT_HFP_CHK_FAIL_RET(a_mtkapi_bt_hfclient_retrieve_subscriber_info, i4_ret, BT_HFP_FAIL);

    return CLIR_OK;
}

static INT32 _bt_hfp_cli_send_dtmf (INT32 i4_argc, const CHAR** pps_argv)
{
    INT32  i4_ret;
	CHAR   code;

    if (i4_argc != 2)
    {
        DBG_ERROR((BTHFP_TAG"Err: invalid param!\r\n"));
        return CLIR_INV_ARG;
    }

    code = atoi(pps_argv[1]);
	code = (CHAR)code;

    i4_ret = a_mtkapi_bt_hfclient_send_dtmf(code);
	DBG_ERROR((BTHFP_TAG"a_mtkapi_bt_hfclient_send_dtmf code=%d \r\n",code));
    BT_HFP_CHK_FAIL_RET(a_mtkapi_bt_hfclient_send_dtmf, i4_ret, BT_HFP_FAIL);

    return CLIR_OK;
}

static INT32 _bt_hfp_cli_request_last_voice_tag_number (INT32 i4_argc, const CHAR** pps_argv)     //15
{
    INT32  i4_ret;

    i4_ret = a_mtkapi_bt_hfclient_request_last_voice_tag_number();
	DBG_ERROR((BTHFP_TAG"a_mtkapi_bt_hfclient_request_last_voice_tag_number \r\n"));
    BT_HFP_CHK_FAIL_RET(a_mtkapi_bt_hfclient_request_last_voice_tag_number, i4_ret, BT_HFP_FAIL);

    return CLIR_OK;
}

static INT32 _bt_hfp_cli_send_at_cmd (INT32 i4_argc, const CHAR** pps_argv)
{
    INT32  i4_ret;
	INT32  cmd;
	INT32  val1;
	INT32  val2;
	const  CHAR *arg;

    if (i4_argc != 5)
    {
        DBG_ERROR((BTHFP_TAG"Err: invalid param!\r\n"));
        return CLIR_INV_ARG;
    }

	cmd = atoi(pps_argv[1]);
	val1 = atoi(pps_argv[2]);
	val2 = atoi(pps_argv[3]);
	arg = pps_argv[4];

    i4_ret = a_mtkapi_bt_hfclient_send_at_cmd(cmd,val1,val2,arg);
	DBG_ERROR((BTHFP_TAG"a_mtkapi_bt_hfclient_send_at_cmd cmd=%d , val1=%d , val2=%d , arg=%s\r\n",
		                                                  cmd,val1,val2,arg));
    BT_HFP_CHK_FAIL_RET(a_mtkapi_bt_hfclient_send_at_cmd, i4_ret, BT_HFP_FAIL);

    return CLIR_OK;
}

#if 0
static INT32 _bt_hfp_cli_cb_hndlrs (INT32 i4_argc, const CHAR** pps_argv)
{
    INT32  i4_ret;

    i4_ret = c_rpc_reg_mtk_bt_service_hfclient_cb_hndlrs();
	DBG_ERROR((BTHFP_TAG"c_rpc_reg_mtk_bt_service_hfclient_cb_hndlrs \r\n"));
    BT_HFP_CHK_FAIL_RET(c_rpc_reg_mtk_bt_service_hfclient_cb_hndlrs, i4_ret, BT_HFP_FAIL);

    return CLIR_OK;
}
#endif

static INT32 _bt_hfp_cli_hfp_status_info (INT32 i4_argc, const CHAR** pps_argv)
{
    INT32  i4_ret;

    i4_ret = _bt_hfp_cbk_status_print();
	DBG_ERROR((BTHFP_TAG"_bt_hfp_cbk_status_print \r\n"));
    BT_HFP_CHK_FAIL_RET(_bt_hfp_cbk_status_print, i4_ret, BT_HFP_FAIL);

    return CLIR_OK;
}



#endif /*CONFIG_SUPPORT_BT_HFP*/

static INT32 _bt_hfp_test_mic_thread (INT32 i4_argc, const CHAR** pps_argv)
{
    INT32  i4_ret;

	DBG_ERROR((BTHFP_TAG"%s \r\n",__FUNCTION__));
    _mic_hfp_in_thread();
    return CLIR_OK;
}
static INT32 _bt_hfp_test_bt_driver_thread (INT32 i4_argc, const CHAR** pps_argv)
{
    INT32  i4_ret;

	DBG_ERROR((BTHFP_TAG"%s \r\n",__FUNCTION__));
    _bt_driver_in_thread();
    return CLIR_OK;
}

static INT32 _bt_hfp_test_dump_data (INT32 i4_argc, const CHAR** pps_argv)
{
    INT8 cmd = 0;
    if (i4_argc != 2)
    {
        DBG_ERROR((BTHFP_TAG"Err: invalid param!\r\n"));
        return CLIR_INV_ARG;
    }

	cmd = atoi(pps_argv[1]);

    open_hfp_dump(cmd);
    return CLIR_OK;
}
static INT32 _bt_hfp_test_enable (INT32 i4_argc, const CHAR** pps_argv)
{
    _set_test_enable();
    return CLIR_OK;
}

static INT32 test_webrtc_aec_cli(INT32 i4_argc, const CHAR** pps_argv)
{
    //test_webrtc_file_aec();
    test_internal_dac_aec();
    return CLIR_OK;
}
//----------------------------------------------------------------------------------------


/* main command table */
static CLI_EXEC_T at_bt_hfp_cmd_tbl[] =
{
    {
        CLI_GET_DBG_LVL_STR,
        NULL,
        _bt_hfp_cli_get_dbg_level,
        NULL,
        CLI_GET_DBG_LVL_HELP_STR,
        CLI_GUEST
    },
	{
        CLI_SET_DBG_LVL_STR,
        NULL,
        _bt_hfp_cli_set_dbg_level,
        NULL,
        CLI_SET_DBG_LVL_HELP_STR,
        CLI_GUEST
    },

#if CONFIG_SUPPORT_BT_HFP
	{
		"connect",
		"cnt",
		_bt_hfp_cli_connect,
		NULL,
		"hfp connect",
		CLI_GUEST
	},
	{
		"disconnect",
		"discnt",
		_bt_hfp_cli_disconnect,
		NULL,
		"hfp disconnect",
		CLI_GUEST
	},
	{
		"connect_audio",
		"cnt_aud",
		_bt_hfp_cli_connect_audio,
		NULL,
		"hfp connect to audio",
		CLI_GUEST
	},
	{
		"disconnect_audio",
		"discnt_aud",
		_bt_hfp_cli_disconnect_audio,
		NULL,
		"hfp disconnect to audio",
		CLI_GUEST
	},
	{
		"start_vr",          //5
		"start_rcn",
		_bt_hfp_cli_start_voice_recognition,
		NULL,
		"hfp start voice recognition",
		CLI_GUEST
	},
	{
		"stop_vr",
		"stop_rcn",
		_bt_hfp_cli_stop_voice_recognition,
		NULL,
		"hfp stop voice recognition",
		CLI_GUEST
	},
	{
		"volume_control",
		"volume",
		_bt_hfp_cli_volume_control,
		NULL,
		"hfp volume control",
		CLI_GUEST
	},
	{
		"dial",
		"dail",
		_bt_hfp_cli_dail,
		NULL,
		"hfp dail",
		CLI_GUEST
	},
	{
		"dial_memory",
		"memory",
		_bt_hfp_cli_dail_memory,
		NULL,
		"hfp dail memory",
		CLI_GUEST
	},
	{
		"call_action",           //10
		"action",
		_bt_hfp_cli_call_action,
		NULL,
		"hfp call action",
		CLI_GUEST
	},
	{
		"query_calls",
		"query_calls",
		_bt_hfp_cli_query_current_calls,
		NULL,
		"query current calls",
		CLI_GUEST
	},
	{
		"query_operator",
		"query_oprt",
		_bt_hfp_cli_query_current_operator_name,
		NULL,
		"hfp query current operator name",
		CLI_GUEST
	},
	{
		"retrieve_subscriber",
		"retrieve",
		_bt_hfp_cli_retrieve_subscriber_info,
		NULL,
		"retrieve subscriber info",
		CLI_GUEST
	},
	{
		"dtmf",
		"dtmf",
		_bt_hfp_cli_send_dtmf,
		NULL,
		"send dtmf",
		CLI_GUEST
	},
	{
		"request_voice_tag",          //15
		"request_voice_tag",
		_bt_hfp_cli_request_last_voice_tag_number,
		NULL,
		"request last voice tag number",
		CLI_GUEST
	},
	{
		"atcmd",
		"at_cmd",
		_bt_hfp_cli_send_at_cmd,
		NULL,
		"send at cmd",
		CLI_GUEST
	},

#if 0
	{
		"cb_hndlrs",          //17
		"cb_hd",
		_bt_hfp_cli_cb_hndlrs,
		NULL,
		"hfp connect",
		CLI_GUEST
	},
#endif
    {
		"info",          //17
		"sta_info",
		_bt_hfp_cli_hfp_status_info,
		NULL,
		"hfp state info",
		CLI_GUEST
	},


#endif /*CONFIG_SUPPORT_BT_HFP*/
    {
		"mic_thread",          //5
		"mic",
		_bt_hfp_test_mic_thread,
		NULL,
		"mic_thread test",
		CLI_GUEST
	},
	{
		"hfp_driver_thread",          //5
		"hfp_driver",
		_bt_hfp_test_bt_driver_thread,
		NULL,
		"hfp_driver_thread test",
		CLI_GUEST
	},
	{
		"dump_data",          //5
		"dump_data",
		_bt_hfp_test_dump_data,
		NULL,
		"dump_data",
		CLI_GUEST
	},
    {
		"enable_test",          //5
		"test",
		_bt_hfp_test_enable,
		NULL,
		"enable_test",
		CLI_GUEST
	},
	{
		"aec_test",          //6
		"aec_test",
		test_webrtc_aec_cli,
		NULL,
		"aec_test",
		CLI_GUEST
	},
 	END_OF_CLI_CMD_TBL
};

/* multiroom test root command table */
static CLI_EXEC_T at_bt_hfp_root_cmd_tbl[] =
{
	{
	    "bt_hfp",
	    "hfp",
	    NULL,
	    at_bt_hfp_cmd_tbl,
	    "bluetooth hfp commands",
	    CLI_GUEST
	},
	END_OF_CLI_CMD_TBL
};
/*-----------------------------------------------------------------------------
 * export methods implementations
 *---------------------------------------------------------------------------*/
INT32 _bt_hfp_cli_attach_cmd_tbl(VOID)
{
    return (u_cli_attach_cmd_tbl(at_bt_hfp_root_cmd_tbl, CLI_CAT_APP, CLI_GRP_GUI));
}


