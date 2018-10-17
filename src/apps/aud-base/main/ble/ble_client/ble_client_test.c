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
 * MediaTek Inc. (C) 2014. All rights reserved.
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
#include <string.h>
#include <stdio.h>
#include<unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <pthread.h>
#include <sys/time.h>


/* application level */
#include "u_aee.h"
#include "u_cli.h"
#include "u_dbg.h"
#include "u_os.h"
#include "u_appman.h"
#include "u_sm.h"
#include "u_app_thread.h"
#include "u_amb.h"
#include "u_am.h"


/*    btmw   */
#include "mtk_bt_service_gattc_wrapper.h"

/* private */
#include "ble_client_test.h"

static VOID _ble_test_gatt_decode_adv_data (UINT8* adv_data, BT_GATT_ADV_DATA_T *parse_data)
{
    UINT8* ptr = adv_data;
    UCHAR count = 0;
    while (1)
    {
        CHAR length = *ptr;
        if (length == 0) break;
        UCHAR type = *(ptr+1);
        UCHAR value_len = length-1;
        UCHAR* value = (UCHAR*)malloc(value_len);
        memcpy(value, ptr+2, value_len);
        if (count < 10)
        {
            parse_data->data[count].type = type;
            parse_data->data[count].len= value_len;
            parse_data->data[count].value= value;
        }
        ptr = ptr+length+1;
        switch (type)
        {
            case 0x01: //Flags
                BLE_LOG(BLE_LOG_INFO," Flags : %02X\n",value[0]);
                break;
            case 0x02: //16-bit uuid
            case 0x03: //16-bit uuid
                {
                    CHAR temp[value_len*2+1];
                    int i = 0;
                    int j = 0;
                    for (i = value_len-1 ; i >= 0 ; i--)
                    {
                        sprintf(&temp[j*2],"%02X",value[i]);
                        j++;
                    }
                    BLE_LOG(BLE_LOG_INFO," 16-bit Service Class length: %d UUIDs : %s\n",value_len,temp);
                }
                break;
            case 0x04: //32-bit uuid
            case 0x05: //32-bit uuid
                {
                    CHAR temp[value_len*2+1];
                    int i = 0;
                    int j = 0;
                    for (i = value_len-1 ; i >= 0 ; i--)
                    {
                        sprintf(&temp[j*2],"%02X",value[i]);
                        j++;
                    }
                    BLE_LOG(BLE_LOG_INFO," 32-bit Service Class length: %d UUIDs : %s\n",value_len,temp);
                }
                break;
            case 0x06: //128-bit uuid
            case 0x07: //128-bit uuid
                {
                    CHAR temp[value_len*2+1];
                    int i = 0;
                    int j = 0;
                    for (i = value_len-1 ; i >= 0 ; i--)
                    {
                        sprintf(&temp[j*2],"%02X",value[i]);
                        j++;
                    }
                    BLE_LOG(BLE_LOG_INFO," 128-bit Service Class length: %d UUIDs : %s\n",value_len,temp);
                }
                break;
            case 0x08: //Shortened Local Name
                BLE_LOG(BLE_LOG_INFO," Shortened Local length: %d Name : %s\n",value_len,value);
                break;
            case 0x09: //Complete Local Name
                BLE_LOG(BLE_LOG_INFO," Complete Local length: %d Name : %s\n",value_len,value);
                break;
            case 0x0A: //Tx Power Level
                BLE_LOG(BLE_LOG_INFO," Tx Power Level : %d\n",value[0]));
                break;
            case 0x1B: //LE Bluetooth Device Address
                {
                    BLE_LOG(BLE_LOG_INFO," LE Bluetooth Device Address : %02X:%02X:%02X:%02X:%02X:%02X\n",
                    value[5], value[4], value[3],
                    value[2], value[1], value[0]);
                }
                break;
            case 0xFF: //Manufacturer Specific Data
                {
                    CHAR temp[value_len*2+1];
                    int i = 0;
                    int j = 0;
                    for (i = value_len-1 ; i >= 0 ; i--)
                    {
                        sprintf(&temp[j*2],"%02X",value[i]);
                        j++;
                    }
                    BLE_LOG(BLE_LOG_INFO," Manufacturer Specific Data : %s\n",temp);
                }
                break;
            default:
                {
                    CHAR temp[length*2];
                    int i = 0;
                    for (i = 0 ; i < length ; i++)
                    {
                        sprintf(&temp[i*2],"%02X",value[i]);
                    }
                    BLE_LOG(BLE_LOG_INFO," Type:%02X Length:%d Data:%s\n",type,length,temp);
                }
                break;
        }
        count++;
    }
}

VOID ble_test_gattc_scan_result_callback(BT_GATTC_SCAN_RST_T *pt_scan_result,
                                                                  VOID* pv_tag)
{
    if (NULL == pt_scan_result)
    {
        BLE_LOG(BLE_LOG_INFO," pt_scan_result is NULL!!\r\n"));
        return;
    }
    BLE_LOG(BLE_LOG_INFO," pt_scan_result->btaddr =%s\n", pt_scan_result->btaddr);

    BT_GATT_ADV_DATA_T adv_data;
    _ble_gatt_decode_adv_data(pt_scan_result->adv_data, &adv_data);
}


