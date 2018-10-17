DESCRIPTION = "This module serves the common part driver of connectivity"
LICENSE = "MediaTekProprietary"
LIC_FILES_CHKSUM = "file://LICENSE;md5=e1696b147d49d491bcb4da1a57173fff"

inherit workonsrc
WORKONSRC = "${TOPDIR}/../src/connectivity/combo_tool"

inherit autotools

inherit systemd

SYSTEMD_PACKAGES = "${PN}"
SYSTEMD_SERVICE_${PN} = "wmtd.service launcher.service poweronwifi.service stp_dump.service"
FILES_${PN} += "${systemd_unitdir}/system/wmtd.service"
FILES_${PN} += "${systemd_unitdir}/system/launcher.service"
FILES_${PN} += "${systemd_unitdir}/system/poweronwifi.service"
FILES_${PN} += "${systemd_unitdir}/system/stp_dump.service"
FILES_${PN} += "/lib/firmware/mt6630_ant_m1.cfg"

do_compile () {
    if test "${TARGET_PLATFORM}" = "mt2712"; then
        oe_runmake all CFLAGS="-DMTK_COMBO_USING_PATCH_NAME=1"
    fi
}

do_install_append() {
        install -d ${D}/lib/firmware
        install -m 0755 ${S}/cfg_folder/mt6630_ant_m1.cfg ${D}/lib/firmware
    if ${@bb.utils.contains('DISTRO_FEATURES','systemd','true','false',d)}; then
        install -d ${D}${systemd_unitdir}/system/
        install -m 0644 ${B}/wmtd.service ${D}${systemd_unitdir}/system
        install -m 0644 ${B}/launcher.service ${D}${systemd_unitdir}/system
        install -m 0644 ${B}/poweronwifi.service ${D}${systemd_unitdir}/system
        install -m 0644 ${B}/stp_dump.service ${D}${systemd_unitdir}/system
    fi
}

