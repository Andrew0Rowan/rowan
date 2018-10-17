require ../poky/meta/recipes-graphics/images/core-image-weston.bb

IMAGE_LINGUAS = " "

LICENSE = "MIT"

inherit mkstateimage
inherit slimko
inherit create-link
inherit create-nfsb
inherit create-sparse-image

do_rootfs[depends] += "mtk-core-image-recovery-initramfs:do_image_complete"

CORE_IMAGE_EXTRA_INSTALL += " \
    packagegroup-mtk-gstreamer1.0 \
    packagegroup-gstreamer1.0-full \
    packagegroup-mtk-upgrade-kit-native \
    packagegroup-mtk-upgrade-kit \
"

IMAGE_INSTALL_append = " \
    energy-aware \
    evtest \
    fb-test \
    i2c-tools \
    libmtp \
    mali \
    mtk-alsa-ucm \
    mtd-utils \
    udev-extraconf \
    usbutils \
    util-linux \
    ${@base_contains("TARGET_PLATFORM", "mt2701", "vpud", "", d)} \
    ${@base_contains("TARGET_PLATFORM", "mt2712", "lib32-vpud", "", d)} \
    mdpd \
    mtk-ovl-adapter \
    tzapp \
    boot-assist \
    tzdata \
    alsa-utils \
    pulseaudio-server \
    pulseaudio-misc \
"
CORE_IMAGE_BASE_INSTALL_remove = "weston-init weston-ini-conf weston-examples gtk+3-demo clutter-1.0-examples"

install_pulseaudio_service_to_profile() {
    echo "pulseaudio --start" > ${IMAGE_ROOTFS}${ROOT_HOME}/.profile
}

ROOTFS_POSTPROCESS_COMMAND += " install_pulseaudio_service_to_profile;"
