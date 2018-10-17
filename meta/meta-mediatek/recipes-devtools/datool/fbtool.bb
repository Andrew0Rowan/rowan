inherit deploy externalsrc

DESCRIPTION = "MediaTek fastboot DA tool"
EXTERNALSRC = "${TOPDIR}/../src/devtools/datool"
EXTERNALSRC_BUILD = "${TOPDIR}/../src/devtools/datool"

LICENSE = "GPLv2"
LIC_FILES_CHKSUM = "file://COPYING;md5=4db5960ea4b2e32e99fcdf93de0652e3"
BBCLASSEXTEND = "native nativesdk"
ALLOW_EMPTY_${PN} = "1"

do_install () {
}

do_deploy () {
        install -d ${DEPLOYDIR}
        install -m 0644 fbtool.py -t ${DEPLOYDIR}
        cp -af pyserial ${DEPLOYDIR}
}

addtask deploy before do_build after do_compile

