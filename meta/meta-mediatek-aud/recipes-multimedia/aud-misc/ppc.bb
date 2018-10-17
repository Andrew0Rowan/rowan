DESCRIPTION = "ppc"
LICENSE = "MediaTekProprietary"
LIC_FILES_CHKSUM = "file://LICENSE;md5=e1696b147d49d491bcb4da1a57173ff"
DEPENDS += "ppc-framework tzapp"
RDEPENDS_${PN} += "tzapp"

PPC_SRC = "${TOPDIR}/../src/multimedia/audio-misc/ppc"

inherit workonsrc

WORKONSRC = "${PPC_SRC}"

do_compile() {
	make
}

do_install() {
    oe_runmake \
    PREFIX="${prefix}" DESTDIR="${D}" PACKAGE_ARCH="${PACKAGE_ARCH}" install
}

FILES_${PN} += "${libdir}"
FILES_${PN}-dev = ""
FILES_${PN}-dev += "${includedir}"
INSANE_SKIP_${PN} += "ldflags"
INSANE_SKIP_${PN} += "already-stripped"
