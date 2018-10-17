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
#include <platform/mtk_i2c.h>
#include <platform/mtk_timer.h>

#define FL3236_SHUTDOWN_REG     0x00
#define FL3236_PWM_REG          0x01
#define FL3236_UPDATE_REG       0x25
#define FL3236_LED_REG          0x26
#define FL3236_CONTROL_REG      0x4a
#define FL3236_FREQUENCY_REG    0x4b
#define FL3236_RESET_REG        0x4f

#define SHUTDOWN_REG_NORMAL      0x01
#define SHUTDOWN_REG_SHUTDOWN    0x00
#define UPDATE_REG_UPDATE        0x00
#define CONTROL_REG_SHUTDOWN_ALL 0x01
#define CONTROL_REG_WORK_ALL     0x00
#define FREQUENCY_REG_3KHZ       0x00
#define FREQUENCY_REG_22KHZ      0x01
#define RESET_REG_RESET          0x00

#define FL3236_SPEED             22 //22kHz
#define FL3236_BUS_NUM           1
#define FL3236_DEV_ADDR          0x3f

u8 write_data[2];

static int fl3236_led_init(void)
{
	write_data[0] = FL3236_SHUTDOWN_REG;
	write_data[1] = SHUTDOWN_REG_NORMAL;
	mtk_i2c_write(FL3236_BUS_NUM, FL3236_DEV_ADDR, FL3236_SPEED, write_data, 2);

	write_data[0] = FL3236_FREQUENCY_REG;
	write_data[1] = FREQUENCY_REG_22KHZ;
	mtk_i2c_write(FL3236_BUS_NUM, FL3236_DEV_ADDR, FL3236_SPEED, write_data, 2);

	return 0;
}

int fl3236_led_blue(void)
{
    u8 i = 0;
	for(;i<12;i++)
	{
		if(i<8)
		{
	        write_data[0] = FL3236_PWM_REG+2+i*3;
			write_data[1] = 0xaf;
			mtk_i2c_write(FL3236_BUS_NUM, FL3236_DEV_ADDR, FL3236_SPEED, write_data, 2);

			write_data[0] = FL3236_LED_REG+2+i*3;
			write_data[1] = 0x01;
			mtk_i2c_write(FL3236_BUS_NUM, FL3236_DEV_ADDR, FL3236_SPEED, write_data, 2);
		}
		else
		{
		    write_data[0] = FL3236_PWM_REG + i*3;
			write_data[1] = 0xaf;
			mtk_i2c_write(FL3236_BUS_NUM, FL3236_DEV_ADDR, FL3236_SPEED, write_data, 2);

            write_data[0] = FL3236_LED_REG + i*3;
			write_data[1] = 0x01;
			mtk_i2c_write(FL3236_BUS_NUM, FL3236_DEV_ADDR, FL3236_SPEED, write_data, 2);
		}
	}

	//updata register
	write_data[0] = FL3236_UPDATE_REG;
	write_data[1] = UPDATE_REG_UPDATE;
	mtk_i2c_write(FL3236_BUS_NUM, FL3236_DEV_ADDR, FL3236_SPEED, write_data, 2);

	return 0;
}

int fl3236_init(void)
{
   fl3236_led_init();
   fl3236_led_blue();

   return 0;
}
