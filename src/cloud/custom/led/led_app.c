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

#include "common.h"
#include "led_app.h"
#include "fl3236_led.h"

int led_fmanager_init(led_fmanager *led_manager)
{
#ifdef FL3236IC_LED
    led_manager->count = 1;
    led_manager->rgb = 3;
    led_manager->led_init = fl3236_led_init;
    led_manager->led_dev_valid = fl3236_dev_valid;
    led_manager->led_marquee = fl3236_led_marquee;
    led_manager->led_red = fl3236_led_red;
    led_manager->led_blue = fl3236_led_blue;
    led_manager->led_green = fl3236_led_green;
    led_manager->led_off = fl3236_leds_off;
    led_manager->led_set_volume = fl3236_led_set_volume;
    led_manager->led_bootup_start = fl3236_led_bootup_start;
    led_manager->led_bootup_stop = fl3236_led_bootup_stop;
    led_manager->led_voice_detect_start = fl3236_led_voice_detect_start;
    led_manager->led_voice_detect_stop = fl3236_led_voice_detect_stop;
    led_manager->led_voice_blinking_start = fl3236_led_voice_blinking_start;
    led_manager->led_voice_blinking_stop = fl3236_led_voice_blinking_stop;
    led_manager->led_connect_blinking_start = fl3236_led_connect_blinking_start;
    led_manager->led_connect_blinking_stop = fl3236_led_connect_blinking_stop;
    led_manager->led_thinking_show = fl3236_led_thinking_show;
#endif
    return ADAPTOR_OK;
}

EXPORT_SYMBOL int led_play_tts(int status) {

    APDATOR_LOG(ERROR,"Audio led_play_tts(status = %d)!",status);

    led_fmanager *led_manager;
    led_manager = (led_fmanager*)malloc(sizeof(led_fmanager));
    led_fmanager_init(led_manager);

    if (status == LED_ON) {
        if (led_manager->led_dev_valid())
            led_manager->led_voice_detect_start();
    } else {
        if (led_manager->led_dev_valid())
            led_manager->led_voice_detect_stop();
    }

    return ADAPTOR_OK;
}

EXPORT_SYMBOL int led_voice_wakeup() {

    APDATOR_LOG(ERROR,"Audio led_voice_wakeup!");

    led_fmanager *led_manager;
    led_manager = (led_fmanager*)malloc(sizeof(led_fmanager));
    led_fmanager_init(led_manager);

	if (led_manager->led_dev_valid())
	   led_manager->led_marquee(led_manager->count, led_manager->rgb);

    //app_ctrl_led_voice_wakeup_close_timer_start();

    return ADAPTOR_OK;
}

EXPORT_SYMBOL int led_mic_mute(int status) {

    APDATOR_LOG(ERROR,"Audio led_microphone_mute(status = %d)!",status);

    led_fmanager *led_manager;
    led_manager = (led_fmanager*)malloc(sizeof(led_fmanager));
    led_fmanager_init(led_manager);

    if (status == LED_ON) {
        if (led_manager->led_dev_valid())
            led_manager->led_red();
    } else {
        if (led_manager->led_dev_valid())
            led_manager->led_off();
    }

    return ADAPTOR_OK;
}

EXPORT_SYMBOL int led_ble_server_open(int status) {

    APDATOR_LOG(ERROR,"Audio led_ble_server_open(status = %d)!",status);
    led_fmanager *led_manager;
    led_manager = (led_fmanager*)malloc(sizeof(led_fmanager));
    led_fmanager_init(led_manager);

    if (status == LED_ON) {
        if (led_manager->led_dev_valid())
            led_manager->led_green();
    } else {
        if (led_manager->led_dev_valid())
            led_manager->led_off();
    }

    return ADAPTOR_OK;
}

EXPORT_SYMBOL int led_wifi_connecting(int status) {

    APDATOR_LOG(ERROR,"Audio led_wifi_connecting(status = %d)!",status);

    led_fmanager *led_manager;
    led_manager = (led_fmanager*)malloc(sizeof(led_fmanager));
    led_fmanager_init(led_manager);

    if (status == LED_ON) {
        if (led_manager->led_dev_valid())
            led_manager->led_connect_blinking_start();
    } else {
        if (led_manager->led_dev_valid())
            led_manager->led_connect_blinking_stop();
    }

    return ADAPTOR_OK;
}

EXPORT_SYMBOL int led_wifi_connected_success(int status) {

    APDATOR_LOG(ERROR,"Audio led_wifi_connected_success(status = %d)!",status);

    led_fmanager *led_manager;
    led_manager = (led_fmanager*)malloc(sizeof(led_fmanager));
    led_fmanager_init(led_manager);

    if (status == LED_ON) {
        if (led_manager->led_dev_valid())
            led_manager->led_blue();
    } else {
        if (led_manager->led_dev_valid())
            led_manager->led_off();
    }

    return ADAPTOR_OK;
}

EXPORT_SYMBOL int led_wifi_connected_fail(int status) {

    APDATOR_LOG(ERROR,"Audio led_wifi_connected_fail(status = %d)!",status);

    led_fmanager *led_manager;
    led_manager = (led_fmanager*)malloc(sizeof(led_fmanager));
    led_fmanager_init(led_manager);

    if (status == LED_ON) {
        if (led_manager->led_dev_valid())
            led_manager->led_red();
    } else {
        if (led_manager->led_dev_valid())
            led_manager->led_off();
    }

    return ADAPTOR_OK;
}

EXPORT_SYMBOL int led_system_power_on(int status) {

    APDATOR_LOG(ERROR,"Audio led_system_power_on_status(status = %d)!",status);

    led_fmanager *led_manager;
    led_manager = (led_fmanager*)malloc(sizeof(led_fmanager));
    led_fmanager_init(led_manager);

    if (status == LED_ON) {
        if (led_manager->led_dev_valid())
            led_manager->led_bootup_start();
    } else {
        if (led_manager->led_dev_valid())
            led_manager->led_bootup_stop();
    }

    return ADAPTOR_OK;
}

EXPORT_SYMBOL int led_thinking(void) {

    led_fmanager *led_manager;
    led_manager = (led_fmanager*)malloc(sizeof(led_fmanager));
    led_fmanager_init(led_manager);

    led_manager->led_thinking_show();
}

EXPORT_SYMBOL int led_voice_detect_enable(int status) {

    led_fmanager *led_manager;
    led_manager = (led_fmanager*)malloc(sizeof(led_fmanager));
    led_fmanager_init(led_manager);

    if (status == LED_ON) {
        APDATOR_LOG(ERROR, "Audio led voice detect enable");
        led_manager->led_voice_detect_start();
    } else {
        APDATOR_LOG(ERROR, "Audio led voice detect disable");
        led_manager->led_voice_detect_stop();
    }
}

EXPORT_SYMBOL int led_volume_show(int volume) {
    APDATOR_LOG(ERROR, "Audio led_volume_show volume = %d", volume);

    led_fmanager *led_manager;
    led_manager = (led_fmanager*)malloc(sizeof(led_fmanager));
    led_fmanager_init(led_manager);

    led_manager->volume = volume;
    led_manager->led_set_volume(led_manager->volume);
}

EXPORT_SYMBOL int led_init(void) {
    APDATOR_LOG(ERROR, "Audio led_init\n");
    led_fmanager *led_manager;
    led_manager = (led_fmanager*)malloc(sizeof(led_fmanager));
    led_fmanager_init(led_manager);

    led_manager->led_init();
}