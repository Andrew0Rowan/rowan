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


/*-----------------------------------------------------------------------------
 * $RCSfile: upg.c $
 * $Revision:0.1
 * $Date: 2017/04/17
 * $Author:pingan.liu
 * $CCRevision: $
 * $SWAuthor:  $
 * $MD5HEX: $
 *
 * Description:upg app thread


 *---------------------------------------------------------------------------*/
/*-----------------------------------------------------------------------------
                    include files
-----------------------------------------------------------------------------*/
#include <fcntl.h>
#include <time.h>

#include "upg.h"
#include "upg_cli.h"
#include "u_assistant_stub.h"
#include "u_cli.h"

static UPG_APP_OBJ_T t_g_upg_app = {0};

VOID _upg_parse_ota_progress_thread(VOID * arg)
{
	UPG_FUNCTION_BEGIN
	int i4_ret = 0;
	int progress = 0;
	int ota_progress_fd = -1;
	HANDLE_T h_app = NULL_HANDLE;

	ota_progress_fd = open("/tmp/ota_progress",O_RDONLY);
	if(-1 == ota_progress_fd)
	{
		printf("<UPG> open file fail,exit thread\n");
		u_thread_exit( );
	}
	else
	{
		while(1)
		{
			u_thread_delay(1000);

			read(ota_progress_fd,&progress,sizeof(int));

			printf("<UPG> current progress is %d \n",progress);

			ASSISTANT_STUB_OTA_PROGRESS_T ota_progress = {0};
			strncpy(ota_progress.command,"/system/ota_progress",ASSISTANT_STUB_COMMAND_MAX_LENGTH);

			ota_progress.progress = progress;

			/*send  ota progress to assistant_stub */
			i4_ret = u_am_get_app_handle_from_name(&h_app, ASSISTANT_STUB_THREAD_NAME);
			if(0 != i4_ret)
			{
			   printf("<WIFI_SETTING_PROC> get handle fail!,i4_ret=%ld\n",i4_ret);
			}
			i4_ret = u_app_send_appmsg(h_app,
									  E_APP_MSG_TYPE_ASSISTANT_STUB,
									  MSG_FROM_UPG,
									  ASSISTANT_STUB_CMD_OTA_PROGRESS,
									  &ota_progress,
									  sizeof(ASSISTANT_STUB_OTA_PROGRESS_T));
			if(0 == i4_ret)
			{
			   printf("<UPG> u_app_send_appmsg success !!!\n");
			}
			else
			{
			   printf("<UPG> u_app_send_appmsg fail !!!\n");
			}

			if(100 == progress)
			{
				printf("<UPG> upg is success 100% , thread exit!!!\n");
				close(ota_progress_fd);
				u_thread_exit( );
			}
		}
	}

	UPG_FUNCTION_END
}

static VOID _upg_handle_ota_upgrade(ASSISTANT_STUB_OTA_UPGRADE_T * ota_upgrade)
{
	UPG_FUNCTION_BEGIN
	int i4_ret=0;
	//int command_fd = -1;
	char ota_cmd[OTA_UPG_CMD_LENGTH];

	printf("ota_upgrade->command is %s\n",ota_upgrade->command);
	printf("ota_upgrade->ota_url is %s\n",ota_upgrade->ota_url);

#if 0
	/*create /tmp/update/command*/
	command_fd = open("/tmp/update/command", O_CREAT | O_RDWR,733);
	if (-1 == command_fd)
	{
		printf("open /tmp/update/command fail !!!\n");
		return ;
	}

	i4_ret = write(command_fd,ota_upgrade->ota_url,strlen(ota_upgrade->ota_url));
	if(i4_ret < 0)
	{
		printf("wirte /tmp/update/command fail !!!\n");
		close(command_fd);
		command_fd =-1;
		return ;
	}
	else
	{
		close(command_fd);
		command_fd = -1;
	}
#endif

	/*add start upg program flow:system call xiayin's program*/
    snprintf(ota_cmd,OTA_UPG_CMD_LENGTH,"/bin/upgrade_app %s",ota_upgrade->ota_url);
    printf("ota_cmd is %s\n",ota_cmd);
	i4_ret = system(ota_cmd);

	/*remove the /data/update.zip*/
	memset(ota_cmd,0,OTA_UPG_CMD_LENGTH);
	sprintf(ota_cmd,"rm -rf %s",ota_upgrade->ota_url);
	printf("upg_cmd is %s\n",ota_cmd);
	system(ota_cmd);

	if(0 == i4_ret)
	{
		/*save the time stamp to /data/upg_check*/
		time_t finish_time = time(NULL);
		struct tm *t = localtime(&finish_time);
		memset(ota_cmd,0,OTA_UPG_CMD_LENGTH);
		sprintf(ota_cmd,"echo upg upgrade success on %d-%02d-%02d %02d:%02d:%02d > /data/upg_check",\
			    t->tm_year + 1900,t->tm_mon+1,t->tm_mday,t->tm_hour,t->tm_min,t->tm_sec);
		printf("upg_cmd is %s\n",ota_cmd);
		system(ota_cmd);

		/*Set the version upgrade info to /data/upg_info*/
		memset(ota_cmd,0,OTA_UPG_CMD_LENGTH);
		sprintf(ota_cmd,"echo upgrade time:%d-%02d-%02d %02d:%02d:%02d > /data/upg_info",t->tm_year + 1900,t->tm_mon+1,t->tm_mday,t->tm_hour,t->tm_min,t->tm_sec);
		system(ota_cmd);

		memset(ota_cmd,0,OTA_UPG_CMD_LENGTH);
		sprintf(ota_cmd,"echo last version:`cat /temp/version/ali_version.ini` >> /data/upg_info");
		system(ota_cmd);

		/*Do the ota_before_reboot.sh before reboot*/
		memset(ota_cmd,0,OTA_UPG_CMD_LENGTH);
		sprintf(ota_cmd,"chmod +x /data/config_backup/ota_before_reboot.sh;sh /data/config_backup/ota_after_reboot.sh");
		system(ota_cmd);

		/*reboot system*/
		i4_ret = system("reboot -f");
	}
	else
	{
		printf("/bin/upgrade_app start fail !!!!!\n");
		/*save the time stamp to /data/upg_check*/
		time_t finish_time = time(NULL);
		struct tm *t = localtime(&finish_time);
		memset(ota_cmd,0,OTA_UPG_CMD_LENGTH);
		sprintf(ota_cmd,"echo upg upgrade fail on %d-%02d-%02d %02d:%02d:%02d > /data/upg_check",\
			    t->tm_year + 1900,t->tm_mon+1,t->tm_mday,t->tm_hour,t->tm_min,t->tm_sec);
		printf("upg_cmd is %s\n",ota_cmd);
		system(ota_cmd);

		/*Delete prompt,led and cloud.json config files after ota upgrade failed*/
		memset(ota_cmd,0,OTA_UPG_CMD_LENGTH);
		sprintf(ota_cmd,"rm /data/config_backup -rf;sync");
		system(ota_cmd);
	}

#if 0
	if(0 == i4_ret)
	{
		/*add thread to parse ota progress*/
	    i4_ret = u_thread_create(&h_thread,
	                             UPG_OTA_PROGRESS_THREAD_NAME,
	                             UPG_OTA_PROGRESS_THREAD_STACK_SIZE,
	                             UPG_OTA_PROGRESS_THREAD_PRIORATY,
	                             _upg_parse_ota_progress_thread,
	                             0,
	                             NULL);
	    if (0 != i4_ret)
	    {
	        printf("<UPG> ERR: create _upg_parse_ota_progress_thread failed, i4_ret [%ld],at L%d\r\n", i4_ret, __LINE__);
	        return AEER_FAIL;
	    }
	}
	else
	{
		printf("<UPG> upgrade_app start fali\n");
	}
#endif

	UPG_FUNCTION_END

}
static VOID _upg_handle_assistant_stub_msg(APPMSG_T * t_app_msg)
{
	UPG_FUNCTION_BEGIN
	switch(t_app_msg->ui4_msg_type)
	{
		case ASSISTANT_STUB_CMD_OTA_UPGRADE:
		{

			ASSISTANT_STUB_OTA_PROGRESS_T * ota_progress = (ASSISTANT_STUB_OTA_PROGRESS_T *)(t_app_msg->p_usr_msg);
			_upg_handle_ota_upgrade(ota_progress);
		}
		break;
		default:
		{
			printf("<UPG> ERROR ASSISTANT STUB MSG TYPE!\n");
		}
		break;

	}
	UPG_FUNCTION_END
}

static INT32 _upg_app_init(const CHAR* ps_name, HANDLE_T h_app)
{
	UPG_FUNCTION_BEGIN

	INT32    i4_ret;
    HANDLE_T h_thread = NULL_HANDLE;
	UINT8    volume;

	memset(&t_g_upg_app, 0, sizeof(UPG_APP_OBJ_T));

	t_g_upg_app.h_app=h_app;

	if (t_g_upg_app.b_app_init_ok)
    {
        return AEER_OK;
    }

#ifdef CLI_SUPPORT
	i4_ret = _upg_cli_attach_cmd_tbl();
	if ((CLIR_NOT_INIT != i4_ret) && (CLIR_OK != i4_ret))
	{
		printf("Err: _upg_cli_attach_cmd_tbl() failed, ret=%ld\r\n",i4_ret);
		return AEER_FAIL;
	}
	_upg_set_dbg_level(DBG_INIT_LEVEL_APP_UPG);
#endif/* CLI_SUPPORT */

	t_g_upg_app.b_app_init_ok=TRUE;

	UPG_FUNCTION_END

	return 0;
}

static INT32 _upg_app_process_msg(HANDLE_T     h_app,
									      UINT32       ui4_type,
										  const VOID*  pv_msg,
										  SIZE_T       z_msg_len,
										  BOOL         b_paused)
{
	UPG_FUNCTION_BEGIN

	UCHAR*       puc_name;
    INT32        i4_ret = 0;
	APPMSG_T *   app_msg = (APPMSG_T * )pv_msg;

	if (!t_g_upg_app.b_app_init_ok)
    {
        return AEER_FAIL;
    }

	if (ui4_type < AMB_BROADCAST_OFFSET)
	{
		/* private message */
		switch(ui4_type)
		{
			case E_APP_MSG_TYPE_ASSISTANT_STUB:
			{
				printf("<UPG> E_APP_MSG_TYPE_ASSISTANT_STUB\n");
				switch(app_msg->ui4_sender_id)
				{
					case MSG_FROM_ASSISTANT_STUB:
					{
						printf("<UPG> MSG_FROM_ASSISTANT_STUB\n");
						_upg_handle_assistant_stub_msg(app_msg);
					}
					break;

					default:
					{
						printf("<UPG> ERROR sender id\n");
					}
					break;
				}
            }
			break;

			default:
			break;
		}
	}
	else
	{
		switch(ui4_type)
		{
			case E_APP_MSG_TYPE_STATE_MNGR:
			{
				printf("<UPG> E_APP_MSG_TYPE_STATE_MNGR\n");
            }
			break;

			default:
			break;
		}
	}

	UPG_FUNCTION_END

    return 0;
}

static INT32 _upg_app_exit(HANDLE_T h_app, APP_EXIT_MODE_T e_exit_mode)
{
	UPG_FUNCTION_BEGIN
	t_g_upg_app.b_app_init_ok=FALSE;
	UPG_FUNCTION_END

	return 0;
}

VOID a_upg_register(AMB_REGISTER_INFO_T* pt_reg)
{
	UPG_FUNCTION_BEGIN

    if (t_g_upg_app.b_app_init_ok)
    {
		printf("<UPG> a_upg_register done,just return\n");
        return;
    }

    strncpy(pt_reg->s_name, UPG_THREAD_NAME,sizeof(UPG_THREAD_NAME));
    pt_reg->t_fct_tbl.pf_init                   = _upg_app_init;
    pt_reg->t_fct_tbl.pf_exit                   = _upg_app_exit;
    pt_reg->t_fct_tbl.pf_process_msg            = _upg_app_process_msg;
    pt_reg->t_desc.ui8_flags = AEE_FLAG_WRITE_CONFIG|AEE_FLAG_WRITE_FLM|AEE_FLAG_WRITE_TSL|AEE_FLAG_WRITE_SVL;
    pt_reg->t_desc.t_thread_desc.z_stack_size = UPG_STACK_SIZE;
    pt_reg->t_desc.t_thread_desc.ui1_priority = UPG_THREAD_PRIORITY;
    pt_reg->t_desc.t_thread_desc.ui2_num_msgs = UPG_NUM_MSGS;
    pt_reg->t_desc.ui4_app_group_id = 0;
    pt_reg->t_desc.ui4_app_id = 0;
    pt_reg->t_desc.ui2_msg_count = UPG_MSGS_COUNT;
    pt_reg->t_desc.ui2_max_msg_size = UPG_MAX_MSGS_SIZE;

	UPG_FUNCTION_END
}
