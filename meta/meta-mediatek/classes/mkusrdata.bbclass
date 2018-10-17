STATE_DIR_IMAGE_SIZE ?= "${MKUBIFS_USRDATA_IMAGE_SZ}"
STATE_DIR_IMAGE_PATH = "${DEPLOY_DIR_IMAGE}/userdata"
STATE_DIR_CONTENT = "${IMAGE_ROOTFS}/../usrdata"
IMAGE_FSTYPES_NO_WHITESPACE="$(echo "${IMAGE_FSTYPES}" | tr -d '[:space:]')"
USERDATA_FS_PARTITION = "userdata_fs.ubi"
USERDATA_PARTITION = "userdata.ubi"

mk_userdata_image() {
	mkdir -p ${STATE_DIR_CONTENT}/tmp
	mkdir -p ${STATE_DIR_CONTENT}/etc
	mkdir -p ${STATE_DIR_CONTENT}/var
	mkdir -p ${STATE_DIR_CONTENT}/usr
	mkdir -p ${STATE_DIR_CONTENT}/usr/bin
	mkdir -p ${STATE_DIR_CONTENT}/usr/sbin
	mkdir -p ${STATE_DIR_CONTENT}/config
	mkdir -p ${STATE_DIR_CONTENT}/misc/public/bluetooth_hfp/

	if [ -e ${IMAGE_ROOTFS}/usr/bin/algo-data-server ]; then
	    mkdir -p ${STATE_DIR_CONTENT}/mnano
	    cp -af ${TOPDIR}/../prebuilt/support/assistant-sdk/mpctec/mnpc_v2.dat ${STATE_DIR_CONTENT}/mnano
	fi

	if [ -e ${IMAGE_ROOTFS}/usr/bin/WenzhiDemo ]; then
	mkdir -p ${STATE_DIR_CONTENT}/wenzhi
	mkdir -p ${STATE_DIR_CONTENT}/wenzhi/model
	mkdir -p ${STATE_DIR_CONTENT}/wenzhi/wav

  cp -af ${TOPDIR}/../prebuilt/support/assistant-sdk/wenzhi/model/* ${STATE_DIR_CONTENT}/wenzhi/model
  cp -af ${TOPDIR}/../prebuilt/support/assistant-sdk/wenzhi/wav/* ${STATE_DIR_CONTENT}/wenzhi/wav
  fi

	if [ ! -d ${IMAGE_ROOTFS}/data/misc ]; then
		install -d ${IMAGE_ROOTFS}/data/misc
	fi

	cp -af ${TOPDIR}/../src/apps/aud-base/public/auto_set_mac.sh ${STATE_DIR_CONTENT}/config
	cp -af ${IMAGE_ROOTFS}/data/misc ${STATE_DIR_CONTENT}	
	if [ -f ${IMAGE_ROOTFS}/usr/lib/public/bluetooth_hfp/Sleep_Away.mp3 ]; then
	  cp -a ${IMAGE_ROOTFS}/usr/lib/public/bluetooth_hfp/Sleep_Away.mp3 ${STATE_DIR_CONTENT}/misc/public/bluetooth_hfp/
	  rm -rf ${IMAGE_ROOTFS}/usr/lib/public/bluetooth_hfp/Sleep_Away.mp3
	fi

	cp -a ${IMAGE_ROOTFS}/usr/sbin/tcpdump ${STATE_DIR_CONTENT}/usr/sbin
	rm -f ${IMAGE_ROOTFS}/usr/sbin/tcpdump

	cp -a ${IMAGE_ROOTFS}/etc/wpa_supplicant.conf ${STATE_DIR_CONTENT}/etc/wpa_supplicant.conf
	cp -a ${IMAGE_ROOTFS}/usr/bin/adbd ${STATE_DIR_CONTENT}/usr/bin
	cp -a ${IMAGE_ROOTFS}/usr/bin/android-gadget-setup ${STATE_DIR_CONTENT}/usr/bin
	rm -rf ${IMAGE_ROOTFS}/usr/bin/adbd
	rm -rf ${IMAGE_ROOTFS}/usr/bin/android-gadget-setup

	if [ -f ${IMAGE_ROOTFS}/usr/bin/hcidump ]; then
		cp -a ${IMAGE_ROOTFS}/usr/bin/hcidump ${STATE_DIR_CONTENT}/usr/sbin
		rm -f ${IMAGE_ROOTFS}/usr/bin/hcidump
	fi

	if [ -f ${IMAGE_ROOTFS}/usr/bin/iperf ]; then
		cp -a ${IMAGE_ROOTFS}/usr/bin/iperf ${STATE_DIR_CONTENT}/usr/bin
		rm -rf ${IMAGE_ROOTFS}/usr/bin/iperf
	fi

	rm -rf ${STATE_DIR_IMAGE_PATH}
	install -d ${STATE_DIR_CONTENT}
    for i in ${STATE_DIR_CONTENT}; do
        STATE_DIR_FILE_PATH=${STATE_DIR_IMAGE_PATH}/${i##${STATE_DIR_CONTENT}}
        install -d ${STATE_DIR_FILE_PATH}
        if [ "$(ls -A ${i})" ]; then
                cp -a ${i}/* ${STATE_DIR_FILE_PATH}
        else
                echo "${i} is empty"
        fi
    done
    if test "${IMAGE_FSTYPES_NO_WHITESPACE}" = "ubi"; then
	echo \[ubifs\] > ubinize.cfg
	echo mode=ubi >> ubinize.cfg
	echo image=${USERDATA_FS_PARTITION} >> ubinize.cfg
	echo vol_id=0 >> ubinize.cfg
	echo vol_size=${MKUBIFS_USRDATA_VOL_SZ}
	echo vol_type=dynamic >> ubinize.cfg
	echo vol_name=useradata >> ubinize.cfg
	echo vol_flags=autoresize >> ubinize.cfg
	dd if=/dev/zero of=${DEPLOY_DIR_IMAGE}/${USERDATA_FS_PARTITION} seek=${STATE_DIR_IMAGE_SIZE} count=0 bs=1k
	mkfs.ubifs -r ${STATE_DIR_FILE_PATH} ${MKUBIFS_USRDATA_ARGS} -o ${USERDATA_FS_PARTITION}
	#ubinize -o ${USERDATA_PARTITION} -m 2048 -p 128KiB -s 2048 ubinize.cfg
	ubinize -o ${USERDATA_PARTITION} ${UBINIZE_ARGS} ubinize.cfg
	cp ${USERDATA_PARTITION} ${DEPLOY_DIR_IMAGE}/${USERDATA_PARTITION}
    elif test "${IMAGE_FSTYPES_NO_WHITESPACE}" = "ext4"; then
	STATE_PARTITION="userdata.ext4"
	dd if=/dev/zero of=${DEPLOY_DIR_IMAGE}/${STATE_PARTITION} seek=${STATE_DIR_IMAGE_SIZE} count=204800 bs=1k
	mkfs.ext4 -F -i 4096 ${DEPLOY_DIR_IMAGE}/${STATE_PARTITION} -d ${STATE_DIR_IMAGE_PATH}
    else
	echo "No method to make ${IMAGE_FSTYPES_NO_WHITESPACE} type state image"
    fi
}

ROOTFS_POSTPROCESS_COMMAND += " mk_userdata_image;"


