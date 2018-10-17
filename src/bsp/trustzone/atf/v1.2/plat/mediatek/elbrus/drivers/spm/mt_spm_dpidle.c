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

/*
 * only for internal debug
 */
#define SPM_PCMWDT_EN           1
#define SPM_BYPASS_SYSPWREQ     1

struct wake_status spm_wakesta; /* record last wakesta */
static wake_reason_t spm_wake_reason = WR_NONE;

/*
 * SW code for SPM deepidle
 */
#if defined(CONFIG_MICROTRUST_TEE_SUPPORT)
#define WAKE_SRC_FOR_DPIDLE \
	(WAKE_SRC_R12_PCM_TIMER | \
	WAKE_SRC_R12_APXGPT1_EVENT_B |      \
	WAKE_SRC_R12_PMCU_WDT_EVENT_B | \
	WAKE_SRC_R12_KP_IRQ_B | \
	WAKE_SRC_R12_CONN2AP_SPM_WAKEUP_B | \
	WAKE_SRC_R12_EINT_EVENT_B | \
	WAKE_SRC_R12_CONN_WDT_IRQ_B | \
	WAKE_SRC_R12_CCIF0_EVENT_B | \
	WAKE_SRC_R12_CCIF1_EVENT_B | \
	WAKE_SRC_R12_PMCU_SPM_IRQ_B | \
	WAKE_SRC_R12_USBX_CDSC_B | \
	WAKE_SRC_R12_USBX_POWERDWN_B | \
	WAKE_SRC_R12_EINT_EVENT_SECURE_B | \
	WAKE_SRC_R12_MD1_WDT_B | \
	WAKE_SRC_R12_CLDMA_EVENT_B | \
	WAKE_SRC_R12_SCP_SPM_IRQ_B | \
	WAKE_SRC_R12_SCP_WDT_EVENT_B | \
	WAKE_SRC_R12_C2K_WDT_IRQ_B | \
	WAKE_SRC_R12_SYS_CIRQ_IRQ_B | \
	WAKE_SRC_R12_CSYSPWREQ_B | \
	WAKE_SRC_R12_ALL_MD32_WAKEUP_B)
#else
#define WAKE_SRC_FOR_DPIDLE \
	(WAKE_SRC_R12_PCM_TIMER | \
	WAKE_SRC_R12_APXGPT1_EVENT_B |      \
	WAKE_SRC_R12_PMCU_WDT_EVENT_B | \
	WAKE_SRC_R12_KP_IRQ_B | \
	WAKE_SRC_R12_CONN2AP_SPM_WAKEUP_B | \
	WAKE_SRC_R12_EINT_EVENT_B | \
	WAKE_SRC_R12_CONN_WDT_IRQ_B | \
	WAKE_SRC_R12_CCIF0_EVENT_B | \
	WAKE_SRC_R12_CCIF1_EVENT_B | \
	WAKE_SRC_R12_PMCU_SPM_IRQ_B | \
	WAKE_SRC_R12_USBX_CDSC_B | \
	WAKE_SRC_R12_USBX_POWERDWN_B | \
	WAKE_SRC_R12_EINT_EVENT_SECURE_B | \
	WAKE_SRC_R12_MD1_WDT_B | \
	WAKE_SRC_R12_CLDMA_EVENT_B | \
	WAKE_SRC_R12_SCP_SPM_IRQ_B | \
	WAKE_SRC_R12_SCP_WDT_EVENT_B | \
	WAKE_SRC_R12_C2K_WDT_IRQ_B | \
	WAKE_SRC_R12_SYS_CIRQ_IRQ_B | \
	WAKE_SRC_R12_CSYSPWREQ_B | \
	WAKE_SRC_R12_SEJ_EVENT_B | \
	WAKE_SRC_R12_ALL_MD32_WAKEUP_B)
#endif /* #if defined(CONFIG_MICROTRUST_TEE_SUPPORT) */

static struct pwr_ctrl dpidle_ctrl = {
	.wake_src = WAKE_SRC_FOR_DPIDLE,

#if SPM_BYPASS_SYSPWREQ
	.syspwreq_mask = 1,
#endif

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
	.spm_ap_standby_con.bits.reg_mpwfi_op = 1, /* WFI_OP_AND */
	.spm_ap_standby_con.bits.reg_mp0_cputop_idle_mask = 0,
	.spm_ap_standby_con.bits.reg_mp1_cputop_idle_mask = 0,
	.spm_ap_standby_con.bits.reg_debugtop_idle_mask = 0,
	.spm_ap_standby_con.bits.reg_mp_top_idle_mask = 0,
	.spm_ap_standby_con.bits.reg_mcusys_idle_mask = 0,
	.spm_ap_standby_con.bits.reg_md_ddr_en_0_dbc_en = 0,
	.spm_ap_standby_con.bits.reg_md_ddr_en_1_dbc_en = 0,
	.spm_ap_standby_con.bits.reg_conn_ddr_en_dbc_en = 0,
	.spm_ap_standby_con.bits.reg_md32_mask_b = 1,
	.spm_ap_standby_con.bits.reg_md_0_mask_b = 1,
	.spm_ap_standby_con.bits.reg_md_1_mask_b = 0,
	.spm_ap_standby_con.bits.reg_scp_mask_b = 1,
	.spm_ap_standby_con.bits.reg_srcclkeni0_mask_b = 1,
	.spm_ap_standby_con.bits.reg_srcclkeni1_mask_b = 1,
	.spm_ap_standby_con.bits.reg_md_apsrc_1_sel = 0,
	.spm_ap_standby_con.bits.reg_md_apsrc_0_sel = 0,
	.spm_ap_standby_con.bits.reg_conn_mask_b = 1,
	.spm_ap_standby_con.bits.reg_conn_apsrc_sel = 0,

	/* SPM_SRC_REQ */
	.spm_src_req.bits.reg_spm_apsrc_req = 1,	/* main code only */
	.spm_src_req.bits.reg_spm_f26m_req = 1,		/* main code only */
	.spm_src_req.bits.reg_spm_infra_req = 1,	/* main code only */
	.spm_src_req.bits.reg_spm_ddren_req = 1,	/* main code only */
	.spm_src_req.bits.reg_spm_vrf18_req = 1,	/* main code only */
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
	.spm_src_mask.bits.reg_md_apsrc_req_0_infra_mask_b = 1,
	.spm_src_mask.bits.reg_md_apsrc_req_1_infra_mask_b = 0,
	.spm_src_mask.bits.reg_conn_srcclkena_infra_mask_b = 0,
	.spm_src_mask.bits.reg_conn_infra_req_mask_b = 1,
	.spm_src_mask.bits.reg_md32_srcclkena_infra_mask_b = 1,
	.spm_src_mask.bits.reg_md32_infra_req_mask_b = 1,
	.spm_src_mask.bits.reg_scp_srcclkena_infra_mask_b = 1,
	.spm_src_mask.bits.reg_scp_infra_req_mask_b = 1,
	.spm_src_mask.bits.reg_srcclkeni0_infra_mask_b = 0,
	.spm_src_mask.bits.reg_srcclkeni1_infra_mask_b = 0,
	.spm_src_mask.bits.reg_ccif0_md_event_mask_b = 1,
	.spm_src_mask.bits.reg_ccif0_ap_event_mask_b = 1,
	.spm_src_mask.bits.reg_ccif1_md_event_mask_b = 1,
	.spm_src_mask.bits.reg_ccif1_ap_event_mask_b = 1,
	.spm_src_mask.bits.reg_ccif2_md_event_mask_b = 1,
	.spm_src_mask.bits.reg_ccif2_ap_event_mask_b = 1,
	.spm_src_mask.bits.reg_ccif3_md_event_mask_b = 1,
	.spm_src_mask.bits.reg_ccif3_ap_event_mask_b = 1,
	.spm_src_mask.bits.reg_ccifmd_md1_event_mask_b = 0,
	.spm_src_mask.bits.reg_ccifmd_md2_event_mask_b = 0,
	.spm_src_mask.bits.reg_c2k_ps_rccif_wake_mask_b = 1,
	.spm_src_mask.bits.reg_c2k_l1_rccif_wake_mask_b = 0,
	.spm_src_mask.bits.reg_ps_c2k_rccif_wake_mask_b = 1,
	.spm_src_mask.bits.reg_l1_c2k_rccif_wake_mask_b = 0,
	.spm_src_mask.bits.reg_dqssoc_req_mask_b = 0,
	.spm_src_mask.bits.reg_disp2_req_mask_b = 0,
	.spm_src_mask.bits.reg_md_ddr_en_0_mask_b = 1,
	.spm_src_mask.bits.reg_md_ddr_en_1_mask_b = 0,
	.spm_src_mask.bits.reg_conn_ddr_en_mask_b = 1,

	/* SPM_SRC2_MASK */
	.spm_src2_mask.bits.reg_disp0_req_mask_b = 0,
	.spm_src2_mask.bits.reg_disp1_req_mask_b = 0,
	.spm_src2_mask.bits.reg_disp_od_req_mask_b = 0,
	.spm_src2_mask.bits.reg_mfg_req_mask_b = 0,
	.spm_src2_mask.bits.reg_vdec0_req_mask_b = 0,
	.spm_src2_mask.bits.reg_gce_vrf18_req_mask_b = 0,
	.spm_src2_mask.bits.reg_gce_req_mask_b = 0,
	.spm_src2_mask.bits.reg_lpdma_req_mask_b = 0,
	.spm_src2_mask.bits.reg_srcclkeni1_cksel2_mask_b = 1,
	.spm_src2_mask.bits.reg_conn_srcclkena_cksel2_mask_b = 1,
	.spm_src2_mask.bits.reg_srcclkeni0_cksel3_mask_b = 1,
	.spm_src2_mask.bits.reg_md32_apsrc_req_ddren_mask_b = 0,
	.spm_src2_mask.bits.reg_scp_apsrc_req_ddren_mask_b = 1,
	.spm_src2_mask.bits.reg_md_vrf18_req_0_mask_b = 1,
	.spm_src2_mask.bits.reg_md_vrf18_req_1_mask_b = 0,
	.spm_src2_mask.bits.reg_next_dvfs_level0_mask_b = 0,
	.spm_src2_mask.bits.reg_next_dvfs_level1_mask_b = 0,
	.spm_src2_mask.bits.reg_next_dvfs_level2_mask_b = 0,
	.spm_src2_mask.bits.reg_next_dvfs_level3_mask_b = 0,
	.spm_src2_mask.bits.reg_next_dvfs_level4_mask_b = 0,
	.spm_src2_mask.bits.reg_msdc1_dvfs_halt_mask = 1,
	.spm_src2_mask.bits.reg_msdc2_dvfs_halt_mask = 1,
	.spm_src2_mask.bits.reg_msdc3_dvfs_halt_mask = 1,
	.spm_src2_mask.bits.reg_sw2spm_int0_mask_b = 1,
	.spm_src2_mask.bits.reg_sw2spm_int1_mask_b = 1,
	.spm_src2_mask.bits.reg_sw2spm_int2_mask_b = 1,
	.spm_src2_mask.bits.reg_sw2spm_int3_mask_b = 1,
	.spm_src2_mask.bits.reg_pmcu2spm_int0_mask_b = 1,
	.spm_src2_mask.bits.reg_pmcu2spm_int1_mask_b = 1,
	.spm_src2_mask.bits.reg_pmcu2spm_int2_mask_b = 1,
	.spm_src2_mask.bits.reg_pmcu2spm_int3_mask_b = 1,

	/* SPM_WAKEUP_EVENT_MASK */
	.reg_wakeup_event_mask = 0xF0F82218,

	/* SPM_EXT_WAKEUP_EVENT_MASK */
	.reg_ext_wakeup_event_mask = 0,

	/* MP0_CPU0_WFI_EN */
	.mp0_cpu0_wfi_en = 1,

	/* MP0_CPU1_WFI_EN */
	.mp0_cpu1_wfi_en = 1,

	/* MP0_CPU2_WFI_EN */
	.mp0_cpu2_wfi_en = 1,

	/* MP0_CPU3_WFI_EN */
	.mp0_cpu3_wfi_en = 1,

	/* MP1_CPU0_WFI_EN */
	.mp1_cpu0_wfi_en = 1,

	/* MP1_CPU1_WFI_EN */
	.mp1_cpu1_wfi_en = 1,

	/* MP1_CPU2_WFI_EN */
	.mp1_cpu2_wfi_en = 1,

	/* MP1_CPU3_WFI_EN */
	.mp1_cpu3_wfi_en = 1,

	/* DEBUG0_WFI_EN */
	.debug0_wfi_en = 1,

	/* DEBUG1_WFI_EN */
	.debug1_wfi_en = 1,

	/* DEBUG2_WFI_EN */
	.debug2_wfi_en = 0,

	/* DEBUG3_WFI_EN */
	.debug3_wfi_en = 0,

	/* Auto-gen End */
};

struct spm_lp_scen __spm_dpidle = {
	.pwrctrl = &dpidle_ctrl,
};

void spm_dpidle_args(__uint64_t x1, __uint64_t x2, __uint64_t x3)
{
	struct pwr_ctrl *pwrctrl;

	pwrctrl = __spm_dpidle.pwrctrl;
	pwrctrl->pcm_flags = x1;
}

void go_to_dpidle_before_wfi(void)
{
	struct pcm_desc *pcmdesc = NULL;
	struct pwr_ctrl *pwrctrl;
	uint64_t mpidr = read_mpidr();
	__uint32_t cpu = platform_get_core_pos(mpidr);

	if (dyna_load_pcm[DYNA_LOAD_PCM_SUSPEND].ready)
		pcmdesc = &(dyna_load_pcm[DYNA_LOAD_PCM_SUSPEND].desc);
	pwrctrl = __spm_dpidle.pwrctrl;

	INFO("Online CPU is %d, suspend FW ver. is %s\n",
			cpu, pcmdesc->version);

	INFO("sec = %u, wakesrc = 0x%x (%u)(%u)\n",
			pwrctrl->timer_val, pwrctrl->wake_src,
			is_cpu_pdn(pwrctrl->pcm_flags),
			is_infra_pdn(pwrctrl->pcm_flags));

	__spm_set_cpu_status(cpu);
	__spm_reset_and_init_pcm(pcmdesc);
	__spm_kick_im_to_fetch(pcmdesc);
	__spm_init_pcm_register();
	__spm_init_event_vector(pcmdesc);

	__spm_sync_vcore_dvfs_power_control(pwrctrl, __spm_vcorefs.pwrctrl);

	__spm_set_power_control(pwrctrl);
	__spm_set_wakeup_event(pwrctrl);

/* TODO: sleep deepidle */
#if 0
#if SPM_PCMWDT_EN
	if (!pwrctrl->wdt_disable)
		__spm_set_pcm_wdt(1);
#endif
#endif

	__spm_kick_pcm_to_run(pwrctrl);

}

void go_to_dpidle_after_wfi(void)
{
	struct pcm_desc *pcmdesc = NULL;
/* TODO: sleep deepidle */
#if 0
	struct pwr_ctrl *pwrctrl;
#endif

	if (dyna_load_pcm[DYNA_LOAD_PCM_SUSPEND].ready)
		pcmdesc = &(dyna_load_pcm[DYNA_LOAD_PCM_SUSPEND].desc);

/* TODO: sleep deepidle */
#if 0
	pwrctrl = __spm_dpidle.pwrctrl;

#if SPM_PCMWDT_EN
	if (!pwrctrl->wdt_disable)
		__spm_set_pcm_wdt(0);
#endif
#endif

	__spm_get_wakeup_status(&spm_wakesta);
	__spm_clean_after_wakeup();
	spm_wake_reason = __spm_output_wake_reason(&spm_wakesta, pcmdesc);

	INFO("spm_wake_reason=%d\n", spm_wake_reason);
}

void spm_dpidle(void)
{
	spm_lock_get();
	go_to_dpidle_before_wfi();
	spm_lock_release();
}

void spm_dpidle_finish(void)
{
	spm_lock_get();
	go_to_dpidle_after_wfi();
	spm_lock_release();
}
