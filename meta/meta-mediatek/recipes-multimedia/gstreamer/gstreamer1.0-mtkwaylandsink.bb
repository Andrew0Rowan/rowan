SUMMARY = "Plugins for the GStreamer multimedia framework 1.x"
HOMEPAGE = "http://gstreamer.freedesktop.org/"
BUGTRACKER = "https://bugzilla.gnome.org/enter_bug.cgi?product=Gstreamer"
SECTION = "multimedia"

DEPENDS += "gstreamer1.0 libdrm gstreamer1.0-plugins-base gstreamer1.0-plugins-bad glib-2.0 wayland mali weston"

inherit workonsrc autotools pkgconfig upstream-version-is-even gobject-introspection

LIBV = "1.0"
require ../../../poky/meta/recipes-multimedia/gstreamer/gst-plugins-package.inc

# Orc enables runtime JIT compilation of data processing routines from Orc
# bytecode to SIMD instructions for various architectures (currently SSE, MMX,
# MIPS, Altivec and NEON are supported).

GSTREAMER_ORC ?= "orc"

PACKAGECONFIG[debug] = "--enable-debug,--disable-debug"
PACKAGECONFIG[orc] = "--enable-orc,--disable-orc,orc orc-native"
PACKAGECONFIG[valgrind] = "--enable-valgrind,--disable-valgrind,valgrind"

export ORCC = "${STAGING_DIR_NATIVE}${bindir}/orcc"

EXTRA_OECONF = " \
    --disable-examples \
"

PACKAGES_DYNAMIC = "^${PN}-.*"

LICENSE = "GPLv2+ & LGPLv2+ & LGPLv2.1+"

LIC_FILES_CHKSUM = "file://COPYING;md5=a6f89e2100d9b6cdffcea4f398e37343"

WORKONSRC = "${TOPDIR}/../src/multimedia/gst-mtkwaylandsink"
LIBGSTWAYLAND_LA = "${TMPDIR}/sysroots/${MACHINE}/${libdir}/libgstwayland-1.0.la"


do_configure_prepend() {
	#!/bin/sh

	if [ ! -f "${WORKONSRC}/src/linux-dmabuf-unstable-v1-protocol.c" ]; then
		cp ${TMPDIR}/sysroots/${MACHINE}/usr/include/weston/linux-dmabuf-unstable-v1-protocol.c ${WORKONSRC}/src/
	fi

	if [ ! -f "${WORKONSRC}/src/ivi-application-protocol.c" ]; then
		cp ${TMPDIR}/sysroots/${MACHINE}/usr/include/weston/ivi-application-protocol.c ${WORKONSRC}/src/
	fi

	if [ ! -f "${WORKONSRC}/src/xdg-shell-unstable-v5-protocol.c" ]; then
		cp ${TMPDIR}/sysroots/${MACHINE}/usr/include/weston/xdg-shell-unstable-v5-protocol.c ${WORKONSRC}/src/
	fi

	if [ ! -f "${WORKONSRC}/src/weston-configure-mtk-protocol.c" ]; then
		cp ${TMPDIR}/sysroots/${MACHINE}/usr/include/weston/weston-configure-mtk-protocol.c ${WORKONSRC}/src/
	fi
}

do_compile () {
	oe_runmake \
	    CFLAGS="`pkg-config --cflags libdrm weston wayland-protocols wayland-scanner wayland-client wayland-server wayland-cursor wayland-egl gstreamer-plugins-bad-1.0` ${CFLAGS}" \
	    LDFLAGS="`pkg-config --libs libdrm weston wayland-protocols wayland-scanner wayland-client wayland-server wayland-cursor wayland-egl gstreamer-plugins-bad-1.0` ${LDFLAGS}" \
	    LIBGSTWAYLAND_LA=${LIBGSTWAYLAND_LA}
}
