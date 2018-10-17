# Copyright Statement:
#
# This software/firmware and related documentation ("MediaTek Software") are
# protected under relevant copyright laws. The information contained herein is
# confidential and proprietary to MediaTek Inc. and/or its licensors. Without
# the prior written permission of MediaTek inc. and/or its licensors, any
# reproduction, modification, use or disclosure of MediaTek Software, and
# information contained herein, in whole or in part, shall be strictly
# prohibited.

# MediaTek Inc. (C) 2010. All rights reserved.
#
# BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
# THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
# RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
# ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
# WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
# WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
# NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
# RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
# INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
# TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
# RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
# OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
# SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
# RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
# STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
# ENTIRE ANDCUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
# RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
# MEDIATEK SOFTWARE AT ISSUE,OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
# CHARGE PAID BY RECEIVER TOMEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
#
# The following software/firmware and/or related documentation ("MediaTek
# Software") have been modified by MediaTek Inc. All revisions are subject to
# any receiver's applicable license agreements with MediaTek Inc.

ifeq ($(MTK_BT_SUPPORT), yes)
###########################################################################
# MTK BT SOLUTION
###########################################################################

BLUETOOTH_EXTERNAL_INCLUDE_PATH := vendor/mediatek/proprietary/external/bluetooth/tool/vendor_libs/mtk/bluedroid/external
BLUETOOTH_MEDIATEK_INCLUDE_PATH := vendor/mediatek/proprietary/external/bluetooth/bluedroid/mediatek/include
BLUETOOTH_SCRIPT_PATH := vendor/mediatek/proprietary/external/bluetooth/tool/script
MEDIATEK_TO_PROJECT_PATH := ../../../../../../../..
SCRIPT_TO_PROJECT_PATH := ../../../../../../..

ifeq ($(TARGET_PRODUCT), full_aud8516p1_linux)
CUSTOM_INCLUDE_PATH := device/mediatek/aud8516p1_linux/custom/cgen
endif

ifeq ($(TARGET_PRODUCT), full_aud8516p1_slc_linux)
CUSTOM_INCLUDE_PATH := device/mediatek/aud8516p1_slc_linux/custom/cgen
endif

ifeq ($(TARGET_PRODUCT), full_aud8516p1_b52_linux)
CUSTOM_INCLUDE_PATH := device/mediatek/aud8516p1_b52_linux/custom/cgen
endif

ifeq ($(TARGET_PRODUCT), full_aud8521p1_linux)
CUSTOM_INCLUDE_PATH := device/mediatek/aud8521p1_linux/custom/cgen
endif

ifeq ($(TARGET_PRODUCT), full_aud8521m0_linux)
CUSTOM_INCLUDE_PATH := device/mediatek/aud8521m0_linux/custom/cgen
endif

ifeq ($(TARGET_PRODUCT), full_aud8516p1_sdk_linux)
CUSTOM_INCLUDE_PATH := device/mediatek/aud8516p1_sdk_linux/custom/cgen
endif

ifeq ($(TARGET_PRODUCT), full_aud8516p1_slc_sdk_linux)
CUSTOM_INCLUDE_PATH := device/mediatek/aud8516p1_slc_sdk_linux/custom/cgen
endif

$(info $(shell cp $(CUSTOM_INCLUDE_PATH)/cfgdefault/CFG_BT_Default.h $(BLUETOOTH_EXTERNAL_INCLUDE_PATH)))
$(info $(shell cp $(CUSTOM_INCLUDE_PATH)/cfgfileinc/CFG_BT_File.h $(BLUETOOTH_EXTERNAL_INCLUDE_PATH)))
$(info $(shell cp $(CUSTOM_INCLUDE_PATH)/inc/CFG_file_lid.h $(BLUETOOTH_EXTERNAL_INCLUDE_PATH)))
$(warning product $(CUSTOM_INCLUDE_PATH))

ifeq ($(LINUX_KERNEL_VERSION), kernel-3.10)
$(info $(shell cd $(BLUETOOTH_MEDIATEK_INCLUDE_PATH) && cp -f uhid_3_10.h uhid.h && cd $(MEDIATEK_TO_PROJECT_PATH)))
else ifeq ($(LINUX_KERNEL_VERSION), kernel-3.18)
$(info $(shell cd $(BLUETOOTH_MEDIATEK_INCLUDE_PATH) && cp -f uhid_3_18.h uhid.h && cd $(MEDIATEK_TO_PROJECT_PATH)))
else ifeq ($(LINUX_KERNEL_VERSION), kernel-4.4)
$(info $(shell cd $(BLUETOOTH_MEDIATEK_INCLUDE_PATH) && cp -f uhid_4_4.h uhid.h && cd $(MEDIATEK_TO_PROJECT_PATH)))
else
$(info $(shell cd $(BLUETOOTH_MEDIATEK_INCLUDE_PATH) && cp -f uhid_3_18.h uhid.h && cd $(MEDIATEK_TO_PROJECT_PATH)))
endif

$(info $(shell cd $(BLUETOOTH_SCRIPT_PATH) && sh clean_all_rpc.sh && sh build_all_rpc.sh && cd $(SCRIPT_TO_PROJECT_PATH)))

endif