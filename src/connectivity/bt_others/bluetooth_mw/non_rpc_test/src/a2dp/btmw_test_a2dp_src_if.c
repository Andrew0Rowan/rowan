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
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>
//#include <sys/time.h>

#include "c_bt_mw_a2dp_common.h"
#include "c_bt_mw_a2dp_src.h"
#include "btmw_test_cli.h"
#include "btmw_test_debug.h"
#if !ENABLE_A2DP_ADEV && ENABLE_A2DP_SRC
#include "bt_a2dp_alsa_uploader.h"
#else
#include "mtk_audio.h"
#endif

#define BTMW_TEST_CMD_KEY_A2DP_SRC        "MW_A2DP_SRC"
#define PCM_BUFFER_SIZE 512*8     // at least 512
/* 0:pause, 1: play */
static pthread_t btmw_test_stream_handle = -1;
static CHAR btmw_test_pcm_file[128] = "/data/usb/music/48000/input_48000.pcm";


extern CHAR g_a2dp_addr_test[18];
extern BT_A2DP_STREAM_STATE g_a2dp_stream_state;

// CLI handler
static int btmw_test_a2dp_src_connect_int_handler(int argc, char *argv[]);
static int btmw_test_a2dp_src_disconnect_handler(int argc, char *argv[]);
static int btmw_test_a2dp_src_active_handler(int argc, char *argv[]);
static int btmw_test_a2dp_get_sink_dev_list_handler(int argc, char *argv[]);
static int btmw_test_a2dp_src_write_stream_handler(int argc, char *argv[]);
static int btmw_test_a2dp_src_set_audio_hw_dbg_lvl_handler(int argc, char *argv[]);



UINT32 bt_get_microseconds(VOID)
{
    struct timespec now;
    clock_gettime(CLOCK_MONOTONIC, &now);
    return (now.tv_sec * 1000000 + now.tv_nsec / 1000);
}

static int btmw_test_a2dp_src_set_audio_hw_dbg_lvl_handler(int argc, char *argv[])
{
    UINT8 u1_log_lvl = 0;
    BTMW_TEST_Logi("%s()\n", __func__);

    if (argc != 1)
    {
        BTMW_TEST_Loge("[USAGE] audio_hw_log <0~6>");
        return -1;
    }

    u1_log_lvl = atoi(argv[0]);
    if (0 <= u1_log_lvl && 6 >= u1_log_lvl)
    {
        c_btm_a2dp_set_dbg_flag(BT_A2DP_DBG_SET_HW_AUDIO_LOG_LV, u1_log_lvl);
    }
    else
    {
        BTMW_TEST_Loge("input error\n");
        BTMW_TEST_Loge("please input audio_hw_log <0~6>\n");
    }
    return 0;
}

static int btmw_test_a2dp_src_active_handler(int argc, char *argv[])
{
    UINT8 u1_enable = 0;
    BTMW_TEST_Logi("%s()\n", __func__);

    if (argc != 1)
    {
        BTMW_TEST_Loge("[USAGE] active_src [1:enable|0:disable]");
        return -1;
    }

    u1_enable = atoi(argv[0]);

    c_btm_a2dp_src_enable(u1_enable);

    return 0;
}



static int btmw_test_a2dp_get_sink_dev_list_handler(int argc, char *argv[])
{
    BT_A2DP_DEVICE_LIST device_list;
    INT32 ret = 0;
    BTMW_TEST_Logi("%s()\n", __func__);

    memset((void*)&device_list, 0, sizeof(device_list));

    ret = c_btm_a2dp_src_get_dev_list(&device_list);
    BTMW_TEST_Logi("get paired sink device list result:\n", __func__);
    if(BT_SUCCESS == ret)
    {
        if (0 == device_list.dev_num)
        {
            BTMW_TEST_Logi("no paired sink device\n");
        }
        else
        {
            INT32 i = 0;
            for(i=0;i<device_list.dev_num;i++)
            {
                BTMW_TEST_Logi("device[%d]: %s\n", i, device_list.dev[i].addr);
                BTMW_TEST_Logi("======================================\n");
            }
        }
    }
    else
    {
        BTMW_TEST_Logi("get paired sink device failed: %d\n", ret);
    }

    return 0;
}

static int btmw_test_a2dp_src_connect_int_handler(int argc, char *argv[])
{
    CHAR *ptr;

    BTMW_TEST_Logd("%s()\n", __func__);

    if (argc != 1)
    {
        BTMW_TEST_Loge("[USAGE] connect <addr>");
        return -1;
    }
    if (strlen(argv[0]) < 17)
    {
        BTMW_TEST_Loge("<addr> invalid. Good example is \"AA:BB:CC:DD:EE:FF\"");
        BTMW_TEST_Loge("[USAGE] connect <addr>");
        return -1;
    }

    ptr = argv[0];
    c_btm_a2dp_connect(ptr, BT_A2DP_ROLE_SRC);

    return 0;
}

static int btmw_test_a2dp_src_disconnect_handler(int argc, char *argv[])
{
    CHAR *ptr;

    BTMW_TEST_Logd("%s()\n", __func__);

    if (argc != 1)
    {
        BTMW_TEST_Loge("[USAGE] disconnect <addr>");
        return -1;
    }
    if (strlen(argv[0]) < 17)
    {
        BTMW_TEST_Loge("<addr> invalid. Good example is \"AA:BB:CC:DD:EE:FF\"");
        BTMW_TEST_Loge("[USAGE] disconnect <addr>");
        return -1;
    }

    ptr = argv[0];
    c_btm_a2dp_disconnect(ptr);
    return 0;
}

static VOID* btmw_test_write_audio_data_thread(VOID *arg)
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
    hw_device_t* device = NULL;
    struct audio_hw_device *hw_dev = NULL;
    struct audio_stream_out *stream = NULL;

    BTMW_TEST_Logd("Input file name  : %s\n", local_test_pcm_file);
    /* open file & allocate memory */
    fInputPCM = fopen(local_test_pcm_file, "rb");
    if (fInputPCM == NULL)
    {
        BTMW_TEST_Loge("Can't open input PCM file!\n");
        btmw_test_stream_handle = -1;
        return NULL;
    }
    BTMW_TEST_Logd("open input PCM file success!\n");

    //fInputDumpPCM = fopen("/data/sda1/send_before.pcm", "wb+");

    pcm_buffer = (UINT8 *)malloc(PCM_BUFFER_SIZE);
    if (pcm_buffer == NULL)
    {
        fclose(fInputPCM);
        BTMW_TEST_Loge("Can't allocat buffer\n");
        btmw_test_stream_handle = -1;
        return NULL;
    }

    if (0 > adev_open(NULL, AUDIO_HARDWARE_INTERFACE, &device))
    {
        BTMW_TEST_Loge("open bt adev fail\n");
        goto send_audio_data_end;
    }
    hw_dev = (struct audio_hw_device *)device;
    if (0 > hw_dev->open_output_stream(hw_dev, 0, 0, 0, 0, &stream, 0))
    {
        BTMW_TEST_Loge("open out stream fail\n");
        goto send_audio_data_end;
    }

#if 0
    struct timeval last;
    struct timeval tv, tv1;

    gettimeofday(&last, NULL);
#endif
    while(read_cnt < 1 && g_a2dp_addr_test[0] != 0)
    {
        memset(pcm_buffer, 0, PCM_BUFFER_SIZE);
        total_pcm_len = fread(pcm_buffer, sizeof(UINT8), pcm_frame_len, fInputPCM );
        if (total_pcm_len == 0)
        {
            BTMW_TEST_Loge("total_pcm_len==0\n");
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
    btmw_test_stream_handle = -1;
    BTMW_TEST_Logd("Send audio finished\n");
    return NULL;
}


static int btmw_test_a2dp_src_write_stream_handler(int argc, char *argv[])
{
    INT32 result;

    BTMW_TEST_Logd("%s(), file:%s\n", __func__, argv[0]);
    strcpy(btmw_test_pcm_file, argv[0]);

    if(-1 == btmw_test_stream_handle)
    {
        result = pthread_create(&btmw_test_stream_handle, NULL,
            btmw_test_write_audio_data_thread, btmw_test_pcm_file);
        if (result)
        {
            BTMW_TEST_Logd("pthread_create failed! (%d)\n", result);
        }
    }
    else
    {
        BTMW_TEST_Logw("streaming thread has been created!\n");
    }

    return 0;
}


static BTMW_TEST_CLI btmw_test_a2dp_src_cli_commands[] =
{
    {"connect",       btmw_test_a2dp_src_connect_int_handler,               " = connect <addr>"},
    {"disconnect",    btmw_test_a2dp_src_disconnect_handler,                " = disconnect <addr>"},
    {"active_src",    btmw_test_a2dp_src_active_handler,                    " = active_src <1:enable|0:disable>"},
    {"get_paired_dev",btmw_test_a2dp_get_sink_dev_list_handler,             " = get_paired_dev"},
    {"write_stream",  btmw_test_a2dp_src_write_stream_handler,              " = write_stream <file-path>"},
    {"audio_hw_log",  btmw_test_a2dp_src_set_audio_hw_dbg_lvl_handler,      " = audio_hw_log <0~6>"},
    {NULL, NULL, NULL},
};

int btmw_test_a2dp_src_cmd_handler(int argc, char **argv)
{
    BTMW_TEST_CLI *cmd, *match = NULL;
    INT32 ret = 0;
    INT32 count = 0;

    BTMW_TEST_Logd("%s argc: %d, argv[0]: %s\n", __func__, argc, argv[0]);

    cmd = btmw_test_a2dp_src_cli_commands;
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
        BTMW_TEST_Logd("Unknown command '%s'\n", argv[0]);

        btmw_test_print_cmd_help(BTMW_TEST_CMD_KEY_A2DP_SRC, btmw_test_a2dp_src_cli_commands);
        ret = -1;
    }
    else
    {
        match->handler(argc - 1, &argv[1]);
    }

    return ret;
}

INT32 btmw_test_a2dp_src_init(VOID)
{
    INT32 ret = 0;
    BTMW_TEST_MOD a2dp_src_mod = {0};

    a2dp_src_mod.mod_id = BTMW_TEST_MOD_A2DP_SRC;
    strncpy(a2dp_src_mod.cmd_key, BTMW_TEST_CMD_KEY_A2DP_SRC, sizeof(a2dp_src_mod.cmd_key));
    a2dp_src_mod.cmd_handler = btmw_test_a2dp_src_cmd_handler;
    a2dp_src_mod.cmd_tbl = btmw_test_a2dp_src_cli_commands;

    ret = btmw_test_register_mod(&a2dp_src_mod);
    BTMW_TEST_Logd("btmw_test_register_mod() for SRC returns: %d\n", ret);
#if !ENABLE_A2DP_ADEV && ENABLE_A2DP_SRC
    c_btm_a2dp_src_register_uploader(bt_a2dp_alsa_get_uploader());
    bt_a2dp_alsa_register_output_callback(c_btm_a2dp_src_send_stream_data);
#endif
    return ret;
}


