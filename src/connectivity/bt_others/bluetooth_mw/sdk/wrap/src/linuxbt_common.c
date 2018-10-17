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


/* FILE NAME:  bt_mw_common.h
 * AUTHOR: Hongliang Hu
 * PURPOSE:
 *      It provides bluetooth common structure to wrap.
 * NOTES:
 */


#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stddef.h>
#include <ctype.h>
#include "bluetooth.h"
#include "linuxbt_common.h"

INT32 linuxbt_btaddr_stoh(CHAR *btaddr_s, bt_bdaddr_t *bdaddr_h)
{
    INT32 i;
    if (NULL == btaddr_s || NULL == bdaddr_h)
    {
        return BT_ERR_STATUS_PARM_INVALID;
    }
    for (i = 0; i <6; i++)
    {
        bdaddr_h->address[i] = strtoul(btaddr_s, &btaddr_s, 16);
        btaddr_s++;
    }
    return BT_SUCCESS;
}

INT32 linuxbt_btaddr_htos(bt_bdaddr_t *bdaddr_h, CHAR *btaddr_s)
{
    if (NULL == btaddr_s || NULL == bdaddr_h)
    {
        return BT_ERR_STATUS_PARM_INVALID;
    }
    snprintf(btaddr_s, 18, "%02X:%02X:%02X:%02X:%02X:%02X",
             bdaddr_h->address[0],
             bdaddr_h->address[1],
             bdaddr_h->address[2],
             bdaddr_h->address[3],
             bdaddr_h->address[4],
             bdaddr_h->address[5]);
    btaddr_s[17] = '\0';

    return BT_SUCCESS;
}

/**
 * FUNCTION NAME: linuxbt_print_uuid
 * PURPOSE:
 *      The function is used to convert uuid from bt_uuid_t* to char*.
 * INPUT:
 *      uuid        -- array to store uuid
 *      uuid_s      -- string to sore uuid
 * OUTPUT:
 *      None
 * RETURN:
 *      None
 * NOTES:
 *      None
 */
void linuxbt_print_uuid(bt_uuid_t* uuid, char* uuid_s)
{
    //FUNC_ENTRY;
    int i;
    char *ptr;
    ptr = uuid_s;
    for (i = 15; i >= 0; i--)
    {
        sprintf(ptr, "%02X", uuid->uu[i]);
        ptr+=2;
        if (i == 12 || i == 10 || i == 8 || i == 6)
        {
            *ptr = '-';
            ptr++;
        }
    }
    *ptr = '\0';
}

VOID linuxbt_uuid_stoh(CHAR *uuid_s,  bt_uuid_t *uuid)
{
    INT32 i = 0,j = 0;
    INT32 size = 0;
    CHAR  temp[3];
    CHAR  temp_uuid[16];
    temp[2] = '\0';

    if (NULL == uuid_s || NULL == uuid)
    {
        return;
    }

    size = strlen(uuid_s);
    while (i < size)
    {
        if (uuid_s[i] == '-' || uuid_s[i] == '\0')
        {
            i++;
            continue;
        }
        temp[0] = uuid_s[i];
        temp[1] = uuid_s[i+1];
        temp_uuid[j] = strtoul(temp, NULL, 16);
        i+=2;
        j++;
    }

    if (size <= 8)   // 16bits uuid or 32bits uuid
    {
        if (size == 4)
        {
            temp_uuid[2] = temp_uuid[0];
            temp_uuid[3] = temp_uuid[1];
            temp_uuid[0] = 0;
            temp_uuid[1] = 0;
        }
        temp_uuid[4] = 0x00;
        temp_uuid[5] = 0x00;
        temp_uuid[6] = 0x10;
        temp_uuid[7] = 0x00;

        temp_uuid[8] = 0x80;
        temp_uuid[9] = 0x00;
        temp_uuid[10] = 0x00;
        temp_uuid[11] = 0x80;

        temp_uuid[12] = 0x5F;
        temp_uuid[13] = 0x9B;
        temp_uuid[14] = 0x34;
        temp_uuid[15] = 0xFB;
    }

    for (i = 0; i < 16; i++)
    {
        uuid->uu[i]= temp_uuid[15 - i];
    }
}

EXPORT_SYMBOL INT32 ascii_2_hex(CHAR *p_ascii, INT32 len, UINT8 *p_hex)
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

BT_ERR_STATUS_T linuxbt_return_value_convert(bt_status_t ret)
{
    BT_ERR_STATUS_T status = BT_SUCCESS;
    if (BT_STATUS_SUCCESS == ret)
    {
        status = BT_SUCCESS;
    }
    else if (BT_STATUS_FAIL == ret)
    {
        status = BT_ERR_STATUS_FAIL;
    }
    else if (BT_STATUS_NOT_READY== ret)
    {
        status = BT_ERR_STATUS_NOT_READY;
    }
    else if (BT_STATUS_NOMEM== ret)
    {
        status = BT_ERR_STATUS_NOMEM;
    }
    else if (BT_STATUS_BUSY== ret)
    {
        status = BT_ERR_STATUS_BUSY;
    }
    else if (BT_STATUS_DONE== ret)
    {
        status = BT_ERR_STATUS_DONE;
    }
    else if (BT_STATUS_UNSUPPORTED== ret)
    {
        status = BT_ERR_STATUS_UNSUPPORTED;
    }
    else if (BT_STATUS_PARM_INVALID== ret)
    {
        status = BT_ERR_STATUS_PARM_INVALID;
    }
    else if (BT_STATUS_UNHANDLED== ret)
    {
        status = BT_ERR_STATUS_UNHANDLED;
    }
    else if (BT_STATUS_AUTH_FAILURE== ret)
    {
        status = BT_ERR_STATUS_AUTH_FAILURE;
    }
    else if (BT_STATUS_RMT_DEV_DOWN== ret)
    {
        status = BT_ERR_STATUS_RMT_DEV_DOWN;
    }
    return status;
}

