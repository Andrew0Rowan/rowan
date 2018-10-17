DESCRIPTION = "Adaptor Prog"
LICENSE = "MediaTekProprietary"
LIC_FILES_CHKSUM = "file://COPYING;md5=d7810fab7487fb0aad327b76f1be7cd7"
DEPENDS += "nvram ${@base_contains('LICENSE_FLAGS_BLUEZ', 'yes', '', 'bluetooth' ,d)} curl wifi-mw wlanMon mtk-alsa-plugins smartaudioplayer"

APPS_SRC = "${TOPDIR}/../src/apps/aud-base"

inherit workonsrc

WORKONSRC = "${APPS_SRC}"
LDFLAGS += "-L ${S}/so"

do_compile() {
	cd ${S}/progs/app_cli && make
	cd ${S}/progs/nvram_save && make
	cd ${S}/progs/factoryrf && make
	cd ${S}/library/webrtc_aec && make
	cd ${S}/progs/uart_control && make
}

do_install() {
   install -d ${D}${libdir}
   install -d ${D}${bindir}
   install -m 755 ${S}progs/app_cli/app_cli ${D}${bindir}
   install -m 755 ${S}progs/nvram_save/nvram_save ${D}${bindir}
   install -m 755 ${S}progs/factoryrf/factory_rf ${D}${bindir}
   install -m 755 ${S}progs/uart_control/uart_control ${D}${bindir}
   install -m 755 ${S}library/webrtc_aec/libaec.so ${D}${libdir}
   install -m 755 ${S}so/${base_libdir}/*.so ${D}/${libdir}
}

FILES_${PN} += "${libdir} ${bindir}"
INSANE_SKIP_${PN} += "already-stripped ldflags"
FILES_${PN}-dev = ""
