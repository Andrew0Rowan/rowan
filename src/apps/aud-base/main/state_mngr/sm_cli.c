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


#include <string.h>

#include "u_dbg.h"

#include "u_aee.h"

#include "sm_cli.h"
#include "u_app_thread.h"
#include "sm.h"
#include "u_sm.h"
#include "u_key_def.h"
#include "u_assistant_stub.h"

#ifdef CLI_SUPPORT
#include "u_cli.h"


extern SM_OBJ_T g_t_sm;
extern SOURCE_INFO_T g_t_source_list[];
extern UINT32 _sm_get_msg_from_source(SM_PLAYBACK_SOURCE_E e_source);

static INT32 _sm_cli_get_dbg_level (INT32 i4_argc, const CHAR** pps_argv)
{
    INT32  i4_ret;

    i4_ret = u_cli_show_dbg_level(sm_get_dbg_level());

    return i4_ret;
}

static INT32 _sm_cli_set_dbg_level (INT32 i4_argc, const CHAR** pps_argv)
{
    UINT16 ui2_dbg_level;
    INT32  i4_ret;

    i4_ret = u_cli_parse_dbg_level(i4_argc, pps_argv, &ui2_dbg_level);

    if (i4_ret == CLIR_OK){
        sm_set_dbg_level(ui2_dbg_level);
    }

    return i4_ret;
}


static INT32 _sm_test (INT32 i4_argc, const CHAR** pps_argv)
{
    INT32 i4_ret;
    APPMSG_T t_msg;

    if (i4_argc < 4)
    {
        SM_ERR("usage: app.sm.test [MSG_FROM_XXX] [SM_XXX_GRP] [BODY]\n");
        return CLIR_INV_ARG;
    }

    t_msg.ui4_sender_id = atoi(pps_argv[1]);
    t_msg.ui4_msg_type = SM_MAKE_MSG(atoi(pps_argv[2]), atoi(pps_argv[3]), 0);
    _sm_send_msg(t_msg.ui4_sender_id, t_msg.ui4_msg_type,SM_THREAD_NAME);

    return CLIR_OK;
}

static INT32 _sm_show_source(INT32 i4_argc, const CHAR** pps_argv)
{
    SM_MSG("source:[%d] status:[%d]\n", u_sm_get_current_source(), u_sm_get_current_playback_status());
    return CLIR_OK;
}

static INT32 _sm_show_network(INT32 i4_argc, const CHAR** pps_argv)
{
    BOOL b_network_connect = u_sm_is_network_connect();

    SM_MSG("network status:[%s]\n", b_network_connect ? "connect" : "disconnect");
    return CLIR_OK;
}

static INT32 _sm_show_standby(INT32 i4_argc, const CHAR** pps_argv)
{
    if (u_sm_is_enter_fake_standby())
    {
        SM_MSG("Current is [fake-standby]!\n");
    }
    else if (u_sm_is_enter_suspend_standby())
    {
        SM_MSG("Current is [suspend-standby]!\n");
    }
    else
    {
        SM_MSG("Current is [non-standby]!\n");
    }

    return CLIR_OK;
}

static INT32 _sm_set_fake_standby(INT32 i4_argc, const CHAR** pps_argv)
{
    if (i4_argc < 2)
    {
        SM_ERR("usage: app.sm.set_fake_standby [0/1]\n");
        return CLIR_INV_ARG;
    }

    u_sm_set_fake_standby(atoi(pps_argv[1]));

    return CLIR_OK;
}

static INT32 _sm_request_play(INT32 i4_argc, const CHAR** pps_argv)
{
    UINT32 ui4_sender_id = 0;

    ui4_sender_id = _sm_get_msg_from_source(atoi(pps_argv[1]));
    _sm_send_msg(ui4_sender_id, SM_MAKE_MSG(SM_REQUEST_GRP, SM_BODY_PLAY, 0), SM_THREAD_NAME);
    return CLIR_OK;
}

static INT32 _sm_inform_stop(INT32 i4_argc, const CHAR** pps_argv)
{
    UINT32 ui4_sender_id = 0;

    ui4_sender_id = _sm_get_msg_from_source(atoi(pps_argv[1]));

    _sm_send_msg(ui4_sender_id, SM_MAKE_MSG(SM_INFORM_GRP, SM_BODY_STOP, 0), SM_THREAD_NAME);

    return CLIR_OK;
}

static INT32 _sm_set_suspend_standby_flag(INT32 i4_argc, const CHAR** pps_argv)
{
    _sm_set_ecopwer_flag(atoi(pps_argv[1]));
    SM_ERR("%s, set flag:%d\n",__FUNCTION__,atoi(pps_argv[1]));
    return CLIR_OK;
}

/* wifi test command table */
static CLI_EXEC_T at_ht_wifi_cmd_tbl[] =
{

    END_OF_CLI_CMD_TBL
};

/* main command table */
static CLI_EXEC_T at_sm_cmd_tbl[] =
{
    {
        CLI_GET_DBG_LVL_STR,
        NULL,
        _sm_cli_get_dbg_level,
        NULL,
        CLI_GET_DBG_LVL_HELP_STR,
        CLI_GUEST
    },
    {
        CLI_SET_DBG_LVL_STR,
        NULL,
        _sm_cli_set_dbg_level,
        NULL,
        CLI_SET_DBG_LVL_HELP_STR,
        CLI_GUEST
    },
    {
        "test",
        NULL,
        _sm_test,
        NULL,
        "test",
        CLI_GUEST
    },
    {
        "show_source",
        NULL,
        _sm_show_source,
        NULL,
        "show source's name and playback status",
        CLI_GUEST
    },
    {
        "show_network",
        NULL,
        _sm_show_network,
        NULL,
        "show network status",
        CLI_GUEST
    },
    {
        "standby",
        NULL,
        _sm_show_standby,
        NULL,
        "show standby mode",
        CLI_GUEST
    },
    {
        "set_fake_standby",
        NULL,
        _sm_set_fake_standby,
        NULL,
        "set fake standby",
        CLI_GUEST
    },
    {
        "request_play",
        NULL,
        _sm_request_play,
        NULL,
        "request_play 1",
        CLI_GUEST
    },
    {
        "inform_stop",
        NULL,
        _sm_inform_stop,
        NULL,
        "request_play 1",
        CLI_GUEST
    },
    {
        "set_suspend_flag",
        NULL,
        _sm_set_suspend_standby_flag,
        NULL,
        "set_suspend_flag",
        CLI_GUEST
    },
    END_OF_CLI_CMD_TBL
};

static CLI_EXEC_T at_sm_root_cmd_tbl[] =
{
    {
        "sm",
        "sm",
        NULL,
        at_sm_cmd_tbl,
        "sm commands",
        CLI_GUEST
    },
    END_OF_CLI_CMD_TBL
};

INT32 sm_cli_attach_cmd_tbl(VOID)
{
    return (u_cli_attach_cmd_tbl(at_sm_root_cmd_tbl, CLI_CAT_APP, CLI_GRP_GUI));
}





#endif
