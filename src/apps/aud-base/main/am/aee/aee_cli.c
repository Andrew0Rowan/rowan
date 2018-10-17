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
 * $RCSfile: aee_cli.c,v $
 * $Revision: #1 $
 * $Date: 2016/03/07 $
 * $Author: bdbm01 $
 *
 * Description:
 *         This file contains the AEE CLI-related functions.
 *---------------------------------------------------------------------------*/


/*-----------------------------------------------------------------------------
                    include files
 ----------------------------------------------------------------------------*/

#include "aee.h"


#ifdef CLI_SUPPORT
#include "u_cli.h"
#include "u_dbg.h"
/*-----------------------------------------------------------------------------
 * Name: aee_cli_show_debug_level
 *
 * Description: This API shows the current AEE debug level.
 *
 * Inputs:  i4_argc   Contains the argument count.
 *          pps_argv  Contains the arguments.
 *
 * Outputs: -
 *
 * Returns: CLIR_OK    Routine successful.
 ----------------------------------------------------------------------------*/
static INT32 aee_cli_show_debug_level (INT32         i4_argc,
                                       const CHAR**  pps_argv)
{
    INT32  i4_res;

    i4_res = u_cli_show_dbg_level(aee_get_dbg_level());

    return i4_res;
}

/*-----------------------------------------------------------------------------
 * Name: aee_cli_set_debug_level
 *
 * Description: This API sets the AEE debug level.
 *
 * Inputs:  i4_argc   Contains the argument count.
 *          pps_argv  Contains the arguments.
 *
 * Outputs: -
 *
 * Returns: CLIR_OK    Routine successful.
 ----------------------------------------------------------------------------*/
static INT32 aee_cli_set_debug_level (INT32         i4_argc,
                                      const CHAR**  pps_argv)
{
    INT32   i4_res;
    UINT16  ui2_dbg_level;

    i4_res = u_cli_parse_dbg_level(i4_argc, pps_argv, & ui2_dbg_level);

    if (i4_res == CLIR_OK)
    {
        i4_res = aee_set_dbg_level(ui2_dbg_level);

        i4_res = (i4_res == AEER_OK) ? CLIR_OK : CLIR_CMD_EXEC_ERROR;
    }
    else
    {
        i4_res = CLIR_CMD_EXEC_ERROR;
    }

    return i4_res;
}

/*-----------------------------------------------------------------------------
 * Name: aee_cli_list_aees
 *
 * Description: This API lists the AEEs.
 *
 * Inputs:  i4_argc   Contains the argument count.
 *          pps_argv  Contains the arguments.
 *
 * Outputs: -
 *
 * Returns: CLIR_OK    Routine successful.
 ----------------------------------------------------------------------------*/
static INT32 aee_cli_list_aees (INT32         i4_argc,
                                const CHAR**  pps_argv)
{
    if (i4_argc == 1)
    {
        aee_list_all_aees_info();
    }
    else
    {
        INT32  i4_i;

        for (i4_i = 1; i4_i < i4_argc; i4_i++)
        {
            AEE_T*  pt_aee;

            pt_aee = aee_find_by_name(pps_argv[i4_i]);

            if (pt_aee != NULL)
            {
                aee_list_aee_info(pt_aee);
            }
            else
            {
                dbg_print(DBG_PREFIX"\"%s\" does not exist\n",
                           pps_argv[i4_i]);
            }
        }
    }

    return CLIR_OK;
}
#endif /* CLI_SUPPORT */

/*-----------------------------------------------------------------------------
 * Name: aee_cli_init
 *
 * Description: This API attaches the AEE CLI tables if CLI_SUPPORT is defined,
 *              otherwise it does nothing.
 *
 * Inputs:  -
 *
 * Outputs: -
 *
 * Returns: -
 ----------------------------------------------------------------------------*/
VOID aee_cli_init (VOID)
{
#ifdef CLI_SUPPORT
    static CLI_EXEC_T  at_aee_cli_tbl[] = {
        /* get debug level */
        {
            CLI_GET_DBG_LVL_STR,
            NULL,
            aee_cli_show_debug_level,
            NULL,
            CLI_GET_DBG_LVL_HELP_STR,
            CLI_GUEST
        },
        /* set debug level */
        {
            CLI_SET_DBG_LVL_STR,
            NULL,
            aee_cli_set_debug_level,
            NULL,
            CLI_SET_DBG_LVL_HELP_STR,
            CLI_GUEST
        },
        /* show all AEEs */
        {
            "list",
            "l",
            aee_cli_list_aees,
            NULL,
            "List all AEEs",
            CLI_GUEST
        },
        END_OF_CLI_CMD_TBL
    };

    static CLI_EXEC_T at_aee_root_cli_tbl[] = {
        {
            "aee",
            NULL,
            NULL,
            at_aee_cli_tbl,
            "AEE commands",
            CLI_GUEST
        },
        END_OF_CLI_CMD_TBL
    };

    static BOOL  b_attached = FALSE;

    CRIT_STATE_T  t_state;
    BOOL          b_attach;

    t_state = u_crit_start();

    b_attach = ! b_attached;

    b_attached = TRUE;

    u_crit_end(t_state);

    if (b_attach)
    {
        INT32  i4_res;

        i4_res = u_cli_attach_cmd_tbl(at_aee_root_cli_tbl,
                                      CLI_CAT_MW,
                                      CLI_GRP_NONE);

        if (i4_res != CLIR_OK)
        {
            t_state = u_crit_start();

            b_attached = FALSE;

            u_crit_end(t_state);
        }
    }
#endif /* CLI_SUPPORT */
}
