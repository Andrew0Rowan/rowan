DESCRIPTION = "mtk-audio-service"
LICENSE = "MediaTekProprietary"
LIC_FILES_CHKSUM = "file://LICENSE;md5=e1696b147d49d491bcb4da1a57173ff"
DEPENDS += "alsa-lib ppc"

PDCT_LIB = "${TOPDIR}/../prebuilt/support/pdct/pdct/mt8516/${KERNEL_ARCH}"
MAS_SRC = "${TOPDIR}/../src/multimedia/audio-misc/mtk-audio-service"
BTA2DP_SRC = "${TOPDIR}/../src/connectivity/bt_others/bluetooth_mw/sdk/inc/hardware"

inherit workonsrc

WORKONSRC = "${MAS_SRC}"

do_compile() {
	make MCFLAG="-I${BTA2DP_SRC}" BLICFLAG="-L${PDCT_LIB} -lpdct -ltz_efuse -ltz_uree"
}

do_install() {
    oe_runmake \
    PREFIX="${prefix}" DESTDIR="${D}" PACKAGE_ARCH="${PACKAGE_ARCH}" install
}

FILES_${PN} += "${libdir}"
FILES_${PN}-dev = ""
FILES_${PN}-dev += "${includedir}"
INSANE_SKIP_${PN} += "already-stripped"
INSANE_SKIP_${PN} += "ldflags"
