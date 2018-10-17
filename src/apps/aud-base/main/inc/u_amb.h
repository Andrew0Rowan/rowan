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



#ifndef _A_AMB_H_
#define _A_AMB_H_

/*-----------------------------------------------------------------------------
                    include files
-----------------------------------------------------------------------------*/
#include "u_common.h"
#include "u_appman.h"

/*-----------------------------------------------------------------------------
                    macros, defines, typedefs, enums
 ----------------------------------------------------------------------------*/
/* application manager broker API return values */
#define  AMBR_OK                  ((INT32)    0)
#define  AMBR_FAIL                ((INT32)   -1)
#define  AMBR_INV_ARG             ((INT32)   -2)
#define  AMBR_NOT_AUTHORIZED      ((INT32)   -3)
#define  AMBR_BE_REGISTERED       ((INT32)   -4)

#define  APP_MNGR_NAME            "app_mngr"   /* application manager name */
#define  APP_NAME_MAX_LEN         (16)  /* maximum length of application name */

#define  BROADCAST_MSG_MAX_LEN    (24) /* maximum length of broadcast messages */
#define  DISPATCH_MSG_MAX_LEN     (20) /* maximum length of dispatched messages */




/*
   Registration information.
*/
typedef struct _AMB_REGISTER_INFO_T
{
    struct _AMB_REGISTER_INFO_T*  pt_prev;
    struct _AMB_REGISTER_INFO_T*  pt_next;

    CHAR                   s_name[APP_NAME_MAX_LEN+1];      /* application name */
    APP_FCT_TBL_T          t_fct_tbl;                       /* function table */
    APP_DESC_T             t_desc;                          /* description */
}   AMB_REGISTER_INFO_T;

/*-----------------------------------------------------------------------------
                    typedefs for application manager
 ----------------------------------------------------------------------------*/

/*
   Broadcast message.
*/
typedef struct
{
    APP_MSG_TYPE_E         ui4_type;                        /* message type */
    UCHAR                  uc_msg[BROADCAST_MSG_MAX_LEN];   /* broadcast message */
    SIZE_T                 z_msg_len;                       /* message length */
}   AMB_BROADCAST_MSG_T;

/*
   Dispatch message.
*/
typedef struct
{
    CHAR*                  ps_name;                         /* application name */
    UINT32                 ui4_type;                        /* message type */
    UCHAR                  uc_msg[DISPATCH_MSG_MAX_LEN];    /* broadcast message */
    SIZE_T                 z_msg_len;                       /* message length */
}   AMB_DISPATCH_MSG_T;
/*
   Application Manager requests.
*/
typedef enum
{
    AMB_REQUEST_UNKNOWN    =    0,

    AMB_REQUEST_START,                  /* start an application */
    AMB_REQUEST_RESUME,                 /* resume an application */
    AMB_REQUEST_PAUSE,                  /* pause an application */
    AMB_REQUEST_EXIT,                   /* exit an application */
    AMB_REQUEST_REG,                    // register app to am
    AMB_REQUEST_BROADCAST_MSG,          /* broadcast messages */
    AMB_REQUEST_DISPATCH_MSG,           /* dispatch messages to a specific applcation */
    AMB_REQUEST_TOTAL_NUMBER

}   AMB_REQUEST_TYPE_T;

typedef enum
{
    AMB_APP_UNKNOW ,
	AMB_APP_IS_INITING , //app callback in init process
	AMB_APP_IS_INITED ,//app callback  init done
	AMB_APP_IS_EXITING ,
	AMB_APP_IS_EXITED ,
	AMB_APP_STATUS_MAX,

}AMB_APP_STATUS_TYPE_T;


/*-----------------------------------------------------------------------------
                    functions declarations
 ----------------------------------------------------------------------------*/
extern INT32 u_amb_init (VOID);
extern VOID  u_amb_destroy (VOID);
extern INT32 u_amb_register_app (AMB_REGISTER_INFO_T* pt_reg);
extern INT32 u_amb_start_app (const CHAR* ps_name);
extern INT32 u_amb_exit_app (const CHAR* ps_name);
extern INT32 u_amb_sync_start_app (const CHAR* ps_name);
extern INT32 u_amb_sync_exit_app (const CHAR* ps_name);
extern INT32 u_amb_broadcast_msg (AMB_BROADCAST_MSG_T *pmsg, BOOL    b_sync);
extern AMB_REGISTER_INFO_T* u_amb_get_register_info (const CHAR* ps_name);
#endif /* _A_AMB_H_ */
