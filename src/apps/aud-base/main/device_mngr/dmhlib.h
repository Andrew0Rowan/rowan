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


#ifndef _DMHLIB_H_
#define _DMHLIB_H_

#include    <stdio.h>
#include    <stdlib.h>
#include    <stddef.h>
#include    <unistd.h>
#include    <string.h>
#include    <dirent.h>
#include    <fcntl.h>
#include    <ctype.h>
#include    <sys/stat.h>
#include    <sys/un.h>
#include    <sys/ioctl.h>
#include    <sys/socket.h>
#include    <sys/types.h>
#include    <linux/types.h>
#include    <linux/netlink.h>
#include    <errno.h>
#include    <pwd.h>
#include    "dmh.h"

#define     DEFAULT_PATH_LEVEL  5

extern INT32 dmh_add_fake_hub_dev(const char *ps_dev_path,struct _DMH_DEV_T **pt_return_dev);
extern INT32 dmh_nfy_func(struct _DMH_DEV_T *pt_current_dev,UINT8 ui1_level,DEV_STATUS_T e_status);
extern INT32 dmh_parse_uevent_file(const CHAR *p_dev_path,BLK_UEVENT_ATTR_T *pt_uevent_attr);
extern INT32 dmh_free_uevent_file(BLK_UEVENT_ATTR_T *pt_uevent_attr);
extern INT32 dmh_get_attr_product_id(const CHAR *ps_dev_path);
extern INT32 dmh_get_attr_vendor_id(const CHAR *ps_dev_path);
extern INT32 dmh_add_real_hub_dev(const char *ps_dev_path,struct _DMH_DEV_T **pt_return_dev);
extern INT32 dmh_add_usb_storage_dev(const char *ps_dev_path);
extern INT32 dmh_add_usb_partition_dev(const char *ps_dev_path,const char *ps_dev_nod);
extern INT32 dmh_free_dev(struct _DMH_DEV_T *pt_usb_dev);
extern INT32 dmh_get_attr_num(const CHAR *ps_dev_path,const CHAR *ps_attr_name);
extern INT32 dmh_reg_root_nfy_fct(DMH_ROOT_DEV_T ui4_root_dev_id, DEV_DEV_ST_NFY_T *pt_nfy);

#endif

