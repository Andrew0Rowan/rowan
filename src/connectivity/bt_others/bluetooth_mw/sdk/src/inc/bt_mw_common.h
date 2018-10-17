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
 * AUTHOR:
 * PURPOSE:
 *      It provides bluetooth common structure to MW.
 * NOTES:
 */

#ifndef _BT_MW_COMMON_H_
#define _BT_MW_COMMON_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
//#include <signal.h>
#include "mdroid_buildcfg.h"
#include "u_bt_mw_common.h"
#include "bt_mw_log.h"

#define BLUETOOTH_MW_LOCAL_FOLDER  MW_STORAGE_PATH"/bluetooth"
#define BLUETOOTH_STACK_LOCAL_FOLDER  MW_STORAGE_PATH"/bluedroid"

#define BLUETOOTH_RM_BT_CONFIG_CMD      "rm -rf /data/misc/bluedroid/bt_config.*"
#define BLUETOOTH_RM_BT_SNOOP_HCI_CMD   "rm -rf /data/misc/bluedroid/bt_btsnoop*"
#define BLUETOOTH_RM_BT_STACK_LOG_CMD   "rm -rf /data/misc/bluedroid/bt_stack.log"
#define BLUETOOTH_RM_BAK_FILE_CMD       "rm -rf /data/misc/bluedroid/*.bak"
#define BLUETOOTH_RM_BT_PROPERTY_CMD    "rm -rf "BT_MISC_PATH"bt_property"

#define BLUETOOTH_ORIGINAL_NAME         "BLUETOOTH_AUDIO"
#define BT_GATT_UUID_ARRAY_SIZE         16


#define BTWM_ID_GAP          0
#define BTWM_ID_A2DP         1
#define BTWM_ID_AVRCP        2
#define BTWM_ID_HIDH         3
#define BTWM_ID_GATT         4
#define BTWM_ID_HFP          5
#define BTWM_ID_SPP          6
#define BTWM_ID_MAX          7

#define BTMW_EVT_START(id)       ((id) << 8)

VOID bluetooth_uuid_stos(CHAR *uuid_s,  CHAR *uuid);
VOID bluetooth_uuid_stoh(CHAR *uuid_s,  CHAR *uuid);
#endif /* _BT_MW_COMMON_H_ */

