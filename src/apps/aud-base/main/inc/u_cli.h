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

 * $RCSfile: u_cli.h,v $ u_cli_h,v $
 * $Revision: #1 $
 * $Date: 2016/07/22 $
 * $Author: bdbm01 $
 * $CCRevision: /main/DTV_X_HQ_int/DTV_X_ATSC/11 $
 * $SWAuthor: Alec Lu $
 * $MD5HEX: d3f4bd3088d0839e70c155f2e911dd7a $
 *
 * Description:
 *         This header file contains CLI related definitions, which are
 *         known to applications and middleware.
 *---------------------------------------------------------------------------*/

#ifndef _U_CLI_H_
#define _U_CLI_H_

/*-----------------------------------------------------------------------------
                    include files
 ----------------------------------------------------------------------------*/
#include "u_common.h"
#include "u_dbg.h"

/*-----------------------------------------------------------------------------
                    macros, defines, typedefs, enums
 ----------------------------------------------------------------------------*/

#define CLI_CMD_BUF_SIZE				1028		/* Maximum size of command buffer */
#define CLI_CMD_BUF_ROW_NUM			    16          /* Number of command buffers supported */


/* CLI API return values */
#define CLIR_INV_CMD_USAGE          ((INT32)    1)
#define CLIR_OK                     ((INT32)    0)
#define CLIR_NOT_INIT               ((INT32)   -1)
#define CLIR_ALREADY_INIT           ((INT32)   -2)
#define CLIR_NOT_ENABLED            ((INT32)   -3)
#define CLIR_INV_ARG                ((INT32)   -4)
#define CLIR_INV_CMD_TBL            ((INT32)   -5)
#define CLIR_CMD_TOO_LONG           ((INT32)   -6)
#define CLIR_ALIAS_TOO_LONG         ((INT32)   -7)
#define CLIR_CMD_TBL_FULL           ((INT32)   -8)
#define CLIR_ALIAS_TBL_FULL         ((INT32)   -9)
#define CLIR_CMD_NOT_FOUND          ((INT32)  -10)
#define CLIR_DIR_NOT_FOUND          ((INT32)  -11)
#define CLIR_CMD_EXEC_ERROR         ((INT32)  -12)
#define CLIR_UNKNOWN_CMD            ((INT32)  -13)
#define CLIR_CMD_TBL_NULL           ((INT32)  -14)


/* ASCII key definiton */
#define CLI_ASCII_KEY_CTRL_B        ((INT8) 0x02)

/* Definition of get & set debug level command and help strings */
#define CLI_GET_DBG_LVL_STR         "gdl"
#define CLI_SET_DBG_LVL_STR         "sdl"
#define CLI_GET_DBG_LVL_HELP_STR    "Get debug level (e=error, a=api, i=info, n=none)"
#define CLI_SET_DBG_LVL_HELP_STR    "Set debug level (e=error, a=api, i=info, n=none)"

/* Definition of group debug level control */
#define CLI_GRP_NONE                ((UINT64)   0x00000000)
#define CLI_GRP_PIPE                ((UINT64)   0x00000001)
#define CLI_GRP_GUI                 ((UINT64)   0x00000002)
#define CLI_GRP_EPG                 ((UINT64)   0x00000004)
#define CLI_GRP_DRV                 ((UINT64)   0x00000008)
#define CLI_GRP_MAX                 ((UINT64)   0x00000010)

/* Definition of get & set time measurement level command and help strings */
#define CLI_GET_TMS_LVL_STR         "gtl"
#define CLI_SET_TMS_LVL_STR         "stl"
#define CLI_GET_TMS_LVL_HELP_STR    "Get TMS level (r=real-time, o=off-line, n=none)"
#define CLI_SET_TMS_LVL_HELP_STR    "Set TMS level (r=real-time, o=off-line, n=none)"

/* Definition of command table terminator */
#define END_OF_CLI_CMD_TBL          {NULL, NULL, NULL, NULL, NULL, CLI_HIDDEN}


/* CLI command access right */
/* New entries must be added to the end of the enumeration before CLI_HIDDEN */
typedef enum
{
	CLI_GUEST,          /* Commands used by end user only */
	CLI_HIDDEN          /* Hide commands from help list */
}   CLI_ACCESS_RIGHT_T;


/* CLI category definition */
typedef enum
{
    CLI_CAT_ROOT = 0,
	CLI_CAT_APP,
	CLI_CAT_MW,
	CLI_CAT_MAX
}   CLI_CAT_T;

/* CLI password calculation definition */
typedef enum
{
    CLI_PASSWD_ODD = 0,
	CLI_PASSWD_EVEN,
	CLI_PASSWD_TWO_DIGITS_SUM_DEC,
	CLI_PASSWD_TWO_DIGITS_SUM_HEX,
	CLI_PASSWD_4EVEN_3ODD,
	CLI_PASSWD_4ODD_3EVEN,
	CLI_PASSWD_REVERSE_7EVEN,
	CLI_PASSWD_MAX_TYPE_NUM
}   CLI_PASSWD_T;


/* CLI execution function */
typedef INT32 (*u_cli_exec_fct)(INT32        i4_argc,
                                const CHAR** pps_argv);

/* CLI command table structure */
typedef struct _CLI_EXEC_T
{
	CHAR*				    ps_cmd_str;			/* Command string */
	CHAR*				    ps_cmd_abbr_str;	/* Command abbreviation string */
	u_cli_exec_fct          pf_exec_fct;        /* Execution function */
	struct _CLI_EXEC_T*     pt_next_level;      /* Next level command table */
	CHAR*				    ps_cmd_help_str;	/* Command help string */
	CLI_ACCESS_RIGHT_T	    e_access_right;		/* Command access right */
}   CLI_EXEC_T;

/* Definition of group info structure */
typedef struct _CLI_GRP_T
{
	CLI_EXEC_T*         pt_cmd_tbl;
    UINT64              aui8_grp_msk;
}   CLI_GRP_T;


extern INT32 u_cli_init(VOID);
extern INT32 u_cli_attach_cmd_tbl(CLI_EXEC_T* pt_tbl, 
                                  CLI_CAT_T   e_category,
                                  UINT64      ui8_group_mask);  
extern INT32 u_cli_detach_cmd_tbl(CLI_EXEC_T* pt_tbl, 
                                  CLI_CAT_T   e_category,
                                  UINT64      ui8_group_mask);  
extern INT32 u_cli_parser(const CHAR* ps_cmd);

extern INT32 u_cli_parser_arg(const CHAR* ps_cmd, ...);

extern INT32 u_cli_parse_dbg_level(INT32        i4_argc, 
                                   const CHAR** pps_argv,
                                   UINT16*      pui2_dbg_level);
extern INT32 u_cli_show_dbg_level(UINT16 ui2_dbg_level);

#endif /* _U_CLI_H_ */

