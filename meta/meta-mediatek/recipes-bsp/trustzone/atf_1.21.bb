inherit atfsrcprebuilt
inherit deploy trustzone-build

DESCRIPTION = "ARM trusted firmware"
LICENSE = "BSD"
ATF_SRC = "${TOPDIR}/../src/bsp/trustzone/atf"
ATF_PREBUILT = "${TOPDIR}/../prebuilt/bsp/atf"
LIC_FILES_CHKSUM = "file://${ATF_SRC}/v1.21/license.md;md5=829bdeb34c1d9044f393d5a16c068371"
PACKAGE_ARCH = "${MACHINE_ARCH}"
ATF_OUT = "${WORKDIR}/out"
ATF_OUT_BINARY = "${ATF_OUT}/${ATF_INSTALL_MODE}/bl31.bin"
DEPLOY_DIR_IMAGE = "${DEPLOY_DIR}/images/${MACHINE}/trustzone"

EXTRA_OEMAKE = "'CROSS_COMPILE=${TARGET_PREFIX}'	\
		'KERNEL_ARCH=${KERNEL_ARCH}' \
		'PLAT=${MTK_MACH_TYPE}'	\
		'MACH_TYPE=${MTK_MACH_TYPE}'	\
		'SECURE_OS=${TEE_SUPPORT}'	\
		'BUILD_BASE=${ATF_OUT}'"

CFLAGS[unexport] = "1"
LDFLAGS[unexport] = "1"
AS[unexport] = "1"
LD[unexport] = "1"
do_compile[nostamp] = "1"

do_compile () {
	if [ -e ${ATF_SRC}/v1.21/Makefile ]; then
        oe_runmake -C ${ATF_SRC}/v1.21 -f ${ATF_SRC}/v1.21/Makefile
	fi
}

do_deploy () {
	install -d ${DEPLOYDIR}
	if [ -e ${ATF_SRC}/v1.21/Makefile ]; then
	install ${ATF_OUT_BINARY} ${DEPLOYDIR}/${ATF_RAW_BINARY}
	install ${ATF_OUT_BINARY} ${DEPLOYDIR}/${ATF_SIGNED_BINARY}
	else
	install ${ATF_PREBUILT}/atf_raw_${KERNEL_ARCH}.img ${DEPLOYDIR}/${ATF_RAW_BINARY}
	install ${ATF_PREBUILT}/atf_signed_${KERNEL_ARCH}.img ${DEPLOYDIR}/${ATF_SIGNED_BINARY}
	fi
}

addtask deploy before do_build after do_install
