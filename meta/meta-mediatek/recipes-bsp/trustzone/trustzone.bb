inherit deploy tzsrcprebuilt trustzone-build trustzone-fitimage

DESCRIPTION = "TrustZone Image"
LICENSE = "MediaTekProprietary"
TZ_SRC = "${TOPDIR}/../src/bsp/trustzone"
TL_SRC = "${TOPDIR}/../src/bsp/trustzone/teeloader"
TZ_PREBUILT = "${TOPDIR}/../prebuilt/bsp/trustzone"
LIC_FILES_CHKSUM = "file://${TZ_SRC}/mtee/build/yocto_build/README;md5=5a126b0ba82af703f1c30cf8d0bb4e13"
PACKAGE_ARCH = "${MACHINE_ARCH}"
DEPENDS += "u-boot-mkimage-native dtc-native lz4-native"

TZ_OUT = "${WORKDIR}/out"

#please make sure all output file under ${TZ_ASSEMBLE_OUT}
#TEELOADER: teeloader.bin
#ATF: ${ATF_RAW_BINARY} ${ATF_SIGNED_BINARY}
#TEE: ${TZ_RAW_BINARY}  ${TZ_SIGNED_BINARY}
#final layout:
#raw tz.img		(not has extra tee verification):
#[teeloader.bin][${ATF_RAW_BINARY}][[${TZ_RAW_BINARY}](if has tee)]
#signed tz.img	(has extra tee verification handled in tee loader):
#[teeloader.bin][${ATF_SIGNED_BINARY}][[${TZ_SIGNED_BINARY}](if has tee)]
#
#trustzone.bb only assemble the final tz.img with the files under ${TZ_ASSEMBLE_OUT}
#

run_lz4_compression() {
	dec_size=0
	fsize=$(stat -c "%s" "${TZ_ASSEMBLE_OUT}/${1}")
	dec_size=$(expr $dec_size + $fsize)
	lz4 -l -c1 ${TZ_ASSEMBLE_OUT}/${1} > ${TZ_ASSEMBLE_OUT}/${1}.lz4
	mv -f ${TZ_ASSEMBLE_OUT}/${1}.lz4 ${TZ_ASSEMBLE_OUT}/${1}
	printf "%08x\n" $dec_size |
		sed 's/\(..\)/\1 /g' | {
		    read ch0 ch1 ch2 ch3;
		    for ch in $ch3 $ch2 $ch1 $ch0; do
			printf `printf '%s%03o' '\\' 0x$ch` >> ${TZ_ASSEMBLE_OUT}/${1};
		    done;
		}
}

do_compile () {
if ${@bb.utils.contains('TRUSTZONE_HEADER','fit','true','false',d)}; then
	# integrate tz image
	if [ -d ${TL_SRC} ]; then
		cp ${TZ_ASSEMBLE_OUT}/teeloader.bin ${TZ_ASSEMBLE_OUT}/${TZ_TMP_BINARY}
	fi
	if [ "${ATF_SUPPORT}" = "yes" ]; then
		cat ${TZ_ASSEMBLE_OUT}/${ATF_BINARY_SELECT} >> ${TZ_ASSEMBLE_OUT}/${TZ_TMP_BINARY}
	fi
	if [ "${TEE_SUPPORT}" != "none" ]; then
		if [ -z "${TRUSTEDOS_ENTRYPOINT}" ] && [ -z "${TRUSTEDOS_LOADADDRESS}" ] ; then
			cat ${TZ_ASSEMBLE_OUT}/${TZ_BINARY_SELECT} >>  ${TZ_ASSEMBLE_OUT}/${TZ_TMP_BINARY}
		fi
	fi

	# compression method LZ4 support
	if [ "${TRUSTZONE_COMPRESS}" = "lz4" ]; then
	    run_lz4_compression ${TZ_TMP_BINARY}

		if [ "${TEE_SUPPORT}" != "none" ]; then
			if [ -n "${TRUSTEDOS_ENTRYPOINT}" ] && [ -n "${TRUSTEDOS_LOADADDRESS}" ]; then
				run_lz4_compression ${TRUSTEDOS_BINARY_SELECT}
			fi
		fi
	fi

	# raw and signed tz image flow
	if [ "${SECURE_BOOT_ENABLE}" = "yes" ] && [ "${FORCE_DISABLE_TEE_ENCRYPTION}" != "yes" ]; then
				mv ${TZ_ASSEMBLE_OUT}/${TZ_TMP_BINARY} ${TZ_ASSEMBLE_OUT}/${TZ_TMP_SIGNED_BINARY}
	else
				mv ${TZ_ASSEMBLE_OUT}/${TZ_TMP_BINARY} ${TZ_ASSEMBLE_OUT}/${TZ_TMP_RAW_BINARY}
	fi
fi
}

do_deploy () {
	install -d ${DEPLOYDIR}
	install ${TZ_ASSEMBLE_OUT}/${TZ_BINARY} ${DEPLOYDIR}/${TZ_BINARY}
	if [ -e ${TZ_ASSEMBLE_OUT}/${TZ_RAW_BINARY} ]; then
		install ${TZ_ASSEMBLE_OUT}/${TZ_RAW_BINARY} ${DEPLOYDIR}/${TZ_RAW_BINARY}
	fi
	if [ -e ${TZ_ASSEMBLE_OUT}/${TZ_SIGNED_BINARY} ]; then
		install ${TZ_ASSEMBLE_OUT}/${TZ_SIGNED_BINARY} ${DEPLOYDIR}/${TZ_SIGNED_BINARY}
	fi
}

addtask deploy before do_build after do_install
