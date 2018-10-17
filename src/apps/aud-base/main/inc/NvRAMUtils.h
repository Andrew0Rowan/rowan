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


#ifndef _NVRAM_UTILS_H__
#define _NVRAM_UTILS_H__

#include <stdbool.h>

struct WIFI_CFG_STRUCT {
    unsigned char mac[6];
};

struct BT_CFG_STRUCT {
    unsigned char mac[6];
};

/* Length serial number, MUST NOT TO MODIFY */
#define SN_LEN 23
/* Length PCB serial number, MUST NOT TO MODIFY */
#define PCBSN_LEN 18


#ifdef __cplusplus
extern "C" {
#endif

/**
* Read WiFi configuration from MTK data NvRAM.
* @param param the WiFi configuration to be read from MTK data NvRAM.
* @return whether the read requested fully succeeded.
*/
bool r_WiFi_CFG(struct WIFI_CFG_STRUCT* param);

/**
* Write WiFi configuration to MTK data NvRAM.
* @param param the WiFi configuration to be written to MTK data NvRAM.
* @return whether the write requested fully succeeded.
*/
bool w_WiFi_CFG(struct WIFI_CFG_STRUCT* param);

/**
* Read Bluetooth configuration from MTK data NvRAM.
* @param param the Bluetooth configuration to be read from MTK data NvRAM.
* @return whether the read requested fully succeeded.
*/
bool r_BT_CFG(struct BT_CFG_STRUCT* param);

/**
* Write Bluetooth configuration to MTK data NvRAM.
* @param param the WiFi configuration to be written to MTK data NvRAM.
* @return whether the write requested fully succeeded.
*/
bool w_BT_CFG(struct BT_CFG_STRUCT* param);

/**
* Backup  MTK data NvRAM (such as WiFi/Bluetooth/SN/PCBSN configuration) to MTK NvRAM raw partition.
* @return whether the backup requested fully succeeded.
*/
bool b_CFG();

bool r_XOCAP();

/**
* Read serial number from MTK data NvRAM.
* @param sn the buffer to store data from MTK data NvRAM,
* expected length not more than SN_LEN ,excluding the terminating null byte ('\0').
* @return whether the read requested fully succeeded.
*/
bool r_SN(char* sn);

/**
* Write serial number to MTK data NvRAM.
* @param sn the buffer to store data want write to  MTK data NvRAM,
* expected length not more than SN_LEN ,excluding the terminating null byte ('\0').
* @return whether the write requested fully succeeded.
*/
bool w_SN(char* sn);

/**
* Read PCB serial number from MTK data NvRAM.
* @param pcbsn the buffer to store data from MTK data NvRAM,
* expected length not more than PCBSN_LEN ,excluding the terminating null byte ('\0').
* @return whether the read requested fully succeeded.
*/
bool r_PCBSN(char* pcbsn);

/**
* Write PCB serial number to MTK data NvRAM.
* @param pcbsn the buffer to store data want write to  MTK data NvRAM,
* expected length not more than PCBSN_LEN ,excluding the terminating null byte ('\0').
* @return whether the write requested fully succeeded.
*/
bool w_PCBSN(char*pcbsn);

#ifdef __cplusplus
}
#endif
#endif /* _NVRAM_UTILS_H__ */
