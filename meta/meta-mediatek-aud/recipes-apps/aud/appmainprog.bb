DESCRIPTION = "Appmainprog"
LICENSE = "MediaTekProprietary"
LIC_FILES_CHKSUM = "file://COPYING;md5=d7810fab7487fb0aad327b76f1be7cd7"
DEPENDS += "smtcn-mw app-prebuilt nvram ${@base_contains('LICENSE_FLAGS_BLUEZ', 'yes', '', 'bluetooth' ,d)} curl wifi-mw wlanMon mtk-alsa-plugins smartaudioplayer prog cjson"

inherit cmake workonsrc

WORKONSRC = "${TOPDIR}/../src/apps/aud-base/main"
S = "${WORKDIR}"

#Pass the flag to cmake
EXTRA_OECMAKE += "${@base_contains('WITHOUT_BLUEDROID','yes','-DNO_BLUEDROID=1','-DNO_BLUEDROID=0',d)}"
EXTRA_OECMAKE += "${@base_contains('BT_LAUNCHER_SUFFIX','6627_','-DCONSYS_WIFI_CHIP=1','-DCONSYS_WIFI_CHIP=0',d)}"
EXTRA_OECMAKE += "${@base_contains('BT_LAUNCHER_SUFFIX','6630_','-DWIFI_CHIP_6630=1','-DWIFI_CHIP_6630=0',d)}"
EXTRA_OECMAKE += "${@base_contains('BT_LAUNCHER_SUFFIX','7668_','-DWIFI_CHIP_7668=1','-DWIFI_CHIP_7668=0',d)}"
EXTRA_OECMAKE += "${@base_contains('GATT_TEST','yes','-DGATT_TEST_SUPPORT=1','-DGATT_TEST_SUPPORT=0',d)}"
EXTRA_OECMAKE += "${@base_contains('LICENSE_FLAGS_GMRENDER','yes','-DDLNA_SUPPORT=1','-DDLNA_SUPPORT=0',d)}"
EXTRA_OECMAKE += "${@base_contains('BOARD_NAME','mt8516-som','-DMT8516_SOM=1','-DMT8516_SOM=0',d)}"
EXTRA_OECMAKE += "${@base_contains('ALGO_WENZHI','yes','-DCONSYS_ALGO_WENZHI=1','-DCONSYS_ALGO_WENZHI=0',d)}"
EXTRA_OECMAKE += "${@base_contains('BLE_ENABLE','yes','-DBLE_SUPPORT=1','-DBLE_SUPPORT=0',d)}"

do_install() {
	install -d ${D}/sbin
  install -d ${D}/${libdir}
	install -d ${D}${bindir}
	install -d ${D}/data/misc

	if ${@bb.utils.contains('LICENSE_FLAGS_BLUEZ', 'yes', 'true', 'false' ,d)}; then
	    echo no need hfp
	else
	   	install -d ${D}/data/misc/public/bluetooth_hfp
	    install -m 755 ${TOPDIR}/../src/apps/aud-base/public/bluetooth_hfp/* ${D}/data/misc/public/bluetooth_hfp
	fi

	install -m 755 ${S}/appmainprog ${D}${bindir}
	install -m 755 ${S}/script/sbin/* ${D}/sbin
	install -m 755 ${S}/script/usr/bin/* ${D}/usr/bin
	
}

FILES_${PN} += "${libdir} /data/misc /usr/lib/public/bluetooth_hfp"
INSANE_SKIP_${PN} += "already-stripped"
FILES_${PN}-dev = ""
