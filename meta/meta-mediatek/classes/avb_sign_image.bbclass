AVBTOOL="${DEPLOY_DIR_IMAGE}/avbtool"
SCATTER_FILE="${DEPLOY_DIR_IMAGE}/partition_emmc.xml"

IMAGE_FSTYPE="img"
IMAGE_DIR_ROOTFS="${DEPLOY_DIR_IMAGE}"

PARTITION_NAME_ROOTFS="ROOTFS"
IMAGE_NAME_ROOTFS="rootfs.${IMAGE_FSTYPE}"

IMAGE_NAME_BOOTIMG="boot.img"
PARTITION_NAME_BOOTIMG="BOOTIMG"
IMAGE_DIR_BOOTIMG="${DEPLOY_DIR_IMAGE}"

AVB_BLOCK_SIZE="1024"
AVB_ALGO="SHA256_RSA2048"
AVB_SYSTEM_KEY_NAME="verified_key"
AVB_SYSTEM_KEY="${MTK_KEY_DIR}/${VERIFIED_KEY}.pem"

DEPENDS += "python-pycrypto-native android-tools-avbtool scatter"

exec_avbtool() {

	echo "check config setting: enable=${SECURE_BOOT_ENABLE};SE_type=${SECURE_BOOT_TYPE}"
	if [ "${SECURE_BOOT_ENABLE}" = "yes" ] && [ "${SECURE_BOOT_TYPE}" = "avb" ]; then
		echo "start AVB sign ... "
		echo "PARTITION_SIZE_BOOTIMG=${PARTITION_SIZE_BOOTIMG}; PARTITION_SIZE_ROOTFS=${PARTITION_SIZE_ROOTFS}"

		if [ "${AVB_ANTIROLLBACK_VERSION}" = "" ]; then
			AVB_ANTIROLLBACK_VERSION="0"
		fi

		echo "start to erase footer: ${IMAGE_NAME_BOOTIMG}"
		${AVBTOOL} erase_footer --image ${IMAGE_DIR_BOOTIMG}/${IMAGE_NAME_BOOTIMG} || true

		echo "start to erase footer: rootfs.${IMAGE_FSTYPE} "
		${AVBTOOL} erase_footer --image ${IMAGE_DIR_ROOTFS}/rootfs.${IMAGE_FSTYPE} || true

		if test -e ${IMAGE_DIR_BOOTIMG}/${IMAGE_NAME_BOOTIMG}; then
			echo "start to sign image: ${IMAGE_NAME_BOOTIMG}"
			${AVBTOOL} add_hash_footer --image ${IMAGE_DIR_BOOTIMG}/${IMAGE_NAME_BOOTIMG} \
				--partition_size ${PARTITION_SIZE_BOOTIMG} \
				--partition_name ${PARTITION_NAME_BOOTIMG} \
				--algorithm ${AVB_ALGO} \
				--key ${AVB_SYSTEM_KEY}
		fi

		if test -e ${IMAGE_DIR_ROOTFS}/rootfs.${IMAGE_FSTYPE}; then
			echo "start to sign image: rootfs.${IMAGE_FSTYPE} "
			${AVBTOOL} add_hashtree_footer --block_size ${AVB_BLOCK_SIZE} \
				--partition_size ${PARTITION_SIZE_ROOTFS} \
				--partition_name ${PARTITION_NAME_ROOTFS} \
				--image ${IMAGE_DIR_ROOTFS}/rootfs.${IMAGE_FSTYPE} \
				--algorithm ${AVB_ALGO} \
				--key ${AVB_SYSTEM_KEY}

			if test -e ${IMAGE_DIR_BOOTIMG}/${IMAGE_NAME_BOOTIMG}; then
				echo "start to make vbmeta image"
				${AVBTOOL} make_vbmeta_image --rollback_index ${AVB_ANTIROLLBACK_VERSION} \
					--include_descriptors_from_image ${IMAGE_DIR_BOOTIMG}/${IMAGE_NAME_BOOTIMG} \
					--include_descriptors_from_image ${IMAGE_DIR_ROOTFS}/rootfs.${IMAGE_FSTYPE} \
					--setup_rootfs_from_kernel ${IMAGE_DIR_ROOTFS}/rootfs.${IMAGE_FSTYPE} \
					--algorithm ${AVB_ALGO} \
					--key ${AVB_SYSTEM_KEY} \
					--output vbmeta.img

				echo "start to append vbmeta image"
				${AVBTOOL} append_vbmeta_image --image ${IMAGE_DIR_BOOTIMG}/${IMAGE_NAME_BOOTIMG} \
					--partition_size ${PARTITION_SIZE_BOOTIMG} \
					--vbmeta_image vbmeta.img
			fi
		fi
	else
		exit 0
	fi
}

python do_avb_sign_img() {
	import xml.dom.minidom

	secure_boot_type = d.getVar('SECURE_BOOT_TYPE', True)
	if secure_boot_type != "avb":
		return

	scatter_file = d.getVar('SCATTER_FILE', True)
	root = xml.dom.minidom.parse(scatter_file)
	for partition in root.childNodes:
		if partition.nodeName == "partition":
			break
	else:
		raise Exception("partition not found")

	lbs = partition.getAttribute("lbs")
	lbs = lbs and eval(lbs) or 512
	for node in partition.childNodes:
		if node.nodeName != "entry":
			continue
		start = eval(node.getAttribute("start"))
		end = eval(node.getAttribute("end"))
		name = node.getAttribute("name")
		size = (end-start+1)*lbs
		if name == "BOOTIMG":
			d.setVar('PARTITION_SIZE_BOOTIMG', str(size))
		if name == "ROOTFS":
			d.setVar('PARTITION_SIZE_ROOTFS', str(size))
	bb.build.exec_func('exec_avbtool', d)
}

addtask avb_sign_img after do_image_qa before do_build
