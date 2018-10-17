DEPENDS += "android-tools-fsutils-native"

do_sparse_image() {
# Usage: img2simg <raw_image_file> <sparse_image_file> [<block_size>]
if test -e ${IMGDEPLOYDIR}/${IMAGE_NAME}.rootfs.${IMAGE_FSTYPES} ; then
    img2simg ${IMGDEPLOYDIR}/${IMAGE_NAME}.rootfs.${IMAGE_FSTYPES} ${DEPLOY_DIR_IMAGE}/rootfs.img
    cp ${IMGDEPLOYDIR}/${IMAGE_NAME}.rootfs.${IMAGE_FSTYPES} ${DEPLOY_DIR_IMAGE}/rootfs.${IMAGE_FSTYPES}

    if test -e ${DEPLOY_DIR_IMAGE}/sign_rootfs ; then
	rm -rf ${DEPLOY_DIR_IMAGE}/sign_rootfs
    fi
fi
}

do_image_complete[postfuncs] += "do_sparse_image"
