inherit deploy trustzone-build

DESCRIPTION = "TEELOADER Image"
LICENSE = "MediaTekProprietary"
TZ_SRC = "${TOPDIR}/../src/bsp/trustzone"
TL_SRC = "${TOPDIR}/../src/bsp/trustzone/teeloader"
TZ_PREBUILT = "${TOPDIR}/../prebuilt/bsp/trustzone"
LIC_FILES_CHKSUM = "file://${TL_SRC}/README;md5=5a126b0ba82af703f1c30cf8d0bb4e13"
TL_OUT = "${WORKDIR}/out"
TL_RAW_OUT = "${TL_OUT}/archive_tl"
TZ_ORIG_KEY = "${MTK_KEY_DIR}/${MTEE_KEY}.pem"
PACKAGE_ARCH = "${MACHINE_ARCH}"
DEPLOY_DIR_IMAGE = "${DEPLOY_DIR}/images/${MACHINE}/trustzone"
do_compile[nostamp] = "1"

do_compile () {
if [ ${SECURE_BOOT_ENABLE} = "yes" ]; then
	if [ -d ${TL_SRC} ]; then
		if [ ${FORCE_DISABLE_TEE_ENCRYPTION} = "yes" ]; then
			TL_VERIFY_ENABLE="0x0"
		else
			TL_VERIFY_ENABLE="0x1"
		fi
		if [ -e ${TZ_ORIG_KEY} ] && [ ${FORCE_DISABLE_TEE_ENCRYPTION} != "yes" ]; then
			cp -rf ${TZ_ORIG_KEY} ${TL_SRC}/${TZ_PLATFORM}/include/mtee_key.pem
			python ${TL_SRC}/${TZ_PLATFORM}/cus_tzimg_dec_key.py
			python ${TL_SRC}/${TZ_PLATFORM}/cus_tzimg_enc_key.py ${TZ_PLATFORM}
			rm -rf ${TL_SRC}/${TZ_PLATFORM}/include/mtee_key.pem
		fi
	else
		TL_VERIFY_ENABLE="0x0"
		TL_ALIGN_SIZE="0x0"
	fi
else
	TL_VERIFY_ENABLE="0x0"
fi

if [ -d ${TL_SRC} ]; then
	TL_RAW_OUT=${TL_RAW_OUT} \
	BASE_ADDR=${TEE_LOADADDRESS} \
	TL_ALIGN_SIZE=${TL_ALIGN_SIZE} \
	CROSS_COMPILE=${TARGET_PREFIX} \
	TL_VERIFY_ENABLE=${TL_VERIFY_ENABLE} \
	TRUSTEDOS_ENTRYPOINT=${TRUSTEDOS_ENTRYPOINT} \
	TZ_PROJECT=${TZ_PROJECT} \
	make -C ${TL_SRC}/${TZ_PLATFORM} -f ${TL_SRC}/${TZ_PLATFORM}/Makefile
fi
}

do_deploy () {
if [ -d ${TL_SRC} ]; then
	install -d ${DEPLOYDIR}
	install ${TL_RAW_OUT}/bin/teeloader.bin ${DEPLOYDIR}/teeloader.bin
fi
}

addtask deploy before do_build after do_install
