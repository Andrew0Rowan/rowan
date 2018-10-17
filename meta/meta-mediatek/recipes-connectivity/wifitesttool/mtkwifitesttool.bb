DESCRIPTION = "Wi-Fi test tool fro HW test"
LICENSE = "MediaTekProprietary"
LIC_FILES_CHKSUM = "file://LICENSE;md5=e1696b147d49d491bcb4da1a57173fff"

inherit workonsrc
WORKONSRC = "${TOPDIR}/../src/connectivity/wifitesttool"

inherit autotools

do_install() {
    oe_runmake PREFIX="${prefix}" DESTDIR="${D}" PACKAGE_ARCH="${PACKAGE_ARCH}" install
}
INSANE_SKIP_${PN} += "already-stripped"
