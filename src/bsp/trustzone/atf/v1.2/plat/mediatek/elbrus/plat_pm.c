/*
 * Copyright (c) 2013-2015, ARM Limited and Contributors. All rights reserved.
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

#include <arch_helpers.h>
#include <arm_gic.h>
#include <artemis.h>
#include <assert.h>
#include <bakery_lock.h>
#include <console.h>
#include <debug.h>
#include <errno.h>
#include <gpio.h>
#include <log.h>
#include <mcucfg.h>
#include <mcsi.h>
#include <mmio.h>
#include <mt_cpuxgpt.h> /* generic_timer_backup() */
#include <mt_spm.h>
#include <mt_spm_dpidle.h>
#include <mt_spm_sodi.h>
#include <mtcmos.h>
#include <mtspmc.h>
#include <plat_dcm.h>
#include <plat_helpers.h>
#include <plat_pm.h>
#include <plat_private.h>
#include <platform.h>
#include <platform_def.h>
#include <power_tracer.h>
#include <psci.h>
#include <psci_compat.h>
#include <rtc.h>
#include <scu.h>
#include <spm.h>
#include <wfifo.h>

#define MCUSYS_VPROC_DOMAIN_REGISTER	0x10202200
#define MP2_MISC_CONFIG3		0x10202208

extern void gic_cpuif_init(void);
extern void gic_rdist_save(void);
extern void gic_rdist_restore(void);
extern void gic_cpuif_deactivate(unsigned int gicc_base);
extern void gic_dist_save(void);
extern void gic_dist_restore(void);
extern void gic_setup(void);

extern void dfd_setup(void);

unsigned int mcusys_vproc_domain_register[128];
int is_mtk_suspend[PLATFORM_CORE_COUNT];
uintptr_t mtk_suspend_footprint_addr = 0;
uintptr_t mtk_suspend_timestamp_addr = 0;

void mtk_suspend_footprint_log(int idx)
{
	unsigned long mpidr = read_mpidr_el1();
	int cpu = (mpidr & 0xff) | ((mpidr & 0xff00) >> 6);

	if (idx == 4) is_mtk_suspend[cpu] = 1;

	if (is_mtk_suspend[cpu] != 1 || mtk_suspend_footprint_addr == 0)
		return;

	mmio_write_32(footprint_addr(cpu),
		      mmio_read_32(footprint_addr(cpu)) | (1 << idx));

	isb();
	dsb();

	if (idx == 11) is_mtk_suspend[cpu] = 0;
}

void mtk_suspend_timestamp_log(int idx)
{
	unsigned long mpidr = read_mpidr_el1();
	int cpu = (mpidr & 0xff) | ((mpidr & 0xff00) >> 6);

	if (is_mtk_suspend[cpu] != 1 || mtk_suspend_timestamp_addr == 0)
		return;

	mmio_write_64(timestamp_addr(cpu, idx), read_cntpct_el0());

	isb();
	dsb();
}

int mt_cluster_ops(int cputop_mpx, int mode, int state)
{
#if CONFIG_SPMC_MODE != 0
	spmc_cputop_mpx_onoff(cputop_mpx, state, mode);
	return 0;
#else
	return -1;
#endif
}

static void mt_lastpc_wfi(void)
{
	INFO("[ATF]%s\n",__func__);
	dsb();
	while (1) {
		wfi();
	}
}

int mt_lastpc_main(void)
{
	int err = 0;
	void (*mt_lastpc_func_ptr)(void);
	INFO("[ATF]%s\n",__func__);
	mt_lastpc_func_ptr = mt_lastpc_wfi;

	mmio_write_32(MP2_MISC_CONFIG3,mmio_read_32(MP2_MISC_CONFIG3)|0xf<<16);/*set BIG poweup on AARCH64*/
	/* last pc wfi entry function*/
	mmio_write_32(MP2_MISC_CONFIG_BOOT_ADDR_L(0), (unsigned long)mt_lastpc_func_ptr);
	mmio_write_32(MP2_MISC_CONFIG_BOOT_ADDR_H(0),0);
	mmio_write_32(MP2_MISC_CONFIG_BOOT_ADDR_L(1), (unsigned long)mt_lastpc_func_ptr);
	mmio_write_32(MP2_MISC_CONFIG_BOOT_ADDR_H(1),0);
	/*release reset of MP2_corex*/
	mmio_write_32(MP2_CPUTOP_PWR_CON, mmio_read_32(MP2_CPUTOP_PWR_CON) & ~(0x1));
	mmio_write_32(MP2_CPU0_PWR_CON, mmio_read_32(MP2_CPU0_PWR_CON) & ~(0x1));
	mmio_write_32(MP2_CPU1_PWR_CON, mmio_read_32(MP2_CPU1_PWR_CON) & ~(0x1));
	mmio_write_32(MP2_CPUTOP_PWR_CON, mmio_read_32(MP2_CPUTOP_PWR_CON) & ~(0x1<<4));

	INFO("[ATF]%s, ret %d\n", __func__, err);
	return err;
}

int mt_core_ops(int cpux, int mode, int state)
{
	INFO("[ATF]%s ,state=%d \n",__func__,state);
	spmc_cpu_corex_onoff(cpux,state,MODE_SPMC_HW);
	return 0;
}

static void plat_spm_suspend(void)
{
	int state_id = psci_get_suspend_stateid();

	switch (state_id) {
	case MTK_MCDI_MODE:
		/* This block is for MCDI */
		break;
	case MTK_SODI_MODE:
		/* This block is for SODI */
		spm_sodi();
		break;
	case MTK_SODI3_MODE:
		/* This block is for SODI3 */
		break;
	case MTK_DPIDLE_MODE:
		/* This block is for Deep Idle */
		spm_dpidle();
		break;
	case MTK_SUSPEND_MODE:
		/* This block is for Suspend */
		spm_suspend();
		break;
	default:
		break;
	}
}

static void plat_spm_suspend_finish(void)
{
	int state_id = psci_get_suspend_stateid();

	switch (state_id) {
	case MTK_MCDI_MODE:
		/* This block is for MCDI */
		break;
	case MTK_SODI_MODE:
		/* This block is for SODI */
		spm_sodi_finish();
		break;
	case MTK_SODI3_MODE:
		/* This block is for SODI3 */
		break;
	case MTK_DPIDLE_MODE:
		/* This block is for Deep Idle */
		spm_dpidle_finish();
		break;
	case MTK_SUSPEND_MODE:
		/* This block is for Suspend */
		spm_suspend_finish();
		break;
	default:
		break;
	}
}

/*******************************************************************************
 * Function which implements the common MTK_platform specific operations to power down a
 * cpu in response to a CPU_OFF or CPU_SUSPEND request.
 ******************************************************************************/
static void plat_cpu_pwrdwn_common(void)
{
	/* Prevent interrupts from spuriously waking up this cpu */
	gic_cpuif_deactivate(0);
}

/*******************************************************************************
 * Function which implements the common MTK_platform specific operations to power down a
 * cluster in response to a CPU_OFF or CPU_SUSPEND request.
 ******************************************************************************/
static void plat_cluster_pwrdwn_common(void)
{
	uint64_t mpidr = read_mpidr_el1();

	/* Disable coherency if this cluster is to be turned off */
	plat_cci_disable();
	disable_scu(mpidr);

	generic_timer_backup();
	gic_dist_save();
}

static void plat_power_domain_on_finish_common(unsigned int afflvl)
{
	unsigned long mpidr;
	int i;

	/* Get the mpidr for this cpu */
	mpidr = read_mpidr_el1();

	/* Perform the common cluster specific operations */
	if (afflvl != MPIDR_AFFLVL0) {
		for (i = 0; i < 128; i++) {
			mmio_write_32(MCUSYS_VPROC_DOMAIN_REGISTER + (i << 2),
				      mcusys_vproc_domain_register[i]);
		}

		gic_setup();
		gic_dist_restore();

		/* Init mcsi SF */
		plat_cci_init_sf();
		/* Enable coherency if this cluster was off */
		enable_scu(mpidr);
		plat_cci_enable();

		/* enable DFD internal dump */
		dfd_setup();
		/* enable mcusys emi wfifo */
		enable_emi_wfifo();
	}
}

/*******************************************************************************
* Private function which is used to determine if any platform actions
* should be performed for the specified affinity instance given its
* state. Nothing needs to be done if the 'state' is not off or if this is not
* the highest affinity level which will enter the 'state'.
*******************************************************************************/
static int32_t plat_do_plat_actions(unsigned int afflvl, unsigned int state)
{
	unsigned int max_phys_off_afflvl;

	assert(afflvl <= MPIDR_AFFLVL2);

	if (state != PSCI_STATE_OFF)
		return -EAGAIN;

	/*
	 * Find the highest affinity level which will be suspended and postpone
	 * all the platform specific actions until that level is hit.
	 */
	max_phys_off_afflvl = psci_get_max_phys_off_afflvl();
	assert(max_phys_off_afflvl != PSCI_INVALID_DATA);
	if (afflvl != max_phys_off_afflvl)
		return -EAGAIN;

	return 0;
}

/*******************************************************************************
 * MTK_platform handler called when an affinity instance is about to enter
 * standby.
 ******************************************************************************/
static void plat_affinst_standby(unsigned int power_state)
{
	unsigned int target_afflvl;

	plat_spm_suspend();

	/* Sanity check the requested state */
	target_afflvl = psci_get_pstate_afflvl(power_state);

	/*
	 * It's possible to enter standby only on affinity level 0 i.e. a cpu
	 * on the MTK_platform. Ignore any other affinity level.
	 */
	if (target_afflvl == MPIDR_AFFLVL0) {
		/*
		 * Enter standby state. dsb is good practice before using wfi
		 * to enter low power states.
		 */
		dsb();
		wfi();
	}

	plat_spm_suspend_finish();
}

/*******************************************************************************
 * MTK_platform handler called when an affinity instance is about to be turned
 * on. The level and mpidr determine the affinity instance.
 ******************************************************************************/
static int plat_affinst_on(unsigned long mpidr,
		    unsigned long sec_entrypoint,
		    unsigned int afflvl,
		    unsigned int state)
{
	int rc = PSCI_E_SUCCESS;
	unsigned long cpu_id;
	unsigned long cluster_id;
	unsigned long linear_id;

	INFO("[ATF]: %s() mpidr = 0x%lx, sec_entrypoint = 0x%lx, afflvl = %d, state = %d\n",
		__FUNCTION__, mpidr, sec_entrypoint, afflvl, state);

	/*
	 * It's possible to turn on only affinity level 0 i.e. a cpu
	 * on the MTK_platform. Ignore any other affinity level.
	 */
	if (afflvl != MPIDR_AFFLVL0)
		return rc;

	cpu_id = mpidr & MPIDR_CPU_MASK;
	cluster_id = mpidr & MPIDR_CLUSTER_MASK;
	linear_id = platform_get_core_pos(mpidr);
	INFO("[ATF]: mt_on[%ld:%ld:%ld]\n", cluster_id, cpu_id, linear_id);

	switch (linear_id) {
		case 0:
		case 1:
		case 2:
		case 3:
			if (!(cpu_bitmask & 0x0f)) {
			INFO("[ATF]: Enable CPUSYS0\n");
	                #if CONFIG_SPMC_MODE == 0
			INFO("Legacy mode\n");
			spm_mtcmos_ctrl_mp0_cputop_shut_down(1);
	                #elif CONFIG_SPMC_MODE == 1
			INFO("SPMC HW mode\n");
			spmc_cputop_mpx_onoff(CPUTOP_MP0,STA_POWER_ON,MODE_SPMC_HW);
			#else
			INFO("SPMC SW mode\n");
			#endif
					}
					mmio_write_32(MP0_MISC_CONFIG3, mmio_read_32(MP0_MISC_CONFIG3) | 0x0000F000);
					mmio_write_32(MP0_MISC_CONFIG_BOOT_ADDR(linear_id), sec_entrypoint);
					INFO("mt_on[%ld:%ld], entry %x\n", cluster_id, cpu_id,
					mmio_read_32(MP0_MISC_CONFIG_BOOT_ADDR(linear_id)));
			break;
		case 4:
		case 5:
		case 6:
		case 7:
				if (!(cpu_bitmask & 0xf0)) {
				        INFO("[ATF]: Enable CPUSYS1\n");
				#if CONFIG_SPMC_MODE == 0
				INFO("Legacy mode\n");
				spm_mtcmos_ctrl_mp1_cputop_shut_down(1);
					#elif CONFIG_SPMC_MODE == 1
				INFO("SPMC HW mode\n");
					spmc_cputop_mpx_onoff(CPUTOP_MP1,STA_POWER_ON,MODE_SPMC_HW);
				#else
				INFO("SPMC SW mode\n");
				#endif
				}
				mmio_write_32(MP1_MISC_CONFIG3, mmio_read_32(MP1_MISC_CONFIG3) | 0x0000F000);
				mmio_write_32(MP1_MISC_CONFIG_BOOT_ADDR(linear_id-4), sec_entrypoint); /* psci_entrypoint */
				INFO("mt_on[%ld:%ld], entry %x\n", cluster_id, cpu_id,
				mmio_read_32(MP1_MISC_CONFIG_BOOT_ADDR(linear_id-4)));
			break;
		case 8:
		case 9:
			if (!(cpu_bitmask & 0xf00)) {
				INFO("[ATF]: Enable CPUSYS2\n");
			spmc_cputop_mpx_onoff(CPUTOP_MP2,STA_POWER_ON,MODE_SPMC_HW);
			}

			mmio_write_32(0x10202208,mmio_read_32(0x10202208)|0xf<<16);//set BIG poweup on AARCH64
			mmio_write_32(MP2_MISC_CONFIG_BOOT_ADDR_L(linear_id-8), sec_entrypoint); /* psci_entrypoint */
			mmio_write_32(MP2_MISC_CONFIG_BOOT_ADDR_H(linear_id-8),0); /* psci_entrypoint */
			INFO("mt_on[%ld:%ld], entry %x\n", cluster_id, cpu_id,
				mmio_read_32(MP2_MISC_CONFIG_BOOT_ADDR_L(linear_id-8)));
			break;
		default:
			ERROR("[ATF]: %s() CPU%ld not exists\n", __FUNCTION__, linear_id);
			break;
	}
	#if CONFIG_SPMC_MODE == 0
	switch (linear_id) {
		case 0:
			spm_mtcmos_ctrl_mp0_cpu0(1);
			break;
		case 1:
			spm_mtcmos_ctrl_mp0_cpu1(1);
			break;
		case 2:
			spm_mtcmos_ctrl_mp0_cpu2(1);
			break;
		case 3:
			spm_mtcmos_ctrl_mp0_cpu3(1);
			break;
		case 4:
			spm_mtcmos_ctrl_mp1_cpu0(1);
			break;
		case 5:
			spm_mtcmos_ctrl_mp1_cpu1(1);
			break;
		case 6:
			spm_mtcmos_ctrl_mp1_cpu2(1);
			break;
		case 7:
			spm_mtcmos_ctrl_mp1_cpu3(1);
			break;
		default:
			ERROR("[ATF]: %s() CPU%d not exists\n", __FUNCTION__, (int)linear_id);
	}
	#elif CONFIG_SPMC_MODE == 1
		INFO("SPMC HW mode\n");
		spmc_cpu_corex_onoff(linear_id,STA_POWER_ON,MODE_SPMC_HW);
	#else
		INFO("SPMC SW mode\n");
		little_spmc_sw_pwr_on(linear_id);
	#endif

	return rc;
}

/*******************************************************************************
 * MTK_platform handler called when an affinity instance is about to be turned
 * off. The level and mpidr determine the affinity instance. The 'state' arg.
 * allows the platform to decide whether the cluster is being turned off and
 * take apt actions.
 *
 * CAUTION: This function is called with coherent stacks so that caches can be
 * turned off, flushed and coherency disabled. There is no guarantee that caches
 * will remain turned on across calls to this function as each affinity level is
 * dealt with. So do not write & read global variables across calls. It will be
 * wise to do flush a write to the global to prevent unpredictable results.
 ******************************************************************************/
static void plat_affinst_off(unsigned int afflvl, unsigned int state)
{
	unsigned long cpu_id;
	unsigned long cluster_id;
	unsigned long linear_id;
	unsigned long mpidr = read_mpidr_el1();
	INFO("[ATF]: %s() mpidr = 0x%lx afflvl = %d\n", __FUNCTION__, mpidr, afflvl);

	/* Determine if any platform actions need to be executed. */
	if (plat_do_plat_actions(afflvl, state) == -EAGAIN)
		return;

	/* Prevent interrupts from spuriously waking up this cpu */
	/*arm_gic_cpuif_deactivate();*/
	gic_rdist_save();
	gic_cpuif_deactivate(0);

	trace_power_flow(mpidr, CPU_DOWN);

	cpu_id = mpidr & MPIDR_CPU_MASK;
	cluster_id = mpidr & MPIDR_CLUSTER_MASK;
	linear_id = platform_get_core_pos(mpidr);
	INFO("[ATF]: mt_off[%ld:%ld:%ld]\n", cluster_id, cpu_id, linear_id);
	INFO("[ATF] Auto shut-off mode\n");
	/*spmc_cpu_corex_onoff(linear_id,STA_POWER_DOWN, MODE_AUTO_SHUT_OFF);*/

	if (afflvl != MPIDR_AFFLVL0) {
		/* Disable coherency if this cluster is to be turned off */
		plat_cci_disable();
		disable_scu(mpidr);

		trace_power_flow(mpidr, CLUSTER_DOWN);
	}
}

/*******************************************************************************
 * MTK_platform handler called when an affinity instance is about to be
 * suspended. The level and mpidr determine the affinity instance. The 'state'
 * arg. allows the platform to decide whether the cluster is being turned off
 * and take apt actions.
 *
 * CAUTION: This function is called with coherent stacks so that caches can be
 * turned off, flushed and coherency disabled. There is no guarantee that caches
 * will remain turned on across calls to this function as each affinity level is
 * dealt with. So do not write & read global variables across calls. It will be
 * wise to do flush a write to the global to prevent unpredictable results.
 ******************************************************************************/
static void plat_affinst_suspend(unsigned long sec_entrypoint,
			  unsigned int afflvl,
			  unsigned int state)
{
	int i;
	unsigned long linear_id = platform_get_core_pos(read_mpidr());

	/* Determine if any platform actions need to be executed. */
	if (plat_do_plat_actions(afflvl, state) == -EAGAIN)
		return;

	plat_spm_suspend();

	if (linear_id < 4) {
		mmio_write_32(MP0_MISC_CONFIG3, mmio_read_32(MP0_MISC_CONFIG3) | 0x0000F000);
		mmio_write_32(MP0_MISC_CONFIG_BOOT_ADDR(linear_id), sec_entrypoint);
	} else if (linear_id < 8) {
		mmio_write_32(MP1_MISC_CONFIG3, mmio_read_32(MP1_MISC_CONFIG3) | 0x0000F000);
		mmio_write_32(MP1_MISC_CONFIG_BOOT_ADDR(linear_id - 4), sec_entrypoint);
	}

#if SPMC_SPARK2
	if (linear_id >= 4) {
		set_cpu_retention_control(0);
		little_spark2_core_enable(linear_id, 0);
	}
	/* TODO */
	/* while(mmio_read_32(SPM_CPU_RET_STATUS) & (1 << linear_id)); */
#endif

	/* Perform the common cpu specific operations */
	plat_cpu_pwrdwn_common();

	/* Perform the common cluster specific operations */
	if (afflvl != MPIDR_AFFLVL0) {
		mcsi_init(0x0C510000, 3);
		plat_cluster_pwrdwn_common();
		for (i = 0; i < 128; i++) {
			mcusys_vproc_domain_register[i] =
				mmio_read_32(MCUSYS_VPROC_DOMAIN_REGISTER + (i << 2));
		}
	}

	MT_LOG_KTIME_CLEAR();

	plat_dcm_mcsi_backup();
}

/*******************************************************************************
 * MTK_platform handler called when an affinity instance has just been powered
 * on after being turned off earlier. The level and mpidr determine the affinity
 * instance. The 'state' arg. allows the platform to decide whether the cluster
 * was turned off prior to wakeup and do what's necessary to setup it up
 * correctly.
 ******************************************************************************/
static void plat_affinst_on_finish(unsigned int afflvl, unsigned int state)
{
	unsigned long mpidr = read_mpidr_el1();
	unsigned ectlr;
#if SPMC_SPARK2
	unsigned long linear_id;
#endif

	INFO("[ATF]: %s() mpidr = 0x%lx afflvl = %d\n", __FUNCTION__, mpidr, afflvl);
	/* Determine if any platform actions need to be executed. */
	if (plat_do_plat_actions(afflvl, state) == -EAGAIN)
		return;

	/* Perform the common cluster specific operations */
	if (afflvl >= MPIDR_AFFLVL1) {
		enable_scu(mpidr);
		INFO("[ATF]: %s() ... enable_scu() ... done\n", __FUNCTION__);

		/* Enable coherency if this cluster was off */
		plat_cci_enable();
		INFO("[ATF]: %s() ... plat_cci_enable() ... done\n", __FUNCTION__);
		trace_power_flow(mpidr, CLUSTER_UP);
	}
	INFO("[ATF]: %s() Set CPUECTLR_SMP_BIT\n", __FUNCTION__);
	/*
	* Turn on intra-cluster coherency if the MTK_platform flavour supports
	* it.
	*/
	ectlr = read_cpuectlr();
	ectlr |= CPUECTLR_SMP_BIT;
	write_cpuectlr(ectlr);

	/* Enable the gic cpu interface */
	//arm_gic_cpuif_setup();
	//arm_gic_pcpu_distif_setup();
	gic_cpuif_init();

#if SPMC_SPARK2
	/* ---------------------------------------------
	 * CPU retention control
	 * Set the CPUECTLR[2:0] = 0x01
	 * ---------------------------------------------
	 */
	linear_id = platform_get_core_pos(mpidr);
	if (linear_id >= 4) {
		set_cpu_retention_control(0x1);
	}
#endif

	gic_rdist_restore();
	trace_power_flow(mpidr, CPU_UP);
	/* TODO: This setup is needed only after a cold boot */
	enable_ns_access_to_cpuectlr();
}

/*******************************************************************************
 * MTK_platform handler called when an affinity instance has just been powered
 * on after having been suspended earlier. The level and mpidr determine the
 * affinity instance.
 ******************************************************************************/
static void plat_affinst_suspend_finish(unsigned int afflvl, unsigned int state)
{
#if SPMC_SPARK2
	unsigned long linear_id;
#endif

	/* Determine if any platform actions need to be executed. */
	if (plat_do_plat_actions(afflvl, state) == -EAGAIN)
		return;

	plat_power_domain_on_finish_common(afflvl);

	plat_spm_suspend_finish();

	/* Enable the gic cpu interface */
	gic_cpuif_init();

#if SPMC_SPARK2
	linear_id = platform_get_core_pos(read_mpidr());
	if (linear_id >= 4) {
		little_spark2_setldo(linear_id);
		little_spark2_core_enable(linear_id, 1);
		set_cpu_retention_control(1);
	}
#endif

	plat_dcm_mcsi_restore();
}

static unsigned int plat_get_sys_suspend_power_state(void)
{
	/* StateID: 0, StateType: 1(power down), PowerLevel: 2(system) */
	return psci_make_powerstate(0, 1, 2);
}

/*******************************************************************************
 * MTK handlers to shutdown/reboot the system
 ******************************************************************************/
static void __dead2 plat_system_off(void)
{
	INFO("MTK System Off\n");

	gpio_set(GPIO120, GPIO_OUT_ZERO);
	rtc_bbpu_power_down();

	wfi();
	ERROR("MTK System Off: operation not handled.\n");
	panic();
}

static void __dead2 plat_system_reset(void)
{
	/* Write the System Configuration Control Register */
	INFO("MTK System Reset\n");

	mmio_clrbits_32(MTK_WDT_BASE,
		(MTK_WDT_MODE_DUAL_MODE | MTK_WDT_MODE_IRQ));
	mmio_setbits_32(MTK_WDT_BASE, (MTK_WDT_MODE_KEY | MTK_WDT_MODE_EXTEN));
	mmio_setbits_32(MTK_WDT_SWRST, MTK_WDT_SWRST_KEY);

	wfi();
	ERROR("MTK System Reset: operation not handled.\n");
	panic();
}

/*******************************************************************************
 * Export the platform handlers to enable psci to invoke them
 ******************************************************************************/
static const plat_pm_ops_t plat_plat_pm_ops = {
	.affinst_standby		= plat_affinst_standby,
	.affinst_on			= plat_affinst_on,
	.affinst_off			= plat_affinst_off,
	.affinst_suspend		= plat_affinst_suspend,
	.affinst_on_finish		= plat_affinst_on_finish,
	.affinst_suspend_finish		= plat_affinst_suspend_finish,
	.system_off			= plat_system_off,
	.system_reset			= plat_system_reset,
	.get_sys_suspend_power_state	= plat_get_sys_suspend_power_state,
};

/*******************************************************************************
 * Export the platform specific power ops & initialize the mtk_platform power
 * controller
 ******************************************************************************/
int platform_setup_pm(const plat_pm_ops_t **plat_ops)
{
	*plat_ops = &plat_plat_pm_ops;
	return 0;
}
