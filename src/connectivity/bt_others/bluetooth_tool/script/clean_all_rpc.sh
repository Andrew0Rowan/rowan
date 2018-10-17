#! /bin/bash

if [ ! $BT_SET_ENVIRONMENT ]; then
    export BT_SET_ENVIRONMENT="yes"
    source set_environment.sh
fi

echo "start clean vendor lib"
sh clean_${VENDOR_LIBRARY}

echo "start clean boots"
sh clean_boots.sh
echo "start clean stack"
sh clean_stack.sh
echo "start clean btut"
sh clean_btut.sh
echo "start clean mw"
sh clean_mw.sh
if [ $ENABLE_A2DP_SINK -eq 1 -a -f ${External_Libs_Path}/libasound.so ]; then
    echo "start clean playback"
    sh clean_playback.sh
fi

if [ $ENABLE_A2DP_SRC -eq 1 -a $ENABLE_A2DP_ADEV -eq 0 -a -f ${External_Libs_Path}/libasound.so ]; then
    echo "start clean uploader"
    sh clean_uploader.sh
fi
#######################################--Begin RPC--########################################
echo "start clean rpc test"
sh clean_mw_rpc_test.sh
echo "start clean non rpc test"
sh clean_mw_non_rpc_test.sh
echo "start clean rpc"
sh clean_rpc.sh
echo "start clean rpc dbg"
sh clean_rpc_dbg.sh
#######################################--End RPC--########################################

if [ -d ${Bluetooth_Tool_Dir}/prebuilts ]; then
    rm -r ${Bluetooth_Tool_Dir}/prebuilts
fi

if [ -d ${External_Libs_Path}/platform ]; then
    rm -r lib*
fi
