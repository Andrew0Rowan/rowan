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
 * MediaTek Inc. (C) 2014. All rights reserved.                                
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


#ifndef _BLUETOOTH_HFP_H
#define _BLUETOOTH_HFP_H

#include "u_common.h"
#include "u_handle.h"
#include "u_timerd.h"
#include "u_bt_mw_hfclient.h"

#include "u_alsa_interface.h"
#include "u_ringbuf.h"
#include "u_msglist.h"
#include "u_datalist.h"
#include <alsa/asoundlib.h>

#define DBG_INIT_LEVEL_APP_BT_HFP       (DBG_LEVEL_API | DBG_LEVEL_ERROR | DBG_LEVEL_INFO | DBG_LAYER_APP)
#undef  DBG_LEVEL_MODULE
#define DBG_LEVEL_MODULE                _bt_hfp_get_dbg_level()

#define BT_HFP_FUNC_ENTRY()   DBG_INFO((BTHFP_TAG"---Enter:Func=%s, Line=%d---\r\n",__FUNCTION__, __LINE__))
#define BT_HFP_FUNC_EXIT()    DBG_INFO((BTHFP_TAG"---Exit:Func=%s, Line=%d---\r\n",__FUNCTION__, __LINE__))

#define BT_HFP_CHK_FAIL_GOTO(_func, _ret, _label)   \
do{if (_ret != 0){DBG_ERROR((BTHFP_TAG"Err:"#_func" return err: ret=%d.\r\n", _ret));goto _label;}}while(0)
#define BT_HFP_CHK_FAIL_RET(_func, _ret, _ret_on_err)   \
do{if (_ret != 0){DBG_ERROR((BTHFP_TAG"Err:"#_func" return err: ret=%d.\r\n", _ret));return _ret_on_err;}}while(0)
#define BT_HFP_CHK_FAIL(_func, _ret)    \
do{if (_ret != 0){DBG_ERROR((BTHFP_TAG"Err:"#_func" return err: ret=%d.\r\n", _ret));}}while(0)

#define BT_AUD_HANDLE_VOLUME_KEY        0   //handle volume key by itself

#define BT_HFP_NAME                     "BLUETOOTH_HFP"
#define BT_HFP_STR_MAX_LEN              60

#define BT_HFP_DEVICE_NAME    "sub0"
#define BT_HFP_RING_PATH      "/data/misc/public/bluetooth_hfp/Sleep_Away.mp3"
#define BTHFP_TAG                       "<BT_HFP> "

#define ALSA_HW_PARAM         60000
#define HFP_ENC_DATA_BUF    ((MIC_IN_SAMPLERATE*2*2*0.03)+CVSD_ENC_INPUT_SAMPLE*2)
#define MIC_IN_SAMPLERATE     16000
#define MIC_IN_CHANNLE        8
#define MIC_IN_BITWIDTH       16

#define HFP_INPUT_SAMPLE 120
#define HFP_MIC_IN_CHANNLE        8

#define BT_HFP_48k_SAMPLERATE      48000

#define HFP_DEC_DATA_BUF    1920
#define BT_HFP_SAMPLERATE     16000
#define BT_HFP_CHANNLE        1
#define BT_HFP_BITWIDTH       16
//for 6627, CVSD enc/dec
#define CVSD_ENC_INPUT_SAMPLE 480
#define CVSD_ENC_OUTPUT_BYTE 60
#define CVSD_DEC_INPUT_BYTE 32
#define CVSD_VALID_DEC_INPUT_BYTE 30
#define CVSD_DEC_OUTPUT_SAMPLE 240
#define CVSD_TEMP_INPUT_SAMPLE 960
#define CVSD_PLC_SAMPLERATE 64000
//for 6627, mSBC enc/dec
#define MSBC_ENC_INPUT_SAMPLE 120
#define MSBC_ENC_OUTPUT_BYTE 57
#define MSBC_DEC_INPUT_BYTE 64
#define MSBC_VALID_DEC_INPUT_BYTE 57
#define MSBC_DEC_OUTPUT_SAMPLE 120
#define MSBC_PLC_SAMPLERATE 16000
//for bt dirver in path of 6630
#define MSBC_OUTPUT_SAMPLE 720
#define CVSD_OUTPUT_SAMPLE 1440
#define BT_READ_MSBC_BYTE 480
#define BT_READ_CVSD_BYTE 480
//for mic in path of 6630
#define MSBC_INPUT_BYTE 480
#define MSBC_INPUT_SAMPLE 240
#define CVSD_INPUT_BYTE 480
#define CVSD_INPUT_SAMPLE 480
//alsa node
#define MIC_IN_DEVICE_NAME     "hw:0,1"
#define MAIN_PCM_DEVICE_NAME    "main"

#ifdef WIFI_CHIP_6630
#define BT_HFP_WRITE_DEVICE     "hw:0,6"
#define BT_HFP_READ_DEVICE      "hw:0,8"
#endif
#ifdef WIFI_CHIP_7668
#define BT_HFP_WRITE_DEVICE     "hw:0,9"
#define BT_HFP_READ_DEVICE      "hw:0,3"
#endif
#ifdef CONSYS_WIFI_CHIP
#define BT_HFP_WRITE_DEVICE     "hw:0,5"
#define BT_HFP_READ_DEVICE      "hw:0,4"
#endif

typedef enum
{
    MSG_DATA_SEND,
    MSG_EXIT,
};

typedef enum{
	HFP_CALLSETUP_NONE = 0,
    HFP_CALLSETUP_INCOMING,
    HFP_CALLSETUP_OUTGOING,
    HFP_CALLSETUP_ALERTING
} BT_HFP_CALLSETUP_T;

typedef struct
{
    pthread_cond_t t_mic_read_cond;
    pthread_cond_t t_bt_driver_read_cond;
    pthread_mutex_t t_mutex;
} TDM_IN_PLAY_COND_T;

/* application structure */
typedef struct _BT_HFP_OBJ_T
{
    HANDLE_T    h_app;                      //app handle
    BOOL        b_app_init_ok;              //app init ok flag
    
    TDM_IN_PLAY_COND_T t_cond;
    VOID*                h_player;         //play ring
}BT_HFP_OBJ_T;

/*hfp status*/
typedef struct{
    BOOL b_bt_hfp_init;                 //init only do one time
    BOOL b_bt_hfp_connect;              //HFP connect
    BOOL b_bt_hfp_audio_connect;        //audio connection
    BT_HFP_CALLSETUP_T b_bt_hfp_call_setup;           //call_setup
    BOOL b_bt_hfp_non_inband;          //non_inband ring
    BOOL b_bt_hfp_alert;              //hfp alert
    BOOL b_bt_hfp_calls_in_process;    //call in_process
    BOOL b_bt_hfp_cmd_complete;        // cmd complete
    BOOL b_bt_hfp_query_calls;        // query calls 
    BOOL b_bt_hfp_media_play;         //alert media play
    BOOL b_bt_hfp_media_prepared;     //media prepare
    BOOL b_bt_hfp_media_stop;
    BOOL b_bt_hfp_player_thread_create;
    BOOL b_bt_hfp_call_ongoing;            //call ongoing
    BOOL b_bt_hfp_mic_spk_thread_creat;    //mic_spk thread create
    BOOL b_bt_hfp_play;
//    BT_AUD_BT_STA_T e_bt_sta;       //bluetooth audio status      
}BT_HFP_STATUS_T;

//the struct of BT audio applicatio's message
typedef struct _BT_HFP_MSG_T
{
    UINT32          ui4_msg_type;
	UINT32          ui4_msg_id;
    UINT32          ui4_data1;
    UINT32          ui4_data2;
    UINT32          ui4_data3;
	char            ui4_str[BT_HFP_STR_MAX_LEN];
} BT_HFP_MSG_T;

enum{
    BT_HFP_PTE_MSG_HFP=1,          //bluetooth_hfp  msg
    
    BT_HFP_PTE_MSG_MAX
};

/*  the struct of call cation */  
typedef struct{
    CHAR* action_str;
    BT_HFCLIENT_CALL_ACTION_T ui4_call_action;
}BT_HFP_CALL_ACTION_T;


/*hfp cbk msg type*/
typedef enum{
    BT_HFP_MSG_GAP_EVENT = 0,       //GAP event
    BT_HFP_MSG_VR_CMD ,             //VR CMD
    BT_HFP_MSG_NETWORK_STATE,          //HFP network state
    BT_HFP_MSG_NETWORK_ROAMING,
    BT_HFP_MSG_NETWORK_SIGNAL,
    BT_HFP_MSG_BATTERY_LEVEL,          //5 
    BT_HFP_MSG_CURRENT_OPERATOR,  
    BT_HFP_MSG_CALL,
	BT_HFP_MSG_CALLSETUP,
	BT_HFP_MSG_CALLHELD,
	BT_HFP_MSG_RESP_AND_HOLD,          //10
	BT_HFP_MSG_CLIP,
	BT_HFP_MSG_CALL_WAITING,      
	BT_HFP_MSG_CURRENT_CALLS,
	BT_HFP_MSG_VOLUME_CHANGE,
	BT_HFP_MSG_NETWORKCMD_COMPLETE,    //15
	BT_HFP_MSG_SUBSCRIBER_INFO,
	BT_HFP_MSG_INBAND_RINGTONE,   
	BT_HFP_MSG_LAST_VOICE_TAG_NUMBER,
	BT_HFP_MSG_RING_INDICATION,
	BT_HFP_MSG_CBK_STATUS_PRINT,

    BT_HFP_MSG_MAX
}BT_HFP_CB_MSG_TYPE_T;

#if CONFIG_SUPPORT_PHONE_BOOK
//the struct of CPBR
typedef struct _BT_HFP_PB_INFO_ENTIRES_T
{
    UINT32          index;
	BT_HFCLIENT_PB_ENTRY_APP_DATA_T    hfp_pb_entry_info;
} BT_HFP_PB_INFO_T;
#endif /*CONFIG_SUPPORT_PHONE_BOOK*/

/*hfp cbk status struct*/
typedef struct _BT_HFP_CBK_STATUS_T
{
	BT_HFCLIENT_VR_STATE_T vr_state;
	BT_HFCLIENT_CONNECTION_STATE_T conn_state;
    BT_HFCLIENT_AUDIO_STATE_T audio_state;
    BT_HFCLIENT_NETWORK_STATE_T network_state;
    BT_HFCLIENT_SERVICE_TYPE_T service_type;
	int signal_strength;	
    int battery_level;
	CHAR operator_name[HFCLIENT_OPERATOR_NAME_LEN + 1];
    BT_HFCLIENT_CALL_T call;
    BT_HFCLIENT_CALLSETUP_T callsetup;
    BT_HFCLIENT_CALLHELD_T callheld;
    BT_HFCLIENT_RESP_AND_HOLD_T resp_and_hold;
	CHAR clip_number[HFCLIENT_NUMBER_LEN + 1];
	CHAR call_waiting_number[HFCLIENT_NUMBER_LEN + 1];
	BT_HFCLIENT_CLCC_CB_DATA_T current_calls[3];
	BT_HFCLIENT_VGM_VGS_CB_DATA_T volume_change;
	BT_HFCLIENT_CNUM_CB_DATA_T subscriber_info;
    BT_HFCLIENT_IN_BAND_RING_STATE_T inband_ring_state;
    CHAR voice_tag_number[HFCLIENT_NUMBER_LEN + 1];
	int spk_volume;
	int mic_volume;
#if CONFIG_SUPPORT_PHONE_BOOK
    BT_HFP_PB_INFO_T pb_info;
#endif /*CONFIG_SUPPORT_PHONE_BOOK*/
}BT_HFP_CBK_STATUS_T;


/*------------------------------------------------------------------------------
                                            funcitons declarations
------------------------------------------------------------------------------*/
extern UINT16 _bt_hfp_get_dbg_level(VOID);
extern VOID _bt_hfp_set_dbg_level(UINT16 ui2_db_level);
extern INT32 _bt_hfp_send_msg_to_itself(UINT32 ui4_msgtype, BT_HFP_MSG_T* pt_bt_msg);
extern INT32 _bt_hfp_player_stop(VOID);
extern INT32 _bt_driver_in_thread(VOID);
extern INT32 _mic_in_thread(VOID);
extern INT32 _bt_hfp_mic_spk_thread_create(VOID);
extern INT32 _bt_hfp_mic_spk_thread_exit(VOID);
extern void *CVSD_UpSample_Init(char *pBuffer);
extern int CVSD_UpSample_GetMemory(void);
extern void CVSD_UpSample_Process(void *pHandle, short *pInSample, short *pOutSample, short *pTempBuffer, int iSourceSamples);
extern void *CVSD_DownSample_Init(char *pBuffer);
extern int CVSD_DownSample_GetMemory(void);
extern void CVSD_DownSample_Process(void *pHandle, short *pInSample, short *pOutSample, short *pTempBuffer, int iSourceSamples);
extern VOID _bt_hfp_set_cvsd_msbc_type(BOOL flag);
extern void CVSD_UpSample_Process_16_To_64(void *pHandle, short *pInSample, short *pOutSample, short *pTempBuffer, int iSourceSamples);
extern void CVSD_UpSample_Process_16_To_32(void *pHandle, short *pInSample, short *pOutSample, short *pTempBuffer, int iSourceSamples);
extern void CVSD_DownSample_Process_64_To_32(void *pHandle, short *pInSample, short *pOutSample, short *pTempBuffer, int iSourceSamples);
extern void CVSD_DownSample_Process_64_To_8(void *pHandle, short *pInSample, short *pOutSample, short *pTempBuffer, int iSourceSamples);
extern void CVSD_DownSample_Process_16_To_8(void *pHandle, short *pInSample, short *pOutSample, short *pTempBuffer, int iSourceSamples);
extern VOID open_hfp_dump(INT8 cmd);
extern VOID _set_test_enable(VOID);
extern VOID test_webrtc_file_aec(VOID);
extern VOID test_internal_dac_aec(VOID);


#endif  //_BLUETOOTH_HFP_H

