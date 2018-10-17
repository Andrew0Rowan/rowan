FILESEXTRAPATHS_append := ":${THISDIR}/${PN}"

do_install_append() {
	rm -fr ${D}/usr/lib
	install -d ${D}/${libdir}
	install -m 755 ${WORKDIR}/build/lib/libPoco*.so ${D}${libdir}
}

FILES_${PN} += "${libdir}"
INSANE_SKIP_${PN} += "already-stripped"

