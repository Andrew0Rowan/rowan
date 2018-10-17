DESCRIPTION = "sensor hardware abstract layer"
LICENSE = "MediaTekProprietary"
MTK_SRC = "${TOPDIR}/../src/navigation/sensor/sensor_hal"
LIC_FILES_CHKSUM = "file://${MTK_SRC}/LICENSE;md5=e1696b147d49d491bcb4da1a57173fff"

inherit deploy workonsrc
WORKONSRC = "${MTK_SRC}"

SECURITY_CFLAGS = ""

FILES_${PN} = "${bindir}/* ${sbindir}/* ${libexecdir}/* ${libdir}/lib*${SOLIBS}\
               ${sysconfdir} ${sharedstatedir} ${localstatedir}\
               ${base_libdir}/*${SOLIBS}\
               ${base_bindir}/*\
               ${base_sbindir}\
               /usr/lib/*.so*\
               /lib/*.so*"
FILES_SOLIBSDEV ?= "${base_libdir}/lib*${SOLIBSDEV} ${libdir}/lib*${SOLIBSDEV}"
FILES_${PN}-dev = "${includedir} ${FILES_SOLIBSDEV} ${libdir}/*.la \
                   ${libdir}/*.o ${libdir}/pkgconfig ${datadir}/pkgconfig \
                   ${datadir}/aclocal ${base_libdir}/*.o \
                   ${libdir}/${BPN}/*.la ${base_libdir}/*.la "

SECTION_${PN}-dev = "devel"
ALLOW_EMPTY_${PN} = "1"
RDEPENDS_${PN}-dev = "${PN} (=${EXTENDPKGV})"

FILES_${PN}-staticdev = "${libdir}/*.a ${base_libdir}/*.a ${libdir}/${BPN}/*.a"
SECTION_${PN}-staticdev = "devel"
RDEPENDS_${PN}-staticdev = "${PN}-dev (=${EXTENDPKGV})"

FILES_${PN}-doc += "/doc"
FILES_${PN}-dbg += "/usr/src/debug \
                  ${base_bindir}/.debug \
                  ${base_libdir}/.debug \
                  ${base_sbindir}/.debug"

# Avoid QA Issue: 'Files/directories were installed but not shipped'
INSANE_SKIP_${PN} += "installed-vs-shipped"

#Skip strip check in QA test.
INSANE_SKIP_${PN} += "already-stripped"

# inherit showinfo

do_compile() {
  oe_runmake PACKAGE_ARCH=${PACKAGE_ARCH}
}

do_install_append() {
    install -d ${D}${base_libdir}
    install -d ${D}${base_bindir}
    install -d ${D}${base_sbindir}
    install -d ${D}${includedir}
}

inherit autotools
