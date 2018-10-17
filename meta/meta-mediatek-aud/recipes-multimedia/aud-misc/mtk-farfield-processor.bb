DESCRIPTION = "mtk-farfield-processor"
LICENSE = "MediaTekProprietary"
LIC_FILES_CHKSUM = "file://LICENSE;md5=e1696b147d49d491bcb4da1a57173ff"
DEPENDS += "mtk-audio-service"

MAS_SRC = "${TOPDIR}/../src/multimedia/audio-misc/mtk-audio-service"
WORKING_SRC = "${TOPDIR}/../src/multimedia/audio-misc/mtk-farfield-processor"

inherit workonsrc 

WORKONSRC = "${WORKING_SRC}"

do_compile() {
	oe_runmake MCFLAG="-I${MAS_SRC}/msb/inc"
}

do_install() {
    oe_runmake \
    PREFIX="${prefix}" DESTDIR="${D}" PACKAGE_ARCH="${PACKAGE_ARCH}" install
}

FILES_${PN} += "${libdir}"
FILES_${PN}-dev = ""
INSANE_SKIP_${PN} += "already-stripped" 
INSANE_SKIP_${PN} += "ldflags"
