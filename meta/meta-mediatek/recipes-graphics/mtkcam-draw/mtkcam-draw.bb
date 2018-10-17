DESCRIPTION = "MediaTek image processing lib"
LICENSE = "MediaTekProprietary"
LIC_FILES_CHKSUM = "file://NOTICE;md5=e1696b147d49d491bcb4da1a57173fff"
SECTION = "libs"

inherit workonsrc
WORKONSRC = "${TOPDIR}/../src/graphics/mtkcam-draw"

DEPENDS = "virtual/egl drm"

FILES_SOLIBSDEV = ""
FILES_${PN} = "${libdir}/libmtkcam-draw.so"

do_compile() {
	oe_runmake \
		CFLAGS="${CFLAGS} -I${STAGING_INCDIR}/drm" \
		LDFLAGS="${LDFLAGS}"
}

do_install() {
	oe_runmake \
		DESTDIR="${D}" LIBDIR="${libdir}" INCDIR="${includedir}"  install
}
