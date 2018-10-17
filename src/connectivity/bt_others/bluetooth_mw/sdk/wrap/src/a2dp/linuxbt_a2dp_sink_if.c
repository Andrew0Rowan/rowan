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
#if defined(MTK_LINUX_A2DP_PLUS) && (MTK_LINUX_A2DP_PLUS == TRUE)
#include "mtk_bt_av.h"
#else
#include "bt_av.h"
#endif
#include "c_mw_config.h"
#if defined(MTK_LINUX_AVRCP_PLUS) && (MTK_LINUX_AVRCP_PLUS == TRUE)
#include "mtk_bt_rc.h"
#else
#include "bt_rc.h"
#endif
#include "bt_audio_track.h"
#include "mtk_audio.h"

#include "linuxbt_gap_if.h"
#include "linuxbt_a2dp_sink_if.h"
#include "linuxbt_common.h"

#include "bt_mw_a2dp_common.h"
#include "linuxbt_common.h"

#include "bt_mw_message_queue.h"

/* NAMING CONSTANT DECLARATIONS
 */

#define LINUXBT_A2DP_SINK_SET_MSG_LEN(msg) do{  \
    msg.hdr.len = sizeof(tBT_MW_A2DP_MSG);      \
    }while(0)

/* MACRO FUNCTION DECLARATIONS
 */
/* DATA TYPE DECLARATIONS
 */
/* GLOBAL VARIABLE DECLARATIONS
 */
/* LOCAL SUBPROGRAM DECLARATIONS
 */

// Callback functions
static void linuxbt_a2dp_sink_connection_state_cb(btav_connection_state_t state,
                                                        bt_bdaddr_t*bd_addr);

static void linuxbt_a2dp_sink_audio_config_cb(bt_bdaddr_t *bd_addr,
                                                    uint32_t sample_rate,
                                                    uint8_t channel_count);
static void linuxbt_a2dp_sink_audio_state_cb(btav_audio_state_t state,
                                                bt_bdaddr_t *bd_addr);

/* STATIC VARIABLE DECLARATIONS
 */

static btav_interface_t *g_bt_a2dp_sink_interface = NULL;
static btav_callbacks_t g_bt_a2dp_sink_callbacks =
{
    sizeof(btav_callbacks_t),
    linuxbt_a2dp_sink_connection_state_cb,
    linuxbt_a2dp_sink_audio_state_cb,
    linuxbt_a2dp_sink_audio_config_cb,
};


/* EXPORTED SUBPROGRAM BODIES
 */
/* LOCAL SUBPROGRAM BODIES
 */


static void linuxbt_a2dp_sink_connection_state_cb(btav_connection_state_t state,
                                                        bt_bdaddr_t *bd_addr)
{
    tBTMW_MSG btmw_msg = {0};

    linuxbt_btaddr_htos(bd_addr, btmw_msg.data.a2dp_msg.addr);
    btmw_msg.data.a2dp_msg.role = BT_MW_A2DP_ROLE_SINK;

    BT_DBG_NORMAL(BT_DEBUG_A2DP, "sink state: %s(%d) ",
                  state==BTAV_CONNECTION_STATE_DISCONNECTED?"DISCONNECTED":
                  state==BTAV_CONNECTION_STATE_CONNECTING?"CONNECTING":
                  state==BTAV_CONNECTION_STATE_CONNECTED?"CONNECTED":
                  state==BTAV_CONNECTION_STATE_DISCONNECTING?"DISCONNECTING":"UNKNOWN",
                  state);

    BT_DBG_NORMAL(BT_DEBUG_A2DP,"addr: %s", btmw_msg.data.a2dp_msg.addr);

    if (state == BTAV_CONNECTION_STATE_DISCONNECTED)
    {
        btmw_msg.hdr.event = BTMW_A2DP_DISCONNECTED;
    }
    else if (state == BTAV_CONNECTION_STATE_CONNECTING)
    {
        btmw_msg.hdr.event = BTMW_A2DP_CONNECTING;
    }
    else if (state == BTAV_CONNECTION_STATE_CONNECTED)
    {
        btmw_msg.hdr.event = BTMW_A2DP_CONNECTED;
    }
    LINUXBT_A2DP_SINK_SET_MSG_LEN(btmw_msg);
    linuxbt_send_msg(&btmw_msg);
}

static void linuxbt_a2dp_sink_audio_config_cb(bt_bdaddr_t *bd_addr,
                                                    uint32_t sample_rate,
                                                    uint8_t channel_count)
{
    tBTMW_MSG btmw_msg = {0};

    BT_MW_FUNC_ENTER(BT_DEBUG_A2DP, "sample_rate: %d, channel_count: %d",
        sample_rate, channel_count);

    btmw_msg.hdr.event = BTMW_A2DP_STREAM_CONFIG;
    linuxbt_btaddr_htos(bd_addr, btmw_msg.data.a2dp_msg.addr);
    btmw_msg.data.a2dp_msg.role = BT_MW_A2DP_ROLE_SINK;
    BT_DBG_NORMAL(BT_DEBUG_A2DP,"addr: %s", btmw_msg.data.a2dp_msg.addr);

    btmw_msg.data.a2dp_msg.data.codec_config.sample_rate = sample_rate;
    btmw_msg.data.a2dp_msg.data.codec_config.channel_num = channel_count;
    LINUXBT_A2DP_SINK_SET_MSG_LEN(btmw_msg);

    linuxbt_send_msg(&btmw_msg);
}

static void linuxbt_a2dp_sink_audio_state_cb(btav_audio_state_t state,
                                                bt_bdaddr_t *bd_addr)
{
    tBTMW_MSG btmw_msg = {0};
    BT_DBG_NORMAL(BT_DEBUG_A2DP, "state: %s(%d) ",
                  state==BTAV_AUDIO_STATE_STARTED?"STARTED":
                  state==BTAV_AUDIO_STATE_STOPPED?"STOPPED":
                  state==BTAV_AUDIO_STATE_REMOTE_SUSPEND?"SUSPEND":"UNKNOWN",
                  state);

    linuxbt_btaddr_htos(bd_addr, btmw_msg.data.a2dp_msg.addr);
    btmw_msg.data.a2dp_msg.role = BT_MW_A2DP_ROLE_SINK;

    if (BTAV_AUDIO_STATE_STARTED == state)
    {
        btmw_msg.hdr.event = BTMW_A2DP_STREAM_START;
    }
    else if (BTAV_AUDIO_STATE_REMOTE_SUSPEND == state
        || BTAV_AUDIO_STATE_STOPPED == state)
    {
        btmw_msg.hdr.event = BTMW_A2DP_STREAM_SUSPEND;
    }
    LINUXBT_A2DP_SINK_SET_MSG_LEN(btmw_msg);

    linuxbt_send_msg(&btmw_msg);
}

INT32 linuxbt_a2dp_sink_connect(CHAR *pbt_addr)
{
    INT32 status = BT_ERR_STATUS_FAIL;
    bt_bdaddr_t bdaddr;

    BT_CHECK_POINTER(BT_DEBUG_A2DP, pbt_addr);

    if ((g_bt_a2dp_sink_interface == NULL))
    {
        BT_DBG_ERROR(BT_DEBUG_A2DP, "[A2DP] Failed to get A2DP SRC interface");
        return BT_ERR_STATUS_NOT_READY;
    }

    linuxbt_btaddr_stoh(pbt_addr, &bdaddr);
    BT_DBG_INFO(BT_DEBUG_A2DP, "A2DP connected to %s", pbt_addr);
    if (g_bt_a2dp_sink_interface && g_bt_a2dp_sink_interface->connect)
    {
        status = g_bt_a2dp_sink_interface->connect(&bdaddr);
    }
    else
    {
        BT_DBG_ERROR(BT_DEBUG_A2DP, "[A2DP] A2DP SINK is not enable");
    }

    return status;
}

INT32 linuxbt_a2dp_sink_disconnect(CHAR *pbt_addr)
{
    INT32 status = BT_SUCCESS;
    bt_bdaddr_t bdaddr;
    BT_CHECK_POINTER(BT_DEBUG_A2DP, pbt_addr);
    if ((g_bt_a2dp_sink_interface == NULL))
    {
        BT_DBG_ERROR(BT_DEBUG_A2DP, "[A2DP] Failed to get A2DP SRC interface");
        return BT_ERR_STATUS_NOT_READY;
    }

    BT_DBG_NOTICE(BT_DEBUG_A2DP, "[A2DP] linuxbt_a2dp_snk_disconnect_handler");

    linuxbt_btaddr_stoh(pbt_addr, &bdaddr);
    BT_DBG_NORMAL(BT_DEBUG_A2DP, "A2DP disconnected to %02X:%02X:%02X:%02X:%02X:%02X",
                  bdaddr.address[0], bdaddr.address[1], bdaddr.address[2],
                  bdaddr.address[3], bdaddr.address[4], bdaddr.address[5]);

    if (g_bt_a2dp_sink_interface && g_bt_a2dp_sink_interface->disconnect)
    {
        status = g_bt_a2dp_sink_interface->disconnect(&bdaddr);
    }
    else
    {
        BT_DBG_ERROR(BT_DEBUG_A2DP, "[A2DP] A2DP SINK is not enable");
    }

    return status;
}

INT32 linuxbt_a2dp_sink_init(VOID)
{
    INT32 ret = 0;
    BT_MW_FUNC_ENTER(BT_DEBUG_A2DP, "");

    // Get A2DP SINK interface
    g_bt_a2dp_sink_interface = (btav_interface_t *)linuxbt_gap_get_profile_interface(BT_PROFILE_ADVANCED_AUDIO_SINK_ID);
    if (g_bt_a2dp_sink_interface == NULL)
    {
        BT_DBG_ERROR(BT_DEBUG_A2DP, "[A2DP] Failed to get A2DP interface");
        return BT_ERR_STATUS_FAIL;
    }

    // Init A2DP SINK interface
    ret = g_bt_a2dp_sink_interface->init(&g_bt_a2dp_sink_callbacks);
    BT_DBG_NORMAL(BT_DEBUG_A2DP, "[A2DP] g_bt_a2dp_sink_callbacks:%p", &g_bt_a2dp_sink_callbacks);
    if (ret == BT_STATUS_SUCCESS)
    {
        BT_DBG_NORMAL(BT_DEBUG_A2DP, "[A2DP] success to init A2DP interface");
    }
    else if (ret == BT_STATUS_DONE)
    {
        BT_DBG_NORMAL(BT_DEBUG_A2DP, "[A2DP] already init A2DP interface");
    }
    else
    {
        BT_DBG_ERROR(BT_DEBUG_A2DP, "[A2DP] Failed to init A2DP interface");
    }

    return linuxbt_return_value_convert(ret);
}


VOID linuxbt_a2dp_sink_deinit(VOID)
{
    BT_MW_FUNC_ENTER(BT_DEBUG_A2DP, "");

    if (g_bt_a2dp_sink_interface != NULL)
    {
        g_bt_a2dp_sink_interface->cleanup();
    }
    g_bt_a2dp_sink_interface = NULL;
    FUNC_EXIT;
}


#if defined(BT_RPC_DBG_SERVER)
EXPORT_SYMBOL int dbg_a2dp_get_g_local_role(int array_index, int offset, char *name, char *data, int length)
{
    if (offset >= 1)
    {
        return 0;
    }

    sprintf(name, "g_local_role");
    return offset + 1;
}
#endif
