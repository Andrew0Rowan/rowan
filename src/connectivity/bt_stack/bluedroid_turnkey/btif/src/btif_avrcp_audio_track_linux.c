/*
 * Copyright (C) 2015 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#define LOG_TAG "linuxbt_audio_track"

#include <assert.h>
#include <stdio.h>
#include "bt_common.h"
#include "btif_avrcp_audio_track.h"
#include "hardware/bt_audio_track.h"

BtifAvrcpAudioTrack *g_pt_track = NULL;

//#define DUMP_PCM_DATA TRUE
#if (defined(DUMP_PCM_DATA) && (DUMP_PCM_DATA == TRUE))
FILE *outputPcmSampleFile;
char outputFilename[128] = BT_TMP_PATH"/bluedroid_output_sample.pcm";
#endif

EXPORT_SYMBOL int BtifAvrcpAudioTrackInit(BtifAvrcpAudioTrack *pt_track_cb)
{
    BTIF_TRACE_API(LOG_TAG, "%s", __func__);
    if(NULL == pt_track_cb)
    {
        BTIF_TRACE_ERROR(LOG_TAG, "%s invalid callback func pointer", __func__);
        return -1;
    }
    g_pt_track = pt_track_cb;
    return 0;
}

EXPORT_SYMBOL int BtifAvrcpAudioTrackDeinit(void)
{
    BTIF_TRACE_API(LOG_TAG, "%s", __func__);

    g_pt_track = NULL;
    return 0;
}

void *BtifAvrcpAudioTrackCreate(int trackFreq, int channelType)
{
    BTIF_TRACE_API(LOG_TAG, "%s btCreateTrack freq %d  channel %d ",
                     __func__, trackFreq, channelType);

#if (defined(DUMP_PCM_DATA) && (DUMP_PCM_DATA == TRUE))
    outputPcmSampleFile = fopen(outputFilename, "wb+");
#endif
    if (g_pt_track && g_pt_track->linux_bt_pb_init)
    {
        g_pt_track->linux_bt_pb_init(trackFreq, channelType);
        return (void *)g_pt_track;
    }
    else
    {
        BTIF_TRACE_WARNING(LOG_TAG, "null %s", __func__);
    }
    return (void *)NULL;
}

void BtifAvrcpAudioTrackStart(void *handle)
{
    assert(handle != NULL);
    BtifAvrcpAudioTrack *trackHolder = (BtifAvrcpAudioTrack*)handle;
    assert(trackHolder != NULL);
    BTIF_TRACE_API(LOG_TAG, "%s btStartTrack", __func__);
    if (trackHolder && trackHolder->linux_bt_pb_play)
    {
        trackHolder->linux_bt_pb_play();
    }
    else
    {
        BTIF_TRACE_WARNING(LOG_TAG, "null %s", __func__);
    }
}

void BtifAvrcpAudioTrackStop(void *handle)
{
    if (handle == NULL)
    {
        BTIF_TRACE_WARNING(LOG_TAG, "%s handle is null.", __func__);
        return;
    }
    /*currently not use it for playback*/
}

void BtifAvrcpAudioTrackDelete(void *handle)
{
    if (handle == NULL)
    {
        BTIF_TRACE_WARNING(LOG_TAG, "%s handle is null.", __func__);
        return;
    }
    BtifAvrcpAudioTrack *trackHolder = (BtifAvrcpAudioTrack*)handle;
    assert(trackHolder != NULL);
    if (trackHolder != NULL)
    {
        BTIF_TRACE_API(LOG_TAG, "%s btStartTrack", __func__);
        if (trackHolder && trackHolder->linux_bt_pb_deinit)
        {
            trackHolder->linux_bt_pb_deinit();
        }
        else
        {
            BTIF_TRACE_WARNING(LOG_TAG, "null %s", __func__);
        }
    }

#if (defined(DUMP_PCM_DATA) && (DUMP_PCM_DATA == TRUE))
    if (outputPcmSampleFile)
    {
        fclose(outputPcmSampleFile);
    }
    outputPcmSampleFile = NULL;
#endif
}

void BtifAvrcpAudioTrackPause(void *handle)
{
    if (handle == NULL)
    {
        BTIF_TRACE_WARNING(LOG_TAG, "%s handle is null.", __func__);
        return;
    }
    BtifAvrcpAudioTrack *trackHolder = (BtifAvrcpAudioTrack*)handle;
    assert(trackHolder != NULL);
    if (trackHolder != NULL)
    {
        BTIF_TRACE_API(LOG_TAG, "%s btStartTrack", __func__);
        if (trackHolder && trackHolder->linux_bt_pb_pause)
        {
            trackHolder->linux_bt_pb_pause();
        }
        else
        {
            BTIF_TRACE_WARNING(LOG_TAG, "null %s", __func__);
        }
    }
}

int BtifAvrcpAudioTrackWriteData(void *handle, void *audioBuffer, int bufferlen)
{
    BtifAvrcpAudioTrack *trackHolder = (BtifAvrcpAudioTrack*)handle;
    assert(trackHolder != NULL);
    int retval = -1;
#if (defined(DUMP_PCM_DATA) && (DUMP_PCM_DATA == TRUE))
    if (outputPcmSampleFile)
    {
        fwrite ((audioBuffer), 1, (size_t)bufferlen, outputPcmSampleFile);
    }
#endif
    if (trackHolder && trackHolder->linux_bt_pb_push_data)
    {
        retval = trackHolder->linux_bt_pb_push_data(audioBuffer, (size_t)bufferlen);
        BTIF_TRACE_VERBOSE(LOG_TAG, "%s btWriteData len = %d ret = %d",
                     __func__, bufferlen, retval);
        return retval;
    }
    else
    {
        BTIF_TRACE_WARNING(LOG_TAG, "null %s", __func__);
    }

    return retval;
}
