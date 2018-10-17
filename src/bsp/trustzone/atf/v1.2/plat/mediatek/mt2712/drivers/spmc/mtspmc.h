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
#include <assert.h>
#include <console.h>
#include <debug.h>
#include <delay_timer.h>
#include <mcucfg.h>
#include <mmio.h>
#include <mtcmos.h>
#include <plat_private.h>
#include <platform.h>
#include <platform_def.h>
#include <psci.h>
#include <spm.h>
#include <artemis.h>

#ifndef __MTSPMC_H__
#define __MTSPMC_H__

#define RETRY_TIME_USEC   (10)
#define RETRY_TIME_CURR   (1)
#define DELAY_TIME_MEASURE (1)//(20000)//(1)

/* #define IGNORE_PWR_ACK		1 */
#define DDR_RESERVE_ENABLE	1
#define AO_REG_PATCH (1)

#ifdef SPMC_DVT
#define MP0_SNOOP_CTRL				(0x10395000)
#define MP1_SNOOP_CTRL				(0x10394000)
#define MP2_SNOOP_CTRL				(0x10396000)
#define MPx_SNOOP_STATUS			(0x1039000C)
#define MPx_SNOOP_ENABLE			(0x3)
#endif

/* APB Module infracfg_ao */
/*
#define INFRA_TOPAXI_PROTECTEN_1_SET (INFRACFG_AO_BASE + 0x2B4)
#define INFRA_TOPAXI_PROTECTEN_1_CLR (INFRACFG_AO_BASE + 0x2B8)
#define INFRA_TOPAXI_PROTECTSTA1_1	(INFRACFG_AO_BASE + 0x2BC)
*/

#define DBGSYS_MP2					0x100062B0

#define SPMC_MP2_CPU0_PWR_CON			SPMC_MP1_CPU0_PWR_CON
#define SPMC_MP2_CPU1_PWR_CON			SPMC_MP1_CPU1_PWR_CON
#define SPMC_MP2_CPUTOP_PWR_CON		SPMC_MP1_CPUTOP_PWR_CON
#define SPMC_MP2_CPUTOP_CLK_DIS		SPMC_MP1_CPUTOP_CLK_DIS
#define MP2_CPU0_STANDBYWFI			MP1_CPU0_STANDBYWFI
#define MP2_CPU1_STANDBYWFI			MP1_CPU1_STANDBYWFI
#define MP2_CPUTOP_STANDBYWFI			MP1_CPUTOP_STANDBYWFI


#define MCUSYS_PROTECTEN_SET		(INFRACFG_AO_BASE + 0x2B4)
#define MCUSYS_PROTECTEN_CLR		(INFRACFG_AO_BASE + 0x2B8)

#define MCUSYS_PROTECTEN_TOPAXI_PROT_EN		(INFRACFG_AO_BASE + 0x220)
#define MCUSYS_PROTECTEN_TOPAXI_PROT_EN_SET	(INFRACFG_AO_BASE + 0x260)
#define MCUSYS_PROTECTEN_TOPAXI_PROT_EN_CLR	(INFRACFG_AO_BASE + 0x264)
#define MCUSYS_PROTECTEN_TOPAXI_PROT_EN1		(INFRACFG_AO_BASE + 0x250)
#define MCUSYS_PROTECTEN_TOPAXI_PROT_EN1_SET	(INFRACFG_AO_BASE + 0x270)
#define MCUSYS_PROTECTEN_TOPAXI_PROT_EN1_CLR	(INFRACFG_AO_BASE + 0x274)

#define IDX_PROTECT_MP0_CACTIVE		(1<<29)
#define IDX_PROTECT_L2C0_CACTIVE	(1<<3)

#define IDX_PROTECT_MP1_CACTIVE		(1<<30)
#define IDX_PROTECT_L2C1_CACTIVE	(1<<4)

// TODO: ACP protection
#define IDX_PROTECT_ACP0_CACTIVE	16
#define IDX_PROTECT_ACP1_CACTIVE	17

#define DVTFlowCtrl				 (0x10200070)

#define mcu_spm_read(addr)		spm_read(addr)
#define mcu_spm_write(addr,  val)	spm_write(addr,  val)

/* #define FPGA_SMP	1 */
#define SPMC_SW_MODE	0
#define SPMC_DEBUG	1
#define SPMC_DVT	0
/* #define SPMC_DVT_UDELAY	0 */
#define SPMC_SPARK2	1
#define CONFIG_SPMC_MODE 1 /* 0:Legacy  1:HW  2:SW */

#define CPUTOP_MP0 0
#define CPUTOP_MP1 1
#define CPUTOP_MP2 2

#define STA_POWER_DOWN		0
#define STA_POWER_ON		1

#define MODE_SPMC_HW        0
#define MODE_AUTO_SHUT_OFF  1
#define MODE_DORMANT        2

#if SPMC_DEBUG
#define PRINTF_SPMC	tf_printf
#else
void __null_error(const char *fmt, ...) { }
#define PRINTF_SPMC	__null_error
#endif

int spmc_init(void);
int spmc_cputop_mpx_onoff(int cputop_mpx, int state, int mode);
int spmc_cpu_corex_onoff(int linear_id, int state, int mode);
int little_spmc_sw_pwr_on(int select);
int little_spmc_sw_pwr_off(int select);
int little_spark2_setldo(unsigned int linear_id);
int little_spark2_core_enable(unsigned int linear_id, unsigned int sw);
unsigned long read_cpuectlr(void);
void write_cpuectlr(unsigned long);
void set_cpu_retention_control(int retention_value);

extern unsigned int cpu_bitmask;
#endif /* __MTCMOS_H__ */
