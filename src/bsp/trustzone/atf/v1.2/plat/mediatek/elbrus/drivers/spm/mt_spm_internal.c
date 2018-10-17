#include <debug.h>
#include <delay_timer.h>
#include <mmio.h>
#include <mt_spm.h>
#include <mt_spm_internal.h>
#include <mt_spm_reg.h>
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

	if (wakesta->r12 & WAKE_SRC_R12_PCM_TIMER) {
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
	mmio_write_32(COMMON_TOP_PWR_ADDR, 0x108C0210);
	mmio_write_32(COMMON_CPU_PWR_ADDR, 0x108C0220);
	mmio_write_32(SPM_SW_NONSERSV_0, 1 << cpu);
	mmio_write_32(SPM_SW_NONSERSV_1, 1 << (cpu + 12));
	if (cpu >= 0 && cpu < 4) {
		mmio_write_32(ARMPLL_CLK_CON,
				(mmio_read_32(ARMPLL_CLK_CON) & ~(MUXSEL_SC_ARMPLL2_LSB | MUXSEL_SC_ARMPLL3_LSB)) |
				(MUXSEL_SC_CCIPLL_LSB | MUXSEL_SC_ARMPLL1_LSB));
	} else if (cpu >= 4 && cpu < 8) {
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

	/* backup mem control from r0 to POWER_ON_VAL0 */
	if (mmio_read_32(SPM_POWER_ON_VAL0) != mmio_read_32(PCM_REG0_DATA)) {
		VERBOSE("VAL0 from 0x%x to 0x%x\n", mmio_read_32(SPM_POWER_ON_VAL0), mmio_read_32(PCM_REG0_DATA));
		mmio_write_32(SPM_POWER_ON_VAL0, mmio_read_32(PCM_REG0_DATA));
	}

	/* disable r0 and r7 to control power */
	mmio_write_32(PCM_PWR_IO_EN, 0);

	/* disable pcm timer after leaving FW */
	mmio_write_32(PCM_CON1, SPM_REGWR_CFG_KEY | (mmio_read_32(PCM_CON1) & ~RG_PCM_TIMER_EN_LSB));
}

void __spm_reset_and_init_pcm(const struct pcm_desc *pcmdesc)
{
	__uint32_t con1;

	/* SPM code swapping */
	if (mmio_read_32(PCM_REG1_DATA) == 0x1)
		spm_code_swapping();

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
			RG_AHBMIF_APBEN_LSB | RG_MD32_APB_INTERNAL_EN_LSB);
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

void __spm_set_power_control(const struct pwr_ctrl *pwrctrl)
{
	/* Auto-gen Start */

	/* SPM_CLK_CON */
	mmio_write_32(SPM_CLK_CON, pwrctrl->spm_clk_con.value);

	/* SPM_AP_STANDBY_CON */
	mmio_write_32(SPM_AP_STANDBY_CON, pwrctrl->spm_ap_standby_con.value);

	/* SPM_SRC_REQ */
	mmio_write_32(SPM_SRC_REQ, pwrctrl->spm_src_req.value);

	/* SPM_SRC_MASK */
	mmio_write_32(SPM_SRC_MASK, pwrctrl->spm_src_mask.value);

	/* SPM_SRC2_MASK */
	mmio_write_32(SPM_SRC2_MASK, pwrctrl->spm_src2_mask.value);

	/* SPM_WAKEUP_EVENT_MASK */
	mmio_write_32(SPM_WAKEUP_EVENT_MASK,
		((pwrctrl->reg_wakeup_event_mask & 0xffffffff) << 0));

	/* SPM_EXT_WAKEUP_EVENT_MASK */
	mmio_write_32(SPM_EXT_WAKEUP_EVENT_MASK,
		((pwrctrl->reg_ext_wakeup_event_mask & 0xffffffff) << 0));

	/* MP0_CPU0_WFI_EN */
	mmio_write_32(MP0_CPU0_WFI_EN,
		((pwrctrl->mp0_cpu0_wfi_en & 0x1) << 0));

	/* MP0_CPU1_WFI_EN */
	mmio_write_32(MP0_CPU1_WFI_EN,
		((pwrctrl->mp0_cpu1_wfi_en & 0x1) << 0));

	/* MP0_CPU2_WFI_EN */
	mmio_write_32(MP0_CPU2_WFI_EN,
		((pwrctrl->mp0_cpu2_wfi_en & 0x1) << 0));

	/* MP0_CPU3_WFI_EN */
	mmio_write_32(MP0_CPU3_WFI_EN,
		((pwrctrl->mp0_cpu3_wfi_en & 0x1) << 0));

	/* MP1_CPU0_WFI_EN */
	mmio_write_32(MP1_CPU0_WFI_EN,
		((pwrctrl->mp1_cpu0_wfi_en & 0x1) << 0));

	/* MP1_CPU1_WFI_EN */
	mmio_write_32(MP1_CPU1_WFI_EN,
		((pwrctrl->mp1_cpu1_wfi_en & 0x1) << 0));

	/* MP1_CPU2_WFI_EN */
	mmio_write_32(MP1_CPU2_WFI_EN,
		((pwrctrl->mp1_cpu2_wfi_en & 0x1) << 0));

	/* MP1_CPU3_WFI_EN */
	mmio_write_32(MP1_CPU3_WFI_EN,
		((pwrctrl->mp1_cpu3_wfi_en & 0x1) << 0));

	/* DEBUG0_WFI_EN */
	mmio_write_32(DEBUG0_WFI_EN,
		((pwrctrl->debug0_wfi_en & 0x1) << 0));

	/* DEBUG1_WFI_EN */
	mmio_write_32(DEBUG1_WFI_EN,
		((pwrctrl->debug1_wfi_en & 0x1) << 0));

	/* DEBUG2_WFI_EN */
	mmio_write_32(DEBUG2_WFI_EN,
		((pwrctrl->debug2_wfi_en & 0x1) << 0));

	/* DEBUG3_WFI_EN */
	mmio_write_32(DEBUG3_WFI_EN,
		((pwrctrl->debug3_wfi_en & 0x1) << 0));
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

		value = mmio_read_32(SPM_AP_STANDBY_CON);
		value &= (~(0x1 << 24));
		mmio_write_32(SPM_AP_STANDBY_CON, value);

		value = mmio_read_32(SPM_SRC_MASK);
		value &= (~(0x1 << 12));
		mmio_write_32(SPM_SRC_MASK, value);
	}
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
		mask &= ~WAKE_SRC_R12_CSYSPWREQ_B;
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
	mmio_write_32(SPM_SW_FLAG, pwrctrl->pcm_flags);
	mmio_write_32(SPM_SW_RSV_0, pwrctrl->pcm_reserve);

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
