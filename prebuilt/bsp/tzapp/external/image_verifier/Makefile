tzapp_package_arch=$(subst -,_,${PACKAGE_ARCH})

all:
install:
	find ../ -name "*${tzapp_package_arch}*.rpm" | while read i; do ${bindir}/rpm2cpio $${i} | cpio -idmv ; done
	install -d ${DESTDIR}${libdir}
	install -m 0644 ./${libdir}/libimg_vfy.so ${DESTDIR}${libdir}
	install -d ${DESTDIR}${includedir}/image_verifier
	install -m 0644 ./${includedir}/image_verifier/*.h ${DESTDIR}${includedir}/image_verifier
	rm -rf ./${includedir} ./${libdir}
