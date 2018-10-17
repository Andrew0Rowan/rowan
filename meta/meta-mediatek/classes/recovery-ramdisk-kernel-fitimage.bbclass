inherit kernel-arch
inherit kernel-uboot-extension

python __anonymous () {
    kerneltype = d.getVar('KERNEL_IMAGETYPE', True)
    recoverykerneldevicetree = d.getVar('RECOVERY_KERNEL_DEVICETREE', True)
    if kerneltype == 'fitImage' and recoverykerneldevicetree != '' :
        depends = d.getVar("DEPENDS", True)
        depends = "%s u-boot-mkimage-native lz4-native dtc-native" % depends
        d.setVar("DEPENDS", depends)
}

do_image_complete[postfuncs] += "do_assemble_recovery_ramdisk_fitimage"

#
# Emit the fitImage ITS header
#
fit_recovery_ramdisk_image_emit_fit_header() {
        cat << EOF >> fit-recovery-ramdisk-image.its
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
fit_recovery_ramdisk_image_emit_section_maint() {
        case $1 in
        imagestart)
                cat << EOF >> fit-recovery-ramdisk-image.its

        images {
EOF
        ;;
        confstart)
                cat << EOF >> fit-recovery-ramdisk-image.its

        configurations {
EOF
        ;;
        sectend)
                cat << EOF >> fit-recovery-ramdisk-image.its
        };
EOF
        ;;
        fitend)
                cat << EOF >> fit-recovery-ramdisk-image.its
};
EOF
        ;;
        esac
}

#
# Emit the fitImage ITS kernel section
#
# $1 ... Image counter
# $2 ... Path to kernel image
# $3 ... Compression type
fit_recovery_ramdisk_image_emit_section_kernel() {

        kernel_csum="sha256"

        cat << EOF >> fit-recovery-ramdisk-image.its
                kernel@${1} {
                        description = "Linux kernel";
                        data = /incbin/("${2}");
                        type = "kernel";
                        arch = "${ARCH}";
                        os = "linux";
                        compression = "${3}";
                        load = <${UBOOT_LOADADDRESS}>;
                        entry = <${UBOOT_ENTRYPOINT}>;
                        hash@1 {
                                algo = "${kernel_csum}";
                        };
                };
EOF
}

#
# Emit the fitImage ITS recovery ramdisk section
#
# $1 ... Image counter
# $2 ... Path to ramdifk image
fitimage_emit_section_recovery_ramdisk() {

        ramdisk_csum="sha256"

        cat << EOF >> fit-recovery-ramdisk-image.its
                ramdisk@${1} {
                        description = "Ramdisk Image";
                        data = /incbin/("${2}");
                        type = "ramdisk";
                        arch = "${ARCH}";
                        os = "linux";
                        compression = "none";
                        load = <${RECOVERY_RAMDISK_LOADADDRESS}>;
                        entry = <${RECOVERY_RAMDISK_LOADADDRESS}>;
                        hash@1 {
                                algo = "${ramdisk_csum}";
                        };
                };
EOF
}

#
# Emit the fitImage ITS DTB section
#
# $1 ... Image counter
# $2 ... Path to DTB image
fit_recovery_ramdisk_image_emit_section_dtb() {

        dtb_csum="sha256"

        cat << EOF >> fit-recovery-ramdisk-image.its
                fdt@${1} {
                        description = "Flattened Device Tree blob";
                        data = /incbin/("${2}");
                        type = "flat_dt";
                        arch = "${ARCH}";
                        compression = "none";
                        load = <${DTB_LOADADDRESS}>;
                        hash@1 {
                                algo = "${dtb_csum}";
                        };
                };
EOF
}

#
# Emit the fitImage ITS configuration section
#
# $1 ... Linux kernel ID
# $2 ... DTB image ID
# $3 ... Ramdisk ID
fit_recovery_ramdisk_image_emit_section_config() {

        conf_csum="sha256,rsa2048"
        conf_key_name="dev"

        # Test if we have any DTBs at all
        if [ -z "${2}" ] ; then
                conf_desc="Boot Linux kernel"
                fdt_line=""
        else
                conf_desc="Boot Linux kernel with FDT blob"
                fdt_line="fdt = \"fdt@${2}\";"
        fi
        kernel_line="kernel = \"kernel@${1}\";"

        # Test if we have ramdisk image
        if [ -z "${3}" ] ; then
                ramdisk_line=""
        else
                ramdisk_line="ramdisk = \"ramdisk@${3}\";"
        fi

        cat << EOF >> fit-recovery-ramdisk-image.its
                default = "conf@1";
                conf@1 {
                        description = "${conf_desc}";
                        ${kernel_line}
                        ${ramdisk_line}
                        ${fdt_line}
                        signature@1 {
                            algo = "${conf_csum}";
                            key-name-hint="${conf_key_name}";
                            sign-images="fdt","kernel";
                        };
                };
EOF
}

do_assemble_recovery_ramdisk_fitimage() {
        cd ${B}
        if test "x${KERNEL_IMAGETYPE}" = "xfitImage" && test -n "${RECOVERY_KERNEL_DEVICETREE}"; then
                kernelcount=1
                dtbcount=1
                ramdiskcount=1
                rm -f fit-recovery-ramdisk-image.its

                fit_recovery_ramdisk_image_emit_fit_header

                #
                # Step 1: Prepare a kernel image section.
                #
                fit_recovery_ramdisk_image_emit_section_maint imagestart

                fit_recovery_ramdisk_image_emit_section_kernel ${kernelcount} ${DEPLOY_DIR_IMAGE}/${KERNEL_IMAGETYPE}-linux.bin-${MACHINE}.bin ${KERNEL_COMPRESS}

                # Step 1.5: Prepare a ramdisk image section.
                #
                fitimage_emit_section_recovery_ramdisk ${ramdiskcount} ${IMGDEPLOYDIR}/${IMAGE_BASENAME}-${MACHINE}.${IMAGE_FSTYPES}

                #
                # Step 2: Prepare a DTB image section
                #
                if test -n "${RECOVERY_KERNEL_DEVICETREE}"; then
                        fit_recovery_ramdisk_image_emit_section_dtb ${dtbcount} ${DEPLOY_DIR_IMAGE}/${RECOVERY_KERNEL_DEVICETREE}
                fi

                fit_recovery_ramdisk_image_emit_section_maint sectend

                #
                # Step 3: Prepare a configurations section
                #
                fit_recovery_ramdisk_image_emit_section_maint confstart

                fit_recovery_ramdisk_image_emit_section_config ${kernelcount} ${dtbcount} ${ramdiskcount}

                fit_recovery_ramdisk_image_emit_section_maint sectend

                fit_recovery_ramdisk_image_emit_section_maint fitend

                #
                # Step 4: Assemble the image
                #
                uboot-mkimage -f fit-recovery-ramdisk-image.its recovery.ramdisk.img

                if [ "${SECURE_BOOT_ENABLE}" = "yes" ]; then
                        mkdir -p ./mykeys
                        cp ${MTK_KEY_DIR}/${VERIFIED_KEY}.crt ./mykeys/dev.crt
                        cp ${MTK_KEY_DIR}/${VERIFIED_KEY}.pem ./mykeys/dev.key
                        uboot-mkimage -D "-I dts -O dtb -p 1024" -k ./mykeys -f fit-recovery-ramdisk-image.its -r recovery.ramdisk.img
                fi

                #
                # Step 5: Install the recovery.ramdisk.img and recovery fit to DEPLOY_DIR_IMAGE folder
                #
                install -d ${DEPLOY_DIR_IMAGE}
                install -m 0644 fit-recovery-ramdisk-image.its recovery.ramdisk.img -t ${DEPLOY_DIR_IMAGE}

        fi
}
