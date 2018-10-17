SUMMARY = "Ldvt init script"
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://${COREBASE}/meta/COPYING.MIT;md5=3da9cfbcb788c80a0384361b4de20420"
DEPENDS = "virtual/kernel"
RDEPENDS_${PN} = "udev udev-extraconf"
SRC_URI = "file://init-ldvt.sh"

S = "${WORKDIR}"

do_install() {
        install -m 0755 ${WORKDIR}/init-ldvt.sh ${D}/ldvt_init
}

FILES_${PN} += " /ldvt_init "

# Due to kernel dependency
PACKAGE_ARCH = "${MACHINE_ARCH}"
