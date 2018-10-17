DESCRIPTION = "Mediatek FlashScript"
LICENSE = "MediaTekProprietary"
LIC_FILES_CHKSUM = "file://${WORKDIR}/README;md5=ecf62296074513b19f1c6e1ae1bd704a"
SRC_URI = "file://README file://flashimage.py file://ubuntu_flashimage.sh"
BBCLASSEXTEND += "native"

inherit deploy

do_deploy () {
	install -d ${DEPLOYDIR}
	install -m 755 ${WORKDIR}/flashimage.py -t ${DEPLOYDIR}
	install -m 755 ${WORKDIR}/ubuntu_flashimage.sh ${DEPLOYDIR}
}

addtask deploy before do_build after do_compile
