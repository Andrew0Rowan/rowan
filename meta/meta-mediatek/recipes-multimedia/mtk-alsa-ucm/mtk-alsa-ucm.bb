DESCRIPTION = "Mediatek ALSA UCM Configration"
LICENSE = "MediaTekProprietary"
LIC_FILES_CHKSUM = "file://README;md5=169c5a78ab6dd37dd1e18c95f6ff67b2"
SRC_URI = "file://README \
           file://alsa.conf.d/mtk_phonecall.conf \
"

S = "${WORKDIR}"
FILES_${PN} = "${datadir}/alsa/ucm/*/*.conf"
FILES_${PN} += "${datadir}/alsa/alsa.conf.d/*.conf"

do_install () {
	install -d ${D}${datadir}/alsa/alsa.conf.d
	install -m 644 ${S}/alsa.conf.d/*.conf ${D}${datadir}/alsa/alsa.conf.d
}
