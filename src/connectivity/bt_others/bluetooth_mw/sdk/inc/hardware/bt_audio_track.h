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
#ifndef BT_AUDIO_TRACK_H
#define BT_AUDIO_TRACK_H
#include <linux/types.h>

typedef void (*BtPlaybackInit)(int trackFreq, int channelType);
typedef void (*BtPlaybackDeinit)(void);
typedef void (*BtPlaybackPlay)(void);
typedef void (*BtPlaybackPause)(void);
typedef int (*BtPlaybackPushData)(void *audioBuffer, int bufferlen);

typedef struct
{
    size_t      size;
    BtPlaybackInit linux_bt_pb_init;
    BtPlaybackDeinit linux_bt_pb_deinit;
    BtPlaybackPlay linux_bt_pb_play;
    BtPlaybackPause linux_bt_pb_pause;
    BtPlaybackPushData linux_bt_pb_push_data;
} BtifAvrcpAudioTrack;

int BtifAvrcpAudioTrackInit(BtifAvrcpAudioTrack *pt_track_cb);
int BtifAvrcpAudioTrackDeinit(void);

#endif
