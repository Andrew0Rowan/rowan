
if(${Platform} MATCHES "MTK")
    message("DeviceIo MTK编译")
    include_directories(../../bluetooth/bluetooth_mw/inc)
    include_directories(../../bluetooth/bluetooth_mw/inc/config)
    include_directories(../../bluetooth/bluetooth_mw/rpc_ipc/inc)
    include_directories(../../bluetooth/bluetooth_mw/btmw_rpc_test/mtk_rpcipc_bt_service/mtk_bt_service_client/inc)
    include_directories(../../bluetooth/bluetooth_mw/btmw_rpc_test/mtk_rpcipc_bt_service/mtk_bt_service_client/src/inc)
    include_directories(../../bluetooth/bluetooth_mw/btmw_rpc_test/mtk_rpcipc_bt_service/mtk_bt_service_server/inc)
    include_directories(../../bluetooth/bluetooth_mw/btmw_rpc_test/mtk_rpcipc_bt_service/mtk_bt_service_server/src/inc)
    include_directories(../../bluetooth/bluetooth_mw/playback/ALSA)
    include_directories(../../bluetooth/main/inc)
    include_directories(../../bluetooth/main/bluetooth)
    include_directories(../../bluetooth/main/utils/inc)
    include_directories(../../bluetooth/library/bluetooth_hfp_dec)
    include_directories(../../bluetooth/multimedia/smartaudioplayer/include)
    include_directories(../../../dcs-device-sdk/common/device-common-lib/third-party/rapidjson-1.1.0/include)
    include_directories(../../Util/include)
    include_directories(../../DeviceIo/include)
    

    include_directories(${FFMPEG_INCLUDE_DIR})
    include_directories(hodar/oper_sn)

    add_library(DeviceIo SHARED
            hodar/DeviceIo.cpp
            hodar/led.cpp
            hodar/key.cpp
            hodar/volume.cpp
            hodar/bluetooth/bluetooth.cpp
            hodar/bluetooth/bluetooth_hfp.cpp
            hodar/bluetooth/bluetooth_hfp_proc.cpp
            hodar/bluetooth/bluetooth_alsa_interface.cpp
            hodar/bluetooth/bluetooth_hfp_datalist.cpp
            hodar/bluetooth/bluetooth_hfp_msglist.cpp
            hodar/bluetooth/bluetooth_gattc_if.cpp
            hodar/bluetooth/bluetooth_gatts_if.cpp
            hodar/mic_array/CxFlash.cpp
            hodar/mic_array/cxdish.cpp
            hodar/mic_array/host_depend.cpp
            hodar/oper_sn/oper_sn.cpp)
    target_link_libraries(DeviceIo Timer "-L${CMAKE_SOURCE_DIR}/Framework/libs/mtk" mas2 msrc audio.a2dp.default mtk_bt_service_client mtk_bt_service_server ipcrpc bt-mw cvsd_mtk msbc_mtk
            smartaudioplayer nvram nvram_custom file_op asound)

    set(VOLUME_DBG_SOURCE_FILES
             hodar/play_boot.cpp
             hodar/volume.cpp
             )
    add_executable(play_boot ${VOLUME_DBG_SOURCE_FILES})
    target_link_libraries(play_boot LoggerUtils)


elseif(${Platform} MATCHES "Hodor")
    message("DeviceIo Hodor编译")
    include_directories(../../bluetooth/bluetooth_mw/inc)
    include_directories(../../bluetooth/bluetooth_mw/inc/config)
    include_directories(../../bluetooth/bluetooth_mw/rpc_ipc/inc)
    include_directories(../../bluetooth/bluetooth_mw/btmw_rpc_test/mtk_rpcipc_bt_service/mtk_bt_service_client/inc)
    include_directories(../../bluetooth/bluetooth_mw/btmw_rpc_test/mtk_rpcipc_bt_service/mtk_bt_service_client/src/inc)
    include_directories(../../bluetooth/bluetooth_mw/btmw_rpc_test/mtk_rpcipc_bt_service/mtk_bt_service_server/inc)
    include_directories(../../bluetooth/bluetooth_mw/btmw_rpc_test/mtk_rpcipc_bt_service/mtk_bt_service_server/src/inc)
    include_directories(../../bluetooth/bluetooth_mw/playback/ALSA)
    include_directories(../../bluetooth/main/inc)
    include_directories(../../bluetooth/main/bluetooth)
    include_directories(../../bluetooth/main/utils/inc)
    include_directories(../../bluetooth/library/bluetooth_hfp_dec)
    include_directories(../../bluetooth/multimedia/smartaudioplayer/include)
    include_directories(../../../dcs-device-sdk/common/device-common-lib/third-party/rapidjson-1.1.0/include)
    include_directories(../../DeviceIo/include)
    

    include_directories(${FFMPEG_INCLUDE_DIR})
    include_directories(hodar/oper_sn)

    add_library(DeviceIo SHARED
            hodar/DeviceIo.cpp
            hodar/led.cpp
            hodar/volume.cpp
            hodar/touch.cpp
            hodar/oper_sn/oper_sn.cpp
            hodar/bluetooth/bluetooth.cpp
            hodar/bluetooth/bluetooth_hfp.cpp
            hodar/bluetooth/bluetooth_hfp_proc.cpp
            hodar/bluetooth/bluetooth_alsa_interface.cpp
            hodar/bluetooth/bluetooth_hfp_datalist.cpp
            hodar/bluetooth/bluetooth_hfp_msglist.cpp
            hodar/bluetooth/bluetooth_gattc_if.cpp
            hodar/bluetooth/bluetooth_gatts_if.cpp
            hodar/mic_array/CxFlash.cpp
            hodar/mic_array/cxdish.cpp
            hodar/mic_array/host_depend.cpp
            )
    target_link_libraries(DeviceIo Timer LoggerUtils "-L${CMAKE_SOURCE_DIR}/Framework/libs/mtk" mas2 msrc audio.a2dp.default mtk_bt_service_client mtk_bt_service_server ipcrpc bt-mw cvsd_mtk msbc_mtk
            smartaudioplayer nvram nvram_custom file_op asound)

    set(VOLUME_DBG_SOURCE_FILES
             hodar/play_boot.cpp
             hodar/volume.cpp
             )
    add_executable(play_boot ${VOLUME_DBG_SOURCE_FILES})
    target_link_libraries(play_boot LoggerUtils)
else()
    message("DeviceIo Ubuntu编译")
	include_directories(${APPLICATION_INCLUDE_DIR})
    add_library(DeviceIo SHARED linux/DeviceIo.cpp)

    target_link_libraries(DeviceIo adaptor)

endif()

target_include_directories(DeviceIo PUBLIC
        "${DeviceIo_SOURCE_DIR}/include"
        "${LoggerUtils_SOURCE_DIR}/include")

# install target
install(TARGETS ${PROJECT_NAME} DESTINATION ${CMAKE_INSTALL_LIBDIR})

