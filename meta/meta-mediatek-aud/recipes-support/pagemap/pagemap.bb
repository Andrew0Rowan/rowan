DESCRIPTION = "pagemap"
LICENSE = "GPLv2"
LIC_FILES_CHKSUM = "file://COPYING;md5=d7810fab7487fb0aad327b76f1be7cd7"

inherit workonsrc
WORKONSRC = "${TOPDIR}/../src/support/pagemap"
PACKAGES = "pagemap"

ALLOW_EMPTY_${PN} = '1'

do_compile() {
	echo "pagemap start compile"
	echo ${WORKONSRC}
	echo ${S}
	cd ${S}
	
        make -j4 SYSROOT_INCLUDE="${TOPDIR}/tmp/sysroots/${MTK_PROJECT}/usr/include"
	
	aarch64-poky-linux-strip --strip-unneeded ${S}/libpagemap.a
}

do_install() {
	install -d ${D}${libdir}
	install -m 755 ${S}/libpagemap.a ${D}${libdir}
}

FILES_${PN}-staticdev += "${libdir}/libportaudio.a"
INSANE_SKIP_${PN} += "already-stripped installed-vs-shipped"
