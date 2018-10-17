FILESEXTRAPATHS_append := ":${THISDIR}/${PN}"

SRC_URI += " \
	file://0001-libav-modify-some-sw-decoders-rank-from-64-to-0.patch"
