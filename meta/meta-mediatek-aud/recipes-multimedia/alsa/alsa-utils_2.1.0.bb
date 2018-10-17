SUMMARY = "ALSA sound utilities"
HOMEPAGE = "http://www.alsa-project.org"
BUGTRACKER = "https://bugtrack.alsa-project.org/alsa-bug/login_page.php"
SECTION = "console/utils"
LICENSE = "GPLv2+"
LIC_FILES_CHKSUM = "file://COPYING;md5=59530bdf33659b29e73d4adb9f9f6552 \
                    file://alsactl/utils.c;beginline=1;endline=20;md5=fe9526b055e246b5558809a5ae25c0b9"
DEPENDS = "alsa-lib ncurses libsamplerate0"

PACKAGECONFIG ??= "udev"
PACKAGECONFIG[bat] = "--enable-bat,--disable-bat,fftw"
PACKAGECONFIG[udev] = "--with-udev-rules-dir=`pkg-config --variable=udevdir udev`/rules.d,,udev"
PACKAGECONFIG[xmlto] = "--enable-xmlto, --disable-xmlto, xmlto-native docbook-xml-dtd4-native docbook-xsl-stylesheets-native"
PV = "1.1.0"
SRC_URI = "ftp://ftp.alsa-project.org/pub/utils/alsa-utils-${PV}.tar.bz2 \
           file://0001-alsactl-don-t-let-systemd-unit-restore-the-volume-wh.patch \
           file://0001-alsabat-rename-to-avoid-naming-conflict.patch \
           file://a.patch \
          "

SRC_URI[md5sum] = "b9d6102fbbd0b68040bb77023ed30c0c"
SRC_URI[sha256sum] = "3b1c3135b76e14532d3dd23fb15759ddd7daf9ffbc183f7a9a0a3a86374748f1"

# lazy hack. needs proper fixing in gettext.m4, see
# http://bugs.openembedded.org/show_bug.cgi?id=2348
# please close bug and remove this comment when properly fixed
#
EXTRA_OECONF_append_libc-uclibc = " --disable-nls"

inherit autotools gettext pkgconfig

# This are all packages that we need to make. Also, the now empty alsa-utils
# ipk depends on them.

FILES_${PN} += "/var/* /lib/* /usr/share/* /usr/bin/*"

ALLOW_EMPTY_alsa-utils = "1"

do_install() {
	autotools_do_install

	# We don't ship this here because it requires a dependency on bash.
	# See alsa-utils-scripts_${PV}.bb
	rm ${D}${sbindir}/alsaconf
	rm ${D}${sbindir}/alsa-info.sh

	if ${@bb.utils.contains('PACKAGECONFIG', 'udev', 'false', 'true', d)}; then
	   # This is where alsa-utils will install its rules if we don't tell it anything else.
	   rm -rf ${D}/lib/udev
	   rmdir --ignore-fail-on-non-empty ${D}/lib
	fi
}
