TZ_BINARY = "tz.img"
ATF_RAW_BINARY="atf_raw.img"
ATF_SIGNED_BINARY="atf_signed.img"

TZ_RAW_BINARY= "tz.raw.img"
TZ_SIGNED_BINARY = "tz_signed.img"

TZ_TMP_SIGNED_BINARY = "tz_tmp_signed.img"
TZ_TMP_RAW_BINARY = "tz_tmp_raw.img"
TZ_TMP_BINARY = "tz_tmp.img"

TRUSTEDOS_RAW_BINARY = "trusedos.img"
TRUSTEDOS_SIGNED_BINARY = "trusedos_signed.img"

TZ_ASSEMBLE_OUT = "${DEPLOY_DIR}/images/${MACHINE}/trustzone"
TL_ALIGN_SIZE = "0xb000"

do_mkdir_assemble () {
        mkdir -p ${TZ_ASSEMBLE_OUT}
}
addtask mkdir_assemble before do_compile after do_configure

python __anonymous () {
        tee_loadaddress = int(d.getVar('TRUSTZONE_LOADADDRESS', True), 16)
        tl_align_size = int(d.getVar('TL_ALIGN_SIZE', True), 16)
        tee_loadaddress = tee_loadaddress - tl_align_size
        tee_loadaddress_str = hex(tee_loadaddress).replace('L', '')
        d.setVar('TEE_LOADADDRESS', tee_loadaddress_str)

        tee_entrypoint = int(d.getVar('TRUSTZONE_ENTRYPOINT', True), 16)
        tee_entrypoint = tee_entrypoint - tl_align_size
        tee_entrypoint_str = hex(tee_entrypoint).replace('L', '')
        d.setVar('TEE_ENTRYPOINT', tee_entrypoint_str)

        image_desc = d.getVar('DESCRIPTION', True)
        if image_desc == 'TrustZone Image':
                multilibs = d.getVar('MULTILIBS', True)
                if multilibs == 'multilib:lib64':
                        d.appendVarFlag('do_cleansstate', 'depends', ' lib64-teeloader:do_cleansstate')
                        d.appendVarFlag('do_compile', 'depends', ' lib64-teeloader:do_deploy')
                else:
                        d.appendVarFlag('do_cleansstate', 'depends', ' teeloader:do_cleansstate')
                        d.appendVarFlag('do_compile', 'depends', ' teeloader:do_deploy')

                atfsupport = d.getVar('ATF_SUPPORT', True)
                if atfsupport == 'yes':
                        if multilibs == 'multilib:lib64':
                                d.appendVarFlag('do_cleansstate', 'depends', ' lib64-atf:do_cleansstate')
                                d.appendVarFlag('do_compile', 'depends', ' lib64-atf:do_deploy')
                        else:
                                d.appendVarFlag('do_cleansstate', 'depends', ' atf:do_cleansstate')
                                d.appendVarFlag('do_compile', 'depends', ' atf:do_deploy')

                tee_support = d.getVar('TEE_SUPPORT', True)
                if tee_support is None:
                        tee_support = "mtee"
                        d.appendVarFlag('do_cleansstate', 'depends', ' tee-'+tee_support+':do_cleansstate')
                        d.appendVarFlag('do_compile', 'depends', ' tee-'+tee_support+':do_deploy')
                if tee_support != 'none':
                        if multilibs == 'multilib:lib64':
                                d.appendVarFlag('do_cleansstate', 'depends', ' lib64-tee-'+tee_support+':do_cleansstate')
                                d.appendVarFlag('do_compile', 'depends', ' lib64-tee-'+tee_support+':do_deploy')
                        else:
                                d.appendVarFlag('do_cleansstate', 'depends', ' tee-'+tee_support+':do_cleansstate')
                                d.appendVarFlag('do_compile', 'depends', ' tee-'+tee_support+':do_deploy')

        secure_boot_enable = d.getVar('SECURE_BOOT_ENABLE', True)
        force_disable_tee_encryption = d.getVar('FORCE_DISABLE_TEE_ENCRYPTION', True)
        if  secure_boot_enable == 'yes' and force_disable_tee_encryption != 'yes' :
                d.setVar("ATF_BINARY_SELECT", '${ATF_SIGNED_BINARY}')
                d.setVar("TZ_BINARY_SELECT", '${TZ_SIGNED_BINARY}')
                d.setVar("TRUSTEDOS_BINARY_SELECT", '${TRUSTEDOS_SIGNED_BINARY}')
        else:
                d.setVar("ATF_BINARY_SELECT", '${ATF_RAW_BINARY}')
                d.setVar("TZ_BINARY_SELECT", '${TZ_RAW_BINARY}')
                d.setVar("TRUSTEDOS_BINARY_SELECT", '${TRUSTEDOS_RAW_BINARY}')
}
