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

#include "DCSApp/DCSApplication.h"
#include <algorithm>
#include <fstream>
#include "DCSApp/ApplicationManager.h"
#include "DCSApp/VoiceAndTouchWakeUpObserver.h"
#include "DCSApp/Configuration.h"
#include "MediaPlayer/TtsPlayerProxy.h"
#include "MediaPlayer/MediaPlayerProxy.h"
#include "MediaPlayer/AlertsPlayerProxy.h"
#include "MediaPlayer/LocalPlayerProxy.h"
#ifdef LOCALTTS
#include "MediaPlayer/LocalTtsProxy.h"
#endif
#include "MediaPlayer/BlueToothPlayerProxy.h"
#include "DCSApp/RecordAudioManager.h"
#include "LoggerUtils/DcsSdkLogger.h"
#include "DCSApp/ActivityMonitorSingleton.h"

namespace duerOSDcsApp {
namespace application {

/// The config path of DCSSDK
static const std::string PATH_TO_CONFIG = "/data/misc/dueros/resources/config.json";

#ifdef MTK8516
/// The runtime config path of DCSSDK
static const std::string PATH_TO_RUNTIME_CONFIG = "/data/duer/runtime.json";
#else
/// The runtime config path of DCSSDK
static const std::string PATH_TO_RUNTIME_CONFIG = "/data/misc/dueros/resources/runtime.json";
#endif

std::unique_ptr<DCSApplication> DCSApplication::create() {
    auto dcsApplication = std::unique_ptr<DCSApplication>(new DCSApplication);
    if (!dcsApplication->initialize()) {
        APP_ERROR("Failed to initialize SampleApplication");
        return nullptr;
    }
    return dcsApplication;
}

void DCSApplication::run() {
    while(true) {
        struct timeval wait = {0, 100 * 1000};
        ::select(0, NULL, NULL, NULL, &wait);
    }
}

bool DCSApplication::initialize() {

    if (!Configuration::getInstance()->readConfig()) {
        APP_ERROR("Failed to init Configuration!");
        return false;
    }

    DeviceIoWrapper::getInstance()->initCommonVolume(Configuration::getInstance()->getCommVol());
    DeviceIoWrapper::getInstance()->initAlertVolume(Configuration::getInstance()->getAlertsVol());
    DeviceIoWrapper::getInstance()->bspInitialization();

    /*
     * Creating the media players.
     */
    Configuration *configuration = Configuration::getInstance();
    auto speakMediaPlayer = mediaPlayer::TtsPlayerProxy::create(configuration->getTtsPlaybackDevice());
    if (!speakMediaPlayer) {
        APP_ERROR("Failed to create media player for speech!");
        return false;
    }

    auto audioMediaPlayer = mediaPlayer::MediaPlayerProxy::create(configuration->getMusicPlaybackDevice());
    if (!audioMediaPlayer) {
        APP_ERROR("Failed to create media player for content!");
        return false;
    }

    auto alertsMediaPlayer = mediaPlayer::AlertsPlayerProxy::create(configuration->getAlertPlaybackDevice());
    if (!alertsMediaPlayer) {
        APP_ERROR("Failed to create media player for alerts!");
        return false;
    }
    //alertsMediaPlayer->setFadeIn(10);

    auto localPromptPlayer = mediaPlayer::LocalPlayerProxy::create(configuration->getInfoPlaybackDevice());
    if (!localPromptPlayer) {
        APP_ERROR("Failed to create media player for local!");
        return false;
    }
#ifdef LOCALTTS
    auto localTtsPlayer = mediaPlayer::LocalTtsProxy::create(configuration->getNattsPlaybackDevice());
    if (!localTtsPlayer) {
        APP_ERROR("Failed to create media player for local tts!");
        return false;
    }
#endif

    auto blueToothPlayer = mediaPlayer::BlueToothPlayerProxy::create();
    if (!blueToothPlayer) {
        APP_ERROR("Failed to create blueToothPlayer!");
        return false;
    }

    auto applicationManager = std::make_shared<ApplicationManager>();

    duerOSDcsSDK::sdkInterfaces::DcsSdkParameters parameters;
    parameters.setPathToConfig(PATH_TO_CONFIG);
    parameters.setPathToRuntimeConfig(PATH_TO_RUNTIME_CONFIG);
    parameters.setDeviceId(DeviceIoWrapper::getInstance()->getDeviceId());
    parameters.setSpeakMediaPlayer(speakMediaPlayer);
    parameters.setAudioMediaPlayer(audioMediaPlayer);
    parameters.setAlertsMediaPlayer(alertsMediaPlayer);
    parameters.setLocalPromptPlayer(localPromptPlayer);
    parameters.setDialogStateObservers({applicationManager});
    parameters.setConnectionObservers(applicationManager);
    parameters.setApplicationImplementation(applicationManager);
    parameters.setLocalMediaPlayer(blueToothPlayer);
    parameters.setEnableSdkWakeup(true);

    // This observer is notified any time a keyword is detected and notifies the DCSSDK to start recognizing.
    auto voiceAndTouchWakeUpObserver = std::make_shared<VoiceAndTouchWakeUpObserver>();
    parameters.setKeyWordObserverInterface(voiceAndTouchWakeUpObserver);

#ifdef LOCALTTS
    parameters.setLocalTtsMediaPlayer(localTtsPlayer);
#endif
    m_dcsSdk = duerOSDcsSDK::sdkInterfaces::DcsSdk::create(parameters);

    if (!m_dcsSdk) {
        APP_ERROR("Failed to create default SDK handler!");
        return false;
    }

    std::shared_ptr<PortAudioMicrophoneWrapper> micWrapper = PortAudioMicrophoneWrapper::create(m_dcsSdk);
    if (!micWrapper) {
        APP_ERROR("Failed to create PortAudioMicrophoneWrapper!");
        return false;
    }
    micWrapper->setRecordDataInputCallback(recordDataInputCallback);

    applicationManager->setMicrophoneWrapper(micWrapper);
    applicationManager->setDcsSdk(m_dcsSdk);
    blueToothPlayer->setDcsSdk(m_dcsSdk);

    voiceAndTouchWakeUpObserver->setDcsSdk(m_dcsSdk);

#ifdef LOCALTTS
    SoundController::getInstance()->addObserver(localTtsPlayer);
#endif
    SoundController::getInstance()->addObserver(localPromptPlayer);
    DeviceIoWrapper::getInstance()->addObserver(voiceAndTouchWakeUpObserver);
    DeviceIoWrapper::getInstance()->addObserver(blueToothPlayer);
    DeviceIoWrapper::getInstance()->setApplicationManager(applicationManager);
    DeviceIoWrapper::getInstance()->volumeChanged();

    DuerLinkWrapper::getInstance()->setCallback(this);

    if (framework::DeviceIo::getInstance()->inOtaMode()) {
        DeviceIoWrapper::getInstance()->setMute(true);
    }

	DuerLinkWrapper::getInstance()->initDuerLink();

    DuerLinkWrapper::getInstance()->startNetworkRecovery();

    if (!DuerLinkWrapper::getInstance()->waitFor(DuerLinkNetworkStatus::DUERLINK_NETWORK_SUCCEEDED)) {
        APP_WARN("Failed to connect to network in 600 seconds!");
        //return false;
    }

    applicationManager->microphoneOn();

#ifdef MTK8516
    DuerLinkWrapper::getInstance()->initDuerLink();

    DuerLinkWrapper::getInstance()->startNetworkRecovery();

    std::chrono::seconds secondsToWait{1};
    if (!m_detectNTPTimer.start(
            secondsToWait, deviceCommonLib::deviceTools::Timer::PeriodType::ABSOLUTE,
            deviceCommonLib::deviceTools::Timer::FOREVER,
            std::bind(&DCSApplication::detectNTPReady, this))) {
        APP_ERROR("Failed to start m_checkNtpTimer");
    }
#else
    //m_dcsSdk->notifySystemTimeReady();
    //m_dcsSdk->notifyNetworkReady(true, "testWifi");
#endif

    DuerLinkWrapper::getInstance()->startDiscoverAndBound(m_dcsSdk->getClientId());

    m_systemUpdateRevWrapper = SystemUpdateRevWrapper::create();

    return true;
}

void DCSApplication::detectNTPReady() {
#if MTK8516
    if (access("/tmp/ntp_successful", F_OK) == 0) {
        m_dcsSdk->notifySystemTimeReady();
        m_detectNTPTimer.stop();
        ActivityMonitorSingleton::getInstance()->updateActiveTimestamp();
    }
#else
	if (m_dcsSdk) {
		APP_INFO("DCSApplication::detectNTPReady!");
		m_dcsSdk->notifySystemTimeReady();
	}
#endif
}

void DCSApplication::networkReady() {
    if (m_dcsSdk) {
		APP_INFO("DCSApplication::networkReady!");
        m_dcsSdk->notifyNetworkReady(DuerLinkWrapper::getInstance()->isFromConfigNetwork(), DeviceIoWrapper::getInstance()->getWifiBssid());
    }
}

void DCSApplication::duerlinkNotifyReceivedData(const std::string& jsonPackageData, int sessionId) {
    if (!DeviceIoWrapper::getInstance()->isSleepMode()) {
        APP_INFO("DCSApplication duerlink_notify_received_data: [%s], sessionId: [%d]",
                 jsonPackageData.c_str(), sessionId);
        if (m_dcsSdk) {
            m_dcsSdk->consumeMessage(jsonPackageData);
        }
    }
}

void DCSApplication::duerlinkNetworkOnlineStatus(bool status) {
    if (m_dcsSdk) {
        m_dcsSdk->informOnlineStatus(status);
    }
}

void DCSApplication::recordDataInputCallback(const char* buffer, unsigned int size) {
    DeviceIoWrapper::getInstance()->transmitAudioToBluetooth(buffer, size);
    /// save record data to file.
    RecordAudioManager::getInstance()->transmitAudioRecordData(buffer, size);
}

}  // namespace application
}  // namespace duerOSDcsApp
