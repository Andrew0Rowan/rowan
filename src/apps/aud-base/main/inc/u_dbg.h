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
 * $RCSfile: u_dbg.h,v $
 * $Revision: #1 $
 * $Date: 2016/07/22 $
 * $Author:
 *
 * Description:
 *         This header file contains debug related definitions, which are
 *         known to applications and middleware.
 *---------------------------------------------------------------------------*/

#ifndef _U_DBG_H_
#define _U_DBG_H_



/*-----------------------------------------------------------------------------
                    include files
-----------------------------------------------------------------------------*/
#include <stdio.h>
#include "u_common.h"
#include <syslog.h>

#ifdef __cplusplus
extern "C" {
#endif


/*-----------------------------------------------------------------------------
                    macros, defines, typedefs, enums
 ----------------------------------------------------------------------------*/
extern UINT32 ui4_disable_print;
extern UINT32 ui4_enable_all_log;


/* Retun values. */
#define DBGR_OK                 ((INT32)   0)
#define DBGR_OPEN_FAIL          ((INT32)  -1)
#define DBGR_INV_ARG            ((INT32)  -2)
#define DBGR_NOT_ENOUGH_MEM     ((INT32)  -3)
#define DBGR_ALREADY_INIT       ((INT32)  -4)
#define DBGR_NOT_INIT           ((INT32)  -5)
#define DBGR_NO_TRACE_BUFFER    ((INT32)  -6)
#define DBGR_NO_OUTPUT_DEVICE   ((INT32)  -7)
#define DBGR_INV_OUTPUT_DEVICE  ((INT32)  -8)
#define DBGR_NOT_ENABLED        ((INT32)  -9)
#define DBGR_DUMP_IN_PROGRESS   ((INT32) -10)
#define DBGR_REG_CB_ACTIVE      ((INT32) -11)


/* Debug level defines. */
#define DBG_LEVEL_NONE   ((UINT16) 0x0000)
#define DBG_LEVEL_ERROR  ((UINT16) 0x0001)
#define DBG_LEVEL_API    ((UINT16) 0x0002)
#define DBG_LEVEL_INFO   ((UINT16) 0x0004)
#define DBG_LEVEL_SANITY   ((UINT16) 0x0008)
//#define DBG_LEVEL_ALL    ((UINT16) 0xffff)
#define DBG_LEVEL_ALL    ((UINT16) 0x00ff)
#define DBG_LEVEL_MASK   DBG_LEVEL_ALL

#define DBG_LAYER_APP    ((UINT16) 0x0100)
#define DBG_LAYER_MMW    ((UINT16) 0x0200)
#define DBG_LAYER_MW     ((UINT16) 0x0400)
#define DBG_LAYER_SYS    ((UINT16) 0x0800)
#define DBG_LAYER_DRV    ((UINT16) 0x1000)
#define DBG_LAYER_ALL    ((UINT16) 0xFF00)
#define DBG_LAYER_MASK   DBG_LAYER_ALL

/*control by /data/log_all file exist or not*/
#define printf(format, args...)                \
do{                                            \
    if(1 == ui4_enable_all_log)                \
	{                                      \
		printf(format, ##args);        \
   	}                                      \
}while(0)

#define DBG_PRINT(format, args...)                      \
do{                                                     \
    if ((ui4_disable_print & DBG_LAYER_MASK) == 0)          \
        printf(format,##args);                          \
}while(0)

#define DBG_LVL_PRINT(set_layer_lvl, lvl, format, args...)     \
do{                                                     \
    if (ui4_disable_print & (set_layer_lvl & DBG_LAYER_MASK))     \
        break;                                          \
    if ((lvl & (set_layer_lvl & DBG_LEVEL_MASK)) != 0)        \
        printf(format,##args);                          \
}while(0)

#define DBG_ERROR_EX(stat...)                                     \
DBG_LVL_PRINT(DBG_LEVEL_MODULE, DBG_LEVEL_ERROR, stat)

#define DBG_API_EX(stat...)                                       \
DBG_LVL_PRINT(DBG_LEVEL_MODULE, DBG_LEVEL_API, stat)

#define DBG_INFO_EX(stat...)                                     \
DBG_LVL_PRINT(DBG_LEVEL_MODULE, DBG_LEVEL_INFO, stat)

#define DBG_SANITY_EX(stat...)                                    \
DBG_LVL_PRINT(DBG_LEVEL_MODULE, DBG_LEVEL_INFO, stat)
/* Common macros to perform CLI controlled debug statements. */
/* Note that an individual SW Module MUST set the macro      */
/* DBG_LEVE_MODULE else a compile error will occur.          */
#undef DBG_ERROR
#undef DBG_API
#undef DBG_INFO

#define DBG_ERROR(_stmt)  DBG_ERROR_EX _stmt
#define DBG_API(_stmt)    DBG_API_EX _stmt
#define DBG_INFO(_stmt)   DBG_INFO_EX _stmt
#if 1
#define DBG_API_IN
#define DBG_API_OUT
#else
#define DBG_API_IN  printf("IN  -> %s\n",__FUNCTION__)
#define DBG_API_OUT printf("OUT -> %s\n",__FUNCTION__)
#endif
#define DBG_LOG_HERE printf("%s:%d\n",__FUNCTION__,__LINE__)

#define dbg_print(stat...) DBG_PRINT(stat)


#define CHECK_FAIL(func) do\
	{\
		int ret = func; \
		if(ret!=0)DBG_ERROR(("fail ret=%d\n",ret));\
	}while(0)
#define CHECK_FAIL_RET(func) do\
	{\
		int ret = func; \
		if(ret!=0){DBG_ERROR(("fail ret=%d\n",ret));return ret;}\
	}while(0)

/* Macro for debug abort / assert. */
#define DBG_ABORT(_code)  do{printf("%s,%d\n",((CHAR*) __FILE__), ((UINT32) __LINE__)); pthread_exit(_code);}while(0)
#define DBG_ASSERT(_expr, _code)  { if (! (_expr)) DBG_ABORT (_code); }

#define dbg_abort(_code)  DBG_ABORT (_code)


extern int dbg_init(void);

#ifdef __cplusplus
}
#endif

#endif /* _U_DBG_H_ */

