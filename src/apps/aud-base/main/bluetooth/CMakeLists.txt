INCLUDE_DIRECTORIES(../inc)
INCLUDE_DIRECTORIES(../utils/inc)
INCLUDE_DIRECTORIES(../../../../../../../../src/connectivity/bt_others/bluetooth_mw/playback/ALSA)
INCLUDE_DIRECTORIES(../../../../../../../../src/connectivity/bt_others/bluetooth_mw/inc)
INCLUDE_DIRECTORIES(../../../../../../../../src/connectivity/bt_others/bluetooth_mw/inc/config)
INCLUDE_DIRECTORIES(../../../../../../../../src/connectivity/bt_others/bluetooth_mw/inc_rpc)
INCLUDE_DIRECTORIES(../../../../smartaudioplayer/1.0-r0/smartaudioplayer-1.0/include)
INCLUDE_DIRECTORIES(../bluetooth_hfp)
IF(${GATT_TEST_SUPPORT} MATCHES 1)
INCLUDE_DIRECTORIES(../bluetooth_gatt_test)
ENDIF()


AUX_SOURCE_DIRECTORY(. BLUETOOTH)

ADD_DEFINITIONS(-D_LARGEFILE64_SOURCE)

ADD_LIBRARY(bluetooth ${BLUETOOTH})
IF(${GATT_TEST_SUPPORT} MATCHES 1)
TARGET_LINK_LIBRARIES(bluetooth mtk_bt_service_client mas2 bluetooth_gatt_test)
ELSE()
TARGET_LINK_LIBRARIES(bluetooth mtk_bt_service_client mas2)
ENDIF()
