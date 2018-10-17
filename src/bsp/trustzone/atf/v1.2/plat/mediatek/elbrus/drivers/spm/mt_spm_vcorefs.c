#include <arch_helpers.h>
#include <debug.h>
#include <mmio.h>
#include <mt_spm.h>
#include <mt_spm_internal.h>
#include <mt_spm_reg.h>
#include <mt_spm_vcorefs.h>
#include <plat_pm.h>
#include <platform.h>
#include <platform_def.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

static struct pwr_ctrl vcorefs_ctrl = {
	.wake_src		= WAKE_SRC_R12_PCM_TIMER,

	/* default VCORE DVFS is disabled */
	.pcm_flags		= (SPM_FLAG_RUN_COMMON_SCENARIO | SPM_FLAG_DIS_VCORE_DVS | SPM_FLAG_DIS_VCORE_DFS),

	/* Auto-gen Start */

	/* SPM_CLK_CON */
	.spm_clk_con.bits.reg_srcclken0_ctl = 0,
	.spm_clk_con.bits.reg_srcclken1_ctl = 0x3,
	.spm_clk_con.bits.reg_spm_lock_infra_dcm = 1,
	.spm_clk_con.bits.reg_srcclken_mask = 1,
	.spm_clk_con.bits.reg_md1_c32rm_en = 0,
	.spm_clk_con.bits.reg_md2_c32rm_en = 0,
	.spm_clk_con.bits.reg_clksq0_sel_ctrl = 0,
	.spm_clk_con.bits.reg_clksq1_sel_ctrl = 1,
	.spm_clk_con.bits.reg_srcclken0_en = 1,
	.spm_clk_con.bits.reg_srcclken1_en = 0,
	.spm_clk_con.bits.reg_sysclk0_src_mask_b = 0,
	.spm_clk_con.bits.reg_sysclk1_src_mask_b = 0x20,

	/* SPM_AP_STANDBY_CON */
	.spm_ap_standby_con.bits.reg_mpwfi_op = WFI_OP_AND,
	.spm_ap_standby_con.bits.reg_mp0_cputop_idle_mask = 1,
	.spm_ap_standby_con.bits.reg_mp1_cputop_idle_mask = 1,
	.spm_ap_standby_con.bits.reg_debugtop_idle_mask = 1,
	.spm_ap_standby_con.bits.reg_mp_top_idle_mask = 1,
	.spm_ap_standby_con.bits.reg_mcusys_idle_mask = 1,
	.spm_ap_standby_con.bits.reg_md_ddr_en_0_dbc_en = 0,
	.spm_ap_standby_con.bits.reg_md_ddr_en_1_dbc_en = 0,
	.spm_ap_standby_con.bits.reg_conn_ddr_en_dbc_en = 0,
	.spm_ap_standby_con.bits.reg_md32_mask_b = 0,
	.spm_ap_standby_con.bits.reg_md_0_mask_b = 0,
	.spm_ap_standby_con.bits.reg_md_1_mask_b = 0,
	.spm_ap_standby_con.bits.reg_scp_mask_b = 0,
	.spm_ap_standby_con.bits.reg_srcclkeni0_mask_b = 0,
	.spm_ap_standby_con.bits.reg_srcclkeni1_mask_b = 0,
	.spm_ap_standby_con.bits.reg_md_apsrc_1_sel = 0,
	.spm_ap_standby_con.bits.reg_md_apsrc_0_sel = 0,
	.spm_ap_standby_con.bits.reg_conn_mask_b = 0,
	.spm_ap_standby_con.bits.reg_conn_apsrc_sel = 0,

	/* SPM_SRC_REQ */
	.spm_src_req.bits.reg_spm_apsrc_req = 0,
	.spm_src_req.bits.reg_spm_f26m_req = 0,
	.spm_src_req.bits.reg_spm_infra_req = 0,
	.spm_src_req.bits.reg_spm_ddren_req = 0,
	.spm_src_req.bits.reg_spm_vrf18_req = 0,
	.spm_src_req.bits.reg_spm_dvfs_level0_req = 0,
	.spm_src_req.bits.reg_spm_dvfs_level1_req = 0,
	.spm_src_req.bits.reg_spm_dvfs_level2_req = 0,
	.spm_src_req.bits.reg_spm_dvfs_level3_req = 0,
	.spm_src_req.bits.reg_spm_dvfs_level4_req = 0,
	.spm_src_req.bits.reg_spm_pmcu_mailbox_req = 0,
	.spm_src_req.bits.reg_spm_sw_mailbox_req = 0,
	.spm_src_req.bits.reg_spm_cksel2_req = 0,
	.spm_src_req.bits.reg_spm_cksel3_req = 0,

	/* SPM_SRC_MASK */
	.spm_src_mask.bits.reg_csyspwreq_mask = 0,
	.spm_src_mask.bits.reg_md_srcclkena_0_infra_mask_b = 0,
	.spm_src_mask.bits.reg_md_srcclkena_1_infra_mask_b = 0,
	.spm_src_mask.bits.reg_md_apsrc_req_0_infra_mask_b = 0,
	.spm_src_mask.bits.reg_md_apsrc_req_1_infra_mask_b = 0,
	.spm_src_mask.bits.reg_conn_srcclkena_infra_mask_b = 0,
	.spm_src_mask.bits.reg_conn_infra_req_mask_b = 0,
	.spm_src_mask.bits.reg_md32_srcclkena_infra_mask_b = 0,
	.spm_src_mask.bits.reg_md32_infra_req_mask_b = 0,
	.spm_src_mask.bits.reg_scp_srcclkena_infra_mask_b = 0,
	.spm_src_mask.bits.reg_scp_infra_req_mask_b = 0,
	.spm_src_mask.bits.reg_srcclkeni0_infra_mask_b = 0,
	.spm_src_mask.bits.reg_srcclkeni1_infra_mask_b = 0,
	.spm_src_mask.bits.reg_ccif0_md_event_mask_b = 0,
	.spm_src_mask.bits.reg_ccif0_ap_event_mask_b = 0,
	.spm_src_mask.bits.reg_ccif1_md_event_mask_b = 0,
	.spm_src_mask.bits.reg_ccif1_ap_event_mask_b = 0,
	.spm_src_mask.bits.reg_ccif2_md_event_mask_b = 0,
	.spm_src_mask.bits.reg_ccif2_ap_event_mask_b = 0,
	.spm_src_mask.bits.reg_ccif3_md_event_mask_b = 0,
	.spm_src_mask.bits.reg_ccif3_ap_event_mask_b = 0,
	.spm_src_mask.bits.reg_ccifmd_md1_event_mask_b = 0,
	.spm_src_mask.bits.reg_ccifmd_md2_event_mask_b = 0,
	.spm_src_mask.bits.reg_c2k_ps_rccif_wake_mask_b = 0,
	.spm_src_mask.bits.reg_c2k_l1_rccif_wake_mask_b = 0,
	.spm_src_mask.bits.reg_ps_c2k_rccif_wake_mask_b = 0,
	.spm_src_mask.bits.reg_l1_c2k_rccif_wake_mask_b = 0,
	.spm_src_mask.bits.reg_dqssoc_req_mask_b = 0,
	.spm_src_mask.bits.reg_disp2_req_mask_b = 0,
	.spm_src_mask.bits.reg_md_ddr_en_0_mask_b = 0,
	.spm_src_mask.bits.reg_md_ddr_en_1_mask_b = 0,
	.spm_src_mask.bits.reg_conn_ddr_en_mask_b = 0,

	/* SPM_SRC2_MASK */
	.spm_src2_mask.bits.reg_disp0_req_mask_b = 0,
	.spm_src2_mask.bits.reg_disp1_req_mask_b = 0,
	.spm_src2_mask.bits.reg_disp_od_req_mask_b = 0,
	.spm_src2_mask.bits.reg_mfg_req_mask_b = 0,
	.spm_src2_mask.bits.reg_vdec0_req_mask_b = 0,
	.spm_src2_mask.bits.reg_gce_vrf18_req_mask_b = 0,
	.spm_src2_mask.bits.reg_gce_req_mask_b = 0,
	.spm_src2_mask.bits.reg_lpdma_req_mask_b = 0,
	.spm_src2_mask.bits.reg_srcclkeni1_cksel2_mask_b = 0,
	.spm_src2_mask.bits.reg_conn_srcclkena_cksel2_mask_b = 0,
	.spm_src2_mask.bits.reg_srcclkeni0_cksel3_mask_b = 0,
	.spm_src2_mask.bits.reg_md32_apsrc_req_ddren_mask_b = 0,
	.spm_src2_mask.bits.reg_scp_apsrc_req_ddren_mask_b = 0,
	.spm_src2_mask.bits.reg_md_vrf18_req_0_mask_b = 0,
	.spm_src2_mask.bits.reg_md_vrf18_req_1_mask_b = 0,
	.spm_src2_mask.bits.reg_next_dvfs_level0_mask_b = 1,
	.spm_src2_mask.bits.reg_next_dvfs_level1_mask_b = 1,
	.spm_src2_mask.bits.reg_next_dvfs_level2_mask_b = 1,
	.spm_src2_mask.bits.reg_next_dvfs_level3_mask_b = 1,
	.spm_src2_mask.bits.reg_next_dvfs_level4_mask_b = 1,
	.spm_src2_mask.bits.reg_msdc1_dvfs_halt_mask = 0,
	.spm_src2_mask.bits.reg_msdc2_dvfs_halt_mask = 0,
	.spm_src2_mask.bits.reg_msdc3_dvfs_halt_mask = 0,
	.spm_src2_mask.bits.reg_sw2spm_int0_mask_b = 0,
	.spm_src2_mask.bits.reg_sw2spm_int1_mask_b = 0,
	.spm_src2_mask.bits.reg_sw2spm_int2_mask_b = 0,
	.spm_src2_mask.bits.reg_sw2spm_int3_mask_b = 0,
	.spm_src2_mask.bits.reg_pmcu2spm_int0_mask_b = 0,
	.spm_src2_mask.bits.reg_pmcu2spm_int1_mask_b = 0,
	.spm_src2_mask.bits.reg_pmcu2spm_int2_mask_b = 0,
	.spm_src2_mask.bits.reg_pmcu2spm_int3_mask_b = 0,

	/* SPM_WAKEUP_EVENT_MASK */
	.reg_wakeup_event_mask = 0xFFFFFFFF,

	/* SPM_EXT_WAKEUP_EVENT_MASK */
	.reg_ext_wakeup_event_mask = 0xFFFFFFFF,

	/* MP0_CPU0_WFI_EN */
	.mp0_cpu0_wfi_en = 0,

	/* MP0_CPU1_WFI_EN */
	.mp0_cpu1_wfi_en = 0,

	/* MP0_CPU2_WFI_EN */
	.mp0_cpu2_wfi_en = 0,

	/* MP0_CPU3_WFI_EN */
	.mp0_cpu3_wfi_en = 0,

	/* MP1_CPU0_WFI_EN */
	.mp1_cpu0_wfi_en = 0,

	/* MP1_CPU1_WFI_EN */
	.mp1_cpu1_wfi_en = 0,

	/* MP1_CPU2_WFI_EN */
	.mp1_cpu2_wfi_en = 0,

	/* MP1_CPU3_WFI_EN */
	.mp1_cpu3_wfi_en = 0,

	/* DEBUG0_WFI_EN */
	.debug0_wfi_en = 0,

	/* DEBUG1_WFI_EN */
	.debug1_wfi_en = 0,

	/* DEBUG2_WFI_EN */
	.debug2_wfi_en = 0,

	/* DEBUG3_WFI_EN */
	.debug3_wfi_en = 0,

	/* Auto-gen End */
};

struct spm_lp_scen __spm_vcorefs = {
	.pwrctrl	= &vcorefs_ctrl,
};

static void spm_dvfsfw_init(__uint64_t boot_up_opp, __uint64_t ch)
{
	__uint32_t dvfs_level[4] = { 0x8810, 0x4408, 0x2202, 0x1101};

	if (ch == DVFSRC_CHANNEL_2)
		dvfs_level[1] = 0x4408;
	else if (ch == DVFSRC_CHANNEL_4)
		dvfs_level[1] = 0x2404;

	mmio_write_32(DVFS_LEVEL, dvfs_level[boot_up_opp]);
}

static void spm_msdc_dvfs_setting(__uint64_t msdc, __uint64_t enable)
{
	mmio_write_32(SPM_SW_NONSERSV_3, (mmio_read_32(SPM_SW_NONSERSV_3) & ~(1U << msdc)) | (enable << msdc));
}

void __spm_sync_vcore_dvfs_power_control(struct pwr_ctrl *dest_pwr_ctrl, const struct pwr_ctrl *src_pwr_ctrl)
{

	__uint32_t dvfs_mask =  SPM_FLAG_DIS_VCORE_DVS | SPM_FLAG_DIS_VCORE_DFS;

	/* SPM_SRC_REQ */
	dest_pwr_ctrl->spm_src_req.bits.reg_spm_dvfs_level0_req = src_pwr_ctrl->spm_src_req.bits.reg_spm_dvfs_level0_req;
	dest_pwr_ctrl->spm_src_req.bits.reg_spm_dvfs_level1_req = src_pwr_ctrl->spm_src_req.bits.reg_spm_dvfs_level1_req;
	dest_pwr_ctrl->spm_src_req.bits.reg_spm_dvfs_level2_req = src_pwr_ctrl->spm_src_req.bits.reg_spm_dvfs_level2_req;
	dest_pwr_ctrl->spm_src_req.bits.reg_spm_dvfs_level3_req = src_pwr_ctrl->spm_src_req.bits.reg_spm_dvfs_level3_req;
	dest_pwr_ctrl->spm_src_req.bits.reg_spm_dvfs_level4_req = src_pwr_ctrl->spm_src_req.bits.reg_spm_dvfs_level4_req;

	/* SPM_SRC2_MASK */
	dest_pwr_ctrl->spm_src2_mask.bits.reg_next_dvfs_level0_mask_b = src_pwr_ctrl->spm_src2_mask.bits.reg_next_dvfs_level0_mask_b;
	dest_pwr_ctrl->spm_src2_mask.bits.reg_next_dvfs_level1_mask_b = src_pwr_ctrl->spm_src2_mask.bits.reg_next_dvfs_level1_mask_b;
	dest_pwr_ctrl->spm_src2_mask.bits.reg_next_dvfs_level2_mask_b = src_pwr_ctrl->spm_src2_mask.bits.reg_next_dvfs_level2_mask_b;
	dest_pwr_ctrl->spm_src2_mask.bits.reg_next_dvfs_level3_mask_b = src_pwr_ctrl->spm_src2_mask.bits.reg_next_dvfs_level3_mask_b;
	dest_pwr_ctrl->spm_src2_mask.bits.reg_next_dvfs_level4_mask_b = src_pwr_ctrl->spm_src2_mask.bits.reg_next_dvfs_level4_mask_b;

	dest_pwr_ctrl->pcm_flags = (dest_pwr_ctrl->pcm_flags & (~dvfs_mask)) |
					(src_pwr_ctrl->pcm_flags & dvfs_mask);

	if (dest_pwr_ctrl->pcm_flags_cust)
		dest_pwr_ctrl->pcm_flags_cust = (dest_pwr_ctrl->pcm_flags_cust & (~dvfs_mask)) |
						(src_pwr_ctrl->pcm_flags & dvfs_mask);
}

static void spm_go_to_vcorefs(__uint64_t spm_flags)
{
	struct pcm_desc *pcmdesc;
	struct pwr_ctrl *pwrctrl;

	if (dyna_load_pcm[DYNA_LOAD_PCM_SUSPEND].ready)
		pcmdesc = &(dyna_load_pcm[DYNA_LOAD_PCM_SUSPEND].desc);

	pwrctrl = __spm_vcorefs.pwrctrl;

	set_pwrctrl_pcm_flags(pwrctrl, spm_flags);

	__spm_reset_and_init_pcm(pcmdesc);

	__spm_kick_im_to_fetch(pcmdesc);

	__spm_init_pcm_register();

	__spm_init_event_vector(pcmdesc);

	__spm_set_power_control(pwrctrl);

	__spm_set_wakeup_event(pwrctrl);

	__spm_kick_pcm_to_run(pwrctrl);
}

void spm_vcorefs_args(__uint64_t x1, __uint64_t x2, __uint64_t x3)
{
	__uint64_t cmd = x1;

	switch (cmd) {
		case VCOREFS_SMC_CMD_0:
			spm_dvfsfw_init(x2, x3);
			break;
		case VCOREFS_SMC_CMD_1:
			spm_go_to_vcorefs(x2);
			break;
		case VCOREFS_SMC_CMD_2:
			spm_msdc_dvfs_setting(x2, x3);
			break;
		default:
			break;
	}
}

