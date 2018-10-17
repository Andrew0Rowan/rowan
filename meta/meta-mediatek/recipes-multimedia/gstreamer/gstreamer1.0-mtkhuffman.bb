SUMMARY = "Plugins for the GStreamer multimedia framework 1.x"
HOMEPAGE = "http://gstreamer.freedesktop.org/"
BUGTRACKER = "https://bugzilla.gnome.org/enter_bug.cgi?product=Gstreamer"
SECTION = "multimedia"

DEPENDS = "gstreamer1.0 glib-2.0-native gstreamer1.0-plugins-base glib-2.0"

inherit autotools pkgconfig upstream-version-is-even gobject-introspection

acpaths = "-I ${S}/common/m4 -I ${S}/m4"

LIBV = "1.0"
require ../../../poky/meta/recipes-multimedia/gstreamer/gst-plugins-package.inc

delete_pkg_m4_file() {
        # This m4 file is out of date and is missing PKG_CONFIG_SYSROOT_PATH tweaks which we need for introspection
        rm "${S}/common/m4/pkg.m4" || true
}

do_configure[prefuncs] += " delete_pkg_m4_file"

PACKAGES_DYNAMIC = "^${PN}-.*"

LICENSE = "GPLv2+ & LGPLv2.1+"

LIC_FILES_CHKSUM = "file://COPYING;md5=a6f89e2100d9b6cdffcea4f398e37343"

inherit workonsrc

WORKONSRC = "${TOPDIR}/../src/multimedia/gst-mtkhuffman"

