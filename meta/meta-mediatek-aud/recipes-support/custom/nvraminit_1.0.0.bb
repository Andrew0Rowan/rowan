#Basic Configuration
DESCRIPTION = "NVRAM environment initialization."
SECTION = "base"
LICENSE = "MediaTekProprietary"
LIC_FILES_CHKSUM = "file://LICENSE;md5=e1696b147d49d491bcb4da1a57173fff"
MTK_SRC = "${TOPDIR}/../prebuilt/support/nvram/bin"

WORKONSRC = "${MTK_SRC}"

inherit workonsrc

ALLOW_EMPTY_${PN} = "1"

do_install () {
    chmod a+x ${MTK_SRC}/bin/*
    cp -af ${MTK_SRC}/bin/* ${TOPDIR}/../meta/poky/scripts
    install -d ${STAGING_INCDIR}/install
    cp -af ${MTK_SRC}/* ${STAGING_INCDIR}/install/

}
