/*
 * Copyright (c) 2014-2015, ARM Limited and Contributors. All rights reserved.
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
#ifndef __MCUCFG_H__
#define __MCUCFG_H__

#include <mt2712_def.h>
#include <stdint.h>

struct mt2712_mcucfg_regs {
	uint32_t mp0_ca7l_cache_config;
	struct {
		uint32_t mem_delsel0;
		uint32_t mem_delsel1;
	} mp0_cpu[4];				/* cpu1..3 is reserved */
	uint32_t mp0_cache_mem_delsel0;
	uint32_t mp0_cache_mem_delsel1;
	uint32_t mp0_axi_config;
	uint32_t mp0_misc_config[2];
	struct {				/* 0x38 */
		uint32_t rv_addr_lw;
		uint32_t rv_addr_hw;
	} mp0_rv_addr[4];
	uint32_t mp0_ca7l_cfg_dis; 		/* 0x58 */
	uint32_t mp0_ca7l_clken_ctrl;
	uint32_t mp0_ca7l_rst_ctrl;
	uint32_t mp0_ca7l_misc_config;
	uint32_t mp0_ca7l_dbg_pwr_ctrl;
	uint32_t mp0_rw_rsvd0;
	uint32_t mp0_rw_rsvd1;
	uint32_t mp0_ro_rsvd;			/* 0x74 */
	uint32_t reserved0_0[354];
	uint32_t mcusys_config_a;		/* 0x600 */
	uint32_t mcusys_config1_a;
	uint32_t mcusys_gic_peribase_a;
	uint32_t reserved3;
	uint32_t sec_range0_start;		/* 0x610 */
	uint32_t sec_range0_end;
	uint32_t sec_range_enable;
	uint32_t l2c_mm_base;
	uint32_t reserved0_1[8];
	uint32_t aclken_div;			/* 0x640 */
	uint32_t pclken_div;
	uint32_t l2c_sram_ctrl;
	uint32_t armpll_jit_ctrl;
	uint32_t cci_addrmap;			/* 0x650 */
	uint32_t cci_config;
	uint32_t cci_periphbase;
	uint32_t cci_nevntcntovfl;
	uint32_t cci_status;			/* 0x660 */
	uint32_t cci_acel_s1_ctrl;
	uint32_t bus_fabric_dcm_ctrl;
	uint32_t misc_dcm_ctrl;
	uint32_t xgpt_ctl;			/* 0x670 */
	uint32_t xgpt_idx;
	uint32_t reserve0_2[1764];

	/* Actually bigcore is MP2 here */
	uint32_t mp1_cpucfg;      		/* 0x2000 + 0x208 */
	uint32_t mp1_miscdbg;
	uint32_t reserved1_0[17];
	uint32_t mp1_rst_ctl;  			/* 0x2254 */
	uint32_t reserved1_1[2];
	uint32_t mp1_clkenm_div;		/* 0x2260 */
	uint32_t reserved1_2[11];
	struct { 				/* 0x2290 */
		uint32_t rv_addr_lw;
		uint32_t rv_addr_hw;
	} mp1_rv_addr[2];
	uint32_t mp1_cputop_spmc0;		/* 0x22a0 */
	uint32_t mp1_cputop_spmc1;
	uint32_t reserved1_3[98];
	uint32_t mp1_cpu0_spmc0;		/* 0x2430 */
	uint32_t mp1_cpu0_spmc1;
	uint32_t mp1_cpu1_spmc0;
	uint32_t mp1_cpu1_spmc1;
};

static struct mt2712_mcucfg_regs *const mt2712_mcucfg = (void *)MCUCFG_BASE;

/* cpu boot mode */
enum {
	MP0_CPUCFG_64BIT_SHIFT = 12,
	MP1_CPUCFG_64BIT_SHIFT = 16, /* 0x2208 */
	MP0_CPUCFG_64BIT = 0xf << MP0_CPUCFG_64BIT_SHIFT,
	MP1_CPUCFG_64BIT = 0xf << MP1_CPUCFG_64BIT_SHIFT
};

/* scu related */
enum {
	MP0_ACINACTM_SHIFT = 4,
	MP1_ACINACTM_SHIFT = 0,
	MP0_ACINACTM = 1 << MP0_ACINACTM_SHIFT,
	MP1_ACINACTM = 1 << MP1_ACINACTM_SHIFT
};

enum {
	MP1_DIS_RGU0_WAIT_PD_CPUS_L1_ACK_SHIFT = 0,
	MP1_DIS_RGU1_WAIT_PD_CPUS_L1_ACK_SHIFT = 4,
	MP1_DIS_RGU2_WAIT_PD_CPUS_L1_ACK_SHIFT = 8,
	MP1_DIS_RGU3_WAIT_PD_CPUS_L1_ACK_SHIFT = 12,
	MP1_DIS_RGU_NOCPU_WAIT_PD_CPUS_L1_ACK_SHIFT = 16,

	MP1_DIS_RGU0_WAIT_PD_CPUS_L1_ACK =
		0xf << MP1_DIS_RGU0_WAIT_PD_CPUS_L1_ACK_SHIFT,
	MP1_DIS_RGU1_WAIT_PD_CPUS_L1_ACK =
		0xf << MP1_DIS_RGU1_WAIT_PD_CPUS_L1_ACK_SHIFT,
	MP1_DIS_RGU2_WAIT_PD_CPUS_L1_ACK =
		0xf << MP1_DIS_RGU2_WAIT_PD_CPUS_L1_ACK_SHIFT,
	MP1_DIS_RGU3_WAIT_PD_CPUS_L1_ACK =
		0xf << MP1_DIS_RGU3_WAIT_PD_CPUS_L1_ACK_SHIFT,
	MP1_DIS_RGU_NOCPU_WAIT_PD_CPUS_L1_ACK =
		0xf << MP1_DIS_RGU_NOCPU_WAIT_PD_CPUS_L1_ACK_SHIFT
};

enum {
	MP1_AINACTS_SHIFT = 4,
	MP1_AINACTS = 1 << MP1_AINACTS_SHIFT
};

enum {
	MP1_SW_CG_GEN_SHIFT = 12,
	MP1_SW_CG_GEN = 1 << MP1_SW_CG_GEN_SHIFT
};

enum {
	MP1_L2RSTDISABLE_SHIFT = 14,
	MP1_L2RSTDISABLE = 1 << MP1_L2RSTDISABLE_SHIFT
};

#define SW_SPARK_EN			(1U << 0)
#define SW_NO_WAIT_FOR_Q_CHANNEL	(1U << 1)
#define MP0_CPUTOP_SPMC_CTL			(MCUCFG_BASE + 0x15C)

#define sw_spark_en                    (1<<0)
#define sw_no_wait_for_q_channel       (1<<1)
#define sw_fsm_override                (1<<2)
#define sw_logic_pre1_pdb              (1<<3)
#define sw_logic_pre2_pdb              (1<<4)
#define sw_logic_pdb                   (1<<5)
#define sw_iso                         (1<<6)
#define sw_sram_sleepb              (0x3F<<7)
#define sw_sram_isointb               (1<<13)
#define sw_clk_dis                    (1<<14)
#define sw_ckiso                      (1<<15)
#define sw_pd                      (0x3F<<16)
#define sw_hot_plug_reset             (1<<22)
#define sw_pwr_on_override_en         (1<<23)
#define sw_pwr_on                     (1<<24)
#define sw_coq_dis                    (1<<25)
#define logic_pdbo_all_off_ack        (1<<26)
#define logic_pdbo_all_on_ack         (1<<27)
#define logic_pre2_pdbo_all_on_ack    (1<<28)
#define logic_pre1_pdbo_all_on_ack    (1<<29)
#define CPUSYS0_CPU0_SPMC_CTL		(MCUCFG_BASE + 0x1c30)
#define CPUSYS0_CPU1_SPMC_CTL		(MCUCFG_BASE + 0x1c34)
#define CPUSYS0_CPU2_SPMC_CTL		(MCUCFG_BASE + 0x1c38)
#define CPUSYS0_CPU3_SPMC_CTL		(MCUCFG_BASE + 0x1c3C)

#define cpu_sw_spark_en				(1<<0)
#define cpu_sw_no_wait_for_q_channel	(1<<1)
#define cpu_sw_fsm_override			(1<<2)
#define cpu_sw_logic_pre1_pdb		(1<<3)
#define cpu_sw_logic_pre2_pdb		(1<<4)
#define cpu_sw_logic_pdb		(1<<5)
#define cpu_sw_iso				(1<<6)
#define cpu_sw_sram_sleepb		(1<<7)
#define cpu_sw_sram_isointb		(1<<8)
#define cpu_sw_clk_dis			(1<<9)
#define cpu_sw_ckiso			(1<<10)
#define cpu_sw_pd				(0x1F<<11)
#define cpu_sw_hot_plug_reset		(1<<16)
#define cpu_sw_powr_on_override_en	(1<<17)
#define cpu_sw_pwr_on			(1<<18)
#define cpu_spark2ldo_allswoff	(1<<19)
#define cpu_pdbo_all_on_ack		(1<<20)
#define cpu_pre2_pdbo_allon_ack		(1<<21)
#define cpu_pre1_pdbo_allon_ack		(1<<22)

#define MP2_CPUTOP_SPMC_CTL			(MCUCFG_BASE + 0x22A0)
#define MP2_CPUTOP_SPMC_STA			(MCUCFG_BASE + 0x22A4)
#define CPUSYS2_PWR_RST_CTL		(MCUCFG_BASE + 0x2008)
#define CPUSYS2_CPU0_SPMC_CTL	(MCUCFG_BASE + 0x2430)
#define CPUSYS2_CPU0_SPMC_STA	(MCUCFG_BASE + 0x2434)
#define CPUSYS2_CPU1_SPMC_CTL	(MCUCFG_BASE + 0x2438)
#define CPUSYS2_CPU1_SPMC_STA	(MCUCFG_BASE + 0x243C)

#define B_SW_HOT_PLUG_RESET		(1<<30)
#define B_SW_PD_OFFSET			(18)
#define B_SW_PD					(0x3f<<B_SW_PD_OFFSET)
#define B_SW_SRAM_SLEEPB_OFFSET	(12)
#define B_SW_SRAM_SLEEPB		(0x3f<<B_SW_SRAM_SLEEPB_OFFSET)
#define B_SW_SRAM_ISOINTB		(1<<9)
#define B_SW_ISO				(1<<8)
#define B_SW_LOGIC_PDB			(1<<7)
#define B_SW_LOGIC_PRE2_PDB		(1<<6)
#define B_SW_LOGIC_PRE1_PDB		(1<<5)
#define B_SW_FSM_OVERRIDE		(1<<4)
#define B_SW_PWR_ON				(1<<3)
#define B_SW_PWR_ON_OVERRIDE_EN	(1<<2)
#define B_SW_NO_WAIT_FOR_Q_CHANNEL	(1<<1)
#define B_SW_SPARK_EN			(1<<0)

#define B_FSM_STATE_OUT_OFFSET		(6)
#define B_FSM_STATE_OUT_MASK		        (0x1f <<B_FSM_STATE_OUT_OFFSET)
#define B_SW_LOGIC_PDBO_ALL_OFF_ACK	    (1<<5)
#define B_SW_LOGIC_PDBO_ALL_ON_ACK	    (1<<4)
#define B_SW_LOGIC_PRE2_PDBO_ALL_ON_ACK	(1<<3)
#define B_SW_LOGIC_PRE1_PDBO_ALL_ON_ACK	(1<<2)


#define B_FSM_OFF				(0<<B_FSM_STATE_OUT_OFFSET)
#define B_FSM_ON				(1<<B_FSM_STATE_OUT_OFFSET)
#define B_FSM_RET				(2<<B_FSM_STATE_OUT_OFFSET)

#define DBUGPWRDUP			(MCUCFG_BASE + 0x075C)
#define PTP3_B_COQ			(MCUCFG_BASE + 0x22BC)
#define B_SW_COQ_DIS		(1<<0)

#define MP0_AXI_CONFIG			(MCUCFG_BASE + 0x02C)
#define MPx_AXI_CONFIG_acinactm	(1<<4)
#define MPx_AXI_CONFIG_ainacts	(1<<5)

#define MP2_AXI_CONFIG			(MCUCFG_BASE + 0x220C)
#define MP2_AXI_CONFIG_acinactm                (1<<0)
#define MP2_AXI_CONFIG_ainacts                 (1<<4)

#define MP0_CA7_MISC_CONFIG		(MCUCFG_BASE + 0x064)
#define MP0_CPU0_STANDBYWFE                    (1<<20)
#define MP0_CPU1_STANDBYWFE                    (1<<21)
#define MP0_CPU2_STANDBYWFE                    (1<<22)
#define MP0_CPU3_STANDBYWFE                    (1<<23)

#define CA15M_MON_SEL			(MCUCFG_BASE + 0x2400)
#define MP1_CA15M_MON_L			(MCUCFG_BASE + 0x2404)
#define MP1_CPU0_STANDBYWFE						(1<<4)
#define MP1_CPU1_STANDBYWFE						(1<<5)

#define MP0_MISC_CONFIG_BOOT_ADDR(cpu)	(MCUCFG_BASE + 0x038 + ((cpu) * 8))
#define MP0_MISC_CONFIG3		(MCUCFG_BASE + 0x03C)
#define MP1_MISC_CONFIG_BOOT_ADDR(cpu)	(MCUCFG_BASE + 0x238 + ((cpu) * 8))
#define MP1_MISC_CONFIG3		(MCUCFG_BASE + 0x23C)

#define MP2_MISC_CONFIG_BOOT_ADDR_L(cpu)	(MCUCFG_BASE + 0x2290 + ((cpu) * 8))
#define MP2_MISC_CONFIG_BOOT_ADDR_H(cpu)	(MCUCFG_BASE + 0x2294 + ((cpu) * 8))

#define MP2_CPUCFG              (MCUCFG_BASE + 0x2208)

#endif  /* __MCUCFG_H__ */
