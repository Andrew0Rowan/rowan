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

/* FILE NAME:  bt_mw_a2dp_snk.c
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

#include <bt_audio_track.h>
#include "bt_mw_a2dp_snk.h"
#include "c_mw_config.h"
#include "linuxbt_gap_if.h"
#include "bt_mw_message_queue.h"

/* NAMING CONSTANT DECLARATIONS
 */
#define BT_A2DP_PLAYBACK_RE_PLAY_TIMEOUT_MS (10000)
static struct timeval gPlaybackDeinitTime;

/* MACRO FUNCTION DECLARATIONS
 */
/* DATA TYPE DECLARATIONS
 */
/* GLOBAL VARIABLE DECLARATIONS
 */
/* LOCAL SUBPROGRAM DECLARATIONS
 */
static VOID bt_mw_a2dp_sink_playback_start_ext(VOID);
static void bt_mw_a2dp_sink_playback_start(int trackFreq, int channelType);
static void bt_mw_a2dp_sink_playback_stop(void);
static void bt_mw_a2dp_sink_playback_play(void);
static void bt_mw_a2dp_sink_playback_pause(void);
static int bt_mw_a2dp_sink_playback_send_data(void *audioBuffer, int bufferLen);

static VOID bt_mw_a2dp_sink_report_player_event(BT_A2DP_PLAYER_EVENT event);
/* STATIC VARIABLE DECLARATIONS
 */
static INT32 gi4SampleRate = 0;
static INT32 gi4ChannelCnt = 0;

static BOOL g_bt_mw_a2dp_snk_report_uninit = FALSE;

static BT_A2DP_PLAYER g_bt_mw_a2dp_sink_player = {0};

BtifAvrcpAudioTrack g_bt_mw_a2dp_sink_track =
{
    sizeof(BtifAvrcpAudioTrack),
    bt_mw_a2dp_sink_playback_start,
    bt_mw_a2dp_sink_playback_stop,
    bt_mw_a2dp_sink_playback_play,
    bt_mw_a2dp_sink_playback_pause,
    bt_mw_a2dp_sink_playback_send_data,
};


/* EXPORTED SUBPROGRAM BODIES
 */
INT32 bt_mw_a2dp_sink_register_player(BT_A2DP_PLAYER *player)
{
    INT32 i4_ret = BT_SUCCESS;
    BT_MW_FUNC_ENTER(BT_DEBUG_A2DP, "callback player %p", player);

    memset(&g_bt_mw_a2dp_sink_player, 0x0, sizeof(BT_A2DP_PLAYER));

    if (NULL == player)
    {
        BT_DBG_ERROR(BT_DEBUG_A2DP, "callback player is null!");
        return BT_ERR_STATUS_NULL_POINTER;
    }
    if (player->start)
    {
        g_bt_mw_a2dp_sink_player.start = player->start;
    }
    else
    {
        BT_DBG_ERROR(BT_DEBUG_A2DP, "player_start_cb player is null!");
        i4_ret = BT_ERR_STATUS_NULL_POINTER;
    }

    if (player->play)
    {
        g_bt_mw_a2dp_sink_player.play = player->play;
    }
    else
    {
        BT_DBG_WARNING(BT_DEBUG_A2DP, "player_play_cb player is null!");
        i4_ret = BT_ERR_STATUS_NULL_POINTER;
    }

    if (player->init)
    {
        g_bt_mw_a2dp_sink_player.init = player->init;
    }
    else
    {
        BT_DBG_WARNING(BT_DEBUG_A2DP, "player_init_cb player is null!");
        i4_ret = BT_ERR_STATUS_NULL_POINTER;
    }

    if (player->deinit)
    {
        g_bt_mw_a2dp_sink_player.deinit = player->deinit;
    }
    else
    {
        BT_DBG_WARNING(BT_DEBUG_A2DP, "player_deinit_cb player is null!");
        i4_ret = BT_ERR_STATUS_NULL_POINTER;
    }

    if (player->pause)
    {
        g_bt_mw_a2dp_sink_player.pause = player->pause;
    }
    else
    {
        BT_DBG_WARNING(BT_DEBUG_A2DP, "player_pause_cb player is null!");
        i4_ret = BT_ERR_STATUS_NULL_POINTER;
    }

    if (player->stop)
    {
        g_bt_mw_a2dp_sink_player.stop = player->stop;
    }
    else
    {
        BT_DBG_WARNING(BT_DEBUG_A2DP, "player_stop_cb player is null!");
        i4_ret = BT_ERR_STATUS_NULL_POINTER;
    }

    if (player->write)
    {
        g_bt_mw_a2dp_sink_player.write = player->write;
    }
    else
    {
        BT_DBG_WARNING(BT_DEBUG_A2DP, "player_push_data_cb player is null!");
        i4_ret = BT_ERR_STATUS_NULL_POINTER;
    }

    if (player->adjust_buf_time)
    {
        g_bt_mw_a2dp_sink_player.adjust_buf_time = player->adjust_buf_time;
    }
    else
    {
        BT_DBG_WARNING(BT_DEBUG_A2DP, "player_adjust_buf_time_cb player is null!");
        i4_ret = BT_ERR_STATUS_NULL_POINTER;
    }

    return i4_ret;
}

/**
 * FUNCTION NAME: bt_mw_a2dp_sink_adjust_buffer_time
 * PURPOSE:
 *      The function is used for adjust buffer time when send data to playback
 * INPUT:
 *      buffer time
 * OUTPUT:
 *      None
 * RETURN:
 *      None
 * NOTES:
 *      None
 */
INT32 bt_mw_a2dp_sink_adjust_buffer_time(UINT32 buffer_time)
{
    BT_MW_FUNC_ENTER(BT_DEBUG_A2DP, "");
    if (g_bt_mw_a2dp_sink_player.adjust_buf_time)
    {
        g_bt_mw_a2dp_sink_player.adjust_buf_time(buffer_time);
    }
    else
    {
        BT_DBG_WARNING(BT_DEBUG_A2DP, "player_adjust_buf_time_cb is null or have init!");
    }
    return BT_SUCCESS;
}

VOID bt_mw_a2dp_sink_start_player(VOID)
{
    BT_MW_FUNC_ENTER(BT_DEBUG_A2DP, "");
    bt_mw_a2dp_sink_playback_start_ext();
    bt_mw_a2dp_sink_playback_play();
    return;
}

VOID bt_mw_a2dp_sink_stop_player(VOID)
{
    BT_MW_FUNC_ENTER(BT_DEBUG_A2DP, "");
    bt_mw_a2dp_sink_playback_pause();
    bt_mw_a2dp_sink_playback_stop();
    return;
}

/* LOCAL SUBPROGRAM BODIES
 */


/**
 * FUNCTION NAME: bt_mw_a2dp_sink_playback_start_ext
 * PURPOSE:
 *      The function is used for init playback and triggered by APP layer.
 * INPUT:
 *      None
 * OUTPUT:
 *      None
 * RETURN:
 *      None
 * NOTES:
 *      None
 */

static VOID bt_mw_a2dp_sink_playback_start_ext(VOID)
{
    BT_MW_FUNC_ENTER(BT_DEBUG_A2DP, "");
    if (g_bt_mw_a2dp_sink_player.start && (!g_bt_mw_a2dp_sink_player.started))
    {
#if ENABLE_BT_WIFI_RATIO_SETTING
        //bt_ratio: 04, wifi_ratio: 01, slot: (24*4+5):31
        linuxbt_gap_set_bt_wifi_ratio(0x04, 0x01);
        BT_DBG_ERROR(BT_DEBUG_A2DP, "BT/Wifi: 4/1");
#endif
        g_bt_mw_a2dp_sink_player.start(gi4SampleRate, gi4ChannelCnt);
        g_bt_mw_a2dp_sink_player.started = 1;
    }
    else
    {
        BT_DBG_WARNING(BT_DEBUG_A2DP, "player_init_cb is null or have init!");
    }
}


/**
 * FUNCTION NAME: bt_mw_a2dp_sink_playback_start
 * PURPOSE:
 *      The function is used for init playback and triggered by stack audio track.
 * INPUT:
 *      trackFreq                -- samplerate
 *      channelType            --channel number
 * OUTPUT:
 *      None
 * RETURN:
 *      None
 * NOTES:
 *      None
 */

static void bt_mw_a2dp_sink_playback_start(int trackFreq, int channelType)
{
    BT_MW_FUNC_ENTER(BT_DEBUG_A2DP, "fs:%d channel:%d", trackFreq, channelType);
    gi4SampleRate = trackFreq;
    gi4ChannelCnt = channelType;
    g_bt_mw_a2dp_snk_report_uninit = FALSE;

#if DISPATCH_A2DP_WITH_PLAYBACK
    BT_DBG_NORMAL(BT_DEBUG_A2DP, "BT playback init should trigger by APP");
#else
    if (g_bt_mw_a2dp_sink_player.start && (!g_bt_mw_a2dp_sink_player.started))
    {
#if ENABLE_BT_WIFI_RATIO_SETTING
        //bt_ratio: 04, wifi_ratio: 01, slot: (24*4+5):31
        linuxbt_gap_set_bt_wifi_ratio(0x04, 0x01);
        BT_DBG_ERROR(BT_DEBUG_A2DP, "BT/Wifi: 4/1");
#endif
        g_bt_mw_a2dp_sink_player.start(trackFreq, channelType);
        g_bt_mw_a2dp_sink_player.started = 1;
    }
    else
    {
        BT_DBG_WARNING(BT_DEBUG_A2DP, "player_init_cb is null or have init!");
    }
#endif
}

void bt_mw_a2dp_sink_playback_stop(void)
{
    BT_MW_FUNC_ENTER(BT_DEBUG_A2DP, "");

    if (g_bt_mw_a2dp_sink_player.stop && g_bt_mw_a2dp_sink_player.started)
    {
        g_bt_mw_a2dp_sink_player.stop();
#if ENABLE_BT_WIFI_RATIO_SETTING
        //bt_ratio: 01, wifi_ratio: 02, slot: (24/2+5):31
        linuxbt_gap_set_bt_wifi_ratio(0x01, 0x02);
        BT_DBG_ERROR(BT_DEBUG_A2DP, "BT/Wifi: 1/2");
#endif
        g_bt_mw_a2dp_sink_player.started = 0;
        gettimeofday(&gPlaybackDeinitTime, NULL);
        BT_DBG_WARNING(BT_DEBUG_A2DP, "playback deinit@%lu.%06lu",
            gPlaybackDeinitTime.tv_sec, gPlaybackDeinitTime.tv_usec);

    }
    else
    {
        BT_DBG_WARNING(BT_DEBUG_A2DP, "player_deinit_cb is null or have deinit!");
    }
}

static void bt_mw_a2dp_sink_playback_play(void)
{
    BT_MW_FUNC_ENTER(BT_DEBUG_A2DP, "");
    if (g_bt_mw_a2dp_sink_player.play && g_bt_mw_a2dp_sink_player.started)
    {
        g_bt_mw_a2dp_sink_player.play();
    }
    else
    {
        BT_DBG_WARNING(BT_DEBUG_A2DP, "player_start_cb is null or playback not init!");
    }
}

static void bt_mw_a2dp_sink_playback_pause(void)
{
    BT_MW_FUNC_ENTER(BT_DEBUG_A2DP, "");
    if (g_bt_mw_a2dp_sink_player.pause && g_bt_mw_a2dp_sink_player.started)
    {
        g_bt_mw_a2dp_sink_player.pause();
    }
    else
    {
        BT_DBG_WARNING(BT_DEBUG_A2DP, "player_pause_cb is null or playback not init!");
    }
}

static int bt_mw_a2dp_sink_playback_send_data(void *audioBuffer, int bufferLen)
{
    if (g_bt_mw_a2dp_sink_player.write)
    {
        if (g_bt_mw_a2dp_sink_player.started)
        {
            g_bt_mw_a2dp_sink_player.write(audioBuffer, bufferLen);
        }
        else
        {
            if (FALSE == g_bt_mw_a2dp_snk_report_uninit)
            {
                g_bt_mw_a2dp_snk_report_uninit = TRUE;
                BT_DBG_WARNING(BT_DEBUG_A2DP, "playback not init, discard data!");
            }

            struct timeval tv;
            gettimeofday(&tv, NULL);
            if (((tv.tv_sec - gPlaybackDeinitTime.tv_sec) * 1000
                + ((tv.tv_usec - gPlaybackDeinitTime.tv_usec) / 1000))
                > BT_A2DP_PLAYBACK_RE_PLAY_TIMEOUT_MS)
            {
                /* update this time or it will trigger many times */
                gettimeofday(&gPlaybackDeinitTime, NULL);
                BT_DBG_WARNING(BT_DEBUG_A2DP,
                    "playback deinit, still rx data@%lu.%06lu, trigger playing",
                    tv.tv_sec, tv.tv_usec);
                bt_mw_a2dp_sink_report_player_event(BT_A2DP_ALSA_PB_EVENT_DATA_COME);
            }

        }
    }
    else
    {
        BT_DBG_WARNING(BT_DEBUG_A2DP, "player_push_data_cb is null!");
        return BT_ERR_STATUS_FAIL;
    }
    return BT_SUCCESS;
}

static VOID bt_mw_a2dp_sink_report_player_event(BT_A2DP_PLAYER_EVENT event)
{
    tBTMW_MSG btmw_msg = {0};

    btmw_msg.hdr.event = BTMW_A2DP_PLAYER_REPORT_EVENT;
    btmw_msg.data.a2dp_msg.data.player_event = event;

    btmw_msg.hdr.len = sizeof(btmw_msg.data.a2dp_msg);
    linuxbt_send_msg(&btmw_msg);
}

INT32 bt_mw_a2dp_sink_player_init(VOID)
{
    BT_MW_FUNC_ENTER(BT_DEBUG_A2DP, "");

    if (NULL != g_bt_mw_a2dp_sink_player.init)
    {
        g_bt_mw_a2dp_sink_player.init(bt_mw_a2dp_sink_report_player_event);
    }

    return BtifAvrcpAudioTrackInit(&g_bt_mw_a2dp_sink_track);
}

INT32 bt_mw_a2dp_sink_player_deinit(VOID)
{
    BT_MW_FUNC_ENTER(BT_DEBUG_A2DP, "");
    if (NULL != g_bt_mw_a2dp_sink_player.deinit)
    {
        g_bt_mw_a2dp_sink_player.deinit();
    }
    return BtifAvrcpAudioTrackDeinit();
}


#if defined(BT_RPC_DBG_SERVER)
EXPORT_SYMBOL int dbg_a2dp_get_gi4SampleRate(int array_index, int offset, char *name, char *data, int length)
{
    if (offset >= 1)
    {
        return 0;
    }

    sprintf(name, "gi4SampleRate");
    //sprintf(data, "%ld", gi4SampleRate);
    return offset + 1;
}

EXPORT_SYMBOL int dbg_a2dp_get_gi4ChannelCnt(int array_index, int offset, char *name, char *data, int length)
{
    if (offset >= 1)
    {
        return 0;
    }

    sprintf(name, "gi4ChannelCnt");
    //sprintf(data, "%ld", gi4ChannelCnt);
    return offset + 1;
}
#endif
