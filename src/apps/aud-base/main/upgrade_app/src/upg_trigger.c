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


#include <string.h>
#include <unistd.h>
#include <stdio.h>
//applicantion
#include <sys/prctl.h>
/* private */
#include "upg_trigger.h"
#include "typedefs.h"


//char g_url[256]={"http://192.168.1.101/"};
//char g_save_path[256]={"/tmp/update.zip"};
char g_url[256];
char g_save_path[256];



size_t _upgrade_download_id_file_cb( void *ptr, size_t size, size_t nmemb, void *stream)
{
    
    FILE *fp=(FILE *)stream;
	
    unsigned char *buffer=(unsigned char *)malloc(size*nmemb);
    if(buffer == NULL){
		printf("memory is not enough \n");
        return 0;
	}
	
    memcpy(buffer,ptr,size*nmemb);
    fwrite(buffer,1,size*nmemb,fp);
    free(buffer);
	
    return size*nmemb;
}

int _upgrade_download_id_file(char *url,char *save_path)
{ 
    FILE *fp = NULL;
	char curl_error_buf[CURL_ERROR_SIZE];
	CURLcode errcode = 0;
	CURL *curl = NULL;
    fp=fopen(save_path,"wb");
		
    if(fp == NULL){
        printf("fail to open save file:%s\n",save_path);
        return -1;
    }
    
    printf("----------start download id file-----\n");
    curl = curl_easy_init( ); 
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, _upgrade_download_id_file_cb);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA , fp);
    curl_easy_setopt(curl, CURLOPT_NOSIGNAL , 1);       
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT_MS, 1000); 
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 5); 
    curl_easy_setopt(curl, CURLOPT_ERRORBUFFER,curl_error_buf);
    curl_easy_setopt(curl, CURLOPT_URL,url);         
    errcode=curl_easy_perform(curl); 
	
    if(CURLE_OK!=errcode){
        printf("<UPG_APP>download fail , err:%d %s %s\n ",errcode,curl_easy_strerror(errcode),curl_error_buf);
        curl_easy_cleanup(curl); 
        curl_global_cleanup(); 
        fclose(fp);
        return -1;
    }else{
        printf("----------finish download id file-----\n");
        curl_easy_cleanup(curl); 
        curl_global_cleanup(); 
        fclose(fp);            
        return 0;
    }           
}

int _upgrade_set_server_url(const char *url)
{
    
    if(strlen(url)>255){		
        printf(("url length is too long\n"));
		return -1;
	}
	if(url != NULL){
        strncpy(g_url,url,strlen(url));   
	}
	
    printf("set server url = %s\n",g_url);	
	return 0;
}

char * _upgrade_get_server_url(const char *url)
{
	return g_url;
}

int _upgrade_set_save_path(const char *ps_save_path)
{

    if(strlen(ps_save_path)>255){		
        printf(("ps_save_path length is too long\n"));
		return -1;
	}
	printf_d("ps_save_path: %s\n", ps_save_path);

	if(ps_save_path != NULL){
        strncpy(g_save_path,ps_save_path,strlen(ps_save_path));  
    }
	
    printf("set save path url = %s\n",g_save_path);	
	return 0;
}

char * _upgrade_get_save_path(const char *ps_save_path)
{
	return g_save_path;
}

int _upgrade_get_ota_package(UPGRADE_FILE_INFO* info )
{		
	if( info == NULL){
        printf("info ls  NULL return !\n");
		return UPG_TRIGGER_FAIL;
	}

    strncpy(info->url,g_url,strlen(g_url));
    strncpy(info->save_path,g_save_path,strlen(g_save_path));
	
    if(_upgrade_download_id_file(info->url,info->save_path) != 0){
        printf_d(("download id file fail\n"));
        return UPG_TRIGGER_FAIL;
    }

    return UPG_TRIGGER_OK;
}

int _mtkapi_upgrade_download_from_net(const char* ps_url,const char* ps_save_path)
{
    int i4_ret = UPG_TRIGGER_FAIL;
    UPGRADE_FILE_INFO info;
	
	if(ps_url == NULL){
		printf_d("http command is null\n");
		return UPG_TRIGGER_FAIL;
	}
	
	if(_upgrade_set_server_url(ps_url) != 0){
        printf_d("set servel_url fail\n");
		return UPG_TRIGGER_INV_ARG;
	}

	if(_upgrade_set_save_path(ps_save_path)!= 0){
        printf_d("set save path fail\n");
		return UPG_TRIGGER_INV_ARG;
	}
	
    memset(&info,0, sizeof(UPGRADE_FILE_INFO));

    //download ota package 
    i4_ret = _upgrade_get_ota_package(&info);
	if(i4_ret != UPG_TRIGGER_OK){
        printf_d("set save path fail\n");		
		return i4_ret;
	}
	
     return UPG_TRIGGER_OK;
}

