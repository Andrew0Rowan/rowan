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


/*-----------------------------------------------------------------------------
                        include files
-----------------------------------------------------------------------------*/
#include <stdio.h>

#include "mtk_bt_service_gattc.h"
#include "mtk_bt_service_gattc_handle.h"
#include "mtk_bt_service_gatt_ipcrpc_struct.h"
#include "ri_common.h"

#define BT_RH_LOG(_stmt...)

static VOID bt_app_gattc_event_cbk_wrapper(BT_GATTC_EVENT_T bt_gatt_event,
                                                       void *pv_tag)
{
    RPC_DECL_VOID(2);
    RPC_CB_NFY_TAG_T  *pt_nfy_tag = (RPC_CB_NFY_TAG_T*)pv_tag;
    RPC_ARG_INP(ARG_TYPE_INT32, bt_gatt_event);
    RPC_ARG_INP(ARG_TYPE_REF_VOID, pt_nfy_tag->pv_tag);
    RPC_DO_CB(pt_nfy_tag->t_id, "bt_app_gattc_event_cbk", pt_nfy_tag->apv_cb_addr_ex[0]);
    RPC_RETURN_VOID;
}

static VOID bt_app_gattc_reg_client_cbk_wrapper(BT_GATTC_REG_CLIENT_T *pt_reg_client_result,
                                                            void *pv_tag)
{
    RPC_DECL_VOID(2);
    RPC_CB_NFY_TAG_T *pt_nfy_tag = (RPC_CB_NFY_TAG_T*)pv_tag;
    RPC_CHECK(bt_rpc_add_ref_desc(RPC_DEFAULT_ID, pt_reg_client_result, RPC_DESC_BT_GATTC_REG_CLIENT_T, NULL));
    RPC_CHECK(bt_rpc_add_ref_buff(RPC_DEFAULT_ID, pt_reg_client_result->uuid, BT_GATT_MAX_UUID_LEN));
    RPC_ARG_IO(ARG_TYPE_REF_DESC, pt_reg_client_result);
    RPC_ARG_INP(ARG_TYPE_REF_VOID, pt_nfy_tag->pv_tag);
    BT_RH_LOG("[_hndlr_]bt_app_gattc_reg_client_cbk_wrapper, client_if = %ld, pt_nfy_tag->apv_cb_addr_ex[1] = %p\n",
        (long)pt_reg_client_result->client_if, pt_nfy_tag->apv_cb_addr_ex[1]);
    RPC_DO_CB(pt_nfy_tag->t_id, "bt_app_gattc_reg_client_cbk", pt_nfy_tag->apv_cb_addr_ex[1]);
    RPC_RETURN_VOID;
}

static VOID bt_app_gattc_scan_result_cbk_wrapper(BT_GATTC_SCAN_RST_T *pt_scan_result ,void* pv_tag)
{
    RPC_DECL_VOID(2);
    RPC_CB_NFY_TAG_T *pt_nfy_tag = (RPC_CB_NFY_TAG_T*)pv_tag;
    RPC_CHECK(bt_rpc_add_ref_desc(RPC_DEFAULT_ID, pt_scan_result, RPC_DESC_BT_GATTC_SCAN_RST_T, NULL));
    RPC_CHECK(bt_rpc_add_ref_buff(RPC_DEFAULT_ID, pt_scan_result->btaddr, MAX_BDADDR_LEN));
    RPC_CHECK(bt_rpc_add_ref_buff(RPC_DEFAULT_ID, pt_scan_result->adv_data, BT_GATT_MAX_ATTR_LEN));
    RPC_ARG_IO(ARG_TYPE_REF_DESC, pt_scan_result);
    RPC_ARG_INP(ARG_TYPE_REF_VOID, pt_nfy_tag->pv_tag);

    BT_RH_LOG("[_hndlr_]bt_app_gattc_scan_result_cbk_wrapper, btaddr = %s, pt_nfy_tag->apv_cb_addr_ex[2] = %p\n",
        pt_scan_result->btaddr, pt_nfy_tag->apv_cb_addr_ex[2]);

    RPC_DO_CB(pt_nfy_tag->t_id, "bt_app_gattc_scan_result_cbk", pt_nfy_tag->apv_cb_addr_ex[2]);

    RPC_RETURN_VOID;
}

static VOID bt_app_gattc_get_gatt_db_cbk_wrapper(BT_GATTC_GET_GATT_DB_T *pt_get_gatt_db_result ,
                                                               void* pv_tag)
{

    RPC_CB_NFY_TAG_T *pt_nfy_tag = (RPC_CB_NFY_TAG_T*)pv_tag;
    BT_GATTC_DB_ELEMENT_T *gatt_db = NULL;
    if (0 != pt_get_gatt_db_result->count)
    {
        gatt_db = malloc(pt_get_gatt_db_result->count * sizeof(BT_GATTC_DB_ELEMENT_T));
        if (NULL == gatt_db)
        {
            BT_RH_LOG("<Server>gatt_db malloc failed\n");
            return;
        }
        memcpy(gatt_db, pt_get_gatt_db_result->gatt_db_element, pt_get_gatt_db_result->count * sizeof(BT_GATTC_DB_ELEMENT_T));
    }

    RPC_DECL_VOID(3);
    RPC_CHECK(bt_rpc_add_ref_desc(RPC_DEFAULT_ID, pt_get_gatt_db_result,
                                  RPC_DESC_BT_GATTC_GET_GATT_DB_T, NULL));
    RPC_CHECK(bt_rpc_add_ref_desc_arr(RPC_DEFAULT_ID, pt_get_gatt_db_result->count,
                                      gatt_db, RPC_DESC_BT_GATTC_DB_ELEMENT_T, NULL));
    RPC_ARG_IO(ARG_TYPE_REF_DESC, pt_get_gatt_db_result);
    RPC_ARG_INP(ARG_TYPE_REF_VOID, pt_nfy_tag->pv_tag);
    RPC_ARG_IO(ARG_TYPE_REF_DESC, gatt_db);
    BT_RH_LOG("[_hndlr_]bt_app_gattc_get_gatt_db_cbk_wrapper, count = %ld, pt_nfy_tag->apv_cb_addr_ex[3] = %p\n",
        pt_get_gatt_db_result->count, pt_nfy_tag->apv_cb_addr_ex[3]);
    RPC_DO_CB(pt_nfy_tag->t_id, "bt_app_gattc_get_gatt_db_cbk", pt_nfy_tag->apv_cb_addr_ex[3]);
    if (NULL != gatt_db)
    {
        free(gatt_db);
    }
    RPC_RETURN_VOID;
}

static VOID bt_app_gattc_get_reg_noti_cbk_wrapper(BT_GATTC_GET_REG_NOTI_RST_T *pt_get_reg_noti_result,
                                                                void* pv_tag)
{
    RPC_DECL_VOID(2);
    RPC_CB_NFY_TAG_T *pt_nfy_tag = (RPC_CB_NFY_TAG_T*)pv_tag;
    RPC_CHECK(bt_rpc_add_ref_desc(RPC_DEFAULT_ID, pt_get_reg_noti_result,
                                  RPC_DESC_BT_GATTC_GET_REG_NOTI_RST_T, NULL));
    RPC_ARG_IO(ARG_TYPE_REF_DESC, pt_get_reg_noti_result);
    RPC_ARG_INP(ARG_TYPE_REF_VOID, pt_nfy_tag->pv_tag);
    BT_RH_LOG("[_hndlr_]bt_app_gattc_get_reg_noti_cbk_wrapper, registered = %ld, pt_nfy_tag->apv_cb_addr_ex[4] = %p\n",
              pt_get_reg_noti_result->registered, pt_nfy_tag->apv_cb_addr_ex[4]);
    RPC_DO_CB(pt_nfy_tag->t_id, "bt_app_gattc_get_reg_noti_cbk", pt_nfy_tag->apv_cb_addr_ex[4]);
    RPC_RETURN_VOID;
}

static VOID bt_app_gattc_notify_cbk_wrapper(BT_GATTC_GET_NOTIFY_T *pt_notify,
                                                       void* pv_tag)
{
    RPC_DECL_VOID(2);
    RPC_CB_NFY_TAG_T *pt_nfy_tag = (RPC_CB_NFY_TAG_T*)pv_tag;
    RPC_CHECK(bt_rpc_add_ref_desc(RPC_DEFAULT_ID, pt_notify, RPC_DESC_BT_GATTC_GET_NOTIFY_T, NULL));
    RPC_ARG_IO(ARG_TYPE_REF_DESC, pt_notify);
    RPC_ARG_INP(ARG_TYPE_REF_VOID, pt_nfy_tag->pv_tag);
    BT_RH_LOG("[_hndlr_]bt_app_gattc_notify_cbk_wrapper, handle = %ld, pt_nfy_tag->apv_cb_addr_ex[5] = %p\n",
        (long)pt_notify->notify_data.handle, pt_nfy_tag->apv_cb_addr_ex[5]);
    RPC_DO_CB(pt_nfy_tag->t_id, "bt_app_gattc_notify_cbk", pt_nfy_tag->apv_cb_addr_ex[5]);
    RPC_RETURN_VOID;
}

static VOID bt_app_gattc_read_char_cbk_wrapper(BT_GATTC_READ_CHAR_RST_T *pt_read_char,
                                                            void* pv_tag)
{
    RPC_DECL_VOID(2);
    RPC_CB_NFY_TAG_T *pt_nfy_tag = (RPC_CB_NFY_TAG_T*)pv_tag;
    RPC_CHECK(bt_rpc_add_ref_desc(RPC_DEFAULT_ID, pt_read_char, RPC_DESC_BT_GATTC_READ_CHAR_RST_T, NULL));
    RPC_ARG_IO(ARG_TYPE_REF_DESC, pt_read_char);
    RPC_ARG_INP(ARG_TYPE_REF_VOID, pt_nfy_tag->pv_tag);
    BT_RH_LOG("[_hndlr_]bt_app_gattc_read_char_cbk_wrapper, handle = %ld, pt_nfy_tag->apv_cb_addr_ex[6] = %p\n",
        (long)pt_read_char->read_data.handle, pt_nfy_tag->apv_cb_addr_ex[6]);
    RPC_DO_CB(pt_nfy_tag->t_id, "bt_app_gattc_read_char_cbk", pt_nfy_tag->apv_cb_addr_ex[6]);
    RPC_RETURN_VOID;
}

static VOID bt_app_gattc_write_char_cbk_wrapper(BT_GATTC_WRITE_CHAR_RST_T *pt_write_char,
                                                             void* pv_tag)
{
    RPC_DECL_VOID(2);
    RPC_CB_NFY_TAG_T *pt_nfy_tag = (RPC_CB_NFY_TAG_T*)pv_tag;
    RPC_CHECK(bt_rpc_add_ref_desc(RPC_DEFAULT_ID, pt_write_char, RPC_DESC_BT_GATTC_WRITE_CHAR_RST_T, NULL));
    RPC_ARG_IO(ARG_TYPE_REF_DESC, pt_write_char);
    RPC_ARG_INP(ARG_TYPE_REF_VOID, pt_nfy_tag->pv_tag);
    BT_RH_LOG("[_hndlr_]bt_app_gattc_write_char_cbk_wrapper, handle = %ld, pt_nfy_tag->apv_cb_addr_ex[7] = %p\n",
        (long)pt_write_char->handle, pt_nfy_tag->apv_cb_addr_ex[7]);
    RPC_DO_CB(pt_nfy_tag->t_id, "bt_app_gattc_write_char_cbk", pt_nfy_tag->apv_cb_addr_ex[7]);
    RPC_RETURN_VOID;
}

static VOID bt_app_gattc_read_desc_cbk_wrapper(BT_GATTC_READ_DESCR_RST_T *pt_read_desc,
                                                             void* pv_tag)
{
    RPC_DECL_VOID(2);
    RPC_CB_NFY_TAG_T *pt_nfy_tag = (RPC_CB_NFY_TAG_T*)pv_tag;
    RPC_CHECK(bt_rpc_add_ref_desc(RPC_DEFAULT_ID, pt_read_desc, RPC_DESC_BT_GATTC_READ_DESCR_RST_T, NULL));
    RPC_ARG_IO(ARG_TYPE_REF_DESC, pt_read_desc);
    RPC_ARG_INP(ARG_TYPE_REF_VOID, pt_nfy_tag->pv_tag);
    BT_RH_LOG("[_hndlr_]bt_app_gattc_read_desc_cbk_wrapper, handle = %ld, pt_nfy_tag->apv_cb_addr_ex[8] = %p\n",
        (long)pt_read_desc->read_data.handle, pt_nfy_tag->apv_cb_addr_ex[8]);
    RPC_DO_CB(pt_nfy_tag->t_id, "bt_app_gattc_read_desc_cbk", pt_nfy_tag->apv_cb_addr_ex[8]);
    RPC_RETURN_VOID;
}

static VOID bt_app_gattc_write_desc_cbk_wrapper(BT_GATTC_WRITE_DESCR_RST_T *pt_write_desc,
                                                             void* pv_tag)
{
    RPC_DECL_VOID(2);
    RPC_CB_NFY_TAG_T *pt_nfy_tag = (RPC_CB_NFY_TAG_T*)pv_tag;
    RPC_CHECK(bt_rpc_add_ref_desc(RPC_DEFAULT_ID, pt_write_desc, RPC_DESC_BT_GATTC_WRITE_DESCR_RST_T, NULL));
    RPC_ARG_IO(ARG_TYPE_REF_DESC, pt_write_desc);
    RPC_ARG_INP(ARG_TYPE_REF_VOID, pt_nfy_tag->pv_tag);
    BT_RH_LOG("[_hndlr_]bt_app_gattc_write_desc_cbk_wrapper, handle = %ld, pt_nfy_tag->apv_cb_addr_ex[9] = %p\n",
        (long)pt_write_desc->handle, pt_nfy_tag->apv_cb_addr_ex[9]);
    RPC_DO_CB(pt_nfy_tag->t_id, "bt_app_gattc_write_desc_cbk", pt_nfy_tag->apv_cb_addr_ex[9]);
    RPC_RETURN_VOID;
}

static VOID
bt_app_gattc_scan_filter_param_cbk_wrapper(BT_GATTC_SCAN_FILTER_PARAM_T *pt_scan_filter_param,
                                                          void* pv_tag)
{
    RPC_DECL_VOID(2);
    RPC_CB_NFY_TAG_T *pt_nfy_tag = (RPC_CB_NFY_TAG_T*)pv_tag;
    RPC_CHECK(bt_rpc_add_ref_desc(RPC_DEFAULT_ID, pt_scan_filter_param,
                                  RPC_DESC_BT_GATTC_SCAN_FILTER_PARAM_T, NULL));
    RPC_ARG_IO(ARG_TYPE_REF_DESC, pt_scan_filter_param);
    RPC_ARG_INP(ARG_TYPE_REF_VOID, pt_nfy_tag->pv_tag);
    BT_RH_LOG("[_hndlr_]bt_app_gattc_scan_filter_param_cbk_wrapper, action = %ld, pt_nfy_tag->apv_cb_addr_ex[10] = %p\n",
        (long)pt_scan_filter_param->action, pt_nfy_tag->apv_cb_addr_ex[10]);
    RPC_DO_CB(pt_nfy_tag->t_id, "bt_app_gattc_scan_filter_param_cbk", pt_nfy_tag->apv_cb_addr_ex[10]);
    RPC_RETURN_VOID;
}

static VOID bt_app_gattc_scan_filter_status_cbk_wrapper(BT_GATTC_SCAN_FILTER_STATUS_T *pt_scan_filter_status,
                                                                      void* pv_tag)
{
    RPC_DECL_VOID(2);
    RPC_CB_NFY_TAG_T *pt_nfy_tag = (RPC_CB_NFY_TAG_T*)pv_tag;
    RPC_CHECK(bt_rpc_add_ref_desc(RPC_DEFAULT_ID, pt_scan_filter_status,
                                  RPC_DESC_BT_GATTC_SCAN_FILTER_STATUS_T, NULL));
    RPC_ARG_IO(ARG_TYPE_REF_DESC, pt_scan_filter_status);
    RPC_ARG_INP(ARG_TYPE_REF_VOID, pt_nfy_tag->pv_tag);
    BT_RH_LOG("[_hndlr_]bt_app_gattc_scan_filter_status_cbk_wrapper, enable = %ld, pt_nfy_tag->apv_cb_addr_ex[11] = %p\n",
        (long)pt_scan_filter_status->enable, pt_nfy_tag->apv_cb_addr_ex[11]);
    RPC_DO_CB(pt_nfy_tag->t_id, "bt_app_gattc_scan_filter_status_cbk", pt_nfy_tag->apv_cb_addr_ex[11]);
    RPC_RETURN_VOID;
}

static VOID bt_app_gattc_scan_filter_cfg_cbk_wrapper(BT_GATTC_SCAN_FILTER_CFG_T *pt_scan_filter_cfg,
                                                                  void* pv_tag)
{
    RPC_DECL_VOID(2);
    RPC_CB_NFY_TAG_T *pt_nfy_tag = (RPC_CB_NFY_TAG_T*)pv_tag;
    RPC_CHECK(bt_rpc_add_ref_desc(RPC_DEFAULT_ID, pt_scan_filter_cfg,
                                  RPC_DESC_BT_GATTC_SCAN_FILTER_CFG_T, NULL));
    RPC_ARG_IO(ARG_TYPE_REF_DESC, pt_scan_filter_cfg);
    RPC_ARG_INP(ARG_TYPE_REF_VOID, pt_nfy_tag->pv_tag);
    BT_RH_LOG("[_hndlr_]bt_app_gattc_scan_filter_cfg_cbk_wrapper, action = %ld, pt_nfy_tag->apv_cb_addr_ex[12] = %p\n",
        (long)pt_scan_filter_cfg->action, pt_nfy_tag->apv_cb_addr_ex[12]);
    RPC_DO_CB(pt_nfy_tag->t_id, "bt_app_gattc_scan_filter_cfg_cbk", pt_nfy_tag->apv_cb_addr_ex[12]);
    RPC_RETURN_VOID;
}

static VOID bt_app_gattc_adv_enable_cbk_wrapper(int status, void* pv_tag)
{
    RPC_DECL_VOID(2);
    RPC_CB_NFY_TAG_T *pt_nfy_tag = (RPC_CB_NFY_TAG_T*)pv_tag;
    RPC_ARG_INP(ARG_TYPE_INT8, status);
    RPC_ARG_INP(ARG_TYPE_REF_VOID, pt_nfy_tag->pv_tag);
    BT_RH_LOG("[_hndlr_]bt_app_gattc_adv_enable_cbk_wrapper, status = %d, pt_nfy_tag->apv_cb_addr_ex[13] = %p\n",
        status, pt_nfy_tag->apv_cb_addr_ex[13]);
    RPC_DO_CB(pt_nfy_tag->t_id, "bt_app_gattc_adv_enable_cbk", pt_nfy_tag->apv_cb_addr_ex[13]);
    RPC_RETURN_VOID;
}

static INT32 _hndlr_x_mtkapi_bt_gattc_register_callback(RPC_ID_T     t_rpc_id,
                                                                     const CHAR*  ps_cb_type,
                                                                     UINT32       ui4_num_args,
                                                                     ARG_DESC_T*  pt_args,
                                                                     ARG_DESC_T*  pt_return)
{
    MTKRPCAPI_BT_APP_GATTC_CB_FUNC_T *p_bt_app_cb_func = NULL;
    MTKRPCAPI_BT_APP_GATTC_CB_FUNC_T bt_app_cb_func;
    RPC_CB_NFY_TAG_T * pt_nfy_tag = NULL;
    VOID * apv_cb_addr[14] = {0};
    memset(&bt_app_cb_func,0,sizeof(MTKRPCAPI_BT_APP_GATTC_CB_FUNC_T));
    BT_RH_LOG("gattc_base_init, pt_args[0].u.pv_desc = %p\n", pt_args[0].u.pv_desc);

    if(ui4_num_args != 2)
    {
        return RPCR_INV_ARGS;
    }
    p_bt_app_cb_func = (MTKRPCAPI_BT_APP_GATTC_CB_FUNC_T*)pt_args[0].u.pv_desc;

    if(p_bt_app_cb_func->bt_gattc_event_cb != NULL)
    {
        apv_cb_addr[0] = p_bt_app_cb_func->bt_gattc_event_cb;
        bt_app_cb_func.bt_gattc_event_cb = bt_app_gattc_event_cbk_wrapper;
    }
    if(p_bt_app_cb_func->bt_gattc_reg_client_cb != NULL)
    {
        apv_cb_addr[1] = p_bt_app_cb_func->bt_gattc_reg_client_cb;
        bt_app_cb_func.bt_gattc_reg_client_cb = bt_app_gattc_reg_client_cbk_wrapper;
    }
    if(p_bt_app_cb_func->bt_gattc_scan_cb != NULL)
    {
        apv_cb_addr[2] = p_bt_app_cb_func->bt_gattc_scan_cb;
        bt_app_cb_func.bt_gattc_scan_cb = bt_app_gattc_scan_result_cbk_wrapper;
    }
    if (p_bt_app_cb_func->bt_gattc_get_gatt_db_cb != NULL)
    {
        apv_cb_addr[3] = p_bt_app_cb_func->bt_gattc_get_gatt_db_cb;
        bt_app_cb_func.bt_gattc_get_gatt_db_cb = bt_app_gattc_get_gatt_db_cbk_wrapper;
    }
    if (p_bt_app_cb_func->bt_gattc_get_reg_noti_cb != NULL)
    {
        apv_cb_addr[4] = p_bt_app_cb_func->bt_gattc_get_reg_noti_cb;
        bt_app_cb_func.bt_gattc_get_reg_noti_cb = bt_app_gattc_get_reg_noti_cbk_wrapper;
    }
    if (p_bt_app_cb_func->bt_gattc_notify_cb != NULL)
    {
        apv_cb_addr[5] = p_bt_app_cb_func->bt_gattc_notify_cb;
        bt_app_cb_func.bt_gattc_notify_cb = bt_app_gattc_notify_cbk_wrapper;
    }
    if (p_bt_app_cb_func->bt_gattc_read_char_cb != NULL)
    {
        apv_cb_addr[6] = p_bt_app_cb_func->bt_gattc_read_char_cb;
        bt_app_cb_func.bt_gattc_read_char_cb = bt_app_gattc_read_char_cbk_wrapper;
    }
    if (p_bt_app_cb_func->bt_gattc_write_char_cb != NULL)
    {
        apv_cb_addr[7] = p_bt_app_cb_func->bt_gattc_write_char_cb;
        bt_app_cb_func.bt_gattc_write_char_cb = bt_app_gattc_write_char_cbk_wrapper;
    }
    if (p_bt_app_cb_func->bt_gattc_read_desc_cb != NULL)
    {
        apv_cb_addr[8] = p_bt_app_cb_func->bt_gattc_read_desc_cb;
        bt_app_cb_func.bt_gattc_read_desc_cb = bt_app_gattc_read_desc_cbk_wrapper;
    }
    if (p_bt_app_cb_func->bt_gattc_write_desc_cb != NULL)
    {
        apv_cb_addr[9] = p_bt_app_cb_func->bt_gattc_write_desc_cb;
        bt_app_cb_func.bt_gattc_write_desc_cb = bt_app_gattc_write_desc_cbk_wrapper;
    }
    if (p_bt_app_cb_func->bt_gattc_scan_filter_param_cb != NULL)
    {
        apv_cb_addr[10] = p_bt_app_cb_func->bt_gattc_scan_filter_param_cb;
        bt_app_cb_func.bt_gattc_scan_filter_param_cb = bt_app_gattc_scan_filter_param_cbk_wrapper;
    }
    if (p_bt_app_cb_func->bt_gattc_scan_filter_status_cb != NULL)
    {
        apv_cb_addr[11] = p_bt_app_cb_func->bt_gattc_scan_filter_status_cb;
        bt_app_cb_func.bt_gattc_scan_filter_status_cb = bt_app_gattc_scan_filter_status_cbk_wrapper;
    }
    if (p_bt_app_cb_func->bt_gattc_scan_filter_cfg_cb != NULL)
    {
        apv_cb_addr[12] = p_bt_app_cb_func->bt_gattc_scan_filter_cfg_cb;
        bt_app_cb_func.bt_gattc_scan_filter_cfg_cb = bt_app_gattc_scan_filter_cfg_cbk_wrapper;
    }
    if (p_bt_app_cb_func->bt_gattc_adv_enable_cb != NULL)
    {
        apv_cb_addr[13] = p_bt_app_cb_func->bt_gattc_adv_enable_cb;
        bt_app_cb_func.bt_gattc_adv_enable_cb = bt_app_gattc_adv_enable_cbk_wrapper;
    }
    pt_nfy_tag  = ri_create_cb_tag(t_rpc_id, apv_cb_addr, 14, pt_args[1].u.pv_arg);

    pt_return->e_type   = ARG_TYPE_INT32;
    pt_return->u.i4_arg = x_mtkapi_bt_gattc_register_callback(&bt_app_cb_func, pt_nfy_tag);
    if (pt_return->u.i4_arg && pt_nfy_tag != NULL)
    {
        ri_free_cb_tag(pt_nfy_tag);
    }
    return RPCR_OK;
}

static INT32 _hndlr_x_mtkapi_bt_gattc_register_app(RPC_ID_T     t_rpc_id,
                                                               const CHAR*  ps_cb_type,
                                                               UINT32       ui4_num_args,
                                                               ARG_DESC_T*  pt_args,
                                                               ARG_DESC_T*  pt_return)
{
    BT_RH_LOG("[_hndlr_]bt_gattc_register_app, arg_1 = %s\n", pt_args[0].u.pc_arg);
    if (ui4_num_args != 1)
    {
        BT_RH_LOG("Invalid ARGS: %d\n", ui4_num_args);
        return RPCR_INV_ARGS;
    }
    pt_return->e_type   = ARG_TYPE_INT32;
    pt_return->u.i4_arg = x_mtkapi_bt_gattc_register_app(pt_args[0].u.pc_arg);
    return RPCR_OK;
}

static INT32 _hndlr_x_mtkapi_bt_gattc_unregister_app(RPC_ID_T     t_rpc_id,
                                                                  const CHAR*  ps_cb_type,
                                                                  UINT32       ui4_num_args,
                                                                  ARG_DESC_T*  pt_args,
                                                                  ARG_DESC_T*  pt_return)
{
    BT_RH_LOG("[_hndlr_]bt_gattc_unregister_app, arg_1 = %ld\n", pt_args[0].u.i4_arg);
    if (ui4_num_args != 1)
    {
        BT_RH_LOG("Invalid ARGS: %d\n", ui4_num_args);
        return RPCR_INV_ARGS;
    }
    pt_return->e_type   = ARG_TYPE_INT32;
    pt_return->u.i4_arg = x_mtkapi_bt_gattc_unregister_app(pt_args[0].u.i4_arg);
    return RPCR_OK;
}

static INT32 _hndlr_x_mtkapi_bt_gattc_scan(RPC_ID_T     t_rpc_id,
                                                     const CHAR*  ps_cb_type,
                                                     UINT32       ui4_num_args,
                                                     ARG_DESC_T*  pt_args,
                                                     ARG_DESC_T*  pt_return)
{
    BT_RH_LOG("[_hndlr_]bt_gattc_scan, arg_1 = %ld\n", pt_args[0].u.i4_arg);
    if (ui4_num_args != 1)
    {
        BT_RH_LOG("Invalid ARGS: %d\n", ui4_num_args);
        return RPCR_INV_ARGS;
    }
    pt_return->e_type   = ARG_TYPE_INT32;
    pt_return->u.i4_arg = x_mtkapi_bt_gattc_scan(pt_args[0].u.b_arg);
    return RPCR_OK;
}
static INT32 _hndlr_x_mtkapi_bt_gattc_unregister_callback(
                         RPC_ID_T     t_rpc_id,
                         const CHAR*  ps_cb_type,
                         UINT32       ui4_num_args,
                         ARG_DESC_T*  pt_args,
                         ARG_DESC_T*  pt_return)
{
    BT_RH_LOG("[_hndlr_]bt_gattc_unregister_callback, arg_1 = %ld\n", pt_args[0].u.i4_arg);

    if (ui4_num_args != 1)
    {
        BT_RH_LOG("Invalid ARGS: %ld\n", ui4_num_args);
        return RPCR_INV_ARGS;
    }

    pt_return->e_type   = ARG_TYPE_INT32;
    pt_return->u.i4_arg = x_mtkapi_bt_gattc_unregister_callback();

    return RPCR_OK;
}

static INT32 _hndlr_x_mtkapi_bt_gattc_connect(RPC_ID_T     t_rpc_id,
                                                         const CHAR*  ps_cb_type,
                                                         UINT32       ui4_num_args,
                                                         ARG_DESC_T*  pt_args,
                                                         ARG_DESC_T*  pt_return)
{
    BT_RH_LOG("[_hndlr_]bt_gattc_open, arg_1 = %ld, arg_2 = %s, arg_3 = %d, arg_4 = %ld\n",
        pt_args[0].u.i4_arg, pt_args[1].u.pc_arg, pt_args[2].u.ui1_arg, pt_args[3].u.i4_arg);
    if (ui4_num_args != 4)
    {
        BT_RH_LOG("Invalid ARGS: %d\n", ui4_num_args);
        return RPCR_INV_ARGS;
    }
    pt_return->e_type   = ARG_TYPE_INT32;
    pt_return->u.i4_arg = x_mtkapi_bt_gattc_connect(pt_args[0].u.i4_arg,
                                                    pt_args[1].u.pc_arg,
                                                    pt_args[2].u.ui1_arg,
                                                    pt_args[3].u.i4_arg);
    return RPCR_OK;
}

static INT32 _hndlr_x_mtkapi_bt_gattc_disconnect(RPC_ID_T     t_rpc_id,
                                                             const CHAR*  ps_cb_type,
                                                             UINT32       ui4_num_args,
                                                             ARG_DESC_T*  pt_args,
                                                             ARG_DESC_T*  pt_return)
{
    BT_RH_LOG("[_hndlr_]bt_gattc_close, arg_1 = %ld, arg_2 = %s, arg_3 = %ld\n",
        pt_args[0].u.i4_arg, pt_args[1].u.pc_arg, pt_args[2].u.i4_arg);
    if (ui4_num_args != 3)
    {
        BT_RH_LOG("Invalid ARGS: %d\n", ui4_num_args);
        return RPCR_INV_ARGS;
    }
    pt_return->e_type   = ARG_TYPE_INT32;
    pt_return->u.i4_arg = x_mtkapi_bt_gattc_disconnect(pt_args[0].u.i4_arg,
                                                       pt_args[1].u.pc_arg,
                                                       pt_args[2].u.i4_arg);
    return RPCR_OK;
}

static INT32 _hndlr_x_mtkapi_bt_gattc_listen(RPC_ID_T     t_rpc_id,
                                                      const CHAR*  ps_cb_type,
                                                      UINT32       ui4_num_args,
                                                      ARG_DESC_T*  pt_args,
                                                      ARG_DESC_T*  pt_return)
{
    BT_RH_LOG("[_hndlr_]bt_gattc_listen, arg_1 = %ld\n", pt_args[0].u.i4_arg);
    if (ui4_num_args != 1)
    {
        BT_RH_LOG("Invalid ARGS: %d\n", ui4_num_args);
        return RPCR_INV_ARGS;
    }
    pt_return->e_type   = ARG_TYPE_INT32;
    pt_return->u.i4_arg = x_mtkapi_bt_gattc_listen(pt_args[0].u.i4_arg);
    return RPCR_OK;
}

static INT32 _hndlr_x_mtkapi_bt_gattc_refresh(RPC_ID_T     t_rpc_id,
                                                        const CHAR*  ps_cb_type,
                                                        UINT32       ui4_num_args,
                                                        ARG_DESC_T*  pt_args,
                                                        ARG_DESC_T*  pt_return)
{
    BT_RH_LOG("[_hndlr_]bt_gattc_refresh, arg_1 = %ld, arg_2 = %s\n",
              pt_args[0].u.i4_arg, pt_args[1].u.pc_arg);
    if (ui4_num_args != 2)
    {
        BT_RH_LOG("Invalid ARGS: %d\n", ui4_num_args);
        return RPCR_INV_ARGS;
    }
    pt_return->e_type   = ARG_TYPE_INT32;
    pt_return->u.i4_arg = x_mtkapi_bt_gattc_refresh(pt_args[0].u.i4_arg,
                                                    pt_args[1].u.pc_arg);
    return RPCR_OK;
}

static INT32 _hndlr_x_mtkapi_bt_gattc_search_service(RPC_ID_T     t_rpc_id,
                                                                   const CHAR*  ps_cb_type,
                                                                   UINT32       ui4_num_args,
                                                                   ARG_DESC_T*  pt_args,
                                                                   ARG_DESC_T*  pt_return)
{
    BT_RH_LOG("[_hndlr_]bt_gattc_search_service, arg_1 = %ld, arg_2 = %s\n",
              pt_args[0].u.i4_arg, pt_args[1].u.pc_arg);
    if (ui4_num_args != 2)
    {
        BT_RH_LOG("Invalid ARGS: %d\n", ui4_num_args);
        return RPCR_INV_ARGS;
    }
    pt_return->e_type   = ARG_TYPE_INT32;
    pt_return->u.i4_arg = x_mtkapi_bt_gattc_search_service(pt_args[0].u.i4_arg,
                                                           pt_args[1].u.pc_arg);

    return RPCR_OK;
}

static INT32 _hndlr_x_mtkapi_bt_gattc_get_gatt_db(RPC_ID_T     t_rpc_id,
                                                              const CHAR*  ps_cb_type,
                                                              UINT32       ui4_num_args,
                                                              ARG_DESC_T*  pt_args,
                                                              ARG_DESC_T*  pt_return)
{
    BT_RH_LOG("[_hndlr_]bt_gattc_get_gatt_db, arg_1 = %ld\n", pt_args[0].u.i4_arg);
    if (ui4_num_args != 1)
    {
        BT_RH_LOG("Invalid ARGS: %d\n", ui4_num_args);
        return RPCR_INV_ARGS;
    }
    pt_return->e_type   = ARG_TYPE_INT32;
    pt_return->u.i4_arg = x_mtkapi_bt_gattc_get_gatt_db(pt_args[0].u.i4_arg);
    return RPCR_OK;
}

static INT32 _hndlr_x_mtkapi_bt_gattc_read_char(RPC_ID_T     t_rpc_id,
                                                            const CHAR*  ps_cb_type,
                                                            UINT32       ui4_num_args,
                                                            ARG_DESC_T*  pt_args,
                                                            ARG_DESC_T*  pt_return)
{
    BT_RH_LOG("[_hndlr_]bt_gattc_read_char, arg_1 = %ld, arg_2 = %ld, arg_3 = %ld\n",
              pt_args[0].u.i4_arg, pt_args[1].u.i4_arg, pt_args[2].u.i4_arg);
    if (ui4_num_args != 3)
    {
        BT_RH_LOG("Invalid ARGS: %d\n", ui4_num_args);
        return RPCR_INV_ARGS;
    }
    pt_return->e_type   = ARG_TYPE_INT32;
    pt_return->u.i4_arg = x_mtkapi_bt_gattc_read_char(pt_args[0].u.i4_arg,
                                                      pt_args[1].u.i4_arg,
                                                      pt_args[2].u.i4_arg);
    return RPCR_OK;
}

static INT32 _hndlr_x_mtkapi_bt_gattc_read_descr(RPC_ID_T     t_rpc_id,
                                                             const CHAR*  ps_cb_type,
                                                             UINT32       ui4_num_args,
                                                             ARG_DESC_T*  pt_args,
                                                             ARG_DESC_T*  pt_return)
{
    BT_RH_LOG("[_hndlr_]bt_gattc_read_descr, arg_1 = %ld, arg_2 = %ld, arg_3 = %ld\n",
              pt_args[0].u.i4_arg, pt_args[1].u.i4_arg, pt_args[2].u.i4_arg);
    if (ui4_num_args != 3)
    {
        BT_RH_LOG("Invalid ARGS: %d\n", ui4_num_args);
        return RPCR_INV_ARGS;
    }
    pt_return->e_type   = ARG_TYPE_INT32;
    pt_return->u.i4_arg = x_mtkapi_bt_gattc_read_descr(pt_args[0].u.i4_arg,
                                                       pt_args[1].u.i4_arg,
                                                       pt_args[2].u.i4_arg);
    return RPCR_OK;
}

static INT32 _hndlr_x_mtkapi_bt_gattc_write_char(RPC_ID_T     t_rpc_id,
                                                             const CHAR*  ps_cb_type,
                                                             UINT32       ui4_num_args,
                                                             ARG_DESC_T*  pt_args,
                                                             ARG_DESC_T*  pt_return)
{
    BT_RH_LOG("[_hndlr_]bt_gattc_write_char, arg_1 = %ld, arg_2 = %ld, arg_3 = %ld, arg_4 = %ld, arg_5 = %ld, arg_6 = %s\n",
              pt_args[0].u.i4_arg, pt_args[1].u.i4_arg, pt_args[2].u.i4_arg,
              pt_args[3].u.i4_arg, pt_args[4].u.i4_arg, pt_args[5].u.pc_arg);
    if (ui4_num_args != 6)
    {
        BT_RH_LOG("Invalid ARGS: %d\n", ui4_num_args);
        return RPCR_INV_ARGS;
    }
    pt_return->e_type   = ARG_TYPE_INT32;
    pt_return->u.i4_arg = x_mtkapi_bt_gattc_write_char(pt_args[0].u.i4_arg,
                                                       pt_args[1].u.i4_arg,
                                                       pt_args[2].u.i4_arg,
                                                       pt_args[3].u.i4_arg,
                                                       pt_args[4].u.i4_arg,
                                                       pt_args[5].u.pc_arg);
    return RPCR_OK;
}

static INT32 _hndlr_x_mtkapi_bt_gattc_write_descr(RPC_ID_T     t_rpc_id,
                                                              const CHAR*  ps_cb_type,
                                                              UINT32       ui4_num_args,
                                                              ARG_DESC_T*  pt_args,
                                                              ARG_DESC_T*  pt_return)
{
    BT_RH_LOG("[_hndlr_]bt_gattc_write_descr, arg_1 = %ld, arg_2 = %ld, arg_3 = %ld, arg_4 = %ld, arg_5 = %ld, arg_6 = %s\n",
              pt_args[0].u.i4_arg, pt_args[1].u.i4_arg, pt_args[2].u.i4_arg,
              pt_args[3].u.i4_arg, pt_args[4].u.i4_arg, pt_args[5].u.pc_arg);
    if (ui4_num_args != 6)
    {
        BT_RH_LOG("Invalid ARGS: %d\n", ui4_num_args);
        return RPCR_INV_ARGS;
    }
    pt_return->e_type   = ARG_TYPE_INT32;
    pt_return->u.i4_arg = x_mtkapi_bt_gattc_write_descr(pt_args[0].u.i4_arg,
                                                        pt_args[1].u.i4_arg,
                                                        pt_args[2].u.i4_arg,
                                                        pt_args[3].u.i4_arg,
                                                        pt_args[4].u.i4_arg,
                                                        pt_args[5].u.pc_arg);
    return RPCR_OK;
}

static INT32 _hndlr_x_mtkapi_bt_gattc_execute_write(RPC_ID_T     t_rpc_id,
                                                                 const CHAR*  ps_cb_type,
                                                                 UINT32       ui4_num_args,
                                                                 ARG_DESC_T*  pt_args,
                                                                 ARG_DESC_T*  pt_return)
{
    BT_RH_LOG("[_hndlr_]bt_gattc_execute_write, arg_1 = %ld, arg_2 = %ld\n",
              pt_args[0].u.i4_arg, pt_args[1].u.i4_arg);
    if (ui4_num_args != 2)
    {
        BT_RH_LOG("Invalid ARGS: %d\n", ui4_num_args);
        return RPCR_INV_ARGS;
    }
    pt_return->e_type   = ARG_TYPE_INT32;
    pt_return->u.i4_arg = x_mtkapi_bt_gattc_execute_write(pt_args[0].u.i4_arg,
                                                          pt_args[1].u.i4_arg);
    return RPCR_OK;
}

static INT32 _hndlr_x_mtkapi_bt_gattc_set_reg_noti(RPC_ID_T     t_rpc_id,
                                                               const CHAR*  ps_cb_type,
                                                               UINT32       ui4_num_args,
                                                               ARG_DESC_T*  pt_args,
                                                               ARG_DESC_T*  pt_return)
{
    BT_RH_LOG("[_hndlr_]bt_gattc_reg_noti, arg_1 = %ld, arg_2 = %s, arg_3 = %ld, arg_4 = %d\n",
              pt_args[0].u.i4_arg, pt_args[1].u.pc_arg,
              pt_args[2].u.i4_arg, pt_args[3].u.b_arg);

    if (ui4_num_args != 4)
    {
        BT_RH_LOG("Invalid ARGS: %d\n", ui4_num_args);
        return RPCR_INV_ARGS;
    }

    pt_return->e_type   = ARG_TYPE_INT32;
    pt_return->u.i4_arg = x_mtkapi_bt_gattc_set_reg_noti(pt_args[0].u.i4_arg,
                                                         pt_args[1].u.pc_arg,
                                                         pt_args[2].u.i4_arg,
                                                         pt_args[3].u.b_arg);
    return RPCR_OK;
}

static INT32 _hndlr_x_mtkapi_bt_gattc_read_rssi(RPC_ID_T     t_rpc_id,
                                                           const CHAR*  ps_cb_type,
                                                           UINT32       ui4_num_args,
                                                           ARG_DESC_T*  pt_args,
                                                           ARG_DESC_T*  pt_return)
{
    BT_RH_LOG("[_hndlr_]bt_gattc_read_rssi, arg_1 = %ld, arg_2 = %s\n",
        pt_args[0].u.i4_arg, pt_args[1].u.pc_arg);
    if (ui4_num_args != 2)
    {
        BT_RH_LOG("Invalid ARGS: %d\n", ui4_num_args);
        return RPCR_INV_ARGS;
    }
    pt_return->e_type   = ARG_TYPE_INT32;
    pt_return->u.i4_arg = x_mtkapi_bt_gattc_read_rssi(pt_args[0].u.i4_arg,
                                                     pt_args[1].u.pc_arg);
    return RPCR_OK;
}

static INT32 _hndlr_x_mtkapi_bt_gattc_scan_filter_param_setup(RPC_ID_T     t_rpc_id,
                                                                              const CHAR*  ps_cb_type,
                                                                              UINT32       ui4_num_args,
                                                                              ARG_DESC_T*  pt_args,
                                                                              ARG_DESC_T*  pt_return)
{
     BT_RH_LOG("[_hndlr_]bt_gattc_scan_filter_param_setup, arg_1 = %p\n", pt_args[0].u.pv_desc);
    if (ui4_num_args != 1)
    {
        BT_RH_LOG("Invalid ARGS: %d\n", ui4_num_args);
        return RPCR_INV_ARGS;
    }
    pt_return->e_type   = ARG_TYPE_INT32;
    pt_return->u.i4_arg = x_mtkapi_bt_gattc_scan_filter_param_setup(pt_args[0].u.pv_desc);
    return RPCR_OK;
}

static INT32 _hndlr_x_mtkapi_bt_gattc_scan_filter_enable(RPC_ID_T     t_rpc_id,
                                                                      const CHAR*  ps_cb_type,
                                                                      UINT32       ui4_num_args,
                                                                      ARG_DESC_T*  pt_args,
                                                                      ARG_DESC_T*  pt_return)
{
     BT_RH_LOG("[_hndlr_]bt_gattc_scan_filter_enable, arg_1 = %ld, arg_2 = %d\n",
               pt_args[0].u.i4_arg, pt_args[1].u.b_arg);
    if (ui4_num_args != 2)
    {
        BT_RH_LOG("Invalid ARGS: %d\n", ui4_num_args);
        return RPCR_INV_ARGS;
    }
    pt_return->e_type   = ARG_TYPE_INT32;
    pt_return->u.i4_arg = x_mtkapi_bt_gattc_scan_filter_enable(pt_args[0].u.i4_arg,
                                                               pt_args[1].u.b_arg);
    return RPCR_OK;
}

static INT32 _hndlr_x_mtkapi_bt_gattc_scan_filter_add_remove(RPC_ID_T     t_rpc_id,
                                                                             const CHAR*  ps_cb_type,
                                                                             UINT32       ui4_num_args,
                                                                             ARG_DESC_T*  pt_args,
                                                                             ARG_DESC_T*  pt_return)
{
     BT_RH_LOG("[_hndlr_]bt_gattc_scan_filter_add_remove, arg_1 = %ld\n",
               pt_args[0].u.i4_arg);
    if (ui4_num_args != 14)
    {
        BT_RH_LOG("Invalid ARGS: %d\n", ui4_num_args);
        return RPCR_INV_ARGS;
    }
    pt_return->e_type   = ARG_TYPE_INT32;
    pt_return->u.i4_arg = x_mtkapi_bt_gattc_scan_filter_add_remove(pt_args[0].u.i4_arg,
                                                                   pt_args[1].u.i4_arg,
                                                                   pt_args[2].u.i4_arg,
                                                                   pt_args[3].u.i4_arg,
                                                                   pt_args[4].u.i4_arg,
                                                                   pt_args[5].u.i4_arg,
                                                                   pt_args[6].u.pc_arg,
                                                                   pt_args[7].u.pc_arg,
                                                                   pt_args[8].u.pc_arg,
                                                                   pt_args[9].u.c_arg,
                                                                   pt_args[10].u.i4_arg,
                                                                   pt_args[11].u.pc_arg,
                                                                   pt_args[12].u.i4_arg,
                                                                   pt_args[13].u.pc_arg);
    return RPCR_OK;
}

static INT32 _hndlr_x_mtkapi_bt_gattc_scan_filter_clear(RPC_ID_T     t_rpc_id,
                                                                    const CHAR*  ps_cb_type,
                                                                    UINT32       ui4_num_args,
                                                                    ARG_DESC_T*  pt_args,
                                                                    ARG_DESC_T*  pt_return)
{
     BT_RH_LOG("[_hndlr_]bt_gattc_scan_filter_clear, arg_1 = %ld, arg_2 = %ld\n",
               pt_args[0].u.i4_arg, pt_args[1].u.i4_arg);
    if (ui4_num_args != 2)
    {
        BT_RH_LOG("Invalid ARGS: %d\n", ui4_num_args);
        return RPCR_INV_ARGS;
    }
    pt_return->e_type   = ARG_TYPE_INT32;
    pt_return->u.i4_arg = x_mtkapi_bt_gattc_scan_filter_clear(pt_args[0].u.i4_arg,
                                                              pt_args[1].u.i4_arg);
    return RPCR_OK;
}

static INT32 _hndlr_x_mtkapi_bt_gattc_get_device_type(RPC_ID_T     t_rpc_id,
                                                      const CHAR*  ps_cb_type,
                                                      UINT32       ui4_num_args,
                                                      ARG_DESC_T*  pt_args,
                                                      ARG_DESC_T*  pt_return)
{
     BT_RH_LOG("[_hndlr_]bt_gattc_get_device_type, arg_1 = %s\n", pt_args[0].u.pc_arg);
    if (ui4_num_args != 1)
    {
        BT_RH_LOG("Invalid ARGS: %d\n", ui4_num_args);
        return RPCR_INV_ARGS;
    }
    pt_return->e_type   = ARG_TYPE_INT32;
    pt_return->u.i4_arg = x_mtkapi_bt_gattc_get_device_type(pt_args[0].u.pc_arg);
    return RPCR_OK;
}

static INT32 _hndlr_x_mtkapi_bt_gattc_configure_mtu(RPC_ID_T     t_rpc_id,
                                                                 const CHAR*  ps_cb_type,
                                                                 UINT32       ui4_num_args,
                                                                 ARG_DESC_T*  pt_args,
                                                                 ARG_DESC_T*  pt_return)
{
     BT_RH_LOG("[_hndlr_]bt_gattc_configure_mtu, arg_1 = %ld, arg_2 = %ld\n",
               pt_args[0].u.i4_arg, pt_args[1].u.i4_arg);
    if (ui4_num_args != 2)
    {
        BT_RH_LOG("Invalid ARGS: %d\n", ui4_num_args);
        return RPCR_INV_ARGS;
    }
    pt_return->e_type   = ARG_TYPE_INT32;
    pt_return->u.i4_arg = x_mtkapi_bt_gattc_configure_mtu(pt_args[0].u.i4_arg,
                                                          pt_args[1].u.i4_arg);
    return RPCR_OK;
}

static INT32 _hndlr_x_mtkapi_bt_gattc_conn_parameter_update(RPC_ID_T     t_rpc_id,
                                                                              const CHAR*  ps_cb_type,
                                                                              UINT32       ui4_num_args,
                                                                              ARG_DESC_T*  pt_args,
                                                                              ARG_DESC_T*  pt_return)
{
     BT_RH_LOG("[_hndlr_]bt_gattc_conn_parameter_update, arg_1 = %s, arg_2 = %ld, arg_3 = %ld, arg_4 = %ld, arg_5 = %ld\n",
               pt_args[0].u.pc_arg, pt_args[1].u.i4_arg, pt_args[2].u.i4_arg,
               pt_args[3].u.i4_arg, pt_args[4].u.i4_arg);
    if (ui4_num_args != 5)
    {
        BT_RH_LOG("Invalid ARGS: %d\n", ui4_num_args);
        return RPCR_INV_ARGS;
    }
    pt_return->e_type   = ARG_TYPE_INT32;
    pt_return->u.i4_arg = x_mtkapi_bt_gattc_conn_parameter_update(pt_args[0].u.pc_arg,
                                                                  pt_args[1].u.i4_arg,
                                                                  pt_args[2].u.i4_arg,
                                                                  pt_args[3].u.i4_arg,
                                                                  pt_args[4].u.i4_arg);
    return RPCR_OK;
}

static INT32 _hndlr_x_mtkapi_bt_gattc_set_scan_parameters(RPC_ID_T     t_rpc_id,
                                                                          const CHAR*  ps_cb_type,
                                                                          UINT32       ui4_num_args,
                                                                          ARG_DESC_T*  pt_args,
                                                                          ARG_DESC_T*  pt_return)
{
     BT_RH_LOG("[_hndlr_]bt_gattc_set_scan_parameters, arg_1 = %ld, arg_2 = %ld, arg_3 = %ld\n",
        pt_args[0].u.i4_arg, pt_args[1].u.i4_arg, pt_args[2].u.i4_arg);
    if (ui4_num_args != 3)
    {
        BT_RH_LOG("Invalid ARGS: %d\n", ui4_num_args);
        return RPCR_INV_ARGS;
    }
    pt_return->e_type   = ARG_TYPE_INT32;
    pt_return->u.i4_arg = x_mtkapi_bt_gattc_set_scan_parameters(pt_args[0].u.i4_arg,
                                                                pt_args[1].u.i4_arg,
                                                                pt_args[2].u.i4_arg);
    return RPCR_OK;
}

static INT32 _hndlr_x_mtkapi_bt_gattc_set_disc_mode(RPC_ID_T     t_rpc_id,
                                                                  const CHAR*  ps_cb_type,
                                                                  UINT32       ui4_num_args,
                                                                  ARG_DESC_T*  pt_args,
                                                                  ARG_DESC_T*  pt_return)
{
     BT_RH_LOG("[_hndlr_]bt_gattc_set_disc_mode, arg_1 = %ld, arg_2 = %ld\n",
               pt_args[0].u.i4_arg, pt_args[1].u.i4_arg);
    if (ui4_num_args != 2)
    {
        BT_RH_LOG("Invalid ARGS: %d\n", ui4_num_args);
        return RPCR_INV_ARGS;
    }
    pt_return->e_type   = ARG_TYPE_INT32;
    pt_return->u.i4_arg = x_mtkapi_bt_gattc_set_disc_mode(pt_args[0].u.i4_arg,
                                                          pt_args[1].u.i4_arg);
    return RPCR_OK;
}

static INT32 _hndlr_x_mtkapi_bt_gattc_multi_adv_enable(RPC_ID_T     t_rpc_id,
                                                                     const CHAR*  ps_cb_type,
                                                                     UINT32       ui4_num_args,
                                                                     ARG_DESC_T*  pt_args,
                                                                     ARG_DESC_T*  pt_return)
{
     BT_RH_LOG("[_hndlr_]bt_gattc_multi_adv_enable, arg_1 = %ld, arg_2 = %ld, arg_3 = %ld\n",
               pt_args[0].u.i4_arg, pt_args[1].u.i4_arg, pt_args[2].u.i4_arg);
    if (ui4_num_args != 7)
    {
        BT_RH_LOG("Invalid ARGS: %d\n", ui4_num_args);
        return RPCR_INV_ARGS;
    }
    pt_return->e_type   = ARG_TYPE_INT32;
    pt_return->u.i4_arg = x_mtkapi_bt_gattc_multi_adv_enable(pt_args[0].u.i4_arg,
                                                             pt_args[1].u.i4_arg,
                                                             pt_args[2].u.i4_arg,
                                                             pt_args[3].u.i4_arg,
                                                             pt_args[4].u.i4_arg,
                                                             pt_args[5].u.i4_arg,
                                                             pt_args[6].u.i4_arg);
    return RPCR_OK;
}

static INT32 _hndlr_x_mtkapi_bt_gattc_multi_adv_update(RPC_ID_T     t_rpc_id,
                                                                      const CHAR*  ps_cb_type,
                                                                      UINT32       ui4_num_args,
                                                                      ARG_DESC_T*  pt_args,
                                                                      ARG_DESC_T*  pt_return)
{
     BT_RH_LOG("[_hndlr_]bt_gattc_multi_adv_update, arg_1 = %ld, arg_2 = %ld, arg_3 = %ld\n",
               pt_args[0].u.i4_arg, pt_args[1].u.i4_arg, pt_args[2].u.i4_arg);
    if (ui4_num_args != 7)
    {
        BT_RH_LOG("Invalid ARGS: %d\n", ui4_num_args);
        return RPCR_INV_ARGS;
    }
    pt_return->e_type   = ARG_TYPE_INT32;
    pt_return->u.i4_arg = x_mtkapi_bt_gattc_multi_adv_update(pt_args[0].u.i4_arg,
                                                             pt_args[1].u.i4_arg,
                                                             pt_args[2].u.i4_arg,
                                                             pt_args[3].u.i4_arg,
                                                             pt_args[4].u.i4_arg,
                                                             pt_args[5].u.i4_arg,
                                                             pt_args[6].u.i4_arg);
    return RPCR_OK;
}

static INT32 _hndlr_x_mtkapi_bt_gattc_multi_adv_setdata(RPC_ID_T     t_rpc_id,
                                                                      const CHAR*  ps_cb_type,
                                                                      UINT32       ui4_num_args,
                                                                      ARG_DESC_T*  pt_args,
                                                                      ARG_DESC_T*  pt_return)
{
     BT_RH_LOG("[_hndlr_]bt_gattc_multi_adv_setdata, arg_1 = %ld, arg_2 = %d, arg_3 = %d\n",
               pt_args[0].u.i4_arg, pt_args[1].u.ui1_arg, pt_args[2].u.ui1_arg);
    if (ui4_num_args != 11)
    {
        BT_RH_LOG("Invalid ARGS: %d\n", ui4_num_args);
        return RPCR_INV_ARGS;
    }

    pt_return->e_type   = ARG_TYPE_INT32;
    pt_return->u.i4_arg = x_mtkapi_bt_gattc_multi_adv_setdata(pt_args[0].u.i4_arg,
                                                              pt_args[1].u.ui1_arg,
                                                              pt_args[2].u.ui1_arg,
                                                              pt_args[3].u.ui1_arg,
                                                              pt_args[4].u.i4_arg,
                                                              pt_args[5].u.i4_arg,
                                                              pt_args[6].u.pc_arg,
                                                              pt_args[7].u.i4_arg,
                                                              pt_args[8].u.pc_arg,
                                                              pt_args[9].u.i4_arg,
                                                              pt_args[10].u.pc_arg);
    return RPCR_OK;
}

static INT32 _hndlr_x_mtkapi_bt_gattc_multi_adv_disable(RPC_ID_T     t_rpc_id,
                                                                      const CHAR*  ps_cb_type,
                                                                      UINT32       ui4_num_args,
                                                                      ARG_DESC_T*  pt_args,
                                                                      ARG_DESC_T*  pt_return)
{
    BT_RH_LOG("[_hndlr_]bt_gattc_multi_adv_disable, arg_1 = %ld\n", pt_args[0].u.i4_arg);
    if (ui4_num_args != 1)
    {
        BT_RH_LOG("Invalid ARGS: %d\n", ui4_num_args);
        return RPCR_INV_ARGS;
    }
    pt_return->e_type   = ARG_TYPE_INT32;
    pt_return->u.i4_arg = x_mtkapi_bt_gattc_multi_adv_disable(pt_args[0].u.i4_arg);
    return RPCR_OK;
}

static INT32 _hndlr_x_mtkapi_bt_gattc_batchscan_cfg_storage(RPC_ID_T     t_rpc_id,
                                                                            const CHAR*  ps_cb_type,
                                                                            UINT32       ui4_num_args,
                                                                            ARG_DESC_T*  pt_args,
                                                                            ARG_DESC_T*  pt_return)
{
    BT_RH_LOG("[_hndlr_]bt_gattc_batchscan_cfg_storage, arg_1 = %ld\n",
              pt_args[0].u.i4_arg);
    if (ui4_num_args != 4)
    {
        BT_RH_LOG("Invalid ARGS: %d\n", ui4_num_args);
        return RPCR_INV_ARGS;
    }
    pt_return->e_type   = ARG_TYPE_INT32;
    pt_return->u.i4_arg = x_mtkapi_bt_gattc_batchscan_cfg_storage(pt_args[0].u.i4_arg,
                                                                  pt_args[1].u.i4_arg,
                                                                  pt_args[2].u.i4_arg,
                                                                  pt_args[3].u.i4_arg);
    return RPCR_OK;
}

static INT32 _hndlr_x_mtkapi_bt_gattc_batchscan_enb_batch_scan(RPC_ID_T     t_rpc_id,
                                                                                  const CHAR*  ps_cb_type,
                                                                                  UINT32       ui4_num_args,
                                                                                  ARG_DESC_T*  pt_args,
                                                                                  ARG_DESC_T*  pt_return)
{
    BT_RH_LOG("[_hndlr_]bt_gattc_batchscan_enb_batch_scan, arg_1 = %ld\n", pt_args[0].u.i4_arg);
    if (ui4_num_args != 6)
    {
        BT_RH_LOG("Invalid ARGS: %d\n", ui4_num_args);
        return RPCR_INV_ARGS;
    }
    pt_return->e_type   = ARG_TYPE_INT32;
    pt_return->u.i4_arg = x_mtkapi_bt_gattc_batchscan_enb_batch_scan(pt_args[0].u.i4_arg,
                                                                     pt_args[1].u.i4_arg,
                                                                     pt_args[2].u.i4_arg,
                                                                     pt_args[3].u.i4_arg,
                                                                     pt_args[4].u.i4_arg,
                                                                     pt_args[5].u.i4_arg);
    return RPCR_OK;
}

static INT32 _hndlr_x_mtkapi_bt_gattc_batchscan_dis_batch_scan(RPC_ID_T     t_rpc_id,
                                                                                 const CHAR*  ps_cb_type,
                                                                                 UINT32       ui4_num_args,
                                                                                 ARG_DESC_T*  pt_args,
                                                                                 ARG_DESC_T*  pt_return)
{
    BT_RH_LOG("[_hndlr_]bt_gattc_batchscan_dis_batch_scan, arg_1 = %ld\n", pt_args[0].u.i4_arg);
    if (ui4_num_args != 1)
    {
        BT_RH_LOG("Invalid ARGS: %d\n", ui4_num_args);
        return RPCR_INV_ARGS;
    }
    pt_return->e_type   = ARG_TYPE_INT32;
    pt_return->u.i4_arg = x_mtkapi_bt_gattc_batchscan_dis_batch_scan(pt_args[0].u.i4_arg);
    return RPCR_OK;
}

static INT32 _hndlr_x_mtkapi_bt_gattc_batchscan_read_reports(RPC_ID_T     t_rpc_id,
                                                                              const CHAR*  ps_cb_type,
                                                                              UINT32       ui4_num_args,
                                                                              ARG_DESC_T*  pt_args,
                                                                              ARG_DESC_T*  pt_return)
{
     BT_RH_LOG("[_hndlr_]bt_gattc_batchscan_read_reports, arg_1 = %ld, arg_2 = %ld\n",
               pt_args[0].u.i4_arg, pt_args[1].u.i4_arg);
    if (ui4_num_args != 2)
    {
        BT_RH_LOG("Invalid ARGS: %d\n", ui4_num_args);
        return RPCR_INV_ARGS;
    }

    pt_return->e_type   = ARG_TYPE_INT32;
    pt_return->u.i4_arg = x_mtkapi_bt_gattc_batchscan_read_reports(pt_args[0].u.i4_arg,
                                                                   pt_args[1].u.i4_arg);
    return RPCR_OK;
}

#if MTK_LINUX_GATTC_LE_NAME
static INT32 _hndlr_x_mtkapi_bt_gattc_set_local_le_name(RPC_ID_T     t_rpc_id,
                                                                       const CHAR*  ps_cb_type,
                                                                       UINT32       ui4_num_args,
                                                                       ARG_DESC_T*  pt_args,
                                                                       ARG_DESC_T*  pt_return)
{
     BT_RH_LOG("[_hndlr_]bt_gattc_set_local_le_name, arg_1 = %ld, arg_2 = %s\n",
               pt_args[0].u.i4_arg, pt_args[1].u.pc_arg);
    if (ui4_num_args != 2)
    {
        BT_RH_LOG("Invalid ARGS: %d\n", ui4_num_args);
        return RPCR_INV_ARGS;
    }
    pt_return->e_type   = ARG_TYPE_INT32;
    pt_return->u.i4_arg = x_mtkapi_bt_gattc_set_local_le_name(pt_args[0].u.i4_arg,
                                                              pt_args[1].u.pc_arg);
    return RPCR_OK;
}
#endif

static INT32 _hndlr_x_mtkapi_bt_gattc_get_connect_result_info(RPC_ID_T     t_rpc_id,
                                                                              const CHAR*  ps_cb_type,
                                                                              UINT32       ui4_num_args,
                                                                              ARG_DESC_T*  pt_args,
                                                                              ARG_DESC_T*  pt_return)
{
    BT_RH_LOG("[_hndlr_]bt_gattc_get_connect_result_info, arg_1 = %p\n", pt_args[0].u.pv_desc);
    if (ui4_num_args != 1)
    {
        BT_RH_LOG("Invalid ARGS: %d\n", ui4_num_args);
        return RPCR_INV_ARGS;
    }
    pt_return->e_type   = ARG_TYPE_INT32;
    pt_return->u.i4_arg = RPCR_OK;
    x_mtkapi_bt_gattc_get_connect_result_info(pt_args[0].u.pv_desc);
    return RPCR_OK;
}

static INT32 _hndlr_x_mtkapi_bt_gattc_get_disconnect_result_info(RPC_ID_T     t_rpc_id,
                                                                                 const CHAR*  ps_cb_type,
                                                                                 UINT32       ui4_num_args,
                                                                                 ARG_DESC_T*  pt_args,
                                                                                 ARG_DESC_T*  pt_return)
{
    BT_RH_LOG("[_hndlr_]bt_gattc_get_disconnect_result_info, arg_1 = %p\n", pt_args[0].u.pv_desc);
    if (ui4_num_args != 1)
    {
        BT_RH_LOG("Invalid ARGS: %d\n", ui4_num_args);
        return RPCR_INV_ARGS;
    }
    pt_return->e_type   = ARG_TYPE_INT32;
    pt_return->u.i4_arg = RPCR_OK;
    x_mtkapi_bt_gattc_get_disconnect_result_info(pt_args[0].u.pv_desc);
    return RPCR_OK;
}

static INT32 _hndlr_x_mtkapi_bt_gattc_read_rssi_result_info(RPC_ID_T     t_rpc_id,
                                                                          const CHAR*  ps_cb_type,
                                                                          UINT32       ui4_num_args,
                                                                          ARG_DESC_T*  pt_args,
                                                                          ARG_DESC_T*  pt_return)
{
    BT_RH_LOG("[_hndlr_]bt_gattc_read_rssi_result_info, arg_1 = %p\n", pt_args[0].u.pv_desc);
    if (ui4_num_args != 1)
    {
        BT_RH_LOG("Invalid ARGS: %d\n", ui4_num_args);
        return RPCR_INV_ARGS;
    }
    pt_return->e_type   = ARG_TYPE_INT32;
    pt_return->u.i4_arg = RPCR_OK;
    x_mtkapi_bt_gattc_read_rssi_result_info(pt_args[0].u.pv_desc);
    return RPCR_OK;
}

INT32 c_rpc_reg_mtk_bt_service_gattc_op_hndlrs(VOID)
{
    RPC_REG_OP_HNDLR(x_mtkapi_bt_gattc_register_callback);
    RPC_REG_OP_HNDLR(x_mtkapi_bt_gattc_register_app);
    RPC_REG_OP_HNDLR(x_mtkapi_bt_gattc_unregister_app);
    RPC_REG_OP_HNDLR(x_mtkapi_bt_gattc_scan);
    RPC_REG_OP_HNDLR(x_mtkapi_bt_gattc_connect);
    RPC_REG_OP_HNDLR(x_mtkapi_bt_gattc_disconnect);
    RPC_REG_OP_HNDLR(x_mtkapi_bt_gattc_listen);
    RPC_REG_OP_HNDLR(x_mtkapi_bt_gattc_refresh);
    RPC_REG_OP_HNDLR(x_mtkapi_bt_gattc_search_service);
    RPC_REG_OP_HNDLR(x_mtkapi_bt_gattc_get_gatt_db);
    RPC_REG_OP_HNDLR(x_mtkapi_bt_gattc_read_char);
    RPC_REG_OP_HNDLR(x_mtkapi_bt_gattc_read_descr);
    RPC_REG_OP_HNDLR(x_mtkapi_bt_gattc_write_char);
    RPC_REG_OP_HNDLR(x_mtkapi_bt_gattc_write_descr);
    RPC_REG_OP_HNDLR(x_mtkapi_bt_gattc_execute_write);
    RPC_REG_OP_HNDLR(x_mtkapi_bt_gattc_set_reg_noti);
    RPC_REG_OP_HNDLR(x_mtkapi_bt_gattc_read_rssi);
    RPC_REG_OP_HNDLR(x_mtkapi_bt_gattc_scan_filter_param_setup);
    RPC_REG_OP_HNDLR(x_mtkapi_bt_gattc_scan_filter_enable);
    RPC_REG_OP_HNDLR(x_mtkapi_bt_gattc_scan_filter_add_remove);
    RPC_REG_OP_HNDLR(x_mtkapi_bt_gattc_scan_filter_clear);
    RPC_REG_OP_HNDLR(x_mtkapi_bt_gattc_get_device_type);
    RPC_REG_OP_HNDLR(x_mtkapi_bt_gattc_configure_mtu);
    RPC_REG_OP_HNDLR(x_mtkapi_bt_gattc_conn_parameter_update);
    RPC_REG_OP_HNDLR(x_mtkapi_bt_gattc_set_scan_parameters);
    RPC_REG_OP_HNDLR(x_mtkapi_bt_gattc_multi_adv_enable);
    RPC_REG_OP_HNDLR(x_mtkapi_bt_gattc_set_disc_mode);
    RPC_REG_OP_HNDLR(x_mtkapi_bt_gattc_multi_adv_update);
    RPC_REG_OP_HNDLR(x_mtkapi_bt_gattc_multi_adv_setdata);
    RPC_REG_OP_HNDLR(x_mtkapi_bt_gattc_multi_adv_disable);
    RPC_REG_OP_HNDLR(x_mtkapi_bt_gattc_batchscan_cfg_storage);
    RPC_REG_OP_HNDLR(x_mtkapi_bt_gattc_batchscan_enb_batch_scan);
    RPC_REG_OP_HNDLR(x_mtkapi_bt_gattc_batchscan_dis_batch_scan);
    RPC_REG_OP_HNDLR(x_mtkapi_bt_gattc_batchscan_read_reports);
    RPC_REG_OP_HNDLR(x_mtkapi_bt_gattc_get_connect_result_info);
    RPC_REG_OP_HNDLR(x_mtkapi_bt_gattc_get_disconnect_result_info);
    RPC_REG_OP_HNDLR(x_mtkapi_bt_gattc_read_rssi_result_info);
#if MTK_LINUX_GATTC_LE_NAME
    RPC_REG_OP_HNDLR(x_mtkapi_bt_gattc_set_local_le_name);
#endif
    RPC_REG_OP_HNDLR(x_mtkapi_bt_gattc_unregister_callback);
    return RPCR_OK;
}


