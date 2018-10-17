/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 * the prior written permission of MediaTek inc. and/or its licensors, any
 * reproduction, modification, use or disclosure of MediaTek Software, and
 * information contained herein, in whole or in part, shall be strictly
 * prohibited.
 *
 * MediaTek Inc. (C) 2010. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 * ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 * NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
 * RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 * INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
 * TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 * RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 * OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
 * SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
 * RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
 * ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
 * RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
 * MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 * CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek
 * Software") have been modified by MediaTek Inc. All revisions are subject to
 * any receiver's applicable license agreements with MediaTek Inc.
 */
#include <arch_helpers.h>
#include <debug.h>
#include <mmio.h>
#include <mt_cpuxgpt.h>
#include <platform_def.h>

#define CPUXGPT_BASE	0x10200000
#define INDEX_BASE	(CPUXGPT_BASE+0x0674)
#define CTL_BASE	(CPUXGPT_BASE+0x0670)

#define GPT_BIT_MASK_L 0x00000000FFFFFFFF
#define GPT_BIT_MASK_H 0xFFFFFFFF00000000

uint64_t	normal_time_base;
uint64_t	atf_time_base;

#define MT_CPUXGPT_ENABLE
#ifdef MT_CPUXGPT_ENABLE

/*need add volatile keyword*/
static inline uint64_t mt_read_cntpct_el0(void)
{
	uint64_t v;

	__asm__ __volatile__("mrs %0,  cntpct_el0" : "=r" (v));

	return v;
}

static uint32_t __read_cpuxgpt(uint32_t reg_index)
{
	uint32_t value = 0;

	mmio_write_32(INDEX_BASE, reg_index);
	value = mmio_read_32(CTL_BASE);

	return value;
}

static void __write_cpuxgpt(uint32_t reg_index, uint32_t value)
{
	mmio_write_32(INDEX_BASE, reg_index);
	mmio_write_32(CTL_BASE, value);
}

static void __cpuxgpt_set_init_cnt(uint32_t countH, uint32_t countL)
{
#if 0
	__write_cpuxgpt(INDEX_CNT_H_INIT, countH);
	/* update count when countL programmed */
	__write_cpuxgpt(INDEX_CNT_L_INIT, countL);
#endif
}

static void set_cpuxgpt_clk(uint32_t div)
{
	uint32_t tmp = 0;

	tmp = __read_cpuxgpt(INDEX_CTL_REG);
	tmp &= CLK_DIV_MASK;
	tmp |= div;
	__write_cpuxgpt(INDEX_CTL_REG, tmp);
}

static void enable_cpuxgpt(void)
{
	uint32_t tmp = 0;

	tmp = __read_cpuxgpt(INDEX_CTL_REG);
	tmp |= EN_CPUXGPT;
	__write_cpuxgpt(INDEX_CTL_REG, tmp);

	NOTICE("CPUxGPT reg(%x)\n", __read_cpuxgpt(INDEX_CTL_REG));
}

/*
unsigned long long read_cntpct_cpuxgpt(void)
{
	uint32_t cnt[2] = {0, 0};
	uint64_t counter = 0;

	cnt[0] = __read_cpuxgpt(0x74);
	cnt[1] = __read_cpuxgpt(0x78);
	counter = (GPT_BIT_MASK_H &(((uint64_t) (cnt[1])) << 32)) | (GPT_BIT_MASK_L&((uint64_t) (cnt[0])));

	return counter;
}
*/

void __delay(uint64_t cycles)
{
	/* volatile unsigned long long i = 0; */
	/* unsigned long long start = read_cntpct_cpuxgpt(); */
	uint64_t start = mt_read_cntpct_el0();
	/*
	do {
		i = read_cntpct_cpuxgpt();
		NOTICE("CPUxGPT pct(%llx)\n", i);
	} while ((i - start) < cycles);
	*/
	#if 1
	while ((mt_read_cntpct_el0() - start) < cycles) {}
		/* NOTICE("CPUxGPT pct(%llx)\n", mt_read_cntpct_el0()); */
	#endif
}

void mt_udelay(uint32_t us)
{
	uint64_t loops;

	if (us < 2000) {
		loops = us * 13; /*arch timer's freq is 13MHz*/
		__delay(loops);
	}
}

void mt_mdelay(uint32_t ms)
{
	uint32_t loops;

	loops = ms;
	while (loops != 0) {
		mt_udelay(1000);
		loops--;
	}
}

void setup_syscnt(void)
{
   /* set cpuxgpt free run,cpuxgpt always free run & oneshot no need to set */
   /* set cpuxgpt 13Mhz clock */
	set_cpuxgpt_clk(CLK_DIV2);
   /* enable cpuxgpt */
	enable_cpuxgpt();
}

#else

static void __cpuxgpt_set_init_cnt(uint32_t countH, uint32_t countL) {}

void setup_syscnt(void) {}

void udelay(uint32_t us) {}

void mdelay(uint32_t ms) {}

#endif

void generic_timer_backup(void)
{
	uint64_t cval;

	cval = read_cntpct_el0();
	__cpuxgpt_set_init_cnt((uint32_t)(cval >> 32), (uint32_t)(cval & 0xffffffff));
}

void atf_sched_clock_init(uint64_t normal_base, uint64_t atf_base)
{
	normal_time_base += normal_base;
	atf_time_base = atf_base;
}

uint64_t atf_sched_clock(void)
{
	uint64_t cval;

	cval = (((read_cntpct_el0() - atf_time_base)*1000)/13) - normal_time_base;

	return cval;
}

/*
  Return: 0 - Trying to disable the CPUXGPT control bit, and not allowed to disable it.
  Return: 1 - reg_addr is not realted to disable the control bit.
*/
uint32_t check_cpuxgpt_write_permission(uint32_t reg_addr, uint32_t reg_value)
{
	uint32_t idx;
	uint32_t ctl_val;

	if (reg_addr == CTL_BASE) {
		idx = mmio_read_32(INDEX_BASE);

		/* idx 0: CPUXGPT system control */
		if (idx == 0) {
			ctl_val = mmio_read_32(CTL_BASE);
			if (ctl_val & 1) {
				/* if enable bit already set, then bit 0 is not allow to set as 0 */
				if (!(reg_value & 1))
					return 0;
			}
		}
	}
	return 1;
}
