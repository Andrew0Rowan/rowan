wenzhi_package_arch=$(subst -,_,${PACKAGE_ARCH})

all:
install:
	install -d ${DESTDIR}${bindir}
	install -d ${DESTDIR}${libdir}
ifeq ("${ADC_SETTING_FLAG}","tlv320")
	install -m 0755 ./WenzhiDemo ${DESTDIR}${bindir}
	install -m 0644 ./libwzrecognizer.so ${DESTDIR}${libdir}
else
	install -m 0755 ./WenzhiDemo_SX ${DESTDIR}${bindir}/WenzhiDemo
	install -m 0644 ./libwzrecognizer_SX.so ${DESTDIR}${libdir}/libwzrecognizer.so
endif
