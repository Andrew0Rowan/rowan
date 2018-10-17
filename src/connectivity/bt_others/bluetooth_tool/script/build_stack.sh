#! /bin/bash

if [ ! $BT_SET_ENVIRONMENT ]; then
    source set_environment.sh
fi

Libhw_Include_Path=${Bluetooth_Tool_Dir}/libhardware/include
Core_Include_Path=${Bluetooth_Tool_Dir}/core/include
Audio_Include_Path=${Bluetooth_Tool_Dir}/media/audio/include
Zlib_Include_Path=${Bluetooth_Tool_Dir}/zlib-1.2.8

if [ "$SUPPORT_AAC" = "yes" ]; then
    ENABLE_AAC_FLAG="-DENABLE_CODEC_AAC"
    AAC_LINK="-laac"
    AAC_Include_Path=${Bluetooth_Tool_Dir}/external_libs/platform/include
else
    ENABLE_AAC_FLAG=""
    AAC_LINK=""
    AAC_Include_Path=${Bluetooth_Tool_Dir}/external_libs/platform/include
fi
Zlib_Path=${External_Libs_Path}

echo ${Bluetooth_Tool_Dir}
cd ${Bluetooth_Tool_Dir}

if [ ! -d "zlib-1.2.8" ]; then
    tar -xvf zlib.tar.gz
    if [ ! -f ${External_Libs_Path}/libz.so ]; then
        echo "use local libz.so"
        cd zlib-1.2.8
        export CC=${CC}
        ./configure && make
        cp ${Bluetooth_Tool_Dir}/zlib-1.2.8/libz.so.1 ${External_Libs_Path}/
        cp ${Bluetooth_Tool_Dir}/zlib-1.2.8/libz.so ${External_Libs_Path}/
        cd ..
    else
        echo "use platform libz.so"
    fi
fi

if [ ! -d "core" ]; then
    tar -xvf core.tar.gz
fi

if [ ! -d "libhardware" ]; then
    tar -xvf libhardware.tar.gz
fi

if [ ! -d "media" ]; then
    tar -xvf media.tar.gz
fi

cd ${Bluetooth_Stack_Dir}

if [ ! -d "third_party" ]; then
    tar -xvf third_party.tar.gz
fi

rm -rf out

gn gen out/Default/ --args="libhw_include_path=\"${Libhw_Include_Path}\" core_include_path=\"${Core_Include_Path}\" audio_include_path=\"${Audio_Include_Path}\" zlib_include_path=\"${Zlib_Include_Path}\" zlib_path=\"-L${Zlib_Path}\" conf_path=\"${Conf_Path}\" cache_path=\"${Cache_Path}\" bt_tmp_path=\"${BT_Tmp_Path}\" bt_misc_path=\"${BT_Misc_Path}\" bt_etc_path=\"${BT_Etc_Path}\" rpc_dbg_flag=\"${Rpc_Dbg_Flag}\" cc=\"${CC}\" cxx=\"${CXX}\" bt_sys_log_flag=\"${BT_SYS_LOG_FLAG}\" enable_aac=\"${ENABLE_AAC_FLAG}\" aac_include_path=\"${AAC_Include_Path}\" aac_link=\"${AAC_LINK}\""

ninja -C out/Default all

if [ ! -d ${Bluetooth_Prebuilts_Dir}/lib ]; then
    mkdir -p ${Bluetooth_Prebuilts_Dir}/lib
fi

if [ ! -d ${Bluetooth_Prebuilts_Dir}/conf ]; then
    mkdir -p ${Bluetooth_Prebuilts_Dir}/conf
fi

cd ${Script_Dir}


if [ -f ${Bluetooth_Stack_Dir}/out/Default/libbluetooth.default.so ]; then
    cp ${Bluetooth_Stack_Dir}/out/Default/libbluetooth.default.so ${Bluetooth_Prebuilts_Dir}/lib/
    cp ${Bluetooth_Stack_Dir}/conf/bt_stack.conf.linux ${Bluetooth_Prebuilts_Dir}/conf/bt_stack.conf
    cp ${Bluetooth_Stack_Dir}/conf/bt_did.conf ${Bluetooth_Prebuilts_Dir}/conf/bt_did.conf
else
    exit 1
fi
if [ -f ${Bluetooth_Stack_Dir}/out/Default/libaudio.a2dp.default.so ]; then
    cp ${Bluetooth_Stack_Dir}/out/Default/libaudio.a2dp.default.so ${Bluetooth_Prebuilts_Dir}/lib/
else
    exit 1
fi