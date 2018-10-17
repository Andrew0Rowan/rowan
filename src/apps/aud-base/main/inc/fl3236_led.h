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

#ifndef _FL3236_H_
#define _FL3236_H_

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>

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

#define LED_LEVEL_1              0x01
#define LED_LEVEL_2              0x03
#define LED_LEVEL_2              0x05
#define LED_LEVEL_4              0x07
#define LED_LEVEL_DEF            LED_LEVEL_1
#define LED_OFF                  0x00
#define LED_PWM_DEF              0xaf

#define REG_GROUP_COUNT          12
#define REG_COUNT                36
#define LED_BLUE_GROUP_MASK      0x0924 //0x 100 100 100 100
#define LED_GREEN_GROUP_MASK     0x0492 //0x 010 010 010 010
#define LED_RED_GROUP_MASK       0x0249 //0x 001 001 001 001

#define CMD_SINGLE            (101)
#define CMD_GROUP_MASK        (102)
#define CMD_GROUP_LINEAR      (103)
#define CMD_GROUP_BATCH       (104)
#define CMD_GROUP_ALL         (105)

#define REG_SHIFT              8
#define DATA_SHIFT             0
#define MASK_SHIFT             16
#define COUNT_SHIFT            16
#define LINEAR_REG_SHIFT       24

#define COLOR_RED            1
#define COLOR_GREEN          2
#define COLOR_BLUE           3

#define GEN_SINGLE(reg,data)  \
       ( (reg) << REG_SHIFT | \
		(data) << DATA_SHIFT)

#define GEN_GROUP_MASK(mask,reg,data)  \
        ((mask) << MASK_SHIFT |  \
		(reg) << REG_SHIFT  | \
		(data) << DATA_SHIFT)

#define GEN_GROUP_LINEAR(reg,data1,data2,data3)  \
        ((reg) << LINEAR_REG_SHIFT | \
		(data1) << MASK_SHIFT | \
		(data2) << REG_SHIFT  | \
		(data3) << DATA_SHIFT)

#define GEN_GROUP_BATCH(reg,count,data)  \
        ((reg) << REG_SHIFT | \
		(count) << COUNT_SHIFT | \
		(data) << DATA_SHIFT)

#define GEN_GROUP_ALL(reg,data)  \
        ((reg) << REG_SHIFT | \
		(data) << DATA_SHIFT)


extern int fl3236_dev_valid(void);
extern int fl3236_led_white(void);
extern int fl3236_led_red(void);
extern int fl3236_led_green(void);
extern int fl3236_led_blue(void);
extern int fl3236_led_init(void);
extern int fl3236_leds_off(void);
extern int fl3236_led_marquee(int count, int rgb);//count=0 loop always, count>0 loop $count times.rgb: red=1, green=2,blue=3
extern int fl3236_led_breathe(int count, int rgb);//count=0 loop always, count>0 loop $count times.rgb: red=1, green=2,blue=3
extern int fl3236_led_rgb(int rgb,int pwm);//rgb: red=1, green=2,blue=3.pwm<0 means not set pwm,pwm>=0 set pwm to current
extern void led_thinking_show(void);
extern void led_bootup_start();
extern void led_bootup_stop();
extern void led_voice_detect_start();
extern void led_voice_detect_stop();
extern void led_set_volume(int volume);
#endif
