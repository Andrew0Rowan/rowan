ppc-framework_package_arch=$(subst -,_,${PACKAGE_ARCH})

all:
install:
	find -name "*${ppc-framework_package_arch}*.rpm" | while read i; do ${bindir}/rpm2cpio $${i} | cpio -idmv ; done
	install -d ${DESTDIR}${libdir}
	install -m 755 ./${libdir}/libppcframework.a ${DESTDIR}${libdir}