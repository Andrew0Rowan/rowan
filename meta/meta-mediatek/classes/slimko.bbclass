#./slimko -i input.ko -o output.ko -c cross-compile-prefix

SLIMKOTOOL="${TOPDIR}/../prebuilt/devtools/slimko/slimko"
slim_kernel_module() {
    install -d ${IMAGE_ROOTFS}/lib/modules
    KERNEL_MODULES=$(find ${IMAGE_ROOTFS}/lib/modules -name "*.ko")
    for i in ${KERNEL_MODULES}; do
        ${SLIMKOTOOL} -i ${i} -o ${i} -c ${TARGET_PREFIX}
    done
}

ROOTFS_POSTPROCESS_COMMAND += " slim_kernel_module;"
