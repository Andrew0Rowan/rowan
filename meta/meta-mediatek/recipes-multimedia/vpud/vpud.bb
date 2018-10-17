DESCRIPTION = "MediaTek VPU daemon"
LICENSE = "MediaTekProprietary"
LIC_FILES_CHKSUM = "file://NOTICE;md5=e1696b147d49d491bcb4da1a57173fff"

inherit workonsrc
WORKONSRC = "${TOPDIR}/../src/multimedia/vpu"

DEPENDS = "fuse"
RDEPENDS_${PN} = "libgcc libstdc++"

inherit pkgconfig

do_compile() {
	oe_runmake \
		TARGET_PLATFORM="${TARGET_PLATFORM}" \
		CFLAGS="`pkg-config --cflags fuse` ${CFLAGS}" \
		LDFLAGS="`pkg-config --libs fuse` ${LDFLAGS}"
}

do_install() {
	oe_runmake \
		PREFIX="${prefix}" DESTDIR="${D}" PACKAGE_ARCH="${PACKAGE_ARCH}" TARGET_PLATFORM="${TARGET_PLATFORM}" install
}


inherit systemd

SYSTEMD_PACKAGES = "${PN}"
SYSTEMD_SERVICE_${PN} = "vpud.service"
FILES_${PN} += "${systemd_unitdir}/system/vpud.service"

do_install_append() {
    if ${@bb.utils.contains('DISTRO_FEATURES','systemd','true','false',d)}; then
        install -d ${D}${systemd_unitdir}/system/
        install -m 0644 ${B}/lib/systemd/system/vpud.service ${D}${systemd_unitdir}/system
    fi
}

INSANE_SKIP_${PN} += "already-stripped"
FILES_${PN}-dev = "dev-elf"
FILES_${PN} += "/usr/lib"

SECURITY_CFLAGS_pn-${PN} = "${SECURITY_NO_PIE_CFLAGS}"
