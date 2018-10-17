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


#ifndef _BLUETOOTH_AUDIO_H_
#define _BLUETOOTH_AUDIO_H_

#include "u_common.h"
#include "u_amb.h"

/*-----------------------------------------------------------------------------
                    macros, defines, typedefs, enums
-----------------------------------------------------------------------------*/
/* return value */
#define APPUTILR_OK                              ((INT32)0)
#define APPUTILR_FAIL                            ((INT32)-1) /* abnormal return must < 0 */
#define APPUTILR_INV_ARG                         ((INT32)-2)
#define APPUTILR_LACK_BUFFER                     ((INT32)-3)
#define APPUTILR_NOT_EXIST                       ((INT32)-4)

#define BT_VOLUME_STEP     10
#define CONFIG_APP_SUPPORT_BT_STICKY_PAIRING     1
#define CONFIG_APP_SUPPORT_BLE_GATT_CLIENT		 0
#define CONFIG_APP_SUPPORT_BLE_GATT_SERVER       0
#define CONFIG_APP_SUPPORT_BT_KEEP_CONNECT                0
#define CONFIG_APP_SUPPORT_BT_BACKGROUND_CONNECTABLE      0
#define CONFIG_APP_SUPPORT_XPL                   0
#define CONFIG_APP_SUPPORT_BT_SRC_MODE           1
#define CONFIG_APP_SUPPORT_A2DP_SINK_SRC_BOTH      1
#if CONFIG_APP_SUPPORT_BT_SRC_MODE
#define BT_AUDIO_NAME_MAX_LENGTH    (256)
#define BT_AUDIO_ANCHOR_MAX_LENGTH  (256)
#define BT_AUDIO_SOURCE_MAX_LENGTH  (32)
#define BT_AUDIO_ALBUM_MAX_LENGTH   (256)
#define SOURCE_MAX_LENGTH        (16)
#endif /*CONFIG_APP_SUPPORT_BT_SRC_MODE*/

#ifdef CONSYS_WIFI_CHIP
#define CONFIG_SUPPORT_BT_HFP                             1
#define CONFIG_SUPPORT_PHONE_BOOK                         1
#endif
#ifdef WIFI_CHIP_6630
#define CONFIG_SUPPORT_BT_HFP                             1
#define CONFIG_SUPPORT_PHONE_BOOK                         1
#endif
#ifdef WIFI_CHIP_7668
#define CONFIG_SUPPORT_BT_HFP                             1
#define CONFIG_SUPPORT_PHONE_BOOK                         1
#endif


enum BLUETOOTH_ENABLE_T
{
    BLUETOOTH_ENABLE = 0,
    BLUETOOTH_DISABLE,
};

//net connection
enum NETWORK_CONNETION_T
{
    NETCONNECTION_ON,
    NETCONNECTION_OFF,
};

typedef enum _CFG_RECID_BLUETOOTH_T
{
    CFG_RECID_BLUETOOTH_ENABLE,
}CFG_RECID_BLUETOOTH_T;

#if CONFIG_APP_SUPPORT_BT_SRC_MODE
typedef struct
{
    char audioName[BT_AUDIO_NAME_MAX_LENGTH+1];
    char audioAnchor[BT_AUDIO_ANCHOR_MAX_LENGTH+1];
    char audioAlbum[BT_AUDIO_ALBUM_MAX_LENGTH+1];
    char audioSource[BT_AUDIO_SOURCE_MAX_LENGTH+1];
    char source[SOURCE_MAX_LENGTH+1];
    UINT32 song_length;
    int  progress;
}BT_SRC_PLAYER_STATUS_T;
#endif /*CONFIG_APP_SUPPORT_BT_SRC_MODE*/

/*------------------------------------------------------------------------------
                                            funcitons declarations
------------------------------------------------------------------------------*/
extern VOID a_bluetooth_audio_init(AMB_REGISTER_INFO_T* pt_reg);
extern INT32 a_bluetooth_btkey_handle(UINT32 ui4_keysta, UINT32 ui4_keyval);
extern INT32 u_bluetooth_clear_bluetooth_data(VOID);
extern INT32 a_bluetooth_get_local_mac_addr(CHAR* pc_mac);
extern INT32 u_bluetooth_set_absolute_volume(UINT8 ui1_value);
extern INT32 u_bluetooth_src_request_mediaInfo_from_url(BT_SRC_PLAYER_STATUS_T *medialnfo);

#endif  /* _BLUETOOTH_AUDIO_H_ */
