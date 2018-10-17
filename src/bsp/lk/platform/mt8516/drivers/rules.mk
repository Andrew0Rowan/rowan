LOCAL_DIR := $(GET_LOCAL_DIR)
MODULE := $(LOCAL_DIR)

MT_DRAM_OBJ_DIR := $(BUILDDIR)/platform/mt8516/drivers
DRAM_DIR := $(LOCAL_DIR)/dram
DRAMK_OBJS := $(patsubst $(DRAM_DIR)/%.c,$(MT_DRAM_OBJ_DIR)/dram/%.o,$(wildcard $(DRAM_DIR)/*.c))

$(warning "DRAMK_OBJS is $(DRAMK_OBJS)")

MODULE_SRCS += \
    $(LOCAL_DIR)/i2c/mtk_i2c.c \
    $(LOCAL_DIR)/uart/uart.c \
    $(LOCAL_DIR)/wdt/mtk_wdt.c \
    $(LOCAL_DIR)/usb/mt_usb.c \
    $(LOCAL_DIR)/key/mtk_key.c \
    $(LOCAL_DIR)/pll/pll.c \
    $(LOCAL_DIR)/spm/spm_mtcmos.c \
    $(LOCAL_DIR)/led/mtk_led.c \
	$(LOCAL_DIR)/led/fl3236_led.c

ifeq ($(WITH_MTK_PMIC_WRAP_AND_PMIC), 1)
MODULE_SRCS += $(LOCAL_DIR)/pwrap/pmic_wrap.c
endif

MODULE_DEPS += \
    lib/bio \
    lib/partition \
    lib/fdt \
    lib/cksum \

include make/module.mk

