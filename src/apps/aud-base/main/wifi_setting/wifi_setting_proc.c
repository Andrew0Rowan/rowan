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
 * $RCSfile: wifi_setting_proc.c $
 * $Revision:
 * $Date:
 * $Author:
 * $CCRevision: $
 * $SWAuthor: TOOL $
 * $MD5HEX: $
 *
 * Description:
 *---------------------------------------------------------------------------*/
/*-----------------------------------------------------------------------------
                    include files
-----------------------------------------------------------------------------*/
#include "u_common.h"
#include "u_dbg.h"
#include "u_os.h"
#include "u_am.h"
#include "wifi_setting.h"
#include "c_net_wlan.h"
#include "c_net_config.h"
#include "u_net_ni_if.h"
#include "u_acfg.h"
#include "u_dhcpc_api.h"
#include "u_timerd.h"
#include "u_misc.h"
#include "u_assistant_stub.h"
#include "u_app_thread.h"
#include "u_app_def.h"

#include "pthread.h"
#include "time.h"
#include "errno.h"

#include "iconv.h"  //econding format  GBK-UTF-8
#include "smt_api.h"

WIFI_SETTING_WLAN_STT_T wlan_scan_connect_status = {0};
SETUP_WLAN_FM_OWNER_T   setup_wlan_fm_owner = WLAN_FM_OWNER_WIRED_WIRELESS_SWITCH;
SETUP_WLAN_SETTING_STATUS_T setup_wlan_set_status = WLAN_SET_NOT_INITED;

static BOOL b_wlan_handle_scan_cmd = FALSE;
static BOOL fg_Need_Save_Ap_Info = FALSE;

extern WIFI_OBJ_T   t_g_this_obj;

static uint8_t g_key[16];
static uint8_t g_key_len = 0;
BOOL   g_check_callback=TRUE;//check callbakc is called

CHAR   wifi_ConnectSsid[NET_802_11_MAX_LEN_SSID+1]={0};
CHAR   wifi_ConnectBssid[NET_802_11_MAX_LEN_BSSID+1]={0};
CHAR   wifi_ConnectPwd[WLAN_KEY_MAX_LEN+1]={0};
INT32  wifi_AuthMode = -1;
BOOL   g_is_scan_enable = FALSE;
BOOL   g_is_upload_ap_list = FALSE;
INT32  g_scan_ap_id = 0;
INT32  fg_wifi_connect_id = 0;
BOOL   fg_wifi_connect_status = 0; /*0==disconnect   1==connect*/
BOOL   need_respose_wifi_connect_rpc = FALSE;
BOOL   fg_wifi_setup_status = FALSE; /*1 == setting , 0 == other*/
SCAN_AP_STAT_T  g_scan_ap_ok=SCAN_AP_STAT_NONE;//judge scan ap is ok
#if 0 /*wifi password error retry*/
int    associate_count = 0;/*retry 4 times*/
#endif

#if CONFIG_APP_SUPPORT_SMART_CONNECTION
BOOL            g_is_start_smart_connection = FALSE; //judge now has up softap
BOOL            g_smart_conncetion_thread_is_run = FALSE;
BOOL            fg_stop_smart_connection= FALSE;//stop smart connection
extern BOOL     SETUP_WIFI_SMART_CONNECTION_IS_ON;
CONNECT_AP_INFO g_ap_info = {{0}};
#endif

// wifi
#define WLAN_TIMER_SCAN_TIMEOUT                  16
#define WLAN_TIMER_ASSOCIATE_TIMEOUT             32 //For bug: BDP00079087.
#define WLAN_TIMER_PBC_TIMEOUT                   120
#define WLAN_TIMER_RESET                         0

#define WLAN_AUTO_PIN_LEN                        16

#define SETUP_WLAN_SCAN_RESULT_SHOW_NUM          160

#define SETUP_WIFI_SCAN_TEXT_LENGTH    ((UINT32)33)

//WiFi key
#define WIFI_AUTH_MODE_WEP_KEY_LEN_10_HEX             10
#define WIFI_AUTH_MODE_WEP_KEY_LEN_5_ASCII            5
#define WIFI_AUTH_MODE_WEP_KEY_LEN_26_HEX             26
#define WIFI_AUTH_MODE_WEP_KEY_LEN_13_ASCII           13
#define WIFI_AUTH_MODE_WAP_KEY_LEN_MIN                8
#define WIFI_AUTH_MODE_WAP_KEY_LEN_MAX                64
#define WIFI_AUTH_MODE_WAP_KEY_LEN_64_HEX             64

static UINT8 _setup_ap_signal_level(UINT16 ui2_level)
{
    if(ui2_level > 61) //>= 58dB
    {
        return 5;
    }
    else if(ui2_level > 53) // -59dB ~ 66dB
    {
        return 4;
    }
    else if(ui2_level > 47)//-67dB ~-73dB
    {
        return 3;
    }
    else if(ui2_level >= 39)// -64~-80dB
    {
        return 2;
    }
    else // -81dB <
    {
        return 1;
    }

}

static void upload_scan_ap_fail(void) {
    HANDLE_T h_app = NULL_HANDLE;
    ASSISTANT_STUB_WIFI_CONNECT_RESPONSE_T wifi_status ={0};

    memcpy(wifi_status.command, STRING_WIFI_CONNECT, ASSISTANT_STUB_COMMAND_MAX_LENGTH);
    wifi_status.wifi_status = SCAN_AP_FAIL;
    wifi_status.id = fg_wifi_connect_id;

    INT32 i4_ret = u_am_get_app_handle_from_name(&h_app, ASSISTANT_STUB_THREAD_NAME);
    if(0 != i4_ret)
    {
       WIFI_MSG("<WIFI_SETTING_PROC> get handle fail!,i4_ret=%ld\n",i4_ret);
    }

    i4_ret = u_app_send_appmsg(h_app,
                              E_APP_MSG_TYPE_ASSISTANT_STUB,
                              MSG_FROM_WIFI_SETTING,
                              ASSISTANT_STUB_CMD_WIFI_CONNECT,
                              &wifi_status,
                              sizeof(wifi_status));
    if(0 == i4_ret)
    {
       WIFI_MSG("<WIFI_SETTING_PROC> u_app_send_appmsg success !!!\n");
    }
    else
    {
       WIFI_MSG("<WIFI_SETTING_PROC> u_app_send_appmsg fail !!!\n");
    }
}

static BOOL _setup_ap_greater(UINT16 ui2_idx_1, UINT16 ui2_idx_2)
{
    UINT8 ui1_level1 = 0;
    UINT8 ui1_level2 = 0;

    if ((ui2_idx_1 >= SETUP_WLAN_SCAN_RESULT_SHOW_NUM) || (ui2_idx_2 >= SETUP_WLAN_SCAN_RESULT_SHOW_NUM))
    {
        return FALSE;
    }

    ui1_level1 = _setup_ap_signal_level(wlan_scan_connect_status.cur_show_result[ui2_idx_1].i2_Level);
    ui1_level2 = _setup_ap_signal_level(wlan_scan_connect_status.cur_show_result[ui2_idx_2].i2_Level);

    //first compare the level, acd
    if (ui1_level1 > ui1_level2)
    {
        return TRUE;
    }
    else if (ui1_level1 == ui1_level2) //secondary compare the name, acd
    {

        if (strcmp(wlan_scan_connect_status.cur_show_result[ui2_idx_1].t_Ssid.ui1_aSsid,
                     wlan_scan_connect_status.cur_show_result[ui2_idx_2].t_Ssid.ui1_aSsid) < 0)
        {
            return TRUE;
        }
        else
        {
            return FALSE;
        }
    }
    else
    {
        return FALSE;
    }

    return FALSE;
}

INT32 a_network_get_favorite_ap(NET_802_11_ASSOCIATE_T *_rSavedWlanAssociate)
{
    WIFI_SETTING_PROC_FUNCTION_BEGIN;

    INT32 i4_ret = WIFI_OK;
    i4_ret = a_cfg_get_wlan_ap(_rSavedWlanAssociate);
    WIFI_MSG("<WIFI_SETTING_PROC> get saved last WiFi information\n");

    if( _rSavedWlanAssociate->t_Bssid[0]==0 &&
        _rSavedWlanAssociate->t_Bssid[1]==0 &&
        _rSavedWlanAssociate->t_Bssid[2]==0 &&
        _rSavedWlanAssociate->t_Bssid[3]==0 &&
        _rSavedWlanAssociate->t_Bssid[4]==0 &&
        _rSavedWlanAssociate->t_Bssid[5]==0    )
    {
        WIFI_MSG("<WIFI_SETTING_PROC> No Last save information exist,return WIFI_FAIL !\n");
        WIFI_SETTING_PROC_FUNCTION_END;

        return WIFI_FAIL;
    }

    WIFI_SETTING_PROC_FUNCTION_END;
    return i4_ret;
}

INT32 setup_wlan_show_scan_result(UINT16 show_total_num)
{
    WIFI_SETTING_PROC_FUNCTION_BEGIN;

    INT32       i4_ret=0;//, i4_count;
    UINT16       i4_num, total_num,i4_next_num;
   // UTF16_T     persent_signal[] = {0x25};  //"%"

    NET_802_11_BSS_INFO_T   temp_item;

    total_num = show_total_num;

    wlan_scan_connect_status.ui1_wlan_total_num = show_total_num;

    WIFI_MSG("<WIFI_SETTING_PROC> setup_wlan_show_scan_result : listbox total items(%d).\n", total_num);

    setup_wlan_fm_owner = WLAN_FM_OWNER_SHOW_SCAN_LIST;

    for(i4_num=0; i4_num < total_num; i4_num++)
    {
        for(i4_next_num=i4_num+1; i4_next_num < total_num; i4_next_num++)
        {
            if((i4_num >= SETUP_WLAN_SCAN_RESULT_SHOW_NUM)||
                (i4_next_num >= SETUP_WLAN_SCAN_RESULT_SHOW_NUM))
            {
                WIFI_MSG("<WIFI_SETTING_PROC> < i4_num || i4_next_num >160   error return \n");
                return WIFI_FAIL;
            }

            //if(wlan_scan_connect_status.cur_show_result[i4_num].i2_Level < wlan_scan_connect_status.cur_show_result[i4_next_num].i2_Level)
            if (_setup_ap_greater(i4_next_num, i4_num))
            {
                temp_item =wlan_scan_connect_status.cur_show_result[i4_num];
                wlan_scan_connect_status.cur_show_result[i4_num]=wlan_scan_connect_status.cur_show_result[i4_next_num];
                wlan_scan_connect_status.cur_show_result[i4_next_num]=temp_item;
            }
        }
    }

    WIFI_SETTING_PROC_FUNCTION_END;

    return i4_ret;
}

INT32 _setup_wlan_associate_start_timer(VOID)
{
    WIFI_SETTING_PROC_FUNCTION_BEGIN;

    INT32 i4_ret = 0;

    if (NULL_HANDLE != t_g_this_obj.t_associate_timer.h_timer)
    {
        //u_timer_stop(h_timer_wlan);
        u_timer_stop(t_g_this_obj.t_associate_timer.h_timer);
    }

#if 0
    i4_ret = u_timer_start(h_timer_wlan,
                           1000,     //2s
                           X_TIMER_FLAG_REPEAT,
                           _setup_wlan_associate_timer_cb,
                           NULL);
#else
    WIFI_SETTING_TIMER_MSG_E e_associate_msg=TIMER_MSG_WIFI_ASSCOCIATE;
    t_g_this_obj.t_associate_timer.e_flags=X_TIMER_FLAG_REPEAT;
    t_g_this_obj.t_associate_timer.ui4_delay=1000;

    i4_ret = u_timer_start(t_g_this_obj.h_app,
                           &t_g_this_obj.t_associate_timer,
                           (void *)&e_associate_msg,
                           sizeof(WIFI_SETTING_TIMER_MSG_E));
#endif

    WIFI_SETTING_PROC_FUNCTION_END;

    return WIFI_OK;
}

INT32 setup_save_wlan_key(VOID)
{
    WIFI_SETTING_PROC_FUNCTION_BEGIN;

    INT32           i4_ret = WIFI_OK;
    NET_802_11_BSS_INFO_T strCurrBss;
    CHAR        bssid_dest_str[32] = {0};
    INT32       i4_idx = 0;

    memset(&strCurrBss, 0, sizeof(NET_802_11_BSS_INFO_T));
    i4_ret = c_net_wlan_get_curr_bss(&strCurrBss);
    if (i4_ret != 0)
    {
        WIFI_MSG("<WIFI_SETTING_PROC> ERR: c_net_wlan_get_curr_bss failed i4_ret=%d \n",i4_ret);
        wlan_scan_connect_status.ui1_ap_count = 1;
        return WIFI_OK;

    }
    snprintf(bssid_dest_str,sizeof(bssid_dest_str), "%2x:%2x:%2x:%2x:%2x:%2x", strCurrBss.t_Bssid[0],strCurrBss.t_Bssid[1],strCurrBss.t_Bssid[2],strCurrBss.t_Bssid[3],strCurrBss.t_Bssid[4],strCurrBss.t_Bssid[5]);

    WIFI_MSG(" Get strCurrBss information:\n");
    WIFI_MSG(" >>>>>>Bssid:%s\n", bssid_dest_str);
    WIFI_MSG(" >>>>>>SSID:%s\n",  strCurrBss.t_Ssid.ui1_aSsid);
    WIFI_MSG(" >>>>>>i2_Level:%d\n",  strCurrBss.i2_Level);
    WIFI_MSG(" >>>>>>i2_Channel:%d\n",  strCurrBss.i2_Channel);
    WIFI_MSG(" >>>>>>e_AuthMode:%d\n",  strCurrBss.e_AuthMode);
    WIFI_MSG(" >>>>>>e_AuthCipher:%d\n",  strCurrBss.e_AuthCipher);
    WIFI_MSG(" >>>>>>i4_MaxRate:%d\n",  strCurrBss.i4_MaxRate);
    WIFI_MSG(" >>>>>>i4_SSID Len:%d\n",   strCurrBss.t_Ssid.ui4_SsidLen);
    WIFI_MSG(" >>>>>>e_802_11_type:%d\n",   strCurrBss.e_802_11_type);
    WIFI_MSG(" >>>>>>is_wps_support:%d\n",   strCurrBss.is_wps_support);
    WIFI_MSG(" >>>>>>bandwitdh is:%d\n",   strCurrBss.e_802_11_bw);

    for (i4_idx = 0; i4_idx < wlan_scan_connect_status.ui1_ap_count; i4_idx++)
    {
        if (0 == strcmp(strCurrBss.t_Ssid.ui1_aSsid,wlan_scan_connect_status._rCurWlanAssociate[i4_idx].t_Ssid.ui1_aSsid))
        {
            WIFI_MSG("<WIFI_SETTING_PROC> setup_save_wlan_key  SSID match: @L%d\n", __LINE__);
            WIFI_MSG("<WIFI_SETTING_PROC> Get index is[%d]: @%d\n", i4_idx, __LINE__);

            WIFI_MSG("<WIFI_SETTING_PROC> GOOD find the ssid !!!\n");
            wlan_scan_connect_status.ui1_ap_count = 1;
            INT32 i=0;
            NET_802_11_ASSOCIATE_T _rSaveWlanAssociate={{0}};
            memcpy(_rSaveWlanAssociate.t_Ssid.ui1_aSsid,wifi_ConnectSsid,NET_802_11_MAX_LEN_SSID+1);
            //memcpy(_rSaveWlanAssociate.t_Key.pui1_PassPhrase,wifi_ConnectPwd,WLAN_KEY_MAX_LEN+1);
            _rSaveWlanAssociate.e_AuthMode=strCurrBss.e_AuthMode;

            _rSaveWlanAssociate.t_Ssid.ui4_SsidLen=strCurrBss.t_Ssid.ui4_SsidLen;
            _rSaveWlanAssociate.e_AuthCipher=strCurrBss.e_AuthCipher;
            for(i=0;i<6;i++)
            {
                memcpy(&(_rSaveWlanAssociate.t_Bssid[i]),&(strCurrBss.t_Bssid[i]),sizeof(UINT8));
                WIFI_MSG("<WIFI_SETTING_PROC> _rSaveWlanAssociate.t_Bssid[%d] is %2x \n",i,_rSaveWlanAssociate.t_Bssid[i]);
            }

#ifndef CONFIG_ADAPTOR_APP_CTRL
            i4_ret = a_cfg_set_wlan_ap(_rSaveWlanAssociate);
#endif /* CONFIG_ADAPTOR_APP_CTRL */
            fg_Need_Save_Ap_Info=FALSE;

            if (NULL != wifi_ConnectPwd)
            {
#ifndef CONFIG_ADAPTOR_APP_CTRL
                i4_ret = a_cfg_set_wlan_key(wifi_ConnectPwd);
#endif /* CONFIG_ADAPTOR_APP_CTRL */
            }

            WIFI_SETTING_PROC_FUNCTION_END;

            return WIFI_OK;

        }

    }

    WIFI_SETTING_PROC_FUNCTION_END;

    return WIFI_FAIL;
}

static WIFI_AUTH_MODE_E transfer_supplicant_to_auth_mode(NET_802_11_AUTH_MODE_T e_AuthMode) {
    if (IEEE_802_11_AUTH_MODE_NONE == e_AuthMode){
        return AUTH_MODE_NONE;
    } else if ((e_AuthMode >= IEEE_802_11_AUTH_MODE_OPEN) && (e_AuthMode <= IEEE_802_11_AUTH_MODE_WEPAUTO)) {
        return AUTH_MODE_WEP;
    } else {
        return AUTH_MODE_WPA;
    }
}

static void fill_ap_info(NET_802_11_BSS_INFO_T *scan_info, ASSISTANT_STUB_AP_INFO_T *ap_info) {
    //ssid
    strncpy(ap_info->ssid, scan_info->t_Ssid.ui1_aSsid, ASSISTANT_STUB_SSID_MAX_LENGTH);

    //bssid
    snprintf(ap_info->bssid, ASSISTANT_STUB_BSSID_MAX_LENGTH, "%x:%x:%x:%x:%x:%x", scan_info->t_Bssid[0], scan_info->t_Bssid[1],
                            scan_info->t_Bssid[2], scan_info->t_Bssid[3], scan_info->t_Bssid[4], scan_info->t_Bssid[5]);

    //authmode
    ap_info->auth_mode = transfer_supplicant_to_auth_mode(scan_info->e_AuthMode);

    //level
    ap_info->level = scan_info->i2_Level_dB;

    //frequency
    ap_info->frequency = scan_info->i2_Freq;
}

static void fill_ap_list(NET_802_11_BSS_INFO_T *scan_info, ASSISTANT_STUB_AP_INFO_T *ap_info, int num) {
    for (INT32 i=0; i < num; i++) {
        fill_ap_info(scan_info+i, ap_info+i);
    }
}

static void send_ap_list_to_stub(ASSISTANT_STUB_GET_AP_LIST_RESPONSE_T *ap_list) {
    HANDLE_T h_app = NULL_HANDLE;
    INT32 i4_ret = u_am_get_app_handle_from_name(&h_app, ASSISTANT_STUB_THREAD_NAME);
    if(0 != i4_ret)
    {
       WIFI_MSG("<WIFI_SETTING_PROC> get handle fail!,i4_ret=%ld\n",i4_ret);
    }

    i4_ret = u_app_send_appmsg(h_app,
                              E_APP_MSG_TYPE_ASSISTANT_STUB,
                              MSG_FROM_WIFI_SETTING,
                              ASSISTANT_STUB_CMD_GET_AP_LIST,
                              ap_list,
                              sizeof(*ap_list));
    if(0 == i4_ret)
    {
       WIFI_MSG("send_ap_list_to_stub success!\n");
    }
    else
    {
       WIFI_MSG("send_ap_list_to_stub fail:[%d]!\n", i4_ret);
    }
}

static void upload_ap_list(NET_802_11_SCAN_RESULT_T *scan_list) {
    INT32 i = 0;
    INT32 total_num = scan_list->ui4_NumberOfItems;
    INT32 times = total_num / ASSISTANT_STUB_AP_LIST_MAX;
    NET_802_11_BSS_INFO_T *scan_result = scan_list->p_BssInfo;
    ASSISTANT_STUB_GET_AP_LIST_RESPONSE_T ap_list = {0};

    strncpy(ap_list.command, STRING_GET_AP_LIST, ASSISTANT_STUB_COMMAND_MAX_LENGTH);
    ap_list.id = g_scan_ap_id;

    do {
        ap_list.list_num = ASSISTANT_STUB_AP_LIST_MAX;
        if (i == times) {
            ap_list.list_num = total_num % ASSISTANT_STUB_AP_LIST_MAX;
            ap_list.finish = TRUE;
        }
        fill_ap_list(scan_result + i*ASSISTANT_STUB_AP_LIST_MAX, ap_list.ap_info, ap_list.list_num);
        send_ap_list_to_stub(&ap_list);
    } while (i++ < times);
}

static void scan_notify_process(void) {
    int i4_ret=0;
    int i4_num=0;
    int i4_total_num=0;
    NET_802_11_SCAN_RESULT_T scan_list;
    NET_802_11_BSS_INFO_T *scan_cur_result;
    CHAR  dest_str[NET_802_11_MAX_LEN_SSID+1] = {0};
    INT32 ui4_ssid_len = 0;

    g_is_scan_enable = FALSE;

    g_scan_ap_ok = SCAN_AP_STAT_FAILE;

    i4_ret = c_net_wlan_get_scan_result(&scan_list);
    if (i4_ret != WIFI_OK)
    {
        WIFI_MSG("<SMART CONNECTION>[c_net_wlan_get_scan_result]failed %d %d\n",i4_ret, __LINE__);
        goto EXIT;
    }

    i4_total_num = scan_list.ui4_NumberOfItems;
    scan_cur_result = scan_list.p_BssInfo;

    WIFI_MSG("<SMART CONNECTION>:WLAN_NOTIFY_SCAN total_num %d %d\n",i4_total_num,__LINE__);

    if (0 == i4_total_num)
    {
        WIFI_MSG("<SMART CONNECTION> no scan result, may be hidden ssid\n");
        goto EXIT;
    }

    for(i4_num = 0; i4_num < i4_total_num; i4_num++)
    {
        memset(dest_str, 0,NET_802_11_MAX_LEN_SSID+1);
        ui4_ssid_len = scan_cur_result[i4_num].t_Ssid.ui4_SsidLen;
        WIFI_MSG("<SMART CONNECTION>------------------------------------\n");
        WIFI_MSG("<SMART CONNECTION>show ssid or bssid ui4_ssid_len is %d\n",ui4_ssid_len);

        if(ui4_ssid_len>0)
        {
            strncpy(dest_str,
                    scan_cur_result[i4_num].t_Ssid.ui1_aSsid,
                    NET_802_11_MAX_LEN_SSID+1);
                    dest_str[NET_802_11_MAX_LEN_SSID]='\0';
        }

        WIFI_MSG("<SMART CONNECTION>scan_cur_result[i4_num].t_Ssid.ui1_aSsid is %s\n",scan_cur_result[i4_num].t_Ssid.ui1_aSsid);

        char ssid_gbk[NET_802_11_MAX_LEN_SSID + 1];
        char ssid_hex[NET_802_11_MAX_LEN_SCAN_SSID + 1];
        memset(ssid_gbk, 0, sizeof(ssid_gbk));
        memset(ssid_hex, 0, sizeof(ssid_hex));
        convert_encoding_format(wifi_ConnectSsid, ssid_gbk); //convert ssid_utf-8 from elian to GBK
        WIFI_MSG("<SMART CONNECTION>ssid_gbk:%s\n", ssid_gbk);
        str_to_strhex(ssid_gbk,ssid_hex,strlen(ssid_gbk));
        WIFI_MSG("<SMART CONNECTION>ssid_hex:%s strlen(ssid_hex):%d\n", ssid_hex, strlen(ssid_hex));
        
        
        char dest_ssid_hex[NET_802_11_MAX_LEN_SCAN_SSID + 1];
        memset(dest_ssid_hex, 0, sizeof(dest_ssid_hex));
        str_to_strhex(dest_str, dest_ssid_hex, strlen(dest_str));
        WIFI_MSG("<SMART CONNECTION>dest_str:%s strlen(dest_str):%d\n", dest_str, strlen(dest_str));
        WIFI_MSG("<SMART CONNECTION>dest_ssid_hex:%s strlen(dest_ssid_hex):%d\n", dest_ssid_hex, strlen(dest_ssid_hex));    

        if ((strlen(ssid_hex)==strlen(dest_ssid_hex))
              &&(0 == strncmp(ssid_hex, dest_ssid_hex,strlen(ssid_hex))))
        {
            memset(&wlan_scan_connect_status.cur_show_result[0],0,sizeof(NET_802_11_BSS_INFO_T));

            wlan_scan_connect_status.ui1_wlan_total_num=1;
            //t_Bssid
            wlan_scan_connect_status.cur_show_result[0].t_Bssid[0]=scan_cur_result[i4_num].t_Bssid[0];
            wlan_scan_connect_status.cur_show_result[0].t_Bssid[1]=scan_cur_result[i4_num].t_Bssid[1];
            wlan_scan_connect_status.cur_show_result[0].t_Bssid[2]=scan_cur_result[i4_num].t_Bssid[2];
            wlan_scan_connect_status.cur_show_result[0].t_Bssid[3]=scan_cur_result[i4_num].t_Bssid[3];
            wlan_scan_connect_status.cur_show_result[0].t_Bssid[4]=scan_cur_result[i4_num].t_Bssid[4];
            wlan_scan_connect_status.cur_show_result[0].t_Bssid[5]=scan_cur_result[i4_num].t_Bssid[5];
            //t_Ssid
            strncpy(wlan_scan_connect_status.cur_show_result[0].t_Ssid.ui1_aSsid,scan_cur_result[i4_num].t_Ssid.ui1_aSsid,scan_cur_result[i4_num].t_Ssid.ui4_SsidLen);
            wlan_scan_connect_status.cur_show_result[0].t_Ssid.ui4_SsidLen = scan_cur_result[i4_num].t_Ssid.ui4_SsidLen;
            //smart connection  ap store
            //memset(g_ap_info.ap_name,0,NET_802_11_MAX_LEN_SSID+1);
            //strncpy(g_ap_info.ap_name,scan_cur_result[i4_num].t_Ssid.ui1_aSsid,NET_802_11_MAX_LEN_SSID+1);
            //g_ap_info.ap_name[NET_802_11_MAX_LEN_SSID]='\0';
            //authmode
            //g_ap_info.ap_authmode=scan_cur_result[i4_num].e_AuthMode;
            //channel
            wlan_scan_connect_status.cur_show_result[0].i2_Channel=scan_cur_result[i4_num].i2_Channel;
            //e_AuthMode
            wlan_scan_connect_status.cur_show_result[0].e_AuthMode=scan_cur_result[i4_num].e_AuthMode;
            //e_AuthCipher
            wlan_scan_connect_status.cur_show_result[0].e_AuthCipher=scan_cur_result[i4_num].e_AuthCipher;
            //i2_Freq
            wlan_scan_connect_status.cur_show_result[0].i2_Freq=scan_cur_result[i4_num].i2_Freq;

            g_scan_ap_ok = SCAN_AP_STAT_OK;
            WIFI_MSG("<SMART CONNECTION>SCAN AP IS OK,ap_name is %s !!!!\n",scan_cur_result[i4_num].t_Ssid.ui1_aSsid);
            goto EXIT;
        }
    }

EXIT:
    if (g_is_upload_ap_list) {
        upload_ap_list(&scan_list);
        g_is_upload_ap_list = FALSE;
    }

    pthread_mutex_lock(&t_g_this_obj.t_mutex);
    pthread_cond_broadcast(&t_g_this_obj.t_cond);
    pthread_mutex_unlock(&t_g_this_obj.t_mutex);
}

static INT32 _wifi_setting_wlan_notify_callback(INT32 i4NotifykId, VOID *pParam)
{
    WIFI_SETTING_PROC_FUNCTION_BEGIN;

    WIFI_MSG_T t_msg;
    INT32 *pi4_cb_result;
    INT32 i4_ret;
    NET_802_11_ASSOCIATE_T *pst_cb_result;
    CHAR  bssid_dest_str[32] = {0};

    WIFI_MSG("<WIFI_SETTING_PROC> _wifi_setting_wlan_notify_callback :ui1_wlan_notify_type %d\n", wlan_scan_connect_status.ui1_wlan_notify_type);

    switch (i4NotifykId)
    {
        case WLAN_NOTIFY_INIT_IND:
            WIFI_MSG("<WIFI_SETTING_PROC>  WLAN_NOTIFY_INIT_IND \n");
            pi4_cb_result    = (INT32 *)pParam;
            t_msg.ui4_msg_id = WIFI_WLAN_MSG;
            t_msg.ui4_data1  = WLAN_NOTIFY_INIT_IND;
            t_msg.ui4_data2  = *pi4_cb_result;
            t_msg.ui4_data3  = (UINT32)NULL;
            break;

        case WLAN_NOTIFY_DEINIT_IND:
            WIFI_MSG("<WIFI_SETTING_PROC>  WLAN_NOTIFY_DEINIT_IND \n");
            pi4_cb_result    = (INT32 *)pParam;
            t_msg.ui4_msg_id = WIFI_WLAN_MSG;
            t_msg.ui4_data1  = WLAN_NOTIFY_DEINIT_IND;
            t_msg.ui4_data2  = *pi4_cb_result;
            t_msg.ui4_data3  = (UINT32)NULL;
            break;

        case WLAN_NOTIFY_START_IND:
            WIFI_MSG("<WIFI_SETTING_PROC>  WLAN_NOTIFY_START_IND \n");
            pi4_cb_result    = (INT32 *)pParam;
            t_msg.ui4_msg_id = WIFI_WLAN_MSG;
            t_msg.ui4_data1  = WLAN_NOTIFY_START_IND;
            t_msg.ui4_data2  = *pi4_cb_result;
            t_msg.ui4_data3  = (UINT32)NULL;
            break;

        case WLAN_NOTIFY_STOP_IND:
            WIFI_MSG("<WIFI_SETTING_PROC>  WLAN_NOTIFY_STOP_IND \n");
            pi4_cb_result    = (INT32 *)pParam;
            t_msg.ui4_msg_id = WIFI_WLAN_MSG;
            t_msg.ui4_data1  = WLAN_NOTIFY_STOP_IND;
            t_msg.ui4_data2  = *pi4_cb_result;
            t_msg.ui4_data3  = (UINT32)NULL;
            break;

        case WLAN_NOTIFY_SCAN:
            WIFI_MSG("WLAN_NOTIFY_SCAN\n");
            scan_notify_process();
            return;

        case WLAN_NOTIFY_ASSOCIATE:
            WIFI_MSG("WLAN_NOTIFY_ASSOCIATE\n");
            pi4_cb_result = (INT32 *)pParam;
            t_msg.ui4_msg_id = WIFI_WLAN_MSG;
            t_msg.ui4_data1 = WLAN_NOTIFY_ASSOCIATE;
            t_msg.ui4_data2 = *pi4_cb_result;//manual_result;
            t_msg.ui4_data3 = (UINT32)NULL;
            break;

        case WLAN_NOTIFY_WPS_ASSOC_SETTING:
            WIFI_MSG("WLAN_NOTIFY_WPS_ASSOC_SETTING\n");
            pst_cb_result = (NET_802_11_ASSOCIATE_T *)pParam;
            t_msg.ui4_msg_id = WIFI_WLAN_MSG;
            t_msg.ui4_data1 = WLAN_NOTIFY_WPS_ASSOC_SETTING;
            t_msg.ui4_data2 = pst_cb_result->t_Ssid.ui4_SsidLen;
            t_msg.ui4_data3 = (UINT32)NULL;

            if (wlan_scan_connect_status.ui1_ap_count < 3) //the buffer only 3
            {
                WIFI_MSG("<WIFI_SETTING_PROC> From Notification: KEY:%s\n, key body:%s\n IsAscii:%d\n",
                       pst_cb_result->t_Key.pui1_PassPhrase,
                       pst_cb_result->t_Key.key_body,
                       pst_cb_result->t_Key.b_IsAscii);

                strncpy(wifi_ConnectSsid,
                        pst_cb_result->t_Ssid.ui1_aSsid,
                        NET_802_11_MAX_LEN_SSID+1);
                wifi_ConnectSsid[NET_802_11_MAX_LEN_SSID]='\0';

                strncpy(wifi_ConnectPwd,
                        pst_cb_result->t_Key.pui1_PassPhrase,
                        WLAN_KEY_MAX_LEN+1);
                wifi_ConnectPwd[WLAN_KEY_MAX_LEN] = '\0';

                memcpy(&wlan_scan_connect_status._rCurWlanAssociate[wlan_scan_connect_status.ui1_ap_count], pst_cb_result, sizeof(NET_802_11_ASSOCIATE_T));
                WIFI_MSG("<WIFI_SETTING_PROC> [WPS] WLAN_NOTIFY_WPS_ASSOC_SETTING start\n");

                snprintf(bssid_dest_str,sizeof(bssid_dest_str), "%2x:%2x:%2x:%2x:%2x:%2x",
                    wlan_scan_connect_status._rCurWlanAssociate[wlan_scan_connect_status.ui1_ap_count].t_Bssid[0],
                    wlan_scan_connect_status._rCurWlanAssociate[wlan_scan_connect_status.ui1_ap_count].t_Bssid[1],
                    wlan_scan_connect_status._rCurWlanAssociate[wlan_scan_connect_status.ui1_ap_count].t_Bssid[2],
                    wlan_scan_connect_status._rCurWlanAssociate[wlan_scan_connect_status.ui1_ap_count].t_Bssid[3],
                    wlan_scan_connect_status._rCurWlanAssociate[wlan_scan_connect_status.ui1_ap_count].t_Bssid[4],
                    wlan_scan_connect_status._rCurWlanAssociate[wlan_scan_connect_status.ui1_ap_count].t_Bssid[5]);

                WIFI_MSG("<WIFI_SETTING_PROC> SSID:%s, SsidLen:%d, eAssocCase:%d, eAuthMode:%d, e_AuthCipher:%d Priority:%d \n",
                    wlan_scan_connect_status._rCurWlanAssociate[wlan_scan_connect_status.ui1_ap_count].t_Ssid.ui1_aSsid,
                    wlan_scan_connect_status._rCurWlanAssociate[wlan_scan_connect_status.ui1_ap_count].t_Ssid.ui4_SsidLen,
                    wlan_scan_connect_status._rCurWlanAssociate[wlan_scan_connect_status.ui1_ap_count].e_AssocCase,
                    wlan_scan_connect_status._rCurWlanAssociate[wlan_scan_connect_status.ui1_ap_count].e_AuthMode,
                    wlan_scan_connect_status._rCurWlanAssociate[wlan_scan_connect_status.ui1_ap_count].e_AuthCipher,
                    wlan_scan_connect_status._rCurWlanAssociate[wlan_scan_connect_status.ui1_ap_count].ui1_Priority);

                WIFI_MSG("<WIFI_SETTING_PROC> KEY:%s\n, key body:%s\n IsAscii:%d\n",
                    wlan_scan_connect_status._rCurWlanAssociate[wlan_scan_connect_status.ui1_ap_count].t_Key.pui1_PassPhrase,
                    wlan_scan_connect_status._rCurWlanAssociate[wlan_scan_connect_status.ui1_ap_count].t_Key.key_body,
                    wlan_scan_connect_status._rCurWlanAssociate[wlan_scan_connect_status.ui1_ap_count].t_Key.b_IsAscii);

                //mtk70255 2011.11.16 fix BD00091922
                memset(wlan_scan_connect_status.wlan_wpa_wep[wlan_scan_connect_status.ui1_ap_count], 0, sizeof(wlan_scan_connect_status.wlan_wpa_wep[wlan_scan_connect_status.ui1_ap_count]));
                strncpy(wlan_scan_connect_status.wlan_wpa_wep[wlan_scan_connect_status.ui1_ap_count],wlan_scan_connect_status._rCurWlanAssociate[wlan_scan_connect_status.ui1_ap_count].t_Key.key_body,sizeof(wlan_scan_connect_status.wlan_wpa_wep[wlan_scan_connect_status.ui1_ap_count]));

                wlan_scan_connect_status._rCurWlanAssociate[wlan_scan_connect_status.ui1_ap_count].t_Key.pui1_PassPhrase = wlan_scan_connect_status.wlan_wpa_wep[wlan_scan_connect_status.ui1_ap_count];
                wlan_scan_connect_status.ui1_ap_count++;

            }
            break;

        case WLAN_NOTIFY_WPS_STATE:
            WIFI_MSG("WLAN_NOTIFY_WPS_STATE\n");
            pi4_cb_result    = (INT32 *)pParam;
            t_msg.ui4_msg_id = WIFI_WLAN_MSG;
            t_msg.ui4_data1  = WLAN_NOTIFY_WPS_STATE;
            t_msg.ui4_data2  = *pi4_cb_result;
            t_msg.ui4_data3  = (UINT32)NULL;
            break;

        case WLAN_NOTIFY_WPS_ASSOCIATE:
            WIFI_MSG("WLAN_NOTIFY_WPS_ASSOCIATE\n");
            pi4_cb_result    = (INT32 *)pParam;
            t_msg.ui4_msg_id = WIFI_WLAN_MSG;
            t_msg.ui4_data1  = WLAN_NOTIFY_WPS_ASSOCIATE;
            t_msg.ui4_data2  = *pi4_cb_result;// scan_result;
            t_msg.ui4_data3  = (UINT32)NULL;
            break;

        default:
            WIFI_MSG("i4NotifykId= %d\n", i4NotifykId);
            break;
     }

    WIFI_MSG("<WIFI_SETTING_PROC> wifi_send_msg>>> SETUP_WLAN_MSG >>> i4NotifykId=%d\n",i4NotifykId);

    i4_ret = wifi_send_msg(&t_msg);
    if (WIFI_OK != i4_ret)
    {
        WIFI_MSG("wifi_send_msg failed %d\n", i4_ret);
    }

    WIFI_SETTING_PROC_FUNCTION_END;
    return i4_ret;
}

static INT32 get_ap_info_from_scan_result(const CHAR *ssid, NET_802_11_ASSOCIATE_T *_rWlanAssociate) {
    INT32 i4_num = 0;
    CHAR *dest_ssid;

    for (i4_num=0; i4_num < wlan_scan_connect_status.ui1_wlan_total_num; i4_num++)
    {
        dest_ssid = wlan_scan_connect_status.cur_show_result[i4_num].t_Ssid.ui1_aSsid;
        WIFI_MSG("ssid is [%s], dest_ssid is [%s]\n",ssid, dest_ssid);

        if (0 == strncmp(ssid, dest_ssid, strlen(ssid)))
        {
            WIFI_MSG("Found the AP!\n");
            WIFI_MSG("=====SSID====%s!\n", dest_ssid);
            WIFI_MSG("=====AUTHMode====%d!\n", wlan_scan_connect_status.cur_show_result[i4_num].e_AuthMode);
            WIFI_MSG("=====AUTHCiper====%d!\n", wlan_scan_connect_status.cur_show_result[i4_num].e_AuthCipher);
            WIFI_MSG("=====SCAN_FREQ====%d!\n",wlan_scan_connect_status.cur_show_result[i4_num].i2_Freq);

            _rWlanAssociate->e_AuthMode   = wlan_scan_connect_status.cur_show_result[i4_num].e_AuthMode;
            _rWlanAssociate->e_AuthCipher = wlan_scan_connect_status.cur_show_result[i4_num].e_AuthCipher;
            //_rWlanAssociate.scan_freq=wlan_scan_connect_status.cur_show_result[i4_num].i2_Freq;
            break;
        }
    }

    if (i4_num == wlan_scan_connect_status.ui1_wlan_total_num)
    {
        WIFI_ERR("get ap info failed!\n");
        return WIFI_FAIL;
    }
    return WIFI_OK;
}

INT32 setup_goto_manual_associate(const CHAR *ssid, const CHAR *bssid, const CHAR* password, NET_802_11_AUTH_MODE_T authmode)
{
    WIFI_SETTING_PROC_FUNCTION_BEGIN;

    UINT8   ui_value;
    INT32   i4_ret = WIFI_OK;
    HANDLE_T h_app = NULL_HANDLE;
    UINT32  ui4_length = 0;
    UINT32 ui4Idx = 0;
    BOOL fgPwdIsAscii = FALSE;
    BOOL fgPwdIsInvalid = FALSE;
    NET_802_11_ASSOCIATE_T _rWlanAssociate = {0};

    if (-1 == authmode) {
        i4_ret = get_ap_info_from_scan_result(ssid, &_rWlanAssociate);
        if (i4_ret) {
            upload_scan_ap_fail();
            return WIFI_FAIL;
        }
    } else {
        _rWlanAssociate.e_AuthMode = authmode;
    }

    char ssid_gbk[NET_802_11_MAX_LEN_SSID + 1];
    memset(ssid_gbk, 0, sizeof(ssid_gbk));
    convert_encoding_format(ssid, ssid_gbk);
    WIFI_MSG("<WIFI_SETTING_PROC>ssid:%s strlen(ssid):%d\n", ssid, strlen(ssid));
    WIFI_MSG("<WIFI_SETTING_PROC>ssid_gbk:%s strlen(ssid_gbk):%d\n", ssid_gbk, strlen(ssid_gbk));
    
    strncpy(wlan_scan_connect_status.wlan_ssid, ssid_gbk, NET_802_11_MAX_LEN_SSID);
    strncpy(wlan_scan_connect_status.wlan_wpa_wep[0], password, SETUP_WIFI_KEY_LEN);

    sscanf(bssid, "%x:%x:%x:%x:%x:%x",
            _rWlanAssociate.t_Bssid,
            _rWlanAssociate.t_Bssid + 1,
			_rWlanAssociate.t_Bssid + 2,
			_rWlanAssociate.t_Bssid + 3,
			_rWlanAssociate.t_Bssid + 4,
			_rWlanAssociate.t_Bssid + 5);

    wlan_scan_connect_status.ui1_ap_count = 1;
    strncpy(_rWlanAssociate.t_Ssid.ui1_aSsid,
            wlan_scan_connect_status.wlan_ssid,
            NET_802_11_MAX_LEN_SSID+1);
    _rWlanAssociate.t_Ssid.ui1_aSsid[NET_802_11_MAX_LEN_SSID]='\0';

    _rWlanAssociate.t_Ssid.ui4_SsidLen = strlen(wlan_scan_connect_status.wlan_ssid);
    _rWlanAssociate.ui1_Priority    = 0;
    _rWlanAssociate.e_AssocCase     = IEEE_802_11_MANUAL_ASSOC;
    _rWlanAssociate.t_Key.pui1_PassPhrase = wlan_scan_connect_status.wlan_wpa_wep[0];
    _rWlanAssociate.t_Key.b_IsAscii = TRUE;

    WIFI_MSG("<WIFI_SETTING_PROC> send associate cmd to mw]:>>>\n");
	WIFI_MSG("<WIFI_SETTING_PROC> Bssid is : %02x:%02x:%02x:%02x:%02x:%02x \n",
            _rWlanAssociate.t_Bssid[0],
            _rWlanAssociate.t_Bssid[1],
            _rWlanAssociate.t_Bssid[2],
            _rWlanAssociate.t_Bssid[3],
            _rWlanAssociate.t_Bssid[4],
            _rWlanAssociate.t_Bssid[5]);

    WIFI_MSG("<WIFI_SETTING_PROC> <MANUAL_ASSOC> SSID:%s, SsidLen:%d, eAssocCase:%d, eAuthMode:%d, e_AuthCipher:%d Priority:%d \n",
            _rWlanAssociate.t_Ssid.ui1_aSsid,
            _rWlanAssociate.t_Ssid.ui4_SsidLen,
            _rWlanAssociate.e_AssocCase,
            _rWlanAssociate.e_AuthMode,
            _rWlanAssociate.e_AuthCipher,
            _rWlanAssociate.ui1_Priority);

    WIFI_MSG("<WIFI_SETTING_PROC> <MANUAL_ASSOC> KEY:%s, IsAscii:%d\n",
            _rWlanAssociate.t_Key.pui1_PassPhrase,
            _rWlanAssociate.t_Key.b_IsAscii);

    //Key is invalid?
    ui4_length = strlen(_rWlanAssociate.t_Key.pui1_PassPhrase);

    //Porting BDP000313474 solution from Demo
     for (ui4Idx = 0; ui4Idx < ui4_length; ui4Idx++)
     {
         if ((_rWlanAssociate.t_Key.pui1_PassPhrase[ui4Idx] > 'f' &&
              _rWlanAssociate.t_Key.pui1_PassPhrase[ui4Idx] <= 'z')
             ||
             (_rWlanAssociate.t_Key.pui1_PassPhrase[ui4Idx] > 'F' &&
             _rWlanAssociate.t_Key.pui1_PassPhrase[ui4Idx] <= 'Z'))
         {
             WIFI_MSG("<WIFI_SETTING_PROC> Get ui4Idx: %d\n", ui4Idx);
             fgPwdIsAscii = TRUE;
             break;
         }
     }

     if(IEEE_802_11_AUTH_MODE_WEPAUTO == _rWlanAssociate.e_AuthMode)
     {
         if(ui4_length == WIFI_AUTH_MODE_WEP_KEY_LEN_10_HEX ||
             (WIFI_AUTH_MODE_WEP_KEY_LEN_26_HEX == ui4_length))
         {
            _rWlanAssociate.t_Key.b_IsAscii = FALSE;
         }
         else if(ui4_length != WIFI_AUTH_MODE_WEP_KEY_LEN_5_ASCII && ui4_length != WIFI_AUTH_MODE_WEP_KEY_LEN_13_ASCII)
         {
             fgPwdIsInvalid = TRUE;
         }
     }
     else if (IEEE_802_11_AUTH_MODE_WPA_PSK                 == _rWlanAssociate.e_AuthMode
         ||   IEEE_802_11_AUTH_MODE_WPA_PSK_GTKIP           == _rWlanAssociate.e_AuthMode
         ||   IEEE_802_11_AUTH_MODE_WPA2_PSK                == _rWlanAssociate.e_AuthMode
         ||   IEEE_802_11_AUTH_MODE_WPA2_PSK_GTKIP          == _rWlanAssociate.e_AuthMode
         ||   IEEE_802_11_AUTH_MODE_WPA2_PSK_GWEP104        == _rWlanAssociate.e_AuthMode
         ||   IEEE_802_11_AUTH_MODE_WPA_PSK_AES             == _rWlanAssociate.e_AuthMode
         ||   IEEE_802_11_AUTH_MODE_WPA_PSK_AES_GTKIP       == _rWlanAssociate.e_AuthMode
         ||   IEEE_802_11_AUTH_MODE_WPA_PSK_TKIP            == _rWlanAssociate.e_AuthMode
         ||   IEEE_802_11_AUTH_MODE_WPA_PSK_TKIP_GWEP104    == _rWlanAssociate.e_AuthMode
         ||   IEEE_802_11_AUTH_MODE_WPA2_PSK_TKIP           == _rWlanAssociate.e_AuthMode
         ||   IEEE_802_11_AUTH_MODE_WPA2_PSK_AES            == _rWlanAssociate.e_AuthMode
         ||   IEEE_802_11_AUTH_MODE_WPA2_PSK_AES_GTKIP      == _rWlanAssociate.e_AuthMode
         ||   IEEE_802_11_AUTH_MODE_WPAPSK_OR_WPA2PSK       == _rWlanAssociate.e_AuthMode
         ||   IEEE_802_11_AUTH_MODE_WPAPSK_OR_WPA2PSK_GTKIP == _rWlanAssociate.e_AuthMode)
     {
        //Porting BDP000313474 solution from Demo
         if(( WIFI_AUTH_MODE_WAP_KEY_LEN_64_HEX == ui4_length)&&
            (FALSE == fgPwdIsAscii))
         {
             _rWlanAssociate.t_Key.b_IsAscii = FALSE;

         }
         else if(ui4_length <WIFI_AUTH_MODE_WAP_KEY_LEN_MIN || ui4_length >WIFI_AUTH_MODE_WAP_KEY_LEN_MAX)
         {
             fgPwdIsInvalid = TRUE;
         }

     }
     else if((IEEE_802_11_AUTH_MODE_NONE == _rWlanAssociate.e_AuthMode) && (0 != ui4_length))
     {
        fgPwdIsInvalid = TRUE;
     }

    if(fgPwdIsInvalid)
    {
        WIFI_MSG("<WIFI_SETTING_PROC> fgPwd Is Invalid , connect fail!\n");
        /*send  wifi_status to assistant_stub */
        if(TRUE == need_respose_wifi_connect_rpc)
        {
            need_respose_wifi_connect_rpc = FALSE;

            ASSISTANT_STUB_WIFI_CONNECT_RESPONSE_T wifi_status ={0};
            memcpy(wifi_status.command, STRING_WIFI_CONNECT, ASSISTANT_STUB_COMMAND_MAX_LENGTH);
            wifi_status.wifi_status= PASSWORD_ERROR;
            wifi_status.id = fg_wifi_connect_id;
            i4_ret = u_am_get_app_handle_from_name(&h_app, ASSISTANT_STUB_THREAD_NAME);
            if(0 != i4_ret)
            {
               WIFI_MSG("<WIFI_SETTING_PROC> get handle fail!,i4_ret=%ld\n",i4_ret);
            }
            i4_ret = u_app_send_appmsg(h_app,
                                      E_APP_MSG_TYPE_ASSISTANT_STUB,
                                      MSG_FROM_WIFI_SETTING,
                                      ASSISTANT_STUB_CMD_WIFI_CONNECT,
                                      &wifi_status,
                                      sizeof(wifi_status));
            if(0 == i4_ret)
            {
               WIFI_MSG("<WIFI_SETTING_PROC> u_app_send_appmsg success !!!\n");
            }
            else
            {
               WIFI_MSG("<WIFI_SETTING_PROC> u_app_send_appmsg fail !!!\n");
            }
        }
        else
        {
           WIFI_MSG("FALSE == need_respose_wifi_connect_rpc , return direct!!!\n");
        }

        return WIFI_FAIL; //Jump to fail page
    }

    wlan_scan_connect_status.ui1_ap_count = 1;
    wlan_scan_connect_status.ui1_wlan_notify_type = WLAN_NOTIFY_ASSOCIATE;
    setup_wlan_fm_owner = WLAN_FM_OWNER_MANUAL_ASSOCIATE_SECURITY_WPS;
    wlan_scan_connect_status._rCurWlanAssociate[0] =_rWlanAssociate;
    wlan_scan_connect_status.ui4_wlan_timer_counter = WLAN_TIMER_ASSOCIATE_TIMEOUT;
    fg_Need_Save_Ap_Info=TRUE;
    i4_ret = c_net_wlan_associate(&_rWlanAssociate, _wifi_setting_wlan_notify_callback);
    if(NET_WLAN_OK != i4_ret)
    {
        if(wlan_scan_connect_status.ui4_wlan_timer_counter > 3)
        {
            wlan_scan_connect_status.ui4_wlan_timer_counter = 3;
        }
        WIFI_MSG("<WIFI_SETTING_PROC> wlan associate fail.\n");
    }

    i4_ret = _setup_wlan_associate_start_timer();

    WIFI_SETTING_PROC_FUNCTION_END;

    return i4_ret;
}

void wifi_setting_notify_bt_wifi_setup_result(void)
{
    INT32 i4_ret = 0;
    HANDLE_T h_app = NULL_HANDLE;

    i4_ret = u_am_get_app_handle_from_name(&h_app, BLUETOOTH_THREAD_NAME);
    if(0 != i4_ret)
    {
        WIFI_MSG("get handle fail, ret:[%d]\n", i4_ret);
        return;
    }

    i4_ret = u_app_send_appmsg(h_app,
                               E_APP_MSG_TYPE_WIFI_SETTING,
                               MSG_FROM_WIFI_SETTING,
                               ASSISTANT_STUB_CMD_WIFI_SETUP_RESULT,
                               NULL,
                               0);
    if(0 == i4_ret)
    {
        WIFI_MSG("u_app_send_appmsg success!\n");
    }
    else
    {
        WIFI_MSG("u_app_send_appmsg failed, ret:[%d]!\n", i4_ret);
    }
}

void wifi_setting_process_favorite_ap_notify(WIFI_MSG_T *pt_msg)
{
    INT32    i4_ret = 0;
    HANDLE_T h_app = NULL_HANDLE;

    WIFI_MSG("<WIFI_SETTING_PROC> ui4_wlan_timer_counter %d\n",wlan_scan_connect_status.ui4_wlan_timer_counter);
    setup_wlan_set_status = WLAN_SET_CONNECT_FAVORVITE_AP;
    if (1 == wlan_scan_connect_status.ui4_wlan_timer_counter || 0 == wlan_scan_connect_status.ui4_wlan_timer_counter)
    {
       WIFI_MSG("<WIFI_SETTING_PROC> [%s]failed\n",__FUNCTION__);
       setup_wlan_fm_owner = WLAN_FM_OWNER_ASSOCIATE_FAILED;
       wlan_scan_connect_status.ui4_wlan_timer_counter = WLAN_TIMER_ASSOCIATE_TIMEOUT;
       a_misc_handle_network_device_active_event(IF_TYPE_IEEE80211,NETWORK_DEVICE_DEACTIVE_EVENT);

       WIFI_MSG("<WIFI_SETTING_PROC> fg_wifi_connect_status is %d\n",fg_wifi_connect_status);
       if(1 == fg_wifi_connect_status)
       {
           fg_wifi_connect_status = 0;
           WIFI_MSG("<ASSISTANT_STUB_APP> ifconfig wlan0 0.0.0.0\n");
           system("ifconfig wlan0 0.0.0.0");

           ASSISTANT_STUB_NETWORK_STATUS_CHANGE_T network_status_change = {0};
           NET_802_11_ASSOCIATE_T _rSavedWlanAssociate={{0}};
           a_cfg_get_wlan_ap(&_rSavedWlanAssociate);
           strncpy(network_status_change.command,"/system/network_status_change",ASSISTANT_STUB_COMMAND_MAX_LENGTH);
           network_status_change.quantity = 0;
           strncpy(network_status_change.status,"disconnect",ASSISTANT_STUB_STATUS_MAX_LENGTH);
           strncpy(network_status_change.ssid,
                   _rSavedWlanAssociate.t_Ssid.ui1_aSsid,
                   NET_802_11_MAX_LEN_SSID);
           snprintf(network_status_change.bssid, ASSISTANT_STUB_BSSID_MAX_LENGTH, "%x:%x:%x:%x:%x:%x",
                    _rSavedWlanAssociate.t_Bssid[0], _rSavedWlanAssociate.t_Bssid[1], _rSavedWlanAssociate.t_Bssid[2],
                    _rSavedWlanAssociate.t_Bssid[3], _rSavedWlanAssociate.t_Bssid[4], _rSavedWlanAssociate.t_Bssid[5]);

           /*send  network_status change  to sm */
           i4_ret = u_am_get_app_handle_from_name(&h_app, SM_THREAD_NAME);
           if(MISC_APPR_OK != i4_ret)
           {
               WIFI_MSG("<WIFI_SETTING_PROC> get handle fail!,i4_ret=%ld\n",i4_ret);
           }
           i4_ret = u_app_send_appmsg(h_app,
                                      E_APP_MSG_TYPE_MISC,
                                      MSG_FROM_WIFI_SETTING,
                                      ASSISTANT_STUB_CMD_NETWORK_STATUS_CHANGE,
                                      &network_status_change,
                                      sizeof(ASSISTANT_STUB_NETWORK_STATUS_CHANGE_T));
           if(0 == i4_ret)
           {
               WIFI_MSG("<WIFI_SETTING_PROC> u_app_send_appmsg success !!!\n");
           }
           else
           {
               WIFI_MSG("<WIFI_SETTING_PROC> u_app_send_appmsg fail !!!\n");
           }
       }
    }
    else
    {
       if (pt_msg->ui4_data2 == WLAN_NFY_MSG_OK)//associate ok.
       {
          if (t_g_this_obj.t_associate_timer.h_timer != (HANDLE_T)NULL)//(h_timer_wlan != (HANDLE_T)NULL)
          {
              //u_timer_stop(h_timer_wlan);
              u_timer_stop(t_g_this_obj.t_associate_timer.h_timer);
          }

          WIFI_MSG("<WIFI_SETTING_PROC> fg_wifi_connect_status is %d line = %d\n",fg_wifi_connect_status, __LINE__);
          if(0 == fg_wifi_connect_status)
          {
              fg_wifi_connect_status = 1;
          }

          setup_wlan_fm_owner = WLAN_FM_OWNER_ASSOCIATE_OK;

          a_misc_handle_network_device_active_event(IF_TYPE_IEEE80211,NETWORK_DEVICE_ACTIVE_EVENT);
       }
       else if (pt_msg->ui4_data2 == WLAN_NFY_MSG_DISCONNECTED)
       {
           WIFI_MSG("<WIFI_SETTING_PROC> WIFI is disconnected...\n");

           WIFI_MSG("<WIFI_SETTING_PROC> fg_wifi_connect_status is %d line = %d\n",fg_wifi_connect_status, __LINE__);

           if(1 == fg_wifi_connect_status)
           {
                fg_wifi_connect_status = 0;
                WIFI_MSG("<WIFI_SETTING_PROC> fg_wifi_connect_status is %d line = %d\n",fg_wifi_connect_status, __LINE__);

                WIFI_MSG("<ASSISTANT_STUB_APP> ifconfig wlan0 0.0.0.0\n");
                system("ifconfig wlan0 0.0.0.0");

                a_misc_handle_network_device_active_event(IF_TYPE_IEEE80211,NETWORK_DEVICE_DEACTIVE_EVENT);

                ASSISTANT_STUB_NETWORK_STATUS_CHANGE_T network_status_change = {0};
                NET_802_11_ASSOCIATE_T _rSavedWlanAssociate={{0}};
                a_cfg_get_wlan_ap(&_rSavedWlanAssociate);
                strncpy(network_status_change.command,"/system/network_status_change",ASSISTANT_STUB_COMMAND_MAX_LENGTH);
                network_status_change.quantity = 0;
                strncpy(network_status_change.status,"disconnect",ASSISTANT_STUB_STATUS_MAX_LENGTH);
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
                    WIFI_MSG("<WIFI_SETTING_PROC> get handle fail!,i4_ret=%ld\n",i4_ret);
                }
                i4_ret = u_app_send_appmsg(h_app,
                                           E_APP_MSG_TYPE_MISC,
                                           MSG_FROM_WIFI_SETTING,
                                           ASSISTANT_STUB_CMD_NETWORK_STATUS_CHANGE,
                                           &network_status_change,
                                           sizeof(ASSISTANT_STUB_NETWORK_STATUS_CHANGE_T));
                if(0 == i4_ret)
                {
                    WIFI_MSG("<WIFI_SETTING_PROC> u_app_send_appmsg success !!!\n");
                }
                else
                {
                    WIFI_MSG("<WIFI_SETTING_PROC> u_app_send_appmsg fail !!!\n");
                }

           }
           else
           {
               WIFI_MSG("<WIFI_SETTING_PROC> fg_wifi_connect_status is %d line = %d\n",fg_wifi_connect_status, __LINE__);
           }

       }
       else
       {
           WIFI_MSG("<WIFI_SETTING_PROC> WLAN_NFY_MSG_ASSOC,:Message = %d,Result = %d\n", pt_msg->ui4_data1,pt_msg->ui4_data2);
       }
    }
}

void wifi_setting_process_non_favorite_ap_notify(WIFI_MSG_T *pt_msg)
{
    INT32    i4_ret = 0;
    HANDLE_T h_app = NULL_HANDLE;

    WIFI_MSG("<WIFI_SETTING_PROC> ui4_wlan_timer_counter %d %d\n",wlan_scan_connect_status.ui4_wlan_timer_counter,__LINE__);
    if (1 == wlan_scan_connect_status.ui4_wlan_timer_counter || 0 == wlan_scan_connect_status.ui4_wlan_timer_counter)
    {
        wlan_scan_connect_status.ui4_wlan_timer_counter = WLAN_TIMER_ASSOCIATE_TIMEOUT;
        a_misc_handle_network_device_active_event(IF_TYPE_IEEE80211,NETWORK_DEVICE_DEACTIVE_EVENT);
        WIFI_MSG("<WIFI_SETTING_PROC> [%s]WLAN associate failed %d\n",__FUNCTION__, __LINE__);

        WIFI_MSG("<WIFI_SETTING_PROC> fg_wifi_connect_status is %d line = %d\n",fg_wifi_connect_status, __LINE__);
        if(1 == fg_wifi_connect_status)
        {
            fg_wifi_connect_status = 0;
        }

        WIFI_MSG("need_respose_wifi_connect_rpc is %d\n",need_respose_wifi_connect_rpc);
        if (TRUE == need_respose_wifi_connect_rpc)
        {
            /*send  wifi_status to assistant_stub */
            need_respose_wifi_connect_rpc = FALSE;

            ASSISTANT_STUB_WIFI_CONNECT_RESPONSE_T wifi_status ={0};
            memcpy(wifi_status.command, STRING_WIFI_CONNECT, ASSISTANT_STUB_COMMAND_MAX_LENGTH);
            wifi_status.wifi_status= ASSOCIATE_TIMEDOUT;
            wifi_status.id = fg_wifi_connect_id;
            WIFI_MSG("<WIFI_SETTING_PROC> associate timeout,connect fail!!!\n");
            i4_ret = u_am_get_app_handle_from_name(&h_app, ASSISTANT_STUB_THREAD_NAME);
            if(0 != i4_ret)
            {
                WIFI_MSG("<WIFI_SETTING_PROC> get handle fail!,i4_ret=%ld\n",i4_ret);
            }
            i4_ret = u_app_send_appmsg(h_app,
                    E_APP_MSG_TYPE_ASSISTANT_STUB,
                    MSG_FROM_WIFI_SETTING,
                    ASSISTANT_STUB_CMD_WIFI_CONNECT,
                    &wifi_status,
                    sizeof(wifi_status));
            if(0 == i4_ret)
            {
                WIFI_MSG("<WIFI_SETTING_PROC> u_app_send_appmsg success !!!\n");
            }
            else
            {
                WIFI_MSG("<WIFI_SETTING_PROC> u_app_send_appmsg fail !!!\n");
            }
        }
        else
        {
            WIFI_MSG("FALSE == need_respose_wifi_connect_rpc , return direct!!!\n");
        }
    }
    else
    {
        if (pt_msg->ui4_data2 == WLAN_NFY_MSG_OK)//associate ok.
        {
            if (t_g_this_obj.t_associate_timer.h_timer != (HANDLE_T)NULL)//(h_timer_wlan != (HANDLE_T)NULL)
            {
                //u_timer_stop(h_timer_wlan);
                u_timer_stop(t_g_this_obj.t_associate_timer.h_timer);
            }

            WIFI_MSG("<WIFI_SETTING_PROC> fg_wifi_connect_status is %d line = %d\n",fg_wifi_connect_status, __LINE__);
            if(0 == fg_wifi_connect_status)
            {
                fg_wifi_connect_status = 1;
            }

            a_misc_handle_network_device_active_event(IF_TYPE_IEEE80211,NETWORK_DEVICE_ACTIVE_EVENT);

            if(fg_Need_Save_Ap_Info)
            {
                WIFI_MSG("<WIFI_SETTING_PROC> setup_save_wlan_key.\n");
                setup_save_wlan_key();
            }

            wlan_scan_connect_status.ui4_wlan_timer_counter=WLAN_TIMER_ASSOCIATE_TIMEOUT;
        }
        else if (pt_msg->ui4_data2 == WLAN_NFY_MSG_NO_MATCHED_CONFIG)
        {
            WIFI_MSG("<WIFI_SETTING_PROC> WLAN_NFY_MSG_NO_MATCHED_CONFIG ,password is error!!!\n");
#if 0 /*wifi password error retry*/
            if (WLAN_FM_OWNER_MANUAL_ASSOCIATE_SECURITY_WPS == setup_wlan_fm_owner)
            {
                associate_count++ ;
                WIFI_MSG("<WIFI_SETTING_PROC> associate retry associate_count=%d\n",associate_count);
                if( associate_count <= 2 )
                {
                    wlan_scan_connect_status.ui4_wlan_timer_counter == WLAN_TIMER_ASSOCIATE_TIMEOUT;

                    //setup_goto_manual_associate(wifi_ConnectSsid,wifi_ConnectPwd,0);

                    return WIFI_OK;
                }
                else
                {
                    WIFI_MSG("<WIFI_SETTING_PROC> associate retry fali !!!!!!\n");
                }
            }
#endif
            /*send  wifi_status to assistant_stub */
            if(TRUE == need_respose_wifi_connect_rpc)
            {
                need_respose_wifi_connect_rpc = FALSE;

                ASSISTANT_STUB_WIFI_CONNECT_RESPONSE_T wifi_status ={0};
                memcpy(wifi_status.command, STRING_WIFI_CONNECT, ASSISTANT_STUB_COMMAND_MAX_LENGTH);
                wifi_status.wifi_status= PASSWORD_ERROR;
                wifi_status.id = fg_wifi_connect_id;
                WIFI_MSG("<WIFI_SETTING_PROC> password error ,connect fail!!!\n");
                i4_ret = u_am_get_app_handle_from_name(&h_app, ASSISTANT_STUB_THREAD_NAME);
                if(0 != i4_ret)
                {
                    WIFI_MSG("<WIFI_SETTING_PROC> get handle fail!,i4_ret=%ld\n",i4_ret);
                }
                i4_ret = u_app_send_appmsg(h_app,
                        E_APP_MSG_TYPE_ASSISTANT_STUB,
                        MSG_FROM_WIFI_SETTING,
                        ASSISTANT_STUB_CMD_WIFI_CONNECT,
                        &wifi_status,
                        sizeof(wifi_status));
                if(0 == i4_ret)
                {
                    WIFI_MSG("<WIFI_SETTING_PROC> u_app_send_appmsg success !!!\n");
                }
                else
                {
                    WIFI_MSG("<WIFI_SETTING_PROC> u_app_send_appmsg fail !!!\n");
                }
            }
            else
            {
                WIFI_MSG("FALSE == need_respose_wifi_connect_rpc , return direct!!!\n");
            }

        }
        else if (pt_msg->ui4_data2 == WLAN_NFY_MSG_PSK_INCORRECT)
        {
            WIFI_MSG("<WIFI_SETTING_PROC> WLAN_NFY_MSG_PSK_INCORRECT\n");

            /*send  wifi_status to assistant_stub */
            if(TRUE == need_respose_wifi_connect_rpc)
            {
                need_respose_wifi_connect_rpc = FALSE;

                ASSISTANT_STUB_WIFI_CONNECT_RESPONSE_T wifi_status ={0};
                memcpy(wifi_status.command, STRING_WIFI_CONNECT, ASSISTANT_STUB_COMMAND_MAX_LENGTH);
                wifi_status.wifi_status = PASSWORD_ERROR;
                wifi_status.id = fg_wifi_connect_id;
                i4_ret = u_am_get_app_handle_from_name(&h_app, ASSISTANT_STUB_THREAD_NAME);
                if(0 != i4_ret)
                {
                    WIFI_MSG("<WIFI_SETTING_PROC> get handle fail!,i4_ret=%ld\n",i4_ret);
                }
                i4_ret = u_app_send_appmsg(h_app,
                        E_APP_MSG_TYPE_ASSISTANT_STUB,
                        MSG_FROM_WIFI_SETTING,
                        ASSISTANT_STUB_CMD_WIFI_CONNECT,
                        &wifi_status,
                        sizeof(wifi_status));
                if(0 == i4_ret)
                {
                    WIFI_MSG("<WIFI_SETTING_PROC> u_app_send_appmsg success !!!\n");
                }
                else
                {
                    WIFI_MSG("<WIFI_SETTING_PROC> u_app_send_appmsg fail !!!\n");
                }
            }
            else
            {
                WIFI_MSG("FALSE == need_respose_wifi_connect_rpc , return direct!!!\n");
            }

        }
        else if (pt_msg->ui4_data2 == WLAN_NFY_MSG_DISCONNECTED)
        {
            WIFI_MSG("<WIFI_SETTING_PROC> WLAN_NFY_MSG_DISCONNECTED WIFI is disconnected...\n");

            WIFI_MSG("<WIFI_SETTING_PROC> fg_wifi_connect_status is %d line = %d\n",fg_wifi_connect_status, __LINE__);

            if(1 == fg_wifi_connect_status)
            {
                fg_wifi_connect_status = 0;

                WIFI_MSG("<ASSISTANT_STUB_APP> ifconfig wlan0 0.0.0.0\n");
                system("ifconfig wlan0 0.0.0.0");

                a_misc_handle_network_device_active_event(IF_TYPE_IEEE80211,NETWORK_DEVICE_DEACTIVE_EVENT);

                /*send  network status change to assistant_stub */
                ASSISTANT_STUB_NETWORK_STATUS_CHANGE_T network_status_change = {0};
                NET_802_11_ASSOCIATE_T _rSavedWlanAssociate={{0}};
                a_cfg_get_wlan_ap(&_rSavedWlanAssociate);
                strncpy(network_status_change.command,"/system/network_status_change",ASSISTANT_STUB_COMMAND_MAX_LENGTH);
                network_status_change.quantity = 0;
                strncpy(network_status_change.status,"disconnect",ASSISTANT_STUB_STATUS_MAX_LENGTH);
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
                    WIFI_MSG("<MISC_DHCP> get handle fail!,i4_ret=%ld\n",i4_ret);
                }
                i4_ret = u_app_send_appmsg(h_app,
                        E_APP_MSG_TYPE_MISC,
                        MSG_FROM_WIFI_SETTING,
                        ASSISTANT_STUB_CMD_NETWORK_STATUS_CHANGE,
                        &network_status_change,
                        sizeof(ASSISTANT_STUB_NETWORK_STATUS_CHANGE_T));
                if(0 == i4_ret)
                {
                    WIFI_MSG("<MISC_DHCP> u_app_send_appmsg success !!!\n");
                }
                else
                {
                    WIFI_MSG("<MISC_DHCP> u_app_send_appmsg fail !!!\n");
                }
            }
        }
        else
        {
            WIFI_MSG("<WIFI_SETTING_PROC> *******WLAN_NFY_MSG_ASSOC,:Message = %d,Result = %d\n", pt_msg->ui4_data1,pt_msg->ui4_data2);
        }
    }
}

void wifi_setting_process_associate_notify(WIFI_MSG_T *pt_msg)
{
    WIFI_MSG("<WIFI_SETTING_PROC> WLAN_NOTIFY_ASSOCIATE type %d,fm %d\n",pt_msg->ui4_data2,setup_wlan_fm_owner);

    if (setup_wlan_fm_owner == WLAN_FM_OWNER_MANUAL_ASSOCIATE_FAVORITE_AP)
    {
        wifi_setting_process_favorite_ap_notify(pt_msg);
        wifi_setting_notify_bt_wifi_setup_result();
    }
    else if (setup_wlan_fm_owner == WLAN_FM_OWNER_SCAN_ASSOCIATE_SECURITY_NONE
        || setup_wlan_fm_owner == WLAN_FM_OWNER_SCAN_ASSOCIATE_SECURITY_WPS
        || setup_wlan_fm_owner == WLAN_FM_OWNER_MANUAL_ASSOCIATE_SECURITY_NONE
        || setup_wlan_fm_owner == WLAN_FM_OWNER_MANUAL_ASSOCIATE_SECURITY_WPS
        || setup_wlan_fm_owner == WLAN_FM_OWNER_MANUAL_ASSOCIATE_FROM_DONGLE_IN
        || setup_wlan_fm_owner == WLAN_FM_OWNER_ASSOCIATE_OK
        || setup_wlan_fm_owner == WLAN_FM_OWNER_WPS_PBC_ASSOCIATE
        || setup_wlan_fm_owner == WLAN_FM_OWNER_WPS_PIN_ASSOCIATE
        || setup_wlan_fm_owner == WLAN_FM_OWNER_ASSOCIATE_FAILED)
    {
        wifi_setting_process_non_favorite_ap_notify(pt_msg);
    }
}

INT32 _wifi_setting_wlan_notify_msg_proc(const VOID *pv_msg)
{
    WIFI_SETTING_PROC_FUNCTION_BEGIN;

    INT32    i4_ret = 0;
    HANDLE_T h_app = NULL_HANDLE;
    NET_802_11_SCAN_RESULT_T scan_list;
    NET_802_11_BSS_INFO_T *scan_cur_result;
    WIFI_MSG_T *pt_msg = (WIFI_MSG_T*)pv_msg;
    UINT16 i4_num, total_num,i4_ssid_null_num=0, i4_ssid_repeat_num = 0;
    BOOL b_repeat = FALSE;

    WIFI_MSG("<WIFI_SETTING_PROC> %s, L%d, pt_msg->ui4_data1 %d result %d\n",__FUNCTION__,__LINE__,pt_msg->ui4_data1,pt_msg->ui4_data2);
    WIFI_MSG("<WIFI_SETTING_PROC> Message = %d\n",pt_msg->ui4_data1 );
    WIFI_MSG("<WIFI_SETTING_PROC> Result = %d\n",pt_msg->ui4_data2 );
    WIFI_MSG("<WIFI_SETTING_PROC> Timer_counter= %d\n",wlan_scan_connect_status.ui4_wlan_timer_counter);

    switch (pt_msg->ui4_data1)
    {
         case WLAN_NOTIFY_INIT_IND:
         {
            WIFI_MSG("<WIFI_SETTING_PROC>  WLAN_NOTIFY_INIT_IND \n");
            if(pt_msg->ui4_data2 == 0)
            {
                WIFI_MSG("<WIFI_SETTING_PROC> OK \n");
            }
            else
            {
                WIFI_MSG("<WIFI_SETTING_PROC> FAIL \n");
            }
         }
         break;

        case WLAN_NOTIFY_DEINIT_IND:
        {
            WIFI_MSG("<WIFI_SETTING_PROC>  WLAN_NOTIFY_DEINIT_IND \n");
            if(pt_msg->ui4_data2 == 0)
            {
                WIFI_MSG("<WIFI_SETTING_PROC> OK \n");
            }
            else
            {
                WIFI_MSG("<WIFI_SETTING_PROC> FAIL \n");
            }
        }
        break;

        case WLAN_NOTIFY_SCAN:
        {
           WIFI_MSG("<WIFI_SETTING_PROC>  WLAN_NOTIFY_SCAN %d\n",pt_msg->ui4_data2);

           if (WLAN_FM_OWNER_AP_SCANING == setup_wlan_fm_owner)
           {
               if (t_g_this_obj.t_scan_timer.h_timer != (HANDLE_T)NULL)//(h_timer_wlan != (HANDLE_T)NULL)
               {
                   //u_timer_stop(h_timer_wlan);
                   u_timer_stop(t_g_this_obj.t_scan_timer.h_timer);
               }
                i4_ret = c_net_wlan_get_scan_result(&scan_list);
                total_num = scan_list.ui4_NumberOfItems;
                scan_cur_result = scan_list.p_BssInfo;
                memset(wlan_scan_connect_status.cur_show_result, 0, sizeof(NET_802_11_BSS_INFO_T) * SETUP_WLAN_SCAN_RESULT_SHOW_NUM);

                WIFI_MSG("<WIFI_SETTING_PROC> WLAN_NOTIFY_SCAN total_num %d %d\n",total_num,__LINE__);
                if (i4_ret != NET_WLAN_OK)
                {
                    WIFI_MSG("<WIFI_SETTING_PROC> c_net_wlan_get_scan_result failed %d %d\n",i4_ret, __LINE__);

                    return WIFI_OK;
                }

                if (0 == total_num)
                {
                    WIFI_MSG("<WIFI_SETTING_PROC> no scan result \n");
                    return WIFI_OK;
                }

                if (total_num > SETUP_WLAN_SCAN_RESULT_SHOW_NUM)
                {
                    total_num = SETUP_WLAN_SCAN_RESULT_SHOW_NUM;
                }


                for(i4_num = 0; i4_num < total_num; i4_num++)
                {
                    WIFI_MSG("<WIFI_SETTING_PROC> item%d>>>>>>>>>>[Ssid    ] %s \n",i4_num,scan_cur_result[i4_num].t_Ssid.ui1_aSsid);
                    WIFI_MSG("<WIFI_SETTING_PROC> item%d>>>>>>>>>>[ssid_len] %d \n",i4_num,scan_cur_result[i4_num].t_Ssid.ui4_SsidLen);
                    WIFI_MSG("<WIFI_SETTING_PROC> item%d>>>>>>>>>>[AuthMode] %d \n",i4_num,scan_cur_result[i4_num].e_AuthMode);
                    WIFI_MSG("<WIFI_SETTING_PROC> item%d>>>>>>>>>>[Level   ] %d \n",i4_num,scan_cur_result[i4_num].i2_Level);
                    WIFI_MSG("<WIFI_SETTING_PROC> item%d>>>>>>>>>>[Quality ] %d \n",i4_num,scan_cur_result[i4_num].i2_Quality);
                    WIFI_MSG("<WIFI_SETTING_PROC> item%d>>>>>>>>>>[Protocal] %d \n",i4_num,scan_cur_result[i4_num].e_802_11_type);
                    WIFI_MSG("<WIFI_SETTING_PROC> item%d>>>>>>>>>>[WPS Flag] %d \n",i4_num,scan_cur_result[i4_num].is_wps_support);
                    // Filter out hidden SSIDS
                    if (*(scan_cur_result[i4_num].t_Ssid.ui1_aSsid)  == '\0'
                       || scan_cur_result[i4_num].t_Ssid.ui4_SsidLen == 0)
                    {
                        WIFI_MSG("<WIFI_SETTING_PROC> ======discard result %d======%d\n",i4_num,__LINE__);
                        i4_ssid_null_num ++;
                        continue;
                    }

                    // Filter out not support SSIDS
                    if (scan_cur_result[i4_num].e_AuthMode == IEEE_802_11_AUTH_MODE_WPA
                     || scan_cur_result[i4_num].e_AuthMode == IEEE_802_11_AUTH_MODE_WPA2)
                    {
                        WIFI_MSG("<WIFI_SETTING_PROC> ======discard result %d======%d\n",i4_num,__LINE__);
                        i4_ssid_null_num ++;
                        continue;
                    }

                    for (i4_ssid_repeat_num = 0; i4_ssid_repeat_num < i4_num - i4_ssid_null_num; i4_ssid_repeat_num++)
                    {
                        if (!strcmp(scan_cur_result[i4_num].t_Ssid.ui1_aSsid,
                            wlan_scan_connect_status.cur_show_result[i4_ssid_repeat_num].t_Ssid.ui1_aSsid))
                        {
                            WIFI_MSG("<WIFI_SETTING_PROC> ======discard result %d ======%d\n",i4_num,__LINE__);
                            WIFI_MSG("<WIFI_SETTING_PROC> ======discard result %s ======\n",scan_cur_result[i4_num].t_Ssid.ui1_aSsid);
                            b_repeat = TRUE;
                            break;
                        }
                    }

                    if (b_repeat)
                    {
                        i4_ssid_null_num ++;
                        b_repeat = FALSE;
                        continue;
                    }

                    if(wlan_scan_connect_status.b_scan_request_from_pin_code)
                    {
                        WIFI_MSG("<WIFI_SETTING_PROC> ======Scan request from Pin code======\n");
                    }

                    if (i4_num < i4_ssid_null_num)
                    {
                        WIFI_MSG("<WIFI_SETTING_PROC> i4_num < i4_ssid_null_num error return %d\n", __LINE__);
                        return i4_ret;
                    }

                    if (SETUP_WLAN_SCAN_RESULT_SHOW_NUM > i4_num - i4_ssid_null_num)
                    {
                        wlan_scan_connect_status.cur_show_result[i4_num - i4_ssid_null_num] = scan_cur_result[i4_num];
                        wlan_scan_connect_status.cur_show_result[i4_num - i4_ssid_null_num].i2_Level = ((wlan_scan_connect_status.cur_show_result[i4_num - i4_ssid_null_num].i2_Level -5 )*50)/95 + 20;
                    }
                    else
                    {
                        WIFI_MSG("<WIFI_SETTING_PROC> i4_num - i4_ssid_null_num > SETUP_WLAN_SCAN_RESULT_SHOW_NUM error return %d\n", __LINE__);
                        return i4_ret;
                    }

                }

                total_num = total_num - i4_ssid_null_num;
                WIFI_MSG("<WIFI_SETTING_PROC> i4_ssid_null_num %d %d\n",i4_ssid_null_num,__LINE__);
                WIFI_MSG("<WIFI_SETTING_PROC> Total Recieve usefull items : %d\n",total_num);
                WIFI_MSG("<WIFI_SETTING_PROC> Total Recieve null    items : %d\n",i4_ssid_null_num);

                setup_wlan_show_scan_result(total_num);

            }
        }
        break;

        case WLAN_NOTIFY_ASSOCIATE:
            wifi_setting_process_associate_notify(pt_msg);
        break;

        case WLAN_NOTIFY_WPS_ASSOC_SETTING:
        {
            WIFI_MSG("<WIFI_SETTING_PROC> WLAN_NOTIFY_WPS_ASSOC_SETTING \n");
            {
                if(pt_msg->ui4_data2>0)
                {
                    WIFI_MSG("<WIFI_SETTING_PROC> OK %d\n", __LINE__);
                }
            }
        }
        break;

        case WLAN_NOTIFY_WPS_STATE:
        {
            WIFI_MSG("<WIFI_SETTING_PROC> WLAN_NOTIFY_WPS_STATE \n");
        }
        break;

        case WLAN_NOTIFY_WPS_ASSOCIATE:
        {
            WIFI_MSG("<WIFI_SETTING_PROC> WLAN_NOTIFY_WPS_ASSOCIATE \n");

            if ( setup_wlan_fm_owner == WLAN_FM_OWNER_WPS_PBC_ASSOCIATE
              ||setup_wlan_fm_owner == WLAN_FM_OWNER_WPS_PIN_ASSOCIATE )
            {
               WIFI_MSG("<WIFI_SETTING_PROC> ui4_wlan_timer_counter %d %d\n",wlan_scan_connect_status.ui4_wlan_timer_counter,__LINE__);
               if (1 == wlan_scan_connect_status.ui4_wlan_timer_counter || 0 == wlan_scan_connect_status.ui4_wlan_timer_counter)
               {
                   wlan_scan_connect_status.ui4_wlan_timer_counter = WLAN_TIMER_ASSOCIATE_TIMEOUT;

                   a_misc_handle_network_device_active_event(IF_TYPE_IEEE80211,NETWORK_DEVICE_DEACTIVE_EVENT);

                   WIFI_MSG("<WIFI_SETTING_PROC> [%s]failed %d\n",__FUNCTION__, __LINE__);

                   setup_wlan_fm_owner = WLAN_FM_OWNER_WPS_ASSOCIATE_FAILED;
               }
               else
               {
                   if (pt_msg->ui4_data2 == WLAN_NFY_MSG_OK)//associate ok.
                   {
                       if (t_g_this_obj.t_associate_timer.h_timer != (HANDLE_T)NULL)//(h_timer_wlan != (HANDLE_T)NULL)
                       {
                           //u_timer_stop(h_timer_wlan);
                           u_timer_stop(t_g_this_obj.t_associate_timer.h_timer);
                       }

                       a_misc_handle_network_device_active_event(IF_TYPE_IEEE80211,NETWORK_DEVICE_ACTIVE_EVENT);

                       setup_wlan_fm_owner = WLAN_FM_OWNER_ASSOCIATE_OK;
                   }
                   else if(WLAN_NFY_MSG_ALREADY_ASSOCIATED == pt_msg->ui4_data2)
                   {
                       WIFI_MSG("<WIFI_SETTING_PROC> WLAN_NFY_MSG_ALREADY_ASSOCIATED \n");
                       WIFI_MSG("<WIFI_SETTING_PROC> WLAN_NFY_MSG_ALREADY_ASSOCIATED \n");
                   }
                   else if(WLAN_NFY_MSG_ASSOC_TIMEOUT == pt_msg->ui4_data2)
                   {
                       if (t_g_this_obj.t_associate_timer.h_timer != (HANDLE_T)NULL)//(h_timer_wlan != (HANDLE_T)NULL)
                       {
                           //u_timer_stop(h_timer_wlan);
                           u_timer_stop(t_g_this_obj.t_associate_timer.h_timer);
                       }

                       WIFI_MSG("<WIFI_SETTING_PROC> WLAN_NFY_MSG_ASSOC_TIMEOUT,:Message = %d,Result = %d\n", pt_msg->ui4_data1,pt_msg->ui4_data2);

                       a_misc_handle_network_device_active_event(IF_TYPE_IEEE80211,NETWORK_DEVICE_DEACTIVE_EVENT);

                       WIFI_MSG("<WIFI_SETTING_PROC> [%s]failed %d\n",__FUNCTION__, __LINE__);

                       wlan_scan_connect_status.ui4_wlan_timer_counter=WLAN_TIMER_RESET;

                       setup_wlan_fm_owner = WLAN_FM_OWNER_WPS_ASSOCIATE_FAILED;

                   }
                   else if(WLAN_NFY_MSG_WPS_PBC_OVERLAP == pt_msg->ui4_data2)
                   {
                      a_misc_handle_network_device_active_event(IF_TYPE_IEEE80211,NETWORK_DEVICE_DEACTIVE_EVENT);

                      WIFI_MSG("<WIFI_SETTING_PROC> [%s]WLAN PBC ASSOCIATE FAIL AS OVERLAP %d\n",__FUNCTION__, __LINE__);

                      wlan_scan_connect_status.ui4_wlan_timer_counter=WLAN_TIMER_RESET;

                      setup_wlan_fm_owner = WLAN_FM_OWNER_WPS_ASSOCIATE_FAILED;

                   }
                   else
                   {
                       WIFI_MSG("<WIFI_SETTING_PROC> WLAN_NFY_MSG_ASSOC,:Message = %d,Result = %d\n", pt_msg->ui4_data1,pt_msg->ui4_data2);
                   }
               }
           }
       }
       break;

       default:
       {
           WIFI_MSG("<WIFI_SETTING_PROC> pt_msg->ui4_data1 = %d\n", pt_msg->ui4_data1);
       }
       break;
    }

    WIFI_SETTING_PROC_FUNCTION_END;

    return WIFI_OK;
}

INT32 _setup_wlan_prepare_to_scan(VOID)
{
    WIFI_SETTING_PROC_FUNCTION_BEGIN;

    INT32 i4_ret = 0;
    setup_wlan_fm_owner = WLAN_FM_OWNER_AP_SCANING;
    b_wlan_handle_scan_cmd = TRUE;
    wlan_scan_connect_status.b_user_cancel = FALSE;

    if (t_g_this_obj.t_scan_timer.h_timer != (HANDLE_T)NULL)//(h_timer_wlan != (HANDLE_T)NULL)
    {
        //u_timer_stop(h_timer_wlan);
        u_timer_stop(t_g_this_obj.t_scan_timer.h_timer);
    }

    wlan_scan_connect_status.ui4_wlan_timer_counter = WLAN_TIMER_SCAN_TIMEOUT + 1;

     //Wifi timer
#if 0
    i4_ret = u_timer_start(h_timer_wlan,
                           1000,
                           X_TIMER_FLAG_REPEAT,
                           _setup_wlan_scan_timer_cb,
                           NULL);
#else
    WIFI_SETTING_TIMER_MSG_E e_scan_msg = TIMER_MSG_WIFI_SCAN;
    t_g_this_obj.t_scan_timer.e_flags   = X_TIMER_FLAG_REPEAT;
    t_g_this_obj.t_scan_timer.ui4_delay = 1000;

    i4_ret = u_timer_start(t_g_this_obj.h_app,
                           &t_g_this_obj.t_scan_timer,
                           (void *)&e_scan_msg,
                           sizeof(WIFI_SETTING_TIMER_MSG_E));
#endif

    WIFI_SETTING_PROC_FUNCTION_END;

    return WIFI_OK;
}

INT32 _setup_wlan_stop_scan(VOID)
{
    WIFI_SETTING_PROC_FUNCTION_BEGIN;

    wlan_scan_connect_status.ui4_wlan_timer_counter = 1;

    WIFI_SETTING_PROC_FUNCTION_END;

    return WIFI_OK;
}

VOID _setup_wlan_scan_timer_cb(VOID)
{
    WIFI_SETTING_PROC_FUNCTION_BEGIN;

    INT32 i4_ret = 0;

    wlan_scan_connect_status.ui4_wlan_timer_counter--;

    WIFI_MSG("<WIFI_SETTING_PROC>:  ui4_wlan_timer_counter %d %d\n",wlan_scan_connect_status.ui4_wlan_timer_counter,__LINE__);

    if (wlan_scan_connect_status.ui4_wlan_timer_counter == WLAN_TIMER_SCAN_TIMEOUT )
    {
        wlan_scan_connect_status.ui4_wlan_timer_counter = WLAN_TIMER_SCAN_TIMEOUT-1;
        wlan_scan_connect_status.ui1_wlan_notify_type   = WLAN_NOTIFY_SCAN;

        i4_ret = c_net_wlan_scan(_wifi_setting_wlan_notify_callback);
        if(0 != i4_ret)
        {
            WIFI_MSG("<WIFI_SETTING_PROC> wlan scan fail %d\n",i4_ret);
        }
    }

   if (wlan_scan_connect_status.ui4_wlan_timer_counter <= 1)
   {
       if (t_g_this_obj.t_scan_timer.h_timer != (HANDLE_T)NULL)//(h_timer_wlan != (HANDLE_T)NULL)
       {
           //u_timer_stop(h_timer_wlan);
           u_timer_stop(t_g_this_obj.t_scan_timer.h_timer);
       }
   }

   WIFI_SETTING_PROC_FUNCTION_END;

    return;
}

VOID _setup_wlan_associate_timer_cb(VOID)
{
    WIFI_SETTING_PROC_FUNCTION_BEGIN;
    INT32 i4_ret;
    HANDLE_T h_app = NULL_HANDLE;
    wlan_scan_connect_status.ui4_wlan_timer_counter--;
    if (wlan_scan_connect_status.ui4_wlan_timer_counter <= 0)
    {
        WIFI_MSG("<WIFI_SETTING_PROC> associate time out.\r\n");

        if(1 == fg_wifi_connect_status)
        {
           fg_wifi_connect_status = 0;
           WIFI_MSG("<ASSISTANT_STUB_APP> ifconfig wlan0 0.0.0.0\n");
           system("ifconfig wlan0 0.0.0.0");

           ASSISTANT_STUB_NETWORK_STATUS_CHANGE_T network_status_change = {0};
           NET_802_11_ASSOCIATE_T _rSavedWlanAssociate={0};
           a_cfg_get_wlan_ap(&_rSavedWlanAssociate);
           strncpy(network_status_change.command,"/system/network_status_change",ASSISTANT_STUB_COMMAND_MAX_LENGTH);
           network_status_change.quantity = 0;
           strncpy(network_status_change.status,"disconnect",ASSISTANT_STUB_STATUS_MAX_LENGTH);
           strncpy(network_status_change.ssid,
                   _rSavedWlanAssociate.t_Ssid.ui1_aSsid,
                   NET_802_11_MAX_LEN_SSID);
           snprintf(network_status_change.bssid, ASSISTANT_STUB_BSSID_MAX_LENGTH, "%x:%x:%x:%x:%x:%x",
                    _rSavedWlanAssociate.t_Bssid[0], _rSavedWlanAssociate.t_Bssid[1], _rSavedWlanAssociate.t_Bssid[2],
                    _rSavedWlanAssociate.t_Bssid[3], _rSavedWlanAssociate.t_Bssid[4], _rSavedWlanAssociate.t_Bssid[5]);

           /*send  network_status cange  to sm */
           i4_ret = u_am_get_app_handle_from_name(&h_app, SM_THREAD_NAME);
           if(MISC_APPR_OK != i4_ret)
           {
               WIFI_MSG("<WIFI_SETTING_PROC> get handle fail!,i4_ret=%ld\n",i4_ret);
           }
           i4_ret = u_app_send_appmsg(h_app,
                                      E_APP_MSG_TYPE_MISC,
                                      MSG_FROM_WIFI_SETTING,
                                      ASSISTANT_STUB_CMD_NETWORK_STATUS_CHANGE,
                                      &network_status_change,
                                      sizeof(ASSISTANT_STUB_NETWORK_STATUS_CHANGE_T));
           if(0 == i4_ret)
           {
               WIFI_MSG("<WIFI_SETTING_PROC> u_app_send_appmsg success !!!\n");
           }
           else
           {
               WIFI_MSG("<WIFI_SETTING_PROC> u_app_send_appmsg fail !!!\n");
           }

        }

        if (NULL_HANDLE != t_g_this_obj.t_associate_timer.h_timer)//(NULL_HANDLE != h_timer_wlan)
        {
            //u_timer_stop(h_timer_wlan);
            u_timer_stop(t_g_this_obj.t_associate_timer.h_timer);
        }
        //_setup_wlan_prepare_to_scan();
    }
    else
    {
        WIFI_MSG("<WIFI_SETTING_PROC> ui4_wlan_timer_counte is %d\n",wlan_scan_connect_status.ui4_wlan_timer_counter);
    }

    WIFI_SETTING_PROC_FUNCTION_END;

    return;
}

VOID _setup_wlan_prepare_assoc_timer_cb(HANDLE_T pt_tm_handle, VOID*  pv_tag)
{
    WIFI_SETTING_PROC_FUNCTION_BEGIN;

    wlan_scan_connect_status.ui4_wlan_timer_counter--;
    WIFI_MSG("<WIFI_SETTING_PROC>  timer cb ui4_wlan_timer_counter %d %d\n",wlan_scan_connect_status.ui4_wlan_timer_counter,__LINE__);

    if (wlan_scan_connect_status.ui4_wlan_timer_counter == WLAN_TIMER_ASSOCIATE_TIMEOUT)
    {
        //Stop it first
        if (t_g_this_obj.t_associate_timer.h_timer != (HANDLE_T)NULL)//(h_timer_wlan != (HANDLE_T)NULL)
        {
            //u_timer_stop(h_timer_wlan);
            u_timer_stop(t_g_this_obj.t_associate_timer.h_timer);
        }

        WIFI_MSG("<WIFI_SETTING_PROC> Connect timeout!\n");
    }

    WIFI_SETTING_PROC_FUNCTION_END;

    return;
}

INT32 setup_wlan_try_connect_favorite_ap_as_dongle_in(VOID)
{
    WIFI_SETTING_PROC_FUNCTION_BEGIN;

    INT32 i4_ret = WIFI_OK;
    NET_802_11_ASSOCIATE_T _rSavedWlanAssociate={{0}};
    CHAR   dest_str[32] = {0};
    CHAR   pc_Key[65] = {0};  // The size of key must be equal to the size of ACFG seeting
    UINT32 ui4_ssid_len;

    i4_ret = a_network_get_favorite_ap(&_rSavedWlanAssociate);
    if (i4_ret) {
        wifi_setting_notify_bt_wifi_setup_result();
        #if CONFIG_SUPPORT_ALGO_WENZHI
        #if CONFIG_APP_SUPPORT_SMART_CONNECTION
            printf("<WIFI_SETTING> Auto to wifi_open_smart_connection\n");
            wifi_open_smart_connection(1);
        #endif
        #endif
        return i4_ret;
    }

    snprintf(dest_str,sizeof(dest_str), "%2x:%2x:%2x:%2x:%2x:%2x",
             _rSavedWlanAssociate.t_Bssid[0],
             _rSavedWlanAssociate.t_Bssid[1],
             _rSavedWlanAssociate.t_Bssid[2],
             _rSavedWlanAssociate.t_Bssid[3],
             _rSavedWlanAssociate.t_Bssid[4],
             _rSavedWlanAssociate.t_Bssid[5]);
    WIFI_MSG("<WIFI_SETTING_PROC> Bssid is : %s\n",dest_str);

    WIFI_MSG("<WIFI_SETTING_PROC> [Favorite]SSID:%s, SsidLen:%d, eAssocCase:%d, eAuthMode:%d, e_AuthCipher:%d Priority:%d \n",
           _rSavedWlanAssociate.t_Ssid.ui1_aSsid,
           _rSavedWlanAssociate.t_Ssid.ui4_SsidLen,
           _rSavedWlanAssociate.e_AssocCase,
           _rSavedWlanAssociate.e_AuthMode,
           _rSavedWlanAssociate.e_AuthCipher,
           _rSavedWlanAssociate.ui1_Priority);

    if(_rSavedWlanAssociate.t_Ssid.ui4_SsidLen > 0)
    {
        WIFI_MSG("<WIFI_SETTING_PROC> wlan_favorite_AP is found \n");

        ui4_ssid_len = _rSavedWlanAssociate.t_Ssid.ui4_SsidLen;

        //if(IEEE_802_11_AUTH_MODE_NONE != _rSavedWlanAssociate.e_AuthMode)
        {
            WIFI_MSG("<WIFI_SETTING_PROC> KeyIsAscii:%d\n",_rSavedWlanAssociate.t_Key.b_IsAscii);
            memset(pc_Key, 0, 65);
            i4_ret = a_cfg_get_wlan_key(pc_Key);
            _rSavedWlanAssociate.t_Key.pui1_PassPhrase = pc_Key;
            WIFI_MSG("<WIFI_SETTING_PROC> pc_Key is %s \n",pc_Key);

            i4_ret = a_cfg_get_wlan_key(_rSavedWlanAssociate.t_Key.pui1_PassPhrase);
            WIFI_CHK_FAIL(i4_ret,i4_ret);
            WIFI_MSG("<WIFI_SETTING_PROC> KEY:%s\n",_rSavedWlanAssociate.t_Key.pui1_PassPhrase);
        }

        wlan_scan_connect_status.ui1_ap_count = 1;
        wlan_scan_connect_status.ui1_wlan_notify_type = WLAN_NOTIFY_ASSOCIATE;
        setup_wlan_fm_owner = WLAN_FM_OWNER_MANUAL_ASSOCIATE_FAVORITE_AP;
        wlan_scan_connect_status._rCurWlanAssociate[0]=_rSavedWlanAssociate;
        wlan_scan_connect_status.ui4_wlan_timer_counter = WLAN_TIMER_ASSOCIATE_TIMEOUT;

        _rSavedWlanAssociate.e_AssocCase = IEEE_802_11_MANUAL_ASSOC;//force set to manual

        i4_ret = c_net_wlan_associate(&_rSavedWlanAssociate, _wifi_setting_wlan_notify_callback);
        if(NET_WLAN_OK != i4_ret)
        {
            if(wlan_scan_connect_status.ui4_wlan_timer_counter > 3)
            {
                wlan_scan_connect_status.ui4_wlan_timer_counter = 3;
            }
            WIFI_MSG("<WIFI_SETTING_PROC> [dongle-in]wlan associate fail.\n");
        }

        i4_ret = _setup_wlan_associate_start_timer();

        WIFI_SETTING_PROC_FUNCTION_END;

        WIFI_CHK_FAIL(i4_ret, i4_ret);

    }

    WIFI_SETTING_PROC_FUNCTION_END;

    return i4_ret;
}

static INT32 transfer_time(struct timespec *abstime, unsigned int ms)
{
    int i4_ret;
    i4_ret = clock_gettime(CLOCK_MONOTONIC, abstime);
    if (i4_ret)
    {
        WIFI_ERR("clock_gettime failed!\n");
        return WIFI_FAIL;
    }
    abstime->tv_sec  += ms / 1000;
    ms %= 1000;
    abstime->tv_nsec += ms * 1000000;
    if (abstime->tv_nsec >= 1000000000)
    {
        abstime->tv_nsec -= 1000000000;
        abstime->tv_sec++;
    }
    return WIFI_OK;
}

VOID convert_encoding_format(const CHAR* source, CHAR* result)
{
    iconv_t cd = iconv_open("GBK", "UTF-8");   //convert UTF-8 to GBK
    if((iconv_t)(-1) == cd)
    {
       WIFI_MSG("<WIFI_SETTING_PROC>Failed to get encoding conversion handle!\n");
       return;
    }
    size_t source_len = strlen(source);
    size_t result_len = NET_802_11_MAX_LEN_SSID;
    if(-1 == iconv(cd, &source, &source_len, &result, &result_len))
    {
        iconv_close(cd);
        WIFI_MSG("<WIFI_SETTING_PROC>Failed to get encoding conversion handle!\n");
        return;
    }
    iconv_close(cd); 
    return;
}

//API: convert a string to a hexadecimal string
VOID str_to_strhex(const CHAR* byte, CHAR* strhex, int byte_len)
{
    char highbyte, lowbyte;
    for(int i = 0; i < byte_len; i++)
    {
        highbyte = byte[i]>>4;
        lowbyte = byte[i]&0x0f;

        highbyte += 0x30;

        if(highbyte > 0x39)
        {
            strhex[i * 2] = highbyte + 0x07;
        }
        else
        {
            strhex[i * 2] = highbyte;
        }

        lowbyte += 0x30;
        if(lowbyte > 0x39)
        {
            strhex[i * 2 + 1] = lowbyte + 0x07;
        }
        else
        {
            strhex[i * 2 + 1] = lowbyte;
        }
    }
    return;
}

static INT32 start_scan(void) {
    if (FALSE == g_is_scan_enable) {
        g_is_scan_enable = TRUE;

        char scan_ssid[NET_802_11_MAX_LEN_SCAN_SSID + 1];
        memset(scan_ssid, 0, sizeof(scan_ssid));
        str_to_strhex(wifi_ConnectSsid, scan_ssid, strlen(wifi_ConnectSsid));
        WIFI_MSG("<WIFI_SETTING_PROC>wifi_ConnectSsid:%s strlen(wifi_ConnectSsid):%d\n", wifi_ConnectSsid, strlen(wifi_ConnectSsid));
        WIFI_MSG("<WIFI_SETTING_PROC>scan_ssid:%s strlen(scan_ssid):%d\n", scan_ssid, strlen(scan_ssid));
        
        //INT32 ret = c_net_wlan_scan(_wifi_setting_wlan_notify_callback);
        INT32 ret = c_net_wlan_scan_with_ssid(_wifi_setting_wlan_notify_callback, scan_ssid, strlen(scan_ssid));
        if (ret) {
            g_is_scan_enable = FALSE;
            WIFI_ERR("c_net_wlan_scan_with_ssid failed, ret:[%d]\n", ret);
            return WIFI_FAIL;
        }
    }
    return WIFI_OK;
}

static INT32 get_result_by_scan_once(void) {
    INT32 ret;
    pthread_mutex_lock(&t_g_this_obj.t_mutex);

    if (t_g_this_obj.b_connect_over) {
        ret = WIFI_FAIL;
        WIFI_MSG("connect over by stub!\n");
        goto EXIT;
    }

    if (start_scan()) {
        goto EXIT;
    }

    struct timespec abstime;
    ret = transfer_time(&abstime, 10000);
    if (ret) {
        WIFI_ERR("_transfer_time failed, ret:[%d]\n", ret);
        goto EXIT;
    }

    ret = pthread_cond_timedwait(&t_g_this_obj.t_cond, &t_g_this_obj.t_mutex, &abstime);
    if (ret) {
        if (ret == ETIMEDOUT) {
            WIFI_ERR("scan time out!\n");
        } else {
            WIFI_ERR("pthread_cond_timedwait failed, ret:[%d]\n", ret);
        }
        goto EXIT;
    }

    if (SCAN_AP_STAT_OK == g_scan_ap_ok) {
        ret = WIFI_OK;
    }

EXIT:
    pthread_mutex_unlock(&t_g_this_obj.t_mutex);
    return ret;
}

static INT32 associate_ap_by_specify(void) {
    INT32 ret = setup_goto_manual_associate(wifi_ConnectSsid, wifi_ConnectBssid, wifi_ConnectPwd, wifi_AuthMode);
    if(ret != 0) {
        g_scan_ap_ok=CONNECT_AP_STAT_FAILE;
        WIFI_MSG("<SMART CONNECTION>CONNECT BY WIFI IS FAILE!!!\n");
        return WIFI_FAIL;
    } else {
        g_scan_ap_ok=CONNECT_AP_STAT_OK;
        return WIFI_OK;
    }
}

static INT32 associate_ap_by_scan(void) {
    INT32 ret;
    INT32 scan_count = 0;

    while (1) {
        if (t_g_this_obj.b_connect_over) {
            return WIFI_FAIL;
        }

        if (scan_count++ >= 3) {
            break;
        }

        ret = get_result_by_scan_once();
        if (ret) {
            continue;
        }

        return associate_ap_by_specify();
    }

    upload_scan_ap_fail();
    WIFI_MSG("scan ap failed!\n");

    return WIFI_FAIL;
}

static INT32 wifi_smart_connection_get_specified_AP_info(void)
{
    INT32 ret;

    if (-1 == wifi_AuthMode) {
        ret = associate_ap_by_scan();
    } else {
        ret = associate_ap_by_specify();

    }
    return ret;
}

void wifi_connect_over(void) {
    WIFI_SETTING_PROC_FUNCTION_BEGIN;

    if (0 == t_g_this_obj.t_thread) {
        return;
    }

    pthread_mutex_lock(&t_g_this_obj.t_mutex);

    t_g_this_obj.b_connect_over = TRUE;
    c_net_wlan_disassociate();

    pthread_cond_broadcast(&t_g_this_obj.t_cond);

    pthread_mutex_unlock(&t_g_this_obj.t_mutex);

    pthread_join(t_g_this_obj.t_thread, NULL);

    t_g_this_obj.t_thread = 0;

    WIFI_SETTING_PROC_FUNCTION_END;
}

void *wifi_connect_thread(void *arg) {
    if (-1 == wifi_AuthMode) {
        associate_ap_by_scan();
    } else {
        associate_ap_by_specify();

    }
    return NULL;
}

INT32 start_wifi_connect(void) {
    if (t_g_this_obj.t_thread) {
        wifi_connect_over();
    }

    t_g_this_obj.b_connect_over = FALSE;

    INT32 ret = pthread_create(&t_g_this_obj.t_thread, NULL, wifi_connect_thread, NULL);
    if (ret) {
        WIFI_ERR("connect thread create failed:[%d]\n", ret);
        t_g_this_obj.t_thread = 0;
        return WIFI_FAIL;
    }

    return WIFI_OK;
}

void handle_assistant_stub_get_ap_list(ASSISTANT_STUB_GET_AP_LIST_T * cmd)
{
    WIFI_SETTING_PROC_FUNCTION_BEGIN;

    g_scan_ap_id = cmd->id;
    g_is_upload_ap_list = TRUE;

    start_scan();

    WIFI_SETTING_PROC_FUNCTION_END;
}

void handle_assistant_stub_wifi_connect_msg(ASSISTANT_STUB_WIFI_CONNECT_T * wifi_connect)
{
    WIFI_SETTING_PROC_FUNCTION_BEGIN;

	WIFI_MSG("wifi_connect->ssid is %s\n",wifi_connect->ssid);
	WIFI_MSG("wifi_connect->bssid is %s\n",wifi_connect->bssid);
	WIFI_MSG("wifi_connect->password is %s\n",wifi_connect->password);
	WIFI_MSG("wifi_connect->auth_mode is %d\n", wifi_connect->auth_mode);
	WIFI_MSG("wifi_connect->id is %d\n",wifi_connect->id);

    strncpy(wifi_ConnectSsid,wifi_connect->ssid,NET_802_11_MAX_LEN_SSID);
    strncpy(wifi_ConnectPwd,wifi_connect->password,WLAN_KEY_MAX_LEN);
	if(*(wifi_connect->bssid))
	{
		strncpy(wifi_ConnectBssid, wifi_connect->bssid, NET_802_11_MAX_LEN_BSSID);
	}
	else
	{
		strncpy(wifi_ConnectBssid, "FF:FF:FF:FF:FF:FF", NET_802_11_MAX_LEN_BSSID);
	}

    switch (wifi_connect->auth_mode) {
        case AUTH_MODE_NONE:
            wifi_AuthMode = IEEE_802_11_AUTH_MODE_NONE;
            break;
        case AUTH_MODE_WEP:
            wifi_AuthMode = IEEE_802_11_AUTH_MODE_WEPAUTO;
            break;
        case AUTH_MODE_WPA:
            wifi_AuthMode = IEEE_802_11_AUTH_MODE_WPAPSK_OR_WPA2PSK;
            break;
        default:
            wifi_AuthMode = -1;
            break;
    }

    fg_wifi_connect_id = wifi_connect->id;

    start_wifi_connect();

    WIFI_SETTING_PROC_FUNCTION_END;
}

void handle_assistant_stub_wifi_connect_over_msg(ASSISTANT_STUB_WIFI_CONNECT_OVER_T * cmd)
{
    WIFI_SETTING_PROC_FUNCTION_BEGIN;
    wifi_connect_over();
    WIFI_SETTING_PROC_FUNCTION_END;
}

INT32 handle_assistant_stub_wifi_setup_result_msg(ASSISTANT_STUB_WIFI_SETUP_RESULT_T * wifi_setup_result)
{
    WIFI_SETTING_PROC_FUNCTION_BEGIN;

    WIFI_MSG("<WIFI_SETTING_PROC> wifi_setup_result->command is %s\n",wifi_setup_result->command);
    WIFI_MSG("<WIFI_SETTING_PROC> wifi_setup_result->result is %d\n",wifi_setup_result->result);

    INT32 i4_ret = 0;
    HANDLE_T h_app = NULL_HANDLE;
    ASSISTANT_STUB_WIFI_SETUP_RESULT_T wifi_config_result = {0};

    fg_wifi_setup_status = FALSE;
    WIFI_MSG("<WIFI_SETTING_PROC> fg_wifi_setup_status is %d\n",fg_wifi_setup_status);

#if 1 /*remove to wifi_setup_result process*/
    /*send wifi_setup_result to bluetooth */
    strncpy(wifi_config_result.command,wifi_setup_result->command,ASSISTANT_STUB_COMMAND_MAX_LENGTH);
    wifi_config_result.result = wifi_setup_result->result;
    i4_ret = u_am_get_app_handle_from_name(&h_app, BLUETOOTH_THREAD_NAME);
    if(0 != i4_ret)
    {
        WIFI_MSG("<WIFI_SETTING_PROC> get handle fail!,i4_ret=%ld\n",i4_ret);
    }
    i4_ret = u_app_send_appmsg(h_app,
                               E_APP_MSG_TYPE_WIFI_SETTING,
                               MSG_FROM_WIFI_SETTING,
                               ASSISTANT_STUB_CMD_WIFI_SETUP_RESULT,
                               &wifi_config_result,
                               sizeof(ASSISTANT_STUB_WIFI_SETUP_RESULT_T));
    if(0 == i4_ret)
    {
        WIFI_MSG("<WIFI_SETTING_PROC> u_app_send_appmsg success !!!\n");
    }
    else
    {
        WIFI_MSG("<WIFI_SETTING_PROC> u_app_send_appmsg fail !!!\n");
    }

    /*send wifi_setup_result to sm */
    i4_ret = u_am_get_app_handle_from_name(&h_app, SM_THREAD_NAME);
    if(0 != i4_ret)
    {
        WIFI_MSG("<WIFI_SETTING_PROC> get handle fail!,i4_ret=%ld\n",i4_ret);
    }
    i4_ret = u_app_send_appmsg(h_app,
                               E_APP_MSG_TYPE_WIFI_SETTING,
                               MSG_FROM_WIFI_SETTING,
                               ASSISTANT_STUB_CMD_WIFI_SETUP_RESULT,
                               &wifi_config_result,
                               sizeof(ASSISTANT_STUB_WIFI_SETUP_RESULT_T));
    if(0 == i4_ret)
    {
        WIFI_MSG("<WIFI_SETTING_PROC> u_app_send_appmsg success !!!\n");
    }
    else
    {
        WIFI_MSG("<WIFI_SETTING_PROC> u_app_send_appmsg fail !!!\n");
    }
#endif

    if(0 ==  wifi_setup_result->result)
    {
        WIFI_MSG("<WIFI_SETTING_PROC> assistant wifi setup is OK !!!\n");
    }
    else
    {
        WIFI_MSG("<WIFI_SETTING_PROC> assistant wifi setup is FAIL ,reconnect to favoriate ap !!!\n");
        setup_wlan_try_connect_favorite_ap_as_dongle_in( );
    }

    WIFI_SETTING_PROC_FUNCTION_END;

    return 0;
}

INT32 _wifi_setting_handle_assistant_stub_msg(APPMSG_T * pt_msg)
{
    WIFI_SETTING_PROC_FUNCTION_BEGIN;

    switch(pt_msg->ui4_msg_type)
    {
        case ASSISTANT_STUB_CMD_GET_AP_LIST:
            handle_assistant_stub_get_ap_list(pt_msg->p_usr_msg);
            break;
        case ASSISTANT_STUB_CMD_WIFI_CONNECT:
            handle_assistant_stub_wifi_connect_msg(pt_msg->p_usr_msg);
            break;
        case ASSISTANT_STUB_CMD_WIFI_CONNECT_OVER:
            handle_assistant_stub_wifi_connect_over_msg(pt_msg->p_usr_msg);
            break;
        case ASSISTANT_STUB_CMD_WIFI_SETUP_RESULT:
            handle_assistant_stub_wifi_setup_result_msg(pt_msg->p_usr_msg);
            break;
        default:
            break;
    }

    WIFI_SETTING_PROC_FUNCTION_END;

    return 0;
}

#if CONFIG_APP_SUPPORT_SMART_CONNECTION
//Now this function has not been used
INT32 wifi_smart_connection_open(VOID)
{
    //OPEN UP ELIAN
    system("iwpriv wlan0 elian start");
    WIFI_MSG("Sys cmd is : iwpriv wlan0 elian start\n");

    return WIFI_OK;
}

//Now this function has not been used
INT32 wifi_smart_connection_close(VOID)
{
    //CLOSE UP ELIAN
    system("iwpriv wlan0 elian stop");
    WIFI_MSG("Sys cmd is : iwpriv wlan0 elian stop\n");

    return WIFI_OK;
}

//Now this function has not been used
INT32 wifi_smart_connection_clear_result(VOID)
{
    //CLEAR ELIAN DATA
    system("iwpriv wlan0 elian clear");
    WIFI_MSG("Sys cmd is : iwpriv wlan0 elian clear\n");

    return WIFI_OK;
}

INT32 wifi_smart_connection_wlan0_connect_ap(VOID)
{
    system("ifconfig wlan0 0.0.0.0");
    WIFI_MSG("Sys cmd is : ifconfig wlan0 0.0.0.0\n");

    //scan ap for setting WIFI infomation
    return start_wifi_connect();
}

VOID wifi_smart_connection_judge_wifi_AuthMode(CHAR* ptr)
{
    if(0 == strncmp(ptr, "NONE", strlen("NONE")))
    {
        wifi_AuthMode = IEEE_802_11_AUTH_MODE_NONE;
    }
    else if(0 == strncmp(ptr, "WEP", strlen("WEP")))
    {
        wifi_AuthMode = IEEE_802_11_AUTH_MODE_WEPAUTO;
    }
    else if(0 == strncmp(ptr, "WPA", strlen("WPA")))
    {
        wifi_AuthMode = IEEE_802_11_AUTH_MODE_WPAPSK_OR_WPA2PSK;
    }
    else
    {
        wifi_AuthMode = -1;
    }
    WIFI_MSG("<SMART CONNECTION>wifi_AuthMode = %d\n", wifi_AuthMode);
}

//Now this function has not been used
static INT32 wifi_smart_connection_get_result_and_setting_network(VOID)
{
    SMART_CONNECT_MESH_INFO p_mesh_info;
    char* ptr=NULL;
    char  smart_buffer[STCN_DRV_DATA_BUFFER_LEN] = {0};
    char* temp=NULL;
    int data_len=0;
    int connection_type=0;//1-wifi 2-wired 3-mesh
    int i_loop=0;
    int i_timer=0;
    char error_ssid[NET_802_11_MAX_LEN_SSID+1]={0};
    char error_pwd[WLAN_KEY_MAX_LEN+1]={0};
    char c_mesh_rcv[32]={0};

    c_net_wlan_disassociate();

    //open smart connection
    wifi_smart_connection_open();

    //be sure to get elian data success
    while(1)
    {
        if (i_timer > 180) {
            //stop elian and clear result
            wifi_smart_connection_close();
            wifi_smart_connection_clear_result();
            goto FAIL;
        }
        memset(g_ap_info.ap_name,0,NET_802_11_MAX_LEN_SSID+1);
        memset(g_ap_info.ap_password,0,WLAN_KEY_MAX_LEN);

        //for stop elian detect
        if(fg_stop_smart_connection)
        {
            fg_stop_smart_connection=FALSE;
            g_is_start_smart_connection=FALSE;
            wifi_smart_connection_close();
            wifi_smart_connection_clear_result();
            return WIFI_OK;
        }

        //stop wifi timer
        if((t_g_this_obj.t_wlan_timer.h_timer!= (HANDLE_T)NULL))
        {
            u_timer_stop(t_g_this_obj.t_wlan_timer.h_timer);
        }

        u_thread_delay(1000);

        //GET ELIAN DATA
        system("iwpriv wlan0 elian result > /tmp/aud_base/elian_result.tmp");
        //WIFI_MSG("Sys cmd is : iwpriv wlan0 elian result > /tmp/aud_base/elian_result.tmp\n");

        //GET AP INFO OR MESH INFO FROM ELIAN DATA
        FILE *fp = fopen("/tmp/aud_base/elian_result.tmp", "r");
        if(fp == NULL)
        {
            WIFI_MSG("<SMART CONNECTION>ERR:%s:%d: elian_result.txt open failed\n",__FUNCTION__, __LINE__);
            return WIFI_FAIL;

        }
        fseek(fp, 0L, SEEK_END);
        int i_file_size = ftell(fp);
        fseek(fp, 0L, SEEK_SET);

        memset(smart_buffer, 0, STCN_DRV_DATA_BUFFER_LEN);
        fread(smart_buffer, i_file_size, 1, fp);

        fclose(fp);
        fp = NULL;

        WIFI_MSG("elian result:[%s]\n", smart_buffer);

        temp = smart_buffer;

        if ((ptr = strstr(temp, "ssid=")) != NULL)
        {
            ptr += strlen("ssid=");
            temp=ptr;
            if((*ptr) == ',')
            {
                WIFI_MSG("ssid is null!\n");
                i_timer++;
            }
            else
            {
                //GET AP SSID
                i_loop=0;
                while(i_loop<NET_802_11_MAX_LEN_SSID)
                {
                    if(strncmp(ptr + i_loop,", pwd=",strlen(", pwd=")) == 0)
                    {
                        temp=ptr + i_loop;
                        break;
                    }

                    g_ap_info.ap_name[i_loop]=*(ptr + i_loop);
                    i_loop++;
                }
                g_ap_info.ap_name[i_loop]='\0';

                strncpy(wifi_ConnectSsid,
                          g_ap_info.ap_name,
                          NET_802_11_MAX_LEN_SSID+1);
                wifi_ConnectSsid[NET_802_11_MAX_LEN_SSID]='\0';

                if ((ptr = strstr(temp,"pwd=")) != NULL)
                {
                    ptr += strlen("pwd=");
                    temp=ptr;
                    //GET AP PWD
                    i_loop=0;
                    while(i_loop<WLAN_KEY_MAX_LENGTH)
                    {
                        if(strncmp(ptr + i_loop,", user=",strlen(", user=")) == 0)
                        {
                            temp=ptr + i_loop;
                            break;
                        }

                        g_ap_info.ap_password[i_loop]=*(ptr + i_loop);
                        i_loop++;
                    }
                    g_ap_info.ap_password[i_loop]='\0';

                    strncpy(wifi_ConnectPwd,
                              g_ap_info.ap_password,
                              WLAN_KEY_MAX_LENGTH+1);
                    wifi_ConnectPwd[WLAN_KEY_MAX_LENGTH]='\0';

                    if ((ptr = strstr(temp, "cust_data_len=")) != NULL)
                    {
                        ptr += strlen("cust_data_len=");
                        temp=ptr;

                        //GET DATA LENGTH
                        data_len=atoi(ptr);
                        WIFI_MSG("cust_data_len=%d!\n",data_len);

                        if(data_len != 0)
                        {
                            if ((ptr = strstr(temp, "cust_data=")) != NULL)
                            {
                                ptr += strlen("cust_data=");
                                temp=ptr;
                                //WIFI_MSG("cust_data=%s!\n",ptr);
                                wifi_smart_connection_judge_wifi_AuthMode(ptr);
                                WIFI_MSG("cust_data=");
                                while((*ptr != '/') && (*ptr != NULL))
                                {
                                    printf("%c",*ptr);
                                    ptr++;
                                }
                                printf("!\n");
                                
                                //get AP bssid
                                if((ptr == strstr(temp, "/")) != NULL)
                                {
                                    ptr += strlen("/");
                                    temp=ptr; 
                                    i_loop=0;
                                    while(i_loop<NET_802_11_MAX_LEN_BSSID)
                                    {
                                        if(strncmp(ptr + i_loop,",",strlen(",")) == 0)
                                        {
                                            temp=ptr + i_loop;
                                            break;
                                        }

                                        g_ap_info.app_addr[i_loop]=*(ptr + i_loop);
                                        i_loop++;
                                    }
                                    g_ap_info.app_addr[i_loop]='\0';

                                }
                                strncpy(wifi_ConnectBssid,
                                          g_ap_info.app_addr,
                                          NET_802_11_MAX_LEN_BSSID+1);
                                wifi_ConnectBssid[NET_802_11_MAX_LEN_BSSID]='\0';
                                WIFI_MSG("wifi_ConnectBssid=%s\n",wifi_ConnectBssid);
                                ptr = temp;

                /*
                                    //get app ip and port
                                    if((ptr = strstr(temp, "(IP)")) != NULL)
                                    {
                                        memset(g_ap_info.app_addr,0,SETUP_WIFI_APP_ADDR_LEN);
                                        ptr=ptr+4;//cut (APP)
                                        i_loop=0;
                                        while((*(ptr) != '(')&&(i_loop<SETUP_WIFI_APP_ADDR_LEN-1))
                                        {
                                            g_ap_info.app_addr[i_loop]=*(ptr);
                                            ptr++;
                                            i_loop++;
                                        }
                                        g_ap_info.app_addr[i_loop]='\0';
                                        WIFI_MSG("(APP)=%s!!!\n",g_ap_info.app_addr);
                                    }
                                    else
                                    {
                                        if(*temp != '(')
                                        {
                                            memset(g_ap_info.app_addr,0,SETUP_WIFI_APP_ADDR_LEN);
                                            i_loop=0;
                                            ptr=temp;
                                            while((*(ptr) != '(')&&(i_loop<SETUP_WIFI_APP_ADDR_LEN-1))
                                            {
                                                g_ap_info.app_addr[i_loop]=*(ptr);
                                                ptr++;
                                                i_loop++;
                                            }
                                            g_ap_info.app_addr[i_loop]='\0';
                                            WIFI_MSG("(APP)=%s!!!\n",g_ap_info.app_addr);
                                        }
                                    }
                                    */


                                if((ptr = strstr(temp, "(AP)")) != NULL)
                                {
                                    WIFI_MSG("(AP)ssid=%s;pwd=%s;cust_data=%s!\n",g_ap_info.ap_name,g_ap_info.ap_password,ptr);
                                    if(strncmp(g_ap_info.ap_name,error_ssid,strlen(g_ap_info.ap_name)) == 0 &&
                                       strncmp(g_ap_info.ap_password,error_pwd,strlen(g_ap_info.ap_password)) == 0)
                                    {
                                        wifi_smart_connection_clear_result();
                                        memset(error_ssid,0,NET_802_11_MAX_LEN_SSID+1);
                                        memset(error_pwd,0,WLAN_KEY_MAX_LENGTH+1);
                                        WIFI_MSG("this is same error ap info!!!!!!\n");
                                        WIFI_MSG("THIS IS WIFI KEY THAT MAYBE INCORECCT!\n");
                                        break;
                                    }
                                    else
                                    {
                                        connection_type=1;//wifi connection
                                        break;
                                    }
                                }

                                else if((ptr = strstr(temp, "(WD)")) != NULL)
                                {
                                    WIFI_MSG("(WD)ssid=%s;pwd=%s;cust_data=%s!\n",g_ap_info.ap_name,g_ap_info.ap_password,ptr);
                                    connection_type=2;//wirled connection
                                    break;
                                }

                                else if((ptr = strstr(temp, "(MS)")) != NULL)
                                {
                                    WIFI_MSG("(MS)ssid=%s;pwd=%s;cust_data=%s!\n",g_ap_info.ap_name,g_ap_info.ap_password,ptr);
                                    connection_type=3;//mesh connection

                                    ptr=ptr+4;//cut (MS)

                                    //GET MESH INFO
                                    i_loop=0;
                                    while(*(ptr) != ',')
                                    {
                                        p_mesh_info.server_ip[i_loop]=*(ptr);
                                        ptr++;
                                        i_loop++;
                                        if(*(ptr) == '/')
                                        {
                                            break;
                                        }
                                    }
                                    p_mesh_info.server_ip[i_loop]='\0';

                                    ptr++;
                                    p_mesh_info.mesh_chanel=atoi(ptr);
                                    while(*(ptr) != ',')
                                    {
                                        ptr++;
                                        if(*(ptr) == '/')
                                        {
                                            break;
                                        }
                                    }

                                    i_loop=0;
                                    ptr++;
                                    memset(c_mesh_rcv,0,32);
                                    while(*(ptr) != ',')
                                    {
                                        c_mesh_rcv[i_loop]=*(ptr);
                                        ptr++;
                                        i_loop++;
                                        if(*(ptr) == '/')
                                        {
                                            break;
                                        }
                                    }
                                    c_mesh_rcv[i_loop]='\0';

                                    snprintf(p_mesh_info.mesh_id,32,"base-mesh-%s",c_mesh_rcv);
                                    p_mesh_info.mesh_id[31]='\0';
                                    snprintf(p_mesh_info.mesh_key,32,"base-mesh-key-%s",c_mesh_rcv);
                                    p_mesh_info.mesh_key[31]='\0';

                                    WIFI_MSG("server_ip=%s;mesh_channel=%d;mesh_id=%s;mesh_key=%s!\n",p_mesh_info.server_ip,p_mesh_info.mesh_chanel,p_mesh_info.mesh_id,p_mesh_info.mesh_key);
                                    break;
                                }

                                else
                                {
                                    WIFI_MSG("(FOR TEST)ssid=%s;pwd=%s;cust_data=%s!\n",g_ap_info.ap_name,g_ap_info.ap_password,g_ap_info.app_addr);
                                    if(strncmp(g_ap_info.ap_name,error_ssid,strlen(g_ap_info.ap_name)) == 0 &&
                                       strncmp(g_ap_info.ap_password,error_pwd,strlen(g_ap_info.ap_password)) == 0)
                                    {
                                        WIFI_MSG("<SMART CONNECTION>this is same error ap info!!!!!!\n");
                                    }
                                    else
                                    {
                                        connection_type=1;//wifi connection
                                        break;
                                    }
                                }

                            }
                        }
                    }
                }
            }
        }
    }

    //stop elian and clear result
    wifi_smart_connection_close();
    wifi_smart_connection_clear_result();

    if (connection_type == 1) { //connect ap
        //wifi_AuthMode = -1;
        if(wifi_smart_connection_wlan0_connect_ap() == WIFI_OK) {
            WIFI_MSG("<SMART CONNECTION>elian setting success!\n");
            return WIFI_OK;
        } else {
            goto FAIL;
        }
    } else if(connection_type == 2) { //conect wirled
        WIFI_MSG("[1]kingfisher do not need it ,we have network auto switch!");
    } else if(connection_type == 3) {//conect mesh
        WIFI_MSG("[1]kingfisher mesh related pending  !");
    }

FAIL:
    WIFI_MSG("<SMART CONNECTION>elian setting failed!\n");
    return WIFI_FAIL;
}

//Now this function has not been used
void wifi_smart_connection_open_thread(void *arg)
{
    WIFI_MSG("<SMART CONNECTION>smart connection thread start!\n");
    g_smart_conncetion_thread_is_run = TRUE;

    wifi_smart_connection_get_result_and_setting_network();

    g_is_start_smart_connection = FALSE;
    g_smart_conncetion_thread_is_run = FALSE;
    WIFI_MSG("<SMART CONNECTION>smart connection thread exit!\n");

    u_thread_exit();
}

//Now this function has not been used
signed int wifi_init_smart_connection_database(void)
{
    signed int i4_ret;

    WIFI_MSG("<SMART CONNECTION>wifi_init_smart_connection_database!!!\n");

    i4_ret = system("mkdir -p /tmp/aud_base");
    WIFI_CHK_FAIL(i4_ret,i4_ret);

    i4_ret = system("chmod -R 777 /tmp/aud_base");
    WIFI_CHK_FAIL(i4_ret,i4_ret);

    return WIFI_OK;
}

void wifi_smart_connection_event_callback(wifi_smart_connection_event_t event, void *data)
{
    uint8_t passwd[64 + 1] = {0};
    uint8_t ssid[32 + 1] = {0};
    uint8_t custom[64 + 1] = {0};
    uint8_t ssid_len = 0;
    uint8_t passwd_len = 0;
    uint8_t custom_len = 0;
    char* ptr = NULL;
    int i_loop = 0, i4_ret = 0;
    HANDLE_T h_app = NULL_HANDLE;

    printf("smtcn_evt_handler event=%d\n", event);

    switch (event) {
    case WIFI_SMART_CONNECTION_EVENT_CHANNEL_LOCKED:
        break;
    case WIFI_SMART_CONNECTION_EVENT_TIMEOUT:
        break;
    case WIFI_SMART_CONNECTION_EVENT_INFO_COLLECTED:
        WIFI_MSG("<SMART CONNECTION> WIFI_SMART_CONNECTION_EVENT_INFO_COLLECTED\n");
        
        ASSISTANT_STUB_NETWORK_STATUS_CHANGE_T network_status_change = {0};
        strncpy(network_status_change.command,"/system/network_status_change",ASSISTANT_STUB_COMMAND_MAX_LENGTH);
        network_status_change.quantity = 0;
        strncpy(network_status_change.status,"Password",ASSISTANT_STUB_STATUS_MAX_LENGTH);
        /*send  network_status cange  to sm */
        i4_ret = u_am_get_app_handle_from_name(&h_app, SM_THREAD_NAME);
        if(MISC_APPR_OK != i4_ret)
        {
            WIFI_MSG("<WIFI_SETTING_PROC> get handle fail!,i4_ret=%ld\n",i4_ret);
        }
        i4_ret = u_app_send_appmsg(h_app,
                                   E_APP_MSG_TYPE_MISC,
                                   MSG_FROM_WIFI_SETTING,
                                   ASSISTANT_STUB_CMD_NETWORK_STATUS_CHANGE,
                                   &network_status_change,
                                   sizeof(ASSISTANT_STUB_NETWORK_STATUS_CHANGE_T));
        if(0 == i4_ret)
        {
            WIFI_MSG("<WIFI_SETTING_PROC> u_app_send_appmsg success !!!\n");
        }
        else
        {
            WIFI_MSG("<WIFI_SETTING_PROC> u_app_send_appmsg fail !!!\n");
        }
        
        if(g_check_callback == FALSE)
        {
            break;
        }
        wifi_smart_connection_get_result(ssid, &ssid_len, passwd, &passwd_len, custom, &custom_len);
        WIFI_MSG("<SMART CONNECTION> ssid[%d]=%s, psk[%d]=%s, cust[%d]=%s\n",
            ssid_len, (char*)ssid, passwd_len, (char*)passwd, custom_len, (char*)custom);
        wifi_smart_connection_stop();
        wifi_smart_connection_deinit();

        //get AP ssid
        strncpy(wifi_ConnectSsid,ssid,NET_802_11_MAX_LEN_SSID+1);
        wifi_ConnectSsid[NET_802_11_MAX_LEN_SSID]='\0';
        WIFI_MSG("<SMART CONNECTION> wifi_ConnectSsid = %s\n",wifi_ConnectSsid);

        //get AP password
        strncpy(wifi_ConnectPwd,passwd,WLAN_KEY_MAX_LENGTH+1);
        wifi_ConnectPwd[WLAN_KEY_MAX_LENGTH]='\0';
        WIFI_MSG("<SMART CONNECTION> wifi_ConnectPwd = %s\n",wifi_ConnectPwd);

        //get AP authmode
        ptr = custom;
        wifi_smart_connection_judge_wifi_AuthMode(ptr);
        while((*ptr != '/') && (*ptr != NULL))
        {
            ptr++;
        }

        //get AP bssid
        if((ptr == strstr(custom, "/")) != NULL)
        {
            ptr += strlen("/");
            i_loop=0;
            while(ptr != NULL && i_loop < NET_802_11_MAX_LEN_BSSID)
            {
                wifi_ConnectBssid[i_loop]=*(ptr + i_loop);
                i_loop++;
            }
            wifi_ConnectBssid[i_loop]='\0';

        }
        ptr = NULL;
        WIFI_MSG("<SMART CONNECTION> wifi_ConnectBssid = %s\n",wifi_ConnectBssid); 

        g_check_callback = FALSE;
        break;
    default:
        break;
    }
}

VOID wifi_configure_station_connect(CONNECT_AP_INFO *wifi_connect)
{
    WIFI_SETTING_PROC_FUNCTION_BEGIN;

    WIFI_MSG(" wifi_connect->ssid is %s",wifi_connect->ap_name);
    WIFI_MSG(" wifi_connect->password is %s",wifi_connect->ap_password);
    WIFI_MSG(" wifi_connect->auth_mode is %d", wifi_connect->ap_authmode);

    strncpy(wifi_ConnectSsid,wifi_connect->ap_name,NET_802_11_MAX_LEN_SSID);
    strncpy(wifi_ConnectPwd,wifi_connect->ap_password,WLAN_KEY_MAX_LEN);
    strncpy(wifi_ConnectBssid, "FF:FF:FF:FF:FF:FF", NET_802_11_MAX_LEN_BSSID);

    switch (wifi_connect->ap_authmode) {
        case AUTH_MODE_NONE:
            wifi_AuthMode = IEEE_802_11_AUTH_MODE_NONE;
            break;
        case AUTH_MODE_WEP:
            wifi_AuthMode = IEEE_802_11_AUTH_MODE_WEPAUTO;
            break;
        case AUTH_MODE_WPA:
            wifi_AuthMode = IEEE_802_11_AUTH_MODE_WPAPSK_OR_WPA2PSK;
            break;
        default:
            wifi_AuthMode = -1;
            break;
    }

    fg_wifi_connect_id = 0;

    start_wifi_connect();

    WIFI_SETTING_PROC_FUNCTION_END;
}

VOID wifi_configure_station_disconnect(VOID)
{
    WIFI_SETTING_PROC_FUNCTION_BEGIN;
    wifi_connect_over();
    WIFI_SETTING_PROC_FUNCTION_END;
}

INT32 wifi_start_smart_connection(VOID)
{
    int i4_ret;

    WIFI_MSG("<SMART CONNECTION>wifi_start_smart_connection!\n");

    if(g_is_start_smart_connection == FALSE)
    {
        HANDLE_T h_app = NULL_HANDLE;
        ASSISTANT_STUB_NETWORK_STATUS_CHANGE_T network_status_change = {0};
        strncpy(network_status_change.command,"/system/network_status_change",ASSISTANT_STUB_COMMAND_MAX_LENGTH);
        network_status_change.quantity = 0;
        strncpy(network_status_change.status,"Elian",ASSISTANT_STUB_STATUS_MAX_LENGTH);
        /*send  network_status cange  to sm */
        i4_ret = u_am_get_app_handle_from_name(&h_app, SM_THREAD_NAME);
        if(MISC_APPR_OK != i4_ret)
        {
            WIFI_MSG("<WIFI_SETTING_PROC> get handle fail!,i4_ret=%ld\n",i4_ret);
        }
        i4_ret = u_app_send_appmsg(h_app,
                                   E_APP_MSG_TYPE_MISC,
                                   MSG_FROM_WIFI_SETTING,
                                   ASSISTANT_STUB_CMD_NETWORK_STATUS_CHANGE,
                                   &network_status_change,
                                   sizeof(ASSISTANT_STUB_NETWORK_STATUS_CHANGE_T));
        if(0 == i4_ret)
        {
            WIFI_MSG("<WIFI_SETTING_PROC> u_app_send_appmsg success !!!\n");
        }
        else
        {
            WIFI_MSG("<WIFI_SETTING_PROC> u_app_send_appmsg fail !!!\n");
        }
    
        WIFI_MSG("<SMART CONNECTION>g_is_start_smart_connection is FALSE ,need start thread !\n");
        g_is_start_smart_connection = TRUE;

        //start smart connection
        int i_ret = 0;
        int i_timer = 0;
        g_check_callback = TRUE;
        c_net_wlan_disassociate();
        //get ap info from elian
        i_ret = wifi_smart_connection_init(g_key, g_key_len, wifi_smart_connection_event_callback);
        if (i_ret != WIFI_SMART_CONNECTION_OK)
        {
            WIFI_ERR("wifi_smart_connection_init fail!\n",i_ret, __LINE__);
            return WIFI_FAIL;
        }
        if(wifi_smart_connection_start(0) < 0)
        {
            WIFI_ERR("wifi_smart_connection_start fail\n");
        }

        while(1)
        {
            WIFI_MSG("get elian result time = %ds\n",i_timer);
            //over 3 minutes
            if (i_timer > 180)
            {
                WIFI_MSG("<SMART CONNECTION>elian time is out of 180s!!!\n");
                wifi_smart_connection_stop();
                wifi_smart_connection_deinit();
                return WIFI_FAIL;
            }

            //for stop elian detect
            if(fg_stop_smart_connection)
            {
                fg_stop_smart_connection=FALSE;
                g_is_start_smart_connection=FALSE;
                wifi_smart_connection_stop();
                wifi_smart_connection_deinit();
                return WIFI_OK;
            }

            //stop wifi timer
            if((t_g_this_obj.t_wlan_timer.h_timer!= (HANDLE_T)NULL))
            {
                u_timer_stop(t_g_this_obj.t_wlan_timer.h_timer);
            }

            u_thread_delay(1000);

            if(g_check_callback == FALSE)
            {
                break;
            }
            i_timer++;
        }
        wifi_smart_connection_stop();
        wifi_smart_connection_deinit();

        //start to connect AP
        if(wifi_smart_connection_wlan0_connect_ap() == WIFI_OK)
        {
            WIFI_MSG("<SMART CONNECTION>elian setting success!\n");
            return WIFI_OK;
        }
        else
        {
            WIFI_ERR("<SMART CONNECTION>elian setting failed!\n");
            return WIFI_FAIL;
        }
    }
    else
    {
        WIFI_MSG("<SMART CONNECTION>g_is_start_smart_connection is TRUE ,no need start smart connection again!\n");
    }

    return WIFI_OK;
}

signed int wifi_open_smart_connection(unsigned char ui1IfType)
{
    WIFI_MSG("wifi_open_smart_connection:%d \n",ui1IfType);
    if (1 == ui1IfType)
    {
        WIFI_MSG("<SMART CONNECTION> start smart connection \n");
        fg_stop_smart_connection = FALSE;
        //wifi_init_smart_connection_database();
        wifi_start_smart_connection();
        g_is_start_smart_connection = FALSE;
    }
    else
    {
         WIFI_MSG("<SMART CONNECTION> stop smart connection \n");
         fg_stop_smart_connection = TRUE;
    }

    return WIFI_OK;
}

#endif

INT32 wifi_task_realize(UINT8 i_task, const CHAR *ssid, const CHAR *bssid, const CHAR *password,INT32 authmode)
{
    INT32           i4_ret = WIFI_OK;

    WIFI_WIRED_TASK_T wifi_task = i_task;
    switch(wifi_task)
    {
        case SCAN_AP:
        {
            i4_ret = _setup_wlan_prepare_to_scan();
        }
        break;

        case SCAN_STOP:
        {
            i4_ret = _setup_wlan_stop_scan();

        }
        break;

        case CONNECT_SETTING_AP:
        {
            #if 0 /*switch*/
            i4_ret = setup_goto_manual_associate(ssid, bssid, password, authmode);
            #else
#if CONFIG_APP_SUPPORT_SMART_CONNECTION
            wifi_smart_connection_close();
            wifi_smart_connection_clear_result();
            wifi_open_smart_connection(0);
            u_thread_delay(1000);
#endif
            i4_ret = wifi_smart_connection_get_specified_AP_info();
            #endif
        }
        break;

        case START_SMART_CONNECTION:
        {
#if CONFIG_APP_SUPPORT_SMART_CONNECTION
            i4_ret = wifi_open_smart_connection(1);
#endif
        }
        break;

        case CONNECT_FAVORATE_AP:
        {
            i4_ret = setup_wlan_try_connect_favorite_ap_as_dongle_in();
        }
        break;
#ifndef CONFIG_ADAPTOR_APP_CTRL
        case CLEAN_AP_INFO:
        {
            WIFI_MSG("<WIFI_SETTING_PROC> =======Clean Ap Info=======!!!\n");
            NET_802_11_ASSOCIATE_T _rSavedWlanAssociate = {{0}};
            a_cfg_set_wlan_ap(_rSavedWlanAssociate);
        }
        break;
#endif /* CONFIG_ADAPTOR_APP_CTRL */
        case GET_AP_INFO_LIST:
        {
            WIFI_MSG("<WIFI_SETTING_PROC> =======Get Ap Info List=======!!!\n");
            WIFI_SETTING_PROC_FUNCTION_BEGIN;
            g_is_upload_ap_list = TRUE;
            start_scan();
            WIFI_SETTING_PROC_FUNCTION_END;
        }
        break;
        
        default:
            break;
    }

    return i4_ret;
}

INT32 wifi_elian_task(UINT8 i_task, const CHAR  primary_channel , const CHAR secondary_channel,CHAR * bssid )
{
    INT32 i4_ret = WIFI_OK;

    WIFI_WIRED_TASK_T wifi_task = i_task;
    switch(wifi_task)
    {
        case OPEN_MONITOR:
        {
            /*add vendor_monitor_open*/ /*CLI:app.wifi.elian 0*/
            c_net_monitor_open();
        }
        break;

        case RECV_80211_FRAME:
        {
            /*add create_packet_thread ,thread call vendor_recv_80211_frame*/
            /*CLI:app.wifi.elian 2*/
            c_net_create_packet_thread();
        }
        break;

        case CHANNEL_SWITCH:
        {
            /*add vendor_channel_switch*/ /*CLI:app.wifi.elian 3*/
            c_net_channel_switch();
        }
        break;

        case CLOSE_MONITOR:
        {
            /*add vendor_monitor_close*/ /*CLI:app.wifi.elian 1*/
            c_net_monitor_close();
        }
        break;



        default:
            break;
    }

    return i4_ret;
}

INT32 u_wifi_get_current_info(CHAR *ssid, CHAR *bssid, WIFI_AUTH_MODE_E *auth_mode) {
    NET_802_11_ASSOCIATE_T t_wifi_get = {0};

    INT32 ret = a_cfg_get_wlan_ap(&t_wifi_get);
    if (ret) {
        WIFI_ERR("a_cfg_get_wlan_ap failed:[%d]!\n", ret);
        return WIFI_FAIL;
    }

    WIFI_MSG("ssid:%s.\n",t_wifi_get.t_Ssid.ui1_aSsid);

    if (ssid) {
        strncpy(ssid, t_wifi_get.t_Ssid.ui1_aSsid, ASSISTANT_STUB_SSID_MAX_LENGTH);
    }

    if (bssid) {
        snprintf(bssid, ASSISTANT_STUB_BSSID_MAX_LENGTH, "%x:%x:%x:%x:%x:%x",
                 t_wifi_get.t_Bssid[0], t_wifi_get.t_Bssid[1], t_wifi_get.t_Bssid[2],
                 t_wifi_get.t_Bssid[3], t_wifi_get.t_Bssid[4], t_wifi_get.t_Bssid[5]);
    }

    if (auth_mode) {
        *auth_mode = transfer_supplicant_to_auth_mode(t_wifi_get.e_AuthMode);
    }

    return WIFI_OK;
}

