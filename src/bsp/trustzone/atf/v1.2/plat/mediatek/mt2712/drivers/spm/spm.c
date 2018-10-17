/*
 * Copyright (c) 2015, ARM Limited and Contributors. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.
 *
 * Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * Neither the name of ARM nor the names of its contributors may be used
 * to endorse or promote products derived from this software without specific
 * prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */
#include <arch.h>
#include <bakery_lock.h>
#include <debug.h>
#include <mmio.h>
#include <mt2712_def.h>
#include <mtspmc.h>
#include <platform_emi.h>
#include <pmic_wrap_init.h>
#include <spm.h>
#include <spm_dpidle.h>
#include <spm_pcm.h>
#include <spm_hotplug.h>
#include <spm_suspend.h>

/*
 * System Power Manager (SPM) is a hardware module, which controls cpu or
 * system power for different power scenarios using different firmware, i.e.,
 * - spm_hotplug.c for cpu power control in cpu hotplug flow.
 * - spm_mcdi.c for cpu power control in cpu idle power saving state.
 * - spm_suspend.c for system power control in system suspend scenario.
 *
 * This file provide utility functions common to hotplug, mcdi(idle), suspend
 * power scenarios. A bakery lock (software lock) is incoporated to protect
 * certain critical sections to avoid kicking different SPM firmware
 * concurrently.
 */

#define SPM_SYSCLK_SETTLE       128	/* 3.9ms */

DEFINE_BAKERY_LOCK(spm_lock);

static int spm_hotplug_mcdi_ready __section("tzfw_coherent_mem");
static int spm_sodi_ready __section("tzfw_coherent_mem");
static int spm_dpidle_ready __section("tzfw_coherent_mem");
static int spm_suspend_ready __section("tzfw_coherent_mem");

void spm_lock_init(void)
{
	bakery_lock_init(&spm_lock);
}

void spm_lock_get(void)
{
	bakery_lock_get(&spm_lock);
}

void spm_lock_release(void)
{
	bakery_lock_release(&spm_lock);
}

int is_hotplug_mcdi_ready(void)
{
	return spm_hotplug_mcdi_ready;
}

int is_sodi_ready(void)
{
	return spm_sodi_ready;
}

int is_dpidle_ready(void)
{
	return spm_dpidle_ready;
}

int is_suspend_ready(void)
{
	return spm_suspend_ready;
}

void set_hotplug_mcdi_ready(void)
{
	spm_hotplug_mcdi_ready = 1;
	spm_sodi_ready = 0;
	spm_dpidle_ready = 0;
	spm_suspend_ready = 0;
}

void set_suspend_ready(void)
{
	spm_hotplug_mcdi_ready = 0;
	spm_sodi_ready = 0;
	spm_dpidle_ready = 0;
	spm_suspend_ready = 1;
}

void set_sodi_ready(void)
{
	spm_hotplug_mcdi_ready = 0;
	spm_sodi_ready = 1;
	spm_dpidle_ready = 0;
	spm_suspend_ready = 0;
}

void set_dpidle_ready(void)
{
	spm_hotplug_mcdi_ready = 0;
	spm_sodi_ready = 0;
	spm_dpidle_ready = 1;
	spm_suspend_ready = 0;
}

/* clock API */
#define TOPCKGEN_REG(ofs)	(TOPCKGEN_BASE + ofs)
#define AUDIO_REG(ofs)		(AUDIOSYS_BASE + ofs)
#define MM_REG(ofs)			(MMSYS_BASE + ofs)

#define CLK_GATING_CTRL0	TOPCKGEN_REG(0x20)
#define CLK_GATING_CTRL1	TOPCKGEN_REG(0x24)
#define CLK_GATING_CTRL2	TOPCKGEN_REG(0x3c)
#define CLK_GATING_CTRL8	TOPCKGEN_REG(0x70)
#define AUDIO_TOP_CON0		AUDIO_REG(0x00)
#define MMSYS_CG_CON0		MM_REG(0x100)
#define MMSYS_CG_CON1		MM_REG(0x110)

#define PWR_STA_CONN		BIT(1)
#define PWR_STA_DIS			BIT(3)
#define PWR_STA_MFG_3D		BIT(4)
#define PWR_STA_ISP			BIT(5)
#define PWR_STA_VDE			BIT(7)
#define PWR_STA_MFG_2D		BIT(24)
#define PWR_STA_MFG_ASYNC	BIT(25)

enum subsys_id {
	SYS_CONN,
	SYS_DIS,
	SYS_MFG_3D,
	SYS_ISP,
	SYS_VDE,
	SYS_MFG_2D,
	SYS_MFG_ASYNC,
	NR_SYSS,
};

static int sys_is_on(enum subsys_id id)
{
	unsigned int pwr_sta_mask[] = {
		PWR_STA_CONN,
		PWR_STA_DIS,
		PWR_STA_MFG_3D,
		PWR_STA_ISP,
		PWR_STA_VDE,
		PWR_STA_MFG_2D,
		PWR_STA_MFG_ASYNC,
	};

	unsigned int mask, sta, sta_s;

	mask = pwr_sta_mask[id];
	sta = mmio_read_32(SPM_PWR_STATUS);
	sta_s = mmio_read_32(SPM_PWR_STATUS_2ND);

	return (sta & mask) && (sta_s & mask);
}

static void get_all_clock_state(unsigned int clks[NR_CG_GRPS])
{
	int i;

	for (i = 0; i < NR_CG_GRPS; i++)
		clks[i] = 0;

	clks[CG_CTRL0] = ~mmio_read_32(CLK_GATING_CTRL0) ^ BIT(4);
	clks[CG_CTRL1] = ~mmio_read_32(CLK_GATING_CTRL1);
	clks[CG_CTRL2] = ~mmio_read_32(CLK_GATING_CTRL2);
	clks[CG_CTRL8] = ~mmio_read_32(CLK_GATING_CTRL8);

	if (sys_is_on(SYS_DIS)) {
		clks[CG_MMSYS0] = ~mmio_read_32(MMSYS_CG_CON0);
		clks[CG_MMSYS1] = ~mmio_read_32(MMSYS_CG_CON1);
	}

	clks[CG_AUDIO] = ~mmio_read_32(AUDIO_TOP_CON0);

	if (sys_is_on(SYS_ISP))
		clks[CG_IMGSYS] = 0xffffffff;

	if (sys_is_on(SYS_MFG_ASYNC) && sys_is_on(SYS_MFG_2D) &&
			sys_is_on(SYS_MFG_3D))
		clks[CG_MFGSYS] = 0xffffffff;

	if (sys_is_on(SYS_VDE)) {
		clks[CG_VDEC0] = 0xffffffff;
		clks[CG_VDEC1] = 0xffffffff;
	}
}

unsigned int spm_is_pass_clock_check(
		unsigned int *condition, unsigned int *block)
{
	int i;
	unsigned int clks[NR_CG_GRPS];
	unsigned int r = 0;

	get_all_clock_state(clks);

	for (i = 0; i < NR_CG_GRPS; i++) {
		block[i] = condition[i] & clks[i];
		r |= block[i];
	}

	return r == 0;
}

/*
static const char *grp_get_name(int id)
{
	static const char * const grp_name[] = {
		"CG_CTRL0",
		"CG_CTRL1",
		"CG_CTRL2",
		"CG_CTRL8",
		"CG_MMSYS0",
		"CG_MMSYS1",
		"CG_IMGSYS",
		"CG_MFGSYS",
		"CG_AUDIO",
		"CG_VDEC0",
		"CG_VDEC1",
	};

	return grp_name[id];
}
*/

void spm_dump_clock_masks(
		unsigned int *condition, unsigned int *block)
{
/*
	int i;
	for (i = 0; i < NR_CG_GRPS; i++)
		tf_printf("[%02d]condition_mask[%s]=0x%08x\t\tblock_mask[%s]=0x%08x\n",
			i, grp_get_name(i), condition[i], grp_get_name(i), block[i]);
*/
}
/* end of clock API */

unsigned int spm_is_last_online_cpu(unsigned long mpidr)
{
#if CONFIG_SPMC_MODE == 1
	unsigned long cpu_id = mpidr & MPIDR_CPU_MASK;
	unsigned int i, flag = 0;

	for (i = 0; i < PLATFORM_CLUSTER0_CORE_COUNT; i++) {
		if (i == cpu_id)
			continue;
		flag |= mmio_read_32(SPM_SPMC_MP0_CPU0_PWR_CON + 0x4*i);
	}

	if (!flag)
		return 1;
	else
		return 0;
#else
	unsigned long cpu_id = mpidr & MPIDR_CPU_MASK;
	unsigned int pwr_status, shift, i, flag = 0;

	pwr_status = mmio_read_32(SPM_PWR_STATUS) |
				 mmio_read_32(SPM_PWR_STATUS_2ND);

	for (i = 0; i < PLATFORM_CLUSTER0_CORE_COUNT; i++) {
		if (i == cpu_id)
			continue;
		shift = SPM_CA35_CPU_PWRSTA_SHIFT - i;
		flag |= pwr_status & (1 << shift);
	}

	if (!flag)
		return 1;
	else
		return 0;
#endif
}

/* pwrap API */
enum {
	IDX_DI_VPROC_CA7_NORMAL,		/* 0 */
	IDX_DI_VPROC_CA7_SLEEP,			/* 1 */
	IDX_DI_VSRAM_CA7_FAST_TRSN_EN,	/* 2 */
	IDX_DI_VSRAM_CA7_FAST_TRSN_DIS,	/* 3 */
	IDX_DI_VCORE_NORMAL,			/* 4 */
	IDX_DI_VCORE_SLEEP,				/* 5 */
	IDX_DI_VCORE_PDN_NORMAL,		/* 6 */
	IDX_DI_VCORE_PDN_SLEEP,			/* 7 */
	NR_IDX_DI,
}; /* PMIC_WRAP_PHASE_DEEPIDLE */

enum {
	IDX_SO_VSRAM_CA15L_NORMAL,		/* 0 */
	IDX_SO_VSRAM_CA15L_SLEEP,		/* 1 */
	IDX_SO_VPROC_CA7_NORMAL,		/* 2 */
	IDX_SO_VPROC_CA7_SLEEP,			/* 3 */
	IDX_SO_VCORE_NORMAL,			/* 4 */
	IDX_SO_VCORE_SLEEP,				/* 5 */
	IDX_SO_VSRAM_CA7_FAST_TRSN_EN,	/* 6 */
	IDX_SO_VSRAM_CA7_FAST_TRSN_DIS,	/* 7 */
	NR_IDX_SO,
}; /* PMIC_WRAP_PHASE_SODI */

#define PMIC_WRAP_DVFS_ADR0		(PMIC_WRAP_BASE + 0x0E8)
#define PMIC_WRAP_DVFS_WDATA0	(PMIC_WRAP_BASE + 0x0EC)
#define PMIC_WRAP_DVFS_ADR1		(PMIC_WRAP_BASE + 0x0F0)
#define PMIC_WRAP_DVFS_WDATA1	(PMIC_WRAP_BASE + 0x0F4)
#define PMIC_WRAP_DVFS_ADR2		(PMIC_WRAP_BASE + 0x0F8)
#define PMIC_WRAP_DVFS_WDATA2	(PMIC_WRAP_BASE + 0x0FC)
#define PMIC_WRAP_DVFS_ADR3		(PMIC_WRAP_BASE + 0x100)
#define PMIC_WRAP_DVFS_WDATA3	(PMIC_WRAP_BASE + 0x104)
#define PMIC_WRAP_DVFS_ADR4		(PMIC_WRAP_BASE + 0x108)
#define PMIC_WRAP_DVFS_WDATA4	(PMIC_WRAP_BASE + 0x10C)
#define PMIC_WRAP_DVFS_ADR5		(PMIC_WRAP_BASE + 0x110)
#define PMIC_WRAP_DVFS_WDATA5	(PMIC_WRAP_BASE + 0x114)
#define PMIC_WRAP_DVFS_ADR6		(PMIC_WRAP_BASE + 0x118)
#define PMIC_WRAP_DVFS_WDATA6	(PMIC_WRAP_BASE + 0x11C)
#define PMIC_WRAP_DVFS_ADR7		(PMIC_WRAP_BASE + 0x120)
#define PMIC_WRAP_DVFS_WDATA7	(PMIC_WRAP_BASE + 0x124)

#define NR_PMIC_WRAP_CMD 8

#define PMIC_ADDR_VPROC_VOSEL_ON	0x0220	/* [6:0] */
#define PMIC_ADDR_VCORE_VOSEL_ON	0x0314	/* [6:0] */

struct pmic_wrap_cmd {
	unsigned long cmd_addr;
	unsigned long cmd_wdata;
};

struct pmic_wrap_setting {
	enum pmic_wrap_phase_id phase;
	struct pmic_wrap_cmd addr[NR_PMIC_WRAP_CMD];
	struct {
		struct {
			unsigned long cmd_addr;
			unsigned long cmd_wdata;
		} _[NR_PMIC_WRAP_CMD];
		const int nr_idx;
	} set[NR_PMIC_WRAP_PHASE];
};

#define VOLT_TO_PMIC_VAL(volt)  ((((volt) - 700) * 100 + 625 - 1) / 625)

static struct pmic_wrap_setting pw = {
	.phase = NR_PMIC_WRAP_PHASE,	/* invalid setting for init */

	.addr = {
		 {PMIC_WRAP_DVFS_ADR0, PMIC_WRAP_DVFS_WDATA0,},
		 {PMIC_WRAP_DVFS_ADR1, PMIC_WRAP_DVFS_WDATA1,},
		 {PMIC_WRAP_DVFS_ADR2, PMIC_WRAP_DVFS_WDATA2,},
		 {PMIC_WRAP_DVFS_ADR3, PMIC_WRAP_DVFS_WDATA3,},
		 {PMIC_WRAP_DVFS_ADR4, PMIC_WRAP_DVFS_WDATA4,},
		 {PMIC_WRAP_DVFS_ADR5, PMIC_WRAP_DVFS_WDATA5,},
		 {PMIC_WRAP_DVFS_ADR6, PMIC_WRAP_DVFS_WDATA6,},
		 {PMIC_WRAP_DVFS_ADR7, PMIC_WRAP_DVFS_WDATA7,},
	},

	/* Vproc only, power off: 1, power on: 0 */
	.set[PMIC_WRAP_PHASE_DEEPIDLE] = {
		._[IDX_DI_VPROC_CA7_NORMAL] = {PMIC_ADDR_VPROC_VOSEL_ON,
			VOLT_TO_PMIC_VAL(1150),},
		._[IDX_DI_VPROC_CA7_SLEEP] = {PMIC_ADDR_VPROC_VOSEL_ON,
			VOLT_TO_PMIC_VAL(850),},
		._[IDX_DI_VSRAM_CA7_FAST_TRSN_EN] = {PMIC_ADDR_VCORE_VOSEL_ON,
			VOLT_TO_PMIC_VAL(1125),},
		._[IDX_DI_VSRAM_CA7_FAST_TRSN_DIS] = {PMIC_ADDR_VCORE_VOSEL_ON,
			VOLT_TO_PMIC_VAL(900),},
		._[IDX_DI_VCORE_NORMAL] = {PMIC_ADDR_VPROC_VOSEL_ON,
			VOLT_TO_PMIC_VAL(1000),},
		._[IDX_DI_VCORE_SLEEP] = {PMIC_ADDR_VPROC_VOSEL_ON,
			VOLT_TO_PMIC_VAL(800),},
		._[IDX_DI_VCORE_PDN_NORMAL] = {PMIC_ADDR_VCORE_VOSEL_ON,
			VOLT_TO_PMIC_VAL(1125),},
		._[IDX_DI_VCORE_PDN_SLEEP] = {PMIC_ADDR_VCORE_VOSEL_ON,
			VOLT_TO_PMIC_VAL(900),},
		.nr_idx = NR_IDX_DI,
	},

	/* Vproc only, power off: 1, power on: 0 */
	.set[PMIC_WRAP_PHASE_SODI] = {
		._[IDX_SO_VSRAM_CA15L_NORMAL] = {PMIC_ADDR_VPROC_VOSEL_ON,
			VOLT_TO_PMIC_VAL(1150),},
		._[IDX_SO_VSRAM_CA15L_SLEEP] = {PMIC_ADDR_VPROC_VOSEL_ON,
			VOLT_TO_PMIC_VAL(850),},
		._[IDX_SO_VPROC_CA7_NORMAL] = {PMIC_ADDR_VPROC_VOSEL_ON,
			VOLT_TO_PMIC_VAL(1125),},
		._[IDX_SO_VPROC_CA7_SLEEP] = {PMIC_ADDR_VPROC_VOSEL_ON,
			VOLT_TO_PMIC_VAL(700),},
		._[IDX_SO_VCORE_NORMAL] = {PMIC_ADDR_VPROC_VOSEL_ON,
			VOLT_TO_PMIC_VAL(1000),},
		._[IDX_SO_VCORE_SLEEP] = {PMIC_ADDR_VPROC_VOSEL_ON,
			VOLT_TO_PMIC_VAL(800),},
		._[IDX_SO_VSRAM_CA7_FAST_TRSN_EN] = {PMIC_ADDR_VCORE_VOSEL_ON,
			VOLT_TO_PMIC_VAL(1125),},
		._[IDX_SO_VSRAM_CA7_FAST_TRSN_DIS] = {PMIC_ADDR_VCORE_VOSEL_ON,
			VOLT_TO_PMIC_VAL(1125),},
		.nr_idx = NR_IDX_SO,
	},
};

void spm_set_pmic_phase(enum pmic_wrap_phase_id phase)
{
	int i;
	uint32_t rdata = 0;

	pw.phase = phase;

	/* backup the voltage setting of Vproc */
	if (phase == PMIC_WRAP_PHASE_SODI) {
		pwrap_read(PMIC_ADDR_VPROC_VOSEL_ON, &rdata);
		pw.set[phase]._[IDX_SO_VSRAM_CA15L_NORMAL].cmd_wdata = rdata;
	} else if (phase == PMIC_WRAP_PHASE_DEEPIDLE) {
		pwrap_read(PMIC_ADDR_VPROC_VOSEL_ON, &rdata);
		pw.set[phase]._[IDX_DI_VPROC_CA7_NORMAL].cmd_wdata = rdata;
	}

	for (i = 0; i < pw.set[phase].nr_idx; i++) {
		mmio_write_32(pw.addr[i].cmd_addr, pw.set[phase]._[i].cmd_addr);
		mmio_write_32(pw.addr[i].cmd_wdata, pw.set[phase]._[i].cmd_wdata);
	}
}
/* end of pwrap API */

/* debug API */
const char *reason_name[NR_REASONS] = {
	"by_swt",
	"by_cpu",
	"by_clk",
	"by_scn",
	"by_ofl",
};

unsigned int idle_switch[NR_IDLE_TYPES] = {
	1,	/* dpidle switch */
	1,	/* soidle switch */
};

unsigned long dpidle_cnt[PLATFORM_CORE_COUNT];
unsigned long soidle_cnt[PLATFORM_CORE_COUNT];

static void spm_dump_idle_state(void)
{
	static const char *d = "/sys/kernel/debug/cpuidle";
	int i;

	tf_printf("********** idle state dump **********\n");

	for (i = 0; i < PLATFORM_CORE_COUNT; i++)
		tf_printf("dpidle_cnt[%d]=%lu, soidle_cnt[%d]=%lu\n", i, dpidle_cnt[i], i, soidle_cnt[i]);

	tf_printf("dpidle_switch=%d, soidle_switch=%d\n", idle_switch[IDLE_TYPE_DP], idle_switch[IDLE_TYPE_SO]);

	tf_printf("********** idle command help **********\n");
	tf_printf("status help:   cat %s/idle_state\n", d);
	tf_printf("dpidle help:   cat %s/dpidle_state\n", d);
	tf_printf("soidle help:   cat %s/soidle_state\n", d);
}

void spm_debug_read(unsigned int idle_type)
{
	if (idle_type > NR_IDLE_TYPES)
		return;
	else if (idle_type == IDLE_TYPE_ALL)
		spm_dump_idle_state();
	else if (idle_type == IDLE_TYPE_DP)
		spm_dump_dpidle_state();
}

void spm_debug_write(unsigned int idle_type, unsigned int cmd, unsigned int val)
{
	if (idle_type >= NR_IDLE_TYPES || cmd > NR_CMD_TYPES)
		return;

	if (cmd == CMD_TYPE_SW_SWITCH)
		idle_switch[idle_type] = val;
	else if (idle_type == IDLE_TYPE_DP)
			spm_modify_dpidle_state(cmd, val);
}
/* end of debug API */

void spm_register_init(void)
{
	mmio_write_32(SPM_POWERON_CONFIG_SET, SPM_REGWR_CFG_KEY | SPM_REGWR_EN);

	mmio_write_32(SPM_POWER_ON_VAL0, 0);
	mmio_write_32(SPM_POWER_ON_VAL1, POWER_ON_VAL1_DEF);
	mmio_write_32(SPM_PCM_PWR_IO_EN, 0);

	mmio_write_32(SPM_PCM_CON0, CON0_CFG_KEY | CON0_PCM_SW_RESET);
	mmio_write_32(SPM_PCM_CON0, CON0_CFG_KEY);
	if ((mmio_read_32(SPM_PCM_FSM_STA) & PCM_END_FSM_STA_MASK) != PCM_FSM_STA_DEF)
		WARN("PCM reset failed\n");

	mmio_write_32(SPM_PCM_CON0, CON0_CFG_KEY | CON0_IM_SLEEP_DVS);
	mmio_write_32(SPM_PCM_CON1, CON1_CFG_KEY | CON1_EVENT_LOCK_EN |
		CON1_SPM_SRAM_ISO_B | CON1_SPM_SRAM_SLP_B | CON1_MIF_APBEN);
	mmio_write_32(SPM_PCM_IM_PTR, 0);
	mmio_write_32(SPM_PCM_IM_LEN, 0);

	mmio_setbits_32(SPM_CLK_CON, CC_SRCLKENA_MASK_0);
	/* CC_CLKSQ0_SEL is DONT-CARE in Suspend since PCM_PWR_IO_EN[0]=1 in Suspend */

	mmio_write_32(SPM_PCM_SRC_REQ, 0);
	mmio_write_32(SPM_PCM_PASR_DPD_0, 0);

	/* TODO: check if this means "Set SRCLKENI_MASK=1'b1" */
	mmio_setbits_32(SPM_AP_STANBY_CON, ASC_SRCCLKENI_MASK);

	/* unmask gce_busy_mask (set to 1b1); otherwise, gce (cmd-q) can not notify SPM to exit EMI self-refresh */
	mmio_setbits_32(SPM_PCM_MMDDR_MASK, (1 << 4));

	/* clean ISR status */
	mmio_write_32(SPM_SLEEP_ISR_MASK, ISRM_ALL);
	mmio_write_32(SPM_SLEEP_ISR_STATUS, ISRC_ALL);
	mmio_write_32(SPM_PCM_SW_INT_CLEAR, PCM_SW_INT_ALL);
}

void spm_reset_and_init_pcm(void)
{
	unsigned int con1;
	int i = 0;

	mmio_write_32(SPM_PCM_CON0, CON0_CFG_KEY | CON0_PCM_SW_RESET);
	mmio_write_32(SPM_PCM_CON0, CON0_CFG_KEY);
	while ((mmio_read_32(SPM_PCM_FSM_STA) & PCM_END_FSM_STA_MASK) != PCM_FSM_STA_DEF) {
		i++;
		if (i > 1000) {
			i = 0;
			WARN("PCM reset failed\n");
			break;
		}
	}

	mmio_write_32(SPM_PCM_CON0, CON0_CFG_KEY | CON0_IM_SLEEP_DVS);

	con1 = mmio_read_32(SPM_PCM_CON1) &
		(CON1_PCM_WDT_WAKE_MODE | CON1_PCM_WDT_EN);
	mmio_write_32(SPM_PCM_CON1, con1 | CON1_CFG_KEY | CON1_EVENT_LOCK_EN |
		CON1_SPM_SRAM_ISO_B | CON1_SPM_SRAM_SLP_B |
		CON1_IM_NONRP_EN | CON1_MIF_APBEN);
}

void spm_init_pcm_register(void)
{
	mmio_write_32(SPM_PCM_REG_DATA_INI, mmio_read_32(SPM_POWER_ON_VAL0));
	mmio_write_32(SPM_PCM_PWR_IO_EN, PCM_RF_SYNC_R0);
	mmio_write_32(SPM_PCM_PWR_IO_EN, 0);

	mmio_write_32(SPM_PCM_REG_DATA_INI, mmio_read_32(SPM_POWER_ON_VAL1));
	mmio_write_32(SPM_PCM_PWR_IO_EN, PCM_RF_SYNC_R7);
	mmio_write_32(SPM_PCM_PWR_IO_EN, 0);
}

void spm_set_power_control(const struct pwr_ctrl *pwrctrl)
{
	mmio_write_32(SPM_AP_STANBY_CON, (!pwrctrl->srclkenai_mask << 25) |
		  (!pwrctrl->conn_mask << 23) |
		  (!pwrctrl->mm_ddr_req_mask << 18) |
		  (!pwrctrl->vdec_req_mask << 17) |
		  (!pwrctrl->mfg_req_mask << 16) |
		  (!pwrctrl->disp1_req_mask << 15) |
		  (!pwrctrl->disp0_req_mask << 14) |
		  (!!pwrctrl->mcusys_idle_mask << 7) |
		  (!!pwrctrl->ca15top_idle_mask << 6) |
		  (!!pwrctrl->ca7top_idle_mask << 5) | (!!pwrctrl->wfi_op << 4));
	mmio_write_32(SPM_PCM_SRC_REQ, (!!pwrctrl->pcm_f26m_req << 1) |
		  (!!pwrctrl->pcm_apsrc_req << 0));

	mmio_write_32(SPM_SLEEP_CA7_WFI0_EN, !!pwrctrl->ca7_wfi0_en);
	mmio_write_32(SPM_SLEEP_CA7_WFI1_EN, !!pwrctrl->ca7_wfi1_en);
	mmio_write_32(SPM_SLEEP_CA7_WFI2_EN, !!pwrctrl->ca7_wfi2_en);
	mmio_write_32(SPM_SLEEP_CA7_WFI3_EN, !!pwrctrl->ca7_wfi3_en);
}

void spm_set_pcmwdt(void)
{
	unsigned int con1;

	con1 = mmio_read_32(SPM_PCM_CON1) & ~(CON1_PCM_WDT_WAKE_MODE | CON1_PCM_WDT_EN);
	mmio_write_32(SPM_PCM_CON1, CON1_CFG_KEY | con1);

	if (mmio_read_32(SPM_PCM_TIMER_VAL) > PCM_TIMER_MAX)
			mmio_write_32(SPM_PCM_TIMER_VAL, PCM_TIMER_MAX);
	mmio_write_32(SPM_PCM_WDT_TIMER_VAL, mmio_read_32(SPM_PCM_TIMER_VAL) + PCM_WDT_TIMEOUT);
	mmio_write_32(SPM_PCM_CON1, con1 | CON1_CFG_KEY | CON1_PCM_WDT_EN);
}

void spm_set_wakeup_event(const struct pwr_ctrl *pwrctrl)
{
	unsigned int val, mask;

	if (pwrctrl->timer_val_cust == 0)
		val = pwrctrl->timer_val ? pwrctrl->timer_val : PCM_TIMER_MAX;
	else
		val = pwrctrl->timer_val_cust;

	mmio_write_32(SPM_PCM_TIMER_VAL, val);
	mmio_setbits_32(SPM_PCM_CON1, CON1_CFG_KEY | CON1_PCM_TIMER_EN);

	if (pwrctrl->wake_src_cust == 0)
		mask = pwrctrl->wake_src;
	else
		mask = pwrctrl->wake_src_cust;

	if (pwrctrl->syspwreq_mask)
		mask &= ~WAKE_SRC_SYSPWREQ;

	mmio_write_32(SPM_SLEEP_WAKEUP_EVENT_MASK, ~mask);
	mmio_write_32(SPM_SLEEP_ISR_MASK, 0xfe04);
}

void spm_get_wakeup_status(struct wake_status *wakesta)
{
	wakesta->assert_pc = mmio_read_32(SPM_PCM_REG_DATA_INI);
	wakesta->r12 = mmio_read_32(SPM_PCM_REG12_DATA);
	wakesta->raw_sta = mmio_read_32(SPM_SLEEP_ISR_RAW_STA);
	wakesta->wake_misc = mmio_read_32(SPM_SLEEP_WAKEUP_MISC);
	wakesta->timer_out = mmio_read_32(SPM_PCM_TIMER_OUT);
	wakesta->r13 = mmio_read_32(SPM_PCM_REG13_DATA);
	wakesta->idle_sta = mmio_read_32(SPM_SLEEP_SUBSYS_IDLE_STA);
	wakesta->debug_flag = mmio_read_32(SPM_PCM_REG5_DATA);
	wakesta->event_reg = mmio_read_32(SPM_PCM_EVENT_REG_STA);
	wakesta->isr = mmio_read_32(SPM_SLEEP_ISR_STATUS);
}

void spm_init_event_vector(const struct pcm_desc *pcmdesc)
{
	/* init event vector register */
	mmio_write_32(SPM_PCM_EVENT_VECTOR0, pcmdesc->vec0);
	mmio_write_32(SPM_PCM_EVENT_VECTOR1, pcmdesc->vec1);
	mmio_write_32(SPM_PCM_EVENT_VECTOR2, pcmdesc->vec2);
	mmio_write_32(SPM_PCM_EVENT_VECTOR3, pcmdesc->vec3);
	mmio_write_32(SPM_PCM_EVENT_VECTOR4, pcmdesc->vec4);
	mmio_write_32(SPM_PCM_EVENT_VECTOR5, pcmdesc->vec5);
	mmio_write_32(SPM_PCM_EVENT_VECTOR6, pcmdesc->vec6);
	mmio_write_32(SPM_PCM_EVENT_VECTOR7, pcmdesc->vec7);
	mmio_write_32(SPM_PCM_EVENT_VECTOR8, pcmdesc->vec8);
	mmio_write_32(SPM_PCM_EVENT_VECTOR9, pcmdesc->vec9);
	mmio_write_32(SPM_PCM_EVENT_VECTORA, pcmdesc->vec10);
	mmio_write_32(SPM_PCM_EVENT_VECTORB, pcmdesc->vec11);
	mmio_write_32(SPM_PCM_EVENT_VECTORC, pcmdesc->vec12);
	mmio_write_32(SPM_PCM_EVENT_VECTORD, pcmdesc->vec13);
	mmio_write_32(SPM_PCM_EVENT_VECTORE, pcmdesc->vec14);
	mmio_write_32(SPM_PCM_EVENT_VECTORF, pcmdesc->vec15);

	/* event vector will be enabled by PCM itself */
}

#if	AP_MOVE_PCM_CODE
void cpu_load_pcm_code(const struct pcm_desc *pcmdesc)
{
	int i;
	mmio_write_32(SPM_PCM_CON1, spm_read(SPM_PCM_CON1) | CON1_CFG_KEY | CON1_IM_SLAVE);

	for (i = 0; i < pcmdesc->size; i++) {
		mmio_write_32(SPM_PCM_IM_HOST_RW_PTR, BIT(31) | BIT(30) | i);
		mmio_write_32(SPM_PCM_IM_HOST_RW_DAT, (unsigned int) *(pcmdesc->base + i));
	}
	mmio_write_32(SPM_PCM_IM_HOST_RW_PTR, 0);
}

void check_pcm_code(const struct pcm_desc *pcmdesc)
{
	int i;
	for (i = 0; i < pcmdesc->size; i++) {
		mmio_write_32(SPM_PCM_IM_HOST_RW_PTR, BIT(31) | i);
		if ((mmio_read_32(SPM_PCM_IM_HOST_RW_DAT)) != (unsigned int) *(pcmdesc->base + i))
			cpu_load_pcm_code(pcmdesc);
	}
	mmio_write_32(SPM_PCM_IM_HOST_RW_PTR, 0);
}
#endif

void spm_kick_im_to_fetch(const struct pcm_desc *pcmdesc)
{
	unsigned int ptr = 0, len, con0;

	ptr = (unsigned int)(unsigned long)(pcmdesc->base);
	len = pcmdesc->size - 1;

	INFO("ptr = 0x%x, len = %d\n", ptr, len);

	/* PCM code should not put in 0x00000000 ~ 0x40000000 since this is for SOC REGISTER & SRAM. */
	if (ptr < SOC_REGISTER_ADDRESS)
		ERROR("SPM: Bad memory address is under 0x40000000, ptr = 0x%x!!!!!\n", ptr);

	/* For 4GB mode, DE comment SPM can't move pcm code from 0xC0000000 to 0xFFFFFFFF as limitation for 32bit SPM. */
	if ((ptr >= SPM_MAPPING_LIMIT) && (is_enable_4GB_mode()))
		ERROR("SPM: Can't not access memory address for 4GB mode, ptr = 0x%x!!!!!\n", ptr);

#if	AP_MOVE_PCM_CODE
	INFO("SPM load pcm start!!");
	cpu_load_pcm_code(pcmdesc);
	INFO("SPM check pcm code!!");
	check_pcm_code(pcmdesc);
	INFO("SPM load pcm finish!!");
#else
	if (mmio_read_32(SPM_PCM_IM_PTR) != ptr ||
	    mmio_read_32(SPM_PCM_IM_LEN) != len ||
	    pcmdesc->sess > 2) {

		/* DRAM address + SW offset address for 4GB + HW offset address for 4GB
		   ==> 0x4300bd28 + 0x40000000 + (0x100000000 - 0x40000000) = 0x14300bd28 = 0x4300bd28
		   So SPM would move pcm code from 0x4300bd28 to SRAM for 4GB mode. */
		if(is_enable_4GB_mode())
			mmio_write_32(SPM_PCM_IM_PTR, ptr + OFFSET_ADDRESS_FOR_4GB);
		else
			mmio_write_32(SPM_PCM_IM_PTR, ptr);
		mmio_write_32(SPM_PCM_IM_LEN, len);
	} else {
		mmio_setbits_32(SPM_PCM_CON1, CON1_CFG_KEY | CON1_IM_SLAVE);
	}
#endif
	/* kick IM to fetch (only toggle IM_KICK) */
	con0 = mmio_read_32(SPM_PCM_CON0) & ~(CON0_IM_KICK | CON0_PCM_KICK);
	mmio_write_32(SPM_PCM_CON0, con0 | CON0_CFG_KEY | CON0_IM_KICK);
	mmio_write_32(SPM_PCM_CON0, con0 | CON0_CFG_KEY);
}

void spm_set_sysclk_settle(void)
{
	mmio_write_32(SPM_CLK_SETTLE, SPM_SYSCLK_SETTLE);

	INFO("settle = %u\n", mmio_read_32(SPM_CLK_SETTLE));
}

void spm_kick_pcm_to_run(struct pwr_ctrl *pwrctrl)
{
	mmio_write_32(SPM_PCM_FLAGS, pwrctrl->pcm_flags);

	mmio_clrsetbits_32(SPM_CLK_CON, CC_LOCK_INFRA_DCM,
		(pwrctrl->infra_dcm_lock ? CC_LOCK_INFRA_DCM : 0));
}

void spm_clean_after_wakeup(void)
{
	mmio_write_32(SPM_SLEEP_CPU_WAKEUP_EVENT, 0);
	mmio_clrsetbits_32(SPM_PCM_CON1, CON1_PCM_TIMER_EN, CON1_CFG_KEY);

	mmio_write_32(SPM_SLEEP_WAKEUP_EVENT_MASK, ~0);
	mmio_write_32(SPM_SLEEP_ISR_MASK, 0xFF0C);
	mmio_write_32(SPM_SLEEP_ISR_STATUS, 0xC);
	mmio_write_32(SPM_PCM_SW_INT_CLEAR, 0xFF);
}

enum wake_reason_t spm_output_wake_reason(struct wake_status *wakesta)
{
	enum wake_reason_t wr;
	int i;

	wr = WR_UNKNOWN;

	if (wakesta->assert_pc != 0) {
		ERROR("PCM ASSERT AT %u, r12=0x%x, r13=0x%x, debug_flag=0x%x\n",
		      wakesta->assert_pc, wakesta->r12, wakesta->r13,
		      wakesta->debug_flag);
		return WR_PCM_ASSERT;
	}

	if (wakesta->r12 & WAKE_SRC_SPM_MERGE) {
		if (wakesta->wake_misc & WAKE_MISC_PCM_TIMER)
			wr = WR_PCM_TIMER;
		if (wakesta->wake_misc & WAKE_MISC_CPU_WAKE)
			wr = WR_WAKE_SRC;
	}

	for (i = 1; i < 32; i++) {
		if (wakesta->r12 & (1U << i)) {
			INFO("wake by wakesrc %d\n", i);
			wr = WR_WAKE_SRC;
		}
	}

	if ((wakesta->event_reg & 0x100000) == 0) {
		INFO("pcm sleep abort!\n");
		wr = WR_PCM_ABORT;
	}
	return wr;
}

static void spm_init_pcm_all_in_one(const struct pcm_desc *pcmdesc)
{
	unsigned int con0;

	spm_reset_and_init_pcm();

	spm_kick_im_to_fetch(pcmdesc);

	spm_init_pcm_register();

	spm_init_event_vector(pcmdesc);

	/* init register to match PCM expectation */
	mmio_write_32(SPM_PCM_MAS_PAUSE_MASK, 0xffffffff);
	mmio_write_32(SPM_PCM_REG_DATA_INI, 0);

	/* enable r0 and r7 to control power */
	mmio_write_32(SPM_PCM_PWR_IO_EN, PCM_PWRIO_EN_R0 | PCM_PWRIO_EN_R7);

	/* kick PCM to run (only toggle PCM_KICK) */
	con0 = mmio_read_32(SPM_PCM_CON0) & ~(CON0_IM_KICK | CON0_PCM_KICK);
	mmio_write_32(SPM_PCM_CON0, con0 | CON0_CFG_KEY | CON0_PCM_KICK);
	mmio_write_32(SPM_PCM_CON0, con0 | CON0_CFG_KEY);
}

void spm_set_pcm_cmd(unsigned int cmd)
{
	mmio_clrsetbits_32(SPM_PCM_RESERVE, PCM_CMD_MASK, cmd);
}

void spm_boot_init(void)
{
	/* set spm transaction to secure mode */
	mmio_clrbits_32(0x1000EF00, (0x1 << 2));
	mmio_clrbits_32(0x10207F00, (0x1 << 2));
	mmio_setbits_32(0x1000E500, (0x1 << 9));

	/* Only CPU0 is online during boot, initialize cpu online reserve bit */
	mmio_write_32(SPM_PCM_RESERVE, 0x1001);

	spm_lock_init();
	spm_register_init();

	spm_init_pcm_all_in_one(&all_in_one_pcm);
}

#if PCM_NEED_RELOAD
void spm_reload_pcm_code(void)
{
	spm_init_pcm_all_in_one(&all_in_one_pcm);
}
#endif

#if SPM_CCI_CHECK
void spm_cci_on(unsigned long mpidr)
{
	spm_lock_get();
	if (mpidr & MPIDR_CLUSTER_MASK)
		mmio_clrbits_32(SPM_AP_BSI_REQ, BIT(0));
	else
		mmio_clrbits_32(SPM_AP_BSI_REQ, BIT(1));
	spm_lock_release();
}

void spm_cci_off(unsigned long mpidr)
{
	spm_lock_get();
	if (mpidr & MPIDR_CLUSTER_MASK)
		mmio_setbits_32(SPM_AP_BSI_REQ, BIT(0));
	else
		mmio_setbits_32(SPM_AP_BSI_REQ, BIT(1));
	spm_lock_release();
}
#endif

#if SPM_ATF_LOG

#define MAX_LOG		256

struct spm_atf_log {
	uint32_t msg;
	uint32_t spm_r4, spm_r5, spm_rsv;
};

struct spm_atf_log spm_atf_logs[MAX_LOG] __section("tzfw_coherent_mem");
int spm_log_i __section("tzfw_coherent_mem");

void spm_trace(uint32_t msg)
{
	struct spm_atf_log *p = &spm_atf_logs[spm_log_i];

	if (msg == (LOG_MCDI_ON | LOG_CLUSTER0) ||
	    msg == (LOG_MCDI_ON | LOG_CLUSTER1) ||
	    msg == (LOG_MCDI_OFF | LOG_CLUSTER0) ||
	    msg == (LOG_MCDI_OFF | LOG_CLUSTER1))
		return;

	if (++spm_log_i >= MAX_LOG)
		spm_log_i = 0;

	p->msg = msg;
	p->spm_r4 = mmio_read_32(SPM_PCM_REG4_DATA);
	p->spm_r5 = mmio_read_32(SPM_PCM_REG5_DATA);
	// p->spm_r15 = mmio_read_32(SPM_PCM_REG15_DATA);
	p->spm_rsv = mmio_read_32(SPM_PCM_RESERVE);
}
#endif /* SPM_ATF_LOG */

