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

#ifndef DUEROS_DCS_APP_MEDIAPLAYER_LOCALSYNTHESIZERPLAYER_H_
#define DUEROS_DCS_APP_MEDIAPLAYER_LOCALSYNTHESIZERPLAYER_H_

#include <string>
#include <unistd.h>
#include "TTSManager/TTSManager.h"
#include "MediaPlayer/StreamPool.h"
#include "MediaPlayer/AlsaController.h"
#include "MediaPlayer/PcmResampler.h"
#include "DCSApp/ThreadPoolExecutor.h"

namespace duerOSDcsApp {
namespace mediaPlayer {

using duerOSDcsSDK::ttsManager::TTSManager;
using duerOSDcsSDK::ttsManager::TTSSynthesizeResultObserver;
using application::ThreadPoolExecutor;

enum TtsPlayerStatus {
    TtsPlayerStatusIdle,
    TtsPlayerStatusPlay,
    TtsPlayerStatusStop,
};

class LocalTtsPlayerListener {
public:
    virtual ~LocalTtsPlayerListener(){}

    virtual void speechStarted() = 0;

    virtual void speechFinished() = 0;
};

class LocalSynthesizerPlayer : public TTSSynthesizeResultObserver {
public:
    static std::shared_ptr<LocalSynthesizerPlayer> create(const std::string& audio_dev,
                                                          const std::string& res_path);

    virtual ~LocalSynthesizerPlayer();

    void ttsPlay(const std::string& content);

    void ttsStop();

    void registerListener(LocalTtsPlayerListener *notify);

private:
    LocalSynthesizerPlayer(const std::string& audio_dev,
                           const std::string& synthesize_res_path);

    LocalSynthesizerPlayer(const LocalSynthesizerPlayer&);

    LocalSynthesizerPlayer& operator=(const LocalSynthesizerPlayer&);

    TtsPlayerStatus getStatus();

    void setStatus(TtsPlayerStatus status);

    void executeSpeechStarted();

    void executeSpeechFinished();

    void ttsSynthesizeBegin(const std::string& strIndex);

    void ttsSynthesizeAudioData(const char *stream,
                                unsigned int length,
                                unsigned int charIndex,
                                const std::string& strIndex = "");

    void ttsSynthesizeFinish(const std::string& strIndex = "");

    void ttsSynthesizeFailed(int errCode,
                             const std::string &errDesc,
                             const std::string& strIndex = "");

    static void* playFunc(void *arg);

private:
    ThreadPoolExecutor *m_executor;
    TTSManager* m_ttsMgr;
    std::vector<LocalTtsPlayerListener*> m_listeners;
    AlsaController *m_alsaCtl;
    StreamPool m_streamPool;
    pthread_t m_playThread;
    pthread_mutex_t m_playMutex;
    pthread_cond_t m_playCond;
    TtsPlayerStatus m_status;
    pthread_mutex_t m_statusLock;
    std::string m_ttsContent;
    bool m_firstPackFlag;
    bool m_threadAlive;
    bool m_pushStreamFlag;
    unsigned int m_maxValidLen;
};

}  // mediaPlayer
}  // duerOSDcsApp

#endif  // DUEROS_DCS_APP_MEDIAPLAYER_LOCALSYNTHESIZERPLAYER_H_
