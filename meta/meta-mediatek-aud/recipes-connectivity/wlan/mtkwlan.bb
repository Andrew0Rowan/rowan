DESCRIPTION = "Wlan FW"
LICENSE = "MediaTekProprietary"
LIC_FILES_CHKSUM = "file://LICENSE;md5=e1696b147d49d491bcb4da1a57173fff"
INSANE_SKIP_${PN} += "installed-vs-shipped"
FILES_${PN} += "/lib/firmware/WIFI_RAM_CODE_MT6630"
FILES_${PN} += "/vendor/firmware/WIFI_RAM_CODE_8167"
FILES_${PN} += "/etc/hostapd_mtk.conf"
FILES_${PN} += "/etc/udhcpd.conf"
FILES_${PN} += "/bin/runap.sh"
FILES_${PN} += "/usr/bin/wlan_connect_ap.sh"

inherit workonsrc
WORKONSRC = "${TOPDIR}/../src/connectivity/wlan"

do_install_append () {
if [ ! -e ${WORKONSRC}/Android.mk ]; then
    oe_runmake \
    PREFIX="${prefix}" DESTDIR="${D}" PACKAGE_ARCH="${PACKAGE_ARCH}" COMBO_CHIP_ID_FLAG="${COMBO_CHIP_ID}" install
else
	if test "${COMBO_CHIP_ID}" = "mt6630"; then
		install -d ${D}/lib/firmware
		install -m 0755 ${S}/firmware/audio/WIFI_RAM_CODE_MT6630 ${D}/lib/firmware
	fi

		install -d ${D}/etc/
        install -m 0644 ${S}/hostapd_conf/hostapd.conf.in ${D}/etc/hostapd_mtk.conf
        install -m 0644 ${S}/udhcpd_conf/udhcpd.conf ${D}/etc/udhcpd.conf
        install -d ${D}/bin/
        install -m 0755 ${S}/hostapd_conf/runap.sh ${D}/bin/runap.sh

		install -d ${D}/usr/bin
        install -m 0644 ${S}/connection_script/wlan_connect_ap.sh ${D}/usr/bin
		
	if test "${COMBO_CHIP_ID}" = "mt8167"; then
		install -d ${D}/vendor/firmware
		install -m 0755 ${S}/firmware/WIFI_RAM_CODE_8167 ${D}/vendor/firmware
	fi
fi
}
