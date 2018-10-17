inherit deploy srcprebuilt externalsrc

LICENSE = "MediaTekProprietary"
MTK_SRC = "${TOPDIR}/../src/bsp/lk"
LK_OUT = "${WORKDIR}/out"
LIC_FILES_CHKSUM = "file://${MTK_SRC}/LICENSE;md5=5a1abdab641eec675725c843f43f03af"
TOOLCHAIN_PREFIX = "${TARGET_PREFIX}"
EXTERNALSRC = "${MTK_SRC}"
EXTERNALSRC_BUILD = "${MTK_SRC}"
LK_BINARY = "lk.bin"
PACKAGE_ARCH = "${MACHINE_ARCH}"
DEPENDS += "libgcc"

python __anonymous () {
    defaulttune = d.getVar('DEFAULTTUNE', True)
    if defaulttune == 'cortexa7hf-neon-vfpv4':
        d.setVar("LIBGCC", '${TOPDIR}/../prebuilt/bsp/lk/${LK_PROJECT}/libgcc.a')
    else:
        d.setVar("LIBGCC", '$(${CC} -print-libgcc-file-name)')
}

do_configure () {
	:
}

do_compile () {
	oe_runmake TOOLCHAIN_PREFIX=${TOOLCHAIN_PREFIX} \
                   NOECHO="" \
                   BOOTLOADER_OUT=${LK_OUT} \
                   FULL_PROJECT=${FULL_PROJECT} \
                   USE_ITS_BOOTIMG=yes \
                   LIBGCC=${LIBGCC} \
                   LK_PROJECT=${LK_PROJECT} \
                   LK_BINARY=${LK_BINARY} \
                   ${LK_PROJECT}
}

do_deploy () {
	install -d ${DEPLOYDIR}
	install ${LK_OUT}/build-${LK_PROJECT}/${LK_BINARY} ${DEPLOYDIR}/${LK_BINARY}

}

addtask deploy before do_build after do_compile
