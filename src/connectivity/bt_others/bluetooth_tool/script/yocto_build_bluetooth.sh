#! /bin/bash

BT_Tool_Dir=$1/../src/connectivity/bt_others/bluetooth_tool
export MTK_BT_CHIP_ID=$2
export MTK_BT_PROJECT=$3
export MTK_BT_C4A="no"
export CC=$CC
export CXX=$CXX
export BT_GET_CROSS_COMPILE="yes"
export Asound_Inc_Path=$1/tmp/sysroots/${MTK_BT_PROJECT}/usr/include/alsa

echo $PWD
echo hello yocto bluetooth
echo ${BT_Tool_Dir}

cd ${BT_Tool_Dir}/script
echo $PWD

MTK_PJ_PATH=aud-base

if [ "$MTK_BT_PROJECT" = "aud8516-ali-slc" ]; then
MTK_PJ_PATH=aud-ali
fi

echo "MTK_BT_PROJECT = $MTK_BT_PROJECT"
if [ "$MTK_BT_PROJECT" = "aud8516-ali-slc" ]; then
export BT_SYS_LOG_FLAG=-DMTK_BT_SYS_LOG
fi

export BT_PLAYBACK_ALSA_flag=""

echo ${MTK_BT_PROJECT}
echo ${MTK_PJ_PATH}
if [ ! $BT_SET_ENVIRONMENT ]; then
    export BT_SET_ENVIRONMENT="yes"
    source ${BT_Tool_Dir}/script/set_environment.sh
fi

sh clean_all_rpc.sh

if [ "$MTK_BT_CHIP_ID" = "mt8167" -o "$MTK_BT_CHIP_ID" = "mt6630" ]; then
    #temp mark this copy due to these files not correctly
    cp $1/../src/support/libnvram_custom/CFG/${MTK_BT_PROJECT}/linux-4.4.22/common/cgen/cfgdefault/CFG_BT_Default.h ${Bluetooth_Vendor_Lib_Dir}/mtk/bluedroid/external/platform/
    cp $1/../src/support/libnvram_custom/CFG/${MTK_BT_PROJECT}/linux-4.4.22/common/cgen/cfgfileinc/CFG_BT_File.h ${Bluetooth_Vendor_Lib_Dir}/mtk/bluedroid/external/platform/
    cp $1/../src/support/libnvram_custom/CFG/${MTK_BT_PROJECT}/linux-4.4.22/common/cgen/inc/CFG_file_lid.h ${Bluetooth_Vendor_Lib_Dir}/mtk/bluedroid/external/platform/
    cp $1/tmp/sysroots/${MTK_BT_PROJECT}/${base_libdir}/libnvram_custom.so ${Bluetooth_Vendor_Lib_Dir}/mtk/bluedroid/external/platform/
    cp $1/tmp/sysroots/${MTK_BT_PROJECT}/${base_libdir}/libfile_op.so ${Bluetooth_Vendor_Lib_Dir}/mtk/bluedroid/external/platform/
    cp $1/tmp/sysroots/${MTK_BT_PROJECT}/${base_libdir}/libnvram.so ${Bluetooth_Vendor_Lib_Dir}/mtk/bluedroid/external/platform/
fi

if [ "$SUPPORT_AAC" = "yes" ]; then
    cp $1/../src/multimedia/audio-misc/libaac/libAACdec/include/aacdecoder_lib.h ${BT_Tool_Dir}/external_libs/platform/include/
    cp $1/../src/multimedia/audio-misc/libaac/libAACenc/include/aacenc_lib.h ${BT_Tool_Dir}/external_libs/platform/include/
    cp -r $1/../src/multimedia/audio-misc/libaac/libSYS/include/*.h ${BT_Tool_Dir}/external_libs/platform/include/
fi

cp $1/tmp/sysroots/${MTK_BT_PROJECT}/${libdir}/libasound.so ${BT_Tool_Dir}/external_libs/platform/
cp $1/tmp/sysroots/${MTK_BT_PROJECT}/${libdir}/libz.so ${BT_Tool_Dir}/external_libs/platform/

sh build_all_rpc.sh
