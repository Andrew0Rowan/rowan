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
 *  This is the public interface file for the B3DS
 *  subsystem of BTA, Broadcom's Bluetooth application layer for mobile
 *  phones.
 *
 ******************************************************************************/
#ifndef BTA_B3DS_API_H
#define BTA_B3DS_API_H

#include "bta_api.h"
#include "b3ds_api.h"

/*****************************************************************************
**  Constants and data types
*****************************************************************************/

/* B3DS Callback events */
#define BTA_B3DS_ENABLE_EVT                 0       /* B3DS service is initialized. */
#define BTA_B3DS_SET_BROADCAST_EVT          1
#define BTA_B3DS_START_BROADCAST_EVT        2
#define BTA_B3DS_STOP_BROADCAST_EVT         3
#define BTA_B3DS_START_SYNC_TRAIN_EVT       4
#define BTA_B3DS_STOP_SYNC_TRAIN_EVT        5
#define BTA_B3DS_CONNECT_ANNOUNCE_MSG_EVT   6
#define BTA_B3DS_LEGACY_ASSOC_NOTIFY_EVT    7
#define BTA_B3DS_SEND_FRAME_PERIOD_EVT      8
typedef UINT8 tBTA_B3DS_EVT;


/* B3DS roles */
#define BTA_B3DS_ROLE_3DD       B3DS_ROLE_3DD
#define BTA_B3DS_ROLE_3DG       B3DS_ROLE_3DD
typedef UINT8   tBTA_B3DS_ROLE;

enum {
    BTA_B3DS_VEDIO_MODE_3D            = 0,
    BTA_B3DS_VEDIO_MODE_DUAL_VIEW     = 1,
};
typedef UINT8 tBTA_B3DS_VEDIO_MODE;

enum {
    BTA_B3DS_PERIOD_DYNAMIC_CALCULATED  = 0,
    BTA_B3DS_PERIOD_2D_MODE             = 1,
    BTA_B3DS_PERIOD_3D_MODE_48_HZ       = 2,
    BTA_B3DS_PERIOD_3D_MODE_50_HZ       = 3,
    BTA_B3DS_PERIOD_3D_MODE_59_94_HZ    = 4,
    BTA_B3DS_PERIOD_3D_MODE_60_HZ       = 5,
};
typedef UINT8 tBTA_B3DS_PERIOD;

#define BTA_B3DS_CON_ANNO_MSG_ASSOCIATION_NOTIFICATION  0x01
#define BTA_B3DS_CON_ANNO_MSG_BATTERY_LEVEL             0x02
typedef UINT8 tBTA_B3DS_CON_ANNO_OPTION;

typedef struct
{
    BD_ADDR bd_addr;
    tBTA_B3DS_CON_ANNO_OPTION option;
    UINT8 battery_level;
} tBTA_B3DS_CONNECT_ANNOUNCE;

typedef BD_ADDR   tBTA_B3DS_ASSOC_NOTIFY;

enum {
    BTA_B3DS_SUCCESS,
    BTA_B3DS_FAIL,
};
typedef UINT8 tBTA_B3DS_STATUS;

/* Union of all B3DS callback structures */
typedef union
{
    tBTA_B3DS_STATUS            status;
    tBTA_B3DS_CONNECT_ANNOUNCE  con_anno;
    uint16_t                    frame_period;
} tBTA_B3DS;

/* B3DS callback */
typedef void (tBTA_B3DS_CBACK)(tBTA_B3DS_EVT event, tBTA_B3DS *p_data);

/*****************************************************************************
**  External Function Declarations
*****************************************************************************/
#ifdef __cplusplus
extern "C"
{
#endif

extern void BTA_B3dsEnable(tBTA_B3DS_CBACK p_cback, UINT8 enable_legacy);
extern void BTA_B3dsDisable(void);
extern void BTA_B3dsSetLegacy(UINT8 enable_legacy);
extern void BTA_B3dsSetBroadcast(tBTA_B3DS_VEDIO_MODE vedio_mode, tBTA_B3DS_PERIOD period_mode, UINT32 panel_delay);
extern void BTA_B3dsStartBroadcast(void);
extern void BTA_B3dsStopBroadcast(void);
extern void BTA_B3dsStartSyncTrain(UINT32 sync_train_to);
extern void BTA_B3dsStopSyncTrain(void);
extern void BTA_B3dsSet3dOffsetData(UINT8 *offset_data,UINT16 offset_data_len);
extern void BTA_B3dsSet3dBroadcastData(uint16_t delay, uint8_t dual_view, uint16_t right_open_offset, uint16_t right_close_offset, uint16_t left_open_offset, uint16_t left_close_offset);
extern void BTA_B3dsDisable3d(void);
extern void BTA_B3dsStartClockTriggerCapture(void);

extern tBTA_DM_EIR_CONF bta_b3ds_eir_cfg;

#ifdef __cplusplus
}
#endif

#endif
