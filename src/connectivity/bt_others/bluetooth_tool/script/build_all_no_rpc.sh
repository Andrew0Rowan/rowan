#! /bin/bash

if [ ! $BT_SET_ENVIRONMENT ]; then
    BT_SET_ENVIRONMENT="yes"
    source set_environment.sh
fi

echo "start build vendor lib"
sh build_${VENDOR_LIBRARY} $MTK_BT_CHIP_ID
if [ $? -ne 0 ]; then
    echo vendor_lib compile fail!!
    exit 1
fi

echo "start build stack"
sh build_stack.sh
if [ ! -f ${Bluetooth_Prebuilts_Dir}/lib/libbluetooth.default.so ]; then
    echo build libbluetooth.default.so failed, EXIT!
    exit 1
fi
if [ ! -f ${Bluetooth_Prebuilts_Dir}/lib/libaudio.a2dp.default.so ]; then
    echo build libaudio.a2dp.default.so failed, EXIT!
    exit 1
fi

if [ ! -f ${Bluetooth_Prebuilts_Dir}/conf/bt_stack.conf ]; then
    echo copy bt_stack.conf failed, EXIT!
    exit 1
fi
if [ ! -f ${Bluetooth_Prebuilts_Dir}/conf/bt_did.conf ]; then
    echo copy bt_did.conf failed, EXIT!
    exit 1
fi

echo "start build btut"
sh build_btut.sh
if [ ! -f ${Bluetooth_Prebuilts_Dir}/bin/btut ]; then
    echo build btut failed, EXIT!
    exit 1
fi

echo "start build mw"
sh build_mw.sh
if [ ! -f ${Bluetooth_Prebuilts_Dir}/lib/libbt-mw.so ]; then
    echo build libbt-mw.so failed, EXIT!
    exit 1
fi

#external_lib have ALSA library so should build playback module
if [ -f ${External_Libs_Path}/libasound.so ]; then
    echo "start build playback"
    sh build_playback.sh
    if [ ! -f ${Bluetooth_Prebuilts_Dir}/lib/libbt-alsa-playback.so ]; then
        echo build libbt-alsa-playback.so failed, EXIT!
        exit 1
    fi
else
    echo "no need build playback"
fi
#external_lib have ALSA library so should build uploader module
if [ -f ${External_Libs_Path}/libasound.so ]; then
    echo "start build uploader"
    sh build_uploader.sh
    if [ ! -f ${Bluetooth_Prebuilts_Dir}/lib/libbt-alsa-uploader.so ]; then
        echo build libbt-alsa-uploader.so failed, EXIT!
        exit 1
    fi
else
    echo "no need build uploader"
fi
echo "start build demo"
sh build_non_rpc_test.sh
if [ ! -f ${Bluetooth_Prebuilts_Dir}/bin/btmw-test ]; then
    echo build btmw-test failed, EXIT!
    exit 1
fi

cd ${Script_Dir}
