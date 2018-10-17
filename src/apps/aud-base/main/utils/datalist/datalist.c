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


#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include "u_datalist.h"

#define DATALIST_INFO(...)

#define DATALIST_ERR(...) \
    do \
    { \
        printf("<DATLIST>[%s:%d]:", __FUNCTION__, __LINE__); \
        printf(__VA_ARGS__); \
    } \
    while (0)

void u_datalist_init(DATALIST_T *list_head)
{
    assert(list_head);
    list_head->head = NULL;
}

int u_datalist_enqueue(DATALIST_T *list_head, void *data, size_t size, int id)
{
    assert(list_head);

    DATANODE_T *list_temp = list_head->head;
    DATANODE_T *node = (DATANODE_T *)calloc(sizeof(DATANODE_T), 1);
    if (NULL == node)
    {
        DATALIST_ERR("malloc node failed!\n");
        goto MALLOC_NODE_ERR;
    }

    if (data && size)
    {
        node->data = malloc(size);
        if (NULL == node->data)
        {
            DATALIST_ERR("malloc data failed!\n");
            goto MALLOC_DATA_ERR;
        }
        memcpy(node->data, data, size);
        node->size = size;
    }

    node->id = id;
    node->next = NULL;

    if (NULL == list_temp)
    {
        list_head->head = node;
    }
    else
    {
        while (list_temp->next)
        {
            list_temp = list_temp->next;
        }
        list_temp->next = node;
    }

    return DATALIST_SUCCESS;

MALLOC_DATA_ERR:
    free(node);
MALLOC_NODE_ERR:
    return DATALIST_FAIL;
}

int u_datalist_head_enqueue(DATALIST_T *list_head, void *data, size_t size, int id)
{
    assert(list_head);

   DATANODE_T *node = (DATANODE_T *)calloc(sizeof(DATANODE_T), 1);
    if (NULL == node)
    {
        DATALIST_ERR("malloc node failed!\n");
        goto MALLOC_NODE_ERR;
    }

    if (data && size)
    {
        node->data = malloc(size);
        if (NULL == node->data)
        {
            DATALIST_ERR("malloc data failed!\n");
            goto MALLOC_DATA_ERR;
        }
        memcpy(node->data, data, size);
        node->size = size;
    }

    node->id = id;
    node->next = list_head->head;
    list_head->head = node;

    return DATALIST_SUCCESS;

MALLOC_DATA_ERR:
    free(node);
MALLOC_NODE_ERR:
    return DATALIST_FAIL;
}

DATANODE_T *u_datalist_dequeue(DATALIST_T *list_head)
{
    assert(list_head);
    if (NULL == list_head->head)
    {
        DATALIST_INFO("list is empty!\n");
        return NULL;
    }

    DATANODE_T *node = list_head->head;
    list_head->head = node->next;

    return node;
}

void u_datanode_free(DATANODE_T *node)
{
    if (NULL == node)
    {
        return;
    }

    if (node->data)
    {
        free(node->data);
    }
    free(node);
}

void u_datalist_destroy(DATALIST_T *list_head)
{
    DATANODE_T *pt_node;
    assert(list_head);
    if (NULL == list_head->head)
    {
        return;
    }

    while (pt_node = u_datalist_dequeue(list_head))
    {
        u_datanode_free(pt_node);
    }
}

int u_datalist_is_empty(DATALIST_T *list_head)
{
    return (!(list_head->head));
}

