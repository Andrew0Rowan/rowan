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

/*-----------------------------------------------------------------------
 * This file is for object packing and address tranlsation.
 *
 * Argument package:
 *
 * Basic      +---------------+ <---------------- pv_pack_buff
 *             | State         |
 *             | OP string     |
 *             | Callback Addr |
 *             | Return        |
 *             | Arg count     |
 *             | Element TOP   |
 *  Arguments  +---------------+ <---------------- ARG start
 *             | Argument1     |
 *             | Argument2     |
 *             ~               ~
 *             ~               ~
 *             | ArgumentN     |
 *             +---------------+ <---------------- ARG ex desc
 *             |               |
 *             | Arg1 ex desc  |
 *             ~               ~
 *            ~               ~
 *             | ArgN ex desc  |
 *  Elements   +---------------+ <---------------- Element start.
 *             | ELEM SIZE     |--- Total size of element.
 *             | DESC for Ref1 |--- Block description
 *             | DATA          |--- Pointer refrenced data.
 *             +---------------+
 *             | ELEM SIZE     |
 *             | DESC for Ref2 |
 *             | DATA          |
 *             +---------------+
 *             | ELEM          |
 *             ~               ~
 *             ~               ~
 *             | ELEM          |
 *             +---------------+ <----------------- pv_pack_buff + Element end
 *             |               |
 *             ~ ~ ~ ~ ~ ~ ~ ~ ~
 *
 *
 * DATA & ARGS :
 *
 * 1, Pack the data together in the object graph from 'root' (Arguments).
 *    using DFS/BFS, which goes through each objects and copy its data to package.
 *    And the reference in data and arguments will be modified to be
 *    offset, which is the data address in package substracts package base.
 *
 * 2, Translate the data by resolving offset to local address. The address is
 *    package base + offset. Then a new object graph is rebuilt in receiver site.
 *    We can pass it to receiver's local APIs.
 *
 *
 * TODO : Add return package here:
 *
 * 3, Restore the data by rewriting local address to be RAW reference if data will be
 *    updated to caller local memory. These data with return value will be packed in
 *    return package.
 *
 * 4, Writeback address for output from return package.
 *
 * ---------------------------------------------------------------------*/

/*------------------------------------------------------------------------
                    include files
 -----------------------------------------------------------------------*/
#include <string.h>
#include "_rpc_ipc_util.h"
#include "_ipc_packer.h"
/*------------------------------------------------------------------------
                    macros, defines, typedefs, enums
 -----------------------------------------------------------------------*/
extern BOOL bt_rpcu_tl_log_is_on();

#define MAX_OP_SIZE 64

#define BLK_DESC_SIZE(pv_desc) \
        (SIZE_T)(\
            sizeof(BLK_DESC_T) \
          + ((BLK_DESC_T *)(pv_desc))->ui4_num_entries * sizeof(BLK_DESC_ENTRY_T)\
        )

#define BLK_DESC_GET_ENTRY_START(pt_desc) \
        ((BLK_DESC_ENTRY_T *)((BLK_DESC_T *)(pt_desc) + 1))

#define FOREACH_BLK_ENTRY(pt_blk_desc, pt_blk_entry) \
        for((pt_blk_entry) = BLK_DESC_GET_ENTRY_START((pt_blk_desc));\
            (UINT32)((pt_blk_entry) - BLK_DESC_GET_ENTRY_START((pt_blk_desc))) \
                < (pt_blk_desc)->ui4_num_entries;\
            (pt_blk_entry) ++)

#define PACK_REF_BASIC(pv_pack_buff) ((PACK_BASIC_T *)pv_pack_buff)

#define PACK_REF_ARG_BASE(pv_pack_buff) \
    ((ARG_DESC_T *)(\
        (CHAR *)PACK_REF_BASIC((pv_pack_buff)) + sizeof(PACK_BASIC_T)\
    ))

#define PACK_REF_ARG_END(pv_pack_buff) \
    ((ARG_DESC_T *)(\
        (CHAR *)PACK_REF_ARG_BASE((pv_pack_buff))\
    +   (PACK_REF_BASIC((pv_pack_buff)))->ui4_num_args * sizeof(ARG_DESC_T)\
    ))

#define PACK_REF_ARG_EX_DESC_BASE(pv_pack_buff) \
    ((ARG_EX_DESC_T *)(\
        (CHAR *)PACK_REF_ARG_END((pv_pack_buff))\
    ))

#define PACK_REF_ARG_EX_DESC_END(pv_pack_buff) \
    ((ARG_EX_DESC_T *)(\
        (CHAR *)PACK_REF_ARG_EX_DESC_BASE((pv_pack_buff))\
    +   (PACK_REF_BASIC((pv_pack_buff)))->ui4_num_args * sizeof(ARG_EX_DESC_T)\
    ))


#define PACK_REF_ELEM_BASE(pv_pack_buff) \
    ((VOID *)(\
        (CHAR *)PACK_REF_ARG_EX_DESC_END((pv_pack_buff))\
    ))

#define PACK_REF_ELEM_END(pv_pack_buff) \
    ((VOID *)(\
        (CHAR *)(pv_pack_buff) + PACK_REF_BASIC((pv_pack_buff))->ui4_elem_end \
    ))

#define PACK_FOR_EACH_ELEM(pv_pack_buff, pv_elem, ui4_idx) \
    for((pv_elem) = PACK_REF_ELEM_BASE((pv_pack_buff)), ui4_idx = 0;\
        (CHAR *)pv_elem < (CHAR *)PACK_REF_ELEM_END((pv_pack_buff));\
        (pv_elem) = ELEM_NEXT((pv_elem)), (ui4_idx) ++)


#define ELEM_REF_SIZE(pv_elem) ((SIZE_T *)(pv_elem))

#define ELEM_SIZE(pt_desc) \
    ((SIZE_T)(  sizeof(UINT32) \
              + BLK_DESC_SIZE(pt_desc) \
              + sizeof(VOID *) \
              + (((pt_desc)->z_size + 0x3) & ~0x3))\
    )

#define ELEM_NEXT(pv_elem) \
    ((VOID *)(\
        ((CHAR *)((pv_elem)) + *ELEM_REF_SIZE((pv_elem)))\
    ))

#define ELEM_REF_DESC(pv_elem) \
    ((BLK_DESC_T *)((CHAR *)ELEM_REF_SIZE((pv_elem)) + sizeof(SIZE_T)))

#define ELEM_REF_DATA(pv_elem) \
    ((VOID **) \
         (\
             (CHAR *)ELEM_REF_DESC((pv_elem)) \
           + BLK_DESC_SIZE(ELEM_REF_DESC((pv_elem)))\
         )\
    )



#define PACK_FOREACH_ARG(pv_pack_buff, ui4_arg_idx, pt_arg_itr, pt_arg_ex_desc) \
for((ui4_arg_idx)    = 0, \
    (pt_arg_itr)     = PACK_REF_ARG_BASE((pv_pack_buff)), \
    (pt_arg_ex_desc) = PACK_REF_ARG_EX_DESC_BASE((pv_pack_buff));\
    (ui4_arg_idx)    < PACK_REF_BASIC((pv_pack_buff))->ui4_num_args;\
    (ui4_arg_idx) ++, (pt_arg_itr) ++, (pt_arg_ex_desc) ++)

/*#define IPC_MAKE_OFFSET(pv_base, z_off) ((VOID *)((CHAR *)(pv_base) + (z_off)))*/



#define RET_PACK_REF_RETURN(pv_ret_pack_buff)    ((ARG_DESC_T *)(pv_ret_pack_buff))
#define RET_PACK_REF_BLK_COUNT(pv_ret_pack_buff) ((CHAR *)RET_PACK_REF_RETURN(pv_ret_pack_buff) + sizeof(ARG_DESC_T))
#define RET_PACK_REF_BLK_START(pv_ret_pack_buff) ((CHAR *)RET_PACK_REF_BLK_COUNT(pv_ret_pack_buff) + sizeof(UINT32))

#define RET_PACK_BLK_SIZE(z_data_size)  ((SIZE_T)(((z_data_size) + sizeof(VOID *) + sizeof(SIZE_T) + 0x3) & ~0x3))

#define RET_PACK_BLK_REF_RAW(pv_blk)    ((VOID **)(pv_blk))
#define RET_PACK_BLK_REF_SIZE(pv_blk)   ((SIZE_T *)((CHAR *)RET_PACK_BLK_REF_RAW(pv_blk) + sizeof(VOID *)))
#define RET_PACK_BLK_REF_DATA(pv_blk)   ((SIZE_T *)((CHAR *)RET_PACK_BLK_REF_SIZE(pv_blk) + sizeof(SIZE_T)))
#define RET_PACK_BLK_REF_NEXT(pv_blk)   ((VOID *)((CHAR *)(pv_blk) + RET_PACK_BLK_SIZE(*RET_PACK_BLK_REF_SIZE(pv_blk))))



typedef struct _BLK_DESC_T
{
    ARG_TYPE_T      e_type;
    VOID *          pv_start_raw;
    SIZE_T          z_size;
    UINT32          ui4_num_entries;

    /* Entries will be followed this header structure */
    /*BLK_DESC_ENTRY_T  at_desc_entries [];*/
}  BLK_DESC_T;

typedef struct _BLK_DESC_ENTRY_T
{
    ARG_TYPE_T      e_type;
    SIZE_T          z_offs;

    VOID *          pv_raw;

    /* This is the initial data relative addr
     * in message , used to check whether this is modified
     */
    SIZE_T          z_init_offs;

    /* TODO : Make this in union */
    SIZE_T          z_const_size;
}  BLK_DESC_ENTRY_T;

typedef struct _PACK_BASIC_T
{
    CHAR         ps_op[MAX_OP_SIZE];
    VOID *       pv_cb_addr;
/*    ARG_DESC_T   t_return;*/
    UINT32       ui4_num_args;
    UINT32       ui4_elem_end;
}PACK_BASIC_T;


typedef struct _ARG_EX_DESC_T
{
    VOID *  pv_raw;
}ARG_EX_DESC_T;


/*------------------------------------------------------------------------
                    functions declarations
 -----------------------------------------------------------------------*/
static VOID * _push_elem(VOID * pv_pack_buff, SIZE_T z_max_size, BLK_DESC_T * pt_desc);


static VOID * _find_elem_by_raw(VOID * pv_pack_buff, VOID * pv_raw_addr);

static VOID * _find_elem_by_data(VOID * pv_pack_buff, VOID * pv_data);

static BLK_DESC_T * _get_blk_desc(
    ARG_TYPE_T              t_arg_type,
    VOID *                  pv_addr,
    ipc_get_rpc_desc_fct    pf_get_rpc_desc,
    VOID *                  pv_tag);

static VOID _free_blk_desc(BLK_DESC_T * pt_desc);

static SIZE_T _pack_arg_ref(
    VOID *                      pv_pack_buff,
    SIZE_T                      z_max_size,
    ARG_DESC_T *                pt_arg,
    ipc_get_rpc_desc_fct        pf_get_rpc_desc,
    VOID *                      pv_tag);


static VOID * _get_raw_addr(VOID * pv_pack_buff, VOID * pv_data, BLK_DESC_ENTRY_T * pt_entry);

static VOID _trace_package(VOID * pv_pack_buff);

/*------------------------------------------------------------------------
                    data declarations
 -----------------------------------------------------------------------*/

/*------------------------------------------------------------------------
 * Name:  _push_elem
 *
 * Description: Push a new element to the message buff.
 *
 * Inputs:  pv_pack_buff:    Buff start address
 *          z_max_size: Buff size range
 *          pv_addr:    Raw address
 *          pt_desc:    Block description
 *
 * Outputs: -
 *
 * Returns: Element start address.
 -----------------------------------------------------------------------*/
static VOID *
_push_elem(
    VOID *          pv_pack_buff,
    SIZE_T          z_max_size,
    BLK_DESC_T *    pt_desc)
{
    VOID *              pv_elem;
    PACK_BASIC_T *   pt_basic;
    VOID *              pv_elem_top;
    SIZE_T              z_elem_size;

    pt_basic     = PACK_REF_BASIC(pv_pack_buff);
    pv_elem_top  = PACK_REF_ELEM_END(pv_pack_buff);
    pv_elem      = pv_elem_top;

    z_elem_size  = ELEM_SIZE(pt_desc);
    IPC_ASSERT(pt_basic->ui4_elem_end + z_elem_size <= z_max_size);

    pt_basic->ui4_elem_end += z_elem_size;

    memcpy(ELEM_REF_SIZE(pv_elem), &z_elem_size,            sizeof(UINT32));
    memcpy(ELEM_REF_DESC(pv_elem), pt_desc,                 BLK_DESC_SIZE(pt_desc));
    memcpy(ELEM_REF_DATA(pv_elem), pt_desc->pv_start_raw,   pt_desc->z_size);

    RPC_LOG("Pushed ELEM elem:%p size (0x%x)\n",
            pv_elem,
            z_elem_size);

    return pv_elem;
}


/*------------------------------------------------------------------------
 * Name:  _find_elem_by_raw
 *
 * Description:
 *   Find raw addr in message buff.
 *
 *
 * Inputs:  pv_pack_buff:     Buff start address
 *          pv_raw_addr: Raw address
 *
 * Outputs: -
 *
 * Returns: Element whose block region convers raw address passed.
 -----------------------------------------------------------------------*/
static VOID *
_find_elem_by_raw(VOID * pv_pack_buff, VOID * pv_raw_addr)
{
    VOID *          pv_elem;
    UINT32          ui4_i;
    BLK_DESC_T *    pt_desc;
    /*
     * This is slow. Here packer could build another lookup
     * structure but that would be a waste. If we have some
     * callback funtions to upper layer like get_tag and
     * set_tag to address in the database, that maybe helpful.
     */
    PACK_FOR_EACH_ELEM(pv_pack_buff, pv_elem, ui4_i)
    {
        pt_desc = ELEM_REF_DESC(pv_elem);
        if(     (CHAR *)pv_raw_addr >=  (CHAR *)pt_desc->pv_start_raw
            &&  (CHAR *)pv_raw_addr <   (CHAR *)pt_desc->pv_start_raw +  pt_desc->z_size)
        {
            return pv_elem;
        }
    }
    return NULL;
}

/*------------------------------------------------------------------------
 * Name:  _find_elem_by_raw
 *
 * Description:
 *   Find data addr in message buff.
 *
 * Inputs:  pv_pack_buff:  Buff start address
 *          pv_data:  Data address in buff
 *
 * Outputs: -
 *
 * Returns: Element whose block region convers data address passed.
 -----------------------------------------------------------------------*/
static VOID *
_find_elem_by_data(VOID * pv_pack_buff, VOID * pv_data)
{
    VOID *              pv_elem;
    UINT32              ui4_i;

    PACK_FOR_EACH_ELEM(pv_pack_buff, pv_elem, ui4_i)
    {
        if(    (CHAR *)pv_data >= (CHAR *)ELEM_REF_DATA(pv_elem)
            && (CHAR *)pv_data <
                      (CHAR *)ELEM_REF_DATA(pv_elem)
                  +   ELEM_REF_DESC(pv_elem)->z_size)
        {
            return pv_elem;
        }
    }
    return NULL;
}

/*------------------------------------------------------------------------
 * Name:  _find_elem_by_raw
 *
 * Description: Get size of reference type.
 *
 * Inputs:
 *
 * Outputs: -
 *
 * Returns: Element whose block region convers data address passed.
 -----------------------------------------------------------------------*/
static SIZE_T _get_size_of(ARG_TYPE_T e_type, VOID * pv_ref)
{
    switch(e_type & ARG_MASK_TYPE)
    {
        case ARG_TYPE_REF_CHAR:
            return sizeof(CHAR);
        case ARG_TYPE_REF_INT8:
            return sizeof(INT8);
        case ARG_TYPE_REF_INT16:
            return sizeof(INT16);
        case ARG_TYPE_REF_INT32:
            return sizeof(INT32);
        case ARG_TYPE_REF_UINT8:
            return sizeof(UINT8);
        case ARG_TYPE_REF_UINT16:
            return sizeof(UINT16);
        case ARG_TYPE_REF_UINT32:
            return sizeof(UINT32);
        case ARG_TYPE_REF_SIZE_T:
            return sizeof(SIZE_T);
        case ARG_TYPE_REF_INT64:
            return sizeof(INT64);
        case ARG_TYPE_REF_UINT64:
            return sizeof(UINT64);
        case ARG_TYPE_REF_BOOL:
            return sizeof(BOOL);
        case ARG_TYPE_REF_STR:
            return strlen(pv_ref) + 1;
        default:
            IPC_ASSERT(0);
    }
	return (SIZE_T)0;
}
/*------------------------------------------------------------------------
 * Name:  _get_blk_desc
 *
 * Description:
 *   Create block description (BLK_DESC_T) from RPC_DESC_T
 *
 * Inputs:  t_arg_type:      Arg type
 *          pv_addr:         Block address
 *          pf_get_rpc_desc: The callback to get RPC_DESC_T
 *          pv_tag         : Tag for callback
 * Outputs: -
 *
 * Returns: Block description or NULL if there is error.
 -----------------------------------------------------------------------*/
static BLK_DESC_T * _get_blk_desc(
    ARG_TYPE_T              t_arg_type,
    VOID *                  pv_addr,
    ipc_get_rpc_desc_fct    pf_get_rpc_desc,
    VOID *                  pv_tag)
{
    RPC_DESC_ENTRY_T *  pt_rpc_entry;
    VOID *              pv_start_raw;
    BLK_DESC_T *        pt_desc;
    BLK_DESC_ENTRY_T *  pt_entry;
    RPC_DESC_T *        pt_rpc_desc;

    if((t_arg_type & ARG_MASK_TYPE) != ARG_TYPE_REF_DESC)
    {
        pt_desc = (BLK_DESC_T *)RPC_MALLOC(sizeof(BLK_DESC_T));
        if(pt_desc != NULL)
        {
            pt_desc->e_type          = ARG_TYPE_STRUCT;
            pt_desc->z_size          = _get_size_of(t_arg_type, pv_addr);
            pt_desc->ui4_num_entries = 0;
            pt_desc->pv_start_raw    = pv_addr;
        }
    }
    else
    {
        pt_rpc_desc = pf_get_rpc_desc(pv_addr, pv_tag, &pv_start_raw);

        if(pt_rpc_desc == NULL)
        {
            return NULL;
        }

        pt_desc = RPC_MALLOC(sizeof(BLK_DESC_T) + sizeof(BLK_DESC_ENTRY_T) * pt_rpc_desc->ui4_num_entries);
        if(pt_desc == NULL)
        {
            return NULL;
        }
        pt_desc->e_type             = ARG_TYPE_STRUCT;
        pt_desc->pv_start_raw       = pv_start_raw;
        pt_desc->z_size             = pt_rpc_desc->z_size;
        pt_desc->ui4_num_entries    = 0;

        FOREACH_RPC_ENTRY(pt_rpc_desc, pt_rpc_entry)
        {
            BLK_DESC_ENTRY_T *  pt_insert_entry;
            BLK_DESC_ENTRY_T *  pt_move_entry;
            //SIZE_T              z_insert_idx = 0;
            FOREACH_BLK_ENTRY(pt_desc, pt_insert_entry)
            {
                if(pt_insert_entry->z_offs > pt_rpc_entry->u.z_offs)
                {
                    pt_move_entry = &BLK_DESC_GET_ENTRY_START(pt_desc)[pt_desc->ui4_num_entries];
                    while(pt_move_entry > pt_insert_entry)
                    {
                        memcpy(pt_move_entry, pt_move_entry - 1, sizeof(BLK_DESC_ENTRY_T));
                        pt_move_entry --;
                    }
                    break;
                }
            }

            pt_entry                = pt_insert_entry;
            pt_entry->e_type        = pt_rpc_entry->e_type;
            pt_entry->z_offs        = pt_rpc_entry->u.z_offs;
            pt_entry->pv_raw        = NULL;
            pt_entry->z_init_offs   = 0;
            /*Fix Me:*/
            pt_entry->z_const_size  = pt_rpc_entry->ui4_num_entries;

            pt_desc->ui4_num_entries ++;
        }
    }


    return pt_desc;
}

/*------------------------------------------------------------------------
 * Name:  _free_blk_desc
 *
 * Description:
 *   Free block description
 *
 * Inputs:  pt_desc:    Created block description by _get_blk_desc
 *
 * Outputs: -
 *
 * Returns:
 -----------------------------------------------------------------------*/
static VOID _free_blk_desc(BLK_DESC_T * pt_desc)
{
    RPC_FREE((VOID *)pt_desc);
}

/*------------------------------------------------------------------------
 * Name:  _pack_arg_ref
 *
 * Description:
 *   Packing from an argument.
 *
 * Inputs:  pv_pack_buff:         Message buff base
 *          z_max_size:      Buff range
 *          pt_arg:          Argument
 *          pf_get_rpc_desc: THe callback to get RPC_DESC_T
 *          pv_tag:          Tag for callback
 * Outputs: -
 *
 * Returns: The offset (Relative addr) from the message base or 0
 *          if it is failed.
 -----------------------------------------------------------------------*/
static SIZE_T _pack_arg_ref(
    VOID *                      pv_pack_buff,
    SIZE_T                      z_max_size,
    ARG_DESC_T *                pt_arg,
    ipc_get_rpc_desc_fct        pf_get_rpc_desc,
    VOID *                      pv_tag)
{
    BLK_DESC_T *            pt_desc;
    BLK_DESC_ENTRY_T *      pt_entry;

    BLK_DESC_T *            pt_desc_chld;

    VOID **                 ppv_chld;
    VOID *                  pv_elem_chld;
    VOID *                  pv_elem;
    SIZE_T                  z_offs;
    SIZE_T                  z_ref_offs;

    if(pt_arg->u.pv_desc == NULL)
    {
        return 0;
    }

    if((pv_elem = _find_elem_by_raw(pv_pack_buff, pt_arg->u.pv_desc)) != NULL)
    {
        pt_desc = ELEM_REF_DESC(pv_elem);
        z_ref_offs = (CHAR *)pt_arg->u.pv_desc - (CHAR *)pt_desc->pv_start_raw;

        /* If the argument is marked with ARG_DIR_OUT,
         * this will mark the whole block by this mask .
         */
        if(pt_arg->e_type & ARG_DIR_OUT)
        {
            pt_desc->e_type |= ARG_DIR_OUT;
        }
        return (SIZE_T)(z_ref_offs + (CHAR *)ELEM_REF_DATA(pv_elem) - (CHAR *)pv_pack_buff);
    }

    pt_desc = _get_blk_desc(pt_arg->e_type,
                            pt_arg->u.pv_desc,
                            pf_get_rpc_desc,
                            pv_tag);

    if(pt_desc != NULL)
    {
        /* If the argument is marked with ARG_DIR_OUT,
         * this will mark the whole block by this mask.
         */
        if(pt_arg->e_type & ARG_DIR_OUT)
        {
            pt_desc->e_type |= ARG_DIR_OUT;
        }

        pv_elem = _push_elem(pv_pack_buff, z_max_size, pt_desc);

        z_ref_offs = (CHAR *)pt_arg->u.pv_desc - (CHAR *)pt_desc->pv_start_raw;

        z_offs = z_ref_offs + (CHAR *)ELEM_REF_DATA(pv_elem) - (CHAR *)pv_pack_buff;
    }
    else
    {
        /* We cannot get description */
        _free_blk_desc(pt_desc);
        return (SIZE_T)0;
    }

     _free_blk_desc(pt_desc);

    /*
     * Then for each entry, get child and put its data using BFS
     * Cursor for expending new child is by pv_elem, pt_desc and pt_entry
     */
    do
    {
        pt_desc = ELEM_REF_DESC(pv_elem);

        FOREACH_BLK_ENTRY(pt_desc, pt_entry)
        {
            if(     (pt_entry->e_type & ARG_MASK_TYPE) == ARG_TYPE_REF_DESC
                ||  (pt_entry->e_type & ARG_MASK_TYPE) == ARG_TYPE_REF_STR )
            {
                ppv_chld = (VOID **)((CHAR *)ELEM_REF_DATA(pv_elem) + pt_entry->z_offs);

                pt_entry->pv_raw = *ppv_chld;

                if(*ppv_chld != NULL)
                {
                    if((pv_elem_chld = _find_elem_by_raw(pv_pack_buff, *ppv_chld)) == NULL)
                    {
                        pt_desc_chld = _get_blk_desc(pt_entry->e_type,
                                                     *ppv_chld,
                                                     pf_get_rpc_desc,
                                                     pv_tag);

                        if(pt_desc_chld == NULL)
                        {
                            /* If we cannot get description, it will not be modified
                             * pt_entry->pv_raw = NULL means this is not resolved.
                             */
                            RPC_LOG("Cannot get desc for (RAW:%p)\n", *ppv_chld);
                            pt_entry->z_init_offs = 0;
                        }
                        else
                        {
                            pv_elem_chld = _push_elem(pv_pack_buff,
                                                      z_max_size,
                                                      pt_desc_chld);
                            _free_blk_desc(pt_desc_chld);

                        }
                    }
                    if(pv_elem_chld != NULL)
                    {
                        pt_desc_chld = ELEM_REF_DESC(pv_elem_chld);

                        /* Determine whether the desc is needed for writeback?
                         * Fixme: This just mark blocks 'directly' generated by this
                         * argument. If the block exits already, this will not go on
                         * marking with its children. That means this assume that the
                         * argument which generates this block has a mask of 'OUT'.
                         */
                        if( (pt_arg->e_type & ARG_DIR_OUT) &&
                            ((pt_entry->e_type & ARG_MASK_DIR) != ARG_DIR_INP))
                        {
                            pt_desc_chld->e_type |= ARG_DIR_OUT;
                        }

                        pt_entry->z_init_offs = (CHAR *)*ppv_chld
                                              - (CHAR *)ELEM_REF_DESC(pv_elem_chld)->pv_start_raw
                                              + (CHAR *)ELEM_REF_DATA(pv_elem_chld)
                                              - (CHAR *)pv_pack_buff;


                    }
                }
                else
                {
                    /* We should not resolve NULL reference */
                    RPC_LOG("NULL field\n");
                    pt_entry->z_init_offs = 0;
                }
            }
        }

        pv_elem = ELEM_NEXT(pv_elem);
    } while((CHAR *)pv_elem < (CHAR *)PACK_REF_ELEM_END(pv_pack_buff));


    return z_offs;
}


/*------------------------------------------------------------------------
 * Name:  ipc_packer_pack
 *
 * Description:
 *   Pack objects to a message buff.
 *
 * Inputs:  pv_pack_buff:         Message buff base
 *          z_max_size:      Buff range
 *          ps_op:           Op string( Or CB type string)
 *          pv_cb_addr:      Callback address. (Can be NULL for OP)
 *          ui4_num_args:    Argument count.
 *          pt_args:         Argument list start address
 *          pf_get_rpc_desc: Callback fro RPC_DESC_T
 *          pv_tag:          Tag for callback
 *
 * Outputs: pz_size:         The packed message size.
 *
 * Returns: IPCR_OK:         Packed it successfully.
 -----------------------------------------------------------------------*/
INT32 ipc_packer_pack(
    VOID *                      pv_pack_buff,
    SIZE_T                      z_max_size,
    const CHAR*                 ps_op,
    VOID *                      pv_cb_addr,
    UINT32                      ui4_num_args,
    ARG_DESC_T*                 pt_args,
    ipc_get_rpc_desc_fct        pf_get_rpc_desc,
    VOID *                      pv_tag,
    SIZE_T *                    pz_size)
{
    ARG_DESC_T *        pt_arg_itr;
    UINT32              ui4_arg_idx;
    VOID *              pv_elem_base;
    PACK_BASIC_T *      pt_basic;
    SIZE_T              z_offs;
    ARG_EX_DESC_T *     pt_arg_ex_desc;

    if (z_max_size <= MAX_OP_SIZE + sizeof(PACK_BASIC_T))
    {
        IPC_ASSERT(0);
    }

    RPC_LOG("Start packing, BASE: %p, OP: %s\n", pv_pack_buff, ps_op);

    memset(pv_pack_buff, 0, z_max_size);

    /* Fill message basic section */
    pt_basic = pv_pack_buff;

    strncpy(pt_basic->ps_op, ps_op, MAX_OP_SIZE);
    pt_basic->ps_op[MAX_OP_SIZE-1] = '\0';

    pt_basic->pv_cb_addr = pv_cb_addr;

    /* Fill all the arugments */
    pt_basic->ui4_num_args  = ui4_num_args;
    if(pt_args != NULL)
    {
        memcpy(PACK_REF_ARG_BASE(pv_pack_buff), pt_args, sizeof(ARG_DESC_T) * ui4_num_args);
    }

    /* Prepare to fill elements */
    pv_elem_base = PACK_REF_ELEM_BASE(pv_pack_buff);
    pt_basic->ui4_elem_end  = (CHAR *)pv_elem_base - (CHAR *)pv_pack_buff;

    PACK_FOREACH_ARG(pv_pack_buff, ui4_arg_idx, pt_arg_itr, pt_arg_ex_desc)
    {
        RPC_LOG("ARG:%u , type:%u, val:%p\n",
                (unsigned)ui4_arg_idx,
                (unsigned)pt_arg_itr->e_type,
                pt_arg_itr->u.pv_desc);

        switch(pt_arg_itr->e_type & ARG_MASK_TYPE)
        {
            case ARG_TYPE_REF_CHAR:
            case ARG_TYPE_REF_INT8:
            case ARG_TYPE_REF_INT16:
            case ARG_TYPE_REF_INT32:
            case ARG_TYPE_REF_INT64:
            case ARG_TYPE_REF_UINT8:
            case ARG_TYPE_REF_UINT16:
            case ARG_TYPE_REF_UINT32:
            case ARG_TYPE_REF_UINT64:
            case ARG_TYPE_REF_SIZE_T:
            case ARG_TYPE_REF_BOOL:
            case ARG_TYPE_REF_STR:
            case ARG_TYPE_REF_DESC:
                z_offs = _pack_arg_ref( pv_pack_buff,
                                        z_max_size,
                                        pt_arg_itr,
                                        pf_get_rpc_desc,
                                        pv_tag);

                if(z_offs != 0)
                {
                    RPC_LOG("Modify ARG (RAW %p) to (OFFSET:0x%x)\n",
                            pt_arg_itr->u.pv_desc,
                            (unsigned)z_offs);

                    pt_arg_ex_desc->pv_raw = pt_arg_itr->u.pv_desc;
                    pt_arg_itr->u.pv_desc  = (VOID *)z_offs;
                }
                else
                {
                    pt_arg_ex_desc->pv_raw = NULL;
                }
            break;

            case ARG_TYPE_CHAR:
            case ARG_TYPE_INT8:
            case ARG_TYPE_INT16:
            case ARG_TYPE_INT32:
            case ARG_TYPE_INT64:
            case ARG_TYPE_UINT8:
            case ARG_TYPE_UINT16:
            case ARG_TYPE_UINT32:
            case ARG_TYPE_UINT64:
            case ARG_TYPE_SIZE_T:
            case ARG_TYPE_BOOL:
            /* Note, ARG_TYPE_REF_FUNC and ARG_TYPE_REF_VOID are not resolved */
            case ARG_TYPE_REF_FUNC:
            case ARG_TYPE_REF_VOID:
                pt_arg_ex_desc->pv_raw  = NULL;
            default:
                break;
        }
    }

    if(pz_size != NULL)
    {
        *pz_size = (CHAR *)PACK_REF_ELEM_END(pv_pack_buff) - (CHAR *)pv_pack_buff;
        IPC_ASSERT(*pz_size < z_max_size);
    }

    if(g_b_log == TRUE)
    {
        _trace_package(pv_pack_buff);
    }

    return IPCR_OK;
}

/*------------------------------------------------------------------------
 * Name:  ipc_packer_translate
 *
 * Description:
 *   Translate packed message to be locally accessible
 *
 * Inputs:  pv_pack_buff:         Message buff base
 *          z_max_size:      Buff range
 *
 * Outputs: pps_op:          Op string(Or CB type string)(Can be NULL)
 *          ppv_cb_addr:     Callback address. (Can be NULL)
 *          ppt_arg_start:   Argument list start address (Can be NULL)
 *          pui4_num_args:   Argument count. (Can be NULL)
 *
 * Returns: IPCR_OK:         Packed it successfully.
 *          IPCR_INV_ARGS:   Error happens.
 -----------------------------------------------------------------------*/
INT32
ipc_packer_translate(
    VOID *          pv_pack_buff,
    SIZE_T          z_max_size,
    CHAR **         pps_op,
    VOID **         ppv_cb_addr,
    ARG_DESC_T **   ppt_arg_start,
    UINT32 *        pui4_num_args)
{
    UINT32          ui4_arg_idx;
    ARG_DESC_T *    pt_arg_itr;
    ARG_EX_DESC_T * pt_arg_ex_desc;

    VOID *          pv_elem;
    UINT32          ui4_i;
    PACK_BASIC_T *  pt_basic = PACK_REF_BASIC(pv_pack_buff);

    if(pps_op != NULL)
    {
        *pps_op = pt_basic->ps_op;
    }
    if(ppv_cb_addr != NULL)
    {
        *ppv_cb_addr = pt_basic->pv_cb_addr;
    }
    if(pui4_num_args != NULL)
    {
        *pui4_num_args = pt_basic->ui4_num_args;
    }

    if(ppt_arg_start != NULL)
    {
        *ppt_arg_start = PACK_REF_ARG_BASE(pv_pack_buff);
    }

    PACK_FOREACH_ARG(pv_pack_buff, ui4_arg_idx, pt_arg_itr, pt_arg_ex_desc)
    {
        switch(pt_arg_itr->e_type & ARG_MASK_TYPE)
        {
            case ARG_TYPE_REF_CHAR:
            case ARG_TYPE_REF_INT8:
            case ARG_TYPE_REF_INT16:
            case ARG_TYPE_REF_INT32:
            case ARG_TYPE_REF_INT64:
            case ARG_TYPE_REF_UINT8:
            case ARG_TYPE_REF_UINT16:
            case ARG_TYPE_REF_UINT32:
            case ARG_TYPE_REF_UINT64:
            case ARG_TYPE_REF_SIZE_T:
            case ARG_TYPE_REF_STR:
            case ARG_TYPE_REF_DESC:
            case ARG_TYPE_REF_BOOL:
                if(pt_arg_ex_desc->pv_raw != NULL)
                {
                    /*Fix back the address */
                    RPC_LOG("ARG (OFFSET:%p) is translated to (LOCAL:%p) \n",
                            pt_arg_itr->u.pv_desc,
                            (CHAR *)((CHAR *)pv_pack_buff + (UINT32)pt_arg_itr->u.pv_desc));

                    pt_arg_itr->u.pv_desc = (CHAR *)((CHAR *)pv_pack_buff + (UINT64)pt_arg_itr->u.pv_desc);
                }
            break;
            case ARG_TYPE_REF_VOID:
            break;
            default:
            break;
        }
    }

    PACK_FOR_EACH_ELEM(pv_pack_buff, pv_elem, ui4_i)
    {
        BLK_DESC_T *        pt_desc;
        VOID *              pv_data;
        BLK_DESC_ENTRY_T *  pt_entry;

        pv_data = ELEM_REF_DATA(pv_elem);
        pt_desc = ELEM_REF_DESC(pv_elem);

        pt_entry = BLK_DESC_GET_ENTRY_START(pt_desc);

        FOREACH_BLK_ENTRY(pt_desc, pt_entry)
        {
            if(     (pt_entry->e_type & ARG_MASK_TYPE) == ARG_TYPE_REF_DESC
                ||  (pt_entry->e_type & ARG_MASK_TYPE) == ARG_TYPE_REF_STR )
            {
                if(pt_entry->z_init_offs != 0)
                {
                    VOID ** ppv_chld = (VOID **)((CHAR *)pv_data + pt_entry->z_offs);
                    RPC_LOG("Translated (OFFSET:%p)to (LOCAL:%p)\n",
                            *ppv_chld,
                            ((CHAR *)pv_pack_buff + pt_entry->z_init_offs));
                    *ppv_chld = (CHAR *)pv_pack_buff + pt_entry->z_init_offs;

                }
            }
        }
    }

    if(g_b_log == TRUE)
    {
        _trace_package(pv_pack_buff);
    }
    return IPCR_OK;
}


/*------------------------------------------------------------------------
 * Name:  _get_raw_addr
 *
 * Description:
 *   Caculate raw address for specified data address in buff.
 *   Sometimes the data reference in message buff might be modified.
 *   If the pv_data referenced data in buffer is no longer the
 *   entry described, the entry will be updated for new.
 *
 * Inputs:  pv_pack_buff:    Message buff base
 *          pv_data:    Data address in buff
 *          pt_entry:   The entry in description.
 * Outputs: pt_entry:   The entry in description.
 *
 * Returns: Raw addresss.
 -----------------------------------------------------------------------*/
static VOID *
_get_raw_addr(VOID * pv_pack_buff, VOID * pv_data, BLK_DESC_ENTRY_T * pt_entry)
{
    VOID * pv_raw = NULL;
    VOID * pv_elem;
    SIZE_T z_offs;

    if(    (CHAR *)pv_data >= (CHAR *)PACK_REF_ELEM_BASE(pv_pack_buff)
        && (CHAR *)pv_data < (CHAR *)PACK_REF_ELEM_END(pv_pack_buff))
    {
        z_offs = (CHAR *)pv_data - (CHAR *)pv_pack_buff;

        if(z_offs == pt_entry->z_init_offs)
        {
            pv_raw = pt_entry->pv_raw;
            RPC_ERR(" _get_raw_addr, Not changed ref:%p, data:%p, z_init_offs:%d\n",
                    pv_raw,
                    pv_data,
                    (unsigned)pt_entry->z_init_offs);
        }
        else
        {
            pv_elem = _find_elem_by_data(pv_pack_buff, pv_data);

            IPC_ASSERT(pv_elem != NULL);
            if (NULL == pv_elem)
            {
                RPC_ERR(" _get_raw_addr, NULL == pv_elem\n");
                return NULL;
            }

            pv_raw  = (CHAR *)pv_data
                    - (CHAR *)ELEM_REF_DATA(pv_elem)
                    + (CHAR *)ELEM_REF_DESC(pv_elem)->pv_start_raw;

            RPC_ERR(" _get_raw_addr, Changed ref:%p, data:%p, z_init_offs:%d\n",
                    pv_raw,
                    pv_data,
                    (unsigned)pt_entry->z_init_offs);
        }

    }
    else
    {
        if(pv_data != NULL)
        {
            RPC_ERR(" _get_raw_addr, !!!!Pointer outof range!!!, pv_data: 0x%x\n", pv_data);
            _trace_package(pv_pack_buff);
            pv_raw = pt_entry->pv_raw;
        }
        else
        {
            RPC_ERR(" _get_raw_addr, NULL reference \n");
            _trace_package(pv_pack_buff);
            pv_raw = NULL;
        }

    }
    return pv_raw;

}


/*------------------------------------------------------------------------
 * Name:  ipc_packer_pack_ret
 *
 * Description:
 *   Pack return package to be sent from user "used" pack buff. The translated
 * package will be restored to raw addr and This function will go through pack
 * buff to generate a new package.
 *
 * Inputs:  pv_pack_buff:    Message buff base
 *          pt_return:  The return value of ipc function
 *          z_max_size: Max size of pv_ret_pack_buff.
 * Outputs:
 *          pv_ret_pack_buff: Out put buff
 *          pz_size:          The final package size of pv_ret_pack_buff.
 *  pt_entry:   The entry in description.
 *
 * Returns: Raw addresss.
 -----------------------------------------------------------------------*/
INT32
ipc_packer_pack_ret(
    VOID *                      pv_pack_buff,
    ARG_DESC_T *                pt_return,
    VOID *                      pv_ret_pack_buff,
    SIZE_T                      z_max_size,
    SIZE_T *                    pz_size)
{
    VOID *              pv_elem;
    UINT32              ui4_i;
    UINT32              ui4_blk_count;
    VOID *              pv_ret_pack;

    BLK_DESC_T *        pt_desc;
    VOID *              pv_data;
    VOID *              pv_ret_data;
    BLK_DESC_ENTRY_T *  pt_entry;

    VOID * pv_copy;
    SIZE_T z_copy_off;
    SIZE_T z_copy_size;
    /* Copy the return to the buff */
    memcpy(RET_PACK_REF_RETURN(pv_ret_pack_buff), pt_return, sizeof(ARG_DESC_T));

    /* Init pz_size as first  */
    pv_ret_pack = RET_PACK_REF_BLK_START(pv_ret_pack_buff);
    *pz_size = (CHAR *)pv_ret_pack - (CHAR *)pv_ret_pack_buff;
    ui4_blk_count = 0;

    PACK_FOR_EACH_ELEM(pv_pack_buff, pv_elem, ui4_i)
    {
        pv_data     = ELEM_REF_DATA(pv_elem);
        pt_desc     = ELEM_REF_DESC(pv_elem);

        /* This needed for client. */

        if(pt_desc->e_type & ARG_DIR_OUT)
        {
            FOREACH_BLK_ENTRY(pt_desc, pt_entry)
            {
                if(     (pt_entry->e_type & ARG_MASK_TYPE) == ARG_TYPE_REF_DESC
                    ||  (pt_entry->e_type & ARG_MASK_TYPE) == ARG_TYPE_REF_STR )
                {
                    VOID ** ppv_chld = (VOID **)((CHAR *)pv_data + pt_entry->z_offs);
                    VOID *  pv_raw = _get_raw_addr(pv_pack_buff, *ppv_chld, pt_entry);

                    RPC_LOG("Restored (DATA:%p) to (RAW:%p)\n",
                               *ppv_chld, pv_raw);

                    /* Fix pointer tobe raw */
                    *ppv_chld = pv_raw;
                }
            }


            /* Assume these are sorted */
            pv_copy = pv_data;
            FOREACH_BLK_ENTRY(pt_desc, pt_entry)
            {
                if(pt_entry->e_type == (ARG_TYPE_UINT8 | ARG_DIR_INP))
                {
                    z_copy_off   = (CHAR *)pv_copy - (CHAR *)pv_data;
                    RPC_LOG(" z_copy_off %d, ENTRY OFF %d\n", z_copy_off, pt_entry->z_offs);
                    z_copy_size  = pt_entry->z_offs - z_copy_off;
                    IPC_ASSERT(z_copy_size + z_copy_off <= pt_desc->z_size);

                    if(z_copy_size != 0)
                    {
                        *pz_size += RET_PACK_BLK_SIZE(z_copy_size);
                        IPC_ASSERT(*pz_size <= z_max_size);

                        *RET_PACK_BLK_REF_RAW(pv_ret_pack)  = (CHAR *)pt_desc->pv_start_raw + z_copy_off;
                        *RET_PACK_BLK_REF_SIZE(pv_ret_pack) = z_copy_size;

                        pv_ret_data = RET_PACK_BLK_REF_DATA(pv_ret_pack);

                        RPC_LOG("Split const pv_copy:%p, copy_size %d\n", pv_copy, (unsigned)z_copy_size);
                        memcpy(pv_ret_data, pv_copy, z_copy_size);

                        pv_copy = (CHAR*)pv_copy + z_copy_size + pt_entry->z_const_size;
                        pv_ret_pack = RET_PACK_BLK_REF_NEXT(pv_ret_pack);
                        ui4_blk_count ++;

                    }
                    else
                    {
                        pv_copy = (CHAR*)pv_copy + pt_entry->z_const_size;
                        RPC_LOG("To next const pv_copy:%p, pt_entry->z_const_size %d\n", pv_copy, (unsigned)pt_entry->z_const_size);
                    }

                }
            }

            z_copy_off   = (CHAR *)pv_copy - (CHAR *)pv_data;
            z_copy_size  = pt_desc->z_size - z_copy_off;

            if(z_copy_size != 0)
            {
                *pz_size += RET_PACK_BLK_SIZE(z_copy_size);
                IPC_ASSERT(*pz_size <= z_max_size);

                *RET_PACK_BLK_REF_RAW(pv_ret_pack)  = (CHAR *)pt_desc->pv_start_raw + z_copy_off;
                *RET_PACK_BLK_REF_SIZE(pv_ret_pack) = z_copy_size;

                pv_ret_data = RET_PACK_BLK_REF_DATA(pv_ret_pack);
                RPC_LOG("End pv_copy:%p, copy_size %d\n", pv_copy, (unsigned)z_copy_size);
                memcpy(pv_ret_data, pv_copy, z_copy_size);

                pv_copy = (CHAR*)pv_copy + z_copy_size + z_copy_off;
                pv_ret_pack = RET_PACK_BLK_REF_NEXT(pv_ret_pack);

                ui4_blk_count ++;
            }


        }

    }

    *RET_PACK_REF_BLK_COUNT(pv_ret_pack_buff) = ui4_blk_count;
    RPC_LOG("ret packed size: %d, bc %d\n", *pz_size, ui4_blk_count);

    return IPCR_OK;
}

/*------------------------------------------------------------------------
 * Name:  ipc_pack_writeback
 *
 * Description:
 *   Update data.
 *
 * Inputs:  pv_pack_buff:    Message buff base
 *          z_max_size: Buff range
 *
 *
 * Outputs: pt_return:  Return value from Buff
 *
 * Returns: IPCR_OK Successful
 -----------------------------------------------------------------------*/
INT32
ipc_packer_writeback(
    VOID *         pv_ret_pack_buff,
    SIZE_T         z_max_size,
    ARG_DESC_T   * pt_return)
{
    UINT32              ui4_i;
    UINT32              ui4_count;
    VOID *              pv_blk;
    VOID *              pv_data;
    SIZE_T              z_size;
    VOID *              pv_raw;

    if(pt_return != NULL)
    {
        memcpy(pt_return, RET_PACK_REF_RETURN(pv_ret_pack_buff), sizeof(ARG_DESC_T));
    }

    ui4_count   = *RET_PACK_REF_BLK_COUNT(pv_ret_pack_buff);
    pv_blk      = RET_PACK_REF_BLK_START(pv_ret_pack_buff);

    for(ui4_i = 0; ui4_i < ui4_count; ui4_i ++)
    {
        pv_raw = *RET_PACK_BLK_REF_RAW(pv_blk);
        z_size = *RET_PACK_BLK_REF_SIZE(pv_blk);
        pv_data = RET_PACK_BLK_REF_DATA(pv_blk);

        RPC_LOG("Write 0x%x, %d\n", pv_raw, z_size);
        memcpy(pv_raw, pv_data, z_size);

        pv_blk = RET_PACK_BLK_REF_NEXT(pv_blk);
    }
    return IPCR_OK;
}



/*************************
 * _trace_package the message buff.
 *************************/
static VOID _trace_package(VOID * pv_buff)
{
    ARG_DESC_T *         pt_arg_itr;
    UINT32               ui4_arg_idx;
    ARG_EX_DESC_T *      pt_arg_ex_desc;

    VOID *               pv_elem;
    UINT32               ui4_i;
    PACK_BASIC_T *    pt_basic;

    pt_basic = (PACK_BASIC_T * )pv_buff;


    RPC_LOG("\n\n");
    RPC_LOG("[IPC MSG _trace_package Base:%p]\n", pv_buff);
    RPC_LOG("+-------------------------------------+\n");
    RPC_LOG(" OP:%s\n", pt_basic->ps_op);
    RPC_LOG(" CB Addr:%p\n", pt_basic->pv_cb_addr);
    RPC_LOG(" Num Args:%u\n", (unsigned)pt_basic->ui4_num_args);
    RPC_LOG(" Element end:0x%x\n", (unsigned)pt_basic->ui4_elem_end);
    RPC_LOG(".......................................\n");

    PACK_FOREACH_ARG(pv_buff, ui4_arg_idx, pt_arg_itr, pt_arg_ex_desc)
    {
        if(pt_arg_ex_desc->pv_raw != NULL)
        {
            RPC_LOG(" ARG T:%d, V:%p (R:%p)\n",
                (unsigned)pt_arg_itr->e_type,
                pt_arg_itr->u.pv_desc,
                pt_arg_ex_desc->pv_raw);
        }
        else
        {
            RPC_LOG(
                " ARG T:%d, V:%p \n",
                (unsigned)pt_arg_itr->e_type,
                pt_arg_itr->u.pv_desc);
        }
    }
    PACK_FOR_EACH_ELEM(pv_buff, pv_elem, ui4_i)
    {
       BLK_DESC_T *         pt_desc;
       VOID *               pv_data;
       BLK_DESC_ENTRY_T *   pt_entry;

       pv_data = ELEM_REF_DATA(pv_elem);
       pt_desc = ELEM_REF_DESC(pv_elem);

       pt_entry = BLK_DESC_GET_ENTRY_START(pt_desc);

       RPC_LOG(".......................................\n");
       RPC_LOG(" Raw addr:%p\n", (ELEM_REF_DESC(pv_elem)->pv_start_raw));
#ifndef IPC_DUMP_DATA
       {
           UCHAR * pc_c;
           for(pc_c = pv_data; (UCHAR*)(pc_c - (UINT32)((UCHAR*)pv_data + 8)) <= pt_desc->z_size; pc_c += 8)
           {
                RPC_LOG(" 0x%08x [ %02x %02x %02x %02x %02x %02x %02x %02x ]\n",
                    (unsigned)pc_c,
                    (unsigned)pc_c[0], (unsigned)pc_c[1], (unsigned)pc_c[2], (unsigned)pc_c[3],
                    (unsigned)pc_c[4], (unsigned)pc_c[5], (unsigned)pc_c[6], (unsigned)pc_c[7]);
           }
           switch(pt_desc->z_size - (pc_c - (UCHAR*)pv_data))
           {
                case 1:
                    RPC_LOG(" %p [ %02x                     ]\n",
                        pc_c,
                        (unsigned)pc_c[0]);
                    break;
                case 2:
                    RPC_LOG(" %p [ %02x %02x                   ]\n",
                        pc_c,
                        (unsigned)pc_c[0],
                        (unsigned)pc_c[1]);
                    break;
                case 3:
                    RPC_LOG(" %p [ %02x %02x %02x                ]\n",
                        pc_c,
                        (unsigned)pc_c[0],
                        (unsigned)pc_c[1],
                        (unsigned)pc_c[2]);
                    break;
                case 4:
                    RPC_LOG(" %p [ %02x %02x %02x %02x             ]\n",
                        pc_c,
                        (unsigned)pc_c[0], (unsigned)pc_c[1],
                        (unsigned)pc_c[2], (unsigned)pc_c[3]);
                    break;
                case 5:
                    RPC_LOG(" %p [ %02x %02x %02x %02x %02x          ]\n",
                        pc_c,
                        (unsigned)pc_c[0], (unsigned)pc_c[1],
                        (unsigned)pc_c[2], (unsigned)pc_c[3],
                        (unsigned)pc_c[4]);
                    break;
                case 6:
                    RPC_LOG(" %p [ %02x %02x %02x %02x %02x %02x       ]\n",
                        pc_c,
                        (unsigned)pc_c[0], (unsigned)pc_c[1],
                        (unsigned)pc_c[2], (unsigned)pc_c[3],
                        (unsigned)pc_c[4], (unsigned)pc_c[5]);
                    break;
                case 7:
                    RPC_LOG(" %p [ %02x %02x %02x %02x %02x %02x %02x      ]\n",
                        pc_c,
                        (unsigned)pc_c[0], (unsigned)pc_c[1], (unsigned)pc_c[2],
                        (unsigned)pc_c[3], (unsigned)pc_c[4], (unsigned)pc_c[5],
                        (unsigned)pc_c[6]);
                    break;
                default:
                    break;
           }
       }
#endif
       RPC_LOG(" Desc Size:%u, Num:%u \n", (unsigned)pt_desc->z_size, (unsigned)pt_desc->ui4_num_entries);
       FOREACH_BLK_ENTRY(pt_desc, pt_entry)
       {
           RPC_LOG(" Entry: type: %x, z_offs(TO block):%u, z_init_offs(TO Msg):%u \n", (unsigned)pt_entry->e_type, (unsigned)pt_entry->z_offs, (unsigned)pt_entry->z_init_offs);
           RPC_LOG("    RAW : %p\n", pt_entry->pv_raw);
       }
    }
    RPC_LOG("+-------------------------------------+\n");

    RPC_LOG("\n\n");

    return;
}


