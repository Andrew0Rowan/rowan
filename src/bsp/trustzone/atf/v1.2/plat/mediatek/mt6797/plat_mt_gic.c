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
#include <bl_common.h>
#include <platform_def.h>
#include <platform.h>
#include <stdio.h>
#include <stdint.h>

#include <debug.h>
#include <gic_v2.h>
#include <gic_v3.h>

#include <mmio.h>
#include <mt_gic_v3.h>
#if 0
const unsigned int mt_irq_sec_array[] = {
	MT_IRQ_SEC_SGI_0,
	MT_IRQ_SEC_SGI_1,
	MT_IRQ_SEC_SGI_2,
	MT_IRQ_SEC_SGI_3,
	MT_IRQ_SEC_SGI_4,
	MT_IRQ_SEC_SGI_5,
	MT_IRQ_SEC_SGI_6,
	MT_IRQ_SEC_SGI_7
};

void plat_mt_gic_init(void)
{
	arm_gic_init(BASE_GICC_BASE,
		BASE_GICD_BASE,
		BASE_GICR_BASE,
		mt_irq_sec_array,
		ARRAY_SIZE(mt_irq_sec_array));
}

#endif

#define GIC_DEBUG

#ifndef MAX_GIC_NR
#define MAX_GIC_NR			(1)
#endif

#define MAX_RDIST_NR			(64)

#define DIV_ROUND_UP(n,d)		(((n) + (d) - 1) / (d))

#define SZ_64K                          (0x00010000)

/* helpers for later ICC encode macros
 * Indirect stringification.  Doing two levels allows the parameter to be a
 * macro itself.  For example, compile with -DFOO=bar, __stringify(FOO)
 * converts to "bar".
 */
#define __stringify_1(x)     #x
#define __stringify(x)       __stringify_1(x)

/* end of ICC sysreg encode macros */

#define MPIDR_LEVEL_BITS_SHIFT  3
#define MPIDR_LEVEL_BITS        (1 << MPIDR_LEVEL_BITS_SHIFT)
#define MPIDR_LEVEL_MASK ((1 << MPIDR_LEVEL_BITS) - 1)

#define MPIDR_LEVEL_SHIFT(level) \
        (((1 << level) >> 1) << MPIDR_LEVEL_BITS_SHIFT)

#define MPIDR_AFFINITY_LEVEL(mpidr, level) \
        ((mpidr >> MPIDR_LEVEL_SHIFT(level)) & MPIDR_LEVEL_MASK)

static uint64_t cpu_logical_map[PLATFORM_CORE_COUNT];

/* For saving ATF size, we reduce 1020 -> 512 */
struct gic_chip_data {
	unsigned int saved_enable[DIV_ROUND_UP(512, 32)];
	unsigned int saved_conf[DIV_ROUND_UP(512, 16)];
	unsigned int saved_target[DIV_ROUND_UP(512, 4)];
	unsigned int saved_group[DIV_ROUND_UP(512, 32)];
	unsigned int rdist_base[MAX_RDIST_NR];
};

static struct gic_chip_data gic_data[MAX_GIC_NR];

#if 0
static void gicc_write_grpen0_el1(uint64_t val)
{
	__asm__ volatile("msr_s " __stringify(ICC_GRPEN0_EL1) ", %0" : : "r" (val));
}
#endif

static void gicd_v3_do_wait_for_rwp(unsigned int gicd_base)
{
	while (gicd_v3_read_ctlr(gicd_base) & GICD_V3_CTLR_RWP);
}

static int gic_populate_rdist(unsigned int *rdist_base)
{
	int cpuid = 0;
	unsigned int reg = 0;
	unsigned int base = 0;

	cpuid = platform_get_core_pos(read_mpidr());
	reg = gicd_v3_read_pidr2(MT_GIC_BASE) & GIC_V3_PIDR2_ARCH_MASK;

	if ((reg != GIC_V3_PIDR2_ARCH_GICv3) && (reg != GIC_V3_PIDR2_ARCH_GICv4)) {
		INFO("We are in trouble, not GIC-v3 or v4\n");
		return -1;
	}
#if 0
	/* when mp0 off, the _REAL_ redistributor base should be adjusted to original one
	 * SZ_64K*2 is the total length of a rdist, a page is 64KB, one page for overall control,
	 * one page for SGI. Check GICv3 spec, section 5.4.1 */
	if (is_mp0_off()) {
		base = MT_GIC_RDIST_BASE + (cpuid+4)*SZ_64K*2;
	} else {
		base = MT_GIC_RDIST_BASE + cpuid*SZ_64K*2;
	}
#else
	base = MT_GIC_RDIST_BASE + cpuid*SZ_64K*2;
#endif
	*rdist_base = base;
	/* save a copy for later save/restore use */
	gic_data[0].rdist_base[cpuid] = base;

#ifdef GIC_DEBUG
	INFO("cpu(%d), rdist_base = 0x%x\n", cpuid, *rdist_base);
#endif
	return 0;
}

#if 0
static void enable_sgi_fiq(void)
{
	unsigned int rdist_sgi_base = 0;

	/* get the base of redistributor first */
	if (gic_populate_rdist(&rdist_sgi_base) == -1) {
		return;
	}

	/* the SGI/PPI base is at 64K offset of current rdist base */
	rdist_sgi_base += SZ_64K;

	/* set all SGI/PPI as NS-group1 by default */
	gicd_write_igroupr(rdist_sgi_base, 0, ~0);

	/* set fiq smp call as fiq */
	gicd_clr_igroupr(rdist_sgi_base, FIQ_SMP_CALL_SGI);
	gicd_set_ipriorityr(rdist_sgi_base, FIQ_SMP_CALL_SGI, GIC_HIGHEST_SEC_PRIORITY);
	gicd_set_isenabler(rdist_sgi_base, FIQ_SMP_CALL_SGI);
}
#endif

int gic_cpuif_init(void)
{
	unsigned int rdist_base = 0;
	unsigned int val = 0;
	unsigned int scr_val = 0;
	uint64_t mpidr = 0;
	uint32_t cpu = 0;

	if (gic_populate_rdist(&rdist_base) == -1)
		return -1;

	/* Wake up this CPU redistributor */
	val = mmio_read_32(rdist_base+GICR_V3_WAKER);
	val &= ~GICR_V3_WAKER_ProcessorSleep;
	mmio_write_32(rdist_base+GICR_V3_WAKER, val);

#if 0 // fixme
	/* wait until our children wakeup */
	while (mmio_read_32(rdist_base + GICR_V3_WAKER) &
			GICR_V3_WAKER_ChildrenAsleep);
#endif
	/* set all SGI/PPI as non-secure GROUP1 by default.
	   rdist_base + 64K == SGI_base */
	mmio_write_32(rdist_base+SZ_64K+GICE_V3_IGROUP0, 0xffffffff);
	mmio_write_32(rdist_base+SZ_64K+GICE_V3_IGRPMOD0, 0x0);

	/*
	 * We need to set SCR_EL3.NS in order to see GICv3 non-secure state.
	 * Restore SCR_EL3.NS again before exit.
	 */
	scr_val = read_scr();
	write_scr(scr_val | SCR_NS_BIT);
	isb();	/* ensure NS=1 takes effect before accessing ICC_SRE_EL2 */

	/* enable SRE bit in ICC_SRE_ELx in order */
	val = read_icc_sre_el3();
	write_icc_sre_el3(val | ICC_SRE_EN | ICC_SRE_SRE);
	isb(); /* before enable lower SRE, be sure SRE in el3 takes effect */

	val = read_icc_sre_el2();
	write_icc_sre_el2(val | ICC_SRE_EN | ICC_SRE_SRE);
	isb(); /* before enable lower SRE, be sure SRE in el2 takes effect */

	write_icc_pmr_el1(GIC_PRI_MASK);
	isb();	/* commite ICC_* changes before setting NS=0 */

	/* Restore SCR_EL3 */
	write_scr(scr_val);
	isb();	/* ensure NS=0 takes effect immediately */

	/* MUST set secure copy of icc_sre_el1 as SRE_SRE to enable FIQ,
	see GICv3 spec 4.6.4 FIQ Enable */
	val = read_icc_sre_el1();
	write_icc_sre_el1(val | ICC_SRE_SRE);
	isb(); /* before we can touch other ICC_* system registers, make sure this have effect */

	/* here we go, can handle FIQ after this */
	/* gicc_write_grpen0_el1(0x1); */

	/* everytime we setup the cpu IF, add a SGI as FIQ for smp call debug */
	/* enable_sgi_fiq(); */

	/* init mpidr table for this cpu for later sgi usage */
	mpidr = read_mpidr();
	cpu = platform_get_core_pos(mpidr);
	if (cpu < PLATFORM_CORE_COUNT)
		cpu_logical_map[cpu] = mpidr;

	return 0;
}


static void gic_distif_init(unsigned int gicd_base)
{
	unsigned int ctlr = 0;
	unsigned int irq_set = 0;
	unsigned int i = 0;

	/* disable first before going on */
	ctlr = gicd_v3_read_ctlr(gicd_base);
	ctlr &= ~(GICD_V3_CTLR_ENABLE_G0 | GICD_V3_CTLR_ENABLE_G1NS | GICD_V3_CTLR_ENABLE_G1S);
	gicd_v3_write_ctlr(gicd_base, ctlr);
	gicd_v3_do_wait_for_rwp(gicd_base);

	/* query how many irq sets in our implementation,
	 * the formula is found in section 4.3.2 GIC400 TRM */
	irq_set = (mmio_read_32(gicd_base+GICD_V3_TYPER)&0x1f)+1;

	/* set all SPI as non-secure group1 by default,
	 * index from 1, because GICE_V3_IGROUP0, GICE_V3_IGRPMOD0 are RES0,
	 * equivalent function is provided by GICR_IGROUPR0, GICE_V3_IGRPMOD0,
	 * which are both initialized in gic_cpuif_init() */
	for (i = 1; i < irq_set; i++) {
		mmio_write_32(gicd_base+GICE_V3_IGROUP0+(i<<2), 0xffffffff);
		mmio_write_32(gicd_base+GICE_V3_IGRPMOD0+(i<<2), 0x0);
	}

	/* enable all groups & ARE */
	ctlr = GICD_V3_CTLR_ENABLE_G0 | GICD_V3_CTLR_ENABLE_G1NS | GICD_V3_CTLR_ENABLE_G1S |
		GICD_V3_CTLR_ARE_S | GICD_V3_CTLR_ARE_NS;

	gicd_v3_write_ctlr(gicd_base, ctlr);
	gicd_v3_do_wait_for_rwp(gicd_base);
}


void gic_setup(void)
{
	/*
	unsigned int gicd_base = 0;

	gicd_base = get_plat_config()->gicd_base;
	*/

	gic_distif_init(BASE_GICD_BASE);
	gic_cpuif_init();
	/*
	enable_wdt_fiq();
	*/
}
