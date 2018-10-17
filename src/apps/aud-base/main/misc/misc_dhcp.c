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
/* middleware level*/
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <pthread.h>
#include "u_appman.h"
#include "u_app_thread.h"
#include "u_aee.h"
#include "u_cli.h"
#include "u_os.h"


/* application level */
#include "u_app_def.h"
#include "u_amb.h"
#include "u_am.h"
#include "u_timerd.h"
#include "misc.h"
#include "u_net_ni_if.h"
#include "u_net_wlan.h"
#include "u_net_common.h"
#include "u_net_oid.h"
#include "u_dhcpc_api.h"
#include "u_dbg.h"
#include "u_wifi_setting.h"
#include "u_ipcd.h"
#include "u_assistant_stub.h"

#ifdef CONFIG_ADAPTOR_APP_CTRL
#include "app_ctrl_misc.h"
#endif /* CONFIG_ADAPTOR_APP_CTRL */

BOOL b_dhcp_lease = FALSE;
CHAR psz_ni_name[6] = {0};
BOOL b_autoip_config = FALSE;
BOOL b_ip_is_obtained = FALSE;
BOOL b_auto_ip_timer_start = FALSE;

static BOOL b_ip_is_obtained_from_dhcp = FALSE;
static NET_DHCP_STATUS_T dhcp_status;
NET_MANU_SETTING_INFO_T t_cur_net_info={0};
static BOOL is_ntpdate_run = FALSE;

extern MISC_OBJ_T misc_this_obj;
extern INT32  fg_wifi_connect_id;
extern BOOL   need_respose_wifi_connect_rpc;

INT32 a_network_wifi_ni_name (CHAR *psz_ni_name)
{
    MISC_DHCP_FUNCTION_BEGIN;
    INT32 i4_ret;

    i4_ret = c_net_get_wifi_ni_name(psz_ni_name);

    if(strlen(psz_ni_name)>0)
    {
        printf("<MISC_DHCP> strlen(psz_ni_name)>0 \n");
    }
    else
    {
        printf("<MISC_DHCP> strlen(psz_ni_name) =0 \n");
        strncpy(psz_ni_name, "ra0", 4);
    }
    printf("<MISC_DHCP> a_network_wifi_ni_name:>>> %s \n",psz_ni_name);

    MISC_DHCP_FUNCTION_END;

    return APPUTILR_OK;
}

INT32 a_network_init_wlan(VOID)
{
    MISC_DHCP_FUNCTION_BEGIN;

    INT32 i4_ret;
    HANDLE_T h_app = NULL_HANDLE;
    MISC_MSG_T pt_event ;
    dhcp_status.b_wireless = FALSE;
    dhcp_status.b_ethernet= FALSE;

    /*enable wlan0 interface*/
#if IFCONFIG_FLAG
    int i = 100;
    pid_t status;

    while(i > 0)
    {
        status = system("ifconfig wlan0 up");
        if (status == -1)
        {
            printf("a_network_init_wlan ifconfig wlan0 up first fail.\n");
            u_thread_delay(50);
            i--;
            continue;
        }
        else
        {
            printf("a_network_init_wlan ifconfig wlan0 up first successfully.status = 0x%x, i = %d\n",status,i);

            if (WIFEXITED(status))
            {
                printf("a_network_init_wlan ifconfig wlan0 up second successfully.status = 0x%x,i=%d\n",status,i);
                if ( WEXITSTATUS(status))
                {
                    printf("a_network_init_wlan ifconfig wlan0 up second fail.error = %d\n", WEXITSTATUS(status));
                    u_thread_delay(50);
                    i--;
                    continue;
                }
                else
                {
                    printf("a_network_init_wlan ifconfig wlan0 up third successfully.i = %d\n",i);
                    break;
                }

            }
            else
            {
                printf("a_network_init_wlan ifconfig wlan0 up second fail. error=%d\n",WEXITSTATUS(status));
                u_thread_delay(50);
                i--;
                continue;
            }

        }
    }
#endif

    /*start wpa_supplicant*/
    system("wpa_supplicant -Dnl80211 -iwlan0 -c/etc/wpa_supplicant.conf &");

#if WPA_CUPPLICANT
    i = 100;
    pid_t status_wpa_supplicant;

    while(i > 0)
    {
        status_wpa_supplicant = system("wpa_cli -iwlan0 ping | grep PONG");
        if (status_wpa_supplicant == -1)
        {
            printf("a_network_init_wlan iwpa_cli first fail.\n");
            u_thread_delay(50);
            i--;
            continue;
        }
        else
        {
            printf("a_network_init_wlan wpa_cli first successfully.status = 0x%x,i=%d\n",status_wpa_supplicant,i);

            if (WIFEXITED(status))
            {
                printf("a_network_init_wlan wpa_cli second successfully.status = 0x%x,i=%d\n",status_wpa_supplicant,i);
                if ( WEXITSTATUS(status))
                {
                    printf("a_network_init_wlan wpa_cli second fail.error = %d\n", WEXITSTATUS(status_wpa_supplicant));
                    u_thread_delay(50);
                    i--;
                    continue;
                }
                else
                {
                    printf("a_network_init_wlan wpa_clip third successfully.i=%d\n",i);
                    break;
                }

            }
            else
            {
                printf("a_network_init_wlan wpa_cli second fail. error=%d\n",WEXITSTATUS(status_wpa_supplicant));
                u_thread_delay(50);
                i--;
                continue;
            }

        }
    }
#endif

    c_net_network_init(IF_TYPE_IEEE80211);
#if 0
    a_network_wifi_ni_name(psz_ni_name);
#endif
    memcpy(psz_ni_name,"wlan0",6);
    printf("a_network_wifi_ni_name psz_ni_name is %s \n",psz_ni_name);
    i = 100;
    while(i > 0)
    {
        i4_ret = c_net_wlan_open_connection(psz_ni_name);
        if(i4_ret < 0)
        {
            printf("c_net_wlan_open_connection fail!\n");
            u_thread_delay(50);
            i--;
            continue;
        }
        else
        {
            printf("c_net_wlan_open_connection successfully. i = %d!\n", i);
            break;
        }
    }

    /*add wifi mac temp solution*/
    UINT8 mac_def[6] = {0};
    char mac[20] ={0};
    c_net_wlan_get_mac_addr(mac_def);
    if(mac_def[0]==0 && mac_def[1]==0 && mac_def[2]==0 &&
            mac_def[3]==0 && mac_def[4]==0 && mac_def[5]==0)
    {
        printf("c_net_wlan_get_mac_addr is 0:0:0:0:0\n");
    }
    else
    {
        snprintf(mac,19,"%02X:%02X:%02X:%02X:%02X:%02X",mac_def[0], mac_def[1], mac_def[2], mac_def[3], mac_def[4], mac_def[5]);
        printf("c_net_wlan_get_mac_addr is %s\n",mac);
#ifndef CONFIG_ADAPTOR_APP_CTRL
        a_cfg_set_wlan_mac(mac);
#endif /*CONFIG_ADAPTOR_APP_CTRL*/
    }

#ifndef CONFIG_ADAPTOR_APP_CTRL
    pt_event.ui4_msg=CONNECT_FAVORATE_AP;
    i4_ret = u_am_get_app_handle_from_name(&h_app,WIFI_SETTING_THREAD_NAME);
    if(APPUTILR_OK != i4_ret)
    {
        printf("<MISC_DHCP> get handle fail!,i4_ret=%ld\n",i4_ret);
    }
    i4_ret = u_app_send_msg(h_app,
            E_APP_MSG_TYPE_MISC,
            &pt_event,
            sizeof(MISC_MSG_T),
            NULL,
            NULL);

    if(APPUTILR_OK != i4_ret)
    {
        printf("<MISC_DHCP> send msg fail!\n");
    }
    else
    {
        printf("<MISC_DHCP> send msg suc line=%d!\n",__LINE__);
    }
#endif /*CONFIG_ADAPTOR_APP_CTRL*/

    MISC_DHCP_FUNCTION_END;

    return APPUTILR_OK;
}

INT32 a_network_get_connect_status (UINT8 i4IfType)
{
    MISC_DHCP_FUNCTION_BEGIN;

    INT32 i4_ret=0;
    INT32 ui4_status=0;
    NET_802_11_ASSOCIATE_T _rWlanAssociate = {{0}};

    switch (i4IfType)
    {
        case IF_TYPE_ETHERNET:
            {
                printf("<MISC_DHCP> a_network_get_connect_status\n");
                i4_ret = c_net_ni_get(APP_NI_ETHERNET,
                        OID_CMN_IF_CONNECT_STATUS,
                        (VOID *) &ui4_status);
                break;
            }

        case IF_TYPE_IEEE80211:
            {
                i4_ret = a_cfg_get_wlan_ap(&_rWlanAssociate);
                printf("<MISC_DHCP> SSID:%s, SsidLen:%d, eAssocCase:%d, eAuthMode:%d, e_AuthCipher:%d Priority:%d \n",
                        _rWlanAssociate.t_Ssid.ui1_aSsid,
                        _rWlanAssociate.t_Ssid.ui4_SsidLen,
                        _rWlanAssociate.e_AssocCase,
                        _rWlanAssociate.e_AuthMode,
                        _rWlanAssociate.e_AuthCipher,
                        _rWlanAssociate.ui1_Priority);

                i4_ret = c_net_wlan_wpa_get_status(&ui4_status, &_rWlanAssociate);
                if (NET_WLAN_DRV_INTERFACE_ERR == i4_ret)
                {
                    ui4_status=NI_DRV_EV_WLAN_DISASSOC;
                }

                if (NET_WLAN_NOT_INIT == i4_ret)
                {
                    ui4_status=NI_DRV_EV_WLAN_DISASSOC;
                }

                if (0 == i4_ret)
                {
                    if (WLAN_WPA_COMPLETED == ui4_status)
                    {

                        ui4_status=NI_DRV_EV_WLAN_ASSOC;
                    }
                    else
                    {
                        ui4_status=NI_DRV_EV_WLAN_DISASSOC;
                    }
                }
                break;
            }

        default:
            break;
    }

    printf("<MISC_DHCP> Net type:%d ,connect_status %d \n",i4IfType, ui4_status);

    MISC_DHCP_FUNCTION_END;

    return ui4_status;
}

UINT8 a_network_auto_ip_timer_start(UINT32  ui4_msec)
{
    MISC_DHCP_FUNCTION_BEGIN;

    INT32 i4_ret=APPUTILR_OK;
    printf("<MISC_DHCP> a_network_auto_ip_timer_start: status =%d\n",b_auto_ip_timer_start);

    if (b_auto_ip_timer_start)
    {
        printf("<MISC_DHCP>  timer is started now!dont need to start it agian\n");
    }
    else
    {

#if 0
        i4_ret = u_timer_start(misc_this_obj.h_network_auto_ip,
                (ui4_msec),
                X_TIMER_FLAG_REPEAT,
                misc_timer_cb_fct,
                NULL);
        if (OSR_OK != i4_ret)
        {
            printf("<COMM_FUNC_NETWORK>Start timer failed, error code = %d\n",i4_ret);
            return APPUTILR_OK;
        }
#else
        MISC_TIMER_MSG_E e_auto_ip_msg=TIMER_MSG_AUTO_IP;
        misc_this_obj.auto_ip_timer.e_flags=X_TIMER_FLAG_REPEAT;
        misc_this_obj.auto_ip_timer.ui4_delay=ui4_msec;

        i4_ret = u_timer_start(misc_this_obj.h_app,
                &misc_this_obj.auto_ip_timer,
                (void *)&e_auto_ip_msg,
                sizeof(MISC_TIMER_MSG_E));
        if (OSR_OK != i4_ret)
        {
            printf("<MISC_DHCP>Start timer failed, error code = %d\n",i4_ret);
            return APPUTILR_OK;
        }

#endif
        b_auto_ip_timer_start = TRUE;
        printf("<MISC_DHCP> start timer is success !\n");
    }

    MISC_DHCP_FUNCTION_END;

    return APPUTILR_OK;
}

UINT8 a_network_auto_ip_timer_stop(VOID)
{
    MISC_DHCP_FUNCTION_BEGIN;

    INT32 i4_ret=APPUTILR_OK;
    printf("<MISC_DHCP> a_network_auto_ip_timer_stop: status =%d\n",b_auto_ip_timer_start);

    if (b_auto_ip_timer_start)
    {
        i4_ret = u_timer_stop(misc_this_obj.auto_ip_timer.h_timer);
        if (OSR_OK != i4_ret)
        {
            printf("<MISC_DHCP>c_timer_stop  failed, error code = %d\n",i4_ret);
            return APPUTILR_OK;
        }
        b_auto_ip_timer_start = FALSE;
        printf("<MISC_DHCP> stop timer is success !\n");
    }
    else
    {
        printf("<MISC_DHCP> auto_ip_timer is stopped now!dont need to stop it agian\n");
    }

    MISC_DHCP_FUNCTION_END;

    return APPUTILR_OK;
}

UCHAR * a_network_uint32_to_ipaddr(UCHAR *puc_ip_addr, UINT32 len, UINT32 ui4_ip)
{
    MISC_DHCP_FUNCTION_BEGIN;

    if(len < 16)
    {
        printf("<MISC_DHCP> a_network_uint32_to_ipaddr, buffer too small! skipped!\n");
        return puc_ip_addr;
    }

    ui4_ip = htonl(ui4_ip);
    snprintf((CHAR*)puc_ip_addr,len,"%d.%d.%d.%d",
            (ui4_ip&0xFF000000)>>24,
            (ui4_ip&0xFF0000)>>16,
            (ui4_ip&0xFF00)>>8,
            (ui4_ip&0xFF));

    MISC_DHCP_FUNCTION_END;

    return puc_ip_addr;
}

BOOL a_network_check_ip_addr_avalid(NET_MANU_SETTING_INFO_T pt_net_info)
{
    MISC_DHCP_FUNCTION_BEGIN;

    CHAR ip_addr[16] = {0};

    printf("<MISC_DHCP> ip         : %s\n", a_network_uint32_to_ipaddr(ip_addr, 16, pt_net_info.ui4_ipaddr));
    printf("<MISC_DHCP> subnet mask: %s\n", a_network_uint32_to_ipaddr(ip_addr, 16, pt_net_info.iu4_subnet));
    printf("<MISC_DHCP> gateway    : %s\n", a_network_uint32_to_ipaddr(ip_addr, 16, pt_net_info.ui4_gateway));
    printf("<MISC_DHCP> dns1       : %s\n", a_network_uint32_to_ipaddr(ip_addr, 16, pt_net_info.ui4_dns1));
    printf("<MISC_DHCP> dns2       : %s\n", a_network_uint32_to_ipaddr(ip_addr, 16, pt_net_info.ui4_dns2));

    if ((0 == pt_net_info.ui4_ipaddr)||(0 == pt_net_info.iu4_subnet))
    {
        printf("<MISC_DHCP> IP is not avalid\n");
        return FALSE;
    }

    printf("<MISC_DHCP> IP is avalid\n");

    MISC_DHCP_FUNCTION_END;

    return TRUE;
}

INT32 a_network_get_current_info(NET_MANU_SETTING_INFO_T *pt_net_info)
{
    MISC_DHCP_FUNCTION_BEGIN;

    INT32 i4_ret=0;
    UINT8 ui1IfType=IF_TYPE_IEEE80211;
    MT_DHCP4_INFO_T t_net_info;
    UINT32 ui4_status;

    ui4_status = a_network_get_connect_status(ui1IfType);
    switch (ui4_status)
    {
        case NI_DRV_EV_ETHERNET_PLUGIN:
            {
                printf("<MISC_DHCP> mode is auto, get ip info.\n");
                if(IF_TYPE_ETHERNET == ui1IfType)
                {
                    i4_ret = c_dhcpc_get_info(APP_NI_ETHERNET, &t_net_info);
                }
                else
                {
                    i4_ret = c_dhcpc_get_info(psz_ni_name, &t_net_info);
                }

                if (DHCPC_R_OK != i4_ret)
                {
                    printf("<MISC_DHCP> c_dhcpc_get_info %d\n", i4_ret);
                    return APPUTILR_FAIL;
                }

                pt_net_info->ui4_ipaddr = t_net_info.ui4_ipaddr;
                pt_net_info->iu4_subnet = t_net_info.ui4_subnet;
                pt_net_info->ui4_gateway = t_net_info.ui4_router;
                pt_net_info->ui4_dns1 = t_net_info.ui4_dns1;
                pt_net_info->ui4_dns2 = t_net_info.ui4_dns2;
                break;
            }

        case NI_DRV_EV_ETHERNET_UNPLUG:
            {
                pt_net_info->ui4_ipaddr = 0;
                pt_net_info->iu4_subnet = 0;
                pt_net_info->ui4_gateway = 0;
                pt_net_info->ui4_dns1 = 0;
                pt_net_info->ui4_dns2 = 0;
                break;
            }

        default:
            printf("<MISC_DHCP> ui4_status is NI_DRV_EV_UNKNOW\n");
            break;
    }

    a_network_check_ip_addr_avalid(*pt_net_info);

    MISC_DHCP_FUNCTION_END;

    return APPUTILR_OK;
}

#if 1
static INT32 _network_get_dhcp_lease_time(UINT32 *pui4_lease_time)
{
    MISC_DHCP_FUNCTION_BEGIN;

    INT32    i4_ret = APPUTILR_FAIL;
    int      dhcp_lease_fd = 0;
    UINT32   ui4_read = 0;
    INT32    i4_i = 0;
    CHAR     ac_data[DHCP_LEASE_TIME_NUM_MAX_LEN+1];
    BOOL     b_data_valid = FALSE;

    if (NULL == pui4_lease_time)
    {
        return APPUTILR_FAIL;
    }

    memset(ac_data, 0, sizeof(CHAR)*(DHCP_LEASE_TIME_NUM_MAX_LEN+1));
    dhcp_lease_fd = open(DHCP_LEASE_TIME_PATH,O_RDONLY);
    if(dhcp_lease_fd < 0)
    {
        printf("<MISC_DHCP> open failed at L%d\n", __LINE__);
        goto ERR;
    }

    ui4_read = read(dhcp_lease_fd,ac_data,DHCP_LEASE_TIME_NUM_MAX_LEN);
    if(ui4_read < 0)
    {
        printf("<MISC_DHCP>read failed at L%d\n", __LINE__);
        goto ERR;
    }
    else
    {
        printf("<MISC_DHCP>read ui4_read is %d\n", ui4_read);
    }

    printf("<MISC_DHCP> <AUTOIP>INFO: get the lease time %s!\r\n",ac_data);

    *pui4_lease_time = 0;

    if (ui4_read > 1)
    {
        i4_i = ui4_read;
        if ('0' == ac_data[0])
        {
            *pui4_lease_time = DHCP_LEASE_TIME_DEFAULT;
            i4_ret = APPUTILR_OK;
            goto ERR;
        }

        while (i4_i > 0)
        {
            /* for linux file, the "Enter" character is "0x0a" */
            if (0x0a == ac_data[ui4_read - i4_i])
            {
                b_data_valid = TRUE;
                break;
            }

            if (ac_data[ui4_read - i4_i] >= '0' && ac_data[ui4_read - i4_i] <= '9')
            {
                *pui4_lease_time = 10*(*pui4_lease_time) + (ac_data[ui4_read - i4_i] - '0');
            }
            else
            {
                b_data_valid = FALSE;
                break;
            }

            i4_i--;
        }

        if (!b_data_valid&&(i4_i != 0))
        {
            *pui4_lease_time = DHCP_LEASE_TIME_DEFAULT;
            i4_ret = APPUTILR_OK;
            goto ERR;
        }

    }
    else
    {
        *pui4_lease_time = DHCP_LEASE_TIME_DEFAULT;
    }

    i4_ret = APPUTILR_OK;

ERR:

    i4_ret = close(dhcp_lease_fd);

    MISC_DHCP_FUNCTION_END;

    return i4_ret;
}

#else

static INT32 _network_get_dhcp_lease_time(UINT32 *pui4_lease_time)
{
    MISC_DHCP_FUNCTION_BEGIN

#if 1//CONFIG_LINUX_OS
        INT32    i4_ret = APPUTILR_FAIL;
    HANDLE_T h_file = NULL_HANDLE;
    UINT32   ui4_read = 0;
    INT32    i4_i = 0;
    CHAR     ac_data[DHCP_LEASE_TIME_NUM_MAX_LEN+1];
    BOOL     b_data_valid = FALSE;

    if (NULL == pui4_lease_time)
    {
        return APPUTILR_FAIL;
    }

    memset(ac_data, 0, sizeof(CHAR)*(DHCP_LEASE_TIME_NUM_MAX_LEN+1));
    i4_ret = c_fm_open(FM_ROOT_HANDLE,
            DHCP_LEASE_TIME_PATH, //
            FM_READ_ONLY,
            0666,
            FALSE,
            &h_file);//file handle
    if(FMR_OK != i4_ret || NULL_HANDLE == h_file)
    {
        printf("<MISC_DHCP> c_fm_open failed at L%d\n", __LINE__);
        goto ERR;
    }

    i4_ret = c_fm_read(h_file,ac_data,DHCP_LEASE_TIME_NUM_MAX_LEN,&ui4_read);
    if(FMR_OK != i4_ret)
    {
        printf("<MISC_DHCP>c_fm_read failed at L%d\n", __LINE__);
        goto ERR;
    }

    printf("<MISC_DHCP> <AUTOIP>INFO: get the lease time %s!\r\n",ac_data);
    *pui4_lease_time = 0;
    if (ui4_read > 1)
    {
        i4_i = ui4_read;
        if ('0' == ac_data[0])
        {
            *pui4_lease_time = DHCP_LEASE_TIME_DEFAULT;
            i4_ret = APPUTILR_OK;
            goto ERR;
        }

        while (i4_i > 0)
        {
            /* for linux file, the "Enter" character is "0x0a" */
            if (0x0a == ac_data[ui4_read - i4_i])
            {
                b_data_valid = TRUE;
                break;
            }

            if (ac_data[ui4_read - i4_i] >= '0'&& ac_data[ui4_read - i4_i] <= '9')
            {
                *pui4_lease_time = 10*(*pui4_lease_time) + (ac_data[ui4_read - i4_i] - '0');
            }
            else
            {
                b_data_valid = FALSE;
                break;
            }

            i4_i--;
        }

        if (!b_data_valid&&(i4_i != 0))
        {
            *pui4_lease_time = DHCP_LEASE_TIME_DEFAULT;
            i4_ret = APPUTILR_OK;
            goto ERR;
        }

    }
    else
    {
        *pui4_lease_time = DHCP_LEASE_TIME_DEFAULT;
    }

    i4_ret = APPUTILR_OK;

ERR:
    if (NULL_HANDLE != h_file)
    {
        i4_ret = u_fm_close(h_file);
        if(FMR_OK != i4_ret)
        {
            printf("<MISC_DHCP> u_fm_close failed at L%d\n", __LINE__);
        }
        h_file = NULL_HANDLE;
    }

    MISC_DHCP_FUNCTION_END

        return i4_ret;
#else
    *pui4_lease_time = DHCP_LEASE_TIME_DEFAULT;

    MISC_DHCP_FUNCTION_END

        return APPUTILR_OK;
#endif
}
#endif

static VOID _network_dhcpok_cb(DLNA_DHCPC_EV_T e_event)
{
    MISC_DHCP_FUNCTION_BEGIN;
    int i4_ret = 0;
    HANDLE_T h_app = NULL_HANDLE;
    MISC_NOTIFY_MSG_T     t_notify_msg  = {0};

    switch (e_event)
    {
        case DHCPC_EVENT_SUCCESS_DHCPv4:
            {
                printf("<MISC_DHCP> _network_dhcpok_cb:DHCPC_EVENT_SUCCESS_DHCPv4 \n");
            }
        case DHCPC_EVENT_NEW_ADDRESS_DHCPv4:
            {
                printf("<MISC_DHCP> _network_dhcpok_cb:DHCPC_EVENT_NEW_ADDRESS_DHCPv4 \n");
                printf("<MISC_DHCP> DHCP start OK.\n");
                t_notify_msg.ui4_msg = AUTO_IP_EVENT_SUCCESS_DHCPv4;
                b_ip_is_obtained = TRUE;
                b_ip_is_obtained_from_dhcp = TRUE;

                break;
            }

        case DHCPC_EVENT_FAILURE_DHCPv4:
            {
                printf("<MISC_DHCP> _network_dhcpok_cb:DHCPC_EVENT_FAILURE_DHCPv4 \n");
                printf("<MISC_DHCP> DHCP start Fail.\n");

                /*send wifi_status to assistant_stub */
                if(TRUE == need_respose_wifi_connect_rpc)
                {
                    need_respose_wifi_connect_rpc = FALSE;
                    ASSISTANT_STUB_WIFI_CONNECT_RESPONSE_T wifi_status ={0};
                    memcpy(wifi_status.command, STRING_WIFI_CONNECT, ASSISTANT_STUB_COMMAND_MAX_LENGTH);
                    wifi_status.wifi_status= GET_IP_FAIL;
                    wifi_status.id = fg_wifi_connect_id;

                    /*send  wifi_status to assistant_stub */
                    printf("<MISC_DHCP> get ip fail!!!\n");
                    i4_ret = u_am_get_app_handle_from_name(&h_app, ASSISTANT_STUB_THREAD_NAME);
                    if(0 != i4_ret)
                    {
                        printf("<MISC_DHCP> get handle fail!,i4_ret=%ld\n",i4_ret);
                    }
                    i4_ret = u_app_send_appmsg(h_app,
                            E_APP_MSG_TYPE_ASSISTANT_STUB,
                            MSG_FROM_WIFI_SETTING,
                            ASSISTANT_STUB_CMD_WIFI_CONNECT,
                            &wifi_status,
                            sizeof(wifi_status));
                    if(0 == i4_ret)
                    {
                        printf("<MISC_DHCP> u_app_send_appmsg success !!!\n");
                    }
                    else
                    {
                        printf("<MISC_DHCP> u_app_send_appmsg fail !!!\n");
                    }
                }

                t_notify_msg.ui4_msg = AUTO_IP_EVENT_FAIL_DHCPv4;
                b_ip_is_obtained = FALSE;
                b_ip_is_obtained_from_dhcp = FALSE;
                break;
            }

#if 0//CONFIG_APP_SUPPORT_IPV6
        case DHCPC_EVENT_SUCCESS_AUTOv6:
            {
                printf("<COMM_FUNC_NETWORK> _network_dhcpok_cb:DHCPC_EVENT_SUCCESS_DHCPv6 \n");
                printf("IPv6 address set OK.\n");
                t_notify_msg.ui4_msg = AUTO_IP_EVENT_SUCCESS_DHCPv6;
                b_ipv6_is_obtained = TRUE;
                b_ip_is_obtained_from_dhcp = TRUE;
                break;
            }
        case DHCPC_EVENT_FAILURE_AUTOv6:
            {
                printf("<COMM_FUNC_NETWORK> _network_dhcpok_cb:DHCPC_EVENT_FAILURE_DHCPv6 \n");
                printf("IPv6 address set fail.\n");
                t_notify_msg.ui4_msg = AUTO_IP_EVENT_FAIL_DHCPv6;
                b_ipv6_is_obtained = FALSE;
                b_ip_is_obtained_from_dhcp = FALSE;
                break;
            }
#endif
        default:
            printf("<MISC_DHCP> _network_dhcpok_cb:default \n");
            break;
    }

    a_network_dhcp_msg_proc(t_notify_msg.ui4_msg);

    MISC_DHCP_FUNCTION_END;

    return ;
}

INT32 a_network_dhcpc_start(UINT8 net_type)
{
    MISC_DHCP_FUNCTION_BEGIN;

    INT32 i4_ret;
    INT32 ui4_status;
    printf("<MISC_DHCP> dhcp status: %d \n",dhcp_status.b_wireless);

    if (!dhcp_status.b_wireless)
    {
        i4_ret = c_dhcpc_start(psz_ni_name,_network_dhcpok_cb);

        if (i4_ret != DHCPC_R_OK)
        {
            printf("<MISC_DHCP> c_dhcpc_start Fail %d\n", i4_ret);
        }
        else
        {
            printf("<MISC_DHCP> c_dhcpc_start Success!\n");
            dhcp_status.b_wireless = TRUE;
        }
    }
    else
    {
        i4_ret = c_dhcpc_restart(psz_ni_name);

        if (i4_ret != DHCPC_R_OK)
        {
            printf("<MISC_DHCP> c_dhcpc_restart Fail %d\n", i4_ret);
        }
        else
        {
            printf("<MISC_DHCP> c_dhcpc_restart Success!\n");
            dhcp_status.b_wireless = TRUE;
        }
    }

    MISC_DHCP_FUNCTION_END;

    return APPUTILR_OK;
}

INT32 a_network_dhcpc_stop(UINT8 net_type)
{
    MISC_DHCP_FUNCTION_BEGIN;

    INT32 i4_ret;

    printf("<MISC_DHCP> a_network_dhcpc_stop net %d \n",net_type);

    printf("<MISC_DHCP> dhcp status: %d \n",dhcp_status.b_wireless);

    if (dhcp_status.b_wireless)
    {
        i4_ret = c_dhcpc_stop(psz_ni_name);
        if (i4_ret != DHCPC_R_OK)
        {
            printf("<MISC_DHCP> c_dhcpc_stop fail %d\n", i4_ret);
        }
        else
        {
            printf("<MISC_DHCP> c_dhcpc_stop success!\n");
            dhcp_status.b_wireless=FALSE;
        }
    }

    MISC_DHCP_FUNCTION_END;

    return APPUTILR_OK;
}

INT32 a_network_ipexpire(VOID)
{
    MISC_DHCP_FUNCTION_BEGIN;

    INT32 i4_ret=0;
    UINT8 ui1IfType=IF_TYPE_IEEE80211;

    i4_ret = a_network_dhcpc_start(ui1IfType);

    MISC_DHCP_FUNCTION_END;

    return i4_ret;
}

VOID a_network_device_active_msg_proc(INT32 i4IfType,INT32 i4Event)
{
    MISC_DHCP_FUNCTION_BEGIN;

    INT32    i4_ret = 0;
    HANDLE_T h_app = NULL_HANDLE;
    MISC_DEVICE_ACTIVE_MSG_T t_device_active_msg  = {0};

    t_device_active_msg.i4Event=i4Event;
    t_device_active_msg.i4IfType=i4IfType;

    if (!misc_this_obj.b_app_init_ok)
    {
        return AEER_FAIL;
    }

    i4_ret = u_am_get_app_handle_from_name(&h_app, MISC_THREAD_NAME);
    if(MISC_APPR_OK != i4_ret)
    {
        printf("<MISC_DHCP> get handle fail!,i4_ret=%ld\n",i4_ret);
    }
    i4_ret = u_app_send_appmsg(h_app,
            E_APP_MSG_TYPE_MISC,
            MSG_FROM_WIFI_SETTING,
            MISC_PRI_DEVICE_ACTIVE_MSG,
            &t_device_active_msg,
            sizeof(MISC_DEVICE_ACTIVE_MSG_T));

    MISC_DHCP_FUNCTION_END;

    return ;
}

VOID a_misc_handle_network_device_active_event(INT32 i4IfType,INT32 i4Event)
{
    MISC_DHCP_FUNCTION_BEGIN;
    a_network_device_active_msg_proc(i4IfType,i4Event);
    MISC_DHCP_FUNCTION_END;
}

INT32 _network_ip_mon_register(INT32 i4IfType,ni_ev_notify_fnct pf_notify)
{
    MISC_DHCP_FUNCTION_BEGIN;

    INT32 i4_ret;

    printf("<MISC_DHCP> register ni monitor event, i4IfType: %d\n", i4IfType);

    switch (i4IfType)
    {
        case IF_TYPE_ETHERNET:
            i4_ret = c_net_ni_reg_ev_notify(APP_NI_ETHERNET, NI_DRV_EV_IP_ADDRESS_EXPIRED, pf_notify);
            i4_ret = c_net_ni_reg_ev_notify(APP_NI_ETHERNET, NI_DRV_EV_IP_ADDRESS_CHANGED, pf_notify);
            break;

        case IF_TYPE_IEEE80211:
            i4_ret = c_net_ni_reg_ev_notify(psz_ni_name, NI_DRV_EV_IP_ADDRESS_EXPIRED, pf_notify);
            i4_ret = c_net_ni_reg_ev_notify(psz_ni_name, NI_DRV_EV_IP_ADDRESS_CHANGED, pf_notify);

            break;

        default:
            break;
    }

    MISC_DHCP_FUNCTION_END;

    return APPUTILR_OK;
}

INT32 _network_ip_mon_unregister(INT32 i4IfType,ni_ev_notify_fnct pf_notify)
{
    MISC_DHCP_FUNCTION_BEGIN;

    INT32 i4_ret;
    printf("<MISC_DHCP> unregister ni monitor event, i4IfType: %d\n", i4IfType);

    switch (i4IfType)
    {
        case IF_TYPE_ETHERNET:
            i4_ret = c_net_ni_unreg_ev_notify(APP_NI_ETHERNET, NI_DRV_EV_IP_ADDRESS_EXPIRED, pf_notify);
            i4_ret = c_net_ni_unreg_ev_notify(APP_NI_ETHERNET, NI_DRV_EV_IP_ADDRESS_CHANGED, pf_notify);
            break;

        case IF_TYPE_IEEE80211:
            i4_ret = c_net_ni_unreg_ev_notify(psz_ni_name, NI_DRV_EV_IP_ADDRESS_EXPIRED, pf_notify);
            i4_ret = c_net_ni_unreg_ev_notify(psz_ni_name, NI_DRV_EV_IP_ADDRESS_CHANGED, pf_notify);
            break;

        default:
            break;
    }

    MISC_DHCP_FUNCTION_END;

    return APPUTILR_OK;
}

VOID a_network_dhcp_msg_proc(NI_DRV_EV_T e_event)
{
    MISC_DHCP_FUNCTION_BEGIN;

    INT32    i4_ret = 0;
    HANDLE_T h_app = NULL_HANDLE;

    if (NI_DRV_EV_ETHERNET_UNPLUG == e_event)
    {
        b_ip_is_obtained_from_dhcp = FALSE;
    }

    MISC_NOTIFY_MSG_T t_notify_msg  = {0};
    t_notify_msg.ui4_msg = (UINT32)e_event;


    i4_ret = u_am_get_app_handle_from_name(&h_app, MISC_THREAD_NAME);
    if(MISC_APPR_OK != i4_ret)
    {
        printf("<MISC_DHCP> get handle fail!,i4_ret=%ld\n",i4_ret);
    }


    i4_ret = u_app_send_appmsg(h_app,
            E_APP_MSG_TYPE_MISC,
            MSG_FROM_WIFI_SETTING,
            MISC_PRI_DHCP_MSG,
            &t_notify_msg,
            sizeof(MISC_NOTIFY_MSG_T));

    MISC_DHCP_FUNCTION_END;

    return ;
}

VOID _network_wlan_ip_mon_cb_fct(NI_DRV_EV_T e_event)
{
    MISC_DHCP_FUNCTION_BEGIN;

    if(NI_DRV_EV_IP_ADDRESS_CHANGED == e_event)
    {
        printf("<MISC_DHCP> ip address changed , current interface is wlan\n");
    }
    if(NI_DRV_EV_IP_ADDRESS_EXPIRED == e_event)
    {
        printf("<MISC_DHCP> ip address expired , current interface is wlan\n");
    }
    if(NI_DRV_EV_IP_ADDRESSv6_CHANGED == e_event)
    {
        printf("<MISC_DHCP> ipv6 address changed , current interface is wlan\n");
    }

    a_network_dhcp_msg_proc(e_event);

    MISC_DHCP_FUNCTION_END;
}

VOID a_network_handle_device_active_event(INT32 i4IfType,INT32 i4Event)
{
    MISC_DHCP_FUNCTION_BEGIN;

    if(NETWORK_DEVICE_ACTIVE_EVENT == i4Event)
    {
        printf("<MISC_DHCP> WLAN connected to AP\n");
        a_network_dhcp_msg_proc(NI_DRV_EV_WLAN_ASSOC);
        _network_ip_mon_register(IF_TYPE_IEEE80211,_network_wlan_ip_mon_cb_fct);
    }
    else if(NETWORK_DEVICE_DEACTIVE_EVENT == i4Event)
    {
        printf("<MISC_DHCP> WLAN disconnected from AP \n");
        a_network_dhcp_msg_proc(NI_DRV_EV_WLAN_DISASSOC);
        _network_ip_mon_unregister(IF_TYPE_IEEE80211,_network_wlan_ip_mon_cb_fct);
    }
    else
    {
        printf("<MISC_DHCP> UNKONW DEVICE EVENT\n");
    }
    MISC_DHCP_FUNCTION_END;

    return ;
}

INT32  misc_device_active_msg_process(MISC_DEVICE_ACTIVE_MSG_T * t_device_active_msg)
{
    MISC_DHCP_FUNCTION_BEGIN;
    a_network_handle_device_active_event(t_device_active_msg->i4IfType,
            t_device_active_msg->i4Event);
    MISC_DHCP_FUNCTION_END;

    return AEER_OK;
}

UINT8 a_network_auto_ip_timer_msg_proc(VOID)
{
    MISC_DHCP_FUNCTION_BEGIN;

    INT32 i4_ret;
    UINT8 ui1IfType = 0;
    INT32 ui4_status;

    printf("<MISC_DHCP> cuurent interface is :IF_TYPE_IEEE80211 \n");

    ui1IfType = IF_TYPE_IEEE80211;

    ui4_status= a_network_get_connect_status(ui1IfType);
    if(NI_DRV_EV_ETHERNET_PLUGIN != ui4_status)
    {
        printf("<MISC_DHCP> net %d is not plugin , return\n",ui1IfType);
        a_network_auto_ip_timer_stop();
        return APPUTILR_OK;
    }

    if (b_autoip_config) /*b_autoip_config means manual config ip*/
    {
        i4_ret = a_network_dhcpc_stop(ui1IfType);
        if (APPUTILR_OK != i4_ret)
        {
            printf("<MISC_DHCP> Update a_network_dhcpc_stop failed, code = %d\n", i4_ret);
        }
        i4_ret = a_network_dhcpc_start(ui1IfType);
        if (APPUTILR_OK != i4_ret)
        {
            printf("<MISC_DHCP> Update a_network_dhcpc_start failed, code = %d\n", i4_ret);
        }
    }
    else
    {
        b_dhcp_lease = TRUE;
        i4_ret = a_network_dhcpc_start(ui1IfType);
        if (APPUTILR_OK != i4_ret)
        {
            printf("<MISC_DHCP> Update a_network_dhcpc_start failed, code = %d\n", i4_ret);
        }
    }

    MISC_DHCP_FUNCTION_END;

    return APPUTILR_OK;

}

VOID _misc_process_auto_ip_timer_msg(VOID)
{
    MISC_DHCP_FUNCTION_BEGIN;

    INT32 i4_ret;
    i4_ret = a_network_auto_ip_timer_msg_proc();
    if (AEER_OK != i4_ret)
    {
        printf("<MISC_DHCP> Update a_network_auto_ip_timer_msg_proc failed, code = %d\n", i4_ret);
    }

    MISC_DHCP_FUNCTION_END;

    return;
}

#if 0
VOID misc_timer_cb_fct(HANDLE_T  h_timer,VOID*     pv_tag)
{
    MISC_DHCP_FUNCTION_BEGIN

        INT32               i4_ret = 0;
    MISC_TIMER_MSG_T    misc_timermsg = {0};
    BOOL                b_send = FALSE;

#if 1//CONFIG_SUPPORT_NETWORK_AUTO_IP
    if (h_timer == misc_this_obj.h_network_auto_ip)
    {
        misc_timermsg.ui1_timer_type = MISC_NETWORK_AUTO_IP_TIMER;
        b_send = TRUE;
    }
#endif
    if (b_send)
    {
#if 0
        i4_ret = misc_send_msg(
                MISC_PRI_TIMER_MSG,
                &misc_timermsg,
                sizeof(MISC_TIMER_MSG_T));
        if (AEER_OK != i4_ret)
        {
            printf("<MISC_DHCP>Send Timer msg error in function %s\n", __FUNCTION__);
            return;
        }

#endif
    }

    MISC_DHCP_FUNCTION_END

        return;
}
#endif

INT32 a_network_plugin_notify_msg_proc(VOID)
{
    MISC_DHCP_FUNCTION_BEGIN;

    INT32 i4_ret;
    UINT32 ui4_status = 0;
    UINT8 ui1IfType=IF_TYPE_IEEE80211;

#if 1//CONFIG_SUPPORT_NETWORK_AUTO_IP
    a_network_auto_ip_timer_stop();
    b_autoip_config = FALSE;
#endif
    a_network_dhcpc_stop(IF_TYPE_IEEE80211);
    a_network_dhcpc_start(IF_TYPE_IEEE80211);
#if 0//CONFIG_APP_SUPPORT_IPV6
    UINT8 ui1_ipv6_en;
    i4_ret = a_cfg_get_ipv6_enable(&ui1_ipv6_en);
    COMM_FUNC_CHK_FAIL(i4_ret,i4_ret);

    if(!ui1_ipv6_en)
    {
        a_network_set_ipv6_all(ui1IfType);
    }
    else
    {
        a_network_stop_ipv6_all(ui1IfType);
    }
#endif

    MISC_DHCP_FUNCTION_END;

    return APPUTILR_OK;
}

static INT32 _misc_handle_dhcp_plugin_notify()
{
    MISC_DHCP_FUNCTION_BEGIN;

    INT32 i4_ret = 0;

    i4_ret = a_network_plugin_notify_msg_proc();
    if (APPUTILR_OK != i4_ret)
    {
        printf("<MISC_DHCP>_misc_handle_dhcp_plugin_notify failed, error code = %d, at L%d\n", i4_ret, __LINE__);

        MISC_DHCP_FUNCTION_END;

        return i4_ret;
    }

    MISC_DHCP_FUNCTION_END;

    return i4_ret;
}

static INT32 _misc_handle_dhcp_unplug_notify()
{
    MISC_DHCP_FUNCTION_BEGIN;

    INT32 i4_ret = 0;

    i4_ret=a_network_dhcpc_stop(IF_TYPE_IEEE80211);
    if (AEER_OK != i4_ret)
    {
        printf("<MISC_DHCP>_misc_handle_dhcp_unplug_notify failed, error code = %d, at L%d\n", i4_ret, __LINE__);

        MISC_DHCP_FUNCTION_END;

        return i4_ret;
    }

    MISC_DHCP_FUNCTION_END;

    return i4_ret;
}

static INT32  _misc_handle_dhcp_expire_notify()//add msz00450 20080520 for DHCP
{
    MISC_DHCP_FUNCTION_BEGIN;

    INT32 i4_ret = 0;

    i4_ret = a_network_ipexpire();
    if (AEER_OK != i4_ret)
    {
        printf("<MISC_DHCP>_misc_handle_dhcp_expire_notify failed, error code = %d, at L%d\n", i4_ret, __LINE__);

        MISC_DHCP_FUNCTION_END;

        return i4_ret;
    }

    MISC_DHCP_FUNCTION_END;

    return i4_ret;
}

INT32 _misc_ntpdate(VOID)
{
    INT8 i4_ret = 0;
    FILE *pf_tmp = NULL;
    BOOL is_ntpdate_success = FALSE;
    char word[20] = {0};
    INT8 num = 0;

    while(!is_ntpdate_success)
    {
        system("ntpdate pool.ntp.org 2>&1 | tee /tmp/ntpdate.txt");
        pf_tmp = fopen("/tmp/ntpdate.txt", "r+");
        if(pf_tmp == NULL)
        {
            printf("ntpdate.txt file does not exist!\n");
            continue;
        }
        if(EOF == fgetc(pf_tmp))
        {
            printf("if ntpdate.txt is empty, return!\n");
            if(NULL != pf_tmp)
            {
                fclose(pf_tmp);
            }
            break;
        }
        while(!feof(pf_tmp))
        {
            fscanf(pf_tmp,"%s",word);
            //printf("ntpdate word:%s!!\n",word);
            if(0 == strncmp(word,"offset",6))
            {
                printf("ntpdate success!!\n");
                num++;
            }
        }
        if(num != 0)
        {
            is_ntpdate_success = TRUE;
            system("hwclock -w");
        }
        else
        {
            printf("ntpdate fail!!\n");
            sleep(1);
        }
        if(NULL != pf_tmp)
        {
            fclose(pf_tmp);
        }
        num = 0;
    }

#ifdef CONFIG_ADAPTOR_APP_CTRL
    /*start ntpd*/
    if (system("/usr/sbin/ntpd -u ntp:ntp -p /run/ntpd.pid -g") == -1) {
        printf("[app_ctrl_ntp] start ntpd fail.");
        return AEER_FAIL;
    }
#else
    is_ntpdate_run = FALSE;
    printf("exit ntpdate process!!\n");
#endif /* CONFIG_ADAPTOR_APP_CTRL */

    return AEER_OK;
}
static INT32 _misc_ntpdate_thread(VOID)
{
    INT32 i4_ret;
    pthread_t ntid;
    pthread_attr_t t_attr;

    i4_ret = pthread_attr_init(&t_attr);
    if(0 != i4_ret)
    {
        printf("pthread_attr_init error!\n");
        goto ATTR_INIT_ERR;
    }

    i4_ret = pthread_attr_setdetachstate(&t_attr, PTHREAD_CREATE_DETACHED);
    if(0 != i4_ret)
    {
        printf("pthread_attr_setdetachstate error!\n");
        goto ATTR_SET_ERR;
    }

    is_ntpdate_run = TRUE;
    i4_ret = pthread_create(&ntid, &t_attr, _misc_ntpdate, NULL);
    if(0 != i4_ret)
    {
        printf("pthread_create error!\n");
        is_ntpdate_run = FALSE;
        goto ATTR_SET_ERR;
    }

    pthread_attr_destroy(&t_attr);
    return AEER_OK;

ATTR_SET_ERR:
    pthread_attr_destroy(&t_attr);
ATTR_INIT_ERR:

    return AEER_FAIL;
}

int misc_mdns_get_dev_type_name(char * dev_type_name)
{
    int ret = 0;
    UINT8 mac_def[6] = {0};

    c_net_wlan_get_mac_addr(mac_def);
    if(mac_def[0]==0 && mac_def[1]==0 && mac_def[2]==0 &&
            mac_def[3]==0 && mac_def[4]==0 && mac_def[5]==0)
    {
        printf("<MISC_DHCP> c_net_wlan_get_mac_addr is 0:0:0:0:0\n");
        ret = -1;
    }
    else
    {
        snprintf(dev_type_name,25,"mt8516%02X:%02X:%02X:%02X:%02X:%02X",mac_def[0], mac_def[1], mac_def[2], mac_def[3], mac_def[4], mac_def[5]);
        printf("<MISC_DHCP> c_net_wlan_get_mac_addr is %s\n",dev_type_name);
    }

    return  ret ;
}

static INT32 _misc_handle_dhcp_ipchange_notify()
{
    MISC_DHCP_FUNCTION_BEGIN;

    INT32 i4_ret;
    CHAR typename[25] = {0};
    const UINT32 MDNS_MAX = 128;
    CHAR mdnsregister[MDNS_MAX];
    HANDLE_T h_app = NULL_HANDLE;

    memset(mdnsregister, 0, MDNS_MAX);

    /* update system time */
    printf("<MISC_DHCP> ntpdate update system time\n");
    //system("ntpdate pool.ntp.org &");
    if(!is_ntpdate_run)
    {
        _misc_ntpdate_thread();
    }
    else
    {
        printf("<MISC_DHCP> ntpdate is running\n");
    }

    printf("<MISC_DHCP> touch /tmp/wifi_ok\n");
    system("touch /tmp/wifi_ok");

#ifdef CONFIG_ADAPTOR_APP_CTRL
    /*send  WIFI network status change to app ctrl */
    app_ctrl_misc_report_wifi_connection_status(CONNECT_SUCCESS);

    /*send  NTP status change to app ctrl */
    app_ctrl_misc_report_ntp_status(is_ntpdate_run);
#endif /* CONFIG_ADAPTOR_APP_CTRL */

    /*send wifi_status to assistant_stub */
    if(TRUE == need_respose_wifi_connect_rpc)
    {
        need_respose_wifi_connect_rpc = FALSE;
        ASSISTANT_STUB_WIFI_CONNECT_RESPONSE_T wifi_status ={0};
        memcpy(wifi_status.command, STRING_WIFI_CONNECT, ASSISTANT_STUB_COMMAND_MAX_LENGTH);
        wifi_status.wifi_status= CONNECT_SUCCESS;
        wifi_status.id = fg_wifi_connect_id;

        /*send  wifi_status to assistant_stub */
        printf("<MISC_DHCP> get ip OK!!!\n");
        i4_ret = u_am_get_app_handle_from_name(&h_app, ASSISTANT_STUB_THREAD_NAME);
        if(0 != i4_ret)
        {
            printf("<MISC_DHCP> get handle fail!,i4_ret=%ld\n",i4_ret);
        }
        i4_ret = u_app_send_appmsg(h_app,
                E_APP_MSG_TYPE_ASSISTANT_STUB,
                MSG_FROM_WIFI_SETTING,
                ASSISTANT_STUB_CMD_WIFI_CONNECT,
                &wifi_status,
                sizeof(wifi_status));
        if(0 == i4_ret)
        {
            printf("<MISC_DHCP> u_app_send_appmsg success !!!\n");
        }
        else
        {
            printf("<MISC_DHCP> u_app_send_appmsg fail !!!\n");
        }
    }

    ASSISTANT_STUB_NETWORK_STATUS_CHANGE_T network_status_change = {0};
    NET_802_11_ASSOCIATE_T _rSavedWlanAssociate={{0}};
    a_cfg_get_wlan_ap(&_rSavedWlanAssociate);

    strncpy(network_status_change.command,"/system/network_status_change",ASSISTANT_STUB_COMMAND_MAX_LENGTH);
    network_status_change.quantity = 100;
    strncpy(network_status_change.status,"connect",ASSISTANT_STUB_STATUS_MAX_LENGTH);
    strncpy(network_status_change.ssid,
            _rSavedWlanAssociate.t_Ssid.ui1_aSsid,
            NET_802_11_MAX_LEN_SSID);
    snprintf(network_status_change.bssid, ASSISTANT_STUB_BSSID_MAX_LENGTH, "%x:%x:%x:%x:%x:%x",
             _rSavedWlanAssociate.t_Bssid[0], _rSavedWlanAssociate.t_Bssid[1], _rSavedWlanAssociate.t_Bssid[2],
             _rSavedWlanAssociate.t_Bssid[3], _rSavedWlanAssociate.t_Bssid[4], _rSavedWlanAssociate.t_Bssid[5]);

    /*send  network status change to sm */
    i4_ret = u_am_get_app_handle_from_name(&h_app, SM_THREAD_NAME);
    if(MISC_APPR_OK != i4_ret)
    {
        printf("<MISC_DHCP> get handle fail!,i4_ret=%ld\n",i4_ret);
    }
    i4_ret = u_app_send_appmsg(h_app,
            E_APP_MSG_TYPE_MISC,
            MSG_FROM_WIFI_SETTING,
            ASSISTANT_STUB_CMD_NETWORK_STATUS_CHANGE,
            &network_status_change,
            sizeof(ASSISTANT_STUB_NETWORK_STATUS_CHANGE_T));
    if(0 == i4_ret)
    {
        printf("<MISC_DHCP> u_app_send_appmsg success !!!\n");
    }
    else
    {
        printf("<MISC_DHCP> u_app_send_appmsg fail !!!\n");
    }
    system("killall mdnsd");
    system("/usr/bin/mdnsd");

	#if CONFIG_SUPPORT_DLNA_APP
	system("killall -9 gmediarender");
	system("/usr/lib64/gmediarender &");
	#endif

    misc_mdns_get_dev_type_name(typename);

    snprintf(mdnsregister, MDNS_MAX, "dns-sd -R %s _http._tcp local 3023 /tmp/elianmdns &", typename);
    printf("<MISC_DHCP> mdnsregister is %s\n",mdnsregister);

    system(mdnsregister);

    wifi_setting_notify_bt_wifi_setup_result();

    MISC_DHCP_FUNCTION_END;

    return AEER_OK;
}

INT32 a_network_auto_ip_dhcpc_success_proc(VOID)
{
    MISC_DHCP_FUNCTION_BEGIN;

    INT32   i4_ret=0;
    UINT32  ui4_lease_time = AUTO_DHCP_TIMER_DELAY;

    if(b_dhcp_lease ==FALSE)
    {
        a_network_get_current_info(&t_cur_net_info);
    }

    i4_ret = _network_get_dhcp_lease_time(&ui4_lease_time);
    if (APPUTILR_OK != i4_ret)
    {
        printf("<MISC_DHCP> can't get the lease time!  \n");
    }

    printf("<MISC_DHCP> Get lease_time=%d s.\n",ui4_lease_time);
    ui4_lease_time = ui4_lease_time*1000;

    a_network_auto_ip_timer_stop();
    b_autoip_config = FALSE;
    a_network_auto_ip_timer_start(ui4_lease_time);

    MISC_DHCP_FUNCTION_END;

    return APPUTILR_OK;
}

INT32 a_network_auto_ip_dhcpc_fail_proc(VOID)
{
    MISC_DHCP_FUNCTION_BEGIN;

    INT32   i4_ret=0;
    INT32   cur_net_type = IF_TYPE_IEEE80211;
    UCHAR   ac_mac_info[10] = {0};
    UINT8   wlan_mac_info[6] = {0};
    INT32   i4_count = 0;
    UINT32  ui4_status = 0;

    if(b_autoip_config)
    {
        printf("<MISC_DHCP> Auto IP have get,do not do auto ip again\n");
        return APPUTILR_OK;
    }

    while(1)
    {
        i4_count ++;

        if(IF_TYPE_IEEE80211 == cur_net_type)
        {
            i4_ret = c_net_wlan_get_mac_addr(wlan_mac_info);
            i4_ret = c_net_autoip_config(psz_ni_name, (CHAR*)wlan_mac_info);
        }
        else
        {
            i4_ret = NET_FAILED;
        }

        if(NET_OK == i4_ret)
        {
            printf("<MISC_DHCP>c_net_autoip_config return NET_OK i4_count=%x\n",i4_count);
            break;
        }
        else
        {
            u_thread_delay(1000);
            printf("<MISC_DHCP>c_net_autoip_config return fail i4_count=%x\n",i4_count);
        }
    }

    a_network_auto_ip_timer_stop();
    b_autoip_config = TRUE;
    a_network_auto_ip_timer_start(AUTO_DHCP_TIMER_DELAY);

    MISC_DHCP_FUNCTION_END;

    return APPUTILR_OK;
}

INT32  misc_dhcp_msg_process(MISC_NOTIFY_MSG_T * t_notify_msg)
{
    MISC_DHCP_FUNCTION_BEGIN;

    INT32 i4_ret = 0;
    APPMSG_T t_misc_appmsg = {0};
    UINT8 ui1_netconnection;
    UINT8 ui_value;

    printf("<MISC_DHCP> misc_dhcp_msg_process :ui4_msg=%d \n",t_notify_msg->ui4_msg);

    switch (t_notify_msg->ui4_msg)
    {
        case NI_DRV_EV_ETHERNET_PLUGIN:
            {
                i4_ret = _misc_handle_dhcp_plugin_notify();
                if (APPUTILR_OK != i4_ret)
                {
                    printf("<MISC_DHCP>Update _misc_handle_dhcp_plugin_notify failed, code = %d\n", i4_ret);
                }
            }
            break;

        case NI_DRV_EV_ETHERNET_UNPLUG:
            {
                i4_ret = _misc_handle_dhcp_unplug_notify();
                if (APPUTILR_OK != i4_ret)
                {
                    printf("<MISC_DHCP>Update _misc_handle_dhcp_unplug_notify failed, code = %d\n", i4_ret);
                }
            }
            break;

        case NI_DRV_EV_IP_ADDRESS_CHANGED:
            {
                i4_ret = _misc_handle_dhcp_ipchange_notify();
                if (APPUTILR_OK != i4_ret)
                {
                    printf("<MISC_DHCP>Update _misc_handle_dhcp_ipchange_notify failed, code = %d\n", i4_ret);
                }
            }
            break;

        case NI_DRV_EV_IP_ADDRESS_EXPIRED:
            {
                i4_ret = _misc_handle_dhcp_expire_notify();
                if (APPUTILR_OK != i4_ret)
                {
                    printf("<MISC_DHCP>Update _misc_handle_dhcp_expire_notify failed, code = %d\n", i4_ret);
                }
            }
            break;

        default:
            break;
    }

#if 0//CONFIG_SUPPORT_NETWORK_AUTO_IP  alibaba need auto ip
    if (t_notify_msg->ui4_msg == AUTO_IP_EVENT_SUCCESS_DHCPv4)
    {
        printf("<DHCP_DHCP> misc_dhcp_msg_process:AUTO_IP_EVENT_SUCCESS_DHCPv4 \n");
        i4_ret = a_network_auto_ip_dhcpc_success_proc();
        if (APPUTILR_OK != i4_ret)
        {
            printf("<MISC_DHCP>Update a_network_auto_ip_dhcpc_success_proc failed, code = %d\n", i4_ret);
        }
    }
    else if (t_notify_msg->ui4_msg == AUTO_IP_EVENT_FAIL_DHCPv4)
    {
        printf("<DHCP_DHCP> misc_dhcp_msg_process:AUTO_IP_EVENT_FAIL_DHCPv4 \n");
        i4_ret = a_network_auto_ip_dhcpc_fail_proc();
        if (APPUTILR_OK != i4_ret)
        {
            printf("<DHCP_DHCP> Update a_network_auto_ip_dhcpc_fail_proc failed, code = %d\n", i4_ret);
        }
    }
#endif

    MISC_DHCP_FUNCTION_END;

    return APPUTILR_OK;
}

