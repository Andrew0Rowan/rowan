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

/*********************************************************************************
* Function Name   : queue_init()                                                 *
* Description     : Used to Reset the queue.                                     *
* Return Parameter: doesn't return anything.                                     *
* Argument        : q  - adress of a queue.                                      *
*********************************************************************************/
static void queue_init(queue *q)
{
    q->head = -1;
    q->tail = -1;
}

/*********************************************************************************
* Function Name   : queue_create()                                               *
* Description     : creates a queue and initializes.                             *
* Return Parameter: returns created Queue address on success, if not NULL.       *
* Argument        : max_q_size  - number of elements in a queue.                 *
*********************************************************************************/
queue* queue_create(unsigned int  max_q_size)
{
    queue *ptr = NULL;

    ptr = (queue *)malloc(sizeof(struct cirqueue));
    if (ptr != NULL)
    {
        memset(ptr, 0, sizeof(struct cirqueue));
        ptr->q_size = max_q_size;
        ptr->array = (char **) malloc(max_q_size);  //allocating memory for the pointer array.
        if (ptr->array != NULL)
        {
            memset(ptr->array, 0, max_q_size);
            queue_init(ptr);
        }
        else
        {
            free(ptr);
            ptr=NULL;
        }
    }
    return ptr;
}

/*********************************************************************************
* Function Name   : queue_destroy()                                              *
* Description     : destroys a queue i.e., frees all allocated memory.           *
* Return Parameter: returns TRUE if the passed address is found and freed.       *
*                   returns FALSE if the passed address is NULL.                 *
* Argument        : q  - adress of the queue to be destroyed.                    *
*********************************************************************************/
BOOLEAN queue_destroy(queue *q)
{
    unsigned int size;
    if (q != NULL)
    {
        while (!queue_get_unit_size(q, &size))
        {
            if (0 > queue_get(q, NULL, size))// Freeing the complete Queue data.
                break;
        }
        free(q->array);
        free(q);
        return TRUE;
     }
     return FALSE;
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

    if (queue_check_queue_full(q))
        return QUEUE_FULL_ERROR;

    if (!buf)
        return QUEUE_PUT_INVALID_INPUT_BUFFER;

    ptr = (unsigned int *)malloc(size + sizeof(unsigned int));
    if (ptr == NULL)
        return QUEUE_PUT_SYS_MEM_ERROR;

    memset(ptr, 0, size + sizeof(unsigned int));

    if (q->head == -1)
        q->head = 0;

    ++q->tail;
    q->tail = q->tail % q->q_size; //wrapping around by taking tail mode of Queue size. Logic for Circular Queue

    *ptr = (unsigned int)size;

    q->array[q->tail] = (char *)ptr;    //lets save the allocated ptr

    ++ptr;

    memcpy(ptr, buf, size);     //copying data contents in to the allocated memory
    return 0;
}

/*********************************************************************************
* Function Name   : queue_check_queuefull()                                      *
* Description     : checks whether the recieved queue is full.                   *
* Return Parameter: returns TRUE if queue is full else FALSE.                    *
* Argument        : q   - address to a queue.                                    *
*********************************************************************************/
BOOLEAN queue_check_queue_full(queue *q)
{
    if (q != NULL)
    {
        if (((q->tail + 1) % (q->q_size)) == q->head)  //checking the queue full condition.
            return TRUE;
    }
    return FALSE;
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
    if (queue_check_queue_empty(q))
    {
        return QUEUE_EMPTY_ERROR;
    }
    if (size != NULL)
    {
        if (((unsigned int *)q->array[q->head]) != NULL)
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
* Return Parameter: returns error code in case of error.             *
* Argument        : q       - address to a queue.                                *
*               data    - memory pointer to which data to be copied.         *
*                   size    - data size to be copied.                            *
*********************************************************************************/
int queue_get(queue *q,char *buf, unsigned int size)
{
    unsigned int *ptr;
    char *cpy_buf;

    if (queue_check_queue_empty(q))
    {
        return QUEUE_EMPTY_ERROR;
    }

    if (size != (*((unsigned int *)q->array[q->head])))
        return QUEUE_GET_SIZE_MISS_MATCH;

    if (q->head == q->tail)        //this condition becomes true when there is a single element in queue.
    {
        if (buf != NULL)
        {
            ptr = (unsigned int *) q->array[q->head];
            ++ptr;  //skip length while returning
            cpy_buf = (char *) ptr;
            memcpy(buf, cpy_buf, (size));
        }
        free(q->array[q->head]);
        queue_init(q);
    }
    else
    {
        if (buf != NULL && size != 0)
        {
            ptr = (unsigned int *) q->array[q->head];
            ++ptr;  //skip length while returning
            cpy_buf = (char *) ptr;
            memcpy(buf, cpy_buf, (size));
        }
        free(q->array[q->head]);    //remove one element from the Queue.
        ++q->head;
        q->head = q->head % q->q_size;  //update head of the queue.
    }
    return 0;
}

/**********************************************************************************
* Function Name   : queue_check_queue_empty()                                     *
* Description     : checks whether the recieved queue is empty.                   *
* Return Parameter: returns TRUE if queue is empty else FALSE.                    *
* Argument        : q - address to a queue                                        *
**********************************************************************************/
BOOLEAN queue_check_queue_empty(queue *q)
{
    if (q != NULL)
    {
        if ((q->head == -1) && (q->tail == -1))
            return TRUE;
    }
    return FALSE;
}

/*********************************** end of queue.c *******************************/
