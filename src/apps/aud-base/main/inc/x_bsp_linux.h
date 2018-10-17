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




#ifndef _X_BSP_LINUX_H_
#define _X_BSP_LINUX_H_

#include <linux/ioctl.h>
#include "x_timer.h"

#define DEV_BSP "/dev/bsp"


/* struct */
typedef struct
{
    struct 
	{
        HAL_TIME_T tOlder;
        HAL_TIME_T tNewer;
    }in;
	struct
	{
        HAL_TIME_T tResult;
	}out;
}BSP_HAL_GET_DELTA_TIME_T;

typedef BOOL (*_CallBackfunc)(UINT32 u4FuncId);
typedef CHAR *(*_GetIcVerFunc)(void);

typedef struct
{
	UINT32	FuncID;
	BOOL	ret_val;
	_CallBackfunc CallBackfunc;
	_GetIcVerFunc GetIcVerFunc;
	
}BSP_GET_FUNCTION_ID;


typedef INT32 (*_SetEfuseKsbcFunc)(INT32 u4Ksbc);
typedef struct
{
	_SetEfuseKsbcFunc SetEfuseKsbFunc;
	
}BSP_COMMON_FUNCTION_LIST;


#define VERSION_BUF		0x100

typedef struct
{
	UINT32	u4InData;
  UINT32	u4OutData;	
}BSP_JTAG_DBG_Info;

typedef struct
{
    INT32	u4InData;
    INT32	u4OutData;	
}BSP_COMMON_Info;

/* ioctl id */
#define IOCTL_BSP_HAL_GET_TIME       		_IOR('b', 1, HAL_TIME_T)
#define IOCTL_BSP_HAL_GET_DELTA_TIME 		_IOWR('b', 2, BSP_HAL_GET_DELTA_TIME_T)
#define IOCTL_BSP_GET_IC_FUNCTION			_IOR('b', 3, BSP_GET_FUNCTION_ID)
#define IOCTL_BSP_GET_IC_SUB_VERSION			_IOR('b', 4, char *)
#define IOCTL_BSP_GetHDMI_JTAG_CONDITION	_IOR('b', 5, BSP_JTAG_DBG_Info)
#define IOCTL_BSP_Set_HDMI_JTAG				_IOWR('b', 6, BSP_JTAG_DBG_Info)
#define IOCTL_BSP_GET_IC_MAIN_VERSION			_IO('b', 7)
#define IOCTL_BSP_REBOOT			_IO('b', 8) // for upg prog reboot
#define IOCTL_BSP_POWER_DOWN			_IO('b', 9)
#define IOCTL_BSP_EFUSE_SET_KSBC	_IOR('b', 10, BSP_COMMON_Info)


/* error id */
#define ERR_BSP_HAL_GET_TIME            1
#define ERR_BSP_HAL_GET_DELTA_TIME_IN   2
#define ERR_BSP_HAL_GET_DELTA_TIME_OUT  3
#define ERR_BSP_GET_IC_FUNCTION_IN	4
#define ERR_BSP_GET_IC_FUNCTION_OUT	5
#define ERR_BSP_GET_IC_FUNCTION_PNT_NULL	6
#define ERR_BSP_GET_IC_VERSION_IN	7
#define ERR_BSP_GET_IC_VERSION_OUT	8
#define ERR_BSP_GET_IC_VERSION_PNT_NULL	9
#define ERR_BSP_HDMI_JTAG_IN	10
#define ERR_BSP_HDMI_JTAG_OUT	11
#define ERR_BSP_EFUSE_KSBC_IN	12
#define ERR_BSP_EFUSE_KSBC_OUT	13


#endif // _BSP_LINUX_H_
