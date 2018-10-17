#! /bin/bash

if [ ! $BT_SET_ENVIRONMENT ]; then
    source set_environment.sh
fi

rm -rf ${Bluetooth_Tool_Dir}/prebuilts/bin/btmw-test
rm -rf ${Bluetooth_Tool_Dir}/prebuilts/lib/libbtmw-test.so

cd ${Bluetooth_Mw_Dir}/non_rpc_test
rm -rf out
cd ${Script_Dir}