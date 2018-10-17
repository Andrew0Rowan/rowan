#! /bin/bash

if [ ! $BT_SET_ENVIRONMENT ]; then
    source set_environment.sh
fi

rm -rf ${Bluetooth_Tool_Dir}/prebuilts/lib/libipcrpc.so
rm -rf ${Bluetooth_Tool_Dir}/prebuilts/lib/libmtk_bt_ipcrpc_struct.so
rm -rf ${Bluetooth_Tool_Dir}/prebuilts/lib/libmtk_bt_service_client.so
rm -rf ${Bluetooth_Tool_Dir}/prebuilts/lib/libmtk_bt_service_server.so
rm -rf ${Bluetooth_Tool_Dir}/prebuilts/bin/btservice

cd ${Bluetooth_Mw_Dir}/rpc
rm -rf out
cd ${Script_Dir}