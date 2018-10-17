LOCAL_DIR := $(GET_LOCAL_DIR)
MODULE := $(LOCAL_DIR)

MODULE_SRCS += \
    $(LOCAL_DIR)/mtk_ecc_hal.c \
    $(LOCAL_DIR)/mtk_nfi_hal.c \
    $(LOCAL_DIR)/mtk_nand_device.c \
    $(LOCAL_DIR)/mtk_nand_nal.c \
    $(LOCAL_DIR)/mtk_nand_bdev.c \

MODULE_DEPS += \
    lib/bio \
    lib/partition \

include make/module.mk
