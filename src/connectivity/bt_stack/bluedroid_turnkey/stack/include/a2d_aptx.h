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
 *  This is the interface to utility functions for dealing with APTX data
 *  frames and codec capabilities.
 *
 ******************************************************************************/

#ifndef A2D_APTX_H
#define A2D_APTX_H

#if defined(MTK_LINUX)
#include "mdroid_buildcfg.h"
#endif

/*****************************************************************************
**  Type Definitions
*****************************************************************************/
/* data type for the aptX Codec Information Element*/
#define NON_A2DP_MEDIA_CT 0xff
#define APTX_VENDOR_ID    0
#define APTX_CODEC_ID     1
#define APTX_SAMPLE_RATE  2
#define APTX_CHANNEL      3
#define APTX_FUTURE1      4
#define APTX_FUTURE2      5

typedef unsigned char UINT8;
typedef unsigned short UINT16;
//typedef unsigned long  UINT32;
typedef short SINT16;
#if defined(MTK_COMMON) && (MTK_COMMON == TRUE) && defined(MTK_LINUX)
typedef int SINT32;
#else
typedef long SINT32;
#endif
typedef UINT16 CsrCodecType;

/* aptX codec specific settings*/
#define BTA_AV_CO_APTX_CODEC_LEN 9

#define CSR_APTX_VENDOR_ID (0x0000004F)
#define CSR_APTX_CODEC_ID_BLUETOOTH ((CsrCodecType) 0x0001)
#define CSR_APTX_SAMPLERATE_44100 (0x20)
#define CSR_APTX_SAMPLERATE_48000 (0x10)
#define CSR_APTX_CHANNELS_STEREO (0x02)
#define CSR_APTX_CHANNELS_MONO (0x01)
#define CSR_APTX_FUTURE_1 (0x00)
#define CSR_APTX_FUTURE_2 (0x00)
#define CSR_APTX_OTHER_FEATURES_NONE (0x00000000)
#define CSR_AV_APTX_AUDIO (0x00)
#define CSR_APTX_CHANNEL (0x0001)
#define CSR_APTX_SAMPLERATE (0x22)

typedef struct
{
    UINT32 vendorId;
    UINT16 codecId;            /* Codec ID for aptX */
    UINT8     sampleRate;   /* Sampling Frequency */
    UINT8   channelMode;    /* STEREO/DUAL/MONO */
    UINT8   future1;
    UINT8   future2;
} tA2D_APTX_CIE;


typedef struct
{
    SINT16 s16SamplingFreq;        /* 16k, 32k, 44.1k or 48k*/
    SINT16 s16ChannelMode;        /* mono, dual, streo or joint streo*/
    UINT16 u16BitRate;
    UINT16 *ps16NextPcmBuffer;
    UINT16 as16PcmBuffer[256];
    UINT8  *pu8Packet;
    UINT8  *pu8NextPacket;
    UINT16 u16PacketLength;
    void* aptxEncoder;
} APTX_ENC_PARAMS;

UINT8 A2D_BldAptxInfo(UINT8 media_type, tA2D_APTX_CIE *p_ie, UINT8 *p_result);
UINT8 A2D_ParsAptxInfo(tA2D_APTX_CIE *p_ie, UINT8 *p_info, BOOLEAN for_caps);
void APTX_Encoder_Init(APTX_ENC_PARAMS);
UINT8 bta_av_aptx_cfg_in_cap(UINT8 *p_cfg, tA2D_APTX_CIE *p_cap);
UINT8 bta_av_get_current_codec();
UINT8* bta_av_get_current_codecInfo();
BOOLEAN bta_av_co_audio_get_aac_aptx_config(UINT8 *p_config, UINT16 *p_minmtu, UINT8 type);

#endif  // A2D_APTX_H

