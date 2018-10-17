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

#ifndef __BTMW_TEST_CLI_H__
#define __BTMW_TEST_CLI_H__

#include <time.h>

#define BTMW_TEST_SUCCESS 0
#define BTMW_TEST_FAILED (-1)

#define BTMW_TEST_MAX_ARGS 25
#define BTMW_TEST_MAX_KEY_LEN    32

#define BTMW_TEST_MAX_PATH_LEN   256
#define BTMW_TEST_HISTORY_FILE   ".btmw_test_history"

#define BTMW_TEST_MAX_MODULES    40

typedef struct btmw_test_cli_t
{
    const char *cmd;
    int (*handler)(int argc, char *argv[]);
    const char *usage;
} BTMW_TEST_CLI;

typedef int (*BTMW_TEST_CMD_HANDLER)(int argc, char **argv);

enum
{
    BTMW_TEST_MOD_GAP = 0,
    BTMW_TEST_MOD_A2DP_SINK,
    BTMW_TEST_MOD_A2DP_SRC,
    BTMW_TEST_MOD_GATT_CLIENT,
    BTMW_TEST_MOD_GATT_SERVER,
    BTMW_TEST_MOD_HID,
    BTMW_TEST_MOD_AVRCP_CT,
    BTMW_TEST_MOD_AVRCP_TG,
    BTMW_TEST_MOD_CONF,
    BTMW_TEST_MOD_TOOLS,
    BTMW_TEST_MOD_SPP,
    BTMW_TEST_MOD_HFCLIENT,
    BTMW_TEST_MOD_NUM,
};

enum
{
    OPT_CLI_DEBUG = 1,
    OPT_CLI_FLAGS,
};

typedef struct btmw_test_mod_t
{
    int mod_id;
    char cmd_key[BTMW_TEST_MAX_KEY_LEN];
    BTMW_TEST_CMD_HANDLER cmd_handler;
    BTMW_TEST_CLI *cmd_tbl;
} BTMW_TEST_MOD;

extern void btmw_test_print_cmd_help(const char* prefix, BTMW_TEST_CLI *tbl);
extern int btmw_test_register_mod(BTMW_TEST_MOD *mod);
extern int btmw_test_set_btcli_handler(int argc, char *argv[]);
extern int btmw_test_init();
extern int btmw_test_run();
extern int btmw_test_deinit();
extern void btmw_test_log_init(void);
extern void btmw_test_edit_cmd_cb(void *cntx, char *cmd);
extern void btmw_test_do_cli_cmd(int argc, char **argv);

#endif
