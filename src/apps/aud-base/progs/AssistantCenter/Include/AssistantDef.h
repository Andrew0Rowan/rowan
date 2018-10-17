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


#ifndef __ASSISTANTDEF_H__
#define __ASSISTANTDEF_H__

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdio.h>
//#include "u_common.h"
#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#if !defined (_NO_TYPEDEF_BYTE_) && !defined (_TYPEDEF_BYTE_)
typedef unsigned char  BYTE;
#define _TYPEDEF_BYTE_
#endif

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
typedef unsigned long  UTF32_T;

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

#define _TYPEDEF_BOOL_

#ifdef TRUE
#undef TRUE
#endif

#ifdef FALSE
#undef FALSE
#endif

#define TRUE ((BOOL) 1)
#define FALSE ((BOOL) 0)
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

/* The following macros can be used to convert to and from standard    */
/* endian. Standard endian is defined as BIG ENDIAN in the middleware. */
/* Note that one, and only one, of the definitions _CPU_BIG_ENDIAN_ or */
/* _CPU_LITTLE_ENDIAN_ must be set.                                    */
#ifndef __BIG_ENDIAN
#ifndef _CPU_LITTLE_ENDIAN_
#define _CPU_LITTLE_ENDIAN_        1
#undef _CPU_BIG_ENDIAN_
#endif
#else
#define _CPU_BIG_ENDIAN_
#undef _CPU_LITTLE_ENDIAN_
#endif

/* The following macros swap 16, 32 or 64 bit values from big */
/* to little or from little to big endian.                    */
#define SWAP_END_16(_val)                             \
    (((((UINT16) _val) & ((UINT16) 0x00ff)) << 8)  |  \
     ((((UINT16) _val) & ((UINT16) 0xff00)) >> 8))

#define SWAP_END_32(_val)                                  \
    (((((UINT32) _val) & ((UINT32) 0x000000ff)) << 24)  |  \
     ((((UINT32) _val) & ((UINT32) 0x0000ff00)) <<  8)  |  \
     ((((UINT32) _val) & ((UINT32) 0x00ff0000)) >>  8)  |  \
     ((((UINT32) _val) & ((UINT32) 0xff000000)) >> 24))

#define SWAP_END_64(_val)                                             \
    (((((UINT64) _val) & ((UINT64) 0x00000000000000ffLL)) << 56)   |  \
     ((((UINT64) _val) & ((UINT64) 0x000000000000ff00LL)) << 40)   |  \
     ((((UINT64) _val) & ((UINT64) 0x0000000000ff0000LL)) << 24)   |  \
     ((((UINT64) _val) & ((UINT64) 0x00000000ff000000LL)) <<  8)   |  \
     ((((UINT64) _val) & ((UINT64) 0x000000ff00000000LL)) >>  8)   |  \
     ((((UINT64) _val) & ((UINT64) 0x0000ff0000000000LL)) >> 24)   |  \
     ((((UINT64) _val) & ((UINT64) 0x00ff000000000000LL)) >> 40)   |  \
     ((((UINT64) _val) & ((UINT64) 0xff00000000000000LL)) >> 56))


/* The following macros return a 16, 32 or 64 bit value given a reference */
/* to a memory location and the endian of the data representation.        */
#define GET_UINT16_FROM_PTR_BIG_END(_ptr)      \
    ((((UINT16) (((UINT8*) _ptr) [0])) << 8) | \
     ((UINT16)  (((UINT8*) _ptr) [1])))

#define GET_UINT32_FROM_PTR_BIG_END(_ptr)       \
    ((((UINT32) (((UINT8*) _ptr) [0])) << 24) | \
     (((UINT32) (((UINT8*) _ptr) [1])) << 16) | \
     (((UINT32) (((UINT8*) _ptr) [2])) <<  8) | \
     ((UINT32)  (((UINT8*) _ptr) [3])))

#define GET_UINT64_FROM_PTR_BIG_END(_ptr)       \
    ((((UINT64) (((UINT8*) _ptr) [0])) << 56) | \
     (((UINT64) (((UINT8*) _ptr) [1])) << 48) | \
     (((UINT64) (((UINT8*) _ptr) [2])) << 40) | \
     (((UINT64) (((UINT8*) _ptr) [3])) << 32) | \
     (((UINT64) (((UINT8*) _ptr) [4])) << 24) | \
     (((UINT64) (((UINT8*) _ptr) [5])) << 16) | \
     (((UINT64) (((UINT8*) _ptr) [6])) <<  8) | \
     ((UINT64)  (((UINT8*) _ptr) [7])))

#define GET_INT16_FROM_PTR_BIG_END(_ptr)  ((INT16) GET_UINT16_FROM_PTR_BIG_END (_ptr))
#define GET_INT32_FROM_PTR_BIG_END(_ptr)  ((INT32) GET_UINT32_FROM_PTR_BIG_END (_ptr))
#define GET_INT64_FROM_PTR_BIG_END(_ptr)  ((INT64) GET_UINT64_FROM_PTR_BIG_END (_ptr))

#define GET_UINT16_FROM_PTR_LITTLE_END(_ptr)   \
    ((((UINT16) (((UINT8*) _ptr) [1])) << 8) | \
     ((UINT16)  (((UINT8*) _ptr) [0])))

#define GET_UINT32_FROM_PTR_LITTLE_END(_ptr)    \
    ((((UINT32) (((UINT8*) _ptr) [3])) << 24) | \
     (((UINT32) (((UINT8*) _ptr) [2])) << 16) | \
     (((UINT32) (((UINT8*) _ptr) [1])) <<  8) | \
     ((UINT32)  (((UINT8*) _ptr) [0])))

#define GET_UINT64_FROM_PTR_LITTLE_END(_ptr)    \
    ((((UINT64) (((UINT8*) _ptr) [7])) << 56) | \
     (((UINT64) (((UINT8*) _ptr) [6])) << 48) | \
     (((UINT64) (((UINT8*) _ptr) [5])) << 40) | \
     (((UINT64) (((UINT8*) _ptr) [4])) << 32) | \
     (((UINT64) (((UINT8*) _ptr) [3])) << 24) | \
     (((UINT64) (((UINT8*) _ptr) [2])) << 16) | \
     (((UINT64) (((UINT8*) _ptr) [1])) <<  8) | \
     ((UINT64)  (((UINT8*) _ptr) [0])))

#define GET_INT16_FROM_PTR_LITTLE_END(_ptr)  ((INT16) GET_UINT16_FROM_PTR_LITTLE_END (_ptr))
#define GET_INT32_FROM_PTR_LITTLE_END(_ptr)  ((INT32) GET_UINT32_FROM_PTR_LITTLE_END (_ptr))
#define GET_INT64_FROM_PTR_LITTLE_END(_ptr)  ((INT64) GET_UINT64_FROM_PTR_LITTLE_END (_ptr))

#define GET_UINT16_FROM_PTR_STD_END(_ptr)  GET_UINT16_FROM_PTR_BIG_END (_ptr)
#define GET_UINT32_FROM_PTR_STD_END(_ptr)  GET_UINT32_FROM_PTR_BIG_END (_ptr)
#define GET_UINT64_FROM_PTR_STD_END(_ptr)  GET_UINT64_FROM_PTR_BIG_END (_ptr)
#define GET_INT16_FROM_PTR_STD_END(_ptr)   ((INT16) GET_UINT16_FROM_PTR_BIG_END (_ptr))
#define GET_INT32_FROM_PTR_STD_END(_ptr)   ((INT32) GET_UINT32_FROM_PTR_BIG_END (_ptr))
#define GET_INT64_FROM_PTR_STD_END(_ptr)   ((INT64) GET_UINT64_FROM_PTR_BIG_END (_ptr))


/* The following macros place a 16, 32 or 64 bit value in big or */
/* little endian format into a memory location.                  */
#define PUT_UINT16_TO_PTR_BIG_END(_val, _ptr)                        \
    ((UINT8*) _ptr) [0] = (UINT8) ((((UINT16) _val) & 0xff00) >> 8); \
    ((UINT8*) _ptr) [1] = (UINT8) (((UINT16)  _val) & 0x00ff);

#define PUT_UINT32_TO_PTR_BIG_END(_val, _ptr)                             \
    ((UINT8*) _ptr) [0] = (UINT8) ((((UINT32) _val) & 0xff000000) >> 24); \
    ((UINT8*) _ptr) [1] = (UINT8) ((((UINT32) _val) & 0x00ff0000) >> 16); \
    ((UINT8*) _ptr) [2] = (UINT8) ((((UINT32) _val) & 0x0000ff00)>>   8); \
    ((UINT8*) _ptr) [3] = (UINT8) (((UINT32)  _val) & 0x000000ff);

#define PUT_UINT64_TO_PTR_BIG_END(_val, _ptr)                                       \
    ((UINT8*) _ptr) [0] = (UINT8) ((((UINT64) _val) & 0xff00000000000000LL) >> 56); \
    ((UINT8*) _ptr) [1] = (UINT8) ((((UINT64) _val) & 0x00ff000000000000LL) >> 48); \
    ((UINT8*) _ptr) [2] = (UINT8) ((((UINT64) _val) & 0x0000ff0000000000LL) >> 40); \
    ((UINT8*) _ptr) [3] = (UINT8) ((((UINT64) _val) & 0x000000ff00000000LL) >> 32); \
    ((UINT8*) _ptr) [4] = (UINT8) ((((UINT64) _val) & 0x00000000ff000000LL) >> 24); \
    ((UINT8*) _ptr) [5] = (UINT8) ((((UINT64) _val) & 0x0000000000ff0000LL) >> 16); \
    ((UINT8*) _ptr) [6] = (UINT8) ((((UINT64) _val) & 0x000000000000ff00LL) >>  8); \
    ((UINT8*) _ptr) [7] = (UINT8) (((UINT64)  _val) & 0x00000000000000ffLL);

#define PUT_INT16_TO_PTR_BIG_END(_val, _ptr)  PUT_UINT16_TO_PTR_BIG_END (_val, _ptr)
#define PUT_INT32_TO_PTR_BIG_END(_val, _ptr)  PUT_UINT32_TO_PTR_BIG_END (_val, _ptr)
#define PUT_INT64_TO_PTR_BIG_END(_val, _ptr)  PUT_UINT64_TO_PTR_BIG_END (_val, _ptr)

#define PUT_UINT16_TO_PTR_LITTLE_END(_val, _ptr)                     \
    ((UINT8*) _ptr) [1] = (UINT8) ((((UINT16) _val) & 0xff00) >> 8); \
    ((UINT8*) _ptr) [0] = (UINT8) (((UINT16)  _val) & 0x00ff);

#define PUT_UINT32_TO_PTR_LITTLE_END(_val, _ptr)                          \
    ((UINT8*) _ptr) [3] = (UINT8) ((((UINT32) _val) & 0xff000000) >> 24); \
    ((UINT8*) _ptr) [2] = (UINT8) ((((UINT32) _val) & 0x00ff0000) >> 16); \
    ((UINT8*) _ptr) [1] = (UINT8) ((((UINT32) _val) & 0x0000ff00)>>   8); \
    ((UINT8*) _ptr) [0] = (UINT8) (((UINT32)  _val) & 0x000000ff);

#define PUT_UINT64_TO_PTR_LITTLE_END(_val, _ptr)                                    \
    ((UINT8*) _ptr) [7] = (UINT8) ((((UINT64) _val) & 0xff00000000000000LL) >> 56); \
    ((UINT8*) _ptr) [6] = (UINT8) ((((UINT64) _val) & 0x00ff000000000000LL) >> 48); \
    ((UINT8*) _ptr) [5] = (UINT8) ((((UINT64) _val) & 0x0000ff0000000000LL) >> 40); \
    ((UINT8*) _ptr) [4] = (UINT8) ((((UINT64) _val) & 0x000000ff00000000LL) >> 32); \
    ((UINT8*) _ptr) [3] = (UINT8) ((((UINT64) _val) & 0x00000000ff000000LL) >> 24); \
    ((UINT8*) _ptr) [2] = (UINT8) ((((UINT64) _val) & 0x0000000000ff0000LL) >> 16); \
    ((UINT8*) _ptr) [1] = (UINT8) ((((UINT64) _val) & 0x000000000000ff00LL) >>  8); \
    ((UINT8*) _ptr) [0] = (UINT8) (((UINT64)  _val) & 0x00000000000000ffLL);

#define PUT_INT16_TO_PTR_LITTLE_END(_val, _ptr)  PUT_UINT16_TO_PTR_LITTLE_END (_val, _ptr)
#define PUT_INT32_TO_PTR_LITTLE_END(_val, _ptr)  PUT_UINT32_TO_PTR_LITTLE_END (_val, _ptr)
#define PUT_INT64_TO_PTR_LITTLE_END(_val, _ptr)  PUT_UINT64_TO_PTR_LITTLE_END (_val, _ptr)

#define PUT_UINT16_TO_PTR_STD_END(_val, _ptr)  PUT_UINT16_TO_PTR_BIG_END (_val, _ptr)
#define PUT_UINT32_TO_PTR_STD_END(_val, _ptr)  PUT_UINT32_TO_PTR_BIG_END (_val, _ptr)
#define PUT_UINT64_TO_PTR_STD_END(_val, _ptr)  PUT_UINT64_TO_PTR_BIG_END (_val, _ptr)
#define PUT_INT16_TO_PTR_STD_END(_val, _ptr)   PUT_UINT16_TO_PTR_BIG_END (_val, _ptr)
#define PUT_INT32_TO_PTR_STD_END(_val, _ptr)   PUT_UINT32_TO_PTR_BIG_END (_val, _ptr)
#define PUT_INT64_TO_PTR_STD_END(_val, _ptr)   PUT_UINT64_TO_PTR_BIG_END (_val, _ptr)


#if !defined (_NO_TYPEDEF_HANDLE_T_) && !defined (_TYPEDEF_HANDLE_T_)
typedef UINT32  HANDLE_T;
#endif

#if !defined (NULL_HANDLE)
#define NULL_HANDLE  ((HANDLE_T) 0)
#endif

#define AST_OK           0
#define AST_FAIL        -1

//#define TRUE            1
//#define FALSE           0

#define DBG_INFO    printf
#define DBG_ERROR   printf

#define AST_TAG "<AST>"
#define AST_INFO(fmt, args...) do{DBG_INFO("%s[%s:%d]:" fmt, AST_TAG, __FUNCTION__, __LINE__, ##args);}while(0)
#define AST_ERR(fmt, args...)  do{DBG_ERROR("%s[%s:%d]:" fmt, AST_TAG, __FUNCTION__, __LINE__, ##args);}while(0)
#define AST_MSG(fmt, args...)  do{DBG_ERROR("%s[%s:%d]:" fmt, AST_TAG, __FUNCTION__, __LINE__, ##args);}while(0)

#define AST_ASSERT(cond) \
        do { \
            if (!cond) { \
                AST_ERR("%s is wrong!\n", #cond); \
                abort(); \
            } \
        } while(0)

typedef void* handle_t;
typedef char int8;
typedef unsigned char uint8;
typedef short int16;
typedef unsigned short uint16;
typedef int int32;
typedef unsigned int uint32;
typedef long long int64;
typedef unsigned long long uint64;
//typedef int BOOL;

#ifdef __cplusplus
}
#endif

#endif
