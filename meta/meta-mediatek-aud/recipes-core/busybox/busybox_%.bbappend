FILESEXTRAPATHS_append := ":${THISDIR}/${PN}"

SRC_URI_append = " \
                   file://syslogctl \
                   file://mdlogctl \
                   file://syslog-start \
                   file://klogd-start \
                   file://busybox-syslog.default \
                   file://add-syslog.cfg \
                   file://busybox-syslog.service.in \
                   file://busybox-klogd.service.in \
                   file://tcpdump-start \
                   file://tcpdump.service.in \
                   file://ring_buf.patch \
                   file://echo.patch \
                   file://syslog.conf \
                 "

inherit systemd

FILES_${PN}-syslog_append = " ${systemd_unitdir}/system/busybox-syslog.service \
                              ${systemd_unitdir}/system/busybox-klogd.service \
                              ${systemd_unitdir}/system/tcpdump.service \
                            "

do_install_append() {
    cp ${WORKDIR}/syslogctl ${D}/bin/syslogctl
    cp ${WORKDIR}/mdlogctl ${D}/bin/mdlogctl
    cp ${WORKDIR}/syslog-start ${D}/bin/syslog-start
    cp ${WORKDIR}/klogd-start ${D}/bin/klogd-start
    cp ${WORKDIR}/tcpdump-start ${D}/bin/tcpdump-start
    cp ${WORKDIR}/syslog.conf ${D}/etc/syslog.conf
    install -d ${D}${systemd_unitdir}/system
    install -d ${D}${sysconfdir}/systemd/system
    install -d ${D}${sysconfdir}/systemd/system/multi-user.target.wants
    install -m 0644 ${WORKDIR}/tcpdump.service.in ${D}${systemd_unitdir}/system/tcpdump.service
    ln -sf ${systemd_unitdir}/system/busybox-syslog.service ${D}${sysconfdir}/systemd/system/syslog.service
    ln -sf ${systemd_unitdir}/system/busybox-syslog.service ${D}${sysconfdir}/systemd/system/multi-user.target.wants/busybox-syslog.service
    ln -sf ${systemd_unitdir}/system/busybox-klogd.service ${D}${sysconfdir}/systemd/system/multi-user.target.wants/busybox-klogd.service
    ln -sf ${systemd_unitdir}/system/tcpdump.service ${D}${sysconfdir}/systemd/system/multi-user.target.wants/tcpdump.service
}

