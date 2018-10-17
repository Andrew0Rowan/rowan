inherit deploy srcprebuilt externalsrc

DESCRIPTION = "Scatter File for FlashTool"
LICENSE = "MediaTekProprietary"
MTK_SRC = "${TOPDIR}/../src/bsp/scatter"
SCATTER_SCRIPTS_DIR = "${MTK_SRC}/scripts"
LIC_FILES_CHKSUM = "file://${MTK_SRC}/README;md5=ee4e5f73850b12ab5da2fa41dc560729"
EXTERNALSRC = "${MTK_SRC}"
EXTERNALSRC_BUILD = "${MTK_SRC}"
SCATTER_OUT = "${WORKDIR}/out"
NAND_HEADER_VERSION ?= "1.0"

do_compile () {
        ${SCATTER_SCRIPTS_DIR}/gen-partitions.sh ${WORKDIR}/${SCATTER_PROJECT} \
        ${SCATTER_OUT} ${BOOTDEV_TYPE} ${NAND_CHIP_NAME} ${NAND_HEADER_VERSION}
}

do_deploy () {
        install -d ${DEPLOYDIR}
        install ${SCATTER_OUT}/* -t ${DEPLOYDIR}
        install ${WORKDIR}/${SCATTER_PROJECT}/*.xml -t ${DEPLOYDIR}
}

addtask deploy before do_build after do_compile
