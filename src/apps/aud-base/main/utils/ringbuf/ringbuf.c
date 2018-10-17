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
#include <stdlib.h>
#include <pthread.h>
#include "u_dbg.h"
#include "u_ringbuf.h"

#define RINGBUF_INFO(...)

#define RINGBUF_ERR(...) \
    do \
    { \
        printf("<RINGBUF>[%s:%d]:", __FUNCTION__, __LINE__); \
        printf(__VA_ARGS__); \
    } \
    while (0)

typedef struct
{
    VOID *p_buf;
    UINT32 ui4_size;
    UINT32 ui4_write;
    UINT32 ui4_read;
} RINGBUF_T;

static UINT32 _ringbuf_get_available_size(RINGBUF_T *pt_ringbuf)
{
    UINT32 ui4_write = pt_ringbuf->ui4_write;
    UINT32 ui4_read  = pt_ringbuf->ui4_read;
    UINT32 ui4_size  = pt_ringbuf->ui4_size;

    if (ui4_write >= ui4_read)
    {
        return ui4_size - (ui4_write - ui4_read) - 1;
    }
    else
    {
        return ui4_read - ui4_write - 1;
    }
}

static VOID _ringbuf_write_data(RINGBUF_T *pt_ringbuf, VOID *p_buf, UINT32 ui4_size)
{
    memcpy(pt_ringbuf->p_buf + pt_ringbuf->ui4_write, p_buf, ui4_size);
    pt_ringbuf->ui4_write += ui4_size;

    if (pt_ringbuf->ui4_write == pt_ringbuf->ui4_size)
    {
        pt_ringbuf->ui4_write = 0;
    }
}

static VOID _ringbuf_write_data_loopback(RINGBUF_T *pt_ringbuf, VOID *p_buf, UINT32 ui4_size)
{
    UINT32 ui4_first_size = pt_ringbuf->ui4_size - pt_ringbuf->ui4_write;
    UINT32 ui4_second_size = ui4_size - ui4_first_size;

    memcpy(pt_ringbuf->p_buf + pt_ringbuf->ui4_write, p_buf, ui4_first_size);
    memcpy(pt_ringbuf->p_buf, p_buf + ui4_first_size, ui4_second_size);
    pt_ringbuf->ui4_write = ui4_second_size;
}

static VOID _ringbuf_read_data(RINGBUF_T *pt_ringbuf, VOID *p_buf, UINT32 ui4_size)
{
    memcpy(p_buf, pt_ringbuf->p_buf + pt_ringbuf->ui4_read, ui4_size);
    pt_ringbuf->ui4_read += ui4_size;

    if (pt_ringbuf->ui4_read == pt_ringbuf->ui4_size)
    {
        pt_ringbuf->ui4_read = 0;
    }
}

static VOID _ringbuf_read_data_loopback(RINGBUF_T *pt_ringbuf, VOID *p_buf, UINT32 ui4_size)
{
    UINT32 ui4_first_size = pt_ringbuf->ui4_size - pt_ringbuf->ui4_read;
    UINT32 ui4_second_size = ui4_size - ui4_first_size;

    memcpy(p_buf, pt_ringbuf->p_buf + pt_ringbuf->ui4_read, ui4_first_size);
    memcpy(p_buf + ui4_first_size, pt_ringbuf->p_buf, ui4_second_size);
    pt_ringbuf->ui4_read = ui4_second_size;
}

static UINT32 _ringbuf_get_used_size(RINGBUF_T *pt_ringbuf)
{
    return pt_ringbuf->ui4_size - 1 - _ringbuf_get_available_size(pt_ringbuf);
}

RINGBUF_H u_ringbuf_malloc(UINT32 ui4_size)
{
    INT32 i4_ret;
    RINGBUF_T *pt_ringbuf;

    pt_ringbuf = (RINGBUF_T *)calloc(sizeof(RINGBUF_T), sizeof(INT8));
    if (NULL == pt_ringbuf)
    {
        RINGBUF_ERR("malloc ringbuf failed! size:%d\n", sizeof(RINGBUF_T));
        goto MALLOC_RINGBUF_ERR;
    }

    //malloc ui4_size + 1:ringbuf must reserve 1 byte to calc space
    pt_ringbuf->p_buf = calloc(ui4_size + 1, sizeof(INT8));
    if (NULL == pt_ringbuf->p_buf)
    {
        RINGBUF_ERR("malloc buf failed! size:%d\n", ui4_size);
        goto MALLOC_BUF_ERR;
    }
    pt_ringbuf->ui4_size = ui4_size + 1;

    return pt_ringbuf;

MALLOC_BUF_ERR:
    free(pt_ringbuf);
MALLOC_RINGBUF_ERR:
    return NULL;
}

VOID u_ringbuf_free(RINGBUF_H handle)
{
    RINGBUF_T *pt_ringbuf = (RINGBUF_T *)handle;
    if (NULL == pt_ringbuf)
    {
        RINGBUF_ERR("handle can't be NULL");
        return;
    }

    if (NULL != pt_ringbuf->p_buf)
    {
        free(pt_ringbuf->p_buf);
    }

    free(pt_ringbuf);
}

INT32 u_ringbuf_write(RINGBUF_H handle, VOID *p_data, UINT32 ui4_size)
{
    UINT32 ui4_available_size;
    RINGBUF_T *pt_ringbuf = (RINGBUF_T *)handle;

    if (NULL == pt_ringbuf)
    {
        RINGBUF_ERR("ringbuf handle cann't be NULL");
        return RINGBUF_INV_ARG;
    }

    if (NULL == p_data)
    {
        RINGBUF_ERR("arg not valid!\n");
        return RINGBUF_INV_ARG;
    }

    if (0 == ui4_size)
    {
        RINGBUF_INFO("size is 0!\n");
        return ui4_size;
    }


    ui4_available_size = _ringbuf_get_available_size(pt_ringbuf);
    if (ui4_size > ui4_available_size)
    {
        RINGBUF_INFO("write size(%d) is bigger than available size(%d)\n", ui4_size, ui4_available_size);
        ui4_size = ui4_available_size;
    }

    if (pt_ringbuf->ui4_size >= (pt_ringbuf->ui4_write + ui4_size))
    {
        _ringbuf_write_data(pt_ringbuf, p_data, ui4_size);
    }
    else
    {
        _ringbuf_write_data_loopback(pt_ringbuf, p_data, ui4_size);
    }

    return ui4_size;
}

INT32 u_ringbuf_read(RINGBUF_H handle, VOID *p_buf, UINT32 ui4_size)
{
    UINT32 ui4_data_size;
    RINGBUF_T *pt_ringbuf = (RINGBUF_T *)handle;

    if (NULL == pt_ringbuf)
    {
        RINGBUF_ERR("ringbuf handle cann't be NULL");
        return RINGBUF_INV_ARG;
    }

    if (NULL == p_buf)
    {
        RINGBUF_ERR("arg not valid!\n");
        return RINGBUF_INV_ARG;
    }

    if (0 == ui4_size)
    {
        RINGBUF_INFO("size is 0!\n");
        return ui4_size;
    }


    ui4_data_size = _ringbuf_get_used_size(pt_ringbuf);
    if (ui4_size > ui4_data_size)
    {
        RINGBUF_INFO("read size(%d) is bigger than data size(%d)\n", ui4_size, ui4_data_size);
        ui4_size = ui4_data_size;
    }

    if (pt_ringbuf->ui4_size >= (pt_ringbuf->ui4_read + ui4_size))
    {
        _ringbuf_read_data(pt_ringbuf, p_buf, ui4_size);
    }
    else
    {
        _ringbuf_read_data_loopback(pt_ringbuf, p_buf, ui4_size);
    }

    return ui4_size;
}

INT32 u_ringbuf_clear(RINGBUF_H handle)
{
    RINGBUF_T *pt_ringbuf = (RINGBUF_T *)handle;

    if ((NULL == pt_ringbuf) || (NULL == pt_ringbuf->p_buf))
    {
        RINGBUF_ERR("ringbuf handle cann't be NULL");
        return RINGBUF_INV_ARG;
    }

    memset(pt_ringbuf->p_buf, 0, pt_ringbuf->ui4_size);
    pt_ringbuf->ui4_read = 0;
    pt_ringbuf->ui4_write = 0;

    return RINGBUF_OK;
}

INT32 u_ringbuf_get_used_size(RINGBUF_H handle)
{
    RINGBUF_T *pt_ringbuf = (RINGBUF_T *)handle;

    if (NULL == pt_ringbuf)
    {
        RINGBUF_ERR("ringbuf handle cann't be NULL\n");
        return RINGBUF_INV_ARG;
    }

	return (INT32)_ringbuf_get_used_size(pt_ringbuf);
}
