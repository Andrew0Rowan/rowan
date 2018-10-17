tzapp_package_arch=$(subst -,_,${PACKAGE_ARCH})

all:
install:
	find ../ -name "*${tzapp_package_arch}*.rpm" | while read i; do ${bindir}/rpm2cpio $${i} | cpio -idmv ; done
	install -d ${DESTDIR}${libdir}
	install -m 0644 ./${libdir}/libtz_efuse.so ${DESTDIR}${libdir}
	install -d ${DESTDIR}${includedir}/efuse
	install -m 0644 ./${includedir}/efuse/*.h ${DESTDIR}${includedir}/efuse
	rm -rf ./${includedir} ./${libdir}
