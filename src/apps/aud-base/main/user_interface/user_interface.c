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
#include "u_user_interface.h"
/* private */
#include "user_interface.h"
#include "user_interface_cli.h"

static USER_INTERFACE_OBJ_T t_g_user_interface               = {0};
static UINT16 ui2_g_user_interface_dbg_level = DBG_INIT_LEVEL_APP_USER_INTERFACE;
static BOOL f_is_factory_reset = FALSE;
static BOOL f_is_wifi_setup = FALSE;
static BOOL f_is_bt_paring = FALSE;


/*-----------------------------------------------------------------------------
 * private methods declarations
 *---------------------------------------------------------------------------*/
static INT32 _user_interface_start (
                    const CHAR*                 ps_name,
                    HANDLE_T                    h_app
                    );
static INT32 _user_interface_exit (
                    HANDLE_T                    h_app,
                    APP_EXIT_MODE_T             e_exit_mode
                    );
static INT32 _user_interface_process_msg (
                    HANDLE_T                    h_app,
                    UINT32                      ui4_type,
                    const VOID*                 pv_msg,
                    SIZE_T                      z_msg_len,
                    BOOL                        b_paused
                    );
/*---------------------------------------------------------------------------
 * Name
 *      a_user_interface_register
 * Description      -
 * Input arguments  -
 * Output arguments -
 * Returns          -
 *---------------------------------------------------------------------------*/
VOID a_user_interface_register(
                AMB_REGISTER_INFO_T*            pt_reg
                )
{
    if (t_g_user_interface.b_app_init_ok == TRUE) {
        return;
    }

    /* Application can only use middleware's c_ API */
    memset(pt_reg->s_name, 0, sizeof(CHAR)*(APP_NAME_MAX_LEN + 1));
    strncpy(pt_reg->s_name, USER_INTERFACE_THREAD_NAME, APP_NAME_MAX_LEN);
    pt_reg->t_fct_tbl.pf_init                   = _user_interface_start;
    pt_reg->t_fct_tbl.pf_exit                   = _user_interface_exit;
    pt_reg->t_fct_tbl.pf_process_msg            = _user_interface_process_msg;

    pt_reg->t_desc.ui8_flags                    = DEFAULT_THREAD_FLAG;
    pt_reg->t_desc.t_thread_desc.z_stack_size   = DEFAULT_STACK_SIZE;
    pt_reg->t_desc.t_thread_desc.ui1_priority   = DEFAULT_THREAD_PRIORITY;
    pt_reg->t_desc.t_thread_desc.ui2_num_msgs   = DEFAULT_NUM_MSGS;

    pt_reg->t_desc.ui2_msg_count                = DEFAULT_MSGS_COUNT;
    pt_reg->t_desc.ui2_max_msg_size             = DEFAULT_MAX_MSGS_SIZE;
    return;
}

UINT16 user_interface_get_dbg_level(VOID)
{
    return (ui2_g_user_interface_dbg_level | DBG_LAYER_APP);
}

VOID user_interface_set_dbg_level(UINT16 ui2_db_level)
{
    ui2_g_user_interface_dbg_level = ui2_db_level;
}


static INT32 _user_interface_start (
                    const CHAR*                 ps_name,
                    HANDLE_T                    h_app
                    )
{
    INT32 i4_ret;
    pthread_t      t_key_monitor_thread;//,t_key_monitor_thread_1,t_key_monitor_thread_2;


    memset(&t_g_user_interface, 0, sizeof(USER_INTERFACE_OBJ_T));
    t_g_user_interface.h_app = h_app;


    if (t_g_user_interface.b_app_init_ok)
    {
        return AEER_OK;
    }
#ifdef CLI_SUPPORT
    i4_ret = user_interface_cli_attach_cmd_tbl();
    if ((i4_ret != CLIR_NOT_INIT) && 
        (i4_ret != CLIR_OK))
    {
        DBG_ERROR((UI_TAG"Err: user_interface_cli_attach_cmd_tbl() failed, ret=%ld\r\n",i4_ret ));
        return AEER_FAIL;
    }
    user_interface_set_dbg_level(DBG_INIT_LEVEL_APP_USER_INTERFACE);
#endif/* CLI_SUPPORT */

    t_g_user_interface.b_app_init_ok = TRUE;

    DBG_ERROR(("[user_interface]<fox> %s: #%d: create event monitor\n", __FUNCTION__, __LINE__));

        /*create key event monitor thread*/
    i4_ret = pthread_create(&t_key_monitor_thread, NULL, user_interface_key_event_monitor_thread, NULL);
    if (0 != i4_ret)
    {
        DBG_ERROR(("[user_interface] %s: #%d: return %ld\n", __FUNCTION__, __LINE__, i4_ret));
        return i4_ret;
    }

    return AEER_OK;
}

static INT32 _user_interface_exit (
                    HANDLE_T                    h_app,
                    APP_EXIT_MODE_T             e_exit_mode
                    )
{
    INT32 i4_ret;

    t_g_user_interface.b_app_init_ok = FALSE;

    return AEER_OK;
}

/*---------------------------------------------------------------------------
 * Name
 *      _user_interface_process_msg
 * Description      -
 * Input arguments  -
 * Output arguments -
 * Returns          -
 *---------------------------------------------------------------------------*/
static INT32 _user_interface_process_msg (
                    HANDLE_T                    h_app,
                    UINT32                      ui4_type,
                    const VOID*                 pv_msg,
                    SIZE_T                      z_msg_len,
                    BOOL                        b_paused
                    )
{
    UCHAR* pc_name;
    UINT32             i4_ret;
    const CHAR *pc_keysta, *pc_keyval;
    DBG_API_IN;

    if (t_g_user_interface.b_app_init_ok == FALSE) {
        return AEER_FAIL;
    }

    if (ui4_type < AMB_BROADCAST_OFFSET)
    {
        switch (ui4_type){
        default:
                break;
        }
    }
    else
    {
        switch (ui4_type){
        case E_APP_MSG_TYPE_DUT_STATE:
                {
                    // should get power on/off msg here
                }
                break;
        case E_APP_MSG_TYPE_USB_DEV:
                {
                    // should get usb plug in /out, file system mount/unmount msg.
                }
                break;
        default:
                break;
        }
        DBG_INFO((UI_TAG"am broadcast message\n"));
    }
    DBG_API_OUT;
    return AEER_OK;
}

VOID u_ui_set_factory_reset_flag(BOOL flag)
{
    f_is_factory_reset = flag;
}
BOOL u_ui_get_factory_reset_flag(VOID)
{
    return f_is_factory_reset;
}
VOID u_ui_set_wifi_setup_flag(BOOL flag)
{
    f_is_wifi_setup = flag;
}
BOOL u_ui_get_wifi_setup_flag(VOID)
{
    return f_is_wifi_setup;
}
VOID u_ui_set_bt_paring_flag(BOOL flag)
{
    f_is_bt_paring = flag;
}
BOOL u_ui_get_bt_paring_flag(VOID)
{
    return f_is_bt_paring;
}


