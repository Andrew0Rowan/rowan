LOCAL_DIR := $(GET_LOCAL_DIR)

MODULE := $(LOCAL_DIR)

ARCH ?= arm64
ARM_CPU ?= cortex-a53
WITH_SMP ?= 0
WITH_KERNEL_VM ?= 1
WITH_MTK_PMIC_WRAP_AND_PMIC ?= 0

LK_HEAP_IMPLEMENTATION ?= miniheap

GLOBAL_INCLUDES += -I$(LK_TOP_DIR)/include \

MODULE_SRCS += \
    $(LOCAL_DIR)/platform.c \
    $(LOCAL_DIR)/debug.c \
    $(LOCAL_DIR)/interrupts.c \
    $(LOCAL_DIR)/timer.c \

ifeq ($(WITH_KERNEL_VM),1)
KERNEL_ASPACE_BASE ?= 0xffff000000200000
KERNEL_ASPACE_SIZE ?= 0x00000000f0000000
MMU_IDENT_SIZE_SHIFT ?= 32
endif
MEMBASE ?= 0x200000
KERNEL_LOAD_OFFSET ?= 0x1000
MEMSIZE ?= 0x00040000   # 256K
MACH_TYPE := 8516

MODULE_DEPS += \
    dev/interrupt/arm_gic \
    dev/timer/arm_generic \
    lib/bio \
    lib/partition \
    lib/fdt \

ifeq ($(WITH_KERNEL_VM),1)
GLOBAL_DEFINES += MMU_IDENT_SIZE_SHIFT=$(MMU_IDENT_SIZE_SHIFT)
endif

ifeq ($(WITH_MTK_PMIC_WRAP_AND_PMIC),1)
GLOBAL_DEFINES += WITH_MTK_PMIC_WRAP_AND_PMIC=$(WITH_MTK_PMIC_WRAP_AND_PMIC)
endif

GLOBAL_DEFINES += \
    MEMBASE=$(MEMBASE) \
    MEMSIZE=$(MEMSIZE) \
    RAMBASE=$(RAMBASE) \
    MACH_TYPE=$(MACH_TYPE) \
    PLATFORM_SUPPORTS_PANIC_SHELL=1 \
    WITH_NO_FP=1 \

GLOBAL_CFLAGS += -fno-stack-protector \

LINKER_SCRIPT += \
    $(BUILDDIR)/system-onesegment.ld

include make/module.mk $(LOCAL_DIR)/drivers/rules.mk
