DESCRIPTION = "eth-mac-setting"
LICENSE = "MediaTekProprietary"
LIC_FILES_CHKSUM = "file://COPYING;md5=d7810fab7487fb0aad327b76f1be7cd7"

APPS_SRC = "${TOPDIR}/../src/apps/aud-base/eth-mac-setting/"

inherit workonsrc systemd

WORKONSRC = "${APPS_SRC}"

SYSTEMD_PACKAGES = "${PN}"
SYSTEMD_SERVICE_${PN} = "eth-mac-setting.service"
FILES_${PN} += "${systemd_unitdir}/system/eth-mac-setting.service"


do_install() {
   install -d ${D}${bindir}
   install -m 755 ${S}eth-mac-setting.sh ${D}${bindir}
   install -d ${D}${systemd_unitdir}/system
   install -m 755 ${S}eth-mac-setting.service ${D}${systemd_unitdir}/system
}

FILES_${PN} += ""
INSANE_SKIP_${PN} += "already-stripped"
FILES_${PN}-dev = ""