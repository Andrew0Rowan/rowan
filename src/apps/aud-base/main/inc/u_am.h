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


#ifndef _U_AM_H_
#define _U_AM_H_

#include "u_common.h"
#include "u_amb.h"
/*-----------------------------------------------------------------------------
                    macros, defines, typedefs, enums
 ----------------------------------------------------------------------------*/
/*
   API return values of Application Manager.
*/
#define AMR_OK                   ((INT32)    0)        /* OK */
#define AMR_FAIL                 ((INT32)   -1)        /* Something error? */
#define AMR_INV_ARG              ((INT32)   -2)        /* Invalid arguemnt. */
#define AMR_NOT_AUTHORIZED       ((INT32)   -3)        /* The application is not registered. */
#define AMR_UNKNOWN_REQUEST      ((INT32)   -4)        /* Unknown request to Application Manager. */
#define AMR_RESUME_DISALLOW      ((INT32)   -5)        /* The application is not allowed to be resumed. */
#define AMR_ALREADY_STARTED      ((INT32)   -6)        /* The application has already been started. */

#define  AMB_BROADCAST_SIZE      ((UINT32)  64)

/* Application manager */
#define  AMB_BROADCAST_APPMAN    \
         	 ((UINT32) (0 * AMB_BROADCAST_SIZE + AMB_BROADCAST_OFFSET))
         
#define  AMB_BROADCAST_MISC         \
			 ((UINT32) (1 * AMB_BROADCAST_SIZE + AMB_BROADCAST_OFFSET))
	
#define  AMB_BROADCAST_APP_UTIL   \
			 ((UINT32) (9 * AMB_BROADCAST_SIZE + AMB_BROADCAST_OFFSET))

/* APP_UTIL message */
typedef enum
{
    APP_UTIL_BRDCST_MSG_TO_APP= AMB_BROADCAST_APP_UTIL,
    APP_MSG_UPG_TO_NFX,
    APP_UTIL_BRDCST_MSG_MAX
}   APP_UTIL_BRDCST_MSG_T;

/*   Broadcast messages of Application Manager.*/
typedef enum
{
    AM_BRDCST_MSG_SYSTEM_KEY = AMB_BROADCAST_APPMAN,   /* A system key is pressed down. */
    AM_BRDCST_MSG_SYSTEM_KEY_REPEAT,                   /* A system key is being held. */
    AM_BRDCST_MSG_PRE_POWER_OFF,                       /* The system is going to be power-off. This message will be broadcast before AM_BRDCST_MSG_POWER_OFF. */
    AM_BRDCST_MSG_POWER_ON,                            /* The system is going to be power-on. */
    AM_BRDCST_MSG_POWER_OFF,                           /* The system is going to be power-off. */
    AM_BRDCST_MSG_POWER_CUT,                           /* The system is ready to be power-cut. */
    AM_BRDCST_MSG_APP_ACTIVE,                          /* An application is active. */
    AM_BRDCST_MSG_APP_PAUSED,                          /* An application is paused. */
    AM_BRDCST_MSG_CHANNEL_SCAN_ACTIVE,                 /* Channel scan is active. */
    AM_BRDCST_MSG_CHANNEL_SCAN_INACTIVE,               /* Channel scan is inactive. */
    AM_BRDCST_MSG_DEFAULT_KEY_HANDLER,                 /* Default key handler request. */
    AM_BRDCST_MSG_APP_EXIT ,                            /* An application is exited*/      //added by msz00442@080214 for killing related application
    AM_BRDCST_MSG_ENTER_FAKE_STANDBY,                  /* The system is ready to enter fake standby mode */
    AM_BRDCST_MSG_EXIT_FAKE_STANDBY,                   /* The system is ready to exit fake standby mode */
    AM_BRDCST_MSG_MAX
}   AM_BRDCST_MSG_T;


/*-----------------------------------------------------------------------------
                    functions declarations
 ----------------------------------------------------------------------------*/

extern VOID  u_am_init (AMB_REGISTER_INFO_T* pt_reg);
extern VOID u_am_unint(VOID);
extern INT32 u_am_get_app_handle_from_name (HANDLE_T*    pt_handle, const CHAR*    ps_name);

#endif /* _U_AM_H_ */
