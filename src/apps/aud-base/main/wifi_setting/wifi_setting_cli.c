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
#include "u_os.h"
#include "stdio.h"
#include "wifi_setting.h"
#include "stdlib.h"
#include "u_acfg.h"

/*-----------------------------------------------------------------------------
 * structure, constants, macro definitions
 *---------------------------------------------------------------------------*/
/*-----------------------------------------------------------------------------
 * private methods declarations
 *---------------------------------------------------------------------------*/
extern CHAR   wifi_ConnectSsid[NET_802_11_MAX_LEN_SSID+1];
extern CHAR   wifi_ConnectBssid[NET_802_11_MAX_LEN_BSSID+1];
extern CHAR   wifi_ConnectPwd[WLAN_KEY_MAX_LEN+1];
extern INT32  wifi_AuthMode;
static UINT16 ui2_g_wifi_dbg_level = DBG_LEVEL_ALL;

CHAR primary_channel;
CHAR secondary_channel;
CHAR bssid[7] = {0};

UINT16 wifi_get_dbg_level(VOID)
{
    return ui2_g_wifi_dbg_level | DBG_LAYER_APP;
}

VOID wifi_set_dbg_level(UINT16 ui2_db_level)
{
    ui2_g_wifi_dbg_level = ui2_db_level;
}


static INT32 _wifi_cli_set_dbg_level (INT32 i4_argc, const CHAR** pps_argv)
{
    INT32  i4_ret;
#ifdef CLI_SUPPORT
    UINT16 ui2_dbg_level;

    i4_ret = u_cli_parse_dbg_level(i4_argc, pps_argv, &ui2_dbg_level);

    if (i4_ret == CLIR_OK){
        wifi_set_dbg_level(ui2_dbg_level);
    }
#endif
    return i4_ret;
}

#ifdef CLI_SUPPORT

/*-----------------------------------------------------------------------------
 * variable declarations
 *---------------------------------------------------------------------------*/
/* command table *//*-----------------------------------------------------------------------------

 * Name: wifi_get_dbg_level
 *
 * Description: This API returns the current setting of debug level.
 *
 * Inputs: -
 *
 * Outputs: -
 *
 * Returns: The current setting of debug level.
 ----------------------------------------------------------------------------*/


/*-----------------------------------------------------------------------------
 * Name: wifi_set_dbg_level
 *
 * Description: This API set the debug level.
 *
 * Inputs: -
 *
 * Outputs: -
 *
 * Returns: The current setting of debug level.
 ----------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------
 * private methods implementations
 *---------------------------------------------------------------------------*/
/*-----------------------------------------------------------------------------
 * Name: _wifi_cli_get_dbg_level
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
static INT32 _wifi_cli_get_dbg_level (INT32 i4_argc, const CHAR** pps_argv)
{
    INT32  i4_ret;

    i4_ret = u_cli_show_dbg_level(wifi_get_dbg_level());

    return i4_ret;
}
/*-----------------------------------------------------------------------------
 * Name: _wifi_cli_set_dbg_level
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


static INT32 _wifi_cli_set_wifi_task (INT32 i4_argc, const CHAR** pps_argv)
{
    INT32  i4_ret;
	UINT32 ui4Value1 = 0;
	UINT32 ui4Value2 = 0;//NET_802_11_AUTH_MODE_MAX;
	if(i4_argc==1)
	{
	    printf("usage: task {START_SOFTAP|STOP_SOFTAP|SCAN_AP|SCAN_STOP|CONNECT_FAVORATE_AP|CONNECT_SETTING_AP|CONNECT_WPS_AP|SWITCH_TO_WIRED|SWITCH_TO_WIRELESS}\r\n");
		return -1;
	}
	ui4Value1 = (UINT8)(atoi(pps_argv[1]));

	if(pps_argv[2]!=NULL)
	{
		strncpy(wifi_ConnectSsid,pps_argv[2],NET_802_11_MAX_LEN_SSID+1);
		wifi_ConnectSsid[NET_802_11_MAX_LEN_SSID]='\0';
	}
	
	if(pps_argv[3]!=NULL)
	{
		strncpy(wifi_ConnectBssid,pps_argv[3],NET_802_11_MAX_LEN_BSSID+1);
		wifi_ConnectBssid[NET_802_11_MAX_LEN_BSSID]='\0';
	}
	
	if(pps_argv[4]!=NULL)
	{
		strncpy(wifi_ConnectPwd,pps_argv[4],WLAN_KEY_MAX_LEN+1);
		wifi_ConnectPwd[WLAN_KEY_MAX_LEN]='\0';
	}
	
	if(pps_argv[5]!=NULL)
	{
		ui4Value2 = atoi(pps_argv[5]);
		wifi_AuthMode = atoi(pps_argv[5]);
	}
	else
	{
		wifi_AuthMode = -1;
	}
	WIFI_MSG("argv[1]:[%s] argv[2]:[%s] argv[3]:[%s] argv[4]:[%s] argv[5]:[%s]\n", pps_argv[1], pps_argv[2], pps_argv[3], pps_argv[4], pps_argv[5]);
	WIFI_MSG("ssid:[%s] bssid:[%s] passwd:[%s] authmode:[%d]\n", wifi_ConnectSsid, wifi_ConnectBssid, wifi_ConnectPwd, wifi_AuthMode);
	i4_ret = wifi_task_realize(ui4Value1, pps_argv[2], pps_argv[3], pps_argv[4], ui4Value2);

	return i4_ret;
}


static INT32 _wifi_cli_elian_debug (INT32 i4_argc, const CHAR** pps_argv)
{
    INT32  i4_ret;
	UINT32 ui4Value1 = 0;
	UINT32 ui4Value2 = 0;

	if(i4_argc==1)
	{
		printf("usage: task {START_SOFTAP|STOP_SOFTAP|SCAN_AP|SCAN_STOP|CONNECT_FAVORATE_AP|CONNECT_SETTING_AP|CONNECT_WPS_AP|SWITCH_TO_WIRED|SWITCH_TO_WIRELESS}\r\n");
		return -1;
	}

	if(pps_argv[1]!=NULL)
	{
    	ui4Value1 = (UINT8)(atoi(pps_argv[1]));
	}

	if(pps_argv[2]!=NULL)
	{
		primary_channel = atoi(pps_argv[2]);
	}

	if(pps_argv[3]!=NULL)
	{
		secondary_channel = atoi(pps_argv[3]);
	}

	if(pps_argv[3]!=NULL)
	{
		strncpy(bssid,pps_argv[4],7-1);
	}

	i4_ret = wifi_elian_task(ui4Value1, primary_channel, secondary_channel,bssid);

    return i4_ret;
}

static CLI_EXEC_T at_wifi_cmd_tbl[] =
{
 	{
        CLI_GET_DBG_LVL_STR,
        NULL,
        _wifi_cli_get_dbg_level,
        NULL,
        CLI_GET_DBG_LVL_HELP_STR,
        CLI_GUEST
    },
	{
        CLI_SET_DBG_LVL_STR,
        NULL,
        _wifi_cli_set_dbg_level,
        NULL,
        CLI_SET_DBG_LVL_HELP_STR,
        CLI_GUEST
    },
    {
        "task",
        NULL,
        _wifi_cli_set_wifi_task,
        NULL,
        "wifi task",
        CLI_GUEST
    },
    {
        "elian",
        NULL,
        _wifi_cli_elian_debug,
        NULL,
        "elian",
        CLI_GUEST
    },
 	END_OF_CLI_CMD_TBL
};

/* SVL Builder root command table */
static CLI_EXEC_T at_wifi_root_cmd_tbl[] =
{
	{   "wifi",
	    "wifi",
	    NULL,
	    at_wifi_cmd_tbl,
	    "WIFI commands",
	    CLI_GUEST
	},
	END_OF_CLI_CMD_TBL
};

/*-----------------------------------------------------------------------------
 * export methods implementations
 *---------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------
 * Name:    wifi_cli_attach_cmd_tbl
 * Description:
 * Input arguments:  -
 * Output arguments: -
 * Returns:
 *----------------------------------------------------------------------------*/
INT32 wifi_cli_attach_cmd_tbl(VOID)
{
    return (u_cli_attach_cmd_tbl(at_wifi_root_cmd_tbl, CLI_CAT_APP, CLI_GRP_GUI));
}

#endif
