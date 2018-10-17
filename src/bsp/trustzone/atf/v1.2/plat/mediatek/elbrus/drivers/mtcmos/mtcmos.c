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
#include <platform_def.h>
#include <mtcmos.h>
#include <spm.h>
unsigned int cpu_bitmask = 1;
#include <debug.h>
#include <delay_timer.h>
#include <mcucfg.h>



#if 0
enum {
	SRAM_ISOINT_B	= 1U << 6,
	SRAM_CKISO	= 1U << 5,
	PWR_CLK_DIS	= 1U << 4,
	PWR_ON_2ND	= 1U << 3,
	PWR_ON		= 1U << 2,
	PWR_ISO		= 1U << 1,
	PWR_RST_B	= 1U << 0
};
enum {
	L1_PDN_ACK	= 1U << 8,
	L1_PDN		= 1U << 0
};

enum {
	LITTLE_CPU3	= 1U << 12,
	LITTLE_CPU2	= 1U << 11,
	LITTLE_CPU1	= 1U << 10,
};

enum {
	SRAM_PDN           = 0xf << 8,
	DIS_SRAM_ACK       = 0x1 << 12,
	AUD_SRAM_ACK       = 0xf << 12,
};

enum {
	DIS_PWR_STA_MASK   = 0x1 << 3,
	AUD_PWR_STA_MASK   = 0x1 << 24,
};

static void mtcmos_ctrl_little_off(unsigned int linear_id)
{
	uint32_t reg_pwr_con;
	uint32_t reg_l1_pdn;
	uint32_t bit_cpu;

	switch (linear_id) {
	case 1:
		reg_pwr_con = SPM_CA7_CPU1_PWR_CON;
		reg_l1_pdn = SPM_CA7_CPU1_L1_PDN;
		bit_cpu = LITTLE_CPU1;
		break;
	case 2:
		reg_pwr_con = SPM_CA7_CPU2_PWR_CON;
		reg_l1_pdn = SPM_CA7_CPU2_L1_PDN;
		bit_cpu = LITTLE_CPU2;
		break;
	case 3:
		reg_pwr_con = SPM_CA7_CPU3_PWR_CON;
		reg_l1_pdn = SPM_CA7_CPU3_L1_PDN;
		bit_cpu = LITTLE_CPU3;
		break;
	default:
		/* should never come to here */
		return;
	}

	/* enable register control */
	mmio_write_32(SPM_POWERON_CONFIG_SET,
			(SPM_PROJECT_CODE << 16) | (1U << 0));

	mmio_setbits_32(reg_pwr_con, PWR_ISO);
	mmio_setbits_32(reg_pwr_con, SRAM_CKISO);
	mmio_clrbits_32(reg_pwr_con, SRAM_ISOINT_B);
	mmio_setbits_32(reg_l1_pdn, L1_PDN);

	while (!(mmio_read_32(reg_l1_pdn) & L1_PDN_ACK))
		continue;

	mmio_clrbits_32(reg_pwr_con, PWR_RST_B);
	mmio_setbits_32(reg_pwr_con, PWR_CLK_DIS);
	mmio_clrbits_32(reg_pwr_con, PWR_ON);
	mmio_clrbits_32(reg_pwr_con, PWR_ON_2ND);

	while ((mmio_read_32(SPM_PWR_STATUS) & bit_cpu) ||
	       (mmio_read_32(SPM_PWR_STATUS_2ND) & bit_cpu))
		continue;
}
#endif

void mtcmos_little_cpu_off(void)
{
	/* turn off little cpu 1 - 3 */
#if 0
	mtcmos_ctrl_little_off(1);
	mtcmos_ctrl_little_off(2);
	mtcmos_ctrl_little_off(3);
#endif
}


#define IGNORE_PWR_ACK		1
#define DDR_RESERVE_ENABLE	1
#define IDX_PROTECT_MP0_CACTIVE		10
#define IDX_PROTECT_MP1_CACTIVE		11
#define IDX_PROTECT_ICC0_CACTIVE	12
#define IDX_PROTECT_ICD0_CACTIVE	13
#define IDX_PROTECT_ICC1_CACTIVE	14
#define IDX_PROTECT_ICD1_CACTIVE	15
#define IDX_PROTECT_L2C0_CACTIVE	26
#define IDX_PROTECT_L2C1_CACTIVE	27
/* APB Module infracfg_ao */
#define INFRA_TOPAXI_PROTECTEN		(INFRACFG_AO_BASE + 0x220)
#define INFRA_TOPAXI_PROTECTSTA0	(INFRACFG_AO_BASE + 0x224)
#define INFRA_TOPAXI_PROTECTSTA1	(INFRACFG_AO_BASE + 0x228)
#define INFRA_TOPAXI_PROTECTEN_1	(INFRACFG_AO_BASE + 0x250)
#define INFRA_TOPAXI_PROTECTSTA0_1	(INFRACFG_AO_BASE + 0x254)
#define INFRA_TOPAXI_PROTECTSTA1_1	(INFRACFG_AO_BASE + 0x258)
/* */
#define STA_POWER_DOWN		0
#define STA_POWER_ON		1
#define mcu_spm_read(addr)		spm_read(addr)
#define mcu_spm_write(addr, val)	spm_write(addr, val)
/*
 * SPMC Mode
 */
#ifdef MTCMOS_CTRL_MP0
/*
 * MTCMOS Legacy Mode
 */
int spm_mtcmos_ctrl_mp0_cputop_dormant(int state)
{
	int err = 0;
	const unsigned int mp0_mask =
		(1 << IDX_PROTECT_ICC0_CACTIVE) |
		(1 << IDX_PROTECT_ICD0_CACTIVE) |
		(1 << IDX_PROTECT_MP0_CACTIVE) |
		(1 << IDX_PROTECT_L2C0_CACTIVE);
	/* TINFO="enable SPM register control" */
	mcu_spm_write(POWERON_CONFIG_EN, (SPM_PROJECT_CODE << 16) | (0x1 << 0));
	if (state == STA_POWER_DOWN) {
		/* TINFO="Start to turn off MP0_CPUTOP" */
		/* TINFO="Set STANDBYWFIL2 for Cluster 0" */
		*MP0_AXI_CONFIG |= 0x00000010;
		*(volatile unsigned int *)(0x10394000) &= 0xfffffffc;
		*(volatile unsigned int *)(0x10395000) &= 0xfffffffc;
		/* TINFO="Wait STANDBYWFIL2 for Cluster 0" */
		while ((*MP0_CA7_MISC_CONFIG & (1 << 28)) != (1 << 28));
		/* TINFO="Set ADB pwrdnreqn for Cluster 0" */
		*INFRA_TOPAXI_PROTECTEN_1 |= mp0_mask;
		/* TINFO="Wait ADB pwrdnreqn for Cluster 0" */
		while ((*INFRA_TOPAXI_PROTECTSTA1_1 & mp0_mask) != mp0_mask);
		/* TINFO="Set PWR_ISO = 1" */
		mcu_spm_write(MP0_CPUTOP_PWR_CON, mcu_spm_read(MP0_CPUTOP_PWR_CON) | PWR_ISO);
		/* TINFO="Set SRAM_CKISO = 1" */
		mcu_spm_write(MP0_CPUTOP_PWR_CON, mcu_spm_read(MP0_CPUTOP_PWR_CON) | SRAM_CKISO);
		/* TINFO="Set SRAM_ISOINT_B = 0" */
		mcu_spm_write(MP0_CPUTOP_PWR_CON, mcu_spm_read(MP0_CPUTOP_PWR_CON) & ~SRAM_ISOINT_B);
		/* TINFO="Set SRAM_SLEEP_B = 0" */
		mcu_spm_write(MP0_CPUTOP_PWR_CON, mcu_spm_read(MP0_CPUTOP_PWR_CON) & ~SRAM_SLEEP_B);
		/* TINFO="Wait until MP0_CPUTOP_SRAM_SLEEP_B_ACK = 0" */
		while (mcu_spm_read(MP0_CPUTOP_PWR_CON) & SRAM_SLEEP_B_ACK ) {
			/* SRAM SLEEP delay IP clock is 26MHz. Print SRAM / MTCMOS control and PWR_ACK for debug. */
		}
		/* TINFO="Set PWR_RST_B = 0" */
		mcu_spm_write(MP0_CPUTOP_PWR_CON, mcu_spm_read(MP0_CPUTOP_PWR_CON) & ~PWR_RST_B);
		/* TINFO="Set PWR_CLK_DIS = 1" */
		mcu_spm_write(MP0_CPUTOP_PWR_CON, mcu_spm_read(MP0_CPUTOP_PWR_CON) | PWR_CLK_DIS);
		/* TINFO="Set PWR_ON = 0" */
		mcu_spm_write(MP0_CPUTOP_PWR_CON, mcu_spm_read(MP0_CPUTOP_PWR_CON) & ~PWR_ON);
		/* TINFO="Set PWR_ON_2ND = 0" */
		mcu_spm_write(MP0_CPUTOP_PWR_CON, mcu_spm_read(MP0_CPUTOP_PWR_CON) & ~PWR_ON_2ND);
#ifndef IGNORE_PWR_ACK
		/* TINFO="Wait until PWR_STATUS = 0 and PWR_STATUS_2ND = 0" */
		while ((mcu_spm_read(PWR_STATUS) & MP0_CPUTOP_PWR_STA_MASK)
		       || (mcu_spm_read(PWR_STATUS_2ND) & MP0_CPUTOP_PWR_STA_MASK)) {
			/* No logic between pwr_on and pwr_ack. Print SRAM / MTCMOS control and PWR_ACK for debug. */
		}
#endif
		/* TINFO="Finish to turn off MP0_CPUTOP" */
	} else {    /* STA_POWER_ON */
		/* TINFO="Start to turn on MP0_CPUTOP" */
		/* TINFO="Set PWR_ON = 1" */
		mcu_spm_write(MP0_CPUTOP_PWR_CON, mcu_spm_read(MP0_CPUTOP_PWR_CON) | PWR_ON);
		/* TINFO="Set PWR_ON_2ND = 1" */
		mcu_spm_write(MP0_CPUTOP_PWR_CON, mcu_spm_read(MP0_CPUTOP_PWR_CON) | PWR_ON_2ND);
#ifndef IGNORE_PWR_ACK
		/* TINFO="Wait until PWR_STATUS = 1 and PWR_STATUS_2ND = 1" */
		while (!(mcu_spm_read(PWR_STATUS) & MP0_CPUTOP_PWR_STA_MASK)
		       || !(mcu_spm_read(PWR_STATUS_2ND) & MP0_CPUTOP_PWR_STA_MASK)) {
			/* No logic between pwr_on and pwr_ack. Print SRAM / MTCMOS control and PWR_ACK for debug. */
		}
#endif
		/* TINFO="Set PWR_ISO = 0" */
		mcu_spm_write(MP0_CPUTOP_PWR_CON, mcu_spm_read(MP0_CPUTOP_PWR_CON) & ~PWR_ISO);
		/* TINFO="Set SRAM_SLEEP_B = 1" */
		mcu_spm_write(MP0_CPUTOP_PWR_CON, mcu_spm_read(MP0_CPUTOP_PWR_CON) | SRAM_SLEEP_B);
		/* TINFO="Wait until MP0_CPUTOP_SRAM_SLEEP_B_ACK = 1" */
		while (!(mcu_spm_read(MP0_CPUTOP_PWR_CON) & SRAM_SLEEP_B_ACK)) {
			/* SRAM SLEEP delay IP clock is 26MHz. Print SRAM / MTCMOS control and PWR_ACK for debug. */
		}
		/* TINFO="Delay 1us" */
		udelay(1);
		/* TINFO="Set SRAM_ISOINT_B = 1" */
		mcu_spm_write(MP0_CPUTOP_PWR_CON, mcu_spm_read(MP0_CPUTOP_PWR_CON) | SRAM_ISOINT_B);
		/* TINFO="Delay 1us" */
		udelay(1);
		/* TINFO="Set SRAM_CKISO = 0" */
		mcu_spm_write(MP0_CPUTOP_PWR_CON, mcu_spm_read(MP0_CPUTOP_PWR_CON) & ~SRAM_CKISO);
		/* TINFO="Set PWR_CLK_DIS = 0" */
		mcu_spm_write(MP0_CPUTOP_PWR_CON, mcu_spm_read(MP0_CPUTOP_PWR_CON) & ~PWR_CLK_DIS);
		/* TINFO="Set PWR_RST_B = 1" */
		mcu_spm_write(MP0_CPUTOP_PWR_CON, mcu_spm_read(MP0_CPUTOP_PWR_CON) | PWR_RST_B);
		/* TINFO="Release bus protect" */
		/* TINFO="Release ADB pwrdnreqn for Cluster 0" */
		*INFRA_TOPAXI_PROTECTEN_1 &= 0xffffffff ^ mp0_mask;
		/* TINFO="Wait ADB ~pwrdnreqn for Cluster 0" */
		while ((*INFRA_TOPAXI_PROTECTSTA1_1 & mp0_mask) != 0);
		/* TINFO="Finish to turn on MP0_CPUTOP" */
	}
	return err;
}
int spm_mtcmos_ctrl_mp0_cputop_shut_down(int state)
{
	int err = 0;
	const unsigned int mp0_mask =
		(1 << IDX_PROTECT_ICC0_CACTIVE) |
		(1 << IDX_PROTECT_ICD0_CACTIVE) |
		(1 << IDX_PROTECT_MP0_CACTIVE) |
		(1 << IDX_PROTECT_L2C0_CACTIVE);
	/* TINFO="enable SPM register control" */
	mcu_spm_write(POWERON_CONFIG_EN, (SPM_PROJECT_CODE << 16) | (0x1 << 0));
	if (state == STA_POWER_DOWN) {
		/* TINFO="Start to turn off MP0_CPUTOP" */
		/* TINFO="Set STANDBYWFIL2 for Cluster 0" */
		*MP0_AXI_CONFIG |= 0x00000010;
		*(volatile unsigned int *)(0x10394000) &= 0xfffffffc;
		*(volatile unsigned int *)(0x10395000) &= 0xfffffffc;
		/* TINFO="Wait STANDBYWFIL2 for Cluster 0" */
		while ((*MP0_CA7_MISC_CONFIG & (1 << 28)) != (1 << 28));
		/* TINFO="Set ADB pwrdnreqn for Cluster 0" */
		*INFRA_TOPAXI_PROTECTEN_1 |= mp0_mask;
		/* TINFO="Wait ADB pwrdnreqn for Cluster 0" */
		while ((*INFRA_TOPAXI_PROTECTSTA1_1 & mp0_mask) != mp0_mask);
		/* TINFO="Set PWR_ISO = 1" */
		mcu_spm_write(MP0_CPUTOP_PWR_CON, mcu_spm_read(MP0_CPUTOP_PWR_CON) | PWR_ISO);
		/* TINFO="Set SRAM_CKISO = 1" */
		mcu_spm_write(MP0_CPUTOP_PWR_CON, mcu_spm_read(MP0_CPUTOP_PWR_CON) | SRAM_CKISO);
		/* TINFO="Set SRAM_ISOINT_B = 0" */
		mcu_spm_write(MP0_CPUTOP_PWR_CON, mcu_spm_read(MP0_CPUTOP_PWR_CON) & ~SRAM_ISOINT_B);
		/* TINFO="Set SRAM_PDN = 1" */
		mcu_spm_write(MP0_CPUTOP_PWR_CON, mcu_spm_read(MP0_CPUTOP_PWR_CON) | MP0_CPUTOP_SRAM_PDN);
		/* TINFO="Wait until MP0_CPUTOP_SRAM_PDN_ACK = 1" */
		while (!(mcu_spm_read(MP0_CPUTOP_PWR_CON) & MP0_CPUTOP_SRAM_PDN_ACK)) {
			/* SRAM PDN delay IP clock is 26MHz. Print SRAM control and ACK for debug. */
		}
		/* TINFO="Set PWR_RST_B = 0" */
		mcu_spm_write(MP0_CPUTOP_PWR_CON, mcu_spm_read(MP0_CPUTOP_PWR_CON) & ~PWR_RST_B);
		/* TINFO="Set PWR_CLK_DIS = 1" */
		mcu_spm_write(MP0_CPUTOP_PWR_CON, mcu_spm_read(MP0_CPUTOP_PWR_CON) | PWR_CLK_DIS);
		/* TINFO="Set PWR_ON = 0" */
		mcu_spm_write(MP0_CPUTOP_PWR_CON, mcu_spm_read(MP0_CPUTOP_PWR_CON) & ~PWR_ON);
		/* TINFO="Set PWR_ON_2ND = 0" */
		mcu_spm_write(MP0_CPUTOP_PWR_CON, mcu_spm_read(MP0_CPUTOP_PWR_CON) & ~PWR_ON_2ND);
#ifndef IGNORE_PWR_ACK
		/* TINFO="Wait until PWR_STATUS = 0 and PWR_STATUS_2ND = 0" */
		while ((mcu_spm_read(PWR_STATUS) & MP0_CPUTOP_PWR_STA_MASK)
		       || (mcu_spm_read(PWR_STATUS_2ND) & MP0_CPUTOP_PWR_STA_MASK)) {
			/* No logic between pwr_on and pwr_ack. Print SRAM / MTCMOS control and PWR_ACK for debug. */
		}
#endif
		/* TINFO="Finish to turn off MP0_CPUTOP" */
	} else {    /* STA_POWER_ON */
		/* TINFO="Start to turn on MP0_CPUTOP" */
		/* TINFO="Set PWR_ON = 1" */
		mcu_spm_write(MP0_CPUTOP_PWR_CON, mcu_spm_read(MP0_CPUTOP_PWR_CON) | PWR_ON);
		/* TINFO="Set PWR_ON_2ND = 1" */
		mcu_spm_write(MP0_CPUTOP_PWR_CON, mcu_spm_read(MP0_CPUTOP_PWR_CON) | PWR_ON_2ND);
#ifndef IGNORE_PWR_ACK
		/* TINFO="Wait until PWR_STATUS = 1 and PWR_STATUS_2ND = 1" */
		while (!(mcu_spm_read(PWR_STATUS) & MP0_CPUTOP_PWR_STA_MASK)
		       || !(mcu_spm_read(PWR_STATUS_2ND) & MP0_CPUTOP_PWR_STA_MASK)) {
			/* No logic between pwr_on and pwr_ack. Print SRAM / MTCMOS control and PWR_ACK for debug. */
		}
#endif
		/* TINFO="Set PWR_ISO = 0" */
		mcu_spm_write(MP0_CPUTOP_PWR_CON, mcu_spm_read(MP0_CPUTOP_PWR_CON) & ~PWR_ISO);
		/* TINFO="Set SRAM_PDN = 0" */
		mcu_spm_write(MP0_CPUTOP_PWR_CON, mcu_spm_read(MP0_CPUTOP_PWR_CON) & ~(0x1 << 8));
		/* TINFO="Wait until MP0_CPUTOP_SRAM_PDN_ACK = 0" */
		while (mcu_spm_read(MP0_CPUTOP_PWR_CON) & MP0_CPUTOP_SRAM_PDN_ACK) {
			/* SRAM PDN delay IP clock is 26MHz. Print SRAM control and ACK for debug. */
		}
		/* TINFO="Delay 1us" */
		udelay(1);
		/* TINFO="Set SRAM_ISOINT_B = 1" */
		mcu_spm_write(MP0_CPUTOP_PWR_CON, mcu_spm_read(MP0_CPUTOP_PWR_CON) | SRAM_ISOINT_B);
		/* TINFO="Delay 1us" */
		udelay(1);
		/* TINFO="Set SRAM_CKISO = 0" */
		mcu_spm_write(MP0_CPUTOP_PWR_CON, mcu_spm_read(MP0_CPUTOP_PWR_CON) & ~SRAM_CKISO);
		/* TINFO="Set PWR_CLK_DIS = 0" */
		mcu_spm_write(MP0_CPUTOP_PWR_CON, mcu_spm_read(MP0_CPUTOP_PWR_CON) & ~PWR_CLK_DIS);
		/* TINFO="Set PWR_RST_B = 1" */
		mcu_spm_write(MP0_CPUTOP_PWR_CON, mcu_spm_read(MP0_CPUTOP_PWR_CON) | PWR_RST_B);
		/* TINFO="Release bus protect" */
		/* TINFO="Release ADB pwrdnreqn for Cluster 0" */
		*INFRA_TOPAXI_PROTECTEN_1 &= 0xffffffff ^ mp0_mask;
		/* TINFO="Wait ADB ~pwrdnreqn for Cluster 0" */
		while ((*INFRA_TOPAXI_PROTECTSTA1_1 & mp0_mask) != 0);
		/* TINFO="Finish to turn on MP0_CPUTOP" */
	}
	return err;
}
int spm_mtcmos_ctrl_mp0_cpu0(int state)
{
	int err = 0;
	int i = 0;
	/* TINFO="enable SPM register control" */
	mcu_spm_write(POWERON_CONFIG_EN, (SPM_PROJECT_CODE << 16) | (0x1 << 0));
	if (state == STA_POWER_DOWN) {
		/* TINFO="Start to turn off MP0_CPU0" */
		/* TINFO="Set PWR_ISO = 1" */
		mcu_spm_write(MP0_CPU0_PWR_CON, mcu_spm_read(MP0_CPU0_PWR_CON) | PWR_ISO);
		/* TINFO="Set SRAM_ISOINT_B = 0" */
		mcu_spm_write(MP0_CPU0_PWR_CON, mcu_spm_read(MP0_CPU0_PWR_CON) & ~SRAM_ISOINT_B);
		/* TINFO="Set SRAM_CKISO = 1" */
		mcu_spm_write(MP0_CPU0_PWR_CON, mcu_spm_read(MP0_CPU0_PWR_CON) | SRAM_CKISO);
		/* TINFO="Set PWR_RST_B = 0" */
		mcu_spm_write(MP0_CPU0_PWR_CON, mcu_spm_read(MP0_CPU0_PWR_CON) & ~PWR_RST_B);
		/* TINFO="Set PWR_CLK_DIS = 1" */
		mcu_spm_write(MP0_CPU0_PWR_CON, mcu_spm_read(MP0_CPU0_PWR_CON) | PWR_CLK_DIS);
		/* TINFO="Set PWR_ON = 0" */
		mcu_spm_write(MP0_CPU0_PWR_CON, mcu_spm_read(MP0_CPU0_PWR_CON) & ~PWR_ON);
		/* TINFO="Set PWR_ON_2ND = 0" */
		mcu_spm_write(MP0_CPU0_PWR_CON, mcu_spm_read(MP0_CPU0_PWR_CON) & ~PWR_ON_2ND);
		/* TINFO="Set SRAM_SLPB = 0" */
		/* mcu_spm_write(MP0_CPU0_PWR_CON, mcu_spm_read(MP0_CPU0_PWR_CON) & ~SRAM_SLEEP_B); */
#ifndef IGNORE_PWR_ACK
		/* TINFO="Wait until PWR_STATUS = 0 and PWR_STATUS_2ND = 0" */
		while ((mcu_spm_read(PWR_STATUS) & MP0_CPU0_PWR_STA_MASK)
		       || (mcu_spm_read(PWR_STATUS_2ND) & MP0_CPU0_PWR_STA_MASK)) {
			/* No logic between pwr_on and pwr_ack. Print SRAM / MTCMOS control and PWR_ACK for debug. */
		}
#endif
		/* TINFO="Set SRAM_PDN = 1" */
		mcu_spm_write(MP0_CPU0_PWR_CON, mcu_spm_read(MP0_CPU0_PWR_CON) | SRAM_PDN);
		/* TINFO="Wait until MP0_CPU0_SRAM_PDN_ACK = 1" */
		while (!(mcu_spm_read(MP0_CPU0_PWR_CON) & SRAM_PDN_ACK)) {
			/* SRAM PDN delay IP clock is 26MHz. Print SRAM control and ACK for debug. */
		}
		/* TINFO="Set SRAM_PD_SLPB_CLAMP = 1" */
		mcu_spm_write(MP0_CPU0_PWR_CON, mcu_spm_read(MP0_CPU0_PWR_CON) | SRAM_PD_SLPB_CLAMP);
		/* TINFO="Finish to turn off MP0_CPU0" */
	} else {    /* STA_POWER_ON */
		/* TINFO="Start to turn on MP0_CPU0" */
		/* TINFO="Set SRAM_SLPB = 0" */
		mcu_spm_write(MP0_CPU0_PWR_CON, mcu_spm_read(MP0_CPU0_PWR_CON) & ~SRAM_SLEEP_B);
		/* TINFO="Set PWR_RST_B = 0" */
		mcu_spm_write(MP0_CPU0_PWR_CON, mcu_spm_read(MP0_CPU0_PWR_CON) & ~PWR_RST_B);
		/* TINFO="Set PWR_CLK_DIS = 1" */
		mcu_spm_write(MP0_CPU0_PWR_CON, mcu_spm_read(MP0_CPU0_PWR_CON) | PWR_CLK_DIS);
		/* TINFO="Set SRAM_PDN = 1" */
		mcu_spm_write(MP0_CPU0_PWR_CON, mcu_spm_read(MP0_CPU0_PWR_CON) | SRAM_PDN);
		/* TINFO="Wait until MP0_CPU0_SRAM_PDN_ACK = 1" */
		/* while (!(mcu_spm_read(MP0_CPU0_PWR_CON) & SRAM_PDN_ACK)) { */
			/* SRAM PDN delay IP clock is 26MHz. Print SRAM control and ACK for debug. */
		/* } */
		for (i = 0; i < 100; i++);
		/* TINFO="Set SRAM_PD_SLPB_CLAMP = 0" */
		mcu_spm_write(MP0_CPU0_PWR_CON, mcu_spm_read(MP0_CPU0_PWR_CON) & ~SRAM_PD_SLPB_CLAMP);
		/* TINFO="Set SRAM_PDN = 0" */
		mcu_spm_write(MP0_CPU0_PWR_CON, mcu_spm_read(MP0_CPU0_PWR_CON) & ~(0x1 << 8));
		/* TINFO="Wait until MP0_CPU0_SRAM_PDN_ACK = 0" */
		while (mcu_spm_read(MP0_CPU0_PWR_CON) & SRAM_PDN_ACK) {
			/* SRAM PDN delay IP clock is 26MHz. Print SRAM control and ACK for debug. */
		}
		/* TINFO="Set PWR_ON = 1" */
		mcu_spm_write(MP0_CPU0_PWR_CON, mcu_spm_read(MP0_CPU0_PWR_CON) | PWR_ON);
		/* TINFO="Set PWR_ON_2ND = 1" */
		mcu_spm_write(MP0_CPU0_PWR_CON, mcu_spm_read(MP0_CPU0_PWR_CON) | PWR_ON_2ND);
#ifndef IGNORE_PWR_ACK
		/* TINFO="Wait until PWR_STATUS = 1 and PWR_STATUS_2ND = 1" */
		while (!(mcu_spm_read(PWR_STATUS) & MP0_CPU0_PWR_STA_MASK)
		       || !(mcu_spm_read(PWR_STATUS_2ND) & MP0_CPU0_PWR_STA_MASK)) {
			/* No logic between pwr_on and pwr_ack. Print SRAM / MTCMOS control and PWR_ACK for debug. */
		}
#endif
		/* TINFO="Set SRAM_SLPB = 1" */
		mcu_spm_write(MP0_CPU0_PWR_CON, mcu_spm_read(MP0_CPU0_PWR_CON) | SRAM_SLEEP_B);
		/* TINFO="Set SRAM_ISOINT_B = 1" */
		mcu_spm_write(MP0_CPU0_PWR_CON, mcu_spm_read(MP0_CPU0_PWR_CON) | SRAM_ISOINT_B);
		/* TINFO="Set PWR_ISO = 0" */
		mcu_spm_write(MP0_CPU0_PWR_CON, mcu_spm_read(MP0_CPU0_PWR_CON) & ~PWR_ISO);
		/* TINFO="Delay 1us" */
		udelay(1);
		/* TINFO="Set SRAM_CKISO = 0" */
		mcu_spm_write(MP0_CPU0_PWR_CON, mcu_spm_read(MP0_CPU0_PWR_CON) & ~SRAM_CKISO);
		/* TINFO="Set PWR_CLK_DIS = 0" */
		mcu_spm_write(MP0_CPU0_PWR_CON, mcu_spm_read(MP0_CPU0_PWR_CON) & ~PWR_CLK_DIS);
		/* TINFO="Set PWR_RST_B = 1" */
		mcu_spm_write(MP0_CPU0_PWR_CON, mcu_spm_read(MP0_CPU0_PWR_CON) | PWR_RST_B);
		/* TINFO="Finish to turn on MP0_CPU0" */
	}
	return err;
}
int spm_mtcmos_ctrl_mp0_cpu1(int state)
{
	int err = 0;
	/* TINFO="enable SPM register control" */
	mcu_spm_write(POWERON_CONFIG_EN, (SPM_PROJECT_CODE << 16) | (0x1 << 0));
	if (state == STA_POWER_DOWN) {
		/* TINFO="Start to turn off MP0_CPU1" */
		/* TINFO="Set PWR_ISO = 1" */
		mcu_spm_write(MP0_CPU1_PWR_CON, mcu_spm_read(MP0_CPU1_PWR_CON) | PWR_ISO);
		/* TINFO="Set SRAM_ISOINT_B = 0" */
		mcu_spm_write(MP0_CPU1_PWR_CON, mcu_spm_read(MP0_CPU1_PWR_CON) & ~SRAM_ISOINT_B);
		/* TINFO="Set SRAM_CKISO = 1" */
		mcu_spm_write(MP0_CPU1_PWR_CON, mcu_spm_read(MP0_CPU1_PWR_CON) | SRAM_CKISO);
		/* TINFO="Set PWR_RST_B = 0" */
		mcu_spm_write(MP0_CPU1_PWR_CON, mcu_spm_read(MP0_CPU1_PWR_CON) & ~PWR_RST_B);
		/* TINFO="Set PWR_CLK_DIS = 1" */
		mcu_spm_write(MP0_CPU1_PWR_CON, mcu_spm_read(MP0_CPU1_PWR_CON) | PWR_CLK_DIS);
		/* TINFO="Set PWR_ON = 0" */
		mcu_spm_write(MP0_CPU1_PWR_CON, mcu_spm_read(MP0_CPU1_PWR_CON) & ~PWR_ON);
		/* TINFO="Set PWR_ON_2ND = 0" */
		mcu_spm_write(MP0_CPU1_PWR_CON, mcu_spm_read(MP0_CPU1_PWR_CON) & ~PWR_ON_2ND);
		/* TINFO="Set SRAM_SLPB = 0" */
		/* mcu_spm_write(MP0_CPU1_PWR_CON, mcu_spm_read(MP0_CPU1_PWR_CON) & ~SRAM_SLEEP_B); */
#ifndef IGNORE_PWR_ACK
		/* TINFO="Wait until PWR_STATUS = 0 and PWR_STATUS_2ND = 0" */
		while ((mcu_spm_read(PWR_STATUS) & MP0_CPU1_PWR_STA_MASK)
		       || (mcu_spm_read(PWR_STATUS_2ND) & MP0_CPU1_PWR_STA_MASK)) {
				/* No logic between pwr_on and pwr_ack. Print SRAM / MTCMOS control and PWR_ACK for debug. */
		}
#endif
		/* TINFO="Set SRAM_PDN = 1" */
		mcu_spm_write(MP0_CPU1_PWR_CON, mcu_spm_read(MP0_CPU1_PWR_CON) | SRAM_PDN);
		/* TINFO="Wait until MP0_CPU1_SRAM_PDN_ACK = 1" */
		while (!(mcu_spm_read(MP0_CPU1_PWR_CON) & SRAM_PDN_ACK)) {
			/* SRAM PDN delay IP clock is 26MHz. Print SRAM control and ACK for debug. */
		}
		/* TINFO="Set SRAM_PD_SLPB_CLAMP = 1" */
		mcu_spm_write(MP0_CPU1_PWR_CON, mcu_spm_read(MP0_CPU1_PWR_CON) | SRAM_PD_SLPB_CLAMP);
		/* TINFO="Finish to turn off MP0_CPU1" */
	} else {    /* STA_POWER_ON */
		/* TINFO="Start to turn on MP0_CPU1" */
		/* TINFO="Set SRAM_SLPB = 0" */
		mcu_spm_write(MP0_CPU1_PWR_CON, mcu_spm_read(MP0_CPU1_PWR_CON) & ~SRAM_SLEEP_B);
		/* TINFO="Set PWR_RST_B = 0" */
		mcu_spm_write(MP0_CPU1_PWR_CON, mcu_spm_read(MP0_CPU1_PWR_CON) & ~PWR_RST_B);
		/* TINFO="Set PWR_CLK_DIS = 1" */
		mcu_spm_write(MP0_CPU1_PWR_CON, mcu_spm_read(MP0_CPU1_PWR_CON) | PWR_CLK_DIS);
		/* TINFO="Set SRAM_PDN = 1" */
		mcu_spm_write(MP0_CPU1_PWR_CON, mcu_spm_read(MP0_CPU1_PWR_CON) | SRAM_PDN);
		#ifndef DDR_RESERVE_ENABLE
		/* TINFO="Wait until MP0_CPU0_SRAM_PDN_ACK = 1" */
		while (!(mcu_spm_read(MP0_CPU1_PWR_CON) & SRAM_PDN_ACK)) {
			/* SRAM PDN delay IP clock is 26MHz. Print SRAM control and ACK for debug. */
		}
		#endif
		/* TINFO="Set SRAM_PD_SLPB_CLAMP = 0" */
		mcu_spm_write(MP0_CPU1_PWR_CON, mcu_spm_read(MP0_CPU1_PWR_CON) & ~SRAM_PD_SLPB_CLAMP);
		/* TINFO="Set SRAM_PDN = 0" */
		mcu_spm_write(MP0_CPU1_PWR_CON, mcu_spm_read(MP0_CPU1_PWR_CON) & ~(0x1 << 8));
		/* TINFO="Wait until MP0_CPU0_SRAM_PDN_ACK = 0" */
		while (mcu_spm_read(MP0_CPU1_PWR_CON) & SRAM_PDN_ACK) {
			/* SRAM PDN delay IP clock is 26MHz. Print SRAM control and ACK for debug. */
		}
		/* TINFO="Set PWR_ON = 1" */
		mcu_spm_write(MP0_CPU1_PWR_CON, mcu_spm_read(MP0_CPU1_PWR_CON) | PWR_ON);
		/* TINFO="Set PWR_ON_2ND = 1" */
		mcu_spm_write(MP0_CPU1_PWR_CON, mcu_spm_read(MP0_CPU1_PWR_CON) | PWR_ON_2ND);
#ifndef IGNORE_PWR_ACK
		/* TINFO="Wait until PWR_STATUS = 1 and PWR_STATUS_2ND = 1" */
		while (!(mcu_spm_read(PWR_STATUS) & MP0_CPU1_PWR_STA_MASK)
		       || !(mcu_spm_read(PWR_STATUS_2ND) & MP0_CPU1_PWR_STA_MASK)) {
			/* No logic between pwr_on and pwr_ack. Print SRAM / MTCMOS control and PWR_ACK for debug. */
		}
#endif
		/* TINFO="Set SRAM_SLPB = 1" */
		mcu_spm_write(MP0_CPU1_PWR_CON, mcu_spm_read(MP0_CPU1_PWR_CON) | SRAM_SLEEP_B);
		/* TINFO="Set SRAM_ISOINT_B = 1" */
		mcu_spm_write(MP0_CPU1_PWR_CON, mcu_spm_read(MP0_CPU1_PWR_CON) | SRAM_ISOINT_B);
		/* TINFO="Set PWR_ISO = 0" */
		mcu_spm_write(MP0_CPU1_PWR_CON, mcu_spm_read(MP0_CPU1_PWR_CON) & ~PWR_ISO);
		/* TINFO="Delay 1us" */
		udelay(1);
		/* TINFO="Set SRAM_CKISO = 0" */
		mcu_spm_write(MP0_CPU1_PWR_CON, mcu_spm_read(MP0_CPU1_PWR_CON) & ~SRAM_CKISO);
		/* TINFO="Set PWR_CLK_DIS = 0" */
		mcu_spm_write(MP0_CPU1_PWR_CON, mcu_spm_read(MP0_CPU1_PWR_CON) & ~PWR_CLK_DIS);
		/* TINFO="Set PWR_RST_B = 1" */
		mcu_spm_write(MP0_CPU1_PWR_CON, mcu_spm_read(MP0_CPU1_PWR_CON) | PWR_RST_B);
		/* TINFO="Finish to turn on MP0_CPU1" */
	}
	return err;
}
int spm_mtcmos_ctrl_mp0_cpu2(int state)
{
	int err = 0;
	/* TINFO="enable SPM register control" */
	mcu_spm_write(POWERON_CONFIG_EN, (SPM_PROJECT_CODE << 16) | (0x1 << 0));
	if (state == STA_POWER_DOWN) {
		/* TINFO="Start to turn off MP0_CPU2" */
		/* TINFO="Set PWR_ISO = 1" */
		mcu_spm_write(MP0_CPU2_PWR_CON, mcu_spm_read(MP0_CPU2_PWR_CON) | PWR_ISO);
		/* TINFO="Set SRAM_ISOINT_B = 0" */
		mcu_spm_write(MP0_CPU2_PWR_CON, mcu_spm_read(MP0_CPU2_PWR_CON) & ~SRAM_ISOINT_B);
		/* TINFO="Set SRAM_CKISO = 1" */
		mcu_spm_write(MP0_CPU2_PWR_CON, mcu_spm_read(MP0_CPU2_PWR_CON) | SRAM_CKISO);
		/* TINFO="Set PWR_RST_B = 0" */
		mcu_spm_write(MP0_CPU2_PWR_CON, mcu_spm_read(MP0_CPU2_PWR_CON) & ~PWR_RST_B);
		/* TINFO="Set PWR_CLK_DIS = 1" */
		mcu_spm_write(MP0_CPU2_PWR_CON, mcu_spm_read(MP0_CPU2_PWR_CON) | PWR_CLK_DIS);
		/* TINFO="Set PWR_ON = 0" */
		mcu_spm_write(MP0_CPU2_PWR_CON, mcu_spm_read(MP0_CPU2_PWR_CON) & ~PWR_ON);
		/* TINFO="Set PWR_ON_2ND = 0" */
		mcu_spm_write(MP0_CPU2_PWR_CON, mcu_spm_read(MP0_CPU2_PWR_CON) & ~PWR_ON_2ND);
		/* TINFO="Set SRAM_SLPB = 0" */
		/* mcu_spm_write(MP0_CPU2_PWR_CON, mcu_spm_read(MP0_CPU2_PWR_CON) & ~SRAM_SLEEP_B); */
#ifndef IGNORE_PWR_ACK
		/* TINFO="Wait until PWR_STATUS = 0 and PWR_STATUS_2ND = 0" */
		while ((mcu_spm_read(PWR_STATUS) & MP0_CPU2_PWR_STA_MASK)
		       || (mcu_spm_read(PWR_STATUS_2ND) & MP0_CPU2_PWR_STA_MASK)) {
			/* No logic between pwr_on and pwr_ack. Print SRAM / MTCMOS control and PWR_ACK for debug. */
		}
#endif
		/* TINFO="Set SRAM_PDN = 1" */
		mcu_spm_write(MP0_CPU2_PWR_CON, mcu_spm_read(MP0_CPU2_PWR_CON) | MP0_CPU2_SRAM_PDN);
		/* TINFO="Wait until MP0_CPU2_SRAM_PDN_ACK = 1" */
		while (!(mcu_spm_read(MP0_CPU2_PWR_CON) & SRAM_PDN_ACK)) {
			/* SRAM PDN delay IP clock is 26MHz. Print SRAM control and ACK for debug. */
		}
		/* TINFO="Set SRAM_PD_SLPB_CLAMP = 1" */
		mcu_spm_write(MP0_CPU2_PWR_CON, mcu_spm_read(MP0_CPU2_PWR_CON) | SRAM_PD_SLPB_CLAMP);
		/* TINFO="Finish to turn off MP0_CPU2" */
	} else {    /* STA_POWER_ON */
		/* TINFO="Start to turn on MP0_CPU2" */
		/* TINFO="Set SRAM_SLPB = 0" */
		mcu_spm_write(MP0_CPU2_PWR_CON, mcu_spm_read(MP0_CPU2_PWR_CON) & ~SRAM_SLEEP_B);
		/* TINFO="Set PWR_RST_B = 0" */
		mcu_spm_write(MP0_CPU2_PWR_CON, mcu_spm_read(MP0_CPU2_PWR_CON) & ~PWR_RST_B);
		/* TINFO="Set PWR_CLK_DIS = 1" */
		mcu_spm_write(MP0_CPU2_PWR_CON, mcu_spm_read(MP0_CPU2_PWR_CON) | PWR_CLK_DIS);
		/* TINFO="Set SRAM_PDN = 1" */
		mcu_spm_write(MP0_CPU2_PWR_CON, mcu_spm_read(MP0_CPU2_PWR_CON) | MP0_CPU2_SRAM_PDN);
		#ifndef DDR_RESERVE_ENABLE
		/* TINFO="Wait until MP0_CPU2_SRAM_PDN_ACK = 1" */
		while (!(mcu_spm_read(MP0_CPU2_PWR_CON) & SRAM_PDN_ACK)) {
			/* SRAM PDN delay IP clock is 26MHz. Print SRAM control and ACK for debug. */
		}
		#endif
		/* TINFO="Set SRAM_PD_SLPB_CLAMP = 0" */
		mcu_spm_write(MP0_CPU2_PWR_CON, mcu_spm_read(MP0_CPU2_PWR_CON) & ~SRAM_PD_SLPB_CLAMP);
		/* TINFO="Set SRAM_PDN = 0" */
		mcu_spm_write(MP0_CPU2_PWR_CON, mcu_spm_read(MP0_CPU2_PWR_CON) & ~(0x1 << 8));
		/* TINFO="Wait until MP0_CPU2_SRAM_PDN_ACK = 0" */
		while (mcu_spm_read(MP0_CPU2_PWR_CON) & MP0_CPU2_SRAM_PDN_ACK) {
			/* SRAM PDN delay IP clock is 26MHz. Print SRAM control and ACK for debug. */
		}
		/* TINFO="Set PWR_ON = 1" */
		mcu_spm_write(MP0_CPU2_PWR_CON, mcu_spm_read(MP0_CPU2_PWR_CON) | PWR_ON);
		/* TINFO="Set PWR_ON_2ND = 1" */
		mcu_spm_write(MP0_CPU2_PWR_CON, mcu_spm_read(MP0_CPU2_PWR_CON) | PWR_ON_2ND);
#ifndef IGNORE_PWR_ACK
		/* TINFO="Wait until PWR_STATUS = 1 and PWR_STATUS_2ND = 1" */
		while (!(mcu_spm_read(PWR_STATUS) & MP0_CPU2_PWR_STA_MASK)
		       || !(mcu_spm_read(PWR_STATUS_2ND) & MP0_CPU2_PWR_STA_MASK)) {
			/* No logic between pwr_on and pwr_ack. Print SRAM / MTCMOS control and PWR_ACK for debug. */
		}
#endif
		/* TINFO="Set SRAM_SLPB = 1" */
		mcu_spm_write(MP0_CPU2_PWR_CON, mcu_spm_read(MP0_CPU2_PWR_CON) | SRAM_SLEEP_B);
		/* TINFO="Set SRAM_ISOINT_B = 1" */
		mcu_spm_write(MP0_CPU2_PWR_CON, mcu_spm_read(MP0_CPU2_PWR_CON) | SRAM_ISOINT_B);
		/* TINFO="Set PWR_ISO = 0" */
		mcu_spm_write(MP0_CPU2_PWR_CON, mcu_spm_read(MP0_CPU2_PWR_CON) & ~PWR_ISO);
		/* TINFO="Delay 1us" */
		udelay(1);
		/* TINFO="Set SRAM_CKISO = 0" */
		mcu_spm_write(MP0_CPU2_PWR_CON, mcu_spm_read(MP0_CPU2_PWR_CON) & ~SRAM_CKISO);
		/* TINFO="Set PWR_CLK_DIS = 0" */
		mcu_spm_write(MP0_CPU2_PWR_CON, mcu_spm_read(MP0_CPU2_PWR_CON) & ~PWR_CLK_DIS);
		/* TINFO="Set PWR_RST_B = 1" */
		mcu_spm_write(MP0_CPU2_PWR_CON, mcu_spm_read(MP0_CPU2_PWR_CON) | PWR_RST_B);
		/* TINFO="Finish to turn on MP0_CPU2" */
	}
	return err;
}
int spm_mtcmos_ctrl_mp0_cpu3(int state)
{
	int err = 0;
	/* TINFO="enable SPM register control" */
	mcu_spm_write(POWERON_CONFIG_EN, (SPM_PROJECT_CODE << 16) | (0x1 << 0));
	if (state == STA_POWER_DOWN) {
		/* TINFO="Start to turn off MP0_CPU3" */
		/* TINFO="Set PWR_ISO = 1" */
		mcu_spm_write(MP0_CPU3_PWR_CON, mcu_spm_read(MP0_CPU3_PWR_CON) | PWR_ISO);
		/* TINFO="Set SRAM_ISOINT_B = 0" */
		mcu_spm_write(MP0_CPU3_PWR_CON, mcu_spm_read(MP0_CPU3_PWR_CON) & ~SRAM_ISOINT_B);
		/* TINFO="Set SRAM_CKISO = 1" */
		mcu_spm_write(MP0_CPU3_PWR_CON, mcu_spm_read(MP0_CPU3_PWR_CON) | SRAM_CKISO);
		/* TINFO="Set PWR_RST_B = 0" */
		mcu_spm_write(MP0_CPU3_PWR_CON, mcu_spm_read(MP0_CPU3_PWR_CON) & ~PWR_RST_B);
		/* TINFO="Set PWR_CLK_DIS = 1" */
		mcu_spm_write(MP0_CPU3_PWR_CON, mcu_spm_read(MP0_CPU3_PWR_CON) | PWR_CLK_DIS);
		/* TINFO="Set PWR_ON = 0" */
		mcu_spm_write(MP0_CPU3_PWR_CON, mcu_spm_read(MP0_CPU3_PWR_CON) & ~PWR_ON);
		/* TINFO="Set PWR_ON_2ND = 0" */
		mcu_spm_write(MP0_CPU3_PWR_CON, mcu_spm_read(MP0_CPU3_PWR_CON) & ~PWR_ON_2ND);
		/* TINFO="Set SRAM_SLPB = 0" */
		/* mcu_spm_write(MP0_CPU3_PWR_CON, mcu_spm_read(MP0_CPU3_PWR_CON) & ~SRAM_SLEEP_B); */
#ifndef IGNORE_PWR_ACK
		/* TINFO="Wait until PWR_STATUS = 0 and PWR_STATUS_2ND = 0" */
		while ((mcu_spm_read(PWR_STATUS) & MP0_CPU3_PWR_STA_MASK)
		       || (mcu_spm_read(PWR_STATUS_2ND) & MP0_CPU3_PWR_STA_MASK)) {
			/* No logic between pwr_on and pwr_ack. Print SRAM / MTCMOS control and PWR_ACK for debug. */
		}
#endif
		/* TINFO="Set SRAM_PDN = 1" */
		mcu_spm_write(MP0_CPU3_PWR_CON, mcu_spm_read(MP0_CPU3_PWR_CON) | MP0_CPU3_SRAM_PDN);
		/* TINFO="Wait until MP0_CPU3_SRAM_PDN_ACK = 1" */
		while (!(mcu_spm_read(MP0_CPU3_PWR_CON) & SRAM_PDN_ACK)) {
			/* SRAM PDN delay IP clock is 26MHz. Print SRAM control and ACK for debug. */
		}
		/* TINFO="Set SRAM_PD_SLPB_CLAMP = 1" */
		mcu_spm_write(MP0_CPU3_PWR_CON, mcu_spm_read(MP0_CPU3_PWR_CON) | SRAM_PD_SLPB_CLAMP);
		/* TINFO="Finish to turn off MP0_CPU3" */
	} else {    /* STA_POWER_ON */
		/* TINFO="Start to turn on MP0_CPU3" */
		/* TINFO="Set SRAM_SLPB = 0" */
		mcu_spm_write(MP0_CPU3_PWR_CON, mcu_spm_read(MP0_CPU3_PWR_CON) & ~SRAM_SLEEP_B);
		/* TINFO="Set PWR_RST_B = 0" */
		mcu_spm_write(MP0_CPU3_PWR_CON, mcu_spm_read(MP0_CPU3_PWR_CON) & ~PWR_RST_B);
		/* TINFO="Set PWR_CLK_DIS = 1" */
		mcu_spm_write(MP0_CPU3_PWR_CON, mcu_spm_read(MP0_CPU3_PWR_CON) | PWR_CLK_DIS);
		/* TINFO="Set SRAM_PDN = 1" */
		mcu_spm_write(MP0_CPU3_PWR_CON, mcu_spm_read(MP0_CPU3_PWR_CON) | MP0_CPU3_SRAM_PDN);
		#ifndef DDR_RESERVE_ENABLE
		/* TINFO="Wait until MP0_CPU3_SRAM_PDN_ACK = 1" */
		while (!(mcu_spm_read(MP0_CPU3_PWR_CON) & MP0_CPU3_SRAM_PDN_ACK)) {
			/* SRAM PDN delay IP clock is 26MHz. Print SRAM control and ACK for debug. */
		}
		#endif
		/* TINFO="Set SRAM_PD_SLPB_CLAMP = 0" */
		mcu_spm_write(MP0_CPU3_PWR_CON, mcu_spm_read(MP0_CPU3_PWR_CON) & ~SRAM_PD_SLPB_CLAMP);
		/* TINFO="Set SRAM_PDN = 0" */
		mcu_spm_write(MP0_CPU3_PWR_CON, mcu_spm_read(MP0_CPU3_PWR_CON) & ~(0x1 << 8));
		/* TINFO="Wait until MP0_CPU3_SRAM_PDN_ACK = 0" */
		while (mcu_spm_read(MP0_CPU3_PWR_CON) & MP0_CPU3_SRAM_PDN_ACK) {
			/* SRAM PDN delay IP clock is 26MHz. Print SRAM control and ACK for debug. */
		}
		/* TINFO="Set PWR_ON = 1" */
		mcu_spm_write(MP0_CPU3_PWR_CON, mcu_spm_read(MP0_CPU3_PWR_CON) | PWR_ON);
		/* TINFO="Set PWR_ON_2ND = 1" */
		mcu_spm_write(MP0_CPU3_PWR_CON, mcu_spm_read(MP0_CPU3_PWR_CON) | PWR_ON_2ND);
#ifndef IGNORE_PWR_ACK
		/* TINFO="Wait until PWR_STATUS = 1 and PWR_STATUS_2ND = 1" */
		while (!(mcu_spm_read(PWR_STATUS) & MP0_CPU3_PWR_STA_MASK)
		       || !(mcu_spm_read(PWR_STATUS_2ND) & MP0_CPU3_PWR_STA_MASK)) {
			/* No logic between pwr_on and pwr_ack. Print SRAM / MTCMOS control and PWR_ACK for debug. */
		}
#endif
		/* TINFO="Set SRAM_SLPB = 1" */
		mcu_spm_write(MP0_CPU3_PWR_CON, mcu_spm_read(MP0_CPU3_PWR_CON) | SRAM_SLEEP_B);
		/* TINFO="Set SRAM_ISOINT_B = 1" */
		mcu_spm_write(MP0_CPU3_PWR_CON, mcu_spm_read(MP0_CPU3_PWR_CON) | SRAM_ISOINT_B);
		/* TINFO="Set PWR_ISO = 0" */
		mcu_spm_write(MP0_CPU3_PWR_CON, mcu_spm_read(MP0_CPU3_PWR_CON) & ~PWR_ISO);
		/* TINFO="Delay 1us" */
		udelay(1);
		/* TINFO="Set SRAM_CKISO = 0" */
		mcu_spm_write(MP0_CPU3_PWR_CON, mcu_spm_read(MP0_CPU3_PWR_CON) & ~SRAM_CKISO);
		/* TINFO="Set PWR_CLK_DIS = 0" */
		mcu_spm_write(MP0_CPU3_PWR_CON, mcu_spm_read(MP0_CPU3_PWR_CON) & ~PWR_CLK_DIS);
		/* TINFO="Set PWR_RST_B = 1" */
		mcu_spm_write(MP0_CPU3_PWR_CON, mcu_spm_read(MP0_CPU3_PWR_CON) | PWR_RST_B);
		/* TINFO="Finish to turn on MP0_CPU3" */
	}
	return err;
}
#endif /* MTCMOS_CTRL_MP0 */
int spm_mtcmos_ctrl_mp1_cputop_dormant(int state)
{
	int err = 0;
	const unsigned int mp1_mask =
		(1 << IDX_PROTECT_ICC1_CACTIVE) |
		(1 << IDX_PROTECT_ICD1_CACTIVE) |
		(1 << IDX_PROTECT_MP1_CACTIVE) |
		(1 << IDX_PROTECT_L2C1_CACTIVE);
	/* TINFO="enable SPM register control" */
	mcu_spm_write(POWERON_CONFIG_EN, (SPM_PROJECT_CODE << 16) | (0x1 << 0));
	if (state == STA_POWER_DOWN) {
		/* TINFO="Start to turn off MP0_CPUTOP" */
		/* TINFO="Set STANDBYWFIL2 for Cluster 0" */
		*(volatile unsigned int *)MP1_AXI_CONFIG |= 0x00000010;
		*(volatile unsigned int *)(0x10394000) &= 0xfffffffc;
		*(volatile unsigned int *)(0x10395000) &= 0xfffffffc;
		/* TINFO="Wait STANDBYWFIL2 for Cluster 0" */
		while ((*(volatile unsigned int *)MP1_CA7_MISC_CONFIG & (1 << 28)) != (1 << 28));
		/* TINFO="Set ADB pwrdnreqn for Cluster 0" */
		*(volatile unsigned int *)INFRA_TOPAXI_PROTECTEN_1 |= mp1_mask;
		/* TINFO="Wait ADB pwrdnreqn for Cluster 0" */
		while ((*(volatile unsigned int *)INFRA_TOPAXI_PROTECTSTA1_1 & mp1_mask) != mp1_mask);
		/* TINFO="Set PWR_ISO = 1" */
		mcu_spm_write(MP1_CPUTOP_PWR_CON, mcu_spm_read(MP1_CPUTOP_PWR_CON) | PWR_ISO);
		/* TINFO="Set SRAM_CKISO = 1" */
		mcu_spm_write(MP1_CPUTOP_PWR_CON, mcu_spm_read(MP1_CPUTOP_PWR_CON) | SRAM_CKISO);
		/* TINFO="Set SRAM_ISOINT_B = 0" */
		mcu_spm_write(MP1_CPUTOP_PWR_CON, mcu_spm_read(MP1_CPUTOP_PWR_CON) & ~SRAM_ISOINT_B);
		/* TINFO="Set SRAM_SLEEP_B = 0" */
		mcu_spm_write(MP1_CPUTOP_PWR_CON, mcu_spm_read(MP1_CPUTOP_PWR_CON) & ~SRAM_SLEEP_B);
		/* TINFO="Wait until MP1_CPUTOP_SRAM_SLEEP_B_ACK = 0" */
		while (mcu_spm_read(MP1_CPUTOP_PWR_CON) & SRAM_SLEEP_B_ACK) {
			/* SRAM SLEEP delay IP clock is 26MHz. Print SRAM / MTCMOS control and PWR_ACK for debug. */
		}
		/* TINFO="Set PWR_RST_B = 0" */
		mcu_spm_write(MP1_CPUTOP_PWR_CON, mcu_spm_read(MP1_CPUTOP_PWR_CON) & ~PWR_RST_B);
		/* TINFO="Set PWR_CLK_DIS = 1" */
		mcu_spm_write(MP1_CPUTOP_PWR_CON, mcu_spm_read(MP1_CPUTOP_PWR_CON) | PWR_CLK_DIS);
		/* TINFO="Set PWR_ON = 0" */
		mcu_spm_write(MP1_CPUTOP_PWR_CON, mcu_spm_read(MP1_CPUTOP_PWR_CON) & ~PWR_ON);
		/* TINFO="Set PWR_ON_2ND = 0" */
		mcu_spm_write(MP1_CPUTOP_PWR_CON, mcu_spm_read(MP1_CPUTOP_PWR_CON) & ~PWR_ON_2ND);
#ifndef IGNORE_PWR_ACK
		/* TINFO="Wait until PWR_STATUS = 0 and PWR_STATUS_2ND = 0" */
		while ((mcu_spm_read(PWR_STATUS) & MP1_CPUTOP_PWR_STA_MASK)
		       || (mcu_spm_read(PWR_STATUS_2ND) & MP1_CPUTOP_PWR_STA_MASK)) {
			/* No logic between pwr_on and pwr_ack. Print SRAM / MTCMOS control and PWR_ACK for debug. */
		}
#endif
		/* TINFO="Finish to turn off MP1_CPUTOP" */
	}  else {    /* STA_POWER_ON */
		/* TINFO="Start to turn on MP1_CPUTOP" */
		/* TINFO="Set PWR_ON = 1" */
		mcu_spm_write(MP1_CPUTOP_PWR_CON, mcu_spm_read(MP1_CPUTOP_PWR_CON) | PWR_ON);
		/* TINFO="Set PWR_ON_2ND = 1" */
		mcu_spm_write(MP1_CPUTOP_PWR_CON, mcu_spm_read(MP1_CPUTOP_PWR_CON) | PWR_ON_2ND);
#ifndef IGNORE_PWR_ACK
		/* TINFO="Wait until PWR_STATUS = 1 and PWR_STATUS_2ND = 1" */
		while (!(mcu_spm_read(PWR_STATUS) & MP1_CPUTOP_PWR_STA_MASK)
		       || !(mcu_spm_read(PWR_STATUS_2ND) & MP1_CPUTOP_PWR_STA_MASK)) {
			/* No logic between pwr_on and pwr_ack. Print SRAM / MTCMOS control and PWR_ACK for debug. */
		}
#endif
		/* TINFO="Set PWR_ISO = 0" */
		mcu_spm_write(MP1_CPUTOP_PWR_CON, mcu_spm_read(MP1_CPUTOP_PWR_CON) & ~PWR_ISO);
		/* TINFO="Set SRAM_SLEEP_B = 1" */
		mcu_spm_write(MP1_CPUTOP_PWR_CON, mcu_spm_read(MP1_CPUTOP_PWR_CON) | SRAM_SLEEP_B );
		/* TINFO="Wait until MP1_CPUTOP_SRAM_SLEEP_B_ACK = 1" */
		while (!(mcu_spm_read(MP1_CPUTOP_PWR_CON) & SRAM_SLEEP_B_ACK)) {
			/* SRAM SLEEP delay IP clock is 26MHz. Print SRAM / MTCMOS control and PWR_ACK for debug. */
		}
		/* TINFO="Delay 1us" */
		udelay(1);
		/* TINFO="Set SRAM_ISOINT_B = 1" */
		mcu_spm_write(MP1_CPUTOP_PWR_CON, mcu_spm_read(MP1_CPUTOP_PWR_CON) | SRAM_ISOINT_B);
		/* TINFO="Delay 1us" */
		udelay(1);
		/* TINFO="Set SRAM_CKISO = 0" */
		mcu_spm_write(MP1_CPUTOP_PWR_CON, mcu_spm_read(MP1_CPUTOP_PWR_CON) & ~SRAM_CKISO);
		/* TINFO="Set PWR_CLK_DIS = 0" */
		mcu_spm_write(MP1_CPUTOP_PWR_CON, mcu_spm_read(MP1_CPUTOP_PWR_CON) & ~PWR_CLK_DIS);
		/* TINFO="Set PWR_RST_B = 1" */
		mcu_spm_write(MP1_CPUTOP_PWR_CON, mcu_spm_read(MP1_CPUTOP_PWR_CON) | PWR_RST_B);
		/* TINFO="Release bus protect" */
		/* TINFO="Release ADB pwrdnreqn for Cluster 1" */
		*(volatile unsigned int *)INFRA_TOPAXI_PROTECTEN_1 &= 0xffffffff ^ mp1_mask;
		/* TINFO="Wait ADB ~pwrdnreqn for Cluster 1" */
		while ((*(volatile unsigned int *)INFRA_TOPAXI_PROTECTSTA1_1 & mp1_mask) != 0);
		/* TINFO="Finish to turn on MP1_CPUTOP" */
	}
	return err;
}
int spm_mtcmos_ctrl_mp1_cputop_shut_down(int state)
{
	int err = 0;
	const unsigned int mp1_mask =
		(1 << IDX_PROTECT_ICC1_CACTIVE) |
		(1 << IDX_PROTECT_ICD1_CACTIVE) |
		(1 << IDX_PROTECT_MP1_CACTIVE) |
		(1 << IDX_PROTECT_L2C1_CACTIVE);
	INFO("enable SPM register control\n");
	mcu_spm_write(POWERON_CONFIG_EN, (SPM_PROJECT_CODE << 16) | (0x1 << 0));
	if (state == STA_POWER_DOWN) {
		INFO("Start to turn off MP1_CPUTOP\n");
		INFO("Set STANDBYWFIL2 for Cluster 1\n");
		*(volatile unsigned int *)MP1_AXI_CONFIG |= 0x00000010;
		*(volatile unsigned int *)(0x10394000) &= 0xfffffffc;
		*(volatile unsigned int *)(0x10395000) &= 0xfffffffc;
		INFO("Wait STANDBYWFIL2 for Cluster 1\n");
		while ((*(volatile unsigned int *)MP1_CA7_MISC_CONFIG & (1 << 28)) != (1 << 28));
		INFO("Set ADB pwrdnreqn for Cluster 1\n");
		*(volatile unsigned int *)INFRA_TOPAXI_PROTECTEN_1 |= mp1_mask;
		INFO("Wait ADB pwrdnreqn for Cluster 1\n");
		while ((*(volatile unsigned int *)INFRA_TOPAXI_PROTECTSTA1_1 & mp1_mask) != mp1_mask);
		INFO("Set PWR_ISO = 1\n");
		mcu_spm_write(MP1_CPUTOP_PWR_CON, mcu_spm_read(MP1_CPUTOP_PWR_CON) | PWR_ISO);
		INFO("Set SRAM_CKISO = 1\n");
		mcu_spm_write(MP1_CPUTOP_PWR_CON, mcu_spm_read(MP1_CPUTOP_PWR_CON) | SRAM_CKISO);
		INFO("Set SRAM_ISOINT_B = 0\n");
		mcu_spm_write(MP1_CPUTOP_PWR_CON, mcu_spm_read(MP1_CPUTOP_PWR_CON) & ~SRAM_ISOINT_B);
		INFO("Set SRAM_PDN = 1\n");
		mcu_spm_write(MP1_CPUTOP_PWR_CON, mcu_spm_read(MP1_CPUTOP_PWR_CON) | SRAM_PDN);

		INFO("Set PWR_RST_B = 0\n");
		mcu_spm_write(MP1_CPUTOP_PWR_CON, mcu_spm_read(MP1_CPUTOP_PWR_CON) & ~PWR_RST_B);
		INFO("Set PWR_CLK_DIS = 1\n");
		mcu_spm_write(MP1_CPUTOP_PWR_CON, mcu_spm_read(MP1_CPUTOP_PWR_CON) | PWR_CLK_DIS);
		INFO("Set PWR_ON = 0\n");
		mcu_spm_write(MP1_CPUTOP_PWR_CON, mcu_spm_read(MP1_CPUTOP_PWR_CON) & ~PWR_ON);
		INFO("Set PWR_ON_2ND = 0\n");
		mcu_spm_write(MP1_CPUTOP_PWR_CON, mcu_spm_read(MP1_CPUTOP_PWR_CON) & ~PWR_ON_2ND);
#ifndef IGNORE_PWR_ACK
		INFO("Wait until PWR_STATUS = 0 and PWR_STATUS_2ND = 0\n");
		while ((mcu_spm_read(PWR_STATUS) & MP1_CPUTOP_PWR_STA_MASK)
		       || (mcu_spm_read(PWR_STATUS_2ND) & MP1_CPUTOP_PWR_STA_MASK)) {
			/* No logic between pwr_on and pwr_ack. Print SRAM / MTCMOS control and PWR_ACK for debug. */
		}
#endif
		INFO("Finish to turn off MP1_CPUTOP\n");
	}  else {    /* STA_POWER_ON */
		INFO("Start to turn on MP1_CPUTOP\n");
		INFO("Set PWR_ON = 1\n");
		mcu_spm_write(MP1_CPUTOP_PWR_CON, mcu_spm_read(MP1_CPUTOP_PWR_CON) | PWR_ON);
		INFO("Set PWR_ON_2ND = 1\n");
		mcu_spm_write(MP1_CPUTOP_PWR_CON, mcu_spm_read(MP1_CPUTOP_PWR_CON) | PWR_ON_2ND);
#ifndef IGNORE_PWR_ACK
		INFO("Wait until PWR_STATUS = 1 and PWR_STATUS_2ND = 1\n");
		while (!(mcu_spm_read(PWR_STATUS) & MP1_CPUTOP_PWR_STA_MASK)
		       || !(mcu_spm_read(PWR_STATUS_2ND) & MP1_CPUTOP_PWR_STA_MASK)) {
			/* No logic between pwr_on and pwr_ack. Print SRAM / MTCMOS control and PWR_ACK for debug. */
		}
#endif
		INFO("Set PWR_ISO = 0\n");
		mcu_spm_write(MP1_CPUTOP_PWR_CON, mcu_spm_read(MP1_CPUTOP_PWR_CON) & ~PWR_ISO);
		INFO("Set SRAM_PDN = 0\n");
		mcu_spm_write(MP1_CPUTOP_PWR_CON, mcu_spm_read(MP1_CPUTOP_PWR_CON) & ~(0x1 << 8));
		INFO("Wait until MP1_CPUTOP_SRAM_PDN_ACK = 0\n");
		while (mcu_spm_read(MP1_CPUTOP_PWR_CON) & SRAM_PDN_ACK) {
			/* SRAM PDN delay IP clock is 26MHz. Print SRAM control and ACK for debug. */
		}
		INFO("Delay 1us\n");
		udelay(1);
		INFO("Set SRAM_ISOINT_B = 1\n");
		mcu_spm_write(MP1_CPUTOP_PWR_CON, mcu_spm_read(MP1_CPUTOP_PWR_CON) | SRAM_ISOINT_B);
		INFO("Delay 1us\n");
		udelay(1);
		INFO("Set SRAM_CKISO = 0\n");
		mcu_spm_write(MP1_CPUTOP_PWR_CON, mcu_spm_read(MP1_CPUTOP_PWR_CON) & ~SRAM_CKISO);
		INFO("Set PWR_CLK_DIS = 0\n");
		mcu_spm_write(MP1_CPUTOP_PWR_CON, mcu_spm_read(MP1_CPUTOP_PWR_CON) & ~PWR_CLK_DIS);
		INFO("Set PWR_RST_B = 1\n");
		mcu_spm_write(MP1_CPUTOP_PWR_CON, mcu_spm_read(MP1_CPUTOP_PWR_CON) | PWR_RST_B);
		INFO("Release bus protect\n");
		INFO("Release ADB pwrdnreqn for Cluster 1\n");
		*(volatile unsigned int *)INFRA_TOPAXI_PROTECTEN_1 &= 0xffffffff ^ mp1_mask;
		INFO("Wait ADB ~pwrdnreqn for Cluster 1\n");
		while ((*(volatile unsigned int *)INFRA_TOPAXI_PROTECTSTA1_1 & mp1_mask) != 0);
		INFO("Finish to turn on MP1_CPUTOP\n");
	}
	return err;
}
int spm_mtcmos_ctrl_mp1_cpu0(int state)
{
	int err = 0;
	INFO("enable SPM register control\n");
	mcu_spm_write(POWERON_CONFIG_EN, (SPM_PROJECT_CODE << 16) | (0x1 << 0));
	if (state == STA_POWER_DOWN) {
		INFO("Start to turn off MP1_CPU0\n");
		INFO("Set PWR_ISO = 1\n");
		mcu_spm_write(MP1_CPU0_PWR_CON, mcu_spm_read(MP1_CPU0_PWR_CON) | PWR_ISO);
		INFO("Set SRAM_ISOINT_B = 0\n");
		mcu_spm_write(MP1_CPU0_PWR_CON, mcu_spm_read(MP1_CPU0_PWR_CON) & ~SRAM_ISOINT_B);
		INFO("Set SRAM_CKISO = 1\n");
		mcu_spm_write(MP1_CPU0_PWR_CON, mcu_spm_read(MP1_CPU0_PWR_CON) | SRAM_CKISO);
		INFO("Set PWR_RST_B = 0\n");
		mcu_spm_write(MP1_CPU0_PWR_CON, mcu_spm_read(MP1_CPU0_PWR_CON) & ~PWR_RST_B);
		INFO("Set PWR_CLK_DIS = 1\n");
		mcu_spm_write(MP1_CPU0_PWR_CON, mcu_spm_read(MP1_CPU0_PWR_CON) | PWR_CLK_DIS);
		INFO("Set PWR_RST_B = 0\n");
		mcu_spm_write(MP1_CPU0_PWR_CON, mcu_spm_read(MP1_CPU0_PWR_CON) & ~PWR_RST_B);
		INFO("Set PWR_ON = 0\n");
		mcu_spm_write(MP1_CPU0_PWR_CON, mcu_spm_read(MP1_CPU0_PWR_CON) & ~PWR_ON);
		INFO("Set PWR_ON_2ND = 0\n");
		mcu_spm_write(MP1_CPU0_PWR_CON, mcu_spm_read(MP1_CPU0_PWR_CON) & ~PWR_ON_2ND);
		/* TINFO="Set SRAM_SLPB = 0" */
		/* mcu_spm_write(MP1_CPU0_PWR_CON, mcu_spm_read(MP1_CPU0_PWR_CON) & ~SRAM_SLEEP_B); */
#ifndef IGNORE_PWR_ACK
		INFO("Wait until PWR_STATUS = 0 and PWR_STATUS_2ND = 0\n");
		while ((mcu_spm_read(PWR_STATUS) & MP1_CPU0_PWR_STA_MASK)
		       || (mcu_spm_read(PWR_STATUS_2ND) & MP1_CPU0_PWR_STA_MASK)) {
			/* No logic between pwr_on and pwr_ack. Print SRAM / MTCMOS control and PWR_ACK for debug. */
		}
#endif
		INFO("Set SRAM_PDN = 1\n");
		mcu_spm_write(MP1_CPU0_PWR_CON, mcu_spm_read(MP1_CPU0_PWR_CON) | SRAM_PDN);

		INFO("Set SRAM_PD_SLPB_CLAMP = 1\n");
		mcu_spm_write(MP1_CPU0_PWR_CON, mcu_spm_read(MP1_CPU0_PWR_CON) | SRAM_PD_SLPB_CLAMP);
		INFO("Finish to turn off MP1_CPU0\n");
	} else {    /* STA_POWER_ON */
		INFO("Start to turn on MP1_CPU0\n");
		INFO("Set SRAM_SLPB = 0\n");
		mcu_spm_write(MP1_CPU0_PWR_CON, mcu_spm_read(MP1_CPU0_PWR_CON) & ~SRAM_SLEEP_B);
		INFO("Set PWR_RST_B = 0\n");
		mcu_spm_write(MP1_CPU0_PWR_CON, mcu_spm_read(MP1_CPU0_PWR_CON) & ~PWR_RST_B);
		INFO("Set PWR_CLK_DIS = 1\n");
		mcu_spm_write(MP1_CPU0_PWR_CON, mcu_spm_read(MP1_CPU0_PWR_CON) | PWR_CLK_DIS);
		INFO("Set SRAM_PDN = 1\n");
		mcu_spm_write(MP1_CPU0_PWR_CON, mcu_spm_read(MP1_CPU0_PWR_CON) | SRAM_PDN);
                #ifndef DDR_RESERVE_ENABLE
		INFO("Wait until MP1_CPU0_SRAM_PDN_ACK = 1\n");
		while (!(mcu_spm_read(MP1_CPU0_PWR_CON) & SRAM_PDN_ACK)) {
			/* SRAM PDN delay IP clock is 26MHz. Print SRAM control and ACK for debug. */
		}
		#endif
		INFO("Set SRAM_PD_SLPB_CLAMP = 0\n");
		mcu_spm_write(MP1_CPU0_PWR_CON, mcu_spm_read(MP1_CPU0_PWR_CON) & ~SRAM_PD_SLPB_CLAMP);
		INFO("Set SRAM_PDN = 0\n");
		mcu_spm_write(MP1_CPU0_PWR_CON, mcu_spm_read(MP1_CPU0_PWR_CON) & ~(0x1 << 8));
		INFO("Wait until MP1_CPU0_SRAM_PDN_ACK = 0\n");
		while (mcu_spm_read(MP1_CPU0_PWR_CON) & SRAM_PDN_ACK) {
			/* SRAM PDN delay IP clock is 26MHz. Print SRAM control and ACK for debug. */
		}
		INFO("Set PWR_ON = 1\n");
		mcu_spm_write(MP1_CPU0_PWR_CON, mcu_spm_read(MP1_CPU0_PWR_CON) | PWR_ON);
		INFO("Set PWR_ON_2ND = 1\n");
		mcu_spm_write(MP1_CPU0_PWR_CON, mcu_spm_read(MP1_CPU0_PWR_CON) | PWR_ON_2ND);
#ifndef IGNORE_PWR_ACK
		INFO("Wait until PWR_STATUS = 1 and PWR_STATUS_2ND = 1i\n");
		while (!(mcu_spm_read(PWR_STATUS) & MP1_CPU0_PWR_STA_MASK)
		       || !(mcu_spm_read(PWR_STATUS_2ND) & MP1_CPU0_PWR_STA_MASK)) {
			/* No logic between pwr_on and pwr_ack. Print SRAM / MTCMOS control and PWR_ACK for debug. */
		}
#endif
		INFO("Set SRAM_SLPB = 1\n");
		mcu_spm_write(MP1_CPU0_PWR_CON, mcu_spm_read(MP1_CPU0_PWR_CON) | SRAM_SLEEP_B);
		INFO("Set SRAM_ISOINT_B = 1\n");
		mcu_spm_write(MP1_CPU0_PWR_CON, mcu_spm_read(MP1_CPU0_PWR_CON) | SRAM_ISOINT_B);
		INFO("Set PWR_ISO = 0\n");
		mcu_spm_write(MP1_CPU0_PWR_CON, mcu_spm_read(MP1_CPU0_PWR_CON) & ~PWR_ISO);
		INFO("Delay 1us\n");
		udelay(1);
		INFO("Set SRAM_CKISO = 0\n");
		mcu_spm_write(MP1_CPU0_PWR_CON, mcu_spm_read(MP1_CPU0_PWR_CON) & ~SRAM_CKISO);
		INFO("Set PWR_CLK_DIS = 0\n");
		mcu_spm_write(MP1_CPU0_PWR_CON, mcu_spm_read(MP1_CPU0_PWR_CON) & ~PWR_CLK_DIS);
		INFO("Set PWR_RST_B = 1\n");
		mcu_spm_write(MP1_CPU0_PWR_CON, mcu_spm_read(MP1_CPU0_PWR_CON) | PWR_RST_B);
		INFO("Finish to turn on MP1_CPU0\n");
	}
	return err;
}
int spm_mtcmos_ctrl_mp1_cpu1(int state)
{
	int err = 0;
	/* TINFO="enable SPM register control" */
	mcu_spm_write(POWERON_CONFIG_EN, (SPM_PROJECT_CODE << 16) | (0x1 << 0));
	if (state == STA_POWER_DOWN) {
		/* TINFO="Start to turn off MP1_CPU1" */
		/* TINFO="Set PWR_ISO = 1" */
		mcu_spm_write(MP1_CPU1_PWR_CON, mcu_spm_read(MP1_CPU1_PWR_CON) | PWR_ISO);
		/* TINFO="Set SRAM_ISOINT_B = 0" */
		mcu_spm_write(MP1_CPU1_PWR_CON, mcu_spm_read(MP1_CPU1_PWR_CON) & ~SRAM_ISOINT_B);
		/* TINFO="Set SRAM_CKISO = 1" */
		mcu_spm_write(MP1_CPU1_PWR_CON, mcu_spm_read(MP1_CPU1_PWR_CON) | SRAM_CKISO);
		/* TINFO="Set PWR_RST_B = 0" */
		mcu_spm_write(MP1_CPU1_PWR_CON, mcu_spm_read(MP1_CPU1_PWR_CON) & ~PWR_RST_B);
		/* TINFO="Set PWR_CLK_DIS = 1" */
		mcu_spm_write(MP1_CPU1_PWR_CON, mcu_spm_read(MP1_CPU1_PWR_CON) | PWR_CLK_DIS);
		/* TINFO="Set PWR_RST_B = 0" */
		mcu_spm_write(MP1_CPU1_PWR_CON, mcu_spm_read(MP1_CPU1_PWR_CON) & ~PWR_RST_B);
		/* TINFO="Set PWR_ON = 0" */
		mcu_spm_write(MP1_CPU1_PWR_CON, mcu_spm_read(MP1_CPU1_PWR_CON) & ~PWR_ON);
		/* TINFO="Set PWR_ON_2ND = 0" */
		mcu_spm_write(MP1_CPU1_PWR_CON, mcu_spm_read(MP1_CPU1_PWR_CON) & ~PWR_ON_2ND);
		/* TINFO="Set SRAM_SLPB = 0" */
		/* mcu_spm_write(MP1_CPU1_PWR_CON, mcu_spm_read(MP1_CPU1_PWR_CON) & ~SRAM_SLEEP_B); */
#ifndef IGNORE_PWR_ACK
		/* TINFO="Wait until PWR_STATUS = 0 and PWR_STATUS_2ND = 0" */
		while ((mcu_spm_read(PWR_STATUS) & MP1_CPU1_PWR_STA_MASK)
		       || (mcu_spm_read(PWR_STATUS_2ND) & MP1_CPU1_PWR_STA_MASK)) {
			/* No logic between pwr_on and pwr_ack. Print SRAM / MTCMOS control and PWR_ACK for debug. */
		}
#endif
		/* TINFO="Set SRAM_PDN = 1" */
		mcu_spm_write(MP1_CPU1_PWR_CON, mcu_spm_read(MP1_CPU1_PWR_CON) | SRAM_PDN);
		/* TINFO="Wait until MP1_CPU1_SRAM_PDN_ACK = 1" */
		while (!(mcu_spm_read(MP1_CPU1_PWR_CON) & SRAM_PDN_ACK)) {
			/* SRAM PDN delay IP clock is 26MHz. Print SRAM control and ACK for debug. */
		}
		/* TINFO="Set SRAM_PD_SLPB_CLAMP = 1" */
		mcu_spm_write(MP1_CPU1_PWR_CON, mcu_spm_read(MP1_CPU1_PWR_CON) | SRAM_PD_SLPB_CLAMP);
		/* TINFO="Finish to turn off MP1_CPU1" */
	} else {    /* STA_POWER_ON */
		/* TINFO="Start to turn on MP1_CPU1" */
		/* TINFO="Set SRAM_SLPB = 0" */
		mcu_spm_write(MP1_CPU1_PWR_CON, mcu_spm_read(MP1_CPU1_PWR_CON) & ~SRAM_SLEEP_B);
		/* TINFO="Set PWR_RST_B = 0" */
		mcu_spm_write(MP1_CPU1_PWR_CON, mcu_spm_read(MP1_CPU1_PWR_CON) & ~PWR_RST_B);
		/* TINFO="Set PWR_CLK_DIS = 1" */
		mcu_spm_write(MP1_CPU1_PWR_CON, mcu_spm_read(MP1_CPU1_PWR_CON) | PWR_CLK_DIS);
		/* TINFO="Set SRAM_PDN = 1" */
		mcu_spm_write(MP1_CPU1_PWR_CON, mcu_spm_read(MP1_CPU1_PWR_CON) | SRAM_PDN);
		#ifndef DDR_RESERVE_ENABLE
		/* TINFO="Wait until MP1_CPU1_SRAM_PDN_ACK = 1" */
		while (!(mcu_spm_read(MP1_CPU1_PWR_CON) & SRAM_PDN_ACK)) {
			/* SRAM PDN delay IP clock is 26MHz. Print SRAM control and ACK for debug. */
		}
		#endif
		/* TINFO="Set SRAM_PD_SLPB_CLAMP = 0" */
		mcu_spm_write(MP1_CPU1_PWR_CON, mcu_spm_read(MP1_CPU1_PWR_CON) & ~SRAM_PD_SLPB_CLAMP);
		/* TINFO="Set SRAM_PDN = 0" */
		mcu_spm_write(MP1_CPU1_PWR_CON, mcu_spm_read(MP1_CPU1_PWR_CON) & ~(0x1 << 8));
		/* TINFO="Wait until MP1_CPU1_SRAM_PDN_ACK = 0" */
		while (mcu_spm_read(MP1_CPU1_PWR_CON) & SRAM_PDN_ACK) {
			/* SRAM PDN delay IP clock is 26MHz. Print SRAM control and ACK for debug. */
		}
		/* TINFO="Set PWR_ON = 1" */
		mcu_spm_write(MP1_CPU1_PWR_CON, mcu_spm_read(MP1_CPU1_PWR_CON) | PWR_ON);
		/* TINFO="Set PWR_ON_2ND = 1" */
		mcu_spm_write(MP1_CPU1_PWR_CON, mcu_spm_read(MP1_CPU1_PWR_CON) | PWR_ON_2ND);
#ifndef IGNORE_PWR_ACK
		/* TINFO="Wait until PWR_STATUS = 1 and PWR_STATUS_2ND = 1" */
		while (!(mcu_spm_read(PWR_STATUS) & MP1_CPU1_PWR_STA_MASK)
		       || !(mcu_spm_read(PWR_STATUS_2ND) & MP1_CPU1_PWR_STA_MASK)) {
			/* No logic between pwr_on and pwr_ack. Print SRAM / MTCMOS control and PWR_ACK for debug. */
		}
#endif
		/* TINFO="Set SRAM_SLPB = 1" */
		mcu_spm_write(MP1_CPU1_PWR_CON, mcu_spm_read(MP1_CPU1_PWR_CON) | SRAM_SLEEP_B);
		/* TINFO="Set SRAM_ISOINT_B = 1" */
		mcu_spm_write(MP1_CPU1_PWR_CON, mcu_spm_read(MP1_CPU1_PWR_CON) | SRAM_ISOINT_B);
		/* TINFO="Set PWR_ISO = 0" */
		mcu_spm_write(MP1_CPU1_PWR_CON, mcu_spm_read(MP1_CPU1_PWR_CON) & ~PWR_ISO);
		/* TINFO="Delay 1us" */
		udelay(1);
		/* TINFO="Set SRAM_CKISO = 0" */
		mcu_spm_write(MP1_CPU1_PWR_CON, mcu_spm_read(MP1_CPU1_PWR_CON) & ~SRAM_CKISO);
		/* TINFO="Set PWR_CLK_DIS = 0" */
		mcu_spm_write(MP1_CPU1_PWR_CON, mcu_spm_read(MP1_CPU1_PWR_CON) & ~PWR_CLK_DIS);
		/* TINFO="Set PWR_RST_B = 1" */
		mcu_spm_write(MP1_CPU1_PWR_CON, mcu_spm_read(MP1_CPU1_PWR_CON) | PWR_RST_B);
		/* TINFO="Finish to turn on MP1_CPU1" */
	}
	return err;
}
int spm_mtcmos_ctrl_mp1_cpu2(int state)
{
	int err = 0;
	/* TINFO="enable SPM register control" */
	mcu_spm_write(POWERON_CONFIG_EN, (SPM_PROJECT_CODE << 16) | (0x1 << 0));
	if (state == STA_POWER_DOWN) {
		/* TINFO="Start to turn off MP1_CPU2" */
		/* TINFO="Set PWR_ISO = 1" */
		mcu_spm_write(MP1_CPU2_PWR_CON, mcu_spm_read(MP1_CPU2_PWR_CON) | PWR_ISO);
		/* TINFO="Set SRAM_ISOINT_B = 0" */
		mcu_spm_write(MP1_CPU2_PWR_CON, mcu_spm_read(MP1_CPU2_PWR_CON) & ~SRAM_ISOINT_B);
		/* TINFO="Set SRAM_CKISO = 1" */
		mcu_spm_write(MP1_CPU2_PWR_CON, mcu_spm_read(MP1_CPU2_PWR_CON) | SRAM_CKISO);
		/* TINFO="Set PWR_RST_B = 0" */
		mcu_spm_write(MP1_CPU2_PWR_CON, mcu_spm_read(MP1_CPU2_PWR_CON) & ~PWR_RST_B);
		/* TINFO="Set PWR_CLK_DIS = 1" */
		mcu_spm_write(MP1_CPU2_PWR_CON, mcu_spm_read(MP1_CPU2_PWR_CON) | PWR_CLK_DIS);
		/* TINFO="Set PWR_RST_B = 0" */
		mcu_spm_write(MP1_CPU2_PWR_CON, mcu_spm_read(MP1_CPU2_PWR_CON) & ~PWR_RST_B);
		/* TINFO="Set PWR_ON = 0" */
		mcu_spm_write(MP1_CPU2_PWR_CON, mcu_spm_read(MP1_CPU2_PWR_CON) & ~PWR_ON);
		/* TINFO="Set PWR_ON_2ND = 0" */
		mcu_spm_write(MP1_CPU2_PWR_CON, mcu_spm_read(MP1_CPU2_PWR_CON) & ~PWR_ON_2ND);
		/* TINFO="Set SRAM_SLPB = 0" */
		/* mcu_spm_write(MP1_CPU2_PWR_CON, mcu_spm_read(MP1_CPU2_PWR_CON) & ~SRAM_SLEEP_B); */
#ifndef IGNORE_PWR_ACK
		/* TINFO="Wait until PWR_STATUS = 0 and PWR_STATUS_2ND = 0" */
		while ((mcu_spm_read(PWR_STATUS) & MP1_CPU2_PWR_STA_MASK)
		       || (mcu_spm_read(PWR_STATUS_2ND) & MP1_CPU2_PWR_STA_MASK)) {
			/* No logic between pwr_on and pwr_ack. Print SRAM / MTCMOS control and PWR_ACK for debug. */
		}
#endif
		/* TINFO="Set SRAM_PDN = 1" */
		mcu_spm_write(MP1_CPU2_PWR_CON, mcu_spm_read(MP1_CPU2_PWR_CON) | SRAM_PDN);
		/* TINFO="Wait until MP1_CPU2_SRAM_PDN_ACK = 1" */
		while (!(mcu_spm_read(MP1_CPU2_PWR_CON) & SRAM_PDN_ACK)) {
			/* SRAM PDN delay IP clock is 26MHz. Print SRAM control and ACK for debug. */
		}
		/* TINFO="Set SRAM_PD_SLPB_CLAMP = 1" */
		mcu_spm_write(MP1_CPU2_PWR_CON, mcu_spm_read(MP1_CPU2_PWR_CON) | SRAM_PD_SLPB_CLAMP);
		/* TINFO="Finish to turn off MP1_CPU2" */
	} else {    /* STA_POWER_ON */
		/* TINFO="Start to turn on MP1_CPU2" */
		/* TINFO="Set SRAM_SLPB = 0" */
		mcu_spm_write(MP1_CPU2_PWR_CON, mcu_spm_read(MP1_CPU2_PWR_CON) & ~SRAM_SLEEP_B);
		/* TINFO="Set PWR_RST_B = 0" */
		mcu_spm_write(MP1_CPU2_PWR_CON, mcu_spm_read(MP1_CPU2_PWR_CON) & ~PWR_RST_B);
		/* TINFO="Set PWR_CLK_DIS = 1" */
		mcu_spm_write(MP1_CPU2_PWR_CON, mcu_spm_read(MP1_CPU2_PWR_CON) | PWR_CLK_DIS);
		/* TINFO="Set SRAM_PDN = 1" */
		mcu_spm_write(MP1_CPU2_PWR_CON, mcu_spm_read(MP1_CPU2_PWR_CON) | SRAM_PDN);
		#ifndef DDR_RESERVE_ENABLE
		/* TINFO="Wait until MP1_CPU2_SRAM_PDN_ACK = 1" */
		while (!(mcu_spm_read(MP1_CPU2_PWR_CON) & SRAM_PDN_ACK)) {
			/* SRAM PDN delay IP clock is 26MHz. Print SRAM control and ACK for debug. */
		}
                #endif
		/* TINFO="Set SRAM_PD_SLPB_CLAMP = 0" */
		mcu_spm_write(MP1_CPU2_PWR_CON, mcu_spm_read(MP1_CPU2_PWR_CON) & ~SRAM_PD_SLPB_CLAMP);
		/* TINFO="Set SRAM_PDN = 0" */
		mcu_spm_write(MP1_CPU2_PWR_CON, mcu_spm_read(MP1_CPU2_PWR_CON) & ~(0x1 << 8));
		/* TINFO="Wait until MP1_CPU2_SRAM_PDN_ACK = 0" */
		while (mcu_spm_read(MP1_CPU2_PWR_CON) & SRAM_PDN_ACK) {
			/* SRAM PDN delay IP clock is 26MHz. Print SRAM control and ACK for debug. */
		}
		/* TINFO="Set PWR_ON = 1" */
		mcu_spm_write(MP1_CPU2_PWR_CON, mcu_spm_read(MP1_CPU2_PWR_CON) | PWR_ON);
		/* TINFO="Set PWR_ON_2ND = 1" */
		mcu_spm_write(MP1_CPU2_PWR_CON, mcu_spm_read(MP1_CPU2_PWR_CON) | PWR_ON_2ND);
#ifndef IGNORE_PWR_ACK
		/* TINFO="Wait until PWR_STATUS = 1 and PWR_STATUS_2ND = 1" */
		while (!(mcu_spm_read(PWR_STATUS) & MP1_CPU2_PWR_STA_MASK)
		       || !(mcu_spm_read(PWR_STATUS_2ND) & MP1_CPU2_PWR_STA_MASK)) {
			/* No logic between pwr_on and pwr_ack. Print SRAM / MTCMOS control and PWR_ACK for debug. */
		}
#endif
		/* TINFO="Set SRAM_SLPB = 1" */
		mcu_spm_write(MP1_CPU2_PWR_CON, mcu_spm_read(MP1_CPU2_PWR_CON) | SRAM_SLEEP_B);
		/* TINFO="Set SRAM_ISOINT_B = 1" */
		mcu_spm_write(MP1_CPU2_PWR_CON, mcu_spm_read(MP1_CPU2_PWR_CON) | SRAM_ISOINT_B);
		/* TINFO="Set PWR_ISO = 0" */
		mcu_spm_write(MP1_CPU2_PWR_CON, mcu_spm_read(MP1_CPU2_PWR_CON) & ~PWR_ISO);
		/* TINFO="Delay 1us" */
		udelay(1);
		/* TINFO="Set SRAM_CKISO = 0" */
		mcu_spm_write(MP1_CPU2_PWR_CON, mcu_spm_read(MP1_CPU2_PWR_CON) & ~SRAM_CKISO);
		/* TINFO="Set PWR_CLK_DIS = 0" */
		mcu_spm_write(MP1_CPU2_PWR_CON, mcu_spm_read(MP1_CPU2_PWR_CON) & ~PWR_CLK_DIS);
		/* TINFO="Set PWR_RST_B = 1" */
		mcu_spm_write(MP1_CPU2_PWR_CON, mcu_spm_read(MP1_CPU2_PWR_CON) | PWR_RST_B);
		/* TINFO="Finish to turn on MP1_CPU2" */
	}
	return err;
}
int spm_mtcmos_ctrl_mp1_cpu3(int state)
{
	int err = 0;
	/* TINFO="enable SPM register control" */
	mcu_spm_write(POWERON_CONFIG_EN, (SPM_PROJECT_CODE << 16) | (0x1 << 0));
	if (state == STA_POWER_DOWN) {
		/* TINFO="Start to turn off MP1_CPU3" */
		/* TINFO="Set PWR_ISO = 1" */
		mcu_spm_write(MP1_CPU3_PWR_CON, mcu_spm_read(MP1_CPU3_PWR_CON) | PWR_ISO);
		/* TINFO="Set SRAM_ISOINT_B = 0" */
		mcu_spm_write(MP1_CPU3_PWR_CON, mcu_spm_read(MP1_CPU3_PWR_CON) & ~SRAM_ISOINT_B);
		/* TINFO="Set SRAM_CKISO = 1" */
		mcu_spm_write(MP1_CPU3_PWR_CON, mcu_spm_read(MP1_CPU3_PWR_CON) | SRAM_CKISO);
		/* TINFO="Set PWR_RST_B = 0" */
		mcu_spm_write(MP1_CPU3_PWR_CON, mcu_spm_read(MP1_CPU3_PWR_CON) & ~PWR_RST_B);
		/* TINFO="Set PWR_CLK_DIS = 1" */
		mcu_spm_write(MP1_CPU3_PWR_CON, mcu_spm_read(MP1_CPU3_PWR_CON) | PWR_CLK_DIS);
		/* TINFO="Set PWR_RST_B = 0" */
		mcu_spm_write(MP1_CPU3_PWR_CON, mcu_spm_read(MP1_CPU3_PWR_CON) & ~PWR_RST_B);
		/* TINFO="Set PWR_ON = 0" */
		mcu_spm_write(MP1_CPU3_PWR_CON, mcu_spm_read(MP1_CPU3_PWR_CON) & ~PWR_ON);
		/* TINFO="Set PWR_ON_2ND = 0" */
		mcu_spm_write(MP1_CPU3_PWR_CON, mcu_spm_read(MP1_CPU3_PWR_CON) & ~PWR_ON_2ND);
		/* TINFO="Set SRAM_SLPB = 0" */
		/* mcu_spm_write(MP1_CPU3_PWR_CON, mcu_spm_read(MP1_CPU3_PWR_CON) & ~SRAM_SLEEP_B); */
#ifndef IGNORE_PWR_ACK
		/* TINFO="Wait until PWR_STATUS = 0 and PWR_STATUS_2ND = 0" */
		while ((mcu_spm_read(PWR_STATUS) & MP1_CPU3_PWR_STA_MASK)
		       || (mcu_spm_read(PWR_STATUS_2ND) & MP1_CPU3_PWR_STA_MASK)) {
			/* No logic between pwr_on and pwr_ack. Print SRAM / MTCMOS control and PWR_ACK for debug. */
		}
#endif
		/* TINFO="Set SRAM_PDN = 1" */
		mcu_spm_write(MP1_CPU3_PWR_CON, mcu_spm_read(MP1_CPU3_PWR_CON) | SRAM_PDN);
		/* TINFO="Wait until MP1_CPU3_SRAM_PDN_ACK = 1" */
		while (!(mcu_spm_read(MP1_CPU3_PWR_CON) & SRAM_PDN_ACK)) {
			/* SRAM PDN delay IP clock is 26MHz. Print SRAM control and ACK for debug. */
		}
		/* TINFO="Set SRAM_PD_SLPB_CLAMP = 1" */
		mcu_spm_write(MP1_CPU3_PWR_CON, mcu_spm_read(MP1_CPU3_PWR_CON) | SRAM_PD_SLPB_CLAMP);
		/* TINFO="Finish to turn off MP1_CPU3" */
	} else {    /* STA_POWER_ON */
		/* TINFO="Start to turn on MP1_CPU3" */
		/* TINFO="Set SRAM_SLPB = 0" */
		mcu_spm_write(MP1_CPU3_PWR_CON, mcu_spm_read(MP1_CPU3_PWR_CON) & ~SRAM_SLEEP_B);
		/* TINFO="Set PWR_RST_B = 0" */
		mcu_spm_write(MP1_CPU3_PWR_CON, mcu_spm_read(MP1_CPU3_PWR_CON) & ~PWR_RST_B);
		/* TINFO="Set PWR_CLK_DIS = 1" */
		mcu_spm_write(MP1_CPU3_PWR_CON, mcu_spm_read(MP1_CPU3_PWR_CON) | PWR_CLK_DIS);
		/* TINFO="Set SRAM_PDN = 1" */
		mcu_spm_write(MP1_CPU3_PWR_CON, mcu_spm_read(MP1_CPU3_PWR_CON) | SRAM_PDN);
		#ifndef DDR_RESERVE_ENABLE
		/* TINFO="Wait until MP1_CPU3_SRAM_PDN_ACK = 1" */
		while (!(mcu_spm_read(MP1_CPU3_PWR_CON) & SRAM_PDN_ACK)) {
			/* SRAM PDN delay IP clock is 26MHz. Print SRAM control and ACK for debug. */
		}
		#endif
		/* TINFO="Set SRAM_PD_SLPB_CLAMP = 0" */
		mcu_spm_write(MP1_CPU3_PWR_CON, mcu_spm_read(MP1_CPU3_PWR_CON) & ~SRAM_PD_SLPB_CLAMP);
		/* TINFO="Set SRAM_PDN = 0" */
		mcu_spm_write(MP1_CPU3_PWR_CON, mcu_spm_read(MP1_CPU3_PWR_CON) & ~(0x1 << 8));
		/* TINFO="Wait until MP1_CPU3_SRAM_PDN_ACK = 0" */
		while (mcu_spm_read(MP1_CPU3_PWR_CON) & SRAM_PDN_ACK) {
			/* SRAM PDN delay IP clock is 26MHz. Print SRAM control and ACK for debug. */
		}
		/* TINFO="Set PWR_ON = 1" */
		mcu_spm_write(MP1_CPU3_PWR_CON, mcu_spm_read(MP1_CPU3_PWR_CON) | PWR_ON);
		/* TINFO="Set PWR_ON_2ND = 1" */
		mcu_spm_write(MP1_CPU3_PWR_CON, mcu_spm_read(MP1_CPU3_PWR_CON) | PWR_ON_2ND);
#ifndef IGNORE_PWR_ACK
		/* TINFO="Wait until PWR_STATUS = 1 and PWR_STATUS_2ND = 1" */
		while (!(mcu_spm_read(PWR_STATUS) & MP1_CPU3_PWR_STA_MASK)
		       || !(mcu_spm_read(PWR_STATUS_2ND) & MP1_CPU3_PWR_STA_MASK)) {
			/* No logic between pwr_on and pwr_ack. Print SRAM / MTCMOS control and PWR_ACK for debug. */
		}
#endif
		/* TINFO="Set SRAM_SLPB = 1" */
		mcu_spm_write(MP1_CPU3_PWR_CON, mcu_spm_read(MP1_CPU3_PWR_CON) | SRAM_SLEEP_B);
		/* TINFO="Set SRAM_ISOINT_B = 1" */
		mcu_spm_write(MP1_CPU3_PWR_CON, mcu_spm_read(MP1_CPU3_PWR_CON) | SRAM_ISOINT_B);
		/* TINFO="Set PWR_ISO = 0" */
		mcu_spm_write(MP1_CPU3_PWR_CON, mcu_spm_read(MP1_CPU3_PWR_CON) & ~PWR_ISO);
		/* TINFO="Delay 1us" */
		udelay(1);
		/* TINFO="Set SRAM_CKISO = 0" */
		mcu_spm_write(MP1_CPU3_PWR_CON, mcu_spm_read(MP1_CPU3_PWR_CON) & ~SRAM_CKISO);
		/* TINFO="Set PWR_CLK_DIS = 0" */
		mcu_spm_write(MP1_CPU3_PWR_CON, mcu_spm_read(MP1_CPU3_PWR_CON) & ~PWR_CLK_DIS);
		/* TINFO="Set PWR_RST_B = 1" */
		mcu_spm_write(MP1_CPU3_PWR_CON, mcu_spm_read(MP1_CPU3_PWR_CON) | PWR_RST_B);
		/* TINFO="Finish to turn on MP1_CPU3" */
	}
	return err;
}
/*
 *
 */
int mtcmos_cluster0_cpu_on(unsigned int linear_id)
{
	INFO("%s(%d)\n", __FUNCTION__, linear_id);
	return 0;
}
int mtcmos_cluster0_cpu_off(unsigned int linear_id)
{
	INFO("%s(%d)\n", __FUNCTION__, linear_id);
#if 0
	uint32_t reg_pwr_con;
	uint32_t reg_l1_pdn;
	uint32_t bit_cpu;
	switch (linear_id) {
	case 1:
		reg_pwr_con = SPM_CA7_CPU1_PWR_CON;
		reg_l1_pdn = SPM_CA7_CPU1_L1_PDN;
		bit_cpu = LITTLE_CPU1;
		break;
	case 2:
		reg_pwr_con = SPM_CA7_CPU2_PWR_CON;
		reg_l1_pdn = SPM_CA7_CPU2_L1_PDN;
		bit_cpu = LITTLE_CPU2;
		break;
	case 3:
		reg_pwr_con = SPM_CA7_CPU3_PWR_CON;
		reg_l1_pdn = SPM_CA7_CPU3_L1_PDN;
		bit_cpu = LITTLE_CPU3;
		break;
	default:
		/* should never come to here */
		return;
	}
	/* enable register control */
	mmio_write_32(SPM_POWERON_CONFIG_SET,
			(SPM_PROJECT_CODE << 16) | (1U << 0));
	mmio_setbits_32(reg_pwr_con, PWR_ISO);
	mmio_setbits_32(reg_pwr_con, SRAM_CKISO);
	mmio_clrbits_32(reg_pwr_con, SRAM_ISOINT_B);
	mmio_setbits_32(reg_l1_pdn, L1_PDN);
	while (!(mmio_read_32(reg_l1_pdn) & L1_PDN_ACK))
		continue;
	mmio_clrbits_32(reg_pwr_con, PWR_RST_B);
	mmio_setbits_32(reg_pwr_con, PWR_CLK_DIS);
	mmio_clrbits_32(reg_pwr_con, PWR_ON);
	mmio_clrbits_32(reg_pwr_con, PWR_ON_2ND);
	while ((mmio_read_32(SPM_PWR_STATUS) & bit_cpu) ||
	       (mmio_read_32(SPM_PWR_STATUS_2ND) & bit_cpu))
		continue;
#endif
	return 0;
}
int mtcmos_cluster0_cpusys_on(unsigned int linear_id)
{
	INFO("power on CPUSYS_0 %s(%d)\n", __FUNCTION__, linear_id);
	return 0;
}
int mtcmos_cluster0_cpusys_off(unsigned int linear_id)
{
	INFO("power off CPUSYS_0 %s(%d)\n", __FUNCTION__, linear_id);
	return 0;
}
/*
 * Cluster 1
 */
int mtcmos_cluster1_cpu_on(unsigned int linear_id)
{
	INFO("%s(%d)\n", __FUNCTION__, linear_id);
	return 0;
}
int mtcmos_cluster1_cpu_off(unsigned int linear_id)
{
	INFO("%s(%d)\n", __FUNCTION__, linear_id);
	return 0;
}
int mtcmos_cluster1_cpusys_on(unsigned int linear_id)
{
	INFO("power on CPUSYS_1 %s(%d)\n", __FUNCTION__, linear_id);
	return 0;
}
int mtcmos_cluster1_cpusys_off(unsigned int linear_id)
{
	INFO("power off CPUSYS_1 %s(%d)\n", __FUNCTION__, linear_id);
	return 0;
}
/*
 *
 */
int mtcmos_cluster0_power(unsigned int onoff)
{
	if (onoff == 0) {
		mtcmos_cluster0_cpu_off(3);
		mtcmos_cluster0_cpu_off(2);
		mtcmos_cluster0_cpu_off(1);
	} else if (onoff == 1) {
		mtcmos_cluster0_cpu_on(1);
		mtcmos_cluster0_cpu_on(2);
		mtcmos_cluster0_cpu_on(3);
	} else {
		ERROR("%s(%d)\n", __FUNCTION__, onoff);
		return -1;
	}
	return 0;
}
int mtcmos_cluster1_power(unsigned int onoff)
{

	if (onoff == 0) {
		mtcmos_cluster1_cpu_off(7);
		mtcmos_cluster1_cpu_off(6);
		mtcmos_cluster1_cpu_off(5);
		mtcmos_cluster1_cpu_off(4);
	} else if (onoff == 1) {
		mtcmos_cluster1_cpu_on(4);
		mtcmos_cluster1_cpu_on(5);
		mtcmos_cluster1_cpu_on(6);
		mtcmos_cluster1_cpu_on(7);
	} else {
		ERROR("%s(%d)\n", __FUNCTION__, onoff);
		return -1;
	}
	return 0;
}


