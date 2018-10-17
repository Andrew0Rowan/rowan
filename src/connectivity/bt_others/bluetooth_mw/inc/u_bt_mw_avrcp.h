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


/* FILE NAME:  u_bt_mw_avrcp.h
 * AUTHOR: Hongliang Hu
 * PURPOSE:
 *      It provides bluetooth avrcp structure to APP.
 * NOTES:
 */


#ifndef _U_BT_MW_AVRCP_H_
#define _U_BT_MW_AVRCP_H_

/*-----------------------------------------------------------------------------
                    include files
-----------------------------------------------------------------------------*/

#include "u_bt_mw_common.h"

/*-----------------------------------------------------------------------------
                    macros, defines, typedefs, enums
 ----------------------------------------------------------------------------*/
#define BT_AVRCP_UID_SIZE           (8)
#define BT_AVRCP_MAX_APP_SETTINGS   (8)

#define BT_AVRCP_MAX_ATTR_CNT       (7)
#define BT_AVRCP_MAX_NAME_LEN       (255)


typedef enum
{
    AVRCP_PLAY_STATUS_STOPPED = 0x00,       /* Stopped */
    AVRCP_PLAY_STATUS_PLAYING = 0x01,       /* Playing */
    AVRCP_PLAY_STATUS_PAUSED = 0x02,        /* Paused  */
    AVRCP_PLAY_STATUS_FORWARDSEEK= 0x03,    /* Fwd Seek*/
    AVRCP_PLAY_STATUS_REWINDSEEK= 0x04,     /* Rev Seek*/
    AVRCP_PLAY_STATUS_MAX = 0xFF,           /* Error   */
}BT_AVRCP_PLAY_STATUS;

typedef enum
{
    BT_AVRCP_KEY_STATE_PRESS = 0,   /* key is press   */
    BT_AVRCP_KEY_STATE_RELEASE,     /* key is release */
    BT_AVRCP_KEY_STATE_AUTO,        /* key is press and release */
    BT_AVRCP_KEY_STATE_MAX
}BT_AVRCP_KEY_STATE;

typedef enum
{
    BT_AVRCP_CMD_TYPE_PLAY = 0,     /* play          */
    BT_AVRCP_CMD_TYPE_PAUSE,        /* pause         */
    BT_AVRCP_CMD_TYPE_FWD,          /* forward/next  */
    BT_AVRCP_CMD_TYPE_BWD,          /* backward      */
    BT_AVRCP_CMD_TYPE_FFWD,         /* fast forward  */
    BT_AVRCP_CMD_TYPE_RWD,          /* reward        */
    BT_AVRCP_CMD_TYPE_STOP,         /* stop          */
    BT_AVRCP_CMD_TYPE_VOL_UP,       /* volume up     */
    BT_AVRCP_CMD_TYPE_VOL_DOWN,     /* volume down   */

    BT_AVRCP_CMD_TYPE_MAX
} BT_AVRCP_CMD_TYPE;



typedef enum
{
    BT_AVRCP_EVENT_CONNECTED,               /* AVRCP connected */
    BT_AVRCP_EVENT_DISCONNECTED,            /* AVRCP disconnected */

    BT_AVRCP_EVENT_TRACK_CHANGE,            /* remote trach change */
    BT_AVRCP_EVENT_POS_CHANGE,              /* remote position change */
    BT_AVRCP_EVENT_PLAY_STATUS_CHANGE,      /* remote play status change */
    BT_AVRCP_EVENT_VOLUME_CHANGE,           /* remote absolute volume change */

    BT_AVRCP_EVENT_SET_VOLUME_REQ,          /* remote set local absolute volume */
    BT_AVRCP_EVENT_PASSTHROUGH_CMD_REQ,     /* remote send passthrough command */
    BT_AVRCP_EVENT_MAX
} BT_AVRCP_EVENT;

typedef enum
{
    BT_AVRCP_REG_EVT_TRACK_CHANGED,          /* track change */
    BT_AVRCP_REG_EVT_PLAY_POS_CHANGED,       /* position change */
    BT_AVRCP_REG_EVT_PLAY_STATUS_CHANGED,    /* play status change */
    BT_AVRCP_REG_EVT_TRACK_REACHED_END,      /* track reach end */
    BT_AVRCP_REG_EVT_TRACK_REACHED_START,    /* track reach start */
    BT_AVRCP_REG_EVT_APP_SETTINGS_CHANGED,   /* app setting change */
    BT_AVRCP_REG_EVT_AVAL_PLAYERS_CHANGED,   /* available player change */
    BT_AVRCP_REG_EVT_ADDR_PLAYER_CHANGED,    /* addressed player change */
    BT_AVRCP_REG_EVT_ABS_VOLUME_CHANGED,     /* absolute volume change */
    BT_AVRCP_REG_EVT_MAX,
} BT_AVRCP_REG_EVT_ID;

typedef enum {
    BT_AVRCP_MEDIA_ATTR_TITLE = 0x01,       /* title */
    BT_AVRCP_MEDIA_ATTR_ARTIST = 0x02,      /* artist */
    BT_AVRCP_MEDIA_ATTR_ALBUM = 0x03,       /* album */
    BT_AVRCP_MEDIA_ATTR_TRACK_NUM = 0x04,   /* track number */
    BT_AVRCP_MEDIA_ATTR_NUM_TRACKS = 0x05,  /* number of the track */
    BT_AVRCP_MEDIA_ATTR_GENRE = 0x06,       /* genre */
    BT_AVRCP_MEDIA_ATTR_PLAYING_TIME = 0x07,/* play time */
} BT_AVRCP_MEDIA_ATTR;

/* player setting attribute */
typedef enum {
    BT_AVRCP_PLAYER_EQUAL_VAL_OFF = 0x01,   /* equalizer OFF */
    BT_AVRCP_PLAYER_EQUAL_VAL_ON = 0x02,    /* equalizer ON */
} BT_AVRCP_PLAYER_EQUAL_VAL;

typedef enum {
    BT_AVRCP_PLAYER_REPEAT_VAL_OFF = 0x01,      /* repeat mode OFF */
    BT_AVRCP_PLAYER_REPEAT_VAL_SINGLE = 0x02,   /* repeat mode Single */
    BT_AVRCP_PLAYER_REPEAT_VAL_ALL = 0x03,      /* repeat mode ALL */
    BT_AVRCP_PLAYER_REPEAT_VAL_GROUP = 0x04     /* repeate mode GROUP */
} BT_AVRCP_PLAYER_REPEAT_VAL;

typedef enum {
    BT_AVRCP_PLAYER_SHUFFLE_VAL_OFF = 0x01, /* shuffle ON */
    BT_AVRCP_PLAYER_SHUFFLE_VAL_ALL = 0x02, /* shuffle OFF */
    BT_AVRCP_PLAYER_SHUFFLE_VAL_GROUP = 0x03 /* shuffle GROUP */
} BT_AVRCP_PLAYER_SHUFFLE_VAL;

typedef enum {
    BT_AVRCP_PLAYER_SCAN_VAL_OFF = 0x01,    /* scan OFF */
    BT_AVRCP_PLAYER_SCAN_VAL_ALL = 0x02,    /* scan ALL */
    BT_AVRCP_PLAYER_SCAN_VAL_GROUP = 0x03   /* scan GROUP */
} BT_AVRCP_PLAYER_SCAN_VAL;


typedef enum {
    BT_AVRCP_PLAYER_ATTR_EQUALIZER = 0x01,  /* equalizer */
    BT_AVRCP_PLAYER_ATTR_REPEAT = 0x02,     /* repeat mode */
    BT_AVRCP_PLAYER_ATTR_SHUFFLE = 0x03,    /* shuffle status */
    BT_AVRCP_PLAYER_ATTR_SCAN = 0x04,       /* scan status */
} BT_AVRCP_PLAYER_ATTR;



typedef struct
{
    CHAR title[BT_AVRCP_MAX_NAME_LEN];
    CHAR artist[BT_AVRCP_MAX_NAME_LEN];
    CHAR album[BT_AVRCP_MAX_NAME_LEN];
    UINT32 current_track_number;
    UINT32 number_of_tracks;
    CHAR genre[BT_AVRCP_MAX_NAME_LEN];
    UINT32 position;
}BT_AVRCP_MEDIA_INFO;


typedef struct
{
    UINT8 num_attr;                         /* number of attribute in attrs */
    UINT8 attr_ids[BT_AVRCP_UID_SIZE];      /* attribute ID, BT_AVRCP_PLAYER_ATTR */
    UINT8 attr_values[BT_AVRCP_MAX_APP_SETTINGS];
} BT_AVRCP_PLAYER_SETTING;

typedef struct {
    UINT16 player_id;
    UINT16 uid_counter;
} BT_AVRCP_ADDR_PLAYER;


typedef struct
{
    UINT8 abs_volume; /* unit: 1% */
}BT_AVRCP_SET_VOL_REQ;

typedef struct
{
    BT_AVRCP_MEDIA_INFO element_attr;
}BT_AVRCP_TRACK_CHANGE;

typedef struct
{
    UINT32 song_len; /* unit: ms */
    UINT32 song_pos; /* unit: ms */
}BT_AVRCP_POS_CHANGE;

typedef struct
{
    BT_AVRCP_PLAY_STATUS play_status;
}BT_AVRCP_PLAY_STATUS_CHANGE;

typedef struct
{
    UINT8 abs_volume; /* unit: 1% */
}BT_AVRCP_VOLUME_CHANGE;


typedef struct
{
    BT_AVRCP_CMD_TYPE cmd_type;
    BT_AVRCP_KEY_STATE action;
}BT_AVRCP_PASSTHROUGH_CMD_REQ;

typedef struct
{
    UINT32 feature;
}BT_AVRCP_FEATURE_RSP;

typedef union
{
    /* CT field */
    BT_AVRCP_TRACK_CHANGE track_change;
    BT_AVRCP_POS_CHANGE pos_change;
    BT_AVRCP_PLAY_STATUS_CHANGE play_status_change;
    BT_AVRCP_VOLUME_CHANGE volume_change;
    BT_AVRCP_FEATURE_RSP feature_rsp;

    /* TG field */
    BT_AVRCP_PASSTHROUGH_CMD_REQ passthrough_cmd_req;
    BT_AVRCP_SET_VOL_REQ set_vol_req;
}BT_AVRCP_EVENT_DATA;

typedef struct
{
    CHAR addr[MAX_BDADDR_LEN];
    BT_AVRCP_EVENT event;
    BT_AVRCP_EVENT_DATA data;
}BT_AVRCP_EVENT_PARAM;


typedef void (*BT_AVRCP_EVENT_HANDLE_CB)(BT_AVRCP_EVENT_PARAM *param);


typedef struct
{
    UINT32 song_pos;    /* unit: ms */
    UINT32 song_len;    /* unit: ms */
    BT_AVRCP_PLAY_STATUS play_status;
} BT_AVRCP_PLAYER_STATUS;

typedef struct
{
    BT_AVRCP_MEDIA_INFO media_info; /* song media information */
    UINT8 track[BT_AVRCP_UID_SIZE]; /* all 0, song selected, all FF, no song selected */
}BT_AVRCP_PLAYER_MEDIA_INFO;


#endif /*  _U_BT_MW_AVRCP_H_ */

