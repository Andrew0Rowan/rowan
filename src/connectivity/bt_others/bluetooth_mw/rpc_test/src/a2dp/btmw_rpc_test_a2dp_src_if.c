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

#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <time.h>
#include <stdlib.h>
#include <asoundlib.h>

#include "mtk_audio.h"
#include "btmw_rpc_test_cli.h"
#include "btmw_rpc_test_debug.h"
#include "btmw_rpc_test_a2dp_src_if.h"
#include "mtk_bt_service_a2dp_wrapper.h"

#define BTMW_RPC_TEST_CMD_KEY_A2DP_SRC        "MW_RPC_A2DP_SRC"
#define PCM_BUFFER_SIZE 512*8     // at least 512
/* 0:pause, 1: play */
static pthread_t btmw_rpc_test_stream_handle = -1;
static CHAR btmw_rpc_test_pcm_file[128] = "/data/usb/music/48000/input_48000.pcm";

static INT32 g_bt_src_stream_is_suspend = 0;

extern int adev_open(const hw_module_t* module, const char* name,
                         hw_device_t** device);


extern CHAR g_rpc_a2dp_addr_test[18];
extern BT_A2DP_STREAM_STATE g_rpc_a2dp_stream_state;

#if defined(MTK_BT_PLAYBACK_DEFAULT_ALSA)
#define ALSA_DEVICE_PLAYER "default" /* for MTK branch tree */
#else
#define ALSA_DEVICE_PLAYER "main"
#endif
#define FRAGMENT_SAMPLES    (4096*4)

#define BTMW_RPC_A2DP_SRC_CASE_RETURN_STR(const) case const: return #const;

static snd_pcm_t *s_alsa_handle = NULL;

static snd_pcm_uframes_t chunk_size = 0;

static UINT32 u4buffer_time = 500000; /* ring buffer length in us */
static UINT32 u4period_time = 1000; /* period time in us */

static snd_pcm_uframes_t period_frames = 0;
static snd_pcm_uframes_t buffer_frames = 0;
static INT32 i4avail_min = -1;
static INT32 i4start_delay = 200000;
static INT32 i4stop_delay = 0;


// CLI handler
static int btmw_rpc_test_a2dp_src_connect_int_handler(int argc, char *argv[]);
static int btmw_rpc_test_a2dp_src_disconnect_handler(int argc, char *argv[]);
static int btmw_rpc_test_a2dp_src_active_handler(int argc, char *argv[]);
static int btmw_rpc_test_a2dp_get_sink_dev_list_handler(int argc, char *argv[]);
static int btmw_rpc_test_a2dp_src_write_stream_handler(int argc, char *argv[]);
static int btmw_rpc_test_a2dp_src_set_audio_hw_dbg_lvl_handler(int argc, char *argv[]);

static CHAR* btmw_rpc_test_a2dp_src_app_role(BT_A2DP_ROLE local_role)
{
    switch((int)local_role)
    {
        BTMW_RPC_A2DP_SRC_CASE_RETURN_STR(BT_A2DP_ROLE_SRC)
        BTMW_RPC_A2DP_SRC_CASE_RETURN_STR(BT_A2DP_ROLE_SINK)
        default: return "UNKNOWN_ROLE";
   }
}

UINT32 bt_get_microseconds(VOID)
{
    struct timespec now;
    clock_gettime(CLOCK_MONOTONIC, &now);
    return (now.tv_sec * 1000000 + now.tv_nsec / 1000);
}

static int btmw_rpc_test_a2dp_src_set_audio_hw_dbg_lvl_handler(int argc, char *argv[])
{
    BT_A2DP_DBG_PARAM param;
    BTMW_RPC_TEST_Logi("%s()\n", __func__);

    if (argc != 1)
    {
        BTMW_RPC_TEST_Loge("[USAGE] audio_hw_log <0~6>");
        return -1;
    }

    param.hw_audio_log_lv = atoi(argv[0]);
    if (0 <= param.hw_audio_log_lv && 6 >= param.hw_audio_log_lv)
    {
        a_mtkapi_a2dp_set_dbg_flag(BT_A2DP_DBG_SET_HW_AUDIO_LOG_LV, &param);
    }
    else
    {
        BTMW_RPC_TEST_Loge("input error\n");
        BTMW_RPC_TEST_Loge("please input audio_hw_log <0~6>\n");
    }
    return 0;
}

static int btmw_rpc_test_a2dp_src_show_dev_info_handler(int argc, char *argv[])
{
    BT_A2DP_DBG_PARAM param;
    BTMW_RPC_TEST_Logi("%s()\n", __func__);

    a_mtkapi_a2dp_set_dbg_flag(BT_A2DP_DBG_SHOW_INFO, &param);

    return 0;
}


static int btmw_rpc_test_a2dp_src_active_handler(int argc, char *argv[])
{
    UINT8 u1_enable = 0;
    BTMW_RPC_TEST_Logi("%s()\n", __func__);

    if (argc != 1)
    {
        BTMW_RPC_TEST_Loge("[USAGE] active_src [1:enable|0:disable]");
        return -1;
    }

    u1_enable = atoi(argv[0]);

    a_mtkapi_a2dp_src_enable(u1_enable);

    return 0;
}


static int btmw_rpc_test_a2dp_get_sink_dev_list_handler(int argc, char *argv[])
{
    BT_A2DP_DEVICE_LIST device_list;
    INT32 ret = 0;
    BTMW_RPC_TEST_Logi("%s()\n", __func__);

    memset((void*)&device_list, 0, sizeof(device_list));

    ret = a_mtkapi_a2dp_src_get_dev_list(&device_list);
    BTMW_RPC_TEST_Logi("get paired sink device list result:\n", __func__);
    BTMW_RPC_TEST_Logi("======================================\n");
    if(BT_SUCCESS == ret)
    {
        if (0 == device_list.dev_num)
        {
            BTMW_RPC_TEST_Logi("no paired sink device\n");
        }
        else
        {
            INT32 i = 0;
            for(i=0;i<device_list.dev_num;i++)
            {
                BTMW_RPC_TEST_Logi("device[%d]: %s, name:%s, role:%s\n",
                    i, device_list.dev[i].addr, device_list.dev[i].name,
                    btmw_rpc_test_a2dp_src_app_role(device_list.dev[i].role));
                BTMW_RPC_TEST_Logi("======================================\n");
            }
        }
    }
    else
    {
        BTMW_RPC_TEST_Logi("get paired sink device failed: %d\n", ret);
    }

    return 0;
}

static int btmw_rpc_test_a2dp_src_connect_int_handler(int argc, char *argv[])
{
    CHAR *ptr;

    BTMW_RPC_TEST_Logd("%s()\n", __func__);

    if (argc != 1)
    {
        BTMW_RPC_TEST_Loge("[USAGE] connect <addr>");
        return -1;
    }
    if (strlen(argv[0]) < 17)
    {
        BTMW_RPC_TEST_Loge("<addr> invalid. Good example is \"AA:BB:CC:DD:EE:FF\"");
        BTMW_RPC_TEST_Loge("[USAGE] connect <addr>");
        return -1;
    }

    ptr = argv[0];
    a_mtkapi_a2dp_connect(ptr, BT_A2DP_ROLE_SRC);

    return 0;
}

static int btmw_rpc_test_a2dp_src_disconnect_handler(int argc, char *argv[])
{
    CHAR *ptr;

    BTMW_RPC_TEST_Logd("%s()\n", __func__);

    if (argc != 1)
    {
        BTMW_RPC_TEST_Loge("[USAGE] disconnect <addr>");
        return -1;
    }
    if (strlen(argv[0]) < 17)
    {
        BTMW_RPC_TEST_Loge("<addr> invalid. Good example is \"AA:BB:CC:DD:EE:FF\"");
        BTMW_RPC_TEST_Loge("[USAGE] disconnect <addr>");
        return -1;
    }

    ptr = argv[0];
    a_mtkapi_a2dp_disconnect(ptr);
    return 0;
}

static VOID* btmw_rpc_test_write_audio_data_thread(VOID *arg)
{
    UINT8 *pcm_buffer = NULL;
    CHAR *local_test_pcm_file = (CHAR *)arg;
    FILE *fInputPCM;
    INT32 result=0;
    INT32 pcm_frame_len = PCM_BUFFER_SIZE;
    INT32 total_pcm_len;
    int read_cnt = 0;
    hw_device_t* device = NULL;
    struct audio_hw_device *hw_dev = NULL;
    struct audio_stream_out *stream = NULL;
    g_bt_src_stream_is_suspend = 0;

    BTMW_RPC_TEST_Logd("Input file name  : %s\n", local_test_pcm_file);
    /* open file & allocate memory */
    fInputPCM = fopen(local_test_pcm_file, "rb");
    if (fInputPCM == NULL)
    {
        BTMW_RPC_TEST_Loge("Can't open input PCM file!\n");
        btmw_rpc_test_stream_handle = -1;
        return NULL;
    }
    BTMW_RPC_TEST_Logd("open input PCM file success!\n");

    //fInputDumpPCM = fopen("/data/sda1/send_before.pcm", "wb+");

    pcm_buffer = (UINT8 *)malloc(PCM_BUFFER_SIZE);
    if (pcm_buffer == NULL)
    {
        fclose(fInputPCM);
        BTMW_RPC_TEST_Loge("Can't allocat buffer\n");
        btmw_rpc_test_stream_handle = -1;
        return NULL;
    }

    if (0 > adev_open(NULL, AUDIO_HARDWARE_INTERFACE, &device))
    {
        BTMW_RPC_TEST_Loge("open bt adev fail\n");
        goto send_audio_data_end;
    }
    hw_dev = (struct audio_hw_device *)device;
    if (0 > hw_dev->open_output_stream(hw_dev, 0, 0, 0, 0, &stream, 0))
    {
        BTMW_RPC_TEST_Loge("open out stream fail\n");
        goto send_audio_data_end;
    }

#if 0
    struct timeval last;
    struct timeval tv, tv1;

    gettimeofday(&last, NULL);
#endif
    BTMW_RPC_TEST_Logd("addr[0]=%d, cli_mode=%d, suspend=%d\n",
        g_rpc_a2dp_addr_test[0], g_cli_pts_mode, g_bt_src_stream_is_suspend);
    while(read_cnt < 1 && ((g_rpc_a2dp_addr_test[0] != 0 && 0 == g_cli_pts_mode)
        || (0 != g_cli_pts_mode && !g_bt_src_stream_is_suspend)))
    {
        memset(pcm_buffer, 0, PCM_BUFFER_SIZE);
        total_pcm_len = fread(pcm_buffer, sizeof(UINT8), pcm_frame_len, fInputPCM );
        if (total_pcm_len == 0)
        {
            BTMW_RPC_TEST_Loge("total_pcm_len==0\n");
            fseek(fInputPCM, 0L, SEEK_SET);
            read_cnt++;

            continue;
        }


#if 0
        gettimeofday(&tv, NULL);
        if (((tv.tv_sec - last.tv_sec) * 1000000 + (long)tv.tv_usec - (long)last.tv_usec) > 500000)
        {
#if 0
            hw_dev->close_output_stream(hw_dev, stream);
            device->close(device);
            adev_open(NULL, AUDIO_HARDWARE_INTERFACE, &device);
            hw_dev = (struct audio_hw_device *)device;
            hw_dev->open_output_stream(hw_dev, 0, 0, 0, 0, &stream, 0);
#else
            hw_dev->set_parameters(hw_dev, "A2dpSuspended=true");
            hw_dev->set_parameters(hw_dev, "A2dpSuspended=false");
#endif
            last = tv;
        }
#endif


        stream->write(stream, pcm_buffer, total_pcm_len);
    }
send_audio_data_end:

    if (NULL != hw_dev)
    {
        hw_dev->close_output_stream(hw_dev, stream);
    }

    if (NULL != device)
    {
        device->close(device);
    }
    fclose(fInputPCM);
    fInputPCM = NULL;
    free(pcm_buffer);
    pcm_buffer = NULL;
    btmw_rpc_test_stream_handle = -1;
    BTMW_RPC_TEST_Logd("Send audio finished\n");
    return NULL;
}


static int btmw_rpc_test_a2dp_src_write_stream_handler(int argc, char *argv[])
{
    INT32 result;

    BTMW_RPC_TEST_Logd("%s(), file:%s\n", __func__, argv[0]);
    strcpy(btmw_rpc_test_pcm_file, argv[0]);

    if(-1 == btmw_rpc_test_stream_handle)
    {
        result = pthread_create(&btmw_rpc_test_stream_handle, NULL,
            btmw_rpc_test_write_audio_data_thread, btmw_rpc_test_pcm_file);
        if (result)
        {
            BTMW_RPC_TEST_Logd("pthread_create failed! (%d)\n", result);
        }
    }
    else
    {
        BTMW_RPC_TEST_Logw("streaming thread has been created!\n");
    }

    return 0;
}



static INT32 btmw_rpc_test_play_audio_set_params(INT32 fs, INT32 channel_num)
{
    BTMW_RPC_TEST_Logd("+++into");
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
        BTMW_RPC_TEST_Loge("Broken configuration for playback: no configurations available: %s", snd_strerror(i4ret));
        return i4ret;
    }
    /* set the sample format */
    i4ret = snd_pcm_hw_params_set_format(s_alsa_handle, hwparams, SND_PCM_FORMAT_S16_LE);
    if (i4ret < 0)
    {
        BTMW_RPC_TEST_Loge("Sample format not available for playback: %s", snd_strerror(i4ret));
        return i4ret;
    }
    /* set the interleaved read/write format */
    i4ret = snd_pcm_hw_params_set_access(s_alsa_handle, hwparams, SND_PCM_ACCESS_RW_INTERLEAVED);
    if (i4ret < 0)
    {
        BTMW_RPC_TEST_Loge("Access type not available for playback: %s", snd_strerror(i4ret));
        return i4ret;
    }
    /* set the count of channels */
    i4ret = snd_pcm_hw_params_set_channels(s_alsa_handle, hwparams, channel_num);
    if (i4ret < 0)
    {
        BTMW_RPC_TEST_Loge("Channels count (%i) not available for playbacks: %s", channel_num, snd_strerror(i4ret));
        return i4ret;
    }
    /* set the stream sampling rate */
    i4ret = snd_pcm_hw_params_set_rate(s_alsa_handle, hwparams, fs, 0);
    if (i4ret < 0)
    {
        BTMW_RPC_TEST_Loge("Rate %iHz not available for playback: %s", fs, snd_strerror(i4ret));
        return i4ret;
    }

    u4rate = fs;
    if ((u4buffer_time == 0) && (buffer_frames == 0))
    {
        i4ret = snd_pcm_hw_params_get_buffer_time_max(hwparams, &u4buffer_time, 0);
        if (i4ret < 0)
        {
            BTMW_RPC_TEST_Loge("fail to get max buffer time:%d, %s", i4ret, snd_strerror(i4ret));
            return i4ret;
        }
        BTMW_RPC_TEST_Logd("u4buffer_time:%d", u4buffer_time);
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
        BTMW_RPC_TEST_Loge("fail to get period size:%d, %s", i4ret, snd_strerror(i4ret));
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
        BTMW_RPC_TEST_Loge("fail to get buffer size:%d, %s", i4ret, snd_strerror(i4ret));
        return i4ret;
    }

    i4ret = snd_pcm_hw_params(s_alsa_handle, hwparams);
    if (i4ret < 0)
    {
        BTMW_RPC_TEST_Loge("Unable to install hw params");
        return i4ret;
    }

    snd_pcm_hw_params_get_period_size(hwparams, &chunk_size, 0);
    snd_pcm_hw_params_get_buffer_size(hwparams, &buffer_size);
    BTMW_RPC_TEST_Logd("chunk_size:%lu, buffer_size:%lu", chunk_size, buffer_size);
    if (chunk_size == buffer_size)
    {
        BTMW_RPC_TEST_Loge("Can't use period equal to buffer size (%lu == %lu)",
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
        BTMW_RPC_TEST_Loge("fail to set start threshold:%d, %s", i4ret, snd_strerror(i4ret));
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
        BTMW_RPC_TEST_Loge("fail to set stop threshold:%d, %s", i4ret, snd_strerror(i4ret));
        return i4ret;
    }

    /* write the parameters to the playback device */
    if ((i4ret = snd_pcm_sw_params(s_alsa_handle, swparams)) < 0)
    {
        BTMW_RPC_TEST_Loge("unable to install sw params");
        return i4ret;
    }

    snd_pcm_sw_params_get_start_threshold(swparams, &start_threshold);
    snd_pcm_sw_params_get_stop_threshold(swparams, &stop_threshold);
    BTMW_RPC_TEST_Logd("start_threshold:%lu, stop_threshold:%lu", start_threshold, stop_threshold);
    //snd_pcm_hw_params_free(hwparams);
    //snd_pcm_sw_params_free(swparams);
    BTMW_RPC_TEST_Logd("---exit");
    return 0;
}

static INT32 btmw_rpc_test_play_audio_dsp_open(INT32 fs, INT32 channel_num)
{
    BTMW_RPC_TEST_Logd("+++into");
    INT32 i4ret = 0;

    //snd_pcm_hw_params_t *hwparams;

    if (s_alsa_handle != NULL)
    {
        BTMW_RPC_TEST_Logw("---exit already opened s_alsa_handle");
        return 0;
    }

    i4ret = snd_pcm_open(&s_alsa_handle, ALSA_DEVICE_PLAYER, SND_PCM_STREAM_PLAYBACK, 0 );
    BTMW_RPC_TEST_Logd("fs %d, channel num %d ", fs, channel_num);
    BTMW_RPC_TEST_Logd("dsp_open %s i4ret=%d[%s]", ALSA_DEVICE_PLAYER, i4ret, snd_strerror(i4ret));
    if (i4ret < 0)
    {
        BTMW_RPC_TEST_Loge("Cannot open %s ERROR %d[%s]", ALSA_DEVICE_PLAYER, i4ret, snd_strerror(i4ret));
        s_alsa_handle = NULL;
        return  -1;
    }

    btmw_rpc_test_play_audio_set_params(fs, channel_num);


    snd_pcm_prepare(s_alsa_handle);

    BTMW_RPC_TEST_Logd("---exit");
    return 0;
}

static INT32 btmw_rpc_test_play_audio_dsp_write(UINT8 *buf, UINT32 size)
{
    INT32 i4ret = 0;

    if (s_alsa_handle == NULL)
    {
        BTMW_RPC_TEST_Loge("s_alsa_handle == NULL");
        return -1;
    }

    i4ret = snd_pcm_writei(s_alsa_handle, buf, size/4);
    if (i4ret < 0)
    {
        BTMW_RPC_TEST_Loge("ALSA ERROR %d[%s]", i4ret, snd_strerror(i4ret));
        snd_pcm_prepare(s_alsa_handle);
        if ((i4ret = snd_pcm_prepare(s_alsa_handle))<0)
        {
            BTMW_RPC_TEST_Loge("ALSA snd_pcm_prepare ERROR %d[%s]", i4ret, snd_strerror(i4ret));
        }
    }
    //BTMW_RPC_TEST_Logv("alsa write i4ret = %d", i4ret);
    return i4ret;
}

static INT32 btmw_rpc_test_play_audio_dsp_close(VOID)
{
    INT32 i4ret = 0;
    BTMW_RPC_TEST_Logd("+++into");
    if (s_alsa_handle == NULL)
    {
        BTMW_RPC_TEST_Loge("---exit s_alsa_handle == NULL");
        return -1;
    }
    if (s_alsa_handle != NULL)
    {
        i4ret = snd_pcm_close(s_alsa_handle);
        if (i4ret == 0)
        {
            BTMW_RPC_TEST_Logd("dsp_close success");
        }
        else
        {
            BTMW_RPC_TEST_Loge("dsp_close fail i4ret=%d[%s]", i4ret, snd_strerror(i4ret));
        }
        s_alsa_handle = NULL;
    }

    BTMW_RPC_TEST_Logd("---exit");
    return i4ret;
}



static VOID* btmw_rpc_test_play_audio_data_thread(VOID *arg)
{
    UINT8 *pcm_buffer = NULL;
    CHAR *local_test_pcm_file = (CHAR *)arg;
    FILE *fInputPCM;
    INT32 result=0;
    INT32 pcm_frame_len = PCM_BUFFER_SIZE;
    INT32 total_pcm_len;
    INT32 now = 0;
    UINT32 interval = 0;
    INT32 wait = 0;
    UINT32 wlength = 2;
    UINT32 channel_num = 2;
    UINT32 sample_rate = 48000;
    int read_cnt = 0;

    BTMW_RPC_TEST_Logd("Input file name  : %s\n", local_test_pcm_file);
    /* open file & allocate memory */
    fInputPCM = fopen(local_test_pcm_file, "rb");
    if (fInputPCM == NULL)
    {
        BTMW_RPC_TEST_Loge("Can't open input PCM file!\n");
        btmw_rpc_test_stream_handle = -1;
        return NULL;
    }
    BTMW_RPC_TEST_Logd("open input PCM file success!\n");

    //fInputDumpPCM = fopen("/data/sda1/send_before.pcm", "wb+");

    pcm_buffer = (UINT8 *)malloc(PCM_BUFFER_SIZE);
    if (pcm_buffer == NULL)
    {
        fclose(fInputPCM);
        BTMW_RPC_TEST_Loge("Can't allocat buffer\n");
        btmw_rpc_test_stream_handle = -1;
        return NULL;
    }

    if (0 > btmw_rpc_test_play_audio_dsp_open(48000, 2))
    {
        fclose(fInputPCM);
        free(pcm_buffer);
        btmw_rpc_test_stream_handle = -1;
        return NULL;
    }

    g_bt_src_stream_is_suspend = 0;
    BTMW_RPC_TEST_Logd("addr[0]=%d, cli_mode=%d, suspend=%d\n",
        g_rpc_a2dp_addr_test[0], g_cli_pts_mode, g_bt_src_stream_is_suspend);

    while((g_rpc_a2dp_addr_test[0] != 0 && 0 == g_cli_pts_mode
            && g_rpc_a2dp_stream_state == BT_A2DP_STREAM_STATE_PLAYING)
            || (0 != g_cli_pts_mode && !g_bt_src_stream_is_suspend))
    {
        memset(pcm_buffer, 0, PCM_BUFFER_SIZE);
        total_pcm_len = fread(pcm_buffer, sizeof(UINT8), pcm_frame_len, fInputPCM );
        if (total_pcm_len == 0)
        {
            BTMW_RPC_TEST_Loge("total_pcm_len==0\n");
            fseek(fInputPCM, 0L, SEEK_SET);
            read_cnt++;

            continue;
        }

        btmw_rpc_test_play_audio_dsp_write(pcm_buffer, total_pcm_len);
    }
send_audio_data_end:
    btmw_rpc_test_play_audio_dsp_close();

    fclose(fInputPCM);
    fInputPCM = NULL;
    free(pcm_buffer);
    pcm_buffer = NULL;

    btmw_rpc_test_stream_handle = -1;
    BTMW_RPC_TEST_Logd("Send audio finished\n");
    return NULL;
}



static int btmw_rpc_test_a2dp_src_play_stream_handler(int argc, char *argv[])
{
    INT32 result;

    BTMW_RPC_TEST_Logd("%s(), file:%s\n", __func__, argv[0]);
    strcpy(btmw_rpc_test_pcm_file, argv[0]);

    if(-1 == btmw_rpc_test_stream_handle)
    {
        result = pthread_create(&btmw_rpc_test_stream_handle, NULL,
            btmw_rpc_test_play_audio_data_thread, btmw_rpc_test_pcm_file);
        if (result)
        {
            BTMW_RPC_TEST_Logd("pthread_create failed! (%d)\n", result);
        }
    }
    else
    {
        BTMW_RPC_TEST_Logw("streaming thread has been created!\n");
    }

    return 0;
}
static int btmw_rpc_test_a2dp_src_suspend_stream_handler(int argc, char *argv[])
{
    g_bt_src_stream_is_suspend = 1;
    return 0;
}

static BTMW_RPC_TEST_CLI btmw_rpc_test_a2dp_src_cli_commands[] =
{
    {"connect",       btmw_rpc_test_a2dp_src_connect_int_handler,               " = connect <addr>"},
    {"disconnect",    btmw_rpc_test_a2dp_src_disconnect_handler,                " = disconnect <addr>"},
    {"active_src",    btmw_rpc_test_a2dp_src_active_handler,                    " = active_src <1:enable|0:disable>"},
    {"get_paired_dev",btmw_rpc_test_a2dp_get_sink_dev_list_handler,             " = get_paired_dev"},
    {"write_stream",  btmw_rpc_test_a2dp_src_write_stream_handler,              " = write_stream <file-path>"},
    {"audio_hw_log",  btmw_rpc_test_a2dp_src_set_audio_hw_dbg_lvl_handler,      " = audio_hw_log <0~6>"},
    {"show_info",     btmw_rpc_test_a2dp_src_show_dev_info_handler,             " = show_info"},
    {"play_stream",   btmw_rpc_test_a2dp_src_play_stream_handler,               " = play_stream"},
    {"suspend_stream",   btmw_rpc_test_a2dp_src_suspend_stream_handler,         " = suspend_stream"},
    {NULL, NULL, NULL},
};

int btmw_rpc_test_a2dp_src_cmd_handler(int argc, char **argv)
{
    BTMW_RPC_TEST_CLI *cmd, *match = NULL;
    INT32 ret = 0;
    INT32 count = 0;

    BTMW_RPC_TEST_Logd("%s argc: %d, argv[0]: %s\n", __func__, argc, argv[0]);

    cmd = btmw_rpc_test_a2dp_src_cli_commands;
    while (cmd->cmd)
    {
        if (!strcmp(cmd->cmd, argv[0]))
        {
            match = cmd;
            count = 1;
            break;
        }
        cmd++;
    }

    if (count == 0)
    {
        BTMW_RPC_TEST_Logd("Unknown command '%s'\n", argv[0]);

        btmw_rpc_test_print_cmd_help(BTMW_RPC_TEST_CMD_KEY_A2DP_SRC, btmw_rpc_test_a2dp_src_cli_commands);
        ret = -1;
    }
    else
    {
        match->handler(argc - 1, &argv[1]);
    }

    return ret;
}

INT32 btmw_rpc_test_a2dp_src_init(VOID)
{
    INT32 ret = 0;
    BTMW_RPC_TEST_MOD a2dp_src_mod = {0};

    a2dp_src_mod.mod_id = BTMW_RPC_TEST_MOD_A2DP_SRC;
    strncpy(a2dp_src_mod.cmd_key, BTMW_RPC_TEST_CMD_KEY_A2DP_SRC, sizeof(a2dp_src_mod.cmd_key));
    a2dp_src_mod.cmd_handler = btmw_rpc_test_a2dp_src_cmd_handler;
    a2dp_src_mod.cmd_tbl = btmw_rpc_test_a2dp_src_cli_commands;

    ret = btmw_rpc_test_register_mod(&a2dp_src_mod);
    BTMW_RPC_TEST_Logd("btmw_rpc_test_register_mod() for SRC returns: %d\n", ret);

    return ret;
}


