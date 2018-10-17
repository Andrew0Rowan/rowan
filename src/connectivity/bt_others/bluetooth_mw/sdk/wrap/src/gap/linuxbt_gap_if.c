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

/* FILE NAME:  linuxbt_gap_if.c
 * PURPOSE:
 *  {1. What is covered in this file - function and scope.}
 *  {2. Related documents or hardware information}
 * NOTES:
 *  {Something must be known or noticed}
 *  {1. How to use these functions - Give an example.}
 *  {2. Sequence of messages if applicable.}
 *  {3. Any design limitation}
 *  {4. Any performance limitation}
 *  {5. Is it a reusable component}
 *
 */

#include <stdlib.h>
#include <string.h>

#include "bt_mw_common.h"
#include "bluetooth.h"
#include "linuxbt_common.h"
#include "linuxbt_gap_if.h"
#include "bt_mw_gap.h"

#if defined(MTK_LINUX_GAP) && (MTK_LINUX_GAP == TRUE)
#include "mtk_bluetooth.h"
#endif
#include"bt_mw_message_queue.h"

#define UNUSED_ATTR __attribute__((unused))

static bluetooth_device_t *g_bt_device = NULL;
static const bt_interface_t *g_bt_interface = NULL;

extern int open_bluetooth_stack(const struct hw_module_t *module,
                                UNUSED_ATTR char const *name,
                                struct hw_device_t **abstraction) ;

// Callback functions declaration
void linuxbt_gap_state_changed_cb(bt_state_t state);

void linuxbt_gap_properties_cb(bt_status_t status,
                               int num_properties,
                               bt_property_t *properties);

void linuxbt_gap_remote_device_properties_cb(bt_status_t status,
        bt_bdaddr_t *bd_addr,
        int num_properties,
        bt_property_t *properties);

void linuxbt_gap_device_found_cb(int num_properties,
                                 bt_property_t *properties);

void linuxbt_gap_discovery_state_changed_cb(bt_discovery_state_t state);

void linuxbt_gap_pin_request_cb(bt_bdaddr_t *remote_bd_addr,
                                bt_bdname_t *bd_name, uint32_t cod,bool min_16_digit);


void linuxbt_gap_ssp_request_cb(bt_bdaddr_t *remote_bd_addr,
                                bt_bdname_t *bd_name,
                                uint32_t cod,
                                bt_ssp_variant_t pairing_variant,
                                uint32_t pass_key);

void linuxbt_gap_bond_state_changed_cb(bt_status_t status,
                                       bt_bdaddr_t *remote_bd_addr,
                                       bt_bond_state_t state);

void linuxbt_gap_acl_state_changed_cb(bt_status_t status,
                                      bt_bdaddr_t *remote_bd_addr,
                                      bt_acl_state_t state);

void linuxbt_gap_acl_disconnect_reason_callback(bt_bdaddr_t *remote_bd_addr, uint8_t reason);
void linuxbt_gap_get_rssi_cb(bt_status_t status, bt_bdaddr_t *remote_bd_addr , int rssi_value);
void linuxbt_gap_get_bonded_device_cb();


static bt_callbacks_t g_bt_callbacks =
{
    sizeof(bt_callbacks_t),
    linuxbt_gap_state_changed_cb,
    linuxbt_gap_properties_cb,
    linuxbt_gap_remote_device_properties_cb,
    linuxbt_gap_device_found_cb,
    linuxbt_gap_discovery_state_changed_cb,
    linuxbt_gap_pin_request_cb,
    linuxbt_gap_ssp_request_cb,
    linuxbt_gap_bond_state_changed_cb,
    linuxbt_gap_acl_state_changed_cb,
    NULL,
    NULL,
    NULL,
    NULL,
};


#if defined(MTK_LINUX_GAP) && (MTK_LINUX_GAP == TRUE)
static const btgap_ex_interface_t *linuxbt_gap_ex_interface = NULL;

static btgap_ex_callbacks_t linuxbt_gap_ex_callbacks =
{
    sizeof(btgap_ex_callbacks_t),
    linuxbt_gap_get_rssi_cb,
    linuxbt_gap_acl_disconnect_reason_callback,
};

int linuxbtgap_send_hci_handler(char *ptr)
{
    int i = 0;
    uint8_t   rpt_size = 0;
    uint8_t   hex_bytes_filled;
    uint8_t hex_buf[200] = {0};
    uint16_t   hex_len = 0;
    bt_status_t ret = BT_STATUS_SUCCESS;

    if (NULL == ptr)
    {
        BT_DBG_ERROR(BT_DEBUG_GAP, "[HID] Usage : linuxbt_gap_send_hci_handler ([command hex)])");
        return BT_ERR_STATUS_PARM_INVALID;
    }
    if (NULL == linuxbt_gap_ex_interface)
    {
        BT_DBG_ERROR(BT_DEBUG_GAP, "Failed to get GAP interface");
        return BT_ERR_STATUS_FAIL;
    }
    rpt_size = strlen(ptr);
    hex_len = (strlen(ptr) + 1) / 2;

    BT_DBG_INFO(BT_DEBUG_GAP, "rpt_size=%ld, hex_len=%ld", (unsigned long)rpt_size, (unsigned long)hex_len);
    hex_bytes_filled = ascii_2_hex(ptr, hex_len, hex_buf);
    BT_DBG_INFO(BT_DEBUG_GAP, "hex_bytes_filled=%ld", (unsigned long)hex_bytes_filled);
    for (i=0;i<hex_len;i++)
    {
        BT_DBG_NOTICE(BT_DEBUG_GAP, "hex values= %02X",hex_buf[i]);
    }
    if (hex_bytes_filled)
    {
        ret = linuxbt_gap_ex_interface->send_hci((uint8_t*)hex_buf, hex_bytes_filled);
        BT_DBG_INFO(BT_DEBUG_GAP, "send_hci");
        return linuxbt_return_value_convert(ret);
    }
    else
    {
        BT_DBG_ERROR(BT_DEBUG_GAP, "hex_bytes_filled <= 0");
        return BT_ERR_STATUS_PARM_INVALID;
    }
    // return 0;

}

int linuxbt_gap_get_rssi_handler(char *pbt_addr)
{
    BT_DBG_NORMAL(BT_DEBUG_GAP, "%s()", __FUNCTION__);
    bt_bdaddr_t bdaddr;
    bt_status_t ret = BT_STATUS_SUCCESS;
    memset(&bdaddr, 0, sizeof(bt_bdaddr_t));

    if (NULL == linuxbt_gap_ex_interface)
    {
        BT_DBG_ERROR(BT_DEBUG_GAP, "Failed to get GAP extended interface");
        return BT_ERR_STATUS_FAIL;
    }
    linuxbt_btaddr_stoh(pbt_addr, &bdaddr);
    BT_DBG_NORMAL(BT_DEBUG_GAP, "BTADDR = %02X:%02X:%02X:%02X:%02X:%02X",
                  bdaddr.address[0], bdaddr.address[1], bdaddr.address[2],
                  bdaddr.address[3], bdaddr.address[4], bdaddr.address[5]);
    ret = linuxbt_gap_ex_interface->get_rssi(&bdaddr);
    return linuxbt_return_value_convert(ret);
}
#endif

int linuxbt_gap_enable_handler(void)
{
    FUNC_ENTRY;

    bt_status_t ret = BT_STATUS_SUCCESS;
    if (NULL == g_bt_interface)
    {
        BT_DBG_ERROR(BT_DEBUG_GAP, "Failed to get GAP interface");
        return BT_ERR_STATUS_FAIL;
    }
    ret = g_bt_interface->enable(false);
    return linuxbt_return_value_convert(ret);
}

int linuxbt_gap_disable_handler(void)
{
    FUNC_ENTRY;

    bt_status_t ret = BT_STATUS_SUCCESS;
    if (NULL == g_bt_interface)
    {
        BT_DBG_ERROR(BT_DEBUG_GAP, "Failed to get GAP interface");
        return BT_ERR_STATUS_FAIL;
    }
    ret = g_bt_interface->disable();
    return linuxbt_return_value_convert(ret);
}

int linuxbt_gap_get_adapter_properties_handler(void)
{
    FUNC_ENTRY;

    bt_status_t ret = BT_STATUS_SUCCESS;
    if (NULL == g_bt_interface)
    {
        BT_DBG_ERROR(BT_DEBUG_GAP, "Failed to get GAP interface");
        return BT_ERR_STATUS_FAIL;
    }
    ret = g_bt_interface->get_adapter_properties();
    return linuxbt_return_value_convert(ret);
}

int linuxbt_gap_set_device_name_handler(char *pname)
{
    bt_property_t property;
    bt_property_t *property_p;
    bt_status_t ret = BT_STATUS_SUCCESS;

    memset(&property, 0, sizeof(bt_property_t));
    if (NULL == pname)
    {
        BT_DBG_ERROR(BT_DEBUG_GAP, "null pointer of pname");
        return BT_ERR_STATUS_PARM_INVALID;
    }
    if (NULL == g_bt_interface)
    {
        BT_DBG_ERROR(BT_DEBUG_GAP, "Failed to get GAP interface");
        return BT_ERR_STATUS_FAIL;
    }

    property_p = &property;

    property_p->type = BT_PROPERTY_BDNAME;
    property_p->len = strlen(pname);
    property_p->val = pname;

    ret = g_bt_interface->set_adapter_property(property_p);
    return linuxbt_return_value_convert(ret);
}

int linuxbt_gap_set_scan_mode(int mode)
{
    bt_property_t property;
    bt_property_t *property_p;
    bt_scan_mode_t scan_mode;
    bt_status_t ret = BT_STATUS_SUCCESS;

    memset(&property, 0, sizeof(bt_property_t));
    scan_mode = (bt_scan_mode_t)mode;

    if (NULL == g_bt_interface)
    {
        BT_DBG_ERROR(BT_DEBUG_GAP, "Failed to get GAP interface");
        return BT_ERR_STATUS_FAIL;
    }
    property_p = &property;
    property_p->type = BT_PROPERTY_ADAPTER_SCAN_MODE;
    property_p->len = sizeof(bt_scan_mode_t);
    property_p->val = (void*)&scan_mode;

    ret = g_bt_interface->set_adapter_property(property_p);

    return linuxbt_return_value_convert(ret);
}

int linuxbt_gap_start_discovery_handler(void)
{
    FUNC_ENTRY;

    bt_status_t ret = BT_STATUS_SUCCESS;
    if (NULL == g_bt_interface)
    {
        BT_DBG_ERROR(BT_DEBUG_GAP, "Failed to get GAP interface");
        return BT_ERR_STATUS_FAIL;
    }
    ret = g_bt_interface->start_discovery();

    return linuxbt_return_value_convert(ret);
}

int linuxbt_gap_cancel_discovery_handler(void)
{
    FUNC_ENTRY;
    bt_status_t ret = BT_STATUS_SUCCESS;
    if (NULL == g_bt_interface)
    {
        BT_DBG_ERROR(BT_DEBUG_GAP, "Failed to get GAP interface");
        return BT_ERR_STATUS_FAIL;
    }
    ret = g_bt_interface->cancel_discovery();

    return linuxbt_return_value_convert(ret);
}

int linuxbt_gap_create_bond_handler(char *pbt_addr, int transport)
{
    FUNC_ENTRY;
    bt_bdaddr_t bdaddr;
    bt_status_t ret = BT_STATUS_SUCCESS;
    memset(&bdaddr, 0, sizeof(bt_bdaddr_t));

    if (NULL == pbt_addr)
    {
        BT_DBG_ERROR(BT_DEBUG_GAP, "null pointer of pbt_addr");
        return BT_ERR_STATUS_PARM_INVALID;
    }
    if (NULL == g_bt_interface)
    {
        BT_DBG_ERROR(BT_DEBUG_GAP, "Failed to get GAP interface");
        return BT_ERR_STATUS_FAIL;
    }
    linuxbt_btaddr_stoh(pbt_addr, &bdaddr);
    BT_DBG_NORMAL(BT_DEBUG_GAP, "start pairing to %02X:%02X:%02X:%02X:%02X:%02X",
                  bdaddr.address[0], bdaddr.address[1], bdaddr.address[2],
                  bdaddr.address[3], bdaddr.address[4], bdaddr.address[5]);

    ret = g_bt_interface->create_bond(&bdaddr, transport);

    return linuxbt_return_value_convert(ret);
}

int linuxbt_gap_remove_bond_handler(char *pbt_addr)
{
    FUNC_ENTRY;

    bt_bdaddr_t bdaddr;
    bt_status_t ret = BT_STATUS_SUCCESS;

    memset(&bdaddr, 0, sizeof(bt_bdaddr_t));
    if (NULL == pbt_addr)
    {
        BT_DBG_ERROR(BT_DEBUG_GAP, "null pointer of pbt_addr");
        return BT_ERR_STATUS_PARM_INVALID;
    }
    if (NULL == g_bt_interface)
    {
        BT_DBG_ERROR(BT_DEBUG_GAP, "Failed to get GAP interface");
        return BT_ERR_STATUS_FAIL;
    }
    linuxbt_btaddr_stoh(pbt_addr, &bdaddr);
    BT_DBG_NORMAL(BT_DEBUG_GAP, "unpair to %02X:%02X:%02X:%02X:%02X:%02X",
                  bdaddr.address[0], bdaddr.address[1], bdaddr.address[2],
                  bdaddr.address[3], bdaddr.address[4], bdaddr.address[5]);

    ret = g_bt_interface->remove_bond(&bdaddr);

    return linuxbt_return_value_convert(ret);
}

int linuxbt_gap_config_hci_snoop_log_handler(unsigned char enable)
{
    FUNC_ENTRY;

    bt_status_t ret = BT_STATUS_SUCCESS;

    if (NULL == g_bt_interface)
    {
        BT_DBG_ERROR(BT_DEBUG_GAP, "Failed to get GAP interface");
        return BT_ERR_STATUS_FAIL;
    }
    ret = g_bt_interface->config_hci_snoop_log(enable);

    return linuxbt_return_value_convert(ret);
}

VOID linuxbt_parse_device_properties(BLUETOOTH_DEVICE *device,
                                                   int num_properties,
                                                   bt_property_t *properties)
{
    FUNC_ENTRY;
    bt_property_t *property;
    CHAR *name;
    bt_bdaddr_t* btaddr;
    bt_uuid_t *uuid;
    UINT8 bonded_dev_num = 0;

    BT_CHECK_POINTER_RETURN(BT_DEBUG_COMM, properties);
    //BT_DBG_NORMAL(BT_DEBUG_GAP, "======Receive Properties pointer = : %p=======",p_msg->properties);
    BT_DBG_NORMAL(BT_DEBUG_GAP, "============Propertie num : %d================",num_properties);
    for (UINT8 i = 0; i < num_properties; i++)
    {
        property = &properties[i];
        switch (property->type)
        {
        case BT_PROPERTY_BDNAME:
            name = (CHAR *)property->val;
            //BT_DBG_NORMAL(BT_DEBUG_GAP, "Propertie name raw data : 0x%X,0x%X,0x%X,0x%X,0x%X,0x%X,0x%X,0x%X,0x%X,",name,name+1,name+2,name+3,name+4,name+5,name+6,name+7,name+8);
            if (strlen(name) > 0)
            {
                strncpy(device->name, name, property->len);
                device->name[property->len] = '\0';
                BT_DBG_NORMAL(BT_DEBUG_GAP, "[GAP] type = %ld, len = %ld, bdname = %s",
                                          (long)property->type,
                                          (long)property->len, device->name);
            }
            else
            {
                BT_DBG_NORMAL(BT_DEBUG_GAP, "[GAP] type = %ld, len = %ld, bdname is null",
                                          (long)property->type,
                                          (long)property->len);
            }
            break;
        case BT_PROPERTY_BDADDR:
            btaddr = (bt_bdaddr_t *)property->val;
            linuxbt_btaddr_htos(btaddr, device->bdAddr);
            BT_DBG_NORMAL(BT_DEBUG_GAP, "[GAP] type = %ld, len = %ld, bdaddr = %s",
                                       (long)property->type, (long)property->len,
                                       device->bdAddr);
            break;
        case BT_PROPERTY_CLASS_OF_DEVICE:
            device->cod= *((UINT32 *)(property->val));
            BT_DBG_NORMAL(BT_DEBUG_GAP, "[GAP] type = %ld, len = %ld, cod = 0x%02X",
                                        (long)property->type, (long)property->len,
                                        (UINT32)(device->cod));
            break;
        case BT_PROPERTY_REMOTE_RSSI:
            device->rssi = *((INT16*)(property->val));
            BT_DBG_NORMAL(BT_DEBUG_GAP, "[GAP] type = %ld, len = %ld, rssi = 0x%02X",
                                        (long)property->type,(long)property->len,
                                        device->rssi);
            break;
        case BT_PROPERTY_TYPE_OF_DEVICE:
            device->devicetype = *((UINT32 *)(property->val));
            BT_DBG_NORMAL(BT_DEBUG_GAP, "[GAP] type = %ld, len = %ld, devtype = 0x%02X",
                (long)property->type, (long)property->len, device->devicetype);
            break;
        case BT_PROPERTY_ADAPTER_SCAN_MODE:
            BT_DBG_INFO(BT_DEBUG_GAP, "[GAP] type = %ld, len = %ld, scan mode = %ld",
                (long)property->type, (long)property->len, *((UINT32 *)(property->val)));
            break;
        case BT_PROPERTY_ADAPTER_DISCOVERY_TIMEOUT:
            BT_DBG_INFO(BT_DEBUG_GAP, "[GAP] type = %ld, len = %ld, disc_timeout = %ld",
                (long)property->type, (long)property->len, *((UINT32 *)(property->val)));
            break;
        case BT_PROPERTY_UUIDS:
            uuid = (bt_uuid_t*)property->val;
            UINT8 len = property->len;
            for (UINT8 j=0; j<len; j+=16)
            {
                BT_DBG_INFO(BT_DEBUG_GAP, "[GAP] type = %ld, len = %ld, uuid = %02X%02X%02X%02X-%02X%02X-%02X%02X-%02X%02X-%02X%02X%02X%02X%02X%02X",
                                (long)property->type, (long)property->len,
                                uuid->uu[j+0], uuid->uu[j+1], uuid->uu[j+2], uuid->uu[j+3],
                                uuid->uu[j+4], uuid->uu[j+5], uuid->uu[j+6], uuid->uu[j+7],
                                uuid->uu[j+8], uuid->uu[j+9], uuid->uu[j+10], uuid->uu[j+11],
                                uuid->uu[j+12], uuid->uu[j+13], uuid->uu[j+14], uuid->uu[j+15]
                               );
            }
            break;
        case BT_PROPERTY_ADAPTER_BONDED_DEVICES:
            bonded_dev_num = property->len / sizeof(bt_bdaddr_t);
            btaddr = (bt_bdaddr_t *)property->val;
            BT_DBG_INFO(BT_DEBUG_GAP, "[GAP] type = %ld, len = %ld", (long)property->type, (long)property->len);
            for (UINT8 k=0; k<bonded_dev_num; k++)
            {
                BT_DBG_INFO(BT_DEBUG_GAP, "[GAP] bonded_addr = %02X:%02X:%02X:%02X:%02X:%02X",
                          btaddr[k].address[0], btaddr[k].address[1], btaddr[k].address[2],
                          btaddr[k].address[3], btaddr[k].address[4], btaddr[k].address[5]);
            }
            break;
        default:
            BT_DBG_INFO(BT_DEBUG_GAP, "[GAP] type = %ld, len = %ld, Others",
                (long)property->type, (long)property->len);
            break;
        }
    }
    BT_DBG_NORMAL(BT_DEBUG_GAP, "============Properties End================");
}

void linuxbt_gap_properties_cb(bt_status_t status,
                               int num_properties,
                               bt_property_t *properties)
{

    tBTMW_MSG msg = {0};
    BT_DBG_NORMAL(BT_DEBUG_GAP, " status: %ld", (long)status);

    linuxbt_parse_device_properties(&(msg.data.device_info.device), num_properties, properties);
    msg.data.device_info.device_kind = BT_DEVICE_LOCAL;
    msg.hdr.event = BTMW_GAP_DEVICE_INFO_EVT;
    msg.hdr.len = sizeof(tBTMW_GAP_DEVICE_INFO);
    linuxbt_send_msg(&msg);
}

void linuxbt_gap_remote_device_properties_cb(bt_status_t status,
        bt_bdaddr_t *bd_addr,
        int num_properties,
        bt_property_t *properties)
{
    tBTMW_MSG msg = {0};
    BT_DBG_NORMAL(BT_DEBUG_GAP, " status: %ld", (long)status);

    linuxbt_parse_device_properties(&(msg.data.device_info.device), num_properties, properties);
    if(strlen(msg.data.device_info.device.bdAddr) == 0)
    {
        linuxbt_btaddr_htos(bd_addr, msg.data.device_info.device.bdAddr);
    }
    BT_DBG_NORMAL(BT_DEBUG_GAP, "status:%d, device:%s", status, msg.data.device_info.device.bdAddr);

    msg.data.device_info.device_kind = BT_DEVICE_BONDED;
    msg.hdr.event = BTMW_GAP_DEVICE_INFO_EVT;
    msg.hdr.len = sizeof(tBTMW_GAP_DEVICE_INFO);

    if(strlen(msg.data.device_info.device.bdAddr) || strlen(msg.data.device_info.device.name))
    {
        linuxbt_send_msg(&msg);
    }
}

void linuxbt_gap_device_found_cb(int num_properties,
                                 bt_property_t *properties)
{
    tBTMW_MSG msg = {0};
    BT_DBG_NORMAL(BT_DEBUG_GAP, "device found");

    linuxbt_parse_device_properties(&(msg.data.device_info.device), num_properties, properties);
    msg.data.device_info.device_kind = BT_DEVICE_SCAN;
    msg.hdr.event = BTMW_GAP_DEVICE_INFO_EVT;
    msg.hdr.len = sizeof(tBTMW_GAP_DEVICE_INFO);
    linuxbt_send_msg(&msg);

}


void linuxbt_gap_pin_request_cb(bt_bdaddr_t *remote_bd_addr,
                                bt_bdname_t *bd_name, uint32_t cod,bool min_16_digit)
{
    BT_DBG_NORMAL(BT_DEBUG_GAP, "%s()", __FUNCTION__);

    bt_pin_code_t pin;
    char str_pin[16];
    uint32_t pin_code;
    bt_status_t ret = BT_STATUS_SUCCESS;
    uint8_t fg_accept = 1;

    if (NULL == g_bt_interface)
    {
        BT_DBG_ERROR(BT_DEBUG_GAP, "Failed to get GAP interface");
        return;
    }
    memset(&pin, 0, 16);
    pin_code = 100000 + (uint32_t)(random() % 900000 - 1);
    snprintf(str_pin, sizeof(str_pin), "%d", pin_code);
    memcpy(pin.pin, str_pin, sizeof(pin.pin));

    bt_gap_get_pin_code_cb(&pin, (UINT8 *)&fg_accept);

    ret = g_bt_interface->pin_reply(remote_bd_addr, (uint8_t)fg_accept, 6, &pin);
    if (BT_SUCCESS != linuxbt_return_value_convert(ret))
    {
        BT_DBG_ERROR(BT_DEBUG_GAP, "pin_reply error!\n");
    }
}


void linuxbt_gap_ssp_request_cb(bt_bdaddr_t *remote_bd_addr,
                                bt_bdname_t *bd_name, uint32_t cod,
                                bt_ssp_variant_t pairing_variant,
                                uint32_t passkey)
{
    BT_DBG_NORMAL(BT_DEBUG_GAP, "%s()", __FUNCTION__);
    bt_status_t ret = BT_STATUS_SUCCESS;
    uint8_t fg_accept = 1;

    if (remote_bd_addr)
    {
        bt_bdaddr_t *btaddr = remote_bd_addr;

        BT_DBG_INFO(BT_DEBUG_GAP, "REMOTE BDADDR = %02X:%02X:%02X:%02X:%02X:%02X",
                      btaddr->address[0], btaddr->address[1], btaddr->address[2],
                      btaddr->address[3], btaddr->address[4], btaddr->address[5]);
    }
    else
    {
        BT_DBG_ERROR(BT_DEBUG_GAP, "remote_bd_addr is NULL!");
    }
    if (NULL == g_bt_interface)
    {
        BT_DBG_ERROR(BT_DEBUG_GAP, "Failed to get GAP interface");
        return;
    }
    if (bd_name)
    {
        BT_DBG_NORMAL(BT_DEBUG_GAP, "BDNAME = %s", bd_name->name);
    }
    BT_DBG_INFO(BT_DEBUG_GAP, "cod = 0x%08X, pairing_variant = %ld, passkey = %ld.", cod, (long)pairing_variant, (unsigned long)passkey);
    BT_DBG_INFO(BT_DEBUG_GAP, "passkey = %ld.", (unsigned long)passkey);

    bt_gap_get_passkey_cb(passkey, (UINT8 *)&fg_accept);
    ret = g_bt_interface->ssp_reply(remote_bd_addr, pairing_variant, fg_accept, passkey);
    if (BT_SUCCESS != linuxbt_return_value_convert(ret))
    {
        BT_DBG_ERROR(BT_DEBUG_GAP, "ssp_reply error!\n");
    }
}

void linuxbt_gap_state_changed_cb(bt_state_t state)
{
    tBTMW_MSG msg = {0};
    BT_DBG_NORMAL(BT_DEBUG_GAP, "%s() state: %ld", __FUNCTION__, (long)state);

    switch (state)
    {
    case BT_STATE_OFF:
        BT_DBG_NOTICE(BT_DEBUG_GAP, "BT STATE OFF");
        msg.hdr.event = BTMW_GAP_STATE_EVT;
        msg.hdr.len = sizeof(tBTMW_GAP_STATE);
        msg.data.gap_state.state = GAP_STATE_OFF;

        linuxbt_send_msg(&msg);
        break;

    case BT_STATE_ON:
        BT_DBG_NOTICE(BT_DEBUG_GAP, "BT STATE ON");
        msg.hdr.event = BTMW_GAP_STATE_EVT;
        msg.hdr.len = sizeof(tBTMW_GAP_STATE);
        msg.data.gap_state.state = GAP_STATE_ON;
        linuxbt_send_msg(&msg);
        break;

    default:
        break;
    }
}

void linuxbt_gap_discovery_state_changed_cb(bt_discovery_state_t state)
{
    tBTMW_MSG msg = {0};
    BT_DBG_NORMAL(BT_DEBUG_GAP, " state: %ld", (long)state);

    switch (state)
    {
    case BT_DISCOVERY_STOPPED:
        BT_DBG_NOTICE(BT_DEBUG_GAP, "BT Search Device Stop.");
        msg.hdr.event = BTMW_GAP_STATE_EVT;
        msg.hdr.len = sizeof(tBTMW_GAP_STATE);
        msg.data.gap_state.state = GAP_STATE_DISCOVERY_STOPED;
        linuxbt_send_msg(&msg);
        break;

    case BT_DISCOVERY_STARTED:
        BT_DBG_NOTICE(BT_DEBUG_GAP, "BT Search Device Start...");
        msg.hdr.event = BTMW_GAP_STATE_EVT;
        msg.hdr.len = sizeof(tBTMW_GAP_STATE);
        msg.data.gap_state.state = GAP_STATE_DISCOVERY_STARTED;
        linuxbt_send_msg(&msg);
        break;
    default:
        break;
    }
}

void linuxbt_gap_bond_state_changed_cb(bt_status_t status,
                                       bt_bdaddr_t *remote_bd_addr,
                                       bt_bond_state_t state)
{
    tBTMW_MSG msg = {0};
    BT_DBG_NORMAL(BT_DEBUG_GAP, "%s(), status = %ld, state = %ld", __FUNCTION__, (long)status, (long)state);

    switch (status)
    {
    case BT_STATUS_SUCCESS:
        BT_DBG_NOTICE(BT_DEBUG_GAP, "BT bond status is successful(%ld), ", (long)status);
        break;
    default:
        BT_DBG_NOTICE(BT_DEBUG_GAP, "BT bond status is failed(%ld), ", (long)status);
        break;
    }

    switch (state)
    {
    case BT_BOND_STATE_NONE:
        msg.data.gap_state.state = GAP_STATE_NO_BOND;
        BT_DBG_NOTICE(BT_DEBUG_GAP, "state is none.");
        break;
    case BT_BOND_STATE_BONDING:
        msg.data.gap_state.state = GAP_STATE_BONDING;
        BT_DBG_NOTICE(BT_DEBUG_GAP, "state is bonding.");
        break;
    case BT_BOND_STATE_BONDED:
        msg.data.gap_state.state = GAP_STATE_BONDED;
        BT_DBG_NOTICE(BT_DEBUG_GAP, "state is bonded.");
        break;
    default:
        break;
    }

    if (remote_bd_addr)
    {
        bt_bdaddr_t *btaddr = remote_bd_addr;
        BT_DBG_NOTICE(BT_DEBUG_GAP, "REMOTE BDADDR = %02X:%02X:%02X:%02X:%02X:%02X",
                          btaddr->address[0], btaddr->address[1], btaddr->address[2],
                          btaddr->address[3], btaddr->address[4], btaddr->address[5]);
        linuxbt_btaddr_htos(btaddr, msg.data.gap_state.bd_addr);
    }
    else
    {
        BT_DBG_ERROR(BT_DEBUG_GAP, "remote_bd_addr is NULL!");
    }

    msg.hdr.event = BTMW_GAP_STATE_EVT;
    msg.hdr.len = sizeof(tBTMW_GAP_STATE);
    linuxbt_send_msg(&msg);

}

void linuxbt_gap_acl_state_changed_cb(bt_status_t status, bt_bdaddr_t *remote_bd_addr,
                                      bt_acl_state_t state)
{
    tBTMW_MSG msg = {0};
    BT_DBG_NORMAL(BT_DEBUG_GAP, "status = %ld, state = %ld",  (long)status, (long)state);

    switch (status)
    {
    case BT_STATUS_SUCCESS:
        BT_DBG_NOTICE(BT_DEBUG_GAP, "BT bond status is successful(%ld), ", (long)status);
        break;
    default:
        BT_DBG_NOTICE(BT_DEBUG_GAP, "BT bond status is failed(%ld), ", (long)status);
        break;
    }
    //bt_gap_state.status = status;

    switch (state)
    {
    case BT_ACL_STATE_CONNECTED:
        msg.data.gap_state.state = GAP_STATE_ACL_CONNECTED;
        BT_DBG_NOTICE(BT_DEBUG_GAP, "acl is connected.");

        break;
    case BT_ACL_STATE_DISCONNECTED:
        BT_DBG_NOTICE(BT_DEBUG_GAP, "acl is disconnected.");
        msg.data.gap_state.state = GAP_STATE_ACL_DISCONNECTED;
        break;
    default:
        break;
    }

    if (remote_bd_addr)
    {
        bt_bdaddr_t *btaddr = remote_bd_addr;
        linuxbt_btaddr_htos(btaddr, msg.data.gap_state.bd_addr);
        BT_DBG_NORMAL(BT_DEBUG_GAP, "REMOTE BDADDR = %02X:%02X:%02X:%02X:%02X:%02X",
                          btaddr->address[0], btaddr->address[1], btaddr->address[2],
                          btaddr->address[3], btaddr->address[4], btaddr->address[5]);
    }
    else
    {
        BT_DBG_ERROR(BT_DEBUG_GAP, "remote_bd_addr is NULL!");
    }

    msg.hdr.event = BTMW_GAP_STATE_EVT;
    msg.hdr.len = sizeof(tBTMW_GAP_STATE);
    linuxbt_send_msg(&msg);

}

void linuxbt_gap_acl_disconnect_reason_callback(bt_bdaddr_t *remote_bd_addr, uint8_t reason)
{
    tBTMW_MSG msg = {0};
    if (NULL == remote_bd_addr)
    {
        BT_DBG_ERROR(BT_DEBUG_GAP, "null pointer of remote_bd_addr");
        return;
    }

    BT_DBG_NORMAL(BT_DEBUG_GAP, "REMOTE BDADDR = %02X:%02X:%02X:%02X:%02X:%02X",
                remote_bd_addr->address[0], remote_bd_addr->address[1], remote_bd_addr->address[2],
                remote_bd_addr->address[3], remote_bd_addr->address[4], remote_bd_addr->address[5]);
    BT_DBG_NORMAL(BT_DEBUG_GAP, "disconnect reason = %ld",(unsigned long)reason);

    linuxbt_btaddr_htos(remote_bd_addr, msg.data.gap_state.bd_addr);
    msg.data.gap_state.state = GAP_STATE_ACL_DISCONNECTED;
    msg.data.gap_state.reason = reason ;
    msg.hdr.event = BTMW_GAP_STATE_EVT;
    msg.hdr.len = sizeof(tBTMW_GAP_STATE);
    linuxbt_send_msg(&msg);

}



void linuxbt_gap_get_rssi_cb(bt_status_t status, bt_bdaddr_t *remote_bd_addr , int rssi_value)
{
    BT_DBG_NORMAL(BT_DEBUG_GAP, "%s()  %ld ", __FUNCTION__, (long)(rssi_value));
    bt_gap_get_rssi_result_cb(rssi_value);
}

const void *linuxbt_gap_get_profile_interface(const char *profile_id)
{
    if (NULL != g_bt_interface)
    {
        return g_bt_interface->get_profile_interface(profile_id);
    }
    else
    {
        BT_DBG_ERROR(BT_DEBUG_GAP, "Failed to get GAP interface");
        return NULL;
    }
}

int linuxbt_gap_init(void)
{
    FUNC_ENTRY;

    bt_status_t ret = BT_STATUS_SUCCESS;
    // Init bluetooth interface.
    open_bluetooth_stack(NULL, "Stack Linux", (struct hw_device_t**)&g_bt_device);
    if (NULL == g_bt_device)
    {
        BT_DBG_ERROR(BT_DEBUG_GAP, "Failed to open Bluetooth stack.");
        return BT_ERR_STATUS_NOT_READY;
    }

    g_bt_interface = g_bt_device->get_bluetooth_interface();
    if (NULL == g_bt_interface)
    {
        BT_DBG_ERROR(BT_DEBUG_GAP, "Failed to get Bluetooth interface");
        return BT_ERR_STATUS_FAIL;
    }
    //g_bt_interface->set_os_callouts(&g_callouts);

    ret = g_bt_interface->init(&g_bt_callbacks);
    if (BT_SUCCESS != linuxbt_return_value_convert(ret))
    {
        BT_DBG_ERROR(BT_DEBUG_GAP, "gap init error!");
    }

#if defined(MTK_LINUX_GAP) && (MTK_LINUX_GAP == TRUE)
    linuxbt_gap_ex_interface = (btgap_ex_interface_t *) linuxbt_gap_get_profile_interface(BT_PROFILE_GAP_EX_ID);
    if (NULL == linuxbt_gap_ex_interface)
    {
        BT_DBG_ERROR(BT_DEBUG_GAP, "Failed to get Bluetooth extended interface");
        return BT_ERR_STATUS_FAIL;
    }

    ret = linuxbt_gap_ex_interface->init(&linuxbt_gap_ex_callbacks);
    if (BT_SUCCESS != linuxbt_return_value_convert(ret))
    {
        BT_DBG_ERROR(BT_DEBUG_GAP, "gap init error!");
    }
#endif

    return linuxbt_return_value_convert(ret);
}

int linuxbt_gap_deinit(void)
{
    //bt_status_t ret = BT_STATUS_SUCCESS;

    if (NULL != g_bt_interface)
    {
        //g_bt_interface->disable();
        g_bt_interface->cleanup();
    }
    else
    {
        BT_DBG_ERROR(BT_DEBUG_GAP, "Failed to get GAP interface");
        return BT_ERR_STATUS_FAIL;
    }


    return BT_SUCCESS;//linuxbt_return_value_convert(ret);
}

int linuxbt_gap_set_bt_wifi_ratio(uint8_t bt_ratio, uint8_t wifi_ratio)
{
    uint8_t cmd[5] = {0};

    cmd[0] = 0xf1;
    cmd[1] = 0xfc;
    cmd[2] = 0x02;
    cmd[3] = bt_ratio;
    cmd[4] = wifi_ratio;

#if defined(MTK_LINUX_GAP) && (MTK_LINUX_GAP == TRUE)
    linuxbt_gap_ex_interface->send_hci(cmd, sizeof(cmd));
#endif

    return BT_SUCCESS;
}

#if defined(MTK_LINUX_GAP) && (MTK_LINUX_GAP == TRUE)

int linuxbt_interop_database_add(uint16_t feature, bt_bdaddr_t *remote_bd_addr,size_t len)
{
    if(NULL == g_bt_interface)
    {
        BT_DBG_ERROR(BT_DEBUG_GAP, "Failed to get GAP interface");
        return BT_ERR_STATUS_FAIL;
    }
    g_bt_interface->interop_database_add(feature, remote_bd_addr,len);
    return BT_SUCCESS;
}

int linuxbt_interop_database_clear()
{
    if(NULL == g_bt_interface)
    {
        BT_DBG_ERROR(BT_DEBUG_GAP, "Failed to get GAP interface");
        return BT_ERR_STATUS_FAIL;
    }
    g_bt_interface->interop_database_clear();
    return BT_SUCCESS;
}

#endif

