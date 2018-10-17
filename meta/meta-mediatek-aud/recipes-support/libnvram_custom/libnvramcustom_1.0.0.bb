#Basic Configuration
DESCRIPTION = "Customization."
SECTION = "base"
LICENSE = "MediaTekProprietary"
LIC_FILES_CHKSUM = "file://LICENSE;md5=e1696b147d49d491bcb4da1a57173fff"
DEPENDS = "custom"
MTK_SRC = "${TOPDIR}/../src/support/libnvram_custom"
NVRAM_PREFIX = "${STAGING_INCDIR}/install"

WORKONSRC = "${MTK_SRC}"
BB_INCLUDE_ADD = "--sysroot=${STAGING_DIR_HOST} -I${TOPDIR}/../src/support/nvram/libnvram"
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
                'LINUX_KERNEL=${LINUX_KERNEL}'\
                'COMBO_CHIP_ID=${COMBO_CHIP_ID}'"

FILES_${PN} = "${base_libdir}/*.so\
               ${base_bindir}\
               ${base_sbindir}\
               /mnt\
               /tmp\
               /etc\
               /test"

FILES_${PN}-dev = "${includedir}"

FILES_${PN}-staticdev = "${base_libdir}/*.a"

FILES_${PN}-doc = "/doc"

FILES_${PN}-dbg = "/usr/src/debug \
				   ${base_bindir}/.debug \
				   ${base_libdir}/.debug \
				   ${base_sbindir}/.debug"


#Skip strip check in QA test.
INSANE_SKIP_${PN} += "already-stripped"


do_compile () {
    unset LDFLAGS
    oe_runmake all ROOT=${STAGING_DIR_HOST}${exec_prefix} BOOTDEV_TYPE=${BOOTDEV_TYPE}
}

do_install () {
    oe_runmake install ROOT=${D}

	if [ -d "${D}/include" ]; then
		install -d ${D}${includedir}
		cp -af ${D}/include/* ${D}${includedir}
		rm -rf ${D}/include
	fi
}

addtask nvramclean
do_nvramclean () {
    oe_runmake clean
}

INSANE_SKIP_${PN} += "ldflags"
