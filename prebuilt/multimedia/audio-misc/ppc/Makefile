ppc_package_arch=$(subst -,_,${PACKAGE_ARCH})

all:
install:
	find -name "*${ppc_package_arch}*.rpm" | while read i; do ${bindir}/rpm2cpio $${i} | cpio -idmv ; done
	install -d ${DESTDIR}${libdir}
	install -d ${DESTDIR}${includedir}
	install -m 755 ./${libdir}/libmbdrc_mtk.so ${DESTDIR}${libdir}
	install -m 755 ./${libdir}/libppcmodel.so ${DESTDIR}${libdir}
	install -m 755 ./${includedir}/ppc_control.h ${DESTDIR}/${includedir}
