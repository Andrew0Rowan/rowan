DESCRIPTION = "Aispeech"
LICENSE = "MediaTekProprietary"
LIC_FILES_CHKSUM = "file://COPYING;md5=d7810fab7487fb0aad327b76f1be7cd7"
DEPENDS="alsa-lib AssistantCenter aispeech-prebuilt"

APPS_SRC = "${TOPDIR}/../src/support/assistant-ctrl/aispeech"

inherit workonsrc systemd

WORKONSRC = "${APPS_SRC}"
SYSTEMD_PACKAGES = "${PN}"
SYSTEMD_SERVICE_${PN} = "Aispeech.service"
do_compile() {
	make
}

do_install() {
	install -d ${D}/usr/lib/prompt
	install -d ${D}/${bindir}
	install -d ${D}${systemd_unitdir}/system
	install -m 755 ${S}/Aispeech ${D}/${bindir}
	install -m 755 ${S}/Aispeech.sh ${D}/${bindir}
	install -m 755 ${S}/Aispeech.service ${D}${systemd_unitdir}/system
	install -m 755 ${S}/prompt/* ${D}/usr/lib/prompt
}

FILES_${PN} += "${bindir} ${systemd_unitdir}/system/Aispeech.service /usr/lib/prompt"
INSANE_SKIP_${PN} += "already-stripped"
FILES_${PN}-dev = ""
