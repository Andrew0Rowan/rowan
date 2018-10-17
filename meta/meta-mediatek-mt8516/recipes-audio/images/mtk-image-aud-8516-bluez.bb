IMAGE_INSTALL = "packagegroup-core-boot-aud ${ROOTFS_PKGMANAGE_BOOTSTRAP} ${CORE_IMAGE_EXTRA_INSTALL}"

IMAGE_LINGUAS = " "

LICENSE = "MIT"

inherit core-image
inherit create-link
inherit create-mtdverity
inherit mkusrdata
inherit pack-update-zip

CORE_IMAGE_EXTRA_INSTALL += " \
    packagegroup-mtk-upgrade-kit-native \
"

IMAGE_FEATURES_append = " \
	${@base_contains('ENABLE_MTD_VERITY', 'yes', 'read-only-rootfs', '' ,d)} \
"

IMAGE_INSTALL_append = " \
    openssl \
    wpa-supplicant \
    alsa-utils \
    alsa-lib \
    wpa-supplicant-passphrase \
    mtkcombotool\
    mtkwlan \
    mtkcombo \
    ${@base_contains('LICENSE_FLAGS_BLUEZ', 'yes', 'bluez5', 'bluetooth' ,d)} \
    ${@base_contains('LICENSE_FLAGS_BLUEZ', 'yes', 'boots', '' ,d)} \
    fuse \
    curl \
    prog \
    appmainprog \
    ppc \
    ppccli \
    mtk-audio-service \
    mtk-alsa-plugins \
    wifi-mw \
    wlanMon \
    android-tools-adbd \
    iperf \
    mtd-utils \
    mtd-utils-ubifs \
    custom \
    nvram \
    libnvramcustom \
    upgrade_app \
    tcpdump \
    iptables \
    buildinfo \
    tzapp \
    coreutils \
    data-ubi-mnt \
    ble-mesh \
    smtcn-mw \
"

