FILESEXTRAPATHS_append := ":${THISDIR}/files"

do_install(){
	install -d ${D}${sbindir}
	install -m 0755 ${S}/iwpriv ${D}${sbindir}
}
