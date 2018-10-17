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
 * $RCSfile: x_net_wlan_ctrl.c,v $
 * $Revision:
 * $Date:
 * $Author: yali.wu $
 *
 * Description:
 *         This file contains API of wlan associating
 *---------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------
						 include files
 -----------------------------------------------------------------------------*/

#include "x_net_wlan_ctrl.h"
#include "wifi_log.h"
#include "wifi_com.h"
#include "wlan_command.h"
#include "wifi_parse.h"
#include <stdio.h>
#include <string.h>
#include "inc/wps_defs.h"
#include "inc/acfg_drv_if.h"

#include <sys/prctl.h>
#define MAX_CBK_NUM 10

#if CONFIG_LOT26_POWER_SWITCH
#define USB_WLAN_STATUS_FILE    "/proc/driver/usb/dongle_status"
#endif

extern INT32 wps_proc_cred(char * cred, NET_802_11_ASSOCIATE_T *assoc);
extern INT32 wpa_get_scan_results_from_ret_buf(char * buf,NET_802_11_SCAN_RESULT_T *pScanResult);

extern INT32 wlan_check_init_state(void);
extern const char* get_if_name();
extern int u1WlanDrv_NormalizeSignal(int signal);

#if PLATFORM_IS_TV
extern void p2p_notify_events(WLAN_NOTIFY_ID_T event , void * param );
#endif
int wlan_associate(NET_802_11_ASSOCIATE_T *pAssociate);

x_net_wlan_notify_fct _cbk_list[MAX_CBK_NUM] = {NULL};
static NET_802_11_ASSOCIATE_WEP_T g_Associate = {{{0}}};
INT32 wps_c_status = NET_WLAN_OK;
BOOL g_b_wps_associate = FALSE;
BOOL b_wps_cancel = FALSE;
#if WLAN_MULTI_CREDENTIAL_SELECTION
extern struct wpa_ctrl *mon_conn;
BOOL b_start_scan_for_cred = FALSE;
BOOL b_scan_completed = FALSE;
BOOL b_read_event_for_cred_started = FALSE;
BOOL b_stop_read_event = FALSE;
NET_802_11_ASSOCIATE_T g_wps_cred[WLAN_MAX_WPS_CRED_NUM] = {{{0}}};
int g_cred_num = 0;
static INT32 wps_multi_credential_selection(NET_802_11_ASSOCIATE_T *select_creds, NET_802_11_ASSOCIATE_T *multi_creds, int cred_num);
static INT32 wps_get_cfg_from_cred(NET_802_11_ASSOCIATE_T *AP_Cfg, NET_802_11_ASSOCIATE_T *cred);
#endif

static int supc_ess_list_compar(const void *a, const void *b)
{
    const NET_802_11_ESS_INFO_T *wa = a;
    const NET_802_11_ESS_INFO_T *wb = b;
    return (wb->i2_Level - wa->i2_Level);
}

#if CONFIG_LOT26_POWER_SWITCH
/* If wlan conncted, write 1 to "/proc/driver/usb/dongle_status", else write 0 */
INT32 x_update_wlan_status_to_usb_driver(bool status)
{
    WIFI_LOG_ENTRY(WIFI_API, (""));

    char *buf = NULL;
    size_t buflen = 1024;
    INT32 ret = -1;
    char cmd[64] = {'\0'};

    if (access(USB_WLAN_STATUS_FILE, W_OK) == -1)
    {
        perror(USB_WLAN_STATUS_FILE);
        WIFI_LOG(WIFI_ERROR,("WLAN status will not be sync to USB driver"));
        return -1;
    }
    
    buf = malloc(buflen*sizeof(char));
    if (!buf)
    {
        WIFI_LOG(WIFI_ERROR,(" malloc memory failed!%s, %d\n", __FUNCTION__, __LINE__));
        return -1;
    }
    
    if (doStringCommand("STATUS", buf, &buflen) == NULL)
    {
        ret = NET_WLAN_ERROR;
        goto out;
    }

    //if (strncmp(buf, "wpa_state=COMPLETED", strlen ("wpa_state=COMPLETED")) == 0)
    if (status)
    {
        snprintf(cmd, sizeof(cmd), "echo 1 > %s", USB_WLAN_STATUS_FILE);
        doScriptCmd(cmd);
    }
    else
    {
        snprintf(cmd, sizeof(cmd), "echo 0 > %s", USB_WLAN_STATUS_FILE);
        doScriptCmd(cmd);
    }

    ret = 0;

out:
    if (buf)
    {
        free(buf);
    }

    return ret;
}
#endif //CONFIG_LOT26_POWER_SWITCH

INT32 x_net_wlan_wpa_reg_cbk(x_net_wlan_notify_fct pfNotify)
{

    if (pfNotify == NULL )//NULL notify , ignore it .
    {
        return 0;
    }
    WIFI_LOG(WIFI_DEBUG ,("x_net_wlan_wpa_reg_cbk pfNotify %08x!\n",pfNotify));

    _cbk_list[0] = pfNotify;

    return (0);
}

INT32 x_net_wlan_wpa_unreg_cbk(x_net_wlan_notify_fct pfNotify)
{
    int i ;
    if (pfNotify == NULL )//NULL notify , ignore it .
    {
        return 0;
    }
    for ( i = 0; i < MAX_CBK_NUM ; i++)
    {
        if (_cbk_list[i] == pfNotify ) //alread in cbk list ,do not insert it again .
        {
            WIFI_LOG(WIFI_DEBUG ,(">>>>>>>[wifi] unreg cbk notify function ok, removed!\n"));
            _cbk_list[i] = NULL;
            return 0;
        }
    }

    WIFI_LOG(WIFI_DEBUG ,("x_net_wlan_wpa_unreg_cbk list is full ,max value is %d!\n",MAX_CBK_NUM));
    return -1;
}


INT32 x_net_wlan_read_e2p(int e2pAddr, UINT32 *e2pValue)
{
    WIFI_LOG_ENTRY(LogDefault, ("e2p Address=0x%04x", e2pAddr));

    struct iwreq wrq;
    int sock;
    char e2pStr[32] = {0};
    char e2pValStr[8] = {0};
    UINT32 e2pValTmp = 0;
    char *p = NULL;
    int rc;

    if ( !e2pValue )
    {
        WIFI_LOG(LogDefault, ("Invalid parameters, pointer of e2pValue is NULL."));
        return NET_WLAN_ERROR;
    }

    sock = socket(AF_INET, SOCK_STREAM, 0 );
    if ( sock < 0 )
    {
        WIFI_LOG(LogDefault, ("Failed to create socket %s(%d): %s.", __FILE__, __LINE__, strerror(errno)));
        return NET_WLAN_ERROR;
    }

    memset(e2pStr, 0, sizeof(e2pStr));
    memset(e2pValStr, 0, sizeof(e2pValStr));

    strncpy(wrq.ifr_name, "ra0", WLAN_WLAN_IF_MAX_LEN);
    snprintf(e2pStr, sizeof(e2pStr), "%x", e2pAddr);
    e2pStr[sizeof(e2pStr) - 1] = '\0';

    wrq.u.data.pointer = (void *) e2pStr;
    wrq.u.data.length = strlen(e2pStr);
    wrq.u.data.flags = 0;

    rc = ioctl(sock, RTPRIV_IOCTL_E2P, &wrq);
    if ( rc == -1 )
    {
        WIFI_LOG(LogDefault, ("!!ioctl failed, %s(%d): %s.", __FILE__, __LINE__, strerror(errno)));
        close( sock );
        return NET_WLAN_ERROR;
    }
    close( sock );

    WIFI_LOG(LogDefault, ("EEPROM Value=%s.", e2pStr));

    p = strchr(e2pStr, ':');
    if ( !p )
    {
        WIFI_LOG(LogDefault, ("EEPROM Value is invalid."));
        return NET_WLAN_ERROR;
    }

    p++;
    strncpy(e2pValStr, p, sizeof(e2pValStr));
    WIFI_LOG(LogDefault, ("e2pValStr=%s.", e2pValStr));
    e2pValTmp = strtol(e2pValStr, NULL, 16);
    *e2pValue = 0xFF & e2pValTmp;
    WIFI_LOG(LogDefault, ("e2pValue=0x%04X.", *e2pValue));

    return NET_WLAN_OK;
}


/*------------------------------------------------------------------*/
/*! @brief: Get frequency offset from eeprom
*  @param [in] freqoffset - the pointer of frequency offset
*  @return     result
*  @retval     0 -success
*  @retval     otherwise -failure
*
*  frequency offset hex value = 0xf4[bit 6:0]+/-0xf5[bit 5:0]+/-0xf6[bit 5:0]
*  0xf5, 0xf6[bit 7], 0: invalid, 1: valid
*  0xf5, 0xf6[bit 6], 0: increase, 1: decrease
*/
/*------------------------------------------------------------------*/
INT32 x_net_get_wlan_freq_offset(int addr1, int addr2, int addr3, int *freqoffset)
{
    WIFI_LOG_ENTRY(WIFI_API, (""));

    if ( !freqoffset || addr1 < 0 || addr2 < 0 || addr3 < 0 )
    {
        WIFI_LOG(WIFI_ERROR, ("!! Invalid parameters"));
        return NET_WLAN_ERROR;
    }

    UINT32 addr1Value = 0;
    UINT32 addr2Value = 0;
    UINT32 addr3Value = 0;
    UINT32 Freqoffset = 0;
    int ret;

    ret = x_net_wlan_read_e2p(/*0xf4*/addr1, &addr1Value);
    if ( ret != NET_WLAN_OK )
    {
        WIFI_LOG(WIFI_ERROR, ("Failed to get eeprom data, address=%04X.", addr1));
        return NET_WLAN_ERROR;
    }

    ret = x_net_wlan_read_e2p(/*0xf5*/addr2, &addr2Value);
    if ( ret != NET_WLAN_OK )
    {
        WIFI_LOG(WIFI_ERROR, ("Failed to get eeprom data, address=%04X.", addr2));
        return NET_WLAN_ERROR;
    }

    ret = x_net_wlan_read_e2p(/*0xf6*/addr3, &addr3Value);
    if ( ret != NET_WLAN_OK )
    {
        WIFI_LOG(WIFI_ERROR, ("Failed to get eeprom data, address=%04X.", addr3));
        return NET_WLAN_ERROR;
    }

    WIFI_LOG(WIFI_INFO, ("%02x value=%04X, %02x value=0x%04X, %02x value=0x%04X.", 
        addr1Value, addr2Value, addr3Value));

    Freqoffset = 0x7f & addr1Value;

    if ( (0x80 & addr2Value) != 0 )
    {
        if ( (0x40 & addr2Value) != 0 )
        {
            Freqoffset -= (0x3f & addr2Value);
        }
        else
        {
            Freqoffset += (0x3f & addr2Value);
        }
    }

    if ( (0x80 & addr3Value) != 0 )
    {
        if ( (0x40 & addr3Value) != 0 )
        {
            Freqoffset -= (0x3f & addr3Value);
        }
        else
        {
            Freqoffset += (0x3f & addr3Value);
        }
    }

    *freqoffset = Freqoffset;

    WIFI_LOG(WIFI_INFO, ("Frequency offset=%d.", *freqoffset));

    return NET_WLAN_OK;
}


INT32 x_net_wlan_get_mt7662_freq_offset(int *freqoffset)
{
    WIFI_LOG_ENTRY(WIFI_API, (""));

    if ( !freqoffset )
    {
        WIFI_LOG(WIFI_ERROR, ("!! Invalid parameters"));
        return NET_WLAN_ERROR;
    }

    UINT32 addr_3a = 0x3a;
    UINT32 addr_9e = 0x9e;
    UINT32 addr_9f = 0x9f;
    UINT32 addr_3a_Value = 0;
    UINT32 addr_9e_Value = 0;
    UINT32 addr_9f_Value = 0;
    int Freqoffset = 0;
    int ret;

    ret = x_net_wlan_read_e2p(/*0x3a*/addr_3a, &addr_3a_Value);
    if ( ret != NET_WLAN_OK )
    {
        WIFI_LOG(WIFI_ERROR, ("Failed to get eeprom data, address=%04X.", addr_3a));
        return NET_WLAN_ERROR;
    }

    ret = x_net_wlan_read_e2p(/*0x9e*/addr_9e, &addr_9e_Value);
    if ( ret != NET_WLAN_OK )
    {
        WIFI_LOG(WIFI_ERROR, ("Failed to get eeprom data, address=%04X.", addr_9e));
        return NET_WLAN_ERROR;
    }

    ret = x_net_wlan_read_e2p(/*0x9f*/addr_9f, &addr_9f_Value);
    if ( ret != NET_WLAN_OK )
    {
        WIFI_LOG(WIFI_ERROR, ("Failed to get eeprom data, address=%04X.", addr_9f));
        return NET_WLAN_ERROR;
    }

    WIFI_LOG(WIFI_INFO, ("%02x value=%04X, %02x value=0x%04X, %02x value=0x%04X.", 
        addr_3a_Value, addr_9e_Value, addr_9f_Value));

    //if 0x9F = 0 or FF, Frequency offset = 0x3a[bit 6:0] +/- 0x9E[bit 6:0]
    if ( 0 == addr_9f_Value || 0xff == addr_9f_Value )
    {
        if ( (0x80 & addr_9e_Value) == 0 ) //if 0x9e[bit 7] = 0, "+"
            Freqoffset = (0x7f & addr_3a_Value) + (0x7f & addr_9e_Value);
        else //if 0x9e[bit 7] = 1, "-"
            Freqoffset = (0x7f & addr_3a_Value) - (0x7f & addr_9e_Value);
    }
    else//if 0x9F = 0 or FF, Frequency offset = 0x3a[bit 6:0] +/- 0x9E[bit 6:0]
    {
        if ( (0x80 & addr_9e_Value) == 0 ) //if 0x9e[bit 7] = 0, "+"
            Freqoffset = (0x7f & addr_9f_Value) + (0x7f & addr_9e_Value);
        else //if 0x9e[bit 7] = 1, "-"
            Freqoffset = (0x7f & addr_9f_Value) - (0x7f & addr_9e_Value);
    }

    *freqoffset = Freqoffset;

    WIFI_LOG(WIFI_INFO, ("Frequency offset=%d.", *freqoffset));

    return NET_WLAN_OK;
}

INT32 x_net_wlan_get_mac_addr(UINT8 *pMacAddr)
{
    WIFI_LOG_ENTRY(WIFI_API,(""));
    INT32 i4Ret = NET_WLAN_OK;

    struct ifreq ifr;
    int sockfd;
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0)
    {
        WIFI_LOG(WIFI_ERROR,(" >>>>> socket error %d, %s\n", errno, strerror(errno)));
        return NET_WLAN_ERROR;
    }
    strncpy(ifr.ifr_name, get_if_name(), WLAN_WLAN_IF_MAX_LEN);
    if (ioctl(sockfd, SIOCGIFHWADDR, &ifr) == -1)
    {
        WIFI_LOG(WIFI_ERROR ,("ioctl error!\n"));
        i4Ret = NET_WLAN_DRV_INTERFACE_ERR;
        close (sockfd);
        return i4Ret;
    }
    memcpy(pMacAddr, ifr.ifr_hwaddr.sa_data, 6);
    WIFI_LOG(WIFI_DEBUG ,("pMacAddr = %02X:%02X:%02X:%02X:%02X:%02X\n", pMacAddr[0], pMacAddr[1], pMacAddr[2], pMacAddr[3], pMacAddr[4], pMacAddr[5]));
    close (sockfd);
    return i4Ret;

}

void _notify_events(WLAN_NOTIFY_ID_T event , void * param )
{
    int i ;
    for ( i = 0; i < MAX_CBK_NUM ; i++)
    {
        if (_cbk_list[i] == NULL)
            continue ;
        _cbk_list[i](event ,param);
    }
}

void wifi_parse_event(char* buf,size_t buf_len)
{
    if (buf_len <= 0 || buf == NULL ||
            (strncmp(buf, WPA_EVENT_BSS_ADDED, strlen(WPA_EVENT_BSS_ADDED)) == 0 )||
            (strncmp(buf, WPA_EVENT_BSS_REMOVED, strlen(WPA_EVENT_BSS_REMOVED)) == 0))
        return;

    NET_802_11_NFY_T t_nfy;
    x_memset(&t_nfy, 0, sizeof(NET_802_11_NFY_T));
    int len = buf_len -1;
    if ( strncmp(buf, DRIVER_VERSION, strlen(DRIVER_VERSION)) == 0 )
    {
        WIFI_LOG(WIFI_DEBUG ,("C2H: notify-app: WLAN_NOTIFY_DRIVER_VERSION\n"));
        t_nfy.i4NotifykId = WLAN_NOTIFY_DRIVER_VERSION;

        if (buf != NULL)
        {
            if (strlen(buf) < DRIVER_VERSION_LENGTH + 16)
            {
                strncpy(t_nfy.pDriverVer, buf + 16, strlen(buf) - 16);
                t_nfy.pDriverVer[sizeof(t_nfy.pDriverVer) - 1] = '\0';
                WIFI_LOG(WIFI_INFO, ("t_nfy.pDriverVer=%s\n", t_nfy.pDriverVer));
            }
        }
    }
    if ( strncmp(buf, "CTRL-EVENT-CONNECTED", 20) == 0)
    {
        WIFI_LOG(WIFI_DEBUG ,("C2H: notify-app: WLAN_NOTIFY_ASSOCIATE:WLAN_NFY_MSG_OK\n"));
        t_nfy.i4NotifykId = WLAN_NOTIFY_ASSOCIATE;
        t_nfy.pParam[0] = WLAN_NFY_MSG_OK;
#if PLATFORM_IS_TV
        p2p_notify_events(t_nfy.i4NotifykId, (void *)t_nfy.pParam); //for p2p concurrent
#endif

#if CONFIG_LOT26_POWER_SWITCH
        x_update_wlan_status_to_usb_driver(true);
#endif //CONFIG_LOT26_POWER_SWITCH		
    }
    if ( strncmp(buf, "CTRL-EVENT-DISCONNECTED", 23) == 0)
    {
        WIFI_LOG(WIFI_DEBUG ,("C2H: notify-app: WLAN_NOTIFY_ASSOCIATE:WLAN_NOTIFY_DISCONNECTED\n"));
#if WLAN_MULTI_CREDENTIAL_SELECTION
        WIFI_LOG(WIFI_DEBUG, ("g_red_num=%d, wps_c_status=%d", g_cred_num, wps_c_status));

        if (g_b_wps_associate == TRUE && g_cred_num > 0 &&
                (wps_c_status == WLAN_MW_WPS_WAIT_EAP_FAIL || wps_c_status == WLAN_MW_WPS_CONNECTED))
        {
            INT32 ret = -1;
            BOOL b_cred_invalid = FALSE;
            NET_802_11_ASSOCIATE_T select_wps_cred;
            NET_802_11_ASSOCIATE_T AP_CFG;

            memset(&select_wps_cred, 0, sizeof(NET_802_11_ASSOCIATE_T));
            memset(&AP_CFG, 0, sizeof(NET_802_11_ASSOCIATE_T));

            if (g_cred_num > 1)
            {
                ret = wps_multi_credential_selection(&select_wps_cred, g_wps_cred, g_cred_num);
                if (ret != 0)
                {
                    WIFI_LOG(WIFI_DEBUG, ("multiple credentials selection failed, credentials are invalid!"));
                    b_cred_invalid = TRUE;
                }
            }
            else
            {
                select_wps_cred = g_wps_cred[0];
            }

            wps_get_cfg_from_cred(&AP_CFG, &select_wps_cred);

            if (b_cred_invalid)
            {
                /* notify WLAN_NOTIFY_WPS_ASSOCIATE, WLAN_NFY_MSG_WPS_CREDENTIAL_INVALID */
                t_nfy.i4NotifykId = WLAN_NOTIFY_WPS_ASSOCIATE;
                t_nfy.pParam[0] = WLAN_NFY_MSG_WPS_CREDENTIAL_INVALID;
                _notify_events(t_nfy.i4NotifykId, t_nfy.pParam);
            }
            else
            {
                /* notify WLAN_NOTIFY_WPS_ASSOC_SETTING*/
                _notify_events(WLAN_NOTIFY_WPS_ASSOC_SETTING, &AP_CFG);

                /* notify WLAN_NOTIFY_WPS_STATE, WLAN_MW_WPS_CONNECTED */
                t_nfy.i4NotifykId = WLAN_NOTIFY_WPS_STATE;
                t_nfy.pParam[0] = WLAN_MW_WPS_CONNECTED;
                _notify_events(t_nfy.i4NotifykId, t_nfy.pParam);
            }
        }
#endif
        t_nfy.i4NotifykId = WLAN_NOTIFY_ASSOCIATE;
        t_nfy.pParam[0] = WLAN_NFY_MSG_DISCONNECTED;
#if PLATFORM_IS_TV
        p2p_notify_events(t_nfy.i4NotifykId, (void *)t_nfy.pParam);
#endif

#if CONFIG_LOT26_POWER_SWITCH
        x_update_wlan_status_to_usb_driver(false);
#endif
    }

    if ( strncmp(buf, "WLAN_NFY_MSG_WPS_PBC_OVERLAP", 28) == 0)
    {
        WIFI_LOG(WIFI_DEBUG ,("C2H: notify-app: WLAN_NOTIFY_WPS_ASSOCIATE:WLAN_NFY_MSG_WPS_PBC_OVERLAP\n"));
        t_nfy.i4NotifykId = WLAN_NOTIFY_WPS_ASSOCIATE;
        t_nfy.pParam[0] = WLAN_NFY_MSG_WPS_PBC_OVERLAP;
    }

    if ( strncmp(buf, "WPS-OVERLAP-DETECTED", 11) == 0)
    {
        WIFI_LOG(WIFI_DEBUG ,("C2H: notify-app: WLAN_NOTIFY_WPS_ASSOCIATE:WLAN_NFY_MSG_WPS_PBC_OVERLAP\n"));
        t_nfy.i4NotifykId = WLAN_NOTIFY_WPS_ASSOCIATE;
        t_nfy.pParam[0] = WLAN_NFY_MSG_WPS_PBC_OVERLAP;
    }

    if ( strncmp(buf, "WPS-SUCCESS", 11) == 0)
    {
        WIFI_LOG(WIFI_DEBUG ,("C2H: notify-app: WLAN_NOTIFY_WPS_STATE:WLAN_MW_WPS_CONNECTED\n"));
        t_nfy.i4NotifykId = WLAN_NOTIFY_WPS_STATE;
        t_nfy.pParam[0] = WLAN_MW_WPS_CONNECTED;
        wps_c_status = WLAN_MW_WPS_CONNECTED;

#if !WLAN_MULTI_CREDENTIAL_SELECTION
        /* move to "CTRL-EVENT-DISCONNECTED" event for multiple credential selection */
        /* notify WLAN_NOTIFY_WPS_STATE*/
        _notify_events(t_nfy.i4NotifykId, t_nfy.pParam);
        return;
#endif
    }

    if ( strncmp(buf, "WPS-TIMEOUT", 11) == 0)
    {
        WIFI_LOG(WIFI_DEBUG ,("C2H: notify-app: WLAN_NOTIFY_WPS_ASSOCIATE:WLAN_NFY_MSG_WPS_SCAN_TIMEOUT\n"));
        t_nfy.i4NotifykId = WLAN_NOTIFY_WPS_ASSOCIATE;
        t_nfy.pParam[0] = WLAN_NFY_MSG_WPS_SCAN_TIMEOUT;
    }
    if ( strncmp(buf, "WPS-FAIL", 8) == 0)
    {
        if ((len - 22) > 0 && (strstr(buf, "(TKIP Only Prohibited)") != NULL))
        {
            WIFI_LOG(WIFI_DEBUG ,("C2H: notify-app: WLAN_NOTIFY_WPS_STATE:WLAN_MW_WPS_FAIL,WLAN_NFY_MSG_WPS_CREDENTIAL_INVALID\n"));
#if CONFIG_WLAN_SW_FOR_CUSTOM_JSN
            t_nfy.i4NotifykId = WLAN_NOTIFY_WPS_ASSOCIATE;
            t_nfy.pParam[0] = WLAN_NFY_MSG_WPS_CREDENTIAL_INVALID;
#else
            t_nfy.i4NotifykId = WLAN_NOTIFY_WPS_STATE;
            t_nfy.pParam[0] = WLAN_MW_WPS_FAIL;
            t_nfy.pParam[1] = WLAN_NFY_MSG_WPS_CREDENTIAL_INVALID;
#endif
            wps_c_status = WLAN_MW_WPS_FAIL;
        }
        else if ((len - 16) > 0 && (strstr(buf, "(WEP Prohibited)") != NULL))
        {
            WIFI_LOG(WIFI_DEBUG ,("C2H: notify-app: WLAN_NOTIFY_WPS_STATE:WLAN_MW_WPS_FAIL,WLAN_E_WPS_CREDENTIAL_WEP\n"));
#if CONFIG_WLAN_SW_FOR_CUSTOM_JSN
            t_nfy.i4NotifykId = WLAN_NOTIFY_WPS_ASSOCIATE;
            t_nfy.pParam[0] = WLAN_NFY_MSG_WPS_CREDENTIAL_WEP;
#else
            t_nfy.i4NotifykId = WLAN_NOTIFY_WPS_STATE;
            t_nfy.pParam[0] = WLAN_MW_WPS_FAIL;
            t_nfy.pParam[1] = WLAN_NFY_MSG_WPS_CREDENTIAL_WEP;
#endif
            wps_c_status = WLAN_MW_WPS_FAIL;
        }
        else if (len > 12 && strstr(buf, "config_error=") != NULL)
        {
            /* <3>WPS-FAIL msg=8 config_error=18 */
            t_nfy.i4NotifykId = WLAN_NOTIFY_WPS_ASSOCIATE;
            WIFI_LOG(WIFI_DEBUG ,("C2H: notify-app: WLAN_NOTIFY_WPS_ASSOCIATE:PIN_MISTACH or SCAN_TIMEOUT\n"));

            int err_code = -1;
            char err_str[8] = {"\0"};
            char *p = strstr(buf, "config_error=");
            p = p + strlen("config_error=");
            strncpy(err_str, p, 3);
            err_code = atoi(err_str);

            WIFI_LOG(WIFI_DEBUG, ("WPS config error code = %d", err_code));

            switch (err_code)
            {
            case 18:
                t_nfy.pParam[0] = WLAN_NFY_MSG_WPS_PIN_MISMATCH;
                break;
            default:
                t_nfy.pParam[0] = WLAN_NFY_MSG_WPS_SCAN_TIMEOUT;
                break;
            }
            wps_c_status = WLAN_MW_WPS_FAIL;

        }
        else
        {
            WIFI_LOG(WIFI_DEBUG ,("C2H: notify-app: WLAN_NOTIFY_WPS_ASSOCIATE:WLAN_NFY_MSG_WPS_SCAN_TIMEOUT\n"));
            t_nfy.i4NotifykId = WLAN_NOTIFY_WPS_ASSOCIATE;
            t_nfy.pParam[0] = WLAN_NFY_MSG_WPS_SCAN_TIMEOUT;
            wps_c_status = WLAN_MW_WPS_FAIL;
        }
    }
    if ( strncmp(buf, "CTRL-EVENT-SCAN-RESULTS", 22) == 0)
    {
        WIFI_LOG(WIFI_DEBUG ,("C2H: notify-app: WLAN_NOTIFY_SCAN:WLAN_NFY_MSG_OK\n"));
        t_nfy.i4NotifykId = WLAN_NOTIFY_SCAN;
        t_nfy.pParam[0] = WLAN_NFY_MSG_OK;
    }
    if ( strncmp(buf, "WPA: 4-Way Handshake failed", 26) == 0)
    {
        WIFI_LOG(WIFI_DEBUG ,("C2H: notify-app: WLAN_NOTIFY_ASSOCIATE:WLAN_NFY_MSG_NO_MATCHED_CONFIG\n"));
        t_nfy.i4NotifykId = WLAN_NOTIFY_ASSOCIATE;
        t_nfy.pParam[0] = WLAN_NFY_MSG_NO_MATCHED_CONFIG;
    }
	if ( strncmp(buf, "Trying to associate with", 24) == 0 ) {
        /* <3>Trying to associate with 00:24:a5:11:49:ea (SSID='Buffalo1234' freq=2442 MHz) */
        WIFI_LOG(WIFI_DEBUG ,("C2H: notify-app: WLAN_NOTIFY_ASSOCIATE: " 
            "WLAN_NFY_MSG_ASSOC_START\n"));
        t_nfy.i4NotifykId = WLAN_NOTIFY_ASSOCIATE;
        t_nfy.pParam[0] = WLAN_NFY_MSG_ASSOC_START;
        char ssid[33] = {'\0'};
        char *pssid = NULL;
        char freq[8] = {'\0'};
        char *pfreq = NULL, *p = NULL;
        int  ssidlen = 0;

        pfreq = strstr(buf, "freq=");
        if ( pfreq ) {
            p = pfreq;
            pfreq = pfreq + strlen("freq=");
            strncpy(freq, pfreq, 4);
            t_nfy.pParam[1] = atoi(freq);
            
            pssid = strstr(buf, "SSID=");
            if ( pssid ) {
                pssid = pssid + strlen("SSID='");
                ssidlen = pssid - p - 2; //remove '''(single quote mark) and ' '(space)
                if ( ssidlen > 32 ) {
                    ssidlen = 32;
                }
                t_nfy.pParam[2] = ssidlen;
                memcpy(ssid, pssid, ssidlen);
                memcpy(&t_nfy.pParam[3], ssid, ssidlen);
            }
        }

        /*  pParam[0]: Notify message type, pParam[1]: frequency, pParam[2]: ssid length, pParam[3]: ssid*/
        WIFI_LOG(WIFI_DEBUG ,("WPA association has started: ssid=%s, freq=%d", 
            ssid, t_nfy.pParam[1]));
    }
    if ( strncmp(buf, "Authentication with", 19) == 0 ||
        strncmp(buf, "CTRL-EVENT-ASSOC-REJECT", 23) == 0 )
    {
        WIFI_LOG(WIFI_DEBUG,("fgWepShared=%d, wps_asso=%d.", g_Associate.fgWepShared, g_b_wps_associate));

        if (g_Associate.fgWepAutoMode && ! g_b_wps_associate)
        {
            WIFI_LOG(WIFI_DEBUG ,("Try another WEP auth mode\n"));
            g_Associate.fgWepShared = TRUE;
            wlan_associate(&g_Associate.t_Assocaite);
            g_Associate.fgWepShared = FALSE;
        }
        else if (strncmp(buf, "Authentication with", 19) == 0)
        {
            WIFI_LOG(WIFI_DEBUG ,("C2H: notify-app: WLAN_NOTIFY_ASSOCIATE:WLAN_NFY_MSG_AUTH_TIMEOUT\n"));
            g_Associate.fgWepShared = FALSE;
            t_nfy.i4NotifykId = WLAN_NOTIFY_ASSOCIATE;
            t_nfy.pParam[0] = WLAN_NFY_MSG_AUTH_TIMEOUT;
        }
    }
    if ( strncmp(buf, "CTRL-EVENT-EAP-METHOD", 19) == 0)
    {
        WIFI_LOG(WIFI_DEBUG ,("C2H: notify-app: WLAN_NOTIFY_WPS_STATE:WLAN_MW_WPS_ASSOCIATING\n"));
        t_nfy.i4NotifykId = WLAN_NOTIFY_WPS_STATE;
#if CONFIG_WLAN_SW_FOR_CUSTOM_JSN
        t_nfy.pParam[0] = WLAN_MW_WPS_WAIT_WSC_M2;
#else
        t_nfy.pParam[0] = WLAN_MW_WPS_ASSOCIATING; //WLAN_MW_WPS_WAIT_WSC_M2 need to sync or not? if BD app use this param
#endif
        wps_c_status = WLAN_MW_WPS_ASSOCIATING;
    }
    if (strncmp(buf,"CTRL-EVENT-EAP-FAILURE EAP authentication failed",48) == 0)
    {
        //WIFI_LOG(WIFI_DEBUG ,("C2H: notify-app: WLAN_NOTIFY_WPS_STATE:WLAN_MW_WPS_WAIT_EAP_FAIL\n"));
        //t_nfy.i4NotifykId = WLAN_NOTIFY_WPS_STATE;
        //t_nfy.pParam[0] = WLAN_MW_WPS_WAIT_EAP_FAIL;

        wps_c_status = WLAN_MW_WPS_WAIT_EAP_FAIL;
    }

    if (strncmp(buf,"CTRL-EVENT-TERMINATING",22) == 0)
    {
        WIFI_LOG(WIFI_DEBUG ,("C2H: notify-app: WLAN_NOTIFY_SUPPLICANT_TERMINATING\n"));
        t_nfy.i4NotifykId = WLAN_NOTIFY_SUPPLICANT_TERMINATING;
        t_nfy.pParam[0] = WLAN_NFY_MSG_OK;
    }
    if (strncmp(buf,"CTRL-EVENT-CANCEL-CONNECT",25) == 0)
    {
        WIFI_LOG(WIFI_DEBUG ,("C2H: notify-app: WLAN_NOTIFY_CANCEL_CONNECT\n"));
        t_nfy.i4NotifykId = WLAN_NOTIFY_CANCEL_CONNECT;
        t_nfy.pParam[0] = WLAN_NFY_MSG_OK;
    }
    else if (strncmp(buf, WPA_EVENT_WOW_MAGIC_PACKET_RECEIVED, 26) == 0)
    {
        WIFI_LOG(WIFI_DEBUG ,("C2H: notify-app: WLAN_NOTIFY_WOW_MAGIC_PACKET_RECEIVED\n"));
        t_nfy.i4NotifykId = WLAN_NOTIFY_WOW_MAGIC_PACKET_RECEIVED;
        t_nfy.pParam[0] = WLAN_NFY_MSG_OK;
    }
    else if (strncmp(buf, WPA_EVENT_WOW_WAKEUP_REASON_RECEIVED, 26) == 0)
    {
        WIFI_LOG(WIFI_DEBUG ,("C2H: notify-app: WLAN_NOTIFY_WOW_WAKEUP_REASON_RECEIVED\n"));
        t_nfy.i4NotifykId = WLAN_NOTIFY_WOW_WAKEUP_REASON_RECEIVED;
        t_nfy.pParam[0] = WLAN_NFY_MSG_OK;

        char *pos = buf + strlen(WPA_EVENT_WOW_WAKEUP_REASON_RECEIVED);
        char *end = NULL;
        char reason_str[64] = {'\0'};
        UINT32 reason_code = 0;
        int len = 0;

        end = strstr(pos, "reason=");
        if (end)
        {
            pos = end + strlen("reason=");
            end = strchr(pos, ')');
            if (end)
            {
                len = (end - pos) < sizeof(reason_str)? (end - pos):(sizeof(reason_str) - 1);
                strncpy(reason_str, pos, len);
                reason_code = StrToInt(reason_str);
                t_nfy.pParam[1] = reason_code;
                WIFI_LOG(WIFI_DEBUG, ("WOW wakeup reason=%d(%s).\n", 
                    t_nfy.pParam[1], reason_str));
            }
        }
        else
        {
            WIFI_LOG(WIFI_ERROR, ("!!Failed to get WOW wakeup reason code.\n"));
        }
    }

    if (t_nfy.i4NotifykId != 0)
    {
        _notify_events(t_nfy.i4NotifykId, t_nfy.pParam);
    }

    if ( strncmp(buf, "WPS-CRED-RECEIVED", 17) == 0)
    {
        NET_802_11_ASSOCIATE_T assoc;
        memset(&assoc, 0, sizeof(NET_802_11_ASSOCIATE_T));
        WIFI_LOG(WIFI_DEBUG , (">>>>>>>>wps_proc_cred\n"));
        int ret = wps_proc_cred(buf + 18, &assoc);//haifei add for memcpy cause system crash ,if wps_proc_cred function return failed .
        if (ret != 0)
        {
            WIFI_LOG(WIFI_DEBUG ,(">>>>>>> %d,%s,wps_proc_cred failed!\n", __LINE__, __FUNCTION__));
        }
#if WLAN_MULTI_CREDENTIAL_SELECTION
        if (g_cred_num < WLAN_MAX_WPS_CRED_NUM)
        {
            g_wps_cred[g_cred_num] = assoc;
            g_cred_num++;
        }
        else
        {
            WIFI_LOG(WIFI_DEBUG, ("received credentials exceed limit number, discard it."));
        }
#endif
#if !WLAN_MULTI_CREDENTIAL_SELECTION
        WIFI_LOG(WIFI_DEBUG ,("C2H:notify-app: WLAN_NOTIFY_WPS_ASSOC_SETTING\n"));
        t_nfy.i4NotifykId = WLAN_NOTIFY_WPS_ASSOC_SETTING;
        _notify_events(t_nfy.i4NotifykId, &assoc);
#endif
    }
    /* ====================== TDLS events ====================== */
    else if (strncmp(buf, TDLS_EVENT_DISC_RESP,
		strlen(TDLS_EVENT_DISC_RESP)) == 0)
    {
        WIFI_LOG(WIFI_DEBUG ,("C2H: notify-app: WLAN_NOTIFY_TDLS_DISCOVERY_RESP\n"));
        t_nfy.i4NotifykId = WLAN_NOTIFY_TDLS_DISCOVERY_RESP;
		WLAN_TDLS_Device_Identifier_t tdlsDevice = {{0}};
        char *p = NULL;
        char macStr[MAC_STR_LEN + 1] = "\0";
        p = strstr(buf  + strlen(TDLS_EVENT_DISC_RESP), "src=");
        if (p)
        {
            p = p + strlen("str=");
            strncpy(macStr, p, sizeof(macStr) - 1);
            str_to_mac(macStr, tdlsDevice.macAddr);
        }
        _notify_events(t_nfy.i4NotifykId, &tdlsDevice);
    }
    else if (strncmp(buf, TDLS_EVENT_PEER_CONNECTED, 
		strlen(TDLS_EVENT_PEER_CONNECTED)) == 0)
    {
        WIFI_LOG(WIFI_DEBUG ,("C2H: notify-app: TDLS_EVENT_PEER_CONNECTED\n"));
        t_nfy.i4NotifykId = WLAN_NOTIFY_TDLS_PEER_CONNECTED;
		WLAN_TDLS_Device_Identifier_t tdlsDevice = {{0}};
        char *p = NULL;
        char macStr[MAC_STR_LEN + 1] = "\0";
        p = strstr(buf  + strlen(TDLS_EVENT_PEER_CONNECTED), "peer=");
        if (p)
        {
            p = p + strlen("peer=");
            strncpy(macStr, p, sizeof(macStr) - 1);
            str_to_mac(macStr, tdlsDevice.macAddr);
        }
        _notify_events(t_nfy.i4NotifykId, &tdlsDevice);
    }
    else if (strncmp(buf, TDLS_EVENT_PEER_DISCONNECTED,
		strlen(TDLS_EVENT_PEER_DISCONNECTED) ) == 0)
    {
        WIFI_LOG(WIFI_DEBUG ,("C2H: notify-app: WLAN_NOTIFY_TDLS_PEER_DISCONNECTED\n"));
        t_nfy.i4NotifykId = WLAN_NOTIFY_TDLS_PEER_DISCONNECTED;
		WLAN_TDLS_Device_Identifier_t tdlsDevice = {{0}};
        char *p = NULL;
        char macStr[MAC_STR_LEN + 1] = "\0";
        p = strstr(buf  + strlen(TDLS_EVENT_PEER_DISCONNECTED), "peer=");
        if (p)
        {
            p = p + strlen("peer=");
            strncpy(macStr, p, sizeof(macStr) - 1);
            str_to_mac(macStr, tdlsDevice.macAddr);
        }
        _notify_events(t_nfy.i4NotifykId, &tdlsDevice);
    }
    return;
}

int wlan_set_network(int net_id,const char * param1,const char * param2)
{
    char cmdstr[160] = { '\0' };

    memset(cmdstr, 0, sizeof(cmdstr));
    snprintf(cmdstr,sizeof(cmdstr),"SET_NETWORK %d %s %s",net_id,param1,param2);
    cmdstr[sizeof(cmdstr) - 1] = '\0';

    WIFI_LOG (WIFI_DEBUG, (">>>>>>> %s,%d, cmd = %s \n",__FUNCTION__,__LINE__ ,cmdstr));

    if (!doBooleanCommand(cmdstr, "OK"))
    {
        WIFI_LOG(WIFI_DEBUG ,(">>>>>>> %s,%d fail,\n",__FUNCTION__,__LINE__));
        return NET_WLAN_ERROR;
    }

    return  NET_WLAN_OK;
}
int wlan_set_auth_mode_wpa_psk
(NET_802_11_ASSOCIATE_T *pAssociate, INT32 fgGTKIP)
{
    WIFI_LOG_ENTRY(WIFI_DEBUG, (""));
    char* long_cmd = NULL;
    char cmd[50] = {0};
    int key_len = 0;
    int buflen = 0;
    int ret = NET_WLAN_OK;
    if (wlan_set_network(0,"key_mgmt","WPA-PSK") != NET_WLAN_OK)
    {
        ret = NET_WLAN_DRV_INTERFACE_ERR;
        WIFI_LOG(WIFI_DEBUG, (""));
        goto out1;
    }


    if (wlan_set_network(0,"proto","WPA") != NET_WLAN_OK)
    {
        ret = NET_WLAN_DRV_INTERFACE_ERR;
        WIFI_LOG(WIFI_DEBUG, (""));
        goto out1;
    }

    if (pAssociate->t_Key.pui1_PassPhrase == NULL)
    {
        ret = NET_WLAN_INVALID_PARAM;
        WIFI_LOG(WIFI_DEBUG, ("psk is null..."));
        goto out1;
    }

    key_len = strlen(pAssociate->t_Key.pui1_PassPhrase);
    buflen = key_len + 3;
    if ((long_cmd = malloc(buflen)) == NULL)
        return -1;
    if (key_len < 64)
        snprintf(long_cmd, buflen, "\"%s\"",pAssociate->t_Key.pui1_PassPhrase);
    else
        snprintf(long_cmd, buflen, "%s",pAssociate->t_Key.pui1_PassPhrase);

    WIFI_LOG(WIFI_DEBUG, ("psk=%s, lenth=%d.", pAssociate->t_Key.pui1_PassPhrase, key_len));
    if (wlan_set_network(0,"psk",long_cmd) != NET_WLAN_OK)
    {
        ret = NET_WLAN_DRV_INTERFACE_ERR;
        WIFI_LOG(WIFI_DEBUG, (""));
        goto out1;
    }


    if (wlan_set_network(0,"pairwise","CCMP TKIP") != NET_WLAN_OK)
    {
        ret = NET_WLAN_DRV_INTERFACE_ERR;
        WIFI_LOG(WIFI_DEBUG, (""));
        goto out1;
    }

    if (fgGTKIP)
    {
        snprintf(cmd, sizeof(cmd), "TKIP");
    }
    else
    {
        snprintf(cmd, sizeof(cmd), "CCMP TKIP");
    }

    if (wlan_set_network(0,"group",cmd) != NET_WLAN_OK)
    {
        ret = NET_WLAN_DRV_INTERFACE_ERR;
        WIFI_LOG(WIFI_DEBUG, (""));
        goto out1;
    }


out1:

    if (long_cmd)
    {
        free(long_cmd);
        long_cmd = NULL;
    }

    return ret;
}

int wlan_set_auth_mode_wpa_psk_aes
(NET_802_11_ASSOCIATE_T *pAssociate, INT32 fgGTKIP)
{
    WIFI_LOG_ENTRY(WIFI_DEBUG, (""));
    char* long_cmd = NULL;
    char cmd[50] = {0};
    int key_len = 0;
    int buflen = 0;
    int ret = NET_WLAN_OK;
    if (wlan_set_network(0,"key_mgmt","WPA-PSK") != NET_WLAN_OK)
    {
        ret = NET_WLAN_DRV_INTERFACE_ERR;
        WIFI_LOG(WIFI_DEBUG, (""));
        goto out1;
    }


    if (wlan_set_network(0,"proto","WPA") != NET_WLAN_OK)
    {
        ret = NET_WLAN_DRV_INTERFACE_ERR;
        WIFI_LOG(WIFI_DEBUG, (""));
        goto out1;
    }

    if (pAssociate->t_Key.pui1_PassPhrase == NULL)
    {
        ret = NET_WLAN_INVALID_PARAM;
        WIFI_LOG(WIFI_DEBUG, ("psk is null..."));
        goto out1;
    }

    key_len = strlen(pAssociate->t_Key.pui1_PassPhrase);
    buflen = key_len + 3;
    if ((long_cmd = malloc(buflen)) == NULL)
        return -1;
    if (key_len < 64)
        snprintf(long_cmd, buflen, "\"%s\"",pAssociate->t_Key.pui1_PassPhrase);
    else
        snprintf(long_cmd, buflen, "%s",pAssociate->t_Key.pui1_PassPhrase);

    WIFI_LOG(WIFI_DEBUG, ("psk=%s, lenth=%d.", pAssociate->t_Key.pui1_PassPhrase, key_len));
    if (wlan_set_network(0,"psk",long_cmd) != NET_WLAN_OK)
    {
        ret = NET_WLAN_DRV_INTERFACE_ERR;
        WIFI_LOG(WIFI_DEBUG, (""));
        goto out1;
    }


    if (wlan_set_network(0,"pairwise","CCMP") != NET_WLAN_OK)
    {
        ret = NET_WLAN_DRV_INTERFACE_ERR;
        WIFI_LOG(WIFI_DEBUG, (""));
        goto out1;
    }

    if (fgGTKIP)
    {
        snprintf(cmd, sizeof(cmd), "TKIP");
    }
    else
    {
        snprintf(cmd, sizeof(cmd), "CCMP TKIP");
    }

    if (wlan_set_network(0,"group",cmd) != NET_WLAN_OK)
    {
        ret = NET_WLAN_DRV_INTERFACE_ERR;
        WIFI_LOG(WIFI_DEBUG, (""));
        goto out1;
    }


out1:

    if (long_cmd)
    {
        free(long_cmd);
        long_cmd = NULL;
    }

    return ret;
}


int wlan_set_auth_mode_wpa_psk_tkip
(NET_802_11_ASSOCIATE_T *pAssociate, INT32 fgGWEP104)
{
    WIFI_LOG_ENTRY(WIFI_DEBUG, (""));
    char* long_cmd = NULL;
    char cmd[50] = {0};
    int key_len = 0;
    int buflen = 0;
    int ret = NET_WLAN_OK;
    if (wlan_set_network(0,"key_mgmt","WPA-PSK") != NET_WLAN_OK)
    {
        ret = NET_WLAN_DRV_INTERFACE_ERR;
        WIFI_LOG(WIFI_DEBUG, (""));
        goto out1;
    }


    if (wlan_set_network(0,"proto","WPA") != NET_WLAN_OK)
    {
        ret = NET_WLAN_DRV_INTERFACE_ERR;
        WIFI_LOG(WIFI_DEBUG, (""));
        goto out1;
    }

    if (pAssociate->t_Key.pui1_PassPhrase == NULL)
    {
        ret = NET_WLAN_INVALID_PARAM;
        WIFI_LOG(WIFI_DEBUG, ("psk is null..."));
        goto out1;
    }

    key_len = strlen(pAssociate->t_Key.pui1_PassPhrase);
    buflen = key_len + 3;
    if ((long_cmd = malloc(buflen)) == NULL)
        return -1;
    if (key_len < 64)
        snprintf(long_cmd, buflen, "\"%s\"",pAssociate->t_Key.pui1_PassPhrase);
    else
        snprintf(long_cmd, buflen, "%s",pAssociate->t_Key.pui1_PassPhrase);

    WIFI_LOG(WIFI_DEBUG, ("psk=%s, lenth=%d.", pAssociate->t_Key.pui1_PassPhrase, key_len));
    if (wlan_set_network(0,"psk",long_cmd) != NET_WLAN_OK)
    {
        ret = NET_WLAN_DRV_INTERFACE_ERR;
        WIFI_LOG(WIFI_DEBUG, (""));
        goto out1;
    }


    if (wlan_set_network(0,"pairwise","TKIP") != NET_WLAN_OK)
    {
        ret = NET_WLAN_DRV_INTERFACE_ERR;
        WIFI_LOG(WIFI_DEBUG, (""));
        goto out1;
    }

    if (fgGWEP104)
    {
        snprintf(cmd, sizeof(cmd), "WEP104");
    }
    else
    {
        snprintf(cmd, sizeof(cmd), "CCMP TKIP");
    }

    if (wlan_set_network(0,"group",cmd) != NET_WLAN_OK)
    {
        ret = NET_WLAN_DRV_INTERFACE_ERR;
        WIFI_LOG(WIFI_DEBUG, (""));
        goto out1;
    }


out1:

    if (long_cmd)
    {
        free(long_cmd);
        long_cmd = NULL;
    }

    return ret;
}

int wlan_set_auth_mode_wpa_wpa2_psk(NET_802_11_ASSOCIATE_T *pAssociate, INT32 fgGTKIP)
{
    WIFI_LOG_ENTRY(WIFI_DEBUG, (""));
    char* long_cmd = NULL;
    char cmd[50] = {0};
    int key_len = 0;
    int buflen = 0;
    int ret = NET_WLAN_OK;
    if (wlan_set_network(0,"key_mgmt","WPA-PSK") != NET_WLAN_OK)
    {
        ret = NET_WLAN_DRV_INTERFACE_ERR;
        WIFI_LOG(WIFI_DEBUG, (""));
        goto out1;
    }


    if (wlan_set_network(0,"proto","RSN WPA") != NET_WLAN_OK)
    {
        ret = NET_WLAN_DRV_INTERFACE_ERR;
        WIFI_LOG(WIFI_DEBUG, (""));
        goto out1;
    }

    if (pAssociate->t_Key.pui1_PassPhrase == NULL)
    {
        ret = NET_WLAN_INVALID_PARAM;
        WIFI_LOG(WIFI_DEBUG, ("psk is null..."));
        goto out1;
    }

    key_len = strlen(pAssociate->t_Key.pui1_PassPhrase);
    buflen = key_len + 3;
    if ((long_cmd = malloc(buflen)) == NULL)
        return -1;
    if (key_len < 64)
        snprintf(long_cmd, buflen, "\"%s\"",pAssociate->t_Key.pui1_PassPhrase);
    else
        snprintf(long_cmd, buflen, "%s",pAssociate->t_Key.pui1_PassPhrase);

    WIFI_LOG(WIFI_DEBUG, ("psk=%s, lenth=%d.", pAssociate->t_Key.pui1_PassPhrase, key_len));
    if (wlan_set_network(0,"psk",long_cmd) != NET_WLAN_OK)
    {
        ret = NET_WLAN_DRV_INTERFACE_ERR;
        WIFI_LOG(WIFI_DEBUG, (""));
        goto out1;
    }


    if (wlan_set_network(0,"pairwise","CCMP TKIP") != NET_WLAN_OK)
    {
        ret = NET_WLAN_DRV_INTERFACE_ERR;
        WIFI_LOG(WIFI_DEBUG, (""));
        goto out1;
    }
    // new setting
    snprintf(cmd, sizeof(cmd), "CCMP TKIP WEP104 WEP40");

    if (wlan_set_network(0,"group",cmd) != NET_WLAN_OK)
    {
        ret = NET_WLAN_DRV_INTERFACE_ERR;
        WIFI_LOG(WIFI_DEBUG, (""));
        goto out1;
    }


out1:

    if (long_cmd)
    {
        free(long_cmd);
        long_cmd = NULL;
    }

    return ret;
}

int wlan_set_auth_mode_wpa2_psk(NET_802_11_ASSOCIATE_T *pAssociate, INT32 fgGTKIP, INT32 fgGWEP104)
{
    WIFI_LOG_ENTRY(WIFI_DEBUG, (""));
    char* long_cmd = NULL;
    char cmd[50] = {0};
    int key_len = 0;
    int buflen = 0;
    int ret = NET_WLAN_OK;
    if (wlan_set_network(0,"key_mgmt","WPA-PSK") != NET_WLAN_OK)
    {
        ret = NET_WLAN_DRV_INTERFACE_ERR;
        WIFI_LOG(WIFI_DEBUG, (""));
        goto out1;
    }


    if (wlan_set_network(0,"proto","RSN") != NET_WLAN_OK)
    {
        ret = NET_WLAN_DRV_INTERFACE_ERR;
        WIFI_LOG(WIFI_DEBUG, (""));
        goto out1;
    }

    if (pAssociate->t_Key.pui1_PassPhrase == NULL)
    {
        ret = NET_WLAN_INVALID_PARAM;
        WIFI_LOG(WIFI_DEBUG, ("psk is null..."));
        goto out1;
    }

    key_len = strlen(pAssociate->t_Key.pui1_PassPhrase);
    buflen = key_len + 3;
    if ((long_cmd = malloc(buflen)) == NULL)
        return -1;
    if (key_len < 64)
        snprintf(long_cmd, buflen, "\"%s\"",pAssociate->t_Key.pui1_PassPhrase);
    else
        snprintf(long_cmd, buflen, "%s",pAssociate->t_Key.pui1_PassPhrase);

    WIFI_LOG(WIFI_DEBUG, ("psk=%s, lenth=%d.", pAssociate->t_Key.pui1_PassPhrase, key_len));
    if (wlan_set_network(0,"psk",long_cmd) != NET_WLAN_OK)
    {
        ret = NET_WLAN_DRV_INTERFACE_ERR;
        WIFI_LOG(WIFI_DEBUG, (""));
        goto out1;
    }

    //new setting
    if (wlan_set_network(0,"pairwise","CCMP") != NET_WLAN_OK)
    {
        ret = NET_WLAN_DRV_INTERFACE_ERR;
        WIFI_LOG(WIFI_DEBUG, (""));
        goto out1;
    }

    if (fgGTKIP)
    {
        snprintf(cmd, sizeof(cmd), "TKIP");
    }
    else if (fgGWEP104)
    {
        snprintf(cmd, sizeof(cmd), "WEP104");
    }
    else
    {
        snprintf(cmd, sizeof(cmd), "CCMP TKIP");
    }

    if (wlan_set_network(0,"group",cmd) != NET_WLAN_OK)
    {
        ret = NET_WLAN_DRV_INTERFACE_ERR;
        WIFI_LOG(WIFI_DEBUG, (""));
        goto out1;
    }


out1:

    if (long_cmd)
    {
        free(long_cmd);
        long_cmd = NULL;
    }

    return ret;
}



int wlan_set_auth_mode_wpa2_psk_aes(NET_802_11_ASSOCIATE_T *pAssociate, INT32 fgGTKIP)
{
    WIFI_LOG_ENTRY(WIFI_DEBUG, (""));
    char* long_cmd = NULL;
    char cmd[50] = {0};
    int key_len = 0;
    int buflen = 0;
    int ret = NET_WLAN_OK;
    if (wlan_set_network(0,"key_mgmt","WPA-PSK") != NET_WLAN_OK)
    {
        ret = NET_WLAN_DRV_INTERFACE_ERR;
        WIFI_LOG(WIFI_DEBUG, (""));
        goto out1;
    }


    if (wlan_set_network(0,"proto","RSN") != NET_WLAN_OK)
    {
        ret = NET_WLAN_DRV_INTERFACE_ERR;
        WIFI_LOG(WIFI_DEBUG, (""));
        goto out1;
    }

    if (pAssociate->t_Key.pui1_PassPhrase == NULL)
    {
        ret = NET_WLAN_INVALID_PARAM;
        WIFI_LOG(WIFI_DEBUG, ("psk is null..."));
        goto out1;
    }

    key_len = strlen(pAssociate->t_Key.pui1_PassPhrase);
    buflen = key_len + 3;
    if ((long_cmd = malloc(buflen)) == NULL)
        return -1;
    if (key_len < 64)
        snprintf(long_cmd, buflen, "\"%s\"",pAssociate->t_Key.pui1_PassPhrase);
    else
        snprintf(long_cmd, buflen, "%s",pAssociate->t_Key.pui1_PassPhrase);

    WIFI_LOG(WIFI_DEBUG, ("psk=%s, lenth=%d.", pAssociate->t_Key.pui1_PassPhrase, key_len));
    if (wlan_set_network(0,"psk",long_cmd) != NET_WLAN_OK)
    {
        ret = NET_WLAN_DRV_INTERFACE_ERR;
        WIFI_LOG(WIFI_DEBUG, (""));
        goto out1;
    }


    if (wlan_set_network(0,"pairwise","CCMP") != NET_WLAN_OK)
    {
        ret = NET_WLAN_DRV_INTERFACE_ERR;
        WIFI_LOG(WIFI_DEBUG, (""));
        goto out1;
    }

    if (fgGTKIP)
    {
        snprintf(cmd, sizeof(cmd), "TKIP");
    }
    else
    {
        snprintf(cmd, sizeof(cmd), "CCMP TKIP");
    }

    if (wlan_set_network(0,"group",cmd) != NET_WLAN_OK)
    {
        ret = NET_WLAN_DRV_INTERFACE_ERR;
        WIFI_LOG(WIFI_DEBUG, (""));
        goto out1;
    }


out1:

    if (long_cmd)
    {
        free(long_cmd);
        long_cmd = NULL;
    }

    return ret;
}




int wlan_set_auth_mode_wpa2_psk_tkip(NET_802_11_ASSOCIATE_T *pAssociate)
{
    WIFI_LOG_ENTRY(WIFI_DEBUG, (""));
    char* long_cmd = NULL;
    int key_len = 0;
    int buflen = 0;
    int ret = NET_WLAN_OK;
    if (wlan_set_network(0,"key_mgmt","WPA-PSK") != NET_WLAN_OK)
    {
        ret = NET_WLAN_DRV_INTERFACE_ERR;
        WIFI_LOG(WIFI_DEBUG, (""));
        goto out1;
    }


    if (wlan_set_network(0,"proto","RSN") != NET_WLAN_OK)
    {
        ret = NET_WLAN_DRV_INTERFACE_ERR;
        WIFI_LOG(WIFI_DEBUG, (""));
        goto out1;
    }

    if (pAssociate->t_Key.pui1_PassPhrase == NULL)
    {
        ret = NET_WLAN_INVALID_PARAM;
        WIFI_LOG(WIFI_DEBUG, ("psk is null..."));
        goto out1;
    }

    key_len = strlen(pAssociate->t_Key.pui1_PassPhrase);
    buflen = key_len + 3;
    if ((long_cmd = malloc(buflen)) == NULL)
        return -1;
    if (key_len < 64)
        snprintf(long_cmd, buflen, "\"%s\"",pAssociate->t_Key.pui1_PassPhrase);
    else
        snprintf(long_cmd, buflen, "%s",pAssociate->t_Key.pui1_PassPhrase);

    WIFI_LOG(WIFI_DEBUG, ("psk=%s, lenth=%d.", pAssociate->t_Key.pui1_PassPhrase, key_len));
    if (wlan_set_network(0,"psk",long_cmd) != NET_WLAN_OK)
    {
        ret = NET_WLAN_DRV_INTERFACE_ERR;
        WIFI_LOG(WIFI_DEBUG, (""));
        goto out1;
    }


    if (wlan_set_network(0,"pairwise","TKIP") != NET_WLAN_OK)
    {
        ret = NET_WLAN_DRV_INTERFACE_ERR;
        WIFI_LOG(WIFI_DEBUG, (""));
        goto out1;
    }


    if (wlan_set_network(0,"group","CCMP TKIP") != NET_WLAN_OK)
    {
        ret = NET_WLAN_DRV_INTERFACE_ERR;
        WIFI_LOG(WIFI_DEBUG, (""));
        goto out1;
    }

out1:

    if (long_cmd)
    {
        free(long_cmd);
        long_cmd = NULL;
    }

    return ret;
}




int wlan_set_auth_mode_open(NET_802_11_ASSOCIATE_T *pAssociate)
{
    int ret = 0;
    WIFI_LOG_ENTRY(WIFI_DEBUG, (""));

    WIFI_LOG(WIFI_DEBUG,("IEEE_802_11_AUTH_MODE_OPEN!\n"));
    if (wlan_set_network(0,"auth_alg","OPEN") != NET_WLAN_OK)
    {
        ret = NET_WLAN_DRV_INTERFACE_ERR;
        goto out1;
    }

    if (wlan_set_network(0,"key_mgmt","NONE") != NET_WLAN_OK)
    {
        ret = NET_WLAN_DRV_INTERFACE_ERR;
        goto out1;
    }

out1:
    return ret;
}


int wlan_set_auth_mode_shared(NET_802_11_ASSOCIATE_T *pAssociate)
{
    char* long_cmd = NULL;
    int ret = 0;
    INT32 key_len = 0;
    int buflen = 0;
    WIFI_LOG_ENTRY(WIFI_DEBUG, (""));

    WIFI_LOG(WIFI_DEBUG,("IEEE_802_11_AUTH_MODE_SHARED!\n"));
    if (wlan_set_network(0,"auth_alg","SHARED") != NET_WLAN_OK)
    {
        ret = NET_WLAN_DRV_INTERFACE_ERR;
        goto out1;
    }


    if (wlan_set_network(0,"key_mgmt","NONE") != NET_WLAN_OK)
    {
        ret = NET_WLAN_DRV_INTERFACE_ERR;
        goto out1;
    }

    //WEP key
    if (pAssociate->t_Key.pui1_PassPhrase == NULL)
    {
        ret = NET_WLAN_DRV_INTERFACE_ERR;
        goto out1;
    }
    key_len = strlen(pAssociate->t_Key.pui1_PassPhrase);
    buflen = key_len + 3;
    if ((long_cmd = malloc(buflen)) == NULL)
        return -1;

    if (key_len == 10 || key_len == 26)
        snprintf(long_cmd, buflen, "%s",pAssociate->t_Key.pui1_PassPhrase);
    else
        snprintf(long_cmd, buflen, "\"%s\"",pAssociate->t_Key.pui1_PassPhrase);

    //wep key tx idx
    if (wlan_set_network(0,"wep_key0",long_cmd) != NET_WLAN_OK)
    {
        WIFI_LOG(WIFI_DEBUG, ("wep_key0 is null..."));
        ret = NET_WLAN_DRV_INTERFACE_ERR;
        goto out1;
    }


    //wep key tx idx
    if (wlan_set_network(0,"wep_tx_keyidx","0") != NET_WLAN_OK)
    {
        ret = NET_WLAN_DRV_INTERFACE_ERR;
        goto out1;
    }
out1:

    if (long_cmd)
    {
        free(long_cmd);
        long_cmd = NULL;
    }

    return ret;
}


int wlan_set_auth_mode_wepauto(NET_802_11_ASSOCIATE_T *pAssociate)
{
    char* long_cmd = NULL;
    char cmd[50] = {0};
    int ret = 0;
    INT32 key_len = 0;
    int buflen = 0;
    WIFI_LOG_ENTRY(WIFI_DEBUG, (""));

    WIFI_LOG(WIFI_DEBUG,("IEEE_802_11_AUTH_MODE_SHARED!\n"));
    if (g_Associate.fgWepShared)
    {
        snprintf(cmd, sizeof(cmd), "SHARED");
    }
    else
    {
        snprintf(cmd, sizeof(cmd), "OPEN");
    }
    if (wlan_set_network(0,"auth_alg",cmd) != NET_WLAN_OK)
    {
        ret = NET_WLAN_DRV_INTERFACE_ERR;
        goto out1;
    }


    if (wlan_set_network(0,"key_mgmt","NONE") != NET_WLAN_OK)
    {
        ret = NET_WLAN_DRV_INTERFACE_ERR;
        goto out1;
    }

    //WEP key
    if (pAssociate->t_Key.pui1_PassPhrase == NULL)
    {
        ret = NET_WLAN_DRV_INTERFACE_ERR;
        goto out1;
    }
    key_len = strlen(pAssociate->t_Key.pui1_PassPhrase);
    buflen = key_len + 3;
    if ((long_cmd = malloc(buflen)) == NULL)
        return -1;

    if (key_len == 10 || key_len == 26)
        snprintf(long_cmd, buflen, "%s",pAssociate->t_Key.pui1_PassPhrase);
    else
        snprintf(long_cmd, buflen, "\"%s\"",pAssociate->t_Key.pui1_PassPhrase);

    //wep key tx idx
    if (wlan_set_network(0,"wep_key0",long_cmd) != NET_WLAN_OK)
    {
        WIFI_LOG(WIFI_DEBUG, ("wep_key0 is null..."));
        ret = NET_WLAN_DRV_INTERFACE_ERR;
        goto out1;
    }


    //wep key tx idx
    if (wlan_set_network(0,"wep_tx_keyidx","0") != NET_WLAN_OK)
    {
        ret = NET_WLAN_DRV_INTERFACE_ERR;
        goto out1;
    }
out1:

    if (long_cmd)
    {
        free(long_cmd);
        long_cmd = NULL;
    }

    return ret;
}



int wlan_set_auth_mode_none(NET_802_11_ASSOCIATE_T *pAssociate)
{
    WIFI_LOG_ENTRY(WIFI_DEBUG, (""));

    int ret = 0;
    if (wlan_set_network(0,"key_mgmt","NONE") != NET_WLAN_OK)
    {
        ret = NET_WLAN_DRV_INTERFACE_ERR;
    }

    return ret;
}

int wlan_set_remove_network(NET_802_11_ASSOCIATE_T *pAssociate)
{
    int ret = 0;
    WIFI_LOG_ENTRY(WIFI_DEBUG, (""));
    if (!doBooleanCommand("REMOVE_NETWORK all", "OK"))
    {
        ret = NET_WLAN_DRV_INTERFACE_ERR;
        WIFI_LOG(WIFI_DEBUG, ("send fail."));
    }
    return ret;
}
static int wlan_set_scan_ssid_network(NET_802_11_ASSOCIATE_T *pAssociate)
{
    int ret = 0;
    WIFI_LOG_ENTRY(WIFI_DEBUG, (""));

    if (wlan_set_network(0,"scan_ssid","1") != NET_WLAN_OK)
    {
        ret = NET_WLAN_DRV_INTERFACE_ERR;
        WIFI_LOG(WIFI_DEBUG, ("send fail."));
    }

    return ret;
}

int wlan_set_add_network(NET_802_11_ASSOCIATE_T *pAssociate)
{
    int ret = 0;
    WIFI_LOG_ENTRY(WIFI_DEBUG, (""));

    if (!doBooleanCommand("ADD_NETWORK", "0"))
    {
        ret = NET_WLAN_DRV_INTERFACE_ERR;
        WIFI_LOG(WIFI_DEBUG, ("send fail."));
    }
    return ret;
}

static int wlan_set_ssid_network(NET_802_11_ASSOCIATE_T *pAssociate)
{
    int ret = 0;
    char *cmd = NULL;
    WIFI_LOG_ENTRY(WIFI_DEBUG, (""));

    /* Note: ssid for wpa_cli requires quotes '"SSID'" */
    cmd = malloc(pAssociate->t_Ssid.ui4_SsidLen+1+2);
    if (!cmd)
    {
        WIFI_LOG(WIFI_DEBUG, ("Alloc memory failed, %s, Line:%d", __FUNCTION__, __LINE__));
        return -1;
    }
    x_memset(cmd,0,pAssociate->t_Ssid.ui4_SsidLen+1+2);
    //x_snprintf(argv[6],pAssociate->t_Ssid.ui4_SsidLen+2, "\"%s\"", pAssociate->t_Ssid.ui1_aSsid);
    x_snprintf(cmd, pAssociate->t_Ssid.ui4_SsidLen+1+2, "\"%s\"", pAssociate->t_Ssid.ui1_aSsid);
    if (wlan_set_network(0,"ssid",cmd) != NET_WLAN_OK)
    {
        ret = NET_WLAN_DRV_INTERFACE_ERR;
        WIFI_LOG(WIFI_DEBUG, (""));
        goto out1;
    }

    WIFI_LOG(WIFI_DEBUG,("ssid = %s,ssid_len = %d\n",cmd,(int)pAssociate->t_Ssid.ui4_SsidLen));
out1:

    if (cmd)
    {
        free(cmd);
        cmd = NULL;
    }

    return ret;
}

static int wlan_set_network_bssid(NET_802_11_ASSOCIATE_T *pAssociate)
{
    int ret = 0;
    char cmd[20] = {'\0'};
    WIFI_LOG_ENTRY(WIFI_DEBUG, (""));

    snprintf(cmd, sizeof(cmd), MACSTR, MAC2STR(pAssociate->t_Bssid));
    if (strncmp(cmd, "00:00:00:00:00:00", 17) == 0 ||
        strncmp(cmd, "ff:ff:ff:ff:ff:ff", 17) == 0 ||
        strncmp(cmd, "FF:FF:FF:FF:FF:FF", 17) == 0)
    {
        WIFI_LOG(WIFI_ERROR, ("Invlid bssid = %s\n", cmd));
        return -1;
    }

    if (wlan_set_network(0, "bssid", cmd) != NET_WLAN_OK)
    {
        ret = NET_WLAN_DRV_INTERFACE_ERR;
        WIFI_LOG(WIFI_DEBUG, (""));
    }

    WIFI_LOG(WIFI_DEBUG, ("set_network bssid = %s\n", cmd));

    return ret;
}

int wlan_set_enable_network(NET_802_11_ASSOCIATE_T *pAssociate)
{
    int ret = 0;
    WIFI_LOG_ENTRY(WIFI_DEBUG, (""));

    if (!doBooleanCommand("ENABLE_NETWORK 0", "OK"))
    {
        ret = NET_WLAN_DRV_INTERFACE_ERR;
        WIFI_LOG(WIFI_DEBUG, ("send fail."));
    }

    return ret;
}

/* "anytime connection" is dropped, so remove this part */
#if 0 //CONFIG_WLAN_SW_FOR_CUSTOM_JSN //porting "anytime connection"
static int wlan_set_autoscan_exponential(NET_802_11_ASSOCIATE_T *pAssociate)
{
   // return 1;

    int ret = 0;
    WIFI_LOG_ENTRY(WIFI_DEBUG, (""));
    if (!doBooleanCommand("AUTOSCAN exponential:2:30", "OK"))
    {
        ret = NET_WLAN_DRV_INTERFACE_ERR;
        WIFI_LOG(WIFI_DEBUG, ("send fail."));
    }

    return ret;
}
static int wlan_set_sta_autoconnect(NET_802_11_ASSOCIATE_T *pAssociate)
{
    //return 1;

    int ret = 0;
    WIFI_LOG_ENTRY(WIFI_DEBUG, (""));

    if (!doBooleanCommand("STA_AUTOCONNECT 1", "OK"))
    {
        ret = NET_WLAN_DRV_INTERFACE_ERR;
        WIFI_LOG(WIFI_DEBUG, ("send fail."));
    }

    return ret;
}
#endif

int wlan_set_reconnect_network(NET_802_11_ASSOCIATE_T *pAssociate)
{
    int ret = 0;
    WIFI_LOG_ENTRY(WIFI_DEBUG, (""));

    if (!doBooleanCommand("RECONNECT", "OK"))
    {
        ret = NET_WLAN_DRV_INTERFACE_ERR;
        WIFI_LOG(WIFI_DEBUG, ("send fail."));
    }
    return ret;
}

int wlan_associate(NET_802_11_ASSOCIATE_T *pAssociate)
{
    WIFI_LOG(WIFI_DEBUG ,("wlan_associate ssid[%s] auth[%d]cipher[%d]\n", pAssociate->t_Ssid.ui1_aSsid, pAssociate->e_AuthMode, pAssociate->e_AuthCipher));
    int ret = 0;
    CHAR key_tmp[WLAN_KEY_MAX_LENGTH + 1] = {0};

    WIFI_LOG_ENTRY(WIFI_DEBUG, ("Ssid=%s,len=%ld,e_AssocCase=%d, Cipher=%d, Mode=%d, b_IsAscii=%d, pass_phrase=%s, key_body=%s, bssid=%02x:%02x:%02x:%02x:%02x:%02x, prio=%d.\n",
                                pAssociate->t_Ssid.ui1_aSsid,
                                pAssociate->t_Ssid.ui4_SsidLen,
                                pAssociate->e_AssocCase,
                                pAssociate->e_AuthCipher,
                                pAssociate->e_AuthMode,
                                pAssociate->t_Key.b_IsAscii,
                                pAssociate->t_Key.pui1_PassPhrase,
                                pAssociate->t_Key.key_body,
                                pAssociate->t_Bssid[0],pAssociate->t_Bssid[1],pAssociate->t_Bssid[2],pAssociate->t_Bssid[3],pAssociate->t_Bssid[4],pAssociate->t_Bssid[5],
                                pAssociate->ui1_Priority));

    memcpy(g_Associate.t_Assocaite.t_Bssid, pAssociate->t_Bssid, 6);
    g_Associate.t_Assocaite.t_Ssid = pAssociate->t_Ssid;
    g_Associate.t_Assocaite.ui1_Priority = pAssociate->ui1_Priority;
    g_Associate.t_Assocaite.e_AssocCase = pAssociate->e_AssocCase;
    g_Associate.t_Assocaite.e_AuthCipher = pAssociate->e_AuthCipher;
    g_Associate.t_Assocaite.e_AuthMode = pAssociate->e_AuthMode;
    g_Associate.t_Assocaite.t_Key.b_IsAscii = pAssociate->t_Key.b_IsAscii;
    memset(key_tmp, 0, sizeof(key_tmp));
    strncpy(key_tmp, pAssociate->t_Key.pui1_PassPhrase, sizeof(key_tmp) - 1);
    memset(g_Associate.t_Assocaite.t_Key.key_body, 0,
        sizeof(g_Associate.t_Assocaite.t_Key.key_body));
    strncpy(g_Associate.t_Assocaite.t_Key.key_body, key_tmp,
        sizeof(g_Associate.t_Assocaite.t_Key.key_body) - 1);
    g_Associate.t_Assocaite.t_Key.pui1_PassPhrase = g_Associate.t_Assocaite.t_Key.key_body;
    g_Associate.fgWepAutoMode = FALSE;


    if (!doBooleanCommand("DISCONNECT", "OK"))
    {
        WIFI_LOG(WIFI_DEBUG, ("send fail."));
        ret = NET_WLAN_DRV_INTERFACE_ERR;
        goto out;
    }
    ret = wlan_set_remove_network(pAssociate);
    if (ret < 0)
        goto out;

    ret = wlan_set_add_network(pAssociate);
    if (ret < 0)
        goto out;


    if (wlan_set_network(0,"auth_alg","OPEN") != NET_WLAN_OK)
    {
        ret = NET_WLAN_DRV_INTERFACE_ERR;

        WIFI_LOG(WIFI_DEBUG, ("send fail."));
        goto out;
    }


    WIFI_LOG(WIFI_INFO, ("pAssociate->e_AuthMode=%d.", pAssociate->e_AuthMode));
    switch (pAssociate->e_AuthMode)
    {
    case IEEE_802_11_AUTH_MODE_WPA_PSK:
        ret =  wlan_set_auth_mode_wpa_psk(pAssociate, 0);
        if (ret < 0)
            goto out;
        break;
    case IEEE_802_11_AUTH_MODE_WPA_PSK_AES:
        ret =  wlan_set_auth_mode_wpa_psk_aes(pAssociate, 0);
        if (ret < 0)
            goto out;
        break;
    case IEEE_802_11_AUTH_MODE_WPA_PSK_TKIP:
        ret =  wlan_set_auth_mode_wpa_psk_tkip(pAssociate, 0);
        if (ret < 0)
            goto out;
        break;
    case IEEE_802_11_AUTH_MODE_WPAPSK_OR_WPA2PSK:
        ret = wlan_set_auth_mode_wpa_wpa2_psk(pAssociate, 0);
        if (ret < 0)
            goto out;
        break;
    case IEEE_802_11_AUTH_MODE_WPA_PSK_GTKIP:
        ret = wlan_set_auth_mode_wpa_psk(pAssociate, 1);
        if (ret < 0)
            goto out;
        break;
    case IEEE_802_11_AUTH_MODE_WPA_PSK_AES_GTKIP:
        ret = wlan_set_auth_mode_wpa_psk_aes(pAssociate, 1);
        if (ret < 0)
            goto out;
        break;
    case IEEE_802_11_AUTH_MODE_WPA_PSK_TKIP_GWEP104:
        ret = wlan_set_auth_mode_wpa_psk_tkip(pAssociate, 1);
        if (ret < 0)
        {
            goto out;
        }
        break;
    case IEEE_802_11_AUTH_MODE_WPA2_PSK_GTKIP:
        ret = wlan_set_auth_mode_wpa2_psk(pAssociate, 1, 0);
        if (ret < 0)
            goto out;
        break;
    case IEEE_802_11_AUTH_MODE_WPA2_PSK_GWEP104:
        ret = wlan_set_auth_mode_wpa2_psk(pAssociate, 0, 1);
        if (ret < 0)
        {
            goto out;
        }
        break;
    case IEEE_802_11_AUTH_MODE_WPA2_PSK_AES_GTKIP:
        ret = wlan_set_auth_mode_wpa2_psk_aes(pAssociate, 1);
        if (ret < 0)
            goto out;
        break;
    case IEEE_802_11_AUTH_MODE_WPAPSK_OR_WPA2PSK_GTKIP:
        ret = wlan_set_auth_mode_wpa_wpa2_psk(pAssociate, 1);
        if (ret < 0)
            goto out;
        break;
    case IEEE_802_11_AUTH_MODE_WPA2_PSK:
        ret = wlan_set_auth_mode_wpa2_psk(pAssociate, 0, 0);
        if (ret < 0)
            goto out;
        break;

    case IEEE_802_11_AUTH_MODE_WPA2_PSK_AES:
        ret = wlan_set_auth_mode_wpa2_psk_aes(pAssociate, 0);
        if (ret < 0)
            goto out;
        break;

    case IEEE_802_11_AUTH_MODE_WPA2_PSK_TKIP:
        ret = wlan_set_auth_mode_wpa2_psk_tkip(pAssociate);
        if (ret < 0)
            goto out;
        break;

    case IEEE_802_11_AUTH_MODE_OPEN:
        ret = wlan_set_auth_mode_open(pAssociate);
        if (ret < 0)
            goto out;
        break;

    case IEEE_802_11_AUTH_MODE_SHARED:
        ret = wlan_set_auth_mode_shared(pAssociate);
        if (ret < 0)
            goto out;
        break;

    case IEEE_802_11_AUTH_MODE_WEPAUTO:
        g_Associate.fgWepAutoMode = TRUE;
        ret = wlan_set_auth_mode_wepauto(pAssociate);
        if (ret < 0)
            goto out;
        break;

    case IEEE_802_11_AUTH_MODE_NONE:
        ret = wlan_set_auth_mode_none(pAssociate);
        if (ret < 0)
            goto out;
        break;

    default:
        break;
    }

    ret = wlan_set_scan_ssid_network(pAssociate);
    if (ret < 0)
        goto out;

    ret = wlan_set_ssid_network(pAssociate);
    if (ret < 0)
        goto out;

    wlan_set_network_bssid(pAssociate);

    ret = wlan_set_enable_network(pAssociate);
    if (ret < 0)
        goto out;
/* "anytime connection" is dropped, so remove this part */
#if 0 //CONFIG_WLAN_SW_FOR_CUSTOM_JSN //porting "anytime connection"
	ret = wlan_set_autoscan_exponential(pAssociate);
	//if (ret < 0)			
	//  goto out;
	
	ret = wlan_set_sta_autoconnect(pAssociate);
	//if (ret < 0)			
	//  goto out;
#endif
    ret = wlan_set_reconnect_network(pAssociate);
    if (ret < 0)
        goto out;
out:
    return ret;
    return 0;
}

INT32 x_net_wlan_scan_with_ssid(x_net_wlan_notify_fct pfNotify, char * ssid, int ssid_len )
{
    if (wlan_check_init_state() != NET_WLAN_OK)
    {
        return NET_WLAN_NOT_INIT;
    }
    if (NULL == ssid)
    {
        WIFI_LOG(WIFI_WARNING, ("Invalid Param\n"));
        return NET_WLAN_ERROR;
    }
    x_net_wlan_wpa_reg_cbk(pfNotify);

    if (0 == strncmp (get_if_name(), "ra0", strlen ("ra0")))
    {
        char ch_iwpriv_cmd[512] = {0};
#if PLATFORM_IS_TV
	{
		char data[256] = {0};
		struct iwreq wrq;
		int sock;
		int rc;
		
		sock = socket(AF_INET, SOCK_STREAM, 0 );
		if ( sock < 0 )
		{
			WIFI_LOG(WIFI_ERROR, ("x_net_wlan_scan_with_ssid Create socket failed %s" ,strerror(errno)));
			return NET_WLAN_ERROR;
		}
		
		memset(&wrq, 0, sizeof(wrq));
		strncpy(wrq.ifr_name, get_if_name(), strlen ("ra0"));
		snprintf (data, sizeof(data), "SiteSurvey=%s", ssid);

		wrq.u.data.pointer = (void *) data;
		wrq.u.data.length = strlen(data);
		wrq.u.data.flags = 0;
		
		rc = ioctl(sock, RTPRIV_IOCTL_SET, &wrq);
		if ( rc == -1 )
		{
			WIFI_LOG(WIFI_DEBUG, ("ioctl fail: %s(%d): %s", __FILE__, __LINE__, strerror(errno)));
			close( sock );
			return NET_WLAN_ERROR;
		}
		
		close( sock );
	}
        snprintf (ch_iwpriv_cmd, sizeof (ch_iwpriv_cmd), "/3rd/bin/wpa_supplicant/common/iwpriv ra0 set SiteSurvey=\"%s\"", ssid);
#else
        snprintf (ch_iwpriv_cmd, sizeof (ch_iwpriv_cmd), "iwpriv ra0 set SiteSurvey=\"%s\"", ssid);
#endif
        if (doScriptCmd (ch_iwpriv_cmd) != 0 )
        {
            WIFI_LOG(WIFI_ERROR, ("fail to system iwpriv ssid for ra0\n"));
            return NET_WLAN_ERROR;
        }

    }
    else if (0 == strncmp (get_if_name(), "wlan0", strlen ("wlan0")))
    {
        char cmd[100]={0};
        char reply[256]= {'\0'};
        int i4Ret=NET_WLAN_ERROR;
        snprintf(cmd, sizeof(cmd), "SCAN ssid %s", ssid);
        if (doCommand(cmd, reply, sizeof(reply)) != 0)
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

    }
    usleep(10000);
    return NET_WLAN_OK;

}

INT32 x_net_wlan_get_scan_result(NET_802_11_SCAN_RESULT_T *pScanResult)
{
    if (wlan_check_init_state() != NET_WLAN_OK)
    {
        return NET_WLAN_NOT_INIT;
    }

    size_t  reply_len = 1024*12;
    char    *reply = NULL;

    reply = (char *)malloc(reply_len * sizeof(char));
    if (reply == NULL)
    {
        WIFI_LOG(WIFI_ERROR, ("malloc memory failed!%s, %d", __FUNCTION__, __LINE__));
        return NET_WLAN_ERROR;
    }

    memset( reply, 0, reply_len );

    if (doStringCommand("SCAN_RESULTS", reply, &reply_len) == NULL)
    {
        free(reply);
        return NET_WLAN_ERROR;
    }

    WIFI_LOG(WIFI_INFO, ("========= Scan Result ========="));
    WIFI_LOG(WIFI_INFO, ( "%s", reply ));

    if (wpa_get_scan_results_from_ret_buf(reply,pScanResult) < 0)
    {
        free(reply);
        return NET_WLAN_DRV_INTERFACE_ERR;
    }
    free(reply);
    return NET_WLAN_OK;

}
static NET_802_11_ESS_LIST_T *pEssList_last = NULL;

INT32 x_net_wlan_get_ess_list(NET_802_11_ESS_LIST_T *pEssList_result)
{
    INT32 i, j, ret;
    INT32 new_ess;
    UINT32 ess_size,num;
    NET_802_11_SCAN_RESULT_T rBssList, *pBssList;
    NET_802_11_BSS_INFO_T *pBssInfo = NULL;
    NET_802_11_ESS_LIST_T *pEssList, *tmp;
    NET_802_11_ESS_INFO_T *pEssInfo = NULL;


    if (pEssList_result == NULL)
    {
        return -1;
    }

    ret = x_net_wlan_get_scan_result(&rBssList);
    if (ret)
        return ret;
    pBssList = &rBssList;

    if ((pBssList->ui4_NumberOfItems == 0) ||
            (pBssList->p_BssInfo == NULL))
    {
        pEssList_result->ui4_NumberOfItems = 0;//haifei add for tell uper layer current esslist is empty.
        WIFI_LOG(WIFI_DEBUG , ("ESS list is empty"));
        return 0;
    }
    ess_size = (pBssList->ui4_NumberOfItems * sizeof(NET_802_11_ESS_INFO_T) +
                sizeof(NET_802_11_ESS_LIST_T));
    pEssList = malloc(ess_size);
    if (pEssList == NULL)
    {
        WIFI_LOG(WIFI_DEBUG , ("Failed to allocate memory for ESS list"));
        return -1;
    }
    memset(pEssList,0,ess_size);
    pEssList->p_EssInfo = (NET_802_11_ESS_INFO_T*)((UINT32)pEssList + sizeof(NET_802_11_ESS_LIST_T));

    pEssList->ui4_NumberOfItems = 0;
    for (i = 0; i < pBssList->ui4_NumberOfItems; i++)
    {
        new_ess = TRUE;
        pBssInfo = (NET_802_11_BSS_INFO_T *) & pBssList->p_BssInfo[i];
        for (j = 0; j < pEssList->ui4_NumberOfItems; j++)
        {
            pEssInfo = (NET_802_11_ESS_INFO_T *) & pEssList->p_EssInfo[j];
            if (pBssInfo->t_Ssid.ui4_SsidLen &&
                    (pBssInfo->t_Ssid.ui4_SsidLen == pEssInfo->t_Ssid.ui4_SsidLen) &&
                    (memcmp(pBssInfo->t_Ssid.ui1_aSsid,
                            pEssInfo->t_Ssid.ui1_aSsid,
                            pBssInfo->t_Ssid.ui4_SsidLen) == 0))
            {
                if (pBssInfo->i2_Level > pEssInfo->i2_Level)
                {
                    /* replace with a stronger BSS */
                    memcpy(pEssInfo->t_Bssid, pBssInfo->t_Bssid, ETH_ALEN);
                    pEssInfo->e_AuthMode = pBssInfo->e_AuthMode;
                    pEssInfo->e_AuthCipher = pBssInfo->e_AuthCipher;
                    pEssInfo->i2_Level = pBssInfo->i2_Level;
                    pEssInfo->is_wps_support = pBssInfo->is_wps_support;
                }
                new_ess = FALSE;
                break;
            }
        }

        if (new_ess)
        {
            /* copy the BSS to ESS list */
            pEssInfo = (NET_802_11_ESS_INFO_T *) & pEssList->p_EssInfo[pEssList->ui4_NumberOfItems];
            pEssList->ui4_NumberOfItems ++;

            memcpy(pEssInfo->t_Bssid, pBssInfo->t_Bssid, ETH_ALEN);
            pEssInfo->t_Ssid.ui4_SsidLen = pBssInfo->t_Ssid.ui4_SsidLen;
            strncpy(pEssInfo->t_Ssid.ui1_aSsid,
                    pBssInfo->t_Ssid.ui1_aSsid,
                    pBssInfo->t_Ssid.ui4_SsidLen);
            pEssInfo->e_AuthMode = pBssInfo->e_AuthMode;
            pEssInfo->e_AuthCipher = pBssInfo->e_AuthCipher;
            pEssInfo->i2_Level = pBssInfo->i2_Level;
            pEssInfo->is_wps_support = pBssInfo->is_wps_support;
        }
    }

    /* Free unneeded memory for unused ess list entries */
    num = pEssList->ui4_NumberOfItems;
    tmp = (NET_802_11_ESS_LIST_T *)realloc(pEssList, num * sizeof(NET_802_11_ESS_INFO_T) +
                                           sizeof(NET_802_11_ESS_LIST_T));
    if (tmp)
    {
        pEssList = tmp;
    }

    qsort(pEssList->p_EssInfo, num, sizeof(NET_802_11_ESS_INFO_T),
          supc_ess_list_compar);
    memcpy(pEssList_result, pEssList, sizeof(NET_802_11_ESS_LIST_T));
    if (pEssList_last != NULL)
    {
        free(pEssList_last);
        pEssList_last = NULL;
    }
    pEssList_last = pEssList;

    return NET_WLAN_OK;

}

#ifdef DISABLE_TV_WORLD_NW_SETTING
#define WLAN_STATUS_PATH "/tmp/wlanstatus.txt"
#define WLAN_DISCONNECT_PATH "/tmp/wlandisconnect.txt"
extern char   if_name[WLAN_WLAN_IF_MAX_LEN];
#endif

INT32 x_net_wlan_disassociate(VOID)
{
#ifndef DISABLE_TV_WORLD_NW_SETTING
    if (wlan_check_init_state() != NET_WLAN_OK)
        return NET_WLAN_NOT_INIT;

    if (!doBooleanCommand("DISCONNECT", "OK"))
    {
        WIFI_LOG(WIFI_ERROR ,(">>>>>>> %s,%d fail,\n",__FUNCTION__,__LINE__));
        return NET_WLAN_ERROR;
    }
#else

    char cmdStr[256]= {0};
    FILE* fp = NULL;
    snprintf(cmdStr,sizeof(cmdStr), "wpa_cli -iwlan0 disconnect > %s;sync;sync;", WLAN_DISCONNECT_PATH);
    doScriptCmd(cmdStr);
    memset(cmdStr, 0, sizeof(cmdStr));

    fp = fopen(WLAN_DISCONNECT_PATH, "r");
    if (!fp)
    {
        WIFI_LOG(WIFI_ERROR,("open disconnect file error!"));
        return -1;
    }
    fgets(cmdStr, sizeof(cmdStr), fp);
    fclose(fp);
    if (strstr(cmdStr,"OK") == NULL)
    {
        return NET_WLAN_ERROR;
    }
#endif
    return NET_WLAN_OK;

}

INT32 x_net_wlan_wpa_signal_poll(SIGNAL_DATA_T *pSignal)
{
    WIFI_LOG_ENTRY(WIFI_DEBUG, (""));

    char    cmd[32] = {'\0'};
    char    reply[256] = {'\0'};
    size_t  replylen = sizeof(reply);
    int     _tmp_rssi = -1;
    int     _tmp_linkspeed = -1;
    int     _tmp_noise = -1;
    int     _tmp_freq = -1;
    char    *pos = NULL;
    char    *end = NULL;

    if ( !pSignal )
    {
        WIFI_LOG(WIFI_ERROR,("Invalid parameter, so return!"));
        return NET_WLAN_ERROR;
    }

    snprintf(cmd, sizeof(cmd), "SIGNAL_POLL");
    if (doStringCommand(cmd, reply, &replylen) == NULL)
    {
        WIFI_LOG(WIFI_ERROR,("Failed to run cmd 'signal_poll'"));
        return NET_WLAN_ERROR;
    }

    WIFI_LOG(WIFI_INFO,("signal_poll info=%s", reply));

    pos = reply;
    while ( pos && *pos )
    {
        end = strchr(pos, '\n');
        if ( end )
            *end++ = '\0';

        if ( strncmp(pos, "RSSI=", strlen("RSSI=")) == 0 )
        {
            pos = pos + strlen("RSSI=");
            _tmp_rssi = atoi(pos);
        }
        else if ( strncmp(pos, "LINKSPEED=", strlen("LINKSPEED=")) == 0 )
        {
            pos = pos + strlen("LINKSPEED=");
            _tmp_linkspeed = atoi(pos);
        }
        else if ( strncmp(pos, "NOISE=", strlen("NOISE=")) == 0 )
        {
            pos = pos + strlen("NOISE=");
            _tmp_noise = atoi(pos);
        }
        else if ( strncmp(pos, "FREQUENCY=", strlen("FREQUENCY=")) == 0 )
        {
            pos = pos + strlen("FREQUENCY=");
            _tmp_freq = atoi(pos);
        }
        else
        {
            WIFI_LOG(WIFI_ERROR,("Not recongnize info=%s", pos));
        }

        pos = end;
    }

    WIFI_LOG(WIFI_INFO,("'signal_poll' result: rssi=%d, linkspeed=%d, noise=%d, freq=%d",
        _tmp_rssi, _tmp_linkspeed, _tmp_noise, _tmp_freq));

    _tmp_rssi = u1WlanDrv_NormalizeSignal(_tmp_rssi);

    pSignal->rssi = _tmp_rssi;
    pSignal->linkSpeed = _tmp_linkspeed;
    pSignal->noise = _tmp_noise;
    pSignal->freq = _tmp_freq;

    return 0;
}


#define WLAN_IWPRIV_STATUS_PATH "/tmp/iwprivstatus.txt"

INT32 x_net_wlan_get_curr_bss(NET_802_11_BSS_INFO_T *pCurrBss)
{
#ifndef DISABLE_TV_WORLD_NW_SETTING
    if (wlan_check_init_state() != NET_WLAN_OK)
        return NET_WLAN_NOT_INIT;
#endif
    WIFI_LOG_ENTRY(WIFI_API,(""));

    char reply[2048] = {0};
    char cmdStr[256]= {0};
    NET_802_11_BSS_INFO_T _currBssInfo;
#ifndef DISABLE_TV_WORLD_NW_SETTING
    size_t reply_len = sizeof(reply);

    if (doStringCommand("STATUS", reply, &reply_len) == NULL)
        return NET_WLAN_ERROR;
#else
    FILE* fp = NULL;
    snprintf(cmdStr, sizeof(cmdStr),"wpa_cli -iwlan0 status > %s;sync;sync;", WLAN_STATUS_PATH);
    doScriptCmd(cmdStr);

    fp = fopen(WLAN_STATUS_PATH, "r");
    if (!fp)
    {
        WIFI_LOG(WIFI_ERROR,("open status file error!"));
        return -1;
    }

    int read_num = fread(reply, 1,sizeof(reply), fp);

    fclose(fp);
    if (read_num <= 0)
    {
        return -1;
    }
//    memset(if_name , 0 , WLAN_WLAN_IF_MAX_LEN);
//    strncpy (if_name, "ra0", sizeof ("ra0"));
#endif
    memset(&_currBssInfo, 0, sizeof(_currBssInfo));
    if (wpa_get_status_from_ret_buf(reply, &_currBssInfo) < 0)
    {
        return NET_WLAN_DRV_INTERFACE_ERR;
    }
#if 1
    FILE* fp_iw = NULL;
    snprintf(cmdStr, sizeof(cmdStr),"iwpriv %s connStatus > %s;sync;sync;", get_if_name(),WLAN_IWPRIV_STATUS_PATH);
    doScriptCmd(cmdStr);

    fp_iw = fopen(WLAN_IWPRIV_STATUS_PATH, "r");
    if (!fp_iw)
    {
        WIFI_LOG(WIFI_DEBUG,("open status file error!"));
        return -1;
    }
    memset(reply,0,sizeof(reply));
    fgets(reply, sizeof(reply), fp_iw);

    WIFI_LOG(WIFI_DEBUG ,("iwpriv status: %s",reply));
    if (fp_iw)
    {
        fclose(fp_iw);
    }

    char *status = NULL;
    status = strstr(reply,"connStatus:");
    if (status)
    {
        WIFI_LOG(WIFI_DEBUG ,("iwpriv status: %s",status));
        if (strncmp(status+11,"Connected",9) == 0)
        {
            WIFI_LOG(WIFI_DEBUG ,("iwpriv status: WLAN_WPA_COMPLETED"));
            _currBssInfo.u8_wpa_status = WLAN_WPA_COMPLETED;
        }
        else if (strncmp(status+11,"Disconnect",10) == 0)
        {
            WIFI_LOG(WIFI_DEBUG ,("iwpriv status: WLAN_WPA_DISCONNECTED"));
            _currBssInfo.u8_wpa_status = WLAN_WPA_DISCONNECTED;
        }
    }
#endif

    if ( _currBssInfo.i2_Freq <= 0 || 
        _currBssInfo.i2_Channel <= 0 ||
        _currBssInfo.i2_Level <= 0 )
    {
        SIGNAL_DATA_T signalDate;
        memset(&signalDate, 0, sizeof(signalDate));
        x_net_wlan_wpa_signal_poll(&signalDate);
        _currBssInfo.i2_Freq = signalDate.freq;
        _currBssInfo.i2_Channel = frequency_to_channel(signalDate.freq);
        _currBssInfo.i2_Level = signalDate.rssi;
		_currBssInfo.i4_MaxRate = signalDate.linkSpeed * 1024; //linkSpeed is current rate(units: Mbps)
    }

    *pCurrBss = _currBssInfo;

    return NET_WLAN_OK;

}

#define WLAN_IWPRIV_STAT_RSSI_PATH "/tmp/iwprivstatrssi.txt"

INT32 x_net_wlan_get_rssi(NET_802_11_WIFI_RSSI_T *pRssi)
{

    WIFI_LOG_ENTRY(WIFI_DEBUG, (""));
    INT32 i4Ret = NET_WLAN_OK;
    //for vizio

    char cmd[128] = {"\0"};
    char reply[128] = {"\0"};
    FILE *fp = NULL;
    char *p = NULL;
    char *p2 = NULL;
    char *p3 = NULL;
    char _tmp[10];
    //NET_802_11_WIFI_RSSI_T *pCurrBss;

    pRssi->i4Rssi_a = 0;
    pRssi->i4Rssi_b = 0;
    pRssi->i4Rssi_c = 0;
    /* iwpriv ra0 stat*/
#if PLATFORM_IS_TV
    snprintf(cmd, sizeof(cmd), "/3rd/bin/wpa_supplicant/common/iwpriv %s stat > %s;sync;sync", get_if_name(),WLAN_IWPRIV_STAT_RSSI_PATH);
#else
    snprintf(cmd, sizeof(cmd), "iwpriv %s stat > %s;sync;sync",get_if_name(),WLAN_IWPRIV_STAT_RSSI_PATH);
#endif
    doScriptCmd(cmd);
    fp = fopen(WLAN_IWPRIV_STAT_RSSI_PATH, "r");
    if (!fp)
    {
        WIFI_LOG(WIFI_DEBUG,("open stat rssi file error!"));
        return -1;
    }
    memset(reply,0,sizeof(reply));
    if ( 0 == strncmp(get_if_name(), "wlan0", strlen ("wlan0")) ||
        0 == strncmp(get_if_name(), "ra0", strlen ("ra0")) )
    {
        while ( fgets(reply, sizeof(reply), fp))
        {
            p=strstr(reply,"RSSI");
            if (!p)
                continue;
            p=strstr(reply,"-");
            if (p)
            {
                p2=strstr(p," ");
                if (p2)
                {
                    strncpy(_tmp,p,p2-p);
                    pRssi->i4Rssi_a=strtol(_tmp ,NULL ,10);
                }
            }
            p=p2+1;
            if (p)
            {
                p2=strstr(p," ");
                if (p2)
                {
//	  WIFI_LOG(WIFI_DEBUG ,("p=%0x p2=%0x p2-p1=%0x",p, p2,p2-p));
                    strncpy(_tmp,p,p2-p);
                    pRssi->i4Rssi_b=strtol(_tmp ,NULL ,10);
                }
            }
            p=p2+1;
            p2=p;
            if (p)
            {
                while (*p2!='\0' && *p2!=' ')
                    p2++;
                if (p2)
                {
//	  WIFI_LOG(WIFI_DEBUG ,("p=%0x p2=%0x p2-p1=%0x",p, p2,p2-p));
                    strncpy(_tmp,p,p2-p);
                    pRssi->i4Rssi_c=strtol(_tmp ,NULL ,10);
                    p=NULL;
                    p2=NULL;
                    break;
                }
            }
            break;
        }
    }
    else
    {
        while ( fgets(reply, sizeof(reply), fp))
        {
            p=strstr(reply,"RSSI-A");
            p2=strstr(reply,"RSSI-B");
            p3=strstr(reply,"RSSI-C");
            if (!p && !p2 && !p3)
                continue;
            if (p)
            {
                p=p+6;   //length of("RSSI-A")
                p=strstr(p,"-");
                if (p)
                {
                    strncpy(_tmp,p,sizeof(_tmp));
                    pRssi->i4Rssi_a=strtol(_tmp ,NULL ,10);
                    p=NULL;
                }
            }
            if (p2)
            {
                p2=p2+6;  //length of("RSSI-B")
                p2=strstr(p2,"-");
                if (p2)
                {
                    strncpy(_tmp,p2,sizeof(_tmp));
                    pRssi->i4Rssi_b=strtol(_tmp ,NULL ,10);
                    p2=NULL;
                }
            }
            if (p3)
            {
                p3=p3+6;   //length of("RSSI-C")
                p3=strstr(p3,"-");
                if (p3)
                {
                    strncpy(_tmp,p3,sizeof(_tmp));
                    pRssi->i4Rssi_c=strtol(_tmp ,NULL ,10);
                    p3=NULL;
                    break;
                }
            }
        }
    }

    WIFI_LOG(WIFI_DEBUG ,("iwpriv RSSI = %d %d %d",pRssi->i4Rssi_a, pRssi->i4Rssi_b,pRssi->i4Rssi_c));
    if (fp)
    {
        fclose(fp);
    }

    return i4Ret;
}


INT32 _get_STA_bandwidth(INT32 *bw)
{
    WIFI_LOG_ENTRY(WIFI_DEBUG, (""));

    char _cmd_buf[256] = {0};
    int i = 0;
    FILE *fp = NULL;
    char *pbw = NULL;

    if (bw == NULL)
    {
        WIFI_LOG(WIFI_DEBUG, ("Invalid parameter, failed get bandwidth!"));
        return -1;
    }

    *bw = 0;

    memset(_cmd_buf, 0, sizeof(_cmd_buf));
#if PLATFORM_IS_TV
    snprintf(_cmd_buf,sizeof(_cmd_buf), "/3rd/bin/wpa_supplicant/common/iwpriv %s show HtBw",get_if_name());
#else
    snprintf(_cmd_buf,sizeof(_cmd_buf), "iwpriv %s show HtBw",get_if_name());
#endif
    fp = popen(_cmd_buf, "r");
    if (!fp)
    {
        WIFI_LOG(WIFI_DEBUG, ("%s excute failed, can not get bandwidth!", _cmd_buf));
        return -1;
    }

    memset(_cmd_buf, 0, sizeof(_cmd_buf));
    i = fread(_cmd_buf, sizeof(char), 256, fp);
    WIFI_LOG(WIFI_DEBUG, ("read %d bytes, read results: %s\n", i, _cmd_buf));

    if (fp != NULL)
    {
        pclose(fp);
        fp = NULL;
    }

    /* ra0  show:   20 MHz */
    pbw = strstr(_cmd_buf, "20");
    if (pbw != NULL)
    {
        *bw = 20;
        WIFI_LOG(WIFI_DEBUG, ("get bandwidth = 20MHz."));
    }
    else
    {
        pbw = strstr(_cmd_buf, "40");
        if (pbw != NULL)
        {
            *bw = 40;
            WIFI_LOG(WIFI_DEBUG, ("get bandwidth = 40MHz."));
        }
        else
        {
            WIFI_LOG(WIFI_DEBUG, ("Failed get bandwidth!"));
            return -1;
        }
    }

    return 0;
}
INT32 _wpa_determine_wireless_type(INT32 freq, INT32 bw, char *bitrate_str, NET_802_11_T *wireless_type)
{
    WIFI_LOG_ENTRY(WIFI_DEBUG, ("freq=%d, bw=%d, bitrate=%s", freq, bw, bitrate_str));
    char bitrate[8] = {'\0'};
    char *p = NULL;
    int i = 0;
    int bitrate_len = 0;

    p = bitrate_str;
    for (; i<sizeof(bitrate) - 1; i++)
    {
        if ((*p >= '0' && *p <= '9') || (*p == '.') )
        {
            bitrate[i] = *p++;
        }
        else
        {
            break;
        }
    }
    bitrate_len = strlen(bitrate);

    WIFI_LOG(WIFI_DEBUG, ("bitrate=%s, bitrate_len=%d\n", bitrate, bitrate_len));

    if (bw == 20)
    {
        /* 802.11b (2.4G) */
        if ((strncmp(bitrate, "1", bitrate_len) == 0
                || strncmp(bitrate, "2", bitrate_len) == 0
                || strncmp(bitrate, "5.5", bitrate_len) == 0
                || strncmp(bitrate, "11", bitrate_len) == 0)
                && (freq == 2))
        {
            *wireless_type = OID_802_11_B;
            WIFI_LOG(WIFI_DEBUG, ("connection wireless type = %d (802.11b)", *wireless_type));
            return 0;
        }

        /* 802.11a (5G) */
        if ((strncmp(bitrate, "6", bitrate_len) == 0
                || strncmp(bitrate, "9", bitrate_len) == 0
                || strncmp(bitrate, "12", bitrate_len) == 0
                || strncmp(bitrate, "18", bitrate_len) == 0
                || strncmp(bitrate, "24", bitrate_len) == 0
                || strncmp(bitrate, "36", bitrate_len) == 0
                || strncmp(bitrate, "48", bitrate_len) == 0
                || strncmp(bitrate, "54", bitrate_len) == 0)
                && (freq == 5))
        {
            *wireless_type = OID_802_11_A;
            WIFI_LOG(WIFI_DEBUG, ("connection wireless type = %d (802.11a)", *wireless_type));
            return 0;
        }

        /* 802.11g (2.4G) */
        if ((strncmp(bitrate, "1", bitrate_len) == 0
                || strncmp(bitrate, "2", bitrate_len) == 0
                || strncmp(bitrate, "6", bitrate_len) == 0
                || strncmp(bitrate, "9", bitrate_len) == 0
                || strncmp(bitrate, "12", bitrate_len) == 0
                || strncmp(bitrate, "18", bitrate_len) == 0
                || strncmp(bitrate, "24", bitrate_len) == 0
                || strncmp(bitrate, "36", bitrate_len) == 0
                || strncmp(bitrate, "48", bitrate_len) == 0
                || strncmp(bitrate, "54", bitrate_len) == 0)
                && (freq == 2))
        {
            *wireless_type = OID_802_11_G;
            WIFI_LOG(WIFI_DEBUG, ("connection wireless type = %d (802.11g)", *wireless_type));
            return 0;
        }

        /* 802.11n (2.4G or 5G) */
        if (strncmp(bitrate, "6.5", bitrate_len) == 0
                || strncmp(bitrate, "13", bitrate_len) == 0
                || strncmp(bitrate, "19.5", bitrate_len) == 0
                || strncmp(bitrate, "26", bitrate_len) == 0
                || strncmp(bitrate, "39", bitrate_len) == 0
                || strncmp(bitrate, "52", bitrate_len) == 0
                || strncmp(bitrate, "58.5", bitrate_len) == 0
                || strncmp(bitrate, "65", bitrate_len) == 0
                || strncmp(bitrate, "78", bitrate_len) == 0
                || strncmp(bitrate, "104", bitrate_len) == 0
                || strncmp(bitrate, "117", bitrate_len) == 0
                || strncmp(bitrate, "130", bitrate_len) == 0)
        {
            *wireless_type = OID_802_11_N;
            WIFI_LOG(WIFI_DEBUG, ("connection wireless type = %d (802.11n)", *wireless_type));
            return 0;
        }

        /* 802.11n (2.4G and 5G) */
        if (strncmp(bitrate, "7.2", bitrate_len) == 0
                || strncmp(bitrate, "14.4", bitrate_len) == 0
                || strncmp(bitrate, "21.7", bitrate_len) == 0
                || strncmp(bitrate, "28.9", bitrate_len) == 0
                || strncmp(bitrate, "43.3", bitrate_len) == 0
                || strncmp(bitrate, "57.8", bitrate_len) == 0
                || strncmp(bitrate, "65", bitrate_len) == 0
                || strncmp(bitrate, "72.2", bitrate_len) == 0
                || strncmp(bitrate, "86.7", bitrate_len) == 0
                || strncmp(bitrate, "115.6", bitrate_len) == 0
                //|| strncmp(bitrate_str, "130", 3) == 0
                || strncmp(bitrate, "144.4", bitrate_len) == 0)
        {
            *wireless_type = OID_802_11_N;
            WIFI_LOG(WIFI_DEBUG, ("connection wireless type = %d (802.11n)", *wireless_type));
            return 0;
        }
    }
    else if (bw == 40)
    {
        /* 802.11n (5G) */
        if (strncmp(bitrate, "13.5", bitrate_len) == 0
                || strncmp(bitrate, "27", bitrate_len) == 0
                || strncmp(bitrate, "40.5", bitrate_len) == 0
                || strncmp(bitrate, "54", bitrate_len) == 0
                || strncmp(bitrate, "81", bitrate_len) == 0
                || strncmp(bitrate, "108", bitrate_len) == 0
                || strncmp(bitrate, "121.5", bitrate_len) == 0
                || strncmp(bitrate, "135", bitrate_len) == 0
                || strncmp(bitrate, "162", bitrate_len) == 0
                || strncmp(bitrate, "216", bitrate_len) == 0
                || strncmp(bitrate, "243", bitrate_len) == 0
                || strncmp(bitrate, "270", bitrate_len) == 0)
        {
            *wireless_type = OID_802_11_N;
            WIFI_LOG(WIFI_DEBUG, ("connection wireless type = %d (802.11n)", *wireless_type));
            return 0;
        }

        /* 802.11n (2.4G and 5G) */
        if (strncmp(bitrate, "15", bitrate_len) == 0
                || strncmp(bitrate, "30", bitrate_len) == 0
                || strncmp(bitrate, "45", bitrate_len) == 0
                || strncmp(bitrate, "60", bitrate_len) == 0
                || strncmp(bitrate, "90", bitrate_len) == 0
                || strncmp(bitrate, "120", bitrate_len) == 0
                //|| strncmp(bitrate_str, "135", 3) == 0
                || strncmp(bitrate, "150", bitrate_len) == 0
                || strncmp(bitrate, "180", bitrate_len) == 0
                || strncmp(bitrate, "240", bitrate_len) == 0
                || strncmp(bitrate, "270", bitrate_len) == 0
                || strncmp(bitrate, "300", bitrate_len) == 0)
        {
            *wireless_type = OID_802_11_N;
            WIFI_LOG(WIFI_DEBUG, ("connection wireless type = %d (802.11n)", *wireless_type));
            return 0;
        }
    }
    else
    {
        WIFI_LOG(WIFI_DEBUG, ("Get wireless type failed."));
        return -1;
    }

    WIFI_LOG(WIFI_DEBUG, ("Get wireless type failed."));
    return -1;
}

/* Get RF band information *
** returns: 0x01: 2.4GHz, 0x02: 5GHz, 0x03: 2.4/5GHz, -1: Failure */
INT32 get_RF_band(void)
{
    WIFI_LOG_ENTRY(WIFI_API, (" "));
    int ret = -1;
    int i = 0, dongleNum = 0;
    int found = 0;
    int rfband=WPS_RF_24GHZ;
    char *dongleName[] = {"7601", "7603", "7650", "7662"};
    char cmd[BUF_LENTH] = {'\0'};

    dongleNum = sizeof(dongleName)/sizeof(char *);
    for (i = 0; i < dongleNum; i++)
    {
        memset(cmd, 0, sizeof(cmd));
        snprintf(cmd, sizeof(cmd), "iwpriv %s e2p 00 | grep \"%s\"",get_if_name(),dongleName[i]);
        ret = doScriptCmd(cmd);
        if (ret == 0)
        {
            if (strncmp(dongleName[i], "7601", strlen("7601")) == 0 ||
                    strncmp(dongleName[i], "7603", strlen("7603")) == 0)
            {
                rfband = WPS_RF_24GHZ;
                found = 1;
                WIFI_LOG(WIFI_INFO, ("Module is \"%s\", RF band is 2.4G.", dongleName[i]));
                break;
            }
            else if (strncmp(dongleName[i], "7650", strlen("7650")) == 0 ||
                    strncmp(dongleName[i], "7662", strlen("7662")) == 0)
            {
                rfband = WPS_RF_24GHZ | WPS_RF_50GHZ;
                found = 1;
                WIFI_LOG(WIFI_INFO, ("Module is \"%s\", RF band is 2.4G & 5G."));
                break;
            }
            else
            {
                WIFI_LOG(WIFI_DEBUG, ("Unknown dongle chip, return default value 2.4G."));
            }
        }
    }

    if ( found != 1 )
        WIFI_LOG(WIFI_DEBUG, ("Unknown RF band information, return default value 2.4G."));

    return rfband;
}


#if WLAN_MULTI_CREDENTIAL_SELECTION
/****************************************************************
** Because the "wpa_cli_recv_pending()" function is waiting for                  **
** scanning finished during multiple credential selection,                           **
** so "_wlan_msg_proc_thread" thread is blocked,                                    **
** it can't read "<3>CTRL-EVENT-SCAN-RESTULTS".                                 **
** This thread is used to read "<3>CTRL-EVENT-SCAN-RESTULTS" event     **
** when doing multiple credential selection.                                             **
*****************************************************************/
static void *_read_scan_result_event_thread_for_multi_cred(VOID * pvArg)
{
    WIFI_LOG(WIFI_API, ("Start!"));

    pthread_detach(pthread_self());

    prctl(PR_SET_NAME,"readscanres",0,0,0);
    int count = 2400;
    char buf[BUF_LENTH];
    size_t len = sizeof(buf) - 1;

    if (mon_conn == NULL)
    {
        WIFI_LOG(WIFI_DEBUG, (" ctrl_conn = NULL, exit thread!"));
        b_read_event_for_cred_started = FALSE;
        return NULL;
    }

    while (count--)
    {
        if (wpa_ctrl_pending(mon_conn) > 0)
        {
            memset(buf, 0, sizeof(buf));
            len = sizeof(buf) - 1;

            if (wpa_ctrl_recv(mon_conn, buf, &len) == 0)
            {
                buf[len] = '\0';

                WIFI_LOG(WIFI_DEBUG, ("Received event(length=%d): %s", len, buf));

                if (strncmp(buf+3, "CTRL-EVENT-SCAN-RESULTS", 22) == 0)
                {
                    if (b_start_scan_for_cred)
                    {
                        b_scan_completed = TRUE;
                        //break;
                    }
                }
            }
        }
        else
        {
            usleep(50*1000);
        }

        if (b_wps_cancel)
        {
            break;
        }

        if (b_stop_read_event)
        {
            break;
        }
    }

    WIFI_LOG(WIFI_DEBUG, ("Exit thread: %s", __FUNCTION__));
    b_read_event_for_cred_started = FALSE;
    return NULL;
}


/* Start "_read_scan_result_event_thread_for_multi_cred" thread */
INT32 start_read_scan_result_thread(void)
{
    WIFI_LOG(WIFI_API, (" "));

    if (b_read_event_for_cred_started)
    {
        WIFI_LOG(WIFI_DEBUG, ("=== Reading event thread for WPS multiple credential selection has been started! ==="));
        return 0;
    }

    pthread_t h_wlan_thread;

    if (pthread_create(&h_wlan_thread,
                       NULL,
                       _read_scan_result_event_thread_for_multi_cred,
                       NULL) != OSR_OK)
    {
        WIFI_LOG(WIFI_DEBUG, ("Thread create failed @ %s", __FUNCTION__));
        return (-1);
    }

    b_read_event_for_cred_started = TRUE;

    return 0;
}

/* Sort multiple credentials *
** according order fo encryption type: TKIP/AES > AES > TKIP > NONE > WEP > Unknown */
INT32 sort_multi_credentials(NET_802_11_ASSOCIATE_T *creds, int cred_num)
{
    int i = 0;
    int j = 0;

    WIFI_LOG_ENTRY(WIFI_API, ("Sort credentials according the order of Encryption Types: TKIP/AES > AES > TKIP > NONE > WEP > Unknown"));

    NET_802_11_ASSOCIATE_T cred_sorted[WLAN_MAX_WPS_CRED_NUM];
    memset(cred_sorted, 0, sizeof(NET_802_11_ASSOCIATE_T) * WLAN_MAX_WPS_CRED_NUM);

    if (cred_num > WLAN_MAX_WPS_CRED_NUM)
    {
        WIFI_LOG(WIFI_DEBUG, ("%s: Credentials number is too great, number=%d", __FUNCTION__, cred_num));
        cred_num = WLAN_MAX_WPS_CRED_NUM;
    }

    /* find all TKIP/AES */
    for (i = 0; i < cred_num; i++)
    {
        if (creds[i].e_AuthCipher == IEEE_802_11_AUTH_CIPHER_AES_OR_TKIP)
        {
            cred_sorted[j] = creds[i];
            j++;
        }
    }

    if (j < cred_num)
    {
        /* find all AES */
        for (i = 0; i < cred_num; i++)
        {
            if (creds[i].e_AuthCipher == IEEE_802_11_AUTH_CIPHER_AES)
            {
                cred_sorted[j] = creds[i];
                j++;
            }
        }
    }

    if (j < cred_num)
    {
        /* find all TKIP */
        for (i = 0; i < cred_num; i++)
        {
            if (creds[i].e_AuthCipher == IEEE_802_11_AUTH_CIPHER_TKIP)
            {
                cred_sorted[j] = creds[i];
                j++;
            }
        }
    }

    if (j < cred_num)
    {
        /* find all NONE */
        for (i = 0; i < cred_num; i++)
        {
            if (creds[i].e_AuthCipher == IEEE_802_11_AUTH_CIPHER_NONE)
            {
                cred_sorted[j] = creds[i];
                j++;
            }
        }
    }

    if (j < cred_num)
    {
        /* find all WEP */
        for (i = 0; i < cred_num; i++)
        {
            if (creds[i].e_AuthCipher == IEEE_802_11_AUTH_CIPHER_WEP40 ||
                    creds[i].e_AuthCipher == IEEE_802_11_AUTH_CIPHER_WEP104)
            {
                cred_sorted[j] = creds[i];
                j++;
            }
        }
    }

    if (j < cred_num)
    {
        /* find all Unknowns */
        for (i = 0; i < cred_num; i++)
        {
            if (creds[i].e_AuthCipher != IEEE_802_11_AUTH_CIPHER_AES_OR_TKIP &&
                    creds[i].e_AuthCipher != IEEE_802_11_AUTH_CIPHER_AES &&
                    creds[i].e_AuthCipher != IEEE_802_11_AUTH_CIPHER_TKIP &&
                    creds[i].e_AuthCipher != IEEE_802_11_AUTH_CIPHER_NONE &&
                    creds[i].e_AuthCipher != IEEE_802_11_AUTH_CIPHER_WEP40 &&
                    creds[i].e_AuthCipher != IEEE_802_11_AUTH_CIPHER_WEP104)
            {
                cred_sorted[j] = creds[i];
                j++;
            }
        }
    }

    memcpy(creds, cred_sorted, sizeof(NET_802_11_ASSOCIATE_T) * WLAN_MAX_WPS_CRED_NUM);

    WIFI_LOG(WIFI_INFO, ("====== Credentials sorted: ======"));
    for (i=0; i<cred_num; i++)
    {
        WIFI_LOG(WIFI_INFO, ("\n[Credential %d]: ssid=%s bssid=%02X:%02X:%02X:%02X:%02X:%02X auth=%d encrypt=%d key=%s rf_band=%d\n",
                             i,
                             creds[i].t_Ssid.ui1_aSsid, \
                             creds[i].t_Bssid[0], creds[i].t_Bssid[1], creds[i].t_Bssid[2], creds[i].t_Bssid[3], creds[i].t_Bssid[4], creds[i].t_Bssid[5], \
                             creds[i].e_AuthMode, \
                             creds[i].e_AuthCipher, \
                             creds[i].t_Key.key_body, \
                             creds[i].ui1_Priority));
    }

    return 0;
}


/* Select a credential from multiple credentials */
static INT32 wps_multi_credential_selection(NET_802_11_ASSOCIATE_T *select_creds, NET_802_11_ASSOCIATE_T *multi_creds, int cred_num)
{
    WIFI_LOG_ENTRY(WIFI_API, (" "));

    NET_802_11_SCAN_RESULT_T ScanResult;
    NET_802_11_ASSOCIATE_T tmp_cred;
    BOOL b_SSID_found = FALSE;
    INT32 rfband = WPS_RF_24GHZ;
    INT32 count = 0;
    INT32 ret = -1;
    int i,j;
    time_t starttime;
    time_t nowtime;
    UINT32 u4_scan_result_num;

    b_read_event_for_cred_started = FALSE;
    b_stop_read_event = FALSE;

    if (select_creds == NULL || multi_creds == NULL)
    {
        WIFI_LOG(WIFI_DEBUG, ("Invalid parameters!"));
        return -1;
    }

    /* Sort credentails */
    sort_multi_credentials(multi_creds, cred_num);

    time(&starttime);

    /* Get RF band */
    rfband = get_RF_band();

    while (1)
    {
        if (b_wps_cancel)
        {
            return -1;
        }

        /* Scan */
        if (!doBooleanCommand("SCAN", "OK"))
        {
            WIFI_LOG(WIFI_DEBUG, ("%s, Line:%d fail,\n",__FUNCTION__,__LINE__));
            //return NET_WLAN_ERROR;
        }
        b_start_scan_for_cred = TRUE;
        b_scan_completed = FALSE;

        /* Start a thread to check whether SCAN request has been finished */
        start_read_scan_result_thread();

        /* Waiting for scanning completed */
        count = 0;
        while (1)
        {
            if (b_wps_cancel)
            {
                return -1;
            }

            if (b_scan_completed)
            {
                break;
            }
            else
            {
                usleep(200*1000);

                count++;
                if (count >= 150)
                {
                    WIFI_LOG(WIFI_DEBUG, ("%s: scan timeout.", __FUNCTION__));
                    break;
                }
                WIFI_LOG(WIFI_DEBUG, ("%s: Waiting scan request completed......", __FUNCTION__));
            }
        }

        /* Get scan results */
        ret = x_net_wlan_get_scan_result(&ScanResult);
        b_start_scan_for_cred = FALSE;
        u4_scan_result_num = ScanResult.ui4_NumberOfItems;
        if (ret != 0 || ScanResult.ui4_NumberOfItems == 0)
        {
            WIFI_LOG(WIFI_DEBUG, ("Failed  to get scan results or there is no AP is scaned!"));
        }
        else
        {
            /* check if ssids in credentials are in the scan results */
            memset(&tmp_cred, 0, sizeof(NET_802_11_ASSOCIATE_T));
            tmp_cred.e_AuthMode = -1;
            tmp_cred.e_AuthCipher = -1;
            tmp_cred.ui1_Priority = WPS_RF_24GHZ;
            for (j=0; j < cred_num; j++)
            {
                for (i=0; i < u4_scan_result_num; i++)
                {
                    WIFI_LOG(WIFI_DEBUG, ("credential[%d] ssid=%s, scan result[%d] ssid=%s",
                                          j, multi_creds[j].t_Ssid.ui1_aSsid, i, ScanResult.p_BssInfo[i].t_Ssid.ui1_aSsid));

                    if (strncmp(multi_creds[j].t_Ssid.ui1_aSsid, ScanResult.p_BssInfo[i].t_Ssid.ui1_aSsid,
                                NET_802_11_MAX_LEN_SSID) == 0)
                    {
                        WIFI_LOG(WIFI_DEBUG, ("credential SSID found in scan results, credential RF band=%d, dongle RF band=%d", multi_creds[j].ui1_Priority, rfband));

                        switch (rfband)
                        {
                        case WPS_RF_24GHZ:
                            if (multi_creds[j].ui1_Priority == WPS_RF_50GHZ)
                            {
                                WIFI_LOG(WIFI_DEBUG, ("RF band is not matched, credential RF band=%d, dongle RF band=%d", multi_creds[j].ui1_Priority, rfband));
                                continue;
                            }
                            break;
                        case WPS_RF_50GHZ:
                            if (multi_creds[j].ui1_Priority == WPS_RF_24GHZ)
                            {
                                WIFI_LOG(WIFI_DEBUG, ("RF band is not matched, credential RF band=%d, dongle RF band=%d", multi_creds[j].ui1_Priority, rfband));
                                continue;
                            }
                            break;
                        default:
                            break;
                        }

                        WIFI_LOG(WIFI_DEBUG, ("RF band matched!Credential[%d] will be stored, ssid=%s, auth mode=%d, cipher mode=%d, RF band=%d",
                                              j, multi_creds[j].t_Ssid.ui1_aSsid, multi_creds[j].e_AuthMode, multi_creds[j].e_AuthCipher, multi_creds[j].ui1_Priority));
                        WIFI_LOG(WIFI_DEBUG, ("Current stored credential ssid=%s, auth mode=%d, cipher mode=%d, RF band=%d",
                                              tmp_cred.t_Ssid.ui1_aSsid, tmp_cred.e_AuthMode, tmp_cred.e_AuthCipher, tmp_cred.ui1_Priority));

                        if ((int)(multi_creds[j].e_AuthMode) > (int)(tmp_cred.e_AuthMode))
                        {
                            WIFI_LOG(WIFI_DEBUG, ("Credential[%d]'s auth mode is higher than current credential's. cred auth=%d, current auth=%d", j, multi_creds[j].e_AuthMode, tmp_cred.e_AuthMode));
                            tmp_cred = multi_creds[j];
                            WIFI_LOG(WIFI_DEBUG, ("Credential[%d] has been stored, ssid=%s, auth mode=%d, cipher mode=%d, RF band=%d",
                                                  j, tmp_cred.t_Ssid.ui1_aSsid, tmp_cred.e_AuthMode, tmp_cred.e_AuthCipher, tmp_cred.ui1_Priority));
                        }

                        b_SSID_found = TRUE;
                    }
                }
            }

            if (b_SSID_found)
            {
                WIFI_LOG(WIFI_DEBUG, ("ssid in mutiple credentials has been found in scan results!"));
                break;
            }
            else
            {
                WIFI_LOG(WIFI_DEBUG, ("ssid in mutiple credentials has not been found in scan results!"));
            }
        }

        time(&nowtime);
        if (nowtime - starttime > 60)
        {
            WIFI_LOG(WIFI_DEBUG, ("Select first credential that RF band is supported or RF band is not included."));

            /* Select first credential that RF band value is supported or RF band value is not included. */
            for (j=0; j < cred_num; j++)
            {
                if (rfband == WPS_RF_24GHZ) /* if dongle only support 2.4G */
                {
                    if (multi_creds[j].ui1_Priority != WPS_RF_50GHZ)    /* if credential's RF band is not for 5G */
                    {
                        tmp_cred = multi_creds[j];
                        break;
                    }
                }
                else if (rfband == WPS_RF_50GHZ)    /* if dongle only support 5G */
                {
                    if (multi_creds[j].ui1_Priority != WPS_RF_24GHZ)    /* if credential's RF band is not for 2.4G */
                    {
                        tmp_cred = multi_creds[j];
                        break;
                    }
                }
                else
                {
                    tmp_cred = multi_creds[j];
                    break;
                }
            }

            /* No available Credentials! */
            if (j >= cred_num)
            {
                WIFI_LOG(WIFI_DEBUG, ("===== No available Credentials! ====="));
                b_stop_read_event = TRUE;
                return -1;
            }

            WIFI_LOG(WIFI_DEBUG, ("Select credential[%d], ssid=%s, authmode=%d, ciphermode=%d, RF_band=%d",
                                  j, tmp_cred.t_Ssid.ui1_aSsid, tmp_cred.e_AuthMode, tmp_cred.e_AuthCipher, tmp_cred.ui1_Priority));

            break;
        }
        else
        {
            /* waiting for 5 seconds if need a new scanning */
            count = 0;
            struct timeval timeout = {0, 200000};
            while (1)
            {
                if (b_wps_cancel)
                {
                    return -1;
                }

                //usleep(200000);
                timeout.tv_sec = 0;
                timeout.tv_usec = 200*1000;
                select(0, NULL, NULL, NULL, &timeout);

                count++;
                if (count >= 25)
                {
                    WIFI_LOG(WIFI_DEBUG, ("%s: 5 seconds have elapsed, start scan again.", __FUNCTION__));
                    break;
                }
                WIFI_LOG(WIFI_DEBUG, ("%s: Waiting 5 seconds for scanning again......", __FUNCTION__));
            }
        }
    }

    WIFI_LOG(WIFI_DEBUG, ("Mutiple credentials selection completed!"));

    *select_creds = tmp_cred;
    b_stop_read_event = TRUE;

    WIFI_LOG(WIFI_DEBUG, ("===== selected credential: ====="));
    WIFI_LOG(WIFI_DEBUG, ("                 bssid=" MACSTR, MAC2STR(select_creds->t_Bssid)));
    WIFI_LOG(WIFI_DEBUG, ("                  ssid=%s", select_creds->t_Ssid.ui1_aSsid));
    WIFI_LOG(WIFI_DEBUG, ("                  AuthMode=%d", select_creds->e_AuthMode));
    WIFI_LOG(WIFI_DEBUG, ("                  Authcipher=%d", select_creds->e_AuthCipher));
    WIFI_LOG(WIFI_DEBUG, ("                  key=%s isAscii=%d\n", select_creds->t_Key.key_body, select_creds->t_Key.b_IsAscii));
    return 0;
}


/* Map credential to SONY's request */
static INT32 wps_get_cfg_from_cred(NET_802_11_ASSOCIATE_T *AP_Cfg, NET_802_11_ASSOCIATE_T *cred)
{
    WIFI_LOG_ENTRY(WIFI_API, (" "));

    if (AP_Cfg == NULL || cred == NULL)
    {
        WIFI_LOG(WIFI_DEBUG, ("Invalid parameters!"));
        return -1;
    }

    memset(AP_Cfg->t_Bssid, 0xff, ETH_ALEN);
    AP_Cfg->t_Ssid.ui4_SsidLen = cred->t_Ssid.ui4_SsidLen;
    strncpy(AP_Cfg->t_Ssid.ui1_aSsid, cred->t_Ssid.ui1_aSsid, NET_802_11_MAX_LEN_SSID);
    AP_Cfg->t_Ssid.ui1_aSsid[NET_802_11_MAX_LEN_SSID] = '\0';

    if (cred->e_AuthCipher == IEEE_802_11_AUTH_CIPHER_WEP40 ||
            cred->e_AuthCipher == IEEE_802_11_AUTH_CIPHER_WEP104)    /* WEP */
    {
        AP_Cfg->e_AuthMode = IEEE_802_11_AUTH_MODE_WEP;
        AP_Cfg->e_AuthCipher = IEEE_802_11_AUTH_CIPHER_WEP40;
    }
    else if (cred->e_AuthMode == IEEE_802_11_AUTH_MODE_NONE &&
             cred->e_AuthCipher == IEEE_802_11_AUTH_CIPHER_NONE)   /* NONE */
    {
        AP_Cfg->e_AuthMode = IEEE_802_11_AUTH_MODE_NONE;
        AP_Cfg->e_AuthCipher = IEEE_802_11_AUTH_CIPHER_NONE;
    }
    else    /* Shared(0x04); WPA-Enterprise(0x08); WPA2-Enterprise(0x10); WPA-PSK(0x02); WPS2-PSK(0x20); WPA_OR_WPA2-PSK(0x22) ...*/
    {
        AP_Cfg->e_AuthMode = IEEE_802_11_AUTH_MODE_WPAPSK_OR_WPA2PSK;
        AP_Cfg->e_AuthCipher = IEEE_802_11_AUTH_CIPHER_AES_OR_TKIP;
    }

    strncpy(AP_Cfg->t_Key.key_body, cred->t_Key.key_body, WLAN_KEY_MAX_LENGTH);
    AP_Cfg->t_Key.key_body[WLAN_KEY_MAX_LENGTH] = '\0';
    AP_Cfg->t_Key.pui1_PassPhrase = AP_Cfg->t_Key.key_body;
    if (strlen(cred->t_Key.key_body) == WLAN_KEY_MAX_LENGTH) /*64 hex characters key*/
    {
        AP_Cfg->t_Key.b_IsAscii = FALSE;
    }
    else
    {
        AP_Cfg->t_Key.b_IsAscii = TRUE;
    }

    WIFI_LOG(WIFI_DEBUG, ("===== get AP configration from credential: ====="));
    WIFI_LOG(WIFI_DEBUG, ("                     bssid=" MACSTR, MAC2STR(AP_Cfg->t_Bssid)));
    WIFI_LOG(WIFI_DEBUG, ("                     ssid=%s", AP_Cfg->t_Ssid.ui1_aSsid));
    WIFI_LOG(WIFI_DEBUG, ("                     AuthMode=%d", AP_Cfg->e_AuthMode));
    WIFI_LOG(WIFI_DEBUG, ("                     Authcipher=%d", AP_Cfg->e_AuthCipher));
    WIFI_LOG(WIFI_DEBUG, ("                     key=%s isAscii=%d\n", AP_Cfg->t_Key.key_body, AP_Cfg->t_Key.b_IsAscii));

    WIFI_LOG(WIFI_DEBUG, ("Leave %s()", __FUNCTION__));

    return 0;
}
#endif


