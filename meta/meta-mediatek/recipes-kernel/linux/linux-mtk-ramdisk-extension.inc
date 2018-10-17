inherit kernel externalsrc ramdisk-fitimage-extension
DEPENDS_append_aarch64 = " libgcc"
KERNEL_CC_append_aarch64 = " ${TOOLCHAIN_OPTIONS}"
KERNEL_LD_append_aarch64 = " ${TOOLCHAIN_OPTIONS}"

LICENSE = "GPLv2"
LIC_FILES_CHKSUM = "file://COPYING;md5=d7810fab7487fb0aad327b76f1be7cd7"
EXTERNALSRC = "${KERNEL_SRC}"
STAGING_KERNEL_DIR = "${KERNEL_SRC}"
LINUX_VERSION_EXTENSION = "-custom"
KERNEL_EXTRA_ARGS = "dtbs"
KERNEL_CONFIG_COMMAND = "oe_runmake_call -C ${S} O=${B} ${KBUILD_DEFCONFIG}"

INITRAMFS_IMAGE = "mtk-core-image-minimal-initramfs"
INITRAMFS_IMAGE_BUNDLE = "1"

SRC_IMG_KEY_FILE = "${MTK_KEY_DIR}/${VERIFIED_KEY}.pem"
DST_IMG_KEY_FILE = "${TOPDIR}/../src/devtools/nfsb/rsa.key"

do_configure_prepend() {
	install -d ${TMPDIR}/work-shared/${MACHINE}
	ln -nfs ${STAGING_KERNEL_DIR} ${TMPDIR}/work-shared/${MACHINE}/kernel-source
}

