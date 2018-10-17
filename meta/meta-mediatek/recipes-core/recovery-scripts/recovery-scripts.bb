DESCRIPTION = "An Example for Recovery Mode"
LICENSE = "MediaTekProprietary"
LIC_FILES_CHKSUM = "file://README;md5=6b2da25093d94b7c2bd0b5ce697f27b9"
SRC_URI = "file://README \
           file://recovery.sh \
"
S = "${WORKDIR}"
FILES_${PN} = "${bindir}/recovery.sh"

do_install () {
        install -d ${D}${bindir}
        install -m 755 ${S}/recovery.sh ${D}${bindir}/recovery.sh
}

