#! /bin/bash

if [ ! $BT_SET_ENVIRONMENT ]; then
    source set_environment.sh
fi

rm -rf ${Bluetooth_Tool_Dir}/prebuilts/lib/libbluetooth.default.so
rm -rf ${Bluetooth_Tool_Dir}/prebuilts/lib/libaudio.a2dp.default.so
rm -rf ${Bluetooth_Tool_Dir}/prebuilts/conf/*.conf


cd ${Bluetooth_Tool_Dir}
rm -rf zlib-1.2.8
rm -rf core
rm -rf libhardware
rm -rf media

cd ${Bluetooth_Stack_Dir}

rm -rf out
rm -rf third_party

cd ${Script_Dir}
