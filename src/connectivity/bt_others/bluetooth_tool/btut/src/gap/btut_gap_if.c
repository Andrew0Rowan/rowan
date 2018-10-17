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

#include <stdlib.h>
#include <string.h>
#include <hardware/bluetooth.h>

#include "btif_util.h"
#include "btut_cli.h"
#include "btut_debug.h"
#include "btut_gap_if.h"
#include "btut_a2dp_sink_if.h"
#include "btut_a2dp_src_if.h"
#include "btut_avrcp_if.h"
#include "btut_avrcp_tg_if.h"
#include "btut_gatt_if.h"
#include "btut_hid_if.h"

extern int open_bluetooth_stack (const struct hw_module_t* module, char const* name,
    struct hw_device_t** abstraction);

static int btut_gap_set_scan_mode (int mode);
static int btut_gap_enable_handler(int argc, char *argv[]);
static int btut_gap_disable_handler(int argc, char *argv[]);
static int btut_gap_get_adapter_properties_handler(int argc, char *argv[]);
static int btut_gap_set_device_name_handler(int argc, char *argv[]);
static int btut_gap_set_scan_mode_handler(int argc, char *argv[]);
static int btut_gap_set_cod_handler(int argc, char *argv[]);
static int btut_gap_get_remote_device_properties_handler(int argc, char *argv[]);
static int btut_gap_get_remote_device_property_handler(int argc, char *argv[]);
static int btut_gap_get_remote_service_record_handler(int argc, char *argv[]);
static int btut_gap_get_remote_services_handler(int argc, char *argv[]);
static int btut_gap_start_discovery_handler(int argc, char *argv[]);
static int btut_gap_cancel_discovery_handler(int argc, char *argv[]);
static int btut_gap_create_bond_handler(int argc, char *argv[]);
static int btut_gap_remove_bond_handler(int argc, char *argv[]);
static int btut_gap_cancel_bond_handler(int argc, char *argv[]);
static int btut_gap_config_hci_snoop_log_handler(int argc, char *argv[]);
static int btut_gap_get_rssi_handler(int argc, char *argv[]);
static void btut_gap_show_properties_debug(int num_properties, bt_property_t *properties);
static void btut_gap_show_properties_info(int num_properties, bt_property_t *properties);
static void btut_gap_btaddr_stoh(char *btaddr_s, bt_bdaddr_t *bdaddr_h);
static int btut_gap_send_hci_handler(int argc, char *argv[]);

// Callback functions declaration
static void btut_gap_state_changed_cb(bt_state_t state);
static void btut_gap_properties_cb(bt_status_t status,
                                       int num_properties,
                                       bt_property_t *properties);
static void btut_gap_remote_device_properties_cb(bt_status_t status,
                                                        bt_bdaddr_t *bd_addr,
                                                        int num_properties,
                                                        bt_property_t *properties);
static void btut_gap_device_found_cb(int num_properties,
                                           bt_property_t *properties);
static void btut_gap_discovery_state_changed_cb(bt_discovery_state_t state);
static void btut_gap_pin_request_cb(bt_bdaddr_t *remote_bd_addr,
                                         bt_bdname_t *bd_name, uint32_t cod, bool min_16_digit);
static void btut_gap_ssp_request_cb(bt_bdaddr_t *remote_bd_addr,
                                                bt_bdname_t *bd_name, uint32_t cod,
                                                bt_ssp_variant_t pairing_variant,
                                                uint32_t pass_key);
static void btut_gap_bond_state_changed_cb(bt_status_t status,
                                                         bt_bdaddr_t *remote_bd_addr,
                                                         bt_bond_state_t state);
static void btut_gap_acl_state_changed_cb(bt_status_t status, bt_bdaddr_t *remote_bd_addr,
                                            bt_acl_state_t state);
static void btut_gap_thread_event_cb(bt_cb_thread_evt evt);
static void btut_gap_dut_mode_recv_cb(uint16_t opcode, uint8_t *buf, uint8_t len);
static void btut_gap_le_test_mode_cb(bt_status_t status, uint16_t num_packets);

int default_scan_mode = 2;
static bt_state_t power_state = BT_STATE_OFF;
static bluetooth_device_t *g_bt_device;
const bt_interface_t *g_bt_interface;
static bt_callbacks_t g_bt_callbacks =
{
    sizeof(bt_callbacks_t),
    btut_gap_state_changed_cb,
    btut_gap_properties_cb,
    btut_gap_remote_device_properties_cb,
    btut_gap_device_found_cb,
    btut_gap_discovery_state_changed_cb,
    btut_gap_pin_request_cb,
    btut_gap_ssp_request_cb,
    btut_gap_bond_state_changed_cb,
    btut_gap_acl_state_changed_cb,
    btut_gap_thread_event_cb,
    btut_gap_dut_mode_recv_cb,
    btut_gap_le_test_mode_cb,
    NULL,
};

static BTUT_CLI bt_gap_cli_commands[] = {
    { "enable",                 btut_gap_enable_handler,                    "= gap enable" },
    { "disable",                btut_gap_disable_handler,                   "= gap disable" },
    { "status",                 btut_gap_get_adapter_properties_handler,    "= get local bluetooth device properties" },
    { "set_local_name",         btut_gap_set_device_name_handler,           "= set local bluetooth device name" },
    { "set_scan_mode",          btut_gap_set_scan_mode_handler,             "= set bluetooth scan mode ([mode, 0:NONE | 1:CONNECTABLE | 2:CONNECTABLE and DISCOVERABLE])" },
    { "start_discovery",        btut_gap_start_discovery_handler,           "= search device" },
    { "cancel_discovery",       btut_gap_cancel_discovery_handler,          "= stop search device" },
    { "start_sdp",              btut_gap_get_remote_services_handler,       "= start sdp query ([addr])" },
    { "create_bond",            btut_gap_create_bond_handler,               "= start pairing" },
    { "remove_bond",            btut_gap_remove_bond_handler,               "= remove pairing" },
    { "cancel_bond",            btut_gap_cancel_bond_handler,               "= cancel pairing" },
    { "config_hci_snoop_log",   btut_gap_config_hci_snoop_log_handler,      "= enable virtual sniffer" },
    { "get_rssi",               btut_gap_get_rssi_handler,                  "= get rssi([addr])" },
    { "send_hci",               btut_gap_send_hci_handler,                  "= send hci[data(hex)]" },
    { "set_cod",                btut_gap_set_cod_handler,                   "= set class of device (cod [unsigned int in hex or decimal format- 0xabcdef or 123456])"},
    { "get_remote_service_record", btut_gap_get_remote_service_record_handler,    "= get remote service record ([addr][16bit uuid in hex] ex: 110A for A2DP AudioSource)" },
    { "get_remote_device_properties", btut_gap_get_remote_device_properties_handler,   "= get remote device properties [addr]" },
    { "get_remote_device_property",   btut_gap_get_remote_device_property_handler,     "= get remote device property [addr][property]"},
    { NULL, NULL, NULL }
};

static int btut_gap_send_hci_handler(int argc, char *argv[])
{
    int i=0;
    uint8_t   rpt_size = strlen(argv[0]);
    uint8_t   hex_bytes_filled;
    uint8_t   hex_buf[200] = {0};
    uint16_t   hex_len = (strlen(argv[0]) + 1) / 2;

    if (argc <=0 )
    {
        BTUT_Loge("[GAP] Usage : btut_gap_send_hci_handler ([command hex)])\n", __func__);
        return -1;
    }

    BTUT_Logi("%s:rpt_size, hex_len=%d", __func__, rpt_size, hex_len);
    hex_bytes_filled = ascii_2_hex(argv[0], hex_len, hex_buf);
    BTUT_Logi("%s:hex_bytes_filled=%d", __func__, hex_bytes_filled);
    for(i=0;i<hex_len;i++)
    {
       BTUT_Logi("hex values= %02X",hex_buf[i]);
    }
    if (hex_bytes_filled)
    {
        //g_bt_interface->send_hci((uint8_t*)hex_buf,hex_bytes_filled);
        BTUT_Logi("send_hci");
        return 0;
    }
    else
    {
        BTUT_Logi("%s:hex_bytes_filled <= 0", __func__);
        return -1;
    }
    return 0;
}

static int btut_gap_get_rssi_handler(int argc, char *argv[])
{
    char *ptr;
    bt_bdaddr_t bdaddr;
    BTUT_Logi("[GAP] %s()\n", __func__);
    if (argc <= 0)
    {
        BTUT_Logi("Usage :\n");
        BTUT_Logi("  GAP get_rssi [BT_ADDR]\n");
        return -1;
    }
    ptr = argv[0];
    btut_gap_btaddr_stoh(ptr, &bdaddr);
    BTUT_Logd("BTADDR = %02X:%02X:%02X:%02X:%02X:%02X\n",
        bdaddr.address[0], bdaddr.address[1], bdaddr.address[2],
        bdaddr.address[3], bdaddr.address[4], bdaddr.address[5]);
    //g_bt_interface->get_rssi(&bdaddr);
    return 0;
}

static int btut_gap_enable_handler(int argc, char *argv[])
{
    BTUT_Logi("[GAP] %s()\n", __func__);

    g_bt_interface->enable(false);

    return 0;
}

static int btut_gap_disable_handler(int argc, char *argv[])
{
    BTUT_Logi("[GAP] %s()\n", __func__);

    g_bt_interface->disable();

    return 0;
}

static int btut_gap_get_adapter_properties_handler(int argc, char *argv[])
{
    BTUT_Logi("[GAP] %s()\n", __func__);

    if(power_state == BT_STATE_OFF)
    {
    BTUT_Logi("Bluetooth state = OFF");
    }
    g_bt_interface->get_adapter_properties();

    return 0;
}

static int btut_gap_set_device_name_handler(int argc, char *argv[])
{
    bt_property_t property;
    bt_property_t *property_p;
    char *ptr;

    if (argc < 1)
    {
        BTUT_Logi("Usage :\n");
        BTUT_Logi("  GAP set_device_name [BT_NAME]\n");

        return -1;
    }

    ptr = argv[0];
    property_p = &property;

    property_p->type = BT_PROPERTY_BDNAME;
    property_p->len = strlen(ptr);
    property_p->val = ptr;

    g_bt_interface->set_adapter_property(property_p);

    return 0;
}

static int btut_gap_set_scan_mode(int mode)
{
    bt_property_t property;
    bt_property_t *property_p;
    bt_scan_mode_t scan_mode;

    scan_mode = (bt_scan_mode_t)mode;

    property_p = &property;
    property_p->type = BT_PROPERTY_ADAPTER_SCAN_MODE;
    property_p->len = sizeof(bt_scan_mode_t);
    property_p->val = (void*)&scan_mode;

    g_bt_interface->set_adapter_property(property_p);

    return 0;
}

static int btut_gap_set_scan_mode_handler(int argc, char *argv[])
{
    bt_property_t property;
    bt_property_t *property_p;
    bt_scan_mode_t scan_mode;
    char *ptr;

    if (argc < 1)
    {
        BTUT_Logi("Usage :\n");
        BTUT_Logi("  GAP set_scan_mode [0:none, 1:connectable, 2:connectable_discoverable]\n");

        return -1;
    }

    ptr = argv[0];
    property_p = &property;

    scan_mode = (bt_scan_mode_t)atoi(ptr);

    property_p->type = BT_PROPERTY_ADAPTER_SCAN_MODE;
    property_p->len = sizeof(bt_scan_mode_t);
    property_p->val = (void*)&scan_mode;

    g_bt_interface->set_adapter_property(property_p);

    return 0;
}

static int btut_gap_set_cod_handler(int argc, char *argv[])
{
    unsigned int cod;
    bt_property_t property;
    bt_property_t *property_p;
    char *ptr;

    if (argc < 1)
    {
        BTUT_Logi("Usage :\n");
        BTUT_Logi("  GAP set_cod [cod (unsigned int in hex or decimal format- 0xabcdef or 123456)]\n");

        return -1;
    }

    ptr = argv[0];
    property_p = &property;

    if(ptr[0] == '0' && (ptr[1] == 'x' || ptr[1] == 'X') )
        cod = (unsigned int)strtol(ptr, NULL, 16);
    else
        cod = (unsigned int)strtol(ptr, NULL, 0);
    BTUT_Logi("COD : %x", cod);

    property_p->type = BT_PROPERTY_CLASS_OF_DEVICE;
    property_p->len = sizeof(unsigned int);
    property_p->val = &cod;

    g_bt_interface->set_adapter_property(property_p);
    return 0;
}

static int btut_gap_get_remote_device_properties_handler(int argc, char *argv[])
{
    char *ptr;
    bt_bdaddr_t bdaddr;

    BTUT_Logi("[GAP] %s()\n", __func__);

    if (argc <= 0)
    {
        BTUT_Logi("Usage :\n");
        BTUT_Logi("  GAP get_remote_device_properties [BT_ADDR]\n");

        return -1;
    }

    ptr = argv[0];
    btut_gap_btaddr_stoh(ptr, &bdaddr);
    BTUT_Logi("BTADDR = %02X:%02X:%02X:%02X:%02X:%02X\n",
        bdaddr.address[0], bdaddr.address[1], bdaddr.address[2],
        bdaddr.address[3], bdaddr.address[4], bdaddr.address[5]);

    g_bt_interface->get_remote_device_properties(&bdaddr);

    return 0;
}

static int btut_gap_get_remote_device_property_handler(int argc, char *argv[])
{
    char *ptr;
    bt_bdaddr_t bdaddr;
    bt_property_type_t num;

    BTUT_Logi("[GAP] %s()\n", __func__);

    if (argc < 2)
    {
        BTUT_Logi("Usage is :\n");
        BTUT_Logi("  GAP get_remote_device_property [BT_ADDR] [1 - 12]\n");

        return -1;
    }

    ptr = argv[0];
    btut_gap_btaddr_stoh(ptr, &bdaddr);
    BTUT_Logi("BTADDR = %02X:%02X:%02X:%02X:%02X:%02X\n",
        bdaddr.address[0], bdaddr.address[1], bdaddr.address[2],
        bdaddr.address[3], bdaddr.address[4], bdaddr.address[5]);

    num = (bt_property_type_t)atoi(argv[1]);

    g_bt_interface->get_remote_device_property(&bdaddr, num);

    return 0;
}

static void btut_gap_uuid_stoh(char *uuid_s,  bt_uuid_t *uuid)
{
    int i = 0,j = 0;
    int size = strlen(uuid_s);
    char temp[3];
    temp[2] = '\0';

    while (i < size) {
        if (uuid_s[i] == '-' || uuid_s[i] == '\0') {
            i++;
            continue;
        }
        temp[0] = uuid_s[i];
        temp[1] = uuid_s[i+1];
        uuid->uu[j] = strtoul(temp, NULL, 16);
        i+=2;
        j++;
    }

    if (size <= 8) { // 16bits uuid or 32bits uuid
        if (size == 4) {
            uuid->uu[2] = uuid->uu[0];
            uuid->uu[3] = uuid->uu[1];
            uuid->uu[0] = 0;
            uuid->uu[1] = 0;
        }
        uuid->uu[4] = 0x00;
        uuid->uu[5] = 0x00;
        uuid->uu[6] = 0x10;
        uuid->uu[7] = 0x00;

        uuid->uu[8] = 0x80;
        uuid->uu[9] = 0x00;
        uuid->uu[10] = 0x00;
        uuid->uu[11] = 0x80;

        uuid->uu[12] = 0x5F;
        uuid->uu[13] = 0x9B;
        uuid->uu[14] = 0x34;
        uuid->uu[15] = 0xFB;
    }
}

static int btut_gap_get_remote_service_record_handler(int argc, char *argv[])
{
    char *ptr;
    bt_bdaddr_t bdaddr;
    bt_uuid_t uuid;
    int size = 0;

    BTUT_Logi("[GAP] %s()\n", __func__);

    if (argc < 2)
    {
        BTUT_Logi("Usage :\n");
        BTUT_Logi("get_remote_service_record [BT_ADDR][128 bit UUID]\n");

        return -1;
    }

    ptr = argv[0];
    btut_gap_btaddr_stoh(ptr, &bdaddr);
    BTUT_Logi("ADDR = %02X:%02X:%02X:%02X:%02X:%02X\n",
        bdaddr.address[0], bdaddr.address[1], bdaddr.address[2],
        bdaddr.address[3], bdaddr.address[4], bdaddr.address[5]);

    ptr = argv[1];
    size = strlen(ptr);
    if (size > 36)
    {
        BTUT_Loge("Enter 128 bit UUID\n");
        return -1;
    }
    btut_gap_uuid_stoh(ptr, &uuid);
    BTUT_Logi("UUID = %02X%02X%02X%02X-%02X%02X-%02X%02X-%02X%02X-%02X%02X%02X%02X%02X%02X\n",
        uuid.uu[0], uuid.uu[1], uuid.uu[2], uuid.uu[3],
        uuid.uu[4], uuid.uu[5], uuid.uu[6], uuid.uu[7],
        uuid.uu[8], uuid.uu[9], uuid.uu[10], uuid.uu[11],
        uuid.uu[12], uuid.uu[13], uuid.uu[14], uuid.uu[15]);

    g_bt_interface->get_remote_service_record(&bdaddr, &uuid);

    return 0;
}

static int btut_gap_get_remote_services_handler(int argc, char *argv[])
{
    char *ptr;
    bt_bdaddr_t bdaddr;

    BTUT_Logi("[GAP] %s()\n", __func__);

    if (argc <= 0)
    {
        BTUT_Logi("Usage :\n");
        BTUT_Logi("  get_remote_device_properties [BT_ADDR]\n");

        return -1;
    }

    ptr = argv[0];
    btut_gap_btaddr_stoh(ptr, &bdaddr);
    BTUT_Logi("CBTADDR = %02X:%02X:%02X:%02X:%02X:%02X\n",
        bdaddr.address[0], bdaddr.address[1], bdaddr.address[2],
        bdaddr.address[3], bdaddr.address[4], bdaddr.address[5]);

    g_bt_interface->get_remote_services(&bdaddr);

    return 0;
}

static int btut_gap_start_discovery_handler(int argc, char *argv[])
{
    BTUT_Logi("[GAP] %s()\n", __func__);

    g_bt_interface->start_discovery();

    return 0;
}

static int btut_gap_cancel_discovery_handler(int argc, char *argv[])
{
    BTUT_Logi("[GAP] %s()\n", __func__);

    g_bt_interface->cancel_discovery();

    return 0;
}

static int btut_gap_create_bond_handler(int argc, char *argv[])
{
    char *ptr;
    bt_bdaddr_t bdaddr;

    BTUT_Logi("[GAP] %s()\n", __func__);

    if (argc <= 0)
    {
        BTUT_Logi("Usage :\n");
        BTUT_Logi("  GAP create_bond [BT_ADDR]\n");

        return -1;
    }

    ptr = argv[0];
    btut_gap_btaddr_stoh(ptr, &bdaddr);
    BTUT_Logi("Cancel pairing to %02X:%02X:%02X:%02X:%02X:%02X\n",
        bdaddr.address[0], bdaddr.address[1], bdaddr.address[2],
        bdaddr.address[3], bdaddr.address[4], bdaddr.address[5]);

    g_bt_interface->create_bond(&bdaddr, 0);

    return 0;
}

static int btut_gap_remove_bond_handler(int argc, char *argv[])
{
    char *ptr;
    bt_bdaddr_t bdaddr;

    BTUT_Logi("[GAP] %s()\n", __func__);

    if (argc <= 0)
    {
        BTUT_Logi("Usage :\n");
        BTUT_Logi("  GAP remove_bond [BT_ADDR]\n");

        return -1;
    }

    ptr = argv[0];
    btut_gap_btaddr_stoh(ptr, &bdaddr);
    BTUT_Logi("Cancel pairing to %02X:%02X:%02X:%02X:%02X:%02X\n",
        bdaddr.address[0], bdaddr.address[1], bdaddr.address[2],
        bdaddr.address[3], bdaddr.address[4], bdaddr.address[5]);

    g_bt_interface->remove_bond(&bdaddr);

    return 0;
}

static int btut_gap_cancel_bond_handler(int argc, char *argv[])
{
    char *ptr;
    bt_bdaddr_t bdaddr;

    BTUT_Logi("[GAP] %s()\n", __func__);

    if (argc <= 0)
    {
        BTUT_Logi("Usage :\n");
        BTUT_Logi("  GAP cancel_bond [BT_ADDR]\n");

        return -1;
    }

    ptr = argv[0];
    btut_gap_btaddr_stoh(ptr, &bdaddr);
    BTUT_Logi("Cancel pairing to %02X:%02X:%02X:%02X:%02X:%02X\n",
        bdaddr.address[0], bdaddr.address[1], bdaddr.address[2],
        bdaddr.address[3], bdaddr.address[4], bdaddr.address[5]);

    g_bt_interface->cancel_bond(&bdaddr);

    return 0;
}

static int btut_gap_config_hci_snoop_log_handler(int argc, char *argv[])
{
    unsigned int enable;
    char *ptr;

    BTUT_Logi("[GAP] %s()\n", __func__);

    if (argc < 1)
    {
        BTUT_Logi("Usage :\n");
        BTUT_Logi("  GAP config_hci_snoop_log [1|0]\n");

        return -1;
    }

    ptr = argv[0];

    enable = atoi(ptr);
    g_bt_interface->config_hci_snoop_log(enable == 0 ? 0 : 1);

    return 0;
}

static void btut_gap_btaddr_stoh(char *btaddr_s, bt_bdaddr_t *bdaddr_h)
{
    int i;

    for (i = 0; i <6; i++)
    {
        bdaddr_h->address[i] = strtoul(btaddr_s, &btaddr_s, 16);
        btaddr_s++;
    }
}

static void btut_gap_show_properties_debug(int num_properties, bt_property_t *properties)
{
    int i, j;
    int len;
    char *name;
    bt_scan_mode_t *mode;
    bt_bdaddr_t *btaddr;
    bt_uuid_t *uuid;
    bt_property_t *property;
    unsigned int *cod;
    unsigned int *devicetype;
    unsigned int *disc_timeout;
    short *rssi;

    BTUT_Logi("=======================================Properties==============================================\n");
    BTUT_Logi("[GAP] num_properties = %d\n", num_properties);

    for (i = 0; i < num_properties; i++)
    {
        property = &properties[i];

        switch (property->type)
        {
        /* 1 */
        case BT_PROPERTY_BDNAME:
            name = (char *)property->val;
            BTUT_Logi("[GAP] type = %d, len = %d, bdname = %s\n", property->type, property->len, name);

            break;
        /* 2 */
        case BT_PROPERTY_BDADDR:
            btaddr = (bt_bdaddr_t *)property->val;

            BTUT_Logi("[GAP] type = %d, len = %d, bdaddr = %02X:%02X:%02X:%02X:%02X:%02X\n", property->type, property->len,
                btaddr->address[0], btaddr->address[1], btaddr->address[2],
                btaddr->address[3], btaddr->address[4], btaddr->address[5]);

            break;
        /* 3 */
        case BT_PROPERTY_UUIDS:
            uuid = (bt_uuid_t*)property->val;
            len = property->len;

            for (j=0; j<len; j+=16)
            {
                BTUT_Logi("[GAP] type = %d, len = %d, uuid = %02X%02X%02X%02X-%02X%02X-%02X%02X-%02X%02X-%02X%02X%02X%02X%02X%02X\n",
                    property->type, property->len,
                    uuid->uu[j+0], uuid->uu[j+1], uuid->uu[j+2], uuid->uu[j+3],
                    uuid->uu[j+4], uuid->uu[j+5], uuid->uu[j+6], uuid->uu[j+7],
                    uuid->uu[j+8], uuid->uu[j+9], uuid->uu[j+10], uuid->uu[j+11],
                    uuid->uu[j+12], uuid->uu[j+13], uuid->uu[j+14], uuid->uu[j+15]
                    );
            }

            break;
        /* 4 */
        case BT_PROPERTY_CLASS_OF_DEVICE:
            cod = (unsigned int *)property->val;

            BTUT_Logi("[GAP] type = %d, len = %d, cod = 0x%lX\n", property->type, property->len, *cod);
            break;
        /* 5 */
        case BT_PROPERTY_TYPE_OF_DEVICE:
            devicetype= (unsigned int *)property->val;

            /* 0 - BLE, 1 - BT, 2 - DUAL MODE */
            BTUT_Logi("[GAP] type = %d, len = %d, devicetype = %d\n", property->type, property->len, *devicetype);
            break;
        /* 6 */
        case BT_PROPERTY_SERVICE_RECORD:
            BTUT_Logi("[GAP] type = %d\n", property->type);
            break;
        /* 7 */
        case BT_PROPERTY_ADAPTER_SCAN_MODE:
            mode = (bt_scan_mode_t *)property->val;

            BTUT_Logi("[GAP] type = %d, len = %d, scan mode = %d\n", property->type, property->len, (uint32_t)*mode);
            break;
        /* 8 */
        case BT_PROPERTY_ADAPTER_BONDED_DEVICES:
            btaddr = (bt_bdaddr_t *)property->val;

            BTUT_Logi("[GAP] type = %d, len = %d, bonded_addr = %02X:%02X:%02X:%02X:%02X:%02X\n", property->type, property->len,
                btaddr->address[0], btaddr->address[1], btaddr->address[2],
                btaddr->address[3], btaddr->address[4], btaddr->address[5]);
            break;
        /* 9 */
        case BT_PROPERTY_ADAPTER_DISCOVERY_TIMEOUT:
            disc_timeout = (unsigned int *)property->val;

            BTUT_Logi("[GAP] type = %d, len = %d, disc_timeout = %d\n", property->type, property->len, *disc_timeout);
            break;
        /* 10 */
        case BT_PROPERTY_REMOTE_FRIENDLY_NAME:
            name = (char *)property->val;
            BTUT_Logi("[GAP] type = %d, len = %d, bdname = %s\n", property->type, property->len, name);

            break;
        /* 11 */
        case BT_PROPERTY_REMOTE_RSSI:
            rssi = (short *)property->val;

            BTUT_Logi("[GAP] type = %d, len = %d, rssi = %d\n", property->type, property->len, *rssi);
            break;
        /* 12 */
        case BT_PROPERTY_REMOTE_VERSION_INFO:
            BTUT_Logi("[GAP] type = %d, len = %d, Others.\n", property->type, property->len);
            break;
        /* FF */
        case BT_PROPERTY_REMOTE_DEVICE_TIMESTAMP:
            BTUT_Logi("[GAP] type = %d, len = %d, Others.\n", property->type, property->len);
            break;
        default:
            BTUT_Logi("[GAP] type = %d, len = %d, Others.\n", property->type, property->len);
            break;
        }
    }

    BTUT_Logi("==========================================End==================================================\n");
}

static void btut_gap_show_properties_info(int num_properties, bt_property_t *properties)
{
    int i, j;
    int len;
    char *name;
    bt_scan_mode_t *mode;
    bt_bdaddr_t *btaddr;
    bt_uuid_t *uuid;
    bt_property_t *property;
    unsigned int *cod;
    unsigned int *devicetype;
    unsigned int *disc_timeout;
    short *rssi;

    BTUT_Logi("=======================================Properties==============================================\n");
    if( power_state == BT_STATE_ON )
    {
    BTUT_Logi("Bluetooth state = POWER ON");
    }
    else
    {
    BTUT_Logi("Bluetooth state = POWER OFF");
    }

    BTUT_Logi("[GAP] num_properties = %d\n", num_properties);

    for (i = 0; i < num_properties; i++)
    {
        property = &properties[i];

        switch (property->type)
        {
        /* 1 */
        case BT_PROPERTY_BDNAME:
            name = (char *)property->val;
            BTUT_Logi("[GAP] type = %d, len = %d, bdname = %s\n", property->type, property->len, name);

            break;
        /* 2 */
        case BT_PROPERTY_BDADDR:
            btaddr = (bt_bdaddr_t *)property->val;

            BTUT_Logi("[GAP] type = %d, len = %d, bdaddr = %02X:%02X:%02X:%02X:%02X:%02X\n", property->type, property->len,
                btaddr->address[0], btaddr->address[1], btaddr->address[2],
                btaddr->address[3], btaddr->address[4], btaddr->address[5]);

            break;
        /* 3 */
        case BT_PROPERTY_UUIDS:
            uuid = (bt_uuid_t*)property->val;
            len = property->len;

            for (j=0; j<len; j+=16)
            {
                BTUT_Logi("[GAP] type = %d, len = %d, uuid = %02X%02X%02X%02X-%02X%02X-%02X%02X-%02X%02X-%02X%02X%02X%02X%02X%02X\n",
                    property->type, property->len,
                    uuid->uu[j+0], uuid->uu[j+1], uuid->uu[j+2], uuid->uu[j+3],
                    uuid->uu[j+4], uuid->uu[j+5], uuid->uu[j+6], uuid->uu[j+7],
                    uuid->uu[j+8], uuid->uu[j+9], uuid->uu[j+10], uuid->uu[j+11],
                    uuid->uu[j+12], uuid->uu[j+13], uuid->uu[j+14], uuid->uu[j+15]
                    );
            }

            break;
        /* 4 */
        case BT_PROPERTY_CLASS_OF_DEVICE:
            cod = (unsigned int *)property->val;

            BTUT_Logi("[GAP] type = %d, len = %d, cod = 0x%lX\n", property->type, property->len, *cod);
            break;
        /* 5 */
        case BT_PROPERTY_TYPE_OF_DEVICE:
            devicetype= (unsigned int *)property->val;

            /* 0 - BLE, 1 - BT, 2 - DUAL MODE */
            BTUT_Logi("[GAP] type = %d, len = %d, devicetype = %d\n", property->type, property->len, *devicetype);
            break;
        /* 6 */
        case BT_PROPERTY_SERVICE_RECORD:
            BTUT_Logi("[GAP] type = %d, len = %d, Others.\n", property->type, property->len);
            break;
        /* 7 */
        case BT_PROPERTY_ADAPTER_SCAN_MODE:
            mode = (bt_scan_mode_t *)property->val;

            BTUT_Logi("[GAP] type = %d, len = %d, scan mode = %d\n", property->type, property->len, (uint32_t)*mode);
            break;
        /* 8 */
        case BT_PROPERTY_ADAPTER_BONDED_DEVICES:
            btaddr = (bt_bdaddr_t *)property->val;

            BTUT_Logi("[GAP] type = %d, len = %d, bonded_addr = %02X:%02X:%02X:%02X:%02X:%02X\n", property->type, property->len,
                btaddr->address[0], btaddr->address[1], btaddr->address[2],
                btaddr->address[3], btaddr->address[4], btaddr->address[5]);
            break;
        /* 9 */
        case BT_PROPERTY_ADAPTER_DISCOVERY_TIMEOUT:
            disc_timeout = (unsigned int *)property->val;

            BTUT_Logi("[GAP] type = %d, len = %d, disc_timeout = %d\n", property->type, property->len, *disc_timeout);
            break;
        /* 10 */
        case BT_PROPERTY_REMOTE_FRIENDLY_NAME:
            name = (char *)property->val;
            BTUT_Logi("[GAP] type = %d, len = %d, bdname = %s\n", property->type, property->len, name);

            break;
        /* 11 */
        case BT_PROPERTY_REMOTE_RSSI:
            rssi = (short *)property->val;

            BTUT_Logi("[GAP] type = %d, len = %d, rssi = %d\n", property->type, property->len, *rssi);
            break;
        /* 12 */
        case BT_PROPERTY_REMOTE_VERSION_INFO:
            BTUT_Logi("[GAP] type = %d, len = %d, Others.\n", property->type, property->len);
            break;
        /* FF */
        case BT_PROPERTY_REMOTE_DEVICE_TIMESTAMP:
            BTUT_Logi("[GAP] type = %d, len = %d, Others.\n", property->type, property->len);
            break;
        default:
            BTUT_Logi("[GAP] type = %d, len = %d, Others.\n", property->type, property->len);
            break;
        }
    }

    BTUT_Logi("==========================================End==================================================\n");
}

// Callback functions implementation
static void btut_gap_state_changed_cb(bt_state_t state)
{
    BTUT_Logd("[GAP] %s() state: %s (%d)\n", __func__, state==BT_STATE_ON?"ON":state==BT_STATE_OFF?"OFF":"UNKNOWN", state);

    switch (state)
    {
        case BT_STATE_OFF:
            BTUT_Logi("[GAP] BT STATE OFF\n");
            power_state = BT_STATE_OFF;
            btut_a2dp_sink_deinit();
            btut_a2dp_src_deinit();
            btut_rc_deinit();
            btut_rc_tg_deinit();
            btut_hid_deinit();
            btut_gatt_deinit();
            break;
        case BT_STATE_ON:
            BTUT_Logi("[GAP] BT STATE ON\n");

            if (default_scan_mode)
            {
                btut_gap_set_scan_mode(default_scan_mode);
            }
            btut_a2dp_sink_init();
            btut_a2dp_src_init();
            btut_rc_init();
            btut_rc_tg_init();
            btut_hid_init();
            btut_gatt_init();
            power_state = BT_STATE_ON;
            break;
        default:
            break;
    }
}

static void btut_gap_properties_cb(bt_status_t status,
                                       int num_properties,
                                       bt_property_t *properties)
{
    BTUT_Logd("[GAP] %s() status: %s (%d)\n", __func__, status==BT_STATUS_SUCCESS?"SUCCESS":"FAILED", status);

    btut_gap_show_properties_info(num_properties, properties);
}

static void btut_gap_remote_device_properties_cb(bt_status_t status,
                                                        bt_bdaddr_t *bd_addr,
                                                        int num_properties,
                                                        bt_property_t *properties)
{
    BTUT_Logd("[GAP] %s() status: %s(%d)\n", __func__, status==BT_STATUS_SUCCESS?"SUCCESS":"FAILED", status);
    BTUT_Logd("[GAP] REMOTE BDADDR = %02X:%02X:%02X:%02X:%02X:%02X\n",
        bd_addr->address[0], bd_addr->address[1], bd_addr->address[2],
        bd_addr->address[3], bd_addr->address[4], bd_addr->address[5]);

    btut_gap_show_properties_debug(num_properties, properties);
}

static void btut_gap_device_found_cb(int num_properties,
                                           bt_property_t *properties)
{
    BTUT_Logd("[GAP] %s()\n", __func__);

    btut_gap_show_properties_debug(num_properties, properties);
}

static void btut_gap_discovery_state_changed_cb(bt_discovery_state_t state)
{
    BTUT_Logd("[GAP] %s() state: %d\n", __func__, state);

    switch (state)
    {
        case BT_DISCOVERY_STOPPED:
            BTUT_Logi("[GAP] BT Search Device Stop.\n");
            break;
        case BT_DISCOVERY_STARTED:
            BTUT_Logi("[GAP] BT Search Device Start...\n");
            break;
        default:
            break;
    }
}

static void btut_gap_pin_request_cb(bt_bdaddr_t *remote_bd_addr,
                                         bt_bdname_t *bd_name, uint32_t cod, bool min_16_digit)
{
    bt_pin_code_t pin;
    BTUT_Logd("[GAP] %s()\n", __func__);

    memset(&pin, 0, 16);
    memset(&pin, '0', 4);
    g_bt_interface->pin_reply(remote_bd_addr, 1, 4, &pin);
}

static void btut_gap_ssp_request_cb(bt_bdaddr_t *remote_bd_addr,
                                                bt_bdname_t *bd_name, uint32_t cod,
                                                bt_ssp_variant_t pairing_variant,
                                                uint32_t pass_key)
{
    BTUT_Logd("[GAP] %s()\n", __func__);

    // Iverson debug
    if (remote_bd_addr)
    {
        bt_bdaddr_t *btaddr = remote_bd_addr;

        BTUT_Logd("[GAP] REMOTE BDADDR = %02X:%02X:%02X:%02X:%02X:%02X\n",
            btaddr->address[0], btaddr->address[1], btaddr->address[2],
            btaddr->address[3], btaddr->address[4], btaddr->address[5]);
    }

    if (bd_name)
    {
        BTUT_Logd("[GAP] BDNAME = %s\n", bd_name);
    }
    BTUT_Logd("[GAP] cod = 0x%08X, pairing_variant = %d, pass_key = %d.\n", cod, pairing_variant, pass_key);
    BTUT_Logi("[GAP] pass_key = %d.\n", pass_key);
    g_bt_interface->ssp_reply(remote_bd_addr, pairing_variant, 1, pass_key);
}

static void btut_gap_bond_state_changed_cb(bt_status_t status,
                                                         bt_bdaddr_t *remote_bd_addr,
                                                         bt_bond_state_t state)
{
    BTUT_Logd("[GAP] %s(), status = %d, state = %d\n", __func__, status, state);

    switch (status)
    {
        case BT_STATUS_SUCCESS:
            BTUT_Logi("[GAP] BT bond status is successful(%d), ", status);
            break;
        default:
            BTUT_Logi("[GAP] BT bond status is failed(%d), ", status);
            break;
    }

    switch (state)
    {
        case BT_BOND_STATE_NONE:
            BTUT_Logi("state is none.\n");
            break;
        case BT_BOND_STATE_BONDING:
            BTUT_Logi("state is bonding.\n");
            break;
        case BT_BOND_STATE_BONDED:
            BTUT_Logi("state is bonded.\n");
            break;
        default:
            break;
    }

    if (remote_bd_addr)
    {
        bt_bdaddr_t *btaddr = remote_bd_addr;

        if ( (status == BT_STATUS_SUCCESS) && (state == BT_BOND_STATE_BONDED) )
        {
            BTUT_Logi("[GAP] REMOTE BDADDR = %02X:%02X:%02X:%02X:%02X:%02X\n",
                btaddr->address[0], btaddr->address[1], btaddr->address[2],
                btaddr->address[3], btaddr->address[4], btaddr->address[5]);
        }
        else
        {
            BTUT_Logd("[GAP] REMOTE BDADDR = %02X:%02X:%02X:%02X:%02X:%02X\n",
                btaddr->address[0], btaddr->address[1], btaddr->address[2],
                btaddr->address[3], btaddr->address[4], btaddr->address[5]);
        }
    }
}

static void btut_gap_acl_state_changed_cb(bt_status_t status, bt_bdaddr_t *remote_bd_addr,
                                            bt_acl_state_t state)
{
    BTUT_Logd("[GAP] %s(), status = %d, state = %d\n", __func__, status, state);

    switch (status)
    {
        case BT_STATUS_SUCCESS:
            BTUT_Logi("[GAP] BT bond status is successful(%d), ", status);
            break;
        default:
            BTUT_Logi("[GAP] BT bond status is failed(%d), ", status);
            break;
    }

    switch (state)
    {
        case BT_ACL_STATE_CONNECTED:
            BTUT_Logi("acl is connected.\n");
            break;
        case BT_ACL_STATE_DISCONNECTED:
            BTUT_Logi("acl is disconnected.\n");
            break;
        default:
            break;
    }

    if (remote_bd_addr)
    {
        bt_bdaddr_t *btaddr = remote_bd_addr;

        if(status == BT_STATUS_SUCCESS)
        {
            BTUT_Logi("[GAP] REMOTE BDADDR = %02X:%02X:%02X:%02X:%02X:%02X\n",
                btaddr->address[0], btaddr->address[1], btaddr->address[2],
                btaddr->address[3], btaddr->address[4], btaddr->address[5]);
        }
        else
        {
            BTUT_Logd("[GAP] REMOTE BDADDR = %02X:%02X:%02X:%02X:%02X:%02X\n",
                btaddr->address[0], btaddr->address[1], btaddr->address[2],
                btaddr->address[3], btaddr->address[4], btaddr->address[5]);
        }
    }
}

static void btut_gap_thread_event_cb(bt_cb_thread_evt evt)
{
    BTUT_Logd("[GAP] %s()\n", __func__);
}

static void btut_gap_dut_mode_recv_cb(uint16_t opcode, uint8_t *buf, uint8_t len)
{
    BTUT_Logd("[GAP] %s()\n", __func__);
}

static void btut_gap_le_test_mode_cb(bt_status_t status, uint16_t num_packets)
{
    BTUT_Logd("[GAP] %s()\n", __func__);
}

// For handling incoming commands from CLI.
int btut_gap_cmd_handler(int argc, char **argv)
{
    BTUT_CLI *cmd, *match = NULL;
    int ret = 0;
    int count;

    count = 0;
    cmd = bt_gap_cli_commands;

    BTUT_Logi("[GAP] argc: %d, argv[0]: %s\n", argc, argv[0]);

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
        btut_print_cmd_help(CMD_KEY_GAP, bt_gap_cli_commands);
        ret = -1;
    }
    else
    {
        match->handler(argc - 1, &argv[1]);
    }

    return ret;
}

const void *btut_gap_get_profile_interface(const char *profile_id)
{
    if (g_bt_interface != NULL)
    {
        return g_bt_interface->get_profile_interface(profile_id);
    }

    return NULL;
}

int btut_gap_init()
{
    int ret = 0;
    BTUT_MOD gap_mod = {0};

    // Init bluetooth interface.
    open_bluetooth_stack(NULL, "BlueDroid Linux", (hw_device_t **)&g_bt_device);
    if (g_bt_device == NULL)
    {
        BTUT_Loge("[GAP] Failed to open Bluetooth stack.\n");
        return -1;
    }

    g_bt_interface = g_bt_device->get_bluetooth_interface();
    if (g_bt_device == NULL)
    {
        BTUT_Loge("[GAP] Failed to get Bluetooth interface\n");
        return -1;
    }

    g_bt_interface->init(&g_bt_callbacks);
    g_bt_interface->config_hci_snoop_log(1);

    // Register to btut_cli.
    gap_mod.mod_id = BTUT_MOD_GAP;
    strncpy(gap_mod.cmd_key, CMD_KEY_GAP, sizeof(gap_mod.cmd_key));
    gap_mod.cmd_handler = btut_gap_cmd_handler;
    gap_mod.cmd_tbl = bt_gap_cli_commands;

    ret = btut_register_mod(&gap_mod);
    BTUT_Logi("[GAP] btut_register_mod() returns: %d\n", ret);

    //default gap enable
    ret = g_bt_interface->enable(false);

    return ret;
}

int btut_gap_deinit_profiles()
{
    BTUT_Logi("%s()\n", __func__);

    btut_a2dp_sink_deinit();
    btut_a2dp_src_deinit();
    btut_rc_deinit();
    btut_rc_tg_deinit();
    btut_hid_deinit();
    btut_gatt_deinit();
    return 0;
}

int btut_gap_deinit()
{
    g_bt_interface->disable();
    g_bt_interface->cleanup();
    return 0;
}
