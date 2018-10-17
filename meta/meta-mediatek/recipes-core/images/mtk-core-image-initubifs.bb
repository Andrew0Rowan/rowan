# Simple ubifs image. Only add bootimage in the rootfs
DESCRIPTION = "Small image capable of booting a device. The kernel includes \
the Minimal RAM-based Initial Root Filesystem (initramfs), which finds the \
first 'init' program more efficiently."

PACKAGE_INSTALL = ""

# Do not pollute the initrd image with rootfs features
IMAGE_FEATURES = ""

IMAGE_LINGUAS = ""

LICENSE = "MIT"

IMAGE_FSTYPES = "ubi"
inherit core-image

inherit boot-ubifs
do_rootfs[depends] += "virtual/kernel:do_deploy"
do_rootfs[postfuncs] += "add_boot_image"

IMAGE_ROOTFS_SIZE = "8192"
