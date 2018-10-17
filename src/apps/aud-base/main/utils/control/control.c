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


#include "u_control.h"
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#define CONTROL_INFO(...)

#define CONTROL_ERR(...) \
    do \
    { \
        printf("<CONTROL>[%s:%d]:", __FUNCTION__, __LINE__); \
        printf(__VA_ARGS__); \
    } \
    while (0)

typedef struct {
    pthread_mutex_t mMutex;
    pthread_cond_t mCond;
} CONTROL_T;

CONTROL_H u_control_init() {
    int ret;
    pthread_condattr_t condattr;

    CONTROL_T *handle = (CONTROL_T *)malloc(sizeof(CONTROL_T));
    if (NULL == handle) {
        CONTROL_ERR("CONTROL_T malloc failed!\n");
        goto MALLOC_ERR;
    }

    ret = pthread_mutex_init(&handle->mMutex, NULL);
    if (ret) {
        CONTROL_ERR("mutex init failed!\n");
        goto MUTEX_ERR;
    }

    ret = pthread_condattr_init(&condattr);
    if (ret) {
        CONTROL_ERR("condattr init failed!\n");
        goto ATTR_INIT_ERR;
    }

    ret = pthread_condattr_setclock(&condattr, CLOCK_MONOTONIC);
    if (ret) {
        CONTROL_ERR("condattr setclock failed!\n");
        goto ATTR_INIT_ERR;
    }

    ret = pthread_cond_init(&handle->mCond, &condattr);
    if (ret) {
        CONTROL_ERR("cond init failed!\n");
        goto ATTR_INIT_ERR;
    }

    return handle;
ATTR_INIT_ERR:
    pthread_mutex_destroy(&handle->mMutex);
MUTEX_ERR:
    free(handle);
MALLOC_ERR:
    return NULL;
}

void u_control_deinit(CONTROL_H arg) {
    CONTROL_T *handle = (CONTROL_T *)arg;
    if (NULL == arg) {
        CONTROL_ERR("handle can't be NULL!\n");
        return;
    }
    pthread_mutex_destroy(&handle->mMutex);
    pthread_cond_destroy(&handle->mCond);
    free(handle);
}

void u_control_pause(CONTROL_H arg) {
    CONTROL_T *handle = (CONTROL_T *)arg;
    if (NULL == arg) {
        CONTROL_ERR("handle can't be NULL!\n");
        return;
    }
    pthread_mutex_lock(&handle->mMutex);
    pthread_cond_wait(&handle->mCond, &handle->mMutex);
    pthread_mutex_unlock(&handle->mMutex);
}

void u_control_resume(CONTROL_H arg) {
    CONTROL_T *handle = (CONTROL_T *)arg;
    if (NULL == arg) {
        CONTROL_ERR("handle can't be NULL!\n");
        return;
    }
    pthread_mutex_lock(&handle->mMutex);
    pthread_cond_broadcast(&handle->mCond);
    pthread_mutex_unlock(&handle->mMutex);
}

