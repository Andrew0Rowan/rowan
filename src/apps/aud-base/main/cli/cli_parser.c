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
 * $RCSfile: cli_parser.c,v $
 * $Revision: #1 $
 * $Date: 2016/07/22 $
 * $Author: bdbm01 $
 * $CCRevision: /main/DTV_X_HQ_int/DTV_X_ATSC/18 $
 * $SWAuthor: Clear Case Administrator $
 * $MD5HEX: b794305807e8df685f62004bee4502b0 $
 *
 * Description:
 *         This program will handle string parsing of user's input to CLI
 *         console.
 *---------------------------------------------------------------------------*/
/*-----------------------------------------------------------------------------
                    include files
 ----------------------------------------------------------------------------*/

#include "unistd.h"
#include <fcntl.h>
#include <termios.h>
#include <signal.h>
#include <errno.h>
#include <sys/ioctl.h>
#include "stdlib.h"
#include <sys/ioctl.h>
#include <string.h>

#include "u_common.h"
#include "u_dbg.h"
#include "_cli.h"


static INT32 _cli_cmd_exec(INT32 i4_argc, const CHAR** pps_argv);
static INT32 _cli_cmd_uart0_set_baudrate(INT32 i4_argc, const CHAR** pps_argv);
static INT32 _cli_cmd_change_dir(INT32 i4_argc, const CHAR** pps_argv);
static INT32 _cli_cmd_list_cmd(INT32 i4_argc, const CHAR** pps_argv);
static INT32 _cli_cmd_set_DisablePrint(INT32 i4_argc, const CHAR** pps_argv);
/*-----------------------------------------------------------------------------
                    macros, defines, typedefs, enums
 ----------------------------------------------------------------------------*/
 
#define HELP_STR_POS            ((UINT32)    24)
#define MAX_STR_LEN             ((UINT32)   128)

#define IS_PRINTABLE(c)         ((((c) > ASCII_NULL) && ((c) < ASCII_KEY_SPACE)) ? 0 : 1)
#define IS_SPACE(c)             (((c) == ' ' || (c) == '\n' || (c) == '\t' || (c) == '\r' || (c) == '\a') ? 1 : 0)
#define IS_DOT(c)               (((c) == ASCII_KEY_DOT) ? 1 : 0)
#define IS_ROOT(c)              (((c) == ASCII_KEY_ROOT) ? 1 : 0)


/*-----------------------------------------------------------------------------
                    data declarations
 ----------------------------------------------------------------------------*/
UINT32 ui4_disable_print = 0;     //control all debug log level

static CLI_EXEC_T*              apt_cli_array[CLI_CAT_MAX][CLI_MAX_CMD_TBL_NUM];
static UINT32                   aui4_cli_tbl_cnt[CLI_CAT_MAX];
static CLI_GRP_T                at_cli_mod_grp_info[CLI_CAT_MAX][CLI_MAX_CMD_TBL_NUM];
static CHAR                     as_argv[CLI_MAX_ARG_NUM][CLI_MAX_ARG_LEN];
static CLI_EXEC_T*              apt_dir_link[CLI_MAX_CMD_TBL_LEVEL];
static UINT32                   ui4_dir_link_idx;
static CLI_EXEC_T*              pt_cur_cmd_tbl;
static BOOL                     b_is_cmd_tbl;
static CHAR*                    ps_cli_prompt_str;


/* Mandatory command table */
static CLI_EXEC_T at_mandatory_cmd_tbl[] =
{
 	{"cd",          NULL,   _cli_cmd_change_dir,        NULL,   "Change directory",                     CLI_HIDDEN},
    {"ls",          NULL,   _cli_cmd_list_cmd,          NULL,   "List commands",                        CLI_HIDDEN},
 	{"dis_log",     NULL,   _cli_cmd_set_DisablePrint,  NULL,   "disable layer app,driver,mmw,mw,sys",  CLI_HIDDEN},
    {"exec",        NULL,   _cli_cmd_exec,  			NULL,   "execute linux command",                CLI_GUEST},

 	END_OF_CLI_CMD_TBL
};

/* Application command table */
static CLI_EXEC_T at_app_cmd_tbl[] =
{
    {CLI_CAT_APP_STR,       NULL,                       NULL,       NULL,       "Application",      CLI_GUEST},
	END_OF_CLI_CMD_TBL
};

/* Middleware command table */
static CLI_EXEC_T at_mw_cmd_tbl[] =
{
	{CLI_CAT_MW_STR,        NULL,                       NULL,       NULL,       "Middleware",               CLI_GUEST},
	END_OF_CLI_CMD_TBL
};

/* uart0 cmd in the root layer and only need the first password */
static CLI_EXEC_T at_uart_root_cmd_table[] =
{
    {"setbaudrate",     "setbr",  _cli_cmd_uart0_set_baudrate,  NULL,   "Set uart baudrate",                        CLI_GUEST},
	END_OF_CLI_CMD_TBL
};

/*-----------------------------------------------------------------------------
                    function declarations
 ----------------------------------------------------------------------------*/
static BOOL _is_cmd_tbl_array(CLI_EXEC_T* pt_tbl);

static VOID _get_cmd_tbl_array(CLI_EXEC_T*   pt_tbl,
                               BOOL          b_skip_manda_tbl,
                               UINT32*       pui4_tbl_cnt,
                               CLI_EXEC_T*** pppt_cmd_tbl_array);

static BOOL _format_help_str(CHAR*       ps_dst,
                             UINT32      ui4_dst_len,
                             const CHAR* ps_cmd_str,
                             const CHAR* ps_cmd_abbr_str,
                             const CHAR* ps_cmd_help_str);

static VOID _show_help(CLI_EXEC_T* pt_tbl);

static INT32 _find_cmd_argv(const CHAR*  ps_cmd,
                            UINT32       ui4_argv_num,
                            UINT32       ui4_argv_len,
                            CHAR* const* pps_argv);

static CLI_EXEC_T* _search_tbl(const CHAR* ps_cmd,
                               CLI_EXEC_T* pt_tbl);

static INT32 _parse_cmd(INT32        i4_argc,
                        const CHAR** pps_argv,
                        CLI_EXEC_T*  pt_tbl);

static INT32 _generate_prompt(VOID);



/*----------------------------------------------------------------------------*/
static UINT32 _strlen(const CHAR* ps_str)
{
    if (ps_str == NULL)
    {
        return 0;
    }
    return strlen(ps_str);
}

static UINT32 _str2hex(const CHAR* pszStr, UINT32 u4Len)
{
	UINT32 u4Idx;
	UINT32 u4ReturnValue = 0;

	if ((pszStr == NULL) || (u4Len == 0))
	{
		return 0;
	}

	u4Len = (u4Len > 8) ? 8 : u4Len;

	for (u4Idx = 0;
		u4Idx < u4Len;
		u4Idx++)
	{
		if ((pszStr[u4Idx] >= '0') && (pszStr[u4Idx] <= '9'))
		{
			u4ReturnValue = u4ReturnValue << 4;
			u4ReturnValue += (UINT32)(UINT8)(pszStr[u4Idx] - '0');
		}
		else
		if ((pszStr[u4Idx] >= 'A') && (pszStr[u4Idx] <= 'F'))
		{
			u4ReturnValue = u4ReturnValue << 4;
			u4ReturnValue += (UINT32)(UINT8)(pszStr[u4Idx] - 'A' ) + 10;
		}
		else
		if ((pszStr[u4Idx] >= 'a') && (pszStr[u4Idx] <= 'f'))
		{
			u4ReturnValue = u4ReturnValue << 4;
			u4ReturnValue += (UINT32)(UINT8)(pszStr[u4Idx] - 'a') + 10;
		}
		else
		{
			return 0;
		}
	}

	return u4ReturnValue;
}

static UINT32 _str2dec(const CHAR* pszStr, UINT32 u4Len)
{
	UINT32 u4Idx;
	UINT32 u4ReturnValue = 0;

	if ((pszStr == NULL) || (u4Len == 0))
	{
		return 0;
	}

	// 0xFFFFFFFF = 4294967295
	u4Len = (u4Len > 10) ? 10 : u4Len;

	for (u4Idx = 0;
		u4Idx < u4Len;
		u4Idx++)
	{
		if ((pszStr[u4Idx] >= '0') && (pszStr[u4Idx] <= '9'))
		{
			u4ReturnValue *= 10;
			u4ReturnValue += (UINT32)(UINT8)(pszStr[u4Idx] - '0');
		}
		else
		{
			return 0;
		}
	}

	return u4ReturnValue;
}

static UINT32 _str2int(const CHAR* pszStr)
{
	UINT32 u4Len;

	if (pszStr == NULL)
	{
		return 0;
	}

	u4Len = _strlen(pszStr);

	if (u4Len > 2)
	{
		if ((pszStr[0] == '0') && (pszStr[1] == 'x'))
		{
			return _str2hex(&pszStr[2], u4Len - 2);
		}
	}

	return _str2dec(pszStr, u4Len);
}



/*-----------------------------------------------------------------------------
 * Name: _get_cmd_tbl_array
 *
 * Description: The API checks if input command table links to a command table
 *              array.
 *
 * Inputs:  pt_tbl      The command table to check.
 *
 * Outputs: -
 *
 * Returns: TRUE        This table links to a command table array.
 *          FALSE       This table doea not link to a command table array.
 ----------------------------------------------------------------------------*/
static BOOL _is_cmd_tbl_array(CLI_EXEC_T* pt_tbl)
{
    if ((pt_tbl == NULL)
        || (strcmp(pt_tbl->ps_cmd_str, CLI_CAT_APP_STR) == 0)
        || (strcmp(pt_tbl->ps_cmd_str, CLI_CAT_MW_STR) == 0))

    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}


/*-----------------------------------------------------------------------------
 * Name: _get_cmd_tbl_array
 *
 * Description: The API returns current available command table array and its
 *              element count.
 *
 * Inputs:  pt_tbl                  The command table to reference.
 *          b_skip_manda_tbl        Indicates if skipping mandatory command
 *                                  table is required.
 *
 * Outputs: pui4_tbl_cnt            The command table element count.
 *          pppt_cmd_tbl_array      The command table array.
 *
 * Returns: -
 ----------------------------------------------------------------------------*/
static VOID _get_cmd_tbl_array(CLI_EXEC_T*   pt_tbl,
                               BOOL          b_skip_manda_tbl,
                               UINT32*       pui4_tbl_cnt,
                               CLI_EXEC_T*** pppt_cmd_tbl_array)
{
    if (pt_tbl == NULL) /* Root */
    {
        if (b_skip_manda_tbl)
        {
            *pui4_tbl_cnt = aui4_cli_tbl_cnt[CLI_CAT_ROOT] - 1;
            *pppt_cmd_tbl_array = &(apt_cli_array[CLI_CAT_ROOT][CLI_MANDA_CMD_TBL_IDX + 1]);
        }
        else
        {
            *pui4_tbl_cnt = aui4_cli_tbl_cnt[CLI_CAT_ROOT];
            *pppt_cmd_tbl_array = &(apt_cli_array[CLI_CAT_ROOT][0]);
        }
    }
    else
    {
        if (strcmp(pt_tbl->ps_cmd_str, CLI_CAT_APP_STR) == 0)
        {
            *pui4_tbl_cnt = aui4_cli_tbl_cnt[CLI_CAT_APP];
            *pppt_cmd_tbl_array = &(apt_cli_array[CLI_CAT_APP][0]);
        }
        else if (strcmp(pt_tbl->ps_cmd_str, CLI_CAT_MW_STR) == 0)
        {
            *pui4_tbl_cnt = aui4_cli_tbl_cnt[CLI_CAT_MW];
            *pppt_cmd_tbl_array = &(apt_cli_array[CLI_CAT_MW][0]);
        }
        else
        {
            *pui4_tbl_cnt = 0;
            *pppt_cmd_tbl_array = NULL;
        }
    }
}


/*-----------------------------------------------------------------------------
 * Name: _format_help_str
 *
 * Description: The API formats help string for display.
 *
 * Inputs:  ui4_dst_len         Destination buffer size.
 *          ps_cmd_str          Command string.
 *          ps_cmd_abbr_str     Command abbreviation string.
 *          ps_cmd_help_str     Command description.
 *
 * Outputs: ps_dst              Destination buffer.
 *
 * Returns: TRUE                Routine successful.
 *          FALSE               Routine failed.
 ----------------------------------------------------------------------------*/
static BOOL _format_help_str(CHAR*       ps_dst,
                             UINT32      ui4_dst_len,
                             const CHAR* ps_cmd_str,
                             const CHAR* ps_cmd_abbr_str,
                             const CHAR* ps_cmd_help_str)
{
    UINT32      ui4_cmd_str_len;
    UINT32      ui4_cmd_abbr_str_len=0;
    UINT32      ui4_help_str_len=0;
    UINT32      ui4_idx;

    /* Check arguments */
	/* No need check abbr_str && help str, some item is NULL*/
    if ((ps_cmd_str == NULL))
    {
        return FALSE;
    }
	if((NULL == ps_cmd_abbr_str) && (NULL == ps_cmd_help_str))
	{
		CLI_DBG("ps_cmd_abbr_str and ps_cmd_help_str may be NULL\n\r");//just fix clockwork issue...infact, no need check this two attrib item.
	}
	
    /* Format => xxx(xxx):        xxxxxxxx */
    ui4_cmd_str_len = _strlen(ps_cmd_str);

    if((NULL != ps_cmd_abbr_str) || (NULL != ps_cmd_help_str))
    {
        ui4_cmd_abbr_str_len = _strlen(ps_cmd_abbr_str) + 2; /* (xxx) */
        ui4_help_str_len = _strlen(ps_cmd_help_str);
    }
    /* Check if destination buffer size is big enough */
    if (ui4_cmd_str_len + ui4_cmd_abbr_str_len + ui4_help_str_len + 1 > ui4_dst_len)
    {
        return FALSE;
    }

    /* Format command and command abbreviation strings */
    if (ps_cmd_abbr_str != NULL)
    {
         snprintf(ps_dst,ui4_dst_len, "%s(%s):", ps_cmd_str, ps_cmd_abbr_str);
    }
    else
    {
        snprintf(ps_dst,ui4_dst_len, "%s:", ps_cmd_str);
    }

    /* Copy command help string */
    if (ps_cmd_help_str != NULL)
    {
        for (ui4_idx = _strlen(ps_dst); ui4_idx < HELP_STR_POS; ui4_idx++)
        {
            ps_dst[ui4_idx] = ' ';
        }

        strncpy(ps_dst + HELP_STR_POS, ps_cmd_help_str,ui4_dst_len-HELP_STR_POS);
    }

    return TRUE;
}


/*-----------------------------------------------------------------------------
 * Name: _show_help
 *
 * Description: This API shows CLI command table help strings.
 *
 * Inputs:  pt_tbl      The command table.
 *
 * Outputs: -
 *
 * Returns: -
 ----------------------------------------------------------------------------*/
static VOID _show_help(CLI_EXEC_T* pt_tbl)
{
    UINT32          ui4_cmd_idx = 0;
    UINT32          ui4_tbl_cnt;
    CHAR            s_help[MAX_STR_LEN];
    CLI_EXEC_T*     pt_cmd_tbl;
    CLI_EXEC_T**    ppt_cmd_tbl_array;
    UINT32          ui4_idx;

	if (b_is_cmd_tbl) /* Commad table */
	{
        if (pt_tbl == NULL)
        {
            return;
        }

        dbg_print("[Help]\n\r");

		while (pt_tbl[ui4_cmd_idx].ps_cmd_help_str != NULL)
		{
		    if (_format_help_str(s_help,
                                 MAX_STR_LEN,
                                 pt_tbl[ui4_cmd_idx].ps_cmd_str,
                                 pt_tbl[ui4_cmd_idx].ps_cmd_abbr_str,
                                 pt_tbl[ui4_cmd_idx].ps_cmd_help_str) == TRUE)
            {
                if (pt_tbl[ui4_cmd_idx].e_access_right != CLI_HIDDEN)
                {
                    dbg_print(s_help);
                    dbg_print("\n\r");
                }
            }

            ui4_cmd_idx++;
		}
	}
	else /* Command table array */
	{
	    _get_cmd_tbl_array(pt_tbl, FALSE, &ui4_tbl_cnt, &ppt_cmd_tbl_array);

        if (ui4_tbl_cnt > 0)
        {
			dbg_print("[Help]\n\r");
        }

		for (ui4_idx = 0; ui4_idx < ui4_tbl_cnt; ui4_idx++)
		{
		    ui4_cmd_idx = 0;
		    pt_cmd_tbl = ppt_cmd_tbl_array[ui4_idx];
			while ((pt_cmd_tbl[ui4_cmd_idx].ps_cmd_str != NULL) ||
			       (pt_cmd_tbl[ui4_cmd_idx].ps_cmd_abbr_str != NULL) ||
			       (pt_cmd_tbl[ui4_cmd_idx].ps_cmd_help_str != NULL))
			{
			    if (_format_help_str(s_help,
			                         MAX_STR_LEN,
			                         pt_cmd_tbl[ui4_cmd_idx].ps_cmd_str,
			                         pt_cmd_tbl[ui4_cmd_idx].ps_cmd_abbr_str,
			                         pt_cmd_tbl[ui4_cmd_idx].ps_cmd_help_str) == TRUE)
                {
                    if (pt_cmd_tbl[ui4_cmd_idx].e_access_right != CLI_HIDDEN)
                    {
                        dbg_print(s_help);
                        dbg_print("\n\r");
                    }
                }

				ui4_cmd_idx++;
			}
		}
	}
}



/*-----------------------------------------------------------------------------
 * Name: _show_all_subdir_and_cmd
 *
 * Description: This API shows all CLI command table help strings.
 *
 * Inputs:  pt_tbl      The command table.
 *
 * Outputs: -
 *
 * Returns: -
 ----------------------------------------------------------------------------*/
static VOID _show_all_subdir_and_cmd(CLI_EXEC_T* pt_tbl)
{
    UINT32          ui4_cmd_idx = 0;
    UINT32          ui4_tbl_cnt;
    CHAR            s_help[MAX_STR_LEN];
    CLI_EXEC_T*     pt_cmd_tbl;
    CLI_EXEC_T**    ppt_cmd_tbl_array;
    UINT32          ui4_idx;

	if (b_is_cmd_tbl) /* Commad table */
	{
        if (pt_tbl == NULL)
        {
            return;
        }

		while (pt_tbl[ui4_cmd_idx].ps_cmd_help_str != NULL)
		{
		    if (_format_help_str(s_help,
                                 MAX_STR_LEN,
                                 pt_tbl[ui4_cmd_idx].ps_cmd_str,
                                 pt_tbl[ui4_cmd_idx].ps_cmd_abbr_str,
                                 pt_tbl[ui4_cmd_idx].ps_cmd_help_str) == TRUE)
            {
            	if(pt_tbl[ui4_cmd_idx].pf_exec_fct == NULL)
                {
                    dbg_print("cd ");
	            }
	            else
	            {
				    dbg_print("   ");
				}
                dbg_print(s_help);

				dbg_print("	right is %d", pt_tbl[ui4_cmd_idx].e_access_right);
				dbg_print("\n\r");

            }

            ui4_cmd_idx++;
		}
	}
	else /* Command table array */
	{
	    _get_cmd_tbl_array(pt_tbl, FALSE, &ui4_tbl_cnt, &ppt_cmd_tbl_array);

		dbg_print("[Help]\n\r");

		for (ui4_idx = 0; ui4_idx < ui4_tbl_cnt; ui4_idx++)
		{
		    ui4_cmd_idx = 0;
		    pt_cmd_tbl = ppt_cmd_tbl_array[ui4_idx];

			while ((pt_cmd_tbl[ui4_cmd_idx].ps_cmd_str != NULL) ||
			       (pt_cmd_tbl[ui4_cmd_idx].ps_cmd_abbr_str != NULL) ||
			       (pt_cmd_tbl[ui4_cmd_idx].ps_cmd_help_str != NULL))
			{
			    if (_format_help_str(s_help,
			                         MAX_STR_LEN,
			                         pt_cmd_tbl[ui4_cmd_idx].ps_cmd_str,
			                         pt_cmd_tbl[ui4_cmd_idx].ps_cmd_abbr_str,
			                         pt_cmd_tbl[ui4_cmd_idx].ps_cmd_help_str) == TRUE)
                {

					if(pt_cmd_tbl[ui4_cmd_idx].pf_exec_fct == NULL)
					{
						dbg_print("cd ");
					}
					else
					{
						dbg_print("   ");
					}
					dbg_print(s_help);

					dbg_print("	right is %d", pt_cmd_tbl[ui4_cmd_idx].e_access_right);
					dbg_print("\n\r");

                }

				ui4_cmd_idx++;
			}
		}
	}
}




/*-----------------------------------------------------------------------------
 * Name: _search_tbl
 *
 * Description: This API finds out a command table corresponding to the
 *              command.
 *
 * Inputs:  ps_cmd      The command to search.
 *          pt_tbl      The command table for search.
 *
 * Outputs: -
 *
 * Returns: NULL        No matched command table found.
 *          Other       The address of matched command table.
 ----------------------------------------------------------------------------*/
static CLI_EXEC_T* _search_tbl(const CHAR* ps_cmd,
                               CLI_EXEC_T* pt_tbl)
{
	const CHAR*     ps_dir_str;
	UINT32          ui4_dir_str_len;
	UINT32          ui4_cmd_len;
	UINT32          ui4_cmd_abbr_len;
	UINT32          ui4_cmd_idx;
	BOOL            b_found = FALSE;

    /* Check arguments */
	if ((ps_cmd == NULL) ||
	    (pt_tbl == NULL))
	{
		return NULL;
	}

	ui4_dir_str_len = 0;
	ps_dir_str = ps_cmd;
	while ((!IS_DOT(*ps_dir_str)) &&
		   (*ps_dir_str != ASCII_NULL))
	{
		ui4_dir_str_len++;
		ps_dir_str++;
	}

    /* Search commmad from current command table */
    ui4_cmd_idx = 0;
	while (pt_tbl[ui4_cmd_idx].ps_cmd_str != NULL)
	{
	    /* Compare command string */
		ui4_cmd_len = _strlen(pt_tbl[ui4_cmd_idx].ps_cmd_str);
		if ((ui4_dir_str_len == ui4_cmd_len) &&
			(strncmp(pt_tbl[ui4_cmd_idx].ps_cmd_str, ps_cmd, ui4_dir_str_len) == 0))
		{
			b_found = TRUE;
			break;
		}

        /* Compare command abbreviation string */
		if (pt_tbl[ui4_cmd_idx].ps_cmd_abbr_str != NULL)
		{
			ui4_cmd_abbr_len = _strlen(pt_tbl[ui4_cmd_idx].ps_cmd_abbr_str);

			if ((ui4_dir_str_len == ui4_cmd_abbr_len) &&
				(strncmp(pt_tbl[ui4_cmd_idx].ps_cmd_abbr_str, ps_cmd, ui4_dir_str_len) == 0))
			{
				b_found = TRUE;
				break;
			}
		}

		ui4_cmd_idx++;
	}

    /* Return the matched command structure */
	if (b_found)
	{
		return &pt_tbl[ui4_cmd_idx];
	}

	return NULL;
}


/*-----------------------------------------------------------------------------
 * Name: _parse_cmd
 *
 * Description: This API parses and executes a command.
 *
 * Inputs:  i4_argc         Contains the argument count.
 *          pps_argv        Contains the arguments.
 *          pt_tbl          The command table to start parsing.
 *
 * Outputs: -
 *
 * Returns: CLIR_OK                 Routine successful.
 *          CLIR_INV_CMD_USAGE      Invalid command usage.
 *          CLIR_INV_ARG            One or more invalid arguments.
 *          CLIR_INV_CMD_TBL        Invalid CLI command table format.
 *          CLIR_CMD_NOT_FOUND      CLI command not found.
 *          CLIR_DIR_NOT_FOUND      CLI directory not found.
 ----------------------------------------------------------------------------*/
static INT32 _parse_cmd(INT32        i4_argc,
                        const CHAR** pps_argv,
                        CLI_EXEC_T*  pt_tbl)
{
    UINT32          ui4_idx;
	const CHAR*     ps_dir_str;
	CLI_EXEC_T*     pt_cmd_tbl = NULL;
    UINT32          ui4_tbl_cnt;
    CLI_EXEC_T**    ppt_cmd_tbl_array;
    BOOL            b_cmd_tbl_orig;
    /* Check arguments */
	if (pps_argv == NULL)
	{
		return CLIR_INV_ARG;
	}

	ps_dir_str = pps_argv[0];
	while ((!IS_DOT(*ps_dir_str)) &&
		   (*ps_dir_str != ASCII_NULL))
	{
		ps_dir_str++;
	}

    if (_is_cmd_tbl_array(pt_tbl) == TRUE)
    {
        _get_cmd_tbl_array(pt_tbl, FALSE, &ui4_tbl_cnt, &ppt_cmd_tbl_array);

    	for (ui4_idx = 0; ui4_idx < ui4_tbl_cnt; ui4_idx++)
    	{
    	    pt_cmd_tbl = _search_tbl(pps_argv[0], ppt_cmd_tbl_array[ui4_idx]);

    		if (pt_cmd_tbl != NULL)
    		{
    			break;
    		}
    	}
    }
    else
    {
        pt_cmd_tbl = _search_tbl(pps_argv[0], pt_tbl);
    }

    /* Execute command */
    if (pt_cmd_tbl != NULL)
    {
 		if ((pt_cmd_tbl->pt_next_level == NULL) &&
 		    (pt_cmd_tbl->pf_exec_fct == NULL))
 		{
            if (_is_cmd_tbl_array(pt_cmd_tbl) != TRUE)
            {
                return CLIR_INV_CMD_TBL;
            }
 		}

		if (pt_cmd_tbl->pf_exec_fct != NULL)
		{
			if ((pt_cmd_tbl->pt_next_level == NULL) ||
				(!IS_DOT(*ps_dir_str) && (i4_argc > 1))) /* Entry can be a directory or a command */
			{
				return (pt_cmd_tbl->pf_exec_fct(i4_argc,
				                                pps_argv));
			}
		}

 		if (pt_cmd_tbl->pt_next_level != NULL)
		{
			if (*ps_dir_str == ASCII_NULL)
			{
			    /* Show CLI help if command contains only directory or module name */
			    if (i4_argc == 1)
			    {
                    if (_is_cmd_tbl_array(pt_cmd_tbl) == TRUE)
                    {
                        _show_help(pt_cmd_tbl);
                    }
                    else
                    {
                        b_cmd_tbl_orig = b_is_cmd_tbl;
                        b_is_cmd_tbl = TRUE;
    				    _show_help(pt_cmd_tbl->pt_next_level);
    				    b_is_cmd_tbl = b_cmd_tbl_orig;
                    }

                    return CLIR_INV_CMD_USAGE;
                }
                else
				{
				    return CLIR_CMD_NOT_FOUND;
				}
			}

			pps_argv[0] = ++ps_dir_str; /* Skip '.' character */

			/* Go to next level */
    		if (_is_cmd_tbl_array(pt_cmd_tbl) == TRUE)
    		{
    		    return (_parse_cmd(i4_argc, pps_argv, pt_cmd_tbl));
			}
			else
			{
			    return (_parse_cmd(i4_argc, pps_argv, pt_cmd_tbl->pt_next_level));
			}
		}
    }

	return CLIR_CMD_NOT_FOUND;
}


/*-----------------------------------------------------------------------------
 * Name: _generate_prompt
 *
 * Description: This API generates CLI prompt string according to current
 *              directory level.
 *
 * Inputs:  -
 *
 * Outputs: -
 *
 * Returns: -
 ----------------------------------------------------------------------------*/
static INT32 _generate_prompt(VOID)
{
	UINT32      ui4_total_len = 0;
	UINT32      ui4_dir_str_len;
	CHAR*       ps_dir_str;
	UINT32      ui4_idx;

    //set cli prompt
    //Get Root cli prompt.
    ps_cli_prompt_str = cli_get_prompt_str_buf();
	CLI_ASSERT(ps_cli_prompt_str != NULL);

    ps_cli_prompt_str[0] = ASCII_NULL;

	strncat(ps_cli_prompt_str, CLI_PROMPT_STR, sizeof(CLI_PROMPT_STR));

    //Get sub path dir prompt.
	for (ui4_idx = 1; ui4_idx <= ui4_dir_link_idx; ui4_idx++)
	{
		if (apt_dir_link[ui4_idx]->ps_cmd_abbr_str != NULL)
		{
			ps_dir_str = apt_dir_link[ui4_idx]->ps_cmd_abbr_str;
		}
		else
		{
			ps_dir_str = apt_dir_link[ui4_idx]->ps_cmd_str;
		}

		ui4_dir_str_len = _strlen(ps_dir_str);

		if (ui4_total_len + ui4_dir_str_len > CLI_CMD_BUF_SIZE)
		{
            strncat(ps_cli_prompt_str, ">", 1);
			return 0;
		}

		strncat(ps_cli_prompt_str, ".", 1);
		strncat(ps_cli_prompt_str, ps_dir_str, (ui4_dir_str_len + 1));
		ui4_total_len += (ui4_dir_str_len + 1); /* .xxx */
	}

	strncat(ps_cli_prompt_str, ">", 1);

	return 0;
}



/*-----------------------------------------------------------------------------
 * Name: _cli_parser_init
 *
 * Description: This API is used to init cli parser module
 *
 * Inputs:  NULL
 *
 * Outputs: NULL
 *
 * Returns:  NULL
 ----------------------------------------------------------------------------*/
static void _cli_parser_init(void)
{
    /* Init internal variables */
    ui4_dir_link_idx = 0;
    apt_dir_link[0] = NULL;
    pt_cur_cmd_tbl = NULL;
    b_is_cmd_tbl = FALSE;
}

static VOID _cli_set_cli_prompt(VOID)
{
    /* Set up prompt string */
    ps_cli_prompt_str = cli_get_prompt_str_buf();
    ps_cli_prompt_str[0] = ASCII_NULL;

	strncat(ps_cli_prompt_str, CLI_PROMPT_STR, sizeof(CLI_PROMPT_STR));

    strncat(ps_cli_prompt_str, ">", 1);
}

/*-----------------------------------------------------------------------------
 * Name: _find_cmd_argv
 *
 * Description: The API finds all the arguments of a command string and stores
 *              them into argument buffer.
 *
 * Inputs:  ps_cmd              Contains the command to be parse.
 *          ui4_argv_num        The maximum argument number that argument
 *                              buffer supports.
 *          ui4_argv_len        The maximum length per argument.
 *
 * Outputs: pps_argv            Points to argument buffer.
 *
 * Returns: Number of arguments found.
 ----------------------------------------------------------------------------*/
static INT32 _find_cmd_argv(const CHAR*  ps_cmd,
                            UINT32       ui4_argv_num,
                            UINT32       ui4_argv_len,
                            CHAR* const* pps_argv)
{
    INT32       i4_argc = 0;
    CHAR*       ps_argv;
    CHAR        c_char;
    UINT32      ui4_cmd_idx;
	UINT8       ui1_quote_state; /* 0: no quote found
	                                1: first single quote (') found
	                                2: first double quota (") found  */

    /* Check arguments */
	if ((ps_cmd == NULL) ||
	    (ui4_argv_num == 0) ||
	    (ui4_argv_len == 0) ||
	    (pps_argv == NULL))
	{
		return 0;
	}

    /* Start finding arguments of a command */
    c_char = *ps_cmd;
    while ((c_char != ASCII_NULL) &&
           (c_char != ASCII_KEY_PUNCH))
    {
        ps_argv = pps_argv[i4_argc];

		/* Search the first non-space and printable character */
	    while (!IS_PRINTABLE(c_char) || IS_SPACE(c_char))
	    {
	        c_char = *(++ps_cmd);
	    }

        /* Grab an argument */
        ui4_cmd_idx = 0;
		ui1_quote_state = 0;
		while (IS_PRINTABLE(c_char) &&
               ((ui1_quote_state == 0 && !IS_SPACE(c_char)) ||
				(ui1_quote_state == 1 && c_char != ASCII_KEY_SGL_QUOTE) ||
				(ui1_quote_state == 2 && c_char != ASCII_KEY_DBL_QUOTE)))
        {
			if (c_char == ASCII_NULL ||             /* End of string */
			    ui4_cmd_idx >= (ui4_argv_len - 1))	/* Exceed maximum argument length */
			{
	        	*ps_argv = ASCII_NULL;
				break;
			}

            /* Take care of quote issue */
			if ((ui1_quote_state != 2) &&
			    (c_char == ASCII_KEY_SGL_QUOTE))
			{
				if (ui1_quote_state == 0)
				{
					ui1_quote_state = 1;
				}
				else if (ui1_quote_state == 1)
				{
					ui1_quote_state = 0;
				}
	        	c_char = *(++ps_cmd);
				ui4_cmd_idx++;
				continue;
			}
			else if ((ui1_quote_state != 1) &&
			         (c_char == ASCII_KEY_DBL_QUOTE))
			{
				if (ui1_quote_state == 0)
				{
					ui1_quote_state = 2;
				}
				else if (ui1_quote_state == 2)
				{
					ui1_quote_state = 0;
				}
	        	c_char = *(++ps_cmd);
				ui4_cmd_idx++;
				continue;
			}

            /* Copy the character into argument buffer */
            *ps_argv = c_char;
            ps_argv++;
            c_char = *(++ps_cmd);
            ui4_cmd_idx++;
        }

		if ((c_char == ASCII_KEY_SGL_QUOTE) ||
		    (c_char == ASCII_KEY_DBL_QUOTE))
		{
            c_char = *(++ps_cmd);
            ui4_cmd_idx++;
		}

		if (ui4_cmd_idx > 0)
		{
	        *ps_argv = ASCII_NULL;
	        i4_argc++;
		}

        if ((UINT32)i4_argc >= ui4_argv_num)
		{
			break;
		}
    }

	return i4_argc;
}

/*-----------------------------------------------------------------------------
 ----------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------
 * Name: _cli_cmd_change_dir
 *
 * Description: This API changes current command level.
 *
 * Inputs:  i4_argc     Number of arguments.
 *          pps_argv    Points to the argument array.
 *
 * Outputs: -
 *
 * Returns: CLIR_OK                 Routine successful.
 *          CLIR_INV_CMD_USAGE      Invalid command usage.
 ----------------------------------------------------------------------------*/
static INT32 _cli_cmd_change_dir(INT32 i4_argc, const CHAR** pps_argv)
{
	const CHAR*     ps_dir_str;
	const CHAR*     ps_cur_dir;
	CLI_EXEC_T*     pt_tbl;
    UINT32          ui4_tbl_cnt;
    CLI_EXEC_T**    ppt_cmd_tbl_array;
	UINT32          ui4_idx;

    /* cd [directory path] */

    /* Check arguments */
	if ((i4_argc != 2) ||
	    (pps_argv == NULL) ||
	    (pps_argv[1] == NULL))
	{
		_show_help(pt_cur_cmd_tbl);
		return CLIR_INV_CMD_USAGE;
	}

	ps_dir_str = pps_argv[1];
	while (*ps_dir_str != ASCII_NULL)
	{
		if (IS_DOT(*ps_dir_str))
		{
			ps_dir_str++;
			if (IS_DOT(*ps_dir_str)) /* cd .. */
			{
				ps_dir_str++;
				if (ui4_dir_link_idx > 0)
				{
					ui4_dir_link_idx--;

					if (ui4_dir_link_idx == 0)
					{
						pt_cur_cmd_tbl = NULL;
						b_is_cmd_tbl = FALSE;
					}
					else
					{
    					/* Check if currect directory points to a command
    					   table or command table array */
                        pt_tbl = apt_dir_link[ui4_dir_link_idx];
                		if (_is_cmd_tbl_array(pt_tbl) == TRUE)
                		{
                		    pt_cur_cmd_tbl = pt_tbl;
                            b_is_cmd_tbl = FALSE;
                		}
                		else
                		{
                		    pt_cur_cmd_tbl = pt_tbl->pt_next_level;
                		    b_is_cmd_tbl = TRUE;
               		    }
					}
					_generate_prompt();
                    _show_help(pt_cur_cmd_tbl);

				}
			}
			else
			{
				return CLIR_DIR_NOT_FOUND;
			}
 		}
		else
		{
    		if (IS_ROOT(*ps_dir_str)) /* cd / */
    		{
    		    ps_dir_str++;
    			ui4_dir_link_idx = 0;
    			pt_cur_cmd_tbl = NULL;
    			b_is_cmd_tbl = FALSE;
    			_generate_prompt();
                _show_help(pt_cur_cmd_tbl);

    		}
    		else /* cd xxx.xxx.xxx... */
    		{
    			ps_cur_dir = ps_dir_str;
    			if (b_is_cmd_tbl) /* Command table */
    			{
    			    pt_tbl = _search_tbl(ps_cur_dir, pt_cur_cmd_tbl);
    			}
    			else /* Command table array */
    			{
    			    pt_tbl = NULL;
    			    _get_cmd_tbl_array(pt_cur_cmd_tbl, TRUE, &ui4_tbl_cnt, &ppt_cmd_tbl_array);

            		for (ui4_idx = 0; ui4_idx < ui4_tbl_cnt; ui4_idx++)
            		{
            		    pt_tbl = _search_tbl(ps_cur_dir, ppt_cmd_tbl_array[ui4_idx]);

    					if (pt_tbl != NULL)
    					{
    						break;
    					}
    				}
    			}
    			if ((pt_tbl != NULL) && (pt_tbl->pt_next_level != NULL))
    			{
    				if (ui4_dir_link_idx < (CLI_MAX_CMD_TBL_LEVEL - 1))
    				{
    				    /* Print out current directory path */
    					ui4_dir_link_idx++;
    					apt_dir_link[ui4_dir_link_idx] = pt_tbl;
    					_generate_prompt();

    					/* Check if currect directory points to a command
    					   table or command table array */
                		if (_is_cmd_tbl_array(pt_tbl) == TRUE)
                		{
                		    pt_cur_cmd_tbl = pt_tbl;
                            b_is_cmd_tbl = FALSE;
                		}
                		else
                		{
                		    pt_cur_cmd_tbl = pt_tbl->pt_next_level;
                		    b_is_cmd_tbl = TRUE;
               		    }
                        _show_help(pt_cur_cmd_tbl);

    				}
    				else
    				{
    				    /* There is something wrong => change directory back to root */
            			ui4_dir_link_idx = 0;
            			pt_cur_cmd_tbl = NULL;
            			b_is_cmd_tbl = FALSE;
            			_generate_prompt();
                        _show_help(pt_cur_cmd_tbl);

    				}
    			}
    			else
    			{
    				return CLIR_DIR_NOT_FOUND;
    			}

                /* Proceed next level directory path */
    			while (!IS_DOT(*ps_dir_str) && (*ps_dir_str != ASCII_NULL))
    			{
    				ps_dir_str++;
    			}

    			/* Skip '.' character */
    			if (IS_DOT(*ps_dir_str))
    			{
    			    ps_dir_str++;
    			}
    		}
        }
	}

	return CLIR_OK;
}


/*-----------------------------------------------------------------------------
 * Name: _cli_cmd_list_cmd
 *
 * Description: This API list commands at current command level.
 *
 * Inputs:  i4_argc     Number of arguments.
 *          pps_argv    Points to the argument array.
 *
 * Outputs: -
 *
 * Returns: CLIR_OK                 Routine successful.
 *          CLIR_INV_CMD_USAGE      Invalid command usage.
 ----------------------------------------------------------------------------*/
static INT32 _cli_cmd_list_cmd(INT32 i4_argc, const CHAR** pps_argv)
{
    /* ls */
	if (2 == i4_argc && 0 == strcmp(pps_argv[1], "all"))
	{
		_show_all_subdir_and_cmd(pt_cur_cmd_tbl);
	}
	else
	{
		_show_help(pt_cur_cmd_tbl);
	}

	return CLIR_OK;
}

/*-----------------------------------------------------------------------------
 * Name: _cli_cmd_set_admin_access_right
 *
 * Description: This API changes system access right.
 *
 * Inputs:  i4_argc     Number of arguments.
 *          pps_argv    Points to the argument array.
 *
 * Outputs: -
 *
 * Returns: CLIR_OK                 Routine successful.
 *          CLIR_INV_CMD_USAGE      Invalid command usage.
 ----------------------------------------------------------------------------*/
static INT32 _cli_cmd_set_DisablePrint(INT32 i4_argc, const CHAR** pps_argv)
{
	int i=0;
	UINT32 app_en=0,driver_en=0,mmw_en=0,mw_en=0,sys_en=0;

	for (i=1;i<=i4_argc;i++)
	{
		if (strcmp (pps_argv[i], "app") == 0)
		{
			app_en=DBG_LAYER_APP;
		}
		else if(strcmp (pps_argv[i], "mw") == 0)
		{
			mw_en=DBG_LAYER_MW;
		}
		else if(strcmp (pps_argv[i], "sys") == 0)
		{
			sys_en=DBG_LAYER_SYS;
		}
		else if(strcmp (pps_argv[i], "on") == 0)
		{
			 sys_en=DBG_LAYER_ALL;
		}
		else
             {
			i=i4_argc+1;
		}

	}
	ui4_disable_print = app_en | driver_en | mmw_en | mw_en | sys_en;

    return CLIR_OK;

}


//used to execute linux shell cmd
static INT32 _cli_cmd_exec(INT32        i4_argc,
                                       const CHAR** pps_argv)
{
    /* Check arguments */
	if (i4_argc <2 )
	{
		dbg_print("Invalid cmd! \ne.g: exec programx\n");
		return CLIR_OK;
	}
	

    CHAR  szBuf[256];
    memset(szBuf,0, 256);
	switch(i4_argc)
	{
	case 2:
		snprintf(szBuf,256,"%s", (CHAR * )(pps_argv[1]));
		break;
	case 3:
		snprintf(szBuf,256,"%s %s", (CHAR * )(pps_argv[1]),(CHAR * )(pps_argv[2]));
		break;
	case 4:
		snprintf(szBuf,256,"%s %s %s", (CHAR * )(pps_argv[1]),(CHAR * )(pps_argv[2]),(CHAR * )(pps_argv[3]));
		break;
	case 5:
		snprintf(szBuf,256,"%s %s %s %s", (CHAR * )(pps_argv[1]),(CHAR * )(pps_argv[2]),(CHAR * )(pps_argv[3]),(CHAR * )(pps_argv[4]));
		break;		
	case 6:
		snprintf(szBuf,256,"%s %s %s %s %s", (CHAR * )(pps_argv[1]),(CHAR * )(pps_argv[2]),(CHAR * )(pps_argv[3]),(CHAR * )(pps_argv[4]),(CHAR * )(pps_argv[5]));
		break;	
    default:
       dbg_print("no arg \n"); 

	}
	
    system(szBuf);
    dbg_print("call %s\n", szBuf);

    return CLIR_OK;
}

static INT32 _cli_cmd_uart0_set_baudrate(INT32 i4_argc, const CHAR** pps_argv)
{
    INT32 i4BaudRate = 0;
    int  i4_baud=B115200;
    int tty = 0;
    struct termios options;
    
    if(i4_argc < 2)
    {
		dbg_print("Err:set baudrate, please give the bandrate value\n");
        return CLIR_OK;
    }
    CLI_DBG("setbr baudrate\n");
    
    i4BaudRate = _str2int((CHAR * )(pps_argv[1]));

    switch(i4BaudRate)
    {
        case 115200:
            i4_baud=B115200;
            break;
        case 230400:
            i4_baud=B230400;
            break;
        case 460800:
            i4_baud=B460800;
            break;
        case 921600:
            i4_baud=B921600;
            break;
        case 57600:
            i4_baud=B57600;
            break;
        case 28800:
            //i4_baud=B28800;
            break;
        case 9600:
            i4_baud=B9600;
            break;
        default:
            break;
    }
    tty = open("/dev/ttyMT0", O_RDWR | O_NOCTTY);
    fcntl(tty, F_SETFL, 0);
    tcgetattr(tty, &options);
        
    cfsetispeed(&options, i4_baud);
    cfsetospeed(&options, i4_baud);
    tcsetattr(tty, TCSANOW, &options);
    close(tty);
    
    return CLIR_OK;
}


/*-----------------------------------------------------------------------------
 * Name: cli_parser
 *
 * Description: This API parses a CLI command and performs corresponding
 *              operation.
 *
 * Inputs:  ps_cmd      Contain the command to parse.
 *
 * Outputs: -
 *
 * Returns: CLIR_OK                 Routine successful.
 *          CLIR_INV_ARG            One or more invalid arguments.
 *          CLIR_UNKNOWN_CMD        Unknown CLI command.
 *          CLIR_CMD_NOT_FOUND      CLI command not found.
 *          CLIR_DIR_NOT_FOUND      CLI directory not found.
 *          CLIR_NOT_ENABLED        CLI is not enabled.
 ----------------------------------------------------------------------------*/
INT32 cli_parser(const CHAR* ps_cmd)
{
	INT32           i4_return = 0;
	INT32           i4_argc;
	UINT32          ui4_idx;
	CHAR*           ps_argv[CLI_MAX_ARG_NUM];
	

    /* Check arguments */
	if (ps_cmd == NULL)
	{
		return CLIR_INV_ARG;
	}

    /* Init argument buffer */
	for (ui4_idx = 0; ui4_idx < CLI_MAX_ARG_NUM; ui4_idx++)
	{
   		ps_argv[ui4_idx] = as_argv[ui4_idx];
   		as_argv[ui4_idx][0] = 0;
	}

    /* Get all the arguments of the command */
	i4_argc = _find_cmd_argv(ps_cmd,
	                         CLI_MAX_ARG_NUM,
	                         CLI_MAX_ARG_LEN,
	                         ps_argv);


	if (i4_argc > 0)
	{

		/* Command parsing: mandatory table */
		i4_return = _parse_cmd(i4_argc,
		                       (const CHAR**)ps_argv,
		                       apt_cli_array[CLI_CAT_ROOT][CLI_MANDA_CMD_TBL_IDX]);


		if (i4_return != CLIR_CMD_NOT_FOUND)
		{
			return i4_return;
		}
		else
		{
    		/* Command parsing: other command tables */
    		if (i4_argc > 0)
    		{
				i4_return = _parse_cmd(i4_argc,
				                       (const CHAR**)ps_argv,
				                       pt_cur_cmd_tbl);

				if (i4_return != CLIR_CMD_NOT_FOUND)
				{
					return i4_return;
				}
    		}
        }
	}

    if ((i4_argc > 0) && (i4_return == CLIR_CMD_NOT_FOUND))
    {
	    return i4_return;
	}
	else
	{
    	/* Display CLI help */
        _show_help(pt_cur_cmd_tbl);

	    return CLIR_OK;
	}
}

/*-----------------------------------------------------------------------------
 * Name: cli_parser_attach_cmd_tbl
 *
 * Description: This API attaches a command table to another command table.
 *
 * Inputs:  pt_tbl          The command table to be attached.
 *          e_category      The category that the command table belongs to.
 *          ui8_group_mask  The group(s) that the command table belongs to.
 *
 * Outputs: -
 *
 * Returns: CLIR_OK                 Routine successful.
 *          CLIR_INV_ARG            One or more invalid arguments.
 *          CLIR_CMD_TBL_FULL       Command table is full.
 *          CLIR_GROUP_TBL_FULL     Module-to-group table is full.
 *          CLIR_NOT_ENABLED        CLI is not enabled.
 ----------------------------------------------------------------------------*/
INT32 cli_parser_attach_cmd_tbl(CLI_EXEC_T* pt_tbl,
                                CLI_CAT_T   e_category,
                                UINT64      ui8_group_mask)
{
    UINT32          ui4_idx;
    UINT32*         pui4_tbl_cnt;
    CLI_EXEC_T*     pt_cmd_tbl = NULL;
    CLI_EXEC_T**    ppt_cmd_tbl_array;
    CLI_GRP_T*      pt_mod_grp_info;
    CLI_EXEC_T*     pt_tbl_tmp_a = NULL;
    CLI_EXEC_T*     pt_tbl_tmp_b = NULL;
    UINT64          ui8_group_mask_tmp_a = 0;
    UINT64          ui8_group_mask_tmp_b = 0;
    BOOL            b_replace_cmd_tbl = FALSE;
    BOOL            b_sort_cmd_tbl = FALSE;
    BOOL            b_swap = FALSE;
	CLI_DBG("----------attach pt_tbl %s----------\n", pt_tbl->ps_cmd_str);

    /* Check arguments */
	if ((pt_tbl == NULL) ||
	    (pt_tbl->ps_cmd_str == NULL) ||
	    (e_category >= CLI_CAT_MAX))
	{
		return CLIR_INV_ARG;
	}

    /* Attach the command table to requested category */
    switch (e_category)
    {
        case CLI_CAT_ROOT:
            pui4_tbl_cnt = &aui4_cli_tbl_cnt[CLI_CAT_ROOT];
            ppt_cmd_tbl_array = &(apt_cli_array[CLI_CAT_ROOT][0]);
            pt_mod_grp_info = &(at_cli_mod_grp_info[CLI_CAT_ROOT][0]);
            break;

        case CLI_CAT_APP:
            pui4_tbl_cnt = &aui4_cli_tbl_cnt[CLI_CAT_APP];
            pt_cmd_tbl = at_app_cmd_tbl;
            ppt_cmd_tbl_array = &(apt_cli_array[CLI_CAT_APP][0]);
            pt_mod_grp_info = &(at_cli_mod_grp_info[CLI_CAT_APP][0]);
            break;

        case CLI_CAT_MW:
            pui4_tbl_cnt = &aui4_cli_tbl_cnt[CLI_CAT_MW];
            pt_cmd_tbl = at_mw_cmd_tbl;
            ppt_cmd_tbl_array = &(apt_cli_array[CLI_CAT_MW][0]);
            pt_mod_grp_info = &(at_cli_mod_grp_info[CLI_CAT_MW][0]);
            break;

        default:
            return CLIR_INV_ARG;
    }

    /* Check if buffer size is ok */
	if (*pui4_tbl_cnt >= CLI_MAX_CMD_TBL_NUM)
	{
		return CLIR_CMD_TBL_FULL;
	}

	/* Check if command table sorting is required */
    if (*pui4_tbl_cnt > 0 && e_category > CLI_CAT_ROOT)
    {
        b_sort_cmd_tbl = TRUE;
    }

	/* If the command table is duplicated, replace existed commad table with new one */
	for (ui4_idx = 0; ui4_idx < *pui4_tbl_cnt; ui4_idx++)
	{

		if ((strcmp(pt_tbl->ps_cmd_str, ppt_cmd_tbl_array[ui4_idx]->ps_cmd_str) == 0))
        {
        	if((ppt_cmd_tbl_array[ui4_idx]->ps_cmd_abbr_str) == NULL)//this case means find cli table,but abbr str set as null.
        	{
				b_replace_cmd_tbl = TRUE;
            	break;
			}else if((strcmp(pt_tbl->ps_cmd_abbr_str, ppt_cmd_tbl_array[ui4_idx]->ps_cmd_abbr_str) == 0))//this case means find cli table,but abbr str had set
			{
	            b_replace_cmd_tbl = TRUE;
			}
            break;
        }
    }

    if (b_replace_cmd_tbl)
    {
        ppt_cmd_tbl_array[ui4_idx] = pt_tbl;
        pt_mod_grp_info[ui4_idx].pt_cmd_tbl = pt_tbl;
        pt_mod_grp_info[ui4_idx].aui8_grp_msk = ui8_group_mask;

        if (*pui4_tbl_cnt == 0 && e_category > CLI_CAT_ROOT)
        {
            pt_cmd_tbl[0].pt_next_level = pt_tbl;
        }
    }
    else
    {
        ui4_idx = *pui4_tbl_cnt;
        (*pui4_tbl_cnt)++;//update the array for count
        if (b_sort_cmd_tbl)
        {
            /* Start sortig */
        	for (ui4_idx = 0; ui4_idx < *pui4_tbl_cnt; ui4_idx++)
        	{
        	    if (!b_swap && ui4_idx < *pui4_tbl_cnt - 1)
        	    {
            	    if (strcmp(pt_tbl->ps_cmd_str, ppt_cmd_tbl_array[ui4_idx]->ps_cmd_str) == -1)
            	    {
            	        b_swap = TRUE;
            	        pt_tbl_tmp_b = ppt_cmd_tbl_array[ui4_idx];
            	        ui8_group_mask_tmp_b = pt_mod_grp_info[ui4_idx].aui8_grp_msk;

                        ppt_cmd_tbl_array[ui4_idx] = pt_tbl;
                        pt_mod_grp_info[ui4_idx].pt_cmd_tbl = pt_tbl;
                        pt_mod_grp_info[ui4_idx].aui8_grp_msk = ui8_group_mask;

                        if (ui4_idx == 0 && e_category > CLI_CAT_ROOT)
                        {
                            pt_cmd_tbl[0].pt_next_level = pt_tbl;
                        }
            	    }
            	}
        	    else
        	    {
        	        if (b_swap)
        	        {
            	        /* Do command table swap */
            	        pt_tbl_tmp_a = ppt_cmd_tbl_array[ui4_idx];
            	        ui8_group_mask_tmp_a = pt_mod_grp_info[ui4_idx].aui8_grp_msk;
            	        ppt_cmd_tbl_array[ui4_idx] = pt_tbl_tmp_b;
            	        pt_mod_grp_info[ui4_idx].pt_cmd_tbl = pt_tbl_tmp_b;
            	        pt_mod_grp_info[ui4_idx].aui8_grp_msk = ui8_group_mask_tmp_b;
            	        pt_tbl_tmp_b = pt_tbl_tmp_a;
            	        ui8_group_mask_tmp_b = ui8_group_mask_tmp_a;
                    }
                    else
                    {
                        ppt_cmd_tbl_array[ui4_idx] = pt_tbl;
                        pt_mod_grp_info[ui4_idx].pt_cmd_tbl = pt_tbl;
                        pt_mod_grp_info[ui4_idx].aui8_grp_msk = ui8_group_mask;
                    }
        	    }
        	}
        }
        else
        {
            ppt_cmd_tbl_array[ui4_idx] = pt_tbl;
            pt_mod_grp_info[ui4_idx].pt_cmd_tbl = pt_tbl;
            pt_mod_grp_info[ui4_idx].aui8_grp_msk = ui8_group_mask;

            if (ui4_idx == 0 && e_category > CLI_CAT_ROOT)
            {
                pt_cmd_tbl[0].pt_next_level = pt_tbl;
            }
        }
    }
	return CLIR_OK;

}


/*-----------------------------------------------------------------------------
 * Name: cli_parser_detach_cmd_tbl
 *
 * Description: This API detaches a command table.
 *
 * Inputs:  pt_tbl          The command table to be detached.
 *          e_category      The category that the command table belongs to.
 *          ui8_group_mask  The group(s) that the command table belongs to.
 *
 * Outputs: -
 *
 * Returns: CLIR_OK                 Routine successful.
 *          CLIR_INV_ARG            One or more invalid arguments.
 *          CLIR_CMD_TBL_NULL       Command table is null.
 *          CLIR_NOT_ENABLED        CLI is not enabled.
 ----------------------------------------------------------------------------*/
INT32 cli_parser_detach_cmd_tbl(CLI_EXEC_T* pt_tbl,
                                CLI_CAT_T   e_category,
                                UINT64      ui8_group_mask)
{
    UINT32          ui4_idx;
    UINT32*         pui4_tbl_cnt;
	CLI_EXEC_T*     pt_cmd_tbl = NULL;
    CLI_EXEC_T**    ppt_cmd_tbl_array;
    CLI_GRP_T*      pt_mod_grp_info;
    BOOL            b_replace_cmd_tbl = FALSE;
    
    CLI_DBG("----------detach pt_tbl %s----------\n", pt_tbl->ps_cmd_str);

    /* Check arguments */
	if ((pt_tbl == NULL) ||
	    (pt_tbl->ps_cmd_str == NULL) ||
	    (e_category >= CLI_CAT_MAX))
	{
		return CLIR_INV_ARG;
	}

    /* Attach the command table to requested category */
    switch (e_category)
    {
        case CLI_CAT_ROOT:
            pui4_tbl_cnt = &aui4_cli_tbl_cnt[CLI_CAT_ROOT];
            ppt_cmd_tbl_array = &(apt_cli_array[CLI_CAT_ROOT][0]);
            pt_mod_grp_info = &(at_cli_mod_grp_info[CLI_CAT_ROOT][0]);
            break;

        case CLI_CAT_APP:
            pui4_tbl_cnt = &aui4_cli_tbl_cnt[CLI_CAT_APP];
            pt_cmd_tbl = at_app_cmd_tbl;
            ppt_cmd_tbl_array = &(apt_cli_array[CLI_CAT_APP][0]);
            pt_mod_grp_info = &(at_cli_mod_grp_info[CLI_CAT_APP][0]);
            break;

        case CLI_CAT_MW:
            pui4_tbl_cnt = &aui4_cli_tbl_cnt[CLI_CAT_MW];
            pt_cmd_tbl = at_mw_cmd_tbl;
            ppt_cmd_tbl_array = &(apt_cli_array[CLI_CAT_MW][0]);
            pt_mod_grp_info = &(at_cli_mod_grp_info[CLI_CAT_MW][0]);
            break;

        default:
            return CLIR_INV_ARG;
    }

    /* Check if buffer size is ok */
	if (0 == *pui4_tbl_cnt)
	{
		return CLIR_CMD_TBL_NULL;
	}

	/* If the command table is duplicated, replace existed commad table with new one */
	for (ui4_idx = 0; ui4_idx < *pui4_tbl_cnt; ui4_idx++)
	{
	    BOOL fgCond1 = FALSE, fgCond2 = FALSE;

	    if (NULL == pt_tbl->ps_cmd_str && NULL == ppt_cmd_tbl_array[ui4_idx]->ps_cmd_str)
	        fgCond1 = TRUE;
	    else if (NULL == pt_tbl->ps_cmd_str || NULL == ppt_cmd_tbl_array[ui4_idx]->ps_cmd_str)
	        fgCond1 = FALSE;
	    else if (strcmp(pt_tbl->ps_cmd_str, ppt_cmd_tbl_array[ui4_idx]->ps_cmd_str) == 0)
	        fgCond1 = TRUE;

	    if (NULL == pt_tbl->ps_cmd_abbr_str && NULL == ppt_cmd_tbl_array[ui4_idx]->ps_cmd_abbr_str)
	        fgCond2 = TRUE;
	    else if (NULL == pt_tbl->ps_cmd_abbr_str || NULL == ppt_cmd_tbl_array[ui4_idx]->ps_cmd_abbr_str)
	        fgCond2 = FALSE;
	    else if ((NULL != pt_tbl->ps_cmd_abbr_str)&&(NULL != ppt_cmd_tbl_array[ui4_idx]->ps_cmd_abbr_str))
	    {
	    	//need check the whole two str not null, so can do strcmp opt.
	    	if (strcmp(pt_tbl->ps_cmd_abbr_str, ppt_cmd_tbl_array[ui4_idx]->ps_cmd_abbr_str) == 0)
				fgCond2 = TRUE;
		}
	        

	    if (TRUE == fgCond1 && TRUE == fgCond2)
        {
        	if (pt_mod_grp_info[ui4_idx].pt_cmd_tbl == pt_tbl
				&&
				pt_mod_grp_info[ui4_idx].aui8_grp_msk == ui8_group_mask)
            {
            	b_replace_cmd_tbl = TRUE;
            	break;
        	}
        }
    }

    if (b_replace_cmd_tbl)
    {
    	for (; ui4_idx < *pui4_tbl_cnt; ui4_idx++)
    	{
    		if (CLI_MAX_CMD_TBL_NUM - 1 > ui4_idx)
    		{
    			ppt_cmd_tbl_array[ui4_idx] = ppt_cmd_tbl_array[ui4_idx + 1];
				pt_mod_grp_info[ui4_idx] = pt_mod_grp_info[ui4_idx + 1];
    		}
			else if (CLI_MAX_CMD_TBL_NUM - 1 == ui4_idx)
			{
				ppt_cmd_tbl_array[ui4_idx] = NULL;
				pt_mod_grp_info[ui4_idx].pt_cmd_tbl = NULL;
				pt_mod_grp_info[ui4_idx].aui8_grp_msk = CLI_GRP_NONE;
			}
    	}
    	(*pui4_tbl_cnt)--;

    }
    else
    {
        return CLIR_CMD_NOT_FOUND;
    }

	return CLIR_OK;

}

/*-----------------------------------------------------------------------------
 * Name: cli_parser_clear_cmd_tbl
 *
 * Description: This API removes all command tables from CLI root then
 *              attaches default ones.
 *
 * Inputs:  -
 *
 * Outputs: -
 *
 * Returns: CLIR_OK                 Routine successful.
 *          CLIR_INV_ARG            One or more invalid arguments.
 *          CLIR_CMD_TBL_FULL       Command table is full.
 *          CLIR_GROUP_TBL_FULL     Module-to-group table is full.
 *          CLIR_NOT_ENABLED        CLI is not enabled.
 ----------------------------------------------------------------------------*/
INT32 cli_parser_clear_cmd_tbl(VOID)
{
    INT32       i4_return;
	UINT32      ui4_i;
	UINT32      ui4_j;

    /* Remove all command tables attached to CLI root */
    for (ui4_i = CLI_CAT_ROOT; ui4_i < CLI_CAT_MAX; ui4_i++)
    {
    	for (ui4_j = 0; ui4_j < CLI_MAX_CMD_TBL_NUM; ui4_j++)
    	{
            apt_cli_array[ui4_i][ui4_j] = NULL;
            aui4_cli_tbl_cnt[ui4_i] = 0;
    	    at_cli_mod_grp_info[ui4_i][ui4_j].pt_cmd_tbl = NULL;
    	    at_cli_mod_grp_info[ui4_i][ui4_j].aui8_grp_msk = CLI_GRP_NONE;
    	}
    }

    /* Attach default command tables to CLI root */
    at_app_cmd_tbl[0].pt_next_level = NULL;
    at_mw_cmd_tbl[0].pt_next_level = NULL;

	/* Mandatory command table */
	i4_return = cli_parser_attach_cmd_tbl(at_mandatory_cmd_tbl, CLI_CAT_ROOT, CLI_GRP_NONE);
	if (i4_return != CLIR_OK)
	{
	    dbg_print("Err:initial man cli cmd table fail\n");
	    return i4_return;
	}

	/* Application command table */
	i4_return = cli_parser_attach_cmd_tbl(at_app_cmd_tbl, CLI_CAT_ROOT, CLI_GRP_NONE);
	if (i4_return != CLIR_OK)
	{
	    dbg_print("Err:initial app cli cmd table fail\n");
	    return i4_return;
	}

	/* Middleware command table */
	i4_return = cli_parser_attach_cmd_tbl(at_mw_cmd_tbl, CLI_CAT_ROOT, CLI_GRP_NONE);
	if (i4_return != CLIR_OK)
	{
	    dbg_print("Err:initial mw cli cmd table fail\n");
	    return i4_return;
	}

    i4_return = cli_parser_attach_cmd_tbl(at_uart_root_cmd_table, CLI_CAT_ROOT, CLI_GRP_NONE);
	if (i4_return != CLIR_OK)
	{
	    dbg_print("Err:initial uart cli cmd table fail\n");
	    return i4_return;
	}
    
    /* Init internal variables */
    _cli_parser_init();

	/* Set up prompt string */
    _cli_set_cli_prompt();

  	return CLIR_OK;
}

