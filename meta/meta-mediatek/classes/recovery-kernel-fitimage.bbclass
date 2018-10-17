inherit kernel-uboot-extension

python __anonymous () {
    kerneltype = d.getVar('KERNEL_IMAGETYPE', True)
    recoverykerneldevicetree = d.getVar('RECOVERY_KERNEL_DEVICETREE', True)
    if kerneltype == 'fitImage' and recoverykerneldevicetree != '' :
        depends = d.getVar("DEPENDS", True)
        depends = "%s u-boot-mkimage-native lz4-native dtc-native" % depends
        d.setVar("DEPENDS", depends)

        # Override KERNEL_IMAGETYPE_FOR_MAKE variable, which is internal
        # to kernel.bbclass . We have to override it, since we pack zImage
        # (at least for now) into the fitImage .
        kernelarch = d.getVar('KERNEL_ARCH', True)
        if kernelarch == 'arm':
                d.setVar("KERNEL_IMAGETYPE_FOR_MAKE", "zImage")
        elif kernelarch == 'arm64':
                d.setVar("KERNEL_IMAGETYPE_FOR_MAKE", "Image")
        else:
                print("please set KERNEL_ARCH variable.")
                sys.exit(1)
}

#
# Emit the fitImage ITS header
#
fit_recovery_image_emit_fit_header() {
        cat << EOF >> fit-recovery-image.its
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
fit_recovery_image_emit_section_maint() {
        case $1 in
        imagestart)
                cat << EOF >> fit-recovery-image.its

        images {
EOF
        ;;
        confstart)
                cat << EOF >> fit-recovery-image.its

        configurations {
EOF
        ;;
        sectend)
                cat << EOF >> fit-recovery-image.its
        };
EOF
        ;;
        fitend)
                cat << EOF >> fit-recovery-image.its
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
fit_recovery_image_emit_section_kernel() {

        kernel_csum="sha256"

        ENTRYPOINT=${UBOOT_ENTRYPOINT}
        if test -n "${UBOOT_ENTRYSYMBOL}"; then
                ENTRYPOINT=`${HOST_PREFIX}nm ${S}/vmlinux | \
                        awk '$3=="${UBOOT_ENTRYSYMBOL}" {print $1}'`
        fi

        cat << EOF >> fit-recovery-image.its
                kernel@${1} {
                        description = "Linux kernel";
                        data = /incbin/("${2}");
                        type = "kernel";
                        arch = "${UBOOT_ARCH}";
                        os = "linux";
                        compression = "${3}";
                        load = <${UBOOT_LOADADDRESS}>;
                        entry = <${ENTRYPOINT}>;
                        hash@1 {
                                algo = "${kernel_csum}";
                        };
                };
EOF
}

#
# Emit the fitImage ITS DTB section
#
# $1 ... Image counter
# $2 ... Path to DTB image
fit_recovery_image_emit_section_dtb() {

        dtb_csum="sha256"

        cat << EOF >> fit-recovery-image.its
                fdt@${1} {
                        description = "Flattened Device Tree blob";
                        data = /incbin/("${2}");
                        type = "flat_dt";
                        arch = "${UBOOT_ARCH}";
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
fit_recovery_image_emit_section_config() {

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

        cat << EOF >> fit-recovery-image.its
                default = "conf@1";
                conf@1 {
                        description = "${conf_desc}";
                        ${kernel_line}
                        ${fdt_line}
                        signature@1 {
                            algo = "${conf_csum}";
                            key-name-hint="${conf_key_name}";
                            sign-images="fdt","kernel";
                        };
                };
EOF
}

do_assemble_recovery_fitimage() {
        cd ${B}
        if test "x${KERNEL_IMAGETYPE}" = "xfitImage" && test -n "${RECOVERY_KERNEL_DEVICETREE}"; then
                kernelcount=1
                dtbcount=""
                rm -f fit-recovery-image.its

                fit_recovery_image_emit_fit_header

                #
                # Step 1: Prepare a kernel image section.
                #
                fit_recovery_image_emit_section_maint imagestart

                uboot_prep_kimage
                fit_recovery_image_emit_section_kernel "${kernelcount}" linux.bin "${linux_comp}"

                #
                # Step 2: Prepare a DTB image section
                #
                dtbcount=1
                for DTB in ${RECOVERY_KERNEL_DEVICETREE}; do
                        if echo ${DTB} | grep -q '/dts/'; then
                                bbwarn "${DTB} contains the full path to the the dts file, but only the dtb name should be used."
                                DTB=`basename ${DTB} | sed 's,\.dts$,.dtb,g'`
                        fi

                        # go through device tree blob dir for 32/64 bits kernel
                        DTB_PATH="arch/${ARCH}/boot/dts/mediatek/${DTB}"
                        if [ ! -e "${DTB_PATH}" ]; then
                                DTB_PATH="arch/${ARCH}/boot/dts/${DTB}"
                                if [ ! -e "${DTB_PATH}" ]; then
                                        DTB_PATH="arch/${ARCH}/boot/${DTB}"
                                fi
                        fi

                        fit_recovery_image_emit_section_dtb ${dtbcount} ${DTB_PATH}
                        dtbcount=`expr ${dtbcount} + 1`
                done

                fit_recovery_image_emit_section_maint sectend
                # Force the first Kernel and DTB in the default config
                kernelcount=1
                dtbcount=1

                #
                # Step 3: Prepare a configurations section
                #
                fit_recovery_image_emit_section_maint confstart

                fit_recovery_image_emit_section_config ${kernelcount} ${dtbcount}

                fit_recovery_image_emit_section_maint sectend

                fit_recovery_image_emit_section_maint fitend

                #
                # Step 4: Assemble the image
                #
                uboot-mkimage -f fit-recovery-image.its arch/${ARCH}/boot/fitRecoveryImage

                if [ "${SECURE_BOOT_ENABLE}" = "yes" ]; then
                        mkdir -p ./mykeys
                        cp ${MTK_KEY_DIR}/${VERIFIED_KEY}.crt ./mykeys/dev.crt
                        cp ${MTK_KEY_DIR}/${VERIFIED_KEY}.pem ./mykeys/dev.key
                        uboot-mkimage -D "-I dts -O dtb -p 1024" -k ./mykeys -f fit-recovery-image.its -r arch/${ARCH}/boot/fitRecoveryImage
                fi
        fi
}

addtask assemble_recovery_fitimage before do_install after do_compile

kernel_do_deploy_append() {
        # Update deploy directory
        if test "x${KERNEL_IMAGETYPE}" = "xfitImage" && test -n "${RECOVERY_KERNEL_DEVICETREE}"; then
                cd ${B}
                install -m 0644 fit-recovery-image.its ${DEPLOYDIR}/fit-recovery-image.its
                install -m 0644 arch/${ARCH}/boot/fitRecoveryImage ${DEPLOYDIR}/recovery.img
        fi
}
