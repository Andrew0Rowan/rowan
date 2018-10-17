DESCRIPTION = "Package group used by Mediatek to provide all GStreamer plugins from the \
base, good, and bad packages, as well as the ugly and libav ones if commercial packages \
are whitelisted, and plugins for the required hardware acceleration (if supported by the SoC)."
SUMMARY = "Mediatek package group - full set of all GStreamer 1.0 plugins"
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://${COREBASE}/LICENSE;md5=4d92cd373abda3937c2bc47fbc49d690 \
                    file://${COREBASE}/meta/COPYING.MIT;md5=3da9cfbcb788c80a0384361b4de20420"

PACKAGE_ARCH_mt2701 = "${MACHINE_ARCH}"

inherit packagegroup

RDEPENDS_${PN} = " \
    gstreamer1.0-plugins-base \
    gstreamer1.0-plugins-good \
    gstreamer1.0-plugins-bad \
    gstreamer1.0-mtkwaylandsink \
    gstreamer1.0-mtkmdp \
    gstreamer1.0-mtkhuffman \
    ${@base_contains('LICENSE_FLAGS_WHITELIST', 'commercial', 'gstreamer1.0-plugins-ugly', '', d)} \
    ${@base_contains('LICENSE_FLAGS_WHITELIST', 'commercial', 'gstreamer1.0-libav', '', d)} \
"
