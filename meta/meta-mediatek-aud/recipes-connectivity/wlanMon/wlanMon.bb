DESCRIPTION = "Wi-Fi Middleware share library"
LICENSE = "MediaTekProprietary"
LIC_FILES_CHKSUM = "file://LICENSE;md5=d7810fab7487fb0aad327b76f1be7cd7"
#DEPENDS += "libpthread libipcd"

inherit workonsrc

WORKONSRC = "${TOPDIR}/../src/apps/aud-base/library"
S="${WORKDIR}"

do_compile() {
	cd ${S}/wlanMon
	oe_runmake
}

do_install() {
	install -d ${D}${libdir}
	install -m 644 ${S}/wlanMon/libwlanMon.so ${D}${libdir}
}

FILES_${PN} = "${libdir}/*"
FILES_${PN}-dev = ""

INSANE_SKIP_${PN} += "ldflags"
