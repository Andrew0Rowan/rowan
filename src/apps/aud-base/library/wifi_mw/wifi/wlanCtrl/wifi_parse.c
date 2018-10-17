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
 * $RCSfile: wifi_parse.c,v $
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

#include "wifi_parse.h"
#include "wifi_log.h"
#include <string.h>
#include "inc/wps_defs.h"
#include <stdlib.h>
#include "wifi_com.h"

#ifndef ETH_ALEN
#define ETH_ALEN 6
#endif
extern const char* get_if_name();
static NET_802_11_BSS_INFO_T    t_net_wlan_bss_info[WLAN_MAX_SCAN_RESULT];

extern size_t printf_decode(UINT8 *buf, size_t maxlen, const char *str);


typedef struct _STR_KEY_MAP_
{
    char str[40];
    int  key;
}STR_KEY_MAP;

int str_key_map (STR_KEY_MAP *strumap, int size, char* str, int* key)
{
    int i = 0;
    for (i = 0; i < size; i++)
    {
        if (0 == strncmp (str, strumap[i].str, strlen (strumap[i].str)))
        {
            *key = strumap[i].key;
            WIFI_LOG(WIFI_DEBUG, ("find the key [%d][%s]", *key,strumap[i].str));
            return 0;
        }
    }
    WIFI_LOG(WIFI_ERROR, ("can not find the map key"));
    return -1;
}


int frequency_to_channel(int freq)
{
    int _channel = -1;

    if (freq >= 2412 && freq <= 2472) /* 2.407 GHz, channels 1..13 */
    {
        if ((freq - 2407) % 5)
            _channel = -1;
        else
            _channel = ((freq - 2407) / 5);
    } 
    else if (freq == 2484) /* channel 14 */
    {
        _channel = 14;
    } 
    else if (freq >= 5000 && freq <= 6000) /* 5 GHz */
    {
        if ((freq - 5000) % 5)
            _channel = -1;
        else
            _channel = ((freq - 5000) / 5);
    } 
    else if (freq >= 58320 && freq <= 64800) /* 60 GHz, channels 1..4 */
    {
        if ((freq - 58320) % 2160)
            _channel = -1;
        else
            _channel = ((freq - 56160) / 2160);
    }
    else if (freq >= 4900 && freq < 5000)
    {
        if ((freq - 4000) % 5)
            _channel = -1;
        else
            _channel = ((freq - 4000) / 5);
    }
    else
    {
        _channel = -1;
    }

    if ( _channel == -1 )
    {
        WIFI_LOG(WIFI_ERROR, ("Frequency %d is invalid.", freq));
        return -1;
    }

    return _channel;
}


int channel_to_frequency(int channel)
{
    if (channel >= 1 && channel <= 13)
    {
        return (channel * 5 + 2407);
    }
    else if (channel == 14)
    {
        return 2484;
    }
    else if (channel >= 34 && channel <= 165)
    {
        return (5000 + channel * 5);
    }
    else
    {
        WIFI_LOG(WIFI_ERROR, ("Channel %d is invalid.", channel));
    }
    
    return -1;
}

int get_protocal_type(NET_802_11_T * p_e_802_11_type, char * p80211type)
{
    if (p80211type == NULL)
        return -1;

    INT32 u4Len = strlen(p80211type);

    WIFI_LOG(WIFI_INFO, (" p80211type = %s, u4len =%d.", p80211type, u4Len));
    if (strncmp(p80211type, "802.", 4) == 0)
    {
        if (u4Len == 7)
        {
            if (strncmp(p80211type, "802.11a", 7) == 0)
            {
                *p_e_802_11_type = OID_802_11_A;
            }
            else if (strncmp(p80211type, "802.11b", 7) == 0)
            {
                *p_e_802_11_type = OID_802_11_B;
            }
            else if (strncmp(p80211type, "802.11g", 7) == 0)
            {
                *p_e_802_11_type = OID_802_11_G;
            }

            else if (strncmp(p80211type, "802.11n", 7) == 0)
            {
                *p_e_802_11_type = OID_802_11_N;
            }
        }
        else if (u4Len == 9)
        {
            if (strncmp(p80211type, "802.11b/g", 9) == 0)
            {
                *p_e_802_11_type = OID_802_11_B_G;
            }
            else if (strncmp(p80211type, "802.11g/n", 9) == 0)
            {
                *p_e_802_11_type = OID_802_11_G_N;
            }
            else if (strncmp(p80211type, "802.11a/n", 9) == 0)
            {
                *p_e_802_11_type = OID_802_11_A_N;
            }
        }
        else if (u4Len == 11)
        {
            if (strncmp(p80211type, "802.11b/g/n", 11) == 0)
            {
                *p_e_802_11_type = OID_802_11_B_G_N;
            }
            else if (strncmp(p80211type, "802.11a/b/g", 11) == 0)
            {
                *p_e_802_11_type = OID_802_11_A_B_G;
            }
            else if (strncmp(p80211type, "802.11a/g/n", 11) == 0)
            {
                *p_e_802_11_type = OID_802_11_A_G_N;
            }
        }
        else if (u4Len == 13)
        {
            if (strncmp(p80211type, "802.11a/b/g/n", 13) == 0)
            {
                *p_e_802_11_type = OID_802_11_A_B_G_N;
            }
        }
        else
        {
            return -1;
        }
    }
    else
    {
        if (u4Len == 3)
        {
            if (strncmp(p80211type, "11a", 3) == 0)
            {
                *p_e_802_11_type = OID_802_11_A;
            }
            else if (strncmp(p80211type, "11b", 3) == 0)
            {
                *p_e_802_11_type = OID_802_11_B;
            }
            else if (strncmp(p80211type, "11g", 3) == 0)
            {
                *p_e_802_11_type = OID_802_11_G;
            }

            else if (strncmp(p80211type, "11n", 3) == 0)
            {
                *p_e_802_11_type = OID_802_11_N;
            }
        }
        else if (u4Len == 5)
        {
            if (strncmp(p80211type, "11b/g", 5) == 0)
            {
                *p_e_802_11_type = OID_802_11_B_G;
            }
            else if (strncmp(p80211type, "11g/n", 5) == 0)
            {
                *p_e_802_11_type = OID_802_11_G_N;
            }
            else if (strncmp(p80211type, "11a/n", 5) == 0)
            {
                *p_e_802_11_type = OID_802_11_A_N;
            }
        }
        else if (u4Len == 7)
        {
            if (strncmp(p80211type, "11b/g/n", 7) == 0)
            {
                *p_e_802_11_type = OID_802_11_B_G_N;
            }
            else if (strncmp(p80211type, "11a/b/g", 7) == 0)
            {
                *p_e_802_11_type = OID_802_11_A_B_G;
            }
            else if (strncmp(p80211type, "11a/g/n", 7) == 0)
            {
                *p_e_802_11_type = OID_802_11_A_G_N;
            }
        }
        else if (u4Len == 13)
        {
            if (strncmp(p80211type, "11a/b/g/n", 9) == 0)
            {
                *p_e_802_11_type = OID_802_11_A_B_G_N;
            }
        }
        else
        {
            return -1;
        }
    }
    //WPA_LOG_EXIT(LogDefault,(" p80211type = %s, u4len =%d.", p80211type, u4Len));
    return  0;

}

static int tok_security(char * security, INT16 * mode, INT16 *cipher)
{
    char *token;
    int i4_Ret = 0;
    char *res[4] = {NULL};
    int count = 0;
    if (security == NULL)
    {
        i4_Ret = -1;
        goto out;
    }
////////////////////////////////////////////////
    token = strtok(security, "]");

    while (token != NULL)
    {
        if (token == NULL)
        {
            i4_Ret = -1;
            goto out;
        }
        token++;
        if (token == NULL)
        {
            i4_Ret = -1;
            goto out;
        }

        res[count] = malloc(strlen(token) + 1);
        if (NULL == res[count])
        {
            i4_Ret = -1;
            goto out;
        }
        memset(res[count], 0, strlen(token) + 1);
        strncpy(res[count], token, strlen(token) + 1);

        count++;
        token = strtok(NULL, "]");
    }

    int j = 0 ;
    for (j=0; j<count; j++)
    {
        WIFI_LOG(WIFI_INFO,(" tok_security res[%d]=%s\n",j,res[j]));
    }
/////////////////////////////////////////////

    if (res[0] != NULL)
    {
        switch (strlen(res[0]))
        {
        case 3:
            if (strncmp(res[0], "ESS", 3) == 0)
            {
                *mode = IEEE_802_11_AUTH_MODE_NONE;
                *cipher = IEEE_802_11_AUTH_CIPHER_NONE;
                i4_Ret = 0;
            }
            else if (strncmp(res[0], "WEP", 3) == 0)
            {
                *mode = IEEE_802_11_AUTH_MODE_WEPAUTO;
                *cipher = IEEE_802_11_AUTH_CIPHER_WEP40;
                i4_Ret = 0;
            }
            else if (strncmp(res[0], "WPS", 3) == 0)
            {
                if (res[1] != NULL && strlen(res[1]) == 3 && strncmp(res[1], "ESS", 3) == 0)
                {
                    *mode = IEEE_802_11_AUTH_MODE_NONE;
                    *cipher = IEEE_802_11_AUTH_CIPHER_NONE;

                    i4_Ret = 0;
                }
            }
            break;
        case 7:
            if (strncmp(res[0], "WPA-EAP", 7) == 0)
            {
                i4_Ret = -1;
                goto out;
            }
            break;
        case 8:
            if (strncmp(res[0], "WPA-TKIP", 8) == 0)
            {
                *mode = IEEE_802_11_AUTH_MODE_WPA;
                *cipher = IEEE_802_11_AUTH_CIPHER_TKIP;
                i4_Ret = 0;

            }
            else if (strncmp(res[0], "WPA-CCMP", 8) == 0)
            {
                *mode = IEEE_802_11_AUTH_MODE_WPA;
                *cipher = IEEE_802_11_AUTH_CIPHER_AES;
                i4_Ret = 0;
            }
            break;
        case 9:
            if (strncmp(res[0], "WPA2-TKIP", 9) == 0)
            {
                *mode = IEEE_802_11_AUTH_MODE_WPA2;
                *cipher = IEEE_802_11_AUTH_CIPHER_TKIP;
                i4_Ret = 0;

            }
            else if (strncmp(res[0], "WPA2-CCMP", 9) == 0)
            {
                *mode = IEEE_802_11_AUTH_MODE_WPA2;
                *cipher = IEEE_802_11_AUTH_CIPHER_AES;
                i4_Ret = 0;
            }
            break;
        case 12:
            if (strncmp(res[0], "WPA-PSK-TKIP", 12) == 0)
            {
                *mode = IEEE_802_11_AUTH_MODE_WPA_PSK;
                *cipher = IEEE_802_11_AUTH_CIPHER_TKIP;
                i4_Ret = 0;

            }
            else if (strncmp(res[0], "WPA-PSK-CCMP", 12) == 0)
            {
                *mode = IEEE_802_11_AUTH_MODE_WPA_PSK;
                *cipher = IEEE_802_11_AUTH_CIPHER_AES;
                i4_Ret = 0;

            }
            else if (strncmp(res[0], "WPA-EAP-CCMP", 12) == 0)
            {
                *mode = IEEE_802_11_AUTH_MODE_WPA_EAP;
                *cipher = IEEE_802_11_AUTH_CIPHER_AES;
                i4_Ret = 0;

            }
            else if (strncmp(res[0], "WPA-EAP-TKIP", 12) == 0)
            {
                *mode = IEEE_802_11_AUTH_MODE_WPA_EAP;
                *cipher = IEEE_802_11_AUTH_CIPHER_TKIP;
                i4_Ret = 0;
            }
            break;
        case 13:
            if (strncmp(res[0], "WPA2-PSK-TKIP", 13) == 0)
            {
                *mode = IEEE_802_11_AUTH_MODE_WPA2_PSK_TKIP;
                *cipher = IEEE_802_11_AUTH_CIPHER_TKIP;
                i4_Ret = 0;

            }
            else if (strncmp(res[0], "WPA2-PSK-CCMP", 13) == 0)
            {
                *mode = IEEE_802_11_AUTH_MODE_WPA2_PSK;
                *cipher = IEEE_802_11_AUTH_CIPHER_AES;
                i4_Ret = 0;

            }
            else if (strncmp(res[0], "WPA2-EAP-CCMP", 13) == 0)
            {
                *mode = IEEE_802_11_AUTH_MODE_WPA2_EAP;
                *cipher = IEEE_802_11_AUTH_CIPHER_AES;
                i4_Ret = 0;


            }
            else if (strncmp(res[0], "WPA2-EAP-TKIP", 13) == 0)
            {
                *mode = IEEE_802_11_AUTH_MODE_WPA2_EAP;
                *cipher = IEEE_802_11_AUTH_CIPHER_TKIP;
                i4_Ret = 0;

            }
            break;
        case 17:
            if (strncmp(res[0], "WPA-PSK-TKIP+CCMP", 17) == 0)
            {
                WIFI_LOG(WIFI_DEBUG, ("wpa-psk-tkip+ccmp\n"));
                *mode = IEEE_802_11_AUTH_MODE_WPA_PSK;
                *cipher = IEEE_802_11_AUTH_CIPHER_AES_OR_TKIP;
                i4_Ret = 0;
            }
            else if (strncmp(res[0], "WPA-PSK-CCMP+TKIP", 17) == 0)
            {
                *mode = IEEE_802_11_AUTH_MODE_WPA_PSK;
                *cipher = IEEE_802_11_AUTH_CIPHER_AES_OR_TKIP;
                i4_Ret = 0;
            }
            else if (strncmp(res[0], "WPA-EAP-TKIP+CCMP", 17) == 0)
            {
                *mode = IEEE_802_11_AUTH_MODE_WPA_EAP;
                *cipher = IEEE_802_11_AUTH_CIPHER_AES_OR_TKIP;
                i4_Ret = 0;
            }
            else if (strncmp(res[0], "WPA-EAP-CCMP+TKIP", 17) == 0)
            {
                *mode = IEEE_802_11_AUTH_MODE_WPA_EAP;
                *cipher = IEEE_802_11_AUTH_CIPHER_AES_OR_TKIP;
                i4_Ret = 0;
            }
            break;
        case 18:
            if (strncmp(res[0], "WPA2-PSK-TKIP+CCMP", 18) == 0)
            {
                *mode = IEEE_802_11_AUTH_MODE_WPA2_PSK;
                *cipher = IEEE_802_11_AUTH_CIPHER_AES_OR_TKIP;
                i4_Ret = 0;

            }
            else if (strncmp(res[0], "WPA2-PSK-CCMP+TKIP", 18) == 0)
            {
                *mode = IEEE_802_11_AUTH_MODE_WPA2_PSK;
                *cipher = IEEE_802_11_AUTH_CIPHER_AES_OR_TKIP;
                i4_Ret = 0;

            }
            else if (strncmp(res[0], "WPA2-EAP-TKIP+CCMP", 18) == 0)
            {
                *mode = IEEE_802_11_AUTH_MODE_WPA2_EAP;
                *cipher = IEEE_802_11_AUTH_CIPHER_AES_OR_TKIP;
                i4_Ret = 0;
            }
            else if (strncmp(res[0],"WPA-PSK-CCMP+GTKIP",18) == 0)
            {
                *mode = IEEE_802_11_AUTH_MODE_WPA_PSK;
                *cipher = IEEE_802_11_AUTH_CIPHER_AES;
                i4_Ret = 0;
            }
            break;
        case 19:
            if (strncmp(res[0],"WPA2-PSK-CCMP+GTKIP",19) == 0)
            {
                *mode = IEEE_802_11_AUTH_MODE_WPA2_PSK;
                *cipher = IEEE_802_11_AUTH_CIPHER_AES;
                i4_Ret = 0;
            }
        case 20:
            if (strncmp(res[0],"WPA-PSK-TKIP+GWEP104",20) == 0)
            {
                *mode = IEEE_802_11_AUTH_MODE_WPA_PSK;
                *cipher = IEEE_802_11_AUTH_CIPHER_TKIP;
                i4_Ret = 0;
            }
            break;
        case 21:
            if (strncmp(res[0],"WPA2-PSK-TKIP-preauth",21) == 0)
            {
                *mode = IEEE_802_11_AUTH_MODE_WPA2_PSK_TKIP;
                *cipher = IEEE_802_11_AUTH_CIPHER_TKIP;
                i4_Ret = 0;
            }
            else if (strncmp(res[0],"WPA2-PSK-CCMP-preauth",21) == 0)
            {
                *mode = IEEE_802_11_AUTH_MODE_WPA2_PSK;
                *cipher = IEEE_802_11_AUTH_CIPHER_AES;
                i4_Ret = 0;
            }
            else if (strncmp(res[0],"WPA2-EAP-CCMP-preauth",21) == 0)
            {
                *mode = IEEE_802_11_AUTH_MODE_WPA2_EAP;
                *cipher = IEEE_802_11_AUTH_CIPHER_AES;
                i4_Ret = 0;
            }
            else if (strncmp(res[0],"WPA2-EAP-TKIP-preauth",21) == 0)
            {
                *mode = IEEE_802_11_AUTH_MODE_WPA2_EAP;
                *cipher = IEEE_802_11_AUTH_CIPHER_AES;
                i4_Ret = 0;
            }
            break;
        case 23:
            if (strncmp(res[0],"WPA-PSK-TKIP+CCMP+GTKIP",23) == 0)
            {
                *mode = IEEE_802_11_AUTH_MODE_WPA_PSK;
                *cipher = IEEE_802_11_AUTH_CIPHER_AES_OR_TKIP;
                i4_Ret = 0;
            }

            break;

        case 24:
            if (strncmp(res[0],"WPA2-PSK-TKIP+CCMP+GTKIP",24) == 0)
            {
                *mode = IEEE_802_11_AUTH_MODE_WPA2_PSK;
                *cipher = IEEE_802_11_AUTH_CIPHER_AES_OR_TKIP;
                i4_Ret = 0;
            }
            break;
        case 26:
            if (strncmp(res[0],"WPA-PSK-CCMP+GTKIP-preauth",26) == 0)
            {
                *mode = IEEE_802_11_AUTH_MODE_WPA_PSK;
                *cipher = IEEE_802_11_AUTH_CIPHER_AES;
                i4_Ret = 0;
            }
			else if (strncmp(res[0],"WPA2-PSK-CCMP+TKIP-preauth",26) == 0)
            {
                *mode = IEEE_802_11_AUTH_MODE_WPA2_PSK;
                *cipher = IEEE_802_11_AUTH_CIPHER_AES_OR_TKIP;
                i4_Ret = 0;
            }
            else if (strncmp(res[0],"WPA2-PSK-TKIP+CCMP+GWEP104",26) == 0)
            {
                *mode = IEEE_802_11_AUTH_MODE_WPA2_PSK;
                *cipher = IEEE_802_11_AUTH_CIPHER_AES_OR_TKIP;
                i4_Ret = 0;
            }

            break;

        case 27:
            if (strncmp(res[0],"WPA2-PSK-CCMP+GTKIP-preauth",27) == 0)
            {
                *mode = IEEE_802_11_AUTH_MODE_WPA2_PSK;
                *cipher = IEEE_802_11_AUTH_CIPHER_AES;
                i4_Ret = 0;
            }

            break;

        case 31:
            if (strncmp(res[0],"WPA-PSK-TKIP+CCMP+GTKIP-preauth",31) == 0)
            {
                *mode = IEEE_802_11_AUTH_MODE_WPA_PSK ;
                *cipher = IEEE_802_11_AUTH_CIPHER_AES_OR_TKIP;
                i4_Ret = 0;
            }

            break;
        case 32:
            if (strncmp(res[0],"WPA2-PSK-TKIP+CCMP+GTKIP-preauth",32) == 0)
            {
                *mode = IEEE_802_11_AUTH_MODE_WPA2_PSK;
                *cipher = IEEE_802_11_AUTH_CIPHER_AES_OR_TKIP;
                i4_Ret = 0;
            }

            break;

        default:
            WIFI_LOG(WIFI_ERROR, (">>>>>>>>>>>Default res[0] !!"));
            i4_Ret = -1;
            goto out;
        }
    }
    if (res[1] != NULL)
    {
        switch (strlen(res[1]))
        {
        case 3:
            if (strncmp(res[1], "WEP", 3) == 0)
            {
                *mode = IEEE_802_11_AUTH_MODE_WEPAUTO;
                *cipher = IEEE_802_11_AUTH_CIPHER_WEP40;
                i4_Ret = 0;
            }
            break ;
        case 9:
            if (strncmp(res[1], "WPA2-TKIP", 9) == 0)
            {
                i4_Ret = -1;
                goto out;
            }
            else if (strncmp(res[1], "WPA2-CCMP", 9) == 0)
            {
                i4_Ret = -1;
                goto out;
            }
            break;
        case 13:
            if (strncmp(res[1], "WPA2-PSK-TKIP", 13) == 0)
            {
                if (*mode == IEEE_802_11_AUTH_MODE_WPA_PSK)
                {
                    *mode = IEEE_802_11_AUTH_MODE_WPAPSK_OR_WPA2PSK;
                    *cipher = IEEE_802_11_AUTH_CIPHER_TKIP;
                    i4_Ret = 0;
                }
            }
            else if (strncmp(res[1], "WPA2-PSK-CCMP", 13) == 0)
            {
                if (*mode == IEEE_802_11_AUTH_MODE_WPA_PSK)
                {
                    *mode = IEEE_802_11_AUTH_MODE_WPAPSK_OR_WPA2PSK;
                    *cipher = IEEE_802_11_AUTH_CIPHER_AES;
                    i4_Ret = 0;
                }
            }
            else if (strncmp(res[1], "WPA2-EAP-TKIP", 13) == 0)
            {
                if (*mode == IEEE_802_11_AUTH_MODE_WPA_EAP)
                {
                    *mode = IEEE_802_11_AUTH_MODE_WPAEAP_OR_WPA2EAP;
                    *cipher = IEEE_802_11_AUTH_CIPHER_TKIP;
                    i4_Ret = 0;
                }

            }
            else if (strncmp(res[1], "WPA2-EAP-CCMP", 13) == 0)
            {
                if (*mode == IEEE_802_11_AUTH_MODE_WPA_EAP)
                {
                    *mode = IEEE_802_11_AUTH_MODE_WPAEAP_OR_WPA2EAP;
                    *cipher = IEEE_802_11_AUTH_CIPHER_AES;
                    i4_Ret = 0;
                }
            }
            break;
        case 18:
            if (strncmp(res[1], "WPA2-PSK-TKIP+CCMP", 18) == 0)
            {
                if (*mode == IEEE_802_11_AUTH_MODE_WPA_PSK)
                {
                    *mode = IEEE_802_11_AUTH_MODE_WPAPSK_OR_WPA2PSK;
                    *cipher = IEEE_802_11_AUTH_CIPHER_AES_OR_TKIP;
                    i4_Ret = 0;
                }
            }
            else if (strncmp(res[1], "WPA2-EAP-TKIP+CCMP", 18) == 0)
            {
                if (*mode == IEEE_802_11_AUTH_MODE_WPA_EAP)
                {
                    *mode = IEEE_802_11_AUTH_MODE_WPAEAP_OR_WPA2EAP;
                    *cipher = IEEE_802_11_AUTH_CIPHER_AES_OR_TKIP;
                    i4_Ret = 0;
                }
            }
            else if (strncmp(res[1], "WPA2-PSK-CCMP+TKIP", 18) == 0)
            {
                if (*mode == IEEE_802_11_AUTH_MODE_WPA_PSK)
                {
                    *mode = IEEE_802_11_AUTH_MODE_WPAPSK_OR_WPA2PSK;
                    *cipher = IEEE_802_11_AUTH_CIPHER_AES_OR_TKIP;
                    i4_Ret = 0;
                }
            }
            else if (strncmp(res[1], "WPA2-EAP-CCMP+TKIP", 18) == 0)
            {
                if (*mode == IEEE_802_11_AUTH_MODE_WPA_EAP)
                {
                    *mode = IEEE_802_11_AUTH_MODE_WPAEAP_OR_WPA2EAP;
                    *cipher = IEEE_802_11_AUTH_CIPHER_AES_OR_TKIP;
                    i4_Ret = 0;
                }
            }
            break;

        case 19:
            if (strncmp(res[1],"WPA2-PSK-CCMP+GTKIP",19) == 0)
            {
                if ( *mode == IEEE_802_11_AUTH_MODE_WPA_PSK  )
                {
                    *mode = IEEE_802_11_AUTH_MODE_WPAPSK_OR_WPA2PSK ;
                    *cipher = IEEE_802_11_AUTH_CIPHER_AES;
                    i4_Ret = 0;
                }
            }

            break;

        case 21:
            if (strncmp(res[1],"WPA2-PSK-TKIP-preauth",21) == 0)
            {
                if (*mode == IEEE_802_11_AUTH_MODE_WPA_PSK)
                {
                    *mode = IEEE_802_11_AUTH_MODE_WPAPSK_OR_WPA2PSK;
                    *cipher = IEEE_802_11_AUTH_CIPHER_TKIP;
                    i4_Ret = 0;
                }
            }
            else if (strncmp(res[1],"WPA2-PSK-CCMP-preauth",21) == 0)
            {
                if (*mode == IEEE_802_11_AUTH_MODE_WPA_PSK)
                {
                    *mode = IEEE_802_11_AUTH_MODE_WPAPSK_OR_WPA2PSK;
                    *cipher = IEEE_802_11_AUTH_CIPHER_AES;
                    i4_Ret = 0;
                }
            }
            break;
        case 23:
            if (strncmp(res[1],"WPA-EAP-TKIP+CCMP+GTKIP",23) == 0)
            {
                *mode = IEEE_802_11_AUTH_MODE_WPA_EAP;
                *cipher = IEEE_802_11_AUTH_CIPHER_AES_OR_TKIP;
                i4_Ret = 0;
            }
            break;
        case 24:
            if (strncmp(res[1],"WPA2-PSK-TKIP+CCMP+GTKIP",24) == 0)
            {
                if (*mode == IEEE_802_11_AUTH_MODE_WPA_PSK)
                {
                    *mode = IEEE_802_11_AUTH_MODE_WPAPSK_OR_WPA2PSK ;
                    *cipher = IEEE_802_11_AUTH_CIPHER_AES_OR_TKIP;
                    i4_Ret = 0;
                }
            }
            else if (strncmp(res[1],"WPA2-EAP-TKIP+CCMP+GTKIP",24) == 0)
            {
                *mode = IEEE_802_11_AUTH_MODE_WPA2_EAP;
                *cipher = IEEE_802_11_AUTH_CIPHER_AES_OR_TKIP;
                i4_Ret = 0;
            }
            break;

        case 26:
            if (strncmp(res[0],"WPA2-PSK-TKIP+CCMP+GWEP104",26) == 0)
            {
                *mode = IEEE_802_11_AUTH_MODE_WPA2_PSK ;
                *cipher = IEEE_802_11_AUTH_CIPHER_AES_OR_TKIP;
                i4_Ret = 0;
            }

            break;
        case 27:
            if (strncmp(res[1],"WPA2-PSK-CCMP+GTKIP-preauth",27) == 0)
            {
                if (*mode == IEEE_802_11_AUTH_MODE_WPA_PSK)
                {
                    *mode = IEEE_802_11_AUTH_MODE_WPAPSK_OR_WPA2PSK ;
                    *cipher = IEEE_802_11_AUTH_CIPHER_AES;
                    i4_Ret = 0;
                }
            }
            break;

        case 32:
            if (strncmp(res[1],"WPA2-PSK-TKIP+CCMP+GTKIP-preauth",32) == 0)
            {
                if (*mode == IEEE_802_11_AUTH_MODE_WPA_PSK)
                {
                    *mode = IEEE_802_11_AUTH_MODE_WPAPSK_OR_WPA2PSK ;
                    *cipher = IEEE_802_11_AUTH_CIPHER_AES_OR_TKIP;
                    i4_Ret = 0;
                }
            }
            break;
        default:
            WIFI_LOG(WIFI_ERROR, (" >>>>>>>>>>>Default res[1]!!"));
            break;
        }
    }

    /* discard EAP mode AP*/
    if ((*mode == IEEE_802_11_AUTH_MODE_WPAEAP_OR_WPA2EAP)
            ||(*mode == IEEE_802_11_AUTH_MODE_WPA2_EAP)
            ||(*mode == IEEE_802_11_AUTH_MODE_WPA_EAP))
    {
        WIFI_LOG(WIFI_ERROR, (" Not support security mode, discard"));
        i4_Ret = -1;
    }
    /* discard EAP mode AP*/
    int i;
out:
    for (i = 0;i < count;i++)
    {
        if (res[i] != NULL)
        {
            free(res[i]);
            res[i] = NULL;
        }
    }

    return i4_Ret;

}

static int tok_wps_support(char * security)
{
    if (security == NULL)
    {
        return -1;
    }
    if (strstr(security , "[WPS]") != NULL)
    {
        return 1;
    }
    return 0;
}
static int get_scan_info_num (char* s_scan_header)
{
    char * pos = NULL;
    char *end = NULL;
    pos = s_scan_header;
    end  = s_scan_header;
    int count  = 1;

    int len = 0;
    char ch[40] = {0};
    while (end != NULL)
    {
        end = strstr(end, "/");

        if (end == NULL)
        {
            break;
        }
        len = end -pos;
        strncpy(ch, pos, len);

        end++;
        pos = end;
        count++;
    }
    return count;

}

UINT32 get_scan_total_num(char * buf)
{
    UINT32 u4_scan_result_num = 0;
    char *end = NULL;
    end = buf;
    end = strstr(end, "\n");

    if (end == NULL)
    {
        return -1;
    }
    else
    {
        end++;
        if ((end != NULL) && (*end != '\0'))
        {
            u4_scan_result_num++;
        }
    }

    while (end != NULL)
    {
        end = strstr(end, "\n");
        if (end == NULL)
        {
            break;
        }
        else
        {
            end++;
            if ((end != NULL) && (*end != '\0'))
            {
                u4_scan_result_num++;
            }
        }
    }
    return u4_scan_result_num;
}

int proc_scan_bssid(char *str, NET_802_11_BSS_INFO_T *p_bss_info)
{
    if (str == NULL || p_bss_info == NULL)
    {
        WIFI_LOG(WIFI_DEBUG,  ("proc_scan_bssid param error\n"));
        return -1;
    }
    return str_to_mac (str, p_bss_info->t_Bssid);
}

int proc_scan_ssid(char *str, NET_802_11_BSS_INFO_T *p_bss_info)
{
    UINT8 ConvertedSsid[NET_802_11_MAX_LEN_SSID + 1] = {0};

    printf_decode(ConvertedSsid, sizeof(ConvertedSsid), str);

    memcpy (p_bss_info->t_Ssid.ui1_aSsid, ConvertedSsid, NET_802_11_MAX_LEN_SSID);
    p_bss_info->t_Ssid.ui4_SsidLen = strlen (p_bss_info->t_Ssid.ui1_aSsid);

    // if the ssid over NET_802_11_MAX_LEN_SSID,it out of range ,so we use the length tell the apps ,not use modify the struct lili
    if (p_bss_info->t_Ssid.ui4_SsidLen > NET_802_11_MAX_LEN_SSID)
    {
        p_bss_info->t_Ssid.ui4_SsidLen = NET_802_11_MAX_LEN_SSID;
    }

    return 0;
}
int u1WlanK2Drv_NormalizeSignal(int signal)
{
    int answer  = 0;
    if (signal >= 42)
        answer = 94 ;
    else if (signal >= 30)
        answer = 85 + ((94-85)*( signal -30) + (42-30)/2) / (42-30) ;
    else if (signal >= 5)
        answer = 5 + ((signal - 5) * (85-5) + (30-5)/2) / (30-5) ;
    else if (signal >= 1)
        answer = signal;
    else
        answer = 0 ;

    return (answer*100)/94; 		//covert to range0 -100
}

extern int u1WlanDrv_NormalizeSignal(int signal);


extern char   if_name[WLAN_WLAN_IF_MAX_LEN];

int proc_scan_signal_level (char *str, NET_802_11_BSS_INFO_T *p_bss_info)
{

    p_bss_info->i2_Level = (INT16)strtoll(str, NULL, 10);
    p_bss_info->i2_Level_dB = p_bss_info->i2_Level;

#if WLAN_SUPPORT_ORIGINAL_SIGNAL
    WIFI_LOG(WIFI_DEBUG, ("do not process the signal level, use the original value!"));
    return 0;
#endif

    if ((0 == strncmp(if_name, "ath0", strlen ("ath0")))
#ifdef SUPPORT_2_DONGLES
            || (0 == strncmp(if_name, "athmag0", strlen ("athmag0")))
#endif
       )
    {
        p_bss_info->i2_Level = u1WlanK2Drv_NormalizeSignal(p_bss_info->i2_Level);
    }
    else if  ((0 == strncmp(if_name, "ra0", strlen ("ra0"))) || (0 == strncmp(if_name, "wlan0", strlen ("wlan0"))))
    {
        p_bss_info->i2_Level = u1WlanDrv_NormalizeSignal(p_bss_info->i2_Level);
    }
    else
    {
        WIFI_LOG (WIFI_DEBUG, ("unknown wifi interface prco %s",if_name));
    }
    return 0;
}

int proc_scan_protocol_caps(char *str, NET_802_11_BSS_INFO_T *p_bss_info)
{
    char            protocol[ 32 ] = { '\0' };
    char            *pos = NULL;
    char            *end = NULL;
    char            *pBW = NULL;
    int             len = 0;
    NET_802_11_T    e_802_11_type = OID_802_11_B;
    INT32           i4_Ret = -1;

    if ( !str || !p_bss_info )
    {
        WIFI_LOG (WIFI_ERROR, ("Invalid parameters! %s, %d", __FUNCTION__, __LINE__ ));
        return -1;
    }

    pos = str;
    end = pos + strlen( str );

    /* bandwidth */
    pBW = strstr( pos, "BW20" );
    if ( pBW )
    {
        p_bss_info->e_802_11_bw = OID_802_11_BW20;
    }
    else
    {
        pBW = strstr( pos, "BW40" );
        if ( pBW )
        {
            p_bss_info->e_802_11_bw = OID_802_11_BW40;
        }
        else
        {
            WIFI_LOG(WIFI_ERROR, ("can not find \"bandwidth\"!\n"));
        }
    }

    /* protocol */
    if ( pBW )
    {
        len = pBW - pos -1;
    }
    else
    {
        len = end - pos;
    }
    strncpy(protocol, pos, len);
    protocol[sizeof(protocol) - 1] = '\0';

    i4_Ret = get_protocal_type( &e_802_11_type, protocol );
    if ( i4_Ret < 0 )
    {
        WIFI_LOG(WIFI_ERROR, ("Can not find protocol!\n"));
    }

    p_bss_info->e_802_11_type = e_802_11_type;

    return 0;
}

int proc_scan_frequence(char *str, NET_802_11_BSS_INFO_T *p_bss_info)
{
    p_bss_info->i2_Freq = (INT16)strtoll(str, NULL, 10);

    p_bss_info->i2_Channel = frequency_to_channel( p_bss_info->i2_Freq );
    return 0;
}

int proc_scan_flags(char *str, NET_802_11_BSS_INFO_T *p_bss_info)
{
    INT16 mode = 0;
    INT16 cipher = 0;

    //haifei add for parse wps support string to app layer .
    int _ret = tok_wps_support(str);
    if ( _ret == 1)
    {
        p_bss_info->is_wps_support = 1;
    }
    else if (_ret == 0 )
    {
        p_bss_info->is_wps_support = 0;
    }
    else
    {
        p_bss_info->is_wps_support = 2;
    }
    if (tok_security(str, &mode, &cipher) != 0)
    {
        WIFI_LOG(WIFI_DEBUG,  ("No security, discard!\n"));
        return -1 ;

    }/**/
    p_bss_info->e_AuthMode = mode;
    p_bss_info->e_AuthCipher = cipher;
    return 0;
}
int proc_scan_rate(char *str, NET_802_11_BSS_INFO_T *p_bss_info)
{
    p_bss_info->i4_MaxRate = (INT16)strtoll( str, NULL, 10 );

    return 0;
}

int wlan_scan_result_compare(const void *a, const void *b)
{
    const NET_802_11_BSS_INFO_T *pBSSa = (NET_802_11_BSS_INFO_T *)a;
    const NET_802_11_BSS_INFO_T *pBSSb = (NET_802_11_BSS_INFO_T *)b;
    return (pBSSb->i2_Level_dB - pBSSa->i2_Level_dB);
}

INT32 wpa_get_scan_results_from_ret_buf(char * buf,NET_802_11_SCAN_RESULT_T *pScanResult)
{
    UINT32 u4_scan_result_num = 0;
    UINT32 u4_BssInfo_len = 0;
    if (NULL == buf)
    {
        WIFI_LOG(WIFI_ERROR, ("No scan result \n"));
        return -1;
    }

    u4_scan_result_num = get_scan_total_num(buf);
    WIFI_LOG(WIFI_DEBUG,  ("the total num of scan result u4_scan_result_num[%d]\n", (int)u4_scan_result_num));

    if (u4_scan_result_num == -1)
    {
        WIFI_LOG(WIFI_ERROR, ("No scan result content in the buf\n"));
        return -1;
    }
    if (u4_scan_result_num > WLAN_MAX_SCAN_RESULT)
    {
        u4_scan_result_num = WLAN_MAX_SCAN_RESULT;
    }

    pScanResult->ui4_NumberOfItems = u4_scan_result_num;

    u4_BssInfo_len = WLAN_MAX_SCAN_RESULT*sizeof (NET_802_11_BSS_INFO_T);
    pScanResult->p_BssInfo = t_net_wlan_bss_info;
    if (NULL == pScanResult->p_BssInfo)
    {
        WIFI_LOG(WIFI_ERROR, ("wpa_get_scan_results_from_ret_buf malloc Fail!!!\n"));
        return -1;
    }
    memset(pScanResult->p_BssInfo,0,u4_BssInfo_len);
    char *pos = NULL;
    char *end = NULL;

    end = buf;
    pos = end;


    end = strstr(end, "\n");
    if (end == NULL)
        return -1;

    int len = 0;
    char s_scan_header[256] = {0};
    len=end-pos;
    strncpy(s_scan_header,pos,len);

    WIFI_LOG(WIFI_INFO,  ("scan header = %s\n", s_scan_header));


    if (end != NULL)
    {
        end++;
        pos = end;
    }
    int i = 0;
    int cnt_in_Bssinfo = 0;
    for (i = 0; i < u4_scan_result_num; i++)
    {
        WIFI_LOG(WIFI_INFO,  ("The AP number to be parsing=%d (total: %d)\n", (i+1), u4_scan_result_num));
        int len = 0;
        if (end == NULL)
            break;
        if ((i == (u4_scan_result_num -1) ) && (end != NULL) && (*end != '\0'))
        {
            end = strstr(end, "\n");
            if ( end != NULL )
            {
                len = end - pos;
            }
            else
            {
                len = strlen(pos);
            }
        }
        else
        {
            end = strstr(end, "\n");
            if (end == NULL)
                break;
            len = end - pos;
        }

        char *res = malloc (len +1);
        if (NULL == res)
        {
            WIFI_LOG(WIFI_DEBUG,  ("alloc resource failed\n"));
            return -1;
        }

        memset (res, 0,  len + 1);
        strncpy (res, pos, len) ;

        WIFI_LOG(WIFI_INFO,  ("\nScanned AP[%d]=%s\n", i, res));

        end++;
        pos = end;
        //  parase the scan_result to the bss info
        if (tok_scan_bssid_convert (res, &pScanResult->p_BssInfo[cnt_in_Bssinfo], s_scan_header) != 0)
        {
            WIFI_LOG(WIFI_DEBUG,  ("Discard An ap\n"));
            //u4_scan_result_num --;
        }
        else
        {
            cnt_in_Bssinfo++;
        }

        if (res != NULL)
        {
            free (res);
            res = NULL;
        }
    }

    pScanResult->ui4_NumberOfItems = cnt_in_Bssinfo;

    //sort the scan results by AP's signal strength (dB)
    qsort(pScanResult->p_BssInfo, pScanResult->ui4_NumberOfItems, 
        sizeof(NET_802_11_BSS_INFO_T), wlan_scan_result_compare);

    for (i=0;i<cnt_in_Bssinfo;i++)
    {
        WIFI_LOG(WIFI_DEBUG, ("scan res:ssid[%d]=%s,AuthCip=%d, AuthMode=%d, Cha=%d,Fre=%d,Level=%d,Noise=%d,Qua=%d,MaxRate=%d, Caps=%d, ieLen=%d, Num=%d.\n",
                              i,
                              pScanResult->p_BssInfo[i].t_Ssid.ui1_aSsid,
                              pScanResult->p_BssInfo[i].e_AuthCipher,
                              pScanResult->p_BssInfo[i].e_AuthMode,
                              pScanResult->p_BssInfo[i].i2_Channel,
                              pScanResult->p_BssInfo[i].i2_Freq,
                              pScanResult->p_BssInfo[i].i2_Level,
                              pScanResult->p_BssInfo[i].i2_Noise,
                              pScanResult->p_BssInfo[i].i2_Quality,
                              pScanResult->p_BssInfo[i].i4_MaxRate,
                              pScanResult->p_BssInfo[i].u2_Caps,
                              pScanResult->p_BssInfo[i].u4_ieLength,
                              pScanResult->ui4_NumberOfItems));
    }
    return 0;

}

// use the scan_header to parse
int  tok_scan_bssid_convert(char *s_scan, NET_802_11_BSS_INFO_T *p_bss_info, char* s_scan_header)
{
    WIFI_LOG(WIFI_INFO, ("========= Enter tok_scan_bssid_convert() ==========\n"));

    int n_scan_info_num = 0;
    char *pos = NULL;
    char *end = NULL;
    int i4_Ret = 0;
    int i = 0;
    char * res[10] = {NULL};
    int len = 0;
    int len_left ;
    int count = 0;
    //unsigned char t_scan_results[6] = {0};
//	char * bssid_tmp = NULL;
    //memset(t_scan_results, 0, sizeof(unsigned char)*6);
    if ((s_scan == NULL) || (s_scan_header == NULL))
    {
        WIFI_LOG(WIFI_ERROR, ("This result is NULL\n"));
        return 0;
    }
    len_left = strlen (s_scan);
    n_scan_info_num = get_scan_info_num (s_scan_header);
    end = s_scan;
    pos = s_scan;

    WIFI_LOG(WIFI_INFO, ("AP info = %s\n", s_scan));

    while (end != NULL)
    {
        end = strstr(end, "\t"); //the first" "

        if (end == NULL)
        {
            res[count] = malloc(len_left + 1);
            if (res[count] == NULL)
            {
                WIFI_LOG(WIFI_ERROR, ("1 malloc res[%d] fail\n",count));
                return -1;
            }
            memset(res[count], 0, len_left + 1);
            strncpy(res[count], pos, len_left);
            //copy the free of memory
            break;
        }
        len = end - pos;
        res[count] = malloc(len + 1);
        if (res[count] == NULL)
        {
            WIFI_LOG(WIFI_ERROR, ("2 malloc res[%d] fail\n",count));
            return -1;
        }
        memset(res[count], 0, len + 1);
        strncpy(res[count], pos, len);
        len_left = len_left - len;
        if (end != NULL)
        {
            end++;

            pos = end;
            count++;
            len_left--;
            continue;
        }
    }

    end = s_scan_header;
    pos = s_scan_header;

    for (i = 0; i < n_scan_info_num ; i++)
    {
        char s_scan_info[40] = {0};
        if (end == NULL)
        {
            break;
        }
        end = strstr(end, "/");
        if (end == NULL)
        {
            strncpy (s_scan_info, pos,strlen(pos));   // copy the lest of
        }
        else
        {
            len = end -pos;
            strncpy(s_scan_info, pos, len);
            end ++;
            pos = end;
        }

        WIFI_LOG(WIFI_INFO, ("res[%d]=%s, scan info item=%s\n", i, res[i], s_scan_info));

        if (0 == strncmp(s_scan_info, "bssid", strlen("bssid")))
        {
            proc_scan_bssid (res[i], p_bss_info);
        }
        else if (0 == strncmp(s_scan_info, " frequency", strlen(" frequency")))
        {
            proc_scan_frequence (res[i], p_bss_info);
        }
        else if (0 == strncmp (s_scan_info, " signal level", strlen(" signal level")))
        {
            proc_scan_signal_level (res[i], p_bss_info);
        }
        else if (0 == strncmp (s_scan_info, " flags", strlen(" flags")))
        {
            if (proc_scan_flags (res[i], p_bss_info) != 0)
            {
                WIFI_LOG(WIFI_DEBUG, ("Discard SCAN AP in tok_scan_bssid_convet\n"));
                i4_Ret = -1;
                goto out;
            }
        }
        else if (0 == strncmp(s_scan_info, " ssid", strlen(" ssid")))
        {
            proc_scan_ssid (res[i], p_bss_info);
        }
        else if (0 == strncmp(s_scan_info, " rate", strlen(" rate")))
        {
            proc_scan_rate (res[i], p_bss_info);
        }
        else if (0 == strncmp(s_scan_info, " protocol caps", strlen(" protocol caps")))
        {
            proc_scan_protocol_caps (res[i], p_bss_info);
        }
    }

out:

    WIFI_LOG(WIFI_INFO, ("finish parsing one AP information\n"));

    for (i = 0; i <= count; i++)
    {
        if (res[i] != NULL)
        {
            free(res[i]);
            res[i] = NULL;
        }
    }

    return i4_Ret;
}



INT32 wpa_get_status_from_ret_buf(char * buf,NET_802_11_BSS_INFO_T *pCurrBss)
{
    WIFI_LOG(WIFI_DEBUG,  ("wpa_get_status_from_ret_buf\n"));
    INT32 i4_Ret = 0;
    int i = 0;
    char * res[30];
    int len = 0;
    int count = 0;

    char wpa_state[32] = {0};

    if (NULL == buf)
    {
        WIFI_LOG(WIFI_ERROR, ("Get curr bss failed!\n"));
        i4_Ret = -1;
        return i4_Ret;
    }

    char *pos = buf;
    char *end = buf;
    while (end != NULL)
    {
        if (count >= 30)
        {
            break;
        }
        end = strstr(end, "\n"); //the first" "
        if (end == NULL)
            break;

        len = end - pos;
        res[count] = malloc(len + 1);
        memset(res[count], 0, len + 1);
        strncpy(res[count], pos, len);
        if (end != NULL)
        {
            end++;
            pos = end;
            count++;
            continue;
        }
    }
    len = strlen(pos);
    WIFI_LOG(WIFI_DEBUG,("status last pos: %s %d\n",pos,len));
    if ((len > 0) && (count <= 29))
    {
        res[count] = malloc(len + 1);
        memset(res[count], 0, len + 1);
        strncpy(res[count], pos, len);
        count++;
    }

    for (i = 0;i < count;i++)
    {
        pos = strstr(res[i], "=");
        if (pos == NULL)
        {
            WIFI_LOG(WIFI_DEBUG, ("pos NULL res[%d] = %s\n",i,res[i]));
            continue;
        }
        len = pos - res[i];
        pos++;
        if (strncmp(res[i], "wpa_state", strlen ("wpa_state")) == 0)
        {
            strncpy(wpa_state, pos, strlen(pos));
            proc_wpa_state (&pCurrBss->u8_wpa_status, wpa_state);
        }
        if (strncmp(res[i], "ssid", strlen ("ssid")) == 0)
        {
            WIFI_LOG(WIFI_INFO, ("ssid = %s\n",pos));
            UINT8 ssidtmp[NET_802_11_MAX_LEN_SSID + 1] = {'\0'};
            printf_decode(ssidtmp, sizeof(ssidtmp), pos);
            pCurrBss->t_Ssid.ui4_SsidLen = strlen((char *)ssidtmp);
            strncpy(pCurrBss->t_Ssid.ui1_aSsid, (char *)ssidtmp, pCurrBss->t_Ssid.ui4_SsidLen);
        }

        if (pCurrBss->t_Bssid != NULL && strncmp(res[i], "bssid", strlen ("bssid")) == 0)
        {
            if (str_to_mac(pos, pCurrBss->t_Bssid) < 0)
            {
                WIFI_LOG(WIFI_DEBUG, ("[WIFI MW]Get Current Bssid failed!\n"));
                continue;
            }
        }

        if (0 == strncmp(res[i], "pairwise_cipher", strlen("pairwise_cipher")))
        {
            proc_pairwise_cipher_str(&pCurrBss->e_AuthCipher, pos);
        }

        if (0 == strncmp(res[i], "key_mgmt", strlen("key_mgmt")))
        {
            proc_key_mgmt_str(&pCurrBss->e_AuthMode, pos);
        }

        if ( IEEE_802_11_AUTH_MODE_NONE == pCurrBss->e_AuthMode
                && ( IEEE_802_11_AUTH_CIPHER_WEP40 == pCurrBss->e_AuthCipher
                     || IEEE_802_11_AUTH_CIPHER_WEP104 == pCurrBss->e_AuthCipher ) )
        {
            pCurrBss->e_AuthMode = IEEE_802_11_AUTH_MODE_WEPAUTO;
        }
        else if( (IEEE_802_11_AUTH_MODE_WPA2_PSK == pCurrBss->e_AuthMode) 
        	&& (IEEE_802_11_AUTH_CIPHER_TKIP == pCurrBss->e_AuthCipher))
        {
            pCurrBss->e_AuthMode = IEEE_802_11_AUTH_MODE_WPA2_PSK_TKIP;
            WIFI_LOG(WIFI_DEBUG, ("modify AuthMode [%d]", IEEE_802_11_AUTH_MODE_WPA2_PSK_TKIP));
        }

        if ((0 == strncmp(get_if_name(), "ath0", strlen ("ath0")))
#ifdef SUPPORT_2_DONGLES
                || (0 == strncmp(get_if_name(), "athmag0", strlen ("athmag0")))
#endif
           )
        {
            if (0 == strncmp (res[i], "signal_level", strlen ("signal_level")))
            {
                WIFI_LOG(WIFI_DEBUG, ("signal_level = %s\n",pos));
                proc_signal_level_ath0(&pCurrBss->i2_Level, pos);
            }

            if (0 == strncmp (res[i], "rate", strlen ("rate")))
            {
                proc_rate_ath0 (&pCurrBss->i4_MaxRate, pos);
            }
        }
        else if ((0 == strncmp(get_if_name(), "ra0", strlen ("ra0"))) || (0 == strncmp(get_if_name(), "wlan0", strlen ("wlan0"))))
        {
            if (0 == strncmp (res[i], "signal_level", strlen ("signal_level")))
            {
                WIFI_LOG(WIFI_DEBUG, ("signal_level = %s\n",pos));
                proc_signal_level_ra0 (&pCurrBss->i2_Level, pos);
            }

            if (0 == strncmp (res[i], "current_rate", strlen ("current_rate")))
            {
                proc_rate_ra0 (&pCurrBss->i4_MaxRate, pos);
            }
        }

        if (strncmp(res[i], "ap_channel", strlen ("ap_channel")) == 0)
        {
            WIFI_LOG(WIFI_INFO, ("ap_channel = %s\n",pos));
            pCurrBss->i2_Channel = atoi(pos);

            /* Frequency */
            if (0 == pCurrBss->i2_Channel)
            {
                pCurrBss->i2_Freq = 0;
            }
            else if (pCurrBss->i2_Channel >= 1 && pCurrBss->i2_Channel <= 13)
            {
                pCurrBss->i2_Freq = 2412 + (pCurrBss->i2_Channel - 1) * 5;
            }
            else if (14 == pCurrBss->i2_Channel)
            {
                pCurrBss->i2_Freq = 2484;
            }
            else if (pCurrBss->i2_Channel >= 15)
            {
                pCurrBss->i2_Freq = 5000 + pCurrBss->i2_Channel * 5;
            }
        }
        else if ( strncmp(res[i], "freq", strlen ("freq")) == 0 )
        {
            WIFI_LOG(WIFI_INFO, ("ap freq = %sMHz\n", pos));
            pCurrBss->i2_Freq = atoi(pos);
            pCurrBss->i2_Channel = frequency_to_channel(pCurrBss->i2_Freq);
        }
    }
    i4_Ret = 0;
    for (i = 0;i < count;i++)
    {
        if (res[i] != NULL)
        {
            free(res[i]);
            res[i] = NULL;
        }
    }
    return i4_Ret;
}
int proc_signal_level_ra0 (INT16 *i2_level, char* str)
{
    *i2_level = atoi(str);
    *i2_level = u1WlanDrv_NormalizeSignal(*i2_level);
    return 0;
}

int proc_rate_ra0 (INT32 *i4_max_rate, char* str)
{
    char *pos = str;
    char *end = str;
    end = strstr (end, " ");
    if (end == NULL)
    {
        WIFI_LOG(WIFI_ERROR, ("parse rate ra0 error!"));
        return -1;
    }
    int len = 0;
    len = end - pos;
    char s_value[12] = {0};
    strncpy (s_value, pos, len);
    WIFI_LOG(WIFI_DEBUG, ("str = [%s] s_value [%s]", str, s_value));

    *i4_max_rate  = atoi (s_value) * 1024;
    return 0;
}

int proc_rate_ath0 (INT32 *i4_max_rate, char* str)
{
    char* pos = str;
    char* end = str;

    char s_value[12] = {0};
    int i_unit = 0x01;
    if (strstr (end, "M") != NULL)
    {
        end = strstr (end, "M");
        i_unit = i_unit<<10;
    }
    else if (strstr (end, "k") != NULL)
    {
        end = strstr (end, "k") ;
    }
    else if  (strstr (end, "G") != NULL)
    {
        end  = strstr (end, "G");
        i_unit = i_unit<<10;
    }
    else
    {
        WIFI_LOG (WIFI_ERROR,("can not find the uint"));
        return -1;
    }

    int len = 0;
    len = end -pos ;

    strncpy (s_value, pos, len); // the last unit should not be copy

    WIFI_LOG(WIFI_DEBUG, ("pos = [%s] s_value [%s]", pos, s_value));
    *i4_max_rate = atoi (s_value)*i_unit;

    return 0;
}

int proc_signal_level_ath0(INT16 *i2_level, char* str)
{
    *i2_level = atoi(str);
    *i2_level = u1WlanK2Drv_NormalizeSignal(*i2_level);
    return 0;
}


int proc_wpa_state(UINT8 *p_wpa_state, char *str)
{
    if (p_wpa_state == NULL || str == NULL)
    {
        return -1;
    }


    STR_KEY_MAP stru_wpa_state_map[] =
    {
        {"DISCONNECTED", WLAN_WPA_DISCONNECTED},
        {"INACTIVE", WLAN_WPA_INACTIVE},
        {"SCANNING", WLAN_WPA_SCANNING},
        {"ASSOCIATING", WLAN_WPA_ASSOCIATING},
        {"ASSOCIATED", WLAN_WPA_ASSOCIATED},
        {"4WAY_HANDSHAKE", WLAN_WPA_4WAY_HANDSHAKE},
        {"GROUP_HANDSHAKE", WLAN_WPA_GROUP_HANDSHAKE},
        {"COMPLETED", WLAN_WPA_COMPLETED}
    };
    int wpa_state = 0;
    if (0 == str_key_map(stru_wpa_state_map,sizeof (stru_wpa_state_map)/sizeof (stru_wpa_state_map[0]), str, &wpa_state))
    {
        *p_wpa_state = (UINT8)wpa_state;
    }
    return 0;

}

int  proc_pairwise_cipher_str(NET_802_11_AUTH_CIPHER_T  *e_auth_ciper, char * str)
{
    if (NULL == e_auth_ciper || NULL == str )
    {
        WIFI_LOG(WIFI_ERROR,  ("invalid proc_pairwise_cipher_str param \n"));
        return -1;
    }


    STR_KEY_MAP stru_auth_ciper[] =
    {
        {"CCMP", IEEE_802_11_AUTH_CIPHER_AES},
        {"TKIP",IEEE_802_11_AUTH_CIPHER_TKIP},
        {"CCMP TKIP", IEEE_802_11_AUTH_CIPHER_AES_OR_TKIP},
        {"NONE", IEEE_802_11_AUTH_CIPHER_NONE},
        {"WEP-40", IEEE_802_11_AUTH_CIPHER_WEP40},
        {"WEP-104", IEEE_802_11_AUTH_CIPHER_WEP104}
    };

    int auth_ciper = 0;
    if (0 == str_key_map(stru_auth_ciper,sizeof (stru_auth_ciper)/sizeof (stru_auth_ciper[0]), str, &auth_ciper))
    {
        *e_auth_ciper = auth_ciper;
    }

    return 0;
}

int proc_key_mgmt_str(NET_802_11_AUTH_MODE_T *e_auth_mode, char *str)
{
    if (NULL == e_auth_mode || NULL == str)
    {
        WIFI_LOG(WIFI_ERROR,  ("invalid proc_key_mgmt_str param \n"));
        return -1;
    }

    STR_KEY_MAP stru_auth_mode[] =
    {
        {"WPA2-PSK", IEEE_802_11_AUTH_MODE_WPA2_PSK},
        {"WPA-PSK",IEEE_802_11_AUTH_MODE_WPA_PSK},
        {"WPA2-EAP", IEEE_802_11_AUTH_MODE_WPA2},
        {"WPA-EAP", IEEE_802_11_AUTH_MODE_WPA},
        {"NONE", IEEE_802_11_AUTH_MODE_NONE}
    };

    int auth_mode = 0;
    if (0 == str_key_map(stru_auth_mode,sizeof (stru_auth_mode)/sizeof (stru_auth_mode[0]), str, &auth_mode))
    {
        *e_auth_mode = auth_mode;
    }
    return 0;

}



INT32 wps_proc_cred(char * cred, NET_802_11_ASSOCIATE_T *assoc)
{
    NET_802_11_ASSOCIATE_T req;
    memset (&req, 0, sizeof (req));
    INT32 cred_len = 0, network_id = 0, network_id_len = 0;
    UINT16 BitMask = 0;
    char * pStartAdd = cred;

    char ssid[NET_802_11_MAX_LEN_SSID+1] = {0};
    INT32 ssid_len = 0;
    INT32 auth_len = 0;
    INT32 auth_type = 0;

    INT32 encypt_type = 0;
    INT32 encypt_len = 0;

    INT32 network_key_idx = 0;
    INT32 network_key_idx_len = 0;

    INT32 key_len = 0;
    char key_body[64+1] = {0};

    INT32 mac_len;
    unsigned char mac_addr[6]={0};

    int ret = -1;
    unsigned char rf_band = 0;

    cred_len = wps_get_cred_context(cred);
    if (cred_len < 0)
    {
        WIFI_LOG(WIFI_ERROR, ("Credentials is NULL\n"));
//        return -1;
    }
    else
    {
        WIFI_LOG(WIFI_DEBUG, ("cred_len = %d\n", cred_len));
        cred += 8;
        WIFI_LOG(WIFI_DEBUG, ("%s\n", cred));
    }

    network_id_len = wps_get_cred_network_id(cred, &network_id);

    if (network_id_len < 0)
    {
        WIFI_LOG(WIFI_ERROR, ("No network id\n"));
//        return -1;
    }
    else
    {
        WIFI_LOG(WIFI_DEBUG, ("network_id = %02X\n", network_id));
        cred += 8 + network_id_len * 2;
        WIFI_LOG(WIFI_DEBUG, ("%s\n", cred));
    }

    ssid_len = wps_get_cred_ssid(cred, ssid);
    if (ssid_len < 0)
    {
        WIFI_LOG(WIFI_ERROR, ("No ssid\n"));
        // return -1;
    }
    else
    {
        req.t_Ssid.ui4_SsidLen = ssid_len;
        if (req.t_Ssid.ui4_SsidLen > NET_802_11_MAX_LEN_SSID)
        {
            req.t_Ssid.ui4_SsidLen = NET_802_11_MAX_LEN_SSID;
        }
        strncpy(req.t_Ssid.ui1_aSsid, (CHAR *)ssid, NET_802_11_MAX_LEN_SSID);
        cred = cred + (8 + ssid_len * 2);
        WIFI_LOG(WIFI_DEBUG, ("%s\n", cred));
    }

    auth_len = wps_get_cred_auth_type(cred, &auth_type);
    if (auth_len < 0)
    {
        WIFI_LOG(WIFI_ERROR, ("No authentication type!\n"));
//        return -1;
    }
    else
    {
        cred = cred + 8 + auth_len * 2;
        WIFI_LOG(WIFI_DEBUG, ("auth_type = %02X\n", auth_type));
        WIFI_LOG(WIFI_DEBUG, ("%s\n", cred));

        if (auth_type & WPS_AUTH_TYPES)
        {
            WIFI_LOG(WIFI_DEBUG, ("Support multi auth type, choose higher one:%x\n", auth_type));
            for (BitMask = WPS_AUTH_WPA2PSK; BitMask != 0; BitMask = BitMask >> 1)
            {
                if (auth_type & BitMask)
                {
                    auth_type = auth_type & BitMask;
                    break;
                }
            }
            //fix customer's request to report WPS_AUTH_WPAPSK_OR_WPA2PSK
            if (auth_type == WPS_AUTH_WPAPSK_OR_WPA2PSK)
            {
                auth_type = WPS_AUTH_WPAPSK_OR_WPA2PSK;
            }
        }
        else
        {
            WIFI_LOG(WIFI_ERROR,("Unknow auth type:%x\n", (unsigned int)auth_type));
//			return -1;
        }
    }

    encypt_len = wps_get_cred_encypt_type(cred, &encypt_type);
    if (encypt_len < 0)
    {
        WIFI_LOG(WIFI_ERROR, ("No encyption type!\n"));
//        return -1;
    }
    else
    {
        cred = cred + 8 + encypt_len * 2;

        WIFI_LOG(WIFI_DEBUG, ("encypt_type = %02X\n", encypt_type));
        WIFI_LOG(WIFI_DEBUG, ("%s\n", cred));
    }

    network_key_idx_len = wps_get_cred_netkeyidx(cred, &network_key_idx);
    if (network_key_idx_len < 0)
    {
        WIFI_LOG(WIFI_ERROR, ("No network key idx!\n"));
//        return -1;
    }
    else
    {
        if (network_key_idx_len > 0)
            cred = cred + 8 + network_key_idx_len * 2;
        WIFI_LOG(WIFI_DEBUG, ("network_key_idx = %02X\n", network_key_idx));
        WIFI_LOG(WIFI_DEBUG, ("%s\n", cred));
    }

    key_len = wps_get_cred_network_key(cred, key_body);
    if (key_len < 0)
    {
        WIFI_LOG(WIFI_ERROR, ("No keys!\n"));
//        return -1;
    }
    else
    {
        cred = cred + 8 + key_len * 2;
        WIFI_LOG(WIFI_DEBUG, ("key_body = %s\n", key_body));
        WIFI_LOG(WIFI_DEBUG, ("%s\n", cred));
    }

    if (encypt_type & WPS_ENCR_TYPES)
    {
        WIFI_LOG(WIFI_DEBUG, ("Support multi encr type, choose higher one:%x\n", encypt_type));
        for (BitMask = WPS_ENCR_AES; BitMask != 0; BitMask = BitMask >> 1)
        {
            if (encypt_type & BitMask)
            {
                encypt_type = encypt_type & BitMask;
                break;
            }
        }
        //fix customer's request to report WPS_ENCR_AES_OR_TKIP
        if (encypt_type == WPS_ENCR_AES_OR_TKIP)
        {
            encypt_type = WPS_ENCR_AES_OR_TKIP;
        }
    }
    else
    {
        WIFI_LOG(WIFI_DEBUG, ("Unknow encry type:%x\n", encypt_type));
//if Auth type correct but encr type correct, work around it
        switch (auth_type)
        {
        case WPS_AUTH_OPEN:
            encypt_type = WPS_ENCR_NONE;
            break;
        case WPS_AUTH_SHARED:
            encypt_type = WPS_ENCR_WEP;
            break;
        case WPS_AUTH_WPAPSK:
            encypt_type = WPS_ENCR_TKIP;
            break;
        case WPS_AUTH_WPA2PSK:
            encypt_type = WPS_ENCR_AES;
            break;
        default:
            WIFI_LOG(WIFI_ERROR, ("Unknow auth type:%x\n", auth_type));
//                return -1;
        }


    }
    ;
    switch (auth_type)
    {
    case WPS_AUTH_OPEN:
        if (encypt_type == WPS_ENCR_WEP)
            req.e_AuthMode = IEEE_802_11_AUTH_MODE_WEPAUTO;
        else
            req.e_AuthMode = IEEE_802_11_AUTH_MODE_OPEN;
        break;

    case WPS_AUTH_SHARED:
        req.e_AuthMode = IEEE_802_11_AUTH_MODE_WEPAUTO;
        break;
    case WPS_AUTH_WPA:
        req.e_AuthMode = IEEE_802_11_AUTH_MODE_WPA;
        break;

    case WPS_AUTH_WPA2:
        req.e_AuthMode = IEEE_802_11_AUTH_MODE_WPA2;
        break;

    case WPS_AUTH_WPAPSK:
    case WPS_AUTH_WPA2PSK:
    case WPS_AUTH_WPAPSK_OR_WPA2PSK:
        req.e_AuthMode = IEEE_802_11_AUTH_MODE_WPAPSK_OR_WPA2PSK;
        if (key_len == WPAPSK_HEX_KEY_SIZE)
            req.t_Key.b_IsAscii = 0;
        else
            req.t_Key.b_IsAscii = 1; /* 8~63 ascii char */
        break;

    default:
        WIFI_LOG(WIFI_ERROR, ("Unknow auth type:%x\n", auth_type));
        //          return -1;
    }

    switch (encypt_type)
    {
    case WPS_ENCR_NONE:
        req.e_AuthCipher = IEEE_802_11_AUTH_CIPHER_NONE;
        //if(auth_type != WPS_AUTH_OPEN)
        {
            req.t_Key.pui1_PassPhrase = (CHAR *)key_body;
            key_body[0] = '\0';
        }
        /*CR BDP00031735,If Encryption Type in credential is None(0x0001), please treat it
            as "No Security" regardless of Authentication Type in the credential
        */
        req.e_AuthMode = IEEE_802_11_AUTH_MODE_NONE;
        break;

    case WPS_ENCR_WEP:
    {
        if (key_len == WEP_KEY_SIZE_HEX_40)
        {
            req.e_AuthCipher = IEEE_802_11_AUTH_CIPHER_WEP40;
            req.t_Key.b_IsAscii = 0;
        }
        else if (key_len == WEP_KEY_SIZE_HEX_104)
        {
            req.e_AuthCipher = IEEE_802_11_AUTH_CIPHER_WEP104;
            req.t_Key.b_IsAscii = 0;
        }
        else if (key_len == WEP_KEY_SIZE_ASCII_40)
        {
            req.e_AuthCipher = IEEE_802_11_AUTH_CIPHER_WEP40;
            req.t_Key.b_IsAscii = 1;
        }
        else if (key_len == WEP_KEY_SIZE_ASCII_104)
        {
            req.e_AuthCipher = IEEE_802_11_AUTH_CIPHER_WEP104;
            req.t_Key.b_IsAscii = 1;
        }
        else
        {
            WIFI_LOG(WIFI_ERROR, ("Invalid key size for wep: %d\n", (int)key_len));
            //            return -1;
        }

        req.t_Key.pui1_PassPhrase = (CHAR *)key_body;
        break;
    } /* WPS_ENCR_WEP */
    case WPS_ENCR_TKIP:
    case WPS_ENCR_AES:
    case WPS_ENCR_AES_OR_TKIP:
        req.e_AuthCipher = IEEE_802_11_AUTH_CIPHER_AES_OR_TKIP;
        req.t_Key.pui1_PassPhrase = (CHAR *)key_body;
        break;

    default:
        WIFI_LOG(WIFI_ERROR, ("Unknow encry type:%x\n", encypt_type));
#if 1
        req.e_AuthCipher = IEEE_802_11_AUTH_CIPHER_NONE;
        //if(auth_type != WPS_AUTH_OPEN)
        {
            req.t_Key.pui1_PassPhrase = (CHAR *)key_body;
            key_body[0] = '\0';
        }
        break;
#else
        return -1;
#endif
    }

    if (req.t_Key.pui1_PassPhrase)
    {
        memcpy(req.t_Key.key_body,req.t_Key.pui1_PassPhrase,sizeof(req.t_Key.key_body)-1);
    }
    WIFI_LOG(WIFI_DEBUG, ("Notifying SUPC for re-association\n"));
    mac_len = wps_get_cred_mac(cred, mac_addr);
    if (mac_len < 0)
    {
        WIFI_LOG(WIFI_ERROR, ("No mac addr\n"));
        //  return -1;
    }
    else
    {
        memcpy(req.t_Bssid, mac_addr, 6);
        cred = cred + 8 + mac_len * 2;
        WIFI_LOG(WIFI_DEBUG, ("mac_len = %d,mac_addr = %02X:%02X:%02X:%02X:%02X:%02X\n", mac_len, mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]));
        WIFI_LOG(WIFI_DEBUG, ("%s\n", cred));
    }

    memcpy(assoc, &req, sizeof(NET_802_11_ASSOCIATE_T));
    assoc->t_Key.pui1_PassPhrase = (CHAR * )assoc->t_Key.key_body;

    if (cred >= (pStartAdd + cred_len * 2 + 8)) /*100e008cxxxxxxxx......*/
    {
        WIFI_LOG(WIFI_DEBUG, ("No valid credenatial left, cred=%d, end=%d", cred, (pStartAdd + cred_len * 2 + 8)));
        return 0;
    }   /* RF band information is not necessary */

    WIFI_LOG(WIFI_DEBUG,("[Parse RF band]: crdential=%s\n",cred));
    ret = wps_get_cred_RF_band(cred, &rf_band);
    if (ret != -1)
    {
        assoc->ui1_Priority = rf_band;
    }
    else
    {
        WIFI_LOG(WIFI_DEBUG, ("get RF band failed!"));
    }
    return 0;
}

INT32 wps_get_cred_mac(char * cred, unsigned char *mac_addr)
{
    UINT32 mac_len, tmp1, tmp2;
    char * mac_value;
    INT32 i = 0;
    if (cred == NULL || strlen(cred) <= 8)
    {
        WIFI_LOG(WIFI_ERROR, ("Mac is NULL!\n"));
        return -1;
    }
    if (strncmp(cred, "1020", 4) == 0)
    {
        cred += 4;
        mac_value = malloc(5);
        if (mac_value == NULL)
            return -1;
        memset(mac_value, 0, 5);
        strncpy(mac_value, cred, 4);
        mac_len = strtoul(mac_value, NULL, 16);
        cred += 4;
        free(mac_value);
        WIFI_LOG(WIFI_DEBUG, ("cred = %s\n", cred));
        mac_value = malloc(2);
        if (mac_value == NULL)
            return -1;
        for (i = 0;i < mac_len;i++)
        {

            memset(mac_value, 0, 2);
            strncpy(mac_value, cred, 1);
            tmp1 = strtoul(mac_value, NULL, 16);
            cred++;

            memset(mac_value, 0, 2);
            strncpy(mac_value, cred, 1);
            tmp2 = strtoul(mac_value, NULL, 16);
            cred++;
            tmp1 = tmp1 * 16 + tmp2;
            mac_addr[i] = tmp1;
            WIFI_LOG(WIFI_DEBUG, ("tmp1 = %d\n", (int)tmp1));
        }

        free(mac_value);
        return mac_len;
    }
    return -1;
}

INT32 wps_get_cred_network_key(char * cred, char * key_body)
{
    UINT32 key_len, tmp;
    char * key_value;
    char c;
    INT32 i = 0;
    if (cred == NULL || strlen(cred) <= 8)
    {
        WIFI_LOG(WIFI_ERROR,("key idex is NULL!\n"));
        return -1;
    }
    if (strncmp(cred, "1027", 4) == 0)
    {
        cred += 4;
        key_value = malloc(5);
        if (key_value == NULL)
            return -1;
        memset(key_value, 0, 5);
        strncpy(key_value, cred, 4);
        key_len = strtoul(key_value, NULL, 16);
        cred += 4;
        free(key_value);

        key_value = malloc(3);
        if (key_value == NULL)
            return -1;
        for (i = 0;i < key_len;i++)
        {

            memset(key_value, 0, 3);
            strncpy(key_value, cred, 2);
            tmp = strtoul(key_value, NULL, 16);
            c = (char)(tmp);
            strncpy(key_body + i, &c, 1);
            cred += 2;
        }
        free(key_value);
        return key_len;
    }
    return -1;
}

INT32 wps_get_cred_netkeyidx(char * cred, INT32 * key_idx)
{
    UINT32 idx_len;
    char * idx_value;
    if (cred == NULL || strlen(cred) <= 8)
    {
        WIFI_LOG(WIFI_ERROR, ("key idex is NULL!\n"));
        return -1;
    }
    if (strncmp(cred, "1027", 4) == 0)
    {
        return 0;
    }
    if (strncmp(cred, "1028", 4) == 0)
    {
        cred += 4;
        idx_value = malloc(5);
        if (idx_value == NULL)
            return -1;
        memset(idx_value, 0, 5);
        strncpy(idx_value, cred, 4);
        idx_len = strtoul(idx_value, NULL, 16);
        cred += 4;
        free(idx_value);
        WIFI_LOG(WIFI_DEBUG,("idx_len =%d\n", (int)idx_len));
        idx_value = malloc(idx_len * 2 + 1);
        if (idx_value == NULL)
            return -1;
        if (NULL == idx_value)
        {
            WIFI_LOG(WIFI_ERROR, ("key idex malloc fail!\n"));
        }
        memset(idx_value, 0, idx_len*2 + 1);
        strncpy(idx_value, cred, idx_len*2);
        *key_idx = strtoul(idx_value, NULL, 16);
        free(idx_value);
        return idx_len;
    }

    return -1;
}

INT32 wps_get_cred_encypt_type(char * cred, INT32 * encypt_type)
{
    UINT32 encrypt_len;
    char * encryp_value;
    if (cred == NULL || strlen(cred) <= 8)
    {
        WIFI_LOG(WIFI_ERROR, ("encryption type is NULL!\n"));
        return -1;
    }
    if (strncmp(cred, "100f", 4) == 0)
    {
        cred += 4;
        encryp_value = malloc(5);
        if (encryp_value == NULL)
            return -1;
        memset(encryp_value, 0, 5);
        strncpy(encryp_value, cred, 4);
        encrypt_len = strtoul(encryp_value, NULL, 16);
        cred += 4;
        free(encryp_value);

        encryp_value = malloc(encrypt_len * 2 + 1);
        if (encryp_value == NULL)
            return -1;
        if (NULL == encryp_value)
        {
            WIFI_LOG(WIFI_ERROR, ("encryption type malloc fail!\n"));
        }
        memset(encryp_value, 0, encrypt_len*2 + 1);
        strncpy(encryp_value, cred, encrypt_len*2);
        *encypt_type = strtoul(encryp_value, NULL, 16);
        free(encryp_value);
        return encrypt_len;
    }
    return -1;
}


INT32 wps_get_cred_auth_type(char * cred, INT32 * auth_type)
{
    UINT32 auth_len;
    char * auth_value;
    if (cred == NULL || strlen(cred) <= 8)
    {
        WIFI_LOG(WIFI_ERROR, ("auth type is NULL!\n"));
        return -1;
    }
    if (strncmp(cred, "1003", 4) == 0)
    {
        cred += 4;
        auth_value = malloc(5);
        if (auth_value == NULL)
            return -1;
        memset(auth_value, 0, 5);
        strncpy(auth_value, cred, 4);
        auth_len = strtoul(auth_value, NULL, 16);
        cred += 4;
        WIFI_LOG(WIFI_DEBUG, ("auth_len = %d\n", (int)auth_len));
        free(auth_value);

        auth_value = malloc(auth_len * 2 + 1);
        if (auth_value == NULL)
            return -1;
        if (NULL == auth_value)
        {
            WIFI_LOG(WIFI_ERROR, ("auth type malloc fail!\n"));
        }
        memset(auth_value, 0, auth_len*2 + 1);
        strncpy(auth_value, cred, auth_len*2);
        *auth_type = strtoul(auth_value, NULL, 16);

        free(auth_value);
        return auth_len;
    }
    return -1;
}


INT32 wps_get_cred_ssid(char * cred, char * ssid)
{
    UINT32 ssid_len, tmp;
    char * cred_value;
    char * ssid_raw = NULL;
    INT32 i;
    char c;
    if (cred == NULL || strlen(cred) <= 8)
    {
        WIFI_LOG(WIFI_ERROR, ("SSID is NULL\n"));
        return -1;
    }
    if (strncmp(cred, "1045", 4) == 0)
    {
        cred += 4;
        cred_value = malloc(5);
        if (cred_value == NULL)
        {
            return -1;
        }
        memset(cred_value, 0, 5);
        strncpy(cred_value, cred, 4);
        WIFI_LOG(WIFI_DEBUG, ("cred_value = %s\n", cred_value));
        ssid_len = strtoul(cred_value, NULL, 16);
        WIFI_LOG(WIFI_DEBUG, ("ssid_len = %d\n", (int)ssid_len));
        if (cred_value != NULL)
            free(cred_value);
        cred += 4;
        WIFI_LOG(WIFI_DEBUG, ("ssid=\n"));
        for (i = 0;i < ssid_len;i++)
        {
            ssid_raw = malloc(3);
            if (ssid_raw == NULL)
            {
                return -1;
            }
            memset(ssid_raw, 0, 3);
            strncpy(ssid_raw, cred, 2);
            tmp = strtoul(ssid_raw, NULL, 16);
            if ( ssid_raw )
                free( ssid_raw );
            c = (char)(tmp);
            strncpy(ssid + i, &c, 1);
            cred += 2;
        }
        WIFI_LOG(WIFI_DEBUG, ("%s\n", ssid));
        return ssid_len;
    }
    return -1;
}


INT32 wps_get_cred_network_id(char * cred, INT32 * network_id)
{
    UINT32 networkid_len;
    char * networkid;
    char * attr_value;
    if (cred == NULL || strlen(cred) <= 8)
    {
        WIFI_LOG(WIFI_ERROR, ("Network id is NULL\n"));
        return -1;
    }
    WIFI_LOG(WIFI_DEBUG, ("cred = %s\n", cred));
    if (strncmp(cred, "1026", 4) == 0)
    {
        cred = cred + 4;
        attr_value = malloc(5);
        if (attr_value == NULL)
        {
            return -1;
        }
        memset(attr_value, 0, 5);
        strncpy(attr_value, cred, 4);

        networkid_len = strtoul(attr_value, NULL, 16);
        if (attr_value != NULL)
            free(attr_value);
        cred = cred + 4;

        networkid = malloc(networkid_len * 2 + 1);
        if (networkid == NULL)
        {
            return -1;
        }
        memset(networkid, 0, networkid_len*2 + 1);
        strncpy(networkid, cred, networkid_len*2);
        *network_id = strtoul(networkid, NULL, 16);
        if (networkid != NULL)
            free(networkid);
        return networkid_len;
    }
    return -1;
}


INT32 wps_get_cred_context(char * cred)
{
    UINT32 cred_len;
    char cred_value[5] = {0};
    if (cred == NULL || strlen(cred) <= 4)
    {
        WIFI_LOG(WIFI_ERROR, ("credential is NULL\n"));
        return -1;
    }
    if (strncmp(cred, "100e", 4) == 0)
    {
        cred = cred + 4;
        //cred_value = malloc(5);
        //memset(cred_value, 0, 5);
        strncpy(cred_value, cred, 4);
        cred_len = strtoul(cred_value, NULL, 16);
        cred = cred + 4;
        //free(cred_value);
        return cred_len;
    }
    return -1;
}

INT32 wps_get_cred_RF_band(char * cred, unsigned char *prf_band)
{
    UINT32 rf_band_len = -1;

    if (cred == NULL || strlen(cred) <= 8)
    {
        WIFI_LOG(WIFI_WARNING, ("No RF band information in credential!"));
        return -1;
    }

    /* RF band:
    * 103c 0001 01: 2.4GHz only
    * 103c 0001 02: 5GHz only
    * 103c 0001 03: 2.4GHz/5GHz */
    if (strncmp(cred, "103c", 4) == 0)
    {
        /* get RF band length */
        char str_rfband_len[5] = {'\0'};
        cred += 4;
        strncpy(str_rfband_len, cred, 4);
        rf_band_len = strtoul(str_rfband_len,NULL,16);
        if (/*rf_band_len == ULONG_MAX || */rf_band_len != 1) //need to sync?
        {
            WIFI_LOG(WIFI_WARNING, ("RF band length in credential is invalid, RF band length = %d", (int)rf_band_len));
            return -1;
        }

        /*get RF band value */
        cred += 4;
        char tmp_hex[16] = {'\0'};
        char str_rf_band[16] = {'\0'};
        strncpy(tmp_hex, cred, 2);
        *prf_band = strtoul(tmp_hex, NULL, 16);
        switch (*prf_band)
        {
        case WPS_RF_24GHZ:
            strncpy(str_rf_band, "2.4GHz only", sizeof(str_rf_band));
            break;
        case WPS_RF_50GHZ:
            strncpy(str_rf_band, "5GHz only", sizeof(str_rf_band));
            break;
        case (WPS_RF_24GHZ | WPS_RF_50GHZ):
            strncpy(str_rf_band, "2.4GHz/5GHz", sizeof(str_rf_band));
            break;
        default:
            strncpy(str_rf_band, "Invalid RF band", sizeof(str_rf_band));
            break;
        }
        WIFI_LOG(WIFI_DEBUG, ("credential RF band = 0x%04X(%s)", *prf_band, str_rf_band));
    }
    else
    {
        WIFI_LOG(WIFI_DEBUG, ("No RF band information in credential!"));
        return -1;
    }

    return 1;
}

INT32 str_to_mac(char * str, UINT8 * bssid)
{
    INT32 i = 0;
    char * pos, *str_tmp;

    UINT8 mac[ETH_ALEN];
    if (strlen(str) < (ETH_ALEN*2 + ETH_ALEN - 1))
    {
        WIFI_LOG(WIFI_ERROR, ("Mac Address not Complete!\n"));
        return -1;
    }
    pos = str;
    pos = strtok(pos, ":");
    str_tmp = pos;
    mac[0] = str2mac(str_tmp);

    for (i = 1 ; i < (ETH_ALEN - 1); i++)
    {
        pos = strtok(NULL, ":");
        str_tmp = pos;
        mac[i] = str2mac(str_tmp);
    }

    pos = strtok(NULL, " ");
    str_tmp = pos;
    mac[ETH_ALEN-1] = str2mac(str_tmp);

    for (i = 0;i < ETH_ALEN;i++)
    {
        bssid[i] = mac[i];
    }
    return 0;
}


unsigned char str2mac(char*s)
{
    unsigned char u_8_mac = '\0';
    UINT8 uReturnValue = 0;
    char *pszStr = s;
    UINT32 u4Idx;
    if ((pszStr == NULL) )
    {
        return u_8_mac;
    }
    for (u4Idx = 0; u4Idx < 2; u4Idx++)
    {
        if ((pszStr[u4Idx] >= '0') && (pszStr[u4Idx] <= '9'))
        {
            uReturnValue = uReturnValue << 4;
            uReturnValue += (UINT8)(pszStr[u4Idx] - '0');
        }
        else if ((pszStr[u4Idx] >= 'A') && (pszStr[u4Idx] <= 'F'))
        {
            uReturnValue = uReturnValue << 4;
            uReturnValue += (UINT8)(pszStr[u4Idx] - 'A' ) + 10;
        }
        else if ((pszStr[u4Idx] >= 'a') && (pszStr[u4Idx] <= 'f'))
        {
            uReturnValue = uReturnValue << 4;
            uReturnValue += (UINT8)(pszStr[u4Idx] - 'a') + 10;
        }
        else
        {
            return u_8_mac;
        }
    }
    u_8_mac = (unsigned char)uReturnValue;
    return u_8_mac;
}


