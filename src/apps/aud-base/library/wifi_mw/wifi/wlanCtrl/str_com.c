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
 * $RCSfile: str_com.c,v $
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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//#include "_net_wndrv_if.h"
//#include "u_net_wlan.h"
#include <errno.h>
#include <pthread.h>
#include <time.h>


#include <stdarg.h>
#include <fcntl.h>
#include <termios.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include "inc/_timer.h"
#include "u_common.h"

#define c_to_digit_a(x)  (((UINT8)(x) % 10) + (UINT8)'0')
#define WPS_PIN_LEN_8 8
#define WPS_PIN_LEN_4 4

#define DEV_BSP "/dev/bsp"
#define IOCTL_BSP_HAL_GET_TIME       1
#define IOCTL_BSP_HAL_GET_DELTA_TIME 2

//extern char wps_pin_code;
int hex2num(char c);
int hex2byte(const char *hex);
size_t printf_decode(UINT8 *buf, size_t maxlen, const char *str);
static UINT32 StrToHex(const CHAR* pszStr, UINT32 u4Len);

static UINT32 StrToDec(const CHAR* pszStr, UINT32 u4Len);
static UINT32 StrToInt(const CHAR* pszStr);
static INT32 wps_computePinCheckSum(unsigned long pin);
static UINT32 StrToInt(const CHAR* pszStr);
static void HAL_GetTime(HAL_TIME_T* pTime);
static int os_get_random(char *buf, size_t len);
static INT32 i4Wps_generatePin(void *ctx, char *out, UINT32 _8_digit);
extern INT32 x_net_wlan_get_mac_addr(UINT8 *pMacAddr);
static void HAL_GetTime(HAL_TIME_T* pTime)
{
    int ret, fd_bsp;

    fd_bsp = open(DEV_BSP, O_RDWR);
//	ASSERT(fd_bsp != -1);
	
	ret = ioctl(fd_bsp, IOCTL_BSP_HAL_GET_TIME, pTime);
//    ASSERT(ret == 0);

	ret = close(fd_bsp);
//    ASSERT(ret == 0);
}
#define NORMALIZED_SIGNAL_MIN     5
#define NORMALIZED_SIGNAL_MAX   100
#if CONFIG_WLAN_SW_FOR_CUSTOM_JSN || CONFIG_WLAN_SW_FOR_CUSTOM_CBBG
#define WEAK_SIGNAL_STRENGTH     -80  
#define STRONG_SIGNAL_STRENGTH   -45
#else
#define WEAK_SIGNAL_STRENGTH     -88  
#define STRONG_SIGNAL_STRENGTH   -65
#endif
int u1WlanDrv_NormalizeSignal(int signal)
{

    if(signal < WEAK_SIGNAL_STRENGTH)   signal = WEAK_SIGNAL_STRENGTH;
    if(signal > STRONG_SIGNAL_STRENGTH) signal = STRONG_SIGNAL_STRENGTH;

    return  NORMALIZED_SIGNAL_MIN +
            (NORMALIZED_SIGNAL_MAX - NORMALIZED_SIGNAL_MIN) * (signal - WEAK_SIGNAL_STRENGTH) 
             / (STRONG_SIGNAL_STRENGTH - WEAK_SIGNAL_STRENGTH);
}


int hex2num(char c)
{
	if (c >= '0' && c <= '9')
		return c - '0';
	if (c >= 'a' && c <= 'f')
		return c - 'a' + 10;
	if (c >= 'A' && c <= 'F')
		return c - 'A' + 10;
	return -1;
}


int hex2byte(const char *hex)
{
	int a, b;
	a = hex2num(*hex++);
	if (a < 0)
		return -1;
	b = hex2num(*hex++);
	if (b < 0)
		return -1;
	return (a << 4) | b;
}

static UINT32 StrToHex(const CHAR* pszStr, UINT32 u4Len)
{
	UINT32 u4Idx;
	UINT32 u4ReturnValue = 0;

	if ((pszStr == NULL) || (u4Len == 0))
	{
		return 0;
	}

	u4Len = (u4Len > 8) ? 8 : u4Len;

	for (u4Idx = 0;
		u4Idx < u4Len;
		u4Idx++)
	{
		if ((pszStr[u4Idx] >= '0') && (pszStr[u4Idx] <= '9'))
		{
			u4ReturnValue = u4ReturnValue << 4;
			u4ReturnValue += (UINT32)(UINT8)(pszStr[u4Idx] - '0');
		}
		else
		if ((pszStr[u4Idx] >= 'A') && (pszStr[u4Idx] <= 'F'))
		{
			u4ReturnValue = u4ReturnValue << 4;
			u4ReturnValue += (UINT32)(UINT8)(pszStr[u4Idx] - 'A' ) + 10;
		}
		else
		if ((pszStr[u4Idx] >= 'a') && (pszStr[u4Idx] <= 'f'))
		{
			u4ReturnValue = u4ReturnValue << 4;
			u4ReturnValue += (UINT32)(UINT8)(pszStr[u4Idx] - 'a') + 10;
		}
		else
		{
			return 0;
		}
	}

	return u4ReturnValue;
}
static UINT32 StrToDec(const CHAR* pszStr, UINT32 u4Len)
{
	UINT32 u4Idx;
	UINT32 u4ReturnValue = 0;

	if ((pszStr == NULL) || (u4Len == 0))
	{
		return 0;
	}

	// 0xFFFFFFFF = 4294967295
	u4Len = (u4Len > 10) ? 10 : u4Len;

	for (u4Idx = 0;
		u4Idx < u4Len;
		u4Idx++)
	{
		if ((pszStr[u4Idx] >= '0') && (pszStr[u4Idx] <= '9'))
		{
			u4ReturnValue *= 10;
			u4ReturnValue += (UINT32)(UINT8)(pszStr[u4Idx] - '0');
		}
		else
		{
			return 0;
		}
	}

	return u4ReturnValue;
}

static UINT32 StrToInt(const CHAR* pszStr)
{
	UINT32 u4Len;

	if (pszStr == NULL)
	{
		return 0;
	}

	u4Len = strlen(pszStr);

	if (u4Len > 2)
	{
		if ((pszStr[0] == '0') && (pszStr[1] == 'x'))
		{
			return StrToHex(&pszStr[2], u4Len - 2);
		}
	}

	return StrToDec(pszStr, u4Len);
}


size_t printf_decode(UINT8 *buf, size_t maxlen, const char *str)
{
	const char *pos = str;
	size_t len = 0;
	int val;

	while (*pos) {
		if (len + 1 >= maxlen)
			break;
		switch (*pos) {
		case '\\':
			pos++;
			switch (*pos) {
			case '\\':
				buf[len++] = '\\';
				pos++;
				break;
			case '"':
				buf[len++] = '"';
				pos++;
				break;
			case 'n':
				buf[len++] = '\n';
				pos++;
				break;
			case 'r':
				buf[len++] = '\r';
				pos++;
				break;
			case 't':
				buf[len++] = '\t';
				pos++;
				break;
			case 'e':
				buf[len++] = '\e';
				pos++;
				break;
			case 'x':
				pos++;
				val = hex2byte(pos);
				if (val < 0) {
					val = hex2num(*pos);
					if (val < 0)
						break;
					buf[len++] = val;
					pos++;
				} else {
					buf[len++] = val;
					pos += 2;
				}
				break;
			case '0':
			case '1':
			case '2':
			case '3':
			case '4':
			case '5':
			case '6':
			case '7':
				val = *pos++ - '0';
				if (*pos >= '0' && *pos <= '7')
					val = val * 8 + (*pos++ - '0');
				if (*pos >= '0' && *pos <= '7')
					val = val * 8 + (*pos++ - '0');
				buf[len++] = val;
				break;
			default:
                buf[len++] = '\\';
				break;
			}
			break;
		default:
			buf[len++] = *pos++;
			break;
		}
	}
	if (maxlen > len)
		buf[len] = '\0';

	return len;
}


static INT32 wps_computePinCheckSum(unsigned long pin)
{
    unsigned long accum = 0;
    INT32 digit;

    pin *= 10;
    accum += 3*((pin / 10000000) % 10);
    accum += 1*((pin / 1000000) % 10);
    accum += 3*((pin / 100000) % 10);
    accum += 1*((pin / 10000) % 10);
    accum += 3*((pin / 1000) % 10);
    accum += 1*((pin / 100) % 10);
    accum += 3*((pin / 10) % 10);

    digit = ((INT32)accum % 10);
    return ((10 - digit) % 10);
}


static int os_get_random(char *buf, size_t len)
{
    int i = 0;
    HAL_TIME_T _rTime;

#if 0 /* Benny'081126 */
    for(i = 0; i < len; i++)
    {
        buf[i] = 0x11;
    }
#else
    HAL_GetTime(&_rTime);
    for(i = 0; i < len; i++)
    {
        buf[i] = (char)rand();
        buf[i] += _rTime.u4Micros;
        buf[i] = ~buf[i];
    }
#endif    
    return 0;
}

static INT32 i4Wps_generatePin(void *ctx, char *out, UINT32 _8_digit)
{
    INT32 i;
    INT32 len;
    unsigned long pin;
	UINT8 own_addr[6];
    UINT8 zero[8]={'0','0','0','0','0','0','0','0'};
//    struct wpa_supplicant *wpa_s;
    x_net_wlan_get_mac_addr(own_addr);
  //  wpa_s = (struct wpa_supplicant *)ctx;

    if (!out)
        return -1;
    
    if (_8_digit)
    {
        len = WPS_PIN_LEN_8;
    }
    else
    {
        /* 4-digit */
        len = WPS_PIN_LEN_4;
    }
    do
    {
        os_get_random(out, len);
       //transfer to ascii 
        for (i = 0; i<len; i++)
        {
            out[i] += own_addr[5];
            out[i] = c_to_digit_a(out[i]);
        }
    }while(memcmp(out, zero, len) == 0);    //regenerate if all zero
    
    if (_8_digit)
    {
        /* compute checksum */
        out[WPS_PIN_LEN_8-1] = 0x00;
        pin = (unsigned long)StrToInt((CHAR *)out);
        i = wps_computePinCheckSum(pin);
        out[WPS_PIN_LEN_8-1] = c_to_digit_a(i);
    }
    printf("WPS PIN = [%c%c%c%c%c%c%c%c%c\n",
            *out, *(out+1), *(out+2), *(out+3),
            (_8_digit? *(out+4):']'),
            (_8_digit? *(out+5):' '),
            (_8_digit? *(out+6):' '),
            (_8_digit? *(out+7):' '),
            (_8_digit? ']':' '));
    return 0;
}

INT32 wps_generate_pin_code(char *wps_pin_code)
{
    char *out;   
    out = wps_pin_code;
    return i4Wps_generatePin(NULL, out, TRUE);
}


