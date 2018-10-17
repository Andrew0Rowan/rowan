#! /bin/bash

if [ ! $BT_SET_ENVIRONMENT ]; then
    source set_environment.sh
fi

cd ${Bluetooth_Mw_Dir}/rpc/mtk_rpcipc_bt_service/tool/A2DP
./command.sh

cd ${Bluetooth_Mw_Dir}/rpc/mtk_rpcipc_bt_service/tool/AVRCP
./command.sh

cd ${Bluetooth_Mw_Dir}/rpc/mtk_rpcipc_bt_service/tool/GAP
./command.sh

cd ${Bluetooth_Mw_Dir}/rpc/mtk_rpcipc_bt_service/tool/GATT
./command.sh

cd ${Bluetooth_Mw_Dir}/rpc/mtk_rpcipc_bt_service/tool/HFP
./command.sh

cd ${Bluetooth_Mw_Dir}/rpc/mtk_rpcipc_bt_service/tool/HID
./command.sh

cd ${Bluetooth_Mw_Dir}/rpc/mtk_rpcipc_bt_service/tool/SPP
./command.sh


