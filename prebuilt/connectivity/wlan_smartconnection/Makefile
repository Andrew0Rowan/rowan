smtcn-mw_package_arch=$(subst -,_,${PACKAGE_ARCH})

all:
install:
	find -name "*${smtcn-mw_package_arch}*.rpm" | while read i; do ${bindir}/rpm2cpio $${i} | cpio -idmv ; done
	install -d ${DESTDIR}${libdir}
	install -m 755 ./${libdir}/libsmtcn.so ${DESTDIR}/${libdir}/
	install -d ${DESTDIR}${bindir}
	install -m 755 ./${bindir}/smtcn_demo ${DESTDIR}/${bindir}/
	install -d ${DESTDIR}${includedir}
	install -m 755 ./usr/src/debug/smtcn-mw/1.0-r0/smtcn-mw-1.0/smtcn_lib/adapter/inc/*.h ${DESTDIR}/${includedir}/
