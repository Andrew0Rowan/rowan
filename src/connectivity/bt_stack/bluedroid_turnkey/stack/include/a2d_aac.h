/******************************************************************************
 *
 *  Copyright (C) 2010 MediaTek Inc.
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at:
 *
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 ******************************************************************************/

/******************************************************************************
 *
 *  This is the interface to utility functions for dealing with AAC data
 *  frames and codec capabilities.
 *
 ******************************************************************************/
#ifndef A2D_AAC_H
#define A2D_AAC_H
#include "utl.h"
#if defined(MTK_A2DP_SRC_AAC_CODEC) && (MTK_A2DP_SRC_AAC_CODEC == TRUE)
#include "aacenc_lib.h"
#endif
#include "mdroid_buildcfg.h"

/*****************************************************************************
**  macros
*****************************************************************************/
typedef short SINT16;
#if defined(MTK_COMMON) && (MTK_COMMON == TRUE) && defined(MTK_LINUX)
typedef int SINT32;
#else
typedef long SINT32;
#endif


/*****************************************************************************
**  constants
*****************************************************************************/

/* the LOSC of AAC media codec capabilitiy */
#define BTA_AV_CO_AAC_CODEC_LEN (8) // media_type (1 byte) + codec_type (1 byte)  + element (6 byte)

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

#if defined(MTK_A2DP_SRC_AAC_CODEC) && (MTK_A2DP_SRC_AAC_CODEC == TRUE)
typedef struct
{
    UINT16 u16SamplingFreq;        /* 16k, 32k, 44.1k or 48k*/
    UINT16 u16ChannelMode;        /* mono, dual, streo or joint streo*/
    UINT32 u32BitRate;
    UINT16 *ps16NextPcmBuffer;
    UINT16 as16PcmBuffer[MTK_A2DP_AAC_ENC_INPUT_BUF_SIZE];

    UINT8  *pu8Packet;
    UINT8  *pu8NextPacket;
    UINT16 u16PacketLength;
    HANDLE_AACENCODER aacEncoder;
} AAC_ENC_PARAMS;
#endif

UINT8 A2D_BldAACInfo(UINT8 media_type, tA2D_AAC_CIE *p_ie, UINT8 *p_result);
UINT8 A2D_ParsAACInfo(tA2D_AAC_CIE *p_ie, UINT8 *p_info, BOOLEAN for_caps);
UINT8 bta_av_AAC_cfg_in_cap(UINT8 *p_cfg, tA2D_AAC_CIE *p_cap);
#if defined(MTK_A2DP_SRC_AAC_CODEC) && (MTK_A2DP_SRC_AAC_CODEC == TRUE)
HANDLE_AACENCODER AAC_Encoder_Init(AAC_ENC_PARAMS params);
void AAC_Encoder_Deinit(AAC_ENC_PARAMS params);
#endif
// UINT8 bta_av_get_current_codec();
// UINT8* bta_av_get_current_codecInfo();
// BOOLEAN bta_av_co_audio_get_aac_aptx_config(UINT8 *p_config, UINT16 *p_minmtu, UINT8 type);

#endif /* A2D_AAC_H */

