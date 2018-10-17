uboot_prep_kimage() {

        linux_comp=${KERNEL_COMPRESS}

        # uncompressed elf vmlinux
        vmlinux_path="vmlinux"

        if test "${linux_comp}" = "lz4"; then
                linux_suffix=".lz4"
        elif test "${linux_comp}" = "gzip"; then
                linux_suffix=".gz"
        else
                linux_suffix=""
        fi

        ${OBJCOPY} -O binary -R .note -R .comment -S "${vmlinux_path}" linux.bin

        if test "${linux_comp}" = "lz4"; then
                lz4 -l -c1 linux.bin > linux.bin${linux_suffix}
                # append uncompressed filesize info
                dec_size=0
                fsize=$(stat -c "%s" "linux.bin")
                dec_size=$(expr $dec_size + $fsize)
                printf "%08x\n" $dec_size |
                        sed 's/\(..\)/\1 /g' | {
                                read ch0 ch1 ch2 ch3;
                                for ch in $ch3 $ch2 $ch1 $ch0; do
                                        printf `printf '%s%03o' '\\' 0x$ch` >> linux.bin${linux_suffix};
                                done;
                        }
        elif test "${linux_comp}" = "gzip"; then
                gzip -9 linux.bin
        else
                echo "For none case or another compressing"
        fi

        if ! test "${linux_comp}" = "none"; then
                mv -f "linux.bin${linux_suffix}" linux.bin
        else
                echo "No kerenl compression"
        fi

        echo "${linux_comp}"
}
