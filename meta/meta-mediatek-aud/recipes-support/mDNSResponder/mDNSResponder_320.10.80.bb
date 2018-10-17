SUMMARY = "mDNSResponder"
DESCRIPTION = "The mDNSResponder project is a component of Bonjour"
HOMEPAGE = "http://opensource.apple.com/tarballs/mDNSResponder"
LICENSE = "Apache"
SRC_URI[md5sum] = "2f88bd0edae86ff2442827182dd1a394"
SRC_URI[sha256sum] = "e0ccc272e463f1728b6a2a516babab2537e0aae526c5d7ebf4abe227529b2324"

SRC_URI = "http://opensource.apple.com/tarballs/mDNSResponder/mDNSResponder-${PV}.tar.gz \
           file://0001-mdns-compile-adapter.patch "

do_compile() {
	cd ${S}/mDNSPosix && make os=linux
}

do_install() {
	install -d ${D}/${libdir}
	install -d ${D}/${bindir}
	install -m 755 ${S}/mDNSPosix/build/prod/libdns_sd.so ${D}/${libdir}
	install -m 755 ${S}/mDNSPosix/build/prod/mdnsd ${D}/usr/bin
	install -m 755 ${S}/Clients/build/dns-sd ${D}/usr/bin
}

FILES_${PN}+="${libdir} ${bindir}"

INSANE_SKIP_${PN} += "installed-vs-shipped"
INSANE_SKIP_${PN} += "already-stripped"
INSANE_SKIP_${PN} += "ldflags"
INSANE_SKIP_${PN}-dev += "ldflags"