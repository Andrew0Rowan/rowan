IMAGE_PATH = "${DEPLOY_DIR_IMAGE}"
IMAGE_LIST = "${IMAGE_PATH}/tz.img  \
              ${IMAGE_PATH}/boot.img  \
              ${IMAGE_PATH}/rootfs.ext4 \
              ${IMAGE_PATH}/rootfs.ubi "
DEPENDS += "zip-native"

do_packing_images_post_funcs() {
        zip -j ${IMAGE_PATH}/update.zip ${IMAGE_LIST}
}

addtask packing_images_post_funcs after do_image_qa before do_build

