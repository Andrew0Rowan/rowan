#! /bin/bash

if [ ! $1 ]; then
    echo use default chip id:mt8167
    export MTK_BT_CHIP_ID=mt8167
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
export CC=/mtkoss/gnuarm/hard_4.9.2-r116_armv7a-cros/x86_64/armv7a/usr/x86_64-pc-linux-gnu/armv7a-cros-linux-gnueabi/gcc-bin/4.9.x-google/armv7a-cros-linux-gnueabi-gcc
export CXX=/mtkoss/gnuarm/hard_4.9.2-r116_armv7a-cros/x86_64/armv7a/usr/x86_64-pc-linux-gnu/armv7a-cros-linux-gnueabi/gcc-bin/4.9.x-google/armv7a-cros-linux-gnueabi-g++

sh generate_environment.sh ${MTK_BT_CHIP_ID}

if [ ! $BT_SET_ENVIRONMENT ]; then
    export BT_SET_ENVIRONMENT="yes"
    source set_environment.sh
fi

export Asound_Inc_Path=${Bluetooth_Mw_Dir}/playback/include

sh clean_all_rpc.sh

if [ "$MTK_BT_CHIP_ID" = "mt8167" ]; then
    cp ${BT_Tool_Dir}/vendor_libs/mtk/bluedroid/external/consys_CFG_BT_Default.h ${BT_Tool_Dir}/vendor_libs/mtk/bluedroid/external/platform/CFG_BT_Default.h
elif [ "$MTK_BT_CHIP_ID" = "mt6630" ]; then
    cp ${BT_Tool_Dir}/vendor_libs/mtk/bluedroid/external/mt6630_CFG_BT_Default.h ${BT_Tool_Dir}/vendor_libs/mtk/bluedroid/external/platform/CFG_BT_Default.h
fi

if [ "$MTK_BT_CHIP_ID" = "mt8167" -o "$MTK_BT_CHIP_ID" = "mt6630" ]; then
    cp ${BT_Tool_Dir}/vendor_libs/mtk/bluedroid/external/CFG_BT_File.h ${BT_Tool_Dir}/vendor_libs/mtk/bluedroid/external/platform/
    cp ${BT_Tool_Dir}/vendor_libs/mtk/bluedroid/external/CFG_file_lid.h ${BT_Tool_Dir}/vendor_libs/mtk/bluedroid/external/platform/
fi
cp ${BT_Tool_Dir}/external_libs/libasound.so ${BT_Tool_Dir}/external_libs/platform/
#cp ${BT_Tool_Dir}/external_libs/libz.so ${BT_Tool_Dir}/external_libs/platform/

sh build_all_rpc.sh