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
 * $RCSfile: appman.c,v $
 * $Revision: #1 $
 * $Date: 2016/03/07 $
 * $Author: bdbm01 $
 * $CCRevision: /main/DTV_X_HQ_int/1 $
 * $SWAuthor: Iolo Tsai $
 * $MD5HEX: 0772fbcbfc330a3368874e31e29a6d08 $
 *
 * Description:
 *         This file contains the implementation of the application manager
 *         APIs.
 *---------------------------------------------------------------------------*/


/*-----------------------------------------------------------------------------
                    include files
-----------------------------------------------------------------------------*/

#include "aee.h"
#include "u_os.h"
#include "u_dbg.h"
#include "u_appman.h"
#include "u_common.h"
#include <alloca.h>


/*-----------------------------------------------------------------------------
                    macros, defines, typedefs, enums
 ----------------------------------------------------------------------------*/

#define APPMAN_MSG_INIT         0
#define APPMAN_MSG_EXIT         1
#define APPMAN_MSG_PROCESS_MSG  4

#define APPMAN_MSG_PRIORITY 1


typedef struct
{
    UINT32         ui4_command;
    u_app_nfy_fct  pf_nfy;
    VOID*          pv_tag;
    union
    {
        APP_EXIT_MODE_T  e_exit_mode;
        struct
        {
            UINT32  ui4_type;
            SIZE_T  z_msg_len;
        } prc_msg_t;
    } u;
}   APP_MSG_HEADER_T;

typedef struct
{
    APP_FCT_TBL_T  t_fct_tbl;
    HANDLE_T       h_msg_queue;
    VOID*          pv_msg_buffer;
    SIZE_T         z_msg_size;
    CHAR           s_name[AEE_NAME_MAX_LENGTH + 1];
}   APPMAN_MAIN_ARG_T;



/*-----------------------------------------------------------------------------
 * Name: appman_main
 *
 * Description: This API is the application's main function, implemented as a
 *              message loop.
 *
 * Inputs:  h_app   Contains the application handle.
 *          pv_arg  References the function's argument (see APPMAN_MAIN_ARG_T).
 *
 * Outputs: -
 *
 * Returns: -
 ----------------------------------------------------------------------------*/
static VOID appman_main (HANDLE_T  h_app,
                         VOID*     pv_arg)
{
    APPMAN_MAIN_ARG_T*  pt_arg;
    APP_FCT_TBL_T*      pt_fct_tbl;
    APP_MSG_HEADER_T*   pt_msg_header;
    HANDLE_T            h_msg_queue;
    BOOL                b_exit;
    BOOL                b_paused;
    BOOL                b_resumed;
    pt_arg = (APPMAN_MAIN_ARG_T *) pv_arg;

    pt_fct_tbl = & pt_arg->t_fct_tbl;

    pt_msg_header = (APP_MSG_HEADER_T *) pt_arg->pv_msg_buffer;

    h_msg_queue = pt_arg->h_msg_queue;

    b_exit    = FALSE;
    b_paused  = FALSE;
    b_resumed = FALSE;

    while (! b_exit)
    {
        SIZE_T       z_size;
        INT32        i4_res;
        UINT16       ui2_index;

        z_size = pt_arg->z_msg_size;

        i4_res = x_msg_q_receive(& ui2_index,
                                 pt_arg->pv_msg_buffer,
                                 & z_size,
                                 & h_msg_queue,
                                 1,
                                 X_MSGQ_OPTION_WAIT);

        if (i4_res == OSR_OK)
        {
            u_app_nfy_fct  pf_nfy;
            VOID*          pv_tag;

            pf_nfy = pt_msg_header->pf_nfy;
            pv_tag = pt_msg_header->pv_tag;

            switch (pt_msg_header->ui4_command)
            {
                case APPMAN_MSG_PROCESS_MSG:
                {
                    if ((pf_nfy == NULL)
                        ||
                        ! pf_nfy(h_app, pv_tag, APP_NFY_PRE_PROCESS_MSG))
                    {
                        UINT32  ui4_type;
                        VOID*   pv_msg;
                        SIZE_T  z_msg_len;

                        ui4_type  = pt_msg_header->u.prc_msg_t.ui4_type;
                        z_msg_len = pt_msg_header->u.prc_msg_t.z_msg_len;

                        pv_msg = (z_msg_len != 0) ?
                            (VOID *) (pt_msg_header + 1) :
                            NULL;

                        i4_res = pt_fct_tbl->pf_process_msg(h_app,
                                                            ui4_type,
                                                            pv_msg,
                                                            z_msg_len,
                                                            b_paused);

                        if (pf_nfy != NULL)
                        {
                            pf_nfy(h_app, pv_tag,
                                   (i4_res == AEER_OK) ?
                                   APP_NFY_PROCESS_MSG_OK :
                                   APP_NFY_PROCESS_MSG_FAILED);
                        }
                    }
                }
                break;
                case APPMAN_MSG_EXIT:
                {
                    if ((pf_nfy == NULL)
                        ||
                        ! pf_nfy(h_app, pv_tag, APP_NFY_PRE_EXIT))
                    {
                        APP_EXIT_MODE_T  e_exit_mode;

                        e_exit_mode = pt_msg_header->u.e_exit_mode;
                        i4_res = pt_fct_tbl->pf_exit(h_app, e_exit_mode);

                        if (pf_nfy != NULL)
                        {
                            pf_nfy(h_app, pv_tag,
                                   (i4_res == AEER_OK) ?
                                   APP_NFY_EXIT_OK :
                                   APP_NFY_EXIT_FAILED);
                        }

                        b_exit = (i4_res == AEER_OK);
                    }
                }
                break;

                case APPMAN_MSG_INIT:
                {
                    if (pf_nfy != NULL)
                    {
                        b_exit = pf_nfy(h_app, pv_tag, APP_NFY_PRE_INIT);
                    }

                    if (! b_exit)
                    {
                        i4_res = pt_fct_tbl->pf_init(pt_arg->s_name,
                                                     h_app);

                        if (pf_nfy != NULL)
                        {
                            pf_nfy(h_app, pv_tag,
                                   (i4_res == AEER_OK) ?
                                   APP_NFY_INIT_OK :
                                   APP_NFY_INIT_FAILED);
                        }

                        b_exit = (i4_res != AEER_OK);
                    }
                }
                break;

                default:
                    ABORT(DBG_CAT_MESSAGE, DBG_ABRT_INVALID_MESSAGE);
            }
        }
        else
        {
            b_exit = TRUE;
        }
    }

    free(pt_arg->pv_msg_buffer);

    x_msg_q_delete(h_msg_queue);
}

/*-----------------------------------------------------------------------------
 * Name: appman_receive_data
 *
 * Description: This API is the application's receive_data function (see
 *              x_aee_send_data), which maps to u_app_send_msg.
 *
 * Inputs:  h_app       Contains the application handle.
 *          ui4_type    Contains the type of data.
 *          pv_data     References the data buffer.
 *          z_data_len  Contains the length (in bytes) of the data buffer.
 *
 * Outputs: -
 *
 * Returns: AEER_OK             Routine successful.
 *          AEER_OUT_OF_MEMORY  Out of memory.
 *          AEER_FAIL           Routine failed.
 ----------------------------------------------------------------------------*/
static INT32 appman_receive_data (HANDLE_T  h_app,
                                  UINT32    ui4_type,
                                  VOID*     pv_data,
                                  SIZE_T    z_data_len)
{
    return u_app_send_msg (h_app,
                           ui4_type,
                           pv_data,
                           z_data_len,
                           NULL,
                           NULL);
}

/*-----------------------------------------------------------------------------
 * Name: appman_send_msg
 *
 * Description: This API sends a message to the application's message loop. The
 *              message is supposed to be already defined when this API is
 *              called.
 *
 * Inputs:  h_app          Contains the application handle.
 *          pt_msg_header  References the message header.
 *
 * Outputs: -
 *
 * Returns: AEER_OK    Routine successful.
 *          AEER_FAIL  Routine failed.
 ----------------------------------------------------------------------------*/
static INT32 appman_send_msg (HANDLE_T                 h_app,
                              const APP_MSG_HEADER_T*  pt_msg_header)
{
    HANDLE_T  h_msg_queue;
    INT32     i4_res;

    i4_res = aee_get_msg_queue(h_app, & h_msg_queue);

    if (i4_res == AEER_OK)
    {
        i4_res = x_msg_q_send(h_msg_queue,
                              (VOID *) pt_msg_header,
                              sizeof(*pt_msg_header),
                              APPMAN_MSG_PRIORITY);

        i4_res = (i4_res == OSR_OK) ? AEER_OK : AEER_FAIL;
    }

    return i4_res;
}

/*-----------------------------------------------------------------------------
 * Name: u_app_start
 *
 * Description: This API starts an application. The application's init function
 *              will be called first since this API automatically sends a
 *              INIT message to the application's message loop.
 *
 * Inputs:  pt_fct_tbl  References the application's function table.
 *          pt_desc     References the application's description (resources,
 *                      authorization flags, etc).
 *          ps_name     References the application's name.
 *          pf_nfy      References the notify function.
 *          pv_tag      References the tag passed to the notify function.
 *
 * Outputs: ph_app  Contains the application handle.
 *
 * Returns: AEER_OK       Routine successful.
 *          AEER_INV_ARG  Invalid argument.
 *          AEER_FAIL     Routine failed.
 ----------------------------------------------------------------------------*/
static INT32 _app_start (APP_FCT_TBL_T*     pt_fct_tbl,
                   const APP_DESC_T*  pt_desc,
                   const CHAR*        ps_name,
                   u_app_nfy_fct      pf_nfy,
                   VOID*              pv_tag,
                   HANDLE_T*          ph_app)
{
    INT32  i4_res;

    if ((pt_fct_tbl == NULL)
        || (pt_desc == NULL)
        || (ps_name == NULL)
        || (ph_app == NULL)
        || (pt_fct_tbl->pf_init == NULL)
        || (pt_fct_tbl->pf_exit == NULL)
        || (pt_fct_tbl->pf_process_msg == NULL)
        || (pt_desc->ui2_msg_count == 0))
    {
        i4_res = AEER_INV_ARG;
    }
    else
    {
        VOID*   pv_msg_buffer;
        SIZE_T  z_msg_size;

        z_msg_size = sizeof(APP_MSG_HEADER_T) + pt_desc->ui2_max_msg_size;

        pv_msg_buffer = malloc(z_msg_size);

        if (pv_msg_buffer != NULL)
        {
            HANDLE_T  h_msg_queue;

            h_msg_queue = NULL_HANDLE;

            i4_res = x_msg_q_create(& h_msg_queue,
                                    ps_name,
                                    z_msg_size,
                                    pt_desc->ui2_msg_count);

            if (i4_res == OSR_OK)
            {
                AEE_APP_DESC_T     t_aee_app_desc;
                APPMAN_MAIN_ARG_T  t_arg;


                t_arg.t_fct_tbl     = *pt_fct_tbl;
                t_arg.h_msg_queue   = h_msg_queue;
                t_arg.pv_msg_buffer = pv_msg_buffer;
                t_arg.z_msg_size    = z_msg_size;

                strncpy(t_arg.s_name, ps_name, AEE_NAME_MAX_LENGTH);
                t_arg.s_name[AEE_NAME_MAX_LENGTH] = '\0';

                t_aee_app_desc.ui8_flags        = pt_desc->ui8_flags;
                t_aee_app_desc.t_thread_desc    = pt_desc->t_thread_desc;
                t_aee_app_desc.ui4_app_group_id = pt_desc->ui4_app_group_id;
                t_aee_app_desc.ui4_app_id       = pt_desc->ui4_app_id;
                t_aee_app_desc.pf_main         = appman_main;
                t_aee_app_desc.pv_arg          = & t_arg;
                t_aee_app_desc.z_arg_size      = sizeof(t_arg);
                t_aee_app_desc.pf_receive_data = appman_receive_data;

                i4_res = u_aee_create(& t_aee_app_desc,
                                      ps_name,
                                      ph_app);
                if (i4_res == AEER_OK)
                {
                    APP_MSG_HEADER_T  t_msg;
                    aee_set_msg_queue(*ph_app, h_msg_queue);

                    t_msg.ui4_command = APPMAN_MSG_INIT;
                    t_msg.pf_nfy      = pf_nfy;
                    t_msg.pv_tag      = pv_tag;

                    i4_res = appman_send_msg(*ph_app, & t_msg);
                }
                else
                {
                    free(pv_msg_buffer);

                    x_msg_q_delete(h_msg_queue);

                    i4_res = AEER_FAIL;
                }
            }
            else
            {
                free(pv_msg_buffer);

                i4_res = AEER_FAIL;
            }
        }
        else
        {
            i4_res = AEER_OUT_OF_MEMORY;
        }
    }

    return i4_res;
}

/*-----------------------------------------------------------------------------
 * Name: u_app_start
 *
 * Description: This API starts an application. The application's init function
 *              will be called first since this API automatically sends a
 *              INIT message to the application's message loop.
 *
 * Inputs:  pt_fct_tbl  References the application's function table.
 *          pt_desc     References the application's description (resources,
 *                      authorization flags, etc).
 *          ps_name     References the application's name.
 *          pf_nfy      References the notify function.
 *          pv_tag      References the tag passed to the notify function.
 *
 * Outputs: ph_app  Contains the application handle.
 *
 * Returns: AEER_OK                    Routine successful.
 *          AEER_INV_ARG               Invalid argument.
 *          AEER_FAIL                  Routine failed.
 *          AEER_AEE_NO_RIGHTS         Permission denied.
 *          AEER_AEE_OUT_OF_RESOURCES  Not enough resources available.
 ----------------------------------------------------------------------------*/
INT32 u_app_start (APP_FCT_TBL_T*     pt_fct_tbl,
                   const APP_DESC_T*  pt_desc,
                   const CHAR*        ps_name,
                   u_app_nfy_fct      pf_nfy,
                   VOID*              pv_tag,
                   HANDLE_T*          ph_app)
{
    HANDLE_T  h_aux;
    INT32     i4_res = AEER_OK;

    switch (i4_res)
    {
        case AEER_OK:
        {
            i4_res = _app_start(pt_fct_tbl, pt_desc,
                                 ps_name,
                                 pf_nfy, pv_tag,
                                 ph_app);

            if (i4_res == AEER_OK)
            {
                u_handle_link_to_aux(h_aux, *ph_app);
            }

        }
        break;

        case AEER_OUT_OF_RESOURCES:
        {
            i4_res = AEER_AEE_OUT_OF_RESOURCES;
        }
        break;

        default:
        {
            i4_res = AEER_AEE_NO_RIGHTS;
        }
        break;
    }

    return i4_res;
}

/*-----------------------------------------------------------------------------
 * Name: u_app_exit
 *
 * Description: This API sends a message to the application for it to exit.
 *
 * Inputs:  h_app        Contains the handle to the application.
 *          e_exit_mode  Contains the exit mode.
 *          pf_nfy       References the notify function.
 *          pv_tag       References the tag passed to the notify function.
 *
 * Outputs: -
 *
 * Returns: AEER_OK    Routine successful.
 *          AEER_FAIL  Routine failed.
 ----------------------------------------------------------------------------*/
INT32 u_app_exit (HANDLE_T         h_app,
                  APP_EXIT_MODE_T  e_exit_mode,
                  u_app_nfy_fct    pf_nfy,
                  VOID*            pv_tag)
{
    APP_MSG_HEADER_T  t_msg;

    t_msg.ui4_command = APPMAN_MSG_EXIT;
    t_msg.pf_nfy      = pf_nfy;
    t_msg.pv_tag      = pv_tag;

    t_msg.u.e_exit_mode = e_exit_mode;

    return appman_send_msg(h_app, & t_msg);
}

/*----------------------------------------------------------------------------
 * Function: msg_q_send_2_parts()
 *
 * Description:
 *      this API sends a message to a message queue. The message is made of 2
 *      parts, which will be concatenated into one single message.
 *
 * Inputs:
 *      h_msg_hdl: msg Q handle returned via x_msg_q_create() or
 *                 x_msg_q_attach().
 *      pv_msg_1: pointer to the 1st part of the message data.
 *      z_size_1: size in bytes of "pv_msg_1".
 *      pv_msg_2: pointer to the 2nd part of the message data.
 *      z_size_2: size in bytes of "pv_msg_2".
 *      ui1_pri: priority of this sending message, 0 (highest) - 255 (lowest).
 *
 * Outputs: -
 *
 * Returns:
 *      OSR_OK : routine was successful.
 *      OSR_NOT_INIT : message Q module has not been initiated.
 *      OSR_INV_ARG: an error in the arguments of this API.
 *      OSR_TOO_BIG: the sending message is bigger than allowed, specified
 *                   in x_msg_q_create().
 *      OSR_TOO_MANY: too many messages queued in the msg Q, more that allowed,
 *                    specified in x_msg_q_create().
 *      OSR_INV_HANDLE: an error in handle operation.
 *---------------------------------------------------------------------------*/
#define MSG_SIZE(X)             (((X) + 3) / 4)
#define MSGQ_MAX_DATA_SIZE      4095

#include <string.h>

INT32
msg_q_send_2_parts(HANDLE_T     h_msg_hdl,
                   const VOID*  pv_msg_1,
                   SIZE_T       z_size_1,
                   const VOID*  pv_msg_2,
                   SIZE_T       z_size_2,
                   UINT8        ui1_pri)
{
    HANDLE_T h_thread;
    INT32 i4_res;

    if ((pv_msg_2 != NULL) && (z_size_2 != 0))
    {
        if (u_thread_self(&h_thread) == OSR_OK)
        {
            //CHAR *pc_data = (CHAR *) malloc(z_size_1 + z_size_2);
            CHAR *pc_data = (CHAR *) alloca(z_size_1 + z_size_2);
            if (pc_data == NULL)
            {
                return(OSR_NO_RESOURCE);
            }
            memcpy(pc_data, pv_msg_1, z_size_1);
            memcpy(pc_data + z_size_1, pv_msg_2, z_size_2);
            i4_res = x_msg_q_send(h_msg_hdl,
                                  (VOID *) pc_data,
                                  z_size_1 + z_size_2,
                                  ui1_pri);
            //free(pc_data);
        }
        else
        {
            static CHAR pc_data[MSG_SIZE(MSGQ_MAX_DATA_SIZE) * 4];
            memcpy(pc_data, pv_msg_1, z_size_1);
            memcpy(pc_data + z_size_1, pv_msg_2, z_size_2);
            i4_res = x_msg_q_send(h_msg_hdl,
                                  (VOID *) pc_data,
                                  z_size_1 + z_size_2,
                                  ui1_pri);
        }
    }
    else
    {
        i4_res = x_msg_q_send(h_msg_hdl,
                              pv_msg_1,
                              z_size_1,
                              ui1_pri);
    }

    return i4_res;
}


/*-----------------------------------------------------------------------------
 * Name: u_app_send_msg
 *
 * Description: This API sends a message to the application. The format of the
 *              message is not known by the application manager.
 *
 * Inputs:  h_app      Contains the handle to the application.
 *          ui4_type   Contains the type of the message.
 *          pv_msg     References the message data. Can be NULL.
 *          z_msg_len  Contains the length (in bytes) of the message data.
 *          pf_nfy     References the notify function.
 *          pv_tag     References the tag passed to the notify function.
 *
 * Outputs: -
 *
 * Returns: AEER_OK              Routine successful.
 *          AEER_OUT_OF_MEMORY   Out of memory.
 *          AEER_FAIL            Routine failed.
 ----------------------------------------------------------------------------*/
INT32 u_app_send_msg (HANDLE_T       h_app,
                      APP_MSG_TYPE_E      ui4_type,
                      const VOID*    pv_msg,
                      SIZE_T         z_msg_len,
                      u_app_nfy_fct  pf_nfy,
                      VOID*          pv_tag)
{
    HANDLE_T  h_msg_queue;
    INT32     i4_res;

    i4_res = aee_get_msg_queue(h_app, & h_msg_queue);

    if (i4_res == AEER_OK)
    {
        APP_MSG_HEADER_T  t_msg_header;

        t_msg_header.ui4_command = APPMAN_MSG_PROCESS_MSG;
        t_msg_header.pf_nfy      = pf_nfy;
        t_msg_header.pv_tag      = pv_tag;

        t_msg_header.u.prc_msg_t.ui4_type = ui4_type;

        if ((pv_msg != NULL) && (z_msg_len != 0))
        {
            t_msg_header.u.prc_msg_t.z_msg_len = z_msg_len;
        }
        else
        {
            t_msg_header.u.prc_msg_t.z_msg_len = 0;
            pv_msg = NULL;
        }

        i4_res = msg_q_send_2_parts(h_msg_queue,
                                    (VOID *) & t_msg_header,
                                    sizeof(t_msg_header),
                                    pv_msg,
                                    z_msg_len,
                                    APPMAN_MSG_PRIORITY);
        if (OSR_OK != i4_res)
        {
            DBG_ERROR(("FUN: u_app_send_msg return i4_res(%d),msgSize(%d) \n", i4_res, z_msg_len));
        }
        i4_res = (i4_res == OSR_OK) ? AEER_OK : AEER_FAIL;
    }

    return i4_res;
}
INT32 u_app_send_appmsg(HANDLE_T        h_app,
                                APP_MSG_TYPE_E  e_app_type,
                                UINT32          ui4_sender_id,
                                UINT32          ui4_msg_type,
                                void*           p_usr_msg,
                                SIZE_T          t_size)
{
    INT32 i4_ret;
    APPMSG_T *pt_msg = NULL;

    if (t_size > MAX_USR_MSG_LEN)
    {
        DBG_ERROR(("[%s:%d]usr msg[%u] is too long, max size[%u]!\n", __FUNCTION__, __LINE__, t_size, MAX_USR_MSG_LEN));
        return AEER_FAIL;
    }

    pt_msg = (APPMSG_T *)malloc(sizeof(APPMSG_T) + t_size);
    if (NULL == pt_msg)
    {
        DBG_ERROR(("[%s:%d]malloc appmsg_t failed!\n", __FUNCTION__, __LINE__));
        return AEER_FAIL;
    }

    pt_msg->ui4_sender_id = ui4_sender_id;
    pt_msg->ui4_msg_type = ui4_msg_type;
    if (NULL != p_usr_msg)
    {
        memcpy(pt_msg->p_usr_msg, p_usr_msg, t_size);
    }

    i4_ret = u_app_send_msg(h_app,
                              e_app_type,
                              pt_msg,
                              sizeof(APPMSG_T) + t_size,
                              NULL,
                              NULL);

    free(pt_msg);
    return i4_ret;
}
