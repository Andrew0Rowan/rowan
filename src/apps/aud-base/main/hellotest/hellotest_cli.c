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

#include "u_dbg.h"

#include "u_aee.h"

#include "hellotest_cli.h"
#include "curl/curl.h"
#include "pthread.h"

#ifdef CLI_SUPPORT
#include "u_cli.h"

/*-----------------------------------------------------------------------------
 * structure, constants, macro definitions
 *---------------------------------------------------------------------------*/
#define DEFAULT_DOWNLOAD "http://nodejs.org/dist/v4.2.3/node-v4.2.3-linux-x64.tar.gz"
#define MIN_BUFFER				10
#define MAX_BUFFER				1000000
#define DEFAULT_BUFFER   		1000
#define CURL_DOWNLOAD_TIMEOUT 30*60

typedef struct FILE_DOWNLOAD_INFO_T
 {
	 FILE *fp;
	 CURL *curl;
	 int start_time;
	 int user_buff_length;
	 int b_write_flag;
 }FILE_DOWNLOAD_INFO;

typedef struct DOWNLOAD_TREAD_PARAM_T
 {
	 char url[1024];
	 char save_path[1024] ;
	 int buff_size;//record buff size
	 int b_write_flag;
 }DOWNLOAD_TREAD_PARAM;


/*-----------------------------------------------------------------------------
 * private methods declarations
 *---------------------------------------------------------------------------*/
static INT32  _ht_cli_set_dbg_level (INT32 i4_argc, const CHAR** pps_argv);
static INT32  _ht_cli_get_dbg_level (INT32 i4_argc, const CHAR** pps_argv);
static INT32  _ht_cli_key_event(INT32 i4_argc, const CHAR** pps_argv);
static INT32 _cmd_download_write_file(INT32 i4Argc, const CHAR **szArgv);
void *		 _download_write_flash_thread(void *param);
unsigned int _download_write_flash_thread_cb( void *ptr, unsigned int size, unsigned int nmemb, void *stream);
static INT32 _cmd_download_write_flash_stop(INT32 i4Argc, const CHAR **szArgv);

/*-----------------------------------------------------------------------------
 * variable declarations
 *---------------------------------------------------------------------------*/
FILE_DOWNLOAD_INFO g_file_download_info = {0};
static int s_download_write_flash_flag = 1;//write flash flag

/* wifi test command table */
static CLI_EXEC_T at_ht_wifi_cmd_tbl[] =
{
    
    END_OF_CLI_CMD_TBL
};

/* main command table */
static CLI_EXEC_T at_ht_cmd_tbl[] =
{
 	{
        CLI_GET_DBG_LVL_STR,
        NULL,
        _ht_cli_get_dbg_level,
        NULL,
        CLI_GET_DBG_LVL_HELP_STR,
        CLI_GUEST
    },
	{
        CLI_SET_DBG_LVL_STR,
        NULL,
        _ht_cli_set_dbg_level,
        NULL,
        CLI_SET_DBG_LVL_HELP_STR,
        CLI_GUEST
    },
    {
        "key event trigger",
        "key",
        _ht_cli_key_event,
        NULL,
        "simulate user operation",
        CLI_GUEST
    },
  
	{
		"_cmd_download_write_flash",			
	 	"dwf",	
	 	_cmd_download_write_file,			
	 	NULL,	
	 	"download add write file",			
	 	CLI_GUEST
	},
	{
		"_cmd_download_write_flash_stop", 		
		"dwfs", 
		_cmd_download_write_flash_stop,		
		NULL,	
		"download write flash stop",	
		CLI_GUEST
	},
	END_OF_CLI_CMD_TBL
};
/* SVL Builder root command table */
static CLI_EXEC_T at_ht_root_cmd_tbl[] =
{
	{   
	    "hellotest",
	    "ht",
	    NULL,
	    at_ht_cmd_tbl,
	    "Hellotest commands",
	    CLI_GUEST
	},
	END_OF_CLI_CMD_TBL
};
/*-----------------------------------------------------------------------------
 * export methods implementations
 *---------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------
 * Name:    ht_cli_attach_cmd_tbl
 * Description:
 * Input arguments:  -
 * Output arguments: -
 * Returns:
 *----------------------------------------------------------------------------*/
INT32 ht_cli_attach_cmd_tbl(VOID)
{
    return (u_cli_attach_cmd_tbl(at_ht_root_cmd_tbl, CLI_CAT_APP, CLI_GRP_GUI));
}


/*-----------------------------------------------------------------------------
 * private methods implementations
 *---------------------------------------------------------------------------*/
/*-----------------------------------------------------------------------------
 * Name: _ht_cli_get_dbg_level
 *
 * Description: This API gets the current debug level.
 *
 * Inputs:  i4_argc         Contains the argument count.
 *          pps_argv        Contains the arguments.
 *
 * Outputs: -
 *
 * Returns: CLIR_OK         Routine successful.
 ----------------------------------------------------------------------------*/
static INT32 _ht_cli_get_dbg_level (INT32 i4_argc, const CHAR** pps_argv)
{
    INT32  i4_ret;

    i4_ret = u_cli_show_dbg_level(ht_get_dbg_level());

    return i4_ret;
}
/*-----------------------------------------------------------------------------
 * Name: _ht_cli_set_dbg_level
 *
 * Description: This API sets the debug level.
 *
 * Inputs:  i4_argc         Contains the argument count.
 *          pps_argv        Contains the arguments.
 *
 * Outputs: -
 *
 * Returns: CLIR_OK         Routine successful.
 ----------------------------------------------------------------------------*/
static INT32 _ht_cli_set_dbg_level (INT32 i4_argc, const CHAR** pps_argv)
{
    UINT16 ui2_dbg_level;
    INT32  i4_ret;

    i4_ret = u_cli_parse_dbg_level(i4_argc, pps_argv, &ui2_dbg_level);

    if (i4_ret == CLIR_OK){
        ht_set_dbg_level(ui2_dbg_level);
    }

    return i4_ret;
}
/*-----------------------------------------------------------------------------
 * Name: _ht_cli_key_event
 *
 * Description: This API is only for test purpose.
 *
 * Inputs:  i4_argc         Contains the argument count.
 *          pps_argv        Contains the arguments.
 *
 * Outputs: -
 *
 * Returns: CLIR_OK         Routine successful.
 ----------------------------------------------------------------------------*/
static INT32 _ht_cli_key_event(INT32 i4_argc, const CHAR** pps_argv)
{
	
    return CLIR_OK;
}

static int check_url_valid(const char *url)
{
	if(!strncmp(url, "http://", strlen("http://")) || 
	   !strncmp(url, "https://", strlen("https://")))
	   return 1;

	return 0;
}

void get_download_filename(const char *url, char *file_name)
{
	int i, j = 0;
    int start = 0;
    char *patterns[] = {"http://", "https://", NULL};

	for (i = 0; patterns[i]; i++)
        if (strncmp(url, patterns[i], strlen(patterns[i])) == 0)
            start = strlen(patterns[i]);

	j = 0;
    for (i = start; url[i] != '\0'; i++)
    {
        if (url[i] == '/')
        {
            if (i !=  strlen(url) - 1)
                j = 0;
            continue;
        }
        else
            file_name[j++] = url[i];
    }
    file_name[j] = '\0';
}

static INT32 _cmd_download_write_file(INT32 i4Argc, const CHAR **szArgv)
{
	int i4_ret;
	pthread_t ntid;
	static DOWNLOAD_TREAD_PARAM download_thread_param;
	char file_name[100] = {0};
	pthread_attr_t t_attr;

	memset(&download_thread_param, 0x00, sizeof(download_thread_param));
	if(i4Argc < 4 && i4Argc != 1)
	{
		DBG_ERROR((HTCLI_TAG"Arguments error!\n"));
		DBG_ERROR((HTCLI_TAG"Usage:cmd url buff_size\n"));
		return 0;
	}
	if(szArgv[1] != NULL && check_url_valid(szArgv[1]))
		strncpy(download_thread_param.url, szArgv[1], strlen(szArgv[1]));
	else 
		strncpy(download_thread_param.url, DEFAULT_DOWNLOAD, strlen(DEFAULT_DOWNLOAD));
	if(atoi(szArgv[2]) >= MIN_BUFFER && atoi(szArgv[2]) <= MAX_BUFFER)//100B-1M
		download_thread_param.buff_size = (int)(*szArgv[2]);
	else
		download_thread_param.buff_size = DEFAULT_BUFFER;//1k
	if(atoi(szArgv[3]) == 0)
		download_thread_param.b_write_flag = 0;
	else
		download_thread_param.b_write_flag = 1;

	get_download_filename(download_thread_param.url, file_name);
	snprintf(download_thread_param.save_path, sizeof(download_thread_param.save_path), "/misc/%s", file_name);

	i4_ret = pthread_attr_init(&t_attr);
	if(i4_ret != 0)
	{
		DBG_ERROR((HTCLI_TAG"pthread_attr_init error!\n"));
		goto _PTHREAD_ERR;
	}

	
	i4_ret = pthread_attr_setdetachstate(&t_attr, PTHREAD_CREATE_DETACHED);
	if(i4_ret != 0)
	{
		DBG_ERROR((HTCLI_TAG"pthread_attr_setdetachstate error!\n"));
		goto _SETDETACHSTATE_ERROE;
	}
	pthread_create(&ntid,0,_download_write_flash_thread,&download_thread_param);
	//prctl(PR_SET_NAME,"rcv_down_file");
	//pthread_join(ntid, NULL);
	pthread_attr_destroy(&t_attr);
	return 0;
_PTHREAD_ERR:
	return -1;
_SETDETACHSTATE_ERROE:
	pthread_attr_destroy(&t_attr);
	return -1;
}

void *_download_write_flash_thread(void *param)
{
	FILE *fp;
	char rm_cmd[200] = {0};
	DOWNLOAD_TREAD_PARAM *p_download_thread_param=(DOWNLOAD_TREAD_PARAM *)param;
	
	s_download_write_flash_flag = 1;
	while(s_download_write_flash_flag)
	{
		DBG_INFO((HTCLI_TAG"s_download_write_flash_flag = %d, file name : %s\n",
			s_download_write_flash_flag, p_download_thread_param->save_path));
		if(p_download_thread_param->b_write_flag)
		{
			fp=fopen(p_download_thread_param->save_path,"wb");
			if(fp == NULL)
			{
				DBG_ERROR((HTCLI_TAG"fail to open save file:%s\n",p_download_thread_param->save_path));
				return (void *)-1;
			}
		}
		//start download;
		DBG_INFO((HTCLI_TAG"--------start to download!-----------\n"));
		CURLcode errcode;
		char curl_error_buf[CURL_ERROR_SIZE];
		CURL *curl=curl_easy_init( ); 
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, _download_write_flash_thread_cb);

		g_file_download_info.fp=fp;
		g_file_download_info.curl=curl;
		g_file_download_info.start_time=time(NULL);
		g_file_download_info.user_buff_length = p_download_thread_param->buff_size;
		g_file_download_info.b_write_flag = p_download_thread_param->b_write_flag;
			
		curl_easy_setopt(curl, CURLOPT_WRITEDATA , &g_file_download_info); 
		curl_easy_setopt(curl, CURLOPT_TIMEOUT, CURL_DOWNLOAD_TIMEOUT); 
		curl_easy_setopt(curl, CURLOPT_ERRORBUFFER,curl_error_buf);
		curl_easy_setopt(curl, CURLOPT_URL,p_download_thread_param->url); 

		errcode=curl_easy_perform(curl); 
		if(errcode == 23)
		{
			DBG_INFO((HTCLI_TAG"user stop download!!!\n"));
			curl_easy_cleanup(curl); 
			curl_global_cleanup();
			if(p_download_thread_param->b_write_flag)
				fclose(fp);
			break;
		}
		if(CURLE_OK!=errcode)
		{
			DBG_ERROR((HTCLI_TAG"download fail , err:%d %s %s\n ",errcode,curl_easy_strerror(errcode),curl_error_buf));
			curl_easy_cleanup(curl); 
			curl_global_cleanup();
			if(p_download_thread_param->b_write_flag)
				fclose(fp);

			return (void *)-1;
		}
		else
		{
			//download complete
			DBG_INFO((HTCLI_TAG"--------end to download!-----------\n"));
			curl_easy_cleanup(curl); 
			curl_global_cleanup(); 
			if(p_download_thread_param->b_write_flag)
				fclose(fp);
		}
	usleep(500*1000);
	}

	if(p_download_thread_param->b_write_flag)
	{
		snprintf(rm_cmd, sizeof(rm_cmd), "rm -f %s", p_download_thread_param->save_path);
		DBG_INFO((HTCLI_TAG"Download thread stop!!! rm_cmd = %s\n", rm_cmd));
		//ipcd_exec(rm_cmd, NULL);
        system(rm_cmd);
		//ipcd_exec("sync", NULL);
        system("sync");
	}
	
	return (void*)0;
}

unsigned int _download_write_flash_thread_cb( void *ptr, size_t size, size_t nmemb, void *stream)
{
	FILE_DOWNLOAD_INFO *p_file_download_info=(FILE_DOWNLOAD_INFO *)stream;
	int length = p_file_download_info->user_buff_length;
	int recv_len = size*nmemb;

	if(s_download_write_flash_flag != 1)
		return -1;

	DBG_INFO((HTCLI_TAG"--------receive data:%d\n", recv_len));
	if(recv_len <= 0)
	{
		DBG_ERROR((HTCLI_TAG"No file!!!\n"));
		return -1;
	}
	unsigned char *buffer = (unsigned char *)malloc(length*sizeof(unsigned char)+1);
	if(buffer == NULL)
	{
		DBG_ERROR((HTCLI_TAG"Malloc memory error!\n"));
		return -1;
	}
	
	memset(buffer, 0x00, length+1);
	
	while(recv_len > length)
	{
		memcpy(buffer, ptr, length);
		recv_len -= length;
		ptr += length;
		if(p_file_download_info->b_write_flag)
		{
			fwrite(buffer, 1, length, p_file_download_info->fp);
			fsync(fileno(p_file_download_info->fp));
		}
		memset(buffer, 0x00, length);
	}

	memcpy(buffer, ptr, recv_len);
	if(p_file_download_info->b_write_flag)
	{
		fwrite(buffer, 1, recv_len, p_file_download_info->fp);
		fsync(fileno(p_file_download_info->fp));
	}
	free(buffer);
	
	return size*nmemb;
}

static INT32 _cmd_download_write_flash_stop(INT32 i4Argc, const CHAR **szArgv)
{
	s_download_write_flash_flag = 0;
	DBG_INFO((HTCLI_TAG"_shpa_write_stop: set shpa_write_flash_flag = 0\n"));
	
	return 1;
}
#endif
