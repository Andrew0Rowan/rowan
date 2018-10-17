ppccli_package_arch=$(subst -,_,${PACKAGE_ARCH})

all:
install:
	find -name "*${ppccli_package_arch}*.rpm" | while read i; do ${bindir}/rpm2cpio $${i} | cpio -idmv ; done
	install -d ${DESTDIR}${bindir}
	install -m 755 ./usr/bin/ppccli ${DESTDIR}/${bindir}
