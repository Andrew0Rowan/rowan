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
 *  This file contains internally used B3DS definitions
 *
 ******************************************************************************/

#ifndef  B3DS_INT_H
#define  B3DS_INT_H

#include "bt_target.h"
#include "b3ds_api.h"
#include "btu.h"
#include "osi/include/osi.h"

extern fixed_queue_t *btu_general_alarm_queue;

typedef struct {
    UINT16  left_open;
    UINT16  left_close;
    UINT16  right_open;
    UINT16  right_close;
} tB3DS_SHUTTER_OFFSET;


typedef struct {
    tB3DS_SHUTTER_OFFSET shutter_offset_Default;
    tB3DS_SHUTTER_OFFSET shutter_offset_2D;
    tB3DS_SHUTTER_OFFSET shutter_offset_3D_48HZ;
    tB3DS_SHUTTER_OFFSET shutter_offset_3D_50HZ;
    tB3DS_SHUTTER_OFFSET shutter_offset_3D_59_94HZ;
    tB3DS_SHUTTER_OFFSET shutter_offset_3D_60HZ;
} tB3DS_EEPROM_SHUTTER_OFFSETS;


typedef struct {
    UINT16  period;
    UINT8   period_fraction;
    UINT16  min;
    UINT16  max;
} tB3DS_CAL_FRAME_SYNC_PERIOD;

typedef struct {
    UINT32 frame_sync_instant;          //only 28 bytes are valid, byte0 bit0 ~ byte3 bit3
    // UINT8  video_mode;                             //3D mode 0, Dual mode 1, byte3 bit6
    UINT16 frame_sync_instant_phase;    //0~312 us
    UINT16 left_open_offset;            //0~65535 us
    UINT16 left_close_offset;           //0~65535 us
    UINT16 right_open_offset;           //0~65535 us
    UINT16 right_close_offset;          //0~65535 us
    UINT16 frame_sync_period;           //<=40000 us
    UINT8  frame_sync_period_fraction;  // 0~255  1/256us
} tB3DS_BROADCAST_MSG;

typedef struct {
    tB3DS_VEDIO_MODE    vedio_mode;
    tB3DS_PERIOD        period_mode;
    UINT8               period_mode_dynamic;    // 0 or 1
    UINT32              delay;

    tB3DS_BROADCAST_MSG msg;
    tB3DS_BROADCAST_MSG user_set_msg;

    tB3DS_CAL_FRAME_SYNC_PERIOD cal_frame_sync_period[B3DS_PERIOD_MAX+1];
    tB3DS_SHUTTER_OFFSET shutter_offset[B3DS_PERIOD_MAX+1];
} tB3DS_BROADCAST;

/*  The main B3DS control block
*/
typedef struct
{
    tB3DS_CONNECT_ANNOUNCE_MSG_CB   *b3ds_con_anno_msg_cb;

    // clayton: for BCM
    tB3DS_SEND_FRAME_PEROID_CB *b3ds_send_frame_period_cb;

#if defined(MTK_LEGACY_B3DS_SUPPORT) && (MTK_LEGACY_B3DS_SUPPORT == TRUE)
    tB3DS_REF_PROTO_ASSOC_NOTIFY_EVT_CB *b3ds_ref_proto_assoc_notify_ev_cb;
#endif

    tB3DS_BROADCAST     broadcast;

    alarm_t             *set_2d_timer;
    UINT32              b3ds_sdp_handle;
    UINT8               trace_level;
    UINT8               user_set_bd_data;
    UINT16              current_frame_period;
    UINT16              frame_period_fraction;
} tB3DS_CB;

#ifdef __cplusplus
extern "C" {
#endif

/* Global B3DS data
*/
extern tB3DS_CB  b3ds_cb;
extern const tB3DS_CAL_FRAME_SYNC_PERIOD b3ds_cal_frame_sync_period[];

/* Functions provided by b3ds_main.c
*/
extern UINT32 b3ds_register_with_l2cap (void);
extern UINT32 b3ds_register_with_sdp (UINT16 uuid, char *p_name);
extern void b3ds_proc_cls_data_ind (BD_ADDR bd_addr, BT_HDR *p_buf);

extern void b3ds_set_csb_data(UINT32 clock, UINT16 offset);
extern void b3ds_set_broadcast_data(uint16_t delay, uint8_t dual_view, uint16_t right_open_offset, uint16_t right_close_offset, uint16_t left_open_offset, uint16_t left_close_offset);

extern void b3ds_cal_frame_period(UINT32 clock, UINT16 offset);

extern void b3ds_process_timeout(UNUSED_ATTR void *data);

#ifdef __cplusplus
}
#endif


#endif
