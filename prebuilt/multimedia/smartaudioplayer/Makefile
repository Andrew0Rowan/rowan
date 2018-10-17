smartaudioplayer_package_arch=$(subst -,_,${PACKAGE_ARCH})

all:
install:
	find -name "*${smartaudioplayer_package_arch}*.rpm" | while read i; do ${bindir}/rpm2cpio $${i} | cpio -idmv ; done
	install -d ${DESTDIR}${libdir}
	install -m 0644 ./${libdir}/libsmartaudioplayer.so ${DESTDIR}${libdir}
	install -d ${DESTDIR}/${includedir}
	install -m 0644 ./${includedir}/* ${DESTDIR}/${includedir}/
