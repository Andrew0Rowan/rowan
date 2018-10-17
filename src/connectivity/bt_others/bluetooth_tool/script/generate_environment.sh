#! /bin/bash

if [ ! $MTK_BT_CHIP_ID ]; then
    MTK_BT_CHIP_ID=$1
    echo ${MTK_BT_CHIP_ID}
fi

if [ ! $Script_Dir ]; then
    Script_Dir=$(pwd)
fi
echo script dir: $Script_Dir
if [ ! $Bluetooth_Tool_Dir ]; then
    export Bluetooth_Tool_Dir=${Script_Dir}/..
fi

if [ ! $Bluetooth_Mw_Dir ]; then
    export Bluetooth_Mw_Dir=${Bluetooth_Tool_Dir}/../bluetooth_mw
fi

if [ ! $Bluetooth_Stack_Dir ]; then
    export Bluetooth_Stack_Dir=${Bluetooth_Tool_Dir}/../../bt_stack/bluedroid_turnkey
fi
if [ ! $Bluetooth_Vendor_Lib_Dir ]; then
    if [ ! "$MTK_BT_CHIP_ID" ]; then
        echo please input MTK_BT_CHIP_ID !!!
        exit 1
    elif [ "$MTK_BT_CHIP_ID" = "mt8167" -o "$MTK_BT_CHIP_ID" = "mt6630" ]; then
        if [ -d ${Bluetooth_Tool_Dir}/../vendor_lib_66xx ]; then
            export Bluetooth_Vendor_Lib_Dir=${Bluetooth_Tool_Dir}/../vendor_lib_66xx
            export VENDOR_LIBRARY=vendorlib_6.sh
        elif [ -d ${Bluetooth_Tool_Dir}/../vendor_lib/mt66xx ]; then
            export Bluetooth_Vendor_Lib_Dir=${Bluetooth_Tool_Dir}/../vendor_lib/mt66xx
            export VENDOR_LIBRARY=vendorlib_6.sh
        else
            echo vendor_lib_66xx not exist !!!
            exit 1
        fi
    else
        if [ -d ${Bluetooth_Tool_Dir}/../vendor_lib/mt76xx ]; then
            export Bluetooth_Vendor_Lib_Dir=${Bluetooth_Tool_Dir}/../vendor_lib/mt76xx
        elif [ -d ${Bluetooth_Tool_Dir}/../vendor_lib ]; then
            export Bluetooth_Vendor_Lib_Dir=${Bluetooth_Tool_Dir}/../vendor_lib
        else
            echo vendor_lib not exist !!!
            exit 1
        fi
        export VENDOR_LIBRARY=vendorlib_7.sh
    fi
fi
echo $VENDOR_LIBRARY
if [ ! $Bluetooth_Boots_Dir ]; then
    export Bluetooth_Boots_Dir=${Bluetooth_Tool_Dir}/../boots
fi

##########for different platform only need modify below export variable##########
if [ "$SUPPORT_SPP" = "yes" ]; then
    ENABLE_SPP_PROFILE=1
else
    #spp default is disable
    ENABLE_SPP_PROFILE=0
fi
if [ "$SUPPORT_HIDH" = "yes" ]; then
    ENABLE_HID_PROFILE_H=1
else
    #hid host default is disable
    ENABLE_HID_PROFILE_H=0
fi
if [ "$SUPPORT_HIDD" = "yes" ]; then
    ENABLE_HID_PROFILE_D=1
else
    #hid device default is disable
    ENABLE_HID_PROFILE_D=0
fi
if [ "$SUPPORT_GATT" = "no" ]; then
    ENABLE_GATT_PROFILE=0
else
    ENABLE_GATT_PROFILE=1
fi
if [ "$SUPPORT_AVRCP" = "no" ]; then
    ENABLE_AVRCP_PROFILE=0
else
    ENABLE_AVRCP_PROFILE=1
fi
if [ "$SUPPORT_A2DP_SRC" = "no" ]; then
    ENABLE_A2DP_SRC=0
else
    ENABLE_A2DP_SRC=1
fi
if [ "$SUPPORT_A2DP_ADEV" = "no" ]; then
    ENABLE_A2DP_ADEV=0
else
    ENABLE_A2DP_ADEV=1
fi
if [ "$SUPPORT_A2DP_SINK" = "no" ]; then
    ENABLE_A2DP_SINK=0
else
    ENABLE_A2DP_SINK=1
fi
if [ "$SUPPORT_DISPATCH_A2DP_WITH_PLAYBACK" = "no" ]; then
    DISPATCH_A2DP_WITH_PLAYBACK=0
else
    DISPATCH_A2DP_WITH_PLAYBACK=1
fi
if [ "$SUPPORT_AAC" = "yes" ]; then
    ENABLE_AAC_CODEC=1
else
    ENABLE_AAC_CODEC=0
fi
if [ "$SUPPORT_HFP_CLIENT" = "no" ]; then
    ENABLE_HFP_CLIENT=0
else
    ENABLE_HFP_CLIENT=1
fi
if [ "$SUPPORT_BT_WIFI_RATIO_SETTING" = "no" ]; then
    ENABLE_BT_WIFI_RATIO_SETTING=0
else
    ENABLE_BT_WIFI_RATIO_SETTING=1
fi
MTK_BLE_GGL_SETUP_SUPPORT=0

CROSS_COMPILE=/mtkoss/gnuarm/hard_4.9.2-r116_armv7a-cros/x86_64/armv7a/usr/x86_64-pc-linux-gnu/armv7a-cros-linux-gnueabi/gcc-bin/4.9.x-google/armv7a-cros-linux-gnueabi-
TEMP_PATH=/tmp
MISC_PATH=/data/misc
ETC_PATH=/data/etc
if [ ! ${libdir} ]; then
    export libdir=/usr/lib
fi
PLATFORM_LIBRARY_PATH=${libdir}
##########for different platform only need modify above export variable##########

###generate c_mw_config.h
echo "/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 * the prior written permission of MediaTek inc. and/or its licensors, any
 * reproduction, modification, use or disclosure of MediaTek Software, and
 * information contained herein, in whole or in part, shall be strictly
 * prohibited.
 *
 * MediaTek Inc. (C) 2016-2017. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 * ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 * NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
 * RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 * INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
 * TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 * RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 * OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
 * SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
 * RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
 * ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
 * RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
 * MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 * CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation (\"MediaTek
 * Software\") have been modified by MediaTek Inc. All revisions are subject to
 * any receiver's applicable license agreements with MediaTek Inc.
 */
">$Bluetooth_Mw_Dir/inc/config/c_mw_config.h
echo "#ifndef _C_MW_CONFIG_H_
#define _C_MW_CONFIG_H_

#include \"u_bt_mw_types.h\"
//for bluetooth config
">>$Bluetooth_Mw_Dir/inc/config/c_mw_config.h

echo "#define ENABLE_SPP_PROFILE $ENABLE_SPP_PROFILE">>$Bluetooth_Mw_Dir/inc/config/c_mw_config.h
echo "#define ENABLE_HID_PROFILE_H $ENABLE_HID_PROFILE_H">>$Bluetooth_Mw_Dir/inc/config/c_mw_config.h
echo "#define ENABLE_HID_PROFILE_D $ENABLE_HID_PROFILE_D">>$Bluetooth_Mw_Dir/inc/config/c_mw_config.h
echo "#define ENABLE_GATT_PROFILE $ENABLE_GATT_PROFILE">>$Bluetooth_Mw_Dir/inc/config/c_mw_config.h
echo "#define ENABLE_AVRCP_PROFILE $ENABLE_AVRCP_PROFILE">>$Bluetooth_Mw_Dir/inc/config/c_mw_config.h
echo "#define ENABLE_A2DP_SRC $ENABLE_A2DP_SRC">>$Bluetooth_Mw_Dir/inc/config/c_mw_config.h
echo "#define ENABLE_A2DP_ADEV $ENABLE_A2DP_ADEV">>$Bluetooth_Mw_Dir/inc/config/c_mw_config.h
echo "#define ENABLE_A2DP_SINK $ENABLE_A2DP_SINK">>$Bluetooth_Mw_Dir/inc/config/c_mw_config.h
echo "#define DISPATCH_A2DP_WITH_PLAYBACK $DISPATCH_A2DP_WITH_PLAYBACK">>$Bluetooth_Mw_Dir/inc/config/c_mw_config.h
echo "#define ENABLE_AAC_CODEC $ENABLE_AAC_CODEC">>$Bluetooth_Mw_Dir/inc/config/c_mw_config.h
echo "#define ENABLE_HFP_CLIENT $ENABLE_HFP_CLIENT">>$Bluetooth_Mw_Dir/inc/config/c_mw_config.h
echo "#define ENABLE_BT_WIFI_RATIO_SETTING $ENABLE_BT_WIFI_RATIO_SETTING">>$Bluetooth_Mw_Dir/inc/config/c_mw_config.h
echo "#define MTK_BLE_GGL_SETUP_SUPPORT $MTK_BLE_GGL_SETUP_SUPPORT">>$Bluetooth_Mw_Dir/inc/config/c_mw_config.h

echo "
">>$Bluetooth_Mw_Dir/inc/config/c_mw_config.h
echo "#endif //_C_MW_CONFIG_H_">>$Bluetooth_Mw_Dir/inc/config/c_mw_config.h


###generate set_environment.sh
echo "#! /bin/bash
">set_environment.sh
echo "if [ \"\$BT_GET_CROSS_COMPILE\" = \"yes\" ]; then
    echo use special cross compile
elif [ -z \"\$CC\" ]; then
    export CC=${CROSS_COMPILE}gcc

    export CXX=${CROSS_COMPILE}g++
fi">>set_environment.sh
echo "if [ ! \$ENABLE_A2DP_SRC ]; then
    export ENABLE_A2DP_SRC=$ENABLE_A2DP_SRC
fi
###ENABLE_A2DP_ADEV:use audio device to get PCM data
if [ ! \$ENABLE_A2DP_ADEV ]; then
    export ENABLE_A2DP_ADEV=$ENABLE_A2DP_ADEV
fi
if [ ! \$ENABLE_A2DP_SINK ]; then
    export ENABLE_A2DP_SINK=$ENABLE_A2DP_SINK
fi
if [ ! \$ENABLE_AAC_CODEC ]; then
    export ENABLE_AAC_CODEC=$ENABLE_AAC_CODEC
fi">>set_environment.sh
echo "echo bluetooth use following toolchain">>set_environment.sh
echo "echo \$CC">>set_environment.sh
echo "echo \$CXX">>set_environment.sh
echo "###Script_Dir means currently build dir:script or script/common
if [ ! \$Script_Dir ]; then
    Script_Dir=\$(pwd)
fi
echo script dir: \$Script_Dir
if [ ! \$Bluetooth_Tool_Dir ]; then
    export Bluetooth_Tool_Dir=\${Script_Dir}/..
fi
if [ ! \$Bluetooth_Stack_Dir ]; then
    export Bluetooth_Stack_Dir=\${Bluetooth_Tool_Dir}/../../bt_stack/bluedroid_turnkey
fi
if [ ! \$Bluetooth_Mw_Dir ]; then
    export Bluetooth_Mw_Dir=\${Bluetooth_Tool_Dir}/../bluetooth_mw
fi
">>set_environment.sh
echo "echo \$MTK_BT_CHIP_ID">>set_environment.sh
echo "
if [ ! \$Bluetooth_Vendor_Lib_Dir ]; then
    if [ ! \"\$MTK_BT_CHIP_ID\" ]; then
        echo please input MTK_BT_CHIP_ID !!!
        exit 1
    elif [ \"\$MTK_BT_CHIP_ID\" = \"mt8167\" -o \"\$MTK_BT_CHIP_ID\" = \"mt6630\" ]; then
        if [ -d \${Bluetooth_Tool_Dir}/../vendor_lib_66xx ]; then
            export Bluetooth_Vendor_Lib_Dir=\${Bluetooth_Tool_Dir}/../vendor_lib_66xx
            export VENDOR_LIBRARY=vendorlib_6.sh
        elif [ -d \${Bluetooth_Tool_Dir}/../vendor_lib/mt66xx ]; then
            export Bluetooth_Vendor_Lib_Dir=\${Bluetooth_Tool_Dir}/../vendor_lib/mt66xx
            export VENDOR_LIBRARY=vendorlib_6.sh
        else
            echo vendor_lib_66xx not exist !!!
            exit 1
        fi
    else
        if [ -d \${Bluetooth_Tool_Dir}/../vendor_lib/mt76xx ]; then
            export Bluetooth_Vendor_Lib_Dir=\${Bluetooth_Tool_Dir}/../vendor_lib/mt76xx
        elif [ -d \${Bluetooth_Tool_Dir}/../vendor_lib ]; then
            export Bluetooth_Vendor_Lib_Dir=\${Bluetooth_Tool_Dir}/../vendor_lib
        else
            echo vendor_lib not exist !!!
            exit 1
        fi
        export VENDOR_LIBRARY=vendorlib_7.sh
    fi
fi

echo \$VENDOR_LIBRARY
if [ ! \$Bluetooth_Boots_Dir ]; then
    export Bluetooth_Boots_Dir=\${Bluetooth_Tool_Dir}/../boots
fi
if [ ! \$Bluetooth_Prebuilts_Dir ]; then
    export Bluetooth_Prebuilts_Dir=\${Bluetooth_Tool_Dir}/prebuilts
fi
if [ ! \$Bluedroid_Libs_Path ]; then
    export Bluedroid_Libs_Path=\${Bluetooth_Tool_Dir}/prebuilts/lib
fi

#platform related library:libz.so, libasound.so
if [ ! \$External_Libs_Path ]; then
    export External_Libs_Path=\${Bluetooth_Tool_Dir}/external_libs/platform
fi
">>set_environment.sh

echo "#project related path, integrator should care these path
#BT_Tmp_Path is used for temporay path like as /tmp
if [ ! \$BT_Tmp_Path ]; then
    export BT_Tmp_Path=$TEMP_PATH
fi
#BT_Misc_Path is used for misc path like as /misc
if [ ! \$BT_Misc_Path ]; then
    export BT_Misc_Path=$MISC_PATH
fi
#BT_Etc_Path is used for etc path like as /etc
if [ ! \$BT_Etc_Path ]; then
    export BT_Etc_Path=$ETC_PATH
fi

#stack config file path:bt_stack.conf,bt_did.conf
if [ ! \$Conf_Path ]; then
    export Conf_Path=\${BT_Misc_Path}/bluedroid
fi
#stack record file path.
if [ ! \$Cache_Path ]; then
    export Cache_Path=\${BT_Misc_Path}
fi
#mw record file path, should the same with stack record path.
if [ ! \$Storage_Path ]; then
    export Storage_Path=\${BT_Misc_Path}
fi
#system library file path:libbluetooth.default.so...
if [ ! \$Platform_Libs_Path ]; then
    export Platform_Libs_Path=$PLATFORM_LIBRARY_PATH
fi
##########for different platform only need modify above export variable##########

#if [ ! \$Rpc_Dbg_Flag ]; then
#    export Rpc_Dbg_Flag=-DBT_RPC_DBG_SERVER
#fi
">>set_environment.sh