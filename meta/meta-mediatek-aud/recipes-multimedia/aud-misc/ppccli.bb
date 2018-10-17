DESCRIPTION = "ppc"
LICENSE = "MediaTekProprietary"
LIC_FILES_CHKSUM = "file://LICENSE;md5=e1696b147d49d491bcb4da1a57173ff"
DEPENDS += " "

PPCCLI_SRC = "${TOPDIR}/../src/multimedia/audio-misc/ppccli"

inherit workonsrc

WORKONSRC = "${PPCCLI_SRC}"

do_compile() {
	make
}

do_install() {
    oe_runmake \
    PREFIX="${prefix}" DESTDIR="${D}" PACKAGE_ARCH="${PACKAGE_ARCH}" install
}

FILES_${PN} += "${bindir}"
FILES_${PN}-dev = "" 
INSANE_SKIP_${PN} += "already-stripped"
INSANE_SKIP_${PN} += "ldflags"
