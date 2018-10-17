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

/************************************************************************************
 *
 *  Filename:      btif_b3ds.c
 *
 *  Description:   Bluetooth 3d Synchronization Interface
 *
 *
 ***********************************************************************************/
#include "bt_target.h"

#if defined(MTK_B3DS_SUPPORT) && (MTK_B3DS_SUPPORT == TRUE)

#include <hardware/bluetooth.h>
#include <string.h>
#include "btif_b3ds.h"

#define LOG_TAG "BTIF_B3DS"
#include "btif_common.h"
#include "btif_util.h"
#include "btm_api.h"

#include "bta_api.h"
#include "bta_b3ds_api.h"
#include "btcore/include/bdaddr.h"


#define BTB3DS_ROLE_NONE    0
#define BTB3DS_ROLE_3DD     1
#define BTB3DS_ROLE_3DG     2

#define BTB3DS_LOCAL_ROLE   BTB3DS_ROLE_3DD

typedef struct
{
    int enabled;
    UINT8 enable_legacy;
} btb3ds_cb_t;

btb3ds_cb_t btb3ds_cb;
static int jni_initialized = FALSE, stack_initialized = FALSE;

// clayton: to support broadcom solution
static bt3d_mode_t bt3d_mode = BT3D_MODE_MASTER; // default on

static bt_status_t btb3ds_jni_init_with_legacy_setting(const btb3ds_callbacks_t *callbacks, UINT8 enable_legacy_reference_protocol);
static bt_status_t btb3ds_jni_init_without_legacy_setting(const btb3ds_callbacks_t *callbacks);
static void btb3ds_jni_cleanup(void);
static bt_status_t btb3ds_set_broadcast(btb3ds_vedio_mode_t vedio_mode, btb3ds_frame_sync_period_t frame_sync_period, UINT32 panel_delay);
static bt_status_t btb3ds_start_broadcast(void);
static bt_status_t btb3ds_stop_broadcast(void);
static bt_status_t btb3ds_start_sync_train(UINT32 synchronization_train_to);
static bt_status_t btb3ds_stop_sync_train(void);
static bt_status_t btb3ds_set_3d_offset_data(UINT8 *offset_data,UINT16 offset_data_len);

static void bta_b3ds_callback(tBTA_B3DS_EVT event, tBTA_B3DS *p_data);

// clayton: to support broadcom solution
static bt_status_t btb3ds_set_mode(const bt3d_mode_t mode, bt_bdaddr_t *addr);
static bt_status_t btb3ds_broadcast_3d_data(const bt3d_data_t data);


static btb3ds_interface_t b3ds_if = {
    sizeof(b3ds_if),
    btb3ds_jni_init_with_legacy_setting,
    btb3ds_set_broadcast,
    btb3ds_start_broadcast,
    btb3ds_stop_broadcast,
    btb3ds_start_sync_train,
    btb3ds_stop_sync_train,
    btb3ds_jni_cleanup,
    btb3ds_set_3d_offset_data,
    btb3ds_set_mode,
    btb3ds_jni_init_without_legacy_setting,
    btb3ds_broadcast_3d_data
};

btb3ds_interface_t *btif_b3ds_get_interface()
{
    BTIF_TRACE_DEBUG("btif_b3ds_get_interface");

    return &b3ds_if;
}

static void b3ds_disable()
{
    BTIF_TRACE_DEBUG("b3ds_disable");

    if(btb3ds_cb.enabled)
    {
        btb3ds_cb.enabled = 0;
        BTA_B3dsDisable();
    }
}

void btif_b3ds_cleanup()
{
    BTIF_TRACE_DEBUG("btif_b3ds_cleanup");

    if(stack_initialized)
    {
        b3ds_disable();
    }
    stack_initialized = FALSE;
}

void btif_b3ds_init(UINT8 enable_legacy_reference_protocol)
{
    BTIF_TRACE_DEBUG("btif_b3ds_init: jni_initialized = %d, btb3ds_cb.enabled:%d", jni_initialized, btb3ds_cb.enabled);

    stack_initialized = TRUE;

    if (BTB3DS_LOCAL_ROLE != BTB3DS_ROLE_3DD)
        BTIF_TRACE_ERROR("btif_b3ds_init: only support 3DD role");

#if defined(MTK_B3DS_DEMO) && (MTK_B3DS_DEMO == TRUE)
    jni_initialized = TRUE;
#endif

    if (jni_initialized && !btb3ds_cb.enabled)
    {
        BTIF_TRACE_DEBUG("Enabling B3DS....");
        memset(&btb3ds_cb, 0, sizeof(btb3ds_cb));
        btb3ds_cb.enable_legacy = enable_legacy_reference_protocol;

        BTA_B3dsEnable(bta_b3ds_callback, enable_legacy_reference_protocol);
        btb3ds_cb.enabled = 1;
    }
}

static btb3ds_callbacks_t callback;
static bt_status_t btb3ds_jni_init_with_legacy_setting(const btb3ds_callbacks_t *callbacks, UINT8 enable_legacy_reference_protocol)
{
    BTIF_TRACE_DEBUG("btb3ds_jni_init: stack_initialized = %d, btb3ds_cb.enabled:%d", stack_initialized, btb3ds_cb.enabled);

    jni_initialized = TRUE;
    callback = *callbacks;

    if(stack_initialized && !btb3ds_cb.enabled)
    btif_b3ds_init(enable_legacy_reference_protocol);
    else if (enable_legacy_reference_protocol != btb3ds_cb.enable_legacy)
    {
        BTA_B3dsSetLegacy(enable_legacy_reference_protocol);
        btb3ds_cb.enable_legacy = enable_legacy_reference_protocol;
    }

    return BT_STATUS_SUCCESS;
}

static bt_status_t btb3ds_jni_init_without_legacy_setting(const btb3ds_callbacks_t *callbacks)
{
    BTIF_TRACE_DEBUG("btb3ds_jni_init: stack_initialized = %d", stack_initialized);

    bt_status_t status = btb3ds_jni_init_with_legacy_setting(callbacks, 1);
    return status;
}


static void btb3ds_jni_cleanup()
{
    BTIF_TRACE_DEBUG("btb3ds_jni_cleanup");

    b3ds_disable();
    jni_initialized = FALSE;
}

static bt_status_t btb3ds_set_broadcast(btb3ds_vedio_mode_t vedio_mode, btb3ds_frame_sync_period_t frame_sync_period, UINT32 panel_delay)
{
    BTIF_TRACE_DEBUG("btb3ds_set_broadcast");

    BTA_B3dsSetBroadcast(vedio_mode, frame_sync_period, panel_delay);

    return BT_STATUS_SUCCESS;
}

static bt_status_t btb3ds_start_broadcast()
{
    BTIF_TRACE_DEBUG("btb3ds_start_broadcast");

    BTA_B3dsStartBroadcast();

    return BT_STATUS_SUCCESS;
}

static bt_status_t btb3ds_stop_broadcast()
{
    BTIF_TRACE_DEBUG("btb3ds_stop_broadcast");

    BTA_B3dsStopBroadcast();

    return BT_STATUS_SUCCESS;
}

static bt_status_t btb3ds_start_sync_train(UINT32 synchronization_train_to)
{
    BTIF_TRACE_DEBUG("btb3ds_start_sync_train");

    BTA_B3dsStartSyncTrain(synchronization_train_to);

    return BT_STATUS_SUCCESS;
}

static bt_status_t btb3ds_stop_sync_train()
{
    BTIF_TRACE_DEBUG("btb3ds_stop_sync_train");

    BTA_B3dsStopSyncTrain();

    return BT_STATUS_SUCCESS;
}

static void bta_b3ds_callback_transfer(UINT16 event, char *p_param)
{
    tBTA_B3DS *p_data = (tBTA_B3DS *)p_param;

    BTIF_TRACE_DEBUG("bta_b3ds_callback_transfer");
    switch(event)
    {
        case BTA_B3DS_ENABLE_EVT:
            BTIF_TRACE_DEBUG("bta_b3ds_callback_transfer: BTA_B3DS_ENABLE_EVT");
            if(callback.bcst_state_cb)
                callback.bcst_state_cb(BTB3DS_BCST_STATE_NOT_BROADCASTING, p_data->status == BTA_B3DS_SUCCESS ? BT_STATUS_SUCCESS : BT_STATUS_FAIL);
#if defined(MTK_B3DS_DEMO) && (MTK_B3DS_DEMO == TRUE)
            BTA_B3dsSetBroadcast(BTA_B3DS_VEDIO_MODE_3D, BTA_B3DS_PERIOD_DYNAMIC_CALCULATED,0);
#endif
            break;

         case BTA_B3DS_SET_BROADCAST_EVT:
            BTIF_TRACE_DEBUG("bta_b3ds_callback_transfer: BTA_B3DS_SET_BROADCAST_EVT");
#if defined(MTK_B3DS_DEMO) && (MTK_B3DS_DEMO == TRUE)
            BTA_B3dsStartBroadcast();
#endif
            break;

         case BTA_B3DS_START_BROADCAST_EVT:
            BTIF_TRACE_DEBUG("bta_b3ds_callback_transfer: BTA_B3DS_START_BROADCAST_EVT");
            if(callback.bcst_state_cb)
                callback.bcst_state_cb(BTB3DS_BCST_STATE_BROADCASTING, p_data->status == BTA_B3DS_SUCCESS ? BT_STATUS_SUCCESS : BT_STATUS_FAIL);
#if defined(MTK_B3DS_DEMO) && (MTK_B3DS_DEMO == TRUE)
            BTA_B3dsStartSyncTrain(0x0002EE00);
#endif
            break;

         case BTA_B3DS_STOP_BROADCAST_EVT:
            BTIF_TRACE_DEBUG("bta_b3ds_callback_transfer: BTA_B3DS_STOP_BROADCAST_EVT");
            if(callback.bcst_state_cb)
                callback.bcst_state_cb(BTB3DS_BCST_STATE_NOT_BROADCASTING, p_data->status == BTA_B3DS_SUCCESS ? BT_STATUS_SUCCESS : BT_STATUS_FAIL);
            break;

         case BTA_B3DS_START_SYNC_TRAIN_EVT:
            BTIF_TRACE_DEBUG("bta_b3ds_callback_transfer: BTA_B3DS_START_SYNC_TRAIN_EVT");
            if(callback.sync_state_cb)
                callback.sync_state_cb(BTB3DS_SYNC_STATE_SYNCHRONIZABLE, p_data->status == BTA_B3DS_SUCCESS ? BT_STATUS_SUCCESS : BT_STATUS_FAIL);
            break;

         case BTA_B3DS_STOP_SYNC_TRAIN_EVT:
            BTIF_TRACE_DEBUG("bta_b3ds_callback_transfer: BTA_B3DS_STOP_SYNC_TRAIN_EVT");
            if(callback.sync_state_cb)
                callback.sync_state_cb(BTB3DS_SYNC_STATE_NON_SYNCHRONIZABLE, p_data->status == BTA_B3DS_SUCCESS ? BT_STATUS_SUCCESS : BT_STATUS_FAIL);
            break;

         case BTA_B3DS_CONNECT_ANNOUNCE_MSG_EVT:
            BTIF_TRACE_DEBUG("bta_b3ds_callback_transfer: BTA_B3DS_CONNECT_ANNOUNCE_MSG_EVT");
            if(callback.connect_announce_cb)
                callback.connect_announce_cb((const bt_bdaddr_t*) p_data->con_anno.bd_addr,
                        p_data->con_anno.option & BTA_B3DS_CON_ANNO_MSG_ASSOCIATION_NOTIFICATION, p_data->con_anno.battery_level);

            if(callback.association_cb)
                callback.association_cb((bt_bdaddr_t*) p_data->con_anno.bd_addr);

            if((bt3d_mode == BT3D_MODE_MASTER) && callback.battery_level_cb)
                callback.battery_level_cb((bt_bdaddr_t*) p_data->con_anno.bd_addr, p_data->con_anno.battery_level);

            BTIF_TRACE_DEBUG("bta_b3ds_callback_transfer: battery:%d, addr:0x%02X,0x%02X,0x%02X,0x%02X,0x%02X,0x%02X", p_data->con_anno.battery_level, p_data->con_anno.bd_addr[0], p_data->con_anno.bd_addr[1], p_data->con_anno.bd_addr[2], p_data->con_anno.bd_addr[3], p_data->con_anno.bd_addr[4], p_data->con_anno.bd_addr[5]);
            break;

         case BTA_B3DS_SEND_FRAME_PERIOD_EVT:
             BTIF_TRACE_DEBUG("bta_b3ds_callback_transfer: BTA_B3DS_SEND_FRAME_PERIOD_EVT, period:%d", p_data->frame_period);

             if(callback.frame_period_cb)
                 callback.frame_period_cb(p_data->frame_period);

            break;

         case BTA_B3DS_LEGACY_ASSOC_NOTIFY_EVT:
            BTIF_TRACE_DEBUG("bta_b3ds_callback_transfer: BTA_B3DS_LEGACY_ASSOC_NOTIFY_EVT");
            if(callback.legacy_connect_announce_cb)
                callback.legacy_connect_announce_cb();

            if(callback.association_cb)
                callback.association_cb((bt_bdaddr_t*) p_data->con_anno.bd_addr);

            BTIF_TRACE_DEBUG("bta_b3ds_callback_transfer, addr:0x%02X,0x%02X,0x%02X,0x%02X,0x%02X,0x%02X", p_data->con_anno.bd_addr[0], p_data->con_anno.bd_addr[1], p_data->con_anno.bd_addr[2], p_data->con_anno.bd_addr[3], p_data->con_anno.bd_addr[4], p_data->con_anno.bd_addr[5]);
            break;

         default:
            BTIF_TRACE_WARNING("Unknown b3ds event %d", event);
            break;

    }
}

static void bta_b3ds_callback(tBTA_B3DS_EVT event, tBTA_B3DS *p_data)
{
    BTIF_TRACE_DEBUG("bta_b3ds_callback");
    btif_transfer_context(bta_b3ds_callback_transfer, event, (char*)p_data, sizeof(tBTA_B3DS), NULL);
}

static bt_status_t btb3ds_set_3d_offset_data(UINT8 *offset_data,UINT16 offset_data_len)
{
    BTIF_TRACE_WARNING("btb3ds_set_3d_offset_data : offset_data_len = %d\n", offset_data_len);

    UINT8 i = 0;
    for (i = 0; i < offset_data_len; i++) {
        BTIF_TRACE_WARNING("[DEBUG] offset_data[%02d] = 0x%02X(%d)\n", i, offset_data[i], offset_data[i]);
    }

    BTA_B3dsSet3dOffsetData(offset_data,offset_data_len);
    return BT_STATUS_SUCCESS;
}

static bt_status_t btb3ds_set_mode(const bt3d_mode_t mode, bt_bdaddr_t *addr)
{
    bdstr_t bdstr = {0};
    if(addr)
        bdaddr_to_string(addr, bdstr, sizeof(bdstr));

    BTIF_TRACE_DEBUG("btb3ds_set_mode: mode:%d, bd addr:%s", mode, bdstr);

    if (mode == BT3D_MODE_IDLE) {
        // disable 3d related behavior
        BTA_B3dsDisable3d();
    } else if (bt3d_mode == BT3D_MODE_IDLE && mode == BT3D_MODE_MASTER) {
        // if previous disable and now enable, need to trigger clock
        BTA_B3dsStartClockTriggerCapture();
    }

    bt3d_mode = mode;

    return BT_STATUS_SUCCESS;
}

static bt_status_t btb3ds_broadcast_3d_data(const bt3d_data_t data)
{
    BTIF_TRACE_DEBUG("btb3ds_broadcast_3d_data:dual_view:0x%02X, delay:0x%04X, right_open:0x%02X, rigth_close:0x%02X, left_open:0x%02X, left_close:0x%02X", data.dual_view, data.delay, data.right_open_offset, data.right_close_offset, data.left_open_offset, data.left_close_offset);

    BTA_B3dsSet3dBroadcastData(data.delay, data.dual_view, data.right_open_offset, data.right_close_offset, data.left_open_offset, data.left_close_offset);

    return BT_STATUS_SUCCESS;
}

#endif

