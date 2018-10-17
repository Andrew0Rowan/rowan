PREFIX = ../install
CROSS = arm-linux-
CROSS_COMPILER = $(CROSS:%-=%)
ROOT = ../install/$(CROSS_COMPILER)
DEP_LIBS = 
INSTALL = _install
CLEAN =
NAME = nvram_custom
CODEBASE=common
CONFIG = -I ./src \
	-I$(ROOT)/include/custom/$(LINUX_KERNEL)/$(CODEBASE)/cgen/cfgfileinc \
	-I$(ROOT)/include/custom/$(LINUX_KERNEL)/$(CODEBASE)/cgen/cfgdefault \
	-I$(ROOT)/include/custom/$(LINUX_KERNEL)/$(CODEBASE)/cgen/inc \
	-I$(ROOT)/include/custom/$(LINUX_KERNEL)/$(CODEBASE)/cgen
CFLAGS = -O2 -g -fPIC

ifeq ("$(COMBO_CHIP_ID)", "mt8167")
  CFLAGS += -DMTK_MT8167
else ifeq ("$(COMBO_CHIP_ID)", "mt6630")
  CFLAGS += -DMTK_MT6630
  CFLAGS += -D__MTK_MERGE_INTERFACE_SUPPORT__
endif

ifeq ("$(BOOTDEV_TYPE)", "emmc")
  CFLAGS += -DMTK_EMMC_SUPPORT
endif

LIB = lib$(NAME).a lib$(NAME).so

ALL = $(LIB)

include $(PREFIX)/Makefile.config
include $(PREFIX)/Makefile.template

_install:
	mkdir -p $(ROOT)/${base_libdir}
	cp lib$(NAME).so $(ROOT)/${base_libdir}
    
pack_rootfs:
# shared library
	mkdir -p $(PACK_TO)/${base_libdir}
	cp $(ROOT)/${base_libdir}/lib$(NAME).so $(PACK_TO)/${base_libdir}
	$(CROSS)strip $(PACK_TO)/${base_libdir}/lib$(NAME).so
	mkdir -p $(PACK_INITRAMFS_TO)/${base_libdir}
	cp $(ROOT)/${base_libdir}/lib$(NAME).so $(PACK_INITRAMFS_TO)/${base_libdir}
	$(CROSS)strip $(PACK_INITRAMFS_TO)/${base_libdir}/lib$(NAME).so
	ls -R $(PACK_TO)/${base_libdir}/lib$(NAME).so >> $(LIST)
	du -c -h $(PACK_TO)/${base_libdir}/lib$(NAME).so >> $(SIZE)



