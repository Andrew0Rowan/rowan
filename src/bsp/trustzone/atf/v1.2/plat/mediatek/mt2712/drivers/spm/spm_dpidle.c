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
#include <debug.h>
#include <mmio.h>
#include <mt2712_def.h>
#include <platform.h>
#include <platform_def.h>
#include <plat_mt_gic.h>
#include <scu.h>
#include <spm.h>
#include <spm_hotplug.h>
#include <spm_mcdi.h>
#include <string.h>

#define PCM_CORE0_AWAKE	0xe
#define PCM_CORE1_AWAKE	0xd
#define PCM_CORE2_AWAKE	0xb
#define PCM_CORE3_AWAKE	0x7

#define WAKE_SRC_FOR_DPIDLE	0

static struct pwr_ctrl dpidle_ctrl = {
	.wake_src = WAKE_SRC_FOR_DPIDLE,
	.r0_ctrl_en = 1,
	.r7_ctrl_en = 1,
	.infra_dcm_lock = 1,
	.wfi_op = WFI_OP_AND,
	.ca15_wfi0_en = 1,
	.ca15_wfi1_en = 1,
	.ca15_wfi2_en = 1,
	.ca15_wfi3_en = 1,
	.ca7_wfi0_en = 1,
	.ca7_wfi1_en = 1,
	.ca7_wfi2_en = 1,
	.ca7_wfi3_en = 1,
	.disp0_req_mask = 1,
	.disp1_req_mask = 1,
	.mfg_req_mask = 1,
	.syspwreq_mask = 1,
};

static unsigned int dpidle_condition_mask[NR_CG_GRPS] = {
	0x00000037, /* CG_CTRL0: */
	0x8089B2FC, /* CG_CTRL1: */
	0x017F7F16, /* CG_CTRL2: */
	0x000430FD, /* CG_CTRL8: */
	0x000F0203, /* CG_MMSYS0: */
	0x003FC03C, /* CG_MMSYS1: */
	0x000003E1, /* CG_IMGSYS: */
	0x00000004, /* CG_MFGSYS: */
	0x00000000, /* CG_AUDIO: */
	0x00000001, /* CG_VDEC0: */
	0x00000001, /* CG_VDEC1: */
};
static unsigned int dpidle_block_mask[NR_CG_GRPS];

extern const char *reason_name[NR_REASONS];
static unsigned long dpidle_block_cnt[PLATFORM_CORE_COUNT][NR_REASONS] __section("tzfw_coherent_mem");
static unsigned int dpidle_bypass_cnd;
void spm_dump_dpidle_state(void)
{
	static const char *d = "/sys/kernel/debug/cpuidle/dpidle_state";
	int i,j;

	tf_printf("*********** deep idle state ************\n");

	tf_printf("dpidle_bypass_cnd=%u\n", dpidle_bypass_cnd);

	for (i = 0; i < PLATFORM_CORE_COUNT; i++) {
		tf_printf("CPU%d:\n", i);
		for (j = 0; j < NR_REASONS; j++)
			tf_printf("[%d]dpidle_block_cnt[%s]=%lu\n",
				j, reason_name[j], dpidle_block_cnt[i][j]);
	}

	spm_dump_clock_masks(dpidle_condition_mask, dpidle_block_mask);

	tf_printf("*********** dpidle command help  ************\n");
	tf_printf("dpidle help:   cat %s\n", d);
	tf_printf("switch on/off: echo [dpidle] 1/0 > %s\n", d);
	tf_printf("en_dp_by_bit:  echo enable id > %s\n", d);
	tf_printf("dis_dp_by_bit: echo disable id > %s\n", d);
	tf_printf("bypass clk: echo bypass 1/0 > %s\n", d);
}

static void enable_dpidle_by_mask(int grp, unsigned int mask)
{
	dpidle_condition_mask[grp] &= ~mask;
}

static void disable_dpidle_by_mask(int grp, unsigned int mask)
{
	dpidle_condition_mask[grp] |= mask;
}

static void enable_dpidle_by_bit(int id)
{
	int grp = id / 32;
	unsigned int mask = BIT(id % 32);

	INVALID_GRP_ID(grp);
	enable_dpidle_by_mask(grp, mask);
}

static void disable_dpidle_by_bit(int id)
{
	int grp = id / 32;
	unsigned int mask = BIT(id % 32);

	INVALID_GRP_ID(grp);
	disable_dpidle_by_mask(grp, mask);
}

void spm_modify_dpidle_state(unsigned int cmd, unsigned int val)
{
	if (cmd == CMD_TYPE_ENABLE_MASK_BIT)
		enable_dpidle_by_bit(val);
	else if (cmd == CMD_TYPE_DISABLE_MASK_BIT)
		disable_dpidle_by_bit(val);
	else if (cmd == CMD_TYPE_BYPASS)
		dpidle_bypass_cnd = val;
}

static struct mtk_irq_mask irq_mask;
static unsigned int enter_cpu;
extern unsigned int idle_switch[NR_IDLE_TYPES];
extern unsigned long dpidle_cnt[PLATFORM_CORE_COUNT];
void spm_dpidle_prepare(unsigned long mpidr)
{
	struct pwr_ctrl *pwrctrl = &dpidle_ctrl;
	unsigned int reg5;
	int reason = NR_REASONS;

	if (!idle_switch[IDLE_TYPE_DP]) {
		reason = BY_SWT;
		goto fail;
	}

	if (platform_get_core_pos(mpidr) != 0) {
		reason = BY_CPU;
		goto fail;
	}

	if (!spm_is_last_online_cpu(mpidr)) {
		reason = BY_CPU;
		goto fail;
	}

	memset(dpidle_block_mask, 0, NR_CG_GRPS * sizeof(unsigned int));
	if (!spm_is_pass_clock_check(dpidle_condition_mask, dpidle_block_mask)) {
		if (!(dpidle_bypass_cnd & 0x1)) {
			reason = BY_CLK;
			goto fail;
		}
	}

	if (is_hotplug_mcdi_ready()) {
		spm_mcdi_switch_to_dpidle();
	} else {
		reason = BY_SCN;
		goto fail;
	}

	/* error handling when other CPU is awake during offload process */
	reg5 = mmio_read_32(SPM_PCM_REG5_DATA) & 0xF;
	if (!(reg5 == PCM_CORE0_AWAKE || reg5 == PCM_CORE1_AWAKE ||
		  reg5 == PCM_CORE2_AWAKE || reg5 == PCM_CORE3_AWAKE)) {
		spm_set_pcm_cmd(PCM_CMD_HP_MCDI_PCM);
		reason = BY_OFL;
		goto fail;
	}

	spm_set_pmic_phase(PMIC_WRAP_PHASE_DEEPIDLE);

	mt_irq_mask_all(&irq_mask);
	mt_irq_unmask_for_sleep(SPM_IRQ0_PHY_ID);

	pwrctrl->pcm_flags = PDEF_DISABLE_26M;
	spm_set_power_control(pwrctrl);
	spm_set_wakeup_event(pwrctrl);
	spm_kick_pcm_to_run(pwrctrl);
	set_dpidle_ready();

	spm_mcdi_wfi_sel_enter(mpidr);
	disable_scu(mpidr);
	spm_set_pcm_cmd(PCM_CMD_SUSPEND_PCM);

fail:
	enter_cpu = platform_get_core_pos(mpidr);
	if (reason < NR_REASONS)
		dpidle_block_cnt[enter_cpu][reason]++;

	return;
}

void spm_dpidle_finish(unsigned long mpidr)
{
	struct wake_status wakesta;
	enum wake_reason_t dpidle_wake_reason = WR_NONE;

	spm_get_wakeup_status(&wakesta);
	spm_clean_after_wakeup();
	dpidle_wake_reason = spm_output_wake_reason(&wakesta);
	INFO("dpidle_wake_reason=%d\n", dpidle_wake_reason);

	mt_irq_mask_restore(&irq_mask);

	enable_scu(mpidr);
	spm_mcdi_wfi_sel_leave(mpidr);
	spm_mcdi_prepare_back_from_dpidle();

	dpidle_cnt[enter_cpu]++;
}
