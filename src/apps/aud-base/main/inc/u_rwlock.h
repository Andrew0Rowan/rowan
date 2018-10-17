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


/*----------------------------------------------------------------------------*
 * $RCSfile: u_rwlock.h,v $
 * $Revision: #1 $
 * $Date: 2016/07/22 $
 * $Author: bdbm01 $
 *
 * Description: 
 *           This header file contains Resource Manager lock related API
 *           definitions, which are exported to other Resource Manager
 *           components.
 *---------------------------------------------------------------------------*/

#ifndef _U_RWLOCK_H_
#define _U_RWLOCK_H_


/*-----------------------------------------------------------------------------
                    include files
-----------------------------------------------------------------------------*/

#include "u_common.h"
#include "u_handle.h"

/*---------------------------------------------------------------------------
 Constants, enumerations, and macros
----------------------------------------------------------------------------*/
    
#define    RWLR_OK                  ((INT32)  0) 
#define    RWLR_FAIL                ((INT32) -1) 
#define    RWLR_INVALID_HANDLE      ((INT32) -2) 
#define    RWLR_DELETE_IN_PROGRESS  ((INT32) -3)
#define    RWLR_WOULD_BLOCK         ((INT32) -4) 
 
/* Enumeration for read-write lock option */

typedef    enum
{
    RWL_OPTION_WAIT = 1,
    RWL_OPTION_NOWAIT 
} RWL_OPTION_T;

extern INT32 u_rwl_create_lock   (HANDLE_T* ph_rw_lock);
extern INT32 u_rwl_read_lock     (HANDLE_T h_rw_lock, RWL_OPTION_T e_option);
extern INT32 u_rwl_write_lock    (HANDLE_T h_rw_lock, RWL_OPTION_T e_option);
extern INT32 u_rwl_release_lock  (HANDLE_T h_rw_lock);
extern INT32 u_rwl_delete_lock   (HANDLE_T h_rw_lock);


extern BOOL  u_rwl_read_lock_grabbed(HANDLE_T  h_rw_lock);
extern BOOL  u_rwl_write_lock_grabbed(HANDLE_T h_rw_lock);

#endif /* _U_RWLOCK_H_ */
