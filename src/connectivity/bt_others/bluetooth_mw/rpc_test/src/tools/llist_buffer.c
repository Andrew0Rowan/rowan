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

#include <string.h>
#include <stdlib.h>

#include "llist_buffer.h"

l_list* llist_buffer_create()
{
    l_list *handle;

    handle = (l_list *)malloc(sizeof(l_list));
    if (handle == NULL)
        return handle;
    memset(handle, 0, sizeof(l_list));
    handle->next = NULL;
    handle->usrdata = NULL;
    return handle;
}

unsigned char llist_buffer_write(l_list *handle, char *usrdata, unsigned int size)
{
    l_list *new, *temp;
    void *destination;

    if (!handle)
        return LLIST_WRITE_INVALID_HANDLE_ERR;

    destination = (void *)malloc(size);
    if (destination == NULL)
        return LLIST_WRITE_MALLOC_ERR1;

    memset(destination, 0, size);
    if (handle->next == NULL)
    {
        handle->usrdata = destination;
        memcpy(&handle->usrdata[0], usrdata,  size);
    }
    else
    {
        if ((new = (l_list *)malloc(sizeof(l_list))) == NULL)
        {
            free(destination);
            return LLIST_WRITE_MALLOC_ERR2;
        }
        memset(new, 0, sizeof(l_list));
        new->usrdata = destination;
        new->next = NULL;
        memcpy(&new->usrdata[0],  usrdata,  size);

        for (temp = handle; (temp->next) != NULL; temp = temp->next);
        temp->next = new;
    }
    return SUCCESS;
}

unsigned char llist_buffer_read_first_node(l_list *handle, char *data_buf, char **data_ptr, unsigned int unit_size, unsigned int index)
{
    if (!handle)
        return LLIST_READ_INVALID_HANDLE_ERR;
    if (!index )
        return LLIST_READ_INVALID_ARGUMENT_ERR;

    if (index == 1)
    {
        if (data_buf != NULL)
            memcpy(data_buf, &handle->usrdata[0], unit_size);
        if (data_ptr)
            *data_ptr = &handle->usrdata[0];
    }
    else
    {
        index = ((index - 1) * unit_size);
        if (data_buf != NULL)
            memcpy(data_buf, &handle->usrdata[index], unit_size);
        if (data_ptr)
            *data_ptr = &handle->usrdata[index];
    }
    return SUCCESS;
}

l_list* llist_buffer_delete_first_node_and_update_list(l_list *handle)
{
    l_list *prev;

    if (!handle)
        return NULL;
    prev = handle;
    handle = handle->next;
    if (prev->usrdata != NULL)
    {
        free(prev->usrdata);
        prev->usrdata = NULL;
    }
    if (prev->next != NULL)// only free if this is not the only node left.
        free(prev);
    else
        return prev;

    return handle;
}

void llist_buffer_destroy(l_list *handle)
{
    l_list *prev;

    while (handle != NULL)
    {
        prev = handle;
        handle = handle->next;
        if (prev->usrdata != NULL)
            free(prev->usrdata);
        free(prev);
    }
}
