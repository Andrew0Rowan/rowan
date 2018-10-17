DESCRIPTION = "Aispeech prebuilt"
LICENSE = "Aispeech"

APPS_SRC = "${TOPDIR}/../prebuilt/support/assistant-sdk/aispeech/"

inherit workonsrc

WORKONSRC = "${APPS_SRC}"

SYSTEMD_PACKAGES = "${PN}"

do_install() {
	install -d ${D}/${libdir}
	install -m 0644 ${S}/so/${base_libdir}/*.so ${D}/${libdir}
  install -d ${D}/res
	install -d ${D}/res/aispeech 
	install -d ${D}/res/aispeech/cfg
	install -d ${D}/res/aispeech/param
	install -d ${D}/res/aispeech/authfile
	install -d ${D}/res/aispeech/res
	install -d ${D}/res/aispeech/res/circle
	install -d ${D}/res/aispeech/res/vad

	install -m 755 ${S}/cfg/* ${D}/res/aispeech/cfg
	install -m 755 ${S}/param/* ${D}/res/aispeech/param
	install -m 755 ${S}/authfile/* ${D}/res/aispeech/authfile
	install -m 755 ${S}/res/circle/* ${D}/res/aispeech/res/circle
	install -m 755 ${S}/res/vad/* ${D}/res/aispeech/res/vad
}

FILES_${PN} += "${libdir} res/ aispeech/"
INSANE_SKIP_${PN} += "already-stripped"
FILES_${PN}-dev = ""
