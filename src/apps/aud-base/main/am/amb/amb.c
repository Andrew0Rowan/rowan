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
                    include files
-----------------------------------------------------------------------------*/
#include "u_os.h"
#include "u_amb.h"
#include "u_dbg.h"
#include "u_app_thread.h"
#include "u_handle.h"


/*-----------------------------------------------------------------------------
                    macros, defines, typedefs, enums
 ----------------------------------------------------------------------------*/
#define RET_ON_ERR(_expr) if(_expr != 0){DBG_ERROR (("<AMB> ERR: file = %s, line = %d, reason = %d\n\r", __FILE__, __LINE__, _expr)); return -1;}

/* synchronous operations */
#define  _SYNC_BEGIN(h_sema)    u_sema_lock(h_sema, X_SEMA_OPTION_WAIT)
#define  _SYNC_END(h_sema)      u_sema_unlock(h_sema)
#define  _WAIT(h_sema)          u_sema_lock(h_sema, X_SEMA_OPTION_WAIT)
#define  _CONTINUE(h_sema)      u_sema_unlock(h_sema)

/* message queue retry setting */
#define  MSGQ_RETRY_TIMES       10  /* retry times */
#define  MSGQ_RETRY_DELAY       10  /* retry delay (in milliseconds) */

#undef   DBG_LEVEL_MODULE
#define  DBG_LEVEL_MODULE       amb_get_dbg_level()
/*-----------------------------------------------------------------------------
                    data declaraions
 ----------------------------------------------------------------------------*/
static UINT16                   ui2_g_dbg_level = DBG_LEVEL_ERROR;    /* debug level */

/* registration informations */
static AMB_REGISTER_INFO_T*     pt_g_regs    = NULL;

/* Application Manager handle */
static HANDLE_T                 h_g_am       = NULL_HANDLE;

/* mutex semaphore */
static HANDLE_T                 h_g_mtx_sema = NULL_HANDLE;

/* binary semaphore */
static HANDLE_T                 h_g_bin_sema = NULL_HANDLE;


/* maximum application description */
static APP_DESC_T               t_g_app_desc = {
                                    ~((UINT64)0),    /* ui8_flags */
                                    {
                                        128 * 1024,  /* z_stack_size */
                                        APP_THREAD_HIGHEST_PRIORITY, /* ui1_priority */ //modify by msz00292@20080201
                                        1024         /* ui2_num_msgs */
                                    },
                                    0,               /* ui4_app_group_id */
                                    0,               /* ui4_app_id */
                                    ASSISTANT_MAX_MSG_NUM,            /* ui2_msg_count */
                                    ASSISTANT_MAX_MSG_SIZE          /* ui2_max_msg_size */
                                };

/*-----------------------------------------------------------------------------
                    function declarations
 ----------------------------------------------------------------------------*/
static CHAR* _amb_authorized_app (const CHAR* ps_name);
static BOOL  _amb_aee_nfy_fct (HANDLE_T h_amb, VOID* pv_tag, APP_NFY_REASON_T e_nfy_reason);
static INT32 _amb_lifecycle_req (const CHAR* ps_name, AMB_REQUEST_TYPE_T e_req_type, BOOL b_sync);
static INT32 _amb_send_request_to_am (AMB_REQUEST_TYPE_T e_req_type, const VOID* pv_msg, SIZE_T z_msg_len, BOOL b_sync);

VOID   amb_set_dbg_level (UINT16 ui2_dbg_level);
UINT16 amb_get_dbg_level (VOID);

/*-----------------------------------------------------------------------------
 * Name: _amb_authorized_app
 *
 * Description: This API checks if the requested application name was
 *              registered in the system.
 *
 * Inputs:  ps_name   References the application name.
 *
 * Outputs: -
 *
 * Returns: A pointer to the s_name field of REGISTER_INFO structure if it
 *          exists, or NULL.
 ----------------------------------------------------------------------------*/
static CHAR* _amb_authorized_app (const CHAR* ps_name)
{
    CHAR*                 ps_app;
    AMB_REGISTER_INFO_T*  pt_reg;

    ps_app = NULL;
    pt_reg = pt_g_regs;

    while (pt_reg != NULL)
    {
        if (strcmp (pt_reg->s_name, ps_name) == 0)
        {
            ps_app = pt_reg->s_name;

            break;
        }

        pt_reg = pt_reg->pt_next;
    }

    return ps_app;
}

/*----------------------------------------------------------------------------
 * Name: _amb_aee_nfy_fct
 *
 * Description: This function is called by the AEE when a message is received,
 *              and when a message is processed.
 *
 * Inputs:  h_amb           Contains the Application Manager broker handle.
 *          pv_tag          References a tag value.An asynchronous call is
 *                          made if pv_tag is NULL, or a synchronous call is
 *                          made if pv_tag is not a NULL pointer.
 *          e_nfy_reason    Contains the notification reason.
 *
 * Outputs: -
 *
 * Returns: TRUE            The message is to be ignored.
 *          FALSE           The message is to be processed.
 ----------------------------------------------------------------------------*/
static BOOL _amb_aee_nfy_fct (HANDLE_T          h_amb,
                              VOID*             pv_tag,
                              APP_NFY_REASON_T  e_nfy_reason)
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
            if (pv_tag != NULL) /* synchronous call */
            {
                *((BOOL*) pv_tag) = TRUE; /* successful */

                _CONTINUE (h_g_bin_sema); /* unlock semaphore */
            }
            break;

        case APP_NFY_INIT_FAILED:
        case APP_NFY_EXIT_FAILED:
        case APP_NFY_PROCESS_MSG_FAILED:
            if (pv_tag != NULL) /* synchronous call */
            {
                *((BOOL*) pv_tag) = FALSE; /* failed */

                _CONTINUE (h_g_bin_sema); /* unlock semaphore */
            }

            break;

        default:
            break;
    }

    return FALSE;
}

/*-----------------------------------------------------------------------------
 * Name: _amb_send_request_to_am
 *
 * Description: This API sends a request to Application Manager.
 *
 * Inputs:  e_req_type           References to request type.
 *          pv_msg               Message data.
 *          z_msg_len            Message lenght.
 *          b_sync               Synchronous or asynchronous request.
 *
 * Outputs: -
 *
 * Returns: AMBR_OK              Routine successful.
 *          AMBR_FAIL            Routine failed.
 *          AMBR_INV_ARG         Invalid argument.
 *          AMBR_NOT_AUTHORIZED  Not authorized.
 ----------------------------------------------------------------------------*/
static INT32 _amb_send_request_to_am (AMB_REQUEST_TYPE_T  e_req_type,
                                      const VOID*         pv_msg,
                                      SIZE_T              z_msg_len,
                                      BOOL                b_sync)
{
    INT32     i4_res;
    UINT32    ui4_retry;
    BOOL      b_ok;

    i4_res    = AMBR_OK;
    b_ok      = TRUE;
    ui4_retry = 0;

    /* send request to Application Manager */
    while (ui4_retry <= MSGQ_RETRY_TIMES)
    {
        if (b_sync) /* synchronous request */
        {
            i4_res = u_app_send_msg (h_g_am,
                                     e_req_type,
                                     pv_msg,
                                     z_msg_len,
                                     _amb_aee_nfy_fct,
                                     & b_ok /* synchronous call */);

            if (i4_res == AEER_OK)
            {
                /* wait until Application Manager processes this message */
                _WAIT (h_g_bin_sema);

                break;
            }
        }
        else /* asynchronous request */
        {
          //  dbg_print("<DEBUG>send msg to am\n");
            i4_res = u_app_send_msg (h_g_am,
                                     e_req_type,
                                     pv_msg,
                                     z_msg_len,
                                     _amb_aee_nfy_fct,
                                     NULL /* asynchronous call */);

            if (i4_res == AEER_OK)
            {
                break;
            }
        }

        u_thread_delay (MSGQ_RETRY_DELAY);
        ui4_retry ++;
    }

    return (i4_res == AEER_OK && b_ok) ? AMBR_OK : AMBR_FAIL;
}

/*-----------------------------------------------------------------------------
 * Name: _amb_lifecycle_req
 *
 * Description: This API sends a lifecycle request to Application Manager. Note
 *              that this API is an asynchronous call by using caller's thread.
 *              It is important to understand that this API just sends a request
 *              to Application Manager. When will this request be executed
 *              depends on the schedule of the Application Manager.
 *
 * Inputs:  ps_name              References the application name.
 *          e_req_mode           References the request mode.
 *          b_sync               Synchronous or asynchronous request.
 *
 * Outputs: -
 *
 * Returns: AMBR_OK              Routine successful.
 *          AMBR_FAIL            Routine failed.
 *          AMBR_INV_ARG         Invalid argument.
 *          AMBR_NOT_AUTHORIZED  Not authorized.
 ----------------------------------------------------------------------------*/
static INT32 _amb_lifecycle_req (const CHAR*         ps_name,
                                 AMB_REQUEST_TYPE_T  e_req_type,
                                 BOOL                b_sync)
{
    INT32   i4_res;
    CHAR*   ps_app;

    i4_res = AMBR_OK;

    if (ps_name == NULL)
    {
        i4_res = AMBR_INV_ARG;
    }
    else
    {
        ps_app = _amb_authorized_app(ps_name);

        if (ps_app != NULL)
        {
            /* send a lifecycle request to Application Manager */
            i4_res = _amb_send_request_to_am (e_req_type,
                                              ps_app,
                                              APP_NAME_MAX_LEN+1,
                                              b_sync);
        }
        else /* not authorized */
        {
            i4_res = AMBR_NOT_AUTHORIZED;
        }
    }

    return i4_res;
}

/*----------------------------------------------------------------------------
 * Name: u_amb_init
 *
 * Description: This API performs the initialization for Application Manager
 *              Broker. This function should be called by Application Manager
 *              in its initialization function.
 *
 * Inputs:  -
 *
 * Outputs: -
 *
 * Returns: AMBR_OK          Routine successful.
 *          AMBR_FAIL        Routine failed.
 ----------------------------------------------------------------------------*/
INT32 u_amb_init (VOID)
{
    DBG_API (("<AMB> u_amb_init\n\r"));

    /* get Application Manager handle */
    RET_ON_ERR (u_aee_get_handle (APP_MNGR_NAME, & h_g_am));

    /* create a mutex semaphore */
    RET_ON_ERR (u_sema_create (& h_g_mtx_sema,
                               X_SEMA_TYPE_MUTEX,
                               X_SEMA_STATE_UNLOCK /* unlock */));

    /* create a binary semaphore */
    RET_ON_ERR (u_sema_create (& h_g_bin_sema,
                               X_SEMA_TYPE_BINARY,
                               X_SEMA_STATE_LOCK /* lock */));
#if CLI_SUPPORT
    /* initialize cli */
    RET_ON_ERR (amb_cli_init ());
#endif
    return  AMBR_OK;
}

/*----------------------------------------------------------------------------
 * Name: u_amb_destroy
 *
 * Description: This API cleans up the resources allocated by the system. Note
 *              that only Application Manager should call this API. It is
 *              because these resources are particularly allocated for Application
 *              Manager by using system thread in x_appl_init().
 *
 * Inputs:  -
 *
 * Outputs: -
 *
 * Returns: -
 ----------------------------------------------------------------------------*/
VOID  u_amb_destroy (VOID)
{
    INT32                 i4_res;

    DBG_API (("<AMB> u_amb_destroy\n\r"));

    /* free Application Manager handle */
    if (h_g_am != NULL_HANDLE)
    {
        i4_res = u_aee_free_handle (h_g_am);

        if (i4_res != HR_OK)
        {
            DBG_ERROR (("<AMB> ERR: free handle failed = %d\n\r", i4_res));
        }
    }

    /* delete semaphores */
    i4_res = u_sema_delete (h_g_mtx_sema);

    if (i4_res != OSR_OK)
    {
        DBG_ERROR (("<AMB> ERR: free mutex semaphore failed = %d\n\r", i4_res));
    }

    i4_res = u_sema_delete (h_g_bin_sema);

    if (i4_res != OSR_OK)
    {
        DBG_ERROR (("<AMB> ERR: free binary semaphore failed = %d\n\r", i4_res));
    }
}

/*-----------------------------------------------------------------------------
 * Name: u_amb_get_register_into
 *
 * Description: This API finds REGISTER_INFO given a name.
 *
 * Inputs:  ps_name    References the name of the application.
 *
 * Outputs: -
 *
 * Returns: A pointer to the REGISTER_INFO structure if it exists, or NULL.
 ----------------------------------------------------------------------------*/
AMB_REGISTER_INFO_T*  u_amb_get_register_info (const CHAR*  ps_name)
{
    AMB_REGISTER_INFO_T*  pt_reg;

    pt_reg = pt_g_regs;

    while (pt_reg != NULL)
    {
        if (strcmp (pt_reg->s_name, ps_name) == 0)
        {
            break;
        }

        pt_reg = pt_reg->pt_next;
    }

    return pt_reg;
}

/*----------------------------------------------------------------------------
 * Name: u_amb_register_app
 *
 * Description: This API performs the registration of an application.
 *
 * Inputs:  pt_reg           Registration information of the application.
 *
 * Outputs: -
 *
 * Returns: AMBR_OK          Routine successful.
 *          AMBR_INV_ARG     Invalid arguments.
 *          AMBR_FAIL        Routine failed.
 ----------------------------------------------------------------------------*/
INT32 u_amb_register_app (AMB_REGISTER_INFO_T*  pt_reg)
{
    INT32                 i4_res;

    DBG_API_IN;

    i4_res = AMBR_OK;

    if ((pt_reg->s_name == NULL) /* application name */
        /* lifecycle callback functions */
        || (pt_reg->t_fct_tbl.pf_init == NULL)
        || (pt_reg->t_fct_tbl.pf_exit == NULL)
        || (pt_reg->t_fct_tbl.pf_process_msg == NULL)
        /* thread descriptor */
        || (pt_reg->t_desc.t_thread_desc.z_stack_size > t_g_app_desc.t_thread_desc.z_stack_size)
        || (pt_reg->t_desc.t_thread_desc.ui1_priority < t_g_app_desc.t_thread_desc.ui1_priority)
        || (pt_reg->t_desc.t_thread_desc.ui2_num_msgs > t_g_app_desc.t_thread_desc.ui2_num_msgs)
        || (pt_reg->t_desc.ui2_msg_count > t_g_app_desc.ui2_msg_count)
        || (pt_reg->t_desc.ui2_max_msg_size > t_g_app_desc.ui2_max_msg_size))

    {
        i4_res = AMBR_INV_ARG;
    }
    else if (_amb_authorized_app (pt_reg->s_name) != NULL) /* has been registered */
    {
        i4_res = AMBR_BE_REGISTERED;
    }
    else
    {
        _SYNC_BEGIN (h_g_mtx_sema);
        /* add to the list */
        pt_reg->pt_next = pt_g_regs;
        pt_reg->pt_prev = NULL;

        if (pt_g_regs != NULL)
        {
            pt_g_regs->pt_prev = pt_reg;
        }

        pt_g_regs = pt_reg;

		i4_res = _amb_send_request_to_am (AMB_REQUEST_REG, pt_reg->s_name,strlen(pt_reg->s_name)+1 , TRUE);/* synchronous call */
		if(i4_res !=0 )
			DBG_ERROR(("_amb_send_request_to_am fail in %s\n",__FUNCTION__));


        _SYNC_END (h_g_mtx_sema);
    }

    return i4_res;
}

/*-----------------------------------------------------------------------------
 * Name: u_amb_start_app
 *
 * Description: This API sends a start request to Application Manager.
 *
 * Inputs:  ps_name              References the application name.
 *
 * Outputs: -
 *
 * Returns: AMBR_OK              Routine successful.
 *          AMBR_FAIL            Routine failed.
 *          AMBR_INV_ARG         Invalid argument.
 *          AMBR_NOT_AUTHORIZED  Not authorized.
 ----------------------------------------------------------------------------*/
INT32 u_amb_start_app (const CHAR*  ps_name)
{
    DBG_API (("<AMB> start %s\n\r", ps_name));

    return _amb_lifecycle_req (ps_name, AMB_REQUEST_START, FALSE);
}

/*-----------------------------------------------------------------------------
 * Name: u_amb_resume_app
 *
 * Description: This API sends a reume request to Application Manager.
 *
 * Inputs:  ps_name              References the application name.
 *
 * Outputs: -
 *
 * Returns: AMBR_OK              Routine successful.
 *          AMBR_FAIL            Routine failed.
 *          AMBR_INV_ARG         Invalid argument.
 *          AMBR_NOT_AUTHORIZED  Not authorized.
 ----------------------------------------------------------------------------*/
INT32 u_amb_resume_app (const CHAR*  ps_name)
{
    DBG_API (("<AMB> resume %s\n\r", ps_name));

    return _amb_lifecycle_req (ps_name, AMB_REQUEST_RESUME, FALSE);
}

/*-----------------------------------------------------------------------------
 * Name: u_amb_pause_app
 *
 * Description: This API sends a pause request to Application Manager.
 *
 * Inputs:  ps_name              References the application name.
 *
 * Outputs: -
 *
 * Returns: AMBR_OK              Routine successful.
 *          AMBR_FAIL            Routine failed.
 *          AMBR_INV_ARG         Invalid argument.
 *          AMBR_NOT_AUTHORIZED  Not authorized.
 ----------------------------------------------------------------------------*/
INT32 u_amb_pause_app (const CHAR*  ps_name)
{
    DBG_API (("<AMB> pause %s\n\r", ps_name));

    return _amb_lifecycle_req (ps_name, AMB_REQUEST_PAUSE, FALSE);
}

/*-----------------------------------------------------------------------------
 * Name: u_amb_exit_app
 *
 * Description: This API sends an exit request to Application Manager.
 *
 * Inputs:  ps_name              References the application name.
 *
 * Outputs: -
 *
 * Returns: AMBR_OK              Routine successful.
 *          AMBR_FAIL            Routine failed.
 *          AMBR_INV_ARG         Invalid argument.
 *          AMBR_NOT_AUTHORIZED  Not authorized.
 ----------------------------------------------------------------------------*/
INT32 u_amb_exit_app (const CHAR*  ps_name)
{
    DBG_API (("<AMB> exit %s\n\r", ps_name));

    return _amb_lifecycle_req (ps_name, AMB_REQUEST_EXIT, FALSE);
}


/*-----------------------------------------------------------------------------
 * Name: u_amb_sync_start_app
 *
 * Description: This API sends a synchronous start request to Application Manager.
 *
 * Inputs:  ps_name              References the application name.
 *
 * Outputs: -
 *
 * Returns: AMBR_OK              Routine successful.
 *          AMBR_FAIL            Routine failed.
 *          AMBR_INV_ARG         Invalid argument.
 *          AMBR_NOT_AUTHORIZED  Not authorized.
 ----------------------------------------------------------------------------*/
INT32 u_amb_sync_start_app (const CHAR*  ps_name)
{
    DBG_API (("<AMB> sync. start %s\n\r", ps_name));

    return _amb_lifecycle_req (ps_name, AMB_REQUEST_START, TRUE);
}

/*-----------------------------------------------------------------------------
 * Name: u_amb_sync_exit_app
 *
 * Description: This API sends a synchronous exit request to Application Manager.
 *
 * Inputs:  ps_name              References the application name.
 *
 * Outputs: -
 *
 * Returns: AMBR_OK              Routine successful.
 *          AMBR_FAIL            Routine failed.
 *          AMBR_INV_ARG         Invalid argument.
 *          AMBR_NOT_AUTHORIZED  Not authorized.
 ----------------------------------------------------------------------------*/
INT32 u_amb_sync_exit_app (const CHAR*  ps_name)
{
    DBG_API (("<AMB> sync. exit %s\n\r", ps_name));

    return _amb_lifecycle_req (ps_name, AMB_REQUEST_EXIT, TRUE);
}
/*-----------------------------------------------------------------------------
 * Name: u_amb_broadcast_msg
 *
 * Description: This API broadcasts a message to all applications.
 *              This API will will eventually call the u_app_send_msg API of the
 *              AEE to send the broadcast message to the message queue of every
 *              application.
 *
 * Inputs:  ui4_type         References the message type.
 *          pv_msg           References the message.
 *          z_msg_len        References the message length. Must be less than or
 *                           equal to 16 bytes.
            b_sync           synchronously/asynchronously call
 *
 * Outputs: -
 *
 * Returns: AMBR_OK          Routine successful.
 *          AMBR_FAIL        Routine failed.
 *          AMBR_INV_ARG     Invalid argument.
 ----------------------------------------------------------------------------*/
INT32 u_amb_broadcast_msg (AMB_BROADCAST_MSG_T *pmsg, BOOL    b_sync)
{
    INT32                i4_res;

    DBG_API_IN;

    i4_res = AMBR_OK;

    if ((pmsg == NULL) || (pmsg->z_msg_len > BROADCAST_MSG_MAX_LEN) || (AMB_BROADCAST_OFFSET > pmsg->ui4_type))
    {
        i4_res = AMBR_INV_ARG;
    }
    else
    {
        i4_res = _amb_send_request_to_am (AMB_REQUEST_BROADCAST_MSG,
                                          pmsg,
                                          sizeof (AMB_BROADCAST_MSG_T),
                                          b_sync);  /* sync/async call */
    }
    return i4_res;
}

/*-----------------------------------------------------------------------------
 * Name: u_amb_dispatch_msg
 *
 * Description: This API dispatches a message a specific application. Note that
 *              the maximum length of a message should not exceed 32 bytes.
 *
 * Inputs:  ps_name          Application name.
 *          ui4_type         References the message type.
 *          pv_msg           References the message.
 *          z_msg_len        References the message length. Must be less than or
 *                           equal to 32 bytes.
 *
 * Outputs: -
 *
 * Returns: AMBR_OK          Routine successful.
 *          AMBR_FAIL        Routine failed.
 *          AMBR_INV_ARG     Invalid argument.
 ----------------------------------------------------------------------------*/
INT32 u_amb_dispatch_msg (AMB_DISPATCH_MSG_T *pmsg)
{
    INT32                i4_res;
    CHAR*                ps_app;

    if ((pmsg == NULL) ||  (pmsg->z_msg_len > DISPATCH_MSG_MAX_LEN))
    {
        i4_res = AMBR_INV_ARG;
    }

    else if (((ps_app = _amb_authorized_app(pmsg->ps_name)) == NULL) &&
             (strcmp(pmsg->ps_name, APP_MNGR_NAME) != 0))
    {
        i4_res = AMBR_NOT_AUTHORIZED;
    }
    else
    {
        /* send a dispatch message request to Application Manager */
        i4_res = _amb_send_request_to_am (AMB_REQUEST_DISPATCH_MSG,
                                          pmsg,
                                          sizeof (AMB_DISPATCH_MSG_T),
                                          FALSE); /* asynchronous call */
    }

    return i4_res;
}

/*-----------------------------------------------------------------------------
 * Name: amb_set_dbg_level
 *
 * Description: This API sets the debug level of Application Manager Broker.
 *
 * Inputs: ui2_dbg_level     Debug level to be set.
 *
 * Outputs: -
 *
 * Returns: -
 ----------------------------------------------------------------------------*/
VOID amb_set_dbg_level (UINT16 ui2_dbg_level)
{
    ui2_g_dbg_level = ui2_dbg_level;
}

/*-----------------------------------------------------------------------------
 * Name: amb_get_dbg_level
 *
 * Description: This API gets the current debug level of Application Manager
 *              Broker.
 *
 * Inputs: -
 *
 * Outputs: -
 *
 * Returns: Current debug level
 ----------------------------------------------------------------------------*/
UINT16 amb_get_dbg_level (VOID)
{
    return ui2_g_dbg_level | DBG_LAYER_APP;
}

