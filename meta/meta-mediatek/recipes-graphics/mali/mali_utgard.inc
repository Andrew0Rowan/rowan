DESCRIPTION = "Mali-450 GPU"
LICENSE = "MediaTekProprietary"
LIC_FILES_CHKSUM = "file://NOTICE;md5=4c46d84528c2908a46ac79d54284016d"
#inherit externalsrc
#EXTERNALSRC = "${TOPDIR}/../prebuilt/graphics/mali/utgard"
#EXTERNALSRC_BUILD = "${TOPDIR}/../prebuilt/graphics/mali/utgard"
inherit workonsrc
WORKONSRC = "${TOPDIR}/../src/graphics/mali/utgard"

DEPENDS = "libdrm wayland"
PROVIDES = "virtual/egl virtual/libgles1 virtual/libgles2 virtual/libgl virtual/mesa"
RPROVIDES_${PN} = "libgles2"

inherit pkgconfig

do_compile() {
	oe_runmake mali_ver="${mali_ver}" -f Makefile.mediatek
}

do_install() {
	oe_runmake mali_ver="${mali_ver}" -f Makefile.mediatek \
	           PREFIX="${prefix}" DESTDIR="${D}" SRCDIR="${B}" PACKAGE_ARCH="${PACKAGE_ARCH}" install
}

do_install_append() {
	ln -nfs libMali.so ${D}${libdir}/libMali.so.1

        ln -nfs libMali.so ${D}${libdir}/libEGL.so
        ln -nfs libMali.so ${D}${libdir}/libEGL.so.1
        ln -nfs libMali.so ${D}${libdir}/libEGL.so.1.0.0
        ln -nfs libMali.so ${D}${libdir}/libEGL.so.1.4

        ln -nfs libMali.so ${D}${libdir}/libgbm.so
        ln -nfs libMali.so ${D}${libdir}/libgbm.so.1
        ln -nfs libMali.so ${D}${libdir}/libgbm.so.1.0.0

        ln -nfs libMali.so ${D}${libdir}/libGLESv1_CM.so
        ln -nfs libMali.so ${D}${libdir}/libGLESv1_CM.so.1
        ln -nfs libMali.so ${D}${libdir}/libGLESv1_CM.so.1.1
        ln -nfs libMali.so ${D}${libdir}/libGLESv1_CM.so.1.1.0

        ln -nfs libMali.so ${D}${libdir}/libGLESv2.so
        ln -nfs libMali.so ${D}${libdir}/libGLESv2.so.2
        ln -nfs libMali.so ${D}${libdir}/libGLESv2.so.2.0.0

        ln -nfs libMali.so ${D}${libdir}/libwayland-egl.so
        ln -nfs libMali.so ${D}${libdir}/libwayland-egl.so.1
        ln -nfs libMali.so ${D}${libdir}/libwayland-egl.so.1.0.0
}

RDEPENDS_${PN}-dev = ""

FILES_${PN} = "${libdir}/libMali.so* \
               ${libdir}/libGLESv2.so* \
               ${libdir}/libEGL.so* \
               ${libdir}/libgbm.so* \
               ${libdir}/libGLESv1_CM.so* \
               ${libdir}/libwayland-egl.so* "

FILES_${PN}-dev = "${libdir}/libMali.la \
                   ${libdir}/libEGL.la \
                   ${libdir}/pkgconfig/egl.pc \
                   ${includedir}/EGL/eglextchromium.h \
                   ${includedir}/EGL/eglext.h \
                   ${includedir}/EGL/egl.h \
                   ${includedir}/EGL/eglmesaext.h \
                   ${includedir}/EGL/eglplatform.h \
                   ${libdir}/libgbm.la \
                   ${libdir}/pkgconfig/gbm.pc \
                   ${includedir}/gbm.h \
                   ${libdir}/libGLESv2.la \
                   ${libdir}/pkgconfig/glesv2.pc  \
                   ${includedir}/GLES2/gl2ext.h  \
                   ${includedir}/GLES2/gl2.h  \
                   ${includedir}/GLES2/gl2platform.h \
                   ${includedir}/GLES3/gl3ext.h \
                   ${libdir}/libGLESv1_CM.la \
                   ${libdir}/pkgconfig/glesv1_cm.pc \
                   ${includedir}/GLES/egl.h \
                   ${includedir}/GLES/glext.h \
                   ${includedir}/GLES/gl.h \
                   ${includedir}/GLES/glplatform.h \
                   ${libdir}/libwayland-egl.la \
                   ${libdir}/pkgconfig/wayland-egl.pc \
                   ${includedir}/KHR/khrplatform.h"

INHIBIT_PACKAGE_DEBUG_SPLIT = "1"
INSANE_SKIP_${PN} += "dev-so"
INSANE_SKIP_${PN} += "dev-deps"
INSANE_SKIP_${PN} += "already-stripped"

