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



#ifndef    _U_MISC_H_
#define    _U_MISC_H_
/*-----------------------------------------------------------------------------
                    include files
-----------------------------------------------------------------------------*/
#define MISC_FUNCTION_BEGIN printf("<MISC> %s %d line begin \n",__FUNCTION__,__LINE__);
#define MISC_FUNCTION_END   printf("<MISC> %s %d line end \n",__FUNCTION__,__LINE__);

#define MISC_DHCP_FUNCTION_BEGIN printf("<MISC_DHCP> %s %d line begin \n",__FUNCTION__,__LINE__)
#define MISC_DHCP_FUNCTION_END   printf("<MISC_DHCP> %s %d line end \n",__FUNCTION__,__LINE__)

#define DBG_INIT_LEVEL_APP_MISC   (DBG_LEVEL_ALL|DBG_LAYER_APP)

#define AUTO_IP_EVENT_SUCCESS_DHCPv4 	(UINT32)100
#define AUTO_IP_EVENT_FAIL_DHCPv4    	(UINT32)101
#define AUTO_IP_EVENT_SUCCESS_DHCPv6 	(UINT32)102
#define AUTO_IP_EVENT_FAIL_DHCPv6    	(UINT32)103

#define NETWORK_DEVICE_ACTIVE_EVENT		(UINT32)100
#define NETWORK_DEVICE_DEACTIVE_EVENT	(UINT32)200

#define	MISC_APPR_OK	              (0)
#define MISC_APPR_FAIL                (-1)

#define NI_WIRELESS_LINUX  "ath0"    /* wireless */
#define APP_NI_ETHERNET    NI_ETHERNET_0

#define APP_NI_WIRELESS    NI_WIRELESS_LINUX
#define NI_NET_NAME_DEF    NI_ETHERNET_0
#define AUTO_DHCP_TIMER_DELAY 	60000
#define DHCP_LEASE_TIME_DEFAULT (4*60)//4min


typedef struct _MISC_DEVICE_ACTIVE_MSG_T
{
    INT32 i4IfType;
	INT32 i4Event;
}MISC_DEVICE_ACTIVE_MSG_T;

typedef struct _MISC_NOTIFY_MSG_T
{
    UINT32  ui4_msg;
}MISC_NOTIFY_MSG_T;

typedef enum
{
	MISC_PRI_TIMER_MSG,
	MISC_PRI_DHCP_MSG,
	MISC_PRI_DEVICE_ACTIVE_MSG,
    MISC_PRI_MAX_MSG
} MISC_PRI_MSG_T;


typedef struct
{
    UINT32 ui4_ipaddr;
    UINT32 iu4_subnet;
    UINT32 ui4_gateway;
    UINT32 ui4_dns1;
    UINT32 ui4_dns2;
}NET_MANU_SETTING_INFO_T;

typedef struct
{
    BOOL	b_ethernet;
    BOOL    b_wireless;
} NET_DHCP_STATUS_T;

typedef struct _MISC_MSG_T
{
    UINT32          ui4_msg;
    VOID*           pv_param1;
    VOID*           pv_param2;
} MISC_MSG_T;

extern INT32  a_network_init_wlan(VOID);
extern VOID   _misc_process_auto_ip_timer_msg(VOID);
extern INT32  misc_dhcp_msg_process(MISC_NOTIFY_MSG_T * t_notify_msg);
extern INT32  misc_device_active_msg_process(MISC_DEVICE_ACTIVE_MSG_T * t_device_active_msg);
#endif

