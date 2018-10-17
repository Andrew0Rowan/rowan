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
#include "btut_cli.h"
#include "edit.h"
#include "eloop.h"
#include "util.h"
#include "btut_debug.h"

// Module header files
#include "btut_tools_if.h"
#include "btut_gap_if.h"

// Macro definition
#define CMD_KEY_CNF                         "CNF"

#if defined(MTK_BT_SYS_LOG)
__attribute__((visibility("default")))unsigned int ui4_enable_all_log = 0;
#endif

//extern void set_loglv(int module, int lv);
//extern void GetVersion(void);

// Data structure
typedef struct _btut_cntx_t
{
    // History file path
    char hst_path[BTUT_MAX_PATH_LEN];
    // Registered modules
    int mods_cnt;
    BTUT_MOD mods[BTUT_MAX_MODULES];
} BTUT_CNTX;

enum
{
    OPT_CLI_DEBUG = 1,
    OPT_CLI_FLAGS,
    OPT_LIB_DEBUG,
    OPT_SCAN_MODE,
};

// The socket addr for being filled when receving message.
static BTUT_CNTX g_btut_cntx;

static int btut_print_help(void);
static int btut_cnf_init(void);
static int btut_cnf_cmd_handler(int argc, char **argv);
static int btut_set_log_lv_handler(int argc, char *argv[]);
static int btut_get_version_handler(int argc, char *argv[]);
static int btut_set_btcli_handler(int argc, char *argv[]);

static int btut_edit_filter_hst_cb(void *cntx, const char *cmd);

static BTUT_CLI bt_cnf_cli_commands[] =
{
    { "set_log_lv", btut_set_log_lv_handler,
      "= (1~3) 1:bluetooth.so 2:mtkbt 3:both; (0~5) log level" },
    { "set_btcli", btut_set_btcli_handler,
      "= set btcli log level" },
    { "get_version", btut_get_version_handler,
      "= get blueangel version" },
    { NULL, NULL, NULL }
};

int btut_init();
int btut_run();
int btut_deinit();

static BTUT_MOD *btut_find_mod_by_id(int mod_id);

static void btut_signal_handler(int sig)
{
    btut_deinit();
    exit(0);
}

static void btut_sigchld_handler(int sig)
{
    while (waitpid(-1, NULL, WNOHANG) > 0) {}
}

static void btut_cmd_quit()
{
    struct itimerval itv;

    // Disable profiles first.
    btut_gap_deinit_profiles();
    signal(SIGALRM, btut_signal_handler);

    // Start a timer to disable BT in 3 seconds.
    itv.it_value.tv_sec = 3;
    itv.it_value.tv_usec = 0;
    itv.it_interval.tv_sec = 0;
    itv.it_interval.tv_usec = 0;

    setitimer(ITIMER_REAL, &itv, NULL);
}

static void btut_edit_cmd_cb(void *cntx, char *cmd)
{
    BTUT_MOD *mod;
    char cmd_buf[BTUT_MAX_PATH_LEN] = {0};
    char *argv[BTUT_MAX_ARGS];
    int i, found = 0, argc = 0;

    strncpy(cmd_buf, cmd, BTUT_MAX_PATH_LEN);
    argc = util_tokenize_cmd(cmd_buf, argv, BTUT_MAX_ARGS);

    // BTUT_Logd("[BTUT] argc: %d, argv[0]: %s\n", argc, argv[0]);

    if (argc > 0)
    {
        if (!strncmp(argv[0], "quit", 4) ||
            !strncmp(argv[0], "exit", 4) ||
            !strncmp(argv[0], "q", 1))
        {
            btut_cmd_quit();
            return;
        }

        if (!strncmp(argv[0], "help", 4) )
        {
            btut_print_help();

            return;
        }

        for (i = 0; i < g_btut_cntx.mods_cnt; i++)
        {
            mod = &g_btut_cntx.mods[i];
            if (!strncmp(mod->cmd_key, argv[0], sizeof(mod->cmd_key)))
            {
                mod->cmd_handler( argc - 1, &argv[1]);
                found = 1;
                break;
            }
        }

        if (!found)
        {
            BTUT_Logw("[BTUT] Invalid cmd: %s\n", argv[0]);
            btut_print_help();
        }
    }
}

static void btut_edit_eof_cb(void *cntx)
{
    eloop_terminate();
}

static char **btut_build_1st_cmds(void)
{
    BTUT_MOD *mod;
    char **res;
    int i, count;

    count = g_btut_cntx.mods_cnt + 1;
    res = util_zalloc(count * sizeof(char *));
    if (res == NULL)
    {
        return NULL;
    }

    for (i = 0; i < g_btut_cntx.mods_cnt; i++)
    {
        mod = &g_btut_cntx.mods[i];

        res[i] = strdup(mod->cmd_key);
    }
    res[i] = NULL;

    return res;
}

static char **btut_build_2nd_cmds(BTUT_MOD *mod)
{
    BTUT_CLI *tbl;
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

static char **btut_edit_cpl_cb(void *cntx, const char *cmd, int pos)
{

    BTUT_MOD *mod;
    char cmd_buf[BTUT_MAX_PATH_LEN] = {0};
    char *argv[BTUT_MAX_ARGS];
    const char *end;
    int i, argc = 0;

    strncpy(cmd_buf, cmd, BTUT_MAX_PATH_LEN);
    argc = util_tokenize_cmd(cmd_buf, argv, BTUT_MAX_ARGS);

    if (argc < 2)
    {
        end = strchr(cmd, ' ');
        if (end == NULL || cmd + pos < end)
        {
            return btut_build_1st_cmds();
        }
    }
    else
    {
        end = strchr(cmd, ' ');
        cmd = end + 1;
        end = strchr(cmd, ' ');
        if (end == NULL || cmd + pos < end)
        {
            for (i = 0; i < g_btut_cntx.mods_cnt; i++)
            {
                mod = &g_btut_cntx.mods[i];
                if (!strncmp(mod->cmd_key, argv[0], sizeof(mod->cmd_key)))
                {
                   return btut_build_2nd_cmds(mod);
                }
            }
        }
    }

    return NULL;
}

static int btut_edit_filter_hst_cb(void *cntx, const char *cmd)
{
    return 0;
}
static BTUT_MOD *btut_find_mod_by_id(int mod_id)
{
    BTUT_MOD *mod = NULL;
    int i;

    for (i = 0; i < g_btut_cntx.mods_cnt; i++)
    {
        if (g_btut_cntx.mods[i].mod_id == mod_id)
        {
            mod = &g_btut_cntx.mods[i];
            break;
        }
    }

    return mod;
}

int btut_register_mod(BTUT_MOD *mod)
{
    int i = 0;

    if (mod == NULL)
    {
        BTUT_Loge("[BTUT] mod is NULL\n");
        return -1;
    }

    if (mod->cmd_handler == NULL)
    {
        BTUT_Loge("[BTUT] cmd_handler: %x\n", mod->cmd_handler);
        return -1;
    }

    if (g_btut_cntx.mods_cnt >= BTUT_MAX_MODULES - 1)
    {
        BTUT_Logw("[BTUT] module table is full. mods_cnts: %d\n", g_btut_cntx.mods_cnt);
        return -1;
    }

    if (btut_find_mod_by_id(mod->mod_id) != NULL)
    {
        BTUT_Logw("[BTUT] duplicated registration for mod_id: %d\n", mod->mod_id);
        return -1;
    }

    i = g_btut_cntx.mods_cnt;
    g_btut_cntx.mods[i].mod_id = mod->mod_id;
    strncpy(g_btut_cntx.mods[i].cmd_key, mod->cmd_key, BTUT_MAX_KEY_LEN);
    g_btut_cntx.mods[i].cmd_handler = mod->cmd_handler;
    g_btut_cntx.mods[i].cmd_tbl = mod->cmd_tbl;

    g_btut_cntx.mods_cnt++;

    return 0;
}

static int btut_cnf_init(void)
{
    int ret = 0;
    BTUT_MOD gap_mod = {0};

    // Register to btut_cli.
    gap_mod.mod_id = BTUT_MOD_CNF;
    strncpy(gap_mod.cmd_key, CMD_KEY_CNF, sizeof(gap_mod.cmd_key));
    gap_mod.cmd_handler = btut_cnf_cmd_handler;
    gap_mod.cmd_tbl = bt_cnf_cli_commands;

    ret = btut_register_mod(&gap_mod);
    BTUT_Logi("[GAP] btut_register_mod() returns: %d\n", ret);

    return ret;
}

static int btut_cnf_cmd_handler(int argc, char **argv)
{
    BTUT_CLI *cmd, *match = NULL;
    int ret = 0;
    int count;

    count = 0;
    cmd = bt_cnf_cli_commands;

    BTUT_Logi("[GAP] argc: %d, argv[0]: %s\n", argc, argv[0]);

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
        BTUT_Logi("Unknown command '%s'\n", argv[0]);

        btut_print_cmd_help(CMD_KEY_CNF, bt_cnf_cli_commands);
        ret = -1;
    }
    else
    {
        match->handler(argc - 1, &argv[1]);
    }

    return ret;
}

static int btut_set_log_lv_handler(int argc, char *argv[])
{
    int lv;
    int module;

    BTUT_Logi("[GAP] %s()\n", __func__);

    if (argc < 2)
    {
        BTUT_Loge("Usage :\n");
        BTUT_Loge("CNF set_log_lv [1~3] [0~5]\n");
        BTUT_Loge("(1~3) 1:bluetooth.so 2:mtkbt 3:both; (0~5) log level  [0 - enable all logs, 5 - disable all logs]\n");
        return -1;
    }

    module = atoi(argv[0]);
    lv = atoi(argv[1]);

    if (module < 0 || module > 3)
    {
        BTUT_Loge("modules range is 1~3!!!\n");
        return -1;
    }
    if (lv < 0 || lv > 5)
    {
        BTUT_Loge("level range is 0~5!!!\n");
        return -1;
    }

    //set_loglv(module, lv);
    return 0;
}

static int btut_get_version_handler(int argc, char *argv[])
{
    BTUT_Logi("[GAP] %s()\n", __func__);
    //GetVersion();
    return 0;
}

static int btut_set_btcli_handler(int argc, char *argv[])
{
    unsigned short flag;

    BTUT_Logi("[GAP] %s()\n", __func__);

    if (argc < 1)
    {
        BTUT_Logi("Usage :\n");
        BTUT_Logi("  CNF set_btcli [bitmap1] [bitmap2\n");
        BTUT_Logi("     Bit 0 - BTUT_LOG_LV_VBS\n");
        BTUT_Logi("     Bit 1 - BTUT_LOG_LV_INF\n");
        BTUT_Logi("     Bit 2 - BTUT_LOG_LV_DBG\n");
        BTUT_Logi("     Bit 3 - BTUT_LOG_LV_WRN\n");
        BTUT_Logi("     Bit 4 - BTUT_LOG_LV_ERR\n");
        BTUT_Logi("\n");
        BTUT_Logi("     Bit 8 - BTUT_LOG_FLAG_COLOR\n");
        BTUT_Logi("     Bit 9 - BTUT_LOG_FLAG_TIMESTAMP\n");

        return -1;
    }

    flag = (unsigned short)strtol(argv[0], NULL, 16);

    BTUT_Log_SetFlag(flag);

    BTUT_Logi("[GAP] lv = %x\n", flag);

    return 0;
}

int btut_init()
{
    char cwd[BTUT_MAX_PATH_LEN] = {0};

    memset(&g_btut_cntx, 0, sizeof(BTUT_CNTX));
    getcwd(cwd, sizeof(cwd));

    snprintf(g_btut_cntx.hst_path, BTUT_MAX_PATH_LEN, "%s/%s", cwd, BTUT_HISTORY_FILE);
    BTUT_Logv("History file path: %s\n", g_btut_cntx.hst_path);

    if (eloop_init())
    {
        BTUT_Loge("Failed to init eloop.\n");
        return -1;
    }

    signal(SIGINT, btut_signal_handler);
    signal(SIGTERM, btut_signal_handler);
    signal(SIGCHLD, btut_sigchld_handler);

    btut_tools_init();
    btut_cnf_init();
    btut_gap_init();

    BTUT_Logi("[BTUT] init ok\n");
    return 0;
}

int btut_run()
{
    BTUT_Logi("[BTUT] running.\n");

    edit_init(
        btut_edit_cmd_cb,
        btut_edit_eof_cb,
        btut_edit_cpl_cb,
        NULL, g_btut_cntx.hst_path);

    eloop_run();

    return 0;
}

int btut_deinit()
{
    int i = 0;

    BTUT_Logi("[BTUT] deinit. Register mods: %d\n", g_btut_cntx.mods_cnt);

    btut_gap_deinit();

    edit_set_finish(1);
    edit_deinit(g_btut_cntx.hst_path, btut_edit_filter_hst_cb);

    eloop_terminate();
    eloop_destroy();

    for (i = 0; i < g_btut_cntx.mods_cnt; i++)
    {
        // close(g_btut_cntx.mods[i].s);
    }

    return 0;
}

static int btut_print_help(void)
{
    BTUT_MOD *mod;
    int i;

    for (i = 0; i < g_btut_cntx.mods_cnt; i++)
    {
        mod = &g_btut_cntx.mods[i];

        btut_print_cmd_help(mod->cmd_key, mod->cmd_tbl);
    }

    return 0;
}

void btut_print_cmd_help(const char* prefix, BTUT_CLI *tbl)
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

static void usage(void)
{
    printf(
"Usage: btcli [OPTION]...\n"
"\n"
"-h, --help              help\n"
"-c, --cli=#             choose cli mode. 0=disable, 1=enable\n"
"    --cli_debug=#       choose cli debug bitmap. #=2 byte hex number\n"
"    --lib_debug=#       choose library debug level. #=decimal number\n"
"-s  --scan_mode=#       choose scan mode. #=[0:none, 1:connectable, 2:connectable_discoverable]\n"
        );
}

int main(int argc, char *argv[])
{
    int option_index;
    static const char short_options[] = "hc:s:";
    static const struct option long_options[] = {
        {"help", 0, 0, 'h'},
        {"cli", 1, 0, 'c'},
        {"cli_debug", 1, 0, OPT_CLI_DEBUG},
        {"lib_debug", 1, 0, OPT_LIB_DEBUG},
        {"scan_mode", 1, 0, OPT_SCAN_MODE},
        {0, 0, 0, 0}
    };
    int c;
    int cli_mode = 1;
    int cli_debug = 0;
    int lib_debug = 0;
    char *cli_debug_bitmap;
    char *lib_debug_level;
    int opt_argc;
    char *opt_argv[2];
#if defined(MTK_BT_SYS_LOG)
    /*init output log type*/
    if (0 == access("/data/log_all", 0))
    {
        printf("enable all ouput in btservice!!\n");
        ui4_enable_all_log = 1;
    }

#endif

    while ((c = getopt_long(argc, argv, short_options, long_options, &option_index)) != -1)
    {
        switch (c)
        {
        case 'h':
            usage();
            return 0;
        case 'c':
            cli_mode = strtol(optarg, NULL, 0);
            break;
        case OPT_CLI_DEBUG:
            cli_debug = 1;
            cli_debug_bitmap = optarg;
            break;
        case OPT_LIB_DEBUG:
            lib_debug = 1;
            lib_debug_level = optarg;
            break;
        case 's':
            default_scan_mode = strtol(optarg, NULL, 0);
            break;
        default:
            fprintf(stderr, "Try --help' for more information.\n");
            return 1;
        }
    }

    if (cli_debug)
    {
        opt_argc = 2;
        opt_argv[0] = cli_debug_bitmap;
        opt_argv[1] = "-1";

        btut_set_btcli_handler(opt_argc, &opt_argv[0]);
    }

    if (btut_init())
    {
        return -1;
    }

    if (lib_debug)
    {
        opt_argc = 2;
        opt_argv[0] = "3";
        opt_argv[1] = lib_debug_level;

        btut_set_log_lv_handler(opt_argc, &opt_argv[0]);
    }

    if (cli_mode)
    {
        btut_run();
        btut_deinit();
    }
    else
    {
        pause();
    }

    return 0;
}
