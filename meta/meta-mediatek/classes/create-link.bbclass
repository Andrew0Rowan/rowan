soft_link_to_rootfs() {
        ln -nfs ${IMAGE_NAME}.rootfs.${IMAGE_FSTYPES} ${DEPLOY_DIR_IMAGE}/rootfs.${IMAGE_FSTYPES}
}

ROOTFS_POSTPROCESS_COMMAND += " soft_link_to_rootfs;"
