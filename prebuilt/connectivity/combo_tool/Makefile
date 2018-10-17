mtkcombotool_package_arch=$(subst -,_,${PACKAGE_ARCH})

all:
install:
	find -name "*${mtkcombotool_package_arch}*.rpm" | while read i; do ${bindir}/rpm2cpio $${i} | cpio -idmv ; done
	install -d ${DESTDIR}${bindir}	
	install -m 0755 ./${bindir}/wmt_loader ${DESTDIR}/${bindir}
	install -m 0755 ./${bindir}/6620_launcher ${DESTDIR}/${bindir}
	install -m 0755 ./${bindir}/power_on_wifi ${DESTDIR}/${bindir}
	install -m 0755 ./${bindir}/wmt_loopback  ${DESTDIR}/${bindir}
	install -m 0755 ./${bindir}/stp_dump  ${DESTDIR}/${bindir}
	install -m 755 ./usr/bin/8516_wlan.sh ${DESTDIR}/${bindir}
	install -d ${DESTDIR}${systemd_unitdir}/system/
	install -m 755 ./usr/bin/8516_wlan.sh ${DESTDIR}/${bindir}
	install -m 0644 ./lib/systemd/system/wmtd.service ${DESTDIR}${systemd_unitdir}/system
	install -m 0644 ./lib/systemd/system/launcher_6627.service ${DESTDIR}${systemd_unitdir}/system
	install -m 0644 ./lib/systemd/system/stp_dump.service ${DESTDIR}${systemd_unitdir}/system
	
	install -d ${DESTDIR}/lib/firmware
	install -m 0755 ./lib/firmware/WMT_SOC.cfg ${DESTDIR}/lib/firmware

