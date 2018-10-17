#! /bin/sh

NAND_HEADER_VERSION=$1
NAND_CHIP_NAME=$2
PARTITION_XML=$3

if [ "${NAND_CHIP_NAME}" = "" ]; then
	echo "ERROR: please input NAND_CHIP_NAME"
	exit 1
else
	echo "NAND: ${NAND_CHIP_NAME}"
fi

if [ "${NAND_HEADER_VERSION}" = "" ]; then
	echo "ERROR: please input NAND_HEADER_VERSION"
	exit 2
else
	echo "NAND_HEADER_VERSION: ${NAND_HEADER_VERSION}"
	if [ "${NAND_HEADER_VERSION}" = "2.0" ]; then
		RANDOM_EN=0
	elif [ "${NAND_HEADER_VERSION}" = "1.1" ]; then
		RANDOM_EN=1
	fi
fi

if [ "${PARTITION_XML}" = "" ]; then
	echo "ERROR: please input PARTITION_XML"
	exit 3
else
	echo "PARTITION_XML: ${PARTITION_XML}"
fi

OUTPUT_FLD="output"
IMAGE_FLD="images"
BURN_SUFFIX="-burn"
BURN_IMG="${PWD}/${OUTPUT_FLD}/nand-burn.img"
BURN_LIST_CSV="${PWD}/${OUTPUT_FLD}/partition_table.csv"
BURN_LIST_DEF="${PWD}/${OUTPUT_FLD}/partition_table.def"
rm -rf ${PWD}/${OUTPUT_FLD}/*${BURN_SUFFIX}
rm -rf ${BURN_IMG}
touch ${BURN_IMG}

rm -rf ${BURN_LIST_CSV}
touch ${BURN_LIST_CSV}
rm -rf ${BURN_LIST_DEF}
touch ${BURN_LIST_DEF}

python ${PWD}/tools/nand-utils/gen_burner_partition_table_def.py ${NAND_CHIP_NAME} \
	${NAND_HEADER_VERSION} ${BURN_LIST_DEF} 0 0 0 0

LBS=$(grep -o 'lbs="[0-9]*"' ${PARTITION_XML} | grep -o [0-9]*)

MBR_NAND="MBR_NAND"
MBR_NAND_BRUN="MBR_NAND-burn"
PART_IDX=0

cat ${PARTITION_XML} | while read line
do
	IMAGE_NAME=""
	LBA_START=""
	LBA_END=""
	CHECK=$(echo $line | grep "name")
	if [ "${CHECK}" != "" ]; then
		PARTITION=${line#*name=}
		PARTITION=${PARTITION#*\"}
		PARTITION=${PARTITION%%\"*}
	else
		PARTITION=""
	fi
	if [ "${PARTITION}" = "UBOOT" ]; then
		IMAGE_NAME="lk.img"
	fi
	if [ "${PARTITION}" = "TEE1" -o "${PARTITION}" = "TEE2" ]; then
		IMAGE_NAME="tz.img"
	fi
	if [ "${PARTITION}" = "USRDATA" ]; then
		IMAGE_NAME="userdata.ubi"
	fi
	if [ "${PARTITION}" = "BOOTIMG1" -o "${PARTITION}" = "BOOTIMG2" ]; then
		IMAGE_NAME="boot.img"
	fi
	if [ "${PARTITION}" = "ROOTFS1" -o "${PARTITION}" = "ROOTFS2" ]; then
		IMAGE_NAME="rootfs.ubi"
	fi

	if [ "${PARTITION}" != "" ]; then
		echo ${PART_IDX} ${PARTITION}
		if [ "${IMAGE_NAME}" != "" -a ! -f "${PWD}/${IMAGE_FLD}/${IMAGE_NAME}" ]; then
			echo "file "${PWD}/${IMAGE_FLD}/${IMAGE_NAME}" not exist!! "
			IMAGE_NAME=""
		fi
		LBA_START=${line#*start=}
		LBA_START=${LBA_START#*\"}
		LBA_START=${LBA_START%%\"*}
		LBA_END=${line#*end=}
		LBA_END=${LBA_END#*\"}
		LBA_END=${LBA_END%%\"*}
		((LBA_NUM=${LBA_END}-${LBA_START}+1))
		((PART_SIZE=${LBS}*${LBA_NUM}))
		((PART_START_ADDR=${LBS}*${LBA_START}))

		if [ "${PART_IDX}" = "0" ]; then
			((MBR_PART_SIZE=${LBS}*${LBA_START}))
			((MBR_LBA_END=${LBA_START}-1))
			python ${PWD}/tools/nand-utils/gen_burner_image.pyc ${NAND_CHIP_NAME} \
				${PWD}/${IMAGE_FLD}/${MBR_NAND} ${PWD}/${OUTPUT_FLD}/${MBR_NAND_BRUN} ${NAND_HEADER_VERSION} ${RANDOM_EN}
			touch ${PWD}/${IMAGE_FLD}/${MBR_NAND_BRUN}
			cat ${PWD}/${OUTPUT_FLD}/${MBR_NAND_BRUN} >> ${PWD}/${IMAGE_FLD}/${MBR_NAND_BRUN}
			python ${PWD}/tools/nand-utils/pad_dummy_data_by_partition.pyc ${NAND_CHIP_NAME} \
				${PWD}/${IMAGE_FLD}/${MBR_NAND_BRUN} ${MBR_PART_SIZE} ${NAND_HEADER_VERSION}
			cat ${PWD}/${IMAGE_FLD}/${MBR_NAND_BRUN} > ${BURN_IMG}
			python ${PWD}/tools/nand-utils/gen_burner_partition_table_def.py ${NAND_CHIP_NAME} \
				${NAND_HEADER_VERSION} ${BURN_LIST_DEF} ${PWD}/${OUTPUT_FLD}/${MBR_NAND_BRUN} 0 ${MBR_LBA_END} 1
			python ${PWD}/tools/nand-utils/gen_burner_partition_table_csv.py ${NAND_CHIP_NAME} \
				${NAND_HEADER_VERSION} ${BURN_LIST_CSV} ${PWD}/${OUTPUT_FLD}/${MBR_NAND_BRUN} 0 ${MBR_LBA_END} "MBR_NAND"
		fi

	fi

	if [ "${IMAGE_NAME}" != "" ]; then
		PART_IDX=$((${PART_IDX}+1))
		if [ ! -f "${PWD}/${IMAGE_FLD}/${IMAGE_NAME}${BURN_SUFFIX}" ]; then
			python ${PWD}/tools/nand-utils/gen_burner_image.pyc ${NAND_CHIP_NAME} \
				${PWD}/${IMAGE_FLD}/${IMAGE_NAME} ${PWD}/${OUTPUT_FLD}/${IMAGE_NAME}${BURN_SUFFIX} ${NAND_HEADER_VERSION} ${RANDOM_EN}
			touch ${PWD}/${IMAGE_FLD}/${IMAGE_NAME}${BURN_SUFFIX}
			cat ${PWD}/${OUTPUT_FLD}/${IMAGE_NAME}${BURN_SUFFIX} >> ${PWD}/${IMAGE_FLD}/${IMAGE_NAME}${BURN_SUFFIX}
			python ${PWD}/tools/nand-utils/pad_dummy_data_by_partition.pyc ${NAND_CHIP_NAME} \
				${PWD}/${IMAGE_FLD}/${IMAGE_NAME}${BURN_SUFFIX} ${PART_SIZE} ${NAND_HEADER_VERSION}
		fi
		cat ${PWD}/${IMAGE_FLD}/${IMAGE_NAME}${BURN_SUFFIX} >> ${BURN_IMG}
		python ${PWD}/tools/nand-utils/gen_burner_partition_table_def.py ${NAND_CHIP_NAME} \
			${NAND_HEADER_VERSION} ${BURN_LIST_DEF} ${PWD}/${OUTPUT_FLD}/${IMAGE_NAME}${BURN_SUFFIX} ${LBA_START} ${LBA_END} 1
		python ${PWD}/tools/nand-utils/gen_burner_partition_table_csv.py ${NAND_CHIP_NAME} \
			${NAND_HEADER_VERSION} ${BURN_LIST_CSV} ${PWD}/${OUTPUT_FLD}/${IMAGE_NAME}${BURN_SUFFIX} ${LBA_START} ${LBA_END} ${PARTITION}
	elif [ "${PARTITION}" != "" ]; then
		PART_IDX=$((${PART_IDX}+1))
		touch ${PWD}/${IMAGE_FLD}/${PARTITION}${BURN_SUFFIX}
		python ${PWD}/tools/nand-utils/pad_dummy_data_by_partition.pyc ${NAND_CHIP_NAME} \
			${PWD}/${IMAGE_FLD}/${PARTITION}${BURN_SUFFIX} ${PART_SIZE} ${NAND_HEADER_VERSION}
		cat ${PWD}/${IMAGE_FLD}/${PARTITION}${BURN_SUFFIX} >> ${BURN_IMG}
	fi
done

python ${PWD}/tools/nand-utils/gen_burner_partition_table_def.py ${NAND_CHIP_NAME} \
	${NAND_HEADER_VERSION} ${BURN_LIST_DEF} 0 0 0 2

rm -f ${PWD}/${IMAGE_FLD}/*${BURN_SUFFIX}
