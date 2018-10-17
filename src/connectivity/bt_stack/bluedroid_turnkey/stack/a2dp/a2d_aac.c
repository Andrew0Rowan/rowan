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
#include "a2d_api.h"
#include "a2d_aac.h"
#include "bta_av_api.h"
#include <utils/Log.h>
#include "utl.h"
#include <math.h>
#include <stdio.h>

/******************************************************************************
**
** Function         A2D_Bld_AACInfo
**
******************************************************************************/
UINT8 A2D_BldAACInfo(UINT8 media_type, tA2D_AAC_CIE *p_ie, UINT8 *p_result)
{
    tA2D_STATUS status;
    UINT8 tmp = 0;

    APPL_TRACE_DEBUG("<AAC> A2D_BldAACInfo");

    if( p_ie == NULL || p_result == NULL )
    {
        /* if any unused bit is set */
        status = A2D_INVALID_PARAMS;
    }
    else
    {
        UINT8 status = 0;
        status = A2D_SUCCESS;
        *p_result++ = BTA_AV_CO_AAC_CODEC_LEN;
        *p_result++ = media_type;
        *p_result++ = A2D_MEDIA_CT_M24;

        /* Media Codec Specific Information Element */
        *p_result++ = p_ie->object_type;                        // Octet 0

        *p_result++ = (UINT8)((p_ie->samp_freq >> 4) & 0xFF);   // Octet 1
        tmp = (UINT8)(p_ie->samp_freq & 0x0F) << 4;
        tmp |= (p_ie->channels & 0x0F);
        *p_result++ = tmp;                                      // Octet 2
        printf("Octet 2, channel(b3,b2):%d\n", tmp);
        *p_result++ = (UINT8)(p_ie->bit_rate_high);             // Octet 3
        *p_result++ = (UINT8)(p_ie->bit_rate_mid);              // Octet 4
        *p_result++ = (UINT8)(p_ie->bit_rate_low);              // Octet 5
    }
    return status;
}

/******************************************************************************
**
** Function         A2D_ParsAACInfo
**
******************************************************************************/

tA2D_STATUS A2D_ParsAACInfo(tA2D_AAC_CIE *p_ie, UINT8 *p_info, BOOLEAN for_caps)
{
    tA2D_STATUS status = A2D_SUCCESS;
    UINT8 losc;
    UINT16 tmp_samplerate = 0;
    UINT32 tmp_bitrate = 0;

    if (p_ie == NULL || p_info == NULL)
    {
        return A2D_INVALID_PARAMS;
    }
    APPL_TRACE_DEBUG("<AAC> A2D_ParsAACInfo");

    losc = *p_info;
    p_info += 2;

    /* If the function is called for the wrong Media Type or Media Codec Type */
    if (losc != BTA_AV_CO_AAC_CODEC_LEN || *p_info != A2D_MEDIA_CT_M24)
        return A2D_WRONG_CODEC;

    p_info++; //octet0
    p_ie->object_type = *p_info;        // object_type
    p_info++; //octet1, contain 8K~ 44K
    tmp_samplerate = *p_info;
    p_info++; //octet2, contain 48K~ 96K
    p_ie->samp_freq = ((tmp_samplerate << 4) | ((UINT16)(((*p_info) & 0xF0) >> 4))); //samp_freq
    p_ie->channels = (*p_info) & AAC_CHANNEL_MSK;           // channels
    p_info++; //octet3
    p_ie->bit_rate_high = *p_info;    // VBR + bitrate_high
    p_info++; //octet4
    p_ie->bit_rate_mid = *p_info;     // bitrate_mid
    p_info++; //octet5
    p_ie->bit_rate_low = *p_info;     // bitrate_low

    if (for_caps != FALSE)
    {
        return status;
    }
    /*
    if (A2D_BitsSet(p_ie->object_type) != A2D_SET_ZERO_BIT)
        status = A2D_AAC_BAD_OBJECT_TYPE;
    */
    if (A2D_BitsSet(p_ie->samp_freq) != A2D_SET_ONE_BIT)
    {
        status = A2D_BAD_SAMP_FREQ;
    }
    if (A2D_BitsSet(p_ie->channels) != A2D_SET_ONE_BIT)
    {
        status = A2D_BAD_CH_MODE;
    }
    return status;
}

#if defined(MTK_A2DP_SNK_AAC_CODEC) && (MTK_A2DP_SNK_AAC_CODEC == TRUE)
/*******************************************************************************
**
** Function         bta_av_aac_cfg_matches_cap
**
** Description      This function checks whether an AAC codec configuration
**                  is allowable for the given codec capabilities.
**
** when called      This function will be called when local(A2DP SNK) initiate the connection to the peer(A2DP SRC)
**
** Returns          0 if ok, nonzero if error.
**
*******************************************************************************/
UINT8 bta_av_aac_cfg_matches_cap(UINT8 *p_cfg, tA2D_AAC_CIE *p_cap)
{
    UINT8           status = 0;
    tA2D_AAC_CIE    cfg_cie;
    APPL_TRACE_DEBUG("<AAC> bta_av_aac_cfg_matches_cap");
    /* parse configuration */
    if ((status = A2D_ParsAACInfo(&cfg_cie, p_cfg, FALSE)) != 0)
    {
        return status;
    }

    APPL_TRACE_DEBUG(" object_type peer: 0%x, capability  0%x", cfg_cie.object_type, p_cap->object_type);
    APPL_TRACE_DEBUG(" samp_freq peer: 0%x, capability  0%x", cfg_cie.samp_freq, p_cap->samp_freq);
    APPL_TRACE_DEBUG(" p_cap peer: 0%x, capability  0%x", cfg_cie.channels, p_cap->channels);

    if ((cfg_cie.object_type & p_cap->object_type) == 0)
    {
        status = A2D_AAC_BAD_PARAMETER;
    }
    if ((cfg_cie.samp_freq & p_cap->samp_freq) == 0)
    {
        status = A2D_AAC_BAD_PARAMETER;
    }
    if ((cfg_cie.channels & p_cap->channels) == 0)
    {
        status = A2D_AAC_BAD_PARAMETER;
    }
    return status;
}

/*******************************************************************************
**
** Function         bta_av_AAC_cfg_in_cap
**
** Description      This function checks whether an AAC codec configuration
**                  is allowable for the given codec capabilities.
**
** Returns          0 if ok, nonzero if error.
**
*******************************************************************************/
UINT8 bta_av_AAC_cfg_in_cap(UINT8 *p_cfg, tA2D_AAC_CIE *p_cap)
{
    UINT8           status = 0;
    tA2D_AAC_CIE    cfg_cie;

    /* parse configuration */
    if ((status = A2D_ParsAACInfo(&cfg_cie, p_cfg, FALSE)) != 0)
    {
        return status;
    }
    if ((cfg_cie.object_type & p_cap->object_type) == 0)
    {
        status = A2D_AAC_BAD_PARAMETER;
    }
    if ((cfg_cie.samp_freq & p_cap->samp_freq) == 0)
    {
        status = A2D_AAC_BAD_PARAMETER;
    }
    if ((cfg_cie.channels & p_cap->channels) == 0)
    {
        status = A2D_AAC_BAD_PARAMETER;
    }
    return status;
}
#endif

/*******************************************************************************
**
** Function         AAC_Encoder_Init
**
** Description      This function initialises the AAC encoder
**
** Returns          HANDLE_AACENCODER
**
*******************************************************************************/
#if defined(MTK_A2DP_SRC_AAC_CODEC) && (MTK_A2DP_SRC_AAC_CODEC == TRUE)
HANDLE_AACENCODER AAC_Encoder_Init(AAC_ENC_PARAMS params)
{
    //1. Open AAC Encoder
    if (AACENC_OK != aacEncOpen(&params.aacEncoder, 0, 2))
    {
        APPL_TRACE_ERROR("aacEncOpen failed!");
    }

    //2. Set AAC Profile
    if (AACENC_OK != aacEncoder_SetParam(params.aacEncoder, AACENC_AOT, AOT_AAC_LC))
    {
       APPL_TRACE_ERROR("Set AAC Profile Failed!");
    }

    //3. Set AAC Sample Rate
    if (AACENC_OK != aacEncoder_SetParam(params.aacEncoder, AACENC_SAMPLERATE, params.u16SamplingFreq))
    {
       APPL_TRACE_ERROR("Set AAC Sample Rate Failed");
    }

    //4. Set AAC Bitrate
    if (AACENC_OK != aacEncoder_SetParam(params.aacEncoder, AACENC_BITRATE, params.u32BitRate))
    {
      APPL_TRACE_ERROR("Set AAC Bitrate Failed");
    }

    //5. Set AAC Channel Mode
    if (AACENC_OK != aacEncoder_SetParam(params.aacEncoder, AACENC_CHANNELMODE, params.u16ChannelMode))
    {
        APPL_TRACE_ERROR("Set AAC Channel Mode Failed");
    }

    //6. Set AAC Bitstream format
    if (AACENC_OK != aacEncoder_SetParam(params.aacEncoder, AACENC_TRANSMUX, TT_MP4_LATM_MCP0))
    {
       APPL_TRACE_ERROR("Set AAC Sample Rate Failed");
    }

    //7. Init AAC encoder parameter
    if (AACENC_OK != aacEncEncode(params.aacEncoder, NULL, NULL, NULL, NULL) )
    {
       APPL_TRACE_ERROR("accEncEncode init Failed");
    }
    return params.aacEncoder;
}




/*******************************************************************************
**
** Function         AAC_Encoder_Deinit
**
** Description      This function de-initialises the AAC encoder
**
** Returns          void.
**
*******************************************************************************/
void AAC_Encoder_Deinit(AAC_ENC_PARAMS params)
{
    if (AACENC_INVALID_HANDLE == (UINT32) params.aacEncoder)
    {
       APPL_TRACE_ERROR("Invalid aacEncoder handler");
        return;
    }
    else if (AACENC_OK != aacEncClose(&params.aacEncoder))
    {
       APPL_TRACE_ERROR("Close AAC Encoder Error");
    }
}
#endif

