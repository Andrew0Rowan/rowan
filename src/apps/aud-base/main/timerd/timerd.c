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


#include <string.h>
/* application level */
#include "u_amb.h"
#include "u_common.h"
#include "u_app_thread.h"
#include "u_cli.h"
#include "u_os.h"
#include "u_assert.h"
/* private */
#include "timerd.h"
#include "timerd_cli.h"

static TIMERD_OBJ_T t_g_timerd               = {0};
UINT16 ui2_g_timerd_dbg_level = DBG_LEVEL_ERROR;

/*-----------------------------------------------------------------------------
 * private methods declarations
 *---------------------------------------------------------------------------*/
static INT32 _timerd_start (
                    const CHAR*                 ps_name,
                    HANDLE_T                    h_app
                    );
static INT32 _timerd_exit (
                    HANDLE_T                    h_app,
                    APP_EXIT_MODE_T             e_exit_mode
                    );
static INT32 _timerd_process_msg (
                    HANDLE_T                    h_app,
                    UINT32                      ui4_type,
                    const VOID*                 pv_msg,
                    SIZE_T                      z_msg_len,
                    BOOL                        b_paused
                    );
/*---------------------------------------------------------------------------
 * Name
 *      a_app_set_registration
 * Description      -
 * Input arguments  -
 * Output arguments -
 * Returns          -
 *---------------------------------------------------------------------------*/
VOID a_timerd_register(
                AMB_REGISTER_INFO_T*            pt_reg
                )
{
    if (t_g_timerd.b_app_init_ok == TRUE) {
        return;
    }
	DBG_API_IN;
    /* Application can only use middleware's c_ API */
    memset(pt_reg->s_name, 0, sizeof(CHAR)*(APP_NAME_MAX_LEN + 1));
    strncpy(pt_reg->s_name, TIMERD_THREAD_NAME, APP_NAME_MAX_LEN);
    pt_reg->t_fct_tbl.pf_init                   = _timerd_start;
    pt_reg->t_fct_tbl.pf_exit                   = _timerd_exit;
    pt_reg->t_fct_tbl.pf_process_msg            = _timerd_process_msg;

    pt_reg->t_desc.ui8_flags                    = ~((UINT64)0);
    pt_reg->t_desc.t_thread_desc.z_stack_size   = 4096;
    pt_reg->t_desc.t_thread_desc.ui1_priority   = 200;
    pt_reg->t_desc.t_thread_desc.ui2_num_msgs   = 20;

    pt_reg->t_desc.ui2_msg_count                = 20;
    pt_reg->t_desc.ui2_max_msg_size             = 64;
    return;
}

UINT16 timerd_get_dbg_level(VOID)
{
    return (ui2_g_timerd_dbg_level | DBG_LAYER_APP);
}

VOID timerd_set_dbg_level(UINT16 ui2_db_level)
{
    ui2_g_timerd_dbg_level = ui2_db_level;
}


static INT32 _timerd_start (
                    const CHAR*                 ps_name,
                    HANDLE_T                    h_app
                    )
{
    INT32 i4_ret;
    DBG_API_IN;

    memset(&t_g_timerd, 0, sizeof(TIMERD_OBJ_T));
    t_g_timerd.h_app = h_app;


    if (t_g_timerd.b_app_init_ok)
    {
        return AEER_OK;
    }

#ifdef CLI_SUPPORT
    i4_ret = timerd_cli_attach_cmd_tbl();
    if ((i4_ret != CLIR_NOT_INIT) && (i4_ret != CLIR_OK))
    {
        DBG_ERROR((TIMERD_TAG"Err: ht_cli_attach_cmd_tbl() failed, ret=%ld\r\n",
            i4_ret ));
        return AEER_FAIL;
    }
#endif/* CLI_SUPPORT */

    t_g_timerd.b_app_init_ok = TRUE;
    DBG_API_OUT;

	ASSERT(os_timer_init()==0);

    return AEER_OK;
}

static INT32 _timerd_exit (
                    HANDLE_T                    h_app,
                    APP_EXIT_MODE_T             e_exit_mode
                    )
{
    INT32 i4_ret;
    DBG_API_IN;

    t_g_timerd.b_app_init_ok = FALSE;

    DBG_API_OUT;
    return AEER_OK;
}

/*---------------------------------------------------------------------------
 * Name
 *      _timerd_process_msg
 * Description      -
 * Input arguments  -
 * Output arguments -
 * Returns          -
 *---------------------------------------------------------------------------*/
static INT32 _timerd_process_msg (
                    HANDLE_T                    h_app,
                    UINT32                      ui4_type,
                    const VOID*                 pv_msg,
                    SIZE_T                      z_msg_len,
                    BOOL                        b_paused
                    )
{
    UCHAR* pc_name;
    TIMERD_MSG_T* pt_ht_msg;
    UINT32             i4_ret;
    const CHAR *pc_keysta, *pc_keyval;
    DBG_API_IN;

    if (t_g_timerd.b_app_init_ok == FALSE) {
        return AEER_FAIL;
    }

    if (ui4_type < AMB_BROADCAST_OFFSET)
    {
        DBG_INFO(("private message id is %d\n", pt_ht_msg->ui4_msg_id));
    }
    else
    {
        DBG_INFO(("am broadcast message id is %d\n", pt_ht_msg->ui4_msg_id));
    }
    DBG_API_OUT;
    return AEER_OK;
}


