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

#include "DCSApp/PortAudioMicrophoneWrapper.h"
#include "DCSApp/DeviceIoWrapper.h"
#include "LoggerUtils/DcsSdkLogger.h"

namespace duerOSDcsApp {
namespace application {
    
static const int NUM_INPUT_CHANNELS = 1;
static const int NUM_OUTPUT_CHANNELS = 0;
static const double SAMPLE_RATE = 16000;
#ifndef USE_ALSA_RECORDER
static const unsigned long PREFERRED_SAMPLES_PER_CALLBACK = paFramesPerBufferUnspecified;
#endif
std::unique_ptr<PortAudioMicrophoneWrapper> PortAudioMicrophoneWrapper::create(
    std::shared_ptr<duerOSDcsSDK::sdkInterfaces::DcsSdk> dcsSdk) {
    if (!dcsSdk) {
        APP_ERROR("Invalid dcsSdk passed to PortAudioMicrophoneWrapper");
        return nullptr;
    }
    std::unique_ptr<PortAudioMicrophoneWrapper> portAudioMicrophoneWrapper(new PortAudioMicrophoneWrapper(dcsSdk));
    if (!portAudioMicrophoneWrapper->initialize()) {
        APP_ERROR("Failed to initialize PortAudioMicrophoneWrapper");
        return nullptr;
    }
    return portAudioMicrophoneWrapper;
}

PortAudioMicrophoneWrapper::PortAudioMicrophoneWrapper(std::shared_ptr<duerOSDcsSDK::sdkInterfaces::DcsSdk> dcsSdk) :
        m_dcsSdk{dcsSdk},
#ifdef USE_ALSA_RECORDER
        _m_rec{nullptr},
#else
        m_paStream{nullptr},
#endif
        m_callBack{nullptr} {
}

PortAudioMicrophoneWrapper::~PortAudioMicrophoneWrapper() {
#ifdef USE_ALSA_RECORDER
    _m_rec->recorderClose();
#else
    Pa_StopStream(m_paStream);
    Pa_CloseStream(m_paStream);
    Pa_Terminate();
#endif
}
#ifdef USE_ALSA_RECORDER
void PortAudioMicrophoneWrapper::recordDataInputCallback(char* buffer, long unsigned int size, void *userdata) {
    PortAudioMicrophoneWrapper* wrapper = static_cast<PortAudioMicrophoneWrapper*>(userdata);
    if (wrapper->running == false) {
        APP_ERROR("Record not started");
        return;
    }
    ssize_t returnCode = 1;
    if (!DeviceIoWrapper::getInstance()->isPhoneConnected()) {
        returnCode = wrapper->m_dcsSdk->writeAudioData(buffer, size/2);
    }

    if (wrapper->m_callBack && buffer && size != 0) {
        wrapper->m_callBack((const char *)buffer, size);
    }
    if (returnCode <= 0) {
        APP_ERROR("Failed to write to stream.");
        return;
    }
    return;
}
#endif
bool PortAudioMicrophoneWrapper::initialize() {
#ifdef USE_ALSA_RECORDER
    int ret  = 0;
#ifndef USE_MPC_RECORDER

    unsigned int i = 0;
    char *cmd[] = {"amixer cset name='Left PGA Mux' 2",
                   "amixer cset name='Right PGA Mux' 2",
                   "amixer cset name='O09 I03 Switch' on",
                   "amixer cset name='O10 I04 Switch' on",
                   "amixer cset name='AIN Mux' 0",
                   "amixer cset name='AIF TX Mux' 1"};

//set mixer
    for (i = 0; i < sizeof(cmd) / sizeof(cmd[0]); i++)
    {
        ret = system(cmd[i]);
        if (0 != ret)
        {
            APP_ERROR("system [%s] failed!\n", cmd[i]);
        }
    }
#endif
    APP_INFO("use alsa recorder");
#ifdef USE_MPC_RECORDER
    _m_rec = MpcRecorder::create();
#else
    _m_rec = new AudioRecorder();
#endif
    if (_m_rec != nullptr) {
        ret = _m_rec->recorderOpen("default", recordDataInputCallback, this);
        if (0 != ret) {
            APP_ERROR("Failed to open recorder");
            return false;
        }
    } else {
            APP_ERROR("Failed to new AudioRecorder");
            return false;
    }
    return true;
#else
    PaError err;

    unsigned int i = 0;
    int i4_ret = 0;
    char *cmd[] = {"amixer cset name='Left PGA Mux' 2",
                   "amixer cset name='Right PGA Mux' 2",
                   "amixer cset name='O09 I03 Switch' on",
                   "amixer cset name='O10 I04 Switch' on",
                   "amixer cset name='AIN Mux' 0",
                   "amixer cset name='AIF TX Mux' 1"};

    
//set mixer
    for (i = 0; i < sizeof(cmd) / sizeof(cmd[0]); i++)
    {
        i4_ret = system(cmd[i]);
        if (0 != i4_ret)
        {
            APP_ERROR("system [%s] failed!\n", cmd[i]);
        }
    }
	
    err = Pa_Initialize();
    if (err != paNoError) {
        APP_ERROR("Failed to initialize PortAudio");
        return false;
    }
    err = Pa_OpenDefaultStream(
        &m_paStream,
        NUM_INPUT_CHANNELS,
        NUM_OUTPUT_CHANNELS,
        paInt16,
        SAMPLE_RATE,
        PREFERRED_SAMPLES_PER_CALLBACK,
        PortAudioCallback,
        this);
    if (err != paNoError) {
        APP_ERROR("Failed to open PortAudio default stream");
        return false;
    }
    return true;
#endif
}

bool PortAudioMicrophoneWrapper::startStreamingMicrophoneData() {
    std::lock_guard<std::mutex> lock{m_mutex};
#ifdef USE_ALSA_RECORDER
#ifdef USE_MPC_RECORDER
    _m_rec->startRecording();
#else
    running = true;
#endif
    return true;
#else
    PaError err = Pa_StartStream(m_paStream);
    if (err != paNoError) {
        APP_ERROR("Failed to start PortAudio stream");
        return false;
    }
    return true;
#endif
}

void PortAudioMicrophoneWrapper::setRecordDataInputCallback(void (*callBack)(const char *buffer, unsigned int size)) {
    m_callBack = callBack;
}

bool PortAudioMicrophoneWrapper::stopStreamingMicrophoneData() {
    std::lock_guard<std::mutex> lock{m_mutex};
#ifdef USE_ALSA_RECORDER
#ifdef USE_MPC_RECORDER
    _m_rec->stopRecording();
#else
    running = false;
#endif
    return true;
#else
    PaError err = Pa_StopStream(m_paStream);
    if (err != paNoError) {
        APP_ERROR("Failed to stop PortAudio stream");
        return false;
    }
    return true;
#endif
}

#ifndef USE_ALSA_RECORDER
int PortAudioMicrophoneWrapper::PortAudioCallback(
    const void* inputBuffer,
    void* outputBuffer,
    unsigned long numSamples,
    const PaStreamCallbackTimeInfo* timeInfo,
    PaStreamCallbackFlags statusFlags,
    void* userData) {
    PortAudioMicrophoneWrapper* wrapper = static_cast<PortAudioMicrophoneWrapper*>(userData);

    ssize_t returnCode = 1;
    if (!DeviceIoWrapper::getInstance()->isPhoneConnected()) {
        returnCode = wrapper->m_dcsSdk->writeAudioData(inputBuffer, numSamples);
    }

    if (wrapper->m_callBack && inputBuffer && numSamples != 0) {
        wrapper->m_callBack((const char *)inputBuffer, 2 * numSamples);
    }
    if (returnCode <= 0) {
        APP_ERROR("Failed to write to stream.");
        return paAbort;
    }
    return paContinue;
}
#endif

}  // namespace application
}  // namespace duerOSDcsApp
