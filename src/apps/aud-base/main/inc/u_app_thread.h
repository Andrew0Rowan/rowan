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




#ifndef __U_APP_THREAD_H__
#define __U_APP_THREAD_H__


#include "u_app_priority.h"

/* Example

#define EXAMPLE_TASK_NAME        ExampleTask"
#define EXAMPLE_TASK_STACK_SZ    2048
#define EXAMPLE_TASK_PRIORITY    RIORITY(PRIORITY_CLASS_NORMAL, PRIORITY_LAYER_MIDDLEWARE, 0)

*/

/* Application */
//#define APP_THREAD_HIGHEST_PRIORITY  200
#define APP_THREAD_HIGHEST_PRIORITY     PRIORITY(PRIORITY_CLASS_HIGH, PRIORITY_LAYER_UI, -5)


#define TIMERD_THREAD_NAME          "timerd"
#define HELLOTEST_THREAD_NAME       "hellotest"
#define PPC_THREAD_NAME				"ppcSetting"
#define USER_INTERFACE_THREAD_NAME	"user_interface"
#define AISPEECH_MANAGER			"aispeech_manager"

#define DEFAULT_THREAD_FLAG  		(~((UINT64)0))
#define DEFAULT_STACK_SIZE          (4096)
#define DEFAULT_THREAD_PRIORITY     PRIORITY(PRIORITY_CLASS_NORMAL, PRIORITY_LAYER_UI, 0)
#define DEFAULT_NUM_MSGS            (32)
#define DEFAULT_MSGS_COUNT          (32)
#define DEFAULT_MAX_MSGS_SIZE       (512)


#define ACFG_THREAD_NAME            "acfg"
#define ACFG_STACK_SIZE             (1024*64)
#define ACFG_THREAD_PRIORITY        PRIORITY(PRIORITY_CLASS_NORMAL, PRIORITY_LAYER_UI, 0)
#define ACFG_NUM_MSGS               (32)
#define ACFG_MSGS_COUNT             (32)
#define ACFG_MAX_MSGS_SIZE          (4096)

#define SM_THREAD_NAME              "state_mngr"
#define SM_STACK_SIZE               (1024 * 64)
#define SM_THREAD_PRIORITY          PRIORITY(PRIORITY_CLASS_NORMAL, PRIORITY_LAYER_UI, 0)
#define SM_NUM_MSGS                 (128)
#define SM_MSGS_COUNT               (128)
#define SM_MAX_MSGS_SIZE            (4096)

#define DM_THREAD_NAME              "dev_mngr"
#define DM_STACK_SIZE               (1024 * 64)
#define DM_THREAD_PRIORITY          PRIORITY(PRIORITY_CLASS_HIGH, PRIORITY_LAYER_UI, -5)
#define DM_NUM_MSGS                 (64)
#define DM_MSGS_COUNT               (128)
#define DM_MSGS_SIZE                (4096)

#define BTN_SOUND_THREAD_NAME       "button_sound"
#define BTN_SOUND_STACK_SIZE        (1024 * 64)
#define BTN_SOUND_THREAD_PRIORITY   PRIORITY(PRIORITY_CLASS_HIGH, PRIORITY_LAYER_UI, 0)
#define BTN_SOUND_NUM_MSGS          (64)
#define BTN_SOUND_MSGS_COUNT        (128)
#define BTN_SOUND_MAX_MSGS_SIZE     (4096)

/*c4a stub*/
#define C4A_STUB_THREAD_NAME 	    "c4a_stub"
#define C4A_STUB_STACK_SZ    	    (1024 * 64)
#define C4A_STUB_THREAD_PRIORITY    PRIORITY(PRIORITY_CLASS_HIGH, PRIORITY_LAYER_UI, 0)
#define C4A_STUB_THREAD_MSG_NUM     (64)
#define C4A_STUB_MSGS_COUNT         (128)
#define C4A_STUB_MAX_MSGS_SIZE      (4096)

#define UPG_CONTROL_THREAD_NAME     "upg_control"
#define UPG_CONTROL_STACK_SIZE      (1024 * 64)
#define UPG_CONTROL_THREAD_PRIORITY PRIORITY(PRIORITY_CLASS_NORMAL, PRIORITY_LAYER_UI, 0)
#define UPG_CONTROL_NUM_MSGS        (32)
#define UPG_CONTROL_MSGS_COUNT      (32)
#define UPG_CONTROL_MAX_MSGS_SIZE   (4096)

#define UPG_THREAD_NAME     		"upg"
#define UPG_STACK_SIZE      		(1024 * 64)
#define UPG_THREAD_PRIORITY 		PRIORITY(PRIORITY_CLASS_NORMAL, PRIORITY_LAYER_UI, 0)
#define UPG_NUM_MSGS       			(32)
#define UPG_MSGS_COUNT      		(32)
#define UPG_MAX_MSGS_SIZE   		(4096)

/* bluetooth */
#define BLUETOOTH_THREAD_NAME       "bluetooth"
#define BLUETOOTH_STACK_SIZE        (1024 * 64)
#define BLUETOOTH_THREAD_PRIORITY   PRIORITY(PRIORITY_CLASS_NORMAL, PRIORITY_LAYER_UI, 0)
#define BLUETOOTH_NUM_MSGS          (64)
#define BLUETOOTH_MSGS_COUNT        (128)
#define BLUETOOTH_MAX_MSGS_SIZE     (4096)

#define PB_URI_THREAD_NAME          "playback_uri"
#define PB_URI_STACK_SIZE           (1024 * 128)
#define PB_URI_THREAD_PRIORITY      PRIORITY(PRIORITY_CLASS_HIGH, PRIORITY_LAYER_UI, 0)
#define PB_URI_NUM_MSGS             (64)
#define PB_URI_MSGS_COUNT           (128)
#define PB_URI_MAX_MSGS_SIZE        (4096)

#define PB_TTS_THREAD_NAME          "playback_tts"
#define PB_TTS_STACK_SIZE           (1024 * 128)
#define PB_TTS_THREAD_PRIORITY      PRIORITY(PRIORITY_CLASS_HIGH, PRIORITY_LAYER_UI, 0)
#define PB_TTS_NUM_MSGS             (64)
#define PB_TTS_MSGS_COUNT           (128)
#define PB_TTS_MAX_MSGS_SIZE        (4096)

#define PB_DLNA_THREAD_NAME          "playback_dlna"
#define PB_DLNA_STACK_SIZE           (1024 * 128)
#define PB_DLNA_THREAD_PRIORITY      PRIORITY(PRIORITY_CLASS_HIGH, PRIORITY_LAYER_UI, 0)
#define PB_DLNA_NUM_MSGS             (64)
#define PB_DLNA_MSGS_COUNT           (128)
#define PB_DLNA_MAX_MSGS_SIZE        (4096)


#define PB_PROMPT_THREAD_NAME       "playback_prompt"
#define PB_PROMPT_STACK_SIZE        (1024 * 128)
#define PB_PROMPT_THREAD_PRIORITY   PRIORITY(PRIORITY_CLASS_HIGH, PRIORITY_LAYER_UI, 0)
#define PB_PROMPT_NUM_MSGS          (64)
#define PB_PROMPT_MSGS_COUNT        (128)
#define PB_PROMPT_MAX_MSGS_SIZE     (4096)

/*assistant stub*/
#define ASSISTANT_STUB_THREAD_NAME 	  "assistant_stub"
#define ASSISTANT_STUB_STACK_SZ    	  (1024 * 128)
#define ASSISTANT_STUB_THREAD_PRIORITY  PRIORITY(PRIORITY_CLASS_HIGH, PRIORITY_LAYER_UI, 0)
#define ASSISTANT_STUB_THREAD_MSG_NUM   (64)
#define ASSISTANT_STUB_MSGS_COUNT       (128)
#define ASSISTANT_STUB_MAX_MSGS_SIZE    (4096)

/*wifi_setting*/
#define WIFI_SETTING_THREAD_NAME      "wifi_setting"
#define WIFI_SETTING_STACK_SZ         (1024 * 128)
#define WIFI_SETTING_THREAD_PRIORITY  PRIORITY(PRIORITY_CLASS_NORMAL, PRIORITY_LAYER_UI, 0)
#define WIFI_SETTING_NUM_MSGS         (64)
#define WIFI_SETTING_MSGS_COUNT       (128)
#define WIFI_SETTING_MAX_MSGS_SIZE    (4096)

/*misc*/
#define MISC_THREAD_NAME 	  "misc"
#define MISC_STACK_SZ    	  (1024 * 128)
#define MISC_THREAD_PRIORITY  PRIORITY(PRIORITY_CLASS_HIGH, PRIORITY_LAYER_UI, 0)
#define MISC_THREAD_MSG_NUM   (64)
#define MISC_MSGS_COUNT       (128)
#define MISC_MAX_MSGS_SIZE    (4096)

#if 1//CONFIG_SUPPORT_BT_HFP
#define BLUETOOTH_HFP_THREAD_NAME       "bluetooth_hfp"
#define BLUETOOTH_HFP_STACK_SIZE        (1024 * 64)
#define BLUETOOTH_HFP_THREAD_PRIORITY   PRIORITY(PRIORITY_CLASS_NORMAL, PRIORITY_LAYER_UI, 0)
#define BLUETOOTH_HFP_NUM_MSGS          (64)
#define BLUETOOTH_HFP_MSGS_COUNT        (128)
#define BLUETOOTH_HFP_MAX_MSGS_SIZE     (4096)
#endif /*CONFIG_SUPPORT_BT_HFP*/

#define PB_TDM_IN_THREAD_NAME          "playback_tdm_in"
#define PB_TDM_IN_STACK_SIZE           (1024 * 128)
#define PB_TDM_IN_THREAD_PRIORITY      PRIORITY(PRIORITY_CLASS_HIGH, PRIORITY_LAYER_UI, 0)
#define PB_TDM_IN_NUM_MSGS             (64)
#define PB_TDM_IN_MSGS_COUNT           (128)
#define PB_TDM_IN_MAX_MSGS_SIZE        (4096)

#ifdef CONFIG_ADAPTOR_APP_CTRL
/*adaptor*/
#define APP_CTRL_THREAD_NAME      "adaptor"
#define APP_CTRL_STACK_SZ         (1024 * 128)
#define APP_CTRL_THREAD_PRIORITY  PRIORITY(PRIORITY_CLASS_HIGH, PRIORITY_LAYER_UI, 0)
#define APP_CTRL_NUM_MSGS         (64)
#define APP_CTRL_MSGS_COUNT       (128)
#define APP_CTRL_MAX_MSGS_SIZE    (4096)
#endif /* CONFIG_ADAPTOR_APP_CTRL */

#ifdef CONFIG_BLE_SUPPORT
/*ble*/
#define BLE_THREAD_NAME      "ble"
#define BLE_STACK_SZ         (1024 * 128)
#define BLE_THREAD_PRIORITY  PRIORITY(PRIORITY_CLASS_HIGH, PRIORITY_LAYER_UI, 0)
#define BLE_NUM_MSGS         (64)
#define BLE_MSGS_COUNT       (128)
#define BLE_MAX_MSGS_SIZE    (4096)
#endif /* CONFIG_BLE_SUPPORT */

#endif // __U_APP_THREAD_H__

