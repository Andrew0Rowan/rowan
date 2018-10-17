inherit externalsrc

DESCRIPTION = "mediatek update image demo"
LICENSE = "MediaTekProprietary"
DEPENDS += "libdrm curl"
MTK_SRC = "${TOPDIR}/../src/apps/aud-base/main/upgrade_app"
LIC_FILES_CHKSUM = "file://LICENSE;md5=e1696b147d49d491bcb4da1a57173fff"
PN='upgrade_app'

INSANE_SKIP_${PN} +="installed-vs-shipped"

S="${WORKDIR}"

inherit workonsrc
WORKONSRC = "${TOPDIR}/../src/apps/aud-base/main/upgrade_app"

inherit pkgconfig
inherit systemd

SYSTEMD_PACKAGES = "${PN}"
SYSTEMD_SERVICE_${PN} = "upgrade_app.service"
FILES_${PN} += "${systemd_unitdir}/system/upgrade_app.service"

do_compile() {
	BOOTDEV_TYPE=${BOOTDEV_TYPE} \
	oe_runmake \
	CFLAGS=" -lpthread ${CFLAGS}" \
	LDFLAGS="-ldrm -lstdc++ -lpthread -lm -lcurl ${LDFLAGS}"
}

do_install(){
	oe_runmake \
	PREFIX="${PREFIX}" DESTDIR="${D}" install
}

do_install_append() {
	install -d ${D}${systemd_unitdir}/system
	install -m 0755 ${TOPDIR}/../meta/meta-mediatek-aud/recipes-apps/upgrade_app/upgrade_app.service ${D}${systemd_unitdir}/system
}
