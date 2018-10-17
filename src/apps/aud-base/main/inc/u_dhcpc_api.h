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



#ifndef U_DHCPC_API_H
#define U_DHCPC_API_H

enum
{
    DHCPC_R_OK              =  0,
    DHCPC_R_INIT_FAILED     = -1,
    DHCPC_R_DEINIT_FAILED   = -2,
    DHCPC_R_NI_NOT_EXIST    = -3,
    DHCPC_R_MAP_FULL        = -4,
    DHCPC_R_START_FAILED    = -5,
    DHCPC_R_OSAI_FAILED     = -6,
    DHCPC_R_STOP_FAILED     = -7,
    DHCPC_R_NO_REQUEST_ID   = -8,
    DHCPC_R_RESTART_FAILED  = -9,
    DHCPC_R_GET_INFO_FAILED = -10,
    DHCPC_R_INVALID_ARGS    = -11,
    DHCPC_R_INTERNAL_FAILED = -12,
    DHCPC_R_ALREADY_INIT    = -13,
    DHCPC_R_NOT_INIT        = -14
};

typedef enum
{
    DHCPC_EVENT_SUCCESS_DHCPv4     = 1,
    DHCPC_EVENT_FAILURE_DHCPv4     = 2,
    DHCPC_EVENT_NEW_ADDRESS_DHCPv4 = 3,
    DHCPC_EVENT_SUCCESS_LINKLOCAL  = 4,
    DHCPC_EVENT_FAILURE_LINKLOCAL  = 5,
    DHCPC_EVENT_SUCCESS_AUTOv6 = 6,
    DHCPC_EVENT_FAILURE_AUTOv6 = 7,
} DLNA_DHCPC_EV_T;

typedef VOID (*x_dhcpc_nfy_fct) (DLNA_DHCPC_EV_T e_event);

//#if CONFIG_MW_CUSTOM_SFLP
typedef INT32 (*x_pctool_upg_nfy)(VOID);
//#endif


#define IPV6_NETADDR_LEN 16
#define IPV6_PTRADDR_LEN 46

typedef enum
{
	IPV6_MODE_IGNORE_RA = 0,
	IPV6_MODE_RA,
}IPV6_IP_MODE;

typedef enum
{
	IPV6_SETTING_MANUAL =0,
	IPV6_SETTING_AUTO,
}IPV6_SETTING_TYPE;


typedef enum{
	NET_V6_MANUAL = 0,
	NET_V6_STATELESS,
	NET_V6_STATEFUL,
}NET_IP_STATE_V6;



typedef struct 
{
	int RA_mode;
	int ip_mode;
	int dns_mode;
	int ip_prefix;
	char ip_addr[46];
	char gw_addr[46];
	char dns1[46];
	char dns2[46];
	x_dhcpc_nfy_fct cb;
}IPV6_CONFIG_INFO_T;


#endif
