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


#ifndef _BT_DBG_DEF_H_
#define _BT_DBG_DEF_H_

#if defined(BT_RPC_DBG_SERVER) || defined(BT_RPC_DBG_CLIENT)
#define MOD_LOCATION 24
#define BT_DBG_R_FILE "/tmp/bt-dbg-r"
#define BT_DBG_W_FILE "/tmp/bt-dbg-w"
#define HEAD "<"
#define TYPE "@"
#define ARRY "#"
#define NAME "$"
#define MEMB "^"
#define TAIL ">"
#define DATA "&"
#define HEAD_C '<'
#define TYPE_C '@'
#define ARRY_C '#'
#define NAME_C '$'
#define MEMB_C '^'
#define TAIL_C '>'
#define DATA_C '&'

#if defined(BT_RPC_DBG_SERVER)
typedef int (*DBG_R)(int array_index, int offset, char *name, char *data, int length);
#endif

typedef enum
{
    DBG_START = 0,

    DBG_GAP,
    DBG_L2CAP,
    DBG_GATT,
    DBG_A2DP,
    DBG_HID,
    DBG_AVRCP,
    DBG_HFP,
    DBG_SPP,

    DBG_END
}DBG_MOD;

enum
{
    DBG_OP_NONE,
    DBG_OP_READ,
    DBG_OP_WRITE
};

typedef struct {
    int     cmd;
#if defined(BT_RPC_DBG_SERVER)
    DBG_R   func_r;
#endif
#if defined(BT_RPC_DBG_CLIENT)
    char    *dscr;
#endif
}SUB_CMD;

typedef struct
{
    char    *c_mod;
    DBG_MOD d_mod;
    SUB_CMD *s_cmd;
    int     length;
    char    *dscr;
}MAIN_CMD;

typedef struct
{
    char buf[256];
    int  length;
}BT_RPC_DBG_BUF;

#ifndef NULL
#define NULL 0
#endif

#endif

#endif

