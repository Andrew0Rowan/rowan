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

#ifndef DUEROS_DCS_APP_MEDIAPLAYER_SIMPLEAUDIOPLAYERIMPL_H_
#define DUEROS_DCS_APP_MEDIAPLAYER_SIMPLEAUDIOPLAYERIMPL_H_

#include "DCSApp/ThreadPoolExecutor.h"
#include "AudioPlayerInterface.h"
#include "AlsaController.h"
#include "AudioDecoder.h"
#include <atomic>

namespace duerOSDcsApp {
namespace mediaPlayer {
using application::ThreadPoolExecutor;

enum PlayMode {
    PLAYMODE_NORMAL,
    PLAYMODE_LOOP
};

class SimpleAudioPlayerImpl : public IAudioPlayer {
public:
    SimpleAudioPlayerImpl(const std::string &audio_dev);

    ~SimpleAudioPlayerImpl();

    void registerListener(AudioPlayerListener *notify);

    /**
     * set play mode of the player.
     *
     * @param mode the mode : PLAYMODE_NORMAL PLAYMODE_LOOP
     * @param val loop times, useful when the mode is PLAYMODE_LOOP; loop forever when 0.
     * @return void
     */
    void setPlayMode(PlayMode mode, unsigned int val);

    void audioPlay(const std::string &url,
                    RES_FORMAT format,
                    unsigned int offset,
                    unsigned int report_interval);

    void audioPause();

    void audioResume();

    void audioStop();

    unsigned long getProgress();

    unsigned long getDuration();

    void setFadeIn(int timeSec);

    void executePlaybackStarted();

    void executePlaybackFinished();

private:
    SimpleAudioPlayerImpl(const SimpleAudioPlayerImpl &);

    SimpleAudioPlayerImpl &operator=(const SimpleAudioPlayerImpl &);

    static void *playFunc(void *arg);

    float getFadeAttenuate();

    void fadeinReset();

    void fadeinProcess(int  size);

private:
    ThreadPoolExecutor *m_executor;
    AlsaController *m_alsaController;
    AudioDecoder *m_audioDecoder;
    AudioPlayerListener *m_listener;
    uint8_t *m_pcmBuffer;
    uint8_t *m_frameBuffer;
    std::string m_resPath;
    int m_FadeinTimeMs;
    int m_FadeinTimeProgressMs;
    pthread_t m_threadId;
    pthread_mutex_t m_mutex;
    pthread_cond_t m_playCond;
    std::atomic<bool> m_stopFlag;
    std::atomic<bool> m_playReadyFlag;
    std::atomic<bool> m_exitFlag;
};

}  // namespace mediaPlayer
}  // namespace duerOSDcsApp

#endif  // DUEROS_DCS_APP_MEDIAPLAYER_SIMPLEAUDIOPLAYERIMPL_H_
