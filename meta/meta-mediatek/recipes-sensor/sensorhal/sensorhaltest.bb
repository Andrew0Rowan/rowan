#Basic Configuration
DESCRIPTION = "This module is a sensorhal test library."
LICENSE = "MediaTekProprietary"
LIC_FILES_CHKSUM = "file://LICENSE;md5=e1696b147d49d491bcb4da1a57173fff"
DEPENDS += "sensorhal systemd"
inherit workonsrc
WORKONSRC = "${TOPDIR}/../src/navigation/sensor/sensorhaltest"

do_install() {
    oe_runmake PREFIX="${prefix}" DESTDIR="${D}" install
}

inherit autotools
