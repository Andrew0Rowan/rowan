LICENSE = "ZelusTekProprietary"

GDFLAGS += "${@base_contains('PEACEINGALAXY','yes','','-D_GUARDIAN_',d)}"

inherit workonsrc
WORKONSRC = "${TOPDIR}/../src/support/guardian"

do_compile() {
	oe_runmake CFLAGS="${GDFLAGS}"
}

do_install() {
    oe_runmake PREFIX="${prefix}" DESTDIR="${D}" PACKAGE_ARCH="${PACKAGE_ARCH}" install
}

FILES_${PN} += "${libdir}"
FILES_${PN}-dev = "" 
INSANE_SKIP_${PN} += "already-stripped"
INSANE_SKIP_${PN} += "ldflags"
