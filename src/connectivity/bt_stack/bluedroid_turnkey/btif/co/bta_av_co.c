/******************************************************************************
 *
 *  Copyright (C) 2004-2012 Broadcom Corporation
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
 *  This is the advanced audio/video call-out function implementation for
 *  BTIF.
 *
 ******************************************************************************/

#include "string.h"
#include "a2d_api.h"
#include "a2d_sbc.h"
#include "bta_sys.h"
#include "bta_av_api.h"
#include "bta_av_co.h"
#include "bta_av_ci.h"
#include "bta_av_sbc.h"

#include "btif_media.h"
#include "sbc_encoder.h"
#include "btif_av_co.h"
#if defined(MTK_A2DP_SRC_APTX_CODEC) && (MTK_A2DP_SRC_APTX_CODEC == TRUE)
#include <cutils/properties.h>
#include "a2d_aptx.h"
#endif
#if defined(MTK_A2DP_SRC_AAC_CODEC) || defined(MTK_A2DP_SNK_AAC_CODEC)
#include "a2d_aac.h"
#endif
#include "btif_util.h"
#include "osi/include/mutex.h"
#if defined(MTK_STACK_CONFIG_BL) && (MTK_STACK_CONFIG_BL == TRUE)
#include "interop_mtk.h"
#endif

#if (defined(MTK_A2DP_SRC_AAC_CODEC) && (MTK_A2DP_SRC_AAC_CODEC == TRUE)) || (defined(MTK_A2DP_SRC_AAC_CODEC) && (MTK_A2DP_SRC_AAC_CODEC == TRUE))
UINT8 aac_enabled = 1;
#endif

/*****************************************************************************
 **  Constants
 *****************************************************************************/

#define FUNC_TRACE()     APPL_TRACE_DEBUG("%s", __FUNCTION__);

/* Macro to retrieve the number of elements in a statically allocated array */
#define BTA_AV_CO_NUM_ELEMENTS(__a) (sizeof(__a)/sizeof((__a)[0]))

/* MIN and MAX macros */
#define BTA_AV_CO_MIN(X,Y) ((X) < (Y) ? (X) : (Y))
#define BTA_AV_CO_MAX(X,Y) ((X) > (Y) ? (X) : (Y))

/* Macro to convert audio handle to index and vice versa */
#define BTA_AV_CO_AUDIO_HNDL_TO_INDX(hndl) (((hndl) & (~BTA_AV_CHNL_MSK)) - 1)
#define BTA_AV_CO_AUDIO_INDX_TO_HNDL(indx) (((indx) + 1) | BTA_AV_CHNL_AUDIO)


/* Offsets to access codec information in SBC codec */
#define BTA_AV_CO_SBC_FREQ_CHAN_OFF    3
#define BTA_AV_CO_SBC_BLOCK_BAND_OFF   4
#define BTA_AV_CO_SBC_MIN_BITPOOL_OFF  5
#define BTA_AV_CO_SBC_MAX_BITPOOL_OFF  6

#define BTA_AV_CO_SBC_MAX_BITPOOL  53
#if defined(MTK_LINUX_A2DP_DEFAULT_SAMPLING_RATE) && (MTK_LINUX_A2DP_DEFAULT_SAMPLING_RATE == TRUE)
#define BTIF_AV_SBC_DEFAULT_SAMP_FREQ A2D_SBC_IE_SAMP_FREQ_48
#define BTA_AV_CO_SRC_SBC_MAX_BITPOOL  51
#endif
/* SCMS-T protect info */
const UINT8 bta_av_co_cp_scmst[BTA_AV_CP_INFO_LEN] = "\x02\x02\x00";

/* SBC SRC codec capabilities */
const tA2D_SBC_CIE bta_av_co_sbc_caps =
{
#if defined(MTK_LINUX_A2DP_DEFAULT_SAMPLING_RATE) && (MTK_LINUX_A2DP_DEFAULT_SAMPLING_RATE == TRUE)
    (A2D_SBC_IE_SAMP_FREQ_48), /* samp_freq */
#else
    (A2D_SBC_IE_SAMP_FREQ_44), /* samp_freq */
#endif
    (A2D_SBC_IE_CH_MD_JOINT), /* ch_mode */
    (A2D_SBC_IE_BLOCKS_16), /* block_len */
    (A2D_SBC_IE_SUBBAND_8), /* num_subbands */
    (A2D_SBC_IE_ALLOC_MD_L), /* alloc_mthd */
#if defined(MTK_LINUX_A2DP_DEFAULT_SAMPLING_RATE) && (MTK_LINUX_A2DP_DEFAULT_SAMPLING_RATE == TRUE)
    BTA_AV_CO_SRC_SBC_MAX_BITPOOL,
#else
    BTA_AV_CO_SBC_MAX_BITPOOL, /* max_bitpool */
#endif
    A2D_SBC_IE_MIN_BITPOOL /* min_bitpool */
};

/* SBC SINK codec capabilities */
const tA2D_SBC_CIE bta_av_co_sbc_sink_caps =
{
    (A2D_SBC_IE_SAMP_FREQ_48 | A2D_SBC_IE_SAMP_FREQ_44), /* samp_freq */
    (A2D_SBC_IE_CH_MD_MONO | A2D_SBC_IE_CH_MD_STEREO | A2D_SBC_IE_CH_MD_JOINT | A2D_SBC_IE_CH_MD_DUAL), /* ch_mode */
    (A2D_SBC_IE_BLOCKS_16 | A2D_SBC_IE_BLOCKS_12 | A2D_SBC_IE_BLOCKS_8 | A2D_SBC_IE_BLOCKS_4), /* block_len */
    (A2D_SBC_IE_SUBBAND_4 | A2D_SBC_IE_SUBBAND_8), /* num_subbands */
    (A2D_SBC_IE_ALLOC_MD_L | A2D_SBC_IE_ALLOC_MD_S), /* alloc_mthd */
#if defined(MTK_LINUX_A2DP_DEFAULT_BITPOLL) && (MTK_LINUX_A2DP_DEFAULT_BITPOLL == TRUE)
    BTA_AV_CO_SBC_MAX_BITPOOL, /* max_bitpool */
#else
    A2D_SBC_IE_MAX_BITPOOL, /* max_bitpool */
#endif
    A2D_SBC_IE_MIN_BITPOOL /* min_bitpool */
};

#if !defined(BTIF_AV_SBC_DEFAULT_SAMP_FREQ)
#define BTIF_AV_SBC_DEFAULT_SAMP_FREQ A2D_SBC_IE_SAMP_FREQ_44
#endif

#if defined(MTK_A2DP_SRC_APTX_CODEC) && (MTK_A2DP_SRC_APTX_CODEC == TRUE)
#define BLUETOOTH_APTX_PROPERTY "bluetooth.aptx"
#endif

/* Default SBC codec configuration */
const tA2D_SBC_CIE btif_av_sbc_default_config =
{
    BTIF_AV_SBC_DEFAULT_SAMP_FREQ,   /* samp_freq */
    A2D_SBC_IE_CH_MD_JOINT,         /* ch_mode */
    A2D_SBC_IE_BLOCKS_16,           /* block_len */
    A2D_SBC_IE_SUBBAND_8,           /* num_subbands */
    A2D_SBC_IE_ALLOC_MD_L,          /* alloc_mthd */
    BTA_AV_CO_SBC_MAX_BITPOOL,      /* max_bitpool */
    A2D_SBC_IE_MIN_BITPOOL          /* min_bitpool */
};

#if defined(MTK_A2DP_SRC_APTX_CODEC) && (MTK_A2DP_SRC_APTX_CODEC == TRUE)
static BOOLEAN bta_av_co_audio_codec_match(const UINT8 *p_codec_caps);
const tA2D_APTX_CIE bta_av_co_aptx_caps =
{
    CSR_APTX_VENDOR_ID,
    CSR_APTX_CODEC_ID_BLUETOOTH,
    CSR_APTX_SAMPLERATE_44100,
    CSR_APTX_CHANNELS_STEREO|CSR_APTX_CHANNELS_MONO,
    CSR_APTX_FUTURE_1,
    CSR_APTX_FUTURE_2
};
const tA2D_APTX_CIE btif_av_aptx_default_config =
{
    CSR_APTX_VENDOR_ID,
    CSR_APTX_CODEC_ID_BLUETOOTH,
    CSR_APTX_SAMPLERATE_44100,
    CSR_APTX_CHANNELS_STEREO,
    CSR_APTX_FUTURE_1,
    CSR_APTX_FUTURE_2
};
#endif // MTK_A2DP_SRC_APTX_CODEC
#if defined(MTK_A2DP_SRC_AAC_CODEC) && (MTK_A2DP_SRC_AAC_CODEC == TRUE)
const tA2D_AAC_CIE bta_av_co_aac_caps =
{
    AAC_MPEG2_LC,
#if defined(MTK_LINUX) && defined(MTK_COMMON) && (MTK_COMMON == TRUE)
    AAC_SAMPLE_FREQ_48k,
#else
    AAC_SAMPLE_FREQ_44k,
#endif
    AAC_CHANNEL_2,
    AAC_DEFAULT_BITRATE_HIGH,
    AAC_DEFAULT_BITRATE_MID,
    AAC_DEFAULT_BITRATE_LOW
};
const tA2D_AAC_CIE btif_av_aac_default_config =
{
    AAC_MPEG2_LC,
#if defined(MTK_LINUX) && defined(MTK_COMMON) && (MTK_COMMON == TRUE)
    AAC_SAMPLE_FREQ_48k,
#else
    AAC_SAMPLE_FREQ_44k,
#endif
    AAC_CHANNEL_2,
    AAC_DEFAULT_BITRATE_HIGH,
    AAC_DEFAULT_BITRATE_MID,
    AAC_DEFAULT_BITRATE_LOW
};
#endif //MTK_A2DP_SRC_AAC_CODEC

#if defined(MTK_A2DP_SNK_AAC_CODEC) && (MTK_A2DP_SNK_AAC_CODEC == TRUE)
const tA2D_AAC_CIE bta_av_co_aac_sink_caps =
{
    AAC_MPEG2_LC,
    AAC_SAMPLE_FREQ_44k | AAC_SAMPLE_FREQ_48k,
    AAC_CHANNEL_1 | AAC_CHANNEL_2,
    AAC_DEFAULT_BITRATE_HIGH,
    AAC_DEFAULT_BITRATE_MID,
    AAC_DEFAULT_BITRATE_LOW
};
const tA2D_AAC_CIE btif_av_aac_default_sink_config =
{
    AAC_MPEG2_LC,
    AAC_SAMPLE_FREQ_44k | AAC_SAMPLE_FREQ_48k,
    AAC_CHANNEL_1 | AAC_CHANNEL_2,
    AAC_DEFAULT_BITRATE_HIGH,
    AAC_DEFAULT_BITRATE_MID,
    AAC_DEFAULT_BITRATE_LOW
};
#endif //MTK_A2DP_SNK_AAC_CODEC

/*****************************************************************************
**  Local data
*****************************************************************************/
typedef struct
{
    UINT8 sep_info_idx;                 /* local SEP index (in BTA tables) */
    UINT8 seid;                         /* peer SEP index (in peer tables) */
    UINT8 codec_type;                   /* peer SEP codec type */
    UINT8 codec_caps[AVDT_CODEC_SIZE];  /* peer SEP codec capabilities */
    UINT8 num_protect;                  /* peer SEP number of CP elements */
    UINT8 protect_info[BTA_AV_CP_INFO_LEN];  /* peer SEP content protection info */
} tBTA_AV_CO_SINK;

typedef struct
{
    BD_ADDR         addr;               /* address of audio/video peer */
    tBTA_AV_CO_SINK snks[BTIF_SV_AV_AA_SEP_INDEX]; /* array of supported sinks */
    tBTA_AV_CO_SINK srcs[BTIF_SV_AV_AA_SEP_INDEX]; /* array of supported srcs */
    UINT8           num_snks;           /* total number of sinks at peer */
    UINT8           num_srcs;           /* total number of srcs at peer */
    UINT8           num_seps;           /* total number of seids at peer */
    UINT8           num_rx_snks;        /* number of received sinks */
    UINT8           num_rx_srcs;        /* number of received srcs */
    UINT8           num_sup_snks;       /* number of supported sinks in the snks array */
    UINT8           num_sup_srcs;       /* number of supported srcs in the srcs array */
    tBTA_AV_CO_SINK *p_snk;             /* currently selected sink */
    tBTA_AV_CO_SINK *p_src;             /* currently selected src */
    UINT8           codec_cfg[AVDT_CODEC_SIZE]; /* current codec configuration */
    BOOLEAN         cp_active;          /* current CP configuration */
    BOOLEAN         acp;                /* acceptor */
    BOOLEAN         recfg_needed;       /* reconfiguration is needed */
    BOOLEAN         opened;             /* opened */
    UINT16          mtu;                /* maximum transmit unit size */
    UINT16          uuid_to_connect;    /* uuid of peer device */
} tBTA_AV_CO_PEER;

typedef struct
{
    BOOLEAN active;
    UINT8 flag;
} tBTA_AV_CO_CP;

typedef struct
{
    /* Connected peer information */
    tBTA_AV_CO_PEER peers[BTA_AV_NUM_STRS];
    /* Current codec configuration - access to this variable must be protected */
    tBTIF_AV_CODEC_INFO codec_cfg;
    tBTIF_AV_CODEC_INFO codec_cfg_setconfig; /* remote peer setconfig preference */

    tBTA_AV_CO_CP cp;
} tBTA_AV_CO_CB;

/* Control block instance */
static tBTA_AV_CO_CB bta_av_co_cb;

#if defined(MTK_A2DP_SNK_AAC_CODEC) && (MTK_A2DP_SNK_AAC_CODEC == TRUE)
static UINT8 local_supported_SNK_SEPs_num = 0;
#endif

static BOOLEAN bta_av_co_audio_codec_build_config(const UINT8 *p_codec_caps, UINT8 *p_codec_cfg);
static void bta_av_co_audio_peer_reset_config(tBTA_AV_CO_PEER *p_peer);
static BOOLEAN bta_av_co_cp_is_scmst(const UINT8 *p_protectinfo);
static BOOLEAN bta_av_co_audio_sink_has_scmst(const tBTA_AV_CO_SINK *p_sink);
static BOOLEAN bta_av_co_audio_peer_supports_codec(tBTA_AV_CO_PEER *p_peer, UINT8 *p_snk_index);
static BOOLEAN bta_av_co_audio_media_supports_config(UINT8 codec_type, const UINT8 *p_codec_cfg);
static BOOLEAN bta_av_co_audio_sink_supports_config(UINT8 codec_type, const UINT8 *p_codec_cfg);
static BOOLEAN bta_av_co_audio_peer_src_supports_codec(tBTA_AV_CO_PEER *p_peer, UINT8 *p_src_index);



/*******************************************************************************
 **
 ** Function         bta_av_co_cp_is_active
 **
 ** Description      Get the current configuration of content protection
 **
 ** Returns          TRUE if the current streaming has CP, FALSE otherwise
 **
 *******************************************************************************/
BOOLEAN bta_av_co_cp_is_active(void)
{
    FUNC_TRACE();
    return bta_av_co_cb.cp.active;
}

/*******************************************************************************
 **
 ** Function         bta_av_co_cp_get_flag
 **
 ** Description      Get content protection flag
 **                  BTA_AV_CP_SCMS_COPY_NEVER
 **                  BTA_AV_CP_SCMS_COPY_ONCE
 **                  BTA_AV_CP_SCMS_COPY_FREE
 **
 ** Returns          The current flag value
 **
 *******************************************************************************/
UINT8 bta_av_co_cp_get_flag(void)
{
    FUNC_TRACE();
    return bta_av_co_cb.cp.flag;
}

/*******************************************************************************
 **
 ** Function         bta_av_co_cp_set_flag
 **
 ** Description      Set content protection flag
 **                  BTA_AV_CP_SCMS_COPY_NEVER
 **                  BTA_AV_CP_SCMS_COPY_ONCE
 **                  BTA_AV_CP_SCMS_COPY_FREE
 **
 ** Returns          TRUE if setting the SCMS flag is supported else FALSE
 **
 *******************************************************************************/
BOOLEAN bta_av_co_cp_set_flag(UINT8 cp_flag)
{
    FUNC_TRACE();

#if defined(BTA_AV_CO_CP_SCMS_T) && (BTA_AV_CO_CP_SCMS_T == TRUE)
#else
    if (cp_flag != BTA_AV_CP_SCMS_COPY_FREE)
    {
        return FALSE;
    }
#endif
    bta_av_co_cb.cp.flag = cp_flag;
    return TRUE;
}

/*******************************************************************************
 **
 ** Function         bta_av_co_get_peer
 **
 ** Description      find the peer entry for a given handle
 **
 ** Returns          the control block
 **
 *******************************************************************************/
static tBTA_AV_CO_PEER *bta_av_co_get_peer(tBTA_AV_HNDL hndl)
{
    UINT8 index;
    FUNC_TRACE();

    index = BTA_AV_CO_AUDIO_HNDL_TO_INDX(hndl);

    /* Sanity check */
    if (index >= BTA_AV_CO_NUM_ELEMENTS(bta_av_co_cb.peers))
    {
        APPL_TRACE_ERROR("bta_av_co_get_peer peer index out of bounds:%d", index);
        return NULL;
    }

    return &bta_av_co_cb.peers[index];
}

/*******************************************************************************
 **
 ** Function         bta_av_co_audio_init
 **
 ** Description      This callout function is executed by AV when it is
 **                  started by calling BTA_AvRegister().  This function can be
 **                  used by the phone to initialize audio paths or for other
 **                  initialization purposes.
 **
 **
 ** Returns          Stream codec and content protection capabilities info.
 **
 *******************************************************************************/
BOOLEAN bta_av_co_audio_init(UINT8 *p_codec_type, UINT8 *p_codec_info, UINT8 *p_num_protect,
        UINT8 *p_protect_info, UINT8 index)
{
    FUNC_TRACE();

    APPL_TRACE_DEBUG("bta_av_co_audio_init: %d", index);

    /* By default - no content protection info */
    *p_num_protect = 0;
    *p_protect_info = 0;

    /* reset remote preference through setconfig */
    bta_av_co_cb.codec_cfg_setconfig.id = BTIF_AV_CODEC_NONE;

    switch (index)
    {
    case BTIF_SV_AV_AA_SBC_INDEX:
#if defined(BTA_AV_CO_CP_SCMS_T) && (BTA_AV_CO_CP_SCMS_T == TRUE)
    {
        UINT8 *p = p_protect_info;

        /* Content protection info - support SCMS-T */
        *p_num_protect = 1;
        *p++ = BTA_AV_CP_LOSC;
        UINT16_TO_STREAM(p, BTA_AV_CP_SCMS_T_ID);

    }
#endif
        /* Set up for SBC codec  for SRC*/
        *p_codec_type = BTA_AV_CODEC_SBC;

        /* This should not fail because we are using constants for parameters */
        A2D_BldSbcInfo(AVDT_MEDIA_AUDIO, (tA2D_SBC_CIE *) &bta_av_co_sbc_caps, p_codec_info);

        /* Codec is valid */
        return TRUE;
#if defined(MTK_A2DP_SRC_APTX_CODEC) && (MTK_A2DP_SRC_APTX_CODEC == TRUE)
    case BTIF_SV_AV_AA_CSR_APTX_INDEX:
        *p_codec_type = NON_A2DP_MEDIA_CT;
        A2D_BldAptxInfo(AVDT_MEDIA_AUDIO, (tA2D_APTX_CIE *) &bta_av_co_aptx_caps, p_codec_info);
         return TRUE;
#endif    // MTK_A2DP_SRC_APTX_CODEC
#if defined(MTK_A2DP_SRC_AAC_CODEC) && (MTK_A2DP_SRC_AAC_CODEC == TRUE)
    case BTIF_SV_AV_AA_AAC_INDEX:
        *p_codec_type = BTA_AV_CODEC_M24;
        A2D_BldAACInfo(AVDT_MEDIA_AUDIO, (tA2D_AAC_CIE *) &bta_av_co_aac_caps, p_codec_info);
        return TRUE;
#endif    // MTK_A2DP_SRC_AAC_CODEC
#if (BTA_AV_SINK_INCLUDED == TRUE)
    case BTIF_SV_AV_AA_SBC_SINK_INDEX:
        *p_codec_type = BTA_AV_CODEC_SBC;

        /* This should not fail because we are using constants for parameters */
        A2D_BldSbcInfo(AVDT_MEDIA_AUDIO, (tA2D_SBC_CIE *) &bta_av_co_sbc_sink_caps, p_codec_info);
#if defined(MTK_A2DP_SNK_AAC_CODEC) && (MTK_A2DP_SNK_AAC_CODEC == TRUE)
        local_supported_SNK_SEPs_num ++;
#endif
        /* Codec is valid */
        return TRUE;
#if defined(MTK_A2DP_SNK_AAC_CODEC) && (MTK_A2DP_SNK_AAC_CODEC == TRUE)
    case BTIF_SV_AV_AA_AAC_SINK_INDEX:
        *p_codec_type = BTA_AV_CODEC_M24;

        /* This should not fail because we are using constants for parameters */
        A2D_BldAACInfo(AVDT_MEDIA_AUDIO, (tA2D_AAC_CIE *) &bta_av_co_aac_sink_caps, p_codec_info);
        local_supported_SNK_SEPs_num ++;
        /* Codec is valid */
        return TRUE;
#endif // #if defined(MTK_A2DP_SNK_AAC_CODEC) && (MTK_A2DP_SNK_AAC_CODEC == TRUE)

#endif
    default:
        /* Not valid */
        return FALSE;
    }
}

/*******************************************************************************
 **
 ** Function         bta_av_co_audio_disc_res
 **
 ** Description      This callout function is executed by AV to report the
 **                  number of stream end points (SEP) were found during the
 **                  AVDT stream discovery process.
 **
 **
 ** Returns          void.
 **
 *******************************************************************************/
void bta_av_co_audio_disc_res(tBTA_AV_HNDL hndl, UINT8 num_seps, UINT8 num_snk,
        UINT8 num_src, BD_ADDR addr, UINT16 uuid_local)
{
    tBTA_AV_CO_PEER *p_peer;

    FUNC_TRACE();

    APPL_TRACE_DEBUG("bta_av_co_audio_disc_res h:x%x num_seps:%d num_snk:%d num_src:%d",
            hndl, num_seps, num_snk, num_src);

    /* Find the peer info */
    p_peer = bta_av_co_get_peer(hndl);
    if (p_peer == NULL)
    {
        APPL_TRACE_ERROR("bta_av_co_audio_disc_res could not find peer entry");
        return;
    }

    /* Sanity check : this should never happen */
    if (p_peer->opened)
    {
        APPL_TRACE_ERROR("bta_av_co_audio_disc_res peer already opened");
    }

    /* Copy the discovery results */
    bdcpy(p_peer->addr, addr);
    p_peer->num_snks = num_snk;
    p_peer->num_srcs = num_src;
    p_peer->num_seps = num_seps;
    p_peer->num_rx_snks = 0;
    p_peer->num_rx_srcs = 0;
    p_peer->num_sup_snks = 0;
    if (uuid_local == UUID_SERVCLASS_AUDIO_SINK)
        p_peer->uuid_to_connect = UUID_SERVCLASS_AUDIO_SOURCE;
    else if (uuid_local == UUID_SERVCLASS_AUDIO_SOURCE)
        p_peer->uuid_to_connect = UUID_SERVCLASS_AUDIO_SINK;
}

/*******************************************************************************
 **
 ** Function         bta_av_build_src_cfg
 **
 ** Description      This function will build preferred config from src capabilities
 **
 **
 ** Returns          Pass or Fail for current getconfig.
 **
 *******************************************************************************/
void bta_av_build_src_cfg (UINT8 *p_pref_cfg, UINT8 *p_src_cap)
{
    tA2D_SBC_CIE    src_cap;
    tA2D_SBC_CIE    pref_cap;
    UINT8           status = 0;

    /* initialize it to default SBC configuration */
    A2D_BldSbcInfo(AVDT_MEDIA_AUDIO, (tA2D_SBC_CIE *) &btif_av_sbc_default_config, p_pref_cfg);
    /* now try to build a preferred one */
    /* parse configuration */
    if ((status = A2D_ParsSbcInfo(&src_cap, p_src_cap, TRUE)) != 0)
    {
         APPL_TRACE_DEBUG(" Cant parse src cap ret = %d", status);
         return ;
    }

    if (src_cap.samp_freq & A2D_SBC_IE_SAMP_FREQ_48)
        pref_cap.samp_freq = A2D_SBC_IE_SAMP_FREQ_48;
    else if (src_cap.samp_freq & A2D_SBC_IE_SAMP_FREQ_44)
        pref_cap.samp_freq = A2D_SBC_IE_SAMP_FREQ_44;

    if (src_cap.ch_mode & A2D_SBC_IE_CH_MD_JOINT)
        pref_cap.ch_mode = A2D_SBC_IE_CH_MD_JOINT;
    else if (src_cap.ch_mode & A2D_SBC_IE_CH_MD_STEREO)
        pref_cap.ch_mode = A2D_SBC_IE_CH_MD_STEREO;
    else if (src_cap.ch_mode & A2D_SBC_IE_CH_MD_DUAL)
        pref_cap.ch_mode = A2D_SBC_IE_CH_MD_DUAL;
    else if (src_cap.ch_mode & A2D_SBC_IE_CH_MD_MONO)
        pref_cap.ch_mode = A2D_SBC_IE_CH_MD_MONO;

    if (src_cap.block_len & A2D_SBC_IE_BLOCKS_16)
        pref_cap.block_len = A2D_SBC_IE_BLOCKS_16;
    else if (src_cap.block_len & A2D_SBC_IE_BLOCKS_12)
        pref_cap.block_len = A2D_SBC_IE_BLOCKS_12;
    else if (src_cap.block_len & A2D_SBC_IE_BLOCKS_8)
        pref_cap.block_len = A2D_SBC_IE_BLOCKS_8;
    else if (src_cap.block_len & A2D_SBC_IE_BLOCKS_4)
        pref_cap.block_len = A2D_SBC_IE_BLOCKS_4;

    if (src_cap.num_subbands & A2D_SBC_IE_SUBBAND_8)
        pref_cap.num_subbands = A2D_SBC_IE_SUBBAND_8;
    else if(src_cap.num_subbands & A2D_SBC_IE_SUBBAND_4)
        pref_cap.num_subbands = A2D_SBC_IE_SUBBAND_4;

    if (src_cap.alloc_mthd & A2D_SBC_IE_ALLOC_MD_L)
        pref_cap.alloc_mthd = A2D_SBC_IE_ALLOC_MD_L;
    else if(src_cap.alloc_mthd & A2D_SBC_IE_ALLOC_MD_S)
        pref_cap.alloc_mthd = A2D_SBC_IE_ALLOC_MD_S;

    pref_cap.max_bitpool = src_cap.max_bitpool;
    pref_cap.min_bitpool = src_cap.min_bitpool;

    A2D_BldSbcInfo(AVDT_MEDIA_AUDIO, (tA2D_SBC_CIE *) &pref_cap, p_pref_cfg);
}

#if defined(MTK_A2DP_SNK_AAC_CODEC) && (MTK_A2DP_SNK_AAC_CODEC == TRUE)
void bta_av_build_aac_src_cfg (UINT8 *p_pref_cfg, UINT8 *p_src_cap)
{
    tA2D_AAC_CIE    src_cap;
    tA2D_AAC_CIE    pref_cap;
    UINT8           status = 0;
    // UINT32          local_bitrate = 0, peer_bitrate = 0;


    /* initialize it to default SBC configuration */
    A2D_BldAACInfo(AVDT_MEDIA_AUDIO, (tA2D_AAC_CIE *) &btif_av_aac_default_sink_config, p_pref_cfg);
    /* now try to build a preferred one */
    /* parse configuration */
    if ((status = A2D_ParsAACInfo(&src_cap, p_src_cap, TRUE)) != 0)
    {
         APPL_TRACE_DEBUG(" Cant parse src cap ret = %d", status);
         return ;
    }

    if (src_cap.object_type & AAC_MPEG2_LC)
    {
        pref_cap.object_type = AAC_MPEG2_LC;
    }
    else
    {
        APPL_TRACE_ERROR("%s, src_cap.object_type:%d", __FUNCTION__, src_cap.object_type);
    }

    if (src_cap.samp_freq & AAC_SAMPLE_FREQ_48k)
    {
        pref_cap.samp_freq = AAC_SAMPLE_FREQ_48k;
    }
    else if (src_cap.samp_freq & AAC_SAMPLE_FREQ_44k)
    {
        pref_cap.samp_freq = AAC_SAMPLE_FREQ_44k;
    }
    else
    {
        APPL_TRACE_ERROR("%s, src_cap.samp_freq:%d", __FUNCTION__, src_cap.samp_freq);
    }

    if (src_cap.channels & AAC_CHANNEL_2)
    {
        pref_cap.channels = AAC_CHANNEL_2;
    }
    else if (src_cap.channels & AAC_CHANNEL_1)
    {
        pref_cap.channels = AAC_CHANNEL_1;
    }
    else
    {
        APPL_TRACE_ERROR("%s, src_cap.channels:%d", __FUNCTION__, src_cap.channels);
    }

    if (src_cap.bit_rate_high & A2D_AAC_VBR_MSK == 0)
    {
        APPL_TRACE_WARNING("%s, RMT SRC don't support VBR", __FUNCTION__);
    }

    pref_cap.bit_rate_high = src_cap.bit_rate_high;
    pref_cap.bit_rate_mid  = src_cap.bit_rate_mid;
    pref_cap.bit_rate_low  = src_cap.bit_rate_low;

    A2D_BldAACInfo(AVDT_MEDIA_AUDIO, (tA2D_AAC_CIE *) &pref_cap, p_pref_cfg);
}
#endif

/*******************************************************************************
 **
 ** Function         bta_av_audio_sink_getconfig
 **
 ** Description      This callout function is executed by AV to retrieve the
 **                  desired codec and content protection configuration for the
 **                  A2DP Sink audio stream in Initiator.
 **
 **
 ** Returns          Pass or Fail for current getconfig.
 **
 *******************************************************************************/
UINT8 bta_av_audio_sink_getconfig(tBTA_AV_HNDL hndl, tBTA_AV_CODEC codec_type,
        UINT8 *p_codec_info, UINT8 *p_sep_info_idx, UINT8 seid, UINT8 *p_num_protect,
        UINT8 *p_protect_info)
{

    UINT8 result = A2D_FAIL;
    BOOLEAN supported;
    tBTA_AV_CO_PEER *p_peer;
    tBTA_AV_CO_SINK *p_src;
    UINT8 pref_cfg[AVDT_CODEC_SIZE];
    UINT8 index;

    FUNC_TRACE();

    APPL_TRACE_DEBUG("bta_av_audio_sink_getconfig handle:0x%x codec_type:%d seid:%d",
                                                               hndl, codec_type, seid);
    APPL_TRACE_DEBUG("num_protect:0x%02x protect_info:0x%02x%02x%02x",
        *p_num_protect, p_protect_info[0], p_protect_info[1], p_protect_info[2]);

    /* Retrieve the peer info */
    p_peer = bta_av_co_get_peer(hndl);
    if (p_peer == NULL)
    {
        APPL_TRACE_ERROR("bta_av_audio_sink_getconfig could not find peer entry");
        return A2D_FAIL;
    }

    APPL_TRACE_DEBUG("bta_av_audio_sink_getconfig peer(o=%d,n_snks=%d,n_rx_snks=%d,n_sup_snks=%d)",
            p_peer->opened, p_peer->num_srcs, p_peer->num_rx_srcs, p_peer->num_sup_srcs);

    p_peer->num_rx_srcs++;

    /* Check if this is a supported configuration */
    supported = FALSE;
    switch (codec_type)
    {
        case BTA_AV_CODEC_SBC:
            supported = TRUE;
            break;
#if defined(MTK_A2DP_SNK_AAC_CODEC) && (MTK_A2DP_SNK_AAC_CODEC == TRUE)
        case BTA_AV_CODEC_M24:
            supported = TRUE;
            break;
#endif
        default:
            break;
    }

    if (supported)
    {
        /* If there is room for a new one */
        if (p_peer->num_sup_srcs < BTA_AV_CO_NUM_ELEMENTS(p_peer->srcs))
        {
            p_src = &p_peer->srcs[p_peer->num_sup_srcs++];
#if defined(MTK_A2DP_SNK_AAC_CODEC) && (MTK_A2DP_SNK_AAC_CODEC == TRUE)
            if (codec_type == BTA_AV_CODEC_M24)
            {
                APPL_TRACE_DEBUG("%s, saved caps[%x:%x:%02x:%02x:%02x:%02x:%02x:%02x]",
                        __FUNCTION__, p_codec_info[1], p_codec_info[2], p_codec_info[3],
                        p_codec_info[4], p_codec_info[5], p_codec_info[6],p_codec_info[7],p_codec_info[8]);
            }
            else if (codec_type == BTA_AV_CODEC_SBC)
            {
#endif
                APPL_TRACE_DEBUG("%s, saved caps[%x:%x:%x:%x:%x:%x]",
                        __FUNCTION__, p_codec_info[1], p_codec_info[2],
                        p_codec_info[3], p_codec_info[4], p_codec_info[5],
                        p_codec_info[6]);
#if defined(MTK_A2DP_SNK_AAC_CODEC) && (MTK_A2DP_SNK_AAC_CODEC == TRUE)
            }
#endif
            memcpy(p_src->codec_caps, p_codec_info, AVDT_CODEC_SIZE);
            p_src->codec_type = codec_type;
            p_src->sep_info_idx = *p_sep_info_idx;
            p_src->seid = seid;
            p_src->num_protect = *p_num_protect;
            memcpy(p_src->protect_info, p_protect_info, BTA_AV_CP_INFO_LEN);
        }
        else
        {
            APPL_TRACE_ERROR("bta_av_audio_sink_getconfig no more room for SRC info");
        }
    }

    /* If last SNK get capabilities or all supported codec caps retrieved */
    if ((p_peer->num_rx_srcs == p_peer->num_srcs) ||
        (p_peer->num_sup_srcs == BTA_AV_CO_NUM_ELEMENTS(p_peer->srcs)))
    {
        APPL_TRACE_DEBUG("bta_av_audio_sink_getconfig last SRC reached");

        /* Protect access to bta_av_co_cb.codec_cfg */
        mutex_global_lock();

#if defined(MTK_A2DP_SNK_AAC_CODEC) && (MTK_A2DP_SNK_AAC_CODEC == TRUE)
        APPL_TRACE_DEBUG("%s, local_supported_SNK_SEPs_num:%d", __FUNCTION__,
                         local_supported_SNK_SEPs_num);
#endif
        /* Find a src that matches the codec config */
        if (bta_av_co_audio_peer_src_supports_codec(p_peer, &index))
        {
            APPL_TRACE_DEBUG(" Codec Supported ");
            p_src = &p_peer->srcs[index];

            /* Build the codec configuration for this sink */
            {
                /* Save the new configuration */
                p_peer->p_src = p_src;
                /* get preferred config from src_caps */
#if defined(MTK_A2DP_SNK_AAC_CODEC) && (MTK_A2DP_SNK_AAC_CODEC == TRUE)
                if (p_src->codec_type == BTA_AV_CODEC_M24)
                {
                    APPL_TRACE_DEBUG("%s, to call bta_av_build_aac_src_cfg", __FUNCTION__);
                    bta_av_build_aac_src_cfg(pref_cfg, p_src->codec_caps);
                }
                else if (p_src->codec_type == BTA_AV_CODEC_SBC)
                {
#endif
                bta_av_build_src_cfg(pref_cfg, p_src->codec_caps);
#if defined(MTK_A2DP_SNK_AAC_CODEC) && (MTK_A2DP_SNK_AAC_CODEC == TRUE)
                }
#endif
                memcpy(p_peer->codec_cfg, pref_cfg, AVDT_CODEC_SIZE);
#if defined(MTK_A2DP_SNK_AAC_CODEC) && (MTK_A2DP_SNK_AAC_CODEC == TRUE)
                if (codec_type == BTA_AV_CODEC_M24)
                {
                    APPL_TRACE_DEBUG("%s, bta_av_audio_sink_getconfig saved caps[%x:%x:%x:%x:%x:%x:%x:%x]",
                            __FUNCTION__, p_codec_info[1], p_codec_info[2], p_codec_info[3],
                            p_codec_info[4], p_codec_info[5], p_codec_info[6],p_codec_info[7],p_codec_info[8]);
                }
                else if (codec_type == BTA_AV_CODEC_SBC)
                {
#endif
                    APPL_TRACE_DEBUG("bta_av_audio_sink_getconfig  p_codec_info[%x:%x:%x:%x:%x:%x]",
                            p_peer->codec_cfg[1], p_peer->codec_cfg[2], p_peer->codec_cfg[3],
                            p_peer->codec_cfg[4], p_peer->codec_cfg[5], p_peer->codec_cfg[6]);
#if defined(MTK_A2DP_SNK_AAC_CODEC) && (MTK_A2DP_SNK_AAC_CODEC == TRUE)
                }
#endif
                /* By default, no content protection */
                *p_num_protect = 0;

#if defined(BTA_AV_CO_CP_SCMS_T) && (BTA_AV_CO_CP_SCMS_T == TRUE)
                    p_peer->cp_active = FALSE;
                    bta_av_co_cb.cp.active = FALSE;
#endif

                    *p_sep_info_idx = p_src->sep_info_idx;
                    memcpy(p_codec_info, p_peer->codec_cfg, AVDT_CODEC_SIZE);
                result =  A2D_SUCCESS;
            }
        }
        /* Protect access to bta_av_co_cb.codec_cfg */
        mutex_global_unlock();
    }
    return result;
}
/*******************************************************************************
 **
 ** Function         bta_av_co_audio_getconfig
 **
 ** Description      This callout function is executed by AV to retrieve the
 **                  desired codec and content protection configuration for the
 **                  audio stream.
 **
 **
 ** Returns          Stream codec and content protection configuration info.
 **
 *******************************************************************************/
UINT8 bta_av_co_audio_getconfig(tBTA_AV_HNDL hndl, tBTA_AV_CODEC codec_type,
                                UINT8 *p_codec_info, UINT8 *p_sep_info_idx, UINT8 seid, UINT8 *p_num_protect,
                                UINT8 *p_protect_info)

{
    UINT8 result = A2D_FAIL;
    BOOLEAN supported;
    tBTA_AV_CO_PEER *p_peer;
    tBTA_AV_CO_SINK *p_sink;
    UINT8 codec_cfg[AVDT_CODEC_SIZE];
    UINT8 index;
#if defined(MTK_A2DP_SRC_APTX_CODEC) && (MTK_A2DP_SRC_APTX_CODEC == TRUE)
    BOOLEAN codec_matched = false;
#endif

    FUNC_TRACE();

    /* Retrieve the peer info */
    p_peer = bta_av_co_get_peer(hndl);
    if (p_peer == NULL)
    {
        APPL_TRACE_ERROR("bta_av_co_audio_getconfig could not find peer entry");
        return A2D_FAIL;
    }

    if (p_peer->uuid_to_connect == UUID_SERVCLASS_AUDIO_SOURCE)
    {
        result = bta_av_audio_sink_getconfig(hndl, codec_type, p_codec_info, p_sep_info_idx,
                                             seid, p_num_protect, p_protect_info);
        return result;
    }
    APPL_TRACE_DEBUG("bta_av_co_audio_getconfig handle:0x%x codec_type:%d seid:%d",
                                                              hndl, codec_type, seid);
    APPL_TRACE_DEBUG("num_protect:0x%02x protect_info:0x%02x%02x%02x",
        *p_num_protect, p_protect_info[0], p_protect_info[1], p_protect_info[2]);

    APPL_TRACE_DEBUG("bta_av_co_audio_getconfig peer(o=%d,n_snks=%d,n_rx_snks=%d,n_sup_snks=%d)",
            p_peer->opened, p_peer->num_snks, p_peer->num_rx_snks, p_peer->num_sup_snks);

    p_peer->num_rx_snks++;

    /* Check if this is a supported configuration */
    supported = FALSE;
    switch (codec_type)
    {
    case BTA_AV_CODEC_SBC:
        supported = TRUE;
        break;
#if defined(MTK_A2DP_SRC_APTX_CODEC) && (MTK_A2DP_SRC_APTX_CODEC == TRUE)
    case NON_A2DP_MEDIA_CT:
        if(p_codec_info[3] != (UINT8)(CSR_APTX_VENDOR_ID & 0xff))
        {
            APPL_TRACE_ERROR("[bta_av_co_audio_getconfig] Not APTX");
        }
        else
        {
            supported = TRUE;
            if(bta_av_co_audio_codec_match(p_codec_info))
            {
                index = p_peer->num_rx_snks;
                codec_matched = true;
            }
            APPL_TRACE_DEBUG("bta_av_co_audio_peer_supports_codec: aptX , matched: %d" , codec_matched);
        }
        break;
#endif
#if defined(MTK_A2DP_SRC_AAC_CODEC) && (MTK_A2DP_SRC_AAC_CODEC == TRUE)
    case BTA_AV_CODEC_M24:
        supported = TRUE;
        APPL_TRACE_DEBUG("[bta_av_co_audio_getconfig] BTA_AV_CODEC_M24 (AAC codec)");
        break;
#endif

    default:
        break;
    }

    if (supported)
    {
        /* If there is room for a new one */
        if (p_peer->num_sup_snks < BTA_AV_CO_NUM_ELEMENTS(p_peer->snks))
        {
            p_sink = &p_peer->snks[p_peer->num_sup_snks++];

            APPL_TRACE_DEBUG("bta_av_co_audio_getconfig saved caps[%x:%x:%x:%x:%x:%x]",
                    p_codec_info[1], p_codec_info[2], p_codec_info[3],
                    p_codec_info[4], p_codec_info[5], p_codec_info[6]);

            memcpy(p_sink->codec_caps, p_codec_info, AVDT_CODEC_SIZE);
            p_sink->codec_type = codec_type;
            p_sink->sep_info_idx = *p_sep_info_idx;
            p_sink->seid = seid;
            p_sink->num_protect = *p_num_protect;
            memcpy(p_sink->protect_info, p_protect_info, BTA_AV_CP_INFO_LEN);
        }
        else
        {
            APPL_TRACE_ERROR("bta_av_co_audio_getconfig no more room for SNK info");
        }
    }

    /* If last SNK get capabilities or all supported codec capa retrieved */
    if ((p_peer->num_rx_snks == p_peer->num_snks) ||
#if defined(MTK_A2DP_SRC_APTX_CODEC) && (MTK_A2DP_SRC_APTX_CODEC == TRUE)
        codec_matched ||
#endif
        (p_peer->num_sup_snks == BTA_AV_CO_NUM_ELEMENTS(p_peer->snks)))
    {
        APPL_TRACE_DEBUG("bta_av_co_audio_getconfig last sink reached");

        /* Protect access to bta_av_co_cb.codec_cfg */
        mutex_global_lock();

        /* Find a sink that matches the codec config */
        if (bta_av_co_audio_peer_supports_codec(p_peer, &index))
        {
            /* stop fetching caps once we retrieved a supported codec */
            if (p_peer->acp)
            {
                *p_sep_info_idx = p_peer->num_seps;
                APPL_TRACE_EVENT("no need to fetch more SEPs");
            }

            p_sink = &p_peer->snks[index];

            /* Build the codec configuration for this sink */
            if (bta_av_co_audio_codec_build_config(p_sink->codec_caps, codec_cfg))
            {
                APPL_TRACE_DEBUG("bta_av_co_audio_getconfig reconfig p_codec_info[%x:%x:%x:%x:%x:%x]",
                        codec_cfg[1], codec_cfg[2], codec_cfg[3],
                        codec_cfg[4], codec_cfg[5], codec_cfg[6]);

                /* Save the new configuration */
                p_peer->p_snk = p_sink;
                memcpy(p_peer->codec_cfg, codec_cfg, AVDT_CODEC_SIZE);

                /* By default, no content protection */
                *p_num_protect = 0;

#if defined(BTA_AV_CO_CP_SCMS_T) && (BTA_AV_CO_CP_SCMS_T == TRUE)
                /* Check if this sink supports SCMS */
                if (bta_av_co_audio_sink_has_scmst(p_sink)
#if defined(MTK_STACK_CONFIG_BL) && (MTK_STACK_CONFIG_BL == TRUE)
                    && (!interop_mtk_match_addr_name(INTEROP_MTK_A2DP_NOT_SET_SCMT, (const bt_bdaddr_t *)&p_peer->addr))
#endif
#if (defined(MTK_A2DP_SRC_APTX_CODEC) && (MTK_A2DP_SRC_APTX_CODEC == TRUE)) || (defined(MTK_A2DP_SRC_AAC_CODEC) && (MTK_A2DP_SRC_AAC_CODEC == TRUE))
                    && (p_peer->p_snk->codec_type == BTA_AV_CODEC_SBC)
#endif
                    )
                {
                    p_peer->cp_active = TRUE;
                    bta_av_co_cb.cp.active = TRUE;
                    *p_num_protect = BTA_AV_CP_INFO_LEN;
                    memcpy(p_protect_info, bta_av_co_cp_scmst, BTA_AV_CP_INFO_LEN);
                }
                else
                {
                    p_peer->cp_active = FALSE;
                    bta_av_co_cb.cp.active = FALSE;
                }
#endif

                /* If acceptor -> reconfig otherwise reply for configuration */
                if (p_peer->acp)
                {
                    if (p_peer->recfg_needed)
                    {
                        APPL_TRACE_DEBUG("bta_av_co_audio_getconfig call BTA_AvReconfig(x%x)", hndl);
                        BTA_AvReconfig(hndl, TRUE, p_sink->sep_info_idx, p_peer->codec_cfg, *p_num_protect, (UINT8 *)bta_av_co_cp_scmst);
                    }
                }
                else
                {
                    *p_sep_info_idx = p_sink->sep_info_idx;
                    memcpy(p_codec_info, p_peer->codec_cfg, AVDT_CODEC_SIZE);
                }
                result =  A2D_SUCCESS;
            }
        }
        /* Protect access to bta_av_co_cb.codec_cfg */
        mutex_global_unlock();
    }
    return result;
}

/*******************************************************************************
 **
 ** Function         bta_av_co_audio_setconfig
 **
 ** Description      This callout function is executed by AV to set the codec and
 **                  content protection configuration of the audio stream.
 **
 **
 ** Returns          void
 **
 *******************************************************************************/
void bta_av_co_audio_setconfig(tBTA_AV_HNDL hndl, tBTA_AV_CODEC codec_type,
        UINT8 *p_codec_info, UINT8 seid, BD_ADDR addr, UINT8 num_protect, UINT8 *p_protect_info,
        UINT8 t_local_sep, UINT8 avdt_handle)
{
    tBTA_AV_CO_PEER *p_peer;
    UINT8 status = A2D_SUCCESS;
    UINT8 category = A2D_SUCCESS;
    BOOLEAN recfg_needed = FALSE;
    BOOLEAN codec_cfg_supported = FALSE;
    UNUSED(seid);
    UNUSED(addr);

    FUNC_TRACE();

    APPL_TRACE_DEBUG("bta_av_co_audio_setconfig p_codec_info[%x:%x:%x:%x:%x:%x]",
            p_codec_info[1], p_codec_info[2], p_codec_info[3],
            p_codec_info[4], p_codec_info[5], p_codec_info[6]);
    APPL_TRACE_DEBUG("num_protect:0x%02x protect_info:0x%02x%02x%02x",
        num_protect, p_protect_info[0], p_protect_info[1], p_protect_info[2]);

    /* Retrieve the peer info */
    p_peer = bta_av_co_get_peer(hndl);
    if (p_peer == NULL)
    {
        APPL_TRACE_ERROR("bta_av_co_audio_setconfig could not find peer entry");

        /* Call call-in rejecting the configuration */
        bta_av_ci_setconfig(hndl, A2D_BUSY, AVDT_ASC_CODEC, 0, NULL, FALSE, avdt_handle);
        return;
    }
    APPL_TRACE_DEBUG("bta_av_co_audio_setconfig peer(o=%d,n_snks=%d,n_rx_snks=%d,n_sup_snks=%d)",
            p_peer->opened, p_peer->num_snks, p_peer->num_rx_snks, p_peer->num_sup_snks);

    /* Sanity check: should not be opened at this point */
    if (p_peer->opened)
    {
        APPL_TRACE_ERROR("bta_av_co_audio_setconfig peer already in use");
    }

#if defined(BTA_AV_CO_CP_SCMS_T) && (BTA_AV_CO_CP_SCMS_T == TRUE)
    if (num_protect != 0)
    {
        /* If CP is supported */
        if ((num_protect != 1) ||
            (bta_av_co_cp_is_scmst(p_protect_info) == FALSE))
        {
            APPL_TRACE_ERROR("bta_av_co_audio_setconfig wrong CP configuration");
            status = A2D_BAD_CP_TYPE;
            category = AVDT_ASC_PROTECT;
        }
    }
#else
    /* Do not support content protection for the time being */
    if (num_protect != 0)
    {
        APPL_TRACE_ERROR("bta_av_co_audio_setconfig wrong CP configuration");
        status = A2D_BAD_CP_TYPE;
        category = AVDT_ASC_PROTECT;
    }
#endif
    if (status == A2D_SUCCESS)
    {
        if(AVDT_TSEP_SNK == t_local_sep)
        {
            codec_cfg_supported = bta_av_co_audio_sink_supports_config(codec_type, p_codec_info);
            APPL_TRACE_DEBUG(" Peer is  A2DP SRC ");
        }
        if(AVDT_TSEP_SRC == t_local_sep)
        {
            codec_cfg_supported = bta_av_co_audio_media_supports_config(codec_type, p_codec_info);
            APPL_TRACE_DEBUG(" Peer is A2DP SINK ");
        }
        /* Check if codec configuration is supported */
        if (codec_cfg_supported)
        {

            /* Protect access to bta_av_co_cb.codec_cfg */
            mutex_global_lock();

#if defined(MTK_COMMON) && (MTK_COMMON == TRUE)
            bta_av_co_cb.codec_cfg.id = codec_type;
            APPL_TRACE_DEBUG(" codec_cfg.id =%d", bta_av_co_cb.codec_cfg.id);
#endif

            /* Check if the configuration matches the current codec config */
            switch (bta_av_co_cb.codec_cfg.id)
            {
            case BTIF_AV_CODEC_SBC:
                if ((codec_type != BTA_AV_CODEC_SBC) || memcmp(p_codec_info, bta_av_co_cb.codec_cfg.info, 5))
                {
                    recfg_needed = TRUE;
                }
                else if ((num_protect == 1) && (!bta_av_co_cb.cp.active))
                {
                    recfg_needed = TRUE;
                }

                /* if remote side requests a restricted notify sinks preferred bitpool range as all other params are
                   already checked for validify */
                APPL_TRACE_EVENT("remote peer setconfig bitpool range [%d:%d]",
                   p_codec_info[BTA_AV_CO_SBC_MIN_BITPOOL_OFF],
                   p_codec_info[BTA_AV_CO_SBC_MAX_BITPOOL_OFF] );

                bta_av_co_cb.codec_cfg_setconfig.id = BTIF_AV_CODEC_SBC;
                memcpy(bta_av_co_cb.codec_cfg_setconfig.info, p_codec_info, AVDT_CODEC_SIZE);
                if(AVDT_TSEP_SNK == t_local_sep)
                {
                    /* If Peer is SRC, and our cfg subset matches with what is requested by peer, then
                                         just accept what peer wants */
                    memcpy(bta_av_co_cb.codec_cfg.info, p_codec_info, AVDT_CODEC_SIZE);
                    recfg_needed = FALSE;
                }
#if defined(MTK_COMMON) && (MTK_COMMON == TRUE)
                /* codec_cfg_supported is true, which means remote set_configuration parameters can match local capability.
                    So it is reasonable to save the configure param as current codec_cfg for further codec setup.
                    Still keep recfg_needed as the original value to keep the possiblity to do reconfiguration.  */
                memcpy(bta_av_co_cb.codec_cfg.info, p_codec_info, AVDT_CODEC_SIZE);
#endif
                break;
#if defined(MTK_A2DP_SRC_APTX_CODEC) && (MTK_A2DP_SRC_APTX_CODEC == TRUE)
            case NON_A2DP_MEDIA_CT:
                if ((codec_type != NON_A2DP_MEDIA_CT) || memcmp(p_codec_info, bta_av_co_cb.codec_cfg.info, 5))
                {
                    recfg_needed = TRUE;
                }
                else if ((num_protect == 1) && (!bta_av_co_cb.cp.active))
                {
                    recfg_needed = TRUE;
                }
                bta_av_co_cb.codec_cfg_setconfig.id = NON_A2DP_MEDIA_CT;
                memcpy(bta_av_co_cb.codec_cfg_setconfig.info, p_codec_info, AVDT_CODEC_SIZE);

                /* codec_cfg_supported is true, which means remote set_configuration parameters can match local capability.
                    So it is reasonable to save the configure param as current codec_cfg for further codec setup.
                    Still keep recfg_needed as the original value to keep the possiblity to do reconfiguration.  */
                memcpy(bta_av_co_cb.codec_cfg.info, p_codec_info, AVDT_CODEC_SIZE);
                break;
#endif
#if (defined(MTK_A2DP_SRC_AAC_CODEC) && (MTK_A2DP_SRC_AAC_CODEC == TRUE) || defined(MTK_A2DP_SNK_AAC_CODEC) && (MTK_A2DP_SNK_AAC_CODEC == TRUE))
            case BTIF_AV_CODEC_AAC:
                bta_av_co_cb.codec_cfg_setconfig.id = BTIF_AV_CODEC_AAC;
                memcpy(bta_av_co_cb.codec_cfg_setconfig.info, p_codec_info, AVDT_CODEC_SIZE);

                /* codec_cfg_supported is true, which means remote set_configuration parameters can match local capability.
                    So it is reasonable to save the configure param as current codec_cfg for further codec setup.
                    Still keep recfg_needed as the original value to keep the possiblity to do reconfiguration.  */
                memcpy(bta_av_co_cb.codec_cfg.info, p_codec_info, AVDT_CODEC_SIZE);
                if(AVDT_TSEP_SNK == t_local_sep)
                {
                    /* If Peer is SRC, and our cfg subset matches with what is requested by peer, then
                                         just accept what peer wants */
                    recfg_needed = FALSE;
                }
                break;
#endif
            default:
                APPL_TRACE_ERROR("bta_av_co_audio_setconfig unsupported cid %d", bta_av_co_cb.codec_cfg.id);
                recfg_needed = TRUE;
                break;
            }
            /* Protect access to bta_av_co_cb.codec_cfg */
            mutex_global_unlock();
        }
        else
        {
            category = AVDT_ASC_CODEC;
            status = A2D_WRONG_CODEC;
        }
    }

    if (status != A2D_SUCCESS)
    {
        APPL_TRACE_DEBUG("bta_av_co_audio_setconfig reject s=%d c=%d", status, category);

        /* Call call-in rejecting the configuration */
        bta_av_ci_setconfig(hndl, status, category, 0, NULL, FALSE, avdt_handle);
    }
    else
    {
        /* Mark that this is an acceptor peer */
        p_peer->acp = TRUE;
        p_peer->recfg_needed = recfg_needed;

        APPL_TRACE_DEBUG("bta_av_co_audio_setconfig accept reconf=%d", recfg_needed);

        /* Call call-in accepting the configuration */
        bta_av_ci_setconfig(hndl, A2D_SUCCESS, A2D_SUCCESS, 0, NULL, recfg_needed, avdt_handle);
    }
}

/*******************************************************************************
 **
 ** Function         bta_av_co_audio_open
 **
 ** Description      This function is called by AV when the audio stream connection
 **                  is opened.
 **
 **
 ** Returns          void
 **
 *******************************************************************************/
void bta_av_co_audio_open(tBTA_AV_HNDL hndl, tBTA_AV_CODEC codec_type, UINT8 *p_codec_info,
                          UINT16 mtu)
{
    tBTA_AV_CO_PEER *p_peer;
#if defined(MTK_A2DP_SRC_APTX_CODEC) && (MTK_A2DP_SRC_APTX_CODEC == TRUE)
#else
    UNUSED(p_codec_info);
#endif

    FUNC_TRACE();

    APPL_TRACE_DEBUG("bta_av_co_audio_open mtu:%d codec_type:%d", mtu, codec_type);

    /* Retrieve the peer info */
    p_peer = bta_av_co_get_peer(hndl);
    if (p_peer == NULL)
    {
        APPL_TRACE_ERROR("bta_av_co_audio_setconfig could not find peer entry");
    }
    else
    {
        p_peer->opened = TRUE;
        p_peer->mtu = mtu;

#if defined(MTK_A2DP_SRC_APTX_CODEC) && (MTK_A2DP_SRC_APTX_CODEC == TRUE)
        if (p_codec_info == NULL)
        {
            APPL_TRACE_ERROR("bta_av_co_audio_open codec info is null");
            return;
        }
        APPL_TRACE_DEBUG("bta_av_co_audio_open p_codec_info[%x:%x:%x:%x:%x:%x]",
                p_codec_info[1], p_codec_info[2], p_codec_info[3],
                p_codec_info[4], p_codec_info[5], p_codec_info[6]);
        if ((codec_type == NON_A2DP_MEDIA_CT) && (p_codec_info[3] == (UINT8)(CSR_APTX_VENDOR_ID & 0xff)))
        {
            property_set(BLUETOOTH_APTX_PROPERTY, "1");
        }
        else
        {
            property_set(BLUETOOTH_APTX_PROPERTY, "0");
        }
#endif
    }
}

/*******************************************************************************
 **
 ** Function         bta_av_co_audio_close
 **
 ** Description      This function is called by AV when the audio stream connection
 **                  is closed.
 **
 **
 ** Returns          void
 **
 *******************************************************************************/
void bta_av_co_audio_close(tBTA_AV_HNDL hndl, tBTA_AV_CODEC codec_type, UINT16 mtu)

{
    tBTA_AV_CO_PEER *p_peer;
    UNUSED(codec_type);
    UNUSED(mtu);

    FUNC_TRACE();

    APPL_TRACE_DEBUG("bta_av_co_audio_close");

    /* Retrieve the peer info */
    p_peer = bta_av_co_get_peer(hndl);
    if (p_peer)
    {
        /* Mark the peer closed and clean the peer info */
        memset(p_peer, 0, sizeof(*p_peer));

#if defined(MTK_A2DP_SRC_APTX_CODEC) && (MTK_A2DP_SRC_APTX_CODEC == TRUE)
        property_set(BLUETOOTH_APTX_PROPERTY, "0");
#endif
    }
    else
    {
        APPL_TRACE_ERROR("bta_av_co_audio_close could not find peer entry");
    }

    /* reset remote preference through setconfig */
    bta_av_co_cb.codec_cfg_setconfig.id = BTIF_AV_CODEC_NONE;
}

/*******************************************************************************
 **
 ** Function         bta_av_co_audio_start
 **
 ** Description      This function is called by AV when the audio streaming data
 **                  transfer is started.
 **
 **
 ** Returns          void
 **
 *******************************************************************************/
void bta_av_co_audio_start(tBTA_AV_HNDL hndl, tBTA_AV_CODEC codec_type,
                           UINT8 *p_codec_info, BOOLEAN *p_no_rtp_hdr)
{
    UNUSED(hndl);
    UNUSED(codec_type);
    UNUSED(p_codec_info);
    UNUSED(p_no_rtp_hdr);

    FUNC_TRACE();

    APPL_TRACE_DEBUG("bta_av_co_audio_start");

}

/*******************************************************************************
 **
 ** Function         bta_av_co_audio_stop
 **
 ** Description      This function is called by AV when the audio streaming data
 **                  transfer is stopped.
 **
 **
 ** Returns          void
 **
 *******************************************************************************/
extern void bta_av_co_audio_stop(tBTA_AV_HNDL hndl, tBTA_AV_CODEC codec_type)
{
    UNUSED(hndl);
    UNUSED(codec_type);

    FUNC_TRACE();

    APPL_TRACE_DEBUG("bta_av_co_audio_stop");
}

/*******************************************************************************
 **
 ** Function         bta_av_co_audio_src_data_path
 **
 ** Description      This function is called to manage data transfer from
 **                  the audio codec to AVDTP.
 **
 ** Returns          Pointer to the GKI buffer to send, NULL if no buffer to send
 **
 *******************************************************************************/
void * bta_av_co_audio_src_data_path(tBTA_AV_CODEC codec_type, UINT32 *p_len,
                                     UINT32 *p_timestamp)
{
    BT_HDR *p_buf;
    UNUSED(p_len);

    FUNC_TRACE();

    p_buf = btif_media_aa_readbuf();
    if (p_buf != NULL)
    {
        switch (codec_type)
        {
        case BTA_AV_CODEC_SBC:
            /* In media packet SBC, the following information is available:
             * p_buf->layer_specific : number of SBC frames in the packet
             * p_buf->word[0] : timestamp
             */
            /* Retrieve the timestamp information from the media packet */
            *p_timestamp = *((UINT32 *) (p_buf + 1));

            /* Set up packet header */
            bta_av_sbc_bld_hdr(p_buf, p_buf->layer_specific);
            break;
#if defined(MTK_A2DP_SRC_AAC_CODEC) && (MTK_A2DP_SRC_AAC_CODEC == TRUE)
        case BTA_AV_CODEC_M24:
            *p_timestamp = *((UINT32 *) (p_buf + 1));
            break;
#endif
#if defined(MTK_A2DP_SRC_APTX_CODEC) && (MTK_A2DP_SRC_APTX_CODEC == TRUE)
        case NON_A2DP_MEDIA_CT:
            break;
#endif
        default:
            APPL_TRACE_ERROR("bta_av_co_audio_src_data_path Unsupported codec type (%d)", codec_type);
            break;
        }
#if defined(BTA_AV_CO_CP_SCMS_T) && (BTA_AV_CO_CP_SCMS_T == TRUE)
        {
            UINT8 *p;
            if (bta_av_co_cp_is_active())
            {
                p_buf->len++;
                p_buf->offset--;
                p = (UINT8 *)(p_buf + 1) + p_buf->offset;
                *p = bta_av_co_cp_get_flag();
            }
        }
#endif
    }
    return p_buf;
}

/*******************************************************************************
 **
 ** Function         bta_av_co_audio_drop
 **
 ** Description      An Audio packet is dropped. .
 **                  It's very likely that the connected headset with this handle
 **                  is moved far away. The implementation may want to reduce
 **                  the encoder bit rate setting to reduce the packet size.
 **
 ** Returns          void
 **
 *******************************************************************************/
void bta_av_co_audio_drop(tBTA_AV_HNDL hndl)
{
    FUNC_TRACE();

    APPL_TRACE_ERROR("bta_av_co_audio_drop dropped: x%x", hndl);
}

/*******************************************************************************
 **
 ** Function         bta_av_co_audio_delay
 **
 ** Description      This function is called by AV when the audio stream connection
 **                  needs to send the initial delay report to the connected SRC.
 **
 **
 ** Returns          void
 **
 *******************************************************************************/
void bta_av_co_audio_delay(tBTA_AV_HNDL hndl, UINT16 delay)
{
    FUNC_TRACE();

    APPL_TRACE_ERROR("bta_av_co_audio_delay handle: x%x, delay:0x%x", hndl, delay);
}

#if defined(MTK_COMMON) && (MTK_COMMON == TRUE)
void bta_av_co_audio_get_sep_info(UINT8 tsep, UINT8 *start_idx, UINT8 *max_sep_idx)
{
    if (AVDT_TSEP_SRC == tsep)
    {
        *start_idx = BTIF_SV_AV_AA_SBC_INDEX;
        *max_sep_idx = BTIF_SV_AV_AA_SBC_SINK_INDEX;
    }
    else
    {
        *start_idx = BTIF_SV_AV_AA_SBC_SINK_INDEX;
        *max_sep_idx = BTIF_SV_AV_AA_SEP_INDEX;
    }
}
#endif

/*******************************************************************************
 **
 ** Function         bta_av_co_audio_codec_build_config
 **
 ** Description      Build the codec configuration
 **
 ** Returns          TRUE if the codec was built successfully, FALSE otherwise
 **
 *******************************************************************************/
static BOOLEAN bta_av_co_audio_codec_build_config(const UINT8 *p_codec_caps, UINT8 *p_codec_cfg)
{
    FUNC_TRACE();

    memset(p_codec_cfg, 0, AVDT_CODEC_SIZE);

    switch (bta_av_co_cb.codec_cfg.id)
    {
    case BTIF_AV_CODEC_SBC:
        /*  only copy the relevant portions for this codec to avoid issues when
            comparing codec configs covering larger codec sets than SBC (7 bytes) */
        memcpy(p_codec_cfg, bta_av_co_cb.codec_cfg.info, BTA_AV_CO_SBC_MAX_BITPOOL_OFF+1);

        /* Update the bit pool boundaries with the codec capabilities */
        p_codec_cfg[BTA_AV_CO_SBC_MIN_BITPOOL_OFF] = p_codec_caps[BTA_AV_CO_SBC_MIN_BITPOOL_OFF];
        p_codec_cfg[BTA_AV_CO_SBC_MAX_BITPOOL_OFF] = p_codec_caps[BTA_AV_CO_SBC_MAX_BITPOOL_OFF];

        APPL_TRACE_EVENT("bta_av_co_audio_codec_build_config : bitpool min %d, max %d",
                    p_codec_cfg[BTA_AV_CO_SBC_MIN_BITPOOL_OFF],
                    p_codec_caps[BTA_AV_CO_SBC_MAX_BITPOOL_OFF]);
        break;
#if defined(MTK_A2DP_SRC_APTX_CODEC) && (MTK_A2DP_SRC_APTX_CODEC == TRUE)
    case NON_A2DP_MEDIA_CT:
        memcpy(p_codec_cfg, bta_av_co_cb.codec_cfg.info, BTA_AV_CO_APTX_CODEC_LEN+1);
        break;
#endif
#if defined(MTK_A2DP_SRC_AAC_CODEC) && (MTK_A2DP_SRC_AAC_CODEC == TRUE)
    case BTIF_AV_CODEC_AAC:
        memcpy(p_codec_cfg, bta_av_co_cb.codec_cfg.info, BTA_AV_CO_AAC_CODEC_LEN+1);
        break;
#endif
    default:
        APPL_TRACE_ERROR("bta_av_co_audio_codec_build_config: unsupported codec id %d", bta_av_co_cb.codec_cfg.id);
        return FALSE;
        break;
    }
    return TRUE;
}

/*******************************************************************************
 **
 ** Function         bta_av_co_audio_codec_cfg_matches_caps
 **
 ** Description      Check if a codec config matches a codec capabilities
 **
 ** Returns          TRUE if it codec config is supported, FALSE otherwise
 **
 *******************************************************************************/
static BOOLEAN bta_av_co_audio_codec_cfg_matches_caps(UINT8 codec_id, const UINT8 *p_codec_caps, const UINT8 *p_codec_cfg)
{
    FUNC_TRACE();

    switch(codec_id)
    {
    case BTIF_AV_CODEC_SBC:

        APPL_TRACE_EVENT("bta_av_co_audio_codec_cfg_matches_caps : min %d/%d max %d/%d",
           p_codec_caps[BTA_AV_CO_SBC_MIN_BITPOOL_OFF],
           p_codec_cfg[BTA_AV_CO_SBC_MIN_BITPOOL_OFF],
           p_codec_caps[BTA_AV_CO_SBC_MAX_BITPOOL_OFF],
           p_codec_cfg[BTA_AV_CO_SBC_MAX_BITPOOL_OFF]);

        /* Must match all items exactly except bitpool boundaries which can be adjusted */
        if (!((p_codec_caps[BTA_AV_CO_SBC_FREQ_CHAN_OFF] & p_codec_cfg[BTA_AV_CO_SBC_FREQ_CHAN_OFF]) &&
              (p_codec_caps[BTA_AV_CO_SBC_BLOCK_BAND_OFF] & p_codec_cfg[BTA_AV_CO_SBC_BLOCK_BAND_OFF])))
        {
            APPL_TRACE_EVENT("FALSE %x %x %x %x",
                    p_codec_caps[BTA_AV_CO_SBC_FREQ_CHAN_OFF],
                    p_codec_cfg[BTA_AV_CO_SBC_FREQ_CHAN_OFF],
                    p_codec_caps[BTA_AV_CO_SBC_BLOCK_BAND_OFF],
                    p_codec_cfg[BTA_AV_CO_SBC_BLOCK_BAND_OFF]);
            return FALSE;
        }
        break;
#if defined(MTK_A2DP_SRC_AAC_CODEC) && (MTK_A2DP_SRC_AAC_CODEC == TRUE)
    case BTIF_AV_CODEC_AAC:
            APPL_TRACE_DEBUG("bta_av_co_audio_codec_cfg_matches_caps : BTIF_AV_CODEC_AAC");
            APPL_TRACE_DEBUG("@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@");
            APPL_TRACE_DEBUG("p_codec_caps[0], %x", p_codec_caps[0]);
            APPL_TRACE_DEBUG("p_codec_caps[1], %x", p_codec_caps[1]);
            APPL_TRACE_DEBUG("p_codec_caps[2], %x", p_codec_caps[2]);
            APPL_TRACE_DEBUG("p_codec_caps[3], %x", p_codec_caps[3]);
            APPL_TRACE_DEBUG("p_codec_caps[4], %x", p_codec_caps[4]);
            APPL_TRACE_DEBUG("p_codec_caps[5], %x", p_codec_caps[5]);
            APPL_TRACE_DEBUG("p_codec_caps[6], %x", p_codec_caps[6]);
            APPL_TRACE_DEBUG("p_codec_caps[7], %x", p_codec_caps[7]);
            APPL_TRACE_DEBUG("p_codec_caps[8], %x", p_codec_caps[8]);
            APPL_TRACE_DEBUG("p_codec_caps[9], %x", p_codec_caps[9]);
            APPL_TRACE_DEBUG("p_codec_cfg[0], %x", p_codec_cfg[0]);
            APPL_TRACE_DEBUG("p_codec_cfg[1], %x", p_codec_cfg[1]);
            APPL_TRACE_DEBUG("p_codec_cfg[2], %x", p_codec_cfg[2]);
            APPL_TRACE_DEBUG("p_codec_cfg[3], %x", p_codec_cfg[3]);
            APPL_TRACE_DEBUG("p_codec_cfg[4], %x", p_codec_cfg[4]);
            APPL_TRACE_DEBUG("p_codec_cfg[5], %x", p_codec_cfg[5]);
            APPL_TRACE_DEBUG("p_codec_cfg[6], %x", p_codec_cfg[6]);
            APPL_TRACE_DEBUG("p_codec_cfg[7], %x", p_codec_cfg[7]);
            APPL_TRACE_DEBUG("p_codec_cfg[8], %x", p_codec_cfg[8]);
            APPL_TRACE_DEBUG("p_codec_cfg[9], %x", p_codec_cfg[9]);
            APPL_TRACE_DEBUG("@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@");

            tA2D_AAC_CIE    peer_snk_cap;
            tA2D_AAC_CIE    local_src_cap;
            A2D_ParsAACInfo(&peer_snk_cap, p_codec_caps, TRUE);
            A2D_ParsAACInfo(&local_src_cap, p_codec_cfg, TRUE);

            if(!((p_codec_caps[0] == BTA_AV_CO_AAC_CODEC_LEN) &&
                (p_codec_caps[2] == BTIF_AV_CODEC_AAC) &&
                (peer_snk_cap.object_type & local_src_cap.object_type) &&    // opject type
                (peer_snk_cap.samp_freq & local_src_cap.samp_freq) &&        // sampling rate
                (peer_snk_cap.channels & local_src_cap.channels)             //  channels
                ))
            {
                APPL_TRACE_EVENT("p_codec_cfg[0]:0x%x, p_codec_cfg[2]:0x%x",p_codec_caps[0], p_codec_caps[2]);
                APPL_TRACE_EVENT("peer_snk_cap  -->[object_type:0x%x][samp_freq:0x%x][channels:0x%x]", peer_snk_cap.object_type, peer_snk_cap.samp_freq, peer_snk_cap.channels);
                APPL_TRACE_EVENT("local_src_cap -->[object_type:0x%x][samp_freq:0x%x][channels:0x%x]", local_src_cap.object_type, local_src_cap.samp_freq, local_src_cap.channels);
                return FALSE;
            }
        break;
#endif
#if defined(MTK_A2DP_SRC_APTX_CODEC) && (MTK_A2DP_SRC_APTX_CODEC == TRUE)
    case NON_A2DP_MEDIA_CT:
            APPL_TRACE_DEBUG("@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@");
            APPL_TRACE_DEBUG("p_codec_caps[0], %x", p_codec_caps[0]);
            APPL_TRACE_DEBUG("p_codec_caps[1], %x", p_codec_caps[1]);
            APPL_TRACE_DEBUG("p_codec_caps[2], %x", p_codec_caps[2]);
            APPL_TRACE_DEBUG("p_codec_caps[3], %x", p_codec_caps[3]);
            APPL_TRACE_DEBUG("p_codec_caps[4], %x", p_codec_caps[4]);
            APPL_TRACE_DEBUG("p_codec_caps[5], %x", p_codec_caps[5]);
            APPL_TRACE_DEBUG("p_codec_caps[6], %x", p_codec_caps[6]);
            APPL_TRACE_DEBUG("p_codec_caps[7], %x", p_codec_caps[7]);
            APPL_TRACE_DEBUG("p_codec_caps[8], %x", p_codec_caps[8]);
            APPL_TRACE_DEBUG("p_codec_caps[9], %x", p_codec_caps[9]);
            APPL_TRACE_DEBUG("p_codec_cfg[0], %x", p_codec_cfg[0]);
            APPL_TRACE_DEBUG("p_codec_cfg[1], %x", p_codec_cfg[1]);
            APPL_TRACE_DEBUG("p_codec_cfg[2], %x", p_codec_cfg[2]);
            APPL_TRACE_DEBUG("p_codec_cfg[3], %x", p_codec_cfg[3]);
            APPL_TRACE_DEBUG("p_codec_cfg[4], %x", p_codec_cfg[4]);
            APPL_TRACE_DEBUG("p_codec_cfg[5], %x", p_codec_cfg[5]);
            APPL_TRACE_DEBUG("p_codec_cfg[6], %x", p_codec_cfg[6]);
            APPL_TRACE_DEBUG("p_codec_cfg[7], %x", p_codec_cfg[7]);
            APPL_TRACE_DEBUG("p_codec_cfg[8], %x", p_codec_cfg[8]);
            APPL_TRACE_DEBUG("p_codec_cfg[9], %x", p_codec_cfg[9]);
            APPL_TRACE_DEBUG("BTA_AV_CO_APTX_CODEC_LEN, %x", BTA_AV_CO_APTX_CODEC_LEN);
            APPL_TRACE_DEBUG("NON_A2DP_MEDIA_CT, %x", NON_A2DP_MEDIA_CT);
            APPL_TRACE_DEBUG("@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@");
             if(!((p_codec_caps[0] == BTA_AV_CO_APTX_CODEC_LEN) &&
                        (p_codec_caps[2] == NON_A2DP_MEDIA_CT) &&
                        (p_codec_caps[3] == p_codec_cfg[3] ) && //vendor id
                        (p_codec_caps[4] == p_codec_cfg[4]) &&
                        (p_codec_caps[5] == p_codec_cfg[5]) &&
                        (p_codec_caps[6] == p_codec_cfg[6]) &&
                        (p_codec_caps[9] & p_codec_cfg[9]))) //sampling rate
                 {
                     APPL_TRACE_EVENT("FALSE : Caps-> capability len:%x configured:%x,  codec type:%x configured:%x",
                                       p_codec_caps[0],BTA_AV_CO_APTX_CODEC_LEN, p_codec_caps[2], NON_A2DP_MEDIA_CT);
                     APPL_TRACE_EVENT(" Caps -> vendor id: %x %x %x %x", p_codec_caps[3], p_codec_caps[4], p_codec_caps[5], p_codec_caps[6]);
                     APPL_TRACE_EVENT(" configured: %x %x %x %x",p_codec_caps[3], p_codec_caps[4], p_codec_caps[5], p_codec_caps[6]);
                     APPL_TRACE_EVENT(" Caps -> sample rate: %x configured %x",p_codec_caps[9], p_codec_cfg[9]);

                     return FALSE;
                 }
                 break;
#endif
    default:
        APPL_TRACE_ERROR("bta_av_co_audio_codec_cfg_matches_caps: unsupported codec id %d", codec_id);
        return FALSE;
        break;
    }
    APPL_TRACE_EVENT("TRUE");

    return TRUE;
}

/*******************************************************************************
 **
 ** Function         bta_av_co_audio_codec_match
 **
 ** Description      Check if a codec capabilities supports the codec config
 **
 ** Returns          TRUE if the connection supports this codec, FALSE otherwise
 **
 *******************************************************************************/
static BOOLEAN bta_av_co_audio_codec_match(const UINT8 *p_codec_caps)
{
    FUNC_TRACE();

    return bta_av_co_audio_codec_cfg_matches_caps(bta_av_co_cb.codec_cfg.id, p_codec_caps, bta_av_co_cb.codec_cfg.info);
}

/*******************************************************************************
 **
 ** Function         bta_av_co_audio_peer_reset_config
 **
 ** Description      Reset the peer codec configuration
 **
 ** Returns          Nothing
 **
 *******************************************************************************/
static void bta_av_co_audio_peer_reset_config(tBTA_AV_CO_PEER *p_peer)
{
    FUNC_TRACE();

    /* Indicate that there is no currently selected sink */
    p_peer->p_snk = NULL;
}

/*******************************************************************************
 **
 ** Function         bta_av_co_cp_is_scmst
 **
 ** Description      Check if a content protection service is SCMS-T
 **
 ** Returns          TRUE if this CP is SCMS-T, FALSE otherwise
 **
 *******************************************************************************/
static BOOLEAN bta_av_co_cp_is_scmst(const UINT8 *p_protectinfo)
{
    UINT16 cp_id;
    FUNC_TRACE();

    if (*p_protectinfo >= BTA_AV_CP_LOSC)
    {
        p_protectinfo++;
        STREAM_TO_UINT16(cp_id, p_protectinfo);
        if (cp_id == BTA_AV_CP_SCMS_T_ID)
        {
            APPL_TRACE_DEBUG("bta_av_co_cp_is_scmst: SCMS-T found");
            return TRUE;
        }
    }

    return FALSE;
}

/*******************************************************************************
 **
 ** Function         bta_av_co_audio_sink_has_scmst
 **
 ** Description      Check if a sink supports SCMS-T
 **
 ** Returns          TRUE if the sink supports this CP, FALSE otherwise
 **
 *******************************************************************************/
static BOOLEAN bta_av_co_audio_sink_has_scmst(const tBTA_AV_CO_SINK *p_sink)
{
    UINT8 index;
    const UINT8 *p;
    FUNC_TRACE();

    /* Check if sink supports SCMS-T */
    index = p_sink->num_protect;
    p = &p_sink->protect_info[0];

    while (index)
    {
        if (bta_av_co_cp_is_scmst(p))
        {
            return TRUE;
        }
        /* Move to the next SC */
        p += *p + 1;
        /* Decrement the SC counter */
        index--;
    }
    APPL_TRACE_DEBUG("bta_av_co_audio_sink_has_scmst: SCMS-T not found");
    return FALSE;
}

#if !(defined(MTK_COMMON) && (MTK_COMMON == TRUE))
/*******************************************************************************
 **
 ** Function         bta_av_co_audio_sink_supports_cp
 **
 ** Description      Check if a sink supports the current content protection
 **
 ** Returns          TRUE if the sink supports this CP, FALSE otherwise
 **
 *******************************************************************************/
static BOOLEAN bta_av_co_audio_sink_supports_cp(const tBTA_AV_CO_SINK *p_sink)
{
    FUNC_TRACE();

    /* Check if content protection is enabled for this stream */
    if (bta_av_co_cp_get_flag() != BTA_AV_CP_SCMS_COPY_FREE)
    {
        return bta_av_co_audio_sink_has_scmst(p_sink);
    }
    else
    {
        APPL_TRACE_DEBUG("bta_av_co_audio_sink_supports_cp: not required");
        return TRUE;
    }
}
#endif

/*******************************************************************************
 **
 ** Function         bta_av_co_audio_peer_supports_codec
 **
 ** Description      Check if a connection supports the codec config
 **
 ** Returns          TRUE if the connection supports this codec, FALSE otherwise
 **
 *******************************************************************************/
static BOOLEAN bta_av_co_audio_peer_supports_codec(tBTA_AV_CO_PEER *p_peer, UINT8 *p_snk_index)
{
    int index;
    UINT8 codec_type;
    FUNC_TRACE();

#if defined(MTK_A2DP_SRC_APTX_CODEC) && (MTK_A2DP_SRC_APTX_CODEC == TRUE)
    /* Configure the codec type to look for */
    codec_type = NON_A2DP_MEDIA_CT;
    for (index = 0; index < p_peer->num_sup_snks; index++)
    {
        if (p_peer->snks[index].codec_type == NON_A2DP_MEDIA_CT)
        {
            if (p_snk_index)
            {
                *p_snk_index = index;
            }
            APPL_TRACE_DEBUG("bta_av_co_audio_peer_supports_codec: aptX" );
            if(bta_av_co_audio_codec_match(p_peer->snks[index].codec_caps))
            {
                return TRUE;
            }
        }
    }
    APPL_TRACE_ERROR("bta_av_co_audio_peer_supports_codec: no available aptX sink" );
#endif // MTK_A2DP_SRC_APTX_CODEC == TRUE

#if defined(MTK_A2DP_SRC_AAC_CODEC) && (MTK_A2DP_SRC_AAC_CODEC == TRUE)
    bta_av_co_cb.codec_cfg.id = BTIF_AV_CODEC_AAC;  // if AAC is enabled and we cannot find APTX codec in remote device
    if (bta_av_co_cb.codec_cfg_setconfig.id == BTIF_AV_CODEC_NONE &&
            A2D_BldAACInfo(A2D_MEDIA_TYPE_AUDIO, (tA2D_AAC_CIE *)&btif_av_aac_default_config, bta_av_co_cb.codec_cfg.info) != A2D_SUCCESS)
    {
        APPL_TRACE_ERROR("bta_av_co_audio_peer_supports_codec A2D_BldSbcInfo failed");
    }

    codec_type = BTIF_AV_CODEC_AAC;
    for (index = 0; index < p_peer->num_sup_snks; index++)
    {
        APPL_TRACE_DEBUG("[SRC] aac_enabled:%d", aac_enabled);
        if (!aac_enabled)
        {
            APPL_TRACE_DEBUG("[SRC] AAC disabled");
            break;
        }
        if (p_peer->snks[index].codec_type == BTIF_AV_CODEC_AAC)
        {
            if (p_snk_index)
            {
                *p_snk_index = index;
            }
            APPL_TRACE_DEBUG("bta_av_co_audio_peer_supports_codec: AAC" );
            if(bta_av_co_audio_codec_match(p_peer->snks[index].codec_caps))
            {
                return TRUE;
            }
        }
    }
#endif //MTK_A2DP_SRC_AAC_CODEC == TRUE

#if (defined(MTK_A2DP_SRC_APTX_CODEC) && (MTK_A2DP_SRC_APTX_CODEC == TRUE)) || (defined(MTK_A2DP_SRC_AAC_CODEC) && (MTK_A2DP_SRC_AAC_CODEC == TRUE))
    bta_av_co_cb.codec_cfg.id = BTIF_AV_CODEC_SBC;
    if (bta_av_co_cb.codec_cfg_setconfig.id == BTIF_AV_CODEC_NONE &&
            A2D_BldSbcInfo(A2D_MEDIA_TYPE_AUDIO, (tA2D_SBC_CIE *)&btif_av_sbc_default_config, bta_av_co_cb.codec_cfg.info) != A2D_SUCCESS)
    {
        APPL_TRACE_ERROR("bta_av_co_audio_peer_supports_codec A2D_BldSbcInfo failed");
    }
#endif
    codec_type = bta_av_co_cb.codec_cfg.id;


    for (index = 0; index < p_peer->num_sup_snks; index++)
    {
        if (p_peer->snks[index].codec_type == codec_type)
        {
            switch (bta_av_co_cb.codec_cfg.id)
            {
            case BTIF_AV_CODEC_SBC:
                if (p_snk_index) *p_snk_index = index;
                return bta_av_co_audio_codec_match(p_peer->snks[index].codec_caps);
                break;


            default:
                APPL_TRACE_ERROR("bta_av_co_audio_peer_supports_codec: unsupported codec id %d", bta_av_co_cb.codec_cfg.id);
                return FALSE;
                break;
            }
        }
    }
    return FALSE;
}

/*******************************************************************************
 **
 ** Function         bta_av_co_audio_peer_src_supports_codec
 **
 ** Description      Check if a peer acting as src supports codec config
 **
 ** Returns          TRUE if the connection supports this codec, FALSE otherwise
 **
 *******************************************************************************/
static BOOLEAN bta_av_co_audio_peer_src_supports_codec(tBTA_AV_CO_PEER *p_peer, UINT8 *p_src_index)
{
    int index;
    UINT8 codec_type;
    FUNC_TRACE();

#if defined(MTK_A2DP_SNK_AAC_CODEC) && (MTK_A2DP_SNK_AAC_CODEC == TRUE)
    codec_type = BTIF_AV_CODEC_AAC;
    for (index = 0; index < p_peer->num_sup_srcs; index++)
    {
        APPL_TRACE_DEBUG("[SNK] aac_enabled:%d", aac_enabled);
        if (!aac_enabled)
        {
            APPL_TRACE_DEBUG("[SNK] AAC disabled");
            break;
        }
        if (p_peer->srcs[index].codec_type == codec_type)
        {
            if (p_src_index)
            {
                *p_src_index = index;
            }
            APPL_TRACE_DEBUG("bta_av_co_audio_peer_src_supports_codec: AAC" );
            if (0 ==  bta_av_aac_cfg_matches_cap((UINT8 *)p_peer->srcs[index].codec_caps,
                                                 (tA2D_AAC_CIE *)&bta_av_co_aac_sink_caps))
            {
                return TRUE;
            }
        }
    }
#endif

#if (defined(MTK_A2DP_SRC_APTX_CODEC) && (MTK_A2DP_SRC_APTX_CODEC == TRUE)) || (defined(MTK_A2DP_SRC_AAC_CODEC) && (MTK_A2DP_SRC_AAC_CODEC == TRUE))
        bta_av_co_cb.codec_cfg.id = BTIF_AV_CODEC_SBC;
        if (bta_av_co_cb.codec_cfg_setconfig.id == BTIF_AV_CODEC_NONE &&
                A2D_BldSbcInfo(A2D_MEDIA_TYPE_AUDIO, (tA2D_SBC_CIE *)&btif_av_sbc_default_config, bta_av_co_cb.codec_cfg.info) != A2D_SUCCESS)
        {
            APPL_TRACE_ERROR("bta_av_co_audio_peer_src_supports_codec A2D_BldSbcInfo failed");
        }
#endif

    /* Configure the codec type to look for */
    codec_type = bta_av_co_cb.codec_cfg.id;


    for (index = 0; index < p_peer->num_sup_srcs; index++)
    {
        if (p_peer->srcs[index].codec_type == codec_type)
        {
            switch (bta_av_co_cb.codec_cfg.id)
            {
            case BTIF_AV_CODEC_SBC:
                if (p_src_index) *p_src_index = index;
                if (0 ==  bta_av_sbc_cfg_matches_cap((UINT8 *)p_peer->srcs[index].codec_caps,
                                                     (tA2D_SBC_CIE *)&bta_av_co_sbc_sink_caps))
                {
                    return TRUE;
                }
                break;

            default:
                APPL_TRACE_ERROR("peer_src_supports_codec: unsupported codec id %d",
                                                            bta_av_co_cb.codec_cfg.id);
                return FALSE;
                break;
            }
        }
    }
    return FALSE;
}

/*******************************************************************************
 **
 ** Function         bta_av_co_audio_sink_supports_config
 **
 ** Description      Check if the media source supports a given configuration
 **
 ** Returns          TRUE if the media source supports this config, FALSE otherwise
 **
 *******************************************************************************/
static BOOLEAN bta_av_co_audio_sink_supports_config(UINT8 codec_type, const UINT8 *p_codec_cfg)
{
    FUNC_TRACE();

    switch (codec_type)
    {
    case BTA_AV_CODEC_SBC:
        if (bta_av_sbc_cfg_in_cap((UINT8 *)p_codec_cfg, (tA2D_SBC_CIE *)&bta_av_co_sbc_sink_caps))
        {
            return FALSE;
        }
        break;
#if defined(MTK_A2DP_SNK_AAC_CODEC) && (MTK_A2DP_SNK_AAC_CODEC == TRUE)
    case BTA_AV_CODEC_M24:  // AAC
        if (bta_av_AAC_cfg_in_cap((UINT8 *)p_codec_cfg, (tA2D_AAC_CIE *)&bta_av_co_aac_sink_caps))
        {
            return FALSE;
        }
        break;
#endif


    default:
        APPL_TRACE_ERROR("bta_av_co_audio_media_supports_config unsupported codec type %d", codec_type);
        return FALSE;
        break;
    }
    return TRUE;
}

/*******************************************************************************
 **
 ** Function         bta_av_co_audio_media_supports_config
 **
 ** Description      Check if the media sink supports a given configuration
 **
 ** Returns          TRUE if the media source supports this config, FALSE otherwise
 **
 *******************************************************************************/
static BOOLEAN bta_av_co_audio_media_supports_config(UINT8 codec_type, const UINT8 *p_codec_cfg)
{
    FUNC_TRACE();

    switch (codec_type)
    {
    case BTA_AV_CODEC_SBC:
        if (bta_av_sbc_cfg_in_cap((UINT8 *)p_codec_cfg, (tA2D_SBC_CIE *)&bta_av_co_sbc_caps))
        {
            return FALSE;
        }
        break;
#if defined(MTK_A2DP_SRC_APTX_CODEC) && (MTK_A2DP_SRC_APTX_CODEC == TRUE)
    case NON_A2DP_MEDIA_CT:
        if (bta_av_aptx_cfg_in_cap((UINT8 *)p_codec_cfg, (tA2D_APTX_CIE *)&bta_av_co_aptx_caps))
        {
            return FALSE;
        }
        break;
#endif
#if defined(MTK_A2DP_SRC_AAC_CODEC) && (MTK_A2DP_SRC_AAC_CODEC == TRUE)
    case BTA_AV_CODEC_M24:
        APPL_TRACE_DEBUG("bta_av_co_audio_media_supports_config : BTIF_AV_CODEC_AAC");
        break;
#endif

    default:
        APPL_TRACE_ERROR("bta_av_co_audio_media_supports_config unsupported codec type %d", codec_type);
        return FALSE;
        break;
    }
    return TRUE;
}

/*******************************************************************************
 **
 ** Function         bta_av_co_audio_codec_supported
 **
 ** Description      Check if all opened connections are compatible with a codec
 **                  configuration and content protection
 **
 ** Returns          TRUE if all opened devices support this codec, FALSE otherwise
 **
 *******************************************************************************/
BOOLEAN bta_av_co_audio_codec_supported(tBTIF_STATUS *p_status)
{
    UINT8 index;
    UINT8 snk_index;
    tBTA_AV_CO_PEER *p_peer;
    tBTA_AV_CO_SINK *p_sink;
    UINT8 codec_cfg[AVDT_CODEC_SIZE];
    UINT8 num_protect = 0;
#if defined(BTA_AV_CO_CP_SCMS_T) && (BTA_AV_CO_CP_SCMS_T == TRUE)
    BOOLEAN cp_active;
#endif

    FUNC_TRACE();

    APPL_TRACE_DEBUG("bta_av_co_audio_codec_supported");

    /* Check AV feeding is supported */
    *p_status = BTIF_ERROR_SRV_AV_FEEDING_NOT_SUPPORTED;

    for (index = 0; index < BTA_AV_CO_NUM_ELEMENTS(bta_av_co_cb.peers); index++)
    {
        p_peer = &bta_av_co_cb.peers[index];
        if (p_peer->opened)
        {
            if (bta_av_co_audio_peer_supports_codec(p_peer, &snk_index))
            {
                p_sink = &p_peer->snks[snk_index];

#if defined(MTK_COMMON) && (MTK_COMMON == TRUE)
#else
                /* Check that this sink is compatible with the CP */
                if (!bta_av_co_audio_sink_supports_cp(p_sink))
                {
                    APPL_TRACE_DEBUG("bta_av_co_audio_codec_supported sink %d of peer %d doesn't support cp",
                            snk_index, index);
                    *p_status = BTIF_ERROR_SRV_AV_CP_NOT_SUPPORTED;
                    return FALSE;
                }
#endif
                /* Build the codec configuration for this sink */
                if (bta_av_co_audio_codec_build_config(p_sink->codec_caps, codec_cfg))
                {
#if defined(BTA_AV_CO_CP_SCMS_T) && (BTA_AV_CO_CP_SCMS_T == TRUE)
                    /* Check if this sink supports SCMS */
                    cp_active = bta_av_co_audio_sink_has_scmst(p_sink);
#if defined(MTK_STACK_CONFIG_BL) && (MTK_STACK_CONFIG_BL == TRUE)
                    if ((cp_active == TRUE) && (interop_mtk_match_addr_name(INTEROP_MTK_A2DP_NOT_SET_SCMT, (const bt_bdaddr_t *)&p_peer->addr)))
                    {
                        cp_active = FALSE;
                        APPL_TRACE_DEBUG("SCMS_T black device, re-set cp_active to be FALSE");
                    }
#endif
#if (defined(MTK_A2DP_SRC_APTX_CODEC) && (MTK_A2DP_SRC_APTX_CODEC == TRUE)) || (defined(MTK_A2DP_SRC_AAC_CODEC) && (MTK_A2DP_SRC_AAC_CODEC == TRUE))
                    if(bta_av_co_cb.codec_cfg.id != BTIF_AV_CODEC_SBC)  // aptx & AAC should be excluded for SCMS-T
                    {
                        cp_active = FALSE;
                    }
#endif
#endif //defined(BTA_AV_CO_CP_SCMS_T) && (BTA_AV_CO_CP_SCMS_T == TRUE)
                    /* Check if this is a new configuration (new sink or new config) */
                    if ((p_sink != p_peer->p_snk) ||
                        (memcmp(codec_cfg, p_peer->codec_cfg, AVDT_CODEC_SIZE))
#if defined(BTA_AV_CO_CP_SCMS_T) && (BTA_AV_CO_CP_SCMS_T == TRUE)
                        || (p_peer->cp_active != cp_active)
#endif
                        )
                    {
                        /* Save the new configuration */
                        p_peer->p_snk = p_sink;
                        memcpy(p_peer->codec_cfg, codec_cfg, AVDT_CODEC_SIZE);
#if defined(BTA_AV_CO_CP_SCMS_T) && (BTA_AV_CO_CP_SCMS_T == TRUE)
                        p_peer->cp_active = cp_active;
                        if (p_peer->cp_active)
                        {
                            bta_av_co_cb.cp.active = TRUE;
                            num_protect = BTA_AV_CP_INFO_LEN;
                        }
                        else
                        {
                            bta_av_co_cb.cp.active = FALSE;
                        }
#endif
                        APPL_TRACE_DEBUG("bta_av_co_audio_codec_supported call BTA_AvReconfig(x%x)", BTA_AV_CO_AUDIO_INDX_TO_HNDL(index));
                        BTA_AvReconfig(BTA_AV_CO_AUDIO_INDX_TO_HNDL(index), TRUE, p_sink->sep_info_idx,
                                p_peer->codec_cfg, num_protect, (UINT8 *)bta_av_co_cp_scmst);
                    }
                }
            }
            else
            {
                APPL_TRACE_DEBUG("bta_av_co_audio_codec_supported index %d doesn't support codec", index);
                return FALSE;
            }
        }
    }

    *p_status = BTIF_SUCCESS;
    return TRUE;
}

/*******************************************************************************
 **
 ** Function         bta_av_co_audio_codec_reset
 **
 ** Description      Reset the current codec configuration
 **
 ** Returns          void
 **
 *******************************************************************************/
void bta_av_co_audio_codec_reset(void)
{
    mutex_global_lock();
    FUNC_TRACE();

    /* Reset the current configuration to SBC */
    bta_av_co_cb.codec_cfg.id = BTIF_AV_CODEC_SBC;

    if (A2D_BldSbcInfo(A2D_MEDIA_TYPE_AUDIO, (tA2D_SBC_CIE *)&btif_av_sbc_default_config, bta_av_co_cb.codec_cfg.info) != A2D_SUCCESS)
    {
        APPL_TRACE_ERROR("bta_av_co_audio_codec_reset A2D_BldSbcInfo failed");
    }

#if defined(MTK_A2DP_SRC_AAC_CODEC) && (MTK_A2DP_SRC_AAC_CODEC == TRUE)
    bta_av_co_cb.codec_cfg.id = BTIF_AV_CODEC_AAC;
    if(A2D_BldAACInfo(A2D_MEDIA_TYPE_AUDIO, (tA2D_AAC_CIE *)&btif_av_aac_default_config, bta_av_co_cb.codec_cfg.info) != A2D_SUCCESS)
    {
        APPL_TRACE_ERROR("bta_av_co_audio_codec_reset A2D_BldAACInfo failed");
    }
#endif
#if defined(MTK_A2DP_SRC_APTX_CODEC) && (MTK_A2DP_SRC_APTX_CODEC == TRUE)
    bta_av_co_cb.codec_cfg.id = NON_A2DP_MEDIA_CT;
    if(A2D_BldAptxInfo(A2D_MEDIA_TYPE_AUDIO, (tA2D_APTX_CIE *)&btif_av_aptx_default_config, bta_av_co_cb.codec_cfg.info) != A2D_SUCCESS)
    {
        APPL_TRACE_ERROR("bta_av_co_audio_codec_reset A2D_BldAptxInfo failed");
    }
#endif
    mutex_global_unlock();
}

/*******************************************************************************
 **
 ** Function         bta_av_co_audio_set_codec
 **
 ** Description      Set the current codec configuration from the feeding type.
 **                  This function is starting to modify the configuration, it
 **                  should be protected.
 **
 ** Returns          TRUE if successful, FALSE otherwise
 **
 *******************************************************************************/
BOOLEAN bta_av_co_audio_set_codec(const tBTIF_AV_MEDIA_FEEDINGS *p_feeding, tBTIF_STATUS *p_status)
{
    tA2D_SBC_CIE sbc_config;
    tBTIF_AV_CODEC_INFO new_cfg;
#if defined(MTK_A2DP_SRC_APTX_CODEC) && (MTK_A2DP_SRC_APTX_CODEC == TRUE)
    tA2D_APTX_CIE aptx_config;
#endif
#if defined(MTK_A2DP_SRC_AAC_CODEC) && (MTK_A2DP_SRC_AAC_CODEC == TRUE)
    tA2D_AAC_CIE aac_config;
#endif

    FUNC_TRACE();

    /* Check AV feeding is supported */
    *p_status = BTIF_ERROR_SRV_AV_FEEDING_NOT_SUPPORTED;

    APPL_TRACE_DEBUG("bta_av_co_audio_set_codec cid=%d", p_feeding->format);

    /* Supported codecs */
    switch (p_feeding->format)
    {
    case BTIF_AV_CODEC_PCM:
        new_cfg.id = BTIF_AV_CODEC_SBC;

        sbc_config = btif_av_sbc_default_config;
        if ((p_feeding->cfg.pcm.num_channel != 1) &&
            (p_feeding->cfg.pcm.num_channel != 2))
        {
            APPL_TRACE_ERROR("bta_av_co_audio_set_codec PCM channel number unsupported");
            return FALSE;
        }
        if ((p_feeding->cfg.pcm.bit_per_sample != 8) &&
            (p_feeding->cfg.pcm.bit_per_sample != 16))
        {
            APPL_TRACE_ERROR("bta_av_co_audio_set_codec PCM sample size unsupported");
            return FALSE;
        }
#if defined(MTK_A2DP_SRC_AAC_CODEC) && (MTK_A2DP_SRC_AAC_CODEC == TRUE)
        new_cfg.id = A2D_MEDIA_CT_M24;
        aac_config = btif_av_aac_default_config;
#endif
#if defined(MTK_A2DP_SRC_APTX_CODEC) && (MTK_A2DP_SRC_APTX_CODEC == TRUE)
        new_cfg.id = NON_A2DP_MEDIA_CT;
        aptx_config = btif_av_aptx_default_config;
#endif
        switch (p_feeding->cfg.pcm.sampling_freq)
        {
        case 8000:
        case 12000:
        case 16000:
        case 24000:
        case 32000:
        case 48000:
            sbc_config.samp_freq = A2D_SBC_IE_SAMP_FREQ_48;
#if defined(MTK_A2DP_SRC_APTX_CODEC) && (MTK_A2DP_SRC_APTX_CODEC == TRUE)
            aptx_config.sampleRate = CSR_APTX_SAMPLERATE_48000;
#endif
#if defined(MTK_A2DP_SRC_AAC_CODEC) && (MTK_A2DP_SRC_AAC_CODEC == TRUE)
            aac_config.samp_freq = AAC_SAMPLE_FREQ_48k;
#endif
            break;

        case 11025:
        case 22050:
        case 44100:
            sbc_config.samp_freq = A2D_SBC_IE_SAMP_FREQ_44;
#if defined(MTK_A2DP_SRC_APTX_CODEC) && (MTK_A2DP_SRC_APTX_CODEC == TRUE)
            aptx_config.sampleRate = CSR_APTX_SAMPLERATE_44100;
#endif
#if defined(MTK_A2DP_SRC_AAC_CODEC) && (MTK_A2DP_SRC_AAC_CODEC == TRUE)
            aac_config.samp_freq = AAC_SAMPLE_FREQ_44k;
#endif
            break;
        default:
            APPL_TRACE_ERROR("bta_av_co_audio_set_codec PCM sampling frequency unsupported");
            return FALSE;
            break;
        }
        /* Build the codec config */
        if (A2D_BldSbcInfo(A2D_MEDIA_TYPE_AUDIO, &sbc_config, new_cfg.info) != A2D_SUCCESS)
        {
            APPL_TRACE_ERROR("bta_av_co_audio_set_codec A2D_BldSbcInfo failed");
            return FALSE;
        }
#if defined(MTK_A2DP_SRC_AAC_CODEC) && (MTK_A2DP_SRC_AAC_CODEC == TRUE)
        if (A2D_BldAACInfo(A2D_MEDIA_TYPE_AUDIO, &aac_config, new_cfg.info) != A2D_SUCCESS)
        {
            APPL_TRACE_ERROR("bta_av_co_audio_set_codec A2D_BldAACInfo failed");
            return FALSE;
        }
#endif
#if defined(MTK_A2DP_SRC_APTX_CODEC) && (MTK_A2DP_SRC_APTX_CODEC == TRUE)
        if (A2D_BldAptxInfo(A2D_MEDIA_TYPE_AUDIO, &aptx_config, new_cfg.info) != A2D_SUCCESS)
        {
            APPL_TRACE_ERROR("bta_av_co_audio_set_codec A2D_BldAptxInfo failed");
            return FALSE;
        }
#endif

        break;


    default:
        APPL_TRACE_ERROR("bta_av_co_audio_set_codec Feeding format unsupported");
        return FALSE;
        break;
    }

#if defined(MTK_COMMON) && (MTK_COMMON == TRUE)
    APPL_TRACE_DEBUG("codec_cfg_setconfig.id=%d", bta_av_co_cb.codec_cfg_setconfig.id);
    /* It does not make sense to always reset current as default configuration.
        If current configure is set by remote, it is unnecessary to reset it. */
    if (bta_av_co_cb.codec_cfg_setconfig.id == BTIF_AV_CODEC_NONE)
        memcpy(&bta_av_co_cb.codec_cfg, &new_cfg, sizeof(tBTIF_AV_CODEC_INFO));
#else
    /* The new config was correctly built */
    bta_av_co_cb.codec_cfg = new_cfg;
#endif

    /* Check all devices support it */
    *p_status = BTIF_SUCCESS;
#if defined(MTK_A2DP_SRC_SINK_BOTH) && (MTK_A2DP_SRC_SINK_BOTH == TRUE)
    if (bta_av_co_cb.codec_cfg_setconfig.id == BTIF_AV_CODEC_NONE)
    {
        return bta_av_co_audio_codec_supported(p_status);
    }
    else
    {
        return TRUE;
    }
#else
    return bta_av_co_audio_codec_supported(p_status);
#endif
}
//#if (defined(MTK_A2DP_SRC_APTX_CODEC) && (MTK_A2DP_SRC_APTX_CODEC == TRUE)) || (defined(MTK_A2DP_SRC_AAC_CODEC) && (MTK_A2DP_SRC_AAC_CODEC == TRUE))
#if defined(MTK_LINUX_A2DP_PLUS) && (MTK_LINUX_A2DP_PLUS == TRUE)
UINT8 bta_av_get_current_codec(void)
{
    return bta_av_co_cb.codec_cfg.id;
}

UINT8* bta_av_get_current_codecInfo(void)
{
    return &bta_av_co_cb.codec_cfg.info[0];
}

BOOLEAN bta_av_co_audio_get_aac_aptx_config(UINT8 *p_config, UINT16 *p_minmtu, UINT8 type)
{
    BOOLEAN result = FALSE;
    tBTA_AV_CO_PEER *p_peer;
    APPL_TRACE_EVENT("bta_av_co_cb.codec_cfg.id : codec 0x%x", bta_av_co_cb.codec_cfg.id);
    *p_minmtu = 0xFFFF;
    mutex_global_lock();

    for (UINT8 index = 0; index < BTA_AV_CO_NUM_ELEMENTS(bta_av_co_cb.peers); index++)
    {
        p_peer = &bta_av_co_cb.peers[index];
        if (p_peer->opened)
        {
            if (p_peer->mtu < *p_minmtu)
            {
                APPL_TRACE_EVENT("bta_av_co_audio_get_aac_aptx_config, mtu = %d", p_peer->mtu);
                *p_minmtu = p_peer->mtu;
            }
        }
    }
#if defined(MTK_A2DP_SRC_APTX_CODEC) && (MTK_A2DP_SRC_APTX_CODEC == TRUE)
    if (type == NON_A2DP_MEDIA_CT)
    {
        tA2D_APTX_CIE *aptx_config = (tA2D_APTX_CIE *)p_config;
        if(A2D_ParsAptxInfo(aptx_config, bta_av_co_cb.codec_cfg.info, FALSE) == A2D_SUCCESS) {
            result = TRUE;
        }
        else
        {
            memcpy((tA2D_APTX_CIE *) p_config, &btif_av_aptx_default_config, sizeof(tA2D_APTX_CIE));
        }
    }
#endif
#if defined(MTK_A2DP_SRC_AAC_CODEC) && (MTK_A2DP_SRC_AAC_CODEC == TRUE)
    if (type == BTIF_AV_CODEC_AAC)
    {
        tA2D_AAC_CIE *aac_config = (tA2D_AAC_CIE *)p_config;
        if(A2D_ParsAACInfo(aac_config, bta_av_co_cb.codec_cfg.info, FALSE) == A2D_SUCCESS) {
            result = TRUE;
        }
        else
        {
            memcpy((tA2D_AAC_CIE *) p_config, &btif_av_aac_default_config, sizeof(tA2D_AAC_CIE));
        }
    }
#endif
    mutex_global_unlock();
    return result;
}
//#endif  //#if (MTK_A2DP_SRC_APTX_CODEC == TRUE) ||(MTK_A2DP_SRC_AAC_CODEC == TRUE)
#endif // #if defined(MTK_LINUX_A2DP_PLUS) && (MTK_LINUX_A2DP_PLUS == TRUE)


/*******************************************************************************
 **
 ** Function         bta_av_co_audio_get_sbc_config
 **
 ** Description      Retrieves the SBC codec configuration.  If the codec in use
 **                  is not SBC, return the default SBC codec configuration.
 **
 ** Returns          TRUE if codec is SBC, FALSE otherwise
 **
 *******************************************************************************/
BOOLEAN bta_av_co_audio_get_sbc_config(tA2D_SBC_CIE *p_sbc_config, UINT16 *p_minmtu)
{
    BOOLEAN result = FALSE;
    UINT8 index, jndex;
    tBTA_AV_CO_PEER *p_peer;
    tBTA_AV_CO_SINK *p_sink;

    APPL_TRACE_EVENT("bta_av_co_cb.codec_cfg.id : codec 0x%x", bta_av_co_cb.codec_cfg.id);

    /* Minimum MTU is by default very large */
    *p_minmtu = 0xFFFF;

    mutex_global_lock();
    if (bta_av_co_cb.codec_cfg.id == BTIF_AV_CODEC_SBC)
    {
        if (A2D_ParsSbcInfo(p_sbc_config, bta_av_co_cb.codec_cfg.info, FALSE) == A2D_SUCCESS)
        {
            for (index = 0; index < BTA_AV_CO_NUM_ELEMENTS(bta_av_co_cb.peers); index++)
            {
                p_peer = &bta_av_co_cb.peers[index];
                if (p_peer->opened)
                {
                    if (p_peer->mtu < *p_minmtu)
                    {
                        *p_minmtu = p_peer->mtu;
                    }
                    for (jndex = 0; jndex < p_peer->num_sup_snks; jndex++)
                    {
                        p_sink = &p_peer->snks[jndex];
                        if (p_sink->codec_type == A2D_MEDIA_CT_SBC)
                        {
                            /* Update the bitpool boundaries of the current config */
                            p_sbc_config->min_bitpool =
                               BTA_AV_CO_MAX(p_sink->codec_caps[BTA_AV_CO_SBC_MIN_BITPOOL_OFF],
                                             p_sbc_config->min_bitpool);
                            p_sbc_config->max_bitpool =
                               BTA_AV_CO_MIN(p_sink->codec_caps[BTA_AV_CO_SBC_MAX_BITPOOL_OFF],
                                             p_sbc_config->max_bitpool);
                            APPL_TRACE_EVENT("bta_av_co_audio_get_sbc_config : sink bitpool min %d, max %d",
                                 p_sbc_config->min_bitpool, p_sbc_config->max_bitpool);
                            break;
                        }
                    }
                }
            }
            result = TRUE;
        }
    }

    if (!result)
    {
        /* Not SBC, still return the default values */
        *p_sbc_config = btif_av_sbc_default_config;
    }
    mutex_global_unlock();

    return result;
}

/*******************************************************************************
 **
 ** Function         bta_av_co_audio_discard_config
 **
 ** Description      Discard the codec configuration of a connection
 **
 ** Returns          Nothing
 **
 *******************************************************************************/
void bta_av_co_audio_discard_config(tBTA_AV_HNDL hndl)
{
    tBTA_AV_CO_PEER *p_peer;

    FUNC_TRACE();

    /* Find the peer info */
    p_peer = bta_av_co_get_peer(hndl);
    if (p_peer == NULL)
    {
        APPL_TRACE_ERROR("bta_av_co_audio_discard_config could not find peer entry");
        return;
    }

    /* Reset the peer codec configuration */
    bta_av_co_audio_peer_reset_config(p_peer);
}

/*******************************************************************************
 **
 ** Function         bta_av_co_init
 **
 ** Description      Initialization
 **
 ** Returns          Nothing
 **
 *******************************************************************************/
void bta_av_co_init(void)
{
    FUNC_TRACE();

    /* Reset the control block */
    memset(&bta_av_co_cb, 0, sizeof(bta_av_co_cb));

    bta_av_co_cb.codec_cfg_setconfig.id = BTIF_AV_CODEC_NONE;

#if defined(BTA_AV_CO_CP_SCMS_T) && (BTA_AV_CO_CP_SCMS_T == TRUE)
    bta_av_co_cp_set_flag(BTA_AV_CP_SCMS_COPY_NEVER);
#else
    bta_av_co_cp_set_flag(BTA_AV_CP_SCMS_COPY_FREE);
#endif

    /* Reset the current config */
    bta_av_co_audio_codec_reset();
}


/*******************************************************************************
 **
 ** Function         bta_av_co_peer_cp_supported
 **
 ** Description      Checks if the peer supports CP
 **
 ** Returns          TRUE if the peer supports CP
 **
 *******************************************************************************/
BOOLEAN bta_av_co_peer_cp_supported(tBTA_AV_HNDL hndl)
{
    tBTA_AV_CO_PEER *p_peer;
    tBTA_AV_CO_SINK *p_sink;
    UINT8 index;

    FUNC_TRACE();

    /* Find the peer info */
    p_peer = bta_av_co_get_peer(hndl);
    if (p_peer == NULL)
    {
        APPL_TRACE_ERROR("bta_av_co_peer_cp_supported could not find peer entry");
        return FALSE;
    }

    for (index = 0; index < p_peer->num_sup_snks; index++)
    {
        p_sink = &p_peer->snks[index];
        if (p_sink->codec_type == A2D_MEDIA_CT_SBC)
        {
            return bta_av_co_audio_sink_has_scmst(p_sink);
        }
    }
    APPL_TRACE_ERROR("bta_av_co_peer_cp_supported did not find SBC sink");
    return FALSE;
}


/*******************************************************************************
 **
 ** Function         bta_av_co_get_remote_bitpool_pref
 **
 ** Description      Check if remote side did a setconfig within the limits
 **                  of our exported bitpool range. If set we will set the
 **                  remote preference.
 **
 ** Returns          TRUE if config set, FALSE otherwize
 **
 *******************************************************************************/

BOOLEAN bta_av_co_get_remote_bitpool_pref(UINT8 *min, UINT8 *max)
{
    /* check if remote peer did a set config */
    if (bta_av_co_cb.codec_cfg_setconfig.id == BTIF_AV_CODEC_NONE)
        return FALSE;

    *min = bta_av_co_cb.codec_cfg_setconfig.info[BTA_AV_CO_SBC_MIN_BITPOOL_OFF];
    *max = bta_av_co_cb.codec_cfg_setconfig.info[BTA_AV_CO_SBC_MAX_BITPOOL_OFF];

    return TRUE;
}

#if defined(MTK_PTS_AV_TEST) && (MTK_PTS_AV_TEST == TRUE)  //tt pts test
/*******************************************************************************
 **
 ** Function         bta_av_co_pts_force_reconfig
 **
 ** Description      To trigger a reconfigurated process. If av is in a streaming state, it will enter
 **                   suspend mode first.
 **
 ** Returns          n/a
 **
 *******************************************************************************/
void bta_av_co_pts_force_reconfig(void)
{
    UINT8 index = 0;
    UINT8 snk_index = 0;
    tBTA_AV_CO_PEER *p_peer;
    tBTA_AV_CO_SINK *p_src;
    //UINT8 codec_cfg[AVDT_CODEC_SIZE];
    UINT8 num_protect = 0;

    APPL_TRACE_DEBUG("[pts] run %s ",__FUNCTION__);

    for (index = 0; index < BTA_AV_CO_NUM_ELEMENTS(bta_av_co_cb.peers); index++)
    {
        p_peer = &bta_av_co_cb.peers[index];

        if (p_peer->opened)
        {
            p_src = &p_peer->srcs[snk_index];

            if(0)//for debug message only
            {
                APPL_TRACE_DEBUG("[pts] current selected src = %p",p_peer->p_src);
                APPL_TRACE_DEBUG("[pts] current selected sink = %p",p_peer->p_snk);
                APPL_TRACE_DEBUG("[pts] num_sup_srcs = %d",p_peer->num_sup_srcs);
                APPL_TRACE_DEBUG("[pts] num_sup_snks = %d",p_peer->num_sup_snks);
                APPL_TRACE_DEBUG("[pts] src->sep_info_idx = %d ,src->seid = %d",p_src->sep_info_idx,p_src->seid);
                UINT8 i;
                for(i=0;i<3;i++){
                    APPL_TRACE_DEBUG("[pts] peer srcs[%d] 0x%x %x %x ",i,p_peer->srcs[i].sep_info_idx,p_peer->srcs[i].seid,p_peer->srcs[i].codec_type);
                    APPL_TRACE_DEBUG("[pts] peer snks[%d] 0x%x %x %x ",i,p_peer->snks[i].sep_info_idx,p_peer->snks[i].seid,p_peer->snks[i].codec_type);
                }
                APPL_TRACE_DEBUG("[pts] bta_av_co_cb.codec_cfg = 0x%x %x %x %x",bta_av_co_cb.codec_cfg.info[0],bta_av_co_cb.codec_cfg.info[1],bta_av_co_cb.codec_cfg.info[2],bta_av_co_cb.codec_cfg.info[3]);
            }

            BTA_AvReconfig(BTA_AV_CO_AUDIO_INDX_TO_HNDL(index), TRUE, p_src->sep_info_idx,
                           bta_av_co_cb.codec_cfg.info, num_protect, (UINT8 *)bta_av_co_cp_scmst);
        }
    }
}
#endif //(end -- #ifdef MTK_PTS_AV_TEST )


