DESCRIPTION = "Mediatek MT7668 BT/WIFI Driver insmod"
LICENSE = "MediaTekProrietary"
LIC_FILES_CHKSUM = "file://COPYING;md5=d7810fab7487fb0aad327b76f1be7cd7"

INSMOD_SRC = "${TOPDIR}/../meta/meta-mediatek-aud/recipes-kernel/modules"

inherit workonsrc systemd

WORKONSRC = "${INSMOD_SRC}"

SYSTEMD_PACKAGES = "${PN}"
SYSTEMD_SERVICE_${PN} = "mt7668_drv_insmod.service"
FILES_${PN} += "${systemd_unitdir}/system/mt7668_drv_insmod.service"

do_install() {
	install -d ${D}/etc
	install -m 755 ${WORKONSRC}/mt7668_drv_insmod.sh ${D}/etc/mt7668_drv_insmod.sh
	install -d ${D}${systemd_unitdir}/system
	install -m 755 ${WORKONSRC}/mt7668_drv_insmod.service ${D}${systemd_unitdir}/system
}

FILES_${PN}+="etc/mt7668_drv_insmod.sh ${systemd_unitdir}"
