DESCRIPTION = "MediaTek PQ daemon"
LICENSE = "MediaTekProprietary"

inherit workonsrc
WORKONSRC = "${TOPDIR}/../src/graphics/disppq"

DEPENDS = "drm"

inherit pkgconfig

do_configure() {
        :
}

do_compile() {
	oe_runmake \
		TARGET_PLATFORM="${TARGET_PLATFORM}" \
		CFLAGS+="${CFLAGS} -I${STAGING_INCDIR}/drm"
}

do_install() {
	oe_runmake \
		PREFIX="${prefix}" PACKAGE_ARCH="${PACKAGE_ARCH}" TARGET_PLATFORM="${TARGET_PLATFORM}" DESTDIR="${D}" LIBDIR="${libdir}" INCDIR="${includedir}" install
}

FILES_${PN}-dev = ""
FILES_${PN} += "${includedir} ${libdir} ${bindir} ${sysconfdir}/pq"


