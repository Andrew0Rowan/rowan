DESCRIPTION = "Wi-Fi Middleware share library"
LICENSE = "MediaTekProprietary"
LIC_FILES_CHKSUM = "file://LICENSE;md5=d7810fab7487fb0aad327b76f1be7cd7"
#DEPENDS += "libpthread libipcd"

inherit workonsrc

WORKONSRC = "${TOPDIR}/../src/connectivity/wlan_smartconnection"
S="${WORKDIR}"

do_compile() {
    echo $PWD
    echo smtcn-mw start compile
    make TOPDIR=${TOPDIR} MTK_PROJECT=${MTK_PROJECT} WORKONSRC=${WORKONSRC}
    echo smtcn-mw end compile
}

do_install() {
    oe_runmake \
    PREFIX="${prefix}" DESTDIR="${D}" "WORKDIR=${S}" TOPDIR="${TOPDIR}" PACKAGE_ARCH="${PACKAGE_ARCH}" install
}

FILES_${PN} += "${libdir}/*"
FILES_${PN} += "${bindir}"
FILES_${PN} += "${includedir}"
FILES_${PN}-dev = ""

INSANE_SKIP_${PN} += "ldflags"
INSANE_SKIP_${PN} +="already-stripped"
