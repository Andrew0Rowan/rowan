ifeq ($(MTK_BT_SUPPORT), yes)

LOCAL_PATH:= $(call my-dir)
include $(LOCAL_PATH)/script/Android.mk

####################################################################
# Stack
####################################################################
include $(CLEAR_VARS)
LOCAL_PREBUILT_LIBS += prebuilts/lib/libbluetooth.default.so
LOCAL_PREBUILT_LIBS += prebuilts/lib/libaudio.a2dp.default.so
include $(BUILD_MULTI_PREBUILT)

####################################################################
# MW
####################################################################
include $(CLEAR_VARS)
LOCAL_MODULE := libbt-mw
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_SRC_FILES_32 := prebuilts/lib/libbt-mw.so
LOCAL_MODULE_SUFFIX := .so
LOCAL_MULTILIB := 32
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := libbt-alsa-playback
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_SRC_FILES_32 := prebuilts/lib/libbt-alsa-playback.so
LOCAL_MODULE_SUFFIX := .so
LOCAL_MULTILIB := 32
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := btmw-test
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_CLASS := EXECUTABLES
LOCAL_SRC_FILES_32 := prebuilts/bin/btmw-test
LOCAL_MULTILIB := 32
include $(BUILD_PREBUILT)

####################################################################
# RPC
####################################################################
include $(CLEAR_VARS)
LOCAL_MODULE := libmtk_bt_service_client
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_SRC_FILES_32 := prebuilts/lib/libmtk_bt_service_client.so
LOCAL_MODULE_SUFFIX := .so
LOCAL_MULTILIB := 32
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := libmtk_bt_service_server
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_SRC_FILES_32 := prebuilts/lib/libmtk_bt_service_server.so
LOCAL_MODULE_SUFFIX := .so
LOCAL_MULTILIB := 32
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := btmw-rpc-test
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_CLASS := EXECUTABLES
LOCAL_SRC_FILES_32 := prebuilts/bin/btmw-rpc-test
LOCAL_MULTILIB := 32
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := btservice
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_CLASS := EXECUTABLES
LOCAL_SRC_FILES_32 := prebuilts/bin/btservice
LOCAL_MULTILIB := 32
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := libipcrpc
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_SRC_FILES_32 := prebuilts/lib/libipcrpc.so
LOCAL_MODULE_SUFFIX := .so
LOCAL_MULTILIB := 32
include $(BUILD_PREBUILT)

endif
