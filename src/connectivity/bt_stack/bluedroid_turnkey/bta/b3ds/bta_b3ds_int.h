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
#if defined(MTK_B3DS_SUPPORT) && (MTK_B3DS_SUPPORT == TRUE)

#ifndef BTA_B3DS_INT_H
#define BTA_B3DS_INT_H

#include "bta_sys.h"
#include "bta_b3ds_api.h"
#include "bta_dm_int.h"

#define B3DS_OFFSET_DATA_LEN 32

/* B3DS events */
enum
{
    BTA_B3DS_API_ENABLE_EVT = BTA_SYS_EVT_START(BTA_ID_B3DS),
    BTA_B3DS_API_DISABLE_EVT,
    BTA_B3DS_API_SET_LEGACY_EVT,
    BTA_B3DS_API_SET_BROADCAST_EVT,
    BTA_B3DS_API_START_BROADCAST_EVT,
    BTA_B3DS_API_STOP_BROADCAST_EVT,
    BTA_B3DS_API_START_SYNC_TRAIN_EVT,
    BTA_B3DS_API_STOP_SYNC_TRAIN_EVT,
    BTA_B3DS_API_SET_OFFSET_DATA_EVT,
    BTA_B3DS_API_STOP_3D_EVT,
    BTA_B3DS_API_START_CLOCK_TRIGGERED_EVT,
    BTA_B3DS_API_SET_BROADCAST_DATA_EVT
};

/* data type for BTA_B3DS_API_ENABLE_EVT */
typedef struct
{
    BT_HDR              hdr;                        /* Event header */
    tBTA_B3DS_CBACK     *p_cback;                   /* B3DS callback function */
    UINT8               enable_legacy;
} tBTA_B3DS_API_ENABLE;

typedef struct
{
    BT_HDR              hdr;                        /* Event header */
    UINT8               enable_legacy;
} tBTA_B3DS_API_SET_LEGACY;

typedef struct
{
   BT_HDR                  hdr;                               /* Event header */
   UINT8                   offset_data[B3DS_OFFSET_DATA_LEN];
   UINT16                  offset_data_len;
} tBTA_B3DS_SET_OFFSET_DATA;

typedef struct
{
    BT_HDR                  hdr;                        /* Event header */
    tBTA_B3DS_VEDIO_MODE    vedio_mode;
    tBTA_B3DS_PERIOD        period_mode;
    UINT32                  panel_delay;
} tBTA_B3DS_API_SET_BROADCAST;

typedef struct
{
    BT_HDR                  hdr;
    uint16_t                left_open_offset;
    uint16_t                left_close_offset;
    uint16_t                right_open_offset;
    uint16_t                right_close_offset;
    uint16_t                delay;
    uint8_t                 dual_view;
} tBTA_B3DS_BROADCAST_DATA;

typedef struct
{
    BT_HDR              hdr;                        /* Event header */
    UINT32              sync_train_to;
} tBTA_B3DS_API_START_SYNC_TRAIN;

/* union of all data types */
typedef union
{
    BT_HDR                      hdr;
    tBTA_B3DS_API_ENABLE        api_enable;
    tBTA_B3DS_API_SET_LEGACY    api_set_legacy;
    tBTA_B3DS_API_SET_BROADCAST         api_set_broadcast;
    tBTA_B3DS_API_START_SYNC_TRAIN      api_start_sync_train;
    tBTA_B3DS_SET_OFFSET_DATA           api_set_offset_data;
    tBTA_B3DS_BROADCAST_DATA    api_set_broadcast_data;
} tBTA_B3DS_DATA;

/* main control block */
typedef struct
{
    tBTA_B3DS_CBACK *p_cback;                        /* B3DS callback function */
    UINT8           enable_legacy;
    UINT8           bcst_set;
    UINT32          sync_train_to;

    UINT8           broadcasting;
    UINT8           synchronizable;
} tBTA_B3DS_CB;

/*****************************************************************************
**  Global data
*****************************************************************************/
extern tBTA_B3DS_CB  bta_b3ds_cb;

/*****************************************************************************
**  Function prototypes
*****************************************************************************/
extern BOOLEAN bta_b3ds_hdl_event(BT_HDR *p_msg);

/* action functions */
extern void bta_b3ds_enable(void);
extern void bta_b3ds_disable(void);
extern void bta_b3ds_refresh_eir();
extern void bta_b3ds_set_broadcast(tBTA_B3DS_DATA *p_data);
extern void bta_b3ds_start_broadcast(void);
extern void bta_b3ds_stop_broadcast(void);
extern void bta_b3ds_start_sync_train(tBTA_B3DS_DATA *p_data);
extern void bta_b3ds_stop_sync_train(void);
extern void bta_b3ds_set_offset_data(tBTA_B3DS_DATA *p_data);

#endif
#endif
