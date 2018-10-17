CRIPTION = "Combo FW"
LICENSE = "MediaTekProprietary"
LIC_FILES_CHKSUM = "file://LICENSE;md5=e1696b147d49d491bcb4da1a57173fff"
INSANE_SKIP_${PN} += "installed-vs-shipped"
FILES_${PN} = "/lib/firmware/mt6630_patch_e3_0_hdr.bin /lib/firmware/mt6630_patch_e3_1_hdr.bin"

inherit workonsrc
WORKONSRC = "${TOPDIR}/../src/connectivity/combo_tool/patch_folder"

do_install () {
        install -d ${D}/lib/firmware
        install -m 0755 ${S}/mt6630_patch_e3_0_hdr.bin ${D}/lib/firmware
        install -m 0755 ${S}/mt6630_patch_e3_1_hdr.bin ${D}/lib/firmware
}
