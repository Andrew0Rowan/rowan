LOCAL_DIR := $(GET_LOCAL_DIR)
MODULE := $(LOCAL_DIR)

MODULE_SRCS += \
    $(LOCAL_DIR)/msdc.c \
    $(LOCAL_DIR)/mmc_core.c \

MODULE_DEPS += \
    lib/bio \
    lib/partition \

include make/module.mk
