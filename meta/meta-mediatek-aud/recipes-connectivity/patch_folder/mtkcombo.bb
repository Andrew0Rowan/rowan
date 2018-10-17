CRIPTION = "Combo FW"
LICENSE = "MediaTekProprietary"
LIC_FILES_CHKSUM = "file://LICENSE;md5=e1696b147d49d491bcb4da1a57173fff"
INSANE_SKIP_${PN} += "installed-vs-shipped"
FILES_${PN} += "/lib/firmware/mt6630_patch_e3_0_hdr.bin /lib/firmware/mt6630_patch_e3_1_hdr.bin"
FILES_${PN} += "/lib/firmware/ROMv2_lm_patch_1_0_hdr.bin /lib/firmware/ROMv2_lm_patch_1_1_hdr.bin"

inherit workonsrc
WORKONSRC = "${TOPDIR}/../src/connectivity/combo_tool/patch_folder"

do_install () {
if [ ! -e ${WORKONSRC}/ROMv2_lm_patch_1_0_hdr.bin ]; then
    oe_runmake \
    PREFIX="${prefix}" DESTDIR="${D}" PACKAGE_ARCH="${PACKAGE_ARCH}" COMBO_CHIP_ID_FLAG="${COMBO_CHIP_ID}" install
else
	if test "${COMBO_CHIP_ID}" = "mt6630"; then
		install -d ${D}/lib/firmware
		install -m 0755 ${S}/mt6630_patch_e3_0_hdr.bin ${D}/lib/firmware
        install -m 0755 ${S}/mt6630_patch_e3_1_hdr.bin ${D}/lib/firmware
	fi
	
	if test "${COMBO_CHIP_ID}" = "mt8167"; then
		install -d ${D}/lib/firmware
        install -m 0755 ${S}/ROMv2_lm_patch_1_0_hdr.bin ${D}/lib/firmware
        install -m 0755 ${S}/ROMv2_lm_patch_1_1_hdr.bin ${D}/lib/firmware
	fi
fi
}
