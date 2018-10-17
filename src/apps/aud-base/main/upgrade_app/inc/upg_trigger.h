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


#ifndef _UPG_TRIGGER_H_
#define _UPG_TRIGGER_H_
/*-----------------------------------------------------------------------------
                    include files
-----------------------------------------------------------------------------*/
//#include "/worktmp/codebase/yocto_8516_slc/src/apps/aud-base/library/upg_trigger/curl/curl.h"
#include "curl/curl.h"

#include <stdio.h>
 
#define UPG_TRIGGER_OK                   ((int)    0)        /* OK */
#define UPG_TRIGGER_FAIL                 ((int)   -1)        /* Something error? */
#define UPG_TRIGGER_INV_ARG              ((int)   -2)        /* Invalid arguemnt. */


/* define value*/
#define BUILD_VERSION_LEN_MAX  64
#define UPG_CMD_LEN            ((char) 255)
#define UPG_MAX_FILE_PATH_LEN        ((int)1024)
#define CURL_DOWNLOAD_TIMEOUT  10*60

#define NETWORK_UPGRADE_BASE_URL    "http://192.168.1.100/"
#define UPGRADE_SAVE_PATH           "/data/misc/"
#define UPGRADE_ID_FILE_NAME        "upg_id_file.bin"
#define UPGRADE_OTA_FILE_NAME       "ota.zip"
#define OTA_PACKAGE_PATH            "/data/misc/ota.zip"
#define PROPERTY_VALUE_MAX 92

/*app private  msg*/

typedef struct UPGRADE_FILE_INFO_T
{
    char force_update[256];
    int upgrade_file_version;
    int upgrade_file_size;

    char url[256];
    char save_path[256] ;
    unsigned char fgfs_ubi;
    
}UPGRADE_FILE_INFO;

typedef struct FILE_DOWNLOAD_INFO_T
{
    FILE *fp;
    CURL *curl;
    int total_file_size;
    int downloaded_size;
    int prev_percentage;
    int download_status;
    int start_time;
    int in_progress;
}FILE_DOWNLOAD_INFO;

/*------------------------------------------------------------------------------
                                            funcitons declarations
------------------------------------------------------------------------------*/

int _mtkapi_upgrade_download_from_net( const char* ps_url, const char* ps_save_path);
char * _upgrade_get_save_path(const char *ps_save_path);
char * _upgrade_get_server_url(const char *url);
                                                                                                 
#endif /* _UPG_TRIGGER_H_ */
