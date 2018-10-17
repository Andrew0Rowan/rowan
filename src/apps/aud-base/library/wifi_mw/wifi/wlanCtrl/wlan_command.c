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
 * $RCSfile: wlan_command.c,v $
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

#include "wifi_com.h"

#include "wlan_command.h"
#include "wifi_log.h"

#if 0//PLATFORM_IS_BDP
#include "u_ipcd.h"
#endif

#include <stdlib.h>
#include <string.h>

extern struct wpa_ctrl *ctrl_conn;
extern struct wpa_ctrl *p2p_ctrl_conn;

extern sem_t  g_hWlanCmdSema;
extern int g_isWlanSemaInit;


SEMA_HANDLE h_WlanCmdSema={.is_semainited=false};
SEMA_HANDLE h_P2pCmdSema={.is_semainited=false};
SEMA_HANDLE h_WlanNfySema={.is_semainited=false};
SEMA_HANDLE h_P2pNfySema={.is_semainited=false};

int wifi_sema_init(SEMA_HANDLE * handle)
{
    int ret;

    if (handle == NULL)
    {
        return -1;
    }
    ret = sem_init(&(handle->hSemalock), 0, 1);
    if (ret != OSR_OK)
    {
        return -1;
    }
    handle->is_semainited = true;
    return 0;
}

int sema_uninit(SEMA_HANDLE * handle)
{
    int ret;

    if (handle == NULL)
    {
        return -1;
    }
    if (handle->is_semainited)
    {
        ret = sem_destroy(&(handle->hSemalock));
        if (ret != OSR_OK)
        {
            return -1;
        }
    }
    handle->is_semainited = false;
    return 0;
}

int sema_lock(SEMA_HANDLE * handle)
{
    int ret;
    if (handle == NULL)
    {
        return -1;
    }
    if (handle->is_semainited)
    {
        ret = sem_wait(&(handle->hSemalock));
        if (ret != OSR_OK)
        {
            return -1;
        }
    }
    return 0;
}

int sema_unlock(SEMA_HANDLE * handle)
{
    int ret;
    if (handle == NULL)
    {
        return -1;
    }
    if (handle->is_semainited)
    {
        ret = sem_post(&(handle->hSemalock));
        if (ret != OSR_OK)
        {
            return -1;
        }
    }
    return 0;
}


int wifi_wait_for_event(struct wpa_ctrl  *ctrl, char* buf, size_t buf_len)
{
    int ret;
    size_t nread = buf_len-1;

    if (ctrl == NULL)
    {
        WIFI_LOG (WIFI_DEBUG,(">>>>>>> %s,%d,connect loss \n",__FUNCTION__,__LINE__));
        return -1;
    }
    ret = wpa_ctrl_recv(ctrl, buf, &nread);
    if (ret < 0)
    {
        WIFI_LOG (WIFI_DEBUG,(">>>>>>> %s,%d,receive error \n",__FUNCTION__,__LINE__));
        strncpy(buf, WPA_EVENT_TERMINATING " - recv error", buf_len-1);
        buf[buf_len-1] = '\0';
        return strlen(buf);
    }

    buf[nread] = '\0';

    if (ret == 0 && nread == 0)
    {
        WIFI_LOG (WIFI_DEBUG,(">>>>>>> %s,%d,receive eof \n",__FUNCTION__,__LINE__));
        strncpy(buf, WPA_EVENT_TERMINATING " - signal 0 received", buf_len-1);
        buf[buf_len-1] = '\0';
        return strlen(buf);
    }
    //strip <N> printf buf
    if (buf[0] == '<')
    {
        char *match = strchr(buf, '>');
        if (match != NULL)
        {
            nread -= (match+1-buf);

            if (!(strncmp(match+1, WPA_EVENT_BSS_ADDED, strlen(WPA_EVENT_BSS_ADDED)) == 0 ||
                    strncmp(match+1, WPA_EVENT_BSS_REMOVED, strlen(WPA_EVENT_BSS_REMOVED)) == 0 ||
                    strncmp(match+1, WPS_EVENT_ENROLLEE_SEEN, strlen(WPS_EVENT_ENROLLEE_SEEN)) == 0))
            {
                WIFI_LOG(WIFI_DEBUG,("Received event: %s\n", buf));
            }
            memmove(buf, match+1, nread+1);
        }
    }

    return nread;
}

int doCommand(const char *cmd, char *replybuf, int replybuflen)
{
	if(sema_lock(&h_WlanCmdSema) != 0)
	{
		WIFI_LOG(WIFI_ERROR,("Lock h_WlanCmdSema fail!\n"));
		return -1;
	}

    size_t reply_len = replybuflen - 1;

    if (wifi_command(cmd, replybuf, &reply_len) != 0)
    {
		sema_unlock(&h_WlanCmdSema);
        return -1;
    }
    else
    {
        if (reply_len > 0 && replybuf[reply_len-1] == '\n')
            replybuf[reply_len-1] = '\0';
        else
            replybuf[reply_len] = '\0';		
		sema_unlock(&h_WlanCmdSema);
        return 0;
    }
}

int doIntCommand(const char *cmd)
{
    char reply[256] = {0};

    if (doCommand(cmd, reply, sizeof(reply)) != 0)
    {
        return -1;
    }
    else
    {
        return atoi(reply);
    }
}

bool doBooleanCommand(const char *cmd, const char *expect)
{
    char reply[256]= {0};

    if (doCommand(cmd, reply, sizeof(reply)) != 0)
    {
        return false;
    }
    else
    {
        WIFI_LOG (WIFI_DEBUG, (">>>>>>> %s,%d,REPLY IS= %s \n",__FUNCTION__,__LINE__ ,reply));
        return (strcmp(reply, expect) == 0);
    }
}

char* doStringCommand(const char *cmd ,char *reply ,size_t *reply_len)
{

    if (doCommand(cmd, reply, *reply_len) != 0)
    {
        WIFI_LOG (WIFI_DEBUG, (">>>>>>> %s,%d,cmd= %s return string NULL\n",__FUNCTION__,__LINE__ ,cmd));
        return NULL;
    }
    else
    {
        return reply;
    }
}

int wifi_send_command(struct wpa_ctrl *ctrl, const char *cmd, char *reply, size_t *reply_len)
{
    int ret = -1;

    if (ctrl == NULL)
    {
        WIFI_LOG (WIFI_DEBUG, (">>>>>>> %s,%d,connect miss\n",__FUNCTION__,__LINE__));

        return -1;
    }

    WIFI_LOG (WIFI_DEBUG,(">>>>>>> %s,%d,cmd= %s\n",__FUNCTION__,__LINE__ ,cmd));
    ret = wpa_ctrl_request(ctrl, cmd, strlen(cmd), reply, reply_len, NULL);
    if (ret == -2)
    {
        return -2;
    }
    else if (ret < 0 || strncmp(reply, "FAIL", 4) == 0)
    {
        return -1;
    }
    if (strncmp(cmd, "PING", 4) == 0)
    {
        reply[*reply_len] = '\0';
    }
    return 0;
}

int wifi_command(const char *cmd, char *reply, size_t *reply_len)
{    
    return wifi_send_command(ctrl_conn,cmd,reply,reply_len);
}


int p2p_command(const char *cmd, char *reply, size_t *reply_len)
{
    return wifi_send_command(p2p_ctrl_conn,cmd,reply,reply_len);
}

int dop2pCommand(const char *cmd, char *replybuf, int replybuflen)
{
    size_t reply_len = replybuflen - 1;

	if(sema_lock(&h_P2pCmdSema) != 0)
	{
		WIFI_LOG(WIFI_ERROR,("[P2P]Lock h_WlanCmdSema fail!\n"));
		return -1;
	}


    if (p2p_command(cmd, replybuf, &reply_len) != 0)
    {
        sema_unlock(&h_P2pCmdSema);
        return -1;
    }

    else
    {
        if (reply_len > 0 && replybuf[reply_len-1] == '\n')
            replybuf[reply_len-1] = '\0';
        else
            replybuf[reply_len] = '\0';
        sema_unlock(&h_P2pCmdSema);
        return 0;
    }
}


bool dop2pBooleanCommand(const char *cmd, const char *expect)
{
    char reply[256];

    if (dop2pCommand(cmd, reply, sizeof(reply)) != 0)
    {
        return FALSE;
    }
    else
    {
        WIFI_LOG(WIFI_DEBUG,("[P2P]>>>>>>> %s,%d,REPLY IS= %s \n",__FUNCTION__,__LINE__ ,reply));
        return (strcmp(reply, expect) == 0);
    }
}

int dop2pStringCommand(const char *cmd ,char *reply ,size_t reply_len)
{
    if (dop2pCommand(cmd, reply, reply_len) != 0)
    {
        WIFI_LOG(WIFI_ERROR,("[P2P]>>>>>>> %s,%d,cmd= %s return string NULL\n",__FUNCTION__,__LINE__ ,cmd));
        return -1;
    }
    else
    {
        return 0;
    }
}

int doScriptCmd(char *cmd)
{
#if 0//PLATFORM_IS_BDP
        return ipcd_exec(cmd,NULL);
#else
        return system(cmd);
#endif
}


