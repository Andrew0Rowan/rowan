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

/* FILE NAME:  bt_a2dp_alsa_playback.c
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
#include <unistd.h>
#include <pthread.h>
#include <sys/time.h>

#include "u_bt_mw_common.h"
#include "bt_a2dp_alsa_playback.h"
#include "c_bt_mw_a2dp_snk.h"
#include "osi/include/log.h"
#include "bt_mw_log.h"
#include "bt_mw_common.h"
#include <asoundlib.h>

/* NAMING CONSTANT DECLARATIONS
 */
#define BT_ALSA_PLAYBACK_DETACH_IN_THREAD /* do dsp in one thread */

typedef enum
{
    BT_A2DP_ALSA_PB_STATUS_UNINIT = 0,
    BT_A2DP_ALSA_PB_STATUS_OPENED,
    BT_A2DP_ALSA_PB_STATUS_PLAYED,
    BT_A2DP_ALSA_PB_STATUS_PAUSED,
    BT_A2DP_ALSA_PB_STATUS_STOPED,
    BT_A2DP_ALSA_PB_STATUS_MAX
}BT_A2DP_ALSA_PB_STATUS;

/* MACRO FUNCTION DECLARATIONS
 */

#define ALSA_DBG_MINOR(s, ...) BT_DBG_MINOR(BT_DEBUG_PB, s, ## __VA_ARGS__)

#define ALSA_DBG_INFO(s, ...) BT_DBG_INFO(BT_DEBUG_PB, s, ## __VA_ARGS__)

#define ALSA_DBG_NOTICE(s, ...) BT_DBG_NOTICE(BT_DEBUG_PB, s, ## __VA_ARGS__)

#define ALSA_DBG_NORMAL(s, ...) BT_DBG_NORMAL(BT_DEBUG_PB, s, ## __VA_ARGS__)

#define ALSA_DBG_WARNING(s, ...) BT_DBG_WARNING(BT_DEBUG_PB, s, ## __VA_ARGS__)

#define ALSA_DBG_ERROR(s, ...) BT_DBG_ERROR(BT_DEBUG_PB, s, ## __VA_ARGS__)

#define BT_A2DP_ALSA_PB_REPORT_EVENT(event) do{          \
        if(g_bt_a2dp_alsa_pb_event_cb)                   \
        {                                                \
            ALSA_DBG_NORMAL("report event:%d", event);   \
            g_bt_a2dp_alsa_pb_event_cb(event);           \
        }                                                \
    }while(0)


/* DATA TYPE DECLARATIONS
 */

typedef struct
{
    BOOL fgPlayBackInit;    /* if playback is inited */
    BOOL fgAudioReset;      /* if reset audio        */
}BT_PLAYBACK_CB_T;

/* GLOBAL VARIABLE DECLARATIONS
 */
/* LOCAL SUBPROGRAM DECLARATIONS
 */
/* STATIC VARIABLE DECLARATIONS
 */

static BT_A2DP_PLAYER_EVENT_CB g_bt_a2dp_alsa_pb_event_cb = NULL;
/* EXPORTED SUBPROGRAM BODIES
 */
/* LOCAL SUBPROGRAM BODIES
 */

#ifdef BT_ALSA_PLAYBACK_DETACH_IN_THREAD
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/types.h>

//#define BT_ALSA_PLAYBACK_TEST


//#define BT_ALSA_PLAYBACK_MEM_CHECK

//#define BT_ALSA_PLAYBACK_STAT_SPEED


#define BT_ALSA_PLAYBACK_BUF_SIZE (4096)

/*
when to buffer audio data: once playback audio_status is BT_A2DP_ALSA_PB_STATUS_PLAYED
and audio data is sent to the alsa playback
*/
// #define BT_ALSA_PLAYBACK_ADD_BUFFER
#ifdef BT_ALSA_PLAYBACK_ADD_BUFFER
#define BT_ALSA_PLAYBACK_BUF_CNT  (150)
/*
when audio data come to MW, the first N ms audio data (packets) will be buffered.
At present, the g_buffer_time value should respond to BT_ALSA_PLAYBACK_BUF_CNT.
*/
#define BT_ALSA_PLAYBACK_DELAY_BUF_TIME  (0) // default buffer time if "ADD_BUF" mechanism is enabled
int g_buffer_time = BT_ALSA_PLAYBACK_DELAY_BUF_TIME;
BOOL force_deque = FALSE;
#else
#define BT_ALSA_PLAYBACK_BUF_CNT  (56)  /* it same as btif_media queue size */
#endif

#define BT_ALSA_PLAYBACK_CMD_Q_SIZE (10)
#define BT_ALSA_PLAYBACK_DATA_Q_SIZE BT_ALSA_PLAYBACK_BUF_CNT

#define BT_ALSA_PLAYBACK_CMD_OPEN       (0)
#define BT_ALSA_PLAYBACK_CMD_CLOSE      (1)
#define BT_ALSA_PLAYBACK_CMD_PLAY       (2)
#define BT_ALSA_PLAYBACK_CMD_PAUSE      (3)
#define BT_ALSA_PLAYBACK_CMD_DATA       (4)
#define BT_ALSA_PLAYBACK_CMD_QUIT       (5)


typedef struct
{
    int type;       /* item type */
    union
    {
        struct
        {
            int fs;
            int channels;
        }param;
    }u;
}BT_ALSA_PLAYBACK_QUEUE_ITEM;


typedef struct
{
    unsigned int w_pos; /* write position */
    unsigned int r_pos; /* read position */
    unsigned int w_cnt; /* write counter */
    unsigned int r_cnt; /* read counter */

    unsigned int capacity; /* queue size */
    BT_ALSA_PLAYBACK_QUEUE_ITEM *items; /* queue items */
}BT_ALSA_PLAYBACK_QUEUE;


typedef struct
{
    unsigned char *buf;
    unsigned int data_len;
}BT_ALSA_PLAYBACK_MEM_ITEM;

typedef struct
{
    unsigned int w_pos; /* write position */
    unsigned int r_pos;  /* read position */
    unsigned int w_cnt; /* write counter */
    unsigned int r_cnt;  /* read counter */


    unsigned char *data_mem;
    BT_ALSA_PLAYBACK_MEM_ITEM mem_items[BT_ALSA_PLAYBACK_BUF_CNT];

#ifdef BT_ALSA_PLAYBACK_ADD_BUFFER
    /* only for data queue, to check whether has buffer or not; set it to FALSE when init and A2DP disconnect */
    BOOL buffer_done;
    int total_buffered_data_len;
    int total_buffered_packet_num;
    int fs;
    int channels;
#endif

}BT_ALSA_PLAYBACK_MEM_QUEUE;

typedef struct
{
    int inited;
    int has_q_data;  /* 0-no data in cmd_q/data_q, 1-has data in cmd_q/data_q */
    pthread_t work_thread; /* work thread, process dsp_open/close/write */
    pthread_mutex_t lock;
    pthread_mutex_t mem_lock; /* memory pool lock */
    pthread_cond_t signal;
    BT_ALSA_PLAYBACK_QUEUE *cmd_q;
    BT_ALSA_PLAYBACK_MEM_QUEUE *data_q;
}BT_ALSA_PLAYBACK_CB;

static BT_ALSA_PLAYBACK_CB s_bt_alsa_pb_cb;
static void bt_a2dp_alsa_pb_enq_cmd(BT_ALSA_PLAYBACK_QUEUE_ITEM *cmd);
static int bt_a2dp_alsa_pb_enq_data(unsigned char *data, int len);
static int bt_a2dp_alsa_pb_init(BT_A2DP_PLAYER_EVENT_CB event_cb);
static int bt_a2dp_alsa_pb_deinit(void);

#ifdef BT_ALSA_PLAYBACK_ADD_BUFFER
static void bt_a2dp_alsa_pb_reset_data_queue(void);
static int bt_a2dp_alsa_pb_buffer_time(int data_len, int fs, int channels, int wlength );
#endif /* BT_ALSA_PLAYBACK_ADD_BUFFER */


#ifdef BT_ALSA_PLAYBACK_STAT_SPEED
static int s_bt_alsa_write_start = 0;
static struct timeval s_bt_start_write_tm;
static long s_bt_alsa_write_bytes = 0;
static struct timeval g_last_write_tm;

static int s_bt_alsa_push_start = 0;
static struct timeval s_bt_start_push_tm;
static long s_bt_alsa_push_bytes = 0;
static struct timeval g_last_push_tm;
#endif /* BT_ALSA_PLAYBACK_STAT_SPEED */



#ifdef BT_ALSA_PLAYBACK_TEST
static void bt_a2dp_alsa_pb_test(void);
#endif /* BT_ALSA_PLAYBACK_TEST */

#endif /* BT_ALSA_PLAYBACK_DETACH_IN_THREAD */


#if defined(MTK_BT_PLAYBACK_DEFAULT_ALSA)
#define ALSA_DEVICE_PLAYER "default" /* for MTK branch tree */
#else
#define ALSA_DEVICE_PLAYER "main"
#endif
#define FRAGMENT_SAMPLES    (4096*4)

static snd_pcm_t *s_alsa_handle = NULL;

static snd_pcm_uframes_t chunk_size = 0;

static UINT32 u4buffer_time = 500000; /* ring buffer length in us */
static UINT32 u4period_time = 1000; /* period time in us */

static snd_pcm_uframes_t period_frames = 0;
static snd_pcm_uframes_t buffer_frames = 0;
static INT32 i4avail_min = -1;
static INT32 i4start_delay = 200000;
static INT32 i4stop_delay = 0;

BT_PLAYBACK_CB_T g_bt_playback_cb = {0};
BT_A2DP_ALSA_PB_STATUS audio_status = BT_A2DP_ALSA_PB_STATUS_UNINIT;
static BT_A2DP_PLAYER g_bt_a2dp_alsa_player = {0};

INT32 bt_a2dp_alsa_player_start(INT32 trackFreq, INT32 channelType);
INT32 bt_a2dp_alsa_player_play(VOID);
INT32 bt_a2dp_alsa_player_pause(VOID);
INT32 bt_a2dp_alsa_player_stop(VOID);
INT32 bt_a2dp_alsa_player_write(VOID *data, INT32 datalen);
INT32 bt_a2dp_alsa_player_adjust_buf_time(UINT32 buffer_time);

EXPORT_SYMBOL INT32 bt_a2dp_alsa_player_adjust_buf_time(UINT32 buffer_time)
{
#ifdef BT_ALSA_PLAYBACK_ADD_BUFFER

    if (1 == s_bt_alsa_pb_cb.inited)
    {
        if (buffer_time > 1000)
        {
            ALSA_DBG_ERROR("<addbuf> buffer too long time!");
            return 0;
        }
        g_buffer_time = buffer_time;
        ALSA_DBG_NORMAL("<addbuf> adjust g_buffer_time:%d ms", g_buffer_time);
    }
    else
    {
        ALSA_DBG_ERROR("<addbuf> playback not init!");
    }
#else
    ALSA_DBG_ERROR("<addbuf> ADD_BUFFER is disabled!");
#endif
    return 0;
}

EXPORT_SYMBOL BT_A2DP_PLAYER* bt_a2dp_alsa_get_player(VOID)
{
    g_bt_a2dp_alsa_player.init = bt_a2dp_alsa_pb_init;
    g_bt_a2dp_alsa_player.deinit = bt_a2dp_alsa_pb_deinit;
    g_bt_a2dp_alsa_player.start = bt_a2dp_alsa_player_start;
    g_bt_a2dp_alsa_player.stop = bt_a2dp_alsa_player_stop;
    g_bt_a2dp_alsa_player.play = bt_a2dp_alsa_player_play;
    g_bt_a2dp_alsa_player.pause = bt_a2dp_alsa_player_pause;
    g_bt_a2dp_alsa_player.write = bt_a2dp_alsa_player_write;
    g_bt_a2dp_alsa_player.adjust_buf_time = bt_a2dp_alsa_player_adjust_buf_time;

#ifdef BT_ALSA_PLAYBACK_TEST
    sleep(1);
    bt_a2dp_alsa_pb_test();
#endif

    return &g_bt_a2dp_alsa_player;
}


static INT32 set_params(INT32 fs, INT32 channel_num)
{
    ALSA_DBG_NORMAL("+++into");
    INT32 i4ret;
    size_t n;
    UINT32 u4rate;
    snd_pcm_uframes_t start_threshold;
    snd_pcm_uframes_t stop_threshold;
    snd_pcm_uframes_t buffer_size;
    snd_pcm_hw_params_t *hwparams;
    snd_pcm_sw_params_t *swparams;

    snd_pcm_hw_params_alloca(&hwparams);
    snd_pcm_sw_params_alloca(&swparams);

    /* choose all parameters */
    i4ret = snd_pcm_hw_params_any(s_alsa_handle, hwparams);
    if (i4ret < 0)
    {
        ALSA_DBG_ERROR("Broken configuration for playback: no configurations available: %s", snd_strerror(i4ret));
        return i4ret;
    }
    /* set the sample format */
    i4ret = snd_pcm_hw_params_set_format(s_alsa_handle, hwparams, SND_PCM_FORMAT_S16_LE);
    if (i4ret < 0)
    {
        ALSA_DBG_ERROR("Sample format not available for playback: %s", snd_strerror(i4ret));
        return i4ret;
    }
    /* set the interleaved read/write format */
    i4ret = snd_pcm_hw_params_set_access(s_alsa_handle, hwparams, SND_PCM_ACCESS_RW_INTERLEAVED);
    if (i4ret < 0)
    {
        ALSA_DBG_ERROR("Access type not available for playback: %s", snd_strerror(i4ret));
        return i4ret;
    }
    /* set the count of channels */
    i4ret = snd_pcm_hw_params_set_channels(s_alsa_handle, hwparams, channel_num);
    if (i4ret < 0)
    {
        ALSA_DBG_ERROR("Channels count (%i) not available for playbacks: %s", channel_num, snd_strerror(i4ret));
        return i4ret;
    }
    /* set the stream sampling rate */
    i4ret = snd_pcm_hw_params_set_rate(s_alsa_handle, hwparams, fs, 0);
    if (i4ret < 0)
    {
        ALSA_DBG_ERROR("Rate %iHz not available for playback: %s", fs, snd_strerror(i4ret));
        return i4ret;
    }

    u4rate = fs;
    if ((u4buffer_time == 0) && (buffer_frames == 0))
    {
        i4ret = snd_pcm_hw_params_get_buffer_time_max(hwparams, &u4buffer_time, 0);
        if (i4ret < 0)
        {
            ALSA_DBG_ERROR("fail to get max buffer time:%d, %s", i4ret, snd_strerror(i4ret));
            return i4ret;
        }
        ALSA_DBG_NORMAL("u4buffer_time:%d", u4buffer_time);
        if (u4buffer_time > 500000)
        {
            u4buffer_time = 500000;
        }
    }
    if ((u4period_time == 0) && (period_frames == 0))
    {
        if (u4buffer_time > 0)
        {
            u4period_time = u4buffer_time / 4;
        }
        else
        {
            period_frames = buffer_frames / 4;
        }
    }
    if (u4period_time > 0)
    {
        i4ret = snd_pcm_hw_params_set_period_time_near(s_alsa_handle, hwparams,
                &u4period_time, 0);
    }
    else
    {
        i4ret = snd_pcm_hw_params_set_period_size_near(s_alsa_handle, hwparams,
                &period_frames, 0);
    }
    if (i4ret < 0)
    {
        ALSA_DBG_ERROR("fail to get period size:%d, %s", i4ret, snd_strerror(i4ret));
        return i4ret;
    }
    if (u4buffer_time > 0)
    {
        i4ret = snd_pcm_hw_params_set_buffer_time_near(s_alsa_handle, hwparams,
                &u4buffer_time, 0);
    }
    else
    {
        i4ret = snd_pcm_hw_params_set_buffer_size_near(s_alsa_handle, hwparams,
                &buffer_frames);
    }
    if (i4ret < 0)
    {
        ALSA_DBG_ERROR("fail to get buffer size:%d, %s", i4ret, snd_strerror(i4ret));
        return i4ret;
    }

    i4ret = snd_pcm_hw_params(s_alsa_handle, hwparams);
    if (i4ret < 0)
    {
        ALSA_DBG_ERROR("Unable to install hw params");
        return i4ret;
    }

    snd_pcm_hw_params_get_period_size(hwparams, &chunk_size, 0);
    snd_pcm_hw_params_get_buffer_size(hwparams, &buffer_size);
    ALSA_DBG_NORMAL("chunk_size:%lu, buffer_size:%lu", chunk_size, buffer_size);
    if (chunk_size == buffer_size)
    {
        ALSA_DBG_ERROR("Can't use period equal to buffer size (%lu == %lu)",
                       chunk_size, buffer_size);
        return i4ret;
    }

    /* get the current swparams */
    snd_pcm_sw_params_current(s_alsa_handle, swparams);
    if (i4avail_min < 0)
    {
        n = chunk_size;
    }
    else
    {
        n = (double) u4rate * i4avail_min / 1000000;
    }
    i4ret = snd_pcm_sw_params_set_avail_min(s_alsa_handle, swparams, n);

    /* round up to closest transfer boundary */
    n = buffer_size;
    if (i4start_delay <= 0)
    {
        start_threshold = n + (double) u4rate * i4start_delay / 1000000;
    }
    else
    {
        start_threshold = (double) u4rate * i4start_delay / 1000000;
    }
    if (start_threshold < 1)
    {
        start_threshold = 1;
    }
    if (start_threshold > n)
    {
        start_threshold = n;
    }
    /* start the transfer when the buffer is almost full: */
    /* (buffer_size / avail_min) * avail_min */
    i4ret = snd_pcm_sw_params_set_start_threshold(s_alsa_handle, swparams, start_threshold);
    if (i4ret < 0)
    {
        ALSA_DBG_ERROR("fail to set start threshold:%d, %s", i4ret, snd_strerror(i4ret));
        return i4ret;
    }
    if (i4stop_delay <= 0)
    {
        stop_threshold = buffer_size + (double) u4rate * i4stop_delay / 1000000;
    }
    else
    {
        stop_threshold = (double) u4rate * i4stop_delay / 1000000;
    }
    i4ret = snd_pcm_sw_params_set_stop_threshold(s_alsa_handle, swparams, stop_threshold);
    if (i4ret < 0)
    {
        ALSA_DBG_ERROR("fail to set stop threshold:%d, %s", i4ret, snd_strerror(i4ret));
        return i4ret;
    }

    /* write the parameters to the playback device */
    if ((i4ret = snd_pcm_sw_params(s_alsa_handle, swparams)) < 0)
    {
        ALSA_DBG_ERROR("unable to install sw params");
        return i4ret;
    }

    snd_pcm_sw_params_get_start_threshold(swparams, &start_threshold);
    snd_pcm_sw_params_get_stop_threshold(swparams, &stop_threshold);
    ALSA_DBG_NORMAL("start_threshold:%lu, stop_threshold:%lu", start_threshold, stop_threshold);
    //snd_pcm_hw_params_free(hwparams);
    //snd_pcm_sw_params_free(swparams);
    ALSA_DBG_NORMAL("---exit");
    return 0;
}

static INT32 dsp_open(INT32 fs, INT32 channel_num)
{
    ALSA_DBG_NORMAL("+++into");
    INT32 i4ret = 0;

    //snd_pcm_hw_params_t *hwparams;

    if (s_alsa_handle != NULL)
    {
        ALSA_DBG_WARNING("---exit already opened s_alsa_handle");
        return 0;
    }

    i4ret = snd_pcm_open(&s_alsa_handle, ALSA_DEVICE_PLAYER, SND_PCM_STREAM_PLAYBACK, 0 );
    ALSA_DBG_NORMAL("fs %d, channel num %d ", fs, channel_num);
    ALSA_DBG_NORMAL("dsp_open %s i4ret=%d[%s]", ALSA_DEVICE_PLAYER, i4ret, snd_strerror(i4ret));
    if (i4ret < 0)
    {
        ALSA_DBG_ERROR("Cannot open %s ERROR %d[%s]", ALSA_DEVICE_PLAYER, i4ret, snd_strerror(i4ret));
        s_alsa_handle = NULL;
        return  -1;
    }
    BT_A2DP_ALSA_PB_REPORT_EVENT(BT_A2DP_ALSA_PB_EVENT_START);

    set_params(fs, channel_num);

#if 0

    snd_pcm_hw_params_malloc(&hwparams);
    /* choose all parameters */
    i4ret = snd_pcm_hw_params_any(s_alsa_handle, hwparams);
    if (i4ret < 0)
    {
        ALSA_DBG_ERROR("Broken configuration for playback: no configurations available: %s", snd_strerror(i4ret));
        return i4ret;
    }
    /* set the sample format */
    i4ret = snd_pcm_hw_params_set_format(s_alsa_handle, hwparams, SND_PCM_FORMAT_S16_LE);
    if (i4ret < 0)
    {
        ALSA_DBG_ERROR("Sample format not available for playback: %s", snd_strerror(i4ret));
        return i4ret;
    }
    /* set the interleaved read/write format */
    i4ret = snd_pcm_hw_params_set_access(s_alsa_handle, hwparams, SND_PCM_ACCESS_RW_INTERLEAVED);
    if (i4ret < 0)
    {
        ALSA_DBG_ERROR("Access type not available for playback: %s", snd_strerror(i4ret));
        return i4ret;
    }
    /* set the count of channels */
    i4ret = snd_pcm_hw_params_set_channels(s_alsa_handle, hwparams, 2);
    if (i4ret < 0)
    {
        ALSA_DBG_ERROR("Channels count (%i) not available for playbacks: %s", channel_num, snd_strerror(i4ret));
        return i4ret;
    }
    /* set the stream sampling u4rate */
    i4ret = snd_pcm_hw_params_set_rate(s_alsa_handle, hwparams, fs, 0);
    if (i4ret < 0)
    {
        ALSA_DBG_ERROR("Rate %iHz not available for playback: %s", fs, snd_strerror(i4ret));
        return i4ret;
    }
    /* set the buffer time */
    snd_pcm_hw_params_set_buffer_time_near(s_alsa_handle, hwparams, &u4buffer_time, 0);
    if (i4ret < 0)
    {
        ALSA_DBG_ERROR("Rate %iHz not available for playback: %s", fs, snd_strerror(i4ret));
        return i4ret;
    }
    /* set the period time */
    snd_pcm_hw_params_set_period_time_near(s_alsa_handle, hwparams, &u4period_time, 0);
    if (i4ret < 0)
    {
        ALSA_DBG_ERROR("Rate %iHz not available for playback: %s", fs, snd_strerror(i4ret));
        return i4ret;
    }

    //snd_pcm_hw_params_set_buffer_size(s_alsa_handle, hwparams, FRAGMENT_SAMPLES * 4);
    //snd_pcm_hw_params_set_period_size(s_alsa_handle, hwparams, 10, 0);
    snd_pcm_hw_params(s_alsa_handle, hwparams);
    snd_pcm_hw_params_free(hwparams);
#endif

    snd_pcm_prepare(s_alsa_handle);
    audio_status = BT_A2DP_ALSA_PB_STATUS_OPENED;

    ALSA_DBG_NORMAL("---exit");
    return 0;
}

static INT32 dsp_write(UINT8 *buf, UINT32 size)
{
    INT32 i4ret = 0;

    if (s_alsa_handle == NULL)
    {
        ALSA_DBG_ERROR("s_alsa_handle == NULL");
        return -1;
    }

    i4ret = snd_pcm_writei(s_alsa_handle, buf, size/4);
    if (i4ret < 0)
    {
        ALSA_DBG_ERROR("ALSA ERROR %d[%s]", i4ret, snd_strerror(i4ret));
        snd_pcm_prepare(s_alsa_handle);
        if ((i4ret = snd_pcm_prepare(s_alsa_handle))<0)
        {
            ALSA_DBG_ERROR("ALSA snd_pcm_prepare ERROR %d[%s]", i4ret, snd_strerror(i4ret));
        }
    }
    ALSA_DBG_MINOR("alsa write i4ret = %d", i4ret);
    return i4ret;
}

static INT32 dsp_close(VOID)
{
    INT32 i4ret = 0;
    ALSA_DBG_NORMAL("+++into");
    if (s_alsa_handle == NULL)
    {
        ALSA_DBG_ERROR("---exit s_alsa_handle == NULL");
        return -1;
    }
    if (s_alsa_handle != NULL)
    {
        i4ret = snd_pcm_close(s_alsa_handle);
        if (i4ret == 0)
        {
            ALSA_DBG_NORMAL("dsp_close success");
            BT_A2DP_ALSA_PB_REPORT_EVENT(BT_A2DP_ALSA_PB_EVENT_STOP);
        }
        else
        {
            ALSA_DBG_ERROR("dsp_close fail i4ret=%d[%s]", i4ret, snd_strerror(i4ret));
        }
        s_alsa_handle = NULL;
    }
    audio_status = BT_A2DP_ALSA_PB_STATUS_UNINIT;


    ALSA_DBG_NORMAL("---exit");
    return i4ret;
}

INT32 bt_a2dp_alsa_player_start(INT32 trackFreq, INT32 channelNum)
{
#ifdef BT_ALSA_PLAYBACK_DETACH_IN_THREAD
    BT_ALSA_PLAYBACK_QUEUE_ITEM cmd = {0};
    ALSA_DBG_NORMAL("+++into");

    memset((void*)&cmd, 0, sizeof(cmd));
    cmd.type = BT_ALSA_PLAYBACK_CMD_OPEN;
    cmd.u.param.fs = trackFreq;
    cmd.u.param.channels = channelNum;
    bt_a2dp_alsa_pb_enq_cmd(&cmd);

    return BT_SUCCESS;
#else
    INT32 i4_ret = BT_SUCCESS;

    if (FALSE == g_bt_playback_cb.fgPlayBackInit)
    {
        i4_ret = dsp_open(trackFreq, channelNum);
        g_bt_playback_cb.fgPlayBackInit = TRUE;
    }
    else
    {
        ALSA_DBG_WARNING("BT playback have init,no need init again");
    }
    return i4_ret;
#endif
}

INT32 bt_a2dp_alsa_player_stop(VOID)
{
#ifdef BT_ALSA_PLAYBACK_DETACH_IN_THREAD
    BT_ALSA_PLAYBACK_QUEUE_ITEM cmd = {0};
    ALSA_DBG_NORMAL("+++into");

    memset((void*)&cmd, 0, sizeof(cmd));
    cmd.type = BT_ALSA_PLAYBACK_CMD_CLOSE;
    bt_a2dp_alsa_pb_enq_cmd(&cmd);
    return BT_SUCCESS;
#else
    ALSA_DBG_NORMAL("+++into");
    INT32 i4_ret = BT_SUCCESS;

    if (TRUE == g_bt_playback_cb.fgPlayBackInit)
    {
        g_bt_playback_cb.fgPlayBackInit = FALSE;
        i4_ret = dsp_close();
    }
    else
    {
        ALSA_DBG_WARNING("BT playback have not init,no need deinit");
    }

    ALSA_DBG_NORMAL("---exit");
    return i4_ret;
#endif
}

static INT32 bluetooth_a2dp_pb_write_data(UINT8 *pPcmBuf, UINT32 u4PcmLen)
{
    if ((NULL == pPcmBuf) || (0 >= u4PcmLen))
    {
        ALSA_DBG_ERROR("invalid data");
        return BT_ERR_STATUS_PARM_INVALID;
    }

#ifdef BT_ALSA_PLAYBACK_STAT_SPEED
    if (1 == s_bt_alsa_write_start)
    {
        s_bt_alsa_write_start = 2;
        gettimeofday(&s_bt_start_write_tm,NULL);
        s_bt_alsa_write_bytes = 0;
    }
    else if (2 == s_bt_alsa_write_start)
    {
        s_bt_alsa_write_bytes += u4PcmLen;
        gettimeofday(&g_last_write_tm,NULL);
    }
#endif
    ALSA_DBG_MINOR("send %ld data", (long)u4PcmLen);
    dsp_write(pPcmBuf, u4PcmLen);

    return BT_SUCCESS;
}

INT32 bt_a2dp_alsa_player_play(VOID)
{
#ifdef BT_ALSA_PLAYBACK_DETACH_IN_THREAD
    BT_ALSA_PLAYBACK_QUEUE_ITEM cmd = {0};
    ALSA_DBG_NORMAL("+++into");

    memset((void*)&cmd, 0, sizeof(cmd));
    cmd.type = BT_ALSA_PLAYBACK_CMD_PLAY;
    bt_a2dp_alsa_pb_enq_cmd(&cmd);
    return BT_SUCCESS;
#else
    if (FALSE == g_bt_playback_cb.fgPlayBackInit)
    {
        ALSA_DBG_NOTICE("BT playback have not init");
        return BT_SUCCESS;
    }
    if ((audio_status != BT_A2DP_ALSA_PB_STATUS_OPENED)
            && (audio_status != BT_A2DP_ALSA_PB_STATUS_PAUSED)
            && (audio_status != BT_A2DP_ALSA_PB_STATUS_STOPED))
    {
        ALSA_DBG_NORMAL("BT AUDIO wrong status, current status = %d", audio_status);
    }
    else
    {
        audio_status = BT_A2DP_ALSA_PB_STATUS_PLAYED;
        ALSA_DBG_NORMAL("BT AUDIO current status = %d", audio_status);
    }

    return BT_SUCCESS;
#endif
}

INT32 bt_a2dp_alsa_player_pause(VOID)
{
#ifdef BT_ALSA_PLAYBACK_DETACH_IN_THREAD
    BT_ALSA_PLAYBACK_QUEUE_ITEM cmd = {0};
    ALSA_DBG_NORMAL("+++into");

    memset((void*)&cmd, 0, sizeof(cmd));
    cmd.type = BT_ALSA_PLAYBACK_CMD_PAUSE;
    bt_a2dp_alsa_pb_enq_cmd(&cmd);
    return BT_SUCCESS;
#else
    ALSA_DBG_NORMAL("+++into");

    if (FALSE == g_bt_playback_cb.fgPlayBackInit)
    {
        ALSA_DBG_NOTICE("BT playback have not init");
        return BT_SUCCESS;
    }
    if (audio_status != BT_A2DP_ALSA_PB_STATUS_PLAYED)
    {
        ALSA_DBG_NOTICE("BT AUDIO wrong status, current status = %d", audio_status);
    }
    else
    {
        audio_status = BT_A2DP_ALSA_PB_STATUS_PAUSED;
        ALSA_DBG_NORMAL("BT AUDIO current status = %d", audio_status);
    }

    return BT_SUCCESS;
#endif
}

INT32 bt_a2dp_alsa_player_write(VOID *data, INT32 datalen)
{
#ifdef BT_ALSA_PLAYBACK_DETACH_IN_THREAD
    return bt_a2dp_alsa_pb_enq_data(data, datalen);
#else
    UINT32 u4PcmLen = datalen;
    UINT8 *pu1PcmBuf = (UINT8 *)data;
    INT32 i4_ret = BT_SUCCESS;

    if (NULL == data || 0 == datalen)
    {
        ALSA_DBG_ERROR("data is null(%p) or data len=%d", data, datalen);
        return BT_ERR_STATUS_FAIL;
    }

    ALSA_DBG_INFO("data=%p, datalen=%u", data, datalen);

    if (FALSE == g_bt_playback_cb.fgPlayBackInit)
    {
        ALSA_DBG_NOTICE("bt playback does not init");
        return i4_ret;
    }

    if (TRUE == g_bt_playback_cb.fgAudioReset)
    {
        ALSA_DBG_NOTICE("audio_reset");
        return i4_ret;
    }

    if (audio_status != BT_A2DP_ALSA_PB_STATUS_PLAYED)
    {
        ALSA_DBG_NOTICE("audio_status is not played %d", audio_status);
        return i4_ret;
    }

    if (BT_SUCCESS != bluetooth_a2dp_pb_write_data(pu1PcmBuf, u4PcmLen))
    {
        ALSA_DBG_ERROR("play pcm failed, pcmbuf:%p, len:%lu", pu1PcmBuf, (long)u4PcmLen);
        return i4_ret;
    }

    return i4_ret;
#endif
}

/******************************************************************************
 */



#ifdef BT_ALSA_PLAYBACK_DETACH_IN_THREAD

static BT_ALSA_PLAYBACK_QUEUE* bt_a2dp_alsa_pb_alloc_q(int capacity)
{
    BT_ALSA_PLAYBACK_QUEUE* q = NULL;
    q = (BT_ALSA_PLAYBACK_QUEUE *)malloc(sizeof(BT_ALSA_PLAYBACK_QUEUE));
    if (NULL == q)
    {
        ALSA_DBG_ERROR("allocate queue fail");
        return NULL;
    }

    q->w_pos = 0;
    q->r_pos = 0;
    q->w_cnt = 0;
    q->r_cnt = 0;
    q->capacity = capacity;
    q->items = (BT_ALSA_PLAYBACK_QUEUE_ITEM *)malloc(sizeof(BT_ALSA_PLAYBACK_QUEUE_ITEM)*capacity);
    if (NULL == q->items)
    {
        free(q);
        ALSA_DBG_ERROR("allocate queue items fail");
        return NULL;
    }

    return q;
}

static void bt_a2dp_alsa_pb_free_q(BT_ALSA_PLAYBACK_QUEUE *q)
{
    if (NULL == q)
    {
        return;
    }

    if (NULL != q->items)
    {
        free(q->items);
        q->items = NULL;
    }

    free(q);

    return;
}

static int bt_a2dp_alsa_pb_enq(BT_ALSA_PLAYBACK_QUEUE *q, BT_ALSA_PLAYBACK_QUEUE_ITEM *items)
{
    if (NULL == q)
    {
        return -1;
    }

    if ((q->w_cnt - q->r_cnt) >= q->capacity)
    {
        ALSA_DBG_ERROR("queue is full, w_cnt=%u, r_cnt=%u, capcity=%u",
            q->w_cnt, q->r_cnt, q->capacity);
        return -1;
    }

    q->items[q->w_pos] = *items;
    q->w_pos++;
    q->w_cnt++;
    if (q->w_pos >= q->capacity)
    {
        q->w_pos = 0;
    }
    return 0;
}

static int bt_a2dp_alsa_pb_deq(BT_ALSA_PLAYBACK_QUEUE *q, BT_ALSA_PLAYBACK_QUEUE_ITEM *items)
{
    if (NULL == q)
    {
        return -1;
    }

    if (q->w_cnt == q->r_cnt)
    {
        //ALSA_DBG_ERROR("queue is empty, w_cnt=%u, r_cnt=%u, capcity=%u",
        //    q->w_cnt, q->r_cnt, q->capacity);
        return -1;
    }

    *items = q->items[q->r_pos];
    q->r_pos++;
    q->r_cnt++;
    if (q->r_pos >= q->capacity)
    {
        q->r_pos = 0;
    }

    return 0;
}

#ifdef BT_ALSA_PLAYBACK_ADD_BUFFER
static void bt_a2dp_alsa_pb_reset_data_queue(void)
{
    ALSA_DBG_NORMAL("<addbuf> --enter %s", __FUNCTION__);

    int i = 0;

    if (NULL == s_bt_alsa_pb_cb.data_q)
    {
        ALSA_DBG_ERROR("<addbuf> data_q is NULL");
        return;
    }
    force_deque = FALSE;
    s_bt_alsa_pb_cb.data_q->buffer_done = FALSE;
    s_bt_alsa_pb_cb.data_q->total_buffered_data_len = 0;
    s_bt_alsa_pb_cb.data_q->total_buffered_packet_num= 0;

    bt_a2dp_alsa_pb_reset_mem_q(s_bt_alsa_pb_cb.data_q);

    ALSA_DBG_NORMAL("<addbuf> --exit %s", __FUNCTION__);
}

static int bt_a2dp_alsa_pb_buffer_time(int data_len, int fs, int channels, int wlength )
{
    int total_buffered_time = 0; // ms
    total_buffered_time = (1000 * data_len /
                        (fs * channels * wlength));
    if (total_buffered_time == 0)
    {
        ALSA_DBG_WARNING("<addbuf> warning!!! data_len:%d, fs:%d, channels:%d", data_len, fs, channels);
    }
    return total_buffered_time;
}
#endif

#ifdef BT_ALSA_PLAYBACK_MEM_CHECK
static void bt_a2dp_alsa_pb_mem_pool_check(BT_ALSA_PLAYBACK_MEM_NODE **mem_pool)
{
    BT_ALSA_PLAYBACK_MEM_NODE *node = NULL;
    char *temp_byte = 0;
    int i = 0;
    if ((NULL == mem_pool) || (NULL == *mem_pool))
    {
        return;
    }

    node = *mem_pool;
    while(NULL != node)
    {
        if (node < s_bt_alsa_pb_cb.data_mem
            || node >= s_bt_alsa_pb_cb.data_mem + BT_ALSA_PLAYBACK_BUF_SIZE*BT_ALSA_PLAYBACK_BUF_CNT)
        {
            ALSA_DBG_ERROR("error data addr: %p over scope(%p~%p)", node,
                s_bt_alsa_pb_cb.data_mem, s_bt_alsa_pb_cb.data_mem
                + BT_ALSA_PLAYBACK_BUF_SIZE*BT_ALSA_PLAYBACK_BUF_CNT);
            assert("error data");
        }
#if 0
        temp_byte = (char*)(node + 1);
        for (i=0;i<BT_ALSA_PLAYBACK_BUF_SIZE-sizeof(*node);i++)
        {
            if(temp_byte[i] != 0xaa)
            {
                ALSA_DBG_ERROR("error data[%d] %x", i, temp_byte[i]);
                assert("error data");
            }
        }
#endif
        node = node->next;
    }
}

#endif


static void bt_a2dp_alsa_pb_enq_mem(BT_ALSA_PLAYBACK_MEM_QUEUE *mem_q,
    unsigned char *data, unsigned int data_len)
{
    if (NULL == mem_q || NULL == data || 0 == data_len)
    {
        ALSA_DBG_ERROR("mem_q(%p) or data(%p) or data_len(%u) is invalid",
            mem_q, data, data_len);
        return;
    }

    if ((mem_q->w_cnt - mem_q->r_cnt) >= BT_ALSA_PLAYBACK_BUF_CNT)
    {
        ALSA_DBG_ERROR("queue is full, w_cnt=%u, r_cnt=%u, capcity=%u",
            mem_q->w_cnt, mem_q->r_cnt, BT_ALSA_PLAYBACK_BUF_CNT);
        return;
    }

    if (mem_q->w_pos >= BT_ALSA_PLAYBACK_BUF_CNT)
    {
        ALSA_DBG_ERROR("invalid w_pos=%u", mem_q->w_pos);
        return;
    }

    if (NULL == mem_q->mem_items[mem_q->w_pos].buf)
    {
        ALSA_DBG_ERROR("mem_items[%u] buff null", mem_q->w_pos);
        return;
    }

    if ( data_len > BT_ALSA_PLAYBACK_BUF_SIZE)
    {
        ALSA_DBG_ERROR("data_len =%u to large", data_len);
        data_len = BT_ALSA_PLAYBACK_BUF_SIZE;
    }


    memcpy(mem_q->mem_items[mem_q->w_pos].buf, data, data_len);
    mem_q->mem_items[mem_q->w_pos].data_len = data_len;
    ALSA_DBG_MINOR("mem_items[%d], buf=%p, data_len =%u ", mem_q->w_pos,
        mem_q->mem_items[mem_q->w_pos].buf, data_len);

    mem_q->w_pos++;
    mem_q->w_cnt++;
    if (mem_q->w_pos >= BT_ALSA_PLAYBACK_BUF_CNT)
    {
        mem_q->w_pos = 0;
    }

    return;
}



static int bt_a2dp_alsa_pb_deq_mem(BT_ALSA_PLAYBACK_MEM_QUEUE *mem_q)
{
    if (NULL == mem_q)
    {
        ALSA_DBG_ERROR("mem_q(%p) is invalid", mem_q);
        return -1;
    }

    if (mem_q->w_cnt == mem_q->r_cnt)
    {
        return -1;
    }

    mem_q->r_pos++;
    mem_q->r_cnt++;
    if (mem_q->r_pos >= BT_ALSA_PLAYBACK_BUF_CNT)
    {
        mem_q->r_pos = 0;
    }

    return 0;
}

static int bt_a2dp_alsa_pb_peak_front_mem(BT_ALSA_PLAYBACK_MEM_QUEUE *mem_q,
    BT_ALSA_PLAYBACK_MEM_ITEM *mem_item)
{
    if (NULL == mem_q || NULL == mem_item)
    {
        ALSA_DBG_ERROR("mem_q(%p) or mem_item(%p) is invalid", mem_q, mem_item);
        return -1;
    }

    if (mem_q->w_cnt == mem_q->r_cnt)
    {
        return -1;
    }

    *mem_item = mem_q->mem_items[mem_q->r_pos];

    ALSA_DBG_MINOR("mem_items[%d], buf=%p, data_len =%u ", mem_q->r_pos,
        mem_item->buf, mem_item->data_len);

    return 0;
}


static void bt_a2dp_alsa_pb_reset_mem_q(BT_ALSA_PLAYBACK_MEM_QUEUE *mem_q)
{
    if (NULL == mem_q)
    {
        return;
    }

    while(0 == bt_a2dp_alsa_pb_deq_mem(mem_q));

    return;
}

static BT_ALSA_PLAYBACK_MEM_QUEUE * bt_a2dp_alsa_pb_alloc_mem_q(void)
{
    BT_ALSA_PLAYBACK_MEM_QUEUE *tmp_mem_q = NULL;
    int i = 0;

    tmp_mem_q = (BT_ALSA_PLAYBACK_MEM_QUEUE *)malloc(sizeof(BT_ALSA_PLAYBACK_MEM_QUEUE));
    if (NULL == tmp_mem_q)
    {
        ALSA_DBG_NORMAL("alloc buf fail");
        return NULL;
    }

    memset((void*)tmp_mem_q, 0, sizeof(BT_ALSA_PLAYBACK_MEM_QUEUE));

    tmp_mem_q->data_mem = malloc(BT_ALSA_PLAYBACK_BUF_SIZE*BT_ALSA_PLAYBACK_BUF_CNT);
    if (NULL == tmp_mem_q->data_mem)
    {
        free(tmp_mem_q);
        return NULL;
    }

    for(i=0;i<BT_ALSA_PLAYBACK_BUF_CNT;i++)
    {
        tmp_mem_q->mem_items[i].data_len = 0;
        tmp_mem_q->mem_items[i].buf = tmp_mem_q->data_mem + i * BT_ALSA_PLAYBACK_BUF_SIZE;
    }

    return tmp_mem_q;
}

static void bt_a2dp_alsa_pb_free_mem_q(BT_ALSA_PLAYBACK_MEM_QUEUE *mem_q)
{
    if (NULL == mem_q)
    {
        return;
    }

    free(mem_q->data_mem);

    free(mem_q);
    return;
}

static void bt_a2dp_alsa_pb_open(INT32 fs, INT32 channel_num)
{
    if (FALSE == g_bt_playback_cb.fgPlayBackInit)
    {
#ifdef BT_ALSA_PLAYBACK_STAT_SPEED
        s_bt_alsa_write_start = 1;
        s_bt_alsa_push_start = 1;
#endif
        dsp_open(fs, channel_num);
        g_bt_playback_cb.fgPlayBackInit = TRUE;

#ifdef BT_ALSA_PLAYBACK_ADD_BUFFER
        ALSA_DBG_NORMAL("<addbuf> g_buffer_time :%d", g_buffer_time);
        s_bt_alsa_pb_cb.data_q->fs = fs;
        s_bt_alsa_pb_cb.data_q->channels = channel_num;
        ALSA_DBG_NORMAL("<addbuf> data_q:fs(%d),channels(%d);fs(%ld),channel_num(%ld)",
                                  s_bt_alsa_pb_cb.data_q->fs,
                                  s_bt_alsa_pb_cb.data_q->channels,
                                  fs,
                                  channel_num
                                  );
#endif // #ifdef BT_ALSA_PLAYBACK_ADD_BUFFER

    }
    else
    {
        ALSA_DBG_WARNING("BT playback have init,no need init again");
    }
}

static void bt_a2dp_alsa_pb_close(void)
{
    if (TRUE == g_bt_playback_cb.fgPlayBackInit)
    {
#ifdef BT_ALSA_PLAYBACK_STAT_SPEED
        s_bt_alsa_write_start = 0;
        s_bt_alsa_push_start = 0;
        ALSA_DBG_NORMAL("write bytes: %d, write time: %ld us",
            s_bt_alsa_write_bytes, (g_last_write_tm.tv_sec-s_bt_start_write_tm.tv_sec)*1000000
            +(g_last_write_tm.tv_usec-s_bt_start_write_tm.tv_usec));
        ALSA_DBG_NORMAL("push bytes: %d, push time: %ld us",
            s_bt_alsa_push_bytes, (g_last_push_tm.tv_sec-s_bt_start_push_tm.tv_sec)*1000000
            +(g_last_push_tm.tv_usec-s_bt_start_push_tm.tv_usec));
#endif

        dsp_close();
        g_bt_playback_cb.fgPlayBackInit = FALSE;
    }
    else
    {
        ALSA_DBG_WARNING("BT playback have not init,no need deinit");
    }
}

static void bt_a2dp_alsa_pb_play(void)
{
    if (FALSE == g_bt_playback_cb.fgPlayBackInit)
    {
        ALSA_DBG_NOTICE("BT playback have not init");
        return;
    }
    if ((audio_status != BT_A2DP_ALSA_PB_STATUS_OPENED)
        && (audio_status != BT_A2DP_ALSA_PB_STATUS_PAUSED)
        && (audio_status != BT_A2DP_ALSA_PB_STATUS_STOPED))
    {
        ALSA_DBG_NORMAL("BT AUDIO wrong status, current status = %d", audio_status);
    }
    else
    {
        if (BT_A2DP_ALSA_PB_STATUS_PLAYED != audio_status)
        {
            ALSA_DBG_NORMAL("BT AUDIO current status = %d", audio_status);
        }
        audio_status = BT_A2DP_ALSA_PB_STATUS_PLAYED;
    }

    return;
}

static void bt_a2dp_alsa_pb_pause(void)
{
    if (FALSE == g_bt_playback_cb.fgPlayBackInit)
    {
        ALSA_DBG_NOTICE("BT playback have not init");
        return;
    }

    if (audio_status != BT_A2DP_ALSA_PB_STATUS_PLAYED)
    {
        ALSA_DBG_NOTICE("BT AUDIO wrong status, current status = %d", audio_status);
    }
    else
    {
        audio_status = BT_A2DP_ALSA_PB_STATUS_PAUSED;
        ALSA_DBG_NORMAL("BT AUDIO current status = %d", audio_status);
    }

    return;
}

static void bt_a2dp_alsa_pb_push(void *data, int datalen)
{
    UINT32 u4PcmLen = datalen;
    UINT8 *pu1PcmBuf = (UINT8 *)data;

    if ((NULL == data) || (0 == datalen))
    {
        ALSA_DBG_ERROR("data is null(%p) or data len=%d", data, datalen);
        return;
    }

    ALSA_DBG_INFO("data=%p, datalen=%u", data, datalen);

    if (FALSE == g_bt_playback_cb.fgPlayBackInit)
    {
        ALSA_DBG_NOTICE("bt playback does not init");
        return;
    }

    if (TRUE == g_bt_playback_cb.fgAudioReset)
    {
        ALSA_DBG_NOTICE("audio_reset");
        return;
    }

    if (audio_status != BT_A2DP_ALSA_PB_STATUS_PLAYED)
    {
        ALSA_DBG_NOTICE("audio_status is not played %d", audio_status);
        return;
    }

    if (BT_SUCCESS != bluetooth_a2dp_pb_write_data(pu1PcmBuf, u4PcmLen))
    {
        ALSA_DBG_ERROR("play pcm failed, pcmbuf:%p, len:%lu", pu1PcmBuf, (long)u4PcmLen);
        return;
    }

    return;
}


static void bt_a2dp_alsa_pb_adjust_priority(int start)
{
    int priority = start ? -19 : -16;
    if (setpriority(PRIO_PROCESS, 0, priority) < 0)
    {
        ALSA_DBG_WARNING("failed to change priority to %d", priority);
    }

    return;
}



static int bt_a2dp_alsa_pb_handle_cmd(BT_ALSA_PLAYBACK_CB *pb_cb,
    BT_ALSA_PLAYBACK_QUEUE_ITEM *cmd_item)
{
    if (NULL == cmd_item)
    {
        return 0;
    }
    ALSA_DBG_NORMAL("cmd type=%d", cmd_item->type);

    if (BT_ALSA_PLAYBACK_CMD_OPEN == cmd_item->type)
    {
        bt_a2dp_alsa_pb_open(cmd_item->u.param.fs, cmd_item->u.param.channels);

        bt_a2dp_alsa_pb_adjust_priority(1);
    }
    else if(BT_ALSA_PLAYBACK_CMD_CLOSE == cmd_item->type)
    {
        bt_a2dp_alsa_pb_close();
        bt_a2dp_alsa_pb_reset_mem_q(pb_cb->data_q);
        bt_a2dp_alsa_pb_adjust_priority(0);
#ifdef BT_ALSA_PLAYBACK_ADD_BUFFER
        return -1;
#endif // #ifdef BT_ALSA_PLAYBACK_ADD_BUFFER
    }
    else if(BT_ALSA_PLAYBACK_CMD_PLAY == cmd_item->type)
    {
#ifdef BT_ALSA_PLAYBACK_ADD_BUFFER
        if (pb_cb->data_q)
        {
            ALSA_DBG_NORMAL("<addbuf> process CMD_PLAY, reset data queue to re-buffering");
            bt_a2dp_alsa_pb_reset_data_queue();
        }
#endif // #ifdef BT_ALSA_PLAYBACK_ADD_BUFFER
        bt_a2dp_alsa_pb_play();
    }
    else if(BT_ALSA_PLAYBACK_CMD_PAUSE == cmd_item->type)
    {
        bt_a2dp_alsa_pb_pause();
    }
    else if(BT_ALSA_PLAYBACK_CMD_QUIT == cmd_item->type)
    {
        pthread_exit(NULL);
    }
    else
    {
        ALSA_DBG_ERROR("invalid cmd: %u", cmd_item->type);
    }

    return 0;
}

#ifdef BT_ALSA_PLAYBACK_ADD_BUFFER
static int bt_a2dp_alsa_pb_handle_buffer_add(BT_ALSA_PLAYBACK_CB *pb_cb)
{
    static int buffered_time = 0;
    buffered_time = bt_a2dp_alsa_pb_buffer_time(pb_cb->data_q->total_buffered_data_len,
                                                pb_cb->data_q->fs,
                                                pb_cb->data_q->channels,
                                                2);
    if ((TRUE == force_deque) ||
        ((pb_cb->data_q->buffer_done == FALSE) &&
        ((pb_cb->data_q->total_buffered_packet_num >= BT_ALSA_PLAYBACK_BUF_CNT) ||
        (buffered_time > g_buffer_time))))
    {
        int pushed_packet_num = 0;
        int tmp_total_buffered_packet_num = pb_cb->data_q->total_buffered_packet_num;
        if (TRUE == force_deque)
        {
            ALSA_DBG_NORMAL("<addbuf> no data buffer occur, force deque!");
            force_deque = FALSE;
        }
        else
        {
            ALSA_DBG_NORMAL("<addbuf> total_buffered_packet_num:%d (%d), total_buffered_data_len:%d", tmp_total_buffered_packet_num, pb_cb->data_q->total_buffered_packet_num, pb_cb->data_q->total_buffered_data_len);
            ALSA_DBG_NORMAL("<addbuf> has buffered about %d ms data, dequeue all of them and send them to alsa.", buffered_time);
        }
        pb_cb->data_q->buffer_done = TRUE;
        do {
            if (0 == bt_a2dp_alsa_pb_peak_front_mem(pb_cb->data_q, &mem_item))
            {
                bt_a2dp_alsa_pb_push(mem_item.buf, mem_item.data_len);
                bt_a2dp_alsa_pb_deq_mem(pb_cb->data_q);
                pushed_packet_num ++;
            }
            else
            {
                ALSA_DBG_ERROR("<addbuf> bt_a2dp_alsa_pb_deq fail!");
            }
        } while (pushed_packet_num < tmp_total_buffered_packet_num);

        ALSA_DBG_NORMAL("<addbuf> finish precess buffered data!");
    }
    else
    {
        if (pb_cb->data_q->buffer_done == TRUE)
        {
            return 1;
        }
    }

    return 0;
}
#endif


static void* bt_a2dp_alsa_pb_handler(void *data)
{
    BT_ALSA_PLAYBACK_CB *pb_cb = (BT_ALSA_PLAYBACK_CB *)data;
    BT_ALSA_PLAYBACK_QUEUE_ITEM cmd_item;
    BT_ALSA_PLAYBACK_MEM_ITEM mem_item;

#ifdef BT_ALSA_PLAYBACK_ADD_BUFFER
    int add_buf_handle_ret = 0;
#endif

    memset((void*)&mem_item, 0, sizeof(mem_item));
    memset((void*)&cmd_item, 0, sizeof(cmd_item));

    while (1)
    {
        pthread_mutex_lock(&pb_cb->lock);
        if(!pb_cb->has_q_data)
        {
            pthread_cond_wait(&pb_cb->signal, &pb_cb->lock);
        }
        pb_cb->has_q_data = 0;
        pthread_mutex_unlock(&pb_cb->lock);

        while(1)
        {
            if (0 == bt_a2dp_alsa_pb_deq(pb_cb->cmd_q, &cmd_item))
            {
                if (bt_a2dp_alsa_pb_handle_cmd(pb_cb, &cmd_item) < 0)
                {
                    break;
                }
            }
#ifdef BT_ALSA_PLAYBACK_ADD_BUFFER
            add_buf_handle_ret = bt_a2dp_alsa_pb_handle_buffer_add(pb_cb);
            if (0 == add_buf_handle_ret)
            {
                break;
            }
#endif // #ifdef BT_ALSA_PLAYBACK_ADD_BUFFER

            if (0 == bt_a2dp_alsa_pb_peak_front_mem(pb_cb->data_q, &mem_item))
            {
                bt_a2dp_alsa_pb_push(mem_item.buf, mem_item.data_len);
                bt_a2dp_alsa_pb_deq_mem(pb_cb->data_q);
            }
            else
            {
                break;
            }
        }
    }

    return NULL;
}

static int bt_a2dp_alsa_pb_init(BT_A2DP_PLAYER_EVENT_CB event_cb)
{
    ALSA_DBG_NORMAL("inited=%d", s_bt_alsa_pb_cb.inited);
    if (1 == s_bt_alsa_pb_cb.inited)
    {
        return 0;
    }

    pthread_mutex_init(&s_bt_alsa_pb_cb.lock, NULL);
    pthread_mutex_init(&s_bt_alsa_pb_cb.mem_lock, NULL);

    pthread_cond_init(&s_bt_alsa_pb_cb.signal, NULL);

    s_bt_alsa_pb_cb.cmd_q = bt_a2dp_alsa_pb_alloc_q(BT_ALSA_PLAYBACK_CMD_Q_SIZE);
    if (NULL == s_bt_alsa_pb_cb.cmd_q)
    {
        ALSA_DBG_ERROR("alloc cmd q fail");
        goto __ERROR;
    }

    s_bt_alsa_pb_cb.data_q = bt_a2dp_alsa_pb_alloc_mem_q();
    if (NULL == s_bt_alsa_pb_cb.data_q)
    {
        ALSA_DBG_ERROR("alloc mem mem_q fail");
        goto __ERROR;
    }

    if(0 < pthread_create(&s_bt_alsa_pb_cb.work_thread, NULL,
        bt_a2dp_alsa_pb_handler, (void*)&s_bt_alsa_pb_cb))
    {
        ALSA_DBG_ERROR("create bt alsa thread fail");
        goto __ERROR;
    }
    g_bt_a2dp_alsa_pb_event_cb = event_cb;
    s_bt_alsa_pb_cb.inited = 1;

    return 0;
__ERROR:
    if (NULL != s_bt_alsa_pb_cb.data_q)
    {
        bt_a2dp_alsa_pb_free_mem_q(s_bt_alsa_pb_cb.data_q);
        s_bt_alsa_pb_cb.data_q = NULL;
    }

    if (NULL != s_bt_alsa_pb_cb.cmd_q)
    {
        bt_a2dp_alsa_pb_free_q(s_bt_alsa_pb_cb.cmd_q);
        s_bt_alsa_pb_cb.cmd_q = NULL;
    }

    pthread_mutex_destroy(&s_bt_alsa_pb_cb.lock);
    pthread_mutex_destroy(&s_bt_alsa_pb_cb.mem_lock);
    pthread_cond_destroy(&s_bt_alsa_pb_cb.signal);
    return -1;
}

static int bt_a2dp_alsa_pb_deinit(void)
{
    BT_ALSA_PLAYBACK_QUEUE_ITEM cmd_item = {0};
    ALSA_DBG_NORMAL("inited=%d", s_bt_alsa_pb_cb.inited);
    if (0 == s_bt_alsa_pb_cb.inited)
    {
        return 0;
    }
    memset((void*)&cmd_item, 0, sizeof(cmd_item));
    cmd_item.type = BT_ALSA_PLAYBACK_CMD_QUIT;
    bt_a2dp_alsa_pb_enq(s_bt_alsa_pb_cb.cmd_q, &cmd_item);
    pthread_mutex_lock(&s_bt_alsa_pb_cb.lock);
    s_bt_alsa_pb_cb.has_q_data = 1;
    pthread_cond_signal(&s_bt_alsa_pb_cb.signal);
    pthread_mutex_unlock(&s_bt_alsa_pb_cb.lock);

    pthread_join(s_bt_alsa_pb_cb.work_thread, NULL);
    bt_a2dp_alsa_pb_free_mem_q(s_bt_alsa_pb_cb.data_q);

    if (NULL != s_bt_alsa_pb_cb.cmd_q)
    {
        bt_a2dp_alsa_pb_free_q(s_bt_alsa_pb_cb.cmd_q);
        s_bt_alsa_pb_cb.cmd_q = NULL;
    }

    pthread_mutex_destroy(&s_bt_alsa_pb_cb.lock);
    pthread_mutex_destroy(&s_bt_alsa_pb_cb.mem_lock);
    pthread_cond_destroy(&s_bt_alsa_pb_cb.signal);

    g_bt_a2dp_alsa_pb_event_cb = NULL;

    s_bt_alsa_pb_cb.inited = 0;
    return 0;
}

static void bt_a2dp_alsa_pb_enq_cmd(BT_ALSA_PLAYBACK_QUEUE_ITEM *cmd)
{
    if (NULL == cmd)
    {
        return;
    }

    bt_a2dp_alsa_pb_enq(s_bt_alsa_pb_cb.cmd_q, cmd);

    pthread_mutex_lock(&s_bt_alsa_pb_cb.lock);
    s_bt_alsa_pb_cb.has_q_data = 1;
    pthread_cond_signal(&s_bt_alsa_pb_cb.signal);
    pthread_mutex_unlock(&s_bt_alsa_pb_cb.lock);

    return;
}

static int bt_a2dp_alsa_pb_enq_data(unsigned char *data, int len)
{
    if ((NULL == data) || (0 >= len))
    {
        return BT_ERR_STATUS_PARM_INVALID;
    }

#ifdef BT_ALSA_PLAYBACK_STAT_SPEED
    if (1 == s_bt_alsa_push_start)
    {
        s_bt_alsa_push_start = 2;
        gettimeofday(&s_bt_start_push_tm,NULL);
        s_bt_alsa_push_bytes = 0;
    }
    else if (2 == s_bt_alsa_push_start)
    {
        s_bt_alsa_push_bytes += len;
        gettimeofday(&g_last_push_tm,NULL);
    }
#endif

    bt_a2dp_alsa_pb_enq_mem(s_bt_alsa_pb_cb.data_q, data, len);
#ifdef BT_ALSA_PLAYBACK_ADD_BUFFER
    if (FALSE == s_bt_alsa_pb_cb.data_q->buffer_done)
    {
        s_bt_alsa_pb_cb.data_q->total_buffered_data_len += len;
        s_bt_alsa_pb_cb.data_q->total_buffered_packet_num ++;
        //ALSA_DBG_ERROR("<addbuf> total_buffered_packet_num:%d", s_bt_alsa_pb_cb.data_q->total_buffered_packet_num);
    }
#endif // #ifdef BT_ALSA_PLAYBACK_ADD_BUFFER

    pthread_mutex_lock(&s_bt_alsa_pb_cb.lock);
    s_bt_alsa_pb_cb.has_q_data = 1;
    pthread_cond_signal(&s_bt_alsa_pb_cb.signal);
    pthread_mutex_unlock(&s_bt_alsa_pb_cb.lock);

    return BT_SUCCESS;
}

#ifdef BT_ALSA_PLAYBACK_TEST
static void* bt_a2dp_alsa_pb_test_push_handler(void *data)
{
    unsigned char *buf = NULL;
    ALSA_DBG_NORMAL("start");
    sleep(10);
    buf = (unsigned char*)malloc(4800);
    if (NULL == buf)
    {
        ALSA_DBG_ERROR("alloc test buffer fail");
        return NULL;
    }
    bt_a2dp_alsa_pb_adjust_priority(1);

    while (1)
    {
        bt_a2dp_alsa_player_write(buf, 4800);
        usleep(random()%5000+20000);
    }

    return NULL;
}

static void* bt_a2dp_alsa_pb_test_open_close_handler(void *data)
{
    ALSA_DBG_NORMAL("start");
    sleep(10);
    while (1)
    {
        bt_a2dp_alsa_player_start(48000, 2);
        usleep(random() % 1000000 + 100000);
        bt_a2dp_alsa_player_stop();
    }
    return NULL;
}

static void bt_a2dp_alsa_pb_test(void)
{
    pthread_t t1, t2, t3;
    ALSA_DBG_NORMAL("start");

    pthread_create(&t1, NULL, bt_a2dp_alsa_pb_test_open_close_handler, NULL);
    pthread_create(&t2, NULL, bt_a2dp_alsa_pb_test_push_handler, NULL);
    pthread_create(&t3, NULL, bt_a2dp_alsa_pb_test_open_close_handler, NULL);

    return;
}
#endif

#endif
/******************************************************************************
 */

