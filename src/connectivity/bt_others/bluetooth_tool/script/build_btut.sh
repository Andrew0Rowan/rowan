#! /bin/bash

if [ ! $BT_SET_ENVIRONMENT ]; then
    source set_environment.sh
fi

Libhw_Include_Path=${Bluetooth_Tool_Dir}/libhardware/include
Core_Include_Path=${Bluetooth_Tool_Dir}/core/include
Btif_Include_Path=${Bluetooth_Stack_Dir}/btif/include
Stack_Include_Path=${Bluetooth_Stack_Dir}/stack/include
Utils_Include_Path=${Bluetooth_Stack_Dir}/utils/include
Mtk_Bt_Include_Path=${Bluetooth_Stack_Dir}/mediatek/include
if [ ! -f ${External_Libs_Path}/libz.so ]; then
    External_Libs_Path=${Bluetooth_Tool_Dir}/external_libs/local_lib
fi

cd ${Bluetooth_Tool_Dir}/btut

rm -rf out

gn gen out/Default/ --args="libhw_include_path=\"${Libhw_Include_Path}\" core_include_path=\"${Core_Include_Path}\" btif_include_path=\"${Btif_Include_Path}\" stack_include_path=\"${Stack_Include_Path}\" utils_include_path=\"${Utils_Include_Path}\" mtk_bt_include_path=\"${Mtk_Bt_Include_Path}\" bluedroid_libs_path=\"-L${Bluedroid_Libs_Path}\" external_libs_path=\"-L${External_Libs_Path}\" bt_tmp_path=\"${BT_Tmp_Path}\" bt_misc_path=\"${BT_Misc_Path}\" bt_etc_path=\"${BT_Etc_Path}\" bt_sys_log_flag=\"${BT_SYS_LOG_FLAG}\" cc=\"${CC}\" cxx=\"${CXX}\""
ninja -C out/Default all

if [ ! -d ${Bluetooth_Prebuilts_Dir}/bin ]; then
    mkdir -p ${Bluetooth_Prebuilts_Dir}/bin
fi

if [ -f ${Bluetooth_Tool_Dir}/btut/out/Default/btut ]; then
    cp ${Bluetooth_Tool_Dir}/btut/out/Default/btut ${Bluetooth_Prebuilts_Dir}/bin/
else
    exit 1
fi

cd ${Script_Dir}