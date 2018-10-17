DESCRIPTION = "Mediatek MT7668 WiFi Firmware Binary"
LICENSE = "GPLv2"
LIC_FILES_CHKSUM = "file://COPYING;md5=d7810fab7487fb0aad327b76f1be7cd7"
DEPENDS = "mt7668-wifi-drv"
MT7668_SRC = "${TOPDIR}/../src/connectivity"

inherit workonsrc
WORKONSRC = "${MT7668_SRC}/bin/mt7668"

do_install() {
	echo ${D}
   
	if test "${COMBO_CHIP_ID}" = "mt7668"; then
		install -d ${D}/lib/firmware/
		install -m 0755 ${S}/WIFI_RAM_CODE_MT7668.bin ${D}/lib/firmware
		install -m 0755 ${S}/mt7668_patch_e2_hdr.bin ${D}/lib/firmware
		install -m 0755 ${S}/WIFI_RAM_CODE2_SDIO_MT7668.bin ${D}/lib/firmware
	fi
}

FILES_${PN} += "/lib/firmware/"
FILES_${PN} += "/data/misc/connectivity/"
