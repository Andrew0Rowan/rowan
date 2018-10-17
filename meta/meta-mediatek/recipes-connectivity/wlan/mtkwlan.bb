DESCRIPTION = "Wlan FW"
LICENSE = "MediaTekProprietary"
LIC_FILES_CHKSUM = "file://LICENSE;md5=e1696b147d49d491bcb4da1a57173fff"
INSANE_SKIP_${PN} += "installed-vs-shipped"
FILES_${PN} = "/lib/firmware/WIFI_RAM_CODE_MT6630"
FILES_${PN} += "/etc/hostapd_mtk.conf"
FILES_${PN} += "/etc/dhcpd.conf"
FILES_${PN} += "/bin/runap.sh"

inherit workonsrc
WORKONSRC = "${TOPDIR}/../src/connectivity/wlan"

do_install_append () {
	install -d ${D}/lib/firmware
	install -m 0755 ${S}/firmware/WIFI_RAM_CODE_MT6630 ${D}/lib/firmware
        install -d ${D}/etc/
        install -m 0644 ${S}/hostapd_conf/hostapd.conf.in ${D}/etc/hostapd_mtk.conf
        install -m 0644 ${S}/dhcpd_conf/dhcpd.sample.conf ${D}/etc/dhcpd.conf
        install -d ${D}/bin/
        install -m 0755 ${S}/hostapd_conf/runap.sh ${D}/bin/runap.sh
}
