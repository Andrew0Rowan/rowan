#! /bin/bash

if [ ! $1 ]; then
    echo use default chip id:mt7662
    export MTK_BT_CHIP_ID=mt7662
else
    export MTK_BT_CHIP_ID=$1
fi

if [ ! $Script_Dir ]; then
    Script_Dir=$(pwd)
fi
if [ ! $BT_Tool_Dir ]; then
    export BT_Tool_Dir=${Script_Dir}/..
fi

echo $BT_Tool_Dir
export MTK_BT_C4A="no"
export MTK_BT_YOCTOR="no"

sh generate_environment.sh ${MTK_BT_CHIP_ID}

if [ ! $BT_SET_ENVIRONMENT ]; then
    export BT_SET_ENVIRONMENT="yes"
    source set_environment.sh
fi

export Asound_Inc_Path=${Bluetooth_Mw_Dir}/playback/include

sh clean_all_rpc.sh
rm ${Bluetooth_Mw_Dir}/inc/config/c_mw_config.h
rm ${Script_Dir}/set_environment.sh