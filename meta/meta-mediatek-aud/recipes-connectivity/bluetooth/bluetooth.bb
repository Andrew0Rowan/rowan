inherit workonsrc

DESCRIPTION = "Mediatek modified bluetooth"
LICENSE = "Apache"
LIC_FILES_CHKSUM = "file://LICENSE;md5=e1696b147d49d491bcb4da1a57173fff"
DEPENDS += "alsa-lib zlib nvram ${@bb.utils.contains('LICENSE_FLAGS_AAC', 'yes', 'aac', '' ,d)}"
WORKONSRC = "${TOPDIR}/../src/connectivity/bt_others/bluetooth_tool"

inherit systemd
SYSTEMD_PACKAGES = "${PN}"

SYSTEMD_SERVICE_${PN} = "${@bb.utils.contains('BT_NOT_INIT', 'yes', '', '${BT_LAUNCHER_SUFFIX}btservice.service', d)}"
FILES_${PN} += "${@bb.utils.contains('BT_NOT_INIT', 'yes', '', '${systemd_unitdir}/system/${BT_LAUNCHER_SUFFIX}btservice.service', d)}"

do_compile() {
        echo $PWD
        echo bluetooth start compile
        echo ${WORKONSRC}
        if ${@bb.utils.contains('LICENSE_FLAGS_AAC', 'yes', 'true', 'false' ,d)}; then
            export SUPPORT_AAC="yes"
        else
            export SUPPORT_AAC="no"
        fi
        export SUPPORT_SPP="no"
        export SUPPORT_HIDH="no"
        export SUPPORT_HIDD="no"
        export SUPPORT_GATT="yes"
        export SUPPORT_AVRCP="yes"
        export SUPPORT_A2DP_SRC="yes"
        export SUPPORT_A2DP_ADEV="yes"
        export SUPPORT_A2DP_SINK="yes"
        export SUPPORT_HFP_CLIENT="yes"
        export SUPPORT_BT_WIFI_RATIO_SETTING="yes"
        export SUPPORT_DISPATCH_A2DP_WITH_PLAYBACK="yes"
        cd ${WORKONSRC}/script
        sh generate_environment.sh ${COMBO_CHIP_ID}
        cd ${S}
        echo support aac codec ? ${SUPPORT_AAC}
        sh ${S}/script/yocto_build_bluetooth.sh ${TOPDIR} ${COMBO_CHIP_ID} ${MTK_PROJECT}
        if [ $? -ne 0 ]; then
            echo bluetooth compile fail!!
            exit 1
        fi
        echo bluetooth end compile
}

do_install() {
    install -d ${D}${libdir}
    install -m 755 ${WORKONSRC}/prebuilts/lib/* ${D}${libdir}/

    install -d ${D}/usr/bin
    install -m 0755 ${WORKONSRC}/prebuilts/bin/* ${D}/usr/bin/

    install -d ${D}/data/misc/bluedroid
    install -m 0644 ${WORKONSRC}/prebuilts/conf/* ${D}/data/misc/bluedroid/

    if ${@bb.utils.contains('BT_NOT_INIT','yes','false','true',d)}; then
        if ${@bb.utils.contains('DISTRO_FEATURES','systemd','true','false',d)}; then
            install -d ${D}${systemd_unitdir}/system/
            install -m 0644 ${B}/${BT_LAUNCHER_SUFFIX}btservice.service ${D}${systemd_unitdir}/system
        fi
    fi
    sh ${S}/script/yocto_clean_bluetooth.sh ${TOPDIR} ${COMBO_CHIP_ID}
}

FILES_${PN} += "/data/misc/bluedroid ${libdir}"
FILES_${PN}-dev = ""
INSANE_SKIP_${PN} += "ldflags"
