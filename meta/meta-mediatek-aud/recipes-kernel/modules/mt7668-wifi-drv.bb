DESCRIPTION = "Mediatek MT7668 WiFi Driver"
LICENSE = "GPLv2"
LIC_FILES_CHKSUM = "file://COPYING;md5=d7810fab7487fb0aad327b76f1be7cd7"
DEPENDS = "linux-mtk-extension"
MT7668_SRC = "${TOPDIR}/../src/kernel/modules/connectivity"

inherit workonsrc
WORKONSRC = "${MT7668_SRC}/wlan_driver/gen4m"
HIF = "sdio"
MODULE_NAME = "wlan_${COMBO_CHIP_ID}_${HIF}"
LINUX_SRC = "${TOPDIR}/tmp/work/${MACHINE_ARCH}-poky-${TARGET_OS}/${PREFERRED_PROVIDER_virtual/kernel}/4.4-${PR}/${PREFERRED_PROVIDER_virtual/kernel}-4.4/"
LDFLAGS = "-L ${TOPDIR}/tmp/sysroots/${MTK_PROJECT}/${libdir}"
PATH += "${LINUX_SRC}"

do_configure(){
}

do_compile() {
	if test "${COMBO_CHIP_ID}" = "mt7668"; then
		echo mt7668 wifi start compile
		echo ${WORKONSRC}
		echo ${S}

		cd ${S} && make -f Makefile.ce TOPDIR=${TOPDIR} HIF=${HIF} MODULE_NAME=${MODULE_NAME} DRIVER_DIR=${S} LINUX_SRC=${LINUX_SRC} ARCH=arm64 CROSS_COMPILE=aarch64-poky-linux- PLATFORM=MT8516
		echo mt7668 wifi end compile
	fi
}

do_install() {
	echo ${D}
   
	if test "${COMBO_CHIP_ID}" = "mt7668"; then
		install -d ${D}/lib/modules/mt7668/
		install -m 0644 ${S}/${MODULE_NAME}.ko ${D}/lib/modules/mt7668/
	fi
}

FILES_${PN} += "/lib/modules/mt7668"
INSANE_SKIP_${PN} += "already-stripped"
FILES_${PN}-dev = ""
