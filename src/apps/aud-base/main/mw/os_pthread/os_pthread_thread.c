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




#include <errno.h>
#include <limits.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/mman.h>
#include <sys/prctl.h>
#include <signal.h>
#include "u_assert.h"
#include "u_dbg.h"
#include "u_os.h"



#define INVALID_THREAD          NULL
#define MEMORY_ALIGNMENT        8
#define THREAD_NAME_LEN         16
#define THREAD_PRI_RANGE_LOW    (UINT8) 1
#define THREAD_PRI_RANGE_HIGH   (UINT8) 254
#define DEFAULT_TASK_SLICE      5


typedef struct os_thread_pvt_light
{
    struct os_thread_pvt_light *previous;
    struct os_thread_pvt_light *next;

    UINT32  ui4_key;
    u_os_thread_pvt_del_fct  pf_pvt_del;
    VOID *pv_pvt;
} OS_THREAD_PVT_LIGHT_T;


typedef struct os_thread_light
{
    struct os_thread_light *previous;
    struct os_thread_light *next;
    pthread_t thread;
    pthread_cond_t cond;
    pthread_mutex_t mutex;
    BOOL suspend;

    SIZE_T z_stack_size;
    CHAR s_name[THREAD_NAME_LEN + 1];
    u_os_thread_main_fct pf_main_rtn;
    OS_THREAD_PVT_LIGHT_T *pt_pvt;
    SIZE_T z_arg_size;
    UINT8 au1_arg_local[1];
} OS_THREAD_LIGHT_T;

//---------------------------------------------------------------------


static UINT8 from_sched_priority(int sched_priority)
{
    return (UINT8)((100 - sched_priority) * 256 / 100);
}


static int to_sched_priority(UINT8 ui1_priority)
{
    int sched_priority;
    sched_priority = 100 - (int)ui1_priority * 100 / 256;
    if (sched_priority < 1) sched_priority = 1;
    if (sched_priority > 99) sched_priority = 99;
    return sched_priority;
}


//---------------------------------------------------------------------


// Reaper thread attributes
#define REAPER_STACK_SIZE       PTHREAD_STACK_MIN

// thread exit reaper
static struct
{
    pthread_t thread;
    pthread_cond_t cond;
    pthread_mutex_t mutex;
    OS_THREAD_LIGHT_T *thread_exit;
    BOOL quit;
} s_exitReaper;


static void *ExitReaperProc(void *arg)
{
    OS_THREAD_LIGHT_T *pt_thread;

    UNUSED(arg);

    VERIFY(pthread_mutex_lock(&s_exitReaper.mutex) == 0);
    while (!s_exitReaper.quit)
    {
        // Wait for exiting request
        VERIFY(pthread_cond_wait(&s_exitReaper.cond, &s_exitReaper.mutex) == 0);

        // Extract thread to be terminated
        pt_thread = s_exitReaper.thread_exit;
        ASSERT(pt_thread != NULL);

        // Wait for thread terminated
        if (pt_thread->z_stack_size != 0)
        {
            VERIFY(pthread_join(pt_thread->thread, NULL) == 0);
        }

        // Release resources
        VERIFY(pthread_mutex_destroy(&pt_thread->mutex) == 0);
        VERIFY(pthread_cond_destroy(&pt_thread->cond) == 0);

        free(pt_thread);
    }
    VERIFY(pthread_mutex_unlock(&s_exitReaper.mutex) == 0);

    return NULL;
}


static void SignalReaper(OS_THREAD_LIGHT_T *pt_thread)
{
    VERIFY(pthread_mutex_lock(&s_exitReaper.mutex) == 0);
    s_exitReaper.thread_exit = pt_thread;
    VERIFY(pthread_cond_signal(&s_exitReaper.cond) == 0);
    VERIFY(pthread_mutex_unlock(&s_exitReaper.mutex) == 0);
}


static void InitReaper(void)
{
    pthread_attr_t     attr;
    struct sched_param param;

    // Initialize mailbox used to perform thread exiting
    VERIFY(pthread_cond_init(&s_exitReaper.cond, NULL) == 0);
    VERIFY(pthread_mutex_init(&s_exitReaper.mutex, NULL) == 0);

    // Create the exit reaper thread
    VERIFY(pthread_attr_init(&attr) == 0);
    VERIFY(pthread_attr_setstacksize(&attr, REAPER_STACK_SIZE) == 0);
    //VERIFY(pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED) == 0);
    //VERIFY(pthread_attr_setschedpolicy(&attr, SCHED_RR) == 0);
    //param.sched_priority = to_sched_priority(1);
    //VERIFY(pthread_attr_setschedparam(&attr, &param) == 0);
    VERIFY(pthread_create(&s_exitReaper.thread, &attr, ExitReaperProc, NULL) == 0);
    VERIFY(pthread_attr_destroy(&attr) == 0);
}



//---------------------------------------------------------------------


static pthread_key_t s_thread_key;
static pthread_rwlock_t s_thread_list_mutex;
static OS_THREAD_LIGHT_T *s_thread_list;

static void thread_list_add(OS_THREAD_LIGHT_T *pt_thread)
{
    if (s_thread_list != NULL)
    {
        pt_thread->previous = s_thread_list->previous;
        pt_thread->next = s_thread_list;
        s_thread_list->previous->next = pt_thread;
        s_thread_list->previous = pt_thread;
    }
    else
    {
        s_thread_list = pt_thread->next = pt_thread->previous = pt_thread;
    }
}


static void thread_list_remove(OS_THREAD_LIGHT_T *pt_thread)
{
    if (pt_thread->previous == pt_thread)
    {
        s_thread_list = NULL;
    }
    else
    {
        pt_thread->previous->next = pt_thread->next;
        pt_thread->next->previous = pt_thread->previous;
        if (s_thread_list == pt_thread)
        {
            s_thread_list = pt_thread->next;
        }
    }
}


static OS_THREAD_LIGHT_T *thread_find_handle(HANDLE_T h_th_hdl)
{
    OS_THREAD_LIGHT_T *pt_thread = s_thread_list;
    if (pt_thread == NULL)
    {
        return NULL;
    }
    do
    {
        if (pt_thread == (OS_THREAD_LIGHT_T *)(h_th_hdl))
        {
            return pt_thread;
        }
        pt_thread = pt_thread->next;
    } while (pt_thread != s_thread_list);

    return NULL;
}


static OS_THREAD_LIGHT_T *thread_find_obj(const CHAR *ps_name)
{
    OS_THREAD_LIGHT_T *pt_thread = s_thread_list;
    if (pt_thread == NULL)
    {
        return NULL;
    }
    do
    {
        if (strncmp(pt_thread->s_name, ps_name, THREAD_NAME_LEN) == 0)
        {
            return pt_thread;
        }
        pt_thread = pt_thread->next;
    } while (pt_thread != s_thread_list);

    return NULL;
}


static void *ThreadProc(void *arg)
{
    OS_THREAD_LIGHT_T *pt_thread = (OS_THREAD_LIGHT_T *)arg;
    ASSERT(pt_thread != NULL);

    prctl(PR_SET_NAME, pt_thread->s_name);

    VERIFY(pthread_setspecific(s_thread_key, pt_thread) == 0);

    // Invoke the original thread function
    pt_thread->pf_main_rtn(pt_thread->z_arg_size != 0 ? pt_thread->au1_arg_local : NULL);

    pthread_exit(NULL);

    return NULL;
}


static INT32 ThreadUpgrade(void)
{
    OS_THREAD_LIGHT_T *pt_thread;


    pt_thread = calloc(1, sizeof(OS_THREAD_LIGHT_T) - sizeof(UINT8));
    if (pt_thread == NULL)
    {
        return OSR_NO_RESOURCE;
    }
    pt_thread->thread = pthread_self();
    VERIFY(pthread_cond_init(&pt_thread->cond, NULL) == 0);
    VERIFY(pthread_mutex_init(&pt_thread->mutex, NULL) == 0);

    VERIFY(pthread_rwlock_wrlock(&s_thread_list_mutex) == 0);
    thread_list_add(pt_thread);
    VERIFY(pthread_rwlock_unlock(&s_thread_list_mutex) == 0);

    VERIFY(pthread_setspecific(s_thread_key, pt_thread) == 0);
    return OSR_OK;
}


INT32 u_thread_create (HANDLE_T*             ph_th_hdl,
                       const CHAR*           ps_name,
                       SIZE_T                z_stack_size,
                       UINT8                 ui1_priority,
                       u_os_thread_main_fct  pf_main_rtn,
                       SIZE_T                z_arg_size,
                       VOID*                 pv_arg)
{
    OS_THREAD_LIGHT_T *pt_thread;

    pthread_attr_t     attr;
    struct sched_param param;

    if (pv_arg == NULL)
    {
        z_arg_size = 0;
    }

    /* check arguments */
    if ((ps_name == NULL) || (ps_name[0] == '\0') || (ph_th_hdl == NULL) ||
        (z_stack_size == 0) || (pf_main_rtn == NULL) ||
        (ui1_priority < THREAD_PRI_RANGE_LOW) || (ui1_priority > THREAD_PRI_RANGE_HIGH) ||
        ((pv_arg != NULL) && (z_arg_size == 0)) ||
        ((pv_arg == NULL) && (z_arg_size != 0)))
    {
        return OSR_INV_ARG;
    }

    // Make sure the stack size is aligned
    z_stack_size = (z_stack_size + MEMORY_ALIGNMENT - 1) & (~(MEMORY_ALIGNMENT - 1));
    if (z_stack_size < PTHREAD_STACK_MIN)
    {
        z_stack_size = PTHREAD_STACK_MIN;
    }
#ifdef POSIX_THREAD_STACKSIZE_MAX
    if (z_stack_size > POSIX_THREAD_STACKSIZE_MAX)
    {
        z_stack_size = POSIX_THREAD_STACKSIZE_MAX;
    }
#endif

    pt_thread = calloc(1, sizeof(OS_THREAD_LIGHT_T) - sizeof(UINT8) + z_arg_size);

    if (pt_thread == NULL)
    {
        free(pt_thread);
        return OSR_NO_RESOURCE;
    }
    pt_thread->z_stack_size = z_stack_size;
    strncpy(pt_thread->s_name, ps_name, THREAD_NAME_LEN);
    pt_thread->pf_main_rtn = pf_main_rtn;
    pt_thread->z_arg_size = z_arg_size;
    if (z_arg_size != 0)
    {
        memcpy(pt_thread->au1_arg_local, pv_arg, z_arg_size);
    }

    VERIFY(pthread_cond_init(&pt_thread->cond, NULL) == 0);
    VERIFY(pthread_mutex_init(&pt_thread->mutex, NULL) == 0);

    VERIFY(pthread_attr_init(&attr) == 0);
    VERIFY(pthread_attr_setstacksize(&attr, z_stack_size) == 0);
    //VERIFY(pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED) == 0);
    //VERIFY(pthread_attr_setschedpolicy(&attr, SCHED_RR) == 0);
    //param.sched_priority = to_sched_priority(ui1_priority);
    //VERIFY(pthread_attr_setschedparam(&attr, &param) == 0);

    VERIFY(pthread_rwlock_wrlock(&s_thread_list_mutex) == 0);
    if (thread_find_obj(ps_name) != NULL)
    {
        VERIFY(pthread_rwlock_unlock(&s_thread_list_mutex) == 0);
        free(pt_thread);
        return OSR_EXIST;
    }

    if (pthread_create(&pt_thread->thread, &attr, ThreadProc, pt_thread) != 0)
    {
        VERIFY(pthread_rwlock_unlock(&s_thread_list_mutex) == 0);
        free(pt_thread);
        return OSR_FAIL;
    }

    thread_list_add(pt_thread);
    VERIFY(pthread_rwlock_unlock(&s_thread_list_mutex) == 0);

    *ph_th_hdl = (HANDLE_T)(pt_thread);
    return OSR_OK;
}

INT32 u_thread_create_cli (HANDLE_T*             ph_th_hdl,
                       const CHAR*           ps_name,
                       SIZE_T                z_stack_size,
                       UINT8                 ui1_priority,
                       u_os_thread_main_fct  pf_main_rtn,
                       SIZE_T                z_arg_size,
                       VOID*                 pv_arg)
{
    OS_THREAD_LIGHT_T *pt_thread;

    pthread_attr_t     attr;
    struct sched_param param;

    if (pv_arg == NULL)
    {
        z_arg_size = 0;
    }

    /* check arguments */
    if ((ps_name == NULL) || (ps_name[0] == '\0') || (ph_th_hdl == NULL) ||
        (z_stack_size == 0) || (pf_main_rtn == NULL) ||
        (ui1_priority < THREAD_PRI_RANGE_LOW) || (ui1_priority > THREAD_PRI_RANGE_HIGH) ||
        ((pv_arg != NULL) && (z_arg_size == 0)) ||
        ((pv_arg == NULL) && (z_arg_size != 0)))
    {
        return OSR_INV_ARG;
    }

    // Make sure the stack size is aligned
    z_stack_size = (z_stack_size + MEMORY_ALIGNMENT - 1) & (~(MEMORY_ALIGNMENT - 1));
    if (z_stack_size < PTHREAD_STACK_MIN)
    {
        z_stack_size = PTHREAD_STACK_MIN;
    }
#ifdef POSIX_THREAD_STACKSIZE_MAX
    if (z_stack_size > POSIX_THREAD_STACKSIZE_MAX)
    {
        z_stack_size = POSIX_THREAD_STACKSIZE_MAX;
    }
#endif

    pt_thread = calloc(1, sizeof(OS_THREAD_LIGHT_T) - sizeof(UINT8) + z_arg_size);

    if (pt_thread == NULL)
    {
        free(pt_thread);
        return OSR_NO_RESOURCE;
    }
    pt_thread->z_stack_size = z_stack_size;
    strncpy(pt_thread->s_name, ps_name, THREAD_NAME_LEN);
    pt_thread->pf_main_rtn = pf_main_rtn;
    pt_thread->z_arg_size = z_arg_size;
    if (z_arg_size != 0)
    {
        memcpy(pt_thread->au1_arg_local, pv_arg, z_arg_size);
    }

    VERIFY(pthread_cond_init(&pt_thread->cond, NULL) == 0);
    VERIFY(pthread_mutex_init(&pt_thread->mutex, NULL) == 0);

    VERIFY(pthread_attr_init(&attr) == 0);
    VERIFY(pthread_attr_setstacksize(&attr, z_stack_size) == 0);
    //VERIFY(pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED) == 0);
    //VERIFY(pthread_attr_setschedpolicy(&attr, SCHED_RR) == 0);
    //param.sched_priority = to_sched_priority(ui1_priority);
    //VERIFY(pthread_attr_setschedparam(&attr, &param) == 0);

    VERIFY(pthread_rwlock_wrlock(&s_thread_list_mutex) == 0);
    if (thread_find_obj(ps_name) != NULL)
    {
        VERIFY(pthread_rwlock_unlock(&s_thread_list_mutex) == 0);
        free(pt_thread);
        return OSR_EXIST;
    }

    if (pthread_create(&pt_thread->thread, &attr, ThreadProc, pt_thread) != 0)
    {
        VERIFY(pthread_rwlock_unlock(&s_thread_list_mutex) == 0);
        free(pt_thread);
        return OSR_FAIL;
    }

    thread_list_add(pt_thread);
    VERIFY(pthread_rwlock_unlock(&s_thread_list_mutex) == 0);

    *ph_th_hdl = (HANDLE_T)(pt_thread);
    return OSR_OK;
}

VOID u_thread_exit (VOID)
{
    pthread_exit(NULL);
}


VOID u_thread_delay (UINT32 ui4_delay)
{
    struct timespec rqtp;
    struct timespec rmtp;

    if (ui4_delay == 0)
    {
        // Delaying 0 means yielding execution
        sched_yield();
        return;
    }

    rqtp.tv_sec  = ui4_delay / 1000; ui4_delay %= 1000;
    rqtp.tv_nsec = ui4_delay * 1000000;
    while (nanosleep(&rqtp, &rmtp) != 0)
    {
        ASSERT(errno == EINTR);
        rqtp = rmtp;
    }
}


INT32 u_thread_set_pri (HANDLE_T  h_th_hdl,
                        UINT8     ui1_new_pri)
{
    OS_THREAD_LIGHT_T *pt_thread;
    int policy;
    struct sched_param param;
    int ret;

    if ((ui1_new_pri < THREAD_PRI_RANGE_LOW) || (ui1_new_pri > THREAD_PRI_RANGE_HIGH))
    {
        return OSR_INV_ARG;
    }

    VERIFY(pthread_rwlock_wrlock(&s_thread_list_mutex) == 0);
    pt_thread = thread_find_handle(h_th_hdl);
    if (pt_thread == NULL)
    {
        VERIFY(pthread_rwlock_unlock(&s_thread_list_mutex) == 0);
        return OSR_INV_HANDLE;
    }

    if (pthread_getschedparam(pt_thread->thread, &policy, &param) != 0)
    {
        VERIFY(pthread_rwlock_unlock(&s_thread_list_mutex) == 0);
        return OSR_FAIL;
    }
    param.sched_priority = to_sched_priority(ui1_new_pri);
    ret = pthread_setschedparam(pt_thread->thread, policy, &param);
    VERIFY(pthread_rwlock_unlock(&s_thread_list_mutex) == 0);

    return ret == 0 ? OSR_OK : OSR_FAIL;
}

INT32 u_thread_get_name  (HANDLE_T  h_th_hdl,
                                      UINT32* s_name)
{
    OS_THREAD_LIGHT_T *pt_thread;

    VERIFY(pthread_rwlock_rdlock(&s_thread_list_mutex) == 0);
    pt_thread = thread_find_handle(h_th_hdl);
    if (pt_thread == NULL)
    {
        VERIFY(pthread_rwlock_unlock(&s_thread_list_mutex) == 0);
        return OSR_INV_HANDLE;
    }

    *s_name =  (UINT32) &pt_thread->s_name[0];
    VERIFY(pthread_rwlock_unlock(&s_thread_list_mutex) == 0);
    return OSR_OK;
}

INT32 u_thread_get_pri (HANDLE_T  h_th_hdl,
                        UINT8*    pui1_pri)
{
    OS_THREAD_LIGHT_T *pt_thread;
    int policy;
    struct sched_param param;
    int ret;

    if (pui1_pri == NULL)
    {
        return OSR_INV_ARG;
    }

    VERIFY(pthread_rwlock_rdlock(&s_thread_list_mutex) == 0);
    pt_thread = thread_find_handle(h_th_hdl);
    if (pt_thread == NULL)
    {
        VERIFY(pthread_rwlock_unlock(&s_thread_list_mutex) == 0);
        return OSR_INV_HANDLE;
    }
    ret = pthread_getschedparam(pt_thread->thread, &policy, &param);
    *pui1_pri = from_sched_priority(param.sched_priority);
    VERIFY(pthread_rwlock_unlock(&s_thread_list_mutex) == 0);

    return ret == 0 ? OSR_OK : OSR_FAIL;
}


VOID u_thread_suspend (VOID)
{
    OS_THREAD_LIGHT_T *pt_thread = (OS_THREAD_LIGHT_T *)pthread_getspecific(s_thread_key);
    ASSERT(pt_thread != INVALID_THREAD);
    VERIFY(pthread_mutex_lock(&pt_thread->mutex) == 0);
    pt_thread->suspend = TRUE;
    VERIFY(pthread_cond_wait(&pt_thread->cond, &pt_thread->mutex) == 0);
    pt_thread->suspend = FALSE;
    VERIFY(pthread_mutex_unlock(&pt_thread->mutex) == 0);
}


INT32 u_thread_resume (HANDLE_T  h_th_hdl)
{
    OS_THREAD_LIGHT_T *pt_thread;

    VERIFY(pthread_rwlock_wrlock(&s_thread_list_mutex) == 0);
    pt_thread = thread_find_handle(h_th_hdl);
    if (pt_thread == NULL)
    {
        VERIFY(pthread_rwlock_unlock(&s_thread_list_mutex) == 0);
        return OSR_INV_HANDLE;
    }

    VERIFY(pthread_mutex_lock(&pt_thread->mutex) == 0);
    if (pt_thread->suspend && pthread_cond_signal(&pt_thread->cond) != 0)
    {
        VERIFY(pthread_mutex_unlock(&pt_thread->mutex) == 0);
        VERIFY(pthread_rwlock_unlock(&s_thread_list_mutex) == 0);
        return OSR_FAIL;
    }
    VERIFY(pthread_mutex_unlock(&pt_thread->mutex) == 0);
    VERIFY(pthread_rwlock_unlock(&s_thread_list_mutex) == 0);
    return OSR_OK;
}



INT32 u_thread_self (HANDLE_T *ph_th_hdl)
{
    OS_THREAD_LIGHT_T *pt_thread;

    if (ph_th_hdl == NULL)
    {
        return OSR_INV_ARG;
    }
    pt_thread = (OS_THREAD_LIGHT_T *)pthread_getspecific(s_thread_key);
    if (pt_thread == INVALID_THREAD)
    {
        INT32 i4_ret = ThreadUpgrade();
        if (i4_ret != OSR_OK)
            return i4_ret;

        pt_thread = (OS_THREAD_LIGHT_T *)pthread_getspecific(s_thread_key);
        if (pt_thread == INVALID_THREAD)
            return OSR_NOT_EXIST;
    }

    *ph_th_hdl = (HANDLE_T)(pt_thread);
    return OSR_OK;
}


INT32 u_thread_stack_stats (HANDLE_T  h_th_hdl,
                            SIZE_T*   pz_alloc_stack,
                            SIZE_T*   pz_max_used_stack)
{

    return OSR_NOT_SUPPORT;
}


static OS_THREAD_PVT_LIGHT_T *thread_find_pvt_key(OS_THREAD_LIGHT_T *pt_thread, UINT32 ui4_key)
{
    OS_THREAD_PVT_LIGHT_T *pt_pvt;

    /* Find the private control block with matching key. */
    pt_pvt = pt_thread->pt_pvt;

    if (pt_pvt == NULL)
    {
        return NULL;
    }

    do
    {
        if (pt_pvt->ui4_key == ui4_key)
        {
            return pt_pvt;
        }
        pt_pvt = pt_pvt->next;
    } while (pt_pvt != pt_thread->pt_pvt);

    return NULL;
}


INT32 u_thread_set_pvt (UINT32                   ui4_key,
                        u_os_thread_pvt_del_fct  pf_pvt_del,
                        VOID*                    pv_pvt)
{
    OS_THREAD_LIGHT_T *pt_thread;
    OS_THREAD_PVT_LIGHT_T *pt_pvt;

    pt_thread = (OS_THREAD_LIGHT_T *)pthread_getspecific(s_thread_key);
    if (pt_thread == INVALID_THREAD)
    {
        return OSR_NOT_EXIST;
    }
    if (thread_find_pvt_key(pt_thread, ui4_key) != NULL)
    {
        return OSR_DUP_KEY;
    }
    pt_pvt = (OS_THREAD_PVT_LIGHT_T *)malloc(sizeof(OS_THREAD_PVT_LIGHT_T));
    if (pt_pvt == NULL)
    {
        return OSR_NO_RESOURCE;
    }

    if (pt_thread->pt_pvt != NULL)
    {
        pt_pvt->previous = pt_thread->pt_pvt->previous;
        pt_pvt->next     = pt_thread->pt_pvt;
        pt_thread->pt_pvt->previous->next = pt_pvt;
        pt_thread->pt_pvt->previous = pt_pvt;
    }
    else
    {
        pt_pvt->previous  = pt_pvt;
        pt_pvt->next      = pt_pvt;
        pt_thread->pt_pvt = pt_pvt;
    }
    pt_pvt->ui4_key    = ui4_key;
    pt_pvt->pf_pvt_del = pf_pvt_del;
    pt_pvt->pv_pvt     = pv_pvt;

    return OSR_OK;
}


INT32 u_thread_get_pvt (UINT32  ui4_key,
                        VOID**  ppv_pvt)
{
    OS_THREAD_LIGHT_T *pt_thread;
    OS_THREAD_PVT_LIGHT_T *pt_pvt;

    pt_thread = (OS_THREAD_LIGHT_T *)pthread_getspecific(s_thread_key);
    if (pt_thread == INVALID_THREAD)
    {
        return OSR_NOT_EXIST;
    }
    pt_pvt = thread_find_pvt_key(pt_thread, ui4_key);
    if (pt_pvt == NULL)
    {
        return OSR_KEY_NOT_FOUND;
    }
    *ppv_pvt = pt_pvt->pv_pvt;
    return OSR_OK;
}


static INT32 u_thread_del_pvt_impl (OS_THREAD_LIGHT_T *pt_thread, OS_THREAD_PVT_LIGHT_T *pt_pvt)
{
    if (pt_pvt->previous == pt_pvt)
    {
        pt_thread->pt_pvt = NULL;
    }
    else
    {
        pt_pvt->previous->next = pt_pvt->next;
        pt_pvt->next->previous = pt_pvt->previous;
        if (pt_thread->pt_pvt == pt_pvt)
        {
            pt_thread->pt_pvt = pt_pvt->next;
        }
    }
    if (pt_pvt->pf_pvt_del != NULL)
    {
        pt_pvt->pf_pvt_del(pt_pvt->ui4_key, pt_pvt->pv_pvt);
    }
    free(pt_pvt);
    return OSR_OK;
}


INT32 u_thread_del_pvt (UINT32 ui4_key)
{
    OS_THREAD_LIGHT_T *pt_thread;
    OS_THREAD_PVT_LIGHT_T *pt_pvt;

    pt_thread = (OS_THREAD_LIGHT_T *)pthread_getspecific(s_thread_key);
    if (pt_thread == INVALID_THREAD)
    {
        return OSR_NOT_EXIST;
    }
    pt_pvt = thread_find_pvt_key(pt_thread, ui4_key);
    if (pt_pvt == NULL)
    {
        return OSR_KEY_NOT_FOUND;
    }
    return u_thread_del_pvt_impl(pt_thread, pt_pvt);
}


static void thread_key_destructor(void *key)
{
    OS_THREAD_LIGHT_T *pt_thread = (OS_THREAD_LIGHT_T *)key;

    while (pt_thread->pt_pvt != NULL)
    {
        VERIFY(u_thread_del_pvt_impl(pt_thread, pt_thread->pt_pvt) == OSR_OK);
    }

    VERIFY(pthread_rwlock_wrlock(&s_thread_list_mutex) == 0);
    thread_list_remove(pt_thread);
    VERIFY(pthread_rwlock_unlock(&s_thread_list_mutex) == 0);

    SignalReaper(pt_thread);
}


INT32 os_thread_init(VOID)
{
    InitReaper();
    VERIFY(pthread_key_create(&s_thread_key, &thread_key_destructor) == 0);
    VERIFY(pthread_rwlock_init(&s_thread_list_mutex, NULL) == 0);


    return OSR_OK;
}

