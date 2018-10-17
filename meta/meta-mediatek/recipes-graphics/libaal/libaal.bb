DESCRIPTION = "MediaTek AAL daemon"
LICENSE = "MediaTekProprietary"

inherit workonsrc
WORKONSRC = "${TOPDIR}/../src/graphics/libaal"

inherit pkgconfig

do_configure() {
        :
}

do_compile() {
	oe_runmake \
		TARGET_PLATFORM="${TARGET_PLATFORM}" \
		CFLAGS+="${CFLAGS}"
}

do_install() {
	oe_runmake \
		PREFIX="${prefix}" PACKAGE_ARCH="${PACKAGE_ARCH}" TARGET_PLATFORM="${TARGET_PLATFORM}" DESTDIR="${D}" LIBDIR="${libdir}" INCDIR="${includedir}" install
}

INSANE_SKIP_${PN} += "already-stripped"
FILES_${PN}-dev = ""
FILES_${PN} += "${includedir} ${libdir} ${bindir}"

