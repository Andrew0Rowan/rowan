/*
 * PortAudioMicrophoneWrapper.h
 *
 * Copyright (c) 2017 Amazon.com, Inc. or its affiliates. All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License").
 * You may not use this file except in compliance with the License.
 * A copy of the License is located at
 *
 *     http://aws.amazon.com/apache2.0/
 *
 * or in the "license" file accompanying this file. This file is distributed
 * on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either
 * express or implied. See the License for the specific language governing
 * permissions and limitations under the License.
 */

#ifndef DUEROS_DCS_APP_SAMPLEAPP_PORTAUDIOMICROPHONEWRAPPER_H_
#define DUEROS_DCS_APP_SAMPLEAPP_PORTAUDIOMICROPHONEWRAPPER_H_

#include <mutex>
#include <thread>
#include <DcsSdk/DcsSdk.h>
#ifdef USE_ALSA_RECORDER
#include "DCSApp/AlsaRecorder.h"
#else
#include <portaudio.h>
#endif

#ifdef USE_MPC_RECORDER
#include "DCSApp/MpcRecorder.h"
#endif

namespace duerOSDcsApp {
namespace application {

/// This acts as a wrapper around PortAudio, a cross-platform open-source audio I/O library.
class PortAudioMicrophoneWrapper {
public:
    /**
     * Creates a @c PortAudioMicrophoneWrapper.
     *
     * @param dcsSdk.
     * @return A unique_ptr to a @c PortAudioMicrophoneWrapper if creation was successful and @c nullptr otherwise.
     */
    static std::unique_ptr<PortAudioMicrophoneWrapper> create(std::shared_ptr<duerOSDcsSDK::sdkInterfaces::DcsSdk> dcsSdk);

    /**
     * Stops streaming from the microphone.
     *
     * @return Whether the stop was successful.
     */
    bool stopStreamingMicrophoneData();

    /**
     * Starts streaming from the microphone.
     *
     * @return Whether the start was successful.
     */
    bool startStreamingMicrophoneData();

    void setRecordDataInputCallback(void (*callBack)(const char *buffer, unsigned int size));

    /**
     * Destructor.
     */
    ~PortAudioMicrophoneWrapper();

private:
    /**
     * Constructor.
     *
     * @param dcsSdk.
     */
    PortAudioMicrophoneWrapper(std::shared_ptr<duerOSDcsSDK::sdkInterfaces::DcsSdk> dcsSdk);

    /**
     * The callback that PortAudio will issue when audio is avaiable to read.
     *
     * @param inputBuffer The temporary buffer that microphone audio data will be available in.
     * @param outputBuffer Not used here.
     * @param numSamples The number of samples available to consume.
     * @param timeInfo Time stamps indicated when the first sample in the buffer was captured. Not used here.
     * @param statusFlags Flags that tell us when underflow or overflow conditions occur. Not used here.
     * @param userData A user supplied pointer.
     * @return A PortAudio code that will indicate how PortAudio should continue.
     */
#ifndef USE_ALSA_RECORDER
    static int PortAudioCallback(
        const void* inputBuffer,
        void* outputBuffer,
        unsigned long numSamples,
        const PaStreamCallbackTimeInfo* timeInfo,
        PaStreamCallbackFlags statusFlags,
        void* userData);
#endif

    static void recordDataInputCallback(char* buffer, long unsigned int size, void *userdata);

    /// Initializes PortAudio
    bool initialize();

    std::shared_ptr<duerOSDcsSDK::sdkInterfaces::DcsSdk> m_dcsSdk;

#ifdef USE_ALSA_RECORDER
#ifdef USE_MPC_RECORDER
    MpcRecorder* _m_rec;
#else
    AudioRecorder* _m_rec;
#endif
    bool running;
#else
    /// The PortAudio stream
    PaStream* m_paStream;
#endif
    void (*m_callBack)(const char *buffer, unsigned int size);

    /**
     * A lock to seralize access to startStreamingMicrophoneData() and stopStreamingMicrophoneData() between different
     * threads.
     */
    std::mutex m_mutex;

};

}  // namespace application
}  // namespace duerOSDcsApp

#endif  // DUEROS_DCS_APP_SAMPLEAPP_PORTAUDIOMICROPHONEWRAPPER_H_
