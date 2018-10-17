if(BUILD_TTS_SDK)
    add_definitions(-DLOCALTTS)
    set(LocalSynthesizerPlayer "MediaPlayer/LocalSynthesizerPlayer.cpp")
    set(LocalTtsProxy "MediaPlayer/LocalTtsProxy.cpp")
    set(TTS_SDK_LIB "ASRTTSManager")
endif()

if(${Platform} STREQUAL Sengled
        OR ${Platform} STREQUAL Ingenic)
    set(vendor_AUDIO_INPUT_SOURCE_FILES "Vendor/${Platform}/src/AudioInputWrapper.cpp")
    set(DCSApplication_SOURCE_FILES "Vendor/${Platform}/src/DCSApplication.cpp")
    include_directories("Vendor/${Platform}/include")
    link_directories("Vendor/${Platform}/lib")
else()
    set(DCSApplication_SOURCE_FILES "DCSApplication.cpp")
endif()

option(USE_MPCTEC_ALGO "Use MPCTEC algorithm for audio signal processing" OFF)

if(USE_MPCTEC_ALGO)
    add_definitions(-DUSE_MPC_RECORDER)
    set(MPCRECORDER_SRC_FILE MpcRecorder.cpp)
    set(MPCTEC_LIB algo_data_client utils)
    message("use mpctec recorder")
else()
    set(MPCRECORDER_SRC_FILE "")
    set(MPCTEC_LIB "")
endif()

if(BUILD_USE_ALSA_RECORDER)
    add_definitions(-DUSE_ALSA_RECORDER)
    set(ALSARECORDER_SRC_FILE "AlsaRecorder.cpp")
    message("use alsa recorder")
else()
    set(ALSARECORDER_SRC_FILE "")
    message("use port audio")
endif()

message("USE_ALSA_RECORDER=${BUILD_USE_ALSA_RECORDER}")
message("${ALSARECORDER_SRC_FILE}")

add_executable(duer_linux
        main.cpp
        PortAudioMicrophoneWrapper.cpp
        ${ALSARECORDER_SRC_FILE}
        ${MPCRECORDER_SRC_FILE}
        ApplicationManager.cpp
        ${DCSApplication_SOURCE_FILES}
        SoundController.cpp
        DeviceIoWrapper.cpp
        Configuration.cpp
        DuerLinkMtkInstance.cpp
        DuerLinkWrapper.cpp
        SystemUpdateRevWrapper.cpp
        ThreadPoolExecutor.cpp
        VoiceAndTouchWakeUpObserver.cpp
        ActivityMonitorSingleton.cpp
        RecordAudioManager.cpp
        MediaPlayer/AlsaController.cpp
        MediaPlayer/StreamPool.cpp
        MediaPlayer/RapidPcmPlayer.cpp
        MediaPlayer/TtsPlayer.cpp
        MediaPlayer/TtsPlayerProxy.cpp
        MediaPlayer/AlertsPlayerProxy.cpp
        MediaPlayer/LocalPlayerProxy.cpp
        MediaPlayer/AudioDecoder.cpp
        MediaPlayer/SimpleAudioplayerImpl.cpp
        MediaPlayer/PcmBufPool.cpp
        MediaPlayer/TimerUtil.cpp
        MediaPlayer/AudioPlayerImpl.cpp
        MediaPlayer/MediaPlayerProxy.cpp
        MediaPlayer/OffsetManager.cpp
        MediaPlayer/BlueToothPlayerProxy.cpp
        MediaPlayer/PcmResampler.cpp
        ${LocalTtsProxy}
        ${LocalSynthesizerPlayer}
        ${vendor_AUDIO_INPUT_SOURCE_FILES})

target_include_directories(duer_linux PUBLIC
    "${DCSApp_SOURCE_DIR}/include"
    "${DCSApp_SOURCE_DIR}/include/MediaPlayer"
    "${BT_MW_INCLUDE_DIR}/inc_rpc"
    "${APPLICATION_INCLUDE_DIR}"
    "${DeviceIo_SOURCE_DIR}/include"
    "${duerLink_SOURCE_DIR}/include"
    "${PORTAUDIO_INCLUDE_DIR}"
    "${FFMPEG_INCLUDE_DIR}"
    "${ASRTTSManager_SOURCE_DIR}/include"
    "${DeviceTools_SOURCE_DIR}/include"
    )

if(BUILD_ONE_LIB)
    set(DCS_SDK_LIB DcsSdk DeviceTools)
else()
    set(DCS_SDK_LIB DcsClient AuthDelegate)
endif()
	
find_library(LIBBTSERVICE_PATH mtk_bt_service_client "${APPLICATION_LD_DIR}")
if(NOT LIBBTSERVICE_PATH)
	message(FATAL_ERROR "mtk_bt_service_client not found in ${APPLICATION_LD_DIR}")
endif(NOT LIBBTSERVICE_PATH)

message(STATUS "${LIBBTSERVICE_PATH} found")

find_library(LIBWIFIMW_PATH wlanMtk "${APPLICATION_LD_DIR}")
if(NOT LIBWIFIMW_PATH)
	message(FATAL_ERROR "wlanMtk not found in ${APPLICATION_LD_DIR}")
endif(NOT LIBWIFIMW_PATH)

message(STATUS "${LIBWIFIMW_PATH} found")

target_link_libraries(duer_linux
        ${DCS_SDK_LIB}
        DeviceIo
        adaptor
        uicustom
        ${LIBBTSERVICE_PATH}
        ${LIBWIFIMW_PATH}
        duerLink
        asound
        avcodec
        avutil
        avformat
        swresample
        rt
        m
        pthread
        nghttp2
        LoggerUtils
        Configuration
        "${TTS_SDK_LIB}"
        ${MPCTEC_LIB}
        )

# use gperftools
if(BUILD_GPERFTOOLS)
    message("启用gperftools")
    target_link_libraries(duer_linux tcmalloc)
endif()

# for unbuntu platform
#target_link_libraries(duer_linux "-L${FFMPEG_LIB_PATH}" asound avcodec avutil avformat
#swresample -L/usr/lib/atlas-base f77blas cblas lapack_atlas atlas)

if(${Platform} STREQUAL Sengled)
   target_link_libraries(duer_linux sqlite3 z sai_preprocess)
endif()

if(${Platform} STREQUAL Ingenic)
   target_link_libraries(duer_linux atomic four_mic_front_end_alg_client hardware utils cJSON network_manager)
endif()

install(TARGETS duer_linux DESTINATION "${CMAKE_INSTALL_BINDIR}/dueros")
