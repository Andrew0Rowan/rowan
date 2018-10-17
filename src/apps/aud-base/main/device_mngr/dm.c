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
#include "u_dm.h"
#include "dm.h"
#include "dm_cli.h"

static DM_OBJ_T t_g_dm    = {0};
static UINT16   ui2_g_dm_dbg_level = DBG_INIT_LEVEL_APP_DM;

/*-----------------------------------------------------------------------------
 * private methods declarations
 *---------------------------------------------------------------------------*/
static INT32 _dm_start (const CHAR *ps_name,HANDLE_T h_app);
static INT32 _dm_exit (HANDLE_T h_app,APP_EXIT_MODE_T e_exit_mode);
static INT32 _dm_process_msg (HANDLE_T   h_app,
                                    UINT32     ui4_type,
                                    const VOID *pv_msg,
                                    SIZE_T     z_msg_len,
                                    BOOL       b_paused);
/*---------------------------------------------------------------------------
 * Name
 * a_app_set_registration
 * Description      -
 * Input arguments  -
 * Output arguments -
 * Returns          -
 *---------------------------------------------------------------------------*/
VOID a_dm_register(AMB_REGISTER_INFO_T *pt_reg)
{
    if (TRUE == t_g_dm.b_app_init_ok)
    {
        return;
    }
    DBG_API_IN;
    /* Application can only use middleware's c_ API */
    memset(pt_reg->s_name, 0, sizeof(CHAR)*(APP_NAME_MAX_LEN + 1));
    strncpy(pt_reg->s_name, DM_THREAD_NAME, APP_NAME_MAX_LEN);
    pt_reg->t_fct_tbl.pf_init                   = _dm_start;
    pt_reg->t_fct_tbl.pf_exit                   = _dm_exit;
    pt_reg->t_fct_tbl.pf_process_msg            = _dm_process_msg;

    pt_reg->t_desc.ui8_flags                    = ~((UINT64)0);
    pt_reg->t_desc.t_thread_desc.z_stack_size   = DM_STACK_SIZE;
    pt_reg->t_desc.t_thread_desc.ui1_priority   = DM_THREAD_PRIORITY;
    pt_reg->t_desc.t_thread_desc.ui2_num_msgs   = DM_NUM_MSGS;

    pt_reg->t_desc.ui2_msg_count                = DM_MSGS_COUNT;
    pt_reg->t_desc.ui2_max_msg_size             = DM_MSGS_SIZE;

    return;
}

UINT16 dm_get_dbg_level(VOID)
{
    return (ui2_g_dm_dbg_level | DBG_LAYER_APP);
}

VOID dm_set_dbg_level(UINT16 ui2_db_level)
{
    ui2_g_dm_dbg_level = ui2_db_level;
}


static INT32 _dm_start (const CHAR *ps_name,HANDLE_T h_app)
{
    INT32 i4_ret;
    DBG_API_IN;
    memset(&t_g_dm, 0, sizeof(DM_OBJ_T));
    t_g_dm.h_app = h_app;

    if (t_g_dm.b_app_init_ok)
    {
        return AEER_OK;
    }

#ifdef CLI_SUPPORT
    i4_ret = dm_cli_attach_cmd_tbl();
    if ((CLIR_NOT_INIT != i4_ret) && (CLIR_OK != i4_ret))
    {
        DM_ERR("Err: dm_cli_attach_cmd_tbl() failed, ret=%ld\r\n",i4_ret);
        return AEER_FAIL;
    }
    dm_set_dbg_level(DBG_INIT_LEVEL_APP_DM);
#endif/* CLI_SUPPORT */

    dm_multi_thread_init();

    t_g_dm.b_app_init_ok = TRUE;
    DBG_API_OUT;

    return AEER_OK;
}

static INT32 _dm_exit (HANDLE_T h_app,APP_EXIT_MODE_T e_exit_mode)
{
    INT32 i4_ret;
    DBG_API_IN;

    t_g_dm.b_app_init_ok = FALSE;

    DBG_API_OUT;
    return AEER_OK;
}

/*---------------------------------------------------------------------------
 * Name
 *      _dm_process_msg
 * Description      -
 * Input arguments  -
 * Output arguments -
 * Returns          -
 *---------------------------------------------------------------------------*/
static INT32 _dm_process_msg (HANDLE_T   h_app,
                                    UINT32     ui4_type,
                                    const VOID *pv_msg,
                                    SIZE_T     z_msg_len,
                                    BOOL       b_paused)
{
    UCHAR      *pc_name;
    UINT32     i4_ret;
    const CHAR *pc_keysta, *pc_keyval;

    DBG_API_IN;

    if (FALSE == t_g_dm.b_app_init_ok)
    {
        return AEER_FAIL;
    }

    if (ui4_type < AMB_BROADCAST_OFFSET)
    {
        DM_INFO("dm receive unicast message[%lu]\n", ui4_type);
    }
    else
    {
        DM_INFO("dm receive broadcast message[%lu]\n", ui4_type);
    }

    DBG_API_OUT;

    return AEER_OK;
}

