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

/*----------------------------------------------------------------------------*
 * No Warranty                                                                *
 * Except as may be otherwise agreed to in writing, no warranties of any      *
 * kind, whether express or implied, are given by MTK with respect to any MTK *
 * Deliverables or any use thereof, and MTK Deliverables are provided on an   *
 * "AS IS" basis.  MTK hereby expressly disclaims all such warranties,        *
 * including any implied warranties of merchantability, non-infringement and  *
 * fitness for a particular purpose and any warranties arising out of course  *
 * of performance, course of dealing or usage of trade.  Parties further      *
 * acknowledge that Company may, either presently and/or in the future,       *
 * instruct MTK to assist it in the development and the implementation, in    *
 * accordance with Company's designs, of certain softwares relating to        *
 * Company's product(s) (the "Services").  Except as may be otherwise agreed  *
 * to in writing, no warranties of any kind, whether express or implied, are  *
 * given by MTK with respect to the Services provided, and the Services are   *
 * provided on an "AS IS" basis.  Company further acknowledges that the       *
 * Services may contain errors, that testing is important and Company is      *
 * solely responsible for fully testing the Services and/or derivatives       *
 * thereof before they are used, sublicensed or distributed.  Should there be *
 * any third party action brought against MTK, arising out of or relating to  *
 * the Services, Company agree to fully indemnify and hold MTK harmless.      *
 * If the parties mutually agree to enter into or continue a business         *
 * relationship or other arrangement, the terms and conditions set forth      *
 * hereunder shall remain effective and, unless explicitly stated otherwise,  *
 * shall prevail in the event of a conflict in the terms in any agreements    *
 * entered into between the parties.                                          *
 *---------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------
 * Copyright (c) 2004, CrystalMedia Technology, Inc.
 * All rights reserved.
 *
 * Unauthorized use, practice, perform, copy, distribution, reproduction,
 * or disclosure of this information in whole or in part is prohibited.
 *-----------------------------------------------------------------------------
 * $RCSfile: u_common.h,v $
 * $Revision: #1 $
 * $Date: 2015/05/13 $
 * $Author: bdbm01 $
 *
 * Description:
 *         This header file contains type definitions common to the whole
 *         Middleware.
 *---------------------------------------------------------------------------*/

#ifndef _U_COMMON_H_
#define _U_COMMON_H_


/*-----------------------------------------------------------------------------
                    include files
 ----------------------------------------------------------------------------*/

#include <stdarg.h>
#include <stddef.h>

#if defined(MTK_BT_SYS_LOG)
#include <stdio.h>
#include <syslog.h>
#endif

/*-----------------------------------------------------------------------------
                    macros, defines, typedefs, enums
 ----------------------------------------------------------------------------*/
#if !defined (_NO_TYPEDEF_UCHAR_) && !defined (_TYPEDEF_UCHAR_)
typedef unsigned char  UCHAR;
#define _TYPEDEF_UCHAR_
#endif

#if !defined (_NO_TYPEDEF_UINT8_) && !defined (_TYPEDEF_UINT8_)
typedef unsigned char  UINT8;

#define _TYPEDEF_UINT8_
#endif

#if !defined (_NO_TYPEDEF_UINT16_) && !defined (_TYPEDEF_UINT16_)
typedef unsigned short  UINT16;

#define _TYPEDEF_UINT16_
#endif

#if !defined (_NO_TYPEDEF_UINT32_) && !defined (_TYPEDEF_UINT32_)

#ifndef EXT_UINT32_TYPE
typedef unsigned int    UINT32;
#else
typedef EXT_UINT32_TYPE  UINT32;
#endif

#define _TYPEDEF_UINT32_
#endif

#if !defined (_NO_TYPEDEF_UINT64_) && !defined (_TYPEDEF_UINT64_)

#ifndef EXT_UINT64_TYPE
typedef unsigned long long  UINT64;
#else
typedef EXT_UINT64_TYPE     UINT64;
#endif

#define _TYPEDEF_UINT64_
#endif

#if !defined (_NO_TYPEDEF_CHAR_) && !defined (_TYPEDEF_CHAR_)
typedef char  CHAR;

#define _TYPEDEF_CHAR_
#endif

#if !defined (_NO_TYPEDEF_INT8_) && !defined (_TYPEDEF_INT8_)
typedef signed char  INT8;

#define _TYPEDEF_INT8_
#endif

#if !defined (_NO_TYPEDEF_INT16_) && !defined (_TYPEDEF_INT16_)
typedef signed short  INT16;

#define _TYPEDEF_INT16_
#endif

#if !defined (_NO_TYPEDEF_INT32_) && !defined (_TYPEDEF_INT32_)

#ifndef EXT_INT32_TYPE
typedef signed int     INT32;
#else
typedef EXT_INT32_TYPE  INT32;
#endif

#define _TYPEDEF_INT32_
#endif

#if !defined (_NO_TYPEDEF_INT64_) && !defined (_TYPEDEF_INT64_)

#ifndef EXT_INT64_TYPE
typedef signed long long  INT64;
#else
typedef EXT_INT64_TYPE    INT64;
#endif

#define _TYPEDEF_INT64_
#endif

#if !defined (_NO_TYPEDEF_SIZE_T_) && !defined (_TYPEDEF_SIZE_T_)

#ifndef EXT_SIZE_T_TYPE
typedef size_t           SIZE_T;
#else
typedef EXT_SIZE_T_TYPE  SIZE_T;
#endif

#define _TYPEDEF_SIZE_T_
#endif

#if !defined (_NO_TYPEDEF_UTF16_T_) && !defined (_TYPEDEF_UTF16_T_)
typedef unsigned short  UTF16_T;

#define _TYPEDEF_UTF16_T_
#endif

#if !defined (_NO_TYPEDEF_UTF32_T_) && !defined (_TYPEDEF_UTF32_T_)
typedef unsigned int  UTF32_T;

#define _TYPEDEF_UTF32_T_
#endif

#if !defined (_NO_TYPEDEF_FLOAT_) && !defined (_TYPEDEF_FLOAT_)
typedef float  FLOAT;

#define _TYPEDEF_FLOAT_
#endif

#if !defined (_NO_TYPEDEF_DOUBLE_)  && !defined (_TYPEDEF_DOUBLE_)
typedef double  DOUBLE;

#define _TYPEDEF_DOUBLE_
#endif

/* Do not typedef VOID but use define. Will keep some */
/* compilers very happy.                              */
#if !defined (_NO_TYPEDEF_VOID_)  && !defined (_TYPEDEF_VOID_)
#undef VOID
#define VOID  void

#define _TYPEDEF_VOID_
#endif

/* Define a boolean as 8 bits. */
#if !defined (_NO_TYPEDEF_BOOL_) && !defined (_TYPEDEF_BOOL_)

#ifndef EXT_BOOL_TYPE
typedef UINT8  BOOL;
#else
typedef signed EXT_BOOL_TYPE  BOOL;
#endif

#if !defined (_NO_TYPEDEF_HANDLE_T_) && !defined (_TYPEDEF_HANDLE_T_)
typedef UINT32  HANDLE_T;

#define _TYPEDEF_HANDLE_T_
#endif


#define _TYPEDEF_BOOL_

#ifdef TRUE
#undef TRUE
#endif

#ifdef FALSE
#undef FALSE
#endif

#define TRUE 1
#define FALSE 0
#endif

/* Variable argument macros. The ones named "va_..." are defined */
/* in the header file "stdarg.h".                                */
#ifdef VA_LIST
#undef VA_LIST
#endif

#ifdef VA_START
#undef VA_START
#endif

#ifdef VA_END
#undef VA_END
#endif

#ifdef VA_ARG
#undef VA_ARG
#endif

#ifdef VA_COPY
#undef VA_COPY
#endif

#define VA_LIST  va_list
#define VA_START va_start
#define VA_END   va_end
#define VA_ARG   va_arg
#define VA_COPY  va_copy


/* Min and max macros. Watch for side effects! */
#define X_MIN(_x, _y)  (((_x) < (_y)) ? (_x) : (_y))
#define X_MAX(_x, _y)  (((_x) > (_y)) ? (_x) : (_y))

/* The following macros are useful to create bit masks. */
#define MAKE_BIT_MASK_8(_val)  (((UINT8)  1) << _val)
#define MAKE_BIT_MASK_16(_val) (((UINT16) 1) << _val)
#define MAKE_BIT_MASK_32(_val) (((UINT32) 1) << _val)
#define MAKE_BIT_MASK_64(_val) (((UINT64) 1) << _val)

/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/
/* COMMENT OTHERS< RPC JUST NEED TYPEDEF */

#if defined(MTK_BT_SYS_LOG)
extern UINT32 ui4_enable_all_log;

/*control by /data/log_all file exist or not*/
#define printf(format, args...)                \
do{                                            \
    if (0 == ui4_enable_all_log)               \
    {                                          \
        syslog(LOG_WARNING, format, ##args);   \
    }                                          \
    else                                       \
    {                                          \
        syslog(LOG_WARNING, format, ##args);   \
        printf(format, ##args);                \
    }                                          \
}while(0)
#endif

#endif /* _U_COMMON_H_ */
