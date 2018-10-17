inherit deploy workonsrc

DESCRIPTION = "MTK In-house TEE Userspace Library/Application"
LICENSE = "MediaTekProprietary"
WORKONSRC = "${TOPDIR}/../src/bsp/tzapp/external"
LIC_FILES_CHKSUM = "file://uree/README;md5=5a126b0ba82af703f1c30cf8d0bb4e13"
TZ_UREE_OUT = "${WORKDIR}/out"
TZ_EFUSE_OUT = "${WORKDIR}/out"
TZ_IMGVFY_OUT = "${WORKDIR}/out"
TZ_SECURE_API_OUT = "${WORKDIR}/out"
TZ_UREE_BINRARY_OUT = "${TZ_UREE_OUT}/binraies"
TZ_EFUSE_BINRARY_OUT = "${TZ_EFUSE_OUT}/binraies"
TZ_IMGVFY_BINRARY_OUT = "${TZ_IMGVFY_OUT}/binraies"
TZ_SECURE_API_BINRARY_OUT = "${TZ_SECURE_API_OUT}/binraies"
TZ_UREE_LIBRARY = "libtz_uree.so"
TZ_EFUSE_LIBRARY = "libtz_efuse.so"
TZ_IMGVFY_LIBRARY = "libimg_vfy.so"
TZ_SECURE_API_LIBRARY = "libsecure_api.so"

# Avoid QA Issue: No GNU_HASH in the elf binary
INSANE_SKIP_${PN} = "ldflags"
INSANE_SKIP_${PN}-dev = "ldflags"
FILES_${PN} = "${libdir}/${TZ_UREE_LIBRARY}"
FILES_${PN} += "${libdir}/${TZ_EFUSE_LIBRARY}"
FILES_${PN} += "${libdir}/${TZ_IMGVFY_LIBRARY}"
FILES_${PN} += "${libdir}/${TZ_SECURE_API_LIBRARY}"
FILES_${PN}-dev = "${includedir} \
		${includedir}/uree/system.h \
		${includedir}/uree/mem.h \
		${includedir}/uree/dbg.h \
		${includedir}/tz_cross/trustzone.h \
		${includedir}/tz_cross/tz_gcpu.h \
		${includedir}/efuse/efuse.h \
		${includedir}/tz_cross/ta_mem.h \
		${includedir}/tz_cross/ta_sbcv.h \
		${includedir}/image_verifier/img_vfy.h \
		${includedir}/secure_api/mtk_crypto_api_user.h "

# Avoid QA Issue: 'Files/directories were installed but not shipped'
INSANE_SKIP_${PN} += "installed-vs-shipped"
DEPENDS += " openssl "


do_compile() {
if [ -e uree/makefile ]; then
    oe_runmake -C uree CROSS_COMPILE=${TARGET_PREFIX} MTK_PROJECT=${MTK_PROJECT} TZ_UREE_OUT=${TZ_UREE_OUT} TZ_UREE_BINRARY_OUT=${TZ_UREE_BINRARY_OUT}
fi
if [ -e efuse/makefile ]; then
    oe_runmake -C efuse CROSS_COMPILE=${TARGET_PREFIX} TZ_EFUSE_OUT=${TZ_EFUSE_OUT} TZ_EFUSE_BINRARY_OUT=${TZ_EFUSE_BINRARY_OUT}
fi
if [ -e image_verifier/makefile ]; then
    oe_runmake -C image_verifier CROSS_COMPILE=${TARGET_PREFIX} MTK_PROJECT=${MTK_PROJECT} TZ_IMGVFY_OUT=${TZ_IMGVFY_OUT} TZ_IMGVFY_BINRARY_OUT=${TZ_IMGVFY_BINRARY_OUT}
fi
if [ -e secure_api/makefile ]; then
    oe_runmake -C secure_api CROSS_COMPILE=${TARGET_PREFIX} MTK_PROJECT=${MTK_PROJECT} TZ_SECURE_API_OUT=${TZ_SECURE_API_OUT} TZ_SECURE_API_BINRARY_OUT=${TZ_SECURE_API_BINRARY_OUT}
fi


}

do_install() {
    oe_runmake -C uree PREFIX="${prefix}" DESTDIR="${D}" PACKAGE_ARCH="${PACKAGE_ARCH}" TZ_UREE_BINRARY_OUT="${TZ_UREE_BINRARY_OUT}" install
    oe_runmake -C efuse PREFIX="${prefix}" DESTDIR="${D}" TZ_PROJECT=${TZ_PROJECT} TZ_PLATFORM=${TZ_PLATFORM} PACKAGE_ARCH="${PACKAGE_ARCH}" TZ_EFUSE_BINRARY_OUT="${TZ_EFUSE_BINRARY_OUT}" install
    oe_runmake -C image_verifier PREFIX="${prefix}" DESTDIR="${D}" PACKAGE_ARCH="${PACKAGE_ARCH}" TZ_IMGVFY_BINRARY_OUT="${TZ_IMGVFY_BINRARY_OUT}" install
    oe_runmake -C secure_api PREFIX="${prefix}" DESTDIR="${D}" PACKAGE_ARCH="${PACKAGE_ARCH}" TZ_SECURE_API_BINRARY_OUT="${TZ_SECURE_API_BINRARY_OUT}" install

}



INSANE_SKIP_${PN} += "already-stripped"

