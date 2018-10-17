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

#include "DCSApp/SoundController.h"
#include "DCSApp/ActivityMonitorSingleton.h"

namespace duerOSDcsApp {
namespace application {

using namespace duerOSDcsSDK::sdkInterfaces;

SoundController *SoundController::mSoundController = NULL;
pthread_once_t SoundController::m_initOnce = PTHREAD_ONCE_INIT;
pthread_once_t SoundController::m_destroyOnce = PTHREAD_ONCE_INIT;

SoundController::SoundController() {
    pthread_mutex_init(&m_mutex, NULL);
    m_destroyOnce = PTHREAD_ONCE_INIT;
    m_pcmPlayer = new mediaPlayer::RapidPcmPlayer();
}

SoundController::~SoundController() {
    pthread_mutex_destroy(&m_mutex);
    m_initOnce = PTHREAD_ONCE_INIT;
}

SoundController *SoundController::getInstance() {
    pthread_once(&m_initOnce, &SoundController::init);
    return mSoundController;
}

void SoundController::addObserver(std::shared_ptr<LocalSourcePlayerInterface> mediaInterface) {
    if (mediaInterface) {
        m_observers.insert(mediaInterface);
    }
}

void SoundController::init() {
    if (mSoundController == NULL) {
        mSoundController = new SoundController();
    }
}

void SoundController::destory() {
    if (mSoundController) {
        delete mSoundController;
        mSoundController = NULL;
    }
}

void SoundController::release() {
    pthread_once(&m_destroyOnce, SoundController::destory);
}

void SoundController::audioPlay(const std::string &source,
                                bool needFocus,
                                void (*start_callback)(void *arg),
                                void *start_cb_arg,
                                void (*finish_callback)()) {
    for (auto observer : m_observers) {
        if (observer) {
            observer->playLocalSource(source, needFocus, start_callback, start_cb_arg, finish_callback);
        }
    }
}

void SoundController::wakeUp() {
    audioPlay("/data/misc/dueros/appresources/du.mp3", false, NULL, NULL, NULL);
}

void SoundController::linkStartFirst() {
    audioPlay("/data/misc/dueros/appresources/link_start_first.mp3", true, NULL, NULL, NULL);
}

void SoundController::linkStart() {
    audioPlay("/data/misc/dueros/appresources/link_start.mp3", true, NULL, NULL, NULL);
}

void SoundController::linkConnecting() {
    audioPlay("/data/misc/dueros/appresources/link_connecting.mp3", true, NULL, NULL, NULL);
}

void SoundController::linkSuccess(void(*callback)()) {
    audioPlay("/data/misc/dueros/appresources/link_success.mp3", true, NULL, NULL, callback);
}

void SoundController::linkFailedPing(void(*callback)()) {
    audioPlay("/data/misc/dueros/appresources/link_failed_ping.mp3", true, NULL, NULL, NULL);
}

void SoundController::linkFailedIp(void(*callback)()) {
    audioPlay("/data/misc/dueros/appresources/link_failed_ip.mp3", true, NULL, NULL, callback);
}

void SoundController::linkExit(void(*callback)()) {
    audioPlay("/data/misc/dueros/appresources/link_exit.mp3", true, NULL, NULL, callback);
}

void SoundController::reLink() {
    audioPlay("/data/misc/dueros/appresources/re_link.mp3", true, NULL, NULL, NULL);
}

void SoundController::reLinkSuccess(void(*callback)()) {
    audioPlay("/data/misc/dueros/appresources/re_link_success.mp3", true, NULL, NULL, callback);
}

void SoundController::reLinkFailed() {
    audioPlay("/data/misc/dueros/appresources/re_link_failed.mp3", true, NULL, NULL, NULL);
}

void SoundController::btUnpaired() {
    audioPlay("/data/misc/dueros/appresources/bt_unpaired.mp3", true, NULL, NULL, NULL);
}

void SoundController::btPairSuccess(void(*callback)()) {
    audioPlay("/data/misc/dueros/appresources/bt_pair_success.mp3", true, NULL, NULL, callback);
}

void SoundController::btPairFailedPaired() {
    audioPlay("/data/misc/dueros/appresources/bt_pair_failed_paired.mp3", true, NULL, NULL, NULL);
}

void SoundController::btPairFailedOther() {
    audioPlay("/data/misc/dueros/appresources/bt_pair_failed_other.mp3", true, NULL, NULL, NULL);
}

void SoundController::btDisconnect(void(*callback)()) {
    audioPlay("/data/misc/dueros/appresources/bt_disconnect.mp3", true, NULL, NULL, callback);
}

void SoundController::networkConnectFailed() {
    audioPlay("/data/misc/dueros/appresources/network_connect_failed.mp3", true, NULL, NULL, NULL);
}

void SoundController::networkSlow() {
    audioPlay("/data/misc/dueros/appresources/network_slow.mp3", false, NULL, NULL, NULL);
}

void SoundController::openBluetooth(void(*callback)(void *arg), void *arg) {
    audioPlay("/data/misc/dueros/appresources/open_bluetooth.mp3", true, callback, arg, NULL);
}

void SoundController::closeBluetooth(void(*callback)(void *arg), void *arg) {
    audioPlay("/data/misc/dueros/appresources/close_bluetooth.mp3", true, callback, arg, NULL);
}

void SoundController::volume() {
    m_pcmPlayer->stop();
    m_pcmPlayer->play();
    ActivityMonitorSingleton::getInstance()->updateActiveTimestamp();
}

void SoundController::playTts(const std::string& content, bool needFocus, void (*callback)()) {
    for (auto observer : m_observers) {
        if (observer) {
            observer->playTts(content, needFocus, callback);
        }
    }
}

void SoundController::serverConnecting() {
    audioPlay("/data/misc/dueros/appresources/server_connecting.mp3", false, NULL, NULL, NULL);
}

void SoundController::serverConnectFailed() {
    audioPlay("/data/misc/dueros/appresources/server_connect_failed.mp3", false, NULL, NULL, NULL);
}

void SoundController::bleNetworkConfig() {
    audioPlay("/data/misc/dueros/appresources/ble_network_config.mp3", false, NULL, NULL, NULL);
}

void SoundController::accountUnbound(void(*callback)()) {
    audioPlay("/data/misc/dueros/appresources/unbound.mp3", true, NULL, NULL, callback);
}

void SoundController::hotConnected() {
    audioPlay("/data/misc/dueros/appresources/hot_connected.mp3", false, NULL, NULL, NULL);
}

void SoundController::waitLogin() {
    audioPlay("/data/misc/dueros/appresources/wait_login.mp3", false, NULL, NULL, NULL);
}

}  // namespace application
}  // namespace duerOSDcsApp
