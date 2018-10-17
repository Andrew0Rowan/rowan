inherit deploy

DESCRIPTION = "Android Verify Boot sign tool from Android external/avb"
LICENSE = "Apache-2.0"
LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/Apache-2.0;md5=89aea4e17d99a7cacdbeed46a0096b10"
SRC_URI = "git://android.googlesource.com/platform/external/avb;protocol=https"
SRCREV = "fd9c18d298af8ffef64d736bf7f0900196af9b7a"

SRC_URI[sha256sum] = "833893127582352d05d925e36a2ba82d7837105ec8915cd82c7ba9abdddcf680"
SRC_URI[md5sum] = "1935b6e637d9413cc48115e7602dec4f"
S = "${WORKDIR}/git"

do_compile () {
}

do_install () {
}

do_deploy () {
	install -d ${DEPLOYDIR}
	install -m 755 ${S}/avbtool -t ${DEPLOYDIR}
}

addtask deploy before do_build after do_compile
