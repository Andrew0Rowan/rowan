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
#include <hardware/bluetooth.h>
#include <hardware/bt_gatt.h>
#include <hardware/bt_gatt_client.h>

#include "btif_util.h"
#include "btut_cli.h"
#include "btut_debug.h"
#include "btut_gatt_if.h"
#include "btut_gattc_if.h"

#define PTS_DISCOVER_ALL_PRIMARY_SERVICES  1
#define PTS_DISCOVER_PRIMARY_SERVICES_BY_UUID 2
#define PTS_FIND_INCLUDE_SERVICES 3
#define PTS_DISCOVER_ALL_CHARACTERISTICS_OF_SERVICE 4
#define PTS_DISCOVER_ALL_CHARACTERISTICS_BY_UUID 5
#define PTS_DISCOVER_ALL_CHARACTERISTICS_DESCRIPTOR 6
#define PTS_DISCOVER_READ_CHARACTERISTICS_VALUE 7
#define PTS_DISCOVER_READ_CHARACTERISTICS_BY_UUID 8
#define PTS_READ_LONG_CHARACTERISTIC_VALUE 9
#define PTS_READ_CHARACTERISTIC_DESCRIPTOR_BY_HANDLE  10
#define PTS_READ_LONG_CHARACTERISTIC_DESCRIPTOR_BY_HANDLE  11
#define PTS_READ_BY_TYPE 12
#define PTS_WRITE_CHAR_VALUE_BY_HANDLE 13
#define PTS_WRITE_CHAR_DESCRIPTOR_BY_HANDLE 14
#define PTS_WRITE_WITHOUT_RESPONSE_BY_HANDLE 15
#define PTS_SINGED_WRITE_WITHOUT_RESPONSE_BY_HANDLE 16

static int PTS_discovery_all_primary_services(int argc, char **argv);
static int PTS_discovery_primary_service_by_uuid(int argc, char **argv);
static int PTS_find_include_services(int argc, char **argv);
static int PTS_discover_all_characteristics_of_service(int argc, char **argv);
static int PTS_discover_all_characteristics_by_uuid(int argc, char **argv);
static int PTS_discover_all_characteristic_descriptors(int argc, char **argv);
static int PTS_read_characteristic_value_by_handle(int argc, char **argv);
static int PTS_read_using_characteristic_uuid(int argc, char **argv);
static int PTS_read_long_charactetistic_value_by_handle(int argc, char **argv);
static int PTS_read_characteristic_descriptor_by_handle(int argc, char **argv);
static int PTS_read_long_characteristic_descriptor_by_handle(int argc, char **argv);
static int PTS_read_by_type(int argc, char **argv);
static int PTS_write_characteristics_value_by_handle(int argc, char **argv);
static int PTS_write_characteristic_descriptor_by_handle(int argc, char **argv);
static int PTS_write_without_response_by_handle(int argc, char **argv);
static int PTS_signed_write_without_response_by_handle(int argc, char **argv);

const btgatt_client_interface_t *btut_gattc_interface;
static int btut_client_if = -1;
static int ADVERTISING_CHANNEL_ALL = 7;

static int PTS_discovery_all_primary_services(int argc, char **argv)
{
   int command = 0;
   btgatt_test_params_t params;

   BTUT_Logi("[GATTC] %s()\n", __func__);
   if (btut_client_if == -1)
   {
       BTUT_Logi("[GATTC] no client app registered now.\n");
       return 0;
   }
   if (argc <=2)
   {
       BTUT_Logi(" Usage :\n");
       BTUT_Logi(" PTS_discovery_all_primary_services conn_id start_handle end_handle \n");
       return -1;
   }
   params.u1=strtoul(argv[1], NULL, 16);
   params.u2=strtoul(argv[2], NULL, 16);
   params.u3=PTS_DISCOVER_ALL_PRIMARY_SERVICES;

   BTUT_Logi("disc %02X, %02X %02X", params.u1, params.u2, params.u3);

   btut_gattc_interface->test_command(command, &params);
   return 0;
}

static int PTS_discovery_primary_service_by_uuid(int argc, char **argv)
{
    int command = 0;
    btgatt_test_params_t params;

    BTUT_Logi("[GATTC] %s()\n", __func__);
    if (btut_client_if == -1)
    {
        BTUT_Logi("[GATTC] no client app registered now.\n");
        return 0;
    }
    if (argc <=2)
    {
        BTUT_Logi("Usage :\n");
        BTUT_Logi(" PTS_discovery_primary_service_by_uuid conn_id start_handle end_handle \n");
        return -1;
    }
    params.u1=strtoul(argv[1], NULL, 16);
    params.u2=strtoul(argv[2], NULL, 16);
    params.u3=PTS_DISCOVER_PRIMARY_SERVICES_BY_UUID;

    BTUT_Logi("disc %02X, %02X %02X", params.u1, params.u2, params.u3);

    btut_gattc_interface->test_command(command, &params);
    return 0;
}

static int PTS_find_include_services(int argc, char **argv)
{
    int command = 0;
    btgatt_test_params_t params;

    BTUT_Logi("[GATTC] %s()\n", __func__);
    if (btut_client_if == -1)
    {
        BTUT_Logi("[GATTC] no client app registered now.\n");
        return 0;
    }
    if (argc <=2)
    {
        BTUT_Logi("Usage :\n");
        BTUT_Logi(" PTS_find_include_services conn_id start_handle end_handle \n");
        return -1;
    }
    params.u1=strtoul(argv[1], NULL, 16);
    params.u2=strtoul(argv[2], NULL, 16);
    params.u3=PTS_FIND_INCLUDE_SERVICES;

    BTUT_Logi("disc %02X, %02X %02X", params.u1, params.u2, params.u3);

    btut_gattc_interface->test_command(command, &params);
    return 0;
}

static int PTS_discover_all_characteristics_of_service(int argc, char **argv)
{
    int command = 0;
    btgatt_test_params_t params;

    BTUT_Logi("[GATTC] %s()\n", __func__);
    if (btut_client_if == -1)
    {
        BTUT_Logi("[GATTC] no client app registered now.\n");
        return 0;
    }
    if (argc <=2)
    {
        BTUT_Logi(" Usage :\n");
        BTUT_Logi(" PTS_discover_all_characteristics_of_service conn_id start_handle end_handle \n");
        return -1;
    }
    params.u1=strtoul(argv[1], NULL, 16);
    params.u2=strtoul(argv[2], NULL, 16);
    params.u3=PTS_DISCOVER_ALL_CHARACTERISTICS_OF_SERVICE;

    BTUT_Logi("disc %02X, %02X %02X", params.u1, params.u2, params.u3);

    btut_gattc_interface->test_command(command, &params);
    return 0;
}

static int PTS_discover_all_characteristics_by_uuid(int argc, char **argv)
{
    int command = 0;
    btgatt_test_params_t params;

    BTUT_Logi("[GATTC] %s()\n", __func__);
    if (btut_client_if == -1)
    {
        BTUT_Logi("[GATTC] no client app registered now.\n");
        return 0;
    }
    if (argc <=2)
    {
        BTUT_Logi("Usage :\n");
        BTUT_Logi("  PTS_discover_all_characteristics_by_uuid conn_id start_handle end_handle \n");
        return -1;
    }
    params.u1=strtoul(argv[1], NULL, 16);
    params.u2=strtoul(argv[2], NULL, 16);
    params.u3=PTS_DISCOVER_ALL_CHARACTERISTICS_BY_UUID;

    BTUT_Logi("disc %02X, %02X %02X", params.u1, params.u2, params.u3);

    btut_gattc_interface->test_command(command, &params);
    return 0;
}

static int PTS_discover_all_characteristic_descriptors(int argc, char **argv)
{
    int command = 0;
    btgatt_test_params_t params;

    BTUT_Logi("[GATTC] %s()\n", __func__);
    if (btut_client_if == -1)
    {
        BTUT_Logi("[GATTC] no client app registered now.\n");
        return 0;
    }
    if (argc <=2)
    {
        BTUT_Logi("Usage :\n");
        BTUT_Logi("  PTS_discover_all_characteristic_descriptors conn_id start_handle end_handle \n");
        return -1;
    }
    params.u1=strtoul(argv[1], NULL, 16);
    params.u2=strtoul(argv[2], NULL, 16);
    params.u3=PTS_DISCOVER_ALL_CHARACTERISTICS_DESCRIPTOR;

    BTUT_Logi("disc %02X, %02X %02X", params.u1, params.u2, params.u3);

    btut_gattc_interface->test_command(command, &params);
    return 0;
}

static int PTS_read_characteristic_value_by_handle(int argc, char **argv)
{
    int command = 0;
    btgatt_test_params_t params;

    BTUT_Logi("[GATTC] %s()\n", __func__);
    if (btut_client_if == -1)
    {
        BTUT_Logi("[GATTC] no client app registered now.\n");
        return 0;
    }
    if (argc <=2)
    {
        BTUT_Logi("  Usage :\n");
        BTUT_Logi("  PTS_read_characteristic_value_by_handle conn_id start_handle end_handle \n");
        return -1;
    }
    params.u1=strtoul(argv[1], NULL, 16);
    params.u2=strtoul(argv[2], NULL, 16);
    params.u3=PTS_DISCOVER_READ_CHARACTERISTICS_VALUE;

    BTUT_Logi("disc %02X, %02X %02X", params.u1, params.u2, params.u3);

    btut_gattc_interface->test_command(command, &params);
    return 0;
}

static int PTS_read_using_characteristic_uuid(int argc, char **argv)
{
    int command = 0;
    btgatt_test_params_t params;

    BTUT_Logi("[GATTC] %s()\n", __func__);
    if (btut_client_if == -1)
    {
        BTUT_Logi("[GATTC] no client app registered now.\n");
        return 0;
    }
    if (argc <=2)
    {
        BTUT_Logi("Usage :\n");
        BTUT_Logi("  PTS_read_using_characteristic_uuid conn_id start_handle end_handle \n");
        return -1;
    }
    params.u1=strtoul(argv[1], NULL, 16);
    params.u2=strtoul(argv[2], NULL, 16);
    params.u3=PTS_DISCOVER_READ_CHARACTERISTICS_BY_UUID;

    BTUT_Logi("disc %02X, %02X %02X", params.u1, params.u2, params.u3);

    btut_gattc_interface->test_command(command, &params);
    return 0;
}

static int PTS_read_long_charactetistic_value_by_handle(int argc, char **argv)
{
    int command = 0;
    btgatt_test_params_t params;

    BTUT_Logi("[GATTC] %s()\n", __func__);
    if (btut_client_if == -1)
    {
        BTUT_Logi("[GATTC] no client app registered now.\n");
        return 0;
    }
    if (argc <=2)
    {
        BTUT_Logi("Usage :\n");
        BTUT_Logi("  PTS_read_long_charactetistic_value_by_handle conn_id start_handle end_handle \n");
        return -1;
    }
    params.u1=strtoul(argv[1], NULL, 16);
    params.u2=strtoul(argv[2], NULL, 16);
    params.u3=PTS_READ_LONG_CHARACTERISTIC_VALUE;

    BTUT_Logi("disc %02X, %02X %02X", params.u1, params.u2, params.u3);

    btut_gattc_interface->test_command(command, &params);
    return 0;
}

static int PTS_read_characteristic_descriptor_by_handle(int argc, char **argv)
{
    int command = 0;
    btgatt_test_params_t params;

    BTUT_Logi("[GATTC] %s()\n", __func__);
    if (btut_client_if == -1)
    {
        BTUT_Logi("[GATTC] no client app registered now.\n");
        return 0;
    }
    if (argc <=2)
    {
        BTUT_Logi("Usage :\n");
        BTUT_Logi("  PTS_read_characteristic_descriptor_by_handle conn_id start_handle end_handle \n");
        return -1;
    }
    params.u1=strtoul(argv[1], NULL, 16);
    params.u2=strtoul(argv[2], NULL, 16);
    params.u3=PTS_READ_CHARACTERISTIC_DESCRIPTOR_BY_HANDLE;

    BTUT_Logi("disc %02X, %02X %02X", params.u1, params.u2, params.u3);

    btut_gattc_interface->test_command(command, &params);
    return 0;
}

static int PTS_read_long_characteristic_descriptor_by_handle(int argc, char **argv)
{
    int command = 0;
    btgatt_test_params_t params;

    BTUT_Logi("[GATTC] %s()\n", __func__);
    if (btut_client_if == -1)
    {
        BTUT_Logi("[GATTC] no client app registered now.\n");
        return 0;
    }
    if (argc <=2)
    {
        BTUT_Logi("Usage :\n");
        BTUT_Logi("  PTS_read_long_characteristic_descriptor_by_handle conn_id start_handle end_handle \n");
        return -1;
    }
    params.u1=strtoul(argv[1], NULL, 16);
    params.u2=strtoul(argv[2], NULL, 16);
    params.u3=PTS_READ_LONG_CHARACTERISTIC_DESCRIPTOR_BY_HANDLE;

    BTUT_Logi("disc %02X, %02X %02X", params.u1, params.u2, params.u3);

    btut_gattc_interface->test_command(command, &params);
    return 0;
}

static int PTS_read_by_type(int argc, char **argv)
{
    int command = 0;
    btgatt_test_params_t params;

    BTUT_Logi("[GATTC] %s()\n", __func__);
    if (btut_client_if == -1)
    {
        BTUT_Logi("[GATTC] no client app registered now.\n");
        return 0;
    }
    if (argc <=2)
    {
        BTUT_Logi("Usage :\n");
        BTUT_Logi("  PTS_read_by_type conn_id start_handle end_handle \n");
        return -1;
    }
    params.u1=strtoul(argv[1], NULL, 16);
    params.u2=strtoul(argv[2], NULL, 16);
    params.u3=PTS_READ_BY_TYPE;

    BTUT_Logi("disc %02X, %02X %02X", params.u1, params.u2, params.u3);

    btut_gattc_interface->test_command(command, &params);
    return 0;
}

static int PTS_write_characteristics_value_by_handle(int argc, char **argv)
{
    int command = 0;
    btgatt_test_params_t params;

    BTUT_Logi("[GATTC] %s()\n", __func__);
    if (btut_client_if == -1)
    {
        BTUT_Logi("[GATTC] no client app registered now.\n");
        return 0;
    }
    if (argc <=2)
    {
        BTUT_Logi("Usage :\n");
        BTUT_Logi("  PTS_write_characteristics_value_by_handle  conn_id handle size value \n");
        return -1;
    }
    params.u1=strtoul(argv[1], NULL, 16);
    params.u2=strtoul(argv[2], NULL, 16);
    params.u3=PTS_WRITE_CHAR_VALUE_BY_HANDLE;
    params.u4=strtoul(argv[3], NULL, 16);

    BTUT_Logi("disc %02X, %02X %02X", params.u1, params.u2, params.u3);

    btut_gattc_interface->test_command(command, &params);
    return 0;
}

static int PTS_write_characteristic_descriptor_by_handle(int argc, char **argv)
{
    int command = 0;
    btgatt_test_params_t params;

    BTUT_Logi("[GATTC] %s()\n", __func__);
    if (btut_client_if == -1)
    {
        BTUT_Logi("[GATTC] no client app registered now.\n");
        return 0;
    }
    if (argc <=2)
    {
        BTUT_Logi("Usage :\n");
        BTUT_Logi("  PTS_write_characteristic_descriptor_by_handle conn_id handle value \n");
        return -1;
    }
    params.u1=strtoul(argv[1], NULL, 16);
    params.u2=strtoul(argv[2], NULL, 16);
    params.u3=PTS_WRITE_CHAR_DESCRIPTOR_BY_HANDLE;
    params.u4=strtoul(argv[3], NULL, 16);

    BTUT_Logi("disc %02X, %02X %02X", params.u1, params.u2, params.u3);

    btut_gattc_interface->test_command(command, &params);
    return 0;
}

static int PTS_write_without_response_by_handle(int argc, char **argv)
{
    int command = 0;
    btgatt_test_params_t params;

    BTUT_Logi("[GATTC] %s()\n", __func__);
    if (btut_client_if == -1)
    {
        BTUT_Logi("[GATTC] no client app registered now.\n");
        return 0;
    }
    if (argc <=2)
    {
        BTUT_Logi("Usage :\n");
        BTUT_Logi(" PTS_write_without_response_by_handlee conn_id start_handle end_handle \n");
        return -1;
    }
    params.u1=strtoul(argv[1], NULL, 16);
    params.u2=strtoul(argv[2], NULL, 16);
    params.u3=PTS_WRITE_WITHOUT_RESPONSE_BY_HANDLE;
    params.u4=strtoul(argv[3], NULL, 16);

    BTUT_Logi("disc %02X, %02X %02X", params.u1, params.u2, params.u3);

    btut_gattc_interface->test_command(command, &params);
    return 0;
}

static int PTS_signed_write_without_response_by_handle(int argc, char **argv)
{
    int command = 0;
    btgatt_test_params_t params;

    BTUT_Logi("[GATTC] %s()\n", __func__);
    if (btut_client_if == -1)
    {
        BTUT_Logi("[GATTC] no client app registered now.\n");
        return 0;
    }
    if (argc <=2)
    {
        BTUT_Logi("Usage :\n");
        BTUT_Logi("  PTS_signed_write_without_response_by_handle conn_id start_handle size value \n");
        return -1;
    }
    params.u1=strtoul(argv[1], NULL, 16);
    params.u2=strtoul(argv[2], NULL, 16);
    params.u3=PTS_SINGED_WRITE_WITHOUT_RESPONSE_BY_HANDLE;
    params.u4=strtoul(argv[3], NULL, 16);

    BTUT_Logi("disc %02X, %02X %02X", params.u1, params.u2, params.u3);

    btut_gattc_interface->test_command(command, &params);
    return 0;
}

//Basic Gatt Client function
static int btut_gattc_register_app(int argc, char **argv)
{
    char *ptr = NULL;
    bt_uuid_t uuid;

    BTUT_Logi("[GATTC] %s()\n", __func__);

    if (argc < 1)
    {
        return -1;
    }

    /*if (btut_client_if != -1)
    {
        btut_gattc_interface->unregister_client(btut_client_if);
    }*/

    ptr = argv[0];
    btut_gatt_uuid_stoh(ptr, &uuid);
    btut_gattc_interface->register_client(&uuid);

    return 0;
}

static int btut_gattc_unregister_app(int argc, char **argv)
{
    BTUT_Logi("[GATTC] %s()\n", __func__);

    /*if (btut_client_if == -1)
    {
        BTUT_Logi("[GATTC] Unregister client : no client app need to unregister.\n");
        return 0;
    }*/
    if (argc < 1)
    {
        BTUT_Logi("Usage :\n");
        BTUT_Logi("  GATTC unregister_app client_if\n");
        return -1;
    }
    int client_if = 0;
    client_if = atoi(argv[0]);
    btut_gattc_interface->unregister_client(client_if);
    btut_client_if = -1;
    return 0;
}

static int btut_gattc_scan(int argc, char **argv)
{
    BTUT_Logi("[GATTC] %s()\n", __func__);
    if (btut_client_if == -1)
    {
        BTUT_Logi("[GATTC] Scan : no client app registered now.\n");
        return 0;
    }

    btut_gattc_interface->scan(true);
    return 0;
}

static int btut_gattc_stop_scan(int argc, char **argv)
{
    BTUT_Logi("[GATTC] %s()\n", __func__);
    if (btut_client_if == -1)
    {
        BTUT_Logi("[GATTC] Stop scan : no client app registered now.\n");
        return 0;
    }
    btut_gattc_interface->scan(false);
    return 0;
}

static int btut_gattc_open(int argc, char **argv)
{
    BTUT_Logi("[GATTC] %s()\n", __func__);
    /*if (btut_client_if == -1)
    {
        BTUT_Logi("[GATTC] no client app registered now.\n");
        return 0;
    }*/
    char *ptr;
    bool is_direct = true;
    int transport = 0;
    int client_if =0;
    bt_bdaddr_t bdaddr;

    if (argc < 2)
    {
        BTUT_Logi("Usage :\n");
        BTUT_Logi("  GATTC open client_if BD_ADDR [isDirect:true|false] [transport]\n");
        return -1;
    }
    client_if = atoi(argv[0]);
    ptr = argv[1];
    if (argc >= 3)
    {
        // set isDirect, opt.
        char *temp = argv[2];
        if (strcmp(temp,"1")||strcmp(temp,"true")||strcmp(temp,"TRUE"))
        {
            is_direct = true;
        }
        else
        {
            is_direct = false;
        }
    }

    if (argc >= 4)
    {
        // set transport, opt.
        char *temp = argv[3];
        transport = atoi(temp);
    }

    btut_gatt_btaddr_stoh(ptr, &bdaddr);
    BTUT_Logi("GATTC connect to %02X:%02X:%02X:%02X:%02X:%02X ,\n",
        bdaddr.address[0], bdaddr.address[1], bdaddr.address[2],
        bdaddr.address[3], bdaddr.address[4], bdaddr.address[5]);

    btut_gattc_interface->connect(client_if,&bdaddr,is_direct,transport);

    return 0;
}

static int btut_gattc_close(int argc, char **argv)
{
    BTUT_Logi("[GATTC] %s()\n", __func__);
    /*if (btut_client_if == -1)
    {
        BTUT_Logi("[GATTC] no client app registered now.\n");
        return 0;
    }*/
    char *ptr;
    int conn_id = 0;
    bt_bdaddr_t bdaddr;
    int client_if = 0;
    if (argc < 3)
    {
        BTUT_Logi("Usage :\n");
        BTUT_Logi("  GATTC close client_if BD_ADDR conn_id \n");
        return -1;
    }
    client_if = atoi(argv[0]);
    ptr = argv[1];
    char *temp = argv[2];
    conn_id = atoi(temp);

    btut_gatt_btaddr_stoh(ptr, &bdaddr);
    BTUT_Logi("GATTC disconnect to %02X:%02X:%02X:%02X:%02X:%02X ,\n",
        bdaddr.address[0], bdaddr.address[1], bdaddr.address[2],
        bdaddr.address[3], bdaddr.address[4], bdaddr.address[5]);

    btut_gattc_interface->disconnect(client_if,&bdaddr,conn_id);

    return 0;
}

static int btut_gattc_listen(int argc, char **argv)
{
    BTUT_Logi("[GATTC] %s()\n", __func__);
    /*if (btut_client_if == -1)
    {
        BTUT_Logi("[GATTC] no client app registered now.\n");
        return 0;
    }*/
    int client_if = 0;
    if (argc < 1)
    {
        BTUT_Logi("Usage :\n");
        BTUT_Logi("  GATTC listen client_if \n");
        return -1;
    }
    client_if = atoi(argv[0]);
    btut_gattc_interface->listen(client_if,true);
    return 0;
}

static int btut_gattc_refresh(int argc, char **argv)
{
    BTUT_Logi("[GATTC] %s()\n", __func__);
    /*if (btut_client_if == -1)
    {
        BTUT_Logi("[GATTC] no client app registered now.\n");
        return 0;
    }*/

    bt_bdaddr_t bdaddr;
    char *ptr;
    int client_if = 0;
    if (argc < 2)
    {
        BTUT_Logi("Usage :\n");
        BTUT_Logi("  GATTC refresh client_if BD_ADDR\n");
        return -1;
    }
    client_if = atoi(argv[0]);
    ptr = argv[1];
    btut_gatt_btaddr_stoh(ptr, &bdaddr);
    BTUT_Logi("GATTC refresh %02X:%02X:%02X:%02X:%02X:%02X ,\n",
        bdaddr.address[0], bdaddr.address[1], bdaddr.address[2],
        bdaddr.address[3], bdaddr.address[4], bdaddr.address[5]);
    btut_gattc_interface->refresh(client_if,&bdaddr);
    return 0;
}

static int btut_gattc_search_service(int argc, char **argv)
{
    char *ptr;
    bt_uuid_t uuid;
    int conn_id = 0;

    BTUT_Logi("[GATTC] %s()\n", __func__);
    if (btut_client_if == -1)
    {
        BTUT_Logi("[GATTC] no client app registered now.\n");
        return 0;
    }
    if (argc < 1)
    {
        BTUT_Logi("Usage :\n");
        BTUT_Logi("  GATTC search_service conn_id [uuid]\n");
        return -1;
    }
    if (argc == 2)
    {
        ptr = argv[1];
        btut_gatt_uuid_stoh(ptr, &uuid);
    }

    char *temp = argv[0];
    conn_id = atoi(temp);
    if (argc == 2)
    {
        btut_gattc_interface->search_service(conn_id,&uuid);
    }
    else if(argc == 1)
    {
        btut_gattc_interface->search_service(conn_id, NULL);
    }
    return 0;
}

static int btut_gattc_get_gatt_db(int argc, char **argv)
{
    int conn_id = 0;

    BTUT_Logi("[GATTC] %s()\n", __func__);
    if (btut_client_if == -1)
    {
        BTUT_Logi("[GATTC] no client app registered now.\n");
        return 0;
    }
    if (argc < 1)
    {
        BTUT_Logi("Usage :\n");
        BTUT_Logi("  GATTC get_gatt_db conn_id \n");
        return -1;
    }
    conn_id = atoi(argv[0]);
    btut_gattc_interface->get_gatt_db(conn_id);
    return 0;
}

static int btut_gattc_read_char(int argc, char **argv)
{
    int conn_id = 0;
    int auth_req = 0;
    int char_handle = 0;

    BTUT_Logi("[GATTC] %s()\n", __func__);
    if (btut_client_if == -1)
    {
        BTUT_Logi("[GATTC] no client app registered now.\n");
        return 0;
    }
    if (argc < 2)
    {
        BTUT_Logi("Usage :\n");
        BTUT_Logi("  GATTC read_char conn_id characteristic_handle [auth_req]\n");
        return -1;
    }
    char *temp = argv[0];
    conn_id = atoi(temp);
    temp = argv[1];
    char_handle = atoi(temp);
    if (argc == 3)
    {
        temp = argv[2];
        auth_req = atoi(temp);
    }
    btut_gattc_interface->read_characteristic(conn_id,char_handle,auth_req);
    return 0;
}

static int btut_gattc_read_descr(int argc, char **argv)
{
    int conn_id = 0;
    int auth_req = 0;
    int descr_handle = 0;

    BTUT_Logi("[GATTC] %s()\n", __func__);
    if (btut_client_if == -1)
    {
        BTUT_Logi("[GATTC] no client app registered now.\n");
        return 0;
    }
    if (argc < 2)
    {
        BTUT_Logi("Usage :\n");
        BTUT_Logi("  GATTC read_descr conn_id descr_handle [auth_req]\n");
        return -1;
    }
    char *temp = argv[0];
    conn_id = atoi(temp);
    temp = argv[1];
    descr_handle = atoi(temp);
    if (argc == 3)
    {
        temp = argv[2];
        auth_req = atoi(temp);
    }
    btut_gattc_interface->read_descriptor(conn_id,descr_handle,auth_req);
    return 0;
}

static int btut_gattc_write_char(int argc, char **argv)
{
    int conn_id = 0;
    int auth_req = 0;
    int write_type = 2; //WRITE_TYPE_DEFAULT = 2, WRITE_TYPE_NO_RESPONSE = 1, WRITE_TYPE_SIGNED = 4
    int char_handle = 0;
    char *value;
    BTUT_Logi("[GATTC] %s()\n", __func__);
    if (btut_client_if == -1)
    {
        BTUT_Logd("[GATTC] no client app registered now.\n");
        return 0;
    }
    if (argc < 4)
    {
        BTUT_Logi("Usage :\n");
        BTUT_Logi("  GATTC write_char conn_id char_handle write_type [auth_req] value\n");
        return -1;
    }
    char *temp = argv[0];
    conn_id = atoi(temp);
    temp = argv[1];
    char_handle = atoi(temp);
    temp = argv[2];
    write_type = atoi(temp);
    if (argc == 5)
    {
        temp = argv[3];
        auth_req = atoi(temp);
        value= argv[4];
    }
    else
    {
        value= argv[3];
    }
    btut_gattc_interface->write_characteristic(conn_id,char_handle,write_type,strlen(value),auth_req,value);
    return 0;
}

static int btut_gattc_write_descr(int argc, char **argv)
{
    int conn_id = 0;
    int auth_req = 0;
    int write_type = 2; //WRITE_TYPE_DEFAULT = 2, WRITE_TYPE_NO_RESPONSE = 1, WRITE_TYPE_SIGNED = 4
    int descr_handle = 0;
    char *value;
    BTUT_Logi("[GATTC] %s()\n", __func__);
    if (btut_client_if == -1)
    {
        BTUT_Logd("[GATTC] no client app registered now.\n");
        return 0;
    }
    if (argc < 4)
    {
        BTUT_Logi("Usage :\n");
        BTUT_Logi("  GATTC write_descr conn_id descr_handle write_type [auth_req] value\n");
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

    btut_gattc_interface->write_descriptor(conn_id,descr_handle,write_type,strlen(value),auth_req,value);
    return 0;
}

static int btut_gattc_execute_write(int argc, char **argv)
{
    BTUT_Logi("[GATTC] %s()\n", __func__);
    if (btut_client_if == -1)
    {
        BTUT_Logi("[GATTC] no client app registered now.\n");
        return 0;
    }
    int conn_id = 0;
    int execute = 0;

    if (argc < 2)
    {
        BTUT_Logi("Usage :\n");
        BTUT_Logi("  GATTC execute_write conn_id execute\n");
        return -1;
    }

    char *temp = argv[0];
    conn_id = atoi(temp);
    temp = argv[1];
    execute = atoi(temp);

    btut_gattc_interface->execute_write(conn_id,execute);

    return 0;
}

static int btut_gattc_reg_noti(int argc, char **argv)
{
    int char_handle = 0;
    bt_bdaddr_t bdaddr;
    char *ptr;
    int client_if = 0;
    BTUT_Logi("[GATTC] %s()\n", __func__);
    /*if (btut_client_if == -1)
    {
        BTUT_Logd("[GATTC] no client app registered now.\n");
        return 0;
    }*/
    if (argc < 3)
    {
        BTUT_Logi("Usage :\n");
        BTUT_Logi("  GATTC reg_noti client_if BD_ADDR char_handle\n");
        return -1;
    }
    client_if = atoi(argv[0]);
    ptr = argv[1];
    btut_gatt_btaddr_stoh(ptr, &bdaddr);
    char_handle = atoi(argv[2]);
    btut_gattc_interface->register_for_notification(client_if,&bdaddr,char_handle);
    return 0;
}

static int btut_gattc_dereg_noti(int argc, char **argv)
{
    char *ptr;
    int char_handle = 0;
    bt_bdaddr_t bdaddr;
    int client_if = 0;
    BTUT_Logi("[GATTC] %s()\n", __func__);
    /*if (btut_client_if == -1)
    {
        BTUT_Logd("[GATTC] no client app registered now.\n");
        return 0;
    }*/
    if (argc < 3)
    {
        BTUT_Logi("Usage :\n");
        BTUT_Logi("  GATTC dereg_noti client_if BD_ADDR char_handle\n");
        return -1;
    }
    client_if = atoi(argv[0]);
    ptr = argv[1];
    btut_gatt_btaddr_stoh(ptr, &bdaddr);
    char_handle = atoi(argv[2]);
    btut_gattc_interface->deregister_for_notification(client_if,&bdaddr,char_handle);
    return 0;
}

static int btut_gattc_read_rssi(int argc, char **argv)
{
    BTUT_Logi("[GATTC] %s()\n", __func__);
    /*if (btut_client_if == -1)
    {
        BTUT_Logi("[GATTC] no client app registered now.\n");
        return 0;
    }*/
    char *ptr;
    bt_bdaddr_t bdaddr;
    int client_if = 0;
    if (argc < 2)
    {
        BTUT_Logi("Usage :\n");
        BTUT_Logi("  GATTC read_rssi client_if BD_ADDR \n");
        return -1;
    }
    client_if = atoi(argv[0]);
    ptr = argv[1];
    btut_gatt_btaddr_stoh(ptr, &bdaddr);
    btut_gattc_interface->read_remote_rssi(client_if,&bdaddr);
    return 0;
}

// Scan filter function
//
//#define LE_ACTION_TYPE_ADD         0
//#define LE_ACTION_TYPE_DEL          1
//#define LE_ACTION_TYPE_CLEAR      2

static int btut_gattc_scan_filter_param_setup(int argc, char **argv)
{
    BTUT_Logi("[GATTC] %s()\n", __func__);
    btgatt_filt_param_setup_t scan_filt_param;
    memset(&scan_filt_param, 0, sizeof(btgatt_filt_param_setup_t));
    /*if (btut_client_if == -1)
    {
        BTUT_Logi("[GATTC] Scan : no client app registered now.\n");
        return 0;
    }*/
    if (argc < 2)
    {
         BTUT_Logi("Usage :\n");
         BTUT_Logi("  GATTC sf_param_setup client_if action [filt_index int] [feat_seln int] [list_logic_type hex_string] [filt_logic_type int] [rssi_high_thres int] [rssi_low_thres int] [dely_mode int] [found_timeout int] [lost_timeout int] [found_timeout_cnt int] [num_of_tracking_entries int]\n");
         return -1;
    }
    scan_filt_param.client_if = atoi(argv[0]);
    scan_filt_param.action = atoi(argv[1]);
    int count = 2;
    while (count < argc)
    {
        if (strcmp(argv[count],"filt_index") == 0)
        {
            count++;
            scan_filt_param.filt_index = atoi(argv[count]);
            count++;
            BTUT_Logi("filt_index : %d\n" ,scan_filt_param.filt_index);
            continue;
        }
        else if (strcmp(argv[count],"feat_seln") == 0)
        {
            count++;
            scan_filt_param.feat_seln = atoi(argv[count]);
            count++;
            BTUT_Logi("feat_seln : %d\n" ,scan_filt_param.feat_seln);
            continue;
        }
        else if (strcmp(argv[count],"list_logic_type") == 0)
        {
            count++;
            scan_filt_param.list_logic_type = strtol(argv[count],NULL,16);
            count++;
            BTUT_Logi("list_logic_type : %d\n" ,scan_filt_param.list_logic_type);
            continue;
        }
        else if (strcmp(argv[count],"filt_logic_type") == 0)
        {
            count++;
            scan_filt_param.filt_logic_type = atoi(argv[count]);
            count++;
            BTUT_Logi("filt_logic_type : %d\n" ,scan_filt_param.filt_logic_type);
            continue;
        }
        else if (strcmp(argv[count],"rssi_high_thres") == 0)
        {
            count++;
            scan_filt_param.rssi_high_thres = atoi(argv[count]);
            count++;
            BTUT_Logi("rssi_high_thres : %d\n" ,scan_filt_param.rssi_high_thres);
            continue;
        }
        else if (strcmp(argv[count],"rssi_low_thres") == 0)
        {
            count++;
            scan_filt_param.rssi_low_thres = atoi(argv[count]);
            count++;
            BTUT_Logi("rssi_low_thres : %d\n" ,scan_filt_param.rssi_low_thres);
            continue;
        }
        else if (strcmp(argv[count],"dely_mode") == 0)
        {
            count++;
            scan_filt_param.dely_mode = atoi(argv[count]);
            count++;
            BTUT_Logi("dely_mode : %d\n" ,scan_filt_param.dely_mode);
            continue;
        }
        else if (strcmp(argv[count],"found_timeout") == 0)
        {
            count++;
            scan_filt_param.found_timeout = atoi(argv[count]);
            count++;
            BTUT_Logi("found_timeout : %d\n" ,scan_filt_param.found_timeout);
            continue;
        }
        else if (strcmp(argv[count],"lost_timeout") == 0)
        {
            count++;
            scan_filt_param.lost_timeout = atoi(argv[count]);
            count++;
            BTUT_Logi("lost_timeout : %d\n" ,scan_filt_param.lost_timeout);
            continue;
        }
        else if (strcmp(argv[count],"found_timeout_cnt") == 0)
        {
            count++;
            scan_filt_param.found_timeout_cnt = atoi(argv[count]);
            count++;
            BTUT_Logi("found_timeout_cnt : %d\n" ,scan_filt_param.found_timeout_cnt);
            continue;
        }
        else if (strcmp(argv[count],"num_of_tracking_entries") == 0)
        {
            count++;
            scan_filt_param.num_of_tracking_entries = atoi(argv[count]);
            count++;
            BTUT_Logi("num_of_tracking_entries : %d\n" ,scan_filt_param.num_of_tracking_entries);
            continue;
        }
        count+=2;
    }
    btut_gattc_interface->scan_filter_param_setup(scan_filt_param);
    return 0;
}

static int btut_gattc_scan_filter_enable(int argc, char **argv)
{
    BTUT_Logi("[GATTC] %s()\n", __func__);
    int client_if = 0;
    /*if (btut_client_if == -1)
    {
        BTUT_Logi("[GATTC] Scan : no client app registered now.\n");
        return 0;
    }*/
    if (argc < 1)
    {
        BTUT_Logi("Usage :\n");
        BTUT_Logi("  GATTC scan_filter_enable client_if \n");
        return -1;
    }
    client_if = atoi(argv[0]);
    btut_gattc_interface->scan_filter_enable(client_if, true);
    return 0;
}

static int btut_gattc_scan_filter_disable(int argc, char **argv)
{
    BTUT_Logi("[GATTC] %s()\n", __func__);
    int client_if = 0;
    /*if (btut_client_if == -1)
    {
        BTUT_Logi("[GATTC] Scan : no client app registered now.\n");
        return 0;
    }*/
    if (argc < 1)
    {
        BTUT_Logi("Usage :\n");
        BTUT_Logi("  GATTC scan_filter_disable client_if \n");
        return -1;
    }
    client_if = atoi(argv[0]);
    btut_gattc_interface->scan_filter_enable(client_if, false);
    return 0;
}

static int btut_gattc_scan_filter_add_remove(int argc, char **argv)
{
    BTUT_Logi("[GATTC] %s()\n", __func__);
    int client_if = 0;
    int action = 0;
    int filt_type = 0;
    int filt_index = 0;
    int company_id = 0;
    int company_id_mask = 0;
    bt_uuid_t p_uuid, p_uuid_mask;
    bt_bdaddr_t bd_addr;
    bt_uuid_t *ptr_uuid = NULL;
    bt_uuid_t *ptr_uuid_mask = NULL;
    bt_bdaddr_t *ptr_bd_addr = NULL;
    char addr_type = 0;
    int data_len = 0;
    char* p_data = NULL;
    int mask_len = 0;
    char* p_mask = NULL;
    /*if (btut_client_if == -1)
    {
        BTUT_Logi("[GATTC] Scan : no client app registered now.\n");
        return 0;
    }*/
    if (argc < 2)
    {
         BTUT_Logi("Usage :\n");
         BTUT_Logi("  GATTC scan_filter_add_remove client_if action [filt_index int] [filt_type int] [company_id hex_string] [company_id_mask hex_string] [uuid hex_string] [uuid_mask hex_string] [bd_addr string] [addr_type int] [data hex_string/string] [data_mask hex_string/string]\n");
         return -1;
    }
    client_if = atoi(argv[0]);
    action = atoi(argv[1]);
    int count = 2;
    while (count < argc)
    {
        if (strcmp(argv[count],"filt_index") == 0)
        {
            count++;
            filt_index = atoi(argv[count]);
            count++;
            BTUT_Logi("filt_index : %d\n" ,filt_index);
            continue;
        }
        else if (strcmp(argv[count],"filt_type") == 0)
        {
            count++;
            filt_type = atoi(argv[count]);
            count++;
            BTUT_Logi("filt_type : %d\n" ,filt_type);
            continue;
        }
        else if (strcmp(argv[count],"company_id") == 0)
        {
            count++;
            company_id = strtol(argv[count],NULL,16);
            count++;
            BTUT_Logi("company_id : %d\n" ,company_id);
            continue;
        }
        else if (strcmp(argv[count],"company_id_mask") == 0)
        {
            count++;
            company_id_mask = strtol(argv[count],NULL,16);
            count++;
            BTUT_Logi("company_id_mask : %d\n" ,company_id_mask);
            continue;
        }
        else if (strcmp(argv[count],"uuid") == 0)
        {
            count++;
            char *ptr = argv[count];
            btut_gatt_uuid_stoh(ptr, &(p_uuid));
            ptr_uuid = &p_uuid;
            count++;
            BTUT_Logi("uuid : %s\n" ,ptr);
            continue;
        }
        else if (strcmp(argv[count],"uuid_mask") == 0)
        {
            count++;
            char *ptr = argv[count];
            btut_gatt_uuid_stoh(ptr, &(p_uuid_mask));
            ptr_uuid_mask = &p_uuid_mask;
            count++;
            BTUT_Logi("uuid_mask : %s\n" ,p_uuid_mask);
            continue;
        }
        else if (strcmp(argv[count],"bd_addr") == 0)
        {
            count++;
            char *ptr = argv[count];
            btut_gatt_btaddr_stoh(ptr, &bd_addr);
            ptr_bd_addr = &bd_addr;
            count++;
            BTUT_Logi("bd_addr %02X:%02X:%02X:%02X:%02X:%02X ,\n",
                bd_addr.address[0], bd_addr.address[1], bd_addr.address[2],
                bd_addr.address[3], bd_addr.address[4], bd_addr.address[5]);
            continue;
        }
        else if (strcmp(argv[count],"addr_type") == 0)
        {
            count++;
            addr_type = atoi(argv[count]);
            count++;
            BTUT_Logi("addr_type : %d\n" ,addr_type);
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
                    BTUT_Logi("data : %d\n" ,0);
                    continue;
                    break;
                }
                case 1: // BTM_BLE_PF_SRVC_DATA
                case 5: // BTM_BLE_PF_MANU_DATA
                case 6: // BTM_BLE_PF_SRVC_DATA_PATTERN
                {
                    short hex_len = (strlen(argv[count]) + 1) / 2;
                    char *hex_buf = malloc(hex_len * sizeof(char));
                    ascii_2_hex(argv[count], hex_len, (UINT8 *)hex_buf);
                    p_data = hex_buf;
                    data_len = hex_len;
                    count++;
                    BTUT_Logi("data : %d\n" ,data_len);
                    continue;
                    break;
                }
                case 4: // BTM_BLE_PF_LOCAL_NAME
                {
                    p_data = argv[count];
                    data_len = strlen(argv[count]);
                    count++;
                    BTUT_Logi("data : %d\n" ,data_len);
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
                    BTUT_Logi("data_mask : %d\n" ,0);
                    continue;
                    break;
                }
                case 1: // BTM_BLE_PF_SRVC_DATA
                case 5: // BTM_BLE_PF_MANU_DATA
                case 6: // BTM_BLE_PF_SRVC_DATA_PATTERN
                {
                    short hex_len = (strlen(argv[count]) + 1) / 2;
                    char *hex_buf = malloc(hex_len * sizeof(char));
                    ascii_2_hex(argv[count], hex_len, (UINT8 *)hex_buf);
                    p_mask = hex_buf;
                    mask_len = hex_len;
                    count++;
                    BTUT_Logi("data_mask : %d\n" ,mask_len);
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
                    char *hex_buf = malloc(hex_len * sizeof(char));
                    ascii_2_hex(argv[count], hex_len, (UINT8 *)hex_buf);
                    p_mask = hex_buf;
                    mask_len = hex_len;
                    count++;
                    BTUT_Logi("data_mask : %d\n" ,mask_len);
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
    btut_gattc_interface->scan_filter_add_remove(client_if,action,filt_type,
                               filt_index,company_id,company_id_mask,ptr_uuid,
                               ptr_uuid_mask,ptr_bd_addr,addr_type,data_len,p_data,mask_len,p_mask);
    return 0;
}

static int btut_gattc_scan_filter_clear(int argc, char **argv)
{
    BTUT_Logi("[GATTC] %s()\n", __func__);
    int filt_index = 0;
    int client_if = 0;
    /*if (btut_client_if == -1)
    {
        BTUT_Logi("[GATTC] Scan : no client app registered now.\n");
        return 0;
    }*/
    if (argc < 3)
    {
         BTUT_Logi("Usage :\n");
         BTUT_Logi("  GATTC scan_filter_clear client_if filt_index\n");
         return -1;
    }
    client_if = atoi(argv[0]);
    filt_index = atoi(argv[1]);
    btut_gattc_interface->scan_filter_clear(client_if,filt_index);
    return 0;
}

// Parameters function
static int btut_gattc_get_device_type(int argc, char **argv)
{
    BTUT_Logi("[GATTC] %s()\n", __func__);
    return 0;
}

static int btut_gattc_set_adv_data(int argc, char **argv)
{
    BTUT_Logi("[GATTC] %s()\n", __func__);
    /*if (btut_client_if == -1)
    {
        BTUT_Logi("[GATTC] no client app registered now.\n");
        return 0;
    }*/
    if (argc < 1)
    {
         BTUT_Logi("Usage :\n");
         BTUT_Logi("  GATTC set_adv_data client_if [set_scan_rsp true|false] [include_name true|false] [incl_txpower true|false] [min_interval int] [max_interval int] [appearance int] [manufacturer_data hex_string] [service_data hex_string] [service_uuid hex_string]\n");
         return -1;
    }
    int client_if = 0;
    int count = 1;
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
    int min_interval = 0;
    int max_interval = 0;
    client_if = atoi(argv[0]);
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
            BTUT_Logi("set_scan_rsp : %d\n" ,set_scan_rsp);
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
            BTUT_Logi("include_name : %d\n" ,include_name);
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
            BTUT_Logi("incl_txpower : %d\n" ,incl_txpower);
            continue;
        }
        else if (strcmp(argv[count],"min_interval") == 0)
        {
            count++;
            min_interval = (atoi(argv[count]))*1000/625;
            count++;
            BTUT_Logi("min_interval : %d\n" ,min_interval);
            continue;
        }
        else if (strcmp(argv[count],"max_interval") == 0)
        {
            count++;
            max_interval = (atoi(argv[count]))*1000/625;
            count++;
            BTUT_Logi("max_interval : %d\n" ,max_interval);
            continue;
        }
        else if (strcmp(argv[count],"appearance") == 0)
        {
            count++;
            appearance = atoi(argv[count]);
            count++;
            BTUT_Logi("appearance : %d\n" ,appearance);
            continue;
        }
        else if (strcmp(argv[count],"manufacturer_data") == 0)
        {
            count++;
            short hex_len = (strlen(argv[count]) + 1) / 2;
            char *hex_buf = malloc(hex_len * sizeof(char));
            ascii_2_hex(argv[count], hex_len, (UINT8 *)hex_buf);
            manufacturer_data = hex_buf;
            manufacturer_len = hex_len;
            count++;
            BTUT_Logi("manufacturer_len : %d\n" ,manufacturer_len);
            continue;
        }
        else if (strcmp(argv[count],"service_data") == 0)
        {
            count++;
            short hex_len = (strlen(argv[count]) + 1) / 2;
            char *hex_buf = malloc(hex_len * sizeof(char));
            ascii_2_hex(argv[count], hex_len, (UINT8 *)hex_buf);
            service_data = hex_buf;
            service_data_len = hex_len;
            count++;
            BTUT_Logi("service_data_len : %d\n" ,service_data_len);
            continue;
        }
        else if (strcmp(argv[count],"service_uuid") == 0)
        {
            count++;
            short hex_len = (strlen(argv[count]) + 1) / 2;
            char *hex_buf = malloc(hex_len * sizeof(char));
            ascii_2_hex(argv[count], hex_len, (UINT8 *)hex_buf);
            service_uuid = hex_buf;
            service_uuid_len = hex_len;
            count++;
            BTUT_Logi("service_uuid_len : %d\n" ,service_uuid_len);
            continue;
        }
        count+=2;
    }
    btut_gattc_interface->set_adv_data(client_if,set_scan_rsp, include_name, incl_txpower,min_interval,max_interval,appearance
        ,manufacturer_len, manufacturer_data,service_data_len, service_data,service_uuid_len, service_uuid);
    return 0;
}

static int btut_gattc_configure_mtu(int argc, char **argv)
{
    BTUT_Logi("[GATTC] %s()\n", __func__);
    int conn_id = 0;
    int mtu = 0;
    if (argc < 2)
    {
        BTUT_Logi("Usage :\n");
        BTUT_Logi("  please input GATTC configure_mtu conn_id mtu\n");
        return -1;
    }
    conn_id = atoi(argv[0]);
    mtu = atoi(argv[1]);
    if ((mtu < 23) || (mtu > 517))
    {
        BTUT_Logi("[GATTC] invalid mtu size %d.\n", mtu);
        return -1;
    }
    btut_gattc_interface->configure_mtu(conn_id, mtu);
    return 0;
}

static int btut_gattc_conn_parameter_update(int argc, char **argv)
{
    BTUT_Logi("[GATTC] %s()\n", __func__);
    if (argc < 3)
    {
        BTUT_Logi("Usage :\n");
        BTUT_Logi("  GATTC conn_parameter_update bd_addr min_interval max_interval [latency] [timeout]\n");
        return -1;
    }
    int min_interval = 0;
    int max_interval = 0;
    int latency = 0;
    int timeout = 0;
    bt_bdaddr_t bdaddr;
    latency = 0;
    timeout = 2000;
    char *ptr;
    ptr = argv[0];
    btut_gatt_btaddr_stoh(ptr, &bdaddr);
    char *temp = argv[1];
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
    btut_gattc_interface->conn_parameter_update(&bdaddr, min_interval,
                    max_interval,latency,timeout);
    return 0;
}

static int btut_gattc_set_scan_parameters(int argc, char **argv)
{
    BTUT_Logi("[GATTC] %s()\n", __func__);
    if (argc < 3)
    {
        BTUT_Logi("Usage :\n");
        BTUT_Logi("  GATTC set_scan_parameters client_if scan_interval scan_window\n");
        return -1;
    }
    int client_if = 0;
    int scan_interval = 0;
    int scan_window = 0;
    client_if = atoi(argv[0]);
    char *temp = argv[1];
    scan_interval = (atoi(temp))*1000/625;
    temp = argv[2];
    scan_window = (atoi(temp))*1000/625;
    btut_gattc_interface->set_scan_parameters(client_if, scan_interval,scan_window);
    return 0;
}

// Multiple advertising function
static int btut_gattc_multi_adv_enable(int argc, char **argv)
{
    BTUT_Logi("[GATTC] %s()\n", __func__);
    /*if (btut_client_if == -1)
    {
        BTUT_Logi("[GATTC] no client app registered now.\n");
        return 0;
    }*/

    if (argc < 6)
    {
        BTUT_Logi("Usage :\n");
        BTUT_Logi("  GATTC adv_enable client_if min_interval max_interval adv_type tx_power timeout \n");
        return -1;
    }
    int client_if = 0;
    int min_interval = 0;
    int max_interval = 0;
    int adv_type = 0;
    int tx_power = 0;
    int timeout= 0;
    char *temp = argv[0];
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
    BTUT_Logi("min_int=%u, max_int=%u, adv_type=%u, chnl_map=%u, tx_pwr=%u",min_interval,max_interval,adv_type,ADVERTISING_CHANNEL_ALL,tx_power);
    //btut_gattc_interface->set_adv_data(btut_client_if,false,true,false,min_interval,max_interval,0,0,NULL,0,NULL,0,NULL);
    btut_gattc_interface->multi_adv_enable(client_if,min_interval, max_interval, adv_type,ADVERTISING_CHANNEL_ALL ,tx_power, timeout);
    return 0;
}

static int btut_gattc_multi_adv_update(int argc, char **argv)
{
    BTUT_Logi("[GATTC] %s()\n", __func__);
    /*if (btut_client_if == -1)
    {
        BTUT_Logi("[GATTC] no client app registered now.\n");
        return 0;
    } */

    if (argc < 6)
    {
        BTUT_Logi("Usage :\n");
        BTUT_Logi("  GATTC adv_update client_if min_interval max_interval adv_type tx_power timeout \n");
        return -1;
    }

    int client_if = 0;
    int min_interval = 0;
    int max_interval = 0;
    int adv_type = 0;
    int tx_power = 0;
    int timeout = 0;
    char *temp = argv[0];
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
    btut_gattc_interface->multi_adv_update(client_if,min_interval, max_interval, adv_type,ADVERTISING_CHANNEL_ALL ,tx_power, timeout);
    return 0;
}

static int btut_gattc_multi_adv_setdata(int argc, char **argv)
{
    BTUT_Logi("[GATTC] %s()\n", __func__);
    /*if (btut_client_if == -1)
    {
        BTUT_Logi("[GATTC] no client app registered now.\n");
        return 0;
    }*/
    if (argc < 1)
    {
         BTUT_Logi("Usage :\n");
         BTUT_Logi("  GATTC adv_update_data client_if [set_scan_rsp true|false] [include_name true|false] [incl_txpower true|false] [appearance int] [manufacturer_data hex_string] [service_data hex_string] [service_uuid hex_string]\n");
         return -1;
    }
    int client_if = 0;
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
    char *temp = argv[0];
    client_if = atoi(temp);
    int le_name_len = 0;
    char* le_name = NULL;

    count++;
    while (count < argc)
    {
        BTUT_Logi("[GATTC] %s()\n", argv[count]);
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
            short hex_len = (strlen(argv[count]) + 1) / 2;
            char *hex_buf = malloc(hex_len * sizeof(char));
            ascii_2_hex(argv[count], hex_len, (UINT8 *)hex_buf);
            manufacturer_data = hex_buf;
            manufacturer_len = hex_len;
            count++;
            BTUT_Logi("manufacturer_len : %d\n" ,manufacturer_len);
            continue;
        }
        else if (strcmp(argv[count],"service_data") == 0)
        {
            count++;
            short hex_len = (strlen(argv[count]) + 1) / 2;
            char *hex_buf = malloc(hex_len * sizeof(char));
            ascii_2_hex(argv[count], hex_len, (UINT8 *)hex_buf);
            service_data = hex_buf;
            service_data_len = hex_len;
            count++;
            BTUT_Logi("service_data_len : %d\n" ,service_data_len);
            continue;
        }
        else if (strcmp(argv[count],"service_uuid") == 0)
        {
            count++;
            short hex_len = (strlen(argv[count]) + 1) / 2;
            char *hex_buf = malloc(hex_len * sizeof(char));
            ascii_2_hex(argv[count], hex_len, (UINT8 *)hex_buf);
            service_uuid = hex_buf;
            service_uuid_len = hex_len;
            count++;
            BTUT_Logi("service_uuid_len : %d\n" ,service_uuid_len);
            continue;
        }
        else if (strcmp(argv[count],"le_name") == 0)
        {
            count++;
            le_name_len = strlen(argv[count])+1;
            le_name = malloc(le_name_len * sizeof(char));
            memcpy(le_name, argv[count], le_name_len);
            count++;
            BTUT_Logi("len = %d,le_name : %s\n",le_name_len ,le_name);
            continue;
        }
        count+=2;
    }
    btut_gattc_interface->multi_adv_set_inst_data(client_if,set_scan_rsp, include_name, incl_txpower,appearance
        ,manufacturer_len, manufacturer_data,service_data_len, service_data,service_uuid_len, service_uuid);
    return 0;
}

static int btut_gattc_multi_adv_disable(int argc, char **argv)
{
    BTUT_Logi("[GATTC] %s()\n", __func__);
    /*if (btut_client_if == -1)
    {
        BTUT_Logi("[GATTC] no client app registered now.\n");
        return 0;
    } */
    if (argc < 1)
    {
        BTUT_Logi("Usage :\n");
        BTUT_Logi("    GATTC adv_disable client_if\n");
        return -1;
    }
    int client_if = 0;
    char *temp = argv[0];
    client_if = atoi(temp);
    btut_gattc_interface->multi_adv_disable(client_if);
    return 0;
}

#if 0
// Batch scan function
static int btut_gattc_batchscan_cfg_storage(int argc, char **argv)
{
    BTUT_Logi("[GATTC] %s()\n", __func__);
    return 0;
}

static int btut_gattc_batchscan_enb_batch_scan(int argc, char **argv)
{
    BTUT_Logi("[GATTC] %s()\n", __func__);
    return 0;
}

static int btut_gattc_batchscan_dis_batch_scan(int argc, char **argv)
{
    BTUT_Logi("[GATTC] %s()\n", __func__);
    return 0;
}

static int btut_gattc_batchscan_read_reports(int argc, char **argv)
{
    BTUT_Logi("[GATTC] %s()\n", __func__);
    return 0;
}
#endif

static int btut_gattc_set_local_le_name(int argc, char **argv)
{
#if MTK_LINUX_GATTC_LE_NAME == TRUE
    int client_if = 0;
    char *name;

    if (argc < 2)
    {
        BTUT_Logi("Usage :\n");
        BTUT_Logi("    GATTC set_le_name client_if name\n");
        return -1;
    }
    client_if = atoi(argv[0]);
    name = argv[1];
    btut_gattc_interface->set_local_le_name(client_if,name);
#endif
    return 0;
}

static BTUT_CLI bt_gattc_cli_commands[] =
{
    {"register_app",            btut_gattc_register_app,            " = register_app uuid"},
    {"unregister_app",          btut_gattc_unregister_app,          " = unregister_app"},
    {"scan",                    btut_gattc_scan,                    " = scan [le_scan_type 1: Active 0:Passive scan]"},
    {"stop_scan",               btut_gattc_stop_scan,               " = stop_scan"},
    {"open",                    btut_gattc_open,                    " = open client_if BD_ADDR [isDirect:true|false] [transport]"},
    {"close",                   btut_gattc_close,                   " = close client_if BD_ADDR conn_id"},
    {"listen",                  btut_gattc_listen,                  " = listen client_if"},
    {"refresh",                 btut_gattc_refresh,                 " = refresh client_if BD_ADDR"},
    {"search_service",          btut_gattc_search_service,          " = search_service conn_id [uuid]"},
    {"get_gatt_db",             btut_gattc_get_gatt_db,             " = get_gatt_db conn_id"},
    {"read_char",               btut_gattc_read_char,               " = read_char conn_id characteristic_handle [auth_req]"},
    {"read_descr",              btut_gattc_read_descr,              " = read_descr conn_id descr_handle [auth_req]"},
    {"write_char",              btut_gattc_write_char,              " = write_char conn_id char_handle write_type [auth_req] value"},
    {"write_descr",             btut_gattc_write_descr,             " = write_descr conn_id descr_handle write_type [auth_req] value"},
    {"execute_write",           btut_gattc_execute_write,           " = execute_write conn_id execute"},
    {"reg_noti",                btut_gattc_reg_noti,                " = reg_noti client_if BD_ADDR char_handle"},
    {"dereg_noti",              btut_gattc_dereg_noti,              " = dereg_noti client_if BD_ADDR char_handle"},
    {"read_rssi",               btut_gattc_read_rssi,               " = read_rssi client_if BD_ADDR"},
    {"scan_filter_param_setup", btut_gattc_scan_filter_param_setup, " = scan_filter_param_setup"},
    {"scan_filter_enable",      btut_gattc_scan_filter_enable,      " = scan_filter_enable"},
    {"scan_filter_disable",     btut_gattc_scan_filter_disable,     " = scan_filter_disable"},
    {"scan_filter_add_remove",  btut_gattc_scan_filter_add_remove,  " = scan_filter_add_remove"},
    {"scan_filter_clear",       btut_gattc_scan_filter_clear,       " = scan_filter_clear"},
    {"get_device_type",         btut_gattc_get_device_type,         " = get_device_type"},
    {"set_adv_data",            btut_gattc_set_adv_data,            " = set_adv_data client_if [set_scan_rsp true|false] [include_name true|false] [incl_txpower true|false] [min_interval int] [max_interval int] [appearance int] [manufacturer_data hex_string] [service_data hex_string] [service_uuid hex_string]"},
    {"configure_mtu",           btut_gattc_configure_mtu,           " = configure_mtu conn_id mtu"},
    {"conn_parameter_update",   btut_gattc_conn_parameter_update,   " = conn_parameter_update bd_addr min_interval max_interval [latency] [timeout]"},
    {"set_scan_parameters",     btut_gattc_set_scan_parameters,     " = set_scan_parameters client_if scan_interval scan_window"},
    {"adv_enable",              btut_gattc_multi_adv_enable,        " = adv_enable client_if min_interval max_interval adv_type tx_power timeout"},
    {"adv_update",              btut_gattc_multi_adv_update,        " = adv_update client_ifmin_interval max_interval adv_type tx_power timeout"},
    {"adv_update_data",         btut_gattc_multi_adv_setdata,       " = adv_update_data client_if [set_scan_rsp true|false] [include_name true|false] [incl_txpower true|false] [appearance int] [manufacturer_data hex_string] [service_data hex_string] [service_uuid hex_string]"},
    {"adv_disable",             btut_gattc_multi_adv_disable,       " = adv_disable client_if"},
    {"set_le_name",             btut_gattc_set_local_le_name,       " = set_le_name client_if name"},
    {"PTS_disc_prim_services",                            PTS_discovery_all_primary_services,               " = PTS_disc_prim_services connid start_handle end_handle "},
    {"PTS_disc_primary_service_by_uuid",                  PTS_discovery_primary_service_by_uuid,            " = PTS_disc_primary_service_by_uuid connid start_handle end_handle "},
    {"PTS_find_include_services",                         PTS_find_include_services,                        " = PTS_find_include_services connid start_handle end_handle "},
    {"PTS_disc_all_characteristics_of_service",           PTS_discover_all_characteristics_of_service,      " = PTS_disc_all_characteristics_of_service connid start_handle end_handle "},
    {"PTS_disc_all_characteristics_by_uuid",              PTS_discover_all_characteristics_by_uuid,         " = PTS_disc_all_characteristics_by_uuid connid start_handle end_handle "},
    {"PTS_disc_all_characteristic_descriptors",           PTS_discover_all_characteristic_descriptors,      " = PTS_disc_all_characteristic_descriptors connid handle end_handle "},
    {"PTS_read_characteristic_value_by_handle",           PTS_read_characteristic_value_by_handle,          " = PTS_read_characteristic_value_by_handle connid handle end_handle "},
    {"PTS_read_using_characteristic_uuid",                PTS_read_using_characteristic_uuid,               " = PTS_read_using_characteristic_uuid connid handle end_handle "},
    {"PTS_read_long_charactetistic_value_by_handle",      PTS_read_long_charactetistic_value_by_handle,     " = PTS_read_long_charactetistic_value_by_handle connid handle1 handle2 "},
    {"PTS_read_characteristic_descriptor_by_handle",      PTS_read_characteristic_descriptor_by_handle,     " = PTS_read_characteristic_descriptor_by_handle connid handle1 end_handle2 "},
    {"PTS_read_long_characteristic_descriptor_by_handle", PTS_read_long_characteristic_descriptor_by_handle," = PTS_read_long_characteristic_descriptor_by_handle connid start_handle end_handle "},
    {"PTS_read_by_type",                                  PTS_read_by_type,                                 " = PTS_read_by_type connid handle end_handle "},
    {"PTS_write_characteristics_value_by_handle",         PTS_write_characteristics_value_by_handle,        " = PTS_write_characteristics_value_by_handle connid handle value size "},
    {"PTS_write_characteristic_descriptor_by_handle",     PTS_write_characteristic_descriptor_by_handle,    " = PTS_write_characteristic_descriptor_by_handle connid handle value size "},
    {"PTS_write_without_response_by_handle",              PTS_write_without_response_by_handle,             " = PTS_write_without_response_by_handle connid handle value size "},
    {"PTS_signed_write_without_response_by_handle",       PTS_signed_write_without_response_by_handle,      " = PTS_signed_write_without_response_by_handle connid start_handle value "},
    /*{"batchscan_cfg_storage",   btut_gattc_batchscan_cfg_storage,   " = batchscan_cfg_storage"},
    {"batchscan_enb_batch_scan", btut_gattc_batchscan_enb_batch_scan," = batchscan_enb_batch_scan"},
    {"batchscan_dis_batch_scan", btut_gattc_batchscan_dis_batch_scan," = batchscan_dis_batch_scan"},
    {"batchscan_read_reports",  btut_gattc_batchscan_read_reports,  " = batchscan_read_reports"},*/
    {NULL, NULL, NULL},
};

static void btut_gattc_register_client_callback(int status, int client_if, bt_uuid_t *app_uuid)
{
    BTUT_Logd("[GATTC] %s()\n", __func__);
    BTUT_Logi("[GATTC] Register client callback :'%d' client_if = %d\n", status ,client_if);
    if (status == 0)
    {
        btut_client_if = client_if;
    }
}

/**
 * FUNCTION NAME: bluetooth_gatt_parse_adv_data
 * PURPOSE:
 *      The function is used for gatt to parse advertising data.
 * INPUT:
 *      adv_data               -- advertising data
 * OUTPUT:
 *      None
 * RETURN:
 *      None
 * NOTES:
 *      None
 */
static void btut_gattc_parse_adv_data (UINT8* adv_data)
{
    UINT8* ptr = adv_data;
    char count = 0;
    char uuid[37];
    char manu_spec_data[128];
    if (NULL == adv_data)
    {
        BTUT_Logd("[GATT] null pointer\n");
        return;
    }
    while (1)
    {
        char length = *ptr;
        if (length == 0)
        {
            break;
        }
        UINT8 type = *(ptr+1);
        UINT8 value_len = length;
        UINT8* value = (UINT8*)malloc(value_len);
        if (NULL == value)
        {
            return;
        }
        memset(value,0,value_len);
        memcpy(value, ptr+2, value_len-1);
        value[value_len-1] = '\0';

        ptr = ptr+length+1;
        switch (type)
        {
            case 0x01: //Flags
                BTUT_Logd("[GATTC] Flags : %02X\n",value[0]);
                break;
            case 0x02: //16-bit uuid
            case 0x03:  //16-bit uuid
            {
                char temp[value_len*2+1];
                INT32 i = 0;
                INT32 j = 0;
                for (i = value_len-1 ; i >= 0 ; i--)
                {
                    snprintf(&temp[j*2],sizeof(temp)-2*j,"%02X",value[i]);
                    j++;
                }
                BTUT_Logd("[GATTC] 16-bit Service Class length: %d UUIDs : %s\n",value_len,temp);
                break;
            }
            case 0x04: //32-bit uuid
            case 0x05:  //32-bit uuid
            {
                char temp[value_len*2+1];
                INT32 i = 0;
                INT32 j = 0;
                for (i = value_len-1 ; i >= 0 ; i--)
                {
                    snprintf(&temp[j*2],sizeof(temp)-2*j,"%02X",value[i]);
                    j++;
                }
                BTUT_Logd("[GATTC] 32-bit Service Class length: %d UUIDs : %s\n",value_len,temp);
                break;
            }
            case 0x06: //128-bit uuid
            case 0x07:  //128-bit uuid
            {
                char temp[value_len*2+1];
                INT32 i = 0;
                INT32 j = 0;
                memset(temp,0,sizeof(temp));
                for (i = value_len-1 ; i >= 0 ; i--)
                {
                    snprintf(&temp[j*2],sizeof(temp)-2*j,"%02X",value[i]);
                    j++;
                }
                BTUT_Logd("[GATTC] 128-bit Service Class length: %d UUIDs : %s\n",value_len,temp);
                if (value_len > sizeof(uuid))
                {
                    BTUT_Logd("[GATTC] value_len:%d invalid\n",value_len);
                    //memcpy(g_scan_rst_info.uuid, temp, sizeof(g_scan_rst_info.uuid));
                    j = 0;
                    for (i = sizeof(uuid)-1 ; i >= 0 ; i--)
                    {
                        uuid[j] = value[i];
                        j++;
                    }
                }
                else
                {
                    //memcpy(g_scan_rst_info.uuid, temp, value_len);
                    j = 0;
                    for (i = value_len-1 ; i >= 0 ; i--)
                    {
                        uuid[j] = value[i];
                        j++;
                    }
                }
                break;
            }
            case 0x08: //Shortened Local Name
                BTUT_Logd("[GATTC] Shortened Local length: %d Name : %s\n",value_len,value);
                break;
            case 0x09: //Complete Local Name
                BTUT_Logd("[GATTC] Complete Local length: %d Name : %s\n",value_len,value);
                break;
            case 0x0A: //Tx Power Level
                BTUT_Logd("[GATTC] Tx Power Level : %d\n",value[0]);
                break;
            case 0x1B:  //LE Bluetooth Device Address
            {
                BTUT_Logd("[GATTC] LE Bluetooth Device Address : %02X:%02X:%02X:%02X:%02X:%02X\n",
                            value[5], value[4], value[3],
                            value[2], value[1], value[0]);
                break;
            }
            case 0xFF:  //Manufacturer Specific Data
            {
                char temp[value_len*2+1];
                INT32 i = 0;
                INT32 j = 0;
                for (i = value_len-1 ; i >= 0 ; i--)
                {
                    snprintf(&temp[j*2],sizeof(temp)-2*j,"%02X",value[i]);
                    j++;
                }
                BTUT_Logd("[GATTC] Manufacturer Specific Data : %s\n",temp);
                strncpy(manu_spec_data, temp, sizeof(manu_spec_data));
                break;
            }
            default:
            {
                char temp[length*2];
                INT32 i = 0;
                for (i = 0 ; i < length ; i++)
                {
                    snprintf(&temp[i*2],sizeof(temp)-2*i,"%02X",value[i]);
                }
                BTUT_Logd("[GATTC] Type:%02X Length:%d Data:%s\n",type,length,temp);
                break;
            }
        }
        free(value);
        count++;
    }
}

static void btut_gattc_scan_result_callback(bt_bdaddr_t* bda, int rssi, uint8_t* adv_data)
{
    BTUT_Logd("[GATTC] %s()\n", __func__);
    BTUT_Logi("Find %02X:%02X:%02X:%02X:%02X:%02X\n , rssi : %d\n",
        bda->address[0], bda->address[1], bda->address[2],
        bda->address[3], bda->address[4], bda->address[5],rssi);
    btut_gattc_parse_adv_data(adv_data);
}

static void btut_gattc_connect_callback(int conn_id, int status, int client_if, bt_bdaddr_t* bda)
{
    BTUT_Logd("[GATTC] %s()\n", __func__);
    BTUT_Logi("%02X:%02X:%02X:%02X:%02X:%02X\n connected, conn_id = %d , status = %d\n",
        bda->address[0], bda->address[1], bda->address[2],
        bda->address[3], bda->address[4], bda->address[5],conn_id,status);
}

static void btut_gattc_disconnect_callback(int conn_id, int status, int client_if, bt_bdaddr_t* bda)
{
    BTUT_Logd("[GATTC] %s()\n", __func__);
    BTUT_Logi("%02X:%02X:%02X:%02X:%02X:%02X\n disconnected, conn_id = %d , status = %d\n",
        bda->address[0], bda->address[1], bda->address[2],
        bda->address[3], bda->address[4], bda->address[5],conn_id,status);
}

static void btut_gattc_search_complete_callback(int conn_id, int status)
{
    BTUT_Logd("[GATTC] %s()\n", __func__);
    BTUT_Logi("search complete status = %d\n",status);
}
#if 0
static void btut_gattc_search_result_callback( int conn_id, btgatt_srvc_id_t *srvc_id)
{
    BTUT_Logd("[GATTC] %s()\n", __func__);
    bt_uuid_t uuid = srvc_id->id.uuid;
    char uuid_s[37];
    btut_gatt_print_uuid(&uuid,uuid_s);

    BTUT_Logi("find service uuid:%s isPrimary = %d\n",uuid_s,srvc_id->is_primary);
}
#endif
static void btut_gattc_register_for_notification_callback(int conn_id, int registered, int status, uint16_t handle)
{
    BTUT_Logd("[GATTC] %s()\n", __func__);

    BTUT_Logi("register char handle:%d, status = %d, registered = %d\n",handle,status,registered);
}

static void btut_gattc_notify_callback(int conn_id, btgatt_notify_params_t *p_data)
{
    BTUT_Logd("[GATTC] %s()\n", __func__);
    BTUT_Logi("notify callback: conn_id = %d, char handle = %d, notify = %d, value len = %d, value = %s\n",
                conn_id, p_data->handle, p_data->is_notify, p_data->len, p_data->value);
}

static void btut_gattc_read_characteristic_callback(int conn_id, int status, btgatt_read_params_t *p_data)
{
    BTUT_Logd("[GATTC] %s()\n", __func__);
    BTUT_Logi("handle = %d\n", p_data->handle);
    BTUT_Logi("status = %d\n", p_data->status);
    BTUT_Logi("value_type = %d\n", p_data->value_type);
    BTUT_Logi("value len = %d\n", p_data->value.len);
    BTUT_Logi("value = %s\n", p_data->value.value);
}

static void btut_gattc_write_characteristic_callback(int conn_id, int status, uint16_t handle)
{
    BTUT_Logd("[GATTC] %s()\n", __func__);
    BTUT_Logi("handle = %d\n", handle);
}

static void btut_gattc_read_descriptor_callback(int conn_id, int status, btgatt_read_params_t *p_data)
{
    BTUT_Logd("[GATTC] %s()\n", __func__);
    BTUT_Logi("handle = %d\n", p_data->handle);
    BTUT_Logi("status = %d\n", p_data->status);
    BTUT_Logi("value_type = %d\n", p_data->value_type);
    BTUT_Logi("value len = %d\n", p_data->value.len);
    BTUT_Logi("value = %s\n", p_data->value.value);
}

static void btut_gattc_write_descriptor_callback(int conn_id, int status, uint16_t handle)
{
    BTUT_Logd("[GATTC] %s()\n", __func__);
    BTUT_Logi("handle = %d\n", handle);
}

static void btut_gattc_execute_write_callback(int conn_id, int status)
{
    BTUT_Logd("[GATTC] %s()\n", __func__);
    BTUT_Logi("execute write status = %d\n",status);
}

static void btut_gattc_read_remote_rssi_callback(int client_if, bt_bdaddr_t* bda, int rssi, int status)
{
    BTUT_Logd("[GATTC] %s()\n", __func__);
    BTUT_Logi("read %02X:%02X:%02X:%02X:%02X:%02X\n rssi = %d , status = %d\n",
        bda->address[0], bda->address[1], bda->address[2],
        bda->address[3], bda->address[4], bda->address[5],rssi,status);
}

static void btut_gattc_listen_callback(int status, int server_if)
{
    BTUT_Logd("[GATTC] %s()\n", __func__);
}

static void btut_gattc_configure_mtu_callback(int conn_id, int status, int mtu)
{
    BTUT_Logd("[GATTC] %s()\n", __func__);
    BTUT_Logi("configure mtu = %d, status = %d\n",mtu,status);
}

static void btut_gattc_scan_filter_cfg_callback(int action, int client_if, int status, int filt_type, int avbl_space)
{
    BTUT_Logd("[GATTC] %s()\n", __FUNCTION__);
    BTUT_Logi("scan_filter_cfg action = %d, client_if = %d, filt_type = %d, status = %d, avbl_space = %d\n",
                action,client_if,filt_type,status,avbl_space);
}

static void btut_gattc_scan_filter_param_callback(int action, int client_if, int status, int avbl_space)
{
    BTUT_Logd("[GATTC] %s()\n", __FUNCTION__);
    BTUT_Logi("scan_filter_param action = %d, status = %d, avbl_space = %d\n",action,status,avbl_space);
}

static void btut_gattc_scan_filter_status_callback(int enable, int client_if, int status)
{
    BTUT_Logd("[GATTC] %s()\n", __FUNCTION__);
    BTUT_Logi("scan_filter_status %d ,status = %d\n",enable,status);
}

static void btut_gattc_multi_adv_enable_callback(int client_if, int status)
{
    BTUT_Logd("[GATTC] %s()\n", __func__);
    BTUT_Logi("status = %d\n",status);
}

static void btut_gattc_multi_adv_update_callback(int client_if, int status)
{
    BTUT_Logd("[GATTC] %s()\n", __func__);
    BTUT_Logi("status = %d\n",status);
}

static void btut_gattc_multi_adv_data_callback(int client_if, int status)
{
    BTUT_Logd("[GATTC] %s()\n", __func__);
    BTUT_Logi("status = %d\n",status);
}

static void btut_gattc_multi_adv_disable_callback(int client_if, int status)
{
    BTUT_Logd("[GATTC] %s()\n", __func__);
    BTUT_Logi("status = %d\n",status);
}

static void btut_gattc_congestion_callback(int conn_id, bool congested)
{

}

static void btut_gattc_batchscan_cfg_storage_callback(int client_if, int status)
{

}

static void btut_gattc_batchscan_enable_disable_callback(int action, int client_if, int status)
{

}

static void btut_gattc_batchscan_reports_callback(int client_if, int status, int report_format,
                                           int num_records, int data_len, uint8_t* rep_data)
{

}

static void btut_gattc_batchscan_threshold_callback(int client_if)
{

}

static void btut_gattc_track_adv_event_callback(btgatt_track_adv_info_t *p_track_adv_info)
{

}

static void btut_gattc_scan_parameter_setup_completed_callback(int client_if,
        btgattc_error_t status)
{
    BTUT_Logd("client_if = %d, status = %d\n", client_if, status);

}

static void btut_gattc_get_gatt_db_callback(int conn_id, btgatt_db_element_t *db, int count)
{
    int i = 0;
    char uuid[37] = {0};
    for (i = 0; i < count; i++)
    {
        memset(uuid, 0, 37);
        btut_gatt_print_uuid(&(db->uuid), uuid);

        BTUT_Logd("type = %d, attribute_handle = %d\n",db->type, db->attribute_handle);
        BTUT_Logd("start_handle = %d, end_handle = %d\n",db->start_handle, db->end_handle);
        BTUT_Logd("id = %d, properties = %d\n",db->id, db->properties);
        BTUT_Logd("uuid = %s\n",uuid);
        BTUT_Logd("\n\n");
        db++;
    }
}

static void btut_gattc_services_removed_callback(int conn_id, uint16_t start_handle, uint16_t end_handle)
{

}

static void btut_gattc_services_added_callback(int conn_id, btgatt_db_element_t *added, int added_count)
{

}

const btgatt_client_callbacks_t btut_gattc_callbacks =
{
    btut_gattc_register_client_callback,
    btut_gattc_scan_result_callback,
    btut_gattc_connect_callback,
    btut_gattc_disconnect_callback,
    btut_gattc_search_complete_callback,
#if 0
    btut_gattc_search_result_callback,
#endif
    btut_gattc_register_for_notification_callback,
    btut_gattc_notify_callback,
    btut_gattc_read_characteristic_callback,
    btut_gattc_write_characteristic_callback,
    btut_gattc_read_descriptor_callback,
    btut_gattc_write_descriptor_callback,
    btut_gattc_execute_write_callback,
    btut_gattc_read_remote_rssi_callback,
    btut_gattc_listen_callback,
    btut_gattc_configure_mtu_callback,
    btut_gattc_scan_filter_cfg_callback,
    btut_gattc_scan_filter_param_callback,
    btut_gattc_scan_filter_status_callback,
    btut_gattc_multi_adv_enable_callback,
    btut_gattc_multi_adv_update_callback,
    btut_gattc_multi_adv_data_callback,
    btut_gattc_multi_adv_disable_callback,
    btut_gattc_congestion_callback,
    btut_gattc_batchscan_cfg_storage_callback,
    btut_gattc_batchscan_enable_disable_callback,
    btut_gattc_batchscan_reports_callback,
    btut_gattc_batchscan_threshold_callback,
    btut_gattc_track_adv_event_callback,
    btut_gattc_scan_parameter_setup_completed_callback,
    btut_gattc_get_gatt_db_callback,
    btut_gattc_services_removed_callback,
    btut_gattc_services_added_callback,
};



// For handling incoming commands from CLI.
int btut_gattc_cmd_handler(int argc, char **argv)
{
    BTUT_CLI *cmd, *match = NULL;
    int ret = 0;
    int count;

    count = 0;
    cmd = bt_gattc_cli_commands;

    BTUT_Logi("[GATTC] argc: %d, argv[0]: %s\n", argc, argv[0]);

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
        BTUT_Logi("[GATTC] Unknown command '%s'\n", argv[0]);

        btut_print_cmd_help(CMD_KEY_GATTC, bt_gattc_cli_commands);
        ret = -1;
    }
    else
    {
        match->handler(argc - 1, &argv[1]);
    }

    return ret;
}

int btut_gattc_init(const btgatt_client_interface_t *interface)
{
    int ret = 0;
    BTUT_MOD gatt_mod = {0};

    // Register command to CLI
    gatt_mod.mod_id = BTUT_MOD_GATT_CLIENT;
    strncpy(gatt_mod.cmd_key, CMD_KEY_GATTC, sizeof(gatt_mod.cmd_key));
    gatt_mod.cmd_handler = btut_gattc_cmd_handler;
    gatt_mod.cmd_tbl = bt_gattc_cli_commands;

    ret = btut_register_mod(&gatt_mod);
    BTUT_Logi("[GATTC] btut_register_mod() returns: %d\n", ret);
    btut_gattc_interface = interface;
    bt_uuid_t uuid;
    btut_gatt_uuid_stoh(BTUT_GATTC_APP_UUID, &uuid);

    if (btut_gattc_interface->register_client(&uuid))
    {
        BTUT_Logi("[GATTC] Register client uuid:'%s'\n", BTUT_GATTC_APP_UUID);
    }
    return ret;
}

int btut_gattc_deinit()
{
    BTUT_Logi("%s", __func__);
    return 0;
}
