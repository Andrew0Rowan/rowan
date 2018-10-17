#Basic Configuration
DESCRIPTION = "spmeta"
SECTION = "base"
LICENSE = "MediaTekProprietary"
LIC_FILES_CHKSUM = "file://LICENSE;md5=e1696b147d49d491bcb4da1a57173fff"
MTK_SRC = "${TOPDIR}/../src/devtools/sp_meta"
SP_PREFIX = "${STAGING_INCDIR}/install"
LINUX_KERNEL = "linux-4.4"

WORKONSRC = "${MTK_SRC}"
BB_CFLAGS_ADD = "--sysroot=${STAGING_DIR_HOST} -g"
BB_LDFLAGS_ADD = "--sysroot=${STAGING_DIR_HOST}"

BB_INCLUDE_ADD  = "-I${MTK_SRC}/common/inc \
                   -I${STAGING_DIR_HOST}  \
                   -I${STAGING_INCDIR} \
                   "

SRC_URI = "file://enableusb.sh \
          "

inherit deploy workonsrc

#Parameters passed to do_compile()


EXTRA_OEMAKE = "CROSS=${TARGET_PREFIX} \
                PREFIX=${SP_PREFIX} \
                PACKAGE_ARCH=${PACKAGE_ARCH} \
                BB_INCLUDE_ADD=${BB_INCLUDE_ADD} \
                BB_LDFLAGS_ADD=${BB_LDFLAGS_ADD} \
                LINUX_KERNEL=${LINUX_KERNEL}"

FILES_${PN} = "${base_libdir}/*.so\
               ${base_bindir}\
               ${base_sbindir}\
               /mnt\
               /tmp\
               /etc\
               /test\
               /usr/bin/enableusb.sh"

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
    if test "${PACKAGE_ARCH}" = "cortexa7hf-vfp-vfpv4-neon" || test "${PACKAGE_ARCH}" = "cortexa7hf-neon-vfpv4"; then
        oe_runmake all ROOT=${STAGING_DIR_HOST} CFLAGS="${BB_CFLAGS_ADD} -mhard-float"
    else
        oe_runmake all ROOT=${STAGING_DIR_HOST} CFLAGS="${BB_CFLAGS_ADD}"
    fi
}

do_install () {
    oe_runmake install ROOT=${D}

    if [ -d "${D}/include" ]; then
        install -d ${D}${includedir}
        cp -af ${D}/include/* ${D}${includedir}
        rm -rf ${D}/include
    fi
}
inherit systemd
SYSTEMD_PACKAGES = "${PN}"
SYSTEMD_SERVICE_${PN} = "keymonitor.service"
FILES_${PN} += "${systemd_unitdir}/system/keymonitor.service"
do_install_append(){
    install -d ${D}/usr/bin
    install -m 0755 ${WORKDIR}/enableusb.sh ${D}/usr/bin/enableusb.sh
    if ${@bb.utils.contains('DISTRO_FEATURES','systemd','true','false',d)}; then
        install -d ${D}${systemd_unitdir}/system/
        install -m 0644 ${B}/lib/systemd/system/keymonitor.service ${D}${systemd_unitdir}/system
    fi
}
addtask bachclean
do_bachclean () {
    oe_runmake clean
}
