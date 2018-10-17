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
 * $RCSfile: _cli.h,v $ -cli_h,v $
 * $Revision: #1 $
 * $Date: 2016/07/22 $
 * $Author: bdbm01 $
 * $CCRevision: /main/DTV_X_HQ_int/DTV_X_ATSC/7 $
 * $SWAuthor: Alec Lu $
 * $MD5HEX: ca6d601bce72fe5d2621e6dd83df1947 $
 *
 * Description:
 *         This is CLI internal include file
 *---------------------------------------------------------------------------*/

#ifndef __CLI_H_
#define __CLI_H_

/*-----------------------------------------------------------------------------
                    include files
 ----------------------------------------------------------------------------*/
#include <stdio.h>
#include <pthread.h>

#include "u_common.h"
#include "u_dbg.h"
#include "u_cli.h"

#include "u_app_priority.h"

/*-----------------------------------------------------------------------------
                    macros, defines, typedefs, enums
 ----------------------------------------------------------------------------*/

/* CLI thread definitions. */
#define CLI_THREAD_NAME                 "cli_thread"
#define CLI_THREAD_DEFAULT_PRIORITY     ((UINT8)   PRIORITY(PRIORITY_CLASS_HIGH, PRIORITY_LAYER_UI, 5))
#define CLI_THREAD_DEFAULT_STACK_SIZE   ((SIZE_T) 4096*4*2)

#define MEMORY_ALIGNMENT        8


/* ASCII key definiton */
#define ASCII_NULL                      ((INT8) 0x00)
#define ASCII_KEY_CTRL_B                ((INT8) 0x02)
#define ASCII_KEY_CTRL_C                ((INT8) 0x03)
#define ASCII_KEY_CTRL_D                ((INT8) 0x04)
#define ASCII_KEY_CTRL_L                ((INT8) 0x0c)
#define ASCII_KEY_BS                    ((INT8) 0x08)
#define ASCII_KEY_NL                    ((INT8) 0x0a)
#define ASCII_KEY_ENTER                 ((INT8) 0x0d)
#define ASCII_KEY_CTRL_W                ((INT8) 0x17)
#define ASCII_KEY_ESC                   ((INT8) 0x1b)
#define ASCII_KEY_SPACE                 ((INT8) 0x20)
#define ASCII_KEY_DBL_QUOTE             ((INT8) 0x22)
#define ASCII_KEY_PUNCH                 ((INT8) 0x23)
#define ASCII_KEY_SGL_QUOTE             ((INT8) 0x27)
#define ASCII_KEY_DOT                   ((INT8) 0x2e)
#define ASCII_KEY_DOLLAR                ((INT8) 0x24)
#define ASCII_KEY_UP                    ((INT8) 0x41)
#define ASCII_KEY_DOWN                  ((INT8) 0x42)
#define ASCII_KEY_RIGHT                 ((INT8) 0x43)
#define ASCII_KEY_LEFT                  ((INT8) 0x44)
#define ASCII_KEY_ARROW                 ((INT8) 0x5b)
#define ASCII_KEY_ROOT                  ((INT8) 0x2f)
#define ASCII_KEY_PRINTABLE_MIN         ((INT8) 0x20)
#define ASCII_KEY_PRINTABLE_MAX         ((INT8) 0x7e)

/* CLI input related */
#define CLI_PROMPT_STR					"Command"	/* CLI prompt string */

/* CLI parser related */
#define CLI_ALL_STR                     "all"
#define CLI_NONE_STR                    "none"

#define CLI_CAT_APP_STR                 "app"
#define CLI_CAT_MW_STR                  "mw"

#define CLI_AR_SUPERVISOR_STR           "supervisor"
#define CLI_AR_ADMIN_STR                "admin"
#define CLI_AR_GUEST_STR                "guest"

#define CLI_MAX_ARG_NUM 				64      /* Maximum number of command arguments supported */
#define CLI_MAX_ARG_LEN				    256     /* Maximum size of an argument */
#define CLI_MAX_CMD_TBL_NUM		        128		/* Maximum number of attached command tables */
#define CLI_MAX_CMD_TBL_LEVEL			8		/* Maximum level of command table */
#define CLI_MANDA_CMD_TBL_IDX			0		/* Mandatory command table index */


/* Macro definition */
//#define CLI_DBG(stat...)    do{dbg_print("[CLI] ");dbg_print(stat);}while(0)
#define CLI_DBG(stat...)

#define CLI_ASSERT(_expr)    DBG_ASSERT(_expr, 0)


/* CLI operation mode */
typedef enum
{
    CLI_ENABLED = 0,
	CLI_ENABLING,
	CLI_DISABLED
}CLI_OP_MODE;

/*-----------------------------------------------------------------------------
                    functions declarations
 ----------------------------------------------------------------------------*/
/* CLI input related */
extern INT32 cli_init(VOID);
extern BOOL cli_is_inited(VOID);
extern CHAR* cli_get_prompt_str_buf(VOID);

/* CLI parser related */
extern INT32 cli_parser_clear_cmd_tbl(VOID);

extern INT32 cli_parser_attach_cmd_tbl(CLI_EXEC_T* pt_tbl,
                                       CLI_CAT_T   e_category,
                                       UINT64      ui8_group_mask);

extern INT32 cli_parser_detach_cmd_tbl(CLI_EXEC_T* pt_tbl,
                                       CLI_CAT_T   e_category,
                                       UINT64      ui8_group_mask);
extern INT32 cli_parser(const CHAR* ps_cmd);

//only for test
extern VOID cli_init_phase1(VOID);
extern VOID cli_init_phase2(VOID);

#endif /* __CLI_H_ */

