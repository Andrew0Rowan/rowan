DESCRIPTION = "SmartAudioPlayer"
LICENSE = "MediaTekProprietary"
LIC_FILES_CHKSUM = "file://COPYING;md5=d7810fab7487fb0aad327b76f1be7cd7"
DEPENDS += "alsa-lib"
DEPENDS += "${@base_contains('LICENSE_FLAGS_WHITELIST','commercial','ffmpeg','',d)}" 

CFLAGS += "${@base_contains('LICENSE_FLAGS_WHITELIST','commercial','-DCONFIG_SUPPORT_FFMPEG=1','',d)}"

PLAYER_SRC = "${TOPDIR}/../src/multimedia/smartaudioplayer"

inherit workonsrc

WORKONSRC = "${PLAYER_SRC}"

do_compile() {
    cd ${S} && make
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
