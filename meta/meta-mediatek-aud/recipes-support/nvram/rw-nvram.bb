DESCRIPTION = "rw_nvram"
LICENSE = "MPLV2"
LIC_FILES_CHKSUM = "file://LICENSE;md5=e1696b147d49d491bcb4da1a57173ff"
DEPENDS = "NvRAMUtils libnvramcustom nvram"
WORKING_SRC = "${TOPDIR}/../src/support/nvram/tests"

inherit workonsrc 

WORKONSRC = "${WORKING_SRC}"

do_compile() {
	oe_runmake
}

do_install() {
	install -d ${D}/${bindir}
	install -m 755 ${S}/rw_nvram ${D}/${bindir}
}

FILES_${PN} += "${bindir}"
FILES_${PN}-dev = "" 
INSANE_SKIP_${PN} += "already-stripped"
