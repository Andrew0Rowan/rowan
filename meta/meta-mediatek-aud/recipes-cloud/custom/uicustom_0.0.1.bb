LICENSE = "MediaTekProprietary"
DESCRIPTION = "User interactions to customize"

inherit cmake workonsrc
WORKONSRC = "${TOPDIR}/../src/cloud/custom"

FILES_${PN} += "${libdir}"
FILES_${PN}-dev = "${includedir}"
