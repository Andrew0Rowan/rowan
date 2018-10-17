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

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "btmw_rpc_test_cli.h"
#include "btmw_rpc_test_debug.h"
#include "btmw_rpc_test_gattc_if.h"
#include "btmw_rpc_test_gatt_if.h"
#include "mtk_bt_service_gattc_wrapper.h"
#include "mtk_bt_service_gatt_wrapper.h"


#define ADVERTISING_CHANNEL_ALL 7    //ADVERTISING_CHANNEL_37 | ADVERTISING_CHANNEL_38 | ADVERTISING_CHANNEL_39;

INT32 g_btm_rpc_client_if = 0;

INT32 ascii_2_hex(CHAR *p_ascii, INT32 len, UINT8 *p_hex)
{
    INT32     x;
    UINT8     c;
    if (NULL == p_ascii || NULL == p_hex)
    {
        return BT_ERR_STATUS_PARM_INVALID;
    }
    for (x = 0; (x < len) && (*p_ascii); x++)
    {
        if (isdigit (*p_ascii))
            c = (*p_ascii - '0') << 4;
        else
            c = (toupper(*p_ascii) - 'A' + 10) << 4;
        p_ascii++;
        if (*p_ascii)
        {
            if (isdigit (*p_ascii))
                c |= (*p_ascii - '0');
            else
                c |= (toupper(*p_ascii) - 'A' + 10);
            p_ascii++;
        }
        *p_hex++ = c;
    }
    return x;
}

static int btmw_rpc_test_gatt_profile_init(int argc, char **argv)
{

    BTMW_RPC_TEST_Logi("[GATTC] %s()\n", __func__);
    return a_mtkapi_bt_gatt_profile_init();
}

static int btmw_rpc_test_gatt_profile_deinit(int argc, char **argv)
{

    BTMW_RPC_TEST_Logi("[GATTC] %s()\n", __func__);
    return a_mtkapi_bt_gatt_profile_deinit();
}




//Basic Gatt Client function
static void btmw_rpc_test_gattc_register_client_callback(BT_GATTC_REG_CLIENT_T *pt_reg_client_result ,
                                                                      void* pv_tag)
{
    if (NULL == pt_reg_client_result)
    {
        BTMW_RPC_TEST_Loge("[GATTC] %s(), pt_reg_client_result is NULL\n", __func__);
        return;
    }
    g_btm_rpc_client_if = pt_reg_client_result->client_if;
    BTMW_RPC_TEST_Logi("[GATTC] %s(), btm_client_if = %d\n", __func__, g_btm_rpc_client_if);
    return;
}

static void btmw_rpc_test_gattc_event_callback(BT_GATTC_EVENT_T bt_gatt_event,
                                                           void* pv_tag)
{
    BTMW_RPC_TEST_Logi("[GATTC] %s(), bt_gattc_event =%d\n",  __func__, bt_gatt_event);
    return;
}

static void btmw_rpc_test_gattc_scan_result_callback(BT_GATTC_SCAN_RST_T *pt_scan_result,
                                                                  void* pv_tag)
{
    if (NULL == pt_scan_result)
    {
        BTMW_RPC_TEST_Loge("[GATTC] %s(), pt_scan_result is NULL\n", __func__);
        return;
    }
    BTMW_RPC_TEST_Logi("[GATTC] %s(), pt_scan_result->btaddr =%s\n",  __func__,
                       pt_scan_result->btaddr);

    btmw_rpc_test_gatt_adv_data_t adv_data;
    btmw_rpc_test_gatt_decode_adv_data(pt_scan_result->adv_data, &adv_data);
}

static void btmw_rpc_test_gattc_get_gatt_db_callback(BT_GATTC_GET_GATT_DB_T *pt_get_gatt_db_result,
                                                                   void* pv_tag)
{
    BT_GATTC_DB_ELEMENT_T *curr_db_ptr = pt_get_gatt_db_result->gatt_db_element;
    int i = 0;

    BTMW_RPC_TEST_Logi("[GATTC] %s(), count =%ld\n",  __func__, (long)pt_get_gatt_db_result->count);
    for (i = 0; i < pt_get_gatt_db_result->count; i++)
    {
        curr_db_ptr->type = pt_get_gatt_db_result->gatt_db_element->type;
        curr_db_ptr->attribute_handle = pt_get_gatt_db_result->gatt_db_element->attribute_handle;
        curr_db_ptr->start_handle = pt_get_gatt_db_result->gatt_db_element->start_handle;
        curr_db_ptr->end_handle = pt_get_gatt_db_result->gatt_db_element->end_handle;
        curr_db_ptr->id = pt_get_gatt_db_result->gatt_db_element->id;
        curr_db_ptr->properties = pt_get_gatt_db_result->gatt_db_element->properties;
        memcpy(pt_get_gatt_db_result->gatt_db_element->uuid, curr_db_ptr->uuid, BT_GATT_MAX_UUID_LEN);

        BTMW_RPC_TEST_Logi("type = %ld, attribute_handle = %d\n",curr_db_ptr->type, curr_db_ptr->attribute_handle);
        BTMW_RPC_TEST_Logi("start_handle = %d, end_handle = %d\n",curr_db_ptr->start_handle, curr_db_ptr->end_handle);
        BTMW_RPC_TEST_Logi("id = %d, properties = %d\n",curr_db_ptr->id, curr_db_ptr->properties);
        BTMW_RPC_TEST_Logi("uuid = %s\n",curr_db_ptr->uuid);
        BTMW_RPC_TEST_Logi("\n\n");
        curr_db_ptr++;
        pt_get_gatt_db_result->gatt_db_element++;
    }
}

static void btmw_rpc_test_gattc_get_reg_noti_callback(BT_GATTC_GET_REG_NOTI_RST_T *pt_get_reg_noti_result,
                                                                    void* pv_tag)
{
    BTMW_RPC_TEST_Logi("[GATTC] %s(), registered = %ld, attribute_handle = %ld\n", __func__,
                        pt_get_reg_noti_result->registered, pt_get_reg_noti_result->handle);
}

static void btmw_rpc_test_gattc_notify_callback(BT_GATTC_GET_NOTIFY_T *pt_notify,
                                                           void* pv_tag)
{
    BTMW_RPC_TEST_Logi("[GATTC] %s(), handle = %ld, bda = %s\n", __func__,
                        (long)pt_notify->notify_data.handle, pt_notify->notify_data.bda);
}

static void btmw_rpc_test_gattc_read_char_callback(BT_GATTC_READ_CHAR_RST_T *pt_read_char,
                                                                void* pv_tag)
{
    BTMW_RPC_TEST_Logi("[GATTC] %s(), handle = %ld, value = %s\n", __func__,
                        (long)pt_read_char->read_data.handle, pt_read_char->read_data.value.value);
}

static void btmw_rpc_test_gattc_write_char_callback(BT_GATTC_WRITE_CHAR_RST_T *pt_write_char,
                                                                 void* pv_tag)
{
    BTMW_RPC_TEST_Logi("[GATTC] %s(), handle = %ld\n", __func__, (long)pt_write_char->handle);
}

static void btmw_rpc_test_gattc_read_desc_callback(BT_GATTC_READ_DESCR_RST_T *pt_read_desc,
                                                                 void* pv_tag)
{
    BTMW_RPC_TEST_Logi("[GATTC] %s(), handle = %ld, value = %s\n", __func__,
                       (long)pt_read_desc->read_data.handle,
                       pt_read_desc->read_data.value.value);
}

static void btmw_rpc_test_gattc_write_desc_callback(BT_GATTC_WRITE_DESCR_RST_T *pt_write_desc,
                                                                  void* pv_tag)
{
    BTMW_RPC_TEST_Logi("[GATTC] %s(), handle = %ld\n", __func__, (long)pt_write_desc->handle);
}

static void btmw_rpc_test_gattc_scan_filter_param_callback(BT_GATTC_SCAN_FILTER_PARAM_T *pt_scan_filter_param,
                                                                          void* pv_tag)
{
    BTMW_RPC_TEST_Logi("[GATTC] %s(), action = %ld\n", __func__, (long)pt_scan_filter_param->action);
}

static void btmw_rpc_test_gattc_scan_filter_status_callback(BT_GATTC_SCAN_FILTER_STATUS_T *pt_scan_filter_status,
                                                                          void* pv_tag)
{
    BTMW_RPC_TEST_Logi("[GATTC] %s(), enable = %ld\n", __func__, (long)pt_scan_filter_status->enable);
}

static void btmw_rpc_test_gattc_scan_filter_cfg_callback(BT_GATTC_SCAN_FILTER_CFG_T *pt_scan_filter_cfg,
                                                                       void* pv_tag)
{
    BTMW_RPC_TEST_Logi("[GATTC] %s(), action = %ld\n", __func__, (long)pt_scan_filter_cfg->action);
}

static void btmw_rpc_test_gattc_adv_enable_callback(UINT8 status, void* pv_tag)
{
    BTMW_RPC_TEST_Logi("[GATTC] %s(), status = %d\n", __func__, status);
}

static INT32 btmw_rpc_test_gattc_base_init(MTKRPCAPI_BT_APP_GATTC_CB_FUNC_T *func,
                                                     void *pv_tag)
{
    BTMW_RPC_TEST_Logi("[GATTC] btmw_rpc_test_gattc_base_init\n");
    return a_mtkapi_bt_gattc_base_init(func, pv_tag);
}

static int btmw_rpc_test_gattc_register_app(int argc, char **argv)
{
    CHAR ptr[130];
    memset(ptr,0,sizeof(ptr));
    BTMW_RPC_TEST_Logi("[GATTC] %s()\n", __func__);
    if (argc < 1)
    {
        return -1;
    }
    strncpy(ptr, argv[0], strlen(argv[0]));
    ptr[strlen(argv[0])] = '\0';
    return a_mtkapi_bt_gattc_register_app(ptr);
}

static int btmw_rpc_test_gattc_unregister_app(int argc, char **argv)
{
    BTMW_RPC_TEST_Logi("[GATTC] %s()\n", __func__);
    if (argc < 1)
    {
        BTMW_RPC_TEST_Logi("Usage :\n");
        BTMW_RPC_TEST_Logi("  GATTC unregister_app <client_if>\n");
        return -1;
    }
    INT32 client_if = 0;
    client_if = atoi(argv[0]);
    return a_mtkapi_bt_gattc_unregister_app(client_if);
}

static int btmw_rpc_test_gattc_scan(int argc, char **argv)
{

    BTMW_RPC_TEST_Logi("[GATTC] %s()\n", __func__);
    return a_mtkapi_bt_gattc_scan();
}

static int btmw_rpc_test_gattc_stop_scan(int argc, char **argv)
{
    BTMW_RPC_TEST_Logi("[GATTC] %s()\n", __func__);
    return a_mtkapi_bt_gattc_stop_scan();
}

static int btmw_rpc_test_gattc_unregister_callback(int argc, char **argv)
{
    BTMW_RPC_TEST_Logi("[GATTC] %s()\n", __func__);
    return a_mtkapi_bt_gattc_unregister_callback();
}

static int btmw_rpc_test_gattc_open(int argc, char **argv)
{
    BTMW_RPC_TEST_Logi("[GATTC] %s()\n", __func__);

    CHAR ps_addr[MAX_BDADDR_LEN];
    UINT8 is_direct = 1;
    INT32 transport = 0;
    INT32 client_if =0;

    if (argc < 2)
    {
        BTMW_RPC_TEST_Logi("Usage :\n");
        BTMW_RPC_TEST_Logi("  GATTC open <client_if> <addr> [isDirect <true|false> [<transport>]]\n");
        return -1;
    }

    client_if = atoi(argv[0]);
    strncpy(ps_addr,argv[1], MAX_BDADDR_LEN);
    ps_addr[MAX_BDADDR_LEN - 1] = '\0';
    if (argc >= 3)
    {
        // set isDirect, opt.
        CHAR *temp = argv[2];
        if (strcmp(temp,"1")||strcmp(temp,"true")||strcmp(temp,"TRUE"))
        {
            is_direct = 1;
        }
        else
        {
            is_direct = 0;
        }
    }

    if (argc >= 4)
    {
        // set transport, opt.
        CHAR *temp = argv[3];
        transport = atoi(temp);
    }
    return a_mtkapi_bt_gattc_open(client_if, ps_addr, is_direct, transport);
}

static int btmw_rpc_test_gattc_close(int argc, char **argv)
{
    BTMW_RPC_TEST_Logi("[GATTC] %s()\n", __func__);

    CHAR ps_addr[MAX_BDADDR_LEN];
    INT32 conn_id = 0;
    INT32 client_if = 0;
    if (argc < 3)
    {
        BTMW_RPC_TEST_Logi("Usage :\n");
        BTMW_RPC_TEST_Logi("  GATTC close <client_if> <addr> <conn_id>\n");
        return -1;
    }

    client_if = atoi(argv[0]);
    strncpy(ps_addr,argv[1], MAX_BDADDR_LEN);
    ps_addr[MAX_BDADDR_LEN - 1] = '\0';
    conn_id = atoi(argv[2]);

    return a_mtkapi_bt_gattc_close(client_if, ps_addr, conn_id);
}

static int btmw_rpc_test_gattc_listen(int argc, char **argv)
{
    BTMW_RPC_TEST_Logi("[GATTC] %s()\n", __func__);
    INT32 client_if = 0;
    if (argc < 1)
    {
        BTMW_RPC_TEST_Logi("Usage :\n");
        BTMW_RPC_TEST_Logi("  GATTC listen <client_if>\n");
        return -1;
    }
    client_if = atoi(argv[0]);
    return a_mtkapi_bt_gattc_listen(client_if);
}

static int btmw_rpc_test_gattc_refresh(int argc, char **argv)
{
    BTMW_RPC_TEST_Logi("[GATTC] %s()\n", __func__);
    INT32 client_if = 0;
    CHAR ps_addr[MAX_BDADDR_LEN];

    if (argc < 2)
    {
        BTMW_RPC_TEST_Logi("Usage :\n");
        BTMW_RPC_TEST_Logi("  GATTC refresh <client_if> <addr>\n");
        return -1;
    }
    client_if = atoi(argv[0]);
    strncpy(ps_addr,argv[1], MAX_BDADDR_LEN);
    ps_addr[MAX_BDADDR_LEN - 1] = '\0';

    return a_mtkapi_bt_gattc_refresh(client_if, ps_addr);
}

static int btmw_rpc_test_gattc_search_service(int argc, char **argv)
{
    CHAR pt_uuid[130] = {0};
    CHAR *pt_uuid_ptr = NULL;
    INT32 conn_id = 0;

    BTMW_RPC_TEST_Logi("[GATTC] %s()\n", __func__);
    if (argc < 1)
    {
        BTMW_RPC_TEST_Logi("Usage :\n");
        BTMW_RPC_TEST_Logi("  GATTC search_service <conn_id> <uuid>\n");
        return -1;
    }
    if (argc == 2)
    {
        strncpy(pt_uuid,argv[1], strlen(argv[1]));
        pt_uuid[strlen(argv[1])] = '\0';
        pt_uuid_ptr = pt_uuid;
    }

    conn_id = atoi(argv[0]);
    return a_mtkapi_bt_gattc_search_service(conn_id, pt_uuid_ptr);
}

static int btmw_rpc_test_gattc_get_gatt_db(int argc, char **argv)
{
    INT32 conn_id = 0;

    BTMW_RPC_TEST_Logi("[GATTC] %s()\n", __func__);
    if (argc < 1)
    {
        BTMW_RPC_TEST_Logi("Usage :\n");
        BTMW_RPC_TEST_Logi("  GATTC get_gatt_db <conn_id>\n");
        return -1;
    }
    conn_id = atoi(argv[0]);
    return a_mtkapi_bt_gattc_get_gatt_db(conn_id);
}

static int btmw_rpc_test_gattc_read_char(int argc, char **argv)
{
    INT32 conn_id = 0;
    INT32 char_handle = 0;
    INT32 auth_req = 0;

    BTMW_RPC_TEST_Logi("[GATTC] %s()\n", __func__);
    if (argc < 2)
    {
        BTMW_RPC_TEST_Logi("Usage :\n");
        BTMW_RPC_TEST_Logi("  GATTC read_char <conn_id> <characteristic_handle> [<auth_req>]\n");
        return -1;
    }
    conn_id = atoi(argv[0]);
    char_handle = atoi(argv[1]);
    if (argc == 3)
    {
        auth_req = atoi(argv[2]);
    }

    return a_mtkapi_bt_gattc_read_char(conn_id, char_handle, auth_req);
}

static int btmw_rpc_test_gattc_read_descr(int argc, char **argv)
{
    INT32 conn_id = 0;
    INT32 auth_req = 0;
    INT32 descr_handle = 0;

    BTMW_RPC_TEST_Logi("[GATTC] %s()\n", __func__);
    if (argc < 2)
    {
        BTMW_RPC_TEST_Logi("Usage :\n");
        BTMW_RPC_TEST_Logi("  GATTC read_descr <conn_id> <descr_handle> [<auth_req>]\n");
        return -1;
    }
    conn_id = atoi(argv[0]);
    descr_handle = atoi(argv[1]);
    if (argc == 3)
    {
        auth_req = atoi(argv[2]);
    }

    return a_mtkapi_bt_gattc_read_descr(conn_id, descr_handle, auth_req);
}

static int btmw_rpc_test_gattc_write_char(int argc, char **argv)
{
    INT32 conn_id = 0;
    INT32 auth_req = 0;
    INT32 write_type = 2; //WRITE_TYPE_DEFAULT = 2, WRITE_TYPE_NO_RESPONSE = 1, WRITE_TYPE_SIGNED = 4
    INT32 char_handle = 0;
    CHAR *value;
    BTMW_RPC_TEST_Logi("[GATTC] %s()\n", __func__);
    if (argc < 4)
    {
        BTMW_RPC_TEST_Logi("Usage :\n");
        BTMW_RPC_TEST_Logi("  GATTC write_char <conn_id> <char_handle> <write_type> [<auth_req>] <value>\n");
        return -1;
    }
    conn_id = atoi(argv[0]);
    char_handle = atoi(argv[1]);
    write_type = atoi(argv[2]);
    if (argc == 5)
    {
        auth_req = atoi(argv[3]);
        value= argv[4];
    }
    else
    {
        value= argv[3];
    }

    return a_mtkapi_bt_gattc_write_char(conn_id, char_handle, write_type, strlen(value), auth_req, value);
}

static int btmw_rpc_test_gattc_write_descr(int argc, char **argv)
{
    INT32 conn_id = 0;
    INT32 auth_req = 0;
    INT32 write_type = 2; //WRITE_TYPE_DEFAULT = 2, WRITE_TYPE_NO_RESPONSE = 1, WRITE_TYPE_SIGNED = 4
    INT32 descr_handle = 0;
    CHAR *value;
    BTMW_RPC_TEST_Logi("[GATTC] %s()\n", __func__);
    if (argc < 4)
    {
        BTMW_RPC_TEST_Logi("Usage :\n");
        BTMW_RPC_TEST_Logi("  GATTC write_descr <conn_id> <descr_handle> <write_type> [<auth_req>] <value>\n");
        return -1;
    }
    conn_id = atoi(argv[0]);
    descr_handle = atoi(argv[1]);
    write_type = atoi(argv[2]);
    if (argc == 5)
    {
        auth_req = atoi(argv[3]);
        value= argv[4];
    }
    else
    {
        value= argv[3];
    }

    return a_mtkapi_bt_gattc_write_descr(conn_id, descr_handle, write_type, strlen(value), auth_req, value);
}

static int btmw_rpc_test_gattc_execute_write(int argc, char **argv)
{
    BTMW_RPC_TEST_Logi("[GATTC] %s()\n", __func__);
    INT32 conn_id = 0;
    INT32 execute = 0;

    if (argc < 2)
    {
        BTMW_RPC_TEST_Logi("Usage :\n");
        BTMW_RPC_TEST_Logi("  GATTC execute_write <conn_id> <execute>\n");
        return -1;
    }
    conn_id = atoi(argv[0]);
    execute = atoi(argv[1]);

    return a_mtkapi_bt_gattc_execute_write(conn_id, execute);
}

static int btmw_rpc_test_gattc_reg_noti(int argc, char **argv)
{
    INT32 char_handle = 0;
    CHAR *bt_addr;
    INT32 client_if = 0;
    BTMW_RPC_TEST_Logi("[GATTC] %s()\n", __func__);

    if (argc < 3)
    {
        BTMW_RPC_TEST_Logi("Usage :\n");
        BTMW_RPC_TEST_Logi("  GATTC reg_noti <client_if> <addr> <char_handle>\n");
        return -1;
    }
    client_if = atoi(argv[0]);
    bt_addr = argv[1];
    char_handle = atoi(argv[2]);
    return a_mtkapi_bt_gattc_reg_noti(client_if, bt_addr, char_handle);
}

static int btmw_rpc_test_gattc_dereg_noti(int argc, char **argv)
{
    CHAR *bt_addr;
    INT32 char_handle = 0;
    INT32 client_if = 0;
    BTMW_RPC_TEST_Logi("[GATTC] %s()\n", __func__);

    if (argc < 3)
    {
        BTMW_RPC_TEST_Logi("Usage :\n");
        BTMW_RPC_TEST_Logi("  GATTC dereg_noti <client_if> <addr> <char_handle>\n");
        return -1;
    }
    client_if = atoi(argv[0]);
    bt_addr = argv[1];
    char_handle = atoi(argv[2]);

    return a_mtkapi_bt_gattc_dereg_noti(client_if, bt_addr, char_handle);
}

static int btmw_rpc_test_gattc_read_rssi(int argc, char **argv)
{
    BTMW_RPC_TEST_Logi("[GATTC] %s()\n", __func__);

    CHAR *bt_addr;
    INT32 client_if = 0;
    if (argc < 2)
    {
        BTMW_RPC_TEST_Logi("Usage :\n");
        BTMW_RPC_TEST_Logi("  GATTC read_rssi <client_if> <addr>\n");
        return -1;
    }
    client_if = atoi(argv[0]);
    bt_addr = argv[1];

    return a_mtkapi_bt_gattc_read_rssi(client_if, bt_addr);
}

// Scan filter function
//#define LE_ACTION_TYPE_ADD         0
//#define LE_ACTION_TYPE_DEL          1
//#define LE_ACTION_TYPE_CLEAR      2
static int btmw_rpc_test_gattc_scan_filter_param_setup(int argc, char **argv)
{
    BTMW_RPC_TEST_Logi("[GATTC] %s()\n", __func__);
    BT_GATTC_FILT_PARAM_SETUP_T scan_filt_param;
    memset(&scan_filt_param, 0, sizeof(BT_GATTC_FILT_PARAM_SETUP_T));
    if (argc < 2)
    {
         BTMW_RPC_TEST_Logi("Usage :\n");
         BTMW_RPC_TEST_Logi("  GATTC scan_filter_param_setup <client_if> <action> [filt_index <int>] [feat_seln <int>] [list_logic_type <hex_string>] [filt_logic_type <int>] [rssi_high_thres <int>] [rssi_low_thres <int>] [dely_mode <int>] [found_timeout <int>] [lost_timeout <int>] [found_timeout_cnt <int>] [num_of_tracking_entries <int>]\n");
         return -1;
    }
    scan_filt_param.client_if = atoi(argv[0]);
    scan_filt_param.action = atoi(argv[1]);
    INT32 count = 2;
    while (count < argc)
    {
        if (strcmp(argv[count],"filt_index") == 0)
        {
            count++;
            scan_filt_param.filt_index = atoi(argv[count]);
            count++;
            BTMW_RPC_TEST_Logi("filt_index : %d\n" ,scan_filt_param.filt_index);
            continue;
        }
        else if (strcmp(argv[count],"feat_seln") == 0)
        {
            count++;
            scan_filt_param.feat_seln = atoi(argv[count]);
            count++;
            BTMW_RPC_TEST_Logi("feat_seln : %d\n" ,scan_filt_param.feat_seln);
            continue;
        }
        else if (strcmp(argv[count],"list_logic_type") == 0)
        {
            count++;
            scan_filt_param.list_logic_type = strtol(argv[count],NULL,16);
            count++;
            BTMW_RPC_TEST_Logi("list_logic_type : %d\n" ,scan_filt_param.list_logic_type);
            continue;
        }
        else if (strcmp(argv[count],"filt_logic_type") == 0)
        {
            count++;
            scan_filt_param.filt_logic_type = atoi(argv[count]);
            count++;
            BTMW_RPC_TEST_Logi("filt_logic_type : %d\n" ,scan_filt_param.filt_logic_type);
            continue;
        }
        else if (strcmp(argv[count],"rssi_high_thres") == 0)
        {
            count++;
            scan_filt_param.rssi_high_thres = atoi(argv[count]);
            count++;
            BTMW_RPC_TEST_Logi("rssi_high_thres : %d\n" ,scan_filt_param.rssi_high_thres);
            continue;
        }
        else if (strcmp(argv[count],"rssi_low_thres") == 0)
        {
            count++;
            scan_filt_param.rssi_low_thres = atoi(argv[count]);
            count++;
            BTMW_RPC_TEST_Logi("rssi_low_thres : %d\n" ,scan_filt_param.rssi_low_thres);
            continue;
        }
        else if (strcmp(argv[count],"dely_mode") == 0)
        {
            count++;
            scan_filt_param.dely_mode = atoi(argv[count]);
            count++;
            BTMW_RPC_TEST_Logi("dely_mode : %d\n" ,scan_filt_param.dely_mode);
            continue;
        }
        else if (strcmp(argv[count],"found_timeout") == 0)
        {
            count++;
            scan_filt_param.found_timeout = atoi(argv[count]);
            count++;
            BTMW_RPC_TEST_Logi("found_timeout : %d\n" ,scan_filt_param.found_timeout);
            continue;
        }
        else if (strcmp(argv[count],"lost_timeout") == 0)
        {
            count++;
            scan_filt_param.lost_timeout = atoi(argv[count]);
            count++;
            BTMW_RPC_TEST_Logi("lost_timeout : %d\n" ,scan_filt_param.lost_timeout);
            continue;
        }
        else if (strcmp(argv[count],"found_timeout_cnt") == 0)
        {
            count++;
            scan_filt_param.found_timeout_cnt = atoi(argv[count]);
            count++;
            BTMW_RPC_TEST_Logi("found_timeout_cnt : %d\n" ,scan_filt_param.found_timeout_cnt);
            continue;
        }
        else if (strcmp(argv[count],"num_of_tracking_entries") == 0)
        {
            count++;
            scan_filt_param.num_of_tracking_entries = atoi(argv[count]);
            count++;
            BTMW_RPC_TEST_Logi("num_of_tracking_entries : %d\n" ,scan_filt_param.num_of_tracking_entries);
            continue;
        }
        count+=2;
    }

    return a_mtkapi_bt_gattc_scan_filter_param_setup(&scan_filt_param);

}

static int btmw_rpc_test_gattc_scan_filter_enable(int argc, char **argv)
{
    BTMW_RPC_TEST_Logi("[GATTC] %s()\n", __func__);
    INT32 client_if = 0;

    if (argc < 1)
    {
        BTMW_RPC_TEST_Logi("Usage :\n");
        BTMW_RPC_TEST_Logi("  GATTC scan_filter_enable client_if \n");
        return -1;
    }
    client_if = atoi(argv[0]);
    return a_mtkapi_bt_gattc_scan_filter_enable(client_if);
}

static int btmw_rpc_test_gattc_scan_filter_disable(int argc, char **argv)
{
    BTMW_RPC_TEST_Logi("[GATTC] %s()\n", __func__);
    INT32 client_if = 0;

    if (argc < 1)
    {
        BTMW_RPC_TEST_Logi("Usage :\n");
        BTMW_RPC_TEST_Logi("  GATTC scan_filter_disable client_if \n");
        return -1;
    }
    client_if = atoi(argv[0]);

    return a_mtkapi_bt_gattc_scan_filter_disable(client_if);
}

static int btmw_rpc_test_gattc_scan_filter_add_remove(int argc, char **argv)
{
    BTMW_RPC_TEST_Logi("[GATTC] %s()\n", __func__);
    INT32 client_if,action;
    INT32 filt_type = 0;
    INT32 filt_index = 0;
    INT32 company_id = 0;
    INT32 company_id_mask = 0;
    CHAR pt_uuid[130] = {0};
    CHAR *ptr_uuid = NULL;
    CHAR pt_uuid_mask[130] = {0};
    CHAR *ptr_uuid_mask = NULL;
    CHAR pt_addr[130] = {0};
    CHAR *ptr_addr = NULL;
    INT32 addr_type=0;
    INT32 data_len=0;
    CHAR* p_data=NULL;
    INT32 mask_len=0;
    CHAR* p_mask=NULL;
    if (argc < 2)
    {
         BTMW_RPC_TEST_Logi("Usage :\n");
         BTMW_RPC_TEST_Logi("  GATTC scan_filter_add_remove <client_if> <action> [filt_index <int>] [filt_type <int>] [company_id <hex_string>] [company_id_mask <hex_string>] [uuid <hex_string>] [uuid_mask <hex_string>] [bd_addr <string>] [addr_type <int>] [data <hex_string/string>] [data_mask <hex_string/string>]\n");
         return -1;
    }
    client_if = atoi(argv[0]);
    action = atoi(argv[1]);
    INT32 count = 2;
    while (count < argc)
    {
        if (strcmp(argv[count],"filt_index") == 0)
        {
            count++;
            filt_index = atoi(argv[count]);
            count++;
            BTMW_RPC_TEST_Logi("filt_index : %d\n" ,filt_index);
            continue;
        }
        else if (strcmp(argv[count],"filt_type") == 0)
        {
            count++;
            filt_type = atoi(argv[count]);
            count++;
            BTMW_RPC_TEST_Logi("filt_type : %d\n" ,filt_type);
            continue;
        }
        else if (strcmp(argv[count],"company_id") == 0)
        {
            count++;
            company_id = strtol(argv[count],NULL,16);
            count++;
            BTMW_RPC_TEST_Logi("company_id : %d\n" ,company_id);
            continue;
        }
        else if (strcmp(argv[count],"company_id_mask") == 0)
        {
            count++;
            company_id_mask = strtol(argv[count],NULL,16);
            count++;
            BTMW_RPC_TEST_Logi("company_id_mask : %d\n" ,company_id_mask);
            continue;
        }
        else if (strcmp(argv[count],"uuid") == 0)
        {
            count++;
            strncpy(pt_uuid,argv[count], strlen(argv[count]));
            pt_uuid[strlen(argv[count])] = '\0';
            ptr_uuid = pt_uuid;
            count++;
            BTMW_RPC_TEST_Logi("uuid : %s\n" ,pt_uuid);
            continue;
        }
        else if (strcmp(argv[count],"uuid_mask") == 0)
        {
            count++;
            strncpy(pt_uuid_mask,argv[count], strlen(argv[count]));
            pt_uuid_mask[strlen(argv[count])] = '\0';
            ptr_uuid_mask = pt_uuid_mask;
            count++;
            BTMW_RPC_TEST_Logi("uuid_mask : %s\n" ,pt_uuid_mask);
            continue;
        }
        else if (strcmp(argv[count],"bd_addr") == 0)
        {
            count++;
            strncpy(pt_addr,argv[count], strlen(argv[count]));
            pt_addr[strlen(argv[count])] = '\0';
            ptr_addr = pt_addr;
            count++;
            continue;
        }
        else if (strcmp(argv[count],"addr_type") == 0)
        {
            count++;
            addr_type = atoi(argv[count]);
            count++;
            BTMW_RPC_TEST_Logi("addr_type : %d\n" ,addr_type);
            continue;
        }
        else if (strcmp(argv[count],"data") == 0)
        {
            count++;
            switch(filt_type)
            {
                case 0: // BTM_BLE_PF_ADDR_FILTER
                case 2: // BTM_BLE_PF_SRVC_UUID
                case 3: // BTM_BLE_PF_SRVC_SOL_UUID
                {
                    count++;
                    BTMW_RPC_TEST_Logi("data : %d\n" ,0);
                    continue;
                    break;
                }
                case 1: // BTM_BLE_PF_SRVC_DATA
                case 5: // BTM_BLE_PF_MANU_DATA
                case 6: // BTM_BLE_PF_SRVC_DATA_PATTERN
                {

                    short hex_len = (strlen(argv[count]) + 1) / 2;
                    CHAR *hex_buf = malloc(hex_len * sizeof(CHAR));
                    CHAR p_argv[256] = {0};
                    strncpy(p_argv, argv[count], strlen(argv[count]));
                    ascii_2_hex(p_argv, hex_len, (unsigned char *)hex_buf);
                    p_data = hex_buf;
                    data_len = hex_len;
                    count++;
                    BTMW_RPC_TEST_Logi("data : %d\n" ,data_len);
                    continue;
                    break;
                }
                case 4: // BTM_BLE_PF_LOCAL_NAME
                {
                    data_len = strlen(argv[count]);
                    p_data = malloc((data_len + 1) * sizeof(CHAR));
                    memcpy(p_data,argv[count],data_len + 1);
                    count++;
                    BTMW_RPC_TEST_Logi("data : %d\n" ,data_len);
                    break;
                }
                default:
                    count++;
                    break;
            }
            continue;
        }
        else if (strcmp(argv[count],"data_mask") == 0)
        {
            count++;
            switch(filt_type)
            {
                case 0: // BTM_BLE_PF_ADDR_FILTER
                case 2: // BTM_BLE_PF_SRVC_UUID
                case 3: // BTM_BLE_PF_SRVC_SOL_UUID
                {
                    count++;
                    BTMW_RPC_TEST_Logi("data_mask : %d\n" ,0);
                    continue;
                    break;
                }
                case 1: // BTM_BLE_PF_SRVC_DATA
                case 5: // BTM_BLE_PF_MANU_DATA
                case 6: // BTM_BLE_PF_SRVC_DATA_PATTERN
                {
                    short hex_len = (strlen(argv[count]) + 1) / 2;
                    CHAR *hex_buf = malloc(hex_len * sizeof(CHAR));
                    CHAR p_argv[256] = {0};
                    strncpy(p_argv, argv[count], strlen(argv[count]));
                    ascii_2_hex(p_argv, hex_len, (unsigned char *)hex_buf);
                    p_mask = hex_buf;
                    mask_len = hex_len;
                    count++;
                    BTMW_RPC_TEST_Logi("data_mask : %d\n" ,mask_len);
                    continue;
                    break;
                }
                case 4: // BTM_BLE_PF_LOCAL_NAME
                {
                #if 0
                    p_mask = argv[count];
                    mask_len = strlen(argv[count]);
                    count++;
                #endif
                    short hex_len = (strlen(argv[count]) + 1) / 2;
                    CHAR *hex_buf = malloc(hex_len * sizeof(CHAR));
                    CHAR p_argv[256] = {0};
                    strncpy(p_argv, argv[count], strlen(argv[count]));
                    ascii_2_hex(p_argv, hex_len, (unsigned char *)hex_buf);
                    p_mask = hex_buf;
                    mask_len = hex_len;
                    count++;
                    BTMW_RPC_TEST_Logi("data_mask : %d\n" ,mask_len);
                    break;
                }
                default:
                    count++;
                    break;
            }
            continue;
        }
        count+=2;
    }

    return a_mtkapi_bt_gattc_scan_filter_add_remove(client_if, action, filt_type, filt_index, company_id,
                                              company_id_mask, ptr_uuid, ptr_uuid_mask, ptr_addr,
                                              addr_type, data_len, p_data, mask_len, p_mask);
}

static int btmw_rpc_test_gattc_scan_filter_clear(int argc, char **argv)
{
    BTMW_RPC_TEST_Logi("[GATTC] %s()\n", __func__);
    INT32 filt_index = 0;
    INT32 client_if = 0;
    if (argc < 2)
    {
         BTMW_RPC_TEST_Logi("Usage :\n");
         BTMW_RPC_TEST_Logi("  GATTC scan_filter_clear <client_if> <filt_index>\n");
         return -1;
    }
    client_if = atoi(argv[0]);
    filt_index = atoi(argv[1]);
    return a_mtkapi_bt_gattc_scan_filter_clear(client_if, filt_index);
}

// Parameters function
static int btmw_rpc_test_gattc_get_device_type(int argc, char **argv)
{
    BTMW_RPC_TEST_Logi("[GATTC] %s()\n", __func__);
    return 0;
}

static int btmw_rpc_test_gattc_set_adv_data(int argc, char **argv)
{
#if 0
    BTMW_RPC_TEST_Logi("[GATTC] %s()\n", __func__);
    if (argc < 1)
    {
         BTMW_RPC_TEST_Logi("Usage :\n");
         BTMW_RPC_TEST_Logi("  GATTC set_adv_data <client_if> [set_scan_rsp <true|false>] [include_name <true|false>] [incl_txpower <true|false>] [min_interval <int>] [max_interval <int>] [appearance <int>] [manufacturer_data <hex_string>] [service_data <hex_string>] [service_uuid <hex_string>]\n");
         return -1;
    }
    int count = 0;
    bool set_scan_rsp = false;
    bool include_name = true;
    bool incl_txpower = false;
    int appearance = 0;
    char *manufacturer_data = NULL;
    char *service_data = NULL;
    char *service_uuid = NULL;
    short manufacturer_len = 0;
    short service_data_len = 0;
    short service_uuid_len = 0;
    int min_interval,max_interval;

    while (count < argc)
    {
        if (strcmp(argv[count],"set_scan_rsp") == 0)
        {
            count++;
            if (strcmp(argv[count],"1") == 0||strcmp(argv[count],"true") == 0||strcmp(argv[count],"TRUE") == 0)
            {
                set_scan_rsp = true;
            }
            else
            {
                set_scan_rsp = false;
            }
            count++;
            BTMW_RPC_TEST_Logi("set_scan_rsp : %d\n" ,set_scan_rsp);
            continue;
        }
        else if (strcmp(argv[count],"include_name") == 0)
        {
            count++;
            if (strcmp(argv[count],"1") == 0||strcmp(argv[count],"true") == 0||strcmp(argv[count],"TRUE") == 0)
            {
                include_name = true;
            }
            else
            {
                include_name = false;
            }
            count++;
            BTMW_RPC_TEST_Logi("include_name : %d\n" ,include_name);
            continue;
        }
        else if (strcmp(argv[count],"incl_txpower") == 0)
        {
            count++;
            if (strcmp(argv[count],"1") == 0||strcmp(argv[count],"true") == 0||strcmp(argv[count],"TRUE") == 0)
            {
                incl_txpower = true;
            }
            else
            {
                incl_txpower = false;
            }
            count++;
            BTMW_RPC_TEST_Logi("incl_txpower : %d\n" ,incl_txpower);
            continue;
        }
        else if (strcmp(argv[count],"min_interval") == 0)
        {
            count++;
            min_interval = (atoi(argv[count]))*1000/625;
            count++;
            BTMW_RPC_TEST_Logi("min_interval : %d\n" ,min_interval);
            continue;
        }
        else if (strcmp(argv[count],"max_interval") == 0)
        {
            count++;
            max_interval = (atoi(argv[count]))*1000/625;
            count++;
            BTMW_RPC_TEST_Logi("max_interval : %d\n" ,max_interval);
            continue;
        }
        else if (strcmp(argv[count],"appearance") == 0)
        {
            count++;
            appearance = atoi(argv[count]);
            count++;
            BTMW_RPC_TEST_Logi("appearance : %d\n" ,appearance);
            continue;
        }
        else if (strcmp(argv[count],"manufacturer_data") == 0)
        {
            count++;
            short hex_len = (strlen(argv[count]) + 1) / 2;
            char *hex_buf = malloc(hex_len * sizeof(char));
            ascii_2_hex(argv[count], hex_len, hex_buf);
            manufacturer_data = hex_buf;
            manufacturer_len = hex_len;
            count++;
            BTMW_RPC_TEST_Logi("manufacturer_len : %d\n" ,manufacturer_len);
            continue;
        }
        else if (strcmp(argv[count],"service_data") == 0)
        {
            count++;
            short hex_len = (strlen(argv[count]) + 1) / 2;
            char *hex_buf = malloc(hex_len * sizeof(char));
            ascii_2_hex(argv[count], hex_len, hex_buf);
            service_data = hex_buf;
            service_data_len = hex_len;
            count++;
            BTMW_RPC_TEST_Logi("service_data_len : %d\n" ,service_data_len);
            continue;
        }
        else if (strcmp(argv[count],"service_uuid") == 0)
        {
            count++;
            short hex_len = (strlen(argv[count]) + 1) / 2;
            char *hex_buf = malloc(hex_len * sizeof(char));
            ascii_2_hex(argv[count], hex_len, hex_buf);
            service_uuid = hex_buf;
            service_uuid_len = hex_len;
            count++;
            BTMW_RPC_TEST_Logi("service_uuid_len : %d\n" ,service_uuid_len);
            continue;
        }
        count+=2;
    }
    //btmw_rpc_test_gattc_interface->set_adv_data(btmw_rpc_test_client_if,set_scan_rsp, include_name, incl_txpower,min_interval,max_interval,appearance
    //    ,manufacturer_len, manufacturer_data,service_data_len, service_data,service_uuid_len, service_uuid);
#endif
    return 0;
}

static int btmw_rpc_test_gattc_configure_mtu(int argc, char **argv)
{
    BTMW_RPC_TEST_Logi("[GATTC] %s()\n", __func__);
    INT32 conn_id = 0;
    INT32 mtu = 0;
    if (argc < 2)
    {
        BTMW_RPC_TEST_Logi("Usage :\n");
        BTMW_RPC_TEST_Logi("  please input GATTC configure_mtu <conn_id> <mtu>\n");
        return -1;
    }
    conn_id = atoi(argv[0]);
    mtu = atoi(argv[1]);
    if ((mtu < 23) || (mtu > 517))
    {
        BTMW_RPC_TEST_Logi("[GATTC] invalid mtu size %d.\n", mtu);
        return -1;
    }
    return a_mtkapi_bt_gattc_configure_mtu(conn_id, mtu);
}

static int btmw_rpc_test_gattc_conn_parameter_update(int argc, char **argv)
{
    BTMW_RPC_TEST_Logi("[GATTC] %s()\n", __func__);
    if (argc < 3)
    {
        BTMW_RPC_TEST_Logi("Usage :\n");
        BTMW_RPC_TEST_Logi("  GATTC conn_parameter_update <addr> <min_interval> <max_interval> [<latency> [<timeout>]]\n");
        return -1;
    }
    INT32 min_interval = 0;
    INT32 max_interval = 0;
    INT32 latency = 0;
    INT32 timeout = 0;
    latency = 0;
    timeout = 2000;
    CHAR *bt_addr;
    bt_addr = argv[0];
    CHAR *temp = argv[1];
    min_interval = (atoi(temp))*1000/625;
    temp = argv[2];
    max_interval = (atoi(temp))*1000/625;
    if (argc > 3)
    {
        temp = argv[3];
        latency = atoi(temp);
    }
    if (argc > 4)
    {
        temp = argv[4];
        timeout = atoi(temp);
    }
    return a_mtkapi_bt_gattc_conn_parameter_update(bt_addr, min_interval, max_interval, latency, timeout);
}

static int btmw_rpc_test_gattc_set_scan_parameters(int argc, char **argv)
{
    BTMW_RPC_TEST_Logi("[GATTC] %s()\n", __func__);
    if (argc < 3)
    {
        BTMW_RPC_TEST_Logi("Usage :\n");
        BTMW_RPC_TEST_Logi("  GATTC set_scan_parameters <client_if> <scan_interval> <scan_window>\n");
        return -1;
    }
    INT32 client_if = 0;
    INT32 scan_interval = 0;
    INT32 scan_window = 0;
    client_if = atoi(argv[0]);
    CHAR *temp = argv[1];
    scan_interval = (atoi(temp))*1000/625;
    temp = argv[2];
    scan_window = (atoi(temp))*1000/625;
    return a_mtkapi_bt_gattc_set_scan_parameters(client_if, scan_interval,scan_window);
}

// Multiple advertising function
static int btmw_rpc_test_gattc_multi_adv_enable(int argc, char **argv)
{
    BTMW_RPC_TEST_Logi("[GATTC] %s()\n", __func__);
    if (argc < 6)
    {
        BTMW_RPC_TEST_Logi("Usage :\n");
        BTMW_RPC_TEST_Logi("  GATTC adv_enable <client_if> <min_interval> <max_interval> <adv_type> <tx_power> <timeout>\n");
        return -1;
    }
    INT32 client_if = 0;
    INT32 min_interval = 0;
    INT32 max_interval = 0;
    INT32 adv_type = 0;
    INT32 tx_power = 0;
    INT32 timeout= 0;
    CHAR *temp = argv[0];
    client_if = atoi(temp);
    temp = argv[1];
    min_interval = (atoi(temp))*1000/625;
    temp = argv[2];
    max_interval = (atoi(temp))*1000/625;
    temp = argv[3];
    adv_type = atoi(temp);
    temp = argv[4];
    tx_power = atoi(temp);
    temp = argv[5];
    timeout = atoi(temp);
    BTMW_RPC_TEST_Logi("min_int=%u, max_int=%u, adv_type=%u, chnl_map=%u, tx_pwr=%u",min_interval,max_interval,adv_type,ADVERTISING_CHANNEL_ALL,tx_power);

    return a_mtkapi_bt_gattc_multi_adv_enable(client_if, min_interval, max_interval,
                                           adv_type, ADVERTISING_CHANNEL_ALL, tx_power, timeout);
}

static int btmw_rpc_test_gattc_set_disc_mode(int argc, char **argv)
{
    BTMW_RPC_TEST_Logi("[GATTC] %s()\n", __func__);
    if (argc < 2)
    {
        BTMW_RPC_TEST_Logi("Usage :\n");
        BTMW_RPC_TEST_Logi("  GATTC set_disc_mode <client_if> <disc_mode>\n");
        return -1;
    }
    INT32 client_if = 0;
    INT32 disc_mode = 0;

    CHAR *temp = argv[0];
    client_if = atoi(temp);
    temp = argv[1];
    disc_mode = atoi(temp);

    BTMW_RPC_TEST_Logi("client_if=%d, disc_mode=%d",client_if, disc_mode);
    return a_mtkapi_bt_gattc_set_disc_mode(client_if, disc_mode);
}


static int btmw_rpc_test_gattc_multi_adv_update(int argc, char **argv)
{
    BTMW_RPC_TEST_Logi("[GATTC] %s()\n", __func__);

    if (argc < 6)
    {
        BTMW_RPC_TEST_Logi("Usage :\n");
        BTMW_RPC_TEST_Logi("  GATTC adv_update <client_if> <min_interval> <max_interval> <adv_type> <tx_power> <timeout>\n");
        return -1;
    }

    INT32 client_if = 0;
    INT32 min_interval = 0;
    INT32 max_interval = 0;
    INT32 adv_type = 0;
    INT32 tx_power = 0;
    INT32 timeout = 0;
    CHAR *temp = argv[0];
    client_if = atoi(temp);
    temp = argv[1];
    min_interval = (atoi(temp))*1000/625;
    temp = argv[2];
    max_interval = (atoi(temp))*1000/625;
    temp = argv[3];
    adv_type = atoi(temp);
    temp = argv[4];
    tx_power = atoi(temp);
    temp = argv[5];
    timeout = atoi(temp);
    return a_mtkapi_bt_gattc_multi_adv_update(client_if, min_interval, max_interval,
                                           adv_type, ADVERTISING_CHANNEL_ALL, tx_power, timeout);
}

static int btmw_rpc_test_gattc_multi_adv_setdata(int argc, char **argv)
{
    BTMW_RPC_TEST_Logi("[GATTC] %s()\n", __func__);
    if (argc < 1)
    {
         BTMW_RPC_TEST_Logi("Usage :\n");
         BTMW_RPC_TEST_Logi("  GATTC adv_update_data <client_if> [set_scan_rsp <true|false>] [include_name <true|false>] [incl_txpower <true|false>] [appearance <int>] [manufacturer_data <hex_string>] [service_data <hex_string>] [service_uuid <hex_string>]\n");
         return -1;
    }
    INT32 client_if = 0;
    UINT8 set_scan_rsp = 0;
    UINT8 include_name = 0;
    UINT8 incl_txpower = 0;
    INT32 appearance = 0;
    INT32 manufacturer_len = 0;
    CHAR* manufacturer_data = NULL;
    INT32 service_data_len = 0;
    CHAR* service_data = NULL;
    INT32 service_uuid_len = 0;
    CHAR* service_uuid = NULL;
    INT32 le_name_len = 0;
    CHAR* le_name = NULL;
    INT32 count = 0;
    CHAR *hex_buf_manufacturer = NULL;
    CHAR *hex_buf_service_data = NULL;
    CHAR *hex_buf_service_uuid = NULL;
    client_if = atoi(argv[0]);
    count++;
    while (count < argc)
    {
        BTMW_RPC_TEST_Logi("[GATTC] %s()\n", argv[count]);
        if (strcmp(argv[count],"set_scan_rsp") == 0)
        {
            count++;
            if (strcmp(argv[count],"1") == 0||strcmp(argv[count],"true") == 0||strcmp(argv[count],"TRUE") == 0)
            {
                set_scan_rsp = 1;
            }
            else
            {
                set_scan_rsp = 0;
            }
            count++;
            continue;
        }
        else if (strcmp(argv[count],"include_name") == 0)
        {
            count++;
            if (strcmp(argv[count],"1") == 0||strcmp(argv[count],"true") == 0||strcmp(argv[count],"TRUE") == 0)
            {
                include_name = 1;
            }
            else
            {
                include_name = 0;
            }
            count++;
            continue;
        }
        else if (strcmp(argv[count],"incl_txpower") == 0)
        {
            count++;
            if (strcmp(argv[count],"1") == 0||strcmp(argv[count],"true") == 0||strcmp(argv[count],"TRUE") == 0)
            {
                incl_txpower = 1;
            }
            else
            {
                incl_txpower = 0;
            }
            count++;
            continue;
        }
        else if (strcmp(argv[count],"appearance") == 0)
        {
            count++;
            appearance = atoi(argv[count]);
            count++;
            continue;
        }
        else if (strcmp(argv[count],"manufacturer_data") == 0)
        {
            count++;
            INT32 hex_len = (strlen(argv[count]) + 1) / 2;
            hex_buf_manufacturer = malloc(hex_len * sizeof(CHAR));
            ascii_2_hex((CHAR *)argv[count], hex_len, (UINT8*)hex_buf_manufacturer);
            manufacturer_data = hex_buf_manufacturer;
            manufacturer_len = hex_len;
            count++;
            BTMW_RPC_TEST_Logi("manufacturer_len : %d\n" ,manufacturer_len);
            continue;
        }
        else if (strcmp(argv[count],"service_data") == 0)
        {
            count++;
            INT32 hex_len = (strlen(argv[count]) + 1) / 2;
            hex_buf_service_data = malloc(hex_len * sizeof(CHAR));
            ascii_2_hex((CHAR *)argv[count], hex_len, (UINT8*)hex_buf_service_data);
            service_data = hex_buf_service_data;
            service_data_len = hex_len;
            count++;
            BTMW_RPC_TEST_Logi("service_data_len : %d\n" ,service_data_len);
            continue;
        }
        else if (strcmp(argv[count],"service_uuid") == 0)
        {
            count++;
            INT32 hex_len = (strlen(argv[count]) + 1) / 2;
            hex_buf_service_uuid = malloc(hex_len * sizeof(CHAR));
            ascii_2_hex((CHAR *)argv[count], hex_len, (UINT8*)hex_buf_service_uuid);
            service_uuid = hex_buf_service_uuid;
            service_uuid_len = hex_len;
            count++;
            BTMW_RPC_TEST_Logi("service_uuid_len : %d\n" ,service_uuid_len);
            continue;
        }
        else if (strcmp(argv[count],"le_name") == 0)
        {
            count++;
            le_name_len = strlen(argv[count])+1;
            if (NULL != le_name)
            {
                free(le_name);
            }
            le_name = malloc(le_name_len * sizeof(CHAR));
            if (le_name)
            {
                memcpy(le_name, argv[count], le_name_len);
            }
            count++;
            BTMW_RPC_TEST_Logi("len = %d,le_name : %s\n",le_name_len ,le_name);
            continue;
        }
        count+=2;
    }
    a_mtkapi_bt_gattc_multi_adv_setdata(client_if,
                                        set_scan_rsp,
                                        include_name,
                                        incl_txpower,
                                        appearance,
                                        manufacturer_len,
                                        manufacturer_data,
                                        service_data_len,
                                        service_data,
                                        service_uuid_len,
                                        service_uuid);
    if (manufacturer_data)
        free(manufacturer_data);
    if (service_data)
        free(service_data);
    if (service_uuid)
        free(service_uuid);
    if (le_name)
        free(le_name);
    return 0;
}

static int btmw_rpc_test_gattc_multi_adv_disable(int argc, char **argv)
{
    BTMW_RPC_TEST_Logi("[GATTC] %s()\n", __func__);
    if (argc < 1)
    {
        BTMW_RPC_TEST_Logi("Usage :\n");
        BTMW_RPC_TEST_Logi("    GATTC adv_disable <client_if>\n");
        return -1;
    }
    INT32 client_if = 0;
    CHAR *temp = argv[0];
    client_if = atoi(temp);

    return a_mtkapi_bt_gattc_multi_adv_disable(client_if);
}

// Batch scan function
static int btmw_rpc_test_gattc_batchscan_cfg_storage(int argc, char **argv)
{
    BTMW_RPC_TEST_Logi("[GATTC] %s()\n", __func__);
    if (argc < 4)
    {
        BTMW_RPC_TEST_Logi("Usage :\n");
        BTMW_RPC_TEST_Logi("    GATTC batchscan_cfg_storage <client_if> <batch_scan_full_max> <batch_scan_trunc_max> <batch_scan_notify_thresh>\n");
        return -1;
    }
    INT32 client_if = 0;
    INT32 batch_scan_full_max = 0;
    INT32 batch_scan_trunc_max = 0;
    INT32 batch_scan_notify_thresh = 0;
    client_if = atoi(argv[0]);
    batch_scan_full_max = atoi(argv[1]);
    batch_scan_trunc_max = atoi(argv[2]);
    batch_scan_notify_thresh = atoi(argv[3]);
    return a_mtkapi_bt_gattc_batchscan_cfg_storage(client_if, batch_scan_full_max, batch_scan_trunc_max, batch_scan_notify_thresh);
}

static int btmw_rpc_test_gattc_batchscan_enb_batch_scan(int argc, char **argv)
{
    BTMW_RPC_TEST_Logi("[GATTC] %s()\n", __func__);
    if (argc < 6)
    {
        BTMW_RPC_TEST_Logi("Usage :\n");
        BTMW_RPC_TEST_Logi("    GATTC enb_batch_scan <client_if> <scan_mode> <scan_interval> <scan_window> <addr_type> <discard_rule>\n");
        return -1;
    }
    INT32 client_if = 0;
    INT32 scan_mode = 3;
    INT32 scan_interval = 0;
    INT32 scan_window = 0;
    INT32 addr_type = 1;
    INT32 discard_rule = 0;
    client_if = atoi(argv[0]);
    scan_mode = atoi(argv[1]);
    scan_interval = atoi(argv[2]);
    scan_window = atoi(argv[3]);
    addr_type = atoi(argv[4]);
    discard_rule = atoi(argv[5]);
    return a_mtkapi_bt_gattc_batchscan_enb_batch_scan(client_if, scan_mode, scan_interval, scan_window, addr_type, discard_rule);;
}

static int btmw_rpc_test_gattc_batchscan_dis_batch_scan(int argc, char **argv)
{
    BTMW_RPC_TEST_Logi("[GATTC] %s()\n", __func__);
    if (argc < 1)
    {
        BTMW_RPC_TEST_Logi("Usage :\n");
        BTMW_RPC_TEST_Logi("    GATTC dis_batch_scan <client_if>\n");
        return -1;
    }
    INT32 client_if = 0;
    client_if = atoi(argv[0]);

    return a_mtkapi_bt_gattc_batchscan_dis_batch_scan(client_if);
}

static int btmw_rpc_test_gattc_batchscan_read_reports(int argc, char **argv)
{
    BTMW_RPC_TEST_Logi("[GATTC] %s()\n", __func__);
    if (argc < 2)
    {
        BTMW_RPC_TEST_Logi("Usage :\n");
        BTMW_RPC_TEST_Logi("    GATTC batchscan_read_reports <client_if> <scan_mode>\n");
        return -1;
    }
    INT32 client_if = 0;
    INT32 scan_mode = 2;
    client_if = atoi(argv[0]);
    scan_mode = atoi(argv[1]);

    return a_mtkapi_bt_gattc_batchscan_read_reports(client_if, scan_mode);
}

static int btmw_rpc_test_gattc_set_local_le_name(int argc, char **argv)
{
    INT32 client_if = 0;
    CHAR *name;
    if (argc < 2)
    {
        BTMW_RPC_TEST_Logi("Usage :\n");
        BTMW_RPC_TEST_Logi("    GATTC set_le_name <client_if> <name>\n");
        return -1;
    }
    client_if = atoi(argv[0]);
    name = argv[1];

    return a_mtkapi_bt_gattc_set_local_le_name(client_if,name);
}

static BTMW_RPC_TEST_CLI btmw_rpc_test_gattc_cli_commands[] =
{
    {(const char *)"register_app",            btmw_rpc_test_gattc_register_app,            (const char *)" = register_app <uuid>"},
    {(const char *)"unregister_app",          btmw_rpc_test_gattc_unregister_app,          (const char *)" = unregister_app <client_if>"},
    {(const char *)"scan",                    btmw_rpc_test_gattc_scan,                    (const char *)" = scan"},
    {(const char *)"stop_scan",               btmw_rpc_test_gattc_stop_scan,               (const char *)" = stop_scan"},
    {(const char *)"open",                    btmw_rpc_test_gattc_open,                    (const char *)" = open <client_if> <addr> [isDirect <true|false> [<transport>]]"},
    {(const char *)"close",                   btmw_rpc_test_gattc_close,                   (const char *)" = close <client_if> <addr> <conn_id>"},
    {(const char *)"listen",                  btmw_rpc_test_gattc_listen,                  (const char *)" = listen <client_if>"},
    {(const char *)"refresh",                 btmw_rpc_test_gattc_refresh,                 (const char *)" = refresh <client_if> <addr>"},
    {(const char *)"search_service",          btmw_rpc_test_gattc_search_service,          (const char *)" = search_service <conn_id> <uuid>"},
    {(const char *)"get_gatt_db",             btmw_rpc_test_gattc_get_gatt_db,             (const char *)" = get_gatt_db <conn_id>"},
    {(const char *)"read_char",               btmw_rpc_test_gattc_read_char,               (const char *)" = read_char <conn_id> <characteristic_handle> [<auth_req>]"},
    {(const char *)"read_descr",              btmw_rpc_test_gattc_read_descr,              (const char *)" = read_descr <conn_id> <descr_handle> [<auth_req>]"},
    {(const char *)"write_char",              btmw_rpc_test_gattc_write_char,              (const char *)" = write_char <conn_id> <char_handle> <write_type> [<auth_req>] <value>"},
    {(const char *)"write_descr",             btmw_rpc_test_gattc_write_descr,             (const char *)" = write_descr <conn_id> <descr_handle> <write_type> [<auth_req>] <value>"},
    {(const char *)"execute_write",           btmw_rpc_test_gattc_execute_write,           (const char *)" = execute_write <conn_id> <execute>"},
    {(const char *)"reg_noti",                btmw_rpc_test_gattc_reg_noti,                (const char *)" = reg_noti <client_if> <addr> <char_handle>"},
    {(const char *)"dereg_noti",              btmw_rpc_test_gattc_dereg_noti,              (const char *)" = dereg_noti <client_if> <addr> <char_handle>"},
    {(const char *)"read_rssi",               btmw_rpc_test_gattc_read_rssi,               (const char *)" = read_rssi <client_if> <addr>"},
    {(const char *)"scan_filter_param_setup", btmw_rpc_test_gattc_scan_filter_param_setup, (const char *)" = scan_filter_param_setup <client_if> <action> [filt_index <int>] [feat_seln <int>] [list_logic_type <hex_string>] [filt_logic_type <int>] [rssi_high_thres <int>] [rssi_low_thres <int>] [dely_mode <int>] [found_timeout <int>] [lost_timeout <int>] [found_timeout_cnt <int>] [num_of_tracking_entries <int>]"},
    {(const char *)"scan_filter_enable",      btmw_rpc_test_gattc_scan_filter_enable,      (const char *)" = scan_filter_enable <client_if>"},
    {(const char *)"scan_filter_disable",     btmw_rpc_test_gattc_scan_filter_disable,     (const char *)" = scan_filter_disable <client_if>"},
    {(const char *)"scan_filter_add_remove",  btmw_rpc_test_gattc_scan_filter_add_remove,  (const char *)" = scan_filter_add_remove <client_if> <action> [filt_index <int>] [filt_type <int>] [company_id <hex_string>] [company_id_mask <hex_string>] [uuid <hex_string>] [uuid_mask <hex_string>] [bd_addr <string>] [addr_type <int>] [data <hex_string/string>] [data_mask <hex_string/string>]"},
    {(const char *)"scan_filter_clear",       btmw_rpc_test_gattc_scan_filter_clear,       (const char *)" = scan_filter_clear <client_if> <filt_index>"},
    {(const char *)"get_device_type",         btmw_rpc_test_gattc_get_device_type,         (const char *)" = get_device_type"},
    {(const char *)"set_adv_data",            btmw_rpc_test_gattc_set_adv_data,            (const char *)" = set_adv_data <client_if> [set_scan_rsp <true|false>] [include_name <true|false>] [incl_txpower <true|false>] [min_interval <int>] [max_interval <int>] [appearance <int>] [manufacturer_data <hex_string>] [service_data <hex_string>] [service_uuid <hex_string>]"},
    {(const char *)"configure_mtu",           btmw_rpc_test_gattc_configure_mtu,           (const char *)" = configure_mtu <conn_id> <mtu>"},
    {(const char *)"conn_parameter_update",   btmw_rpc_test_gattc_conn_parameter_update,   (const char *)" = conn_parameter_update <addr> <min_interval> <max_interval> [<latency> [<timeout>]]"},
    {(const char *)"set_scan_parameters",     btmw_rpc_test_gattc_set_scan_parameters,     (const char *)" = set_scan_parameters <client_if> <scan_interval> <scan_window>"},
    {(const char *)"adv_enable",              btmw_rpc_test_gattc_multi_adv_enable,        (const char *)" = adv_enable <client_if> <min_interval> <max_interval> <adv_type> <tx_power> <timeout>"},
    {(const char *)"set_disc_mode",           btmw_rpc_test_gattc_set_disc_mode,           (const char *)" = set_disc_mode <client_if> <disc_mode>"},
    {(const char *)"adv_update",              btmw_rpc_test_gattc_multi_adv_update,        (const char *)" = adv_update <client_if> <min_interval> <max_interval> <adv_type> <tx_power> <timeout>"},
    {(const char *)"adv_update_data",         btmw_rpc_test_gattc_multi_adv_setdata,       (const char *)" = adv_update_data <client_if> [set_scan_rsp <true|false>] [include_name <true|false>] [incl_txpower <true|false>] [appearance <int>] [manufacturer_data <hex_string>] [service_data <hex_string>] [service_uuid <hex_string>]"},
    {(const char *)"adv_disable",             btmw_rpc_test_gattc_multi_adv_disable,       (const char *)" = adv_disable <client_if>"},
    {(const char *)"set_le_name",             btmw_rpc_test_gattc_set_local_le_name,       (const char *)" = set_le_name <client_if> <name>"},
    {(const char *)"batchscan_cfg_storage",   btmw_rpc_test_gattc_batchscan_cfg_storage,   (const char *)" = batchscan_cfg_storage <client_if> <batch_scan_full_max> <batch_scan_trunc_max> <batch_scan_notify_thresh>"},
    {(const char *)"enb_batch_scan",          btmw_rpc_test_gattc_batchscan_enb_batch_scan,(const char *)" = enb_batch_scan <client_if> <scan_mode> <scan_interval> <scan_window> <addr_type> <discard_rule>"},
    {(const char *)"dis_batch_scan",          btmw_rpc_test_gattc_batchscan_dis_batch_scan,(const char *)" = dis_batch_scan <client_if>"},
    {(const char *)"batchscan_read_reports",  btmw_rpc_test_gattc_batchscan_read_reports,  (const char *)" = batchscan_read_reports <client_if> <scan_mode>"},
    {(const char *)"gatt_profile_init",       btmw_rpc_test_gatt_profile_init,             (const char *)" = gatt_profile_init"},
    {(const char *)"gatt_profile_deinit",     btmw_rpc_test_gatt_profile_deinit,           (const char *)" = gatt_profile_deinit"},
    {NULL, NULL, NULL},
};

int btmw_rpc_test_gattc_cmd_handler(int argc, char **argv)
{
    BTMW_RPC_TEST_CLI *cmd, *match = NULL;
    int ret = 0;
    int count;

    count = 0;
    cmd = btmw_rpc_test_gattc_cli_commands;

    BTMW_RPC_TEST_Logi("[GATTC] argc: %d, argv[0]: %s\n", argc, argv[0]);

    while (cmd->cmd)
    {
        if (!strcmp(cmd->cmd, argv[0]))
        {
            match = cmd;
            count = 1;
            break;
        }
        cmd++;
    }

    if (count == 0)
    {
        BTMW_RPC_TEST_Logi("[GATTC] Unknown command '%s'\n", argv[0]);

        btmw_rpc_test_print_cmd_help(BTMW_RPC_TEST_CMD_KEY_GATTC, btmw_rpc_test_gattc_cli_commands);
        ret = -1;
    }
    else
    {
        match->handler(argc - 1, &argv[1]);
    }

    return ret;
}

int btmw_rpc_test_gattc_init()
{
    int ret = 0;
    BTMW_RPC_TEST_MOD gattc_mod = {0};

    // Register command to CLI
    gattc_mod.mod_id = BTMW_RPC_TEST_MOD_GATT_CLIENT;
    strncpy(gattc_mod.cmd_key, BTMW_RPC_TEST_CMD_KEY_GATTC, sizeof(gattc_mod.cmd_key));
    gattc_mod.cmd_handler = btmw_rpc_test_gattc_cmd_handler;
    gattc_mod.cmd_tbl = btmw_rpc_test_gattc_cli_commands;

    ret = btmw_rpc_test_register_mod(&gattc_mod);
    BTMW_RPC_TEST_Logi("[GATTC] btmw_rpc_test_register_mod() returns: %d\n", ret);
    if (!g_cli_pts_mode)
    {
        MTKRPCAPI_BT_APP_GATTC_CB_FUNC_T func;
        char pv_tag[2] = {0};
        memset(&func, 0, sizeof(MTKRPCAPI_BT_APP_GATTC_CB_FUNC_T));
        func.bt_gattc_reg_client_cb = btmw_rpc_test_gattc_register_client_callback;
        func.bt_gattc_event_cb = btmw_rpc_test_gattc_event_callback;
        func.bt_gattc_scan_cb = btmw_rpc_test_gattc_scan_result_callback;
        func.bt_gattc_get_gatt_db_cb = btmw_rpc_test_gattc_get_gatt_db_callback;
        func.bt_gattc_get_reg_noti_cb = btmw_rpc_test_gattc_get_reg_noti_callback;
        func.bt_gattc_notify_cb = btmw_rpc_test_gattc_notify_callback;
        func.bt_gattc_read_char_cb = btmw_rpc_test_gattc_read_char_callback;
        func.bt_gattc_write_char_cb = btmw_rpc_test_gattc_write_char_callback;
        func.bt_gattc_read_desc_cb = btmw_rpc_test_gattc_read_desc_callback;
        func.bt_gattc_write_desc_cb = btmw_rpc_test_gattc_write_desc_callback;
        func.bt_gattc_scan_filter_param_cb = btmw_rpc_test_gattc_scan_filter_param_callback;
        func.bt_gattc_scan_filter_status_cb = btmw_rpc_test_gattc_scan_filter_status_callback;
        func.bt_gattc_scan_filter_cfg_cb = btmw_rpc_test_gattc_scan_filter_cfg_callback;
        func.bt_gattc_adv_enable_cb = btmw_rpc_test_gattc_adv_enable_callback;
        btmw_rpc_test_gattc_base_init(&func, (void *)pv_tag);
#if 0
        if (0 == c_btm_bt_gattc_register_app(BTMW_RPC_TEST_GATTC_APP_UUID))
        {
            BTMW_RPC_TEST_Logi("[GATTC] Register client uuid:'%s'\n", BTMW_RPC_TEST_GATTC_APP_UUID);
        }
#endif
    }
    return ret;
}

int btmw_rpc_test_gattc_deinit()
{
    BTMW_RPC_TEST_Logi("%s", __func__);
    return 0;
}
