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

#include <stdio.h>
#include <string.h>

#include "_ipc.h"
#include "_rpc_ipc_util.h"

#define RPC_TRACE_HANDLER_TABLE



typedef struct _HASH_NODE_T
{
    /* Do we really need to copy the key?
     * Or just assume that all the key is from
     * constant string.
     */

    CHAR * ps_key;
    VOID * pv_val;
    struct _HASH_NODE_T * pt_next;
    struct _HASH_NODE_T * pt_prev;
}HASH_NODE_T;

static UINT32           ui4_tbl_len_order;
static HASH_NODE_T **   ppt_tbl = NULL;


#ifdef RPC_TRACE_HANDLER_TABLE
static SIZE_T   *       pz_list_len = NULL;
static SIZE_T           z_max_list_len = 0;
static SIZE_T           z_total = 0;
#endif

/*------------------------------------------------------------------------
 * Name:
 *
 * Description: Hash function for op string. Simply use order to void % as
 * the barral size.
 *
 * Inputs:
 * Outputs: -
 * Returns: -
 -----------------------------------------------------------------------*/
static UINT32 _hash_func(const CHAR * ps_str, UINT32 ui4_order)
{
    UINT32 ui4_h;
    const CHAR * pc_c = NULL;

    ui4_h = 0;
    for(pc_c = ps_str; *pc_c != '\0'; pc_c ++)
    {
        ui4_h = ui4_h * 31 + *pc_c;
    }
    return ui4_h & ((1U << ui4_order) - 1);
}

/*------------------------------------------------------------------------
 * Name:
 *
 * Description: Init hash table for op/cb
 *
 * Inputs:
 * Outputs: -
 * Returns: -
 -----------------------------------------------------------------------*/
static INT32 _init_hash(UINT32 ui4_order)
{
    UINT32 ui4_num = 1U << ui4_order;

    ui4_tbl_len_order = ui4_order;

    printf("_init_hash(), ui4_tbl_len_order=%d, ui4_num=%d\n", (int)ui4_tbl_len_order, (int)ui4_num);

    ppt_tbl       = RPC_CALLOC(ui4_num, sizeof(HASH_NODE_T *));

#ifdef RPC_TRACE_HANDLER_TABLE
    pz_list_len   = RPC_CALLOC(ui4_num, sizeof(SIZE_T));
#endif
    return RPCR_OK;
}


/*------------------------------------------------------------------------
 * Name:
 *
 * Description: Uninit hash table for op/cb
 *
 * Inputs:
 * Outputs: -
 * Returns: -
 -----------------------------------------------------------------------*/
VOID _uninit_hash()
{
    printf("_uninit_hash()\n");
	ui4_tbl_len_order = 0;
	RPC_FREE(ppt_tbl);
	ppt_tbl = NULL;
#ifdef RPC_TRACE_HANDLER_TABLE
RPC_FREE(pz_list_len);
pz_list_len = NULL;
#endif
}

/*------------------------------------------------------------------------
 * Name:
 *
 * Description: Add key to hash table.
 * This assume that the key is not in table
 * Inputs:
 * Outputs: -
 * Returns: -
 -----------------------------------------------------------------------*/
static INT32 _hash_add(const CHAR * ps_key, VOID * pv_val)
{
    UINT32 ui4_idx = _hash_func(ps_key, ui4_tbl_len_order);
    HASH_NODE_T * pt_node = NULL;

    if(pv_val == NULL)
    {
        return RPCR_INV_ARGS;
    }
    pt_node = RPC_MALLOC(sizeof(HASH_NODE_T));
    if(pt_node == NULL)
    {
        return RPCR_OUTOFMEMORY;
    }
    memset(pt_node, 0, sizeof(HASH_NODE_T));
    pt_node->pv_val = pv_val;
    /* FIXME: Copy or assign */
    /* OPs should be constant string, ?? */
    pt_node->ps_key = (CHAR *)ps_key;

    if(ppt_tbl[ui4_idx] == NULL)
    {
        ppt_tbl[ui4_idx] = pt_node;
    }
    else
    {
        pt_node->pt_next = ppt_tbl[ui4_idx];
        ppt_tbl[ui4_idx]->pt_prev = pt_node;
        ppt_tbl[ui4_idx] = pt_node;
    }

#ifdef RPC_TRACE_HANDLER_TABLE
    z_total ++;
    pz_list_len[ui4_idx] ++;
    if(pz_list_len[ui4_idx] > z_max_list_len)
    {
        z_max_list_len = pz_list_len[ui4_idx];
        RPC_LOG("Handler table max list len:%u, total:%u\n",
                z_max_list_len,
                z_total);
    }
#endif

    return RPCR_OK;
}
/*  */

/*------------------------------------------------------------------------
 * Name:
 *
 * Description: Get value of key from hash table.
 * Inputs:
 * Outputs: -
 * Returns: -
 -----------------------------------------------------------------------*/
static INT32 _hash_get(const CHAR * ps_key, VOID ** ppv_val)
{
    UINT32 ui4_idx;
    HASH_NODE_T * pt_node = NULL;

    if(NULL == ps_key)
    {
       return RPCR_INV_ARGS;
    }// for klock issue 10515

    ui4_idx = _hash_func(ps_key, ui4_tbl_len_order);
    if(NULL == ppt_tbl[ui4_idx] )
    {
        return RPCR_NOT_FOUND;
    }
    else
    {
        for(pt_node = ppt_tbl[ui4_idx]; pt_node; pt_node = pt_node->pt_next)
        {
            if((pt_node->ps_key != NULL) && (ps_key != NULL))
            {
	            if(strcmp(pt_node->ps_key, ps_key) == 0)
	            {
	                if(ppv_val)
	                {
	                    *ppv_val = pt_node->pv_val;
	                }
	                return RPCR_OK;
	            }
			}
            else
            {
                if(NULL == pt_node->ps_key)
                {
                    printf("pt_node->ps_key is NULL! idx:%u\n", ui4_idx);
                }
                else
                {
                    printf("ps_key is NULL! idx:%u\n", ui4_idx);
                }
            }
        }
    }
    return RPCR_NOT_FOUND;
}

/* TODO: Implement the hash_remove, .... */


typedef enum RPC_HANDLER_TYPE_E_
{
    RPC_HANDLER_TYPE_OP = 0,
    RPC_HANDLER_TYPE_CB
}RPC_HANDLER_TYPE_E;

typedef struct RPC_HANDLER_T_
{
    RPC_HANDLER_TYPE_E e_type;
    union
    {
        rpc_op_hndlr_fct pf_op_hndlr;
        rpc_cb_hndlr_fct pf_cb_hndlr;
    } u;
}RPC_HANDLER_T;

/*------------------------------------------------------------------------
 * Name:
 *
 * Description: Init rpc handler component.
 * Inputs:
 * Outputs: -
 * Returns: -
 -----------------------------------------------------------------------*/
INT32 bt_rpc_handler_init()
{
    _init_hash(10);
    return RPCR_OK;
}

/*------------------------------------------------------------------------
 * Name:
 *
 * Description: Uninit rpc handler component.
 * Inputs:
 * Outputs: -
 * Returns: -
 -----------------------------------------------------------------------*/
 VOID bt_rpc_handler_uninit()
 {
 	_uninit_hash();
 }

/*------------------------------------------------------------------------
 * Name:
 *
 * Description: Register OP handler. If the OP is already in the table, it
 * will return RPCR_EXIST.
 * Inputs:
 * Outputs: -
 * Returns: -
 -----------------------------------------------------------------------*/
EXPORT_SYMBOL INT32 bt_rpc_reg_op_hndlr(
    const CHAR *         ps_op,
    rpc_op_hndlr_fct     pf_op_hndlr)
{

    RPC_HANDLER_T * pt_handler = NULL;

    if( (NULL ==ps_op )  || (NULL == pf_op_hndlr))
    {
        return RPCR_INV_ARGS;
    }

    pt_handler = RPC_MALLOC(sizeof(RPC_HANDLER_T));
    if(NULL == pt_handler)
    {
        return RPCR_OUTOFMEMORY;
    }
    pt_handler->e_type = RPC_HANDLER_TYPE_OP;
    pt_handler->u.pf_op_hndlr = pf_op_hndlr;
    if(NULL == ppt_tbl)
    {
        RPC_FREE(pt_handler);
        RPC_INFO("ppt_tbl is NULL, L%d!\n", __LINE__);
        return RPCR_OUTOFMEMORY;
    }
    if(_hash_get(ps_op, NULL) == IPCR_OK)
    {
        RPC_FREE(pt_handler);
        RPC_LOG(" Try to register duplicate handler, %s\n", ps_op);
        return RPCR_EXIST;
    }
    return _hash_add(ps_op, pt_handler);

}

/*------------------------------------------------------------------------
 * Name:
 *
 * Description: Register CB handler. If the CB is already in the table, it
 * will return RPCR_EXIST.
 * Inputs:
 * Outputs: -
 * Returns: -
 -----------------------------------------------------------------------*/
EXPORT_SYMBOL INT32 bt_rpc_reg_cb_hndlr(
    const CHAR *         ps_cb_type,
    rpc_cb_hndlr_fct     pf_cb_hndlr)
{
    RPC_HANDLER_T * pt_handler = NULL;

    if((NULL == ps_cb_type ) || (NULL == pf_cb_hndlr))
    {
        return RPCR_INV_ARGS;
    }

    pt_handler = RPC_MALLOC(sizeof(RPC_HANDLER_T));
    if((NULL == pt_handler))
    {
        return RPCR_OUTOFMEMORY;
    }
    pt_handler->e_type = RPC_HANDLER_TYPE_CB;
    pt_handler->u.pf_cb_hndlr = pf_cb_hndlr;
    if(NULL == ppt_tbl)
    {
        RPC_FREE(pt_handler);
        RPC_INFO("ppt_tbl is NULL!\n");
        return RPCR_OUTOFMEMORY;
    }
    if(_hash_get(ps_cb_type, NULL) == IPCR_OK)
    {
        RPC_FREE(pt_handler);
        RPC_LOG(" Try to register duplicate handler, %s\n", ps_cb_type);
        return RPCR_EXIST;
    }
    return _hash_add(ps_cb_type, pt_handler);

}

/*------------------------------------------------------------------------
 * Name:
 *
 * Description: Get OP handler by the OP string from table.
 * Inputs:
 * Outputs: -
 * Returns: -
 -----------------------------------------------------------------------*/
INT32 bt_rpc_get_op_hndlr(
    const CHAR *       ps_op,
    rpc_op_hndlr_fct * ppf_handler)
{
    INT32 i4_ret;
    RPC_HANDLER_T * pt_handler = NULL;

    i4_ret = _hash_get(ps_op, (VOID **)(VOID *)&pt_handler);

    if(i4_ret == RPCR_OK)
    {
        *ppf_handler = pt_handler->u.pf_op_hndlr;
        return RPCR_OK;
    }
    else
    {
        RPC_LOG("Not found op handler %s\n", ps_op);
        return RPCR_NOT_FOUND;
    }
}

/*------------------------------------------------------------------------
 * Name:
 *
 * Description: Get CB handler by the CB string from table.
 * Inputs:
 * Outputs: -
 * Returns: -
 -----------------------------------------------------------------------*/
INT32 bt_rpc_get_cb_hndlr(
    const CHAR *       ps_cb_type,
    rpc_cb_hndlr_fct * ppf_handler)
{
    INT32 i4_ret;
    RPC_HANDLER_T * pt_handler = NULL;

    i4_ret = _hash_get(ps_cb_type, (VOID **)(VOID *)&pt_handler);

    if(RPCR_OK ==i4_ret)
    {
        *ppf_handler = pt_handler->u.pf_cb_hndlr;
        return RPCR_OK;
    }
    else
    {
        RPC_LOG("Not found cb handler %s\n", ps_cb_type);
        return RPCR_NOT_FOUND;
    }

}

/*TODO: Unreg*/

