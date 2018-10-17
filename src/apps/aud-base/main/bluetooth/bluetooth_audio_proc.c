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
#include <stdio.h>
#include<unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <pthread.h>
#include <sys/time.h>

/* application level */
#include "u_amb.h"
#include "u_am.h"
#include "u_acfg.h"
#include "u_timerd.h"
#include "u_assistant_stub.h"
#include "Interface.h"
#include "u_aee.h"
#include "u_cli.h"
#include "u_dbg.h"
#include "u_os.h"
#include "u_appman.h"
#include "u_app_def.h"
#include "u_sm.h"
#include "u_app_thread.h"
#include "u_playback_uri.h"
#if CONFIG_APP_SUPPORT_BT_SRC_MODE
#include "u_playback_uri.h"
#endif /*CONFIG_APP_SUPPORT_BT_SRC_MODE*/

/*    btmw   */
#include "c_bt_mw_a2dp_snk.h"
#include "bt_a2dp_alsa_playback.h"
#include "c_bt_mw_gap.h"
#include "u_bt_mw_common.h"
#include "mtk_bt_service_gap_wrapper.h"
#include "mtk_bt_service_a2dp_wrapper.h"
#include "mtk_bt_service_avrcp_wrapper.h"
#include "mtk_bt_service_hfclient_wrapper.h"
#if CONFIG_APP_SUPPORT_BT_SRC_MODE
#include "u_bt_mw_avrcp.h"
#endif /*CONFIG_APP_SUPPORT_BT_SRC_MODE*/

/* private */
#include "u_bluetooth_audio.h"
#include "bluetooth_audio_proc.h"
#include "bluetooth_audio_key_proc.h"
#include "bluetooth_audio.h"
#include "bluetooth_audio_gap.h"
#include "bluetooth_audio_a2dp.h"
#include "bluetooth_audio_avrcp.h"

#if CONFIG_SUPPORT_BT_HFP
#include "bluetooth_hfp.h"
#include "bluetooth_hfp_proc.h"
#endif /*CONFIG_SUPPORT_BT_HFP*/

#ifdef CONFIG_BLE_SUPPORT
#include "ble.h"
#endif
/*-----------------------------------------------------------------------------
 * structure, constants, macro definitions
 *---------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------
 * variable declarations
 *---------------------------------------------------------------------------*/
BT_AUD_BLUETOOTH_STATUS_T g_t_bt_aud_sta;
BT_A2DP_DEVICE_LIST g_t_bt_pair_dev_list;
BT_TIMER_MSG_E e_bt_sticky_pairing_msg;
BT_TIMER_MSG_E e_bt_enter_pairing_msg;
UINT16 g_sm_parameter_id = 0;
ASSISTANT_STUB_PLAYER_STATUS_CHANGE_T g_t_bt_aud_play_inform = {0};
ASSISTANT_STUB_BLUETOOTH_STATUS_CHANGE_T g_t_bt_aud_bt_status = {0};
#if CONFIG_APP_SUPPORT_BT_SRC_MODE
BOOL bt_src_suppurt_absolute_volume = FALSE;
#endif /*CONFIG_APP_SUPPORT_BT_SRC_MODE*/

extern CHAR g_cur_gap_addr[18];
extern CHAR g_cur_a2dp_addr[18];
extern CHAR g_cur_avrcp_addr[18];
extern CHAR g_cur_hfp_addr[18];
extern BT_AUD_OBJ_T g_t_bt_aud;
extern BT_A2DP_ROLE g_cur_a2dp_role;
extern UINT8 g_cur_gap_scan_mode;
#if CONFIG_APP_SUPPORT_BT_SRC_MODE
extern void* g_mas_handle;
extern BT_SRC_PLAYER_STATUS_T bt_medialnfo;
extern BOOL bt_src_suppurt_absolute_volume ;
extern void* g_mas_handle;
#endif /*CONFIG_APP_SUPPORT_BT_SRC_MODE*/
#if CONFIG_SUPPORT_BT_HFP
extern BT_HFP_STATUS_T g_t_bt_hfp_sta;
extern BT_HFP_OBJ_T g_t_g_bt_hfp;
extern BT_HFP_CBK_STATUS_T g_t_bt_hfp_cbk_sta;
#endif /*CONFIG_SUPPORT_BT_HFP*/


#if CONFIG_APP_SUPPORT_BLE_GATT_CLIENT
static BOOL b_gatt_force_stop = FALSE;
#endif/* CONFIG_APP_SUPPORT_BLE_GATT_CLIENT */


/*-----------------------------------------------------------------------------
 * private methods declarations
 *---------------------------------------------------------------------------*/
INT32 _bluetooth_set_init_name(VOID);


/*-----------------------------------------------------------------------------
 * extern methods declarations
 *---------------------------------------------------------------------------*/
extern INT32 _bluetooth_send_key_msg(UINT32 ui4_msgtype,UINT32 ui4_keysta,UINT32 ui4_keyval);



#if CONFIG_APP_SUPPORT_BT_STICKY_PAIRING
INT32 _bluetooth_sticky_pairing_timer_create(VOID)
{
    INT32 i4_ret;

    //create sticky pairing timer
    if(g_t_bt_aud.t_sticky_pairing_timer.h_timer == NULL_HANDLE)
    {
	    g_t_bt_aud.t_sticky_pairing_timer.e_flags = X_TIMER_FLAG_ONCE;
        g_t_bt_aud.t_sticky_pairing_timer.ui4_delay = STICKY_PAIRING_MAX_TIME;//1min
        e_bt_sticky_pairing_msg = BT_TIMER_MSG_STICKY_PAIRING;

        i4_ret =u_timer_create(&g_t_bt_aud.t_sticky_pairing_timer.h_timer);
		BT_AUD_CHK_FAIL(u_timer_create, i4_ret);
	}

    return BT_AUD_OK;
}
#endif/* CONFIG_APP_SUPPORT_BT_STICKY_PAIRING */

INT32 _bluetooth_enter_pairing_timer_create(VOID)
{
    INT32 i4_ret;

    //enter pairing timer
	if(g_t_bt_aud.t_enter_pairing_timer.h_timer == NULL_HANDLE)
	{
	    g_t_bt_aud.t_enter_pairing_timer.e_flags = X_TIMER_FLAG_ONCE;
        g_t_bt_aud.t_enter_pairing_timer.ui4_delay = BT_AUD_ENTER_PARING_TIMEOUT;//2min
        e_bt_enter_pairing_msg = BT_TIMER_MSG_ENTER_PARING;

        i4_ret = u_timer_create(&g_t_bt_aud.t_enter_pairing_timer.h_timer);
		BT_AUD_CHK_FAIL(u_timer_create, i4_ret);
	}

    return BT_AUD_OK;
}

INT32 _bluetooth_timer_create(VOID)
{
    INT32 i4_ret;

    //create key delay timer
    //bt_aud_key_timer_create();

#if CONFIG_APP_SUPPORT_BT_STICKY_PAIRING
    //create sticky pairing timer
    _bluetooth_sticky_pairing_timer_create();
#endif/* CONFIG_APP_SUPPORT_BT_STICKY_PAIRING */

    //enter pairing timer
	_bluetooth_enter_pairing_timer_create();

    return BT_AUD_OK;
}

INT32 _bluetooth_sticky_pairing_timer_delete(VOID)
{
    INT32 i4_ret;

    //delete key delay timer
	if (g_t_bt_aud.t_sticky_pairing_timer.h_timer != NULL_HANDLE)
    {
        i4_ret = u_timer_delete(g_t_bt_aud.t_sticky_pairing_timer.h_timer);
        BT_AUD_CHK_FAIL(u_timer_delete, i4_ret);
        g_t_bt_aud.t_sticky_pairing_timer.h_timer = NULL_HANDLE;
    }

    return BT_AUD_OK;
}

INT32 _bluetooth_enter_pairing_timer_delete(VOID)
{
    INT32 i4_ret;

    //delete enter pairing timer
    if (g_t_bt_aud.t_enter_pairing_timer.h_timer != NULL_HANDLE)
    {
        i4_ret = u_timer_delete(g_t_bt_aud.t_enter_pairing_timer.h_timer);
        BT_AUD_CHK_FAIL(u_timer_delete, i4_ret);
        g_t_bt_aud.t_enter_pairing_timer.h_timer = NULL_HANDLE;
    }
    return BT_AUD_OK;
}

INT32 _bluetooth_timer_delete(VOID)
{
    INT32 i4_ret;

    //delete key delay timer
    //bt_aud_key_timer_delete();

#if CONFIG_APP_SUPPORT_BT_STICKY_PAIRING
    //delete sticky pairing timer
	_bluetooth_sticky_pairing_timer_delete();
#endif/* CONFIG_APP_SUPPORT_BT_STICKY_PAIRING */


    //delete enter pairing timer
	_bluetooth_enter_pairing_timer_delete();

    return BT_AUD_OK;
}

#if CONFIG_APP_SUPPORT_BT_STICKY_PAIRING
VOID _bluetooth_sticky_pairing_timer_cb(VOID)
{
    DBG_API((BTAUD_TAG"time out, stop sticky pairing.\r\n"));
    g_t_bt_aud_sta.b_sticky_pairing_enable = FALSE;
}

INT32 _bluetooth_start_sticky_pairing_timer(VOID)
{
    INT32 i4_ret;
    //UINT32 ui4_dly;

    if(g_t_bt_aud.t_sticky_pairing_timer.h_timer == NULL_HANDLE)
    {
        DBG_ERROR((BTAUD_TAG"sticky pairing timer hasn't created.\r\n"));
        return BT_AUD_FAIL;
    }

    if(g_t_bt_aud_sta.b_sticky_pairing_enable == FALSE)
    {
        //u_bt_aud_get_custom_sticky_pairing_time(&ui4_dly);

        g_t_bt_aud_sta.b_sticky_pairing_enable = TRUE;
        DBG_API((BTAUD_TAG"sticky pairing timer start.\r\n"));

		i4_ret = u_timer_start(g_t_bt_aud.h_app,
			          &g_t_bt_aud.t_sticky_pairing_timer,
			          (void *)&e_bt_sticky_pairing_msg,
			          sizeof(BT_TIMER_MSG_E));
    }
    else
    {
        DBG_API((BTAUD_TAG"sticky pairing timer already start.\r\n"));
    }

    return BT_AUD_OK;
}

INT32 _bluetooth_stop_sticky_pairing_timer(VOID)
{
    INT32 i4_ret;

    if(g_t_bt_aud.t_sticky_pairing_timer.h_timer == NULL_HANDLE)
    {
        DBG_ERROR((BTAUD_TAG"sticky pairing timer hasn't created.\r\n"));
        return BT_AUD_FAIL;
    }

    if(g_t_bt_aud_sta.b_sticky_pairing_enable == TRUE)
    {
        DBG_API((BTAUD_TAG"sticky pairing timer stop.\r\n"));
        g_t_bt_aud_sta.b_sticky_pairing_enable = FALSE;

        i4_ret = u_timer_stop(g_t_bt_aud.t_sticky_pairing_timer.h_timer);
        BT_AUD_CHK_FAIL_RET(u_timer_stop, i4_ret, BT_AUD_FAIL);
    }
    else
    {
        DBG_API((BTAUD_TAG"sticky pairing timer hasn't started.\r\n"));
    }
    return BT_AUD_OK;
}
#endif/* CONFIG_APP_SUPPORT_BT_STICKY_PAIRING */

VOID _bluetooth_enter_pairing_timer_cb(VOID)
{
    INT32 i4_ret;

//    g_t_bt_aud_sta.b_enter_pairing_time_out = TRUE;
    g_t_bt_aud_sta.b_enter_pairing_enable = FALSE;

    DBG_ERROR((BTAUD_TAG"enter_pairing time out.\r\n"));
    DBG_ERROR((BTAUD_TAG"_bluetooth_is_bt_connected: %d, g_cur_gap_scan_mode = %d.\r\n",
						_bluetooth_is_bt_connected(),g_cur_gap_scan_mode));

    i4_ret = _bluetooth_send_pairing_result_to_sm();
	BT_AUD_CHK_FAIL(_bluetooth_send_pairing_result_to_sm, i4_ret);

    if (FALSE == _bluetooth_is_bt_connected())
    {
        //make bluetooth stay in no discoverable,no pairable,no connectable
        DBG_ERROR((BTAUD_TAG"set BT connectable ,but no discoverable.\r\n"));
        i4_ret = a_mtkapi_bt_gap_set_connectable_and_discoverable(TRUE, FALSE);
        BT_AUD_CHK_FAIL(a_mtkapi_bt_gap_set_connectable_and_discoverable, i4_ret);

		g_cur_gap_scan_mode = 1;
    }
}

static INT32 _bluetooth_start_enter_pairing_timer(VOID)
{
    INT32 i4_ret;

    if(g_t_bt_aud.t_enter_pairing_timer.h_timer == NULL_HANDLE)
    {
        DBG_ERROR((BTAUD_TAG"enter pairing timer hasn't created.\r\n"));
        return BT_AUD_FAIL;
    }

    if(g_t_bt_aud_sta.b_enter_pairing_enable == FALSE)
    {
        g_t_bt_aud_sta.b_enter_pairing_enable = TRUE;
        DBG_API((BTAUD_TAG"enter pairing timer start.\r\n"));

		i4_ret = u_timer_start(g_t_bt_aud.h_app,
			          &g_t_bt_aud.t_enter_pairing_timer,
			          (void *)&e_bt_enter_pairing_msg,
			          sizeof(BT_TIMER_MSG_E));

        BT_AUD_CHK_FAIL_RET(u_timer_start, i4_ret, BT_AUD_FAIL);
    }
    else
    {
        DBG_API((BTAUD_TAG"enter_pairing timer already start.\r\n"));
    }

    return BT_AUD_OK;
}

static INT32 _bluetooth_stop_enter_pairing_timer(VOID)
{
    INT32 i4_ret;

    if(g_t_bt_aud.t_enter_pairing_timer.h_timer == NULL_HANDLE)
    {
        DBG_ERROR((BTAUD_TAG"enter pairing timer hasn't created.\r\n"));
        return BT_AUD_FAIL;
    }

    if(g_t_bt_aud_sta.b_enter_pairing_enable == TRUE)
    {
        DBG_API((BTAUD_TAG"enter pairing timer stop.\r\n"));
        g_t_bt_aud_sta.b_enter_pairing_enable = FALSE;

        i4_ret = u_timer_stop(g_t_bt_aud.t_enter_pairing_timer.h_timer);
        BT_AUD_CHK_FAIL_RET(u_timer_stop, i4_ret, BT_AUD_FAIL);
    }
    else
    {
        DBG_API((BTAUD_TAG"enter pairing timer hasn't started.\r\n"));
    }
    return BT_AUD_OK;
}

static INT32 _bluetooth_make_bt_name(CHAR *ac_name)
{
	INT32 i4_ret = BT_AUD_FAIL;
	CHAR ac_bdAddr[MAX_BDADDR_LEN]={0};
	CHAR bt_mac_suf[MAX_BDADDR_LEN] = {0};

	BT_AUD_FUNC_ENTRY();

	//get local mac address
	i4_ret = a_bluetooth_get_local_mac_addr(ac_bdAddr);
	BT_AUD_CHK_FAIL_RET(a_bluetooth_get_local_mac_addr, i4_ret, BT_AUD_FAIL);
	DBG_ERROR((BTAUD_TAG"BT MAC : %s\r\n", ac_bdAddr));

	//only use the last 5 characters of the bluetooth device mac address
	strcpy(bt_mac_suf,ac_bdAddr+strlen(ac_bdAddr)-BT_NAME_SUF_LEN);
	DBG_ERROR((BTAUD_TAG"bt_mac_suf : %s\r\n", bt_mac_suf));

	strcpy(ac_name,BLUETOOTH_NAME_PRE);
	DBG_ERROR((BTAUD_TAG"PRE ac_name : %s\r\n", ac_name));

	//the bluetooth name: ac_name = BLUETOOTH_NAME_PRE + (bt_mac_suf)
	strcat(ac_name,"(");
	strcat(ac_name,bt_mac_suf);
	strcat(ac_name,")");
	DBG_ERROR((BTAUD_TAG"the whole ac_name : %s\r\n", ac_name));

	BT_AUD_FUNC_EXIT();
	return BT_AUD_OK;
}

static INT32 _bluetooth_save_bt_name_to_file(FILE *pf_bt, CHAR *ac_name_flag, CHAR *ac_name)
{
	INT32 i4_ret = BT_AUD_FAIL;
	INT32 i4_length = 0;

	BT_AUD_FUNC_ENTRY();

    i4_length = fwrite(ac_name_flag, 1, 1, pf_bt);  //write the flag to the file
    DBG_ERROR((BTAUD_TAG"MAC--FLAG--fwrite length:%d\n",i4_length));

    i4_length = fwrite(ac_name, strlen(ac_name), 1, pf_bt);  //write data
	DBG_ERROR((BTAUD_TAG"fwrite length:%d\n",i4_length));

	fclose(pf_bt);

	//Set the file permission
	i4_ret = chmod(BLUETOOTH_NAME_SAVE_PATH, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
	BT_AUD_CHK_FAIL_RET(chmod,i4_ret,BT_AUD_FAIL);

	i4_ret = system("sync");  //sync to Flash
	DBG_ERROR((BTAUD_TAG"sync to Flash"));
    if (-1 == i4_ret)
    {
        DBG_ERROR((BTAUD_TAG"exec sync error(%d)\n",i4_ret));
    }

	BT_AUD_FUNC_EXIT();
	return BT_AUD_OK;
}

static INT32 _bluetooth_read_bt_name_from_file(FILE *pf_bt, CHAR *ac_name_flag, CHAR *ac_name)
{
	INT32 i4_ret = BT_AUD_FAIL;
	INT32 i4_length = 0;
	INT32 flen = 0;

	BT_AUD_FUNC_ENTRY();

    fseek(pf_bt,0,SEEK_END); //move to the end of the file
    flen = ftell(pf_bt);     //get the size of the file
    if(flen>BLUETOOTH_DEVNAME_MAXSIZE)
    {
        DBG_ERROR((BTAUD_TAG"Invalid name!!! the name's length is longer than BLUETOOTH_DEVNAME_MAXSIZE"));
	    return BT_AUD_FAIL;
    }
    fseek(pf_bt,0,SEEK_SET); //move to the begin of the file

    i4_length = fread(ac_name_flag,1,1,pf_bt);
    DBG_ERROR((BTAUD_TAG"ac_name_flag :%s \n",ac_name_flag));

    i4_length = fread(ac_name,flen-1,1,pf_bt);
    DBG_ERROR((BTAUD_TAG"fread length:%d\n",i4_length));
    DBG_ERROR((BTAUD_TAG"fread---the ac_name : %s\r\n",ac_name));

    fclose(pf_bt);

	BT_AUD_FUNC_EXIT();
	return BT_AUD_OK;
}


/*  set bluetooth name and saves the name to the file /data/misc/bluetooth_name.txt.*/
INT32 _bluetooth_set_init_name(VOID)
{
    INT32 i4_ret = BT_AUD_OK;
    CHAR ac_name[BLUETOOTH_DEVNAME_MAXSIZE] = {0};
    FILE *pf_bt = NULL;
    INT32 i4_length = 0;
    CHAR ac_name_cur[BLUETOOTH_DEVNAME_MAXSIZE] = {0};
    CHAR ac_name_flag[2]= {0};    //ac_name_flag[0]="1"/"2"  : named by user/named by mac

    DBG_ERROR((BTAUD_TAG"%s\n",__FUNCTION__));
//    memset(ac_name, 0, sizeof(ac_name));

    if (0 == access(BLUETOOTH_NAME_SAVE_PATH, F_OK))
    {
        DBG_ERROR((BTAUD_TAG"/data/misc/bluetooth_name.txt exist \n"));

		pf_bt = fopen(BLUETOOTH_NAME_SAVE_PATH, "r+");
        if (NULL == pf_bt)
        {
            DBG_ERROR((BTAUD_TAG"Open file /data/misc/bluetooth_name.txt failed!\n"));
            return BT_AUD_FAIL;
        }

		if(EOF == fgetc(pf_bt))   //the file is empty,write data into the file
		{
            DBG_ERROR((BTAUD_TAG"the file is empty!!! \n"));
			DBG_ERROR((BTAUD_TAG"write the BT name to the file now!! \n"));

			fclose(pf_bt);
			pf_bt = fopen(BLUETOOTH_NAME_SAVE_PATH, "w+");
			if (NULL == pf_bt)
			{
				DBG_ERROR((BTAUD_TAG"%s,Create file /data/misc/bluetooth_name.txt failed!\n",__FUNCTION__));
				return APP_CFGR_INTERNAL_ERR;
			}

			_bluetooth_make_bt_name(ac_name);
			_bluetooth_save_bt_name_to_file(pf_bt,"2",ac_name);

		}
		else   //BT name has been saved in the file
		{
			_bluetooth_read_bt_name_from_file(pf_bt,ac_name_flag,ac_name);

            /*add flow for the case of manually modify the MAC*/
            if('1'!=ac_name_flag[0])   //BT name is not modified  by CLI or Assistance-SUTB
            {
                DBG_ERROR((BTAUD_TAG"BT name is not modified  by CLI or Assistance-SUTB,ac_name_flag:%s . \r\n",ac_name_flag));

				_bluetooth_make_bt_name(ac_name_cur);

                //compare the two mac
                i4_ret = strcmp(ac_name,ac_name_cur);
                if(0!=i4_ret)   //not the same mac,then write the new name to the file
                {
                    DBG_ERROR((BTAUD_TAG"MAC was modified. \r\n"));
                    strcpy(ac_name,ac_name_cur);
                    pf_bt = fopen(BLUETOOTH_NAME_SAVE_PATH, "w");  //clear the file

                    if (NULL == pf_bt)
                    {
                        DBG_ERROR((BTAUD_TAG"Open file /data/misc/bluetooth_name.txt failed!\n"));
                        return BT_AUD_FAIL;
                    }

                    fseek(pf_bt,0,SEEK_SET);
					_bluetooth_save_bt_name_to_file(pf_bt,"2",ac_name);
                }
            }
        }
    }
    else /* the file is not exist, creat it and write bluetooth local  name  into. */
    {
        //create file and write data
        pf_bt = fopen(BLUETOOTH_NAME_SAVE_PATH, "w+");
        if (NULL == pf_bt)
        {
            DBG_ERROR((BTAUD_TAG"%s,Create file /data/misc/bluetooth_name.txt failed!\n",__FUNCTION__));
            return APP_CFGR_INTERNAL_ERR;
        }

		_bluetooth_make_bt_name(ac_name);
		_bluetooth_save_bt_name_to_file(pf_bt,"2",ac_name);
    }

//	fclose(pf_bt);

	_bluetooth_set_local_name(ac_name);   //set bluetooth local name

    return BT_AUD_OK;
}


//------------------------------------------------------------------------------

static INT32 _bluetooth_register_callback(VOID)
{
	INT32 i4_ret = BT_AUD_FAIL;
	BT_AUD_FUNC_ENTRY();

#ifndef CONFIG_ADAPTOR_APP_CTRL
	i4_ret = _bluetooth_gap_base_init();
	BT_AUD_CHK_FAIL_RET(_bluetooth_gap_base_init, i4_ret, BT_AUD_FAIL);
#endif
	i4_ret = _bluetooth_a2dp_init();
	BT_AUD_CHK_FAIL_RET(_bluetooth_a2dp_init, i4_ret, BT_AUD_FAIL);

	i4_ret = _bluetooth_avrcp_init();
	BT_AUD_CHK_FAIL_RET(_bluetooth_avrcp_init, i4_ret, BT_AUD_FAIL);
#if CONFIG_SUPPORT_BT_HFP
	i4_ret = _bluetooth_hfp_init();
	BT_AUD_CHK_FAIL_RET(_bluetooth_hfp_init, i4_ret, BT_AUD_FAIL);
#endif /*CONFIG_SUPPORT_BT_HFP*/

	BT_AUD_FUNC_EXIT();
	return BT_AUD_OK;
}

static INT32 _bluetooth_enable_sink_and_src(VOID)
{
	INT32 i4_ret = BT_AUD_FAIL;
	BT_AUD_FUNC_ENTRY();

	DBG_ERROR((BTAUD_TAG"Enable SINK and SRC roles. \n"));

	i4_ret = a_mtkapi_a2dp_sink_enable(TRUE);
	BT_AUD_CHK_FAIL_RET(_bluetooth_gap_base_init, i4_ret, BT_AUD_FAIL);

	i4_ret = a_mtkapi_a2dp_src_enable(TRUE);
	BT_AUD_CHK_FAIL_RET(_bluetooth_a2dp_init, i4_ret, BT_AUD_FAIL);

	BT_AUD_FUNC_EXIT();
	return BT_AUD_OK;
}

INT32 _bluetooth_bt_init(VOID)
{
	INT32 i4_ret = BT_AUD_FAIL;

    BT_AUD_FUNC_ENTRY();

    if(!g_t_bt_aud_sta.b_bt_init)
    {
        DBG_API((BTAUD_TAG"bluetooth start init...\r\n"));

		i4_ret = _bluetooth_register_callback();
		BT_AUD_CHK_FAIL_RET(_bluetooth_register_callback, i4_ret, BT_AUD_FAIL);

        memset(&g_t_bt_aud_sta, 0, sizeof(BT_AUD_BLUETOOTH_STATUS_T));
        g_t_bt_aud_sta.b_bt_init = TRUE;
    }
#if CONFIG_APP_SUPPORT_A2DP_SINK_SRC_BOTH
	i4_ret = _bluetooth_enable_sink_and_src();
	BT_AUD_CHK_FAIL_RET(_bluetooth_enable_sink_and_src, i4_ret, BT_AUD_FAIL);

#else /*CONFIG_APP_SUPPORT_A2DP_SINK_SRC_BOTH*/
	i4_ret = a_mtkapi_a2dp_sink_enable(TRUE);
	DBG_ERROR((BTAUD_TAG" set as the SINK role \n"));
	BT_AUD_CHK_FAIL_RET(a_mtkapi_a2dp_sink_enable, i4_ret, BT_AUD_FAIL);
#endif /*CONFIG_APP_SUPPORT_A2DP_SINK_SRC_BOTH*/

	g_cur_a2dp_role = BT_A2DP_ROLE_SINK;

    if(FALSE == _bluetooth_is_bt_power_on())
    {
		_bluetooth_set_bt_power(TRUE);  //4
		DBG_ERROR((BTAUD_TAG" set bt power on. \n"));

		//set bluetooth local init name
		i4_ret = _bluetooth_set_init_name();    //6
		DBG_ERROR((BTAUD_TAG" _bluetooth_set_init_name \n"));
		BT_AUD_CHK_FAIL_RET(_bluetooth_set_init_name, i4_ret, BT_AUD_FAIL);

#if (CONFIG_APP_SUPPORT_BLE_GATT_CLIENT || CONFIG_APP_SUPPORT_BLE_GATT_SERVER)
		DBG_ERROR(("<BT_GATT> call bt_audio_call_gatt_interface().\r\n"));
		i4_ret = bt_audio_call_gatt_interface();
		BT_AUD_CHK_FAIL(bt_audio_call_gatt_interface, i4_ret);
#endif
		DBG_API((BTAUD_TAG"BLUETOOTH_AUD_OPEN.\r\n"));
		//open bluetooth
		i4_ret = _bluetooth_bt_open();
		BT_AUD_CHK_FAIL_GOTO(_bluetooth_bt_open, i4_ret, err_out);

		i4_ret = _bluetooth_paired_dev_list_update(&g_t_bt_pair_dev_list);
		if ((i4_ret != BT_AUD_OK) || (g_t_bt_pair_dev_list.dev_num == 0))
		{
			//can't get pair dev list or not have pair history then force bt into pairing mode
			g_t_bt_aud_sta.b_bt_forced_pairing = TRUE;
		}

		//handle forced paring
		if (TRUE == g_t_bt_aud_sta.b_bt_forced_pairing)  //can not get history
		{
			g_t_bt_aud_sta.b_bt_forced_pairing = FALSE;

			//make bluetooth stay in no discoverable,no pairable,no connectable after bluetooth power on
	        i4_ret = a_mtkapi_bt_gap_set_connectable_and_discoverable(FALSE, FALSE);
	    	DBG_ERROR((BTAUD_TAG"a_mtkapi_bt_gap_set_connectable_and_discoverable\r\n"));
	        BT_AUD_CHK_FAIL(a_mtkapi_bt_gap_set_connectable_and_discoverable(FALSE, FALSE), i4_ret);

			g_cur_gap_scan_mode = 0;
		}
		else//if get history ,auto connect the last pairing device
		{
	        g_t_bt_aud_sta.b_bt_init_auto_connect = TRUE;  //init auto connect

			i4_ret = _bluetooth_auto_connect(1, TRUE);
			BT_AUD_CHK_FAIL_GOTO(_bluetooth_auto_connect, i4_ret, err_out);
		}
	}
    DBG_ERROR((BTAUD_TAG"_bluetooth_bt_init done. b_bt_init=%d.\r\n", g_t_bt_aud_sta.b_bt_init));

#ifdef CONFIG_BLE_SUPPORT
    {
        DBG_ERROR((BTAUD_TAG"INIT and OPEN BLE SERVER.\r\n"));
        i4_ret = ble_app_send_msg_to_ble(MSG_FROM_BT,NULL,0,
            BLE_TO_BLE_MSG_INIT_AND_OPEN_SVR);
    }
#endif

    BT_AUD_FUNC_EXIT();
    return BT_AUD_OK;

err_out:
    //open bt err then close bt
    DBG_ERROR((BTAUD_TAG"Err: bluetooth start fail!\r\n"));

    i4_ret = _bluetooth_bt_close(FALSE);
    BT_AUD_CHK_FAIL(_bluetooth_bt_close, i4_ret);

    return BT_AUD_FAIL;

}

INT32 _bluetooth_bt_open(VOID)
{
    //INT32 i4_ret;

    BT_AUD_FUNC_ENTRY();

	DBG_API((BTAUD_TAG"bluetooth start open...\r\n"));

    if (g_t_bt_aud_sta.b_bt_init == FALSE)
    {
        DBG_ERROR((BTAUD_TAG"bluetooth not init, init now.\r\n"));
        _bluetooth_bt_init();
    }

    if(TRUE == _bluetooth_is_bt_power_setting_on())
    {
        DBG_INFO((BTAUD_TAG"bluetooth is enable.\r\n"));
    }
    else
    {
        DBG_ERROR((BTAUD_TAG"bluetooth is not enable.\r\n"));
        return BT_AUD_FAIL;
    }
    g_t_bt_aud_sta.b_bt_open = TRUE;

    BT_AUD_FUNC_EXIT();
    return BT_AUD_OK;
}

INT32 _bluetooth_bt_close(BOOL b_keep_connect)
{

    BT_AUD_FUNC_ENTRY();

    //if bt on then close
    if (_bluetooth_is_bt_ready() == TRUE)
    {
        DBG_API((BTAUD_TAG"bluetooth start close...\r\n"));

        g_t_bt_aud_sta.b_bt_open = FALSE;
        g_t_bt_aud_sta.b_bt_forced_pairing = FALSE;
        _bluetooth_bt_disconnect(b_keep_connect);
    }
    else
    {
#if CONFIG_APP_SUPPORT_BT_KEEP_CONNECT
        if ((b_keep_connect == FALSE) && (g_t_bt_aud_sta.b_keep_connect == TRUE))
        {
            _bluetooth_break_link();
        }
#endif/* CONFIG_APP_SUPPORT_BT_KEEP_CONNECT */
        DBG_API((BTAUD_TAG"bluetooth is already close.\r\n"));
    }

#if CONFIG_APP_SUPPORT_BLE_GATT_CLIENT
    //if gatt has been forced close before, restart it now
    if (b_gatt_force_stop == TRUE)
    {
        b_gatt_force_stop = FALSE;
        bluetooth_gattc_scan_proc();
        DBG_API((BTAUD_TAG"Restart GATT scan.\r\n"));
    }
#endif/* CONFIG_APP_SUPPORT_BLE_GATT_CLIENT */

    BT_AUD_FUNC_EXIT();
    return BT_AUD_OK;
}

INT32 _bluetooth_start_pairing(VOID)
{
    INT32 i4_ret;

    BT_AUD_FUNC_ENTRY();

    if (_bluetooth_is_bt_ready() == FALSE)
    {
        DBG_ERROR((BTAUD_TAG"Err: please enbale bluetooth first.\r\n"));
        return BT_AUD_FAIL;
    }

    if (TRUE == _bluetooth_is_bt_profile_connected())
    {
        DBG_ERROR((BTAUD_TAG"Err: bluetooth is connected.\r\n"));
        return BT_AUD_OK;
    }

    DBG_API((BTAUD_TAG"bluetooth start pairing...\r\n"));
    g_t_bt_aud_sta.e_bt_sta = BT_AUD_BT_STA_PAIRING;

    DBG_ERROR((BTAUD_TAG"enter_pairing_enable: %d , g_cur_gap_scan_mode = %d.\r\n",
						g_t_bt_aud_sta.b_enter_pairing_enable,g_cur_gap_scan_mode));

	//don not enter pairing mode while A2DP disconnect in A2DP role switch
	if ((BT_AUD_A2DP_SINK_MODE == g_t_bt_aud_sta.e_bt_a2dp_mode_switch)
		|| (BT_AUD_A2DP_SOURCE_MODE == g_t_bt_aud_sta.e_bt_a2dp_mode_switch))
	{
		DBG_API((BTAUD_TAG"A2DP role switch is going, BT would enter pairing while switch successful.\r\n"));
		return BT_AUD_OK;
	}

    if (FALSE == g_t_bt_aud_sta.b_enter_pairing_enable)
    {
		if (2 == g_cur_gap_scan_mode)
		{
			DBG_API((BTAUD_TAG"BT is already connectable and discoverable.\r\n"));
		}
		else
		{
			//make bluetooth stay in no discoverable,but pairable and connectable
	        i4_ret = a_mtkapi_bt_gap_set_connectable_and_discoverable(TRUE, FALSE);
	        DBG_ERROR((BTAUD_TAG"BT is connectable,but non-discoverable. \r\n"));
	        BT_AUD_CHK_FAIL(a_mtkapi_bt_gap_set_connectable_and_discoverable, i4_ret);

			g_cur_gap_scan_mode = 1;
		}
    }else if (g_t_bt_aud_sta.b_enter_pairing_enable)
    {
		i4_ret = a_mtkapi_bt_gap_set_connectable_and_discoverable(TRUE, TRUE);
        DBG_ERROR((BTAUD_TAG"BT is connectable and discoverable. \r\n"));
        BT_AUD_CHK_FAIL(a_mtkapi_bt_gap_set_connectable_and_discoverable, i4_ret);

		g_cur_gap_scan_mode = 2;
	}

#if CONFIG_APP_SUPPORT_BT_STICKY_PAIRING
    if(g_t_bt_aud_sta.b_sticky_pairing_enable == TRUE)
    {
        DBG_API((BTAUD_TAG"start pairing, stop sticky pairing.\r\n"));
        _bluetooth_stop_sticky_pairing_timer();
    }
#endif/* CONFIG_APP_SUPPORT_BT_STICKY_PAIRING */

#if CONFIG_APP_SUPPORT_BLE_GATT_CLIENT
    //if gatt has been forced close before, restart it now
    if (b_gatt_force_stop == TRUE)
    {
        b_gatt_force_stop = FALSE;
        bluetooth_gattc_scan_proc();
        DBG_API((BTAUD_TAG"Restart GATT scan.\r\n"));
    }
#endif/* CONFIG_APP_SUPPORT_BLE_GATT_CLIENT */

    BT_AUD_FUNC_EXIT();
    return BT_AUD_OK;
}

INT32 _bluetooth_start_playing(VOID)
{
    INT32 i4_ret;

    BT_AUD_FUNC_ENTRY();

    if (_bluetooth_is_bt_ready() == FALSE)
    {
        DBG_ERROR((BTAUD_TAG"Err: please enbale bluetooth first.\r\n"));
        return BT_AUD_FAIL;
    }

    if (g_t_bt_aud_sta.b_bt_a2dp_connect!= TRUE)
    {
        DBG_ERROR((BTAUD_TAG"Err: bluetooth isn't in CONNECTED mode.\r\n"));
        return BT_AUD_FAIL;
    }

    DBG_API((BTAUD_TAG"bluetooth start playing...\r\n"));
    g_t_bt_aud_sta.e_bt_sta = BT_AUD_BT_STA_PLAYING;
    //t_bt_aud_sta.b_bt_play = FALSE;

	i4_ret = a_mtkapi_a2dp_sink_start_player();
	BT_AUD_CHK_FAIL(a_mtkapi_a2dp_sink_start_player, i4_ret);
	g_t_bt_aud_sta.b_bt_open_player = TRUE;

#if CONFIG_APP_SUPPORT_BLE_GATT_CLIENT
    //if gatt has been forced close before, restart it now
    if (b_gatt_force_stop == TRUE)
    {
        b_gatt_force_stop = FALSE;
        bluetooth_gattc_scan_proc();
        DBG_API((BTAUD_TAG"Restart GATT scan.\r\n"));
    }
#endif/* CONFIG_APP_SUPPORT_BLE_GATT_CLIENT */

    BT_AUD_FUNC_EXIT();
    return BT_AUD_OK;
}

INT32 _bluetooth_connect(const CHAR *pc_mac_addr)
{
    INT32 i4_ret;
    CHAR ac_mac[MAX_BDADDR_LEN];
    BT_AUD_FUNC_ENTRY();

    strncpy(ac_mac, pc_mac_addr, MAX_BDADDR_LEN);
    ac_mac[MAX_BDADDR_LEN - 1] = '\0';

    //disable connect and discover
    i4_ret = a_mtkapi_bt_gap_set_connectable_and_discoverable(FALSE, FALSE);
    BT_AUD_CHK_FAIL(a_mtkapi_bt_gap_set_connectable_and_discoverable(FALSE, FALSE), i4_ret);

	g_cur_gap_scan_mode = 0;
    //disable pairing
    //i4_ret = c_btm_set_pairable_sync(FALSE);
    //BT_AUD_CHK_FAIL(c_btm_set_pairable_sync(FALSE), i4_ret);

    if (BT_A2DP_ROLE_SRC == g_cur_a2dp_role)
    {
		DBG_ERROR((BTAUD_TAG"local role is SRC.\r\n"));
		i4_ret = a_mtkapi_a2dp_connect(ac_mac,BT_A2DP_ROLE_SRC);
		BT_AUD_CHK_FAIL_RET(a_mtkapi_a2dp_connect, i4_ret, BT_AUD_FAIL);
	}
	else if (BT_A2DP_ROLE_SINK == g_cur_a2dp_role)
	{
		DBG_ERROR((BTAUD_TAG"local role is SINK.\r\n"));

		DBG_ERROR((BTAUD_TAG"try to connect device(MAC=%s).\r\n", ac_mac));
#if CONFIG_SUPPORT_BT_HFP
    //hfp connect first
    i4_ret = a_mtkapi_bt_hfclient_connect(ac_mac);
	DBG_ERROR((BTAUD_TAG BTHFP_TAG"a_mtkapi_bt_hfclient_connect.\r\n"));
	BT_AUD_CHK_FAIL_RET(a_mtkapi_bt_hfclient_connect, i4_ret, BT_AUD_FAIL);
#endif /*CONFIG_SUPPORT_BT_HFP*/

		i4_ret = a_mtkapi_a2dp_connect(ac_mac,BT_A2DP_ROLE_SINK);
	    BT_AUD_CHK_FAIL_RET(a_mtkapi_a2dp_connect, i4_ret, BT_AUD_FAIL);
	}

    BT_AUD_FUNC_EXIT();
    return BT_AUD_OK;
}

INT32 _bluetooth_auto_connect(UINT32 ui4_devnum, BOOL b_start_init)
{
    INT32 i4_ret;
    static UINT32 ui4_num = 0;

    BT_AUD_FUNC_ENTRY();

	if (ui4_devnum < 1)
	{
        DBG_ERROR((BTAUD_TAG"Err: please input the right ui4_devnum.\r\n"));
        return BT_AUD_FAIL;
	}

    if (_bluetooth_is_bt_ready() == FALSE)
    {
        DBG_ERROR((BTAUD_TAG"Err: please enbale bluetooth first.\r\n"));
        return BT_AUD_FAIL;
    }

    if (TRUE == _bluetooth_is_bt_connected())
    {
        DBG_ERROR((BTAUD_TAG"Err: bluetooth is connected.\r\n"));
        return BT_AUD_FAIL;
    }

#if CONFIG_APP_SUPPORT_BLE_GATT_CLIENT
    //close gatt scan to make better BT performance
    if (b_gatt_force_stop == FALSE)
    {
        b_gatt_force_stop = TRUE;
        bluetooth_gattc_stop_scan_proc();
        DBG_API((BTAUD_TAG"Stop GATT scan.\r\n"));
    }
#endif/* CONFIG_APP_SUPPORT_BLE_GATT_CLIENT */

    DBG_ERROR((BTAUD_TAG"bluetooth start auto connect...\r\n"));
    g_t_bt_aud_sta.e_bt_sta = BT_AUD_BT_STA_AUTO_CONNECTING;
	g_cur_gap_scan_mode = 0;

    if (b_start_init == TRUE)   //init start device num
    {
        ui4_num = ui4_devnum;
    }
    else    //not first time, auto increase device num
    {
        ui4_num++;
    }

    if (ui4_num > g_t_bt_pair_dev_list.dev_num)
    {
        DBG_ERROR((BTAUD_TAG"End of pairedlist. pairnum=%d, maxnum=%d\r\n",
            ui4_num, g_t_bt_pair_dev_list.dev_num));
        return BT_AUD_FAIL;
    }

    DBG_API((BTAUD_TAG"auto connect %dth history paired device, devname=%s.\r\n",
        ui4_num,g_t_bt_pair_dev_list.dev[ui4_num - 1].name));

    i4_ret = _bluetooth_connect(g_t_bt_pair_dev_list.dev[ui4_num - 1].addr);
    BT_AUD_CHK_FAIL_RET(_bluetooth_connect, i4_ret, BT_AUD_FAIL);

    BT_AUD_FUNC_EXIT();
    return BT_AUD_OK;
}

INT32 _bluetooth_bt_disconnect(BOOL b_keep_connect)
{
    INT32 i4_ret;

    BT_AUD_FUNC_ENTRY();

   DBG_ERROR((BTAUD_TAG"bluetooth start disconnect...\r\n"));

    //disable connect and discover
    i4_ret = a_mtkapi_bt_gap_set_connectable_and_discoverable(FALSE, FALSE);
	DBG_ERROR((BTAUD_TAG"a_mtkapi_bt_gap_set_connectable_and_discoverable\r\n"));
    BT_AUD_CHK_FAIL(a_mtkapi_bt_gap_set_connectable_and_discoverable, i4_ret);

	g_cur_gap_scan_mode = 0;
    //disable pairing
    //i4_ret = c_btm_set_pairable_sync(FALSE);
    //BT_AUD_CHK_FAIL(c_btm_set_pairable_sync(FALSE), i4_ret);
#if CONFIG_APP_SUPPORT_BT_KEEP_CONNECT
    if ((TRUE == _bluetooth_is_bt_connected()) && (b_keep_connect == TRUE))
    {
        //already connect now, if keep connect , need to pause SRC device
        DBG_API((BTAUD_TAG"keep connect.\r\n"));
        bt_aud_bluetooth_force_pause();
    }
    else
#endif/* CONFIG_APP_SUPPORT_BT_KEEP_CONNECT */
    {
        //no connect or no need to keep connect
        b_keep_connect = FALSE;
    }

    _bluetooth_clear_connect_info(b_keep_connect);

#if CONFIG_APP_SUPPORT_BT_STICKY_PAIRING
    if(g_t_bt_aud_sta.b_sticky_pairing_enable == TRUE)
    {
        DBG_API((BTAUD_TAG"active disconnect, stop sticky pairing.\r\n"));
        _bluetooth_stop_sticky_pairing_timer();
        g_t_bt_aud_sta.b_sticky_pairing_enable = FALSE;
    }
#endif/* CONFIG_APP_SUPPORT_BT_STICKY_PAIRING */

	i4_ret = _bluetooth_profiles_disconnect();
	BT_AUD_CHK_FAIL_RET(_bluetooth_profiles_disconnect, i4_ret, BT_AUD_FAIL);

    BT_AUD_FUNC_EXIT();
    return BT_AUD_OK;
}

INT32 _bluetooth_break_link(VOID)
{
    INT32 i4_ret;

    BT_AUD_FUNC_ENTRY();

    DBG_API((BTAUD_TAG"bluetooth break old link...\r\n"));

    g_t_bt_aud_sta.b_keep_connect = FALSE;

    i4_ret = a_mtkapi_a2dp_disconnect(g_cur_a2dp_addr);
    BT_AUD_CHK_FAIL_RET(g_cur_a2dp_addr, i4_ret, BT_AUD_FAIL);

    BT_AUD_FUNC_EXIT();
    return BT_AUD_OK;
}

INT32 _bluetooth_acfg_increase_volume(VOID)
{
    UINT8 ui1_volume, bt_absolute_volume;
    INT32 i4_ret;
	BT_A2DP_ROLE role;

    DBG_INFO((BTAUD_TAG"u_acfg_increase_volume\n"));

	_bluetooth_a2dp_get_role();

    i4_ret = u_acfg_set_mute(FALSE);
    if (APP_CFGR_OK != i4_ret)
    {
        return i4_ret;
    }

    i4_ret = u_acfg_get_volume(&ui1_volume);
    if (APP_CFGR_OK != i4_ret)
    {
        return i4_ret;
    }

    ui1_volume += BT_VOLUME_STEP;
    if (ui1_volume > VOLUME_MAX)
    {
        ui1_volume = VOLUME_MAX;
    }

    i4_ret = u_acfg_set_volume(ui1_volume, TRUE);
    if (APP_CFGR_OK != i4_ret)
    {
        return i4_ret;
    }

	if(role == BT_A2DP_ROLE_SINK)
    {
        _bluetooth_avrcp_volume_sync(ui1_volume);
    }
#if CONFIG_APP_SUPPORT_BT_SRC_MODE
    else if (role == BT_A2DP_ROLE_SRC)
    {
        if(bt_src_suppurt_absolute_volume)
        {
            bt_absolute_volume = (ui1_volume * BT_AUD_REMOTE_DEV_MAX_VOLUME) / BT_AUD_LOCAL_MAX_VOLUME;
            i4_ret = a_mtkapi_avrcp_change_volume(g_cur_avrcp_addr,bt_absolute_volume);
		    BT_AUD_CHK_FAIL(a_mtkapi_avrcp_change_volume, i4_ret);
        }
        else
        {
           //a_mtkapi_avrcp_passthrough_cmd(BT_CMD_TYPE_VOLUME_UP, BUTTON_ACT_AUTO);
           i4_ret = mas_set_master_vol(g_mas_handle,ui1_volume);
           if(0 == i4_ret)
           {
               DBG_INFO((BTAUD_TAG"mas_set_master_vol success\n"));
           }
           else
           {
               DBG_ERROR((BTAUD_TAG"mas_set_master_vol fail ,i4_ret:%d\n", i4_ret));
           }
        }
    }
#endif /*CONFIG_APP_SUPPORT_BT_SRC_MODE*/
    return APP_CFGR_OK;
}


INT32 _bluetooth_acfg_reduce_volume(VOID)
{
    UINT8 ui1_volume, bt_absolute_volume;
    BT_A2DP_ROLE role;
    INT32 i4_ret;

    DBG_INFO((BTAUD_TAG"u_acfg_reduce_volume\n"));

    i4_ret = u_acfg_set_mute(FALSE);
    if (APP_CFGR_OK != i4_ret)
    {
        return i4_ret;
    }

    i4_ret = u_acfg_get_volume(&ui1_volume);
    if (APP_CFGR_OK != i4_ret)
    {
        return i4_ret;
    }

    if (ui1_volume < BT_VOLUME_STEP)
    {
        ui1_volume = 0;
    }
    else
    {
        ui1_volume -= BT_VOLUME_STEP;
    }

    i4_ret = u_acfg_set_volume(ui1_volume, TRUE);
    if (APP_CFGR_OK != i4_ret)
    {
        return i4_ret;
    }

	if(role == BT_A2DP_ROLE_SINK)
	{
		_bluetooth_avrcp_volume_sync(ui1_volume);
	}
#if CONFIG_APP_SUPPORT_BT_SRC_MODE
	else if (role == BT_A2DP_ROLE_SRC)
	{
		if(bt_src_suppurt_absolute_volume)
		{
		   bt_absolute_volume = (ui1_volume * BT_AUD_REMOTE_DEV_MAX_VOLUME) / BT_AUD_LOCAL_MAX_VOLUME;
		   i4_ret = a_mtkapi_avrcp_change_volume(g_cur_avrcp_addr,bt_absolute_volume);
		   BT_AUD_CHK_FAIL(a_mtkapi_avrcp_change_volume, i4_ret);
		}
		else
		{
		   //a_mtkapi_avrcp_passthrough_cmd(BT_CMD_TYPE_VOLUME_DOWN, BUTTON_ACT_AUTO);
		   i4_ret = mas_set_master_vol(g_mas_handle,ui1_volume);
		   if(0 == i4_ret)
		   {
			   DBG_INFO((BTAUD_TAG"mas_set_master_vol success\n"));
		   }
		   else
		   {
			   DBG_ERROR((BTAUD_TAG"mas_set_master_vol fail, i4_ret:%d\n", i4_ret));
		   }
		}
	}
#endif /*CONFIG_APP_SUPPORT_BT_SRC_MODE*/
    return APP_CFGR_OK;
}

static VOID _bluetooth_process_ui_mute_key(VOID)
{
    BOOL b_vol_mute;
    UINT8 ui1_volume;
	BT_A2DP_ROLE role;
    INT32 i4_ret;

    DBG_INFO((BTAUD_TAG"receive volume mute key!\n"));
    i4_ret = u_acfg_get_mute(&b_vol_mute);
    if (APP_CFGR_OK != i4_ret)
    {
        DBG_ERROR((BTAUD_TAG"u_acfg_get_mute fail ret:%d\n", i4_ret));
        return;
    }

    b_vol_mute = !b_vol_mute;

    i4_ret = u_acfg_set_mute(b_vol_mute);
    if (APP_CFGR_OK != i4_ret)
    {
        DBG_ERROR((BTAUD_TAG"u_acfg_set_mute fail ret:%d\n", i4_ret));
        return;
    }

    if (!b_vol_mute)
    {
        i4_ret = u_acfg_get_volume(&ui1_volume);
        if (APP_CFGR_OK != i4_ret)
        {
            DBG_ERROR((BTAUD_TAG"u_acfg_get_volume fail ret:%d\n", i4_ret));
            return;
        }

        i4_ret = u_acfg_set_volume(ui1_volume, TRUE);
        if (APP_CFGR_OK != i4_ret)
        {
            DBG_ERROR((BTAUD_TAG"u_acfg_set_volume fail ret:%d\n", i4_ret));
            return;
        }

		if(role == BT_A2DP_ROLE_SINK)
        {
            _bluetooth_avrcp_volume_sync(ui1_volume);
        }
#if CONFIG_APP_SUPPORT_BT_SRC_MODE
        else if (role == BT_A2DP_ROLE_SRC)
        {
            if(bt_src_suppurt_absolute_volume)
            {
               i4_ret = a_mtkapi_avrcp_change_volume(g_cur_avrcp_addr,ui1_volume);
			   BT_AUD_CHK_FAIL(a_mtkapi_avrcp_change_volume, i4_ret);
            }
            else
            {
               //a_mtkapi_avrcp_passthrough_cmd(BT_CMD_TYPE_PAUSE, BUTTON_ACT_AUTO);
                i4_ret = mas_set_master_vol(g_mas_handle,ui1_volume);
                if(0 == i4_ret)
                {
                    DBG_INFO((BTAUD_TAG"mas_set_master_vol success\n"));
                }
                else
                {
                    DBG_ERROR((BTAUD_TAG"mas_set_master_vol fail, i4_ret:%d\n", i4_ret));
                }
            }
        }
#endif /*CONFIG_APP_SUPPORT_BT_SRC_MODE*/
    }
}

static VOID _bluetooth_process_ui_volume_up_key(VOID)
{
    INT32 i4_ret;

    DBG_INFO((BTAUD_TAG"receive volume up key!\n"));
    i4_ret = _bluetooth_acfg_increase_volume();
    if (APP_CFGR_OK != i4_ret)
    {
        DBG_ERROR((BTAUD_TAG"u_acfg_increase_volume fail ret:%d\n", i4_ret));
    }
}

static VOID _bluetooth_process_ui_volume_down_key(VOID)
{
    INT32 i4_ret;

    DBG_INFO((BTAUD_TAG"receive volume down key!\n"));
    i4_ret = _bluetooth_acfg_reduce_volume();
    if (APP_CFGR_OK != i4_ret)
    {
        DBG_ERROR((BTAUD_TAG"u_acfg_increase_volume fail ret:%d\n", i4_ret));
    }
}

VOID _bluetooth_ui_msg_process(const VOID *pv_msg, SIZE_T z_msg_len)
{
    APPMSG_T *t_msg = (APPMSG_T *)pv_msg;

    if (z_msg_len != sizeof(*t_msg))
    {
        DBG_ERROR((BTAUD_TAG"ui's msg size[%d] is wrong, should be [%d]!\n", z_msg_len, sizeof(APPMSG_T)));
        return;
    }

    DBG_INFO((BTAUD_TAG"receive ui key value:%d!\n", t_msg->ui4_msg_type));

    switch(t_msg->ui4_msg_type)
    {
        case KEY_MUTE:
            _bluetooth_process_ui_mute_key();
            break;
        case KEY_VOLUMEUP:
            _bluetooth_process_ui_volume_up_key();
            break;
        case KEY_VOLUMEDOWN:
            _bluetooth_process_ui_volume_down_key();
            break;
        default:
            break;
    }
}

VOID _bluetooth_process_timer_msg(const VOID* pv_msg, SIZE_T z_msg_len)
{
    BT_TIMER_MSG_E e_msg = *(BT_TIMER_MSG_E *)pv_msg;

    if (z_msg_len != sizeof(e_msg))
    {
        DBG_ERROR((BTAUD_TAG"timer's msg size %d is wrong, should be %d !\n", z_msg_len, sizeof(e_msg)));
        return;
    }

    DBG_ERROR((BTAUD_TAG"[timer] event:%d\n", e_msg));

    switch(e_msg)
    {
		case BT_TIMER_MSG_STICKY_PAIRING:
            _bluetooth_sticky_pairing_timer_cb();
            break;

		case BT_TIMER_MSG_KEY_DELAY:
			_bluetooth_key_delay_timer_cb();
            break;

        case BT_TIMER_MSG_ENTER_PARING:
            _bluetooth_enter_pairing_timer_cb();
            break;

        default:
            DBG_ERROR((BTAUD_TAG"Err:unrecognize [timer] event:%d\n", e_msg));
            break;
    }
}

//------------------------------------------------------------------------------
INT32 _bluetooth_state_play_proc(VOID)
{
    INT32 i4_ret = BT_AUD_OK;

	BT_AUD_FUNC_ENTRY();

    DBG_ERROR((BTAUD_TAG"_bt_aud_bluetooth_state_play_proc \r\n"));
    DBG_ERROR((BTAUD_TAG"b_bt_state_play_pause_proc: %d \r\n",
						g_t_bt_aud_sta.b_bt_state_play_pause_proc));

	if (BT_A2DP_ROLE_SRC == _bluetooth_a2dp_get_role())
	{
		DBG_ERROR((BTAUD_TAG"Current local role is SRC mode, ignore stream_start/play/data_come event.\n"));
		return BT_AUD_OK;
	}

    if(FALSE == g_t_bt_aud_sta.b_bt_state_play_pause_proc)
    {
        g_t_bt_aud_sta.b_bt_state_play_pause_proc = TRUE;
		g_t_bt_aud_sta.b_bt_play = TRUE;

        DBG_ERROR((BTAUD_TAG"e_bt_sm_cmd_sta: %d ,t_bt_aud_sta.b_bt_source : %d \r\n",
                    		g_t_bt_aud_sta.e_bt_sm_cmd_sta,g_t_bt_aud_sta.b_bt_source));

        if(BT_AUD_SM_REQ_PAUSE == g_t_bt_aud_sta.e_bt_sm_cmd_sta)
        {
            g_t_bt_aud_sta.e_bt_sm_cmd_sta= BT_AUD_SM_IDLE;
        }

		if ((TRUE == g_t_bt_hfp_sta.b_bt_hfp_media_play) || (TRUE == g_t_bt_hfp_sta.b_bt_hfp_mic_spk_thread_creat))
		{
			DBG_ERROR((BTAUD_TAG"HFP is playing, ignore A2DP play event!! \r\n"));

			//g_t_bt_aud_sta.b_bt_forbbid_play_by_hfp = TRUE;
#if 0
			i4_ret = _bluetooh_send_avrcp_pause_proc();
			BT_AUD_CHK_FAIL_RET(_bluetooh_send_avrcp_pause_proc, i4_ret, BT_AUD_FAIL);
#endif
			g_t_bt_aud_sta.b_bt_state_play_pause_proc = FALSE;

			return BT_AUD_OK;
		}

        if(TRUE == g_t_bt_aud_sta.b_bt_source) //switch to BT source
        {
            strncpy(g_t_bt_aud_play_inform.player.status, "play",ASSISTANT_STUB_STATUS_MAX_LENGTH);    // for assistant_stub

            DBG_ERROR((BTAUD_TAG"BT is playing,send inform play to SM!! \r\n"));

            i4_ret = _bluetooth_send_inform_play_to_sm();
            BT_AUD_CHK_FAIL_RET(_bluetooth_send_inform_play_to_sm, i4_ret, BT_AUD_FAIL);

            i4_ret = _bluetooth_send_player_status_to_assistant_stub();
            BT_AUD_CHK_FAIL_RET(_bluetooth_send_player_status_to_assistant_stub, i4_ret, BT_AUD_FAIL);
        }
        else
        {
            DBG_ERROR((BTAUD_TAG"Not BT source ,request to SM fistly!! \r\n"));
            i4_ret = _bluetooth_send_request_play_to_sm();
            BT_AUD_CHK_FAIL_RET(_bluetooth_send_request_play_to_sm, i4_ret, BT_AUD_FAIL);
        }
    }

	BT_AUD_FUNC_EXIT();
    return BT_AUD_OK;
}

INT32 _bluetooth_state_pause_proc(VOID)
{
    INT32 i4_ret;
    INT32 bt_mode;

	BT_AUD_FUNC_ENTRY();

    DBG_ERROR((BTAUD_TAG"_bt_aud_bluetooth_state_pause_proc \r\n"));
    DBG_ERROR((BTAUD_TAG"b_bt_state_play_pause_proc: %d \r\n",
						g_t_bt_aud_sta.b_bt_state_play_pause_proc));

	if (BT_A2DP_ROLE_SRC == _bluetooth_a2dp_get_role())
	{
		DBG_ERROR((BTCLI_TAG"Current local role is SRC mode, ignore stream_suspend/pause event.\n"));
		return BT_AUD_OK;
	}

    if (TRUE == g_t_bt_aud_sta.b_bt_source)
    {
        g_t_bt_aud_sta.b_bt_source = FALSE;
    }
    else
    {
        DBG_ERROR((BTAUD_TAG"Not BT source,no need to change source status. \n"));
    }

    if (TRUE == g_t_bt_aud_sta.b_bt_state_play_pause_proc)
    {
        g_t_bt_aud_sta.b_bt_state_play_pause_proc = FALSE;
		g_t_bt_aud_sta.b_bt_play = FALSE;

        DBG_ERROR((BTAUD_TAG"e_bt_sm_cmd_sta: %d , t_bt_aud_sta.b_bt_source : %d "
                            "b_bt_a2dp_connect : %d \r\n",g_t_bt_aud_sta.e_bt_sm_cmd_sta,
                            g_t_bt_aud_sta.b_bt_source,g_t_bt_aud_sta.b_bt_a2dp_connect));

        if (TRUE == g_t_bt_aud_sta.b_bt_a2dp_connect)
        {
            g_t_bt_aud_sta.e_bt_sta = BT_AUD_BT_STA_CONNECTED;
        }
#if 0
		if (TRUE == g_t_bt_aud_sta.b_bt_forbbid_play_by_hfp)
		{
			g_t_bt_aud_sta.b_bt_forbbid_play_by_hfp = FALSE;
			DBG_ERROR((BTAUD_TAG"BT src was forbidded play by hfp, here just return. \n"));

			return BT_AUD_OK;
		}
#endif
        if ((BT_AUD_SM_REQ_PAUSE != g_t_bt_aud_sta.e_bt_sm_cmd_sta)
			&& (BT_AUD_SM_REQ_STOP != g_t_bt_aud_sta.e_bt_sm_cmd_sta))  //manually pause or once connect then recv the event
        {
			g_t_bt_aud_sta.e_bt_sm_cmd_sta = BT_AUD_SM_IDLE;

			i4_ret = a_mtkapi_a2dp_sink_stop_player();
	        BT_AUD_CHK_FAIL(a_mtkapi_a2dp_sink_start_player, i4_ret);
			g_t_bt_aud_sta.b_bt_open_player = FALSE;

			DBG_ERROR((BTAUD_TAG"BT src is STOPED by manual, send inform STOP to SM \n"));
            i4_ret = _bluetooth_send_inform_stop_to_sm();
            BT_AUD_CHK_FAIL_RET(_bluetooth_send_inform_stop_to_sm, i4_ret, BT_AUD_FAIL);
        }

    //    g_t_bt_aud_sta.b_bt_play_pause_proc = FALSE;
        if (BT_AUD_SM_REQ_PAUSE == g_t_bt_aud_sta.e_bt_sm_cmd_sta)
        {
            strncpy(g_t_bt_aud_play_inform.player.status, "pause",ASSISTANT_STUB_STATUS_MAX_LENGTH);  // for assistant_stub
        }
        else
        {
            strncpy(g_t_bt_aud_play_inform.player.status, "stop",ASSISTANT_STUB_STATUS_MAX_LENGTH);  // for assistant_stub
        }

        i4_ret = _bluetooth_send_player_status_to_assistant_stub();
        BT_AUD_CHK_FAIL_RET(_bluetooth_send_player_status_to_assistant_stub, i4_ret, BT_AUD_FAIL);
    }

	BT_AUD_FUNC_EXIT();
    return BT_AUD_OK;
}

/*send bluetooth status to assistant_stub */
INT32 _bluetooth_send_bt_status_to_assistant_stub(CHAR *bt_status)
{
	INT32 i4_ret = BT_AUD_FAIL;
    HANDLE_T h_app = NULL_HANDLE;
	ASSISTANT_STUB_BLUETOOTH_STATUS_CHANGE_T bt_status_change={0};
	BT_LOCAL_DEV ps_dev_info;
	//tBTMW_GAP_DEVICE_INFO ps_target_dev_info;
    CHAR device_name[MAX_NAME_LEN]={0};

	BT_AUD_FUNC_ENTRY();

    i4_ret = a_mtkapi_bt_gap_get_local_dev_info(&ps_dev_info);
    BT_AUD_CHK_FAIL(a_mtkapi_bt_gap_get_local_dev_info, i4_ret);
    DBG_ERROR((BTAUD_TAG" ====ASSISTANT_STUB=====ps_dev_info.name : %s \r\n",ps_dev_info.name));

	strncpy(bt_status_change.name,
			ps_dev_info.name,
			ASSISTANT_STUB_BT_NAME_MAX_LENGTH);
    DBG_API((BTAUD_TAG" ====ASSISTANT_STUB=====bt_status_change.name : %s \r\n",bt_status_change.name));

	strncpy(bt_status_change.command,
			"/system/bluetooth_status_change",
			ASSISTANT_STUB_COMMAND_MAX_LENGTH);
	DBG_API((BTAUD_TAG" ====ASSISTANT_STUB=====bt_status_change.command : %s \r\n",bt_status_change.command));

	strncpy(bt_status_change.status,
			bt_status,
			ASSISTANT_STUB_STATUS_MAX_LENGTH);
	DBG_API((BTAUD_TAG" ====ASSISTANT_STUB=====bt_status_change.status : %s \r\n",bt_status_change.status));

//    gettimeofday(&sysTime, NULL);
//    DBG_ERROR((BTAUD_TAG "%s,%d,Time start:(%lu.%03lu)s \n",__FUNCTION__,__LINE__,sysTime.tv_sec,sysTime.tv_usec/1000));


//    gettimeofday(&sysTime, NULL);
//    DBG_ERROR((BTAUD_TAG "%s,%d,Time start:(%lu.%03lu)s \n",__FUNCTION__,__LINE__,sysTime.tv_sec,sysTime.tv_usec/1000));

	_bluetooth_get_remote_device_name(device_name,g_cur_gap_addr);

    strncpy(bt_status_change.bt_paired_name,
			device_name,
    		ASSISTANT_STUB_BT_PAIRED_NAME_MAX_LENGTH);
    DBG_ERROR((BTAUD_TAG" ====ASSISTANT_STUB=====bt_status_change.bt_paired_name : %s \r\n",
				bt_status_change.bt_paired_name));

    u_am_get_app_handle_from_name(&h_app,ASSISTANT_STUB_THREAD_NAME);

    i4_ret = u_app_send_appmsg(h_app,
                                E_APP_MSG_TYPE_ASSISTANT_STUB,
                                MSG_FROM_BT,
                                ASSISTANT_STUB_CMD_BLUETOOTH_STATUS_CHANGE,
                                &bt_status_change,
                                sizeof(ASSISTANT_STUB_BLUETOOTH_STATUS_CHANGE_T));
    if (AEER_OK != i4_ret) {
        DBG_ERROR((BTAUD_TAG"\n bluetooth send bt_status to assistant_stub failed, i4_ret[%ld]\n",i4_ret));
    }

    return (i4_ret == AEER_OK ? BT_AUD_OK : BT_AUD_FAIL);
}

INT32 _bluetooth_profile_msg_proc(const VOID *pv_msg)
{
    INT32 i4_ret = BT_AUD_FAIL;
    BT_AUD_MSG_T* pt_bt_msg = (BT_AUD_MSG_T*)pv_msg;
	CHAR* profile_type[]={"NULL","GAP","A2DP","AVRCP"};

    BT_AUD_FUNC_ENTRY();

	DBG_ERROR((BTAUD_TAG"Bluetooth profile(%s) msg come.\r\n",
						profile_type[pt_bt_msg->ui4_msg_type_profile]));

    switch (pt_bt_msg->ui4_msg_type_profile)
    {
        case BT_AUD_MSG_BT_GAP_PROFILE:
        {
            i4_ret = _bluetooth_gap_msg_proc(pt_bt_msg);
            BT_AUD_CHK_FAIL(_bluetooth_gap_event_proc, i4_ret);
        }
		break;

        case BT_AUD_MSG_BT_A2DP_PROFILE:
            i4_ret = _bluetooth_a2dp_event_proc(pt_bt_msg);
            BT_AUD_CHK_FAIL(_bluetooth_a2dp_event_proc, i4_ret);
            break;

        case BT_AUD_MSG_BT_AVRCP_PROFILE:
            i4_ret = _bluetooth_avrcp_event_proc(pt_bt_msg);
            BT_AUD_CHK_FAIL(_bluetooth_avrcp_event_proc, i4_ret);
            break;

        default:
            DBG_ERROR((BTAUD_TAG"Err:can't recognize the profile msgtype %d.\r\n",
								pt_bt_msg->ui4_msg_type_profile));
            break;
    }

    BT_AUD_FUNC_EXIT();
    return BT_AUD_OK;
}

//send play request to sm
INT32 _bluetooth_send_request_play_to_sm(void)
{
    DBG_ERROR(("<BT_AUD> %s\n",__FUNCTION__));

    INT32 i4_ret = BT_AUD_FAIL;
    HANDLE_T h_app = NULL_HANDLE;

    u_am_get_app_handle_from_name(&h_app,SM_THREAD_NAME);

    i4_ret = u_app_send_appmsg(h_app,
                                E_APP_MSG_TYPE_STATE_MNGR,
                                MSG_FROM_BT,
                                SM_MAKE_MSG(SM_REQUEST_GRP,SM_BODY_PLAY,0),
                                NULL,
                                0);
    if (AEER_OK != i4_ret) {
        DBG_ERROR((BTAUD_TAG"_bluetooth_send_play_request_to_sm failed, i4_ret[%ld]\n",i4_ret));
    }

    return (i4_ret == AEER_OK ? BT_AUD_OK : BT_AUD_FAIL);
}

//send inform play to sm
INT32 _bluetooth_send_inform_play_to_sm(void)
{
    DBG_ERROR(("<BT_AUD> %s\n",__FUNCTION__));

	INT32 i4_ret = BT_AUD_FAIL;
    HANDLE_T h_app = NULL_HANDLE;
    SM_PARAM_T t_sm_param = {0};

    DBG_ERROR((BTAUD_TAG"g_sm_parameter_id : %d \n",g_sm_parameter_id));

    u_am_get_app_handle_from_name(&h_app,SM_THREAD_NAME);

    i4_ret = u_app_send_appmsg(h_app,
                                E_APP_MSG_TYPE_STATE_MNGR,
                                MSG_FROM_BT,
                                SM_MAKE_MSG(SM_INFORM_GRP,SM_BODY_PLAY,g_sm_parameter_id),
                                &t_sm_param,
                                sizeof(t_sm_param));
    if (AEER_OK != i4_ret) {
        DBG_ERROR((BTAUD_TAG" _bluetooth_send_inform_play_to_sm failed, i4_ret[%ld]\n",i4_ret));
    }

    return (i4_ret == AEER_OK ? BT_AUD_OK : BT_AUD_FAIL);
}

//send inform stop to sm
INT32 _bluetooth_send_inform_stop_to_sm(void)
{
    DBG_ERROR(("<BT_AUD> %s\n",__FUNCTION__));

	INT32 i4_ret = BT_AUD_FAIL;
    HANDLE_T h_app = NULL_HANDLE;
    SM_PARAM_T t_sm_param = {0};

    DBG_ERROR((BTAUD_TAG"g_sm_parameter_id : %d \n",g_sm_parameter_id));

    u_am_get_app_handle_from_name(&h_app,SM_THREAD_NAME);

    i4_ret = u_app_send_appmsg(h_app,
                                E_APP_MSG_TYPE_STATE_MNGR,
                                MSG_FROM_BT,
                                SM_MAKE_MSG(SM_INFORM_GRP,SM_BODY_STOP,g_sm_parameter_id),
                                &t_sm_param,
                                sizeof(t_sm_param));
    if (AEER_OK != i4_ret) {
        DBG_ERROR((BTAUD_TAG"_bluetooth_send_status_stop_to_sm failed, i4_ret[%ld]\n",i4_ret));
    }

    return (i4_ret == AEER_OK ? BT_AUD_OK : BT_AUD_FAIL);
}

//send inform pause to sm
INT32 _bluetooth_send_inform_pause_to_sm(void)
{
    DBG_ERROR(("<BT_AUD> %s\n",__FUNCTION__));

	INT32 i4_ret = BT_AUD_FAIL;
    HANDLE_T h_app = NULL_HANDLE;
    SM_PARAM_T t_sm_param = {0};


    u_am_get_app_handle_from_name(&h_app,SM_THREAD_NAME);

    DBG_ERROR((BTAUD_TAG"g_sm_parameter_id : %d \n",g_sm_parameter_id));

    i4_ret = u_app_send_appmsg(h_app,
                                E_APP_MSG_TYPE_STATE_MNGR,
                                MSG_FROM_BT,
                                SM_MAKE_MSG(SM_INFORM_GRP,SM_BODY_PAUSE,g_sm_parameter_id),
                                &t_sm_param,
                                sizeof(t_sm_param));
    if (AEER_OK != i4_ret) {
        DBG_ERROR((BTAUD_TAG"_bluetooth_send_status_pause_to_sm failed, i4_ret[%ld]\n",i4_ret));
    }

    return (i4_ret == AEER_OK ? BT_AUD_OK : BT_AUD_FAIL);
}

//send msg of pairing result and timeout to sm duiring enter pairing
INT32 _bluetooth_send_pairing_result_to_sm(void)
{
    DBG_ERROR(("<BT_AUD> %s\n",__FUNCTION__));

	INT32 i4_ret = BT_AUD_FAIL;
    HANDLE_T h_app = NULL_HANDLE;

    u_am_get_app_handle_from_name(&h_app,SM_THREAD_NAME);

    i4_ret = u_app_send_appmsg(h_app,
                                E_APP_MSG_TYPE_BLUETOOTH,
                                MSG_FROM_BT,
                                SM_BT_FINISH_PARING,
                                NULL,
                                0);
    if (AEER_OK != i4_ret) {
        DBG_ERROR((BTAUD_TAG"_bluetooth_send_pairing_result_to_sm failed, i4_ret[%ld]\n",i4_ret));
    }

    return (i4_ret == AEER_OK ? BT_AUD_OK : BT_AUD_FAIL);
}

//pre init volume
INT32 _bluetooth_app_pre_play_init(VOID)
{
	DBG_ERROR(("<BT_AUD> %s\n",__FUNCTION__));

	/*change volume value , set to SCC*/
	INT32 i4_ret = BT_AUD_OK;
	UINT8 ui1_value=0;
	void* handle = NULL;

    /* get current local volume */
	i4_ret = u_acfg_get_volume(&ui1_value);
	if(BT_AUD_OK != i4_ret)
	{
		DBG_ERROR(("<BT_AUD> a_cfg_get_volume failed\n"));
		return BT_AUD_FAIL;
	}

	DBG_ERROR(("<BT_AUD> set volume to ALSA ,ui1_value=%d \n",ui1_value));

	/*change volume value, sync to remote */
	//a_bt_aud_volume_sync(ui1_value);

	return BT_AUD_OK;
}

INT32 _bluetooth_key_cmd_proc (const CHAR *key_cmd)
{
    INT32 i4_ret;
    UINT32 ui4_keymsg = 0, ui4_i = 0;
    BT_AUD_KEY2MSG_T at_key[] = {
        {"PLAYPAUSE",   BT_AUD_KEY_PAUSE_PLAY},
        {"PLAY",        BT_AUD_KEY_PLAY},
        {"PAUSE",       BT_AUD_KEY_PAUSE},
        {"STOP",        BT_AUD_KEY_STOP},
        {"NEXT",        BT_AUD_KEY_NEXT},
        {"PREV",        BT_AUD_KEY_PREV},
        {"SWITCH",      BT_AUD_KEY_SRC_SWITCH},
        {"FORCEDPAIR",  BT_AUD_KEY_FORCED_PAIRING},
        {"VOLUMEDOWN",  BT_AUD_KEY_VOL_DOWN},
        {"VOLUMEUP",    BT_AUD_KEY_VOL_UP},

        {NULL, 0},
    };

    //match the support key
    for (ui4_i = 0; at_key[ui4_i].pc_keystr != NULL ;ui4_i++)
    {
        if(strcmp(at_key[ui4_i].pc_keystr, key_cmd )== 0)
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
            DBG_ERROR(("%s  ",at_key[ui4_i].pc_keystr));
        }
        DBG_ERROR(("\r\n"));
        return BT_AUD_FAIL;
    }

    DBG_API((BTAUD_TAG"%s key press.\r\n", at_key[ui4_i].pc_keystr));

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
            i4_ret = _bluetooth_send_key_msg(BT_AUD_MSG_KEY_PLAY_CTRL,
                                            BT_AUD_KEYSTA_DOWN,
                                            ui4_keymsg);
            BT_AUD_CHK_FAIL_RET(_bluetooth_send_key_msg, i4_ret, CLIR_CMD_EXEC_ERROR);
            break;
    }

    return BT_AUD_OK;
}

INT32 _bluetooth_profiles_disconnect(VOID)
{
    INT32 i4_ret;
	BT_AUD_FUNC_ENTRY();

	DBG_ERROR((BTAUD_TAG" bluetooth start disconnect profiles... \r\n"));

	if(BT_A2DP_ROLE_SINK == g_cur_a2dp_role)
	{		
		DBG_ERROR((BTAUD_TAG"Current A2DP role is SINK.\r\n"));
#if CONFIG_SUPPORT_BT_HFP
		if (g_t_bt_hfp_sta.b_bt_hfp_connect)
		{
			//hfp disconnect
			DBG_ERROR((BTAUD_TAG BTHFP_TAG"HFP disconnect paired device : %s .\r\n",
										  g_cur_hfp_addr));

			i4_ret = a_mtkapi_bt_hfclient_disconnect(g_cur_hfp_addr);
			BT_AUD_CHK_FAIL_RET(a_mtkapi_bt_hfclient_disconnect, i4_ret, BT_HFP_FAIL);
		}
		else
		{
			DBG_ERROR((BTAUD_TAG"HFP is disconnected.\r\n"));
		}
#endif /*CONFIG_SUPPORT_BT_HFP*/
	}
	else if(BT_A2DP_ROLE_SRC == g_cur_a2dp_role)
	{
		DBG_ERROR((BTAUD_TAG"Current A2DP role is SRC.\r\n"));
	}

	DBG_ERROR((BTAUD_TAG BTHFP_TAG"A2DP disconnect paired device : %s .\r\n",
								  g_cur_a2dp_addr));

    i4_ret = a_mtkapi_a2dp_disconnect(g_cur_a2dp_addr);
    BT_AUD_CHK_FAIL_RET(a_mtkapi_a2dp_disconnect, i4_ret, BT_AUD_FAIL);
#if 0
	i4_ret = a_mtkapi_bt_gap_set_connectable_and_discoverable(TRUE, TRUE);
    BT_AUD_CHK_FAIL(a_mtkapi_bt_gap_set_connectable_and_discoverable, i4_ret);

	g_cur_gap_scan_mode = 2;
#endif
	return BT_AUD_OK;
}

INT32 _bluetooth_enter_pairing_key_proc(VOID)
{
    INT32    i4_ret = BT_AUD_OK;
    BT_AVRCP_KEY_STATE action;
	BT_A2DP_ROLE role;

    BT_AUD_FUNC_ENTRY();
    DBG_ERROR((BTAUD_TAG"_bluetooth_enter_pairing_key_proc.\r\n"));
    DBG_ERROR((BTAUD_TAG"_bluetooth_is_bt_profile_connected : %d,"
						"e_bt_src_playback_state : %d.\r\n",
						_bluetooth_is_bt_profile_connected(),
						g_t_bt_aud_sta.e_bt_src_playback_state));

    if (_bluetooth_is_bt_profile_connected())
    {
        DBG_ERROR((BTAUD_TAG"e_bt_src_playback_state : %d.\r\n",
        			g_t_bt_aud_sta.e_bt_src_playback_state));

        role = _bluetooth_a2dp_get_role();

        if ((BT_A2DP_ROLE_SRC == role)
        	&& (BT_AUD_SRC_BP_STA_PLAY == g_t_bt_aud_sta.e_bt_src_playback_state))
        {
        	DBG_ERROR((BTAUD_TAG"Current local role is SRC,playback is playing,stop playback firstly!!\r\n"));
        	i4_ret = _bluetooth_src_avrcp_cmd_proc(BT_AVRCP_CMD_TYPE_PAUSE);
        	BT_AUD_CHK_FAIL(_bluetooth_src_avrcp_cmd_proc, i4_ret);
        }
        u_thread_delay(300);

        //disconnect with device
        DBG_ERROR((BTAUD_TAG"BT is connected , start disconnecting... .\r\n"));

        i4_ret = _bluetooth_profiles_disconnect();
        BT_AUD_CHK_FAIL_RET(_bluetooth_profiles_disconnect, i4_ret, BT_AUD_FAIL);
#if 0
        i4_ret = a_mtkapi_bt_gap_set_connectable_and_discoverable(TRUE, TRUE);
        BT_AUD_CHK_FAIL(a_mtkapi_bt_gap_set_connectable_and_discoverable, i4_ret);

        g_t_bt_aud_sta.e_bt_sta = BT_AUD_BT_STA_PAIRING;
        g_cur_gap_scan_mode = 2;
#endif
    }
    else
    {
        i4_ret = _bluetooth_start_pairing();
        BT_AUD_CHK_FAIL(_bluetooth_start_pairing, i4_ret);
    }

    BT_AUD_FUNC_EXIT();
    return BT_AUD_OK;
}

INT32 _bluetooth_del_bt_paired_proc(VOID)
{
    INT32    i4_ret = BT_AUD_OK;

    BT_AUD_FUNC_ENTRY();
    DBG_ERROR((BTAUD_TAG"_bluetooth_del_bt_paired_proc.\r\n"));
    DBG_ERROR((BTAUD_TAG"_bluetooth_is_bt_connected : %d.\r\n",_bluetooth_is_bt_connected()));

    if (_bluetooth_is_bt_connected())
    {
        DBG_ERROR((BTAUD_TAG"BT is connected , start del paired....\r\n"));

        i4_ret = a_mtkapi_bt_gap_unpair(&g_cur_gap_addr);
        BT_AUD_CHK_FAIL_RET(a_mtkapi_bt_gap_unpair, i4_ret, BT_AUD_FAIL);
        DBG_API((BTAUD_TAG"bluetooth have del paired...\r\n"));

    }
    else
    {
        DBG_ERROR((BTAUD_TAG"There is not BT paired device.\r\n"));
    }

    BT_AUD_FUNC_EXIT();
    return BT_AUD_OK;
}

static INT32 _bluetooth_sm_permit_play_proc(APPMSG_T* pv_msg)
{
	INT32 i4_ret = BT_AUD_FAIL;

	BT_AUD_FUNC_ENTRY();

	g_sm_parameter_id = SM_MSG_ID(pv_msg->ui4_msg_type);  //SM PARAMETER ID
	DBG_ERROR((BTAUD_TAG"g_sm_parameter_id : %d \n",g_sm_parameter_id));

	if(TRUE == g_t_bt_aud_sta.b_bt_a2dp_connect)
	{
		strncpy(g_t_bt_aud_play_inform.player.status, "play",ASSISTANT_STUB_STATUS_MAX_LENGTH);	// for assistant_stub

		g_t_bt_aud_sta.e_bt_sm_cmd_sta = BT_AUD_SM_PMT_PLAY;
		g_t_bt_aud_sta.b_bt_source = TRUE;  //now it is BT source
		DBG_ERROR((BTAUD_TAG"e_bt_sm_cmd_sta : %d , b_bt_source : %d\r\n",
							g_t_bt_aud_sta.e_bt_sm_cmd_sta,g_t_bt_aud_sta.b_bt_source));

		if ((BT_AUD_SYSTEM_FAKE_STANDBY == g_t_bt_aud_sta.e_bt_system_sta) 
		 	||(BT_AUD_SYSTEM_SUSPEND_STANDBY == g_t_bt_aud_sta.e_bt_system_sta))
		{
			DBG_ERROR((BTAUD_TAG"exit standby %d .\n",g_t_bt_aud_sta.e_bt_system_sta));
			g_t_bt_aud_sta.e_bt_system_sta = BT_AUD_SYSTEM_NORMAL;
		}
		 	
		_bluetooth_start_playing();  //enter playing mode

		DBG_ERROR((BTAUD_TAG"BT src is playing, send inform to SM \n"));
		i4_ret = _bluetooth_send_inform_play_to_sm();
		BT_AUD_CHK_FAIL(_bluetooth_send_inform_play_to_sm, i4_ret);

		i4_ret = _bluetooth_send_player_status_to_assistant_stub();
		BT_AUD_CHK_FAIL_RET(_bluetooth_send_player_status_to_assistant_stub, i4_ret, BT_AUD_FAIL);
	}
	else
	{
		DBG_ERROR((BTAUD_TAG"Err:BT is disconnected,send inform STOP to SM .\r\n"));
		i4_ret = _bluetooth_send_inform_stop_to_sm();
		BT_AUD_CHK_FAIL_RET(_bluetooth_send_inform_stop_to_sm, i4_ret, BT_AUD_FAIL);

		if (g_t_bt_aud_sta.b_bt_state_play_pause_proc) //fix avrcp notify play while a2dp is still disconnected
		{
			g_t_bt_aud_sta.b_bt_state_play_pause_proc = FALSE;
		}
	}

	BT_AUD_FUNC_EXIT();
	return BT_AUD_OK;
}

static INT32 _bluetooth_sm_forbid_play_proc(VOID)
{
	INT32 i4_ret = BT_AUD_FAIL;

	BT_AUD_FUNC_ENTRY();

	g_t_bt_aud_sta.b_bt_state_play_pause_proc = FALSE;
	g_t_bt_aud_sta.e_bt_sm_cmd_sta = BT_AUD_SM_FBD_PLAY;
	g_t_bt_aud_sta.e_bt_sta = BT_AUD_BT_STA_CONNECTED;
	DBG_ERROR((BTAUD_TAG"b_bt_state_play_pause_proc : %d \r\n",g_t_bt_aud_sta.b_bt_state_play_pause_proc));

	i4_ret = _bluetooh_send_avrcp_pause_proc();
	BT_AUD_CHK_FAIL_RET(_bluetooh_send_avrcp_pause_proc, i4_ret, BT_AUD_FAIL);

	BT_AUD_FUNC_EXIT();
	return BT_AUD_OK;
}

static INT32 _bluetooth_sm_request_play_proc(APPMSG_T* pv_msg)
{
	INT32 i4_ret = BT_AUD_FAIL;

	BT_AUD_FUNC_ENTRY();

	g_sm_parameter_id = SM_MSG_ID(pv_msg->ui4_msg_type);  //SM PARAMETER ID
	DBG_ERROR((BTAUD_TAG"g_sm_parameter_id : %d \n",g_sm_parameter_id));

	if(TRUE == g_t_bt_aud_sta.b_bt_a2dp_connect)
	{
		g_t_bt_aud_sta.e_bt_sm_cmd_sta = BT_AUD_SM_REQ_PLAY;
		g_t_bt_aud_sta.b_bt_source = TRUE; //now it is BT source
		g_t_bt_aud_sta.e_bt_sta = BT_AUD_BT_STA_PLAYING;
		DBG_ERROR((BTAUD_TAG"e_bt_sm_cmd_sta : %d ,b_bt_source %d \r\n",
							g_t_bt_aud_sta.e_bt_sm_cmd_sta,g_t_bt_aud_sta.b_bt_source));

		i4_ret = _bluetooth_send_avrcp_play_proc();
		BT_AUD_CHK_FAIL_RET(_bluetooth_send_avrcp_play_proc, i4_ret, BT_AUD_FAIL);

		DBG_ERROR((BTAUD_TAG"BT src is playing, send inform to SM \n"));
		i4_ret = _bluetooth_send_inform_play_to_sm();
		BT_AUD_CHK_FAIL(_bluetooth_send_inform_play_to_sm, i4_ret);
	}
	else
	{
		DBG_ERROR((BTAUD_TAG"Err:BT is disconnected,send inform STOP to SM .\r\n"));
		i4_ret = _bluetooth_send_inform_stop_to_sm();
		BT_AUD_CHK_FAIL_RET(_bluetooth_send_inform_stop_to_sm, i4_ret, BT_AUD_FAIL);
	}


	BT_AUD_FUNC_EXIT();
	return BT_AUD_OK;
}

static INT32 _bluetooth_sm_request_pause_proc(VOID)
{
	INT32 i4_ret = BT_AUD_FAIL;

	BT_AUD_FUNC_ENTRY();

	if(TRUE == g_t_bt_aud_sta.b_bt_a2dp_connect)
	{
		g_t_bt_aud_sta.e_bt_sm_cmd_sta = BT_AUD_SM_REQ_PAUSE;
		g_t_bt_aud_sta.b_bt_source = FALSE;  //not BT source
		g_t_bt_aud_sta.b_bt_state_play_pause_proc = FALSE;

		DBG_ERROR((BTAUD_TAG".b_bt_source : %d ,b_bt_state_play_pause_proc : %d \r\n",
							g_t_bt_aud_sta.b_bt_source,g_t_bt_aud_sta.b_bt_state_play_pause_proc));

		if(TRUE==g_t_bt_aud_sta.b_bt_a2dp_connect)
		{
			g_t_bt_aud_sta.e_bt_sta = BT_AUD_BT_STA_CONNECTED;
		}

		i4_ret = _bluetooh_send_avrcp_pause_proc();
		BT_AUD_CHK_FAIL_RET(_bluetooh_send_avrcp_pause_proc, i4_ret, BT_AUD_FAIL);

		DBG_ERROR((BTAUD_TAG"BT src is PAUSE, send inform to SM \n"));
		i4_ret = _bluetooth_send_inform_pause_to_sm();
		BT_AUD_CHK_FAIL(_bluetooth_send_inform_pause_to_sm, i4_ret);
	}
	else
	{
		DBG_ERROR((BTAUD_TAG"Err:BT is disconnected,send inform STOP to SM .\r\n"));
		i4_ret = _bluetooth_send_inform_stop_to_sm();
		BT_AUD_CHK_FAIL_RET(_bluetooth_send_inform_stop_to_sm, i4_ret, BT_AUD_FAIL);
	}

	BT_AUD_FUNC_EXIT();
	return BT_AUD_OK;
}

static INT32 _bluetooth_sm_request_resume_proc(VOID)
{
	INT32 i4_ret = BT_AUD_FAIL;

	BT_AUD_FUNC_ENTRY();

	if (TRUE == g_t_bt_aud_sta.b_bt_a2dp_connect)
	{
		if ((BT_AUD_SM_REQ_PAUSE == g_t_bt_aud_sta.e_bt_sm_cmd_sta)
			|| (BT_AUD_SM_FBD_PLAY == g_t_bt_aud_sta.e_bt_sm_cmd_sta))
		{
			g_t_bt_aud_sta.e_bt_sta = BT_AUD_BT_STA_PLAYING;
			g_t_bt_aud_sta.e_bt_sm_cmd_sta = BT_AUD_SM_REQ_RESUME;	//now it is BT source
			g_t_bt_aud_sta.b_bt_source = TRUE;	//now it is BT source

			DBG_ERROR((BTAUD_TAG"e_bt_sm_cmd_sta : %d , b_bt_source : %d \r\n",
								g_t_bt_aud_sta.e_bt_sm_cmd_sta,g_t_bt_aud_sta.b_bt_source));

#if 0
			if(TRUE == g_t_bt_aud_sta.b_bt_pause_by_sm)
			{
				g_t_bt_aud_sta.b_bt_pause_by_sm = FALSE;
			}
#endif
			i4_ret = _bluetooth_send_avrcp_play_proc();
			BT_AUD_CHK_FAIL_RET(_bluetooth_send_avrcp_play_proc, i4_ret, BT_AUD_FAIL);

			DBG_ERROR((BTAUD_TAG"BT src is playing, send inform to SM \n"));
			i4_ret = _bluetooth_send_inform_play_to_sm();
			BT_AUD_CHK_FAIL(_bluetooth_send_inform_play_to_sm, i4_ret);
		}
		else
		{
			DBG_ERROR((BTAUD_TAG"BT source is not FORBID_PLAY or REQUEST_PAUSE by SM,ingnore RESUME request, e_bt_sm_cmd_sta : %d .\r\n",
								g_t_bt_aud_sta.e_bt_sm_cmd_sta));
			return BT_AUD_OK;
		}
	}
	else
	{
		DBG_ERROR((BTAUD_TAG"Err:BT is disconnected,send inform STOP to SM .\r\n"));
		i4_ret = _bluetooth_send_inform_stop_to_sm();
		BT_AUD_CHK_FAIL_RET(_bluetooth_send_inform_stop_to_sm, i4_ret, BT_AUD_FAIL);
	}

	BT_AUD_FUNC_EXIT();
	return BT_AUD_OK;
}

static INT32 _bluetooth_sm_request_stop_proc(APPMSG_T* pv_msg)
{
	INT32 i4_ret = BT_AUD_FAIL;

	BT_AUD_FUNC_ENTRY();

	if(TRUE == g_t_bt_aud_sta.b_bt_a2dp_connect)
	{
		g_t_bt_aud_sta.e_bt_sm_cmd_sta = BT_AUD_SM_REQ_STOP;  //BT is stoped by SM
		g_t_bt_aud_sta.b_bt_source = FALSE;  //BT source
		DBG_ERROR((BTAUD_TAG"e_bt_sm_cmd_sta : %d, b_bt_source : %d \r\n",
							g_t_bt_aud_sta.e_bt_sm_cmd_sta,g_t_bt_aud_sta.b_bt_source));

		if (0 == strcmp("FAKE_STANDBY",pv_msg->p_usr_msg))
		{ 
			g_t_bt_aud_sta.e_bt_system_sta = BT_AUD_SYSTEM_FAKE_STANDBY;
			DBG_ERROR((BTAUD_TAG"SM request enter fake standby!! \n"));
		}
		else if (0 == strcmp("SUSPEND_STANDBY",pv_msg->p_usr_msg))
		{
			g_t_bt_aud_sta.e_bt_system_sta = BT_AUD_SYSTEM_SUSPEND_STANDBY;
			DBG_ERROR((BTAUD_TAG"SM request enter suspend standby!! \n"));
		}
			
		if (TRUE==g_t_bt_aud_sta.b_bt_a2dp_connect)
		{
			g_t_bt_aud_sta.e_bt_sta = BT_AUD_BT_STA_CONNECTED;
		}

		i4_ret = _bluetooh_send_avrcp_pause_proc();
		BT_AUD_CHK_FAIL_RET(_bluetooh_send_avrcp_pause_proc, i4_ret, BT_AUD_FAIL);

		DBG_ERROR((BTAUD_TAG"BT src is STOP, send inform to SM \n"));
		i4_ret = _bluetooth_send_inform_stop_to_sm();
		BT_AUD_CHK_FAIL_RET(_bluetooth_send_inform_stop_to_sm, i4_ret, BT_AUD_FAIL);
	}
	else
	{
		DBG_ERROR((BTAUD_TAG"Err:BT is disconnected,send inform STOP to SM .\r\n"));
		i4_ret = _bluetooth_send_inform_stop_to_sm();
		BT_AUD_CHK_FAIL_RET(_bluetooth_send_inform_stop_to_sm, i4_ret, BT_AUD_FAIL);
	}

	BT_AUD_FUNC_EXIT();
	return BT_AUD_OK;
}

static INT32 _bluetooth_sm_request_next_proc(APPMSG_T* pv_msg)
{
	INT32 i4_ret = BT_AUD_FAIL;

	BT_AUD_FUNC_ENTRY();

	g_sm_parameter_id = SM_MSG_ID(pv_msg->ui4_msg_type);  //SM PARAMETER ID
	DBG_ERROR((BTAUD_TAG"g_sm_parameter_id : %d \n",g_sm_parameter_id));

	if(TRUE == g_t_bt_aud_sta.b_bt_a2dp_connect)
	{
		g_t_bt_aud_sta.e_bt_sm_cmd_sta = BT_AUD_SM_REQ_NEXT;
		g_t_bt_aud_sta.b_bt_source = TRUE;	//now it is BT source

		DBG_ERROR((BTAUD_TAG"b_bt_source : %d \r\n",g_t_bt_aud_sta.b_bt_source));

		i4_ret = _bluetooth_send_avrcp_next_proc();
		BT_AUD_CHK_FAIL_RET(_bluetooth_send_avrcp_next_proc, i4_ret, BT_AUD_FAIL);

		_bluetooth_start_playing();  //enter playing mode

		DBG_ERROR((BTAUD_TAG"BT src is playing, send inform to SM \n"));
		i4_ret = _bluetooth_send_inform_play_to_sm();
		BT_AUD_CHK_FAIL(_bluetooth_send_inform_play_to_sm, i4_ret);
	}
	else
	{
		DBG_ERROR((BTAUD_TAG"Err:BT is disconnected,send inform STOP to SM .\r\n"));
		i4_ret = _bluetooth_send_inform_stop_to_sm();
		BT_AUD_CHK_FAIL_RET(_bluetooth_send_inform_stop_to_sm, i4_ret, BT_AUD_FAIL);
	}

	BT_AUD_FUNC_EXIT();
	return BT_AUD_OK;
}

static INT32 _bluetooth_sm_request_prev_proc(APPMSG_T* pv_msg)
{
	INT32 i4_ret = BT_AUD_FAIL;

	BT_AUD_FUNC_ENTRY();

	g_sm_parameter_id = SM_MSG_ID(pv_msg->ui4_msg_type);  //SM PARAMETER ID
	DBG_ERROR((BTAUD_TAG"g_sm_parameter_id : %d \n",g_sm_parameter_id));

	if(TRUE == g_t_bt_aud_sta.b_bt_a2dp_connect)
	{
		g_t_bt_aud_sta.e_bt_sm_cmd_sta = BT_AUD_SM_REQ_PREV;
		g_t_bt_aud_sta.b_bt_source = TRUE;	//now it is BT source
		DBG_ERROR((BTAUD_TAG"b_bt_source : %d \r\n",g_t_bt_aud_sta.b_bt_source));

		i4_ret = _bluetooth_send_avrcp_prev_proc();
		BT_AUD_CHK_FAIL_RET(_bluetooth_send_avrcp_prev_proc, i4_ret, BT_AUD_FAIL);

		_bluetooth_start_playing();  //enter playing mode

		DBG_ERROR((BTAUD_TAG"BT src is playing, send inform to SM \n"));
		i4_ret = _bluetooth_send_inform_play_to_sm();
		BT_AUD_CHK_FAIL(_bluetooth_send_inform_play_to_sm, i4_ret);
	}
	else
	{
		DBG_ERROR((BTAUD_TAG"Err:BT is disconnected,send inform STOP to SM .\r\n"));
		i4_ret = _bluetooth_send_inform_stop_to_sm();
		BT_AUD_CHK_FAIL_RET(_bluetooth_send_inform_stop_to_sm, i4_ret, BT_AUD_FAIL);
	}

	BT_AUD_FUNC_EXIT();
	return BT_AUD_OK;
}

static INT32 _bluetooth_sm_request_recnt_proc(VOID)
{
	INT32 i4_ret = BT_AUD_FAIL;

	BT_AUD_FUNC_ENTRY();
	g_t_bt_aud_sta.e_bt_sm_cmd_sta = BT_AUD_SM_REQ_RECNT;

	i4_ret = _bluetooth_bt_reconnect_assistant();
	BT_AUD_CHK_FAIL_RET(_bluetooth_bt_reconnect_assistant, i4_ret, BT_AUD_FAIL);

	BT_AUD_FUNC_EXIT();
	return BT_AUD_OK;
}

static INT32 _bluetooth_sm_request_discnt_proc(VOID)
{
	INT32 i4_ret = BT_AUD_FAIL;

	BT_AUD_FUNC_ENTRY();

	g_t_bt_aud_sta.e_bt_sm_cmd_sta = BT_AUD_SM_REQ_DISCNT;

	if (TRUE == _bluetooth_is_bt_connected())//and already connect now, need to disconnect first
	{
		DBG_ERROR((BTAUD_TAG"_bluetooth_is_bt_connected \n"));

		g_t_bt_aud_sta.b_bt_reconnect_for_wifi = TRUE;
		g_t_bt_aud_sta.b_bt_forced_pairing = FALSE;

		i4_ret = _bluetooth_bt_disconnect(FALSE);
		BT_AUD_CHK_FAIL_RET(_bluetooth_bt_disconnect, i4_ret, BT_AUD_FAIL);
	}
	else if (g_t_bt_aud_sta.e_bt_sta == BT_AUD_BT_STA_AUTO_CONNECTING)//if autoconnecting, stop it
	{
		//g_t_bt_aud_sta.e_bt_sta = BT_AUD_BT_STA_IDLE;
		i4_ret = _bluetooth_start_pairing();// pairing mode
		BT_AUD_CHK_FAIL(_bluetooth_start_pairing, i4_ret);
	}

	DBG_ERROR((BTAUD_TAG"b_bt_source : %d \r\n",g_t_bt_aud_sta.b_bt_source));

	if (g_t_bt_aud_sta.b_bt_source)
	{
		g_t_bt_aud_sta.b_bt_source = FALSE;  //BT source
		DBG_ERROR((BTAUD_TAG"BT src is STOP, send inform to SM \n"));
		i4_ret = _bluetooth_send_inform_stop_to_sm();
		BT_AUD_CHK_FAIL(_bluetooth_send_inform_stop_to_sm, i4_ret);
	}

	BT_AUD_FUNC_EXIT();
	return BT_AUD_OK;
}

static INT32 _bluetooth_sm_request_pair_proc(VOID)
{
	INT32 i4_ret = BT_AUD_FAIL;

	BT_AUD_FUNC_ENTRY();

	g_t_bt_aud_sta.e_bt_sm_cmd_sta = BT_AUD_SM_REQ_PAIR;

	if (TRUE == g_t_bt_aud_sta.b_enter_pairing_enable)
	{
		_bluetooth_stop_enter_pairing_timer();
	}

	_bluetooth_start_enter_pairing_timer();

	i4_ret = _bluetooth_enter_pairing_key_proc();
	BT_AUD_CHK_FAIL(_bluetooth_enter_pairing_key_proc, i4_ret);


	BT_AUD_FUNC_EXIT();
	return BT_AUD_OK;
}

static INT32 _bluetooth_sm_msg_permit_proc(APPMSG_T* pv_msg)
{
	INT32 i4_ret = BT_AUD_FAIL;

	BT_AUD_FUNC_ENTRY();

	switch(SM_MSG_BDY(pv_msg->ui4_msg_type))
	{
		DBG_ERROR((BTAUD_TAG"SM_MSG_BDY is %d \r\n",SM_MSG_BDY(pv_msg->ui4_msg_type)));
		case SM_BODY_PLAY:
		{
			DBG_ERROR((BTAUD_TAG"SM permit BT source play!!! \n"));
			i4_ret = _bluetooth_sm_permit_play_proc(pv_msg);
			BT_AUD_CHK_FAIL(_bluetooth_enter_pairing_key_proc, i4_ret);
		}
		break;

		case SM_BODY_PAUSE:
		{
			DBG_ERROR((BTAUD_TAG"Ignore--- Permit PAUSE by SM\n"));
		}
		break;

		case SM_BODY_STOP:
		{
			DBG_ERROR((BTAUD_TAG"Ignore--- Permit PAUSE by SM\n"));
		}
		break;

		default:
			break;
	}

	BT_AUD_FUNC_EXIT();
	return BT_AUD_OK;
}

static INT32 _bluetooth_sm_msg_forbid_proc(APPMSG_T* pv_msg)
{
	INT32 i4_ret = BT_AUD_FAIL;

	BT_AUD_FUNC_ENTRY();

	if(TRUE == g_t_bt_aud_sta.b_bt_a2dp_connect)
	{
		switch(SM_MSG_BDY(pv_msg->ui4_msg_type))
		{
			DBG_ERROR((BTAUD_TAG"SM_MSG_BDY is %d \r\n",SM_MSG_BDY(pv_msg->ui4_msg_type)));

			case SM_BODY_PLAY:
			{
				DBG_ERROR((BTAUD_TAG"Forbid PLAY by SM \r\n"));
				_bluetooth_sm_forbid_play_proc();
				BT_AUD_CHK_FAIL(_bluetooth_enter_pairing_key_proc, i4_ret);
			}
			break;

			case SM_BODY_PAUSE:
			{
				DBG_ERROR((BTAUD_TAG"Ignore--- Forbid PAUSE by SM\n"));
			}
			break;

			case SM_BODY_STOP:
			{
				DBG_ERROR((BTAUD_TAG"Ignore--- Forbid STOP by SM\n"));
			}
			break;

			default:
			break;
		}
	}
	else
	{
		DBG_ERROR((BTAUD_TAG"Err:BT is disconnected,send inform STOP to SM .\r\n"));
		i4_ret = _bluetooth_send_inform_stop_to_sm();
		BT_AUD_CHK_FAIL_RET(_bluetooth_send_inform_stop_to_sm, i4_ret, BT_AUD_FAIL);
	}

	BT_AUD_FUNC_EXIT();
	return BT_AUD_OK;
}

static INT32 _bluetooth_sm_msg_request_proc(APPMSG_T* pv_msg)
{
	INT32 i4_ret = BT_AUD_FAIL;

	BT_AUD_FUNC_ENTRY();

	switch(SM_MSG_BDY(pv_msg->ui4_msg_type))
	{
		DBG_ERROR((BTAUD_TAG"SM_MSG_BDY is %d \r\n",SM_MSG_BDY(pv_msg->ui4_msg_type)));

		case SM_BODY_PLAY:
		{
			DBG_ERROR((BTAUD_TAG"REQUEST PLAY by SM\n"));

			i4_ret = _bluetooth_sm_request_play_proc(pv_msg);
			BT_AUD_CHK_FAIL(_bluetooth_sm_request_play_proc, i4_ret);
		}
		break;

		case SM_BODY_RESUME:
		{
			DBG_ERROR((BTAUD_TAG"REQUEST RESUME by SM\n"));

			i4_ret = _bluetooth_sm_request_resume_proc();
			BT_AUD_CHK_FAIL(_bluetooth_sm_request_resume_proc, i4_ret);

		}
		break;

		case SM_BODY_PAUSE:
		{
			DBG_ERROR((BTAUD_TAG"REQUEST PAUSE by SM\n"));

			i4_ret = _bluetooth_sm_request_pause_proc();
			BT_AUD_CHK_FAIL(_bluetooth_sm_request_pause_proc, i4_ret);
		}
		break;

		case SM_BODY_STOP:
		{
			DBG_ERROR((BTAUD_TAG"REQUEST STOP by SM\n"));

			i4_ret = _bluetooth_sm_request_stop_proc(pv_msg);
			BT_AUD_CHK_FAIL(_bluetooth_sm_request_stop_proc, i4_ret);
		}
		break;

		case SM_BODY_PREV:
		{
			DBG_ERROR((BTAUD_TAG" REQUEST PREV by SM \n"));

			i4_ret = _bluetooth_sm_request_prev_proc(pv_msg);
			BT_AUD_CHK_FAIL(_bluetooth_sm_request_prev_proc, i4_ret);
		}
		break;

		case SM_BODY_NEXT:
		{
			DBG_ERROR((BTAUD_TAG" REQUEST NEXT by SM \n"));

			i4_ret = _bluetooth_sm_request_next_proc(pv_msg);
			BT_AUD_CHK_FAIL(_bluetooth_sm_request_next_proc, i4_ret);
		}
		break;

		case SM_BODY_BT_RECON:
		{
			DBG_ERROR((BTAUD_TAG" REQUEST RECONNECT by SM \n"));
			i4_ret = _bluetooth_sm_request_recnt_proc();
			BT_AUD_CHK_FAIL(_bluetooth_sm_request_recnt_proc, i4_ret);
		}
		break;

		case SM_BODY_BT_DISCON:
		{
			DBG_ERROR((BTAUD_TAG" REQUEST DISCONNET by SM \n"));

			i4_ret = _bluetooth_sm_request_discnt_proc();
			BT_AUD_CHK_FAIL(_bluetooth_sm_request_discnt_proc, i4_ret);
		}
		break;

		case SM_BODY_BT_PAIRING:
		{
			DBG_ERROR((BTAUD_TAG" REQUEST PAIRING by SM \n"));

			i4_ret = _bluetooth_sm_request_pair_proc();
			BT_AUD_CHK_FAIL(_bluetooth_sm_request_pair_proc, i4_ret);
		}
		break;

#if ((!CONFIG_APP_SUPPORT_A2DP_SINK_SRC_BOTH) && CONFIG_APP_SUPPORT_BT_SRC_MODE)
		case SM_BODY_UI_LONG_SOURCE:
		{
			DBG_ERROR((BTAUD_TAG" REQUEST SOURCE/SINK Switch by SM \n"));
			i4_ret = _bluetooth_reset_a2dp_mode_proc();
			BT_AUD_CHK_FAIL(_bluetooth_reset_a2dp_mode_proc, i4_ret);
		}
		break;
#endif /*((!CONFIG_APP_SUPPORT_A2DP_SINK_SRC_BOTH) && CONFIG_APP_SUPPORT_BT_SRC_MODE)*/

		default:
			break;
	}

	BT_AUD_FUNC_EXIT();
	return BT_AUD_OK;
}

//bluetooth_sm_msg_process
INT32 _bluetooth_sm_msg_process(APPMSG_T* pv_msg)
{
    INT32 i4_ret = BT_AUD_FAIL;
    BT_AUD_FUNC_ENTRY();

    DBG_ERROR((BTAUD_TAG"SM msg come!!! \r\n"));
    DBG_ERROR((BTAUD_TAG"t_bt_aud_sta.b_bt_a2dp_connect: %d \r\n",g_t_bt_aud_sta.b_bt_a2dp_connect));

    switch(SM_MSG_GRP(pv_msg->ui4_msg_type))
    {
        DBG_ERROR((BTAUD_TAG"SM_MSG_GRP is %d \r\n",SM_MSG_GRP(pv_msg->ui4_msg_type)));

		case SM_PERMIT_GRP:
        {
			i4_ret = _bluetooth_sm_msg_permit_proc(pv_msg);
			BT_AUD_CHK_FAIL_GOTO(_bluetooth_sm_msg_permit_proc, i4_ret, err_out);
        }
        break;

        case SM_FORBID_GRP:
        {
			i4_ret = _bluetooth_sm_msg_forbid_proc(pv_msg);
			BT_AUD_CHK_FAIL_GOTO(_bluetooth_sm_msg_forbid_proc, i4_ret, err_out);
        }
        break;

		case SM_REQUEST_GRP:
        {
			i4_ret = _bluetooth_sm_msg_request_proc(pv_msg);
			BT_AUD_CHK_FAIL_GOTO(_bluetooth_sm_msg_request_proc, i4_ret, err_out);
        }
	    break;

	default:
	    DBG_ERROR((BTAUD_TAG"Err:can't recognize the switch msgtype .\r\n"));//, ui4_msgtype));
	    return BT_AUD_FAIL;
    }

    BT_AUD_FUNC_EXIT();
    return BT_AUD_OK;

err_out:
    //sm msg proc fail and notify state manager  fail msg
	i4_ret = _bluetooth_send_inform_stop_to_sm();
	BT_AUD_CHK_FAIL(_bluetooth_send_inform_stop_to_sm, i4_ret);
    return BT_AUD_FAIL;

}

INT32 _bluetooth_playback_msg_process(APPMSG_T* pv_msg)
{
    INT32 i4_ret = 0;
    BT_AUD_FUNC_ENTRY();

    DBG_ERROR((BTAUD_TAG"playback msg come!!! \r\n"));

    switch(pv_msg->ui4_msg_type)
    {
        DBG_ERROR((BTAUD_TAG"playback msg is %d \r\n",pv_msg->ui4_msg_type));

        case PB_BODY_PLAY:
        {
			g_t_bt_aud_sta.e_bt_src_playback_state = BT_AUD_SRC_BP_STA_PLAY;
			i4_ret = _bluetooth_src_update_player_status_to_mw(AVRCP_PLAY_STATUS_PLAYING);
            if (BT_AUD_OK != i4_ret)
            {
                DBG_ERROR(("<BT_AUD>_bluetooth_src_update_player_status_to_mw err, %s\n", __FUNCTION__));
                return BT_AUD_FAIL;
            }
/*
            i4_ret = _bluetooth_src_update_mediainfo_to_mw();
            if (BT_AUD_OK != i4_ret)
            {
                DBG_ERROR(("<BT_AUD>_bluetooth_src_update_mediainfo_to_mw err, %s\n", __FUNCTION__));
                return BT_AUD_FAIL;
            }
*/            
        }
        break;

        case PB_BODY_PAUSE:
        {
			g_t_bt_aud_sta.e_bt_src_playback_state = BT_AUD_SRC_BP_STA_PAUSE;
			i4_ret = _bluetooth_src_update_player_status_to_mw(AVRCP_PLAY_STATUS_PAUSED);
            if (BT_AUD_OK != i4_ret)
            {
                DBG_ERROR(("<BT_AUD>_bluetooth_src_update_player_status_to_mw err, %s\n", __FUNCTION__));
                return BT_AUD_FAIL;
            }

            i4_ret = _bluetooth_src_update_mediainfo_to_mw();
            if (BT_AUD_OK != i4_ret)
            {
                DBG_ERROR(("<BT_AUD>_bluetooth_src_update_mediainfo_to_mw err, %s\n", __FUNCTION__));
                return BT_AUD_FAIL;
            }
        }
        break;

        case PB_BODY_STOP:
        {
			g_t_bt_aud_sta.e_bt_src_playback_state = BT_AUD_SRC_BP_STA_STOP;
			i4_ret = _bluetooth_src_update_player_status_to_mw(AVRCP_PLAY_STATUS_STOPPED);
            if (BT_AUD_OK != i4_ret)
            {
                DBG_ERROR(("<BT_AUD>_bluetooth_src_update_player_status_to_mw err, %s\n", __FUNCTION__));
                return BT_AUD_FAIL;
            }
            i4_ret = _bluetooth_src_update_mediainfo_to_mw();
            if (BT_AUD_OK != i4_ret)
            {
                DBG_ERROR(("<BT_AUD>_bluetooth_src_update_mediainfo_to_mw err, %s\n", __FUNCTION__));
                return BT_AUD_FAIL;
            }
        }
        break;

	default:
	    DBG_ERROR((BTAUD_TAG"Err:can't recognize the playback msgtype.\r\n"));
	    return BT_AUD_FAIL;
    }

    BT_AUD_FUNC_EXIT();
    return BT_AUD_OK;

}
INT32 _bluetooth_set_absolute_volume(UINT8 ui1_value)
{
    INT32 i4_ret = 0;
    BOOL  b_vol_mute = FALSE;

    /*call acfg api to set absolute volume*/
    DBG_INFO(("<BT_AUD>set absolute volume, volume=%d\n",ui1_value));

    /* get current mute status*/
    i4_ret = u_acfg_get_mute(&b_vol_mute);
    if (BT_AUD_OK != i4_ret)
    {
        DBG_ERROR(("<BT_AUD>a_cfg_get_mute error in function %s\n", __FUNCTION__));
        return BT_AUD_FAIL;
    }

    if (b_vol_mute)
    {
        i4_ret = u_acfg_set_mute(FALSE);
        if (BT_AUD_OK != i4_ret)
        {
            DBG_ERROR(("<BT_AUD>u_acfg_set_mute error in function %s\n", __FUNCTION__));
            return BT_AUD_FAIL;
        }
    }

    /*change volume value, set to SCC*/
    i4_ret = u_acfg_set_volume((UINT8)ui1_value, TRUE);
    if (BT_AUD_OK != i4_ret)
    {
        DBG_ERROR(("<MISC>u_acfg_set_volume error in function %s\n", __FUNCTION__));
        return BT_AUD_FAIL;
    }

    return BT_AUD_OK;
}

//------------------------------------------------------------------------------
VOID _bluetooth_clear_connect_info(BOOL b_keep_connect)
{
#if CONFIG_APP_SUPPORT_BT_KEEP_CONNECT
    g_t_bt_aud_sta.b_keep_connect = b_keep_connect;
    if (g_t_bt_aud_sta.b_keep_connect == TRUE)
    {
        //keep connect, keep connect infomation
        g_t_bt_aud_sta.e_bt_sta = BT_AUD_BT_STA_CONNECTED;
    }
    else
#endif/* CONFIG_APP_SUPPORT_BT_KEEP_CONNECT */
    {
        g_t_bt_aud_sta.e_bt_sta = BT_AUD_BT_STA_IDLE;
        g_t_bt_aud_sta.ui4_bt_track_idx = 0;
    }

    g_t_bt_aud_sta.b_play_pause_down = FALSE;
    g_t_bt_aud_sta.b_bt_play = FALSE;
    g_t_bt_aud_sta.b_continue_connect = FALSE;
}

VOID _bluetooth_clear_status(VOID)
{
    DBG_ERROR((BTAUD_TAG"_bluetooth_clear_status.\r\n"));
    g_t_bt_aud_sta.b_bt_a2dp_connect = FALSE;
    g_t_bt_aud_sta.b_bt_state_play_pause_proc = FALSE;
	g_t_bt_aud_sta.e_bt_sm_cmd_sta = BT_AUD_SM_IDLE;
	g_t_bt_aud_sta.e_bt_sta = BT_AUD_BT_STA_DISCONNECT;
}

INT32 _bluetooth_paired_dev_list_update(BT_A2DP_DEVICE_LIST *pt_dev_list)
{
    INT32 i4_ret;
	CHAR* a2dp_device[]={"SINK","SRC"};

	BT_AUD_FUNC_ENTRY();

    if (pt_dev_list == NULL)
    {
        DBG_ERROR((BTAUD_TAG"Err: arg(pt_dev_list) is NULL!\r\n"));
        return BT_AUD_FAIL;
    }

    //get paring history device
    memset(pt_dev_list, 0, sizeof(BT_A2DP_DEVICE_LIST));

	if (BT_A2DP_ROLE_SINK == g_cur_a2dp_role)
	{
	    i4_ret = a_mtkapi_a2dp_sink_get_dev_list(pt_dev_list);
	    BT_AUD_CHK_FAIL_RET(a_mtkapi_a2dp_sink_get_dev_list, i4_ret, BT_AUD_FAIL);
	}
	else if (BT_A2DP_ROLE_SRC== g_cur_a2dp_role)
	{
	    i4_ret = a_mtkapi_a2dp_src_get_dev_list(pt_dev_list);
	    BT_AUD_CHK_FAIL_RET(a_mtkapi_a2dp_sink_get_dev_list, i4_ret, BT_AUD_FAIL);
	}

    DBG_API((BTAUD_TAG"historylist has %d %s paired devices.\r\n",
						pt_dev_list->dev_num,a2dp_device[g_cur_a2dp_role]));

	BT_AUD_FUNC_EXIT();
    return BT_AUD_OK;
}

BT_AUD_BLUETOOTH_STATUS_T* _bluetooth_get_bt_status(VOID)
{
    return &g_t_bt_aud_sta;
}

INT32 _bluetooth_set_local_name(const CHAR* pc_btname)
{
    INT32 i4_ret;
    CHAR ac_newname[MAX_NAME_LEN]={0};

    if(FALSE == _bluetooth_is_bt_power_on())
    {
        DBG_ERROR((BTAUD_TAG"Err: BT has not power on!\r\n"));
        return BT_AUD_FAIL;
    }

    strncpy(ac_newname ,pc_btname, MAX_NAME_LEN);
    ac_newname[MAX_NAME_LEN - 1] = '\0';

    i4_ret = a_mtkapi_bt_gap_set_name(ac_newname);
	DBG_ERROR((BTAUD_TAG"a_mtkapi_bt_gap_set_name ,%s \n",ac_newname));
    BT_AUD_CHK_FAIL_RET(a_mtkapi_bt_gap_set_name, i4_ret, BT_AUD_FAIL);

    //DBG_API((BTAUD_TAG"Set BT name successfully, newname is %s.\r\n", ac_newname));
    DBG_ERROR((BTAUD_TAG"Set BT name successfully, newname is %s.\r\n", ac_newname));

    return BT_AUD_OK;
}

INT32 u_bluetooth_set_bt_name(const CHAR *setname)
{
    INT32 i4_ret;
	INT32 i4_length = 0;
    CHAR ac_oldname[MAX_NAME_LEN]={0};
    FILE *pf_bt = NULL;
    BT_LOCAL_DEV ps_dev_info;

    if(FALSE == _bluetooth_is_bt_power_on())
    {
        DBG_ERROR((BTAUD_TAG"Err: BT has not power on!\r\n"));
        return BT_AUD_FAIL;
    }

    i4_ret = a_mtkapi_bt_gap_get_local_dev_info(&ps_dev_info);
    BT_AUD_CHK_FAIL(a_mtkapi_bt_gap_get_local_dev_info, i4_ret);

	DBG_ERROR((BTAUD_TAG" ====ASSISTANT_STUB=====ps_dev_info.name : %s \r\n",ps_dev_info.name));
	strcpy(ac_oldname,ps_dev_info.name);
	DBG_ERROR((BTAUD_TAG" ====ASSISTANT_STUB=====ac_oldname : %s \r\n",ac_oldname));

    i4_ret = _bluetooth_set_local_name(setname);
    BT_AUD_CHK_FAIL_RET(_bluetooth_set_local_name, i4_ret, BT_AUD_FAIL);

    DBG_API((BTAUD_TAG"====ASSISTANT_STUB==== BT name modified successfully.\r\n"
        "\toldname:%s\r\n"
        "\tnewname:%s\r\n", ac_oldname, setname));

	if (0 == access(BLUETOOTH_NAME_SAVE_PATH, F_OK))  //check the file exist or not
    {
        DBG_ERROR((BTAUD_TAG"====ASSISTANT_STUB====/data/misc/bluetooth_name.txt exist \n"));

		pf_bt = fopen(BLUETOOTH_NAME_SAVE_PATH, "w"); //clear the file
        if (NULL == pf_bt)
        {
            DBG_ERROR((BTAUD_TAG"====ASSISTANT_STUB====Open file /data/misc/bluetooth_name.txt failed!\n"));
            return BT_AUD_FAIL;
        }

        i4_length = fwrite("1", 1, 1, pf_bt);  //write the flag to the file
        DBG_ERROR((BTAUD_TAG"====ASSISTANT_STUB====FLAG--fwrite length:%d\n",i4_length));

		i4_length = fwrite(setname, strlen(setname), 1, pf_bt);  //write the BT name to the file
        DBG_ERROR((BTAUD_TAG"====ASSISTANT_STUB====fwrite length:%d\n",i4_length));

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
       DBG_ERROR((BTAUD_TAG"ASSISTANT_STUB---no file exist !!!!\n"));
	}
    //end add
    return BT_AUD_OK;
}

INT32 _bluetooth_set_name_assistant_stub(BYTE *uc_usr_msg)
{
    BT_AUD_FUNC_ENTRY();
	INT32 i4_ret;
	ASSISTANT_STUB_SET_BT_NAME_T *set_bt_name;
	set_bt_name = (ASSISTANT_STUB_SET_BT_NAME_T*)uc_usr_msg;

	DBG_ERROR((BTAUD_TAG" ====ASSISTANT_STUB=== command : %s \n",set_bt_name->command));
	DBG_ERROR((BTAUD_TAG" ====ASSISTANT_STUB=== bt_name : %s \n",set_bt_name->name));

	i4_ret = u_bluetooth_set_bt_name(set_bt_name->name);
	BT_AUD_CHK_FAIL_RET(u_bluetooth_set_bt_name, i4_ret, BT_AUD_FAIL);
	return BT_AUD_OK;
}

BOOL _bluetooth_is_bt_profile_connected(VOID)
{
	if ((TRUE == g_t_bt_aud_sta.b_bt_a2dp_connect) ||
		(TRUE == g_t_bt_aud_sta.b_bt_avrcp_connect))
#if CONFIG_SUPPORT_BT_HFP
	if ((TRUE == g_t_bt_aud_sta.b_bt_a2dp_connect) ||
		(TRUE == g_t_bt_aud_sta.b_bt_avrcp_connect) ||
		(TRUE == g_t_bt_aud_sta.b_bt_hfp_connect))
#endif /*CONFIG_SUPPORT_BT_HFP*/
	{
		return TRUE;
	}

	return FALSE;
}

BOOL _bluetooth_is_bt_connected(VOID)
{
	switch(g_t_bt_aud_sta.e_bt_sta)
    {
        case BT_AUD_BT_STA_CONNECTED:
        case BT_AUD_BT_STA_PLAYING:
            return TRUE;

        default:
            return FALSE;
    }
}

BOOL _bluetooth_is_bt_ready(VOID)
{
    if ((g_t_bt_aud_sta.b_bt_open == TRUE) && (TRUE == g_t_bt_aud_sta.b_bt_init))
    {
        return TRUE;
    }
    return FALSE;
}

#if CONFIG_APP_SUPPORT_BT_BACKGROUND_CONNECTABLE
BOOL _bluetooth_is_connectable_background(VOID)
{
    return g_t_bt_aud_sta.b_connectable_background;
}
INT32 _bluetooth_set_connectable_background(BOOL b_connectable)
{
    g_t_bt_aud_sta.b_connectable_background = b_connectable;
    DBG_API((BTAUD_TAG"BT connectable background is %s.\r\n",
        ((g_t_bt_aud_sta.b_connectable_background == TRUE)?"on":"off")));
    return BT_AUD_OK;
}
#endif/* CONFIG_APP_SUPPORT_BT_BACKGROUND_CONNECTABLE */

/*----------------------------------------------------------------------------
 * Name         - u_bluetooth_clear_bluetooth_data
 * Description  - clear bluetooth data
 * Returns      - 0:success
----------------------------------------------------------------------------*/
INT32 u_bluetooth_clear_bluetooth_data(VOID)
{
    INT32   i4_ret = 0;
	FILE *pf_bt = NULL;
	BT_A2DP_DEVICE_LIST t_bt_pair_dev_list;
	INT32 i;

    BT_AUD_FUNC_ENTRY();

    DBG_API((BTAUD_TAG"start to clear user bluetooth data...\r\n"));

    if(TRUE == _bluetooth_is_bt_power_on())//when BT poweron, invoke next APIs to make sure BT work well without reset
    {
        // disconnect BT before initialize personal information
        if (TRUE == _bluetooth_is_bt_connected())
        {
            i4_ret = _bluetooth_bt_disconnect(FALSE);
            BT_AUD_CHK_FAIL(_bluetooth_bt_disconnect, i4_ret);

          u_thread_delay(300);
        }
#if 0
		//get the paired device list
        i4_ret = _bluetooth_paired_dev_list_update(&t_bt_pair_dev_list);
	    BT_AUD_CHK_FAIL_RET(_bluetooth_paired_dev_list_update, i4_ret, BT_AUD_FAIL);

		for(i=0;i<t_bt_pair_dev_list.dev_num;i++)
		{
            // remove all the stack of paired relations
            i4_ret = a_mtkapi_bt_gap_paired_dev_erase(t_bt_pair_dev_list.dev[i].addr);
            BT_AUD_CHK_FAIL(a_mtkapi_bt_gap_paired_dev_erase, i4_ret);

			DBG_ERROR((BTAUD_TAG"a_mtkapi_bt_gap_paired_dev_erase num: %d, device address: %s",i,t_bt_pair_dev_list.dev[i].addr));
		}

        // remove all devices from paired device list
        i4_ret = a_mtkapi_bt_gap_del_paired_av_dev_all();   //???
        BT_AUD_CHK_FAIL(a_mtkapi_bt_gap_del_paired_av_dev_all, i4_ret);

		// clear device information
        i4_ret = a_mtkapi_bt_gap_clear_dev_info();
        BT_AUD_CHK_FAIL(x_mtkapi_bt_gap_clear_dev_info, i4_ret);

        i4_ret = a_mtkapi_bt_gap_save_device_history();
        BT_AUD_CHK_FAIL(a_mtkapi_bt_gap_save_device_history, i4_ret);
#endif
        i4_ret = a_mtkapi_bt_bluetooth_factory_reset();
        BT_AUD_CHK_FAIL(a_mtkapi_bt_bluetooth_factory_reset, i4_ret);
    }

    pf_bt = fopen(BLUETOOTH_NAME_SAVE_PATH, "w");  //clear the file
    if (NULL == pf_bt)
    {
        DBG_ERROR((BTAUD_TAG"CLI---Open file /data/misc/bluetooth_name.txt failed!\n"));
        return BT_AUD_FAIL;
    }
	fclose(pf_bt);
    //end add

    // clear bluetooth data used by Blueangel
//    ipcd_exec("rm -r "BLUETOOTH_LOCAL_FOLDER, NULL);

    BT_AUD_FUNC_EXIT();
    return BT_AUD_OK;
}

INT32 a_bluetooth_get_local_mac_addr(CHAR* pc_mac)
{
    INT32 i4_ret;
    BT_LOCAL_DEV t_local_info;

    if(FALSE == _bluetooth_is_bt_power_on())
    {
        DBG_ERROR((BTAUD_TAG"Err: BT has not power on!\r\n"));
        return BT_AUD_FAIL;
    }

    if (NULL == pc_mac)
    {
        DBG_ERROR((BTAUD_TAG"Err: func:%s line:%d. Get a null pointer!\r\n", __FUNCTION__, __LINE__));
        return BT_AUD_FAIL;
    }

    memset(&t_local_info, 0, sizeof(BT_LOCAL_DEV));
    i4_ret = a_mtkapi_bt_gap_get_local_dev_info(&t_local_info);
    BT_AUD_CHK_FAIL_RET(a_mtkapi_bt_gap_get_local_dev_info, i4_ret, BT_AUD_FAIL);

    strcpy(pc_mac, t_local_info.bdAddr);
    DBG_INFO((BTAUD_TAG"get local bluetooth MAC address: %s\r\n", t_local_info.bdAddr));

    return BT_AUD_OK;
}

INT32 _bluetooth_bt_reconnect_assistant(VOID)
{
    INT32 i4_ret;
	BT_AUD_FUNC_ENTRY();

	DBG_ERROR((BTAUD_TAG" _bluetooth_bt_reconnect_assistant \r\n"));

	i4_ret = _bluetooth_profiles_disconnect();
	BT_AUD_CHK_FAIL_RET(_bluetooth_profiles_disconnect, i4_ret, BT_AUD_FAIL);
#if 0
	i4_ret = a_mtkapi_bt_gap_set_connectable_and_discoverable(TRUE, TRUE);
    BT_AUD_CHK_FAIL(a_mtkapi_bt_gap_set_connectable_and_discoverable, i4_ret);

	g_cur_gap_scan_mode = 2;
#endif
	return BT_AUD_OK;
}

INT32 _bluetooth_get_local_mac_from_nvram(CHAR *local_mac)
{
    INT32 i4_ret;
    CHAR ac_bdAddr[MAX_BDADDR_LEN]={0};
    FILE *pf_bt_tmp = NULL;
    INT32 flen = 0;

	BT_AUD_FUNC_ENTRY();

    DBG_ERROR((BTAUD_TAG"get local mac from NVRAM \r\n"));

    if (NULL == local_mac)
    {
        DBG_ERROR((BTAUD_TAG"Err: func:%s line:%d. Get a null pointer!\r\n", __FUNCTION__, __LINE__));
        return BT_AUD_FAIL;
    }

    //get local mac address
    system("wfbt_wr bt r_mac > /tmp/btmac");
    pf_bt_tmp = fopen("/tmp/btmac", "r+");
    if (NULL == pf_bt_tmp)
    {
        DBG_ERROR((BTAUD_TAG"===Assistant===Open file /tmp/btmac failed!\n"));
        return NULL;
    }
    fseek(pf_bt_tmp,0,SEEK_END); //move to the end of the file
    flen = ftell(pf_bt_tmp);     //get the size of the file
    fseek(pf_bt_tmp,19,SEEK_SET); //move to the pos of name
    fread(ac_bdAddr,flen-19,1,pf_bt_tmp);

    DBG_ERROR((BTAUD_TAG"BT NVRAM MAC : %s\r\n", ac_bdAddr));

    strcpy(local_mac, ac_bdAddr);
    DBG_ERROR((BTAUD_TAG"get local bluetooth MAC address: %s\r\n", local_mac));

    BT_AUD_FUNC_EXIT();
	return BT_AUD_OK;
}

ASSISTANT_STUB_BLUETOOTH_STATUS_CHANGE_T* u_bluetooth_get_bt_status_assistant(VOID)
{
    INT32 i4_ret;
    CHAR ac_name[MAX_NAME_LEN]={0};
    CHAR ac_bdAddr[MAX_BDADDR_LEN]={0};
    CHAR bt_mac_suf[MAX_BDADDR_LEN]={0};
    CHAR device_name[MAX_NAME_LEN]={0};

	BT_LOCAL_DEV ps_dev_info;
    INT32 flen = 0;
    FILE *pf_bt = NULL;
    struct timeval sysTime = {0};
    FILE *pf_bt_tmp = NULL;
    //BT_A2DP_DEVICE_LIST pt_target_dev_info;

	BT_AUD_FUNC_ENTRY();

//    gettimeofday(&sysTime, NULL);
//    DBG_ERROR((BTAUD_TAG "%s,%d,Time start:(%lu.%03lu)s \n",__FUNCTION__,__LINE__,sysTime.tv_sec,sysTime.tv_usec/1000));

    if (0 == access(BLUETOOTH_NAME_SAVE_PATH, F_OK))
    {
        DBG_ERROR((BTAUD_TAG"===ASSISTANT===/data/misc/bluetooth_name.txt exist \n"));

		pf_bt = fopen(BLUETOOTH_NAME_SAVE_PATH, "r+");
        if (NULL == pf_bt)
        {
            DBG_ERROR((BTAUD_TAG"===Assistant===Open file /data/misc/bluetooth_name.txt failed!\n"));
            return NULL;
        }

		if(EOF == fgetc(pf_bt))   //the file is empty
		{
            DBG_ERROR((BTAUD_TAG"===Assistant===the file is empty!!! \n"));
			fclose(pf_bt);

//            gettimeofday(&sysTime, NULL);
//            DBG_ERROR((BTAUD_TAG "%s,%d,Time start:(%lu.%03lu)s \n",__FUNCTION__,__LINE__,sysTime.tv_sec,sysTime.tv_usec/1000));

            //get local mac address
            system("wfbt_wr bt r_mac > /tmp/btmac");
            pf_bt_tmp = fopen("/tmp/btmac", "r+");
            if (NULL == pf_bt_tmp)
            {
                DBG_ERROR((BTAUD_TAG"===Assistant===Open file /tmp/btmac failed!\n"));
                return NULL;
            }
            fseek(pf_bt_tmp,0,SEEK_END); //move to the end of the file
	        flen = ftell(pf_bt_tmp);     //get the size of the file
	        fseek(pf_bt_tmp,19,SEEK_SET); //move to the pos of name
	        fread(ac_bdAddr,flen-19,1,pf_bt_tmp);
            fclose(pf_bt_tmp);

            DBG_ERROR((BTAUD_TAG"BT MAC : %s\r\n", ac_bdAddr));

            //only use the last 5 characters of the bluetooth device mac address
    		strncpy(bt_mac_suf,ac_bdAddr+strlen(ac_bdAddr)-BT_NAME_SUF_LEN-1,BT_NAME_SUF_LEN);
            DBG_ERROR((BTAUD_TAG"bt_mac_suf : %s\r\n", bt_mac_suf));

    		strcpy(ac_name,BLUETOOTH_NAME_PRE);
    		DBG_ERROR((BTAUD_TAG"PRE ac_name : %s\r\n", ac_name));

            //the bluetooth name: ac_name = BLUETOOTH_NAME_PRE + (bt_mac_suf)
            strcat(ac_name,"(");
    		strcat(ac_name,bt_mac_suf);
    		strcat(ac_name,")");
    		DBG_ERROR((BTAUD_TAG"the whole ac_name : %s\r\n", ac_name));

//            gettimeofday(&sysTime, NULL);
//            DBG_ERROR((BTAUD_TAG "%s,%d,Time start:(%lu.%03lu)s \n",__FUNCTION__,__LINE__,sysTime.tv_sec,sysTime.tv_usec/1000));

            return &g_t_bt_aud_bt_status;
        }
        else
        {
            fseek(pf_bt,0,SEEK_END); //move to the end of the file
    	    flen = ftell(pf_bt);     //get the size of the file
    	    fseek(pf_bt,1,SEEK_SET); //move to the pos of name
    	    fread(ac_name,flen-1,1,pf_bt);

            DBG_ERROR((BTAUD_TAG"===ASSISTANT_STUB GET BT STATUS===fread---the ac_name : %s\r\n", ac_name));
            fclose(pf_bt);
        }
    }
    else
    {
        DBG_ERROR((BTAUD_TAG"===ASSISTANT_STUB GET BT STATUS===/data/misc/bluetooth_name.txt is not exist \n"));

//        gettimeofday(&sysTime, NULL);
//        DBG_ERROR((BTAUD_TAG "%s,%d,Time start:(%lu.%03lu)s \n",__FUNCTION__,__LINE__,sysTime.tv_sec,sysTime.tv_usec/1000));

        //get local mac address
        system("wfbt_wr bt r_mac > /tmp/btmac");
        pf_bt_tmp = fopen("/tmp/btmac", "r+");
        if (NULL == pf_bt_tmp)
        {
            DBG_ERROR((BTAUD_TAG"===Assistant===Open file /tmp/btmac failed!\n"));
            return NULL;
        }
        fseek(pf_bt_tmp,0,SEEK_END); //move to the end of the file
        flen = ftell(pf_bt_tmp);     //get the size of the file
        fseek(pf_bt_tmp,19,SEEK_SET); //move to the pos of name
        fread(ac_bdAddr,flen-19,1,pf_bt_tmp);
        fclose(pf_bt_tmp);

        DBG_ERROR((BTAUD_TAG"BT MAC : %s\r\n", ac_bdAddr));

        //only use the last 5 characters of the bluetooth device mac address
		strncpy(bt_mac_suf,ac_bdAddr+strlen(ac_bdAddr)-BT_NAME_SUF_LEN-1,BT_NAME_SUF_LEN);
        DBG_ERROR((BTAUD_TAG"bt_mac_suf : %s\r\n", bt_mac_suf));

		strcpy(ac_name,BLUETOOTH_NAME_PRE);
		DBG_ERROR((BTAUD_TAG"PRE ac_name : %s\r\n", ac_name));

        //the bluetooth name: ac_name = BLUETOOTH_NAME_PRE + (bt_mac_suf)
        strcat(ac_name,"(");
		strcat(ac_name,bt_mac_suf);
		strcat(ac_name,")");
		DBG_ERROR((BTAUD_TAG"the whole ac_name : %s\r\n", ac_name));

//        gettimeofday(&sysTime, NULL);
//        DBG_ERROR((BTAUD_TAG "%s,%d,Time start:(%lu.%03lu)s \n",__FUNCTION__,__LINE__,sysTime.tv_sec,sysTime.tv_usec/1000));
    }

//    gettimeofday(&sysTime, NULL);
//    DBG_ERROR((BTAUD_TAG "%s,%d,Time start:(%lu.%03lu)s \n",__FUNCTION__,__LINE__,sysTime.tv_sec,sysTime.tv_usec/1000));

    DBG_ERROR((BTAUD_TAG" ====ASSISTANT_STUB GET BT STATUS=====ac_name : %s \r\n",ac_name));
    strcpy(g_t_bt_aud_bt_status.name,ac_name);

//    gettimeofday(&sysTime, NULL);
//    DBG_ERROR((BTAUD_TAG "%s,%d,Time start:(%lu.%03lu)s \n",__FUNCTION__,__LINE__,sysTime.tv_sec,sysTime.tv_usec/1000));
#if 0
    memset(&pt_target_dev_info, 0, sizeof(BT_TARGET_DEV_INFO));
    i4_ret = a_mtkapi_bt_gap_get_target_info(&pt_target_dev_info);
    BT_AUD_CHK_FAIL_RET(a_mtkapi_bt_gap_get_target_info, i4_ret, BT_AUD_FAIL);

    strcpy(g_t_bt_aud_bt_status.bt_paired_name, pt_target_dev_info.name);
    DBG_ERROR((BTAUD_TAG" ====ALI_STUB GET BT STATUS=====g_t_bt_aud_bt_status.bt_paired_name : %s \r\n",g_t_bt_aud_bt_status.bt_paired_name));
#endif
//    gettimeofday(&sysTime, NULL);
//    DBG_ERROR((BTAUD_TAG "%s,%d,Time start:(%lu.%03lu)s \n",__FUNCTION__,__LINE__,sysTime.tv_sec,sysTime.tv_usec/1000));

    return &g_t_bt_aud_bt_status;
}

ASSISTANT_STUB_PLAYER_STATUS_CHANGE_T* u_bluetooth_get_player_status(VOID)
{
    INT32 i4_ret;
	UINT8 get_volume;
	BT_AUD_FUNC_ENTRY();

	i4_ret = u_acfg_get_volume(&get_volume);
	if(APP_CFGR_OK != i4_ret)
		{
			DBG_ERROR((BTAUD_TAG"Err: u_acfg_get_volume fail, errno=%d\n", i4_ret));
			return i4_ret;
		}
	DBG_ERROR((BTAUD_TAG "volume get from acfg is : %d \n",get_volume));

    g_t_bt_aud_play_inform.player.progress = 0;
    g_t_bt_aud_play_inform.player.volume = get_volume;
	strncpy(g_t_bt_aud_play_inform.player.source,"bluetooth",ASSISTANT_STUB_SOURCE_MAX_LENGTH);
	strncpy(g_t_bt_aud_play_inform.command,"/playback/player_status_change",ASSISTANT_STUB_COMMAND_MAX_LENGTH);

	DBG_API((BTAUD_TAG" ====ASSISTANT_STUB=====g_t_bt_aud_play_inform.command : %s \r\n",g_t_bt_aud_play_inform.command));
	DBG_API((BTAUD_TAG" ====ASSISTANT_STUB=====g_t_bt_aud_play_inform.player.volume : %d \r\n",g_t_bt_aud_play_inform.player.volume));
	DBG_API((BTAUD_TAG" ====ASSISTANT_STUB=====g_t_bt_aud_play_inform.player.source : %s \r\n",g_t_bt_aud_play_inform.player.source));

	return &g_t_bt_aud_play_inform;
}

/*send player information to assistant_stub */
INT32 _bluetooth_send_player_status_to_assistant_stub(VOID)
{
    DBG_ERROR(("<BT_AUD> %s\n",__FUNCTION__));
	BT_AUD_FUNC_ENTRY();

	INT32 i4_ret = BT_AUD_FAIL;
    HANDLE_T h_app = NULL_HANDLE;

	ASSISTANT_STUB_PLAYER_STATUS_CHANGE_T *bt_player_inform = {0};

    bt_player_inform = u_bluetooth_get_player_status();

    DBG_API((BTAUD_TAG" ====ASSISTANT_STUB=====bt_player_inform.command : %s \r\n",bt_player_inform->command));
	DBG_API((BTAUD_TAG" ====ASSISTANT_STUB=====bt_player_inform.player.volume : %d \r\n",bt_player_inform->player.volume));
	DBG_API((BTAUD_TAG" ====ASSISTANT_STUB=====bt_player_inform.player.status : %s \r\n",bt_player_inform->player.status));
	DBG_API((BTAUD_TAG" ====ASSISTANT_STUB=====bt_player_inform.player.source : %s \r\n",bt_player_inform->player.source));
	DBG_API((BTAUD_TAG" ====ASSISTANT_STUB=====bt_player_inform.player.progress : %d \r\n",bt_player_inform->player.progress));

    u_am_get_app_handle_from_name(&h_app,ASSISTANT_STUB_THREAD_NAME);

    i4_ret = u_app_send_appmsg(h_app,
                                E_APP_MSG_TYPE_ASSISTANT_STUB,
                                MSG_FROM_BT,
                                ASSISTANT_STUB_CMD_PLAYER_STATUS_CHANGE,
                                bt_player_inform,
                                sizeof(ASSISTANT_STUB_PLAYER_STATUS_CHANGE_T));
    if (AEER_OK != i4_ret) {
        DBG_ERROR((BTAUD_TAG"\n bluetooth send player informtion to assistant_stub failed, i4_ret[%ld]\n",i4_ret));
    }

    return (i4_ret == AEER_OK ? BT_AUD_OK : BT_AUD_FAIL);
}

INT32 _bluetooth_wifi_status_msg_proc(APPMSG_T* pv_msg)
{
	INT32 i4_ret;
	BT_AUD_FUNC_ENTRY();

	switch(pv_msg->ui4_msg_type)
	{
		case ASSISTANT_STUB_CMD_WIFI_SETUP_RESULT:
		{
            DBG_ERROR((BTAUD_TAG"wifi setting done!!\r\n"));
            DBG_ERROR((BTAUD_TAG"_bluetooth_is_bt_connected : %d ,b_bt_reconnect_for_wifi : %d \r\n",
                                _bluetooth_is_bt_connected(),g_t_bt_aud_sta.b_bt_reconnect_for_wifi));
            if (FALSE == _bluetooth_is_bt_connected())
            {
                if(TRUE == g_t_bt_aud_sta.b_bt_reconnect_for_wifi)
                {
                    DBG_ERROR((BTAUD_TAG"BT connet to the last device....\r\n"));

                    g_t_bt_aud_sta.b_bt_reconnect_for_wifi = FALSE;

                    i4_ret = _bluetooth_paired_dev_list_update(&g_t_bt_pair_dev_list);
                    if ((i4_ret == BT_AUD_OK) || (g_t_bt_pair_dev_list.dev_num != 0))
                    {
                        g_t_bt_aud_sta.b_continue_connect = FALSE;
                        i4_ret = _bluetooth_auto_connect(1, TRUE);
                        if (i4_ret == BT_AUD_OK)
                        {
                            break;
                        }
                    }
                }
                else
                {
                    DBG_ERROR((BTAUD_TAG"No devices connected before wifi_setting , no need to reconnect. \r\n"));
                }
            }
            else
            {
                DBG_ERROR((BTAUD_TAG"BT is conneted.\r\n"));
            }
		}
	    break;
		default:
			DBG_ERROR((BTAUD_TAG"Err: unkown WIFI msg, ui4_msgtype=%d.\r\n", pv_msg->ui4_msg_type));
            break;
    }
	BT_AUD_FUNC_EXIT();
	return BT_AUD_OK;
}

INT32 _bluetooth_disconnect_assistant_stub_msg_proc(BOOL b_keep_connect)
{
	BT_AUD_FUNC_ENTRY();

    INT32 i4_ret;

    i4_ret = _bluetooth_bt_disconnect(b_keep_connect);
	BT_AUD_CHK_FAIL_RET(_bluetooth_bt_disconnect, i4_ret, BT_AUD_FAIL);

    BT_AUD_FUNC_EXIT();
    return BT_AUD_OK;

err_out:
    //open bt err then close bt and notify state manager  fail msg
    DBG_ERROR((BTAUD_TAG"Err: bluetooth disconnect fail!\r\n"));

    i4_ret = _bluetooth_bt_close(FALSE);
    BT_AUD_CHK_FAIL(_bluetooth_bt_close, i4_ret);

    i4_ret = _bluetooth_send_inform_stop_to_sm();
    BT_AUD_CHK_FAIL(_bluetooth_send_inform_stop_to_sm, i4_ret);
    return BT_AUD_FAIL;
}

INT32 _bluetooth_start_play_assistant_stub_msg_proc(VOID)
{
    DBG_ERROR(("<BT_AUD> %s\n",__FUNCTION__));
	BT_AUD_FUNC_ENTRY();

    INT32 i4_ret;

    if(TRUE == g_t_bt_aud_sta.b_bt_a2dp_connect)
    {
        i4_ret = _bluetooth_send_avrcp_play_proc();
        BT_AUD_CHK_FAIL(_bluetooth_send_avrcp_play_proc, i4_ret);
    }
    else
    {
        DBG_ERROR((BTAUD_TAG"BT is disconnected \r\n"));
    }

    BT_AUD_FUNC_EXIT();
    return BT_AUD_OK;
}

INT32 _bluetooth_start_pairing_assistant_stub_msg_proc(VOID)
{
    DBG_ERROR(("<BT_AUD> %s\n",__FUNCTION__));
	BT_AUD_FUNC_ENTRY();

    INT32 i4_ret;

    if (TRUE == g_t_bt_aud_sta.b_enter_pairing_enable)
    {
        _bluetooth_stop_enter_pairing_timer();
    }

    _bluetooth_start_enter_pairing_timer();

    i4_ret = _bluetooth_enter_pairing_key_proc();
    BT_AUD_CHK_FAIL(_bluetooth_enter_pairing_key_proc, i4_ret);

    BT_AUD_FUNC_EXIT();
    return BT_AUD_OK;
}

INT32 _bluetooth_del_bt_paired_assistant_stub_msg_proc(VOID)
{
    INT32 i4_ret = BT_AUD_FAIL;

    BT_AUD_FUNC_ENTRY();

    i4_ret = _bluetooth_del_bt_paired_proc();
    BT_AUD_CHK_FAIL(_bluetooth_del_bt_paired_proc, i4_ret);

    BT_AUD_FUNC_EXIT();
    return BT_AUD_OK;
}

INT32 _bluetooth_bt_power_off_assistant_stub_msg_proc(VOID)
{
    INT32 i4_ret = BT_AUD_FAIL;

    BT_AUD_FUNC_ENTRY();

    i4_ret = _bluetooth_set_bt_power(FALSE);
    DBG_ERROR((BTAUD_TAG" set bt power off."));

    BT_AUD_FUNC_EXIT();
    return BT_AUD_OK;
}

INT32 _bluetooth_play_next_assistant_stub_msg_proc(VOID)
{
    INT32 i4_ret = BT_AUD_FAIL;

	BT_AUD_FUNC_ENTRY();

    if(TRUE == g_t_bt_aud_sta.b_bt_a2dp_connect)
    {
        i4_ret = _bluetooth_send_avrcp_next_proc();
        BT_AUD_CHK_FAIL(_bluetooth_send_avrcp_next_proc, i4_ret);
    }
    else
    {
        DBG_ERROR((BTAUD_TAG"BT is disconnected \r\n"));
    }

    BT_AUD_FUNC_EXIT();
    return BT_AUD_OK;
}

INT32 _bluetooth_play_prev_assistant_stub_msg_proc(VOID)
{
    INT32 i4_ret = BT_AUD_FAIL;

	BT_AUD_FUNC_ENTRY();

    if(TRUE == g_t_bt_aud_sta.b_bt_a2dp_connect)
    {
        i4_ret = _bluetooth_send_avrcp_prev_proc();
        BT_AUD_CHK_FAIL(_bluetooth_send_avrcp_prev_proc, i4_ret);
    }
    else
    {
        DBG_ERROR((BTAUD_TAG"BT is disconnected \r\n"));
    }

    BT_AUD_FUNC_EXIT();
    return BT_AUD_OK;
}

INT32 _bluetooth_assistant_stub_msg_proc(APPMSG_T* pv_msg)
{
	INT32 i4_ret;
	BT_AUD_FUNC_ENTRY();

	switch(pv_msg->ui4_msg_type)
	{
		case ASSISTANT_STUB_CMD_SET_BT_NAME:
		{
		    DBG_ERROR((BTAUD_TAG"Handle ASSISTANT_STUB msg : set BT name \n"));
			i4_ret = _bluetooth_set_name_assistant_stub(pv_msg->p_usr_msg);
			BT_AUD_CHK_FAIL_RET(_bluetooth_assistant_stub_msg_proc, i4_ret, BT_AUD_FAIL);
		}
	    break;

        case ASSISTANT_STUB_CMD_START_BT_PAIR:
        {
            DBG_ERROR((BTAUD_TAG "Handle ASSISTANT_STUB msg :ASSISTANT_STUB_CMD_START_BT_PAIR \n"));
            i4_ret = _bluetooth_start_pairing_assistant_stub_msg_proc();
            BT_AUD_CHK_FAIL_RET(_bluetooth_start_pairing_assistant_stub_msg_proc, i4_ret, BT_AUD_FAIL);
        }
        break;

        case ASSISTANT_STUB_CMD_DEL_BT_PAIRED:
        {
            DBG_ERROR((BTAUD_TAG "Handle ASSISTANT_STUB msg :ASSISTANT_STUB_CMD_DEL_PAIRED \n"));
            i4_ret = _bluetooth_del_bt_paired_assistant_stub_msg_proc();
            BT_AUD_CHK_FAIL_RET(_bluetooth_del_bt_paired_assistant_stub_msg_proc, i4_ret, BT_AUD_FAIL);
        }
        break;

        case ASSISTANT_STUB_CMD_BT_POWER_OFF:
        {
            DBG_ERROR((BTAUD_TAG "Handle ASSISTANT_STUB msg :ASSISTANT_STUB_CMD_BT_POWER_OFF \n"));

            if(TRUE == _bluetooth_is_bt_power_on())
            {
                i4_ret = _bluetooth_bt_power_off_assistant_stub_msg_proc();
                BT_AUD_CHK_FAIL_RET(_bluetooth_bt_power_off_assistant_stub_msg_proc, i4_ret, BT_AUD_FAIL);
             }
            else
            {
                DBG_ERROR((BTAUD_TAG "BT has power offed , ignore POWER_OFF msg \n"));
            }
         }
        break;

        case ASSISTANT_STUB_CMD_BT_DISCONNECT:
        {
            DBG_ERROR((BTAUD_TAG "Handle ASSISTANT_STUB msg :ASSISTANT_STUB_CMD_BT_DISCONNECT \n"));

            if(TRUE == g_t_bt_aud_sta.b_bt_a2dp_connect)
            {
                i4_ret = _bluetooth_disconnect_assistant_stub_msg_proc(FALSE);
                BT_AUD_CHK_FAIL_RET(_bluetooth_disconnect_assistant_stub_msg_proc, i4_ret, BT_AUD_FAIL);
            }
            else
            {
                DBG_ERROR((BTAUD_TAG "BT is disconnected , ignore DISCONNECT msg \n"));
            }
        }
        break;
#if 0
		case ALI_STUB_CMD_PLAY_PREV_AUDIO:
		{
			DBG_ERROR((BTAUD_TAG "Handle ALI_STUB msg :ALI_STUB_CMD_PLAY_PREV_AUDIO \n"));
		    i4_ret = _bt_aud_play_prev_assistant_stub_msg_proc();
			BT_AUD_CHK_FAIL_RET(_bt_aud_play_prev_assistant_stub_msg_proc, i4_ret, BT_AUD_FAIL);
		}
	    break;

		case ALI_STUB_CMD_PLAY_NEXT_AUDIO:
		{
			DBG_ERROR((BTAUD_TAG "Handle ALI_STUB msg :ALI_STUB_CMD_PLAY_NEXT_AUDIO \n"));
			i4_ret = _bt_aud_play_next_assistant_stub_msg_proc();
			BT_AUD_CHK_FAIL_RET(_bt_aud_play_next_assistant_stub_msg_proc, i4_ret, BT_AUD_FAIL);
		}
		break;

        case ALI_STUB_CMD_PLAY_BT_MUSIC:
        {
            DBG_ERROR((BTAUD_TAG "Handle ALI_STUB msg :ALI_STUB_CMD_PLAY_BT_MUSIC \n"));
            i4_ret = _bt_aud_start_play_assistant_stub_msg_proc();
            BT_AUD_CHK_FAIL_RET(_bluetooth_key_cmd_proc, i4_ret, BT_AUD_FAIL);
        }
        break;
#endif

		default:
			DBG_ERROR((BTAUD_TAG"Err: unkown ASSISTANT_STUB msg, ui4_msgtype=%d.\r\n", pv_msg->ui4_msg_type));
            break;
    }
	BT_AUD_FUNC_EXIT();
	return BT_AUD_OK;
}


