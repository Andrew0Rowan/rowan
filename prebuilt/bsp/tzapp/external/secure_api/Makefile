tzapp_package_arch=$(subst -,_,${PACKAGE_ARCH})

all:
install:
	find ../ -name "*${tzapp_package_arch}*.rpm" | while read i; do ${bindir}/rpm2cpio $${i} | cpio -idmv ; done
	install -d ${DESTDIR}${libdir}
	install -m 0644 ./${libdir}/libsecure_api.so ${DESTDIR}${libdir}
	install -d ${DESTDIR}${includedir}/secure_api
	install -m 0644 ./${includedir}/secure_api/*.h ${DESTDIR}${includedir}/secure_api
	rm -rf ./${includedir} ./${libdir}
