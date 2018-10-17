DESCRIPTION = "android-tools-fsutils libraries"
LICENSE = "Apache-2.0"
LIC_FILES_CHKSUM = "file://NOTICE;md5=c1a3ff0b97f199c7ebcfdd4d3fed238e"
SRC_URI = " \
        git://android.googlesource.com/platform/system/core;protocol=https \
        file://build.patch \
"
SRCREV = "c6160d2a0ef648ccb3d217c589c60b5c00b80387"
S = "${WORKDIR}/git"
FILES_${PN} = "${bindir}/*"
DEPENDS += "zlib"
BBCLASSEXTEND = "native nativesdk"

inherit autotools
