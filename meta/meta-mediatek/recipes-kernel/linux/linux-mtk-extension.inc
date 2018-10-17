inherit kernel externalsrc kernel-fitimage-extension recovery-kernel-fitimage

DEPENDS_append_aarch64 = " libgcc"
KERNEL_CC_append_aarch64 = " ${TOOLCHAIN_OPTIONS}"
KERNEL_LD_append_aarch64 = " ${TOOLCHAIN_OPTIONS}"

LICENSE = "GPLv2"
LIC_FILES_CHKSUM = "file://COPYING;md5=d7810fab7487fb0aad327b76f1be7cd7"
EXTERNALSRC = "${KERNEL_SRC}"
STAGING_KERNEL_DIR = "${KERNEL_SRC}"
LINUX_VERSION_EXTENSION = "-custom"
KERNEL_EXTRA_ARGS = "dtbs"
KERNEL_CONFIG_COMMAND = "oe_runmake_call -C ${S} O=${B} ${KBUILD_DEFCONFIG}"

SRC_IMG_KEY_FILE = "${MTK_KEY_DIR}/${VERIFIED_KEY}.pem"
DST_IMG_KEY_FILE = "${TOPDIR}/../src/devtools/nfsb/rsa.key"
MTD_DST_IMG_KEY_FILE = "${TOPDIR}/../src/devtools/nfsb/mtd_verity/rsa.key"

do_configure_prepend() {
	install -d ${TMPDIR}/work-shared/${MACHINE}
	ln -nfs ${STAGING_KERNEL_DIR} ${TMPDIR}/work-shared/${MACHINE}/kernel-source
}

do_compile_prepend() {

       #backwards compatible for FORCE_DISABLE_DM_VERITY which only for NFSB actually
       if [ "${SECURE_BOOT_ENABLE}" = "yes" ] && [ "${SECURE_BOOT_TYPE}" = "" ]; then
              ENABLE_DM_NFSB="yes"
       fi
       if [ "${FORCE_DISABLE_DM_VERITY}" = "yes" ]; then
              ENABLE_DM_NFSB="no"
       fi
       echo "SECURE_BOOT_ENABLE is ${SECURE_BOOT_ENABLE},SECURE_BOOT_TYPE is ${SECURE_BOOT_TYPE}"
       if [ "${SECURE_BOOT_ENABLE}" = "yes" ] && [ "${SECURE_BOOT_TYPE}" = "avb" ]; then
              ENABLE_DM_VERITY="yes"
       fi
       echo "ENABLE_DM_VERITY is ${ENABLE_DM_VERITY}"
       if [ "${SECURE_BOOT_ENABLE}" = "yes" ] && [ "${ENABLE_DM_NFSB}" = "yes" ]; then
              if ! grep -Fxq "CONFIG_DM_NFSB=y" ${B}/.config ; then
                     echo "Error: If SECURE_BOOT_ENABLE is set to yes,kernel must be configed CONFIG_DM_NFSB=y"
                     exit 1
              fi

              if test -e ${SRC_IMG_KEY_FILE}; then
                     cp ${SRC_IMG_KEY_FILE} ${DST_IMG_KEY_FILE}
                     python ${TOPDIR}/../src/devtools/nfsb/pretreat-key.py ${TOPDIR} ${KERNEL_SRC}
              fi
       else
              if grep -Fxq "CONFIG_DM_NFSB=y" ${B}/.config ; then
                     echo "Error: If SECURE_BOOT_ENABLE is set to no,kernel must be configed CONFIG_DM_NFSB=n"
                     exit 1
              fi
       fi

       if [ "${SECURE_BOOT_ENABLE}" = "yes" ] && [ "${ENABLE_DM_VERITY}" = "yes" ]; then
            if ! grep -Fxq "CONFIG_DM_VERITY=y" ${B}/.config ; then
                     echo "Error: If SECURE_BOOT_ENABLE is set to yes,kernel must be configed CONFIG_DM_VERITY=y"
                     exit 1
            fi
       fi

       if [ "${SECURE_BOOT_ENABLE}" = "yes" ] && [ "${ENABLE_MTD_VERITY}" = "yes" ]; then
              if ! grep -Fxq "CONFIG_MTD_VERITY=y" ${B}/.config ; then
                     echo "Error: If SECURE_BOOT_ENABLE and ENABLE_MTD_VERITY is set to yes,kernel must be configed CONFIG_MTD_VERITY=y"
                     exit 1
              fi

              if test -e ${SRC_IMG_KEY_FILE}; then
                     cp ${SRC_IMG_KEY_FILE} ${MTD_DST_IMG_KEY_FILE}
                     python ${TOPDIR}/../src/devtools/nfsb/mtd_verity/pretreat-key.py ${TOPDIR}
              fi
       else
              if grep -Fxq "CONFIG_MTD_VERITY=y" ${B}/.config ; then
                     echo "Error: If SECURE_BOOT_ENABLE and ENABLE_MTD_VERITY is set to no,kernel must be configed CONFIG_MTD_VERITY=n"
                     exit 1
              fi
       fi
}
