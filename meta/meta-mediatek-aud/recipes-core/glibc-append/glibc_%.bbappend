FILESEXTRAPATHS_append := ":${THISDIR}/${PN}"

SRC_URI_append = " \
                   file://hook_printf.patch \
				   file://hook_vprintf.patch \
                 "

