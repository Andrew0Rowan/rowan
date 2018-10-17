DESCRIPTION = "coverity trap"
LICENSE = "MediaTekProprietary"
LIC_FILES_CHKSUM = "file://LICENSE;md5=e1696b147d49d491bcb4da1a57173ff"

SRC = "${TOPDIR}/../meta/base/conf/mt8516/aud8516p1v2-slc-test/coverity-trap"

inherit workonsrc

WORKONSRC = "${SRC}"

do_compile() {
	make
}

do_install() {
   install -d ${D}/${bindir}
   install -m 755 ${S}/coverity-trap ${D}/${bindir}
}
FILES_${PN} += "${bindir}"
FILES_${PN}-dev = ""
INSANE_SKIP_${PN} += "ldflags"
