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
#include <mmio.h>
#include <mt2712_def.h>
#include <platform.h>
#include <spm.h>
#include <spm_hotplug.h>
#include <spm_mcdi.h>
#include <spm_pcm.h>
#include <mtspmc.h>

/*
 * System Power Manager (SPM) is a hardware module, which controls cpu or
 * system power for different power scenarios using different firmware.
 * This driver controls the cpu power in cpu hotplug flow.
 */

static void spm_hotplug_wfi_sel_enter(unsigned int linear_id)
{
	if (linear_id >= 8)	/* pcm_id_from_linear_id: 8 -> 4, 9 -> 5 */
		linear_id -= PLATFORM_CLUSTER1_CORE_COUNT;

#if SPM_ATF_LOG
	spm_trace(LOG_HOTPLUG_OFF | BIT(linear_id));
#endif

	mmio_clrbits_32(SPM_PCM_RESERVE, 1 << linear_id);
	while (mmio_read_32(SPM_PCM_REG4_DATA) != mmio_read_32(SPM_PCM_RESERVE));

	/* SPM WFI Select by core number */
	switch (linear_id) {
	case 0:
		mmio_write_32(SPM_SLEEP_CA7_WFI0_EN, 1);
		break;
	case 1:
		mmio_write_32(SPM_SLEEP_CA7_WFI1_EN, 1);
		break;
	case 2:
		mmio_write_32(SPM_SLEEP_CA7_WFI2_EN, 1);
		break;
	case 3:
		mmio_write_32(SPM_SLEEP_CA7_WFI3_EN, 1);
		break;
	case 4:
		mmio_write_32(SPM_SLEEP_CA15_WFI0_EN, 1);
		break;
	case 5:
		mmio_write_32(SPM_SLEEP_CA15_WFI1_EN, 1);
		break;
	default:
		break;
	}
}

static void spm_hotplug_wfi_sel_leave(unsigned int linear_id)
{
	if (linear_id >= 8)	/* pcm_id_from_linear_id: 8 -> 4, 9 -> 5 */
		linear_id -= PLATFORM_CLUSTER1_CORE_COUNT;

#if SPM_ATF_LOG
	spm_trace(LOG_HOTPLUG_ON | BIT(linear_id));
#endif

	mmio_setbits_32(SPM_PCM_RESERVE, 1 << linear_id);
	while (mmio_read_32(SPM_PCM_REG4_DATA) != mmio_read_32(SPM_PCM_RESERVE));

	/* SPM WFI Select by core number */
	switch (linear_id) {
	case 0:
		mmio_write_32(SPM_SLEEP_CA7_WFI0_EN, 0);
		break;
	case 1:
		mmio_write_32(SPM_SLEEP_CA7_WFI1_EN, 0);
		break;
	case 2:
		mmio_write_32(SPM_SLEEP_CA7_WFI2_EN, 0);
		break;
	case 3:
		mmio_write_32(SPM_SLEEP_CA7_WFI3_EN, 0);
		break;
	case 4:
		mmio_write_32(SPM_SLEEP_CA15_WFI0_EN, 0);
		break;
	case 5:
		mmio_write_32(SPM_SLEEP_CA15_WFI1_EN, 0);
		break;
	default:
		break;
	}
	while (mmio_read_32(SPM_PCM_REG5_DATA) & (1 << linear_id));
}

void spm_go_to_hotplug(void)
{
	struct pwr_ctrl *pwrctrl = spm_hotplug_mcdi.pwrctrl;

	if (!is_hotplug_mcdi_ready()) {
		set_pwrctrl_pcm_flags(pwrctrl, 0);
		spm_set_power_control(pwrctrl);
		spm_set_wakeup_event(pwrctrl);
		spm_kick_pcm_to_run(pwrctrl);
		set_hotplug_mcdi_ready();
		spm_set_pcm_cmd(PCM_CMD_HP_MCDI_PCM);
	}
}

void spm_hotplug_little_cluster_on(void)
{
	spm_go_to_hotplug();

#if SPM_ATF_LOG
	spm_trace(LOG_HOTPLUG_ON | LOG_CLUSTER0);
#endif

	mmio_setbits_32(SPM_PCM_RESERVE, (0x1 << 12));
	while (mmio_read_32(SPM_PCM_REG5_DATA) & (0x1 << 12));
}

void spm_hotplug_big_cluster_on(void)
{
	spm_go_to_hotplug();

#if SPM_ATF_LOG
	spm_trace(LOG_HOTPLUG_ON | LOG_CLUSTER1);
#endif

	mmio_setbits_32(SPM_PCM_RESERVE, (0x1 << 13));
	while (mmio_read_32(SPM_PCM_REG5_DATA) & (0x1 << 13));
}

void spm_hotplug_big_cluster_off(void)
{
	spm_go_to_hotplug();

#if SPM_ATF_LOG
	spm_trace(LOG_HOTPLUG_OFF | LOG_CLUSTER1);
#endif

	mmio_clrbits_32(SPM_PCM_RESERVE, (0x1 << 13));
	while (mmio_read_32(SPM_PCM_REG4_DATA) != mmio_read_32(SPM_PCM_RESERVE));
}

void spm_hotplug_on(unsigned int linear_id)
{
	spm_go_to_hotplug();
	/* turn on CPUx */
	spm_hotplug_wfi_sel_leave(linear_id);
	if (linear_id >= 8) /* pcm_id_from_linear_id: 8 -> 4, 9 -> 5 */
		linear_id -= 4;
	cpu_bitmask |= (1 << linear_id);
}

void spm_hotplug_off(unsigned int linear_id)
{
	spm_go_to_hotplug();
	spm_hotplug_wfi_sel_enter(linear_id);
	if (linear_id >= 8) /* pcm_id_from_linear_id: 8 -> 4, 9 -> 5 */
		linear_id -= 4;
	cpu_bitmask &= ~(1 << linear_id);
}

unsigned int is_big_cluster_on(void)
{
	if ((mmio_read_32(SPM_PCM_REG5_DATA) & (0x1 << 13)) == 0)
		return 1;
	else
		return 0;
}

unsigned int is_little_cluster_on(void)
{
	if ((mmio_read_32(SPM_PCM_REG5_DATA) & (0x1 << 12)) == 0)
		return 1;
	else
		return 0;
}
