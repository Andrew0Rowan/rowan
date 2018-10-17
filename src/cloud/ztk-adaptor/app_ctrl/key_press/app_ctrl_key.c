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

/* application level */
#include "u_amb.h"
#include "u_common.h"
#include "u_app_thread.h"
#include "u_cli.h"
#include "u_os.h"
#include "u_assert.h"
#include "u_user_interface.h"
#include "app_ctrl_key.h"

/* private */
#include "user_interface.h"
#include "user_interface_cli.h"

static VOID app_ctrl_key_send_event(UINT32 keyType,UCHAR *data,UINT32 len) {
    HANDLE_T h_app = NULL_HANDLE;
    INT32 i4_ret = 0;

    if(0 != (i4_ret = u_am_get_app_handle_from_name(&h_app, APP_CTRL_THREAD_NAME))) {
       printf("<APP_CTRL_KEY> get handle fail!,i4_ret=%ld",i4_ret);
       return;
    }

    i4_ret = u_app_send_appmsg(h_app,
                              E_APP_MSG_TYPE_USER_CMD,
                              MSG_FROM_UI,
                              keyType,
                              data,
                              len);
    if(0 == i4_ret) {
       printf("<APP_CTRL_KEY> success!");
    } else {
       printf("<APP_CTRL_KEY> fail!",i4_ret);
    }

    return;
}

VOID app_ctrl_key_process(UINT32 ui2_key_code,UINT32 value) {
    APP_CTRL_KEY_MSG_T keyData = {0};

    switch (ui2_key_code) {

        case KEY_MUTE:
              printf("<app_ctrl_key> short press KEY_MUTE\n");
              break;
        case KEY_MICMUTE:
              printf("<app_ctrl_key> short press KEY_MICMUTE\n");
              break;
        case KEY_BLUETOOTH:
              printf("<app_ctrl_key> short press KEY_SOUCE\n");
              break;
        case KEY_VOLUMEDOWN:
              printf("<app_ctrl_key> short press KEY_VOLUMEDOWN\n");
              keyData.key_value = 10;
              break;
        case KEY_VOLUMEUP:
              printf("<app_ctrl_key> short press KEY_VOLUMEUP\n");
              keyData.key_value = 10;
              break;
        case KEY_APP_CTRL_POWER:
              printf("<app_ctrl_key> short press KEY_APP_CTRL_POWER\n");
        default:
              printf("<app_ctrl_key> short press INVALID");
              break;
    }

    keyData.key_ratio = KEY_SHORT_PRESS_RATIO;

    app_ctrl_key_send_event(ui2_key_code,&keyData,sizeof (APP_CTRL_KEY_MSG_T));

}

VOID app_ctrl_key_long_press_process(UINT32 ui2_key_code) {

    APP_CTRL_KEY_MSG_T keyData = {0};

    switch (ui2_key_code) {

        case KEY_MUTE:
              printf("<app_ctrl_key> long press KEY_MUTE\n");
              break;
        case KEY_MICMUTE:
              printf("<app_ctrl_key> long press KEY_MICMUTE\n");
              break;
        case KEY_BLUETOOTH:
              printf("<app_ctrl_key> long press KEY_SOUCE\n");
              break;
        case KEY_VOLUMEDOWN:
              printf("<app_ctrl_key> long press KEY_VOLUMEDOWN\n");
              break;
        case KEY_VOLUMEUP:
              printf("<app_ctrl_key> long press KEY_VOLUMEUP\n");
              break;
        case KEY_APP_CTRL_POWER:
              printf("<app_ctrl_key> long press KEY_APP_CTRL_POWER\n");
        default:
              printf("<app_ctrl_key> long press INVALID");
              break;
    }

    keyData.key_ratio = KEY_LONG_PRESS_RATIO;
    keyData.key_value = 0;

    app_ctrl_key_send_event(ui2_key_code,&keyData,sizeof (APP_CTRL_KEY_MSG_T));

}
