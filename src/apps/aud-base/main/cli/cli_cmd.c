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
 * $RCSfile: u_cli_input.c,v $
 * $Revision: #1 $
 * $Date: 2016/07/22 $
 * $Author: bdbm01 $
 * $CCRevision: /main/DTV_X_HQ_int/DTV_X_ATSC/4 $
 * $SWAuthor: Alec Lu $
 * $MD5HEX: e6e42ffefefe41c633e95ec138eae58a $
 *
 * Description:
 *         The program will handle UART input to CLI console and is exported
 *         to applications.
 *---------------------------------------------------------------------------*/
/*-----------------------------------------------------------------------------
                    include files
 ----------------------------------------------------------------------------*/
#include <string.h>
 
#include "u_common.h"
#include "u_dbg.h"
#include "_cli.h"

/*-----------------------------------------------------------------------------
                    macros, defines, typedefs, enums
 ----------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------
                    data declarations
 ----------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------
                    function declarations
 ----------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------
 * Name: u_cli_init
 *
 * Description: CLI initialization function.
 *
 * Inputs:  -
 *
 * Outputs: -
 *
 * Returns: CLIR_OK                     Routine successful.
 *          CLIR_ALREADY_INIT           The CLI has already been initialized.
 ----------------------------------------------------------------------------*/
INT32 u_cli_init(VOID)
{
#ifdef CLI_SUPPORT
    return cli_init();
#else
    return CLIR_NOT_ENABLED;
#endif
}

 
/*-----------------------------------------------------------------------------
 * Name: u_cli_attach_cmd_tbl
 *
 * Description: This API attaches a command table to CLI root.
 *
 * Inputs:  pt_tbl          The command table to be attached.
 *          e_category      The category that the command table belongs to.
 *          ui8_group_mask  The group(s) that the command table belongs to.
 *
 * Outputs: -
 *
 * Returns: CLIR_OK                 Routine successful.
 *          CLIR_NOT_INIT           The CLI has not been initialized.
 *          CLIR_INV_ARG            One or more invalid arguments.
 *          CLIR_CMD_TBL_FULL       Command table is full.
 ----------------------------------------------------------------------------*/
INT32 u_cli_attach_cmd_tbl(CLI_EXEC_T* pt_tbl,
                           CLI_CAT_T   e_category,
                           UINT64      ui8_group_mask)
{
#ifdef CLI_SUPPORT
    if (!cli_is_inited())
    {
        return CLIR_NOT_INIT;
    }

    /* Check arguments */
    if (pt_tbl == NULL)
    {
        return CLIR_INV_ARG;
    }

    return (cli_parser_attach_cmd_tbl(pt_tbl, e_category, ui8_group_mask));
#else
    return CLIR_NOT_ENABLED;
#endif
}

/*-----------------------------------------------------------------------------
 * Name: u_cli_detach_cmd_tbl
 *
 * Description: This API detach a command table to CLI root.
 *
 * Inputs:  pt_tbl          The command table to be detached.
 *          e_category      The category that the command table belongs to.
 *          ui8_group_mask  The group(s) that the command table belongs to.
 *
 * Outputs: -
 *
 * Returns: CLIR_OK                 Routine successful.
 *          CLIR_NOT_INIT           The CLI has not been initialized.
 *          CLIR_INV_ARG            One or more invalid arguments.
 *          CLIR_CMD_TBL_FULL       Command table is full.
 ----------------------------------------------------------------------------*/
INT32 u_cli_detach_cmd_tbl(CLI_EXEC_T* pt_tbl,
                           CLI_CAT_T   e_category,
                           UINT64      ui8_group_mask)
{
#ifdef CLI_SUPPORT
    if (!cli_is_inited())
    {
        return CLIR_NOT_INIT;
    }

    /* Check arguments */
    if (pt_tbl == NULL)
    {
        return CLIR_INV_ARG;
    }

    return (cli_parser_detach_cmd_tbl(pt_tbl, e_category, ui8_group_mask));
#else
    return CLIR_NOT_ENABLED;
#endif
}

/*-----------------------------------------------------------------------------
 * Name: u_cli_parser
 *
 * Description: This API parses CLI command and performs corresponding
 *              operation.
 *
 * Inputs:  ps_cmd      Contain the command to parse.
 *
 * Outputs: -
 *
 * Returns: CLIR_OK                 Routine successful.
 *          CLIR_NOT_INIT           The CLI has not been initialized.
 *          CLIR_INV_ARG            One or more invalid arguments.
 *          CLIR_UNKNOWN_CMD        Unknown CLI command.
 ----------------------------------------------------------------------------*/
INT32 u_cli_parser(const CHAR* ps_cmd)
{
#ifdef CLI_SUPPORT
    if (!cli_is_inited())
    {
    	dbg_print("cli is not init\n");
        return CLIR_NOT_INIT;
    }

    return (cli_parser(ps_cmd));
#else
    return CLIR_NOT_ENABLED;
#endif
}

/*-----------------------------------------------------------------------------
 * Name: u_cli_parser_arg
 *
 * Description: This API parses CLI command and performs corresponding
 *              operation. The variable-length argument lists are supported.
 *
 * Inputs:  ps_cmd   Contain the format.
 *          ...      Variable argument list.
 *
 * Outputs: -
 *
 * Returns: CLIR_OK             Routine successful.
 *          CLIR_NOT_INIT       The CLI has not been initialized.
 *          CLIR_INV_ARG        One or more invalid arguments.
 *          CLIR_UNKNOWN_CMD    Unknown CLI command.
 ----------------------------------------------------------------------------*/
INT32 u_cli_parser_arg(const CHAR* ps_cmd, ...)
{
	CHAR    s_buf[CLI_CMD_BUF_SIZE];
	VA_LIST t_ap;

    if (!cli_is_inited())
    {
        return CLIR_NOT_INIT;
    }

	VA_START(t_ap, ps_cmd);
	vsnprintf(s_buf,CLI_CMD_BUF_SIZE, ps_cmd, t_ap); 
	VA_END(t_ap);

	return (cli_parser(s_buf));
}


/*-----------------------------------------------------------------------------
 * Name: u_cli_parse_dbg_level
 *
 * Description: This API parses a CLI command for debug level.
 *
 * Inputs:  i4_argc         Number of arguments.
 *          pps_argv        Points to the argument array.
 *
 * Outputs: pui2_dbg_lvl    The value of debug level after parsing.
 *
 * Returns: CLIR_OK                 Routine successful.
 *          CLIR_CMD_EXEC_ERROR     Input password is incorrect.
 ----------------------------------------------------------------------------*/
INT32 u_cli_parse_dbg_level(INT32        i4_argc,
                            const CHAR** pps_argv,
                            UINT16*      pui2_dbg_level)
{
#ifdef CLI_SUPPORT
    const CHAR*     ps_dbg_level;
    BOOL            b_invalid_arg = FALSE;

    /* xxx.xxx [level] */

    if (!cli_is_inited())
    {
        return CLIR_NOT_INIT;
    }

    /* Check arguments */
	if ((i4_argc < 2) ||
	    (pps_argv == NULL) ||
	    (pui2_dbg_level == NULL))
	{
		return CLIR_INV_ARG;
	}

	*pui2_dbg_level = 0;
	ps_dbg_level = pps_argv[1];

	if (((strchr(ps_dbg_level, 'n') != NULL) ||
	     (strchr(ps_dbg_level, 'N') != NULL)) &&
	    (strlen(ps_dbg_level) > 1))
    {
        return CLIR_INV_ARG;
    }

	if (strcmp(ps_dbg_level, CLI_ALL_STR) == 0)
	{
	    *pui2_dbg_level = DBG_LEVEL_ERROR | DBG_LEVEL_API | DBG_LEVEL_INFO;
        return CLIR_OK;
	}

	while (*ps_dbg_level != 0)
	{
	    switch (*ps_dbg_level)
	    {
	        case 'e':
	        case 'E':
	            *pui2_dbg_level |= DBG_LEVEL_ERROR;
	            break;

	        case 'a':
	        case 'A':
	            *pui2_dbg_level |= DBG_LEVEL_API;
	            break;

            case 'i':
            case 'I':
                *pui2_dbg_level |= DBG_LEVEL_INFO;
                break;

            case 'n':
            case 'N':
                *pui2_dbg_level = 0;
                break;

            default:
                b_invalid_arg = TRUE;
	            break;
	    }

	    ps_dbg_level++;
	}

    if (b_invalid_arg)
    {
        return CLIR_INV_ARG;
    }
    else
    {
        return CLIR_OK;
    }
#else
    return CLIR_NOT_ENABLED;
#endif
}


/*-----------------------------------------------------------------------------
 * Name: u_cli_show_dbg_level
 *
 * Description: This API display debug level in text format.
 *
 * Inputs:  ui2_dbg_lvl          The value of debug level to display.
 *
 * Outputs: -
 *
 * Returns: CLIR_OK                 Routine successful.
 ----------------------------------------------------------------------------*/
INT32 u_cli_show_dbg_level(UINT16 ui2_dbg_level)
{
#ifdef CLI_SUPPORT
    BOOL    b_valid_dbg_level = FALSE;

    if (!cli_is_inited())
    {
        return CLIR_NOT_INIT;
    }

    dbg_print("Debug level = ");

    if (ui2_dbg_level & DBG_LEVEL_ERROR)
    {
        dbg_print("e");
        b_valid_dbg_level = TRUE;
    }

    if (ui2_dbg_level & DBG_LEVEL_API)
    {
        dbg_print("a");
        b_valid_dbg_level = TRUE;
    }

    if (ui2_dbg_level & DBG_LEVEL_INFO)
    {
        dbg_print("i");
        b_valid_dbg_level = TRUE;
    }

    if (b_valid_dbg_level == FALSE)
    {
        dbg_print("%s", CLI_NONE_STR);
    }

    dbg_print("\n\r");

    return CLIR_OK;
#else
    return CLIR_NOT_ENABLED;
#endif
}

//only for test
VOID  u_cli_tset_init_phase1(VOID)
{
    cli_init_phase1();
}

VOID u_cli_test_init_phase2(VOID)
{
    cli_init_phase2();
}
