DESCRIPTION = "Mediatek MT7668 WiFi Test Tool"
LICENSE = "GPLv2"
LIC_FILES_CHKSUM = "file://COPYING;md5=d7810fab7487fb0aad327b76f1be7cd7"

inherit workonsrc
WORKONSRC = "${TOPDIR}/../src/connectivity/wlan_tool/wifi_test_tool/wifitesttool"
MODULE_NAME = "wifitest"

do_compile() {
	if test "${COMBO_CHIP_ID}" = "mt7668"; then
		echo mt7668 wifi tool start compile
		echo ${WORKONSRC}
		echo ${S}

		cd ${S} && oe_runmake CFLAGS="-DCONFIG_YOCTO_EEPROM_PATH=1"
		echo mt7668 wifi tool end compile
	fi
}

do_install() {
	echo ${D}
   
	if test "${COMBO_CHIP_ID}" = "mt7668"; then
		install -d ${D}/usr/sbin/
		install -m 0755 ${S}/${MODULE_NAME} ${D}/usr/sbin
	fi
}

FILES_${PN} += "/usr/sbin"
FILES_${PN}-dev = ""