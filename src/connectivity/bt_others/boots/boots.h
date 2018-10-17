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
 * MediaTek Inc. (C) 2016~2017. All rights reserved.
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

//- vim: set ts=4 sts=4 sw=4 et: --------------------------------------------
#ifndef __BOOTS_H__
#define __BOOTS_H__

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <memory.h>

#include "boots_cfg.h"

//---------------------------------------------------------------------------
#define VERSION                 "1.8.6"

#define NONE "\033[m"
#define BLUE "\033[0;32;34m"
#define RED "\033[0;32;31m"

#define HCI_BUF_SIZE            0x2000

#define BOOTS_MSG_LVL_DBG       4
#define BOOTS_MSG_LVL_INFO      3
#define BOOTS_MSG_LVL_WARN      2
#define BOOTS_MSG_LVL_ERR       1
#define BOOTS_MSG_LVL_NONE      0

#define BPRINT_D_RAW(fmt, ...) \
    do { if (BOOTS_MSG_LVL_DEFAULT >= BOOTS_MSG_LVL_DBG) \
        printf(fmt, ##__VA_ARGS__);                         } while (0);
#define BPRINT_D(fmt, ...) \
    do { if (BOOTS_MSG_LVL_DEFAULT >= BOOTS_MSG_LVL_DBG) \
        printf("[%s:D] "fmt"\n", LOG_TAG, ##__VA_ARGS__);   } while (0);
#define BPRINT_I_RAW(fmt, ...) \
    do { if (BOOTS_MSG_LVL_DEFAULT >= BOOTS_MSG_LVL_INFO) \
        printf(fmt, ##__VA_ARGS__);                         } while (0);
#define BPRINT_I(fmt, ...) \
    do { if (BOOTS_MSG_LVL_DEFAULT >= BOOTS_MSG_LVL_INFO) \
        printf("[%s] "fmt"\n", LOG_TAG, ##__VA_ARGS__);     } while (0);
#define BPRINT_W(fmt, ...) \
    do { if (BOOTS_MSG_LVL_DEFAULT >= BOOTS_MSG_LVL_WARN) \
        printf(BLUE "[%s:W] "fmt"\n" NONE, LOG_TAG, ##__VA_ARGS__);   } while (0);
#define BPRINT_E(fmt, ...) \
    do { if (BOOTS_MSG_LVL_DEFAULT >= BOOTS_MSG_LVL_ERR) \
        printf(RED "[%s:E] "fmt" !!!\n" NONE, LOG_TAG, ##__VA_ARGS__);} while (0);

#define SHOW_RAW(len, buf) \
    do { \
        int i = 0; \
        while (i < len) { \
            printf("%02X ", buf[i]); \
            i++; \
        } \
        printf("\n"); \
    } while (0);

#define MAX(a, b) \
    (((a) > (b)) ? (a) : (b))

#define ARRAY_SIZE(arr) \
    (sizeof(arr) / sizeof((arr)[0]))

#define CHECK_USERID() \
    if (getuid() != 0) { \
        BPRINT_E("Please run boots as ROOT!!!"); \
        exit(1); \
    }

#ifndef UNUSED
#define UNUSED(x) ((void)x)
#endif

/** Boots BT Interface */
typedef enum {
    BOOTS_IF_NONE = 0,
    BOOTS_BTIF_STPBT,
    BOOTS_BTIF_HCI,
    BOOTS_BTIF_ALL,
    BOOTS_CSIF_SKT,
    BOOTS_CSIF_UART,
    BOOTS_CSIF_ETH,
    BOOTS_CLIF_USER,
    BOOTS_CLIF_UART,
} boots_if_e;

typedef struct {
    boots_if_e inf; // interface
    char *n;        // name
    char *p;        // path
} boots_btif_s;

typedef struct {
    boots_if_e      btif;
    boots_if_e      csif;
    boots_if_e      clif;
    char            bt[16];
    char            cs[16];
    char            cli[16];
    int             cs_speed;
    int             cli_speed;
} boots_if_s;

//---------------------------------------------------------------------------
static boots_btif_s boots_btif[] = {
    {BOOTS_BTIF_STPBT,  "stpbt",    "/dev/stpbt"},
    {BOOTS_BTIF_HCI,    "hci",      NULL},
    {BOOTS_IF_NONE,     "",         NULL},      // Should leave bottom
};

//---------------------------------------------------------------------------
#endif // __BOOTS_H__
