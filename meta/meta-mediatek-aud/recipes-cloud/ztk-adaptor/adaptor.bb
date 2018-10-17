DESCRIPTION = "ZTK Adaptor Share Library"
LICENSE = "MediaTekProprietary"
DEPENDS += "smtcn-mw adaptor-prog cjson"

inherit cmake workonsrc
WORKONSRC = "${TOPDIR}/../src/cloud/ztk-adaptor"
ADAPTOR_SHARED_LIB = "adaptor"
ADAPTOR_SHARED_LIB_NAME = "lib${ADAPTOR_SHARED_LIB}.so"

EXTRA_OECMAKE = " ${@base_contains('WITHOUT_BLUEDROID','yes','-DNO_BLUEDROID=1','-DNO_BLUEDROID=0',d)} \
                  ${@base_contains('BT_LAUNCHER_SUFFIX','6627_','-DCONSYS_WIFI_CHIP=1','-DCONSYS_WIFI_CHIP=0',d)} \
                  ${@base_contains('BT_LAUNCHER_SUFFIX','6630_','-DWIFI_CHIP_6630=1','-DWIFI_CHIP_6630=0',d)} \
                  ${@base_contains('BT_LAUNCHER_SUFFIX','7668_','-DWIFI_CHIP_7668=1','-DWIFI_CHIP_7668=0',d)} \
                  ${@base_contains('GATT_TEST','yes','-DGATT_TEST_SUPPORT=1','-DGATT_TEST_SUPPORT=0',d)} \
                  ${@base_contains('LICENSE_FLAGS_GMRENDER','yes','-DDLNA_SUPPORT=1','-DDLNA_SUPPORT=0',d)} \
                  ${@base_contains('BOARD_NAME','mt8516-som','-DMT8516_SOM=1','-DMT8516_SOM=0',d)} \
                  -DBT_MW_INCLUDE_DIR=${TOPDIR}/../src/connectivity/bt_others/bluetooth_mw \
                  -DWIFI_MW_INCLUDE_DIR=${TOPDIR}/../src/apps/aud-base/library/wifi_mw/wifi \
                  -DAPPMAINPROG_SOURCE=${TOPDIR}/../src/apps/aud-base/main \
                  -DADAPTOR_SHARED_LIB=${ADAPTOR_SHARED_LIB} \
                  -DADAPTOR_SOURCE_ADD=1 \
                  -DADAPTOR_APP_CTRL_SOURCE=${TOPDIR}/../src/cloud/ztk-adaptor/app_ctrl \
                  -DCMAKE_BUILD_TYPE=RELEASE \
                  -DCMAKE_INSTALL_LIBDIR=${libdir} \
                  -DCMAKE_INSTALL_BASELIBDIR=${base_libdir} \
                  -DCMAKE_INSTALL_INCLUDEDIR=${includedir} \
                  -DRPMIO2CPIO=${TOPDIR}/tmp/sysroots/x86_64-linux/usr/bin/rpm2cpio"

do_install_append() {
	install -d ${D}/sbin
	install -d ${D}${bindir}
	install -d ${D}/data/misc
	
	if ${@bb.utils.contains('LICENSE_FLAGS_BLUEZ', 'yes', 'true', 'false' ,d)}; then
	    echo no need hfp
	else
	   	install -d ${D}/data/misc/public/bluetooth_hfp
	    install -m 755 ${TOPDIR}/../src/apps/aud-base/public/bluetooth_hfp/* ${D}/data/misc/public/bluetooth_hfp
	fi

	install -m 755 ${TOPDIR}/../src/apps/aud-base/main/script/sbin/* ${D}/sbin
	install -m 755 ${TOPDIR}/../src/apps/aud-base/main/script/usr/bin/* ${D}/usr/bin
}

FILES_${PN} += "${libdir}/${ADAPTOR_SHARED_LIB_NAME}"
FILES_${PN} += "${includedir}/adaptor"
FILES_${PN} += "/data/misc /usr/lib/public/bluetooth_hfp"
FILES_${PN} += "/sbin"
INSANE_SKIP_${PN} = "already-stripped installed-vs-shipped"
FILES_${PN}-dev = ""
