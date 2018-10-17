DESCRIPTION = "procrank"
LICENSE = "GPLv2"
LIC_FILES_CHKSUM = "file://COPYING;md5=d7810fab7487fb0aad327b76f1be7cd7"
#SRC_URI = "https://github.com/nghttp2/nghttp2/releases/download/v1.24.0/nghttp2-1.24.0.tar.gz"
DEPENDS = "pagemap"


inherit workonsrc
WORKONSRC = "${TOPDIR}/../src/support/procrank"
PACKAGES = "procrank"

do_compile() {
	echo "procrank start compile"
	echo ${WORKONSRC}
	echo ${S}
	cd ${S}
	
        make -j4 LIB_DIR="${TOPDIR}/tmp/sysroots/${MTK_PROJECT}/usr/lib64"
	
	aarch64-poky-linux-strip --strip-unneeded ${S}/procrank
}

do_install() {
	install -d ${D}${libdir}
	install -m 755 ${S}/procrank ${D}${libdir}
}

FILES_${PN} += "${libdir}/procrank"
INSANE_SKIP_${PN} += "already-stripped installed-vs-shipped ldflags"
