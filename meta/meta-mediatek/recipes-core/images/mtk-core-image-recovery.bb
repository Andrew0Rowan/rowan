SUMMARY = "A small image just capable of allowing a device to boot."

IMAGE_INSTALL = "packagegroup-core-boot ${ROOTFS_PKGMANAGE_BOOTSTRAP} ${CORE_IMAGE_EXTRA_INSTALL}"

IMAGE_LINGUAS = " "

LICENSE = "MIT"

inherit core-image

DEPENDS += "virtual/kernel"

IMAGE_ROOTFS_SIZE ?= "8192"

IMAGE_INSTALL_append = " \
              util-linux \
              udev-extraconf \
              recovery-scripts \
"
BAD_RECOMMENDATIONS += "busybox-syslog"

install_recovery_fstab() {
    cat > ${IMAGE_ROOTFS}${sysconfdir}/fstab <<EOF
/dev/root            /                    auto       ro                    0  0
proc                 /proc                proc       defaults              0  0
devpts               /dev/pts             devpts     mode=0620,gid=5       0  0
tmpfs                /run                 tmpfs      mode=0755,nodev,nosuid,strictatime 0  0
tmpfs                /var/volatile        tmpfs      defaults              0  0
EOF
}

soft_link_to_recovery_image() {
        ln -nfs ${IMAGE_NAME}.rootfs.${IMAGE_FSTYPES} ${DEPLOY_DIR_IMAGE}/recovery.${IMAGE_FSTYPES}
}

ROOTFS_POSTPROCESS_COMMAND += " install_recovery_fstab; soft_link_to_recovery_image;"

do_rootfs[depends] += "virtual/kernel:do_deploy"

inherit recovery-kernel-fitimage
