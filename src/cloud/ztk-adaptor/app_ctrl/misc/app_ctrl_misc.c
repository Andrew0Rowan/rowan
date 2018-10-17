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


/* middleware level*/
#include "u_appman.h"
#include "u_aee.h"
#include "u_cli.h"
#include "u_misc.h"

#include "u_dbg.h"
#include "u_os.h"

/* application level */
#include "u_amb.h"
#include "u_am.h"
#include "u_common.h"
/* private */
#include "app_ctrl.h"
#include "c_net_config.h"
#include "u_app_thread.h"
#include "u_acfg.h"
#include "u_app_def.h"

static VOID app_ctrl_misc_send_event(VOID *data,SIZE_T len,UINT32 msgType) {
    HANDLE_T h_app = NULL_HANDLE;
    INT32 i4_ret = 0;

    if(0 != (i4_ret = u_am_get_app_handle_from_name(&h_app, APP_CTRL_THREAD_NAME))) {
       APP_CTRL_MSG(ERROR,"<APP_CTRL_MISC> get handle fail!,i4_ret=%ld",i4_ret);
       return;
    }

    i4_ret = u_app_send_appmsg(h_app,
                              E_APP_MSG_TYPE_MISC,
                              MSG_FROM_MISC,
                              msgType,
                              data,
                              len);
    if(0 == i4_ret) {
       APP_CTRL_MSG(ERROR,"<APP_CTRL_MISC> success!");
    } else {
       APP_CTRL_MSG(ERROR,"<APP_CTRL_MISC> fail!",i4_ret);
    }

    return;
}

VOID app_ctrl_misc_report_ntp_status(BOOL ntp_status) {

    app_ctrl_misc_send_event(&ntp_status,sizeof (BOOL),APP_CTRL_REPORT_NTP_STATUS);

    return;
}

VOID app_ctrl_misc_report_wifi_connection_status(UINT32 wifi_conn_status) {

    app_ctrl_misc_send_event(&wifi_conn_status,sizeof (UINT32),APP_CTRL_REPORT_WIFI_CONNECTION_STATUS);

    return;
}