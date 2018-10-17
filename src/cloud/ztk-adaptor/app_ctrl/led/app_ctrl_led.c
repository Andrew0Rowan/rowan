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
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <pthread.h>
#include "u_common.h"
#include "u_appman.h"
#include "u_app_thread.h"
#include "u_timerd.h"
#include "led.h"
#include "fl3236_led.h"
#include "app_ctrl.h"
#include <time.h>

static pthread_mutex_t s_led_app_voice_led_mutex;
static pthread_cond_t s_led_app_voice_led_cond;
static INT32 s_voice_led_blink_running = FALSE;

static pthread_mutex_t s_led_app_connecting_led_mutex;
static pthread_cond_t s_led_app_connecting_led_cond;
static INT32 s_connecting_led_blink_running = FALSE;

static TIMER_TYPE_T    s_voice_wakeup_timer;


static INT32 _app_ctrl_led_voice_led_blinking(VOID) {

    APP_CTRL_MSG(ERROR,"_led_app_blinking start!");

    while (1) {

        pthread_mutex_lock(&s_led_app_voice_led_mutex);

        pthread_cond_wait(&s_led_app_voice_led_cond, &s_led_app_voice_led_mutex);

        while (s_voice_led_blink_running) {

			if (fd_led > 0)
			{
                ioctl(fd_led, LP5523_LED_ON, 100 << 16 | (FIRST_DEVICE) << 12 | COLOR_BLUE << 8 | LED_BY_DEVICE_IDX << 4 | 0 );
                ioctl(fd_led, LP5523_LED_ON, 100 << 16 | (THIRD_DEVICE) << 12 | COLOR_BLUE << 8 | LED_BY_DEVICE_IDX << 4 | 0 );

                usleep(300000);

                ioctl(fd_led, LP5523_LED_OFF, (FIRST_DEVICE) << 12 | COLOR_BLUE << 8 | LED_BY_DEVICE_IDX << 4 | 0 );
                ioctl(fd_led, LP5523_LED_OFF, (THIRD_DEVICE) << 12 | COLOR_BLUE << 8 | LED_BY_DEVICE_IDX << 4 | 0 );

                usleep(300000);
			}
			if (fl3236_dev_valid())
			    fl3236_led_breathe(1,3);
        }

        pthread_mutex_unlock(&s_led_app_voice_led_mutex);

    }

    APP_CTRL_MSG(ERROR,"_led_app_blinking over!");

    return AEER_OK;
}

static INT32 _app_ctrl_led_voice_led_blinking_thread(VOID)
{
    INT32 i4_ret;
    pthread_t ntid;
    pthread_attr_t t_attr;

    i4_ret = pthread_attr_init(&t_attr);
    if(0 != i4_ret) {
        APP_CTRL_MSG(ERROR,"pthread_attr_init error!\n");
        goto ATTR_INIT_ERR;
    }

    i4_ret = pthread_mutex_init(&s_led_app_voice_led_mutex, NULL);
    if(0 != i4_ret) {
        APP_CTRL_MSG(ERROR,"mutex init failed!\n");
        goto MUTEX_INIT_ERR;
    }

    i4_ret = pthread_cond_init(&s_led_app_voice_led_cond, NULL);
    if(0 != i4_ret) {
        APP_CTRL_MSG(ERROR,"cond init failed!\n");
        goto COND_INIT_ERR;
    }

    i4_ret = pthread_create(&ntid, &t_attr, _app_ctrl_led_voice_led_blinking, NULL);
    if(0 != i4_ret) {
        printf("pthread_create error!\n");
        goto ATTR_SET_ERR;
    }

    pthread_attr_destroy(&t_attr);
    return AEER_OK;

ATTR_SET_ERR:
    pthread_cond_destroy(&s_led_app_voice_led_cond);
COND_INIT_ERR:
    pthread_mutex_destroy(&s_led_app_voice_led_mutex);
MUTEX_INIT_ERR:
    pthread_attr_destroy(&t_attr);
ATTR_INIT_ERR:

    return AEER_FAIL;
}

static INT32 _app_ctrl_led_connecting_led_blinking(VOID) {

    APP_CTRL_MSG(ERROR,"_led_app_connecting_led_blinking start!");

    while (1) {

        pthread_mutex_lock(&s_led_app_connecting_led_mutex);

        pthread_cond_wait(&s_led_app_connecting_led_cond, &s_led_app_connecting_led_mutex);

        while (s_connecting_led_blink_running) {

			if (fd_led > 0)
                ioctl(fd_led, LP5523_LED_ON, 100 << 16 | (FOURTH_DEVICE) << 12 | COLOR_GREEN << 8 | LED_BY_LED_IDX << 4 | LED_INDEX_THIRD);
			if (fl3236_dev_valid())
			    fl3236_led_blue();

            usleep(300000);

			if (fd_led > 0)
                ioctl(fd_led, LP5523_LED_OFF, (FOURTH_DEVICE) << 12 | COLOR_GREEN << 8 | LED_BY_LED_IDX << 4 | LED_INDEX_THIRD);
			if (fl3236_dev_valid())
			    fl3236_led_off();

            usleep(300000);

        }

        pthread_mutex_unlock(&s_led_app_connecting_led_mutex);

    }

    APP_CTRL_MSG(ERROR,"_led_app_connecting_led_blinking over!");

    return AEER_OK;
}

static INT32 _app_ctrl_led_connecting_led_blinking_thread(VOID)
{
    INT32 i4_ret;
    pthread_t ntid;
    pthread_attr_t t_attr;

    i4_ret = pthread_attr_init(&t_attr);
    if(0 != i4_ret) {
        APP_CTRL_MSG(ERROR,"pthread_attr_init error!\n");
        goto ATTR_INIT_ERR;
    }

    i4_ret = pthread_mutex_init(&s_led_app_connecting_led_mutex, NULL);
    if(0 != i4_ret) {
        APP_CTRL_MSG(ERROR,"mutex init failed!\n");
        goto MUTEX_INIT_ERR;
    }

    i4_ret = pthread_cond_init(&s_led_app_connecting_led_cond, NULL);
    if(0 != i4_ret) {
        APP_CTRL_MSG(ERROR,"cond init failed!\n");
        goto COND_INIT_ERR;
    }

    i4_ret = pthread_create(&ntid, &t_attr, _app_ctrl_led_connecting_led_blinking, NULL);
    if(0 != i4_ret) {
        printf("pthread_create error!\n");
        goto ATTR_SET_ERR;
    }

    pthread_attr_destroy(&t_attr);
    return AEER_OK;

ATTR_SET_ERR:
    pthread_cond_destroy(&s_led_app_connecting_led_cond);
COND_INIT_ERR:
    pthread_mutex_destroy(&s_led_app_connecting_led_mutex);
MUTEX_INIT_ERR:
    pthread_attr_destroy(&t_attr);
ATTR_INIT_ERR:

    return AEER_FAIL;
}

VOID app_ctrl_led_voice_blinking_start(VOID) {

    APDATOR_LOG(ERROR,"app_ctrl_led_voice_blinking_start(s_voice_led_blink_running=%d)!",
        s_voice_led_blink_running);

    if (s_voice_led_blink_running == FALSE) {
        s_voice_led_blink_running = TRUE;
        pthread_cond_signal(&s_led_app_voice_led_cond);
    }

}

VOID app_ctrl_led_voice_blinking_stop(VOID) {

    APDATOR_LOG(ERROR,"app_ctrl_led_voice_blinking_stop(s_voice_led_blink_running=%d)!",
        s_voice_led_blink_running);

    s_voice_led_blink_running = FALSE;

}

VOID app_ctrl_led_connecting_blinking_start(VOID) {

    APDATOR_LOG(ERROR,"app_ctrl_led_connecting_blinking_start(s_connecting_led_blink_running=%d)!",
        s_connecting_led_blink_running);

    if (s_connecting_led_blink_running == FALSE) {
        s_connecting_led_blink_running = TRUE;
        pthread_cond_signal(&s_led_app_connecting_led_cond);
    }

}

VOID app_ctrl_led_connecting_blinking_stop(VOID) {

    APDATOR_LOG(ERROR,"app_ctrl_led_connecting_blinking_stop(s_connecting_led_blink_running=%d)!",
        s_connecting_led_blink_running);

    s_connecting_led_blink_running = FALSE;

}

VOID app_ctrl_led_voice_wakeup_close(VOID) {

    APP_CTRL_MSG(ERROR,"led_voice_wakeup_close!");

	if (fd_led > 0)
	{
        ioctl(fd_led, LP5523_LED_OFF, (FIRST_DEVICE) << 12 | COLOR_BLUE << 8 | LED_BY_DEVICE_IDX << 4 | 0 );
        ioctl(fd_led, LP5523_LED_OFF, (THIRD_DEVICE) << 12 | COLOR_BLUE << 8 | LED_BY_DEVICE_IDX << 4 | 0 );
	}

}

VOID app_ctrl_led_voice_wakeup_close_timer_start(VOID) {
    HANDLE_T h_app = NULL_HANDLE;
    INT32 i4_ret = 0;
    APP_CTRL_IMER_INDEX_E msgType = APP_CTRL_TIMER_VOICE_WAKEUP_LED_CLOSE;

    APP_CTRL_MSG(ERROR,"led_voice_wakeup_close_timer_start!");

    if(0 != (i4_ret = u_am_get_app_handle_from_name(&h_app, APP_CTRL_THREAD_NAME))) {
       APP_CTRL_MSG(ERROR,"get handle fail!,i4_ret=%ld",i4_ret);
       return;
    }

    i4_ret = u_timer_start(h_app,
            &s_voice_wakeup_timer,
            (void *)&msgType,
            sizeof(APP_CTRL_IMER_INDEX_E));

    if (OSR_OK != i4_ret) {
        printf("<MISC_DHCP>Start timer failed, error code = %d\n",i4_ret);
       return;
    }

}

INT32 app_ctrl_led_init() {
    INT32 i4_ret;

    APP_CTRL_MSG(ERROR,"led_app_init start!");

    i4_ret = u_timer_create(&s_voice_wakeup_timer);
    if (i4_ret != OSR_OK) {
        APP_CTRL_MSG(ERROR,"create h_timer_wlan fail %d\n", i4_ret);
        return AEER_FAIL;
    }

    s_voice_wakeup_timer.e_flags = X_TIMER_FLAG_ONCE;
    s_voice_wakeup_timer.ui4_delay = 100;

    _app_ctrl_led_voice_led_blinking_thread();

    _app_ctrl_led_connecting_led_blinking_thread();

    APP_CTRL_MSG(ERROR,"led_app_init success!");

    return AEER_OK;
}

