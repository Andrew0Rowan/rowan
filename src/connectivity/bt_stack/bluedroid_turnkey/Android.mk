LOCAL_PATH := $(call my-dir)

# Setup Bluetooth local make variables for handling configuration
ifneq ($(BOARD_BLUETOOTH_BDROID_BUILDCFG_INCLUDE_DIR),)
  bluetooth_C_INCLUDES := $(BOARD_BLUETOOTH_BDROID_BUILDCFG_INCLUDE_DIR)
  bluetooth_CFLAGS += -DHAS_BDROID_BUILDCFG
else
  bluetooth_C_INCLUDES :=
  bluetooth_CFLAGS += -DHAS_NO_BDROID_BUILDCFG
endif

ifneq ($(BOARD_BLUETOOTH_BDROID_HCILP_INCLUDED),)
  bluetooth_CFLAGS += -DHCILP_INCLUDED=$(BOARD_BLUETOOTH_BDROID_HCILP_INCLUDED)
endif

ifneq ($(TARGET_BUILD_VARIANT),user)
bluetooth_CFLAGS += -DBLUEDROID_DEBUG
endif

bluetooth_CFLAGS += -DEXPORT_SYMBOL="__attribute__((visibility(\"default\")))"

#to support AVRCP 1.5 browsing features.
#bluetooth_CFLAGS += -DMTK_BT_AVRCP_TG_15_BW

#will not callback browsing conn status in order to fit old HAL layer
#bluetooth_CFLAGS += -DMTK_BT_AVRCP_TG_15_BW_NO_CONN_CB

# Enable MTK patch & feature
bluetooth_C_INCLUDES += $(LOCAL_PATH)/mediatek/include
bluetooth_CFLAGS += -DMTK_BLUEDROID_PATCH

#
# Common C/C++ compiler flags.
#
# -Wno-gnu-variable-sized-type-not-at-end is needed, because struct BT_HDR
#  is defined as a variable-size header in a struct.
# -Wno-typedef-redefinition is needed because of the way the struct typedef
#  is done in osi/include header files. This issue can be obsoleted by
#  switching to C11 or C++.
# -Wno-unused-parameter is needed, because there are too many unused
#  parameters in all the code.
#
bluetooth_CFLAGS += \
  -fvisibility=hidden \
  -Wall \
  -Wextra \
  -Werror \
  -Wno-gnu-variable-sized-type-not-at-end \
  -Wno-typedef-redefinition \
  -Wno-unused-parameter \
  -UNDEBUG \
  -DLOG_NDEBUG=1

bluetooth_CONLYFLAGS += -std=c99
bluetooth_CPPFLAGS :=

ifeq ($(MTK_BT_BLUEDROID_A2DP_APTX), TRUE)
mdroid_CFLAGS += \
  -DMTK_BT_A2DP_SRC_APTX_CODEC

CSR_VENDOR_PATH = $(TOP)/vendor/mediatek/open/csr/aptX

$(shell mkdir -p $(TARGET_OUT_SHARED_LIBRARIES)/../../obj/lib)
$(shell mkdir -p $(TARGET_OUT_SHARED_LIBRARIES)/../../obj/SHARED_LIBRARIES/libbt-aptX-ARM-4.2.2_intermediates/export_includes)
$(shell mkdir -p $(TARGET_OUT_SHARED_LIBRARIES))

$(shell cp $(CSR_VENDOR_PATH)/libbt-aptX-ARM-4.2.2.so $(TARGET_OUT_SHARED_LIBRARIES)/libbt-aptX-ARM-4.2.2.so)
$(shell cp $(CSR_VENDOR_PATH)/libbt-aptX-ARM-4.2.2.so $(TARGET_OUT_SHARED_LIBRARIES)/../../obj/lib/libbt-aptX-ARM-4.2.2.so)
$(shell cp $(CSR_VENDOR_PATH)/libbt-aptX-ARM-4.2.2.so $(TARGET_OUT_SHARED_LIBRARIES)/../../obj/SHARED_LIBRARIES/libbt-aptX-ARM-4.2.2_intermediates/export_includes/libbt-aptX-ARM-4.2.2.so)

LOCAL_MODULE := bt-aptX
LOCAL_MODULE_PATH := $(TARGET_OUT_SHARED_LIBRARIES)
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_SRC_FILES := libbt-aptX-ARM-4.2.2.so
include $(PREBUILT_SHARED_LIBRARY)
endif #MTK_BT_BLUEDROID_A2DP_APTX


include $(call all-subdir-makefiles)

# Cleanup our locals
bluetooth_C_INCLUDES :=
bluetooth_CFLAGS :=
bluetooth_CONLYFLAGS :=
bluetooth_CPPFLAGS :=
