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
#include <stdlib.h>
#include <stdio.h>
#include <hardware/bluetooth.h>
#include <hardware/bt_gatt.h>

#include "btut_cli.h"
#include "btut_debug.h"
#include "btut_gap_if.h"
#include "btut_gatt_if.h"
#include "btut_gattc_if.h"
#include "btut_gatts_if.h"

extern const void *btut_gap_get_profile_interface(const char *profile_id);

static btgatt_interface_t *btut_gatt_interface = NULL;
extern const btgatt_client_callbacks_t btut_gattc_callbacks;
extern const btgatt_server_callbacks_t btut_gatts_callbacks;
const btgatt_callbacks_t btut_gatt_callbacks =
{
        sizeof(btgatt_callbacks_t),
        &btut_gattc_callbacks,
        &btut_gatts_callbacks,
};

void btut_gatt_btaddr_stoh(char *btaddr_s, bt_bdaddr_t *bdaddr_h)
{
    int i;

    for (i = 0; i <6; i++)
    {
        bdaddr_h->address[i] = strtoul(btaddr_s, &btaddr_s, 16);
        btaddr_s++;
    }
}

void btut_gatt_uuid_stoh(char *uuid_s,  bt_uuid_t *uuid)
{
    int i = 0,j = 0;
    int size = strlen(uuid_s);
    char temp[3];
    temp[2] = '\0';
    while (i < size)
    {
        if (uuid_s[i] == '-' || uuid_s[i] == '\0')
        {
            i++;
            continue;
        }
        temp[0] = uuid_s[i];
        temp[1] = uuid_s[i+1];
        uuid->uu[j] = strtoul(temp, NULL, 16);
        i+=2;
        j++;
    }

    if (size <= 8)
    {
        // 16bits uuid or 32bits uuid
        if (size == 4)
        {
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

bool btut_gatt_is_uuid_equal(bt_uuid_t *uuid1,  bt_uuid_t *uuid2)
{
    for (int i = 0; i < 16 ; i++)
    {
        if (uuid1->uu[i]!=uuid2->uu[i]) return false;
    }
    return true;
}

void btut_gatt_print_uuid (bt_uuid_t* uuid, char* uuid_s)
{
    int i;
    char *ptr;
    ptr = uuid_s;
    for (i = 0 ; i < 16 ; i++)
    {
        sprintf(ptr, "%02X", uuid->uu[i]);
        ptr+=2;
        if (i == 3 || i == 5 || i == 7 || i == 9)
        {
            *ptr = '-';
            ptr++;
        }
    }
    *(ptr+1) = '\0';
}

void btut_gatt_decode_adv_data (uint8_t* adv_data, btut_gatt_adv_data_t *parse_data)
{
    uint8_t* ptr = adv_data;
    unsigned char count = 0;
    while (1)
    {
        char length = *ptr;
        if (length == 0) break;
        unsigned char type = *(ptr+1);
        unsigned char value_len = length-1;
        unsigned char* value = (unsigned char*)malloc(value_len);
        memcpy(value, ptr+2, value_len);
        if (count <= 10)
        {
            parse_data->data[count].type = type;
            parse_data->data[count].len= value_len;
            parse_data->data[count].value= value;
        }
        ptr = ptr+length+1;
        switch (type)
        {
            case 0x01: //Flags
                BTUT_Logi("[GATT] Flags : %02X\n",value[0]);
                break;
            case 0x02: //16-bit uuid
            case 0x03: //16-bit uuid
                {
                    char temp[value_len*2+1];
                    int i = 0;
                    int j = 0;
                    for (i = value_len-1 ; i >= 0 ; i--)
                    {
                        sprintf(&temp[j*2],"%02X",value[i]);
                        j++;
                    }
                    BTUT_Logi("[GATT] 16-bit Service Class length: %d UUIDs : %s\n",value_len,temp);
                }
                break;
            case 0x04: //32-bit uuid
            case 0x05: //32-bit uuid
                {
                    char temp[value_len*2+1];
                    int i = 0;
                    int j = 0;
                    for (i = value_len-1 ; i >= 0 ; i--)
                    {
                        sprintf(&temp[j*2],"%02X",value[i]);
                        j++;
                    }
                    BTUT_Logi("[GATT] 32-bit Service Class length: %d UUIDs : %s\n",value_len,temp);
                }
                break;
            case 0x06: //128-bit uuid
            case 0x07: //128-bit uuid
                {
                    char temp[value_len*2+1];
                    int i = 0;
                    int j = 0;
                    for (i = value_len-1 ; i >= 0 ; i--)
                    {
                        sprintf(&temp[j*2],"%02X",value[i]);
                        j++;
                    }
                    BTUT_Logi("[GATT] 128-bit Service Class length: %d UUIDs : %s\n",value_len,temp);
                }
                break;
            case 0x08: //Shortened Local Name
                BTUT_Logi("[GATT] Shortened Local length: %d Name : %s\n",value_len,value);
                break;
            case 0x09: //Complete Local Name
                BTUT_Logi("[GATT] Complete Local length: %d Name : %s\n",value_len,value);
                break;
            case 0x0A: //Tx Power Level
                BTUT_Logi("[GATT] Tx Power Level : %d\n",value[0]);
                break;
            case 0x1B: //LE Bluetooth Device Address
                {
                    BTUT_Logi("[GATT] LE Bluetooth Device Address : %02X:%02X:%02X:%02X:%02X:%02X\n",
                    value[5], value[4], value[3],
                    value[2], value[1], value[0]);
                }
                break;
            case 0xFF: //Manufacturer Specific Data
                {
                    char temp[value_len*2+1];
                    int i = 0;
                    int j = 0;
                    for (i = value_len-1 ; i >= 0 ; i--)
                    {
                        sprintf(&temp[j*2],"%02X",value[i]);
                        j++;
                    }
                    BTUT_Logi("[GATT] Manufacturer Specific Data : %s\n",temp);
                }
                break;
            default:
                {
                    char temp[length*2];
                    int i = 0;
                    for (i = 0 ; i < length ; i++)
                    {
                        sprintf(&temp[i*2],"%02X",value[i]);
                    }
                    BTUT_Logi("[GATT] Type:%02X Length:%d Data:%s\n",type,length,temp);
                }
                break;
        }
        count++;
    }
}

int btut_gatt_init()
{
    BTUT_Logd("[GATTC] %s()\n", __func__);
    int ret = 0;
    //BTUT_MOD hid_mod = {0};

    // Get GATT interface
    btut_gatt_interface = (btgatt_interface_t *) btut_gap_get_profile_interface(BT_PROFILE_GATT_ID);
    if (btut_gatt_interface == NULL)
    {
        BTUT_Loge("[GATT] Failed to get GATT interface\n");
        return -1;
    }

     // Init GATT interface
    if (btut_gatt_interface->init(&btut_gatt_callbacks) != BT_STATUS_SUCCESS)
    {
        BTUT_Loge("[GATT] Failed to init GATT interface\n");
        return -1;
    }
    if ( btut_gattc_init(btut_gatt_interface->client) != 0)
    {
        BTUT_Loge("[GATT] Failed to init GATT client interface\n");
        return -1;
    }
    if ( btut_gatts_init(btut_gatt_interface->server) != 0)
    {
        BTUT_Loge("[GATT] Failed to init GATT server interface\n");
        return -1;
    }
    return ret;
}

int btut_gatt_deinit()
{
    BTUT_Logd("[GATT] %s()\n", __func__);
    // Deinit GATT interface
    if (btut_gatt_interface != NULL)
    {
        btut_gatt_interface->cleanup();
        btut_gattc_deinit();
        //btut_gatts_deinit();
    }
    return 0;
}

