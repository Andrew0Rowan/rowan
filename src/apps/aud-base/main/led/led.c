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


#include "led.h"

void led_horse_race(int light_duty, int color_index)
{
  int j = 0;
  for(j=0;j<3;j++)
  {
    ioctl(fd_led, LP5523_LED_ON, light_duty << 16 | (FIRST_DEVICE) << 12 | color_index << 8 | LED_BY_LED_IDX << 4 | (LED_INDEX_FIRST + j));
    usleep(200000);
  }
  for(j=0;j<3;j++)
  {
    ioctl(fd_led, LP5523_LED_ON, light_duty << 16 | (THIRD_DEVICE) << 12 | color_index << 8 | LED_BY_LED_IDX << 4 | (LED_INDEX_FIRST + j));
    usleep(200000);
  }
  for(j=0;j<3;j++)
  {
    ioctl(fd_led, LP5523_LED_ON, light_duty << 16 | (SECOND_DEVICE) << 12 | color_index << 8 | LED_BY_LED_IDX << 4 | (LED_INDEX_THIRD - j));
    usleep(200000);
  }
  for(j=0;j<3;j++)
  {
    ioctl(fd_led, LP5523_LED_ON, light_duty << 16 | (FOURTH_DEVICE) << 12 | color_index << 8 | LED_BY_LED_IDX << 4 | (LED_INDEX_THIRD - j));
    usleep(200000);
  }
  for(j=0;j<3;j++)
  {
    ioctl(fd_led, LP5523_LED_OFF, (FIRST_DEVICE) << 12 | color_index << 8 | LED_BY_LED_IDX << 4 | (LED_INDEX_FIRST + j));
    usleep(200000);
  }
  for(j=0;j<3;j++)
  {
    ioctl(fd_led, LP5523_LED_OFF, (THIRD_DEVICE) << 12 | color_index << 8 | LED_BY_LED_IDX << 4 | (LED_INDEX_FIRST + j));
    usleep(200000);
  }
  for(j=0;j<3;j++)
  {
    ioctl(fd_led, LP5523_LED_OFF, (SECOND_DEVICE) << 12 | color_index << 8 | LED_BY_LED_IDX << 4 | (LED_INDEX_THIRD - j));
    usleep(200000);
  }
  for(j=0;j<3;j++)
  {
    ioctl(fd_led, LP5523_LED_OFF, (FOURTH_DEVICE) << 12 | color_index << 8 | LED_BY_LED_IDX << 4 | (LED_INDEX_THIRD - j));
    usleep(200000);
  }
}

void led_blink(int light_duty, int blink_times, int color_index)
{
  int i = 0;
  int j = 0;
  for(j=0;j<blink_times;j++)
  {
    for(i=0;i<4;i++)
	{
	  ioctl(fd_led, LP5523_LED_ON, light_duty << 16 | (FIRST_DEVICE + i) << 12 | color_index << 8 | LED_BY_DEVICE_IDX << 4 | 0 );
    }
	usleep(500000);
    for(i=0;i<4;i++)
	{
      ioctl(fd_led, LP5523_LED_OFF, (FIRST_DEVICE + i) << 12 | color_index << 8 | LED_BY_DEVICE_IDX << 4 | 0 );
	}
	usleep(500000);
  }
}

void led_light(int light_duty, int color_index)
{
  int i = 0;
  for(i=0;i<4;i++)
  {
	ioctl(fd_led, LP5523_LED_ON, light_duty << 16 | (FIRST_DEVICE + i) << 12 | color_index << 8 | LED_BY_DEVICE_IDX << 4 | 0 );
  }
}

void led_light_stop(void)
{
  int i = 0;
  for(i=0;i<4;i++)
  {
    ioctl(fd_led, LP5523_LED_OFF, (FIRST_DEVICE + i) << 12 | 0 << 8 | LED_BY_DEVICE_IDX << 4 | 0 );
  }
}

void led_light_eachone(int light_duty, int device_index, int color_index, int led_num)
{
	ioctl(fd_led, LP5523_LED_ON, light_duty << 16 | device_index << 12 | color_index << 8 | LED_BY_LED_IDX << 4 | led_num );
}

void led_light_stop_eachone(int device_index, int led_num)
{
	ioctl(fd_led, LP5523_LED_OFF, device_index << 12 | 0 << 8 | LED_BY_LED_IDX << 4 | led_num );
}
