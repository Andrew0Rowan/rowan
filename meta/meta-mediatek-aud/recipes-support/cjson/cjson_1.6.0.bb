SUMMARY = "C bindings for apps which will manipupate JSON data"
DESCRIPTION = "An ultra-lightwight, portable, sigle-file, simple-as-can-be ANSI-C parser, under MIT license"
HOMEPAGE = "https://sourceforge.net/projects/cjson"
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://LICENSE;md5=218947f77e8cb8e2fa02918dc41c50d0"
SRC_URI = "git://github.com/DaveGamble/cJSON.git;protocol=git;rev=27a4303f87473bc3475ff55af06de187cc8234d6"

S = "${WORKDIR}/git"

inherit cmake

INSANE_SKIP_${PN} += "installed-vs-shipped"
