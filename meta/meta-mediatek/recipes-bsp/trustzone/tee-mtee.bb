inherit workonsrc
inherit deploy trustzone-build

DESCRIPTION = "MTEE Image"
LICENSE = "MediaTekProprietary"
TZ_SRC = "${TOPDIR}/../src/bsp/trustzone"
TZ_PREBUILT = "${TOPDIR}/../prebuilt/bsp/trustzone"
WORKONSRC = "${TZ_SRC}/mtee/build"
WORKONSRC_BUILD = "${TZ_SRC}/mtee/build"
LIC_FILES_CHKSUM = "file://${TZ_SRC}/mtee/build/yocto_build/README;md5=5a126b0ba82af703f1c30cf8d0bb4e13"
TZ_OUT = "${WORKDIR}/out"
TZ_BINARY_OUT = "${TZ_OUT}/bin"
TZ_RAW_OUT = "${TZ_OUT}/archive"
TZ_BINARY = "tz.img"
PACKAGE_ARCH = "${MACHINE_ARCH}"
do_compile[nostamp] = "1"

do_compile () {
if [ "${TEE_ARCH}" = "" ]; then
	TEE_ARCH=${KERNEL_ARCH}
fi
if [ -e ${TZ_SRC}/mtee/build/yocto_build/makefile ] && [ -d ${TZ_SRC}/mtee/source/${TZ_PLATFORM} ]; then
	python 	${TZ_SRC}/mtee/build/yocto_build/android_makefile_parser.py \
		${TZ_SRC}/mtee \
		${TZ_PLATFORM} \
		${TZ_PROJECT} \
		${TEE_ARCH} \
		${TZ_SRC}/mtee/trustzone.mk \
		${TZ_PREBUILT}/${TZ_PROJECT}/prebuilts

		TZ_SRC=${TZ_SRC} \
		TZ_OUT=${TZ_OUT} \
		ARCH=${TEE_ARCH} \
		MTK_PLATFORM=${TZ_PLATFORM} \
		MTK_PROJECT=${TZ_PROJECT} \
		CROSS_COMPILE=${TARGET_PREFIX} \
		make -C ${TZ_SRC}/mtee -f ${TZ_SRC}/mtee/build/yocto_build/makefile
fi
}

do_deploy () {
	install -d ${DEPLOYDIR}
	if [ -e ${TZ_SRC}/mtee/build/yocto_build/makefile ] && [ -d ${TZ_SRC}/mtee/source/${TZ_PLATFORM} ]; then
		install ${TZ_RAW_OUT}/${TZ_BINARY} ${TZ_ASSEMBLE_OUT}/${TZ_RAW_BINARY}
		install ${TZ_BINARY_OUT}/${TZ_SIGNED_BINARY} ${TZ_ASSEMBLE_OUT}/${TZ_SIGNED_BINARY}
		install ${TZ_RAW_OUT}/${TZ_BINARY} ${TZ_ASSEMBLE_OUT}/${TRUSTEDOS_RAW_BINARY}
		install ${TZ_BINARY_OUT}/${TZ_SIGNED_BINARY} ${TZ_ASSEMBLE_OUT}/${TRUSTEDOS_SIGNED_BINARY}
	else
		if [ "${TRUSTZONE_HEADER}" = "fit" ]; then
			echo "fit image case"
			echo ${TZ_PREBUILT}/${TZ_PROJECT}/${TZ_RAW_BINARY} ${TZ_ASSEMBLE_OUT}/${TZ_RAW_BINARY}
			install ${TZ_PREBUILT}/${TZ_PROJECT}/${TZ_RAW_BINARY} ${TZ_ASSEMBLE_OUT}/${TZ_RAW_BINARY}
			install ${TZ_PREBUILT}/${TZ_PROJECT}/${TZ_SIGNED_BINARY} ${TZ_ASSEMBLE_OUT}/${TZ_SIGNED_BINARY}
			install ${TZ_PREBUILT}/${TZ_PROJECT}/${TZ_RAW_BINARY} ${TZ_ASSEMBLE_OUT}/${TRUSTEDOS_RAW_BINARY}
			install ${TZ_PREBUILT}/${TZ_PROJECT}/${TZ_SIGNED_BINARY} ${TZ_ASSEMBLE_OUT}/${TRUSTEDOS_SIGNED_BINARY}
		else
			echo "no fit image case"
			install ${TZ_PREBUILT}/${TZ_PROJECT}/${TZ_BINARY} ${TZ_ASSEMBLE_OUT}/${TZ_BINARY}
		fi
	fi
}

addtask deploy before do_build after do_install
