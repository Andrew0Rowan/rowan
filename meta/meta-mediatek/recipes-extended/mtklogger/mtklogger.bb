DESCRIPTION = "mtklogger is a tool to collect logs when exception"
LICENSE = "MediaTekProprietary"
WORKONSRC = "${TOPDIR}/../src/extended/mtklogger"
LIC_FILES_CHKSUM = "file://LICENSE;md5=e1696b147d49d491bcb4da1a57173fff"

inherit workonsrc
inherit pkgconfig

do_compile() {
	oe_runmake \
		CFLAGS="${CFLAGS}" \
		LDFLAGS="${LDFLAGS}"
}

do_install() {
	oe_runmake \
		PREFIX="${prefix}" DESTDIR="${D}" PACKAGE_ARCH="${PACKAGE_ARCH}" install
}


inherit systemd
SYSTEMD_PACKAGES = "${PN}"
SYSTEMD_SERVICE_${PN} = "mtklogger.service"
FILES_${PN} += "${systemd_unitdir}/system/mtklogger.service"

do_install_append() {
    if ${@bb.utils.contains('DISTRO_FEATURES','systemd','true','false',d)}; then
	install -d ${D}${systemd_unitdir}/system/
	install -m 0644 ${B}/lib/systemd/system/mtklogger.service ${D}${systemd_unitdir}/system
    fi
}

INSANE_SKIP_${PN} += "already-stripped"
