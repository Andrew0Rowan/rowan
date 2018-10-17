#! /bin/bash

if [ ! $BT_SET_ENVIRONMENT ]; then
    source set_environment.sh
fi

export Bluetooth_RPC_Dbg_Dir=${Bluetooth_Tool_Dir}/dbg

MTK_RPC_IPC_Inc_Path=${Bluetooth_Mw_Dir}/rpc/rpc_ipc/inc
MTK_Inc_RPC_Path=${Bluetooth_Mw_Dir}/inc_rpc

Libipcrpc_Path=${Bluetooth_Prebuilts_Dir}/lib
Libmtk_Bt_Service_Client_Path=${Bluetooth_Prebuilts_Dir}/lib

MTK_RPC_DBG_CONF_PATH=${Bluetooth_Mw_Dir}/rpc/mtk_rpcipc_bt_service/mtk_bt_service_server/src/dbg/config

if [ ! -d ${Bluetooth_RPC_Dbg_Dir}/inc ]; then
    mkdir -p ${Bluetooth_RPC_Dbg_Dir}/inc
fi
cp ${MTK_RPC_DBG_CONF_PATH}/*.h ${Bluetooth_RPC_Dbg_Dir}/inc/

cd ${Bluetooth_RPC_Dbg_Dir}

rm -rf out

gn gen out/Default/ --args="mtk_inc_rpc_path=\"${MTK_Inc_RPC_Path}\" mtk_rpcipc_inc_path=\"${MTK_RPC_IPC_Inc_Path}\" libmtk_bt_service_client_path=\"-L${Libmtk_Bt_Service_Client_Path}\" libipcrpc_path=\"-L${Libipcrpc_Path}\" cc=\"${CC}\" cxx=\"${CXX}\""
ninja -C out/Default all

cd ${Script_Dir}

if [ ! -d ${Bluetooth_Prebuilts_Dir}/bin ]; then
    mkdir -p ${Bluetooth_Prebuilts_Dir}/bin
fi

rm ${Bluetooth_RPC_Dbg_Dir}/inc/*.h

if [ -f ${Bluetooth_RPC_Dbg_Dir}/out/Default/bt-dbg ]; then
    cp ${Bluetooth_RPC_Dbg_Dir}/out/Default/bt-dbg ${Bluetooth_Prebuilts_Dir}/bin/
else
    exit 1
fi
