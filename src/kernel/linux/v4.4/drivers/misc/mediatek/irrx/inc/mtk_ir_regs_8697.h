/**
 * Copyright (C) 2016 MediaTek Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See http://www.gnu.org/licenses/gpl-2.0.html for more details.
 */
#ifndef __MTK_IR_REGS_H__
#define __MTK_IR_REGS_H__
#include <linux/types.h>
#include <asm/memory.h>

#define IRRX_CLK_FREQUENCE    (3*1000*1000)	/* 3MHz */

/**************************************************
****************IRRX register define********************
**************************************************/
#define IRRX_COUNT_HIGH_REG			0x200
#define IRRX_CH_BITCNT_MASK         0x0000003f
#define IRRX_CH_BITCNT_BITSFT       0
#define IRRX_CH_1ST_PULSE_MASK      0x0000ff00
#define IRRX_CH_1ST_PULSE_BITSFT    8
#define IRRX_CH_2ND_PULSE_MASK      0x00ff0000
#define IRRX_CH_2ND_PULSE_BITSFT    16
#define IRRX_CH_3RD_PULSE_MASK      0xff000000
#define IRRX_CH_3RD_PULSE_BITSFT    24

#define IRRX_COUNT_MID_REG          0x204
#define IRRX_COUNT_LOW_REG          0x208

#define IRRX_CONFIG_HIGH_REG        0x20c
#define IRRX_AUTO_CLREN             ((u32)(1 << 31))
#define IRRX_CH_DISPD               ((u32)(1 << 15))
#define IRRX_CH_IGB0                ((u32)(1 << 14))
#define IRRX_CH_CHKEN               ((u32)(1 << 13))	/* enable puse width */
#define IRRX_CH_DISCH               ((u32)(1 << 7))
#define IRRX_CH_DISCL               ((u32)(1 << 6))
#define IRRX_CH_IGSYN               ((u32)(1 << 5))
#define IRRX_CH_ORDINV              ((u32)(1 << 4))
#define IRRX_CH_RC5_1ST             ((u32)(1 << 3))
#define IRRX_CH_RC5                 ((u32)(1 << 2))
#define IRRX_CH_IRI                 ((u32)(1 << 1))
#define IRRX_CH_HWIR                ((u32)(1 << 0))

#define IRRX_CH_END_7               ((u32)(0x07 << 16))
#define IRRX_CH_END_15              ((u32)(0x0f << 16))	/* [22:16] */
#define IRRX_CH_END_23              ((u32)(0x17 << 16))
#define IRRX_CH_END_31              ((u32)(0x1f << 16))
#define IRRX_CH_END_39              ((u32)(0x27 << 16))
#define IRRX_CH_END_47              ((u32)(0x2f << 16))
#define IRRX_CH_END_55              ((u32)(0x07 << 16))
#define IRRX_CH_END_63              ((u32)(0x0f << 16))

#define IRRX_CONFIG_LOW_REG         0x210	/* IRCFGL */
#define IRRX_SAPERIOD_MASK          ((u32)0xff<<0)	/* [7:0]   sampling period */
#define IRRX_SAPERIOD_OFFSET        ((u32)0)

#define IRRX_CHK_PERIOD_MASK        ((u32)0x1fff<<8)	/* [20:8]   ir pulse width sample period */
#define IRRX_CHK_PERIOD_OFFSET      ((u32)8)

#define IRRX_THRESHOLD_REG          0x214
#define IRRX_PRDY_SEL               ((u32)0x1<<16)	/*MT8167 */
#define IRRX_THRESHOLD_MASK         ((u32)0x7f<<0)
#define IRRX_THRESHOLD_OFFSET       ((u32)0)

#define IRRX_ICLR_MASK              ((u32)1<<7)	/* interrupt clear reset ir */
#define IRRX_ICLR_OFFSET            ((u32)7)

#define IRRX_DGDEL_MASK             ((u32)3<<8)	/* de-glitch select */
#define IRRX_DGDEL_MASK_MT8167      ((u32)0x1f<<8) /* MT8167 hj 8697 same as 8167 */
#define IRRX_DGDEL_OFFSET           ((u32)8)

#define IRRX_IRCLR                  0x218
#define IRRX_IRCLR_MASK             ((u32)0x1<<0)
#define IRRX_IRCLR_OFFSET           ((u32)0)

#define IRRX_RCMM_THD_REG           0x220
#define IRRX_RCMM_ENABLE_MASK       ((u32)0x1<<31)
#define IRRX_RCMM_ENABLE_OFFSET     ((u32)31)	/* 1 enable rcmm , 0 disable rcmm */
#define IRRX_RCMM_THD_00_MASK       ((u32)0x7f<<0)
#define IRRX_RCMM_THD_00_OFFSET     ((u32)0)
#define IRRX_RCMM_THD_01_MASK       ((u32)0x7f<<7)
#define IRRX_RCMM_THD_01_OFFSET     ((u32)7)
#define IRRX_RCMM_THD_10_MASK       ((u32)0x7f<<14)
#define IRRX_RCMM_THD_10_OFFSET     ((u32)14)
#define IRRX_RCMM_THD_11_MASK       ((u32)0x7f<<21)
#define IRRX_RCMM_THD_11_OFFSET     ((u32)21)

#define IRRX_RCMM_THD_REG0          0x224
#define IRRX_RCMM_THD_20_MASK       ((u32)0x7f<<0)
#define IRRX_RCMM_THD_20_OFFSET     ((u32)0)
#define IRRX_RCMM_THD_21_MASK       ((u32)0x7f<<7)
#define IRRX_RCMM_THD_21_OFFSET     ((u32)7)

#define IRRX_IREXPEN                0x240
#define IRRX_PD_IREXPEN_MASK        ((u32)0x3<<10)
#define IRRX_PD_IREXPEN_OFFSET      ((u32)10)
#define IRRX_IRPDWN_EN              ((u32)(1 << 9))
#define IRRX_BCEPEN                 ((u32)(1 << 8))
#define IRRX_IREXPEN_MASK           ((u32)(0xff << 0))
#define IRRX_IREXPEN_OFFSET         ((u32)0)

#define IRRX_EXPBCNT                0x24c
#define IRRX_IRCHK_CNT              ((u32)0x7f)
#define IRRX_IRCHK_CNT_OFFSET       ((u32)6)
#define IRRX_EXP_BITCNT             ((u32)0x3f)
#define IRRX_EXP_BITCNT_OFFSET      ((u32)0)

#define IRRX_ENEXP_IRM              0x244
#define IRRX_ENEXP_IRL              0x248

#define IRRX_PDWNCNT                0x250
#define IRRX_PDWNCNT_MASK           ((u32)0xffffffff<<0)
#define IRRX_PDWNCNT_MASK_MT8167    ((u32)0xff<<0)/* MT8167 */
#define IRRX_PDWNCNT_OFFSET         ((u32)0)

#define IRRX_EXP_IRL0               0x284
#define IRRX_EXP_IRL1               0x28c
#define IRRX_EXP_IRL2               0x294
#define IRRX_EXP_IRL3               0x29C
#define IRRX_EXP_IRL4               0x2A4
#define IRRX_EXP_IRL5               0x2AC
#define IRRX_EXP_IRL6               0x2B4
#define IRRX_EXP_IRL7               0x2BC
#define IRRX_EXP_IRL8               0x2C4
#define IRRX_EXP_IRL9               0x2CC

#define IRRX_EXP_IRM0               0x280
#define IRRX_EXP_IRM1               0x288
#define IRRX_EXP_IRM2               0x290
#define IRRX_EXP_IRM3               0x298
#define IRRX_EXP_IRM4               0x2A0
#define IRRX_EXP_IRM5               0x2A8
#define IRRX_EXP_IRM6               0x2B0
#define IRRX_EXP_IRM7               0x2B8
#define IRRX_EXP_IRM8               0x2C0
#define IRRX_EXP_IRM9               0x2C8

#define IRRX_CHKDATA0               0x3B0
#define IRRX_CHKDATA1               0x3B4
#define IRRX_CHKDATA2               0x3B8
#define IRRX_CHKDATA3               0x3BC
#define IRRX_CHKDATA4               0x3C0
#define IRRX_CHKDATA5               0x3C4
#define IRRX_CHKDATA6               0x3C8
#define IRRX_CHKDATA7               0x3CC
#define IRRX_CHKDATA8               0x3D0
#define IRRX_CHKDATA9               0x3D4
#define IRRX_CHKDATA10              0x3D8
#define IRRX_CHKDATA11              0x3DC
#define IRRX_CHKDATA12              0x3E0
#define IRRX_CHKDATA13              0x3E4
#define IRRX_CHKDATA14              0x3E8
#define IRRX_CHKDATA15              0x3EC
#define IRRX_CHKDATA16              0x3F0

#define IRRX_IRINT_STA              0x140
#define IRRX_INTSTA_MASK            ((u32)0x1<<13)
#define IRRX_INTSTA_OFFSET          ((u32)13)

#define IRRX_IRINT_EN               0x144
#define IRRX_INTEN_MASK             ((u32)0x1<<13)
#define IRRX_INTEN_OFFSET           ((u32)13)

#define IRRX_IRINT_CLR              0x148
#define IRRX_INTCLR_MASK            ((u32)0x1<<13)
#define IRRX_INTCLR_OFFSET          ((u32)13)

#define IRRX_WDTSET                 0x0004
#define IRRX_WDT                    0x0008

#define IRRX_WDTLMT                 0x000c
#define IRRX_WDTSTA                 0x0010
#define IRRX_POWKEY1                0x0014
#define IRRX_POWKEY2                0x0018
#define IRRX_KEYMASK1               0x001c
#define IRRX_KEYMASK2               0x0020

#define IRRX_IRCKSEL                0x0044 /*IRRX clock divider selection */
#define IRRX_IRCK_DIV1              0x0000
#define IRRX_IRCK_DIV2              0x0001
#define IRRX_IRCK_DIV4              0x0002
#define IRRX_IRCK_DIV8              0x0003
#define IRRX_IRCK_DIV16             0x0004
#define IRRX_IRCK_DIV32             0x0005
#define IRRX_IRCK_DIV64             0x0006
#define IRRX_IRCK_DIV128            0x0007
#define IRRX_IRCK_DIV256            0x0008

#define IRRX_WAKEEN                 0x00f4
#define IRRX_WAKEEN_MASK            ((u32)0x1<<0)
#define IRRX_WAKEEN_OFFSET          ((u32)0)

#define IRRX_WAKECLR                0x00f8
#define IRRX_WAKECLR_MASK           ((u32)0x1<<0)
#define IRRX_WAKECLR_OFFSET         ((u32)0)

#define IRRX_SMCLR                  0x00fc
#define IRRX_SMCLR_MASK             ((u32)0x1<<0)
#define IRRX_SMCLR_OFFSET           ((u32)0)

#define IRRX_SOFTEN                 0x00100
#define IRRX_SOFTEN_MASK            ((u32)0x1<<0)
#define IRRX_SOFTEN_OFFSET          ((u32)0)

#define IRRX_SELECT                 0x0104
#define IRRX_SELECT_MASK            ((u32)0xf<<0)
#define IRRX_SELECT_OFFSET          ((u32)0)

#define IRRX_PDWNOUT                0x0108
#define IRRX_PDWNOUT_MASK           ((u32)0x1<<0)
#define IRRX_PDWNOUT_OFFSET         ((u32)0)

#endif				/* __IRRX_VRF_HW_H__ */
