#! /bin/bash

export BT_GET_CROSS_COMPILE="yes"
BT_Tool_Dir=$1/../src/connectivity/bt_others/bluetooth_tool
if [ $2 ]; then
    export MTK_BT_CHIP_ID=$2
fi
echo $PWD
echo clean yocto bluetooth

cd ${BT_Tool_Dir}/script
echo $PWD

if [ ! $BT_SET_ENVIRONMENT ]; then
    export BT_SET_ENVIRONMENT="yes"
    source ${BT_Tool_Dir}/script/set_environment.sh
fi

sh clean_all_rpc.sh

if [ -d ${Bluetooth_Tool_Dir}/prebuilts ]; then
    rm -r ${Bluetooth_Tool_Dir}/prebuilts
fi
rm -f ${Bluetooth_Mw_Dir}/inc/config/c_mw_config.h
rm -r ${Script_Dir}/set_environment.sh