LOCAL_DIR := $(GET_LOCAL_DIR)

MODULE := $(LOCAL_DIR)

MODULE_SRCS += \
	$(LOCAL_DIR)/sha256_neon_glue.c \
	$(LOCAL_DIR)/sha256-armv8.S\
	$(LOCAL_DIR)/sha256-core.S

include make/module.mk
