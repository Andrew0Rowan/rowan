# Simple initramfs image. Mostly used for live images.
DESCRIPTION = "Small image capable of booting a device. The kernel includes \
the Minimal RAM-based Initial Root Filesystem (initramfs), which finds the \
first 'init' program more efficiently."

PACKAGE_INSTALL = " \
                   initramfs-live-boot \
                   busybox \
                   udev \
                   base-passwd \
                   recovery-scripts \
                   ${ROOTFS_BOOTSTRAP_INSTALL} \
"


# Do not pollute the initrd image with rootfs features
IMAGE_FEATURES = ""

export IMAGE_BASENAME = "mtk-core-image-recovery-initramfs"
IMAGE_LINGUAS = ""

LICENSE = "MIT"

IMAGE_FSTYPES = "${INITRAMFS_FSTYPES}"
inherit core-image

IMAGE_ROOTFS_SIZE = "8192"

BAD_RECOMMENDATIONS += "busybox-syslog"

do_rootfs[depends] += "virtual/kernel:do_deploy"

inherit recovery-ramdisk-kernel-fitimage
