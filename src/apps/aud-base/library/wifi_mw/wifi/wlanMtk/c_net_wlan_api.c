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


/*----------------------------------------------------------------------------*
 * $RCSfile: c_net_wlan_api.c,v $
 * $Revision:
 * $Date:
 * $Author: yali.wu $
 *
 * Description:
 *         This file contains control API of Wifi Driver for Application.
 *---------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------
                    include files
-----------------------------------------------------------------------------*/


#include "../wlanCtrl/x_net_wlan_ctrl.h"
#include "../wlanCtrl/_net_wlan_init.h"
#include "../wlanCtrl/wifi_log.h"
#include "../wlanCtrl/wifi_print.h"
#include "../wlanCtrl/wifi_com.h"
#include "../wlanCtrl/wlan_command.h"
#include "../wlanCtrl/wifi_parse.h"

extern int frequency_to_channel(int freq);
extern int channel_to_frequency(int channel);
extern INT32 wpa_get_status_from_ret_buf(char * buf,NET_802_11_BSS_INFO_T *pCurrBss);
extern int wlan_associate(NET_802_11_ASSOCIATE_T *pAssociate);
extern void _notify_events(WLAN_NOTIFY_ID_T event , void * param );
//extern INT32 wps_generate_pin_code(char *wps_pin_code);
extern int u1WlanDrv_NormalizeSignal(int signal);
extern char   if_name[WLAN_WLAN_IF_MAX_LEN];
extern const char* get_if_name();
extern x_net_wlan_notify_fct pf_wlan_nfy_fct;


char   wps_pin_code[WLAN_PIN_LENGTH] = {'1', '2', '3', '4', '5', '6', '7', '8'};

extern INT32 wps_c_status;
static BOOL b_wlan_send_cmd = FALSE;
static NET_802_11_T g_curr_wireless_type;
extern SEMA_HANDLE h_WlanCmdSema;
extern SEMA_HANDLE h_WlanNfySema;
extern BOOL g_b_wps_associate;
extern BOOL b_wps_cancel;
#if WLAN_MULTI_CREDENTIAL_SELECTION
extern BOOL b_start_scan_for_cred;
extern BOOL b_read_event_for_cred_started;
extern BOOL b_stop_read_event;
extern NET_802_11_ASSOCIATE_T g_wps_cred[WLAN_MAX_WPS_CRED_NUM] ;
extern int g_cred_num;
#endif



#if PLATFORM_IS_TV
INT32 c_net_wlan_start_driver(x_net_wlan_notify_fct pfNotify)
{
    WIFI_LOG_ENTRY(WIFI_API,(""));
    int i4Ret = x_net_wlan_start_driver (pfNotify,0);
    WIFI_CHK_FAIL (i4Ret);
    return i4Ret;
}

INT32 c_net_wlan_stop_driver(x_net_wlan_notify_fct pfNotify)
{
    WIFI_LOG_ENTRY(WIFI_API,(""));
    int i4Ret = x_net_wlan_stop_driver(pfNotify);
    WIFI_CHK_FAIL (i4Ret);
    return i4Ret;
}
#endif

INT32 c_net_wlan_wpa_reg_cbk(x_net_wlan_notify_fct pfNotify)
{
    WIFI_LOG_ENTRY(WIFI_API,(""));
    int i4Ret = x_net_wlan_wpa_reg_cbk (pfNotify);
    WIFI_CHK_FAIL (i4Ret);
    return i4Ret;
}

INT32 c_net_wlan_wpa_unreg_cbk(x_net_wlan_notify_fct pfNotify)
{
    WIFI_LOG_ENTRY(WIFI_API,(""));
    int i4Ret = x_net_wlan_wpa_unreg_cbk (pfNotify);
    WIFI_CHK_FAIL (i4Ret);
    return i4Ret;
}


INT32 c_net_wlan_open_connection(const char *ifname)
{
    WIFI_LOG_ENTRY(WIFI_INFO,("ifname = %s",ifname));
#if PLATFORM_IS_BDP
    x_strncpy(if_name, ifname, strlen(ifname));
#endif
    int i4Ret = x_net_wlan_open_connection(ifname);
	x_net_wps_generate_pin_code(wps_pin_code);
    WIFI_CHK_FAIL (i4Ret);
    i4Ret = wifi_sema_init(&h_WlanCmdSema);
    WIFI_CHK_FAIL (i4Ret);
    i4Ret = wifi_sema_init(&h_WlanNfySema);
    WIFI_CHK_FAIL (i4Ret);
    return i4Ret;
}


INT32 c_net_wlan_open_connection_per_path(const char *ifname, char *ctrl_path, char *moni_path)
{
    WIFI_LOG_ENTRY(WIFI_API,("ifname = %s, ctrl_path=%s, moni_path=%s", ifname, ctrl_path, moni_path));
#if PLATFORM_IS_BDP
    x_strncpy(if_name, ifname, strlen(ifname));
#endif
    int i4Ret = x_net_wlan_open_connection_per_path(ifname, ctrl_path, moni_path);
	x_net_wps_generate_pin_code(wps_pin_code);
    WIFI_CHK_FAIL (i4Ret);
    i4Ret = wifi_sema_init(&h_WlanCmdSema);
    WIFI_CHK_FAIL (i4Ret);
    i4Ret = wifi_sema_init(&h_WlanNfySema);
    WIFI_CHK_FAIL (i4Ret);
    return i4Ret;
}


INT32 c_net_wlan_close_connection(void)
{
    WIFI_LOG_ENTRY(WIFI_API,(""));
    int i4Ret = x_net_wlan_close_connection ();
    WIFI_CHK_FAIL (i4Ret);

    i4Ret = sema_uninit(&h_WlanCmdSema);
    WIFI_CHK_FAIL (i4Ret);
    i4Ret = sema_uninit(&h_WlanNfySema);
    WIFI_CHK_FAIL (i4Ret);

    return i4Ret;
}


/*  Set the time in seconds between scans to find suitable AP */
INT32 c_net_wlan_set_scan_interval(INT32 scan_interval)
{
    WIFI_LOG_ENTRY(WIFI_API,("scan interval=%d", (int)scan_interval));

    char _cmd_buf[128]={'\0'};

    snprintf(_cmd_buf, sizeof(_cmd_buf), "SCAN_INTERVAL %d", (int)scan_interval);
    if (!doBooleanCommand(_cmd_buf, "OK"))
    {
        WIFI_LOG(WIFI_ERROR,(">>>>>>>set scan interval return error!\n"));
        return NET_WLAN_ERROR;
    }

    return NET_WLAN_OK;
}


INT32 c_net_wlan_scan(x_net_wlan_notify_fct pfNotify)
{
    WIFI_LOG_ENTRY(WIFI_API,(""));
    INT32 i4Ret = NET_WLAN_OK;
    char reply[256]= {'\0'};

    if (wlan_check_init_state() != NET_WLAN_OK)
    {
        return NET_WLAN_NOT_INIT;
    }
    x_net_wlan_wpa_reg_cbk(pfNotify);

    if (doCommand("SCAN", reply, sizeof(reply)) != 0)
    {
        WIFI_LOG( WIFI_ERROR, ( "[WIFI MW]>>>>>>> %s, %d fail, REPLY IS= %s \n", __FUNCTION__, __LINE__, reply ) );
        if (strncmp(reply, "FAIL-BUSY", 9) == 0)
        {
            WIFI_LOG(WIFI_ERROR, ("A new scan has been trigger, so reject this scan request."));
            i4Ret = NET_WLAN_OK;
        }
        else
        {
            i4Ret = NET_WLAN_ERROR;
        }
    }
    else
    {
        WIFI_LOG( WIFI_DEBUG, ( "[WIFI MW]>>>>>>> %s, %d, REPLY IS= %s \n", __FUNCTION__, __LINE__, reply ) );
        if (strncmp(reply, "OK", 2) == 0)
        {
            i4Ret = NET_WLAN_OK;
        }
        else
        {
            i4Ret = NET_WLAN_ERROR;
        }
    }

    return i4Ret;
}


INT32 c_net_wlan_set_auto_scan(void)
{
    WIFI_LOG_ENTRY(WIFI_API,(""));
    INT32 i4Ret = NET_WLAN_OK;
    char _cmd_buf[128] = {'\0'};

    if (wlan_check_init_state() != NET_WLAN_OK)
    {
        return NET_WLAN_NOT_INIT;
    }

    snprintf(_cmd_buf, sizeof(_cmd_buf), "SET autoscan exponential:2:30");

    if (!dop2pBooleanCommand(_cmd_buf, "OK"))
    {
        i4Ret = NET_WLAN_ERROR;
        WIFI_LOG(WIFI_ERROR,(">>>>>> c_net_wlan_set_auto_scan command return error\n"));
    }
    
    return i4Ret;
}


INT32 c_net_wlan_get_scan_result(NET_802_11_SCAN_RESULT_T *pScanResult)
{

    WIFI_LOG_ENTRY(WIFI_API,("ifname = %s",if_name));
    int i4Ret = x_net_wlan_get_scan_result (pScanResult);
    WIFI_CHK_FAIL (i4Ret);
    print_scan_result (pScanResult);
    return i4Ret;


}


/* Enable/disable STA automatic reconnection */
INT32 c_net_wlan_set_sta_auto_reconnect(int enableFlag)
{
    WIFI_LOG_ENTRY(WIFI_API, ("STA auto connect enable flag=%d.", enableFlag));
    char _cmd_buf[128]={'\0'};

    snprintf(_cmd_buf, sizeof(_cmd_buf), "STA_AUTOCONNECT %d", enableFlag);
    if (!doBooleanCommand(_cmd_buf, "OK"))
    {
        WIFI_LOG(WIFI_ERROR,(">>>>>>>set sta autoconnect return error!\n"));
        return NET_WLAN_ERROR;
    }

    return NET_WLAN_OK;
}

/* Start STA reconnection */
INT32 c_net_wlan_sta_reconnect(void)
{
    WIFI_LOG_ENTRY(WIFI_API, (""));

    int ret = -1;
    NET_802_11_ASSOCIATE_T Associate;

    ret = wlan_set_reconnect_network(&Associate);
    if ( ret != 0 )
    {
        WIFI_LOG(WIFI_ERROR,(">>>>>>>sta reconnect return error!\n"));
        return NET_WLAN_ERROR;
    }

    return NET_WLAN_OK;
}

INT32 c_net_wlan_enable_network(void)
{
    WIFI_LOG_ENTRY(WIFI_API, (""));

    int ret = -1;
    NET_802_11_ASSOCIATE_T *pAssociate = NULL;
	
    ret = wlan_set_enable_network(pAssociate);
    if ( ret != 0 ) {
        WIFI_LOG(WIFI_ERROR,(">>>>>>>sta enable network error!\n"));
        return NET_WLAN_ERROR;
    }

    return NET_WLAN_OK;
}

INT32 c_net_wlan_associate(NET_802_11_ASSOCIATE_T *pAssociate,
                           x_net_wlan_notify_fct pfNotify)
{
    WIFI_LOG_ENTRY(WIFI_API,(""));
    print_associate (pAssociate);

    INT32 i4Ret = NET_WLAN_OK;
    if (wlan_check_init_state() != NET_WLAN_OK )
    {
        return NET_WLAN_ERROR;
    }
    g_b_wps_associate = FALSE;
#if WLAN_MULTI_CREDENTIAL_SELECTION
    b_stop_read_event = TRUE;
#endif

    x_net_wlan_wpa_reg_cbk(pfNotify);
    i4Ret = wlan_associate(pAssociate);

    WIFI_CHK_FAIL (i4Ret);
    return i4Ret;
}

INT32 c_net_wlan_disassociate(void)
{
    WIFI_LOG_ENTRY(WIFI_API,(""));
    int i4Ret = x_net_wlan_disassociate ();
    WIFI_CHK_FAIL (i4Ret);
    return i4Ret;
}

INT32 c_net_wlan_cancel_associate(VOID)
{
    WIFI_LOG_ENTRY(WIFI_API,(""));
    int i4Ret = x_net_wlan_disassociate ();
    WIFI_CHK_FAIL (i4Ret);
    return i4Ret;

}

INT32 c_net_wlan_get_curr_bss(NET_802_11_BSS_INFO_T *pCurrBss)
{
    WIFI_LOG_ENTRY(WIFI_API,(""));
    int i4Ret = x_net_wlan_get_curr_bss(pCurrBss);
    WIFI_CHK_FAIL (i4Ret);
    return i4Ret;
}
INT32 c_net_wlan_wpa_get_connection_info(NET_802_11_BSS_INFO_T *pCurrBss)
{
    WIFI_LOG_ENTRY(WIFI_API,(""));
    int i4Ret = x_net_wlan_get_curr_bss(pCurrBss);
    WIFI_CHK_FAIL (i4Ret);
    return i4Ret;
}
INT32 c_net_wlan_get_chain_rssi(NET_802_11_CHAIN_RSSI_T *pRssi)
{
    WIFI_LOG_ENTRY(WIFI_API,(""));


    WIFI_LOG (WIFI_DEBUG, ("c_net_wlan_get_chain_rssi is old function, now should not use"));
    return NET_WLAN_ERROR;
}

INT32 c_net_wlan_get_custdata(NET_802_11_ACFG_CUSTDAT_T * pCustdata)
{
    WIFI_LOG_ENTRY(WIFI_API,(""));

    WIFI_LOG(WIFI_DEBUG, ("c_net_wlan_get_custdata is old function, now should not use"));
    return NET_WLAN_ERROR;
}
INT32 c_net_wlan_get_mac_addr(UINT8 *pMacAddr)
{
    WIFI_LOG_ENTRY(WIFI_API,(""));
	return x_net_wlan_get_mac_addr(pMacAddr);
}
INT32 c_net_wlan_wps_connect(NET_802_11_WPS_ASSOC_MODE_T eWpsMode, char *pin, UINT8 *bssid,
                             x_net_wlan_notify_fct pfNotify)
{
    WIFI_LOG_ENTRY(WIFI_API,("eWpsMode [%d]",eWpsMode));
    if (wlan_check_init_state() != NET_WLAN_OK)
    {
        return NET_WLAN_NOT_INIT;
    }

    NET_802_11_NFY_T t_nfy;
    x_memset(&t_nfy, 0, sizeof(NET_802_11_NFY_T));
    BOOL b_ret=FALSE;

    x_net_wlan_wpa_reg_cbk(pfNotify);
    switch (eWpsMode)
    {
    case IEEE_802_11_PIN_SETUP:
    case IEEE_802_11_PBC_SETUP:
    case IEEE_802_11_NFC_SETUP:
    case IEEE_802_11_USB_SETUP:
        break;
    default:
        return NET_WLAN_INVALID_PARAM;
    }
#if WLAN_MULTI_CREDENTIAL_SELECTION
    memset(g_wps_cred, 0, sizeof(NET_802_11_ASSOCIATE_T)*WLAN_MAX_WPS_CRED_NUM);
    g_cred_num = 0;
    b_start_scan_for_cred = FALSE;
    b_read_event_for_cred_started = FALSE;
    b_stop_read_event = FALSE;
#endif
    g_b_wps_associate = TRUE;
    b_wps_cancel = FALSE;

    if (eWpsMode == IEEE_802_11_PBC_SETUP)
    {
        b_ret = doBooleanCommand("WPS_PBC any", "OK");
    }
    else if (eWpsMode == IEEE_802_11_PIN_SETUP)
    {
        char cmd[128] = {0};
        char tmp_wps_pin[WLAN_PIN_LENGTH+1]= {0};
        if (pin)
        {
            memcpy (tmp_wps_pin, pin, WLAN_PIN_LENGTH);
        }
        else
        {
            memcpy (tmp_wps_pin, wps_pin_code, WLAN_PIN_LENGTH);
        }
        if (bssid)
        {
            snprintf(cmd,sizeof(cmd),"WPS_PIN %02x:%02x:%02x:%02x:%02x:%02x %s",
                     bssid[0], bssid[1], bssid[2], bssid[3], bssid[4], bssid[5],tmp_wps_pin);
        }
        else
        {
            snprintf(cmd,sizeof(cmd),"WPS_PIN any %s",tmp_wps_pin);
        }
        b_ret = doBooleanCommand(cmd, "OK");
    }
    if (b_ret)
    {
        WIFI_LOG(WIFI_DEBUG, ("C2H: notify-app: WLAN_MW_WPS_DISCOVER\n"));
        t_nfy.i4NotifykId = WLAN_NOTIFY_WPS_STATE;
        t_nfy.pParam[0] = WLAN_MW_WPS_DISCOVER;
        _notify_events(t_nfy.i4NotifykId, t_nfy.pParam);
        wps_c_status = WLAN_MW_WPS_DISCOVER;
    }
    else
    {
        WIFI_LOG(WIFI_DEBUG, ("C2H: notify-app: WLAN_MW_WPS_FAIL\n"));
        t_nfy.i4NotifykId = WLAN_NOTIFY_WPS_STATE;
        t_nfy.pParam[0] = WLAN_MW_WPS_FAIL;
        _notify_events(t_nfy.i4NotifykId, t_nfy.pParam);
        wps_c_status = WLAN_MW_WPS_FAIL;
    }
    return NET_WLAN_OK;

}


INT32 c_net_wlan_wps_associate(NET_802_11_WPS_ASSOC_MODE_T eWpsMode,
                               x_net_wlan_notify_fct pfNotify)
{
    WIFI_LOG_ENTRY(WIFI_API,("eWpsMode [%d]",eWpsMode));
    return c_net_wlan_wps_connect(eWpsMode,NULL,NULL,pfNotify);
}

INT32 c_net_wlan_wps_get_status(INT32 *status)
{
    WIFI_LOG_ENTRY(WIFI_API,(""));

    if (wlan_check_init_state() != NET_WLAN_OK)
        return NET_WLAN_NOT_INIT;

    return wps_c_status;
}

INT32 c_net_wlan_wps_cancel(void)
{
    WIFI_LOG_ENTRY(WIFI_API,(""));

    if (wlan_check_init_state() != NET_WLAN_OK)
        return NET_WLAN_NOT_INIT;

    if ( !doBooleanCommand("WPS_CANCEL", "OK"))
        return NET_WLAN_ERROR;

    b_wps_cancel = TRUE;

    return NET_WLAN_OK;

}

INT32 c_net_wlan_wps_refresh_pin(void)
{
    WIFI_LOG_ENTRY(WIFI_API,(""));

    INT32 i4Ret = NET_WLAN_OK;

    if (wlan_check_init_state() != NET_WLAN_OK)
        return NET_WLAN_NOT_INIT;

    wps_generate_pin_code(wps_pin_code);
    return i4Ret;
}

INT32 c_net_wlan_wps_get_pin(char *pin, UINT32 len)
{
    WIFI_LOG_ENTRY(WIFI_API,(""));

    INT32 i4Ret = NET_WLAN_OK;
    if (wlan_check_init_state() != NET_WLAN_OK)
        return NET_WLAN_NOT_INIT;

    x_memcpy(pin, wps_pin_code, WLAN_PIN_LENGTH);

    return i4Ret;
}

INT32 c_net_wlan_get_ess_list(NET_802_11_ESS_LIST_T *pEssList_result)
{
    WIFI_LOG_ENTRY(WIFI_API,(""));
    int i4Ret = x_net_wlan_get_ess_list(pEssList_result);
    WIFI_CHK_FAIL (i4Ret);
    print_esslist(pEssList_result);
    return i4Ret;
}

INT32 c_net_wlan_scan_with_ssid(x_net_wlan_notify_fct pfNotify, char * ssid, int ssid_len)
{
    WIFI_LOG_ENTRY(WIFI_API,("ssid = [%s]",ssid));
    int i4Ret = x_net_wlan_scan_with_ssid (pfNotify, ssid, ssid_len);
    WIFI_CHK_FAIL (i4Ret);
    return i4Ret;
}

INT32 c_net_wlan_task_create(void)
{
    WIFI_LOG_ENTRY(WIFI_API,(""));
    return NET_WLAN_OK;
}


INT32 c_net_wlan_set_send_true(void)
{
    if (wlan_check_init_state() != NET_WLAN_OK)
    {
        return NET_WLAN_NOT_INIT;
    }

    b_wlan_send_cmd = TRUE;
    return 0;
}

INT32 c_net_wlan_set_usb_suspend(BOOL enable)
{
    WIFI_LOG_ENTRY(WIFI_API,("enable [%d]",enable));
    char _cmd_buf[32]={0};
    int value = enable?1:0;
    snprintf(_cmd_buf ,sizeof(_cmd_buf),"SET usb_suspend %d",value);

    if (!doBooleanCommand(_cmd_buf, "OK"))
    {
        WIFI_LOG(WIFI_ERROR ,(">>>>>>> %s fail,\n",_cmd_buf));
        return NET_WLAN_ERROR;
    }
    return NET_WLAN_OK;
}

INT32 c_net_wlan_set_usb_powersaving(BOOL enable)
{
    WIFI_LOG_ENTRY(WIFI_API,("enable [%d]",enable));
    char _cmd_buf[32]={0};
    int value = enable?1:0;
    snprintf(_cmd_buf ,sizeof(_cmd_buf),"SET usb_powersaving %d",value);

    if (!doBooleanCommand(_cmd_buf, "OK"))
    {
        WIFI_LOG(WIFI_ERROR ,(">>>>>>> %s fail,\n",_cmd_buf));
        return NET_WLAN_ERROR;
    }
    return NET_WLAN_OK;

}

#ifdef SUPPORT_2_DONGLES
INT32 c_net_wlan_set_use_ifname (char *s_ifname)
{
    WIFI_LOG_ENTRY(WIFI_API,("s_ifname [%s]",s_ifname));

    return x_net_wlan_set_use_ifname(s_ifname);
}

#endif

INT32 c_net_wlan_set_mac_addr(UINT8 *pMacAddr)
{
    INT32 i4Ret = NET_WLAN_OK;

    return i4Ret;
}
INT32 c_net_wlan_get_wlan_version(void)
{
    INT32 i4Ret = NET_WLAN_OK;
    if (wlan_check_init_state() != NET_WLAN_OK)
    {
        return NET_WLAN_NOT_INIT;
    }

    char cmd[60] = {0};
    snprintf(cmd, sizeof(cmd), "wpa_cli -i %s driver_version\n", if_name );
    printf(">>>> cmd->%s", cmd);

    i4Ret = doScriptCmd(cmd);
    if (i4Ret != NET_WLAN_OK)
    {
        printf("[WIFI_MW] get wlan fail.\n");
    }
    return i4Ret;
}

INT32 c_net_wlan_set_debug_level(int fg_enable, int level)
{
    return wlan_set_dbg_level(fg_enable,level);
}

INT32 c_net_wlan_get_debug_level()
{
    return wlan_get_dbg_level();
}

INT32 c_net_wlan_get_curr_connection_info(NET_802_11_BSS_INFO_T *pCurrBss)
{
    INT32 i4Ret = NET_WLAN_OK;
    NET_802_11_SSID_T t_ssid_temp;
    x_memset(&t_ssid_temp, 0, sizeof(NET_802_11_SSID_T));
    NET_802_11_BSS_INFO_T temp_Bss;
    memset(&temp_Bss, 0, sizeof(NET_802_11_BSS_INFO_T));

    if (wlan_check_init_state() != NET_WLAN_OK)
    {
        return NET_WLAN_NOT_INIT;
    }

    WIFI_LOG_ENTRY(WIFI_DEBUG, (" ssid=%s, bssid=%02x:%02x:%02x:%02x:%02x:%02x\n",
                                pCurrBss->t_Ssid.ui1_aSsid,
                                pCurrBss->t_Bssid[0],pCurrBss->t_Bssid[1],pCurrBss->t_Bssid[2],pCurrBss->t_Bssid[3],pCurrBss->t_Bssid[4],pCurrBss->t_Bssid[5]));

    int i;

    char reply[4096] = {0};
    size_t reply_len = sizeof(reply);

    if (doStringCommand("STATUS", reply, &reply_len) == NULL)
        return NET_WLAN_ERROR;


    /* find current bit rate */
    char *pRate = NULL;
    char *pSpace = NULL;
    char rate_str[8] = {'\0'};
    pRate = strstr(reply, "current_rate=");
    if (pRate == NULL)
    {
        i4Ret = NET_WLAN_DRV_INTERFACE_ERR;
        goto out;
    }
    pRate = pRate + strlen("current_rate=");
    pSpace = strstr(pRate, " ");
    if (pSpace == NULL)
    {
        i4Ret = NET_WLAN_DRV_INTERFACE_ERR;
        goto out;
    }
    strncpy(rate_str, pRate, pSpace - pRate);

    if (wpa_get_status_from_ret_buf(reply, pCurrBss) < 0)
    {
        i4Ret = NET_WLAN_DRV_INTERFACE_ERR;
        goto out;
    }



    if (pCurrBss->u8_wpa_status == WLAN_WPA_COMPLETED) //if current state is completed,then get currbss from scan results
    {
#if 1//fix channel problem
        /* when get curr bss, need to copy the spefic AP info from scan result . */
        NET_802_11_SCAN_RESULT_T ScanResult;
        memset(&ScanResult,0,sizeof(NET_802_11_SCAN_RESULT_T));
        if (c_net_wlan_get_scan_result(&ScanResult) < 0)
        {
            printf("Get scan results failed!\n");
            i4Ret = NET_WLAN_DRV_INTERFACE_ERR;
            goto out;
        }

        int j = 0;
        int bssid_matched = 0;
        x_memcpy(&t_ssid_temp, &(pCurrBss->t_Ssid), sizeof(NET_802_11_SSID_T));
        WIFI_LOG(WIFI_DEBUG, ("--------- The current connected AP SSID = %s, BSSID = %02x:%02x:%02x:%02x:%02x:%02x ---------\n",
                              pCurrBss->t_Ssid.ui1_aSsid,
                              pCurrBss->t_Bssid[0], pCurrBss->t_Bssid[1], pCurrBss->t_Bssid[2],
                              pCurrBss->t_Bssid[3], pCurrBss->t_Bssid[4], pCurrBss->t_Bssid[5]));
        for (i = 0; i < ScanResult.ui4_NumberOfItems; i++)
        {
            for (j=0; j<6; j++)
            {
                if (pCurrBss->t_Bssid[j] == ScanResult.p_BssInfo[i].t_Bssid[j])
                {
                    bssid_matched = 1;
                }
                else
                {
                    bssid_matched = 0;
                    break;
                }
            }

            if (1 == bssid_matched)
            {
                if (strncmp(pCurrBss->t_Ssid.ui1_aSsid, ScanResult.p_BssInfo[i].t_Ssid.ui1_aSsid, NET_802_11_MAX_LEN_SSID) == 0)
                {
                    x_memcpy(&temp_Bss, &(ScanResult.p_BssInfo[i]), sizeof(NET_802_11_BSS_INFO_T));
                    //x_memcpy(&(pCurrBss->t_Ssid), &t_ssid_temp, sizeof(NET_802_11_SSID_T)); //for returning SSID of AP which has hidden SSID.

                    INT32 freq = 0;
                    INT32 bw = 0;
                    if (temp_Bss.i2_Freq > 5000)
                    {
                        freq = 5;
                    }
                    else
                    {
                        freq = 2;
                    }

                    if (_get_STA_bandwidth(&bw) < 0)
                    {
                        WIFI_LOG(WIFI_DEBUG, ("failed get bandwidth!"));
                        if (temp_Bss.e_802_11_bw == OID_802_11_BW20)
                        {
                            bw = 20;
                            WIFI_LOG(WIFI_DEBUG, ("set bandwidth = 20MHz, maybe it is not right.\n"));
                        }
                        else
                        {
                            bw = 40;
                            WIFI_LOG(WIFI_DEBUG, ("set bandwidth = 40MHz, maybe it is not right.\n"));
                        }
                    }
                    else  //notify iwpriv result to app
                    {
                        if (bw == 20)
                            temp_Bss.e_802_11_bw = OID_802_11_BW20;
                        else
                            temp_Bss.e_802_11_bw = OID_802_11_BW40;
                    }

                    /* determine which protocol is used for connection */
                    _wpa_determine_wireless_type(freq, bw, rate_str, &g_curr_wireless_type);
                    pCurrBss->e_802_11_type = g_curr_wireless_type;

                    pCurrBss->e_802_11_bw = temp_Bss.e_802_11_bw;
                    pCurrBss->i2_Freq = temp_Bss.i2_Freq;
                    pCurrBss->i2_Channel = temp_Bss.i2_Channel;
                    pCurrBss->i2_Quality = temp_Bss.i2_Quality;
                    pCurrBss->i2_Noise = temp_Bss.i2_Noise;
                    pCurrBss->is_wps_support = temp_Bss.is_wps_support;

                    WIFI_LOG(WIFI_INFO, (" get 1 bss ssid=%s, bssid = %02x:%02x:%02x:%02x:%02x:%02x, AuthCip=%d, AuthMode=%d, Cha=%d,Fre=%d,Level=%d,Noise=%d,Qua=%d,MaxRate=%d, Caps=%d, ieLen=%d.\n",
                                         pCurrBss->t_Ssid.ui1_aSsid,
                                         pCurrBss->t_Bssid[0], pCurrBss->t_Bssid[1], pCurrBss->t_Bssid[2], pCurrBss->t_Bssid[3], pCurrBss->t_Bssid[4], pCurrBss->t_Bssid[5],
                                         pCurrBss->e_AuthCipher,
                                         pCurrBss->e_AuthMode,
                                         pCurrBss->i2_Channel,
                                         pCurrBss->i2_Freq,
                                         pCurrBss->i2_Level,
                                         pCurrBss->i2_Noise,
                                         pCurrBss->i2_Quality,
                                         pCurrBss->e_802_11_type,
                                         pCurrBss->i4_MaxRate,
                                         pCurrBss->is_wps_support,
                                         pCurrBss->u2_Caps,
                                         pCurrBss->u4_ieLength));
                }

                break;
            }
        }

        if (i >= ScanResult.ui4_NumberOfItems)
        {
            WIFI_LOG(WIFI_DEBUG, ("Can't find connected AP information from scan results!"));
            return -1;
        }

#endif

        WIFI_LOG(WIFI_INFO, (" get 2 bss ssid=%s(%d), bssid = %02x:%02x:%02x:%02x:%02x:%02x, AuthCip=%d, AuthMode=%d, Cha=%d,Fre=%d,Level=%d,Noise=%d,Qua=%d,MaxRate=%d, Caps=%d, ieLen=%d.\n",
                             pCurrBss->t_Ssid.ui1_aSsid,
                             pCurrBss->t_Ssid.ui4_SsidLen,
                             pCurrBss->t_Bssid[0], pCurrBss->t_Bssid[1], pCurrBss->t_Bssid[2], pCurrBss->t_Bssid[3], pCurrBss->t_Bssid[4], pCurrBss->t_Bssid[5],
                             pCurrBss->e_AuthCipher,
                             pCurrBss->e_AuthMode,
                             pCurrBss->i2_Channel,
                             pCurrBss->i2_Freq,
                             pCurrBss->i2_Level,
                             pCurrBss->i2_Noise,
                             pCurrBss->i2_Quality,
                             pCurrBss->i4_MaxRate,
                             pCurrBss->u2_Caps,
                             pCurrBss->u4_ieLength));
    }
    else
    {
        x_memset(pCurrBss, 0, sizeof(NET_802_11_BSS_INFO_T));
    }
out:

    return i4Ret;

}


INT32 c_net_wlan_set_ssid_to_driver(char *iface, char *ssid)
{
    WIFI_LOG_ENTRY(LogDefault, (""));

    char ssidTmp[NET_802_11_MAX_LEN_SSID * 2 + 1] = {'\0'};
    char ifaceTmp[16] = { '\0' };
    char cmdStr[128] = {'\0'};
    char *ssidPtr = ssid;
    char *TmpPtr = ssidTmp;
    int i = 0;

    if ( !iface || !ssid )
    {
        WIFI_LOG(WIFI_DEBUG, ("Invalid parameter, ssid is NULL."));
        return NET_WLAN_ERROR;
    }

    //escape '\\', '`', '$' and '"' when ssid includes them for shell
    for (i = 0; i < NET_802_11_MAX_LEN_SSID; i++)
    {
        if ( *ssidPtr == '\0' )
        {
            break;
        }

        if ( *ssidPtr == '\\' || *ssidPtr == '`' || *ssidPtr == '$' || *ssidPtr == '"')
        {
            *TmpPtr++ = '\\';
        }
        *TmpPtr++ = *ssidPtr++;
    }

    ssidTmp[sizeof(ssidTmp) - 1] = '\0';
    strncpy( ifaceTmp, iface, sizeof(ifaceTmp) - 1 );
    snprintf( cmdStr, sizeof(cmdStr), "iwpriv %s set H_SSID=\"%s\"", ifaceTmp, ssidTmp );


    if ( doScriptCmd(cmdStr) != 0 )
    {
        WIFI_LOG(WIFI_DEBUG, ("command failed: %s.", cmdStr));
        return NET_WLAN_ERROR;
    }

    return NET_WLAN_OK;
}

/*
This function is used to get wireless type of current connection.
This function should be called after c_net_wlan_get_curr_bss( ) has been called.
*/
INT32 c_net_wlan_get_curr_wireless_type(NET_802_11_T *wireless_type)
{
    WIFI_LOG_ENTRY(WIFI_DEBUG, (""));

    if (wireless_type == NULL)
    {
        WIFI_LOG(WIFI_DEBUG, ("Invalid parameters!"));
        return -1;
    }
    *wireless_type = g_curr_wireless_type;

    return 0;
}
INT32 c_net_wlan_set_multi_channel(BOOL enable)
{
    if (wlan_check_init_state() != NET_WLAN_OK)
    {
        return NET_WLAN_NOT_INIT;
    }
    char _cmd_buf[32]={0};
    int value = enable?1:0;
    snprintf(_cmd_buf ,sizeof(_cmd_buf),"SET multi_channel_enable %d",value);

    if (!doBooleanCommand(_cmd_buf, "OK"))
    {
        printf( "[WIFI MW]>>>>>>> %s fail,\n",_cmd_buf);
        return NET_WLAN_ERROR;
    }
    return NET_WLAN_OK;
}


/* Get signal srength, freq and so on after Infra connection established */
INT32 c_net_wlan_wpa_signal_poll(NET_802_11_SIGNAL_DATA_T *pSignal)
{
    WIFI_LOG_ENTRY(WIFI_DEBUG, (""));

    SIGNAL_DATA_T _signalData;
    memset(&_signalData, 0, sizeof(_signalData));
    INT32 ret = -1;
    
    ret = x_net_wlan_wpa_signal_poll(&_signalData);
    
    pSignal->Rssi = _signalData.rssi;
    pSignal->LinkSpeed = _signalData.linkSpeed;
    pSignal->Noise = _signalData.noise;
    pSignal->Freq = _signalData.freq;

    return ret;
}


INT32 c_net_wlan_wpa_get_status(INT32 *pStatus,NET_802_11_ASSOCIATE_T *pAssociate)
{
    INT32 i4Ret = NET_WLAN_OK;
    char *reply = NULL;
    WIFI_LOG_ENTRY(WIFI_DEBUG, (""));

    NET_802_11_BSS_INFO_T CurrBss;
    memset(&CurrBss, 0, sizeof(NET_802_11_BSS_INFO_T));

#if WLAN_STATUS_WITH_MATCHED_SSID
    char *scan_reply = NULL;
    int i=0;
    NET_802_11_SCAN_RESULT_T ScanResult;
    int u4_scan_result_num = 0;
    memset(&ScanResult, 0, sizeof(NET_802_11_SCAN_RESULT_T));
#endif

    if (pAssociate == NULL)
    {
        WIFI_LOG(WIFI_WARNING,("pAssociate == NULL\n"));
        i4Ret = NET_WLAN_NOT_INIT;
        return i4Ret;
    }
    if (wlan_check_init_state() != NET_WLAN_OK)
    {
        return NET_WLAN_NOT_INIT;
    }



    pAssociate->t_Ssid.ui4_SsidLen = strlen(pAssociate->t_Ssid.ui1_aSsid);


    size_t reply_len = 4096;
    reply = malloc(reply_len*sizeof(char));
    if (!reply)
    {
        WIFI_LOG(WIFI_ERROR,(" malloc memory failed!%s, %d\n", __FUNCTION__, __LINE__));
        i4Ret = -1;
        goto out;
    }
    memset(reply, 0, reply_len);

#if WLAN_STATUS_WITH_MATCHED_SSID
    char scan_results_cmd[16] = "SCAN_RESULTS";
    size_t replylen = 1024*12;
    scan_reply = (char *)malloc(replylen * sizeof(char));
    if (scan_reply == NULL)
    {
        WIFI_LOG(WIFI_DEBUG, ("malloc memory failed!%s, %d", __FUNCTION__, __LINE__));
        i4Ret = -1;
        goto out;
    }
    memset(scan_reply, 0, replylen);
#endif

    WIFI_LOG(WIFI_DEBUG, (" ssid=%s, len=%d, Cipher=%d, Mode=%d, Bssid=%02x:%02x:%02x:%02x:%02x:%02x; keybody=%s, Pri=%d.",
                          pAssociate->t_Ssid.ui1_aSsid,
                          pAssociate->t_Ssid.ui4_SsidLen,
                          pAssociate->e_AuthCipher,
                          pAssociate->e_AuthMode,
                          pAssociate->t_Bssid[0],
                          pAssociate->t_Bssid[1],
                          pAssociate->t_Bssid[2],
                          pAssociate->t_Bssid[3],
                          pAssociate->t_Bssid[4],
                          pAssociate->t_Bssid[5],
                          pAssociate->t_Key.key_body,
                          pAssociate->ui1_Priority));



    if (doStringCommand("STATUS", reply, &reply_len) == NULL)
    {
        i4Ret = NET_WLAN_ERROR;
        goto out;
    }

    WIFI_LOG(WIFI_DEBUG, ("\nSTATUS result=%s\n", reply));

    if (wpa_get_status_from_ret_buf(reply, &CurrBss)<0)
    {
        i4Ret = NET_WLAN_DRV_INTERFACE_ERR;
        goto out;
    }

    *pStatus = CurrBss.u8_wpa_status;

    WIFI_LOG(WIFI_DEBUG, ("WPA status=%d.", *pStatus));

#if WLAN_STATUS_WITH_MATCHED_SSID
    if (WLAN_WPA_COMPLETED != *pStatus)
    {
        WIFI_LOG(WIFI_DEBUG, ("The WPA status is not COMPLETED."));

        if (doStringCommand(scan_results_cmd, scan_reply, &replylen) == NULL)
        {
            i4Ret = NET_WLAN_ERROR;
            goto out;
        }

        if (wpa_get_scan_results_from_ret_buf(scan_reply, &ScanResult) == 0)
        {
            u4_scan_result_num = ScanResult.ui4_NumberOfItems;
            if (u4_scan_result_num > 0)
            {
                for (i=0; i<u4_scan_result_num; i++)
                {
                    if (strncmp(ScanResult.p_BssInfo[i].t_Ssid.ui1_aSsid,
                                pAssociate->t_Ssid.ui1_aSsid,
                                NET_802_11_MAX_LEN_SSID) == 0)
                    {
                        WIFI_LOG(WIFI_INFO, ("The SSID has be found in scan AP list."));
                        *pStatus = WLAN_NFY_MSG_NO_MATCHED_CONFIG;
                        goto out;
                    }
                }

            }

            WIFI_LOG(WIFI_INFO, ("The SSID didn't be found in scan AP list."));
            *pStatus = WLAN_NFY_MSG_NO_MATCHED_SSID;
        }
    }
#endif

out:

    WIFI_LOG(WIFI_DEBUG, ("in c_net_wlan_wpa_get_status() pStatus = %d\n",(int)(*pStatus)));

    if (reply != NULL)
    {
        free(reply);
        reply = NULL;
    }

#if WLAN_STATUS_WITH_MATCHED_SSID
    if (scan_reply != NULL)
    {
        free(scan_reply);
        scan_reply = NULL;
    }
#endif


    return i4Ret;
}

#if 0
INT32 c_net_wlan_wpa_get_state(WLAN_WPA_STATUS_T *WPAState)
{
    WIFI_LOG_ENTRY(WIFI_DEBUG, (""));

    char reply[512] = {'\0'};
    int replylen = sizeof(reply);
    int _wpastate = -1;
    NET_802_11_BSS_INFO_T CurrBss;
    memset(&CurrBss, 0, sizeof(NET_802_11_BSS_INFO_T));

    if (doStringCommand("STATUS", reply, &replylen) == NULL)
    {
        WIFI_LOG(WIFI_DEBUG, ("!!Failed to get WPA state."));
        return NET_WLAN_ERROR;
    }

    if (wpa_get_status_from_ret_buf(reply, &CurrBss) < 0)
    {
        WIFI_LOG(WIFI_DEBUG, ("!!Failed to get WPA state."));
        return NET_WLAN_ERROR;
    }

    _wpastate = CurrBss.u8_wpa_status;
    *WPAState = (WLAN_WPA_STATUS_T)_wpastate;

    WIFI_LOG(WIFI_INFO, ("WPA State = %d\n",(int)(*WPAState)));

    return 0;
}
#endif

/* Transform channel to frequency */
INT32 c_net_wlan_channel_to_frequency(int channel)
{
    WIFI_LOG_ENTRY(WIFI_DEBUG, (""));
    int freq = -1;
    freq = channel_to_frequency(channel);
    WIFI_LOG(WIFI_INFO, ("Frequency = %d, channel = %d", freq, channel));
    return freq;
}

/* Transform frequency to channel */
INT32 c_net_wlan_frequency_to_channel(int freq)
{
    WIFI_LOG_ENTRY(WIFI_DEBUG, (""));
    int channel = -1;
    channel = frequency_to_channel(freq);
    WIFI_LOG(WIFI_INFO, ("Channel = %d, frequency = %d", freq, channel));
    return channel;
}

INT32 c_net_wlan_get_wep_auth_mode(NET_802_11_AUTH_MODE_T *pWepAuthMode)
{
    char *pbuf = NULL;
    char *pEncrypType = NULL;
    char *pAuthMode = NULL;
    char cmd[64] = {'\0'};
    FILE *fp = NULL;
    int i = 0;

    WIFI_LOG_ENTRY(WIFI_DEBUG, (""));

    if (pWepAuthMode == NULL)
    {
        WIFI_LOG(WIFI_DEBUG, ("invalid parameter, can not get WEP auth mode!"));
        return -1;
    }

    pbuf = (char *)malloc(512);
    if (pbuf == NULL)
    {
        WIFI_LOG(WIFI_DEBUG, ("malloc memory failed, can not get WEP auth mode!"));
        return -1;
    }
    memset(pbuf, 0, 512);
#if PLATFORM_IS_TV
    snprintf(cmd, sizeof(cmd), "/3rd/bin/wpa_supplicant/common/iwpriv %s connStatus",get_if_name());
#else
    snprintf(cmd, sizeof(cmd), "iwpriv %s connStatus",get_if_name());
#endif

    fp = popen(cmd, "r");
    if (!fp)
    {
        WIFI_LOG(WIFI_DEBUG, ("%s excute failed, can not get WEP auth mode!"));
        if (pbuf != NULL)
        {
            free(pbuf);
            pbuf = NULL;
        }
        return -1;
    }

    i = fread(pbuf, sizeof(char), 512, fp);
    WIFI_LOG(WIFI_DEBUG, ("read %d bytes, read results: %s", i, pbuf));

    if (fp != NULL)
    {
        pclose(fp);
        fp = NULL;
    }

    pEncrypType = strstr(pbuf, "EncrypType=WEP");
    if (pEncrypType != NULL)
    {
        pAuthMode = strstr(pbuf, "AuthMode=SHARED");
        if (pAuthMode != NULL)
        {
            *pWepAuthMode = IEEE_802_11_AUTH_MODE_SHARED;
        }
        else
        {
            pAuthMode = strstr(pbuf, "AuthMode=OPEN");
            if (pAuthMode != NULL)
            {
                *pWepAuthMode = IEEE_802_11_AUTH_MODE_OPEN;
            }
            else
            {
                WIFI_LOG(WIFI_DEBUG, ("can not find WEP auth mode!"));
                if (pbuf != NULL)
                {
                    free(pbuf);
                    pbuf = NULL;
                }
                return -1;
            }
        }
    }
    else
    {
        WIFI_LOG(WIFI_DEBUG, ("can not find WEP auth mode!"));
        if (pbuf != NULL)
        {
            free(pbuf);
            pbuf = NULL;
        }
        return -1;
    }

    WIFI_LOG(WIFI_DEBUG, ("Current WEP auth mode = %d", *pWepAuthMode));
    return 0;
}

INT32 c_net_wlan_wpa_set_region_ABand(int i4ABand)
{
    char _cmd_buf[256] = {0};
    int i4Ret = 0;

    x_memset(_cmd_buf, 0, sizeof(_cmd_buf));
#if PLATFORM_IS_TV
    snprintf(_cmd_buf,sizeof(_cmd_buf), "/3rd/bin/wpa_supplicant/common/iwpriv %s set CountryRegionABand=%d", get_if_name(),i4ABand);
#else
    snprintf(_cmd_buf,sizeof(_cmd_buf), "iwpriv %s set CountryRegionABand=%d",get_if_name(),i4ABand);
#endif
    i4Ret = doScriptCmd(_cmd_buf);
    if (i4Ret != 0)
    {
        WIFI_LOG (WIFI_DEBUG,(" \"iwpriv %s set CountryRegionABand=%d\" command fail.\n",get_if_name(), i4ABand));
        return NET_WLAN_ERROR;
    }

    return NET_WLAN_OK;
}


/*----------------------------------------------------------------------------------------**
** Set Country Region, ABand Region, and enable/disable channel list separate for channel control **
** if "pCountryRegion" is NULL, don't set Country Region, if "pABandRegion" is NULL, don't set ABand Region **
** if "pFgChListSepe" is NULL, don't set channel list seperate, *pFgChListSepe should be 0 or 1 **
** if "pPassiveScanABand" is NULL, don't set ABand Passive Scan, *pPassiveScanABand should be 0 or 1 **
**----------------------------------------------------------------------------------------**/
INT32 c_net_wlan_set_region_channel_seperate(char *iface, INT32 *pCountryRegion,
        INT32 *pABandRegion, INT32 *pFgChListSepe, INT32 *pPassiveScanABand)
{
    INT32 i4Ret = NET_WLAN_OK;
    WIFI_LOG_ENTRY(WIFI_DEBUG, (""));
    char cmdbuf[128] = {'\0'};

    if (strncmp(iface, "p2p0", 4) == 0)
    {
        if (pCountryRegion != NULL)
        {
            memset(cmdbuf, 0, sizeof(cmdbuf));
            snprintf(cmdbuf, sizeof(cmdbuf), "iwpriv p2p0 set p2pCountryRegion=%d", (int)*pCountryRegion);
            i4Ret = doScriptCmd(cmdbuf);
            if (i4Ret != 0)
            {
                WIFI_LOG(WIFI_DEBUG, ("command failed: %s.", cmdbuf));
                return NET_WLAN_ERROR;
            }
        }
        else
        {
            WIFI_LOG(WIFI_DEBUG, ("Country Region is NULL."));
        }

        if (pABandRegion != NULL)
        {
            memset(cmdbuf, 0, sizeof(cmdbuf));
            snprintf(cmdbuf, sizeof(cmdbuf), "iwpriv p2p0 set p2pCountryRegionABand=%d", (int)*pABandRegion);
            i4Ret = doScriptCmd(cmdbuf);
            if (i4Ret != 0)
            {
                WIFI_LOG(WIFI_DEBUG, ("command failed: %s.", cmdbuf));
                return NET_WLAN_ERROR;
            }
        }
        else
        {
            WIFI_LOG(WIFI_DEBUG, ("Country Region ABand is NULL."));
        }

        if (pFgChListSepe != NULL)
        {
            if ((*pFgChListSepe != 0) && (*pFgChListSepe != 1))
            {
                WIFI_LOG(WIFI_DEBUG, ("Invalid parameter %d, the flag of channel list seperate enable/disable should be 0 or 1.", (int)*pFgChListSepe));
                return NET_WLAN_ERROR;
            }
            else
            {
                memset(cmdbuf, 0, sizeof(cmdbuf));
                snprintf(cmdbuf, sizeof(cmdbuf), "iwpriv p2p0 set p2pChListSepaEnable=%d", (int)*pFgChListSepe);
                i4Ret = doScriptCmd(cmdbuf);
                if (i4Ret != 0)
                {
                    WIFI_LOG(WIFI_DEBUG, ("command failed: %s.", cmdbuf));
                    return NET_WLAN_ERROR;
                }
            }
        }
        else
        {
            WIFI_LOG(WIFI_DEBUG, ("Flag of Channel List Seperate enable/disable is NULL."));
        }
    }
    else if (strncmp(iface, "ra0", 3) == 0)
    {
        if ( pCountryRegion )
        {
            memset(cmdbuf, 0, sizeof(cmdbuf));
            snprintf(cmdbuf, sizeof(cmdbuf), "iwpriv ra0 set CountryRegion=%d", (int)*pCountryRegion);
            i4Ret = doScriptCmd(cmdbuf);
            if (i4Ret != 0)
            {
                WIFI_LOG (WIFI_DEBUG,(" \"%s\" command fail.\n", cmdbuf));
                return NET_WLAN_ERROR;
            }
        }
        else
        {
            WIFI_LOG(WIFI_DEBUG, ("Country Region is NULL."));
        }

        if ( pABandRegion )
        {
            memset(cmdbuf, 0, sizeof(cmdbuf));
            snprintf(cmdbuf, sizeof(cmdbuf), "iwpriv ra0 set CountryRegionABand=%d", (int)*pABandRegion);
            i4Ret = doScriptCmd(cmdbuf);
            if (i4Ret != 0)
            {
                WIFI_LOG (WIFI_DEBUG,(" \"%s\" command fail.\n", cmdbuf));
                return NET_WLAN_ERROR;
            }
        }
        else
        {
            WIFI_LOG(WIFI_DEBUG, ("ABand Country Region is NULL."));
        }

        if ( pPassiveScanABand )
        {
            if ((*pPassiveScanABand != 0) && (*pPassiveScanABand != 1))
            {
                WIFI_LOG(WIFI_DEBUG, ("Invalid parameter %d, the flag of ABand Passive Scan enable/disable should be 0 or 1.", (int)*pFgChListSepe));
                return NET_WLAN_ERROR;
            }
            else
            {
                memset(cmdbuf, 0, sizeof(cmdbuf));
                snprintf(cmdbuf, sizeof(cmdbuf), "iwpriv ra0 set PassiveScanABand=%d", (int)*pPassiveScanABand);
                i4Ret = doScriptCmd(cmdbuf);
                if (i4Ret != 0)
                {
                    WIFI_LOG (WIFI_DEBUG,(" \"%s\" command fail.\n", cmdbuf));
                    return NET_WLAN_ERROR;
                }
            }
        }
        else
        {
            WIFI_LOG(WIFI_DEBUG, ("ABand Passive Scan is NULL."));
        }
    }
    else
    {
        WIFI_LOG(WIFI_DEBUG, ("Invalid interface name."));
        return NET_WLAN_ERROR;
    }

    return NET_WLAN_OK;
}


/* Get STA bandwidth (ra0 and p2p0) */
INT32 c_net_wlan_get_STA_bandwidth(INT32 *pBW)
{
    INT32 i4Ret = NET_WLAN_OK;
    WIFI_LOG_ENTRY(WIFI_DEBUG, (""));

    i4Ret = _get_STA_bandwidth(pBW);
    if (i4Ret != 0 || (*pBW != 20 && *pBW != 40))
    {
        WIFI_LOG(WIFI_DEBUG, ("get bandwidth failed!"));
        return -1;
    }

    return 0;
}
INT32 c_net_wlan_get_eeprom(NET_802_11_EEPORM_T *pEeprom)
{
    INT32 i4Ret = NET_WLAN_OK;
    WIFI_LOG_ENTRY(WIFI_DEBUG, (""));

    //for Sony

    char cmd[128] = {"\0"};
    char data[16] = {"\0"};
    char reply[128] = {"\0"};
    FILE *fp = NULL;
    int i = 0;
    char *p = NULL;

    /* iwpriv ra0 e2p 110 */
#if PLATFORM_IS_TV
    snprintf(cmd, sizeof(cmd), "/3rd/bin/wpa_supplicant/common/iwpriv %s e2p %x",get_if_name(), (unsigned int)(pEeprom->u4Addr));
#else
    snprintf(cmd, sizeof(cmd), "iwpriv %s e2p %x", get_if_name(), (unsigned int)(pEeprom->u4Addr));
#endif
    fp = popen(cmd, "r");
    if (!fp)
    {
        WIFI_LOG(WIFI_DEBUG, ("read eeprom failed: %s", cmd));
        return -1;
    }

    i = fread(reply, sizeof(unsigned char), sizeof(reply), fp);
    reply[sizeof(reply) - 1] = '\0';
    WIFI_LOG(WIFI_INFO, ("read eeprom: %s", reply));

    if (fp != NULL)
    {
        pclose(fp);
        fp = NULL;
    }

    /* ra0  e2p:
    *   [0x0110]:0x030D
    */
    p = strstr(reply, "]:");
    if (!p)
    {
        WIFI_LOG(WIFI_DEBUG, ("read eeprom failed."));
        return -1;
    }

    p += strlen("]:");

    if (strncmp(p, "0x", 2) == 0)
    {
        p += strlen("0x");
    }

    for (i = 0; i < pEeprom->u4Len; i++)
    {
        memset(data, 0, sizeof(data));
        strncpy(data, p, 2);
        (pEeprom->pData)[i] = strtol(data, NULL, 16);
        p += 2;
    }

    return i4Ret;
}

INT32 c_net_wlan_get_rssi(NET_802_11_WIFI_RSSI_T *pRssi)
{
    WIFI_LOG_ENTRY(WIFI_API,(""));
    int i4Ret = x_net_wlan_get_rssi(pRssi);
    WIFI_CHK_FAIL (i4Ret);
    return i4Ret;

}


INT32 c_net_wlan_check_init_state(void)
{
    INT32 i4Ret = NET_WLAN_OK;
    WIFI_LOG_ENTRY(WIFI_DEBUG, (""));
    if (wlan_check_init_state() != NET_WLAN_OK)
        return NET_WLAN_NOT_INIT;

    if (!doBooleanCommand("PING", "PONG"))
    {
        printf("[WIFI MW]>>>>>>> %s,%d fail,\n",__FUNCTION__,__LINE__);
        return NET_WLAN_ERROR;
    }

    return i4Ret;
}

/*------------------------------------------------------------------*/
/*! @brief: select WLAN antenna
*  @param [in] iface - the pointer of interface, such as "ra0"
*  @param [in] option - the option of antenna selection,
*      0: Antenna Diversity Disable
*      1: Antenna Diversity Enable
*      2: Antenna CON1 2.4G, CON2 5G/BT
*      3: Antenna CON1 5G/BT, CON2 2.4G
*      4: Software Antenna Diversity Enable (Default)
*  @return     result
*  @retval     0 -success
*  @retval     otherwise -failure
*/
/*------------------------------------------------------------------*/
INT32 c_net_wlan_select_antenna(char *iface, int option)
{
    INT32 i4Ret = -1;
    WIFI_LOG_ENTRY(WIFI_DEBUG, (""));

    char cmd[128] = {'\0'};

    snprintf( cmd, sizeof(cmd), "/usr/local/bin/iwpriv %s set ant=%d", iface, option );
    i4Ret = doScriptCmd( cmd );
    if ( i4Ret != 0 )
    {
        WIFI_LOG(LogDefault, ("Failed to run cmd=%s.", cmd));
        return -1;
    }

    return 0;
}


/*------------------------------------------------------------------*/
/*! @brief: start or stop ATE
*  @param [in] iface - the pointer of interface, such as "ra0"
*  @param [in] flag - the flag to start or stop ATE, 0: stop, 1: start
*  @return     result
*  @retval     0 -success
*  @retval     otherwise -failure
*/
/*------------------------------------------------------------------*/
INT32 c_net_wlan_ate_control( char *iface, int flag )
{
    WIFI_LOG_ENTRY(LogDefault, (""));

    int ret = -1;
    char cmd[128] = {'\0'};

    if ( !iface && ( flag != 0 && flag != 1 ) )
    {
        WIFI_LOG(LogDefault, ("Invalid parameters, flag should be 0 or 1."));
        return -1;
    }

    //up interface if it is down
    memset(cmd, 0, sizeof(cmd));
    snprintf( cmd, sizeof(cmd), "ifconfig | grep %s", iface );
    ret = doScriptCmd( cmd );
    if ( ret != 0 )
    {
        memset(cmd, 0, sizeof(cmd));
        snprintf( cmd, sizeof(cmd), "ifconfig %s up", iface );
        ret = doScriptCmd( cmd );
        if ( ret != 0 )
        {
            WIFI_LOG(LogDefault, ("Failed to run cmd=%s.", cmd));
        }
    }

    memset(cmd, 0, sizeof(cmd));
    if ( flag == 0 )
    {
        snprintf( cmd, sizeof(cmd), "/usr/local/bin/iwpriv %s set ATESTOP", iface );
    }
    else if ( flag == 1 )
    {
        snprintf( cmd, sizeof(cmd), "/usr/local/bin/iwpriv %s set ATE=ATESTART", iface );
    }
    else
    {
        WIFI_LOG(LogDefault, ("Invalid parameters, flag should be 0 or 1."));
        return -1;
    }

    ret = doScriptCmd( cmd );
    if ( ret != 0 )
    {
        WIFI_LOG(LogDefault, ("Failed to run cmd=%s.", cmd));
        return -1;
    }

    return 0;
}



/*------------------------------------------------------------------*/
/*! @brief: Set parameters for TX
*  @param [in] iface - the pointer of interface, such as "ra0"
*  @param [in] parame - the pointer of parameter structure
*  @return     result
*  @retval     0 -success
*  @retval     otherwise -failure
*/
/*------------------------------------------------------------------*/
INT32 c_net_wlan_ate_start_Tx( char *iface, NET_802_11_ATE_PARAMETER_T *param)
{
    WIFI_LOG_ENTRY(LogDefault, (""));

    int ret = -1;
    char cmd[128] = {'\0'};
    int TXPOW = 31;
    int Freqoffset = 64;

    if ( !iface || !param )
    {
        WIFI_LOG(LogDefault, ("Invalid parameters."));
        return -1;
    }

    if (param->ModulationFlag == 0)
    {
        TXPOW = 10;
    }

    ret = x_net_get_wlan_freq_offset(0xf4, 0xf5, 0xf6, &Freqoffset);
    if ( ret != NET_WLAN_OK )
        WIFI_LOG(LogDefault, ("Failed to get wlan freqoffset."));

    if ( param->ModulationFlag == 1 )
    {
        memset(cmd, 0, sizeof(cmd));
        snprintf( cmd, sizeof(cmd), "/usr/local/bin/iwpriv %s set ATEDA=%s", iface, "00:11:22:33:44:55" );
        ret = doScriptCmd( cmd );
        if ( ret != 0 )
        {
            WIFI_LOG(LogDefault, ("Failed to run cmd=%s.", cmd));
            return -1;
        }

        memset(cmd, 0, sizeof(cmd));
        snprintf( cmd, sizeof(cmd), "/usr/local/bin/iwpriv %s set ATESA=%s", iface, "00:aa:bb:cc:dd:ee" );
        ret = doScriptCmd( cmd );
        if ( ret != 0 )
        {
            WIFI_LOG(LogDefault, ("Failed to run cmd=%s.", cmd));
            return -1;
        }

        memset(cmd, 0, sizeof(cmd));
        snprintf( cmd, sizeof(cmd), "/usr/local/bin/iwpriv %s set ATEBSSID=%s", iface, "00:11:22:33:44:55" );
        ret = doScriptCmd( cmd );
        if ( ret != 0 )
        {
            WIFI_LOG(LogDefault, ("Failed to run cmd=%s.", cmd));
            return -1;
        }
    }

    memset(cmd, 0, sizeof(cmd));
    snprintf( cmd, sizeof(cmd), "/usr/local/bin/iwpriv %s set ATECHANNEL=%d", iface, param->channelNum );
    ret = doScriptCmd( cmd );
    if ( ret != 0 )
    {
        WIFI_LOG(LogDefault, ("Failed to run cmd=%s.", cmd));
        return -1;
    }

    memset(cmd, 0, sizeof(cmd));
    snprintf( cmd, sizeof(cmd), "/usr/local/bin/iwpriv %s set ATETXMODE=%d", iface, param->WiFiStandard );
    ret = doScriptCmd( cmd );
    if ( ret != 0 )
    {
        WIFI_LOG(LogDefault, ("Failed to run cmd=%s.", cmd));
        return -1;
    }

    memset(cmd, 0, sizeof(cmd));
    snprintf( cmd, sizeof(cmd), "/usr/local/bin/iwpriv %s set ATETXMCS=%d", iface, param->PHYRate);
    ret = doScriptCmd( cmd );
    if ( ret != 0 )
    {
        WIFI_LOG(LogDefault, ("Failed to run cmd=%s.", cmd));
        return -1;
    }

    memset(cmd, 0, sizeof(cmd));
    snprintf( cmd, sizeof(cmd), "/usr/local/bin/iwpriv %s set ATETXBW=%d", iface, 0);
    ret = doScriptCmd( cmd );
    if ( ret != 0 )
    {
        WIFI_LOG(LogDefault, ("Failed to run cmd=%s.", cmd));
        return -1;
    }

    if ( param->ModulationFlag == 1 )
    {
        memset(cmd, 0, sizeof(cmd));
        snprintf( cmd, sizeof(cmd), "/usr/local/bin/iwpriv %s set ATETXGI=%d", iface, 0);
        ret = doScriptCmd( cmd );
        if ( ret != 0 )
        {
            WIFI_LOG(LogDefault, ("Failed to run cmd=%s.", cmd));
            return -1;
        }
    }

    memset(cmd, 0, sizeof(cmd));
    snprintf( cmd, sizeof(cmd), "/usr/local/bin/iwpriv %s set ATETXLEN=%d", iface, 1024);
    ret = doScriptCmd( cmd );
    if ( ret != 0 )
    {
        WIFI_LOG(LogDefault, ("Failed to run cmd=%s.", cmd));
        return -1;
    }

    memset(cmd, 0, sizeof(cmd));
    snprintf( cmd, sizeof(cmd), "/usr/local/bin/iwpriv %s set ATETXANT=%d", iface, param->antNum);
    ret = doScriptCmd( cmd );
    if ( ret != 0 )
    {
        WIFI_LOG(LogDefault, ("Failed to run cmd=%s.", cmd));
        return -1;
    }

    memset(cmd, 0, sizeof(cmd));
    snprintf( cmd, sizeof(cmd), "/usr/local/bin/iwpriv %s set ATETXPOW0=%d", iface, TXPOW);
    ret = doScriptCmd( cmd );
    if ( ret != 0 )
    {
        WIFI_LOG(LogDefault, ("Failed to run cmd=%s.", cmd));
        return -1;
    }

    memset(cmd, 0, sizeof(cmd));
    snprintf( cmd, sizeof(cmd), "/usr/local/bin/iwpriv %s set ATETXPOW1=%d", iface, TXPOW);
    ret = doScriptCmd( cmd );
    if ( ret != 0 )
    {
        WIFI_LOG(LogDefault, ("Failed to run cmd=%s.", cmd));
        return -1;
    }

    memset(cmd, 0, sizeof(cmd));
    snprintf( cmd, sizeof(cmd), "/usr/local/bin/iwpriv %s set ATETXCNT=%d", iface, 0);
    ret = doScriptCmd( cmd );
    if ( ret != 0 )
    {
        WIFI_LOG(LogDefault, ("Failed to run cmd=%s.", cmd));
        return -1;
    }

    if ( param->ModulationFlag == 1 )
    {
        memset(cmd, 0, sizeof(cmd));
        snprintf( cmd, sizeof(cmd), "/usr/local/bin/iwpriv %s set ATEAUTOALC=%d", iface, 1);
        ret = doScriptCmd( cmd );
        if ( ret != 0 )
        {
            WIFI_LOG(LogDefault, ("Failed to run cmd=%s.", cmd));
            return -1;
        }
    }

    memset(cmd, 0, sizeof(cmd));
    snprintf( cmd, sizeof(cmd), "/usr/local/bin/iwpriv %s set ATETXFREQOFFSET=%d", iface, Freqoffset);
    ret = doScriptCmd( cmd );
    if ( ret != 0 )
    {
        WIFI_LOG(LogDefault, ("Failed to run cmd=%s.", cmd));
        return -1;
    }

    memset(cmd, 0, sizeof(cmd));
    snprintf( cmd, sizeof(cmd), "/usr/local/bin/iwpriv %s set ATE=TXFRAME", iface);
    ret = doScriptCmd( cmd );
    if ( ret != 0 )
    {
        WIFI_LOG(LogDefault, ("Failed to run cmd=%s.", cmd));
        return -1;
    }

    if ( param->ModulationFlag == 0 )
    {
        memset(cmd, 0, sizeof(cmd));
        snprintf( cmd, sizeof(cmd), "/usr/local/bin/iwpriv %s set ATE=TXCARR", iface);
        ret = doScriptCmd( cmd );
        if ( ret != 0 )
        {
            WIFI_LOG(LogDefault, ("Failed to run cmd=%s.", cmd));
            return -1;
        }
    }

    return 0;
}


/*------------------------------------------------------------------*/
/*! @brief: Set parameters for RX
*  @param [in] iface - the pointer of interface, such as "ra0"
*  @param [in] parame - the pointer of parameter structure
*  @return     result
*  @retval     0 -success
*  @retval     otherwise -failure
*/
/*------------------------------------------------------------------*/
INT32 c_net_wlan_ate_start_Rx( char *iface, NET_802_11_ATE_PARAMETER_T *param)
{
    WIFI_LOG_ENTRY(LogDefault, (""));

    int ret = -1;
    char cmd[128] = {'\0'};
    int Freqoffset = 64;

    if ( !iface || !param )
    {
        WIFI_LOG(LogDefault, ("Invalid parameters."));
        return -1;
    }

    ret = x_net_get_wlan_freq_offset(0xf4, 0xf5, 0xf6, &Freqoffset);
    if ( ret != NET_WLAN_OK )
        WIFI_LOG(LogDefault, ("Failed to get wlan freqoffset."));

    memset(cmd, 0, sizeof(cmd));
    snprintf( cmd, sizeof(cmd), "/usr/local/bin/iwpriv %s set ATECHANNEL=%d", iface, param->channelNum );
    ret = doScriptCmd( cmd );
    if ( ret != 0 )
    {
        WIFI_LOG(LogDefault, ("Failed to run cmd=%s.", cmd));
        return -1;
    }

    memset(cmd, 0, sizeof(cmd));
    snprintf( cmd, sizeof(cmd), "/usr/local/bin/iwpriv %s set ResetCounter=%d", iface, 0 );
    ret = doScriptCmd( cmd );
    if ( ret != 0 )
    {
        WIFI_LOG(LogDefault, ("Failed to run cmd=%s.", cmd));
        return -1;
    }

    memset(cmd, 0, sizeof(cmd));
    snprintf( cmd, sizeof(cmd), "/usr/local/bin/iwpriv %s set ATETXFREQOFFSET=%d", iface, Freqoffset);
    ret = doScriptCmd( cmd );
    if ( ret != 0 )
    {
        WIFI_LOG(LogDefault, ("Failed to run cmd=%s.", cmd));
        return -1;
    }

    memset(cmd, 0, sizeof(cmd));
    snprintf( cmd, sizeof(cmd), "/usr/local/bin/iwpriv %s set ATETXMODE=%d", iface, param->WiFiStandard );
    ret = doScriptCmd( cmd );
    if ( ret != 0 )
    {
        WIFI_LOG(LogDefault, ("Failed to run cmd=%s.", cmd));
        return -1;
    }

    memset(cmd, 0, sizeof(cmd));
    snprintf( cmd, sizeof(cmd), "/usr/local/bin/iwpriv %s set ATETXMCS=%d", iface, param->PHYRate);
    ret = doScriptCmd( cmd );
    if ( ret != 0 )
    {
        WIFI_LOG(LogDefault, ("Failed to run cmd=%s.", cmd));
        return -1;
    }

    memset(cmd, 0, sizeof(cmd));
    snprintf( cmd, sizeof(cmd), "/usr/local/bin/iwpriv %s set ATETXBW=%d", iface, 0);
    ret = doScriptCmd( cmd );
    if ( ret != 0 )
    {
        WIFI_LOG(LogDefault, ("Failed to run cmd=%s.", cmd));
        return -1;
    }

    memset(cmd, 0, sizeof(cmd));
    snprintf( cmd, sizeof(cmd), "/usr/local/bin/iwpriv %s set ATERXANT=%d", iface, param->antNum);
    ret = doScriptCmd( cmd );
    if ( ret != 0 )
    {
        WIFI_LOG(LogDefault, ("Failed to run cmd=%s.", cmd));
        return -1;
    }

    memset(cmd, 0, sizeof(cmd));
    snprintf( cmd, sizeof(cmd), "/usr/local/bin/iwpriv %s set ATE=RXFRAME", iface);
    ret = doScriptCmd( cmd );
    if ( ret != 0 )
    {
        WIFI_LOG(LogDefault, ("Failed to run cmd=%s.", cmd));
        return -1;
    }

    return 0;
}


/*------------------------------------------------------------------*/
/*! @brief: Get statistics data of RX
*  @return     result
*  @retval     0 -success
*  @retval     otherwise -failure
*/
/*------------------------------------------------------------------*/
INT32 c_net_wlan_ate_get_Rx_state(char *iface)
{
    WIFI_LOG_ENTRY(LogDefault, (""));

    int ret = -1;
    char cmd[128] = {'\0'};

    memset(cmd, 0, sizeof(cmd));
    snprintf( cmd, sizeof(cmd), "/usr/local/bin/iwpriv %s stat", iface);
    ret = doScriptCmd( cmd );
    if ( ret != 0 )
    {
        WIFI_LOG(LogDefault, ("Failed to run cmd=%s.", cmd));
        return -1;
    }

    return 0;
}


/*------------------------------------------------------------------*/
/*! @brief: map PHY rate to MCS
*  @li@c  WiFiStandard -the 802.11 standard.
                           0: CCK 802.11b, 1: OFDM 802.11g/802.11a, 2: Green Field 802.11n, 3: HT_MIX 802.11b/g/n 
*  @param [in] PHYrate - the PHY rate
*  @param [in] mcs - the MCS
*  @return     result
*  @retval     0 -success
*  @retval     otherwise -failure
*/
/*------------------------------------------------------------------*/
INT32 _map_PHYrate_to_mcs(UINT8 WiFiStandard, UINT8 PHYrate, int *mcs)
{
    WIFI_LOG_ENTRY(WIFI_API, (""));
    UINT8 _mcs = 0;

    if ( !mcs )
    {
        WIFI_LOG(WIFI_ERROR, ("!!Invalid parameters."));
        return -1;
    }

    *mcs = -1;

    switch( WiFiStandard )
    {
        case 0: //802.11b
            switch(PHYrate)
            {
                case 1: // 1Mbps
                    _mcs = 0;
                    break;
                case 2: // 2Mbps
                    _mcs = 1;
                    break;
                case 5: // 5.5Mbps
                    _mcs = 2;
                    break;
                case 11: // 11Mbps
                    _mcs = 3;
                    break;
                default:
                    WIFI_LOG(WIFI_ERROR, ("!!Invalid PHY rate."));
                    return -1;
            }
            break;
            
        case 1: //802.11g / 802.11a
            switch(PHYrate)
            {
                case 6: // 6Mbps
                    _mcs = 0;
                    break;
                case 9: // 9Mbps
                    _mcs = 1;
                    break;
                case 12: // 12Mbps
                    _mcs = 2;
                    break;
                case 18: // 18Mbps
                    _mcs = 3;
                    break;
                case 24: // 24Mbps
                    _mcs = 4;
                    break;
                case 36: // 36Mbps
                    _mcs = 5;
                    break;
                case 48: // 48Mbps
                    _mcs = 6;
                    break;
                case 54: // 54Mbps
                    _mcs = 7;
                    break;
                default:
                    WIFI_LOG(WIFI_ERROR, ("!!Invalid PHY rate."));
                    return -1;
            }
            break;

        case 2: //802.11n
            if ( PHYrate <= 15 )
            {
                _mcs = PHYrate;
            }
            else
            {
                WIFI_LOG(WIFI_ERROR, ("!!Invalid PHY rate."));
                return -1;
            }
            break;
            
        default:
            WIFI_LOG(WIFI_ERROR, ("!!Not support 802.11 standard."));
            return -1;
    }

    *mcs = _mcs;

    return 0;
}

INT32 _wlan_ate_start(char *iface)
{
    WIFI_LOG_ENTRY(WIFI_API, (""));
    int ret = -1;
    char cmd[128] = {'\0'};

    if ( !iface )
    {
        WIFI_LOG(WIFI_ERROR, ("Invalid interface name."));
        return -1;
    }

    snprintf( cmd, sizeof(cmd), "/usr/local/bin/iwpriv %s set ATE=ATESTART", iface );
    ret = doScriptCmd( cmd );
    if ( ret != 0 )
    {
        WIFI_LOG(WIFI_ERROR, ("Failed to run cmd=%s.", cmd));
        return -1;
    }

    return 0;
}

INT32 c_net_wlan_RF_initialize(char *iface)
{
    WIFI_LOG_ENTRY(WIFI_API, (""));

    int ret = -1;
    char cmd[128] = {'\0'};

    if ( !iface )
    {
        WIFI_LOG(WIFI_ERROR, ("Invalid interface name."));
        return -1;
    }

    //up interface if it is down
    memset(cmd, 0, sizeof(cmd));
    snprintf( cmd, sizeof(cmd), "ifconfig | grep %s", iface );
    ret = doScriptCmd( cmd );
    if ( ret != 0 )
    {
        memset(cmd, 0, sizeof(cmd));
        snprintf( cmd, sizeof(cmd), "ifconfig %s up", iface );
        ret = doScriptCmd( cmd );
        if ( ret != 0 )
        {
            WIFI_LOG(WIFI_ERROR, ("Failed to run cmd=%s.", cmd));
            return -1;
        }
    }

    ret = _wlan_ate_start(iface);
    if ( ret != 0 )
    {
        WIFI_LOG(WIFI_ERROR, ("!!Failed to initialize WiFi RF test mode."));
        return -1;
    }

    return 0;
}

/*------------------------------------------------------------------*/
/*! @brief: Set parameters for TX
*  @param [in] iface - the pointer of interface, such as "ra0"
*  @param [in] parame - the pointer of parameter structure
*  @return     result
*  @retval     0 -success
*  @retval     otherwise -failure
*/
/*------------------------------------------------------------------*/
INT32 c_net_wlan_RF_start_Tx( char *iface, NET_802_11_ATE_PARAMETER_T *param)
{
    WIFI_LOG_ENTRY(WIFI_API, (""));

    WIFI_LOG(WIFI_ERROR, ("antNum=%d, WiFiStandard=%d, channelNum=%d, "
        "PHYrate=%d, ModulationFlag=%d, BandWidth=%d", 
        (int)(param->antNum), (int)(param->WiFiStandard), 
        (int)(param->channelNum), (int)(param->PHYRate),
        (int)(param->ModulationFlag), (int)(param->BandWidth)));

    int ret = -1;
    char cmd[128] = {'\0'};
    int TXPOW = 31;
    int Freqoffset = 64;

    int MCS = -1;

    if ( !iface || !param )
    {
        WIFI_LOG(WIFI_ERROR, ("Invalid parameters."));
        return -1;
    }

    ret = _wlan_ate_start(iface);
    if ( ret != 0 )
    {
        WIFI_LOG(WIFI_ERROR, ("Failed to start Tx."));
        return -1;
    }

    memset(cmd, 0, sizeof(cmd));
    snprintf( cmd, sizeof(cmd), "/usr/local/bin/iwpriv %s set ATESINGLESKU=%d", iface, 1 );
    ret = doScriptCmd( cmd );
    if ( ret != 0 )
    {
        WIFI_LOG(WIFI_ERROR, ("Failed to run cmd=%s.", cmd));
        return -1;
    }

    memset(cmd, 0, sizeof(cmd));
    snprintf( cmd, sizeof(cmd), "/usr/local/bin/iwpriv %s set ATETXMODE=%d", iface, param->WiFiStandard );
    ret = doScriptCmd( cmd );
    if ( ret != 0 )
    {
        WIFI_LOG(WIFI_ERROR, ("Failed to run cmd=%s.", cmd));
        return -1;
    }

    _map_PHYrate_to_mcs(param->WiFiStandard, param->PHYRate, &MCS);
    if ( MCS < 0 || MCS > 15 )
    {
        WIFI_LOG(WIFI_ERROR, ("!!The PHY rate is not proper, so return."));
        return -1;
    }
    memset(cmd, 0, sizeof(cmd));
    snprintf( cmd, sizeof(cmd), "/usr/local/bin/iwpriv %s set ATETXMCS=%d", iface, MCS);
    ret = doScriptCmd( cmd );
    if ( ret != 0 )
    {
        WIFI_LOG(WIFI_ERROR, ("Failed to run cmd=%s.", cmd));
        return -1;
    }

    memset(cmd, 0, sizeof(cmd));
    snprintf( cmd, sizeof(cmd), "/usr/local/bin/iwpriv %s set ATETXBW=%d", iface, param->BandWidth);
    ret = doScriptCmd( cmd );
    if ( ret != 0 )
    {
        WIFI_LOG(WIFI_ERROR, ("Failed to run cmd=%s.", cmd));
        return -1;
    }

    memset(cmd, 0, sizeof(cmd));
    snprintf( cmd, sizeof(cmd), "/usr/local/bin/iwpriv %s set ATETXGI=%d", iface, 0);
    ret = doScriptCmd( cmd );
    if ( ret != 0 )
    {
        WIFI_LOG(WIFI_ERROR, ("Failed to run cmd=%s.", cmd));
        return -1;
    }

    int txlen = 1058;
    memset(cmd, 0, sizeof(cmd));
    snprintf( cmd, sizeof(cmd), "/usr/local/bin/iwpriv %s set ATETXLEN=%d", iface, txlen);
    ret = doScriptCmd( cmd );
    if ( ret != 0 )
    {
        WIFI_LOG(WIFI_ERROR, ("Failed to run cmd=%s.", cmd));
        return -1;
    }

    memset(cmd, 0, sizeof(cmd));
    snprintf( cmd, sizeof(cmd), "/usr/local/bin/iwpriv %s set ATETXANT=%d", iface, param->antNum);
    ret = doScriptCmd( cmd );
    if ( ret != 0 )
    {
        WIFI_LOG(WIFI_ERROR, ("Failed to run cmd=%s.", cmd));
        return -1;
    }

    memset(cmd, 0, sizeof(cmd));
    snprintf( cmd, sizeof(cmd), "/usr/local/bin/iwpriv %s set ATETXPOW0=%d", iface, TXPOW);
    ret = doScriptCmd( cmd );
    if ( ret != 0 )
    {
        WIFI_LOG(WIFI_ERROR, ("Failed to run cmd=%s.", cmd));
        return -1;
    }

    memset(cmd, 0, sizeof(cmd));
    snprintf( cmd, sizeof(cmd), "/usr/local/bin/iwpriv %s set ATETXPOW1=%d", iface, TXPOW);
    ret = doScriptCmd( cmd );
    if ( ret != 0 )
    {
        WIFI_LOG(WIFI_ERROR, ("Failed to run cmd=%s.", cmd));
        return -1;
    }

    memset(cmd, 0, sizeof(cmd));
    snprintf( cmd, sizeof(cmd), "/usr/local/bin/iwpriv %s set ATETXCNT=%d", iface, 0);
    ret = doScriptCmd( cmd );
    if ( ret != 0 )
    {
        WIFI_LOG(WIFI_ERROR, ("Failed to run cmd=%s.", cmd));
        return -1;
    }

    memset(cmd, 0, sizeof(cmd));
    snprintf( cmd, sizeof(cmd), "/usr/local/bin/iwpriv %s set ATEAUTOALC=%d", iface, 1);
    ret = doScriptCmd( cmd );
    if ( ret != 0 )
    {
        WIFI_LOG(WIFI_ERROR, ("Failed to run cmd=%s.", cmd));
        return -1;
    }

    memset(cmd, 0, sizeof(cmd));
    snprintf( cmd, sizeof(cmd), "/usr/local/bin/iwpriv %s set ATETXPOWEVAL=%d", iface, 1);
    ret = doScriptCmd( cmd );
    if ( ret != 0 )
    {
        WIFI_LOG(WIFI_ERROR, ("Failed to run cmd=%s.", cmd));
        return -1;
    }

    ret = x_net_wlan_get_mt7662_freq_offset(&Freqoffset);
    if ( ret != NET_WLAN_OK )
        WIFI_LOG(WIFI_ERROR, ("Failed to get wlan freqoffset."));

    memset(cmd, 0, sizeof(cmd));
    snprintf( cmd, sizeof(cmd), "/usr/local/bin/iwpriv %s set ATETXFREQOFFSET=%d", iface, Freqoffset);
    ret = doScriptCmd( cmd );
    if ( ret != 0 )
    {
        WIFI_LOG(WIFI_ERROR, ("Failed to run cmd=%s.", cmd));
        return -1;
    }

    memset(cmd, 0, sizeof(cmd));
    snprintf( cmd, sizeof(cmd), "/usr/local/bin/iwpriv %s set ATECHANNEL=%d", iface, param->channelNum );
    ret = doScriptCmd( cmd );
    if ( ret != 0 )
    {
        WIFI_LOG(WIFI_ERROR, ("Failed to run cmd=%s.", cmd));
        return -1;
    }

    memset(cmd, 0, sizeof(cmd));
    if ( param->ModulationFlag == 1 ) {
        snprintf( cmd, sizeof(cmd), "/usr/local/bin/iwpriv %s set ATE=TXFRAME", iface);
    } else if ( param->ModulationFlag == 0 ) {
        snprintf( cmd, sizeof(cmd), "/usr/local/bin/iwpriv %s set ATE=TXCARR", iface);
    }
    ret = doScriptCmd( cmd );
    if ( ret != 0 )
    {
        WIFI_LOG(WIFI_ERROR, ("Failed to run cmd=%s.", cmd));
        return -1;
    }

    memset(cmd, 0, sizeof(cmd));
    snprintf( cmd, sizeof(cmd), "/usr/local/bin/iwpriv %s mac 1340=04001b3f", iface);
    ret = doScriptCmd( cmd );
    if ( ret != 0 )
    {
        WIFI_LOG(WIFI_ERROR, ("Failed to run cmd=%s.", cmd));
        return -1;
    }

    return 0;
}


/*------------------------------------------------------------------*/
/*! @brief: Stop Tx
*  @param [in] iface - the pointer of interface, such as "ra0"
*  @return     result
*  @retval     0 -success
*  @retval     otherwise -failure
*/
/*------------------------------------------------------------------*/
INT32 c_net_wlan_RF_stop_Tx(char *iface)
{
    WIFI_LOG_ENTRY(WIFI_API, (""));

    int ret = -1;

    if ( !iface )
    {
        WIFI_LOG(WIFI_ERROR, ("!! Invalid interface name."));
        return -1;
    }

    ret = _wlan_ate_start(iface);
    if ( ret != 0 )
    {
        WIFI_LOG(WIFI_ERROR, ("Failed to stop Tx."));
        return -1;
    }

    return 0;
}


/*------------------------------------------------------------------*/
/*! @brief: Set parameters for Rx
*  @param [in] iface - the pointer of interface, such as "ra0"
*  @param [in] parame - the pointer of parameter structure
*  @return     result
*  @retval     0 -success
*  @retval     otherwise -failure
*/
/*------------------------------------------------------------------*/
INT32 c_net_wlan_RF_start_Rx( char *iface, NET_802_11_ATE_PARAMETER_T *param)
{
    WIFI_LOG_ENTRY(WIFI_API, (""));

    WIFI_LOG(WIFI_ERROR, ("antNum=%d, WiFiStandard=%d, channelNum=%d, "
        "PHYrate=%d, ModulationFlag=%d, BandWidth=%d", 
        (int)(param->antNum), (int)(param->WiFiStandard), 
        (int)(param->channelNum), (int)(param->PHYRate),
        (int)(param->ModulationFlag), (int)(param->BandWidth)));

    int ret = -1;
    char cmd[128] = {'\0'};
    int Freqoffset = 64;

    if ( !iface || !param )
    {
        WIFI_LOG(WIFI_ERROR, ("!! Invalid parameters."));
        return -1;
    }

    ret = _wlan_ate_start(iface);
    if ( ret != 0 )
    {
        WIFI_LOG(WIFI_ERROR, ("Failed to start Rx."));
        return -1;
    }

    memset(cmd, 0, sizeof(cmd));
    snprintf( cmd, sizeof(cmd), "/usr/local/bin/iwpriv %s set ATESINGLESKU=%d", iface, 1 );
    ret = doScriptCmd( cmd );
    if ( ret != 0 )
    {
        WIFI_LOG(WIFI_ERROR, ("Failed to run cmd=%s.", cmd));
        return -1;
    }

    memset(cmd, 0, sizeof(cmd));
    snprintf( cmd, sizeof(cmd), "/usr/local/bin/iwpriv %s set ResetCounter=%d", iface, 0 );
    ret = doScriptCmd( cmd );
    if ( ret != 0 )
    {
        WIFI_LOG(WIFI_ERROR, ("Failed to run cmd=%s.", cmd));
        return -1;
    }

    ret = x_net_wlan_get_mt7662_freq_offset(&Freqoffset);
    if ( ret != NET_WLAN_OK )
        WIFI_LOG(WIFI_ERROR, ("Failed to get wlan freqoffset."));

    memset(cmd, 0, sizeof(cmd));
    snprintf( cmd, sizeof(cmd), "/usr/local/bin/iwpriv %s set ATETXFREQOFFSET=%d", iface, Freqoffset);
    ret = doScriptCmd( cmd );
    if ( ret != 0 )
    {
        WIFI_LOG(WIFI_ERROR, ("Failed to run cmd=%s.", cmd));
        return -1;
    }

    memset(cmd, 0, sizeof(cmd));
    snprintf( cmd, sizeof(cmd), "/usr/local/bin/iwpriv %s set ATETXMODE=%d", iface, param->WiFiStandard );
    ret = doScriptCmd( cmd );
    if ( ret != 0 )
    {
        WIFI_LOG(WIFI_ERROR, ("Failed to run cmd=%s.", cmd));
        return -1;
    }

    memset(cmd, 0, sizeof(cmd));
    snprintf( cmd, sizeof(cmd), "/usr/local/bin/iwpriv %s set ATETXMCS=%d", iface, param->PHYRate);
    ret = doScriptCmd( cmd );
    if ( ret != 0 )
    {
        WIFI_LOG(WIFI_ERROR, ("Failed to run cmd=%s.", cmd));
        return -1;
    }

    memset(cmd, 0, sizeof(cmd));
    snprintf( cmd, sizeof(cmd), "/usr/local/bin/iwpriv %s set ATETXBW=%d", iface, param->BandWidth);
    ret = doScriptCmd( cmd );
    if ( ret != 0 )
    {
        WIFI_LOG(WIFI_ERROR, ("Failed to run cmd=%s.", cmd));
        return -1;
    }

    memset(cmd, 0, sizeof(cmd));
    snprintf( cmd, sizeof(cmd), "/usr/local/bin/iwpriv %s set ATERXANT=%d", iface, param->antNum);
    ret = doScriptCmd( cmd );
    if ( ret != 0 )
    {
        WIFI_LOG(WIFI_ERROR, ("Failed to run cmd=%s.", cmd));
        return -1;
    }

    memset(cmd, 0, sizeof(cmd));
    snprintf( cmd, sizeof(cmd), "/usr/local/bin/iwpriv %s set ATECHANNEL=%d", iface, param->channelNum );
    ret = doScriptCmd( cmd );
    if ( ret != 0 )
    {
        WIFI_LOG(WIFI_ERROR, ("Failed to run cmd=%s.", cmd));
        return -1;
    }

    memset(cmd, 0, sizeof(cmd));
    snprintf( cmd, sizeof(cmd), "/usr/local/bin/iwpriv %s set ATE=RXFRAME", iface);
    ret = doScriptCmd( cmd );
    if ( ret != 0 )
    {
        WIFI_LOG(WIFI_ERROR, ("Failed to run cmd=%s.", cmd));
        return -1;
    }

    return 0;
}


/*------------------------------------------------------------------*/
/*! @brief: Stop Rx and get statistics data of Rx
*  @param [in] iface - the pointer of interface, such as "ra0"
*  @return     result
*  @retval     0 -success
*  @retval     otherwise -failure
*/
/*------------------------------------------------------------------*/
INT32 c_net_wlan_RF_stop_Rx(char *iface)
{
    WIFI_LOG_ENTRY(WIFI_API, (""));

    int ret = -1;
    char cmd[128] = {'\0'};

    if ( !iface )
    {
        WIFI_LOG(WIFI_ERROR, ("!! Invalid parameters."));
        return -1;
    }

    snprintf( cmd, sizeof(cmd), "/usr/local/bin/iwpriv %s stat", iface);
    ret = doScriptCmd( cmd );
    if ( ret != 0 )
    {
        WIFI_LOG(WIFI_ERROR, ("Failed to run cmd=%s.", cmd));
        return -1;
    }

    ret = _wlan_ate_start(iface);
    if ( ret != 0 )
    {
        WIFI_LOG(WIFI_ERROR, ("Failed to start Rx."));
        return -1;
    }

    return 0;
}
/** ======================== TDLS Functions ======================== **/

/*------------------------------------------------------------------*/
/*! @brief: Send TDLS discovery request.
*  @param [in] iface - the mac address of TDLS entry which want to discovery
*  @return     result
*  @retval     0 -success
*  @retval     otherwise -failure 
*/
/*------------------------------------------------------------------*/
INT32 c_net_wlan_tdls_discovery(WLAN_TDLS_Device_Identifier_t device)
{
    WIFI_LOG_ENTRY(WIFI_API, (""));

    char cmdStr[128] = "\0";
    char reply[8]= "OK";

    if (wlan_check_init_state() != NET_WLAN_OK)
    {
        return NET_WLAN_NOT_INIT;
    }

    snprintf(cmdStr, sizeof(cmdStr), "TDLS_DISCOVER " MACSTR, 
        MAC2STR(device.macAddr));

    if (!doBooleanCommand(cmdStr, reply))
    {
        WIFI_LOG(WIFI_ERROR, ("!!Failed to send TDLS discovery."));
        return NET_WLAN_ERROR;
    }

    WIFI_LOG(WIFI_ERROR, ("Successed to send TDLS discovery."));

    return NET_WLAN_OK;    
}


/*------------------------------------------------------------------*/
/*! @brief: Send TDLS setup request.
*  @param [in] iface - the mac address of TDLS entry which want to setup
*  @return     result
*  @retval     0 -success
*  @retval     otherwise -failure 
*/
/*------------------------------------------------------------------*/
INT32 c_net_wlan_tdls_setup(WLAN_TDLS_Device_Identifier_t device)
{
    WIFI_LOG_ENTRY(WIFI_API, (""));

    char cmdStr[128] = "\0";
    char reply[8]= "OK";

    if (wlan_check_init_state() != NET_WLAN_OK)
    {
        return NET_WLAN_NOT_INIT;
    }

    snprintf(cmdStr, sizeof(cmdStr), "TDLS_SETUP " MACSTR, 
        MAC2STR(device.macAddr));

    if (!doBooleanCommand(cmdStr, reply))
    {
        WIFI_LOG(WIFI_ERROR, ("!!Failed to send TDLS setup."));
        return NET_WLAN_ERROR;
    }

    WIFI_LOG(WIFI_ERROR, ("Successed to send TDLS setup."));

    return NET_WLAN_OK;    
}

/*------------------------------------------------------------------*/
/*! @brief: Send TDLS teardown request.
*  @param [in] iface - the mac address of TDLS entry which want to teardown
*  @return     result
*  @retval     0 -success
*  @retval     otherwise -failure 
*/
/*------------------------------------------------------------------*/
INT32 c_net_wlan_tdls_teardown(WLAN_TDLS_Device_Identifier_t device)
{
    WIFI_LOG_ENTRY(WIFI_API, (""));

    char cmdStr[128] = "\0";
    char reply[8]= "OK";

    if (wlan_check_init_state() != NET_WLAN_OK)
    {
        return NET_WLAN_NOT_INIT;
    }

    snprintf(cmdStr, sizeof(cmdStr), "TDLS_TEARDOWN " MACSTR, 
        MAC2STR(device.macAddr));

    if (!doBooleanCommand(cmdStr, reply))
    {
        WIFI_LOG(WIFI_ERROR, ("!!Failed to send TDLS teardown."));
        return NET_WLAN_ERROR;
    }

    WIFI_LOG(WIFI_ERROR, ("Successed to send TDLS teardown."));

    return NET_WLAN_OK;    
}

/*------------------------------------------------------------------*/
/*! @brief: Get current TDLS WLAN status synchronously.
*  @param [out] iface - the buffer of storing TDLS Entry
*  @return     result
*  @retval     0 -success
*  @retval     otherwise -failure 
*/
/*------------------------------------------------------------------*/
INT32 c_net_wlan_tdls_get_status(WLAN_TDLS_Status_t *tdlsStatus)
{
    WIFI_LOG_ENTRY(WIFI_API, (""));

    char cmdStr[128] = "\0";
    char reply[256] = "\0";
    FILE *fp = NULL;
    char tdlsStatusTempFile[64]= "/tmp/TDLS_status_tmp";
    char *pos = NULL, *end = NULL;

    if (!tdlsStatus)
    {
        WIFI_LOG(WIFI_DEBUG,("!!Invalid argument, return."));
        return -1;
    }

    snprintf(cmdStr, sizeof(cmdStr), "iwpriv %s stat > %s", 
        get_if_name(), tdlsStatusTempFile);

    doScriptCmd(cmdStr);
    fp = fopen(tdlsStatusTempFile, "r");
    if (!fp)
    {
        WIFI_LOG(WIFI_DEBUG,("!!Failed to TDLS status temp file."));
        return -1;
    }

    char macStr[MAC_STR_LEN + 1] = {'\0'};
    WLAN_TDLS_Device_Identifier_t   macTmp;
    WLAN_TDLS_Device_Identifier_t   deviceZero;
    int i = 0;
    NET_802_11_WIFI_RSSI_T RSSI = {0, 0, 0};
    INT32 Rssi = -100;
    int found_resp_rssi = 0;
    int found_conn_rssi = 0;
    memset(deviceZero.macAddr, 0, sizeof(deviceZero.macAddr));

    memset(tdlsStatus, 0, sizeof(WLAN_TDLS_Status_t));

    while ( fgets(reply, sizeof(reply), fp) )
    {
        WIFI_LOG(WIFI_INFO, ("reply=%s", reply));

        found_resp_rssi = 0;
        found_conn_rssi = 0;
        pos = NULL;
        end = NULL;
        Rssi = -100;
        
        /* Parsing RSSI which is taken in TDLS Discovery Response */
        /* TDLS Disc Resp RSSI from (%02x:%02x:%02x:%02x:%02x:%02x) RSSI = %ld %ld %ld */
        if ( strncmp(reply, "TDLS Disc Resp RSSI from", 
            strlen("TDLS Disc Resp RSSI from")) == 0 )
        {
            pos = reply + strlen("TDLS Disc Resp RSSI from");
            found_resp_rssi = 1;
            WIFI_LOG(WIFI_INFO, ("found TDLS Disc Resp RSSI"));
        }
        /* Parsing RSSI of TDLS */
        /* TDLS Entry(%02x:%02x:%02x:%02x:%02x:%02x) RSSI = %ld %ld %ld */
        else if ( strncmp(reply, "TDLS Entry", strlen("TDLS Entry")) == 0 )
        {
            pos = reply + strlen("TDLS Entry");
            found_conn_rssi = 1;
            WIFI_LOG(WIFI_INFO, ("found TDLS Entry RSSI"));
        }
        else
            continue;

        end = strstr(pos, "(");
        if (end) {
            pos = end + strlen("(");
            memset(macStr, 0, sizeof(macStr));
            strncpy(macStr, pos, sizeof(macStr) - 1);
            memset(macTmp.macAddr, 0, sizeof(macTmp.macAddr));
            str_to_mac(macStr, macTmp.macAddr);
        } else {
            WIFI_LOG(WIFI_ERROR, ("!!Abnormal TDLS info, no MAC address, "
                "ignore it."));
            continue;
        }
        
        RSSI.i4Rssi_a = -100;
        RSSI.i4Rssi_b = -100;
        RSSI.i4Rssi_c = -100;
        end = strstr(pos, "RSSI = ");
        if (end)
        {            
            pos = end + strlen("RSSI = ");
            RSSI.i4Rssi_a = strtol(pos, NULL, 10);
            WIFI_LOG(WIFI_INFO, ("parse Rssi_a=%d", RSSI.i4Rssi_a));

            end = strstr(pos, " ");
            if (end)
            {
                pos = end + strlen(" ");
                RSSI.i4Rssi_b = strtol(pos, NULL, 10);
                WIFI_LOG(WIFI_INFO, ("parse Rssi_b=%d", RSSI.i4Rssi_a));
                
                end = strstr(pos, " ");
                if (end)
                {
                    pos = end + strlen(" ");
                    RSSI.i4Rssi_c = strtol(pos, NULL, 10);
                    WIFI_LOG(WIFI_INFO, ("parse Rssi_c=%d", RSSI.i4Rssi_a));
                }
            }

            /* get max value of rssi_a, rssi_b, rssi_c */
            if ( RSSI.i4Rssi_a < 0 ) {
                if ( RSSI.i4Rssi_b < 0 ) {
                    if ( RSSI.i4Rssi_a > RSSI.i4Rssi_b )
                        Rssi = RSSI.i4Rssi_a;
                    else
                        Rssi = RSSI.i4Rssi_b;
                } else {
                    Rssi = RSSI.i4Rssi_a;
                }

                if ( RSSI.i4Rssi_c < 0 ) {
                    if ( Rssi < RSSI.i4Rssi_c )
                        Rssi = RSSI.i4Rssi_c;
                }
            } else if ( RSSI.i4Rssi_b < 0 ) {
                if ( RSSI.i4Rssi_c < 0 ) {
                    if ( RSSI.i4Rssi_b > RSSI.i4Rssi_c )
                        Rssi = RSSI.i4Rssi_b;
                    else
                        Rssi = RSSI.i4Rssi_c;
                } else {
                    Rssi = RSSI.i4Rssi_b;
                }
            } else if ( RSSI.i4Rssi_c < 0 ) {
                Rssi = RSSI.i4Rssi_c;
            } else {
                Rssi = -100;
            }
        }
        else
        {
            WIFI_LOG(WIFI_ERROR, ("!!Abnormal TDLS info, no RSSI value, "
                "ignore it."));
            continue;
        }

        WIFI_LOG(WIFI_INFO, ("MAX Rssi=%d", Rssi));

        for(i = 0; i < WLAN_TDLS_MAX_CONNECTIONS; i++)
        {
            if (memcmp(tdlsStatus->connectionList[i].device.macAddr, 
                deviceZero.macAddr, 
                sizeof(tdlsStatus->connectionList[i].device.macAddr)) == 0 ||
                memcmp(tdlsStatus->connectionList[i].device.macAddr, 
                macTmp.macAddr, 
                sizeof(tdlsStatus->connectionList[i].device.macAddr)) == 0)
            {
                memcpy(tdlsStatus->connectionList[i].device.macAddr,
                    macTmp.macAddr, 
                    sizeof(tdlsStatus->connectionList[i].device.macAddr));
                if ( found_resp_rssi == 1 )
                    tdlsStatus->connectionList[i].rssiDiscoveryResp = Rssi;
                else if ( found_conn_rssi == 1 )
                    tdlsStatus->connectionList[i].rssi = Rssi;
                break;
            }
        }
    }

    if (fp)
    {
        fclose(fp);
    }

    for(i = 0; i < WLAN_TDLS_MAX_CONNECTIONS; i++)
    {
        if (memcmp(tdlsStatus->connectionList[i].device.macAddr, 
            deviceZero.macAddr, 
            sizeof(tdlsStatus->connectionList[i].device.macAddr)) != 0)
        {
            WIFI_LOG(WIFI_INFO, ("TDLS Entry [%d]: mac=" MACSTR ", " "rssi=%d, " 
                "rssiDiscoveryResponse=%d", i, 
                MAC2STR(tdlsStatus->connectionList[i].device.macAddr), 
                tdlsStatus->connectionList[i].rssi, 
                tdlsStatus->connectionList[i].rssiDiscoveryResp));
        }
    }
    
    return 0;
}

