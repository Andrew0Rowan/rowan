inherit workonsrc

DESCRIPTION = "aac library"
LICENSE = "fraunhofer"
LIC_FILES_CHKSUM = "file://LICENSE;md5=e1696b147d49d491bcb4da1a57173fff"
WORKONSRC = "${TOPDIR}/../src/multimedia/audio-misc/libaac"

inherit systemd
SYSTEMD_PACKAGES = "${PN}"

do_compile() {
       echo aac start compile
       oe_runmake all
       echo aac end compile
}

do_install() {
    install -d ${D}${libdir}
    install -m 755 ${S}/libaac.so ${D}${libdir}/
}

FILES_${PN} += "${libdir}"
FILES_${PN}-dev = ""
INSANE_SKIP_${PN} += "ldflags"
