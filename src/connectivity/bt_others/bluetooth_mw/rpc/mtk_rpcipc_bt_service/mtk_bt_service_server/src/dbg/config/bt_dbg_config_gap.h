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


#ifndef _BT_DBG_CONFIG_GAP_H_
#define _BT_DBG_CONFIG_GAP_H_

#if defined(BT_RPC_DBG_SERVER) || defined(BT_RPC_DBG_CLIENT)
enum
{
    GAP_CMD_START = DBG_GAP << MOD_LOCATION & 0xFF000000,

    GAP_CMD_VAR,
    GAP_CMD_STR,
    GAP_CMD_LOCAL_PROPERTY,
    GAP_CMD_LOCAL_RESULT_NUMBER,
    GAP_CMD_INQUIRY_TYPE,
    GAP_CMD_DBG_LEVEL,
    GAP_CMD_END
};

#if defined(BT_RPC_DBG_SERVER)
extern int gap_var_test(int array_index, int offset, char *name, char *data, int length);
extern int gap_str_test(int array_index, int offset, char *name, char *data, int length);
//extern int dbg_gap_get_g_local_property(int array_index, int offset, char *name, char *data, int length);
extern int dbg_gap_get_result_number(int array_index, int offset, char *name, char *data, int length);
extern int dbg_gap_get_inquiry_type(int array_index, int offset, char *name, char *data, int length);
extern int dbg_gap_get_dbg_level(int array_index, int offset, char *name, char *data, int length);



#endif

#if defined(BT_RPC_DBG_SERVER)
#define GAP_DATA_VAR (gap_var_test)
#define GAP_DATA_STR (gap_str_test)
//#define GAP_DATA_LOCAL_PROPERTY (dbg_gap_get_g_local_property)
#define GAP_DATA_LOCAL_RESULT_NUMBER (dbg_gap_get_result_number)
#define GAP_DATA_INQUIRY_TYPE (dbg_gap_get_inquiry_type)
#define GAP_DATA_DBG_LEVEL (dbg_gap_get_dbg_level)

#endif

#if defined(BT_RPC_DBG_CLIENT)
#define GAP_DATA_VAR (HEAD"GAP"TYPE"VAR"ARRY"8"NAME"gap_demo_var"TAIL)
#define GAP_DATA_STR (HEAD"GAP"TYPE"STR"ARRY"16"NAME"gap_demo_str"TAIL)
//#define GAP_DATA_LOCAL_PROPERTY (HEAD"GAP"TYPE"STR"NAME"g_local_property"TAIL)
#define GAP_DATA_LOCAL_RESULT_NUMBER (HEAD"GAP"TYPE"VAR"NAME"g_pt_result_number"TAIL)
#define GAP_DATA_INQUIRY_TYPE (HEAD"GAP"TYPE"VAR"NAME"fg_bt_inquiry_type"TAIL)
#define GAP_DATA_DBG_LEVEL (HEAD"GAP"TYPE"STR"NAME"btmw_trc_map"TAIL)

#endif




#define GAP_CMD_NUM     (GAP_CMD_END - GAP_CMD_START)

static SUB_CMD gap_cmd[GAP_CMD_NUM] =
{
    {GAP_CMD_VAR,   GAP_DATA_VAR},
    {GAP_CMD_STR,   GAP_DATA_STR},
//    {GAP_CMD_LOCAL_PROPERTY,   GAP_DATA_LOCAL_PROPERTY},
    {GAP_CMD_LOCAL_RESULT_NUMBER, GAP_DATA_LOCAL_RESULT_NUMBER},
    {GAP_CMD_INQUIRY_TYPE, GAP_DATA_INQUIRY_TYPE},
    {GAP_CMD_DBG_LEVEL, GAP_DATA_DBG_LEVEL},
    {0,   NULL}
};
#endif

#endif

