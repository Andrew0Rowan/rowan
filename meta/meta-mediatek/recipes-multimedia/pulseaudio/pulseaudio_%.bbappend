FILESEXTRAPATHS_append := ":${THISDIR}/${PN}"

SRC_URI += "file://daemon.conf \
            file://pulseaudio_user.service \
"

do_install_append() {
	install -m 0644 ${WORKDIR}/daemon.conf ${D}${sysconfdir}/pulse/daemon.conf

	if [ "${MTK_AUDIO_SYSTEM}" = "mt2701-cs42448" ]; then
                sed -e 's/; default-sample-channels = 2/default-sample-channels = 8/g' \
                -i ${D}${sysconfdir}/pulse/daemon.conf
	fi
        if [ "${MTK_AUDIO_SYSTEM}" = "mt2712-d1v1" ]; then
                sed -e 's/; default-sample-channels = 2/default-sample-channels = 8/g' \
                -i ${D}${sysconfdir}/pulse/daemon.conf
                sed -e 's/; default-sample-format = s16le/default-sample-format = s32le/g' \
                -i ${D}${sysconfdir}/pulse/daemon.conf
                sed -e 's/; default-sample-rate = 44100/default-sample-rate = 48000/g' \
                -i ${D}${sysconfdir}/pulse/daemon.conf
	fi

	mkdir -p ${D}/etc/systemd/user
	cp ${WORKDIR}/pulseaudio_user.service ${D}/etc/systemd/user/pulseaudio.service
	mkdir -p ${D}/etc/systemd/user/default.target.wants
	ln -sf /etc/systemd/user/pulseaudio.service ${D}/etc/systemd/user/default.target.wants/pulseaudio.service
}
