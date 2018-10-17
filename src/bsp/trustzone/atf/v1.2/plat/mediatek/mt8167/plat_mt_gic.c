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
#include <mt8167_def.h>
#include <platform_def.h>
#include <mmio.h>
#include <debug.h>

/* helpers for later ICC encode macros
 * Indirect stringification.  Doing two levels allows the parameter to be a
 * macro itself.  For example, compile with -DFOO=bar, __stringify(FOO)
 * converts to "bar".
 */
#define __stringify_1(x)	#x
#define __stringify(x)		__stringify_1(x)

/* since gcc not support most ARMv8 ICC sysreg in asm,
 * we learn Linux's way to encode them */
#define sys_reg(op0, op1, crn, crm, op2) \
        ((((op0)-2)<<19)|((op1)<<16)|((crn)<<12)|((crm)<<8)|((op2)<<5))

__asm__ (
"       .irp    num,0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30\n"
"       .equ    __reg_num_x\\num, \\num\n"
"       .endr\n"
"       .equ    __reg_num_xzr, 31\n"
"\n"
"       .macro  mrs_s, rt, sreg\n"
"       .inst   0xd5300000|(\\sreg)|(__reg_num_\\rt)\n"
"       .endm\n"
"\n"
"       .macro  msr_s, sreg, rt\n"
"       .inst   0xd5100000|(\\sreg)|(__reg_num_\\rt)\n"
"       .endm\n"
);
/* end of ICC sysreg encode macros */

#define ICC_GRPEN0_EL1                  sys_reg(3, 0, 12, 12, 6)
#define ICC_GRPEN1_EL1                  sys_reg(3, 0, 12, 12, 7)
#define ICC_GRPEN1_EL3                  sys_reg(3, 6, 12, 12, 7)
#define ICC_IAR0_EL1_			sys_reg(3, 0, 12,  8, 0)
#define ICC_HPPIR0_EL1_			sys_reg(3, 0, 12,  8, 2)
#define ICC_HPPIR1_EL1_			sys_reg(3, 0, 12, 12, 2)
#define ICC_SGI0R_EL1			sys_reg(3, 0, 12, 11, 7)
#define ICC_EOIR0_EL1_			sys_reg(3, 0, 12,  8, 1)

static uint64_t cpu_logical_map[PLATFORM_CORE_COUNT];

const unsigned int mt_irq_sec_array[] = {
	MT_IRQ_SEC_SGI_0,
	MT_IRQ_SEC_SGI_1,
	MT_IRQ_SEC_SGI_2,
	MT_IRQ_SEC_SGI_3,
	MT_IRQ_SEC_SGI_4,
	MT_IRQ_SEC_SGI_5,
	MT_IRQ_SEC_SGI_6,
	MT_IRQ_SEC_SGI_7,
	//WDT_IRQ_BIT_ID,
};

void plat_mt_gic_init(void)
{
	int idx;
	arm_gic_init(BASE_GICC_BASE,
		BASE_GICD_BASE,
		BASE_GICR_BASE,
		mt_irq_sec_array,
		ARRAY_SIZE(mt_irq_sec_array));

	/* set pol control as non-secure */
	for (idx=0;idx<INT_POL_SECCTL_NUM;idx++)
		mmio_write_32(INT_POL_SECCTL0+idx*4, 0);
}

static void gicc_write_sgi0_el1(uint64_t val)
{
	__asm__ volatile("msr_s " __stringify(ICC_SGI0R_EL1) ", %0" : : "r" (val));
}

static uint16_t compute_target_list(uint8_t *cpu, unsigned int map, uint64_t cluster_id)
{
	uint16_t tlist = 0;
	uint8_t target_cpu = 0;

	for (target_cpu = *cpu; target_cpu < PLATFORM_CORE_COUNT; ++target_cpu) {
		uint64_t mpidr = cpu_logical_map[target_cpu];
		uint64_t target_cluster_id = mpidr & ~0xffUL;

		/* gic-500 only support 16 cores in a cluster at max */
		if ((mpidr & 0xff) > 16) {
			INFO("%s:%d: wrong cpu mpidr(%lx)\n",
				__func__, __LINE__, mpidr);
			break;
		}

		/* only check those cores in map */
		if (((1 << target_cpu) & map) == 0) {
			continue;
		}

		/* we assume cluster_id only changes _WHEN_
		 * all cores in the same cluster is counted */
		if (target_cluster_id != cluster_id) {
			break;
		} else {
			/* the core in the same cluster, add it. */
			tlist |= 1 << (mpidr & 0xff);
		}
	}

	/* update cpu for the next cluster */
	*cpu = target_cpu - 1;
	return tlist;
}

static void gic_send_sgi(uint64_t cluster_id, uint16_t tlist, unsigned int irq)
{
	uint64_t val = 0;

	val = irq << 24					|
		(((cluster_id >> 8) & 0xff) << 16)	|
		tlist;
#ifdef GIC_DEBUG
	INFO("\n\n### %lx ###\n\n", val);
#endif
	gicc_write_sgi0_el1(val);
}

static void dump_cpu_map(void)
{
#ifdef GIC_DEBUG
	int i = 0;

	for (;i < PLATFORM_CORE_COUNT; ++i)
		INFO("\n\n@@ cpu(%d) = %lx\n\n", i, cpu_logical_map[i]);
#endif
}

void irq_raise_softirq(unsigned int map, unsigned int irq)
{
	uint8_t cpu = 0;
	uint16_t tlist = 0;
	uint64_t cluster_id = 0;
	uint64_t mpidr = 0;

	dump_cpu_map();

	/*
         * Ensure that stores to Normal memory are visible to the
         * other CPUs before issuing the IPI.
         */
	__asm__ volatile("dmb ishst" : : : "memory");

	for (cpu = 0; cpu < PLATFORM_CORE_COUNT ; ++cpu) {
		/* only check those cores in map */
		if (((1 << cpu) & map) == 0) {
			continue;
		}

		/* gicv3 can only send sgi per cluster,
		 * gather all cores in map in the same cluster */
		mpidr = cpu_logical_map[cpu];
		cluster_id = mpidr & ~0xffUL;
		tlist = compute_target_list(&cpu, map, cluster_id);
#ifdef GIC_DEBUG
		INFO("\n\n%s:%d: mpidr = %lx, cluster_id = %x, tlist = %x\n\n",
			__func__, __LINE__, mpidr, cluster_id, tlist);
		INFO("\n\n%s:%d: cpu = %d\n\n", __func__, __LINE__, cpu);
#endif
		gic_send_sgi(cluster_id, tlist, irq);
	}

	/* Force the above writes to ICC_SGI0R_EL1 to be executed */
	isb();
}
