inherit kernel externalsrc

DEPENDS += "u-boot-mkimage-native"
DEPENDS_append_aarch64 = " libgcc"
KERNEL_CC_append_aarch64 = " ${TOOLCHAIN_OPTIONS}"
KERNEL_LD_append_aarch64 = " ${TOOLCHAIN_OPTIONS}"

LICENSE = "GPLv2"
LIC_FILES_CHKSUM = "file://COPYING;md5=d7810fab7487fb0aad327b76f1be7cd7"
KERNEL_SRC = "${TOPDIR}/../src/kernel/linux/v4.4"
LINUX_VERSION = "4.4.0"
EXTERNALSRC = "${KERNEL_SRC}"
STAGING_KERNEL_DIR = "${KERNEL_SRC}"
LINUX_VERSION_EXTENSION = "-custom"
KERNEL_EXTRA_ARGS = "dtbs"
BOOTIMAGE = "boot.img"

PREBUILT_BSP_DIR = "${TOPDIR}/../prebuilt/bsp/tzapp/"
KERNEL_CONFIG_COMMAND = "oe_runmake_call -C ${S} O=${B} ${KBUILD_DEFCONFIG}"

do_configure_prepend() {
       install -d ${TMPDIR}/work-shared/${MACHINE}
       ln -nfs ${STAGING_KERNEL_DIR} ${TMPDIR}/work-shared/${MACHINE}/kernel-source
}

do_uboot_mkimage() {
		cat ${B}/arch/${KERNEL_ARCH}/boot/Image | gzip -n -f -9 > ${B}/arch/${KERNEL_ARCH}/boot/Image.gz
		cat ${B}/arch/${KERNEL_ARCH}/boot/Image.gz ${B}/arch/${KERNEL_ARCH}/boot/dts/${KERNEL_DTB} > ${B}/arch/${KERNEL_ARCH}/boot/Image.gz-dtb
		${PREBUILT_BSP_DIR}/host/mkimage ${B}/arch/${KERNEL_ARCH}/boot/Image.gz-dtb KERNEL 0xffffffff > ${B}/arch/${KERNEL_ARCH}/boot/Image.gz-dtb.bin
		${PREBUILT_BSP_DIR}/host/acp -fp ${B}/arch/${KERNEL_ARCH}/boot/Image.gz-dtb.bin ${B}/arch/${KERNEL_ARCH}/boot/kernel
		${PREBUILT_BSP_DIR}/host/mkbootimg  --kernel ${B}/arch/${KERNEL_ARCH}/boot/kernel --ramdisk ${PREBUILT_BSP_DIR}/../collect/emptyramdisk --cmdline "bootopt=64S3,32N2,64N2" --base 0x40000000 --kernel_offset 0x00080000 --ramdisk_offset 0x04000000 --tags_offset 0xE000000 --output ${B}/${BOOTIMAGE}
}

do_deploy () {
       install -d ${DEPLOYDIR}
       install ${B}/${BOOTIMAGE} ${DEPLOYDIR}/${BOOTIMAGE}
       install ${PREBUILT_BSP_DIR}/${MTK_PROJECT}/* ${DEPLOYDIR}
}

addtask deploy before do_build after do_compile

