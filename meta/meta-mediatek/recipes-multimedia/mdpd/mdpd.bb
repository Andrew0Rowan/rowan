DESCRIPTION = "MediaTek MDP daemon"
LICENSE = "MediaTekProprietary"
LIC_FILES_CHKSUM = "file://NOTICE;md5=e1696b147d49d491bcb4da1a57173fff"

inherit workonsrc
WORKONSRC = "${TOPDIR}/../src/multimedia/mdpd"

DEPENDS = "fuse"
DEPENDS += "libmdppq"
RDEPENDS_${PN} = "libgcc"
RDEPENDS_${PN} += "libstdc++"
RDEPENDS_${PN} += "libmdppq"

inherit pkgconfig

do_configure() {
        :
}

do_compile() {
	oe_runmake \
		PACKAGE_ARCH="${PACKAGE_ARCH}"  \
		CFLAGS="`pkg-config --cflags fuse` ${CFLAGS}" \
		LDFLAGS="`pkg-config --libs fuse` ${LDFLAGS}"
}

do_install() {
	oe_runmake \
		PREFIX="${prefix}" DESTDIR="${D}" PACKAGE_ARCH="${PACKAGE_ARCH}" install
}


inherit systemd

SYSTEMD_PACKAGES = "${PN}"
SYSTEMD_SERVICE_${PN} = "mdpd.service"
FILES_${PN} += "${systemd_unitdir}/system/mdpd.service"

do_install_append() {
    if ${@bb.utils.contains('DISTRO_FEATURES','systemd','true','false',d)}; then
        install -d ${D}${systemd_unitdir}/system/
        install -m 0644 ${B}/lib/systemd/system/mdpd.service ${D}${systemd_unitdir}/system
    fi
}

INSANE_SKIP_${PN} += "already-stripped"
FILES_${PN}-dev = "dev-elf"
FILES_${PN} += "${libdir}"

SECURITY_CFLAGS_pn-${PN} = "${SECURITY_NO_PIE_CFLAGS}"
