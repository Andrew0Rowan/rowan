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
 * $RCSfile: aee.c,v $
 * $Revision: #1 $
 * $Date: 2016/03/07 $
 * $Author: bdbm01 $
 * $CCRevision: /main/DTV_X_HQ_int/1 $
 * $SWAuthor: Iolo Tsai $
 * $MD5HEX: 0772fbcbfc330a3368874e31e29a6d08 $
 *
 * Description:
 *         This file contains the implementation of the APIs shared with other
 *         middleware components.
 *---------------------------------------------------------------------------*/


/*-----------------------------------------------------------------------------
                    include files
-----------------------------------------------------------------------------*/

#include "aee.h"
#include "pvt_key.h"
#include "u_dbg.h"

/*-----------------------------------------------------------------------------
                    defines
-----------------------------------------------------------------------------*/

#define AEE_RESOURCE_OK(_res)  \
((_res == 0) || (pt_aee->t_res._res >= _res))


/*-----------------------------------------------------------------------------
                    data declaraions
 ----------------------------------------------------------------------------*/

static AEE_T*    pt_aees    = NULL;
static HANDLE_T  h_aee_sem  = NULL_HANDLE;

static UINT16  ui2_aee_dbg_level = DBG_LEVEL_ERROR;
static BOOL   b_aee_init = FALSE;


/*-----------------------------------------------------------------------------
 * Name: aee_get_dbg_level
 *
 * Description: This API gets the debug level of the AEE component.
 *
 * Inputs:  -
 *
 * Outputs: -
 *
 * Returns: the AEE debug level.
 ----------------------------------------------------------------------------*/
UINT16 aee_get_dbg_level (VOID)
{
    return (ui2_aee_dbg_level | DBG_LAYER_MW);
}

/*-----------------------------------------------------------------------------
 * Name: aee_set_dbg_level
 *
 * Description: This API sets the debug level of the AEE component.
 *
 * Inputs:  ui2_dbg_level  Contains the new debug level.
 *
 * Outputs: -
 *
 * Returns: CMR_OK    The debug level was set successfully.
 *          CMR_FAIL  The debug level could not be set.
 ----------------------------------------------------------------------------*/
INT32 aee_set_dbg_level (UINT16  ui2_dbg_level)
{
    ui2_aee_dbg_level = ui2_dbg_level;

    return AEER_OK;
}

/*-----------------------------------------------------------------------------
 * Name: aee_primary_handle_autofree
 *
 * Description: This API does not do anything (besides returning OK) since
 *              such a handle cannot be deleted (it will be deleted when the
 *              application terminates).
 *
 * Inputs:  h_handle  Contains the handle to delete
 *          e_type    Contains the type of the handle.
 *
 * Outputs: -
 *
 * Returns: HR_OK  Routine successful.
 ----------------------------------------------------------------------------*/
static INT32 aee_primary_handle_autofree (HANDLE_T       h_handle,
                                          HANDLE_TYPE_T  e_type)
{
    /*
      nothing to do, the handle will be deleted automatically when the
      application terminates.
     */

    return HR_OK;
}

/*-----------------------------------------------------------------------------
 * Name: aee_secondary_handle_autofree
 *
 * Description: This API deletes the secondary handle (simply calling
 *              handle_free).
 *
 * Inputs:  h_handle  Contains the handle to delete
 *          e_type    Contains the type of the handle.
 *
 * Outputs: -
 *
 * Returns: HR_OK  Routine successful.
 ----------------------------------------------------------------------------*/
static INT32 aee_secondary_handle_autofree (HANDLE_T       h_handle,
                                            HANDLE_TYPE_T  e_type)
{
    return u_handle_free(h_handle, FALSE);
}

/*-----------------------------------------------------------------------------
 * Name: aee_init
 *
 * Description: This API initializes the AEE middleware component. The system
 *              will abort if the initialization failed.
 *
 * Inputs:  -
 *
 * Outputs: -
 *
 * Returns: AEER_OK  Routine successful.
 ----------------------------------------------------------------------------*/
INT32 aee_init (VOID)
{
    CRIT_STATE_T  t_state;
    INT32         i4_res;
    BOOL          b_aee_already_init;

    t_state = u_crit_start();

    b_aee_already_init = b_aee_init;

    u_crit_end(t_state);

    if (! b_aee_already_init)
    {
        static handle_autofree_fct  apf_autofree_fcts[2] =
            {
                aee_primary_handle_autofree,
                aee_secondary_handle_autofree
            };

        i4_res = u_sema_create(& h_aee_sem,
                               X_SEMA_TYPE_MUTEX, 1);

        if (i4_res != OSR_OK)
        {
            ABORT(DBG_CAT_SEMAPHORE, DBG_ABRT_CANNOT_CREATE_SEMAPHORE);
        }

        i4_res = u_handle_set_autofree_tbl(HT_GROUP_AEE,
                                         apf_autofree_fcts);

        if (i4_res != HR_OK)
        {
            ABORT(DBG_CAT_INV_OP, DBG_ABRT_CANNOT_SET_AUTOFREE);
        }

        aee_cli_init();
    }

    i4_res = AEER_OK;

    return i4_res;
}

/*-----------------------------------------------------------------------------
 * Name: aee_uninit
 *
 * Description: This API un-initializes the AEE middleware component. 
 *
 * Inputs:  -
 *
 * Outputs: -
 *
 * Returns: AEER_OK  Routine successful.
 ----------------------------------------------------------------------------*/
INT32 aee_uninit(VOID)
{
    CRIT_STATE_T  t_state;
    INT32         i4_res;
    BOOL          b_aee_already_init;

    t_state = u_crit_start();

    b_aee_already_init = b_aee_init;

    u_crit_end(t_state);

    if (b_aee_already_init)
    {
        i4_res = u_sema_delete(h_aee_sem);

        if (i4_res != OSR_OK)
        {
            ABORT(DBG_CAT_SEMAPHORE, DBG_ABRT_CANNOT_CREATE_SEMAPHORE);
        }

        t_state = u_crit_start();

        b_aee_init = FALSE;

        u_crit_end(t_state);
        
    }

    i4_res = AEER_OK;

    return i4_res;
}

/*-----------------------------------------------------------------------------
 * Name: aee_add
 *
 * Description: This API adds an AEE structure to the list of AEEs.
 *
 * Inputs:  pt_aee  References the AEE to add to the list.
 *
 * Outputs: -
 *
 * Returns: -
 ----------------------------------------------------------------------------*/
static VOID aee_add (AEE_T*  pt_aee)
{
    u_sema_lock(h_aee_sem, X_SEMA_OPTION_WAIT);

    pt_aee->pt_next = pt_aees;
    pt_aees         = pt_aee;

    u_sema_unlock(h_aee_sem);
}

/*-----------------------------------------------------------------------------
 * Name: aee_remove
 *
 * Description: This API removes an AEE structure from the list of AEEs. It
 *              assumes the structure is in the list.
 *
 * Inputs:  pt_aee  References the AEE to remove from the list.
 *
 * Outputs: -
 *
 * Returns: -
 ----------------------------------------------------------------------------*/
static VOID aee_remove (AEE_T*  pt_aee)
{
    AEE_T**  ppt_aee;

    u_sema_lock(h_aee_sem, X_SEMA_OPTION_WAIT);

    ppt_aee = & pt_aees;

    while (*ppt_aee != NULL)
    {
        if (*ppt_aee == pt_aee)
        {
            *ppt_aee = pt_aee->pt_next;
            break;
        }

        ppt_aee = & ((*ppt_aee)->pt_next);
    }

    u_sema_unlock(h_aee_sem);
}

/*-----------------------------------------------------------------------------
 * Name: aee_find_by_name
 *
 * Description: This API finds an AEE given a name.
 *
 * Inputs:  ps_name  References the name of the application.
 *
 * Outputs: -
 *
 * Returns: A pointer to the AEE structure if it exists, or NULL.
 ----------------------------------------------------------------------------*/
AEE_T* aee_find_by_name (const CHAR*  ps_name)
{
    AEE_T*  pt_aee;

    u_sema_lock(h_aee_sem, X_SEMA_OPTION_WAIT);

    pt_aee = pt_aees;

    while (pt_aee != NULL)
    {
        if (strcmp(pt_aee->s_name, ps_name) == 0)
        {
            break;
        }

        pt_aee = pt_aee->pt_next;
    }

    u_sema_unlock(h_aee_sem);

    return pt_aee;
}

#ifdef CLI_SUPPORT
/*-----------------------------------------------------------------------------
 * Name: aee_list_aee_info
 *
 * Description: This API displays information about a specific AEE.
 *
 * Inputs:  pt_aee  References the AEE structure.
 *
 * Outputs: -
 *
 * Returns: -
 ----------------------------------------------------------------------------*/
VOID aee_list_aee_info (const AEE_T*  pt_aee)
{
    UINT16  ui2_current;
    UINT16  ui2_peak;

	dbg_print(DBG_PREFIX" %s [app:%d][thread:%d]\n", pt_aee->s_name, pt_aee->h_app, pt_aee->h_thread);

}

/*-----------------------------------------------------------------------------
 * Name: aee_list_all_aees_info
 *
 * Description: This API displays information about all the AEEs.
 *
 * Inputs:  -
 *
 * Outputs: -
 *
 * Returns: -
 ----------------------------------------------------------------------------*/
VOID aee_list_all_aees_info (VOID)
{
    AEE_T*  pt_aee;

    dbg_print(DBG_PREFIX"AEEs:\n");

    u_sema_lock(h_aee_sem, X_SEMA_OPTION_WAIT);

    pt_aee = pt_aees;

    while (pt_aee != NULL)
    {
        aee_list_aee_info(pt_aee);

        pt_aee = pt_aee->pt_next;
    }

    u_sema_unlock(h_aee_sem);
}
#endif /* DEBUG || CLI_SUPPORT*/

/*-----------------------------------------------------------------------------
 * Name: aee_clean_up
 *
 * Description: This API checks whether the AEE is terminated and will perform
 *              clean-up (i.e. deletion of all handles allocated by the app) if
 *              required. The system will abort if clean-up could not be
 *              performed completely for whatever reason.
 *
 * Inputs:  h_app   Contains the application handle.
 *          pt_aee  References the AEE structure.
 *
 * Outputs: -
 *
 * Returns: -
 ----------------------------------------------------------------------------*/
static VOID aee_clean_up (HANDLE_T  h_app,
                          AEE_T*    pt_aee)
{
    BOOL          b_free;
    aee_remove(pt_aee);

    DBG_INFO((DBG_PREFIX"\"%s\" exits\n", pt_aee->s_name));

    u_handle_free(h_app, FALSE);
}

/*-----------------------------------------------------------------------------
 * Name: aee_clean_relationships
 *
 * Description: This API cleans up the relationships between AEEs. It also
 *              release the handle back into the parent's resource pool.
 *
 * Inputs:  h_app           Contains the application handle.
 *          pt_exiting_aee  References the AEE structure.
 *
 * Outputs: -
 *
 * Returns: -
 ----------------------------------------------------------------------------*/
static VOID aee_clean_relationships (HANDLE_T      h_app,
                                     const AEE_T*  pt_exiting_aee)
{
    HANDLE_T  h_parent_app;

    h_parent_app = pt_exiting_aee->h_parent_app;

    if (h_parent_app != NULL_HANDLE)
    {
        AEE_T*  pt_aee;

        u_sema_lock(h_aee_sem, X_SEMA_OPTION_WAIT);

        pt_aee = pt_aees;

        while (pt_aee != NULL)
        {
        
            /* children of the exiting AEE reset parent to NULL_HANDLE */
            if (pt_aee->h_parent_app == h_app)
            {
                pt_aee->h_parent_app = NULL_HANDLE;
            }

            pt_aee = pt_aee->pt_next;
        }

        u_sema_unlock(h_aee_sem);
    }
}

/*-----------------------------------------------------------------------------
 * Name: aee_main
 *
 * Description: This API is a wrapper to the application's main function that
 *              is specified when the application is created.
 *
 * Inputs:  pv_arg  References the arguments to the main function.
 *
 * Outputs: -
 *
 * Returns: -
 ----------------------------------------------------------------------------*/
static VOID aee_main (VOID*  pv_arg)
{
    AEE_MAIN_ARG_T*  pt_arg;
    INT32            i4_res;

    pt_arg = (AEE_MAIN_ARG_T*) pv_arg;

    aee_add(pt_arg->pt_aee);

    DBG_INFO((DBG_PREFIX"\"%s\" starts (%p). aee_main()\n",
              pt_arg->pt_aee->s_name, pt_arg->pt_aee));

    i4_res = u_thread_set_pvt(PVT_KEY_AEE, NULL, pt_arg->pt_aee);

    if (i4_res == OSR_OK)
    {
        pt_arg->pf_main(pt_arg->h_app,
                        pt_arg->pv_arg);
    }

    free(pt_arg->pv_arg);

    aee_clean_up(pt_arg->h_app, pt_arg->pt_aee);
}

/*-----------------------------------------------------------------------------
 * Name: aee_thread_create_main
 *
 * Description: This API is a wrapper to the main function specified when an
 *              application spawns a new thread.
 *
 * Inputs:  pv_arg  References the arguments to the main function.
 *
 * Outputs: -
 *
 * Returns: -
 ----------------------------------------------------------------------------*/
static VOID aee_thread_create_main (VOID*  pv_arg)
{
    AEE_THREAD_CREATE_ARG_T*  pt_arg;
    INT32                     i4_res;

    pt_arg = (AEE_THREAD_CREATE_ARG_T*) pv_arg;

    i4_res = u_thread_set_pvt(PVT_KEY_AEE, NULL, pt_arg->pt_aee);

    if (i4_res == OSR_OK)
    {
        pt_arg->pf_main(pt_arg->pv_arg);
    }

    free(pt_arg->pv_arg);

    aee_clean_up(pt_arg->h_app, pt_arg->pt_aee);
}

/*-----------------------------------------------------------------------------
 * Name: aee_get_info
 *
 * Description: This API returns information about the application (group id,
 *              app id and AEE structure). It assumes the arguments are valid.
 *
 * Inputs:  -
 *
 * Outputs: pui4_app_group_id  Contains the application group id.
 *          pui4_app_id        Contains the application id.
 *          ppt_aee            Contains a pointer to the AEE structure.
 *
 * Returns: AEER_OK    Routine successful.
 *          AEER_FAIL  Routine failed.
 ----------------------------------------------------------------------------*/
static INT32 aee_get_info (UINT32*  pui4_app_group_id,
                           UINT32*  pui4_app_id,
                           AEE_T**  ppt_aee)
{
    INT32  i4_res;

    i4_res = u_thread_get_pvt(PVT_KEY_AEE, (VOID**) ppt_aee);

    if (i4_res == OSR_OK)
    {
        *pui4_app_group_id = (*ppt_aee)->ui4_app_group_id;
        *pui4_app_id       = (*ppt_aee)->ui4_app_id;

        return AEER_OK;
    }
    else
    {
        return AEER_FAIL;
    }
}

/*-----------------------------------------------------------------------------
 * Name: aee_thread_create
 *
 * Description: This API spawns a new application thread. It must be called
 *              from an application thread.
 *
 * Inputs:  ps_name      Contains the thread name.
 *          z_stacksize  Contains the stack size.
 *          ui1_pri      Contains the thread priority.
 *          pf_main      References the thread main function.
 *          z_arg_size   Contains the size of the pv_arg data.
 *          pv_arg       References the data passed to the main function.
 *
 * Outputs: ph_th_hdl  Contains the thread handle.
 *
 * Returns: AEER_OK             Routine successful.
 *          AEER_OUT_OF_MEMORY  Out of memory.
 *          AEER_FAIL           Routine failed.
 ----------------------------------------------------------------------------*/
INT32 aee_thread_create (HANDLE_T*             ph_th_hdl,
                         CHAR*                 ps_name,
                         SIZE_T                z_stacksize,
                         UINT8                 ui1_pri,
                         u_os_thread_main_fct  pf_main,
                         SIZE_T                z_arg_size,
                         VOID*                 pv_arg)
{
    UINT32  ui4_app_group_id;
    UINT32  ui4_app_id;
    INT32   i4_res;
    AEE_T*  pt_aee;

    ui4_app_group_id = 0;
    ui4_app_id       = 0;
    pt_aee           = NULL;

    i4_res = aee_get_info (& ui4_app_group_id,
                           & ui4_app_id,
                           & pt_aee);

    if (i4_res == AEER_OK)
    {
        AEE_THREAD_CREATE_ARG_T  t_arg;

        if (pv_arg != NULL)
        {
            t_arg.pv_arg = malloc(z_arg_size);

            if (t_arg.pv_arg != NULL)
            {
                memcpy(t_arg.pv_arg, pv_arg, z_arg_size);
            }
            else
            {
                i4_res = AEER_OUT_OF_MEMORY;
            }
        }
        else
        {
            t_arg.pv_arg = NULL;
        }

        if (i4_res == AEER_OK)
        {
            t_arg.pf_main = pf_main;
            t_arg.pt_aee  = pt_aee;
            t_arg.h_app   = pt_aee->h_app;

            i4_res = u_thread_create(ph_th_hdl,
                                     ps_name,
                                     z_stacksize,
                                     ui1_pri,
                                     aee_thread_create_main,
                                     sizeof(t_arg),
                                     & t_arg);

            if (i4_res != OSR_OK)
            {
                free(t_arg.pv_arg);

                i4_res = AEER_FAIL;
            }
            else
            {
                i4_res = AEER_OK;
            }
        }
    }
    else
    {
        i4_res = AEER_FAIL;
    }

    return i4_res;
}

/*-----------------------------------------------------------------------------
 * Name: aee_free_handle
 *
 * Description: This API is called when an application's primary handle is
 *              deleted.
 *
 * Inputs:  h_app         Contains the handle to delete.
 *          e_type        Contains the handle type (should be AEET_PRIMARY_AEE)
 *          pv_obj        References the object attached to the handle.
 *          pv_tag        Contains the handle tag.
 *          b_req_handle  Is set to TRUE if the handle free was initiated with
 *                        this handle else FALSE.
 *
 * Outputs: -
 *
 * Returns: TRUE if the handle free is allowed, FALSE otherwise.
 ----------------------------------------------------------------------------*/
static BOOL aee_free_handle (HANDLE_T       h_app,
                             HANDLE_TYPE_T  e_type,
                             VOID*          pv_obj,
                             VOID*          pv_tag,
                             BOOL           b_req_handle)
{
    BOOL  b_free_allowed;

    b_free_allowed = ! b_req_handle;

    if (b_free_allowed)
    {
        AEE_T*  pt_aee;

        pt_aee = (AEE_T*) pv_obj;

        /* autofree handles */
        {
            HANDLE_T  h_handle = NULL_HANDLE;

            u_handle_next_aux_linked(pt_aee->h_aux, & h_handle);

            while (h_handle != NULL_HANDLE)
            {
                HANDLE_T  h_next_handle = NULL_HANDLE;

                u_handle_next_aux_linked(h_handle, & h_next_handle);

                u_handle_delink_from_aux(h_handle);

                u_handle_autofree(h_handle);

                h_handle = h_next_handle;
            }
        }

        /* free auxilary list "header" */
        u_handle_free(pt_aee->h_aux, FALSE);

        aee_clean_relationships(h_app, pt_aee);

        free(pv_obj);
    }

    return b_free_allowed;
}

/*-----------------------------------------------------------------------------
 * Name: aee_free_secondary_handle
 *
 * Description: This API is called when an application's secondary handle is
 *              deleted.
 *
 * Inputs:  h_app         Contains the handle to delete.
 *          e_type        Contains the handle type (AEET_SECONDARY_AEE)
 *          pv_obj        References the object attached to the handle.
 *          pv_tag        Contains the handle tag.
 *          b_req_handle  Is set to TRUE if the handle free was initiated with
 *                        this handle else FALSE.
 *
 * Outputs: -
 *
 * Returns: TRUE.
 ----------------------------------------------------------------------------*/
static BOOL aee_free_secondary_handle (HANDLE_T       h_app,
                                       HANDLE_TYPE_T  e_type,
                                       VOID*          pv_obj,
                                       VOID*          pv_tag,
                                       BOOL           b_req_handle)
{
    return TRUE;
}

/*-----------------------------------------------------------------------------
 * Name: aee_aux_free_release_handle
 *
 * Description: This API is called when a handle is removed from the auxilary
 *              list. This will increment the number of available handles by 1.
 *
 * Inputs:  pv_obj    References the AEE structure.
 *
 * Outputs: -
 *
 * Returns: -
 ----------------------------------------------------------------------------*/
static VOID aee_aux_free_release_handle (VOID*  pv_obj)
{
    AEE_T*        pt_aee;
    UINT32        ui4_overflow;
    UINT16        ui2_handles;

    pt_aee       = (AEE_T*) pv_obj;
    ui4_overflow = 0;
    ui2_handles  = 1;

    if (ui4_overflow != 0)
    {
        DBG_ERROR((DBG_PREFIX"Handle resource release overflow\n"));
    }
}

/*-----------------------------------------------------------------------------
 * Name: aee_create_op
 *
 * Description: This API creates a new application execution environment.
 *
 * Inputs:  pt_desc  References the application's description (resources, etc).
 *          ps_name  References the application's name.
 *
 * Outputs: ph_app  Contains the application handle.
 *
 * Returns: AEER_OK              Routine successful.
 *          AEER_OUT_OF_HANDLES  Out of handles.
 *          AEER_OUT_OF_MEMORY   Out of memory.
 *          AEER_FAIL            Routine failed (undefined reason).
 ----------------------------------------------------------------------------*/
INT32 aee_create_op (const AEE_APP_DESC_T*	pt_desc,
					 const CHAR*			ps_name,
					 HANDLE_T*				ph_app)
{
	SIZE_T	z_size;
	AEE_T*	pt_aee;
	INT32	i4_res;

	z_size = sizeof(AEE_T);

	pt_aee = (AEE_T *) malloc(z_size);

	if (pt_aee != NULL)
	{
		memset(pt_aee, 0, sizeof(AEE_T));
		
		pt_aee->ui8_flags		 = pt_desc->ui8_flags;
		pt_aee->pf_receive_data  = pt_desc->pf_receive_data;
		pt_aee->ui4_app_group_id = pt_desc->ui4_app_group_id;
		pt_aee->ui4_app_id		 = pt_desc->ui4_app_id;

		strncpy(pt_aee->s_name, ps_name, AEE_NAME_MAX_LENGTH);
		i4_res = u_handle_alloc(AEET_PRIMARY_AEE,
							  pt_aee,
							  NULL, /* tag */
							  aee_free_handle,
							  ph_app);
		if (i4_res == HR_OK)
		{
			i4_res =
				u_handle_alloc_aux_link_head(pt_aee,
										   aee_aux_free_release_handle,
										   & pt_aee->h_aux);
			
			if (i4_res == HR_OK)
			{
				AEE_MAIN_ARG_T	t_arg;

				pt_aee->h_app = *ph_app;

				t_arg.pt_aee  = pt_aee;
				t_arg.h_app   = *ph_app;
				t_arg.pf_main = pt_desc->pf_main;
				t_arg.pv_arg  = NULL;

				if (pt_desc->pv_arg != NULL)
				{
					t_arg.pv_arg = malloc(pt_desc->z_arg_size);

					if (t_arg.pv_arg != NULL)
					{
						memcpy(t_arg.pv_arg,
								 pt_desc->pv_arg,
								 pt_desc->z_arg_size);
					}
				}
				if ((pt_desc->pv_arg == NULL) || (t_arg.pv_arg != NULL))
				{
					AEE_T*	pt_parent_aee;
					SIZE_T	z_stack_size;
					UINT8	ui1_priority;
					RET_AEE_U  u_aee;

					z_stack_size = pt_desc->t_thread_desc.z_stack_size;
					ui1_priority = pt_desc->t_thread_desc.ui1_priority;

					i4_res =
						u_thread_get_pvt(PVT_KEY_AEE,
										 &u_aee.pv_aee);
					pt_parent_aee = u_aee.pt_aee;

					if ((i4_res == OSR_OK) && (pt_parent_aee != NULL))
					{
						pt_aee->h_parent_app = pt_parent_aee->h_app;
					}
					else
					{
						pt_aee->h_parent_app = NULL_HANDLE;
					}
					/* create thread */
					i4_res = u_thread_create(& pt_aee->h_thread,
											 ps_name,
											 z_stack_size,
											 ui1_priority,
											 aee_main,
											 sizeof(t_arg),
											 & t_arg);

					if (i4_res == OSR_OK)
					{
						i4_res = AEER_OK;
					}
					else
					{
						u_handle_free(*ph_app, FALSE);

						i4_res = AEER_FAIL;
					}
				}
				else
				{
					u_handle_free(*ph_app, FALSE);

					i4_res = AEER_FAIL;
				}
			}
			else
			{
				u_handle_free(*ph_app, FALSE);

				i4_res = AEER_OUT_OF_HANDLES;
			}
		}
		else
		{
			free(pt_aee);

			i4_res = AEER_OUT_OF_HANDLES;
		}
	}
	else
	{
		i4_res = AEER_OUT_OF_MEMORY;
	}

	return i4_res;
}

/*-----------------------------------------------------------------------------
 * Name: aee_get_handle_op
 *
 * Description: This API returns a handle to an application (different from the
 *              handle created when the AEE was created).
 *
 * Inputs:  ps_name  References the name of the AEE.
 *
 * Outputs: ph_app  Contains the application handle.
 *
 * Returns: AEER_OK              Routine successful.
 *          AEER_FAIL            No application with such name exists.
 *          AEER_OUT_OF_HANDLES  Out of handles.
 ----------------------------------------------------------------------------*/
INT32 aee_get_handle_op (const CHAR*  ps_name,
                         HANDLE_T*    ph_app)
{
    INT32  i4_res;

    i4_res = u_handle_alloc(AEET_SECONDARY_AEE,
                          NULL,
                          NULL,
                          aee_free_secondary_handle,
                          ph_app);

    if (i4_res == HR_OK)
    {
        AEE_T*  pt_aee;

        u_sema_lock(h_aee_sem, X_SEMA_OPTION_WAIT);

        pt_aee = aee_find_by_name(ps_name);

        if (pt_aee != NULL)
        {
            u_handle_set_obj(*ph_app, pt_aee);

            i4_res = AEER_OK;
        }
        else
        {
			u_handle_free (*ph_app, TRUE);

            i4_res = AEER_FAIL;
        }

        u_sema_unlock(h_aee_sem);
    }
    else
    {
        i4_res = AEER_OUT_OF_HANDLES;
    }

    return i4_res;
}

/*-----------------------------------------------------------------------------
 * Name: aee_send_data_op
 *
 * Description: This API calls the application's receive_data function.
 *
 * Inputs:  h_app       Contains the application handle.
 *          ui4_type    Contains the type of data.
 *          pv_data     References the data to send.
 *          z_data_len  Contains the size (in bytes) of the pv_data buffer.
 *
 * Outputs: -
 *
 * Returns: AEER_OK          Routine successful.
 *          AEER_INV_HANDLE  Invalid handle.
 ----------------------------------------------------------------------------*/
INT32 aee_send_data_op (HANDLE_T  h_app,
                        UINT32    ui4_data,
                        VOID*     pv_data,
                        SIZE_T    z_data_len)
{
    AEE_T*         pt_aee;
    HANDLE_TYPE_T  e_type;
    INT32          i4_res;
    RET_AEE_U      u_aee;

    i4_res = u_handle_get_type_obj(h_app, &e_type, &u_aee.pv_aee);
    pt_aee = u_aee.pt_aee;

    if ((i4_res == HR_OK) &&
        ((e_type == AEET_PRIMARY_AEE) || (e_type == AEET_SECONDARY_AEE)))
    {
        pt_aee->pf_receive_data(h_app, ui4_data, pv_data, z_data_len);

        i4_res = AEER_OK;
    }
    else
    {
        i4_res = AEER_INV_HANDLE;
    }

    return i4_res;
}
/*-----------------------------------------------------------------------------
 * Name: aee_get_msg_queue
 *
 * Description: This API returns the message queue handle associated with the
 *              application. This is only used by the application manager part.
 *
 * Inputs:  h_app           Contains the application handle.
 *
 * Outputs: ph_msg_queue    Contains the message queue handle.
 *
 * Returns: AEER_OK          Routine successful.
 *          AEER_INV_HANDLE  Invalid handle.
 ----------------------------------------------------------------------------*/
INT32 aee_get_msg_queue (HANDLE_T   h_app,
                         HANDLE_T*  ph_msg_queue)
{
    AEE_T*         pt_aee;
    HANDLE_TYPE_T  e_type;
    INT32          i4_res;
    RET_AEE_U      u_aee;

    i4_res = u_handle_get_type_obj(h_app, &e_type, &u_aee.pv_aee);
    pt_aee = u_aee.pt_aee;

    if ((i4_res == HR_OK) &&
        ((e_type == AEET_PRIMARY_AEE) || (e_type == AEET_SECONDARY_AEE)))
    {
        *ph_msg_queue = pt_aee->h_msg_queue;

        i4_res = AEER_OK;
    }
    else
    {
        i4_res = AEER_INV_HANDLE;
    }

    return i4_res;
}

/*-----------------------------------------------------------------------------
 * Name: aee_set_msg_queue
 *
 * Description: This API sets the message queue handle of the application.
 *              This is only used by the application manager part.
 *
 * Inputs:  h_app           Contains the application handle.
 *          h_msg_queue     Contains the message queue handle.
 *
 * Outputs: -
 *
 * Returns: AEER_OK          Routine successful.
 *          AEER_INV_HANDLE  Invalid handle.
 ----------------------------------------------------------------------------*/
INT32 aee_set_msg_queue (HANDLE_T  h_app,
                         HANDLE_T  h_msg_queue)
{
    AEE_T*         pt_aee;
    HANDLE_TYPE_T  e_type;
    INT32          i4_res;
    RET_AEE_U      u_aee;

    i4_res = u_handle_get_type_obj(h_app, &e_type, &u_aee.pv_aee);
    pt_aee = u_aee.pt_aee;

    if ((i4_res == HR_OK) &&
        ((e_type == AEET_PRIMARY_AEE) || (e_type == AEET_SECONDARY_AEE)))
    {
        pt_aee->h_msg_queue = h_msg_queue;

        i4_res = AEER_OK;
    }
    else
    {
        i4_res = AEER_INV_HANDLE;
    }

    return i4_res;
}

/*-----------------------------------------------------------------------------
 * Name: aee_get_thread
 *
 * Description: This API returns the thread handle associated with the
 *              application. The thread handle is the handle of the thread
 *              created automatically when the AEE was created.
 *
 * Inputs:  h_app           Contains the application handle.
 *
 * Outputs: ph_thread       Contains the thread handle.
 *
 * Returns: AEER_OK          Routine successful.
 *          AEER_INV_HANDLE  Invalid handle.
 ----------------------------------------------------------------------------*/
INT32 aee_get_thread (HANDLE_T   h_app,
                      HANDLE_T*  ph_thread)
{
    AEE_T*         pt_aee;
    HANDLE_TYPE_T  e_type;
    INT32          i4_res;
    RET_AEE_U      u_aee;

    i4_res = u_handle_get_type_obj(h_app, &e_type, &u_aee.pv_aee);
    pt_aee = u_aee.pt_aee;

    if ((i4_res == HR_OK) &&
        ((e_type == AEET_PRIMARY_AEE) || (e_type == AEET_SECONDARY_AEE)))
    {
        *ph_thread = pt_aee->h_thread;

        i4_res = AEER_OK;
    }
    else
    {
        i4_res = AEER_INV_HANDLE;
    }

    return i4_res;
}



/*-----------------------------------------------------------------------------
 * Name: aee_get_app_handle
 *
 * Description: This API returns the appication handle associated with the
 *              thread. The thread handle is the handle of the thread
 *              created automatically when the AEE was created.
 *
 * Inputs:  ph_app           Contains the application handle.
 *
 * Outputs: h_thread         Contains the thread handle.
 *
 * Returns: AEER_OK          Routine successful.
 *          AEER_INV_HANDLE  Invalid handle.
 ----------------------------------------------------------------------------*/
INT32 aee_get_app_handle (HANDLE_T *  ph_app,
                      HANDLE_T h_thread)
{
    AEE_T*         pt_aee = pt_aees;

    if(!pt_aee)
    {
        return AEER_NOT_AUTHORIZED;
    }

    while(pt_aee)
    {
        if(pt_aee->h_thread == h_thread)
        {
            *ph_app = pt_aee->h_app;
            return AEER_OK;
        }
        pt_aee = pt_aee->pt_next;
    }
    
    return AEER_INV_HANDLE;
}



/*-----------------------------------------------------------------------------
 * Name: aee_get_name
 *
 * Description: This API returns the name of the AEE, or NULL. Note that the
 *              name is not copied: the API returns a pointer to the internal
 *              name.
 *
 * Inputs:  -
 *
 * Outputs: -
 *
 * Returns: the name of the AEE, or NULL.
 ----------------------------------------------------------------------------*/
const CHAR* aee_get_name (VOID)
{
    const CHAR*  ps_name;
    AEE_T*       pt_aee;
    INT32        i4_res;
    RET_AEE_U    u_aee;
    
    i4_res = u_thread_get_pvt(PVT_KEY_AEE, &u_aee.pv_aee);
    pt_aee = u_aee.pt_aee;
    ps_name = (i4_res == OSR_OK) ? pt_aee->s_name : NULL; 

    return ps_name;
}
