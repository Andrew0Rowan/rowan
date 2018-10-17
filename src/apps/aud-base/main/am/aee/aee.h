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
 * $RCSfile: aee.h,v $
 * $Revision: #1 $
 * $Date: 2016/03/07 $
 * $Author: bdbm01 $
 * $CCRevision: /main/DTV_X/DTV_X_HQ_int/2 $
 * $SWAuthor: Iolo Tsai $
 * $MD5HEX: f0818ab32032e8fd979475b10afbc7e5 $
 *
 * Description:
 *         This header file contains the AEE specific definitions that can be
 *         shared with other components but that are not exported to 3rd
 *         parties.
 *---------------------------------------------------------------------------*/

#ifndef _AEE_H_
#define _AEE_H_


/*-----------------------------------------------------------------------------
                    include files
 ----------------------------------------------------------------------------*/


#include "u_aee.h"
#include "u_os.h"
#include "u_handle.h"

/*-----------------------------------------------------------------------------
                    macros, defines, typedefs, enums
 ----------------------------------------------------------------------------*/

#define AEET_PRIMARY_AEE    (HT_GROUP_AEE + (HANDLE_TYPE_T) 0)
#define AEET_SECONDARY_AEE  (HT_GROUP_AEE + (HANDLE_TYPE_T) 1)

#define AEE_NAME_MAX_LENGTH  16

#define DBG_ABRT_CANNOT_CREATE_SEMAPHORE  0
#define DBG_ABRT_CANNOT_SET_AUTOFREE      1
#define DBG_ABRT_INVALID_MESSAGE          2

#undef DBG_LEVEL_MODULE
#define DBG_LEVEL_MODULE aee_get_dbg_level()

typedef struct _AEE_T
{
    struct _AEE_T*                      pt_next;
    HANDLE_T                    h_app;
    HANDLE_T                    h_parent_app;
    HANDLE_T                    h_aux;
    HANDLE_T                    h_msg_queue;
    HANDLE_T                    h_thread;
    UINT64                      ui8_flags;
    UINT32                      ui4_app_group_id;
    UINT32                      ui4_app_id;
    u_aee_app_receive_data_fct  pf_receive_data;
    VOID*                       pv_tag;
	
    CHAR                        s_name[AEE_NAME_MAX_LENGTH + 1];
}AEE_T;

typedef struct
{
    AEE_T*              pt_aee;
    HANDLE_T            h_app;
    u_aee_app_main_fct  pf_main;
    VOID*               pv_arg;
}   AEE_MAIN_ARG_T;

typedef struct
{
    AEE_T*                pt_aee;
    HANDLE_T              h_app;
    u_os_thread_main_fct  pf_main;
    VOID*                 pv_arg;
}   AEE_THREAD_CREATE_ARG_T;

typedef union 
{
    AEE_T * pt_aee;
    VOID *  pv_aee;
} RET_AEE_U;





/*-----------------------------------------------------------------------------
                    functions declarations
 ----------------------------------------------------------------------------*/

extern INT32 aee_init (VOID);

extern INT32 aee_uninit(VOID);   //Added by Junsong Wang, 20090804

extern INT32 aee_grab_handle_resource (UINT64     ui8_flags,
                                       UINT16     ui2_handles,
                                       HANDLE_T*  ph_aux);

extern INT32 aee_grab_resources (UINT64     ui8_flags,
                                 SIZE_T     z_memory,
                                 UINT64     ui8_files_size,
                                 UINT16     ui2_files,
                                 UINT16     ui2_handles,
                                 UINT16     ui2_threads,
                                 UINT16     ui2_semaphores,
                                 UINT16     ui2_msg_queues,
                                 HANDLE_T*  ph_aux);

extern INT32 aee_release_handle_resource (UINT16  ui2_handles);

extern INT32 aee_release_resources (SIZE_T  z_memory,
                                    UINT64  ui8_files_size,
                                    UINT16  ui2_files,
                                    UINT16  ui2_handles,
                                    UINT16  ui2_threads,
                                    UINT16  ui2_semaphores,
                                    UINT16  ui2_msg_queues);

extern INT32 aee_thread_create (HANDLE_T*             ph_th_hdl,
                                CHAR*                 ps_name,
                                SIZE_T                z_stacksize,
                                UINT8                 ui1_pri,
                                u_os_thread_main_fct  pf_main,
                                SIZE_T                z_arg_size,
                                VOID*                 pv_arg);

extern INT32 aee_get_thread (HANDLE_T   h_app,
                             HANDLE_T*  ph_thread);

extern INT32 aee_get_app_handle (HANDLE_T *  ph_app,
                      HANDLE_T h_thread);

extern const CHAR* aee_get_name (VOID);



extern INT32 aee_create_op (const AEE_APP_DESC_T*  pt_desc,
                            const CHAR*            ps_name,
                            HANDLE_T*              ph_app);

extern INT32 aee_get_handle_op (const CHAR*  ps_name,
                                HANDLE_T*    ph_app);

extern INT32 aee_send_data_op (HANDLE_T  h_app,
                               UINT32    ui4_data,
                               VOID*     pv_data,
                               SIZE_T    z_data_len);
#if AEE_RES_CONTROL_SUPPORT

extern INT32 aee_get_resource_info_op (HANDLE_T           h_app,
                                       AEE_RESOURCE_ID_T  e_res_id,
                                       VOID*              pv_res);
#endif
extern INT32 aee_get_msg_queue (HANDLE_T   h_aee,
                                HANDLE_T*  ph_msg_queue);

extern INT32 aee_set_msg_queue (HANDLE_T  h_aee,
                                HANDLE_T  h_msg_queue);

extern INT32 aee_grab_memory_resource (AEE_T*  pt_aee,
                                       SIZE_T  z_memory);

extern INT32 aee_release_memory_resource (AEE_T*  pt_aee,
                                          SIZE_T  z_memory);

extern UINT16 aee_get_dbg_level (VOID);

extern INT32 aee_set_dbg_level (UINT16  ui2_dbg_level);

extern VOID aee_cli_init (VOID);

extern VOID aee_list_aee_info (const AEE_T*  pt_aee);

extern AEE_T* aee_find_by_name (const CHAR*  ps_name);

extern VOID aee_list_all_aees_info (VOID);

extern VOID aee_mem_free_all_allocated_pointers (AEE_T*  pt_aee);

#endif /* _AEE_H_ */
