FILESEXTRAPATHS_append := ":${THISDIR}/${PN}"

SRC_URI += " \
	file://0001-gstreamer-upgrade-bad-plugins.patch \
"
