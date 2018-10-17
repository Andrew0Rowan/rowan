inherit package

INHIBIT_PACKAGE_STRIP = "1"

DESCRIPTION = "Mediatek ldvt uvvf library"
LICENSE = "MediaTekProprietary"
LIC_FILES_CHKSUM = "file://LICENSE;md5=cfae02679eba352c5d667a0cda7ef56e"

TESTCASE_SRC = "${TOPDIR}/../src/ldvt/ldvt_tc/"
TESTCASE_BIN_DIR = "${D}/usr/local/ldvt/bin"
TESTCASE_BIN_INSTALL = "/usr/local/ldvt/bin"
SRC_URI = "file://${TESTCASE_SRC}"
S = "${TESTCASE_SRC}"

do_compile () {
	oe_runmake
}

do_install () {
	oe_runmake \
		DESTDIR="${D}" install
}

FILES_${PN} += "${TESTCASE_BIN_INSTALL}"
