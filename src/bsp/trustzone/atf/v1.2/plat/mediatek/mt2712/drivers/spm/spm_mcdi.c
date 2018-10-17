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
#include <spm.h>
#include <spm_hotplug.h>
#include <spm_mcdi.h>
#include <spm_pcm.h>
#include <spm_dpidle.h>
#include <mtspmc.h>

/*
 * System Power Manager (SPM) is a hardware module, which controls cpu or
 * system power for different power scenarios using different firmware.
 * This driver controls the cpu power in cpu idle power saving state.
 */

#define PCM_MCDI_CA35_CPUTOP_PWRCTL    (0x1 << 12)
#define PCM_MCDI_CA72_CPUTOP_PWRCTL    (0x1 << 13)

void spm_mcdi_switch_to_dpidle(void)
{
	spm_set_pcm_cmd(0);
	while ((mmio_read_32(SPM_PCM_REG9_DATA)) != (mmio_read_32(SPM_PCM_RESERVE)))
		;
}

void spm_mcdi_wfi_sel_enter(unsigned long mpidr)
{
	unsigned long linear_id = platform_get_core_pos(mpidr);

	if (linear_id >= 8)	/* pcm_id_from_linear_id: 8 -> 4, 9 -> 5 */
		linear_id -= 4;

#if SPM_ATF_LOG
	spm_trace(LOG_MCDI_OFF | BIT(linear_id));
#endif

	mmio_clrbits_32(SPM_PCM_RESERVE, 1 << linear_id);

	while (mmio_read_32(SPM_PCM_REG4_DATA) != mmio_read_32(SPM_PCM_RESERVE));

	/* SPM WFI Select by core number */
	switch (linear_id) {
	case 0:
		mmio_write_32(SPM_CA7_CPU0_IRQ_MASK, 1);
		mmio_write_32(SPM_SLEEP_CA7_WFI0_EN, 1);
		break;
	case 1:
		mmio_write_32(SPM_CA7_CPU1_IRQ_MASK, 1);
		mmio_write_32(SPM_SLEEP_CA7_WFI1_EN, 1);
		break;
	case 2:
		mmio_write_32(SPM_CA7_CPU2_IRQ_MASK, 1);
		mmio_write_32(SPM_SLEEP_CA7_WFI2_EN, 1);
		break;
	case 3:
		mmio_write_32(SPM_CA7_CPU3_IRQ_MASK, 1);
		mmio_write_32(SPM_SLEEP_CA7_WFI3_EN, 1);
		break;
	case 4:
		mmio_write_32(SPM_CA15_CPU0_IRQ_MASK, 1);
		mmio_write_32(SPM_SLEEP_CA15_WFI0_EN, 1);
		break;
	case 5:
		mmio_write_32(SPM_CA15_CPU1_IRQ_MASK, 1);
		mmio_write_32(SPM_SLEEP_CA15_WFI1_EN, 1);
		break;
	default:
		break;
	}
}

void spm_mcdi_wfi_sel_leave(unsigned long mpidr)
{
	unsigned long linear_id = platform_get_core_pos(mpidr);

	if (linear_id >= 8)	/* pcm_id_from_linear_id: 8 -> 4, 9 -> 5 */
		linear_id -= 4;

#if SPM_ATF_LOG
	spm_trace(LOG_MCDI_ON | BIT(linear_id));
#endif

	mmio_setbits_32(SPM_PCM_RESERVE, 1 << linear_id);

	while (mmio_read_32(SPM_PCM_REG4_DATA) != mmio_read_32(SPM_PCM_RESERVE));

	/* SPM WFI Select by core number */
	switch (linear_id) {
	case 0:
		mmio_write_32(SPM_SLEEP_CA7_WFI0_EN, 0);
		mmio_write_32(SPM_CA7_CPU0_IRQ_MASK, 0);
		break;
	case 1:
		mmio_write_32(SPM_SLEEP_CA7_WFI1_EN, 0);
		mmio_write_32(SPM_CA7_CPU1_IRQ_MASK, 0);
		break;
	case 2:
		mmio_write_32(SPM_SLEEP_CA7_WFI2_EN, 0);
		mmio_write_32(SPM_CA7_CPU2_IRQ_MASK, 0);
		break;
	case 3:
		mmio_write_32(SPM_SLEEP_CA7_WFI3_EN, 0);
		mmio_write_32(SPM_CA7_CPU3_IRQ_MASK, 0);
		break;
	case 4:
		mmio_write_32(SPM_CA15_CPU0_IRQ_MASK, 0);
		mmio_write_32(SPM_SLEEP_CA15_WFI0_EN, 0);
		break;
	case 5:
		mmio_write_32(SPM_CA15_CPU1_IRQ_MASK, 0);
		mmio_write_32(SPM_SLEEP_CA15_WFI1_EN, 0);
		break;
	default:
		break;
	}
}

static void spm_mcdi_set_cputop_pwrctrl_for_cluster_off(unsigned long mpidr)
{
	unsigned long cpu_id = mpidr & MPIDR_CPU_MASK;
	unsigned long cluster_id = mpidr & MPIDR_CLUSTER_MASK;
	unsigned int pwr_status, shift, i, flag = 0;

	if (cluster_id) {
		int cpu4_on = mmio_read_32(SPMC_MP0_CPU4_PWR_CON) & BIT(16);
		int cpu5_on = mmio_read_32(SPMC_MP0_CPU5_PWR_CON) & BIT(16);

#if SPM_ATF_LOG
		spm_trace(LOG_MCDI_C2I | LOG_CLUSTER1);
#endif
		if (!(cpu_id == 0 && cpu5_on) && !(cpu_id == 1 && cpu4_on)) {
#if SPM_ATF_LOG
			spm_trace(LOG_MCDI_OFF | LOG_CLUSTER1);
#endif
			mmio_clrbits_32(SPM_PCM_RESERVE,
					PCM_MCDI_CA72_CPUTOP_PWRCTL);
			while (mmio_read_32(SPM_PCM_REG4_DATA) != mmio_read_32(SPM_PCM_RESERVE));
		}
	} else {
		pwr_status = mmio_read_32(SPM_PWR_STATUS) |
					 mmio_read_32(SPM_PWR_STATUS_2ND);

		for (i = 0; i < PLATFORM_CLUSTER0_CORE_COUNT; i++) {
			if (i == cpu_id)
				continue;

			shift = SPM_CA35_CPU_PWRSTA_SHIFT + i;
			flag |= (pwr_status & (1 << shift)) >> shift;
		}

#if SPM_ATF_LOG
		spm_trace(LOG_MCDI_C2I | LOG_CLUSTER0);
#endif
		if (!flag) {
#if SPM_ATF_LOG
			spm_trace(LOG_MCDI_OFF | LOG_CLUSTER0);
#endif
			mmio_clrbits_32(SPM_PCM_RESERVE,
					PCM_MCDI_CA35_CPUTOP_PWRCTL);
			while (mmio_read_32(SPM_PCM_REG4_DATA) != mmio_read_32(SPM_PCM_RESERVE));
		}
	}
}

static void spm_mcdi_clear_cputop_pwrctrl_for_cluster_on(unsigned long mpidr)
{
	unsigned long cluster_id = mpidr & MPIDR_CLUSTER_MASK;

	if (cluster_id) {
#if SPM_ATF_LOG
		spm_trace(LOG_MCDI_ON | LOG_CLUSTER1);
#endif
		mmio_setbits_32(SPM_PCM_RESERVE, PCM_MCDI_CA72_CPUTOP_PWRCTL);
		while (mmio_read_32(SPM_PCM_REG4_DATA) != mmio_read_32(SPM_PCM_RESERVE));
	}
	else {
#if SPM_ATF_LOG
		spm_trace(LOG_MCDI_ON | LOG_CLUSTER0);
#endif
		mmio_setbits_32(SPM_PCM_RESERVE, PCM_MCDI_CA35_CPUTOP_PWRCTL);
		while (mmio_read_32(SPM_PCM_REG4_DATA) != mmio_read_32(SPM_PCM_RESERVE));
	}
}

void spm_mcdi_prepare_back_from_dpidle(void)
{
	struct pwr_ctrl *pwrctrl = spm_hotplug_mcdi.pwrctrl;

	set_pwrctrl_pcm_flags(pwrctrl, 0);
	spm_set_power_control(pwrctrl);
	spm_set_wakeup_event(pwrctrl);
	spm_kick_pcm_to_run(pwrctrl);
	set_hotplug_mcdi_ready();
	spm_set_pcm_cmd(PCM_CMD_HP_MCDI_PCM);
}

void spm_mcdi_prepare_for_off_state(unsigned long mpidr, unsigned int afflvl)
{
	struct pwr_ctrl *pwrctrl = spm_hotplug_mcdi.pwrctrl;

	spm_lock_get();

	if (afflvl == MPIDR_AFFLVL1) {
#if !WORKAROUND_MCDI_DPIDLE
		spm_dpidle_prepare(mpidr);
		if (!is_dpidle_ready())
			spm_sodi_prepare(mpidr);
#endif
	}

	if (!is_sodi_ready() && !is_dpidle_ready()) {
		if (!is_hotplug_mcdi_ready()) {
			set_pwrctrl_pcm_flags(pwrctrl, 0);
			spm_set_power_control(pwrctrl);
			spm_set_wakeup_event(pwrctrl);
			spm_kick_pcm_to_run(pwrctrl);
			set_hotplug_mcdi_ready();
			spm_set_pcm_cmd(PCM_CMD_HP_MCDI_PCM);
		}
		spm_mcdi_wfi_sel_enter(mpidr);
		if (afflvl == MPIDR_AFFLVL1)
			spm_mcdi_set_cputop_pwrctrl_for_cluster_off(mpidr);
	}

	spm_lock_release();
}

void spm_mcdi_finish_for_on_state(unsigned long mpidr, unsigned int afflvl)
{
	unsigned long linear_id = platform_get_core_pos(mpidr);

	spm_lock_get();

#if !WORKAROUND_MCDI_DPIDLE
	if (is_dpidle_ready())
		spm_dpidle_finish(mpidr);
	else if (is_sodi_ready())
		spm_sodi_finish(mpidr);
#endif

#if SPM_ATF_LOG
	if (afflvl == MPIDR_AFFLVL1) {
		unsigned long cluster_id = mpidr & MPIDR_CLUSTER_MASK;

		if (cluster_id)
			spm_trace(LOG_MCDI_C2O | LOG_CLUSTER1);
		else
			spm_trace(LOG_MCDI_C2O | LOG_CLUSTER0);
	}
#endif
	spm_mcdi_clear_cputop_pwrctrl_for_cluster_on(mpidr);
	spm_mcdi_wfi_sel_leave(mpidr);
	mmio_write_32(SPM_PCM_SW_INT_CLEAR, (0x1 << linear_id));

	spm_lock_release();
}
