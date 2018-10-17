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
#include "u_dbg.h"
#include "u_os.h"
#include "u_aee.h"
#include "u_appman.h"
#include "acfg.h"
#include "u_acfg.h"
#include "c_net_wlan.h"

/*-----------------------------------------------------------------------------
                    private variable definition
-----------------------------------------------------------------------------*/
#if 1

static char ac_ssid_def[WLAN_MAC_MAX_LEN+1] = {0};
static char ac_key_def[WLAN_KEY_MAX_LEN+1] = {0};
static NET_802_11_ASSOCIATE_T wlan_ap_default  ={0};

/*-----------------------------------------------------------------------------
                    private function definition
-----------------------------------------------------------------------------*/
BOOL _acfg_ssid_validate(UINT16 ui2_id, VOID* pt_value, SIZE_T z_size)
{
    return TRUE;
}

VOID _acfg_ssid_update(UINT16 ui2_id)
{

}
BOOL _acfg_key_validate(UINT16 ui2_id, VOID* pt_value, SIZE_T z_size)
{
    return TRUE;
}

VOID _acfg_key_update(UINT16 ui2_id)
{

}
BOOL _acfg_wlan_ap_validate(UINT16 ui2_id, VOID* pt_value, SIZE_T z_size)
{
    return TRUE;
}

VOID _acfg_wlan_ap_update(UINT16 ui2_id)
{

}

/*-----------------------------------------------------------------------------
                    export function definition
-----------------------------------------------------------------------------*/
/*! \fn static INT32 acfg_grp_network_init(VOID)
    \brief this function is to initialize the records
    \retval INT32 APP_CFGR_OK if successful
*/

INT32 acfg_grp_network_init(VOID)
{
    SETUP_FIELD(TRUE,
                IDX_WLAN_MAC,                   /* s_name */
                CFG_GRPID_NETWORK,                /* gid of setting ID */
                CFG_RECID_NETWORK_SSID,          /* rid of setting ID */
                CFG_8BIT_T,                       /* type of element */
                WLAN_MAC_MAX_LEN+1,                 /* number of elements */
                ac_ssid_def,                    /* default value */
                WLAN_MAC_MAX_LEN+1,                 /* size of default value */
                _acfg_ssid_validate,                /* validate function, if any */
                _acfg_ssid_update);                 /* update hook fucntion, if any */

    SETUP_FIELD(TRUE,
                IDX_WLAN_KEY,                   /* s_name */
                CFG_GRPID_NETWORK,                /* gid of setting ID */
                CFG_RECID_NETWORK_KEY,          /* rid of setting ID */
                CFG_8BIT_T,                       /* type of element */
                WLAN_KEY_MAX_LEN+1,                                 /* number of elements */
                ac_key_def,                    /* default value */
                WLAN_KEY_MAX_LEN+1,                                /* size of default value */
                _acfg_key_validate,                /* validate function, if any */
                _acfg_key_update);               /* update hook fucntion, if any */
    SETUP_FIELD(TRUE,
                IDX_WLAN_AP,                   /* s_name */
                CFG_GRPID_NETWORK,                /* gid of setting ID */
                CFG_RECID_NETWORK_AP,          /* rid of setting ID */
                CFG_8BIT_T,                       /* type of element */
                sizeof(NET_802_11_ASSOCIATE_T),                 /* number of elements */
                &wlan_ap_default,                    /* default value */
                sizeof(NET_802_11_ASSOCIATE_T),                 /* size of default value */
                _acfg_wlan_ap_validate,                /* validate function, if any */
                _acfg_wlan_ap_update);                 /* update hook fucntion, if any */

    return APP_CFGR_OK;
}

INT32 a_cfg_set_wlan_mac(CHAR *pc_value)
{
    INT32 i4_ret;

    i4_ret = acfg_set(IDX_WLAN_MAC, pc_value, WLAN_MAC_MAX_LEN + 1);
    if(i4_ret != APP_CFGR_OK)
    {
        DBG_ACFG_ERR("<ACFG> a_cfg_set_wlan_mac fail, error code: %d\n", i4_ret);
        return i4_ret;
    }

    return APP_CFGR_OK;
}

INT32 a_cfg_get_wlan_mac(CHAR *pc_value)
{
    INT32    i4_ret;
    SIZE_T   z_size;

    i4_ret = acfg_get(IDX_WLAN_MAC, pc_value, &z_size);
    if(i4_ret != APP_CFGR_OK)
    {
        DBG_ACFG_ERR("<ACFG> a_cfg_get_wlan_mac fail, error code: %d\n", i4_ret);
        return i4_ret;
    }

    return APP_CFGR_OK;
}

INT32 a_cfg_set_wlan_mac_default(VOID)
{
    INT32 i4_ret;

    DBG_ACFG_INFO("<ACFG> a_cfg_set_wlan_mac_default\n");

    i4_ret = acfg_set_default(IDX_WLAN_MAC);
    if(i4_ret != APP_CFGR_OK)
    {
        DBG_ACFG_ERR("<ACFG> a_cfg_set_wlan_mac_default fail, error code: %d\n", i4_ret);
        return i4_ret;
    }

    return APP_CFGR_OK;
}

INT32 a_cfg_set_wlan_key(CHAR *pc_value)
{
    INT32 i4_ret;

    i4_ret = acfg_set(IDX_WLAN_KEY, pc_value, WLAN_KEY_MAX_LEN + 1);
    if(i4_ret != APP_CFGR_OK)
    {
        DBG_ACFG_ERR("<ACFG> a_cfg_set_wlan_key fail, error code: %d\n", i4_ret);
        return i4_ret;
    }

    return APP_CFGR_OK;
}

INT32 a_cfg_get_wlan_key(CHAR *pc_value)
{
    INT32    i4_ret;
    SIZE_T   z_size;

    i4_ret = acfg_get(IDX_WLAN_KEY, pc_value, &z_size);
    if(i4_ret != APP_CFGR_OK)
    {
        DBG_ACFG_ERR("<ACFG> a_cfg_get_wlan_key fail, error code: %d\n", i4_ret);
        return i4_ret;
    }

    return APP_CFGR_OK;
}

INT32 a_cfg_set_wlan_key_default(VOID)
{
    INT32 i4_ret;

    DBG_ACFG_INFO("<ACFG> a_cfg_set_wlan_key_default\n");

    i4_ret = acfg_set_default(IDX_WLAN_KEY);
    if(i4_ret != APP_CFGR_OK)
    {
        DBG_ACFG_ERR("<ACFG> a_cfg_set_wlan_key_default fail, error code: %d\n", i4_ret);
        return i4_ret;
    }

    return APP_CFGR_OK;
}

INT32 a_cfg_set_wlan_ap(NET_802_11_ASSOCIATE_T t_wifi_set )
{
    DBG_ACFG_ERR("<HOMEMENU> %s,L%d\n",__FUNCTION__,__LINE__);
    INT32 i4_ret;

    i4_ret = acfg_set(IDX_WLAN_AP, &t_wifi_set , sizeof(NET_802_11_ASSOCIATE_T));
    if (i4_ret != APP_CFGR_OK)
    {
        DBG_ACFG_ERR("<ACFG> a_cfg_set_wlan_ap fail, error code: %d\n", i4_ret);
        return i4_ret;
    }

    return APP_CFGR_OK;
}


INT32 a_cfg_get_wlan_ap(NET_802_11_ASSOCIATE_T* t_wifi_get)
{
    INT32    i4_ret;
    SIZE_T   z_size;

    i4_ret = acfg_get(IDX_WLAN_AP, t_wifi_get , &z_size);
    if (i4_ret != APP_CFGR_OK)
    {
        DBG_ACFG_ERR("<ACFG> a_cfg_get_wlan_ap fail, error code: %d\n", i4_ret);
        return i4_ret;
    }


    return APP_CFGR_OK;
}


INT32 a_cfg_set_wlan_ap_default(VOID)
{
    INT32 i4_ret;

    DBG_ACFG_ERR("<ACFG> a_cfg_set_wlan_ap_default\n");

    i4_ret = acfg_set_default(IDX_WLAN_AP);
    if (i4_ret != APP_CFGR_OK)
    {
        DBG_ACFG_ERR("<ACFG> a_cfg_set_wlan_ap_default fail, error code: %d\n", i4_ret);
        return i4_ret;
    }

    return APP_CFGR_OK;
}

#endif

