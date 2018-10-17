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


#include "fl3236_led.h"
#include <pthread.h>

#define FL3236_DEV  /dev/fl3236


int fl3236_led_fd = NULL;
static int fl3236_led_bootup_working = 0;
static int fl3236_led_voice_detect_working = 0;
static pthread_t fl3236_bootup_tid;
static pthread_t fl3236_wakeup_tid;

static int _led_set_pwm(int data)
{
    return ioctl(fl3236_led_fd,CMD_GROUP_ALL,GEN_GROUP_ALL(FL3236_PWM_REG,data));
}

static int _led_enable()
{
    return ioctl(fl3236_led_fd,CMD_SINGLE,GEN_SINGLE(FL3236_SHUTDOWN_REG,0x01));
}

int _led_all_channel_disable(void)
{
    return ioctl(fl3236_led_fd,CMD_SINGLE,GEN_SINGLE(FL3236_CONTROL_REG,CONTROL_REG_SHUTDOWN_ALL));
}

int _led_all_channel_enable(void)
{
    return ioctl(fl3236_led_fd,CMD_SINGLE,GEN_SINGLE(FL3236_CONTROL_REG,CONTROL_REG_WORK_ALL));
}

void _led_sleep(int ms)
{
    usleep(ms*1000);
}

int _led_white(void)
{
    return ioctl(fl3236_led_fd,CMD_GROUP_ALL,GEN_GROUP_ALL(FL3236_LED_REG,LED_LEVEL_DEF));
}

int _led_red(void)
{
    // led 1~4 display red
   ioctl(fl3236_led_fd,CMD_GROUP_MASK,GEN_GROUP_MASK(LED_RED_GROUP_MASK,FL3236_LED_REG,LED_LEVEL_DEF));

   //led 5~8 display red
   ioctl(fl3236_led_fd,CMD_GROUP_MASK,GEN_GROUP_MASK(LED_RED_GROUP_MASK,FL3236_LED_REG+REG_GROUP_COUNT,LED_LEVEL_DEF));

   // because led 9~12, firt output pin  is Blue but not Red, so workaround, mask is diff
   ioctl(fl3236_led_fd,CMD_GROUP_MASK,GEN_GROUP_MASK(LED_BLUE_GROUP_MASK,FL3236_LED_REG+REG_GROUP_COUNT*2,LED_LEVEL_DEF));

   return 0;
}

int _led_green(void)
{
    // led 1~4 display red
    ioctl(fl3236_led_fd,CMD_GROUP_MASK,GEN_GROUP_MASK(LED_GREEN_GROUP_MASK,FL3236_LED_REG,LED_LEVEL_DEF));

    //led 5~8 display red
    ioctl(fl3236_led_fd,CMD_GROUP_MASK,GEN_GROUP_MASK(LED_GREEN_GROUP_MASK,FL3236_LED_REG+REG_GROUP_COUNT,LED_LEVEL_DEF));

    // because led 9~12, all leds gren pin is output2, so don't need workaround
    ioctl(fl3236_led_fd,CMD_GROUP_MASK,GEN_GROUP_MASK(LED_GREEN_GROUP_MASK,FL3236_LED_REG+REG_GROUP_COUNT*2,LED_LEVEL_DEF));

    return 0;
}

int _led_blue(void)
{
    // led 1~4 display red
    ioctl(fl3236_led_fd,CMD_GROUP_MASK,GEN_GROUP_MASK(LED_BLUE_GROUP_MASK,FL3236_LED_REG,LED_LEVEL_DEF));

    //led 5~8 display red
    ioctl(fl3236_led_fd,CMD_GROUP_MASK,GEN_GROUP_MASK(LED_BLUE_GROUP_MASK,FL3236_LED_REG+REG_GROUP_COUNT,LED_LEVEL_DEF));

    // because led 9~12, last output pin is Red but not Red, so workaround, mask is diff
    ioctl(fl3236_led_fd,CMD_GROUP_MASK,GEN_GROUP_MASK(LED_RED_GROUP_MASK,FL3236_LED_REG+REG_GROUP_COUNT*2,LED_LEVEL_DEF));

    return 0;
}

int fl3236_leds_off(void)
{
    return ioctl(fl3236_led_fd,CMD_GROUP_ALL,GEN_GROUP_ALL(FL3236_LED_REG,LED_OFF));
}

int fl3236_leds_on(int level)
{
    return ioctl(fl3236_led_fd, CMD_GROUP_ALL, GEN_GROUP_ALL(FL3236_LED_REG, level));
}

int fl3236_led_off(int led)
{
    if ((led < 0) || (led > 11)) {
        printf("fl3236_led_off led(%d) is invalid!\n", led);
        return -1;
    }
    return ioctl(fl3236_led_fd,CMD_GROUP_LINEAR,GEN_GROUP_LINEAR((FL3236_LED_REG + 3 * led),LED_OFF, LED_OFF, LED_OFF));
}

int fl3236_led_on(int led, int level)
{
    if ((led < 0) || (led > 11)) {
        printf("fl3236_led_on led(%d) is invalid!\n", led);
        return -1;
    }
    return ioctl(fl3236_led_fd,CMD_GROUP_LINEAR,GEN_GROUP_LINEAR((FL3236_LED_REG + 3 * led),level, level, level));
}

int fl3236_led_white(void)
{
    fl3236_leds_off();
    _led_set_pwm(LED_PWM_DEF);
	_led_white();
}

int fl3236_led_red(void)
{
   fl3236_leds_off();
   _led_set_pwm(LED_PWM_DEF);
   _led_red();

   return 0;

}

int fl3236_led_green(void)
{
   fl3236_leds_off();
   _led_set_pwm(LED_PWM_DEF);
   _led_green();

   return 0;
}

int fl3236_led_blue(void)
{
   fl3236_leds_off();
   _led_set_pwm(LED_PWM_DEF);
   _led_blue();

   return 0;
}

int fl3236_led_color (unsigned char led, unsigned char red, unsigned char green, unsigned char blue) {
    if (led < 8) {
        ioctl(fl3236_led_fd, CMD_GROUP_LINEAR, GEN_GROUP_LINEAR(FL3236_PWM_REG + 3 * led, red, green, blue));
    } else {
        ioctl(fl3236_led_fd, CMD_GROUP_LINEAR, GEN_GROUP_LINEAR(FL3236_PWM_REG + 3 * led, blue, green, red));
    }
}

int fl3236_leds_color(unsigned char red, unsigned char green, unsigned char blue)
{
    int i;
    for (i = 0; i < REG_GROUP_COUNT; i++) {
        fl3236_led_color(i, red, green, blue);
    }
}

int fl3236_led_rgb(int rgb,int pwm)
{
    if (pwm >= 0)
	    _led_set_pwm(pwm);

    switch (rgb)
	{
	    case COLOR_RED:
		    _led_red();
			break;
		case COLOR_GREEN:
		    _led_green();
			break;
		case COLOR_BLUE:
		    _led_blue();
			break;
		default:
		    break;
	}
	return 0;
}

int fl3236_led_marquee(int count, int rgb)
{
	if (count <= 0)
	    return -1;

    int i,j,k,loop;
	loop=count==0?1:count;

	fl3236_leds_off();

	while (loop>0)
	{
    	for (j=0;j<=REG_GROUP_COUNT;j++)
    	{
        	for (i=0;i<REG_GROUP_COUNT;i++)
        	{
			    k=i+j>11?(i+j)%12:i+j;
        	    ioctl(fl3236_led_fd,CMD_GROUP_LINEAR,GEN_GROUP_LINEAR(FL3236_PWM_REG+3*k,0x0a+i*10,0x0a+i*10,0x0a+i*10));
				if(i==0 && j==0)
				{
				  fl3236_led_rgb(rgb,-1);
				}
        	}//end for i
			_led_sleep(20);
    	}//end for j
		if (count>0)
	        loop --;
	}//end while

	fl3236_leds_off();
    return 0;
}

int fl3236_led_breathe(int count,int rgb)
{
     if (count <= 0)
	     return -1;

     int led_pwm = 0x00;
	 int loop = count==0?1:count;

	 fl3236_leds_off();
	 ioctl(fl3236_led_fd,CMD_GROUP_ALL,GEN_GROUP_ALL(FL3236_PWM_REG,led_pwm));
	 fl3236_led_rgb(rgb,-1);

	 while (loop > 0)
	 {
		 led_pwm = 0x00;
	     while (led_pwm < 0xfd)
	     {
	         ioctl(fl3236_led_fd,CMD_GROUP_ALL,GEN_GROUP_ALL(FL3236_PWM_REG,led_pwm));
	    	 led_pwm += 2;
	    	 //usleep(8000);
	     }

         while (led_pwm >= 0x01)
	     {
	         ioctl(fl3236_led_fd,CMD_GROUP_ALL,GEN_GROUP_ALL(FL3236_PWM_REG,led_pwm));
	    	 led_pwm -= 2;
			 //usleep(8000);
	     }
		 if (count>0)
		     loop --;
     }
	 fl3236_leds_off();

	 return 0;
}

int fl3236_led_init(void)
{
    int i4_ret = 0;

	if (fl3236_led_fd != NULL)
	    printf("[led] led device already init \n");

	fl3236_led_fd = open("/dev/fl3236", O_RDWR);
	if (!fl3236_led_fd)
	{
	    printf("[led] open /dev/fl3236 fail \n");
		return -1;
	}

	return i4_ret;

}

int fl3236_dev_valid(void)
{
    return fl3236_led_fd > 0?1:0;
}

int led_demo_thread_fct(void)
{
    fl3236_led_init();
	int data = 0;

    while (1)
	{
	    _led_set_pwm(0x6f);
    	fl3236_led_white();
    	usleep(1000000);
    	fl3236_led_red();
    	usleep(1000000);
    	fl3236_led_green();
    	usleep(1000000);
    	fl3236_led_blue();
        usleep(1000000);
		fl3236_led_breathe(2,1);
		usleep(1000000);
		fl3236_led_breathe(2,2);
		usleep(1000000);
		fl3236_led_breathe(2,3);
		usleep(1000000);
		fl3236_led_marquee(3,1);
		usleep(1000000);
		fl3236_led_marquee(3,2);
		usleep(1000000);
		fl3236_led_marquee(3,3);
		usleep(1000000);
	}

}


void led_thinking_show() {
    int i = 0, j = 0, loopcount = 0;
    printf("6 fl3236_led_thinking is called!!\n");
    fl3236_leds_off();
    for (i = 0; i < REG_GROUP_COUNT; i++) {
        if ((i % 2) == 0)
            fl3236_led_color(i, 0, 253, 255);
        else
            fl3236_led_color(i, 0, 0, 255);
    }
#if 0
    while(fl3236_led_thinking_working) {
        for (i = j; i < REG_GROUP_COUNT; i += 2) {
            fl3236_led_on(i, LED_LEVEL_DEF);
            _led_sleep(30);
        }
        for (i = j; i < REG_GROUP_COUNT; i += 2)
            fl3236_led_off(i);
        if (j == 0)
            j = 1;
        else
            j = 0;
    }
#endif
    while (loopcount < 30) {
        for (i = 0; i < REG_GROUP_COUNT; i++) {
            if ((i % 2) == 0) {
                fl3236_led_on(i, LED_LEVEL_DEF);
            }
        }
        _led_sleep(30);
        fl3236_leds_off();
        _led_sleep(10);
        loopcount++;
    }
    fl3236_leds_off();
    printf("led thinking thread exit\n");
}


void led_bootup_thread() {
    int i;

    printf("led_bootup_thread is starting!\n");
    fl3236_leds_off();
    while(fl3236_led_bootup_working) {
        for (i = 0; i < REG_GROUP_COUNT; i++) {
            fl3236_leds_color(0, 0, 255);
            fl3236_led_color((REG_GROUP_COUNT - i - 1), 0, 253, 255);
            fl3236_leds_on(LED_LEVEL_DEF);
            _led_sleep(20);
        }
    }
    fl3236_leds_off();
    printf("led_bootup_thread is exit!\n");
}

void led_bootup_start() {
    int ret;
    printf("led_bootup_start is called!\n");
    fl3236_led_bootup_working = 1;

    ret = pthread_create(&fl3236_bootup_tid, NULL, (void *)led_bootup_thread, NULL);
    if (ret < 0) {
        fl3236_led_bootup_working = 0;
        printf("led bootup thread create failed!\n");
    }
    return;
}

void led_bootup_stop() {
    printf("led_bootup_stop is called!\n");
    if (fl3236_led_bootup_working) {
        fl3236_led_bootup_working = 0;
        pthread_join(fl3236_bootup_tid, 0);
    }
}

void led_voice_detect_thread(void) {
    int i;
    fl3236_leds_off();
    fl3236_leds_color(0, 0, 255);
    for (i = 0; i < REG_GROUP_COUNT; i += 3)
    fl3236_led_color(i, 0, 253, 255);
    fl3236_leds_on(LED_LEVEL_DEF);
    while (fl3236_led_voice_detect_working) {
        _led_sleep(30);
    }
    fl3236_leds_off();

}

void led_voice_detect_start() {
    int ret;
    printf("led_wakeup_start is called!\n");
    fl3236_led_voice_detect_working = 1;
    ret = pthread_create(&fl3236_wakeup_tid, NULL, (void *)led_voice_detect_thread, NULL);
    if (ret < 0) {
        fl3236_led_voice_detect_working = 0;
        printf("led voice detect thread creat failed!\n");
    }
    return;
}

void led_voice_detect_stop() {
    printf("led voice detect stop!\n");
    if (fl3236_led_voice_detect_working) {
        fl3236_led_voice_detect_working = 0;
        pthread_join(fl3236_wakeup_tid, 0);
    }
}

void led_set_volume(int volume) {
    int led_count[] = {0, 1, 2, 4, 5, 6, 7, 8, 10, 11, 12}, i, loopcount = 0, j, k;
    if (volume > 100)
        volume = 100;
    if (volume < 0)
        volume = 0;
    printf("led_set_volume is called! volume = %d\n", volume);
    volume = volume / 10;

    fl3236_leds_color(255, 255, 255);
    while (loopcount < 2) {
        for (j = 0; j < REG_GROUP_COUNT; j++) {
            for (i = 0; i < led_count[volume]; i++) {
                k = j + i;
                if (k >= REG_GROUP_COUNT)
                    k = (k % REG_GROUP_COUNT);
                fl3236_led_on((REG_GROUP_COUNT - k - 1), LED_LEVEL_DEF);
            }
            _led_sleep(20);
            fl3236_leds_off();
        }
        loopcount++;
    }

}

