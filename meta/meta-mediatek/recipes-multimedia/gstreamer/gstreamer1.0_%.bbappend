
EXTRA_OECONF += "--with-memory-alignment=pagesize"

FILESEXTRAPATHS_append := ":${THISDIR}/${PN}"

SRC_URI += " \
	file://0001-base-parse-add-to-support-drop-flag.patch \
	file://0002-gstbuffer-add-mtk-avm-index-interface.patch \
"
