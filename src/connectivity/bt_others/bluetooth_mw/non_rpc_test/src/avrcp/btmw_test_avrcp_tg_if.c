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

#include "c_bt_mw_avrcp.h"
#include "c_bt_mw_a2dp_common.h"
#include "btmw_test_avrcp_tg_if.h"

extern CHAR g_avrcp_addr_test;
extern UINT32 g_avrcp_reg_event;
#define BTMW_TEST_CMD_KEY_AVRCP_TG     "MW_AVRCP_TG"




static BTMW_TEST_CLI btmw_test_rc_tg_cli_commands[] =
{
    {NULL, NULL, NULL},
};

int btmw_test_rc_tg_cmd_handler(int argc, char **argv)
{
    BTMW_TEST_CLI *cmd, *match = NULL;
    int ret = 0;
    int count = 0;

    BTMW_TEST_Logd("[AVRCP] argc: %d, argv[0]: %s\n", argc, argv[0]);

    cmd = btmw_test_rc_tg_cli_commands;
    while (cmd->cmd)
    {
        if (!strcmp(cmd->cmd, argv[0]))
        {
            match = cmd;
            count = 1;
            break;
        }
        cmd++;
    }

    if (count == 0)
    {
        BTMW_TEST_Logd("[AVRCP] Unknown command '%s'\n", argv[0]);
        btmw_test_print_cmd_help(BTMW_TEST_CMD_KEY_AVRCP_TG, btmw_test_rc_tg_cli_commands);
        ret = -1;
    }
    else
    {
        match->handler(argc - 1, &argv[1]);
    }

    return ret;
}


int btmw_test_rc_tg_init()
{
    int ret = 0;
    BTMW_TEST_MOD rc_mod = {0};

    BTMW_TEST_Logd("[AVRCP] %s() \n", __func__);

    rc_mod.mod_id = BTMW_TEST_MOD_AVRCP_TG;
    strncpy(rc_mod.cmd_key, BTMW_TEST_CMD_KEY_AVRCP_TG, sizeof(rc_mod.cmd_key));
    rc_mod.cmd_handler = btmw_test_rc_tg_cmd_handler;
    rc_mod.cmd_tbl = btmw_test_rc_tg_cli_commands;

    ret = btmw_test_register_mod(&rc_mod);
    BTMW_TEST_Logd("[AVRCP] btmw_test_register_mod() for TG returns: %d\n", ret);

    return ret;
}

int btmw_test_rc_tg_deinit()
{
    BTMW_TEST_Logd("[AVRCP] %s() \n", __func__);

    return 0;
}
