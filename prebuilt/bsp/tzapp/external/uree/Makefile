tzapp_package_arch=$(subst -,_,${PACKAGE_ARCH})

all:
install:
	find ../ -name "*${tzapp_package_arch}*.rpm" | while read i; do ${bindir}/rpm2cpio $${i} | cpio -idmv ; done
	install -d ${DESTDIR}${libdir}
	install -m 0644 ./${libdir}/libtz_uree.so ${DESTDIR}${libdir}
	install -d ${DESTDIR}${includedir}/uree
	install -m 0644 ./${includedir}/uree/*.h ${DESTDIR}${includedir}/uree
	install -d ${DESTDIR}${includedir}/tz_cross
	install -m 0644 ./${includedir}/tz_cross/*.h ${DESTDIR}${includedir}/tz_cross
	rm -rf ./${includedir} ./${libdir}
