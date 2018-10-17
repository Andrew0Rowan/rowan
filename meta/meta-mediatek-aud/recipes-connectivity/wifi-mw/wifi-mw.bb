DESCRIPTION = "Wi-Fi Middleware share library"
LICENSE = "MediaTekProprietary"
LIC_FILES_CHKSUM = "file://LICENSE;md5=d7810fab7487fb0aad327b76f1be7cd7"
#DEPENDS += "libpthread libipcd"

inherit workonsrc

WORKONSRC = "${TOPDIR}/../src/apps/aud-base/"
S="${WORKDIR}"

do_compile() {
	cd ${S}/library/wifi_mw/wifi
	oe_runmake
}

do_install() {
	install -d ${D}${libdir}
	install -m 644 ${S}/library/wifi_mw/wifi/wlanCtrl/libwlanCtrl.so ${D}${libdir}
  	install -m 644 ${S}/library/wifi_mw/wifi/wlanMtk/libwlanMtk.so ${D}${libdir}
}

FILES_${PN} = "${libdir}/*"
FILES_${PN}-dev = ""

INSANE_SKIP_${PN} += "ldflags"
