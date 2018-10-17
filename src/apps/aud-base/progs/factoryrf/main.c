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


/*
 * Factory rf test only.
 * Note: PC Host will try to capture logs output from here as result,
 * so ensure output "success" or "success + useful infomation" if success,
 * otherwise only output "fail" log in case of failure.
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include "factory.h"

TCASE_RF_CMD cmd_list[] = 
{
	{TCASE_BT_TX_Start, "BT_TX_Start"},
	{TCASE_BT_TX_Stop, "BT_TX_Stop"},
	{TCASE_BT_RX_Start, "BT_RX_Start"},
	{TCASE_BT_RX_Stop, "BT_RX_Stop"},
	{TCASE_BT_Get_RxCount, "BT_Get_RxCount"},
	{TCASE_BT_Set_FreqOffset, "BT_Set_FreqOffset"},
	{TCASE_BT_Get_FreqOffset, "BT_Get_FreqOffset"},
	{TCASE_BT_Write_FreqOffset, "BT_Write_FreqOffset"},
	{TCASE_WIFI_ATE_Mode, "WIFI_ATEMode"},
	{TCASE_WIFI_TX_Start, "WIFI_TX_Start"},
	{TCASE_WIFI_TX_Stop, "WIFI_TX_Stop"},
	{TCASE_WIFI_RX_Start, "WIFI_RX_Start"},
	{TCASE_WIFI_Get_RxCount, "WIFI_Get_RxCount"},
	{TCASE_WIFI_RX_Stop, "WIFI_RX_Stop"},
	{TCASE_WIFI_Write_FreqOffset, "WIFI_Write_FreqOffset"},
	{TCASE_WIFI_Get_FreqOffset, "WIFI_Get_FreqOffset"},
	{TCASE_WIFI_Write_PwrOffset, "WIFI_Write_PwrOffset"},
	{TCASE_WIFI_Get_PwrOffset, "WIFI_Get_PwrOffset"},
	{TCASE_WIFI_Set_FreqOffset, "WIFI_Set_FreqOffset"},
	{TCASE_WIFI_Set_PwrOffset, "WIFI_Set_PwrOffset"},
	{TCASE_WIFI_Write_PwrOffset_5G, "WIFI_Write_PwrOffset_5G"},
	{TCASE_WIFI_Get_PwrOffset_5G, "WIFI_Get_PwrOffset_5G"},
	{TCASE_WIFI_Set_PwrOffset_5G, "WIFI_Set_PwrOffset_5G"},
	{TCASE_RF_MAX, NULL}
};

int get_input_index(char *str)
{
	int ret = -1;
	int i;
	
	if(str == NULL)
	{
		printf("Input paramters error!\n");
		return ret;
	}

	for(i = 0; i < sizeof(cmd_list)/sizeof(cmd_list[0]); i++)
	{
		if(strcmp(str, cmd_list[i].cmd) == 0)
		{
			ret = cmd_list[i].index;
			break;
		}

		if(i >= TCASE_RF_MAX)
		{
			printf("Input parameters error!\n");
			ret = -1;
		}
	}

	//printf("cmd_list[%d].cmd = %s\n", i, cmd_list[i].cmd);
	return ret;	
}

int main(int argc, char* argv[])
{
    int ret = -1;
	int index = -1;
	
	if(argc < 1)
	{
		printf("Input paramters error!\n");
		return -1;
	}
	index = get_input_index(argv[1]);
	
	if(index >= TCASE_RF_MAX || index < 0)
	{
		printf("Input paramters error!\n");
		return -1;
	}
    switch(index) {
		case TCASE_BT_TX_Start:
            ret = BT_TX_Start((char *)argv[2],(char *)argv[3],(char *)argv[4],(char *)argv[5]);
            break;
        case TCASE_BT_TX_Stop:
            ret = BT_TX_Stop();
            break;
        case TCASE_BT_RX_Start:
            ret = BT_RX_Start(argv[2], argv[3], argv[4], argv[5]);
            break;
        case TCASE_BT_RX_Stop:
            ret = BT_RX_Stop();
            break;
		case TCASE_BT_Get_RxCount:
			ret = BT_Get_RxCount();
            break;
		case TCASE_BT_Set_FreqOffset:
			ret = BT_Set_FreqOffset(argv[2]);
            break;
	    case TCASE_BT_Get_FreqOffset:
			ret = BT_Get_FreqOffset();
			break;
	    case TCASE_BT_Write_FreqOffset:
			ret = BT_Write_FreqOffset(argv[2]);
            break;
		case TCASE_WIFI_ATE_Mode:
			ret = WIFI_ATE_Mode();
            break;
		case TCASE_WIFI_TX_Start:
			ret = WIFI_TX_Start(argv[2], argv[3], argv[4], argv[5], argv[6], argv[7], argv[8], argv[9]);
            break;
		case TCASE_WIFI_TX_Stop:
			ret = WIFI_TX_Stop();
            break;
		case TCASE_WIFI_RX_Start:
			ret = WIFI_RX_Start(argv[2], argv[3], argv[4]);
            break;
		case TCASE_WIFI_Get_RxCount:
			ret = WIFI_Get_RxCount();
            break;
		case TCASE_WIFI_RX_Stop:
			ret = WIFI_RX_Stop();
            break;
		case TCASE_WIFI_Write_FreqOffset:
			ret = WIFI_Write_FreqOffset(argv[2]);
            break;
		case TCASE_WIFI_Get_FreqOffset:
			ret = WIFI_Get_FreqOffset();
            break;
		case TCASE_WIFI_Write_PwrOffset:
			ret = WIFI_Write_PwrOffset(argv[2], argv[3], argv[4], argv[5]);
            break;
		case TCASE_WIFI_Get_PwrOffset:
			ret = WIFI_Get_PwrOffset();
            break;
		case TCASE_WIFI_Set_FreqOffset:
			ret = WIFI_Set_FreqOffset(argv[2]);
            break;
		case TCASE_WIFI_Set_PwrOffset:
			ret = WIFI_Set_PwrOffset(argv[2], argv[3], argv[4], argv[5]);
            break;

		case TCASE_WIFI_Write_PwrOffset_5G:
			ret = WIFI_Write_PwrOffset_5G(argv[2], argv[3], argv[4], argv[5],argv[6], argv[7], argv[8], argv[9]);
            break;
		case TCASE_WIFI_Get_PwrOffset_5G:
			ret = WIFI_Get_PwrOffset_5G();
            break;
		case TCASE_WIFI_Set_PwrOffset_5G:
			ret = WIFI_Set_PwrOffset_5G(argv[2], argv[3], argv[4], argv[5],argv[6], argv[7], argv[8], argv[9]);
			break;
        default:
            printf("error cmd!\n");
            break;
    };

    if(0 != ret)
        printf("exec fail!\n");
    return 0;
}
