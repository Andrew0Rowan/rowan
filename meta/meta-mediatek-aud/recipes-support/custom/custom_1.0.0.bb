#Basic Configuration
DESCRIPTION = "Customization."
SECTION = "base"
LICENSE = "MediaTekProprietary"
LIC_FILES_CHKSUM = "file://LICENSE;md5=e1696b147d49d491bcb4da1a57173fff"
MTK_SRC = "${TOPDIR}/../src/support/libnvram_custom/CFG"

DEPENDS = "nvraminit"

WORKONSRC = "${MTK_SRC}"

inherit deploy workonsrc

#Parameters passed to do_compile()
EXTRA_OEMAKE = "'CROSS=${TARGET_PREFIX}'\
                'PROJECT=${PROJ}'\
                'PLATFORM_CHIP=${PLATFORM_CHIP}'\
                'PRJ_FILENAME=${PRJ_FILENAME}'\
                'COMPLIST=${COMPLIST}'\
                'LINUX_KERNEL=${LINUX_KERNEL}'"

ALLOW_EMPTY_${PN} = "1"

FILES_${PN}-dev = "*"
FILES_${PN}  = "*"

do_install () {
    oe_runmake install ROOT=${D}
    install -d ${D}${includedir}
    cp -af ${D}/include/* ${D}${includedir}
    rm -rf ${D}/include

}

addtask nvramclean
do_nvramclean () {
    oe_runmake clean
}
