SUMMARY = "msmtp is an SMTP client."
DESCRIPTION = "A sendmail replacement for use in MTAs like mutt"
HOMEPAGE = "http://msmtp.sourceforge.net/"
SECTION = "console/network"

LICENSE = "GPLv2"

LIC_FILES_CHKSUM = "file://COPYING;md5=94d55d512a9ba36caa9b7df079bae19f"

DEPENDS = "zlib gnutls"

SRC_URI = "http://sourceforge.net/projects/msmtp/files/msmtp/${PV}/${BPN}-${PV}.tar.bz2 \
           file://msmtp-fixup-api-and-compiler-warning.patch \
           file://msmtp-replace-deprecated-function.patch \
           "

SRC_URI[md5sum] = "ba5b61d5f7667d288f1cfadccfff8ac5"
SRC_URI[sha256sum] = "ab794bb014cdaeae0a1460a7aca1869dab8c93383bf01f41aca41b3d99b69509"

EXTRA_OECONF += "--with-libgnutls-prefix=${STAGING_DIR}/${HOST_SYS}"

PACKAGECONFIG ??= ""
PACKAGECONFIG[libidn] = "--with-libidn,--without-libidn,libidn"

inherit gettext autotools update-alternatives

ALTERNATIVE_${PN} = "sendmail"
ALTERNATIVE_TARGET[sendmail] = "${bindir}/msmtp"
ALTERNATIVE_LINK_NAME[sendmail] = "${sbindir}/sendmail"
ALTERNATIVE_PRIORITY = "100"

pkg_postinst_${PN}_linuxstdbase () {
    # /usr/lib/sendmail is required by LSB specification
    [ ! -L $D/usr/lib/sendmail ] && ln -sf ${sbindir}/sendmail $D/usr/lib
}
