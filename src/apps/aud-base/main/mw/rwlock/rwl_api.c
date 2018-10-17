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
 * $RCSfile: rwl_api.c,v $
 * $Revision: #1 $
 * $Date: 2016/07/22 $
 * $Author: bdbm01 $
 *
 * Description:
 *         This file contains Multiple-Reader Single-Writer lock related
 *         function implementations.
 *---------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------
                    include files
 ----------------------------------------------------------------------------*/

#include "u_common.h"
#include "u_os.h"
#include "u_dbg.h"
#include "rwl.h"



#ifndef dbg_rwl
#define dbg_rwl 0
#endif

/*-----------------------------------------------------------------------------
                    data declaraions
 ----------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------
 * Name: _free_rwlock_resource
 *
 * Description: Internal API to free rwlock memory and resource.
 *
 * Inputs:  - Handle to a read-write lock.
 *            Pointer to the rwlock structure.
 *
 * Outputs: - None.
 *
 * Returns: - None.
 *
 ----------------------------------------------------------------------------*/
static VOID _free_rwlock_resource(HANDLE_T h_rw_lock,  MRSW_LOCK_T *pt_rw_lock)
{
    /*  Safe to remove the handle from handle control structure
        Anymore READ/WRITE/RELEASE lock would receive INVALID_HANDLE
    */
    u_handle_free(h_rw_lock, TRUE);

    /* Free the Read-Write lock data structure and the semaphores. */
    if ( u_sema_delete(pt_rw_lock->h_sem_wait_wr) != OSR_OK ||
         u_sema_delete(pt_rw_lock->h_sem_wait_rd) != OSR_OK )
    {
        ABORT(DBG_CAT_SEMAPHORE,DBG_ABRT_CANNOT_DELETE_WAIT_SEMAPHORE);
    }
    free(pt_rw_lock);
}


/*-----------------------------------------------------------------------------
 * Name: u_rwl_create_lock
 *
 * Description: This API creates a Mutiple-Reader Single-Writer lock.
 *
 * Inputs:  - Pointer to a read-write lock handle.
 *
 * Outputs: - None.
 *
 * Returns: - RWLR_OK     A new lock is initialized successfully.
 *            RWLR_FAIL   Cannot create a lock.
 ----------------------------------------------------------------------------*/
INT32 u_rwl_create_lock  (HANDLE_T* ph_rw_lock)
{
    MRSW_LOCK_T* pt_rw_lock ;
    /*
       Create the rw lock object.
    */

    pt_rw_lock = (MRSW_LOCK_T*) malloc(sizeof(MRSW_LOCK_T));


    if ( pt_rw_lock != NULL )
    {

        /* Create the various database access semaphores. Simply */
        /* abort if anyone of them cannot be created.            */
        if (u_sema_create ( &(pt_rw_lock->h_sem_wait_wr), X_SEMA_TYPE_BINARY,
                              X_SEMA_STATE_LOCK) != OSR_OK)
        {
            ABORT (DBG_CAT_SEMAPHORE, DBG_ABRT_CANNOT_CREATE_WR_WAIT_SEMAPHORE);
        }

        if (u_sema_create (&(pt_rw_lock->h_sem_wait_rd), X_SEMA_TYPE_BINARY,
                             X_SEMA_STATE_LOCK) != OSR_OK)
        {
            ABORT (DBG_CAT_SEMAPHORE, DBG_ABRT_CANNOT_CREATE_RD_WAIT_SEMAPHORE);
        }


        /* Initialize counter variables. */
        pt_rw_lock->ui2_wr_access_cnt    = 0;
        pt_rw_lock->ui2_wr_wait_cnt      = 0;
        pt_rw_lock->ui2_rd_access_cnt    = 0;
        pt_rw_lock->ui2_rd_wait_cnt      = 0;
        pt_rw_lock->ui2_rd_inside_wr_cnt = 0;

        /* Initializ the write thread_id variable. */
        pt_rw_lock->h_wr_thread = NULL_HANDLE;

        /* Set the lock delete status to False.  */
        pt_rw_lock->b_del_flag  = FALSE;

        /* create a handle and attach the rw_lock with the handle. */
        if ( u_handle_alloc( READ_WRITE_LOCK, (VOID*) pt_rw_lock,
                           NULL, rwl_handle_cb, ph_rw_lock ) != HR_OK )
        {
          ABORT(DBG_CAT_HANDLE,DBG_ABRT_OUT_OF_MEMORY);
        }
    }
    else
    {
      ABORT(DBG_CAT_MEMORY, DBG_ABRT_OUT_OF_MEMORY);
    }
    return RWLR_OK;
}


/*-----------------------------------------------------------------------------
 * Name: u_rwl_read_lock
 *
 * Description: This API locks the shared variable for a read operation.
 *              I case one or more write operations are active, this API will
 *              simply place the current thread into a holding semaphore until
 *              the write operations have concluded. Once this API returnes,
 *              the shared variable is locked for read operations. Note that
 *              the read lock allows for multiple read accesses.
 *
 * Inputs:  - Handle to the read-write lock object.
 *
 * Outputs: - None.
 *
 * Returns: - RWLR_OK             Read-lock has been acquired.
 *            RWLR_INVALID_HANDLE Invalid read-write lock handle.
 *            RWLR_DELETE_IN_PROGRESS This lock is being deleted.
 *            RWLR_WOULD_BLOCK        This API will block on waiting
 *                                    for the lock.
 ----------------------------------------------------------------------------*/
INT32 u_rwl_read_lock    (HANDLE_T h_rw_lock, RWL_OPTION_T e_option)
{
    BOOL            b_locked;
    CRIT_STATE_T    t_state;
    HANDLE_TYPE_T   e_handle_type;
    MRSW_LOCK_T     *pt_rw_lock ;
    HANDLE_T        h_my_thread;
    RET_MRSW_LOCK_U u_mrsw_lock;

    e_handle_type = INV_HANDLE_TYPE;

    /*
      Start the crtical section here.  If we start the crtical section
      after handle_get_obj(), there is a chance that a context switch
      take place, and that the object will become null (due to delete
      in progress) between the time we get the obj and when obj is used.
    */
    t_state = u_crit_start ();

    /* Get the current thread id value. */
    u_thread_self(&h_my_thread);


    /* Get the type and object reference to the handle. */
    if ( (u_handle_get_type_obj (h_rw_lock,
                               &e_handle_type,
                               &u_mrsw_lock.pv_MRSW_LOCK) == HR_OK) &&
         (e_handle_type == READ_WRITE_LOCK)
        )
    {
        pt_rw_lock = u_mrsw_lock.pt_MRSW_LOCK;

        /* Check to to see if the delete operation on this lock
           is in progress.  If the b_del_flag field in
           the Read-Write lock structure is not FALSE (e.g.,
           a delete request on this lock has been issued), then
           this thread will receive a condition indicating
           delete in progress.
        */
        if ( pt_rw_lock->b_del_flag != FALSE )
        {
            u_crit_end(t_state);
            return RWLR_DELETE_IN_PROGRESS;
        }

        b_locked = FALSE;
        
        #if dbg_rwl
        dbg_sys_printf("-------lock = %d, thread = %d, ra = %d, rw = %d, wa = %d, ww = %d-----read \n",
                    (INT32)pt_rw_lock,
                    (INT32)h_my_thread,
                    pt_rw_lock->ui2_rd_access_cnt,
                    pt_rw_lock->ui2_rd_wait_cnt,
                    pt_rw_lock->ui2_wr_access_cnt,
                    pt_rw_lock->ui2_wr_wait_cnt);
        #endif

        while (! (b_locked) )
        {
            /* Allow read operation if no write operation is  */
            /* active. This means, multiple read operation on */
            /* the component database are allowed.            */
            if ( (pt_rw_lock->ui2_wr_access_cnt == 0)&&
                 (pt_rw_lock->ui2_rd_wait_cnt == 0) &&
                 (pt_rw_lock->ui2_wr_wait_cnt == 0))
            {
                /* obtained read lock */
                pt_rw_lock->ui2_rd_access_cnt++;
                /*
                   Special check here: if rwlock is deleted, then
                   we should return a invalid status, so the
                   client will not proceed.
                */
                if ( pt_rw_lock->b_del_flag != FALSE )
                {
                    u_crit_end(t_state);
                    u_rwl_release_lock(h_rw_lock);
                    return RWLR_INVALID_HANDLE;
                }
                else
                {
                    u_crit_end (t_state);
                    b_locked = TRUE;
                }
            }
            else
            {
                /*
                  Assertion check.  Write thread can not be NULL
                */
              //  if ( pt_rw_lock->h_wr_thread == NULL_HANDLE )
              //  {
               //    ABORT (DBG_CAT_INV_OP, DBG_ABRT_INV_LOCK_STATE);
               // }
                /*
                  Special case: If the thread attempting the read
                  lock has alread obtain the write-lock, then the
                  read-lock will always succeed.
                */
                if ( pt_rw_lock->h_wr_thread == h_my_thread )
                {
                    pt_rw_lock->ui2_rd_inside_wr_cnt++;
                    u_crit_end (t_state);
                    b_locked = TRUE;
                }
                else
                {
                    /*
                      Normal case: where the thread reguesting the
                      read-lock is different than the write-lock
                      thread.
                    */
                    if ( e_option == RWL_OPTION_WAIT )
                    {
                        /* Busy with a write access. The read operation */
                        /* must wait until the write one is finished.   */
                        pt_rw_lock->ui2_rd_wait_cnt++;

                        u_crit_end (t_state);
                        if ( u_sema_lock (pt_rw_lock->h_sem_wait_rd,
                                          X_SEMA_OPTION_WAIT) != OSR_OK )
                        {
                            ABORT(DBG_CAT_SEMAPHORE,
                                  DBG_ABRT_CANNOT_LOCK_WAIT_SEMAPHORE);
                        }
                        // t_state = u_crit_start ();
                        //pt_rw_lock->ui2_rd_wait_cnt--;
                        //pt_rw_lock->ui2_rd_access_cnt++;
                        b_locked = TRUE;
                        //u_crit_end(t_state);
                        
                    }
                    else
                    {
                        /* Don't wait for the lock. return immediately */
                        u_crit_end (t_state);
                        return RWLR_WOULD_BLOCK;
                    }
                }
            }
        }
    }
    else
    {
        u_crit_end(t_state);
        return RWLR_INVALID_HANDLE ;
    }
    return RWLR_OK;
}


/*-----------------------------------------------------------------------------
 * Name: u_rwl_write_lock
 *
 * Description: This API locks the shared variable for a write / update
 *              operation. In the case where one or more read or
 *              another write operation is active, this API will
 *              simply place the current thread into a holding
 *              semaphore until the read or write operations have
 *              concluded. Once this API returnes, the component
 *              database is locked for write operations.
 *
 * Inputs:  - Handle to the read-write lock object.
 *
 * Outputs: - None.
 *
 * Returns: - RWLR_OK             Write-lock has been acquired.
 *            RWLR_INVALID_HANDLE Invalid read-write lock handle.
 *            RWLR_DELETE_IN_PROGRESS This lock is in the process of
 *                                    being deleted.
 ----------------------------------------------------------------------------*/
INT32 u_rwl_write_lock   (HANDLE_T h_rw_lock, RWL_OPTION_T e_option)
{
    BOOL            b_locked;
    CRIT_STATE_T    t_state;
    HANDLE_TYPE_T   e_handle_type;
    MRSW_LOCK_T     *pt_rw_lock ;
    HANDLE_T        h_my_thread;
	RET_MRSW_LOCK_U u_mrsw_lock;

    e_handle_type = INV_HANDLE_TYPE;
    /*
      Start the crtical section here.  If we start the crtical section
      after handle_get_obj(), there is a chance that a context switch
      take place, and that the object will become null (due to delete
      in progress) between the time we get the obj and when obj is used.
    */
    t_state=u_crit_start();

    /* Get the current thread id value. */
    u_thread_self(&h_my_thread);

    /* Get the type and object reference to the handle. */

    if ( (u_handle_get_type_obj (h_rw_lock,
                               &e_handle_type,
                               &u_mrsw_lock.pv_MRSW_LOCK) == HR_OK) &&
         (e_handle_type == READ_WRITE_LOCK)
        )
    {

          pt_rw_lock   = u_mrsw_lock.pt_MRSW_LOCK;

        /* Check to to see if the delete operation on this lock
           is in progress.  If the b_del_flag field in
           the Read-Write lock structure is not FALSE (e.g.,
           a delete request on this lock has been issued), then
           this thread will receive a condition indicating
           delete in progress.
        */
        if ( pt_rw_lock->b_del_flag != FALSE )
        {
            u_crit_end(t_state);
            return RWLR_DELETE_IN_PROGRESS;
        }

        b_locked = FALSE;

        #if dbg_rwl
        dbg_sys_printf("-------lock = %d, thread = %d, ra = %d, rw = %d, wa = %d, ww = %d-----write \n",
                    (INT32)pt_rw_lock,
                    (INT32)h_my_thread,
                    pt_rw_lock->ui2_rd_access_cnt,
                    pt_rw_lock->ui2_rd_wait_cnt,
                    pt_rw_lock->ui2_wr_access_cnt,
                    pt_rw_lock->ui2_wr_wait_cnt);
        #endif
        while (! (b_locked))
        {
            /* Allow write operation ONLY if no other */
            /* read or write operation is active.     */
            if ((pt_rw_lock->ui2_rd_access_cnt == 0)  &&
                (pt_rw_lock->ui2_wr_access_cnt == 0)  &&
                (pt_rw_lock->ui2_rd_wait_cnt == 0)    &&
                (pt_rw_lock->ui2_wr_wait_cnt == 0))
            {
                    pt_rw_lock->ui2_wr_access_cnt++;
                    pt_rw_lock->h_wr_thread = h_my_thread;
                    /*
                       Special check here: if rwlock is deleted, then
                       we should return a invalid status, so the
                       client will not proceed.
                    */
                    if ( pt_rw_lock->b_del_flag != FALSE )
                    {
                        u_crit_end(t_state);
                        u_rwl_release_lock(h_rw_lock);
                        return RWLR_INVALID_HANDLE;
                    }
                    else
                    {
                        u_crit_end (t_state);
                        b_locked = TRUE;
                    }
                
            }
            else
            {
                /*
                  Allow the thread that already acquire the write lock
                  to acquire the write lock again, if no read lock is
                  in effect.
                */
                if (  (pt_rw_lock->ui2_rd_access_cnt == 0)    &&
                      (pt_rw_lock->ui2_rd_inside_wr_cnt == 0) &&
                      (pt_rw_lock->ui2_wr_access_cnt >  0)    &&
                      (pt_rw_lock->h_wr_thread == h_my_thread)
                    )
                 {
                     /* obtained write lock */
                     pt_rw_lock->ui2_wr_access_cnt++;
                     
                     /*
                       Special check here: if rwlock is deleted, then
                       we should return a invalid status, so the
                       client will not proceed.
                     */
                     if ( pt_rw_lock->b_del_flag != FALSE )
                     {
                         u_crit_end(t_state);
                         u_rwl_release_lock(h_rw_lock);
                         return RWLR_INVALID_HANDLE;
                     }
                     else
                     {
                         u_crit_end (t_state);
                         b_locked = TRUE;
                     }
                 }
                 else
                 {
                     if ( e_option == RWL_OPTION_WAIT )
                     {
                         /* Some other READ or WRITE operation is active.
                            Simply */
                         /* put this thread into the waiting queue. */
                         pt_rw_lock->ui2_wr_wait_cnt++;

                         u_crit_end (t_state);
                         if ( u_sema_lock (pt_rw_lock->h_sem_wait_wr,
                                           X_SEMA_OPTION_WAIT) != OSR_OK )
                         {
                             ABORT(DBG_CAT_SEMAPHORE,
                                   DBG_ABRT_CANNOT_LOCK_WAIT_SEMAPHORE);
                         }
                          t_state = u_crit_start ();
                         //pt_rw_lock->ui2_wr_access_cnt++;
                          pt_rw_lock->h_wr_thread = h_my_thread;
                         // pt_rw_lock->ui2_wr_wait_cnt--;
                         b_locked = TRUE;
                         u_crit_end (t_state);

                         
                     }
                     else
                     {
                         /* Don't wait for the lock. return immediately */
                         u_crit_end (t_state);
                         return RWLR_WOULD_BLOCK;
                     }
                 }
            }
        }  /* end of while loop. */
    }
    else
    {
        u_crit_end(t_state);
        return RWLR_INVALID_HANDLE ;
    }
    return RWLR_OK;
}


/*-----------------------------------------------------------------------------
 * Name: u_rwl_release_lock
 *
 * Description: This API unlocks a previously locked shared variable. The
 *              shared variable was locked for a read or write operation.
 *
 * Inputs:  - Handle to the read-write lock object.
 *
 * Outputs: - None.
 *
 * Returns: - RWLR_OK               Read-Write lock has been released.
 *            RWLR_INVALID_HANDLE   Invalid read-write lock handle.
 ----------------------------------------------------------------------------*/
INT32 u_rwl_release_lock (HANDLE_T h_rw_lock)
{
    CRIT_STATE_T t_state;
    HANDLE_TYPE_T   e_handle_type;
    MRSW_LOCK_T     *pt_rw_lock ;
    HANDLE_T        h_my_thread;
    BOOL            b_crit = FALSE;
	RET_MRSW_LOCK_U u_mrsw_lock;

    BOOL            b_del_flag = FALSE;
    UINT16          ui2_wr_access_cnt = 0;
    UINT16          ui2_wr_wait_cnt   = 0;
    UINT16          ui2_rd_access_cnt = 0;
    UINT16          ui2_rd_wait_cnt   = 0;

#define RWLOCK_COPY_STATE_INFO  {b_del_flag=pt_rw_lock->b_del_flag ; ui2_wr_access_cnt=pt_rw_lock->ui2_wr_access_cnt ; ui2_wr_wait_cnt=pt_rw_lock->ui2_wr_wait_cnt ; ui2_rd_access_cnt = pt_rw_lock->ui2_rd_access_cnt ; ui2_rd_wait_cnt=pt_rw_lock->ui2_rd_wait_cnt;}

    e_handle_type = INV_HANDLE_TYPE;

    /*
      Start the crtical section here.  If we start the crtical section
      after u_handle_get_type_obj(), there is a chance that a context switch
      take place, and that the object will become null (due to delete
      in progress) between the time we get the obj and when obj is used.
    */
    t_state = u_crit_start ();
    b_crit = TRUE;

    /* Get the current thread id value. */
    u_thread_self(&h_my_thread);


    /* Get the type and object reference to the handle. */
    if ( (u_handle_get_type_obj (h_rw_lock,
                               &e_handle_type,
                                &u_mrsw_lock.pv_MRSW_LOCK) == HR_OK) &&
         (e_handle_type == READ_WRITE_LOCK)
        )
    {

         pt_rw_lock = u_mrsw_lock.pt_MRSW_LOCK;
         
         #if dbg_rwl
         dbg_sys_printf("------- lock = %d, thread = %d, ra = %d, rw = %d, wa = %d, ww = %d-----release\n",
                        (INT32)pt_rw_lock,
                        (INT32)h_my_thread,
                        pt_rw_lock->ui2_rd_access_cnt,
                        pt_rw_lock->ui2_rd_wait_cnt,
                        pt_rw_lock->ui2_wr_access_cnt,
                        pt_rw_lock->ui2_wr_wait_cnt);
         #endif
        /* Assertion: If the read and write counters are both  */
        /* greater than '0' then there is a serious problem.       */
        if ((pt_rw_lock->ui2_rd_access_cnt > 0)  &&
            (pt_rw_lock->ui2_wr_access_cnt > 0))
        {
            ABORT (DBG_CAT_INV_OP, DBG_ABRT_INV_LOCK_STATE);
        }

        /*
          Special case:  the read-lock is being released from inside
          a write-lock, in the same thread context.
        */
        if (pt_rw_lock->ui2_rd_inside_wr_cnt > 0 )
        {
            /*
              Assertion check. There must already be
              a thread holding the write-lock.
            */
            if ( ! (pt_rw_lock->ui2_wr_access_cnt > 0) )
            {
                ABORT (DBG_CAT_INV_OP, DBG_ABRT_INV_LOCK_STATE);
            }

            /*
              Assertion check: the thread that is releaseing
              the read-lock enclosed by the write-lock must
              be the same thread.
            
                    if ( pt_rw_lock->h_wr_thread != h_my_thread )
                    {
                        ABORT (DBG_CAT_INV_OP, DBG_ABRT_INV_LOCK_STATE);
                    }
            */


            pt_rw_lock->ui2_rd_inside_wr_cnt--;
            b_crit = FALSE;
            u_crit_end (t_state);

            /*
               Don't wake up other pending read/write operation.
               We must keep the existing write-lock state.
            */
            return RWLR_OK;
        }

        /*
          Normal case: releasing read or write lock.
        */
        if (pt_rw_lock->ui2_rd_access_cnt > 0)
        {
            pt_rw_lock->ui2_rd_access_cnt--;

            /*
              Wake up any pending READ
            */
            if (pt_rw_lock->ui2_rd_wait_cnt > 0
                &&(pt_rw_lock->ui2_rd_access_cnt ==0)) //if pt_rw_lock->ui2_rd_access_cnt > 0,  there must be some thread hold lock, So can't  unlock this lock 
            {
                /* make local copy of rwlock state variables before leaving critical sect */
                RWLOCK_COPY_STATE_INFO ;

                /* If some more read operations are pending, unlock them. */
                b_crit = FALSE;
                pt_rw_lock->ui2_rd_wait_cnt--;
                pt_rw_lock->ui2_rd_access_cnt++;

                u_crit_end (t_state);

                if ( u_sema_unlock (pt_rw_lock->h_sem_wait_rd) != OSR_OK )
                {
                    ABORT(DBG_CAT_SEMAPHORE,DBG_ABRT_CANNOT_UNLOCK_WAIT_SEMAPHORE);
                }

            }
            else
            {
                /* Wake up any pending WRITE. */
                /* Note: Only allow a write operation if no more */
                /* read operations are active.             */
                if ((pt_rw_lock->ui2_rd_access_cnt == 0) &&
                    (pt_rw_lock->ui2_wr_wait_cnt > 0))
                {
                    /* make local copy of rwlock state variables before leaving critical sect */
                    RWLOCK_COPY_STATE_INFO ;

                    /* Wake up any pending write lock operation. */
                    b_crit = FALSE;
                    pt_rw_lock->ui2_wr_access_cnt++;
                    //pt_rw_lock->h_wr_thread = h_my_thread;
                    pt_rw_lock->ui2_wr_wait_cnt--;
                    u_crit_end (t_state);

                    if ( u_sema_unlock (pt_rw_lock->h_sem_wait_wr) != OSR_OK )
                    {
                        ABORT(DBG_CAT_SEMAPHORE,DBG_ABRT_CANNOT_UNLOCK_WAIT_SEMAPHORE);
                    }
                }
            }
        }
        else
        {
            if ( pt_rw_lock->ui2_wr_access_cnt > 0 )
            {
                /*
                   Asserting check:
                   The thread performing write-unlock should be the
                   same thread that is currently holding the write lock.
               
                    if ( pt_rw_lock->h_wr_thread != h_my_thread )
                    {
                                   ABORT(DBG_CAT_SEMAPHORE,DBG_ABRT_INV_LOCK_STATE);
                    }
                */

                pt_rw_lock->ui2_wr_access_cnt--;

                /*
                  Nullified the write-thread only if all the
                  last write lock has been released.
                */
                if ( pt_rw_lock->ui2_wr_access_cnt == 0 )
                {
                    pt_rw_lock->h_wr_thread = NULL_HANDLE;
                }

                /*
                  Wake up any pending WRITE.
                */
                if ((pt_rw_lock->ui2_wr_access_cnt == 0) &&
                    (pt_rw_lock->ui2_wr_wait_cnt > 0))
                {
                    /* make local copy of rwlock state variables before leaving critical sect */
                    RWLOCK_COPY_STATE_INFO ;

                    /* If some more write operations are pending, unlock them.*/
                    b_crit = FALSE;
                    pt_rw_lock->ui2_wr_access_cnt++;
                    pt_rw_lock->h_wr_thread = h_my_thread;
                    pt_rw_lock->ui2_wr_wait_cnt--;

                    u_crit_end (t_state);

                    if ( u_sema_unlock (pt_rw_lock->h_sem_wait_wr) != OSR_OK )
                    {
                        ABORT(DBG_CAT_SEMAPHORE,DBG_ABRT_CANNOT_UNLOCK_WAIT_SEMAPHORE);
                    }

                }
                else
                {
                    /* Wake up any pending READ. */
                    /* Note: Only allow a read operation if no more */
                    /* write operations are active.           */
                    if ((pt_rw_lock->ui2_wr_access_cnt == 0) &&
                        (pt_rw_lock->ui2_rd_wait_cnt > 0))
                    {
                        /* make local copy of rwlock state variables before leaving critical sect */
                        RWLOCK_COPY_STATE_INFO ;

                        /* If some more read operations are pending, unlock them. */
                        b_crit = FALSE;  
                        pt_rw_lock->ui2_rd_wait_cnt--;
                        pt_rw_lock->ui2_rd_access_cnt++;
                        
                        u_crit_end (t_state);

                        if ( u_sema_unlock (pt_rw_lock->h_sem_wait_rd) != OSR_OK )
                        {
                            ABORT(DBG_CAT_SEMAPHORE,DBG_ABRT_CANNOT_UNLOCK_WAIT_SEMAPHORE);
                        }

                    }
                }
            }
        }


        if ( b_crit == TRUE )
        {
            RWLOCK_COPY_STATE_INFO ;
            u_crit_end (t_state);
        }

        /*
          All read/write are completed, and no other read/write are
          pending, and it is now safe to free lock object and
          its resource.
        */
        if ( b_del_flag        != FALSE       &&
             ui2_wr_access_cnt == 0           &&
             ui2_wr_wait_cnt   == 0           &&
             ui2_rd_access_cnt == 0           &&
             ui2_rd_wait_cnt   == 0
            )
        {
            _free_rwlock_resource(h_rw_lock, pt_rw_lock);
        }
        else
        {
            /*
              Some other read/write are still pending or the
              lock is not marked for deletion, Continue for
              now.
            */
        }
    }
    else
    {
        u_crit_end(t_state);
        return RWLR_INVALID_HANDLE ;
    }
    return RWLR_OK;
}

/*-----------------------------------------------------------------------------
 * Name: u_rwl_delete_lock
 *
 * Description: This API delete the Read-Write locks.
 *
 * Inputs:  - Handle to the read-write lock object.
 *
 * Outputs: - None.
 *
 * Returns: - RWLR_OK             Read-Write lock has been deleted.
 *            RWLR_INVALID_HANDLE Invalid read-write lock handle.
 *            RWLR_DELETE_IN_PROGRESS This lock is in the process of
 *                                    being deleted.
 ----------------------------------------------------------------------------*/
INT32 u_rwl_delete_lock   (HANDLE_T h_rw_lock)
{
    CRIT_STATE_T    t_state;
    HANDLE_TYPE_T   e_handle_type;
    MRSW_LOCK_T     *pt_rw_lock ;
	RET_MRSW_LOCK_U u_mrsw_lock;

    e_handle_type = INV_HANDLE_TYPE;
    /*
      Start the crtical section here.  If we start the crtical section
      after handle_get_obj(), there is a chance that a context switch
      take place, and that the object will become null (due to delete
      in progress) between the time we get the obj and when obj is used.
    */
    t_state=u_crit_start();

    /* Get the reference to the handle. */
    if ( (u_handle_get_type_obj (h_rw_lock,
                               &e_handle_type,
                               &u_mrsw_lock.pv_MRSW_LOCK) == HR_OK) &&
         (e_handle_type == READ_WRITE_LOCK)
        )
    {
        pt_rw_lock = u_mrsw_lock.pt_MRSW_LOCK;
        /* If the b_del_flag field in the Read-Write lock
           structure is not FALSE, then some other thread has
           call the delete lock operation on this lock. If
           this is the case, then we return with a status indicating
           this condition.
        */
        if ( pt_rw_lock->b_del_flag == FALSE )
        {
            /* Change the b_del_flag to TRUE value. This
               prevents any other client from making new READ or WRITE
               requests. */
            pt_rw_lock->b_del_flag = TRUE;
        }
        else
        {
            u_crit_end(t_state);
            return RWLR_DELETE_IN_PROGRESS;
        }

        /*
          If all othe read/write has completed and none are pending, then
          it is safe to remove the lock object.

        */
        if ( pt_rw_lock->ui2_wr_access_cnt == 0 &&
             pt_rw_lock->ui2_wr_wait_cnt   == 0 &&
             pt_rw_lock->ui2_rd_access_cnt == 0 &&
             pt_rw_lock->ui2_rd_wait_cnt   == 0
            )
        {
            u_crit_end (t_state);
            _free_rwlock_resource(h_rw_lock,  pt_rw_lock);
        }
        else
        {
            /*
              If there are pending and existing read/write operations, then
              the lock can not be free yet.  Just continue now and
              at some future point the resource for the rwlock will be free
              when the last thread calls the release_lock api.
            */
            u_crit_end(t_state);
        }
    }
    else
    {
        u_crit_end(t_state);
        return RWLR_INVALID_HANDLE ;
    }
    return RWLR_OK;
}

/*------------------------------------------------------------------------
 * Name: rwl_read_lock_grabbed
 *
 * Description:  This API checks if the read lock is currently active.
 *               The API returns TRUE, if one or more threads have the
 *               read lock on 'h_rw_lock' handle.  It returns FALSE if no
 *               threads has the read lock on 'h_rw_lock' handle.
 *
 * Inputs:
 *   h_rw_lock:  Handle to the read-write lock object.
 *
 * Outputs:
 *   None.
 *
 * Returns:
 *   TRUE        Some thread(s) have grabbed the read lock on the
 *               specified rwlock handle.
 *
 *   FALSE       No thread(s) has the read lock on the specified rwlock
 *               handle.
 ------------------------------------------------------------------------*/
BOOL  rwl_read_lock_grabbed(HANDLE_T  h_rw_lock)
{
    CRIT_STATE_T    t_state;
    HANDLE_TYPE_T   e_handle_type;
    MRSW_LOCK_T     *pt_rw_lock ;
    BOOL            b_result;
	RET_MRSW_LOCK_U u_mrsw_lock;

    e_handle_type = INV_HANDLE_TYPE;
    b_result      = FALSE;
    /*
      Start the crtical section here.  If we start the crtical section
      after handle_get_obj(), there is a chance that a context switch
      take place, and that the object will become null (due to delete
      in progress) between the time we get the obj and when obj is used.
    */
    t_state=u_crit_start();

    /* Get the reference to the handle. */
    if ( (u_handle_get_type_obj (h_rw_lock,
                               &e_handle_type,
                               &u_mrsw_lock.pv_MRSW_LOCK) == HR_OK) &&
         (e_handle_type == READ_WRITE_LOCK)
        )
    {
        pt_rw_lock = u_mrsw_lock.pt_MRSW_LOCK;
		if ( (pt_rw_lock->ui2_rd_access_cnt > 0 )  ||
             (pt_rw_lock->ui2_rd_inside_wr_cnt > 0 )
            )
        {
            b_result = TRUE;
        }
    }

    u_crit_end(t_state);

    return b_result;
}


/*-------------------------------------------------------------------------
 * Name: u_rwl_write_lock_grabbed
 *
 * Description: This API checks if the write lock on 'h_rw_lock' handle
 *              has been acquired by the caller thread.    The API returns
 *              TRUE only if the current thread has acquired the write lock.
 *              The API returns FALSE if another thread has acquired the
 *              write lock on 'h_rw_lock', or that no write lock on
 *              'h_rw_lock' handle is currently active.
 *
 * Inputs:
 *   h_rw_lock: Handle to the read-write lock object.
 *
 * Outputs:
 *   None.
 *
 * Returns:
 *   TRUE	    The current thread context (e.g., the thread that calls
 *              this API) has acquired the write lock on specified rwlock
 *              handle.
 *
 *   FALSE      The current thread context (e.g., the thread that calls
 *              this API) does not have the write lock on the specified
 *              rwlock handle.
 *
 ------------------------------------------------------------------------*/
BOOL  u_rwl_write_lock_grabbed(HANDLE_T h_rw_lock)
{
    CRIT_STATE_T    t_state;
    HANDLE_TYPE_T   e_handle_type;
    MRSW_LOCK_T     *pt_rw_lock ;
    BOOL            b_result;
    HANDLE_T        h_my_thread;
	RET_MRSW_LOCK_U u_mrsw_lock;

    /* Get the current thread id value. */
    u_thread_self(&h_my_thread);

    e_handle_type = INV_HANDLE_TYPE;
    b_result      = FALSE;
    /*
      Start the crtical section here.  If we start the crtical section
      after handle_get_obj(), there is a chance that a context switch
      take place, and that the object will become null (due to delete
      in progress) between the time we get the obj and when obj is used.
    */
    t_state=u_crit_start();

    /* Get the reference to the handle. */
    if ( (u_handle_get_type_obj (h_rw_lock,
                               &e_handle_type,
                               &(u_mrsw_lock.pv_MRSW_LOCK)) == HR_OK) &&
         (e_handle_type == READ_WRITE_LOCK)
        )
    {
        pt_rw_lock = u_mrsw_lock.pt_MRSW_LOCK;
		if ( (pt_rw_lock->h_wr_thread == h_my_thread) &&
             (pt_rw_lock->ui2_wr_access_cnt > 0)
            )
        {
            b_result = TRUE;
        }
    }

    u_crit_end(t_state);

    return b_result;
}
