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
    ${@base_contains('LICENSE_FLAGS_GMRENDER', 'yes', 'packagegroup-mtk-gstreamer1.0', '' ,d)} \
"

IMAGE_FEATURES_append = " \
	${@base_contains('ENABLE_MTD_VERITY', 'yes', 'read-only-rootfs', '' ,d)} \
"

IMAGE_INSTALL_append = " \
    ${@base_contains('LICENSE_FLAGS_FFMPEG', 'yes', 'ffmpeg', '' ,d)} \
    ${@base_contains('LICENSE_FLAGS_AISPEECH', 'yes', 'aispeech', '' ,d)} \
    ${@base_contains('LICENSE_FLAGS_GMRENDER', 'yes', 'gmediarender', '' ,d)} \
    ${@base_contains('LICENSE_FLAGS_BLUEZ', 'yes', 'bluez5', 'bluetooth' ,d)} \
    ${@base_contains('CLOUD_DUEROS', 'yes', 'dueros-sdk', '' ,d)} \
    ${@base_contains('ALGO_WENZHI', 'yes', 'wenzhi', '',d)} \
    ${@base_contains('ALEXA_AVS', 'yes', 'avs', '', d)} \
    ${@base_contains('ALGO_MPCTEC', 'yes', 'mpctec', '', d)} \
    mDNSResponder \
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
    fuse \
    curl \
    dhcpcd \
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
    guardian \
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
    wireless-tools \
    ntp \
    cjson \
    smtcn-mw \
"

