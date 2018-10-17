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
 * $RCSfile: x_net_p2p_ctrl.c,v $
 * $Revision:
 * $Date:
 * $Author: yali.wu $
 *
 * Description:
 *         This file contains control API of Wifi Direct
 *---------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------
                    include files
-----------------------------------------------------------------------------*/
#include "inc/includes.h"
#include "inc/acfg_drv_if.h"
#include "inc/wps_defs.h"


#include "wifi_com.h"
#include "wpa_ctrl.h"
#include "u_os.h"


#include "x_net_p2p_ctrl.h"
#include "x_net_wlan_ctrl.h"

//#include "x_net_config.h"

#include "wifi_log.h"
#include "wlan_command.h"
#include "_net_wlan_init.h"

#include <sys/prctl.h>



#define P2P_THREAD_NAME            "P2P Thread"
#define P2P_STACK_SIZE             (1024 * 20)
#define P2P_THREAD_PRIORITY       100

#define WFD_SOURCE_DEVICE           1
#define WFD_SINK_DEVICE             2
#define WFD_INVALID_DEVICE          3
/*add for service discovery*/
#define ServiceProtocolType          2
/*add for service discovery*/

extern int u1WlanDrv_NormalizeSignal(int signal);
extern char   if_name[WLAN_WLAN_IF_MAX_LEN];
extern char   p2p_go_name[WLAN_WLAN_IF_MAX_LEN];
void p2p_parse_event(char * buf, size_t buf_len);

void p2p_notify_events(WLAN_NOTIFY_ID_T event , void * param );
x_net_wlan_notify_fct _p2p_cbk_list[MAX_CBK_NUM] = {NULL};

UINT32 wfd_p2p_port = 0 ;
BOOL p2p_for_wfd  = false ;
char wfd_connected_mac[18] = {0};
INT32 p2p_is_current_go = -1;

static INT32 p2p_is_current_forcego = -1;
static char p2p_go_ssid[33] = {0};
static char p2p_go_passphrase[65] = {0};

static struct stat g_ori_lease_stat = {0};



char str_ip[20]= {0} ;

struct stat _ori_stat = {0};

static BOOL cancel_moniter_ip = false;
BOOL fgProvisionReq = FALSE;
BOOL Is_for_listen = FALSE;

extern char p2p_name[WLAN_WLAN_IF_MAX_LEN];
extern char * if_name_map[WIFI_DIRECT_DEV_NUM][WIFI_COL_NUM];

extern struct wpa_ctrl *p2p_ctrl_conn;
extern struct wpa_ctrl *p2p_mon_conn;

static INT32 devTypeSubIdRange[11] = {8, 9, 5, 4, 1, 5, 4, 6, 5, 5, 7};
static char *priDevType[] = {"PC", "server", "mediaCenter", "ultraMobilepc",
                             "notebook", "desktop", "MID", "netbook", "keyboard",
                             "mouse", "joystick", "trackball", "gamingController",
                             "remote", "touchscreen", "biometricReader", "barcodeReader",
                             "printer/printServer", "scanner", "fax", "copier",
                             "printer/scanner/fax/copier", "digitalStillCamera", "videoCamera",
                             "webCamera", "securityCamera", "NAS", "AP", "router", "switch",
                             "gateway", "bridge", "television", "electronicPictureFrame","projector",
                             "monitor", "DAR", "PVR", "MCX", "setTopBox", "mediaServer/mediaAdapter/mediaExtender",
                             "portableVideoPlayer", "xbox", "xbox360", "playstation",
                             "gameConsole/gameConsoleAdapter", "portableGamingDevice", "windowsMobile",
                             "phoneSingleMode", "phoneDualMode", "smartPhoneSingleMode",
                             "smartPhoneDualMode", "audioTuner/audioReceiver", "speakers",
                             "PMP", "headset", "headPhones", "microphone", "homeTheaterSystems"
                            };


static char *  _get_sub_string(char * buf , char * _ori_s ,char * _des_s , int * len );


void p2p_notify_events(WLAN_NOTIFY_ID_T event , void * param )
{
    int i ;
    for ( i = 0; i < MAX_CBK_NUM ; i++)
    {
        if (_p2p_cbk_list[i] == NULL)
            continue ;
        _p2p_cbk_list[i](event ,param);
    }
}


INT32 x_net_wlan_p2p_is_wfd_mode(void)
{
    if ( p2p_for_wfd)
        return 0;
    return -1;
}

static BOOL _get_wfd_pin_from_supplicant(char * buf, char * pin , UINT8 * len )
{
    if ( (pin == NULL ) || ( * len < 8 ) || (buf == NULL))//here pin code length should at least 8
    {
        WIFI_LOG(WIFI_WARNING,("*** get wfd pin from supplicant invalid param!\n"));
        return FALSE;
    }
    if (strncmp(buf,P2P_EVENT_PROV_DISC_SHOW_PIN,23) != 0)
    {
        WIFI_LOG(WIFI_WARNING,("*** get wfd pin from supplicant not a valid event buf!\n"));
        return FALSE;
    }
    //TODO get pin from wpa_supplicant here
    char * p = buf+strlen(P2P_EVENT_PROV_DISC_SHOW_PIN);
    p = strstr(p," ");
    if ( p == NULL )
    {
        WIFI_LOG(WIFI_ERROR,("*** get wfd pin from supplicant parse event buf failed!\n"));
        return FALSE;
    }
    p = p+1;
    int i = 0;
    while ( p[i] != '\0' )
    {
        pin[i] = p[i];
        i++;
        if (i >= 8 )
            break;
    }

    *len = i;
    return TRUE;
}

INT32 x_net_wlan_clean_file (char* path)
{
    FILE * file;
    char buf[64]="";
    file = fopen(path,"w");
    if (file)
        fclose(file);
    else
    {
        snprintf(buf,sizeof(buf), "touch %s", path);
        WIFI_LOG(WIFI_ERROR, ("cmd=[%s]\n", buf));
        doScriptCmd(buf);
        chmod(path, 0777);
    }

	g_ori_lease_stat.st_size = 0 ;
	
    return 0;
}
//  just for wfd requirement .

INT32 x_net_wlan_p2p_set_peer_ip(CHAR * ip , int len)
{
#if 0 //need to sync TV have no NET_802_11_IP_DATA? can be removed if adopt BD get ip solution
    if ( ip == NULL )
    {
        WIFI_LOG(WIFI_ERROR ,(">>>>>> %s,%d -> ip is null !\n",__FUNCTION__,__LINE__));
        return -1;
    }
    if (len >= 20)
    {
        WIFI_LOG(WIFI_ERROR,(">>>>>>%s,%d -> too long of ip addr!\n",__FUNCTION__,__LINE__));
        return -1;
    }

    NET_802_11_IP_DATA _wfd_data ;
    memset(&_wfd_data , 0 ,sizeof(NET_802_11_IP_DATA));
    strcpy(_wfd_data._ip_addr,ip);
    _wfd_data._port = wfd_p2p_port ;
    WIFI_LOG(WIFI_DEBUG,("####################################\n"));
    WIFI_LOG(WIFI_DEBUG,("	 WFD Ip Get Event!\n"));
    WIFI_LOG(WIFI_DEBUG,("*** Ip = %s, Port=%d ***\n",_wfd_data._ip_addr,(int)_wfd_data._port));
    WIFI_LOG(WIFI_DEBUG,("####################################\n"));
    p2p_notify_events(WLAN_NOTIFY_WFD_IP_GET, (void *)&_wfd_data);
#endif
    return 0;
}

INT32 x_net_wlan_p2p_set_wfd_dev_info_subelem(void)
{
    WIFI_LOG_ENTRY(WIFI_API, (""));

    char _cmd_buf[WFD_CMD_BUF_LEN] = {0};
    UINT16 wfd_device_info_value = 0;
    UINT8 wfd_device_info_id = 0;
    UINT16 wfd_device_info_len = 6;
    char wfd_dev_info_subelement[24] = {0};

    /* set WFD Device Type bits (1:0) - 0b01:Primary Sink */
    wfd_device_info_value = wfd_device_info_value | (1 << 0);

    /* set  Coupled Sink Operation Support at WFD Source bit (2) - 0b0: not supported */
    wfd_device_info_value = wfd_device_info_value & ~(1 << 2);

    /* set  Coupled Sink Operation Support at WFD Sink bit (3) - 0b0: not supported */
    wfd_device_info_value = wfd_device_info_value & ~(1 << 3);

    /* set  WFD Session Availability bits (5:4) - 0b01: Available for WFD Session */
    wfd_device_info_value = wfd_device_info_value | (1 << 4);

    /* set CP Support bit (8) - 0b1: Content Protection using HDCP system 2.0/2.1/2.2 supported */
    wfd_device_info_value = wfd_device_info_value | (1 << 8);

    snprintf(wfd_dev_info_subelement, sizeof(wfd_dev_info_subelement), 
        "%d %04x%04x%04x%04x", wfd_device_info_id, wfd_device_info_len,
        wfd_device_info_value, WFD_CMD_VAL_RTSP_PORT, WFD_CMD_VAL_MAX_THROUGHPUT);

    snprintf(_cmd_buf, sizeof(_cmd_buf), "WFD_SUBELEM_SET %s", wfd_dev_info_subelement);

    if (strlen(_cmd_buf) && !dop2pBooleanCommand(_cmd_buf,"OK"))
    {
        WIFI_LOG(WIFI_ERROR,("[WIFI MW]!! Executing wfd_cmd=(%s) Failed!\n", _cmd_buf));
        return -1;
    }
    return NET_WLAN_OK;
}

INT32 x_net_wlan_p2p_enable_wfd_mode(void)
{
    if ( p2p_for_wfd )
        return NET_WLAN_OK;
    else if (0 == strncmp(if_name, "ra0", strlen ("ra0")))
    {
        char _cmd_buf[WFD_CMD_BUF_LEN] = {0};
        WIFI_LOG(WIFI_DEBUG, ("(wfddbg) Enable WFD...\n"));
        snprintf(_cmd_buf,sizeof(_cmd_buf), "SET wfd_wfdEnable %d", WFD_CMD_VAL_ENABLE_WFD);
        if (strlen(_cmd_buf) && !dop2pBooleanCommand(_cmd_buf,"OK"))
        {
            WIFI_LOG(WIFI_ERROR,("Executing wfd_cmd=(%s) Failed!\n", _cmd_buf));
            return -1;
        }
        p2p_for_wfd = TRUE;
    }
    else
    {
        char _cmd_buf[WFD_CMD_BUF_LEN] = {0};
        WIFI_LOG(WIFI_DEBUG, ("(wfddbg) Enable WFD...\n"));
        snprintf(_cmd_buf,sizeof(_cmd_buf), "SET wifi_display %d", WFD_CMD_VAL_ENABLE_WFD);
        if (strlen(_cmd_buf) && !dop2pBooleanCommand(_cmd_buf,"OK"))
        {
            WIFI_LOG(WIFI_ERROR,("!! Executing wfd_cmd=(%s) Failed!\n", _cmd_buf));
            return -1;
        }
        p2p_for_wfd = TRUE;
    }
    return NET_WLAN_OK;
}
static char *  _get_sub_string(char * buf , char * _ori_s ,char * _des_s , int * len )
{
    if ((!buf) || (!_ori_s) || (!_des_s) || (!len))
        return NULL;

    char * p =NULL , * p1 = NULL;
    p = strstr(buf , _ori_s);
    if (p != NULL)
    {
        int length_tmp = strlen(_ori_s);

        p1 = strstr( p+length_tmp , _des_s);

        if (p1 != NULL)
        {
            *len = p1-p-length_tmp;
            return p;
        }
        else if (strcmp(_des_s, " ") == 0)
        {
            p1 = p + length_tmp;

            while (*p1 != '\0')
            {
                p1++;
            }

            *len = p1 - p - length_tmp;
            return p;
        }
    }
    return NULL;
}

static void p2p_primary_device_type_parse(char *buf, NET_802_11_P2P_DATA *data)
{
    int length = 0;
    char *p = _get_sub_string(buf , "pri_dev_type=" , " " , &length);
    if (p != NULL)
    {
        char *oui = strstr(buf, "-0050F204-");
        if (oui != NULL)
        {
            char _tmp[20] = {0};
            x_strncpy(_tmp, p + strlen("pri_dev_type="), length);

            oui = strstr(_tmp, "-0050F204-");
            if (oui == NULL)
            {
                x_strncpy((*data).primaryDevType, "unknown", 8);
                return;
            }
            *oui = '\0';
            *(oui + 9) = '\0';

            UINT32 id = strtoul(_tmp, NULL, 0);
            UINT32 subId = strtoul(oui + 10, NULL, 0);

            if ((id >= 1 && id <= 11)
                    && (subId >= 1 && subId <= devTypeSubIdRange[id - 1]))
            {
                int sum = 0;
                int i = 0;
                for ( i = 0; i < id - 1; i++)
                {
                    sum += devTypeSubIdRange[i];
                }

                sum += (subId - 1);
                int len = strlen(priDevType[sum]);
                x_strncpy((*data).primaryDevType, priDevType[sum], len);
                (*data).primaryDevType[len] = '\0';
            }
            else if (id == 255 && subId >= 0)
            {
                x_strncpy((*data).primaryDevType, "others", 7);
            }
            else
            {
                x_strncpy((*data).primaryDevType, "unknown", 8);
            }
        }
        else
        {
            x_strncpy((*data).primaryDevType, "unknown", 8);
        }
    }

    WIFI_LOG(WIFI_DEBUG,("[P2P]Primary Dev Type is %s!\n", (*data).primaryDevType));
}
static void p2p_device_type_parse(char *buf, NET_802_11_P2P_DATA *data)
{
    int length = 0;
    char *p = _get_sub_string(buf , "device_type=" , " " , &length);
    if (p != NULL)
    {
        char _tmp[10] = {0};
        x_strncpy(_tmp, p + strlen("device_type="), length);

        if (strcmp(_tmp, "0x0") == 0)
        {
            (*data).devType = WFD_SOURCE_DEVICE;
        }
        else if (strcmp(_tmp, "0x1") == 0)
        {
            (*data).devType = WFD_SINK_DEVICE;
        }
        else
        {
            (*data).devType = WFD_INVALID_DEVICE;
        }
    }

    WIFI_LOG(WIFI_DEBUG,("[P2P]device type is %d!\n", (*data).devType));
}

static void p2p_rssi_parse(char *buf, NET_802_11_P2P_DATA *data)
{
    int length = 0;
    char *p = _get_sub_string(buf , "rssi=" , " " , &length);
    if (p != NULL)
    {
        char _tmp[10] = {0};
        x_strncpy(_tmp, p + strlen("rssi="), length);
        char ** _str_rel = NULL;
        (*data).i4rssi = strtol(_tmp ,_str_rel,10);
        (*data).i4rssi = u1WlanDrv_NormalizeSignal((*data).i4rssi);
    }

    WIFI_LOG(WIFI_DEBUG,("[P2P]rssi is %d!\n", (*data).i4rssi));
}

static void p2p_device_mac_parse(char *buf, UINT8 length, NET_802_11_P2P_DATA *data)
{
    char * p = buf + length;
    if (p != NULL)
    {
        x_strncpy((*data)._mac,p,17);
    }
    WIFI_LOG(WIFI_DEBUG,("[P2P]device mac is %s!\n", (*data)._mac));
}

static void p2p_device_iface_mac_parse(char *buf, NET_802_11_P2P_DATA *data)
{
    char * p = strstr(buf , "peer_iface=");
    p=p+strlen("peer_iface=");
    if (p != NULL)
    {
        x_strncpy((*data)._addr,p,17);
    }
    WIFI_LOG(WIFI_DEBUG,("[P2P]peer iface mac is %s!\n", (*data)._addr));
}

static void p2p_device_dev_mac_parse(char *buf, NET_802_11_P2P_DATA *data)
{
    char * p = strstr(buf , "peer_dev=");
    p=p+strlen("peer_dev=");
    if (p != NULL)
    {
        x_strncpy((*data)._mac,p,17);
    }
    WIFI_LOG(WIFI_DEBUG,("[P2P]peer dev mac is %s!\n", (*data)._mac));
}



static void p2p_device_addr_parse(char *buf, NET_802_11_P2P_DATA *data)
{
    char *p = strstr(buf , "_addr=");
    if (p != NULL)
    {
        x_strncpy((*data)._addr, p + x_strlen("_addr="), 17);
    }
    WIFI_LOG(WIFI_DEBUG,("[P2P] device addr is %s!\n", (*data)._addr));
}

static void p2p_device_name_parse(char *buf, NET_802_11_P2P_DATA *data,char *des_s)
{
    int length = 0;
    char *p = NULL;
    
    if ( des_s )
    {
        p = _get_sub_string(buf , "name=\'" , des_s , &length);
    }

    if ( !p || !des_s )
    {
        p = _get_sub_string(buf , "name=\'" , "\' " , &length);
    }
        
    if (p != NULL)
    {
        if (length >= 32)//65 is max name size .
            length = 32;
        x_strncpy((*data)._name,p+strlen("name=\'"),length);
    }
    WIFI_LOG(WIFI_DEBUG,("[P2P] device name is %s!\n", (*data)._name));
}

static void p2p_device_password_id_parse(char *buf, NET_802_11_P2P_DATA *data)
{
    int length = 0;
    char *p = _get_sub_string(buf , "dev_passwd_id=" , " " , &length);
    if (p != NULL)
    {
        char _tmp[20] = {0};
        if (length <= 8)//check if config_methods value is right .
        {
            x_strncpy(_tmp,p+strlen("dev_passwd_id="),length);
            char ** _str_rel = NULL;
            (*data).devpasswdid = strtol(_tmp ,_str_rel ,16);
        }
    }

    WIFI_LOG(WIFI_DEBUG,("[P2P] device password id is %d!\n", (*data).devpasswdid));
}

static void p2p_config_methods_parse(char *buf, NET_802_11_P2P_DATA *data)
{
    int length = 0;
    char *p = _get_sub_string(buf , "config_methods=" , " " , &length);
    if (p != NULL)
    {
        char _tmp[20] = {0};
        if (length <= 8)//check if config_methods value is right .
        {
            x_strncpy(_tmp,p+strlen("config_methods="),length);
            char ** _str_rel = NULL;
            (*data).config_methods = strtol(_tmp ,_str_rel ,16);
        }
    }

    WIFI_LOG(WIFI_DEBUG,("[P2P] device config methods is 0x%4x!\n", (*data).config_methods));
}

static void p2p_device_capability_parse(char *buf, NET_802_11_P2P_DATA *data)
{
    int length = 0;
    char *p = _get_sub_string(buf , "dev_capab=" , " " , &length);
    if (p != NULL)
    {
        char _tmp[20] = {0};
        if (length <= 8)//check if config_methods value is right .
        {
            x_strncpy(_tmp,p+strlen("dev_capab="),length);
            char ** _str_rel = NULL;
            (*data).dev_capab = strtol(_tmp ,_str_rel ,16);
        }
    }

    WIFI_LOG(WIFI_DEBUG,("[P2P] device capability is %d!\n", (*data).dev_capab));
}

static void p2p_group_capability_parse(char *buf, NET_802_11_P2P_DATA *data)
{
    char *p = strstr(buf, "group_capab=");
    if (p != NULL)
    {
        char *gc = p + strlen("group_capab=");
        int pos = 0;
        while (*(gc + pos) != ' ' && *(gc + pos) != '\0')
        {
            pos++;
        }

        if (pos <= 8)
        {
            char _tmp[20] = {0};
            x_strncpy(_tmp,gc,pos);
            char ** _str_rel = NULL;
            (*data).group_capab = strtol(_tmp ,_str_rel ,16);
        }

    }

    WIFI_LOG(WIFI_DEBUG,("[P2P] group capability is %d!\n", (*data).group_capab));
}

static void p2p_device_rtsp_parse(char *buf, NET_802_11_P2P_DATA *data)
{
    int length = 0;
    char *p = _get_sub_string(buf , "wfd_dev_info=" , " " , &length);
    if (p != NULL)
    {
        char _tmp[5] = {0};
        x_strncpy(_tmp,p+12+strlen("wfd_dev_info="),4);
        char ** _str_rel = NULL;
        (*data).rtsp_port = strtol(_tmp ,_str_rel ,16);
        (*data).b_Is_WFD = TRUE;

    }

    WIFI_LOG(WIFI_DEBUG,("[P2P] device_rtsp is %d!\n", (*data).rtsp_port));
}


static void p2p_error_status_code_parse(char *buf, NET_802_11_P2P_DATA *data)
{
    int length = 0;
    char *p = _get_sub_string(buf , "status_code=" , " " , &length);
    if (p != NULL)
    {
        char _tmp[20] = {0};
        if (length <= 8)//check if status code value is right .
        {
            x_strncpy(_tmp,p+strlen("status_code="),length);
            char ** _str_rel = NULL;
            (*data).status_code = strtol(_tmp ,_str_rel ,16);
        }
    }

    WIFI_LOG(WIFI_DEBUG,("[P2P] status code is %d!\n", (*data).status_code));

}
void p2p_parse_event(char * buf, size_t buf_len)
{
    NET_802_11_NFY_T t_nfy;
    x_memset(&t_nfy, 0, sizeof(NET_802_11_NFY_T));
    NET_802_11_P2P_DATA _p2p_data;
    x_memset(&_p2p_data, 0, sizeof(NET_802_11_P2P_DATA));

    if (buf == NULL)
    {
        WIFI_LOG(WIFI_ERROR,("p2p_parse_event buf is null!\n"));
        return;
    }

    if (strncmp(buf, P2P_EVENT_DEVICE_FOUND, strlen(P2P_EVENT_DEVICE_FOUND)) == 0)
    {
        WIFI_LOG(WIFI_DEBUG ,("[P2P]C2H:notify-app: WLAN_NOTIFY_P2P_DEVICE_FOUND\n"));
        t_nfy.i4NotifykId = WLAN_NOTIFY_P2P_DEVICE_FOUND;
        x_memset(&_p2p_data , 0 , sizeof(NET_802_11_P2P_DATA));
        _p2p_data.b_Is_WFD = FALSE;

        p2p_device_mac_parse(buf, strlen(P2P_EVENT_DEVICE_FOUND), &_p2p_data);
        p2p_device_addr_parse(buf, &_p2p_data);
        p2p_device_type_parse(buf, &_p2p_data);
        p2p_rssi_parse(buf, &_p2p_data);
        p2p_primary_device_type_parse(buf, &_p2p_data);
        p2p_device_name_parse(buf, &_p2p_data,"\' rssi=");
        p2p_config_methods_parse(buf, &_p2p_data);
        p2p_device_capability_parse(buf, &_p2p_data);
        p2p_group_capability_parse(buf, &_p2p_data);

        int length_tmp = 0;
        char *p = _get_sub_string(buf , "wfd_dev_info=" , " " , &length_tmp);
        if (p != NULL)
        {
            char _tmp[20] = {0};
            if (0 == strncmp(if_name, "ra0", strlen ("ra0")))
            {
                if (length_tmp <= 8)//check if config_methods value is right .
                {
                    x_strncpy(_tmp,p+strlen("wfd_dev_info="),length_tmp);
                    char ** _str_rel = NULL;
                    _p2p_data.wfd_dev_info = strtol(_tmp ,_str_rel ,16);
                }
            }
            else
            {
                x_strncpy(_tmp,p+6+strlen("wfd_dev_info="),4);
                _p2p_data.rtsp_port = strtol(_tmp ,NULL ,16);
                _p2p_data.b_Is_WFD = TRUE;
                WIFI_LOG(WIFI_DEBUG,("[P2P]rtsp_port is %d \n",(int)_p2p_data.rtsp_port));
            }

        }
        p = _get_sub_string(buf, "rtsp_port="," ",&length_tmp);
        if (p != NULL)
        {
            WIFI_LOG(WIFI_DEBUG,("[ Wifi Notify Event ]** P2P-DEVICE_FOUND ***%s %d\n",p,(int)length_tmp));
            char _tmp[20] = {0};
            if (length_tmp <= 8)//check if config_methods value is right .
            {
                x_strncpy(_tmp,p+strlen("rtsp_port="),length_tmp);

                WIFI_LOG(WIFI_DEBUG,("[ Wifi Notify Event ]** P2P-DEVICE_FOUND ***%s \n",_tmp));
                char ** _str_rel = NULL;
                _p2p_data.rtsp_port = strtol(_tmp ,_str_rel ,10);
                _p2p_data.b_Is_WFD = TRUE;//if we get resp port here ,we think this device is for wfd.
            }

        }
        p = _get_sub_string(buf , "max_throughput=" , " " , &length_tmp);
        if (p != NULL)
        {
            char *_p = p+strlen("max_throughput=");
            char *_p_equal = NULL;
            char space = ' ';
            char *_p_space = NULL;
            _p_equal = strchr(_p, '='); //check there are some data information after max_throughput
            if (_p_equal == NULL)   //no other data information, max_throughput is the last data information
            {
                length_tmp = strlen(p) - strlen("max_throughput=");
            }
            else
            {
                _p_space = strstr(_p, &space);
                if (_p_space != NULL)
                {
                    length_tmp = _p - _p_space;
                }
            }
            char _tmp[20] = {0};
            if (length_tmp <= 8)//check if config_methods value is right .
            {
                x_strncpy(_tmp,p+strlen("max_throughput="),length_tmp);
                char ** _str_rel = NULL;
                _p2p_data.max_throughput = strtol(_tmp ,_str_rel ,10);
                WIFI_LOG(WIFI_DEBUG,("max_throughput = %d\n",(int)_p2p_data.max_throughput));
            }
        }

        if (_p2p_data.i4rssi <= 0 || _p2p_data.i4rssi > 100)
        {
            char _peer[18] = {'\0'};
            if ( strlen(_p2p_data._mac) == 17 ) {
                strncpy(_peer, _p2p_data._mac, sizeof(_peer) - 1);
            } else if ( strlen(_p2p_data._peer) == 17 ) {
                strncpy(_peer, _p2p_data._peer, sizeof(_peer) - 1);
            }
            NET_802_11_P2P_PEER_INFO _p2pPeerInfo;
            memset(&_p2pPeerInfo , 0 , sizeof(NET_802_11_P2P_PEER_INFO));
            x_net_wlan_get_p2p_peer_info(&_p2pPeerInfo, _peer);
            if ( _p2pPeerInfo._rssi > 0 && _p2pPeerInfo._rssi <= 100 )
                _p2p_data.i4rssi = _p2pPeerInfo._rssi;
        }

        WIFI_LOG(WIFI_DEBUG,("[ Wifi Notify Event ]** P2P-DEVICE_FOUND ***%s %d\n",_p2p_data._mac,(int)_p2p_data.rtsp_port));

        if (!Is_for_listen)
        {
            p2p_notify_events(t_nfy.i4NotifykId, (void *)&_p2p_data);
        }
    }


    if (strncmp(buf,P2P_EVENT_GROUP_STARTED,strlen(P2P_EVENT_GROUP_STARTED)) == 0)
    {
        WIFI_LOG(WIFI_DEBUG ,("[P2P]C2H:notify-app: WLAN_NOTIFY_P2P_GROUP_STARTED\n"));
        t_nfy.i4NotifykId = WLAN_NOTIFY_P2P_GROUP_STARTED;
        x_memset(&_p2p_data , 0 , sizeof(NET_802_11_P2P_DATA));

        x_net_wlan_clean_file(P2P_SERVER_IP_FILE_PATH);

        char *p = strstr(buf , "rule GO");
        char *end = NULL;
        int len = 0;
        if (p != NULL)
        {
            _p2p_data.b_Is_GO = TRUE; // if character "rule GO " does not in ssid string .
            _p2p_data.b_Is_ForceGO = FALSE;
            p2p_is_current_go = 1;
            p2p_is_current_forcego = 0;
        }
        else
        {
            char *pcfg80211 = strstr(buf,"p2p");
            p = strstr(buf,"GO");
            if ((p != NULL) && (pcfg80211 != NULL))
            {
                _p2p_data.b_Is_GO = TRUE; // if character "rule ForceGO " does not in ssid string .
                _p2p_data.b_Is_ForceGO = TRUE;
                p2p_is_current_go = 1;
                p2p_is_current_forcego = 1;
                len = p-1-pcfg80211;
                if (p != NULL)
                {
                    if (len > sizeof(p2p_go_name) - 1)
                        len = sizeof(p2p_go_name) - 1;
                    memset(p2p_go_name, 0, sizeof(p2p_go_name));
                    strncpy(p2p_go_name,pcfg80211,len);
                    strncpy( _p2p_data._p2p_go_name,pcfg80211,len);
                }
                WIFI_LOG(WIFI_DEBUG,("[ Wifi Notify Event ]-> *** P2P-GROUP_STARTED I am GO  %s***\n",p2p_go_name));
            }
            else
            {
                p = strstr(buf,"rule CLIENT");
                if (p != NULL)
                {
                    _p2p_data.b_Is_GO = FALSE;
                    p2p_is_current_go = 0;
                    p2p_is_current_forcego = 0;
                }
                else
                {
                    p = strstr(buf,"client");
                    if (p != NULL && (pcfg80211 != NULL))
                    {
                        _p2p_data.b_Is_GO = FALSE;
                        _p2p_data.b_Is_ForceGO = FALSE;
                        p2p_is_current_go = 0;
                        p2p_is_current_forcego = 0;
                        len = p-1-pcfg80211;
                        if (len > sizeof(p2p_go_name) - 1)
                            len = sizeof(p2p_go_name) - 1;
                        memset(p2p_go_name, 0, sizeof(p2p_go_name));
                        strncpy(p2p_go_name,pcfg80211,len);
                        strncpy( _p2p_data._p2p_go_name,pcfg80211,len);
                        WIFI_LOG(WIFI_DEBUG,("[ Wifi Notify Event ]-> *** P2P-GROUP_STARTED I am clent %s***\n",p2p_go_name));
					}                    
                    else
                    {
						p = strstr(buf,"rule ForecGO");
						if (p != NULL)
						{
							_p2p_data.b_Is_GO = TRUE; // if character "rule ForceGO " does not in ssid string .
							_p2p_data.b_Is_ForceGO = TRUE;
							p2p_is_current_go = 1;
							p2p_is_current_forcego = 1;
						}
						else
						{
                        WIFI_LOG(WIFI_DEBUG,("[ Wifi Notify Event ]-> *** P2P-GROUP_STARTED no rule return***\n"));
                        return ;
						}
                    }
                }
            }
            len=0;

        }

        if (p2p_is_current_go == 0)
        {
            p = strstr(buf , "bssid=");
            if (p != NULL)
            {
                x_strncpy(wfd_connected_mac,p+strlen("bssid="),17);
                x_strncpy(_p2p_data._mac,p+strlen("bssid="),17);
            }
            else
            {
                p = strstr(buf , "go_dev_addr=");
                if (p != NULL)
                {
                    x_strncpy(wfd_connected_mac,p+strlen("go_dev_addr="),17);
                    x_strncpy(_p2p_data._mac,p+strlen("go_dev_addr="),17);
                }
            }
        }
        else if (p2p_is_current_go == 1)
        {
            if (p2p_is_current_forcego == 0)
            {
                p = strstr(buf , "peer=");
                if (p != NULL)
                {
                    x_strncpy(wfd_connected_mac,p+strlen("peer="),17);
                    x_strncpy(_p2p_data._mac,p+strlen("peer="),17);//17 means mac addr length include ":"
                }
            }
            else if (p2p_is_current_forcego == 1)
            {
                p = strstr(buf , "ssid=\"");
                end = p;
                if (p != NULL)
                {
                    end = strstr(end,"\" ");
                    if (end != NULL)
                    {
                        len = end - p;
                        len -= strlen("ssid=\"");
                        x_memset(p2p_go_ssid, 0 , sizeof(p2p_go_ssid));
                        if (len > 32)
                        {
                            len = 32;
                        }
                        x_strncpy(p2p_go_ssid,p+strlen("ssid=\""),len);
                        x_strncpy(_p2p_data.go_ssid,p+strlen("ssid=\""),len);
                    }
                }
                p = strstr(buf , "passphrase=\"");
                end = p;
                if (p != NULL)
                {
                    x_memset(p2p_go_passphrase, 0 , sizeof(p2p_go_passphrase));
                    end = strstr(end,"\" ");
                    if (end != NULL)
                    {
                        len = end - p;
                        len -= strlen("passphrase=\"");
                        if (len > 64)
                        {
                            len = 64;
                        }
                        x_strncpy(p2p_go_passphrase,p+strlen("passphrase=\""),len);
                        x_strncpy(_p2p_data.go_passphrase,p+strlen("passphrase=\""),len);
                    }
                    else
                    {
                        x_strncpy(p2p_go_passphrase,p+strlen("passphrase=\""),sizeof(p2p_go_passphrase) - 1);//copy passphrase ,till buf is end .
                        x_strncpy(_p2p_data.go_passphrase,p+strlen("passphrase=\""),sizeof(_p2p_data.go_passphrase) - 1);//copy passphrase ,till buf is end .
                    }
                }
            }

        }

        p = strstr(buf , "peer=");
        if (p != NULL)
        {
            x_strncpy(wfd_connected_mac, p+strlen("peer="), 17);
            x_strncpy(_p2p_data._peer, p+strlen("peer="), 17);//17 means mac addr length include ":"
        }

        p = strstr(buf , "is_peer5G=0");
        if (p != NULL)
        {
            _p2p_data.b_is_peer_5G = FALSE;
        }
        else
        {
            p = strstr(buf , "is_peer5G=1");
            if (p != NULL)
            {
                _p2p_data.b_is_peer_5G = TRUE;
            }
        }

        p = strstr(buf, "opchannel=");
        if (p != NULL)
        {
            char opchstr[8] = {0};
            int i = 0;
            char *p1 = NULL;
            p1 = p + strlen("opchannel=");
            for (; i < sizeof(opchstr) - 1; i++)
            {
                if ((*(p1+i) >= '0') && (*(p1+i) <= '9'))
                {
                    opchstr[i] = *(p1 + i);
                }
                else
                {
                    break;
                }
            }
            _p2p_data.opchannel = atoi(opchstr);
        }

        p = strstr(buf , "is_SCCChannel=0");
        if (p != NULL)
        {
            _p2p_data.b_is_SCCChannel = FALSE;
        }
        else
        {
            p = strstr(buf , "is_SCCChannel=1");
            if (p != NULL)
            {
                _p2p_data.b_is_SCCChannel = TRUE;
            }
        }

        p = strstr(buf, "rtsp_port=");
        end = p;
        if (p != NULL)
        {
            char ** _str_rel = NULL;
            char _tmp[20] = {0};
            end = strstr(end," ");
            if (end != NULL)
            {
                len = end - p;
                len -= strlen("rtsp_port=");
                x_strncpy(_tmp,p+strlen("rtsp_port="),len);
            }
            else
            {
                x_strncpy(_tmp,p+strlen("rtsp_port="),strlen(p+strlen("rtsp_port=")));//copy rtsp_port ,till buf is end .
            }
            wfd_p2p_port = strtol(_tmp ,_str_rel ,10);
            _p2p_data.rtsp_port = wfd_p2p_port;
        }

        if ( p2p_is_current_go == 0 && 
            (_p2p_data.rtsp_port == 0 || strlen(_p2p_data._name) == 0 ))
        {
            char _peer[18] = {'\0'};
            if ( strlen(_p2p_data._mac) == 17 )
            {
                strncpy(_peer, _p2p_data._mac, sizeof(_peer) - 1);
            }
            else if ( strlen(_p2p_data._peer) == 17 )
            {
                strncpy(_peer, _p2p_data._peer, sizeof(_peer) - 1);
            }

            if ( strlen(_peer) == 17 )
            {
                NET_802_11_P2P_PEER_INFO _p2pPeerInfo;
                memset(&_p2pPeerInfo , 0 , sizeof(NET_802_11_P2P_PEER_INFO));
                x_net_wlan_get_p2p_peer_info(&_p2pPeerInfo, _peer);
                WIFI_LOG(WIFI_DEBUG, (" device name=%s(len=%d), RTSP port=%d\n", 
                    _p2pPeerInfo._device_name, strlen(_p2pPeerInfo._device_name),
                    _p2pPeerInfo._rtsp_port));
                
                if ( _p2p_data.rtsp_port == 0 )
                {
                    if ( _p2pPeerInfo._rtsp_port != 0 )
                    {
                        _p2p_data.rtsp_port = _p2pPeerInfo._rtsp_port;
                        wfd_p2p_port = _p2pPeerInfo._rtsp_port;
                    }
                    else if ( wfd_p2p_port != 0 )
                    {
                        _p2p_data.rtsp_port = wfd_p2p_port;
                    }
                }

                if ( strlen(_p2p_data._name) == 0 )
                {
                    if ( strlen(_p2pPeerInfo._device_name) != 0 )
                    {
                        strncpy(_p2p_data._name, _p2pPeerInfo._device_name, sizeof(_p2p_data._name) - 1);
                        _p2p_data._name[sizeof(_p2p_data._name) - 1] = '\0';
                    } 
                }
            }
            else
            {
                WIFI_LOG(WIFI_DEBUG ,("Can't get peer info due to no peer address."));
            }
        }

		/* GO will send IP info if P2P Client includes IP Address Request KDE in the EAPOL-Key frame 2 */
		/* ip_addr=192.168.173.175 ip_mask=255.255.255.0 go_ip_addr=192.168.173.1 */
		/* for BDP00483213 VAIO Pro 13 PC */
		if ( p2p_is_current_go == 0 ) {
			char p2p_client_ip[16] = {'\0'};
			char p2p_netmask[16] = {'\0'};
			char p2p_go_ip[16] = {'\0'};
			int j = 0;
			
			p = strstr(buf, " ip_addr=");
			if (p) {
				p += strlen(" ip_addr=");
				while(p && *p && j < sizeof(p2p_client_ip) - 1) {
					if ((*p >= '0' && *p <= '9') || *p == '.')
						p2p_client_ip[j++] = *p++;
					else
						break;
				}

				strncpy(_p2p_data.p2p_client_ip_addr, p2p_client_ip, sizeof(_p2p_data.p2p_client_ip_addr) - 1);

				WIFI_LOG(WIFI_INFO ,("P2P Client IP: %s", _p2p_data.p2p_client_ip_addr));

				end = p;
				j = 0;
				p = strstr(end, "ip_mask=");
				if (p) {
					p += strlen("ip_mask=");
					while(p && *p && j < sizeof(p2p_netmask) - 1) {
						if ((*p >= '0' && *p <= '9') || *p == '.')
							p2p_netmask[j++] = *p++;
						else
							break;
					}

					strncpy(_p2p_data.p2p_client_netmask, p2p_netmask, sizeof(_p2p_data.p2p_client_netmask) - 1);

					WIFI_LOG(WIFI_INFO ,("P2P netmask: %s", _p2p_data.p2p_client_netmask));

					end = p;
					j = 0;
					p = strstr(end, "go_ip_addr=");
					if (p) {
						p += strlen("go_ip_addr=");
						while(p && *p && j < sizeof(p2p_go_ip) - 1) {
							if ((*p >= '0' && *p <= '9') || *p == '.')
								p2p_go_ip[j++] = *p++;
							else
								break;
						}

						strncpy(_p2p_data.p2p_go_ip_addr, p2p_go_ip, sizeof(_p2p_data.p2p_go_ip_addr) - 1);

						WIFI_LOG(WIFI_INFO ,("P2P GO IP: %s", _p2p_data.p2p_go_ip_addr));
					}
				}
			}
		}

        WIFI_LOG(WIFI_DEBUG,("[ Wifi Notify Event ]** P2P-GROUP_STARTED ***%s %d %d\n",_p2p_data._mac,(int)_p2p_data.rtsp_port,_p2p_data.b_Is_GO));
        p2p_notify_events(t_nfy.i4NotifykId, (void *)&_p2p_data);
    }
    /*------------------*/

    if (strncmp(buf,P2P_EVENT_DEVICE_LOST,strlen(P2P_EVENT_DEVICE_LOST))==0)
    {
        WIFI_LOG(WIFI_DEBUG ,("[P2P]C2H:notify-app: WLAN_NOTIFY_P2P_DEVICE_LOST\n"));
        t_nfy.i4NotifykId = WLAN_NOTIFY_P2P_DEVICE_LOST;
        x_memset(&_p2p_data , 0 , sizeof(NET_802_11_P2P_DATA));

        p2p_device_addr_parse(buf, &_p2p_data);

        p2p_notify_events(t_nfy.i4NotifykId ,(void *)&_p2p_data);
    }

    if (strncmp(buf,P2P_EVENT_GO_NEG_SUCCESS,strlen(P2P_EVENT_GO_NEG_SUCCESS))==0)
    {
        WIFI_LOG(WIFI_DEBUG ,("[P2P]C2H:notify-app: WLAN_NOTIFY_P2P_GO_NEG_SUCCESS\n"));
        p2p_device_dev_mac_parse(buf, &_p2p_data);
        p2p_device_iface_mac_parse(buf, &_p2p_data);
        t_nfy.i4NotifykId = WLAN_NOTIFY_P2P_GO_NEG_SUCCESS;

        p2p_notify_events(t_nfy.i4NotifykId ,(void *)&_p2p_data);
    }

    if (strncmp(buf,P2P_EVENT_GO_NEG_FAILURE,strlen(P2P_EVENT_GO_NEG_FAILURE))==0)
    {
        WIFI_LOG(WIFI_DEBUG ,("[P2P]C2H:notify-app: WLAN_NOTIFY_P2P_GO_NEG_FAILURE\n"));
        t_nfy.i4NotifykId = WLAN_NOTIFY_P2P_GO_NEG_FAILURE;

        p2p_notify_events(t_nfy.i4NotifykId ,NULL);
    }

    if (strncmp(buf,P2P_EVENT_GROUP_FORMATION_SUCCESS,strlen(P2P_EVENT_GROUP_FORMATION_SUCCESS))==0)
    {
        WIFI_LOG(WIFI_DEBUG ,("[P2P]C2H:notify-app: WLAN_NOTIFY_P2P_GROUP_FORMATION_SUCCESS\n"));
        t_nfy.i4NotifykId = WLAN_NOTIFY_P2P_GROUP_FORMATION_SUCCESS;

        p2p_notify_events(t_nfy.i4NotifykId ,NULL);
    }

    if (strncmp(buf,P2P_EVENT_GROUP_FORMATION_FAILURE,strlen(P2P_EVENT_GROUP_FORMATION_FAILURE))==0)
    {
        WIFI_LOG(WIFI_DEBUG ,("[P2P]C2H:notify-app: WLAN_NOTIFY_P2P_GROUP_FORMATION_FAILURE\n"));
        t_nfy.i4NotifykId = WLAN_NOTIFY_P2P_GROUP_FORMATION_FAILURE;

        p2p_notify_events(t_nfy.i4NotifykId ,NULL);
    }

    NET_802_11_P2P_INVITATION_DATA p2p_invitation_data;

    /* P2P-INVITATION-RECEIVED sa=fa:7b:7a:42:02:13 go_dev_addr=f8:7b:7a:42:02:13
        bssid=fa:7b:7a:42:82:13 unknown-network */
    if (strncmp(buf,P2P_EVENT_INVITATION_RECEIVED,strlen(P2P_EVENT_INVITATION_RECEIVED))==0)
    {
        x_memset(&_p2p_data , 0 , sizeof(NET_802_11_P2P_DATA));
        WIFI_LOG(WIFI_DEBUG ,("[P2P]C2H:notify-app: WLAN_NOTIFY_P2P_INVITATION_RECEIVED\n"));
        t_nfy.i4NotifykId = WLAN_NOTIFY_P2P_INVITATION_RECEIVED;

        if (strncmp(buf + strlen(P2P_EVENT_INVITATION_RECEIVED), "sa=", strlen("sa=")) == 0)
        {
            p2p_device_mac_parse(buf, strlen(P2P_EVENT_INVITATION_RECEIVED) + strlen("sa="), &_p2p_data);
        }
        else
        {
            p2p_device_mac_parse(buf, strlen(P2P_EVENT_INVITATION_RECEIVED), &_p2p_data);
        }

        p2p_device_addr_parse(buf, &_p2p_data);
        p2p_primary_device_type_parse(buf, &_p2p_data);
        p2p_device_name_parse(buf, &_p2p_data,NULL);

        p2p_notify_events(t_nfy.i4NotifykId ,&_p2p_data);
    }

    /* P2P-INVITATION-RESULT status=1 */
    if (strncmp(buf,P2P_EVENT_INVITATION_RESULT,strlen(P2P_EVENT_INVITATION_RESULT))==0)
    {
        x_memset(&p2p_invitation_data, 0 , sizeof(NET_802_11_P2P_INVITATION_DATA));
        WIFI_LOG(WIFI_DEBUG ,("[P2P]C2H:notify-app: WLAN_NOTIFY_P2P_INVITATION_RESULT\n"));
        t_nfy.i4NotifykId = WLAN_NOTIFY_P2P_INVITATION_RESULT;

        char *p = strstr(buf, "status=");
        if (p != NULL)
        {
            char _tmp[20] = {0};
            x_strncpy(_tmp, p + strlen("status="),strlen(p + strlen("status=")));

            p2p_invitation_data._status = strtol(_tmp, NULL, 10);
        }

        WIFI_LOG(WIFI_DEBUG,("[WIFI MW]***P2P_EVENT_INVITATION_RESULT status=%ld\n", p2p_invitation_data._status));
        p2p_notify_events(t_nfy.i4NotifykId ,&p2p_invitation_data);
    }

    /* AP-STA-CONNECTED 42:fc:89:a8:96:09 */
    if (strncmp(buf,AP_STA_CONNECTED,strlen(AP_STA_CONNECTED))==0)
    {
        WIFI_LOG(WIFI_DEBUG ,("[P2P]C2H:notify-app: WLAN_NOTIFY_AP_STA_CONNECTED\n"));
        t_nfy.i4NotifykId = WLAN_NOTIFY_AP_STA_CONNECTED;
        x_memset(&_p2p_data , 0 , sizeof(NET_802_11_P2P_DATA));

        char *p = buf + strlen(AP_STA_CONNECTED);
        char *end = NULL;
        int len = 0;
        x_strncpy(_p2p_data._addr, p, 17);
        x_strncpy(wfd_connected_mac,_p2p_data._addr,17);


        p = strstr(buf , "dev_addr=");
        if (p != NULL)
        {
            p += strlen("dev_addr=");
            x_strncpy(_p2p_data._mac, p, 17);
        }
        p = strstr(buf , "is_go=");
        if (p != NULL)
        {
            p += strlen("is_go=");
            if (strncmp(p,"1",1)==0)
            {
                _p2p_data.b_Is_GO = TRUE;
                p2p_is_current_go = TRUE;
            }
            else
            {
                if (p2p_is_current_forcego == 1)
                {
                    _p2p_data.b_Is_legacy_client = TRUE;
                    _p2p_data.b_Is_GO = TRUE;
                    p2p_is_current_go = TRUE;
                }
                else
                {
                    _p2p_data.b_Is_GO = FALSE;
                    p2p_is_current_go = FALSE;
                }
            }
        }
        p = strstr(buf , "update_table=");
        if (p != NULL)
        {
            p += strlen("update_table=");
            if (strncmp(p,"1",1)==0)
            {
                _p2p_data.b_update_ptable= TRUE;
            }
            else
            {
                _p2p_data.b_update_ptable = FALSE;
            }
        }

        p = strstr(buf, "rtsp_port=");
        end = p;
        if (p != NULL)
        {
            char ** _str_rel = NULL;
            char _tmp[20] = {0};
            end = strstr(end," ");
            if (end != NULL)
            {
                len = end - p;
                len -= strlen("rtsp_port=");
                x_strncpy(_tmp,p+strlen("rtsp_port="),len);
            }
            else
            {
                x_strncpy(_tmp,p+strlen("rtsp_port="),strlen(p+strlen("rtsp_port=")));//copy rtsp_port ,till buf is end .
            }
            wfd_p2p_port = strtol(_tmp ,_str_rel ,10);
            _p2p_data.rtsp_port = wfd_p2p_port;
            _p2p_data.b_Is_WFD = TRUE;//if we get resp port here ,we think this device is for wfd.
        }

        p = strstr(buf, "dev_name=");
        if (p != NULL)
        {
            int i = 0;
            char devname[33] = {'\0'};
            p += strlen("dev_name=");
            for (i = 0; i < sizeof(devname) - 1; i++)
            {
                if ( *p != '\0' && *p != '\r' && *p != '\n' )
                    devname[i] = *p++;
                else
                    break;
            }
            devname[sizeof(devname) - 1] = '\0';
            strncpy(_p2p_data._name, devname, sizeof(_p2p_data._name) - 1);
            _p2p_data._name[sizeof(_p2p_data._name) - 1] = '\0';
        }

        if ( _p2p_data.rtsp_port == 0 || strlen(_p2p_data._name) == 0 ) {
            char _peer[18] = {'\0'};
            if ( strlen(_p2p_data._mac) == 17 )
            {
                strncpy(_peer, _p2p_data._mac, sizeof(_peer) - 1);
            }
            else if ( strlen(_p2p_data._peer) == 17 )
            {
                strncpy(_peer, _p2p_data._peer, sizeof(_peer) - 1);
            }

            if ( strlen(_peer) == 17 )
            {
                NET_802_11_P2P_PEER_INFO _p2pPeerInfo;
                memset(&_p2pPeerInfo , 0 , sizeof(NET_802_11_P2P_PEER_INFO));
                x_net_wlan_get_p2p_peer_info(&_p2pPeerInfo, _peer);
                WIFI_LOG(WIFI_DEBUG, (" device name=%s(len=%d), RTSP port=%d\n", 
                    _p2pPeerInfo._device_name, strlen(_p2pPeerInfo._device_name),
                    _p2pPeerInfo._rtsp_port));
                
                if ( _p2p_data.rtsp_port == 0 )
                {
                    if ( _p2pPeerInfo._rtsp_port != 0 )
                    {
                        _p2p_data.rtsp_port = _p2pPeerInfo._rtsp_port;
                        wfd_p2p_port = _p2pPeerInfo._rtsp_port;
                    }
                    else if ( wfd_p2p_port != 0 )
                    {
                        _p2p_data.rtsp_port = wfd_p2p_port;
                    }
                }

                if ( strlen(_p2p_data._name) == 0 )
                {
                    if ( strlen(_p2pPeerInfo._device_name) != 0 )
                    {
                        strncpy(_p2p_data._name, _p2pPeerInfo._device_name, 
                            sizeof(_p2p_data._name) - 1);
                        _p2p_data._name[sizeof(_p2p_data._name) - 1] = '\0';
                    } 
                }
            }
            else
            {
                WIFI_LOG(WIFI_DEBUG ,("Can't get peer info due to no peer address."));
            }
        }

        WIFI_LOG(WIFI_DEBUG,("***AP_STA_CONNECTED mac=%s\n", _p2p_data._mac));
        p2p_notify_events(t_nfy.i4NotifykId ,&_p2p_data);
    }

    /* AP-STA-DISCONNECTED 42:fc:89:a8:96:09 */
    if (strncmp(buf,AP_STA_DISCONNECTED,strlen(AP_STA_DISCONNECTED))==0)
    {
        WIFI_LOG(WIFI_DEBUG ,("[P2P]C2H:notify-app: WLAN_NOTIFY_AP_STA_DISCONNECTED\n"));
        t_nfy.i4NotifykId = WLAN_NOTIFY_AP_STA_DISCONNECTED;
        x_memset(&_p2p_data , 0 , sizeof(NET_802_11_P2P_DATA));

        char *p = buf + strlen(AP_STA_DISCONNECTED);
        x_strncpy(_p2p_data._mac, p, 17);
        WIFI_LOG(WIFI_DEBUG,("***AP_STA_DISCONNECTED mac=%s\n", _p2p_data._mac));
        p2p_notify_events(t_nfy.i4NotifykId ,&_p2p_data);
    }

    /* <3>CTRL-EVENT-DISCONNECTED bssid=52:1a:c5:03:8f:e1 reason=3 locally_generated=1 *
           * <3>CTRL-EVENT-DISCONNECTED bssid=52:1a:c5:03:8f:e1 reason=0 *
           * For wpa_supplicant-2.0, GC case this event will be received when P2P disconnect */
#if PLATFORM_IS_TV
	if ( strncmp(buf, P2P_CLIENT_DISCONNECT, strlen(P2P_CLIENT_DISCONNECT)) == 0)
		{
		 char *p = strstr(buf,"reason=3");
		 if(NULL==p)
		 	{
            WIFI_LOG(WIFI_DEBUG ,("[P2P]C2H:notify-app: WLAN_NOTIFY_P2P_CLIENT_DISCONNECT\n"));
            t_nfy.i4NotifykId = WLAN_NOTIFY_AP_STA_DISCONNECTED;
            x_memset(&_p2p_data , 0 , sizeof(NET_802_11_P2P_DATA));
		    p=strstr(buf,"bssid=");
			if(NULL!=p)
			x_strncpy(_p2p_data._addr, p+strlen("bssid="), 17);
        p = strstr(buf, "locally_generated");
        if (p)
        {
            _p2p_data.status_code = 1;
        }
        else
        {
            _p2p_data.status_code = 0;
        }

		    p2p_notify_events(t_nfy.i4NotifykId ,&_p2p_data);
			WIFI_LOG(WIFI_DEBUG,("***P2P_CLIENT_DISCONNECT bssid=%s\n", _p2p_data._addr));
		 	}
	    }
#else
		if (strncmp(buf,CTRL_EVENT_DISCONNECTED,strlen(CTRL_EVENT_DISCONNECTED))==0)
		{
			WIFI_LOG(WIFI_DEBUG ,("[P2P]C2H:notify-app: WLAN_NOTIFY_DISCONNECTED\n"));
			t_nfy.i4NotifykId = WLAN_NOTIFY_DISCONNECTED;
			x_memset(&_p2p_data , 0 , sizeof(NET_802_11_P2P_DATA));

			char *p = NULL;
			p = strstr(buf, "bssid=");
			if ( p )
			{
				p += strlen("bssid=");
				x_strncpy(_p2p_data._mac, p, 17);
			}

			p = strstr(buf, "locally_generated");
			if (p)
			{
				_p2p_data.status_code = 1;
			}
			else
			{
				_p2p_data.status_code = 0;
			}

			WIFI_LOG(WIFI_DEBUG,("***CTRL-EVENT-DISCONNECTED mac=%s\n", _p2p_data._mac));
			p2p_notify_events(t_nfy.i4NotifykId ,&_p2p_data);
		}
#endif

    if (strncmp(buf,P2P_LEGACY_DISCONNECT,strlen(P2P_LEGACY_DISCONNECT))==0)
    {
        WIFI_LOG(WIFI_DEBUG ,("[P2P]C2H:notify-app: WLAN_NOTIFY_P2P_LEGACY_DISCONNECT\n"));
        t_nfy.i4NotifykId = WLAN_NOTIFY_P2P_LEGACY_DISCONNECT;
        x_memset(&_p2p_data , 0 , sizeof(NET_802_11_P2P_DATA));

        char *p = buf + strlen(P2P_LEGACY_DISCONNECT);
        x_strncpy(_p2p_data._mac, p, 17);

        WIFI_LOG(WIFI_DEBUG,("***P2P_LEGACY_DISCONNECT mac=%s\n", _p2p_data._mac));
        p2p_notify_events(t_nfy.i4NotifykId ,&_p2p_data);
    }
    if (strncmp(buf,P2P_PROV_DISC_RESP,strlen(P2P_PROV_DISC_RESP))==0)
    {
        WIFI_LOG(WIFI_DEBUG ,("[P2P]C2H:notify-app: WLAN_NOTIFY_P2P_PROV_DISC_RESP\n"));
        t_nfy.i4NotifykId = WLAN_NOTIFY_P2P_PROV_DISC_RESP;
        x_memset(&_p2p_data , 0 , sizeof(NET_802_11_P2P_DATA));

        char *p = buf + strlen(P2P_PROV_DISC_RESP);
        x_strncpy(_p2p_data._mac, p, 17);

        p = strstr(buf , "REJECT");
        if (p != NULL)
        {
            _p2p_data.b_Is_REJECT = TRUE;
        }
        WIFI_LOG(WIFI_DEBUG,("***P2P_EVENT_PROV_DISC_PBC_RESP mac=%s\n", _p2p_data._mac));
        p2p_notify_events(t_nfy.i4NotifykId ,&_p2p_data);

    }
    if (strncmp(buf,P2P_EVENT_PROV_DISC_PBC_RESP,strlen(P2P_EVENT_PROV_DISC_PBC_RESP))==0)
    {
        WIFI_LOG(WIFI_DEBUG ,("[P2P]C2H:notify-app: WLAN_NOTIFY_P2P_PROV_DISC_PBC_RESP\n"));

        t_nfy.i4NotifykId = WLAN_NOTIFY_P2P_PROV_DISC_PBC_RESP;
        x_memset(&_p2p_data , 0 , sizeof(NET_802_11_P2P_DATA));
        char *p = buf + strlen(P2P_EVENT_PROV_DISC_PBC_RESP);
        x_strncpy(_p2p_data._mac, p, 17);

        WIFI_LOG(WIFI_DEBUG,("***P2P_EVENT_PROV_DISC_PBC_RESP mac=%s\n", _p2p_data._mac));
        p2p_notify_events(t_nfy.i4NotifykId ,&_p2p_data);
    }

    if (strncmp(buf,P2P_EVENT_GO_NEG_REQUEST,strlen(P2P_EVENT_GO_NEG_REQUEST))==0)
    {
        WIFI_LOG(WIFI_DEBUG ,("[P2P]C2H:notify-app: WLAN_NOTIFY_P2P_GO_NEG_REQUEST\n"));
        t_nfy.i4NotifykId = WLAN_NOTIFY_P2P_GO_NEG_REQUEST;
        x_memset(&_p2p_data , 0 , sizeof(NET_802_11_P2P_DATA));

        p2p_device_mac_parse(buf, strlen(P2P_EVENT_GO_NEG_REQUEST), &_p2p_data);
        p2p_device_password_id_parse(buf,&_p2p_data);
        p2p_primary_device_type_parse(buf, &_p2p_data);
        p2p_config_methods_parse(buf, &_p2p_data);
        p2p_device_name_parse(buf, &_p2p_data,"\' config_methods=");
        p2p_device_rtsp_parse(buf, &_p2p_data);

        if ( strlen(_p2p_data._name) == 0 || _p2p_data.rtsp_port == 0 )
        {
            if ( strlen(_p2p_data._mac) == 17 )
            {
                NET_802_11_P2P_PEER_INFO _p2pPeerInfo;
                memset(&_p2pPeerInfo , 0 , sizeof(NET_802_11_P2P_PEER_INFO));
                x_net_wlan_get_p2p_peer_info(&_p2pPeerInfo, _p2p_data._mac);
                WIFI_LOG(WIFI_DEBUG, (" device name=%s(len=%d), RTSP port=%d\n", 
                    _p2pPeerInfo._device_name, strlen(_p2pPeerInfo._device_name),
                    _p2pPeerInfo._rtsp_port));
                
                if ( _p2p_data.rtsp_port == 0 )
                {
                    if ( _p2pPeerInfo._rtsp_port != 0 )
                    {
                        _p2p_data.rtsp_port = _p2pPeerInfo._rtsp_port;
                        wfd_p2p_port = _p2pPeerInfo._rtsp_port;
                    }
                    else if ( wfd_p2p_port != 0 )
                    {
                        _p2p_data.rtsp_port = wfd_p2p_port;
                    }
                }

                if ( strlen(_p2p_data._name) == 0 )
                {
                    if ( strlen(_p2pPeerInfo._device_name) != 0 )
                    {
                        strncpy(_p2p_data._name, _p2pPeerInfo._device_name, 
                            sizeof(_p2p_data._name) - 1);
                        _p2p_data._name[sizeof(_p2p_data._name) - 1] = '\0';
                    } 
                }
            }
            else
            {
                WIFI_LOG(WIFI_DEBUG ,("Can't get peer info due to no peer address."));
            }
        }
        
        if (!fgProvisionReq)
        {
            if (_p2p_data.devpasswdid == 5) //requestor wants to display pin Register-specified
            {
                t_nfy.i4NotifykId = WLAN_NOTIFY_WFD_PIN_KEYPAD;
                WIFI_LOG(WIFI_DEBUG,("[P2P]notify-app: WLAN_NOTIFY_WFD_PIN_KEYPAD, device password id is %d! We need to do Enter Pin Connect!\n", _p2p_data.devpasswdid));
            }
            else if (_p2p_data.devpasswdid == 4)
            {
                t_nfy.i4NotifykId = WLAN_NOTIFY_P2P_GO_NEG_REQUEST;
                WIFI_LOG(WIFI_DEBUG,("[P2P]device password id is %d! We need to do PBC Connect!\n", _p2p_data.devpasswdid));
            }
            else if (_p2p_data.devpasswdid == 1)//requestor wants to enter  pin user-specified
            {
                t_nfy.i4NotifykId = WLAN_NOTIFY_WFD_PIN_DISPLAY;
                WIFI_LOG(WIFI_DEBUG,("[P2P]notify-app: WLAN_NOTIFY_WFD_PIN_DISPLAY, device password id is %d! We need to do Display Pin Connect!\n", _p2p_data.devpasswdid));
            }

            p2p_notify_events(t_nfy.i4NotifykId ,(void *)&_p2p_data);
        }
        else
        {
            fgProvisionReq = FALSE;
            if (_p2p_data.devpasswdid == 4)
            {
                p2p_notify_events(t_nfy.i4NotifykId ,(void *)&_p2p_data);
            }
        }
    }
    /*------------------*/

    if (strncmp(buf,P2P_EVENT_GROUP_REMOVED,strlen(P2P_EVENT_GROUP_REMOVED))==0)
    {
        WIFI_LOG(WIFI_DEBUG ,("[P2P]C2H:notify-app: WLAN_NOTIFY_P2P_GROUP_REMOVED\n"));
        t_nfy.i4NotifykId = WLAN_NOTIFY_P2P_GROUP_REMOVED;
        t_nfy.pParam[0] = WLAN_NFY_MSG_OK;
        doScriptCmd("ip neigh flush dev p2p0");

        p2p_is_current_go = -1;
        p2p_is_current_forcego = -1;
        wfd_p2p_port = 0;
		if (strncmp(p2p_go_name, "p2p-p2p0-", strlen("p2p-p2p0-")) == 0)
        	memset(p2p_go_name, 0, sizeof(p2p_go_name));
        p2p_notify_events(t_nfy.i4NotifykId, t_nfy.pParam);
    }

    if (strncmp(buf,P2P_EVENT_PROV_DISC_SHOW_PIN,strlen(P2P_EVENT_PROV_DISC_SHOW_PIN))==0)
    {
        WIFI_LOG(WIFI_DEBUG ,("[P2P]C2H:notify-app: WLAN_NOTIFY_PROV_DISC_SHOW_PIN\n"));
        fgProvisionReq = TRUE;
        x_memset(&_p2p_data , 0 , sizeof(NET_802_11_P2P_DATA));
        char pin[10] = {0};
        UINT8 len_tmp = 10;

        p2p_device_mac_parse(buf, strlen(P2P_EVENT_PROV_DISC_SHOW_PIN), &_p2p_data);
        p2p_device_addr_parse(buf, &_p2p_data);
        p2p_device_name_parse(buf, &_p2p_data,"\' config_methods=");
        p2p_primary_device_type_parse(buf, &_p2p_data);
        p2p_config_methods_parse(buf, &_p2p_data);
        p2p_device_capability_parse(buf, &_p2p_data);
        p2p_group_capability_parse(buf, &_p2p_data);

        if (!_get_wfd_pin_from_supplicant(buf , pin,&len_tmp))
        {
            WIFI_LOG(WIFI_DEBUG,("*** in wpa cli recv pending ,get wfd pin error!\n"));
            return;
        }
        t_nfy.i4NotifykId = WLAN_NOTIFY_PROV_DISC_SHOW_PIN;
        strncpy(_p2p_data.pincode,pin,len_tmp);
        p2p_notify_events(t_nfy.i4NotifykId ,(void *)&_p2p_data);

        WIFI_LOG(WIFI_DEBUG,("***************************************\n"));
        WIFI_LOG(WIFI_DEBUG,("*           PIN Code = %s       *\n", pin));
        WIFI_LOG(WIFI_DEBUG,("***************************************\n"));
    }

    if (strncmp(buf,P2P_EVENT_PROV_DISC_ENTER_PIN,strlen(P2P_EVENT_PROV_DISC_ENTER_PIN)) == 0)
    {
        WIFI_LOG(WIFI_DEBUG ,("[P2P]C2H:notify-app: WLAN_NOTIFY_WFD_PIN_KEYPAD\n"));
        fgProvisionReq = TRUE;
        t_nfy.i4NotifykId = WLAN_NOTIFY_PROV_DISC_ENTER_PIN;
        x_memset(&_p2p_data , 0 , sizeof(NET_802_11_P2P_DATA));

        p2p_device_mac_parse(buf, strlen(P2P_EVENT_PROV_DISC_ENTER_PIN), &_p2p_data);
        p2p_device_addr_parse(buf, &_p2p_data);
        p2p_device_name_parse(buf, &_p2p_data,"\' config_methods=");
        p2p_primary_device_type_parse(buf, &_p2p_data);
        p2p_config_methods_parse(buf, &_p2p_data);
        p2p_device_capability_parse(buf, &_p2p_data);
        p2p_group_capability_parse(buf, &_p2p_data);

        p2p_notify_events(t_nfy.i4NotifykId ,(void *)&_p2p_data);
    }

    if (strncmp(buf,P2P_EVENT_PROV_DISC_PBC_REQ,strlen(P2P_EVENT_PROV_DISC_PBC_REQ)) == 0)
    {
        WIFI_LOG(WIFI_DEBUG ,("[P2P]C2H:notify-app: WLAN_NOTIFY_WFD_PBC\n"));
        fgProvisionReq = TRUE;

        t_nfy.i4NotifykId = WLAN_NOTIFY_P2P_PROV_DISC_PBC_REQ;
        x_memset(&_p2p_data , 0 , sizeof(NET_802_11_P2P_DATA));

        p2p_device_mac_parse(buf, strlen(P2P_EVENT_PROV_DISC_PBC_REQ), &_p2p_data);
        p2p_device_addr_parse(buf, &_p2p_data);
        p2p_device_name_parse(buf, &_p2p_data,"\' config_methods=");
        p2p_primary_device_type_parse(buf, &_p2p_data);
        p2p_config_methods_parse(buf, &_p2p_data);
        p2p_device_capability_parse(buf, &_p2p_data);
        p2p_group_capability_parse(buf, &_p2p_data);
        p2p_device_rtsp_parse(buf, &_p2p_data);

        p2p_notify_events(t_nfy.i4NotifykId ,(void *)&_p2p_data);
    }

    if (strncmp(buf,P2P_EVENT_WPS_SEND_M2D,strlen(P2P_EVENT_WPS_SEND_M2D)) == 0)
    {
        WIFI_LOG(WIFI_DEBUG ,("[P2P]C2H:notify-app: WLAN_NOTIFY_P2P_WPS_SEND_M2D\n"));

        t_nfy.i4NotifykId = WLAN_NOTIFY_P2P_WPS_SEND_M2D;
        x_memset(&_p2p_data , 0 , sizeof(NET_802_11_P2P_DATA));

        p2p_notify_events(t_nfy.i4NotifykId ,(void *)&_p2p_data);
    }
    if (strncmp(buf,P2P_EVENT_WPS_RECEIVE_NACK,strlen(P2P_EVENT_WPS_RECEIVE_NACK)) == 0)
    {
        WIFI_LOG(WIFI_DEBUG ,("[P2P]C2H:notify-app: WLAN_NOTIFY_P2P_WPS_RECEIVE_NACK\n"));

        t_nfy.i4NotifykId = WLAN_NOTIFY_P2P_WPS_RECEIVE_NACK;
        x_memset(&_p2p_data , 0 , sizeof(NET_802_11_P2P_DATA));

        p2p_notify_events(t_nfy.i4NotifykId ,(void *)&_p2p_data);
    }

    /* P2P-GO-NEGO-FAIL-INTENT 00:0C:43:22:67:05 */
    if (strncmp(buf, P2P_GO_NEGO_FAIL_INTENT, strlen(P2P_GO_NEGO_FAIL_INTENT))==0)
    {
        WIFI_LOG(WIFI_DEBUG ,("[P2P]C2H:notify-app: P2P_GO_NEGO_FAIL_INTENT\n"));
        t_nfy.i4NotifykId = WLAN_NOTIFY_P2P_GO_NEGO_FAIL_INTENT;
        x_memset(&_p2p_data , 0 , sizeof(NET_802_11_P2P_DATA));

        p2p_device_mac_parse(buf, strlen(P2P_GO_NEGO_FAIL_INTENT), &_p2p_data);

        p2p_notify_events(t_nfy.i4NotifykId ,&_p2p_data);
    }

    /* P2P_GO_NEGO_FAIL_RECV_RESP 00:0C:43:22:67:05 status_code=x */
    if (strncmp(buf, P2P_GO_NEGO_FAIL_RECV_RESP, strlen(P2P_GO_NEGO_FAIL_RECV_RESP))==0)
    {
        WIFI_LOG(WIFI_DEBUG ,("[P2P]C2H:notify-app: WLAN_NOTIFY_P2P_GO_NEGO_FAIL_RECV_RESP\n"));
        t_nfy.i4NotifykId = WLAN_NOTIFY_P2P_GO_NEGO_FAIL_RECV_RESP;
        x_memset(&_p2p_data , 0 , sizeof(NET_802_11_P2P_DATA));

        p2p_device_mac_parse(buf, strlen(P2P_GO_NEGO_FAIL_RECV_RESP), &_p2p_data);
        p2p_error_status_code_parse(buf,&_p2p_data);

        p2p_notify_events(t_nfy.i4NotifykId ,&_p2p_data);
    }

    /* P2P_GO_NEGO_FAIL_RECV_CONFIRM 00:0C:43:22:67:05 status_code=x */
    if (strncmp(buf, P2P_GO_NEGO_FAIL_RECV_CONFIRM, strlen(P2P_GO_NEGO_FAIL_RECV_CONFIRM))==0)
    {
        WIFI_LOG(WIFI_DEBUG ,("[P2P]C2H:notify-app: WLAN_NOTIFY_P2P_GO_NEGO_FAIL_RECV_CONFIRM\n"));
        t_nfy.i4NotifykId = WLAN_NOTIFY_P2P_GO_NEGO_FAIL_RECV_CONFIRM;
        x_memset(&_p2p_data , 0 , sizeof(NET_802_11_P2P_DATA));

        p2p_device_mac_parse(buf, strlen(P2P_GO_NEGO_FAIL_RECV_CONFIRM), &_p2p_data);
        p2p_error_status_code_parse(buf,&_p2p_data);


        p2p_notify_events(t_nfy.i4NotifykId ,&_p2p_data);
    }

    /* P2P_WPS_FAIL 00:0C:43:22:67:05 status_code=x */
    if (strncmp(buf, P2P_WPS_FAIL, strlen(P2P_WPS_FAIL))==0)
    {
        WIFI_LOG(WIFI_DEBUG ,("[P2P]C2H:notify-app: WLAN_NOTIFY_P2P_WPS_FAIL\n"));
        t_nfy.i4NotifykId = WLAN_NOTIFY_P2P_WPS_FAIL;
        x_memset(&_p2p_data , 0 , sizeof(NET_802_11_P2P_DATA));

        p2p_device_mac_parse(buf, strlen(P2P_WPS_FAIL), &_p2p_data);
        p2p_error_status_code_parse(buf,&_p2p_data);

        p2p_notify_events(t_nfy.i4NotifykId ,&_p2p_data);
    }

    /* WPS-FAIL msg=8 config_error=18 */
    if ( strncmp(buf, WPS_EVENT_FAIL, strlen(WPS_EVENT_FAIL)) == 0 )
    {
        WIFI_LOG(WIFI_DEBUG ,("[P2P]C2H:notify-app: WLAN_NOTIFY_P2P_WPS_FAIL\n"));
        t_nfy.i4NotifykId = WLAN_NOTIFY_P2P_WPS_FAIL;
        x_memset(&_p2p_data , 0 , sizeof(NET_802_11_P2P_DATA));

        int err_code = -1;
        char err_str[8] = {"\0"};
        char *p = strstr(buf, "config_error=");
        p = p + strlen("config_error=");
        strncpy(err_str, p, 3);
        err_code = atoi(err_str);

        WIFI_LOG(WIFI_DEBUG, ("WPS config error code = %d", err_code));

        _p2p_data.status_code = err_code;

        p2p_notify_events(t_nfy.i4NotifykId ,&_p2p_data);
    }

    /* P2P_WPS_TIMEOUT 00:0C:43:22:67:05 */
    if (strncmp(buf, P2P_WPS_TIMEOUT, strlen(P2P_WPS_TIMEOUT))==0)
    {
        WIFI_LOG(WIFI_DEBUG ,("[P2P]C2H:notify-app: WLAN_NOTIFY_P2P_WPS_TIMEOUT\n"));
        t_nfy.i4NotifykId = WLAN_NOTIFY_P2P_WPS_TIMEOUT;
        x_memset(&_p2p_data , 0 , sizeof(NET_802_11_P2P_DATA));

        p2p_device_mac_parse(buf, strlen(P2P_WPS_TIMEOUT), &_p2p_data);

        p2p_notify_events(t_nfy.i4NotifykId ,&_p2p_data);
    }

    /* P2P_WPS_2MINS_TIMEOUT */
    if (strncmp(buf, P2P_WPS_2MINS_TIMEOUT, strlen(P2P_WPS_2MINS_TIMEOUT))==0)
    {
        WIFI_LOG(WIFI_DEBUG ,("[P2P]C2H:notify-app: WLAN_NOTIFY_P2P_WPS_2MINS_TIMEOUT\n"));
        t_nfy.i4NotifykId = WLAN_NOTIFY_P2P_WPS_2MINS_TIMEOUT;
        x_memset(&_p2p_data , 0 , sizeof(NET_802_11_P2P_DATA));

        p2p_notify_events(t_nfy.i4NotifykId ,&_p2p_data);
    }

    /* P2P_WPS_PBC_SESSION_OVERLAP */
    if (strncmp(buf, P2P_WPS_PBC_SESSION_OVERLAP, strlen(P2P_WPS_PBC_SESSION_OVERLAP))==0)
    {
        WIFI_LOG(WIFI_DEBUG ,("[P2P]C2H:notify-app: WLAN_NOTIFY_P2P_WPS_PBC_SESSION_OVERLAP\n"));
        t_nfy.i4NotifykId = WLAN_NOTIFY_P2P_WPS_PBC_SESSION_OVERLAP;
        x_memset(&_p2p_data , 0 , sizeof(NET_802_11_P2P_DATA));

        p2p_notify_events(t_nfy.i4NotifykId ,&_p2p_data);
    }

    if (strncmp(buf,WPS_EVENT_SUCCESS,strlen(WPS_EVENT_SUCCESS)) == 0)
    {
        WIFI_LOG(WIFI_DEBUG ,("[P2P]C2H:notify-app: WLAN_NOTIFY_WPS_SUCCESS\n"));

        t_nfy.i4NotifykId = WLAN_NOTIFY_WPS_SUCCESS;
        x_memset(&_p2p_data , 0 , sizeof(NET_802_11_P2P_DATA));

        p2p_notify_events(t_nfy.i4NotifykId ,(void *)&_p2p_data);
    }
    if (strncmp(buf, P2P_EVENT_STOP_CONNECT, strlen(P2P_EVENT_STOP_CONNECT))==0)
    {
        WIFI_LOG(WIFI_DEBUG ,("[P2P]C2H:notify-app: WLAN_NOTIFY_P2P_STOP_CONNECT\n"));
        t_nfy.i4NotifykId = WLAN_NOTIFY_P2P_STOP_CONNECT;
        x_memset(&_p2p_data , 0 , sizeof(NET_802_11_P2P_DATA));
        p2p_device_mac_parse(buf, strlen(P2P_EVENT_STOP_CONNECT), &_p2p_data);

        p2p_notify_events(t_nfy.i4NotifykId ,(void *)&_p2p_data);
    }

    if (strncmp(buf, P2P_EVENT_DEAUTH_AP, strlen(P2P_EVENT_DEAUTH_AP))==0)
    {
        WIFI_LOG(WIFI_DEBUG ,("[P2P]C2H:notify-app: WLAN_NOTIFY_P2P_DEAUTH_AP\n"));
        t_nfy.i4NotifykId = WLAN_NOTIFY_P2P_DEAUTH_AP;
        x_memset(&_p2p_data , 0 , sizeof(NET_802_11_P2P_DATA));
        p2p_device_mac_parse(buf, strlen(P2P_EVENT_DEAUTH_AP), &_p2p_data);

        if (doBooleanCommand("DISCONNECT","OK"))
        {
            WIFI_LOG(WIFI_ERROR,("C2H: notify-app: WLAN_NOTIFY_P2P_DEAUTH_AP SEND disconnect!\n"));
        }
        p2p_notify_events(t_nfy.i4NotifykId ,(void *)&_p2p_data);
    }

    return;
}


static int _string_split(char *strSrc, char *splitStr)
{
    char *p = NULL;

    if (strSrc == NULL || splitStr == NULL)
    {
        return -1;
    }

    p = strstr(strSrc, splitStr);
    if (p != NULL)
    {
        return (p - strSrc);
    }
    else
    {
        return -1;
    }
}

INT32 x_net_wlan_p2p_peer(char *reply, int relpyLen, char *deviceAddress)
{
    char _cmd_buf[WFD_CMD_BUF_LEN]={'\0'};

    if (!reply || !deviceAddress || relpyLen <= 0)
    {
        WIFI_LOG(WIFI_ERROR,(">>>>>>>p2p peer: reply or deviceAddress is null, or relpyLen <= 0 !\n"));
        return -1;
    }

    snprintf(_cmd_buf,sizeof(_cmd_buf), "P2P_PEER %s", deviceAddress);

    if (dop2pStringCommand(_cmd_buf, reply, relpyLen) == -1)
    {
        WIFI_LOG(WIFI_ERROR,(">>>>>>>p2p peer return error!\n"));
        return -1;
    }
    else
    {
        return 0;
    }
}

INT32 x_net_wlan_parse_p2p_peer_info(char *infoStr, 
    NET_802_11_P2P_PEER_INFO *p2pPeerInfo)
{
    char infoItem[256] = {'\0'};
    char *pos = NULL;
    char *p = NULL;
    int len = -1;

    if (!infoStr || !p2pPeerInfo) {
        WIFI_LOG(WIFI_ERROR,("[P2P]>>>>>>>x_net_wlan_parse_p2p_peer_info:" 
            " infoStr or p2pPeerInfo is null!\n"));
        return -1;
    }

    pos = infoStr;
    while (*pos != '\0') {
        len = _string_split(pos, "\n");
        if (len == -1 )
            len = strlen(pos);
        memset(infoItem, 0 , sizeof(infoItem));
        strncpy(infoItem, pos, len < (sizeof(infoItem) - 1)? len:(sizeof(infoItem) - 1));
        pos = pos + len + 1;

        WIFI_LOG(WIFI_INFO,("[P2P]>>>>>>>%s: element item=%s\n", 
            __FUNCTION__, infoItem));

        if ( strncmp(infoItem, "device_name=", strlen("device_name=")) == 0 ) {
            p = infoItem + strlen("device_name=");
            strncpy(p2pPeerInfo->_device_name, p, sizeof(p2pPeerInfo->_device_name) - 1);
            p2pPeerInfo->_device_name[sizeof(p2pPeerInfo->_device_name) - 1] = '\0';
            WIFI_LOG(WIFI_ERROR,("[P2P]>>>>>>>x_net_wlan_parse_p2p_peer_info:" 
            " device name=%s\n", p2pPeerInfo->_device_name));
        } else if ( strncmp(infoItem, "level=", strlen("level=")) == 0 ) {
            p = infoItem + strlen("level=");
            int rssi = 0;
            rssi = strtol(p, NULL, 10);
            p2pPeerInfo->_rssi = u1WlanDrv_NormalizeSignal(rssi);
            WIFI_LOG(WIFI_ERROR,("[P2P]>>>>>>>x_net_wlan_parse_p2p_peer_info:" 
                    " level=%%%d(%s)\n", p2pPeerInfo->_rssi, p));
        } else if ( strncmp(infoItem, "wfd_subelems=", strlen("wfd_subelems=")) == 0 ) {
            char rtspPort[5] = {'\0'};
            //wfd_subelems=00000600101c440032, 
            //00: subelement ID, 0006: subelement length, 0010: wfd device info, 1c44: RTSP port, 0032: max throughput
            p = infoItem + strlen("wfd_subelems=") + 10;
            if (*p != '\0') {
                WIFI_LOG(WIFI_INFO,("[P2P]>>>>>>>%s: p value=%s\n", __FUNCTION__, p));
                strncpy(rtspPort, p, 4);
                char *endpstr = NULL;
                p2pPeerInfo->_rtsp_port = strtol(rtspPort, &endpstr, 16);
                WIFI_LOG(WIFI_ERROR,("[P2P]>>>>>>>x_net_wlan_parse_p2p_peer_info:" 
                    " RTSP port=%d(%s)\n", p2pPeerInfo->_rtsp_port, rtspPort));
            } else {
                p2pPeerInfo->_rtsp_port = 0;
            }
            if (p2pPeerInfo->_rtsp_port == 0) {
                WIFI_LOG(WIFI_ERROR,("[P2P]>>>>>>>x_net_wlan_parse_p2p_peer_info:" 
                            " parse RTSP port failed\n"));
            }
        }else {
            continue;
        }
    }

    return 0;
}


INT32 x_net_wlan_get_p2p_peer_info(
    NET_802_11_P2P_PEER_INFO *peerInfo, char *deviceAddress)
{
    WIFI_LOG_ENTRY(WIFI_API, (""));
    char _reply[2048] = {'\0'};
    int ret = -1;

    if (!peerInfo || !deviceAddress)
    {
        WIFI_LOG(WIFI_WARNING,(">>>>>>>p2p peer: peerInfo or deviceAddress is null!\n"));
        return NET_WLAN_INVALID_PARAM;
    }

    ret = x_net_wlan_p2p_peer(_reply, sizeof(_reply), deviceAddress);
    if ( ret != 0 ) {
        return NET_WLAN_ERROR;
    }

    ret = x_net_wlan_parse_p2p_peer_info(_reply, peerInfo);
    if ( ret != 0 ) {
        return NET_WLAN_ERROR;
    }

    return NET_WLAN_OK;
}


INT32 x_net_wlan_p2p_getInterfaceAddress(char *interfaceAddress, char *deviceAddress)
{
    char peerInfo[256]={'\0'};
    int tokens[256] = {-1};
    int nameValue[256] = {-1};
    int pos = 0;
    int index = 0;
    int i = 0;
    if (deviceAddress == NULL)
    {
        WIFI_LOG(WIFI_ERROR,("[P2P]>>>>>>>p2p getInterfaceAddress: deviceAddress is null!\n"));
        return -1;
    }

    if (x_net_wlan_p2p_peer(peerInfo, sizeof(peerInfo), deviceAddress) == -1)
    {
        WIFI_LOG(WIFI_ERROR,("[P2P]>>>>>>>p2p getInterfaceAddress: p2p peer return error!\n"));
        return -1;
    }

    if (peerInfo[0] != '\n')
    {
        tokens[0] = 0;
        index = 1;
    }


    while ((pos = _string_split(&peerInfo[pos], "\n")) != -1)
    {
        pos += 1;
        if (peerInfo[pos] != '\0')
        {
            tokens[index++] = pos;
        }
        else
        {
            break;
        }
    }

    for ( i = 0; i < index; i++)
    {
        int j = 0;
        int temp = tokens[i];
        pos = temp;

        if (_string_split(&peerInfo[temp], "intended_addr=") == 0)
        {
            while ((pos = _string_split(&peerInfo[pos], "=")) != -1)
            {
                peerInfo[pos] = '\0';
                pos += 1;
                nameValue[j++] = pos;
            }//while

            if (j != 2)
            {
                break;
            }
            else
            {
                strncpy(interfaceAddress, &peerInfo[nameValue[1]],17);
                return 0;
            }
        }//if
    }//for

    return -1;

}


INT32 x_net_wlan_p2p_get_WPA_key(char *key)
{
    char keyInfo[128] = {'\0'};
    int pos = 0;

    char _cmd_buf[WFD_CMD_BUF_LEN]={'\0'};
    snprintf(_cmd_buf, sizeof(_cmd_buf),"P2P_GET_PASSPHRASE");

    if (dop2pStringCommand(_cmd_buf, keyInfo, sizeof(keyInfo)) == -1)
    {
        WIFI_LOG(WIFI_ERROR,("[P2P]>>>>>>>p2p get WPA_KEY: get p2p_get_passphrase error!\n"));
        return -1;
    }

    if ( strlen(keyInfo) < 8)
    {

        WIFI_LOG(WIFI_ERROR,("[P2P]>>>>>>>p2p get WPA_KEY error: key is not long enough\n"));
        return -1;
    }

    if ((pos = _string_split(keyInfo, "\n")) != -1)
    {
        strncpy(key,keyInfo,pos);
        key[pos] = '\0';
        return 0;
    }
    else
    {
        strncpy(key,keyInfo,strlen(keyInfo));
        WIFI_LOG(WIFI_ERROR,("[P2P]>>>>>>>p2p get WPA_KEY : not including terminating byte \n"));
        return 0;
    }
    return -1;


}

INT32 x_net_wlan_p2p_get_go_ssid(char *gossid)
{

    char status[256]={'\0'};
    int tokens[256] = {-1};
    int pre = 0;
    int pos = 0;
    int index = 0;
    int i = 0;
    char _cmd_buf[WFD_CMD_BUF_LEN]={'\0'};
    snprintf(_cmd_buf,sizeof(_cmd_buf), "STATUS");

    if (dop2pStringCommand(_cmd_buf, status, sizeof(status)) == -1)
    {
        WIFI_LOG(WIFI_ERROR,("[P2P]>>>>>>>p2p getGoSSID: get Status error!\n"));
        return -1;
    }


    if (status[0] != '\n')
    {
        tokens[0] = 0;
        index = 1;
    }


    while ((pos = _string_split(&status[pre], "\n")) != -1)
    {
        pos += 1;
        if (status[pos] != '\0')
        {
            pre += pos ;
            tokens[index++] = pre;
        }
        else
        {
            break;
        }
    }

    for (i = 0; i < index; i++)
    {

        int temp = tokens[i];

        if (_string_split(&status[temp], "ssid=") == 0)
        {
            if ((pos = _string_split(&status[temp], "\n")) != -1)
            {
                strncpy(gossid, &status[temp + strlen("ssid=")], pos - strlen("ssid="));
                WIFI_LOG(WIFI_ERROR,("[P2P]>>>>>>>p2p get_go_ssid: return 0\n"));
                return 0;
            }//while


            else
            {
                WIFI_LOG(WIFI_ERROR,("[P2P]>>>>>>>p2p get_go_ssid: ending error!\n"));
                return -1;
            }
        }//if
    }//for
    WIFI_LOG(WIFI_ERROR,("[P2P]>>>>>>>p2p get_go_ssid: can not find ssid \n"));
    return -1;

}
INT32 x_net_wlan_p2p_getDeviceAddress(char *deviceAddress)
{
    char status[256]={'\0'};
    int tokens[256] = {-1};
    int pre = 0;
    int pos = 0;
    int index = 0;
    int i = 0;

    char _cmd_buf[WFD_CMD_BUF_LEN]={'\0'};
    snprintf(_cmd_buf,sizeof(_cmd_buf), "STATUS");

    if (dop2pStringCommand(_cmd_buf, status, sizeof(status)) == -1)
    {
        WIFI_LOG(WIFI_ERROR,("[P2P]>>>>>>>p2p getDeviceAddress: get Status error!\n"));
        return -1;
    }


    if (status[0] != '\n')
    {
        tokens[0] = 0;
        index = 1;
    }



    while ((pos = _string_split(&status[pre], "\n")) != -1)
    {
        pos += 1;
        if (status[pos] != '\0')
        {
            pre += pos ;
            tokens[index++] = pre;
        }
        else
        {
            break;
        }
    }


    for ( i = 0; i < index; i++)
    {

        int temp = tokens[i];
        if (_string_split(&status[temp], "p2p_device_address=") == 0)
        {
            if ((pos = _string_split(&status[temp], "\n")) != -1)
            {
                strncpy(deviceAddress, &status[temp + strlen("p2p_device_address=")], pos - strlen("p2p_device_address="));
                WIFI_LOG(WIFI_ERROR,("[P2P]>>>>>>>p2p_device_address: return 0\n"));
                return 0;

            }//while

            else
            {
                WIFI_LOG(WIFI_ERROR,("[P2P]>>>>>>>p2p_device_address: ending error!\n"));
                return -1;
            }
        }//if
    }//for
    WIFI_LOG(WIFI_ERROR,("[P2P]>>>>>>>p2p_device_address: can not find device_address \n"));

    return -1;

}
#if PLATFORM_IS_TV
INT32 x_net_p2p_get_current_ip_info(char *name, NW_IP_INFO_T* pt_ip_info)
{
    if (name != NULL)
    {
        WIFI_LOG(WIFI_ERROR, ("[P2P]p2p get current ip info: name is NULL!\n"));
        return -1;
    }

    MT_DHCP4_INFO_T    t_info;

    if (x_dhcpc_get_info(name, &t_info) < 0)
    {
        WIFI_LOG(WIFI_ERROR, ("p2p get current ip info fail!\n"));
        return -1;
    }

    pt_ip_info->ui4_address = t_info.ui4_ipaddr;
    pt_ip_info->ui4_netmask = t_info.ui4_subnet;
    pt_ip_info->ui4_gw      = t_info.ui4_router;
    pt_ip_info->ui4_1st_dns = t_info.ui4_dns1;
    pt_ip_info->ui4_2nd_dns = t_info.ui4_dns2;

    WIFI_LOG(WIFI_DEBUG, ("[P2P]p2p get current ip info success!\n"));
    return 0;
}

/**
 * @brief   DHCP notify callback.
 * @param
 * @retval  VOID.
 */
static void _p2p_dhcpc_nfy_fct (DLNA_DHCPC_EV_T e_event)
{
    WIFI_LOG(WIFI_DEBUG, ("[P2P]Received DHCP notify -> %d\n", e_event));

    NET_802_11_NFY_T t_nfy;

    switch (e_event)
    {
    case DHCPC_EVENT_NEW_ADDRESS_DHCPv4:
    case DHCPC_EVENT_SUCCESS_DHCPv4:
    {
        WIFI_LOG(WIFI_DEBUG, ("[P2P]DHCPC_EVENT_NEW_ADDRESS_DHCPv4 or DHCPC_EVENT_NEW_ADDRESS_DHCPv4.\n"));
        t_nfy.i4NotifykId = WLAN_NOTIFY_P2P_DHCP_SUCCESS_DHCPv4;
        p2p_notify_events(t_nfy.i4NotifykId, NULL);
        break;
    }
    case DHCPC_EVENT_SUCCESS_LINKLOCAL:
        WIFI_LOG(WIFI_DEBUG, ("[P2P]DHCPC_EVENT_SUCCESS_LINKLOCAL.\n"));
        t_nfy.i4NotifykId = WLAN_NOTIFY_P2P_DHCP_SUCCESS_LINKLOCAL;
        p2p_notify_events(t_nfy.i4NotifykId, NULL);
        break;
    case DHCPC_EVENT_FAILURE_DHCPv4:
    case DHCPC_EVENT_FAILURE_LINKLOCAL:
        WIFI_LOG(WIFI_DEBUG, ("[P2P]DHCPC_EVENT_FAILURE_DHCPv4 or DHCPC_EVENT_FAILURE_LINKLOCAL.\n"));
        t_nfy.i4NotifykId = WLAN_NOTIFY_P2P_DHCP_FAILURE_LINKLOCAL;
        p2p_notify_events(t_nfy.i4NotifykId, NULL);
        break;
    default:
        WIFI_LOG(WIFI_ERROR, ("[P2P]Unknown Event!\n"));
        return;
    }
    return;
}

INT32 x_net_p2p_start_dhcpc(char *name)
{
    if (name == NULL)
    {
        WIFI_LOG(WIFI_ERROR, ("[P2P]p2p start dhcpc: name is NULL!\n"));
        return -1;
    }

    x_net_wlan_clean_file(P2P_SERVER_IP_FILE_PATH);

    return x_dhcpc_start(name, _p2p_dhcpc_nfy_fct);
}

INT32 x_net_p2p_restart_dhcpc(char *name)
{
    if (name == NULL)
    {
        WIFI_LOG(WIFI_ERROR, ("[P2P]p2p restart dhcpc: name is NULL!\n"));
        return -1;
    }

    return x_dhcpc_restart(name);
}

INT32 x_net_p2p_stop_dhcpc(char *name)
{
    if (name == NULL)
    {
        WIFI_LOG(WIFI_ERROR, ("[P2P]p2p stop dhcpc: name is NULL!\n"));
        return -1;
    }

    return x_dhcpc_stop(name);
}

#endif


char * _get_ip_from_mac (const char * str_mac,const char * file_path)
{
    WIFI_LOG(WIFI_DEBUG,("mac=%s, path=%s\n", str_mac, file_path));

    FILE* file = NULL;
    //char file_path[100] = "/proj/mtk40577/haifei/dhcpd.leases";
    char read_line[512] = {0};
    char macstr[64] = {0};
    char ipstr[16] = {'\0'};
    int  save_ip_flags = 0;  /* flag for if we have save the ip before we find the str_mac */
    int mac_found = 0;
    char *p_lease = NULL;
    char *p_bracket = NULL;
    int index = 0;

    if (!str_mac)
    {
        WIFI_LOG(WIFI_DEBUG,("args is invalided!!!\n"));
        return NULL;
    }

    memset(macstr, 0, sizeof(macstr));
    snprintf(macstr, sizeof(macstr), "hardware ethernet %s;", str_mac);

    file=fopen(file_path, "r");
    memset(str_ip , 0 , sizeof(str_ip));
    memset(ipstr, 0, sizeof(ipstr));

    if (file)
    {
        while (fgets(read_line, sizeof(read_line), file))
        {
            if (read_line[0] == '#')    /* skip notes */
            {
                continue;
            }

            //printf("read_line=%s", read_line);
            p_lease = strstr(read_line,"lease");
            p_bracket = strstr(read_line, "{");
            if (p_lease != NULL && p_bracket != NULL)
            {
                while (*p_lease)
                {
                    if (((*p_lease >= '0') && (*p_lease <= '9')) || (*p_lease == '.'))
                    {
                        ipstr[index++] = *p_lease;
                    }

                    p_lease++;
                }

                ipstr[index] = '\0';
                WIFI_LOG(WIFI_DEBUG,("######ip is %s\n", ipstr));
                index = 0;         /*ready to save the next ip*/
                if (ipstr[0]) /*if the str_ip is not empty*/
                {
                    save_ip_flags = 1;
                    mac_found = 0;
                }
                else
                {
                    save_ip_flags = 0;
                }
            }
            else
            {
                if (strstr(read_line, macstr))
                {
                    WIFI_LOG(WIFI_DEBUG,("The mac exists\n"));
                    mac_found = 1;
                    memcpy(str_ip, ipstr, sizeof(str_ip));
                }
            }

            memset(read_line , 0 , sizeof(read_line));
        }

        fclose(file);

        if (save_ip_flags == 1 && mac_found == 1)
        {
            return str_ip;
        }
    }
    else
    {
        WIFI_LOG(WIFI_DEBUG,("fopen error\n"));
    }

    return NULL;
}



INT32 get_file_stat(char * file_path , struct stat * stat_val)
{
    if ((file_path == NULL) || (stat_val == NULL))
    {
        WIFI_LOG(WIFI_DEBUG,("get_file_stat failed ,invalid param!\n"));
        return -2;
    }
    if (stat(file_path,stat_val)<0)
    {
        WIFI_LOG(WIFI_DEBUG,("The function stat() failed!!\n"));
        return -1;
    }
    return 0;

}


BOOL is_file_changed(struct stat stat_o , struct stat stat_d)
{
    //WIFI_LOG(WIFI_DEBUG,("old size is %d, new size is %d \n",(int)stat_o.st_size,(int)stat_d.st_size));
    if ( ( stat_o.st_mtime != stat_d.st_mtime ) && ( (stat_o.st_size + 200)<= stat_d.st_size  ) )
    {
        return TRUE;
    }
    return FALSE;
}

typedef void (* dhcpd_notify_fn)(int);
void* poll_dhcpd_lease(void* pIn)
{
    int cnt = 400;
    int ret = 0;
    int file_changed = -1;
    int find_binding = -1;
    char cmd[100] = {0};
    dhcpd_notify_fn cb = (dhcpd_notify_fn )pIn;

    prctl(PR_SET_NAME,"polldhcples",0,0,0);
    pthread_detach(pthread_self());
    snprintf(cmd, 100, "cat %s | grep \"binding state active\"", DHCPD_LEASE_FILE_PATH);
    while ((cnt--)&& (cancel_moniter_ip == false) )
    {
        struct stat _stat_now = {0};
        ret = get_file_stat(DHCPD_LEASE_FILE_PATH, &_stat_now);
        if (0 == ret)
        {
            file_changed = is_file_changed(g_ori_lease_stat,_stat_now);
            find_binding = doScriptCmd(cmd);
            if (file_changed && !find_binding)
            {
                memcpy(&g_ori_lease_stat, &_stat_now, sizeof(struct stat));
                cb(0);
                WIFI_LOG(WIFI_DEBUG,("[WIFI] dhcpd lease acquired! \n"));
                return NULL;
            }
        }

        usleep(50000);
    }
    WIFI_LOG(WIFI_DEBUG,("[WIFI] dhcpd lease acquire timeout or cancel by user!\n"));
    cb(-1);
    return NULL;
}


INT32 x_net_wlan_monitor_dhcpd_lease(void* pIn)
{
    WIFI_LOG_ENTRY(WIFI_API, (""));
    int ret = 0;
    cancel_moniter_ip = false;
    pthread_t p_thread;

    ret = pthread_create(&p_thread, NULL, poll_dhcpd_lease, pIn);
    if (ret)
        WIFI_LOG(WIFI_DEBUG,("[WIFI] Create dhcpd polling thread fail! - %s\n", strerror(errno)));

    return ret;
}
INT32 _P2P_Get_Random_SSID(char *ssid, int buflen)
{
    char ssid_tmp[33] = {"DIRECT-"};
    char ssid_range[63] = {"abcdefghijklmnopqrstuvwxyz0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ"};
    struct timeval tv;
    int index = 0;
    int i = 0;

    if (!ssid)
    {
        WIFI_LOG(WIFI_DEBUG, ("Invalid parameter, %s", __FUNCTION__));
        return -1;
    }

    gettimeofday(&tv, NULL);
    srand((unsigned)tv.tv_usec);

    for (i=0 ; i < 2 ; i++)
    {
        index = rand() % (strlen(ssid_range) - 1);
        ssid_tmp[strlen(ssid_tmp)] = ssid_range[index];
    }

    ssid_tmp[sizeof(ssid_tmp) - 1] = '\0';
    strncpy(ssid, ssid_tmp, buflen - 1);
    ssid[buflen - 1] = '\0';

    WIFI_LOG(WIFI_DEBUG, ("Random SSID=%s", ssid_tmp));

    return 0;
}

INT32 _P2P_Get_Random_Key(char *key, int buflen)
{
    char key_tmp[9] = {'\0'};
    char key_range[63] = {"abcdefghijklmnopqrstuvwxyz0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ"};
    struct timeval tv;
    int index = 0;
    int i = 0;

    if (!key)
    {
        WIFI_LOG(WIFI_DEBUG, ("Invalid parameter, %s", __FUNCTION__));
        return -1;
    }

    gettimeofday(&tv, NULL);
    srand((unsigned)tv.tv_usec);

    for (i=0 ; i < 8 ; i++)
    {
        index = rand() % (strlen(key_range) - 1);
        key_tmp[i] = key_range[index];
    }

    key_tmp[sizeof(key_tmp) - 1] = '\0';
    strncpy(key, key_tmp, buflen - 1);
    key[buflen - 1] = '\0';

    WIFI_LOG(WIFI_DEBUG, ("Random key=%s", key_tmp));

    return 0;
}

INT32 x_net_wlan_cancel_moniter_ip(void)
{

    WIFI_LOG(WIFI_DEBUG,(">>>>>>> enter %s\n",__FUNCTION__));
    cancel_moniter_ip = true ;
    return 0;
}


INT32 x_net_stop_dhcpd(void)
{
    char _cmd_buf[WFD_CMD_BUF_LEN] = {0};

#if PLATFORM_IS_BDP
    snprintf(_cmd_buf , sizeof(_cmd_buf), "/usr/bin/pkill dhcpd");
#else
    snprintf(_cmd_buf , sizeof(_cmd_buf),"sh %s %s", DHCPD_KILL_SCRIPT_PATH, p2p_go_name);
#endif
    WIFI_LOG(WIFI_DEBUG,(">>>>>>> %s \n",_cmd_buf));

    doScriptCmd(_cmd_buf);
    return 0;
}
#ifdef WLAN_SUPPORT_P2P_CFG80211
INT32 x_net_wlan_cfg80211_get_persistent_network_id(int *networkId_buf, int networkIdbuflen, int *networkNum)
{
    WIFI_LOG_ENTRY(WIFI_API, (""));

    char cmd[64] = {0};
    char reply[1024] = {'\0'};
    size_t replylen = sizeof(reply);
    char groupStr[128] = {'\0'};
    char *pos = NULL, *end = NULL;
    int networkid = -1;
    int i = 0;
    int count = 0;
    char numStr[8] = {'\0'};
    char *p = NULL;

    if ( !networkId_buf || networkIdbuflen <= 0 || !networkNum )
        WIFI_LOG (WIFI_DEBUG,("Invalid parameters, %s \n", __FUNCTION__ ));

    memset( networkId_buf, -1, networkIdbuflen );

    /* get networks list */
    snprintf(cmd, sizeof(cmd), "LIST_NETWORKS");

    if ( x_net_p2p_check_init_state() == NET_WLAN_NOT_INIT )
    {
        memset(p2p_name, 0, sizeof(p2p_name));
        strncpy(p2p_name, "p2p0", sizeof(p2p_name) - 1);
        if ( wifi_open_connection(TRUE, WIFI_SOCKET_PATH) != 0)
        {
            WIFI_LOG(WIFI_ERROR,("[P2P] Failed to create temp connection with wpa_supplicant!\n"));
            return NET_WLAN_ERROR;
        }

        if (p2p_command(cmd, reply, &replylen) != 0)
        {
            WIFI_LOG (WIFI_DEBUG,("Failed to run \"%s\"!\n", cmd ));
            return NET_WLAN_ERROR;
        }

        x_net_p2p_close_connection();
    }
    else
    {
        if (dop2pStringCommand(cmd, reply, sizeof(reply)) == -1)
        {
            WIFI_LOG (WIFI_DEBUG,(">>>>>> %s  return error!\n", cmd ));
            return NET_WLAN_ERROR;
        }
    }

    pos = reply;

    /* Remove the prefixion "network id / ssid / bssid / flags\n" */
    end = strchr(pos, '\n');
    if ( end )
        pos = ++end;
    else if ( strstr(pos, "network id") )
        pos += strlen(pos);

    count = 0;

    /*-----------------------------------
        ** network id / ssid / bssid / flags
        ** 0    DIRECT-6p   any [CURRENT]
        ** 1    DIRECT-6p   ba:ee:65:01:9f:89   [DISABLED][P2P-PERSISTENT]
        **-------------------------------------*/
    while ( pos && *pos != '\0' && count < networkIdbuflen )
    {
        networkid = -1;
        end = strchr(pos, '\n');
        if ( end )
        {
            memset(groupStr, 0, sizeof(groupStr));
            strncpy(groupStr, pos, end - pos);
            WIFI_LOG (WIFI_DEBUG,("Persistent group string = %s, %s \n", groupStr, __FUNCTION__ ));
            p = groupStr;
            i = 0;
            memset(numStr, 0, sizeof(numStr));
            while ((*p >= '0') && (*p <= '9') && (i < sizeof(numStr)))
            {
                numStr[i++] = *p++;
            }
            WIFI_LOG (WIFI_DEBUG,("network id string = %s, len=%d, %s \n", numStr, strlen(numStr), __FUNCTION__ ));
            networkid = atoi(numStr);
            networkId_buf[count++] = networkid;
            pos = ++end;
        }
        else if ( strlen(pos) > strlen("DIRECT") )
        {
            WIFI_LOG (WIFI_DEBUG,("Persistent group string = %s, %s \n", pos, __FUNCTION__ ));
            p = pos;
            i = 0;
            memset(numStr, 0, sizeof(numStr));
            while (*p >= '0' && *p <= '9' && i < sizeof(numStr))
            {
                numStr[i++] = *p++;
            }
            WIFI_LOG (WIFI_DEBUG,("network id string = %s, len=%d, %s \n", numStr, strlen(numStr), __FUNCTION__ ));
            networkid = atoi(numStr);
            networkId_buf[count++] = networkid;
            pos += strlen(pos);
        }
        else
            break;
    }

    *networkNum = count;

    WIFI_LOG (WIFI_DEBUG,("Total persistent network = %d\n", *networkNum));

    return NET_WLAN_OK;
}
#endif

/*add for service discovery*/
INT32 x_net_p2p_serv_disc_enable(BOOL enable)
{
	char _cmd_buf[32]={0};
	int value = enable?1:0;
	
	snprintf(_cmd_buf, sizeof(_cmd_buf), "p2p_serv_disc_enable %d",value);
		
	if (!dop2pBooleanCommand(_cmd_buf, "OK"))
	{
		WIFI_LOG(WIFI_DEBUG, ("[Wifi MW] >>>>>>> %s fail,\n",_cmd_buf));
		return NET_WLAN_ERROR;
	}
		
	return NET_WLAN_OK;

}

INT32 x_net_p2p_serv_disc_resq_pre(INT16 protocolType,INT16 version, char *requestData, char *responseData)
{
	char _cmd_buf[256]={'\0'};
	
	if (protocolType != 2)
	{		
		WIFI_LOG(WIFI_ERROR, (">>>>>>>Serv_resq_pre ProtocolType is not 2, but %d!,,set default\n",protocolType));
		protocolType = 2;	
	}
	
	if (version != 16)
	{			
		WIFI_LOG(WIFI_ERROR, (">>>>>>>Serv_resq_pre UpnpVersion is  %d!\n",version));				
	}
	
	if ((!requestData) || (!responseData))
	{
		if (!requestData)
			WIFI_LOG(WIFI_ERROR, (">>>>>>>Serv_resq_pre requestData is NULL!!\n"));
		if (!responseData)
			WIFI_LOG(WIFI_ERROR, (">>>>>>>Serv_resq_pre responseData is NULL!!\n"));
		return FALSE;
	}
	
	if (requestData && (strlen(requestData) > 100))
	{
		WIFI_LOG(WIFI_ERROR, (">>>>>>>Serv_resq_pre RequestData size is too long\n"));
		return FALSE;
	}
	
	if (responseData && (strlen(responseData) > 100))
	{
		WIFI_LOG(WIFI_ERROR, (">>>>>>>Serv_resq_pre responseData size is too long\n"));
		return FALSE;
	}
	
	snprintf(_cmd_buf, sizeof(_cmd_buf), "plp_p2p_serv_disc_resp_pre %d %d %s %s", protocolType, version, requestData, responseData);
	
	if (!dop2pBooleanCommand(_cmd_buf, "OK"))
	{
		WIFI_LOG(WIFI_ERROR, (">>>>>>>p2p serv disc resp pre return error!\n"));
		return FALSE;
	}
	else
	{
		return TRUE;
	}

}

INT32 x_net_p2p_serv_disc_req(char *mac, INT16 protocolType,INT16 version, char *requestData)
{
	char _cmd_buf[256]={'\0'};
		
	if (protocolType != 2)
	{		
		WIFI_LOG(WIFI_ERROR, (">>>>>>>Serv_disc_req ProtocolType is not 2, but %d!,,set default\n",protocolType));
		protocolType = 2;	
	}
		
	if (version != 16)
	{			
		WIFI_LOG(WIFI_ERROR, (">>>>>>>Serv_disc_req UpnpVersion is  %d!\n",version));				
	}		

	if (!requestData)
	{
		WIFI_LOG(WIFI_ERROR, (">>>>>>>Serv_disc_req requestData is NULL!!\n"));
		return FALSE;
	}

	if (!mac)
	{
		WIFI_LOG(WIFI_ERROR, (">>>>>>>Serv_disc_req MAC is NULL!!\n"));
		return FALSE;
	}
	
	if (requestData && (strlen(requestData) > 100))
	{
		WIFI_LOG(WIFI_ERROR, (">>>>>>>Serv_disc_req RequestData size is too long\n"));
		return FALSE;
	}
		
	
		
	snprintf(_cmd_buf, sizeof(_cmd_buf), "plp_p2p_serv_disc_req %s %d %d %s", mac, protocolType, version, requestData);
		
	if (!dop2pBooleanCommand(_cmd_buf, "OK"))
	{
		WIFI_LOG(WIFI_ERROR, (">>>>>>>p2p serv disc resp pre return error!\n"));
		return FALSE;
	}
	else
	{
		return TRUE;
	}

}
/*add for service discovery*/

