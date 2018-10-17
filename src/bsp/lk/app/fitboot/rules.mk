LOCAL_DIR := $(GET_LOCAL_DIR)

MODULE := $(LOCAL_DIR)
MODULE_BUILDDIR := $(call TOBUILDDIR,$(MODULE))
MODULE_INCLUDES += $(MODULE_BUILDDIR)/../../../include

MODULE_DEPS += \
    lib/bio \
    lib/lz4 \
    lib/fdt \
    lib/sha256_neon \
    lib/rsa2048

MODULE_SRCS += \
	$(LOCAL_DIR)/fitboot.c \
	$(LOCAL_DIR)/fit.c \
	$(LOCAL_DIR)/fastboot.c \
	$(LOCAL_DIR)/dl_commands.c \
	$(LOCAL_DIR)/ext_boot.c \
	$(LOCAL_DIR)/image_verify.c \
 
include make/module.mk
