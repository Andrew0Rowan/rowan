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
 *  This file contains the B3DS main functions and state machine.
 *
 ******************************************************************************/

#include "bt_target.h"

#if defined(MTK_B3DS_SUPPORT) && (MTK_B3DS_SUPPORT == TRUE)
#include "bta_api.h"
#include "bta_sys.h"
#include "b3ds_api.h"
#include "bta_api.h"
#include "bta_b3ds_api.h"
#include "bta_b3ds_int.h"
#include "btm_api.h"
#include <string.h>
#include "utl.h"
#include "btu.h"

tBTA_B3DS_CB  bta_b3ds_cb;

INT8 bta_b3ds_eir_inq_tx_power = B3DS_EIR_DEFAULT_INQ_TX_POWER;
UINT8 bta_b3ds_eir_3d_info[2] = {
                                    B3DS_EIR_ASSOCIAION_NOTIFICATION | B3DS_EIR_BATTERY_LEVEL_REPORTING | B3DS_EIR_STARTUP_SYNCHRONIZATION,
                                    B3DS_EIR_MAX_PATH_LOSS_THRESHOLD
};
UINT8 bta_b3ds_eir_legacy_3d_info[5] = {  B3DS_LEGACY_EIR_FIXED_ID_0,
                                          B3DS_LEGACY_EIR_FIXED_ID_1,
                                          B3DS_LEGACY_EIR_FIXED_DATA,
                                          B3DS_LEGACY_EIR_3D_SUPPORTED,
                                          B3DS_EIR_MAX_PATH_LOSS_THRESHOLD
};

/* Extended Inquiry Response */
tBTA_DM_EIR_CONF bta_b3ds_eir_cfg =
{
    50,    /* minimum length of local name when it is shortened */
           /* if length of local name is longer than this and EIR has not enough */
           /* room for all UUID list then local name is shortened to this length */
#if (BTA_EIR_CANNED_UUID_LIST == TRUE)
    8,
    (UINT8 *)bta_dm_eir_uuid16_list,
#else
    {   /* mask of UUID list in EIR */
        0xFFFFFFFF, /* LSB is the first UUID of the first 32 UUIDs in BTM_EIR_UUID_LKUP_TBL */
        0xFFFFFFFF  /* LSB is the first UUID of the next 32 UUIDs in BTM_EIR_UUID_LKUP_TBL */
        /* BTM_EIR_UUID_LKUP_TBL can be overrided */
    },
#endif
    (INT8 *)&bta_b3ds_eir_inq_tx_power,   /* Inquiry TX power         */
    0,      /* length of flags in bytes */
    NULL,   /* flags for EIR */
 #if defined(MTK_LEGACY_B3DS_SUPPORT) && (MTK_LEGACY_B3DS_SUPPORT == TRUE)
    sizeof(bta_b3ds_eir_legacy_3d_info),      /* length of manufacturer specific in bytes */
    (UINT8 *)bta_b3ds_eir_legacy_3d_info,   /* manufacturer specific */
 #else
    0,
    NULL,
 #endif
    0,      /* length of additional data in bytes */
    NULL,    /* additional data */
#if defined(MTK_B3DS_SUPPORT) && (MTK_B3DS_SUPPORT == TRUE)
    (UINT8 *)bta_b3ds_eir_3d_info,
#endif
};

extern void B3DS_SetBroadcast (tB3DS_VEDIO_MODE vedio_mode, tB3DS_PERIOD period_mode, UINT32 delay);


static void bta_b3ds_con_anno_msg_cb(BD_ADDR bd_addr, UINT8 option, UINT8 battery_level)
{
    tBTA_B3DS_CONNECT_ANNOUNCE t;
    APPL_TRACE_DEBUG("bta_b3ds_con_anno_msg_cb");

    memcpy(t.bd_addr, bd_addr, BD_ADDR_LEN);
    t.option = option;
    t.battery_level = battery_level;

    bta_b3ds_cb.p_cback(BTA_B3DS_CONNECT_ANNOUNCE_MSG_EVT, (tBTA_B3DS *)&t);

    L2CA_DISCONNECT_REQ(L2CAP_CONNECTIONLESS_CID);
}

static void bta_b3ds_send_frame_period_cb(uint16_t frame_period)
{
    UINT16* period = (UINT16 *)osi_malloc(sizeof(UINT16));
    APPL_TRACE_DEBUG("bta_b3ds_send_frame_period_cb, period:%d", frame_period);

    *period = frame_period;

    bta_b3ds_cb.p_cback(BTA_B3DS_SEND_FRAME_PERIOD_EVT, (tBTA_B3DS *) period);

    osi_free(period);
}


#if defined(MTK_LEGACY_B3DS_SUPPORT) && (MTK_LEGACY_B3DS_SUPPORT == TRUE)
static void bta_b3ds_ref_proto_assoc_notify_ev_cb ()
{
    APPL_TRACE_DEBUG("bta_b3ds_ref_proto_assoc_notify_ev_cb");

    bta_b3ds_cb.p_cback(BTA_B3DS_LEGACY_ASSOC_NOTIFY_EVT, NULL);
}
#endif

/** Callback for bta_b3ds_start_broadcast */
static void bta_b3ds_start_csb_cb(UINT8 *p_status)
{
    tBTA_B3DS_STATUS status = BTA_B3DS_SUCCESS;
    APPL_TRACE_DEBUG("bta_b3ds_start_csb_cb");

    if((*p_status) != HCI_SUCCESS)
        status = BTA_B3DS_FAIL;
    else
        bta_b3ds_cb.broadcasting = TRUE;

    bta_b3ds_cb.p_cback(BTA_B3DS_START_BROADCAST_EVT, (tBTA_B3DS *) &status);
}

/** Callback for bta_b3ds_stop_broadcast */
static void bta_b3ds_stop_csb_cb(UINT8 *p_status)
{
    tBTA_B3DS_STATUS status = BTA_B3DS_SUCCESS;
    APPL_TRACE_DEBUG("bta_b3ds_stop_csb_cb");

    if((*p_status) != HCI_SUCCESS)
    {
        status = BTA_B3DS_FAIL;
        bta_b3ds_cb.p_cback(BTA_B3DS_STOP_BROADCAST_EVT, (tBTA_B3DS *) &status);
        return;
    }
    else if(bta_b3ds_cb.synchronizable)
        bta_b3ds_cb.p_cback(BTA_B3DS_STOP_SYNC_TRAIN_EVT, (tBTA_B3DS *) &status);

    bta_b3ds_cb.broadcasting = FALSE;
    bta_b3ds_cb.p_cback(BTA_B3DS_STOP_BROADCAST_EVT, (tBTA_B3DS *) &status);
}

/** Callback for bta_b3ds_start_sync_train */
static void bta_b3ds_start_sync_train_cb(UINT8 *p_status)
{
    tBTA_B3DS_STATUS status = BTA_B3DS_SUCCESS;
    APPL_TRACE_DEBUG("bta_b3ds_start_sync_train_cb");

    if((*p_status) != HCI_SUCCESS)
        status = BTA_B3DS_FAIL;
    else
        bta_b3ds_cb.synchronizable = TRUE;

    bta_b3ds_cb.p_cback(BTA_B3DS_START_SYNC_TRAIN_EVT, (tBTA_B3DS *) &status);
}

/** Callback for bta_b3ds_stop_sync_train_cbt */
static void bta_b3ds_stop_sync_train_cb(UINT8 *p_status)
{
    tBTA_B3DS_STATUS status = BTA_B3DS_SUCCESS;
    APPL_TRACE_DEBUG("bta_b3ds_stop_sync_train_cb");

    bta_b3ds_cb.synchronizable = FALSE;

    bta_b3ds_cb.p_cback(BTA_B3DS_STOP_SYNC_TRAIN_EVT, (tBTA_B3DS *) &status);
}

static void bta_b3ds_read_inq_tx_power_cb(void *p)
{
    tBTM_INQ_TXPWR_RESULTS  *p_results = (tBTM_INQ_TXPWR_RESULTS  *)p;
    APPL_TRACE_DEBUG("bta_b3ds_read_inq_tx_power_cb");

    if(p_results->status == BTM_SUCCESS)
    {
        bta_b3ds_eir_inq_tx_power = p_results->tx_power;
        APPL_TRACE_DEBUG("bta_b3ds_read_inq_tx_power_cb: set tx_power = %d", bta_b3ds_eir_inq_tx_power);
    }
    bta_b3ds_eir_cfg.bta_dm_eir_3d_info = bta_b3ds_eir_3d_info;
#if defined(MTK_LEGACY_B3DS_SUPPORT) && (MTK_LEGACY_B3DS_SUPPORT == TRUE)
    if (bta_b3ds_cb.enable_legacy)
    {
        bta_b3ds_eir_cfg.bta_dm_eir_manufac_spec_len = sizeof(bta_b3ds_eir_legacy_3d_info);
        bta_b3ds_eir_cfg.bta_dm_eir_manufac_spec = bta_b3ds_eir_legacy_3d_info;
    }
#endif
    BTA_DmSetEIRConfig(&bta_b3ds_eir_cfg);

}

void bta_b3ds_refresh_eir(void)
{
    APPL_TRACE_DEBUG("bta_b3ds_refresh_eir");

    BTM_ReadInquiryRspTxPower (bta_b3ds_read_inq_tx_power_cb);
}

void bta_b3ds_set_broadcast(tBTA_B3DS_DATA *p_data)
{
    APPL_TRACE_DEBUG("bta_b3ds_set_broadcast");

    bta_b3ds_cb.bcst_set = TRUE;

    B3DS_SetBroadcast(p_data->api_set_broadcast.vedio_mode, p_data->api_set_broadcast.period_mode, p_data->api_set_broadcast.panel_delay);
    bta_b3ds_cb.p_cback(BTA_B3DS_SET_BROADCAST_EVT, NULL);
}

void bta_b3ds_set_broadcast_data(tBTA_B3DS_DATA *p_data)
{
    APPL_TRACE_DEBUG("bta_b3ds_set_broadcast_data");
    B3DS_SetBroadcastData(p_data->api_set_broadcast_data.delay, p_data->api_set_broadcast_data.dual_view, p_data->api_set_broadcast_data.right_open_offset, p_data->api_set_broadcast_data.right_close_offset, p_data->api_set_broadcast_data.left_open_offset, p_data->api_set_broadcast_data.left_close_offset);
}

void bta_b3ds_set_offset_data(tBTA_B3DS_DATA *p_data)
{
    APPL_TRACE_DEBUG("bta_b3ds_set_offset_data");
    B3DS_setOffsetData(p_data->api_set_offset_data.offset_data,p_data->api_set_offset_data.offset_data_len);
}

void bta_b3ds_start_broadcast(void)
{
    tBTA_B3DS_STATUS status = BTA_B3DS_SUCCESS;
    APPL_TRACE_DEBUG("bta_b3ds_start_broadcast");

    if(bta_b3ds_cb.broadcasting)
    {
        bta_b3ds_cb.p_cback(BTA_B3DS_START_BROADCAST_EVT, (tBTA_B3DS *) &status);
        return;
    }

    if(!bta_b3ds_cb.bcst_set)
        APPL_TRACE_ERROR("bta_b3ds_start_broadcast: no broadcast data set");

    if (BTM_SetCsb(TRUE, B3DS_LT_ADDR, FALSE, B3DS_CSB_PKT_TYPE, B3DS_CSB_MIN_INTERVAL,
            B3DS_CSB_MAX_INTERVAL, B3DS_CSB_SUPERVISION_TO, (tBTM_CMPL_CB *) bta_b3ds_start_csb_cb) != BTM_CMD_STARTED)
    {
        APPL_TRACE_ERROR("bta_b3ds_start_broadcast: BTM_SetCsb failed");
        status = BTA_B3DS_FAIL;
        bta_b3ds_cb.p_cback(BTA_B3DS_START_BROADCAST_EVT, (tBTA_B3DS *) &status);
    }
}

void bta_b3ds_stop_broadcast(void)
{
    tBTA_B3DS_STATUS status = BTA_B3DS_SUCCESS;
    APPL_TRACE_DEBUG("bta_b3ds_stop_broadcast");

    if(!bta_b3ds_cb.broadcasting)
    {
        APPL_TRACE_DEBUG("_bta_b3ds_start_sync_train: wrong state: not_broadcasting");
        bta_b3ds_cb.p_cback(BTA_B3DS_STOP_BROADCAST_EVT, (tBTA_B3DS *) &status);
        return;
    }

    if (BTM_SetCsb(FALSE, B3DS_LT_ADDR, FALSE, B3DS_CSB_PKT_TYPE, B3DS_CSB_MIN_INTERVAL,
            B3DS_CSB_MAX_INTERVAL, B3DS_CSB_SUPERVISION_TO, (tBTM_CMPL_CB *) bta_b3ds_stop_csb_cb) != BTM_CMD_STARTED)
    {
        APPL_TRACE_ERROR("bta_b3ds_stop_broadcast: BTM_SetCsb failed");
        status = BTA_B3DS_FAIL;
        bta_b3ds_cb.p_cback(BTA_B3DS_STOP_BROADCAST_EVT, (tBTA_B3DS *) &status);
    }
}

void _bta_b3ds_start_sync_train()
{
    tBTA_B3DS_STATUS status = BTA_B3DS_SUCCESS;
    APPL_TRACE_DEBUG("_bta_b3ds_start_sync_train");

    if(!bta_b3ds_cb.broadcasting)
    {
        APPL_TRACE_ERROR("_bta_b3ds_start_sync_train: wrong state: not_broadcasting");
        return;
    }

    else if (bta_b3ds_cb.synchronizable)
        APPL_TRACE_WARNING("_bta_b3ds_start_sync_train: wrong state: synchronizable");

    bta_b3ds_cb.synchronizable = TRUE;

    if (BTM_StartSyncTrain((tBTM_CMPL_CB *) bta_b3ds_start_sync_train_cb, (tBTM_CMPL_CB *) bta_b3ds_stop_sync_train_cb) != BTM_CMD_STARTED)
    {
        APPL_TRACE_ERROR("_bta_b3ds_start_sync_train: BTM_WriteSyncTrainPara failed");
        status = BTA_B3DS_FAIL;
        bta_b3ds_cb.p_cback(BTA_B3DS_START_SYNC_TRAIN_EVT, (tBTA_B3DS *) &status);
    }
}

void bta_b3ds_start_sync_train(tBTA_B3DS_DATA *p_data)
{
    APPL_TRACE_DEBUG("bta_b3ds_start_sync_train: %x", p_data->api_start_sync_train.sync_train_to);

    if (p_data->api_start_sync_train.sync_train_to != bta_b3ds_cb.sync_train_to)
    {
        bta_b3ds_cb.sync_train_to = p_data->api_start_sync_train.sync_train_to;
        BTM_WriteSyncTrainPara(B3DS_SYNC_TRAIN_MIN_INTERVAL, B3DS_SYNC_TRAIN_MAX_INTERVAL,
                bta_b3ds_cb.sync_train_to, B3DS_SYNC_TRAIN_LEGACY_HOME, NULL);
    }

    _bta_b3ds_start_sync_train();
}

void bta_b3ds_stop_sync_train(void)
{
    tBTA_B3DS_STATUS status = BTA_B3DS_SUCCESS;
    APPL_TRACE_DEBUG("bta_b3ds_stop_sync_train");

    if(!bta_b3ds_cb.synchronizable)
    {
        APPL_TRACE_DEBUG("bta_b3ds_stop_sync_train: wrong state: not_synchronizable");
        bta_b3ds_cb.p_cback(BTA_B3DS_STOP_SYNC_TRAIN_EVT, (tBTA_B3DS *) &status);
        return;
    }

    BTM_WriteSyncTrainPara(B3DS_SYNC_TRAIN_MIN_INTERVAL, B3DS_SYNC_TRAIN_MAX_INTERVAL,
                 B3DS_SYNC_TRAIN_MIN_TO, B3DS_SYNC_TRAIN_LEGACY_HOME, (tBTM_CMPL_CB *) bta_b3ds_stop_sync_train_cb);

    // TODO: Check with 7662 FW

    // if (BTM_StartSyncTrain(NULL, (tBTM_CMPL_CB *) bta_b3ds_stop_sync_train_cb) != BTM_CMD_STARTED)
    // {
    //    APPL_TRACE_ERROR("bta_b3ds_stop_sync_train: BTM_WriteSyncTrainPara failed");
    //    status = BTA_B3DS_FAIL;
    //    bta_b3ds_cb.p_cback(BTA_B3DS_START_SYNC_TRAIN_EVT, (tBTA_B3DS *) &status);
    // }
}

static void bta_b3ds_channel_map_change_evt_cb(void *p)
{
    APPL_TRACE_DEBUG("bta_b3ds_channel_map_change_evt_cb");

    _bta_b3ds_start_sync_train();
}

static void bta_b3ds_slave_page_timeout_evt_cb(void *p)
{
    APPL_TRACE_DEBUG("bta_b3ds_slave_page_timeout_evt_cb");

    _bta_b3ds_start_sync_train();
}

static void bta_b3ds_clock_capture_evt_cb(UINT8 *p)
{
    UINT16 handle, slot_offset;
    UINT8 which_clock;
    UINT32 clock;
    APPL_TRACE_DEBUG("bta_b3ds_clock_capture_evt_cb");

    STREAM_TO_UINT16(handle, p);
    STREAM_TO_UINT8(which_clock, p);
    STREAM_TO_UINT32(clock, p);
    STREAM_TO_UINT16(slot_offset, p);

    B3DS_ClockTick(clock, slot_offset);
}

void bta_b3ds_enable(void)
{
    tB3DS_REGISTER reg_data;

    APPL_TRACE_DEBUG("bta_b3ds_enable");

    reg_data.b3ds_con_anno_msg_cb  = bta_b3ds_con_anno_msg_cb;

    APPL_TRACE_DEBUG("b3ds: register  bta_b3ds_send_frame_period_cb");
    //clayton: add for BCM
    reg_data.b3ds_send_frame_period_cb = bta_b3ds_send_frame_period_cb;

#if defined(MTK_LEGACY_B3DS_SUPPORT) && (MTK_LEGACY_B3DS_SUPPORT == TRUE)
    if (bta_b3ds_cb.enable_legacy)
        reg_data.b3ds_ref_proto_assoc_notify_ev_cb  = bta_b3ds_ref_proto_assoc_notify_ev_cb;
#endif

    B3DS_Register (&reg_data);

    BTM_RegisterForTriggeredClockCaptureEvt((tBTM_CMPL_CB *)bta_b3ds_clock_capture_evt_cb);
    BTM_RegisterForCsbChannelMapChangeEvt((tBTM_CMPL_CB *)bta_b3ds_channel_map_change_evt_cb);
    BTM_RegisterForSlavePageResponseTimeoutEvt((tBTM_CMPL_CB *)bta_b3ds_slave_page_timeout_evt_cb);

    bta_b3ds_refresh_eir();

    // Always Discoverable and Connectable
    BTM_SetDiscoverability (BTM_GENERAL_DISCOVERABLE, 0, 0);
    BTM_SetConnectability (BTM_CONNECTABLE, 0, 0);

    BTM_SetReservedLtAddr(B3DS_LT_ADDR);

    BTM_SetTriggeredClockCapture(0x00, TRUE, B3DS_SET_CLK_LOCAL, FALSE, B3DS_SET_CLK_NUM_CAP_TO_FILTER);

    BTM_WriteSyncTrainPara(B3DS_SYNC_TRAIN_MIN_INTERVAL, B3DS_SYNC_TRAIN_MAX_INTERVAL,
        bta_b3ds_cb.sync_train_to, B3DS_SYNC_TRAIN_LEGACY_HOME, NULL);

    bta_b3ds_cb.p_cback(BTA_B3DS_ENABLE_EVT, NULL);

}

void bta_b3ds_disable(void)
{
    APPL_TRACE_DEBUG("bta_b3ds_disable");

    BTM_RegisterForTriggeredClockCaptureEvt(NULL);
    BTM_RegisterForCsbChannelMapChangeEvt(NULL);
    BTM_RegisterForSlavePageResponseTimeoutEvt(NULL);

    B3DS_Deregister();

    bta_b3ds_eir_cfg.bta_dm_eir_3d_info = NULL;
    bta_b3ds_eir_cfg.bta_dm_eir_manufac_spec_len = 0;
    bta_b3ds_eir_cfg.bta_dm_eir_manufac_spec = NULL;
    BTA_DmSetEIRConfig(&bta_b3ds_eir_cfg);

    BTM_SetTriggeredClockCapture(0x00, FALSE, B3DS_SET_CLK_LOCAL, FALSE, B3DS_SET_CLK_NUM_CAP_TO_FILTER);
}

void bta_b3ds_stop_3d(void)
{
    APPL_TRACE_DEBUG("bta_b3ds_stop_3d");
    APPL_TRACE_DEBUG("bta_b3ds_stop_3d: stop triggered clock capture");
    BTM_SetTriggeredClockCapture(0x00, FALSE, B3DS_SET_CLK_LOCAL, FALSE, B3DS_SET_CLK_NUM_CAP_TO_FILTER);
}

void bta_b3ds_start_clock_triggered_capture(void)
{
    APPL_TRACE_DEBUG("bta_b3ds_start_clock_triggered_capture");
    BTM_SetTriggeredClockCapture(0x00, TRUE, B3DS_SET_CLK_LOCAL, FALSE, B3DS_SET_CLK_NUM_CAP_TO_FILTER);
}

BOOLEAN bta_b3ds_hdl_event(BT_HDR *p_msg)
{
    BOOLEAN     freebuf = TRUE;
    APPL_TRACE_DEBUG("bta_b3ds_hdl_event");

    switch (p_msg->event)
    {
        /* handle enable event */
        case BTA_B3DS_API_ENABLE_EVT:
            APPL_TRACE_DEBUG("bta_b3ds_hdl_event: BTA_B3DS_API_ENABLE_EVT");
            memset(&bta_b3ds_cb, 0, sizeof(bta_b3ds_cb));
            bta_b3ds_cb.sync_train_to = B3DS_SYNC_TRAIN_TO;
            bta_b3ds_cb.p_cback = ((tBTA_B3DS_DATA *)p_msg)->api_enable.p_cback;
            bta_b3ds_cb.enable_legacy = ((tBTA_B3DS_DATA *)p_msg)->api_enable.enable_legacy;

            bta_b3ds_enable();
            break;

        /* handle disable event */
        case BTA_B3DS_API_DISABLE_EVT:
            APPL_TRACE_DEBUG("bta_b3ds_hdl_event: BTA_B3DS_API_DISABLE_EVT");
            bta_b3ds_stop_broadcast();
            bta_b3ds_disable();
            break;

        case BTA_B3DS_API_STOP_3D_EVT:
            APPL_TRACE_DEBUG("bta_b3ds_hdl_event: BTA_B3DS_API_STOP_3D_EVT");
            bta_b3ds_stop_broadcast();
            bta_b3ds_stop_sync_train();
            bta_b3ds_stop_3d();
            break;

        case BTA_B3DS_API_START_CLOCK_TRIGGERED_EVT:
            APPL_TRACE_DEBUG("bta_b3ds_hdl_event: BTA_B3DS_API_START_CLOCK_TRIGGERED_EVT");
            bta_b3ds_start_clock_triggered_capture();
            break;

        case BTA_B3DS_API_SET_LEGACY_EVT:
            APPL_TRACE_DEBUG("bta_b3ds_hdl_event: BTA_B3DS_API_SET_LEGACY_EVT");
            if(((tBTA_B3DS_DATA *)p_msg)->api_set_legacy.enable_legacy != bta_b3ds_cb.enable_legacy)
            {
                bta_b3ds_cb.enable_legacy = ((tBTA_B3DS_DATA *)p_msg)->api_set_legacy.enable_legacy;
                bta_b3ds_refresh_eir();
            }
            break;

        case BTA_B3DS_API_SET_BROADCAST_EVT:
            APPL_TRACE_DEBUG("bta_b3ds_hdl_event: BTA_B3DS_API_SET_BROADCAST_EVT");
            bta_b3ds_set_broadcast((tBTA_B3DS_DATA *) p_msg);
            break;

        case BTA_B3DS_API_SET_BROADCAST_DATA_EVT:
            APPL_TRACE_DEBUG("bta_b3ds_hdl_event: BTA_B3DS_API_SET_BROADCAST_DATA_EVT");
            bta_b3ds_set_broadcast_data((tBTA_B3DS_DATA *) p_msg);
            break;

        case BTA_B3DS_API_START_BROADCAST_EVT:
            APPL_TRACE_DEBUG("bta_b3ds_hdl_event: BTA_B3DS_API_START_BROADCAST_EVT");
            bta_b3ds_start_broadcast();
            break;

        case BTA_B3DS_API_STOP_BROADCAST_EVT:
            APPL_TRACE_DEBUG("bta_b3ds_hdl_event: BTA_B3DS_API_STOP_BROADCAST_EVT");
            bta_b3ds_stop_broadcast();
            break;

        case BTA_B3DS_API_START_SYNC_TRAIN_EVT:
            APPL_TRACE_DEBUG("bta_b3ds_hdl_event: BTA_B3DS_API_START_SYNC_TRAIN_EVT");
            bta_b3ds_start_sync_train((tBTA_B3DS_DATA *) p_msg);
            break;

        case BTA_B3DS_API_STOP_SYNC_TRAIN_EVT:
            APPL_TRACE_DEBUG("bta_b3ds_hdl_event: BTA_B3DS_API_STOP_SYNC_TRAIN_EVT");
            bta_b3ds_stop_sync_train();
            break;


        case BTA_B3DS_API_SET_OFFSET_DATA_EVT:
            APPL_TRACE_DEBUG("bta_b3ds_hdl_event: BTA_B3DS_API_SET_OFFSET_DATA_EVT");
            bta_b3ds_set_offset_data((tBTA_B3DS_DATA *) p_msg);
            break;

        default:
            APPL_TRACE_WARNING("bta_b3ds_hdl_event: No such event");

        break;

    }
    return freebuf;
}

#endif

