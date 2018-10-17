SUMMARY = "gmrender"
DESCRIPTION = "gmediarender"

LICENSE = "GPLv2+"
LICENSE_FLAGS = "commercial"
LIC_FILES_CHKSUM = "file://NOTICE;md5=c1a3ff0b97f199c7ebcfdd4d3fed238e"
SECTION = "libs"

S="${WORKDIR}/git"
SRC_URI[md5sum] = "4fc1d5ce6ef2c27fae01d3e3fdc46240"
SRC_URI[sha256sum] = "85d3251ce3095394d389b56f8b47e7f79e906022e51ce92a4cf822ee2a5fee2c"

SRC_URI = "git://anonscm.debian.org/git/collab-maint/gmrender-resurrect.git;protocol=http;branch=master \
file://0001-gmrender.patch"

SRCREV = "9306fc5ad1d032f387d41fbce7dcba8cf22c8527"

FILES_${PN} = "${bindir} ${libdir}"

DEPENDS += "libupnp gstreamer1.0 gstreamer1.0-plugins-good gstreamer1.0-plugins-bad gstreamer1.0-plugins-base gstreamer1.0-plugins-ugly gstreamer1.0-libav"

do_configure() {
    cd ${S}  && ./autogen.sh && ./configure --host  --host-alias
}

do_compile() {
    cd ${S}  && make clean && make
}

do_install() {
  install -d ${D}${libdir}
  install -m 775 ${S}/src/gmediarender  ${D}${libdir}
}

#inherit autotools
