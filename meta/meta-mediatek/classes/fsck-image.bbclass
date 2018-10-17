
do_fsck_image_pre_funcs() {
    cp ${IMGDEPLOYDIR}/${IMAGE_NAME}.rootfs.${IMAGE_FSTYPES} ${DEPLOY_DIR_IMAGE}/rootfs.${IMAGE_FSTYPES}
    ${STAGING_DIR_NATIVE}${base_sbindir}/fsck.ext4 -f -y ${DEPLOY_DIR_IMAGE}/rootfs.${IMAGE_FSTYPES} || : 
    ${STAGING_DIR_NATIVE}${base_sbindir}/fsck.ext4 -f -y ${DEPLOY_DIR_IMAGE}/rootfs.${IMAGE_FSTYPES} 
}

do_image_complete[prefuncs] += "do_fsck_image_pre_funcs"

