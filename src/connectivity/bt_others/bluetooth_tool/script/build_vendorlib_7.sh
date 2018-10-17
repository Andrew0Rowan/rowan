#! /bin/bash

if [ ! $BT_SET_ENVIRONMENT ]; then
    source set_environment.sh
fi

External_Dir=${Bluetooth_Vendor_Lib_Dir}/external

Mtk_Include_Path=${External_Dir}/include/mediatek/include
Bluedroid_Hci_Include_Path=${External_Dir}/include/hci/include

Libhw_Include_Path=${External_Dir}/libhardware/include

Cutils_Include_Path=${External_Dir}/include/cutils
Sysprop_Include_Path=${External_Dir}/include/sysprop

External_Libs_For_Vendor_Path=${External_Dir}/libs
if [ "$MTK_BT_CHIP_ID" = "mt7662" ]; then
    Chip_Flag=-DMTK_MT7662
elif [ "$MTK_BT_CHIP_ID" = "mt7668" ]; then
    Chip_Flag=-DMTK_MT7668
else
    echo "invalid chip id"
    exit 1
fi

Bdaddr_Flag=-DBD_ADDR_AUTOGEN

if [ ! -d ${Bluetooth_Vendor_Lib_Dir} ]; then
    echo "vendor library path not exist"
    exit 1
fi

cd ${Bluetooth_Vendor_Lib_Dir}

if [ ! -d "build" ]; then
    tar -xvf build.tar.gz
fi

if [ ! -d "external" ]; then
    tar -xvf external.tar.gz
fi

rm -rf out

gn gen out/Default/ --args="mtk_include_path = \"${Mtk_Include_Path}\" bluedroid_hci_include_path = \"${Bluedroid_Hci_Include_Path}\" libhw_include_path=\"${Libhw_Include_Path}\" cutils_include_path=\"${Cutils_Include_Path}\" sysprop_include_path=\"${Sysprop_Include_Path}\" external_libs_for_vendor_path=\"-L${External_Libs_For_Vendor_Path}\" chip_flag=\"${Chip_Flag}\" bdaddr_flag=\"${Bdaddr_Flag}\" cc=\"${CC}\" cxx=\"${CXX}\""
ninja -C out/Default all

cd ${Script_Dir}

if [ ! -d ${Bluetooth_Prebuilts_Dir}/lib ]; then
    mkdir -p ${Bluetooth_Prebuilts_Dir}/lib
fi

if [ -f ${Bluetooth_Vendor_Lib_Dir}/out/Default/libbt-vendor.so ]; then
    cp ${Bluetooth_Vendor_Lib_Dir}/out/Default/libbt-vendor.so ${Bluetooth_Prebuilts_Dir}/lib/
else
    exit 1
fi
