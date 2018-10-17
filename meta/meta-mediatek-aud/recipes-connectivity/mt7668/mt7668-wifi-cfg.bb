DESCRIPTION = "Mediatek MT7668 WiFi Configuration File"
LICENSE = "GPLv2"
LIC_FILES_CHKSUM = "file://COPYING;md5=d7810fab7487fb0aad327b76f1be7cd7"
DEPENDS = "mt7668-wifi-drv"
MT7668_SRC = "${TOPDIR}/../src/connectivity"

inherit workonsrc
WORKONSRC = "${MT7668_SRC}/wlan_cfg/mt7668"

do_install() {
	echo ${D}
   
	if test "${COMBO_CHIP_ID}" = "mt7668"; then
		install -d ${D}/lib/firmware/
		install -m 0755 ${S}/TxPwrLimit_MT76x8.dat ${D}/lib/firmware
		install -m 0755 ${S}/wifi.cfg ${D}/lib/firmware
		install -d ${D}/data/misc/connectivity/
		install -m 0755 ${S}/EEPROM_MT7668.bin ${D}/data/misc/connectivity
	fi
}

FILES_${PN} += "/lib/firmware/"
FILES_${PN} += "/data/misc/connectivity/"


