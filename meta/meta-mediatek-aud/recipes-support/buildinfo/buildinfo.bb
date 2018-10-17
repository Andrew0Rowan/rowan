DESCRIPTION = "buildinfo"
LICENSE = "MediaTekProprietary"
LIC_FILES_CHKSUM = "file://COPYING;md5=d7810fab7487fb0aad327b76f1be7cd7"

APPS_SRC = "${TOPDIR}/../prebuilt/support/buildinfo"

inherit workonsrc 

WORKONSRC = "${APPS_SRC}"


do_install() {
	install -d ${D}/temp
	install -d ${D}/temp/version
	install -m 755 ${S}version.ini ${D}/temp/version/version.ini
}

FILES_${PN}+="temp/version"
