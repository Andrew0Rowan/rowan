DESCRIPTION = "MTK Boot Assist"
LICENSE = "MediaTekProprietary"
LIC_FILES_CHKSUM = "file://LICENSE;md5=e1696b147d49d491bcb4da1a57173fff"
SRC_URI = "file://boot_assist_basic.service \
           file://boot_assist_default.service \
           file://bootsc \
           file://LICENSE \
"

S = "${WORKDIR}"

inherit systemd

SYSTEMD_PACKAGES = "${PN}"
SYSTEMD_SERVICE_${PN} = "boot_assist_basic.service boot_assist_default.service"
FILES_${PN} = "${systemd_unitdir} ${datadir}/boot-assist"
do_install_append() {
    install -d ${D}${datadir}/boot-assist
    install -m 0755 ${S}/bootsc ${D}${datadir}/boot-assist
    if ${@bb.utils.contains('DISTRO_FEATURES','systemd','true','false',d)}; then
        install -d ${D}${systemd_unitdir}/system/
        install -m 0644 ${S}/boot_assist_basic.service ${D}${systemd_unitdir}/system
        install -m 0644 ${S}/boot_assist_default.service ${D}${systemd_unitdir}/system
    fi
}
