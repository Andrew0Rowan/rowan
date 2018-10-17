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

#include "mtk_bt_service_spp.h"
#include "c_bt_mw_spp.h"

#define BT_RC_LOG(_stmt...) \
        do{ \
            if(0){    \
                printf("Func:%s Line:%d--->: ", __FUNCTION__, __LINE__);   \
                printf(_stmt); \
                printf("\n"); \
            }        \
        }   \
        while(0)


INT32 x_mtkapi_spp_connect(CHAR* addr, CHAR* uuid)
{
    INT32 i4_ret = 0;
    i4_ret = c_btm_spp_connect(addr, uuid);
    if (i4_ret != 0)
    {
        BT_RC_LOG("c_btm_spp_connect fail\n");
    }
    else
    {
        BT_RC_LOG("c_btm_spp_connect success\n");
    }
    return i4_ret;
}

INT32 x_mtkapi_spp_disconnect(CHAR *addr, CHAR *uuid)
{
    INT32 i4_ret = 0;
    i4_ret = c_btm_spp_disconnect(addr, uuid);
    if (i4_ret != 0)
    {
        BT_RC_LOG("c_btm_spp_disconnect fail\n");
    }
    else
    {
        BT_RC_LOG("c_btm_spp_disconnect success\n");
    }
    return i4_ret;
}

INT32 x_mtkapi_spp_send_data(CHAR* bd_addr, CHAR *uuid, CHAR* str, INT32 len)
{
    INT32 i4_ret = 0;
    i4_ret = c_btm_spp_send_data(bd_addr, uuid, str, len);
    if (i4_ret != 0)
    {
        BT_RC_LOG("c_btm_spp_send_data fail\n");
    }
    else
    {
        BT_RC_LOG("c_btm_spp_send_data success\n");
    }
    return i4_ret;
}

INT32 x_mtkapi_spp_enable_devb(CHAR *servername, CHAR* uuid)
{
    INT32 i4_ret = 0;
    i4_ret = c_btm_spp_start_server(servername, uuid);
    if (i4_ret != 0)
    {
        BT_RC_LOG("c_btm_spp_start_server fail\n");
    }
    else
    {
        BT_RC_LOG("c_btm_spp_start_server success\n");
    }
    return i4_ret;
}

INT32 x_mtkapi_spp_disable_devb(CHAR *uuid)
{
    INT32 i4_ret = 0;
    i4_ret = c_btm_spp_stop_server(uuid);
    if (i4_ret != 0)
    {
        BT_RC_LOG("c_btm_spp_stop_server fail\n");
    }
    else
    {
        BT_RC_LOG("c_btm_spp_stop_server success\n");
    }
    return i4_ret;
}


