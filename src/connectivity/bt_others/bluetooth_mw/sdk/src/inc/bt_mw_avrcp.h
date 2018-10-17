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

/* FILE NAME:  bt_mw_avrcp.h
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
#ifndef BT_MW_AVRCP_H
#define BT_MW_AVRCP_H
/* INCLUDE FILE DECLARATIONS
 */

#include "u_bt_mw_common.h"
#include "u_bt_mw_avrcp.h"
#include "bt_mw_common.h"

/* NAMING CONSTANT DECLARATIONS
 */

/* State flag for Passthrough commands
*/
#define AVRC_STATE_PRESS    0
#define AVRC_STATE_RELEASE  1

/* Operation ID list for Passthrough commands
*/
#define AVRC_ID_SELECT      0x00    /* select */
#define AVRC_ID_UP          0x01    /* up */
#define AVRC_ID_DOWN        0x02    /* down */
#define AVRC_ID_LEFT        0x03    /* left */
#define AVRC_ID_RIGHT       0x04    /* right */
#define AVRC_ID_RIGHT_UP    0x05    /* right-up */
#define AVRC_ID_RIGHT_DOWN  0x06    /* right-down */
#define AVRC_ID_LEFT_UP     0x07    /* left-up */
#define AVRC_ID_LEFT_DOWN   0x08    /* left-down */
#define AVRC_ID_ROOT_MENU   0x09    /* root menu */
#define AVRC_ID_SETUP_MENU  0x0A    /* setup menu */
#define AVRC_ID_CONT_MENU   0x0B    /* contents menu */
#define AVRC_ID_FAV_MENU    0x0C    /* favorite menu */
#define AVRC_ID_EXIT        0x0D    /* exit */
#define AVRC_ID_0           0x20    /* 0 */
#define AVRC_ID_1           0x21    /* 1 */
#define AVRC_ID_2           0x22    /* 2 */
#define AVRC_ID_3           0x23    /* 3 */
#define AVRC_ID_4           0x24    /* 4 */
#define AVRC_ID_5           0x25    /* 5 */
#define AVRC_ID_6           0x26    /* 6 */
#define AVRC_ID_7           0x27    /* 7 */
#define AVRC_ID_8           0x28    /* 8 */
#define AVRC_ID_9           0x29    /* 9 */
#define AVRC_ID_DOT         0x2A    /* dot */
#define AVRC_ID_ENTER       0x2B    /* enter */
#define AVRC_ID_CLEAR       0x2C    /* clear */
#define AVRC_ID_CHAN_UP     0x30    /* channel up */
#define AVRC_ID_CHAN_DOWN   0x31    /* channel down */
#define AVRC_ID_PREV_CHAN   0x32    /* previous channel */
#define AVRC_ID_SOUND_SEL   0x33    /* sound select */
#define AVRC_ID_INPUT_SEL   0x34    /* input select */
#define AVRC_ID_DISP_INFO   0x35    /* display information */
#define AVRC_ID_HELP        0x36    /* help */
#define AVRC_ID_PAGE_UP     0x37    /* page up */
#define AVRC_ID_PAGE_DOWN   0x38    /* page down */
#define AVRC_ID_POWER       0x40    /* power */
#define AVRC_ID_VOL_UP      0x41    /* volume up */
#define AVRC_ID_VOL_DOWN    0x42    /* volume down */
#define AVRC_ID_MUTE        0x43    /* mute */
#define AVRC_ID_PLAY        0x44    /* play */
#define AVRC_ID_STOP        0x45    /* stop */
#define AVRC_ID_PAUSE       0x46    /* pause */
#define AVRC_ID_RECORD      0x47    /* record */
#define AVRC_ID_REWIND      0x48    /* rewind */
#define AVRC_ID_FAST_FOR    0x49    /* fast forward */
#define AVRC_ID_EJECT       0x4A    /* eject */
#define AVRC_ID_FORWARD     0x4B    /* forward */
#define AVRC_ID_BACKWARD    0x4C    /* backward */
#define AVRC_ID_ANGLE       0x50    /* angle */
#define AVRC_ID_SUBPICT     0x51    /* subpicture */
#define AVRC_ID_F1          0x71    /* F1 */
#define AVRC_ID_F2          0x72    /* F2 */
#define AVRC_ID_F3          0x73    /* F3 */
#define AVRC_ID_F4          0x74    /* F4 */
#define AVRC_ID_F5          0x75    /* F5 */
#define AVRC_ID_VENDOR      0x7E    /* vendor unique */
#define AVRC_KEYPRESSED_RELEASE 0x80

typedef enum
{
  BT_MW_AVRCP_ROLE_CT,
  BT_MW_AVRCP_ROLE_TG,
  BT_MW_AVRCP_ROLE_MAX
} BT_MW_AVRCP_ROLE;


enum
{
    /* device manager local device API events */
    BTMW_AVRCP_CONNECTED = BTMW_EVT_START(BTWM_ID_AVRCP),
    BTMW_AVRCP_DISCONNECTED,
    BTMW_AVRCP_FEATURE,
    BTMW_AVRCP_REG_EVENT_RSP,
    BTMW_AVRCP_TRACK_CHANGE,
    BTMW_AVRCP_PLAY_POS_CHANGE,
    BTMW_AVRCP_PLAY_STATUS_CHANGE,
    BTMW_AVRCP_VOLUME_CHANGE,

    /* TG event */
    BTMW_AVRCP_SET_VOLUME_REQ,
    BTMW_AVRCP_REG_EVENT_REQ,
    BTMW_AVRCP_GET_PLAYSTATUS_REQ,
    BTMW_AVRCP_GET_ELEMENT_ATTR_REQ,
    BTMW_AVRCP_PASSTHROUGH_CMD_REQ,

    BTMW_AVRCP_NOTIFY_APP,

    BTMW_AVRCP_MAX_EVT,
};  // event


typedef struct
{
    BT_AVRCP_REG_EVT_ID event_id;
    UINT32 param;
}BT_MW_AVRCP_REG_EVENT_REQ;

typedef struct
{
    BT_AVRCP_MEDIA_ATTR attrs[BT_AVRCP_MAX_ATTR_CNT];
    INT32 attr_cnt;
}BT_MW_AVRCP_ELEMENT_ATTR_REQ;


typedef struct
{
    UINT8 track_uid[BT_AVRCP_UID_SIZE];
}BT_AVRCP_REG_EVENT_TRACK_CHANGE;

typedef struct
{
    UINT32 song_pos;
}BT_AVRCP_REG_EVENT_POS_CHANGE;

typedef struct
{
    BT_AVRCP_PLAY_STATUS play_status;
}BT_AVRCP_REG_EVENT_PLAY_STATUS_CHANGE;

typedef struct
{
    BT_AVRCP_PLAYER_SETTING setting;
}BT_AVRCP_REG_EVENT_PLAYER_SETTING_CHANGE;

typedef struct
{
    BT_AVRCP_ADDR_PLAYER player;
}BT_AVRCP_REG_EVENT_ADDR_PLAYER_CHANGE;


typedef union
{
    BT_AVRCP_REG_EVENT_PLAY_STATUS_CHANGE play_status_change;
    BT_AVRCP_REG_EVENT_TRACK_CHANGE track_change;
    BT_AVRCP_REG_EVENT_POS_CHANGE pos_change;
    BT_AVRCP_REG_EVENT_PLAYER_SETTING_CHANGE play_setting_change;
    BT_AVRCP_REG_EVENT_ADDR_PLAYER_CHANGE addr_player_change;
} BT_AVRCP_EVENT_CHANGE_RSP;


/* MACRO FUNCTION DECLARATIONS
 */

#define BT_MW_AVRCP_CASE_RETURN_STR(const) case const: return #const;

/* DATA TYPE DECLARATIONS
 */


typedef struct BT_MW_AVRCP_MSG
{
    BT_MW_AVRCP_ROLE role;
    CHAR addr[MAX_BDADDR_LEN];
    UINT8 label;
    union
    {
        UINT32 feature;
        BT_AVRCP_SET_VOL_REQ set_vol_req;
        BT_AVRCP_VOLUME_CHANGE volume_change;
        BT_AVRCP_TRACK_CHANGE track_change;
        BT_AVRCP_PLAY_STATUS_CHANGE play_status_change;
        BT_AVRCP_POS_CHANGE pos_change;
        BT_AVRCP_PASSTHROUGH_CMD_REQ passthrough_cmd_req;
        BT_MW_AVRCP_ELEMENT_ATTR_REQ media_attr_req;
        BT_MW_AVRCP_REG_EVENT_REQ reg_event_req;
    }data;
}tBT_MW_AVRCP_MSG;

/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */

INT32 bt_mw_avrcp_send_passthrough_cmd(CHAR *addr,
    BT_AVRCP_CMD_TYPE cmd_type, BT_AVRCP_KEY_STATE key_state);

INT32 bt_mw_avrcp_send_vendor_unique_cmd(CHAR *addr, UINT8 key,
    BT_AVRCP_KEY_STATE key_state);

INT32 bt_mw_avrcp_change_volume(CHAR *addr, UINT8 volume);


INT32 bt_mw_avrcp_register_callback(BT_AVRCP_EVENT_HANDLE_CB avrcp_handle);

INT32 bt_mw_avrcp_update_player_status(BT_AVRCP_PLAYER_STATUS *play_status);
INT32 bt_mw_avrcp_update_player_media_info(BT_AVRCP_PLAYER_MEDIA_INFO *player_media_info);
INT32 bt_mw_avrcp_update_absolute_volume(const UINT8 abs_volume);

#endif /* End of BT_MW_AVRCP_H */


