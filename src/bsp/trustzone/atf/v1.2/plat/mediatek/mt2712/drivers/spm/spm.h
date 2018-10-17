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
#ifndef __SPM_H__
#define __SPM_H__

#define SPM_ATF_LOG	1	/* spm trace log */
#define SPM_CCI_CHECK		1	/* spm check cci enable/disalbe with PCM AIO_0615_2 */

#define CLK_CFG_8		(0x100000d0)
#define INFRA_BIT0		(1U)
#define CLK_CFG_BIT24		(1U << 24)
#define CLK_CFG_BIT25		(1U << 25)

#define SPM_POWERON_CONFIG_SET			(SPM_BASE + 0x000)
#define SPM_POWER_ON_VAL0			(SPM_BASE + 0x010)
#define SPM_POWER_ON_VAL1			(SPM_BASE + 0x014)
#define SPM_CLK_SETTLE				(SPM_BASE + 0x100)
#define SPM_MP0_CPU0_PWR_CON			(SPM_BASE + 0x200)
#define SPM_MP0_CPUTOP_PWR_CON			(SPM_BASE + 0x208)
#define SPM_MP0_CPU1_PWR_CON			(SPM_BASE + 0x218)
#define SPM_MP0_CPU2_PWR_CON			(SPM_BASE + 0x21c)
#define SPM_MP0_CPU3_PWR_CON			(SPM_BASE + 0x220)
#define SPM_MP0_CPUTOP_SRM_PDN			(SPM_BASE + 0x244)
#define SPM_MP0_CPUTOP_SRM_SLEEP		(SPM_BASE + 0x248)
#define SPM_MP0_CPU0_SRM_PDN			(SPM_BASE + 0x25C)
#define SPM_MP0_CPU1_SRM_PDN			(SPM_BASE + 0x264)
#define SPM_MP0_CPU2_SRM_PDN			(SPM_BASE + 0x26c)
#define SPM_MP0_CPU3_SRM_PDN			(SPM_BASE + 0x274)
#define SPM_MD32_SRAM_CON			(SPM_BASE + 0x2c8)
#define SPM_PCM_CON0				(SPM_BASE + 0x310)
#define SPM_PCM_CON1				(SPM_BASE + 0x314)
#define SPM_PCM_IM_PTR				(SPM_BASE + 0x318)
#define SPM_PCM_IM_LEN				(SPM_BASE + 0x31c)
#define SPM_PCM_REG_DATA_INI			(SPM_BASE + 0x320)
#define SPM_PCM_EVENT_VECTOR0			(SPM_BASE + 0x340)
#define SPM_PCM_EVENT_VECTOR1			(SPM_BASE + 0x344)
#define SPM_PCM_EVENT_VECTOR2			(SPM_BASE + 0x348)
#define SPM_PCM_EVENT_VECTOR3			(SPM_BASE + 0x34c)
#define SPM_PCM_MAS_PAUSE_MASK			(SPM_BASE + 0x354)
#define SPM_PCM_PWR_IO_EN			(SPM_BASE + 0x358)
#define SPM_PCM_TIMER_VAL			(SPM_BASE + 0x35c)
#define SPM_PCM_TIMER_OUT			(SPM_BASE + 0x360)
#define SPM_PCM_REG0_DATA			(SPM_BASE + 0x380)
#define SPM_PCM_REG1_DATA			(SPM_BASE + 0x384)
#define SPM_PCM_REG2_DATA			(SPM_BASE + 0x388)
#define SPM_PCM_REG3_DATA			(SPM_BASE + 0x38c)
#define SPM_PCM_REG4_DATA			(SPM_BASE + 0x390)
#define SPM_PCM_REG5_DATA			(SPM_BASE + 0x394)
#define SPM_PCM_REG6_DATA			(SPM_BASE + 0x398)
#define SPM_PCM_REG7_DATA			(SPM_BASE + 0x39c)
#define SPM_PCM_REG8_DATA			(SPM_BASE + 0x3a0)
#define SPM_PCM_REG9_DATA			(SPM_BASE + 0x3a4)
#define SPM_PCM_REG10_DATA			(SPM_BASE + 0x3a8)
#define SPM_PCM_REG11_DATA			(SPM_BASE + 0x3ac)
#define SPM_PCM_REG12_DATA			(SPM_BASE + 0x3b0)
#define SPM_PCM_REG13_DATA			(SPM_BASE + 0x3b4)
#define SPM_PCM_REG14_DATA			(SPM_BASE + 0x3b8)
#define SPM_PCM_REG15_DATA			(SPM_BASE + 0x3bc)
#define SPM_PCM_EVENT_REG_STA			(SPM_BASE + 0x3c0)
#define SPM_PCM_FSM_STA				(SPM_BASE + 0x3c4)
#define SPM_PCM_IM_HOST_RW_PTR			(SPM_BASE + 0x3c8)
#define SPM_PCM_IM_HOST_RW_DAT			(SPM_BASE + 0x3cc)
#define SPM_PCM_EVENT_VECTOR4			(SPM_BASE + 0x3d0)
#define SPM_PCM_EVENT_VECTOR5			(SPM_BASE + 0x3d4)
#define SPM_PCM_EVENT_VECTOR6			(SPM_BASE + 0x3d8)
#define SPM_PCM_EVENT_VECTOR7			(SPM_BASE + 0x3dc)
#define SPM_PCM_SW_INT_SET			(SPM_BASE + 0x3e0)
#define SPM_PCM_SW_INT_CLEAR			(SPM_BASE + 0x3e4)
#define SPM_CLK_CON				(SPM_BASE + 0x400)
#define CPU_EXT_BUCK_ISO			(SPM_BASE + 0x404)
#define SPM_SLEEP_PTPOD2_CON			(SPM_BASE + 0x408)
#define SPM_APMCU_PWRCTL			(SPM_BASE + 0x600)
#define SPM_AP_DVFS_CON_SET			(SPM_BASE + 0x604)
#define SPM_AP_STANBY_CON			(SPM_BASE + 0x608)
#define SPM_PWR_STATUS				(SPM_BASE + 0x60c)
#define SPM_PWR_STATUS_2ND			(SPM_BASE + 0x610)
#define SPM_AP_BSI_REQ				(SPM_BASE + 0x614)
#define SPM_SLEEP_TIMER_STA			(SPM_BASE + 0x720)
#define SPM_SLEEP_WAKEUP_EVENT_MASK		(SPM_BASE + 0x810)
#define SPM_SLEEP_CPU_WAKEUP_EVENT		(SPM_BASE + 0x814)
#define SPM_SLEEP_MD32_WAKEUP_EVENT_MASK	(SPM_BASE + 0x818)
#define SPM_PCM_WDT_TIMER_VAL			(SPM_BASE + 0x824)
#define SPM_PCM_WDT_TIMER_OUT			(SPM_BASE + 0x828)
#define SPM_PCM_MD32_MAILBOX			(SPM_BASE + 0x830)
#define SPM_PCM_MD32_IRQ			(SPM_BASE + 0x834)
#define SPM_SLEEP_ISR_MASK			(SPM_BASE + 0x900)
#define SPM_SLEEP_ISR_STATUS			(SPM_BASE + 0x904)
#define SPM_SLEEP_ISR_RAW_STA			(SPM_BASE + 0x910)
#define SPM_SLEEP_MD32_ISR_RAW_STA		(SPM_BASE + 0x914)
#define SPM_SLEEP_WAKEUP_MISC			(SPM_BASE + 0x918)
#define SPM_SLEEP_BUS_PROTECT_RDY		(SPM_BASE + 0x91c)
#define SPM_SLEEP_SUBSYS_IDLE_STA		(SPM_BASE + 0x920)
#define SPM_PCM_RESERVE				(SPM_BASE + 0xb00)
#define SPM_PCM_RESERVE2			(SPM_BASE + 0xb04)
#define SPM_PCM_FLAGS				(SPM_BASE + 0xb08)
#define SPM_PCM_SRC_REQ				(SPM_BASE + 0xb0c)
#define SPM_PCM_DEBUG_CON			(SPM_BASE + 0xb20)
#define SPM_CA7_CPU0_IRQ_MASK			(SPM_BASE + 0xb30)
#define SPM_CA7_CPU1_IRQ_MASK			(SPM_BASE + 0xb34)
#define SPM_CA7_CPU2_IRQ_MASK			(SPM_BASE + 0xb38)
#define SPM_CA7_CPU3_IRQ_MASK			(SPM_BASE + 0xb3c)
#define SPM_CA15_CPU0_IRQ_MASK			(SPM_BASE + 0xb40)
#define SPM_CA15_CPU1_IRQ_MASK			(SPM_BASE + 0xb44)
#define SPM_CA15_CPU2_IRQ_MASK			(SPM_BASE + 0xb48)
#define SPM_CA15_CPU3_IRQ_MASK			(SPM_BASE + 0xb4c)
#define SPM_PCM_PASR_DPD_0			(SPM_BASE + 0xb60)
#define SPM_PCM_PASR_DPD_1			(SPM_BASE + 0xb64)
#define SPM_PCM_PASR_DPD_2			(SPM_BASE + 0xb68)
#define SPM_PCM_PASR_DPD_3			(SPM_BASE + 0xb6c)
#define SPM_PCM_EVENT_EN				(SPM_BASE + 0xc00)
#define SPM_PCM_EVENT_VECTOR8           (SPM_BASE + 0xc04)
#define SPM_PCM_EVENT_VECTOR9           (SPM_BASE + 0xc08)
#define SPM_PCM_EVENT_VECTORA           (SPM_BASE + 0xc0c)
#define SPM_PCM_EVENT_VECTORB           (SPM_BASE + 0xc10)
#define SPM_PCM_EVENT_VECTORC           (SPM_BASE + 0xc14)
#define SPM_PCM_EVENT_VECTORD           (SPM_BASE + 0xc18)
#define SPM_PCM_EVENT_VECTORE           (SPM_BASE + 0xc1c)
#define SPM_PCM_EVENT_VECTORF           (SPM_BASE + 0xc20)
#define SPM_PCM_RESERVE5                (SPM_BASE + 0xc24)
#define SPM_PCM_RESERVE6                (SPM_BASE + 0xc28)
#define SPM_PCM_RESERVE7                (SPM_BASE + 0xc2c)
#define SPM_PCM_RESERVE8                (SPM_BASE + 0xc30)
#define SPMC_MP0_CPU0_PWR_CON			(SPM_BASE + 0xc40)
#define SPMC_MP0_CPU1_PWR_CON			(SPM_BASE + 0xc44)
#define SPMC_MP0_CPU2_PWR_CON			(SPM_BASE + 0xc48)
#define SPMC_MP0_CPU3_PWR_CON			(SPM_BASE + 0xc4c)
#define SPMC_MP0_CPU4_PWR_CON			(SPM_BASE + 0xc60)
#define SPMC_MP0_CPU5_PWR_CON			(SPM_BASE + 0xc64)
#define SPMC_MP0_CPUTOP_PWR_CON		(SPM_BASE + 0xc50)
#define SPMC_MP0_CPUTOP_CLK_DIS		(SPM_BASE + 0xc58)
#define SPMC_MP1_CPUTOP_CLK_DIS		(SPM_BASE + 0xc78)
#define SPMC_BYPASS				(SPM_BASE + 0xc5c)
#define SPMC_MP1_CPU0_PWR_CON			(SPM_BASE + 0xc60)
#define SPMC_MP1_CPU1_PWR_CON			(SPM_BASE + 0xc64)
#define SPMC_MP1_CPUTOP_PWR_CON			(SPM_BASE + 0xc70)
#define SPM_SLEEP_CA7_WFI0_EN			(SPM_BASE + 0xf00)
#define SPM_SLEEP_CA7_WFI1_EN			(SPM_BASE + 0xf04)
#define SPM_SLEEP_CA7_WFI2_EN			(SPM_BASE + 0xf08)
#define SPM_SLEEP_CA7_WFI3_EN			(SPM_BASE + 0xf0c)
#define SPM_SLEEP_CA15_WFI0_EN			(SPM_BASE + 0xf10)
#define SPM_SLEEP_CA15_WFI1_EN			(SPM_BASE + 0xf14)
#define SPM_SLEEP_CA15_WFI2_EN			(SPM_BASE + 0xf18)
#define SPM_SLEEP_CA15_WFI3_EN			(SPM_BASE + 0xf1c)

#if 1
/* To DO */
#define SPM_PCM_MMDDR_MASK              (SPM_BASE + 0xb1c)
#define SPM_SPMC_MP0_CPU0_PWR_CON       (SPM_BASE + 0xc40)
#define SPM_SPMC_MP0_CPU1_PWR_CON       (SPM_BASE + 0xc44)
#define SPM_SPMC_MP0_CPU2_PWR_CON       (SPM_BASE + 0xc48)
#define SPM_SPMC_MP0_CPU3_PWR_CON       (SPM_BASE + 0xc4c)
#define SPM_SPMC_MP0_CPUTOP_PWR_CON     (SPM_BASE + 0xc50)
#define SPM_SPMC_MP0_SRAM_SLP           (SPM_BASE + 0xc54)
#define SPM_SPMC_MP0_CPUTOP_CLK_DIS     (SPM_BASE + 0xc58)
#define SPM_SPMC_BYPASS                 (SPM_BASE + 0xc5c)
#define SPM_DPY_MISC                    (SPM_BASE + 0xc60)
#define SPM_SLEEP_CA7_WFI0_EN           (SPM_BASE + 0xf00)
#define SPM_SLEEP_CA7_WFI1_EN           (SPM_BASE + 0xf04)
#define SPM_SLEEP_CA7_WFI2_EN           (SPM_BASE + 0xf08)
#define SPM_SLEEP_CA7_WFI3_EN           (SPM_BASE + 0xf0c)
#endif


#define AP_PLL_CON3		0x1020900c
#define AP_PLL_CON4		0x10209010

#define SPM_IRQ0_PHY_ID		187

#define SPM_PROJECT_CODE	0xb16

#define SPM_REGWR_EN		(1U << 0)
#define SPM_REGWR_CFG_KEY	(SPM_PROJECT_CODE << 16)

/* #define SPMC_MP0_CPU0_PWR_CON (SPM_BASE + 0xc40) */
/* #define SPMC_MP0_CPU1_PWR_CON (SPM_BASE + 0xc44) */
/* #define SPMC_MP0_CPU2_PWR_CON (SPM_BASE + 0xc48) */
/* #define SPMC_MP0_CPU3_PWR_CON (SPM_BASE + 0xc4c) */
#define	SPMC_PWR_ON_2ND		(1U << 0)
#define	SPMC_PWR_RST_B		(1U << 1)
#define	SPMC_PWR_ON			(1U << 2)
#define	SPMC_PWR_CLK_DIS	(1U << 0)

/* #define SPM_MP0_CPU1_PWR_CON	(SPM_BASE + 0x218) */
/* #define SPM_MP0_CPU2_PWR_CON	(SPM_BASE + 0x21c) */
/* #define SPM_MP0_CPU3_PWR_CON	(SPM_BASE + 0x220) */
#define	PWR_RST_B	(1U << 0)
#define	PWR_ISO		(1U << 1)
#define	PWR_ON		(1U << 2)
#define	PWR_ON_2ND	(1U << 3)
#define	PWR_CLK_DIS	(1U << 4)
#define	SRAM_CKISO	(1U << 5)
#define	SRAM_ISOINT_B	(1U << 6)
#define PD_SLPB_CLAMP	(1U << 7)
#define	DDRPHY_MISC	(1U << 12)

/* SPM_MP0_CPUTOP_SRM_SLEEP		(SPM_BASE + 0x248) */
#define MEM_SLP_B	(1U << 0)

/* SPM_MP0_CPU0_SRM_PDN			(SPM_BASE + 0x25C) */
/* SPM_MP0_CPU1_SRM_PDN			(SPM_BASE + 0x264) */
/* SPM_MP0_CPU2_SRM_PDN			(SPM_BASE + 0x26c) */
/* SPM_MP0_CPU3_SRM_PDN			(SPM_BASE + 0x274) */
#define MEM_PD		(1U << 0)
#define MEM_PD_ACK	(1U << 8)

/* SPM_PWR_STATUS 	(SPM_BASE + 0x60c) */
/* SPM_PWR_STATUS_2ND	(SPM_BASE + 0x610)*/
#define	MP0_CPUTOP	(1U << 8)
#define	MP0_CPU3	(1U << 12)
#define	MP0_CPU2	(1U << 11)
#define	MP0_CPU1	(1U << 10)
#define	MP0_CPU0	(1U << 9)

#define PCM_SW_INT0                     (1U << 0)
#define PCM_SW_INT1                     (1U << 1)
#define PCM_SW_INT2                     (1U << 2)
#define PCM_SW_INT3                     (1U << 3)
#define PCM_SW_INT4                     (1U << 4)
#define PCM_SW_INT5                     (1U << 5)
#define PCM_SW_INT6                     (1U << 6)
#define PCM_SW_INT7                     (1U << 7)
#define PCM_SW_INT_ALL          (PCM_SW_INT7 | PCM_SW_INT6 | PCM_SW_INT5 |      \
                                                         PCM_SW_INT4 | PCM_SW_INT3 | PCM_SW_INT2 |      \
                                                         PCM_SW_INT1 | PCM_SW_INT0)

/* SPM_SLEEP_TIMER_STA		(SPM_BASE + 0x720) */
#define	MP0_CPU0_STANDBYWFI	(1U << 16)
#define	MP0_CPU1_STANDBYWFI	(1U << 17)
#define	MP0_CPU2_STANDBYWFI	(1U << 18)
#define	MP0_CPU3_STANDBYWFI	(1U << 19)
#define	MP1_CPU0_STANDBYWFI	(1U << 20)
#define	MP1_CPU1_STANDBYWFI	(1U << 21)
#define	MP1_CPU2_STANDBYWFI	(1U << 22)
#define	MP1_CPU3_STANDBYWFI	(1U << 23)
#define	MP0_CPUTOP_STANDBYWFI	(1U << 24)
#define	MP1_CPUTOP_STANDBYWFI	(1U << 25)

//Old define for MT8173, no longer to support
#define SPM_CPU_PDN_DIS		(0)
#define SPM_INFRA_PDN_DIS	(0)
#define SPM_DDRPHY_PDN_DIS	(0)
#define SPM_DUALVCORE_PDN_DIS	(0)
#define SPM_PASR_DIS		(0)
#define SPM_DPD_DIS		(0)
#define SPM_SODI_DIS		(0)
#define SPM_MEMPLL_RESET	(0)
#define SPM_MAINPLL_PDN_DIS	(0)
#define SPM_CPU_DVS_DIS		(0)
#define SPM_CPU_DORMANT		(0)
#define SPM_EXT_VSEL_GPIO103   (0)
#define SPM_DDR_HIGH_SPEED     (0)
#define SPM_OPT                (0)
#define SPM_MCUSYS_PDN_DIS     (0)

//---PCM FLAGS BIT DEFINE (PDEF_PCM_FLAGS)
#define PDEF_DISABLE_CPU_PDN	(1U << 0)
#define PDEF_DISABLE_CPUTOP_PDN (1U << 1)
#define PDEF_DISABLE_L2_DORMANT (1U << 2)
#define PDEF_DISABLE_MCU_PDN	(1U << 3)
#define PDEF_DISABLE_BUS26M	(1U << 4)
#define PDEF_DISABLE_MPLLOFF	(1U << 5)
#define PDEF_DISABLE_DDRPHY_S1	(1U << 6)
#define PDEF_DISABLE_DDRPHY_S0	(1U << 7)
#define PDEF_DISABLE_FHC_SLEEP	(1U << 8)
#define PDEF_DISABLE_VPROC_LOW	(1U << 9)
#define PDEF_DISABLE_INFRA_PDN	(1U << 10)
#define PDEF_DISABLE_26M	(1U << 11)
#define PDEF_DISABLE_26M_OFF	(1U << 12)
#define PDEF_DISABLE_XO_OFF	(1U << 13)
#define PDEF_DISABLE_32K	(1U << 14)
#define PDEF_DISABLE_PLL_OFF	(1U << 15)
#define PDEF_DISABLE_VCORE_PDN	(1U << 16)

#define POWER_ON_VAL1_DEF	0x01011820
#define PCM_FSM_STA_DEF		0x48490
#define PCM_END_FSM_STA_DEF	0x08490
#define PCM_END_FSM_STA_MASK	0x7fff0
#define PCM_HANDSHAKE_SEND1	0xbeefbeef

#define PCM_WDT_TIMEOUT		(30 * 32768)
#define PCM_TIMER_MAX		(0xffffffff - PCM_WDT_TIMEOUT)

#define CON0_PCM_KICK		(1U << 0)
#define CON0_IM_KICK		(1U << 1)
#define CON0_IM_SLEEP_DVS	(1U << 3)
#define CON0_PCM_SW_RESET	(1U << 15)
#define CON0_CFG_KEY		(SPM_PROJECT_CODE << 16)

#define CON1_IM_SLAVE		(1U << 0)
#define CON1_MIF_APBEN		(1U << 3)
#define CON1_PCM_TIMER_EN	(1U << 5)
#define CON1_IM_NONRP_EN	(1U << 6)
#define CON1_PCM_WDT_EN		(1U << 8)
#define CON1_PCM_WDT_WAKE_MODE	(1U << 9)
#define CON1_SPM_SRAM_SLP_B	(1U << 10)
#define CON1_SPM_SRAM_ISO_B	(1U << 11)
#define CON1_EVENT_LOCK_EN	(1U << 12)
#define CON1_CFG_KEY		(SPM_PROJECT_CODE << 16)

#define PCM_PWRIO_EN_R0		(1U << 0)
#define PCM_PWRIO_EN_R7		(1U << 7)
#define PCM_RF_SYNC_R0		(1U << 16)
#define PCM_RF_SYNC_R2		(1U << 18)
#define PCM_RF_SYNC_R5		(1U << 21)
#define PCM_RF_SYNC_R6		(1U << 22)
#define PCM_RF_SYNC_R7		(1U << 23)

#define CC_SYSCLK0_EN_0		(1U << 0)
#define CC_SYSCLK0_EN_1		(1U << 1)
#define CC_SYSCLK1_EN_0		(1U << 2)
#define CC_SYSCLK1_EN_1		(1U << 3)
#define CC_SYSSETTLE_SEL	(1U << 4)
#define CC_LOCK_INFRA_DCM	(1U << 5)
#define CC_SRCLKENA_MASK_0	(1U << 6)
#define CC_CXO32K_RM_EN_MD1	(1U << 9)
#define CC_CXO32K_RM_EN_MD2	(1U << 10)
#define CC_CLKSQ1_SEL		(1U << 12)
#define CC_DISABLE_DORM_PWR	(1U << 14)
#define CC_MD32_DCM_EN		(1U << 18)

#define ISRM_TWAM                       (1U << 2)
#define ISRM_PCM_RETURN         (1U << 3)
#define ISRM_RET_IRQ0           (1U << 8)
#define ISRM_RET_IRQ1           (1U << 9)
#define ISRM_RET_IRQ2           (1U << 10)
#define ISRM_RET_IRQ3           (1U << 11)
#define ISRM_RET_IRQ4           (1U << 12)
#define ISRM_RET_IRQ5           (1U << 13)
#define ISRM_RET_IRQ6           (1U << 14)
#define ISRM_RET_IRQ7           (1U << 15)

#define ISRM_RET_IRQ_AUX        (ISRM_RET_IRQ7 | ISRM_RET_IRQ6 |        \
                                                         ISRM_RET_IRQ5 | ISRM_RET_IRQ4 |        \
                                                         ISRM_RET_IRQ3 | ISRM_RET_IRQ2 |        \
                                                         ISRM_RET_IRQ1)
#define ISRM_ALL_EXC_TWAM       (ISRM_RET_IRQ_AUX | ISRM_RET_IRQ0 | ISRM_PCM_RETURN)
#define ISRM_ALL                        (ISRM_ALL_EXC_TWAM | ISRM_TWAM)

#define ISRS_TWAM                       (1U << 2)
#define ISRS_PCM_RETURN         (1U << 3)
#define ISRS_SW_INT0            (1U << 4)

#define ISRC_TWAM                       ISRS_TWAM
#define ISRC_ALL_EXC_TWAM       ISRS_PCM_RETURN
#define ISRC_ALL                        (ISRC_ALL_EXC_TWAM | ISRC_TWAM)

#define CC_SYSCLK0_EN_0         (1U << 0)
#define CC_SYSCLK0_EN_1         (1U << 1)
#define CC_SYSCLK1_EN_0         (1U << 2)
#define CC_SYSCLK1_EN_1         (1U << 3)
#define CC_SYSSETTLE_SEL        (1U << 4)
#define CC_LOCK_INFRA_DCM       (1U << 5)
#define CC_SRCLKENA_MASK_0      (1U << 6)
#define CC_CXO32K_RM_EN_MD1     (1U << 9)
#define CC_CXO32K_RM_EN_MD2     (1U << 10)
#define CC_CLKSQ1_SEL           (1U << 12)
#define CC_DISABLE_DORM_PWR     (1U << 14)
#define CC_MD32_DCM_EN          (1U << 18)

#define ASC_SRCCLKENI_MASK      (1U << 25)

#define WFI_OP_AND		1
#define WFI_OP_OR		0

#define WAKE_MISC_PCM_TIMER	(1U << 19)
#define WAKE_MISC_CPU_WAKE	(1U << 20)

/* MT2712 remove these wake up source */
#define WAKE_SRC_LOW_BAT            0
#define WAKE_SRC_PMIC_EINT_0        0
#define WAKE_SRC_PMIC_EINT_1        0
#define WAKE_SRC_USB_PDN            0
#define WAKE_SRC_USB_CD             0
#define WAKE_SRC_SPM_MERGE     		0

/* define WAKE_SRC_XXX */
#define WAKE_SRC_KP                 (1U << 2)
#define WAKE_SRC_WDT                (1U << 3)
#define WAKE_SRC_GPT                (1U << 4)
#define WAKE_SRC_GPT_MD32           (1U << 5)
#define WAKE_SRC_EINT               (1U << 6)
#define WAKE_SRC_EINT_MD32          (1U << 7)
#define WAKE_SRC_PE2_P0	            (1U << 8)  /* MT2712 New Add */
#define WAKE_SRC_PE2_P1             (1U << 9)  /* MT2712 New Add */
#define WAKE_SRC_MD32               (1U << 10) /* MT2712 remove MD32 and add CM4, keep this wake up source for CM4 */
#define WAKE_SRC_RTC                (1U << 14) /* MT2712 New Add */
#define WAKE_SRC_USB_INIT_C         (1U << 15) /* MT2712 New Add */
#define WAKE_SRC_IRRX	            (1U << 17) /* MT2712 New Add */
#define WAKE_SRC_PMIC_ME32          (1U << 18)
#define WAKE_SRC_UART0              (1U << 19)
#define WAKE_SRC_AFE                (1U << 20)
#define WAKE_SRC_THERM              (1U << 21)
#define WAKE_SRC_CIRQ               (1U << 22)
#define WAKE_SRC_AUD_MD32           (1U << 23)
#define WAKE_SRC_SYSPWREQ           (1U << 24)
#define WAKE_SRC_MD_WDT             (1U << 25)
#define WAKE_SRC_USB_INIT_D         (1U << 26) /* MT2712 New Add */
#define WAKE_SRC_SEJ                (1U << 27)
#define WAKE_SRC_ALL_MD32           (1U << 28)
#define WAKE_SRC_CPU_IRQ            (1U << 29)

#define PCM_HOTPLUG_MCDI_EXEC	0xbeef0000
#define PCM_HOTPLUG_MCDI_STANDARD_OFFLOAD	0xabcd0000
#define PCM_HOTPLUG_MCDI_DIR_OFFLOAD	0xdcba0000
#define PCM_HOTPLUG_MCDI_CMD_MASK	0xffff0000
#define PCM_HOTPLUG_MCDI_EXIT	0xdead

#define SPM_CA72_CPU_PWRSTA_SHIFT		16
#define SPM_CA53_CPU_PWRSTA_SHIFT		9
#define SPM_CA72_CPUTOP_PWRSTA_SHIFT	15
#define SPM_CA53_CPUTOP_PWRSTA_SHIFT	8

#define AP_MOVE_PCM_CODE			(0)

/* SPM Mapping limit */
#define SOC_REGISTER_ADDRESS			(0x40000000) /* Address for Soc Register is under 0x40000000 */
#define OFFSET_ADDRESS_FOR_4GB			(0x40000000)
#define SPM_MAPPING_LIMIT			(OFFSET_ADDRESS_FOR_4GB + 0x80000000)

enum {
        CG_CTRL0,
        CG_CTRL1,
        CG_CTRL2,
        CG_CTRL8,
        CG_MMSYS0,
        CG_MMSYS1,
        CG_IMGSYS,
        CG_MFGSYS,
        CG_AUDIO,
        CG_VDEC0,
        CG_VDEC1,
        NR_CG_GRPS,
};

#define NR_CG_GRPS	12
#define INVALID_GRP_ID(grp)	\
	if (grp < 0 || grp >= NR_CG_GRPS)	\
			ERROR("invalid clock group id=%d\n", grp);	\

#define BIT(x)	(0x1 << x)

#define PCM_CMD_MASK				0xffff0000
#define PCM_CMD_SUSPEND_PCM			0x50d10000
#define PCM_CMD_HP_MCDI_PCM			0xbeef0000
#define PCM_CMD_HP_MCDI_STANDARD_OFFLOAD	0xabcd0000
#define PCM_CMD_HP_MCDI_DIR_OFFLOAD		0xdcba0000
#define PCM_STA_HP_MCDI_EXIT			0xdead

#define SPM_CA35_CPU_PWRSTA_SHIFT	9

#define TOP_DCMCTL              (INFRACFG_AO_BASE + 0x10)
#define PCM_NEED_RELOAD		1

enum wake_reason_t {
	WR_NONE = 0,
	WR_UART_BUSY = 1,
	WR_PCM_ASSERT = 2,
	WR_PCM_TIMER = 3,
	WR_PCM_ABORT = 4,
	WR_WAKE_SRC = 5,
	WR_UNKNOWN = 6,
};

struct pwr_ctrl {
	unsigned int pcm_flags;
	unsigned int pcm_flags_cust;
	unsigned int pcm_reserve;
	unsigned int timer_val;
	unsigned int timer_val_cust;
	unsigned int wake_src;
	unsigned int wake_src_cust;
	unsigned short r0_ctrl_en;
	unsigned short r7_ctrl_en;
	unsigned short infra_dcm_lock;
	unsigned short pcm_apsrc_req;
	unsigned short pcm_f26m_req;

	unsigned short mcusys_idle_mask;
	unsigned short ca15top_idle_mask;
	unsigned short ca7top_idle_mask;
	unsigned short wfi_op;
	unsigned short ca15_wfi0_en;
	unsigned short ca15_wfi1_en;
	unsigned short ca15_wfi2_en;
	unsigned short ca15_wfi3_en;
	unsigned short ca7_wfi0_en;
	unsigned short ca7_wfi1_en;
	unsigned short ca7_wfi2_en;
	unsigned short ca7_wfi3_en;

	unsigned short conn_mask;

	unsigned short gce_req_mask;
	unsigned short disp0_req_mask;
	unsigned short disp1_req_mask;
	unsigned short mfg_req_mask;
	unsigned short vdec_req_mask;
	unsigned short mm_ddr_req_mask;

	unsigned short syspwreq_mask;
	unsigned short srclkenai_mask;

	unsigned short param1;
	unsigned short param2;
	unsigned short param3;
};

struct wake_status {
	unsigned int assert_pc;
	unsigned int r12;
	unsigned int raw_sta;
	unsigned int wake_misc;
	unsigned int timer_out;
	unsigned int r13;
	unsigned int idle_sta;
	unsigned int debug_flag;
	unsigned int event_reg;
	unsigned int isr;
};

struct pcm_desc {
	const char *version;		/* PCM code version */
	const unsigned int *base;	/* binary array base */
	const unsigned short size;	/* binary array size */
	const unsigned char sess;	/* session number */
	const unsigned char replace;	/* replace mode */
	const unsigned char addr_2nd;

	unsigned int vec0;		/* event vector 0 config */
	unsigned int vec1;		/* event vector 1 config */
	unsigned int vec2;		/* event vector 2 config */
	unsigned int vec3;		/* event vector 3 config */
	unsigned int vec4;		/* event vector 4 config */
	unsigned int vec5;		/* event vector 5 config */
	unsigned int vec6;		/* event vector 6 config */
	unsigned int vec7;		/* event vector 7 config */
	unsigned int vec8;		/* event vector 8 config */
	unsigned int vec9;		/* event vector 9 config */
	unsigned int vec10;		/* event vector A config */
	unsigned int vec11;		/* event vector B config */
	unsigned int vec12;		/* event vector C config */
	unsigned int vec13;		/* event vector D config */
	unsigned int vec14;		/* event vector E config */
	unsigned int vec15;		/* event vector F config */
};

struct spm_lp_scen {
	const struct pcm_desc *pcmdesc;
	struct pwr_ctrl *pwrctrl;
};

#define EVENT_VEC(event, resume, imme, pc)	\
	(((pc) << 16) |				\
	 (!!(imme) << 7) |			\
	 (!!(resume) << 6) |			\
	 ((event) & 0x3f))

#define spm_read(addr)		mmio_read_32(addr)
#define spm_write(addr, val)	mmio_write_32(addr, val)

#define is_cpu_pdn(flags)	(!((flags) & SPM_CPU_PDN_DIS))
#define is_infra_pdn(flags)	(!((flags) & SPM_INFRA_PDN_DIS))
#define is_ddrphy_pdn(flags)	(!((flags) & SPM_DDRPHY_PDN_DIS))

static inline void set_pwrctrl_pcm_flags(struct pwr_ctrl *pwrctrl,
					 unsigned int flags)
{
	flags &= ~SPM_EXT_VSEL_GPIO103;

	if (pwrctrl->pcm_flags_cust == 0)
		pwrctrl->pcm_flags = flags;
	else
		pwrctrl->pcm_flags = pwrctrl->pcm_flags_cust;
}

static inline void set_pwrctrl_pcm_data(struct pwr_ctrl *pwrctrl,
					unsigned int data)
{
	pwrctrl->pcm_reserve = data;
}

unsigned int spm_is_pass_clock_check(unsigned int *condition, unsigned int *block);
void spm_dump_clock_masks(unsigned int *condition, unsigned int *block);
unsigned int spm_is_last_online_cpu(unsigned long mpidr);
enum pmic_wrap_phase_id {
	PMIC_WRAP_PHASE_DEEPIDLE,
	PMIC_WRAP_PHASE_SODI,
	NR_PMIC_WRAP_PHASE,
};
void spm_set_pmic_phase(enum pmic_wrap_phase_id phase);
void spm_regs_save(void);
void spm_regs_restore(void);

enum {
	IDLE_TYPE_DP = 0,
	IDLE_TYPE_SO = 1,
	IDLE_TYPE_ALL = 2,
	NR_IDLE_TYPES = IDLE_TYPE_ALL,
};
enum {
	CMD_TYPE_SW_SWITCH = 0,
	CMD_TYPE_ENABLE_MASK_BIT = 1,
	CMD_TYPE_DISABLE_MASK_BIT = 2,
	CMD_TYPE_BYPASS = 3,
	NR_CMD_TYPES = 4,
};
enum {
	BY_SWT = 0,	/* idle_switch */
	BY_CPU = 1,	/* not little cpu or not last cpu */
	BY_CLK = 2,	/* clock condition */
	BY_SCN = 3,	/* previous scenario */
	BY_OFL = 4,	/* offload fail */
	NR_REASONS = 5,
};
void spm_debug_read(unsigned int idle_type);
void spm_debug_write(unsigned int idle_type, unsigned int cmd, unsigned int val);

void spm_reset_and_init_pcm(void);

void spm_init_pcm_register(void);	/* init r0 and r7 */
void spm_set_power_control(const struct pwr_ctrl *pwrctrl);
void spm_set_wakeup_event(const struct pwr_ctrl *pwrctrl);

void spm_reset_and_init_pcm(void);
void spm_init_pcm_register(void);	/* init r0 and r7 */
void spm_set_power_control(const struct pwr_ctrl *pwrctrl);
void spm_set_pcmwdt(void);
void spm_set_wakeup_event(const struct pwr_ctrl *pwrctrl);

void spm_get_wakeup_status(struct wake_status *wakesta);
void spm_set_sysclk_settle(void);
void spm_kick_pcm_to_run(struct pwr_ctrl *pwrctrl);
void spm_clean_after_wakeup(void);
enum wake_reason_t spm_output_wake_reason(struct wake_status *wakesta);
void spm_set_pcm_cmd(unsigned int cmd);
void spm_register_init(void);
void spm_init_event_vector(const struct pcm_desc *pcmdesc);
void spm_kick_im_to_fetch(const struct pcm_desc *pcmdesc);
void spm_set_sysclk_settle(void);
int is_hotplug_mcdi_ready(void);
int is_sodi_ready(void);
int is_dpidle_ready(void);
int is_suspend_ready(void);
int is_pcm_ready(void);
void set_hotplug_mcdi_ready(void);
void set_sodi_ready(void);
void set_dpidle_ready(void);
void set_suspend_ready(void);

void spm_lock_init(void);
void spm_lock_get(void);
void spm_lock_release(void);
void spm_boot_init(void);
#if PCM_NEED_RELOAD
void spm_reload_pcm_code(void);
#endif

#if SPM_CCI_CHECK
void spm_cci_on(unsigned long mpidr);
void spm_cci_off(unsigned long mpidr);
#endif /* SPM_CCI_CHECK */

#if SPM_ATF_LOG
#define LOG_HOTPLUG_OFF		(0x0010 << 16)
#define LOG_HOTPLUG_ON		(0x0011 << 16)
#define LOG_HOTPLUG_ON_F	(0x0012 << 16)
#define LOG_HOTPLUG_ON_F0	(0x0013 << 16)
#define LOG_HOTPLUG_C2I		(0x0014 << 16)
#define LOG_HOTPLUG_C2O		(0x0015 << 16)
#define LOG_MCDI_OFF		(0x0020 << 16)
#define LOG_MCDI_ON		(0x0021 << 16)
#define LOG_MCDI_C2I		(0x0024 << 16)
#define LOG_MCDI_C2O		(0x0025 << 16)
#define LOG_WAKEUP		(0x0031 << 16)
#define LOG_CPU0		0x0001
#define LOG_CPU1		0x0002
#define LOG_CPU2		0x0004
#define LOG_CPU3		0x0008
#define LOG_CPU4		0x0010
#define LOG_CPU5		0x0020
#define LOG_CLUSTER0		0x1000
#define LOG_CLUSTER1		0x2000

void spm_trace(uint32_t msg);
#endif /* SPM_ATF_LOG */
#endif /* __SPM_H__ */
