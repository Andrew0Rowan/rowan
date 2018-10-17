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

#include <cstring>
#include "MediaPlayer/TtsPlayerProxy.h"
#include "DCSApp/ActivityMonitorSingleton.h"
#include "LoggerUtils/DcsSdkLogger.h"

namespace duerOSDcsApp {
namespace mediaPlayer {

using namespace duerOSDcsApp::application;
using namespace duerOSDcsSDK::sdkInterfaces;
using application::ThreadPoolExecutor;

#define READ_TIMEOUT_MS 3000
#define RETRY_READ_TIMEOUT_MS 800
#define ATTACHMENT_READ_CHUNK_SIZE 512

std::shared_ptr<TtsPlayerProxy> TtsPlayerProxy::create(const std::string& audio_device) {
    APP_INFO("createCalled");
    std::shared_ptr<TtsPlayerProxy> mediaPlayer(new TtsPlayerProxy(audio_device));
    if (mediaPlayer->init()) {
        return mediaPlayer;
    } else {
        return nullptr;
    }
};

TtsPlayerProxy::TtsPlayerProxy(const std::string& audio_device) :
        m_playerObserver{nullptr},
        m_tts_player{nullptr} {
    m_executor = ThreadPoolExecutor::getInstance();
    m_tts_player = new TtsPlayer(audio_device);
    m_tts_player->registerListener(this);
}

bool TtsPlayerProxy::init() {
    return true;
}

TtsPlayerProxy::~TtsPlayerProxy() {
    APP_INFO("~TtsPlayerProxy");
    stop();
    if (m_tts_player) {
        delete m_tts_player;
        m_tts_player = nullptr;
    }
}

MediaPlayerStatus TtsPlayerProxy::setSource(
        std::shared_ptr<AttachmentReader> reader) {
    APP_INFO("setSourceCalled");
    handleSetAttachmentReaderSource(std::move(reader));
    return MediaPlayerStatus::SUCCESS;
}

MediaPlayerStatus TtsPlayerProxy::setSource(const std::string& audio_file_path, bool repeat) {
    APP_INFO("setSourceCalled");
    return MediaPlayerStatus::FAILURE;
}

MediaPlayerStatus TtsPlayerProxy::setSource(const std::string& url) {
    APP_INFO("setSourceForUrlCalled");
    return MediaPlayerStatus::FAILURE;
}

MediaPlayerStatus TtsPlayerProxy::play() {
    APP_INFO("playCalled");
    m_executor->submit([this] () { executePlay(); });
    return MediaPlayerStatus::SUCCESS;
}

void TtsPlayerProxy::executePlay() {
    APP_INFO("executePlayCalled");
    m_tts_player->ttsPlay();
    char buffer[ATTACHMENT_READ_CHUNK_SIZE];
    APP_INFO("Going to read data from AttachmentReader.");
    auto status = AttachmentReader::ReadStatus::OK;
    bool first_packet_flag = false;
    bool trigger_retry = false;
    std::chrono::milliseconds timeout_ms{READ_TIMEOUT_MS};
    std::chrono::milliseconds retry_timeout_ms{RETRY_READ_TIMEOUT_MS};
    while (true) {
        size_t size = 0;
        if (trigger_retry) {
            size = m_attachment_reader->read(buffer,
                                             ATTACHMENT_READ_CHUNK_SIZE,
                                             &status,
                                             retry_timeout_ms);
        } else {
            size = m_attachment_reader->read(buffer,
                                             ATTACHMENT_READ_CHUNK_SIZE,
                                             &status,
                                             timeout_ms);
        }
        

        //report to sdk when first tts packet received.
        if (!first_packet_flag) {
            first_packet_flag = true;
            executeRecvFirstpacket();
        }

        if (status == AttachmentReader::ReadStatus::OK ||
            status == AttachmentReader::ReadStatus::OK_WOULDBLOCK) {
            m_tts_player->pushData(buffer, size);
            trigger_retry = false;
        } else if (status == AttachmentReader::ReadStatus::OK_TIMEDOUT) {
            if (trigger_retry) {
                break;
            }
            trigger_retry = true;
            APP_INFO("############AttachmentReader Read Timeout");
        } else if (status == AttachmentReader::ReadStatus::CLOSED ||
                   status == AttachmentReader::ReadStatus::ERROR_OVERRUN ||
                   status == AttachmentReader::ReadStatus::ERROR_BYTES_LESS_THAN_WORD_SIZE ||
                   status == AttachmentReader::ReadStatus::ERROR_INTERNAL) {
            break;
        }
    }
    m_tts_player->ttsEnd();
    m_attachment_reader->close();
    APP_INFO("Finish reading data from AttachmentReader.");
}

MediaPlayerStatus TtsPlayerProxy::stop() {
    APP_INFO("stopCalled");
    if (m_tts_player) {
        m_tts_player->ttsStop();
    }
    return MediaPlayerStatus::SUCCESS;
}

MediaPlayerStatus TtsPlayerProxy::pause() {
    APP_INFO("pausedCalled");
    if (m_tts_player) {
        m_tts_player->ttsStop();
    }
    return MediaPlayerStatus::SUCCESS;
}

MediaPlayerStatus TtsPlayerProxy::resume() {
    APP_INFO("resumeCalled");
    return MediaPlayerStatus::SUCCESS;
}

std::chrono::milliseconds TtsPlayerProxy::getOffset() {
    APP_INFO("getOffsetInMillisecondsCalled");
    return std::chrono::milliseconds(0);
}

void TtsPlayerProxy::setObserver(std::shared_ptr<MediaPlayerObserverInterface> observer) {
    APP_INFO("setObserverCalled");
    if (observer) {
        m_playerObserver = observer;
    }
}

void TtsPlayerProxy::handleSetAttachmentReaderSource(
        std::shared_ptr<AttachmentReader> reader) {
    APP_INFO("handleSetSourceCalled");

    if (reader != nullptr) {
        m_attachment_reader = reader;
    } else {
        APP_ERROR("handleSetAttachmentReaderSourceFailed reader is null");
    }
}

void TtsPlayerProxy::executeRecvFirstpacket() {
    APP_INFO("callingexecuteRecvFirstpacket");
    if (m_playerObserver) {
        m_playerObserver->onRecvFirstpacket();
    }
}

void TtsPlayerProxy::playbackStarted() {
    APP_INFO("callingOnPlaybackStarted");
    if (m_playerObserver) {
        m_playerObserver->onPlaybackStarted();
    }
}

void TtsPlayerProxy::playbackStopped(StopStatus stopStatus) {
    APP_INFO("report PlaybackFinished");
    if (m_playerObserver) {
        m_playerObserver->onPlaybackFinished();
    }
    ActivityMonitorSingleton::getInstance()->updateActiveTimestamp();
}

}  // namespace mediaPlayer
}  // namespace duerOSDcsApp
