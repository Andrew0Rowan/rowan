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
#include <asm_macros.S>
#include <bl_common.h>
#include <artemis.h>
#include <cpu_macros.S>
#include <plat_macros.S>

	/* ---------------------------------------------
	 * Disable L1 data cache
	 * ---------------------------------------------
	 */
func artemis_disable_dcache
	mrs	x1, sctlr_el3
	bic	x1, x1, #SCTLR_C_BIT
	msr	sctlr_el3, x1
	isb
	ret
endfunc artemis_disable_dcache

	/* ---------------------------------------------
	 * Disable intra-cluster coherency
	 * ---------------------------------------------
	 */
func artemis_disable_smp
	mrs	x0, CPUECTLR_EL1
	bic	x0, x0, #CPUECTLR_SMP_BIT
	msr	CPUECTLR_EL1, x0
	isb
	dsb	sy
	ret
endfunc artemis_disable_smp

func artemis_reset_func
	/* ---------------------------------------------
	 * As a bare minimum enable the SMP bit if it is
	 * not already set.
	 * Clobbers : x0
	 * ---------------------------------------------
	 */

#if ERRATA_ARTEMIS_854221
	mrs	x0, S3_0_C15_C0_1
	orr	x0, x0, #1<<2
	msr	S3_0_C15_C0_1, x0
#endif

#if ERRATA_ARTEMIS_855423
	mrs	x0, S3_0_C15_C0_2
	orr	x0, x0, #1<<7
	msr	S3_0_C15_C0_2, x0
#endif

	/* clr [10] TDOSA */
	mrs     x0, MDCR_EL3
	bic     x0, x0, #(1<<10)
	msr     MDCR_EL3, x0
	mrs     x0, MDCR_EL3
	bic     x0, x0, #(1<<9)
	msr     MDCR_EL3, x0
	mrs     x0, MDCR_EL3
	bic     x0, x0, #(1<<6)
	msr     MDCR_EL3, x0

	mrs	x0, CPUECTLR_EL1
	tst	x0, #CPUECTLR_SMP_BIT
	b.ne	skip_smp_setup
	orr	x0, x0, #CPUECTLR_SMP_BIT
	msr	CPUECTLR_EL1, x0
	isb
skip_smp_setup:
	ret
endfunc artemis_reset_func

func artemis_core_pwr_dwn
	mov	x18, x30

	/* ---------------------------------------------
	 * Turn off caches.
	 * ---------------------------------------------
	 */
	bl	artemis_disable_dcache

	/* ---------------------------------------------
	 * Flush L1 caches.
	 * ---------------------------------------------
	 */
	mov	x0, #DCCISW
	bl	dcsw_op_level1

	/* ---------------------------------------------
	 * Come out of intra cluster coherency
	 * ---------------------------------------------
	 */
	mov	x30, x18
	b	artemis_disable_smp
endfunc artemis_core_pwr_dwn

func artemis_cluster_pwr_dwn
	mov	x18, x30

	/* ---------------------------------------------
	 * Turn off caches.
	 * ---------------------------------------------
	 */
	bl	artemis_disable_dcache

	/* ---------------------------------------------
	 * Flush L1 caches.
	 * ---------------------------------------------
	 */
	mov	x0, #DCCISW
	bl	dcsw_op_level1

	/* ---------------------------------------------
	 * Disable the optional ACP.
	 * ---------------------------------------------
	 */
	bl	plat_disable_acp

	/* ---------------------------------------------
	 * Flush L2 caches.
	 * ---------------------------------------------
	 */
	mov	x0, #DCCISW
	bl	dcsw_op_level2

	/* ---------------------------------------------
	 * Come out of intra cluster coherency
	 * ---------------------------------------------
	 */
	mov	x30, x18
	b	artemis_disable_smp
endfunc artemis_cluster_pwr_dwn

	/* ---------------------------------------------
	 * This function provides artemis specific
	 * register information for crash reporting.
	 * It needs to return with x6 pointing to
	 * a list of register names in ascii and
	 * x8 - x15 having values of registers to be
	 * reported.
	 * ---------------------------------------------
	 */
.section .rodata.artemis_regs, "aS"
artemis_regs:  /* The ascii list of register names to be reported */
	.asciz	"cpuectlr_el1", ""

func artemis_cpu_reg_dump
	adr	x6, artemis_regs
	mrs	x8, CPUECTLR_EL1
	ret
endfunc artemis_cpu_reg_dump

declare_cpu_ops artemis, ARTEMIS_MIDR
