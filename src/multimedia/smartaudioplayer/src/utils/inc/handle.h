/********************************************************************************************
 *     LEGAL DISCLAIMER
 *
 *     (Header of MediaTek Software/Firmware Release or Documentation)
 *
 *     BY OPENING OR USING THIS FILE, BUYER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 *     THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE") RECEIVED
 *     FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO BUYER ON AN "AS-IS" BASIS
 *     ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES, EXPRESS OR IMPLIED,
 *     INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR
 *     A PARTICULAR PURPOSE OR NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY
 *     WHATSOEVER WITH RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 *     INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND BUYER AGREES TO LOOK
 *     ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. MEDIATEK SHALL ALSO
 *     NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE RELEASES MADE TO BUYER'S SPECIFICATION
 *     OR TO CONFORM TO A PARTICULAR STANDARD OR OPEN FORUM.
 *
 *     BUYER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND CUMULATIVE LIABILITY WITH
 *     RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION,
 *     TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE
 *     FEES OR SERVICE CHARGE PAID BY BUYER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 *     THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE WITH THE LAWS
 *     OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF LAWS PRINCIPLES.
 ************************************************************************************************/
/*-----------------------------------------------------------------------------
 * $RCSfile: u_handle.h,v $
 * $Revision: #1 $
 * $Date: 2016/03/07 $
 * $Author: bdbm01 $
 *
 * Description:
 *         This header file contains handle specific definitions, which are
 *         exported.
 *---------------------------------------------------------------------------*/

#ifndef _HANDLE_H_
#define _HANDLE_H_


/*-----------------------------------------------------------------------------
                    include files
-----------------------------------------------------------------------------*/

#include "common.h"


/*-----------------------------------------------------------------------------
                    macros, defines, typedefs, enums
 ----------------------------------------------------------------------------*/

/* Specify handle data types */
#if !defined (_NO_TYPEDEF_HANDLE_T_) && !defined (_TYPEDEF_HANDLE_T_)
typedef void* HANDLE_T;

#define _TYPEDEF_HANDLE_T_
#endif

typedef UINT16  HANDLE_TYPE_T;

#if !defined (NULL_HANDLE)
#define NULL_HANDLE  ((HANDLE_T) 0)
#endif

#define INV_HANDLE_TYPE  ((HANDLE_TYPE_T) 0)

/* Handle API return values */
#define HR_OK                   ((INT32)   0)
#define HR_INV_ARG              ((INT32)  -1)
#define HR_INV_HANDLE           ((INT32)  -2)
#define HR_OUT_OF_HANDLES       ((INT32)  -3)
#define HR_NOT_ENOUGH_MEM       ((INT32)  -4)
#define HR_ALREADY_INIT         ((INT32)  -5)
#define HR_NOT_INIT             ((INT32)  -6)
#define HR_RECURSION_ERROR      ((INT32)  -7)
#define HR_NOT_ALLOWED          ((INT32)  -8)
#define HR_ALREADY_LINKED       ((INT32)  -9)
#define HR_NOT_LINKED           ((INT32) -10)
#define HR_FREE_NOT_ALLOWED     ((INT32) -11)
#define HR_INV_AUX_HEAD         ((INT32) -12)
#define HR_INV_HANDLE_TYPE      ((INT32) -13)
#define HR_CANNOT_REG_WITH_CLI  ((INT32) -14)


/* Specify handle groups */
#define HT_GROUP_SIZE	 ((HANDLE_TYPE_T) 64)
#define HT_GROUP_AEE      ((HANDLE_TYPE_T) (1 * HT_GROUP_SIZE))  /* AEE Manager */
#define HT_GROUP_COMMON ((HANDLE_TYPE_T) (2 * HT_GROUP_SIZE)) 
/* A handle type of value '0' will be reserved as invalid handle. */
/* The first HT_GROUP_SIZE handle type values are reserved by the */
/* handle library. The first group starts at value HT_GROUP_SIZE. */
#define HT_RES_BY_HANDLE_LIB  HT_GROUP_SIZE // Reserved handle group
#define HT_NB_GROUPS  (2)

/* Handle types which are recognized and treated in a special manner by the */
/* handle librray MUST fall between the values '1' and the value specified  */
/* by the macro HT_RES_BY_HANDLE_LIB.                                       */
#define HT_AUX_LINK_HEAD  ((HANDLE_TYPE_T)  1)

/* Return values of (*handle_parse_fct) */
typedef enum
{
    HP_NEXT,
    HP_BREAK,
    HP_RESTART
}   HP_TYPE_T;


/* Handle link structure */
typedef struct _HANDLE_LINK_T
{
    UINT32  ui4_data [2];
}   HANDLE_LINK_T;


/* Callback functions */
typedef BOOL (*handle_free_fct) (HANDLE_T       h_handle,
                                 HANDLE_TYPE_T  e_type,
                                 VOID*          pv_obj,
                                 VOID*          pv_tag,
                                 BOOL           b_req_handle);
typedef VOID (*handle_aux_free_fct) (VOID*  pv_obj);
typedef HP_TYPE_T (*handle_parse_fct) (UINT16         ui2_count,
                                       UINT16         ui2_max_count,
                                       HANDLE_T       h_handle,
                                       HANDLE_TYPE_T  e_type,
                                       VOID*          pv_obj,
                                       VOID*          pv_tag,
                                       VOID*          pv_parse_data);

typedef INT32 (*handle_autofree_fct) (HANDLE_T       h_handle,
                                      HANDLE_TYPE_T  e_type);


/*-----------------------------------------------------------------------------
                    functions declarations
 ----------------------------------------------------------------------------*/

extern INT32 u_handle_alloc (HANDLE_TYPE_T    e_type,
                           VOID*            pv_obj,
                           VOID*            pv_tag,
                           handle_free_fct  pf_free,
                           HANDLE_T*        ph_handle);

extern INT32 u_handle_delink (HANDLE_LINK_T*  pt_h_link,
                            HANDLE_T        h_handle);
extern INT32 u_handle_free (HANDLE_T  h_handle,
                          BOOL      b_req_handle);
extern INT32 u_handle_free_all (HANDLE_LINK_T*  pt_h_link);

extern INT32 u_handle_get_type_obj (HANDLE_T        h_handle,
                                  HANDLE_TYPE_T*  pe_type,
                                  VOID**          ppv_obj);

extern INT32 u_handle_set_obj (HANDLE_T  h_handle,
                             VOID*     pv_obj);

extern INT32 u_handle_usr_init (UINT16   ui2_num_handles);
extern INT32 u_handle_usr_uninit(VOID);  

extern INT32 u_handle_link (HANDLE_LINK_T*  pt_h_link,
                          HANDLE_T        h_handle);



/* Bug-14  2004-12-07  ffr. Add API to handle secondary linked handles. */
extern INT32 u_handle_alloc_aux_link_head (VOID*                pv_obj,
                                         handle_aux_free_fct  pf_aux_free,
                                         HANDLE_T*            ph_aux_head);

extern INT32 u_handle_link_to_aux (HANDLE_T  h_aux_head,
                                 HANDLE_T  h_handle);

extern INT32 u_handle_delink_from_aux (HANDLE_T  h_handle);
extern INT32 u_handle_next_aux_linked (HANDLE_T   h_curr_handle,
                                     HANDLE_T*  ph_next_handle);

extern INT32 u_handle_set_autofree_tbl (HANDLE_TYPE_T         e_group,
                                      handle_autofree_fct*  pf_autofree_fcts);

extern VOID u_handle_autofree (HANDLE_T  h_handle);


#endif /* _U_HANDLE_H_ */
