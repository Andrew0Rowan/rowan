DESCRIPTION = "AssistantCenter"
LICENSE = "MediaTekProprietary"
LIC_FILES_CHKSUM = "file://COPYING;md5=d7810fab7487fb0aad327b76f1be7cd7"
DEPENDS += "cjson"

APPS_SRC = "${TOPDIR}/../src/apps/aud-base/progs/AssistantCenter"

inherit workonsrc

WORKONSRC = "${APPS_SRC}"

do_compile() {
	make
}

do_install() {
	install -d ${D}/${libdir}
	install -m 755 ${S}/libAssistantCenter.so  ${D}/${libdir}
}

FILES_${PN} += "${libdir} ${bindir}"
INSANE_SKIP_${PN} += "already-stripped"
FILES_${PN}-dev = ""
