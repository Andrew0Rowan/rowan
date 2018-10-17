FILESEXTRAPATHS_append := ":${THISDIR}/files"

SRC_URI_append = " file://dl_addr.ini"
do_deploy_append () {
        install -d ${DEPLOYDIR}
        install -m 0644 ${WORKDIR}/dl_addr.ini -t ${DEPLOYDIR}
}
