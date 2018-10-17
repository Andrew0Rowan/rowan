DESCRIPTION = "This module serves the common part driver of connectivity"
LICENSE = "MediaTekProprietary"
LIC_FILES_CHKSUM = "file://LICENSE;md5=e1696b147d49d491bcb4da1a57173fff"

FILESEXTRAPATHS_append := ":${THISDIR}/${PN}"
SRC_URI += " \
	file://8516_wlan.sh \
    "

GDFLAGS += "${@base_contains('PEACEINGALAXY','yes','','-D_GUARDIAN_',d)}"
DEPENDS = "${@base_contains('PEACEINGALAXY','yes','','guardian',d)}" 
LIBGD = "${@base_contains('PEACEINGALAXY','yes','','-lgd',d)}"
inherit workonsrc
WORKONSRC = "${TOPDIR}/../src/connectivity/combo_tool"

inherit autotools

inherit systemd
SYSTEMD_PACKAGES = "${PN}"
SYSTEMD_SERVICE_${PN} = "wmtd.service launcher${WIFI_LAUNCHER_SUFFIX}.service stp_dump.service"

FILES_${PN} += "${systemd_unitdir}/system/wmtd.service"
FILES_${PN} += "${systemd_unitdir}/system/launcher${WIFI_LAUNCHER_SUFFIX}.service"
FILES_${PN} += "${systemd_unitdir}/system/stp_dump.service"
FILES_${PN} += "/lib/firmware/WMT_SOC.cfg"
FILES_${PN} += "/lib/firmware/mt6630_ant_m1.cfg"

do_compile () {
    if [ -e ${WORKONSRC}/Makefile.am ]; then
        if test "${TARGET_PLATFORM}" = "mt8516"; then
            echo "GDFLAGS=${GDFLAGS}"
            echo ${LIBGD}
            oe_runmake all CFLAGS="${GDFLAGS} -DMTK_COMBO_USING_PATCH_NAME=1 -DMTK_COMBO_ENABLE_WIFI_IN_LAUNCHER=1" LIBGD=${LIBGD}
        fi
    fi
} 

do_install() {
if [ ! -e ${WORKONSRC}/Makefile.am ]; then
   oe_runmake \
   PREFIX="${prefix}" DESTDIR="${D}" PACKAGE_ARCH="${PACKAGE_ARCH}" install
else
   install -d ${D}${bindir}
   install -m 0755 ${B}/wmt_loader ${D}${bindir}
   install -m 0755 ${B}/6620_launcher ${D}${bindir}
   install -m 0755 ${B}/power_on_wifi ${D}${bindir}
   install -m 0755 ${B}/stp_dump ${D}${bindir}
   install -m 0755 ${B}/wmt_loopback ${D}${bindir}
fi
}

do_install_append() {
if [ -e ${WORKONSRC}/Makefile.am ]; then
	install -d ${D}
	install -m 0755 ${WORKDIR}/8516_wlan.sh ${D}/usr/bin

	if ${@bb.utils.contains('DISTRO_FEATURES','systemd','true','false',d)}; then
		install -d ${D}${systemd_unitdir}/system/
		install -m 0644 ${B}/wmtd.service ${D}${systemd_unitdir}/system
		install -m 0644 ${B}/launcher${WIFI_LAUNCHER_SUFFIX}.service ${D}${systemd_unitdir}/system
		install -m 0644 ${B}/stp_dump.service ${D}${systemd_unitdir}/system
	fi

	if test "${COMBO_CHIP_ID}" = "mt8167"; then
        install -d ${D}/lib/firmware
        install -m 0755 ${S}/cfg_folder/WMT_SOC.cfg ${D}/lib/firmware
	fi

	if test "${COMBO_CHIP_ID}" = "mt6630"; then
	    install -d ${D}/lib/firmware
        install -m 0755 ${S}/cfg_folder/mt6630_ant_m1.cfg ${D}/lib/firmware
	fi
fi
}
INSANE_SKIP_${PN} += "already-stripped"
