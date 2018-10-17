LOCAL_DIR := $(GET_LOCAL_DIR)

MODULE := $(LOCAL_DIR)

MODULE_SRCS += \
	$(LOCAL_DIR)/armv4-mont.S \
	$(LOCAL_DIR)/exponentiation.c\
	$(LOCAL_DIR)/rsa.c

include make/module.mk
