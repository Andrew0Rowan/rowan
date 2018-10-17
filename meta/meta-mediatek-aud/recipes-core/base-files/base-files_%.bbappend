FILESEXTRAPATHS_prepend := "${THISDIR}/${PN}:"

do_install_append() {
    ln -sf ../usr/share/zoneinfo/UTC ${D}${sysconfdir}/localtime
    ln -sf ../proc/self/mounts ${D}${sysconfdir}/mtab
}
