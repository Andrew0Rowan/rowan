FILESEXTRAPATHS_append := ":${THISDIR}/${PN}"

SRC_URI += "file://0001-hmi-controller-use-output_w-h-instead-of-current_mod.patch \
            file://0002-libinput-fix-issue-for-touch_event-cause-weston-cras.patch \
            file://0003-compose-engine-turn-to-pixman-if-gl-init-fail.patch        \
            file://0004-notification-add-notification-interface-and-sample-f.patch \
            file://0005-weston-do-not-check-master-fd-for-DRM-driver.patch         \
            file://0006-weston-install-client-protocol-to-sysroot-for-other-.patch \
            file://0007-atomic-weston-support-atomic-mode.patch                    \
            file://0008-yuv-format-support-yuv-for-primary-and-overlay-path.patch  \
            file://0009-drm_prop-add-interface-for-mtk-drm-module.patch            \
            file://0010-overlay-enhancement-for-mtk-drm-module.patch               \
            file://0011-tool-add-mtk-performance-tool.patch                        \
            file://0012-mdp-renderer-weston-support-mdp-for-overlay-plane.patch    \
            file://0013-dump_plane-add-interface-and-implement-for-dump-buff.patch \
            file://0014-subsurface-update-subsurface-opacity-with-mainsurfac.patch \
            file://0015-weston-screenshooter-add-weston-screen-shooter.patch \
            file://rawdata.argb       \
            file://rawdata.nv12       \
            file://rawdata.nv16       \
            file://rawdata.rgb565     \
            file://rawdata.xrgb       \
            file://rawdata.yu12       \
            file://rawdata.yuyv       \
            file://rawdata.yv12       \
            file://rawdatatest.argb       \
"

DEPENDS += "libdrm"
FILES_${PN} += "${bindir}/* ${libdir}/weston/* ${sysconfdir}/xdg"
FILES_${PN}-dbg += "${libdir}/weston/.debug/*"

do_compile_prepend() {
	cp ${WORKDIR}/rawdata* ${S}/data/
}

do_compile() {
        oe_runmake \
                CFLAGS+="-I${STAGING_INCDIR}/libdrm"
}
do_install_append() {
    install -d ${D}/usr/share/weston/
    install -m 0644 ${WORKDIR}/rawdatatest.argb ${D}/usr/share/weston/

    WESTON_INI_CONFIG=${sysconfdir}/xdg/weston
    install -d ${D}${WESTON_INI_CONFIG}
    echo "" >> ${D}${WESTON_INI_CONFIG}/weston.ini
    echo "[output]" >> ${D}${WESTON_INI_CONFIG}/weston.ini
    echo "name=DSI-1" >> ${D}${WESTON_INI_CONFIG}/weston.ini
    echo "transform=90" >> ${D}${WESTON_INI_CONFIG}/weston.ini
}
