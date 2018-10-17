DESCRIPTION = "new_program"
LICENSE = "MediaTekProprietary"

APPS_SRC = "${TOPDIR}/../src/apps/aud-base/new_program"

inherit workonsrc systemd

WORKONSRC = "${APPS_SRC}"

SYSTEMD_PACKAGES = "${PN}"
SYSTEMD_SERVICE_${PN} = "new_program.service"

do_compile() {
	make
}

do_install() {
   install -d ${D}${bindir}
   install -d ${D}${systemd_unitdir}/system
   
   install -m 755 ${S}/new_program ${D}${bindir}
   install -m 755 ${S}/new_program.service ${D}${systemd_unitdir}/system
}

FILES_${PN} += "${systemd_unitdir}/system/new_program.service"
INSANE_SKIP_${PN} += "already-stripped ldflags"
FILES_${PN}-dev = ""
