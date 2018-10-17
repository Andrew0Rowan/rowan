DESCRIPTION = "MediaTek OVL ADAPTER"
LICENSE = "MediaTekProprietary"
LIC_FILES_CHKSUM = "file://NOTICE;md5=e1696b147d49d491bcb4da1a57173fff"
SECTION = "libs"

inherit workonsrc
WORKONSRC = "${TOPDIR}/../src/multimedia/mtk-ovl-adapter"

#RDEPENDS_${PN} = "libgcc"
#RDEPENDS_${PN} = "libstdc++"


FILES_SOLIBSDEV = ""
FILES_${PN} = "${libdir}/libmtk-ovl-adapter.so"

do_compile() {
	oe_runmake \
		CFLAGS="${CFLAGS}" \
		LDFLAGS="${LDFLAGS}"
}

do_install() {
	oe_runmake \
		DESTDIR="${D}" LIBDIR="${libdir}" INCDIR="${includedir}"  install
}

