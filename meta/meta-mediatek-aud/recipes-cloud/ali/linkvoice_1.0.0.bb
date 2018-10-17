LICENSE = "ZelusTekProprietary"

DESCRIPTION = "Aliyun Link Voice SDK"

DEPENDS = "libopus libwebsockets cjson portaudio-v19"

inherit workonsrc

WORKONSRC = "${TOPDIR}/../src/cloud/ali/linkvoice"

do_install() {
    oe_runmake DESTDIR="${D}" install
}

INSANE_SKIP_${PN} += "ldflags dev-deps"
