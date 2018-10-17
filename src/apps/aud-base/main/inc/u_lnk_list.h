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


/*----------------------------------------------------------------------------*
 * $RCSfile: u_lnk_list.h,v $
 * $Revision: #1 $
 * $Date: 2016/07/22 $
 * $Author: bdbm01 $
 * $CCRevision: /main/DTV_X_HQ_int/2 $
 * $SWAuthor: Iolo Tsai $
 * $MD5HEX: aa23ebe9cc41adcdeac276369d5e7768 $
 *
 * Description: 
 *         This header file contains single and double link list macros.
 *---------------------------------------------------------------------------*/

#ifndef _U_LNK_LIST_H_
#define _U_LNK_LIST_H_

/*-----------------------------------------------------------------------------
                    include files
-----------------------------------------------------------------------------*/


/*-----------------------------------------------------------------------------
                    macros, defines, typedefs, enums
-----------------------------------------------------------------------------*/

/*
 *  Double link list
 *
 *          pt_head
 *         ---------------|
 *         |              v
 *  -----------           -------------           -------------
 *  |         |           |           |  pt_next  |           |  pt_next
 *  |         |           |  DLIST_   |---------->|  DLIST_   |-----> NULL
 *  | DLIST_T |           |  ENTRY_T  |           |  ENTRY_T  |
 *  |         | NULL <----|           |<----------|           |
 *  |         |  pt_prev  |           |  pt_prev  |           |
 *  -----------           -------------           -------------
 *       |                                        ^
 *       -----------------------------------------|
 *          pt_tail
 *
 */
#define DLIST_T(type)           \
    struct {                    \
        struct type *pt_head;   \
        struct type *pt_tail;   \
    }
    
#define DLIST_ENTRY_T(type)     \
    struct {                    \
        struct type *pt_prev;   \
        struct type *pt_next;   \
    }
    
#define DLIST_HEAD(q) ((q)->pt_head)

#define DLIST_TAIL(q) ((q)->pt_tail)

#define DLIST_IS_EMPTY(q) ((q)->pt_head == NULL)

#define DLIST_NEXT(ent, field)  ((ent)->field.pt_next)

#define DLIST_PREV(ent, field)  ((ent)->field.pt_prev)

#define DLIST_INIT(q)                       \
    do                                      \
    {                                       \
        (q)->pt_head = (q)->pt_tail = NULL; \
    } while(0)
 
#define DLIST_FOR_EACH(var, q, field)       \
    for ((var) = (q)->pt_head;              \
         (var);                             \
         (var) = (var)->field.pt_next)
         
#define DLIST_FOR_EACH_BACKWARD(var, q, field)  \
    for ((var) = (q)->pt_tail;                  \
         (var);                                 \
         (var) = (var)->field.pt_prev)

#define DLIST_INSERT_HEAD(ent, q, field)    \
    do                                      \
    {                                       \
        (ent)->field.pt_prev = NULL;                        \
        if (((ent)->field.pt_next = (q)->pt_head) == NULL)  \
        {                                                   \
            (q)->pt_tail = (ent);                           \
        }                                                   \
        else                                                \
        {                                                   \
            ((q)->pt_head)->field.pt_prev = (ent);          \
        }                                                   \
        (q)->pt_head = (ent);                               \
    } while(0)
    
#define DLIST_INSERT_TAIL(ent, q, field)                    \
    do                                                      \
    {                                                       \
        (ent)->field.pt_next = NULL;                        \
        if (((ent)->field.pt_prev = (q)->pt_tail) == NULL)  \
        {                                                   \
            (q)->pt_head = (ent);                           \
        }                                                   \
        else                                                \
        {                                                   \
            ((q)->pt_tail)->field.pt_next = (ent);          \
        }                                                   \
        (q)->pt_tail = (ent);                               \
    } while(0)

#define DLIST_INSERT_BEFORE(new, old, q, field)             \
    do                                                      \
    {                                                       \
        (new)->field.pt_next = (old);                       \
        if (((new)->field.pt_prev = (old)->field.pt_prev) == NULL)  \
        {                                                   \
            (q)->pt_head = (new);                           \
        }                                                   \
        else                                                \
        {                                                   \
            ((old)->field.pt_prev)->field.pt_next = (new);  \
        }                                                   \
        (old)->field.pt_prev = (new);                       \
    } while(0)


#define DLIST_INSERT_AFTER(new, old, q, field)              \
    do                                                      \
    {                                                       \
        (new)->field.pt_prev = (old);                       \
        if (((new)->field.pt_next = (old)->field.pt_next) == NULL)  \
        {                                                   \
            (q)->pt_tail = (new);                           \
        }                                                   \
        else                                                \
        {                                                   \
            ((old)->field.pt_next)->field.pt_prev = (new);  \
        }                                                   \
        (old)->field.pt_next = (new);                       \
    } while(0)
    
#define DLIST_REMOVE(ent, q, field)                         \
    do                                                      \
    {                                                       \
        if ((q)->pt_tail == (ent))                          \
        {                                                   \
            (q)->pt_tail = (ent)->field.pt_prev;            \
        }                                                   \
        else                                                \
        {                                                   \
            ((ent)->field.pt_next)->field.pt_prev = (ent)->field.pt_prev;   \
        }                                                   \
        if ((q)->pt_head == (ent))                          \
        {                                                   \
            (q)->pt_head = (ent)->field.pt_next;            \
        }                                                   \
        else if (NULL != (ent)->field.pt_prev)              \
        {                                                   \
            ((ent)->field.pt_prev)->field.pt_next = (ent)->field.pt_next;   \
        }                                                                    \
        if((ent) != NULL)                                   \
        {                                                    \
            (ent)->field.pt_next = NULL;                     \
            (ent)->field.pt_prev = NULL;                     \
        }                                                     \
    } while(0)

 
/*
 *  Single link list
 *
 *              pt_head
 *            ------------|
 *            |^          v
 *  -----------|          -------------           -------------
 *  |         ||          |           |  pt_next  |           |  pt_next
 *  |         ||          |  SLIST_   |---------->|  SLIST_   |-----> NULL
 *  | SLIST_T ||          |  ENTRY_T  |^          |  ENTRY_T  |
 *  |         |-----------|           |-----------|           |
 *  |         |  pt_prev  |           |  pt_prev  |           |
 *  -----------           -------------           -------------
 *
 */
#define SLIST_T(type)           \
    struct {                    \
        struct type *pt_head;   \
    }

#define SLIST_ENTRY_T(type)     \
    struct {                    \
        struct type *pt_next;   \
        struct type **pt_prev;  /* address of previous pt_next */ \
    }
    
#define SLIST_FIRST(list)       ((list)->pt_head)

#define SLIST_IS_EMPTY(list)    ((list)->pt_head == NULL)

#define	SLIST_NEXT(ent, field)	((ent)->field.pt_next)

#define SLIST_INIT(list)            \
    {                               \
        ((list)->pt_head = NULL);   \
    }

#define SLIST_FOR_EACH(var, list, field)    \
    for ((var) = (list)->pt_head;           \
         (var);                             \
         (var) = (var)->field.pt_next)
         
#define SLIST_INSERT_HEAD(new, list, field)                             \
    do                                                                  \
    {                                                                   \
        if (((new)->field.pt_next = (list)->pt_head) != NULL)           \
        {                                                               \
            ((list)->pt_head)->field.pt_prev = &((new)->field.pt_next); \
        }                                                               \
        (list)->pt_head = (new);                                        \
        (new)->field.pt_prev = &((list)->pt_head);                      \
    } while (0)
    
#define SLIST_INSERT_AFTER(new, old, field)                         \
    do                                                              \
    {                                                               \
        if (((new)->field.pt_next = (old)->field.pt_next) != NULL)  \
        {                                                           \
            ((old)->field.pt_next)->field.pt_prev = &((new)->field.pt_next); \
        }                                                           \
        (old)->field.pt_next = (new);                               \
        (new)->field.pt_prev = &((old)->field.pt_next);             \
    } while (0)
    

#define SLIST_INSERT_BEFORE(new, old, field)                        \
    do                                                              \
    {                                                               \
        (new)->field.pt_next = (old);                               \
        (new)->field.pt_prev = (old)->field.pt_prev;                \
        *((old)->field.pt_prev) = (new);                            \
        (old)->field.pt_prev = &((new)->field.pt_next);             \
    } while (0)


#define SLIST_REMOVE(ent, field)                                            \
    do                                                                      \
    {                                                                       \
        *((ent)->field.pt_prev) = (ent)->field.pt_next;                     \
        if ((ent)->field.pt_next != NULL)                                   \
        {                                                                   \
            ((ent)->field.pt_next)->field.pt_prev = (ent)->field.pt_prev;   \
        }                                                                   \
        (ent)->field.pt_next = NULL;                                        \
        (ent)->field.pt_prev = NULL;                                        \
    } while (0)


#endif /* _X_LNK_LIST_H_ */
