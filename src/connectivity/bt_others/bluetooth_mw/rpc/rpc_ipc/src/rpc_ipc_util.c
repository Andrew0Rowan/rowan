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
 * MediaTek Inc. (C) 2016-2017. All rights reserved.
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


#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <semaphore.h>
#include <errno.h>

#include "_rpc_ipc_util.h"
#include "_ipc.h"

/* Should not be more than 255 levels of call stack */
#define MAX_LOGGER_STACK_DEPTH 255
typedef struct RPC_TL_LOGGER_T_
{
    INT32   i4_cur;
    rpc_log_fct pf_log_fct[MAX_LOGGER_STACK_DEPTH];
    INT32       i4_level[MAX_LOGGER_STACK_DEPTH];
}RPC_TL_LOGGER_T;

static OS_FNCT_T  t_os_fct_tbl = {NULL};

/* Local thread Key for logger */
static VOID * pv_tls_log_key = NULL;
/*
 * The total memory just record the usage, it might not
 * be the real memory used.
 */
static SIZE_T z_total_memory = 0;
static UINT8 ui_log_start_cnt = 0;

VOID * ipc_os_traced_malloc(SIZE_T z_size)
{
    VOID * pv_ret;

    pv_ret = malloc(z_size);
    if(pv_ret != NULL)
    {
        z_total_memory ++;
    }
    if((z_total_memory & 0xff) == 0)
    {
        RPC_LOG("MC M :%d\n", z_total_memory);
    }
    return pv_ret;
}

VOID * ipc_os_traced_calloc(SIZE_T z_num, SIZE_T z_size)
{
    VOID * pv_ret;
    pv_ret = calloc(z_num, z_size);
    if(pv_ret != NULL)
    {
        z_total_memory ++;
    }
    if((z_total_memory & 0xff) == 0)
    {
        RPC_LOG("MC C :%d\n", z_total_memory);
    }

    return pv_ret;
}

VOID ipc_os_traced_free(VOID * pv_addr)
{
    free(pv_addr);

    z_total_memory --;
    if((z_total_memory & 0xff) == 0)
    {
        RPC_LOG("MC F :%d\n", z_total_memory);
    }

}


/*------------------------------------------------------------------------
 * Name:
 *
 * Description: Init with OS function table.
 *
 * Inputs:
 * Outputs: -
 * Returns: -
 -----------------------------------------------------------------------*/
static VOID * pv_log_key;
VOID rpcu_os_init(const OS_FNCT_T * pt_os_fcts)
{
    if(pt_os_fcts != NULL)
    {
        t_os_fct_tbl = *pt_os_fcts;
    }
    else
    {
        t_os_fct_tbl.pf_thread_create = NULL;
    }

    //mtk40074 add  if(pv_log_key == NULL)
    if(pv_log_key == NULL)
    {
        pv_log_key = rpcu_os_new_tls_key(NULL);
    }
}

VOID rpcu_os_uninit()
{
    if(pv_log_key != NULL)
	{
		rpcu_os_delete_tls_key(pv_log_key);
		pv_log_key = NULL;
	}
}


BOOL rpcu_tl_log_is_on()
{
    BOOL *pb_on;

    printf("rpcu_tl_log_is_on is called\n");

    if (NULL == pv_log_key)
    {
        printf("pv_log_key is NULL\n");

        return FALSE;
    }

    pb_on = (BOOL *)(rpcu_os_tls_get(pv_log_key));
    if (NULL != pb_on)
    {
        printf("log is : %d\n", (int)(*pb_on));

        return *pb_on;
    }

    return FALSE;
}

EXPORT_SYMBOL BOOL bt_rpcu_tl_log_start()
{
    if (ui_log_start_cnt == 0)
      ui_log_start_cnt ++;
    else
    {
      printf("bt_rpcu_tl_log_start is already done\n");
      return TRUE;
    }

    printf("YZ bt_rpcu_tl_log_start is called\n");

    rpcu_tl_log_is_on();

    rpcu_os_tls_set(pv_log_key, (VOID *)TRUE);

    return TRUE;
}

EXPORT_SYMBOL VOID bt_rpcu_tl_log_end()
{
    rpcu_os_tls_set(pv_log_key, (VOID *)FALSE);
}


VOID rpcu_default_output_log(INT32 i4_level, CHAR* ps_string)
{
    printf("%s", ps_string);
}

typedef struct _PTHD_MAIN_ARG_T
{
    VOID *       pv_arg;
    os_main_fct  pf_main;
}PTHD_MAIN_ARG_T;

/*------------------------------------------------------------------------
 * Name:
 *
 * Description: Thread main function wrapper for pthread.
 *
 * Inputs:
 * Outputs: -
 * Returns: -
 -----------------------------------------------------------------------*/
static VOID * _pthread_main_wrapper(VOID * pv_arg)
{
    PTHD_MAIN_ARG_T * pt_arg = pv_arg;

    pt_arg->pf_main(pt_arg->pv_arg);

    RPC_FREE(pt_arg->pv_arg);
    RPC_FREE(pt_arg);

    pthread_exit(NULL);
    return NULL;
}

/*------------------------------------------------------------------------
 * Name:
 *
 * Description: Create a thread.
 *
 * Inputs:
 * Outputs: -
 * Returns: -
 -----------------------------------------------------------------------*/
INT32 rpcu_os_create_thread(
    VOID        (* pf_main)(VOID *),
    VOID *      pv_arg,
    SIZE_T      z_size,
    UINT8       ui1_prio,
    SIZE_T      z_stack_size)
{

    INT32           i4_ret;
    pthread_attr_t  t_attr;

    //pthread_attr_init(&t_attr);
    //pthread_attr_setinheritsched(&t_attr, PTHREAD_EXPLICIT_SCHED);

    if(t_os_fct_tbl.pf_thread_create == NULL)
    {
        pthread_t t_pth;
        pthread_attr_init(&t_attr);    //mtk40156
        pthread_attr_setinheritsched(&t_attr, PTHREAD_EXPLICIT_SCHED);//mtk40156
        if (z_stack_size > 0)
	{
            pthread_attr_setstacksize(&t_attr, z_stack_size);
        }
        PTHD_MAIN_ARG_T * pt_arg = RPC_MALLOC(sizeof(PTHD_MAIN_ARG_T));
        if(pt_arg == NULL)
        {
            pthread_attr_destroy(&t_attr);//mtk40156
            RPC_ERR("IPC thread start fail1: RPC_MALLOC(%d)\n", sizeof(PTHD_MAIN_ARG_T));
            return IPCR_OUTOFMEMORY;
        }
        pt_arg->pv_arg  = RPC_MALLOC(z_size);
        if(pt_arg->pv_arg  == NULL)
        {
            RPC_FREE(pt_arg);
            pthread_attr_destroy(&t_attr);//mtk40156
            RPC_ERR("IPC thread start fail2: RPC_MALLOC(%d)\n", z_size);
            return IPCR_OUTOFMEMORY;
        }

        pt_arg->pf_main = pf_main;
        memcpy(pt_arg->pv_arg, pv_arg, z_size);

        /* TODO: Fill other attributes */
        i4_ret = pthread_create(&t_pth,
                                &t_attr,
                                _pthread_main_wrapper,
                                pt_arg);
        if(i4_ret != 0)
        {
            RPC_FREE(pt_arg->pv_arg);
            RPC_FREE(pt_arg);
            pthread_attr_destroy(&t_attr);//mtk40156
            RPC_ERR("IPC thread start fail3, errorno(%d): %s\n", errno, strerror(errno));
            return IPCR_OSERR;
        }
	i4_ret = pthread_detach(t_pth);
	if (i4_ret != 0)  //thread finish will free under resouce.
	{
	    RPC_ERR("IPC thread start4(%ld)+, errorno(%d): %s, t_pth: %d\n", i4_ret, errno, strerror(errno), (int)t_pth);
	    pthread_attr_destroy(&t_attr);
            return IPCR_OSERR;
        }
	pthread_attr_destroy(&t_attr);

    }
    else
    {
        i4_ret = t_os_fct_tbl.pf_thread_create(pf_main,
                                             z_stack_size,
                                             ui1_prio,
                                             z_size,
                                             pv_arg);
	if (!i4_ret)
	{
	    RPC_ERR("IPC thread start fail 444\n");
	}
    }
    return IPCR_OK;
}

/*------------------------------------------------------------------------
 * Name: rpcu_os_get_cur_thd_desc()
 *
 * Description: Get a description from current thread
 *
 *
 ------------------------------------------------------------------------*/
INT32 rpcu_os_get_cur_thd_desc(RPCU_THREAD_DESC_T * pt_thd_desc)
{
    INT32               i4_ret;
    INT32               i4_policy;
    pthread_t           t_cur_thd;
    struct sched_param  t_param;

    t_cur_thd = pthread_self();

    if((i4_ret = pthread_getschedparam(t_cur_thd, (int *)&i4_policy, &t_param)) == 0)
    {
        pt_thd_desc->i4_policy = i4_policy;
        pt_thd_desc->i4_prio   = t_param.sched_priority;
    }
    else
    {
        return IPCR_OSERR;
    }

    return IPCR_OK;
}

/*------------------------------------------------------------------------
 * Name: rpcu_os_set_cur_thd_desc()
 *
 * Description: Set a description to current thread
 *
 *
 ------------------------------------------------------------------------*/
INT32 rpcu_os_set_cur_thd_desc(const RPCU_THREAD_DESC_T * pt_thd_desc)
{
    INT32               i4_ret;
    pthread_t           t_cur_thd;
    struct sched_param  t_param;

    t_cur_thd = pthread_self();

    t_param.sched_priority = pt_thd_desc->i4_prio;

    if((i4_ret = pthread_setschedparam(t_cur_thd, pt_thd_desc->i4_policy, &t_param)) != 0)
    {
        return IPCR_OSERR;
    }

    return IPCR_OK;
}

/*------------------------------------------------------------------------
 * Name: rpcu_os_thd_desc_eqaul()
 *
 * Description:Check whether two thread descriptions are the same
 *
 *
 ------------------------------------------------------------------------*/
BOOL rpcu_os_thd_desc_eqaul(const RPCU_THREAD_DESC_T * pt_thd_desc1,
                             const RPCU_THREAD_DESC_T * pt_thd_desc2)
{
    if(memcmp(pt_thd_desc1, pt_thd_desc2, sizeof(RPCU_THREAD_DESC_T)) == 0)
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

/*------------------------------------------------------------------------
 * Name: rpcu_os_thd_desc_cpy()
 *
 * Description: Copy one thread description to other.
 *
 *
 ------------------------------------------------------------------------*/
INT32 rpcu_os_thd_desc_cpy(RPCU_THREAD_DESC_T * pt_thd_desc_dest,
                           const RPCU_THREAD_DESC_T * pt_thd_desc_src)
{
    memcpy(pt_thd_desc_dest, pt_thd_desc_src, sizeof(RPCU_THREAD_DESC_T));
    return IPCR_OSERR;
}
/*------------------------------------------------------------------------
 * Name:
 *
 * Description: Create semaphore
 *
 * Inputs:
 * Outputs: -
 * Returns: -
 -----------------------------------------------------------------------*/
INT32 rpcu_os_sem_create(INT32 i4_value, VOID ** ppv_sem)
{
    sem_t * pt_sem;

    pt_sem = RPC_MALLOC(sizeof(sem_t));
    sem_init(pt_sem, 0, i4_value);

    *ppv_sem = pt_sem;

    return IPCR_OK;
}

/*------------------------------------------------------------------------
 * Name:
 *
 * Description: Delete semaphore.
 *
 * Inputs:
 * Outputs: -
 * Returns: -
 -----------------------------------------------------------------------*/
VOID rpcu_os_sem_delete(VOID * pv_sem)
{
    sem_t * pt_sem;

    pt_sem = (sem_t *)pv_sem;

    sem_destroy(pt_sem);

    RPC_FREE(pt_sem);
}

/*------------------------------------------------------------------------
 * Name:
 *
 * Description: Get real time to timespec structure.  If ui4_timeout is
 * 0xffffffff, it will return IPCR_TIMEOUT.
 *
 * Inputs:
 * Outputs: -
 * Returns: -
 -----------------------------------------------------------------------*/
static INT32 _get_rtime(UINT32 ui4_timeout, struct timespec * pt_tm)
{
    if(ui4_timeout != 0xffffffff)
    {
        if(clock_gettime(CLOCK_MONOTONIC, pt_tm) == -1)
        {
            RPC_ERR("clock_gettime err\n");
            return IPCR_OSERR;
        }
        pt_tm->tv_sec  += ui4_timeout/1000;
        pt_tm->tv_nsec += ui4_timeout - pt_tm->tv_sec * 1000;
        return IPCR_TIMEOUT;
    }
    return IPCR_OK;
}

/*------------------------------------------------------------------------
 * Name:
 *
 * Description: Lock semaphore.
 *
 * Inputs:
 * Outputs: -
 * Returns: -
 -----------------------------------------------------------------------*/
INT32 rpcu_os_sem_lock(VOID * pv_sem)
{
    sem_t * pt_sem;
    INT32   i4_ret;

    IPC_ASSERT(pv_sem != NULL);

    pt_sem = (sem_t *)pv_sem;
    do
    {
        i4_ret = sem_wait(pt_sem);
        if(i4_ret < 0 && errno != EINTR)
        {
            return IPCR_OSERR;
        }
    }while(i4_ret < 0 /*&& errno == EINTR*/);

    return IPCR_OK;
}

/*------------------------------------------------------------------------
 * Name:
 *
 * Description: Lock semaphore with timeout.
 *
 * Inputs:
 * Outputs: -
 * Returns: -
 -----------------------------------------------------------------------*/
INT32 rpcu_os_sem_lock_timeout(VOID * pv_sem, UINT32 ui4_timeout)
{
    /* TODO: add timeout */
    sem_t * pt_sem;
    struct timespec t_tm;
    INT32   i4_ret;

    IPC_ASSERT(pv_sem != NULL);

    pt_sem = (sem_t *)pv_sem;

    i4_ret = _get_rtime(ui4_timeout, &t_tm);

    if(i4_ret == IPCR_TIMEOUT)
    {
        do{
            i4_ret = sem_timedwait(pt_sem, &t_tm);
            if(i4_ret < 0)
            {
                if(errno == ETIMEDOUT)
                {
                    return IPCR_TIMEOUT;
                }
                else if(errno != EINTR)
                {
                    return IPCR_OSERR;
                }
            }
        }while(i4_ret < 0 /*&& errno == EINTR*/);
    }
    else if(i4_ret != IPCR_OSERR)
    {
        do
        {
            i4_ret = sem_wait(pt_sem);
            if(i4_ret < 0 && errno != EINTR)
            {
                return IPCR_OSERR;
            }
        }while(i4_ret < 0 /*&& errno == EINTR*/);
    }
    else
    {
        return i4_ret;
    }
    return IPCR_OK;
}

/*------------------------------------------------------------------------
 * Name:
 *
 * Description: Unlock semaphore.
 *
 * Inputs:
 * Outputs: -
 * Returns: -
 -----------------------------------------------------------------------*/
INT32 rpcu_os_sem_unlock(VOID * pv_sem)
{
    sem_t * pt_sem;
    pt_sem = (sem_t *)pv_sem;
    sem_post(pt_sem);

    return IPCR_OK;
}


/*------------------------------------------------------------------------
 * Name:
 *
 * Description: Create mutex.
 *
 * Inputs:
 * Outputs: -
 * Returns: -
 -----------------------------------------------------------------------*/
INT32 rpcu_os_mutex_create(VOID ** ppv_mtx)
{
    pthread_mutexattr_t mtx_attr;
    pthread_mutex_t     * pt_mtx;

    pt_mtx = RPC_MALLOC(sizeof(pthread_mutex_t));
    if(pt_mtx == NULL)
    {
        return IPCR_OUTOFMEMORY;
    }
    pthread_mutexattr_init(&mtx_attr);
    pthread_mutexattr_settype(&mtx_attr, PTHREAD_MUTEX_RECURSIVE);
    pthread_mutex_init(pt_mtx, &mtx_attr);
    pthread_mutexattr_destroy(&mtx_attr);

    *ppv_mtx = pt_mtx;

    return IPCR_OK;
}
/*------------------------------------------------------------------------
 * Name:
 *
 * Description: Delete mutex.
 *
 * Inputs:
 * Outputs: -
 * Returns: -
 -----------------------------------------------------------------------*/
VOID rpcu_os_mutex_delete(VOID * pv_mtx)
{
    pthread_mutex_t     * pt_mtx;

    pt_mtx = pv_mtx;

    pthread_mutex_destroy(pt_mtx);

    RPC_FREE(pt_mtx);

}

/*------------------------------------------------------------------------
 * Name:
 *
 * Description: Lock mutex.
 *
 * Inputs:
 * Outputs: -
 * Returns: -
 -----------------------------------------------------------------------*/
INT32 rpcu_os_mutex_lock(VOID * pv_mtx)
{
    pthread_mutex_lock(pv_mtx);

    return IPCR_OK;
}

/*------------------------------------------------------------------------
 * Name:
 *
 * Description: Try to lock mutex without block.
 *
 * Inputs:
 * Outputs: -
 * Returns: -
 -----------------------------------------------------------------------*/
 INT32 rpcu_os_mutex_trylock(VOID * pv_mtx)
{
	return pthread_mutex_trylock(pv_mtx);
}

/*------------------------------------------------------------------------
 * Name:
 *
 * Description: Lock mutex with timeout.
 *
 * Inputs:
 * Outputs: -
 * Returns: -
 -----------------------------------------------------------------------*/
INT32 rpcu_os_mutex_lock_timeout(VOID * pv_mtx, UINT32 ui4_timeout)
{
    pthread_mutex_t *   pt_mtx;
    struct timespec     t_tm;
    INT32               i4_ret;

    IPC_ASSERT(pv_mtx != NULL);

    pt_mtx = pv_mtx;

    i4_ret = _get_rtime(ui4_timeout, &t_tm);

    if(i4_ret == IPCR_TIMEOUT)
    {
        i4_ret = pthread_mutex_timedlock(pt_mtx, &t_tm);
        if(i4_ret < 0)
        {
            if(errno == ETIMEDOUT)
            {
                return IPCR_TIMEOUT;
            }
            else if(errno != EINTR)
            {
                return IPCR_OSERR;
            }
        }
    }
    else if(i4_ret != IPCR_OSERR)
    {
        i4_ret = pthread_mutex_lock(pt_mtx);
        if(i4_ret < 0)
        {
            return IPCR_OSERR;
        }
    }
    else
    {
        return i4_ret;
    }

    return IPCR_OK;
}

/*------------------------------------------------------------------------
 * Name:
 *
 * Description: Unlock mutex.
 *
 * Inputs:
 * Outputs: -
 * Returns: -
 -----------------------------------------------------------------------*/
INT32 rpcu_os_mutex_unlock(VOID * pv_mtx)
{
    pthread_mutex_unlock(pv_mtx);

    return IPCR_OK;
}



/*------------------------------------------------------------------------
 * Name:
 *
 * Description: Get current thread id.
 *
 * Inputs:
 * Outputs: -
 * Returns: -
 -----------------------------------------------------------------------*/
VOID * rpcu_os_thread_self()
{
    return (VOID *)pthread_self();
}


/*------------------------------------------------------------------------
 * Name:
 *
 * Description: Create Thread Local Storage key.
 *
 * Inputs:
 * Outputs: -
 * Returns: -
 -----------------------------------------------------------------------*/
VOID * rpcu_os_new_tls_key(VOID (* pf_dtor)(VOID *))
{
    pthread_key_t * pt_key;

    pt_key = RPC_MALLOC(sizeof(pthread_key_t));
    if(pt_key == NULL)
    {
        printf("RPC_MALLOC fail\n");

        return NULL;
    }

    if(pthread_key_create(pt_key, pf_dtor) != 0)
    {
        printf("pthread_key_create fail\n");

        RPC_FREE(pt_key);

        return NULL;
    }

    return (VOID *)pt_key;
}

/*------------------------------------------------------------------------
 * Name:
 *
 * Description: Delete TLS key.
 *
 * Inputs:
 * Outputs: -
 * Returns: -
 -----------------------------------------------------------------------*/
VOID rpcu_os_delete_tls_key(VOID * pv_key)
{
    if(pv_key != NULL)
    {
        pthread_key_delete(*(pthread_key_t *)pv_key);
        RPC_FREE(pv_key);
    }
}

/*------------------------------------------------------------------------
 * Name:
 *
 * Description: Get value from TLS key for current thread.
 *
 * Inputs:
 * Outputs: -
 * Returns: -
 -----------------------------------------------------------------------*/
VOID * rpcu_os_tls_get(VOID * pv_key)
{
    if(pv_key == NULL) return (VOID *)IPCR_INV_ARGS;
    return pthread_getspecific(*(pthread_key_t *)pv_key);
}

/*------------------------------------------------------------------------
 * Name:
 *
 * Description: Set value to TLS key for current thread.
 *
 * Inputs:
 * Outputs: -
 * Returns: -
 -----------------------------------------------------------------------*/
INT32 rpcu_os_tls_set(VOID * pv_key, VOID * pv_val)
{
    pthread_setspecific(*(pthread_key_t *)pv_key, pv_val);
    return IPCR_OK;
}



static VOID _logger_clean(VOID * pv_key)
{
    free(pv_key);
}


INT32 rpcu_init_tls_logger()
{
    //mtk40074 add "if(pv_tls_log_key == NULL)"
    if(pv_tls_log_key == NULL)
    {
        pv_tls_log_key = rpcu_os_new_tls_key(_logger_clean);
    }
    return 0;
}

INT32 rpcu_push_logger(rpc_log_fct pf_log_fct, INT32 i4_level)
{
    RPC_TL_LOGGER_T * pt_log_stack;

	/*if(pv_tls_log_key == NULL) return IPCR_INV_ARGS;*/
    if((pt_log_stack = rpcu_os_tls_get(pv_tls_log_key)) == (void *)IPCR_INV_ARGS)
        return IPCR_INV_ARGS;
    if(pt_log_stack == NULL)
    {
        pt_log_stack = malloc(sizeof(RPC_TL_LOGGER_T));
        if (pt_log_stack == NULL)
        {
            IPC_ASSERT(0);
            return 0;
        }
        pt_log_stack->i4_cur = -1;
        rpcu_os_tls_set(pv_tls_log_key, pt_log_stack);
    }

    pt_log_stack->i4_cur ++;
    pt_log_stack->pf_log_fct[pt_log_stack->i4_cur] = pf_log_fct;
    pt_log_stack->i4_level[pt_log_stack->i4_cur] = i4_level;
    return 0;
}

static VOID _default_log(INT32 i4_level, char * ps_string)
{
    printf("<DEFAULT LOG>%s", ps_string);
}
INT32 rpcu_push_default_logger()
{
    return rpcu_push_logger(_default_log, RPC_LOG_ALL);
}


INT32 rpcu_pop_logger( )
{
    RPC_TL_LOGGER_T * pt_log_stack;
	if((pt_log_stack = rpcu_os_tls_get(pv_tls_log_key)) == (void *)IPCR_INV_ARGS)
		return IPCR_INV_ARGS;
    if(pt_log_stack == NULL)
    {
       return -1;
    }

    pt_log_stack->i4_cur --;
	return 0;
}

/*------------------------------------------------------------------------
 * Name:
 *
 * Description: Get top logger on the stack
 *
 * Inputs:
 * Outputs: -
 * Returns: -
 -----------------------------------------------------------------------*/
INT32 rpcu_logger_stack_get(rpc_log_fct * ppf_log_fct, INT32 * pi4_level)
{
    RPC_TL_LOGGER_T * pt_log_stack;

    if((pt_log_stack = rpcu_os_tls_get(pv_tls_log_key)) == NULL)
    {
        return -1;
    }
    if(pt_log_stack->i4_cur < 0)
    {
        return -1;
    }

    *ppf_log_fct = pt_log_stack->pf_log_fct[pt_log_stack->i4_cur];
    *pi4_level = pt_log_stack->i4_level[pt_log_stack->i4_cur];
    return RPCR_OK;
}

/*------------------------------------------------------------------------
 * Name:
 *
 * Description: Set top logger on the stack
 *
 * Inputs:
 * Outputs: -
 * Returns: -
 -----------------------------------------------------------------------*/
INT32 rpcu_logger_stack_set(rpc_log_fct pf_log_fct, INT32 i4_level)
{
    RPC_TL_LOGGER_T * pt_log_stack;

    if((pt_log_stack = rpcu_os_tls_get(pv_tls_log_key)) == NULL)
    {
        return -1;
    }
    if(pt_log_stack->i4_cur < 0)
    {
        return -1;
    }

    pt_log_stack->pf_log_fct[pt_log_stack->i4_cur] = pf_log_fct;
    pt_log_stack->i4_level[pt_log_stack->i4_cur] = i4_level;
    return RPCR_OK;
}

/*------------------------------------------------------------------------
 * Name:
 *
 * Description: This will call stack top callback log function
 *
 * Inputs:
 * Outputs: -
 * Returns: -
 -----------------------------------------------------------------------*/
EXPORT_SYMBOL INT32 rpcu_log(INT32 i4_level, CHAR * ps_fmt, ...)
{
    RPC_TL_LOGGER_T * pt_log_stack;
    CHAR s_buff[255 + 1];

    va_list t_vl;

    if((pt_log_stack = rpcu_os_tls_get(pv_tls_log_key)) == NULL)
    {
        return -1;
    }
    if(pt_log_stack  == (void *)IPCR_INV_ARGS)
    {
        return IPCR_INV_ARGS;
    }
    if(pt_log_stack->i4_cur < 0)
    {
        return -1;
    }

    memset(&t_vl, 0, sizeof(va_list));
    va_start(t_vl, ps_fmt);

    if((pt_log_stack->i4_level[pt_log_stack->i4_cur] & i4_level) && pt_log_stack->pf_log_fct[pt_log_stack->i4_cur])
    {
        vsnprintf(s_buff, 255, ps_fmt, t_vl);
        pt_log_stack->pf_log_fct[pt_log_stack->i4_cur](i4_level, s_buff);
    }

    va_end(t_vl);

    return RPCR_OK;
}


