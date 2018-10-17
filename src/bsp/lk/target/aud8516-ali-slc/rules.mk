LOCAL_DIR := $(GET_LOCAL_DIR)
MODULE := ${LOCAL_DIR}

GLOBAL_INCLUDES += \
    $(LOCAL_DIR)/include

MODULE_SRCS += \
    $(LOCAL_DIR)/target.c

PLATFORM := mt8516

TZ_PART_NAME := TEE1
RECOVERY_TZ_PART_NAME = TEE2
DTBO_PART_NAME := DTBO
BOOT_PART_NAME := BOOTIMG1
RECOVERY_BOOT_PART_NAME = BOOTIMG2

ifeq ($(WITH_KERNEL_VM),1)
# The physical dram address is 0x40000000 and its virtual address 0xffff00000d600000
# The SCRATCH_ADDR virtual address would be 0xffff00000d600000 + 0x4000000 = 0xffff00000f400000
# The BL33_ADDR virtual address would be 0xffff00000d600000 + 0x1e00000 = 0xffff00000f400000
SCRATCH_ADDR		:= 0xffff000011600000UL
BL33_ADDR               := 0xffff00000f400000UL
NAND_BUF_ADDR           := 0xffff00001b900000UL
else
SCRATCH_ADDR            := 0x44000000
BL33_ADDR               := 0x41e00000
NAND_BUF_ADDR           := 0x4e300000
endif
SCRATCH_SIZE		:= 0x04000000 # 64MB

MAX_TEE_DRAM_SIZE	:= 0x04000000 # reduce to 64M since DRAM in mt2635 only 256M
MAX_KERNEL_SIZE		:= 0x02000000 # 32M
MAX_DTB_SIZE		:= 0x00200000 # 2M
MAX_LZ4_BUF_SIZE	:= 0x00100000 # 1M
MAX_NAND_BUF_SIZE	:= 0x00100000 # 1M

# SCRATCH_ADDR + SCRATCH_SIZE for upgrade
# SCRATCH_ADDR + MAX_TEE_DRAM_SIZE + MAX_KERNEL_SIZE
# + MAX_DTB_SIZE + MAX_LZ4_BUF_SIZE for boot
# NAND_BUF_ADDR + MAX_NAND_BUF_SIZE for nand reserved dram buffer

GLOBAL_CFLAGS +=  -DTZ_PART_NAME=\"$(TZ_PART_NAME)\" \
                  -DDTBO_PART_NAME=\"$(DTBO_PART_NAME)\" \
                  -DBOOT_PART_NAME=\"$(BOOT_PART_NAME)\" \
                  -DRECOVERY_BOOT_PART_NAME=\"$(RECOVERY_BOOT_PART_NAME)\" \
                  -DRECOVERY_TZ_PART_NAME=\"$(RECOVERY_TZ_PART_NAME)\" \

GLOBAL_DEFINES += SCRATCH_SIZE=$(SCRATCH_SIZE) \
                  SCRATCH_ADDR=$(SCRATCH_ADDR) \
                  BL33_ADDR=$(BL33_ADDR) \
                  MAX_TEE_DRAM_SIZE=$(MAX_TEE_DRAM_SIZE) \
                  MAX_KERNEL_SIZE=$(MAX_KERNEL_SIZE) \
                  MAX_DTB_SIZE=$(MAX_DTB_SIZE) \
                  MAX_LZ4_BUF_SIZE=$(MAX_LZ4_BUF_SIZE) \
                  NAND_BUF_ADDR=$(NAND_BUF_ADDR) \
                  MAX_NAND_BUF_SIZE=$(MAX_NAND_BUF_SIZE)

MODULE_DEPS += \
    platform/$(PLATFORM)/drivers/nand \

include make/module.mk
