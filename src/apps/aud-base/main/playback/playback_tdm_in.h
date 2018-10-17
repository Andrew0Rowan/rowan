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


#ifndef _TDM_IN_H_
#define _TDM_IN_H_

#include "u_handle.h"
#include "u_dbg.h"
#include "u_alsa_interface.h"
#include "u_ringbuf.h"
#include <alsa/asoundlib.h>

#define DBG_INIT_LEVEL_APP_TDM_IN (DBG_LEVEL_ALL|DBG_LAYER_APP)

#undef  DBG_LEVEL_MODULE
#define DBG_LEVEL_MODULE tdm_in_get_dbg_level()

#define TDM_IN_TAG             "<tdm_in>"
#define MAIN_PCM_DEVICE_NAME   "main"
#define TDM_IN_DEVICE_NAME     "hw:0,1"

#define TDM_IN_INFO(...)    do{DBG_INFO((TDM_IN_TAG));DBG_INFO(("[%s:%d]:", __FUNCTION__, __LINE__));DBG_INFO((__VA_ARGS__));}while(0)
#define TDM_IN_ERR(...)     do{DBG_ERROR((TDM_IN_TAG));DBG_INFO(("[%s:%d]:", __FUNCTION__, __LINE__));DBG_ERROR((__VA_ARGS__));}while(0)
#define TDM_IN_MSG(...)     TDM_IN_ERR(__VA_ARGS__)

#define TDM_IN_SAMPLERATE     48000
#define TDM_IN_CHANNLE        8
#define TDM_IN_BITWIDTH       32

typedef struct
{
    pthread_cond_t t_read_cond;
    pthread_cond_t t_record_pause_cond;
    pthread_cond_t t_play_pause_cond;
    pthread_mutex_t t_mutex;
} TDM_IN_PLAY_COND_T;

/* application structure */
typedef struct _TDM_IN_OBJ_T
{
    HANDLE_T            h_app;
    BOOL                b_app_init_ok;
    BOOL                b_pause;
    BOOL                b_thread_running;
    BOOL                b_switch;
    TDM_IN_PLAY_COND_T  t_play_cond;
}TDM_IN_OBJ_T;

#endif
