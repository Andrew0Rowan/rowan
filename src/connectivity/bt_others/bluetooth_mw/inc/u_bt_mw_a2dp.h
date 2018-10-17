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


/* FILE NAME:  u_bt_mw_a2dpp.h
 * AUTHOR: Hongliang Hu
 * PURPOSE:
 *      It provides bluetooth a2dp structure to APP.
 * NOTES:
 */


#ifndef _U_BT_MW_A2DP_H_
#define _U_BT_MW_A2DP_H_

/*-----------------------------------------------------------------------------
                    include files
-----------------------------------------------------------------------------*/

#include "u_bt_mw_common.h"
/*-----------------------------------------------------------------------------
                    macros, defines, typedefs, enums
 ----------------------------------------------------------------------------*/

#define BT_A2DP_MAX_DEVICE_NUM  (10)



typedef enum
{
    BT_A2DP_STREAM_STATE_SUSPEND,   /* Stream is suspend */
    BT_A2DP_STREAM_STATE_PLAYING,   /* Stream is playing */
    BT_A2DP_STREAM_STATE_MAX,
} BT_A2DP_STREAM_STATE;


typedef enum
{
    BT_A2DP_ROLE_SRC,   /* Source role */
    BT_A2DP_ROLE_SINK,  /* Sink role   */
    BT_A2DP_ROLE_MAX
}BT_A2DP_ROLE;


typedef enum
{
    BT_A2DP_EVENT_CONNECTED,        /* A2DP connected             */
    BT_A2DP_EVENT_DISCONNECTED,     /* A2DP disconnected          */
    BT_A2DP_EVENT_CONNECT_TIMEOUT,  /* A2DP Connectin Timeout     */
    BT_A2DP_EVENT_STREAM_SUSPEND,   /* A2DP suspend               */
    BT_A2DP_EVENT_STREAM_START,     /* A2DP start                 */
    BT_A2DP_EVENT_CONNECT_COMING,   /* A2DP connect comming       */
    BT_A2DP_EVENT_PLAYER_EVENT,     /* A2DP Local playback status */
    BT_A2DP_EVENT_ROLE_CHANGED,     /* A2DP sink/src role change  */
    BT_A2DP_EVENT_MAX
} BT_A2DP_EVENT;

typedef enum
{
    BT_A2DP_DBG_SET_HW_AUDIO_LOG_LV,    /* modify BT audio device log level */
    BT_A2DP_DBG_SHOW_INFO,              /* modify BT audio device log level */
    BT_A2DP_DBG_MAX,
} BT_A2DP_DBG_FLAG;

typedef enum
{
    BT_A2DP_ALSA_PB_EVENT_STOP,         /* local player STOPED      */
    BT_A2DP_ALSA_PB_EVENT_STOP_FAIL,    /* local player stop fail   */
    BT_A2DP_ALSA_PB_EVENT_START,        /* local player STARTED     */
    BT_A2DP_ALSA_PB_EVENT_START_FAIL,   /* local player start fail  */
    BT_A2DP_ALSA_PB_EVENT_DATA_COME,    /* remote audio data come from remote SRC device */
    BT_A2DP_ALSA_PB_EVENT_MAX
} BT_A2DP_PLAYER_EVENT;

typedef enum
{
    BT_A2DP_CODEC_TYPE_SBC,
    BT_A2DP_CODEC_TYPE_MP3,
    BT_A2DP_CODEC_TYPE_AAC,
    BT_A2DP_CODEC_TYPE_LDAC,
} BT_A2DP_CODEC_TYPE;

typedef enum
{
    BT_A2DP_CHANNEL_MODE_MONO,
    BT_A2DP_CHANNEL_MODE_DUAL,
    BT_A2DP_CHANNEL_MODE_STEREO,
    BT_A2DP_CHANNEL_MODE_JOINT,
}BT_A2DP_CHANNEL_MODE;

typedef enum
{
    BT_A2DP_AAC_OBJ_TYPE_MPEG2_LC,
    BT_A2DP_AAC_OBJ_TYPE_MPEG4_LC,
    BT_A2DP_AAC_OBJ_TYPE_MPEG4_LTP,
    BT_A2DP_AAC_OBJ_TYPE_MPEG4_SCA,
}BT_A2DP_AAC_OBJ_TYPE;


typedef struct
{
    CHAR addr[MAX_BDADDR_LEN];      /* history connected device address */
    CHAR name[MAX_NAME_LEN];
    BT_A2DP_ROLE role;
}BT_A2DP_CONNECTED_DEVICE;

typedef struct
{
    UINT32                   dev_num;   /* history A2DP connected device count */
    BT_A2DP_CONNECTED_DEVICE dev[BT_A2DP_MAX_DEVICE_NUM];
}BT_A2DP_DEVICE_LIST;

typedef struct
{
    UINT32  samp_freq;                      /* Sampling frequency */
    BT_A2DP_CHANNEL_MODE ch_mode;           /* Channel mode */
    UINT8   block_len;                      /* Block length */
    UINT8   num_subbands;                   /* Number of subbands */
    UINT8   alloc_mthd;                     /* 0-SNR, 1-Loundness */
    UINT8   max_bitpool;                    /* Maximum bitpool */
    UINT8   min_bitpool;                    /* Minimum bitpool */
} BT_A2DP_SBC_CONF;

typedef struct
{
    UINT8 layer;                    /* 0-layer1, 1-layer2, 2-layer3 */
    UINT8 crc;                      /* 0-protection not support, 1-support */
    BT_A2DP_CHANNEL_MODE ch_mode;   /* 0-mono, 1-dual, 2-stereo, 3-joint stereo */
    UINT8 mpf;                      /* 0-mpf-1, 1-mpf-2 */
    UINT32 sample_rate;             /* sample rate */
    BOOL vbr;                       /* vbr support */
    UINT32 bit_rate;                /* bit rate, unit: Kbps */
} BT_A2DP_MP3_CONF;                 /* all MPEG-1,2 Audio */

typedef struct
{
    BT_A2DP_AAC_OBJ_TYPE object_type;
    UINT32 samp_freq;
    UINT8  channels;
    BOOL   vbr;
    UINT32 bitrate;
} BT_A2DP_AAC_CONF;

typedef struct
{
    UINT32 sample_rate;
    BT_A2DP_CHANNEL_MODE channel_mode;
}BT_A2DP_LDAC_CONF;


typedef union
{
    BT_A2DP_SBC_CONF sbc_conf;
    BT_A2DP_MP3_CONF mp3_conf;
    BT_A2DP_AAC_CONF aac_conf;
    BT_A2DP_LDAC_CONF ldac_cap;
} BT_A2DP_CODEC_CONF;

typedef struct
{
    BT_A2DP_CODEC_TYPE codec_type; // SBC, MP3,AAC,etc
    BT_A2DP_CODEC_CONF codec_conf;
} BT_A2DP_CONF;


typedef union
{
    INT32 hw_audio_log_lv;  /* BT audio device log level */
} BT_A2DP_DBG_PARAM;

/* when APP will register a Player into BT MW, it is used to report player
 * status to MW and MW will report it to APP
 */
typedef VOID (*BT_A2DP_PLAYER_EVENT_CB)(BT_A2DP_PLAYER_EVENT event);

/* prepare player, like: open audio device, set playback parameter
 */
typedef INT32 (*BT_A2DP_PLAYER_START)(INT32 trackFreq, INT32 channelType);

/* play player, after this, BT streaming data can be pushed into player.
 */
typedef INT32 (*BT_A2DP_PLAYER_PLAY)(VOID);

/* pause player
 */
typedef INT32 (*BT_A2DP_PLAYER_PAUSE)(VOID);

/* init player, when A2DP sink is enable, this will be called once.
 */
typedef INT32 (*BT_A2DP_PLAYER_INIT)(BT_A2DP_PLAYER_EVENT_CB event_cb);

/* deinit player, when A2DP sink is disable, this will be called once.
 */
typedef INT32 (*BT_A2DP_PLAYER_DEINIT)(VOID);

/* stop player, like: close audio device
 */
typedef INT32 (*BT_A2DP_PLAYER_STOP)(VOID);

/* push A2DP streaming data into player to playback
 */
typedef INT32 (*BT_A2DP_PLAYER_WRITE)(VOID *audioBuffer, INT32 bufferLen);

/* adjust player buffer time
 */
typedef INT32 (*BT_A2DP_PLAYER_ADJUST_BUFFER_TIME)(UINT32 buffer_time);

/* app can implement a BT player and register it into BT MW.
 * player is used when it's a A2DP sink device.
 */
typedef struct
{
    BT_A2DP_PLAYER_INIT init;
    BT_A2DP_PLAYER_DEINIT deinit;
    BT_A2DP_PLAYER_START start;
    BT_A2DP_PLAYER_STOP stop;
    BT_A2DP_PLAYER_PLAY play;
    BT_A2DP_PLAYER_PAUSE pause;
    BT_A2DP_PLAYER_WRITE write;
    BT_A2DP_PLAYER_ADJUST_BUFFER_TIME adjust_buf_time;
    UINT8 started;
}BT_A2DP_PLAYER;

/* init uploader.
 * freq: sample rate
 * channel: channel number
 */
typedef INT32 (*BT_A2DP_UPLOADER_INIT)(INT32 freq, INT32 channel);

/* start uploader, it is used to ask uploader to get PCM data and send out to
 * BT MW.
 * delay_ms: after this time, uploader begin to get PCM data.
 */
typedef INT32 (*BT_A2DP_UPLOADER_START)(INT32 delay_ms);

/* stop uploader, it is used to stop get PCM data and sending.
 */
typedef INT32 (*BT_A2DP_UPLOADER_STOP)(VOID);

/* deinit uploader
 */
typedef INT32 (*BT_A2DP_UPLOADER_DEINIT)(VOID);

/* app can implement a BT uploader and register it into BT MW.
 * uploader is used when it's a A2DP Source device.
 */
typedef struct
{
    BT_A2DP_UPLOADER_INIT init;
    BT_A2DP_UPLOADER_START start;
    BT_A2DP_UPLOADER_STOP stop;
    BT_A2DP_UPLOADER_DEINIT deinit;
}BT_A2DP_UPLOADER;

/* when report a BT_A2DP_EVENT_CONNECTED, app can get the audio parameter.
 */
typedef struct
{
    BT_A2DP_ROLE local_role;
    UINT32 sample_rate;
    UINT32 channel_num;
    BT_A2DP_CONF config;
} BT_A2DP_EVENT_CONNECTED_DATA;


/* when disable a role, it may be pending because of connection exist, so
 * BT MW will disconnect all connection and then report a event to app.
 */
typedef struct
{
    BT_A2DP_ROLE role;
    BOOL         enable;
} BT_A2DP_EVENT_ROLE_CHANGE_DATA;

/* when report a BT_A2DP_EVENT_ to app, some event will report some data
 */
typedef union
{
    BT_A2DP_EVENT_CONNECTED_DATA connected_data;  /* connected data */
    BT_A2DP_PLAYER_EVENT player_event;            /* player event   */
    BT_A2DP_EVENT_ROLE_CHANGE_DATA role_change;   /* change role  */
} BT_A2DP_EVENT_DATA;

/* when report a BT_A2DP_EVENT_ to app, this paramter is passed to APP
 */
typedef struct
{
    BT_A2DP_EVENT event;        /* event id */
    CHAR addr[MAX_BDADDR_LEN];  /* which device report this event */
    BT_A2DP_EVENT_DATA data;    /* event data */
}BT_A2DP_EVENT_PARAM;

/* app should provide a A2DP event handler and register to BT MW
 */
typedef void (*BT_A2DP_EVENT_HANDLE_CB)(BT_A2DP_EVENT_PARAM *param);

#endif /*  _U_BT_MW_A2DP_H_ */

