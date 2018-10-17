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
    read-only-rootfs      \
"
IMAGE_INSTALL_append = " \
    ${@base_contains('LICENSE_FLAGS_WHITELIST', 'commercial', 'ffmpeg', '' ,d)} \
    ${@base_contains('LICENSE_FLAGS_AISPEECH', 'yes', 'aispeech', '' ,d)} \
    mDNSResponder \
    coverity-trap \
    openssl \
    hostapd \
    wpa-supplicant \
    alsa-utils \
    alsa-lib \
    dhcp-server-config \
    wpa-supplicant-passphrase \
    mtkcombotool\
    mtkwlan \
    mtkcombo \
    hostapd \
    bluetooth \
    fuse \
    curl \
    dhcpcd \
    prog \
    appmainprog \
    AssistantCenter \
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
    mtkwifitesttool \
    glibc-gconv-gb18030 \
    glibc-gconv-gbgbk \
    glibc-gconv-gbk \
    glibc-gconv-euc-cn \
    glibc-gconv-libgb \
    utf-gbk \
    dnsmasq \
    wireless-tools \
    ntp \
    cjson \
"

install_public_prog(){
install -d ${IMAGE_ROOTFS}/usr/lib/public

install -d ${IMAGE_ROOTFS}/usr/lib/public/bluetooth_hfp
install -m 755 ${TOPDIR}/../src/apps/aud-base/public/bluetooth_hfp/* ${IMAGE_ROOTFS}/usr/lib/public/bluetooth_hfp
}

ROOTFS_POSTPROCESS_COMMAND += " install_public_prog;"
