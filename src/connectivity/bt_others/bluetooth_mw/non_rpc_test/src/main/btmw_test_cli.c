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

// System header files
#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/un.h>
#include <sys/wait.h>
#include <unistd.h>
#include <getopt.h>

// Local header files.
#include "u_bt_mw_types.h"
#include "btmw_test_cli.h"
#include "edit.h"
#include "eloop.h"
#include "util.h"
#include "btmw_test_debug.h"

// Module header files
#include "btmw_test_tools_if.h"
#include "btmw_test_gap_if.h"
#include "btmw_test_gatt_if.h"
#include "btmw_test_a2dp_sink_if.h"
#include "btmw_test_a2dp_src_if.h"
#include "btmw_test_avrcp_if.h"
#include "btmw_test_avrcp_tg_if.h"
#include "btmw_test_hid_if.h"
#include "btmw_test_spp_if.h"
#include "btmw_test_hfclient_if.h"

// Macro definition
#define BTMW_TEST_CMD_KEY_CNF    "MW_CONF"

#if defined(MTK_BT_SYS_LOG)
__attribute__((visibility("default")))UINT32 ui4_enable_all_log = 0;
#endif

// Data structure
typedef struct _btmw_test_cntx_t
{
    // History file path
    char hst_path[BTMW_TEST_MAX_PATH_LEN];
    // Registered modules
    int mods_cnt;
    BTMW_TEST_MOD mods[BTMW_TEST_MAX_MODULES];
} BTMW_TEST_CNTX;



// The socket addr for being filled when receving message.
static BTMW_TEST_CNTX g_btmw_test_cntx;

static int btmw_test_print_help(void);
static int btmw_test_conf_init(void);
static int btmw_test_conf_cmd_handler(int argc, char **argv);
static int btmw_test_get_version_handler(int argc, char *argv[]);
static int btmw_test_edit_filter_hst_cb(void *cntx, const char *cmd);

static BTMW_TEST_CLI btmw_test_conf_cli_commands[] =
{
    { "set_btcli", btmw_test_set_btcli_handler,
      " = set btcli log level <bitmap>"},
    { "get_version", btmw_test_get_version_handler,
      " = get mw version"},
    { NULL, NULL, NULL }
};

int btmw_test_init();
int btmw_test_run();
int btmw_test_deinit();

static BTMW_TEST_MOD *btmw_test_find_mod_by_id(int mod_id);

static void btmw_test_signal_handler(int sig)
{
    btmw_test_deinit();
    exit(0);
}

static void btmw_test_sigchld_handler(int sig)
{
    while (waitpid(-1, NULL, WNOHANG) > 0) {}
}

static void btmw_test_cmd_quit()
{
    struct itimerval itv;

    signal(SIGALRM, btmw_test_signal_handler);

    // Start a timer to disable BT in 3 seconds.
    itv.it_value.tv_sec = 3;
    itv.it_value.tv_usec = 0;
    itv.it_interval.tv_sec = 0;
    itv.it_interval.tv_usec = 0;

    setitimer(ITIMER_REAL, &itv, NULL);
}

EXPORT_SYMBOL int btmw_test_cli_init(void)
{
    static int inited = 0;
    if (inited == 1)
    {
        return 0;
    }

    memset(&g_btmw_test_cntx, 0, sizeof(BTMW_TEST_CNTX));

    btmw_test_tools_init();
    btmw_test_conf_init();
    btmw_test_gap_reg();
    btmw_test_gatt_init();
    btmw_test_hid_init();
    btmw_test_a2dp_src_init();
    btmw_test_a2dp_sink_init();
    btmw_test_rc_init();
    btmw_test_rc_tg_init();
    btmw_test_spp_init();
    btmw_test_hfclient_init();

    inited = 1;
    BTMW_TEST_Logi("[BTMW_TEST] init ok\n");
    return 0;
}

EXPORT_SYMBOL void btmw_test_do_cli_cmd(int argc, char **argv)
{
    BTMW_TEST_MOD *mod;
    int i, found = 0;

    for (i = 0; i < g_btmw_test_cntx.mods_cnt; i++)
    {
        mod = &g_btmw_test_cntx.mods[i];
        if (!strncmp(mod->cmd_key, argv[0], sizeof(mod->cmd_key)))
        {
            BTMW_TEST_Logw("[BTMW_TEST] cmd: %s\n", argv[1]);
            printf("[BTMW_TEST] cmd: %s\n", argv[1]);
            mod->cmd_handler( argc - 1, argv+1);
            found = 1;
            break;
        }
    }

    if (!found)
    {
        BTMW_TEST_Logw("[BTMW_TEST] Invalid cmd: %s\n", argv[0]);
        printf("[BTMW_TEST] Invalid cmd: %s\n", argv[0]);
        btmw_test_print_help();
    }
}


EXPORT_SYMBOL void btmw_test_edit_cmd_cb(void *cntx, char *cmd)
{
    BTMW_TEST_MOD *mod;
    char cmd_buf[BTMW_TEST_MAX_PATH_LEN] = {0};
    char *argv[BTMW_TEST_MAX_ARGS];
    int i, found = 0, argc = 0;

    strncpy(cmd_buf, cmd, BTMW_TEST_MAX_PATH_LEN);
    argc = util_tokenize_cmd(cmd_buf, argv, BTMW_TEST_MAX_ARGS);

    if (argc > 0)
    {
        if (!strncmp(argv[0], "quit", 4) ||
            !strncmp(argv[0], "exit", 4) ||
            !strncmp(argv[0], "q", 1))
        {
            btmw_test_cmd_quit();
            return;
        }

        if (!strncmp(argv[0], "help", 4) )
        {
            btmw_test_print_help();

            return;
        }

        for (i = 0; i < g_btmw_test_cntx.mods_cnt; i++)
        {
            mod = &g_btmw_test_cntx.mods[i];
            if (!strncmp(mod->cmd_key, argv[0], sizeof(mod->cmd_key)))
            {
                mod->cmd_handler( argc - 1, &argv[1]);
                found = 1;
                break;
            }
        }

        if (!found)
        {
            BTMW_TEST_Logw("[BTMW_TEST] Invalid cmd: %s\n", argv[0]);
            btmw_test_print_help();
        }
    }
}

static void btmw_test_edit_eof_cb(void *cntx)
{
    eloop_terminate();
}

static char **btmw_test_build_1st_cmds(void)
{
    BTMW_TEST_MOD *mod;
    char **res;
    int i, count;

    count = g_btmw_test_cntx.mods_cnt + 1;
    res = util_zalloc(count * sizeof(char *));
    if (res == NULL)
    {
        return NULL;
    }

    for (i = 0; i < g_btmw_test_cntx.mods_cnt; i++)
    {
        mod = &g_btmw_test_cntx.mods[i];

        res[i] = strdup(mod->cmd_key);
    }
    res[i] = NULL;

    return res;
}

static char **btmw_test_build_2nd_cmds(BTMW_TEST_MOD *mod)
{
    BTMW_TEST_CLI *tbl;
    char **res;
    int i, count;

    tbl = mod->cmd_tbl;

    count = 0;
    for (i = 0; tbl[i].cmd; i++)
    {
        count++;
    }
    count++;

    res = util_zalloc(count * sizeof(char *));

    for (i = 0; tbl[i].cmd; i++)
    {
        res[i] = strdup(tbl[i].cmd);
        if (res[i] == NULL)
            break;
    }

    return res;
}

static char **btmw_test_edit_cpl_cb(void *cntx, const char *cmd, int pos)
{

    BTMW_TEST_MOD *mod;
    char cmd_buf[BTMW_TEST_MAX_PATH_LEN] = {0};
    char *argv[BTMW_TEST_MAX_ARGS];
    const char *end;
    int i, argc = 0;

    strncpy(cmd_buf, cmd, BTMW_TEST_MAX_PATH_LEN);
    argc = util_tokenize_cmd(cmd_buf, argv, BTMW_TEST_MAX_ARGS);

    if (argc < 2)
    {
        end = strchr(cmd, ' ');
        if (end == NULL || cmd + pos < end)
        {
            return btmw_test_build_1st_cmds();
        }
    }
    else
    {
        end = strchr(cmd, ' ');
        cmd = end + 1;
        end = strchr(cmd, ' ');
        if (end == NULL || cmd + pos < end)
        {
            for (i = 0; i < g_btmw_test_cntx.mods_cnt; i++)
            {
                mod = &g_btmw_test_cntx.mods[i];
                if (!strncmp(mod->cmd_key, argv[0], sizeof(mod->cmd_key)))
                {
                   return btmw_test_build_2nd_cmds(mod);
                }
            }
        }
    }

    return NULL;
}

static int btmw_test_edit_filter_hst_cb(void *cntx, const char *cmd)
{
    return 0;
}
static BTMW_TEST_MOD *btmw_test_find_mod_by_id(int mod_id)
{
    BTMW_TEST_MOD *mod = NULL;
    int i;

    for (i = 0; i < g_btmw_test_cntx.mods_cnt; i++)
    {
        if (g_btmw_test_cntx.mods[i].mod_id == mod_id)
        {
            mod = &g_btmw_test_cntx.mods[i];
            break;
        }
    }

    return mod;
}

int btmw_test_register_mod(BTMW_TEST_MOD *mod)
{
    int i = 0;

    if (mod == NULL)
    {
        BTMW_TEST_Loge("[BTMW_TEST] mod is NULL\n");
        return -1;
    }

    if (mod->cmd_handler == NULL)
    {
        BTMW_TEST_Loge("[BTMW_TEST] cmd_handler: %x\n", mod->cmd_handler);
        return -1;
    }

    if (g_btmw_test_cntx.mods_cnt >= BTMW_TEST_MAX_MODULES - 1)
    {
        BTMW_TEST_Logw("[BTMW_TEST] module table is full. mods_cnts: %d\n", g_btmw_test_cntx.mods_cnt);
        return -1;
    }

    if (btmw_test_find_mod_by_id(mod->mod_id) != NULL)
    {
        BTMW_TEST_Logw("[BTMW_TEST] duplicated registration for mod_id: %d\n", mod->mod_id);
        return -1;
    }

    i = g_btmw_test_cntx.mods_cnt;
    g_btmw_test_cntx.mods[i].mod_id = mod->mod_id;
    strncpy(g_btmw_test_cntx.mods[i].cmd_key, mod->cmd_key, BTMW_TEST_MAX_KEY_LEN);
    g_btmw_test_cntx.mods[i].cmd_handler = mod->cmd_handler;
    g_btmw_test_cntx.mods[i].cmd_tbl = mod->cmd_tbl;

    g_btmw_test_cntx.mods_cnt++;

    return 0;
}

static int btmw_test_conf_init(void)
{
    int ret = 0;
    BTMW_TEST_MOD btmw_test_conf_mod = {0};

    // Register to btmw_test_cli.
    btmw_test_conf_mod.mod_id = BTMW_TEST_MOD_CONF;
    strncpy(btmw_test_conf_mod.cmd_key, BTMW_TEST_CMD_KEY_CNF, sizeof(btmw_test_conf_mod.cmd_key));
    btmw_test_conf_mod.cmd_handler = btmw_test_conf_cmd_handler;
    btmw_test_conf_mod.cmd_tbl = btmw_test_conf_cli_commands;

    ret = btmw_test_register_mod(&btmw_test_conf_mod);
    BTMW_TEST_Logi("[CONF] btmw_test_register_mod() returns: %d\n", ret);

    return ret;
}

static int btmw_test_conf_cmd_handler(int argc, char **argv)
{
    BTMW_TEST_CLI *cmd, *match = NULL;
    int ret = 0;
    int count;

    count = 0;
    cmd = btmw_test_conf_cli_commands;

    BTMW_TEST_Logi("[GAP] argc: %d, argv[0]: %s\n", argc, argv[0]);

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
        BTMW_TEST_Logi("Unknown command '%s'\n", argv[0]);

        btmw_test_print_cmd_help(BTMW_TEST_CMD_KEY_CNF, btmw_test_conf_cli_commands);
        ret = -1;
    }
    else
    {
        match->handler(argc - 1, &argv[1]);
    }

    return ret;
}

static int btmw_test_get_version_handler(int argc, char *argv[])
{
    BTMW_TEST_Logi("[GAP] %s()\n", __func__);
    //GetVersion();
    return 0;
}

EXPORT_SYMBOL void btmw_test_log_init(void)
{
#if defined(MTK_BT_SYS_LOG)
    /*init output log type*/
    if (0 == access("/data/log_all", 0))
    {
        printf("enable all ouput in btmw_test!!\n");
        ui4_enable_all_log = 1;
    }
#endif
}

EXPORT_SYMBOL int btmw_test_set_btcli_handler(int argc, char *argv[])
{
    unsigned short flag;

    BTMW_TEST_Logi("[GAP] %s()\n", __func__);

    if (argc < 1)
    {
        BTMW_TEST_Logi("Usage :\n");
        BTMW_TEST_Logi("  MW_CONF set_btcli <bitmap>\n");
        BTMW_TEST_Logi("     Bit 0 - BTMW_TEST_LOG_LV_VBS\n");
        BTMW_TEST_Logi("     Bit 1 - BTMW_TEST_LOG_LV_INF\n");
        BTMW_TEST_Logi("     Bit 2 - BTMW_TEST_LOG_LV_DBG\n");
        BTMW_TEST_Logi("     Bit 3 - BTMW_TEST_LOG_LV_WRN\n");
        BTMW_TEST_Logi("     Bit 4 - BTMW_TEST_LOG_LV_ERR\n");
        BTMW_TEST_Logi("\n");
        BTMW_TEST_Logi("     Bit 8 - BTMW_TEST_LOG_FLAG_COLOR\n");
        BTMW_TEST_Logi("     Bit 9 - BTMW_TEST_LOG_FLAG_TIMESTAMP\n");

        return -1;
    }

    flag = (unsigned short)strtol(argv[0], NULL, 16);

    BTMW_TEST_Log_SetFlag(flag);

    BTMW_TEST_Logi("[GAP] lv = %x\n", flag);

    return 0;
}

EXPORT_SYMBOL int btmw_test_init()
{
    char cwd[BTMW_TEST_MAX_PATH_LEN] = {0};

    memset(&g_btmw_test_cntx, 0, sizeof(BTMW_TEST_CNTX));
    getcwd(cwd, sizeof(cwd));

    snprintf(g_btmw_test_cntx.hst_path, BTMW_TEST_MAX_PATH_LEN, "%s/%s", cwd, BTMW_TEST_HISTORY_FILE);
    BTMW_TEST_Logv("History file path: %s\n", g_btmw_test_cntx.hst_path);

    if (eloop_init())
    {
        BTMW_TEST_Loge("Failed to init eloop.\n");
        return -1;
    }

    signal(SIGINT, btmw_test_signal_handler);
    signal(SIGTERM, btmw_test_signal_handler);
    signal(SIGCHLD, btmw_test_sigchld_handler);

    btmw_test_tools_init();
    btmw_test_conf_init();
    btmw_test_gap_init();
    btmw_test_gatt_init();
    btmw_test_hid_init();
    btmw_test_a2dp_sink_init();
    btmw_test_a2dp_src_init();
    btmw_test_rc_init();
    btmw_test_rc_tg_init();
    btmw_test_spp_init();
    btmw_test_hfclient_init();
    BTMW_TEST_Logi("[BTMW_TEST] init ok\n");
    return 0;
}

EXPORT_SYMBOL int btmw_test_run()
{
    BTMW_TEST_Logi("[BTMW_TEST] running.\n");

    edit_init(
        btmw_test_edit_cmd_cb,
        btmw_test_edit_eof_cb,
        btmw_test_edit_cpl_cb,
        NULL, g_btmw_test_cntx.hst_path);

    eloop_run();

    return 0;
}

EXPORT_SYMBOL int btmw_test_deinit()
{
    BTMW_TEST_Logi("[BTMW_TEST] deinit. Register mods: %d\n", g_btmw_test_cntx.mods_cnt);

    edit_set_finish(1);
    edit_deinit(g_btmw_test_cntx.hst_path, btmw_test_edit_filter_hst_cb);

    eloop_terminate();
    eloop_destroy();

    return 0;
}

static int btmw_test_print_help(void)
{
    BTMW_TEST_MOD *mod;
    int i;

    for (i = 0; i < g_btmw_test_cntx.mods_cnt; i++)
    {
        mod = &g_btmw_test_cntx.mods[i];

        btmw_test_print_cmd_help(mod->cmd_key, mod->cmd_tbl);
    }

    return 0;
}

void btmw_test_print_cmd_help(const char* prefix, BTMW_TEST_CLI *tbl)
{
    int i;
    char c;

    printf("=============================== %s ===============================\n", prefix);

    while(tbl->cmd)
    {
        if(prefix)
        {
            printf("  %s %s", prefix, tbl->cmd);
        }
        else
        {
            printf("  %s", tbl->cmd);
        }


        for (i = 0; (c = tbl->usage[i]); i++)
        {
            printf("%c", c);
            if (c == '\n')
                printf("%s", prefix);
        }
        printf("\n");

        tbl++;
    }
}

