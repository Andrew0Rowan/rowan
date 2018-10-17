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


#include "queue.h"
#include <stdio.h>

/**** static functions ****/

/***************************/



/*********************************************************************************
* Function Name   : queue_create()                                               *
* Description     : creates a queue and initializes.                             *
* Return Parameter: returns created Queue address on success, if not NULL.       *
* Argument        : max_q_size  - number of elements in a queue.                 *
*********************************************************************************/
queue* queue_create(unsigned int  max_q_size, unsigned int entry_size)
{
    queue *q_ptr = NULL;
    char *ptr_entry = NULL;
    unsigned int total_size = 0;
    int i = 0;

    /* align entry size */
    entry_size = (entry_size + sizeof(int) - 1) / sizeof(int) * sizeof(int);

    /* add the length of buffer length field */
    entry_size += sizeof(unsigned int);

    /* total size need allocated */
    total_size = max_q_size * entry_size;

    total_size += sizeof(struct cirqueue);

    q_ptr = (queue *)malloc(total_size);
    if (NULL == q_ptr)
    {
        printf("[AV_Multi][Queue] allocate memory failed");
        return NULL;
    }

    memset(q_ptr, 0, total_size);
    q_ptr->array = (char **) malloc(max_q_size * sizeof(char *));
    if (NULL == q_ptr->array)
    {
        free(q_ptr);
        printf("[AV_Multi][Queue] allocate memory failed");
        return NULL;
    }

    memset(q_ptr->array, 0, max_q_size * sizeof(char *));
    q_ptr->q_size = max_q_size;
    q_ptr->head = 0;
    q_ptr->tail = 0;

    /* init array */
    ptr_entry = (char*)(q_ptr+1);
    for (i = 0;i < max_q_size;i++)
    {
        q_ptr->array[i] = ptr_entry;
        ptr_entry += entry_size;
    }

    return q_ptr;
}

/*********************************************************************************
* Function Name   : queue_destroy()                                              *
* Description     : destroys a queue i.e., frees all allocated memory.           *
* Return Parameter: returns 1 if the passed address is found and freed.       *
*                   returns 0 if the passed address is NULL.                 *
* Argument        : q  - adress of the queue to be destroyed.                    *
*********************************************************************************/
int queue_destroy(queue *q)
{
    if(q != NULL)
    {
        free(q->array);
        free(q);
        return 1;
     }
     return 0;
}

/*********************************************************************************
* Function Name   : queue_put()                                                  *
* Description     : pushes value into to the queue.                              *
* Return Parameter: returns 0 for success and error code in case of error.       *
* Argument        : q    - address of the queue.                                 *
*                   buf  - memory pointer of a data to be pushed into queue.     *
*                   size - size to be pushed into queue.                         *
*********************************************************************************/
int  queue_put(queue *q,char *buf, unsigned int size)
{
    unsigned int *ptr = NULL;

    if ((NULL == q) || (NULL == buf))
    {
        printf("[Queue] NULL pointer, q=%p, buf=%p", q, buf);
        return QUEUE_PUT_SYS_MEM_ERROR;
    }

    if (((q->tail + 1) % q->q_size) == q->head)
    {
        printf("[Queue] q is full");
        return QUEUE_FULL_ERROR;
    }

    ptr = (unsigned int *)q->array[q->tail];

    *ptr = size;
    ptr++;
    memcpy(ptr, buf, size);

    q->tail = (q->tail + 1) % q->q_size;

    return 0;
}

/*********************************************************************************
* Function Name   : queue_check_queuefull()                                      *
* Description     : checks whether the recieved queue is full.                   *
* Return Parameter: returns 1 if queue is full else 0.                    *
* Argument        : q   - address to a queue.                                    *
*********************************************************************************/
int queue_check_queue_full(queue *q)
{
    if(q != NULL)
    {
        if (((q->tail + 1) % q->q_size) != q->head)
        {
            return 0;
        }
    }
    return 1;
}

/*********************************************************************************
* Function Name   : queue_get_unit_size()                                        *
* Description     : copy the buffer from the queue and delete it from the queue. *
* Return Parameter: returns error code in case of error.                         *
* Argument        : q       - address to a queue.                                *
*                   size    - size of the data of first element of Queue.        *
*********************************************************************************/
int queue_get_unit_size(queue *q, unsigned int *size)
{
    if(queue_check_queue_empty(q))
    {
        return QUEUE_EMPTY_ERROR;
    }
    if(size != NULL)
    {
        if(((unsigned int *)q->array[q->head]) != NULL)
        {
            *size = (*((unsigned int *)q->array[q->head]));
             return 0;
        }
    }
    return QUEUE_GET_UNIT_SIZE_FAILED;
}

/*********************************************************************************
* Function Name   : queue_get()                                                  *
* Description     : copy the buffer from the queue and delete it from the queue. *
* Return Parameter: returns error code in case of error.			 *
* Argument        : q       - address to a queue.                                *
* 	            data    - memory pointer to which data to be copied.     	 *
*                   size    - data size to be copied.                            *
*********************************************************************************/
int queue_get(queue *q,char *buf, unsigned int size)
{
    unsigned int *ptr_buf;
    char *cpy_buf;

    if (NULL == q)
    {
        return QUEUE_PUT_SYS_MEM_ERROR;
    }

    if (q->head == q->tail)
    {
        printf("[AV_Multi][Queue] q is empty, head=%d, tail=%d", q->head, q->tail);
        return QUEUE_EMPTY_ERROR;
    }

    ptr_buf = (unsigned int *)q->array[q->head];
    if(size != (*ptr_buf))
    {
        printf("[Multi][Queue]size is not match(%u <> %u)", size, (*ptr_buf));
        return QUEUE_GET_SIZE_MISS_MATCH;
    }

    cpy_buf = (char*)(++ptr_buf);
    memcpy(buf, cpy_buf, size);
    q->head = (q->head + 1) % q->q_size;

    return 0;
}

/**********************************************************************************
* Function Name   : queue_check_queue_empty()                                     *
* Description     : checks whether the recieved queue is empty.                   *
* Return Parameter: returns 1 if queue is empty else 0.                    *
* Argument        : q - address to a queue                                        *
**********************************************************************************/
int queue_check_queue_empty(queue *q)
{
    if(q != NULL)
    {
        if (q->head == q->tail)
        {
            return 1;
        }
    }
    return 0;
}

int queue_get_size(queue *q)
{
    if (NULL == q)
    {
        return 0;
    }

    return (q->tail + q->q_size - q->head) % q->q_size;
}

int queue_get_cap(queue *q)
{
    if (NULL == q || q->q_size <= 1)
    {
        return 0;
    }

    return q->q_size - 1;
}

/*********************************** end of queue.c *******************************/
