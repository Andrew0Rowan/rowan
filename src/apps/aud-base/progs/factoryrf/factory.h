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


#ifndef __FACTORY_H__
#define __FACTORY_H__

enum{
	TCASE_BT_TX_Start = 1,
    TCASE_BT_TX_Stop,
    TCASE_BT_RX_Start,
    TCASE_BT_RX_Stop,
    TCASE_BT_Get_RxCount,
    TCASE_BT_Set_FreqOffset,
    TCASE_BT_Get_FreqOffset,
    TCASE_BT_Write_FreqOffset,
    TCASE_WIFI_ATE_Mode,
    TCASE_WIFI_TX_Start,
	TCASE_WIFI_TX_Stop,
	TCASE_WIFI_RX_Start,
	TCASE_WIFI_Get_RxCount,
	TCASE_WIFI_RX_Stop,
	TCASE_WIFI_Write_FreqOffset,
	TCASE_WIFI_Get_FreqOffset,
	TCASE_WIFI_Write_PwrOffset,
	TCASE_WIFI_Get_PwrOffset,
	TCASE_WIFI_Set_FreqOffset,
	TCASE_WIFI_Set_PwrOffset,
	TCASE_WIFI_Write_PwrOffset_5G,
	TCASE_WIFI_Get_PwrOffset_5G,
	TCASE_WIFI_Set_PwrOffset_5G,
	TCASE_RF_MAX
};

typedef struct
{
	int index;
	char cmd[30];
}TCASE_RF_CMD;

int BT_TX_Start(char *freq,char *pkttype,char *payload,char *pktcount);
int BT_TX_Stop(void);
int BT_RX_Start(char* freq, char *type, char *payload, char *second_time);
int BT_RX_Stop(void);
int BT_Get_RxCount(void);
int BT_Set_FreqOffset(char *freqOffset);
int BT_Get_FreqOffset(void);
int BT_Write_FreqOffset(char *freqOffset);

int WIFI_ATE_Mode(void);
int WIFI_TX_Start(char *freq, char *rate,
				char *bandwidth,char *pwr0, char *pwr1,
				char *pwr2, char *freqoffset, char *pktcount);
int WIFI_TX_Stop(void);
int WIFI_RX_Start(char *freq, char *rate, char *bandwidth);
int WIFI_Get_RxCount(void);
int WIFI_RX_Stop(void);
int WIFI_Write_FreqOffset(char *freqOffset);
int WIFI_Get_FreqOffset(void);
int WIFI_Write_PwrOffset(char *l_offset, char *m_offset, char *h_offset, char *all_offset);
int WIFI_Get_PwrOffset(void);
int WIFI_Set_FreqOffset(char *freqOffset);
int WIFI_Set_PwrOffset(char *l_offset, char *m_offset, char *h_offset, char *all_offset);

#endif
