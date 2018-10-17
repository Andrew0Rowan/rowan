LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)

CAL_CFLAGS := -Wall-ansi

LOCAL_SRC_FILES := common.c main.c

#LOCAL_STATIC_LIBRARIES := libc
#LOCAL_FORCE_STATIC_EXECUTABLE := true

LOCAL_MODULE:= picus
LOCAL_MODULE_TAGS := optional

include $(BUILD_EXECUTABLE)

