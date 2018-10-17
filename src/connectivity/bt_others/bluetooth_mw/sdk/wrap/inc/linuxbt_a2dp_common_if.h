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
 * MediaTek Inc. (C) 2016-2017. All rights reserved.
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

/* FILE NAME:  linuxbt_a2dp_common_if.h
 * PURPOSE:
 *  {1. What is covered in this file - function and scope.}
 *  {2. Related documents or hardware information}
 * NOTES:
 *  {Something must be known or noticed}
 *  {1. How to use these functions - Give an example.}
 *  {2. Sequence of messages if applicable.}
 *  {3. Any design limitation}
 *  {4. Any performance limitation}
 *  {5. Is it a reusable component}
 *
 *
 *
 */
#ifndef LINUXBT_A2DP_COMMON_IF_H
#define LINUXBT_A2DP_COMMON_IF_H
/* INCLUDE FILE DECLARATIONS
 */
/* NAMING CONSTANT DECLARATIONS
 */

/* data type for the SBC Codec Information Element*/

/* for Codec Specific Information Element */
//SBC
#define A2D_SBC_IE_SAMP_FREQ_MSK    0xF0    /* b7-b4 sampling frequency */
#define A2D_SBC_IE_SAMP_FREQ_16     0x80    /* b7:16  kHz */
#define A2D_SBC_IE_SAMP_FREQ_32     0x40    /* b6:32  kHz */
#define A2D_SBC_IE_SAMP_FREQ_44     0x20    /* b5:44.1kHz */
#define A2D_SBC_IE_SAMP_FREQ_48     0x10    /* b4:48  kHz */

#define A2D_SBC_IE_CH_MD_MSK        0x0F    /* b3-b0 channel mode */
#define A2D_SBC_IE_CH_MD_MONO       0x08    /* b3: mono */
#define A2D_SBC_IE_CH_MD_DUAL       0x04    /* b2: dual */
#define A2D_SBC_IE_CH_MD_STEREO     0x02    /* b1: stereo */
#define A2D_SBC_IE_CH_MD_JOINT      0x01    /* b0: joint stereo */

#define A2D_SBC_IE_BLOCKS_MSK       0xF0    /* b7-b4 number of blocks */
#define A2D_SBC_IE_BLOCKS_4         0x80    /* 4 blocks */
#define A2D_SBC_IE_BLOCKS_8         0x40    /* 8 blocks */
#define A2D_SBC_IE_BLOCKS_12        0x20    /* 12blocks */
#define A2D_SBC_IE_BLOCKS_16        0x10    /* 16blocks */

#define A2D_SBC_IE_SUBBAND_MSK      0x0C    /* b3-b2 number of subbands */
#define A2D_SBC_IE_SUBBAND_4        0x08    /* b3: 4 */
#define A2D_SBC_IE_SUBBAND_8        0x04    /* b2: 8 */

#define A2D_SBC_IE_ALLOC_MD_MSK     0x03    /* b1-b0 allocation mode */
#define A2D_SBC_IE_ALLOC_MD_S       0x02    /* b1: SNR */
#define A2D_SBC_IE_ALLOC_MD_L       0x01    /* b0: loundess */

// AAC
#define AAC_MPEG2_LC (0x80)
#define AAC_MPEG4_LC (0x40)
#define AAC_MPEG4_LTP (0x20)
#define AAC_MPEG4_SCA (0x10)

#define AAC_SAMPLE_FREQ_MSK (0x0FFF)  /* sampling frequency */
#define AAC_SAMPLE_FREQ_96k (0x0001)
#define AAC_SAMPLE_FREQ_88k (0x0002)
#define AAC_SAMPLE_FREQ_64k (0x0004)
#define AAC_SAMPLE_FREQ_48k (0x0008)
#define AAC_SAMPLE_FREQ_44k (0x0010)
#define AAC_SAMPLE_FREQ_32k (0x0020)
#define AAC_SAMPLE_FREQ_24k (0x0040)
#define AAC_SAMPLE_FREQ_22k (0x0080)
#define AAC_SAMPLE_FREQ_16k (0x0100)
#define AAC_SAMPLE_FREQ_12k (0x0200)
#define AAC_SAMPLE_FREQ_11k (0x0400)
#define AAC_SAMPLE_FREQ_8k  (0x0800)

#define AAC_CHANNEL_MSK (0x0C)    /* channels */
#define AAC_CHANNEL_1 (0x08)
#define AAC_CHANNEL_2 (0x04)

#define A2D_AAC_VBR_MSK          (0x80)    /* VBR mask */
#define AAC_DEFAULT_BITRATE_HIGH (0x86) // VBR and bitrate
#define AAC_DEFAULT_BITRATE_MID (0)  // bitrate
#define AAC_DEFAULT_BITRATE_LOW (0) // bitrate

/* MACRO FUNCTION DECLARATIONS
 */
/* DATA TYPE DECLARATIONS
 */

typedef struct
{
    UINT8   samp_freq;      /* Sampling frequency */
    UINT8   ch_mode;        /* Channel mode */
    UINT8   block_len;      /* Block length */
    UINT8   num_subbands;   /* Number of subbands */
    UINT8   alloc_mthd;     /* Allocation method */
    UINT8   max_bitpool;    /* Maximum bitpool */
    UINT8   min_bitpool;    /* Minimum bitpool */
} tA2D_SBC_CIE;

typedef struct
{
    UINT8 object_type;
    /*
                UINT16 samp_freq;
                b15~b12,     b11 ~   b4,     b3  ~  b0
                                  8K   ~  44K,   48K ~ 96K
    */
    UINT16 samp_freq;            /* Sampling Frequency Octet1 & Octet2*/
    UINT8 channels;
    UINT32 bit_rate_high;
    UINT32 bit_rate_mid;
    UINT32 bit_rate_low;
} tA2D_AAC_CIE;

/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */




extern INT32 linuxbt_a2dp_ext_init(VOID);
extern INT32 linuxbt_a2dp_change_thread_priority_handler(INT32 priority);
extern INT32 linuxbt_a2dp_codec_enable_handler(BT_A2DP_CODEC_TYPE codec_type, BOOL enable);

#endif /* End of LINUXBT_A2DP_COMMON_IF_H */
