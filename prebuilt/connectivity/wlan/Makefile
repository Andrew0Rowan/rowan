mtkwlan_package_arch=$(subst -,_,${PACKAGE_ARCH})

all:
install:
	find -name "*${mtkwlan_package_arch}*.rpm" | while read i; do ${bindir}/rpm2cpio $${i} | cpio -idmv ; done
ifeq (${COMBO_CHIP_ID_FLAG},mt6630)
	install -d ${DESTDIR}/lib/firmware
	install -m 0755 ./lib/firmware/audio/WIFI_RAM_CODE_MT6630 ${DESTDIR}/lib/firmware
endif

ifeq (${COMBO_CHIP_ID_FLAG},mt8167)
	install -d ${DESTDIR}/vendor/firmware
	install -m 0755 ./vendor/firmware/WIFI_RAM_CODE_8167 ${DESTDIR}/vendor/firmware
endif
	install -d ${DESTDIR}/etc/
	install -m 0644 ./etc/hostapd_mtk.conf ${DESTDIR}/etc/hostapd_mtk.conf
	install -m 0644 ./etc/udhcpd.conf ${DESTDIR}/etc/udhcpd.conf
	install -d ${DESTDIR}/bin/
	install -m 0755 ./bin/runap.sh ${DESTDIR}/bin/runap.sh
	install -d ${DESTDIR}/usr/bin
	install -m 0644 ./usr/bin/wlan_connect_ap.sh ${DESTDIR}/usr/bin
