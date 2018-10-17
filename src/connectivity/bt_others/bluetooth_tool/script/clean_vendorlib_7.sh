#! /bin/bash

if [ ! $BT_SET_ENVIRONMENT ]; then
    source set_environment.sh
fi

rm -rf ${Bluetooth_Tool_Dir}/prebuilts/lib/libbt-vendor.so

cd ${Bluetooth_Vendor_Lib_Dir}/
rm -rf BUILD.gn
rm -rf .gn
rm -rf build
rm -rf external
rm -rf out
cd ${Script_Dir}