LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_SRC_FILES := \
	conf/btif_config_mtk_util.c \
	conf/mdroid_stack_config.c \
	interop/interop_mtk.c

LOCAL_MODULE := libbt-mtk_cust
LOCAL_MODULE_CLASS := STATIC_LIBRARIES
LOCAL_MODULE_TAGS := optional

LOCAL_C_INCLUDES := \
	$(LOCAL_PATH)/include \
	$(LOCAL_PATH)/../include \
	$(LOCAL_PATH)/../device/include \
	$(LOCAL_PATH)/../btcore/include \
	$(LOCAL_PATH)/../stack/include \
	$(LOCAL_PATH)/../osi/include \
	$(LOCAL_PATH)/../btif/include \
	$(LOCAL_PATH)/../ \
	$(bluetooth_C_INCLUDES)

LOCAL_CFLAGS += $(bluetooth_CFLAGS) -DMTK_STACK_CONFIG_BL
LOCAL_CONLYFLAGS += $(bluetooth_CONLYFLAGS)
LOCAL_CPPFLAGS += $(bluetooth_CPPFLAGS)

include $(BUILD_STATIC_LIBRARY)
