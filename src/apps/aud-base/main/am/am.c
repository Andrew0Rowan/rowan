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

// please do not add any silly code in this file, pls.

/*-----------------------------------------------------------------------------
                    include files
-----------------------------------------------------------------------------*/
#include "u_assert.h"
#include "u_aee.h"
#include "u_amb.h"
#include "u_am.h"
#include "u_dbg.h"
#include "u_common.h"
#include "am.h"

/*-----------------------------------------------------------------------------
                    defines
-----------------------------------------------------------------------------*/
#define RET_ON_ERR(_expr) if(_expr != 0){DBG_ERROR (("<AM> ERR: file = %s, line = %d, reason = %d\n\r", __FILE__, __LINE__, _expr)); return -1;}

static INT32 _am_broadcast_msg (AMB_BROADCAST_MSG_T*    pt_msg,  BOOL   b_sync);
static INT32 _am_dispatch_msg (AMB_DISPATCH_MSG_T*    pt_msg);

/*-----------------------------------------------------------------------------
                    data declaraions
 ----------------------------------------------------------------------------*/
static AM_T  t_g_am;

/*-----------------------------------------------------------------------------
                    function prototypes
 ----------------------------------------------------------------------------*/
/* AEE callback functions */
static BOOL  _am_app_nfy_fct (HANDLE_T h_app, VOID* pv_tag, APP_NFY_REASON_T  e_nfy_reason);
static INT32 _am_app_init_fct (const CHAR* ps_name, HANDLE_T h_app);
static INT32 _am_app_exit_fct (HANDLE_T h_app, APP_EXIT_MODE_T e_exit_mode);
static INT32 _am_app_process_msg_fct (HANDLE_T h_app, UINT32 ui4_type, const VOID* pv_msg, SIZE_T z_msg_len, BOOL b_paused);
static INT32 _am_app_send_msg (UINT32 ui4_type, const VOID* pv_msg, SIZE_T z_msg_len);

static INT32 _am_start_app (const CHAR* ps_name);
static INT32 _am_exit_app (const CHAR* ps_name);


static BOOL  _am_is_time_stamp_valid (UINT32 ui4_time_stamp);


/*-----------------------------------------------------------------------------
                    public function declarations
 ----------------------------------------------------------------------------*/
/*-----------------------------------------------------------------------------
 * Name: am_set_dbg_level
 *
 * Description: This API sets the debug level of Application Manager.
 *
 * Inputs: ui2_dbg_level     Debug level to be set.
 *
 * Outputs: -
 *
 * Returns: -
 ----------------------------------------------------------------------------*/
VOID am_set_dbg_level (UINT16 ui2_dbg_level)
{
    t_g_am.ui2_dbg_level = ui2_dbg_level;
}

/*-----------------------------------------------------------------------------
 * Name: am_get_dbg_level
 *
 * Description: This API gets the current debug level of Application Manager.
 *
 * Inputs: -
 *
 * Outputs: -
 *
 * Returns: Current debug level
 ----------------------------------------------------------------------------*/
UINT16 am_get_dbg_level (VOID)
{
    return t_g_am.ui2_dbg_level | DBG_LAYER_APP;
}

/*----------------------------------------------------------------------------
 * Name: am_set_tms_level
 *
 * Description: This API sets time measure level for Application Manager.
 *
 * Inputs: ui2_tms_level       Time measure level to be set.
 *
 * Outputs:
 *
 * Returns:
 ----------------------------------------------------------------------------*/
VOID am_set_tms_level (UINT16 ui2_tms_level)
{
    t_g_am.ui2_tms_level = ui2_tms_level;
}

/*----------------------------------------------------------------------------
 * Name: am_get_tms_level
 *
 * Description: This API gets current time measure level of Application Manager.
 *
 * Inputs:
 *
 * Outputs:
 *
 * Returns: Current time measure level
 ----------------------------------------------------------------------------*/
UINT16 am_get_tms_level (VOID)
{
    return t_g_am.ui2_tms_level;
}

INT32 am_get_app_id(VOID)
{
    return t_g_am.ui4_app_id;
}
const CHAR* am_get_app_name(INT32 i4_id)
{
    return t_g_am.as_app_name[i4_id];
}

/*-----------------------------------------------------------------------------
                    private function declarations
 ----------------------------------------------------------------------------*/
/*-----------------------------------------------------------------------------
 * Name: _am_app_send_msg
 *
 * Description: The funcation sends a message to the message queue of application
 *              manager.
 *
 * Inputs:  ui4_type            Contains the type of the message.
 *          pv_msg              References the message data.
 *          z_msg_len           Contains the length (in bytes) of pv_msg.
 *
 * Outputs: -
 *
 * Returns: 0                   Successful.
 *          Any other values    Failed.
 ----------------------------------------------------------------------------*/
static INT32 _am_app_send_msg (UINT32         ui4_type,
                               const VOID*    pv_msg,
                               SIZE_T         z_msg_len)
{
    INT32     i4_res;
    UINT32    ui4_retry;

    i4_res    = AEER_OK;
    ui4_retry = 0;

    while (ui4_retry <= MSGQ_RETRY_TIMES)
    {
         i4_res = u_app_send_msg (t_g_am.h_am,
                                  ui4_type,
                                  pv_msg,
                                  z_msg_len,
                                  _am_app_nfy_fct,
                                  NULL /* Asynchronous send */);

         if (i4_res == AEER_OK)
         {
             break;
         }

         u_thread_delay (MSGQ_RETRY_DELAY);
         ui4_retry ++;
    }

    if (i4_res != AEER_OK)
    {
        DBG_ERROR (("<AM> ERR: send message = %d failed = %d\n\r", ui4_type, i4_res));
    }

    return (i4_res == AEER_OK) ? AMR_OK : AMR_FAIL;
}
/*-----------------------------------------------------------------------------
 * Name: _am_app_nfy_fct
 *
 * Description: This function is called in the context of the application
 *              manager when a message is received, and when a message is
 *              processed. When a message is received, this API will be called
 *              with one of the APP_NFY_PRE_* notifiction reasons. In that case,
 *              this API should return TRUE if the message is to be ignored, or
 *              FALSE is the message is to be processed. If the message is
 *              processed, then this API will be called again, this time after
 *              the message is processed, with one of the APP_NFY_*_OK or
 *              APP_NFY_*_FAILED notification reasons.
 *
 * Inputs:  h_app               Contains the application handle.
 *          pv_tag              References a tag value. An asynchronous call is
 *                              made if pv_tag is NULL, or a synchronous call is
 *                              made if pv_tag is not a NULL pointer.
 *          e_nfy_reason        Contains the notification reason.
 *
 * Outputs: -
 *
 * Returns: AEER_OK             Routine successful.
 *          Any other value     Routine failed.
 ----------------------------------------------------------------------------*/
static BOOL _am_app_nfy_fct (HANDLE_T            h_app,
                             VOID*               pv_tag,
                             APP_NFY_REASON_T    e_nfy_reason)
{
    /*
    TRUE  = ignore message
    FALSE = process message
    */

    switch (e_nfy_reason)
    {
        case APP_NFY_INIT_OK:
        case APP_NFY_EXIT_OK:
        case APP_NFY_PROCESS_MSG_OK:
        {
            if (pv_tag != NULL) /* Synchronous call */
            {
                *((BOOL*) pv_tag) = TRUE; /* Successful */

                _CONTINUE (t_g_am.h_bin_sema); /* Unlock semaphore. */
            }
        }
        break;

        case APP_NFY_INIT_FAILED:
        case APP_NFY_EXIT_FAILED:
        case APP_NFY_PROCESS_MSG_FAILED:
        {
            if (pv_tag != NULL) /* Synchronous call */
            {
                *((BOOL*) pv_tag) = FALSE; /* Failed */

                _CONTINUE (t_g_am.h_bin_sema); /* Unlock semaphore. */
            }
        }
        break;

        case APP_NFY_PRE_INIT:
        case APP_NFY_PRE_EXIT:
        case APP_NFY_PRE_PROCESS_MSG:
        {
            /* Do nothing. */
        }
        break;

        default: /* Unknown messages */
        {
            DBG_ERROR (("<AM> ERR: unknown message, handle = %d, tag=%p\n\r",
                       (int) h_app, pv_tag));
        }
        break;
    }

    return FALSE;
}

/*-----------------------------------------------------------------------------
 * Name: _am_app_init_fct
 *
 * Description: This function is called in the context of the application
 *              manager when the application starts.
 *
 * Inputs:  ps_name             References the application's name.
 *          h_app               Contains the application handle.
 *
 * Outputs: -
 *
 * Returns: AEER_OK             Routine successful.
 *          Any other value     Routine failed.
 ----------------------------------------------------------------------------*/
static INT32 _am_app_init_fct (const CHAR*    ps_name,
                               HANDLE_T       h_app)
{
    INT32                   i4_res;
    /* Store Application Manager handle. */
    t_g_am.h_am = h_app;

    /* Initialize Application Manager Broker. */
    RET_ON_ERR (u_amb_init());

    /* Create a mutex semaphore. */
    RET_ON_ERR (u_sema_create (& t_g_am.h_mtx_sema,
                               X_SEMA_TYPE_MUTEX,
                               X_SEMA_STATE_UNLOCK /* Unlock */));

    /* Create a binary semaphore. */
    RET_ON_ERR (u_sema_create (& t_g_am.h_bin_sema,
                               X_SEMA_TYPE_BINARY,
                               X_SEMA_STATE_LOCK /* Lock */));
#ifdef CLI_SUPPORT
	/* Initialize cli. */
    RET_ON_ERR (am_cli_init ());
	am_set_dbg_level(DBG_LEVEL_ERROR);
#endif

    return AMR_OK;
}


/*-----------------------------------------------------------------------------
 * Name: _am_app_exit_fct
 *
 * Description: This function is called in the context of the application
 *              manager when the application quits. This API should be
 *              responsible for saving data, clean-up, etc.
 *
 * Inputs:  h_app               Contains the application handle.
 *          e_exit_mode         Contains the exit mode.
 *
 * Outputs: -
 *
 * Returns: AEER_OK             Routine successful.
 *          Any other value     Routine failed.
 ----------------------------------------------------------------------------*/
static INT32 _am_app_exit_fct (HANDLE_T           h_app,
                               APP_EXIT_MODE_T    e_exit_mode)
{
    INT32  i4_res;

#if CLI_SUPPORT
    am_cli_uninit();
#endif
    /* Delete semaphores. */
    if (t_g_am.h_mtx_sema != NULL_HANDLE)
    {
        i4_res = u_sema_delete (t_g_am.h_mtx_sema);

        if (i4_res != OSR_OK)
        {
            DBG_ERROR (("<AM> ERR: delete semaphore failed = %d\n\r", i4_res));
        }
        else
        {
            t_g_am.h_mtx_sema = NULL_HANDLE;
        }
    }

    if (t_g_am.h_bin_sema != NULL_HANDLE)
    {
        i4_res = u_sema_delete (t_g_am.h_bin_sema);

        if (i4_res != OSR_OK)
        {
            DBG_ERROR (("<AM> ERR: delete semaphore failed = %d\n\r", i4_res));
        }
        else
        {
            t_g_am.h_bin_sema = NULL_HANDLE;
        }
    }
#if CLI_SUPPORT
    amb_cli_uninit();
#endif
    /* Release resources in Application Manager Broker. */
    u_amb_destroy ();


    return AEER_OK;
}

/*-----------------------------------------------------------------------------
 * Name: _am_app_process_msg_fct
 *
 * Description: This function is called in the context of the application
 *              manager when a message is processed.
 *
 * Inputs:  h_app               Contains the application handle.
 *          ui4_type            Contains the type of the data contained in
 *                              pv_msg, or simple a request type. Values are
 *                              defined by the application.
 *          pv_msg              References the message data.
 *          z_msg_len           Contains the length (in bytes) of pv_msg.
 *          b_paused            Sepcifies whether the application is currently
 *                              paused.
 *
 * Outputs: -
 *
 * Returns: AEER_OK             Routine successful.
 *          Any other value     Routine failed.
 ----------------------------------------------------------------------------*/
static INT32 _am_app_process_msg_fct (HANDLE_T       h_app,
                                      UINT32         ui4_type,
                                      const VOID*    pv_msg,
                                      SIZE_T         z_msg_len,
                                      BOOL           b_paused)
{
    INT32                  i4_res;
    AMB_BROADCAST_MSG_T    t_msg;


    i4_res = AEER_OK;


    if(ui4_type < AMB_REQUEST_TOTAL_NUMBER)
    {
        switch (ui4_type)
        {
            case AMB_REQUEST_START:                   /* Start request */
            {
                i4_res = _am_start_app ((const CHAR*) pv_msg);
            }
            break;

            case AMB_REQUEST_EXIT:                    /* Exit request */
            {
                i4_res = _am_exit_app ((const CHAR*) pv_msg);
            }
            break;

			case AMB_REQUEST_REG:
			{
				strncpy (t_g_am.as_app_name[t_g_am.ui4_app_id], (char *)pv_msg,APP_NAME_MAX_LEN+1);		
				t_g_am.as_app_name[t_g_am.ui4_app_id][APP_NAME_MAX_LEN] = '\0';		
				t_g_am.ui4_app_id ++;
			}
			break;
            case AMB_REQUEST_DISPATCH_MSG:            /* Dispatch message */
            {
                i4_res = _am_dispatch_msg ((AMB_DISPATCH_MSG_T*) pv_msg);
            }
            break;
            case AMB_REQUEST_BROADCAST_MSG:           /* Broadcast message */
            {
                i4_res = _am_broadcast_msg ((AMB_BROADCAST_MSG_T*) pv_msg, FALSE);
            }
            break;

	        default:   /* Unknown request */
            {
                i4_res = AMR_UNKNOWN_REQUEST;
            }
            break;
        }
    }
    else
    {
        switch (ui4_type)
        {
        
          default:   /* Unknown request */
          {
              i4_res = AMR_INV_ARG;
          }
          break;
        }
    }

    return (i4_res == AMR_OK) ? AEER_OK : AEER_FAIL;
}

/*-----------------------------------------------------------------------------
 * Name: _am_is_time_stamp_valid
 *
 * Description: This function checks if time stamp is valid.
 *
 * Inputs:  pt_key_event         System key event structure.
 *
 * Outputs: -
 *
 * Returns: TRUE                 Valid.
 *          FALSE                Invalid.
 ----------------------------------------------------------------------------*/
static BOOL _am_is_time_stamp_valid (UINT32    ui4_time_stamp)
{
    UINT32    ui4_current_tick;
    UINT32    ui4_delta_time;
    BOOL      b_valid;

    b_valid = FALSE;

    if (ui4_time_stamp == 0) /* The key should always be processed. */
    {
        b_valid = TRUE;
    }
    else
    {
        /* Check if the key is out-of-date. */
        ui4_current_tick = u_os_get_sys_tick (); /* Get current system tick. */

        if (ui4_current_tick >= ui4_time_stamp) /* Current system tick must be greater than time stamp. */
        {
            /* Calculate delta time. */
            ui4_delta_time = (ui4_current_tick - ui4_time_stamp) * u_os_get_sys_tick_period ();

            if (ui4_delta_time <= MAX_KEY_DELAY) /* Check if delta time is greater than maximum delay. */
            {
                b_valid = TRUE;
            }
        }
    }

    return b_valid;
}

/*-----------------------------------------------------------------------------
 * Name: _am_start_app
 *
 * Description: This function registers the application to the AEE.
 *
 * Inputs:  ps_name             Application name.
 *
 * Outputs: -
 *
 * Returns: AMR_OK              Routine successful.
 *          AMR_FAIL            Routine failed.
 ----------------------------------------------------------------------------*/
static INT32 _am_start_app (const CHAR*    ps_name)
{
    INT32                   i4_res;
    BOOL                    b_ok, b_found_app;
    UINT32                  ui4_i, ui4_app_id;
    APP_DESC_T              t_app_desc;
    AMB_REGISTER_INFO_T*    pt_reg;

    i4_res      = AMR_OK;
    b_found_app = FALSE;
    b_ok        = FALSE;//mtk70599@090417 klocwork
    ui4_app_id  = 0;

    pt_reg = u_amb_get_register_info (ps_name);

    if (pt_reg != NULL) /* Registration information is found. */
    {
        for (ui4_i = 0; ui4_i < t_g_am.ui4_app_id; ui4_i ++)
        {
            if (strcmp(ps_name, t_g_am.as_app_name[ui4_i]) == 0)
            {
                ui4_app_id = ui4_i;

                b_found_app = TRUE;

                break;
            }
        }

        if (b_found_app) /* Application is found. */
        {
            if (t_g_am.ah_app[ui4_app_id] == NULL_HANDLE) /* Application has not been created. */
            {
                /* Register to AEE. */
                t_app_desc.ui8_flags                     = ~((UINT64)0);

                t_app_desc.t_thread_desc.z_stack_size    = pt_reg->t_desc.t_thread_desc.z_stack_size;
                t_app_desc.t_thread_desc.ui1_priority    = pt_reg->t_desc.t_thread_desc.ui1_priority;
                t_app_desc.t_thread_desc.ui2_num_msgs    = pt_reg->t_desc.t_thread_desc.ui2_num_msgs;

                t_app_desc.ui4_app_group_id              = 0;
                t_app_desc.ui4_app_id                    = ui4_app_id+1; /* ID 0 is used by Application Manager */
                t_app_desc.ui2_msg_count                 = pt_reg->t_desc.ui2_msg_count;

                if (pt_reg->t_desc.ui2_max_msg_size < (BROADCAST_MSG_MAX_LEN)) /* Message size is less than BROADCAST_MSG_MAX_LEN. */
                {
                    t_app_desc.ui2_max_msg_size          = BROADCAST_MSG_MAX_LEN;
                }
                else
                {
                    t_app_desc.ui2_max_msg_size          = pt_reg->t_desc.ui2_max_msg_size;
                }

                i4_res = u_app_start (& pt_reg->t_fct_tbl,
                                      & t_app_desc,
                                      pt_reg->s_name,
                                      _am_app_nfy_fct,
                                      & b_ok, /* Synchronous call */
                                      & t_g_am.ah_app[ui4_app_id]);

                if (i4_res == AEER_OK)
                {
                    /* Wait until AEE starts the application. */
#ifdef CONFIG_TIME_MEASUREMENT
                    TMS_BEGIN (TMS_NAME_APP_INIT);
#endif /* CONFIG_TIME_MEASUREMENT */

                    _WAIT (t_g_am.h_bin_sema);

#ifdef CONFIG_TIME_MEASUREMENT
                    TMS_END (TMS_NAME_APP_INIT);
#endif /* CONFIG_TIME_MEASUREMENT */

                    if (! b_ok) /* Start application failed. */
                    {
                        DBG_ERROR (("<AM> ERR: start %s failed\n\r", ps_name));

                        t_g_am.ah_app[ui4_app_id] = NULL_HANDLE;

                        i4_res = AMR_FAIL;
                    }
                }
                else /* Start application request failed. */
                {
                   t_g_am.ah_app[ui4_app_id] = NULL_HANDLE;

                   DBG_ERROR (("<AM> ERR: start %s failed = %d\n\r", ps_name, i4_res));
                }

                i4_res = (i4_res == AEER_OK) ? AMR_OK : AMR_FAIL;
            }
            else /* The application has been started. */
            {
                i4_res = AMR_ALREADY_STARTED;
            }
        }
        else /* Cannot find application. */
        {
            DBG_ERROR (("<AM> ERR: cannot find %s or the application had been started = %d\n\r", ps_name, i4_res));

            i4_res = AMR_NOT_AUTHORIZED;
        }
    }
    else /* Cannot find registration information. */
    {
        DBG_ERROR (("<AM> ERR: cannot find registration information of %s\n\r", ps_name));

        i4_res = AMR_NOT_AUTHORIZED;
    }

    return i4_res;
}
/*-----------------------------------------------------------------------------
 * Name: _am_exit_app
 *
 * Description: This function exits the requested application.
 *
 * Inputs:  ps_name             References application name.
 *
 * Outputs: -
 *
 * Returns: AMR_OK              Routine successful.
 *          AMR_FAIL            Routine failed.
 *          AMR_NOT_AUTHORIZED  Not registered.
 ----------------------------------------------------------------------------*/
static INT32 _am_exit_app (const CHAR*    ps_name)
{
    INT32     i4_res;
    UINT32    ui4_retry;
    UINT32    ui4_i;
    UINT32    ui4_app_id;
    BOOL      b_found_app;
    BOOL      b_ok;

    i4_res      = AMR_OK;
    b_found_app = FALSE;
    b_ok        = FALSE;//mtk70599@090417 klocwork
    ui4_app_id  = 0;

    DBG_INFO (("<AM>Call _am_exit_app %s \n\r", ps_name));

    for (ui4_i = 0; ui4_i < t_g_am.ui4_app_id; ui4_i ++)
    {
        /* The application must have been created by the AEE. */
        if ((t_g_am.ah_app[ui4_i] != NULL_HANDLE) &&
            (strcmp (ps_name, t_g_am.as_app_name[ui4_i]) == 0))
        {
            ui4_app_id = ui4_i;
            b_found_app = TRUE;

            break;
        }
    }

    if (! b_found_app)
    {
        i4_res = AMR_NOT_AUTHORIZED;
    }
    else
    {
        DBG_INFO (("<AM> exit %s\n\r", ps_name));

        i4_res    = AMR_OK;
        ui4_retry = 0;

        while (ui4_retry <= MSGQ_RETRY_TIMES)
        {
            i4_res = u_app_exit (t_g_am.ah_app[ui4_app_id],
                                 APP_EXIT_DEMAND,
                                 _am_app_nfy_fct,
                                 & b_ok /* Synchronous call */);


            if (i4_res == AEER_OK)
            {
                /* Wait until AEE exits the application */
#ifdef CONFIG_TIME_MEASUREMENT
                TMS_BEGIN (TMS_NAME_APP_EXIT);
#endif /* CONFIG_TIME_MEASUREMENT */

                _WAIT (t_g_am.h_bin_sema);

#ifdef CONFIG_TIME_MEASUREMENT
                TMS_END (TMS_NAME_APP_EXIT);
#endif /* CONFIG_TIME_MEASUREMENT */

                t_g_am.ah_app[ui4_app_id] = NULL_HANDLE;

                break;
            }

            u_thread_delay (MSGQ_RETRY_DELAY);
            ui4_retry ++;
        }

        if ((i4_res != AEER_OK) || (! b_ok))
        {
            DBG_ERROR (("<AM> ERR: exit %s failed = %d\n\r", ps_name, i4_res));
        }

    }

    return i4_res;
}
/*-----------------------------------------------------------------------------
 * Name: _am_broadcast_lifecycle_msg
 *
 * Description: The function boradcasts a lifecycle message to all applications
 *              when the lifecycle state of applications are changed.
 *
 * Inputs:  ui4_app_id          Application ID.
 *          ui4_type            Broadcast message type.
 *
 * Outputs: -
 *
 * Returns: AMR_OK              Routine successful.
 *          AMR_FAIL            Routine failed.
 ----------------------------------------------------------------------------*/
static INT32 _am_broadcast_lifecycle_msg (UINT32    ui4_app_id,
                                          UINT32    ui4_type)
{
    INT32                  i4_res;
    SIZE_T                 z_msg_len;
    AMB_BROADCAST_MSG_T    t_msg;

    memset (& t_msg, 0, sizeof (AMB_BROADCAST_MSG_T));

    z_msg_len      = strlen (t_g_am.as_app_name[ui4_app_id]);
    t_msg.ui4_type = ui4_type;
    memcpy (t_msg.uc_msg, t_g_am.as_app_name[ui4_app_id], z_msg_len);

    if (z_msg_len < APP_NAME_MAX_LEN)
    {
        t_msg.uc_msg[z_msg_len] = 0; /* NULL terminated */
        t_msg.z_msg_len = z_msg_len + 1;
    }
    else
    {
        t_msg.z_msg_len = z_msg_len;
    }

    /* Asynchronous broadcast. */
    DBG_INFO (("<AM> broadcast lifecycle messages, app = %s, type = %d\n\r", t_g_am.as_app_name[ui4_app_id], ui4_type));

    i4_res = _am_broadcast_msg (& t_msg, FALSE);

    if (i4_res != AMR_OK)
    {
        
            DBG_ERROR (("<AM> ERR: broadcast message failed = %d\n\r", i4_res));

    }

    return i4_res;
}

/*-----------------------------------------------------------------------------
 * Name: _am_broadcast_msg
 *
 * Description: The function boradcasts a message to all applications that have
 *              been created by the AEE.
 *
 * Inputs:  pt_msg              References the message.
 *          b_sync              Synchronous broadcast.
 *
 * Outputs: -
 *
 * Returns: AMR_OK              Routine successful.
 *          AMR_FAIL            Routine failed.
 ----------------------------------------------------------------------------*/
static INT32 _am_broadcast_msg (AMB_BROADCAST_MSG_T*    pt_msg,  BOOL   b_sync)
{
    INT32     i4_res;
    UINT32    ui4_retry;
    UINT32    ui4_i;
    UINT32    ui4_app_id;
    BOOL      b_ok;

    i4_res = AMR_OK;

    for (ui4_i = 0; ui4_i < t_g_am.ui4_app_id; ui4_i ++)
    {
        /* Application who registered last will receive broadcast message first. */
        ui4_app_id = t_g_am.ui4_app_id - ui4_i - 1;

        ui4_retry = 0;

        while (ui4_retry <= MSGQ_RETRY_TIMES)
        {
            /* The application must have been created by the AEE. */
            if (t_g_am.ah_app[ui4_app_id] != NULL_HANDLE)
            {
                if (b_sync) /* Synchronously broadcast */
                {
                	//dbg_print("<DEBUG>broadcast message to:%s\n",t_g_am.as_app_name[ui4_app_id]);
                    i4_res = u_app_send_msg (t_g_am.ah_app[ui4_app_id],
                                             pt_msg->ui4_type,
                                             pt_msg->uc_msg,
                                             pt_msg->z_msg_len,
                                             _am_app_nfy_fct,
                                             & b_ok /* Synchronous call */);

                    if (i4_res == AEER_OK)
                    {
                        /* Wait until application processes this message. */
                        _WAIT (t_g_am.h_bin_sema);
						//dbg_print("<DEBUG>return from:%s\n",t_g_am.as_app_name[ui4_app_id]);
                        break;
                    }
					else
					{
						dbg_print("<DEBUG>error:%d\n",i4_res);
					}
                }
                else /* Ssynchronously broadcast */
                {
                    i4_res = u_app_send_msg (t_g_am.ah_app[ui4_app_id],
                                             pt_msg->ui4_type,
                                             pt_msg->uc_msg,
                                             pt_msg->z_msg_len,
                                             _am_app_nfy_fct,
                                             NULL /* Asynchronous call */);

                    if (i4_res == AEER_OK)
                    {
                        break;
                    }
                }
            }

            u_thread_delay (MSGQ_RETRY_DELAY);
            ui4_retry ++;
        }

        if (i4_res != AEER_OK)
        {
            DBG_ERROR (("<AM> ERR: broadcast message to %s failed = %d\n\r",
                       t_g_am.as_app_name[ui4_app_id],
                       i4_res));
        }
    }

    return (i4_res == AEER_OK) ? AMR_OK : AMR_FAIL;
}


/*-----------------------------------------------------------------------------
 * Name: _am_dispatch_msg
 *
 * Description: The function dispatches a message to a specific application.
 *
 * Inputs:  pt_msg              References the message.
 *
 * Outputs: -
 *
 * Returns: AMR_OK              Routine successful.
 *          AMR_FAIL            Routine failed.
 ----------------------------------------------------------------------------*/
static INT32 _am_dispatch_msg (AMB_DISPATCH_MSG_T*    pt_msg)
{
    INT32     i4_res;
    BOOL      b_found_app;
    BOOL      b_send_to_am;
    UINT32    ui4_app_id;
    UINT32    ui4_retry;
    UINT32    ui4_i;

    i4_res       = AMR_OK;
    b_found_app  = FALSE;
    b_send_to_am = FALSE;
    ui4_app_id   = 0;

    if (strcmp (pt_msg->ps_name, APP_MNGR_NAME) == 0)
    {
        b_send_to_am = TRUE;
    }
    else
    {
        for (ui4_i = 0; ui4_i < t_g_am.ui4_app_id; ui4_i ++)
        {
            /* The application must have been created by the AEE. */
            if ((t_g_am.ah_app[ui4_i] != NULL_HANDLE) &&
                (strcmp (pt_msg->ps_name, t_g_am.as_app_name[ui4_i]) == 0))
            {
                ui4_app_id = ui4_i;
                b_found_app = TRUE;

                break;
            }
        }
    }

    if ((! b_found_app) && (! b_send_to_am))
    {
        i4_res = AMR_NOT_AUTHORIZED;
    }
    else
    {
        ui4_retry = 0;

        while (ui4_retry <= MSGQ_RETRY_TIMES)
        {
            /* Dispatch message. */
            if (b_send_to_am) /* Send to Application Manager. */
            {
                i4_res = u_app_send_msg (t_g_am.h_am,
                                         pt_msg->ui4_type,
                                         pt_msg->uc_msg,
                                         pt_msg->z_msg_len,
                                         _am_app_nfy_fct,
                                         NULL /* Asynchronous call */);
            }
            else
            {
                i4_res = u_app_send_msg (t_g_am.ah_app[ui4_app_id],
                                         pt_msg->ui4_type,
                                         pt_msg->uc_msg,
                                         pt_msg->z_msg_len,
                                         _am_app_nfy_fct,
                                         NULL /* Asynchronous call */);
            }

            if (i4_res == AEER_OK)
            {
                break;
            }

            u_thread_delay (MSGQ_RETRY_DELAY);
            ui4_retry ++;
        }

        if (i4_res != AEER_OK)
        {
            if (b_send_to_am)
            {
                DBG_ERROR (("<AM> ERR: dispatch message to Application Manager failed = %d\n\r", i4_res));
            }
            else
            {
                DBG_ERROR (("<AM> ERR: dispatch message to %s failed = %d\n\r", t_g_am.as_app_name[ui4_app_id], i4_res));
            }
        }
    }

    return (i4_res == AEER_OK) ? AMR_OK : AMR_FAIL;
}
/*-----------------------------------------------------------------------------
 * Name:  u_am_get_app_handle_from_name
 *
 * Description: This API returns the handle of application.
 *
 * Inputs:  ps_name        The name of application.
 *
 * Outputs: pt_handle      The handle of application.
 *
 * Returns: AMR_OK              Successul.
 *          AMR_INV_ARG         Input buffer is NULL.
 *          AMR_NOT_AUTHORIZED  The application has not authorized
 ----------------------------------------------------------------------------*/
INT32 u_am_get_app_handle_from_name (
    HANDLE_T*    pt_handle,
    const CHAR*    ps_name)
{
    UINT8 ui1_idx = 0;

    if ((ps_name == NULL) || (pt_handle == NULL))
    {
        return AMR_INV_ARG;
    }
    if (t_g_am.ui4_app_id >= MAX_APP_NUM)////mtk70599@090223 fix kclocwork bug
    {
        DBG_ERROR (("<AM> ERR: Buffer overflow,outside the bounds. t_g_am.ui4_app_id= %d. Function:%s\n\r",
                   t_g_am.ui4_app_id,__FUNCTION__));
        return AMR_FAIL;
    }

    for (ui1_idx = 0; ui1_idx < t_g_am.ui4_app_id; ui1_idx++)
    {
        if (0 == strcmp(ps_name, t_g_am.as_app_name[ui1_idx]))
        {
            *pt_handle = t_g_am.ah_app[ui1_idx];

            return AMR_OK;
        }
    }

    return AMR_NOT_AUTHORIZED;
}

VOID u_am_unint(VOID)
{
    INT32 i4_ret;
    BOOL b_exit = FALSE;

    /* to do before exiting here*/
    i4_ret = u_app_exit(t_g_am.h_am, APP_EXIT_FORCE, NULL, &b_exit);
    if (AMR_OK != i4_ret)
    {
        DBG_ERROR(("<app unint>u_app_exit failed\n"));
    }

    return;
}

/*----------------------------------------------------------------------------
 * Name: u_am_init
 *
 * Description: This API performs the initialization of the application
 *              manager.
 *
 * Inputs:  pt_cfg            Init configurations.
 *
 * Outputs: pt_reg            References the registration structure.
 *
 * Returns: -
 ----------------------------------------------------------------------------*/
VOID u_am_init (AMB_REGISTER_INFO_T*    pt_reg)
{
    APP_DESC_T  t_am_desc = {
                    ~((UINT64)0),   /* ui8_flags */
                    {
                        8 * 1024,   /* z_stack_size */
                        175,        /* ui1_priority */
                        16          /* ui2_num_msgs */
                    },
                    0,              /* ui4_app_group_id */
                    0,              /* ui4_app_id */
                    32,             /* ui2_msg_count */
                    sizeof (AMB_BROADCAST_MSG_T)      /* ui2_max_msg_size */
                };

    /* Reset t_g_am */
    memset(&t_g_am, 0, sizeof(AM_T));

    /* Fill registration information. */
    strncpy (pt_reg->s_name, APP_MNGR_NAME,APP_NAME_MAX_LEN+1);
    pt_reg->s_name[APP_NAME_MAX_LEN] = '\0';

    /* Lifecycle callback functions */
    pt_reg->t_fct_tbl.pf_init        = _am_app_init_fct;
    pt_reg->t_fct_tbl.pf_exit        = _am_app_exit_fct;
    pt_reg->t_fct_tbl.pf_process_msg = _am_app_process_msg_fct;
    /* Application Manager descriptor */
    pt_reg->t_desc                   = t_am_desc;

}


