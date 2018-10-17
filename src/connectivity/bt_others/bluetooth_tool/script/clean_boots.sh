#! /bin/bash

if [ ! $BT_SET_ENVIRONMENT ]; then
    source set_environment.sh
fi

rm -rf ${Bluetooth_Tool_Dir}/prebuilts/bin/boots_srv
rm -rf ${Bluetooth_Tool_Dir}/prebuilts/bin/boots


cd ${Bluetooth_Boots_Dir}

rm -rf out

cd ${Script_Dir}
