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
#include <stdio.h>
#include <pthread.h>

#include "u_common.h"
#include "u_dbg.h"
#include "curl/curl.h"
#include "upg_utility.h"
#include "upg_control.h"

#undef   DBG_LEVEL_MODULE
#define  DBG_LEVEL_MODULE       DBG_LEVEL_ALL

#define UPG_TRIGGER_TAG                "<UPG_CTRL>"
#define DBG_UPG_TRIGGER_INFO(_stmt)    do{DBG_INFO((UPG_TRIGGER_TAG));DBG_INFO(_stmt);}while(0)
#define DBG_UPG_TRIGGER_API(_stmt)     do{DBG_API((UPG_TRIGGER_TAG));DBG_API(_stmt);}while(0)
#define DBG_UPG_TRIGGER_DEFAULT(_stmt) do{DBG_ERROR((UPG_TRIGGER_TAG));DBG_ERROR(_stmt);}while(0)
#define DBG_UPG_TRIGGER_ERR(_stmt)                                          \
do{                                                                         \
    DBG_ERROR((UPG_TRIGGER_TAG));                                           \
    DBG_ERROR(("(%s:%s:%d)-->",__FILE__,__func__,__LINE__));                \
    DBG_ERROR(_stmt);                                                       \
}while(0)

#define UPG_TRIGGER_ASSERT(err) \
    do{                 \
        if(err<0){      \
            DBG_ERROR(("<upg><ASSERT> %s L%d err:%d\n",__FUNCTION__,__LINE__,err)); \
            while(1);   \
        }               \
    }while(0)


#define UPG_TRIGGER_OK                   ((int)    0)        /* OK */
#define UPG_TRIGGER_FAIL                 ((int)   -1)        /* Something error? */
#define UPG_TRIGGER_INV_ARG              ((int)   -2)        /* Invalid arguemnt. */

/* define value*/
#define UPGRADE_IMG_NUM             (4)                  /* assuming only 4 images to upgrade */
#define UPG_CMD_LEN                 (255)
#define UPG_MAX_FILE_PATH_LEN       (255)
#define CURL_DOWNLOAD_TIMEOUT       (10*60)

#define NETWORK_UPGRADE_BASE_URL    "http://192.168.1.100/"

/* for id file download */
#define UPGRADE_ID_FILE_NAME        "upg_id_file.bin"
#define UPGRADE_ID_FILE_SAVE_PATH   "/data/misc/"
#define UPGRADE_ID_FILE_TMP_PATH    "/tmp/"

/* for sectionally package download */
#define UPGRADE_OTA_FILE_NAME       "update.bin"
#define UPGRADE_OTA_FILE_SAVE_PATH  "/tmp/update/"

/* for full update package download */
#define UPGRADE_FULL_PKG_NAME       "update.zip"
#define UPGRADE_FULL_PKG_SAVE_PATH  "/data/misc/"

#define UPGRADE_DOWNLOAD_RESUME_TIMES           (60)        /* try 60 times, about 1 min if connection timeout */
#define UPGRADE_TRANSFER_FLASH_SIZE             (1<<21)     /*  transfer around 2MB data to upg_app for flashing every times */

typedef enum
{
    UPDATE_FROM_CARD = 0x00,
    UPDATE_FROM_NET  = 0x01,
    UPDATE_FROM_TEMP = 0x02,
    UPDATE_GET_OTA_INFO= 0X03
}UPDATE_TYPE;

/*app private  msg*/

typedef struct UPGRADE_FILE_INFO_T
{
    BOOL is_force_update;
    INT32 upgrade_file_version;
    INT32 upgrade_file_size;
    BOOL is_low_mem_upg;
    CHAR url[256];
    CHAR save_path[256] ;
    UCHAR fgfs_ubi;
}UPGRADE_FILE_INFO;

typedef struct UPGRADE_IMAGE_INFO_T
{
    ZIP_HEADER img_header;
    INT32 img_data_offset;
    INT32 img_blk_size;
    INT32 img_flash_unit_size;
    BOOL is_flash_unit_done;
    INT32 img_downloaded_size;
    BOOL is_img_download_done;
    INT32 img_flashed_size;
    BOOL is_img_flashed_done;

}UPGRADE_IMAGE_INFO;

typedef struct DOWNLOAD_TREAD_PARAM_T
{
    CHAR url[256];
    CHAR save_path[256];
    BOOL is_full_pkg_update;
    INT32 upg_file_size;
    FILE* fp;
    CURL* curl;
    BOOL is_curl_inited;
    CURLcode errcode;
    CHAR curl_error_buf[CURL_ERROR_SIZE];
    BOOL  is_head_info;
    INT32 requested_size;
    INT32 downloaded_size;
    CHAR* conntent_buf;
    BOOL  is_request_done;
    pthread_cond_t flash_cond;
    pthread_mutex_t flash_mutex;
    CHAR img_num;
    CHAR processing_img_idx;
    UPGRADE_IMAGE_INFO image[UPGRADE_IMG_NUM];
}DOWNLOAD_TREAD_PARAM;

typedef struct UPGRADE_DL_MSG_T
{
    APPMSG_T pt_msg;
    DOWNLOAD_TREAD_PARAM* dl_param;
}UPGRADE_DL_MSG;

typedef SIZE_T (*CURL_CB_FCT)(VOID*,SIZE_T,SIZE_T,VOID*);

/*------------------------------------------------------------------------------
                                            funcitons declarations
------------------------------------------------------------------------------*/
INT32 _upgrade_download_wakeup_data_flash_done(DOWNLOAD_TREAD_PARAM* dl_param, BOOL* is_done);
INT32 _upgrade_start_download_firmware(UINT32 t_delivery_type, const CHAR* ps_url, const CHAR* ps_fwPath);
CHAR* _upgrade_get_save_path(const char *ps_save_path);
CHAR* _upgrade_get_server_url(const char *url);

#endif /* _UPG_TRIGGER_H_ */
