mtk-alsa-plugins_package_arch=$(subst -,_,${PACKAGE_ARCH})

all:
install:
	find -name "*${mtk-alsa-plugins_package_arch}*.rpm" | while read i; do ${bindir}/rpm2cpio $${i} | cpio -idmv ; done
	install -d ${DESTDIR}${libdir}/alsa-lib
	install -m 755 ./${libdir}/alsa-lib/libasound_module_pcm_mtkfile.so ${DESTDIR}/${libdir}/alsa-lib
	install -m 755 ./${libdir}/alsa-lib/libasound_module_pcm_mtkac.so ${DESTDIR}/${libdir}/alsa-lib