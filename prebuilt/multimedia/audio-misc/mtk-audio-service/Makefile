mtk-audio-service_package_arch=$(subst -,_,${PACKAGE_ARCH})

all:
install:
	find -name "*${mtk-audio-service_package_arch}*.rpm" | while read i; do ${bindir}/rpm2cpio $${i} | cpio -idmv ; done
	install -d ${DESTDIR}${libdir}
	install -d ${DESTDIR}${bindir}
	install -d ${DESTDIR}${includedir}
	install -m 755 ./${libdir}/libmsb.so ${DESTDIR}${libdir}
	install -m 755 ./${libdir}/libmil.so ${DESTDIR}/${libdir}
	install -m 755 ./${libdir}/libmtsrc.so ${DESTDIR}/${libdir}
	install -m 755 ./${libdir}/libmsrc.so ${DESTDIR}/${libdir}
	install -m 755 ./${libdir}/libmas.so ${DESTDIR}/${libdir}
	install -m 755 ./${libdir}/libmas2.so ${DESTDIR}/${libdir}
	install -m 755 ./${libdir}/libpcm_merge.so ${DESTDIR}/${libdir}
	install -m 755 ./${libdir}/libblisrc32.so ${DESTDIR}/${libdir}
	install -m 755 ./usr/bin/mas_test ${DESTDIR}/${bindir}
	install -m 755 ./usr/bin/mas_test2 ${DESTDIR}/${bindir}
	install -m 755 ./usr/bin/data_send2 ${DESTDIR}/${bindir}
	install -m 755 ./usr/bin/mas_dbg ${DESTDIR}/${bindir}
	install -m 755 ./usr/bin/pcm_merge_test ${DESTDIR}/${bindir}
	install -m 755 ./usr/include/mas_defs.h ${DESTDIR}/${includedir}
	install -m 755 ./usr/include/mas_interface.h ${DESTDIR}/${includedir}
	install -m 755 ./usr/include/mas_lib.h ${DESTDIR}/${includedir}
	install -m 755 ./usr/include/pcm_merge.h ${DESTDIR}/${includedir}
