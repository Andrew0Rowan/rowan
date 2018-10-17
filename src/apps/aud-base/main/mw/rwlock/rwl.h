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
 * $RCSfile: _rwl.h,v $
 * $Revision: #1 $
 * $Date: 2016/07/22 $
 * $Author: bdbm01 $
 *
 * Description: 
 *         This header file contains Resource Manager lock related API
 *         definitions, which are exported to other Resource Manager
 *         components.
 *---------------------------------------------------------------------------*/

#ifndef __RWL_H_
#define __RWL_H_


/*-----------------------------------------------------------------------------
  include files
  -----------------------------------------------------------------------------*/

#include "u_common.h"
#include "u_dbg.h"
#include "u_handle.h"
#include "u_rwlock.h"
/*---------------------------------------------------------------------------
  Constants, enumerations, and macros
  ----------------------------------------------------------------------------*/

/* Error code. */
#define DBG_ABRT_CANNOT_CREATE_WR_WAIT_SEMAPHORE  ((UINT32) 0x00000010)
#define DBG_ABRT_CANNOT_CREATE_RD_WAIT_SEMAPHORE  ((UINT32) 0x00000011)
#define DBG_ABRT_INV_LOCK_STATE                   ((UINT32) 0x00000012)
#define DBG_ABRT_OUT_OF_MEMORY                    ((UINT32) 0x00000013)
#define DBG_ABRT_INVALID_HANDLE                   ((UINT32) 0x00000014)
#define DBG_ABRT_INVALID_HANDLE_TYPE              ((UINT32) 0x00000015)
#define DBG_ABRT_CANNOT_DELETE_WAIT_SEMAPHORE     ((UINT32) 0x00000016)
#define DBG_ABRT_CANNOT_LOCK_WAIT_SEMAPHORE       ((UINT32) 0x00000017)
#define DBG_ABRT_CANNOT_UNLOCK_WAIT_SEMAPHORE     ((UINT32) 0x00000018)

/* Handle type for the Read-Write Lock */
#define READ_WRITE_LOCK  (HT_GROUP_COMMON + ((HANDLE_TYPE_T) 1))

/*---------------------------------------------------------------------------
  Structure for the rw lock
  ----------------------------------------------------------------------------*/
    
typedef struct _MRSW_LOCK_T
{
    HANDLE_T h_sem_wait_wr;  /* Thread waiting queue to perform read/write */
    HANDLE_T h_sem_wait_rd;  /* operation on component database.           */

    UINT16   ui2_wr_access_cnt; /* write access and wait counters which are */
    UINT16   ui2_wr_wait_cnt; /* used to control access to shared variable. */

    UINT16   ui2_rd_access_cnt; /* read access and wait counters which are */
    UINT16   ui2_rd_wait_cnt; /* used to control access to shared variable. */

    UINT16   ui2_rd_inside_wr_cnt; /* read access count which occurs while 
                                      the same thread has already obtained
                                      write lock. */

    HANDLE_T h_wr_thread; /* The thread currently has the write lock. */

    BOOL     b_del_flag;  /* flag to indicate lock deletion in progress. */

} MRSW_LOCK_T ;



typedef union
{
    MRSW_LOCK_T * pt_MRSW_LOCK;
	void        * pv_MRSW_LOCK;
} RET_MRSW_LOCK_U;


/*-----------------------------------------------------------------------------
  functions declarations
  ----------------------------------------------------------------------------*/

extern BOOL rwl_handle_cb (HANDLE_T       h_obj,
                           HANDLE_TYPE_T  e_type,
                           VOID*          pv_obj,
                           VOID*          pv_tag,
                           BOOL           b_req_handle);


#endif /* __RWL_H_ */
