DESCRIPTION = "mpctec"
LICENSE = "MediaTekProprietary"
DEPENDS += " "

MPCTEC_SRC = "${TOPDIR}/../prebuilt/support/assistant-sdk/mpctec"

inherit workonsrc

WORKONSRC = "${MPCTEC_SRC}"

do_compile() {
    cd ${S} && make
}

do_install() {
	oe_runmake \
	PREFIX="${prefix}" DESTDIR="${D}" PACKAGE_ARCH="${PACKAGE_ARCH}" install
}

INSANE_SKIP_${PN} += "already-stripped"
FILES_${PN}-dev = ""
FILES_${PN} += "${bindir}"
FILES_${PN} += "${libdir}"

INSANE_SKIP_${PN} += "ldflags"
