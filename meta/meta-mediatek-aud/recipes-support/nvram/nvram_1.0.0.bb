#Basic Configuration
DESCRIPTION = "NVRAM."
SECTION = "base"
LICENSE = "MediaTekProprietary"
LIC_FILES_CHKSUM = "file://LICENSE;md5=e1696b147d49d491bcb4da1a57173fff"
DEPENDS = "libnvramcustom"
MTK_SRC = "${TOPDIR}/../src/support/nvram"
NVRAM_PREFIX = "${STAGING_INCDIR}/install"

WORKONSRC = "${MTK_SRC}"
BB_INCLUDE_ADD = "--sysroot=${STAGING_DIR_HOST}"
BB_LDFLAGS_ADD = "--sysroot=${STAGING_DIR_HOST}"

inherit deploy workonsrc

#Parameters passed to do_compile()
EXTRA_OEMAKE = "'CROSS=${TARGET_PREFIX}'\
                'PROJECT=${PROJ}'\
				'PREFIX=${NVRAM_PREFIX}'\
				'PACKAGE_ARCH=${PACKAGE_ARCH}'\
                'PLATFORM_CHIP=${PLATFORM_CHIP}'\
                'BB_INCLUDE_ADD=${BB_INCLUDE_ADD}'\
                'BB_LDFLAGS_ADD=${BB_LDFLAGS_ADD}'\
                'LINUX_KERNEL=${LINUX_KERNEL}'"

FILES_${PN} = "${base_libdir}/*.so\
               ${base_bindir}\
               ${base_sbindir}\
               /mnt\
               /tmp\
               /etc\
               /usr/out\
               /test"

FILES_${PN}-dev = "${includedir} \
                   /out"

FILES_${PN}-staticdev = "${base_libdir}/*.a"

FILES_${PN}-doc = "/doc"

FILES_${PN}-dbg = "/usr/src/debug \
				   ${base_sbindir}/.debug \
				   ${base_libdir}/.debug"

#Skip strip check in QA test.
INSANE_SKIP_${PN} += "already-stripped"

SYSTEMD_PACKAGES = "${PN}"
SYSTEMD_SERVICE_${PN} = "nvram_daemon.service"
FILES_${PN} += "${systemd_unitdir}/system/nvram_daemon.service"

do_compile () {
    unset LDFLAGS
    oe_runmake all ROOT=${STAGING_DIR_HOST}${exec_prefix}
}

do_install () {
    oe_runmake install ROOT=${D}

	if [ -d "${D}/include" ]; then
		install -d ${D}${includedir}
		cp -af ${D}/include/* ${D}${includedir}
		rm -rf ${D}/include
	fi
}

do_install_append() {
	install -d ${D}${systemd_unitdir}/system
	install -m 0755 ${TOPDIR}/../meta/meta-mediatek-aud/recipes-support/nvram/nvram_daemon.service ${D}${systemd_unitdir}/system
}

addtask nvramclean
do_nvramclean () {
    oe_runmake clean
}

INSANE_SKIP_${PN} += "ldflags"
