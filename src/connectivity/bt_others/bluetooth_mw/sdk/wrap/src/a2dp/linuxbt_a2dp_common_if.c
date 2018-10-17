/********************************************************************************************
 *     LEGAL DISCLAIMER
 *
 *     (Header of MediaTek Software/Firmware Release or Documentation)
 *
 *     BY OPENING OR USING THIS FILE, BUYER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 *     THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE") RECEIVED
 *     FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO BUYER ON AN "AS-IS" BASIS
 *     ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES, EXPRESS OR IMPLIED,
 *     INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR
 *     A PARTICULAR PURPOSE OR NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY
 *     WHATSOEVER WITH RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 *     INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND BUYER AGREES TO LOOK
 *     ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. MEDIATEK SHALL ALSO
 *     NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE RELEASES MADE TO BUYER'S SPECIFICATION
 *     OR TO CONFORM TO A PARTICULAR STANDARD OR OPEN FORUM.
 *
 *     BUYER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND CUMULATIVE LIABILITY WITH
 *     RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION,
 *     TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE
 *     FEES OR SERVICE CHARGE PAID BY BUYER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 *     THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE WITH THE LAWS
 *     OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF LAWS PRINCIPLES.
 ************************************************************************************************/
/* FILE NAME:  linuxbt_a2dp_sink_if.c
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
/* INCLUDE FILE DECLARATIONS
 */

#include <string.h>
#include <sys/time.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <sys/un.h>
#include <errno.h>
#include <time.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <pthread.h>

#include "bluetooth.h"
#include "bt_mw_common.h"
#include "linuxbt_common.h"
#include "linuxbt_gap_if.h"
#include "bt_mw_a2dp_common.h"
#include "linuxbt_a2dp_common_if.h"

#include "mtk_bluetooth.h"
#if defined(MTK_LINUX_A2DP_PLUS) && (MTK_LINUX_A2DP_PLUS == TRUE)
#include "mtk_bt_av.h"
#else
#include "bt_av.h"
#endif
#include "c_mw_config.h"

#include "bt_mw_message_queue.h"

#define A2D_MEDIA_CT_SBC        0x00    /* SBC media codec type */
#define A2D_MEDIA_CT_M12        0x01    /* MPEG-1, 2 Audio media codec type */
#define A2D_MEDIA_CT_M24        0x02    /* MPEG-2, 4 AAC media codec type */


#define LINUXBT_A2DP_COMMON_SET_MSG_LEN(msg) do{   \
    msg.hdr.len = sizeof(tBT_MW_A2DP_MSG);      \
    }while(0)

static void linuxbt_a2dp_codec_config_cb(bt_bdaddr_t *bd_addr, uint8_t codec_type, uint8_t *codec_config);

static btav_ext_interface_t *g_bt_a2dp_ext_interface = NULL;  // for both A2DP SRC & SNK
static btav_ext_callbacks_t g_bt_a2dp_ext_callbacks =
{
    sizeof(btav_ext_callbacks_t),
    linuxbt_a2dp_codec_config_cb,
};

static INT32 linuxbt_a2dp_convert_aac_codec(tA2D_AAC_CIE *tmp_aac_config, BT_A2DP_AAC_CONF *aac_config)
{
    UINT32 tmp_bitrate = 0;

    if ((NULL == tmp_aac_config) || (NULL == aac_config))
    {
        BT_DBG_ERROR(BT_DEBUG_A2DP, "aac config NULL");
        return BT_ERR_STATUS_NOT_READY;
    }

    if (AAC_MPEG2_LC & tmp_aac_config->object_type)
    {
        aac_config->object_type = BT_A2DP_AAC_OBJ_TYPE_MPEG2_LC;
    }
    else if (AAC_MPEG4_LC & tmp_aac_config->object_type)
    {
        aac_config->object_type = BT_A2DP_AAC_OBJ_TYPE_MPEG4_LC;
    }
    else if (AAC_MPEG4_LTP & tmp_aac_config->object_type)
    {
        aac_config->object_type = BT_A2DP_AAC_OBJ_TYPE_MPEG4_LTP;
    }
    else
    {
        aac_config->object_type = BT_A2DP_AAC_OBJ_TYPE_MPEG4_SCA;
    }

    if (tmp_aac_config->samp_freq == AAC_SAMPLE_FREQ_48k)
    {
        aac_config->samp_freq = 48000;
    }
    else
    {
        aac_config->samp_freq = 44100;
    }

    if (tmp_aac_config->channels & AAC_CHANNEL_1)
    {
        aac_config->channels = 1;
    }
    else
    {
        aac_config->channels = 2;
    }

    aac_config->vbr = (tmp_aac_config->bit_rate_high & 0x80) ? TRUE : FALSE;
    tmp_bitrate  = (tmp_aac_config->bit_rate_high & ~0x80) << 16;
    tmp_bitrate +=  tmp_aac_config->bit_rate_mid  << 8;
    tmp_bitrate +=  tmp_aac_config->bit_rate_low;
    aac_config->bitrate = tmp_bitrate;

    return BT_SUCCESS;
}

static INT32 linuxbt_a2dp_convert_sbc_codec(tA2D_SBC_CIE *tmp_sbc_config, BT_A2DP_SBC_CONF *sbc_config)
{
    if ((NULL == tmp_sbc_config) || (NULL == sbc_config))
    {
        BT_DBG_ERROR(BT_DEBUG_A2DP, " sbc config NULL");
        return BT_ERR_STATUS_NOT_READY;
    }

    if (tmp_sbc_config->samp_freq & A2D_SBC_IE_SAMP_FREQ_44)
    {
        sbc_config->samp_freq = 44100;
    }
    else
    {
        sbc_config->samp_freq = 48000;
    }

    if (tmp_sbc_config->num_subbands & A2D_SBC_IE_SUBBAND_4)
    {
        sbc_config->num_subbands = 4;
    }
    else
    {
        sbc_config->num_subbands = 8;
    }

    if (tmp_sbc_config->ch_mode & A2D_SBC_IE_CH_MD_MONO)
    {
        sbc_config->ch_mode = BT_A2DP_CHANNEL_MODE_MONO;
    }
    else if (tmp_sbc_config->ch_mode & A2D_SBC_IE_CH_MD_DUAL)
    {
        sbc_config->ch_mode = BT_A2DP_CHANNEL_MODE_DUAL;
    }
    else if (tmp_sbc_config->ch_mode & A2D_SBC_IE_CH_MD_STEREO)
    {
        sbc_config->ch_mode = BT_A2DP_CHANNEL_MODE_STEREO;
    }
    else
    {
        sbc_config->ch_mode = BT_A2DP_CHANNEL_MODE_JOINT;
    }

    if (tmp_sbc_config->block_len & A2D_SBC_IE_BLOCKS_4)
    {
        sbc_config->block_len = 4;
    }
    else if (tmp_sbc_config->block_len & A2D_SBC_IE_BLOCKS_8)
    {
        sbc_config->block_len = 8;
    }
    else if (tmp_sbc_config->block_len & A2D_SBC_IE_BLOCKS_12)
    {
        sbc_config->block_len = 12;
    }
    else
    {
        sbc_config->block_len = 16;
    }

    if (tmp_sbc_config->alloc_mthd & A2D_SBC_IE_ALLOC_MD_S)
    {
        sbc_config->alloc_mthd = 0;
    }
    else
    {
        sbc_config->alloc_mthd = 1;
    }

    sbc_config->min_bitpool =   tmp_sbc_config->min_bitpool;
    sbc_config->max_bitpool =   tmp_sbc_config->max_bitpool;

    return BT_SUCCESS;
}

static void linuxbt_a2dp_codec_config_cb(bt_bdaddr_t *bd_addr, uint8_t codec_type, uint8_t *codec_config)
{
    tBTMW_MSG btmw_msg = {0};

    linuxbt_btaddr_htos(bd_addr, btmw_msg.data.a2dp_msg.addr);
    BT_DBG_NORMAL(BT_DEBUG_A2DP, "addr: %s, codectype[%d]", btmw_msg.data.a2dp_msg.addr, codec_type);

    switch (codec_type)
    {
        case A2D_MEDIA_CT_M24:
            BT_DBG_NORMAL(BT_DEBUG_A2DP, "codec is AAC");
            linuxbt_a2dp_convert_aac_codec((tA2D_AAC_CIE *)codec_config,
                &btmw_msg.data.a2dp_msg.data.config.codec_conf.aac_conf);
            btmw_msg.data.a2dp_msg.data.config.codec_type = BT_A2DP_CODEC_TYPE_AAC;
            break;
        case A2D_MEDIA_CT_SBC:
            BT_DBG_NORMAL(BT_DEBUG_A2DP, "codec is SBC");
            linuxbt_a2dp_convert_sbc_codec((tA2D_SBC_CIE *)codec_config,
                &btmw_msg.data.a2dp_msg.data.config.codec_conf.sbc_conf);

            btmw_msg.data.a2dp_msg.data.config.codec_type = BT_A2DP_CODEC_TYPE_SBC;
            break;
        default:
            BT_DBG_ERROR(BT_DEBUG_A2DP, "unknown codec type [0x%x]", codec_type);
            return;
    }

    btmw_msg.hdr.event = BTMW_A2DP_CODEC_CONFIG;
    LINUXBT_A2DP_COMMON_SET_MSG_LEN(btmw_msg);
    linuxbt_send_msg(&btmw_msg);
}

INT32 linuxbt_a2dp_ext_init(VOID)
{
    INT32 ret = 0;

    g_bt_a2dp_ext_interface = (btav_ext_interface_t *) linuxbt_gap_get_profile_interface(BT_PROFILE_ADVANCED_AUDIO_EXT_ID);
    if (NULL == g_bt_a2dp_ext_interface)
    {
        BT_DBG_ERROR(BT_DEBUG_A2DP, "Failed to get A2DP extended interface");
        return -1;
    }
    else
    {
        BT_DBG_NORMAL(BT_DEBUG_A2DP, "get A2DP extended interface successfully");
    }
    // Init A2DP SRC ext interface
    ret = g_bt_a2dp_ext_interface->init(&g_bt_a2dp_ext_callbacks);
    if (ret == BT_STATUS_SUCCESS)
    {
        BT_DBG_NORMAL(BT_DEBUG_A2DP, "init A2DP extended interface successfully");
    }
    else
    {
        BT_DBG_NORMAL(BT_DEBUG_A2DP, "Fail to init A2DP extended interface");
    }

    return linuxbt_return_value_convert(ret);
}

INT32 linuxbt_a2dp_change_thread_priority_handler(INT32 priority)
{
    BT_MW_FUNC_ENTER(BT_DEBUG_A2DP, "priority[%ld]", priority);

    if (g_bt_a2dp_ext_interface != NULL)
    {
        g_bt_a2dp_ext_interface->change_thread_priority(priority);
    }
    else
    {
        BT_DBG_ERROR(BT_DEBUG_A2DP, "[A2DP] Failed to get A2DP ext interface");
    }

    return BT_SUCCESS;
}

INT32 linuxbt_a2dp_codec_enable_handler(BT_A2DP_CODEC_TYPE codec_type, BOOL enable)
{
    BT_MW_FUNC_ENTER(BT_DEBUG_A2DP, "codec_type[%ld], enable[%d]", codec_type, enable);
    uint8_t codec_type_tmp;

    if (codec_type == BT_A2DP_CODEC_TYPE_AAC)
    {
        codec_type_tmp = A2D_MEDIA_CT_M24;
    }
    else
    {
        return BT_ERR_STATUS_FAIL;
    }

    if (g_bt_a2dp_ext_interface != NULL)
    {
        g_bt_a2dp_ext_interface->codec_enable(codec_type_tmp, enable);
    }
    else
    {
        BT_DBG_ERROR(BT_DEBUG_A2DP, "[A2DP] Failed to get A2DP ext interface");
    }

    return BT_SUCCESS;
}

