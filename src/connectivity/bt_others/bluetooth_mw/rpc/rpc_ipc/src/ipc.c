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

/*------------------------------------------------------------------------
                    include files
 -----------------------------------------------------------------------*/

#include <stdio.h>
#include <string.h>
#include <sys/un.h>
#include <sys/socket.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "_ipc_packer.h"
#include "_rpc_ipc_util.h"
#include "_ipc.h"

#define MAX_PATH 255
#define MAX_SV_NAME_SIZE 127

#ifdef RPC_ENABLE_MULTI_CLIENT
#define IPC_SERVER_NAME_LENGTH 64
#define IPC_CLIENT_NUM 8
#endif

/*------------------------------------------------------------------------
                    macros, defines, typedefs, enums
 -----------------------------------------------------------------------*/
extern BOOL bt_rpcu_tl_log_is_on();


#define SOCK_CHECK(stmt, i4_ret, check_exp)\
do\
{\
    i4_ret = (stmt);\
    if(!(check_exp))\
    {\
        RPC_ERR(" %s = %d, errno:%d\n", #stmt, i4_ret, (int)errno);\
    }\
}while(0)


/*  Message:
 *
 *  +-----------+  <--- size of message
 *  | DATA size |
 *  +-----------+
 *  | Type      |
 *  +-----------+
 *  |           |
 *  | MESSAGE   |
 *  +-----------+
 *
 *  MESSAE: OP: Packed argument data;
 */

#define MAX_MSG_SIZE 8192

typedef struct IPC_CFG_T_
{
    CHAR ps_sock_path[MAX_PATH + 1];
    BOOL b_log;

}IPC_CFG_T;

typedef struct IPC_STACK_ITEM_T_
{
    BOOL  b_local;
    const CHAR * ps_op;
    struct IPC_STACK_ITEM_T_ * pt_next;
    struct IPC_STACK_ITEM_T_ * pt_prev;
}IPC_STACK_ITEM_T;


typedef struct _IPC_MSG_BASE_T_
{
    SIZE_T      z_size;
    UINT32      ui4_type;

    /* Sync log control */
    INT32                i4_log_level;
    /*
     * This will sync some status with IPC thread and it's corresponding client thread
     */
    RPCU_THREAD_DESC_T t_thd_desc;

}IPC_MSG_BASE_T;

typedef struct IPC_SESS_T_
{

    IPC_ID_T t_id;
    UINT32                   ui4_ptHandle;
    ipc_close_fct            pf_close;
    ipc_proc_msg_fct         pf_proc_msg;

    union {
        ipc_proc_rpc_op_fct  pf_proc_op;
        ipc_proc_rpc_cb_fct  pf_proc_cb;
    }u;

    /* Socket */
    int      i4_it_sock;
    int      i4_ct_sock;

    /*
     * Semaphore for protect message body, it's length and type
     * sending in different threads. Deprecated.
     */
    /* VOID *   pv_send_sem;*/

    RPCU_THREAD_DESC_T t_cur_thd_desc;

    /*
     *  Synchronize multiple threads except IPC Thread
     *  accessing same IPC ID.
     */
    VOID *   pv_sync_mtx;

    /*call back lock*/
    VOID *   cb_table_lock;

    /* Table management, protected by pv_tbl_lock */
    struct  IPC_SESS_T_ * pt_next;
    struct  IPC_SESS_T_ * pt_prev;

    /*Refer to the Client_T object it belonged to*/
    struct IPC_CLIENT_T_ * pt_client;

    /*Referenced count */
    UINT32  ui4_refc;
    UINT8   ui1_multiClientc;

    /* The ipc thread of this session */
    VOID *  pv_ipc_thd;

    /* THis is for User store private data */
    VOID *  pv_tag;

    INT32 i4_log_level;
    rpc_log_fct pf_log_fct;

    INT32   i4_depth_in_ipc_thread;
    INT32   i4_depth_in_ctx_thread;

    INT32           i4_pending_in_ctx_thread;
    const CHAR *    ps_cur_op_in_ctx_thread;
    INT32           i4_pending_in_ipc_thread;
    const CHAR *    ps_cur_op_in_ipc_thread;

    IPC_STACK_ITEM_T * pt_stack_top_in_ctx_thread;
    IPC_STACK_ITEM_T * pt_stack_top_in_ipc_thread;
}IPC_SESS_T;

/*for exchanging message when a client connects to a server */
 typedef struct MSG_
{
	INT32 i4_cmd;
	VOID * self_ref;
	VOID * rmt_ref;
}MSG;

/*A struct used for defining a unique client. A client can have more than one link with a server.
**Both the client side and server side must have a client_t object for one link.*/
typedef struct IPC_CLIENT_T_
{
	/*refer to the remote equivalent client_t object*/
	struct IPC_CLIENT_T_ * pv_remote_tag;

	/*to maintain the linked list corresponding to a client_t object*/
	struct IPC_SESS_T_    * pt_sess_head;
	struct IPC_SESS_T_    * pt_sess_tail;
#ifdef RPC_ENABLE_MULTI_CLIENT
    char                  server_name[IPC_SERVER_NAME_LENGTH];
#endif

}IPC_CLIENT_T;

/*------------------------------------------------------------------------
                    functions declarations
 -----------------------------------------------------------------------*/
static VOID _set_cur_id(IPC_ID_T t_id);

static IPC_ID_T _get_cur_id();

static INT32 _init_sess_tbl(UINT32 ui4_num);

static IPC_SESS_T * _get_sess(IPC_ID_T t_id, BOOL b_add_ref, BOOL b_multi_client);

static IPC_SESS_T * _get_sess_ex(IPC_ID_T t_id, BOOL b_add_ref, BOOL b_multi_client);

static VOID add_sess_t(IPC_CLIENT_T * pt_client, IPC_ID_T t_id);

static IPC_ID_T _alloc_id(
    VOID *               pv_tag,
    ipc_close_fct        pf_close,
    ipc_proc_msg_fct     pf_proc_msg,
    ipc_proc_rpc_op_fct  pf_proc_rpc_op,
    ipc_proc_rpc_cb_fct  pf_proc_rpc_cb,
    int                  i4_it_sock,
    int                  i4_ct_sock,
    INT32                i4_log_level,
    rpc_log_fct          pf_log_fct);

static BOOL _self_is_ipc_thd(IPC_SESS_T * pt_sess);

static INT32 _send_msg(
    IPC_SESS_T * pt_sess,
    UINT32 ui4_msg_type,
    VOID * pv_msg,
    SIZE_T z_size);

static INT32 _recv_msg(
    IPC_SESS_T *    pt_sess,
    UINT32 *        pui4_msg_type,
    VOID *          pv_msg,
    SIZE_T *        pz_size,
    UINT32          ui4_timeout);

static INT32 _proc_ret(
    IPC_SESS_T *    pt_sess,
    VOID *          pv_msg,
    SIZE_T          z_msg_size,
    ARG_DESC_T *    pt_return);

static INT32 _proc_cb(
    IPC_SESS_T *    pt_sess,
    VOID *          pv_msg,
    size_t          z_msg_size);

static INT32 _proc_op(
    IPC_SESS_T *    pt_sess,
    VOID *          pv_msg,
    size_t          z_msg_size);

static void  _server_ipc_thd(VOID * pv_arg);

static void  _ipc_thd(VOID * pv_arg);

static INT32 _server_start_ipc_thd(IPC_ID_T t_id);

static INT32 _start_ipc_thd(IPC_ID_T t_id);

static VOID _ipc_listen_thd(VOID * pv_arg);

static INT32 _proc_loop(
    IPC_SESS_T *    pt_sess,
/*    VOID *          pv_msg,*/
    ARG_DESC_T *    pt_return,
    UINT32          ui4_timeout);

/*------------------------------------------------------------------------
                    data declarations
 -----------------------------------------------------------------------*/
/* Static APIs and variables for sess table.
 * IPC id is the index of table.
 */
static IPC_SESS_T * pt_free;
static IPC_SESS_T * pt_sess_tbl;
static SIZE_T  ui4_max_sess_num = 0;

static VOID * pv_tbl_lock;

/*Thread local storage key for a session, currently is necessary*/
static VOID * pv_key_sess = NULL;

static VOID * pv_key_client_auto_close = NULL;

/*Thread local storage key for a client_t object*/
static VOID * pv_key_client = NULL;

static IPC_CFG_T t_cfg;

BOOL   g_b_log = FALSE;
EXPORT_SYMBOL char * g_ps_exe_name = "Not Specified";

static SIZE_T  z_used_id = 0;

#define MAX_LOG_SIZE 255

#ifndef RPC_ENABLE_MULTI_CLIENT
/*if the first link comes, then b_client_new is TRUE, or is FALSE*/
static BOOL b_client_new = TRUE;
#endif

/*if init sess tbl for the first time, then b_init_first is TRUE, or is FALSE*/
static BOOL b_init_first = TRUE;

#ifndef RPC_ENABLE_MULTI_CLIENT
static IPC_CLIENT_T * pt_client;

#else
static IPC_CLIENT_T *pt_clients[IPC_CLIENT_NUM] = {NULL};
#endif


/*------------------------------------------------------------------------
 * Name:
 *
 * Description: Push the op/cb into RPC calling stack.
 *
 * Inputs:
 * Outputs: -
 * Returns: -
 -----------------------------------------------------------------------*/
static IPC_STACK_ITEM_T * _stack_push_op(
    IPC_STACK_ITEM_T * pt_cur_top,
    const CHAR * ps_op,
    BOOL b_local)
{
    IPC_STACK_ITEM_T * pt_top;

    pt_top = RPC_MALLOC(sizeof(IPC_STACK_ITEM_T));
    IPC_ASSERT(pt_top != NULL);

    if (NULL == pt_top)
    {
        printf("malloc fail!!!\n");
        return NULL;
    }

    memset(pt_top, 0, sizeof(IPC_STACK_ITEM_T));

    pt_top->pt_next = pt_cur_top;
    pt_top->ps_op   = ps_op;
    pt_top->b_local = b_local;
    pt_top->pt_next = pt_top->pt_prev = NULL;

    if(pt_cur_top != NULL)
    {
        pt_cur_top->pt_next = pt_top;
        pt_top->pt_prev     = pt_cur_top;
    }
    return pt_top;
}

/*------------------------------------------------------------------------
 * Name:
 *
 * Description: Pop the op/cb from RPC calling stack.
 *
 * Inputs:
 * Outputs: -
 * Returns: -
 -----------------------------------------------------------------------*/
static IPC_STACK_ITEM_T * _stack_pop_op(
    IPC_STACK_ITEM_T * pt_cur_top)
{
    IPC_STACK_ITEM_T * pt_top = NULL;

    if(pt_cur_top != NULL)
    {
        pt_top = pt_cur_top->pt_prev;
        RPC_FREE(pt_cur_top);
		pt_cur_top = NULL; //MTK40438 add for kloc issue
    }

    return pt_top;
}

/*------------------------------------------------------------------------
 * Name:
 *
 * Description: Destroy RPC calling stack.
 *
 * Inputs:
 * Outputs: -
 * Returns: -
 -----------------------------------------------------------------------*/
static VOID _stack_destroy(IPC_STACK_ITEM_T * pt_cur_top)
{
    IPC_STACK_ITEM_T * pt_top = NULL;
    while(pt_cur_top != NULL)
    {
        pt_top = pt_cur_top->pt_prev;
        RPC_FREE(pt_cur_top);
        pt_cur_top = pt_top;
    }
}

/*------------------------------------------------------------------------
 * Name:
 *
 * Description: Trace RPC calling stack.
 *
 * Inputs:
 * Outputs: -
 * Returns: -
 -----------------------------------------------------------------------*/
static VOID _trace_stack(IPC_STACK_ITEM_T * pt_cur_top)
{
    while(pt_cur_top != NULL)
    {
        RPC_LOG("  %s\n", pt_cur_top->ps_op);
        pt_cur_top = pt_cur_top->pt_prev;
    }
}

/*
 * FIXME :
 *
 */
 /*------------------------------------------------------------------------
 * Name:
 *
 * Description: Set IPC id as "current" id in current thread. IPC thread will
 * set current id to what it is processing for. Client may use this as "default
 * id". FIXME: If client has several links, which one should be the default one is not
 * decided. (Currently is the last one).
 *
 * Inputs:
 * Outputs: -
 * Returns: -
 -----------------------------------------------------------------------*/
static VOID _set_cur_id(IPC_ID_T t_id)
{
    rpcu_os_tls_set(pv_key_sess, (VOID *)t_id);
}

/*------------------------------------------------------------------------
 * Name:
 *
 * Description: Auto close the client if client dose not closed when client
 * thread exits.
 *
 * Inputs:
 * Outputs: -
 * Returns: -
 -----------------------------------------------------------------------*/
static VOID _ipc_client_auto_close(VOID * pv_key)
{
    IPC_ID_T t_id = (IPC_ID_T)pv_key;
    printf("pv_key_sess=%p, t_id is: %d, thread=%p\n", pv_key_sess, t_id, rpcu_os_thread_self());
    RPC_ERR("pv_key_sess=%p, t_id is: %d, thread=%p\n", pv_key_sess, t_id, rpcu_os_thread_self());
    ipc_close_client(t_id, TRUE);
}

/*------------------------------------------------------------------------
 * Name:
 *
 * Description: Get current id
 *
 * Inputs:
 * Outputs: -
 * Returns: -
 -----------------------------------------------------------------------*/
static IPC_ID_T _get_cur_id()
{
    IPC_ID_T t_id;

    t_id = (IPC_ID_T)rpcu_os_tls_get(pv_key_sess);

    RPC_LOG("pv_key_sess=%p, t_id is: %ld, thread=%p\n", pv_key_sess, t_id, rpcu_os_thread_self());

    if(t_id == 0)
    {
        return IPCR_NOT_OPENED;
    }
    return t_id;
}

/*------------------------------------------------------------------------
 * Name:
 *
 * Description: Init session table with a maximal size and some resources .
 *
 * Inputs:
 * Outputs: -
 * Returns: -
 -----------------------------------------------------------------------*/
static INT32
_init_sess_tbl(UINT32 ui4_num)
{
    if(TRUE == b_init_first)
    {
        b_init_first = FALSE;
        UINT32 ui4_i;

        rpcu_os_mutex_create(&pv_tbl_lock);

        ui4_max_sess_num = ui4_num;

        RPC_LOG("ui4_max_sess_num=%d\n", (int)ui4_num);

        pt_sess_tbl = RPC_MALLOC(sizeof(IPC_SESS_T) * ui4_num);
        if(pt_sess_tbl == NULL)
        {
            return IPCR_OUTOFMEMORY;
        }
        memset(pt_sess_tbl, 0, sizeof(IPC_SESS_T) * ui4_num);

        /* Default ID 0 is used for special purpose.  */
        for(ui4_i = 1; ui4_i < ui4_num; ui4_i ++)
        {
            pt_sess_tbl[ui4_i].t_id     = ui4_i;
            pt_sess_tbl[ui4_i].ui4_refc = 0;
			pt_sess_tbl[ui4_i].ui4_ptHandle = 0xffffffff;
            if(ui4_i < ui4_num - 1)
            {
                pt_sess_tbl[ui4_i].pt_next = &pt_sess_tbl[ui4_i + 1];
            }
            else
            {
                pt_sess_tbl[ui4_i].pt_next = NULL;
            }
        }

        pt_free = &pt_sess_tbl[1];
        printf("pt_free = &pt_sess_tbl[1]\n");

        return IPCR_OK;
    }

    return IPCR_OK;
}

/*------------------------------------------------------------------------
 * Name:
 *
 * Description: Uninit session table.
 *
 * Inputs:
 * Outputs: -
 * Returns: -
 -----------------------------------------------------------------------*/
VOID _uninit_sess_tbl()
{
    RPC_LOG("_uninit_sess_tbl()\n");
    rpcu_os_mutex_delete(pv_tbl_lock);

    if(pt_sess_tbl != NULL)
    {
		RPC_FREE(pt_sess_tbl);
		pt_sess_tbl = NULL;
		pt_free = NULL;
    }
    b_init_first = TRUE;
}

/*------------------------------------------------------------------------
 * Name:
 *
 * Description: Allocate a new id from session table. If there is no free
 * one, this will failed with return value 0.
 *
 * Inputs:
 * Outputs: -
 * Returns: -
 -----------------------------------------------------------------------*/

static IPC_ID_T _alloc_id(
    VOID *               pv_tag,
    ipc_close_fct        pf_close,
    ipc_proc_msg_fct     pf_proc_msg,
    ipc_proc_rpc_op_fct  pf_proc_rpc_op,
    ipc_proc_rpc_cb_fct  pf_proc_rpc_cb,
    int                  i4_it_sock,
    int                  i4_ct_sock,
    INT32                i4_log_level,
    rpc_log_fct          pf_log_fct)
{
    IPC_SESS_T * pt_sess;


    rpcu_os_mutex_lock(pv_tbl_lock);

    if(pt_free != NULL)
    {
        pt_sess             = pt_free;
        pt_free             = pt_free->pt_next;
        pt_sess->ui4_refc   = 1;
		pt_sess->ui1_multiClientc = 1;

        rpcu_os_mutex_unlock(pv_tbl_lock);

        pt_sess->pf_close        = pf_close;
        pt_sess->pf_proc_msg     = pf_proc_msg;
        /*FIX here, for cb and op */
        if(pf_proc_rpc_op != NULL)
        {
            pt_sess->u.pf_proc_op   = pf_proc_rpc_op;
        }
        else if(pf_proc_rpc_cb != NULL)
        {
            pt_sess->u.pf_proc_cb   = pf_proc_rpc_cb;
        }
        rpcu_os_mutex_create(&pt_sess->pv_sync_mtx);
        rpcu_os_mutex_create(&pt_sess->cb_table_lock);

        pt_sess->i4_it_sock         = i4_it_sock;
        pt_sess->i4_ct_sock         = i4_ct_sock;
        pt_sess->pt_next            = NULL;

        pt_sess->pv_tag             = pv_tag;

        pt_sess->i4_log_level       = i4_log_level;
        pt_sess->pf_log_fct         = pf_log_fct;
        z_used_id ++;

        RPC_INFO("New IPC ID (Count %d)\n", z_used_id);

        pt_sess->pt_stack_top_in_ctx_thread = NULL;
        pt_sess->pt_stack_top_in_ipc_thread = NULL;

    }
    else
    {
        printf("pt_free == NULL\n");
        rpcu_os_mutex_unlock(pv_tbl_lock);
        /* Or resize */
        return 0;
    }
    return pt_sess->t_id;
}


/*------------------------------------------------------------------------
 * Name:
 *
 * Description: Get the session by id and it may inc reference count.
 *
 * Inputs:
 * Outputs: -
 * Returns: -
 -----------------------------------------------------------------------*/
static IPC_SESS_T * _get_sess(IPC_ID_T t_id, BOOL b_add_ref, BOOL b_multi_client)
{
    IPC_SESS_T * pt_sess;

    RPC_LOG("[RPCIPC]_get_sess is called, t_id is: %ld\n", t_id);

    if(t_id == IPC_DEFAULT_ID)
    {
        t_id = _get_cur_id();
    }

    if(t_id < 0 || ((UINT32)t_id) >= ui4_max_sess_num)
    {
        return NULL;
    }

    rpcu_os_mutex_lock(pv_tbl_lock);

    pt_sess = &pt_sess_tbl[(INT32)t_id];

    /* Get session which is freed */
    if(pt_sess->ui4_refc == 0)
    {
        rpcu_os_mutex_unlock(pv_tbl_lock);
        return NULL;
    }
    if(b_add_ref == TRUE)
    {
        pt_sess->ui4_refc ++;
        RPC_LOG("[RPCIPC]_get_sess:pt_sess->ui4_refc=%d, ipc thread=%p, cur thread=%p\n", (int)(pt_sess->ui4_refc), pt_sess->pv_ipc_thd, rpcu_os_thread_self());
    }

	if(b_multi_client)
    {
        pt_sess->ui1_multiClientc ++;
        RPC_LOG("[RPCIPC]pt_sess->ui1_multiClientc=%d, ipc thread=%p, cur thread=%p\n", (int)(pt_sess->ui1_multiClientc), pt_sess->pv_ipc_thd, rpcu_os_thread_self());
    }
    rpcu_os_mutex_unlock(pv_tbl_lock);
    return pt_sess;
}

/*------------------------------------------------------------------------
 * Name:
 *
 * Description: Get the session by id and it may inc reference count,espcially for the do_cb while
 the client connection closed.
 *
 * Inputs:
 * Outputs: -
 * Returns: -
 -----------------------------------------------------------------------*/

static IPC_SESS_T * _get_sess_ex(IPC_ID_T t_id, BOOL b_add_ref, BOOL b_multi_client)
{
    IPC_SESS_T * pt_sess;
	UINT32 ui4_pHandle;
	INT32 i= 0;
	BOOL b_other_ct = FALSE;

    RPC_LOG("[RPCIPC]<Server>_get_sess_ex is called, t_id is: %ld\n", t_id);

    if (t_id == IPC_DEFAULT_ID)
    {
        t_id = _get_cur_id();
    }

    if (t_id < 0 || (UINT32)t_id >= ui4_max_sess_num)
    {
        return NULL;
    }

    rpcu_os_mutex_lock(pv_tbl_lock);

    pt_sess = &pt_sess_tbl[(INT32)t_id];
	ui4_pHandle = pt_sess->ui4_ptHandle;
	RPC_LOG("[RPCIPC]_get_sess_ex:t_id = %ld,ui4_pHandle= %ld\n",t_id,ui4_pHandle);
    /* Get session which is freed */
    if (pt_sess->ui4_refc == 0)
    {

    	IPC_SESS_T * pt_tmp_sess;
        for(i = 1; (UINT32)i < ui4_max_sess_num;i++)
        {
          pt_tmp_sess = &pt_sess_tbl[i];
		  if((ui4_pHandle == pt_tmp_sess->ui4_ptHandle) && (i!=t_id) && (pt_tmp_sess->pv_ipc_thd != NULL))
		  {
		    printf("[RPCIPC]_get_sess_ex:find valid pt_sess,t_id =%d\n",i);
		    pt_sess = pt_tmp_sess;
		    b_other_ct = TRUE;
		    break;
		  }
        }
    }
	else
	{
	  b_other_ct = TRUE;
	}

	if(!b_other_ct)
	{
	    printf("[RPCIPC]_get_sess_ex:No valid pt_sess\n");
		rpcu_os_mutex_unlock(pv_tbl_lock);
        return NULL;
	}

    if (b_add_ref == TRUE)
    {
        pt_sess->ui4_refc ++;
    }

	if(b_multi_client)
    {
        pt_sess->ui1_multiClientc ++;
        RPC_LOG("pt_sess->ui1_multiClientc=%d, ipc thread=%p, cur thread=%p\n", (int)(pt_sess->ui1_multiClientc), pt_sess->pv_ipc_thd, rpcu_os_thread_self());
    }

    rpcu_os_mutex_unlock(pv_tbl_lock);
    return pt_sess;
}

/*------------------------------------------------------------------------
 * Name:
 *
 * Description: Unref a session. This will descrease the reference count.
 *
 * Inputs:
 * Outputs: -
 * Returns: -
 -----------------------------------------------------------------------*/
static VOID _unref_sess(IPC_SESS_T * pt_sess)
{
    IPC_ASSERT(pt_sess != NULL);

    if (NULL == pt_sess) //mtk401456 klocwork issue
    {
        printf("NULL == pt_sess\n");
        return ;
    }

    rpcu_os_mutex_lock(pv_tbl_lock);

    pt_sess->ui4_refc --;

    RPC_LOG("[RPCIPC]_unref_sess:pt_sess->ui4_refc=%d, ipc thread=%p, cur thread=%p\n", (int)(pt_sess->ui4_refc), pt_sess->pv_ipc_thd, rpcu_os_thread_self());

    if(pt_sess->ui4_refc == 0)
    {
        RPC_LOG("[RPCIPC]_unref_sess:Unref the sess to 0, RPC_FREE id: %d\n", (int)pt_sess->t_id);
        /* Clean the resource */
		if (pt_sess->i4_ct_sock >= 0)
		{
            printf("[RPCIPC]_unref_sess:Close(pt_sess->i4_ct_sock: %d)\n", pt_sess->i4_ct_sock );
			close(pt_sess->i4_ct_sock);
			pt_sess->i4_ct_sock = -1;
		}
		if (pt_sess->i4_it_sock >= 0)
		{
            printf("[RPCIPC]_unref_sess:Close(pt_sess->i4_it_sock: %d)\n", pt_sess->i4_it_sock);
			close(pt_sess->i4_it_sock);
			pt_sess->i4_it_sock = -1;
		}

		pt_sess->pv_ipc_thd = NULL;

        rpcu_os_mutex_delete(pt_sess->pv_sync_mtx);
		pt_sess->pv_sync_mtx = NULL;

        rpcu_os_mutex_delete(pt_sess->cb_table_lock);
		pt_sess->cb_table_lock = NULL;

        _stack_destroy(pt_sess->pt_stack_top_in_ctx_thread);
        _stack_destroy(pt_sess->pt_stack_top_in_ipc_thread);

        /* Add to free list */
        pt_sess->pt_next = pt_free;
        pt_free          = pt_sess;

        z_used_id --;
        RPC_INFO("Unref IPC ID (Count %d)\n", z_used_id);

    }

    rpcu_os_mutex_unlock(pv_tbl_lock);
}

/*------------------------------------------------------------------------
 * Name:
 *
 * Description: Set context thread socket to the IPC id if it is prepared.
 *
 * Inputs:
 * Outputs: -
 * Returns: -
 -----------------------------------------------------------------------*/
static INT32 _set_ct_sock(
    IPC_ID_T             t_id,
    int                  i4_ct_sock)
{
    IPC_SESS_T * pt_sess = _get_sess(t_id, TRUE, FALSE);

    /*TODO Check ID here !!*/
    if(pt_sess == NULL)
    {
        return IPCR_INV_IPC_ID;
    }
    pt_sess->i4_ct_sock = i4_ct_sock;
    _unref_sess(pt_sess);

    return IPCR_OK;
}


/*------------------------------------------------------------------------
 * Name:
 *
 * Description: Check current thread is the IPC thread for a session.
 *
 * Inputs:
 * Outputs: -
 * Returns: -
 -----------------------------------------------------------------------*/
static BOOL _self_is_ipc_thd(IPC_SESS_T * pt_sess)
{
    VOID * pv_thd;

    pv_thd = rpcu_os_thread_self();

    if(pv_thd == pt_sess->pv_ipc_thd)
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}


/*------------------------------------------------------------------------
 * Name:
 *
 * Description: Enter a session. Context threads to call ipc_do_... must be
 * synchronized. This entering/leaving is recursive by mutex. if current thread is
 * IPC thread, currently nothing is done.
 *
 * Inputs:
 * Outputs: -
 * Returns: -
 -----------------------------------------------------------------------*/
static INT32 _enter_sess(IPC_SESS_T * pt_sess, const CHAR * ps_op, UINT32 ui4_timeout)
{
    INT32 i4_ret;
    IPC_CLIENT_T * pt_client;
    IPC_SESS_T *    pt_temp_sess;

    if(_self_is_ipc_thd(pt_sess) == FALSE)
    {
        if((i4_ret = rpcu_os_mutex_lock_timeout(pt_sess->pv_sync_mtx, 10000)) != IPCR_OK)  // time out: 10s
        {
        	pt_client = pt_sess->pt_client;
		for(pt_temp_sess = pt_client->pt_sess_head;pt_temp_sess != pt_client->pt_sess_tail;pt_temp_sess = pt_temp_sess->pt_next)
        {
			if( (i4_ret= (rpcu_os_mutex_lock_timeout(pt_temp_sess->pv_sync_mtx, 10000))) == IPCR_OK)
			{
				return IPCR_OK;
			}
		}
            return IPCR_TIMEOUT;
        }
	return IPCR_OK;
    }
    return IPCR_OK;
}


/*------------------------------------------------------------------------
 * Name:
 *
 * Description: Leave a session. Context threads to call ipc_do_... must be
 * synchronized. This entering/leaving is recursive by mutex. if current thread is
 * IPC thread, currently nothing is done.
 *
 * Inputs:
 * Outputs: -
 * Returns: -
 -----------------------------------------------------------------------*/
static VOID _leave_sess(IPC_SESS_T * pt_sess)
{
    INT32 i4_ret;
    if(_self_is_ipc_thd(pt_sess) == FALSE)
    {
        i4_ret = rpcu_os_mutex_unlock(pt_sess->pv_sync_mtx);
        IPC_ASSERT(i4_ret == IPCR_OK);
    }
}

/*------------------------------------------------------------------------
 * Name:
 *
 * Description: Enter a session. Context threads to call ipc_do_cb... must be
 * synchronized.
 *
 * Inputs:
 * Outputs: -
 * Returns: -
 -----------------------------------------------------------------------*/
static INT32 _lock_sess_table(IPC_SESS_T * pt_sess)
{
    INT32 i4_ret;
    if((i4_ret = rpcu_os_mutex_lock(pt_sess->cb_table_lock)) != IPCR_OK)
    {
        return IPCR_INV_IPC_ID;
    }
	return IPCR_OK;
}


/*------------------------------------------------------------------------
 * Name:
 *
 * Description: Leave a session. Context threads to call ipc_do_cb.. must be
 * synchronized.
 *
 * Inputs:
 * Outputs: -
 * Returns: -
 -----------------------------------------------------------------------*/
static VOID _unlock_sess_table(IPC_SESS_T * pt_sess)
{
    INT32 i4_ret;
    i4_ret = rpcu_os_mutex_unlock(pt_sess->cb_table_lock);
    IPC_ASSERT(i4_ret == IPCR_OK);
}

/*------------------------------------------------------------------------
 * Name:
 *
 * Description: Push op into IPC calling stack.
 *
 * Inputs:
 * Outputs: -
 * Returns: -
 -----------------------------------------------------------------------*/
static VOID _push_op(IPC_SESS_T * pt_sess, const CHAR * ps_op, BOOL b_local)
{
    if(_self_is_ipc_thd(pt_sess) == FALSE)
    {
        pt_sess->pt_stack_top_in_ctx_thread =
            _stack_push_op(pt_sess->pt_stack_top_in_ctx_thread, ps_op, b_local);
    }
    else
    {
        pt_sess->pt_stack_top_in_ipc_thread =
            _stack_push_op(pt_sess->pt_stack_top_in_ipc_thread, ps_op, b_local);
    }
}

/*------------------------------------------------------------------------
 * Name:
 *
 * Description: Pop op from IPC calling stack.
 *
 * Inputs:
 * Outputs: -
 * Returns: -
 -----------------------------------------------------------------------*/
static VOID _pop(IPC_SESS_T * pt_sess)
{
    if(_self_is_ipc_thd(pt_sess) == FALSE)
    {
        pt_sess->pt_stack_top_in_ctx_thread =
            _stack_pop_op(pt_sess->pt_stack_top_in_ctx_thread);
    }
    else
    {
        pt_sess->pt_stack_top_in_ipc_thread =
            _stack_pop_op(pt_sess->pt_stack_top_in_ipc_thread);
    }
}

/*------------------------------------------------------------------------
 * Name:
 *
 * Description: Send message to the other side. If the current thread is IPC
 * thread this will use IPC thread socket. If the current thread is context
 * thread this will use context thread socket of this session.
 *
 * Inputs:
 * Outputs: -
 * Returns: -
 -----------------------------------------------------------------------*/
static INT32 _send_msg(
    IPC_SESS_T * pt_sess,
    UINT32 ui4_msg_type,
    VOID * pv_msg,
    SIZE_T z_size)
{
    IPC_MSG_BASE_T  t_msg_base;
    INT32           i4_sock;
    int             i4_ret;

    memset(&t_msg_base, 0, sizeof(IPC_MSG_BASE_T));
    if(pt_sess != NULL)
    {
        t_msg_base.z_size    = z_size;
        t_msg_base.ui4_type  = ui4_msg_type;

        t_msg_base.i4_log_level  = pt_sess->i4_log_level;

        /* Now the message base's thread description should be packed. */
        if(rpcu_os_get_cur_thd_desc(&t_msg_base.t_thd_desc) == IPCR_OK)
        {
            rpcu_os_thd_desc_cpy(&pt_sess->t_cur_thd_desc, &t_msg_base.t_thd_desc);
        }
        else
        {
            RPC_ERR("Thread desc cannot be synced...\n");
            /* I don't want to fail now ... */
        }

        if(_self_is_ipc_thd(pt_sess) == TRUE)
        {
            i4_sock = pt_sess->i4_it_sock;
        }
        else
        {
            i4_sock = pt_sess->i4_ct_sock;
        }

        SOCK_CHECK(send(i4_sock, &t_msg_base, sizeof(IPC_MSG_BASE_T), 0), i4_ret, i4_ret > 0);
        SOCK_CHECK(send(i4_sock, pv_msg, z_size, 0), i4_ret, i4_ret > 0);

        RPC_LOG("Sent Msg type:%d size:%d\n", (int)(t_msg_base.ui4_type) , (int)(t_msg_base.z_size));

        return IPCR_OK;
    }

    return IPCR_NOT_OPENED;
}


/*------------------------------------------------------------------------
 * Name:
 *
 * Description: Receive message from the other side. If the current thread is IPC
 * thread this will use IPC thread socket. If the current thread is context
 * thread this will use context thread socket of this session.
 *
 * Inputs:
 * Outputs: -
 * Returns: -
 -----------------------------------------------------------------------*/
static INT32 _recv_msg(
    IPC_SESS_T *    pt_sess,
    UINT32 *        pui4_msg_type,
    VOID *          pv_msg,
    SIZE_T *        pz_size,
    UINT32          ui4_timeout)
{
    IPC_MSG_BASE_T  t_msg_base;
    int             i4_ret;
    INT32           i4_sock;

    if(_self_is_ipc_thd(pt_sess) == TRUE)
    {
        i4_sock = pt_sess->i4_it_sock;
    }
    else
    {
        i4_sock = pt_sess->i4_ct_sock;
    }

    memset(&t_msg_base, 0, sizeof(IPC_MSG_BASE_T));
    SOCK_CHECK(recv(i4_sock, &t_msg_base, sizeof(IPC_MSG_BASE_T), 0), i4_ret, i4_ret > 0);
    if(i4_ret <= 0)
    {
        return IPCR_CNN_FAIL;
    }
    RPC_LOG("Recved Msg base in IT, type:%u size:%u\n", (unsigned)t_msg_base.ui4_type , t_msg_base.z_size);
    if(t_msg_base.z_size > MAX_MSG_SIZE)
    {
        return IPCR_RCVD_ERR;
    }
    if(t_msg_base.z_size > 0)
    {
        SOCK_CHECK(recv(i4_sock, pv_msg, t_msg_base.z_size, 0), i4_ret, i4_ret > 0);
        if(i4_ret <= 0)
        {
            return IPCR_CNN_FAIL;
        }
    }
    pt_sess->i4_log_level = t_msg_base.i4_log_level;
    rpcu_logger_stack_set(pt_sess->pf_log_fct, pt_sess->i4_log_level);
    /* Now the message base's thread description should be processed. */
    if(rpcu_os_thd_desc_eqaul(&pt_sess->t_cur_thd_desc, &t_msg_base.t_thd_desc) == FALSE)
    {
        /* Not equaled, set the desc from revced */
        RPC_LOG("Detect thread priority changed\n");
        if(rpcu_os_set_cur_thd_desc(&t_msg_base.t_thd_desc) == IPCR_OK)
        {
            RPC_LOG("Thread desc synced...\n");
            rpcu_os_thd_desc_cpy(&pt_sess->t_cur_thd_desc, &t_msg_base.t_thd_desc);
        }
        else
        {
            RPC_ERR("Thread desc cannot be synced...\n");
            /* I don't want to fail now ... */
        }

    }

    *pz_size        = t_msg_base.z_size;
    *pui4_msg_type  = t_msg_base.ui4_type;

    return IPCR_OK;
}

/*------------------------------------------------------------------------
 * Name:
 *
 * Description: Process return message. Memory will be updated.
 *
 * Inputs:
 * Outputs: -
 * Returns: -
 -----------------------------------------------------------------------*/
static INT32 _proc_ret(
    IPC_SESS_T *    pt_sess,
    VOID *          pv_msg,
    SIZE_T          z_msg_size,
    ARG_DESC_T *    pt_return)
{
    RPC_LOG(" Proc ret \n");
    ipc_packer_writeback(pv_msg, z_msg_size, pt_return);
    return IPCR_OK;

}

/*------------------------------------------------------------------------
 * Name:
 *
 * Description: Process callback message. This will call callback handler
 * callback and send return message to the other side.This may enter
 * recursive calling with ipc_do_op/ipc_do_cb  from hander.
 *
 * Inputs:
 * Outputs: -
 * Returns: -
 -----------------------------------------------------------------------*/
static INT32 _proc_cb(
    IPC_SESS_T *    pt_sess,
    VOID *          pv_msg,
    size_t          z_msg_size)
{
    CHAR *      ps_cb_type;
    VOID *      pv_cb_addr;
    UINT32      ui4_num_args_cb;
    ARG_DESC_T* pt_args_cb;
    ARG_DESC_T  t_return_cb;
    INT32       i4_ret;
    VOID *      pv_repack_msg;

    RPC_LOG(" Proc CB \n");

    if((i4_ret = ipc_packer_translate(pv_msg,
                                    z_msg_size,
                                    &ps_cb_type,
                                    &pv_cb_addr,
                                    &pt_args_cb,
                                    &ui4_num_args_cb)) != IPCR_OK)
    {
        return i4_ret;
    }

    RPC_LOG(" Call CB hdlr \n");
    if((i4_ret = _enter_sess(pt_sess, ps_cb_type, 0)) == IPCR_OK)
    {
    _push_op(pt_sess, ps_cb_type, TRUE);
    }else
    {
        RPC_ERR("err: enter sess fail\n");
        return i4_ret;
    }
    if((i4_ret = pt_sess->u.pf_proc_cb(pt_sess->t_id,
                                       ps_cb_type,
                                       pv_cb_addr,
                                       ui4_num_args_cb,
                                       pt_args_cb,
                                       &t_return_cb)) != IPCR_OK)
    {
        _leave_sess(pt_sess);
        return i4_ret;
    }

    _pop(pt_sess);
    _leave_sess(pt_sess);

    pv_repack_msg = malloc(MAX_MSG_SIZE);
    if(pv_repack_msg == NULL)
    {
        return IPCR_OUTOFMEMORY;
    } else {
		memset(pv_repack_msg, 0, MAX_MSG_SIZE);
    }

    if((i4_ret = ipc_packer_pack_ret(pv_msg,
                                     &t_return_cb,
                                     pv_repack_msg,
                                     MAX_MSG_SIZE,
                                     &z_msg_size)) != IPCR_OK)
    {
        free(pv_repack_msg);
        return i4_ret;
    }

    RPC_LOG(" Send CB RET \n");
    if((i4_ret = _send_msg(pt_sess, IPCM_TYPE_RET, pv_repack_msg, z_msg_size)) != IPCR_OK)
    {
        free(pv_repack_msg);
        return i4_ret;
    }
    free(pv_repack_msg);
    return IPCR_OK;
}

/*------------------------------------------------------------------------
 * Name:
 *
 * Description: Process op message. This will call op handler and send return
 * message to the other side.
 * This may enter recursive calling with ipc_do_op/ipc_do_cb from hander.
 *
 * Inputs:
 * Outputs: -
 * Returns: -
 -----------------------------------------------------------------------*/
static INT32 _proc_op(
    IPC_SESS_T *    pt_sess,
    VOID *          pv_msg,
    size_t          z_msg_size)
{
    CHAR *      ps_op;
    VOID *      pv_cb_addr;
    UINT32      ui4_num_args;
    ARG_DESC_T* pt_args;
    ARG_DESC_T  t_return;
    INT32       i4_ret;
    VOID *      pv_repack_msg;
    RPC_LOG(" Proc OP \n");

    if((i4_ret = ipc_packer_translate(pv_msg,
                                z_msg_size,
                                &ps_op,
                                &pv_cb_addr,
                                &pt_args,
                                &ui4_num_args)) != IPCR_OK)
    {
        return i4_ret;
    }

    pt_sess->ui4_ptHandle = pt_args[0].u.ui4_arg;

    RPC_LOG(" Call OP hdlr \n");

    if((i4_ret = _enter_sess(pt_sess, ps_op, 0)) == IPCR_OK)
    {
    _push_op(pt_sess, ps_op, TRUE);
    }else
    {
        RPC_ERR("err: enter sess fail\n");
        return i4_ret;
    }

    if((i4_ret = pt_sess->u.pf_proc_op(pt_sess->t_id,
                                   ps_op,
                                   ui4_num_args,
                                   pt_args,
                                   &t_return)) != IPCR_OK)
    {
        _leave_sess(pt_sess);
        return i4_ret;
    }
    _pop(pt_sess);
    _leave_sess(pt_sess);

    pv_repack_msg = malloc(MAX_MSG_SIZE);
    if(pv_repack_msg == NULL)
    {
        return IPCR_OUTOFMEMORY;
    } else {
		memset(pv_repack_msg, 0, MAX_MSG_SIZE);
    }

    if((i4_ret = ipc_packer_pack_ret(pv_msg,
                                     &t_return,
                                     pv_repack_msg,
                                     MAX_MSG_SIZE,
                                     &z_msg_size)) != IPCR_OK)
    {
        free(pv_repack_msg);
        return i4_ret;
    }

    RPC_LOG(" Send OP RET \n");

    if((i4_ret = _send_msg(pt_sess, IPCM_TYPE_RET, pv_repack_msg, z_msg_size)) != IPCR_OK)
    {
        free(pv_repack_msg);
        return i4_ret;
    }
    free(pv_repack_msg);
    return IPCR_OK;
}

/*------------------------------------------------------------------------
 * Name:
 *
 * Description: Process message from other side. This is root entry of the
 * message processing. It will redispatch message by message type. This may
 * enter recursive calling with ipc_do_op/ipc_do_cb in _proc_op/_proc_cb.
 *
 * Inputs:
 * Outputs: -
 * Returns: -
 -----------------------------------------------------------------------*/
static INT32 _proc_msg(
    IPC_SESS_T *    pt_sess,
    VOID *          pv_msg,
    ARG_DESC_T *    pt_return,
    UINT32          ui4_timeout,
    BOOL *          pb_returned)
{
    UINT32          ui4_msg_type;
    SIZE_T          z_msg_size;
    INT32           i4_ret;

    *pb_returned = FALSE;

    if ((i4_ret = _recv_msg(pt_sess, &ui4_msg_type, pv_msg, &z_msg_size, ui4_timeout))
                 == IPCR_OK)
    {
        switch (ui4_msg_type & IPCM_MASK_TYPE)
        {
            case IPCM_TYPE_OP:
                _proc_op(pt_sess, pv_msg, z_msg_size);
            break;
            case IPCM_TYPE_CB:
                _proc_cb(pt_sess, pv_msg, z_msg_size);
            break;
            case IPCM_TYPE_RET:
                _proc_ret(pt_sess, pv_msg, z_msg_size, pt_return);
                *pb_returned = TRUE;
            break;
            case IPCM_TYPE_ERROR:
                return IPCR_RCVD_ERR;
            break;
            default:
                /*TODO: ASSERT or call user's message proc function */
            break;
        }
    }
    return i4_ret;
}

/*------------------------------------------------------------------------
 * Name:
 *
 * Description: IPC Server thread main function. This will in a message processing
 * loop if the connection is not destroyed.
 *
 * Inputs:
 * Outputs: -
 * Returns: -
 -----------------------------------------------------------------------*/
static void  _server_ipc_thd(VOID * pv_arg)
{
    VOID *          pv_msg;
    IPC_SESS_T *    pt_sess;
    IPC_ID_T        t_id;
    INT32           i4_ret;
    BOOL            b_returned;

    t_id    = *(IPC_ID_T *)pv_arg;

    /* There is a ref when calling alloc_id before IPC thread
     * starts, so we don't add new ref.
     */
    pt_sess = _get_sess(t_id, FALSE, FALSE);
    IPC_ASSERT(pt_sess != NULL);

    if (NULL == pt_sess) //mtk40156 for klocwork issue
    {
        RPC_ERR("NULL == pt_sess\n");
        return ;
    }

    rpcu_push_logger(pt_sess->pf_log_fct, pt_sess->i4_log_level);

    /* Update IPC thread handle */
    pt_sess->pv_ipc_thd = rpcu_os_thread_self();

    rpcu_os_get_cur_thd_desc(&pt_sess->t_cur_thd_desc);

    /* This thread's default ID is t_id */
    _set_cur_id(t_id);

    printf("[RPCIPC]IPC Server thread(threadID=%p:t_id:%d)started\n",rpcu_os_thread_self(), (int)t_id);

    pv_msg = RPC_MALLOC(MAX_MSG_SIZE);
    if(pv_msg != NULL)
    {
        memset(pv_msg, 0, MAX_MSG_SIZE);
        do
        {
            i4_ret = _proc_msg(pt_sess, pv_msg, NULL, 0xffffffff, &b_returned);

            /* This should not happen, ignore or break?*/
            /*if(i4_ret == IPCR_OK && b_returned == TRUE)
            {
                continue;
            }*/

        } while(i4_ret == IPCR_OK);

        RPC_FREE(pv_msg);

        //RPC_INFO("IPC Thread (id:%d) is being finished:\n", (int)t_id);
    }

    if(pt_sess->pf_close != NULL)
    {
        pt_sess->pf_close(pt_sess->pv_tag);
    }

    _unref_sess(pt_sess);

    printf("[RPCIPC]IPC Server Thread(ThreadID=%p:t_id:%d) closed\n",rpcu_os_thread_self(), (int)t_id);

}

/*------------------------------------------------------------------------
 * Name:
 *
 * Description: Start IPC Server thread.
 *
 * Inputs:
 * Outputs: -
 * Returns: -
 -----------------------------------------------------------------------*/
static INT32 _server_start_ipc_thd(IPC_ID_T t_id)
{
    INT32 i4_ret;

    //printf("[RPCIPC]<Server>App Start IPC Listen thread threadID=0x%x\n", (unsigned int)rpcu_os_thread_self());

    if ((i4_ret = rpcu_os_create_thread(_server_ipc_thd, (VOID *)&t_id, sizeof(IPC_ID_T), 120, 1024 * 8)) != IPCR_OK)
    {
        RPC_ERR(" START IPC THREAD FAILED\n");
    }

    return i4_ret;
}

/*------------------------------------------------------------------------
 * Name:
 *
 * Description: IPC Client thread main function. This will in a message processing
 * loop if the connection is not destroyed.
 *
 * Inputs:
 * Outputs: -
 * Returns: -
 -----------------------------------------------------------------------*/
static void  _ipc_thd(VOID * pv_arg)
{
    VOID *          pv_msg;
    IPC_SESS_T *    pt_sess;
    IPC_ID_T        t_id;
    INT32           i4_ret;
    BOOL            b_returned;

    //printf("[RPCIPC]using current IPC client thread=0x%x\n", (unsigned int)rpcu_os_thread_self());
    t_id    = *(IPC_ID_T *)pv_arg;

    /* There is a ref when calling alloc_id before IPC thread
     * starts, so we don't add new ref.
     */
    pt_sess = _get_sess(t_id, FALSE, FALSE);
    IPC_ASSERT(pt_sess != NULL);

    if (NULL == pt_sess) //mtk40156 for klocwork issue
    {
        RPC_ERR("NULL == pt_sess\n");
        return ;
    }

    rpcu_push_logger(pt_sess->pf_log_fct, pt_sess->i4_log_level);

    /* Update IPC thread handle */
    pt_sess->pv_ipc_thd = rpcu_os_thread_self();

    rpcu_os_get_cur_thd_desc(&pt_sess->t_cur_thd_desc);

    /* This thread's default ID is t_id */
    _set_cur_id(t_id);

    printf("[RPCIPC]IPC client thread started(ThreadID==%p :t_id=%d)\n", pt_sess->pv_ipc_thd, (int)t_id);

    pv_msg = RPC_MALLOC(MAX_MSG_SIZE);
    if(pv_msg != NULL)
    {
        memset(pv_msg, 0, MAX_MSG_SIZE);
        do
        {
            i4_ret = _proc_msg(pt_sess, pv_msg, NULL, 0xffffffff, &b_returned);

            /* This should not happen, ignore or break?*/
            /*if(i4_ret == IPCR_OK && b_returned == TRUE)
            {
                continue;
            }*/

        } while(i4_ret == IPCR_OK);

        RPC_FREE(pv_msg);

        RPC_LOG("[RPCIPC]<client>IPC Thread (id:%d) is being finished:\n", (int)t_id);
    }

    if(pt_sess->pf_close != NULL)
    {
        pt_sess->pf_close(pt_sess->pv_tag);
    }

    _unref_sess(pt_sess);

    printf("[RPCIPC]IPC client thread(ThreadID==%p :t_id=%d)closed\n",rpcu_os_thread_self(), (int)t_id);

}

/*------------------------------------------------------------------------
 * Name:
 *
 * Description: Start IPC Client thread.
 *
 * Inputs:
 * Outputs: -
 * Returns: -
 -----------------------------------------------------------------------*/
static INT32 _start_ipc_thd(IPC_ID_T t_id)
{
    INT32 i4_ret;

    printf("[RPCIPC]<client>App Thread(%p) create ipc thread-----\n", rpcu_os_thread_self());

    if((i4_ret = rpcu_os_create_thread(_ipc_thd, (VOID *)&t_id, sizeof(IPC_ID_T), 120, 1024 * 8)) != IPCR_OK)
    {
        RPC_ERR(" START IPC THREAD FAILED\n");
    }

    return i4_ret;
}

/* CMMD > 0 used to transfor the server id as the seria number. */
#define IPC_ACK_CMD_REQ   ((INT32)0)
#define IPC_ACK_CMD_SUCC  ((INT32)-1)
#define IPC_ACK_CMD_FAIL  ((INT32)-2)

/*------------------------------------------------------------------------
 * Name:
 *
 * Description: IPC listen thread main function. This will wait for connections
 * and confirm ACK for dual-link. It will allocate new IPC id for new connection.
 * This will exit if the server is closed.
 *
 * Inputs:
 * Outputs: -
 * Returns: -
 -----------------------------------------------------------------------*/
static VOID _ipc_listen_thd(VOID * pv_arg)
{
    IPC_ID_T    t_id;
    IPC_ID_T    t_new_id;
    IPC_SESS_T* pt_sess;

    INT32       i4_new_sock;
    INT32       i4_ret;
    UINT32      i4_cmd;
#if 0  //add for 64bit compatible, @4.10
		CHAR		recv_buf[12];
#else
		CHAR		recv_buf[sizeof(MSG)];
#endif
    IPC_CLIENT_T * pt_client;

    t_id = *(IPC_ID_T *)pv_arg;

    /* A ref count is reserved by alloc_id */
    pt_sess = _get_sess(t_id, FALSE, FALSE);
    IPC_ASSERT(pt_sess != NULL);

    if (NULL == pt_sess) //mtk40156 for klocwork issue
    {
        RPC_ERR("NULL == pt_sess\n");
        return ;
    }

    rpcu_push_logger(pt_sess->pf_log_fct, pt_sess->i4_log_level);

    while(1)
    {
        i4_new_sock = accept(pt_sess->i4_it_sock, NULL, NULL);

        RPC_LOG("Accepted a sock, %d\n", (int)i4_new_sock);
        if(i4_new_sock < 0)
        {
            break;
        }

        MSG * msg = (MSG*)RPC_MALLOC(sizeof(MSG));
        if (NULL == msg)
        {
            RPC_ERR("Cannot alloc new msg\n");
            break;
        }
        /* FIX ME, TMP */
        if(recv(i4_new_sock, recv_buf, sizeof(recv_buf), 0) == sizeof(recv_buf))
        {
            memcpy(msg,recv_buf,sizeof(recv_buf));
	     i4_cmd = msg->i4_cmd;
         printf("msg rx from fd: %d, cmd=%u, svr_ref=%p, ct_ref=%p\n",
                i4_new_sock, i4_cmd, msg->rmt_ref, msg->self_ref);
	     if(NULL == msg->rmt_ref)
            {
                if(i4_cmd == IPC_ACK_CMD_REQ)
                {
                    pt_client = (IPC_CLIENT_T *)RPC_MALLOC(sizeof(IPC_CLIENT_T));
                    if (NULL == pt_client)
                    {
                        RPC_ERR("Cannot alloc new IPC CLIENT\n");
                        RPC_FREE(msg);
                        break;
                    }
                    memset(pt_client, 0, sizeof(IPC_CLIENT_T));
                    rpcu_os_tls_set(pv_key_client,pt_client);
                    pt_client->pv_remote_tag = msg->self_ref;
                    t_new_id = _alloc_id(pt_sess->pv_tag,
                                     pt_sess->pf_close,
                                     pt_sess->pf_proc_msg,
                                     pt_sess->u.pf_proc_op,
                                     NULL,
                                     i4_new_sock,
                                                -1,
                                                pt_sess->i4_log_level,
                                                pt_sess->pf_log_fct);
                    add_sess_t(pt_client,t_new_id);
                    if(t_new_id == 0)
                    {
                        RPC_ERR("[RPCIPC]<Server>Cannot alloc t_id for connection\n");
                        close(i4_new_sock);
						if (msg != NULL)
						{
						   RPC_FREE(msg); //MTK40438 add for kloc issue
						}
                        continue;
                    }
					printf("[RPCIPC]<Server>alloc t_id =%d @line:%d\n",(int)t_new_id,__LINE__);
                    if ((i4_ret = _server_start_ipc_thd(t_new_id)) != IPCR_OK)
                    {
                        RPC_ERR("Cannot start thread new IPC id for connection\n");
                        close(i4_new_sock);
						if (msg != NULL)
						{
						   RPC_FREE(msg); //MTK40438 add for kloc issue
						}
                        continue;
                    }
                    msg->i4_cmd = t_new_id;
                    msg->rmt_ref = msg->self_ref;
                    msg->self_ref = pt_client;
                    send(i4_new_sock, (char*)msg, sizeof(MSG), 0);
                }
                else if(msg->self_ref==NULL)
                {
                    if(_set_ct_sock((IPC_ID_T)i4_cmd, i4_new_sock) == IPCR_OK)
                    {
                        i4_cmd = IPC_ACK_CMD_SUCC;
                        send(i4_new_sock, &i4_cmd, sizeof(INT32), 0);
                    }
                    else
                    {
                        i4_cmd = IPC_ACK_CMD_FAIL;
                        send(i4_new_sock, &i4_cmd, sizeof(INT32), 0);
                        RPC_ERR("Cannot finish ACK for connection\n");
                        close(i4_new_sock);
						if (msg != NULL)
						{
						   RPC_FREE(msg); //MTK40438 add for kloc issue
						}
                        continue;
                    }
                }
            }
            else
            {
                if(i4_cmd == IPC_ACK_CMD_REQ)
                {
                    pt_client = msg->rmt_ref;
                    t_new_id = _alloc_id(pt_sess->pv_tag,
                                         pt_sess->pf_close,
                                         pt_sess->pf_proc_msg,
                                         pt_sess->u.pf_proc_op,
                                         NULL,
                                         i4_new_sock,
                                         -1,
                                         pt_sess->i4_log_level,
                                         pt_sess->pf_log_fct);
                    add_sess_t(pt_client,t_new_id);
                    if(t_new_id == 0)
                    {
                        RPC_ERR("[RPCIPC]<Server>Cannot alloc t_id for connection\n");
                        close(i4_new_sock);
						if (msg != NULL)
						{
						   RPC_FREE(msg); //MTK40438 add for kloc issue
						}
                        continue;
                    }
					printf("[RPCIPC]<Server>alloc t_id =%d @line:%d\n",(int)t_new_id,__LINE__);
                    if ((i4_ret = _server_start_ipc_thd(t_new_id)) != IPCR_OK)
                    {
                        RPC_ERR("Cannot start thread new IPC id for connection\n");
                        close(i4_new_sock);
						if (msg != NULL)
						{
						   RPC_FREE(msg); //MTK40438 add for kloc issue
						}
                        continue;
                    }
                    send(i4_new_sock, &t_new_id, sizeof(INT32), 0);
                }
                else
                    {
                    if(_set_ct_sock((IPC_ID_T)i4_cmd, i4_new_sock) == IPCR_OK)
                    {
                        i4_cmd = IPC_ACK_CMD_SUCC;
                        send(i4_new_sock, &i4_cmd, sizeof(INT32), 0);
                    }
                    else
                    {
                        i4_cmd = IPC_ACK_CMD_FAIL;
                        send(i4_new_sock, &i4_cmd, sizeof(INT32), 0);
                        RPC_ERR("Cannot finish ACK for connection\n");
                        close(i4_new_sock);
						if (msg != NULL)
						{
						   RPC_FREE(msg); //MTK40438 add for kloc issue
						}
                        continue;
                    }
                }
            }
        }
        else
        {
            RPC_ERR("ACK Error\n");
            close(i4_new_sock);
			if (msg != NULL)
			{
			   RPC_FREE(msg); //MTK40438 add for kloc issue
			}
            continue;
        }
		if (msg != NULL)
	 	{
	 		RPC_FREE(msg); //MTK40438 add for kloc issue
		}
    }
    printf("[RPCIPC]<Server>Listen thread exists\n");
    _unref_sess(pt_sess);
}

/*------------------------------------------------------------------------
 * Name:
 *
 * Description: Process message in a loop for repeated CB messages.
 * This will return if a return message is processed.
 *
 * Inputs:
 * Outputs: -
 * Returns: -
 -----------------------------------------------------------------------*/
static INT32 _proc_loop(
    IPC_SESS_T *    pt_sess,
    ARG_DESC_T *    pt_return,
    UINT32          ui4_timeout)
{
    INT32   i4_ret = IPCR_OK;


    BOOL    b_returned;
    VOID *  pv_msg;

    pv_msg = malloc(MAX_MSG_SIZE);
    if(pv_msg == NULL)
    {
        return IPCR_OUTOFMEMORY;
    } else {
		memset(pv_msg, 0, MAX_MSG_SIZE);
    }

    do
    {
        i4_ret = _proc_msg(pt_sess, pv_msg, pt_return, 0xffffffff, &b_returned);
        if(i4_ret == IPCR_OK && b_returned == TRUE)
        {
            break;
        }
    }while(i4_ret == IPCR_OK);

    free(pv_msg);
    /*Fix here, i4_ret*/
    return i4_ret;
}

/*------------------------------------------------------------------------
 * Name:
 *
 * Description: Trace IPC stack.
 *
 * Inputs:
 * Outputs: -
 * Returns: -
 -----------------------------------------------------------------------*/
VOID ipc_trace_stack()
{
    UINT32 ui4_i;

    rpcu_os_mutex_lock(pv_tbl_lock);
    for(ui4_i = 0; ui4_i < ui4_max_sess_num; ui4_i ++)
    {
        if(pt_sess_tbl[ui4_i].ui4_refc != 0)
        {

            if(pt_sess_tbl[ui4_i].pt_stack_top_in_ctx_thread != NULL)
            {
                RPC_LOG("<%s> CT Stack: \n", g_ps_exe_name);
                _trace_stack(pt_sess_tbl[ui4_i].pt_stack_top_in_ctx_thread);
            }
            if(pt_sess_tbl[ui4_i].pt_stack_top_in_ipc_thread != NULL)
            {
                RPC_LOG("<%s> IT Stack: \n", g_ps_exe_name);
                _trace_stack(pt_sess_tbl[ui4_i].pt_stack_top_in_ipc_thread);
            }
        }
    }
    rpcu_os_mutex_unlock(pv_tbl_lock);


}

/*------------------------------------------------------------------------
 * Name:
 *
 * Description: IPC monitor thread main function.
 *
 * Inputs:
 * Outputs: -
 * Returns: -
 -----------------------------------------------------------------------*/
static VOID _monitor_main(VOID * pv_arg)
{

    while(1)
    {
        sleep(5);
        ipc_trace_stack();
        RPC_LOG("\n");
    }
}

/*------------------------------------------------------------------------
 * Name:
 *
 * Description: Start IPC monitor thread.
 *
 * Inputs:
 * Outputs: -
 * Returns: -
 -----------------------------------------------------------------------*/
static VOID _monitor_start()
{
    INT32 i4_ret;

    RPC_INFO(" Start IPC Monitor\n");

    if((i4_ret = rpcu_os_create_thread(
        _monitor_main,
        NULL, 0, 120, 1024 * 8)) != IPCR_OK)
    {
        RPC_ERR(" START IPC MONITOR THREAD FAILED\n");
    }


}

/*------------------------------------------------------------------------
 * Name:
 *
 * Description: Set client close func and tag.
 *
 * Inputs:
 * Outputs: -
 * Returns: -
 -----------------------------------------------------------------------*/
INT32 ipc_set_close_fct(IPC_ID_T t_ipc_id, void * pv_tag, ipc_close_fct pf_close)
     /*Need chain? ..., void ** ppv_tag, ipc_close_fct * ppf_close*/
{
    IPC_SESS_T *    pt_sess = NULL;
    //INT32           i4_ret  = IPCR_OK;

    pt_sess = _get_sess(t_ipc_id, TRUE, FALSE);

    if(pt_sess != NULL)
    {
        pt_sess->pf_close = pf_close;
        pt_sess->pv_tag   = pv_tag;

        return IPCR_OK;
    }
    else
    {
        return IPCR_NOT_OPENED;
    }
}

/*------------------------------------------------------------------------
 * Name:
 *
 * Description: This API is called by a client and requests the server to
 *              perform an operation as specified in argument 'ps_op'.
 *              This function is a synchronous operation and will only return
 *              once the server has completed the requested operation.
 *
 * Inputs:
 * Outputs: -
 * Returns: -
 -----------------------------------------------------------------------*/
INT32 ipc_do_op (
    IPC_ID_T                  t_ipc_id,
    const CHAR*               ps_op,
    UINT32                    ui4_num_args,
    ARG_DESC_T*               pt_args,
    ARG_DESC_T*               pt_return,
    UINT32                    ui4_timeout,
    ipc_get_rpc_desc_fct      pf_get_rpc_desc,
    VOID*                     pv_tag)
{
    VOID *          pv_msg = NULL;
    SIZE_T          z_msg_size;
    IPC_SESS_T *    pt_sess = NULL;
    INT32           i4_ret = IPCR_OK;

    /*TODO: RPC should check these. Assert here? */
    if(   ps_op     == NULL
       || pt_return == NULL
       || (pt_args  == NULL && ui4_num_args != 0)
       || (pt_args  != NULL && ui4_num_args == 0))
    {
        RPC_ERR("Invalid arg "
            "t_ipc_id:%d, "
            "ps_op:%s, "
            "pt_args:0x%x, ui4_num_args: %d,"
            "pt_return:0x%x\n",
            t_ipc_id,
            ps_op,
            pt_args, ui4_num_args,
            pt_return);

        return IPCR_INV_ARGS;
    }

    pt_sess = _get_sess(t_ipc_id, TRUE, FALSE);

    if(pt_sess != NULL)
    {
        RPC_LOG(" Start ipc_do_op in \n");
        pv_msg = RPC_MALLOC(MAX_MSG_SIZE);

        if(pv_msg != NULL)
        {
            memset(pv_msg, 0, MAX_MSG_SIZE);
            if((i4_ret = ipc_packer_pack(pv_msg,
                                  MAX_MSG_SIZE,
                                  ps_op,
                                  NULL,
                                  ui4_num_args,
                                  pt_args,
                                  pf_get_rpc_desc,
                                  pv_tag,
                                  &z_msg_size)) == IPCR_OK)
            {
                if((i4_ret = _lock_sess_table(pt_sess)) == IPCR_OK)
                {
                    if((i4_ret = _send_msg(pt_sess, IPCM_TYPE_OP, pv_msg, z_msg_size)) == IPCR_OK)
                    {
                        _push_op(pt_sess, ps_op, FALSE);

                        i4_ret = _proc_loop(pt_sess, pt_return, ui4_timeout);

                        _pop(pt_sess);
                    }
                    _unlock_sess_table(pt_sess);
                }
            }
        }
        else
        {
            i4_ret = IPCR_OUTOFMEMORY;
        }
    }
    else
    {
        RPC_ERR("YZ pt_sess is NULL\n");

        i4_ret = IPCR_NOT_OPENED;
    }

    if(pt_sess != NULL)
    {
        _unref_sess(pt_sess);
    }
    if(pv_msg != NULL)
    {
        RPC_FREE(pv_msg);
    }

    if(i4_ret != IPCR_OK)
    {
        RPC_ERR("ipc_do_op %s failed ret:%d\n", ps_op, (int)(i4_ret));
    }
    return i4_ret;

}

/*------------------------------------------------------------------------
 * Name:
 *
 * Description: Do callback to client. This function is a synchronous
 *              operation and will only return once the client has completed
 *              the requested asynchronous callback operation.
 *
 * Inputs:
 * Outputs: -
 * Returns: -
 -----------------------------------------------------------------------*/
INT32 ipc_do_cb(
    IPC_ID_T                  t_ipc_id,
    const CHAR*               ps_cb_type,
    VOID*                     pv_cb_addr,
    UINT32                    ui4_num_args,
    ARG_DESC_T*               pt_args,
    ARG_DESC_T*               pt_return,
    UINT32                    ui4_timeout,
    ipc_get_rpc_desc_fct      pf_get_rpc_desc,
    VOID*                     pv_tag)
{
    VOID *          pv_msg  = NULL;
    SIZE_T          z_msg_size = 0;
    IPC_SESS_T *    pt_sess = NULL;
    int           i4_ret  = IPCR_OK;

    /*TODO: RPC should check these. Assert here? */
    if(   ps_cb_type    == NULL
       || pt_return     == NULL
       || (pt_args      == NULL && ui4_num_args != 0)
       || (pt_args      != NULL && ui4_num_args == 0)
       || t_ipc_id      </*=*/ 0)
       //|| pv_cb_addr    == NULL)
    {
        RPC_ERR("Invalid arg "
                "t_ipc_id:%d, "
                "pv_cb_addr:0x%x, "
                "ps_cb_type:%s, "
                "pt_args:0x%x, ui4_num_args: %d,"
                "pt_return:0x%x\n",
                t_ipc_id,
                pv_cb_addr,
                ps_cb_type,
                pt_args, ui4_num_args,
                pt_return);

        return IPCR_INV_ARGS;
    }


    pt_sess = _get_sess_ex(t_ipc_id, TRUE, FALSE);

    if(pt_sess != NULL)
    {
        RPC_LOG(" Start ipc_do_cb \n");
        pv_msg = RPC_MALLOC(MAX_MSG_SIZE);

        if(pv_msg != NULL)
        {
            memset(pv_msg, 0, MAX_MSG_SIZE);
            if((i4_ret = ipc_packer_pack(pv_msg,
                                  MAX_MSG_SIZE,
                                  ps_cb_type,
                                  pv_cb_addr,
                                  ui4_num_args,
                                  pt_args,
                                  pf_get_rpc_desc,
                                  pv_tag,
                                  &z_msg_size)) == IPCR_OK)
            {
                if((i4_ret = _lock_sess_table(pt_sess)) == IPCR_OK)
                {
                    if((i4_ret = _send_msg(pt_sess, IPCM_TYPE_CB, pv_msg, z_msg_size)) == IPCR_OK)
                    {
                        _push_op(pt_sess, ps_cb_type, FALSE);

                        i4_ret = _proc_loop(pt_sess, pt_return, ui4_timeout);

                        _pop(pt_sess);
                    }
                    _unlock_sess_table(pt_sess);
                }
            }
        }
        else
        {
            i4_ret = IPCR_OUTOFMEMORY;
        }
    }
    else
    {
        i4_ret = IPCR_NOT_OPENED;
    }

    if(pt_sess != NULL)
    {
        _unref_sess(pt_sess);
    }
    if(pv_msg != NULL)
    {
        RPC_FREE(pv_msg);
		pv_msg = NULL;
    }

    if(i4_ret != IPCR_OK)
    {
    	if ( i4_ret != IPCR_TIMEOUT)
        {
            RPC_ERR("ipc_do_cb %s failed ret:%d\n", ps_cb_type, i4_ret);
        }
    }

    return i4_ret;
}


/*------------------------------------------------------------------------
 * Name:
 *
 * Description: This API is called by a server and opens the server's communication port.
 * This will create listening socket and start listening thread.
 *
 * Inputs:
 * Outputs: -
 * Returns: -
 -----------------------------------------------------------------------*/
IPC_ID_T ipc_open_server (
    const CHAR*          ps_server,
    VOID *               pv_tag,
    ipc_close_fct        pf_close,
    ipc_proc_msg_fct     pf_proc_msg,
    ipc_proc_rpc_op_fct  pf_proc_rpc_op)
{
    if(pt_sess_tbl == NULL) return RPCR_OUTOFMEMORY;

    IPC_ID_T            t_id;
    INT32               i4_ret;
    struct sockaddr_un  t_serv_addr;
    int                 i4_sock;

    rpc_log_fct         pf_log_fct = NULL;
    INT32               i4_log_level = RPC_LOG_NONE;

    if(ps_server == NULL || pf_proc_rpc_op == NULL)
    {
        return IPCR_INV_ARGS;
    }

    rpcu_logger_stack_get(&pf_log_fct, &i4_log_level);

    i4_sock  = socket(PF_UNIX, SOCK_STREAM, 0);
    if(i4_sock < 0)
    {
       RPC_ERR(" SOCK create errno:%d\n", (int)errno);
       return IPCR_CNN_FAIL;

    }

    if(strlen(t_cfg.ps_sock_path) + strlen(ps_server) >= sizeof(t_serv_addr.sun_path) - 1)
    {
        close(i4_sock);
        RPC_ERR("Too long IPC server name or path name\n");
        return IPCR_INV_SERVER;
    }

    t_serv_addr.sun_family = AF_UNIX;
    snprintf(t_serv_addr.sun_path, sizeof(t_serv_addr.sun_path), "%s/%s", t_cfg.ps_sock_path, ps_server);

    mkdir(t_cfg.ps_sock_path, 0777);
    unlink(t_serv_addr.sun_path);

    if(bind(i4_sock, (const void *)&t_serv_addr, sizeof(struct sockaddr_un)) == -1)
    {
        close(i4_sock);
        RPC_ERR(" BIND ERRRRR, SOCK [%d] errno:%d\n", (int)i4_sock, (int)errno);
        return IPCR_CNN_FAIL;
    }

    /*FIXME*/
    listen(i4_sock, 256);

    /* Here there will be a ref count to sess and unrefed in ipc thread */
    t_id = _alloc_id(
        pv_tag,
        pf_close,
        pf_proc_msg,
        pf_proc_rpc_op,
        NULL,
        i4_sock,
        -1,
        i4_log_level,
        pf_log_fct);
    if(t_id != (IPC_ID_T)0)
    {
        _set_cur_id(t_id);
        i4_ret = rpcu_os_create_thread(_ipc_listen_thd, (VOID *)&t_id, sizeof(IPC_ID_T), 120, 4096);
        if(i4_ret != IPCR_OK)
        {
            _unref_sess(_get_sess(t_id, FALSE, FALSE));
            return i4_ret;
        }
    }
    else
    {
        close(i4_sock);
        RPC_ERR("[RPCIPC]<Server> Allocate Listen thread ID failed\n");
    }
    return t_id;

}

/*------------------------------------------------------------------------
 * Name:
 *
 * Description: This will close server. Close all the linkage.
 *
 * Inputs:
 * Outputs: -
 * Returns: -
 -----------------------------------------------------------------------*/
INT32 ipc_close_server (IPC_ID_T  t_ipc_id)
{
    /* TODO: Close all the connections ? */
    IPC_SESS_T * pt_sess;

    if(t_ipc_id == RPC_DEFAULT_ID)
    {
        return IPCR_INV_ARGS;
    }

    pt_sess = _get_sess(t_ipc_id, TRUE, FALSE);
    if(pt_sess != NULL)
    {
        shutdown(pt_sess->i4_it_sock, SHUT_RDWR);
        shutdown(pt_sess->i4_ct_sock, SHUT_RDWR);
        _unref_sess(pt_sess);
		printf("[RPCIPC]<Server>ipc_close_server closed\n");
        return IPCR_OK;
    }
    else
    {
        printf("[RPCIPC]<Server>ipc server is not exist\n");
        return IPCR_NOT_OPENED;
    }

}

/*-----------------------------------------------------------------------
*
*Description:Add a new session to linked list maintained by a pt_client.Both the client and
*                  server can call this API while a new link created.
*
-------------------------------------------------------------------------*/
VOID add_sess_t(IPC_CLIENT_T * pt_client, IPC_ID_T t_id)
{
	IPC_SESS_T * pt_sess;
	pt_sess = &(pt_sess_tbl[t_id]);
	pt_sess->pt_client = pt_client;
	pt_client->pt_sess_tail = NULL;

	if((pt_sess->pt_prev = pt_client->pt_sess_tail)!=NULL)
	{
        printf("[RPCIPC]1 add_sess_t():t_id=%d,i4_it_sock=%d, i4_ct_sock=%d\n",(int)t_id, pt_sess->i4_it_sock, pt_sess->i4_ct_sock);
            (pt_sess->pt_prev)->pt_next = pt_sess;
        printf("[RPCIPC]1 add_sess_t():t_id=%d,i4_it_sock=%d, i4_ct_sock=%d\n",(int)t_id, pt_sess->i4_it_sock, pt_sess->i4_ct_sock);
	}
	else
	{
            pt_client->pt_sess_head = pt_sess;
        printf("[RPCIPC]2 add_sess_t():t_id=%d,i4_it_sock=%d, i4_ct_sock=%d\n",(int)t_id, pt_sess->i4_it_sock, pt_sess->i4_ct_sock);
	}
	pt_sess->pt_next = NULL;
	pt_client->pt_sess_tail = pt_sess;
}

#ifdef RPC_ENABLE_MULTI_CLIENT

IPC_CLIENT_T* ipc_client_exist(
    const CHAR*          ps_server)
{
    int i = 0;
    for(i=0;i<IPC_CLIENT_NUM;i++)
    {
        if (NULL != pt_clients[i])
        {
            if(0 == strncmp(ps_server, pt_clients[i]->server_name, IPC_SERVER_NAME_LENGTH-1))
            {
                return pt_clients[i];
            }
        }
    }
    return NULL;
}

BOOL ipc_save_client(
    IPC_CLIENT_T * pt_client)
{
    int i = 0;
    for(i=0;i<IPC_CLIENT_NUM;i++)
    {
        if (NULL == pt_clients[i])
        {
            pt_clients[i] = pt_client;
            return TRUE;
        }
    }
    printf("save client fail, client=%p\n", pt_client);

    return FALSE;
}
#endif
/*------------------------------------------------------------------------
 * Name:
 *
 * Description: This API is called by a client and opens a communication link
 *              to the server. One client may only open one communication link
 *              with one server. However, one client may open multiple
 *              communication links with different servers.
 *              This will connect to server by dual-connection. And it will send
 *              and reseive ACK messages to build dual connections.
 *
 * Inputs:
 * Outputs: -
 * Returns: -
 -----------------------------------------------------------------------*/
IPC_ID_T ipc_open_client (
    const CHAR*          ps_server,
    VOID *               pv_tag,
    ipc_close_fct        pf_close,
    ipc_proc_msg_fct     pf_proc_msg,
    ipc_proc_rpc_cb_fct  pf_proc_rpc_cb)
{
    struct sockaddr_un  t_serv_addr;
    int                 i4_it_sock  = -1;
    int                 i4_ct_sock  = -1;
    int                 i4_ret      = IPCR_OK;
    IPC_ID_T            t_id        = 0;
    INT32               i4_cmd      = 0;
#if 0  //add for 64bit compatible, @4.10
		CHAR		recv_buf[12];
#else
		CHAR		recv_buf[sizeof(MSG)];
#endif

    rpc_log_fct         pf_log_fct = NULL;
    INT32               i4_log_level = RPC_LOG_NONE;
#ifdef RPC_ENABLE_MULTI_CLIENT
    IPC_CLIENT_T * pt_client;
#endif
    printf("[RPCIPC]<Client>App(threadID=%p)call ipc_open_client\n",rpcu_os_thread_self());

    if(pt_sess_tbl == NULL)
    {
        RPC_ERR("[RPCIPC]<Client>pt_sess_tbl is NULL\n");
        RPC_ERR("[RPCIPC]<Client>App open a IPC client failed,error:%d\n",RPCR_OUTOFMEMORY);
        return RPCR_OUTOFMEMORY;
    }

    if(ps_server == NULL || pf_proc_rpc_cb == NULL)
    {
        RPC_ERR("[RPCIPC]<Client>%s:ps_server or pf_proc_rpc_cb is NULL\n");
        RPC_ERR("[RPCIPC]<Client>App open a IPC client failed,error:%d\n",IPCR_INV_ARGS);
        return IPCR_INV_ARGS;
    }

    rpcu_logger_stack_get(&pf_log_fct, &i4_log_level);


    /* FixMe , how to find the size limit of sun_path */
    if(strlen(t_cfg.ps_sock_path) + strlen(ps_server) >= sizeof(t_serv_addr.sun_path) - 1)
    {
        RPC_ERR("[RPCIPC]<Client>Too long IPC server name or path name\n");
        RPC_ERR("[RPCIPC]<Client>App open a IPC client failed,error:%d\n",IPCR_INV_SERVER);
        return IPCR_INV_SERVER;
    }

	#if 1  //mtk40074 add for multiclient using the same thread
    IPC_SESS_T * pt_sess = NULL;
    pt_sess = _get_sess(IPC_DEFAULT_ID, TRUE, TRUE);
    if(pt_sess != NULL)
    {
        printf("SN test ipc_open_client(), this thread already has valid session, tid:%d\n", pt_sess->t_id);
        return pt_sess->t_id;
    }
    #endif

    t_serv_addr.sun_family = AF_UNIX;
    snprintf(t_serv_addr.sun_path, sizeof(t_serv_addr.sun_path), "%s/%s", t_cfg.ps_sock_path, ps_server);

    i4_ct_sock  = socket(PF_UNIX, SOCK_STREAM, 0);
    if(i4_ct_sock < 0)
    {
        RPC_ERR("[YZ RPCIPC]<Client>create CT socket errno:%d\n", (int)errno);
       i4_ret = IPCR_CNN_FAIL;
    }

    printf("[YZ RPCIPC]<Client>create CT socket fd: %d\n",i4_ct_sock);

    if(i4_ret == IPCR_OK)
    {
        printf("[RPCIPC]<Client>create it socket()\n");
        i4_it_sock  = socket(PF_UNIX, SOCK_STREAM, 0);
        if(i4_it_sock < 0)
        {
            RPC_ERR("[RPCIPC]<Client>create it socket errno:%d\n", (int)errno);
           i4_ret = IPCR_CNN_FAIL;
        }
    }

    printf("[RPCIPC]<Client>%s create IT socket fd: %d\n",__FUNCTION__, i4_it_sock);

    MSG * msg = (MSG*)RPC_MALLOC(sizeof(MSG));
    if ( NULL == msg )
    {
        printf("RPC_MALLOC for msg is error\n");
        return IPCR_OUTOFMEMORY;
    }

    if(i4_ret == IPCR_OK)
    {
        printf("ct connect()...\n");
        i4_ret = connect(i4_ct_sock, (const void *)&t_serv_addr, sizeof(struct sockaddr_un));
        if(i4_ret < 0)
        {
            RPC_ERR("[RPCIPC]<Client> SOCK Connection fail errno:%d\n", (int)errno);
            i4_ret = IPCR_CNN_FAIL;
#ifdef RPC_ENABLE_MULTI_CLIENT
            goto __OPEN_FAIL;
#endif
        }
        printf("[RPCIPC]<Client>ct connect Server IPC successful\n");
    }

    //printf("[RPCIPC]<Client>RPC_MALLOC()...\n");

	IPC_ASSERT(msg != NULL); //MTK40438 add for kloc issue
#ifdef RPC_ENABLE_MULTI_CLIENT
    pt_client = ipc_client_exist(ps_server);
    printf("check client %p server %p for service %s\n",
        pt_client, pt_client!=NULL?pt_client->pv_remote_tag:NULL, ps_server);
    if(NULL == pt_client)
#else
    if(TRUE == b_client_new)
#endif
    {
        RPC_INFO("TRUE == b_client_new()...\n");
#ifndef RPC_ENABLE_MULTI_CLIENT
        b_client_new = FALSE;
#endif
        pt_client = (IPC_CLIENT_T *)RPC_MALLOC(sizeof(IPC_CLIENT_T));
        if ( NULL == pt_client )
        {
            printf("[RPCIPC]<Client>RPC_MALLOC pt_client fail\n");
            if ( msg != NULL )
            {
                printf("[RPCIPC]<Client>RPC_FREE msg\n");
                RPC_FREE(msg);
                msg = NULL;
            }
            return IPCR_OUTOFMEMORY;
        }
        IPC_ASSERT(pt_client != NULL);
		if (pt_client != NULL) //MTK40438 , ADD for kloc issue
        memset(pt_client, 0, sizeof(IPC_CLIENT_T));

        printf("[RPCIPC]<Client>rpcu_os_tls_set()...\n");

#ifdef RPC_ENABLE_MULTI_CLIENT
        strncpy(pt_client->server_name, ps_server, IPC_SERVER_NAME_LENGTH-1);
        ipc_save_client(pt_client);
#endif

        rpcu_os_tls_set(pv_key_client,pt_client);
		if (msg != NULL) //MTK40438, add for kloc issue
        {
        msg->i4_cmd = IPC_ACK_CMD_REQ;
        msg->self_ref = pt_client;
        msg->rmt_ref = 0;
		}else
		{
            printf("[RPCIPC]<Client> msg is NULL, Line: %d\n", __LINE__);
		}
    if(i4_ret == IPCR_OK)
    {
        i4_cmd = IPC_ACK_CMD_REQ;
            printf("[RPCIPC]<Client>ct sock  send()...\n");
            if(send(i4_ct_sock, (CHAR*)msg, sizeof(MSG), 0) <= 0)
        {
                RPC_ERR("[RPCIPC]<Client> ACK Fail (%d)\n", (int)i4_cmd);
                i4_ret = IPCR_CNN_FAIL;
            }
        }
        if(i4_ret == IPCR_OK)
        {
            printf("[RPCIPC]<Client>ct recv()...\n");
            if(recv(i4_ct_sock, recv_buf, sizeof(recv_buf), 0) != sizeof(recv_buf))
             {
                RPC_ERR("[RPCIPC]<Client> ACK Fail (%d)\n", (int)i4_cmd);
                i4_ret = IPCR_CNN_FAIL;
            }
            else
            {
                memcpy(msg,recv_buf,sizeof(recv_buf));
                i4_cmd = msg->i4_cmd;
                pt_client->pv_remote_tag = msg->self_ref;
            }
        }
        if(i4_ret == IPCR_OK)
        {
            if(connect(i4_it_sock, (const void *)&t_serv_addr, sizeof(struct sockaddr_un)) < 0)
            {
                RPC_ERR("[RPCIPC]<Client> SOCK Connection fail errno:%d\n", (int)errno);
                i4_ret = IPCR_CNN_FAIL;
            }
        }

        if(i4_ret == IPCR_OK)
        {
            msg->i4_cmd = i4_cmd;
            msg->self_ref = 0;
            msg->rmt_ref = 0;
            int i_temp;
            if((i_temp=send(i4_it_sock, (char*)msg, sizeof(MSG), 0) )!= sizeof(MSG))
            {
                RPC_ERR("[RPCIPC]<Client> ACK Fail (%d)\n", (int)i4_cmd);
                i4_ret = IPCR_CNN_FAIL;
            }
        }
    }
    else
    {
        rpcu_os_tls_set(pv_key_client,pt_client);
		if (msg != NULL) //MTK40438, add for kloc issue
        {
        msg->i4_cmd = IPC_ACK_CMD_REQ;
        msg->self_ref = pt_client;
        msg->rmt_ref = pt_client->pv_remote_tag;
		}else
		{
            printf("[RPCIPC]<Client> msg is NULL, Line: %d\n", __LINE__);
		}
        if(i4_ret == IPCR_OK)
        {
            i4_cmd = IPC_ACK_CMD_REQ;
            if(send(i4_ct_sock, (CHAR*)msg, sizeof(MSG), 0) <= 0)
            {
                RPC_ERR("[RPCIPC]<Client> ACK Fail (%d)\n", (int)i4_cmd);
                i4_ret = IPCR_CNN_FAIL;
            }
    }

    if(i4_ret == IPCR_OK)
    {
        if(recv(i4_ct_sock, &i4_cmd, sizeof(INT32), 0) != sizeof(INT32))
        {
                RPC_ERR("[RPCIPC]<Client> ACK Fail (%d)\n", (int)i4_cmd);
            i4_ret = IPCR_CNN_FAIL;
        }
    }

    if(i4_ret == IPCR_OK)
    {
        if(connect(i4_it_sock, (const void *)&t_serv_addr, sizeof(struct sockaddr_un)) < 0)
        {
                RPC_ERR("[RPCIPC]<Client> SOCK Connection fail errno:%d\n", (int)errno);
            i4_ret = IPCR_CNN_FAIL;
        }
    }

    if(i4_ret == IPCR_OK)
    {
            msg->i4_cmd = i4_cmd;
    	    msg->self_ref = pt_client;
    	    msg->rmt_ref = pt_client->pv_remote_tag;
            int i_temp;
            if((i_temp=send(i4_it_sock, (char*)msg, sizeof(MSG), 0) )!= sizeof(MSG))
        {
                RPC_ERR("[RPCIPC]<Client> ACK Fail (%d)\n", (int)i4_cmd);
            i4_ret = IPCR_CNN_FAIL;
        }
    }
    }

    if(i4_ret == IPCR_OK)
    {
	int i_temp;
        if(( i_temp= recv(i4_it_sock, &i4_cmd, sizeof(INT32), 0) )!= sizeof(INT32))
        {
            RPC_ERR("[RPCIPC]<Client> ACK Fail (%d)\n", (int)i4_cmd);
            i4_ret = IPCR_CNN_FAIL;
        }
    }

    if(i4_ret == IPCR_OK)
    {
        /* Here there will be a ref count to sess and unrefed in ipc thread */
        if((t_id = _alloc_id(pv_tag,
                             pf_close,
                             pf_proc_msg,
                             NULL,
                             pf_proc_rpc_cb,
                             i4_it_sock,
                             i4_ct_sock,
                             i4_log_level,
                             pf_log_fct)) != (IPC_ID_T)0)
        {
			printf("[RPCIPC]<Client> alloc t_id =%d\n",(int)t_id);
            _set_cur_id(t_id);
	        add_sess_t(pt_client,t_id);
            /* Use this to auto close the session if current thread exits */
            rpcu_os_tls_set(pv_key_client_auto_close, (VOID *)t_id);

            i4_ret = _start_ipc_thd(t_id);
        }
        else
        {
            i4_ret = IPCR_OUTOFMEMORY;
            RPC_ERR("[RPCIPC]<Client> Allocate Client ID failed\n");
        }
    }
#ifdef RPC_ENABLE_MULTI_CLIENT
__OPEN_FAIL:
#endif
	if ( msg != NULL)
	{
		RPC_FREE(msg); //mtk40438 add for kloc issue
	}

    if(i4_ret == IPCR_OK)
    {
        RPC_LOG("[RPCIPC]<Client>App open IPC client successful\n");
        return t_id;
    }
    else
    {
        /* Error handling
        	  * t_id == 0 is _alloc_id fail, so dont enter _unref_sess, so close(fd) at here. else will close at _unref_sess() method.
        	*/
        if(i4_it_sock >= 0 && t_id == 0)
        {
            close(i4_it_sock);
        }
        if(i4_ct_sock >= 0 && t_id == 0)
        {
            close(i4_ct_sock);
        }
        if(t_id != 0)
        {
            RPC_ERR("[RPCIPC]<Client>open IPC client failed, L%d\n", __LINE__);
            _unref_sess(_get_sess(t_id, FALSE, FALSE));
        }
        return i4_ret;
    }
}

/*------------------------------------------------------------------------
 * Name:
 *
 * Description: This API is called by a server and opens the server's communication port.
 *
 * Inputs:
 * Outputs: -
 * Returns: -
 -----------------------------------------------------------------------*/
INT32 ipc_close_client (IPC_ID_T  t_ipc_id, BOOL bAuto)
{
    /* FIXME */
    IPC_SESS_T * pt_sess;

    printf("[RPCIPC]<Client>ipc_close_client, Auto close: %s\n", bAuto?"YES":"NO");

    pt_sess = _get_sess(t_ipc_id, TRUE, FALSE);

    if(pt_sess != NULL)
    {
				/*
				 * This will awake the IPC thread, and then
				 * the IPC thread will do some cleaning work.		  *
				 */
     #if 1 //mtk40074 add for multiclient using the same thread
		if(1 == pt_sess->ui1_multiClientc)
		{
			printf("[RPCIPC]<Client>close it shut:%d, ct shut:%d\n", pt_sess->i4_it_sock, pt_sess->i4_ct_sock);
			shutdown(pt_sess->i4_it_sock, SHUT_RDWR);
			shutdown(pt_sess->i4_ct_sock, SHUT_RDWR);
		}
		else
		{
			rpcu_os_mutex_lock(pv_tbl_lock);
			pt_sess->ui4_refc -= 1;
			rpcu_os_mutex_unlock(pv_tbl_lock);
			printf("[RPCIPC]<Client>close Ref: %u, Multiclient:%d\n",pt_sess->ui4_refc, pt_sess->ui1_multiClientc);
		}
		if (FALSE == bAuto)  //If not auto close, need wait auto close exit
		{
			while(1)
			{
				RPC_LOG("[RPCIPC]<Client>Wait... Auto close\n");
				if(pt_sess->ui4_refc == pt_sess->ui1_multiClientc)
				{
					rpcu_os_mutex_lock(pv_tbl_lock);
					pt_sess->ui1_multiClientc -= 1;
					 rpcu_os_mutex_unlock(pv_tbl_lock);
					printf("[RPCIPC]<Client>Auto close complete, client:%d\n", pt_sess->ui1_multiClientc);
					break;
				}
				usleep(10*1000);
			}
		}
		else
		{
			 RPC_LOG("[RPCIPC]<Client>Auto close true\n");
			 rpcu_os_mutex_lock(pv_tbl_lock);
			 pt_sess->ui1_multiClientc -= 1;
			 rpcu_os_mutex_unlock(pv_tbl_lock);
		}
        #else

        shutdown(pt_sess->i4_it_sock, SHUT_RDWR);
        shutdown(pt_sess->i4_ct_sock, SHUT_RDWR);
        if (FALSE == bAuto)  //If not auto close, need wait auto close exit
        {
            while(1)
            {
                printf("[RPCIPC]<Client>Wait... Auto close!\n");
                if(1 == pt_sess->ui4_refc)
                {
                    printf("[RPCIPC]<Client>Auto close complete!\n");
					break;
                }
                usleep(10*1000);
            }
        }
		#endif
        _unref_sess(pt_sess);
        return IPCR_OK;
    }
    else
    {
        return IPCR_NOT_OPENED;
    }
}

/* Push logger of RPC-ID to the current stack.
 * If the id is incorrect or some error happens,
 * an default logger will be pushed to the stack
 * for showing the errors.
 */
VOID ipc_push_logger(IPC_ID_T t_ipc_id)
{
    IPC_SESS_T * pt_sess;

    pt_sess = _get_sess(t_ipc_id, FALSE, FALSE);

    if(pt_sess != NULL)
    {
        rpcu_push_logger(pt_sess->pf_log_fct, pt_sess->i4_log_level);
    }
    else
    {
        rpcu_push_default_logger();
    }
}

/* Pop logger from current logger stack.
 * ID is reserved.
 */
VOID ipc_pop_logger(IPC_ID_T t_ipc_id)
{
    rpcu_pop_logger();
}

/*
 * Copy the current stack top logger to the ID.
 * Use this function to set new logger to the
 * id by pushing/modifying statck top log function
 * before this function.
 */

VOID ipc_update_logger(IPC_ID_T t_ipc_id)
{
    IPC_SESS_T * pt_sess;

    pt_sess = _get_sess(t_ipc_id, FALSE, FALSE);

    if(pt_sess != NULL)
    {
        rpcu_logger_stack_get(&pt_sess->pf_log_fct, &pt_sess->i4_log_level);
    }
}


/* Old log control, leave it and deprecated */
BOOL ipc_log_on()
{
    BOOL b_old = g_b_log;
    g_b_log = TRUE;
    return b_old;
}

VOID ipc_log_reset(BOOL b_old)
{
    g_b_log = b_old;
}

/*------------------------------------------------------------------------
 * Name:
 *
 * Description: Init ipc and read configuration file.Init the session table.
 *
 * Inputs:
 * Outputs: -
 * Returns: -
 -----------------------------------------------------------------------*/
INT32 ipc_init(UINT32 ui4_max_id, const OS_FNCT_T*  pt_os_fnct)
{
    INT32  i4_ret;

    FILE * pt_f;
    CHAR * ps_line = NULL;
    SIZE_T z_len;
    CHAR * pc_c;
    CHAR   ps_key[32];
    CHAR   ps_val[256];
    ssize_t read_size;

    printf("ipc_init is called, L%d, size(INT32)=%ld, sizeof(int)=%ld\n", __LINE__, sizeof(INT32), sizeof(int));

    pt_f = fopen("/proc/self/cmdline", "rt");
    if(pt_f != NULL)
    {
        if((read_size = getline(&ps_line, &z_len, pt_f)) > 0)
        {
            printf("getline,ps_line=%s, read_size=%d, z_len=%d\n", ps_line, (int)read_size, (int)z_len);
            g_ps_exe_name = RPC_MALLOC(z_len + 1);
            if(g_ps_exe_name != NULL && read_size > 0)
            {
                strncpy(g_ps_exe_name, ps_line, read_size);
            }
            else
            {
                g_ps_exe_name = "unknown";
            }
        }

		if (ps_line != NULL)
		{
			RPC_FREE(ps_line);
			printf("RPC_FREE for ps_line, L%d\n", __LINE__);
			ps_line = NULL;
		}

        fclose(pt_f);
    }

    strncpy(t_cfg.ps_sock_path, BT_TMP_PATH, strlen(BT_TMP_PATH));

    pt_f = fopen("mtk_ipc.cfg", "rt");

    if(pt_f != NULL)
    {
        printf("ipc_init open mtk_ipc.cfg success\n");
        while(getline(&ps_line, &z_len, pt_f) > 0)
        {
            for(pc_c = ps_line; (*pc_c != '\0') && (*pc_c ==' ' || *pc_c == '\t'); pc_c ++)
            {
            }
            if(*pc_c == '#')
            {
                continue;
            }
            if(sscanf(ps_line, "%31s %255s\n", ps_key, ps_val) == 2)
            {
                if(strcmp(ps_key, "sock_path") == 0)
                {
                    strncpy(t_cfg.ps_sock_path, ps_val, strlen(ps_val));
                }
                else if(strcmp(ps_key, "log") == 0)
                {
                    if(strcmp(ps_val, "on") == 0)
                    {
                        //g_b_log = TRUE;
                    }
                    else
                    {
                    }
                }
                else if(strcmp(ps_key, "monitor") == 0)
                {
                    if(strcmp(ps_val, "on") == 0)
                    {
                        _monitor_start();
                    }
                }
                RPC_LOG("Read cfg %s, V %s\n", ps_key, ps_val);
             }
        }

		if (ps_line != NULL)
		{
			free(ps_line);
			ps_line = NULL;
		}
        fclose(pt_f);
    }

    printf("ipc_init rpcu_os_init\n");
    rpcu_os_init(pt_os_fnct);
    if (NULL == pv_key_sess)
    {
    if((pv_key_sess = rpcu_os_new_tls_key(NULL)) == NULL)
    {
            printf("rpcu_os_new_tls_key error, L%d\n", __LINE__);
        return IPCR_OSERR;
    }
    }
    printf("ipc_init is called, pv_key_sess=%p, L%d\n", pv_key_sess, __LINE__);
    if (NULL == pv_key_client_auto_close)
    {
    if((pv_key_client_auto_close = rpcu_os_new_tls_key(_ipc_client_auto_close)) == NULL)
    {
        RPC_ERR("error for create\n");
        return IPCR_OSERR;
    }
    }
    printf("ipc_init, pv_key_client=%p\n", pv_key_client);
    if (NULL == pv_key_client)
    {
    if((pv_key_client = rpcu_os_new_tls_key(NULL)) == NULL)
    {
        printf("ipc_init is called, L%d\n", __LINE__);
  	 return IPCR_OSERR;
    }
    }
    if((i4_ret = _init_sess_tbl(ui4_max_id)) != IPCR_OK)
    {
        printf("_init_sess_tbl fail, L%d\n", __LINE__);
        rpcu_os_delete_tls_key(pv_key_sess);
    }
    printf("ipc_init is called, i4_ret=%d\n", i4_ret);
    return i4_ret;
}

/*------------------------------------------------------------------------
 * Name:
 *
 *Description: This API is called by a client or server context and uninitializes the IPC
 *component. This function is called from API 'bt_rpc_uninit'. Note that this function must be
 *called the same number of times 'ipc_init' has been called and will only perform the actual
 *uninitialization when this function has been called the same number of times as 'ipc_init'.
 *
 * Inputs:
 * Outputs: -
 * Returns: -
 -----------------------------------------------------------------------*/

VOID ipc_uninit()
{
    printf("ipc_uninit()\n");
	rpcu_os_uninit();

	_uninit_sess_tbl();

	if (g_ps_exe_name != NULL)
	{
		RPC_FREE(g_ps_exe_name);
		g_ps_exe_name = NULL;
	}

	if(pv_key_sess != NULL)
	{
		rpcu_os_delete_tls_key(pv_key_sess);
		pv_key_sess = NULL;
	}
	if(pv_key_client_auto_close != NULL)
	{
		rpcu_os_delete_tls_key(pv_key_client_auto_close);
		pv_key_client_auto_close = NULL;
	}
	if(pv_key_client != NULL)
	{
		rpcu_os_delete_tls_key(pv_key_client);
		pv_key_client = NULL;
	}
}

