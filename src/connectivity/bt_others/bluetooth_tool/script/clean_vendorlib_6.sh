#! /bin/bash

if [ ! $BT_SET_ENVIRONMENT ]; then
    source set_environment.sh
fi

rm -rf ${Bluetooth_Tool_Dir}/prebuilts/lib/libbt-vendor.so
rm -rf ${Bluetooth_Tool_Dir}/prebuilts/lib/libbluetooth_hw_test.so
rm -rf ${Bluetooth_Tool_Dir}/prebuilts/lib/libbluetooth_mtk_pure.so
rm -rf ${Bluetooth_Tool_Dir}/prebuilts/lib/libbluetooth_relayer.so
rm -rf ${Bluetooth_Tool_Dir}/prebuilts/lib/libbluetoothem_mtk.so
rm -rf ${Bluetooth_Tool_Dir}/prebuilts/bin/autobt

cd ${Bluetooth_Tool_Dir}
rm -rf libhardware

cd ${Bluetooth_Vendor_Lib_Dir}
rm -rf out
rm -rf mtk/bluedroid/external/platform/CFG_BT_Default.h
rm -rf mtk/bluedroid/external/platform/CFG_BT_File.h
rm -rf mtk/bluedroid/external/platform/CFG_file_lid.h
rm -rf mtk/bluedroid/external/platform/libfile_op.so
rm -rf mtk/bluedroid/external/platform/libnvram.so
rm -rf mtk/bluedroid/external/platform/libnvram_custom.so

cd ${Script_Dir}

