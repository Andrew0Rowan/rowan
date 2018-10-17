FILESEXTRAPATHS_append := ":${THISDIR}/files"

SRC_URI += "file://services \
	    file://localtime \
"

FILES_${PN} += "/etc"

do_install_append(){
	install -d ${D}/etc
	install -m 0755 ${S}/../localtime ${D}/etc
	install -m 0755 ${S}/../services ${D}/etc
}
