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
#include <string.h>
#include "u_assert.h"
#include "u_os.h"
#include "u_dbg.h"
#include "u_8516.h"

#define MSGQ_NAME_LEN           16
#define MSGQ_MAX_DATA_SIZE      (4096+512)
#define MSGQ_MAX_MSGS           4095

typedef struct os_msg_q_light
{
    struct os_msg_q_light *previous;
    struct os_msg_q_light *next;
    pthread_cond_t read_cond;
    pthread_cond_t write_cond;
    pthread_mutex_t mutex;
    BYTE *read;
    BYTE *write;
    BYTE *end;
    SIZE_T message_size;
    SIZE_T z_maxsize;
    UINT16 ui2_msg_count;
    INT16 i2_refcount;
    CHAR s_name[MSGQ_NAME_LEN + 1];
    BYTE start[1];
} OS_MSGQ_LIGHT_T;


static pthread_mutex_t s_msg_q_list_mutex;
static OS_MSGQ_LIGHT_T *s_msg_q_list;


static void msg_q_list_add(OS_MSGQ_LIGHT_T *pt_msg_q)
{
    if (s_msg_q_list != NULL)
    {
        pt_msg_q->previous = s_msg_q_list->previous;
        pt_msg_q->next = s_msg_q_list;
        s_msg_q_list->previous->next = pt_msg_q;
        s_msg_q_list->previous = pt_msg_q;
    }
    else
    {
        s_msg_q_list = pt_msg_q->next = pt_msg_q->previous = pt_msg_q;
    }
}


static void msg_q_list_remove(OS_MSGQ_LIGHT_T *pt_msg_q)
{
    if (pt_msg_q->previous == pt_msg_q)
    {
        s_msg_q_list = NULL;
    }
    else
    {
        pt_msg_q->previous->next = pt_msg_q->next;
        pt_msg_q->next->previous = pt_msg_q->previous;
        if (s_msg_q_list == pt_msg_q)
        {
            s_msg_q_list = pt_msg_q->next;
        }
    }
}


static OS_MSGQ_LIGHT_T *msg_q_find_obj(const CHAR *ps_name)
{
    OS_MSGQ_LIGHT_T *pt_msg_q = s_msg_q_list;
    if (pt_msg_q == NULL)
    {
        return NULL;
    }
    do
    {
        if (strncmp(pt_msg_q->s_name, ps_name, MSGQ_NAME_LEN) == 0)
        {
            return pt_msg_q;
        }
        pt_msg_q = pt_msg_q->next;
    } while (pt_msg_q != s_msg_q_list);

    return NULL;
}


INT32
x_msg_q_create(HANDLE_T     *ph_msg_hdl,
               const CHAR   *ps_name,
               SIZE_T       z_msg_size,
               UINT16       ui2_msg_count)
{
    OS_MSGQ_LIGHT_T *pt_msg_q;
    SIZE_T message_size;
    pthread_condattr_t condattr;

    /* check arguments */
    if ((ps_name == NULL) || (ps_name[0] == '\0') || (ph_msg_hdl == NULL) ||
        (z_msg_size > MSGQ_MAX_DATA_SIZE) || (ui2_msg_count > MSGQ_MAX_MSGS) ||
        (z_msg_size == 0) || (ui2_msg_count == 0))
    {
        return OSR_INV_ARG;
    }

    message_size = sizeof(SIZE_T) + ((z_msg_size + 3) & ~3);
    pt_msg_q = calloc(1, sizeof(OS_MSGQ_LIGHT_T) - sizeof(BYTE) + message_size * (ui2_msg_count + 1));
    if (pt_msg_q == NULL)
    {
        return OSR_NO_RESOURCE;
    }

    pt_msg_q->read = pt_msg_q->start;
    pt_msg_q->write = pt_msg_q->start;
    pt_msg_q->end = pt_msg_q->start + message_size * (ui2_msg_count + 1);
    pt_msg_q->message_size = message_size;
    pt_msg_q->z_maxsize = z_msg_size;
    pt_msg_q->ui2_msg_count = ui2_msg_count;
    pt_msg_q->i2_refcount = 1;
    strncpy(pt_msg_q->s_name, ps_name, MSGQ_NAME_LEN);

    VERIFY(pthread_condattr_init(&condattr) == 0);
    VERIFY(pthread_condattr_setclock(&condattr, CLOCK_MONOTONIC) == 0);
    VERIFY(pthread_cond_init(&pt_msg_q->read_cond, &condattr) == 0);
    VERIFY(pthread_cond_init(&pt_msg_q->write_cond, &condattr) == 0);
    VERIFY(pthread_mutex_init(&pt_msg_q->mutex, NULL) == 0);

    VERIFY(pthread_mutex_lock(&s_msg_q_list_mutex) == 0);
    if (msg_q_find_obj(ps_name) != NULL)
    {
        VERIFY(pthread_mutex_unlock(&s_msg_q_list_mutex) == 0);
        free(pt_msg_q);
        return OSR_EXIST;
    }
    msg_q_list_add(pt_msg_q);
    VERIFY(pthread_mutex_unlock(&s_msg_q_list_mutex) == 0);

    *ph_msg_hdl = (HANDLE_T)(pt_msg_q);
    return OSR_OK;
}


INT32
x_msg_q_attach(HANDLE_T     *ph_msg_hdl,
               const CHAR   *ps_name)
{
    OS_MSGQ_LIGHT_T *pt_msg_q;

    /* arguments check */
    if ((ps_name == NULL) || (ps_name[0] == '\0') ||
        (ph_msg_hdl == NULL_HANDLE))
    {
        return OSR_INV_ARG;
    }

    VERIFY(pthread_mutex_lock(&s_msg_q_list_mutex) == 0);
    pt_msg_q = msg_q_find_obj(ps_name);
    if (pt_msg_q == NULL)
    {
        VERIFY(pthread_mutex_unlock(&s_msg_q_list_mutex) == 0);
        return OSR_NOT_EXIST;
    }
    pt_msg_q->i2_refcount++;
    VERIFY(pthread_mutex_unlock(&s_msg_q_list_mutex) == 0);

    *ph_msg_hdl = (HANDLE_T)(pt_msg_q);

    return OSR_OK;
}


INT32
x_msg_q_delete(HANDLE_T h_msg_hdl)
{
    OS_MSGQ_LIGHT_T *pt_msg_q;
    INT16 i2_refcount;

    pt_msg_q = (OS_MSGQ_LIGHT_T *)(h_msg_hdl);

    VERIFY(pthread_mutex_lock(&s_msg_q_list_mutex) == 0);
    i2_refcount = --pt_msg_q->i2_refcount;
    if (i2_refcount > 0)
    {
        VERIFY(pthread_mutex_unlock(&s_msg_q_list_mutex) == 0);
        return OSR_OK;
    }
    msg_q_list_remove(pt_msg_q);
    VERIFY(pthread_mutex_unlock(&s_msg_q_list_mutex) == 0);

    VERIFY(pthread_mutex_destroy(&pt_msg_q->mutex) == 0);
    VERIFY(pthread_cond_destroy(&pt_msg_q->write_cond) == 0);
    VERIFY(pthread_cond_destroy(&pt_msg_q->read_cond) == 0);

    free(pt_msg_q);
    return OSR_OK;
}


INT32
x_msg_q_send(HANDLE_T   h_msg_hdl,
             const VOID *pv_msg,
             SIZE_T     z_size,
             UINT8      ui1_pri)
{
    OS_MSGQ_LIGHT_T *pt_msg_q;
    BYTE *write;
    int ret;

    if ((pv_msg == NULL) || (z_size == 0))
    {
        return OSR_INV_ARG;
    }

    pt_msg_q = (OS_MSGQ_LIGHT_T *)(h_msg_hdl);

    if (z_size > pt_msg_q->z_maxsize)
    {
        return OSR_TOO_BIG;
    }
    ret = pthread_mutex_lock(&pt_msg_q->mutex);
    if (ret != 0)
    {
        goto err;
    }
    write = pt_msg_q->write + pt_msg_q->message_size;
    if (write == pt_msg_q->end)
    {
        write = pt_msg_q->start;
    }
    if (write == pt_msg_q->read)
    {
        VERIFY(pthread_mutex_unlock(&pt_msg_q->mutex) == 0);
        return OSR_TOO_MANY;
    }
    *(SIZE_T *)(pt_msg_q->write) = z_size;
    memcpy(pt_msg_q->write + 4, pv_msg, z_size);
    pt_msg_q->write = write;
    VERIFY(pthread_cond_broadcast(&pt_msg_q->write_cond) == 0);
    VERIFY(pthread_mutex_unlock(&pt_msg_q->mutex) == 0);
    return OSR_OK;

err:
    switch (ret)
    {
    case EINVAL:
        return OSR_INV_HANDLE;

    default:
        return OSR_FAIL;
    }
}


INT32
x_msg_q_receive(UINT16          *pui2_index,
                VOID            *pv_msg,
                SIZE_T          *pz_size,
                HANDLE_T        *ph_msgq_hdl_list,
                UINT16          ui2_msgq_hdl_count,
                MSGQ_OPTION_T   e_option)
{
    OS_MSGQ_LIGHT_T *pt_msg_q;
    BYTE *read;
    SIZE_T z_size;
    int ret;

    /* check arguments */
    if (e_option != X_MSGQ_OPTION_WAIT && e_option != X_MSGQ_OPTION_NOWAIT)
    {
        return OSR_INV_ARG;
    }

    if ((pui2_index == NULL) || (pv_msg == NULL) ||
        (pz_size == NULL) || (*pz_size == 0) ||
        (ph_msgq_hdl_list == NULL) || (ui2_msgq_hdl_count == 0))
    {
        return OSR_INV_ARG;
    }

    if (ui2_msgq_hdl_count != 1)
    {
        return OSR_NOT_SUPPORT;
    }

    pt_msg_q = (OS_MSGQ_LIGHT_T *)(ph_msgq_hdl_list[0]);
    ret = pthread_mutex_lock(&pt_msg_q->mutex);
    if (ret != 0)
    {
        goto err;
    }
    while (pt_msg_q->read == pt_msg_q->write)
    {
        if (e_option == X_MSGQ_OPTION_NOWAIT)
        {
            VERIFY(pthread_mutex_unlock(&pt_msg_q->mutex) == 0);
            return OSR_NO_MSG;
        }
        VERIFY(pthread_cond_wait(&pt_msg_q->write_cond, &pt_msg_q->mutex) == 0);
    }
    read = pt_msg_q->read;
    z_size = *(SIZE_T *)(read);
    if (z_size > *pz_size)
    {
        z_size = *pz_size;
    }
    memcpy(pv_msg, read + 4, z_size);
    read += pt_msg_q->message_size;
    if (read == pt_msg_q->end)
    {
        read = pt_msg_q->start;
    }
    pt_msg_q->read = read;
    VERIFY(pthread_cond_broadcast(&pt_msg_q->read_cond) == 0);
    VERIFY(pthread_mutex_unlock(&pt_msg_q->mutex) == 0);

    *pui2_index = 0;
    *pz_size = z_size;

    return OSR_OK;

err:
    switch (ret)
    {
    case EINVAL:
        return OSR_INV_HANDLE;

    default:
        return OSR_FAIL;
    }
}


INT32
x_msg_q_receive_timeout(UINT16          *pui2_index,
                        VOID            *pv_msg,
                        SIZE_T          *pz_size,
                        HANDLE_T        *ph_msgq_hdl_list,
                        UINT16          ui2_msgq_hdl_count,
                        UINT32          ui4_time)
{
    OS_MSGQ_LIGHT_T *pt_msg_q;
    BYTE *read;
    SIZE_T z_size;
    struct timespec abstime;
    int ret;

    if ((pui2_index == NULL) || (pv_msg == NULL) ||
        (pz_size == NULL) || (*pz_size == 0) ||
        (ph_msgq_hdl_list == NULL) || (ui2_msgq_hdl_count == 0))
    {
        return OSR_INV_ARG;
    }

    if (ui2_msgq_hdl_count != 1)
    {
        return OSR_NOT_SUPPORT;
    }

    VERIFY(clock_gettime(CLOCK_MONOTONIC, &abstime) == 0);
    abstime.tv_sec  += ui4_time / 1000; ui4_time %= 1000;
    abstime.tv_nsec += ui4_time * 1000000;
    if (abstime.tv_nsec >= 1000000000)
    {
        abstime.tv_nsec -= 1000000000;
        abstime.tv_sec++;
    }

    pt_msg_q = (OS_MSGQ_LIGHT_T *)(ph_msgq_hdl_list[0]);
    ret = pthread_mutex_lock(&pt_msg_q->mutex);
    if (ret != 0)
    {
        goto err;
    }
    while (pt_msg_q->read == pt_msg_q->write)
    {
        ret = pthread_cond_timedwait(&pt_msg_q->write_cond, &pt_msg_q->mutex, &abstime);
        if (ret != 0)
        {
            VERIFY(pthread_mutex_unlock(&pt_msg_q->mutex) == 0);
            goto err;
        }
    }
    read = pt_msg_q->read;
    z_size = *(SIZE_T *)(read);
    if (z_size > *pz_size)
    {
        z_size = *pz_size;
    }
    memcpy(pv_msg, read + 4, z_size);
    read += pt_msg_q->message_size;
    if (read == pt_msg_q->end)
    {
        read = pt_msg_q->start;
    }
    pt_msg_q->read = read;
    VERIFY(pthread_cond_broadcast(&pt_msg_q->read_cond) == 0);
    VERIFY(pthread_mutex_unlock(&pt_msg_q->mutex) == 0);

    *pui2_index = 0;
    *pz_size = z_size;

    return OSR_OK;

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


INT32
x_msg_q_num_msgs(HANDLE_T   h_msg_hdl,
                 UINT16     *pui2_num_msgs)
{
    OS_MSGQ_LIGHT_T *pt_msg_q;
    SIZE_T messages;

    if (pui2_num_msgs == NULL)
    {
        return OSR_INV_ARG;
    }

    pt_msg_q = (OS_MSGQ_LIGHT_T *)(h_msg_hdl);

    messages = pt_msg_q->write - pt_msg_q->read;
    if (pt_msg_q->write < pt_msg_q->read)
    {
        messages += pt_msg_q->end - pt_msg_q->start;
    }
    messages /= pt_msg_q->message_size;
    *pui2_num_msgs = (UINT16)(messages);

    return OSR_OK;
}


INT32
x_msg_q_get_max_msg_size(HANDLE_T   h_msg_hdl,
                         SIZE_T     *pz_max_size)
{
    OS_MSGQ_LIGHT_T *pt_msg_q;

    if (pz_max_size == NULL)
    {
        return OSR_INV_ARG;
    }

    pt_msg_q = (OS_MSGQ_LIGHT_T *)(h_msg_hdl);
    *pz_max_size = pt_msg_q->z_maxsize;

    return OSR_OK;
}
INT32
x_msg_q_flush(HANDLE_T   h_msg_hdl)
{
    OS_MSGQ_LIGHT_T *pt_msg_q;
    int ret;

    if(h_msg_hdl == NULL_HANDLE)
    {
        return OSR_INV_HANDLE;
    }
    pt_msg_q = (OS_MSGQ_LIGHT_T *)(h_msg_hdl);

    ret = pthread_mutex_lock(&pt_msg_q->mutex);
    if (ret != 0)
    {
        goto err;
    }

    pt_msg_q->read = pt_msg_q->start;
    pt_msg_q->write = pt_msg_q->start;
    pt_msg_q->end = pt_msg_q->start + pt_msg_q->message_size * (pt_msg_q->ui2_msg_count + 1);

    VERIFY(pthread_cond_broadcast(&pt_msg_q->write_cond) == 0);
    VERIFY(pthread_mutex_unlock(&pt_msg_q->mutex) == 0);

    return OSR_OK;

err:
    switch (ret)
    {
        case EINVAL:
            return OSR_INV_HANDLE;

        default:
            return OSR_FAIL;
    }
}


INT32
msg_q_init(VOID)
{
    VERIFY(pthread_mutex_init(&s_msg_q_list_mutex, NULL) == 0);
    return OSR_OK;
}

