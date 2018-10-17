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
#include <stdlib.h>

#ifdef CLI_SUPPORT

#include "u_am.h"
#include "u_cli.h"
#include "u_dbg.h"


extern VOID   am_set_dbg_level (UINT16 ui2_dbg_level);
extern UINT16 am_get_dbg_level (VOID);
extern VOID   am_set_tms_level (UINT16 ui2_tms_level);
extern UINT16 am_get_tms_level (VOID);

/*-----------------------------------------------------------------------------
 * funcation declarations
 *---------------------------------------------------------------------------*/
static INT32  _am_cli_get_dbg_level (INT32 i4_argc, const CHAR** pps_argv);
static INT32  _am_cli_set_dbg_level (INT32 i4_argc, const CHAR** pps_argv);

#ifdef CONFIG_TIME_MEASUREMENT
static INT32  _am_cli_get_tms_level (INT32 i4_argc, const CHAR** pps_argv);
static INT32  _am_cli_set_tms_level (INT32 i4_argc, const CHAR** pps_argv);
#endif /* CONFIG_TIME_MEASUREMENT */
static INT32 _am_cli_get_ic_func(INT32 i4_argc, const CHAR** pps_argv);

/*-----------------------------------------------------------------------------
 * variable declarations
 *---------------------------------------------------------------------------*/
/* command table */
static CLI_EXEC_T at_am_cmd_tbl[] =
{

    {
        CLI_GET_DBG_LVL_STR,
        NULL,
        _am_cli_get_dbg_level,
        NULL,
        CLI_GET_DBG_LVL_HELP_STR,
        CLI_GUEST
    },
    {
        CLI_SET_DBG_LVL_STR,
        NULL,
        _am_cli_set_dbg_level,
        NULL,
        CLI_SET_DBG_LVL_HELP_STR,
        CLI_GUEST
    },
#ifdef CONFIG_TIME_MEASUREMENT
    {
        CLI_SET_TMS_LVL_STR,
        NULL,
        _am_cli_set_tms_level,
        NULL,
        CLI_SET_TMS_LVL_HELP_STR,
        CLI_GUEST
    },
    {
        CLI_GET_TMS_LVL_STR,
        NULL,
        _am_cli_get_tms_level,
        NULL,
        CLI_GET_TMS_LVL_HELP_STR,
        CLI_GUEST
    },
#endif /* CONFIG_TIME_MEASUREMENT */
    {
        "get ic func",
        NULL,
        _am_cli_get_ic_func,
        NULL,
        "get ic func",
        CLI_GUEST
    },

    END_OF_CLI_CMD_TBL
};

/* root command table */
static CLI_EXEC_T at_am_root_cmd_tbl[] =
{
    {"am", NULL, NULL, at_am_cmd_tbl, "AM commands", CLI_GUEST},
    END_OF_CLI_CMD_TBL
};

/*-----------------------------------------------------------------------------
 * export functions implementation
 *---------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------
 * Name:    am_cli_attach_cmd_tbl
 * Description:
 * Input arguments:  -
 * Output arguments: -
 * Returns:
 *----------------------------------------------------------------------------*/
INT32 am_cli_attach_cmd_tbl(VOID)
{
	return (u_cli_attach_cmd_tbl(at_am_root_cmd_tbl, CLI_CAT_APP, CLI_GRP_NONE));
}

/*-----------------------------------------------------------------------------
 * static functions implementation
 *---------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------
 * Name: _am_cli_get_dbg_level
 *
 * Description: This API gets the current debug level of Application Manager.
 *
 * Inputs:  i4_argc         Contains the argument count.
 *          pps_argv        Contains the arguments.
 *
 * Outputs: -
 *
 * Returns: CLIR_OK         Routine successful.
 ----------------------------------------------------------------------------*/
static INT32 _am_cli_get_dbg_level (INT32 i4_argc, const CHAR** pps_argv)
{
    INT32  i4_res;

    i4_res = u_cli_show_dbg_level(am_get_dbg_level());

    return i4_res;
}

/*-----------------------------------------------------------------------------
 * Name: _am_cli_set_dbg_level
 *
 * Description: This API sets the debug level of Application Manager.
 *
 * Inputs:  i4_argc         Contains the argument count.
 *          pps_argv        Contains the arguments.
 *
 * Outputs: -
 *
 * Returns: CLIR_OK         Routine successful.
 ----------------------------------------------------------------------------*/
static INT32 _am_cli_set_dbg_level (INT32 i4_argc, const CHAR** pps_argv)
{
    INT32       i4_res;
    UINT16      ui2_dbg_level;

    i4_res = u_cli_parse_dbg_level (i4_argc, pps_argv, & ui2_dbg_level);

    if (i4_res == CLIR_OK)
    {
        am_set_dbg_level (ui2_dbg_level);
    }
    else
    {
        dbg_print ("<AM> ERR: set debug level failed=%d\n\r", i4_res);
    }

    return i4_res;
}

#ifdef CONFIG_TIME_MEASUREMENT
/*-----------------------------------------------------------------------------
 * Name: _am_cli_set_tms_level
 *
 * Description: This API sets the time measure level of Application Manager.
 *
 * Inputs:  i4_argc         Contains the argument count.
 *          pps_argv        Contains the arguments.
 *
 * Outputs: -
 *
 * Returns: CLIR_OK         Routine successful.
 ----------------------------------------------------------------------------*/
static INT32 _am_cli_set_tms_level (INT32 i4_argc, const CHAR** pps_argv)
{
    UINT32      i4_res;
    UINT16      ui2_tms_level;

    i4_res = u_cli_parse_tms_level (i4_argc, pps_argv, & ui2_tms_level);

    if (i4_res == CLIR_OK)
    {
        am_set_tms_level (ui2_tms_level);
    }
    else
    {
        dbg_print ("<AM> ERR: set tms level failed=%d\n\r", i4_res);
    }

    u_cli_show_tms_level (am_get_tms_level());

    return CLIR_OK;
}

/*-----------------------------------------------------------------------------
 * Name: _am_cli_get_tms_level
 *
 * Description: This API gets the time measure level of Application Manager.
 *
 * Inputs:  i4_argc         Contains the argument count.
 *          pps_argv        Contains the arguments.
 *
 * Outputs: -
 *
 * Returns: CLIR_OK         Routine successful.
 ----------------------------------------------------------------------------*/
static INT32 _am_cli_get_tms_level (INT32 i4_argc, const CHAR** pps_argv)
{
    return u_cli_show_tms_level (am_get_tms_level ());
}
#endif /* CONFIG_TIME_MEASUREMENT */

static INT32 _am_cli_get_ic_func(INT32 i4_argc, const CHAR** pps_argv)
{
  return 0;
}



/*-----------------------------------------------------------------------------
 * Name: am_cli_init
 *
 * Description: This API initializes CLI component for Application Manager.
 *
 * Inputs:  -
 *
 * Outputs: -
 *
 * Returns: AMR_OK                     Routine successful.
 *          AMR_FAIL                   CLI operation is failed.
 ----------------------------------------------------------------------------*/
INT32 am_cli_init (VOID)
{
    INT32       i4_res;

    /* attach command table to CLI */
    i4_res = am_cli_attach_cmd_tbl ();

    if (i4_res != CLIR_OK)
    {
        return AMR_FAIL;
    }
    return AMR_OK;
}

INT32 am_cli_uninit (VOID)
{
    return (u_cli_detach_cmd_tbl(at_am_root_cmd_tbl, CLI_CAT_APP, CLI_GRP_NONE));
}

#endif /* CLI_SUPPORT */


