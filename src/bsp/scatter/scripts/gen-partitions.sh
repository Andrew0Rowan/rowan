#! /bin/sh

PROJECT=$1
OUT=$2
BOOTDEV_TYPE=$3
NAND_CHIP_NAME=$4
NAND_HEADER_VERSION=$5

if [ ! -e ${PROJECT}/gen-partitions.ini ]; then
	echo ERROR: can not find $1
	exit 1
fi

mkdir -p ${OUT}
cat ${PROJECT}/gen-partitions.ini | while read p s m t
do
	if [ "$t" = "" ]; then
		python scripts/gen-partition.py ${PROJECT}/$p ${PROJECT}/$s ${OUT}/$m
	else
		python scripts/gen-partition.py ${PROJECT}/$p ${PROJECT}/$s ${OUT}/$m ${OUT}/$t
	fi

	if [ "${BOOTDEV_TYPE}" = "nand" ]; then
		if [ "${NAND_HEADER_VERSION}" = "2.0" ]; then
			if [ "$m" = "MBR_NAND" ]; then
				python -B ${PWD}/tools/nand-utils/gen_nand_header.py ${NAND_CHIP_NAME} ${OUT}/$m
			fi
		elif [ "${NAND_HEADER_VERSION}" = "1.1" ]; then
			lk_start=$(grep 'name="UBOOT"' ${PROJECT}/$p | grep -o 'start="[0-9]*"' | grep -o [0-9]*)
			if [ "${lk_start}" = "" ]; then
				echo ERROR: can not find UBOOT in ${PROJECT}/$p
				exit 1
			fi
			temp_header_file="temp_nand_header.bin"
			python -B ${PWD}/tools/nand-utils/gen_nand_header_v11.py ${NAND_CHIP_NAME} ${lk_start} ${OUT}/${temp_header_file}
			cat ${OUT}/$m >> ${OUT}/${temp_header_file}
			rm ${OUT}/$m
			mv ${OUT}/${temp_header_file} ${OUT}/$m
		else
			if [ "${NAND_CHIP_NAME}" = "" ]; then
				echo ERROR: please check NAND_CHIP_NAME ${NAND_CHIP_NAME}
				exit 2
			fi

			PAGE_SIZE=$(grep -o '^'${NAND_CHIP_NAME}'	[0-9]*' nand-setting.txt | grep -o '[0-9]*$')
			LBS=$(grep -o 'lbs="[0-9]*"' ${PROJECT}/$p | grep -o [0-9]*)
			if [ "${PAGE_SIZE}" != "${LBS}" ]; then
				echo ERROR: PAGE_SIZE ${PAGE_SIZE} of NAND ${NAND_CHIP_NAME} != LBS ${LBS} of partition table ${PROJECT}/$p
				exit 3
			fi

			LK1=$(grep 'name="LK1"' ${PROJECT}/$p | grep -o 'start="[0-9]*"' | grep -o [0-9]*)
			if [ "${LK1}" = "" ]; then
				echo ERROR: can not find LK1 in ${PROJECT}/$p
				exit 4
			fi

			LK2=$(grep 'name="LK2"' ${PROJECT}/$p | grep -o 'start="[0-9]*"' | grep -o [0-9]*)
			if [ "${LK2}" = "" ]; then
				echo ERROR: can not find LK2 in ${PROJECT}/$p
				exit 5
			fi

			python scripts/dev-info-hdr-tool.py ${BOOTDEV_TYPE} ${OUT}/$m ${OUT}/$m ${NAND_CHIP_NAME} ${LK1} ${LK2}
		fi
	fi
done

