DESCRIPTION = "app prebuilt"
LICENSE = "MediaTekProprietary"

APPS_SRC = "${TOPDIR}/../src/apps/aud-base/so"

inherit workonsrc

WORKONSRC = "${APPS_SRC}"

SYSTEMD_PACKAGES = "${PN}"

do_install() {
   install -d ${D}/${libdir}
   install -m 0644 ${S}/${base_libdir}/*.so ${D}/${libdir}
}

FILES_${PN} += "${libdir} /etc"
INSANE_SKIP_${PN} += "already-stripped"
FILES_${PN}-dev = ""
INSANE_SKIP_${PN} += "ldflags"
