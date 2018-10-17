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


#include "u_cli.h"
#include "u_dbg.h"
#include "u_app_thread.h"
#include "u_misc.h"

static UINT16 ui2_g_misc_dbg_level = DBG_INIT_LEVEL_APP_MISC;

UINT16 _misc_get_dbg_level(VOID)
{
    return (ui2_g_misc_dbg_level | DBG_LAYER_APP);
}

VOID _misc_set_dbg_level(UINT16 ui2_db_level)
{
    ui2_g_misc_dbg_level = ui2_db_level;
}

static INT32 _misc_cli_set_dbg_level (INT32 i4_argc, const CHAR ** pps_argv)
{
    UINT16 ui2_dbg_level;
    INT32  i4_ret;

    i4_ret = u_cli_parse_dbg_level(i4_argc, pps_argv, &ui2_dbg_level);
    if (CLIR_OK == i4_ret)
	{
        _misc_set_dbg_level(ui2_dbg_level);
    }

    return i4_ret;
}

static INT32 _misc_cli_get_dbg_level (INT32 i4_argc, const CHAR ** pps_argv)
{
    INT32  i4_ret;

    i4_ret = u_cli_show_dbg_level(_misc_get_dbg_level());

    return i4_ret;
}

static INT32 _misc_cli_test (INT32 i4_argc, const CHAR ** pps_argv)
{
	MISC_FUNCTION_BEGIN		
	MISC_FUNCTION_END
    return 0;
}

/* command table */
static CLI_EXEC_T _misc_cmd_tbl[] =
{
    {
        CLI_GET_DBG_LVL_STR,
        NULL,
        _misc_cli_get_dbg_level,
        NULL,
        CLI_GET_DBG_LVL_HELP_STR,
        CLI_GUEST
    },
    {
        CLI_SET_DBG_LVL_STR,
        NULL,
        _misc_cli_set_dbg_level,
        NULL,
        CLI_SET_DBG_LVL_HELP_STR,
        CLI_GUEST
    },
    {
        "test",
        "test",
        _misc_cli_test,
        NULL,
        "[misc]_misc_test",
        CLI_GUEST
    },
    END_OF_CLI_CMD_TBL
};

static CLI_EXEC_T _misc_root_cmd_tbl[] =
{
    {   
		"misc",
        "misc",
        NULL,
        _misc_cmd_tbl,
        "misc commands",
        CLI_GUEST
    },
    END_OF_CLI_CMD_TBL
};

INT32 _misc_cli_attach_cmd_tbl(VOID)
{
    return (u_cli_attach_cmd_tbl(_misc_root_cmd_tbl, CLI_CAT_APP, CLI_GRP_GUI));
}
