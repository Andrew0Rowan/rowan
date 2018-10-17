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


#ifndef _BT_DBG_CONFIG_AVRCP_H_
#define _BT_DBG_CONFIG_AVRCP_H_

#if defined(BT_RPC_DBG_SERVER) || defined(BT_RPC_DBG_CLIENT)
enum
{
    AVRCP_CMD_START = DBG_AVRCP << MOD_LOCATION & 0xFF000000,
    AVRCP_CMD_MEDIA_INFO,   // g_media_info
    AVRCP_CMD_PLAYER_STATUS,  // g_player_status

    AVRCP_CMD_END
};

#if defined(BT_RPC_DBG_SERVER)
extern int dbg_avrcp_get_g_media_info(int array_index, int offset, char *name, char *data, int length);
extern int dbg_avrcp_get_g_player_status(int array_index, int offset, char *name, char *data, int length);

#endif

#if defined(BT_RPC_DBG_SERVER)
#define AVRCP_DATA_MEDIA_INFO    (dbg_avrcp_get_g_media_info)
#define AVRCP_DATA_PLAYER_STATUS (dbg_avrcp_get_g_player_status)

#endif
#if defined(BT_RPC_DBG_CLIENT)
#define AVRCP_DATA_MEDIA_INFO     (HEAD"AVRCP"TYPE"STR"NAME"g_media_info"TAIL)
#define AVRCP_DATA_PLAYER_STATUS  (HEAD"AVRCP"TYPE"STR"NAME"g_player_status"TAIL)

#endif

#define AVRCP_CMD_NUM   (AVRCP_CMD_END - AVRCP_CMD_START)

static SUB_CMD avrcp_cmd[AVRCP_CMD_NUM] =
{
    {AVRCP_CMD_MEDIA_INFO,      AVRCP_DATA_MEDIA_INFO},
    {AVRCP_CMD_PLAYER_STATUS,   AVRCP_DATA_PLAYER_STATUS},

    {0,   NULL}
};
#endif

#endif

