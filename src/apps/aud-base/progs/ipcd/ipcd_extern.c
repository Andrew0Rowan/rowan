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


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ipcd.h"

typedef struct {
	const char* original_cmd;
	const char* replace_cmd;
}CUSTOM_COMMAND_T;

static CUSTOM_COMMAND_T s_custom_command_func_map[] = {
	{"insmod", "insmod_r"},
	{"rmmod", "rmmod_r"},
	{NULL, NULL}
};

static int parse_command_line(char * in_command_line, char **command_name, char **args)
{

	char *pc_token = NULL;

	*command_name = strtok_r(in_command_line, " ", &pc_token);
	if(!(*command_name))
	{
		DEBUG_ERROR("invalid third param\n");
		return -1;
	}
	
	*args = *command_name + strlen(*command_name) + 1;
	return 0;
}

static const char* find_custom_command(const char *orignal_command){

	CUSTOM_COMMAND_T *pt_cmd = (CUSTOM_COMMAND_T *)s_custom_command_func_map;

	while(pt_cmd->original_cmd) {
		if(0 == strcmp(pt_cmd->original_cmd, orignal_command)){
			return pt_cmd->replace_cmd;
		}

		pt_cmd ++;
	}

	return orignal_command;
}

char * get_real_command(char *in_command) {

	char *command_name = NULL;
	char *args  = NULL;
	const char* real_command = NULL;
	char *new_command_line = NULL;
	int new_command_len = strlen(in_command);


	if(0 != parse_command_line(in_command, &command_name, &args)){
		DEBUG_ERROR("parse_command_line error\n");
		return NULL;
	}

	real_command = find_custom_command((const char *)command_name);

	new_command_len = strlen(real_command) + 1;
	new_command_len += args ? strlen(args) : 0;

	new_command_line = (char *)calloc(new_command_len + 1, 1);
	if(!new_command_line){
		DEBUG_ERROR("memory error\n");
		return NULL;
	}

	if(args)
	  snprintf(new_command_line, new_command_len + 1, "%s %s", real_command, args);
	else
	  snprintf(new_command_line, new_command_len + 1, "%s", real_command);

	return new_command_line;
}

