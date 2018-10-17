FILESEXTRAPATHS_append := ":${THISDIR}/files"
SRC_URI_append = " file://${MACHINE}"

do_deploy_append () {
	install -d ${DEPLOYDIR}
	install -m 755 ${WORKDIR}/${MACHINE}/flashproc.py -t ${DEPLOYDIR}
}
