DESCRIPTION = "Wi-Fi Probe Function library"
LICENSE = "MediaTekProprietary"
LIC_FILES_CHKSUM = "file://LICENSE;md5=d7810fab7487fb0aad327b76f1be7cd7"
#DEPENDS += "libpthread"

inherit workonsrc
inherit systemd

SYSTEMD_PACKAGES = "${PN}"

WORKONSRC = "${TOPDIR}/../src/apps/aud-base/library"
S="${WORKDIR}"

do_compile() {
	cd ${S}/wlan_probe
	oe_runmake
}

do_install() {
	install -d ${D}${libdir}
	install -m 644 ${S}/wlan_probe/wlan_probe_lib/libwlanProbe.so ${D}${libdir}
	install -d ${D}${sbindir}
	install -m 0755 ${S}/wlan_probe/wlan_probe_demo/wlan_probe_demo ${D}${sbindir}
}

FILES_${PN} += "${libdir}/*"
FILES_${PN} += "${sbindir}"
FILES_${PN}-dev = ""

INSANE_SKIP_${PN} += "ldflags"
INSANE_SKIP_${PN} +="installed-vs-shipped"
