#! /bin/bash

if [ ! $BT_SET_ENVIRONMENT ]; then
    source set_environment.sh
fi

Mw_Inc_Path=${Bluetooth_Mw_Dir}/sdk/inc
Stack_Include_Path=${Bluetooth_Stack_Dir}/mediatek/include
Stack_Root_Path=${Bluetooth_Stack_Dir}
Libasound_Path=${External_Libs_Path}

echo "TARGET_PRODUCT = $TARGET_PRODUCT"
echo "MTK_BT_PROJECT = $MTK_BT_PROJECT"

cd ${Bluetooth_Mw_Dir}/playback

rm -rf out

gn gen out/Default/ --args="stack_include_path=\"${Stack_Include_Path}\" stack_root_path=\"${Stack_Root_Path}\" asound_inc_path=\"${Asound_Inc_Path}\" mw_inc_path=\"${Mw_Inc_Path}\" libasound_path=\"-L${Libasound_Path}\" bluedroid_libs_path=\"-L${Bluedroid_Libs_Path}\" bt_sys_log_flag=\"${BT_SYS_LOG_FLAG}\" bt_tmp_path=\"${BT_Tmp_Path}\" bt_misc_path=\"${BT_Misc_Path}\" bt_etc_path=\"${BT_Etc_Path}\" cc=\"${CC}\" cxx=\"${CXX}\""
ninja -C out/Default all

cd ${Script_Dir}

if [ ! -d ${Bluetooth_Prebuilts_Dir}/lib ]; then
    mkdir -p ${Bluetooth_Prebuilts_Dir}/lib
fi

if [ -f ${Bluetooth_Mw_Dir}/playback/out/Default/libbt-alsa-playback.so ]; then
    cp ${Bluetooth_Mw_Dir}/playback/out/Default/libbt-alsa-playback.so ${Bluetooth_Prebuilts_Dir}/lib/
else
    exit 1
fi
