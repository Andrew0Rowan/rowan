#! /bin/bash

if [ ! $BT_SET_ENVIRONMENT ]; then
    source set_environment.sh
fi

Mw_Config_Path=${Bluetooth_Mw_Dir}/inc/config
Mw_Include_Path=${Bluetooth_Mw_Dir}/inc
Mw_Inc_Path=${Bluetooth_Mw_Dir}/sdk/inc
Libbt_Mw_Path=${Bluetooth_Prebuilts_Dir}/lib

if [ $ENABLE_A2DP_SINK -eq 1 -a -f ${External_Libs_Path}/libasound.so ]; then
    PLAYBACK_LINK="-lbt-alsa-playback"
    Mw_ALSA_Inc_Path=${Bluetooth_Mw_Dir}/playback/ALSA
    echo link ALSA PLAYBACK
else
    PLAYBACK_LINK=""
    Mw_ALSA_Inc_Path=$Mw_Include_Path
fi
if [ $ENABLE_A2DP_SRC -eq 1 -a $ENABLE_A2DP_ADEV -eq 0 -a -f ${External_Libs_Path}/libasound.so ]; then
    UPLOADER_LINK="-lbt-alsa-uploader"
    Mw_ALSA_Uploader_Inc_Path=${Bluetooth_Mw_Dir}/uploader/ALSA
    echo link ALSA UPLOADER
else
    UPLOADER_LINK=""
    Mw_ALSA_Uploader_Inc_Path=$Mw_Include_Path
fi
#if Asound_Inc_Path not set, it means should not use alsa
if [ ! $Asound_Inc_Path ]; then
    Asound_Inc_Path=$Mw_Include_Path
fi

cd ${Bluetooth_Mw_Dir}/non_rpc_test

rm -rf out

gn gen out/Default/ --args="mw_config_path=\"${Mw_Config_Path}\" mw_include_path=\"${Mw_Include_Path}\" mw_alsa_inc_path=\"${Mw_ALSA_Inc_Path}\" asound_inc_path=\"${Asound_Inc_Path}\" mw_alsa_uploader_inc_path=\"${Mw_ALSA_Uploader_Inc_Path}\" libbt_playback_link = \"${PLAYBACK_LINK}\" libbt_uploader_link = \"${UPLOADER_LINK}\" libbt_mw_path=\"-L${Libbt_Mw_Path}\" external_libs_path=\"-L${External_Libs_Path}\" bt_sys_log_flag=\"${BT_SYS_LOG_FLAG}\" bt_tmp_path=\"${BT_Tmp_Path}\" bt_misc_path=\"${BT_Misc_Path}\" bt_etc_path=\"${BT_Etc_Path}\" cc=\"${CC}\" cxx=\"${CXX}\""
ninja -C out/Default all

cd ${Script_Dir}

if [ ! -d ${Bluetooth_Prebuilts_Dir}/bin ]; then
    mkdir -p ${Bluetooth_Prebuilts_Dir}/bin
fi
if [ ! -d ${Bluetooth_Prebuilts_Dir}/lib ]; then
    mkdir -p ${Bluetooth_Prebuilts_Dir}/lib
fi
if [ -f ${Bluetooth_Mw_Dir}/non_rpc_test/out/Default/libbtmw-test.so ]; then
    cp ${Bluetooth_Mw_Dir}/non_rpc_test/out/Default/libbtmw-test.so ${Bluetooth_Prebuilts_Dir}/lib/
else
    exit 1
fi

if [ -f ${Bluetooth_Mw_Dir}/non_rpc_test/out/Default/btmw-test ]; then
    cp ${Bluetooth_Mw_Dir}/non_rpc_test/out/Default/btmw-test ${Bluetooth_Prebuilts_Dir}/bin/
else
    exit 1
fi