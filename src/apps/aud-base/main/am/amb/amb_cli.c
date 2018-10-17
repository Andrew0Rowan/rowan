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



#include "u_amb.h"

/*-----------------------------------------------------------------------------
 * structure, constants, macro definitions
 *---------------------------------------------------------------------------*/

#ifdef CLI_SUPPORT
#include "u_cli.h"
#include "u_dbg.h"

#undef   DBG_LEVEL_MODULE
#define  DBG_LEVEL_MODULE       amb_get_dbg_level()

/*-----------------------------------------------------------------------------
 * funcation declarations
 *---------------------------------------------------------------------------*/
extern VOID   amb_set_dbg_level (UINT16 ui2_dbg_level);
extern UINT16 amb_get_dbg_level (VOID);

static INT32  _amb_cli_get_dbg_level (INT32 i4_argc, const CHAR** pps_argv);
static INT32  _amb_cli_set_dbg_level (INT32 i4_argc, const CHAR** pps_argv);

static INT32  _amb_cli_start_app (INT32 i4_argc, const CHAR** pps_argv);
static INT32  _amb_cli_pause_app (INT32 i4_argc, const CHAR** pps_argv);
static INT32  _amb_cli_exit_app (INT32 i4_argc, const CHAR** pps_argv);

static INT32 _amb_cli_scan_active (INT32 i4_argc, const CHAR** pps_argv);

/*-----------------------------------------------------------------------------
 * variable declarations
 *---------------------------------------------------------------------------*/
/* command table */
static CLI_EXEC_T at_amb_cmd_tbl[] =
{
    {
        "start_app",
        "s",
        _amb_cli_start_app,
        NULL,
        "Usage: s app_name",
        CLI_GUEST
    },
       
    {
        "scan active",
        "sa",
        _amb_cli_scan_active,
        NULL,
        "Usage: sa",
        CLI_GUEST
    },
    {
        CLI_GET_DBG_LVL_STR,
        NULL,
        _amb_cli_get_dbg_level,
        NULL,
        CLI_GET_DBG_LVL_HELP_STR,
        CLI_GUEST
    },
    {
        CLI_SET_DBG_LVL_STR,
        NULL,
        _amb_cli_set_dbg_level,
        NULL,
        CLI_SET_DBG_LVL_HELP_STR,
        CLI_GUEST
    },

    END_OF_CLI_CMD_TBL
};

/* root command table */
static CLI_EXEC_T at_amb_root_cmd_tbl[] =
{
    {"amb", NULL, NULL, at_amb_cmd_tbl, "AMB commands", CLI_GUEST},
    END_OF_CLI_CMD_TBL
};

/*-----------------------------------------------------------------------------
 * export functions implementation
 *---------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------
 * Name:    amb_cli_attach_cmd_tbl
 * Description:
 * Input arguments:  -
 * Output arguments: -
 * Returns:
 *----------------------------------------------------------------------------*/
INT32 amb_cli_attach_cmd_tbl(VOID)
{
    return (u_cli_attach_cmd_tbl(at_amb_root_cmd_tbl, CLI_CAT_APP, CLI_GRP_NONE));
}

/*-----------------------------------------------------------------------------
 * static functions implementation
 *---------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------
 * Name: _amb_cli_get_dbg_level
 *
 * Description: This API gets the current debug level of Application Manager Broker.
 *
 * Inputs:  i4_argc         Contains the argument count.
 *          pps_argv        Contains the arguments.
 *
 * Outputs: -
 *
 * Returns: CLIR_OK         Routine successful.
 ----------------------------------------------------------------------------*/
static INT32 _amb_cli_get_dbg_level (INT32 i4_argc, const CHAR** pps_argv)
{
    INT32  i4_res;

    i4_res = u_cli_show_dbg_level(amb_get_dbg_level());

    return i4_res;
}


/*-----------------------------------------------------------------------------
 * Name: _amb_cli_set_dbg_level
 *
 * Description: This API sets the debug level of Application Manager Broker
 *
 * Inputs:  i4_argc         Contains the argument count.
 *          pps_argv        Contains the arguments.
 *
 * Outputs: -
 *
 * Returns: CLIR_OK         Routine successful.
 ----------------------------------------------------------------------------*/
static INT32 _amb_cli_set_dbg_level (INT32 i4_argc, const CHAR** pps_argv)
{
    INT32       i4_res;
    UINT16      ui2_dbg_level;

    i4_res = u_cli_parse_dbg_level(i4_argc, pps_argv, & ui2_dbg_level);

    if (i4_res == CLIR_OK)
    {
        amb_set_dbg_level (ui2_dbg_level);
    }
    else
    {
        dbg_print ("<AMB> ERR: set debug level failed=%d\n\r", i4_res);
    }

    return i4_res;
}

/*----------------------------------------------------------------------------
 * Name:    _amb_cli_start_app
 * Description:
 * Input arguments:  -
 * Output arguments: -
 * Returns:
 *----------------------------------------------------------------------------*/
static INT32 _amb_cli_start_app (INT32 i4_argc, const CHAR** pps_argv)
{
    CHAR   s_app_name[APP_NAME_MAX_LEN+1]; /* application name */

    s_app_name[0] = 0;

    if (i4_argc > 1)
    {
        //c_strcpy (s_app_name, pps_argv[1]);//klocwork issue mtk70599@090302
        
        strncpy (s_app_name, pps_argv[1],APP_NAME_MAX_LEN+1);
    }

    if (u_amb_get_register_info (s_app_name) != NULL)
    {
        u_amb_start_app (s_app_name);
    }

    return CLIR_OK;
}


/*----------------------------------------------------------------------------
 * Name:    _amb_cli_exit_app
 * Description:
 * Input arguments:  -
 * Output arguments: -
 * Returns:
 *----------------------------------------------------------------------------*/
static INT32 _amb_cli_exit_app (INT32 i4_argc, const CHAR** pps_argv)
{
    CHAR   s_app_name[APP_NAME_MAX_LEN+1]; /* application name */

    s_app_name[0] = 0;

    if (i4_argc > 1)
    {
       // c_strcpy (s_app_name, pps_argv[1]);//klocwork issue mtk70599@090302
       strncpy (s_app_name, pps_argv[1],APP_NAME_MAX_LEN+1);
    }

    if (u_amb_get_register_info (s_app_name) != NULL)
    {
        u_amb_exit_app (s_app_name);
    }

    return CLIR_OK;
}

/*----------------------------------------------------------------------------
 * Name:    _amb_cli_channel_scan_active
 * Description:
 * Input arguments:  -
 * Output arguments: -
 * Returns:
 *----------------------------------------------------------------------------*/
static INT32 _amb_cli_scan_active (INT32 i4_argc, const CHAR** pps_argv)
{
	AMB_BROADCAST_MSG_T msg;
	msg.ui4_type = E_APP_MSG_SCAN_ACTIVE ;
	msg.z_msg_len = 0;
	if(0!=u_amb_broadcast_msg (&msg, FALSE))
		DBG_ERROR(("u_amb_broadcast_msg failed.\n"));
							   

    return CLIR_OK;
}

/*-----------------------------------------------------------------------------
 * Name: amb_cli_init
 *
 * Description: This API initializes CLI component for Application Manager Broker.
 *
 * Inputs:  -
 *
 * Outputs: -
 *
 * Returns: AMBR_OK                     Routine successful.
 *          AMBR_FAIL                   CLI operation is failed.
 ----------------------------------------------------------------------------*/
INT32 amb_cli_init (VOID)
{
#ifdef CLI_SUPPORT
    INT32       i4_res;

    /* attach command table to CLI */
    i4_res = amb_cli_attach_cmd_tbl ();

    if (i4_res != CLIR_OK)
    {
        return AMBR_FAIL;
    }
    return AMBR_OK;
#else
    return AMBR_OK;
#endif /* CLI_SUPPORT */
}

INT32 amb_cli_uninit (VOID)
{
#ifdef CLI_SUPPORT
    return (u_cli_detach_cmd_tbl(at_amb_root_cmd_tbl, CLI_CAT_APP, CLI_GRP_NONE));
#else
    return AMBR_OK;
#endif /* CLI_SUPPORT */
}

#endif /* CLI_SUPPORT */

