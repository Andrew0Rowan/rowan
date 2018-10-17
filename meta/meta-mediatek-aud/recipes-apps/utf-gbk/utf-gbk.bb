#Basic Configuration
DESCRIPTION = "This module is a test app for UTF to GBK"
LICENSE = "MediaTekProprietary"
LIC_FILES_CHKSUM = "file://LICENSE;md5=e1696b147d49d491bcb4da1a57173fff"
inherit workonsrc
WORKONSRC = "${TOPDIR}/../meta/meta-mediatek-aud/recipes-apps/utf-gbk/src"

do_install() {
    oe_runmake PREFIX="${prefix}" DESTDIR="${D}" install
}

inherit autotools
