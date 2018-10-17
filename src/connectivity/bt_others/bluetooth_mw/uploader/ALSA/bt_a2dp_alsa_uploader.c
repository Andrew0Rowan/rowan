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

/* FILE NAME:  bt_a2dp_alsa_uploader.c
 * PURPOSE:
 *  {1. What is covered in this file - function and scope.}
 *  {2. Related documents or hardware information}
 * NOTES:
 *  {Something must be known or noticed}
 *  {1. How to use these functions - Give an example.}
 *  {2. Sequence of messages if applicable.}
 *  {3. Any design limitation}
 *  {4. Any performance limitation}
 *  {5. Is it a reusable component}
 *
 *
 *
 */
/* INCLUDE FILE DECLARATIONS
 */
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <assert.h>
#include <fcntl.h>
#include <sys/prctl.h>
#include <sys/time.h>
#include <time.h>
#include <asoundlib.h>

#include "bt_a2dp_alsa_uploader.h"
#include "queue.h"
#include "bt_mw_log.h"

/* NAMING CONSTANT DECLARATIONS
 */
typedef enum
{
    BT_ALSA_UPL_LOG_ERROR = 0,
    BT_ALSA_UPL_LOG_WARNING,
    BT_ALSA_UPL_LOG_NORMAL,
    BT_ALSA_UPL_LOG_NOTICE,
    BT_ALSA_UPL_LOG_INFO,
    BT_ALSA_UPL_LOG_MINOR = 5,
    BT_ALSA_UPL_LOG_MAX,
}BT_ALSA_UPL_LOG_T;

/* MACRO FUNCTION DECLARATIONS
 */

#define BT_ALSA_CAP_DEV "default"

#define BT_ALSA_UPL_DEBUG

#define BT_ALSA_CAP_QUEUE_SIZE          (4*8*2)
#define BT_ALSA_CAP_QUEUE_ITEM_SIZE     (4*1024)

#define BT_ALSA_CAP_CHUNK_BYTES         (3072+1024)
#define BT_ALSA_PUSH_BYTES              (4096)

#define BT_ALSA_UPL_LOG_MINOR(s, ...) BT_DBG_MINOR(BT_DEBUG_UPL, s, ## __VA_ARGS__)

#define BT_ALSA_UPL_LOG_INFO(s, ...) BT_DBG_INFO(BT_DEBUG_UPL, s, ## __VA_ARGS__)

#define BT_ALSA_UPL_LOG_NOTICE(s, ...) BT_DBG_NOTICE(BT_DEBUG_UPL, s, ## __VA_ARGS__)

#define BT_ALSA_UPL_LOG_NORMAL(s, ...) BT_DBG_NORMAL(BT_DEBUG_UPL, s, ## __VA_ARGS__)

#define BT_ALSA_UPL_LOG_WARNING(s, ...) BT_DBG_WARNING(BT_DEBUG_UPL, s, ## __VA_ARGS__)

#define BT_ALSA_UPL_LOG_ERROR(s, ...) BT_DBG_ERROR(BT_DEBUG_UPL, s, ## __VA_ARGS__)


/* DATA TYPE DECLARATIONS
 */
typedef struct
{
    int inited;
    snd_pcm_t *cap_handle;
    queue *cap_q;
    int need_quit;
    int enq_data;
    int drop_ms; /* after starting, drop some data */
    int sample_rate;
    int channels;
    pthread_t cap_thread;
    pthread_t send_thread;
    pthread_cond_t cap_cond;
    pthread_mutex_t cap_mutex;
} BT_ALSA_UPL_CTX;


/* GLOBAL VARIABLE DECLARATIONS
 */
/* LOCAL SUBPROGRAM DECLARATIONS
 */
static INT32 bt_a2dp_alsa_uploader_init(INT32 freq, INT32 channel);
static INT32 bt_a2dp_alsa_uploader_deinit(VOID);
static INT32 bt_a2dp_alsa_uploader_start(INT32 delay_ms);
static INT32 bt_a2dp_alsa_uploader_stop(VOID);

/* STATIC VARIABLE DECLARATIONS
 */
static BT_A2DP_UPLOADER s_bt_a2dp_alsa_uploader =
{
    bt_a2dp_alsa_uploader_init,
    bt_a2dp_alsa_uploader_start,
    bt_a2dp_alsa_uploader_stop,
    bt_a2dp_alsa_uploader_deinit,
};

static BT_A2DP_ALSA_OUTPUT_CALLBACK s_bt_a2dp_alsa_output_cb = NULL;

static BT_ALSA_UPL_CTX s_bt_a2dp_alsa_upl_ctx = {0};


#ifdef BT_ALSA_UPL_DEBUG
static UINT32 s_bt_a2dp_alsa_wait_us = 0; /* if wait some time after open dev */

static UINT32 s_bt_a2dp_read_frames = 0; /* if it is 0, default is chunk size */
static UINT32 s_bt_a2dp_dump_enable = 0; /* enable pcm dump */
static FILE *s_bt_a2dp_outputPcmSampleFile = NULL;
static char s_bt_a2dp_outputFilename[128] = "/tmp/src_sample.pcm";

static UINT32 s_bt_a2dp_alsa_send_wait_us = 0;

static UINT32 s_bt_a2dp_alsa_drop_us = 0;
#endif

static UINT32 s_bt_a2dp_push_bytes = BT_ALSA_PUSH_BYTES; /* the data length put in data queue */

//static int fatal_errors = 0;
static int monotonic = 1;
//static snd_pcm_stream_t stream = SND_PCM_STREAM_CAPTURE;
//static int verbose = 0;
//static snd_output_t *log;
static int quiet_mode = 0;
static size_t bits_per_sample, bits_per_frame;
static size_t chunk_bytes = 0;
static snd_pcm_format_t format = SND_PCM_FORMAT_S16_LE;
static snd_pcm_uframes_t chunk_size = 0;
#define HAVE_CLOCK_GETTIME 1

#define bt_a2dp_alsa_timersub(a, b, result) \
do { \
    (result)->tv_sec = (a)->tv_sec - (b)->tv_sec; \
    (result)->tv_usec = (a)->tv_usec - (b)->tv_usec; \
        if ((result)->tv_usec < 0) { \
        --(result)->tv_sec; \
        (result)->tv_usec += 1000000; \
    } \
} while (0)

#define timermsub(a, b, result) \
    do { \
        (result)->tv_sec = (a)->tv_sec - (b)->tv_sec; \
        (result)->tv_nsec = (a)->tv_nsec - (b)->tv_nsec; \
        if ((result)->tv_nsec < 0) { \
            --(result)->tv_sec; \
            (result)->tv_nsec += 1000000000L; \
        } \
    } while (0)

/* EXPORTED SUBPROGRAM BODIES
 */

EXPORT_SYMBOL BT_A2DP_UPLOADER* bt_a2dp_alsa_get_uploader(VOID)
{
    return &s_bt_a2dp_alsa_uploader;
}

EXPORT_SYMBOL VOID bt_a2dp_alsa_register_output_callback(BT_A2DP_ALSA_OUTPUT_CALLBACK output_cb)
{
    s_bt_a2dp_alsa_output_cb = output_cb;

    return;
}

EXPORT_SYMBOL VOID bt_a2dp_alsa_uploader_set_debug_flag(INT32 flag, UINT32 value)
{
    switch (flag)
    {
#ifdef BT_ALSA_UPL_DEBUG
        case 0:
            s_bt_a2dp_alsa_wait_us = value;
            break;
        case 1:
            s_bt_a2dp_read_frames = value;
            break;
        case 2:
            s_bt_a2dp_dump_enable = value;
            break;
        case 3:
            s_bt_a2dp_push_bytes = value > BT_ALSA_PUSH_BYTES ? BT_ALSA_PUSH_BYTES : value;
            break;
        case 4:
            s_bt_a2dp_alsa_send_wait_us = value;
            break;
        case 5:
            s_bt_a2dp_alsa_drop_us = value;
            break;
#endif
        default:
            break;
    }

    return;
}


/* LOCAL SUBPROGRAM BODIES
 */

static snd_pcm_t* bt_a2dp_alsa_openCapDev(const CHAR *pCapDevName, unsigned int freq, int channel)
{
    int err = 0;
    snd_pcm_t *capture_handle = NULL;
    snd_pcm_hw_params_t *hw_params = NULL;
    snd_pcm_sw_params_t *sw_params = NULL;
    static unsigned int buffer_time = 0;//500000;
    static unsigned int period_time = 0;//1000;
    unsigned int u4rate;
    static snd_pcm_uframes_t buffer_frames = 0;
    snd_pcm_uframes_t buffer_size;
    snd_pcm_uframes_t start_threshold;
    snd_pcm_uframes_t stop_threshold;
    static snd_pcm_uframes_t period_frames = 0;
    size_t n;
    static int start_delay = 1;//200000;
    static int stop_delay = 0;
    static int avail_min = -1;

    if ((err = snd_pcm_open(&capture_handle, pCapDevName, SND_PCM_STREAM_CAPTURE, 0)) < 0)
    {
        BT_ALSA_UPL_LOG_ERROR("cannot open audio device %s (%s)", pCapDevName, snd_strerror(err));
        goto __ERROR;
    }

    BT_ALSA_UPL_LOG_NORMAL("open audio device %s success", pCapDevName);

    snd_pcm_hw_params_alloca(&hw_params);
    snd_pcm_sw_params_alloca(&sw_params);

    if ((err = snd_pcm_hw_params_any(capture_handle, hw_params)) < 0)
    {
        BT_ALSA_UPL_LOG_ERROR("cannot initialize hw param %s", snd_strerror(err));
        goto __ERROR;
    }

    if ((err = snd_pcm_hw_params_set_access(capture_handle, hw_params, SND_PCM_ACCESS_RW_INTERLEAVED)) < 0)
    {
        BT_ALSA_UPL_LOG_ERROR("cannot set access type %s", snd_strerror(err));
        goto __ERROR;
    }

    if ((err = snd_pcm_hw_params_set_format(capture_handle, hw_params, format)) < 0)
    {
        BT_ALSA_UPL_LOG_ERROR("cannot set sample format %s", snd_strerror(err));
        goto __ERROR;
    }

    if ((err = snd_pcm_hw_params_set_rate(capture_handle, hw_params, freq, 0)) < 0)
    {
        BT_ALSA_UPL_LOG_ERROR("cannot set sample rate %s", snd_strerror(err));
        goto __ERROR;
    }

    u4rate = freq;
    BT_ALSA_UPL_LOG_NORMAL("set sample rate %u success", freq);

    if ((err = snd_pcm_hw_params_set_channels(capture_handle, hw_params, channel)) < 0)
    {
        BT_ALSA_UPL_LOG_ERROR("cannot set sample rate %s", snd_strerror(err));
        goto __ERROR;
    }
    BT_ALSA_UPL_LOG_NORMAL("set channel %u success", channel);

    BT_ALSA_UPL_LOG_NORMAL("buffer_time=%u buffer_frames=%lu", buffer_time, buffer_frames);

    if ((buffer_time == 0) && (buffer_frames == 0))
    {
        if ((err = snd_pcm_hw_params_get_buffer_time_max(hw_params, &buffer_time, 0)) < 0)
        {
            BT_ALSA_UPL_LOG_ERROR("fail to get max buffer time: %s", snd_strerror(err));
            goto __ERROR;
        }

        BT_ALSA_UPL_LOG_NORMAL("buffer_time:%d", buffer_time);
        if (buffer_time > 500000)
        {
            buffer_time = 500000;
        }
        BT_ALSA_UPL_LOG_NORMAL("buffer_time:%d", buffer_time);
    }

    BT_ALSA_UPL_LOG_NORMAL("period_time=%u period_frames=%lu", period_time, period_frames);
    if ((period_time == 0) && (period_frames == 0))
    {
        if (buffer_time > 0)
        {
            period_time = buffer_time / 4;
        }
        else
        {
            period_frames = buffer_frames / 4;
        }
    }
    if (period_time > 0)
    {
        if ((err = snd_pcm_hw_params_set_period_time_near(capture_handle, hw_params, &period_time, 0)) < 0)
        {
            BT_ALSA_UPL_LOG_ERROR("fail to set period time: %s", snd_strerror(err));
            goto __ERROR;
        }

        BT_ALSA_UPL_LOG_NORMAL("success to set period time: %u", period_time);
    }
    else
    {
        if ((err = snd_pcm_hw_params_set_period_size_near(capture_handle, hw_params, &period_frames, 0)) < 0)
        {
            BT_ALSA_UPL_LOG_ERROR("fail to set period size: %s", snd_strerror(err));
            goto __ERROR;
        }
        BT_ALSA_UPL_LOG_NORMAL("success to set period size: %lu", period_frames);
    }

    if (buffer_time > 0)
    {
        if ((err = snd_pcm_hw_params_set_buffer_time_near(capture_handle, hw_params, &buffer_time, 0)) < 0)
        {
            BT_ALSA_UPL_LOG_ERROR("fail to set buffer time: %s", snd_strerror(err));
            goto __ERROR;
        }
        BT_ALSA_UPL_LOG_NORMAL("success to set buffer time: %u", buffer_time);
    }
    else
    {
        if ((err = snd_pcm_hw_params_set_buffer_size_near(capture_handle, hw_params, &buffer_frames)) < 0)
        {
            BT_ALSA_UPL_LOG_ERROR("fail to set buffer size: %s", snd_strerror(err));
            goto __ERROR;
        }
        BT_ALSA_UPL_LOG_NORMAL("success to set buffer size: %lu", buffer_frames);
    }

    //monotonic = snd_pcm_hw_params_is_monotonic(hw_params);
    BT_ALSA_UPL_LOG_NORMAL("monotonic: %d", snd_pcm_hw_params_is_monotonic(hw_params));

    if ((err = snd_pcm_hw_params(capture_handle, hw_params)) < 0)
    {
        BT_ALSA_UPL_LOG_ERROR("cannot set hw param %s", snd_strerror(err));
        goto __ERROR;
    }

    snd_pcm_hw_params_get_period_size(hw_params, &chunk_size, 0);
    snd_pcm_hw_params_get_buffer_size(hw_params, &buffer_size);
    BT_ALSA_UPL_LOG_NORMAL("chunk_size:%lu, buffer_size:%lu", chunk_size, buffer_size);
    if (chunk_size == buffer_size)
    {
        BT_ALSA_UPL_LOG_ERROR("Can't use period equal to buffer size (%lu == %lu)",
                       chunk_size, buffer_size);
        goto __ERROR;
    }

    snd_pcm_sw_params_current(capture_handle, sw_params);
    if (avail_min < 0)
    {
        n = chunk_size;
    }
    else
    {
        n = (double) u4rate * avail_min / 1000000;
    }

    if ((err = snd_pcm_sw_params_set_avail_min(capture_handle, sw_params, n)) < 0)
    {
        BT_ALSA_UPL_LOG_ERROR("cannot set sw param avail min%s", snd_strerror(err));
        goto __ERROR;
    }

    n = buffer_size;
    start_threshold = (double) u4rate * start_delay / 1000000;
    if (start_threshold < 1)
    {
        start_threshold = 1;
    }
    if (start_threshold > n)
    {
        start_threshold = n;
    }
    BT_ALSA_UPL_LOG_NORMAL("start threshold: %lu", start_threshold);
    if ((err = snd_pcm_sw_params_set_start_threshold(capture_handle, sw_params, start_threshold)) < 0)
    {
        BT_ALSA_UPL_LOG_ERROR("cannot set sw param start threshold %s", snd_strerror(err));
        goto __ERROR;
    }


    stop_threshold = buffer_size + (double) u4rate * stop_delay / 1000000;
    BT_ALSA_UPL_LOG_NORMAL("stop threshold: %lu", stop_threshold);
    if ((err = snd_pcm_sw_params_set_stop_threshold(capture_handle, sw_params, stop_threshold)) < 0)
    {
        BT_ALSA_UPL_LOG_ERROR("cannot set sw param stop threshold %s", snd_strerror(err));
        goto __ERROR;
    }

    if ((err = snd_pcm_sw_params(capture_handle, sw_params)) < 0)
    {
        BT_ALSA_UPL_LOG_ERROR("cannot set sw param %s", snd_strerror(err));
        goto __ERROR;
    }

    bits_per_sample = snd_pcm_format_physical_width(format);
    bits_per_frame = bits_per_sample * channel;
    chunk_bytes = chunk_size * bits_per_frame / 8;

    snd_pcm_sw_params_get_start_threshold(sw_params, &start_threshold);
    snd_pcm_sw_params_get_stop_threshold(sw_params, &stop_threshold);

    BT_ALSA_UPL_LOG_NORMAL("start_threshold:%lu, stop_threshold:%lu", start_threshold, stop_threshold);
    //snd_pcm_hw_params_free(hw_params);
    //snd_pcm_sw_params_free(sw_params);
    hw_params = NULL;
    sw_params = NULL;

    return capture_handle;
__ERROR:
    BT_ALSA_UPL_LOG_NORMAL("open audio device %s fail", pCapDevName);
#if 0
    if (NULL != hw_params)
    {
        snd_pcm_hw_params_free(hw_params);
    }

    if (NULL != sw_params)
    {
        snd_pcm_sw_params_free(sw_params);
    }
#endif

    if (NULL != capture_handle)
    {
        snd_pcm_close(capture_handle);
    }
    return NULL;
}

static INT32 bt_a2dp_alsa_closeCapDev(snd_pcm_t *cap_handle)
{
    int err = 0;
    if (NULL != cap_handle)
    {
        if ((err = snd_pcm_close(cap_handle)) < 0)
        {
            BT_ALSA_UPL_LOG_NORMAL("close cap handle fail %s", snd_strerror(err));
            return -1;
        }
    }
    return 0;
}

/* I/O error handler */
static void bt_a2dp_alsa_xrun(snd_pcm_t *handle)
{
    snd_pcm_status_t *status;
    int res;

    snd_pcm_status_alloca(&status);
    if ((res = snd_pcm_status(handle, status))<0) {
        BT_ALSA_UPL_LOG_ERROR("status error: %s", snd_strerror(res));
        return;
    }

    if (snd_pcm_status_get_state(status) == SND_PCM_STATE_XRUN) {
        if (monotonic) {
#ifdef HAVE_CLOCK_GETTIME
            struct timespec now, diff, tstamp;
            clock_gettime(CLOCK_MONOTONIC, &now);
            snd_pcm_status_get_trigger_htstamp(status, &tstamp);
            timermsub(&now, &tstamp, &diff);
            BT_ALSA_UPL_LOG_ERROR("overrun!!! now(%ld.%09ld), tsamp(%ld.%09ld), (at least %.3f ms long)",
                now.tv_sec, now.tv_nsec,
                tstamp.tv_sec, tstamp.tv_nsec,
                diff.tv_sec * 1000 + diff.tv_nsec / 1000000.0);
            if (diff.tv_sec * 1000 + diff.tv_nsec / 1000000 > 0)
            {
                usleep(diff.tv_sec * 1000000 + diff.tv_nsec / 1000);
            }
#else
            fprintf(stderr, "%s !!!", "underrun");
#endif
        } else {
            struct timeval now, diff, tstamp;

            gettimeofday(&now, 0);
            snd_pcm_status_get_trigger_tstamp(status, &tstamp);
            bt_a2dp_alsa_timersub(&now, &tstamp, &diff);
            BT_ALSA_UPL_LOG_ERROR("overrun!!! now(%ld.%06ld), tsamp(%ld.%06ld), (at least %.3f ms long)",
            now.tv_sec, now.tv_usec,
            tstamp.tv_sec, tstamp.tv_usec,
            diff.tv_sec * 1000 + diff.tv_usec / 1000.0);
            if (diff.tv_sec * 1000 + diff.tv_usec / 10000 > 0)
            {
                usleep(diff.tv_sec * 1000000 + diff.tv_usec);
            }
        }

        if ((res = snd_pcm_prepare(handle))<0) {
            BT_ALSA_UPL_LOG_ERROR("xrun: prepare error: %s", snd_strerror(res));
            return;
        }
        return; /* ok, data should be accepted again */
    }

    if (snd_pcm_status_get_state(status) == SND_PCM_STATE_DRAINING) {
        BT_ALSA_UPL_LOG_ERROR("capture stream format change? attempting recover...");
        if ((res = snd_pcm_prepare(handle))<0) {
            BT_ALSA_UPL_LOG_ERROR("xrun(DRAINING): prepare error: %s", snd_strerror(res));
            return;
        }
        return;
    }
    BT_ALSA_UPL_LOG_ERROR("read/write error, state = %s", snd_pcm_state_name(snd_pcm_status_get_state(status)));
    return;
}

/* I/O suspend handler */
static void bt_a2dp_alsa_suspend(snd_pcm_t *handle)
{
    int res;

    if (!quiet_mode)
        BT_ALSA_UPL_LOG_ERROR("Suspended. Trying resume. ");
    while ((res = snd_pcm_resume(handle)) == -EAGAIN)
    sleep(1); /* wait until suspend flag is released */
    if (res < 0) {
        if (!quiet_mode)
        BT_ALSA_UPL_LOG_ERROR("Failed. Restarting stream. ");
        if ((res = snd_pcm_prepare(handle)) < 0) {
        BT_ALSA_UPL_LOG_ERROR("suspend: prepare error: %s", snd_strerror(res));
        return;
        }
    }
    if (!quiet_mode)
    BT_ALSA_UPL_LOG_ERROR("Done.");
}

#if 0
static void bt_a2dp_alsa_cap_wait_for_data(queue *q, int pcm_bytes)
{
    struct timeval now;
    long current_us = 0;
    int play_us = 0, wait_us = 0;
    static long last_us = 0;
    gettimeofday(&now, NULL);
    current_us = now.tv_sec * 1000000 + now.tv_usec;

    if (0 == last_us)
    {
        last_us = current_us;
        return;
    }

    play_us = 1000 * (1000 * pcm_bytes * 8 / (bits_per_sample * 2 * 48000));

    wait_us = last_us + play_us - current_us;
    //BT_ALSA_UPL_LOG_ERROR("pcm_bytes=%d, wait_us=%d, play_us=%d", pcm_bytes, wait_us, play_us);

    if (0 < wait_us)
    {
#if 1
        if (50000 < wait_us)
        {
            BT_ALSA_UPL_LOG_ERROR("wait more than 50ms %d", wait_us);
        }
        else
        {
            if (queue_get_size(q) >= queue_get_cap(q) * 3 / 4)
            {
                wait_us += 30000;
                BT_ALSA_UPL_LOG_NORMAL("more than 3/4, wait %d ms", wait_us/1000);
                usleep(wait_us);
            }
            else if (queue_get_size(q) >= queue_get_cap(q) / 2)
            {
                wait_us += 20000;
                BT_ALSA_UPL_LOG_NORMAL("more than 1/2, wait %d ms", wait_us/1000);
                usleep(wait_us);
            }
            else if (queue_get_size(q) <= queue_get_cap(q) / 8)
            {
                wait_us = -30000;
                //BT_ALSA_UPL_LOG_NORMAL("less than 1/8");
                if (wait_us > 0)
                {
                    BT_ALSA_UPL_LOG_NORMAL("less than 1/8, wait %d ms", wait_us/1000);
                    usleep(wait_us);
                }
            }
            else
            {
                BT_ALSA_UPL_LOG_NORMAL("wait %d ms", wait_us/1000);
                usleep(wait_us);
            }
        }
#else
        BT_ALSA_UPL_LOG_NORMAL("wait %d ms", wait_us/1000);
        usleep(wait_us);
#endif
    }

#if 0
    if (0 > wait_us)
    {
        last_us = 0;
    }
    else
    {
        last_us += (play_us + wait_us);
    }
#else
    //if (0 < wait_us)
    {
        last_us = current_us;
    }
#endif
    return;
}
#endif

static void* bt_a2dp_alsa_cap_handler(VOID * args)
{
    INT32 i4_ret = -1;
    struct sched_param param;
    INT32 policy;
    int err = 0;
    int has_put = 0;
    char rd_buf[BT_ALSA_CAP_CHUNK_BYTES];
    int rd_cnt = 0;
    char save_buf[BT_ALSA_PUSH_BYTES];
    int save_pos = 0;
    int cp_pos = 0;
    int cp_cnt = 0;
    int left_cnt = 0;
#ifdef BT_ALSA_UPL_DEBUG
    int frames = s_bt_a2dp_read_frames == 0 ? chunk_size : s_bt_a2dp_read_frames;
#else
    int frames = chunk_size;
#endif

#ifdef BT_ALSA_UPL_DEBUG
    struct timeval tv, tv1, diff;
    struct timeval send_tv, send_tv1, send_diff;
#endif
    BT_ALSA_UPL_CTX *upl_ctx = (BT_ALSA_UPL_CTX*)args;

    i4_ret = pthread_getschedparam(pthread_self(), &policy, &param);
    if (0 != i4_ret)
    {
        BT_ALSA_UPL_LOG_ERROR("pthread_getschedparam i4_ret:%d", i4_ret);
    }
    BT_ALSA_UPL_LOG_NORMAL("priority:%ld, Policy:%ld, cap_handle=%p",
        (long)param.sched_priority, (long)policy, upl_ctx->cap_handle);

    prctl(PR_SET_NAME, "alsa_get_pcm_handle", 0, 0, 0);

#if 0
    BT_ALSA_UPL_LOG_NORMAL("snd_pcm_prepare", upl_ctx->cap_handle);
    if ((err = snd_pcm_prepare(upl_ctx->cap_handle)) < 0)
    {
        BT_ALSA_UPL_LOG_ERROR("cannot prepare cap dev %s", snd_strerror(err));

        return NULL;
    }
    BT_ALSA_UPL_LOG_NORMAL("snd_pcm_prepare OK", upl_ctx->cap_handle);
#endif

    while (!upl_ctx->need_quit)
    {
#ifdef BT_ALSA_UPL_DEBUG
        gettimeofday(&tv, NULL);
        gettimeofday(&send_tv, NULL);
#endif
        BT_ALSA_UPL_LOG_MINOR("read cap dev frame=%d", frames);
        if ((err = snd_pcm_readi(upl_ctx->cap_handle, rd_buf, frames)) != frames)
        {
            BT_ALSA_UPL_LOG_ERROR("read cap dev fail %d", err);
            if (-EPIPE == err)
            {
                bt_a2dp_alsa_xrun(upl_ctx->cap_handle);
            }
            else if (-ESTRPIPE == err)
            {
                bt_a2dp_alsa_suspend(upl_ctx->cap_handle);
            }
            else
            {
                if ((err = snd_pcm_prepare(upl_ctx->cap_handle)) < 0)
                {
                    BT_ALSA_UPL_LOG_ERROR("cannot prepare cap dev %s", snd_strerror(err));
                }
            }
            continue;
        }
        rd_cnt = err * bits_per_frame / 8;
#ifdef BT_ALSA_UPL_DEBUG
        gettimeofday(&tv1,NULL);
        bt_a2dp_alsa_timersub(&tv1, &tv, &diff);
        BT_ALSA_UPL_LOG_MINOR("read success frame=%d, bytes=%d, timeval=%dms, q_size=%d",
            err, rd_cnt, diff.tv_sec*1000+diff.tv_usec/1000, queue_get_size(upl_ctx->cap_q));
        if (s_bt_a2dp_dump_enable && s_bt_a2dp_outputPcmSampleFile)
        {
            fwrite (rd_buf, 1, (size_t)rd_cnt, s_bt_a2dp_outputPcmSampleFile);
        }
#else
        BT_ALSA_UPL_LOG_MINOR("read cap dev success frame=%d, bytes=%d", err, rd_cnt);
#endif

        if (save_pos + rd_cnt < s_bt_a2dp_push_bytes)
        {
            memcpy(&save_buf[save_pos], rd_buf, rd_cnt);
            save_pos += rd_cnt;
        }
        else
        {
            left_cnt = save_pos + rd_cnt;
            cp_pos = 0;
            has_put = 0;
            do
            {
                cp_cnt = s_bt_a2dp_push_bytes - save_pos;
                memcpy(&save_buf[save_pos], &rd_buf[cp_pos], cp_cnt);
                left_cnt -= s_bt_a2dp_push_bytes;
                cp_pos += cp_cnt;
#ifdef BT_ALSA_UPL_DEBUG
                gettimeofday(&send_tv1, NULL);
                bt_a2dp_alsa_timersub(&send_tv1, &send_tv, &send_diff);
                BT_ALSA_UPL_LOG_MINOR("push rd_cnt=%d, cp_cnt=%d, cp_pos=%d, left=%d, save_pos=%d, timeval=%dms",
                    rd_cnt, cp_cnt, cp_pos, left_cnt, save_pos, send_diff.tv_sec*1000+send_diff.tv_usec/1000);
                send_tv = send_tv1;
#endif
                err = queue_put(upl_ctx->cap_q, save_buf, s_bt_a2dp_push_bytes);
                if ( 0 > err)
                {
                    BT_ALSA_UPL_LOG_ERROR("enqueue data fail, err=%d", err);
                    break;
                }
                else
                {
                    has_put = 1;
                }

                save_pos = 0;
            } while(left_cnt >=  s_bt_a2dp_push_bytes);

            if (0 < rd_cnt - cp_pos)
            {
                BT_ALSA_UPL_LOG_MINOR("save rd_cnt=%d, cp_pos=%d, left=%d, save_pos=%d",
                    rd_cnt, cp_pos, rd_cnt - cp_pos, save_pos);
                if (rd_cnt - cp_pos < s_bt_a2dp_push_bytes)
                {
                    memcpy(&save_buf[save_pos], &rd_buf[cp_pos], rd_cnt - cp_pos);
                    save_pos += (rd_cnt - cp_pos);
                }
                else
                {
                    BT_ALSA_UPL_LOG_ERROR("left too much %d >= %d", rd_cnt - cp_pos, s_bt_a2dp_push_bytes);
                }
            }
            if (has_put)
            {
                pthread_mutex_lock(&upl_ctx->cap_mutex);
                upl_ctx->enq_data = 1;
                pthread_cond_signal(&upl_ctx->cap_cond);
                pthread_mutex_unlock(&upl_ctx->cap_mutex);
            }
        }

        //bt_a2dp_alsa_cap_wait_for_data(upl_ctx->cap_q, err*bits_per_frame/8);
    }

    pthread_exit(NULL);
    return NULL;
}

static VOID* bt_a2dp_alsa_send_handler(VOID * args)
{
    char buf[BT_ALSA_CAP_CHUNK_BYTES];
    unsigned int size = 0;
    BT_ALSA_UPL_CTX *upl_ctx = (BT_ALSA_UPL_CTX*)args;
    int drop_count = upl_ctx->drop_ms * upl_ctx->sample_rate
        * upl_ctx->channels * 2 / 1000 / s_bt_a2dp_push_bytes;

#ifdef BT_ALSA_UPL_DEBUG
    struct timeval now, diff, send_time;
    gettimeofday(&now, 0);
#endif
    prctl(PR_SET_NAME, "alsa_send_pcm_handle", 0, 0, 0);

#ifdef BT_ALSA_UPL_DEBUG
    usleep(s_bt_a2dp_alsa_send_wait_us);
#endif

    while (1)
    {
        pthread_mutex_lock(&upl_ctx->cap_mutex);
        if ((0 == upl_ctx->enq_data) && (0 == upl_ctx->need_quit))
        {
            pthread_cond_wait(&upl_ctx->cap_cond, &upl_ctx->cap_mutex);
        }
        upl_ctx->enq_data = 0;

        if (1 == upl_ctx->need_quit)
        {
            pthread_mutex_unlock(&upl_ctx->cap_mutex);
            BT_ALSA_UPL_LOG_NORMAL("exit send pcm thread");
            break;
        }

        pthread_mutex_unlock(&upl_ctx->cap_mutex);

        while (!queue_check_queue_empty(upl_ctx->cap_q))
        {
            if (0 > queue_get_unit_size(upl_ctx->cap_q, &size))
            {
                BT_ALSA_UPL_LOG_ERROR("no data in the queue");
                continue;
            }

            queue_get(upl_ctx->cap_q, buf, size);

            if (NULL != s_bt_a2dp_alsa_output_cb)
            {
#ifdef BT_ALSA_UPL_DEBUG
                gettimeofday(&send_time, 0);
                bt_a2dp_alsa_timersub(&send_time, &now, &diff);
                if (diff.tv_sec * 1000 + diff.tv_usec / 1000 > 50)
                {
                    BT_ALSA_UPL_LOG_NORMAL("send data %d overtime %ld",
                        size, diff.tv_sec * 1000 + diff.tv_usec / 1000);
                }
                BT_ALSA_UPL_LOG_MINOR("send data %d timeval=%ld ms, q_size=%d",
                    size, diff.tv_sec * 1000 + diff.tv_usec / 1000, queue_get_size(upl_ctx->cap_q));
                now = send_time;
#endif
                if (drop_count <= 0)
                {
                    s_bt_a2dp_alsa_output_cb(buf, size);
                }
                else
                {
                    drop_count--;
                }
            }
            else
            {
                BT_ALSA_UPL_LOG_ERROR("please register output callback");
                break;
            }
        }
    }

    return NULL;
}




static INT32 bt_a2dp_alsa_startCapThread(BT_ALSA_UPL_CTX *upl_ctx)
{
    INT32 i4_ret = 0;
    pthread_attr_t attr;
    struct sched_param param;
    param.sched_priority = 5;

    i4_ret = pthread_attr_init(&attr);
    if (0 != i4_ret)
    {
        BT_ALSA_UPL_LOG_ERROR("pthread_attr_init i4_ret:%d", i4_ret);
        return -1;
    }
    i4_ret = pthread_attr_setschedpolicy(&attr, SCHED_RR);
    if (0 != i4_ret)
    {
        BT_ALSA_UPL_LOG_ERROR("pthread_attr_setschedpolicy i4_ret:%d", i4_ret);
    }
    i4_ret = pthread_attr_setschedparam(&attr, &param);
    if (0 != i4_ret)
    {
        BT_ALSA_UPL_LOG_ERROR("pthread_attr_setschedparam i4_ret:%d", i4_ret);
    }
    i4_ret = pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED);
    if (0 != i4_ret)
    {
        BT_ALSA_UPL_LOG_ERROR("pthread_attr_setinheritsched i4_ret:%d", i4_ret);
    }

    if (0 != (i4_ret = pthread_create(&upl_ctx->cap_thread,
                                      &attr,
                                      bt_a2dp_alsa_cap_handler,
                                      (void*)upl_ctx)))
    {
        BT_ALSA_UPL_LOG_ERROR("pthread_create i4_ret:%d", i4_ret);
        assert(0);
    }

    pthread_attr_destroy(&attr);
    return 0;
}

static INT32 bt_a2dp_alsa_startSendThread(BT_ALSA_UPL_CTX *upl_ctx)
{
    INT32 i4_ret = 0;
    pthread_attr_t attr;

    i4_ret = pthread_attr_init(&attr);
    if (0 != i4_ret)
    {
        BT_ALSA_UPL_LOG_ERROR("pthread_attr_init i4_ret:%ld", (long)i4_ret);
        return -1;
    }

    if (0 != (i4_ret = pthread_create(&upl_ctx->send_thread,
                                      &attr,
                                      bt_a2dp_alsa_send_handler,
                                      (void*)upl_ctx)))
    {
        BT_ALSA_UPL_LOG_ERROR("pthread_create i4_ret:%d", i4_ret);
        assert(0);
    }

    pthread_attr_destroy(&attr);
    return 0;
}


static INT32 bt_a2dp_alsa_uploader_init(INT32 freq, INT32 channel)
{
    BT_ALSA_UPL_LOG_NORMAL("--- enter ---");
    BT_ALSA_UPL_LOG_NORMAL("freq=%d, channel=%d", freq, channel);
    if (s_bt_a2dp_alsa_upl_ctx.inited)
    {
        BT_ALSA_UPL_LOG_ERROR("inited");
        return 0;
    }

    if (freq == 0)
    {
        freq = 48000;
    }

    if (channel == 0)
    {
        channel = 2;
    }

    if (NULL != s_bt_a2dp_alsa_upl_ctx.cap_handle)
    {
        BT_ALSA_UPL_LOG_ERROR("alsa dev %s is openned", BT_ALSA_CAP_DEV);
        return 0;
    }

    if ((s_bt_a2dp_alsa_upl_ctx.cap_handle = bt_a2dp_alsa_openCapDev(BT_ALSA_CAP_DEV, freq, channel)) == NULL)
    {
        BT_ALSA_UPL_LOG_ERROR("open alsa dev %s fail", BT_ALSA_CAP_DEV);
        return -1;
    }
#ifdef BT_ALSA_UPL_DEBUG
    usleep(s_bt_a2dp_alsa_wait_us);

    BT_ALSA_UPL_LOG_ERROR("open alsa then wait %u ms", s_bt_a2dp_alsa_wait_us);
#endif

    if (NULL == s_bt_a2dp_alsa_upl_ctx.cap_q)
    {
        s_bt_a2dp_alsa_upl_ctx.cap_q = queue_create(BT_ALSA_CAP_QUEUE_SIZE, BT_ALSA_CAP_QUEUE_ITEM_SIZE);
    }

    pthread_cond_init(&s_bt_a2dp_alsa_upl_ctx.cap_cond, NULL);
    pthread_mutex_init(&s_bt_a2dp_alsa_upl_ctx.cap_mutex, NULL);

#ifdef BT_ALSA_UPL_DEBUG
    if (s_bt_a2dp_dump_enable)
    {
        static int file_no = 0;
        char file_name[128];
        sprintf(file_name, "%s%d", s_bt_a2dp_outputFilename, file_no);
        s_bt_a2dp_outputPcmSampleFile = fopen(file_name, "wb+");

        BT_ALSA_UPL_LOG_NORMAL("open dump file %s fd %p", file_name, s_bt_a2dp_outputPcmSampleFile);
    }
#endif
    s_bt_a2dp_alsa_upl_ctx.enq_data = 0;
    s_bt_a2dp_alsa_upl_ctx.need_quit = 0;

    s_bt_a2dp_alsa_upl_ctx.sample_rate = freq;
    s_bt_a2dp_alsa_upl_ctx.channels = channel;
    s_bt_a2dp_alsa_upl_ctx.inited = 1;

    return 0;
}

static INT32 bt_a2dp_alsa_uploader_deinit(VOID)
{
    BT_ALSA_UPL_LOG_NORMAL("--- enter ---");
    if (0 == s_bt_a2dp_alsa_upl_ctx.inited)
    {
        BT_ALSA_UPL_LOG_ERROR("uninited");
        return 0;
    }

    if (NULL != s_bt_a2dp_alsa_upl_ctx.cap_handle)
    {
        queue_destroy(s_bt_a2dp_alsa_upl_ctx.cap_q);
        s_bt_a2dp_alsa_upl_ctx.cap_q = NULL;
        pthread_cond_destroy(&s_bt_a2dp_alsa_upl_ctx.cap_cond);
        pthread_mutex_destroy(&s_bt_a2dp_alsa_upl_ctx.cap_mutex);
        bt_a2dp_alsa_closeCapDev(s_bt_a2dp_alsa_upl_ctx.cap_handle);
        s_bt_a2dp_alsa_upl_ctx.cap_handle = NULL;
    }
#ifdef BT_ALSA_UPL_DEBUG
    if (s_bt_a2dp_outputPcmSampleFile)
    {
        fclose(s_bt_a2dp_outputPcmSampleFile);
    }
    s_bt_a2dp_outputPcmSampleFile = NULL;
#endif
    s_bt_a2dp_alsa_upl_ctx.inited = 0;
    return 0;
}

static INT32 bt_a2dp_alsa_uploader_start(INT32 delay_ms)
{
    BT_ALSA_UPL_LOG_NORMAL("--- enter ---");
    if (0 == s_bt_a2dp_alsa_upl_ctx.inited)
    {
        BT_ALSA_UPL_LOG_ERROR("uninited");
        return 0;
    }

#ifdef BT_ALSA_UPL_DEBUG
    s_bt_a2dp_alsa_upl_ctx.drop_ms = s_bt_a2dp_alsa_drop_us == 0 ? delay_ms : s_bt_a2dp_alsa_drop_us / 1000;
#else
    s_bt_a2dp_alsa_upl_ctx.drop_ms = delay_ms;
#endif

    if (bt_a2dp_alsa_startSendThread(&s_bt_a2dp_alsa_upl_ctx) < 0)
    {
        return -1;
    }

    if (bt_a2dp_alsa_startCapThread(&s_bt_a2dp_alsa_upl_ctx) < 0)
    {
        return -1;
    }

    BT_ALSA_UPL_LOG_NORMAL("--- exit ---");
    return 0;
}

static INT32 bt_a2dp_alsa_uploader_stop(VOID)
{
    BT_ALSA_UPL_LOG_NORMAL("--- enter ---");
    if (0 == s_bt_a2dp_alsa_upl_ctx.inited)
    {
        BT_ALSA_UPL_LOG_ERROR("uninited");
        return 0;
    }

    pthread_mutex_lock(&s_bt_a2dp_alsa_upl_ctx.cap_mutex);
    s_bt_a2dp_alsa_upl_ctx.need_quit = 1;
    pthread_mutex_unlock(&s_bt_a2dp_alsa_upl_ctx.cap_mutex);
    pthread_cond_signal(&s_bt_a2dp_alsa_upl_ctx.cap_cond);

    BT_ALSA_UPL_LOG_NORMAL("--- wait cap thread ---");
    pthread_join(s_bt_a2dp_alsa_upl_ctx.cap_thread, NULL);
    s_bt_a2dp_alsa_upl_ctx.cap_thread = -1;
    BT_ALSA_UPL_LOG_NORMAL("--- cap thread exit---");

    BT_ALSA_UPL_LOG_NORMAL("--- wait send thread ---");
    pthread_join(s_bt_a2dp_alsa_upl_ctx.send_thread, NULL);
    s_bt_a2dp_alsa_upl_ctx.send_thread = -1;
    BT_ALSA_UPL_LOG_NORMAL("--- send thread exit---");

    s_bt_a2dp_alsa_upl_ctx.need_quit = 0;
    BT_ALSA_UPL_LOG_NORMAL("--- exit ---");
    return 0;
}
