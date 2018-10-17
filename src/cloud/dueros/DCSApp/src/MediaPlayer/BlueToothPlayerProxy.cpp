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

#include "MediaPlayer/BlueToothPlayerProxy.h"
#include "LoggerUtils/DcsSdkLogger.h"

namespace duerOSDcsApp {
namespace mediaPlayer {

std::shared_ptr<BlueToothPlayerProxy> BlueToothPlayerProxy::create() {
    APP_INFO("BlueToothPlayerProxy createCalled");
    std::shared_ptr<BlueToothPlayerProxy> bluetoothPlayer(new BlueToothPlayerProxy());
    if (!bluetoothPlayer) {
        APP_ERROR("BlueToothPlayerProxy Create Failed");
        return nullptr;
    } else {
        return bluetoothPlayer;
    }
}

BlueToothPlayerProxy::BlueToothPlayerProxy() : m_localMediaPlayerPlayActivity{STOPPED} {
    APP_ERROR("BlueToothPlayerProxy Constructor");
}

BlueToothPlayerProxy::~BlueToothPlayerProxy() {
    APP_ERROR("BlueToothPlayerProxy Destructor");
}

void BlueToothPlayerProxy::setObserver(
        std::shared_ptr<LocalMediaPlayerObserverInterface> playerObserver) {
    if (playerObserver) {
        m_playerObserver = playerObserver;
    }
}

LocalMediaPlayerStatus BlueToothPlayerProxy::play() {
    APP_INFO("BlueToothPlayerProxy play");
    m_localMediaPlayerPlayActivity = PLAYING;
    application::DeviceIoWrapper::getInstance()->btResumePlay();
    return LocalMediaPlayerStatus::LOCAL_MEDIA_PLAYER_SUCCESS;
}

LocalMediaPlayerStatus BlueToothPlayerProxy::stop() {
    APP_INFO("BlueToothPlayerProxy stop");
    m_localMediaPlayerPlayActivity = STOPPED;
    application::DeviceIoWrapper::getInstance()->btPausePlay();
    if (m_playerObserver) {
        LocalMediaPlayerPlayInfo playInfo;
        playInfo.m_playerActivity = m_localMediaPlayerPlayActivity;
        playInfo.m_status = BACKGROUND;
        playInfo.m_playerName = BLUETOOTH;
        playInfo.m_audioId = "";
        playInfo.m_title = "";
        playInfo.m_artist = "";
        playInfo.m_album = "";
        playInfo.m_year = "";
        playInfo.m_genre = "";
        m_playerObserver->setLocalMediaPlayerPlayInfo(playInfo);
    }
    return LocalMediaPlayerStatus::LOCAL_MEDIA_PLAYER_SUCCESS;
}

LocalMediaPlayerStatus BlueToothPlayerProxy::pause() {
    APP_INFO("BlueToothPlayerProxy pause");
    m_localMediaPlayerPlayActivity = PAUSED;
    application::DeviceIoWrapper::getInstance()->btPausePlay();
    return LocalMediaPlayerStatus::LOCAL_MEDIA_PLAYER_SUCCESS;
}

LocalMediaPlayerStatus BlueToothPlayerProxy::resume() {
    APP_INFO("BlueToothPlayerProxy resume");
    m_localMediaPlayerPlayActivity = PLAYING;
    application::DeviceIoWrapper::getInstance()->btResumePlay();
    return LocalMediaPlayerStatus::LOCAL_MEDIA_PLAYER_SUCCESS;
}

LocalMediaPlayerStatus BlueToothPlayerProxy::playNext() {
    APP_INFO("BlueToothPlayerProxy playNext");
    m_localMediaPlayerPlayActivity = PLAYING;
    application::DeviceIoWrapper::getInstance()->btPlayNext();
    return LocalMediaPlayerStatus::LOCAL_MEDIA_PLAYER_SUCCESS;
}

LocalMediaPlayerStatus BlueToothPlayerProxy::playPrevious() {
    APP_INFO("BlueToothPlayerProxy playPrevious");
    m_localMediaPlayerPlayActivity = PLAYING;
    application::DeviceIoWrapper::getInstance()->btPlayPrevious();
    return LocalMediaPlayerStatus::LOCAL_MEDIA_PLAYER_SUCCESS;
}

void BlueToothPlayerProxy::btStartPlay() {
    APP_INFO("BlueToothPlayerProxy btStartPlay");
    application::DeviceIoWrapper::deviceioWrapperBtSoundPlayFinished();
    if (m_playerObserver) {
        LocalMediaPlayerPlayInfo playInfo;
        playInfo.m_playerActivity = PLAYING;
        playInfo.m_status = FOREGROUND;
        playInfo.m_playerName = BLUETOOTH;
        playInfo.m_audioId = "";
        playInfo.m_title = "";
        playInfo.m_artist = "";
        playInfo.m_album = "";
        playInfo.m_year = "";
        playInfo.m_genre = "";
        m_playerObserver->setLocalMediaPlayerPlayInfo(playInfo);
    }
    if (m_dcsSdk) {
        m_dcsSdk->enterPlayMusicScene();
    }
}

void BlueToothPlayerProxy::btStopPlay() {
    APP_INFO("BlueToothPlayerProxy btStopPlay");
    if (m_playerObserver) {
        LocalMediaPlayerPlayInfo playInfo;
        if (PAUSED != m_localMediaPlayerPlayActivity) {
            m_localMediaPlayerPlayActivity = STOPPED;
        }
        playInfo.m_playerActivity = m_localMediaPlayerPlayActivity;
        playInfo.m_status = BACKGROUND;
        playInfo.m_playerName = BLUETOOTH;
        playInfo.m_audioId = "";
        playInfo.m_title = "";
        playInfo.m_artist = "";
        playInfo.m_album = "";
        playInfo.m_year = "";
        playInfo.m_genre = "";
        m_playerObserver->setLocalMediaPlayerPlayInfo(playInfo);
    }
    if (m_dcsSdk) {
        m_dcsSdk->exitPlayMusicScene();
    }
}

void BlueToothPlayerProxy::btDisconnect() {
    APP_INFO("BlueToothPlayerProxy btDisconnect");
    if (m_playerObserver) {
        LocalMediaPlayerPlayInfo playInfo;
        m_localMediaPlayerPlayActivity = STOPPED;
        playInfo.m_playerActivity = m_localMediaPlayerPlayActivity;
        playInfo.m_status = CLOSED;
        playInfo.m_playerName = BLUETOOTH;
        playInfo.m_audioId = "";
        playInfo.m_title = "";
        playInfo.m_artist = "";
        playInfo.m_album = "";
        playInfo.m_year = "";
        playInfo.m_genre = "";
        m_playerObserver->setLocalMediaPlayerPlayInfo(playInfo);
    }
    if (m_dcsSdk) {
        m_dcsSdk->exitPlayMusicScene();
    }
}

void BlueToothPlayerProxy::btPowerOff() {
    APP_INFO("BlueToothPlayerProxy btPowerOff");
    if (m_playerObserver) {
        LocalMediaPlayerPlayInfo playInfo;
        m_localMediaPlayerPlayActivity = STOPPED;
        playInfo.m_playerActivity = m_localMediaPlayerPlayActivity;
        playInfo.m_status = CLOSED;
        playInfo.m_playerName = BLUETOOTH;
        playInfo.m_audioId = "";
        playInfo.m_title = "";
        playInfo.m_artist = "";
        playInfo.m_album = "";
        playInfo.m_year = "";
        playInfo.m_genre = "";
        m_playerObserver->setLocalMediaPlayerPlayInfo(playInfo);
    }
    if (m_dcsSdk) {
        m_dcsSdk->exitPlayMusicScene();
    }
}
void BlueToothPlayerProxy::setDcsSdk(
        std::shared_ptr<duerOSDcsSDK::sdkInterfaces::DcsSdk> dcsSdk) {
    m_dcsSdk = dcsSdk;
}

}  // namespace mediaPlayer
}  // namespace duerOSDcsApp
