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
#include "factory.h"

#define CMD_MAX_LENGTH    100
#define CMD_EXEC_SUCC   	"Exe Success"
#define CMD_EXEC_FAIL		"Exe Fail"

#define PRINTERR   \
{ \
	printf("%s\n", CMD_EXEC_FAIL); \
}

#define PRINTSUCC   \
{ \
	printf("%s\n", CMD_EXEC_SUCC); \
}

enum{
	TCASE_OK = 0,
	TCASE_ERR = -1
}TCASE_RETURN_STATUS;

int factory_system_call(char *func, char *cmd)
{
	int ret = TCASE_OK;

	if(cmd == NULL)
		return TCASE_ERR;

	printf("cmd: <%s>:%s\n", func, cmd);
	ret	= system(cmd);
	if(-1 == ret)
	{
		ret = -1;
	}
	else if(WIFEXITED(ret))
	{
		if (WEXITSTATUS(ret) != 0)
				ret = -1;
	}
	else
	{
		ret = -1;
	}

	return ret;
}

int BT_TX_Start(char* freq,char* pkttype,char* payload,char* pktcount)
{
	if(freq == NULL || pkttype == NULL || payload == NULL || pktcount == NULL)
	{
		printf("<c_BT_TX_Start>argument is null!\n");
		PRINTERR;
		return TCASE_ERR;
	}

	char cmd[CMD_MAX_LENGTH];

	//kill btservice
	memset(cmd, "0x00", CMD_MAX_LENGTH);
	strcpy(cmd, "ps | grep /usr/bin/btservice | awk '{print $1}' | xargs kill");
	factory_system_call(__FUNCTION__, cmd);
	usleep(50*1000);
	factory_system_call(__FUNCTION__, cmd);


	//close wifi
	memset(cmd, "0x00", CMD_MAX_LENGTH);
	strcpy(cmd, "echo 0 > /dev/wmtWifi");
	if(factory_system_call(__FUNCTION__, cmd) != 0)
	{
		PRINTERR;
		return TCASE_ERR;
	}

	int i_freq = atoi(freq);
	if(i_freq < 2402 || i_freq > 2480)
	{
		printf("<c_BT_TX_Start>i_freq error, freq is 2402 -- 2480\n");
		PRINTERR;
		return TCASE_ERR;
	}
	int i_pkttype = atoi(pkttype);
	int i_payload = atoi(payload);
	int i_pktcount = atoi(pktcount);

	int channel = atoi(freq) - 2402;
	int d_pattern, d_type;

	switch(i_payload)
	{
		case 0:
			d_pattern = 3;
			break;
		case 1:
		case 2:
			d_pattern = 4;
			break;
		default:
			printf("<c_BT_TX_Start>pattern is error, use default!\n");
			break;
	}

	switch(i_pkttype)
	{
		case 0:
			d_type = 4;
			break;
		case 1:
			d_type = 11;
			break;
		case 2:
			d_type = 15;
			break;
		case 3:
			d_type = 36;
			break;
		case 4:
			d_type = 42;
			break;
		case 5:
			d_type = 46;
			break;
		case 6:
			d_type = 40;
			break;
		case 7:
			d_type = 43;
			break;
		case 8:
			d_type = 47;
			break;
		case 9:
			channel = channel / 2;
			break;
		default:
			printf("<c_BT_TX_Start>type is error!\n");
			break;
	}

	if(i_pkttype == 9)//ble
	{
        d_pattern = 0;
		sprintf(cmd, "autobt bletx --pattern 0x%x --channel %d", d_pattern, channel);
		//strcat(cmd, "autobt blerx");
	}
	else if(i_pkttype >= 0 && i_pkttype <= 8)
	{
		sprintf(cmd, "autobt tx --pattern 0x%x --hopping 00 --channel %d --type 0x%x --length %d", d_pattern, channel, d_type, i_pktcount);
	}

	//close wifi
	if(factory_system_call(__FUNCTION__, "echo 0 > /dev/wmtwifi") != 0)
	{
		PRINTERR;
		return TCASE_ERR;
	}

	if(factory_system_call(__FUNCTION__, cmd) != 0)
	{
		PRINTERR;
		return TCASE_ERR;
	}

	PRINTSUCC;
	return TCASE_OK;
}

int BT_TX_Stop(void)
{
	char cmd[100] = {0};
/*
	if(g_bt_tx_type == 9)
	{
		sprintf(cmd, "%s", "autobt stop bletx");
	}
	else if(g_bt_rx_type >= 0 && g_bt_rx_type <= 8)
	{
		sprintf(cmd, "%s", "autobt stop tx");
	}

	if(factory_system_call(__FUNCTION__, cmd) != 0)
	{
		PRINTERR;
		return TCASE_ERR;
	}
*/
	factory_system_call(__FUNCTION__, "exit");
#if 0
	sprintf(cmd, "%s", "autobt stop bletx");
	factory_system_call(__FUNCTION__, cmd);

	memset(cmd, 0x00, CMD_MAX_LENGTH);
	sprintf(cmd, "%s", "autobt stop tx");
	factory_system_call(__FUNCTION__, cmd);
#endif
	PRINTSUCC;
	return TCASE_OK;

}

int BT_RX_Start(char *freq, char *pkttype, char *payload, char *second_time)
{
	if(freq == NULL || pkttype == NULL || payload == NULL || second_time == NULL)
	{
		PRINTERR;
		printf("<error>arguments is null!\n");
		return TCASE_ERR;
	}

	char cmd[CMD_MAX_LENGTH] = {0};
	int d_type;
	int d_pattern = 3;// d_payload

	//kill btservice
	memset(cmd, "0x00", CMD_MAX_LENGTH);
	strcpy(cmd, "ps | grep /usr/bin/btservice | awk '{print $1}' | xargs kill");
	factory_system_call(__FUNCTION__, cmd);
	usleep(50*1000);
	factory_system_call(__FUNCTION__, cmd);

	int i_freq = atoi(freq);

	if(i_freq < 2402 || i_freq > 2480)
	{
		printf("<c_BT_RX_Start>i_freq error, freq is 2402 -- 2480\n");
		PRINTERR;
		return TCASE_ERR;
	}
	int i_pkttype = atoi(pkttype);
	int i_payload = atoi(payload);

	int channel = atoi(freq) - 2402;
	int d_second_time = atoi(second_time);

	switch(i_payload)
	{
		case 0:
			d_pattern = 3;
			break;
		case 1:
		case 2:
			d_pattern = 4;
			break;
		default:
			printf("<c_BT_RX_Strat>pattern is error, use default!\n");
			break;
	}

	switch(i_pkttype)
	{
		case 0:
			d_type = 4;
			break;
		case 1:
			d_type = 11;
			break;
		case 2:
			d_type = 15;
			break;
		case 3:
			d_type = 36;
			break;
		case 4:
			d_type = 42;
			break;
		case 5:
			d_type = 46;
			break;
		case 6:
			d_type = 40;
			break;
		case 7:
			d_type = 43;
			break;
		case 8:
			d_type = 47;
			break;
		case 9:
			channel = channel / 2;
			break;
		default:
			printf("<c_BT_RX_Strat>type is error!\n");
			break;
	}

	//g_bt_rx_type = i_pkttype;
	if(i_pkttype == 9)//ble
	{
		sprintf(cmd, "autobt blerx --channel %d --second %d", channel, d_second_time);
		//strcat(cmd, "autobt blerx");
	}
	else if(i_pkttype >= 0 && i_pkttype <= 8)
	{
		sprintf(cmd, "autobt nsrx --pattern 0x%x --channel %d --type 0x%x --addr 88C0FFEE --second %d", d_pattern, channel, d_type, d_second_time);
	}

	//close wifi
	if(factory_system_call(__FUNCTION__, "echo 0 > /dev/wmtwifi") != 0)
	{
		PRINTERR;
		return TCASE_ERR;
	}

	if(factory_system_call(__FUNCTION__, cmd) != 0)
	{
		PRINTERR;
		return TCASE_ERR;
	}

	PRINTSUCC;
	return TCASE_OK;
}

int BT_RX_Stop(void)
{
	BT_TX_Stop();
#if 0
	char cmd[100] = {0};

	if(g_bt_rx_type == 9)
	{
		sprintf(cmd, "%s", "autobt stop blerx");
	}
	else if(g_bt_rx_type >= 0 && g_bt_rx_type <= 8)
	{
		sprintf(cmd, "%s", "autobt stop nsrx");
	}

	if(factory_system_call(__FUNCTION__, cmd) != 0)
	{
		PRINTERR;
		return TCASE_ERR;
	}

	PRINTSUCC;
	return TCASE_OK;
#endif
}

int BT_Get_RxCount(void)
{
	char cmd[CMD_MAX_LENGTH] = {0};
	char tmp[300] = {0};
	FILE *fp = NULL;
	char *ch = NULL;
	int ok_packet = 0, total_packet = 0;
	int err_percent;

	sprintf(cmd, "%s", "autobt getrx > /tmp/bt_get_rxcount.txt");
	if(factory_system_call(__FUNCTION__, cmd) != 0)
	{
		printf("<c_BT_Get_RxCount>exec %s error!\n", cmd);
		return -1;
	}

	fp = fopen("/tmp/bt_get_rxcount.txt", "rb");
	if(fp == NULL)
	{
		printf("<c_BT_Get_RxCount>Open wifi_get_rxcount.txt error!\n");
		PRINTERR;
		return -1;
	}

	while(fgets(tmp, 300, fp) != 0)
	{
		ch = strstr(tmp, "Total received packet");
		if(ch != NULL)
		{
			ch += strlen("Total received packet:");
			if(ch != NULL)
			{
				total_packet= atoi(ch);
				printf("total_packet = %d\n", total_packet);
			}
		}

		ch = strstr(tmp, "Packet Error Rate");
		if(ch != NULL)
		{
			ch += strlen("Packet Error Rate:");
			strtok(ch, ".");
			if(ch != NULL)
			{
				err_percent = atoi(ch);
				ok_packet = total_packet - (total_packet*err_percent/100);
			}
		}

	}

	printf("Exe Count_T%d_G%d\n", total_packet, ok_packet);
	fclose(fp);
	return TCASE_OK;
}

/*int c_BT_Get_RxCount()
{
	return 0;
}*/
int BT_Write_FreqOffset(char *freqOffset)
{
	 WIFI_Set_FreqOffset(freqOffset);
	 return 0;
}
 int BT_Get_FreqOffset(void)
 {
	WIFI_Get_FreqOffset();
	return 0;
 }

 int BT_Set_FreqOffset(char *freqOffset)
 {
	WIFI_Set_FreqOffset(freqOffset);
	return 0;
 }
 
int WIFI_ATE_Mode(void)
{
	char cmd[CMD_MAX_LENGTH] = {0};

	memset(cmd, "0x00", CMD_MAX_LENGTH);
	strcpy(cmd, "ps | grep /usr/bin/appmainprog | awk '{print $1}' | xargs kill");
	factory_system_call(__FUNCTION__, cmd);
	usleep(50*1000);
	factory_system_call(__FUNCTION__, cmd);

	//stop elian
	memset(cmd, "0x00", CMD_MAX_LENGTH);
	strcpy(cmd, "iwpriv wlan0 elian stop");
	if(factory_system_call(__FUNCTION__, cmd) != 0)
	{
		PRINTERR;
		//return TCASE_ERR;
	}
	
	//open wifi
	memset(cmd, "0x00", CMD_MAX_LENGTH);
	strcpy(cmd, "echo 1 > /dev/wmtWifi");
	if(factory_system_call(__FUNCTION__, cmd) != 0)
	{
		PRINTERR;
		return TCASE_ERR;
	}

	sprintf(cmd, "%s", "iwpriv wlan0 set_test_mode 2011");
	printf("%s", (factory_system_call(__FUNCTION__, cmd) == TCASE_OK) ? CMD_EXEC_SUCC : CMD_EXEC_FAIL);

	return TCASE_OK;
}

int WIFI_TX_Start(char *freq, char *rate,
	char *bandwidth, char *pwr0, char *pwr1,
	char *pwr2, char *freqoffset, char *pktcount)
{
	if(freq == NULL || rate == NULL || bandwidth == NULL || pwr0 == NULL
		|| pwr1 == NULL || pwr2 == NULL || freqoffset == NULL || pktcount == NULL)
	{
		printf("<c_WIFI_TX_Start>argument is null!\n");
		PRINTERR;
		return TCASE_ERR;
	}

	char cmd[CMD_MAX_LENGTH] = {0};
	unsigned int d_rate, d_bandwidth, d_preamble, d_pktcount, d_gi;
	int i_freq = atoi(freq);
	int i_rate = (int)(atof(rate) * 10);
	int i_bandwidth = 0;
	int i_pwr0 = (int)(atof(pwr0) * 2.0 + 0.5);
	int i_pktcount = atoi(pktcount);
	int i_80211ac = 0;

	if(strcmp(bandwidth, "20") == 0)
		i_bandwidth = 20;
	else if(strcmp(bandwidth, "40") == 0)
		i_bandwidth = 40;
	else if(strcmp(bandwidth, "VHT20") == 0)
	{
		i_bandwidth = 20;
		i_80211ac = 1;
	}
	else if(strcmp(bandwidth, "VHT40") == 0)
	{
		i_bandwidth = 40;
		i_80211ac = 1;
	}
	else if(strcmp(bandwidth, "VHT80") == 0)
	{
		i_bandwidth = 80;
		i_80211ac = 1;
	}
	else
		i_bandwidth = 0;

	//set frequency
	sprintf(cmd, "iwpriv wlan0 driver \"set_test_cmd 18 %d\"", i_freq*1000);
	if(factory_system_call(__FUNCTION__, cmd) != TCASE_OK)
	{
		PRINTERR;
		return TCASE_ERR;
	}
#if 0
	//set Rate
	switch(i_rate)
	{
		case 1:
			d_rate = 0;
			break;
		case 2:
			d_rate = 1;
			break;
		case 5:
			d_rate = 2;
			break;
		case 11:
			d_rate = 3;
			break;
		case 6:
			d_rate = 5;
			break;
		case 12:
			d_rate = 6;
			break;
		case 18:
			d_rate = 7;
			break;
		case 24:
			d_rate = 8;
			break;
		case 36:
			d_rate = 9;
			break;
		case 48:
			d_rate = 10;
			break;
		case 54:
			d_rate = 11;
			break;
		case 65:
			d_rate = 0x80000007;
			break;
		case 135:
			d_rate = 0x80000007;
            break;
		default:
			printf("<c_WIFI_TX_Start>no correct rate, i_rate = %d !\n", i_rate);
			break;
	}
#endif

        d_gi = 0; /*default gi is normal 800ns*/
        //set Rate
        switch(i_rate)
        {
            case 10:
                d_rate = 0;
                break;
            case 20:
                d_rate = 1;
                break;
            case 50:
                d_rate = 2;
                break;
            case 110:
                d_rate = 3;
                break;
            case 60:
                d_rate = 4;
                break;
            case 90:
                d_rate = 5;
                break;
            case 120:
                d_rate = 6;
                break;
            case 180:
                d_rate = 7;
                break;
            case 240:
                d_rate = 8;
                break;
            case 360:
                d_rate = 9;
                break;
            case 480:
                d_rate = 10;
                break;
            case 540:
                d_rate = 11;
                break;
            #if 0
            case 650:
                d_rate = 0x80000007;
                break;
            case 1350:
                d_rate = 0x80000007;
                break;
            #endif
            /*add for 802.11n/ac gi normal*/
            case 65:
            case 135:
            case 293:
                d_rate = 0x80000000;
                d_gi = 0;
                break;
            case 130:
            case 270:
            case 586: /* VHT80 MCS1 LGI 58.5M */
                d_rate = 0x80000001;
                d_gi = 0;
                break;

            case 195:
            case 405:
            case 878:
                d_rate = 0x80000002;
                d_gi = 0;
                break;
            case 260:
            case 541:
            case 1170:
                d_rate = 0x80000003;
                d_gi = 0;
                break;
            case 390:
            case 810:
            case 1755:
                d_rate = 0x80000004;
                d_gi = 0;
                break;
            case 520:
            case 1080:
            case 2340:
                d_rate = 0x80000005;
                d_gi = 0;
                break;

            case 585: /* HT20 MCS6 LGI 58.5M */
            case 1215:
            case 2633:
                d_rate = 0x80000006;
                d_gi = 0;
                break;
            case 650: /* HT20 MCS7 LGI 65M */
            case 1350:
            case 2925:
                d_rate = 0x80000007;
                d_gi = 0;
                break;
            case 780:
            case 1620:
            case 3510:
                d_rate = 0x80000008;
                d_gi = 0;
                break;
            case 1800:
            case 3900:
                d_rate = 0x80000009;
                d_gi = 0;
                break;
            /*add for 802.11n/ac gi short*/
            case 72:
            case 150:
            case 325:
                d_rate = 0x80000000;
                d_gi = 1;
                break;
            case 144:
            case 300:
            case 652: /* VHT80 MCS1 SGI 65M */
                d_rate = 0x80000001;
                d_gi = 1;
                break;

            case 217:
            case 450:
            case 975:
                d_rate = 0x80000002;
                d_gi = 1;
                break;
            case 289:
            case 600:
            case 1300:
                d_rate = 0x80000003;
                d_gi = 1;
                break;
            case 433:
            case 900:
            case 1950:
                d_rate = 0x80000004;
                d_gi = 1;
                break;
            case 578:
            case 1200:
            case 2600:
                d_rate = 0x80000005;
                d_gi = 1;
                break;

            case 651: /* HT20 MCS6 SGI 65M */
            case 1351:
            case 2926:
                d_rate = 0x80000006;
                d_gi = 1;
                break;
            case 722:
            case 1500:
            case 3250:
                d_rate = 0x80000007;
                d_gi = 1;
                break;
            case 867:
            case 1801:
            case 3901:
                d_rate = 0x80000008;
                d_gi = 1;
                break;
            case 2000:
            case 4333:
                d_rate = 0x80000009;
                d_gi = 1;
                break;

            default:
                printf("<c_WIFI_TX_Start>no correct rate, i_rate = %d !\n", i_rate);
                break;
        }


	if ((d_rate <= 3) && (d_rate >= 0))
		d_preamble = 1;
	else if ((d_rate <= 11) && (d_rate >= 4))
		d_preamble = 0;
	else if ((d_rate <= 0x80000007) && (d_rate >= 0x80000000))
		d_preamble = 3;
	else
		d_preamble = 2;

	if(i_80211ac == 1)
		d_preamble = 4;

	/*set preamble 0:normal*/
	memset(cmd, 0x00, 100);
	sprintf(cmd, "iwpriv wlan0 driver \"set_test_cmd 4 %d\"", d_preamble);
	if(factory_system_call(__FUNCTION__, cmd) != 0)
	{
		PRINTERR;
		return TCASE_ERR;
	}

	/*set gi 0:normal 1: short*/
	memset(cmd, 0x00, 100);
	sprintf(cmd, "iwpriv wlan0 driver \"set_test_cmd 16 %d\"", d_gi);
	if(factory_system_call(__FUNCTION__, cmd) != 0)
	{
		PRINTERR;
		return TCASE_ERR;
	}

	memset(cmd, 0x00, 100);
	sprintf(cmd, "iwpriv wlan0 driver \"set_test_cmd 3 0x%x\"", d_rate);
	if(factory_system_call(__FUNCTION__, cmd) != 0)
	{
		PRINTERR;
		return -1;
	}

	//set channel BW

	switch(i_bandwidth)
	{
		case 20:
			d_bandwidth = 0;
			break;
		case 40:
			d_bandwidth = 1;
			break;
		case 80:
			d_bandwidth = 2;
			break;
		default:
			printf("<c_WIFI_TX_Start>no correct bandwidth, only support 20M 40M 80M, now is %s !\n", bandwidth);
			break;
	}

	/* for lagecy or 802.11n set bandwith */
	if(i_80211ac == 0)
	{
		memset(cmd, 0x00, 100);
		sprintf(cmd, "iwpriv wlan0 driver \"set_test_cmd 15 %d\"", d_bandwidth);
		if(factory_system_call(__FUNCTION__, cmd) != 0)
		{
			PRINTERR;
			return TCASE_ERR;
		}
	}

	/* for 802.11ac set bandwith */
	if(i_80211ac == 1)
	{
		memset(cmd, 0x00, 100);
		sprintf(cmd, "iwpriv wlan0 driver \"set_test_cmd 71 %d\"", d_bandwidth);
		if(factory_system_call(__FUNCTION__, cmd) != 0)
		{
			PRINTERR;
			return TCASE_ERR;
		}

		memset(cmd, 0x00, 100);
		sprintf(cmd, "iwpriv wlan0 driver \"set_test_cmd 72 %d\"", d_bandwidth);
		if(factory_system_call(__FUNCTION__, cmd) != 0)
		{
			PRINTERR;
			return TCASE_ERR;
		}

		memset(cmd, 0x00, 100);
		sprintf(cmd, "iwpriv wlan0 driver \"set_test_cmd 73 0\"");
		if(factory_system_call(__FUNCTION__, cmd) != 0)
		{
			PRINTERR;
			return TCASE_ERR;
		}
	}

	//set power unit
	memset(cmd, 0x00, 100);
	strcpy(cmd, "iwpriv wlan0 driver \"set_test_cmd 31 0\"");
	if(factory_system_call(__FUNCTION__, cmd) != 0)
	{
		PRINTERR;
		return TCASE_ERR;
	}

	memset(cmd, 0x00, 100);
	sprintf(cmd, "iwpriv wlan0 driver \"set_test_cmd 2 %d\"", i_pwr0);
	if(factory_system_call(__FUNCTION__, cmd) != 0)
	{
		PRINTERR;
		return TCASE_ERR;
	}

	//set pktcount
	if(i_pktcount < 0 || i_pktcount > 0xffffff)
	{
		printf("<c_WIFI_TX_Start>pkcount error, 0 -- 0xffffff!\n");
		PRINTERR;
		return TCASE_ERR;
	}
	memset(cmd, 0x00, 100);
	sprintf(cmd, "iwpriv wlan0 driver \"set_test_cmd 7 0x%x\"", i_pktcount);
	if(factory_system_call(__FUNCTION__, cmd) != 0)
	{
		PRINTERR;
		return TCASE_ERR;
	}

	//exec
	memset(cmd, 0x00, 100);
	strcpy(cmd, "iwpriv wlan0 driver \"set_test_cmd 1 1\"");
	if(factory_system_call(__FUNCTION__, cmd) != 0)
	{
		PRINTERR;
		return TCASE_ERR;
	}

	PRINTSUCC;
	return TCASE_OK;
}
int WIFI_TX_Stop(void)
{
	char cmd[100] = {0};

	strcpy(cmd, "iwpriv wlan0 driver \"set_test_cmd 1 0\"");

	if(factory_system_call(__FUNCTION__, cmd) != TCASE_OK)
	{
		PRINTERR;
		return TCASE_ERR;
	}

	PRINTSUCC;
	return TCASE_OK;
}
int WIFI_RX_Start(char *freq, char *rate, char *bandwidth)
{
	if(freq == NULL || rate == NULL || bandwidth == NULL)
	{
		printf("<c_WIFI_RX_Start>argument is null!\n");
		PRINTERR;
		return TCASE_ERR;
	}

	char cmd[100] = {0};
	int d_bandwidth = 0, d_rate = 0, d_preamble = 0;
	int i_freq = atoi(freq);
	int i_rate = atoi(rate);
	int i_bandwidth = 0;
	int i_80211ac = 0;

	if(strcmp(bandwidth, "20") == 0)
		i_bandwidth = 20;
	else if(strcmp(bandwidth, "40") == 0)
		i_bandwidth = 40;
	else if(strcmp(bandwidth, "VHT20") == 0)
	{
		i_bandwidth = 20;
		i_80211ac = 1;
	}
	else if(strcmp(bandwidth, "VHT40") == 0)
	{
		i_bandwidth = 40;
		i_80211ac = 1;
	}
	else if(strcmp(bandwidth, "VHT80") == 0)
	{
		i_bandwidth = 80;
		i_80211ac = 1;
	}
	else
		i_bandwidth = 0;

	//set frequency
	sprintf(cmd, "iwpriv wlan0 driver \"set_test_cmd 18 %d\"", i_freq*1000);
	if(factory_system_call(__FUNCTION__, cmd) != 0)
	{
		PRINTERR;
		return -1;
	}

	//set bandwidth

	//set channel BW
	switch(i_bandwidth)
	{
		case 20:
			d_bandwidth = 0;
			break;
		case 40:
			d_bandwidth = 1;
			break;
		case 80:
			d_bandwidth = 2;
			break;
		default:
			printf("<c_WIFI_RX_Start>no correct bandwidth, only support 20M 40M 80M, now is %s !\n", bandwidth);
			break;
	}

	/* for lagecy or 802.11n set bandwith */
	if(i_80211ac == 0)
	{
		memset(cmd, 0x00, 100);
		sprintf(cmd, "iwpriv wlan0 driver \"set_test_cmd 15 %d\"", d_bandwidth);
		if(factory_system_call(__FUNCTION__, cmd) != 0)
		{
			PRINTERR;
			return TCASE_ERR;
		}
	}

	/* for 802.11ac set bandwith */
	if(i_80211ac == 1)
	{
		memset(cmd, 0x00, 100);
		sprintf(cmd, "iwpriv wlan0 driver \"set_test_cmd 71 %d\"", d_bandwidth);
		if(factory_system_call(__FUNCTION__, cmd) != 0)
		{
			PRINTERR;
			return TCASE_ERR;
		}

		memset(cmd, 0x00, 100);
		sprintf(cmd, "iwpriv wlan0 driver \"set_test_cmd 72 %d\"", d_bandwidth);
		if(factory_system_call(__FUNCTION__, cmd) != 0)
		{
			PRINTERR;
			return TCASE_ERR;
		}

		memset(cmd, 0x00, 100);
		sprintf(cmd, "iwpriv wlan0 driver \"set_test_cmd 73 0\"");
		if(factory_system_call(__FUNCTION__, cmd) != 0)
		{
			PRINTERR;
			return TCASE_ERR;
		}
	}

	//set Rate
	switch(i_rate)
	{
		case 1:
			d_rate = 0;
			break;
		case 2:
			d_rate = 1;
			break;
		case 5:
			d_rate = 2;
			break;
		case 11:
			d_rate = 3;
			break;
		case 6:
			d_rate = 5;
			break;
		case 12:
			d_rate = 6;
			break;
		case 18:
			d_rate = 7;
			break;
		case 24:
			d_rate = 8;
			break;
		case 36:
			d_rate = 9;
			break;
		case 48:
			d_rate = 10;
			break;
		case 54:
			d_rate = 11;
			break;
		case 65:
			d_rate = 0x80000007;
			break;
		default:
			printf("<c_WIFI_RX_Start>no correct rate, i_rate = %d !\n", i_rate);
			break;
	}
	if(d_rate == 0 || d_rate == 1 || d_rate == 2 || d_rate == 3)
		d_preamble = 1;
	else if(d_rate == 5 || d_rate == 6 || d_rate == 7 || d_rate == 8 || d_rate == 9 || d_rate == 10 || d_rate == 11)
		d_preamble = 0;
	else if(d_rate == 0x80000007)
		d_preamble = 3;
	else
		d_preamble = 2;

	if(i_80211ac == 1)
		d_preamble = 4;

	/*set preamble 0:normal*/
	memset(cmd, 0x00, 100);
	sprintf(cmd, "iwpriv wlan0 driver \"set_test_cmd 4 %d\"", d_preamble);
	if(factory_system_call(__FUNCTION__, cmd) != 0)
	{
		PRINTERR;
		return -1;
	}

	memset(cmd, 0x00, 100);
	sprintf(cmd, "iwpriv wlan0 driver \"set_test_cmd 3 0x%x\"", d_rate);
	if(factory_system_call(__FUNCTION__, cmd) != 0)
	{
		PRINTERR;
		return -1;
	}

	//before start Rx, get how many packet have received

	//end before start rx
	//start trigger Rx
	memset(cmd, 0x00, 100);
	strcpy(cmd, "iwpriv wlan0 driver \"set_test_cmd 1 2\"");
	if(factory_system_call(__FUNCTION__, cmd) != 0)
	{
		PRINTERR;
		return TCASE_ERR;
	}

	PRINTSUCC;
	return TCASE_OK;
}
int WIFI_Get_RxCount(void)
{
	char cmd[100] = {0};
	FILE *fp = NULL;
	char *ch = NULL;
	int count = 0;

	//RXOK_count
	strcpy(cmd, "iwpriv wlan0 get_test_result 34 0 > /tmp/wifi_get_rxcount.txt");
	if(factory_system_call(__FUNCTION__, cmd) != 0)
	{
		PRINTERR;
		return -1;
	}

	//RXERR_count
	/*printf("<c_Wifi_Get_RxCount> ERROR count:\n");
	strcpy(cmd, "iwpriv wlan0 get_test_result 35 0");
	if(factory_system_call(__FUNCTION__, cmd) != 0)
	{
		printf("<c_Wifi_Get_RxCount>exec %s error!\n", cmd);
		return -1;
	}*/

	fp = fopen("/tmp/wifi_get_rxcount.txt", "rb");
	if(fp == NULL)
	{
		printf("<c_Wifi_Get_RxCount>Open wifi_get_rxcount.txt error!\n");
		PRINTERR;
		return -1;
	}

	fread(cmd, 1, CMD_MAX_LENGTH-1, fp);
	{
		ch = strstr(cmd, "get_test_result");
		if(ch != NULL)
		{
			ch += strlen("get_test_result:");
			if(ch != NULL)
			{
				count = atoi(ch);
				printf("Exe Count_%d\n", count);
				fclose(fp);
				return TCASE_OK;
			}
		}
	}

	fclose(fp);
	return TCASE_ERR;
}
int WIFI_RX_Stop(void)
{
	char cmd[100];

	strcpy(cmd, "iwpriv wlan0 driver \"set_test_cmd 1 0\"");
	if(factory_system_call(__FUNCTION__, cmd) != 0)
	{
		PRINTERR;
		return -1;
	}

	PRINTSUCC;
	return TCASE_OK;
}
int WIFI_Write_FreqOffset(char *freqOffset)
{
	if(freqOffset == NULL)
	{
		PRINTERR;
		return TCASE_ERR;
	}

	char cmd[CMD_MAX_LENGTH] = {0};
	int i_freqOffset = atoi(freqOffset);
	int d_freqOffset;

	if(i_freqOffset < 0 && i_freqOffset >= -63)
	{
		d_freqOffset = 64 - i_freqOffset;
		//sprintf(cmd, "echo -%d > /sys/devices/platform/soc/10210000.xo/xo_board_offset", d_freqOffset);
	}
	else if(i_freqOffset > 0 && i_freqOffset <= 63)
	{
		d_freqOffset = i_freqOffset;
	}
	else
	{
		printf("<c_WIFI_Set_FreqOffset> freqOffset is error!\n");
		PRINTERR;
		return TCASE_ERR;
	}

	sprintf(cmd, "echo 0x%x > /sys/devices/platform/soc/10210000.xo/xo_nvram_board_offset", d_freqOffset);
	if(factory_system_call(__FUNCTION__, cmd) != 0)
	{
		PRINTERR;
		return TCASE_ERR;
	}

	memset(cmd, "0x00", CMD_MAX_LENGTH);
	strcpy(cmd, "nvram_save save");
	if(factory_system_call(__FUNCTION__, cmd) != 0)
	{
		PRINTERR;
		return TCASE_ERR;
	}

	PRINTSUCC;
	return TCASE_OK;
}
int WIFI_Get_FreqOffset(void)
{
	char cmd[100];

	printf("<c_WIFI_Get_FreqOffset>freqOffset :\n");
	strcpy(cmd, "cat /sys/devices/platform/soc/10210000.xo/xo_nvram_board_offset");
	if(factory_system_call(__FUNCTION__, cmd) != 0)
	{
		printf("<c_WIFI_Get_FreqOffset>exec %s error!\n", cmd);
		return -1;
	}

	return TCASE_OK;
}

static int get_wifi_module_type(void)
{
	int module = 0;
	char *ch = NULL;
	char cmd[CMD_MAX_LENGTH] = {0};
	FILE *fp = NULL;

	sprintf(cmd, "iwpriv wlan0 set_mcr 2011 2011");
	if(factory_system_call(__FUNCTION__, cmd) != 0)
	{
		PRINTERR;
		return TCASE_ERR;
	}
		
	sprintf(cmd, "%s", "iwpriv wlan0 get_mcr 0 > /tmp/get_wifi_module.txt");
	if(factory_system_call(__FUNCTION__, cmd) != 0)
	{
		PRINTERR;
		return TCASE_ERR;
	}
		
	fp = fopen("/tmp/get_wifi_module.txt", "rb");
	if(fp == NULL)
	{
		printf("<WIFI_Write_PwrOffset>Open get_wifi_module.txt error!\n");
		PRINTERR;
		return -1;
	}

	memset(cmd, 0x00, CMD_MAX_LENGTH);
	while(fgets(cmd, CMD_MAX_LENGTH, fp) != 0)
	{
		ch = strstr(cmd, "get_mcr");
		if(ch != NULL)
		{
			ch += strlen("get_mcr:");
			if(ch != NULL)
			{
				module= atoi(ch);
				printf("module = 0x%x\n", module);
			}
		}	
	}

	return module;
}

int WIFI_Write_PwrOffset(char *l_offset, char *m_offset, char *h_offset, char *all_offset)
{
	if(l_offset == NULL || m_offset == NULL || h_offset == NULL)
	{
		printf("<c_WIFI_Write_PwrOffset>argument is null!\n");
		return TCASE_ERR;
	}
	int i_l_offset = atoi(l_offset);
	int i_m_offset = atoi(m_offset);
	int i_h_offset = atoi(h_offset);

	int d_l_offset, d_m_offset, d_h_offset;
	char cmd[100] = {0};
	int module = 0;
	//to do
	//what is the limits?

	//6630 or 6627
	module = get_wifi_module_type();

	if(module && ((module & 0xffff) == 0x6630))
	{
		//6630
		if(i_l_offset > 16 || i_l_offset < -16 || i_m_offset > 16 || i_m_offset < -16 || i_h_offset > 16 || i_h_offset < -16)
		{
			PRINTERR;
			printf("Error! parameter should:  -16  --  16\n");
			return TCASE_ERR;
		}
		
		if(i_l_offset >= 0)
		{
			d_l_offset = i_l_offset*8;
		}
		else if(i_l_offset < 0)
		{
			d_l_offset = 256 + (i_l_offset*8);
		}

		if(i_m_offset >= 0)
		{
			d_m_offset = i_m_offset*8;
		}
		else if(i_m_offset < 0)
		{
			d_m_offset = 256 + (i_m_offset*8);
		}

		if(i_h_offset >= 0)
		{
			d_h_offset = i_h_offset*8;
		}
		else if(i_h_offset < 0)
		{
			d_h_offset = 256 + (i_h_offset*8);
		}
		
		if(i_l_offset == 16)
			d_l_offset = 127;
		if(i_m_offset == 16)
			d_m_offset = 127;
		if(i_h_offset == 16)
			d_h_offset = 127;
	}
	else if(module)
	{
		//6627
		if(i_l_offset > 7 || i_l_offset < -7 || i_m_offset > 7 || i_m_offset < -7 || i_h_offset > 7 || i_h_offset < -7)
		{
			PRINTERR;
			printf("Error! parameter should:  -7  --  7\n");
			return TCASE_ERR;
		}

		if(i_l_offset >= 0)
		{
			d_l_offset = i_l_offset*18;
		}
		else if(i_l_offset < 0)
		{
			d_l_offset = 256 + (i_l_offset*18);
		}

		if(i_m_offset >= 0)
		{
			d_m_offset = i_m_offset*18;
		}
		else if(i_m_offset < 0)
		{
			d_m_offset = 256 + (i_m_offset*18);
		}

		if(i_h_offset >= 0)
		{
			d_h_offset = i_h_offset*18;
		}
		else if(i_h_offset < 0)
		{
			d_h_offset = 256 + (i_h_offset*18);
		}
		
	}
	else
	{
		printf("Get wifi module error!\n");
		return TCASE_ERR;
	}

	sprintf(cmd, "iwpriv wlan0 driver \"set_pwr_offset %d %d %d 0\"", d_l_offset, d_m_offset, d_h_offset);
	if(factory_system_call(__FUNCTION__, cmd) != 0)
	{
		PRINTERR;
		return TCASE_ERR;
	}

	//save nvram
	memset(cmd, "0x00", CMD_MAX_LENGTH);
	strcpy(cmd, "nvram_save save");
	if(factory_system_call(__FUNCTION__, cmd) != 0)
	{
		PRINTERR;
		return TCASE_ERR;
	}

	//restart wifi
	memset(cmd, 0x00, CMD_MAX_LENGTH);
	strcpy(cmd, "echo 0 > /dev/wmtWifi");
	if(factory_system_call(__FUNCTION__, cmd) != 0)
	{
		PRINTERR;
		return -1;
	}

	memset(cmd, 0x00, CMD_MAX_LENGTH);
	strcpy(cmd, "echo 1 > /dev/wmtWifi");
	if(factory_system_call(__FUNCTION__, cmd) != 0)
	{
		PRINTERR;
		return -1;
	}

	PRINTSUCC;
	return TCASE_OK;
}
int WIFI_Get_PwrOffset(void)
{
	char cmd[CMD_MAX_LENGTH];
	int module = 0;
	FILE *fp = NULL;
	char *ch = NULL;
	int h_offset, m_offset, l_offset, all_offset;
	int d_h_offset, d_m_offset, d_l_offset, d_all_offset;
	
	module = get_wifi_module_type();
	//strcpy(cmd, "iwpriv wlan0 driver \"get_pwr_offset\" >/tmp/get_pwr.txt");
	strcpy(cmd, "iwpriv wlan0 driver \"get_pwr_offset\" > /tmp/get_power.txt");
	if(factory_system_call(__FUNCTION__, cmd) != 0)
	{
		printf("<c_WIFI_Get_FreqOffset>exec %s error!\n", cmd);
		return -1;
	}

	fp = fopen("/tmp/get_power.txt", "rb");
	if(fp == NULL)
	{
		printf("Open /tmp/get_power.txt error!\n");	
		return TCASE_ERR;
	}

	memset(cmd, 0x00, CMD_MAX_LENGTH);
	while(fgets(cmd, CMD_MAX_LENGTH, fp) != 0)
	{
		ch = strstr(cmd, "L=");
		if(ch != NULL)
		{
			ch += strlen("L=");
			if(ch != NULL)
			{
				d_l_offset= atoi(ch);
			}
		}	

		ch = strstr(cmd, "M=");
		if(ch != NULL)
		{
			ch += strlen("M=");
			if(ch != NULL)
			{
				d_m_offset= atoi(ch);
			}
		}	

		ch = strstr(cmd, "H=");
		if(ch != NULL)
		{
			ch += strlen("H=");
			if(ch != NULL)
			{
				d_h_offset= atoi(ch);
			}
		}	

		ch = strstr(cmd, "ALL=");
		if(ch != NULL)
		{
			ch += strlen("ALL=");
			if(ch != NULL)
			{
				d_all_offset= atoi(ch);
			}
		}	
	}

	if(module && ((module & 0xffff) == 0x6630))
	{
		//6630
		if(d_l_offset > 127)
			l_offset = d_l_offset/8-32;
		else
			l_offset = d_l_offset/8;
		
		if(d_m_offset > 127)
			m_offset = d_m_offset/8-32;
		else
			m_offset = d_m_offset/8;

		if(d_h_offset > 127)
			h_offset = d_h_offset/8-32;
		else
			h_offset = d_h_offset/8;

		if(d_l_offset == 127)
			l_offset = 16;
		if(d_m_offset == 127)
			m_offset = 16;
		if(d_h_offset == 127)
			h_offset = 16;
	}
	else if(module)
	{
		//6627
		if(d_l_offset > 127)
			l_offset = (d_l_offset-256)/18;
		else
			l_offset = d_l_offset/18;
		
		if(d_m_offset > 127)
			m_offset = (d_m_offset-256)/18;
		else
			m_offset = d_m_offset/18;

		if(d_h_offset > 127)
			h_offset = (d_h_offset-256)/18;
		else
			h_offset = d_h_offset/18;
	}
	else
	{
		printf("Get wifi module error!\n");
		return TCASE_ERR;
	}

	printf("L = %d, M = %d, H = %d \n", l_offset, m_offset, h_offset);
	
	PRINTSUCC;
	return TCASE_OK;
}
int WIFI_Set_FreqOffset(char *freqOffset)
{
	if(freqOffset == NULL)
	{
		PRINTERR;
		return TCASE_ERR;
	}

	char cmd[CMD_MAX_LENGTH] = {0};
	int i_freqOffset = atoi(freqOffset);
	int d_freqOffset;

	if(i_freqOffset < 0 && i_freqOffset >= -63)
	{
		d_freqOffset = 64 - i_freqOffset;
		//sprintf(cmd, "echo -%d > /sys/devices/platform/soc/10210000.xo/xo_board_offset", d_freqOffset);
	}
	else if(i_freqOffset >= 0 && i_freqOffset <= 63)
	{
		d_freqOffset = i_freqOffset;
	}
	else
	{
		printf("<c_WIFI_Set_FreqOffset> freqOffset is error!\n");
		PRINTERR;
		return TCASE_ERR;
	}

	sprintf(cmd, "echo 0x%x > /sys/devices/platform/soc/10210000.xo/xo_board_offset", d_freqOffset);
	if(factory_system_call(__FUNCTION__, cmd) != 0)
	{
		PRINTERR;
		return TCASE_ERR;
	}

	PRINTSUCC;
	return TCASE_OK;
}
int WIFI_Set_PwrOffset(char *l_offset, char *m_offset, char *h_offset, char *all_offset)
{
	WIFI_Write_PwrOffset(l_offset, m_offset, h_offset, NULL);
}

int get_5G_driver_offset(int aa)
{
	int bb;
	if(aa > 16 || aa < -16 )
	{
		PRINTERR;
		printf("Error! all parameters should:  -16  --  16\n");
		return (int)TCASE_ERR;
	}

	if(aa >= 0)
	{
		bb = aa*8;
	}
	else if(aa < 0)
	{
		bb = 256 + (aa*8);
	}

	if(aa == 16)
		bb = 127;
	
	return bb;
}

int get_5G_user_offset(int aa)
{
	int bb;
	if(aa > 256 || aa < 0 )
	{
		PRINTERR;
		printf("Error! all parameters should:  -16  --  16\n");
		return (int)TCASE_ERR;
	}

	if(aa > 127)
		bb = aa/8-32;
	else
		bb = aa/8;

	if(aa == 127)
		bb = 16;
	
	return bb;
}


int WIFI_Write_PwrOffset_5G(char *first_offset, char *second_offset, char *third_offset, char *forth_offset,
									char *fifth_offset, char *sixth_offset, char *seventh_offset, char *eighth_offset)
{
	if(first_offset == NULL || second_offset == NULL || third_offset == NULL || forth_offset == NULL ||
		fifth_offset == NULL || sixth_offset == NULL || seventh_offset == NULL || eighth_offset == NULL)
	{
		printf("<c_WIFI_Write_PwrOffset>argument is null! 5G module is 8 groups!\n");
		return TCASE_ERR;
	}
	
	int d_first_offset, d_second_offset, d_third_offset, d_forth_offset;
	int d_fifth_offset, d_sixth_offset, d_seventh_offset, d_eighth_offset;
	
	char cmd[200] = {0};
	int module = 0;
	
	//6630 or 6627
	module = get_wifi_module_type();
	if(module && ((module & 0xffff) == 0x6630))
	{
		//6630
		d_first_offset = get_5G_driver_offset(atoi(first_offset));
		d_second_offset = get_5G_driver_offset(atoi(second_offset));
		d_third_offset = get_5G_driver_offset(atoi(third_offset));
		d_forth_offset = get_5G_driver_offset(atoi(forth_offset));
		d_fifth_offset = get_5G_driver_offset(atoi(fifth_offset));
		d_sixth_offset = get_5G_driver_offset(atoi(sixth_offset));
		d_seventh_offset = get_5G_driver_offset(atoi(seventh_offset));
		d_eighth_offset = get_5G_driver_offset(atoi(eighth_offset));
	}
	else if(module)
	{
		//6627
		printf("5G module id only for 6630!\n");
		return TCASE_ERR;
	}
	else
	{
		printf("Get wifi module error!\n");
		return TCASE_ERR;
	}
	
	sprintf(cmd, "iwpriv wlan0 driver \"SET_5G_PWR_OFFSET %d %d %d %d %d %d %d %d\"", 
		d_first_offset, d_second_offset, d_third_offset, d_forth_offset, 
		d_fifth_offset, d_sixth_offset, d_seventh_offset, d_eighth_offset);
	if(factory_system_call(__FUNCTION__, cmd) != 0)
	{
		PRINTERR;
		return TCASE_ERR;
	}
	
	//save nvram
	memset(cmd, "0x00", CMD_MAX_LENGTH);
	strcpy(cmd, "nvram_save save");
	if(factory_system_call(__FUNCTION__, cmd) != 0)
	{
		PRINTERR;
		return TCASE_ERR;
	}
	
	//restart wifi
	memset(cmd, 0x00, CMD_MAX_LENGTH);
	strcpy(cmd, "echo 0 > /dev/wmtWifi");
	if(factory_system_call(__FUNCTION__, cmd) != 0)
	{
		PRINTERR;
		return -1;
	}
	
	memset(cmd, 0x00, CMD_MAX_LENGTH);
	strcpy(cmd, "echo 1 > /dev/wmtWifi");
	if(factory_system_call(__FUNCTION__, cmd) != 0)
	{
		PRINTERR;
		return -1;
	}
	
	PRINTSUCC;
	return TCASE_OK;
}


int WIFI_Get_PwrOffset_5G(void)
{
	char cmd[200];
	int num[20] = {0};
	int module = 0, i=0;
	FILE *fp = NULL;
	char *p = NULL;
	//int h_offset, m_offset, l_offset, all_offset;
	int d_first_offset, d_second_offset, d_third_offset, d_forth_offset;
	int d_fifth_offset, d_sixth_offset, d_seventh_offset, d_eighth_offset;
	
	module = get_wifi_module_type();
	//strcpy(cmd, "iwpriv wlan0 driver \"get_pwr_offset\" >/tmp/get_pwr.txt");
	strcpy(cmd, "iwpriv wlan0 driver \"GET_5G_PWR_OFFSET\" > /tmp/get_power_5G.txt");
	if(factory_system_call(__FUNCTION__, cmd) != 0)
	{
		printf("<c_WIFI_Get_FreqOffset>exec %s error!\n", cmd);
		return -1;
	}

	fp = fopen("/tmp/get_power_5G.txt", "rb");
	if(fp == NULL)
	{
		printf("Open /tmp/get_power_5G.txt error!\n");	
		return TCASE_ERR;
	}

	memset(cmd, 0x00, 200);
	while(fgets(cmd, 200, fp) != 0)
	{
		p = cmd;
		while(*p != '=' && *p != NULL)
			p++;
		
		while(*p != NULL && *p != 'V')
		{
			if((*p >= '0') && (*p <= '9'))
			{
				num[i] = num[i]*10 + (*p-'0');
			}
			else if(*p == ',')
			{
				i++;
			}
			p++;
		}
		memset(cmd, 0x00, 200);
	}

	if(module && ((module & 0xffff) == 0x6630))
	{
		//6630

		for(i = 0; i < 8; i++)
			num[i] = get_5G_user_offset(num[i]);
	}
	else if(module)
	{
		//6627
		printf("5G module id only for 6630!\n");
		return TCASE_ERR;
	}
	else
	{
		printf("Get wifi module error!\n");
		return TCASE_ERR;
	}

	printf("Value = ");
	for(i = 0; i < 8; i++)
	{
		printf("%d", num[i]);
		if(i != 7)
			printf(",");
	}
	printf("\n");
	
	PRINTSUCC;
	return TCASE_OK;
}

int WIFI_Set_PwrOffset_5G(char *first_offset, char *second_offset, char *third_offset, char *forth_offset,
									char *fifth_offset, char *sixth_offset, char *seventh_offset, char *eighth_offset)
{
	WIFI_Write_PwrOffset_5G(first_offset, second_offset, third_offset, forth_offset, 
							   fifth_offset, sixth_offset, seventh_offset, eighth_offset);
}
