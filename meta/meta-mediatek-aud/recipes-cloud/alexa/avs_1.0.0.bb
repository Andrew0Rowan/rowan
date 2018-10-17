LICENSE = "ZelusTekProprietary"

DESCRIPTION = "AVS SDK"

DEPENDS = "curl sqlite3 adaptor adaptor-prog uicustom"
DEPENDS += "mpctec"
DEPENDS += "${@bb.utils.contains('LICENSE_FLAGS_WHITELIST','commercial','smartaudioplayer','',d)}"

RDEPENDS_${PN} = "curl libsqlite3 adaptor adaptor-prog uicustom"
RDEPENDS_${PN} += "${@bb.utils.contains('LICENSE_FLAGS_WHITELIST','commercial','smartaudioplayer','',d)}"

# include externalsrc to avoid In-source-build error from avs.
inherit cmake externalsrc-ext systemd

EXTERNALSRC = "${TOPDIR}/../src/cloud/alexa/avs-device-sdk-master"
# FIXME: to avoid bad RPATH during do_package_qa
# According to insane.bbclass BASE_WORKDIR/STAGING_DIR_TARGET are not allowed in rpath.
# Might also try to use do_package_qa[noexec] = "1"
EXTERNALSRC_BUILD = "${EXTERNALSRC}/../build"

SYSTEMD_PACKAGES = "${PN}"
SYSTEMD_SERVICE_${PN} = "alexaapp.service"

EXTRA_OECMAKE += "${@base_contains('LICENSE_FLAGS_WHITELIST','commercial','-DCONFIG_SUPPORT_SAP=ON','',d)}"

EXTRA_OECMAKE += " \
    -DSENSORY_THF_KEY_WORD_DETECTOR=ON \
    -DSENSORY_THF_KEY_WORD_DETECTOR_LIB_PATH=${EXTERNALSRC}/ThirdParty/alexa-sensory-thf/lib/libthf.a \
    -DSENSORY_THF_KEY_WORD_DETECTOR_INCLUDE_DIR=${EXTERNALSRC}/ThirdParty/alexa-sensory-thf/include \
    -DACSDK_ALEXA_APP_SUPPORT=ON \
"

# Usually FILES_${PN}-dev shouldn't include non-symlink .so
# Move ${libdir}/lib*.so from FILES_${PN}-dev to FILES_${PN}
FILES_${PN}-dev = " \
    ${includedir} ${base_libdir}/lib*.so ${libdir}/*.la \
    ${libdir}/*.o ${libdir}/pkgconfig ${datadir}/pkgconfig \
    ${datadir}/aclocal ${base_libdir}/*.o \
    ${libdir}/${BPN}/*.la ${base_libdir}/*.la"


FILES_${PN} += " \
    ${libdir}/lib*.so  \
    ${sysconfdir} \
    ${systemd_system_unitdir} \
    ${prefix}/misc/resources \
"
# FYI:
# ${sysconfdir} = /etc
# ${systemd_system_unitdir} = /lib/systemd/system
# ${prefix}/misc/resources = /usr/misc/resources

INSANE_SKIP_${PN} += "already-stripped"
