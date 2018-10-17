DESCRIPTION = "wenzhi"
LICENSE = "MediaTekProprietary"
DEPENDS += " "

WENZHI_SRC = "${TOPDIR}/../prebuilt/support/assistant-sdk/wenzhi"

inherit workonsrc

WORKONSRC = "${WENZHI_SRC}"

do_compile() {
    cd ${S} && make
}

do_install() {
	oe_runmake \
	PREFIX="${prefix}" DESTDIR="${D}" PACKAGE_ARCH="${PACKAGE_ARCH}" ADC_SETTING_FLAG="${ADC_SETTING}" install
}

INSANE_SKIP_${PN} += "already-stripped"
FILES_${PN}-dev = ""
FILES_${PN} += "${bindir}"
FILES_${PN} += "${libdir}"

INSANE_SKIP_${PN} += "ldflags"
