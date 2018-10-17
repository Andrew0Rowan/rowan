DESCRIPTION = "Mali-880 GPU"
LICENSE = "MediatekProprietary"
LIC_FILES_CHKSUM = "file://${TOPDIR}/../prebuilt/graphics/mali/midgard/NOTICE;md5=4610adf2da9e96774ed48a3e3e0fb18f"
#inherit externalsrc
#EXTERNALSRC = "${TOPDIR}/../prebuilt/graphics/mali/midgard"
#EXTERNALSRC_BUILD = "${TOPDIR}/../prebuilt/graphics/mali/midgard"
inherit workonsrc
WORKONSRC = "${TOPDIR}/../src/graphics/mali/midgard/r13p0"

DEPENDS = "libdrm wayland"
PROVIDES = "virtual/egl virtual/libgles1 virtual/libgles2 virtual/libgl virtual/mesa"
inherit pkgconfig

do_compile() {
	oe_runmake LIB_ROOT=${STAGING_LIBDIR} SYSROOT=${TMPDIR}/sysroots/${MACHINE} DONWLOADS=${TOPDIR}/../downloads
}

do_install() {
        oe_runmake mali_ver="${mali_ver}" \
                PREFIX="${prefix}" DESTDIR="${D}" SRCDIR="${B}" PACKAGE_ARCH="${PACKAGE_ARCH}" LIB=${libdir} install
}

do_install_append() {
        ln -nfs libmali.so ${D}${libdir}/libmali.so.0

        ln -nfs libmali.so ${D}${libdir}/libEGL.so
        ln -nfs libmali.so ${D}${libdir}/libEGL.so.1
        ln -nfs libmali.so ${D}${libdir}/libEGL.so.1.0.0
        ln -nfs libmali.so ${D}${libdir}/libEGL.so.1.4

        ln -nfs libmali.so ${D}${libdir}/libgbm.so
        ln -nfs libmali.so ${D}${libdir}/libgbm.so.1
        ln -nfs libmali.so ${D}${libdir}/libgbm.so.1.0.0

        ln -nfs libmali.so ${D}${libdir}/libGLESv1_CM.so
        ln -nfs libmali.so ${D}${libdir}/libGLESv1_CM.so.1
        ln -nfs libmali.so ${D}${libdir}/libGLESv1_CM.so.1.1
        ln -nfs libmali.so ${D}${libdir}/libGLESv1_CM.so.1.1.0

        ln -nfs libmali.so ${D}${libdir}/libGLESv2.so
        ln -nfs libmali.so ${D}${libdir}/libGLESv2.so.2
        ln -nfs libmali.so ${D}${libdir}/libGLESv2.so.2.0.0

        ln -nfs libmali.so ${D}${libdir}/libwayland-egl.so
        ln -nfs libmali.so ${D}${libdir}/libwayland-egl.so.1
        ln -nfs libmali.so ${D}${libdir}/libwayland-egl.so.1.0.0

        ln -nfs libOpenCL.so ${D}${libdir}/libOpenCL.so.1
        ln -nfs libOpenCL.so ${D}${libdir}/libOpenCL.so.1.1
        ln -nfs libOpenCL.so ${D}${libdir}/libOpenCL.so.1.1.0
}

RDEPENDS_${PN}-dev = ""

FILES_${PN} = "${libdir}/libmali.so* \
               ${libdir}/libGLESv2.so* \
               ${libdir}/libEGL.so* \
               ${libdir}/libgbm.so* \
               ${libdir}/libGLESv1_CM.so* \
               ${libdir}/libwayland-egl.so* \
               ${libdir}/libOpenCL.so* \
               ${bindir}/mali_cl_unit \
               ${bindir}/mali_cl_simple_opencl_example "

FILES_${PN}-dev = "${libdir}/libMali.la \
                   ${libdir}/libEGL.la \
                   ${libdir}/libOpenCL.la \
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
                   ${includedir}/GLES3/gl3.h  \
                   ${includedir}/GLES3/gl3platform.h  \
                   ${includedir}/GLES3/gl31.h  \
                   ${includedir}/GLES3/gl32.h \
		   ${includedir}/GLES3/gl3ext.h \
                   ${libdir}/libGLESv1_CM.la \
                   ${libdir}/pkgconfig/glesv1_cm.pc \
                   ${includedir}/GLES/egl.h \
                   ${includedir}/GLES/glext.h \
                   ${includedir}/GLES/gl.h \
                   ${includedir}/GLES/glplatform.h \
                   ${libdir}/libwayland-egl.la \
                   ${libdir}/pkgconfig/wayland-egl.pc \
                   ${includedir}/KHR/khrplatform.h \
                   ${includedir}/CL/cl.h  \
                   ${includedir}/CL/cl.hpp  \
                   ${includedir}/CL/cl_d3d10.h \
                   ${includedir}/CL/cl_d3d11.h  \
                   ${includedir}/CL/cl_dx9_media_sharing.h  \
                   ${includedir}/CL/cl_egl.h  \
                   ${includedir}/CL/cl_ext.h \
                   ${includedir}/CL/cl_gl.h \
                   ${includedir}/CL/cl_gl_ext.h \
                   ${includedir}/CL/cl_platform.h \
                   ${includedir}/CL/opencl.h \
                   ${includedir}/CL_2_0/cl.h  \
                   ${includedir}/CL_2_0/cl.hpp  \
                   ${includedir}/CL_2_0/cl_d3d10.h \
                   ${includedir}/CL_2_0/cl_d3d11.h  \
                   ${includedir}/CL_2_0/cl_dx9_media_sharing.h  \
                   ${includedir}/CL_2_0/cl_egl.h  \
                   ${includedir}/CL_2_0/cl_ext.h \
                   ${includedir}/CL_2_0/cl_gl.h \
                   ${includedir}/CL_2_0/cl_gl_ext.h \
                   ${includedir}/CL_2_0/cl_platform.h \
                   ${includedir}/CL_2_0/opencl.h \
                   ${includedir}/CL_HPP/cl.hpp \
                   ${includedir}/CL_HPP/cl2.hpp"

INHIBIT_PACKAGE_DEBUG_SPLIT = "1"
INSANE_SKIP_${PN} += "dev-so"
INSANE_SKIP_${PN} += "dev-deps"
INSANE_SKIP_${PN} += "already-stripped"

