DESCRIPTION = "Prog"
LICENSE = "MediaTekProprietary"
LIC_FILES_CHKSUM = "file://COPYING;md5=d7810fab7487fb0aad327b76f1be7cd7"
DEPENDS += "app-prebuilt nvram ${@base_contains('LICENSE_FLAGS_BLUEZ', 'yes', '', 'bluetooth' ,d)} curl wifi-mw wlanMon mtk-alsa-plugins smartaudioplayer"

APPS_SRC = "${TOPDIR}/../src/apps/aud-base"

inherit workonsrc systemd

WORKONSRC = "${APPS_SRC}"
LDFLAGS += "-L ${S}/so"

SYSTEMD_PACKAGES = "${PN}"
SYSTEMD_SERVICE_${PN} = "${@base_contains('BT_LAUNCHER_SUFFIX', '7668_', 'appmainprog_7668.service', 'appmainprog.service' ,d)}"
FILES_${PN} += "${@base_contains('BT_LAUNCHER_SUFFIX', '7668_', '${systemd_unitdir}/system/appmainprog_7668.service', '${systemd_unitdir}/system/appmainprog.service' ,d)}"

do_compile() {
	cd ${S}/progs/app_cli && make
	cd ${S}/progs/nvram_save && make
	cd ${S}/progs/factoryrf && make
	cd ${S}/library/webrtc_aec && make
	cd ${S}/progs/uart_control && make
}

do_install() {
   install -d ${D}/${libdir}
   install -d ${D}${bindir}
   install -d ${D}/etc
   install -m 755 ${S}progs/app_cli/app_cli ${D}${bindir}
   install -m 755 ${S}progs/nvram_save/nvram_save ${D}${bindir}
   install -m 755 ${S}progs/factoryrf/factory_rf ${D}${bindir}
   install -m 755 ${S}progs/uart_control/uart_control ${D}${bindir}
   install -d ${D}${systemd_unitdir}/system
   install -m 755 ${S}service/appmainprog.sh ${D}${bindir}
   if test "${COMBO_CHIP_ID}" = "mt7668"; then
       install -m 755 ${S}service/appmainprog_7668.service ${D}${systemd_unitdir}/system
   else      
       install -m 755 ${S}service/appmainprog.service ${D}${systemd_unitdir}/system
   fi
   install -m 755 ${S}library/webrtc_aec/libaec.so ${D}/${libdir}
}

FILES_${PN} += "${libdir} /etc"
INSANE_SKIP_${PN} += "already-stripped ldflags"
FILES_${PN}-dev = ""
