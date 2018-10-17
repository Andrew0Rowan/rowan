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
#include "bt_target.h"

#if defined(MTK_B3DS_SUPPORT) && (MTK_B3DS_SUPPORT == TRUE)

#include <string.h>
#include "bt_types.h"
#include "b3ds_api.h"
#include "b3ds_int.h"
#include "sdp_api.h"
#include "sdpdefs.h"
#include "l2c_api.h"
#include "hcidefs.h"
#include "btm_api.h"
#include "bta_sys.h"

const tB3DS_SHUTTER_OFFSET b3ds_shutter_offsets[] = {
    { 0xffff, 0xffff, 0xffff, 0xffff },
    { 0xffff, 0xffff, 0xffff, 0xffff },     // 2D         - 65535, 65535, 65535, 65535
    { 0x0002, 0x28b1, 0x28b2, 0x5161 },     // 3D 48Hz    -     2, 10417, 10418, 20833
    { 0x0002, 0x270f, 0x2711, 0x4e1e },     // 3D 50Hz    -     2,  9999, 10001, 19998
    { 0x0002, 0x208c, 0x208e, 0x4118 },     // 3D 60Hz    -     2,  8332,  8334, 16664
    { 0x0002, 0x208c, 0x208e, 0x4118 },     // 3D 60Hz    -     2,  8332,  8334, 16664
};

const tB3DS_CAL_FRAME_SYNC_PERIOD b3ds_cal_frame_sync_period[] = {
    { 0,        0,      0,      0     },
    { 0,        0,      0,      0     },
    { 20833,    85,     20491,  21187 },    // 48.8 Hz - 47.2 Hz
    { 20000,    0,      19685,  20326 },    // 50.8 Hz - 49.2 Hz
    { 16666,    171,    16447,  16892 },    // 60.8 Hz - 59.2 Hz
    { 16666,    171,    16447,  16892 },    // 60.8 Hz - 59.2 Hz
};


void B3DS_Register (tB3DS_REGISTER *p_register)
{
    B3DS_TRACE_DEBUG("B3DS_Register");

    b3ds_register_with_l2cap ();

    if (b3ds_cb.b3ds_sdp_handle == 0)
        b3ds_cb.b3ds_sdp_handle = b3ds_register_with_sdp(UUID_SERVCLASS_3D_DISPLAY, "3D Display");

    if (!p_register)
        return;

    b3ds_cb.b3ds_con_anno_msg_cb = p_register->b3ds_con_anno_msg_cb;
    b3ds_cb.b3ds_send_frame_period_cb = p_register->b3ds_send_frame_period_cb;
    b3ds_cb.set_2d_timer = alarm_new("b3ds.set_2D_timer");

#if defined(MTK_LEGACY_B3DS_SUPPORT) && (MTK_LEGACY_B3DS_SUPPORT == TRUE)
    b3ds_cb.b3ds_ref_proto_assoc_notify_ev_cb = p_register->b3ds_ref_proto_assoc_notify_ev_cb;
#endif

    return;
}

void B3DS_Deregister (void)
{
    B3DS_TRACE_DEBUG("B3DS_Deregister");

    if (b3ds_cb.b3ds_sdp_handle != 0)
    {
        SDP_DeleteRecord (b3ds_cb.b3ds_sdp_handle);
        b3ds_cb.b3ds_sdp_handle = 0;
    }

    L2CA_UcdDeregister(BT_PSM_B3DS);

    alarm_cancel(b3ds_cb.set_2d_timer);

    b3ds_cb.b3ds_con_anno_msg_cb = NULL;
    b3ds_cb.b3ds_send_frame_period_cb = NULL;

#if defined(MTK_LEGACY_B3DS_SUPPORT) && (MTK_LEGACY_B3DS_SUPPORT == TRUE)
    b3ds_cb.b3ds_ref_proto_assoc_notify_ev_cb = NULL;
#endif

    // L2CA_Deregister (BT_PSM_B3DS);
}

void B3DS_SetBroadcast (tB3DS_VEDIO_MODE vedio_mode, tB3DS_PERIOD period_mode, UINT32 delay)
{
    B3DS_TRACE_DEBUG("B3DS_SetBroadcast: vedio_mode %d, period_mode %d, delay %d", vedio_mode, period_mode, delay);

    b3ds_cb.broadcast.vedio_mode = vedio_mode;
    b3ds_cb.broadcast.period_mode = period_mode;
    b3ds_cb.broadcast.period_mode_dynamic = (period_mode == B3DS_PERIOD_DYNAMIC_CALCULATED) ? TRUE : FALSE;
    b3ds_cb.broadcast.delay = delay;

    b3ds_set_csb_data(0, 0);
}

void B3DS_SetBroadcastData(uint16_t delay, uint8_t dual_view, uint16_t right_open_offset,
    uint16_t right_close_offset, uint16_t left_open_offset, uint16_t left_close_offset)
{
    B3DS_TRACE_DEBUG("B3DS_SetBroadcastData: delay:0x%02X, dual_view:0x%02X, right_open:0x%02X, rigth_close:0x%02X, left_open:0x%02X, left_close:0x%02X",
        delay, dual_view, right_open_offset, right_close_offset, left_open_offset, left_close_offset);
    b3ds_set_broadcast_data(delay, dual_view, right_open_offset, right_close_offset, left_open_offset, left_close_offset);
}

void B3DS_ClockTick(UINT32 clock, UINT16 offset)
{
    B3DS_TRACE_DEBUG("B3DS_ClockTick");

    if (b3ds_cb.broadcast.period_mode_dynamic)
        b3ds_cal_frame_period(clock, offset);

    alarm_cancel(b3ds_cb.set_2d_timer);

    alarm_set_on_queue(b3ds_cb.set_2d_timer,
        B3DS_SET_2D_TIMEOUT_MS,
        b3ds_process_timeout, NULL,
        btu_general_alarm_queue);

    b3ds_set_csb_data(clock, offset);
}

UINT8 B3DS_SetTraceLevel (UINT8 new_level)
{
    B3DS_TRACE_DEBUG("B3DS_SetTraceLevel");

    if (new_level != 0xFF)
        b3ds_cb.trace_level = new_level;

    return (b3ds_cb.trace_level);
}

void B3DS_Init ()
{
    UINT8 i = 0;
    B3DS_TRACE_DEBUG("B3DS_Init");
    memset (&b3ds_cb, 0, sizeof (tB3DS_CB));

#if defined(B3DS_INITIAL_TRACE_LEVEL)
    b3ds_cb.trace_level = B3DS_INITIAL_TRACE_LEVEL;
#else
    b3ds_cb.trace_level = BT_TRACE_LEVEL_NONE;	  /* No traces */
#endif

    // Default value
    b3ds_cb.broadcast.vedio_mode = B3DS_VEDIO_MODE_3D;
    b3ds_cb.broadcast.period_mode = B3DS_PERIOD_DYNAMIC_CALCULATED;
    b3ds_cb.broadcast.period_mode_dynamic = TRUE;
    b3ds_cb.broadcast.delay = 0;
    b3ds_cb.user_set_bd_data = 0;

    b3ds_cb.broadcast.msg.frame_sync_period = B3DS_PERIOD_INVALID;

    for(i = 0; i<=B3DS_PERIOD_MAX; i++) {
        b3ds_cb.broadcast.shutter_offset[i] = b3ds_shutter_offsets[i];
        b3ds_cb.broadcast.cal_frame_sync_period[i] = b3ds_cal_frame_sync_period[i];
    }
}

void B3DS_setOffsetData(UINT8 *offset_data,UINT16 offset_data_len){

     UINT8 i = 0;
     UINT16 offset = 0;

     B3DS_TRACE_DEBUG("B3DS_setOffsetData");
     for(i = B3DS_PERIOD_MAX; i>=2; i--){
        offset=(UINT16) (*(offset_data+1) << 8 | (*offset_data & 0xFF));
        offset_data+=2;
        b3ds_cb.broadcast.shutter_offset[i].left_open=offset;
        offset=(UINT16) (*(offset_data+1) << 8 | (*offset_data & 0xFF));
        offset_data+=2;
        b3ds_cb.broadcast.shutter_offset[i].left_close=offset;
        offset=(UINT16) (*(offset_data+1) << 8 | (*offset_data & 0xFF));
        offset_data+=2;
        b3ds_cb.broadcast.shutter_offset[i].right_open=offset;
        offset=(UINT16) (*(offset_data+1) << 8 | (*offset_data & 0xFF));
        offset_data+=2;
        b3ds_cb.broadcast.shutter_offset[i].right_close=offset;

        B3DS_TRACE_DEBUG("[DEBUG]---------------------------------------------------\n");
        B3DS_TRACE_DEBUG("[DEBUG]                  : period                     = %d\n", i);
        B3DS_TRACE_DEBUG("[DEBUG]                  : left_open_offset           = 0x%04X(%d)\n", b3ds_cb.broadcast.shutter_offset[i].left_open, b3ds_cb.broadcast.shutter_offset[i].left_open);
        B3DS_TRACE_DEBUG("[DEBUG]                  : left_close_offset          = 0x%04X(%d)\n", b3ds_cb.broadcast.shutter_offset[i].left_close, b3ds_cb.broadcast.shutter_offset[i].left_close);
        B3DS_TRACE_DEBUG("[DEBUG]                  : right_open_offset          = 0x%04X(%d)\n", b3ds_cb.broadcast.shutter_offset[i].right_open, b3ds_cb.broadcast.shutter_offset[i].right_open);
        B3DS_TRACE_DEBUG("[DEBUG]                  : right_close_offset         = 0x%04X(%d)\n", b3ds_cb.broadcast.shutter_offset[i].right_close, b3ds_cb.broadcast.shutter_offset[i].right_close);
        B3DS_TRACE_DEBUG("[DEBUG]---------------------------------------------------\n");
  }
}

#if defined(MTK_LEGACY_B3DS_SUPPORT) && (MTK_LEGACY_B3DS_SUPPORT == TRUE)
void B3DS_ProcRefProtoAssocNotifyEvt(UINT8 *addr)
{
    B3DS_TRACE_DEBUG("B3DS_ProcRefProtoAssocNotifyEvt, addr:0x%02X,0x%02X,0x%02X,0x%02X,0x%02X,0x%02X", addr[0], addr[1], addr[2], addr[3], addr[4], addr[5]);
    b3ds_cb.b3ds_ref_proto_assoc_notify_ev_cb(addr);
}
#endif

#endif

