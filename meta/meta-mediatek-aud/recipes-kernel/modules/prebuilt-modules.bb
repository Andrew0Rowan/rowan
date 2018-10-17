DESCRIPTION="install prebuilt ko"
LICENSE="GPLv2"
inherit bin_package

PN='prebuilt-modules'
S="${WORKDIR}/${PN}"

SRC_URI=" \
	file://prebuilt-modules.tar.xz \
"

FILES_${PN}-dev=""
FILES_${PN} +="/lib/modules /etc/modules-load.d"
INHIBIT_PACKAGE_STRIP = "1"
