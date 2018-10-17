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



#ifndef    _U_APP_CTRL_H_
#define    _U_APP_CTRL_H_
/*-----------------------------------------------------------------------------
                    include files
-----------------------------------------------------------------------------*/
#include "u_handle.h"
#include "adaptor_log.h"

/*-----------------------------------------------------------------------------
                    macros, defines, typedefs, enums
-----------------------------------------------------------------------------*/

/* return value */
#define APP_CTRL_OK                         ((INT32)0)
#define APP_CTRL_FAIL                       ((INT32)-1) /* abnormal return must < 0 */
#define APP_CTRL_INV_ARG                    ((INT32)-2)

/* message */

/* application structure */
typedef struct _APP_CTRL_OBJ_T
{
    HANDLE_T        h_app;
    BOOL            b_g_is_init;
    pthread_t       t_thread;
    pthread_mutex_t t_mutex;
    pthread_cond_t  t_cond;
}APP_CTRL_OBJ_T;

typedef struct _APP_CTRL_MSG_T
{
    UINT32          ui4_msg_id;
    UINT32          ui4_data1;
    UINT32          ui4_data2;
    UINT32          ui4_data3;
} APP_CTRL_MSG_T;

typedef enum _APP_CTRL_CMD_INDEX_E
{
    APP_CTRL_CMD_WIFI_CONNECT = 0,
    APP_CTRL_CMD_WIFI_DISCONNECT,
    APP_CTRL_CMD_MAX
}APP_CTRL_CMD_INDEX_E;

typedef enum _APP_CTRL_RECV_REPORT_INDEX_E
{
    APP_CTRL_REPORT_WIFI_CONNECTION_STATUS = 0,
    APP_CTRL_REPORT_NTP_STATUS,
    APP_CTRL_REPORT_MAX
}APP_CTRL_RECV_REPORT_INDEX_E;

typedef enum _APP_CTRL_TIMER_INDEX_E
{
    APP_CTRL_TIMER_VOICE_WAKEUP_LED_CLOSE,
    APP_CTRL_TIMER__MAX
} APP_CTRL_IMER_INDEX_E;

#define APP_CTRL_FUNCTION_BEGIN         printf("[APP CTRL] %s %d line begin \n",__FUNCTION__,__LINE__)
#define APP_CTRL_FUNCTION_END           printf("[APP CTRL] %s %d line end \n",__FUNCTION__,__LINE__)
#define APP_CTRL_MSG(tag, fmt, args...)           {if (tag >= DEBUG)      printf("[APP CTRL][%s:%d] " fmt "\n", __FUNCTION__, __LINE__, ## args);}

#endif /*_U_APP_CTRL_H_*/

