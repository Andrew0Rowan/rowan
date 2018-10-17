DESCRIPTION = "DuerOS"
LICENSE = "GPLv2"
DEPENDS += "alsa-utils alsa-lib glibc nghttp2 adaptor uicustom"

inherit cmake workonsrc systemd

WORKONSRC = "${TOPDIR}/../src/cloud/dueros"

EXTRA_OECMAKE = " -DPlatform=ZTK \
                  -DBUILD_ONE_LIB=ON \
                  -DBUILD_USE_ALSA_RECORDER=ON \
                  -DCMAKE_BUILD_TYPE=RELEASE \
							    -DAPPLICATION_INCLUDE_DIR=${TOPDIR}/tmp/sysroots/${MTK_PROJECT}/usr/include/adaptor \
							    -DAPPLICATION_LD_DIR=${TOPDIR}/tmp/sysroots/${MTK_PROJECT}/usr/lib64 \
							    -DADAPTOR_LIBRARY_LD_DIR=${TOPDIR}/tmp/sysroots/${MTK_PROJECT}/usr/lib64 \
							    -DBT_MW_INCLUDE_DIR=${TOPDIR}/../src/connectivity/bt_others/bluetooth_mw \
                  -DCMAKE_INSTALL_MANDIR=${mandir} \
                  -DCMAKE_INSTALL_BINDIR=${bindir} \
                  -DCMAKE_INSTALL_SBINDIR=${sbindir} \
                  -DCMAKE_INSTALL_SYSCONFDIR=${sysconfdir} \
                  -DCMAKE_INSTALL_LIBDIR=${libdir} \
                  -DCMAKE_INSTALL_FULL_LOCALSTATEDIR=${localstatedir}"

SYSTEMD_PACKAGES = "${PN}"
SYSTEMD_SERVICE_${PN} = "dueros.service"
FILES_${PN} += "${systemd_unitdir}/system/dueros.service"

do_install_append () {
    install -d ${D}/data/misc/dueros
    install -m 0755 ${S}sdk/log.config ${D}/data/misc/dueros
    install -m 0644 ${S}conf/dueros_config.json ${D}/data/misc/dueros
		
    install -d ${D}/data/misc/dueros/appresources
    install -m 0755 ${S}appresources/* ${D}/data/misc/dueros/appresources
		
    install -d ${D}/data/misc/dueros/resources
    install -m 0755 ${S}resources/* ${D}/data/misc/dueros/resources
    
    install -d ${D}${systemd_unitdir}/system
    install -m 755 ${S}service/dueros.service ${D}${systemd_unitdir}/system
		
    install -d ${D}${bindir}/dueros
    install -m 755 ${S}service/dueros.sh ${D}${bindir}/dueros
    install -m 755 ${S}service/generator_device_id.sh ${D}${bindir}/dueros

    install -d ${D}/etc/
    install -m 0644 ${S}conf/asound.conf ${D}/etc
}

FILES_${PN} += "${libdir}"
FILES_${PN} += "${bindir}/dueros"
FILES_${PN} += "/data/misc/dueros"
FILES_${PN} += "/data/misc/dueros/appresources"
FILES_${PN} += "/data/misc/dueros/resources"
FILES_${PN} += "/etc"
INSANE_SKIP_${PN} += "already-stripped installed-vs-shipped"
FILES_${PN}-dev = ""
