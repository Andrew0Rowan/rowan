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
 * $RCSfile: dm_linux.h,v $
 * $Revision:
 * $Date:
 * $Author:
 * $SWAuthor: Yan Wang $
 *
 * Description:
 *         This file contains all the transition effect interface APIs
 *---------------------------------------------------------------------------*/

#ifndef _DM_LINUX_H_
#define _DM_LINUX_H_
#include "u_dm.h"
#include "u_handle.h"
#include "u_lnk_list.h"
#include "u_app_priority.h"

#define HT_GROUP_DM    ((HANDLE_TYPE_T)(48 * HT_GROUP_SIZE))  /* Device Manager */
#define DMT_DEV_HANDLE (HT_GROUP_DM + ((HANDLE_TYPE_T) 0))

#define DM_CB_MSG_PRIORITY  1
#define DM_NFY_REQ_PRIORITY 1
#define DM_PRG_NFY_PRIORITY 1

#define DRVT_HW_IDE           ((DRV_TYPE_T)  85)
#define DRVT_HW_USB           ((DRV_TYPE_T)  87)
#define DRVT_USB_MASS_STORAGE ((DRV_TYPE_T) 101)
#define DRVT_HUB              ((DRV_TYPE_T) 105)

/* Device Manager thread and message queue definitions */
#define DM_CB_MSG_Q_NAME                      "dm_cb_msg_q"
#define DM_CB_MSG_Q_NUM                       ((UINT16) 256)

#define DM_NFY_REQ_Q_NAME                     "dm_nfy_req_q"
#define DM_NFY_REQ_Q_NUM                      ((UINT16) 256)

#define DM_CB_MSG_THREAD_NAME                 "dm_cb_msg_thrd"
#define DM_CB_MSG_THREAD_DEFAULT_PRIORITY     ((UINT8)  PRIORITY(PRIORITY_CLASS_HIGH, PRIORITY_LAYER_MIDDLEWARE, 0))
#define DM_CB_MSG_THREAD_DEFAULT_STACK_SIZE   ((SIZE_T) 4096)

#define DM_NFY_REQ_THREAD_NAME                "dm_nfy_req_thrd"
#define DM_NFY_REQ_THREAD_DEFAULT_PRIORITY    ((UINT8)  PRIORITY(PRIORITY_CLASS_HIGH, PRIORITY_LAYER_MIDDLEWARE, 0))
#define DM_NFY_REQ_THREAD_DEFAULT_STACK_SIZE  ((SIZE_T) 4096)

#define DM_AUTOMNT_THREAD_DEFAULT_PRIORITY    ((UINT8)  PRIORITY(PRIORITY_CLASS_HIGH, PRIORITY_LAYER_MIDDLEWARE, 0))
#define DM_AUTOMNT_THREAD_DEFAULT_STACK_SIZE  ((SIZE_T) 4096)

typedef enum _DM_CB_CAT_T
{
    DM_CB_CAT_DEV = 0x0,
    DM_CB_CAT_LDR = 0x100
} DM_CB_CAT_T;

/*  all the CB from device driver should be enumerated here */
typedef enum _DM_CB_TYPE_T
{
    DM_CB_DEV_UNSUPPORT    = DM_CB_CAT_DEV,
    DM_CB_DEV_FAIL,
    DM_CB_DEV_ATTACH,
    DM_CB_DEV_DETACH,

    DM_CB_DEV_PART_DETECTED,
    DM_CB_DEV_PART_REMOVED,
    DM_CB_DEV_MTP_DETECTED,
    DM_CB_DEV_MTP_REMOVED,

    DM_CB_TRAY_ERROR     = DM_CB_CAT_LDR,
} DM_CB_TYPE_T;

typedef enum _DM_MSG_TYPE_T
{
    DM_MSG_TYPE_NORMAL,
    DM_MSG_TYPE_SHUTDOWN,
}DM_MSG_TYPE_T;

#if 1
typedef struct _DM_NFY_REQ_T
{
    DM_MSG_TYPE_T e_msgtype;

    DM_EVENT_E  e_event;
} DM_NFY_REQ_T;
#else
typedef struct _DM_NFY_REQ_T
{
    DM_MSG_TYPE_T e_msgtype; //mtk40224 add msg type for uninit dm
    /*The first two are used to match the event filter. */

    DM_HW_TYPE_T  t_hw_type;
    DM_DEV_TYPE_T  t_dev_type;

    /*if the h_nfy is not NULL_HANDLE, it means the event is only for the specified event handler.*/
    HANDLE_T  h_nfy;

    DM_EVENT_E  e_event;

    /*event data type, prepared according to the evt type  before sent to queue.*/
    VOID *pv_data;
} DM_NFY_REQ_T;
#endif

typedef struct _DM_CB_MSG_T
{
    DM_MSG_TYPE_T e_msgtype; //mtk40224 add msg type for uninit dm
    DM_CB_TYPE_T  t_cb_type;
    DM_DEV_TYPE_T t_dev_type;

    UINT32 ui4_unit_id;
    UINT32 ui4_skt_no;

    UINT32 ui4_major;
    UINT32 ui4_minor;
    CHAR ps_dev_name[DM_MAX_LABEL_NAME_LEN];

    UINT32 ui4_dev_flag;

    VOID *pv_data;
    UINT32 ui4_data_sz;

    /*  This is used to identify the issuer device that issues the CB and
        *  in type of _DM_DEV_DESC_T.  Since the CB msgs are ordered in queue,
         *  we will not encounter the case that this points to an invalid parent. */
    VOID *pv_tag;
} DM_CB_MSG_T;

typedef struct _DM_MNT_DESC_T
{
    UINT32 ui4_part_idx;

    CHAR ps_part_name[DM_MAX_LABEL_NAME_LEN];
    CHAR ps_mnt_path[DM_MAX_LABEL_NAME_LEN];

    SLIST_ENTRY_T(_DM_MNT_DESC_T) t_link;
} DM_MNT_DESC_T;

typedef struct _DM_DEV_DESC_T
{
    HANDLE_T      h_dev;  /*  handle allocated for client */
    HANDLE_T      h_drv;  /*  handle get from Resource Manager */

    DM_HW_TYPE_T  t_hw_type;
    DM_DEV_TYPE_T t_dev_type;

    UINT32        ui4_dev_flag;

    struct _DM_DEV_DESC_T *pt_root_dev;
    struct _DM_DEV_DESC_T *pt_prnt_dev;

    UINT32 ui4_unit_id;
    UINT32 ui4_skt_no;

    UINT32 ui4_major;
    UINT32 ui4_minor;

    VOID *pv_data;
    UINT32 ui4_data_size;

    BOOL  b_avail;

    HANDLE_T  h_dev_root_lbl;  /*  "dev" entry in FM */

    CHAR ps_dev_name[DM_MAX_LABEL_NAME_LEN];

    SLIST_ENTRY_T(_DM_DEV_DESC_T) t_link;

    HANDLE_T h_automnt_sema;

    SLIST_T(_DM_MNT_DESC_T) t_mnt_list;
    HANDLE_T h_mnt_rwlock;

    HANDLE_T h_mnt_nfy;

    VOID *pv_resv;
} DM_DEV_DESC_T;

typedef struct _DM_CORE_T
{
    SLIST_T(_DM_DEV_DESC_T) t_dev_list;
    HANDLE_T h_dev_rwlock;

    HANDLE_T h_nfy_req_thrd;
    HANDLE_T h_nfy_req_q;

    HANDLE_T h_cb_msg_thrd;
    HANDLE_T h_cb_msg_q;

    BOOL b_init;
    UINT16 ui2_dbg_lvl;

    BOOL b_automnt_init;
    DM_COND_T *pt_automnt_cond;
    UINT32 ui4_automnt_cond_ns;
    BOOL b_nfy_thrd_exit;
    BOOL b_cb_thrd_exit;
} DM_CORE_T;

extern INT32 dmh_cb_msg_thread(VOID *pv_data);

#endif

