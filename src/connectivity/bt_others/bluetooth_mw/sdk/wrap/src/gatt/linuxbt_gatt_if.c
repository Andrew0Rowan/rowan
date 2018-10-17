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


/* FILE NAME:  linuxbt_gatt_if.c
 * AUTHOR: Xuemei Yang
 * PURPOSE:
 *      It provides GATT common operation interface to MW.
 * NOTES:
 */


/*-----------------------------------------------------------------------------
                    include files
 ----------------------------------------------------------------------------*/
#include <stddef.h>
#include "bluetooth.h"
#include "u_bt_mw_common.h"
#include "bt_mw_common.h"
#include "bt_mw_log.h"
#include "linuxbt_gap_if.h"
#include "linuxbt_gatt_if.h"
#include "linuxbt_gattc_if.h"
#include "linuxbt_gatts_if.h"
#include "linuxbt_common.h"
#include "bt_gatt.h"
#if defined(MTK_LINUX_GATT) && (MTK_LINUX_GATT == TRUE)
#include "mtk_bt_gatt.h"
#include "mtk_bluetooth.h"
#endif

#if defined(MTK_LINUX_GATT) && (MTK_LINUX_GATT == TRUE)
extern const btgatt_ex_client_callbacks_t linuxbt_gattc_ex_callbacks;
static btgatt_ex_interface_t *linuxbt_gatt_ex_interface = NULL;
static btgatt_ex_callbacks_t linuxbt_gatt_ex_callbacks =
{
    sizeof(btgatt_ex_callbacks_t),
    &linuxbt_gattc_ex_callbacks,
};
#endif

extern const btgatt_client_callbacks_t linuxbt_gattc_callbacks;
extern const btgatt_server_callbacks_t linuxbt_gatts_callbacks;
static btgatt_interface_t *linuxbt_gatt_interface = NULL;
static btgatt_callbacks_t linuxbt_gatt_callbacks =
{
    sizeof(btgatt_callbacks_t),
    &linuxbt_gattc_callbacks,
    &linuxbt_gatts_callbacks,
};

#if defined(MTK_LINUX_GATT) && (MTK_LINUX_GATT == TRUE)
static int linuxbt_gatt_ex_init(void)
{
    bt_status_t ret = BT_STATUS_SUCCESS;
    linuxbt_gatt_ex_interface = (btgatt_ex_interface_t *) linuxbt_gap_get_profile_interface(BT_PROFILE_GATT_EX_ID);
    if (NULL == linuxbt_gatt_ex_interface)
    {
        BT_DBG_ERROR(BT_DEBUG_GATT,"[GATT] Failed to get GATT extened interface");
        return BT_ERR_STATUS_PARM_INVALID;
    }

    ret = linuxbt_gatt_ex_interface->init(&linuxbt_gatt_ex_callbacks);
    if (BT_STATUS_SUCCESS == ret)
    {
        BT_DBG_NOTICE(BT_DEBUG_GATT, "[GATT] success to init GATT extended interface");
    }
    else if (BT_STATUS_DONE == ret)
    {
        BT_DBG_NOTICE(BT_DEBUG_GATT, "[GATT] already init GATT extended interface");
    }

    if (BT_SUCCESS != linuxbt_gattc_ex_init(linuxbt_gatt_ex_interface->ex_client))
    {
        BT_DBG_ERROR(BT_DEBUG_GATT, "[GATT] Failed to init GATT client extented interface");
        return BT_ERR_STATUS_FAIL;
    }
    return linuxbt_return_value_convert(ret);
}
#endif

int linuxbt_gatt_init(void)
{
    bt_status_t ret = BT_STATUS_SUCCESS;
    BT_DBG_NORMAL(BT_DEBUG_GATT, "[GATT] linuxbt_gatt_init");

    // Get GATT interface
    linuxbt_gatt_interface = (btgatt_interface_t *) linuxbt_gap_get_profile_interface(BT_PROFILE_GATT_ID);
    if (NULL == linuxbt_gatt_interface)
    {
        BT_DBG_ERROR(BT_DEBUG_GATT,"[GATT] Failed to get GATT interface");
        return BT_ERR_STATUS_PARM_INVALID;
    }

    // Init GATT interface
    ret = linuxbt_gatt_interface->init(&linuxbt_gatt_callbacks);
    if (BT_STATUS_SUCCESS == ret)
    {
        BT_DBG_NOTICE(BT_DEBUG_GATT, "[GATT] success to init GATT interface");
    }
    else if (BT_STATUS_DONE == ret)
    {
        BT_DBG_NOTICE(BT_DEBUG_GATT, "[GATT] already init GATT interface");
    }

    if (BT_SUCCESS != linuxbt_gattc_init(linuxbt_gatt_interface->client))
    {
        BT_DBG_ERROR(BT_DEBUG_GATT, "[GATT] Failed to init GATT client interface");
        return BT_ERR_STATUS_FAIL;
    }

    if (BT_SUCCESS != linuxbt_gatts_init(linuxbt_gatt_interface->server))
    {
        BT_DBG_ERROR(BT_DEBUG_GATT, "[GATT] Failed to init GATT server interface");
        return BT_ERR_STATUS_FAIL;
    }

#if defined(MTK_LINUX_GATT) && (MTK_LINUX_GATT == TRUE)
    return linuxbt_gatt_ex_init();
#else
    return linuxbt_return_value_convert(ret);
#endif
}

int linuxbt_gatt_deinit(void)
{
    BT_ERR_STATUS_T ret = BT_SUCCESS;
    BT_DBG_NORMAL(BT_DEBUG_GATT, "[GATT] linuxbt_gatt_deinit");
    // Deinit GATT interface
    if (NULL != linuxbt_gatt_interface)
    {
        linuxbt_gatt_interface->cleanup();
        if (BT_SUCCESS != linuxbt_gattc_deinit())
        {
            BT_DBG_ERROR(BT_DEBUG_GATT, "[GATT] Failed to deinit GATT client");
            return BT_ERR_STATUS_FAIL;
        }
        if (BT_SUCCESS != linuxbt_gatts_deinit())
        {
            BT_DBG_ERROR(BT_DEBUG_GATT, "[GATT] Failed to deinit GATT server");
            return BT_ERR_STATUS_FAIL;
        }
    }
    return ret;
}

