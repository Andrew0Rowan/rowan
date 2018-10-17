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
#include <pthread.h>
#include <stdlib.h>
#include <time.h>
#include "u_os.h"
#include "u_assert.h"
#include "timerd.h"
#include "u_appman.h"
#include "u_timerd.h"


#define time_before(a, b) ((a).tv_sec < (b).tv_sec || ((a).tv_sec == (b).tv_sec && (a).tv_nsec < (b).tv_nsec))

typedef struct os_timer_light
{
    struct os_timer_light *previous;
    struct os_timer_light *next;
    struct timespec abstime;
    struct timespec interval;
    u_os_timer_cb_fct pf_callback;
    struct _TIMER_CBFCT_ARGS *pt_args; 	//@ pt_args will allocate in u_timer_start, and release when time_list_remove.
} OS_TIMER_LIGHT_T;


static pthread_t s_timer_thread;
static BOOL s_timer_quit;
static pthread_cond_t s_timer_list_cond;
static pthread_mutex_t s_timer_list_mutex;
static OS_TIMER_LIGHT_T *s_timer_list;
static UINT32 s_timer_res;


static void timer_list_add(OS_TIMER_LIGHT_T *pt_timer)
{
    if (s_timer_list != NULL)
    {
        OS_TIMER_LIGHT_T *p = s_timer_list;
        if (time_before(pt_timer->abstime, p->abstime))
        {
            s_timer_list = pt_timer;
        }
        else
        {
            for (p = p->next; p != s_timer_list; p = p->next)
                if (time_before(pt_timer->abstime, p->abstime))
                    break;
        }
        pt_timer->previous = p->previous;
        pt_timer->next = p;
        p->previous->next = pt_timer;
        p->previous = pt_timer;
    }
    else
    {
        s_timer_list = pt_timer->next = pt_timer->previous = pt_timer;
    }
}


static void timer_list_remove(OS_TIMER_LIGHT_T *pt_timer)
{
	if (pt_timer->interval.tv_sec == 0 && pt_timer->interval.tv_nsec == 0)
	{
		//@ pt_args will allocate in u_timer_start
		if(pt_timer->pt_args!=NULL)
		{
			FREE_SAFE(pt_timer->pt_args->ptimer_msg);
			FREE_SAFE(pt_timer->pt_args);
		}
	}

    if (pt_timer->previous == pt_timer)
    {
        s_timer_list = NULL;
    }
    else
    {
        pt_timer->previous->next = pt_timer->next;
        pt_timer->next->previous = pt_timer->previous;
        if (s_timer_list == pt_timer)
        {
            s_timer_list = pt_timer->next;
        }
    }
    pt_timer->previous = pt_timer->next = NULL;
}


static void *TimerProc(void *arg)
{
    OS_TIMER_LIGHT_T *pt_timer;
    struct timespec abstime;
    int ret;
    struct _TIMER_CBFCT_ARGS *pargs = NULL;

    VERIFY(pthread_mutex_lock(&s_timer_list_mutex) == 0);
    while (!s_timer_quit)
    {
        while (s_timer_list == NULL)
        {
            ret = pthread_cond_wait(&s_timer_list_cond, &s_timer_list_mutex);
            VERIFY(ret == 0);
        }

        VERIFY(clock_gettime(CLOCK_MONOTONIC, &abstime) == 0);

        while (s_timer_list && time_before(s_timer_list->abstime, abstime))
        {
            pt_timer = s_timer_list;

            if ((pt_timer->pf_callback) &&
                    (pt_timer->interval.tv_sec == 0 && pt_timer->interval.tv_nsec == 0) &&
                    (pt_timer->pt_args != NULL)) {
                pargs = malloc(sizeof(struct _TIMER_CBFCT_ARGS));
                VERIFY(pargs);

                pargs->h_app = pt_timer->pt_args->h_app;
                if(pt_timer->pt_args->ptimer_msg != NULL) {
                    pargs->ptimer_msg = malloc(pt_timer->pt_args->msg_size);
                    VERIFY(pargs->ptimer_msg);
                    pargs->msg_size = pt_timer->pt_args->msg_size;
                    memcpy(pargs->ptimer_msg, pt_timer->pt_args->ptimer_msg, pt_timer->pt_args->msg_size);
                } else {
                    pargs->ptimer_msg = NULL;
                    pargs->msg_size = 0;
                }
            }
            timer_list_remove(pt_timer);

            if (pt_timer->interval.tv_sec != 0 || pt_timer->interval.tv_nsec != 0)
            {
                pt_timer->abstime.tv_sec += pt_timer->interval.tv_sec;
                pt_timer->abstime.tv_nsec += pt_timer->interval.tv_nsec;
                if (pt_timer->abstime.tv_nsec >= 1000000000)
                {
                    pt_timer->abstime.tv_nsec -= 1000000000;
                    pt_timer->abstime.tv_sec++;
                }
                timer_list_add(pt_timer);
            }
            VERIFY(pthread_mutex_unlock(&s_timer_list_mutex) == 0);
            if(pt_timer->pf_callback) {
                if (pt_timer->interval.tv_sec == 0 && pt_timer->interval.tv_nsec == 0) {
                    pt_timer->pf_callback((HANDLE_T)(pt_timer), pargs);
                    FREE_SAFE(pargs->ptimer_msg);
                    FREE_SAFE(pargs);
                } else {
                    pt_timer->pf_callback((HANDLE_T)(pt_timer), pt_timer->pt_args);
                }
            }
            pargs = NULL;
            VERIFY(pthread_mutex_lock(&s_timer_list_mutex) == 0);
        }
        if (s_timer_list != NULL)
        {
            ret = pthread_cond_timedwait(&s_timer_list_cond, &s_timer_list_mutex, &s_timer_list->abstime);
            VERIFY(ret == 0 || ret == ETIMEDOUT);
        }
    }
    VERIFY(pthread_mutex_unlock(&s_timer_list_mutex) == 0);
    return NULL;
}


static INT32 _timer_start (HANDLE_T           h_timer,
                     UINT32             ui4_delay,
                     TIMER_FLAG_E      e_flags,
                     u_os_timer_cb_fct  pf_callback,
                     struct _TIMER_CBFCT_ARGS *            pt_args)
{
    OS_TIMER_LIGHT_T *pt_timer;
    struct timespec delay;
    if ((pf_callback == NULL) || (ui4_delay == 0) ||
        ((e_flags != X_TIMER_FLAG_ONCE) && (e_flags != X_TIMER_FLAG_REPEAT)))
    {
        return OSR_INV_ARG;
    }
    pt_timer = (OS_TIMER_LIGHT_T *)(h_timer);
    delay.tv_sec = ui4_delay / 1000;
    delay.tv_nsec = (ui4_delay % 1000) * 1000000;

    VERIFY(pthread_mutex_lock(&s_timer_list_mutex) == 0);
	//@ if pt_timer has been start before,  remove formal one.
    if (pt_timer->previous != NULL)
    {
        timer_list_remove(pt_timer);
    }

    VERIFY(clock_gettime(CLOCK_MONOTONIC, &pt_timer->abstime) == 0);
    pt_timer->abstime.tv_sec += delay.tv_sec;
    pt_timer->abstime.tv_nsec += delay.tv_nsec;
    if (pt_timer->abstime.tv_nsec >= 1000000000)
    {
        pt_timer->abstime.tv_nsec -= 1000000000;
        pt_timer->abstime.tv_sec++;
    }
    if (e_flags == X_TIMER_FLAG_ONCE)
    {
        pt_timer->interval.tv_sec = 0;
        pt_timer->interval.tv_nsec = 0;
    }
    else
    {
        pt_timer->interval.tv_sec = delay.tv_sec;
        pt_timer->interval.tv_nsec = delay.tv_nsec;
    }

    pt_timer->pf_callback = pf_callback;
    pt_timer->pt_args = pt_args;

    timer_list_add(pt_timer);
    VERIFY(pthread_cond_signal(&s_timer_list_cond) == 0);
    VERIFY(pthread_mutex_unlock(&s_timer_list_mutex) == 0);
    return OSR_OK;
}

VOID _os_timer_cb_fct(HANDLE_T  pt_tm_handle, struct _TIMER_CBFCT_ARGS *   pt_msg)
{
    if (pt_msg)
        u_app_send_msg(pt_msg->h_app,E_APP_MSG_TYPE_TIMER,pt_msg->ptimer_msg,pt_msg->msg_size,NULL,NULL);
}

INT32 u_timer_start( HANDLE_T h_app, TIMER_TYPE_T *p_timer, void * p_msg,size_t msg_size)
{
	struct _TIMER_CBFCT_ARGS *pargs = malloc(sizeof(struct _TIMER_CBFCT_ARGS));
	if(pargs==NULL) return (-1);

	pargs->h_app = h_app;
	if(p_msg!=NULL)
	{
		pargs->ptimer_msg = malloc(msg_size);
		if(pargs->ptimer_msg==NULL) return (-1);
		pargs->msg_size = msg_size;
		memcpy(pargs->ptimer_msg , p_msg , msg_size);
	}else{
		pargs->ptimer_msg = NULL;
		pargs->msg_size = 0;
	}

	return _timer_start (p_timer->h_timer,p_timer->ui4_delay,p_timer->e_flags,_os_timer_cb_fct,pargs);

}
INT32 u_timer_stop (HANDLE_T  h_timer)
{
    OS_TIMER_LIGHT_T *pt_timer;
    struct timespec abstime;

    pt_timer = (OS_TIMER_LIGHT_T *)(h_timer);

    VERIFY(pthread_mutex_lock(&s_timer_list_mutex) == 0);
    if (pt_timer->previous != NULL)
    {
        timer_list_remove(pt_timer);
        VERIFY(pthread_cond_signal(&s_timer_list_cond) == 0);
        VERIFY(pthread_mutex_unlock(&s_timer_list_mutex) == 0);
        VERIFY(clock_gettime(CLOCK_MONOTONIC, &abstime) == 0);
        pt_timer->abstime.tv_sec -= abstime.tv_sec;
        if (pt_timer->abstime.tv_nsec < abstime.tv_nsec)
        {
            pt_timer->abstime.tv_nsec += 1000000000;
            pt_timer->abstime.tv_sec--;
        }
        pt_timer->abstime.tv_nsec -= abstime.tv_nsec;
    }
    else
    {
        VERIFY(pthread_mutex_unlock(&s_timer_list_mutex) == 0);
    }
    return OSR_OK;
}


INT32 u_timer_delete (HANDLE_T  h_timer)
{
    OS_TIMER_LIGHT_T *pt_timer;

    pt_timer = (OS_TIMER_LIGHT_T *)(h_timer);

    VERIFY(pthread_mutex_lock(&s_timer_list_mutex) == 0);
    if (pt_timer->previous != NULL)
    {
        timer_list_remove(pt_timer);
        VERIFY(pthread_cond_signal(&s_timer_list_cond) == 0);
    }
    VERIFY(pthread_mutex_unlock(&s_timer_list_mutex) == 0);

    free(pt_timer);
    return OSR_OK;
}


/*Current timer logic do not support call u_timer_resume*/
/*if stop timer will release pt_timer->pt_args which store the msg q handle&msg */
/*then call resume api will crash at timer up call callback function*/
#if 0
INT32 u_timer_resume (HANDLE_T  h_timer)
{
    OS_TIMER_LIGHT_T *pt_timer;
    struct timespec abstime;

    pt_timer = (OS_TIMER_LIGHT_T *)(h_timer);

    VERIFY(pthread_mutex_lock(&s_timer_list_mutex) == 0);
    if (pt_timer->previous != NULL)
    {
        VERIFY(pthread_mutex_unlock(&s_timer_list_mutex) == 0);
        return OSR_FAIL;
    }
    VERIFY(pthread_mutex_unlock(&s_timer_list_mutex) == 0);
    VERIFY(clock_gettime(CLOCK_MONOTONIC, &abstime) == 0);
    pt_timer->abstime.tv_sec  += abstime.tv_sec;
    pt_timer->abstime.tv_nsec += abstime.tv_nsec;
    if (pt_timer->abstime.tv_nsec >= 1000000000)
    {
        pt_timer->abstime.tv_nsec -= 1000000000;
        pt_timer->abstime.tv_sec++;
    }
    VERIFY(pthread_mutex_lock(&s_timer_list_mutex) == 0);
    timer_list_add(pt_timer);
    VERIFY(pthread_cond_signal(&s_timer_list_cond) == 0);
    VERIFY(pthread_mutex_unlock(&s_timer_list_mutex) == 0);

    return OSR_OK;
}
#endif

INT32 u_timer_create (HANDLE_T *ph_timer)
{
    OS_TIMER_LIGHT_T *pt_timer;

    if (ph_timer == NULL)
    {
        return OSR_INV_ARG;
    }

    pt_timer = calloc(1, sizeof(OS_TIMER_LIGHT_T));
    if (pt_timer == NULL)
    {
        return OSR_NO_RESOURCE;
    }
    //FILL_CALLER(pt_timer);

    *ph_timer = (HANDLE_T)(pt_timer);
    return OSR_OK;
}


UINT32 u_os_get_sys_tick (VOID)
{
    struct timespec abstime;
    VERIFY(clock_gettime(CLOCK_MONOTONIC, &abstime) == 0);
    return abstime.tv_sec * (1000 / s_timer_res) + abstime.tv_nsec / (1000000 * s_timer_res);
}


UINT32 u_os_get_sys_tick_period (VOID)
{
    return s_timer_res;
}

INT32 os_timer_init (VOID)
{
    struct timespec res;
    pthread_condattr_t condattr;
    pthread_attr_t     attr;
    struct sched_param param;

    VERIFY(clock_getres(CLOCK_MONOTONIC, &res) == 0);
    s_timer_res = (res.tv_nsec + 500000) / 1000000;

    VERIFY(pthread_condattr_init(&condattr) == 0);
    VERIFY(pthread_condattr_setclock(&condattr, CLOCK_MONOTONIC) == 0);
    VERIFY(pthread_cond_init(&s_timer_list_cond, &condattr) == 0);
    VERIFY(pthread_mutex_init(&s_timer_list_mutex, NULL) == 0);

    VERIFY(pthread_attr_init(&attr) == 0);
#if !CLI_SUPPORT	
    VERIFY(pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED) == 0);
    VERIFY(pthread_attr_setschedpolicy(&attr, SCHED_FIFO) == 0);
    param.sched_priority = sched_get_priority_max(SCHED_FIFO);
    VERIFY(pthread_attr_setschedparam(&attr, &param) == 0);
#endif
    VERIFY(pthread_create(&s_timer_thread, &attr, TimerProc, NULL) == 0);
    return OSR_OK;
}

