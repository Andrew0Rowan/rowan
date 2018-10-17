DESCRIPTION = "MTK EPOS Service"
LICENSE = "MediaTekProprietary"
LIC_FILES_CHKSUM = "file://LICENSE;md5=cfae02679eba352c5d667a0cda7ef56e"

inherit workonsrc
WORKONSRC = "${TOPDIR}/../src/connectivity/epos"

DEPENDS += "openssl zlib"
FILES_${PN}-staticdev = "${libdir}/libepos.a"

ALLOW_EMPTY_${PN} = "1"

do_install() {
	install -d ${D}${libdir}
	install -m 0644 libepos.a ${D}${libdir}/libepos.a
}
