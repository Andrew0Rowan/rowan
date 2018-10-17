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
 * $RCSfile: x_net_wlan_init.c,v $
 * $Revision:
 * $Date:
 * $Author: yali.wu $
 *
 * Description:
 *         This file contains API of wlan init and deinit 
 *---------------------------------------------------------------------------*/
	 
/*-----------------------------------------------------------------------------
						 include files
 -----------------------------------------------------------------------------*/

#include "_net_wlan_init.h"
#include "wifi_log.h"
#include "wifi_com.h"
#include "wlan_command.h"
#include "wifi_parse.h"
#include "u_os.h"
#include <sys/stat.h>
#include <sys/prctl.h>

extern void wifi_parse_event(char* buf,size_t buf_len);
extern void p2p_parse_event(char* buf,size_t buf_len);

extern INT32 wps_generate_pin_code(char *wps_pin_code);
//extern char   wps_pin_code[WLAN_PIN_LENGTH];
extern BOOL p2p_for_wfd;

static BOOL b_wlan_thread_init = FALSE;
static BOOL b_wlan_thread_pause = FALSE;
static BOOL b_wlan_thread_stop = FALSE;

static BOOL b_p2p_thread_init = FALSE;
static BOOL b_p2p_thread_pause = FALSE;
static BOOL b_p2p_thread_stop = FALSE;

extern SEMA_HANDLE h_WlanNfySema;
extern SEMA_HANDLE h_P2pNfySema;

#if PLATFORM_IS_TV
SEMA_HANDLE h_startdriversema = {.is_semainited = FALSE};
static x_net_wlan_notify_fct _drv_notify = NULL;
static x_net_wlan_notify_fct _p2p_drv_notify = NULL;
static pid_t supp_pid = -1;
static pid_t p2p_supp_pid = -1;
static BOOL b_start_driver_thread = FALSE;
static BOOL b_wpa_supplicant_started = FALSE;
static BOOL b_start_p2p_supp_thread = FALSE;
const char * if_name_map[WIFI_DIRECT_DEV_NUM][WIFI_COL_NUM] = {{"ra0"  , "ralink" , "p2p0",WPA_SUPPLICANT_PATH"/common"},
    {"ath0" , "atheros"    , "wlan1","/3rd/wpa_supplicant/atheros"},
    {"rea0" , "realtek", "rp2p0","/3rd/wpa_supplicant/realtek"},
    {"wlan0", "ralink"    , "p2p0",WPA_SUPPLICANT_PATH"/common"}
};
#endif

char   if_name[WLAN_WLAN_IF_MAX_LEN] = {0};
char   p2p_name[WLAN_WLAN_IF_MAX_LEN] = {0};
char   p2p_go_name[WLAN_WLAN_IF_MAX_LEN] = "p2p0";

struct wpa_ctrl *ctrl_conn = NULL;
struct wpa_ctrl *mon_conn = NULL;


struct wpa_ctrl *p2p_ctrl_conn = NULL;
struct wpa_ctrl *p2p_mon_conn = NULL;

//need to sync . set thread name
#define WLAN_THREAD_NAME            "Wlan Thread"
#define WLAN_STACK_SIZE             (1024 * 20)
#define WLAN_THREAD_PRIORITY       100

#if defined(SYS_C4TV_SUPPORT) && defined(SYS_3RD_RO)
#define WPA_SUPPLICANT_CONFIG_PATH "/3rd_rw/wpa_supplicant"
#else
#define WPA_SUPPLICANT_CONFIG_PATH "/3rd/bin/wpa_supplicant"
#endif

char WIFI_SOCKET_PATH[64] = "/tmp/wpa_supplicant/";
char WLAN_CTRL_CONN_PATH[64] = "/tmp/wpa_supplicant/wlan_ctl";
char WLAN_MONI_CONN_PATH[64] = "/tmp/wpa_supplicant/wlan_nfy";
char P2P_CTRL_CONN_PATH[64] = "/tmp/wpa_supplicant/p2p_ctl";
char P2P_MONI_CONN_PATH[64] = "/tmp/wpa_supplicant/p2p_nfy";

pid_t g_PID = -1;

/* the flag of cast for audio function is enable or not */
BOOL g_flagCastForAudioEnable = TRUE;

/* Check "/chrome" folder exist or not to determine cast for audio function is enable or not **
** Because the file system is different when cast for audio function is enabled */
int check_cast_audio_is_enable(void)
{
    WIFI_LOG(WIFI_DEBUG ,("Enter check_cast_audio_is_enable()\n"));

    struct stat test_chrome;
    int ret = -1;

    ret = stat("/chrome", &test_chrome);
    if ( ret == 0 ) {
        g_flagCastForAudioEnable = TRUE;
        WIFI_LOG(WIFI_DEBUG ,("Cast for Audio function is enable.\n"));
    } else {
        if ( errno == ENOENT ) {
            g_flagCastForAudioEnable = FALSE;
            WIFI_LOG(WIFI_DEBUG ,("Cast for Audio function is disable.\n"));
        } else {
            g_flagCastForAudioEnable = TRUE;
            WIFI_LOG(WIFI_DEBUG ,("Cast for Audio function is enable.\n"));
        }
    }

	g_PID = getpid();
    
    WIFI_LOG(WIFI_DEBUG,("Current PID=%d\n", g_PID));

    if ( g_flagCastForAudioEnable ) {
        snprintf(WIFI_SOCKET_PATH, sizeof(WIFI_SOCKET_PATH), "/dev/socket/");
		snprintf(WLAN_CTRL_CONN_PATH, sizeof(WLAN_CTRL_CONN_PATH), 
			"/dev/socket/wifi/wpa_ctrl_%d", g_PID);
		snprintf(WLAN_MONI_CONN_PATH, sizeof(WLAN_MONI_CONN_PATH), 
			"/dev/socket/wifi/wpa_moni_%d", g_PID);
	    snprintf(P2P_CTRL_CONN_PATH, sizeof(P2P_CTRL_CONN_PATH), 
			"/dev/socket/wifi/p2p_ctl_%d", g_PID);
	    snprintf(P2P_MONI_CONN_PATH, sizeof(P2P_MONI_CONN_PATH), 
			"/dev/socket/wifi/p2p_moni_%d", g_PID);
    } else {
        snprintf(WIFI_SOCKET_PATH, sizeof(WIFI_SOCKET_PATH), 
			"/tmp/wpa_supplicant/");
		snprintf(WLAN_CTRL_CONN_PATH, sizeof(WLAN_CTRL_CONN_PATH), 
			"/tmp/wpa_supplicant/wpa_ctrl_%d", g_PID);
		snprintf(WLAN_MONI_CONN_PATH, sizeof(WLAN_MONI_CONN_PATH), 
			"/tmp/wpa_supplicant/wpa_moni_%d", g_PID);
	    snprintf(P2P_CTRL_CONN_PATH, sizeof(P2P_CTRL_CONN_PATH), 
			"/tmp/wpa_supplicant/p2p_ctrl_%d", g_PID);
	    snprintf(P2P_MONI_CONN_PATH, sizeof(P2P_MONI_CONN_PATH), 
			"/tmp/wpa_supplicant/p2p_moni_%d", g_PID);
    }

    WIFI_LOG(WIFI_DEBUG ,("\n control interface dir: %s\n " 
		"wpa control client path: %s\n " "wpa monitor client path: %s\n " 
		"p2p control client path: %s\n " "p2p monitor client path: %s\n", 
        WIFI_SOCKET_PATH, WLAN_CTRL_CONN_PATH, WLAN_MONI_CONN_PATH, 
        P2P_CTRL_CONN_PATH, P2P_MONI_CONN_PATH));

    return 0;
}

const char* get_if_name()
{
    return if_name;
}

static void* _wlan_msg_proc_thread(VOID * pvArg)
{
    pthread_detach(pthread_self());
    WIFI_LOG(WIFI_DEBUG ,("_wlan_msg_proc_thread START.\n"));
    int buf_len =0;
    //char buf[256] = {0};
    char buf[512] = {0}; /* enlarge the buffer size */
    b_wlan_thread_stop = FALSE;	  // when enter this b_wlan_thread_stop should be not stop
    while (1)
    {
        usleep(20000);
        buf_len = 0;
        memset(buf,0,sizeof(buf));
        if (b_wlan_thread_stop == TRUE)
        {
            b_wlan_thread_stop = FALSE;
            break;
        }
        while (b_wlan_thread_pause || mon_conn == NULL)
        {
            WIFI_LOG(WIFI_DEBUG ,("pause_wlan_msg_proc_thread!\n"));
            usleep(50000);
        }
        if (mon_conn == NULL)
        {
            WIFI_LOG(WIFI_DEBUG ,("exit _wlan_msg_proc_thread!\n"));
            break;
        }
		sema_lock(&h_WlanNfySema);
        while (wpa_ctrl_pending(mon_conn) > 0)
        {
            WIFI_LOG(WIFI_INFO,("wifi_wait_for_event start !\n"));
            buf_len = wifi_wait_for_event(mon_conn, buf, sizeof(buf));
            WIFI_LOG(WIFI_INFO,("wifi_wait_for_event out here !\n"));
            wifi_parse_event(buf,buf_len);
        }
        if (wpa_ctrl_pending(mon_conn) < 0)
        {
            WIFI_LOG(WIFI_DEBUG,("Connection to wpa_supplicant lost - break\n"));
			sema_unlock(&h_WlanNfySema);
			break;
        }
		sema_unlock(&h_WlanNfySema);
    }
    b_wlan_thread_init = FALSE;
    WIFI_LOG(WIFI_DEBUG ,("_wlan_msg_proc_thread out here !\n"));
    return NULL;
}

INT32 wlan_thread_init(void)
{
    pthread_t    h_wlan_thread;
    WIFI_LOG(WIFI_DEBUG ,("c_net_wlan_thread_init. in this func\n"));

    if (FALSE == b_wlan_thread_init)
    {
        WIFI_LOG(WIFI_DEBUG ,("c_net_wlan_thread_init.start creat thread\n"));
        if (pthread_create(&h_wlan_thread,NULL,
                           _wlan_msg_proc_thread, NULL) != OSR_OK)
        {
            WIFI_LOG(WIFI_ERROR ,("thread_create.wlan Fail\n"));
            return (-1);
        }
        b_wlan_thread_init = TRUE;
    }
    else
    {
        WIFI_LOG(WIFI_DEBUG ,("wlan_thread_init all right init\n"));
    }

    return 0;

}

INT32 wifi_open_connection(BOOL is_p2p ,const char * path )
{
    char cmd[100]={0};
    if (is_p2p)
    {
        if (p2p_ctrl_conn != NULL && p2p_mon_conn!= NULL)
        {
            WIFI_LOG(WIFI_DEBUG ,("c_net_p2p_open_connection -> p2p socket has already enabled!\n"));
            return 0;
        }
        p2p_ctrl_conn = NULL;
        p2p_mon_conn = NULL;

        if ( g_flagCastForAudioEnable ) {
            snprintf(cmd,sizeof(cmd),"%swpa_%s",path,p2p_name);
        } else {
            snprintf(cmd,sizeof(cmd),"%s%s",path,p2p_name);
        }
		
        p2p_ctrl_conn = wpa_ctrl_open(cmd,P2P_CTRL_CONN_PATH);
        if (p2p_ctrl_conn == NULL)
        {
            return -1;
        }
        p2p_mon_conn = wpa_ctrl_open(cmd,P2P_MONI_CONN_PATH);
        if (p2p_mon_conn == NULL)
        {
            wpa_ctrl_close(p2p_ctrl_conn);
            p2p_ctrl_conn = NULL;
            return -1;
        }
        if (wpa_ctrl_attach(p2p_mon_conn) != 0)
        {
            wpa_ctrl_close(p2p_mon_conn);
            wpa_ctrl_close(p2p_ctrl_conn);
            p2p_ctrl_conn = p2p_mon_conn = NULL;
            return -1;
        }
    }
    else
    {
        if (ctrl_conn != NULL && mon_conn!= NULL)
        {
            WIFI_LOG(WIFI_DEBUG ,("c_net_open_connection -> wifi socket has already enabled!\n"));
            return 0;
        }

        ctrl_conn = NULL;
        mon_conn = NULL;

        if ( g_flagCastForAudioEnable ) {
            snprintf(cmd,sizeof(cmd),"%swpa_%s", path, if_name);
        } else {
            snprintf(cmd,sizeof(cmd),"%s%s", path, if_name);
        }
		
        ctrl_conn = wpa_ctrl_open(cmd,WLAN_CTRL_CONN_PATH);
        if (ctrl_conn == NULL)
        {
            return -1;
        }
        mon_conn = wpa_ctrl_open(cmd,WLAN_MONI_CONN_PATH);
        if (mon_conn == NULL)
        {
            wpa_ctrl_close(ctrl_conn);
            ctrl_conn = NULL;
            return -1;
        }
        if (wpa_ctrl_attach(mon_conn) != 0)
        {
            wpa_ctrl_close(mon_conn);
            wpa_ctrl_close(ctrl_conn);
            ctrl_conn = mon_conn = NULL;
            return -1;
        }
    }
    WIFI_LOG(WIFI_DEBUG ,("wifi_open_connection -> success!\n"));
    return 0;
}


INT32 wifi_open_connection_per_path(BOOL is_p2p ,const char * path, char *ctrl_path, char *moni_path )
{
    char cmd[100]={0};
    if (is_p2p)
    {
        if (p2p_ctrl_conn != NULL && p2p_mon_conn!= NULL)
        {
            WIFI_LOG(WIFI_DEBUG ,("c_net_p2p_open_connection -> p2p socket has already enabled!\n"));
            return 0;
        }
        p2p_ctrl_conn = NULL;
        p2p_mon_conn = NULL;

        if ( g_flagCastForAudioEnable ) {
            snprintf(cmd,sizeof(cmd),"%swpa_%s",path,p2p_name);
        } else {
            snprintf(cmd,sizeof(cmd),"%s%s",path,p2p_name);
        }
		
        p2p_ctrl_conn = wpa_ctrl_open(cmd, ctrl_path);
        if (p2p_ctrl_conn == NULL)
        {
            return -1;
        }
        p2p_mon_conn = wpa_ctrl_open(cmd, moni_path);
        if (p2p_mon_conn == NULL)
        {
            wpa_ctrl_close(p2p_ctrl_conn);
            p2p_ctrl_conn = NULL;
            return -1;
        }
        if (wpa_ctrl_attach(p2p_mon_conn) != 0)
        {
            wpa_ctrl_close(p2p_mon_conn);
            wpa_ctrl_close(p2p_ctrl_conn);
            p2p_ctrl_conn = p2p_mon_conn = NULL;
            return -1;
        }
    }
    else
    {
        if (ctrl_conn != NULL && mon_conn!= NULL)
        {
            WIFI_LOG(WIFI_DEBUG ,("c_net_open_connection -> wifi socket has already enabled!\n"));
            return 0;
        }

        ctrl_conn = NULL;
        mon_conn = NULL;

        if ( g_flagCastForAudioEnable ) {
            snprintf(cmd,sizeof(cmd),"%swpa_%s", path, if_name);
        } else {
            snprintf(cmd,sizeof(cmd),"%s%s", path, if_name);
        }
		
        ctrl_conn = wpa_ctrl_open(cmd, ctrl_path);
        if (ctrl_conn == NULL)
        {
            return -1;
        }
        mon_conn = wpa_ctrl_open(cmd, moni_path);
        if (mon_conn == NULL)
        {
            wpa_ctrl_close(ctrl_conn);
            ctrl_conn = NULL;
            return -1;
        }
        if (wpa_ctrl_attach(mon_conn) != 0)
        {
            wpa_ctrl_close(mon_conn);
            wpa_ctrl_close(ctrl_conn);
            ctrl_conn = mon_conn = NULL;
            return -1;
        }
    }
    WIFI_LOG(WIFI_DEBUG ,("wifi_open_connection -> success!\n"));
    return 0;
}


INT32 wlan_check_init_state(void)
{
    if (ctrl_conn == NULL)
    {
        WIFI_LOG(WIFI_ERROR ,("Not connect to wpa_supplicant!\n"));
        return NET_WLAN_NOT_INIT;
    }
    return NET_WLAN_OK;
}

INT32 x_net_wps_generate_pin_code(char *wps_pin_code)
{
	return wps_generate_pin_code(wps_pin_code);
}

INT32 x_net_wlan_open_connection(const char *ifname)
{
	check_cast_audio_is_enable();
	
    if (wifi_open_connection(FALSE, WIFI_SOCKET_PATH) < 0)
    {
        WIFI_LOG(WIFI_ERROR, ("could not connect to wpa supplicant\n"));
        return -1;
    }

    if (wlan_thread_init() != 0)
    {
        WIFI_LOG(WIFI_DEBUG,(" %s,%d,b_wlan_thread_init %d b_wlan_thread_pause %d\n",__FUNCTION__,__LINE__,b_wlan_thread_init,b_wlan_thread_pause));
    }
    //wps_generate_pin_code(wps_pin_code); // tood!!! generate pin code  first use should be change
    return 0;

}


INT32 x_net_wlan_open_connection_per_path(const char *ifname, char *ctrl_path, char *moni_path)
{
	check_cast_audio_is_enable();
	
    if (wifi_open_connection_per_path(FALSE, WIFI_SOCKET_PATH, ctrl_path, moni_path) < 0)
    {
        WIFI_LOG(WIFI_ERROR, ("could not connect to wpa supplicant\n"));
        return -1;
    }

    if (wlan_thread_init() != 0)
    {
        WIFI_LOG(WIFI_DEBUG,(" %s,%d,b_wlan_thread_init %d b_wlan_thread_pause %d\n",__FUNCTION__,__LINE__,b_wlan_thread_init,b_wlan_thread_pause));
    }
    //wps_generate_pin_code(wps_pin_code); // tood!!! generate pin code  first use should be change
    return 0;

}


INT32 x_net_wlan_close_connection(void)
{
    INT32 count = 10;
    if (ctrl_conn != NULL)
    {
        wpa_ctrl_close(ctrl_conn);
        ctrl_conn = NULL;
    }
    if (mon_conn != NULL)
    {
        b_wlan_thread_stop = TRUE;
        while (count-- > 0)
        {
            if (!b_wlan_thread_init)
            {
                break;
            }
            else
            {
                usleep(50000);
                WIFI_LOG(WIFI_INFO ,("x_net_wlan_close_connection wait for recv %d\n",b_wlan_thread_init));
            }
        }
		
		sema_lock(&h_WlanNfySema);
        wpa_ctrl_detach(mon_conn);
        wpa_ctrl_close(mon_conn);
		sema_unlock(&h_WlanNfySema);
        mon_conn = NULL;
    }
    b_wlan_thread_init = FALSE;
    WIFI_LOG(WIFI_DEBUG ,("x_net_wlan_close_connection! init %d count %d\n",b_wlan_thread_init,count));
    return 0;
}


BOOL x_net_check_p2p_thread_inited(void)
{
    return b_p2p_thread_init;
}
static void* _p2p_msg_proc_thread(VOID * pvArg)
{
    pthread_detach(pthread_self());
    int buf_len =0;
    char buf[512] = {0};
    b_p2p_thread_stop = FALSE;
    prctl(PR_SET_NAME,"p2pmsgproc",0,0,0);
    WIFI_LOG(WIFI_DEBUG,("[P2P]_p2p_msg_proc_thread START.\n"));
    while (1)
    {
        usleep(20000);

        if (b_p2p_thread_stop == TRUE)
        {
            b_p2p_thread_stop = FALSE;
            break;
        }
        while (b_p2p_thread_pause)
        {
            usleep(50000);
        }
        if (p2p_ctrl_conn == NULL)
        {
            WIFI_LOG(WIFI_ERROR,("[P2P]connect lose exit _p2p_msg_proc_thread!\n"));
            break;
        }
		sema_lock(&h_P2pNfySema);
        while (wpa_ctrl_pending(p2p_mon_conn) > 0)
        {
            buf_len = 0;
            memset(buf,0,sizeof(buf));
            buf_len = wifi_wait_for_event(p2p_mon_conn, buf, sizeof(buf));
            p2p_parse_event(buf,buf_len);
        }
        if (wpa_ctrl_pending(p2p_mon_conn) < 0)
        {
            WIFI_LOG(WIFI_ERROR,("[P2P]Connection to wpa_supplicant lost - break\n"));			
			sema_unlock(&h_P2pNfySema);
			break;
        }
		sema_unlock(&h_P2pNfySema);
    }//while(1)
    WIFI_LOG(WIFI_DEBUG,("[P2P]_p2p_msg_proc_thread END.\n"));
    b_p2p_thread_init = FALSE;
    return NULL;
}

INT32 p2p_thread_init(void)
{
    pthread_t    h_p2p_thread;
    if (FALSE == b_p2p_thread_init)
    {
        WIFI_LOG(WIFI_DEBUG,("[P2P]p2p_thread_init.\n"));

        if (pthread_create(&h_p2p_thread,NULL,
                           _p2p_msg_proc_thread, NULL) != OSR_OK)
        {
            WIFI_LOG(WIFI_DEBUG,("[P2P]thread_create.p2p Fail\n"));
            return (-1);
        }
        b_p2p_thread_init = TRUE;
    }
    return 0;
}


INT32 x_net_p2p_check_init_state(void)
{
    if (p2p_ctrl_conn == NULL)
    {
        WIFI_LOG(WIFI_ERROR,("[P2P]p2p Not connect to wpa_supplicant!\n"));
        return NET_WLAN_NOT_INIT;
    }
    return NET_WLAN_OK;
}



INT32 x_net_p2p_open_connection(void)
{
    WIFI_LOG_ENTRY(WIFI_API,(""));

	check_cast_audio_is_enable();
	
    if ( wifi_open_connection(TRUE,WIFI_SOCKET_PATH) != 0)
    {
        WIFI_LOG(WIFI_ERROR,("[P2P]Enter x_net_p2p_open_connection failed!\n"));
        return -1;
    }

    if (p2p_thread_init() != 0)
    {
        WIFI_LOG(WIFI_ERROR,("[P2P] p2p_thread_init failed!\n"));
        return -1;
    }

    WIFI_LOG(WIFI_DEBUG,("[P2P] x_net_p2p_open_connection! success\n"));
    return 0;
}

INT32 x_net_p2p_send_to_socket(void)
{
    //exit the p2p thread
    b_p2p_thread_stop = TRUE;
    INT32 count = 10;
    while (count-- > 0)
    {
        if (!b_p2p_thread_init)
        {
            break;
        }
        else
        {
            usleep(50000);
            WIFI_LOG(WIFI_INFO ,("[P2P] x_net_p2p_send_to_socket wait for recv %d\n",b_p2p_thread_init));
        }
    }
    b_p2p_thread_init = FALSE;
    return 0;
}


INT32 x_net_p2p_close_connection(void)
{
    WIFI_LOG_ENTRY(WIFI_API,(""));
    if (p2p_ctrl_conn != NULL)
    {
        wpa_ctrl_close(p2p_ctrl_conn);
        p2p_ctrl_conn = NULL;
    }
    if (p2p_mon_conn != NULL)
    {
    
		sema_lock(&h_P2pNfySema);
        wpa_ctrl_detach(p2p_mon_conn);
        wpa_ctrl_close(p2p_mon_conn);
		sema_unlock(&h_P2pNfySema);
        p2p_mon_conn = NULL;
    }

    WIFI_LOG(WIFI_DEBUG,("[P2P] x_net_p2p_close_connection! init %d\n",b_p2p_thread_init));
    p2p_for_wfd = FALSE;
    return 0;
}

#ifdef SUPPORT_2_DONGLES
INT32 x_net_wlan_set_use_ifname(char *s_ifname)
{
    memset(if_name , 0 , WLAN_WLAN_IF_MAX_LEN);
    strncpy (if_name, s_ifname, sizeof (if_name));
    return 0;
}
#endif

#if PLATFORM_IS_TV
INT32 wlan_check_supplicant_ready(char * ifname)
{
    if (ifname == NULL)
    {
        return NET_WLAN_INVALID_PARAM;
    }
    char check_supplicant_ready_sh[512] = {0};

    snprintf(check_supplicant_ready_sh,sizeof(check_supplicant_ready_sh),"sh %s/check_supplicant_ready.sh %s %s",WPA_SUPPLICANT_PATH,ifname,WPA_SUPPLICANT_PATH);

    int ret = doScriptCmd(check_supplicant_ready_sh);
    ret = ret >> 8;
    WIFI_LOG(WIFI_ERROR , (">>>>>check_supplicant_ready_sh ret %d errno %s!\n",ret,strerror(errno)));
    if (ret != 0 )//check if check_supplicant_ready script exit abnormal .
    {
        WIFI_LOG(WIFI_ERROR , (">>>>>check_supplicant_ready fail!\n"));
        return NET_WLAN_ERROR;
    }
	b_wpa_supplicant_started = true;
    return NET_WLAN_OK;
}
int _wifi_kill(pid_t pid)
{
    int ret;
    ret = kill(pid,SIGTERM);
    wait(0);
    return ret;
}

int _wifi_kill_supplicant()
{
    if (supp_pid > 0)
    	{
		WIFI_LOG(WIFI_DEBUG ,(">>>>>>> %s,%d, pid is <0 %d \n",__FUNCTION__,__LINE__,supp_pid));
        return _wifi_kill(supp_pid);
    	}
    return 0;
}

pid_t _start_process(const char* path,char * const argv[])
{
    pid_t pid = -1;
    WIFI_LOG(WIFI_DEBUG ,(">>>>>>> %s,%d,in \n",__FUNCTION__,__LINE__));
    if ((pid = fork()) == 0)
    {
        WIFI_LOG(WIFI_DEBUG ,(">>>>>>> %s,%d,in child child \n",__FUNCTION__,__LINE__));
        execv(path,argv);
    }
    else
    {
        WIFI_LOG(WIFI_DEBUG ,(">>>>>>> %s,%d,in father child pid %d\n",__FUNCTION__,__LINE__,pid));
        return pid;
    }
    return pid;
}


BOOL wifi0_check(const char* ifname)
{
    int s = 0;
    int ret = -1;
    struct ifreq ifr;

    memset(&ifr, 0, sizeof(struct ifreq));

    WIFI_LOG(WIFI_DEBUG ,(">>>>>>> %s, %s, %d,\n",__FUNCTION__,ifname,__LINE__));

    memset(&ifr, 0, sizeof(struct ifreq));
    strncpy(ifr.ifr_name, ifname,strlen(ifname));

    s = socket(AF_INET, SOCK_DGRAM, 0);
    if (s < 0)
    {
        WIFI_LOG(WIFI_ERROR ,(">>>>>>> %s,%d,fail to create socket\n",__FUNCTION__,__LINE__));
        return FALSE;
    }
    ret = ioctl(s, SIOCGIFFLAGS, &ifr);
    close(s);

    if (ret == 0)
    {
        return TRUE;
    }
    return FALSE;
}


static void* _start_wlan_supplicant_thread(VOID * pvArg)
{
    NET_802_11_NFY_T t_nfy = {0};
    int if_id = 0;
    char _cmd_path[50] = {0};
    char _cmd_interface[20] = {0};
    char _cmd_driver_if[20] = {0};
#ifdef SUPPORT_P2P_CONCURRENT
    char _cmd_p2p_interface[20] = {0};
#endif
    int isP2p = (int )pvArg;
    pthread_detach(pthread_self());

    prctl(PR_SET_NAME,"startwlansupplicant",0,0,0);
    WIFI_LOG(WIFI_DEBUG ,(">>>>>>> _start_wlan_supplicant_thread,pvArg 0x%8x,isp2p %d\n",pvArg,isP2p));
#ifndef SUPPORT_P2P_CONCURRENT
    int count  = 10;
#endif
#if 0//def SUPPORT_2_DONGLES
    if ((0 == strncmp(if_name, "ath0", strlen ("ath0")))
            || (0 == strncmp(if_name, "athmag0", strlen ("athmag0")))
       )
    {
        if_id = 0;
        strcpy (_cmd_driver_if, "-Dathr");
    }
    else if (0 == strncmp(if_name, "ra0", strlen ("ra0")))
    {
        if_id = 3;
        strcpy (_cmd_driver_if, "-Dwext");
    }
    else if (0 == strncmp(if_name, "rea0", strlen ("rea0")))
    {
        if_id = 2;
        strcpy (_cmd_driver_if, "-Dwext");
    }
    else
    {
        if_id = WIFI_DIRECT_DEV_NUM;
    }

    if (if_id < WIFI_DIRECT_DEV_NUM)
    {
        if (wifi0_check(if_name))
        {
            sprintf(_cmd_path,"%s/wpa_supplicant",if_name_map[if_id][WIFI_INTERFACE_SUPPLICANT_PATH]);
            sprintf(_cmd_interface,"-i%s",if_name);
        }
        else
        {
            if_id = WIFI_DIRECT_DEV_NUM;
        }
    }

#else

    for (; if_id < WIFI_DIRECT_DEV_NUM; if_id++)
    {
        if (wifi0_check(if_name_map[if_id][WIFI_INTERFACE_NAME]))
        {
            WIFI_LOG(WIFI_DEBUG ,(">>>>>>> %s,%d,find interface %s\n",__FUNCTION__,__LINE__,if_name_map[if_id][WIFI_INTERFACE_NAME]));
            memset(if_name , 0 , WLAN_WLAN_IF_MAX_LEN);
            strncpy(if_name , if_name_map[if_id][WIFI_INTERFACE_NAME],WLAN_WLAN_IF_MAX_LEN);
            snprintf(_cmd_path,sizeof(_cmd_path),"%s/wpa_supplicant",if_name_map[if_id][WIFI_INTERFACE_SUPPLICANT_PATH]);
            snprintf(_cmd_interface,sizeof(_cmd_interface),"-i%s",if_name);

            if ((0 == strncmp (if_name, "wlan0", strlen ("wlan0")))
                    || (0 == strncmp (if_name, "ath0", strlen ("ath0"))))
            {
                strncpy (_cmd_driver_if, "-Dnl80211",sizeof("-Dnl80211"));
            }
            else
            {
                strncpy (_cmd_driver_if, "-Dwext",sizeof("-Dwext"));
            }
            WIFI_LOG(WIFI_DEBUG ,(">>>>>>> %s,%d,cmd_path %s\n",__FUNCTION__,__LINE__,_cmd_path));
            WIFI_LOG(WIFI_DEBUG ,(">>>>>>> %s,%d,cmd interface %s\n",__FUNCTION__,__LINE__,_cmd_interface));
            break;
        }
    }

#endif
    do
    {
        if (if_id == WIFI_DIRECT_DEV_NUM)
        {
            WIFI_LOG(WIFI_DEBUG , (">>>>>_strart_wlan_driver_thread; no any device exist!\n"));
            break;
        }
#ifdef SUPPORT_P2P_CONCURRENT
        memset(p2p_name , 0 , WLAN_WLAN_IF_MAX_LEN);
        strncpy(p2p_name , if_name_map[if_id][WIFI_INTERFACE_P2P_NAME],WLAN_WLAN_IF_MAX_LEN);

        WIFI_LOG(WIFI_DEBUG , (">>>>>_strart_wlan_driver_thread;p2p_name %s!\n",p2p_name));
        snprintf(_cmd_p2p_interface,sizeof(_cmd_p2p_interface),"-i%s",p2p_name);
        WIFI_LOG(WIFI_DEBUG , (">>>>>_strart_wlan_driver_thread;wifi0_check(p2p_name) %d!\n",wifi0_check(p2p_name)));

        char * argv[] = {"wpa_supplicant",_cmd_driver_if,_cmd_interface,"-c",WPA_SUPPLICANT_CONFIG_PATH"/wpa.conf",
                         "-N",_cmd_driver_if,_cmd_p2p_interface,"-c",WPA_SUPPLICANT_CONFIG_PATH"/p2p.conf","-puse_p2p_group_interface=1","-qq",(char*)0
                        };
        if ((supp_pid = _start_process(_cmd_path,argv)) < 0)
        {
            WIFI_LOG(WIFI_ERROR , (">>>>>_strart_wlan_driver_thread call system return error! %s \n",strerror(errno)));
            break;
        }

        if (wlan_check_supplicant_ready(if_name) != NET_WLAN_OK)
        {
            break;
        }
#else
        char * argv[] = {"wpa_supplicant",_cmd_driver_if,_cmd_interface,"-c",WPA_SUPPLICANT_CONFIG_PATH"/wpa.conf","-dd",(char*)0};
        if ((supp_pid = _start_process(_cmd_path,argv)) < 0)
        {
            WIFI_LOG(WIFI_ERROR , (">>>>>_strart_wlan_driver_thread call system return error! %s \n",strerror(errno)));
            break;
        }

        while (count-- > 0)  //todo!!! need modify?
        {
            usleep(500000);
            if (x_net_wlan_open_connection(NULL) != 0)
                continue;
            break;
        }
        if (count == 0)
        {
            WIFI_LOG(WIFI_ERROR , (">>>>>_strart_wlan_driver_thread; open_connection fail!\n"));
            break;
        }
#endif

        if (_drv_notify)
        {
            if (isP2p)
            {
				WIFI_LOG(WIFI_DEBUG ,(">>>>>>> notify p2p supp start ok pid is %d \n",supp_pid));
                t_nfy.i4NotifykId = WLAN_NOTIFY_P2P_SUPP_START_IND;
            }
            else
            {
				WIFI_LOG(WIFI_DEBUG ,(">>>>>>> notify wlan supp start ok pid is %d \n",supp_pid));
                t_nfy.i4NotifykId = WLAN_NOTIFY_DRIVER_START_IND;
            }
            t_nfy.pParam[0] = WLAN_NFY_MSG_OK;
            (_drv_notify)(t_nfy.i4NotifykId , t_nfy.pParam);
        }
        b_start_driver_thread = FALSE;

//        sema_uninit(&h_startdriversema);
//        WIFI_LOG(WIFI_DEBUG ,(">>>>>>> supplicant ready unlock here\n"));
        return NULL;
    }
    while (0);

    x_net_wlan_stop_driver(NULL);

    if (_drv_notify)
    {
        if (isP2p)
        {
			WIFI_LOG(WIFI_DEBUG ,(">>>>>>> notify p2p supp start fail\n"));
            t_nfy.i4NotifykId = WLAN_NOTIFY_P2P_SUPP_START_FAILED;
        }
        else
        {
			WIFI_LOG(WIFI_DEBUG ,(">>>>>>> notify wlan supp start fail\n"));
            t_nfy.i4NotifykId = WLAN_NOTIFY_DRIVER_START_FAILED;
        }
        t_nfy.pParam[0] = WLAN_NFY_MSG_INIT_FAIL;
        (_drv_notify)(t_nfy.i4NotifykId , t_nfy.pParam);
    }
    b_start_driver_thread = FALSE;

//    sema_uninit(&h_startdriversema);
    return NULL;
}

//BD no need to start supplicant at mw
INT32 _start_wlan_driver(x_net_wlan_notify_fct pfNotify,int isP2p)
{
    pthread_t    h_wlan_thread;
    if (FALSE == b_start_driver_thread)
    {
        WIFI_LOG(WIFI_DEBUG ,(" _start_wlan_driver. %08x, %d\n",&isP2p,isP2p));
        _drv_notify = pfNotify;
		
        WIFI_LOG(WIFI_DEBUG ,(" _start_wlan_driver. notify function %08x\n",_drv_notify));
        if (pthread_create(&h_wlan_thread,NULL,
                           _start_wlan_supplicant_thread,(void *)isP2p) != OSR_OK)
        {
            WIFI_LOG(WIFI_DEBUG ,(" thread_create.wlan Fail\n"));
            return NET_WLAN_ERROR;
        }
        b_start_driver_thread = TRUE;
        return 0;
    }
    else
    {
        return NET_WLAN_SUPPLICANT_STARTING;
    }
}



INT32 x_net_wlan_start_driver(x_net_wlan_notify_fct pfNotify,int isP2p)
{
    WIFI_LOG(WIFI_DEBUG ,("x_net_wlan_start_driver isP2p %d\n",isP2p));
    if (h_startdriversema.is_semainited == FALSE)
    {
        INT32 i4ret;
        i4ret = wifi_sema_init(&h_startdriversema);
        if (i4ret != 0)
        {
            WIFI_LOG(WIFI_DEBUG ,("wifi_sema_init  h_startdriversema fail \n"));
            return -1;
        }
        else
        {
            WIFI_LOG(WIFI_DEBUG ,("sema_create  h_startdriversema success \n"));
        }
    }
    sema_lock(&h_startdriversema);
    WIFI_LOG(WIFI_DEBUG ,(">>>>>>> isP2p %d lock here\n",isP2p));
    if ( supp_pid > 0 || b_start_driver_thread)
    {
#ifdef SUPPORT_P2P_CONCURRENT
        if (pfNotify)
        {
            NET_802_11_NFY_T t_nfy = {0};
            if ((isP2p) && (!p2p_ctrl_conn))
            {
                INT32 count = 10;
                WIFI_LOG(WIFI_DEBUG , (">>>> c_net_wlan_start_driver p2p b_start_driver_thread %d!\n",b_start_driver_thread));

                while (count-- > 0)
                {
                    if (!b_start_driver_thread)
                    {
                        break;
                    }
                    else
                    {
                        usleep(100000);
                    }
                }
				if(b_wpa_supplicant_started)
					{
                t_nfy.i4NotifykId = WLAN_NOTIFY_P2P_SUPP_START_IND;
                t_nfy.pParam[0] = WLAN_NFY_MSG_OK;
                (pfNotify)(t_nfy.i4NotifykId , t_nfy.pParam);
			   WIFI_LOG(WIFI_DEBUG , (">>>> c_net_wlan_start_driver p2p b_start_driver_thread %d!\n",b_start_driver_thread));
					}
            }

            if ((!isP2p) && (!ctrl_conn))
            {
                INT32 count = 10;
                WIFI_LOG(WIFI_DEBUG , (">>>> c_net_wlan_start_driver wlan b_start_driver_thread %d!\n",b_start_driver_thread));
                while (count-- > 0)
                {
                    if (!b_start_driver_thread)
                    {
                        break;
                    }
                    else
                    {
                        usleep(100000);
                    }
                }
				if(b_wpa_supplicant_started)
					{
                t_nfy.i4NotifykId = WLAN_NOTIFY_DRIVER_START_IND;
                t_nfy.pParam[0] = WLAN_NFY_MSG_OK;
                (pfNotify)(t_nfy.i4NotifykId , t_nfy.pParam);
				WIFI_LOG(WIFI_DEBUG , (">>>> c_net_wlan_start_driver wlan b_start_driver_thread %d!\n",b_start_driver_thread));
					}
            }
        }
#endif
        sema_unlock(&h_startdriversema);
		if(b_wpa_supplicant_started)
			{
        WIFI_LOG(WIFI_DEBUG , (">>>> c_net_wlan_start_driver -> driver has started! unlock\n"));
        return NET_WLAN_OK;
		}
		return NET_WLAN_SUPPLICANT_STARTING;
    }
    INT32 ret = _start_wlan_driver(pfNotify,isP2p);
    if (ret != 0)
    {
        sema_unlock(&h_startdriversema);

        WIFI_LOG(WIFI_ERROR , (">>> _start_wlan_driver return error unlock!\n"));
        return ret;
    }else
    	{
	sema_unlock(&h_startdriversema);
    WIFI_LOG(WIFI_ERROR , (">>> _start_wlan_driver return ok unlock!\n"));
    return NET_WLAN_SUPPLICANT_STARTING;
    	}
}

INT32 x_net_wlan_stop_driver(x_net_wlan_notify_fct pfNotify)
{
    INT32 count = 10;
    WIFI_LOG(WIFI_DEBUG,(" x_net_wlan_stop_driver start!\n"));
    if ( supp_pid < 0 )
    {
        //sema_uninit(&h_startdriversema);
        WIFI_LOG(WIFI_ERROR, (">>>> x_net_wlan_stop_driver -> driver not started!\n"));
        return 0;
    }
    if (pfNotify != NULL)
        _drv_notify = pfNotify;

    // exist the thread init
    if (mon_conn != NULL)
    {
        b_wlan_thread_stop = TRUE;
        while (count-- > 0)
        {
            if (!b_wlan_thread_init)
            {
                break;
            }
            else
            {
                usleep(50000);
                WIFI_LOG(WIFI_INFO ,("x_net_wlan_stop_driver wait for recv %d\n",b_wlan_thread_init));
            }
        }
		
		sema_lock(&h_WlanNfySema);
        wpa_ctrl_detach(mon_conn);
		sema_unlock(&h_WlanNfySema);
        mon_conn = NULL;
        b_wlan_thread_init = FALSE;
    }


#ifdef SUPPORT_P2P_CONCURRENT
    if (x_net_check_p2p_thread_inited())
    {
        x_net_p2p_send_to_socket();
    }
#endif
//    if (NULL != ctrl_conn )
//    {
        if (!doBooleanCommand("TERMINATE","OK"))
        {
            WIFI_LOG(WIFI_DEBUG ,(">>>>>>> %s,%d, pid is %d \n",__FUNCTION__,__LINE__,supp_pid));
            if ( _wifi_kill_supplicant() != 0)
            {
                WIFI_LOG(WIFI_ERROR,(">>>>>>> %s,%d, kill fail %s \n",__FUNCTION__,__LINE__,strerror(errno)));
                supp_pid = -1;
				b_wpa_supplicant_started = FALSE;
                return -1;
            }
        }
        else
            wait(0);
//    }
#ifdef SUPPORT_P2P_CONCURRENT
#if 0
    else if (NULL != p2p_ctrl_conn)
    {
        if (!dop2pBooleanCommand("TERMINATE","OK"))
        {
            WIFI_LOG(WIFI_DEBUG ,(">>>>>>> %s,%d, pid is %d \n",__FUNCTION__,__LINE__,supp_pid));
            if ( _wifi_kill_supplicant() != 0)
            {
                WIFI_LOG(WIFI_ERROR,(">>>>>>> %s,%d, kill fail %s \n",__FUNCTION__,__LINE__,strerror(errno)));
                supp_pid = -1;
                return -1;
            }
        }
        else
            wait(0);
    }
#endif
    x_net_p2p_close_connection();
    x_net_wlan_close_connection();
#endif
//    sema_uninit(&h_startdriversema);
    supp_pid = -1;
    b_wpa_supplicant_started = FALSE;
    b_start_driver_thread = FALSE;
    return 0;
}


int _p2p_kill_supplicant()
{
    if (p2p_supp_pid > 0)
        return _wifi_kill(p2p_supp_pid);
    WIFI_LOG(WIFI_DEBUG,("[P2P]>>>>>>> %s,%d, pid is <0 %d \n",__FUNCTION__,__LINE__,p2p_supp_pid));
    return 0;
}



INT32 x_net_p2p_isSupplicantReady()
{
    char reply[256] = {'\0'};
    if (dop2pStringCommand("PING", reply, sizeof(reply)) < 0)
    {
        WIFI_LOG(WIFI_ERROR, (">>>>>dop2pStringCommand ping error\n"));
        return -1;
    }
    else
    {
        if (strncmp(reply, "PONG", 4) == 0)
        {
            WIFI_LOG(WIFI_DEBUG, (">>>>>Supplicant is ready!\n"));
            return 0;
        }
        else
        {
            WIFI_LOG(WIFI_DEBUG, (">>>>>Supplicant is not ready!\n"));
            return 1;
        }
    }
}




static void* _start_p2p_supplicant_thread(VOID * pvArg)
{
    NET_802_11_NFY_T t_nfy;
    int if_id = 0;
    char _cmd_path[50] = {0};
    char _cmd_interface[20] = {0};
    int count  = 10;
    char _if_cmd[50] = {0};

    prctl(PR_SET_NAME,"startp2psupplicant",0,0,0);
    x_strcpy(_if_cmd , "/sbin/ifconfig ra0 up");
    doScriptCmd(_if_cmd);
    if (errno != 0)
    {
        WIFI_LOG(WIFI_ERROR, (">>>>>_ifconfif ra0 up error %s!\n",strerror(errno)));
        return NULL;
    }
    for (; if_id < WIFI_DIRECT_DEV_NUM; if_id++)
    {
        if (wifi0_check(if_name_map[if_id][WIFI_INTERFACE_P2P_NAME]))
        {
            WIFI_LOG(WIFI_DEBUG ,(">>>>>>> %s,%d,find interface %s\n",__FUNCTION__,__LINE__,if_name_map[if_id][WIFI_INTERFACE_NAME]));
            memset(p2p_name , 0 , WLAN_WLAN_IF_MAX_LEN);
            strncpy(p2p_name , if_name_map[if_id][WIFI_INTERFACE_P2P_NAME],WLAN_WLAN_IF_MAX_LEN);
            snprintf(_cmd_path,sizeof(_cmd_path),"%s/wpa_supplicant",if_name_map[if_id][WIFI_INTERFACE_SUPPLICANT_PATH]);
            snprintf(_cmd_interface,sizeof(_cmd_interface),"-i%s",p2p_name);

            WIFI_LOG(WIFI_DEBUG ,(">>>>>>> %s,%d,cmd_path %s\n",__FUNCTION__,__LINE__,_cmd_path));
            WIFI_LOG(WIFI_DEBUG ,(">>>>>> %s,%d,cmd interface %s\n",__FUNCTION__,__LINE__,_cmd_interface));
            break;
        }
    }
    do
    {
        if (if_id == WIFI_DIRECT_DEV_NUM)
        {
            WIFI_LOG(WIFI_DEBUG , (">>>>>_strart_wlan_driver_thread; no any device exist!\n"));
            break;
        }

        char * argv[] = {"wpa_supplicant","-Dwext",_cmd_interface,"-c",WPA_SUPPLICANT_PATH"/p2p.conf","-dd",(char*)0};

        if ((p2p_supp_pid = _start_process(_cmd_path,argv)) < 0)
        {
            WIFI_LOG(WIFI_ERROR , (">>>>>_strart_p2p_driver_thread call system return error! %s \n",strerror(errno)));
            break;
        }

        WIFI_LOG(WIFI_DEBUG ,("====="));
        while (count --> 0)
        {
            usleep(500000);
            if (x_net_p2p_open_connection() != 0)
                continue;
            break;
        }
        if (count == 0)
        {
            WIFI_LOG(WIFI_ERROR , (">>>>>_strart_p2p_driver_thread; open_connection fail!\n"));
            break;
        }

        if (_p2p_drv_notify)
        {
            int ret = 0;
            int i = 0;
            for (i = 0; i < 10; i++)
            {
                ret = x_net_p2p_isSupplicantReady();
                if (ret == 0)
                {
                    t_nfy.i4NotifykId = WLAN_NOTIFY_P2P_SUPP_START_IND;
                    t_nfy.pParam[0] = WLAN_NFY_MSG_OK;
                    (_p2p_drv_notify)(t_nfy.i4NotifykId , t_nfy.pParam);
                    break;
                }
                else if (ret == -1)
                {
                    continue;
                }
                else //supplicant not ready!
                {
                    WIFI_LOG(WIFI_DEBUG, (">>>>>Don't receive pong, try again!\n"));
                }
            }
            //start driver exec shell script return ok , then keep p2p device name , we will use it later .
        }
        b_start_p2p_supp_thread = FALSE;
        return NULL;
    }
    while (0);

    x_net_p2p_stop_supplicant(NULL);

    if (_p2p_drv_notify)
    {
        t_nfy.i4NotifykId = WLAN_NOTIFY_P2P_SUPP_START_FAILED;
        t_nfy.pParam[0] = WLAN_NFY_MSG_INIT_FAIL;
        (_p2p_drv_notify)(t_nfy.i4NotifykId , t_nfy.pParam);
    }
    b_start_p2p_supp_thread = FALSE;
    return NULL;
}

INT32 _start_p2p_supplicant(x_net_wlan_notify_fct pfNotify)
{
    pthread_t    h_wlan_thread;

    if (FALSE == b_start_p2p_supp_thread)
    {
        b_start_p2p_supp_thread = TRUE;
        WIFI_LOG(WIFI_DEBUG ,("_start_p2p_supplicant.\n"));
        _p2p_drv_notify = pfNotify;

        if (pthread_create(&h_wlan_thread,NULL,
                           _start_p2p_supplicant_thread, NULL) != OSR_OK)
        {
            WIFI_LOG(WIFI_DEBUG ,("x_thread_create.p2p Fail\n"));
            return (-1);
        }
    }
    return 0;
}

INT32 x_net_p2p_start_supplicant(x_net_wlan_notify_fct pfNotify)
{
    WIFI_LOG(WIFI_DEBUG ,("x_net_p2p_start_supplicant"));

    if ( p2p_supp_pid > 0 )
    {
        WIFI_LOG(WIFI_DEBUG , (">>>> x_net_p2p_start_supplicant -> driver has started!\n"));
        return 0;
    }
    INT32 ret = _start_p2p_supplicant(pfNotify);
    if (ret != 0)
    {
        WIFI_LOG(WIFI_DEBUG , (">>> x_net_p2p_start_supplicant -> _start_p2p_supplicant  return error!\n"));
        return -1;
    }
    return 0;
}

INT32 x_net_p2p_stop_supplicant(x_net_wlan_notify_fct pfNotify)
{
    x_net_p2p_send_to_socket();

    WIFI_LOG(WIFI_DEBUG ,("x_net_wlan_stop_supplicant start!\n"));
    if ( p2p_supp_pid < 0 )
    {
        WIFI_LOG(WIFI_DEBUG , (">>>> x_net_wlan_stop_supplicant -> supplicant not started!\n"));
        return 0;
    }

    if (pfNotify != NULL)
        _p2p_drv_notify = pfNotify;

    if (NULL != p2p_ctrl_conn )
    {
        if (!dop2pBooleanCommand("TERMINATE","OK"))
            WIFI_LOG(WIFI_ERROR ,(">>>>>>> %s,%d, terminate fail\n",__FUNCTION__,__LINE__));
    }

    WIFI_LOG(WIFI_DEBUG ,(">>>>>>> %s,%d, pid is %d \n",__FUNCTION__,__LINE__,p2p_supp_pid));
    if ( _p2p_kill_supplicant() != 0)
    {
        WIFI_LOG(WIFI_ERROR ,(">>>>>>> %s,%d, kill fail %s \n",__FUNCTION__,__LINE__,strerror(errno)));
        return -1;
    }

    x_net_p2p_close_connection();

    p2p_supp_pid = -1;
    return 0;
}
#endif
