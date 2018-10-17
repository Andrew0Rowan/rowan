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
#include <bakery_lock.h>
#include <debug.h>
#include <delay_timer.h>
#include <mmio.h>
#include <mt2712_def.h>
#include <plat_mt_cirq.h>
#include <plat_mt_gic.h>
#include <spm.h>
#include <spm_suspend.h>

/*
 * System Power Manager (SPM) is a hardware module, which controls cpu or
 * system power for different power scenarios using different firmware.
 * This driver controls the system power in system suspend flow.
 */

#define spm_is_wakesrc_invalid(wakesrc)	\
	(!!((unsigned int)(wakesrc) & 0xfc7f3a9b))

const unsigned int spm_flags = 0;

enum wake_reason_t spm_wake_reason = WR_NONE;

static struct mtk_irq_mask irq_mask;

/*
 * SPM settings for suspend scenario
 */
struct pwr_ctrl spm_suspend_ctrl = {
	.wake_src		= WAKE_SRC_FOR_SUSPEND,
	.r0_ctrl_en		= 1,
	.r7_ctrl_en		= 1,
	.infra_dcm_lock		= 1,
	.wfi_op			= WFI_OP_AND,
	.pcm_apsrc_req = 0,
	.ca7top_idle_mask	= 0,
	.ca15top_idle_mask	= 0,
	.mcusys_idle_mask	= 1,
	.mfg_req_mask		= 0,
	.srclkenai_mask		= 1,
	.ca7_wfi0_en		= 1,
	.ca7_wfi1_en    	= 1,
	.ca7_wfi2_en    	= 1,
	.ca7_wfi3_en    	= 1,
	.ca15_wfi0_en   	= 1,
	.ca15_wfi1_en   	= 1,
	.ca15_wfi2_en   	= 1,
	.ca15_wfi3_en   	= 1,
	.syspwreq_mask		= 1,
};

/*
 * go_to_sleep_before_wfi() - trigger SPM to enter suspend scenario
 */
static unsigned int pcm_original_cmd;

static void go_to_sleep_before_wfi(const unsigned int spm_flags)
{
	struct pwr_ctrl *pwrctrl;
#if 0
	unsigned int capcode;
#endif
	pcm_original_cmd = mmio_read_32(SPM_PCM_RESERVE);

#if PCM_NEED_RELOAD
	spm_reload_pcm_code();
#endif
	mt_irq_mask_all(&irq_mask);
	mt_irq_unmask_for_sleep(SPM_IRQ0_PHY_ID);
	mt_cirq_clone_gic();
	mt_cirq_enable();

	/* TODO: add uart APIs ?*/
#if 0
	if (is_infra_pdn(pwrctrl->pcm_flags))
		mtk_uart_save();
#endif
	mmio_write_32(TOP_DCMCTL, mmio_read_32(TOP_DCMCTL) & ~(1U));
	mmio_write_32(0x10001220, mmio_read_32(0x10001220) | (1U << 12));

#if !PCM_NEED_RELOAD
	if (!is_suspend_ready()) {
#endif
		pwrctrl = &spm_suspend_ctrl;
		set_pwrctrl_pcm_flags(pwrctrl, spm_flags);

#if 0
		/* Read XO cap code */
		capcode = mt_get_current_capid();
		spm_write(SPM_PCM_RESERVE8, capcode);
#endif

		spm_set_sysclk_settle();

		/* TODO: add uart APIs*/

		INFO("sec = %u, wakesrc = 0x%x (%u)(%u)\n",
		     pwrctrl->timer_val, pwrctrl->wake_src,
		     is_cpu_pdn(pwrctrl->pcm_flags),
		     is_infra_pdn(pwrctrl->pcm_flags));
		spm_set_power_control(pwrctrl);
		spm_set_wakeup_event(pwrctrl);
		spm_set_pcmwdt();
		spm_kick_pcm_to_run(pwrctrl);
		set_suspend_ready();
#if !PCM_NEED_RELOAD
	}
#endif
	spm_set_pcm_cmd(PCM_CMD_SUSPEND_PCM);
}

/*
 * go_to_sleep_after_wfi() - get wakeup reason after
 * leaving suspend scenario and clean up SPM settings
 */
static enum wake_reason_t go_to_sleep_after_wfi(void)
{
	struct wake_status wakesta;
	static enum wake_reason_t last_wr = WR_NONE;

	spm_get_wakeup_status(&wakesta);
	mmio_write_32(SPM_PCM_CON1,
		CON1_CFG_KEY | (mmio_read_32(SPM_PCM_CON1) & ~CON1_PCM_WDT_EN));
	spm_clean_after_wakeup();

	/* TODO: add uart APIs*/
#if 0
	if (is_infra_pdn(pwrctrl->pcm_flags))
		mtk_uart_restore();
#endif

	last_wr = spm_output_wake_reason(&wakesta);

	mmio_write_32(TOP_DCMCTL, mmio_read_32(TOP_DCMCTL) | (1U));

	mt_irq_mask_restore(&irq_mask);
	mt_cirq_flush();
	mt_cirq_disable();

	spm_set_pcm_cmd(pcm_original_cmd);

	return last_wr;
}

void spm_system_suspend(void)
{
	spm_lock_get();
	go_to_sleep_before_wfi(spm_flags);
	spm_lock_release();
}

void spm_system_suspend_finish(void)
{
	spm_lock_get();
	spm_wake_reason = go_to_sleep_after_wfi();
	INFO("spm_wake_reason=%d\n", spm_wake_reason);
	spm_lock_release();
}
