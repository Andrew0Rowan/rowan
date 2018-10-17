add_boot_image() {
    install -d ${IMAGE_ROOTFS}/boot/
    install ${DEPLOY_DIR_IMAGE}/boot.img ${IMAGE_ROOTFS}/boot/
}
