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
 * $RCSfile: c_net_p2p_api.c,v $
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
#include "../wlanCtrl/inc/includes.h"
#include "../wlanCtrl/inc/acfg_drv_if.h"
#include "../wlanCtrl/inc/wps_defs.h"

#include "../wlanCtrl/x_net_p2p_ctrl.h"
#include "../wlanCtrl/_net_wlan_init.h"
#include "../wlanCtrl/wifi_log.h"
#include "../wlanCtrl/wifi_print.h"
#include "../wlanCtrl/wifi_com.h"
#include "../wlanCtrl/wlan_command.h"
#include "../wlanCtrl/wifi_parse.h"
#if PLATFORM_IS_BDP
//#include "openssl/evp.h"
#endif



#define WLAN_GET_IFCONFIG_PATH   "/tmp/ifconfig.txt"
#ifdef WLAN_SUPPORT_P2P_CFG80211
#define CFG80211_WPA_SUPPLICANT_P2P_CONFIG_FILE  "/mnt/ubi_boot/P2P_DEV_CONF"
#endif
#define P2P_MAX_ASSOC_TAB         (3)
#define CMD_BUF_LEN 			  (100)

P2P_ASSOC_PER_ENTRY_T _p2p_assoc_per_entry[P2P_MAX_ASSOC_TAB];

extern BOOL p2p_for_wfd;
extern BOOL Is_for_listen;
extern BOOL fgProvisionReq ;
extern char wfd_connected_mac[18];
extern INT32 p2p_is_current_go ;
extern UINT32 wfd_p2p_port;
extern char str_ip[20];

extern x_net_wlan_notify_fct _p2p_cbk_list[MAX_CBK_NUM];

static BOOL _dhcpd_started = FALSE;
EEPROM_PER_ENTRY_T _per_entry[MAX_P2P_TABLE_SIZE];

extern struct stat _ori_stat;


extern char p2p_name[WLAN_WLAN_IF_MAX_LEN];
extern char p2p_go_name[WLAN_WLAN_IF_MAX_LEN];
extern char if_name[WLAN_WLAN_IF_MAX_LEN];

extern int u1WlanDrv_NormalizeSignal(int signal);

extern SEMA_HANDLE h_P2pCmdSema;
extern SEMA_HANDLE h_P2pNfySema;

const char *wfd_cmd_list[] =
{
    [WFD_P2P_CMD_DEV_TYPE]              = "wfd_devType"
    ,[WFD_P2P_CMD_SOURCE_COUPLED]       = "wfd_sourceCoup"
    ,[WFD_P2P_CMD_SINK_COUPLED]         = "wfd_sinkCoup"
    ,[WFD_P2P_CMD_SESSION_AVAILABLE]    = "wfd_sessionAvail"
    ,[WFD_P2P_CMD_RTSP_PORT]            = "wfd_rtspPort"
    ,[WFD_P2P_CMD_MAX_THROUGHPUT]       = "wfd_maxThroughput"
#if PLATFORM_IS_TV
    ,[WFD_P2P_CMD_DEVICE_NAME]          = "device_name"
#endif
// 	,[WFD_P2P_CMD_SET_P2P_OP_CHANNEL]   = "p2p_oper_channel"
    ,[WFD_P2P_CMD_SET_CONTENT_PROTECT]   = "wfd_contentProtect"
    ,[WFD_P2P_CMD_MAX]                  = NULL
};

#if PLATFORM_IS_TV
static INT32 c_net_wlan_get_mac4B(char *ifname, char *buf, int buf_len);
#endif
#if PLATFORM_IS_TV
INT32 c_net_p2p_isSupplicantReady()
{
    WIFI_LOG_ENTRY(WIFI_API, (""));
    int i4Ret = x_net_p2p_isSupplicantReady ();
    WIFI_CHK_FAIL (i4Ret);
    return i4Ret;
}


INT32 c_net_p2p_stop_supplicant(x_net_wlan_notify_fct pfNotify)//stop && close connection
{
    WIFI_LOG_ENTRY(WIFI_API, (""));

#ifdef SUPPORT_P2P_CONCURRENT
    int i4Ret = x_net_wlan_stop_driver (pfNotify);
#else
    int i4Ret = x_net_p2p_stop_supplicant (pfNotify);
#endif
    WIFI_CHK_FAIL (i4Ret);
    return i4Ret;
}

INT32 c_net_p2p_start_supplicant(x_net_wlan_notify_fct pfNotify)
{
    WIFI_LOG_ENTRY(WIFI_API, (""));
#ifdef SUPPORT_P2P_CONCURRENT
    int i4Ret = x_net_wlan_start_driver (pfNotify,1);
#else
    int i4Ret = x_net_p2p_start_supplicant (pfNotify);
#endif
    WIFI_CHK_FAIL (i4Ret);
    return i4Ret;
}
#endif

#if PLATFORM_IS_TV
INT32 c_net_p2p_get_current_ip_info(char *name, NW_IP_INFO_T* pt_ip_info)
{
    WIFI_LOG_ENTRY(WIFI_API, (""));
    int i4Ret = x_net_p2p_get_current_ip_info (name,pt_ip_info);
    WIFI_CHK_FAIL (i4Ret);
    return i4Ret;
}

INT32 c_net_p2p_start_dhcpc(char *name)
{
    WIFI_LOG_ENTRY(WIFI_API, (""));
    int i4Ret = x_net_p2p_start_dhcpc (name);
    WIFI_CHK_FAIL (i4Ret);
    return i4Ret;
}

INT32 c_net_p2p_restart_dhcpc(char *name)
{
    WIFI_LOG_ENTRY(WIFI_API, (""));
    int i4Ret = x_net_p2p_restart_dhcpc (name);
    WIFI_CHK_FAIL (i4Ret);
    return i4Ret;
}

INT32 c_net_p2p_stop_dhcpc(char *name)
{
    WIFI_LOG_ENTRY(WIFI_API, (""));
    int i4Ret = x_net_p2p_stop_dhcpc (name);
    WIFI_CHK_FAIL (i4Ret);
    return i4Ret;
}

#endif

INT32 c_net_wlan_p2p_reg_cbk(x_net_wlan_notify_fct pfNotify)
{
    WIFI_LOG_ENTRY(WIFI_API, (""));
    int i ;
    if (pfNotify == NULL )//NULL notify , ignore it .
    {
        return 0;
    }

    for ( i = 0; i < MAX_CBK_NUM ; i++)
    {
        if (_p2p_cbk_list[i] == pfNotify ) //alread in cbk list ,do not insert it again .
        {
            WIFI_LOG(WIFI_DEBUG ,(">>>>>>>[wifi] reg cbk notify function already in list!\n"));
            return 0;
        }
        if (_p2p_cbk_list[i] == NULL ) //instert it .
        {
            _p2p_cbk_list[i] = pfNotify;
            return 0;
        }
    }

    WIFI_LOG(WIFI_DEBUG ,("[WiFi MW]x_net_p2p_reg_cbk list is full ,max value is %d!\n",MAX_CBK_NUM));
    return (-1);

}
INT32 c_net_p2p_reg_cbk(x_net_wlan_notify_fct pfNotify)
{
    return c_net_wlan_p2p_reg_cbk(pfNotify);

}
INT32 c_net_p2p_unreg_cbk(x_net_wlan_notify_fct pfNotify)
{
    WIFI_LOG_ENTRY(WIFI_API, (""));
    int i ;
    if (pfNotify == NULL )//NULL notify , ignore it .
    {
        return 0;
    }
    for ( i = 0; i < MAX_CBK_NUM ; i++)
    {
        if (_p2p_cbk_list[i] == pfNotify ) //alread in cbk list ,do not insert it again .
        {
            WIFI_LOG(WIFI_DEBUG ,(">>>>>>>[wifi] unreg cbk notify function ok, removed!\n"));
            _p2p_cbk_list[i] = NULL;
            return 0;
        }
    }

    WIFI_LOG(WIFI_DEBUG,("[WiFi MW]x_net_p2p_reg_cbk list is full ,max value is %d!\n",MAX_CBK_NUM));
    return -1;

}

INT32 c_net_wlan_p2p_is_wfd_mode(void)
{
    WIFI_LOG_ENTRY(WIFI_API, (""));
    if ( p2p_for_wfd)
        return 0;
    return -1;
}

INT32 c_net_wlan_check_p2p_init_state(void)
{
    WIFI_LOG_ENTRY(WIFI_API, (""));
    return x_net_p2p_check_init_state ();
}

INT32 c_net_wlan_stop_p2p_dhcpd_sever(void)
{
    WIFI_LOG_ENTRY(WIFI_API, (""));

    if (_dhcpd_started != TRUE)
    {
        WIFI_LOG(WIFI_WARNING,("dhcp server has already stopped!\n"));
        return 0;
    }
    x_net_stop_dhcpd();
    x_net_wlan_clean_file(DHCPD_LEASE_FILE_PATH);
    _dhcpd_started = FALSE;
    return 0;

}
INT32 c_net_wlan_start_p2p_dhcpd_sever_only(char * dev , NET_DHCP_CONFIG dhcps_cfg )
{
    WIFI_LOG_ENTRY(WIFI_API, (""));
	char go_iface[32] = "p2p0";
	
    if (_dhcpd_started == TRUE)
    {
        WIFI_LOG(WIFI_WARNING,(" dhcp server has already started!\n"));
        return 0;
    }
    if (p2p_go_name[0] == 0)
        WIFI_LOG(WIFI_ERROR,("use default p2p iface name \"%s\" for dhcpd\n", go_iface));
	else
		strncpy(go_iface, p2p_go_name, sizeof(go_iface) - 1);
	
    doScriptCmd("rm /var/run/dhcpd.pid");

    char cmd[1024]={0};
    int ret =0;

#if 1//PLATFORM_IS_BDP temp mark for align branch 's dhcp config is not set to 192.168.211.161
    snprintf(cmd, 200, "ifconfig %s 192.168.211.161 netmask 255.255.255.224", p2p_go_name);
#else
    snprintf(cmd, 200, "ifconfig %s 192.168.5.1 netmask 255.255.255.0", p2p_name);
#endif

    ret |= doScriptCmd(cmd);
    if (ret != 0)
    {
        WIFI_LOG(WIFI_ERROR,("cmd %s return error, ret = %d\n",cmd,ret));
        return -1;
    }
    x_memset(cmd, 0, sizeof(cmd));
    x_net_wlan_clean_file(DHCPD_LEASE_FILE_PATH);
    snprintf(cmd , sizeof(cmd), "echo \"\"> %s ;%s -cf %s -lf %s %s",
             DHCPD_LEASE_FILE_PATH,
             DHCPD_EXEC_PATH,
             DHCPD_CONFIG_FILE_PATH,
             DHCPD_LEASE_FILE_PATH, p2p_go_name);
    WIFI_LOG (WIFI_DEBUG,("before:%s\n", cmd));

    ret = doScriptCmd(cmd);
    if (ret != 0)
    {
        WIFI_LOG (WIFI_ERROR,("system start dhcpd server error ,ret = %d,erorno = %d!\n",ret,errno));
        return -1;
    }
    WIFI_LOG (WIFI_DEBUG,("after:%s\n", cmd));
    _dhcpd_started = TRUE;

    return 0;
}

#if PLATFORM_IS_BDP
INT32 c_net_wlan_start_p2p_dhcpd_sever(char * dev , NET_DHCP_CONFIG dhcps_cfg)
{
    WIFI_LOG_ENTRY(WIFI_API, (""));
    c_net_wlan_stop_p2p_dhcpd_sever();
    return c_net_wlan_start_p2p_dhcpd_sever_only(dev,dhcps_cfg);
}
#else
INT32 c_net_wlan_start_p2p_dhcpd_sever(char * addr,char * mask )
{
    WIFI_LOG_ENTRY(WIFI_API, (""));
    if (_dhcpd_started == TRUE)
    {
        WIFI_LOG(WIFI_WARNING,(" dhcp server has already started!\n"));
        return 0;
    }
    if (p2p_name[0] == 0)
    {
        WIFI_LOG(WIFI_ERROR,("start dhcp server ,device not enabled!\n"));
        return -1;
    }

    doScriptCmd("rm /var/run/dhcpd.pid");

    char cmd[1024]={0};
    int ret =0;

#if PLATFORM_IS_BDP
    snprintf(cmd, 200, "ifconfig %s 192.168.211.161 netmask 255.255.255.224", p2p_go_name);
#else
    snprintf(cmd, 200, "ifconfig %s 192.168.179.1 netmask 255.255.255.0", p2p_go_name);
#endif

    ret |= doScriptCmd(cmd);
    if (ret != 0)
    {
        WIFI_LOG(WIFI_ERROR,("cmd %s return error, ret = %d\n",cmd,ret));
        return -1;
    }
    x_memset(cmd, 0, sizeof(cmd));
    x_net_wlan_clean_file(DHCPD_LEASE_FILE_PATH);
    snprintf(cmd , sizeof(cmd), "echo \"\"> %s ;%s -cf %s -lf %s %s",
             DHCPD_LEASE_FILE_PATH,
             DHCPD_EXEC_PATH,
             DHCPD_CONFIG_FILE_PATH,
             DHCPD_LEASE_FILE_PATH, p2p_go_name);
    WIFI_LOG (WIFI_DEBUG,("before:%s\n", cmd));

    ret = doScriptCmd(cmd);
    if (ret != 0)
    {
        WIFI_LOG (WIFI_ERROR,("system start dhcpd server error ,ret = %d,erorno = %d!\n",ret,errno));
        return -1;
    }
    WIFI_LOG (WIFI_DEBUG,("after:%s\n", cmd));
    _dhcpd_started = TRUE;
    return 0;
}
#endif

/* if current p2p module is for wfd , we need parse peer ip here. */
INT32 c_net_p2p_get_other_ip_withmac(const char* mac,char *IP, UINT32 *port)
{
    WIFI_LOG_ENTRY(WIFI_API, (""));

    if (IP == NULL || port == NULL)
    {
        WIFI_LOG (WIFI_WARNING,("the arguments are invalid.\n"));
        return -1;
    }

    const char * checkmac = NULL;
    if (mac == NULL)
    {
        checkmac = wfd_connected_mac;
    }
    else
    {
        WIFI_LOG (WIFI_DEBUG,("find ip of mac(%s)\n",mac));
        checkmac = mac;
    }

    INT32 ret = -1;
    if ( p2p_is_current_go )//current we are go , parse dhcpd.lease file.
    {
        char * ip = _get_ip_from_mac(checkmac, DHCPD_LEASE_FILE_PATH);
        if ( ip != NULL )
        {
            *port = wfd_p2p_port;
            memcpy(IP, ip, strlen(ip));
            IP[strlen(ip)] = '\0';
            WIFI_LOG (WIFI_DEBUG,("*** I am GO,peer GC Ip = %s, Port=%d ***\n", IP, (int)(*port)));
        }
    }
    else //we are client ,just read p2p_s_ip file .
    {
        struct stat _stat_now = {0};
        ret = get_file_stat(P2P_SERVER_IP_FILE_PATH,&_stat_now);
        if (ret == 0)
        {
            if (is_file_changed(_ori_stat,_stat_now));
            {
                memcpy(&_ori_stat , &_stat_now , sizeof(struct stat));
                FILE * fl = fopen(P2P_SERVER_IP_FILE_PATH ,"r");
                if (fl != NULL)
                {
                    char buf[100] = {0};
                    int read_num = fread(buf,1,100,fl);
                    if (read_num > 0 )
                    {

                        *port = wfd_p2p_port;
                        int buflen = strlen(buf);
                        int i = 0;
                        for (; i < buflen; i++)
                        {
                            if ((buf[i] >= '0' && buf[i] <= '9') || buf[i] == '.')
                            {
                                IP[i] = buf[i];
                            }
                            else
                            {
                                IP[i] = '\0';
                                break;
                            }
                        }
                        WIFI_LOG (WIFI_DEBUG,("*** I am GC,peer GO Ip = %s, Port=%d ***\n", IP, (int)(*port)));
                    }

                    if (fl)
                    {
                        fclose(fl);
                    }
                }
                else
                {
                    WIFI_LOG (WIFI_ERROR,("Open p2p server ip file failed!\n"));
                    return -1;
                }
            }
        }
    }

    return 0;
}
INT32 c_net_p2p_get_other_ip(char *IP, UINT32 *port)
{
    return c_net_p2p_get_other_ip_withmac(NULL,IP,port);
}
INT32 c_net_wlan_enable_wifi_direct(const char *ifname)
{
    WIFI_LOG_ENTRY(WIFI_API, (""));
#if PLATFORM_IS_BDP
    x_strncpy(p2p_name, ifname, strlen(ifname));
#endif
    int i4Ret = x_net_p2p_open_connection ();
    WIFI_CHK_FAIL (i4Ret);
    i4Ret = wifi_sema_init(&h_P2pCmdSema);
    WIFI_CHK_FAIL (i4Ret);
    i4Ret = wifi_sema_init(&h_P2pNfySema);
    WIFI_CHK_FAIL (i4Ret);

    return i4Ret;
}
INT32 c_net_p2p_open_connection(void)
{
    WIFI_LOG_ENTRY(WIFI_API, (""));
    int i4Ret = c_net_wlan_enable_wifi_direct("p2p0");
    WIFI_CHK_FAIL (i4Ret);
    return i4Ret;
}

INT32 c_net_p2p_send_to_socket(void)
{
    WIFI_LOG_ENTRY(WIFI_API, (""));
    int i4Ret = x_net_p2p_send_to_socket ();
    WIFI_CHK_FAIL (i4Ret);
    return i4Ret;
}


INT32 c_net_wlan_disable_wifi_direct(void)
{
    WIFI_LOG_ENTRY(WIFI_API, (""));
    if (x_net_check_p2p_thread_inited())
    {
        x_net_p2p_send_to_socket();
    }
    int i4Ret = x_net_p2p_close_connection ();
    WIFI_CHK_FAIL (i4Ret);

    x_net_stop_dhcpd();
    i4Ret = sema_uninit(&h_P2pCmdSema);
    WIFI_CHK_FAIL (i4Ret);
    i4Ret = sema_uninit(&h_P2pNfySema);
    WIFI_CHK_FAIL (i4Ret);

    return i4Ret;
}
INT32 c_net_p2p_close_connection(void)
{
    WIFI_LOG_ENTRY(WIFI_API, (""));
    int i4Ret = c_net_wlan_disable_wifi_direct();
    WIFI_CHK_FAIL (i4Ret);
    return i4Ret;
}

INT32 c_net_wlan_p2p_set_peer_ip(CHAR * ip , int len)
{
    WIFI_LOG_ENTRY(WIFI_API, (""));
    int i4Ret = x_net_wlan_p2p_set_peer_ip (ip,len);
    WIFI_CHK_FAIL (i4Ret);
    return i4Ret;
}

INT32 c_net_wlan_p2p_disable_wfd_mode(void)
{
    WIFI_LOG_ENTRY(WIFI_API, (""));

    if (0 == strncmp(if_name, "ra0", strlen ("ra0")))
    {
        char _cmd_buf[WFD_CMD_BUF_LEN] = {0};
        WIFI_LOG(WIFI_DEBUG, ("(wfddbg) disable WFD...\n"));
        snprintf(_cmd_buf,sizeof(_cmd_buf), "SET wfd_wfdEnable %d", WFD_CMD_VAL_DISABLE_WFD);
        if (strlen(_cmd_buf) && !dop2pBooleanCommand(_cmd_buf,"OK"))
        {
            WIFI_LOG(WIFI_ERROR,("Executing wfd_cmd=(%s) Failed!\n", _cmd_buf));
            return -1;
        }
    }
    else
    {
        char _cmd_buf[WFD_CMD_BUF_LEN] = {0};
        WIFI_LOG(WIFI_DEBUG, ("(wfddbg) disable WFD...\n"));
        snprintf(_cmd_buf,sizeof(_cmd_buf), "SET wifi_display %d", WFD_CMD_VAL_DISABLE_WFD);
        if (strlen(_cmd_buf) && !dop2pBooleanCommand(_cmd_buf,"OK"))
        {
            WIFI_LOG(WIFI_ERROR,("!! Executing wfd_cmd=(%s) Failed!\n", _cmd_buf));
            return -1;
        }
    }
    p2p_for_wfd = FALSE;
    return NET_WLAN_OK;
}


INT32 c_net_wlan_p2p_init_wfd_settings(void)
{
    WIFI_LOG_ENTRY(WIFI_API, (""));
    int index  ;
    char _cmd_buf[WFD_CMD_BUF_LEN] = {0};

    for (index = 0; index < WFD_P2P_CMD_MAX ; index++ )
    {
        x_memset(_cmd_buf, 0, WFD_CMD_BUF_LEN);
        switch (index)
        {
        case WFD_P2P_CMD_DEV_TYPE:
            snprintf(_cmd_buf,sizeof(_cmd_buf), "SET %s %d",wfd_cmd_list[index], WFD_CMD_VAL_DEV_TYPE_SINK);
            break;

        case WFD_P2P_CMD_SOURCE_COUPLED:
            snprintf(_cmd_buf,sizeof(_cmd_buf), "SET %s %d",wfd_cmd_list[index], WFD_CMD_VAL_SOURCE_COUPLED);
            break;

        case WFD_P2P_CMD_SINK_COUPLED:
            snprintf(_cmd_buf,sizeof(_cmd_buf), "SET %s %d",wfd_cmd_list[index], WFD_CMD_VAL_SINK_COUPLED);
            break;

        case WFD_P2P_CMD_SESSION_AVAILABLE:
            snprintf(_cmd_buf,sizeof(_cmd_buf), "SET %s %d",wfd_cmd_list[index], WFD_CMD_VAL_SESSION_AVAILABLE);
            break;

        case WFD_P2P_CMD_RTSP_PORT:
            snprintf(_cmd_buf,sizeof(_cmd_buf), "SET %s %d", wfd_cmd_list[index], WFD_CMD_VAL_RTSP_PORT);
            break;

        case WFD_P2P_CMD_MAX_THROUGHPUT:
            snprintf(_cmd_buf,sizeof(_cmd_buf), "SET %s %d",wfd_cmd_list[index], WFD_CMD_VAL_MAX_THROUGHPUT);
            break;
#if PLATFORM_IS_TV
        case WFD_P2P_CMD_DEVICE_NAME:
        {
            char mac4B[5] = {0x00}; /* 4 byte mac addr */

            c_net_wlan_get_mac4B(p2p_name, mac4B, sizeof(mac4B));
            /* Make the default device name unique, which looks like:
                    MTK-DTV-Sink-xxxx
               , where xxxx are the last four octets of device p2p if mac addr.
            */
            snprintf(_cmd_buf,sizeof(_cmd_buf), "SET %s %s-%s",wfd_cmd_list[index], WFD_DEFAULT_DEVICE_NAME, mac4B);
            break;
        }

#endif
        case WFD_P2P_CMD_SET_CONTENT_PROTECT:
            snprintf(_cmd_buf,sizeof(_cmd_buf), "SET %s %d",wfd_cmd_list[index], WFD_CMD_VAL_ENABLE_CONTENT_PROTECT);
            break;

        default:
            WIFI_LOG(WIFI_ERROR,("[WIFI MW]!! Executing wfd_cmd=(%s) Failed! no command\n", _cmd_buf));
            break;

        }

        if (strlen(_cmd_buf) && !dop2pBooleanCommand(_cmd_buf,"OK"))
        {
            WIFI_LOG(WIFI_ERROR,("[WIFI MW]!! Executing wfd_cmd=(%s) Failed!\n", _cmd_buf));
            /* If those commands failed, set wfd device info subelement by "WFD_SUBELEM_SET" */
            x_net_wlan_p2p_set_wfd_dev_info_subelem();
            break;
        }
    }

    return NET_WLAN_OK;

}


INT32 c_net_wlan_p2p_enable_nat( const char * if_name , char * dns)
{
    WIFI_LOG_ENTRY(WIFI_API, (""));
    char start_crosscon_sh[256] = {0};
    if (if_name == NULL)
    {
        WIFI_LOG(WIFI_ERROR, ("c_net_wlan_p2p_enable_nat if_name null!\n"));
        return -1;
    }
#if PLATFORM_IS_BDP
    strncpy(start_crosscon_sh , "sh /etc/wifi.script/p2p_cross_conn.sh ",sizeof("sh /etc/wifi.script/p2p_cross_conn.sh "));
#else
    strncpy(start_crosscon_sh , "sh /3rd/bin/iptables/p2p_crosscon_start.sh ",sizeof("sh /3rd/bin/iptables/p2p_crosscon_start.sh "));
#endif
    strncat(start_crosscon_sh , if_name,strlen(if_name));
    strncat(start_crosscon_sh , " ",1);
    strncat(start_crosscon_sh , dns,strlen(dns));
    int ret = doScriptCmd(start_crosscon_sh);

#if PLATFORM_IS_BDP
    if (ret < 0)
#else
    if ((errno != 0) || ((ret >> 8) != 0))
#endif
    {
        WIFI_LOG(WIFI_ERROR, ("system start_crosscon_sh  return error!\n"));
        return -1;
    }
    return NET_WLAN_OK;

}

INT32 c_net_wlan_p2p_disable_nat(void)
{
    WIFI_LOG_ENTRY(WIFI_API, (""));
    doScriptCmd("echo \"0\" >> /proc/sys/net/ipv4/ip_forward");
    if (errno != 0)
    {
        WIFI_LOG(WIFI_ERROR, ("c_net_wlan_p2p_disable_nat system error!\n"));
        return -1;
    }
    return NET_WLAN_OK;
}


INT32 c_net_wlan_p2p_set_dev_name( const char * dev_name )
{
    WIFI_LOG_ENTRY(WIFI_API, (""));
    INT32 i4Ret = NET_WLAN_OK;

    if ((!dev_name) || (dev_name && (strlen(dev_name) > 32)))
    {
        WIFI_LOG(WIFI_WARNING,(">>>>>>> set_dev_name error! invalid parameter!\n"));
        return NET_WLAN_INVALID_PARAM;
    }

    char _cmd_buf[WFD_CMD_BUF_LEN]={0};
    x_memset(_cmd_buf , 0 ,WFD_CMD_BUF_LEN);
    snprintf(_cmd_buf,sizeof(_cmd_buf) ,"SET device_name %s",dev_name);
    if (!dop2pBooleanCommand(_cmd_buf, "OK"))
    {
        i4Ret = NET_WLAN_ERROR;
    }
    return i4Ret;

}



INT32 c_net_wlan_p2p_find(x_net_wlan_notify_fct pfNotify,BOOL for_listen)
{
    WIFI_LOG_ENTRY(WIFI_API, (""));

    INT32 i4Ret = NET_WLAN_OK;
    c_net_wlan_p2p_reg_cbk(pfNotify);

    char _cmd_buf[WFD_CMD_BUF_LEN]={0};
    Is_for_listen = for_listen;

    if ( p2p_for_wfd )//current is wfd mode .
    {
        x_memset(_cmd_buf , 0 ,WFD_CMD_BUF_LEN);
        snprintf(_cmd_buf,sizeof(_cmd_buf) ,"P2P_FIND wfd 1");
    }
    else
    {
        snprintf(_cmd_buf,sizeof(_cmd_buf) , "P2P_FIND");
    }

    if (!dop2pBooleanCommand(_cmd_buf, "OK"))
    {
        i4Ret = NET_WLAN_ERROR;
        WIFI_LOG(WIFI_ERROR,(">>>>>>>>>>>>>>>> c_net_wlan_p2p_find command return error\n"));
    }
    return i4Ret;

}

INT32 c_net_wlan_p2p_stop_find(void)
{
    WIFI_LOG_ENTRY(WIFI_API, (""));
    INT32 i4Ret = NET_WLAN_OK;

    char _cmd_buf[100] = {0};
    snprintf(_cmd_buf,sizeof(_cmd_buf) , "P2P_STOP_FIND" );

    if (!dop2pBooleanCommand(_cmd_buf, "OK"))
    {
        WIFI_LOG(WIFI_ERROR,(">>>>>>>>>>>>>>>> c_net_wlan_p2p_stop_find command return error\n"));
        i4Ret = NET_WLAN_ERROR;
    }
    return i4Ret;

}
INT32 c_net_wlan_p2p_connect(INT32 mode , const char * mac , const char * pin)
{
    WIFI_LOG_ENTRY(WIFI_API, (""));
    INT32 i4Ret = NET_WLAN_OK;
    if ((mac == NULL) || ((mode == P2P_CONNECT_MODE_PIN ) && (pin == NULL)))
    {
        WIFI_LOG(WIFI_ERROR,(">>>>>>> c_net_wlan_p2p_connect wrong param\n"));
        return NET_WLAN_INVALID_PARAM;
    }
    if (strlen(mac) != 17)
        return NET_WLAN_INVALID_PARAM;

    x_memset(wfd_connected_mac, 0 , 18);
    strncpy(wfd_connected_mac ,mac ,17);

    char _cmd_buf[WFD_CMD_BUF_LEN] = {0};
    if (mode == P2P_CONNECT_MODE_PIN || mode == P2P_CONNECT_MODE_PBC_DIS)
    {
        strncat(_cmd_buf , "P2P_CONNECT ",strlen("P2P_CONNECT "));
    }
    else
    {
        strncat(_cmd_buf , "P2P_PROV_DISC ",strlen("P2P_PROV_DISC "));
    }
    strncat(_cmd_buf , mac,strlen(mac));

    if (mode == P2P_CONNECT_MODE_PBC || mode == P2P_CONNECT_MODE_PBC_DIS)
        strncat(_cmd_buf , " pbc",strlen(" pbc"));
    else if (mode == P2P_CONNECT_MODE_PIN)
    {
        strncat(_cmd_buf , " ",strlen(" "));
        strncat(_cmd_buf, pin,strlen(pin));
    }
    else if ( mode == P2P_CONNECT_MODE_PIN_KEYPAD )//but how can we pass my pin code to app ?  because app need show our pin code .
    {
        strncat(_cmd_buf , " keypad",strlen(" keypad"));
    }
    else if ( mode == P2P_CONNECT_MODE_PIN_DISPLAY )
    {
        strncat(_cmd_buf , " display",strlen(" display"));
    }

    strncat(_cmd_buf , " persistent", strlen(" persistent"));

    if (!dop2pBooleanCommand(_cmd_buf, "OK"))
    {
        WIFI_LOG(WIFI_ERROR,(">>>>>> c_net_wlan_p2p_connect -> return error!i4Ret = %d\n", i4Ret));
        i4Ret = NET_WLAN_ERROR;
    }
    return i4Ret;

}

INT32 c_net_wlan_p2p_pin_display_accept()
{
    WIFI_LOG_ENTRY(WIFI_API, (""));
    INT32 i4Ret = NET_WLAN_OK;

    return i4Ret;
}

INT32 c_net_wlan_p2p_pbc_accept(const char * mac)
{
    WIFI_LOG_ENTRY(WIFI_API, (""));
    INT32 i4Ret = NET_WLAN_OK;
    if (mac == NULL)
    {
        WIFI_LOG(WIFI_ERROR,(">>>> c_net_wlan_p2p_pbc_accept wrong param!\n"));
        return NET_WLAN_INVALID_PARAM;
    }

    INT32 _mac_len = x_strlen(mac);
    if (_mac_len != 17)
        return NET_WLAN_INVALID_PARAM ;
    x_memset(wfd_connected_mac, 0 , 18);
    x_strncpy(wfd_connected_mac ,mac ,17);

    char _cmd_buf[WFD_CMD_BUF_LEN] = {0};
    snprintf(_cmd_buf,sizeof(_cmd_buf),"P2P_CONNECT %s pbc",mac);

    if (!dop2pBooleanCommand(_cmd_buf, "OK"))
    {
        i4Ret = NET_WLAN_ERROR;
        WIFI_LOG(WIFI_ERROR,(">>>>>> c_net_wlan_p2p_pbc_accept command return error\n"));
    }
    return i4Ret;

}


INT32 c_net_wlan_p2p_pin_connect(const char * mac , const char * pin)
{
    WIFI_LOG_ENTRY(WIFI_API, (""));
    INT32 i4Ret = NET_WLAN_OK;
    if ((mac == NULL) || (pin == NULL))
    {
        WIFI_LOG(WIFI_ERROR,(">>>> c_net_wlan_p2p_pin_connect wrong param!\n"));
        return NET_WLAN_INVALID_PARAM;
    }

    //// haifei add for just keep mac addr for wfd connect event .
    INT32 _mac_len = x_strlen(mac);
    if (_mac_len != 17)
        return NET_WLAN_INVALID_PARAM ;
    x_memset(wfd_connected_mac, 0 , 18);
    x_strncpy(wfd_connected_mac ,mac ,17);

    char _cmd_buf[WFD_CMD_BUF_LEN] = {0};
    snprintf(_cmd_buf,sizeof(_cmd_buf),"P2P_CONNECT %s %s",mac,pin);

    if (!dop2pBooleanCommand(_cmd_buf, "OK"))
    {
        i4Ret = NET_WLAN_ERROR;
        WIFI_LOG(WIFI_ERROR,(">>>>>> c_net_wlan_p2p_pin_connect return error!\n"));
    }
    return i4Ret;

}

INT32 c_net_wlan_p2p_pbc_connect(const char * mac)
{
    WIFI_LOG_ENTRY(WIFI_API, (""));
    INT32 i4Ret = NET_WLAN_OK;
    if ((mac == NULL) || (x_strlen(mac) != 17))
    {
        WIFI_LOG(WIFI_WARNING,(">>>> c_net_wlan_p2p_pbc_connect wrong param!\n"));
        return NET_WLAN_INVALID_PARAM;
    }

    x_memset(wfd_connected_mac, 0 , 18);
    x_strncpy(wfd_connected_mac ,mac ,17);

    char _cmd_buf[WFD_CMD_BUF_LEN] = {0};
    snprintf(_cmd_buf,sizeof(_cmd_buf),"P2P_PROV_DISC %s pbc",mac);
    if (!dop2pBooleanCommand(_cmd_buf, "OK"))
    {
        i4Ret = NET_WLAN_ERROR;
        WIFI_LOG(WIFI_ERROR,(">>>>>> c_net_wlan_p2p_pbc_connect  return error\n"));
    }
    return i4Ret;

}

#if PLATFORM_IS_TV
INT32 c_net_wlan_p2p_disconnect(const char * mac)
{
    WIFI_LOG_ENTRY(WIFI_API, (""));
    INT32 i4Ret = NET_WLAN_ERROR;
    if (0 == strncmp(if_name, "wlan0", strlen ("wlan0")))
    {
        if ((mac == NULL) || (x_strlen(mac) != 17))
        {
            WIFI_LOG(WIFI_WARNING,(">>>> c_net_wlan_p2p_disconnect wrong param!\n"));
            return NET_WLAN_INVALID_PARAM;
        }
        x_memset(wfd_connected_mac, 0 , 18);
        x_strncpy(wfd_connected_mac ,mac ,17);
        char _cmd_buf[WFD_CMD_BUF_LEN]={'\0'};
        memset(_cmd_buf , 0 ,WFD_CMD_BUF_LEN);
        sprintf(_cmd_buf , "%s/common/wpa_cli -i%s P2P_DISCONNECT %s",WPA_SUPPLICANT_PATH , p2p_go_name, wfd_connected_mac);
        WIFI_LOG (WIFI_DEBUG,(">>>>>>> %s,%d,cmd= %s\n",__FUNCTION__,__LINE__ ,_cmd_buf));
        i4Ret = doScriptCmd(_cmd_buf);
        return i4Ret;
    }
    else
    {
        char _cmd_buf[WFD_CMD_BUF_LEN] = {0};
        snprintf(_cmd_buf,sizeof(_cmd_buf), "P2P_DISCONNECT");
        if (!dop2pBooleanCommand(_cmd_buf, "OK"))
        {
            WIFI_LOG (WIFI_ERROR,(">>>>>> c_net_wlan_p2p_disconnect return error!\n"));
            return NET_WLAN_ERROR;
        }
        return NET_WLAN_OK;
    }

}
#else
INT32 c_net_wlan_p2p_disconnect()
{
    WIFI_LOG_ENTRY(WIFI_API, (""));
    INT32 i4Ret = NET_WLAN_OK;
    char _cmd_buf[WFD_CMD_BUF_LEN] = {0};
    snprintf(_cmd_buf,sizeof(_cmd_buf), "P2P_DISCONNECT");
    if (!dop2pBooleanCommand(_cmd_buf, "OK"))
    {
        WIFI_LOG (WIFI_ERROR,(">>>>>> c_net_wlan_p2p_disconnect return error!\n"));
        i4Ret = NET_WLAN_ERROR;
    }
    return i4Ret;

}
#endif
INT32 c_net_wlan_p2p_set_del_client(const char * mac)
{
    WIFI_LOG_ENTRY(WIFI_API, (""));
    INT32 i4Ret = NET_WLAN_ERROR;

    char _cmd_buf[WFD_CMD_BUF_LEN]={'\0'};

    if (mac == NULL)
    {
        WIFI_LOG(WIFI_WARNING,(">>>>>>>p2p set delete client: the mac is null!\n"));
        return NET_WLAN_INVALID_PARAM;
    }

    if (0 == strncmp(if_name, "wlan0", strlen ("wlan0")))
    {
#if PLATFORM_IS_TV
        snprintf(_cmd_buf,sizeof(_cmd_buf), "%s/common/wpa_cli -i%s P2P_SET STA_REMOVE %s",WPA_SUPPLICANT_PATH,p2p_go_name,mac);
#else
        snprintf(_cmd_buf,sizeof(_cmd_buf), "wpa_cli -i%s P2P_SET STA_REMOVE %s",p2p_go_name,mac);
#endif
        i4Ret = doScriptCmd(_cmd_buf);
        WIFI_LOG (WIFI_DEBUG,(">>>>>>> %s,%d,cmd= %s i4Ret=%d\n",__FUNCTION__,__LINE__ ,_cmd_buf,i4Ret));
        return i4Ret;
    }
    else
    {
        snprintf(_cmd_buf,sizeof(_cmd_buf), "SET p2p_del_client %s", mac);

        if (!dop2pBooleanCommand(_cmd_buf, "OK"))
        {
            WIFI_LOG(WIFI_ERROR,(">>>>>> p2p set delete client %s return error \n",mac));
            return NET_WLAN_ERROR;
        }
        return NET_WLAN_OK;
    }
}

/* cut P2P connection for P2P GO */
INT32 c_net_wlan_p2p_disassociate(const char *mac)
{
    WIFI_LOG_ENTRY(WIFI_API, (""));

    char _cmd_buf[WFD_CMD_BUF_LEN]={'\0'};

    if (mac == NULL)
    {
        WIFI_LOG(WIFI_ERROR,(">>>>>>>p2p disassociate: the mac is null!\n"));
        return NET_WLAN_INVALID_PARAM;
    }

    snprintf(_cmd_buf,sizeof(_cmd_buf), "DISASSOCIATE %s", mac);

    if (!dop2pBooleanCommand(_cmd_buf, "OK"))
    {
        WIFI_LOG(WIFI_ERROR,(">>>>>> p2p disassociate %s return error \n",mac));
        return NET_WLAN_ERROR;
    }
    
    return NET_WLAN_OK;
}

INT32 c_net_wlan_p2p_set_go_ssid(char *ssid,char *passphrase)
{
    WIFI_LOG_ENTRY(WIFI_API, (""));
    char _cmd_buf[WFD_CMD_BUF_LEN]={'\0'};
    char go_ssid[33] = {'\0'};
    char go_passphrase[65] = {'\0'};


    if ((ssid == NULL) || (ssid && (strlen(ssid) > 32)))
    {
        WIFI_LOG(WIFI_ERROR,(">>>>>>>p2p set go ssid: ssid len is too long or ssid null!\n"));
        return NET_WLAN_INVALID_PARAM;
    }
    if (passphrase == NULL)
    {
        strncpy(go_passphrase,"12345678",sizeof(go_passphrase) - 1);
    }
    else
    {
        strncpy(go_passphrase,passphrase,sizeof(go_passphrase) - 1);
    }
    strncpy(go_ssid,ssid,sizeof(go_ssid) - 1);

    snprintf(_cmd_buf,sizeof(_cmd_buf), "P2P_GO_SSID ssid=%s wpapsk=%s", go_ssid,go_passphrase);

    if (!dop2pBooleanCommand(_cmd_buf, "OK"))
    {
        WIFI_LOG(WIFI_ERROR,(">>>>>>>p2p set go ssid return error!\n"));
        return NET_WLAN_ERROR;
    }
    else
    {
        return NET_WLAN_OK;
    }

}

INT32 c_net_wlan_p2p_set_go_intent_value(UINT8 intent)
{
    WIFI_LOG_ENTRY(WIFI_API, (("intent %d"),intent));
    char _cmd_buf[100] = {0};

    if (intent < 0 || intent > 15)
        return NET_WLAN_INVALID_PARAM;

    snprintf(_cmd_buf,sizeof(_cmd_buf) ,"SET p2p_go_intent %d",intent);

    BOOL b_ret = dop2pBooleanCommand(_cmd_buf,"OK");
    if (!b_ret)
    {
        WIFI_LOG(WIFI_DEBUG,(">>>>>>>  set go intent %d return error \n",intent));
        return NET_WLAN_ERROR;
    }
    return NET_WLAN_OK;

}

INT32 c_net_wlan_p2p_enable_wfd_mode(void)
{
    WIFI_LOG_ENTRY(WIFI_API, (""));
    int i4Ret = 0;
    i4Ret = c_net_wlan_p2p_init_wfd_settings();
    WIFI_CHK_FAIL (i4Ret);
#if 0
    i4Ret = c_net_wlan_p2p_set_go_intent_value(15);
    WIFI_CHK_FAIL (i4Ret);
#endif
    i4Ret = x_net_wlan_p2p_enable_wfd_mode();
    WIFI_CHK_FAIL (i4Ret);
    return i4Ret;
}
INT32 c_net_wlan_p2p_set_as_wfd_mode(BOOL wfd_mode)
{
    int i4Ret = 0;
    if (wfd_mode)
    {
        i4Ret = c_net_wlan_p2p_enable_wfd_mode();
    }
    else
    {
        i4Ret = c_net_wlan_p2p_disable_wfd_mode();
    }
    return i4Ret;
}

/* for wpa_supplicant-2.4, enable/disable wifi display by command "SET wifi_display 1/0" to wpa_supplicant */
INT32 c_net_wlan_p2p_enable_wifi_display(int enable)
{
    WIFI_LOG_ENTRY(WIFI_API, (""));
    char _cmd_buf[WFD_CMD_BUF_LEN] = {0};
    WIFI_LOG(WIFI_DEBUG, ("Enable wifi display...\n"));
    
    snprintf(_cmd_buf,sizeof(_cmd_buf), "SET wifi_display %d", enable?1:0);
    if (strlen(_cmd_buf) && !dop2pBooleanCommand(_cmd_buf,"OK"))
    {
        WIFI_LOG(WIFI_ERROR,("!! Executing wfd_cmd=(%s) Failed!\n", _cmd_buf));
        return -1;
    }

    return 0;
}

/* for wpa_supplicant-2.4, Set WFD device information subelements by command "WFD_SUBELEM_SET 0 00060111022a0014" */
INT32 c_net_wlan_p2p_set_wfd_dev_info_subelem(void)
{
    WIFI_LOG_ENTRY(WIFI_API, (""));

    return x_net_wlan_p2p_set_wfd_dev_info_subelem();
}


INT32 c_net_wlan_get_ifconfig(char *ifname, char *macbuf, char *ipbuf, char *maskbuf)
{
    WIFI_LOG_ENTRY(WIFI_API, (""));
    char cmdStr[256];
    FILE* fp = NULL;
    char *ptr = NULL;
    int mac_found = 0, ip_found = 0, mask_found = 0;
    int idx = 0;

    if (!macbuf || !ipbuf || !maskbuf)
        return -1;

    x_memset(cmdStr, 0, sizeof(cmdStr));
    snprintf(cmdStr,sizeof(cmdStr), "ifconfig %s > %s;sync;sync;", ifname, WLAN_GET_IFCONFIG_PATH);
    doScriptCmd(cmdStr);

    x_memset(cmdStr, 0, sizeof(cmdStr));
    fp = fopen(WLAN_GET_IFCONFIG_PATH, "r");
    if (!fp)
    {
        WIFI_LOG(WIFI_DEBUG,( "%s: file open failed\n", __FUNCTION__));
        return -1;
    }
    while (fgets(cmdStr, sizeof(cmdStr), fp))
    {
        if ((ptr = strstr(cmdStr, "HWaddr ")) != NULL)
        {
            /* found mac */
            ptr += strlen("HWaddr ");
            strncpy(macbuf, ptr, 17);
            mac_found = 1;
        }
        if ((ptr = strstr(cmdStr, "inet addr:")) != NULL)
        {
            /* found ip */
            ptr += strlen("inet addr:");
            idx = 0;
            while ((((*ptr)>='0') && ((*ptr)<='9')) || ((*ptr)=='.'))
            {
                ipbuf[idx++] = *ptr;
                ptr ++;
            }
            ip_found = 1;
        }
        if ((ptr = strstr(cmdStr, "Mask:")) != NULL)
        {
            /* found mask */
            ptr += strlen("Mask:");
            idx = 0;
            while ((((*ptr)>='0') && ((*ptr)<='9')) || ((*ptr)=='.'))
            {
                maskbuf[idx++] = *ptr;
                ptr ++;
            }
            mask_found = 1;
        }
        x_memset(cmdStr, 0, sizeof(cmdStr));
    }

    if (fp)
        fclose(fp);
    if (mac_found && ip_found && mask_found)
        return 0;
    else
        return -1;

}

#if PLATFORM_IS_TV
static INT32 c_net_wlan_get_mac4B(char *ifname, char *buf, int buf_len)
{
    WIFI_LOG_ENTRY(WIFI_API, (""));
    char tmpmac[32], tmpbuf[32];
    char *tmpptr = NULL;

    if (!ifname || !buf || buf_len<5)
        return -1;

    x_memset(tmpmac, 0, sizeof(tmpmac));
    x_memset(tmpbuf, 0, sizeof(tmpbuf));
    x_memset(buf, 0, buf_len);
    /* because we don't care about ip & mask, so use same tmpbuf */
    c_net_wlan_get_ifconfig(ifname, tmpmac, tmpbuf, tmpbuf);
    /* the output mac should be: 00:00:00:00:a1:f9 */
    if (strlen(tmpmac) != 17)
    {
        WIFI_LOG(WIFI_DEBUG,("%s: Error! MAC addr length mismatch(%d)\n", __FUNCTION__, strlen(tmpmac)));
        return -1;
    }
    /* find the last four bytes */
    tmpptr = tmpmac;
    tmpptr += 12;
    strncpy(buf, tmpptr, 2);
    tmpptr+=3;
    strncat(buf, tmpptr, 2);

    return 0;
}
#endif

INT32 c_net_wlan_p2p_signalPoll(char *reply)
{
    WIFI_LOG_ENTRY(WIFI_API, (""));
    char _cmd_buf[WFD_CMD_BUF_LEN]={'\0'};
    snprintf(_cmd_buf, sizeof(_cmd_buf), "SIGNAL_POLL");

    if (dop2pStringCommand(_cmd_buf, reply, sizeof(reply)) == -1)
    {
        WIFI_LOG(WIFI_DEBUG,(">>>>>>>p2p signal poll return error!\n"));
        return -1;
    }
    else
    {
        return 0;
    }

}
INT32 c_net_wlan_p2p_wpsPbc(const char * mac)
{
    WIFI_LOG_ENTRY(WIFI_API, (""));
    char _cmd_buf[100]={'\0'};
    int i4Ret = -1;
    if (mac == NULL)
    {
        snprintf(_cmd_buf, sizeof(_cmd_buf), "WPS_PBC any");
    }
    else
    {
        snprintf(_cmd_buf, sizeof(_cmd_buf), "WPS_PBC %s", mac);
    }

    if (0 == strncmp(if_name, "wlan0", strlen ("wlan0")) ||
        0 == strncmp(p2p_go_name, "p2p-p2p0-", strlen("p2p-p2p0-")))
    {
#if PLATFORM_IS_TV
        snprintf(_cmd_buf,sizeof(_cmd_buf), "%s/common/wpa_cli -i%s WPS_PBC any",WPA_SUPPLICANT_PATH,p2p_go_name);
#else
        snprintf(_cmd_buf,sizeof(_cmd_buf), "wpa_cli -i%s WPS_PBC any",p2p_go_name);
#endif
        i4Ret = doScriptCmd(_cmd_buf);
        WIFI_LOG (WIFI_DEBUG,(">>>>>>> %s,%d,cmd= %s i4Ret=%d\n",__FUNCTION__,__LINE__ ,_cmd_buf,i4Ret));
        return i4Ret;
    }
    else
    {
        if (!dop2pBooleanCommand(_cmd_buf, "OK"))
        {
            WIFI_LOG (WIFI_DEBUG,(">>>>>>>p2p wps pbc return error!\n"));
            return -1;
        }
        else
            return 0;
    }
}

INT32 c_net_wlan_p2p_wpsPin_withMac(const char * mac,const char *pin)
{
    WIFI_LOG_ENTRY(WIFI_API, (""));
    char _cmd_buf[WFD_CMD_BUF_LEN]={'\0'};
    if (pin == NULL || x_strlen(pin) != 8)
    {
        WIFI_LOG(WIFI_ERROR,("pin code is invalid ! pin = %s \n", pin));
        return -1;
    }
    if (mac == NULL)
    {
        snprintf(_cmd_buf, sizeof(_cmd_buf), "WPS_PIN any %s", pin);
    }
    else
    {
        if (x_strlen(mac) != 17)
        {
            WIFI_LOG(WIFI_ERROR,(">>>>>>>mac address is invalid mac = %s!\n",mac));
            return -1;
        }
        snprintf(_cmd_buf, sizeof(_cmd_buf), "WPS_PIN %s %s",mac, pin);
    }
    if (!dop2pBooleanCommand(_cmd_buf, pin))
    {
        WIFI_LOG(WIFI_ERROR,(">>>>>>>p2p wps pin return error!\n"));
        return -1;
    }
    else
    {
        return 0;
    }
}

INT32 c_net_wlan_p2p_wpsPin(const char *pin)
{
    return c_net_wlan_p2p_wpsPin_withMac(NULL,pin);
}

INT32 c_net_wlan_p2p_wpsCancel(void)
{
    WIFI_LOG_ENTRY(WIFI_API, (""));
    char _cmd_buf[WFD_CMD_BUF_LEN]={'\0'};
    snprintf(_cmd_buf, sizeof(_cmd_buf), "WPS_CANCEL");

    if (!dop2pBooleanCommand(_cmd_buf, "OK"))
    {
        WIFI_LOG(WIFI_ERROR,(">>>>>>>p2p wps cancel return error!\n"));
        return -1;
    }
    else
    {
        return 0;
    }

}

INT32 c_net_wlan_p2p_setPersistentReconnect(BOOL enabled)
{
    WIFI_LOG_ENTRY(WIFI_API, ("enable %d",enabled));
    int value = (enabled == TRUE) ? 1 : 0;

    char _cmd_buf[WFD_CMD_BUF_LEN]={'\0'};
    snprintf(_cmd_buf, sizeof(_cmd_buf), "SET persistent_reconnect %d", value);

    if (!dop2pBooleanCommand(_cmd_buf, "OK"))
    {
        WIFI_LOG(WIFI_ERROR,(">>>>>>>p2p set persistent reconnect return error!\n"));
        return NET_WLAN_ERROR;
    }
    else
    {
        return NET_WLAN_OK;
    }
}

INT32 c_net_wlan_p2p_setDeviceType(char *type)
{
    WIFI_LOG_ENTRY(WIFI_API, (""));
    char _cmd_buf[WFD_CMD_BUF_LEN]={'\0'};

    if (type == NULL)
    {
        WIFI_LOG(WIFI_WARNING,(">>>>>>>p2p set device type: the type is null!\n"));
        return NET_WLAN_INVALID_PARAM;
    }

    snprintf(_cmd_buf, sizeof(_cmd_buf), "SET device_type %s", type);

    if (!dop2pBooleanCommand(_cmd_buf, "OK"))
    {
        WIFI_LOG(WIFI_ERROR,(">>>>>>>p2p set device type return error!\n"));
        return NET_WLAN_ERROR;
    }
    else
    {
        return NET_WLAN_OK;
    }

}

INT32 c_net_wlan_p2p_setOperchannel(INT16 channel)
{
    WIFI_LOG_ENTRY(WIFI_API, ("channel %d",channel));
    char _cmd_buf[WFD_CMD_BUF_LEN]={'\0'};
    snprintf(_cmd_buf, sizeof(_cmd_buf), "SET p2p_oper_channel %d", channel);
    if (!dop2pBooleanCommand(_cmd_buf, "OK"))
    {
        WIFI_LOG(WIFI_ERROR,(">>>>>>>p2p set p2p_oper_channel return error!\n"));
        return NET_WLAN_ERROR;
    }
    else
    {
        return NET_WLAN_OK;
    }
}

INT32 c_net_wlan_p2p_get_operation_channel(int *channel)
{
    WIFI_LOG_ENTRY(WIFI_API, (""));
    char reply[8] = {'\0'};
    if (dop2pStringCommand("P2P_OPR_CH", reply, sizeof(reply)) < 0)
    {
        WIFI_LOG(WIFI_ERROR,(">>>>>>>p2p get p2p_oper_channel return error!\n"));
        return NET_WLAN_ERROR;
    }

    INT16 i2Ret = (INT16)reply[0];
    WIFI_LOG (WIFI_DEBUG, ("[WIFI MW]c_net_wlan_p2p_getOperchannel %d",i2Ret));
    if (i2Ret != -1)
    {
        *channel = i2Ret;
    }
    return NET_WLAN_OK;
}

INT32 c_net_wlan_p2p_getAutoOprch(int *channel)
{
    WIFI_LOG_ENTRY(WIFI_API, (""));
    char reply[8] = {'\0'};
    if (dop2pStringCommand("P2P_AUTO_OPR_CH", reply, sizeof(reply)) < 0)
    {
        WIFI_LOG(WIFI_ERROR,(">>>>>>>p2p get p2p auto_oper_channel return error!\n"));
        return NET_WLAN_ERROR;
    }
    INT16 i2Ret = (INT16)reply[0];
    WIFI_LOG(WIFI_DEBUG,(">>>>>>>p2p get p2p auto_oper_channel return %d!\n",i2Ret));
    if (i2Ret != -1)
    {
        *channel = i2Ret;
    }
    return NET_WLAN_OK;
}


INT32 c_net_wlan_p2p_set_auto_channel_selection(char *iface, int auto_ch_select)
{
    WIFI_LOG_ENTRY(WIFI_API, ("auto channel selection flag=%d.", auto_ch_select));
    char _cmd_buf[WFD_CMD_BUF_LEN]={'\0'};

    snprintf(_cmd_buf, sizeof(_cmd_buf), "iwpriv %s set p2pAutoChannel=%d", iface, auto_ch_select);

    if ( doScriptCmd(_cmd_buf) != 0 )
    {
        WIFI_LOG(WIFI_DEBUG, ("command failed: %s.", _cmd_buf));
        return NET_WLAN_ERROR;
    }

    return NET_WLAN_OK;
}


INT32 c_net_wlan_p2p_listen(int timeout)
{
    WIFI_LOG_ENTRY(WIFI_API, ("timeout=%d.", timeout));
    char _cmd_buf[WFD_CMD_BUF_LEN]={'\0'};

    if (timeout <= 0)
    {
        snprintf(_cmd_buf, sizeof(_cmd_buf), "P2P_LISTEN");
    }
    else
    {
        snprintf(_cmd_buf, sizeof(_cmd_buf), "P2P_LISTEN %d", timeout);
    }
    if (!dop2pBooleanCommand(_cmd_buf, "OK"))
    {
        WIFI_LOG(WIFI_ERROR,(">>>>>>>p2p listen return error!\n"));
        return NET_WLAN_ERROR;
    }
    else
    {
        return NET_WLAN_OK;
    }

}

INT32 c_net_wlan_p2p_listen_timeout(int timeout)
{
    return c_net_wlan_p2p_listen(timeout); //for build,need remove
}

//stop p2p listen
INT32 c_net_wlan_p2p_stop_listen(void)
{
    WIFI_LOG_ENTRY(WIFI_API, (""));
    char _cmd_buf[WFD_CMD_BUF_LEN]={'\0'};

    snprintf(_cmd_buf, sizeof(_cmd_buf), "P2P_LISTEN STOP");
        
    if (!dop2pBooleanCommand(_cmd_buf, "OK")) {
        WIFI_LOG(WIFI_ERROR,(">>>>>>>p2p listen stop return error!\n"));
        return NET_WLAN_ERROR;
    }
    
    return NET_WLAN_OK;
}


INT32 c_net_wlan_p2p_setListenchannel(INT16 channel)
{
    WIFI_LOG_ENTRY(WIFI_API, ("channel %d",channel));
    char _cmd_buf[WFD_CMD_BUF_LEN]={'\0'};
    snprintf(_cmd_buf, sizeof(_cmd_buf), "SET p2p_listen_channel %d", channel);
    if (!dop2pBooleanCommand(_cmd_buf, "OK"))
    {
        WIFI_LOG(WIFI_ERROR,(">>>>>>>p2p set p2p_oper_channel return error!\n"));
        return NET_WLAN_ERROR;
    }
    else
    {
        return NET_WLAN_OK;
    }
}


INT32 c_net_wlan_p2p_flush()
{
    WIFI_LOG_ENTRY(WIFI_API, (""));
    char _cmd_buf[WFD_CMD_BUF_LEN]={'\0'};

    snprintf(_cmd_buf, sizeof(_cmd_buf), "P2P_FLUSH");

    if (!dop2pBooleanCommand(_cmd_buf, "OK"))
    {
        WIFI_LOG(WIFI_ERROR,(">>>>>>>p2p flush return error!\n"));
        return NET_WLAN_ERROR;
    }
    else
    {
        return NET_WLAN_OK;
    }

}


INT32 c_net_wlan_p2p_cancelConnect()
{
    WIFI_LOG_ENTRY(WIFI_API, (""));
    char _cmd_buf[WFD_CMD_BUF_LEN]={'\0'};

    snprintf(_cmd_buf, sizeof(_cmd_buf), "P2P_CANCEL");

    if (!dop2pBooleanCommand(_cmd_buf, "OK"))
    {
        WIFI_LOG(WIFI_ERROR,(">>>>>>>p2p cancel connect return error!\n"));
        return NET_WLAN_ERROR;
    }
    else
    {
        return NET_WLAN_OK;
    }

}

INT32 c_net_wlan_p2p_groupAdd()
{
    WIFI_LOG_ENTRY(WIFI_API, (""));

    char _cmd_buf[WFD_CMD_BUF_LEN]={'\0'};

    snprintf(_cmd_buf, sizeof(_cmd_buf), "P2P_GROUP_ADD");

    if (!dop2pBooleanCommand(_cmd_buf, "OK"))
    {
        WIFI_LOG(WIFI_ERROR,(">>>>>>>p2p group add return error!\n"));
        return NET_WLAN_ERROR;
    }
    else
    {
        return NET_WLAN_OK;
    }

}

INT32 c_net_wlan_p2p_groupAdd_withssid(const char *ssid,const char *passphrase)
{
    WIFI_LOG_ENTRY(WIFI_API, ("ssid=%s,passphrase=%s",ssid,passphrase));
    char _cmd_buf[200]={'\0'};
    char go_ssid[33] = {'\0'};
    char go_passphrase[65] = {'\0'};

    if ((ssid == NULL) || (ssid && (strlen(ssid) > 32)))
    {
        WIFI_LOG(WIFI_WARNING,(">>>>>>>p2p set go ssid: ssid len is too long or ssid null!,use default\n"));
        strncpy(go_ssid,"DIRECT-nm",strlen("DIRECT-nm"));
    }
    else
    {
        strncpy(go_ssid,ssid,sizeof(go_ssid) - 1);
    }
    if (passphrase == NULL)
    {
        WIFI_LOG(WIFI_WARNING,(">>>>>>>passphrase is null! use default\n"));
        strncpy(go_passphrase,"12345678",sizeof(go_passphrase) - 1);
    }
    else
    {
        strncpy(go_passphrase,passphrase,sizeof(go_passphrase) - 1);
    }


    snprintf(_cmd_buf, sizeof(_cmd_buf), "P2P_GROUP_ADD ssid=%s wpapsk=%s", go_ssid,go_passphrase);

    if (!dop2pBooleanCommand(_cmd_buf, "OK"))
    {
        WIFI_LOG(WIFI_ERROR,(">>>>>>>p2p group add_with_ssid return error!\n"));
        return NET_WLAN_ERROR;
    }
    else
    {
        return NET_WLAN_OK;
    }

}


INT32 c_net_wlan_p2p_groupRemove(char *iface)
{
    WIFI_LOG_ENTRY(WIFI_API, (""));
    char _cmd_buf[WFD_CMD_BUF_LEN]={'\0'};

    if (iface == NULL || 
        strncmp(p2p_go_name, "p2p-p2p0-", strlen("p2p-p2p0-")) == 0)
    {
        snprintf(_cmd_buf, sizeof(_cmd_buf), "P2P_GROUP_REMOVE %s", p2p_go_name);
    }
    else
    {
        snprintf(_cmd_buf, sizeof(_cmd_buf), "P2P_GROUP_REMOVE %s", iface);
    }

    if (!dop2pBooleanCommand(_cmd_buf, "OK"))
    {
        WIFI_LOG(WIFI_ERROR,(">>>>>>>p2p group remove return error!\n"));
        return NET_WLAN_ERROR;
    }
    else
    {
        return NET_WLAN_OK;
    }

}



INT32 c_net_wlan_p2p_reject(char *deviceAddress)
{
    WIFI_LOG_ENTRY(WIFI_API, (""));
    char _cmd_buf[WFD_CMD_BUF_LEN]={'\0'};

    if (deviceAddress == NULL)
    {
        WIFI_LOG(WIFI_WARNING,(">>>>>>>p2p reject: deviceAddress is null!\n"));
        return NET_WLAN_INVALID_PARAM;
    }

    snprintf(_cmd_buf,sizeof(_cmd_buf), "P2P_REJECT %s", deviceAddress);

    if (!dop2pBooleanCommand(_cmd_buf, "OK"))
    {
        WIFI_LOG(WIFI_ERROR,(">>>>>>>p2p reject return error!\n"));
        return NET_WLAN_ERROR;
    }
    else
    {
        fgProvisionReq = FALSE;
        return NET_WLAN_OK;
    }

}

INT32 c_net_wlan_p2p_reinvoke(int netId, char *deviceAddress)
{
    WIFI_LOG_ENTRY(WIFI_API, ("netid %d",netId));
    char _cmd_buf[WFD_CMD_BUF_LEN]={'\0'};

    if (deviceAddress == NULL)
    {
        WIFI_LOG(WIFI_WARNING,(">>>>>>>p2p reinvoke: deviceAddress is null!\n"));
        return NET_WLAN_INVALID_PARAM;
    }

    if (netId < 0)
    {
        WIFI_LOG(WIFI_WARNING,(">>>>>>>p2p reinvoke: netId < 0!\n"));
        return NET_WLAN_INVALID_PARAM;
    }

    snprintf(_cmd_buf,sizeof(_cmd_buf), "P2P_INVITE persistent=%d peer=%s", netId, deviceAddress);

    if (!dop2pBooleanCommand(_cmd_buf, "OK"))
    {
        WIFI_LOG(WIFI_ERROR,(">>>>>>>p2p reinvoke return error!\n"));
        return NET_WLAN_ERROR;
    }
    else
    {
        return NET_WLAN_OK;
    }

}

INT32 c_net_wlan_p2p_invite(char *groupInterface, char *groupOwnerDeviceAddress, char *deviceAddress)
{
    WIFI_LOG_ENTRY(WIFI_API, (""));
    char _cmd_buf[WFD_CMD_BUF_LEN]={'\0'};

    if (deviceAddress == NULL)
    {
        WIFI_LOG(WIFI_WARNING,(">>>>>>>p2p invite: deviceAddress is null!\n"));
        return NET_WLAN_INVALID_PARAM;
    }

    snprintf(_cmd_buf,sizeof(_cmd_buf), "P2P_INVITE group=%s peer=%s", p2p_go_name, deviceAddress);

    if (groupInterface == NULL || groupOwnerDeviceAddress == NULL)
    {
        if (!dop2pBooleanCommand(_cmd_buf, "OK"))
        {
            WIFI_LOG(WIFI_ERROR,(">>>>>>>p2p invite return error!\n"));
            return NET_WLAN_ERROR;
        }
        else
        {
            return NET_WLAN_OK;
        }
    }
    else
    {
        snprintf(_cmd_buf,sizeof(_cmd_buf), "P2P_INVITE group=%s peer=%s go_dev_addr=%s", groupInterface,
                 deviceAddress, groupOwnerDeviceAddress);

        if (!dop2pBooleanCommand(_cmd_buf, "OK"))
        {
            WIFI_LOG(WIFI_ERROR,(">>>>>>>p2p invite return error!\n"));
            return NET_WLAN_ERROR;
        }
        else
        {
            return NET_WLAN_OK;
        }
    }
}


INT32 c_net_wlan_p2p_peer(char *reply, char *deviceAddress)
{
    WIFI_LOG_ENTRY(WIFI_API, (""));
    char _cmd_buf[WFD_CMD_BUF_LEN]={'\0'};

    if (deviceAddress == NULL)
    {
        WIFI_LOG(WIFI_WARNING,(">>>>>>>p2p peer: deviceAddress is null!\n"));
        return NET_WLAN_INVALID_PARAM;
    }

    snprintf(_cmd_buf,sizeof(_cmd_buf), "P2P_PEER %s", deviceAddress);

    if (dop2pStringCommand(_cmd_buf, reply, sizeof(reply)) == -1)
    {
        WIFI_LOG(WIFI_ERROR,(">>>>>>>p2p peer return error!\n"));
        return NET_WLAN_ERROR;
    }
    else
    {
        return NET_WLAN_OK;
    }

}

INT32 c_net_wlan_get_p2p_peer_info(
    NET_802_11_P2P_PEER_INFO *peerInfo, char *deviceAddress)
{
    WIFI_LOG_ENTRY(WIFI_API, (""));

    return x_net_wlan_get_p2p_peer_info(peerInfo, deviceAddress);
}


INT32 c_net_wlan_p2p_getInterfaceAddress(char *interfaceAddress, char *deviceAddress)
{
    WIFI_LOG_ENTRY(WIFI_API, (""));
    int i4Ret = x_net_wlan_p2p_getInterfaceAddress (interfaceAddress,deviceAddress);
    WIFI_CHK_FAIL (i4Ret);
    return i4Ret;
}

INT32 c_net_wlan_p2p_getDeviceAddress(char *deviceAddress)
{
    WIFI_LOG_ENTRY(WIFI_API, (""));
    int i4Ret = x_net_wlan_p2p_getDeviceAddress (deviceAddress);
    WIFI_CHK_FAIL (i4Ret);
    return i4Ret;
}

INT32 c_net_wlan_p2p_get_go_ssid(char *gossid)
{
    WIFI_LOG_ENTRY(WIFI_API, (""));
    int i4Ret = x_net_wlan_p2p_get_go_ssid(gossid);
    WIFI_CHK_FAIL (i4Ret);
    return i4Ret;
}

INT32 c_net_wlan_p2p_get_wpa_key(char *wpakey)
{
    WIFI_LOG_ENTRY(WIFI_API, (""));
    int i4Ret = x_net_wlan_p2p_get_WPA_key(wpakey);
    WIFI_CHK_FAIL (i4Ret);
    return i4Ret;
}


INT32 c_net_wlan_p2p_set_persistent(BOOL enable)
{
#if PLATFORM_IS_TV
    if (0 == strncmp(if_name, "ra0", strlen ("ra0")))
    {
#endif
        WIFI_LOG_ENTRY(WIFI_API, (""));

        char _cmd_buf[32]={0};
        int value = enable?1:0;
        snprintf(_cmd_buf ,sizeof(_cmd_buf),"SET p2p_persistent_enable %d",value);

        if (!dop2pBooleanCommand(_cmd_buf, "OK"))
        {
            WIFI_LOG(WIFI_DEBUG ,(">>>>>>> c_net_wlan_p2p_set_persistent command return error\n"));
            return NET_WLAN_ERROR;
        }
        return NET_WLAN_OK;
#if PLATFORM_IS_TV
    }
    return NET_WLAN_OK;
#endif
}

INT32 c_net_wlan_setPersistentTable(PERSISTENT_ENTRY_T* per_entry)
{
    WIFI_LOG_ENTRY(WIFI_API, (""));
    EEPROM_PER_ENTRY_T* EepPerEntry;
    int NumOfEntry;
    if (!per_entry)
    {
        return NET_WLAN_ERROR;
    }
    NumOfEntry = per_entry->NumOfEntry;
    EepPerEntry = per_entry->eep_per_entry;

    if (!EepPerEntry)
    {
        WIFI_LOG(WIFI_DEBUG ,("c_net_wlan_setPersistentTable EepPerEntry null\n"));
        return NET_WLAN_ERROR;
    }

    struct iwreq wrq;
    int sock;
    int i, j;
    RT_P2P_PERSISTENT_ENTRY P2pPerEntry[MAX_P2P_TABLE_SIZE];
    int rc;

    sock = socket(AF_INET, SOCK_STREAM, 0 );
    if ( sock < 0 )
    {
        WIFI_LOG(WIFI_DEBUG ,("create socket fail\n"));
        return NET_WLAN_ERROR;
    }

    x_memset(P2pPerEntry, 0, sizeof(P2pPerEntry));

    for ( i=0, j= 0; i<NumOfEntry; i++)
    {
        if (EepPerEntry[i].is_valid)
        {
            P2pPerEntry[j].bValid = EepPerEntry[i].is_valid;
            P2pPerEntry[j].MyRule = EepPerEntry[i].my_p2p_rule;
            x_memcpy (P2pPerEntry[j].Addr, EepPerEntry[i].peer_mac, ETH_ALEN);
#if P2P_DEVICENAME_IN_PERSISTAB
            x_memcpy(P2pPerEntry[j].DeviceName, EepPerEntry[i].DeviceName, sizeof(P2pPerEntry[j].DeviceName));
#endif
            x_memcpy (P2pPerEntry[j].Profile.MacAddr, EepPerEntry[i].ap_mac, ETH_ALEN);
            P2pPerEntry[j].Profile.KeyIndex = EepPerEntry[i].key_index;
            P2pPerEntry[j].Profile.AuthType = EepPerEntry[i].auth_type;
            P2pPerEntry[j].Profile.EncrType = EepPerEntry[i].encr_type;
            x_memcpy(P2pPerEntry[j].Profile.SSID.Ssid,  EepPerEntry[i].ssid, EepPerEntry[i].ssid_len);
            P2pPerEntry[j].Profile.SSID.SsidLength = EepPerEntry[i].ssid_len;
            x_memcpy(P2pPerEntry[j].Profile.Key,  EepPerEntry[i].key, 64);
            P2pPerEntry[j].Profile.KeyLength = EepPerEntry[i].key_len;
            j++;
        }
    }

    strncpy(wrq.ifr_name, p2p_name, WLAN_WLAN_IF_MAX_LEN);

    wrq.u.data.pointer = (void *) P2pPerEntry;
    wrq.u.data.length = sizeof(P2pPerEntry);
    wrq.u.data.flags = OID_802_11_P2P_PERSISTENT_TABLE | OID_GET_SET_TOGGLE;

    rc = ioctl(sock, RT_PRIV_IOCTL, &wrq);
    if ( rc == -1 )
    {
        WIFI_LOG(WIFI_DEBUG ,("socket %d ioctl fail",sock));
        perror("ioctl failure: ");
        close( sock );
        return NET_WLAN_ERROR;
    }

    close( sock );
    return NET_WLAN_OK;


}


INT32 c_net_wlan_getPersistentTable(char* mac,PERSISTENT_ENTRY_T* per_entry)
{
    WIFI_LOG_ENTRY(WIFI_API, (""));
    INT32 i4Ret = NET_WLAN_OK;
    PRT_P2P_PERSISTENT_ENTRY pP2pPerTab = NULL;
    int replylen = sizeof(RT_P2P_PERSISTENT_ENTRY)*MAX_P2P_TABLE_SIZE;
    if (per_entry == NULL)
    {
        i4Ret = NET_WLAN_ERROR;
        return i4Ret;
    }
    pP2pPerTab = (PRT_P2P_PERSISTENT_ENTRY)malloc(replylen);
    if (pP2pPerTab == NULL)
    {
        i4Ret = NET_WLAN_ERROR;
        return i4Ret;
    }
    x_memset(pP2pPerTab,0,replylen);
    per_entry->NumOfEntry = 0;
    per_entry->eep_per_entry = _per_entry;

    EEPROM_PER_ENTRY_T* eep_per_entry = per_entry->eep_per_entry;
    if (dop2pStringCommand("GET p2p_per_tab", (char *)pP2pPerTab, replylen) < 0)
    {
        WIFI_LOG(WIFI_DEBUG ,(">>>>>>>p2p get p2p_per_tab return error!\n" ));
        i4Ret = NET_WLAN_ERROR;
        free(pP2pPerTab);
        return i4Ret;
    }

    int idx = 0;
    int found = 0;
    if (mac)
    {
        for (idx = 0; idx< MAX_P2P_TABLE_SIZE; idx++)
        {
            if (!pP2pPerTab[idx].bValid)
                continue;

            if (memcmp(mac,pP2pPerTab[idx].Addr,6) == 0)
            {
                per_entry->NumOfEntry = 1;
                eep_per_entry[found].is_valid = pP2pPerTab[idx].bValid;
                eep_per_entry[found].my_p2p_rule =  pP2pPerTab[idx].MyRule;
                x_memcpy (eep_per_entry[found].peer_mac,pP2pPerTab[idx].Addr , ETH_ALEN);
                x_memcpy (eep_per_entry[found].ap_mac,pP2pPerTab[idx].Profile.MacAddr , ETH_ALEN);
#if P2P_DEVICENAME_IN_PERSISTAB
                x_memset(eep_per_entry[found].DeviceName, 0, sizeof(eep_per_entry[found].DeviceName));
                x_memcpy(eep_per_entry[found].DeviceName, pP2pPerTab[idx].DeviceName, sizeof(eep_per_entry[found].DeviceName) - 1);
#endif

                eep_per_entry[found].key_index = pP2pPerTab[idx].Profile.KeyIndex;
                eep_per_entry[found].auth_type = pP2pPerTab[idx].Profile.AuthType;
                eep_per_entry[found].encr_type = pP2pPerTab[idx].Profile.EncrType;
                x_memcpy(eep_per_entry[found].ssid, pP2pPerTab[idx].Profile.SSID.Ssid , pP2pPerTab[idx].Profile.SSID.SsidLength);
                eep_per_entry[found].ssid_len = pP2pPerTab[idx].Profile.SSID.SsidLength;
                x_memcpy(eep_per_entry[found].key, pP2pPerTab[idx].Profile.Key , 64);
                eep_per_entry[found].key_len = pP2pPerTab[idx].Profile.KeyLength;
            }
        }
    }
    else
    {
        x_memset(eep_per_entry,0,sizeof(EEPROM_PER_ENTRY_T)*MAX_P2P_TABLE_SIZE);
        for (idx = 0; idx< MAX_P2P_TABLE_SIZE; idx++)
        {
            if (!pP2pPerTab[idx].bValid)
                continue;

            eep_per_entry[found].is_valid = pP2pPerTab[idx].bValid;
            eep_per_entry[found].my_p2p_rule =  pP2pPerTab[idx].MyRule;
            x_memcpy (eep_per_entry[found].peer_mac,pP2pPerTab[idx].Addr , ETH_ALEN);
            x_memcpy (eep_per_entry[found].ap_mac,pP2pPerTab[idx].Profile.MacAddr , ETH_ALEN);
#if P2P_DEVICENAME_IN_PERSISTAB
            x_memset(eep_per_entry[found].DeviceName, 0, sizeof(eep_per_entry[found].DeviceName));
            x_memcpy(eep_per_entry[found].DeviceName, pP2pPerTab[idx].DeviceName, sizeof(eep_per_entry[found].DeviceName) - 1);
#endif

            eep_per_entry[found].key_index = pP2pPerTab[idx].Profile.KeyIndex;
            eep_per_entry[found].auth_type = pP2pPerTab[idx].Profile.AuthType;
            eep_per_entry[found].encr_type = pP2pPerTab[idx].Profile.EncrType;
            x_memcpy(eep_per_entry[found].ssid, pP2pPerTab[idx].Profile.SSID.Ssid , pP2pPerTab[idx].Profile.SSID.SsidLength);
            eep_per_entry[found].ssid_len = pP2pPerTab[idx].Profile.SSID.SsidLength;
            x_memcpy(eep_per_entry[found].key, pP2pPerTab[idx].Profile.Key , 64);
            eep_per_entry[found].key_len = pP2pPerTab[idx].Profile.KeyLength;

            found ++;
        }

        WIFI_LOG (WIFI_DEBUG,(">>>> table number %d", found));
        per_entry->NumOfEntry = found;
    }

    free(pP2pPerTab);
    return i4Ret;


}
/* delete entry in persistent table *
** mac: mac of specified entry want to delete, FF:FF:FF:FF:FF:FF delete all entry in persistent table *
*/
INT32 c_net_wlan_deletePersistentEntry(UINT8 *mac)  //for Sony
{
    WIFI_LOG_ENTRY(WIFI_API, (" MAC =%02x:%02x:%02x:%02x:%02x:%02x.", *(mac), *(mac+1), *(mac+2), *(mac+3),
                              *(mac+4), *(mac+5)));

    char MAC[18] = {0};
    char *del_tab_mac = "ff:ff:ff:ff:ff:ff";
    struct iwreq wrq;
    int sock;
    int rc;

    sock = socket(AF_INET, SOCK_STREAM, 0 );
    if ( sock < 0 )
    {
        WIFI_LOG(WIFI_ERROR, ("c_net_wlan_deletePersistentEntry Create socket failed %s" ,strerror(errno)));
        return NET_WLAN_ERROR;
    }

    snprintf((char *)MAC, sizeof(MAC), "%02x:%02x:%02x:%02x:%02x:%02x", *(mac), *(mac+1), *(mac+2), *(mac+3),
             *(mac+4), *(mac+5));

    x_memset(&wrq, 0, sizeof(wrq));
    strncpy(wrq.ifr_name, p2p_name, WLAN_WLAN_IF_MAX_LEN);
    wrq.u.data.pointer = (void *) MAC;
    wrq.u.data.length = sizeof(MAC);

    if (strncmp(MAC, del_tab_mac, sizeof(MAC) -1) == 0)  /* delete persistent table */
    {
        wrq.u.data.flags = OID_DELETE_PERSISTENT_TABLE | OID_GET_SET_TOGGLE;
    }
    else    /* delete a persistent entry in persistent table */
    {
        wrq.u.data.flags = OID_DELETE_PERSISTENT_ENTRY | OID_GET_SET_TOGGLE;
    }

    rc = ioctl(sock, RT_PRIV_IOCTL, &wrq);
    if ( rc == -1 )
    {
        WIFI_LOG(WIFI_DEBUG, ("ioctl fail: %s(%d): %s", __FILE__, __LINE__, strerror(errno)));
        close( sock );
        return NET_WLAN_ERROR;
    }

    close( sock );
    return NET_WLAN_OK;
}


/*------------------------------------------------------------------*/
/*! @brief: Get all network ids of persistent networks
*  @param [in] networkId_buf - the address of buffer stored network_id
*  @param [in] networkIdbuflen - the length of the buffer stored network_id
*  @param [out] networkNum - the address of total number of persistent table
*  @return     result
*  @retval     0 -success
*  @retval     otherwise -failure 
*/
/*------------------------------------------------------------------*/
INT32 c_net_wlan_p2p_cfg80211_get_persistent_network_ids(int *networkId_buf, 
    int networkIdbuflen, int *networkNum)
{
    WIFI_LOG_ENTRY(WIFI_API, (""));

    return x_net_wlan_cfg80211_get_persistent_network_id(networkId_buf, networkIdbuflen, networkNum);
}

/*------------------------------------------------------------------*/
/*! @brief: Save configuration for wpa_supplicant
*  @param - none
*  @return     result
*  @retval     0 -success
*  @retval     otherwise -failure 
*/
/*------------------------------------------------------------------*/
INT32 c_net_wlan_p2p_cfg_save_conf(void)
{
    WIFI_LOG_ENTRY(WIFI_API, (""));
    
    char cmd[128] = {'\0'};

    snprintf(cmd, sizeof(cmd), "SAVE_CONFIG");
    if (!dop2pBooleanCommand(cmd, "OK")) {
        WIFI_LOG(WIFI_DEBUG ,("Run command: \"%s\" return error", cmd));
        return -1;
    }

    return 0;
}


INT32 c_net_wlan_p2p_associate_table(P2P_ASSOC_ENTRY_T *AssocTab)
{
    WIFI_LOG_ENTRY(WIFI_API, (""));

    INT32 i4Ret = NET_WLAN_OK;
    char _cmd_buf[16] = {0};
    PP2P_STA_ASSOC_LIST pP2pAssocTab = NULL;
    int replylen = sizeof(P2P_STA_ASSOC_LIST);
    if (AssocTab == NULL)
    {
        i4Ret = NET_WLAN_ERROR;
        return i4Ret;
    }
    pP2pAssocTab = (PP2P_STA_ASSOC_LIST)malloc(replylen);
    if (pP2pAssocTab == NULL)
    {
        i4Ret = NET_WLAN_ERROR;
        return i4Ret;
    }
    AssocTab->NumOfEntry = 0;
    AssocTab->p2p_assoc_per_entry = _p2p_assoc_per_entry;
    P2P_ASSOC_PER_ENTRY_T * assoc_per_entry = AssocTab->p2p_assoc_per_entry;

    snprintf(_cmd_buf ,sizeof(_cmd_buf), "P2P_ASSOC_TAB");
    if (dop2pStringCommand(_cmd_buf, (char*)pP2pAssocTab, sizeof(P2P_STA_ASSOC_LIST)) == -1)
    {
        WIFI_LOG (WIFI_DEBUG,(">>>>>> c_net_wlan_p2p_associate_table  return error!\n" ));
        i4Ret = NET_WLAN_ERROR;
        free(pP2pAssocTab);
        return i4Ret;
    }
    x_memset(AssocTab->p2p_assoc_per_entry,0,sizeof(P2P_ASSOC_PER_ENTRY_T)*P2P_MAX_ASSOC_TAB);

    if (pP2pAssocTab->maclist_count !=0)
    {
        int i = 0;
        AssocTab->NumOfEntry = pP2pAssocTab->maclist_count;
        for (i = 0; i< pP2pAssocTab->maclist_count && i < P2P_MAX_ASSOC_TAB; i++)
        {
            x_memcpy(assoc_per_entry[i].peer_mac,pP2pAssocTab->maclist[i].octet,ETH_ALEN);
            x_memcpy(assoc_per_entry[i].dev_name,pP2pAssocTab->device_name[i].ssid,sizeof(pP2pAssocTab->device_name[i].ssid));
            x_memcpy(assoc_per_entry[i].dev_type,pP2pAssocTab->device_type[i].dev_type,sizeof(pP2pAssocTab->device_type[i].dev_type));
            assoc_per_entry[i].is_wfd = pP2pAssocTab->is_wfd[i];
            assoc_per_entry[i].is_p2p = pP2pAssocTab->is_p2p[i];
            assoc_per_entry[i].rtsp_port = pP2pAssocTab->rtsp_port[i];
            assoc_per_entry[i].i4rssi = u1WlanDrv_NormalizeSignal(pP2pAssocTab->rssi[i]);

            WIFI_LOG (WIFI_DEBUG,("p2p_assoc_tab num %d dev_name %s \
				mac = %02x:%02x:%02x:%02x:%02x:%02x,rssi %hhd signal_level %d, rtsp_port %d\n",
                                  i+1,assoc_per_entry[i].dev_name,
                                  assoc_per_entry[i].peer_mac[0],assoc_per_entry[i].peer_mac[1],assoc_per_entry[i].peer_mac[2],
                                  assoc_per_entry[i].peer_mac[3],assoc_per_entry[i].peer_mac[4],assoc_per_entry[i].peer_mac[5],
                                  (int)pP2pAssocTab->rssi[i],(int)assoc_per_entry[i].i4rssi,(int)assoc_per_entry[i].rtsp_port));
        }
    }
    free(pP2pAssocTab);
    return i4Ret;
}

INT32 c_net_wlan_wfd_setwfdsessionavail(BOOL enabled)
{
    char cmd[CMD_BUF_LEN] = {0};
    int value = (enabled) ? 1 : 0;
    snprintf(cmd, sizeof(cmd), "SET wfd_sessionAvail %d", value);

    BOOL b_ret = dop2pBooleanCommand(cmd,"OK");
    if (!b_ret)
    {
        WIFI_LOG (WIFI_DEBUG,(" c_net_wlan_wfd_setwfdsessionavail fail.\n"));
        return -1;
    }
    return 0;
}


INT32 c_net_wlan_p2p_set_HDCP(BOOL flag)
{
    char _cmd_buf[64]={'\0'};
    int value = (flag) ? 1 : 0;
    snprintf(_cmd_buf, sizeof(_cmd_buf), "SET wfd_contentProtect %d", value);

    BOOL b_ret = dop2pBooleanCommand(_cmd_buf,"OK");
    if (!b_ret)
    {
        WIFI_LOG (WIFI_DEBUG,(" c_net_wlan_p2p_set_HDCP fail.\n"));
        return -1;
    }
    return 0;
}

INT32 c_net_wlan_p2p_check_peer_channel(int value)
{
    WIFI_LOG_ENTRY(WIFI_API, ("channel policy %d",value));
    char cmd[CMD_BUF_LEN] = {0};
    snprintf(cmd,sizeof(cmd), "SET p2p_check_peer_channel %d", value);

    BOOL b_ret = dop2pBooleanCommand(cmd,"OK");
    if (!b_ret)
    {
        WIFI_LOG (WIFI_DEBUG,(" c_net_wlan_p2p_check_peer_channel fail.\n"));
        return -1;
    }
    return 0;
}
INT32 c_net_wlan_p2p_get_mac_addr(UINT8 *pMacAddr)
{
    INT32 i4Ret = NET_WLAN_OK;

    WIFI_LOG_ENTRY(WIFI_API, (""));

    struct ifreq ifr;
    int sockfd;
    sockfd = socket(AF_INET,SOCK_DGRAM,0);
    if (sockfd < 0)
    {
        WIFI_LOG (WIFI_DEBUG,("socket() fail, errorno=%d, @%s()\n", errno, __FUNCTION__));
        return -1;
    }

    strncpy(ifr.ifr_name, p2p_name, WLAN_WLAN_IF_MAX_LEN);
    if (ioctl(sockfd,SIOCGIFHWADDR,&ifr) == -1)
    {
        WIFI_LOG (WIFI_DEBUG,("ioctl() fail, errorno=%d, @%s()\n", errno, __FUNCTION__));
        i4Ret = NET_WLAN_DRV_INTERFACE_ERR;
        close(sockfd);
        return i4Ret;
    }
    x_memcpy(pMacAddr,ifr.ifr_hwaddr.sa_data,6);
    close(sockfd);
    WIFI_LOG (WIFI_DEBUG,("p2p mac address = %02X:%02X:%02X:%02X:%02X:%02X.\n\r", pMacAddr[0],pMacAddr[1],pMacAddr[2],pMacAddr[3],pMacAddr[4],pMacAddr[5]));
    return i4Ret;
}

INT32 c_net_wlan_p2p_set_ageout(char *mac, int time)
{
    char _cmd_buf[WFD_CMD_BUF_LEN]={'\0'};
    snprintf(_cmd_buf,sizeof(_cmd_buf), "P2P_CLIENT_AGEOUT %s %d", mac,time);
    if (!dop2pBooleanCommand(_cmd_buf, "OK"))
    {
        WIFI_LOG(WIFI_ERROR,(">>>>>>>p2p set p2p_set_ageout return error!\n"));
        return FALSE;
    }
    else
    {
        WIFI_LOG(WIFI_ERROR,(">>>>>>>p2p set p2p_set_ageout %s %d\n", mac,time));
        return TRUE;
    }
}


INT32 c_net_wlan_p2p_display_port_getpin(const char * mac )
{
    return c_net_wlan_p2p_connect(P2P_CONNECT_MODE_PIN_KEYPAD,mac,NULL);
}
INT32 c_net_wlan_p2p_keypad_port_displaypin(const char * mac)
{
    return c_net_wlan_p2p_connect(P2P_CONNECT_MODE_PIN_DISPLAY,mac,NULL);
}

INT32 c_net_wlan_set_auto_go(int fgEnableAutoGo, const char * ssid, const char * psk)
{
    INT32 ret;
    if (fgEnableAutoGo)
    {
        ret = c_net_wlan_p2p_groupAdd_withssid(ssid,psk);
    }
    else
    {
        ret = c_net_wlan_p2p_groupRemove("p2p0");
    }
    return ret;
}

INT32 c_net_wlan_p2p_keypad_port_pinconnect(const char * mac, const char * pin)
{
    return c_net_wlan_p2p_pin_connect( mac,pin);
}
INT32 c_net_wlan_p2p_display_pin_connect(const char * mac, const char * pin)
{
    WIFI_LOG_ENTRY(WIFI_API,(""));
    INT32 i4Ret = NET_WLAN_OK;
    if ((mac == NULL) || (pin == NULL) || (x_strlen(mac)!= 17))
    {
        WIFI_LOG(WIFI_WARNING,(">>>> c_net_wlan_p2p_display_pin_connect wrong param!\n"));
        return NET_WLAN_INVALID_PARAM;
    }
    x_memset(wfd_connected_mac, 0 , 18);
    x_strncpy(wfd_connected_mac ,mac ,17);

    char cmd[CMD_BUF_LEN] = {0};
    snprintf(cmd,sizeof(cmd),"P2P_CONNECT %s %s display",mac,pin);
    if (!dop2pBooleanCommand(cmd, "OK"))
    {
        i4Ret = NET_WLAN_ERROR;
        WIFI_LOG(WIFI_ERROR,(">>>>>> c_net_wlan_p2p_display_pin_connect command return error!\n"));
    }
    return i4Ret;
}
INT32 c_net_wlan_p2p_set_dev_type( const char * dev_type )
{
    INT32 i4Ret = NET_WLAN_OK;
    char cmd[CMD_BUF_LEN] = {0};
    if (!dev_type)
    {
        WIFI_LOG (WIFI_DEBUG,(">>>>>>> set_dev_name error! invalid parameter!\n"));
        return NET_WLAN_INVALID_PARAM;
    }
    snprintf(cmd, sizeof(cmd), "SET device_type %s", dev_type);
    if (!dop2pBooleanCommand(cmd, "OK"))
    {
        WIFI_LOG (WIFI_DEBUG,("[WIFI_MW] c_net_wlan_p2p_set_dev_type fail.\n"));
        return NET_WLAN_ERROR;
    }
    return i4Ret;
}


INT32 c_net_wlan_monitor_dhcpd_lease(void* pIn) //need to sync
{
    return x_net_wlan_monitor_dhcpd_lease(pIn);
}

INT32 c_net_wlan_cancel_moniter_ip(void)
{
    x_net_wlan_cancel_moniter_ip();
    return NET_WLAN_OK;
}

INT32 c_net_wlan_p2p_get_random_ssid(char *ssid, int buflen)
{
    WIFI_LOG_ENTRY(WIFI_API, (""));

    INT32 i4Ret = -1;

    if (!ssid)
    {
        WIFI_LOG(WIFI_WARNING, ("Invalid parameter, %s", __FUNCTION__));
        return -1;
    }

    if (buflen < 9) /* DIRECT-xy */
    {
        WIFI_LOG(WIFI_WARNING, ("buffer length is too small, it must be no less than 9 bytes. %s", __FUNCTION__));
        return -1;
    }

    i4Ret = _P2P_Get_Random_SSID(ssid, buflen);
    if (i4Ret != 0)
    {
        WIFI_LOG(WIFI_ERROR, ("Get random SSID failed, %s", __FUNCTION__));
        return -1;
    }

    return 0;
}


INT32 c_net_wlan_p2p_get_random_key(char *key, int buflen)
{
    WIFI_LOG_ENTRY(WIFI_API, (""));

    INT32 i4Ret = -1;

    if (!key)
    {
        WIFI_LOG(WIFI_WARNING, ("Invalid parameter, %s", __FUNCTION__));
        return -1;
    }

    if (buflen < 8) /* the shortest length of key is 8 bytes */
    {
        WIFI_LOG(WIFI_WARNING, ("buffer length is too small, it must be no less than 8 bytes. %s", __FUNCTION__));
        return -1;
    }

    i4Ret = _P2P_Get_Random_Key(key, buflen);
    if (i4Ret != 0)
    {
        WIFI_LOG(WIFI_ERROR, ("Get random key failed, %s", __FUNCTION__));
        return -1;
    }

    return 0;
}
#if 0
/* -----------------------------------------------------------------
convert ssid, passphrase and salt(4096) into a crypto key.
ssid: ssid reference
ssid_len: the length of ssid
passphrase_len: the length of passpharse
passphrase: passphrase reference
PSK: crypto key reference
PSK_len: specified length of crypto key, must be 64
-------------------------------------------------------------------- */
INT32 c_net_wlan_calculate_PSK_from_ssid_passphrase(
    char *passphrase, int passphrase_len, char *ssid, int ssid_len, char *PSK, int PSK_len)
{
    WIFI_LOG_ENTRY(WIFI_API, ("passphrase=%s, passphrase_len=%d, ssid=%s, ssid_len=%d, PSK_len=%d",
                              passphrase, passphrase_len, ssid, ssid_len, PSK_len));

    unsigned char key[32] = {0};
    char psktmp[65] = {'\0'};
    char *p = NULL;
    int i = 0;

    if (!ssid || !passphrase)
    {
        WIFI_LOG(WIFI_ERROR, ("Invalid parameters, %s", __FUNCTION__));
        return -1;
    }

    if (ssid_len > 32)
    {
        WIFI_LOG(WIFI_ERROR, ("SSID must be no more than 32 characters."));
        return -1;
    }

    if (passphrase_len < 8 || passphrase_len > 63)
    {
        WIFI_LOG(WIFI_ERROR, ("Passphrase must be 8..63 characters."));
        return -1;
    }

    if (PSK_len > 64)
    {
        WIFI_LOG(WIFI_ERROR, ("PSK length must be no more than 64 bytes."));
        return -1;
    }
#if PLATFORM_IS_BDP
    PKCS5_PBKDF2_HMAC_SHA1(passphrase, passphrase_len, (unsigned char *)ssid, ssid_len, 4096, 32, key);
#endif

    p = psktmp;
    for (i = 0; i < 32; i++)
    {
        snprintf(p, sizeof(psktmp) - strlen(psktmp), "%02x", key[i]);
        p = psktmp + strlen(psktmp);
    }

    psktmp[64] = '\0';

    x_memcpy(PSK, psktmp, PSK_len);

    WIFI_LOG(WIFI_DEBUG, ("PSK length=%d, PSK=%s\n", strlen(psktmp), psktmp));

    return 0;
}
#endif
INT32 c_net_wlan_p2p_up_do(void)
{
    INT32 i4Ret = NET_WLAN_OK;
    char cmd[60] = {0};
    snprintf(cmd, sizeof(cmd), "iwpriv p2p0 set p2pWscMode=2\n");
    WIFI_LOG (WIFI_DEBUG,(">>>> cmd->%s", cmd));
    doScriptCmd(cmd);
    x_memset(cmd , 0 , 60);
    snprintf(cmd, sizeof(cmd), "iwpriv p2p0 set p2pWscConf=3\n");
    WIFI_LOG (WIFI_DEBUG,(">>>> cmd->%s", cmd));
    doScriptCmd(cmd);
    return i4Ret;
}
INT32 c_net_wlan_p2p_unreg_cbk(x_net_wlan_notify_fct pfNotify)
{
    WIFI_LOG_ENTRY(WIFI_API,(""));
    int i ;
    if (pfNotify == NULL )//NULL notify , ignore it .
    {
        return 0;
    }
    for ( i = 0; i < MAX_CBK_NUM ; i++)
    {
        if (_p2p_cbk_list[i] == pfNotify ) //alread in cbk list ,do not insert it again .
        {
            WIFI_LOG(WIFI_DEBUG ,(">>>>>>>p2p unreg cbk notify function ok, removed!\n"));
            _p2p_cbk_list[i] = NULL;
            return 0;
        }
    }

    WIFI_LOG(WIFI_DEBUG,("c_net_wlan_p2p_unreg_cbk list is full ,max value is %d!\n",MAX_CBK_NUM));
    return -1;
}
INT32 c_net_wlan_set_auto_accept(BOOL enable)
{
    char _cmd_buf[32]={0};
    int value = enable?1:0;
    snprintf(_cmd_buf , sizeof(_cmd_buf), "SET p2p_auto_accept %d",value);

    if (!dop2pBooleanCommand(_cmd_buf, "OK"))
    {
        WIFI_LOG (WIFI_DEBUG,( ">>>>>>> %s fail,\n",_cmd_buf));
        return NET_WLAN_ERROR;
    }
    return NET_WLAN_OK;
}

INT32 c_net_wlan_enable_wac_config(BOOL enable)
{
#ifdef CONFIG_WLAN_SW_ENABLE_WAC
    char ps_cmd[64];
    if (enable)
    {
        snprintf(ps_cmd, sizeof(ps_cmd), "iwpriv p2p0 set airplayEnable=1");
        doScriptCmd(ps_cmd);
        snprintf(ps_cmd, sizeof(ps_cmd), "iwpriv p2p0 set P2pOpMode=1");
        doScriptCmd(ps_cmd);
        snprintf(ps_cmd, sizeof(ps_cmd), "iwpriv p2p0 set AuthMode=OPEN");
        doScriptCmd(ps_cmd);
        snprintf(ps_cmd, sizeof(ps_cmd), "iwpriv p2p0 set EncrypType=NONE");
        doScriptCmd(ps_cmd);
    }
    else
    {
        snprintf(ps_cmd, sizeof(ps_cmd), "iwpriv p2p0 set airplayEnable=0");
        doScriptCmd(ps_cmd);
        snprintf(ps_cmd, sizeof(ps_cmd), "iwpriv p2p0 set P2pOpMode=0");
        doScriptCmd(ps_cmd);
        snprintf(ps_cmd, sizeof(ps_cmd), "iwpriv p2p0 set AuthMode=WPA2PSK");
        doScriptCmd(ps_cmd);
        snprintf(ps_cmd, sizeof(ps_cmd), "iwpriv p2p0 set EncrypType=AES");
        doScriptCmd(ps_cmd);
    }
#endif
    return NET_WLAN_OK;
}


INT32 c_net_wlan_press_push_button(void)
{
    return NET_WLAN_OK;
}

#ifdef WLAN_SUPPORT_P2P_CFG80211
/*------------------------------------------------------------------*/
/*! @brief: Reconfigure the P2P by new configuration
*  @param  - none
*  @return     result
*  @retval     0 -success
*  @retval     otherwise -failure
*/
/*------------------------------------------------------------------*/
INT32 c_net_wlan_p2p_cfg_reconfigure(void)
{
    WIFI_LOG_ENTRY(WIFI_API, (""));

    char cmd[128] = {'\0'};
    snprintf(cmd, sizeof(cmd), "RECONFIGURE");
    if (!dop2pBooleanCommand(cmd, "OK"))
    {
        WIFI_LOG(WIFI_DEBUG ,("Run command: \"%s\" return error", cmd));
        return NET_WLAN_ERROR;
    }

    return NET_WLAN_OK;
}


INT32 c_net_wlan_cfg80211_get_persistent_group(PERSISTENT_ENTRY_T* per_entry)
{
    WIFI_LOG_ENTRY(WIFI_API, (""));

    INT32 i4Ret = NET_WLAN_OK;
    int networkid = -1;
    int i = 0, count = 0;
    UINT8 peerMac[ETH_ALEN] = {0};
    char macStr[MAC_STR_LEN + 1] = {'\0'};
    UINT8 bssid[ETH_ALEN] = {0};
    char ssid[NET_802_11_MAX_LEN_SSID + 1] = {'\0'};
    int ssidLen = 0;
    char _cmd_buf[128] = {'\0'};
    char reply[512] = {'\0'};
    int networkId[MAX_P2P_TABLE_SIZE];
    int networkNum = 0;
    char *pMac = NULL;

    if ( !per_entry )
    {
        WIFI_LOG (WIFI_DEBUG,("Invalid parameters, %s \n", __FUNCTION__ ));
		return -1;
    }

    per_entry->NumOfEntry = 0;
    per_entry->eep_per_entry = _per_entry;

    memset(networkId, -1, sizeof(networkId));

    i4Ret = x_net_wlan_cfg80211_get_persistent_network_id(networkId, sizeof(networkId), &networkNum);
    if ( i4Ret != NET_WLAN_OK )
    {
        WIFI_LOG (WIFI_DEBUG,("No persistent group, %s\n", __FUNCTION__ ));
        return -1;
    }

    count = 0;
    memset(_per_entry, 0, sizeof(_per_entry));

    for (i = 0; i < networkNum; i++)
    {
        networkid = networkId[i];
        memset(bssid, 0, sizeof(bssid));
        memset(ssid, 0, sizeof(ssid));

        /* get bssid in persistent group */
        memset(_cmd_buf, 0, sizeof(_cmd_buf));
        memset(reply, 0, sizeof(reply));
        snprintf(_cmd_buf ,sizeof(_cmd_buf), "GET_NETWORK %d bssid", networkid);
        if (dop2pStringCommand(_cmd_buf, reply, sizeof(reply)) == -1)
        {
            WIFI_LOG (WIFI_DEBUG,("cmd \"%s\" return error!\n", _cmd_buf ));
        }
        str_to_mac(reply, bssid);

        WIFI_LOG (WIFI_DEBUG,("persistent group: bssid=" MACSTR ", %s\n", MAC2STR(bssid), __FUNCTION__ ));

        /* get ssid in persistent group */
        memset(_cmd_buf, 0, sizeof(_cmd_buf));
        memset(reply, 0, sizeof(reply));
        snprintf(_cmd_buf ,sizeof(_cmd_buf), "GET_NETWORK %d ssid", networkid);
        if (dop2pStringCommand(_cmd_buf, reply, sizeof(reply)) == -1)
        {
            WIFI_LOG (WIFI_DEBUG,("cmd \"%s\" return error!\n", _cmd_buf ));
        }
        strncpy(ssid, reply, sizeof(ssid) - 1);
        ssidLen = strlen(ssid);

        WIFI_LOG (WIFI_DEBUG,("persistent group: ssid=%s, %s\n", ssid, __FUNCTION__ ));

        /* get all p2p clients in persistetn group */
        memset(_cmd_buf, 0, sizeof(_cmd_buf));
        memset(reply, 0, sizeof(reply));
        snprintf(_cmd_buf ,sizeof(_cmd_buf), "GET_NETWORK %d p2p_client_list", networkid);
        if (dop2pStringCommand(_cmd_buf, reply, sizeof(reply)) == -1)
        {
            WIFI_LOG (WIFI_DEBUG,("cmd \"%s\" return error!\n", _cmd_buf ));
            continue;
        }

        WIFI_LOG (WIFI_DEBUG,("persistent group: client list=%s, %s\n", reply, __FUNCTION__ ));

        pMac = reply;
        for (; count < MAX_P2P_TABLE_SIZE && strlen(pMac) >= MAC_STR_LEN; count++)
        {
            _per_entry[count].my_p2p_rule = 1;
            /* get peer mac */
            memset(macStr, 0, sizeof(macStr));
            strncpy(macStr, pMac, MAC_STR_LEN);
            str_to_mac(macStr, peerMac);
            memcpy(_per_entry[count].peer_mac, peerMac, ETH_ALEN);
            pMac = pMac + MAC_STR_LEN + 1;

            memcpy(_per_entry[count].ap_mac, bssid, ETH_ALEN);
            strncpy(_per_entry[count].ssid, ssid, NET_802_11_MAX_LEN_SSID);
            _per_entry[count].ssid_len = ssidLen;
            _per_entry[count].auth_type = 0x20;
            _per_entry[count].encr_type = 0x08;
        }
    }

    per_entry->NumOfEntry = count;

    WIFI_LOG (WIFI_DEBUG,(">>>> table persistent entry number %d", per_entry->NumOfEntry));

    return i4Ret;
}



INT32 c_net_wlan_cfg80211_delete_Persistent_Entry(UINT8 *mac)  //for Sony
{
    WIFI_LOG_ENTRY(WIFI_API, (" MAC =" MACSTR, MAC2STR(mac)));

    INT32 i4Ret = NET_WLAN_OK;
    int networkid = -1;
    int i = 0;
    char cmd[256] = {'\0'};
    int networkId[MAX_P2P_TABLE_SIZE];
    int networkNum = 0;
    int tempconnection = 0;

    if ( !mac )
    {
        WIFI_LOG (WIFI_DEBUG,("Invalid parameters, %s \n", __FUNCTION__ ));
		return -1;
    }

    memset(networkId, -1, sizeof(networkId));

    i4Ret = x_net_wlan_cfg80211_get_persistent_network_id(networkId, sizeof(networkId), &networkNum);
    if ( i4Ret != NET_WLAN_OK )
    {
        WIFI_LOG (WIFI_DEBUG,("No persistent group, %s\n", __FUNCTION__ ));
        return -1;
    }

    if ( x_net_p2p_check_init_state() == NET_WLAN_NOT_INIT )
    {
        memset(p2p_name, 0, sizeof(p2p_name));
        strncpy(p2p_name, "p2p0", sizeof(p2p_name) - 1);
        if ( wifi_open_connection(TRUE, WIFI_SOCKET_PATH) != 0)
        {
            WIFI_LOG(WIFI_ERROR,("[P2P] Failed to create temp connection with wpa_supplicant!\n"));
            return -1;
        }

        i4Ret = wifi_sema_init(&h_P2pCmdSema);
        WIFI_CHK_FAIL (i4Ret);

        tempconnection = 1;
    }

    if ( mac[0] == 0xff &&
            mac[1] == 0xff &&
            mac[2] == 0xff &&
            mac[3] == 0xff &&
            mac[4] == 0xff &&
            mac[5] == 0xff )
    {
        for (i = 0; i < networkNum; i++)
        {
            networkid = networkId[i];
            snprintf(cmd ,sizeof(cmd), "REMOVE_NETWORK %d", networkid);
            if (!dop2pBooleanCommand(cmd, "OK"))
            {
                WIFI_LOG (WIFI_DEBUG,(">>>>>> %s  return error!\n", __FUNCTION__ ));
            }
        }

        memset(cmd, 0, sizeof(cmd));
        snprintf(cmd ,sizeof(cmd), "rm -f %s", CFG80211_WPA_SUPPLICANT_P2P_CONFIG_FILE);
        if (doScriptCmd (cmd) != 0 )
        {
            WIFI_LOG(WIFI_ERROR, ("Failed to remove file \"%s\"\n", CFG80211_WPA_SUPPLICANT_P2P_CONFIG_FILE));
        }
    }
    else
    {
        snprintf(cmd ,sizeof(cmd), "P2P_REMOVE_CLIENT_PER_STA_KEY " MACSTR, MAC2STR(mac));
        if (!dop2pBooleanCommand(cmd, "OK"))
        {
            WIFI_LOG (WIFI_DEBUG,(">>>>>> %s  return error!\n", __FUNCTION__ ));
            i4Ret = NET_WLAN_ERROR;
        }
    }

    if ( tempconnection == 1 )
    {
        x_net_p2p_close_connection();
        i4Ret = sema_uninit(&h_P2pCmdSema);
        WIFI_CHK_FAIL (i4Ret);
    }

    return i4Ret;
}


/*------------------------------------------------------------------*/
/*! @brief: Get mode of P2P network with specified network id
*  @param [in] network_id - the network id of P2P network which want to get the mode
*  @param [out] mode - the network mode, 3: GO
*  @return     result
*  @retval     0 -success
*  @retval     otherwise -failure
*/
/*------------------------------------------------------------------*/
INT32 c_net_wlan_p2p_cfg_get_network_mode(int network_id, int *mode)
{
    WIFI_LOG_ENTRY(WIFI_API, (""));

    if ( network_id < 0 || !mode )
    {
        WIFI_LOG(WIFI_DEBUG ,("Invalid Parameters, %s()", __FUNCTION__));
        return NET_WLAN_ERROR;
    }

    char cmd[128] = {'\0'};
    char reply[8] = {'\0'};
    *mode = -1;

    snprintf(cmd, sizeof(cmd), "GET_NETWORK %d mode", network_id);
    dop2pStringCommand(cmd, reply, sizeof(reply));
    if ( reply[0] < '0' || reply[0] > '9' )
        return -1;

    *mode = atoi(reply);
    WIFI_LOG(WIFI_DEBUG,("reply=%s, mode=%d", reply, *mode));

    return 0;
}


/*------------------------------------------------------------------*/
/*! @brief: Get p2p client list of a P2P network
*  @param [in] network_id - the network id of specified P2P network
*  @param [out] client_list - the buffer of p2p client list, the length is 180 or more.
*  @return     result
*  @retval     0 -success
*  @retval     otherwise -failure 
*/
/*------------------------------------------------------------------*/
INT32 c_net_wlan_p2p_cfg_get_client_list(int network_id, 
            char *client_list)
{
    WIFI_LOG_ENTRY(WIFI_API, (""));

    if ( network_id < 0 || !client_list ) {
        WIFI_LOG(WIFI_DEBUG ,("Invalid Parameters, %s()", __FUNCTION__));
        return NET_WLAN_ERROR;
    }

    char cmd[128] = {'\0'};
    char reply[256] = {'\0'};
    int ret = -1;

    snprintf(cmd, sizeof(cmd), "GET_NETWORK %d p2p_client_list", network_id);
    ret = dop2pStringCommand(cmd, reply, sizeof(reply));
    if ( ret < 0 ) {
        WIFI_LOG(WIFI_DEBUG ,("Failed to run cmd \"%s\", %s()", cmd, 
            __FUNCTION__));
        return NET_WLAN_ERROR;
    }

    WIFI_LOG(WIFI_DEBUG,("(network %d)p2p client list=%s", network_id, reply));
    strncpy(client_list, reply, sizeof(client_list) - 1);

    return 0; 
}


/*------------------------------------------------------------------*/
/*! @brief: Set priority of P2P and STA, when P2P connection is conflict with STA
*  @param [in] iface - the interface prefer to keep when there is a conflict, 0: STA, 1: P2P
*  @return     result
*  @retval     0 -success
*  @retval     otherwise -failure 
*/
/*------------------------------------------------------------------*/
INT32 c_net_wlan_p2p_cfg_set_connection_prefer(int iface)
{
    WIFI_LOG_ENTRY(WIFI_API, (""));

    if ( iface != 0 && iface != 1 ) {
        WIFI_LOG(WIFI_DEBUG ,("Invalid Parameters, %s()", __FUNCTION__));
        return NET_WLAN_ERROR;
    }

    char cmd[128] = {'\0'};
    char iface_name[16] = {'\0'};

    if ( iface == 0 ) {
        snprintf(iface_name, sizeof(iface_name), "sta");
    } else if ( iface == 1 ) {
        snprintf(iface_name, sizeof(iface_name), "p2p");
    }

    snprintf(cmd, sizeof(cmd), "P2P_SET conc_pref %s", iface_name);
    if ( !doBooleanCommand(cmd, "OK") ) {
        WIFI_LOG(WIFI_DEBUG ,("Failed to run cmd \"%s\", %s()", cmd, 
            __FUNCTION__));
        return NET_WLAN_ERROR;
    }

    return 0; 
}


/*------------------------------------------------------------------*/
/*! @brief: Statrt a new P2P Group or an exist one with specific network id
*  @param [in] network_id - the network id of P2P network, -1: start a new P2P Group,
                                              other: an exist P2P Group
*  @param [in] freq -the frequency which P2P Group start at, 0: not specify frequency,
                                                other: specify a frequency
*  @return     result
*  @retval     0 -success
*  @retval     otherwise -failure
*/
/*------------------------------------------------------------------*/
INT32 c_net_wlan_p2p_cfg_groupAdd_Persistent(int network_id,
        int freq)
{
    WIFI_LOG_ENTRY(WIFI_API, (""));
    int ch = -1;

    if ( network_id < -1 )
    {
        WIFI_LOG(WIFI_DEBUG ,("Invalid Parameters, %s()", __FUNCTION__));
        return NET_WLAN_ERROR;
    }

    char cmd[128] = {'\0'};

    if ( network_id == -1 )
        snprintf(cmd, sizeof(cmd), "P2P_GROUP_ADD persistent");
    else
        snprintf(cmd, sizeof(cmd), "P2P_GROUP_ADD persistent=%d", network_id);

    ch = frequency_to_channel(freq);
    if ( ch > 0 )
        snprintf(cmd + strlen(cmd), sizeof(cmd) - strlen(cmd), " freq=%d", freq);

    if (!dop2pBooleanCommand(cmd, "OK"))
    {
        WIFI_LOG(WIFI_DEBUG ,("Run command: \"%s\" return error", cmd));
        return -1;
    }

    return 0;
}

/*------------------------------------------------------------------*/
/*! @brief: Remove all P2P network with specified network id
*  @param [in] network_id - the pointer of network id, ("0", "1", "2" ... or "all")
*  @return     result
*  @retval     0 -success
*  @retval     otherwise -failure
*/
/*------------------------------------------------------------------*/
INT32 c_net_wlan_p2p_cfg_remove_network(char *network_id)
{
    WIFI_LOG_ENTRY(WIFI_API, (""));

    if ( !network_id )
    {
        WIFI_LOG(WIFI_DEBUG ,("Invalid Parameters, %s()", __FUNCTION__));
        return NET_WLAN_ERROR;
    }

    char cmd[128] = {'\0'};

    snprintf(cmd, sizeof(cmd), "REMOVE_NETWORK %s", network_id);
    if (!dop2pBooleanCommand(cmd, "OK"))
    {
        WIFI_LOG(WIFI_DEBUG ,("Run command: \"%s\" return error", cmd));
        return -1;
    }

    return 0;
}

INT32 c_net_wlan_p2p_cfg80211_auto_channel_selection(int auto_ch_select)
{
    WIFI_LOG_ENTRY(WIFI_API, ("cfg80211 mode auto channel selection flag=%d.", auto_ch_select));
    char _cmd_buf[WFD_CMD_BUF_LEN]={'\0'};

    snprintf(_cmd_buf, sizeof(_cmd_buf), "SET p2p_auto_channel_select %d", auto_ch_select);

    if (!dop2pBooleanCommand(_cmd_buf, "OK"))
    {
        WIFI_LOG(WIFI_ERROR,(">>>>>>>p2p set auto channel selection return error!\n"));
        return NET_WLAN_ERROR;
    }

    return NET_WLAN_OK;
}


/*------------------------------------------------------------------*/
/*! @brief: Connect to a P2P Device with a specific network id (start a persistent group)
*  @param [in] addr -peer device address
*  @param [in] pin - PIN code
*  @param [in] mode - the connection mode, such as "pbc", "display" or "keypad"
*  @param [in] network_id - the network id of P2P persistent group, -1: join an exist P2P Group; >=0: start a P2P Group
*  @return     result
*  @retval     0 -success
*  @retval     otherwise -failure
*/
/*------------------------------------------------------------------*/
INT32 c_net_wlan_p2p_cfg_accept_persistent_connection(
    char *addr, char *pin, INT32 mode, int network_id)
{
    WIFI_LOG_ENTRY(WIFI_API, (""));

    if ( !addr ||
            strlen(addr) != 17 ||
            ((mode == P2P_CONNECT_MODE_PIN_DISPLAY || mode == P2P_CONNECT_MODE_PIN_KEYPAD ) && !pin) )
    {
        WIFI_LOG(WIFI_DEBUG ,("Invalid Parameters, %s()", __FUNCTION__));
        return NET_WLAN_ERROR;
    }

    char cmd[256] = {'\0'};

    if ( mode == P2P_CONNECT_MODE_PBC )
        snprintf(cmd, sizeof(cmd), "P2P_CONNECT %s pbc", addr);
    else if ( mode == P2P_CONNECT_MODE_PIN_DISPLAY )
        snprintf(cmd, sizeof(cmd), "P2P_CONNECT %s %s display", addr, pin);
    else if ( mode == P2P_CONNECT_MODE_PIN_KEYPAD )
        snprintf(cmd, sizeof(cmd), "P2P_CONNECT %s %s keypad", addr, pin);
    else
    {
        WIFI_LOG(WIFI_DEBUG ,("Invalid Connection Mode, %s()", __FUNCTION__));
        return NET_WLAN_ERROR;
    }

    if ( network_id >= 0 )
        snprintf( cmd + strlen(cmd), sizeof(cmd) - strlen(cmd), " persistent=%d", network_id );
    else if ( network_id == -1 )
        snprintf( cmd + strlen(cmd), sizeof(cmd) - strlen(cmd), " persistent join" );
    else
    {
        WIFI_LOG(WIFI_DEBUG ,("Invalid network id, %s()", __FUNCTION__));
        return NET_WLAN_ERROR;
    }

    if (!dop2pBooleanCommand(cmd, "OK"))
    {
        WIFI_LOG(WIFI_DEBUG ,("Run command: \"%s\" return error.", cmd));
        return NET_WLAN_ERROR;
    }

    return NET_WLAN_OK;
}
#endif

/*add for service discovery*/
INT32 c_net_p2p_serv_disc_enable(BOOL enable)
{
	WIFI_LOG(WIFI_DEBUG, ("[WIFI MW]c_net_p2p_serv_disc_enable"));
	int i4Ret = x_net_p2p_serv_disc_enable (enable);
	WIFI_CHK_FAIL (i4Ret);
	return i4Ret;


}

INT32 c_net_p2p_serv_disc_resq_pre(INT16 protocolType,INT16 version, char *requestData, char *responseData)
{
	WIFI_LOG(WIFI_DEBUG, ("[WIFI MW]c_net_p2p_serv_disc_resq_pre"));
	int i4Ret = x_net_p2p_serv_disc_resq_pre (protocolType, version, requestData, responseData);
	WIFI_CHK_FAIL (i4Ret);
	return i4Ret;

}

INT32 c_net_p2p_serv_disc_req(char *mac, INT16 protocolType,INT16 version, char *requestData)
{
	WIFI_LOG(WIFI_DEBUG, ("[WIFI MW]c_net_p2p_serv_disc_req"));
	int i4Ret = x_net_p2p_serv_disc_req (mac, protocolType, version, requestData);
	WIFI_CHK_FAIL (i4Ret);
	return i4Ret;

}
/*add for service discovery*/

