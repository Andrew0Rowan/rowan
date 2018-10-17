#! /bin/bash

if [ ! $BT_SET_ENVIRONMENT ]; then
    source set_environment.sh
fi

Boots_Socket_Path=$BT_Misc_Path

cd ${Bluetooth_Boots_Dir}

rm -rf out

gn gen out/Default/ --args="boots_socket_path = \"${Boots_Socket_Path}\" cc=\"${CC}\" cxx=\"${CXX}\""
ninja -C out/Default all

cd ${Script_Dir}

if [ ! -d ${Bluetooth_Prebuilts_Dir}/bin ]; then
    mkdir -p ${Bluetooth_Prebuilts_Dir}/bin
fi

if [ -f ${Bluetooth_Boots_Dir}/out/Default/boots_srv ]; then
    cp ${Bluetooth_Boots_Dir}/out/Default/boots_srv ${Bluetooth_Prebuilts_Dir}/bin/
else
    exit 1
fi

if [ -f ${Bluetooth_Boots_Dir}/out/Default/boots ]; then
    cp ${Bluetooth_Boots_Dir}/out/Default/boots ${Bluetooth_Prebuilts_Dir}/bin/
else
    exit 1
fi
