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


/*-----------------------------------------------------------------------------
 * $RCSfile: u_appman.h,v $
 * $Revision: #1 $
 * $Date: 2016/03/07 $
 * $Author: bdbm01 $
 * $CCRevision: /main/DTV_X/DTV_X_HQ_int/3 $
 * $SWAuthor: Iolo Tsai $
 * $MD5HEX: 7073777ef840de31d560641fd88f5e7f $
 *
 * Description:
 *         This header file contains AEE specific definitions, which are
 *         exported.
 *---------------------------------------------------------------------------*/

#ifndef _U_APPMAN_H_
#define _U_APPMAN_H_


/*-----------------------------------------------------------------------------
                    include files
-----------------------------------------------------------------------------*/

#include "u_common.h"
#include "u_handle.h"
#include "u_aee.h"
#include "u_os.h"

/*-----------------------------------------------------------------------------
                    macros, defines, typedefs, enums
 ----------------------------------------------------------------------------*/
#define MAX_USR_MSG_LEN     4000

typedef struct _APPMSG_T
{
    UINT32     ui4_sender_id;
    UINT32     ui4_msg_type;
    INT8       p_usr_msg[0];
}APPMSG_T;


typedef enum
{
    MSG_FROM_SM = 0,
    MSG_FROM_BT,
    MSG_FROM_AM,
    MSG_FROM_LINEIN,
    MSG_FROM_C4A_STUB,
    MSG_FROM_UPG_CONTROL,
    MSG_FROM_UPG,
    MSG_FROM_UI,
    MSG_FROM_DM,
    MSG_FROM_URI,
    MSG_FROM_TTS,
    MSG_FROM_PROMPT,
    MSG_FROM_WIFI_SETTING,
    MSG_FROM_MISC,
    MSG_FROM_ALARM,
    MSG_FROM_ASSISTANT_STUB,
    MSG_FROM_PB_MIC_IN,
    MSG_FROM_BT_HFP,
    MSG_FROM_DLNA,
#ifdef CONFIG_ADAPTOR_APP_CTRL
    MSG_FROM_APP_CTRL,
#endif /* CONFIG_ADAPTOR_APP_CTRL */
#ifdef CONFIG_BLE_SUPPORT
    MSG_FROM_BLE,
#endif /* CONFIG_BLE_SUPPORT */
    MSG_FROM_TDMIN
}MSG_SENDER_ID_E;

typedef enum
{
    APP_NFY_PRE_INIT,
    APP_NFY_INIT_OK,
    APP_NFY_INIT_FAILED,
    APP_NFY_PRE_EXIT,
    APP_NFY_EXIT_OK,
    APP_NFY_EXIT_FAILED,
    APP_NFY_PRE_PROCESS_MSG,
    APP_NFY_PROCESS_MSG_OK,
    APP_NFY_PROCESS_MSG_FAILED
}   APP_NFY_REASON_T;

typedef enum
{
    APP_EXIT_FORCE,
    APP_EXIT_DEMAND
}   APP_EXIT_MODE_T;


typedef BOOL (*u_app_nfy_fct) (HANDLE_T          h_app,
                               VOID*             pv_tag,
                               APP_NFY_REASON_T  e_nfy_reason);

typedef INT32 (*u_app_init_fct) (const CHAR*  ps_name,
                                 HANDLE_T     h_app);

typedef INT32 (*u_app_exit_fct) (HANDLE_T         h_app,
                                 APP_EXIT_MODE_T  e_exit_mode);
typedef INT32 (*u_app_process_msg_fct) (HANDLE_T     h_app,
                                        UINT32       ui4_type,
                                        const VOID*  pv_msg,
                                        SIZE_T       z_msg_len,
                                        BOOL         b_paused);

typedef struct
{
    u_app_init_fct         pf_init;
    u_app_exit_fct         pf_exit;
    u_app_process_msg_fct  pf_process_msg;
}   APP_FCT_TBL_T;

typedef struct
{
    UINT64               ui8_flags;
    THREAD_DESCR_T       t_thread_desc;
    UINT32               ui4_app_group_id;
    UINT32               ui4_app_id;
    UINT16               ui2_msg_count;
    UINT16               ui2_max_msg_size;
}   APP_DESC_T;

#define  AMB_BROADCAST_OFFSET     ((UINT32)  (1<<16))  /* broadcast group offset */

/**
Msg Type is a set of certain messages.
msg Id in msg is to identify a message.
*/
typedef enum{
    E_APP_MSG_TYPE_TIMER = 0,
    E_APP_MSG_TYPE_USER_CMD,
    E_APP_MSG_TYPE_BUTTON,
    E_APP_MSG_TYPE_URI,
    E_APP_MSG_TYPE_TTS,
    E_APP_MSG_TYPE_PROMPT,
    E_APP_MSG_TYPE_ACFG,
    E_APP_MSG_TYPE_AI_MANAGER,
    E_APP_MSG_TYPE_ASSISTANT_STUB,
    E_APP_MSG_TYPE_STATE_MNGR,
    E_APP_MSG_TYPE_BLUETOOTH,
    E_APP_MSG_TYPE_BLUETOOTH_HFP,
    E_APP_MSG_TYPE_WIFI_SETTING,
    E_APP_PRI_AI_MANAGER_MSG,
    E_APP_MSG_TYPE_MISC,
    E_APP_PRI_ALSA_PB_MSG,
    E_APP_MSG_TYPE_UPG_CTRL,
    E_APP_MSG_TYPE_DLNA,
#ifdef CONFIG_ADAPTOR_APP_CTRL
    E_APP_MSG_TYPE_APP_CTRL,
#endif /* CONFIG_ADAPTOR_APP_CTRL */
#ifdef CONFIG_BLE_SUPPORT
    E_APP_MSG_TYPE_BLE,
#endif /* CONFIG_BLE_SUPPORT */
    E_APP_MSG_TYPE_BROADCAST_OFFSET = AMB_BROADCAST_OFFSET,
    E_APP_MSG_SCAN_ACTIVE,
    E_APP_MSG_TYPE_USB_DEV,
    E_APP_MSG_TYPE_NETWORK,
    E_APP_MSG_TYPE_DUT_STATE,
    E_APP_MSG_TYPE_C4A_STUB,
    E_APP_MSG_TYPE_MAX
}APP_MSG_TYPE_E;

extern INT32 u_app_send_msg (HANDLE_T       h_app,
                      APP_MSG_TYPE_E      ui4_type,
                      const VOID*    pv_msg,
                      SIZE_T         z_msg_len,
                      u_app_nfy_fct  pf_nfy,
                      VOID*          pv_tag);

extern INT32 u_app_send_appmsg(HANDLE_T        h_app,
                               APP_MSG_TYPE_E  e_app_type,
                               UINT32          ui4_sender_id,
                               UINT32          ui4_msg_type,
                               void*           p_usr_msg,
                               SIZE_T          t_size);
#endif /* _U_APPMAN_H_ */
