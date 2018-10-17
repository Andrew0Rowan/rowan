inherit workonsrc

DESCRIPTION = "bluetooth mesh"
LICENSE = "Airoha"
LIC_FILES_CHKSUM = "file://LICENSE;md5=e1696b147d49d491bcb4da1a57173fff"
DEPENDS += "${@base_contains('LICENSE_FLAGS_BLUEZ', 'yes', 'bluez5', 'bluetooth' ,d)}"
WORKONSRC = "${TOPDIR}/../src/connectivity/bt_others/ble_mesh"

do_compile() {
    echo $PWD
    echo mesh start compile
    make TOPDIR=${TOPDIR} MTK_PROJECT=${MTK_PROJECT} WORKONSRC=${WORKONSRC}
    echo mesh end compile
}

do_install() {
    oe_runmake DESTDIR="${D}" "WORKONSRC=${WORKONSRC}" TOPDIR="${TOPDIR}" install
}

FILES_${PN} += "/data/misc/mesh /usr/bin ${libdir}/"
FILES_${PN}-dev = ""
INSANE_SKIP_${PN} += "already-stripped"
INSANE_SKIP_${PN} += "ldflags"
