mpctec_package_arch=$(subst -,_,${PACKAGE_ARCH})

all:
install:
	install -d ${DESTDIR}/${bindir}
	install -d ${DESTDIR}${libdir}
	install -m 0755 ./algo-data-server ${DESTDIR}/${bindir}
	install -m 0644 ./lib/* ${DESTDIR}${libdir}
