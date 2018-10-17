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

/* application level*/
#include "u_appman.h"
#include "u_aee.h"
#include "u_cli.h"
#include "u_key_def.h"
#include "u_dbg.h"
#include "u_os.h"
#include "u_amb.h"
#include "u_am.h"
#include "u_common.h"
#include "u_sm.h"
#include "u_assistant_stub.h"


/*  btmw  */
#include "mtk_bt_service_gap_wrapper.h"
#include "mtk_bt_service_a2dp_wrapper.h"
#include "mtk_bt_service_avrcp_wrapper.h"


/* private */
#include "bluetooth_audio_proc.h"
#include "bluetooth_audio_key_proc.h"
#include "bluetooth_audio.h"


/*-----------------------------------------------------------------------------
 * structure, constants, macro definitions
 *---------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------
 * variable declarations
 *---------------------------------------------------------------------------*/
//static HANDLE_T h_key_delay_timer = NULL_HANDLE;//timer for key delay process
static BOOL b_key_handle_enable = FALSE;        // enable key handle
BT_TIMER_MSG_E e_bt_key_delay_msg;
static BOOL b_ff_key_press = FALSE;     //FF key pressing
static BOOL b_fr_key_press = FALSE;     //FR key pressing

extern BT_AUD_BLUETOOTH_STATUS_T g_t_bt_aud_sta;
extern BT_A2DP_DEVICE_LIST g_t_bt_pair_dev_list;
extern BT_AUD_OBJ_T g_t_bt_aud;
extern CHAR g_cur_avrcp_addr[18];



/*-----------------------------------------------------------------------------
 * private methods declarations
 *---------------------------------------------------------------------------*/

//-----------------------------------------------------------------------------
INT32 bt_aud_key_timer_create(VOID)
{
    INT32 i4_ret;

    //create key delay timer

	if(g_t_bt_aud.t_key_delay_timer.h_timer == NULL_HANDLE)
	{
	    g_t_bt_aud.t_key_delay_timer.e_flags = X_TIMER_FLAG_ONCE;
        g_t_bt_aud.t_key_delay_timer.ui4_delay = BT_AUD_STREAM_FIRST_CHK_TIMEOUT;//10000ms
        e_bt_key_delay_msg = BT_TIMER_MSG_KEY_DELAY;

        i4_ret = u_timer_create(&g_t_bt_aud.t_key_delay_timer.h_timer);
		BT_AUD_CHK_FAIL(u_timer_create, i4_ret);
 //       u_timer_start(t_g_bt_aud.h_app, &t_g_bt_aud.t_stream_chk_timer, (void *)&e_bt_stream_chk_msg, sizeof(BT_TIMER_MSG_E));
	}

	#if 0
    if (h_key_delay_timer == NULL_HANDLE)
    {
        i4_ret = u_timer_create(&h_key_delay_timer);
        BT_AUD_CHK_FAIL(u_timer_create, i4_ret);
    }
    #endif

    b_key_handle_enable = TRUE;

    return BT_AUD_OK;
}
INT32 bt_aud_key_timer_delete(VOID)
{
    INT32 i4_ret;

    //delete key delay timer
    if (g_t_bt_aud.t_key_delay_timer.h_timer != NULL_HANDLE)
    {
        i4_ret = u_timer_delete(g_t_bt_aud.t_key_delay_timer.h_timer);
        BT_AUD_CHK_FAIL(u_timer_delete, i4_ret);
        g_t_bt_aud.t_key_delay_timer.h_timer = NULL_HANDLE;
    }
    b_key_handle_enable = FALSE;

    return BT_AUD_OK;
}

VOID _bluetooth_key_delay_timer_cb(VOID)
{
    DBG_INFO((BTAUD_TAG"bluetooth key delay time out.\r\n"));
    b_key_handle_enable = TRUE;
}
static INT32 _bt_aud_start_key_delay_timer(VOID)
{
    INT32 i4_ret;
	i4_ret = u_timer_start(g_t_bt_aud.h_app,
			          &g_t_bt_aud.t_key_delay_timer,
			          (void *)&e_bt_key_delay_msg,
			          sizeof(BT_TIMER_MSG_E));
    BT_AUD_CHK_FAIL_RET(u_timer_start, i4_ret, BT_AUD_FAIL);

	#if 0
    i4_ret = u_timer_start(h_key_delay_timer,
                            BT_AUD_KEY_DELAY_TIMEOUT,
                            X_TIMER_FLAG_ONCE,
                            _bluetooth_key_delay_timer_cb,
                            NULL);
    BT_AUD_CHK_FAIL_RET(u_timer_start, i4_ret, BT_AUD_FAIL);
    #endif

    b_key_handle_enable = FALSE;
    return BT_AUD_OK;
}
//return value: OK -- need to continue handle key; FAIL -- no need to handle key
static INT32 _bt_aud_key_pre_proc(UINT32 ui4_key, UINT32 ui4_keysta)
{
	BT_AUD_FUNC_ENTRY();

	DBG_INFO((BTAUD_TAG"_bt_aud_key_pre_proc, keysta=%x keyval=%x.\r\n",
        				ui4_keysta, ui4_key));
	
	switch(ui4_key)
    {
        case BT_AUD_KEY_NEXT:   //next
        case BT_AUD_KEY_PREV:   //prev
        case BT_AUD_KEY_PAUSE:  //pause
        case BT_AUD_KEY_PLAY:   //play
        case BT_AUD_KEY_STOP:   //stop
        case BT_AUD_KEY_PAUSE_PLAY:   //play&pause
            if (ui4_keysta != BT_AUD_KEYSTA_DOWN)   //only care key down
            {
				DBG_INFO((BTAUD_TAG"======debug===== keysta(x)=%x , keysta(d)=%d.\r\n",
        				ui4_keysta,ui4_keysta));
				return BT_AUD_FAIL;
            }
            break;

        case BT_AUD_KEY_VOL_UP:   //volume up
        case BT_AUD_KEY_VOL_DOWN:   //volume down
            if (ui4_keysta == BT_AUD_KEYSTA_UP)   //don't care key up
            {
				DBG_INFO((BTAUD_TAG"Don't care key up.\r\n"));
				return BT_AUD_FAIL;
            }
            else if (ui4_keysta == BT_AUD_KEYSTA_DOWN)
            {
                //delay after handle next key event
                _bt_aud_start_key_delay_timer();
            }
            else
            {
                if (FALSE == b_key_handle_enable)
                {
                    DBG_INFO((BTAUD_TAG"No need to handle key.\r\n"));
                    return BT_AUD_FAIL;
                }
                else
                {
                    DBG_INFO((BTAUD_TAG"Need to handle key.\r\n"));
                    //delay after handle next key event
                    _bt_aud_start_key_delay_timer();
                }
            }
            break;
        case BT_AUD_KEY_FF:   //ff
        case BT_AUD_KEY_FR:   //fr
            if (ui4_keysta == BT_AUD_KEYSTA_REPEAT)//don't care repeat
            {
				DBG_INFO((BTAUD_TAG"ignore KEY_FF/KEY_FR repeat.\r\n"));
				return BT_AUD_FAIL;
            }
            break;
        default:
			DBG_INFO((BTAUD_TAG"can not recognize key value.\r\n"));
            return BT_AUD_FAIL;
    }

    return BT_AUD_OK;
}

/*\fn INT32 _bt_aud_bluetooth_next_key_proc(UINT32 ui4_key, UINT32 ui4_keysta)
    \brief this function is to process next key
    \retval INT32 if BT_AUD_OK, this function is called successful;else, failed
*/
static INT32 _bt_aud_bluetooth_next_key_proc(UINT32 ui4_key, UINT32 ui4_keysta)
{
    INT32 i4_ret = BT_AUD_OK;
    BT_AVRCP_KEY_STATE action;

    BT_AUD_FUNC_ENTRY();

    action = BT_AVRCP_KEY_STATE_PRESS;
    //call btm interface
    i4_ret = a_mtkapi_avrcp_send_passthrough_cmd(g_cur_avrcp_addr, BT_AVRCP_CMD_TYPE_FWD, action);
    BT_AUD_CHK_FAIL_RET(a_mtkapi_avrcp_send_passthrough_cmd, i4_ret, BT_AUD_FAIL);

    action = BT_AVRCP_KEY_STATE_RELEASE;
    i4_ret = a_mtkapi_avrcp_send_passthrough_cmd(g_cur_avrcp_addr, BT_AVRCP_CMD_TYPE_FWD, action);
    BT_AUD_CHK_FAIL_RET(a_mtkapi_avrcp_send_passthrough_cmd, i4_ret, BT_AUD_FAIL);

    BT_AUD_FUNC_EXIT();
    return BT_AUD_OK;
}

/* \fn INT32 _bt_aud_bluetooth_prev_key_proc(UINT32 ui4_key, UINT32 ui4_keysta)
    \brief this function is to process prev key
    \retval INT32 if BT_AUD_OK, this function is called successful;else, failed
*/
static INT32 _bt_aud_bluetooth_prev_key_proc(UINT32 ui4_key, UINT32 ui4_keysta)
{
    INT32     i4_ret = BT_AUD_OK;
    BT_AVRCP_KEY_STATE action;

    BT_AUD_FUNC_ENTRY();

    action = BT_AVRCP_KEY_STATE_PRESS;
    //call btm interface
    i4_ret = a_mtkapi_avrcp_send_passthrough_cmd(g_cur_avrcp_addr, BT_AVRCP_CMD_TYPE_BWD, action);
    BT_AUD_CHK_FAIL_RET(a_mtkapi_avrcp_send_passthrough_cmd, i4_ret, BT_AUD_FAIL);

    action = BT_AVRCP_KEY_STATE_RELEASE;
    //call btm interface
    i4_ret = a_mtkapi_avrcp_send_passthrough_cmd(g_cur_avrcp_addr, BT_AVRCP_CMD_TYPE_BWD, action);
    BT_AUD_CHK_FAIL_RET(a_mtkapi_avrcp_send_passthrough_cmd, i4_ret, BT_AUD_FAIL);

    BT_AUD_FUNC_EXIT();
    return BT_AUD_OK;
}



/*\fn INT32 _bt_aud_bluetooth_pause_key_proc(UINT32 ui4_key, UINT32 ui4_keysta)
    \brief this function is to process pause key
    \retval INT32 if BT_AUD_OK, this function is called successful;else, failed
*/
static INT32 _bt_aud_bluetooth_pause_key_proc(UINT32 ui4_key, UINT32 ui4_keysta)
{
    INT32    i4_ret = BT_AUD_OK;
    BT_AUD_FUNC_ENTRY();

    BT_AVRCP_KEY_STATE action;
    action = BT_AVRCP_KEY_STATE_PRESS;
    i4_ret = a_mtkapi_avrcp_send_passthrough_cmd(g_cur_avrcp_addr, BT_AVRCP_CMD_TYPE_PAUSE, action);
    BT_AUD_CHK_FAIL_RET(a_mtkapi_avrcp_send_passthrough_cmd, i4_ret, BT_AUD_FAIL);

    action = BT_AVRCP_KEY_STATE_RELEASE;
    i4_ret = a_mtkapi_avrcp_send_passthrough_cmd(g_cur_avrcp_addr, BT_AVRCP_CMD_TYPE_PAUSE, action);
    BT_AUD_CHK_FAIL_RET(a_mtkapi_avrcp_send_passthrough_cmd, i4_ret, BT_AUD_FAIL);

    BT_AUD_FUNC_EXIT();
    return BT_AUD_OK;
}


/* \fn INT32 _bt_aud_bluetooth_play_key_proc(UINT32 ui4_key, UINT32 ui4_keysta)
    \brief this function is to process play key
    \retval INT32 if BT_AUD_OK, this function is called successful;else, failed
*/
static INT32 _bt_aud_bluetooth_play_key_proc(UINT32 ui4_key, UINT32 ui4_keysta)
{
    INT32    i4_ret = BT_AUD_OK;
    BT_AVRCP_KEY_STATE action;

    BT_AUD_FUNC_ENTRY();

    //if FF key no release, release now
    if (b_ff_key_press == TRUE)
    {
        DBG_API((BTAUD_TAG"FF key no release, release now.\r\n"));
        action = BT_AVRCP_KEY_STATE_RELEASE;
        i4_ret = a_mtkapi_avrcp_send_passthrough_cmd(g_cur_avrcp_addr, BT_AVRCP_CMD_TYPE_FFWD, action);
        BT_AUD_CHK_FAIL(a_mtkapi_avrcp_send_passthrough_cmd, i4_ret);
        b_ff_key_press = FALSE;
    }

    //if FR key no release, release now
    if (b_fr_key_press == TRUE)
    {
        DBG_API((BTAUD_TAG"FR key no release, release now.\r\n"));
        action = BT_AVRCP_KEY_STATE_RELEASE;
        i4_ret = a_mtkapi_avrcp_send_passthrough_cmd(g_cur_avrcp_addr, BT_AVRCP_CMD_TYPE_RWD, action);
        BT_AUD_CHK_FAIL(a_mtkapi_avrcp_send_passthrough_cmd, i4_ret);
        b_fr_key_press = FALSE;
    }

    action = BT_AVRCP_KEY_STATE_PRESS;
    i4_ret = a_mtkapi_avrcp_send_passthrough_cmd(g_cur_avrcp_addr, BT_AVRCP_CMD_TYPE_PLAY, action);
    BT_AUD_CHK_FAIL_RET(a_mtkapi_avrcp_send_passthrough_cmd, i4_ret, BT_AUD_FAIL);

    action = BT_AVRCP_KEY_STATE_RELEASE;
    i4_ret = a_mtkapi_avrcp_send_passthrough_cmd(g_cur_avrcp_addr, BT_AVRCP_CMD_TYPE_PLAY, action);
    BT_AUD_CHK_FAIL_RET(a_mtkapi_avrcp_send_passthrough_cmd, i4_ret, BT_AUD_FAIL);

    BT_AUD_FUNC_EXIT();
    return BT_AUD_OK;
}
static INT32 _bt_aud_bluetooth_stop_key_proc(UINT32 ui4_key, UINT32 ui4_keysta)
{
    INT32 i4_ret = BT_AUD_OK;
    BT_AVRCP_KEY_STATE action;

    BT_AUD_FUNC_ENTRY();

    action = BT_AVRCP_KEY_STATE_PRESS;
    //call btm interface
    i4_ret = a_mtkapi_avrcp_send_passthrough_cmd(g_cur_avrcp_addr, BT_AVRCP_CMD_TYPE_STOP, action);
    BT_AUD_CHK_FAIL_RET(a_mtkapi_avrcp_send_passthrough_cmd, i4_ret, BT_AUD_FAIL);

    action = BT_AVRCP_KEY_STATE_RELEASE;
    //call btm interface
    i4_ret = a_mtkapi_avrcp_send_passthrough_cmd(g_cur_avrcp_addr, BT_AVRCP_CMD_TYPE_STOP, action);
    BT_AUD_CHK_FAIL_RET(a_mtkapi_avrcp_send_passthrough_cmd, i4_ret, BT_AUD_FAIL);

    BT_AUD_FUNC_EXIT();
    return BT_AUD_OK;
}
static INT32 _bt_aud_bluetooth_play_pause_key_proc(UINT32 ui4_key, UINT32 ui4_keysta)
{
    INT32 i4_ret = BT_AUD_OK;
    static BOOL b_play = FALSE; //save last action

    BT_AUD_FUNC_ENTRY();

    if (g_t_bt_aud_sta.b_play_pause_down == FALSE)//key first down or last action already respond
    {
        if (g_t_bt_aud_sta.b_bt_play == FALSE)//pause,change to play
        {
            b_play = TRUE;
        }
        else//playing ,change to pause
        {
            b_play = FALSE;
        }
    }
    else    //last action not respond, next action need to use opposite action
    {
        DBG_API((BTAUD_TAG"last action no respond, use opposite action.\r\n"));
        b_play = !b_play;
    }

    if (b_play == FALSE)
    {
        i4_ret = _bt_aud_bluetooth_pause_key_proc(BT_AUD_KEY_PAUSE, ui4_keysta);
        BT_AUD_CHK_FAIL_RET(_bt_aud_bluetooth_pause_key_proc, i4_ret, BT_AUD_FAIL);
    }
    else
    {
        i4_ret = _bt_aud_bluetooth_play_key_proc(BT_AUD_KEY_PLAY, ui4_keysta);
        BT_AUD_CHK_FAIL_RET(_bt_aud_bluetooth_play_key_proc, i4_ret, BT_AUD_FAIL);
    }

    g_t_bt_aud_sta.b_play_pause_down = TRUE;//record this action,wait for respond and  clear it

    //make sure play/pause key can use in case src don't return playstatus
    //g_t_bt_aud_sta.b_bt_play = !t_bt_aud_sta.b_bt_play;

    BT_AUD_FUNC_EXIT();
    return i4_ret;
}

static INT32 _bt_aud_bluetooth_volume_down_key_proc(UINT32 ui4_key, UINT32 ui4_keysta)
{
    INT32 i4_ret = BT_AUD_OK;
    BT_AVRCP_KEY_STATE action;

    BT_AUD_FUNC_ENTRY();

    action = BT_AVRCP_KEY_STATE_PRESS;
    //call btm interface
    i4_ret = a_mtkapi_avrcp_send_passthrough_cmd(g_cur_avrcp_addr, BT_AVRCP_CMD_TYPE_VOL_DOWN, action);
    BT_AUD_CHK_FAIL_RET(a_mtkapi_avrcp_send_passthrough_cmd, i4_ret, BT_AUD_FAIL);

    action = BT_AVRCP_KEY_STATE_RELEASE;
    //call btm interface
    i4_ret = a_mtkapi_avrcp_send_passthrough_cmd(g_cur_avrcp_addr, BT_AVRCP_CMD_TYPE_VOL_DOWN, action);
    BT_AUD_CHK_FAIL_RET(a_mtkapi_avrcp_send_passthrough_cmd, i4_ret, BT_AUD_FAIL);

    BT_AUD_FUNC_EXIT();
    return BT_AUD_OK;
}

static INT32 _bt_aud_bluetooth_volume_up_key_proc(UINT32 ui4_key, UINT32 ui4_keysta)
{
    INT32       i4_ret = BT_AUD_OK;
    BT_AVRCP_KEY_STATE action;

    BT_AUD_FUNC_ENTRY();

    action = BT_AVRCP_KEY_STATE_PRESS;
    //call btm interface
    i4_ret = a_mtkapi_avrcp_send_passthrough_cmd(g_cur_avrcp_addr,BT_AVRCP_CMD_TYPE_VOL_UP, action);
    BT_AUD_CHK_FAIL_RET(a_mtkapi_avrcp_send_passthrough_cmd, i4_ret, BT_AUD_FAIL);

    action = BT_AVRCP_KEY_STATE_RELEASE;
    //call btm interface
    i4_ret = a_mtkapi_avrcp_send_passthrough_cmd(g_cur_avrcp_addr, BT_AVRCP_CMD_TYPE_VOL_UP, action);
    BT_AUD_CHK_FAIL_RET(a_mtkapi_avrcp_send_passthrough_cmd, i4_ret, BT_AUD_FAIL);

    BT_AUD_FUNC_EXIT();
    return BT_AUD_OK;
}

static INT32 _bt_aud_bluetooth_ff_key_proc(UINT32 ui4_key, UINT32 ui4_keysta)
{
    INT32       i4_ret = BT_AUD_OK;
    BT_AVRCP_KEY_STATE action;

    BT_AUD_FUNC_ENTRY();

    if (ui4_keysta == BT_AUD_KEYSTA_DOWN)
    {
        action = BT_AVRCP_KEY_STATE_PRESS;
        //call btm interface
        i4_ret = a_mtkapi_avrcp_send_passthrough_cmd(g_cur_avrcp_addr, BT_AVRCP_CMD_TYPE_FFWD, action);
        BT_AUD_CHK_FAIL_RET(a_mtkapi_avrcp_send_passthrough_cmd, i4_ret, BT_AUD_FAIL);
        b_ff_key_press = TRUE;
    }
    else if (ui4_keysta == BT_AUD_KEYSTA_UP)
    {
        action = BT_AVRCP_KEY_STATE_RELEASE;
        //call btm interface
        i4_ret = a_mtkapi_avrcp_send_passthrough_cmd(g_cur_avrcp_addr, BT_AVRCP_CMD_TYPE_FFWD, action);
        BT_AUD_CHK_FAIL_RET(a_mtkapi_avrcp_send_passthrough_cmd, i4_ret, BT_AUD_FAIL);
        b_ff_key_press = FALSE;
    }

    BT_AUD_FUNC_EXIT();
    return BT_AUD_OK;
}

static INT32 _bt_aud_bluetooth_fr_key_proc(UINT32 ui4_key, UINT32 ui4_keysta)
{
    INT32       i4_ret = BT_AUD_OK;
    BT_AVRCP_KEY_STATE action;
    BT_AUD_FUNC_ENTRY();

    if (ui4_keysta == BT_AUD_KEYSTA_DOWN)
    {
        action = BT_AVRCP_KEY_STATE_PRESS;
        //call btm interface
        i4_ret = a_mtkapi_avrcp_send_passthrough_cmd(g_cur_avrcp_addr, BT_AVRCP_CMD_TYPE_RWD, action);
        BT_AUD_CHK_FAIL_RET(a_mtkapi_avrcp_send_passthrough_cmd, i4_ret, BT_AUD_FAIL);
        b_fr_key_press = TRUE;
    }
    else if (ui4_keysta == BT_AUD_KEYSTA_UP)
    {
        action = BT_AVRCP_KEY_STATE_RELEASE;
        //call btm interface
        i4_ret = a_mtkapi_avrcp_send_passthrough_cmd(g_cur_avrcp_addr, BT_AVRCP_CMD_TYPE_RWD, action);
        BT_AUD_CHK_FAIL_RET(a_mtkapi_avrcp_send_passthrough_cmd, i4_ret, BT_AUD_FAIL);
        b_fr_key_press = FALSE;
    }

    BT_AUD_FUNC_EXIT();
    return BT_AUD_OK;
}

static INT32 _bt_aud_play_ctrl_key_proc(UINT32 ui4_keysta,UINT32 ui4_keyval)
{
    INT32 i4_ret;
    BT_AUD_FUNC_ENTRY();

    if (FALSE == _bluetooth_is_bt_connected())
    {
        //try to connect with the last device when pressing play/pause key in pairing mode
        if ((BT_AUD_BT_STA_PAIRING == g_t_bt_aud_sta.e_bt_sta)
            && ((BT_AUD_KEY_PAUSE_PLAY == ui4_keyval) || (BT_AUD_KEY_PLAY == ui4_keyval))
            && (BT_AUD_KEYSTA_DOWN == ui4_keysta))
        {
            DBG_API((BTAUD_TAG"get PLAY/PAUSE key in pairing mode, try to connect last device.\r\n"));

            i4_ret = _bluetooth_paired_dev_list_update(&g_t_bt_pair_dev_list);
            if ((i4_ret == BT_AUD_OK) && (g_t_bt_pair_dev_list.dev_num != 0))
            {
                g_t_bt_aud_sta.b_continue_connect = FALSE;
                i4_ret = _bluetooth_auto_connect(1, TRUE);
                BT_AUD_CHK_FAIL(_bluetooth_auto_connect, i4_ret);
            }
        }
        else
        {
            DBG_INFO((BTAUD_TAG"bluetooth haven't connected, ignore key msg.\r\n"));
        }
    }
    else
    {

        i4_ret = _bt_aud_key_pre_proc(ui4_keyval, ui4_keysta);
        if (i4_ret != BT_AUD_OK)//if no need to handle key, return OK
        {
            DBG_INFO((BTAUD_TAG"key filter.\r\n"));
            return BT_AUD_OK;
        }

        switch(ui4_keyval)
        {
            case BT_AUD_KEY_NEXT:   //next
                DBG_API((BTAUD_TAG"key: NEXT.\r\n"));
                i4_ret = _bt_aud_bluetooth_next_key_proc(ui4_keyval, ui4_keysta);
                BT_AUD_CHK_FAIL_RET(_bt_aud_bluetooth_next_key_proc, i4_ret, BT_AUD_FAIL);
#if CONFIG_VFD_ET6238
	            vfd_drv_disp_msg("NEXT",0);
#endif
                break;
            case BT_AUD_KEY_PREV:   //prev
                DBG_API((BTAUD_TAG"key: PREV.\r\n"));
                i4_ret = _bt_aud_bluetooth_prev_key_proc(ui4_keyval, ui4_keysta);
                BT_AUD_CHK_FAIL_RET(_bt_aud_bluetooth_prev_key_proc, i4_ret, BT_AUD_FAIL);
#if CONFIG_VFD_ET6238
	            vfd_drv_disp_msg("PREV",0);
#endif
                break;
            case BT_AUD_KEY_PAUSE:  //pause
                DBG_API((BTAUD_TAG"key: PAUSE.\r\n"));
                i4_ret = _bt_aud_bluetooth_pause_key_proc(ui4_keyval, ui4_keysta);
                BT_AUD_CHK_FAIL_RET(_bt_aud_bluetooth_pause_key_proc, i4_ret, BT_AUD_FAIL);
                break;
            case BT_AUD_KEY_PLAY:   //play
                DBG_API((BTAUD_TAG"key: PLAY.\r\n"));
                i4_ret = _bt_aud_bluetooth_play_key_proc(ui4_keyval, ui4_keysta);
                BT_AUD_CHK_FAIL_RET(_bt_aud_bluetooth_play_key_proc, i4_ret, BT_AUD_FAIL);
                break;
            case BT_AUD_KEY_STOP:   //stop
                DBG_API((BTAUD_TAG"key: STOP.\r\n"));
                i4_ret = _bt_aud_bluetooth_stop_key_proc(ui4_keyval, ui4_keysta);
                BT_AUD_CHK_FAIL_RET(_bt_aud_bluetooth_stop_key_proc, i4_ret, BT_AUD_FAIL);
                break;
            case BT_AUD_KEY_PAUSE_PLAY:   //play&pause
                DBG_API((BTAUD_TAG"key: PLAY/PAUSE.\r\n"));
                i4_ret = _bt_aud_bluetooth_play_pause_key_proc(ui4_keyval, ui4_keysta);
                BT_AUD_CHK_FAIL_RET(_bt_aud_bluetooth_play_pause_key_proc, i4_ret, BT_AUD_FAIL);
                break;
            case BT_AUD_KEY_VOL_UP:   //volume up
                DBG_API((BTAUD_TAG"key: VOL_UP.\r\n"));
                i4_ret = _bt_aud_bluetooth_volume_up_key_proc(ui4_keyval, ui4_keysta);
                BT_AUD_CHK_FAIL_RET(_bt_aud_bluetooth_volume_up_key_proc, i4_ret, BT_AUD_FAIL);
                break;
            case BT_AUD_KEY_VOL_DOWN:   //volume down
                DBG_API((BTAUD_TAG"key: VOL_DOWN.\r\n"));
                i4_ret = _bt_aud_bluetooth_volume_down_key_proc(ui4_keyval, ui4_keysta);
                BT_AUD_CHK_FAIL_RET(_bt_aud_bluetooth_volume_down_key_proc, i4_ret, BT_AUD_FAIL);
                break;
            case BT_AUD_KEY_FF:   //ff
                DBG_API((BTAUD_TAG"key: FF.\r\n"));
                i4_ret = _bt_aud_bluetooth_ff_key_proc(ui4_keyval, ui4_keysta);
                BT_AUD_CHK_FAIL_RET(_bt_aud_bluetooth_ff_key_proc, i4_ret, BT_AUD_FAIL);
                break;
            case BT_AUD_KEY_FR:   //fr
                DBG_API((BTAUD_TAG"key: FR.\r\n"));
                i4_ret = _bt_aud_bluetooth_fr_key_proc(ui4_keyval, ui4_keysta);
                BT_AUD_CHK_FAIL_RET(_bt_aud_bluetooth_fr_key_proc, i4_ret, BT_AUD_FAIL);
                break;
            default:
                DBG_INFO((BTAUD_TAG"nonsupport key, keyval=%d.\r\n", ui4_keyval));
                break;
        }
    }

    BT_AUD_FUNC_EXIT();
    return BT_AUD_OK;
}

INT32 bt_aud_key_msg_proc(const VOID *pv_msg)
{
    INT32 i4_ret;
    UINT32 ui4_keysta, ui4_keyval;
    BT_AUD_MSG_T *pt_bt_msg = (BT_AUD_MSG_T *)pv_msg;
    BT_AUD_FUNC_ENTRY();

    ui4_keysta = pt_bt_msg->ui4_data1;
    ui4_keyval = pt_bt_msg->ui4_data3;

    DBG_INFO((BTAUD_TAG"get key msg,id=%d keysta=%x keyval=%x.\r\n",
        pt_bt_msg->ui4_msg_id, ui4_keysta, ui4_keyval));

    switch(pt_bt_msg->ui4_msg_id)
    {
        case BT_AUD_MSG_KEY_PLAY_CTRL:   //play ctrl key
            i4_ret = _bt_aud_play_ctrl_key_proc(ui4_keysta, ui4_keyval);
            BT_AUD_CHK_FAIL_RET(_bt_aud_play_ctrl_key_proc, i4_ret, AEER_FAIL);
            break;
        case BT_AUD_MSG_KEY_BTKEY:      //BTKEY
            if (BT_AUD_KEY_FORCED_PAIRING == ui4_keysta)
            {
                DBG_API((BTAUD_TAG"BTkey: forced pairing.\r\n"));
                g_t_bt_aud_sta.b_bt_forced_pairing = TRUE;
            }
            else
            {
                DBG_API((BTAUD_TAG"BTkey: quick switch.\r\n"));
                g_t_bt_aud_sta.b_bt_forced_pairing = FALSE;
            }
            if (_bluetooth_is_bt_ready() == FALSE)    //not open, need to req open from sourceswitch
            {
                i4_ret = _bluetooth_send_request_play_to_sm();
				BT_AUD_CHK_FAIL_RET(_bluetooth_send_request_play_to_sm, i4_ret, BT_AUD_FAIL);
            }
            else    //already open
            {
                if (g_t_bt_aud_sta.b_bt_forced_pairing == TRUE)   //if forced pairing
                {
                    if (TRUE == _bluetooth_is_bt_connected())//and already connect now, need to disconnect first
                    {
                        g_t_bt_aud_sta.b_bt_forced_pairing = FALSE;
						
						i4_ret = _bluetooth_bt_disconnect(FALSE);
                        BT_AUD_CHK_FAIL_RET(_bluetooth_bt_disconnect, i4_ret, BT_AUD_FAIL);
#if 0
						i4_ret = bt_aud_bt_stop();
                        BT_AUD_CHK_FAIL_RET(bt_aud_bt_stop, i4_ret, BT_AUD_FAIL);
					    i4_ret = _bluetooth_start_pairing();
                        BT_AUD_CHK_FAIL(_bluetooth_start_pairing, i4_ret);
#endif
                    }
                    else if (g_t_bt_aud_sta.e_bt_sta == BT_AUD_BT_STA_AUTO_CONNECTING)//if autoconnecting, stop it
                    {
                        //g_t_bt_aud_sta.e_bt_sta = BT_AUD_BT_STA_IDLE;
                        i4_ret = _bluetooth_start_pairing();// pairing mode
                        BT_AUD_CHK_FAIL(_bluetooth_start_pairing, i4_ret);
                    }
                }
            }
            break;

        default:
            DBG_API((BTAUD_TAG"unkown key msgid, id=%d.\r\n", pt_bt_msg->ui4_msg_id));
            break;
    }

    BT_AUD_FUNC_EXIT();
    return BT_AUD_OK;
}

INT32 bt_aud_bluetooth_force_pause(VOID)
{
    INT32 i4_ret;
    DBG_INFO((BTAUD_TAG"Force to pause.\r\n"));

    i4_ret = _bt_aud_bluetooth_pause_key_proc(BT_AUD_KEY_PAUSE, BT_AUD_KEYSTA_DOWN);
    BT_AUD_CHK_FAIL_RET(_bt_aud_bluetooth_pause_key_proc, i4_ret, BT_AUD_FAIL);

    return BT_AUD_OK;
}

INT32 bt_aud_bluetooth_continue_play(VOID)
{
    INT32 i4_ret;
    DBG_INFO((BTAUD_TAG"Force to pause.\r\n"));

    i4_ret = _bt_aud_bluetooth_play_key_proc(BT_AUD_KEY_PLAY, BT_AUD_KEYSTA_DOWN);
    BT_AUD_CHK_FAIL_RET(_bt_aud_bluetooth_play_key_proc, i4_ret, BT_AUD_FAIL);

    return BT_AUD_OK;
}


