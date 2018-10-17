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
 * MediaTek Inc. (C) 2016-2017. All rights reserved.
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

#ifndef __LED_APP_H__
#define __LED_APP_H__

#ifdef  __cplusplus
extern "C" {
#endif

#define FL3236IC_LED 1
#define LED_ON 1
#define LED_OFF 0

#ifndef EXPORT_SYMBOL
#define EXPORT_SYMBOL __attribute__((visibility("default")))
#endif
#define ADAPTOR_OK 0
#define ADAPTOR_FAIL -1

enum
{
    DEBUG = 0,
    INFO,
    WARN,
    ERROR,
};  // event

#define APDATOR_LOG(tag, fmt, args...)           {if (tag >= DEBUG)      printf("[ADAPTOR APP][%s:%d] " fmt "\n", __FUNCTION__, __LINE__, ## args);}

int fd_led;

typedef struct led_function_manager {
	int count;
	int rgb;
	int volume;
	int pwm;
	int (*led_dev_valid)(void);
	int (*led_init)(void);
	int (*led_marquee)(int count, int rgb);
	int (*led_red)(void);
	int (*led_green)(void);
	int (*led_blue)(void);
	int (*led_off)(void);
	int (*led_set_volume)(int volume);
	int (*led_voice_blinking_start)(void);
	int (*led_voice_blinking_stop)(void);
	int (*led_connect_blinking_start)(void);
	int (*led_connect_blinking_stop)(void);
	int (*led_bootup_start)(void);
	int (*led_bootup_stop)(void);
	int (*led_voice_detect_start)(void);
	int (*led_voice_detect_stop)(void);
	int (*led_thinking_show)(void);
} led_fmanager;

int led_init(void);
int led_play_tts(int status);
int led_voice_wakeup();
int led_mic_mute(int status);
int led_ble_server_open(int status);
int led_wifi_connecting(int status);
int led_wifi_connected_success(int status);
int led_wifi_connected_fail(int status);
int led_system_power_on(int status);
int led_thinking(void);
int led_voice_detect_enable(int status);
int led_volume_show(int volume);
#ifdef  __cplusplus
}
#endif

#endif /* __LED_APP_H__ */