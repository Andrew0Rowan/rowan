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


#ifndef _BLUETOOTH_AUDIO_KEY_PROC_H_
#define _BLUETOOTH_AUDIO_KEY_PROC_H_

#include "u_common.h"
#include "u_key_def.h"

#define BT_AUD_KEY_DELAY_TIMEOUT   150      //ms, widget will send key event after 200ms,so time out value must less than it

//key status
#define BT_AUD_KEYSTA_REPEAT    2//WGL_MSG_KEY_REPEAT
#define BT_AUD_KEYSTA_DOWN      0//WGL_MSG_KEY_DOWN
#define BT_AUD_KEYSTA_UP        1//WGL_MSG_KEY_UP

#define BLUETOOTH_BTN_NEXT          0x0000f000    
#define BLUETOOTH_BTN_PREV          0x0000f001
#define BLUETOOTH_BTN_PLAY          0x0000f002
#define BLUETOOTH_BTN_STOP          0x0000f003
#define BLUETOOTH_BTN_PAUSE         0x0000f004
#if CONFIG_APP_CUSTOM_VIZIO_IR_MAP
#define BLUETOOTH_BTN_PLAY_PAUSE    0x0000f005
#endif
#define BLUETOOTH_BTN_FF            0x0000f006
#define BLUETOOTH_BTN_FR            0x0000f007
#define BTN_BLUETOOTH               0x0000f008
#define BLUETOOTH_BTN_VOL_UP        0x0000f009
#define BLUETOOTH_BTN_VOL_DOWN      0x0000f00a
//key val
#define BT_AUD_KEY_NEXT         BLUETOOTH_BTN_NEXT    //next 
#define BT_AUD_KEY_PREV         BLUETOOTH_BTN_PREV    //prev
#define BT_AUD_KEY_PLAY         BLUETOOTH_BTN_PLAY    //play
#define BT_AUD_KEY_STOP         BLUETOOTH_BTN_STOP    //stop
#define BT_AUD_KEY_PAUSE        BLUETOOTH_BTN_PAUSE   //pause
#if CONFIG_APP_CUSTOM_VIZIO_IR_MAP
#define BT_AUD_KEY_PAUSE_PLAY   BLUETOOTH_BTN_PLAY_PAUSE  //pause and play
#else
#define BT_AUD_KEY_PAUSE_PLAY   BTN_SELECT  //pause and play     //u_key_def.h
#endif
#define BT_AUD_KEY_FF           BLUETOOTH_BTN_FF      //FF
#define BT_AUD_KEY_FR           BLUETOOTH_BTN_FR      //FB
#define BT_AUD_KEY_BLUETOOTH    BTN_BLUETOOTH   //BTKEY
#define BT_AUD_KEY_VOL_UP       BLUETOOTH_BTN_VOL_UP  //volume up
#define BT_AUD_KEY_VOL_DOWN     BLUETOOTH_BTN_VOL_DOWN//volume down

#define BT_AUD_KEY_SRC_SWITCH       BT_AUD_KEYSTA_UP      //SRC SWITCH 
#define BT_AUD_KEY_FORCED_PAIRING   BT_AUD_KEYSTA_REPEAT    //FORCED PAIRING 


/*------------------------------------------------------------------------------
                                            funcitons declarations
------------------------------------------------------------------------------*/
extern INT32 bt_aud_key_timer_create(VOID);
extern INT32 bt_aud_key_timer_delete(VOID);
extern INT32 bt_aud_key_msg_proc(const VOID *pv_msg);

extern INT32 bt_aud_bluetooth_force_pause(VOID);
extern INT32 bt_aud_bluetooth_continue_play(VOID);

#endif  //_BLUETOOTH_AUDIO_KEY_PROC_H_