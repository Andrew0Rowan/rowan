
python __anonymous () {
        depends = d.getVar("DEPENDS", True)
        depends = "%s u-boot-mkimage-native dtc-native" % depends
        d.setVar("DEPENDS", depends)
}

# Emit fit header
# input param:
#        $1: fit its
fitimage_emit_fit_header() {
        cat << EOF >> $1
/dts-v1/;

/ {
        description = "bl";
        #address-cells = <1>;
EOF
}

# Emit the fitimage image section
# input param:
#        $1: fit its
#        $2: path to padding binary
#        $3: path to lk binary
#        $4: lk load address
#        $5: lk entry address
#        $6: hash algorithm
fitimage_emit_fit_image_section() {
        cat << EOF >> $1

        images {
                bl@1 {
                        description = "bl";
                        padding = /incbin/("${2}");
                        data = /incbin/("${3}");
                        type = "kernel";
                        compression = "none";
                        load = <${4}>;
                        entry = <${5}>;
                        hash@1 {
                                algo = "${6}";
                        };
                };
        };
EOF
}

# Emit fitimage configuration section
# input param:
#        $1: fit its
#        $2: hash algorithm
#        $3: rsa algorithm
#        $4: key name hint
fitimage_emit_fit_conf_section() {
        cat << EOF >> $1

        configurations {
                default = "conf@1";
                conf@1 {
                        description = "bl";
                        kernel = "bl@1";
                        signature@1 {
                                algo = "${2},${3}";
                                key-name-hint = "${4}";
                        };
                };
        };
EOF
}

# Emit fitimage fit end
# input param:
#        $1: fit its
fitimage_emit_fit_end() {
        cat << EOF >> $1
};
EOF
}

# get_fit_data_offset: get data offset from fit image
# input param:
#        $1: fit image
get_fit_data_offset() {
        FIT_IMAGE=$1

        TMPF=$(mktemp ${WORKDIR}/dump-fit-img.XXXXXX)

        # dump fit image
        fdtdump -d ${FIT_IMAGE} > ${TMPF}

        # get props
        LINE=$(grep -n '^ *data' ${TMPF} | grep -o '^[0-9]*')
        LINE=`expr ${LINE} - 1`
        ADDR=$(grep -n "^// [[:xdigit:]]*: value" ${TMPF} | grep "^${LINE}://" | \
             grep -o " [[:xdigit:]]*" | grep -o "[[:xdigit:]]*")

        # remove temp files
        rm -f ${TMPF}

        echo ${ADDR}
}

# check_fit_image: check fit image and also check load/entry address if 2nd
#                  param was present
# input param:
#        $1: fit image
#        $2: if present, will also check load and entry validity
check_fit_image() {
        FIT_IMAGE=$1
        CHECK_OPT=$2

        # check default cfg
        CFG=$(fdtget ${FIT_IMAGE} /configurations default)
        if [ "${CFG}" = "" ]; then
                echo "ERROR: no default in /configurations"
                exit 1
        fi

        # check image name
        IMG_NAME=$(fdtget ${FIT_IMAGE} /configurations/${CFG} kernel)
        if [ "${IMG_NAME}" = "" ]; then
                echo "ERROR: no image name (kernel prop) in /configurations/${CFG}"
                exit 1
        fi

        # check data
        DATA=$(fdtget ${FIT_IMAGE} /images/${IMG_NAME} data)
        if [ "${DATA}" = "" ]; then
                echo "ERROR: no data in /images/${IMG_NAME}"
                exit 1
        fi

        # check load
        LOAD_D=$(fdtget ${FIT_IMAGE} /images/${IMG_NAME} load)
        if [ "$LOAD_D" = "" ]; then
                echo "ERROR: no load in /images/${IMG_NAME}"
                exit 1
        fi

        # check entry
        ENTRY_D=$(fdtget ${FIT_IMAGE} /images/${IMG_NAME} entry)
        if [ "$ENTRY_D" = "" ]; then
                echo "ERROR: no entry in /images/${IMG_NAME}"
                exit 1
        fi

        if [ "${CHECK_OPT}" != "" ]; then
                OFFSET=$(get_fit_data_offset ${FIT_IMAGE})
                LOAD=$(printf "%x" ${LOAD_D})
                if [ "`expr ${LOAD} : '.*\(....\)'`" != "${OFFSET}" ]; then
                        echo ERROR: load ${LOAD} is not align with data offset ${OFFSET}
                        exit 1
                fi

                LEN=$(echo ${DATA} | wc -w)
                ENTRY=$(printf "%x" ${ENTRY_D})
                END_D=`echo "${LOAD_D} + ${LEN} * 4" | bc`
                END=$(printf "%x" ${END_D})
                if [ ${ENTRY_D} -lt ${LOAD_D} -o ${END_D} -le ${ENTRY_D} ]; then
                        echo ERROR: entry ${ENTRY} is not in data ${LOAD} - ${END}
                        exit 1
                fi
        fi
}

# generate fit image: generate lk image in fit format
# input param:
#        $1: fit image its file
#        $2: key dir
#        $3: output file
gen_fit_image() {
        FIT_ITS=$1
        KEYDIR=$2
        OUT=$3
        DTB=$(mktemp ${WORKDIR}/dtb.XXXXXX)
        KEYDTSI=$(mktemp ${WORKDIR}/key.XXXXXX.dtsi)
        KEYITS=$(mktemp ${WORKDIR}/key-fit-image.XXXXXX.its)

        echo "/dts-v1/; / {};" | dtc -O dtb > ${DTB}
        uboot-mkimage -f ${FIT_ITS} -k ${KEYDIR} -K ${DTB} ${OUT}
        dtc -I dtb ${DTB} | tail -n+2 > ${KEYDTSI}

        sed "s,\/dts-v1\/;,\0\n\/include\/ \"${KEYDTSI}\"," < ${FIT_ITS} > ${KEYITS}
        uboot-mkimage -f ${KEYITS} -k ${KEYDIR} ${OUT}

        # remove temporary files
        rm -f ${DTB} ${KEYDTSI} ${KEYITS}
}

# gen_sbc_key_hash
# input param:
#        $1: fit image
gen_sbc_key_hash() {
        FDT_IMG=$1
        TMP_FILE=$(mktemp ${WORKDIR}/col_rgn.XXXXXX.bin)

        DEF_CONF=`fdtget -t s ${FDT_IMG} /configurations default`
        SIG_NODE=`fdtget -l ${FDT_IMG} /configurations/${DEF_CONF}`
        KEY_HINT=`fdtget -t s ${FDT_IMG} /configurations/${DEF_CONF}/${SIG_NODE} key-name-hint`
        SHAX=`fdtget -t s ${FDT_IMG} /configurations/${DEF_CONF}/${SIG_NODE} algo | cut -d, -f1`
        ${WORKDIR}/fit-lk/extract_region ${FDT_IMG} ${TMP_FILE} /signature/key-${KEY_HINT}
        KEY_HASH=`${SHAX}sum ${TMP_FILE} | cut -d " " -f 1`
        echo "Hash algo: ${SHAX}"
        echo "key_hash: ${KEY_HASH}"

        for i in $(seq 1 8 ${#KEY_HASH}); do
                end_i=`echo "$i+7" | bc`
                qb=`echo ${KEY_HASH} | cut -c${i}-${end_i} | \
                        sed 's,\(..\)\(..\)\(..\)\(..\),\4\3\2\1,g'`
                idx=`echo "${i} / 8" | bc`
                echo "keyhash_${idx}: 0x"$qb
        done

        rm -f ${TMP_FILE}
}

gen_lk_fit_header() {
        # generate temp files
        BL_BIN=$(mktemp ${WORKDIR}/bl.XXXXXX.bin)
        FIT_LK_IMG=${WORKDIR}/fit-lk.img
        FIT_ITS=$(mktemp ${WORKDIR}/fit-bl-image.XXXXXX.its)
        KEYDIR=${WORKDIR}/lk-key
        PADDING=$(mktemp ${WORKDIR}/padding.XXXXXX.bin)

        # copy raw lk binary to working directory
        cp ${LK_OUT}/build-${LK_PROJECT}/${LK_BINARY} ${BL_BIN}

        # generate a zero-size padding binary
        touch ${PADDING}

        # prepare rsa key
        mkdir -p ${KEYDIR}

        case "${SBC_RSA_ALGO}" in
                rsa2048 | rsassa-pss2048)
                        cp -f ${MTK_KEY_DIR}/${SBC_KEY}.crt ${KEYDIR}/dev.crt
                        cp -f ${MTK_KEY_DIR}/${SBC_KEY}.pem ${KEYDIR}/dev.key
                        ;;
                rsa3072 | rsassa-pss3072)
                        cp -f ${MTK_KEY_DIR}/${SBC_KEY_RSA3072}.crt ${KEYDIR}/dev.crt
                        cp -f ${MTK_KEY_DIR}/${SBC_KEY_RSA3072}.pem ${KEYDIR}/dev.key
                        ;;
                rsa4096 | rsassa-pss4096)
                        cp -f ${MTK_KEY_DIR}/${SBC_KEY_RSA4096}.crt ${KEYDIR}/dev.crt
                        cp -f ${MTK_KEY_DIR}/${SBC_KEY_RSA4096}.pem ${KEYDIR}/dev.key
                        ;;
                *)
                        echo ${SBC_RSA_ALGO} ": not supported."
                        exit 1
                        ;;
        esac

        # generate base its file
        fitimage_emit_fit_header ${FIT_ITS}
        fitimage_emit_fit_image_section ${FIT_ITS} ${PADDING} ${BL_BIN} \
                ${LK_LOADADDRESS} ${LK_ENTRYPOINT} ${IMAGE_HASH_ALGO}
        fitimage_emit_fit_conf_section ${FIT_ITS} ${SBC_HASH_ALGO} ${SBC_RSA_ALGO} dev
        fitimage_emit_fit_end ${FIT_ITS}

        # 1st pass: generate fit-lk image to get padding size
        gen_fit_image ${FIT_ITS} ${KEYDIR} ${FIT_LK_IMG}
        check_fit_image ${FIT_LK_IMG}

        # get data offset and calculate padding size
        # padding size = lk load offset - data offset
        DATA_OFFSET=$(get_fit_data_offset ${FIT_LK_IMG})
        DATA_OFFSET_D=`echo "ibase=16;obase=A;${DATA_OFFSET}" | bc`
        LK_LOAD_OFFSET_D=$(printf "%d" ${LK_LOAD_OFFSET})
        PADDING_SIZE=`echo "${LK_LOAD_OFFSET_D} - ${DATA_OFFSET_D}" | bc`

        rm -f ${PADDING}
        dd if=/dev/zero of=${PADDING} bs=1 count=${PADDING_SIZE}

        # 2nd pass: generate final fit-lk image
        gen_fit_image ${FIT_ITS} ${KEYDIR} ${FIT_LK_IMG}
        check_fit_image ${FIT_LK_IMG} all

        cp ${FIT_LK_IMG} ${WORKDIR}/${LK_IMAGE}

        # gen key hash for convenience
        gen_sbc_key_hash ${FIT_LK_IMG}

        # remove temp files
        rm -f ${BL_BIN} ${FIT_ITS} ${PADDING}
        rm -rf ${KEYDIR}
}

gen_lk_gfh_header() {
        cp ${LK_OUT}/build-${LK_PROJECT}/${LK_BINARY} ${WORKDIR}/temp_gfh

        if [ "${SECURE_BOOT_ENABLE}" = "yes" ]; then
                cp ${MTK_KEY_DIR}/${SBC_KEY}.pem ${KEY_DIR}/root_prvk.pem
        fi

        python ${PBP_DIR}/pbp.py -g ${GFH_DIR}/${TARGET_PLATFORM}/gfh_conf.ini \
               -i ${KEY_DIR}/lk_key.ini -func sign \
               -o ${WORKDIR}/temp_gfh ${WORKDIR}/temp_gfh

        if [ "${BOOTDEV_TYPE}" != "nand" ]; then
                python ${DEV_INFO_HDR_TOOL} \
                       ${BOOTDEV_TYPE} ${WORKDIR}/temp_gfh ${WORKDIR}/${LK_IMAGE}
        else
                cp ${WORKDIR}/temp_gfh ${WORKDIR}/${LK_IMAGE}
        fi
}
