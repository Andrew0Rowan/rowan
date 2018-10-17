DESCRIPTION = "Mediatek MT7668 ATE Tool"
LICENSE = "GPLv2"
LIC_FILES_CHKSUM = "file://COPYING;md5=d7810fab7487fb0aad327b76f1be7cd7"

inherit workonsrc
WORKONSRC = "${TOPDIR}/../src/connectivity/wlan_daemon/ated_ext"
MODULE_NAME = "ated"
CFLAGS += "-I ${TOPDIR}/tmp/sysroots/${MTK_PROJECT}/usr/include/"
CFLAGS += "-I ${TOPDIR}/tmp/sysroots/${MTK_PROJECT}/usr/lib64/"
LDFLAGS += "-L ${TOPDIR}/tmp/sysroots/${MTK_PROJECT}/usr/lib64/"
LDFLAGS += "-L ${TOPDIR}/tmp/sysroots/${MTK_PROJECT}/lib64/"
do_compile() {
	if test "${COMBO_CHIP_ID}" = "mt7668"; then
		echo mt7668 ate tool start compile
		echo ${WORKONSRC}
		echo ${S}

		cd ${S} && make PLATFORM=MT8516 CROSS_COMPILE=aarch64-poky-linux- MODULE_NAME=${MODULE_NAME} v=2 m=3 p=2
		echo mt7668 ate tool end compile
	fi
}

do_install() {
	echo ${D}
   
	if test "${COMBO_CHIP_ID}" = "mt7668"; then
		install -d ${D}/usr/sbin/
		install -m 0755 ${S}/${MODULE_NAME} ${D}/usr/sbin
	fi
}

FILES_${PN} += "/usr/sbin"
FILES_${PN}-dev = ""
