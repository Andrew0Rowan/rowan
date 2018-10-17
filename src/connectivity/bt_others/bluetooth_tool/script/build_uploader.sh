#! /bin/bash

if [ ! $BT_SET_ENVIRONMENT ]; then
    source set_environment.sh
fi

Asound_Inc_Path=${Bluetooth_Mw_Dir}/playback/include
Mw_Inc_Path=${Bluetooth_Mw_Dir}/sdk/inc

Libasound_Path=${External_Libs_Path}
Libbt_Mw_Path=${Bluetooth_Prebuilts_Dir}/lib

cd ${Bluetooth_Mw_Dir}/uploader

rm -rf out

gn gen out/Default/ --args="asound_inc_path=\"${Asound_Inc_Path}\" mw_inc_path=\"${Mw_Inc_Path}\" libasound_path=\"-L${Libasound_Path}\" libbt_mw_path=\"-L${Libbt_Mw_Path}\" cc=\"${CC}\" cxx=\"${CXX}\""
ninja -C out/Default all

cd ${Script_Dir}

if [ ! -d ${Bluetooth_Prebuilts_Dir}/lib ]; then
    mkdir -p ${Bluetooth_Prebuilts_Dir}/lib
fi

cp ${Bluetooth_Mw_Dir}/uploader/out/Default/*.so ${Bluetooth_Prebuilts_Dir}/lib/