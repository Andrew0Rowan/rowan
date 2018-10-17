/*
 * Copyright (c) 2017 Baidu, Inc. All Rights Reserved.
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */

#ifndef DUEROS_DCS_APP_MEDIAPLAYER_AUDIOPLAYERIMPL_H_
#define DUEROS_DCS_APP_MEDIAPLAYER_AUDIOPLAYERIMPL_H_

#include <string>
#include <unistd.h>
#include <sys/time.h>
#include <atomic>
#include "AlsaController.h"
#include "PcmBufPool.h"
#include "TimerUtil.h"
#include "AudioPlayerInterface.h"
#include "AudioPlayerListener.h"
#include "DCSApp/ThreadPoolExecutor.h"

#ifdef __cplusplus
extern "C"
{
#endif
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswresample/swresample.h>
#ifdef __cplusplus
};
#endif

namespace duerOSDcsApp {
namespace mediaPlayer {
using application::ThreadPoolExecutor;

class AudioPlayerImpl : public IAudioPlayer,
                        public StreamInterruptListener {
public:
    AudioPlayerImpl(const std::string &audio_dev);

    ~AudioPlayerImpl();

    void registerListener(AudioPlayerListener *notify);

    void audioPlay(const std::string &url,
                    RES_FORMAT format,
                    unsigned int offset,
                    unsigned int report_interval);

    void audioStop();

    void audioPause();

    void audioResume();

    unsigned long getProgress();

    unsigned long getDuration();

private:
    AudioPlayerImpl(const AudioPlayerImpl &);

    AudioPlayerImpl &operator=(const AudioPlayerImpl &);

    bool retryDecode();

    void freeFormatContext();

    void freeCodecContext();

    void handleHungryState();

    bool seekBy(unsigned int offsetMs);

    void pushStream(unsigned int channels, const void *buffer, unsigned long buff_size);

    void onStreamInterrupt() override;

    void executePlaybackStarted();

    void executePlaybackUnderbuffer(bool beforeStart);

    void executePlaybackBufferrefill();

    void executePlaybackError();

    void executePlaybackStopped();

    void executePlaybackPaused();

    void executePlaybackResumed();

    void executePlaybackFinished();

    void executePlaybackNearlyFinished();

    static void *decodeFunc(void *arg);

    static void *playFunc(void *arg);

    static void ffmpegLogOutput(void *ptr, int level, const char *fmt, va_list vl);

    static int interruptCallback(void *ctx);

private:
    ThreadPoolExecutor *m_executor;
    AVFormatContext *m_formatCtx;
    AVCodecContext *m_codecCtx;
    AVCodecParameters *m_codecParam;
    uint8_t *m_pcmBuffer;
    uint8_t *m_frameBuffer;
    AlsaController *m_alsaController;
    AudioPlayerListener *m_listener;
    AudioPlayerStatus m_status;
    std::string m_url;
    pthread_t m_decodeThread;
    pthread_t m_playThread;
    unsigned int m_progressMs;
    unsigned int m_durationMs;
    unsigned int m_bytesPersecond;
    unsigned int m_reportIntervalMs;
    unsigned int m_startOffsetMs;
    unsigned int m_frameTimestamp;
    unsigned int m_outChannel;
    unsigned int m_progressStartMs;
    int m_sampleRate;
    int m_bytesPersample;
    std::atomic<bool> m_seekable;
    std::atomic<bool> m_pauseFlag;
    std::atomic<bool> m_stopFlag;
    std::atomic<bool> m_finishFlag;
    std::atomic<bool> m_firstPackFlag;
    std::atomic<bool> m_nearlyFinishFlag;
    std::atomic<bool> m_shouldBreakFlag;
    pthread_mutex_t m_alsaMutex;
    pthread_mutex_t m_decodeMutex;
    pthread_mutex_t m_playMutex;
    pthread_cond_t m_decodeCond;
    pthread_cond_t m_playCond;
    std::atomic<bool> m_threadAlive;
};

}  // mediaPlayer
}  // duerOSDcsApp

#endif  // DUEROS_DCS_APP_MEDIAPLAYER_AUDIOPLAYERIMPL_H_