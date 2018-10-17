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
                    include files
-----------------------------------------------------------------------------*/
#include "u_common.h"
#include "u_dbg.h"

//xinmei
//#include "u_scc.h"
//#include "u_dlna_dmp.h"

#include "u_dbg.h"
#include "u_os.h"
//#include "c_cfg.h"
#include "u_aee.h"
#include "u_appman.h"

//xinmei
//#include "c_scc.h"
//#include "c_dlna_dmp_api.h"
//#include "c_mtklib_cfg.h"

//#include "u_app_util.h"  //xinmei
#include "acfg.h"
#include "u_acfg.h"
#include "u_bluetooth_audio.h"


//extern INT32 _acfg_store_request(VOID);

/*-----------------------------------------------------------------------------
                    private variable definition
-----------------------------------------------------------------------------*/
#if 1// (CONFIG_APP_SUPPORT_BLUETOOTH_UI || CONFIG_APP_SUPPORT_BLUETOOTH_AUDIO)
static BOOL b_bt_enable_def = (UINT8)BLUETOOTH_ENABLE;

/*-----------------------------------------------------------------------------
                    private function definition
-----------------------------------------------------------------------------*/
BOOL _acfg_bluetooth_validate(UINT16 ui2_id, VOID* pt_value, SIZE_T z_size)
{
    UINT8 ui_temp = *(UINT8 *)pt_value;

    switch (ui_temp)
    {
    case BLUETOOTH_ENABLE:
    case BLUETOOTH_DISABLE:
        break;
    default:
        DBG_ERROR(("<ACFG> ERROR, _acfg_bluetooth_validate, invalid value : %d\n", ui_temp));
        return FALSE;
    }

    return TRUE;
}

//xinmei  change the position of the Function
INT32 u_cfg_get_bluetooth_enable(UINT8 *pui1_value)
{
    INT32    i4_ret;
    SIZE_T   z_size;
    BOOL   pb_tz_data;

    i4_ret = acfg_get(IDX_BLUETOOTH_ENABLE, &pb_tz_data , &z_size); //IDX_BLUETOOTH_ENABLE
    if (i4_ret != APP_CFGR_OK)
    {
        DBG_ACFG_ERR("u_cfg_get_bluetooth_enable fail, error code: %d\n", i4_ret);
        return i4_ret;
    }
    *pui1_value = pb_tz_data;

    DBG_ACFG_INFO("u_cfg_get_bluetooth_enable, value : %d\n", *pui1_value);

    return APP_CFGR_OK;
}


VOID _acfg_bluetooth_update(UINT16 ui2_id)
{
    UINT8   ui1_switch = NETCONNECTION_OFF;
    u_cfg_get_bluetooth_enable(&ui1_switch);

    switch (ui1_switch)
    {
    case BLUETOOTH_ENABLE:
        {
            break;
        }
    case BLUETOOTH_DISABLE:
        {
            break;
        }
    default:
        {
            DBG_ACFG_ERR("ERROR, _acfg_bluetooth_update, fail : %d\n", ui1_switch);
            break;
        }
    }
#if 1  //xinmei
#if 1 //CONFIG_SUPPORT_NEW_ACFG
    _acfg_store_request();
#endif
#endif
}

/*-----------------------------------------------------------------------------
                    export function definition
-----------------------------------------------------------------------------*/
/*! \fn static INT32 acfg_grp_network_init(VOID)
    \brief this function is to initialize the records
    \retval INT32 APP_CFGR_OK if successful
*/

INT32 acfg_grp_bluetooth_init(VOID)
{
    SETUP_FIELD(TRUE,
                IDX_BLUETOOTH_ENABLE,                  /*s_name */  //xinmei
                CFG_GRPID_BLUETOOTH,                 /* gid of setting ID */  //xinmei
                CFG_RECID_BLUETOOTH_ENABLE,            /* rid of setting ID */
                CFG_8BIT_T,                       /* type of element */
                1,                                /* number of elements */
                &b_bt_enable_def,             /* default value */
                1,                                /* size of default value */
                _acfg_bluetooth_validate,        /* validate function, if any */
                _acfg_bluetooth_update);          /* update hook fucntion, if any */
 //               ACFG_LOCATION_FLASH,              /* store location */         //xinmei
 //               ACFG_ACCESS_PUBLIC|ACFG_ACCESS_FACTORY);     /* Access Mask */  //xinmei

    return APP_CFGR_OK;
}

INT32 u_cfg_set_bluetooth_enable(UINT8 ui1_value)
{
    INT32 i4_ret;

    DBG_INFO(("<ACFG> u_cfg_set_bluetooth_enable, value : %d\n", ui1_value));

    i4_ret = acfg_set(IDX_BLUETOOTH_ENABLE, &ui1_value , 1);  //xinmei IDX_BLUETOOTH_ENABLE
    if (i4_ret != APP_CFGR_OK)
    {
        DBG_ACFG_ERR("u_cfg_set_bluetooth_enable fail, error code: %d\n", i4_ret);
        return i4_ret;
    }

    return APP_CFGR_OK;
}

//xinmei  move up
#if 0
INT32 u_cfg_get_bluetooth_enable(UINT8 *pui1_value)
{
    INT32    i4_ret;
    SIZE_T   z_size;
    BOOL   pb_tz_data;

    i4_ret = acfg_get(IDX_BLUETOOTH_ENABLE, &pb_tz_data , &z_size); //IDX_BLUETOOTH_ENABLE
    if (i4_ret != APP_CFGR_OK)
    {
        DBG_ERROR(("<ACFG> u_cfg_get_bluetooth_enable fail, error code: %d\n", i4_ret));
        return i4_ret;
    }

    *pui1_value = pb_tz_data;

    DBG_INFO(("<ACFG> u_cfg_get_bluetooth_enable, value : %d\n", *pui1_value));

    return APP_CFGR_OK;
}

#endif

INT32 u_cfg_set_bluetooth_enable_default(VOID)
{
    INT32 i4_ret;

    DBG_INFO(("<ACFG> u_cfg_set_bluetooth_enable_default\n"));

    i4_ret = acfg_set_default(IDX_BLUETOOTH_ENABLE);//IDX_BLUETOOTH_ENABLE);  //xinmei
    if (i4_ret != APP_CFGR_OK)
    {
        DBG_ACFG_ERR("u_cfg_set_bluetooth_enable_default fail, error code: %d\n", i4_ret);
        return i4_ret;
    }

    return APP_CFGR_OK;
}

#endif

