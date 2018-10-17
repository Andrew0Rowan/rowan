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

#include <platform/mt8516.h>
#include <platform/mtk_timer.h>
#include <platform/mt_reg_base.h>

#define CPUXGPT_BASE    IO_PHYS + 0x200000
#define INDEX_BASE      ((volatile unsigned int *)(CPUXGPT_BASE+0x0674))
#define CTL_BASE        ((volatile unsigned int *)(CPUXGPT_BASE+0x0670))

void set_cntfrq(unsigned long freq)
{
#if ARCH_ARM64

#else
    __asm__ volatile("mcr p15, 0, %0, c14, c0, 0\n" :: "r"(freq));
#endif
}


void cpuxgpt_enable(void)
{
    *CTL_BASE   = (0x201);
    *INDEX_BASE = (0);
}


/* delay msec mseconds */
extern lk_time_t current_time(void);
void mdelay(unsigned long msec)
{
    lk_time_t start = current_time();

    while (start + msec > current_time());
}

/* delay usec useconds */
extern lk_bigtime_t current_time_hires(void);
void udelay(unsigned long usec)
{
    lk_bigtime_t start = current_time_hires();

    while (start + usec > current_time_hires());
}

/*
 * busy wait
 */
void gpt_busy_wait_us(u32 timeout_us)
{
    udelay(timeout_us);
}

void gpt_busy_wait_ms(u32 timeout_ms)
{
    mdelay(timeout_ms);
}

void mtk_timer_init (void)
{
    cpuxgpt_enable();
}
