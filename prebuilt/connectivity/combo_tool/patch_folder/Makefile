mtkcombo_package_arch=$(subst -,_,${PACKAGE_ARCH})

all:
install:
	find -name "*${mtkcombo_package_arch}*.rpm" | while read i; do ${bindir}/rpm2cpio $${i} | cpio -idmv ; done
ifeq (${COMBO_CHIP_ID_FLAG},mt6630)
		install -d ${DESTDIR}/lib/firmware
		install -m 0755 ./lib/firmware/mt6630_patch_e3_0_hdr.bin ${DESTDIR}/lib/firmware
		install -m 0755 ./lib/firmware/mt6630_patch_e3_1_hdr.bin ${DESTDIR}/lib/firmware
endif

ifeq (${COMBO_CHIP_ID_FLAG},mt8167)
		install -d ${DESTDIR}/lib/firmware
		install -m 0755 ./lib/firmware/ROMv2_lm_patch_1_0_hdr.bin ${DESTDIR}/lib/firmware
		install -m 0755 ./lib/firmware/ROMv2_lm_patch_1_1_hdr.bin ${DESTDIR}/lib/firmware
endif
