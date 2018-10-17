inherit deploy srcprebuilt externalsrc lk-image

LICENSE = "MIT"
MTK_SRC = "${TOPDIR}/../src/bsp/lk"
LK_OUT = "${WORKDIR}/out"
LIC_FILES_CHKSUM = "file://${MTK_SRC}/LICENSE;md5=25394d472e4c06f4d61140e88861fb5b"
SRC_URI = "file://gfh file://dev_info file://pbp file://key file://lk_dts file://dummy_img file://fit-lk"
#S = "${WORKDIR}"
DEPENDS += "u-boot-mkimage-native bc-native"

TOOLCHAIN_PREFIX = "${TARGET_PREFIX}"
EXTERNALSRC = "${MTK_SRC}"
EXTERNALSRC_BUILD = "${MTK_SRC}"
LK_BINARY = "lk.bin"
LK_IMAGE = "lk.img"
PACKAGE_ARCH = "${MACHINE_ARCH}"
DEPENDS += "libgcc"
GFH_DIR = "${WORKDIR}/gfh"
PBP_DIR = "${WORKDIR}/pbp"
KEY_DIR = "${WORKDIR}/key"
DTS_DIR = "${WORKDIR}/lk_dts"
DUMMY_IMG_DIR="${WORKDIR}/dummy_img"

IC_NAME="$(echo ${TARGET_PLATFORM}|tr '[a-z]' '[A-Z]')"
DEV_INFO_HDR_TOOL = "${TOPDIR}/../src/bsp/scatter/scripts/dev-info-hdr-tool.py"

python __anonymous () {
    defaulttune = d.getVar('DEFAULTTUNE', True)
    d.setVar("LIBGCC", '${TOPDIR}/../prebuilt/bsp/lk/${DEFAULTTUNE}/libgcc.a')
}

do_configure () {
	:
}

do_genkey () {
	mkdir -p ${LK_OUT}/include
	if [ "${SECURE_BOOT_ENABLE}" = "yes" ]; then
		mkdir -p ${WORKDIR}/mykeys
		dtc -p 0x3ff ${DTS_DIR}/lk.dts -O dtb -o ${DTS_DIR}/lk.dtb
		cp ${MTK_KEY_DIR}/${VERIFIED_KEY}.crt ${WORKDIR}/mykeys/dev.crt
		cp ${MTK_KEY_DIR}/${VERIFIED_KEY}.pem ${WORKDIR}/mykeys/dev.key
		uboot-mkimage -D "-I dts -O dtb -p 1024" -F -k ${WORKDIR}/mykeys -K ${DTS_DIR}/lk.dtb -r ${DUMMY_IMG_DIR}/fitImage
		python ${WORKDIR}/dev_info/dtb-transfer-array.py ${DTS_DIR}/lk.dtb ${DTS_DIR}/blob.h
		cp ${DTS_DIR}/blob.h ${LK_OUT}/include/blob.h
		rm -rf ${WORKDIR}/mykeys
	else
		cp ${DTS_DIR}/tmp_blob.txt ${LK_OUT}/include/blob.h
	fi
}

do_compile () {
	if [ "${DEFAULTTUNE}" = "aarch64" ]; then
		oe_runmake ARCH_arm64_TOOLCHAIN_PREFIX=${TOOLCHAIN_PREFIX} \
		           NOECHO="" \
		           BUILDROOT=${LK_OUT} \
		           LIBGCC="" \
		           CFLAGS="" \
		           DEBUG=0 \
		           SECURE_BOOT_TYPE=${SECURE_BOOT_TYPE} \
		           AVB_ENABLE_ANTIROLLBACK=${AVB_ENABLE_ANTIROLLBACK} \
		           ${LK_PROJECT}
	else
		oe_runmake TOOLCHAIN_PREFIX=${TOOLCHAIN_PREFIX} \
		           NOECHO="" \
		           BUILDROOT=${LK_OUT} \
		           LIBGCC=${LIBGCC} \
		           CFLAGS="" \
		           DEBUG=0 \
		           SECURE_BOOT_TYPE=${SECURE_BOOT_TYPE} \
		           AVB_ENABLE_ANTIROLLBACK=${AVB_ENABLE_ANTIROLLBACK} \
		           ${LK_PROJECT}
	fi
}

do_genheader () {
	if [ "${FIT_LK_IMAGE}" = "yes" ]; then
		gen_lk_fit_header
	else
		gen_lk_gfh_header
	fi
}

do_deploy () {
	install -d ${DEPLOYDIR}
	install ${LK_OUT}/build-${LK_PROJECT}/${LK_BINARY} ${DEPLOYDIR}/${LK_BINARY}
	install ${WORKDIR}/${LK_IMAGE} ${DEPLOYDIR}/${LK_IMAGE}
}

addtask genkey before do_compile after do_configure
addtask genheader before do_deploy after do_compile
addtask deploy before do_build after do_compile

