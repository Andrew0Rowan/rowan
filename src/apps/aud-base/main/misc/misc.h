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



#ifndef    _MISC_H_
#define    _MISC_H_
/*-----------------------------------------------------------------------------
                    include files
-----------------------------------------------------------------------------*/
#include "u_misc.h"
#include "u_amb.h"
#include "u_timerd.h"

#define APPUTILR_OK                              ((INT32)0)
#define APPUTILR_FAIL                            ((INT32)-1)
#define APPUTILR_INV_ARG                         ((INT32)-2)
#define APPUTILR_LACK_BUFFER                     ((INT32)-3)
#define APPUTILR_NOT_EXIST                       ((INT32)-4)

#define IFCONFIG_FLAG  (1)
#define WPA_CUPPLICANT (0)

#define DHCP_LEASE_TIME_NUM_MAX_LEN  ((UINT8)10)
#define DHCP_LEASE_TIME_PATH 		 ("/var/dhcpc.lease")

typedef struct _MISC_OBJ_T
{
    HANDLE_T                h_app;
    AMB_APP_STATUS_TYPE_T   t_app_status;
    BOOL                    b_app_init_ok;
#if 0
    HANDLE_T                h_network_auto_ip;
#else
	TIMER_TYPE_T			auto_ip_timer;
#endif
}MISC_OBJ_T;

typedef struct _MISC_TIMER_MSG_T
{
    UINT8   ui1_timer_type;
}MISC_TIMER_MSG_T;

typedef enum    _MISC_TIMER_TYPE_E
{
#if 1//CONFIG_SUPPORT_NETWORK_AUTO_IP
    MISC_NETWORK_AUTO_IP_TIMER,
    MISC_NETWORK_MAX_TIMER
#endif
}MISC_TIMER_TYPE_E;

typedef enum
{
    TIMER_MSG_AUTO_IP,
    TIMER_MSG_MAX
} MISC_TIMER_MSG_E;

#endif

