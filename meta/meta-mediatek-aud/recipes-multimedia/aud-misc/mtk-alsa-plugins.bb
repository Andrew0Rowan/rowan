DESCRIPTION = "mtk-alsa-plugins"
LICENSE = "MediaTekProprietary"
LIC_FILES_CHKSUM = "file://LICENSE;md5=e1696b147d49d491bcb4da1a57173ff"
DEPENDS += "alsa-lib  mtk-audio-service"

MAS_SRC = "${TOPDIR}/../src/multimedia/audio-misc/mtk-audio-service"
WORKING_SRC = "${TOPDIR}/../src/multimedia/audio-misc/mtk-alsa-plugins"

inherit workonsrc 

WORKONSRC = "${WORKING_SRC}"

do_compile() {
	oe_runmake MCFLAG="-I${MAS_SRC}/include -I${MAS_SRC}/mil/inc \
 -I${MAS_SRC}/msb/inc -I${MAS_SRC}/mas_v2/inc"
}

do_install() {
    oe_runmake \
    PREFIX="${prefix}" DESTDIR="${D}" PACKAGE_ARCH="${PACKAGE_ARCH}" install
}

FILES_${PN} += "${libdir}/alsa-lib"
FILES_${PN}-dev = ""
INSANE_SKIP_${PN} += "already-stripped" 

INSANE_SKIP_${PN} += "ldflags"
