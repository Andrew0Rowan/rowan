DESCRIPTION = "MTK eFuse Writer"
LICENSE = "MediaTekProprietary"
LIC_FILES_CHKSUM = "file://LICENSE;md5=e1696b147d49d491bcb4da1a57173fff"

inherit workonsrc
WORKONSRC = "${TOPDIR}/../src/support/efuse_writer"

DEPENDS += "tzapp"
RDEPENDS_${PN} += "tzapp"

do_compile() {
        oe_runmake
}

do_install() {
	install -d ${D}${bindir}
	install -m 0755 ewriter ${D}${bindir}/
}
