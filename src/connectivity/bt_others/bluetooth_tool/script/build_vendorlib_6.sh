#! /bin/bash

if [ ! $1 ]; then
    echo use default chip id:mt8167
    export MTK_BT_CHIP_ID=mt8167
else
    export MTK_BT_CHIP_ID=$1
fi
if [ ! $BT_SET_ENVIRONMENT ]; then
    BT_SET_ENVIRONMENT="yes"
    source set_environment.sh
fi

Mtk_Include_Path=${Bluetooth_Stack_Dir}/mediatek/include
Bluedroid_Hci_Include_Path=${Bluetooth_Stack_Dir}/hci/include

Libhw_Include_Path=${Bluetooth_Tool_Dir}/libhardware/include

Nvram_Include_Path=${Bluetooth_Vendor_Lib_Dir}/mtk/bluedroid/external/nvram
Cutils_Include_Path=${Bluetooth_Vendor_Lib_Dir}/mtk/bluedroid/external/cutils
Sysprop_Include_Path=${Bluetooth_Vendor_Lib_Dir}/mtk/bluedroid/external/sysprop
External_Include_For_Vendor_Path=${Bluetooth_Vendor_Lib_Dir}/mtk/bluedroid/external/platform

External_Libs_For_Vendor_Path=${Bluetooth_Vendor_Lib_Dir}/mtk/bluedroid/external/platform


Bdaddr_Flag=-DBD_ADDR_AUTOGEN

#yocto or not
if [ "$MTK_BT_C4A" = "" -o "$MTK_BT_C4A" = "no" ]; then
  #yocto not use C4A and use nvram
  C4A_Flag=-DMTK_BT_NONE_C4A
  FILE_OP_LINK="-lfile_op",
  NVRAM_CUSTOM_LINK="-lnvram_custom"
  NVRAM_LINK="-lnvram"
  NVRAM_FLAG=-DMTK_BT_NVRAM
else
  #none yocto use C4A use properity
  C4A_Flag=-DMTK_BT_C4A
  CUTILS_LINK="-lcutils"
  NVRAM_LINK="-lnvram"
  NVRAM_FLAG=-DMTK_BT_NVRAM
fi

if [ "$MTK_BT_CHIP_ID" = "mt8167" ]; then
  Chip_Flag=-DMTK_MT8167
elif [ "$MTK_BT_CHIP_ID" = "mt6630" ]; then
  Chip_Flag=-DMTK_MT6630
  MERGE_INTERFACE=-D__MTK_MERGE_INTERFACE_SUPPORT__
fi

if [ ! -f ${External_Include_For_Vendor_Path}/CFG_BT_Default.h ]; then
    echo use default CFG_BT_Default.h
    if [ "$MTK_BT_CHIP_ID" = "mt8167" ]; then
        cp ${External_Include_For_Vendor_Path}/../consys_CFG_BT_Default.h ${External_Include_For_Vendor_Path}/CFG_BT_Default.h
    elif [ "$MTK_BT_CHIP_ID" = "mt6630" ]; then
        cp ${External_Include_For_Vendor_Path}/../mt6630_CFG_BT_Default.h ${External_Include_For_Vendor_Path}/CFG_BT_Default.h
    fi
fi
if [ ! -f ${External_Include_For_Vendor_Path}/CFG_BT_File.h ]; then
    echo use default CFG_BT_File.h
    cp ${External_Include_For_Vendor_Path}/../CFG_BT_File.h ${External_Include_For_Vendor_Path}/
fi
if [ ! -f ${External_Include_For_Vendor_Path}/CFG_file_lid.h ]; then
    echo use default CFG_file_lid.h
    cp ${External_Include_For_Vendor_Path}/../CFG_file_lid.h ${External_Include_For_Vendor_Path}/
fi

if [ ! -f ${External_Libs_For_Vendor_Path}/libnvram_custom.so ]; then
    echo use default libnvram_custom.so
    cp ${External_Libs_For_Vendor_Path}/../libnvram_custom.so ${External_Libs_For_Vendor_Path}/
fi
if [ ! -f ${External_Libs_For_Vendor_Path}/libfile_op.so ]; then
    echo use default libfile_op.so
    cp ${External_Libs_For_Vendor_Path}/../libfile_op.so ${External_Libs_For_Vendor_Path}/
fi
if [ ! -f ${External_Libs_For_Vendor_Path}/libnvram.so ]; then
    echo use default libnvram.so
    cp ${External_Libs_For_Vendor_Path}/../libnvram.so ${External_Libs_For_Vendor_Path}/
fi

cd ${Bluetooth_Tool_Dir}

if [ ! -d "libhardware" ]; then
    tar -xvf libhardware.tar.gz
fi

cd ${Bluetooth_Vendor_Lib_Dir}

rm -rf out

gn gen out/Default/ --args="mtk_include_path = \"${Mtk_Include_Path}\" bluedroid_hci_include_path = \"${Bluedroid_Hci_Include_Path}\" libhw_include_path=\"${Libhw_Include_Path}\" nvram_include_path=\"${Nvram_Include_Path}\" cutils_include_path=\"${Cutils_Include_Path}\" sysprop_include_path=\"${Sysprop_Include_Path}\" external_include_for_vendor_path=\"${External_Include_For_Vendor_Path}\" external_libs_for_vendor_path=\"-L${External_Libs_For_Vendor_Path}\" chip_flag=\"${Chip_Flag}\" bdaddr_flag=\"${Bdaddr_Flag}\" c4a_flag=\"${C4A_Flag}\" cutils_link = \"${CUTILS_LINK}\" file_op_link = \"${FILE_OP_LINK}\" nvram_custom_link = \"${NVRAM_CUSTOM_LINK}\" nvram_link = \"${NVRAM_LINK}\" nvram_flag = \"${NVRAM_FLAG}\" bt_sys_log_flag=\"${BT_SYS_LOG_FLAG}\" bt_tmp_path=\"${BT_Tmp_Path}\" bt_misc_path=\"${BT_Misc_Path}\" bt_etc_path=\"${BT_Etc_Path}\" rpc_dbg_flag=\"${Rpc_Dbg_Flag}\" cc=\"${CC}\" cxx=\"${CXX}\" merge_interface=\"${MERGE_INTERFACE}\""
ninja -C out/Default all

cd ${Script_Dir}

if [ ! -d ${Bluetooth_Prebuilts_Dir}/lib ]; then
    mkdir -p ${Bluetooth_Prebuilts_Dir}/lib
fi
if [ ! -d ${Bluetooth_Prebuilts_Dir}/bin ]; then
    mkdir -p ${Bluetooth_Prebuilts_Dir}/bin
fi
if [ -f ${Bluetooth_Vendor_Lib_Dir}/out/Default/libbt-vendor.so ]; then
    cp ${Bluetooth_Vendor_Lib_Dir}/out/Default/libbt-vendor.so ${Bluetooth_Prebuilts_Dir}/lib/
else
    exit 1
fi
if [ -f ${Bluetooth_Vendor_Lib_Dir}/out/Default/libbluetooth_hw_test.so ]; then
    cp ${Bluetooth_Vendor_Lib_Dir}/out/Default/libbluetooth_hw_test.so ${Bluetooth_Prebuilts_Dir}/lib/
else
    exit 1
fi
if [ -f ${Bluetooth_Vendor_Lib_Dir}/out/Default/libbluetooth_mtk_pure.so ]; then
    cp ${Bluetooth_Vendor_Lib_Dir}/out/Default/libbluetooth_mtk_pure.so ${Bluetooth_Prebuilts_Dir}/lib/
else
    exit 1
fi
if [ -f ${Bluetooth_Vendor_Lib_Dir}/out/Default/libbluetooth_relayer.so ]; then
    cp ${Bluetooth_Vendor_Lib_Dir}/out/Default/libbluetooth_relayer.so ${Bluetooth_Prebuilts_Dir}/lib/
else
    exit 1
fi
if [ -f ${Bluetooth_Vendor_Lib_Dir}/out/Default/libbluetoothem_mtk.so ]; then
    cp ${Bluetooth_Vendor_Lib_Dir}/out/Default/libbluetoothem_mtk.so ${Bluetooth_Prebuilts_Dir}/lib/
else
    exit 1
fi
if [ -f ${Bluetooth_Vendor_Lib_Dir}/out/Default/autobt ]; then
    cp ${Bluetooth_Vendor_Lib_Dir}/out/Default/autobt ${Bluetooth_Prebuilts_Dir}/bin/
else
    exit 1
fi
#if [ -f ${Bluetooth_Vendor_Lib_Dir}/out/Default/boots_srv ]; then
#    cp ${Bluetooth_Vendor_Lib_Dir}/out/Default/boots_srv ${Bluetooth_Prebuilts_Dir}/bin/
#else
#    exit 1
#fi
#if [ -f ${Bluetooth_Vendor_Lib_Dir}/out/Default/boots ]; then
#    cp ${Bluetooth_Vendor_Lib_Dir}/out/Default/boots ${Bluetooth_Prebuilts_Dir}/bin/
#else
#    exit 1
#fi
