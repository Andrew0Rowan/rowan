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


#ifndef __CONFIG_H_
#define __CONFIG_H_

//update package name
#define UPDATE_FILE "update.zip"

//decompress tool
#define UNZIP_CMD "unzip"

//where the tool store
#define UPTOOL "./usr/bin/uptool"

//SD /USB should reserve MIN_FREE_SIZE MB for unzip
#define MIN_FREE_SIZE 800
#define SIZE_MB (1024*1024)

//index of partiton nor in  image_list
#define NOR_INDEX 0
//index of partiton mmc in image_list
#define MMC_INDEX 3

#define MAX_SLOT   ((int) sizeof(dev_attrs)/sizeof(dev_attrs[0]))
#define MAX_INDEX  ((int) sizeof(image_list)/sizeof(image_list[0]))

//global variable

//to define which device to check for update 
//now USB portA,portB,SD
struct dev_attr{
	const char *dev_node;
	const char *mount_point;
}dev_attrs[]={
	{"/dev/sda1","/run/media/sda1" },
	{"/dev/sdb1","/run/media/sdb1" },
	{"/dev/mmcblk1p1","/run/media/mmcblk1p1"} ,
};

//to define which images will be update in update_app binary
struct images{
	const char *partition;
	const char *image_name;
}image_list[]={
	//partitions on nor :
	{"nor0","MBR_NOR"},
	{"UBOOT","lk.img"},
	{"TEE1","tz.img"},
	//partitions on emmc
	{"mmc0","MBR_NOR_EMMC"},
	{"RECOVERY","recovery.ramdisk.img"},
	{"ROOTFS","rootfs.ext4"},
	{"STATE","state.ext4"},
	{"mmc0boot0","recovery_lk.img"},
	{"TEE2","tz.img"},
	{"BOOTIMG","boot.img"},
};




#endif

