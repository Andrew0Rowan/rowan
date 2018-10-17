DESCRIPTION = "MediaTek mdp pq library"
LICENSE = "MediaTekProprietary"
LIC_FILES_CHKSUM = "file://NOTICE;md5=e1696b147d49d491bcb4da1a57173fff"
SECTION = "libs"

inherit workonsrc
WORKONSRC = "${TOPDIR}/../src/multimedia/libpq"

RDEPENDS_${PN} = "libgcc"
RDEPENDS_${PN} += "libstdc++"

inherit pkgconfig

do_configure() {
        :
}

do_compile() {
	oe_runmake \
		PACKAGE_ARCH="${PACKAGE_ARCH}" \
		CFLAGS="`pkg-config` ${CFLAGS}" \
		LDFLAGS="`pkg-config` ${LDFLAGS}"
}

do_install() {
	oe_runmake \
		PREFIX="${prefix}" DESTDIR="${D}" PACKAGE_ARCH="${PACKAGE_ARCH}"  install
}

INSANE_SKIP_${PN} += "already-stripped"
FILES_${PN}-dev = "dev-elf"
FILES_${PN} += "${libdir}"

SECURITY_CFLAGS_pn-${PN} = "${SECURITY_NO_PIE_CFLAGS}"
