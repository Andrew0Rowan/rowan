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

/* FILE NAME:  bt_mw_a2dp_src.c
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
#include <stdio.h>
#include <unistd.h>

#include "c_mw_config.h"
#include "bt_mw_common.h"
#include "bt_mw_a2dp_common.h"
#include "bt_mw_a2dp_src.h"
#include "linuxbt_a2dp_src_if.h"
/* NAMING CONSTANT DECLARATIONS
 */
/* MACRO FUNCTION DECLARATIONS
 */
/* DATA TYPE DECLARATIONS
 */
/* GLOBAL VARIABLE DECLARATIONS
 */
/* LOCAL SUBPROGRAM DECLARATIONS
 */
#if ENABLE_A2DP_SRC && !ENABLE_A2DP_ADEV
static INT32 bt_mw_a2dp_src_get_start_delay(VOID);
#endif

/* STATIC VARIABLE DECLARATIONS
 */
#if ENABLE_A2DP_SRC && !ENABLE_A2DP_ADEV
static BT_A2DP_UPLOADER bt_mw_a2dp_src_uploader = {0};
#endif
/* EXPORTED SUBPROGRAM BODIES
 */


INT32 bt_mw_a2dp_src_register_uploader(BT_A2DP_UPLOADER *uploader)
{
#if !ENABLE_A2DP_SRC || ENABLE_A2DP_ADEV
    return BT_ERR_STATUS_UNSUPPORTED;
#else
    INT32 i4_ret = BT_SUCCESS;
    BT_MW_FUNC_ENTER(BT_DEBUG_A2DP, "uploader=%p", uploader);
    memset(&bt_mw_a2dp_src_uploader, 0x0, sizeof(BT_A2DP_UPLOADER));

    if (NULL == uploader)
    {
        BT_DBG_ERROR(BT_DEBUG_A2DP, "callback uploader is null!");
        return BT_ERR_STATUS_NULL_POINTER;
    }
    if (uploader->init)
    {
        bt_mw_a2dp_src_uploader.init = uploader->init;
    }
    else
    {
        BT_DBG_ERROR(BT_DEBUG_A2DP, "bt_upd_init_cb uploader is null!");
        i4_ret = BT_ERR_STATUS_NULL_POINTER;
    }

    if (uploader->start)
    {
        bt_mw_a2dp_src_uploader.start = uploader->start;
    }
    else
    {
        BT_DBG_WARNING(BT_DEBUG_A2DP, "bt_upd_start_cb uploader is null!");
        i4_ret = BT_ERR_STATUS_NULL_POINTER;
    }

    if (uploader->stop)
    {
        bt_mw_a2dp_src_uploader.stop = uploader->stop;
    }
    else
    {
        BT_DBG_WARNING(BT_DEBUG_A2DP, "bt_upd_stop_cb uploader is null!");
        i4_ret = BT_ERR_STATUS_NULL_POINTER;
    }

    if (uploader->deinit)
    {
        bt_mw_a2dp_src_uploader.deinit = uploader->deinit;
    }
    else
    {
        BT_DBG_WARNING(BT_DEBUG_A2DP, "bt_upd_deinit_cb uploader is null!");
        i4_ret = BT_ERR_STATUS_NULL_POINTER;
    }

    return i4_ret;
#endif
}


INT32 bt_mw_a2dp_src_send_stream_data(const CHAR *data, INT32 len)
{
#if !ENABLE_A2DP_SRC || ENABLE_A2DP_ADEV
    return BT_ERR_STATUS_UNSUPPORTED;
#else
    return linuxbt_send_stream_data(data, len);
#endif
}


INT32 bt_mw_a2dp_src_start_uploader(UINT32 freq, UINT8 channel)
{
#if !ENABLE_A2DP_SRC || ENABLE_A2DP_ADEV
    return BT_ERR_STATUS_UNSUPPORTED;
#else
    INT32 delay_ms = 0;
    BT_MW_FUNC_ENTER(BT_DEBUG_A2DP, "freq=%d, channel=%u", freq, channel);

    if (NULL != bt_mw_a2dp_src_uploader.init)
    {
        bt_mw_a2dp_src_uploader.init(freq, channel);
    }
    else
    {
        BT_DBG_WARNING(BT_DEBUG_A2DP, "bt_upd_init_cb func is null!");
    }
    delay_ms = bt_mw_a2dp_src_get_start_delay();
    if (NULL != bt_mw_a2dp_src_uploader.start)
    {
        bt_mw_a2dp_src_uploader.start(delay_ms);
    }
    else
    {
        BT_DBG_WARNING(BT_DEBUG_A2DP, "bt_upd_start_cb func is null!");
    }
    return BT_SUCCESS;
#endif
}




INT32 bt_mw_a2dp_src_stop_uploader(VOID)
{
    BT_MW_FUNC_ENTER(BT_DEBUG_A2DP, "");
#if !ENABLE_A2DP_SRC || ENABLE_A2DP_ADEV
    return BT_ERR_STATUS_UNSUPPORTED;
#else
    /*add A2DP SRC handle*/
    if (NULL != bt_mw_a2dp_src_uploader.stop)
    {
        bt_mw_a2dp_src_uploader.stop();
    }
    else
    {
        BT_DBG_WARNING(BT_DEBUG_A2DP, "bt_upd_stop_cb func is null!");
    }
    if (NULL != bt_mw_a2dp_src_uploader.deinit)
    {
        bt_mw_a2dp_src_uploader.deinit();
    }
    else
    {
        BT_DBG_WARNING(BT_DEBUG_A2DP, "bt_upd_deinit_cb func is null!");
    }

    return BT_SUCCESS;
#endif
}

INT32 bt_mw_a2dp_src_set_audio_log_lv(UINT8 log_level)
{
#if !ENABLE_A2DP_SRC || ENABLE_A2DP_ADEV
    return BT_ERR_STATUS_UNSUPPORTED;
#else
    return linuxbt_a2dp_set_audio_hw_log_lvl(log_level);
#endif
}


/* LOCAL SUBPROGRAM BODIES
 */

#if ENABLE_A2DP_SRC && !ENABLE_A2DP_ADEV

static INT32 bt_mw_a2dp_src_get_start_delay(VOID)
{
    /* SBH50 is no sound when 1st connection and need wait 3s to send AVDTP_START */
    /* In case
     * 1. 1st connection
     * 2. send data after connected(on phone, play music before connecting.
     */
    if (0 == strncmp(/*g_bt_target_dev_info.name*/"a", "SBH50", 6))
    {
        return 3000;
    }

    return 0;
}
#endif




