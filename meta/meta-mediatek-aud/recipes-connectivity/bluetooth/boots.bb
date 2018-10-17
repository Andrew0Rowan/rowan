DESCRIPTION = "MTK boots"
LICENSE = "MediaTekProprietary"
LIC_FILES_CHKSUM = "file://LICENSE;md5=e1696b147d49d491bcb4da1a57173fff"

inherit workonsrc
WORKONSRC = "${TOPDIR}/../src/connectivity/bt_others/boots"

INSANE_SKIP_${PN} += "already-stripped"

FILES_${PN} += "${bindir}/boots"
FILES_${PN} += "${bindir}/boots_srv"

do_compile() {
    oe_runmake PREFIX="${prefix}" DESTDIR="${D}"
}

do_install() {
    install -d ${D}${bindir}
    install -m 0755 boots ${D}${bindir}
    install -m 0755 boots_srv ${D}${bindir}
}
