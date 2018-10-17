INCLUDE_DIRECTORIES(../inc)
INCLUDE_DIRECTORIES(../bluetooth)
INCLUDE_DIRECTORIES(../utils/inc)
INCLUDE_DIRECTORIES(../../../../../../../../src/apps/aud-base/library/bluetooth_hfp_dec)
INCLUDE_DIRECTORIES(../../../../../../../../src/connectivity/bt_others/bluetooth_mw/playback/ALSA)
INCLUDE_DIRECTORIES(../../../../../../../../src/connectivity/bt_others/bluetooth_mw/inc)
INCLUDE_DIRECTORIES(../../../../../../../../src/connectivity/bt_others/bluetooth_mw/inc/config)
INCLUDE_DIRECTORIES(../../../../../../../../src/connectivity/bt_others/bluetooth_mw/inc_rpc)
INCLUDE_DIRECTORIES(../../../../smartaudioplayer/1.0-r0/smartaudioplayer-1.0/include)
INCLUDE_DIRECTORIES(../../../../../../../../src/apps/aud-base/library/webrtc_aec)

AUX_SOURCE_DIRECTORY(. BLUETOOTH_HFP)

ADD_DEFINITIONS(-D_LARGEFILE64_SOURCE)

ADD_LIBRARY(bluetooth_hfp ${BLUETOOTH_HFP})
TARGET_LINK_LIBRARIES(bluetooth_hfp cvsd_mtk msbc_mtk aec pcm_merge msb msrc)
