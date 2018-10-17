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

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include<unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

/* application level */
#include "c_bt_mw_hidh.h"
#include "u_appman.h"
#include "u_aee.h"
#include "u_cli.h"
#include "u_app_thread.h"
#include "u_dbg.h"
#include "u_os.h"
#include "u_amb.h"
#include "u_am.h"
#include "u_sm.h"
#include "u_assistant_stub.h"

/*    btmw   */
#include "c_bt_mw_gap.h"
#include "mtk_bt_service_gap_wrapper.h"
#include "mtk_bt_service_a2dp_wrapper.h"
#include "mtk_bt_service_avrcp_wrapper.h"
#include "mtk_bt_service_hidh_wrapper.h"

/* private */
#include "u_bluetooth_audio.h"
#include "bluetooth_audio_proc.h"
#include "bluetooth_audio.h"
#include "bluetooth_audio_a2dp.h"
#include "bluetooth_audio_key_proc.h"



/*-----------------------------------------------------------------------------
 * variable declarations
 *---------------------------------------------------------------------------*/
extern BT_A2DP_ROLE g_cur_a2dp_role;
extern UINT8 g_cur_gap_scan_mode;


 
/*-----------------------------------------------------------------------------
 * extern methods declarations
 *---------------------------------------------------------------------------*/
extern INT32 _bluetooth_bt_close(BOOL b_keep_connect);

//=====================================================================


/*-----------------------------------------------------------------------------
 * Name: _bluetooth_cli_get_dbg_level
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
static INT32 _bluetooth_cli_get_dbg_level (INT32 i4_argc, const CHAR** pps_argv)
{
    INT32  i4_ret;

	DBG_ERROR((BTAUD_TAG BTCLI_TAG"_bluetooth_cli_get_dbg_level\r\n"));

    i4_ret = u_cli_show_dbg_level(_bluetooth_get_dbg_level());

    return i4_ret;
}
/*-----------------------------------------------------------------------------
 * Name: _bluetooth_cli_set_dbg_level
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
static INT32 _bluetooth_cli_set_dbg_level (INT32 i4_argc, const CHAR** pps_argv)
{
    UINT16 ui2_dbg_level;
    INT32  i4_ret;

	DBG_ERROR((BTAUD_TAG BTCLI_TAG"_bluetooth_cli_set_dbg_level\r\n"));

    i4_ret = u_cli_parse_dbg_level(i4_argc, pps_argv, &ui2_dbg_level);

    if (i4_ret == CLIR_OK){
        _bluetooth_set_dbg_level(ui2_dbg_level);
    }

    return i4_ret;
}

INT32 _bluetooth_send_key_msg(UINT32 ui4_msgtype,
                                            UINT32 ui4_keysta,
                                            UINT32 ui4_keyval)
{
    INT32  i4_ret;
    BT_AUD_MSG_T t_msg;

	t_msg.ui4_msg_type = BT_AUD_PTE_MSG_KEY;
    t_msg.ui4_msg_id = ui4_msgtype;
    t_msg.ui4_data1  = ui4_keysta;
    t_msg.ui4_data3  = ui4_keyval;
    t_msg.ui4_data2  = 0;
	
    i4_ret = _bluetooth_send_msg_to_itself(E_APP_MSG_TYPE_BLUETOOTH,&t_msg);
    BT_AUD_CHK_FAIL_RET(_bluetooth_send_msg_to_itself, i4_ret, BT_AUD_FAIL);
    return BT_AUD_OK;
}

static INT32 _bluetooth_cli_key_simulator (INT32 i4_argc, const CHAR** pps_argv)
{
    INT32 i4_ret;
    UINT32 ui4_keymsg = 0, ui4_i = 0;
    BT_AUD_KEY2MSG_T at_key[] = {
        {"PLAY",        BT_AUD_KEY_PLAY},
        {"PAUSE",       BT_AUD_KEY_PAUSE},
        {"STOP",        BT_AUD_KEY_STOP},
        {"NEXT",        BT_AUD_KEY_NEXT},
        {"PREV",        BT_AUD_KEY_PREV},

        {NULL, 0},
    };

	DBG_ERROR((BTAUD_TAG BTCLI_TAG"_bluetooth_cli_key_simulator\r\n"));

    if (i4_argc != 2)
    {
        DBG_ERROR((BTAUD_TAG BTCLI_TAG"Err: invalid param!usage: cmd [key]\r\n"));
        return CLIR_INV_ARG;
    }
    //match the support key
    for (ui4_i = 0; at_key[ui4_i].pc_keystr != NULL ;ui4_i++)
    {
        if(strcmp(at_key[ui4_i].pc_keystr, pps_argv[1]) == 0)
        {
            ui4_keymsg = at_key[ui4_i].ui4_keymsg;
            break;
        }
    }

    if (at_key[ui4_i].pc_keystr == NULL)
    {
        DBG_ERROR((BTAUD_TAG"Err: invalid keyval!, valid keyval is:\r\n\t"));
        for (ui4_i = 0; at_key[ui4_i].pc_keystr != NULL ;ui4_i++)
        {
            DBG_ERROR((BTCLI_TAG"%s  ",at_key[ui4_i].pc_keystr));
        }
        DBG_ERROR(("\r\n"));
        return CLIR_INV_ARG;
    }

    DBG_API((BTAUD_TAG BTCLI_TAG"%s virtual key press.\r\n", at_key[ui4_i].pc_keystr));

#if 0
    switch(ui4_keymsg)
    {
        case BT_AUD_KEY_SRC_SWITCH:
        case BT_AUD_KEY_FORCED_PAIRING:
            //BTkey down
            i4_ret = _bluetooth_send_key_msg(BT_AUD_MSG_KEY_BTKEY,
                                            BT_AUD_KEYSTA_DOWN,
                                            BT_AUD_KEY_BLUETOOTH);
            BT_AUD_CHK_FAIL_RET(_bluetooth_send_key_msg, i4_ret, CLIR_CMD_EXEC_ERROR);

            if (ui4_keymsg == BT_AUD_KEY_FORCED_PAIRING)
            {
                //BTkey repeat
                i4_ret = _bluetooth_send_key_msg(BT_AUD_MSG_KEY_BTKEY,
                                                BT_AUD_KEYSTA_REPEAT,
                                                BT_AUD_KEY_BLUETOOTH);
                BT_AUD_CHK_FAIL_RET(_bluetooth_send_key_msg, i4_ret, CLIR_CMD_EXEC_ERROR);
            }

            //BTkey up
            i4_ret = _bluetooth_send_key_msg(BT_AUD_MSG_KEY_BTKEY,
                                            BT_AUD_KEYSTA_UP,
                                            BT_AUD_KEY_BLUETOOTH);
            BT_AUD_CHK_FAIL_RET(_bluetooth_send_key_msg, i4_ret, CLIR_CMD_EXEC_ERROR);
            break;

        case BT_AUD_KEY_VOL_DOWN:
        case BT_AUD_KEY_VOL_UP:
            i4_ret = _bluetooth_send_key_msg(BT_AUD_MSG_KEY_PLAY_CTRL,
                                            BT_AUD_KEYSTA_DOWN,
                                            ui4_keymsg);
            BT_AUD_CHK_FAIL_RET(_bluetooth_send_key_msg, i4_ret, CLIR_CMD_EXEC_ERROR);

            i4_ret = _bluetooth_send_key_msg(BT_AUD_MSG_KEY_PLAY_CTRL,
                                            BT_AUD_KEYSTA_UP,
                                            ui4_keymsg);
            BT_AUD_CHK_FAIL_RET(_bluetooth_send_key_msg, i4_ret, CLIR_CMD_EXEC_ERROR);
            break;

        default:
				break;
   }
#endif 

	i4_ret = _bluetooth_send_key_msg(BT_AUD_MSG_KEY_PLAY_CTRL,
									BT_AUD_KEYSTA_DOWN,
									ui4_keymsg);
	BT_AUD_CHK_FAIL_RET(_bluetooth_send_key_msg, i4_ret, CLIR_CMD_EXEC_ERROR);

    return CLIR_OK;
}

static INT32 _bluetooth_cli_autoconnect (INT32 i4_argc, const CHAR** pps_argv)
{
    INT32  i4_ret;
	BT_AUD_FUNC_ENTRY();

    if (i4_argc != 3)
    {
        DBG_ERROR((BTAUD_TAG"Err: invalid param!usage: connect [0:src_mode | 1:sink_mode] xxxxxx\r\n"));
        return CLIR_INV_ARG;
    }
	
	DBG_ERROR((BTAUD_TAG"input remote A2DP device type: %s\r\n",pps_argv[1]));
	if (0 == strcmp("sink", pps_argv[1]))
	{
		g_cur_a2dp_role = BT_A2DP_ROLE_SRC;
	}else if (0 == strcmp("src", pps_argv[1]))
	{
		g_cur_a2dp_role = BT_A2DP_ROLE_SINK;
	}else
	{
		DBG_ERROR((BTAUD_TAG"Err: invalid role type!, valid role type is:sink src\r\n\t"));
	}

	if (_bluetooth_is_bt_profile_connected())
	{
		DBG_ERROR((BTAUD_TAG"BT is connected!\r\n"));
		i4_ret = _bluetooth_bt_disconnect(FALSE);
		BT_AUD_CHK_FAIL_RET(_bluetooth_bt_disconnect, i4_ret, BT_AUD_FAIL);
	}

    i4_ret = _bluetooth_connect(pps_argv[2]);
    BT_AUD_CHK_FAIL_RET(_bluetooth_connect, i4_ret, BT_AUD_FAIL);

	BT_AUD_FUNC_EXIT();
	return CLIR_OK;
}


static INT32 _bluetooth_cli_bt_init (INT32 i4_argc, const CHAR** pps_argv)
{
    INT32  i4_ret;

    #if 0
	if (i4_argc != 2)
    {
        DBG_ERROR((BTAUD_TAG"Err: invalid param!\r\n"));
        return CLIR_INV_ARG;
    }
    #endif
    i4_ret = _bluetooth_bt_init();
    BT_AUD_CHK_FAIL_RET(_bluetooth_bt_init, i4_ret, BT_AUD_FAIL);

    return CLIR_OK;
}

/*--------------------------------btmw-------------------------------------------*/

static INT32 _bluetooth_cli_mw_set_level (UINT32 i4_argc, const CHAR** pps_argv)
{
    INT32  i4_ret;
    INT32 layer;
    UINT32 level;
    #if 0
	if (i4_argc != 2)
    {
        DBG_ERROR((BTAUD_TAG"Err: invalid param!\r\n"));
        return CLIR_INV_ARG;
    }
    #endif
	layer = atoi(pps_argv[1]);
    level = atoi(pps_argv[2]);
	i4_ret=a_mtkapi_bt_gap_set_dbg_level(layer,level);
    BT_AUD_CHK_FAIL_RET(a_mtkapi_bt_gap_set_dbg_level, i4_ret, BT_AUD_FAIL);

    return CLIR_OK;
}

static INT32 _bluetooth_cli_mw_info (INT32 i4_argc, const CHAR** pps_argv)
{
    INT32  i4_ret;
    BT_LOCAL_DEV ps_dev_info = {0};

	DBG_ERROR((BTAUD_TAG BTCLI_TAG"_bluetooth_cli_mw_info \r\n"));

    i4_ret = a_mtkapi_bt_gap_get_local_dev_info(&ps_dev_info);
    BT_AUD_CHK_FAIL_RET(a_mtkapi_bt_gap_get_local_dev_info, i4_ret, BT_AUD_FAIL);

	DBG_ERROR((BTAUD_TAG BTCLI_TAG"bdAddr: %s, name: %s, gap_state: %d",
		       ps_dev_info.bdAddr,ps_dev_info.name,
		       ps_dev_info.state));

    return CLIR_OK;
}

static int _bluetooth_cli_mw_hid_connect_int_handler(int i4_argc, const CHAR** pps_argv)
{
    INT32  i4_ret;

	DBG_ERROR((BTAUD_TAG BTCLI_TAG"_bluetooth_cli_mw_hid_connect_int_handler \r\n"));

	if (i4_argc != 2)
    {
        DBG_ERROR((BTAUD_TAG"Err: invalid param!\r\n"));
        return CLIR_INV_ARG;
    }

    i4_ret = a_mtkapi_hidh_connect(pps_argv[1]);
    BT_AUD_CHK_FAIL_RET(a_mtkapi_hidh_connect, i4_ret, BT_AUD_FAIL);

	return CLIR_OK;
}
static int _bluetooth_mw_hid_disconnect_handler(int i4_argc, const CHAR** pps_argv)
{
    INT32  i4_ret;

	DBG_ERROR((BTAUD_TAG BTCLI_TAG"_bluetooth_mw_hid_disconnect_handler \r\n"));

    if (i4_argc != 2)
    {
        DBG_ERROR((BTAUD_TAG"Err: invalid param!\r\n"));
        return CLIR_INV_ARG;
    }

    i4_ret = a_mtkapi_hidh_disconnect(pps_argv[1]);
    BT_AUD_CHK_FAIL_RET(a_mtkapi_hidh_disconnect, i4_ret, BT_AUD_FAIL);

    return CLIR_OK;
}

//send hci cmd
static int _bluetooth_cli_mw_gap_send_hci(int i4_argc, const CHAR** pps_argv)
{
    INT32  i4_ret;
	CHAR *hci_cmd;

	BT_AUD_FUNC_ENTRY();

	if (i4_argc != 2)
    {
        DBG_ERROR((BTAUD_TAG"Err: invalid param!\r\n"));
        return CLIR_INV_ARG;
    }

	hci_cmd = (CHAR *)pps_argv[1];
    a_mtkapi_bt_gap_send_hci(hci_cmd);

	BT_AUD_FUNC_EXIT();
	return CLIR_OK;
}

/*----------------------------end btmw------------------------------*/



static INT32 _bluetooth_cli_set_con_dis (INT32 i4_argc, const CHAR** pps_argv)
{
    INT32  i4_ret;
	BOOL fg_conn;
	BOOL fg_disc;

	DBG_ERROR((BTCLI_TAG"_bluetooth_cli_set_con_dis.\r\n"));

	fg_conn=atoi(pps_argv[1]);
	fg_disc=atoi(pps_argv[2]);
    i4_ret = a_mtkapi_bt_gap_set_connectable_and_discoverable(fg_conn,fg_disc);
    BT_AUD_CHK_FAIL_RET(a_mtkapi_bt_gap_set_connectable_and_discoverable, i4_ret, BT_AUD_FAIL);

	if ((1== fg_conn) && (1 == fg_disc))
	{
		g_cur_gap_scan_mode = 2;
	}
	else if (((1 == fg_conn) && (0 == fg_disc)) ||
		((0 == fg_conn) && (1 == fg_disc)))
	{
		g_cur_gap_scan_mode = 1;
	}
	else if ((0 == fg_conn) && (0 == fg_disc))
	{
		g_cur_gap_scan_mode =0;
	}

	DBG_ERROR((BTCLI_TAG"g_cur_gap_scan_mode = %d\r\n",g_cur_gap_scan_mode));

    return CLIR_OK;
}

extern VOID u_ui_set_bt_paring_flag(BOOL flag);

static INT32 _bluetooth_cli_enter_pairing (INT32 i4_argc, const CHAR** pps_argv)
{
	INT32 i4_ret = 0;
    APPMSG_T t_msg = {0};

	DBG_ERROR((BTCLI_TAG"_bluetooth_cli_enter_pairing.\r\n"));

    t_msg.ui4_sender_id = MSG_FROM_SM;
    t_msg.ui4_msg_type = SM_MAKE_MSG(SM_REQUEST_GRP, SM_BODY_BT_PAIRING, 0);
    _ui_send_msg(t_msg.ui4_sender_id, t_msg.ui4_msg_type, BLUETOOTH_THREAD_NAME);

    u_ui_set_bt_paring_flag(TRUE);

	return CLIR_OK;
}

static INT32 _bluetooth_cli_stop_bt (INT32 i4_argc, const CHAR** pps_argv)
{
	INT32 i4_ret = BT_AUD_FAIL;
    HANDLE_T h_app = NULL_HANDLE;

//    t_msg.ui4_sender_id = MSG_FROM_BT;
	DBG_ERROR((BTCLI_TAG"_bluetooth_cli_stop_bt.\r\n"));

    u_am_get_app_handle_from_name(&h_app,BLUETOOTH_THREAD_NAME);

    i4_ret = u_app_send_appmsg(h_app,
                                E_APP_MSG_TYPE_STATE_MNGR,
                                MSG_FROM_BT,
                                SM_MAKE_MSG(SM_REQUEST_GRP,SM_BODY_STOP,0),
                                NULL,
                                0);
    if (AEER_OK != i4_ret) {
        DBG_ERROR((BTAUD_TAG"_bluetooth_send_status_pause_to_sm failed, i4_ret[%ld]\n",i4_ret));
    }

    return (i4_ret == AEER_OK ? BT_AUD_OK : BT_AUD_FAIL);
}

#if 0
static INT32 _bt_aud_cli_switch_to_bt (INT32 i4_argc, const CHAR** pps_argv)
{
    BOOL b_bt_open;
    INT32  i4_ret;
    APPMSG_T t_msg;
    HANDLE_T    h_app;

    if (i4_argc != 2)
    {
        DBG_ERROR((BTAUD_TAG"Err: invalid param!\r\n"));
        return CLIR_INV_ARG;
    }
    //simulate source switch to send the play/stop msg
    t_msg.ui4_sender_id = MSG_FROM_SOURCE_SWITCH;
    b_bt_open = atoi(pps_argv[1]);
    if (b_bt_open == 0)
    {
        DBG_API((BTAUD_TAG"switch input src to another.\r\n"));
        t_msg.ui4_msg_type = PLAY_STOP_REQ;
    }
    else
    {
        DBG_API((BTAUD_TAG"switch input src to BT.\r\n"));
        t_msg.ui4_msg_type = PLAY_START_REQ;
    }


    i4_ret = u_am_get_app_handle_from_name(&h_app, BLUETOOTH_THREAD_NAME);
    BT_AUD_CHK_FAIL_RET(u_am_get_app_handle_from_name, i4_ret, CLIR_CMD_EXEC_ERROR);

    i4_ret = u_app_send_msg(h_app,
                    APP_UTIL_BRDCST_MSG_TO_APP,
                    &t_msg,
                    sizeof(APPMSG_T),
                    NULL,
                    NULL);
    BT_AUD_CHK_FAIL_RET(u_app_send_msg, i4_ret, CLIR_CMD_EXEC_ERROR);

    return CLIR_OK;
}
#endif

static INT32 _bluetooth_cli_get_bt_status (INT32 i4_argc, const CHAR** pps_argv)
{
    const CHAR* ac_onoff[] = {"FALSE","TRUE"};
    const CHAR* ac_btsta[] = {"IDLE","PAIRING","CONNECTED","PLAYING","CONNECTING","DISCONNECTED"};
    BT_AUD_BLUETOOTH_STATUS_T* pt_bt_aud_status;
    pt_bt_aud_status = _bluetooth_get_bt_status();
    CHAR ac_oldname[MAX_NAME_LEN]={0};
    INT32 i4_ret;
	BT_LOCAL_DEV ps_dev_info;
	BT_A2DP_ROLE role;
	CHAR* a2dp_mode[]={"SRC","SINK","UNKNOW"};

	role = _bluetooth_a2dp_get_role();

    i4_ret = a_mtkapi_bt_gap_get_local_dev_info(&ps_dev_info);
    BT_AUD_CHK_FAIL(a_mtkapi_bt_gap_get_local_dev_info, i4_ret);

    DBG_ERROR((BTAUD_TAG BTCLI_TAG"ps_dev_info.name : %s \r\n",ps_dev_info.name));
	strcpy(ac_oldname,ps_dev_info.name);
	DBG_ERROR((BTAUD_TAG BTCLI_TAG"ac_oldname : %s \r\n",ac_oldname));

    DBG_ERROR((BTAUD_TAG BTCLI_TAG "bluetooth audio status:\r\n"));
    DBG_ERROR(("bt_init:%s\r\n"
				"bt_open:%s\r\n"
				"bt_status:%s\r\n"
        		"bt_forced_paring:%s\r\n"
        		"bt_track_idx:%d\r\n"
        		"b_bt_play:%s\r\n"
        		"b_continue_connect:%s\r\n"
        		"b_sticky_pairing_enable:%s\r\n"
        		"b_connectable_background:%s\r\n"
        		"current local A2DP role:%s\r\n"
        		"BT name: %s\r\n",
        		ac_onoff[pt_bt_aud_status->b_bt_init],
        		ac_onoff[pt_bt_aud_status->b_bt_open],
        		ac_btsta[pt_bt_aud_status->e_bt_sta],
        		ac_onoff[pt_bt_aud_status->b_bt_forced_pairing],
        		pt_bt_aud_status->ui4_bt_track_idx,
        		ac_onoff[pt_bt_aud_status->b_bt_play],
        		ac_onoff[pt_bt_aud_status->b_continue_connect],
        		ac_onoff[pt_bt_aud_status->b_sticky_pairing_enable],
        		ac_onoff[pt_bt_aud_status->b_connectable_background],
        		a2dp_mode[role],
        		ac_oldname));

    return CLIR_OK;
}

static INT32 _bluetooth_cli_bt_scan (INT32 i4_argc, const CHAR** pps_argv)
{
    INT32 i4_ret;
    DBG_API((BTAUD_TAG BTCLI_TAG"start bt inquiry scan...\r\n"));

    if (FALSE == _bluetooth_is_bt_power_on())
    {
        DBG_ERROR((BTAUD_TAG"Err: Bluetooth isn't power on.\r\n"));
        return CLIR_CMD_EXEC_ERROR;
    }

    i4_ret = a_mtkapi_bt_gap_start_inquiry_scan(0);
    BT_AUD_CHK_FAIL_RET(a_mtkapi_bt_gap_start_inquiry_scan, i4_ret, CLIR_CMD_EXEC_ERROR);
    return CLIR_OK;
}
static INT32 _bluetooth_cli_bt_get_rssi (INT32 i4_argc, const CHAR** pps_argv)
{
    INT16 i4_rssi;
    INT32 i4_ret;
    CHAR ac_mac[MAX_BDADDR_LEN];
    BT_AUD_BLUETOOTH_STATUS_T* pt_bt_aud_status;

	BT_AUD_FUNC_ENTRY();

    if (i4_argc != 2)
    {
        DBG_ERROR((BTAUD_TAG"Err: invalid param! Usage: cmd [MAC]\r\n"));
        return CLIR_INV_ARG;
    }

    pt_bt_aud_status = _bluetooth_get_bt_status();
    //make sure bt already connected with other device before calling a_mtkapi_bt_gap_get_rssi
    if ((BT_AUD_BT_STA_CONNECTED != pt_bt_aud_status->e_bt_sta)
        && (BT_AUD_BT_STA_PLAYING != pt_bt_aud_status->e_bt_sta))
    {
        DBG_ERROR((BTAUD_TAG"Err: Bluetooth hasn't been connected.\r\n"));
        return CLIR_CMD_EXEC_ERROR;
    }

    strncpy(ac_mac, pps_argv[1], MAX_BDADDR_LEN);
    ac_mac[MAX_BDADDR_LEN - 1] = '\0';

    i4_ret = a_mtkapi_bt_gap_get_rssi(ac_mac, &i4_rssi);
    BT_AUD_CHK_FAIL_RET(a_mtkapi_bt_gap_get_rssi, i4_ret, CLIR_CMD_EXEC_ERROR);

    DBG_ERROR((BTAUD_TAG BTCLI_TAG"Device(%s) RSSI=%d.\r\n",ac_mac, (INT32)i4_rssi));

	BT_AUD_FUNC_EXIT();
    return CLIR_OK;
}

static INT32 _bluetooth_cli_bt_set_name(INT32 i4_argc, const CHAR** pps_argv)
{
    INT32 i4_ret;
	INT32 i4_length = 0;
    CHAR ac_oldname[MAX_NAME_LEN]={0};
    FILE *pf_bt = NULL;
    BT_LOCAL_DEV ps_dev_info;

	BT_AUD_FUNC_ENTRY();

    if (i4_argc != 2)
    {
        DBG_ERROR((BTAUD_TAG"Err: invalid param! Usage: cmd [name]\r\n"));
        return CLIR_INV_ARG;
    }

    if(FALSE == _bluetooth_is_bt_power_on())
    {
        DBG_ERROR((BTAUD_TAG"Err: BT has not power on!\r\n"));
        return BT_AUD_FAIL;
    }

    i4_ret = a_mtkapi_bt_gap_get_local_dev_info(&ps_dev_info);
    BT_AUD_CHK_FAIL(a_mtkapi_bt_gap_get_local_dev_info, i4_ret);

	DBG_ERROR((BTAUD_TAG" ====DBG=====ps_dev_info.name : %s \r\n",ps_dev_info.name));
	strcpy(ac_oldname,ps_dev_info.name);
	DBG_ERROR((BTAUD_TAG" ====DBG=====ac_oldname : %s \r\n",ac_oldname));

    i4_ret = _bluetooth_set_local_name(pps_argv[1]);
    BT_AUD_CHK_FAIL_RET(_bluetooth_set_local_name, i4_ret, CLIR_CMD_EXEC_ERROR);

    DBG_API((BTAUD_TAG"CLI---BT name modified successfully.\r\n"
        "\toldname:%s\r\n"
        "\tnewname:%s\r\n", ac_oldname, pps_argv[1]));

	if (0 == access(BLUETOOTH_NAME_SAVE_PATH, F_OK))  //check the file exist or not
    {
        DBG_ERROR((BTAUD_TAG"CLI---/data/misc/bluetooth_name.txt exist \n"));

		pf_bt = fopen(BLUETOOTH_NAME_SAVE_PATH, "w"); //clear the file
        if (NULL == pf_bt)
        {
            DBG_ERROR((BTAUD_TAG"CLI---Open file /data/misc/bluetooth_name.txt failed!\n"));
            return BT_AUD_FAIL;
        }

        i4_length = fwrite("1", 1, 1, pf_bt);  //write the flag to the file
        DBG_ERROR((BTAUD_TAG"CLI--- FLAG--fwrite length:%d\n",i4_length));

		i4_length = fwrite(pps_argv[1], strlen(pps_argv[1]), 1, pf_bt);  //write the BT nameto the file
        DBG_ERROR((BTAUD_TAG"CLI--- NAME--fwrite length:%d\n",i4_length));

		#if 0   //just for debug
        fseek(pf_bt,0,SEEK_END); //move to the end of the file
		flen = ftell(pf_bt);     //get the size of the file
		if(flen>BLUETOOTH_DEVNAME_MAXSIZE)
		{
            DBG_ERROR((BTAUD_TAG"Invalid name!!! the name's length is longer than BLUETOOTH_DEVNAME_MAXSIZE"));
			return BT_AUD_FAIL;
		}
		fseek(pf_bt,0,SEEK_SET); //move to the begin of the file

		i4_length = fread(ac_name,flen,1,pf_bt);
		DBG_ERROR((BTAUD_TAG"fread length:%d\n",i4_length));
        DBG_ERROR((BTAUD_TAG"fread---the ac_name : %s\r\n", ac_name)

	    #endif

        fclose(pf_bt);

		i4_ret = system("sync");  //sync to Flash
		DBG_ERROR((BTAUD_TAG"sync to Flash"));
        if (-1 == i4_ret)
        {
        DBG_ERROR((BTAUD_TAG"exec sync error(%d)\n",i4_ret));
        //return APP_CFGR_CANT_INIT;
        }
	}
	else     //the file is not exist
	{
       DBG_ERROR((BTAUD_TAG"CLI---no file exist !!!!\n"));
	}
    //end add

	BT_AUD_FUNC_EXIT();
    return CLIR_OK;
}

static INT32 _bluetooth_cli_bt_set_absolute_volume(INT32 i4_argc, const CHAR** pps_argv)
{
    UINT8 ui1_vol;
    INT32 i4_ret;
    BT_AUD_MSG_T t_msg = {0};
    if (i4_argc != 2)
    {
        DBG_ERROR((BTAUD_TAG"Err: invalid param! Usage: cmd [vol]\r\n"));
        return CLIR_INV_ARG;
    }
    ui1_vol = atoi(pps_argv[1]);
    DBG_ERROR((BTAUD_TAG"set absolute volume, volume=%d.\r\n",ui1_vol));


    t_msg.ui4_data1     = ui1_vol;
	
    i4_ret = _bluetooth_send_msg_to_itself(E_APP_MSG_TYPE_BLUETOOTH, &t_msg);
    BT_AUD_CHK_FAIL_RET(_bluetooth_send_msg_to_itself, i4_ret, CLIR_CMD_EXEC_ERROR);

    return CLIR_OK;
}

static INT32 _bluetooth_cli_reset_bt_data(INT32 i4_argc, const CHAR** pps_argv)
{
    INT32 i4_ret;

	DBG_ERROR((BTAUD_TAG BTCLI_TAG"_bluetooth_cli_reset_bt_data.\r\n"));

    i4_ret = u_bluetooth_clear_bluetooth_data();
    BT_AUD_CHK_FAIL_RET(u_bluetooth_clear_bluetooth_data, i4_ret, CLIR_CMD_EXEC_ERROR);

    DBG_ERROR((BTAUD_TAG BTCLI_TAG"bluetooth user data has been cleared!\r\n"));
    return CLIR_OK;
}

static INT32 _bluetooth_cli_get_bt_mac_addr(INT32 i4_argc, const CHAR** pps_argv)
{
    INT32 i4_ret;
    CHAR ac_bdAddr[MAX_BDADDR_LEN]={0};

	DBG_ERROR((BTAUD_TAG BTCLI_TAG"_bluetooth_cli_get_bt_mac_addr.\r\n"));

    i4_ret = a_bluetooth_get_local_mac_addr(ac_bdAddr);
    BT_AUD_CHK_FAIL_RET(a_bluetooth_get_local_mac_addr, i4_ret, CLIR_CMD_EXEC_ERROR);

    DBG_ERROR((BTAUD_TAG BTCLI_TAG"BT MAC : %s .\r\n", ac_bdAddr));
    return CLIR_OK;
}

static INT32 _bluetooth_cli_power_on_off (INT32 i4_argc, const CHAR** pps_argv)
{
    INT32  i4_ret;
	BOOL b_power_on;

	DBG_ERROR((BTAUD_TAG BTCLI_TAG"_bluetooth_cli_power_on_off. \n"));
	
	if (i4_argc != 2)
    {
        DBG_ERROR((BTAUD_TAG"Err: invalid param!\r\n"));
        return CLIR_INV_ARG;
    }
	b_power_on = atoi(pps_argv[1]);
	if(b_power_on)
	{
        _bluetooth_set_bt_power(TRUE);  //4
        DBG_ERROR((BTAUD_TAG BTCLI_TAG" set bt power on. \n"));
	}
    else
    {

        i4_ret = _bluetooth_bt_close(TRUE);
	    BT_AUD_CHK_FAIL_RET(_bluetooth_bt_close, i4_ret, BT_AUD_FAIL);

        _bluetooth_set_bt_power(FALSE);  //4
        DBG_ERROR((BTAUD_TAG BTCLI_TAG" set bt power off. \n"));
    }
    return CLIR_OK;
}

#if ((!CONFIG_APP_SUPPORT_A2DP_SINK_SRC_BOTH) && CONFIG_APP_SUPPORT_BT_SRC_MODE)
static INT32 _bluetooth_cli_reset_a2dp_mode(INT32 argc, const CHAR** argv)
{
    INT32 i4_ret;
    UINT8 u1_enable = 0;
    BT_A2DP_ROLE role;

	BT_AUD_FUNC_ENTRY();

    _bluetooth_a2dp_get_role();

    if (argc != 2)
    {
        DBG_ERROR((BTCLI_TAG"[USERGUIDE] setmode [1:src | 0:sink]"));
        return -1;
    }

    u1_enable = atoi( argv[1]);
    if (1 == u1_enable)
    {
		//make bluetooth stay in no discoverable,no pairable,no connectable before switch role
	    i4_ret = a_mtkapi_bt_gap_set_connectable_and_discoverable(FALSE, FALSE);
		DBG_ERROR((BTAUD_TAG BTCLI_TAG"set BT no connectable and no discoverable.\r\n"));
	    BT_AUD_CHK_FAIL(a_mtkapi_bt_gap_set_connectable_and_discoverable(FALSE, FALSE), i4_ret);

		g_cur_gap_scan_mode = 0;

		i4_ret = _bluetooth_switch_to_source_mode_proc();
		BT_AUD_CHK_FAIL(_bluetooth_switch_to_sink_mode_proc, i4_ret);
    }

    else if (0 == u1_enable)
    {
		//make bluetooth stay in no discoverable,no pairable,no connectable before switch role
	    i4_ret = a_mtkapi_bt_gap_set_connectable_and_discoverable(FALSE, FALSE);
		DBG_ERROR((BTAUD_TAG"set BT non-connectable and non-discoverable.\r\n"));
	    BT_AUD_CHK_FAIL(a_mtkapi_bt_gap_set_connectable_and_discoverable, i4_ret);

		g_cur_gap_scan_mode = 0;

		i4_ret = _bluetooth_switch_to_sink_mode_proc();
		BT_AUD_CHK_FAIL(_bluetooth_switch_to_sink_mode_proc, i4_ret);
    }
    else
    {
        DBG_ERROR((BTCLI_TAG"input error\n"));
        DBG_ERROR((BTCLI_TAG"please input param: [1:src | 0:sink]\n"));
    }

    role = _bluetooth_a2dp_get_role();
    DBG_ERROR((BTCLI_TAG"role swicth successful: %s mode\n", role==BT_A2DP_ROLE_SRC?"src":(role==BT_A2DP_ROLE_SINK?"sink":"unknown")));

	BT_AUD_FUNC_EXIT();
    return CLIR_OK;
}
#endif /*((!CONFIG_APP_SUPPORT_A2DP_SINK_SRC_BOTH) && CONFIG_APP_SUPPORT_BT_SRC_MODE)*/
static INT32 _bluetooth_cli_reconnect_assistant(INT32 i4_argc, const CHAR** pps_argv)
{
	INT32 i4_ret;

	DBG_ERROR((BTAUD_TAG BTCLI_TAG"_bluetooth_cli_reconnect_assistant \r\n"));

	i4_ret = _bluetooth_bt_reconnect_assistant();
	BT_AUD_CHK_FAIL_RET(_bluetooth_bt_reconnect_assistant, i4_ret, CLIR_CMD_EXEC_ERROR);

	DBG_ERROR((BTAUD_TAG BTCLI_TAG"ASSISTANT_STUB---bluetooth reconnect!!\r\n"));
	return CLIR_OK;
}

INT32 _bluetooth_cli_get_player_status(INT32 i4_argc, const CHAR** pps_argv)
{
    ASSISTANT_STUB_PLAYER_STATUS_CHANGE_T *bt_player_status;

	DBG_ERROR((BTAUD_TAG BTCLI_TAG"bt_aud_get_cli_player_status \r\n"));

	bt_player_status = u_bluetooth_get_player_status();

	DBG_ERROR((BTAUD_TAG BTCLI_TAG" ====ASSISTANT_STUB=====bt_player_status.command : %s \r\n",bt_player_status->command));
	DBG_ERROR((BTAUD_TAG BTCLI_TAG" ====ASSISTANT_STUB=====bt_player_status.player.volume : %d \r\n",bt_player_status->player.volume));
	DBG_ERROR((BTAUD_TAG BTCLI_TAG" ====ASSISTANT_STUB=====bt_player_status.player.status : %s \r\n",bt_player_status->player.status));
	DBG_ERROR((BTAUD_TAG BTCLI_TAG" ====ASSISTANT_STUB=====bt_player_status.player.source : %s \r\n",bt_player_status->player.source));
	DBG_ERROR((BTAUD_TAG BTCLI_TAG" ====ASSISTANT_STUB=====bt_player_status.player.progress : %d \r\n",bt_player_status->player.progress));

	return CLIR_OK;
}


/* main command table */
static CLI_EXEC_T at_bt_aud_cmd_tbl[] =
{
    {
        CLI_GET_DBG_LVL_STR,
        NULL,
        _bluetooth_cli_get_dbg_level,
        NULL,
        CLI_GET_DBG_LVL_HELP_STR,
        CLI_GUEST//CLI_SUPERVISOR
    },
	{
        CLI_SET_DBG_LVL_STR,
        NULL,
        _bluetooth_cli_set_dbg_level,
        NULL,
        CLI_SET_DBG_LVL_HELP_STR,
        CLI_GUEST//CLI_SUPERVISOR
    },
/*--------------------CLI for btmw---------------------------*/
	{
		"mw_set_lv",
		"mw_lv",
		_bluetooth_cli_mw_set_level,
		NULL,
		"set mw debug level",
		CLI_GUEST//CLI_SUPERVISOR
	},

	{
		"mw_info",
		"mw_info",
		_bluetooth_cli_mw_info,
		NULL,
		"show btmw info",
		CLI_GUEST//CLI_SUPERVISOR
	},

	{
		"mw_hid_connect",
		"hid_con",
		_bluetooth_cli_mw_hid_connect_int_handler,
		NULL,
		"HID connect",
		CLI_GUEST//CLI_SUPERVISOR
	},
	{
		"mw_hid_disconnect",
		"hid_disc",
		_bluetooth_mw_hid_disconnect_handler,
		NULL,
		"HID disconnect",
		CLI_GUEST//CLI_SUPERVISOR
	},
	{
		"mw_send_hci",
		"mw_hci",
		_bluetooth_cli_mw_gap_send_hci,
		NULL,
		"send hci cmd",
		CLI_GUEST//CLI_SUPERVISOR
	},

/*---------------------end	CLI for btmw -------------------------*/

    {
        "keypress",
        "key",
        _bluetooth_cli_key_simulator,
        NULL,
        "key simulator for bluetooth",
        CLI_GUEST
    },

    {
        "set_con_dis",
        "con_dis",
        _bluetooth_cli_set_con_dis,
        NULL,
        "set bt connect and discover",
        CLI_GUEST//CLI_SUPERVISOR
    },

    {
        "autoconnect",
        "connect",
        _bluetooth_cli_autoconnect,
        NULL,
        "make BT auto connect with remote device",
        CLI_GUEST//CLI_SUPERVISOR
    },
    {
        "btinfo",
        "info",
        _bluetooth_cli_get_bt_status,
        NULL,
        "show bt aud status",
        CLI_GUEST//CLI_SUPERVISOR
    },

    {
        "bt_enter_pairing",
        "btpair",
        _bluetooth_cli_enter_pairing,
        NULL,
        "BT enter pairing mode",
        CLI_GUEST//CLI_SUPERVISOR
    },
#if 0
	{
        "btstop",
        "btstop",
        _bluetooth_cli_stop_bt,
        NULL,
        "stop BT src",
        CLI_GUEST
    },

    {
        "btinit",
        "btinit",
        _bluetooth_cli_bt_init,
        NULL,
        "_bt_aud_init",
        CLI_GUEST//CLI_SUPERVISOR
    },
#endif

#if ((!CONFIG_APP_SUPPORT_A2DP_SINK_SRC_BOTH) && CONFIG_APP_SUPPORT_BT_SRC_MODE)
    {
        "setbtmode",
        "setmode",
        _bluetooth_cli_reset_a2dp_mode,
        NULL,
        "reset BT mode [1:src | 0:sink]",
        CLI_GUEST
    },
#endif /*((!CONFIG_APP_SUPPORT_A2DP_SINK_SRC_BOTH) && CONFIG_APP_SUPPORT_BT_SRC_MODE)*/
    {
        "btscan",
        "scan",
        _bluetooth_cli_bt_scan,
        NULL,
        "start bt inquiry scan",
        CLI_GUEST
    },
    {
        "getrssi",
        "getrssi",
        _bluetooth_cli_bt_get_rssi,
        NULL,
        "get device RSSI, cmd [MAC]",
        CLI_GUEST
    },
    {
        "setname",
        "setname",
        _bluetooth_cli_bt_set_name,
        NULL,
        "set BT name, usage: cmd [name]",
        CLI_GUEST
    },
    {
        "setvol",
        "setvol",
        _bluetooth_cli_bt_set_absolute_volume,
        NULL,
        "set absolute volume, usage: cmd [vol]",
        CLI_GUEST//CLI_SUPERVISOR
    },
    {
        "cleardata",
        "clear",
        _bluetooth_cli_reset_bt_data,
        NULL,
        "reset bluetooth user data",
        CLI_GUEST//CLI_SUPERVISOR
    },
    {
        "btmac",
        NULL,
        _bluetooth_cli_get_bt_mac_addr,
        NULL,
        "show bluetooth BD address",
        CLI_GUEST//CLI_SUPERVISOR
    },
    {
        "bt_power_on_off",
        "power",
        _bluetooth_cli_power_on_off,
        NULL,
        "set bt power on/off",
        CLI_GUEST//CLI_SUPERVISOR
    },

    {
        "bt_reconnect",
        "recnt",
        _bluetooth_cli_reconnect_assistant,
        NULL,
        "set bt reconnect",
        CLI_GUEST//CLI_SUPERVISOR
    },

    {
        "bt_get_player_status",
        "player_status",
        _bluetooth_cli_get_player_status,
        NULL,
        "get player status",
        CLI_GUEST//CLI_SUPERVISOR
    },

 	END_OF_CLI_CMD_TBL
};

/* multiroom test root command table */
static CLI_EXEC_T at_bt_aud_root_cmd_tbl[] =
{
	{
	    "btaud",
	    "bt",
	    NULL,
	    at_bt_aud_cmd_tbl,
	    "bluetooth audio commands",
	    CLI_GUEST
	},
	END_OF_CLI_CMD_TBL
};
/*-----------------------------------------------------------------------------
 * export methods implementations
 *---------------------------------------------------------------------------*/
INT32 _bluetooth_cli_attach_cmd_tbl(VOID)
{
    return (u_cli_attach_cmd_tbl(at_bt_aud_root_cmd_tbl, CLI_CAT_APP, CLI_GRP_GUI));
}

#if 0
INT32 _bluetooth_cli_detach_cmd_tbl(VOID)
{
    return (u_cli_detach_cmd_tbl(at_bt_aud_root_cmd_tbl, CLI_CAT_APP, CLI_GRP_GUI));//(x_cli_detach_cmd_tbl(at_bt_aud_root_cmd_tbl, CLI_CAT_APP, CLI_GRP_GUI));  //xinmie
}
#endif
