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


/*-----------------------------------------------------------------------------
 *
 * Description:
 *
 *---------------------------------------------------------------------------*/

#ifndef X_TIMER_H
#define X_TIMER_H

#include "x_typedef.h"

//============================================================================
// Type definitions
//============================================================================

// Note: For a system with 100Hz timer tick, an UINT32 can only represent
// about 500 days. It may be insufficient for certain cases, but it's more
// convenient and efficient to process a 32-bit integer rather than a 64-
// bit integer.
//
typedef struct
{
  UINT32    u4L;                    //Low 32bits
  UINT32    u4H;                   //Hish 32bits
} HAL_RAW_TIME_T;

//typedef struct
//{
  //UINT32    u4Ticks;                    //Number of timer interrupts from startup
  //UINT32    u4Cycles;                   //System cycles from last timer interrupt
//} HAL_RAW_TIME_32bit_T;

typedef struct
{
  UINT32    u4Seconds;                  //Number of seconds from startup
  UINT32    u4Micros;                   //Remainder in microsecond
} HAL_TIME_T;

//============================================================================
// Function prototypes
//============================================================================
extern BOOL HAL_InitTimer(void);
extern BOOL HAL_ResetTimer(void);
extern void HAL_GetRawTime(HAL_RAW_TIME_T* pRawTime);
extern void HAL_GetTime(HAL_TIME_T* pTime);
extern void HAL_RawToTime(const HAL_RAW_TIME_T* pRawTime, HAL_TIME_T* pTime);

extern void HAL_GetDeltaTime(HAL_TIME_T* pResult, HAL_TIME_T* pOlder,
    HAL_TIME_T* pNewer);

extern void HAL_GetDeltaRawTime(HAL_RAW_TIME_T* pResult,
    const HAL_RAW_TIME_T* pOlder, const HAL_RAW_TIME_T* pNewer);

extern void HAL_Delay_us(UINT32 u4Micros);

extern BOOL HAL_InitManualTimer(UINT32 u4TimerInterval);

extern UINT32 HAL_GetFineTick(void);
extern void HAL_GetSysUptime(UINT32 *pui4_sec, UINT32 *pui4_micro_sec);

#endif	// X_TIMER_H

