
python __anonymous () {
        depends = d.getVar("DEPENDS", True)
        depends = "%s u-boot-mkimage-native lz4-native" % depends
        d.setVar("DEPENDS", depends)
}

#
# Emit the fitImage ITS header
#
fitimage_emit_fit_header() {
        cat << EOF >> ${WORKDIR}/fit-image.its
/dts-v1/;

/ {
        description = "U-Boot fitImage for ${DISTRO_NAME}/${PV}/${MACHINE}";
        #address-cells = <1>;
EOF
}

#
# Emit the fitImage section bits
#
# $1 ... Section bit type: imagestart - image section start
#                          confstart  - configuration section start
#                          sectend    - section end
#                          fitend     - fitimage end
#
fitimage_emit_section_maint() {
        case $1 in
        imagestart)
                cat << EOF >> ${WORKDIR}/fit-image.its

        images {
EOF
        ;;
        confstart)
                cat << EOF >> ${WORKDIR}/fit-image.its

        configurations {
EOF
        ;;
        sectend)
                cat << EOF >> ${WORKDIR}/fit-image.its
        };
EOF
        ;;
        fitend)
                cat << EOF >> ${WORKDIR}/fit-image.its
};
EOF
        ;;
        esac
}

fitimage_emit_section_trustedos() {

        trustedos_csum="sha256"

        cat << EOF >> ${WORKDIR}/fit-image.its
                trustedos@1 {
                        description = "Trusted OS Image";
                        data = /incbin/("${1}");
                        type = "tee";
                        arch = "arm";
                        os = "linux";
                        compression = "${2}";
                        load = <${TRUSTEDOS_LOADADDRESS}>;
                        entry = <${TRUSTEDOS_ENTRYPOINT}>;
                        hash@1 {
                                algo = "${trustedos_csum}";
                        };
                };
EOF
}

#
# Emit the fitImage ITS trustzone section
#
# $1 ... Path to trustzone image
# $2 ... Compression type
fitimage_emit_section_trustzone() {

        trustzone_csum="sha256"

if [ -d ${TL_SRC} ]; then
        cat << EOF >> ${WORKDIR}/fit-image.its
                tee@1 {
                        description = "Tee Image";
                        data = /incbin/("${1}");
                        type = "kernel";
                        arch = "arm";
                        os = "linux";
                        compression = "${2}";
                        load = <${TEE_LOADADDRESS}>;
                        entry = <${TEE_ENTRYPOINT}>;
                        hash@1 {
                                algo = "${trustzone_csum}";
                        };
                };
EOF
else
        cat << EOF >> ${WORKDIR}/fit-image.its
                tee@1 {
                        description = "Tee Image";
                        data = /incbin/("${1}");
                        type = "kernel";
                        arch = "arm";
                        os = "linux";
                        compression = "${2}";
                        load = <${TRUSTZONE_LOADADDRESS}>;
                        entry = <${TRUSTZONE_ENTRYPOINT}>;
                        hash@1 {
                                algo = "${trustzone_csum}";
                        };
                };
EOF
fi
}

#
# Emit the fitImage ITS configuration section
#
# $1 ... trustzone image ID
fitimage_emit_section_config() {

        conf_csum="sha256,rsa2048"
        conf_key_name="dev"

        conf_desc="${MTK_PROJECT} configuration"

        trustzone_line="kernel = \"tee@1\";"
         if [ -n "${TRUSTEDOS_ENTRYPOINT}" ] && [ -n "${TRUSTEDOS_LOADADDRESS}" ] ; then
            trustedos_line="tee = \"trustedos@1\";"
            sign_images_line="sign-images = \"kernel\", \"tee\";"
        else
            trustedos_line=""
            sign_images_line="sign-images = \"kernel\";"
        fi

        cat << EOF >> ${WORKDIR}/fit-image.its
                default = "conf@1";
                conf@1 {
                        description = "${conf_desc}";
                        ${trustzone_line}
                        ${trustedos_line}
                        signature@1 {
                                algo = "${conf_csum}";
                                key-name-hint="${conf_key_name}";
                                ${sign_images_line}
                        };
                };
EOF
}

do_assemble_fitimage() {

                rm -f ${WORKDIR}/fit-image.its

                fitimage_emit_fit_header

                #
                # Step 1: Prepare a trustzone image section.
                #
                fitimage_emit_section_maint imagestart

                #uboot_prep_kimage
                if [ -d ${TL_SRC} ]; then
                        if [ "${SECURE_BOOT_ENABLE}" = "yes" ] && [ "${FORCE_DISABLE_TEE_ENCRYPTION}" != "yes" ]; then
                                fitimage_emit_section_trustzone ${TZ_ASSEMBLE_OUT}/${TZ_TMP_SIGNED_BINARY} ${TRUSTZONE_COMPRESS}
                                if [ -n "${TRUSTEDOS_ENTRYPOINT}" ] && [ -n "${TRUSTEDOS_LOADADDRESS}" ] ; then
                                    fitimage_emit_section_trustedos ${TZ_ASSEMBLE_OUT}/${TRUSTEDOS_SIGNED_BINARY} ${TRUSTZONE_COMPRESS}
                                fi
                        else
                                fitimage_emit_section_trustzone ${TZ_ASSEMBLE_OUT}/${TZ_TMP_RAW_BINARY} ${TRUSTZONE_COMPRESS}
                                 if [ -n "${TRUSTEDOS_ENTRYPOINT}" ] && [ -n "${TRUSTEDOS_LOADADDRESS}" ] ; then
                                    fitimage_emit_section_trustedos ${TZ_ASSEMBLE_OUT}/${TRUSTEDOS_RAW_BINARY} ${TRUSTZONE_COMPRESS}
                                fi
                        fi
                else
                        fitimage_emit_section_trustzone ${TZ_ASSEMBLE_OUT}/${TZ_RAW_BINARY} ${TRUSTZONE_COMPRESS}
                fi

                fitimage_emit_section_maint sectend

                #
                # Step 2: Prepare a configurations section
                #
                fitimage_emit_section_maint confstart

                fitimage_emit_section_config

                fitimage_emit_section_maint sectend

                fitimage_emit_section_maint fitend

                #
                # Step 3: Assemble the image
                #
                uboot-mkimage -f ${WORKDIR}/fit-image.its ${TZ_ASSEMBLE_OUT}/${TZ_BINARY}

                if [ "${SECURE_BOOT_ENABLE}" = "yes" ]; then
                        mkdir -p ./mykeys
                        cp ${MTK_KEY_DIR}/${VERIFIED_KEY}.crt ./mykeys/dev.crt
                        cp ${MTK_KEY_DIR}/${VERIFIED_KEY}.pem ./mykeys/dev.key
                        uboot-mkimage -D "-I dts -O dtb -p 1024" -k ./mykeys -f ${WORKDIR}/fit-image.its -r ${TZ_ASSEMBLE_OUT}/${TZ_BINARY}
                fi
}

addtask assemble_fitimage before do_install after do_compile
