STATE_DIR_IMAGE_SIZE ?= "131072"
STATE_DIR_IMAGE_PATH = "${DEPLOY_DIR_IMAGE}/state"
STATE_DIR_CONTENT ?= "${IMAGE_ROOTFS}/home ${IMAGE_ROOTFS}/var ${IMAGE_ROOTFS}/usr/lib/tmpfiles.d"
IMAGE_FSTYPES_NO_WHITESPACE="$(echo "${IMAGE_FSTYPES}" | tr -d '[:space:]')"

mk_state_image() {
    rm -rf ${STATE_DIR_IMAGE_PATH}
    install -d ${STATE_DIR_CONTENT} ${STATE_DIR_IMAGE_PATH}/usr/local ${STATE_DIR_IMAGE_PATH}/srv
    for i in ${STATE_DIR_CONTENT}; do
        STATE_DIR_FILE_PATH=${STATE_DIR_IMAGE_PATH}/${i##${IMAGE_ROOTFS}}
        install -d ${STATE_DIR_FILE_PATH}
        cp -a ${i}/* ${STATE_DIR_FILE_PATH}
    done

    if test "${IMAGE_FSTYPES_NO_WHITESPACE}" = "ubi"; then
        STATE_PARTITION="state.ubifs"
        STATE_UBI="state.ubi"
        STATE_UBINIZE_CFG="ubinize-state.cfg"
        dd if=/dev/zero of=${DEPLOY_DIR_IMAGE}/${STATE_PARTITION} seek=${STATE_DIR_IMAGE_SIZE} count=0 bs=1k
        cat << EOF > ${STATE_UBINIZE_CFG}
[ubifs]
mode=ubi
image=${DEPLOY_DIR_IMAGE}/${STATE_PARTITION}
vol_id=0
vol_type=dynamic
vol_name=state
vol_flags=autoresize
EOF
        mkfs.ubifs -r ${STATE_DIR_IMAGE_PATH} -o ${DEPLOY_DIR_IMAGE}/${STATE_PARTITION} ${MKUBIFS_ARGS}
        ubinize -o ${DEPLOY_DIR_IMAGE}/${STATE_UBI} ${UBINIZE_ARGS} ${STATE_UBINIZE_CFG}
    elif test "${IMAGE_FSTYPES_NO_WHITESPACE}" = "ext4"; then
        STATE_PARTITION="state.ext4"
        dd if=/dev/zero of=${DEPLOY_DIR_IMAGE}/${STATE_PARTITION} seek=${STATE_DIR_IMAGE_SIZE} count=0 bs=1k
        mkfs.ext4 -F -i 4096 ${DEPLOY_DIR_IMAGE}/${STATE_PARTITION} -d ${STATE_DIR_IMAGE_PATH}
    else
        echo "No method to make ${IMAGE_FSTYPES_NO_WHITESPACE} type state image"
    fi
}

ROOTFS_POSTPROCESS_COMMAND += " mk_state_image;"

mk_rootfs_folder() {
    install -d ${IMAGE_ROOTFS}/mnt/STATE ${IMAGE_ROOTFS}/srv
    if test "${DISTRO}" != "poky-agl"; then
        install -d ${IMAGE_ROOTFS}/usr/local
    fi
}

ROOTFS_PREPROCESS_COMMAND += " mk_rootfs_folder;"
