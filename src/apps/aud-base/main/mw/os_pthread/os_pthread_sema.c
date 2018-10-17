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

#include "u_assert.h"
#include "u_os.h"
#include "u_dbg.h"
#include "u_8516.h"

#define INVALID_PTHREAD_T ((pthread_t)(-1))


typedef struct os_sema_light
{
    SEMA_TYPE_T e_type;
    pthread_cond_t cond;
    pthread_mutex_t mutex;
    pthread_t thread;
    UINT32 ui4_value;
    INT16 i2_selfcount;
} OS_SEMA_LIGHT_T;


INT32 u_sema_create (HANDLE_T*    ph_sema_hdl,
                     SEMA_TYPE_T  e_type,
                     UINT32       ui4_init_value)
{
    OS_SEMA_LIGHT_T *pt_sema;
    pthread_condattr_t condattr;

    /* check arguments */
    if ((ph_sema_hdl == NULL) ||
        ((e_type != X_SEMA_TYPE_BINARY) && (e_type != X_SEMA_TYPE_MUTEX) &&
         (e_type != X_SEMA_TYPE_COUNTING)))
    {
        return OSR_INV_ARG;
    }

    if (((e_type == X_SEMA_TYPE_BINARY) || (e_type == X_SEMA_TYPE_MUTEX)) &&
         (ui4_init_value != X_SEMA_STATE_LOCK) &&
         (ui4_init_value != X_SEMA_STATE_UNLOCK))
    {
        return OSR_INV_ARG;
    }

    if ((e_type == X_SEMA_TYPE_COUNTING) &&
        (((INT32) ui4_init_value) < ((INT32) X_SEMA_STATE_LOCK)))
    {
        return OSR_INV_ARG;
    }

    pt_sema = calloc(1, sizeof(OS_SEMA_LIGHT_T));
    if (pt_sema == NULL)
    {
        return OSR_NO_RESOURCE;
    }
    //FILL_CALLER(pt_sema);

    pt_sema->e_type = e_type;
    VERIFY(pthread_condattr_init(&condattr) == 0);
    VERIFY(pthread_condattr_setclock(&condattr, CLOCK_MONOTONIC) == 0);
    if (pthread_cond_init(&pt_sema->cond, &condattr) != 0 ||
        pthread_mutex_init(&pt_sema->mutex, NULL) != 0)
    {
        free(pt_sema);
        return OSR_FAIL;
    }
    pt_sema->ui4_value = ui4_init_value;

    if (e_type == X_SEMA_TYPE_MUTEX)
    {
        if (ui4_init_value == X_SEMA_STATE_LOCK)
        {
            pt_sema->thread = pthread_self();
            pt_sema->i2_selfcount++;
        }
        else
        {
            pt_sema->thread = INVALID_PTHREAD_T;
            pt_sema->i2_selfcount = 0;
        }
    }else
    {
    	pt_sema->thread = INVALID_PTHREAD_T;
    }

    *ph_sema_hdl = (HANDLE_T)(pt_sema);
    return OSR_OK;
}


INT32 u_sema_delete (HANDLE_T  h_sema_hdl)
{
    OS_SEMA_LIGHT_T *pt_sema = (OS_SEMA_LIGHT_T *)(h_sema_hdl);
    if (pthread_mutex_destroy(&pt_sema->mutex) != 0 ||
        pthread_cond_destroy(&pt_sema->cond) != 0)
    {
        return OSR_FAIL;
    }

    free(pt_sema);
    return OSR_OK;
}


INT32 u_sema_lock (HANDLE_T       h_sema_hdl,
                   SEMA_OPTION_T  e_option)
{
    OS_SEMA_LIGHT_T *pt_sema = (OS_SEMA_LIGHT_T *)(h_sema_hdl);
    int ret;

    pthread_t thread = pthread_self();
    if (e_option != X_SEMA_OPTION_WAIT && e_option != X_SEMA_OPTION_NOWAIT)
    {
        return OSR_INV_ARG;
    }

    if (pt_sema->e_type == X_SEMA_TYPE_MUTEX)
    {
        if (!pthread_equal(pt_sema->thread, thread))
        {
            ret = pthread_mutex_lock(&pt_sema->mutex);
            if (ret != 0)
            {
                goto err;
            }
            while (pt_sema->ui4_value == 0)
            {
                if (e_option == X_SEMA_OPTION_NOWAIT)
                {
                    VERIFY(pthread_mutex_unlock(&pt_sema->mutex) == 0);
                    return OSR_WOULD_BLOCK;
                }
                ret = pthread_cond_wait(&pt_sema->cond, &pt_sema->mutex);
                if (ret != 0)
                {
                    VERIFY(pthread_mutex_unlock(&pt_sema->mutex) == 0);
                    goto err;
                }
            }
            pt_sema->ui4_value--;
            VERIFY(pthread_mutex_unlock(&pt_sema->mutex) == 0);
            pt_sema->thread = thread;
        }
        pt_sema->i2_selfcount++;
        return OSR_OK;
    }
    else
    {
        ret = pthread_mutex_lock(&pt_sema->mutex);
        if (ret != 0)
        {
            goto err;
        }
        while (pt_sema->ui4_value == 0)
        {
            if (e_option == X_SEMA_OPTION_NOWAIT)
            {
                VERIFY(pthread_mutex_unlock(&pt_sema->mutex) == 0);
                return OSR_WOULD_BLOCK;
            }
            ret = pthread_cond_wait(&pt_sema->cond, &pt_sema->mutex);
            if (ret != 0)
            {
                VERIFY(pthread_mutex_unlock(&pt_sema->mutex) == 0);
                goto err;
            }
        }
        pt_sema->ui4_value--;
        VERIFY(pthread_mutex_unlock(&pt_sema->mutex) == 0);
	pt_sema->thread = thread;
        return OSR_OK;
    }

err:
    switch (ret)
    {
    case EINVAL:
        return OSR_INV_HANDLE;

    default:
        return OSR_FAIL;
    }
}


INT32 u_sema_lock_timeout (HANDLE_T  h_sema_hdl,
                           UINT32    ui4_time)
{
    OS_SEMA_LIGHT_T *pt_sema = (OS_SEMA_LIGHT_T *)(h_sema_hdl);
    struct timespec abstime;
    int ret;

    pthread_t thread = pthread_self();
    if (ui4_time == 0)
    {
        INT32 i4 = u_sema_lock(h_sema_hdl, X_SEMA_OPTION_NOWAIT);
        return i4 != OSR_WOULD_BLOCK ? i4 : OSR_TIMEOUT;
    }

    VERIFY(clock_gettime(CLOCK_MONOTONIC, &abstime) == 0);
    abstime.tv_sec  += ui4_time / 1000; ui4_time %= 1000;
    abstime.tv_nsec += ui4_time * 1000000;
    if (abstime.tv_nsec >= 1000000000)
    {
        abstime.tv_nsec -= 1000000000;
        abstime.tv_sec++;
    }

    if (pt_sema->e_type == X_SEMA_TYPE_MUTEX)
    {
        if (!pthread_equal(pt_sema->thread, thread))
        {
            ret = pthread_mutex_lock(&pt_sema->mutex);
            if (ret != 0)
            {
                goto err;
            }
            while (pt_sema->ui4_value == 0)
            {
                ret = pthread_cond_timedwait(&pt_sema->cond, &pt_sema->mutex, &abstime);
                if (ret != 0)
                {
                    VERIFY(pthread_mutex_unlock(&pt_sema->mutex) == 0);
                    goto err;
                }
            }
            pt_sema->ui4_value--;
            VERIFY(pthread_mutex_unlock(&pt_sema->mutex) == 0);
            pt_sema->thread = thread;
        }
        pt_sema->i2_selfcount++;
        return OSR_OK;
    }
    else
    {
        ret = pthread_mutex_lock(&pt_sema->mutex);
        if (ret != 0)
        {
            goto err;
        }
        while (pt_sema->ui4_value == 0)
        {
            ret = pthread_cond_timedwait(&pt_sema->cond, &pt_sema->mutex, &abstime);
            if (ret != 0)
            {
                VERIFY(pthread_mutex_unlock(&pt_sema->mutex) == 0);
                goto err;
            }
        }
        pt_sema->ui4_value--;
        VERIFY(pthread_mutex_unlock(&pt_sema->mutex) == 0);
	pt_sema->thread = thread;
        return OSR_OK;
    }

err:
    switch (ret)
    {
    case ETIMEDOUT:
        return OSR_TIMEOUT;

    case EINVAL:
        return OSR_INV_HANDLE;

    default:
        return OSR_FAIL;
    }
}


INT32 u_sema_unlock (HANDLE_T  h_sema_hdl)
{
    OS_SEMA_LIGHT_T *pt_sema = (OS_SEMA_LIGHT_T *)(h_sema_hdl);
    int ret;

    if (pt_sema->e_type == X_SEMA_TYPE_MUTEX)
    {
        if (!pthread_equal(pt_sema->thread, pthread_self()))
        {
            return OSR_FAIL;
        }
        pt_sema->i2_selfcount--;
        if (pt_sema->i2_selfcount != 0)
        {
            return OSR_OK;
        }
        pt_sema->thread = INVALID_PTHREAD_T;

        ret = pthread_mutex_lock(&pt_sema->mutex);
        if (ret != 0)
        {
            goto err;
        }
        pt_sema->ui4_value++;
        if (pt_sema->ui4_value == 1)
        {
            VERIFY(pthread_cond_signal(&pt_sema->cond) == 0);
        }
        VERIFY(pthread_mutex_unlock(&pt_sema->mutex) == 0);

        return OSR_OK;
    }
    else
    {
    	pt_sema->thread = INVALID_PTHREAD_T;
        ret = pthread_mutex_lock(&pt_sema->mutex);
        if (ret != 0)
        {
            goto err;
        }
        pt_sema->ui4_value++;
        if (pt_sema->ui4_value == 1)
        {
            VERIFY(pthread_cond_signal(&pt_sema->cond) == 0);
        }
        VERIFY(pthread_mutex_unlock(&pt_sema->mutex) == 0);

        return OSR_OK;
    }

err:
    switch (ret)
    {
    case EINVAL:
        return OSR_INV_HANDLE;

    default:
        return OSR_FAIL;
    }
}


