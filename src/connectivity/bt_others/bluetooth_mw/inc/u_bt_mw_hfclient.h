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

#ifndef _U_BT_MW_HFCLIENT_H_
#define _U_BT_MW_HFCLIENT_H_

#include "u_bt_mw_common.h"

#define HFCLIENT_OPERATOR_NAME_LEN 16
#define HFCLIENT_NUMBER_LEN        32
#define HFCLIENT_CPBR_NUMBER_LEN   50           /*phonebook entry:<number> max length*/
#define HFCLIENT_CPBR_NAME_LEN     50           /*phonebook entry:<text> max length*/
#define HFCLIENT_PB_STORAGE_COUNT  12

typedef enum
{
    BT_HFCLIENT_ENABLE = 0,
    BT_HFCLIENT_DISABLE,
} BT_HFCLIENT_ABILITY_STATE_T;

typedef enum
{
    BT_HFCLIENT_CONNECTION_STATE_DISCONNECTED = 0,
    BT_HFCLIENT_CONNECTION_STATE_CONNECTING,
    BT_HFCLIENT_CONNECTION_STATE_CONNECTED,
    BT_HFCLIENT_CONNECTION_STATE_SLC_CONNECTED,
    BT_HFCLIENT_CONNECTION_STATE_DISCONNECTING,
} BT_HFCLIENT_CONNECTION_STATE_T;

typedef enum
{
    BT_HFCLIENT_AUDIO_STATE_DISCONNECTED = 0,
    BT_HFCLIENT_AUDIO_STATE_CONNECTING,
    BT_HFCLIENT_AUDIO_STATE_CONNECTED,
    BT_HFCLIENT_AUDIO_STATE_CONNECTED_MSBC,
} BT_HFCLIENT_AUDIO_STATE_T;

typedef enum
{
    BT_HFCLIENT_VR_STATE_STOPPED = 0,
    BT_HFCLIENT_VR_STATE_STARTED
} BT_HFCLIENT_VR_STATE_T;

typedef enum
{
    BT_HFCLIENT_VOLUME_TYPE_SPK = 0,
    BT_HFCLIENT_VOLUME_TYPE_MIC
} BT_HFCLIENT_VOLUME_TYPE_T;

typedef enum
{
    BT_HFCLIENT_NETWORK_STATE_NOT_AVAILABLE = 0,
    BT_HFCLIENT_NETWORK_STATE_AVAILABLE
} BT_HFCLIENT_NETWORK_STATE_T;

typedef enum
{
    BT_HFCLIENT_SERVICE_TYPE_HOME = 0,
    BT_HFCLIENT_SERVICE_TYPE_ROAMING
} BT_HFCLIENT_SERVICE_TYPE_T;

typedef enum
{
    BT_HFCLIENT_CALL_STATE_ACTIVE = 0,
    BT_HFCLIENT_CALL_STATE_HELD,
    BT_HFCLIENT_CALL_STATE_DIALING,
    BT_HFCLIENT_CALL_STATE_ALERTING,
    BT_HFCLIENT_CALL_STATE_INCOMING,
    BT_HFCLIENT_CALL_STATE_WAITING,
    BT_HFCLIENT_CALL_STATE_HELD_BY_RESP_HOLD,
} BT_HFCLIENT_CALL_STATE_T;

typedef enum
{
    BT_HFCLIENT_CALL_NO_CALLS_IN_PROGRESS = 0,
    BT_HFCLIENT_CALL_CALLS_IN_PROGRESS
} BT_HFCLIENT_CALL_T;

typedef enum
{
    BT_HFCLIENT_CALLSETUP_NONE = 0,
    BT_HFCLIENT_CALLSETUP_INCOMING,
    BT_HFCLIENT_CALLSETUP_OUTGOING,
    BT_HFCLIENT_CALLSETUP_ALERTING
} BT_HFCLIENT_CALLSETUP_T;

typedef enum
{
    BT_HFCLIENT_CALLHELD_NONE = 0,
    BT_HFCLIENT_CALLHELD_HOLD_AND_ACTIVE,
    BT_HFCLIENT_CALLHELD_HOLD,
} BT_HFCLIENT_CALLHELD_T;

typedef enum
{
    BT_HFCLIENT_RESP_AND_HOLD_HELD = 0,
    BT_HFCLIENT_RESP_AND_HOLD_ACCEPT,
    BT_HFCLIENT_RESP_AND_HOLD_REJECT,
} BT_HFCLIENT_RESP_AND_HOLD_T;

typedef enum
{
    BT_HFCLIENT_CALL_DIRECTION_OUTGOING = 0,
    BT_HFCLIENT_CALL_DIRECTION_INCOMING
} BT_HFCLIENT_CALL_DIRECTION_T;

typedef enum
{
    BT_HFCLIENT_CALL_MPTY_TYPE_SINGLE = 0,
    BT_HFCLIENT_CALL_MPTY_TYPE_MULTI
} BT_HFCLIENT_CALL_MPTY_TYPE_T;

typedef enum
{
    BT_HFCLIENT_CMD_COMPLETE_OK = 0,
    BT_HFCLIENT_CMD_COMPLETE_ERROR,
    BT_HFCLIENT_CMD_COMPLETE_ERROR_NO_CARRIER,
    BT_HFCLIENT_CMD_COMPLETE_ERROR_BUSY,
    BT_HFCLIENT_CMD_COMPLETE_ERROR_NO_ANSWER,
    BT_HFCLIENT_CMD_COMPLETE_ERROR_DELAYED,
    BT_HFCLIENT_CMD_COMPLETE_ERROR_BLACKLISTED,
    BT_HFCLIENT_CMD_COMPLETE_ERROR_CME
} BT_HFCLIENT_CMD_COMPLETE_TYPE_T;

typedef enum
{
    BT_HFCLIENT_CALL_ACTION_CHLD_0 = 0,
    BT_HFCLIENT_CALL_ACTION_CHLD_1,
    BT_HFCLIENT_CALL_ACTION_CHLD_2,
    BT_HFCLIENT_CALL_ACTION_CHLD_3,
    BT_HFCLIENT_CALL_ACTION_CHLD_4,
    BT_HFCLIENT_CALL_ACTION_CHLD_1x,
    BT_HFCLIENT_CALL_ACTION_CHLD_2x,
    BT_HFCLIENT_CALL_ACTION_ATA,
    BT_HFCLIENT_CALL_ACTION_CHUP,
    BT_HFCLIENT_CALL_ACTION_BTRH_0,
    BT_HFCLIENT_CALL_ACTION_BTRH_1,
    BT_HFCLIENT_CALL_ACTION_BTRH_2,
} BT_HFCLIENT_CALL_ACTION_T;

typedef enum
{
    BT_HFCLIENT_SERVICE_UNKNOWN = 0,
    BT_HFCLIENT_SERVICE_VOICE,
    BT_HFCLIENT_SERVICE_FAX
} BT_HFCLIENT_SUBSCRIBER_SERVICE_TYPE_T;

typedef enum
{
    BT_HFCLIENT_IN_BAND_RINGTONE_NOT_PROVIDED = 0,
    BT_HFCLIENT_IN_BAND_RINGTONE_PROVIDED,
} BT_HFCLIENT_IN_BAND_RING_STATE_T;

/*****************************************************************************
**  Callback Event to app
*****************************************************************************/
typedef enum
{
    BTAPP_HFCLIENT_CONNECTION_CB_EVT,        /*HFP Connection status*/
    BTAPP_HFCLIENT_AUDIO_CONNECTION_CB_EVT,  /*HFP audio connection status*/
    BTAPP_HFCLIENT_BVRA_CB_EVT,              /*AG changed voice recognition setting*/
    BTAPP_HFCLIENT_IND_SERVICE_CB_EVT,       /*network status*/
    BTAPP_HFCLIENT_IND_ROAM_CB_EVT,          /*network roaming status*/
    BTAPP_HFCLIENT_IND_SIGNAL_CB_EVT,        /*network signal strength*/
    BTAPP_HFCLIENT_IND_BATTCH_CB_EVT,        /*battery level*/
    BTAPP_HFCLIENT_COPS_CB_EVT,              /*current operator name*/
    BTAPP_HFCLIENT_IND_CALL_CB_EVT,          /*call*/
    BTAPP_HFCLIENT_IND_CALLSETUP_CB_EVT,     /*callsetup*/
    BTAPP_HFCLIENT_IND_CALLHELD_CB_EVT,      /*callheld*/
    BTAPP_HFCLIENT_BTRH_CB_EVT,              /*bluetooth response and hold event*/
    BTAPP_HFCLIENT_CLIP_CB_EVT,              /*Calling line identification event*/
    BTAPP_HFCLIENT_CCWA_CB_EVT,              /*Call waiting notification*/
    BTAPP_HFCLIENT_CLCC_CB_EVT,              /*current call event*/
    BTAPP_HFCLIENT_VGM_VGS_CB_EVT,           /*volume change*/
    BTAPP_HFCLIENT_CMD_COMPLETE_CB_EVT,      /*command complete*/
    BTAPP_HFCLIENT_CNUM_CB_EVT,              /*subscriber information event*/
    BTAPP_HFCLIENT_BSIR_CB_EVT,              /*in-band ring tone setting changed event*/
    BTAPP_HFCLIENT_BINP_CB_EVT,              /*last voice tag number*/
    BTAPP_HFCLIENT_RING_IND_CB_EVT,          /*HF Client ring indication */
    BTAPP_HFCLIENT_ABILITY_CB_EVT,           /*HFP interface enable/disable event*/
    BTAPP_HFCLIENT_CPBR_ENTRY_EVT,               /*Callback phonebook entry to APP*/
    BTAPP_HFCLIENT_CPBR_READY_EVT,               /*Notify App that phonebook entries of mw ready*/
    BTAPP_HFCLIENT_CPBR_DONE_EVT,                /*Notify App that Read phonebook entries from mw Done*/
}BTAPP_HFCLIENT_CB_EVENT;

/*****************************************************************************
**  Callback Data to app
*****************************************************************************/
typedef struct
{
    BT_HFCLIENT_ABILITY_STATE_T state;
}BT_HFCLIENT_ABILITY_CB_DATA_T;

typedef struct
{
    BT_HFCLIENT_CONNECTION_STATE_T  state;
    UINT32                          peer_feat;
    UINT32                          chld_feat;
    CHAR                            addr[MAX_BDADDR_LEN];
}BT_HFCLIENT_CONNECTION_CB_DATA_T;

typedef struct
{
    BT_HFCLIENT_AUDIO_STATE_T       state;
    CHAR                            addr[MAX_BDADDR_LEN];
}BT_HFCLIENT_AUDIO_CONNECTION_CB_DATA_T;


typedef struct
{
    BT_HFCLIENT_VR_STATE_T          vr_state;
}BT_HFCLIENT_BVRA_CB_DATA_T;


typedef struct
{
    BT_HFCLIENT_NETWORK_STATE_T     network_state;
}BT_HFCLIENT_IND_SERVICE_CB_DATA_T;


typedef struct
{
    BT_HFCLIENT_SERVICE_TYPE_T      service_type;
}BT_HFCLIENT_IND_ROAM_CB_DATA_T;

typedef struct
{
    int                             signal_strength;
}BT_HFCLIENT_IND_SIGNAL_CB_DATA_T;


typedef struct
{
    int                             battery_level;
}BT_HFCLIENT_IND_BATTCH_CB_DATA_T;


typedef struct
{
    CHAR                            operator_name[HFCLIENT_OPERATOR_NAME_LEN + 1];
}BT_HFCLIENT_COPS_CB_DATA_T;


typedef struct
{
    BT_HFCLIENT_CALL_T              call;
}BT_HFCLIENT_IND_CALL_CB_DATA_T;


typedef struct
{
    BT_HFCLIENT_CALLSETUP_T         callsetup;
}BT_HFCLIENT_IND_CALLSETUP_CB_DATA_T;


typedef struct
{
    BT_HFCLIENT_CALLHELD_T          callheld;
}BT_HFCLIENT_IND_CALLHELD_CB_DATA_T;

typedef struct
{
    BT_HFCLIENT_RESP_AND_HOLD_T     resp_and_hold;
}BT_HFCLIENT_BTRH_CB_DATA_T;


typedef struct
{
    CHAR                      number[HFCLIENT_NUMBER_LEN + 1];  /*phone number*/
}BT_HFCLIENT_CLIP_CB_DATA_T;


typedef struct
{
    CHAR                      number[HFCLIENT_NUMBER_LEN + 1];  /*phone number*/
}BT_HFCLIENT_CCWA_CB_DATA_T;


typedef struct
{
    int index;
    BT_HFCLIENT_CALL_DIRECTION_T    dir;
    BT_HFCLIENT_CALL_STATE_T        state;
    BT_HFCLIENT_CALL_MPTY_TYPE_T    mpty;
    CHAR                            number[HFCLIENT_NUMBER_LEN + 1];  /*phone number*/
}BT_HFCLIENT_CLCC_CB_DATA_T;


typedef struct
{
    BT_HFCLIENT_VOLUME_TYPE_T               type;
    int volume;
}BT_HFCLIENT_VGM_VGS_CB_DATA_T;


typedef struct
{
    BT_HFCLIENT_CMD_COMPLETE_TYPE_T         type;
    int cme;
}BT_HFCLIENT_CMD_COMPLETE_CB_DATA_T;

typedef struct
{
    CHAR                                    number[HFCLIENT_NUMBER_LEN + 1];  /*phone number*/
    BT_HFCLIENT_SUBSCRIBER_SERVICE_TYPE_T   type;
}BT_HFCLIENT_CNUM_CB_DATA_T;

typedef struct
{
    BT_HFCLIENT_IN_BAND_RING_STATE_T        state;
}BT_HFCLIENT_BSIR_CB_DATA_T;

typedef struct
{
    CHAR                                    number[HFCLIENT_NUMBER_LEN + 1];  /*phone number*/
}BT_HFCLIENT_BINP_CB_DATA_T;

/*Phonebook entry data callback from BTA*/
typedef struct
{
    UINT16                     index;                                 /*entry index*/
    char                       number[HFCLIENT_CPBR_NUMBER_LEN + 1];  /*phone number*/
    UINT16                     type;                                  /*region type*/
    char                       name[HFCLIENT_CPBR_NAME_LEN + 1];      /*contact name*/
}BT_HFCLIENT_PHONEBOOK_ENTRY_T;

/*Phonebook entry data notify to BT App*/
typedef struct
{
    CHAR                        number[HFCLIENT_CPBR_NUMBER_LEN + 1];
    CHAR                        name[HFCLIENT_CPBR_NAME_LEN + 1];
}BT_HFCLIENT_PB_ENTRY_APP_DATA_T;

typedef struct
{
    UINT8                                   storage_lookup[HFCLIENT_PB_STORAGE_COUNT];
}BT_HFCLIENT_CPBS_CB_DATA_T;

typedef struct
{
    int                                     idx_max;
}BT_HFCLIENT_CPBR_COUNT_CB_DATA_T;

typedef struct
{
    BT_HFCLIENT_PHONEBOOK_ENTRY_T           pb_entry;
}BT_HFCLIENT_CPBR_ENTRY_CB_DATA_T;


typedef union
{
    BT_HFCLIENT_ABILITY_CB_DATA_T                  ability_cb;
    BT_HFCLIENT_CONNECTION_CB_DATA_T               connect_cb;
    BT_HFCLIENT_AUDIO_CONNECTION_CB_DATA_T         auido_connect_cb;
    BT_HFCLIENT_BVRA_CB_DATA_T                     bvra_cb;
    BT_HFCLIENT_IND_SERVICE_CB_DATA_T              service_cb;
    BT_HFCLIENT_IND_ROAM_CB_DATA_T                 roam_cb;
    BT_HFCLIENT_IND_SIGNAL_CB_DATA_T               signal_cb;
    BT_HFCLIENT_IND_BATTCH_CB_DATA_T               battery_cb;
    BT_HFCLIENT_COPS_CB_DATA_T                     cops_cb;
    BT_HFCLIENT_IND_CALL_CB_DATA_T                 call_cb;
    BT_HFCLIENT_IND_CALLSETUP_CB_DATA_T            callsetup_cb;
    BT_HFCLIENT_IND_CALLHELD_CB_DATA_T             callheld_cb;
    BT_HFCLIENT_BTRH_CB_DATA_T                     btrh_cb;
    BT_HFCLIENT_CLIP_CB_DATA_T                     clip_cb;
    BT_HFCLIENT_CCWA_CB_DATA_T                     ccwa_cb;
    BT_HFCLIENT_CLCC_CB_DATA_T                     clcc_cb;
    BT_HFCLIENT_VGM_VGS_CB_DATA_T                  volume_cb;
    BT_HFCLIENT_CMD_COMPLETE_CB_DATA_T             cmd_complete_cb;
    BT_HFCLIENT_CNUM_CB_DATA_T                     cnum_cb;
    BT_HFCLIENT_BSIR_CB_DATA_T                     bsir_cb;
    BT_HFCLIENT_BINP_CB_DATA_T                     binp_cb;
    BT_HFCLIENT_CPBS_CB_DATA_T                     cpbs_cb;
    BT_HFCLIENT_CPBR_COUNT_CB_DATA_T               cpbr_count_cb;
    BT_HFCLIENT_CPBR_ENTRY_CB_DATA_T               cpbr_entry_cb;
    BT_HFCLIENT_PB_ENTRY_APP_DATA_T                pb_entry_app;
}BT_HFCLIENT_CB_DATA;


typedef struct
{
    BTAPP_HFCLIENT_CB_EVENT   event;
    BT_HFCLIENT_CB_DATA       data;
}BT_HFCLIENT_EVENT_PARAM;

typedef void (*BT_HFCLIENT_EVENT_HANDLE_CB)(BT_HFCLIENT_EVENT_PARAM *param);

#endif /*  _U_BT_MW_HFCLIENT_H_ */
