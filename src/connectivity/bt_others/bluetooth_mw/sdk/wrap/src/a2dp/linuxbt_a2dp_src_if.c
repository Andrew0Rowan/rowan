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

/* FILE NAME:  linuxbt_a2dp_src_if.c
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
#include <string.h>
#include <sys/time.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <sys/un.h>
#include <errno.h>
#include <time.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <pthread.h>

#include "bluetooth.h"
#include "bt_mw_common.h"
#include "bt_mw_a2dp_common.h"
#include "linuxbt_common.h"
#include "mtk_bluetooth.h"
#if defined(MTK_LINUX_A2DP_PLUS) && (MTK_LINUX_A2DP_PLUS == TRUE)
#include "mtk_bt_av.h"
#else
#include "bt_av.h"
#endif
#include "c_mw_config.h"
#if defined(MTK_LINUX_AVRCP_PLUS) && (MTK_LINUX_AVRCP_PLUS == TRUE)
#include "mtk_bt_rc.h"
#else
#include "bt_rc.h"
#endif
#include "bt_audio_track.h"
#include "mtk_audio.h"

#include "linuxbt_gap_if.h"
#include "linuxbt_a2dp_src_if.h"

#include "bt_mw_message_queue.h"

/* NAMING CONSTANT DECLARATIONS
 */
/* MACRO FUNCTION DECLARATIONS
 */
#define LINUXBT_A2DP_SRC_SET_MSG_LEN(msg) do{   \
    msg.hdr.len = sizeof(tBT_MW_A2DP_MSG);      \
    }while(0)
/* DATA TYPE DECLARATIONS
 */
/* GLOBAL VARIABLE DECLARATIONS
 */
/* LOCAL SUBPROGRAM DECLARATIONS
 */
/* STATIC VARIABLE DECLARATIONS
 */
/* EXPORTED SUBPROGRAM BODIES
 */
/* LOCAL SUBPROGRAM BODIES
 */


#if !ENABLE_A2DP_ADEV
struct audio_hw_device g_audio_dev;
struct audio_stream_out *g_stream_out;
static BOOL g_fg_audio_path_init = FALSE;
static pthread_mutex_t g_adev_lock = PTHREAD_MUTEX_INITIALIZER;

#ifdef A2DP_SRC_STAT_TX_SPEED
static int g_start_stat = 0; /* if start stat send data */
static struct timeval g_start_tx_tm;
static int g_tx_data_bytes = 0;
static struct timeval g_last_tx_tm;
#endif

extern int adev_open_output_stream(struct audio_hw_device *dev,
                                       audio_io_handle_t handle,
                                       audio_devices_t devices,
                                       audio_output_flags_t flags,
                                       struct audio_config *config,
                                       struct audio_stream_out **stream_out,
                                       const char *address);
extern void adev_close_output_stream(struct audio_hw_device *dev,
                                         struct audio_stream_out *stream);
#endif
extern uint8_t audio_set_trace_level(uint8_t new_level);


// Callback functions
static void linuxbt_a2dp_src_connection_state_cb(btav_connection_state_t state,
                                                        bt_bdaddr_t *bd_addr);
static void linuxbt_a2dp_src_audio_state_cb(btav_audio_state_t state,
                                                   bt_bdaddr_t* bd_addr);
static void linuxbt_a2dp_src_audio_config_cb(bt_bdaddr_t *bd_addr,
                                                    uint32_t sample_rate,
                                                    uint8_t channel_count);

#if !ENABLE_A2DP_ADEV
VOID init_audio_path(VOID)
{
    BT_DBG_NORMAL(BT_DEBUG_A2DP, "[A2DP] Enter+++ %s()", __FUNCTION__);
    pthread_mutex_lock(&g_adev_lock);
    memset(&g_audio_dev, 0x0, sizeof(g_audio_dev));
    g_stream_out = NULL;
    adev_open_output_stream(&g_audio_dev, 0, 0, 0, NULL, &g_stream_out, NULL);
    g_fg_audio_path_init = TRUE;
    pthread_mutex_unlock(&g_adev_lock);
    BT_DBG_NORMAL(BT_DEBUG_A2DP, "[A2DP] Exit--- %s()", __FUNCTION__);
}

VOID uninit_audio_path(VOID)
{
    BT_DBG_NORMAL(BT_DEBUG_A2DP, "[A2DP] Enter+++ %s()", __FUNCTION__);
    pthread_mutex_lock(&g_adev_lock);
    if (NULL != g_stream_out)
    {
        g_stream_out->common.set_parameters(&g_stream_out->common, "closing=true");
        g_stream_out->common.standby(&g_stream_out->common);
        adev_close_output_stream(&g_audio_dev, g_stream_out);
        g_stream_out = NULL;
        g_fg_audio_path_init = FALSE;
    }
    pthread_mutex_unlock(&g_adev_lock);
#ifdef A2DP_SRC_STAT_TX_SPEED
    BT_DBG_NORMAL(BT_DEBUG_A2DP, "[A2DP] tx bytes: %d, tx time: %ld us",
        g_tx_data_bytes, (g_last_tx_tm.tv_sec-g_start_tx_tm.tv_sec)*1000000
        +(g_last_tx_tm.tv_usec-g_start_tx_tm.tv_usec));
#endif
    BT_DBG_NORMAL(BT_DEBUG_A2DP, "[A2DP] Exit--- %s()", __FUNCTION__);
}
#endif

static btav_interface_t *g_bt_a2dp_src_interface = NULL;
static btav_callbacks_t g_bt_a2dp_src_callbacks =
{
    sizeof(btav_callbacks_t),
    linuxbt_a2dp_src_connection_state_cb,
    linuxbt_a2dp_src_audio_state_cb,
    linuxbt_a2dp_src_audio_config_cb,
};

#if defined(MTK_LINUX_A2DP_PLUS) && (MTK_LINUX_A2DP_PLUS == TRUE)
static btav_src_ext_interface_t *g_bt_a2dp_src_ext_interface = NULL;
static btav_src_ext_callbacks_t g_bt_a2dp_src_ext_callbacks =
{
    sizeof(btav_src_ext_callbacks_t),
};
#endif

static void linuxbt_a2dp_src_connection_state_cb(btav_connection_state_t state,
                                                        bt_bdaddr_t *bd_addr)
{
    tBTMW_MSG btmw_msg = {0};

    linuxbt_btaddr_htos(bd_addr, btmw_msg.data.a2dp_msg.addr);
    btmw_msg.data.a2dp_msg.role = BT_MW_A2DP_ROLE_SRC;
    BT_MW_FUNC_ENTER(BT_DEBUG_A2DP, "addr=%s", btmw_msg.data.a2dp_msg.addr);

    BT_DBG_NORMAL(BT_DEBUG_A2DP, "[A2DP] src state: %s(%d) ",
                  state==BTAV_CONNECTION_STATE_DISCONNECTED?"DISCONNECTED":
                  state==BTAV_CONNECTION_STATE_CONNECTING?"CONNECTING":
                  state==BTAV_CONNECTION_STATE_CONNECTED?"CONNECTED":
                  state==BTAV_CONNECTION_STATE_DISCONNECTING?"DISCONNECTING":
                  "UNKNOWN",
                  state);



    if (state == BTAV_CONNECTION_STATE_DISCONNECTED)
    {
        btmw_msg.hdr.event = BTMW_A2DP_DISCONNECTED;
    }
    else if (state == BTAV_CONNECTION_STATE_CONNECTED)
    {
        btmw_msg.hdr.event = BTMW_A2DP_CONNECTED;
    }
    else if (state == BTAV_CONNECTION_STATE_DISCONNECTING)
    {
        btmw_msg.hdr.event = BTMW_A2DP_STREAM_CLOSING;

    }
    else if (state==BTAV_CONNECTION_STATE_CONNECTING)
    {
        btmw_msg.hdr.event = BTMW_A2DP_CONNECTING;
    }
    LINUXBT_A2DP_SRC_SET_MSG_LEN(btmw_msg);
    linuxbt_send_msg(&btmw_msg);
}

static void linuxbt_a2dp_src_audio_state_cb(btav_audio_state_t state,
                                                bt_bdaddr_t *bd_addr)
{

    tBTMW_MSG btmw_msg = {0};

    linuxbt_btaddr_htos(bd_addr, btmw_msg.data.a2dp_msg.addr);
    btmw_msg.data.a2dp_msg.role = BT_MW_A2DP_ROLE_SRC;
    BT_DBG_NORMAL(BT_DEBUG_A2DP, "[A2DP] state: %s(%d) ",
                  state==BTAV_AUDIO_STATE_STARTED?"STARTED":
                  state==BTAV_AUDIO_STATE_STOPPED?"STOPPED":
                  state==BTAV_AUDIO_STATE_REMOTE_SUSPEND?"SUSPEND":"UNKNOWN",
                  state);
    BT_MW_FUNC_ENTER(BT_DEBUG_A2DP, "addr=%s", btmw_msg.data.a2dp_msg.addr);

    if (BTAV_AUDIO_STATE_STARTED == state)
    {
#ifdef A2DP_SRC_STAT_TX_SPEED
        g_start_stat = 1;
#endif
        btmw_msg.hdr.event = BTMW_A2DP_STREAM_START;
    }
    else if (BTAV_AUDIO_STATE_REMOTE_SUSPEND == state)
    {
#ifdef A2DP_SRC_STAT_TX_SPEED
        g_start_stat = 0;
#endif
        btmw_msg.hdr.event = BTMW_A2DP_STREAM_SUSPEND;
    }
    else if (BTAV_AUDIO_STATE_STOPPED == state)
    {
#ifdef A2DP_SRC_STAT_TX_SPEED
        g_start_stat = 0;
#endif
        btmw_msg.hdr.event = BTMW_A2DP_STREAM_SUSPEND;
    }
    LINUXBT_A2DP_SRC_SET_MSG_LEN(btmw_msg);
    linuxbt_send_msg(&btmw_msg);
}

static void linuxbt_a2dp_src_audio_config_cb(bt_bdaddr_t *bd_addr,
                                                uint32_t sample_rate,
                                                uint8_t channel_count)
{
    tBTMW_MSG btmw_msg = {0};
    BT_MW_FUNC_ENTER(BT_DEBUG_A2DP, "sample_rate: %d, channel_count: %d",
        sample_rate, channel_count);

    BT_DBG_INFO(BT_DEBUG_A2DP, "sample_rate: %d, channel_count: %d",
        sample_rate, channel_count);

    btmw_msg.hdr.event = BTMW_A2DP_STREAM_CONFIG;
    linuxbt_btaddr_htos(bd_addr, btmw_msg.data.a2dp_msg.addr);
    btmw_msg.data.a2dp_msg.role = BT_MW_A2DP_ROLE_SRC;
    BT_DBG_NORMAL(BT_DEBUG_A2DP,"addr: %s", btmw_msg.data.a2dp_msg.addr);

    btmw_msg.data.a2dp_msg.data.codec_config.sample_rate = sample_rate;
    btmw_msg.data.a2dp_msg.data.codec_config.channel_num = channel_count;

    LINUXBT_A2DP_SRC_SET_MSG_LEN(btmw_msg);
    linuxbt_send_msg(&btmw_msg);
}

INT32 linuxbt_a2dp_src_connect(CHAR *pbt_addr)
{
    INT32 status = BT_SUCCESS;
    bt_bdaddr_t bdaddr;

    BT_MW_FUNC_ENTER(BT_DEBUG_A2DP, "addr=%s", pbt_addr);
    if (NULL == pbt_addr)
    {
        BT_DBG_ERROR(BT_DEBUG_A2DP, "null pointer");
        return BT_ERR_STATUS_FAIL;
    }
    linuxbt_btaddr_stoh(pbt_addr, &bdaddr);
    BT_DBG_INFO(BT_DEBUG_A2DP, "A2DP connected to %s as SOURCE", pbt_addr);
    if (g_bt_a2dp_src_interface && g_bt_a2dp_src_interface->connect)
    {
        g_bt_a2dp_src_interface->connect(&bdaddr);
    }
    else
    {
        BT_DBG_ERROR(BT_DEBUG_A2DP, "[A2DP] A2DP SRC is not enable");
    }

    return status;
}

INT32 linuxbt_a2dp_src_disconnect(CHAR *pbt_addr)
{
    INT32 status = BT_SUCCESS;
    bt_bdaddr_t bdaddr;
    BT_MW_FUNC_ENTER(BT_DEBUG_A2DP, "addr=%s", pbt_addr);
    if ((g_bt_a2dp_src_interface == NULL) )
    {
        BT_DBG_ERROR(BT_DEBUG_A2DP, "[A2DP] Failed to get A2DP SRC interface");
        return BT_ERR_STATUS_NOT_READY;
    }

    BT_DBG_NOTICE(BT_DEBUG_A2DP, "[A2DP] linuxbt_a2dp_snk_disconnect_handler");

    linuxbt_btaddr_stoh(pbt_addr, &bdaddr);
    BT_DBG_NORMAL(BT_DEBUG_A2DP, "A2DP disconnected to %s", pbt_addr);

    if (g_bt_a2dp_src_interface && g_bt_a2dp_src_interface->disconnect)
    {
        status = g_bt_a2dp_src_interface->disconnect(&bdaddr);
    }
    else
    {
        BT_DBG_ERROR(BT_DEBUG_A2DP, "[A2DP] A2DP SRC is not enable");
    }

    return status;
}

INT32 linuxbt_a2dp_src_start_handler(CHAR *pbt_addr)
{
    BT_MW_FUNC_ENTER(BT_DEBUG_A2DP, "addr=%s", pbt_addr);
#if !ENABLE_A2DP_ADEV
    bt_bdaddr_t bdaddr;

    if (g_bt_a2dp_src_interface == NULL)
    {
        BT_DBG_ERROR(BT_DEBUG_A2DP, "[A2DP] Failed to get A2DP interface");
        return BT_ERR_STATUS_NOT_READY;
    }

    linuxbt_btaddr_stoh(pbt_addr, &bdaddr);
    BT_DBG_INFO(BT_DEBUG_A2DP, "A2DP start stream with %s", pbt_addr);
    pthread_mutex_lock(&g_adev_lock);
    if (g_fg_audio_path_init && NULL != g_stream_out)
    {
        g_stream_out->common.set_parameters(&g_stream_out->common, "A2dpSuspended=false");
    }
    pthread_mutex_unlock(&g_adev_lock);
#else
    BT_DBG_ERROR(BT_DEBUG_A2DP, "don't support start from mw");
#endif
    return BT_SUCCESS;
}


INT32 linuxbt_a2dp_src_suspend_handler(CHAR *pbt_addr)
{
    BT_MW_FUNC_ENTER(BT_DEBUG_A2DP, "addr=%s", pbt_addr);
#if !ENABLE_A2DP_ADEV
    bt_bdaddr_t bdaddr;

    if (g_bt_a2dp_src_interface == NULL)
    {
        BT_DBG_ERROR(BT_DEBUG_A2DP, "[A2DP] Failed to get A2DP interface");
        return BT_ERR_STATUS_NOT_READY;
    }
    linuxbt_btaddr_stoh(pbt_addr, &bdaddr);
    BT_DBG_INFO(BT_DEBUG_A2DP, "A2DP suspend stream with %s", pbt_addr);

    pthread_mutex_lock(&g_adev_lock);
    if (g_fg_audio_path_init && NULL != g_stream_out)
    {
        g_stream_out->common.set_parameters(&g_stream_out->common, "A2dpSuspended=true");
    }
    pthread_mutex_unlock(&g_adev_lock);
#else
    BT_DBG_ERROR(BT_DEBUG_A2DP, "don't support suspend from mw");
#endif
    return BT_SUCCESS;
}



INT32 linuxbt_a2dp_src_init(VOID)
{
    BT_MW_FUNC_ENTER(BT_DEBUG_A2DP, "");
    INT32 ret = 0;

    // Get A2DP SRC interface
    g_bt_a2dp_src_interface = (btav_interface_t *) linuxbt_gap_get_profile_interface(BT_PROFILE_ADVANCED_AUDIO_ID);
    if (g_bt_a2dp_src_interface == NULL)
    {
        BT_DBG_ERROR(BT_DEBUG_A2DP, "[A2DP] Failed to get A2DP interface");
        return BT_ERR_STATUS_FAIL;
    }

    // Init A2DP interface
    ret = g_bt_a2dp_src_interface->init(&g_bt_a2dp_src_callbacks);
    if (ret == BT_STATUS_SUCCESS)
    {
        BT_DBG_NORMAL(BT_DEBUG_A2DP, "[A2DP] success to init A2DP interface");
    }
    else if (ret == BT_STATUS_DONE)
    {
        BT_DBG_NORMAL(BT_DEBUG_A2DP, "[A2DP] already init A2DP interface");
    }
    else
    {
        BT_DBG_ERROR(BT_DEBUG_A2DP, "[A2DP] Failed to init A2DP interface");
    }

#if defined(MTK_LINUX_A2DP_PLUS) && (MTK_LINUX_A2DP_PLUS == TRUE)
    // Get A2DP SRC ext interface
    g_bt_a2dp_src_ext_interface = (btav_src_ext_interface_t *) linuxbt_gap_get_profile_interface(BT_PROFILE_ADVANCED_AUDIO_SRC_EXT_ID);
    if (NULL == g_bt_a2dp_src_ext_interface)
    {
        BT_DBG_ERROR(BT_DEBUG_A2DP, "Failed to get A2DP SRC extended interface");
        return -1;
    }
    else
    {
        BT_DBG_NORMAL(BT_DEBUG_A2DP, "get A2DP SRC extended interface successfully");
    }
    // Init A2DP SRC ext interface
    ret = g_bt_a2dp_src_ext_interface->init(&g_bt_a2dp_src_ext_callbacks);
    if (ret == BT_STATUS_SUCCESS)
    {
        BT_DBG_NORMAL(BT_DEBUG_A2DP, "init A2DP SRC extended interface successfully");
    }
    else
    {
        BT_DBG_NORMAL(BT_DEBUG_A2DP, "Fail to init A2DP SRC extended interface");
    }
#endif

    return linuxbt_return_value_convert(ret);
}

VOID linuxbt_a2dp_src_deinit(VOID)
{
    BT_MW_FUNC_ENTER(BT_DEBUG_A2DP, "");

    if (g_bt_a2dp_src_interface != NULL)
    {
        g_bt_a2dp_src_interface->cleanup();
    }
    g_bt_a2dp_src_interface = NULL;
    FUNC_EXIT;
}

INT32 linuxbt_send_stream_data(const CHAR *data, INT32 len)
{
#if !ENABLE_A2DP_ADEV
    INT32 i4_ret = 0;
    if (g_bt_a2dp_src_interface == NULL)
    {
        BT_DBG_ERROR(BT_DEBUG_A2DP, "[A2DP] Failed to get A2DP interface");
        return BT_ERR_STATUS_FAIL;
    }
    BT_DBG_INFO(BT_DEBUG_A2DP, "[A2DP] send data length:%ld begin", (long)len);
#ifdef A2DP_SRC_STAT_TX_SPEED
    if (1 == g_start_stat)
    {
        gettimeofday(&g_start_tx_tm, NULL);
        g_tx_data_bytes = 0;
        g_start_stat = 0;
    }
    else (2 == g_start_stat)
    {
        gettimeofday(&g_last_tx_tm, NULL);
        g_tx_data_bytes += len;
    }
#endif

    if (0 == pthread_mutex_trylock(&g_adev_lock))
    {
        if (g_fg_audio_path_init && (NULL != g_stream_out) && (NULL != g_stream_out->write))
        {
            i4_ret = g_stream_out->write(g_stream_out, data, len);
            BT_DBG_INFO(BT_DEBUG_A2DP, "[A2DP] i4_ret:%ld", (long)i4_ret);
        }
        pthread_mutex_unlock(&g_adev_lock);

        BT_DBG_INFO(BT_DEBUG_A2DP, "[A2DP] send data length:%ld end", (long)len);
    }
    else
    {
        BT_DBG_INFO(BT_DEBUG_A2DP, "[A2DP] audio path not init, drop it.");
    }
#else
    BT_DBG_ERROR(BT_DEBUG_A2DP, "don't support send stream from mw");
#endif
    return BT_SUCCESS;
}

INT32 linuxbt_a2dp_set_audio_hw_log_lvl(UINT8 log_level)
{
    BT_MW_FUNC_ENTER(BT_DEBUG_A2DP, "log_level=%d", log_level);
    return audio_set_trace_level(log_level);
}

