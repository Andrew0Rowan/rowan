#include <debug.h>
#include <delay_timer.h>
#include <mmio.h>
#include <mt_spm.h>
#include <mt_spm_internal.h>
#include <mt_spm_reg.h>
#include <mt_spm_resource_req.h>
#include <platform_def.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

/**************************************
 * Config and Parameter
 **************************************/

/**************************************
 * Define and Declare
 **************************************/

/**************************************
 * Function and API
 **************************************/

int spm_for_gps_flag;

wake_reason_t __spm_output_wake_reason(const struct wake_status *wakesta,
		                                       const struct pcm_desc *pcmdesc)
{
	int i;
	wake_reason_t wr = WR_UNKNOWN;

	if (wakesta->assert_pc != 0) {
		/* add size check for vcoredvfs */
		ERROR("PCM ASSERT AT %u (%s%s), r13 = 0x%x, debug_flag = 0x%x\n",
				wakesta->assert_pc, (wakesta->assert_pc > pcmdesc->size) ? "NOT " : "",
				pcmdesc->version, wakesta->r13, wakesta->debug_flag);
		return WR_PCM_ASSERT;
	}

	if (wakesta->r12 & WAKE_SRC_R12_PCMTIMER) {
		if (wakesta->wake_misc & WAKE_MISC_PCM_TIMER) {
			wr = WR_PCM_TIMER;
		}
		if (wakesta->wake_misc & WAKE_MISC_TWAM) {
			wr = WR_WAKE_SRC;
		}
		if (wakesta->wake_misc & WAKE_MISC_CPU_WAKE) {
			wr = WR_WAKE_SRC;
		}
	}
	for (i = 1; i < 32; i++) {
		if (wakesta->r12 & (1U << i))
			wr = WR_WAKE_SRC;
	}

	INFO("timer_out = %u, r13 = 0x%x, debug_flag = 0x%x\n",
			wakesta->timer_out, wakesta->r13, wakesta->debug_flag);

	INFO("r12 = 0x%x, r12_ext = 0x%x, raw_sta = 0x%x, idle_sta = 0x%x, event_reg = 0x%x, isr = 0x%x\n",
			wakesta->r12, wakesta->r12_ext, wakesta->raw_sta, wakesta->idle_sta,
			wakesta->event_reg, wakesta->isr);

	INFO("raw_ext_sta = 0x%x, wake_misc = 0x%x", wakesta->raw_ext_sta,
			wakesta->wake_misc);
	return wr;
}

void __spm_set_cpu_status(int cpu)
{
	if (cpu >= 0 && cpu < 4) {
		mmio_write_32(COMMON_TOP_PWR_ADDR, 0x10b00208);
		mmio_write_32(COMMON_CPU_PWR_ADDR, 0x10b0020C);
		mmio_write_32(ARMPLL_CLK_CON,
				(mmio_read_32(ARMPLL_CLK_CON) & ~(MUXSEL_SC_ARMPLL2_LSB | MUXSEL_SC_ARMPLL3_LSB)) |
				(MUXSEL_SC_CCIPLL_LSB | MUXSEL_SC_ARMPLL1_LSB));
	} else if (cpu >= 4 && cpu < 8) {
		mmio_write_32(COMMON_TOP_PWR_ADDR, 0x10b0021C);
		mmio_write_32(COMMON_CPU_PWR_ADDR, 0x10b00220);
		mmio_write_32(ARMPLL_CLK_CON,
				(mmio_read_32(ARMPLL_CLK_CON) & ~(MUXSEL_SC_ARMPLL1_LSB | MUXSEL_SC_ARMPLL3_LSB)) |
				(MUXSEL_SC_CCIPLL_LSB | MUXSEL_SC_ARMPLL2_LSB));
	} else {
		ERROR("%s: error cpu number %d\n", __func__, cpu);
	}
}

static void spm_code_swapping(void)
{
	__uint32_t con1;
	int retry = 0, timeout = 5000;

	con1 = mmio_read_32(SPM_WAKEUP_EVENT_MASK);

	mmio_write_32(SPM_WAKEUP_EVENT_MASK, (con1 & ~(0x1)));
	mmio_write_32(SPM_CPU_WAKEUP_EVENT, 1);

	while ((mmio_read_32(SPM_IRQ_STA) & RG_PCM_IRQ_MSK_LSB) == 0) {
		if (retry > timeout) {
			ERROR("[%s] r15: 0x%x, r6: 0x%x, r1: 0x%x, pcmsta: 0x%x, irqsta: 0x%x [%d]\n",
				__func__,
				mmio_read_32(PCM_REG15_DATA), mmio_read_32(PCM_REG6_DATA), mmio_read_32(PCM_REG1_DATA),
				mmio_read_32(PCM_FSM_STA), mmio_read_32(SPM_IRQ_STA), timeout);
		}
		udelay(1);
		retry++;
	}

	mmio_write_32(SPM_CPU_WAKEUP_EVENT, 0);
	mmio_write_32(SPM_WAKEUP_EVENT_MASK, con1);
}

void __spm_reset_and_init_pcm(const struct pcm_desc *pcmdesc)
{
	__uint32_t con1;
	__uint32_t spm_pwr_on_val0_mask = 0;
	__uint32_t spm_pwr_on_val0_read = 0;
	__uint32_t spm_pwr_on_val0_write = 0;

	/* SPM code swapping */
	if (mmio_read_32(PCM_REG1_DATA) == 0x1)
	if ((mmio_read_32(PCM_REG1_DATA) == 0x1) && !(mmio_read_32(PCM_REG15_DATA) == 0x0))
		spm_code_swapping();

	/* backup mem control from r0 to POWER_ON_VAL0 */
	if (!(mmio_read_32(PCM_REG1_DATA) == 0x1))
		spm_pwr_on_val0_mask = 0x30F80000;
	else
		spm_pwr_on_val0_mask = 0x10F80000;

	spm_pwr_on_val0_read = mmio_read_32(SPM_POWER_ON_VAL0);
	spm_pwr_on_val0_read &= spm_pwr_on_val0_mask;

	spm_pwr_on_val0_write = mmio_read_32(PCM_REG0_DATA);
	spm_pwr_on_val0_write &= ~spm_pwr_on_val0_mask;
	spm_pwr_on_val0_write |= spm_pwr_on_val0_read;

	if (mmio_read_32(SPM_POWER_ON_VAL0) != spm_pwr_on_val0_write) {
		VERBOSE("VAL0 from 0x%x to 0x%x\n", mmio_read_32(SPM_POWER_ON_VAL0), spm_pwr_on_val0_write);
		mmio_write_32(SPM_POWER_ON_VAL0, spm_pwr_on_val0_write);
	}

	/* disable r0 and r7 to control power */
	mmio_write_32(PCM_PWR_IO_EN, 0);

	/* disable pcm timer after leaving FW */
	mmio_write_32(PCM_CON1, SPM_REGWR_CFG_KEY | (mmio_read_32(PCM_CON1) & ~RG_PCM_TIMER_EN_LSB));

	/* reset PCM */
	mmio_write_32(PCM_CON0, SPM_REGWR_CFG_KEY | PCM_CK_EN_LSB | PCM_SW_RESET_LSB);
	mmio_write_32(PCM_CON0, SPM_REGWR_CFG_KEY | PCM_CK_EN_LSB);
	if ((mmio_read_32(PCM_FSM_STA) & 0x7fffff) != PCM_FSM_STA_DEF)
		ERROR("reset pcm(PCM_FSM_STA=0x%x)\n", mmio_read_32(PCM_FSM_STA));

	/* init PCM_CON0 (disable event vector) */
	mmio_write_32(PCM_CON0, SPM_REGWR_CFG_KEY | PCM_CK_EN_LSB | RG_EN_IM_SLEEP_DVS_LSB);

	/* init PCM_CON1 (disable PCM timer but keep PCM WDT setting) */
	con1 = mmio_read_32(PCM_CON1) & (RG_PCM_WDT_WAKE_LSB | RG_PCM_WDT_EN_LSB);
	mmio_write_32(PCM_CON1, con1 | SPM_REGWR_CFG_KEY | REG_EVENT_LOCK_EN_LSB |
			REG_SPM_SRAM_ISOINT_B_LSB | RG_PCM_WDT_EN_LSB |
			(pcmdesc->replace ? 0 : RG_IM_NONRP_EN_LSB) |
			RG_AHBMIF_APBEN_LSB | RG_SSPM_APB_INTERNAL_EN_LSB);
}

void __spm_kick_im_to_fetch(const struct pcm_desc *pcmdesc)
{
	__uint32_t ptr, len, con0;

	/* tell IM where is PCM code (use slave mode if code existed) */
	ptr = pcmdesc->base_dma;
	len = pcmdesc->size - 1;
	if (mmio_read_32(PCM_IM_PTR) != ptr || mmio_read_32(PCM_IM_LEN) != len || pcmdesc->sess > 2) {
		mmio_write_32(PCM_IM_PTR, ptr);
		mmio_write_32(PCM_IM_LEN, len);
	} else {
		mmio_write_32(PCM_CON1, mmio_read_32(PCM_CON1) | SPM_REGWR_CFG_KEY | RG_IM_SLAVE_LSB);
	}

	/* kick IM to fetch (only toggle IM_KICK) */
	con0 = mmio_read_32(PCM_CON0) & ~(RG_IM_KICK_L_LSB | RG_PCM_KICK_L_LSB);
	mmio_write_32(PCM_CON0, con0 | SPM_REGWR_CFG_KEY | PCM_CK_EN_LSB | RG_IM_KICK_L_LSB);
	mmio_write_32(PCM_CON0, con0 | SPM_REGWR_CFG_KEY | PCM_CK_EN_LSB);
}

void __spm_init_pcm_register(void)
{
	/* init r0 with POWER_ON_VAL0 */
	mmio_write_32(PCM_REG_DATA_INI, mmio_read_32(SPM_POWER_ON_VAL0));
	mmio_write_32(PCM_PWR_IO_EN, PCM_RF_SYNC_R0);
	mmio_write_32(PCM_PWR_IO_EN, 0);

	/* init r7 with POWER_ON_VAL1 */
	mmio_write_32(PCM_REG_DATA_INI, mmio_read_32(SPM_POWER_ON_VAL1));
	mmio_write_32(PCM_PWR_IO_EN, PCM_RF_SYNC_R7);
	mmio_write_32(PCM_PWR_IO_EN, 0);
}

void __spm_init_event_vector(const struct pcm_desc *pcmdesc)
{
	/* init event vector register */
	mmio_write_32(PCM_EVENT_VECTOR0, pcmdesc->vec0);
	mmio_write_32(PCM_EVENT_VECTOR1, pcmdesc->vec1);
	mmio_write_32(PCM_EVENT_VECTOR2, pcmdesc->vec2);
	mmio_write_32(PCM_EVENT_VECTOR3, pcmdesc->vec3);
	mmio_write_32(PCM_EVENT_VECTOR4, pcmdesc->vec4);
	mmio_write_32(PCM_EVENT_VECTOR5, pcmdesc->vec5);
	mmio_write_32(PCM_EVENT_VECTOR6, pcmdesc->vec6);
	mmio_write_32(PCM_EVENT_VECTOR7, pcmdesc->vec7);
	mmio_write_32(PCM_EVENT_VECTOR8, pcmdesc->vec8);
	mmio_write_32(PCM_EVENT_VECTOR9, pcmdesc->vec9);
	mmio_write_32(PCM_EVENT_VECTOR10, pcmdesc->vec10);
	mmio_write_32(PCM_EVENT_VECTOR11, pcmdesc->vec11);
	mmio_write_32(PCM_EVENT_VECTOR12, pcmdesc->vec12);
	mmio_write_32(PCM_EVENT_VECTOR13, pcmdesc->vec13);
	mmio_write_32(PCM_EVENT_VECTOR14, pcmdesc->vec14);
	mmio_write_32(PCM_EVENT_VECTOR15, pcmdesc->vec15);
}

void __spm_src_req_update(const struct pwr_ctrl *pwrctrl, unsigned int resource_usage)
{
	__uint8_t spm_vrf18_req = (resource_usage & SPM_RESOURCE_MAINPLL) ? 1 : pwrctrl->reg_spm_vrf18_req;
	__uint8_t spm_apsrc_req = (resource_usage & SPM_RESOURCE_DRAM)    ? 1 : pwrctrl->reg_spm_apsrc_req;
	__uint8_t spm_ddren_req = (resource_usage & SPM_RESOURCE_DRAM)    ? 1 : pwrctrl->reg_spm_ddren_req;
	__uint8_t spm_f26m_req  = (resource_usage & SPM_RESOURCE_CK_26M)  ? 1 : pwrctrl->reg_spm_f26m_req;

	/* SPM_SRC_REQ */
	mmio_write_32(SPM_SRC_REQ,
		((spm_apsrc_req & 0x1) << 0) |
		((spm_f26m_req & 0x1) << 1) |
		((pwrctrl->reg_spm_infra_req & 0x1) << 2) |
		((spm_ddren_req & 0x1) << 3) |
		((spm_vrf18_req & 0x1) << 4) |
		((pwrctrl->reg_spm_dvfs_level0_req & 0x1) << 5) |
		((pwrctrl->reg_spm_dvfs_level1_req & 0x1) << 6) |
		((pwrctrl->reg_spm_dvfs_level2_req & 0x1) << 7) |
		((pwrctrl->reg_spm_dvfs_level3_req & 0x1) << 8) |
		((pwrctrl->reg_spm_dvfs_level4_req & 0x1) << 9) |
		((pwrctrl->reg_spm_sspm_mailbox_req & 0x1) << 10) |
		((pwrctrl->reg_spm_sw_mailbox_req & 0x1) << 11) |
		((pwrctrl->reg_spm_cksel2_req & 0x1) << 12) |
		((pwrctrl->reg_spm_cksel3_req & 0x1) << 13));
}


void __spm_set_power_control(const struct pwr_ctrl *pwrctrl)
{
	/* Auto-gen Start */

	/* SPM_CLK_CON */
	mmio_write_32(SPM_CLK_CON,
		((pwrctrl->reg_srcclken0_ctl & 0x3) << 0) |
		((pwrctrl->reg_srcclken1_ctl & 0x3) << 2) |
		((pwrctrl->reg_spm_lock_infra_dcm & 0x1) << 5) |
		((pwrctrl->reg_srcclken_mask & 0x7) << 6) |
		((pwrctrl->reg_md1_c32rm_en & 0x1) << 9) |
		((pwrctrl->reg_md2_c32rm_en & 0x1) << 10) |
		((pwrctrl->reg_clksq0_sel_ctrl & 0x1) << 11) |
		((pwrctrl->reg_clksq1_sel_ctrl & 0x1) << 12) |
		((pwrctrl->reg_srcclken0_en & 0x1) << 13) |
		((pwrctrl->reg_srcclken1_en & 0x1) << 14) |
		((pwrctrl->reg_sysclk0_src_mask_b & 0x7f) << 16) |
		((pwrctrl->reg_sysclk1_src_mask_b & 0x7f) << 23));

	/* SPM_SRC_REQ */
	mmio_write_32(SPM_SRC_REQ,
		((pwrctrl->reg_spm_apsrc_req & 0x1) << 0) |
		((pwrctrl->reg_spm_f26m_req & 0x1) << 1) |
		((pwrctrl->reg_spm_infra_req & 0x1) << 2) |
		((pwrctrl->reg_spm_ddren_req & 0x1) << 3) |
		((pwrctrl->reg_spm_vrf18_req & 0x1) << 4) |
		((pwrctrl->reg_spm_dvfs_level0_req & 0x1) << 5) |
		((pwrctrl->reg_spm_dvfs_level1_req & 0x1) << 6) |
		((pwrctrl->reg_spm_dvfs_level2_req & 0x1) << 7) |
		((pwrctrl->reg_spm_dvfs_level3_req & 0x1) << 8) |
		((pwrctrl->reg_spm_dvfs_level4_req & 0x1) << 9) |
		((pwrctrl->reg_spm_sspm_mailbox_req & 0x1) << 10) |
		((pwrctrl->reg_spm_sw_mailbox_req & 0x1) << 11) |
		((pwrctrl->reg_spm_cksel2_req & 0x1) << 12) |
		((pwrctrl->reg_spm_cksel3_req & 0x1) << 13));

	/* SPM_SRC_MASK */
	mmio_write_32(SPM_SRC_MASK,
		((pwrctrl->reg_csyspwreq_mask & 0x1) << 0) |
		((pwrctrl->reg_md_srcclkena_0_infra_mask_b & 0x1) << 1) |
		((pwrctrl->reg_md_srcclkena_1_infra_mask_b & 0x1) << 2) |
		((pwrctrl->reg_md_apsrc_req_0_infra_mask_b & 0x1) << 3) |
		((pwrctrl->reg_md_apsrc_req_1_infra_mask_b & 0x1) << 4) |
		((pwrctrl->reg_conn_srcclkena_infra_mask_b & 0x1) << 5) |
		((pwrctrl->reg_conn_infra_req_mask_b & 0x1) << 6) |
		((pwrctrl->reg_sspm_srcclkena_infra_mask_b & 0x1) << 7) |
		((pwrctrl->reg_sspm_infra_req_mask_b & 0x1) << 8) |
		((pwrctrl->reg_scp_srcclkena_infra_mask_b & 0x1) << 9) |
		((pwrctrl->reg_scp_infra_req_mask_b & 0x1) << 10) |
		((pwrctrl->reg_srcclkeni0_infra_mask_b & 0x1) << 11) |
		((pwrctrl->reg_srcclkeni1_infra_mask_b & 0x1) << 12) |
		((pwrctrl->reg_srcclkeni2_infra_mask_b & 0x1) << 13) |
		((pwrctrl->reg_ccif0_md_event_mask_b & 0x1) << 14) |
		((pwrctrl->reg_ccif0_ap_event_mask_b & 0x1) << 15) |
		((pwrctrl->reg_ccif1_md_event_mask_b & 0x1) << 16) |
		((pwrctrl->reg_ccif1_ap_event_mask_b & 0x1) << 17) |
		((pwrctrl->reg_ccif2_md_event_mask_b & 0x1) << 18) |
		((pwrctrl->reg_ccif2_ap_event_mask_b & 0x1) << 19) |
		((pwrctrl->reg_ccif3_md_event_mask_b & 0x1) << 20) |
		((pwrctrl->reg_ccif3_ap_event_mask_b & 0x1) << 21) |
		((pwrctrl->reg_ccifmd_md1_event_mask_b & 0x1) << 22) |
		((pwrctrl->reg_ccifmd_md2_event_mask_b & 0x1) << 23) |
		((pwrctrl->reg_c2k_ps_rccif_wake_mask_b & 0x1) << 24) |
		((pwrctrl->reg_c2k_l1_rccif_wake_mask_b & 0x1) << 25) |
		((pwrctrl->reg_ps_c2k_rccif_wake_mask_b & 0x1) << 26) |
		((pwrctrl->reg_l1_c2k_rccif_wake_mask_b & 0x1) << 27) |
		((pwrctrl->reg_disp2_req_mask_b & 0x1) << 28) |
		((pwrctrl->reg_md_ddr_en_0_mask_b & 0x1) << 29) |
		((pwrctrl->reg_md_ddr_en_1_mask_b & 0x1) << 30) |
		((pwrctrl->reg_conn_ddr_en_mask_b & 0x1) << 31));

	/* SPM_SRC2_MASK */
	mmio_write_32(SPM_SRC2_MASK,
		((pwrctrl->reg_disp0_req_mask_b & 0x1) << 0) |
		((pwrctrl->reg_disp1_req_mask_b & 0x1) << 1) |
		((pwrctrl->reg_disp_od_req_mask_b & 0x1) << 2) |
		((pwrctrl->reg_mfg_req_mask_b & 0x1) << 3) |
		((pwrctrl->reg_vdec0_req_mask_b & 0x1) << 4) |
		((pwrctrl->reg_gce_req_mask_b & 0x1) << 5) |
		((pwrctrl->reg_gce_vrf18_req_mask_b & 0x1) << 6) |
		((pwrctrl->reg_lpdma_req_mask_b & 0x1) << 7) |
		((pwrctrl->reg_conn_srcclkena_cksel2_mask_b & 0x1) << 8) |
		((pwrctrl->reg_sspm_apsrc_req_ddren_mask_b & 0x1) << 9) |
		((pwrctrl->reg_scp_apsrc_req_ddren_mask_b & 0x1) << 10) |
		((pwrctrl->reg_md_vrf18_req_0_mask_b & 0x1) << 11) |
		((pwrctrl->reg_md_vrf18_req_1_mask_b & 0x1) << 12) |
		((pwrctrl->reg_next_dvfs_level0_mask_b & 0x1) << 13) |
		((pwrctrl->reg_next_dvfs_level1_mask_b & 0x1) << 14) |
		((pwrctrl->reg_next_dvfs_level2_mask_b & 0x1) << 15) |
		((pwrctrl->reg_next_dvfs_level3_mask_b & 0x1) << 16) |
		((pwrctrl->reg_next_dvfs_level4_mask_b & 0x1) << 17) |
		((pwrctrl->reg_sw2spm_int0_mask_b & 0x1) << 18) |
		((pwrctrl->reg_sw2spm_int1_mask_b & 0x1) << 19) |
		((pwrctrl->reg_sw2spm_int2_mask_b & 0x1) << 20) |
		((pwrctrl->reg_sw2spm_int3_mask_b & 0x1) << 21) |
		((pwrctrl->reg_sspm2spm_int0_mask_b & 0x1) << 22) |
		((pwrctrl->reg_sspm2spm_int1_mask_b & 0x1) << 23) |
		((pwrctrl->reg_sspm2spm_int2_mask_b & 0x1) << 24) |
		((pwrctrl->reg_sspm2spm_int3_mask_b & 0x1) << 25) |
		((pwrctrl->reg_dqssoc_req_mask_b & 0x1) << 26));

	/* SPM_SRC3_MASK */
	mmio_write_32(SPM_SRC3_MASK,
		((pwrctrl->reg_mpwfi_op & 0x1) << 0) |
		((pwrctrl->reg_spm_resource_req_rsv1_4_mask_b & 0x1) << 1) |
		((pwrctrl->reg_spm_resource_req_rsv1_3_mask_b & 0x1) << 2) |
		((pwrctrl->reg_spm_resource_req_rsv1_2_mask_b & 0x1) << 3) |
		((pwrctrl->reg_spm_resource_req_rsv1_1_mask_b & 0x1) << 4) |
		((pwrctrl->reg_spm_resource_req_rsv1_0_mask_b & 0x1) << 5) |
		((pwrctrl->reg_spm_resource_req_rsv0_4_mask_b & 0x1) << 6) |
		((pwrctrl->reg_spm_resource_req_rsv0_3_mask_b & 0x1) << 7) |
		((pwrctrl->reg_spm_resource_req_rsv0_2_mask_b & 0x1) << 8) |
		((pwrctrl->reg_spm_resource_req_rsv0_1_mask_b & 0x1) << 9) |
		((pwrctrl->reg_spm_resource_req_rsv0_0_mask_b & 0x1) << 10) |
		((pwrctrl->reg_srcclkeni2_cksel3_mask_b & 0x1) << 11) |
		((pwrctrl->reg_srcclkeni2_cksel2_mask_b & 0x1) << 12) |
		((pwrctrl->reg_srcclkeni1_cksel3_mask_b & 0x1) << 13) |
		((pwrctrl->reg_srcclkeni1_cksel2_mask_b & 0x1) << 14) |
		((pwrctrl->reg_srcclkeni0_cksel3_mask_b & 0x1) << 15) |
		((pwrctrl->reg_srcclkeni0_cksel2_mask_b & 0x1) << 16) |
		((pwrctrl->reg_md_ddr_en_0_dbc_en & 0x1) << 17) |
		((pwrctrl->reg_md_ddr_en_1_dbc_en & 0x1) << 18) |
		((pwrctrl->reg_conn_ddr_en_dbc_en & 0x1) << 19) |
		((pwrctrl->reg_sspm_mask_b & 0x1) << 20) |
		((pwrctrl->reg_md_0_mask_b & 0x1) << 21) |
		((pwrctrl->reg_md_1_mask_b & 0x1) << 22) |
		((pwrctrl->reg_scp_mask_b & 0x1) << 23) |
		((pwrctrl->reg_srcclkeni0_mask_b & 0x1) << 24) |
		((pwrctrl->reg_srcclkeni1_mask_b & 0x1) << 25) |
		((pwrctrl->reg_srcclkeni2_mask_b & 0x1) << 26) |
		((pwrctrl->reg_md_apsrc_1_sel & 0x1) << 27) |
		((pwrctrl->reg_md_apsrc_0_sel & 0x1) << 28) |
		((pwrctrl->reg_conn_mask_b & 0x1) << 29) |
		((pwrctrl->reg_conn_apsrc_sel & 0x1) << 30) |
		((pwrctrl->reg_md_srcclkena_0_vrf18_mask_b & 0x1) << 31));

	/* SPM_WAKEUP_EVENT_MASK */
	mmio_write_32(SPM_WAKEUP_EVENT_MASK,
		((pwrctrl->reg_wakeup_event_mask & 0xffffffff) << 0));

	/* SPM_EXT_WAKEUP_EVENT_MASK */
	mmio_write_32(SPM_EXT_WAKEUP_EVENT_MASK,
		((pwrctrl->reg_ext_wakeup_event_mask & 0xffffffff) << 0));

	/* MCU0_WFI_EN */
	mmio_write_32(MCU0_WFI_EN,
		((pwrctrl->mcu0_wfi_en & 0x1) << 0));

	/* MCU1_WFI_EN */
	mmio_write_32(MCU1_WFI_EN,
		((pwrctrl->mcu1_wfi_en & 0x1) << 0));

	/* MCU2_WFI_EN */
	mmio_write_32(MCU2_WFI_EN,
		((pwrctrl->mcu2_wfi_en & 0x1) << 0));

	/* MCU3_WFI_EN */
	mmio_write_32(MCU3_WFI_EN,
		((pwrctrl->mcu3_wfi_en & 0x1) << 0));

	/* MCU4_WFI_EN */
	mmio_write_32(MCU4_WFI_EN,
		((pwrctrl->mcu4_wfi_en & 0x1) << 0));

	/* MCU5_WFI_EN */
	mmio_write_32(MCU5_WFI_EN,
		((pwrctrl->mcu5_wfi_en & 0x1) << 0));

	/* MCU6_WFI_EN */
	mmio_write_32(MCU6_WFI_EN,
		((pwrctrl->mcu6_wfi_en & 0x1) << 0));

	/* MCU7_WFI_EN */
	mmio_write_32(MCU7_WFI_EN,
		((pwrctrl->mcu7_wfi_en & 0x1) << 0));

	/* MCU8_WFI_EN */
	mmio_write_32(MCU8_WFI_EN,
		((pwrctrl->mcu8_wfi_en & 0x1) << 0));

	/* MCU9_WFI_EN */
	mmio_write_32(MCU9_WFI_EN,
		((pwrctrl->mcu9_wfi_en & 0x1) << 0));

	/* MCU10_WFI_EN */
	mmio_write_32(MCU10_WFI_EN,
		((pwrctrl->mcu10_wfi_en & 0x1) << 0));

	/* MCU11_WFI_EN */
	mmio_write_32(MCU11_WFI_EN,
		((pwrctrl->mcu11_wfi_en & 0x1) << 0));

	/* MCU12_WFI_EN */
	mmio_write_32(MCU12_WFI_EN,
		((pwrctrl->mcu12_wfi_en & 0x1) << 0));

	/* MCU13_WFI_EN */
	mmio_write_32(MCU13_WFI_EN,
		((pwrctrl->mcu13_wfi_en & 0x1) << 0));

	/* MCU14_WFI_EN */
	mmio_write_32(MCU14_WFI_EN,
		((pwrctrl->mcu14_wfi_en & 0x1) << 0));

	/* MCU15_WFI_EN */
	mmio_write_32(MCU15_WFI_EN,
		((pwrctrl->mcu15_wfi_en & 0x1) << 0));

	/* MCU16_WFI_EN */
	mmio_write_32(MCU16_WFI_EN,
		((pwrctrl->mcu16_wfi_en & 0x1) << 0));

	/* MCU17_WFI_EN */
	mmio_write_32(MCU17_WFI_EN,
		((pwrctrl->mcu17_wfi_en & 0x1) << 0));
	/* Auto-gen End */

	/* for gps only case */
	if (spm_for_gps_flag) {
		__uint32_t value;

		INFO("for gps only case\n");
		value = mmio_read_32(SPM_CLK_CON);
		value &= (~(0x1 << 6));
		value &= (~(0x1 << 13));
		value |= (0x1 << 1);
		value &= (~(0x1 << 0));
		mmio_write_32(SPM_CLK_CON, value);

		value = mmio_read_32(SPM_SRC3_MASK);
		value &= (~(0x1 << 25));
		mmio_write_32(SPM_SRC3_MASK, value);

		value = mmio_read_32(SPM_SRC_MASK);
		value &= (~(0x1 << 12));
		mmio_write_32(SPM_SRC_MASK, value);
	}

	mmio_write_32(SPM_SW_RSV_5,
			(mmio_read_32(SPM_SW_RSV_5) & ~(0xf << 8 | 0x3 << 1)) |
			(1 << (pwrctrl->opp_level + 8)));
}

void __spm_set_wakeup_event(const struct pwr_ctrl *pwrctrl)
{
	__uint32_t val, mask, isr;

	if (pwrctrl->timer_val_cust == 0)
		val = pwrctrl->timer_val ? pwrctrl->timer_val : PCM_TIMER_MAX;
	else
		val = pwrctrl->timer_val_cust;

	mmio_write_32(PCM_TIMER_VAL, val);
	mmio_write_32(PCM_CON1, mmio_read_32(PCM_CON1) | SPM_REGWR_CFG_KEY | RG_PCM_TIMER_EN_LSB);

	/* unmask AP wakeup source */
	if (pwrctrl->wake_src_cust == 0)
		mask = pwrctrl->wake_src;
	else
		mask = pwrctrl->wake_src_cust;

	if (pwrctrl->syspwreq_mask)
		mask &= ~WAKE_SRC_R12_CSYSPWRUPREQ_B;
	mmio_write_32(SPM_WAKEUP_EVENT_MASK, ~mask);

	/* unmask SPM ISR (keep TWAM setting) */
	isr = mmio_read_32(SPM_IRQ_MASK) & REG_TWAM_IRQ_MASK_LSB;
	mmio_write_32(SPM_IRQ_MASK, isr | ISRM_RET_IRQ_AUX);
}

void __spm_kick_pcm_to_run(struct pwr_ctrl *pwrctrl)
{
	__uint32_t con0;

	/* init register to match PCM expectation */
	mmio_write_32(SPM_MAS_PAUSE_MASK_B, 0xffffffff);
	mmio_write_32(SPM_MAS_PAUSE2_MASK_B, 0xffffffff);
	mmio_write_32(PCM_REG_DATA_INI, 0);

	/* set PCM flags and data */
	if (pwrctrl->pcm_flags_cust_clr != 0)
		pwrctrl->pcm_flags &= ~pwrctrl->pcm_flags_cust_clr;
	if (pwrctrl->pcm_flags_cust_set != 0)
		pwrctrl->pcm_flags |= pwrctrl->pcm_flags_cust_set;
	if (pwrctrl->pcm_flags1_cust_clr != 0)
		pwrctrl->pcm_flags1 &= ~pwrctrl->pcm_flags1_cust_clr;
	if (pwrctrl->pcm_flags1_cust_set != 0)
		pwrctrl->pcm_flags1 |= pwrctrl->pcm_flags1_cust_set;
	mmio_write_32(SPM_SW_FLAG, pwrctrl->pcm_flags);
	/* cannot modify pcm_flags1[15:12] which is from bootup setting */
	mmio_write_32(SPM_RSV_CON2, (mmio_read_32(SPM_RSV_CON2) & 0xf000) | (pwrctrl->pcm_flags1 & 0xfff));

	/* enable r0 and r7 to control power */
	mmio_write_32(PCM_PWR_IO_EN, PCM_PWRIO_EN_R0 | PCM_PWRIO_EN_R7);

	/* kick PCM to run (only toggle PCM_KICK) */
	con0 = mmio_read_32(PCM_CON0) & ~(RG_IM_KICK_L_LSB | RG_PCM_KICK_L_LSB);
	mmio_write_32(PCM_CON0, con0 | SPM_REGWR_CFG_KEY | PCM_CK_EN_LSB | RG_PCM_KICK_L_LSB);
	mmio_write_32(PCM_CON0, con0 | SPM_REGWR_CFG_KEY | PCM_CK_EN_LSB);
}


void __spm_get_wakeup_status(struct wake_status *wakesta)
{
	/* get PC value if PCM assert (pause abort) */
	wakesta->assert_pc = mmio_read_32(PCM_REG_DATA_INI);

	/* get wakeup event */
	wakesta->r12 = mmio_read_32(SPM_SW_RSV_0);        /* backup of PCM_REG12_DATA */
	wakesta->r12_ext = mmio_read_32(PCM_REG12_EXT_DATA);
	wakesta->raw_sta = mmio_read_32(SPM_WAKEUP_STA);
	wakesta->raw_ext_sta = mmio_read_32(SPM_WAKEUP_EXT_STA);
	wakesta->wake_misc = mmio_read_32(SPM_BSI_D0_SR);   /* backup of SPM_WAKEUP_MISC */

	/* get sleep time */
	wakesta->timer_out = mmio_read_32(SPM_BSI_D1_SR);   /* backup of PCM_TIMER_OUT */

	/* get other SYS and co-clock status */
	wakesta->r13 = mmio_read_32(PCM_REG13_DATA);
	wakesta->idle_sta = mmio_read_32(SUBSYS_IDLE_STA);

	/* get debug flag for PCM execution check */
	wakesta->debug_flag = mmio_read_32(SPM_SW_DEBUG);

	/* get special pattern (0xf0000 or 0x10000) if sleep abort */
	wakesta->event_reg = mmio_read_32(SPM_BSI_D2_SR);   /* PCM_EVENT_REG_STA */

	/* get ISR status */
	wakesta->isr = mmio_read_32(SPM_IRQ_STA);
}

void __spm_clean_after_wakeup(void)
{
	/* [Vcorefs] can not switch back to POWER_ON_VAL0 here,
	   the FW stays in VCORE DVFS which use r0 to Ctrl MEM */
	/* disable r0 and r7 to control power */
	/* mmio_write_32(PCM_PWR_IO_EN, 0); */

	/* clean CPU wakeup event */
	mmio_write_32(SPM_CPU_WAKEUP_EVENT, 0);

	/* [Vcorefs] not disable pcm timer here, due to the
	   following vcore dvfs will use it for latency check */
	/* clean PCM timer event */
	/* mmio_write_32(PCM_CON1, SPM_REGWR_CFG_KEY | (mmio_read_32(PCM_CON1) & ~RG_PCM_TIMER_EN_LSB)); */

	/* clean wakeup event raw status (for edge trigger event) */
	mmio_write_32(SPM_WAKEUP_EVENT_MASK, ~0);

	/* clean ISR status (except TWAM) */
	mmio_write_32(SPM_IRQ_MASK, mmio_read_32(SPM_IRQ_MASK) | ISRM_ALL_EXC_TWAM);
	mmio_write_32(SPM_IRQ_STA, ISRC_ALL_EXC_TWAM);
	mmio_write_32(SPM_SWINT_CLR, PCM_SW_INT_ALL);
}

void __spm_set_pcm_wdt(int en)
{
	/* enable PCM WDT (normal mode) to start count if needed */
	if (en) {
		__uint32_t con1;

		con1 = mmio_read_32(PCM_CON1) & ~(RG_PCM_WDT_WAKE_LSB | RG_PCM_WDT_EN_LSB);
		mmio_write_32(PCM_CON1, SPM_REGWR_CFG_KEY | con1);

		if (mmio_read_32(PCM_TIMER_VAL) > PCM_TIMER_MAX)
			mmio_write_32(PCM_TIMER_VAL, PCM_TIMER_MAX);
		mmio_write_32(PCM_WDT_VAL, mmio_read_32(PCM_TIMER_VAL) + PCM_WDT_TIMEOUT);
		mmio_write_32(PCM_CON1, con1 | SPM_REGWR_CFG_KEY | RG_PCM_WDT_EN_LSB);
	} else {
		mmio_write_32(PCM_CON1, SPM_REGWR_CFG_KEY | (mmio_read_32(PCM_CON1) &
		~RG_PCM_WDT_EN_LSB));
	}

}
