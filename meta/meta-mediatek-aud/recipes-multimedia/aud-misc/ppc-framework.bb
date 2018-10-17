DESCRIPTION = "ppc-framework"
LICENSE = "MediaTekProprietary"
LIC_FILES_CHKSUM = "file://LICENSE;md5=e1696b147d49d491bcb4da1a57173ff"
DEPENDS += " "

PDCT_LIB = "${TOPDIR}/../prebuilt/support/pdct/pdct/mt8516"
PPC_SRC = "${TOPDIR}/../src/multimedia/audio-misc/ppc"
PPCFRAMEWORK_SRC = "${TOPDIR}/../src/multimedia/audio-misc/ppc_framework"

inherit workonsrc

WORKONSRC = "${PPCFRAMEWORK_SRC}"

ALLOW_EMPTY_${PN} = "1"

do_compile() {
	make MCFLAG="-I${PPC_SRC}/model/inc -L${PDCT_LIB} -lpdct -ltz_efuse -ltz_uree"
}

do_install() {
    oe_runmake \
    PREFIX="${prefix}" DESTDIR="${D}" PACKAGE_ARCH="${PACKAGE_ARCH}" install
}

FILES_${PN} += "${libdir}"
FILES_${PN}-dev = "" 
FILES_${PN}-staticdev = "${libdir}/*.a"