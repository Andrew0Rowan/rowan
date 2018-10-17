#! /bin/bash

if [ ! $1 ]; then
    echo use default chip id:mt7662
    export MTK_BT_CHIP_ID=mt7662
else
    export MTK_BT_CHIP_ID=$1
fi
if [ ! $BT_SET_ENVIRONMENT ]; then
    export BT_SET_ENVIRONMENT="yes"
    source set_environment.sh
fi

sh clean_all_rpc.sh

echo "start release build"
sh build_all_rpc.sh
if [ $? -ne 0 ]; then
    echo rpc all compile fail!!
    exit 1
fi

if [ ! -d ${Bluetooth_Prebuilts_Dir}/include ]; then
    mkdir -p ${Bluetooth_Prebuilts_Dir}/include
fi
cp -rf ${Bluetooth_Mw_Dir}/inc ${Bluetooth_Prebuilts_Dir}/include/
cp -rf ${Bluetooth_Mw_Dir}/inc_rpc ${Bluetooth_Prebuilts_Dir}/include/

cd ${Script_Dir}
