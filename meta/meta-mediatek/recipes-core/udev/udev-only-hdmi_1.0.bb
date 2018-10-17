SUMMARY = "Extra machine specific configuration files"
DESCRIPTION = "Extra machine specific configuration files for udev, specifically blacklist information."
LICENSE = "MediaTekProprietary"

SRC_URI = " \
       file://hdmi_hotplug.rules \
       file://hdmi_hotplug.sh \
"

S = "${WORKDIR}"

do_install() {
    install -d ${D}${sysconfdir}/udev/rules.d
    install -m 0644 ${WORKDIR}/hdmi_hotplug.rules ${D}${sysconfdir}/udev/rules.d/hdmi_hotplug.rules
    install -d ${D}${sysconfdir}/udev/scripts/
    install -m 0755 ${WORKDIR}/hdmi_hotplug.sh ${D}${sysconfdir}/udev/scripts/hdmi_hotplug.sh
}

FILES_${PN} = "${sysconfdir}/udev"
RDEPENDS_${PN} = "udev"
