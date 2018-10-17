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

#include "user_interface_cli.h"

#include "u_gpio.h"


#ifdef CLI_SUPPORT
#include "u_cli.h"
/*-----------------------------------------------------------------------------
 * structure, constants, macro definitions
 *---------------------------------------------------------------------------*/
/*-----------------------------------------------------------------------------
 * private methods declarations
 *---------------------------------------------------------------------------*/
static INT32  _user_interface_cli_set_dbg_level (INT32 i4_argc, const CHAR** pps_argv);
static INT32  _user_interface_cli_get_dbg_level (INT32 i4_argc, const CHAR** pps_argv);
static INT32  _user_interface_cli_key_event(INT32 i4_argc, const CHAR** pps_argv);
static INT32  _user_interface_cli_gpio_set(INT32 i4_argc, const CHAR** pps_argv);


/*-----------------------------------------------------------------------------
 * variable declarations
 *---------------------------------------------------------------------------*/


/* main command table */
static CLI_EXEC_T at_user_interface_cmd_tbl[] =
{
    {
        CLI_GET_DBG_LVL_STR,
        NULL,
        _user_interface_cli_get_dbg_level,
        NULL,
        CLI_GET_DBG_LVL_HELP_STR,
        CLI_GUEST
    },
    {
        CLI_SET_DBG_LVL_STR,
        NULL,
        _user_interface_cli_set_dbg_level,
        NULL,
        CLI_SET_DBG_LVL_HELP_STR,
        CLI_GUEST
    },
    {
        "key event trigger",
        "key",
        _user_interface_cli_key_event,
        NULL,
        "simulate user operation",
        CLI_GUEST
    },
    {
        "gpio_set",
        "gpio_set",
        _user_interface_cli_gpio_set,
        NULL,
        "gpio setting in user space",
        CLI_GUEST
    },    
  
    END_OF_CLI_CMD_TBL
};
/* SVL Builder root command table */
static CLI_EXEC_T at_user_interface_root_cmd_tbl[] =
{
    {   
        "user_interface",
        "uc",
        NULL,
        at_user_interface_cmd_tbl,
        "user_interface commands",
        CLI_GUEST
    },
    END_OF_CLI_CMD_TBL
};
/*-----------------------------------------------------------------------------
 * export methods implementations
 *---------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------
 * Name:    user_interface_cli_attach_cmd_tbl
 * Description:
 * Input arguments:  -
 * Output arguments: -
 * Returns:
 *----------------------------------------------------------------------------*/
INT32 user_interface_cli_attach_cmd_tbl(VOID)
{
    return (u_cli_attach_cmd_tbl(at_user_interface_root_cmd_tbl, CLI_CAT_APP, CLI_GRP_GUI));
}


/*-----------------------------------------------------------------------------
 * private methods implementations
 *---------------------------------------------------------------------------*/
/*-----------------------------------------------------------------------------
 * Name: _user_interface_cli_get_dbg_level
 *
 * Description: This API gets the current debug level.
 *
 * Inputs:  i4_argc         Contains the argument count.
 *          pps_argv        Contains the arguments.
 *
 * Outputs: -
 *
 * Returns: CLIR_OK         Routine successful.
 ----------------------------------------------------------------------------*/
static INT32 _user_interface_cli_get_dbg_level (INT32 i4_argc, const CHAR** pps_argv)
{
    INT32  i4_ret;

    i4_ret = u_cli_show_dbg_level(user_interface_get_dbg_level());

    return i4_ret;
}
/*-----------------------------------------------------------------------------
 * Name: _user_interface_cli_set_dbg_level
 *
 * Description: This API sets the debug level.
 *
 * Inputs:  i4_argc         Contains the argument count.
 *          pps_argv        Contains the arguments.
 *
 * Outputs: -
 *
 * Returns: CLIR_OK         Routine successful.
 ----------------------------------------------------------------------------*/
static INT32 _user_interface_cli_set_dbg_level (INT32 i4_argc, const CHAR** pps_argv)
{
    UINT16 ui2_dbg_level;
    INT32  i4_ret;

    i4_ret = u_cli_parse_dbg_level(i4_argc, pps_argv, &ui2_dbg_level);

    if (i4_ret == CLIR_OK){
        user_interface_set_dbg_level(ui2_dbg_level);
    }

    return i4_ret;
}
/*-----------------------------------------------------------------------------
 * Name: _user_interface_cli_echo
 *
 * Description: This API is only for test purpose.
 *
 * Inputs:  i4_argc         Contains the argument count.
 *          pps_argv        Contains the arguments.
 *
 * Outputs: -
 *
 * Returns: CLIR_OK         Routine successful.
 ----------------------------------------------------------------------------*/
static INT32 _user_interface_cli_key_event(INT32 i4_argc, const CHAR** pps_argv)
{

    return CLIR_OK;
}

/*-----------------------------------------------------------------------------
 * Name: _atoi
 *
 * Description: This API is Convert the initial portion of the string pointed to int.
 *
 * Inputs:  param ps pointer to CHAR string.
 *
 * Outputs: -
 *
 * Returns: return the converted value.return (-1) if ps contain not digit.
                  0                          Routine successful.
                  Any other value     Routine failed.
 ----------------------------------------------------------------------------*/

static INT32  _atoi (const CHAR* ps)
{
    INT32  i4_total;

    i4_total = 0;

    while (*ps != 0)
    {
        if ((*ps >= '0') && 
            (*ps <= '9'))
        {
            /* accumulate digit */
            i4_total = 10 * i4_total + (*ps - '0');
            ps++; /* get next char */
        }
        else /* not a digit */
        {
            return (-1);
        }
    }

    return i4_total;
}


/*-----------------------------------------------------------------------------
 * Name: _user_interface_cli_gpio_set
 *
 * Description: This API is only for test purpose.
 *
 * Inputs:  i4_argc         Contains the argument count.
 *          pps_argv        Contains the arguments.
 *
 * Outputs: -
 *
 * Returns: CLIR_OK         Routine successful.
 ----------------------------------------------------------------------------*/
static INT32 _user_interface_cli_gpio_set(INT32 i4_argc, const CHAR** pps_argv)
{
    int ret = 0;
    UINT32 i4_gpio_number = 0;
    UINT32 i4_OutIn = 0;
    UINT32 i4_high_low = 0;
  
    if (4 == i4_argc)
    {  
        i4_gpio_number = (UINT32)_atoi(pps_argv[1]); //gpio number
        i4_OutIn = (UINT32)_atoi(pps_argv[2]);     // 1 is output, 0 is input
        i4_high_low = (UINT32)_atoi(pps_argv[3]);   // 1 is high level, 0 is low level
     
        gpio_config(i4_gpio_number, i4_OutIn, i4_high_low);
       
        DBG_ERROR(("_user_interface_cli_gpio_set success, i4_gpio_number=%d, i4_OutIn =%d, i4_high_low=%d\n", i4_gpio_number,i4_OutIn,i4_high_low));    
         
    }
    else
    {
        DBG_ERROR(("_user_interface_cli_gpio_set failed: illegal parameter\n"));
    }
    return CLIR_OK;
}

#endif
